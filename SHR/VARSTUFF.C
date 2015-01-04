#include "graphint.h"   
#include "extrndb.h"  
#include "shapefil.h"
#include "umio.h"   

#include "gmextern.h"


struct	tm	tmtime;

typedef struct {
	int NumVars;
	int MaxVars;
	HANDLE hVarNameTable;
	HANDLE VarHandles[];
}VARSPACE;
typedef VARSPACE *LPVARSPACE;

static	LPVARSPACE pVarSpace = 0;
static	HANDLE hGlobalVarSpace = 0;
static	HANDLE hLocalVarSpace = 0;
static	DWORD	VarTime=1;
//static	int	NumVars=0;
//static	HANDLE	VarHandles[MAXGLOBALS];
static	char	shrtxt[256]; 
//static	HANDLE	hVarNameTable=0; 
static	BOOL	FoundLiteral=FALSE;   
static	LPCOMBOFILE	CurrentComboFile=NULL;
static	double	BaseUnitsPerOrthoPixel=1;  
static	short	nChangedGlobals=0;
static	HANDLE	ChangedGlobals[MAX_CHANGED_GLOBALS];     
static	HANDLE	UseOnlyOneDBHandle=0;
static	ULONG	NumFetch=0;
static	LPSTR	MBHMess, MBHTitle;
static	HWND	dlgListWnd = 0;
static	UINT	dlgListList;

extern char	VirtPrinterImageFile[256];

  typedef struct {
  			double r1, r2;
  			int		i1;
  		} Key; 
  Key *lpKey;
 
static	struct	{short	File, Field;} SFFieldData;        

void SetShowContourLines (BOOL In);
void SetShowDepthColors (BOOL In);
void SetHighlightDepth (int In);


HANDLE CreateVarSpace(int type)
{
	HANDLE hSpace;
	LPVARSPACE pVarSpace;

	switch (type)
	{
	case VARSPACE_GLOBAL:
		hSpace = GSSiGlobAlloc(2000, GHND, sizeof(VARSPACE) + MAXGLOBALS*sizeof(HANDLE));
		pVarSpace = GlobalLock(hSpace);
		pVarSpace->MaxVars = MAXGLOBALS;
		GlobalUnlock(hSpace);
		hGlobalVarSpace = hSpace;
		break;
	case VARSPACE_LOCAL:
		hSpace = GSSiGlobAlloc(2001, GHND, sizeof(VARSPACE) + MAX_LOCAL_VARS*sizeof(HANDLE));
		pVarSpace = GlobalLock(hSpace);
		pVarSpace->MaxVars = MAX_LOCAL_VARS;
		GlobalUnlock(hSpace);
		break;
	}
	return hSpace;
}

void SetVarSpace(int type, HANDLE hVarSpace)
{
	switch (type)
	{
	case VARSPACE_GLOBAL:
		break;
	case VARSPACE_LOCAL:
		hLocalVarSpace = hVarSpace;
		break;
	}
	return;
}
HANDLE SetVarSpaceFromName(LPSTR Name)
{
	HANDLE hVarSpace;

	if (*Name == '~' && hLocalVarSpace)
		hVarSpace = hLocalVarSpace;
	else
		hVarSpace = hGlobalVarSpace;
	return hVarSpace;
}

void DestroyVarSpace(HANDLE hVarSpace)
{
	if (hVarSpace == (HANDLE)-1)
	{
		hVarSpace = hGlobalVarSpace;
		HaveDL = FALSE;
	}
	if (hVarSpace)
	{
		LPVARSPACE savepVarSpace = pVarSpace;

		pVarSpace = GlobalLock(hVarSpace);
		CloseVars();
		GSSiGlobUlFree(&hVarSpace);
		pVarSpace = savepVarSpace;
	}
	return;
}
void SetUseOnlyOneDBHandle (HANDLE handle)
{
	UseOnlyOneDBHandle = handle;
	return;
}

void Wait (long MicroSeconds)
#if ENABLETRACE
{GSSiEnterProg (514);
#endif
{   
	MSG	msg; 
	UINT	MS=(UINT)MicroSeconds;  
	
	if (!MS)
		return;
	SetTimer (hWndMain,9999,MS,0);
	msg.wParam = 0; 
	while (msg.wParam != 9999)
		GSSiGetMessage (&msg,hWndMain,WM_TIMER,WM_TIMER); 
	KillTimer (hWndMain,9999);
{
#if ENABLETRACE
GSSiExitProg (514);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void Wait2 (long MicroSeconds)
#if ENABLETRACE
{GSSiEnterProg (514);
#endif
{   
	MSG	msg; 
	UINT	MS=(UINT)MicroSeconds;  
	
	if (!MS)
		return;
	SetTimer (hWndMain,9999,MS,0);
	msg.message = msg.wParam = 0; 
	while (msg.message != WM_TIMER || msg.wParam != 9999)
	{
		GSSiGetMessage (&msg,0,0,0); 
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	}
	KillTimer (hWndMain,9999);
{
#if ENABLETRACE
GSSiExitProg (514);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL UpdateGlobalFile (LPSTR RptFileIn,LPSTR VName,LPSTR Value)
#if ENABLETRACE
{GSSiEnterProg (515);
#endif
{   
	HANDLE	hStr=GSSiGlobAlloc ( 183,GMEM_MOVEABLE,4096);
	LPSTR	str=GlobalLock (hStr);
	HFILE	Fid; 
	LPSTR	lpEq,lpDot;
	HFILE	FidOut;
	char	OldFile[MAX_PATH], NewFile[MAX_PATH], RptFile[MAX_PATH], VarName[36];
	OFSTRUCTGM	OFStruct;
	BOOL	rtn=FALSE, Found=FALSE;
	BOOL	saveAllowCache = AllowCache;
	
	AllowCache = FALSE;
	_fstrcpy (RptFile,RptFileIn);
	ExpandText (RptFile);
	_fstrcpy(VarName,"[");
	_fstrcat(VarName,VName);
	_fstrcat(VarName,"]");
	Fid = GSSiOpenFile (RptFile,&OFStruct,OF_READ); 
	if (Fid==HFILE_ERROR) 
	{   
		if (Value)
		{
			makedirectories (RptFile,FALSE,FALSE);
			Fid = GSSiOpenFile (RptFile,&OFStruct,OF_CREATE); 
			if (Fid!=HFILE_ERROR) 
			{
				sprintf (str,"%s=%s",VarName,Value);
				fputstring (str,Fid);
				GSSiClose (Fid);
				GSSiGlobUlFree (&hStr);  
				rtn = TRUE;
			} 
		}
		GSSiGlobUlFree (&hStr);
		AllowCache = saveAllowCache;
{
#if ENABLETRACE
GSSiExitProg (515);
#endif
		return rtn;  
}    
	}
	_fullpath(OldFile,RptFile,sizeof(OldFile));
	_fstrcpy(NewFile,OldFile);
	lpDot = _fstrrchr(NewFile,'.');
	if (lpDot) *lpDot=0;
	_fstrcat(NewFile,".ugf");
	FidOut = GSSiOpenFile(NewFile,&OFStruct,OF_CREATE);  
	
	while (fgetstring (str,1024,Fid)) 
	{
		if (lpEq = _fstrchr(str,'='))
		{
			*lpEq=0;
			if (!_fstricmp(VarName,str))
			{   
				*lpEq++ = '=';
				if (Value)
					_fstrcpy (lpEq,Value);
				else
					*str=0;   
				Found = TRUE;
			} 
			else
				*lpEq = '=';
		}
		if (*str)
			fputstring (str,FidOut);
	}
	if (!Found && Value)
	{
		sprintf (str,"%s=%s",VarName,Value);
		fputstring (str,FidOut);
		Found = TRUE;
	}                     
	GSSiClose (Fid);
	GSSiClose (FidOut);
	CloseAllRequestedFiles(FALSE);
	if (!GSSiRemove(OldFile))
		GSSiRename (NewFile,OldFile);  
	else
		MessageBox (0,OldFile,"Unable to update file",MB_ICONEXCLAMATION);
	GSSiGlobUlFree (&hStr);
	AllowCache = saveAllowCache;
{
#if ENABLETRACE
GSSiExitProg (515);
#endif
	return Found;
}
#if ENABLETRACE
}
#endif
}
 
void GetRunDate (LPSTR RunID, LPSTR Date)
#if ENABLETRACE
{GSSiEnterProg (516);
#endif
{
	char	str[260];
	HFILE	Fid;  
	OFSTRUCTGM	OFStruct;
	LPSTR	lpComma; 
	
	Fid = GSSiOpenFile ("rundate.txt",&OFStruct,OF_READ);   
	*Date = 0;
	if (Fid==HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (516);
#endif
		return;  
}
	
	while (fgetstring (str,256,Fid)) 
	{
		if (lpComma = _fstrchr(str,','))
		{
			*lpComma++=0;
			if (!_fstricmp(RunID,str))
			{
				_fstrcpy(Date,lpComma);
				goto Exit;
			}
		}
	}                     
Exit:
	GSSiClose (Fid);
	
{
#if ENABLETRACE
GSSiExitProg (516);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

DWORD NextVarTime (void)
#if ENABLETRACE
{GSSiEnterProg (517);
#endif
{
	if (VarTime < ULONG_MAX)
		VarTime++;
	else
		VarTime = 0;
		/* reset all times to 0 */
{
#if ENABLETRACE
GSSiExitProg (517);
#endif
	return VarTime;
}
#if ENABLETRACE
}
#endif
}
                                         
BOOL LoadGlobalInit (LPSTR File,BOOL First)
#if ENABLETRACE
{GSSiEnterProg (518);
#endif
{   
	HANDLE	hMem=GSSiGlobAlloc (1504,GMEM_MOVEABLE,4096*3);
	LPSTR	str=GlobalLock (hMem), str2=str+4096, mess=str2+4096;
	HFILE	Fid;  
	int		n=0; 
	BOOL	noerr=TRUE; 
	LPSTR	lpEq, lpEnd;  
	BOOL	SaveIgnoreFileOpenError=IgnoreFileOpenError;
	static	BOOL	MainLoaded=FALSE;

	strcpy (str,File);
	ExpandText (str);
	_fullpath (str2,str,MAX_PATH);
	strcpy (mess,"[%DL]global.ini");
	ExpandText (mess);
	if (!stricmp (str2,mess))
	{
		if (MainLoaded)
		{
			GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (518);
#endif
			return TRUE;
}
		}
		MainLoaded = TRUE;
	}
	strlwr (str);
	if (strstr (str,"\\global.ini"))
		LayerID = 0;  
	IgnoreFileOpenError=TRUE;
	Fid = GSSiOpenFile (File,0,OF_READ);
	IgnoreFileOpenError = SaveIgnoreFileOpenError;
	if (Fid==HFILE_ERROR) 
	{ 
/*		LPSTR	pDot;
		
		_fstrcpy (str,File);
		if ((pDot = _fstrrchr (str,'.')))
		{
			_fstrcpy (pDot,".inb");
			Fid = GSSiOpenFile (str,0,OF_READ); 
			if (Fid != HFILE_ERROR)
				goto Next;
		} */
		GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (518);
#endif
		return TRUE; 
}
	}
Next:
	if (TraceOn)
	{ 
		sprintf (str,"Loading global init file %s",File);
		GSSiTrace (str,0);
	} 
	while (fgetstring (str,4090,Fid))
	{
		n++;
		if (str[0] != '\0' && str[0] != '#')
		{ 
/*			lpEq = _fstrchr (str,'=');
			if (!lpEq)
			{   
				sprintf (str2,"Error on line %i of %s",n,File);               
				noerr = FALSE;
				GSSiMsgBox( GetFocus(),str,str2,MB_ICONEXCLAMATION);
			}
			else
			{
				lpEq--;
				if (str[0] != '[' && *lpEq != ']')
				{
					sprintf (str2,"Error on line %i of %s",n,File);               
					noerr = FALSE;
					GSSiMsgBox( GetFocus(),str,str2,MB_ICONEXCLAMATION);
				}
				else
				{   
					*lpEq = '\0';  
					lpEq+=2;
					lpEnd = _fstrchr(lpEq,0);
					lpEnd--;
					if (*lpEnd == ';') *lpEnd = 0;
					_fstrcpy (str2,lpEq);
					if (First || _fstricmp (&str[1],"%DATA_LOC"))
						SetGlobalValue (&str[1],str2);  
					if (TraceOn)
					{ 
						sprintf (mess,"%s: %s",&str[1],str2);
						GSSiTrace (mess,0);
					}
					
				}
			}*/
			if (PeopleNet && !strncmp (str,"[STATE]=",8))
			{
				char str2[64]="[%STATE]=";
				
				strcat (str2,&str[8]);
				ProcessText (str2);
			}
			else
				ProcessText (str);
		}
	}
	GSSiClose (Fid);
	if (TraceOn)
	{ 
		sprintf (str,"Close global init file %s",File);
		GSSiTrace (str,0);
	}  
	GSSiGlobUlFree (&hMem);
	
{
#if ENABLETRACE
GSSiExitProg (518);
#endif
	return noerr;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetCurrentPNDBName (LPSTR Name)
{
	short	NumFields=0, NumIndexFields=1; 
	short	CurrentVersion=1; 
	HANDLE	hStr=GSSiGlobAlloc ( 184,GMEM_MOVEABLE,1024);
	LPSTR	DefStr = GlobalLock (hStr); 
	USHORT	StringID=IDS_PNDEF;
	BOOL	doDelete=FALSE;

	GetTempDir (DefStr);
	if (!Name)
	{
		doDelete = TRUE;
		Name = &DefStr[512];
	}
   	sprintf (Name,"%s\\grt%i%i.gmd",DefStr,CurrentVersion); 
   	if (!ExistFile (Name))
   	{
		if (doDelete)
			goto RtnFalse;
		if (!LoadString(hInst, StringID, DefStr, 512))
			goto RtnFalse;  
		if (!CreateGWDDatabase (Name,CurrentVersion,FALSE,0,NumIndexFields,DefStr))
			goto RtnFalse; 
	}
	else if (doDelete)
		GSSiRemove (Name);
	GSSiGlobUlFree (&hStr);
	return TRUE;
RtnFalse:
	GSSiGlobUlFree (&hStr);
	return FALSE;
}

BOOL GetCurrentGraphicsDBName (LPSTR Name,short Type)
#if ENABLETRACE
{GSSiEnterProg (519);
#endif
{
	short	NumFields=0, NumIndexFields=1; 
	short	CurrentVersion=1006; 
	HANDLE	hStr=GSSiGlobAlloc ( 184,GMEM_MOVEABLE,1024);
	LPSTR	DefStr = GlobalLock (hStr); 
	USHORT	StringID=IDS_GRAPHICSDEF;
	BOOL	doDelete=FALSE;
    
    if (Type == 2)
    {   
    	NumFields = 0;
    	CurrentVersion = 2003;
    	StringID = IDS_GRAPHIC2DEF;
    }
	GetTempDir (DefStr);
	if (!Name)
	{
		doDelete = TRUE;
		Name = &DefStr[512];
	}
   	sprintf (Name,"%s\\grt%i%i.gmd",DefStr,Type,CurrentVersion); 
   	if (!ExistFile (Name))
   	{
		if (doDelete)
			goto RtnFalse;
		if (!LoadString(hInst, StringID, DefStr, 512))
			goto RtnFalse;  
		if (StringID == IDS_GRAPHICSDEF)
		{
			if (!LoadString(hInst, IDS_GRAPHICSDEF2, _fstrchr (DefStr,0), 512))
				goto RtnFalse;
		}  
		else if (StringID == IDS_GRAPHIC2DEF)
		{
			if (!LoadString(hInst, IDS_GRAPHIC2DEF2, _fstrchr (DefStr,0), 512))
				goto RtnFalse;
		}  
		if (!CreateGWDDatabase (Name,CurrentVersion,FALSE,0,NumIndexFields,DefStr))
			goto RtnFalse; 
	}
	else if (doDelete)
		GSSiRemove (Name);
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (519);
#endif
	return TRUE;
}
RtnFalse:
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (519);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 

int ReadMultiLine (HFILE Fid,LPSTR str)
{   
	LPSTR	strb=str;   
	long	loc;
	
	fgetstring (str,1024,Fid);
	loc = GSSillseek (Fid,0,1);
	str = _fstrchr (str,0); 
	if (fgetstring (str,1024,Fid))
		while (*str == ' ')
		{
			loc = GSSillseek (Fid,0,1);
			str = _fstrchr (str,0);  
			if (!fgetstring (str,1024,Fid))
				break;
		} 
	GSSillseek (Fid,loc,0);
	*str = 0;
	return _fstrlen (str);
}

int GetNumDBFields (HANDLE hDB)
{
	int rtn = 0;
	if (hDB)
	{
		LPOPENSQLDATA	SQLPtr = (LPOPENSQLDATA)GlobalLock (hDB); 
		LPOPENFILEDATA	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
		rtn = FilePtr->NumFields;
		GlobalUnlock (SQLPtr->OFHandle);
		GlobalUnlock (hDB);
	}
	return rtn;
}


BOOL SetSQLDatabaseFields (LPSTR Name)
{
	HANDLE hDB = 0;
	LPOPENSQLDATA	SQLPtr;
	LPOPENFILEDATA	FilePtr;
	LPSQLDATABASE	pSQL;
	int	i;
	LPFIELDINFO	pFieldInfo;
	char	str[256];

	if (!OpenDataFile (Name,"",BT_READ,&hDB))
		return FALSE;
	SQLPtr = (LPOPENSQLDATA)GlobalLock (hDB); 
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	pSQL=(LPSQLDATABASE)GlobalLock (FilePtr->FileHandle);
	pSQL->NumFields = 0;
	{
		LPOPENSQLDATA	SQLPtr = GlobalLock (pSQL->DBHandle);
		LPOPENFILEDATA	FilePtr = GlobalLock (SQLPtr->OFHandle);

		FilePtr->NumFields = 0;
		GlobalUnlock (SQLPtr->OFHandle);
		GlobalUnlock (pSQL->DBHandle);
	}
	GlobalUnlock (FilePtr->FileHandle);
	FilePtr->NumFields = 0;
	GlobalUnlock (SQLPtr->OFHandle); 
	GlobalUnlock (hDB);
	FetchDBRec (hDB);
	SQLPtr = (LPOPENSQLDATA)GlobalLock (hDB); 
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	pSQL=(LPSQLDATABASE)GlobalLock (FilePtr->FileHandle);
	{
		LPOPENSQLDATA	SQLPtr = GlobalLock (pSQL->DBHandle);
		LPOPENFILEDATA	FilePtr = GlobalLock (SQLPtr->OFHandle);
		HFILE	Fid = GSSiOpenFile (Name,0,OF_READWRITE);

		fgetstring (str,254,Fid);
		while (stricmp (str,"#FIELDS") && fgetstring (str,254,Fid));

		pFieldInfo = &FilePtr->FldInfo;
		for (i=0;i<FilePtr->NumFields;i++,pFieldInfo++)
		{
			sprintf (str,"%i,%i,%i,%i,%i,%i,%s",
									   pFieldInfo->type,pFieldInfo->index,pFieldInfo->radix,
									   pFieldInfo->scale,pFieldInfo->length,pFieldInfo->precision,pFieldInfo->name);
			fputstring (str,Fid);
		}
		GSSiClose (Fid);
		GlobalUnlock (SQLPtr->OFHandle);
		GlobalUnlock (pSQL->DBHandle);
	}
	GlobalUnlock (FilePtr->FileHandle);
	GlobalUnlock (SQLPtr->OFHandle); 
	GlobalUnlock (hDB);
	CloseDataFile (TRUE,&hDB);
	return TRUE;
}

HANDLE	OpenSQLDatabase (LPSTR Name,LPSTR SQL)
{
    HFILE	Fid;
	LPSQLDATABASE	pDB;   
	HANDLE	hDB;  
	char	str[130]; 
	HANDLE	hMem;   
	LPSTR	pMem;
	
    Fid = GSSiOpenFile (Name,0,OF_READ);
    if (Fid == HFILE_ERROR)
    	return 0; 
    hDB = GSSiGlobAlloc (1505,GHND,USHRT_MAX);
    pDB = (LPSQLDATABASE)GlobalLock (hDB);
    fgetstring (pDB->DBName,126,Fid);
    ReadMultiLine (Fid,pDB->Select);
    ReadMultiLine (Fid,pDB->From);
//    ReadMultiLine (Fid,pDB->Where); 
    fgetstring (str,32,Fid);
    while (fgetstring (str,128,Fid))
    {   
		int ii=sscanf (str,"%i,%i,%i,%i,%i,%i,%s",
										   &pDB->FldInfo[pDB->NumFields].type,
										   &pDB->FldInfo[pDB->NumFields].index,
										   &pDB->FldInfo[pDB->NumFields].radix,
										   &pDB->FldInfo[pDB->NumFields].scale,
										   &pDB->FldInfo[pDB->NumFields].length,
										   &pDB->FldInfo[pDB->NumFields].precision,
										   pDB->FldInfo[pDB->NumFields].name);
//    	_fstrcpy (pDB->FldInfo[pDB->NumFields].name,str);
    	pDB->NumFields++;
    }
	GSSiClose (Fid);  
	hMem = GSSiGlobAlloc (1506,GMEM_MOVEABLE,USHRT_MAX);
	pMem = GlobalLock (hMem);  
	_fstrcpy (pMem,pDB->Select);
	_fstrcat (pMem,pDB->From); 
	if (*SQL)
	{
		_fstrcat (pMem," WHERE "); 
		_fstrcat (pMem,SQL);
	}
	if (!OpenDataFile (pDB->DBName,pMem,BT_READ,&pDB->DBHandle))   
  	{
  		GSSiGlobUlFree (&hDB); 
  		GSSiGlobUlFree (&hMem);
  		return 0;
  	}
	GSSiGlobUlFree (&hMem);
  	GlobalUnlock (hDB);
  	return hDB;
}

BOOL GetNextPipeDelimitedValue (LPSTR str,LPINT pPos,LPSTR Value)
{
	int		lval=0;
	LPSTR	pBeg;

	if (str[(*pPos)++] != '|') //str[7]
		return FALSE;

	pBeg = &str[*pPos];

	while (str[*pPos] && str[*pPos] != '|')
	{
		lval++;
		(*pPos)++;
	}
	if (str[*pPos] != '|')
		return FALSE;
	(*pPos)++;
	strncpy0 (Value,pBeg,lval);
	return TRUE;
}

HANDLE	OpenLISTVARDatabase (LPSTR Name)
{
	LPLISTVARDATABASE	pDB;   
	HANDLE	hDB;  
	char	str[260];
	int		savePos, maxlen=0;
	
    hDB = GSSiGlobAlloc (1672,GHND,USHRT_MAX);
    pDB = (LPLISTVARDATABASE)GlobalLock (hDB);
	GetGlobalCVal (&Name[1],pDB->Value,0);
	if (*pDB->Value != '|')
	{
		GSSiGlobUlFree (&hDB);
		goto Exit;
	}
	pDB->NumFields = 1;
	pDB->Pos = 1;
	while (pDB->Value[pDB->Pos] && pDB->Value[pDB->Pos] != '|')
		pDB->Pos++;
	if (!pDB->Value[pDB->Pos])
	{
		GSSiGlobUlFree (&hDB);
		goto Exit;
	}
	pDB->Value[pDB->Pos++] = 0;
	strcpy (pDB->FldInfo[0].name,&pDB->Value[1]);

	savePos = pDB->Pos;
	while (GetNextPipeDelimitedValue (pDB->Value,&pDB->Pos,str))
	{
		maxlen = max (maxlen,strlen (str));
		pDB->NumRows++;
	}
	pDB->Pos = savePos;

    pDB->FldInfo[0].type = BT_CHAR;
    pDB->FldInfo[0].index = 0;
    pDB->FldInfo[0].radix = 0;
    pDB->FldInfo[0].scale = 0;
    pDB->FldInfo[0].length = maxlen;
    pDB->FldInfo[0].precision = 0;
   	strcpy (pDB->FldInfo[0].name,&pDB->Value[1]);
  	GlobalUnlock (hDB);
Exit:
  	return hDB;
}

short OpenDataFile (LPSTR InName, LPSTR SQL, short Access, HANDLE *hDB)
#if ENABLETRACE
{GSSiEnterProg (520);
#endif
{ 
	short	i, NumFields,Type, l, nf; 
	LPSTR	pTable=0,pEnd;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	LPFIELDINFO	lpFieldInfoSave, lpFieldInfo;
	HANDLE	FileHandle=0, handle, hFields;
	LPGWDHEADER	lpGWDHead;
	LPSTR	lpEq, lpDelim,lpFieldStart, pDot;
	short	index, LocDelim;   
	LPSTR	IDName;
	LPSTR	str, cName;   
   	LPSTR	drive,dir,fname,ext, driver, table;   
   	LPSTR	Name, TxtRecord;
   	LPOFSTRUCTGM	pOFStruct;
	HANDLE	hMem=0;
	HFILE	Fid=HFILE_ERROR;    
	long	GMTextFirstLine=0; 
	BOOL	HaveNonStandardFields=FALSE;  
	LPSTR	lpEQ;
	BOOL	RetrieveFields = FALSE;
	
    if (*hDB) 
    {
		SQLPtr = (LPOPENSQLDATA) GlobalLock (*hDB);
        FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
	    Type = FilePtr->Type;  
	    GlobalUnlock (SQLPtr->OFHandle);
	    GlobalUnlock (*hDB);
{
#if ENABLETRACE
GSSiExitProg (520);
#endif
	    return Type;
}
	} 
	if (!*InName)
		goto RtnFalse;
	if (Access == -1)
	{
		Access = BT_READ;
		RetrieveFields = TRUE;
	}
	hMem=GSSiGlobAlloc ( 185,GHND,USHRT_MAX);
	str = GlobalLock (hMem);
	IDName = str + 256;
	cName = IDName + 256;
	drive = cName + 256;
	dir = drive + 8;
	fname = dir + 128;
	ext = fname + 64;
	driver = ext + 8;   
	table = driver + 128;
	pOFStruct = (LPOFSTRUCTGM)(table + 256); 
	TxtRecord = (LPSTR) (pOFStruct + 1);
	Name=cName;
    _fstrcpy (Name,InName);
    ExpandText (Name);
	ExpandText (Name);//for $SYMATTRFILE(sym)
    _fstrlwr (Name); 
    if (_fstrstr (Name,"attribut\\graphics.gmd"))   
    {
    	LPSTR lpEQ = _fstrchr (Name,'=');
    	
    	if (lpEQ)
    		lpEQ++;
    	else
    		lpEQ = Name;  
    	GetCurrentGraphicsDBName (lpEQ,1);
    }
    if (_fstrstr (Name,"attribut\\graphic2.gmd"))   
    {
    	LPSTR lpEQ = _fstrchr (Name,'=');
    	
    	if (lpEQ)
    		lpEQ++;
    	else
    		lpEQ = Name;  
    	GetCurrentGraphicsDBName (lpEQ,2);
    }
//    ExpandText (Name);    
    
    if (!_fstricmp (Name,"HLTLIST"))
    {   
		OpenHighlightList(0,0);
    	HltFetchPos = BT_FIRST;
		Type = HLTLIST_DATAFILE;
    	goto RtnTrue;
    }
    if (!_fstricmp (Name,"POLY"))
    {   
    	GSSiGlobFree (&hPolyCoord);
		CurPolyCoord = 0;
		Type = POLY_DATAFILE;
		if (!strnicmp (SQL,"%INT_REFNO=",11))
		{
			long	Refno = atol (&SQL[11]);

    		goto RtnTrue;
		}
		if (!strcmp (SQL,"%PICKED"))
		{
			if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&NumPolyCoord,&hPolyCoord))
				goto RtnTrue;
		}
		goto RtnFalse;
    }
    
	lpEq = MatchLev (Name,'=');
	LocDelim = _fstrcspn (Name,",|(;");
	if ((long)lpEq - (long)Name > LocDelim) 
		lpEq = 0;
	if (lpEq) 
	{
		*lpEq = 0;
		_fstrcpy (IDName,Name);
		*lpEq = '=';
		Name = ++lpEq;
	}
	l = _fstrlen (Name);
    if (_fstrncmp(Name,"ODBC|",5))
    	_fstrupr (Name);
    if (_fstrstr(Name,".GMD"))
    {
    	Type = UMIFS_DATAFILE;  
    	if (_fstrstr (Name,"INFOBOXES.GMD"))
    		CreateInfoBoxGMD (Name);
    }
    else if (_fstrstr(Name,".ORA"))
    {   
    	pDot = _fstrrchr (Name,'.');
    	_fstrcpy (pDot,".GMD");
    	Type = ORA_DATAFILE; 
    }
	else if (*Name == '.')
    	Type = LISTVAR_DATAFILE;
	else if (strstr (Name,"THEME:"))
    	Type = THEME_HLTFILE;
    else if (_fstrstr(Name,".SQL"))
    	Type = SQL_DATAFILE;
    else if (_fstrstr(Name,".SHP"))
    	Type = SHAPE_DATAFILE;
    else if (_fstrstr(Name,".PND"))
	{
    	Type = PN_DATAFILE;
    	GetCurrentPNDBName (Name);
	}
    else if (_fstrstr(Name,".MDB("))  
    	Type = PGDB_DATAFILE;    
    else if (_fstrstr(Name,".GDB"))  
    	Type = FGDB_DATAFILE;    
    else if (_fstrstr(Name,".GCN"))
    	Type = GMCENSUS_DATAFILE;
    else if (_fstrstr(Name,".GCF"))
    	Type = COMBO_DATAFILE;
    else if (_fstrstr(Name,".DBF"))
    	Type = DBF_DATAFILE;
    else if (!_fstrncmp (Name,"ODBC|",5))
    	Type =ODBC_DATAFILE;
    else if (strstr(Name,".TXT") || strstr(Name,".CSV"))
    	Type =GMTEXT_DATAFILE;
    else if (_fstrstr(Name,".HLT"))
    	Type =HLTLIST_DATAFILE;
    else if (_fstrstr(Name,".DTM") || _fstrstr(Name,".LDR") || _fstrstr(Name,"INDEX.TIN"))
    	Type =DTM_DATAFILE;
    else if (_fstrstr(Name,".BMP") || _fstrstr(Name,".JPG") || _fstrstr(Name,".TIF"))
    	Type =IMAGE_DATAFILE;
    else
    	goto RtnFalse;
   // if ((FilePtr = InOpenFileList(Name,Type,Access)))
   // 	goto ProcessSQL;
	switch (Type)
	{   
		case 0:
			FileHandle = NULL;
			break;
			

	    case PN_DATAFILE:
		case UMIFS_DATAFILE:
		case ORA_DATAFILE:
			FileHandle = OpenGWDatabase (Name,Access);
			break;

		case GMCENSUS_DATAFILE:  
		{
		    LPGWDHEADER lpGWDHead;

			if ((FileHandle = OpenGWDatabase (Name,Access)))
			{
				lpGWDHead = (LPGWDHEADER)GlobalLock (FileHandle); 
				pDot = _fstrrchr (Name,'.');
				_fstrcpy (pDot,".FLD");
				lpGWDHead->BTHandle[lpGWDHead->NumIndex] = BT_OPEN (Name,0,BT_READ,0);
				_fstrcpy (pDot,".GCN");
				GlobalUnlock (FileHandle);   
			}
		}
			break;

		case THEME_HLTFILE:
		{
		  	short iview;
    	
			for (iview=0;iview<*pNumViewports;iview++)
			{   
        		if (!_fstricmp (&Name[6],pViewports[iview]->Name)) 
        		{
        			if (pViewports[iview]->pTheme)
        			{   
        				LPTHEME SaveTheme = CurTheme;
        				
        				CurTheme = pViewports[iview]->pTheme;
        				if (OpenThemeHighlightFile (BT_READ))
        				{    
							FileHandle = CurTheme->hHighlightFile;
        					CurTheme = SaveTheme;
			    			break;
			    		} 
			    		CurTheme = SaveTheme;
        			}
        		}
			}
		}
		break;

		case COMBO_DATAFILE:
			FileHandle = OpenComboDatabase (Name,SQL);
			break;

		case FOXPRO_DATAFILE:
	       	FileHandle = OpenExternalDatabase (Name); 
	       	break;

	    case MSACCESS_DATAFILE:
	       	FileHandle = OpenExternalDatabase (Name);
 	    	break;  
 	    	
 	    case SQL_DATAFILE:
 	    	FileHandle = OpenSQLDatabase (Name,SQL);
 	    	break;

		case LISTVAR_DATAFILE:
 	    	FileHandle = OpenLISTVARDatabase (Name);
 	    	break;

 	    case IMAGE_DATAFILE:
 	    	FileHandle = LoadDIB32(Name,FALSE);
 	    	break;

	    case DTM_DATAFILE:
	       	FileHandle = DTMOpen (Name,DBL_MAX,BT_READ,0);
 	    	break; 

        case ODBC_DATAFILE:
			{
				LPSTR	pDBQ=strstr (Name,";DBQ=");

				if (pDBQ)
				{
					strcpy (str,pDBQ+5);
					if ((pEnd = strpbrk (str,";|")))
						*pEnd = 0;
					if (!ExistFile (str))
					{
						FileHandle = NULL;
						break;
					}
				}

				//ODBCTerminate (TRUE);
	       		FileHandle = OpenExternalDatabase (Name); 
				if(FileHandle)
				{
		       		pTable = (_fstrchr(Name,'|')) ;
		       		if(pTable)
		       		{ 
		       		  pTable++;
		       		  pTable = (_fstrchr(pTable,'|')) ;
					  if(pTable)
					  { 
		       			pTable++;
		          		FileHandle =(HANDLE)OpenDatabaseTable ((int)FileHandle, pTable);
					  }
					}  
				}
				if (!pTable)
				{
					pDBQ=strstr (Name,";DBQ=");

					SubstituteDBQ (InName,pDBQ);
				}
			}
 	    	break;
        case PGDB_DATAFILE:  
        {
        	char	Name2[256];
        	
		    if (!(pTable = _fstrrchr (Name,'('))) 
		    	break; 
		    *pTable++ = 0; 
		    *LastChr (pTable) = 0; 
		    sprintf (Name2,"ODBC|MS Access Database;DBQ=%s|%s",Name,pTable);
	       	FileHandle = OpenExternalDatabase (Name2); 
            if(FileHandle)
	          	FileHandle =(HANDLE)OpenDatabaseTable ((int)FileHandle, pTable);
	    }
 	    	break;
        case FGDB_DATAFILE:  
        {
        	char	Name2[256];
        	
			if ((pTable = _fstrrchr(Name, '(')))
			{
				*pTable++ = 0;
				*LastChr(pTable) = 0;
			}
	       	FileHandle = (HANDLE)OpenFGDB2 (Name,pTable,"");
            if(FileHandle && pTable)
	          	FileHandle =(HANDLE)OpenDatabaseTable ((int)FileHandle, pTable);
	    }
 	    	break;
        case SHAPE_DATAFILE:
        {   
			DBFHandle pDBF;  
			LPDWORD	pDBFAddress;
			LPSTR	pSHP = _fstrstr (Name,".SHP");
            
            if (!pSHP)
            	break;
            _fstrcpy (pSHP,".DBF");
		    pDBF = DBFOpen(Name, "r"); 
		    if (!pDBF)
		    	break;  
		    NumSHPDBFRecs = pDBF->nRecords; 
		    CurrentSHPRec = 0;
		    FileHandle = GSSiGlobAlloc ( 186,GMEM_MOVEABLE,sizeof(pDBF));
		    pDBFAddress = (LPDWORD)GlobalLock (FileHandle);
		    *pDBFAddress = (DWORD)pDBF;  
		    GlobalUnlock (FileHandle);
/*	short	FieldWidth, FieldDecimals;
    nFields= DBFGetFieldCount(pDBF );
    nRecs =  DBFGetRecordCount(pDBF );  
    for (i=0;i<nFields;i++)
    {
    	DBFGetFieldInfo(pDBF, i, FieldName, &FieldWidth,&FieldDecimals );
    }
	x1=DBFReadDoubleAttribute(pDBF,1, 1);
	Refno=DBFReadIntegerAttribute(pDBF,1, 2);
	Refno=DBFReadIntegerAttribute(pDBF,2, 3);
	pLoc=DBFReadStringAttribute(pDBF,3, 4);
	pLoc=DBFReadStringAttribute(pDBF,3, 7);
    DBFClose(pDBF);*/
	    }
 	    	break;
        case DBF_DATAFILE:
        {   
			DBFHandle pDBF;  
			LPDWORD	pDBFAddress;

		    pDBF = DBFOpen(Name, "r"); 
		    if (!pDBF)
		    	break;  
		    NumDBFRecs = pDBF->nRecords; 
		    CurrentDBFRec = 0;
		    FileHandle = GSSiGlobAlloc ( 186,GMEM_MOVEABLE,sizeof(pDBF));
		    pDBFAddress = (LPDWORD)GlobalLock (FileHandle);
		    *pDBFAddress = (DWORD)pDBF;  
		    GlobalUnlock (FileHandle);


/*
        	_splitpath (Name,drive,dir,fname,ext); 
			if (*LastChr (dir) == '\\')
				*LastChr (dir) = 0;
        	sprintf (driver,"dBASE Files;DefaultDir=%s%s",drive,dir);
	       	FileHandle = OpenExternalDatabase (driver); 
            if(FileHandle && *fname != '*')
            {   
            	sprintf (table,"%s%s",fname,ext);
	          	FileHandle =(HANDLE)OpenDatabaseTable ((int)FileHandle, Name);
	        } */
	    }
 	    	break;
        case TEXT_DATAFILE:
        {   
        	HFILE	Fid=GSSiOpenFile (Name,pOFStruct,OF_READ);
        	
        	if (Fid == HFILE_ERROR) 
        	{
				FileHandle = NULL;
				GSSiMessageBox ("Unable to open TXT file",Name,MB_ICONEXCLAMATION,0);
			}
			else
			{   
				GSSiClose (Fid);
		       	_splitpath (pOFStruct->szPathName,drive,dir,fname,ext); 
	        	sprintf (driver,"TXT;DefaultDir=%s%s",drive,dir);
		       	FileHandle = OpenExternalDatabase (driver); 
	            if(FileHandle)
	            {   
	            	sprintf (table,"%s%s",fname,ext);
		          	FileHandle =(HANDLE)OpenDatabaseTable ((int)FileHandle, table);
		        }
		    } 
	    }
 	    	break;
        case GMTEXT_DATAFILE:
        {   
        	
        	Fid=GSSiOpenFile (Name,pOFStruct,OF_READ);
        	
        	if (Fid == HFILE_ERROR) 
        	{
GMTEXT_ERROR:
				FileHandle = NULL;
				//GSSiMessageBox ("Unable to open TXT file",Name,MB_ICONEXCLAMATION);
			}
			else
			{   
				if (!ProcessDelimTextHeader(TxtRecord, Name, Fid, &FileHandle, 0,  IDName))
				{
					GSSiClose (Fid);
					goto GMTEXT_ERROR; 
				} 
				GMTextFirstLine = GSSillseek (Fid,0,1);
		    } 
	    }
    		break;
        case HLTLIST_DATAFILE:
        {   
 			FileHandle = BT_OPEN (Name, 0, BT_READ, 0);
	    }
    		break;
    }
    if(!FileHandle)
    	goto RtnFalse;
    	            
    NumFields = 0;  
    hFields = GSSiGlobAlloc (1752,GMEM_MOVEABLE,MAXFIELDS*sizeof(FIELDINFO));
    lpFieldInfoSave = (LPFIELDINFO)GlobalLock(hFields);  
    index = 0;
	lpFieldInfo = GetFieldInfo (FileHandle,TRUE,Type,&HaveNonStandardFields);
	while (lpFieldInfo)
	{   
		lpFieldInfo->index = index++; 
		lpFieldInfo->hCurVal = 0;
		*lpFieldInfoSave++ = *lpFieldInfo;
		NumFields++;
		lpFieldInfo = GetFieldInfo (FileHandle,FALSE,Type,&HaveNonStandardFields); 
		if (index >= MAXFIELDS)
			break;
	}
	GlobalUnlock(hFields);
    if (!NumFields && pTable) 
    {
    	GSSiGlobFree (&hFields);
    	goto RtnFalse;	
    }
AllocFilePtr: 
	nf = NumFields;
	if (!nf)
		nf = 255;
    handle = GSSiGlobAlloc (1753,GHND,sizeof(OPENFILEDATA)+nf*sizeof(FIELDINFO));
    FilePtr = (LPOPENFILEDATA)GlobalLock (handle);
    FilePtr->myhandle = handle;  
    FilePtr->Type = Type;  
    FilePtr->Fid = Fid;
	if (pTable)
		strcpy(FilePtr->table, pTable);
    FilePtr->HaveNonStandardFields = HaveNonStandardFields;
	{
		char path[MAX_PATH];

		strcpy (path,Name);
		ExpandText (path);
		if (Type == UMIFS_DATAFILE || Type == ORA_DATAFILE || Type == DTM_DATAFILE || Type == GMCENSUS_DATAFILE || Type == SHAPE_DATAFILE || Type == IMAGE_DATAFILE)
			_fullpath (FilePtr->fullpath,path,_MAX_PATH);  
		else
			_fstrcpy (FilePtr->fullpath,path); 
	}
/*	if (FilePtr->Fid != HFILE_ERROR)
		FilePtr->Offset = GSSillseek (FilePtr->Fid,0,1);
	else
		FilePtr->Offset = -1;   */
    FilePtr->FileHandle = FileHandle;
    FilePtr->NumFields = NumFields;
    lpFieldInfoSave = (LPFIELDINFO)GlobalLock(hFields);
	lpFieldInfo = &FilePtr->FldInfo;
	if (NumFields)
		for (i=0;i<NumFields;i++,lpFieldInfoSave++,lpFieldInfo++)
			*lpFieldInfo = *lpFieldInfoSave;
/*	else //enabling this keeps PGDBs from working
	{
		for (i=0;i<nf;i++,lpFieldInfoSave++,lpFieldInfo++) 
		{   
			if (!i)
				_fstrcpy (lpFieldInfo->name,"RESULT");
			else
				sprintf (lpFieldInfo->name,"RESULT%i",i+1);  
			lpFieldInfo->type = SQL_VARCHAR;
			lpFieldInfo->length = 255;
		}
	    FilePtr->NumFields = nf;
	}*/
    GSSiGlobUlFree (&hFields);
    if (FilePtr->Type == SQL_DATAFILE)
    { 
    	LPSQLDATABASE pDB = (LPSQLDATABASE)GlobalLock (FilePtr->FileHandle);
		LPOPENSQLDATA SQLPtr2 = (LPOPENSQLDATA) GlobalLock (pDB->DBHandle);
        LPOPENFILEDATA FilePtr2;
		LPFIELDINFO	lpFieldInfo2;  
		
    	SQLPtr2->OFHandle = GSSiGlobalReAlloc (0,SQLPtr2->OFHandle ,sizeof(OPENFILEDATA)+FilePtr->NumFields*sizeof(FIELDINFO),GHND);
		FilePtr2 = (LPOPENFILEDATA)GlobalLock (SQLPtr2->OFHandle);
        FilePtr2->NumFields = FilePtr->NumFields;   
       	lpFieldInfo = &FilePtr->FldInfo;
       	lpFieldInfo2 = &FilePtr2->FldInfo;
    	for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++,lpFieldInfo2++)
    	{  
    		*lpFieldInfo2 = *lpFieldInfo;
    	}  
    	GlobalUnlock (SQLPtr2->OFHandle);
    	GlobalUnlock (pDB->DBHandle); 
    	GlobalUnlock (FilePtr->FileHandle);
    }
ProcessSQL: 
    handle = GSSiGlobAlloc(1754,GHND,sizeof(OPENSQLDATA)+MAXFIELDINSQL*sizeof(SQLFIELD));
    SQLPtr = (LPOPENSQLDATA)GlobalLock (handle); 
    _fstrcpy (SQLPtr->IDName,IDName);
    FilePtr->SQLHandles[FilePtr->NumSQLs++] = handle;
    SQLPtr->myhandle = handle;
	SQLPtr->Access = Access;
    SQLPtr->MacroID = CurrentMacro;   
    SQLPtr->OFHandle = FilePtr->myhandle; 
    FilePtr->FirstLineOffset = GMTextFirstLine;
	ProcessFileSQL (SQLPtr,FilePtr,SQL); 
	if (Type == SQL_DATAFILE)
	{
		LPSQLDATABASE	pDB = GlobalLock (FilePtr->FileHandle);
		LPOPENSQLDATA	SQLPtr2 = GlobalLock (pDB->DBHandle);
		
		SQLPtr2->MacroID = -1;
		itoa ((int)SQLPtr->myhandle,SQLPtr->myhandleC,16);
		strcpy (SQLPtr2->IDName,SQLPtr->myhandleC);
		GlobalUnlock (pDB->DBHandle);
		GlobalUnlock (FilePtr->FileHandle);
	}
    if (RetrieveFields)
		SQLPtr->IndexToUse = -1;
    GlobalUnlock (SQLPtr->myhandle);  
    GlobalUnlock (FilePtr->myhandle);  
    *hDB = handle;
    AddToOpenFileList(handle); 
RtnTrue:  
	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (520);
#endif
	return Type;
}
RtnFalse:
	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (520);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 

void ExpandSYMATTRKEY(LPSTR str)
{
	LPSTR pLoc = strstr(str, "$SYMATTRKEY(");
	LPSTR pBeg, pRest;

	if (pLoc)
	{
		HANDLE hMem = GSSiGlobAlloc(0, GMEM_MOVEABLE, 4096*2);
		LPSTR  pMem = GlobalLock(hMem);
		LPSTR  pRest = pMem + 4096;

		pBeg = pLoc + 12;
		pBeg = MatchLev(pBeg, ')');
		if (pBeg)
		{
			strcpy(pRest, ++pBeg);
			*pBeg = 0;
			strcpy(pMem, pLoc);
			ExpandText(pMem);
			strcpy(pLoc, pMem);
			strcat(pLoc, pRest);
		}
		GSSiGlobUlFree(&hMem);
	}
	return;
}

void ProcessFileSQL (LPOPENSQLDATA SQLPtr,LPOPENFILEDATA FilePtr,LPSTR SQLIN)
#if ENABLETRACE
{GSSiEnterProg (521);
#endif
{
    short	NumFieldInSQL = 0; 
    HANDLE	hMem=GSSiGlobAlloc ( 187,GMEM_MOVEABLE,4096*2);
    LPSTR	str=GlobalLock (hMem);
	LPSTR	SQL = str + 4096;
    HANDLE	hFields = GSSiGlobAlloc (1755,GHND,MAXFIELDINSQL*sizeof(SQLFIELD));
    LPSQLFIELD	SQLField = (LPSQLFIELD)GlobalLock(hFields);     
	LPSQLFIELD	lpSQLField; 
    LPSTR	lpFieldStart = str;  
    LPSTR	lpEq; 
    LPSTR	pEnd, pBrack, pColon;
    LPGWDHEADER	lpGWDHead;    
	LPGWFLDINFO	lpGWFldInfo; 
	LPFIELDINFO	pFieldInfo;
	char litAndBracket[3] = "@[";
    short	i,ii;
    
	strcpy(SQL, SQLIN);
	if ((pColon = strrchr(SQL, ':')) && !charLevel (pColon,SQL))
	{
		*pColon++ = 0;
		for (i = 0, pFieldInfo = &FilePtr->FldInfo; i < FilePtr->NumFields; i++, pFieldInfo++)
		{
			if (!stricmp(pColon, pFieldInfo->name))
			{
				SQLPtr->singleValID = i + 1;
				break;
			}
		}
	}
	litAndBracket[0] = literalChar;
    _fstrcpy (str,SQL); 
	ExpandSYMATTRKEY(str);
    if (*str == '(' && *LastChr(str) == ')')
    {
	    _fstrcpy (SQLPtr->SQL,&str[1]);  
		*LastChr (SQLPtr->SQL) = 0;
    }
    else if (!_fstrnicmp (str,litAndBracket,2))
    {
    	ExpandText (str);
    	ExpandText (str);
	    _fstrcpy (SQLPtr->SQL,str);  
    } 
    else
	    _fstrcpy (SQLPtr->SQL,str);  
    if (FilePtr->Type == GMTEXT_DATAFILE || FilePtr->Type == ORA_DATAFILE || FilePtr->Type == UMIFS_DATAFILE || FilePtr->Type == GMCENSUS_DATAFILE)
    {   
    	_fstrcpy (str,SQLPtr->SQL);
    	ConvertSQLToLogicP (SQLPtr->SQL,str); 
    }       
    pEnd = SQLPtr->SQL;
    while ((pBrack = _fstrchr (pEnd,'[')))  
    {   
    	pBrack++;
    	if (!(pEnd = _fstrchr (pBrack,']')))
    		break; 
    	if (*pBrack == '.')
    		pBrack++;
    	*pEnd = 0;
   		SQLField->FieldNum = -1;  
   		SQLField->OpCode = 0;
    	SQLField->hGlobal = AllocateVar (pBrack);
    	NumFieldInSQL++;
    	SQLField++; 
    	*pEnd++ = ']';
    }
    if (FilePtr->Type == UMIFS_DATAFILE || FilePtr->Type == ORA_DATAFILE || FilePtr->Type == GMCENSUS_DATAFILE || FilePtr->Type == GMTEXT_DATAFILE) 
    {   
		LPLOGICPSTATEMENT	pStatement;
		LPHANDLE			phStatement; 
		HANDLE				hSaveMacro=0; 
		LPSTR				pMacro, pSaveMacro;
		BOOL				Err;
		BOOL				HaveOR=FALSE;

    	hLogicPStatements = GSSiGlobAlloc ( 188,GMEM_MOVEABLE,1024);
    	nLogicPStatements = 0;
    	if ((pMacro = _fstrstr (SQLPtr->SQL,"=@$MACRO"))) 
    	{
    		hSaveMacro = GSSiGlobAlloc (1507,GMEM_MOVEABLE,1024);   
    		pSaveMacro = GlobalLock (hSaveMacro);
    		
    		_fstrcpy (pSaveMacro,&pMacro[1]);
    		_fstrcpy (pMacro,"=1"); 
    		GlobalUnlock (hSaveMacro);
    	}
    	LogicP (SQLPtr->SQL,&Err); 
    	AllowFltExpand = TRUE;
    	phStatement = (LPHANDLE)GlobalLock (hLogicPStatements); 
    	while (nLogicPStatements>0)
    	{   
    		nLogicPStatements--;
    		pStatement = (LPLOGICPSTATEMENT)GlobalLock (*phStatement); 
	    	{   
//	    		lpGWDHead =(LPGWDHEADER) GlobalLock (FilePtr->FileHandle);
	
//	        	for (i=0,lpGWFldInfo=lpGWDHead->pFldInfo;i<lpGWDHead->NumFields;i++,lpGWFldInfo++)
				if (pStatement->OpCode == '||')
					HaveOR = TRUE;
				else
					for (i=0,pFieldInfo=&FilePtr->FldInfo;i<FilePtr->NumFields;i++,pFieldInfo++)
	        	{	
					Strip (pStatement->Arg1,'"');
//	        		if (!_fstricmp (lpGWFldInfo->Name,pStatement->Arg1))
	        		if (!_fstricmp (pFieldInfo->name,pStatement->Arg1))
	        		{ 
	        			SQLField->FieldNum = i; 
				    	SQLField->hGlobal = 0; 
						if (HaveOR)
		    	   			SQLField->OpCode = -1;
						else
		    	   			SQLField->OpCode = pStatement->OpCode;
		    	   		if (hSaveMacro)
		    	   		{
		    	   			pSaveMacro = GlobalLock (hSaveMacro);
		    	   			ExpandText (pSaveMacro);
				    		_fstrcpy (SQLField->String,pSaveMacro);
		    	   			GSSiGlobUlFree (&hSaveMacro);
		    	   		}
		    	   		else
				    		_fstrcpy (SQLField->String,pStatement->Arg2);
				    	NumFieldInSQL++;
				    	SQLField++; 
	        			break;
	        		}
	        	}
//	        	GlobalUnlock (FilePtr->FileHandle);
	    	}
    		GSSiGlobUlFree (phStatement++);
    	} 
    	GSSiGlobFree (&hSaveMacro);
    	GSSiGlobUlFree (&hLogicPStatements);
	} 
	GlobalUnlock(hFields);
    SQLPtr->st = 2;
    if (NumFieldInSQL)// && FilePtr->Type != GMTEXT_DATAFILE)
    	SQLPtr->NumGlobals = NumFieldInSQL; 
    else
    	SQLPtr->NumGlobals = -1; 
    SQLField = (LPSQLFIELD)GlobalLock (hFields);
    lpSQLField = &SQLPtr->SQLField;
    for (i=0;i<NumFieldInSQL;i++,SQLField++,lpSQLField++)
    	*lpSQLField = *SQLField;  
	if (FilePtr->Type == UMIFS_DATAFILE || FilePtr->Type == ORA_DATAFILE || FilePtr->Type == GMCENSUS_DATAFILE)
	    SQLPtr->IndexToUse =  SelectGWDIndex (FilePtr->FileHandle,SQLPtr,&SQLPtr->IndexToUse);
	else if (FilePtr->Type == HLTLIST_DATAFILE || FilePtr->Type == THEME_HLTFILE)
		SQLPtr->IndexToUse = BT_FIRST;
    GSSiGlobUlFree (&hFields);   
    GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (521);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

int	SelectGWDIndex (HANDLE hFile,LPOPENSQLDATA	SQLPtr,LPSHORT pUnique) 
#if ENABLETRACE
{GSSiEnterProg (522);
#endif
{
	LPGWDHEADER	lpGWDHead;
	LPSQLFIELD	lpSQLField;
	short	index=-1, i,j,k, nif=0, maxf=-1;
	
	if (SQLPtr->NumGlobals == -1)
{
#if ENABLETRACE
GSSiExitProg (522);
#endif
		return 0;
}
	lpGWDHead =(LPGWDHEADER) GlobalLock (hFile);
	for (i=0;i<lpGWDHead->NumIndex;i++)
	{	
		nif=0;
		if (lpGWDHead->NumIndexFields[i] < 0)
			continue;
		if (i && lpGWDHead->SpatialIndex == i)
			continue;
		for (k=0;k<abs(lpGWDHead->NumIndexFields[i]);k++)
		{ 
			for (j=0,lpSQLField=&SQLPtr->SQLField;j<SQLPtr->NumGlobals;j++,lpSQLField++)
			{ 
				if (lpGWDHead->IndexFields[i][k] == lpSQLField->FieldNum &&
				(lpSQLField->OpCode == OPCODE_EQ || lpSQLField->OpCode == OPCODE_GE || lpSQLField->OpCode == OPCODE_GT))
					goto NextField;
			} 
			goto NextIndex;
NextField:
			nif++;  
			if (lpSQLField->OpCode == OPCODE_EQ)
				nif++;
		}
NextIndex:
		if (nif > maxf || (nif == maxf && nif == lpGWDHead->NumIndexFields[i])) 
		{
			index = i; 
			if (lpGWDHead->lKeys[index] < 0)
				*pUnique = 0;
			else
				*pUnique = 1;
			maxf = nif;
		}
	} 
	GlobalUnlock (hFile);
{
#if ENABLETRACE
GSSiExitProg (522);
#endif
	return index;
}
#if ENABLETRACE
}
#endif
}


HANDLE GetDBHandleFromSQL (HANDLE hSQL)
#if ENABLETRACE
{GSSiEnterProg (523);
#endif
{   HANDLE	DBHandle;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;

	SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
	DBHandle = FilePtr->FileHandle;
	GlobalUnlock (SQLPtr->OFHandle);
	GlobalUnlock (hSQL);  
{
#if ENABLETRACE
GSSiExitProg (523);
#endif
	return DBHandle;
}
#if ENABLETRACE
}
#endif
}

HANDLE GetOpenDatabaseFromID (LPSTR id)
{
	HANDLE hDB = 0;

    if (FilePathHandle)
	{
		LPFILEPATH FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle); 
		LPHANDLE lpFileHandle = &FilePathPtr->FileHandle;
 		int i;
   
		for (i=0;i<FilePathPtr->NumFiles;i++,lpFileHandle++)
		{
			if (*lpFileHandle)
			{
				LPOPENSQLDATA SQLPtr = (LPOPENSQLDATA)GlobalLock (*lpFileHandle);
				if (!_fstricmp (SQLPtr->IDName,id))
				{
					hDB = *lpFileHandle;
					GlobalUnlock (*lpFileHandle);
					break;
				}
				GlobalUnlock (*lpFileHandle);
			} 
		}
		GlobalUnlock (FilePathHandle);
	}
	return hDB;
}

BOOL CloseDataFile (BOOL Final, HANDLE *hDB)
#if ENABLETRACE
{GSSiEnterProg (524);
#endif
{   
	HANDLE	handle; 
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	int	i,j, NewNum,ii; 
	
	if (!hDB || !*hDB)
{
#if ENABLETRACE
GSSiExitProg (524);
#endif
		return(TRUE);
}
	
	SQLPtr = (LPOPENSQLDATA)GlobalLock (*hDB); 
	if (!SQLPtr) 
	{
		*hDB = 0;
{
#if ENABLETRACE
GSSiExitProg (524);
#endif
		return TRUE;
}
	}
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	GSSiGlobFree (&FilePtr->BufferHandle); 
	NewNum = FilePtr->NumSQLs;
	for (i=j=0;i<FilePtr->NumSQLs;i++)
	{   
		FilePtr->SQLHandles[j] = FilePtr->SQLHandles[i];
		if (FilePtr->SQLHandles[i] != *hDB)
			j++; 
		else
			NewNum--;
	}
	FilePtr->NumSQLs = NewNum;
	if (FilePtr->NumSQLs)
	{
		GlobalUnlock (SQLPtr->OFHandle); 
		goto DeallocSQL;
	}
	
	switch (FilePtr->Type)
	{
		case THEME_HLTFILE:
			ii=1;
		break;

		case HLTLIST_DATAFILE:
			BT_CLOSE (FilePtr->FileHandle);
			FilePtr->FileHandle = 0;
		break;

		case GMTEXT_DATAFILE:
			GSSiClose (FilePtr->Fid);  
			GSSiGlobFree (&FilePtr->FileHandle);
			break; 

		case LISTVAR_DATAFILE:
			GSSiGlobFree (&FilePtr->FileHandle);
			break; 

	    case DBF_DATAFILE:
		case SHAPE_DATAFILE:
		{
			DBFHandle    pDBF;
			LPDWORD 	 pDBFAddress = (LPDWORD)GlobalLock (FilePtr->FileHandle);
			
			pDBF = (DBFHandle)*pDBFAddress; 
		    DBFClose(pDBF);
			GSSiGlobUlFree (&FilePtr->FileHandle);   
		}
			break;
		case GMCENSUS_DATAFILE:	
		{
			LPGWDHEADER	lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 
			
			BT_CLOSE (lpGWDHead->BTHandle[lpGWDHead->NumIndex]);
			GlobalUnlock (FilePtr->FileHandle);   
		}
	    case PN_DATAFILE:
		case UMIFS_DATAFILE:
		case ORA_DATAFILE:
			CloseGWDatabase (FilePtr->FileHandle);
 			break;

		case DTM_DATAFILE:
			DTMClose (&FilePtr->FileHandle);
 			break;

		case IMAGE_DATAFILE:
			DestroyDIB32 (FilePtr->FileHandle,FALSE);
			break;

		case COMBO_DATAFILE:
			CloseComboDatabase (FilePtr->FileHandle);
 			break;

        case SQL_DATAFILE:   
        {
			LPSQLDATABASE	pDB=(LPSQLDATABASE)GlobalLock(FilePtr->FileHandle);  
	        
	        CloseDataFile (Final,&pDB->DBHandle);
	        GSSiGlobUlFree (&FilePtr->FileHandle);
	    }
            break;
		case FOXPRO_DATAFILE:
	       	if (Final)
	       	{
	          CloseExternalDatabase ((int)FilePtr->FileHandle);
	       	}
              break; 
        case FGDB_DATAFILE:
		{
			ClearCurVals (FilePtr);
			CloseFGDB ((int)FilePtr->FileHandle);
		}
		break;
        case PGDB_DATAFILE:
	    case ODBC_DATAFILE:  
	    //case DBF_DATAFILE:
	    case TEXT_DATAFILE:
	    case MSACCESS_DATAFILE:
	        {
			  ClearCurVals (FilePtr);
			  if (SQLPtr->hstmt)
			  {
				SQLCloseCursor(SQLPtr->hstmt);
				SQLFreeStmt(SQLPtr->hstmt, SQL_DROP);
				SQLPtr->hstmt = NULL;
			  } 
              CloseExternalDatabase ((int)FilePtr->FileHandle);
              break;
	    	} 

    }
    handle = FilePtr->myhandle; 
    FilePtr->myhandle = 0;
    GSSiGlobUlFree (&handle);
DeallocSQL:
    RemoveFromOpenFileList(*hDB); 
	GSSiGlobUlFree (hDB);
{
#if ENABLETRACE
GSSiExitProg (524);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
}  

void SetUDIValue (LPSTR Name, LPSTR Value)
#if ENABLETRACE
{GSSiEnterProg (525);
#endif
{   
	int		i,j;
	LPTAGDEF	pTAGDef;
	
	SetGlobalValue (Name,Value); 
	LoadTAGDef();
	if (NumTAGDef>0)
	{
		pTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
		for (i=0;i<NumTAGDef;i++,pTAGDef++)
		{ 
			if (!_fstrcmp(pTAGDef->Prefix,Name))
			{   
				for (j=0;j<pTAGDef->NumVar;j++)
				{
					SetGlobalValueLen (pTAGDef->VarName[j],&Value[pTAGDef->VarStart[j]],pTAGDef->VarLen[j]);
				}
				goto GotTAG;
			}
		}
	GotTAG:
		GlobalUnlock (hTAGDef);
	}
	SetGlobalValue2 (hUDI,Value,0);
		
{
#if ENABLETRACE
GSSiExitProg (525);
#endif
	return;  
}
#if ENABLETRACE
}
#endif
}  

BOOL LoadTAGDef (void)
#if ENABLETRACE
{GSSiEnterProg (526);
#endif
{   
	HFILE	TDFid; 
	LPSTR	lpSpace;
	OFSTRUCTGM	OFStruct;
	char	TDFile[128], Mess[256];
	int		i,j, lineno=0;
	LPTAGDEF	pTAGDef;
	
	
	if (NumTAGDef<0)
	{   
		char	line[260];
			
		_fstrcpy (TDFile,"[%DL]tagdef.txt");
		TDFid = GSSiOpenFile(TDFile,&OFStruct,OF_READ);
		NumTAGDef=0;
		if (TDFid != HFILE_ERROR)
		{   
		    GSSiGlobFree (&hTAGDef);  
			hTAGDef=GSSiGlobAlloc ( 189,GHND,USHRT_MAX);
			pTAGDef = 0; 
	NextLine: 
			lineno++;
			if (!fgetstring (line,256,TDFid))
				goto EndFile;  
			if (*line=='#' || !*line); 
			else if (line[0]!=' ') 
			{   
				NumTAGDef++;
				if (pTAGDef)
					pTAGDef++;
				else
					pTAGDef = (LPTAGDEF)GlobalLock (hTAGDef); 
				if (sscanf (line,"%s %i %i %i",
					&pTAGDef->Prefix,&pTAGDef->Len,&pTAGDef->IncBeg,&pTAGDef->IncLen) != 4)
				{
					sprintf (Mess,"Invalid line number %i in TAG definition file\r\n%s",lineno,line);
					GSSiMsgBox (GetFocus(),Mess,0,MB_ICONEXCLAMATION,0);
				}
			}
			else
			{   
				lpSpace = _fstrchr (&line[1],' ');
				if (sscanf (&line[1],"%s %i %i %i",
					&pTAGDef->VarName[pTAGDef->NumVar],&pTAGDef->VarStart[pTAGDef->NumVar],
					&pTAGDef->VarLen[pTAGDef->NumVar]) != 3)
				{
					sprintf (Mess,"Invalid line number %i in TAG definition file\r\n%s",lineno,line);
					GSSiMsgBox (GetFocus(),Mess,0,MB_ICONEXCLAMATION,0);
				}
				pTAGDef->VarStart[pTAGDef->NumVar]-=1;
				pTAGDef->NumVar++;
			}  
			goto NextLine;
	EndFile: 
			if (hTAGDef) 
			{
				GlobalUnlock (hTAGDef); 
				hTAGDef = GSSiGlobalReAlloc (0,hTAGDef,(long)NumTAGDef*sizeof(TAGDEF),GMEM_MOVEABLE);
			}
			GSSiClose (TDFid);
		}  
	}
{
#if ENABLETRACE
GSSiExitProg (526);
#endif
	return TRUE; 
}
#if ENABLETRACE
}
#endif
}


void SetUDIValueLen (LPSTR Name, LPSTR Value, short len)
#if ENABLETRACE
{GSSiEnterProg (527);
#endif
{
	char	Value2[66];
	
	strncpy0(Value2,Value,min(65,len));
	SetUDIValue (Name,Value2);
{
#if ENABLETRACE
GSSiExitProg (527);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void SetGlobalValue (LPSTR InName, LPSTR InValue) 
{
	if (*InName)
		SetGlobalValue4 (InName,InValue,FALSE,0,0,0);
	return;
}

void SetGlobalValue4(LPSTR InName, LPSTR InValue, BOOL Raw, LPBREAKPOINT pBrkPt, int bpOffset,int bpLen)
#if ENABLETRACE
{GSSiEnterProg (528);
#endif
{   
	VARPNT	VP;
	HANDLE	handle, hVal; 
	BOOL	BVal, FoundLit;
	LPSTR	lpPar, lpEndPar=0, Value, Name;
	short	Index=0; 
	short	n,ii;
	int		lnValue=strlen (InValue);
    
    if (!ContinueProcessing)
{
#if ENABLETRACE
GSSiExitProg (528);
#endif
    	return; 
} 
	lnValue = max (lnValue+1,4096*4);
    hVal = GSSiGlobAlloc ( 190,GMEM_MOVEABLE,lnValue+256);
    Value = GlobalLock (hVal); 
    Name = Value + lnValue;
   	_fstrcpy (Value,InValue);
	if (!Raw)
	{
		if (*Value == '\'')
		{
			int l = strlen (Value);
			if (l > 1 && Value[l-1] == '\'')
			{
				Value[l-1] = 0;
				Value++;
			}
		}
		if (pBrkPt)
			ExpandTextDB(Value, pBrkPt, bpOffset, bpLen);
		else
			ExpandText(Value);
	}
	FoundLit = FoundLiteral;
	if (*InName == '.')	                  
		_fstrcpy (Name,(InName+1)); 
	else
		_fstrcpy (Name,InName);
	ExpandText (Name);
	if ((lpPar=_fstrchr (Name,'.')))
	{   
		HANDLE	hSQLPtr; 

		*lpPar++ = 0;
			
       	hSQLPtr = GetDBByIDName (Name);
       	if (!hSQLPtr)
       		goto Rtn; 
	    SetGMDField (hSQLPtr,lpPar,Value);
	    goto Rtn;
    }
	lpPar = _fstrrchr(Name,'(');
	if (lpPar)
	{
		lpEndPar = _fstrchr(lpPar,')');
		if (lpEndPar)
		{   
			*lpPar = 0;
			if ((handle = FindVar (Name)))
			{
				VP = (VARPNT)GlobalLock (handle);
				if (!VP->Type)
					*lpPar = '(';
				GlobalUnlock (handle);
			}
			else
				*lpPar = '('; 
			lpPar++;
			Index = atoi(lpPar);
		}
	}
	SetGlobalValue3 (Name,Value,Index,FoundLit);
Rtn:    
	GSSiGlobUlFree (&hVal);
{
#if ENABLETRACE
GSSiExitProg (528);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}          
void SetGlobalValue3 (LPSTR Name, LPSTR Value, short Index, BOOL FoundLit)
#if ENABLETRACE
{GSSiEnterProg (529);
#endif
{
	VARPNT	VP;
	HANDLE	handle; 
	short	ii; 
	
	if (!*Name || _fstrlen (Name) > 61)
{
#if ENABLETRACE
GSSiExitProg (529);
#endif
		return;
}   
//if (!_fstricmp (Name,"%NODENAME"))
//ii=1;
	if (*TraceVar)
	{
		if (!_fstricmp (Name,TraceVar))
			MessageBox (0,Value,Name,MB_OK);
	}
	handle = AllocateVar (Name); 
	VP = (VARPNT)GlobalLock (handle);
	VP->ContainsGorF = FoundLit; 
	GlobalUnlock (handle);  
	SetGlobalValue2 (handle,Value,Index);
	if (FoundLit)
		LinkIncludedVars (handle,Value);
{
#if ENABLETRACE
GSSiExitProg (529);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void SetLinkedVarTime (VARPNT VP)
#if ENABLETRACE
{GSSiEnterProg (530);
#endif
{   
	VARPNT	VPLinked;  
	UINT 	i;
	
	for (i=0;i<VP->NumLinkedVars;i++)
	{
		VPLinked = (VARPNT)GlobalLock (VP->LinkedVar[i]);
		if (!VPLinked)
		{ 
			VP->NumLinkedVars = 0;
{
#if ENABLETRACE
GSSiExitProg (530);
#endif
			return;
}
		}
		VPLinked->changetime = VP->changetime;
		GlobalUnlock (VP->LinkedVar[i]);	
	}
{
#if ENABLETRACE
GSSiExitProg (530);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void LinkIncludedVars (HANDLE hVar,LPSTR Value)
#if ENABLETRACE
{GSSiEnterProg (531);
#endif
{   
	HANDLE	hMem = GSSiGlobAlloc ( 191,GMEM_MOVEABLE,4096);
	LPSTR	pMem = GlobalLock (hMem);
	
	_fstrcpy (pMem,Value);
	LinkToVar = hVar;
	ExpandText (pMem);
	GSSiGlobUlFree (&hMem);
	LinkToVar = 0;
{
#if ENABLETRACE
GSSiExitProg (531);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void SetGlobalValue2 (HANDLE handle, LPSTR Value, short Index)
#if ENABLETRACE
{GSSiEnterProg (532);
#endif
{
	VARPNT	VP;
	BOOL	BVal; 
	short	n, type, i,ii, ib, ie;    
	long	Color;
	char	nullval=0; 
	LPSTR	pCmd;
	RECT	Rect;
    
    if (!handle)
{
#if ENABLETRACE
GSSiExitProg (532);
#endif
    	return; 
}
    if (!Value)
    	Value = &nullval;
	VP = (VARPNT)GlobalLock (handle);
	type = VP->Type; 
//	if (type == 99 && AltProjLocked)
//		ii=1;
//	else 
	{   
		if (VP->ValueIsHandle)
		{
			HANDLE handle2 = (HANDLE)atol (VP->Value);
			GSSiGlobFree (&handle2); 
			VP->ValueIsHandle = 0;
		}
	
		VP->Len = min (MAXVARLEN,_fstrlen(Value));
/*		if (type == 102) //%DL
			_fullpath (VP->Value,Value,MAXVARLEN); 
		else */
		{   
			if (VP->Len == MAXVARLEN)
				Value[MAXVARLEN] = 0;
			_fstrcpy (VP->Value,Value);
		}
		VP->changetime = NextVarTime();
		SetLinkedVarTime (VP);  
		AddToChangedGlobalList (handle);
		if (VP->Type == 99)
			ConvertCoordClose (); 
	} 
	GlobalUnlock (handle);  
	if (!type)
{
#if ENABLETRACE
GSSiExitProg (532);
#endif
		return;
}
	switch (type)
	{
		case 2:
			if (CurView)
			{
				if (atob (Value))
					CurView->FileProjectionType = 1;
				else
					CurView->FileProjectionType = 0;
			}
			break;  
		case 3:
			CurState = atoi (Value); 
			if (CurState > 0 && CurState < 74)
				HaveStates[CurState]=TRUE;
			break;
		case 4:
			ShowScale = atob (Value);
			break;     
		case 5:
			if (!CurVis)
				break;
			if (!_fstrcspn (Value," 1TtYy"))
				BVal = TRUE;
			else
				BVal = 2;  
			if (CurVis->FileIsVisible[Index])
				CurVis->FileIsVisible[Index]=BVal;
			break;     
		case 6:
			DoGraphics = atob (Value);
			break;     
		case 7: 
			SetTrace(atoi(Value));
			break;     
		case 8:
			FillAreas = atob (Value);
			break; 
		case 9:  
			InternalRefno = atol (Value);
			break; 
		case 10:
			MaxPick = atoi (Value);
			MaxPick = min (MaxPick,MAXPICKITEMS-2);
			MaxPick = max (1,MaxPick);
			break;
		case 11:
			if (Index >=0 && Index < 8)
				PenCOLOR[Index] = atol (Value);
			break;
		case 12:
			if (Index >=0 && Index < 8)
				PenWIDTH[Index] = atoi (Value);
			break;   
		case 13:
			if (Index >=0 && Index < 10)
				GlobalColors[Index]=atol (Value);
			break;
		case 14:
			TextFreq = atol (Value);
			break;
		case 15:
			CurFont = atol (Value);
			break;
			   
		case 16:
			//ExpandText (Value);
			if (!MapServer)
			{
				if (CurView)
				{
					if (CurView->hWnd)
						SetWindowText(CurView->hWnd, Value);
					break;
				}
				if (hWndMain)
					SetWindowText(hWndMain, Value);
			}
			break;
			   
		case 19:
			DisplayMarkers = atob (Value);
			break;   
			
		case 20:
			DisplaySymbol = atob (Value);
			break;   
			
		case 35:  
			MaxDisplayPoints = IDNINT (atof (Value));
			break;
			
		case 37:
			SetRefno = atob (Value);
			break;   
			
		case 38:
			if (Index <1 || Index >3) break;
			DisplayCoord[Index-1] = atob (Value);
			break;
			   
		case 39:
			ByState = atob (Value);
			break; 
			
		case 44:  
			UseBinFileList = atob (Value);
			break;
			 
		case 45:
			if (Index >=0 && Index < 8)
				AreaCOLOR[Index] = atol (Value);  
				
		case 46:
			PickLimit = atof (Value);
			break;
				
		case 47:  
			ProcessGCmdStrings = atoi (Value);
			break;
			 
		case 48:
			HighlightColor = atol (Value);
			break;
				
		case 49:  
			Display16BitColor = atob (Value);
			break;
			 
		case 50:  
			FastOrthos = atob (Value);
			break;
			
		case 51:  
			ShowFileBounds = atob (Value);
			break;
			 
		case 52:  
			CursorIsLocked = atob (Value);
			break;
			 
		case 53:
			UserPickAP = atof (Value);  
			if (UserPickAP == 0)
				CurPickCursor = hPickNearCursor;
			else if (UserPickAP > 0)
				CurPickCursor = hPickAPCursor;   
			else
				CurPickCursor = hPickAPCursor;
			break;
				
		case 54:
			AddressTextSize = atof (Value);
			break;
				
		case 55:
			StreetTextSize = atof (Value);
			break;

		case 56:  
			WantStreets = atob (Value);
			break; 
			
		case 57:  
			OrthoAdjustX = atof (Value);
			break;
			
		case 58:  
			OrthoAdjustY = atof (Value);
			break;
			
		case 59:
			for (i = 6;i;i--)
				if (!_fstricmp (Value,AreaUnitOpts[i-1]))
				{
					OutAreaUnits = i;
					SetGlobalValueLong ("%AREAUNITS",OutAreaUnits);
					break; 
				} 
			
			break;
		case 62: 
			n = _fstrlen (Value);
			if (n)
			for (i = 6;i;i--)
				if (!_fstrnicmp (Value,DistUnitOpts[i-1],n))
				{
					OutDistUnits = i;
					SetGlobalValueLong ("%DISTANCEUNITS",OutDistUnits); 
					break;           
				}
			break;
		case 63:
			MinSpeed = atof (Value);
			break;

		case 64:
			MaxSpeed = atof (Value);
			break;

		case 65:
			if (!atob (Value))
			{   
				ProcessErrorString ();
				ContinueProcessing = FALSE; 
			}
					    
   			break;
        
		case 78:
			
			Index = min (Index,32);
			n = sscanf (Value,"%Flf %Flf",&UserPoints[Index].x,&UserPoints[Index].y);  
			break;
			
		case 79: /*%BOUNDS*/ 
			switch (Index)
			{
				default:
				case 1:
				case 2:
					Index = max (1,min (Index,2));
					sscanf (Value,"%Flf %Flf",&UserBounds[Index-1].x,&UserBounds[Index-1].y);
				break;
				case 0:
					sscanf (Value,"%Flf %Flf %Flf %Flf",&UserBounds[0].x,&UserBounds[0].y,&UserBounds[1].x,&UserBounds[1].y);
				break;
			}
		break;
		case 80: /*%VPBOUNDS*/ 
			switch (Index)
			{   
				default:
				case 0:
					sscanf (Value,"%Flf %Flf %Flf %Flf",&CurView->WBounds.xmn, 
												  		&CurView->WBounds.ymn,
												  		&CurView->WBounds.xmx,
												  		&CurView->WBounds.ymx);  
				break;
				case 1:
					sscanf (Value,"%Flf %Flf",&CurView->WBounds.xmn,&CurView->WBounds.ymn);
				break;
				case 2:
					sscanf (Value,"%Flf %Flf",&CurView->WBounds.xmx,&CurView->WBounds.ymx);
				break; 
			}
		CurView->HaveBounds=TRUE;
		CurView->NewBounds = CurView->WBounds;
		break;
		
		case 81:  
			RedisplayOnly = atob (Value);
			break;
			
		case 84:
			BaseUnitsPerOrthoPixel=atof (Value);
			if (!BaseUnitsPerOrthoPixel)
				BaseUnitsPerOrthoPixel=1;
			GetViewportScale(CurView->hDC);

			break;
			
		case 85:
			StretchMode = atoi (Value);
			if (StretchMode < 1 || StretchMode > 4)
				StretchMode = STRETCH_DELETESCANS;
			break;
			
		case 86:
			DoTime = atob (Value);
			break;   
			
		case 89:
			TrackVideo = atob (Value);
			break;
        
        case 90:
        	Index = max (0,min(Index,MAXFONTS-1));
        	_fstrcpy (&FontNames[Index][0],Value); 
        	if (hSizingFont[Index])
        	{
        		DeleteObject (hSizingFont[Index]);
        		hSizingFont[Index]=0; 
        	}
        	break;
        	
		case 91:  
			LayerID = atol (Value);
			break;
			
		case 92:
			ConvertMapVP = -1;
			for (i=0;i<CurView->NumFiles;i++)
			{  
				if (!Expandicmp (Value,CurView->lpFiles[i]))
				{
					ConvertMapFileNo = i;
					ConvertMapVP = CurView->ID;
					break;
				}
			} 
			break;
			
		case 93:
			MaxPointSize = atof (Value);
			break;

		case 94:
			DisplayHLTPattern = atob (Value);
			break;
        
		case 95:
			DisplayLinkedCursors = atob (Value);
			break;
		case 96:
			TrackColor=atol (Value);
			break;
        
        case 97:
        	_fstrcpy (ImageExtension,Value);
        	_fstrlwr (ImageExtension);
        	break;
        	
		case 98:
			PickAllPieces = atob (Value);
			break;

		case 100:
			InclusionOpt=atol (Value);
			break;
        
		case 101:
			TrackWidth=atol (Value);
			break;

		case 102:
			HighlightWidth = atol (Value);
			break;
		
		case 103:
			AutoClearOffset = atob (Value);
			break;
		
		case 104:
			MaskOffsetLine = atob (Value);
			break;
		
		case 105:  
			GraphicsTextFactor = atof (Value);
			break;
		
		case 110:
			UseItemLen = atob (Value);	
			break;
			
		case 111:
			WantFunNames = atob (Value);	
			break;
			
		case 112:
			ProcessAllElements = atob (Value);	
			break;
		
		case 114:
			ShowInvisiblePointsSymbol = atoi (Value);
			break;	

		case 115:
			MaxTextLayer = atoi (Value);
			break;	

		case 116:
			CurrentAZ = LTWOPI(atof (Value));
			CreateDigCursor (CurView->hDC);
			break;	
        
        case 118:
			if (Index >=1 && Index < 5) 
        		TraceType[Index-1]=atob(Value);  
        	else
        		EnableTrace=atoi(Value);
        	break;	
        case 119:
        	WindowColor = atol (Value);
        	break;
            
        case 122:
        	ShowLinkLines = atob (Value);	
        	break;
        case 123:
        	if ((UseRefOrTAGIndex = atob (Value)))
	        	IgnorePrevLayers = FALSE;
	        else 	
	        	IgnorePrevLayers = TRUE;
        	break;
        
        case 124:
        	if (CurTheme)
        	{   
        		Color = atol (Value);
        		if (!Index)
        		{
        			ib=0;
        			ie=MAX_THEME_CLASSES;
        		}
        		else if (Index > 0 && Index <= MAX_THEME_CLASSES)
        		{
        			ib = Index-1;
        			ie = Index;
        		}
        		else
        			break;
        		for (i=ib;i<ie;i++)
        			CurTheme->ClassColor[i] = Color;
        	}
        	break;
        
        case 125:
        {		
			LPCMDSTRING    pCmdStr;   
	
			if (CurView)
			{   
				if (CurView->FunStackHandle)
				{
					pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle); 
					if (*Value)
					{
						pCmdStr->Prompt = PRMT_USERPROMPT;
						_fstrcpy (pCmdStr->PromptText,Value);
					}
					else
						pCmdStr->Prompt = 0;	
					GlobalUnlock (CurView->FunStackHandle);
					SetPrompt (PRMT_USERPROMPT,FALSE);
				} 
			}
		}
		break;
		
        case 126:
        	PickPerim = atoi (Value);	
        	break;
        
        case 127:
        	CurveChordDist = -atof (Value);
        	break;	  
        	
		case 128:
			AutoPan = atob (Value);
			break;
		
		case 129:
			GetSegFromGlobals = atob (Value);
			break;
		case 130:
			if(atob (Value))
				InvertNetDir=-1;
			else
				InvertNetDir=1;
			break;
		
		case 131:  
			GraphicsPointFactor = atof (Value);
			break;
		
		case 132:
			MaskOffsetLine = atob (Value);
			break;
		
		case 133:
			OutlineZoomArea = atob (Value);
			break;
		
		case 134:
			BleedThrough = atob (Value);
			break;
		
		case 136:
			LocationOffset = atof (Value);
			break;
		
		case 138:
			TimeRangeBeg = atol (Value); 
			MonthBeg = SysMonthFromSymTime (TimeRangeBeg);
			break;
		
		case 139:
			TimeRangeEnd = atol (Value); 
			MonthEnd = SysMonthFromSymTime (TimeRangeEnd);
            break;
        
        case 140:
        	if (atob (Value))
        		CurView->FileProjectionType = 2;
        	else
        		CurView->FileProjectionType = 0;
        	break;
        	
        case 142:
        	if (atob (Value))
        		OpenDisplayedRefs();
        	else
        		CloseDisplayedRefs();
        	break;
        	
        case 143:
       		AlwaysUseSymDict = atob (Value);
        	break;
        case 144:
        	LoadCompiledTran = atob (Value);
        	break;
        	
		case 148:  
			LineSymbolFactor = atof (Value);
			break;

        case 151:
       		GetCopySize = atob (Value);
        	break;
		
        case 152:
       		DisplayFiles = atoi (Value);
        	break;   
        case 153:
        	DisplayAllRefs = atob(Value);
        	break;
		case 154:  
			InvisPointFactor = atof (Value);
			break;
		case 155:  
			ShowValSize = atof (Value);
			break;
		case 156: 
			if (_fstrchr(Value,':')) 
				_fstrcpy (NewTAG,Value);
			break;
        case 157:
        	WantOrthoResDisplay = atoi (Value);
        	break;
        case 158:
        	ComputeArea = atob (Value);
        	break;
        case 159:
        	ShowBadSyms = atob (Value);
        	break;
        case 160:
        	PickDeletes = atob (Value);
        	break;
        case 162:
        	StanSQL = atob (Value);
        	break;
        case 163:
        	MinCacheDriveFreeSpace = atol (Value) * 1024 * 1024;
        	break;
        case 165:
        	ShortestRoute = atob (Value);
        	break;
		case 166:
			SnapPolygons = atob (Value);
			break;
        case 168:
       		AutoType = atob (Value);
        	break;
		case 169:
			ShowRefno = atob (Value);
			break;
        case 170:
        	Index = max (0,min(Index,15));
        	FontWidthFactor[Index] = atof(Value); 
        	break;
		case 171:
			GraphicsTextWeight = max (0,min(4,atol (Value)));
			break;
		case 172:  
			PenWidthFactor = atof (Value);
			break;
		case 173:  
			VisListOpt = atoi (Value);
			break;
		case 174:	
	    	AllowPickDeletes = atob (Value);
	    	break;
        case 175:  
        	if (!CurView)
        		break;
			n = atol(Value);
			if (!n)
			for (i = 6;i;i--)
				if (!_fstrnicmp (Value,DistUnitOpts[i-1],_fstrlen(Value)))
				{
					n = i;
					break;           
				}
        	if (CurView->pTheme && CurView->pTheme->ID == GF_DISTANCE_THEME)
        			CurView->pTheme->ValConv = n;
        	else
        	{
        		for (i=0;i<CurView->NumThemes;i++)
        		{
        			if (CurView->pThemes[i]->ID  == GF_DISTANCE_THEME) 
        			{
        				CurView->pThemes[i]->ValConv = n;
						break;
					}
        		}
        	}
        	break;
        
        case 176: 
        	CloseSymDict();
        	break;
        	
        case 177:
			if (MaxFileSizeToCache > 0)
				MaxFileSizeToCache = atol (Value) * 1024 * 1024;
        	break;  
        	
        case 178:
        	ShowValColor = atol (Value);
        	break;  
        	
        case 179:
        	_fstrcpy (NoCache,Value);
        	break;
        
        case 180:
        	DefaultTextColor = atol (Value);
        	break; 
        	
        case 181:
        	CurView->Height = atol (Value);
        	break; 
        	
        case 182:
        	UseCacheAlreadyChecked = atob (Value);
        	break; 
        	
        case 183:
        	CacheAll = atob (Value);
        	break; 
        	
        case 184:
        	_fstrcpy (UserName,Value);
        	break;
        
        case 185:
        	_fstrcpy (NodeName,Value);
        	break;
        
        case 191:
        	CheckTimeStamps = atob (Value);
        	break; 
        	
        case 192:
        	if (!atob (Value))
        	{
        		CloseAllRequestedFiles (FALSE);
        		KeepFilesOpen = FALSE;
        	}
        	else
        		KeepFilesOpen = TRUE;
        	break; 
        
        case 193:
        	CheckForValidPath = atob (Value);
        	break;
        
        case 194:
        	CanDisplayZeroLenghtLines = atob (Value);
        	break;
        			
        case 195: 
        {
        	BOOL	Test = atob (Value);
        	
			break;
        	if (Test != UndoEnabled)
        	{
        		if (!Test)  
        		{
        			SaveDataToUndoFile (0,0,0,0,0);
        			UndoEnabled = FALSE; 
        		}
        		else
        		{
        			UndoEnabled = TRUE; 
        			CreateUndoPoint ("Start");
        		}
        	}
        }
        	break;
        
        case 196:
        	DisplayRasterOpt = atoi(Value);
			if (DisplayRasterOpt < 1 || DisplayRasterOpt > 16)
				DisplayRasterOpt = R2_COPYPEN;
        	break;

        case 197:
        	ShowDeletedOpt = atob (Value);
        	break;
        
        case 198: 
        {
			LPSTR CmdMess;  
			
			if (hCmdMess)
			{
				CmdMess = GlobalLock (hCmdMess);
				_fstrcpy (CmdMess,Value);
	       		GlobalUnlock (hCmdMess);
	       	}
	    }
	    	break; 
        case 199:
        	HaveBlockingWindow = atob (Value);
        	break;
        	
		case 200:
			StoreDynText = atob (Value); 
			break;
			
		case 202:
			{
				LPSTR	pSet = Value;
				LPBYTE	pEnd, pSep, pFrom=ConvertTextFrom, pTo=ConvertTextTo;
				
				while (pSet)
				{
					pEnd = _fstrchr (pSet,',');
					if (pEnd)
						*pEnd++ = 0;
					pSep = _fstrchr (pSet,':');
					if (!pSep)
						break; 
					*pSep++ = 0;
					*pFrom++ = atoi (pSet);
					*pTo++ = atoi (pSep);
					pSet = pEnd;
				} 
				*pFrom = 0;
				*pTo = 0;
			}
			break;

		case 203:
			NumOffsetFailed = atol (Value); 
			break;
			
		case 207:
			MSLink = atol (Value); 
			break;
			
        case 209:
        	Index = max (0,min(Index,MAXFONTS-1));
        	FontColors[Index] = atol (Value); 
			break;

		case 210:
			UseDGNColors = atob (Value); 
			break;
			
		case 211:
			ShowNodesRef = atol (Value); 
			break;

        case 212: HighlightMultRefs = atol (Value);
        	break;
        	
        case 213: ShowSQLErrors = atoi (Value);
        	break;
        	
        case 216: TraceRef = atob (Value);
        	break;
        	
        case 217: LogUsage = atob (Value);
        	break;

        case 218: PickPoints = atob (Value);
        	break;
        case 220:
			if (!MapServer)
			{
				AllowCache = atob(Value);
				if (!AllowCache)
				{
					AddBMPToCache32(0, 0);
					CacheAlreadyChecked(0, 0, 0);
				}
				if (AllowJournal)
					AllowCache = FALSE;
			}
        	break;    
        case 221: 
        	_fstrcpy (MaskAreaFile,Value); 
        	DeleteAllVPRegions ();
        	break;
        case 223:
        	HltAutoClear = atob (Value);
        	break;
        
        case 224:
        	ConvertToGrayTechnique = atoi (Value);
        	break; 
        case 227:
        	DisplayLineBPEP	= atob (Value); 
			break;  
        case 229:  
        	if (*Value == '2')
        		DisplayFailure = 2;
        	else
        		DisplayFailure = atob (Value);
        	break;
        
        case 234:
            if (!_fstricmp(Value,"Feet"))
            	PRJ_UNITS[3] = 1;
         	else if (!_fstricmp(Value,"Meters"))
            	PRJ_UNITS[3] = 2; 
        	break;
        case 235:    
        	GSSiGlobFree (&hEndDisplayCommand);  
        	if (*Value)
        	{
	        	hEndDisplayCommand = GSSiGlobAlloc (1508,GMEM_MOVEABLE,4096);
	        	pCmd = GlobalLock (hEndDisplayCommand);
	        	_fstrcpy (pCmd,Value);
	        	GlobalUnlock (hEndDisplayCommand); 
	        }
        	break;	
        case 236:
        	ShowVirtualPrintAreas = atob (Value);
        	break;
        case 237:
        	BufferedScreen = atob (Value); 
			if (InServerMode)
				BufferedScreen = FALSE;
        	if (!BufferedScreen)
       			ScreenBufferDC ((HWND)1,0); 
       		else if (*pNumViewports)
       		{    
       			SetConfig (1);
       		   	SetViewport(*pCommandViewport);
			    SetupViewports (CurView->hWnd,CurView->hDC,0,MainRect,0); 
			}
        	break;  

        case 238:
        	SetShowValDB (Value);
        	break;

        case 240:
        	UseHollowStreets = atob (Value);
        	break;
        	
        case 241:
        	CurrentVoterID = atol (Value);
        	break;  
        	
        case 242:
        	CurrentVoterDB = atol (Value);
        	break;

        case 243:
        	AutoHighlight = atob (Value);
        	break;
        
        case 244:
        	DisableHalt = atob (Value);
        	break;
        case 245:
        	UseShortSymbols = atob (Value);
        	break;
        case 246:
        	CompressAccuracy = atof (Value);
        	break; 
        case 247:
        	ForceHalfTone = atob (Value);
			break;
        case 248:
        	SymbolInUseShieldsList (-9999);
			break;
		case 249:  
			LLNormFactor = atof (Value);
			break;
		case 250:  
			PrimeNameOnly = atob (Value);
			break; 
		case 251:  
			if (*Value)
				PreBuiltMapDesc = GetDictSymbolNumber (Value);
			else
				PreBuiltMapDesc = 0;
			break;
		case 252:  
			MaxMidpoints = atol (Value);
			break; 

		case 254:
			ShowShieldDir = atoi (Value);  
			break;
			
		case 260:
			UseTestChar = atob (Value);
			break;    
		case 262:
			RandomAreasAreTransparent = atob (Value);
			break;
 		case 263:
			DisplayShields = atob (Value);
			break;
 		case 266:
			ShieldsOnly = atob (Value);
			break;
        case 269:
        	DisplayAreaPoints = atob (Value); 
			break;  
 		case 274:
			TraceWnd = (HWND)atol (Value);
			break;
		case 279:
			AutoOrthoColor = atol (Value);
			break;
		case 280:
			AutoOpaqueSetting = atob (Value);
			break;
		case 283:
			UseMappedFiles = atob (Value);
			break;
		case 284:
			DisplayText = atob (Value);
			break;
		case 285:
			AlwaysUseZoomMacro = atob (Value);
			break;
        case 286:
        	DisplayLinePoints = atob (Value); 
			break;  
		case 287:
			SetLogSocketIO (atob(Value));
			break;
		case 288:
			if (*Value)
				DoNotPickThisRefno = atol(Value);
			else
				DoNotPickThisRefno = LONG_MAX;
			break;
        case 289:
			BlockVehicleDisplay = atob (Value);
			break;
		case 291:
			strcpy (TraceVar,Value);
			break;
		case 292:
			MoveHorzVert = *strupr(Value);
			break;
        case 293:
			PickPointSymbol = atob (Value);
			break;
        case 294:
			dbug = atob (Value);
			break;
		case 295:
			DisplayPartialBuffer = atob (Value);
			break;
		case 296:
			debugvalue = atol (Value);
			break;
		case 297:
			ExpandGrText = atob (Value);
			break;
		case 298:
			ShieldFactor = atof (Value);
			break;
		case 300:
			ReplayDelay = atoi (Value);
			break;
		case 301:
			UseSymnumColor = atob (Value);
			break;
		case 304:
			PrinterMarginLeft = atof (Value);
			break;
		case 305:
			PrinterMarginRight = atof (Value);
			break;
		case 306:
			PrinterMarginTop = atof (Value);
			break;
		case 307:
			PrinterMarginBottom = atof (Value);
			break;
		case 308:
			StreetTextFactor = atof (Value);
			break;
		case 309:
			StreetWidthFactor = atof (Value);
			break;
		case 310:
			StreetWidth = atof (Value);
			break;
		case 311:
			FillStreetWithThisColor = atol (Value);
			break;
		case 312:
			StreetOneWay = atol (Value);
			break;
		case 313:
			GMDMinCode = atol (Value);
			break;
		case 314:
			GMDMaxCode = atol (Value);
			break;
		case 315:
			NoDisplay = atob (Value);
			break;
		case 316:
			SaveContourElev = atob (Value);
			break;
		case 318:
			SetShowContourLines(atob (Value));
			break;
		case 319:
			SetShowDepthColors (atob (Value));
			break;
		case 320:
			SetHighlightDepth (atol(Value));
			break;
		case 321:
			GridPixelWidth = atol (Value);
			break;
		case 322:
			CurrentLakeType = atol (Value); 
			break;
		case 323:
			strcpy (ShieldSaveFile,Value);
			break;
		case 324:
			AllowJournal = atob (Value);
			if (AllowJournal)
			{
				AllowCache = FALSE;
        		CloseAllRequestedFiles (FALSE);
        		KeepFilesOpen = FALSE;
				CheckPointBegin ();
			}
			else
				CheckPointEnd ();
			break;
		case 325:
			strcpy (TraceString,Value);
			break;
		case 326:
			UseShortHighlightList = atob (Value);
			break;
		case 327:
			DepthColorIntensity = atol (Value);
			break;
		case 328:
			ShallowWaterHighlight = atol (Value);
			break;
		case 329:
			WaterLevelOffset = atol (Value);
			break;
		case 330:
			VehicleStatusChanged = atob (Value);
			break;
		case 331:
			strcpy (AltAddressDir,Value);
			break;
		case 332:
			strcpy (GMDCodeList,Value);
			break;
		case 333:
			UseHLTGraphicsFile = atob (Value);
			break;
		case 334:
			strcpy (AutoMoveFile,Value);
			ConvertToNewLocation (0,0);
			break;
		case 335:
			RunFromCache = atob (Value);
			CloseAllRequestedFiles (FALSE);
			if (RunFromCache)
			{
				GetGlobalCVal ("[%DL]",OriginalDL,0);
				//AllowCache = FALSE;
				ExpandText (CachePathnameTo);
				SetGlobalValue("%DATA_LOC",CachePathnameTo);
				SetGlobalValue ("%DL",CachePathnameTo);  
			}
			else if (*OriginalDL)
			{
				SetGlobalValue("%DATA_LOC",OriginalDL);
				SetGlobalValue ("%DL",OriginalDL); 
			}
			ConvertToNewLocation (0,0);
			break;
		case 336:
			strcpy (UserAddressSubDir,Value);
			CloseUserDefinedAddress  (TRUE);
			break;
		case 337:
			strcpy (LogGPSInputFile,Value);
			break;
		case 339:
			if (atob (Value))
				GetPolygonPickAccelerator (1);
			else
				GetPolygonPickAccelerator (2);
			break;
		case 340:
			ConvertPickedItems = atob (Value);
			break;
		case 342:
			strcpy (SymbolImage,Value);
			break;
		case 343:
			VehicleTrackMaxTime = atoi (Value);
			break;
		case 344:
			AllowingSocketConnections = atob (Value);
			break;
		case 345:
			UMIODebug = atob (Value);
			break;
		case 346:
			DisplayOnlyHLT = atob (Value);
			break;
		case 347:
			ItemSymbolWidth = atof (Value);
			break;
		case 351:
			CurrentGridLevel = atoi (Value);
			break;
		case 352:
			CurrentGridRow = atoi (Value);
			break;
		case 353:
			CurrentGridCol = atoi (Value);
			break;
		case 354:
			strcpy (ShowVal.Text,Value);
			break;
		case 357:
			strcpy (VirtPrinterImageFile,Value);
			break;
		case 359:
			strcpy (CfgName,Value);
			break;
		case 360:
			removeContourSize = atof (Value);
			break;
		case 361:
			contourPointMergeDist = atof (Value);
			break;
		case 362:
			contourPointLinearSmooth = atof (Value);
			break;
		case 363:
			displayOriginalContours = atob (Value);
			break;
		case 364:
		{
			int maxc = atoi (Value);
			AddBMPToCache32 (0,(HDIB32)maxc);
		}
 			break;
		case 365:
			WantPNData = atoi (Value);
			break;
		case 366:
			strcpy (TestFileLocation,Value);
			if (*LastChr (TestFileLocation) != '\\' &&
				*LastChr (TestFileLocation) != '/')
				strcat (TestFileLocation,"\\");
			EscapeFunction(FALSE);
			break;
		case 368:
			strcpy(SQLErrorLog, Value);
			break;
		case 369:
			literalChar = Value[0];
			break;
		default:
 			break;
	}
/*	if (TraceOn)
	{
		char	str[300];
		extern	HWND hWndMain;
		sprintf(str,"%s set to %s",Name,Value);
		//GSSiTrace (str);
		SetWindowText(hWndMain,str);
	} */
{
#if ENABLETRACE
GSSiExitProg (532);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}   
void CreateInternalGlobals (void)
#if ENABLETRACE
{GSSiEnterProg (543);
#endif
{
	AllocateTypeVar("%SYS_CLOCK",1,FALSE);
	AllocateTypeVar("%LOCAL_CORRECTION",2,FALSE); 
	AllocateTypeVar("%STATE",3,FALSE);
	AllocateTypeVar("%SHOW_SCALE",4,FALSE);
	AllocateTypeVar("%DISPLAY_AREAS",5,FALSE);
	AllocateTypeVar("%DISPLAY_GRAPHICS",6,FALSE);
	AllocateTypeVar("%TRACE",7,FALSE);
	AllocateTypeVar("%FILL_AREAS",8,TRUE);
	hIntRefno=AllocateTypeVar("%INT_REFNO",9,FALSE);
	hPrefix=AllocateVar("%PREFIX");
	hUDI=AllocateVar("%UDI");
	hTEXT=AllocateVar("%TEXT");
	AllocateTypeVar("%MAXPICK",10,TRUE);
	AllocateTypeVar("%PEN_COLOR",11,FALSE);
	AllocateTypeVar("%PEN_WIDTH",12,FALSE);
	AllocateTypeVar("%SYM_COLOR",13,FALSE);
	AllocateTypeVar("%TEXT_FREQ",14,TRUE);
	AllocateTypeVar("%FONT",15,TRUE);
	AllocateTypeVar("%WT",16,FALSE);
	AllocateTypeVar("%ID",17,FALSE);
	AllocateTypeVar("%PAGE",18,FALSE); 
	AllocateTypeVar("%MARKERS",19,TRUE);
	AllocateTypeVar("%DISPLAY_SYMBOLS",20,TRUE);   
	AllocateTypeVar("%YEAR",21,FALSE);
	AllocateTypeVar("%YEAR1900",22,FALSE);
	AllocateTypeVar("%MONTH",23,FALSE);  
	AllocateTypeVar("%YDAY",24,FALSE); 
	AllocateTypeVar("%MDAY",25,FALSE);
	AllocateTypeVar("%WDAY",26,FALSE);
	AllocateTypeVar("%HOUR",27,FALSE);
	AllocateTypeVar("%MIN",28,FALSE);
	AllocateTypeVar("%SEC",29,FALSE);
	AllocateTypeVar("%CMONTH",30,FALSE);
	AllocateTypeVar("%CDAY",31,FALSE);
	AllocateTypeVar("%DATE",32,FALSE);
	AllocateTypeVar("%TIME",33,FALSE);
	AllocateTypeVar("%MTIME",34,FALSE);			
	AllocateTypeVar("%MAX_POINTS",35,FALSE);			
	AllocateTypeVar("%MOST_POINTS",36,FALSE);			
	AllocateTypeVar("%SET_REFNO",37,FALSE);			
	AllocateTypeVar("%DISPLAY_COORD",38,FALSE);			
	AllocateTypeVar("%BYSTATE",39,FALSE);			
	AllocateTypeVar("%XMN",40,FALSE);			
	AllocateTypeVar("%YMN",41,FALSE);			
	AllocateTypeVar("%XMX",42,FALSE);			
	AllocateTypeVar("%YMX",43,FALSE);			
    AllocateTypeVar("%USEBINFILELIST",44,FALSE);
	AllocateTypeVar("%AREA_COLOR",45,FALSE);
	AllocateTypeVar("%PICK_LIMIT",46,TRUE);
	AllocateTypeVar("%PROCESS_GCMD_STRINGS",47,TRUE);
	AllocateTypeVar("%HIGHLIGHT_COLOR",48,TRUE);
	AllocateTypeVar("%D16",49,FALSE);
	AllocateTypeVar("%FAST_ORTHOS",50,FALSE);
	AllocateTypeVar("%SHOW_FILE_BOUNDS",51,FALSE);
	AllocateTypeVar("%CURSOR_LOCKED",52,FALSE);
	AllocateTypeVar("%PICKAP",53,TRUE);
	AllocateTypeVar("%ADDRESS_TEXT_SIZE",54,TRUE);
	AllocateTypeVar("%STREET_TEXT_SIZE",55,TRUE); 
	AllocateTypeVar("%WANTSTREETS",56,TRUE);  
	AllocateTypeVar("%OAX",57,FALSE);
	AllocateTypeVar("%OAY",58,FALSE);     
	AllocateTypeVar("%AREA_UNITS",59,TRUE);
	AllocateTypeVar("%WX",60,FALSE);
	AllocateTypeVar("%WY",61,FALSE);
	AllocateTypeVar("%DIST_UNITS",62,TRUE);
	AllocateTypeVar("%MIN_SPEED",63,TRUE);
	AllocateTypeVar("%MAX_SPEED",64,TRUE);
	AllocateTypeVar("%C",65,FALSE);
	AllocateTypeVar("%POINTS",78,FALSE); 
	AllocateTypeVar("%BOUNDS",79,FALSE); 
	AllocateTypeVar("%VPBOUNDS",80,FALSE); 
	AllocateTypeVar("%REDISPLAY_ONLY",81,FALSE); 
	AllocateTypeVar("%ESC",82,FALSE); 
	AllocateTypeVar("%CR",83,FALSE);  
	AllocateTypeVar("%BASEUNITSPERORTHOPIXEL",84,FALSE);
	AllocateTypeVar("%BITMAPSTRETCHMODE",85,FALSE);
	AllocateTypeVar("%DOTIME",86,FALSE);	
	AllocateTypeVar("%MINFILETIME",87,FALSE);	
	AllocateTypeVar("%MAXFILETIME",88,FALSE);	
	AllocateTypeVar("%TRACKVIDEO",89,FALSE);	
	AllocateTypeVar("%FONTNAME",90,FALSE);
	AllocateTypeVar("%LAYERID",91,FALSE);	
	AllocateTypeVar("%MAPTOTRAN",92,FALSE);	
	AllocateTypeVar("%MAXPOINTSIZE",93,TRUE);	
	AllocateTypeVar("%DISPLAYHLTPATTERN",94,TRUE);	
	AllocateTypeVar("%LINKCURSORS",95,TRUE);	
	AllocateTypeVar("%TRACK_COLOR",96,TRUE);
	AllocateTypeVar("%IMAGE_EXTENSION",97,FALSE);
	AllocateTypeVar("%PICKALLPIECES",98,TRUE);
	AllocateTypeVar("%ALT_PROJECTION",99,FALSE);
	AllocateTypeVar("%INCLUSIONOPT",100,FALSE);
	AllocateTypeVar("%TRACK_WIDTH",101,TRUE);
	AllocateTypeVar("%HIGHLIGHT_WIDTH",102,TRUE);  
	AllocateTypeVar("%AutoClearOffset",103,TRUE);  
	AllocateTypeVar("%MaskOffsetLine",104,TRUE);  
	AllocateTypeVar("%TextFactor",105,TRUE);  
	AllocateTypeVar("%OUTX",106,FALSE);  
	AllocateTypeVar("%OUTY",107,FALSE);  
	AllocateTypeVar("%OUTPT",108,FALSE);  
	AllocateTypeVar("%CURPT",109,FALSE);  
	AllocateTypeVar("%USEITEMLENGTH",110,TRUE);  
	AllocateTypeVar("%WANTFUNNAMES",111,TRUE);  
	AllocateTypeVar("%PROCESSALLELEMENTS",112,TRUE);  
	AllocateTypeVar("%TAG",113,FALSE);  
	AllocateTypeVar("%INVISPOINTSYM",114,FALSE);  
	AllocateTypeVar("MAX_TEXT_LAYER",115,TRUE);  
	AllocateTypeVar("%AZ",116,FALSE);  
	AllocateTypeVar("%PI",117,FALSE);  
	AllocateTypeVar("%TRACETYPE",118,FALSE);  
	AllocateTypeVar("%WINDOWCOLOR",119,TRUE);
	AllocateTypeVar("%ARG",120,FALSE);
	AllocateTypeVar("%LASTRECTIME",121,FALSE);
	AllocateTypeVar("%SHOWLINKLINES",122,TRUE);
	AllocateTypeVar("%SHOWONLYFIRSTREF",123,FALSE);
	AllocateTypeVar("%CLASSCOLOR",124,FALSE);
	AllocateTypeVar("%P",125,FALSE);
	AllocateTypeVar("%PICKPERIM",126,TRUE);
	AllocateTypeVar("%CURVECHORDDIST",127,TRUE);
	AllocateTypeVar("%AUTOPAN",128,TRUE);
	AllocateTypeVar("%GETSEGDATAFROMGLOBALS",129,TRUE);
	AllocateTypeVar("%INVERTNETDIR",130,FALSE);
	AllocateTypeVar("%PointFactor",131,TRUE);  
	AllocateTypeVar("%MaskZoomArea",132,TRUE);  
	AllocateTypeVar("%OutlineZoomArea",133,TRUE);  
	AllocateTypeVar("%BleedThrough",134,TRUE);  
//	AllocateTypeVar("%OffsetLineOffset",135,TRUE);  
	AllocateTypeVar("%LocationOffset",136,TRUE);  
	AllocateTypeVar("%WinDir",137,FALSE);  
	AllocateTypeVar("%TIMERANGEBEGIN",138,FALSE);  
	AllocateTypeVar("%TIMERANGEEND",139,FALSE);  
	AllocateTypeVar("%PROJECTION",140,FALSE);  
	AllocateTypeVar("%LAYERNUM",141,FALSE);  
	AllocateTypeVar("%DISPLAYONLYONCE",142,FALSE);  
	AllocateTypeVar("%USESYMDICT",143,FALSE);  
	AllocateTypeVar("%LoadCompiledTran",144,FALSE);  
	AllocateTypeVar("%IBFROMBOUNDS",145,FALSE);  
	AllocateTypeVar("%IBRECT",146,FALSE);  
	AllocateTypeVar("%TRACKINGSTATUS",147,FALSE);  
	AllocateTypeVar("%LINESYMBOLFACTOR",148,TRUE);  
	AllocateTypeVar("%TEXTTYPE",149,FALSE);  //for oracle text transfer test report
	AllocateTypeVar("%COPYSIZE",150,FALSE);  
	AllocateTypeVar("%GETCOPYSIZE",151,FALSE);  
	AllocateTypeVar("%DF",152,FALSE);  
	AllocateTypeVar("%SHOWALLREFS",153,TRUE);  
	AllocateTypeVar("%INVISPOINTFACTOR",154,TRUE);  
	AllocateTypeVar("%DISPLAYVALUESIZE",155,TRUE);  
	AllocateTypeVar("%NEWTAG",156,FALSE);  
	AllocateTypeVar("%DISPLAYORTHORES",157,FALSE);  
	AllocateTypeVar("%COMPUTEAREA",158,TRUE);  
	AllocateTypeVar("%SHOWBADSYMBOLS",159,TRUE);  
	AllocateTypeVar("%PICKDELETES",160,FALSE);  
	AllocateTypeVar("%RECNUM",161,FALSE);  
	AllocateTypeVar("%STANDARDSQL",162,FALSE);  
	AllocateTypeVar("%MINCACHEDRIVEFREESPACE",163,FALSE);  
	AllocateTypeVar("%POLYINFOTHEME",164,FALSE); 
	AllocateTypeVar("%SHORTESTROUTE",165,FALSE); 
	AllocateTypeVar("%SNAPPOLYGONS",166,FALSE); 
	AllocateTypeVar("%TEXTLOC",167,FALSE); 
	AllocateTypeVar("%AUTOTYPE",168,FALSE); 
	AllocateTypeVar("%SHOWREFNO",169,FALSE); 
	AllocateTypeVar("%FONTWIDTHFACTOR",170,TRUE); 
	AllocateTypeVar("%TextWeight",171,TRUE);  
	AllocateTypeVar("%LineWidthFactor",172,TRUE);  
	AllocateTypeVar("%VISLISTOPT",173,TRUE);   
	AllocateTypeVar("%ALLOWPICKDELETES",174,FALSE);   
	AllocateTypeVar("%SCALEBARUNITS",175,FALSE);   
	AllocateTypeVar("%SYM_DICT",176,FALSE);   
	AllocateTypeVar("%MaxFileSizeToCache",177,FALSE);  
	AllocateTypeVar("%DISPLAYVALUECOLOR",178,TRUE);  
	AllocateTypeVar("%NOCACHE",179,FALSE);  
	AllocateTypeVar("%TEXTCOLOR",180,TRUE);  
	AllocateTypeVar("%VPHEIGHT",181,FALSE);  
	AllocateTypeVar("%CHECKCACHEONCE",182,FALSE);  
	AllocateTypeVar("%CACHEALL",183,FALSE);  
	AllocateTypeVar("%USERNAME",184,FALSE);  
	AllocateTypeVar("%NODENAME",185,FALSE);  
	AllocateTypeVar("%INPLOTVIEW",186,FALSE);  
	AllocateTypeVar("%USERLEV",187,FALSE);  
	AllocateTypeVar("%VERSION",188,FALSE);  
	AllocateTypeVar("%NUMSYMBOLS",189,FALSE);  
	AllocateTypeVar("%TEMPDIR",190,FALSE);  
	AllocateTypeVar("%CHECKTIMESTAMPS",191,FALSE);  
	AllocateTypeVar("%KFO",192,FALSE);  
	AllocateTypeVar("%CHECKPATH",193,FALSE);  
	AllocateTypeVar("%CANDISPLAYZEROLENGTHLINES",194,FALSE);  
	AllocateTypeVar("%UNDOENABLE",195,FALSE);  
	AllocateTypeVar("%ROP",196,FALSE);  
	AllocateTypeVar("%SHOWDELETEDITEMS",197,FALSE);  
	AllocateTypeVar("%CMDMESS",198,FALSE);  
	AllocateTypeVar("%BLOCKINPUT",199,FALSE);  
	AllocateTypeVar("%LOADDYNAMICTEXT",200,FALSE); 
	AllocateTypeVar("%MINREFNO",201,FALSE); 
	AllocateTypeVar("%CONVERTTEXTCHAR",202,FALSE); 
	AllocateTypeVar("%NUMOFFSETFAILED",203,FALSE); 
	AllocateTypeVar("%SHPRECNO",204,FALSE); 
	AllocateTypeVar("%FTM",205,FALSE); 
	AllocateTypeVar("%MFT",206,FALSE); 
	AllocateTypeVar("%MSLINK",207,FALSE); 
	AllocateTypeVar("%DISPLAYPASS",208,FALSE); 
	AllocateTypeVar("%FONTCOLORS",209,FALSE); 
	AllocateTypeVar("%USEDGNCOLORS",210,FALSE); 
	AllocateTypeVar("%SHOWNODESREF",211,FALSE); 
	AllocateTypeVar("%HIGHLIGHTMULTREFS",212,FALSE); 
	AllocateTypeVar("%SHOWSQLERRORS",213,FALSE); 
	AllocateTypeVar("%SD",214,FALSE); 
	AllocateTypeVar("%DLLDIR",215,FALSE); 
	AllocateTypeVar("%TRACEREF",216,FALSE); 
	AllocateTypeVar("%LOGUSAGE",217,FALSE); 
	AllocateTypeVar("%PICKPOINTS",218,FALSE);   
	AllocateTypeVar("%NEWREFFILE",219,FALSE);   
	AllocateTypeVar("%ALLOWCACHE",220,FALSE);   
	AllocateTypeVar("%MASKAREAFILE",221,FALSE);   
	AllocateTypeVar("%BACKGROUNDTASK",222,FALSE);  
	AllocateTypeVar("%HLTAUTOCLEAR",223,FALSE);
	AllocateTypeVar("%CONVERTTOGRAYTECHNIQUE",224,TRUE);
	hSymNum=AllocateTypeVar("%SYMNUM",225,FALSE);
	AllocateTypeVar("%HLTBOUNDS",226,FALSE);
	AllocateTypeVar("%DISPLAYBPEP",227,FALSE);
	AllocateTypeVar("%NUMCACHEDIR",228,FALSE);
	AllocateTypeVar("%DISPLAYFAILURE",229,FALSE);
	AllocateTypeVar("%AREA",230,FALSE);
	AllocateTypeVar("%PERIMETER",231,FALSE);
	AllocateTypeVar("%LENGTH",232,FALSE);
	AllocateTypeVar("%AZIMUTH",233,FALSE);
	AllocateTypeVar("%UNITS",234,FALSE); 
	AllocateTypeVar("%ENDDISPLAYCOMMAND",235,FALSE); 
	AllocateTypeVar("%SHOWPRINTAREAS",236,FALSE); 
	AllocateTypeVar("%BUFFERSCREEN",237,FALSE); 
	AllocateTypeVar("%SHOWVALDB",238,FALSE); 
	AllocateTypeVar("%HAVEDESTINATION",239,FALSE); 
	AllocateTypeVar("%HOLLOWSTREETS",240,TRUE); 
	AllocateTypeVar("%CURRENTVOTERID",241,FALSE); 
	AllocateTypeVar("%CURRENTVOTERDB",242,FALSE); 
	AllocateTypeVar("%AUTOHIGHLIGHT",243,FALSE); 
	AllocateTypeVar("%DISABLEHALT",244,FALSE);  
	AllocateTypeVar("%USESHORTSYMBOLS",245,FALSE);  
	AllocateTypeVar("%COMPRESSACCURACY",246,FALSE);
	AllocateTypeVar("%HALFTONEALL",247,FALSE);
	AllocateTypeVar("%USESHIELDSYMBOLS",248,FALSE);
	AllocateTypeVar("%LLNORMFACTOR",249,FALSE);
	AllocateTypeVar("%PRIMENAMEONLY",250,FALSE);
	AllocateTypeVar("%PREBUILTMAPSYMBOL",251,FALSE);
	AllocateTypeVar("%MAXMIDPOINTS",252,FALSE);
	AllocateTypeVar("%PRINTPROMPT",253,FALSE);
	AllocateTypeVar("%SHOWSHIELDDIR",254,FALSE);
	AllocateTypeVar("%ALLOWGFMENU",255,FALSE);
	AllocateTypeVar("%AUTOSAVECFG",256,FALSE);
	AllocateTypeVar("%CFGSAVEPROMPT",257,FALSE);
	AllocateTypeVar("%PRINTING",258,FALSE);  
	AllocateTypeVar("%PROFILEFILE",259,FALSE);
	AllocateTypeVar("%USETESTPASS",260,FALSE);
	AllocateTypeVar("%VPZOOM",261,FALSE);     
	AllocateTypeVar("%RANDOMAREASARETRANSPARENT",262,TRUE);
	AllocateTypeVar("%DISPLAYSHIELDS",263,TRUE);
	AllocateTypeVar("%TRAVIDDB",264,FALSE);
	AllocateTypeVar("%TRAVDATADB",265,FALSE);
	AllocateTypeVar("%SHIELDSONLY",266,TRUE);
	AllocateTypeVar("%USERDIR",267,FALSE);
	AllocateTypeVar("%USERLIB",268,FALSE);
	AllocateTypeVar("%DISPLAYAREAPOINTS",269,FALSE); 
	AllocateTypeVar("%ALLOWSYMBOLCREATION",270,FALSE);  
	AllocateTypeVar("%HELPFILE",271,FALSE);  
	AllocateTypeVar("%FILEZML",272,FALSE);  
	AllocateTypeVar("%READPCT",273,FALSE);  
	AllocateTypeVar("%TRACEWND",274,FALSE);
	AllocateTypeVar("%DESKTOPWIDTH",275,FALSE);
	AllocateTypeVar("%DESKTOPHEIGHT",276,FALSE);
	AllocateTypeVar("%WINDOWWIDTH",277,FALSE);
	AllocateTypeVar("%WINDOWHEIGHT",278,FALSE);
	AllocateTypeVar("%AUTO_ORTH_COLOR",279,FALSE);
	AllocateTypeVar("%AUTO_ORTH_OPAQUE",280,FALSE);
	AllocateTypeVar("%DEGTORAD",281,FALSE);
	AllocateTypeVar("%RADTODEG",282,FALSE);
	AllocateTypeVar("%USEMAPPEDFILES",283,FALSE);
	AllocateTypeVar("%DISPLAYTEXT",284,FALSE);
	AllocateTypeVar("%ALWAYSUSEZOOMMACRO",285,FALSE);
	AllocateTypeVar("%DISPLAYLINEPOINTS",286,FALSE); 
	AllocateTypeVar("%LOGSOCKETIO",287,FALSE); 
	AllocateTypeVar("%DONOTPICKREFNO",288,FALSE); 
	AllocateTypeVar("%BlockVehicleDisplay",289,FALSE);
	AllocateTypeVar("%GRAPHICSFILE",290,FALSE);
	AllocateTypeVar("%TRACEVAR",291,FALSE);
	AllocateTypeVar("%MOVEHORZVERT",292,FALSE);
	AllocateTypeVar("%PICKPOINTSYM",293,FALSE);
	AllocateTypeVar("%DEBUG",294,FALSE);
	AllocateTypeVar("%DISPLAYPARTIALBUFFER",295,FALSE);
	AllocateTypeVar("%DEBUGVALUE",296,FALSE);
	AllocateTypeVar("%EXPANDGRAPHICTEXT",297,FALSE);
	AllocateTypeVar("%SHIELDFACTOR",298,TRUE);
	AllocateTypeVar("%CACHEDIR",299,FALSE);
	AllocateTypeVar("%REPLAYDELAY",300,FALSE);
	AllocateTypeVar("%USESYMNUMCOLOR",301,FALSE);
	AllocateTypeVar("%PRINTERWIDTH",302,FALSE);
	AllocateTypeVar("%PRINTERHEIGHT",303,FALSE);
	AllocateTypeVar("%PRINTERMARGINLEFT",304,FALSE);
	AllocateTypeVar("%PRINTERMARGINRIGHT",305,FALSE);
	AllocateTypeVar("%PRINTERMARGINTOP",306,FALSE);
	AllocateTypeVar("%PRINTERMARGINBOTTOM",307,FALSE);
	AllocateTypeVar("%STREETTEXTFACTOR",308,TRUE);
	AllocateTypeVar("%STREETWIDTHFACTOR",309,TRUE);
	AllocateTypeVar("%STREETWIDTH",310,TRUE);
	AllocateTypeVar("%STREETFILLCOLOR",311,TRUE);
	AllocateTypeVar("%ONEWAY",312,FALSE);
	AllocateTypeVar("%GMDCODEMIN",313,TRUE);
	AllocateTypeVar("%GMDCODEMAX",314,TRUE);
	AllocateTypeVar("%NODISPLAY",315,FALSE);
	AllocateTypeVar("%SAVECONTOURELEV",316,FALSE);
	AllocateTypeVar("%SYSTEMPDIR",317,FALSE);  
	AllocateTypeVar("%SHOWHBIRDCONTOURLINES",318,FALSE);  
	AllocateTypeVar("%SHOWHBIRDDEPTHCOLORS",319,FALSE);  
	AllocateTypeVar("%HIGHLIGHTDEPTH",320,FALSE);  
	AllocateTypeVar("%GRIDSIZE",321,FALSE);  
	AllocateTypeVar("%LAKETYPE",322,FALSE);  
	AllocateTypeVar("%SHIELDSAVEFILE",323,FALSE);  
	AllocateTypeVar("%ALLOWJOURNAL",324,FALSE);   
	AllocateTypeVar("%TRACESTRING",325,FALSE); 
	AllocateTypeVar("%SHORTHLTLIST",326,FALSE); 
	AllocateTypeVar("%DepthColorIntensity",327,TRUE); 
	AllocateTypeVar("%ShallowWaterHighlight",328,TRUE); 
	AllocateTypeVar("%WaterLevelOffset",329,TRUE); 
	AllocateTypeVar("%VEHICLESTATUSCHANGED",330,FALSE); 
	AllocateTypeVar("%ALTADDRESSDIR",331,FALSE); 
	AllocateTypeVar("%GMDCODELIST",332,TRUE);
	AllocateTypeVar("%USEHLTGRAPHICSFILE",333,FALSE);
	AllocateTypeVar("%ADDMATCHEDITRECT",0,TRUE);
	AllocateTypeVar("%AUTOMOVEFILE",334,FALSE);
	AllocateTypeVar("%RUNFROMCACHE",335,FALSE);
	AllocateTypeVar("%UserDefinedAddressSubDir",336,FALSE);
	AllocateTypeVar("%GPSInputLogFile",337,FALSE);
	AllocateTypeVar("%ClientSocket",338,FALSE);
	AllocateTypeVar("%UsePickAccelerator",339,FALSE);
	AllocateTypeVar("%ConvertPickedItems",340,FALSE);
	AllocateTypeVar("%CACHEDIRACTUAL",341,FALSE);
	AllocateTypeVar("%SYMBOLIMAGE",342,FALSE);
	AllocateTypeVar("%VEHTRACKMAXTIME",343,TRUE);
	AllocateTypeVar("%ALLOWSOCKETCONNECTIONS",344,FALSE);
	AllocateTypeVar("%TCPTRACE",345,FALSE);
	AllocateTypeVar("%DISPLAYONLYHLT",346,FALSE);
	AllocateTypeVar("%WIDTH",347,FALSE);
	AllocateTypeVar("%TOTHLTLENGTH",348,FALSE);
	AllocateTypeVar("%TOTHLTPERIMETER",349,FALSE);
	AllocateTypeVar("%TOTHLTAREA",350,FALSE);
	AllocateTypeVar("%GRIDZOOM",351,FALSE);
	AllocateTypeVar("%GRIDROW",352,FALSE);
	AllocateTypeVar("%GRIDCOL",353,FALSE);
	AllocateTypeVar("%PROJECTBOUNDS",0,FALSE);
	AllocateTypeVar("%DVTEXT",354,FALSE);
	AllocateTypeVar("%SQFTSQMET",355,FALSE);
	AllocateTypeVar("%SQMETACRES",356,FALSE);
	AllocateTypeVar("%VIRTUALPRINTEROUTFILE",357,FALSE);
	AllocateTypeVar("%SCREENRES",358,FALSE);
	AllocateTypeVar("%CONFIGFILE",359,FALSE);
	AllocateTypeVar("%CONTOURTOOSMALL",360,FALSE);
	AllocateTypeVar("%CONTOURDUPPOINT",361,FALSE);
	AllocateTypeVar("%CONTOURSMOOTH",362,FALSE);
	AllocateTypeVar("%CONTOURDISPLAYORIGINAL",363,FALSE);
	AllocateTypeVar("%MAXBMPCACHE",364,FALSE);
	AllocateTypeVar("%WANTPNDATA",365,FALSE);
	AllocateTypeVar("%TESTDL", 366, FALSE);
	AllocateTypeVar("%WINDOWSVERSION", 367, FALSE);
	AllocateTypeVar("%SQLERRORLOG", 368, FALSE);
	AllocateTypeVar("%LITERALCHAR", 369, FALSE);
	AllocateTypeVar("%SHAPEREC", 370, FALSE);
	AllocateTypeVar("%DBFREC", 371, FALSE);

//	AllocateTypeVar("%DL",191,FALSE);
	
{
#if ENABLETRACE
GSSiExitProg (543);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
short GetGlobalVal (HANDLE hGlobal,LPSTR OutStr,long Index)
#if ENABLETRACE
{GSSiEnterProg (533);
#endif
{   VARPNT	VarPnt, VarPnt2;
	int	l; 
	time_t	systime;     
	DPOINT	DPoint;
	RECT	Rect;
	double	Perimeter;  
	short	CurTyp=CurrentType;
	
	if (!hGlobal)
	{
		*OutStr = 0;
{
#if ENABLETRACE
GSSiExitProg (533);
#endif
		return 0;
}
	}
	VarPnt = (VARPNT)GlobalLock (hGlobal);  
	*OutStr=0;
	switch (VarPnt->Type)
	{   
		default:
		case 99:
		case 92:
		case 0: 
			if (VarPnt->ContainsGorF) 
			{
				HANDLE	hMem = GSSiGlobAlloc ( 192,GMEM_MOVEABLE,4096);
				LPSTR	pMem = GlobalLock (hMem);  
				
				_fstrcpy(pMem,VarPnt->Value); 
				ExpandText (pMem);
				l = _fstrlen (pMem);
				_fstrcpy(OutStr,pMem);
				GSSiGlobUlFree (&hMem);
			}
			else
			{
				_fstrcpy(OutStr,VarPnt->Value);
				l = VarPnt->Len; 
			}             
			GlobalUnlock (hGlobal);
{
#if ENABLETRACE
GSSiExitProg (533);
#endif
			return l;
}
		case 1:  
			time (&systime);
			ltoa ((long)systime,OutStr,10);
           	break;
		case 2:  
			if (CurView->FileProjectionType == 1)
				_fstrcpy (OutStr,"T");
			else
				_fstrcpy (OutStr,"F");
           	break;
		case 3:
			sprintf (OutStr,"%2.2i",CurState);
           	break;
		case 4:  
			if (ShowScale)
				_fstrcpy (OutStr,"T");
			else
				_fstrcpy (OutStr,"F");
           	break;
		case 6:
			btoa (DoGraphics,OutStr);
			break;     
		case 8:
			btoa (FillAreas,OutStr);
			break; 
		case 9:
           	ltoa (InternalRefno,OutStr,10);
           	break;
		case 10:
           	itoa (MaxPick,OutStr,10);
           	break;
		case 14:
			ltoa (TextFreq,OutStr,10);
			break;
		case 15:
           	itoa (CurFont,OutStr,10);
           	break;
		case 17:
           	ltoa (PrintReportID,OutStr,10);
           	break;
		case 18:
           	ltoa (PrintReportPage,OutStr,10);
           	break;
		case 19:
			btoa (DisplayMarkers,OutStr);
			break;   
		case 20:
			btoa (DisplaySymbol,OutStr);
			break;   
		case 21:
           	itoa (tmtime.tm_year+1900,OutStr,10);
           	break;
		case 22:
           	itoa (tmtime.tm_year,OutStr,10);
           	break;
		case 23:
           	itoa (tmtime.tm_mon+1,OutStr,10);
           	break;
		case 24:
           	itoa (tmtime.tm_yday+1,OutStr,10);
           	break;
		case 25:
           	itoa (tmtime.tm_mday,OutStr,10);
           	break;
		case 26:
           	itoa (tmtime.tm_wday+1,OutStr,10);
           	break;
		case 27:
           	itoa (tmtime.tm_hour,OutStr,10);
           	break;
		case 28:
           	itoa (tmtime.tm_min,OutStr,10);
			GlobalUnlock (hGlobal);
{
#if ENABLETRACE
GSSiExitProg (533);
#endif
			return (_fstrlen(OutStr)); 
}
		case 29:
           	itoa (tmtime.tm_sec,OutStr,10);
           	break;
		case 30:
		{
			char CMonth[12][10]={"January","February","March","April","May","June","July","August","September","October","November","December"};  
			if (tmtime.tm_mon < 0 || tmtime.tm_mon > 11)  
				*OutStr=0; 
			else
				_fstrcpy (OutStr,CMonth[tmtime.tm_mon]);
           	break;
		}	
		case 31:
		{
			char CDay[7][10]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};  
			if (tmtime.tm_wday < 0 || tmtime.tm_wday > 6)  
				*OutStr=0; 
			else
				_fstrcpy (OutStr,CDay[tmtime.tm_wday]);
           	break;
		}	
		case 32: 
		{
			short	year;
			   
			if (tmtime.tm_year < 100)
				year = tmtime.tm_year;
			else
				year = tmtime.tm_year-100;
           	sprintf (OutStr,"%i/%i/%2.2i",tmtime.tm_mon+1,tmtime.tm_mday,year);
        }
           	break;
		case 33: 
		{
			char	AMPM[4];
			
			if (tmtime.tm_hour < 12)
				_fstrcpy (AMPM,"AM");
			else
				_fstrcpy (AMPM,"PM");
           	sprintf (OutStr,"%i:%2.2i%s",(tmtime.tm_hour+11)%12+1,tmtime.tm_min,AMPM);
		}
           	break;
		case 34: 
           	sprintf (OutStr,"%i:%2.2i",tmtime.tm_hour,tmtime.tm_min);  
           	break;
		case 36: 
           	ltoa ((long)MostPoints,OutStr,10);
           	break; 
           	
		case 37:
			if (SetRefno)
				*OutStr++ = '1';
			else
				*OutStr++ = '0';
			*OutStr--=0;
			break;   
			
		case 40: 
		case 41:
		case 42:
		case 43:
		{
			LPDOUBLE	pRVal;  
			short	i;
			
			i=VarPnt->Type-40;
			pRVal = &CurView->WBounds.xmn;
			pRVal+=i;
			sprintf (OutStr,"%.14lg",*pRVal);
		}
		break;  
		case 46:
			sprintf (OutStr,"%.14lg",PickLimit);
			break;
				
		case 47:  
           	itoa (ProcessGCmdStrings,OutStr,10);
           	break;
		case 48:
			ltoa (HighlightColor,OutStr,10);
			break;
				
		case 50:
			btoa (FastOrthos,OutStr);
			break;   
			
		case 53:
			sprintf (OutStr,"%.14lg",UserPickAP);
		break;  
		case 54:
			sprintf (OutStr,"%.14lg",AddressTextSize);
		break;  
		case 55:
			sprintf (OutStr,"%.14lg",StreetTextSize);
		break;  
		case 56:  
			btoa (WantStreets,OutStr);
			break; 
		case 60:
			sprintf (OutStr,"%.14lg",CurrentPoint.x);
		break;  
		case 61:
			sprintf (OutStr,"%.14lg",CurrentPoint.y);
		break;  
		case 59:
			_fstrcpy (OutStr,AreaUnitOpts[OutAreaUnits-1]);
		break;
		case 62:
			_fstrcpy (OutStr,DistUnitOpts[OutDistUnits-1]);
		break;
		case 63:
			sprintf (OutStr,"%.14lg",MinSpeed);
		break;  
		case 64:
			sprintf (OutStr,"%.14lg",MaxSpeed);
		break; 
		case 65:  
			if (ContinueProcessing)
				*OutStr++ = '1';
			else
				*OutStr++ = '0';
			*OutStr--=0;
		break;
		case 78: /*%POINTS*/ 
			Index = min (Index,32);
			sprintf (OutStr,"%.14lg %.14lg",UserPoints[Index].x,UserPoints[Index].y);
		break;
		case 79: /*%BOUNDS*/ 
			switch (Index)
			{
				default:
				case 1:
				case 2: 
					Index = max (1,min(Index,2));
					sprintf (OutStr,"%.14lg %.14lg",UserBounds[Index-1].x,UserBounds[Index-1].y);
				break;
				case 0:
					sprintf (OutStr,"%.14lg %.14lg %.14lg %.14lg",UserBounds[0].x,UserBounds[0].y,UserBounds[1].x,UserBounds[1].y);
				break;
			}
		break;
		case 80: /*%VPBOUNDS*/ 
			switch (Index)
			{   
				default:
				case 0:
					sprintf (OutStr,"%.14lg %.14lg %.14lg %.14lg",CurView->WBounds.xmn, 
												  CurView->WBounds.ymn,
												  CurView->WBounds.xmx,
												  CurView->WBounds.ymx);  
				break;
				case 1:
					sprintf (OutStr,"%.14lg %.14lg",CurView->WBounds.xmn,CurView->WBounds.ymn);
				break;
				case 2:
					sprintf (OutStr,"%.14lg %.14lg",CurView->WBounds.xmx,CurView->WBounds.ymx);
				break; 
			} 
		
		break;
		
		case 82:    // ESC
			*OutStr++=0x1B;  
			*OutStr--=0;
		break;
		case 83:   // CR
			*OutStr++=0x0D;  
			*OutStr--=0;
		break;
		
		case 85:
			ltoa (StretchMode,OutStr,10);
			break; 
		
		case 87:
           	ltoa (MinFileTime,OutStr,10);
           	break;

		case 88:
           	ltoa (MaxFileTime,OutStr,10);
           	break; 
           	
        case 90:
        	Index = max (0,min(Index,15));
        	_fstrcpy (OutStr,FontNames[Index]);
        	break; 
        	
        case 91:
           	itoa (LayerID,OutStr,10);
		    break;  
		    
		case 93:
			sprintf (OutStr,"%.14lg",MaxPointSize); 
		break;
		  
		case 94:  
			btoa (DisplayHLTPattern,OutStr);
           	break; 
        
		case 95:
			btoa (DisplayLinkedCursors,OutStr);
			break;
			
		case 96:
			ltoa (TrackColor,OutStr,10);
			break;
        
        case 97:
        	_fstrcpy (OutStr,ImageExtension);
        	break; 

		case 98:  
			btoa (PickAllPieces,OutStr);
           	break; 
        
		case 100:
           	ltoa (InclusionOpt,OutStr,10);
           	break;
           	 
		case 101:
           	ltoa ((long)TrackWidth,OutStr,10);
           	break; 

		case 102:
           	ltoa ((long)HighlightWidth,OutStr,10);
           	break; 
           	
		case 103:
           	ltoa ((long)AutoClearOffset,OutStr,10);
           	break;  
		case 104:
           	ltoa ((long)MaskOffsetLine,OutStr,10);
           	break; 
		case 105:
			sprintf (OutStr,"%.14lg",GraphicsTextFactor);
			break;  
		case 106:
				    ConvertCoordClose ();
					ConvertCoordInit();
			DPoint = CurrentPoint;
			ConvertCoord (&DPoint,1,3);
			sprintf (OutStr,"%.14lg",DPoint.x);  
			break;
		case 107:
				    ConvertCoordClose ();
					ConvertCoordInit();
			DPoint = CurrentPoint;
			ConvertCoord (&DPoint,1,3);
			sprintf (OutStr,"%.14lg",DPoint.y);
			break;  
		case 108:
				    ConvertCoordClose ();
					ConvertCoordInit();
			DPoint = CurrentPoint;
			ConvertCoord (&DPoint,1,3);
			sprintf (OutStr,"%.14lg %.14lg",DPoint.x,DPoint.y);
			break;  
		case 109:
			DPoint = CurrentPoint;
			sprintf (OutStr,"%.14lg %.14lg",DPoint.x,DPoint.y);
			break;
			
		case 110:
			btoa (UseItemLen,OutStr);	
			break;
			
		case 111:
			btoa (WantFunNames,OutStr);	
			break;
			
		case 112:
			btoa (ProcessAllElements,OutStr);	
			break;
		
		case 113:
			VarPnt2 = (VARPNT)GlobalLock (hPrefix);
			_fstrcpy (OutStr,VarPnt2->Value);
			_fstrcat (OutStr,":");
			GlobalUnlock (hPrefix);
			VarPnt2 = (VARPNT)GlobalLock (hUDI);
			_fstrcat (OutStr,VarPnt2->Value);
			GlobalUnlock (hUDI);
        	break;
		case 115:
			itoa (MaxTextLayer,OutStr,10);
			break;	

		case 116:
			sprintf (OutStr,"%.14lg",CurrentAZ);
			break;  
		case 117:
			sprintf (OutStr,"%.14lg",(double)PY);
			break;  
		case 119:
			ltoa (WindowColor,OutStr,10);
			break;	

        case 120:
        	if (!hMacArgs)
        		break;
			if (Index >=1 && Index <= MAX_MACRO_ARGS)
			{   
				LPSTR	pArg;
				
				pArg = GlobalLock (hMacArgs);
				pArg += (Index-1) * 4096;
        		_fstrcpy (OutStr,pArg);
        		GlobalUnlock (hMacArgs);
        	}
        	break;  
		case 121:
           	ultoa (LastRecTime,OutStr,10);
           	break; 

        case 122:
        	btoa (ShowLinkLines,OutStr);	
        	break;
        
		case 123:
			btoa (!IgnorePrevLayers,OutStr);
			break;

        case 125:
        {		
			LPCMDSTRING    pCmdStr;   
	
			if (CurView)
			{   
				if (CurView->FunStackHandle)
				{
					pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle);
					if (pCmdStr->Prompt == PRMT_USERPROMPT)
						_fstrcpy (OutStr,pCmdStr->PromptText);
					GlobalUnlock (CurView->FunStackHandle);
				}
			}
		}
		break;   
		
        case 126:
        	itoa (PickPerim,OutStr,10);	
        	break;
        
		case 127:
			sprintf (OutStr,"%.14lg",CurveChordDist);
		break; 
		
		case 128:  
			btoa (AutoPan,OutStr);
           	break; 
        
		case 129:
			btoa (GetSegFromGlobals,OutStr);
			break; 
			
		case 131:
			sprintf (OutStr,"%.14lg",GraphicsPointFactor);
			break;  

		case 132:
			if (MaskOffsetLine)
				*OutStr++ = '1';
			else
				*OutStr++ = '0';
			*OutStr--=0;
			break;   
			
		case 133:
			if (OutlineZoomArea)
				*OutStr++ = '1';
			else
				*OutStr++ = '0';
			*OutStr--=0;
			break;   
			
		case 134:
			if (BleedThrough)
				*OutStr++ = '1';
			else
				*OutStr++ = '0';
			*OutStr--=0;
			break;   
			
		case 136:
			sprintf (OutStr,"%.14lg",LocationOffset);
		break; 
		
		case 137:
			GetWindowsDirectory (OutStr,144);  
		break; 
			
		case 138:
			ltoa (TimeRangeBeg,OutStr,10);
		break;
			
		case 139:
			ltoa (TimeRangeEnd,OutStr,10);
		break;
			
		case 141:
			itoa (FileNum+1,OutStr,10);
		break;
		
		case 142:
			if (hDisplayedRefs)
				strcpy (OutStr,"1");
			else
				strcpy (OutStr,"0");
			break;

		case 145:
			sprintf (OutStr,"%.14lg %.14lg %.14lg %.14lg",TAGBox.BlowUpBounds.xmn, 
														  TAGBox.BlowUpBounds.ymn,
														  TAGBox.BlowUpBounds.xmx,
														  TAGBox.BlowUpBounds.ymx);  
		    break;
		    
		case 146:
			sprintf (OutStr,"%i %i %i %i",TAGBox.rect.left, 
										  TAGBox.rect.bottom,
										  TAGBox.rect.right,
										  TAGBox.rect.top);  
			break;	
		case 147:
			ltoa (TrackingStatus,OutStr,10);
		break;  
		
		case 148:
			sprintf (OutStr,"%.14lg",LineSymbolFactor);
			break;    
			
		case 149:
			itoa (TextType,OutStr,10);
		break;  

		case 150:
           	ltoa ((long)TotCopySize,OutStr,10);
           	break;  
           	
        case 153:
        	btoa (DisplayAllRefs,OutStr);
        	break;
		case 154:  
			sprintf (OutStr,"%.14lg",InvisPointFactor);
			break;
		case 155:
			sprintf (OutStr,"%.14lg",ShowValSize);
			break; 
			   
        case 157:
        	itoa (WantOrthoResDisplay,OutStr,10);
        	break;
        case 158:
        	btoa (ComputeArea,OutStr);
        	break;
        case 159:
        	btoa (ShowBadSyms,OutStr);
        	break;
        case 160:
        	btoa (PickDeletes,OutStr);
        	break;
		case 161:
           	ltoa (ODBCRecNum,OutStr,10);
           	break; 
           	
        case 164:
           	itoa (PolyInfoTheme,OutStr,10);
           	break;
		case 165:
			itoa (ShortestRoute,OutStr,10);
		break;  
		case 167:
			sprintf (OutStr,"%.14lg %.14lg",TXLoc.x,TXLoc.y);
		break;  
   	    case 171:
   	    	itoa (GraphicsTextWeight,OutStr,10);
   	    break;
		case 172:
			sprintf (OutStr,"%.14lg",PenWidthFactor);
		break;
		case 173:
			itoa (VisListOpt,OutStr,10);
		break; 
        case 174:
        	btoa (AllowPickDeletes,OutStr);
        	break;
        case 178:
        	ltoa (ShowValColor,OutStr,10);
        	break;  
        case 179:
        	_fstrcpy (OutStr,NoCache);
        	break;
        case 180:
        	ltoa (DefaultTextColor,OutStr,10);
        	break; 
        case 181:
			sprintf (OutStr,"%.14lg",CurView->Height);
			break;
        case 184:
			{
				DWORD lUserName = 63;
				
				if (WNetGetUser (NULL,UserName,&lUserName) != NO_ERROR)
					*UserName = 0;
				strlwr (UserName);
        		strcpy (OutStr,UserName);
			}
        	break;
        case 185:
			{
				DWORD	lNodeName= MAX_COMPUTERNAME_LENGTH;

				*NodeName = 0;
				GetComputerName (NodeName,&lNodeName);
        		strcpy (OutStr,NodeName);
			}
        	break;
		case 186:
			btoa (InPlotView,OutStr);
			break;     
        case 188:
        	_fstrcpy (OutStr,GMVersion);
        	break;
        case 189:
        	ltoa ((long)NumSymbols,OutStr,10);
        	break;
        case 190:
        	GetTempDir (OutStr); 
        	break; 
		case 191:  
			btoa (CheckTimeStamps,OutStr);
           	break; 
		case 192:  
			btoa (KeepFilesOpen,OutStr);
           	break; 
        case 195:
        	btoa (UndoEnabled,OutStr);
        	break;
		case 196:
			itoa (DisplayRasterOpt,OutStr,10);
			break;
		case 197:  
			btoa (ShowDeletedOpt,OutStr);
           	break;  
        case 198: 
        {
			LPSTR CmdMess;  
			
			if (hCmdMess)
			{
				CmdMess = GlobalLock (hCmdMess);
				_fstrcpy (OutStr,CmdMess);
	       		GlobalUnlock (hCmdMess);
	       	}
	       	else
	       		*OutStr = 0;
	    }
	    	break; 
	    case 199:
	    	btoa (HaveBlockingWindow,OutStr);
	       	break;
	    case 200:
	    	btoa (StoreDynText,OutStr);
	    case 201:
	    	ltoa (TMPRF$,OutStr,10);
	       	break;
	    case 203:
	    	ltoa (NumOffsetFailed,OutStr,10);
	       	break;
	    case 204:
	    	ltoa (CurrentSHPRec,OutStr,10);
	       	break;
        case 205:
			sprintf (OutStr,"%.14lg",FTM);
			break;
        case 206:
			sprintf (OutStr,"%.14lg",MFT);
			break;
        case 207:
			sprintf (OutStr,"%lu",MSLink);
			break;
		case 208:  
			btoa (ThemeDisplayPass,OutStr);
           	break;  
        case 209:
        	Index = max (0,min(Index,MAXFONTS-1));
        	ltoa (FontColors[Index],OutStr,10);
        	break; 
		case 213:
			itoa (ShowSQLErrors,OutStr,10);
			break;                         
		case 214:
			_fstrcpy (OutStr,StartupDir);
			break;
		case 215:
//			_fstrcpy (OutStr,DLLDir);
			break;
		case 218:  
			btoa (PickPoints,OutStr);
           	break;
		case 220:  
			btoa (AllowCache,OutStr);
           	break; 
        case 221:
        	_fstrcpy (OutStr,MaskAreaFile);
        	break;  
		case 222:
			btoa (BackgroundTask,OutStr);
			break;                         
		case 223:
			btoa (HltAutoClear,OutStr);
			break;                         
		case 225:  
			itoa (CurrentDesc,OutStr,10);
           	break;
		case 226:  
			boundstoa (OutStr,&HLTBounds);
           	break; 
        case 228:   	
           	itoa (NumCachePathnameFrom,OutStr,10);
           	break;
		case 229:
			itoa (DisplayFailure,OutStr,10);
			break; 
		case 230:
			if (InGraphicsProcessor)
			{   
				if (CurrentType == GF_TEXT)
					CurTyp = CurrentTextBaseType;
			    if (CurTyp == GF_AREA)
			    {
					ftoa (OutStr,ComputeAreaAreaD (lpDCurPoints,nPnts,&Perimeter));  
				}
			} 
			break;
		case 231:
			if (InGraphicsProcessor)
			{
				if (CurrentType == GF_TEXT)
					CurTyp = CurrentTextBaseType;
			    if (CurTyp == GF_AREA)
			    {
			    	ComputeAreaAreaD (lpDCurPoints,nPnts,&Perimeter);
					ftoa (OutStr,Perimeter);  
				}
			}
			break;
		case 232:
			if (InGraphicsProcessor)
			{
				if (CurrentType == GF_TEXT)
					CurTyp = CurrentTextBaseType;
			    if (CurTyp == GF_LINE || CurTyp == GF_POLYLINE)
			    {
					ftoa (OutStr,GetPolyLengthD (lpDCurPoints,nPnts));  
				}
			}
			break;                        
		case 233:
			if (InGraphicsProcessor)
			{
				if (CurrentType == GF_TEXT)
					CurTyp = CurrentTextBaseType;
			    if (CurTyp == GF_LINE || CurTyp == GF_POLYLINE)
			    {
					ftoa (OutStr,getazd (lpDCurPoints,&lpDCurPoints[nPnts]));  
				}
			    if (CurTyp == GF_POINT)
			    {
					ftoa (OutStr,PTRot);  
				}
			}
			break;                        
		
        case 234:
            if (PRJ_UNITS[3] == 1)
				strcpy (OutStr,"Feet");
         	else if (PRJ_UNITS[3] == 2)
				strcpy (OutStr,"Meters");
			else
				*OutStr = 0;
        	break;
 
		case 237:
			btoa (BufferedScreen,OutStr);
			break;   
			                      
		case 239:
			btoa (HaveDestination,OutStr);
			break;   
			                      
		case 240:
			btoa (UseHollowStreets,OutStr);
			break;                         
        
		case 241:
			ltoa (CurrentVoterID,OutStr,10);
			break;                         
        
		case 242:
			ltoa (CurrentVoterDB,OutStr,10);
			break;                         
        
		case 243:
			btoa (AutoHighlight,OutStr);
			break;
			                         
		case 246:
			ftoa (OutStr,CompressAccuracy);
			break; 
			
		case 254:
			itoa (ShowShieldDir,OutStr,10);
			break;  
			                      
		case 258:
           	btoa (InPrintProcess,OutStr);
           	break;  
           	
        case 261: 
        	if (CurView)
        		ftoa (OutStr,CurView->ZMScale);
        	break;    
        case 262:
        	btoa (RandomAreasAreTransparent,OutStr);
        	break;
        case 263:
        	btoa (DisplayShields,OutStr);
        	break;
        case 266:
        	btoa (ShieldsOnly,OutStr);
        	break;
        case 273:
			if (NumMemRead + NumFidRead)
			{
				double pct=100*(((double)NumMemRead)/(NumMemRead + NumFidRead));
        		sprintf (OutStr,"%f %ld %ld %ld",pct,NumFileOpen,NumNullOpen,NumActualOpen);
				NumMemRead = NumFidRead = NumFileOpen = NumNullOpen = NumActualOpen = 0;
			}
			else
				strcpy (OutStr,"No io");
        	break;
        case 275:
			GetClientRect (GetDesktopWindow(),&Rect);
        	ltoa (Rect.right-Rect.left,OutStr,10);
        	break;
        case 276:
			GetClientRect (GetDesktopWindow(),&Rect);
        	ltoa (Rect.bottom-Rect.top,OutStr,10);
        	break;
        case 277:
			GetClientRect (hWndMain,&Rect);
        	ltoa (Rect.right-Rect.left,OutStr,10);
        	break;
        case 278:
			GetClientRect (hWndMain,&Rect);
        	ltoa (Rect.bottom-Rect.top,OutStr,10);
        	break;
 		case 279:
			ltoa (AutoOrthoColor,OutStr,10);
			break;  
        case 280:
        	btoa (AutoOpaque,OutStr);
        	break;
		case 281:
			ftoa (OutStr,RADDEG);
			break;
		case 282:
			ftoa (OutStr,DEGRAD);
			break;
        case 283:
			btoa (UseMappedFiles,OutStr);
			break;
		case 285:
			btoa (AlwaysUseZoomMacro,OutStr);
			break;
        case 289:
			btoa (BlockVehicleDisplay,OutStr);
			break;
		case 290:
			strcpy (OutStr,PltName);
			break;
		case 291:
			strcpy (OutStr,TraceVar);
			break;
		case 293:
	       	btoa (PickPointSymbol,OutStr);
			break;
		case 294:
	       	btoa (dbug,OutStr);
			break;
		case 295:
	       	btoa (DisplayPartialBuffer,OutStr);
			break;
		case 296:
			ltoa (debugvalue,OutStr,10);
			break;
		case 297:
			btoa (ExpandGrText,OutStr);
			break;
		case 298:
			ftoa (OutStr,ShieldFactor);
			break;
        case 299:
        	GetCacheDir (OutStr); 
        	break;
		case 300:
			ltoa (ReplayDelay,OutStr,10);
			break;
		case 302:
			ftoa (OutStr,PrinterWidth);
			break;
		case 303:
			ftoa (OutStr,PrinterHeight);
			break;
		case 304:
			ftoa (OutStr,PrinterMarginLeft);
			break;
		case 305:
			ftoa (OutStr,PrinterMarginRight);
			break;
		case 306:
			ftoa (OutStr,PrinterMarginTop);
			break;
		case 307:
			ftoa (OutStr,PrinterMarginBottom);
			break;
		case 308:
			ftoa (OutStr,StreetTextFactor);
			break;
		case 309:
			ftoa (OutStr,StreetWidthFactor);
			break;
		case 310:
			ftoa (OutStr,StreetWidth);
			break;
		case 311:
			ltoa (FillStreetWithThisColor,OutStr,10);
			break;
		case 312:
			ltoa (StreetOneWay,OutStr,10);
			break;
		case 313:
			ltoa (GMDMinCode,OutStr,10);
			break;
		case 314:
			ltoa (GMDMaxCode,OutStr,10);
			break;
		case 315:
			btoa (NoDisplay,OutStr);
			break;
		case 316:
			btoa (SaveContourElev,OutStr);
			break;
        case 317:
        	GetTempPath (MAX_PATH,OutStr); 
        	break; 
		case 321:
			ltoa (GridPixelWidth,OutStr,10);
			break;
		case 323:
			strcpy (OutStr,ShieldSaveFile);
			break;
		case 324:
			btoa (AllowJournal,OutStr);
			break;
		case 326:
			btoa (UseShortHighlightList,OutStr);
			break;
		case 327:
			ltoa (DepthColorIntensity,OutStr,10);
			break;
		case 328:
			ltoa (ShallowWaterHighlight,OutStr,10);
			break;
		case 329:
			ltoa (WaterLevelOffset,OutStr,10);
			break;
		case 331:
			strcpy (OutStr,AltAddressDir);
			break;
		case 332:
			strcpy (OutStr,GMDCodeList);
			break;
		case 333:
			btoa (UseHLTGraphicsFile,OutStr);
			break;
		case 334:
			strcpy (OutStr,AutoMoveFile);
			break;
		case 335:
			btoa (RunFromCache,OutStr);
			break;
		case 336:
			strcpy (OutStr,UserAddressSubDir);
			break;
		case 337:
			strcpy (OutStr,LogGPSInputFile);
			break;
		case 338:
			itoa ((int)CurrentServerSocket,OutStr,10);
			break;
		case 340:
			btoa (ConvertPickedItems,OutStr);
			break;
		case 341:
			strcpy (OutStr,CachePathnameTo);
			break;
		case 342:
			strcpy (OutStr,SymbolImage);
			break;
		case 343:
			itoa (VehicleTrackMaxTime,OutStr,10);
			break;
		case 344:
			btoa (AllowingSocketConnections,OutStr);
			break;
		case 345:
			btoa (UMIODebug,OutStr);
			break;
		case 346:
			btoa (DisplayOnlyHLT,OutStr);
			break;
		case 348:
			ftoa (OutStr,TotHLTLength);
			break;
		case 349:
			ftoa (OutStr,TotHLTPerim);
			break;
		case 350:
			ftoa (OutStr,TotHLTArea);
			break;
		case 351:
			itoa (CurrentGridLevel,OutStr,10);
			break;
		case 352:
			itoa (CurrentGridRow,OutStr,10);
			break;
		case 353:
			itoa (CurrentGridCol,OutStr,10);
			break;
		case 355:
			ftoa (OutStr,SFTSM);
			break;
		case 356:
			ftoa (OutStr,1.0e0/ACRSM);
			break;
		case 358:
			itoa (GetDeviceCaps(CurView->hDC, LOGPIXELSX),OutStr,10);
			break;
		case 359:
			strcpy (OutStr,CfgName);
			break;
		case 365:
			itoa (WantPNData,OutStr,10);
			break;
		case 366:
			strcpy (OutStr,TestFileLocation);
			break;
		case 367:
			GetWindowsVersion(OutStr);
			break;
		case 368:
			strcpy(OutStr, SQLErrorLog);
			break;
		case 369:
			OutStr[0] = literalChar;
			OutStr[1] = 0;
			break;
		case 370:
			ltoa(CurrentSHPRec, OutStr, 10);
			break;
		case 371:
			ltoa(CurrentDBFRec, OutStr, 10);
			break;

	}
	GlobalUnlock (hGlobal);
{
#if ENABLETRACE
GSSiExitProg (533);
#endif
	return (_fstrlen(OutStr)); 
}
#if ENABLETRACE
}
#endif
}

short Expandicmp (LPSTR String1, LPSTR String2)
#if ENABLETRACE
{GSSiEnterProg (534);
#endif
{
	HANDLE	h1=GSSiGlobAlloc ( 193,GMEM_MOVEABLE,4096);
	HANDLE	h2=GSSiGlobAlloc ( 194,GMEM_MOVEABLE,4096);
	LPSTR	pString1=GlobalLock (h1);
	LPSTR	pString2=GlobalLock (h2); 
	short	rtn;
	
	_fstrcpy (pString1,String1);
	ExpandText (pString1); 
	_fstrcpy (pString2,String2);
	ExpandText (pString2); 
	rtn = _fstricmp (pString1,pString2);
	GlobalUnlock (h1);
	GSSiGlobUlFree (&h1);
	GlobalUnlock (h2);
	GSSiGlobUlFree (&h2);
{
#if ENABLETRACE
GSSiExitProg (534);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

void SetGlobalValueReal (LPSTR Name, double val)
#if ENABLETRACE
{GSSiEnterProg (535);
#endif
{   char	txt[128];  

	sprintf (txt,"%.14lg",val);
	SetGlobalValue (Name,txt);
{
#if ENABLETRACE
GSSiExitProg (535);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void SetGlobalValueDPoint (LPSTR Name, DPOINT Point)
#if ENABLETRACE
{GSSiEnterProg (536);
#endif
{   char	txt[64];  

	sprintf (txt,"%.14lg %.14lg",Point.x,Point.y);
	SetGlobalValue (Name,txt);
{
#if ENABLETRACE
GSSiExitProg (536);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void SetGlobalValueBounds (LPSTR Name, LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (536);
#endif
{   char	txt[128];  

	sprintf (txt,"%.14lg %.14lg %.14lg %.14lg",pBounds->xmn,pBounds->ymn,pBounds->xmx,pBounds->ymx);
	SetGlobalValue (Name,txt);
{
#if ENABLETRACE
GSSiExitProg (536);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void SetGlobalValueHandle (LPSTR Name, HANDLE val)
#if ENABLETRACE
{GSSiEnterProg (537);
#endif
{   char	txt[16]; 
	HANDLE	handle; 

	ltoa ((long)val,txt,10);
	SetGlobalValue (Name,txt);

	if ((handle = FindVar(Name))) 
	{
		VARPNT	VarPnt = (VARPNT)GlobalLock(handle); 
		
		VarPnt->ValueIsHandle = TRUE;
		GlobalUnlock (handle);
	} 
{
#if ENABLETRACE
GSSiExitProg (537);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void SetGlobalValueLong (LPSTR Name, long val)
#if ENABLETRACE
{GSSiEnterProg (537);
#endif
{   char	txt[16]; 

	ltoa (val,txt,10);
	SetGlobalValue (Name,txt); 
	
{
#if ENABLETRACE
GSSiExitProg (537);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void SetGlobalValueBool (LPSTR Name, BOOL val)
#if ENABLETRACE
{GSSiEnterProg (538);
#endif
{   char	txt[32]; 
    
    if (val)
		SetGlobalValue (Name,"T");  
	else
		SetGlobalValue (Name,"F");  
{
#if ENABLETRACE
GSSiExitProg (538);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void SetGlobalValueRect (LPSTR Name, RECT Rect)
#if ENABLETRACE
{GSSiEnterProg (539);
#endif
{   char	txt[32]; 
    RECT	RectReverse=Rect;
    
    Rect.top = RectReverse.bottom;
    Rect.bottom = RectReverse.top;
	recttoa (txt,Rect);
	SetGlobalValue (Name,txt);
{
#if ENABLETRACE
GSSiExitProg (539);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void AddToChangedGlobalList (HANDLE handle)
{   
//keeps last 32 changed globals
	if (nChangedGlobals > 31)
		return;
	ChangedGlobals[nChangedGlobals++] = handle;
	return;
}

void SetVarChangeTimes (short opt)
{   
	if (opt) 
	{
		UINT	i;
		VARPNT	VP;
		
		for (i=0;i<nChangedGlobals;i++)
		{   
			VP = (VARPNT)GlobalLock (ChangedGlobals[i]);
			VP->changetime = NextVarTime();
			GlobalUnlock (ChangedGlobals[i]);
		}
	}
	else
		nChangedGlobals = 0;
	return;
}

void SetGlobalValueLen (LPSTR Name, LPSTR Value, short len)
#if ENABLETRACE
{GSSiEnterProg (540);
#endif
{   
	VARPNT	VP;
	LPSTR	EndChar;
	HANDLE	handle;
	
	handle = AllocateVar (Name);  
	VP = (VARPNT)GlobalLock (handle);
	if (VP->ValueIsHandle)
	{
		HANDLE handle = (HANDLE)atol (VP->Value);
		GSSiGlobFree (&handle); 
		VP->ValueIsHandle = 0;
	}
	len = min (len,MAXVARLEN);
	_fstrncpy (VP->Value,Value,len);
	EndChar = VP->Value + len;
	*EndChar = 0;
	VP->Len = _fstrlen (VP->Value);
	VP->changetime = NextVarTime();
	SetLinkedVarTime (VP); 
	GlobalUnlock (handle);  
	AddToChangedGlobalList (handle);
{
#if ENABLETRACE
GSSiExitProg (540);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}     

BOOL SetGlobalFromCheckBox (HWND hWndDlg,UINT Control,LPSTR VarName)
#if ENABLETRACE
{GSSiEnterProg (541);
#endif
{   
	BOOL	rtn;
	
	rtn = SendDlgItemMessage (hWndDlg,Control,BM_GETCHECK,0,0);
	SetGlobalValueBool (VarName,rtn);
{
#if ENABLETRACE
GSSiExitProg (541);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

int SetGlobalFromTextBox (HWND hWndDlg,UINT Control,LPSTR VarName,BOOL ConvertReturns)
#if ENABLETRACE
{GSSiEnterProg (541);
#endif
{   
	int	rtn;
	HANDLE	hText = GSSiGlobAlloc (0,GMEM_MOVEABLE,4098);
	LPSTR	pStr  = GlobalLock (hText);

	rtn = GetWindowText (GetDlgItem (hWndDlg,Control),pStr,4000);
	if (ConvertReturns)
	{
		HANDLE hCvt = GSSiGlobAlloc (0,GMEM_MOVEABLE,rtn*2+2);
		LPSTR	pCvt2, pCvt = pCvt2 = GlobalLock (hCvt);

		while (*pStr)
		{
			switch (*pStr)
			{
			case '\r':
				*pCvt++ = '\\';
				*pCvt++ = 'r';
				break;
			case '\n':
				*pCvt++ = '\\';
				*pCvt++ = 'n';
				break;
			default:
				*pCvt++ = *pStr;
				break;
			}
			*pStr++;
		}
		*pCvt = 0;
		SetGlobalValue (VarName,pCvt2);
		GSSiGlobUlFree (&hCvt);
	}
	else
		SetGlobalValue (VarName,pStr);
	GSSiGlobUlFree (&hText);
{
#if ENABLETRACE
GSSiExitProg (541);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

short ProcessDelimTextHeader(LPSTR INstr, LPSTR File, HFILE Fid, LPHANDLE phDLT, char InDelim, LPSTR IDName)
#if ENABLETRACE
{GSSiEnterProg (542);
#endif
{
	LPSTR	BeginLoc, EndLoc, EqLoc, DashLoc, TypeLoc; 
	char	EndChar, Delim=','; 
	BOOL	Done=FALSE;
	LPSHORT	nDLTvar;
	LPHANDLE	DLTVar; 
	LPSHORT	DLTStart,DLTLen,DLTType; 
	HANDLE	hHead = GSSiGlobAlloc ( 195,GMEM_MOVEABLE,USHRT_MAX);
	LPSTR	str = GlobalLock (hHead);  
	LPSTR	DLTDelim;     
	HFILE	FidHdr;
	char	varname[128];
    
    *phDLT = 0;
	if (File)
	{
    	char	HeaderName[256];
    	HFILE	FidHdr;  
    	LPSTR	pDot,pstr;
        	
    	_fstrcpy (HeaderName,File);
    	if ((pDot=_fstrrchr (HeaderName,'.')))
    		_fstrcpy (pDot,".hdr");
		FidHdr = GSSiOpenFile (HeaderName,0,OF_READ);
		if (FidHdr == HFILE_ERROR)
			FidHdr = Fid;  
		if (!fgetstring (str,USHRT_MAX-2,FidHdr))
			goto Exit; 
		pstr = str;
		while ((int)*pstr < 0) //what is this for?
			pstr++;
		_fstrcpy (INstr,pstr);
		if (FidHdr != Fid)
			GSSiClose (FidHdr);
	}	                                 
	_fstrcpy (str,INstr);
	nDLTvar = 0; 
	*phDLT = GSSiGlobAlloc ( 196,GHND,2 + 1 + MAXDLTVAR * (3*sizeof(short)+sizeof(HANDLE)));
	nDLTvar = (LPSHORT)GlobalLock (*phDLT);
	DLTDelim = (LPSTR)(nDLTvar + 1);
	DLTVar = (LPHANDLE)(DLTDelim + 1);
	DLTStart = (LPSHORT)(DLTVar + MAXDLTVAR);
	DLTLen = (LPSHORT)(DLTStart + MAXDLTVAR);
	DLTType = (LPSHORT)(DLTLen + MAXDLTVAR);

	if (InDelim)
		Delim = InDelim;
	else if (_fstrchr (str,'\t'))
		Delim = '\t';
	else if (_fstrchr (str,'|'))
		Delim = '|';
	else if (!_fstrchr (str,','))
	{
		if (_fstrchr (str,' '))
			Delim = ' '; 
	}  
	*DLTDelim = Delim;
Next:if (*str == '"')
	{
		str++;
		EndChar = '"';
	}
	else
		EndChar = Delim;
	BeginLoc = str;
	EndLoc = _fstrchr(str,EndChar); 
	if (!EndLoc)
	{
		Done=TRUE;
		EndLoc = _fstrchr (str,0); 
	}
	*EndLoc = '\0';
	EqLoc = _fstrchr(str,'='); 
	TypeLoc = _fstrchr(str,'(');
	DLTType[*nDLTvar] = BT_CHAR;
	if (TypeLoc)
	{
		*TypeLoc++ = 0;
 		switch (*TypeLoc)
 		{
	 		case 'C':
				DLTType[*nDLTvar] = BT_CHAR;
			break;
			case 'B':
				DLTType[*nDLTvar] = BT_INTEGER; 
			break;
			case 'R':
				DLTType[*nDLTvar] = BT_REAL;
			break;
			case 'J':
				DLTType[*nDLTvar] = BT_RIGHT_CHAR;
			break; 
			default:
			break;
		}
		TypeLoc++;
		DLTLen[*nDLTvar] = atoi(TypeLoc);
	}
	if (EqLoc)
	{
		*EqLoc = '\0';
		EqLoc++; 
		DashLoc = _fstrchr(EqLoc,'-');
		if (!DashLoc)
			goto Exit;
		*DashLoc = '\0';
		DashLoc++;
		DLTStart[*nDLTvar] = atoi (EqLoc)-1;
		DLTLen[*nDLTvar] = atoi (DashLoc)-DLTStart[*nDLTvar];
		if (DLTStart[*nDLTvar]<0 || DLTLen[*nDLTvar]<0) 
			goto Exit;
	}
	else
		DLTStart[*nDLTvar]=-1;	
	if (IDName && *IDName)
		sprintf(varname, "%s.%s", IDName, BeginLoc);
	else
		strcpy (varname,BeginLoc);
	REPLAC (varname,"[","(",128);
	REPLAC (varname,"]",")",128);
	DLTVar[(*nDLTvar)++] = AllocateVar(varname);  
	if (Done)
	{   
		short	n=*nDLTvar;
		
		if (!n)
			goto Exit;
		if (Delim == '\t')
			*nDLTvar *= -1;
		GlobalUnlock (*phDLT); 
		GSSiGlobUlFree (&hHead);
{
#if ENABLETRACE
GSSiExitProg (542);
#endif
		return(n);
}
	}
	str = ++EndLoc;
	if (!*str)
	{   
		short	n=*nDLTvar;
		
		if (!n)
			goto Exit;
		if (Delim == '\t')
			*nDLTvar *= -1;
		GlobalUnlock (*phDLT);
		GSSiGlobUlFree (&hHead);
{
#if ENABLETRACE
GSSiExitProg (542);
#endif
		return(n);
}
	}
	if (*str == Delim) 
		str++;
	goto Next; 
Exit:
	GSSiGlobUlFree (phDLT);
	GSSiGlobUlFree (&hHead);
{
#if ENABLETRACE
GSSiExitProg (542);
#endif
	return 0;	
}
#if ENABLETRACE
}
#endif
}  

BOOL GetDelimTextData(LPSTR str,HANDLE hDLT)
#if ENABLETRACE
{GSSiEnterProg (544);
#endif
{
	LPSTR	BeginLoc, EndLoc,DLTDelim, LastLoc=strchr (str,0);
	char	Delim=',',EndStr[3]; 
	int		ivar=0, LineLen,l, EndInc; 
	VARPNT	VarPtr;
	LPSHORT	pnDLTvar;
	short	nDLTvar;
	LPHANDLE	DLTVar; 
	LPSHORT	DLTStart,DLTLen,DLTType;
	
	pnDLTvar = (LPSHORT)GlobalLock (hDLT); 
	nDLTvar = abs (*pnDLTvar);  
	DLTDelim = (LPSTR)(pnDLTvar + 1);
	DLTVar = (LPHANDLE)(DLTDelim + 1);
	DLTStart = (LPSHORT)(DLTVar + MAXDLTVAR);
	DLTLen = (LPSHORT)(DLTStart + MAXDLTVAR);
	DLTType = (LPSHORT)(DLTLen + MAXDLTVAR);
	
	Delim = *DLTDelim; 
	for (ivar=0;ivar<nDLTvar;ivar++)
	{   
		VarPtr = (VARPNT)GlobalLock (DLTVar[ivar]);
		VarPtr->Len = 0;    
		*VarPtr->Value = 0;
		VarPtr->changetime = NextVarTime();
		SetLinkedVarTime (VarPtr); 
		if (nDLTvar == 1)
		{
			VarPtr->Len = strlen(str);
			strncpy0(VarPtr->Value, str, VarPtr->Len);
		}
		GlobalUnlock(DLTVar[ivar]);
		AddToChangedGlobalList (DLTVar[ivar]);
	} 
	if (nDLTvar == 1)
		goto RtnTrue;
	ivar = 0;
	if (DLTStart[0]>=0)
	{   
		LineLen = _fstrlen(str);
		for (ivar=nDLTvar-1;ivar>=0;ivar--)
		{   
			VarPtr = (VARPNT)GlobalLock (DLTVar[ivar]);
			if (DLTStart[ivar]<LineLen)
			{   
				char	EndChar;
				
				if (VarPtr->ValueIsHandle)
				{
					HANDLE handle = (HANDLE)atol (VarPtr->Value);
					GSSiGlobFree (&handle); 
					VarPtr->ValueIsHandle = 0;
				}
				BeginLoc = str+DLTStart[ivar];
				EndLoc = BeginLoc + DLTLen[ivar];
				EndChar = *EndLoc;
				*EndLoc = '\0';
				l = min (strlen(BeginLoc),MAXVARLEN);
				VarPtr->Len = min (l,DLTLen[ivar]);
				strncpy0 (VarPtr->Value,BeginLoc,VarPtr->Len); 
				*EndLoc = EndChar; 
			}
			GlobalUnlock (DLTVar[ivar]);
		} 
		goto RtnTrue;
	}	 
Next:if (*str == '"')
	{
		str++;
		EndStr[0] = '"';
		EndStr[1] = Delim;
		EndStr[2] = 0; 
		EndInc = 1;
	}
	else 
	{
		EndStr[0] = Delim;
		EndStr[1] = 0;
		EndInc = 0;
	}
	BeginLoc = str;
	EndLoc = _fstrstr(str,EndStr);
	if (EndLoc)
		*EndLoc = '\0';
	else if (*EndStr = '"')
	{
		if ((EndLoc = _fstrrchr (str,'"')))
			*EndLoc = 0;
	}
	VarPtr = (VARPNT)GlobalLock (DLTVar[ivar]);
	if (VarPtr->ValueIsHandle)
	{
		HANDLE handle = (HANDLE)atol (VarPtr->Value);
		GSSiGlobFree (&handle); 
		VarPtr->ValueIsHandle = 0;
	}
	strncpy0 (VarPtr->Value,BeginLoc,MAXVARLEN); 
	VarPtr->Len = _fstrlen(BeginLoc); 
	GlobalUnlock (DLTVar[ivar]);
	if (!EndLoc || EndLoc >= LastLoc-1)
		goto RtnTrue;
	str = EndLoc + 1 + EndInc; 
	if (!*str)
		goto RtnTrue;
	ivar++;
	if (ivar >= nDLTvar)
		goto RtnTrue;
	goto Next;
	
RtnTrue:
	GlobalUnlock (hDLT);  
{
#if ENABLETRACE
GSSiExitProg (544);
#endif
	return TRUE;
}
RtnFalse:
	GlobalUnlock (hDLT);  
{
#if ENABLETRACE
GSSiExitProg (544);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}  

HANDLE	GetDLTVarHandle (int ivar,HANDLE hDLT)
#if ENABLETRACE
{GSSiEnterProg (545);
#endif
{   
	HANDLE	rtn=0;
	
	LPSHORT	pnDLTvar;
	LPHANDLE	DLTVar; 
	LPSHORT	DLTStart,DLTLen,DLTType;    
	LPSTR	DLTDelim;
	
	pnDLTvar = (LPSHORT)GlobalLock (hDLT); 
	DLTDelim = (LPSTR)(pnDLTvar + 1);
	DLTVar = (LPHANDLE)(DLTDelim + 1);
	DLTStart = (LPSHORT)(DLTVar + MAXDLTVAR);
	DLTLen = (LPSHORT)(DLTStart + MAXDLTVAR);  
	DLTType = (LPSHORT)(DLTLen + MAXDLTVAR);  
	rtn = DLTVar[ivar];
	GlobalUnlock (hDLT);
{
#if ENABLETRACE
GSSiExitProg (545);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL GetDLTVarName (int ivar,HANDLE hDLT, LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (546);
#endif
{
	VARPNT	VarPtr;
	LPSHORT	pnDLTvar; 
	short	nDLTvar;
	LPHANDLE	DLTVar; 
	LPSHORT	DLTStart,DLTLen; 
	LPSTR	DLTDelim;
	
	pnDLTvar = (LPSHORT)GlobalLock (hDLT);  
	nDLTvar = abs (*pnDLTvar);
	DLTDelim = (LPSTR)(pnDLTvar + 1);
	DLTVar = (LPHANDLE)(DLTDelim + 1);
	if (ivar > nDLTvar)
	{
		GlobalUnlock (hDLT);
{
#if ENABLETRACE
GSSiExitProg (546);
#endif
		return FALSE;
}
	}
	ivar--;
	VarPtr = (VARPNT)GlobalLock (DLTVar[ivar]);
	_fstrcpy (Name,VarPtr->Name); 
	GlobalUnlock (DLTVar[ivar]);
	GlobalUnlock (hDLT);
{
#if ENABLETRACE
GSSiExitProg (546);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void SetVarSaveStatus (LPSTR Name,BOOL Save)
#if ENABLETRACE
{GSSiEnterProg (547);
#endif
{   VARPNT  VarPnt;   
	HANDLE	handle;

	handle = AllocateVar (Name);
	VarPnt = (VARPNT)GlobalLock(handle);  
	VarPnt->Save = Save;
	GlobalUnlock (handle);
{
#if ENABLETRACE
GSSiExitProg (547);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void UpdateVarHandle (HANDLE OldHandle,HANDLE NewHandle)
#if ENABLETRACE
{GSSiEnterProg (548);
#endif
{   
	UINT	i,l	;
	VARPNT  VarPnt;
	
	for (i=0;i<pVarSpace->NumVars;i++)
	{   
		if (pVarSpace->VarHandles[i] == OldHandle)
			pVarSpace->VarHandles[i] = NewHandle;
		else
		{
			VarPnt = (VARPNT)GlobalLock(pVarSpace->VarHandles[i]);
			l=VarPnt->NumLinkedVars;  
			while (l--)
			{
				if (VarPnt->LinkedVar[l] == OldHandle)
					VarPnt->LinkedVar[l] = NewHandle;
			}
			GlobalUnlock(pVarSpace->VarHandles[i]);
		}
	}
{
#if ENABLETRACE
GSSiExitProg (548);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL GetVarSaveStatus (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (549);
#endif
{   VARPNT  VarPnt;   
	HANDLE	handle; 
	BOOL	Save;
	short	ii;

	if (!(handle = FindVar(Name)))
{
#if ENABLETRACE
GSSiExitProg (549);
#endif
		return TRUE;
}
	VarPnt = (VARPNT)GlobalLock(handle);  
	Save = VarPnt->Save;
	GlobalUnlock (handle);
	if (!Save)
		ii=1;
{
#if ENABLETRACE
GSSiExitProg (549);
#endif
	return Save;
}
#if ENABLETRACE
}
#endif
}

HANDLE	AllocateVar (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (550);
#endif
{   VARPNT  VarPnt;   
	HANDLE	handle;
	HANDLE	hVarSpace;

	if (handle = FindVar(Name))
{
#if ENABLETRACE
GSSiExitProg (550);
#endif
		return (handle);
}
	
	AddToVarNameTable (Name);
	hVarSpace = SetVarSpaceFromName(Name);
	if (!hVarSpace)
	{
#if ENABLETRACE
		GSSiExitProg(587);
#endif
		return (NULL);
	}
	pVarSpace = GlobalLock(hVarSpace);
	handle = GSSiGlobAlloc(197, GHND, sizeof(VARINFO));
	pVarSpace->VarHandles[pVarSpace->NumVars++] = handle;
	VarPnt = (VARPNT)GlobalLock(handle);  
	VarPnt->Handle = handle;
	_fstrcpy(VarPnt->Name,Name);   
	VarPnt->Save = TRUE;//(*Name != '%'); 2010 01-20 to save street name and width vars for Mpls
	GlobalUnlock (handle);
	GlobalUnlock(hVarSpace);
{
#if ENABLETRACE
GSSiExitProg (550);
#endif
	return (handle);
}

#if ENABLETRACE
}
#endif
}
 
void SetIntRefno (long Ref)
#if ENABLETRACE
{GSSiEnterProg (551);
#endif
{
	VARPNT  VarPnt;  
	
   	InternalRefno = Ref; 
   	if (hIntRefno)
   	{
		VarPnt = (VARPNT)GlobalLock (hIntRefno);
		VarPnt->changetime = NextVarTime();
		SetLinkedVarTime (VarPnt); 
        GlobalUnlock (hIntRefno);
		AddToChangedGlobalList (hIntRefno);
    }
{
#if ENABLETRACE
GSSiExitProg (551);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void SetSymNum (int SymNum)
#if ENABLETRACE
{GSSiEnterProg (551);
#endif
{
	VARPNT  VarPnt;  
	
   	CurrentDesc = SymNum; 
   	if (hSymNum)
   	{
		VarPnt = (VARPNT)GlobalLock (hSymNum);
		VarPnt->changetime = NextVarTime();
		SetLinkedVarTime (VarPnt); 
        GlobalUnlock (hSymNum);
		AddToChangedGlobalList (hSymNum);
    }
{
#if ENABLETRACE
GSSiExitProg (551);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

void AddToVarNameTable (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (552);
#endif
{   
	LPVARNAMEINDEXITEM	lpVN;  
	UINT	i;
	HANDLE	hVarSpace=SetVarSpaceFromName(Name);
	
	if (!hVarSpace)
	{
#if ENABLETRACE
		GSSiExitProg(552);
#endif
		return;
	}
	pVarSpace = GlobalLock(hVarSpace);
	if (pVarSpace->NumVars >= pVarSpace->MaxVars)
		BlowOut ("Maximum globals exceeded",0);
	if (_fstrlen (Name) > 61)
		GSSiMsgBox (GetFocus(),"Length of variable name exceeds 61 characters",Name,MB_ICONEXCLAMATION,0);
	if (!pVarSpace->hVarNameTable)
		pVarSpace->hVarNameTable = GSSiGlobAlloc(198, GMEM_MOVEABLE, (long)sizeof(VARNAMEINDEXITEM)*pVarSpace->MaxVars);
	lpVN = (LPVARNAMEINDEXITEM)GlobalLock(pVarSpace->hVarNameTable);
	for (i = 0; i<pVarSpace->NumVars; i++, lpVN++)
	{
		if (_fstricmp (Name,lpVN->Name) < 0)
		{   
			_fmemmove((LPVOID)(lpVN + 1), (LPVOID)lpVN, ((USHORT)pVarSpace->NumVars - i)*sizeof(VARNAMEINDEXITEM));
			goto Exit;
		}
	}
Exit:
	strncpy0 (lpVN->Name,Name,62);
	lpVN->id = pVarSpace->NumVars;
	GlobalUnlock(pVarSpace->hVarNameTable);
	GlobalUnlock (hVarSpace);
{
#if ENABLETRACE
GSSiExitProg (552);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
 
void DestroyVarNameTable (void)
#if ENABLETRACE
{GSSiEnterProg (553);
#endif
{  
	GSSiGlobFree(&pVarSpace->hVarNameTable);
{
#if ENABLETRACE
GSSiExitProg (553);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}        		

HANDLE	AllocateTypeVar (LPSTR Name,short Type,BOOL Save)
#if ENABLETRACE
{GSSiEnterProg (554);
#endif
{   VARPNT  VarPnt;   
	HANDLE	handle;

	if (handle = FindVar(Name))
	{
		VarPnt = (VARPNT)GlobalLock(handle);  
		if (VarPnt->Type != Type)
			VarPnt->Type = Type;
		VarPnt->Save = Save;
		GlobalUnlock (handle);
{
#if ENABLETRACE
GSSiExitProg (554);
#endif
		return (handle);
}
	}
	AddToVarNameTable (Name);
	
	handle = GSSiGlobAlloc ( 199,GHND,sizeof(VARINFO));
	pVarSpace->VarHandles[pVarSpace->NumVars++] = handle;
	VarPnt = (VARPNT)GlobalLock(handle);  
	VarPnt->Type = Type; 
	VarPnt->Save = Save;
	VarPnt->Handle = handle;
	_fstrcpy(VarPnt->Name,Name);
	GlobalUnlock (handle);
{
#if ENABLETRACE
GSSiExitProg (554);
#endif
	return (handle);
}

#if ENABLETRACE
}
#endif
}
void NextPage (HDC hPr)
#if ENABLETRACE
{GSSiEnterProg (555);
#endif
{   
	PrintReportPage++; 
	if (Printing)
	{
		EndPage (hPr);
		StartPage (hPr);  
	}

{
#if ENABLETRACE
GSSiExitProg (555);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

void ProcessTextDB(LPSTR InText, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen)
#if ENABLETRACE
{
	GSSiEnterProg(556);
#endif
	{
		if (*InText)
		{
			HANDLE	handle = GSSiGlobAlloc(200, GMEM_MOVEABLE, (long)4096 * 4);
			LPSTR	Text = GlobalLock(handle);

			_fstrcpy(Text, InText);
			ExpandTextDB(Text,pBrkPt, bpOffset, bpLen);
			GSSiGlobUlFree(&handle);
		}
{
#if ENABLETRACE
	GSSiExitProg(556);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ProcessText (LPSTR InText)
#if ENABLETRACE
{GSSiEnterProg (556);
#endif
{
	if (*InText)
	{
		HANDLE	handle=GSSiGlobAlloc ( 200,GMEM_MOVEABLE,(long)4096*4);
		LPSTR	Text=GlobalLock (handle);
		
		_fstrcpy (Text,InText);
		ExpandText (Text);
		GSSiGlobUlFree (&handle);
	}
{
#if ENABLETRACE
GSSiExitProg (556);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ProcessGlobal (LPSTR Global)
#if ENABLETRACE
{GSSiEnterProg (557);
#endif
{
	HANDLE	handle=GSSiGlobAlloc ( 201,GMEM_MOVEABLE,(long)4096);
	LPSTR	str=GlobalLock (handle);
	
	GetGlobalCVal (Global,str,0);
	ExpandText (str);
	GSSiGlobUlFree (&handle);
{
#if ENABLETRACE
GSSiExitProg (557);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

HANDLE ProcessIF (LPSTR InLoc,LPBOOL pErr)
{
	long	lIF, lTHEN,lELSE;  
	LPSTR	pEndIF, pStartIF=InLoc;
	LPSTR	pIF, pIF2, pTHEN, pEnd, OutLoc;
	HANDLE	hIF, hMem=0;
	BOOL	IfRtn;
	
	*pErr = 0;			
	pEndIF = _fstrchr (InLoc,0);
	InLoc += 3;
	pEnd = MatchLev (InLoc,')');
	if (!pEnd)
		goto IfError; 
	pIF = InLoc;
	lIF = pEnd++ - InLoc; 
	if (!(pEnd = FirstNonBlank(pEnd)))
		goto IfError;
	if (!_fstrncmp (pEnd,"THEN",4))
		pEnd+=4;
	if (*pEnd != '{')
		goto IfError;   
	InLoc = pEnd;
	InLoc++;
	pTHEN = InLoc;
	pEnd = MatchLev (InLoc,'}');
	if (!pEnd)
		goto IfError; 
	lTHEN = pEnd++ - pTHEN;
	InLoc = pEnd;
	hIF = GSSiGlobAlloc ( 202,GMEM_MOVEABLE,4096);
	pIF2 = GlobalLock (hIF);
	strncpy0 (pIF2,pIF,(size_t)lIF); 
	IfRtn = LogicP (pIF2,pErr); 
	GSSiGlobUlFree (&hIF);
	if (*pErr)
		goto IfError; 
	if (IfRtn)
	{   
		hMem = GSSiGlobAlloc ( 203,GMEM_MOVEABLE,lTHEN+1);
		OutLoc = GlobalLock(hMem);
		strncpy0 (OutLoc,pTHEN,(size_t)lTHEN);
		GlobalUnlock (hMem);
	}
	else
	{
		InLoc = FirstNonBlank(InLoc);
		if (!_fstrncmp (InLoc,"ELSE",4))
		{
			InLoc += 4;
			lELSE = pEndIF - InLoc;
			if (lELSE)
			{
				hMem = GSSiGlobAlloc ( 204,GMEM_MOVEABLE,lELSE+1);
				OutLoc = GlobalLock(hMem);
				strncpy0 (OutLoc,InLoc,(size_t)lELSE);
				GlobalUnlock (hMem);
			}
		}	
	}
	return hMem;
IfError:
	*pErr = 1;
	return hMem;
}

LPSTR ExpandText2 (LPSTR InText)
{
	ExpandText (InText);
	return InText;
}

BOOL FAR PASCAL MESSAGEBOXHALTMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
	static	HANDLE	hSaveBM=0;	

 switch(Message)
   {
    case WM_INITDIALOG:  
    
    	 hSaveBM = EnterBlockingWindow (hWndDlg);
		 SetWindowText (hWndDlg,MBHTitle);
		 SetDlgItemText (hWndDlg,IDC_MESSAGE,MBHMess);
         cwCenter(hWndDlg, 0);
			
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                //GSSiEndDialog(hWndDlg, FALSE,IDCANCEL); 
				GSSiEndDialog(hWndDlg, IDCANCEL,hSaveBM);
            break; 
            
            case IDYES: 
//                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
				GSSiEndDialog(hWndDlg, IDYES,hSaveBM);
            case IDNO: 
//                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
				GSSiEndDialog(hWndDlg, IDNO,hSaveBM);
            break;
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 


UINT MessageBoxHalt (HWND hWnd,LPSTR Mess,LPSTR Title,UINT Flags)
{
	UINT rtn;

	MBHMess = Mess;
	MBHTitle = Title;
	rtn = DialogBox(hInst, (LPSTR)"MESSAGEBOXHALT", hWnd, MESSAGEBOXHALTMsgProc);

	return rtn;
}
LPSTR ExpandTextDB (LPSTR InText,LPBREAKPOINT pBrkPt,int bpOffset,int bpLen)
#if ENABLETRACE
{GSSiEnterProg (558);
#endif
{
	HANDLE	hMem=0, hMemTrace;
	LPSTR	InLoc, OutLoc, NewText, BegBrack, EndBrack, loc, EqLoc, pLchr;
	LPSTR	pEnd, pStr, startLoc = InText;
	long	l,ii;
	BOOL	FoundLit=FALSE, SaveIE=InExpand, ExpandTrace=FALSE;
	int		loopBPOffset=0, whileBpOffset=0;
	int		elseBpOffset=0, thenBpOffset=0;
	
	InExpand = TRUE;
	if (TraceOn)
	{   
		hMem=GSSiGlobAlloc ( 205,GMEM_MOVEABLE,USHRT_MAX);
		pStr = GlobalLock (hMem);
		sprintf (pStr,"EXPIN:%s",InText);
		GSSiTraceLev (pStr,1,1);
		GSSiGlobUlFree (&hMem);
	}
	if (*TraceString && !strnicmp (InText,TraceString,strlen(TraceString)))
	{
		switch (MessageBoxHalt (hWndMain,InText,"Expand Trace?",MB_YESNOCANCEL|MB_APPLMODAL))
		{
		case IDYES:
			ExpandTrace = TRUE;
			break;
		case IDNO:
			break;
		case IDCANCEL:
			*TraceString = 0;
			break;
		}
	}
	InLoc = InText;   
	if (pBrkPt) breakAtPos(InLoc - startLoc, pBrkPt, bpOffset, bpLen,BA_EXPANDTEXT);
	if (*InLoc == '{')
	{   
		LPSTR	lc = LastChr (InLoc);
		if (pBrkPt) breakAtPos(InLoc - startLoc, pBrkPt, bpOffset, bpLen, BA_BEGINBLOCK);
		if (*lc != '}')
			goto OutChar; 
		hMem = GSSiGlobAlloc ( 206,GMEM_MOVEABLE,USHRT_MAX);
		NewText = GlobalLock(hMem); 
		*lc = 0;
		_fstrcpy (NewText,(InLoc+1));
		if (pBrkPt)
			lc = ExpandTextDB(NewText, pBrkPt,bpOffset + (int)((InLoc + 1) - startLoc),bpLen);
		else
			lc = ExpandText(NewText);
		_fstrcpy (InText,NewText);
		GSSiGlobUlFree (&hMem);
		if (ExpandTrace)
			MessageBoxHalt (hWndMain,InText,"Expanded",MB_OK|MB_APPLMODAL);
{
#if ENABLETRACE
GSSiExitProg (558);
#endif
		return InText;
}
	}
	while (*InLoc && ContinueProcessing)
	{
		if (pBrkPt) breakAtPos(InLoc - startLoc, pBrkPt, bpOffset, bpLen, BA_NEXTPOS);
		if (*InLoc == literalChar) /* literal */
		{
			if (!hMem)
			{
				hMem = GSSiGlobAlloc ( 207,GMEM_MOVEABLE,USHRT_MAX);
				NewText = GlobalLock(hMem);
				l = (InLoc - InText);
				if (l>0) _fmemmove(NewText,InText,(size_t) l);
				OutLoc = NewText + l;
			}
			InLoc++;
			*OutLoc++ = *InLoc;
			if (*InLoc && *InLoc != literalChar)
				InLoc++;
			FoundLit = TRUE;
		}
		else if (*InLoc == '[')
		{   
			if (!(EndBrack = MatchLev((LPSTR)(InLoc + 1), ']')))
			{
				if (strnicmp (InLoc+1,"$BOOL",5) && GetDebug())
					MessageBox(0, InLoc, "No matching bracket", MB_ICONEXCLAMATION);
				goto OutChar;
			}
			if (!hMem)
			{
				hMem = GSSiGlobAlloc ( 208,GMEM_MOVEABLE,USHRT_MAX);
				NewText = GlobalLock(hMem);
				l = (InLoc - InText);
				if (l>0) _fmemmove(NewText,InText,(size_t) l);
				OutLoc = NewText + l;
			}
			EqLoc = EndBrack;
			EqLoc++;
			if (*EqLoc=='=')
			{   HANDLE	hMemEq;
				LPSTR	EqText, SemiCLoc, VName; 
				BOOL	FoundSemiC=FALSE;
			    
			    VName = InLoc;
			    VName++;
			    *EndBrack = '\0';
			    SemiCLoc = MatchLev (EqLoc,';');
			    if (!SemiCLoc)
			    	SemiCLoc = _fstrchr (EqLoc,'\0'); 
			    else
			    	FoundSemiC=TRUE;
			    *SemiCLoc = '\0';
				hMemEq = GSSiGlobAlloc ( 209,GMEM_MOVEABLE,USHRT_MAX);
				EqText = GlobalLock(hMemEq);
				l = (SemiCLoc - EqLoc);
				if (l > 0)
				{
					_fmemmove(EqText, ++EqLoc, (size_t)l);
					if (LinkToVar)
					{
						if (pBrkPt)
							ExpandTextDB(EqText, pBrkPt, bpOffset + (int)(EqLoc - startLoc), bpLen);
						else
							ExpandText(EqText);
					}
					else
					{
						SetGlobalValue4(VName, EqText, FALSE, pBrkPt, bpOffset + (int)(EqLoc - startLoc),bpLen);
						if (!ContinueProcessing && DisplayFailure)
						{
							if (DisplayFailure == 2)
								DisplayFailure = 0;
							MessageBox(hWndMain, EqLoc, "Statement failed", MB_OK | MB_APPLMODAL);
						}
					}
				}
				GSSiGlobUlFree (&hMemEq);
				InLoc = SemiCLoc;  
				if (FoundSemiC)
					InLoc++;
			}
			else
			{
				LPSTR il;

				l = EndBrack - InLoc - 1; 
				InLoc++;
				_fmemmove(OutLoc,InLoc,(size_t) l);
				il = InLoc;
				InLoc = ++EndBrack;
				loc = OutLoc + l;
				*loc = '\0';
				if (pBrkPt)
					loc = ExpandTextDB(OutLoc, pBrkPt, bpOffset + (int)(il - startLoc), bpLen);
				else
					loc = ExpandText(OutLoc);
			    l = GetVal(OutLoc,OutLoc);
			    OutLoc+=l;
			}
		}
		else if (!_fstrncmp (InLoc,"WHILE(",6))
		{
			long	lWhile, lLoop, nLoops=-1;  
			LPSTR	pWhile, pWhile2, pLoop;
			HANDLE	hLoop, hWhile, hStr;
				
			InLoc += 6;
			pEnd = MatchLev (InLoc,')');
			if (!pEnd)
			{
				if (GetDebug())
					MessageBox(0, InLoc, "Error in WHILE", MB_ICONEXCLAMATION);
				goto WhileError;
			}
			pWhile = InLoc;
			lWhile = pEnd++ - InLoc; 
			if (*pEnd != '{')
			{
				if (GetDebug())
					MessageBox(0,InLoc, "Error in WHILE", MB_ICONEXCLAMATION);
				goto WhileError;
			}
			InLoc = pEnd;
			InLoc++;
			pEnd = MatchLev (InLoc,'}');
			if (!pEnd)
			{
				if (GetDebug())
					MessageBox(0,InLoc, "Error in WHILE", MB_ICONEXCLAMATION);
				goto WhileError;
			}
			lLoop = pEnd++ - InLoc;
			hLoop = GSSiGlobAlloc(210, GHND, lLoop + 1);
			pLoop = GlobalLock(hLoop);
			if (pBrkPt)
				loopBPOffset = bpOffset + (int)(InLoc - startLoc);

			_fstrncpy(pLoop, InLoc, lLoop);
			GlobalUnlock (hLoop);
			InLoc = pEnd;
			hWhile = GSSiGlobAlloc ( 211,GHND,lWhile+1);
			pWhile2 = GlobalLock (hWhile);
			_fstrncpy (pWhile2,pWhile,(size_t)lWhile); 
			GlobalUnlock (hWhile); 
			if (pBrkPt)
				whileBpOffset = bpOffset + (int)(pWhile - startLoc);
			hStr = GSSiGlobAlloc(212, GMEM_MOVEABLE, USHRT_MAX);
			pStr = GlobalLock (hStr); 
			nLoops = 0;
	NextWhileLoop: 
			if (ContinueProcessing)
			{
				BOOL	Rtn;
				BOOL	rc;
				
				pWhile = GlobalLock (hWhile);
				_fstrcpy (pStr,pWhile);
				GlobalUnlock (hWhile);
				Rtn = LogicPBP(pStr, &rc, pBrkPt,whileBpOffset,bpLen);
				if (Rtn && !rc)
				{
					pLoop = GlobalLock (hLoop);
					_fstrcpy (pStr,pLoop);
					GlobalUnlock (hLoop);
					ExpandTextDB(pStr, pBrkPt, loopBPOffset, bpLen);
					if (ContinueProcessing) 
					{
						nLoops++;
						goto NextWhileLoop;    
					}
				}  
			}
			GSSiGlobUlFree (&hStr);
			GSSiGlobFree (&hLoop);
			GSSiGlobFree (&hWhile);
	WhileError:
			if (!hMem)
			{
				hMem = GSSiGlobAlloc ( 213,GMEM_MOVEABLE,USHRT_MAX);
				NewText = GlobalLock(hMem);
				l = (InLoc - InText);
				if (l>0) _fmemmove(NewText,InText,(size_t) l);
				OutLoc = NewText + l;
			}
			ltoa (nLoops,OutLoc,10);
			OutLoc = _fstrchr (OutLoc,0);
		}
		else if (!_fstrncmp (InLoc,"IF(",3))
		{
			long	lIF, lTHEN;  
			LPSTR	pEndIF, pStartIF=InLoc;
			LPSTR	pIF, pIF2, pTHEN, pELSE;
			int		lIFBP = 0, lIFOffset;
			HANDLE	hIFBP = 0;
			HANDLE	hIF;
			BOOL	IfRtn, IfOK = FALSE;
			BOOL	rc;
				
			if (!(pEndIF = MatchLev (InLoc,';')))
				pEndIF = _fstrchr (InLoc,0);
			InLoc += 3;
			pEnd = MatchLev (InLoc,')');
			if (!pEnd)
			{
				if (GetDebug())
					MessageBox(0, InLoc, "Error in IF conditional statement", MB_ICONEXCLAMATION);
				goto IfError;
			}
			pIF = InLoc;
			lIF = pEnd++ - InLoc; 
			if (!(pEnd = FirstNonBlank(pEnd)))
				goto IfError;
			if (!_fstrncmp (pEnd,"THEN",4))
				pEnd+=4;
			if (*pEnd != '{')
			{
				MessageBox(0, InLoc, "Missing opening brace in IF statement", MB_ICONEXCLAMATION);
				goto IfError;
			}
			InLoc = pEnd;
			InLoc++;
			pTHEN = InLoc;
			pEnd = MatchLev (InLoc,'}');
			if (!pEnd)
			{
				if (GetDebug())
					MessageBox(0, InLoc, "Missing matching brace in IF statement", MB_ICONEXCLAMATION);
				goto IfError;
			}
			lTHEN = pEnd++ - pTHEN;
			InLoc = pEnd;
			hIF = GSSiGlobAlloc ( 214,GMEM_MOVEABLE,USHRT_MAX);
			pIF2 = GlobalLock (hIF);
			strncpy0 (pIF2,pIF,(size_t)lIF); 
			lIFOffset = bpOffset + (int)(pIF - startLoc);
			lIFBP = lIFOffset - bpLen;
			if (pBrkPt && lIFBP > 0)
			{
				IfRtn = LogicPBP(pIF2, &rc, pBrkPt,lIFOffset,bpLen);
			}
			else
				IfRtn = LogicP(pIF2, &rc);
			GSSiGlobUlFree(&hIF);
			if (rc)
				goto IfError; 
			if (!hMem)
			{
				hMem = GSSiGlobAlloc ( 215,GMEM_MOVEABLE,USHRT_MAX);
				NewText = GlobalLock(hMem);
				l = (pStartIF - InText);
				if (l>0) _fmemmove(NewText,InText,(size_t) l);
				OutLoc = NewText + l;
			}  
			if (IfRtn)
			{ 
				int lTHENBP;

				if (*pEndIF == ';')
					InLoc = pEndIF + 1;
				else
					InLoc = pEndIF;
				_fstrncpy (OutLoc,pTHEN,(size_t)lTHEN);
				OutLoc[lTHEN]=0;
				thenBpOffset = bpOffset + (int)(pTHEN - startLoc);
				lTHENBP = bpLen - thenBpOffset;
				if (lTHENBP > 0)
					ExpandTextDB(OutLoc, pBrkPt, thenBpOffset, bpLen);
				else
					ExpandText(OutLoc);
				OutLoc = _fstrchr (OutLoc,0);
			}
			else
			{
				InLoc = FirstNonBlank(InLoc);
				if (!_fstrncmp (InLoc,"ELSE",4))
				{
					InLoc += 4;
					pELSE = InLoc;
					l = pEndIF - InLoc;
					if (l)
					{
						
						int lELSEBP = 0;

						_fstrncpy (OutLoc,pELSE,(size_t)l);
						OutLoc[l]=0;
						if (pBrkPt)
						{
							elseBpOffset = bpOffset + (int)(pELSE - startLoc);
							lELSEBP = bpLen - elseBpOffset;
						}
						if (lELSEBP > 0)
							ExpandTextDB(OutLoc, pBrkPt, elseBpOffset, bpLen);
						else
							ExpandText(OutLoc);
						OutLoc = _fstrchr (OutLoc,0);
					}
				}	
				if (*pEndIF == ';')
					InLoc = pEndIF + 1;
				else
					InLoc = pEndIF;
			}
			IfOK = TRUE;
	IfError:
			if (ContinueProcessing)
				ContinueProcessing = IfOK;
			if (!ContinueProcessing)
				ii=1;
		}
		else if (*InLoc == '$')
		{   
			int	FunID;
			
			if (!(BegBrack = _fstrchr(InLoc,'(')))
				goto OutChar;
			if (!(EndBrack = MatchLev((LPSTR)(BegBrack + 1), ')')))
			{
				if (GetFunctionID(InLoc, BegBrack))
				{
					char mess[128];
					*(BegBrack+1) = 0;
					sprintf(mess, "Unmatched parentheses at %s", InLoc);
					if (GetDebug())
						MessageBox(0, mess, 0, MB_ICONEXCLAMATION);
				}
				goto OutChar;
			}
			if (!(FunID = GetFunctionID(InLoc, BegBrack)))
			{
				InLoc = EndBrack + 1;
				goto OutChar;
			}
			if (pBrkPt)
			{
				char save = *(EndBrack+1);
				*(EndBrack + 1) = 0;
				SetFunctionDBIn(InLoc);
				*(EndBrack + 1) = save;
				breakAtPos(InLoc - startLoc, pBrkPt, bpOffset, bpLen, BA_FUNCTION);
			}
			if (expandOnly && FunID != expandOnly)
				goto OutChar;
			if (AllVarEqQuestionMark == 2)
				AddToODBCParms (InLoc,EndBrack);
			if (!hMem)
			{
				hMem = GSSiGlobAlloc ( 216,GMEM_MOVEABLE,USHRT_MAX);
				NewText = GlobalLock(hMem);
				l = (InLoc - InText);
				if (l>0) _fmemmove(NewText,InText,(size_t) l);
				OutLoc = NewText + l;
			}
			BegBrack++;
			l = EndBrack - BegBrack; 
			InLoc = BegBrack;
			_fmemmove(OutLoc,InLoc,(size_t) l);
			InLoc = ++EndBrack;
			loc = OutLoc + l;
			*loc = '\0';
			if (ContinueProcessing)
			{
				InFunction (FunID,OutLoc);
				if (AllVarEqQuestionMark)
				{
					_fstrcpy (OutLoc,"?");
					l = 1;
				}
				else
				{
					if (FunID > 799 && FunID != 904)
						l = GetFunctionValue3(FunID, OutLoc, OutLoc, pBrkPt, (int)(BegBrack-startLoc)+bpOffset, bpLen);
			    	else if (FunID > 499)
						l = GetFunctionValue2(FunID, OutLoc, OutLoc, pBrkPt, (int)(BegBrack - startLoc) + bpOffset, bpLen);
			    	else if (FunID > 299)
						l = GetFunctionValue1(FunID, OutLoc, OutLoc, pBrkPt, (int)(BegBrack - startLoc) + bpOffset, bpLen);
			    	else
						l = GetFunctionValue(FunID, OutLoc, OutLoc, pBrkPt, (int)(BegBrack - startLoc) + bpOffset, bpLen);
			    }
				OutFunction (FunID,OutLoc);
		    }
		    else
		    	l=0;
			SetFunctionDBOut(OutLoc);
		    OutLoc+=l;
		}
		else
		{
OutChar:	if (hMem)
				*OutLoc++ = *InLoc++;
			else
				InLoc++;
		}
	}
	FoundLiteral = FoundLit;
	if (TraceOn)
	{
		hMemTrace=GSSiGlobAlloc ( 217,GMEM_MOVEABLE,USHRT_MAX);
		pStr = GlobalLock (hMemTrace); 
		if (hMem)   
		{
			l = OutLoc - NewText;
			NewText[l]=0;
			sprintf (pStr,"EXPOUT:%s",NewText); 
		}
		else
			sprintf (pStr,"EXPOUT:%s",InText); 
		GSSiTraceLev (pStr,-1,1);
		GSSiGlobUlFree (&hMemTrace);
	}
	InExpand = SaveIE;
	if (hMem)
	{
		l = OutLoc - NewText;
		if (*NewText == '"' && l > 1)
		{ 
			if (*(LPSTR)(NewText + l-1) == '"')
			{
				*(LPSTR)(NewText + l-1)	= 0;
				NewText++;
				l -= 2;
			}
		}
		*OutLoc++ = '\0'; 
		_fstrcpy(InText,NewText);
		InText+=l;
		GSSiGlobUlFree (&hMem);
		if (ExpandTrace)
			MessageBoxHalt (hWndMain,InText,"Expanded",MB_OK|MB_APPLMODAL);
{
#if ENABLETRACE
GSSiExitProg (558);
#endif
		return (InText);
}
	}
	if (*InText == '"') 
	{
		l = _fstrlen (InText);
		if (l > 1 && *(LPSTR)(InText + l - 1) == '"') 
		{
			_fmemmove (InText,(InText+1),(size_t)l-2); 
			*(LPSTR)(InText + l - 2) = 0;
		}
	}
	if (ExpandTrace)
		MessageBoxHalt (hWndMain,InLoc,"Expanded",MB_OK|MB_APPLMODAL);
{
#if ENABLETRACE
GSSiExitProg (558);
#endif
	return (InLoc);
}
#if ENABLETRACE
}
#endif
}

LPSTR ExpandText(LPSTR InText)
{
	LPSTR rtn = ExpandTextDB(InText, 0, 0, 0);

	return rtn;
}

double GetGlobalDVal (LPSTR Global)
#if ENABLETRACE
{GSSiEnterProg (559);
#endif
{   
	double	rtn=0;
	HANDLE	hStr=GSSiGlobAlloc ( 218,GMEM_MOVEABLE,256);
	LPSTR	str=GlobalLock (hStr);
	
	_fstrcpy (str,Global);
	ExpandText (str);   
	if (*str)
		rtn = atof (str);
	GSSiGlobUlFree (&hStr); 
{
#if ENABLETRACE
GSSiExitProg (559);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

double GetGlobalDVal2 (LPSTR Global,double Default)
#if ENABLETRACE
{GSSiEnterProg (560);
#endif
{
	double	rtn=Default;
	HANDLE	hStr=GSSiGlobAlloc ( 219,GMEM_MOVEABLE,256);
	LPSTR	str=GlobalLock (hStr);
	
	_fstrcpy (str,Global);
	ExpandText (str);   
	if (*str)
		rtn = atof (str);
	GSSiGlobUlFree (&hStr); 
{
#if ENABLETRACE
GSSiExitProg (560);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetGlobalValRaw (LPSTR VName,LPSTR Val)
#if ENABLETRACE
{GSSiEnterProg (561);
#endif
{   
	VARPNT	VP;
	HANDLE	handle;
	
	*Val = 0;
	if ((handle = FindVar (VName)))
	{
		VP = (VARPNT)GlobalLock (handle);
		_fstrcpy (Val,VP->Value);
		GlobalUnlock (handle);
{
#if ENABLETRACE
GSSiExitProg (561);
#endif
		return TRUE;
}
	}
{
#if ENABLETRACE
GSSiExitProg (561);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

BOOL GetGlobalCVal (LPSTR Global,LPSTR Val,LPSTR Default)
#if ENABLETRACE
{GSSiEnterProg (562);
#endif
{
	HANDLE	hStr=GSSiGlobAlloc ( 220,GMEM_MOVEABLE,4096);
	LPSTR	str=GlobalLock (hStr);
	
	if (*Global != '[')
	{
		*str = '[';
		_fstrcpy (&str[1],Global);
		_fstrcat (str,"]");  
	}
	else
		_fstrcpy (str,Global);
	ExpandText (str);   
	if (!*str)
	{   
		if (Default)
			_fstrcpy (Val,Default);
		else
			*Val = 0; 
		GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (562);
#endif
		return FALSE;
}
	}
	_fstrcpy (Val,str);
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (562);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

RECT GetGlobalRectVal (LPSTR Global,LPRECT pDefault) 
#if ENABLETRACE
{GSSiEnterProg (563);
#endif
{
	HANDLE	hStr=GSSiGlobAlloc ( 221,GMEM_MOVEABLE,256);
	LPSTR	str=GlobalLock (hStr);
	RECT	Rect,RectReverse;
	BOOL	Err;
	
	_fstrcpy (str,Global);
	ExpandText (str);   
	if (!*str)
	{   
		if (pDefault)
		{
			GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (563);
#endif
			return *pDefault;
}
		}
	}
	RectReverse = atorect (str,&Err);
	Rect = RectReverse; 
	if (RectReverse.bottom < RectReverse.top)
	{
		Rect.top = RectReverse.bottom;
		Rect.bottom = RectReverse.top; 
	}
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (563);
#endif
	return Rect;
}
#if ENABLETRACE
}
#endif
} 


MNMXCORD GetGlobalBoundsVal (LPSTR Global,LPMNMXCORD pDefault) 
#if ENABLETRACE
{GSSiEnterProg (563);
#endif
{
	HANDLE	hStr=GSSiGlobAlloc ( 221,GMEM_MOVEABLE,256);
	LPSTR	str=GlobalLock (hStr);
	MNMXCORD	Rect;
	BOOL	Err;
	
	_fstrcpy (str,Global);
	ExpandText (str);   
	if (!*str)
	{   
		if (pDefault)
		{
			GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (563);
#endif
			return *pDefault;
}
		}
	}
	Rect = atobounds (str,&Err);
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (563);
#endif
	return Rect;
}
#if ENABLETRACE
}
#endif
} 


long GetGlobalLVal (LPSTR Global)
#if ENABLETRACE
{GSSiEnterProg (564);
#endif
{
	HANDLE	hStr=GSSiGlobAlloc ( 222,GMEM_MOVEABLE,256);
	LPSTR	str=GlobalLock (hStr);
	long	rtn=0;
	
	_fstrcpy (str,Global);
	ExpandText (str);  
	if (*str)
		rtn = atol (str);
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (564);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

long GetGlobalLVal3 (LPSTR Global,long Default)
#if ENABLETRACE
{GSSiEnterProg (565);
#endif
{
	HANDLE	hStr=GSSiGlobAlloc ( 223,GMEM_MOVEABLE,256);
	LPSTR	str=GlobalLock (hStr);
    long	rtn;
    
	sprintf (str,"[%s]",Global);
	rtn = GetGlobalLVal2 (str,Default);
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (565);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

long GetGlobalLVal2 (LPSTR Global,long Default)
#if ENABLETRACE
{GSSiEnterProg (566);
#endif
{
	HANDLE	hStr=GSSiGlobAlloc ( 224,GMEM_MOVEABLE,256);
	LPSTR	str=GlobalLock (hStr);
    long	rtn=Default;
	
	_fstrcpy (str,Global);
	ExpandText (str);
	if (!stricmp (str,"T"))
		rtn = 1;
	else if(!stricmp (str,"F"))
		rtn = 0;
	else if (*str)
		rtn = atol (str);
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (566);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetGlobalPVal (LPSTR Global,LPDPOINT Default,LPDPOINT pPoint)
#if ENABLETRACE
{GSSiEnterProg (567);
#endif
{
	HANDLE	hStr=GSSiGlobAlloc ( 225,GMEM_MOVEABLE,256);
	LPSTR	str=GlobalLock (hStr);
	DPOINT	DPoint;
	
	_fstrcpy (str,Global);
	ExpandText (str);  
	if (!*str)
	{
		if (Default)
			DPoint = *Default;
		else
		{
			GSSiGlobUlFree (&hStr);  
{
#if ENABLETRACE
GSSiExitProg (567);
#endif
			return FALSE;          
}
		}
	}
	else
	{
		if (sscanf (str,"%Flf %Flf",&DPoint.x,&DPoint.y) != 2)
		{
			DPoint.x = 0;
			DPoint.y = 0;
		}
	}  
	GSSiGlobUlFree (&hStr);  
	*pPoint = DPoint;
{
#if ENABLETRACE
GSSiExitProg (567);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetGlobalBVal (LPSTR Global)
#if ENABLETRACE
{GSSiEnterProg (568);
#endif
{
	HANDLE	hStr=GSSiGlobAlloc ( 226,GMEM_MOVEABLE,256);
	LPSTR	str=GlobalLock (hStr);
	BOOL	rtn;
	
	_fstrcpy (str,Global);
	ExpandText (str);
	rtn = atob (str);
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (568);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetGlobalBVal2 (LPSTR Global,BOOL Default)
#if ENABLETRACE
{GSSiEnterProg (569);
#endif
{
	HANDLE	hStr=GSSiGlobAlloc ( 227,GMEM_MOVEABLE,256);
	LPSTR	str=GlobalLock (hStr);
	BOOL	rtn=Default;
	
	_fstrcpy (str,Global);
	ExpandText (str);
	if (*str)
		rtn = atob (str);
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (569);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

int GetVal (LPSTR VarName, LPSTR OutStr)
#if ENABLETRACE
{GSSiEnterProg (570);
#endif
{   VARPNT	VarPnt, VP;
	HANDLE	hGlobal=0;
	int		l;
	LPSTR	lpPar, lpEndPar=0;
	long	Index=0;
    
    if (!ContinueProcessing)
{
#if ENABLETRACE
GSSiExitProg (570);
#endif
    	return 0;
}
	if (AllVarEqQuestionMark==2)
		AddToODBCParms (VarName,0);
	if (AllVarEqQuestionMark==1)
	{
		*OutStr = '?';
{
#if ENABLETRACE
GSSiExitProg (570);
#endif
		return 1;
}
	}
	if (LinkToVar)
	{
		hGlobal = AllocateVar (VarName);
		VP = (VARPNT)GlobalLock (hGlobal); 
		l=VP->NumLinkedVars;  
		while (l--)
		{
			if (VP->LinkedVar[l] == LinkToVar)
			{
				GlobalUnlock (hGlobal);
{
#if ENABLETRACE
GSSiExitProg (570);
#endif
				return 0;
}
			}
		}
		l=VP->NumLinkedVars;
		if (l)
		{   
			HANDLE	OldHandle=hGlobal, NewHandle;
			
			GlobalUnlock (hGlobal); 
			NewHandle = GSSiGlobalReAlloc (0,hGlobal,(long)l*sizeof(HANDLE)+sizeof(VARINFO),GMEM_MOVEABLE); 
			if (OldHandle != NewHandle) 
				UpdateVarHandle (OldHandle,NewHandle);   
			hGlobal = NewHandle;
			VP = (VARPNT)GlobalLock (hGlobal); 
		}
		VP->LinkedVar[VP->NumLinkedVars] = LinkToVar; 
		VP->NumLinkedVars++;
		GlobalUnlock (hGlobal);
{
#if ENABLETRACE
GSSiExitProg (570);
#endif
		return 0; 
}
	}
	if ((l=GetValFromOpenFiles (VarName,OutStr,4096))>=0)
{
#if ENABLETRACE
GSSiExitProg (570);
#endif
		return(l);
}
	
	lpPar = _fstrrchr(VarName,'(');
	if (lpPar)
	{
		lpEndPar = _fstrchr(lpPar,')');
		if (lpEndPar)
		{   
			Index = atol((LPSTR)(lpPar+1)); 
			*lpPar = 0;
			if ((hGlobal = FindVar (VarName)))
			{
				VP = (VARPNT)GlobalLock (hGlobal);
				if (!VP->Type)
				{
					*lpPar = '(';
					GlobalUnlock (hGlobal);    
					hGlobal = 0;
					Index = 0;
				}               
				else
					GlobalUnlock (hGlobal);
			}
			else
				*lpPar = '('; 
		}
	} 
	if (!hGlobal)  
		hGlobal = FindVar(VarName);	
	if (hGlobal)
	{   
{
#if ENABLETRACE
GSSiExitProg (570);
#endif
		return (GetGlobalVal (hGlobal,OutStr,Index));
}
	}
	else
	{   
		*OutStr = 0;
{
#if ENABLETRACE
GSSiExitProg (570);
#endif
		return 0; 
}
		_fstrcpy(shrtxt,VarName);
		_fstrcpy(OutStr,"[");
		_fstrcat(OutStr,shrtxt);
		_fstrcat(OutStr,"]");
{
#if ENABLETRACE
GSSiExitProg (570);
#endif
		return (_fstrlen(shrtxt)+2);
}
	}
#if ENABLETRACE
}
#endif
}  

BOOL ResetFileChangeTime (HANDLE hDB)
{
	LPOPENSQLDATA SQLPtr;
	LPOPENFILEDATA	FilePtr;

	if (!hDB)
		return FALSE;
	SQLPtr = (LPOPENSQLDATA)GlobalLock(hDB);
	SQLPtr->lastreadtime = 0;
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
    if (FilePtr->Type == COMBO_DATAFILE)
	{   
		LPCOMBOHEADER		pComboHeader;
		LPCOMBOFILE  		pComboFile; 
	
		pComboHeader = (LPCOMBOHEADER)GlobalLock (FilePtr->FileHandle); 
		pComboFile = (LPCOMBOFILE)GlobalLock (pComboHeader->hComboFile);
		if (pComboFile->hSQL[0])
		{
			LPOPENSQLDATA SQLPtr2 = (LPOPENSQLDATA)GlobalLock(pComboFile->hSQL[0]);

			SQLPtr2->lastreadtime = 0;
			GlobalUnlock (pComboFile->hSQL[0]);
		}
		GlobalUnlock (pComboHeader->hComboFile);
		GlobalUnlock (FilePtr->FileHandle);
	}
	GlobalUnlock (SQLPtr->OFHandle);
	GlobalUnlock (hDB);
	return TRUE;
}

BOOL GetFileChangeTime (VARPNT VarPtr)
{
    char FileID[32];  
    short	isql;  
    BOOL	rtn=FALSE; 
    LPSTR	pDot;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	LPHANDLE	lpFileHandle;  
	LPFILEPATH	FilePathPtr;

    
    _fstrcpy (FileID,VarPtr->Name);
    if ((pDot = _fstrchr (FileID,'.')))
    {
    	*pDot = 0;
		FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);  
	    isql = FilePathPtr->NumFiles;	
		while (!rtn && isql--)
		{
			lpFileHandle = &FilePathPtr->FileHandle;  
			lpFileHandle += isql;
			if (*lpFileHandle)
			if (!UseOnlyOneDBHandle || (*lpFileHandle == UseOnlyOneDBHandle))
			{   
				SQLPtr = (LPOPENSQLDATA)GlobalLock(*lpFileHandle);
				FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);  
				if (FilePtr->Type != SQL_DATAFILE)
				{
					if (*FileID && !_fstricmp (FileID,SQLPtr->IDName))  
					{
	    				VarPtr->changetime = SQLPtr->lastreadtime;   
	    				rtn = TRUE;
	    			}
	    		}  
	    		GlobalUnlock (SQLPtr->OFHandle);
	    		GlobalUnlock (*lpFileHandle);   
    		} 
    	}
    	GlobalUnlock (FilePathHandle);
    }
   	return rtn;
}
   
BOOL NeedRead (LPOPENSQLDATA SQLPtr)
#if ENABLETRACE
{GSSiEnterProg (571);
#endif
{  
	LPSQLFIELD	lpSQLField; 
	VARPNT		VarPtr;
	DWORD		MaxVarTime=1; 
	int			i;    
	BOOL		FoundVar=FALSE;
	
	if (SQLPtr->st == -999)
	{
		SQLPtr->st = 0;
		goto RtnFalse;
	}
	lpSQLField = &SQLPtr->SQLField;  
	if (SQLPtr->NumGlobals < 0)
	{
		SQLPtr->NumGlobals = 0;
		MaxVarTime = ULONG_MAX;                      
	}
	for (i=0;i<SQLPtr->NumGlobals;i++,lpSQLField++)
	{   
		if (lpSQLField->hGlobal)
		{   
			FoundVar = TRUE;                       
			VarPtr = (VARPNT)GlobalLock (lpSQLField->hGlobal); 
			if (CurrentComboFile && !_fstrnicmp (VarPtr->Name,"CF",2) && VarPtr->Name[3] == '.')
			{
				short FileNo = atoi (&VarPtr->Name[2]);
				if (FileNo < CurrentComboFile->NumFiles)
				{
		             LPOPENSQLDATA SQLPtr2 = (LPOPENSQLDATA)GlobalLock(CurrentComboFile->hSQL[FileNo]);  
		             
		             if ((!SQLPtr2->st && SQLPtr2->lastreadtime > VarPtr->changetime) || (SQLPtr->st !=-999 && NeedRead (SQLPtr2)))
						MaxVarTime = ULONG_MAX;                      
		             GlobalUnlock (CurrentComboFile->hSQL[FileNo]);
			    }
			}
			GetFileChangeTime (VarPtr);
			if (VarPtr->changetime > MaxVarTime)
				MaxVarTime = VarPtr->changetime;
			GlobalUnlock (lpSQLField->hGlobal);
		}
	} 
	if (/*(SQLPtr->NumGlobals && !FoundVar) ||*/ MaxVarTime > SQLPtr->lastreadtime)
{
#if ENABLETRACE
GSSiExitProg (571);
#endif
		return TRUE;
}
RtnFalse:
{
#if ENABLETRACE
GSSiExitProg (571);
#endif
		return FALSE;
}
	
#if ENABLETRACE
}
#endif
}  

BOOL ReadDBSequential (HANDLE hDB,short Index,LPSHORT First)
#if ENABLETRACE
{GSSiEnterProg (572);
#endif
{
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	LPGWFLDINFO	lpGWFldInfo;
	LPGWDHEADER	lpGWDHead;
	HANDLE		hBT;
	int			loc;  
	BOOL		rtn;
	
	if (*First)
		loc = BT_FIRST;
	else
		loc = BT_NEXT;
	*First = FALSE;
	rtn = FALSE;
	
	SQLPtr = (LPOPENSQLDATA)GlobalLock(hDB);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
	
	if (FilePtr->Type == UMIFS_DATAFILE || FilePtr->Type == ORA_DATAFILE || FilePtr->Type == GMCENSUS_DATAFILE)
	{
		lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 
		hBT = lpGWDHead->BTHandle[Index];
	    if (!hBT) goto Exit;
			    
		SQLPtr->st = BT_FIND (hBT,lpGWDHead->pKeys[Index],loc,BT_ANY, (LPSTR)&SQLPtr->Offset);
		if (SQLPtr->st)
			SQLPtr->lastreadtime = 0; 
		else
		{
			rtn = TRUE;
	    	SQLPtr->lastreadtime = LONG_MAX; 
	    }
	}
Exit:
	GlobalUnlock (SQLPtr->OFHandle);
	GlobalUnlock (hDB);  
{
#if ENABLETRACE
GSSiExitProg (572);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

BOOL LogicPFile (LPOPENSQLDATA	SQLPtr,LPSTR SQL,LPBOOL pErr)
{
	BOOL rtn;
	
	if (!*SQL)
		return TRUE;
	LogicPSQLPtr = SQLPtr;
	rtn = LogicP (SQL,pErr);
	LogicPSQLPtr = 0;
	return rtn;
}

int GetDBPos(HANDLE hSQLPtr)
{
	LPOPENSQLDATA	SQLPtr;
	LPOPENFILEDATA	FilePtr;
	int pos = -1;

	if (hSQLPtr)
	{
		SQLPtr = (LPOPENSQLDATA)GlobalLock(hSQLPtr);
		FilePtr = (LPOPENFILEDATA)GlobalLock(SQLPtr->OFHandle);
		pos = GSSillseek(FilePtr->Fid, 0, 1);
		GlobalUnlock(SQLPtr->OFHandle);
		GlobalUnlock(hSQLPtr);
	}
	return pos;
}
BOOL FetchDBRec (HANDLE hSQLPtr)
#if ENABLETRACE
{GSSiEnterProg (573);
#endif
{
	LPOPENSQLDATA	SQLPtr, SaveSQL;
	LPFIELDINFO	lpFieldInfo;
	LPOPENFILEDATA	FilePtr;
	LPSQLFIELD	lpSQLField;
	LPFILEPATH	FilePathPtr;
	LPHANDLE	lpFileHandle;
	LPGWFLDINFO	lpGWFldInfo;
	LPGWDHEADER	lpGWDHead;
	HANDLE		hBT;
	int			irc,ii;  
	short		cond=BT_ANY; 
	HANDLE		hStr=0;
	LPSTR		str;
	
	if (!hSQLPtr)
{
#if ENABLETRACE
GSSiExitProg (573);
#endif
		return FALSE;
}
	irc = 1;
	SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQLPtr);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	lpFieldInfo = &FilePtr->FldInfo;
    switch (FilePtr->Type)
    {
    	case COMBO_DATAFILE:
	    {   
			LPCOMBOHEADER		pComboHeader;
		    LPCOMBOFILE  		pComboFile; 
	    
			pComboHeader = (LPCOMBOHEADER)GlobalLock (FilePtr->FileHandle); 
			pComboFile = (LPCOMBOFILE)GlobalLock (pComboHeader->hComboFile);
			irc = FetchDBRec (pComboFile->hSQL[0]);
			GlobalUnlock (pComboHeader->hComboFile);
			GlobalUnlock (FilePtr->FileHandle); 
			goto Exit;
	    } 
	    break;
		
        case HLTLIST_DATAFILE:
		{
			HIGHLIGHTDATA	HighlightData;
			long	Ref;  
	
			if (!(SQLPtr->st=BT_FIND (FilePtr->FileHandle,(LPSTR)&Ref,SQLPtr->IndexToUse,BT_ANY,(LPSTR)&HighlightData)))
			{
				SQLPtr->IndexToUse = BT_NEXT;
 				SQLPtr->Offset = Ref;
		        PickList[0]=HighlightData.PD;  
			    ProcessPickedItem (0,FALSE);
			}
		}
		break;

        case THEME_HLTFILE:
		{
			THEMEHIGHLIGHTDATA	ThemeHighlightData;
			THEMEHIGHLIGHTKEY	ThemeHighlightKey;
			if (!(SQLPtr->st=BT_FIND (FilePtr->FileHandle,(LPSTR)&ThemeHighlightKey,SQLPtr->IndexToUse,BT_ANY,(LPSTR)&ThemeHighlightData)))
			{
				SQLPtr->IndexToUse = BT_NEXT;
			}
		}
		break;

		case LISTVAR_DATAFILE:
		{
			LPLISTVARDATABASE	pDB;   

			pDB = (LPLISTVARDATABASE)GlobalLock (FilePtr->FileHandle);
			if (GetNextPipeDelimitedValue (pDB->Value,&pDB->Pos,pDB->CurRow))
			{
				SQLPtr->st = 0;
				pDB->CurRowID++;
			}
			else
				SQLPtr->st = 1;
			GlobalUnlock (FilePtr->FileHandle);
		}
		break;
		
    	case GMTEXT_DATAFILE: 
    	{   
    		hStr=GSSiGlobAlloc ( 228,GMEM_MOVEABLE,4096);  
    		str=GlobalLock (hStr);
			if (NeedRead (SQLPtr)) 
			{
				GSSillseek (FilePtr->Fid,FilePtr->FirstLineOffset,0);  
				SQLPtr->st = 0;
			}
			while (!SQLPtr->st)  
			{
		    	SQLPtr->Offset = GSSillseek (FilePtr->Fid,0,1);  
		    	if (!fgetstring (str,4090,FilePtr->Fid))
		    		SQLPtr->st = 1;
		    	else
		    	{   
		    		BOOL	Err;
		    		
			        SQLPtr->st = 0;  
					GetDelimTextData(str,FilePtr->FileHandle);
				    SQLPtr->lastreadtime = NextVarTime ();
					if (LogicPFile (SQLPtr,SQLPtr->SQL,&Err))
						break;
			    }
			}
		}
    	break;
		
		case GMCENSUS_DATAFILE:   
		case ORA_DATAFILE:
		case UMIFS_DATAFILE:
		{ 
    		BOOL	HaveOtherFields=FALSE;
	    	long	nSearch=1;

			if (SQLPtr->IndexToUse < 0)
			{
				irc = 0;
				goto Exit;
			}
			lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 
			hBT = lpGWDHead->BTHandle[SQLPtr->IndexToUse];
		    if (!hBT)
		    {
		    	GlobalUnlock (FilePtr->FileHandle);
		    	irc = 0;
		    	goto Exit;
			}				    
		    if (NeedRead (SQLPtr))
			{
				if (lpGWDHead->Version > 1000)
				{   
					SQLPtr->lastreadtime = NextVarTime ();
				    irc = LoadInternalGMD (lpGWDHead,InternalRefno);
			        GlobalUnlock (FilePtr->FileHandle); 
				    goto Exit;
				}
				else
				{   
		    		int	i, j, n;  
		    		BOOL	FieldSet[8]; 
		    		HANDLE	hMem=GSSiGlobAlloc ( 229,GMEM_MOVEABLE,256);
		    		LPSTR	str=GlobalLock (hMem);
		    		
		    		if (SQLPtr->NumGlobals)
		    		{	
		    			
	    				n = abs(lpGWDHead->NumIndexFields[SQLPtr->IndexToUse]); 		    
		    			if (SQLPtr->IndexToUse && SQLPtr->Unique)
		    				n--;
		    			for (i=0;i<n;i++)
		    				FieldSet[i]=FALSE;
		    			if (SQLPtr->IndexToUse)
		    			{
			    			for (i=0;i<abs(lpGWDHead->NumIndexFields[0]);i++)
			    			{
		    					SetFieldToMinVal (lpGWDHead,lpGWDHead->IndexFields[0][i]);
			    			}
		    			}
						if (lpGWDHead->lKeys[SQLPtr->IndexToUse] < 0)
							cond = BT_GE;
						else
							cond = BT_EQ;
						for (i=0,lpSQLField=&SQLPtr->SQLField;i<SQLPtr->NumGlobals;i++,lpSQLField++) 
						{   
				    		if (lpSQLField->FieldNum >= 0 &&
				    			(lpSQLField->OpCode == OPCODE_EQ || lpSQLField->OpCode == OPCODE_GE || lpSQLField->OpCode == OPCODE_GT))
				    		{   
				    			BOOL SetField = FALSE;
				    			
					    		_fstrcpy (str,lpSQLField->String);
					    		ExpandText (str);
					    		lpGWFldInfo=lpGWDHead->pFldInfo + lpSQLField->FieldNum;
								SetFieldValFromChar(lpGWDHead,lpGWFldInfo,str,FALSE,FALSE);
								for (j=0;j<n;j++)
									if (lpSQLField->FieldNum == lpGWDHead->IndexFields[SQLPtr->IndexToUse][j] && 
				    					(lpSQLField->OpCode == OPCODE_EQ || lpSQLField->OpCode == OPCODE_GE || lpSQLField->OpCode == OPCODE_GT))
				    				{
										FieldSet[j] = TRUE; 
										SetField = TRUE;
									}
								if (SetField)
								{  
									if (lpSQLField->OpCode == OPCODE_GE)
				    					cond = BT_GE;
				    				else if (lpSQLField->OpCode == OPCODE_GT)
				    					cond = BT_GT;
				    			}
								else
									HaveOtherFields = TRUE; 

							}
							else if (lpSQLField->FieldNum >= 0)
								HaveOtherFields = TRUE;
						}
		    			for (i=0;i<abs(lpGWDHead->NumIndexFields[SQLPtr->IndexToUse]);i++)
		    			{
		    				if (!FieldSet[i])
		    				{
		    					SetFieldToMinVal (lpGWDHead,lpGWDHead->IndexFields[SQLPtr->IndexToUse][i]);
		    					cond = BT_GE;
		    				}
		    			}
						GWDFormKey(lpGWDHead,SQLPtr->IndexToUse,TRUE,0,0);
						if (SQLPtr->IndexToUse && cond == BT_EQ) 
							SetReadSecIndex(hBT,TRUE); 
					}                
					else
						cond = BT_ANY;
					SQLPtr->st = BT_FIND (hBT,lpGWDHead->pKeys[SQLPtr->IndexToUse],BT_FIRST,cond, (LPSTR)&SQLPtr->Offset);
					SetReadSecIndex(hBT,FALSE);
					SQLPtr->lastreadtime = NextVarTime ();
					GSSiGlobUlFree (&hMem);
				}
			}
			else if (lpGWDHead->Version > 1000)
			{   
		        GlobalUnlock (FilePtr->FileHandle); 
			    goto Exit;
			}
			else 
			{
				SQLPtr->st = BT_FIND (hBT,lpGWDHead->pKeys[SQLPtr->IndexToUse],BT_NEXT,BT_ANY, (LPSTR)&SQLPtr->Offset);  
			}
			while (WantGMDRecord (FilePtr,SQLPtr,lpGWDHead,cond,HaveOtherFields) > 0)
			{
				SQLPtr->st = BT_FIND (hBT,lpGWDHead->pKeys[SQLPtr->IndexToUse],BT_NEXT,BT_ANY, (LPSTR)&SQLPtr->Offset); 
				nSearch++;
				if (!(nSearch % 100)) 
				{
					if (!CheckForContinue(TRUE,0))
					{
						ContinueProcessing = FALSE;
				     	HaltReport=TRUE;  
						MessageBox (0,"Search cancelled",0,MB_OK);
						break;
					}
				}
			}
			GlobalUnlock (FilePtr->FileHandle);
		} 
		break;
		
		case PN_DATAFILE:
			{
				BOOL doRemove = FALSE;

				if (NeedRead (SQLPtr))
				{

					if (!hIntData)
					{
						hIntData = GSSiGlobAlloc ( 674,GHND,1024); 
						hPNAddData = GSSiGlobAlloc ( 675,GHND,1024); 
						doRemove = TRUE;
					}
					ProcessPickedItem (0,FALSE);
					if (GetPNData (FilePtr->FileHandle,PickList[0].Refno))
						SQLPtr->st = 0;
					else
						SQLPtr->st = 31;
					if (doRemove)
					{
						GSSiGlobFree (&hIntData); 
						GSSiGlobFree (&hPNAddData);
					}
				}
			}
			break;

		case SHAPE_DATAFILE:
			if (*SQLPtr->SQL < ' ')
				CurrentSHPRec++;
			if (CurrentSHPRec < 0 || CurrentSHPRec >= NumSHPDBFRecs)
				SQLPtr->st = 31;
			else
				SQLPtr->st = 0;
			break;
		
		case SQL_DATAFILE: 
		{   
			LPSQLDATABASE	pSQL=(LPSQLDATABASE)GlobalLock (FilePtr->FileHandle);
			
			irc = FetchDBRec (pSQL->DBHandle);
			GlobalUnlock (FilePtr->FileHandle);
			goto Exit;
		}	
		case FGDB_DATAFILE:
		{
		    if (NeedRead (SQLPtr))
		    {   
				short	st;

			    SQLPtr->lastreadtime = NextVarTime ();
		    	if (SQLPtr->hstmt)
		    	{
					FGDBCloseCursor((int)SQLPtr->hstmt);
		        	FGDBFreeStmt((int)SQLPtr->hstmt);
		        	SQLPtr->hstmt = 0;
				    ClearCurVals (FilePtr);
		        } 
		        SQLPtr->st = 0;    
				GetFGDBFieldData (FilePtr,SQLPtr->SQL,&SQLPtr->hstmt,
	            	                  lpFieldInfo, FALSE,0, &st,
									  FilePtr->NumFields, &FilePtr->FldInfo, SQLPtr->singleValID);
				irc = st;
	        }
	        else    
	        {   
	        	int	rc;
		                	
				irc = FetchFGDBRecord(FilePtr, SQLPtr->singleValID);
			    if(irc)
			    {
					FGDBCloseCursor((int)SQLPtr->hstmt);
		        	FGDBFreeStmt((int)SQLPtr->hstmt);
		        	SQLPtr->hstmt = 0;
				    ClearCurVals (FilePtr);
				}		    
			}
			if (irc)
				SQLPtr->st = 31;
		} 
		break;

		case DBF_DATAFILE:
			if (*SQLPtr->SQL < ' ')
				CurrentDBFRec++;
			if (CurrentDBFRec < 0 || CurrentDBFRec >= NumDBFRecs)
				SQLPtr->st = 31;
			else
				SQLPtr->st = 0;
			break;
		break;

		case PGDB_DATAFILE:
		case ODBC_DATAFILE:
		case TEXT_DATAFILE:
		{   
		    if (NeedRead (SQLPtr))
		    {   
				short	st;

			    SQLPtr->lastreadtime = NextVarTime ();
		    	if (SQLPtr->hstmt)
		    	{
					SQLCloseCursor(SQLPtr->hstmt);
		        	SQLFreeStmt(SQLPtr->hstmt, SQL_DROP);
		        	SQLPtr->hstmt = 0;
				    ClearCurVals (FilePtr);
		        } 
		        SQLPtr->st = 0;    
				GetExternalFieldData (FilePtr,SQLPtr->SQL,&SQLPtr->hstmt,
	            	                  lpFieldInfo, FALSE,0, &st,
	                       		   	  FilePtr->NumFields,&FilePtr->FldInfo); //FilePtr->FldInfo[1]
				irc = st;
	        }
	        else    
	        {   
	        	int	rc;
		                	
			    rc = FetchODBCRecord (SQLPtr);  
			    if(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
			    	irc = 0;  
			    else
			    {
					SQLCloseCursor(SQLPtr->hstmt);
		        	SQLFreeStmt(SQLPtr->hstmt, SQL_DROP);
		        	SQLPtr->hstmt = 0;
				    ClearCurVals (FilePtr);
				}		    
			}
			if (irc)
				SQLPtr->st = 31;
		} 
		break;
	}
	if (!SQLPtr->st)
		irc = 1; 
	else
		irc = 0;
Exit: 
 	GlobalUnlock (SQLPtr->OFHandle);
	GlobalUnlock (hSQLPtr);
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (573);
#endif
 	return irc;
}
#if ENABLETRACE
}
#endif
} 

short WantGMDRecord (LPOPENFILEDATA FilePtr,LPOPENSQLDATA SQLPtr,LPGWDHEADER lpGWDHead,short cond,BOOL HaveOtherFields)
#if ENABLETRACE
{GSSiEnterProg (574);
#endif
{ 
//returns 1 if this record not wanted but should continue search else 0 
	short	n,i,j,k, st=0;
	BOOL	err,ii;
	LPSQLFIELD	lpSQLField; 
	LPSTR	pSQL,str, pVal, pPar;
	HANDLE	hStr;
	LPGWFLDINFO	pGWDField;
	BOOL	SaveAVEM = AllVarEqQuestionMark;  
	
	if (SQLPtr->st)
{
#if ENABLETRACE
GSSiExitProg (574);
#endif
		return 0;
}   
	if (cond == BT_EQ && !HaveOtherFields) 
	{
		FillGWDData (lpGWDHead,SQLPtr->Offset);
{
#if ENABLETRACE
GSSiExitProg (574);
#endif
		return 0; 
}
	}   
	AllVarEqQuestionMark = FALSE;
	hStr = GSSiGlobAlloc ( 230,GMEM_MOVEABLE,2048);
	str = GlobalLock (hStr);
	pVal = str + 1024;
	if (FillGWDData (lpGWDHead,SQLPtr->Offset) < 0)
	{
		st = 1;
		goto Exit;
	}
	n = abs(lpGWDHead->NumIndexFields[SQLPtr->IndexToUse]); 		    
	if (SQLPtr->IndexToUse && SQLPtr->Unique)
		n--;
	if ((pPar = MatchLev (SQLPtr->SQL,'(')))
	{
		int	iend = n;

		*pPar = 0;
		n = 0;
		for (j = 0;j < iend;j++)
		{
			char	searchStr[80];
			
			pGWDField = &lpGWDHead->pFldInfo[lpGWDHead->IndexFields[SQLPtr->IndexToUse][j]];
			strcpy (searchStr,pGWDField->Name);
			strcat (searchStr,"=");
			if (!strstr (SQLPtr->SQL,searchStr))
				break;
			n++;
		}
		*pPar = '(';
	}
	for (k=0;k<n;k++)
	{ 
		for (j=0,lpSQLField=&SQLPtr->SQLField;j<SQLPtr->NumGlobals;j++,lpSQLField++)
		{ 
			if (lpGWDHead->IndexFields[SQLPtr->IndexToUse][k] == lpSQLField->FieldNum && 
				(lpSQLField->OpCode == OPCODE_EQ || lpSQLField->OpCode == OPCODE_GE || lpSQLField->OpCode == OPCODE_LE))
			{ 
				if (k && lpSQLField->OpCode == OPCODE_LE)
					continue;
			   	_fstrcpy (str,lpSQLField->String);
    			ExpandText (str);
				ConvertFieldValFromChar(lpGWDHead,lpSQLField->FieldNum,str,pVal);
				if (TestFieldVal (lpGWDHead,lpSQLField->FieldNum,pVal,lpSQLField->OpCode))
				{
					GSSiGlobUlFree (&hStr);
					SQLPtr->st = 1; 
					AllVarEqQuestionMark = SaveAVEM;
{
#if ENABLETRACE
GSSiExitProg (574);
#endif
					return 0; 
}
				}
			}
		}
	}
    st = !LogicPFile (SQLPtr,SQLPtr->SQL,&err); 
Exit:
	GSSiGlobUlFree (&hStr);
	AllVarEqQuestionMark = SaveAVEM;
{
#if ENABLETRACE
GSSiExitProg (574);
#endif
	return st;
}
#if ENABLETRACE
}
#endif
} 

HANDLE GetDBByIDName (LPSTR IDName)
#if ENABLETRACE
{GSSiEnterProg (575);
#endif
{
	LPFIELDINFO	lpFieldInfo;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	LPSQLFIELD	lpSQLField;
	LPFILEPATH	FilePathPtr;
	LPHANDLE	lpFileHandle; 
	HANDLE		hSQLPtr=0;
	UINT		i;

    if (!FilePathHandle)
{
#if ENABLETRACE
GSSiExitProg (575);
#endif
    	return 0;
}
	FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);  
	lpFileHandle = (LPHANDLE)&FilePathPtr->FileHandle;
            
	for (i=0;i<FilePathPtr->NumFiles;i++,lpFileHandle++)
	{
		if (*lpFileHandle)
		{
			SQLPtr = (LPOPENSQLDATA)GlobalLock (*lpFileHandle);
			if (!_fstricmp (SQLPtr->IDName,IDName))  
			{
				hSQLPtr = *lpFileHandle;
				GlobalUnlock (*lpFileHandle);
				GlobalUnlock (FilePathHandle);  
{
#if ENABLETRACE
GSSiExitProg (575);
#endif
				return hSQLPtr;
}
			}
			GlobalUnlock (*lpFileHandle);
		} 
	}
	GlobalUnlock (FilePathHandle);
{
#if ENABLETRACE
GSSiExitProg (575);
#endif
    return 0;  
}
#if ENABLETRACE
}
#endif
}
            
BOOL SaveDBRec (HANDLE hSQLPtr)
#if ENABLETRACE
{GSSiEnterProg (576);
#endif
{
	LPOPENSQLDATA	SQLPtr;
	LPFIELDINFO	lpFieldInfo;
	LPOPENFILEDATA	FilePtr;
	LPSQLFIELD	lpSQLField;
	LPFILEPATH	FilePathPtr;
	LPHANDLE	lpFileHandle;
	LPGWFLDINFO	lpGWFldInfo;
	LPGWDHEADER	lpGWDHead;
	HANDLE		hBT;
	int			irc;  
	BOOL		rtn;
	
	irc = 1;
	SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQLPtr);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	lpFieldInfo = &FilePtr->FldInfo;
    if (FilePtr->Type == COMBO_DATAFILE || FilePtr->Type == GMCENSUS_DATAFILE)
    {
    	irc = 0;
    	goto Exit;   
    } 
	else if (FilePtr->Type == UMIFS_DATAFILE || FilePtr->Type == ORA_DATAFILE)
	{ 
		lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 
		irc = GWDReplaceRecord (lpGWDHead,0,0,-1);
		GlobalUnlock (FilePtr->FileHandle); 
	}
	else if (FilePtr->Type == ODBC_DATAFILE || FilePtr->Type == DBF_DATAFILE || FilePtr->Type == TEXT_DATAFILE)
	{   
    	irc = 0;
    	goto Exit;   
	}
Exit: 
 	GlobalUnlock (SQLPtr->OFHandle);
	GlobalUnlock (hSQLPtr);
{
#if ENABLETRACE
GSSiExitProg (576);
#endif
 	return irc;
}
#if ENABLETRACE
}
#endif
}
   
BOOL SetGMDField (HANDLE hSQLPtr,LPSTR Name,LPSTR Value)
#if ENABLETRACE
{GSSiEnterProg (577);
#endif
{
	LPOPENSQLDATA	SQLPtr;
	LPFIELDINFO	lpFieldInfo;
	LPOPENFILEDATA	FilePtr;
	LPSQLFIELD	lpSQLField;
	LPFILEPATH	FilePathPtr;
	LPHANDLE	lpFileHandle;
	LPGWFLDINFO	lpGWFldInfo;
	LPGWDHEADER	lpGWDHead;
	HANDLE		hBT;
	int			irc;  
	BOOL		rtn;
	
	irc = 1;
	SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQLPtr);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	lpFieldInfo = &FilePtr->FldInfo;
    if (FilePtr->Type == COMBO_DATAFILE || FilePtr->Type == GMCENSUS_DATAFILE)
    {
    	irc = 0;
    	goto Exit;   
    } 
	else if (FilePtr->Type == UMIFS_DATAFILE || FilePtr->Type == ORA_DATAFILE)
	{ 
		lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 
	    SetFieldValFromCharAndName(lpGWDHead,Name,Value,FALSE);
		GlobalUnlock (FilePtr->FileHandle); 
	}
	else if (FilePtr->Type == ODBC_DATAFILE || FilePtr->Type == DBF_DATAFILE || FilePtr->Type == TEXT_DATAFILE)
	{   
    	irc = 0;
    	goto Exit;   
	}
Exit: 
 	GlobalUnlock (SQLPtr->OFHandle);
	GlobalUnlock (hSQLPtr);
{
#if ENABLETRACE
GSSiExitProg (577);
#endif
 	return irc;
}
#if ENABLETRACE
}
#endif
} 

short CompressCensusString (LPSTR pStr)
{   
	HANDLE	hStr=GSSiGlobAlloc ( 231,GMEM_MOVEABLE,4096);
	LPSTR	pNewStr = GlobalLock (hStr); 
	LPSTR	pNewStrBeg = pNewStr;
	LPSTR	pValBeg=pStr; 
	LPSTR	pStrBeg = pStr;
	LPSTR	pLastVal=0;
	short	iCmpCode=0, l,ii;
	
	HANDLE	hOrig = GSSiGlobAlloc ( 232,GMEM_MOVEABLE,4096);
	LPSTR	pOrigStr = GlobalLock (hOrig);
	_fstrcpy (pOrigStr,pStr);      
	
	while (*pStr) 
	{
		if (*pStr == ',')
		{
			*pStr++ = 0;
			if (iCmpCode > 24 || !pLastVal || (pLastVal != pValBeg && _fstrcmp (pLastVal,pValBeg)))
			{
				if (pNewStr != pNewStrBeg)
					*pNewStr++ = 'A' + iCmpCode;
				_fstrcpy (pNewStr,pValBeg);
				pNewStr += _fstrlen (pValBeg); 
				pLastVal = pValBeg;
				pValBeg= pStr;
				iCmpCode = 0;
			}
			else 
			{
				iCmpCode++;
				pValBeg = pStr;
			}
		}
		else
			pStr++;
	}   
	if (pNewStr != pNewStrBeg)
		*pNewStr++ = 'A' + iCmpCode;
	_fstrcpy (pNewStr,pValBeg);
	_fstrcpy (pStrBeg,pNewStrBeg);
	ExpandCensusString (pNewStrBeg);
	if (_fstrcmp (pNewStrBeg,pOrigStr))
		GSSiMsgBox (0,"Compression Fails",0,MB_OK,0);
	GSSiGlobUlFree (&hOrig);
	
	GSSiGlobUlFree (&hStr);
	return _fstrlen (pStrBeg);
}

short ExpandCensusString (LPSTR pStr)
{   
	HANDLE	hStr=GSSiGlobAlloc ( 233,GMEM_MOVEABLE,4096);
	LPSTR	pNewStr = GlobalLock (hStr); 
	LPSTR	pNewStrBeg = pNewStr;
	LPSTR	pStrBeg = pStr;
	LPSTR	pValBeg=pStr;
	short	iCmpCode, l;
	
	while (*pStr) 
	{
		iCmpCode = (short)*pStr - 'A';
		if (iCmpCode >= 0 && iCmpCode < 26)
		{ 
			*pStr++ = 0; 
			l = _fstrlen (pValBeg);
			iCmpCode++;
			while (iCmpCode--)
			{   
				if (pNewStr != pNewStrBeg)
					*pNewStr++ = ',';
				_fstrcpy (pNewStr,pValBeg);
				pNewStr += l;
			}
			pValBeg = pStr;
		}
		else
			pStr++;
	}
	if (pNewStr != pNewStrBeg)
		*pNewStr++ = ',';
	_fstrcpy (pNewStr,pValBeg);
	_fstrcpy (pStrBeg,pNewStrBeg);
	GSSiGlobUlFree (&hStr);
	return _fstrlen (pStrBeg);
}

HANDLE GetOpenHandleFromID (LPSTR ID,int WantType)
{
	HANDLE	hDB = 0;
	int	isql;

	if (FilePathHandle)
	{
		LPFILEPATH	FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);

		isql = FilePathPtr->NumFiles;	
		while (isql--)
		{
			LPHANDLE	lpFileHandle = &FilePathPtr->FileHandle;  

			lpFileHandle += isql;
			if (*lpFileHandle)
			{
				LPOPENSQLDATA	SQLPtr = (LPOPENSQLDATA)GlobalLock(*lpFileHandle);

				if (!stricmp (ID,SQLPtr->IDName))
				{
					LPOPENFILEDATA FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);

					hDB = FilePtr->FileHandle;
					GlobalUnlock (SQLPtr->OFHandle);
					GlobalUnlock(*lpFileHandle);
					break;
				}
				GlobalUnlock(*lpFileHandle);
			}
		}
		GlobalUnlock (FilePathHandle);
	}
	return hDB;
}

void CheckOpenDataFiles (void)
{
    LPFILEPATH  FilePathPtr; 
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	LPHANDLE	lpFileHandle; 
	int		isql;

	if (!FilePathHandle)
		return;

	FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
    
    isql = FilePathPtr->NumFiles;	
	while (isql--)
	{
		lpFileHandle = &FilePathPtr->FileHandle;  
		lpFileHandle += isql;
		if (*lpFileHandle)
		{   
			SQLPtr = (LPOPENSQLDATA)GlobalLock(*lpFileHandle);
			FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); //IgnoreLock=FALSE; 
			GlobalUnlock (SQLPtr->OFHandle);
			GlobalUnlock (*lpFileHandle);
		}
	}
	GlobalUnlock (FilePathHandle);
	return;
}
   
void SetDLGList (HWND hWndDlg,UINT dlgList)
{
	dlgListWnd = hWndDlg;
	dlgListList = dlgList;
	return;
}

short GetDlgListVal (LPSTR VarName,LPSTR Value)
{
	short ln=0;
	char str[4096];
	int i=0;
	*Value  = 0;
 
	if (dlgListWnd)
	while (SendDlgItemMessage(dlgListWnd,dlgListList,LB_GETTEXT,i++,(DWORD)str) != LB_ERR)
	{
		LPSTR pTab = strchr (str,'\t');

		if (pTab)
			*pTab++ = 0;
		else
			pTab = strchr (str,0);
		if (!stricmp (str,VarName))
		{
			strcpy (Value,pTab);
			ln = strlen (pTab);
			break;
		}
	}

	return ln;
}

int GetValFromOpenFiles (LPSTR VarName,LPSTR Value,int maxlval)
#if ENABLETRACE
{GSSiEnterProg (578);
#endif
{   
	LPGWFLDINFO	lpGWFldInfo;
	LPGWDHEADER	lpGWDHead;
	HANDLE		hBT;
	long		Offset, IVal; 
	short		st, i, j, k, len, isql, ifield, irc, rtn=-1,ii; 
	LPSTR		ep, ValC, lpDot;
	LPFIELDINFO	lpFieldInfo,pFileField;
	LPOPENFILEDATA	FilePtr, FilePtr2;
	LPOPENSQLDATA	SQLPtr, SQLPtr2, SQLPtr3;
	LPSQLFIELD	lpSQLField;
	LPHANDLE	lpFileHandle; 
    LPFILEPATH  FilePathPtr; 
	char		IDName[34];  
	BOOL		err;     
	BOOL		ValueInExtendedArea=FALSE, WantRecOffset=FALSE;   
	HANDLE		hStr=0;
	int			CurrentRec;
	
//	*Value = 0;
    lpDot = _fstrchr (VarName,'.');
    if (lpDot)
    {    
    	HANDLE	hGlobal;
    	
    	*lpDot = 0;
    	_fstrcpy (IDName,VarName);  
    	*lpDot ='.';
    	VarName = lpDot + 1;  
    	if (!*IDName)
    	{
    		hGlobal = FindVar(VarName);	
			rtn = GetGlobalVal (hGlobal,Value,0);
			goto GetOut;
        }
		if (!stricmp(IDName, "GRID"))
			ii = 1;
		if (!stricmp (IDName,"BDDLIST"))
		{
			rtn = GetDlgListVal (VarName,Value);
			goto GetOut;
		}
		if (FilePathHandle)
		{
			FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
    
			isql = FilePathPtr->NumFiles;	
			while (isql--)
			{
				lpFileHandle = &FilePathPtr->FileHandle;  
				lpFileHandle += isql;
				if (*lpFileHandle)
				{
					SQLPtr = (LPOPENSQLDATA)GlobalLock(*lpFileHandle);
					if (!stricmp (IDName,SQLPtr->IDName) && *SQLPtr->myhandleC)
					{
						strcpy (IDName,SQLPtr->myhandleC);
						GlobalUnlock(*lpFileHandle);
						break;
					}
					GlobalUnlock(*lpFileHandle);
				}
			}
			GlobalUnlock (FilePathHandle);
		}
    }
    else
    	IDName[0]=0;  
	if (!FilePathHandle)
		goto GetOut; 
	hStr=GSSiGlobAlloc ( 234,GMEM_MOVEABLE,4096+256+4096);
	{
		LPSTR		str=GlobalLock (hStr);
		LPSTR		TempValue=str+4096;
		LPSTR		pSQL=TempValue+256;   
	
/*	if (*VarName == '%')
		goto GetOut;*/
	FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
    
    if (!_fstricmp (VarName,"%RECORDOFFSET"))
    	WantRecOffset = TRUE;
    isql = FilePathPtr->NumFiles;	
	while (isql--)
	{
		lpFileHandle = &FilePathPtr->FileHandle;  
		lpFileHandle += isql;
		if (*lpFileHandle)
		if (!UseOnlyOneDBHandle || (*lpFileHandle == UseOnlyOneDBHandle))
		{   
			SQLPtr = (LPOPENSQLDATA)GlobalLock(*lpFileHandle);
			FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); //IgnoreLock=FALSE; 
			//if (FilePtr->Type == SQL_DATAFILE)
			//	goto Exit;
			if (*IDName && _fstricmp (IDName,SQLPtr->IDName))
				goto Exit;   
			else
			{    
				if (WantRecOffset)
					goto GetData;
				lpFieldInfo = &FilePtr->FldInfo; 
				for (ifield=0;ifield<FilePtr->NumFields;ifield++,lpFieldInfo++)
				{
					if (!_fstricmp (lpFieldInfo->name,VarName))
						goto GetData;
				}
				if (*IDName && *VarName == '#')
				{   
					VarName++;
					ifield = atoi (VarName) - 1;
					lpFieldInfo = &FilePtr->FldInfo; 
					lpFieldInfo	+= ifield;
					goto GetData;
				}
			}
			if (FilePtr->Type == GMCENSUS_DATAFILE)
			{   
				char	CensusKey[32];
				HANDLE	hBT;
				
				lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 
				hBT = lpGWDHead->BTHandle[lpGWDHead->NumIndex];
				GlobalUnlock (FilePtr->FileHandle); 
				_fstrncpy (CensusKey,VarName,32);
				if (BT_FIND (hBT,CensusKey,BT_FIRST,BT_EQ,(LPSTR)&SFFieldData))
					goto Exit;
				ifield = SFFieldData.File+2;  
				ValueInExtendedArea = TRUE;
				goto GetData;
			}
			goto Exit;
	GetData: 
		    switch (FilePtr->Type)
		    { 
				case IMAGE_DATAFILE:
				{
				    BITMAPINFOHEADER DibInfo; 
					DPOINT	WPoint;
			
					GetBitmapInfoFromHandle (&DibInfo,(HDIB32)FilePtr->FileHandle);
	
					switch (lpFieldInfo->index)
					{
					case 0:
						itoa (DibInfo.biWidth,Value,10);
						break;
					case 1:
						itoa (DibInfo.biHeight,Value,10);
						break;
					case 2:
						itoa (DibInfo.biBitCount,Value,10);
						break;
					case 3:
						if (GetImageCoord ((HDIB32)FilePtr->FileHandle, &WPoint,0))
							ftoa (Value,WPoint.x);
						else
							strcpy (Value,"0");
						break;
					case 4:
						if (GetImageCoord ((HDIB32)FilePtr->FileHandle, &WPoint,0))
							ftoa (Value,WPoint.y);
						else
							strcpy (Value,"0");
						break;
					case 5:
						{
							char	DateTaken[32];

						if (GetImageCoord ((HDIB32)FilePtr->FileHandle, &WPoint,DateTaken))
							strcpy (Value,DateTaken);
						else
							strcpy (Value,"0");
						}
						break;
					default:
						*Value = 0;
					}
					goto GotData;
				}
				break;

				case HLTLIST_DATAFILE:
				{
					HIGHLIGHTDATA	HighlightData;
	
					if (!BT_FIND (FilePtr->FileHandle,(LPSTR)&SQLPtr->Offset,BT_FIRST,BT_EQ,(LPSTR)&HighlightData))
					{
						switch (lpFieldInfo->index)
						{
						case 0:
							itoa (SQLPtr->Offset,Value,10);
							break;
						case 1:
							itoa (HighlightData.PD.Desc,Value,10);
							break;
						case 2:
							strcpy (Value,HighlightData.PD.Prefix);
							break;
						case 3:
							strcpy (Value,HighlightData.PD.UDI);
							break;
						default:
							*Value = 0;
						}
						goto GotData;
					}
				}
				break;

				case THEME_HLTFILE:
				{
					THEMEHIGHLIGHTDATA	ThemeHighlightData;
					THEMEHIGHLIGHTKEY	ThemeHighlightKey;
					if (!(SQLPtr->st=BT_FIND (FilePtr->FileHandle,(LPSTR)&ThemeHighlightKey,BT_CURPOS,BT_ANY,(LPSTR)&ThemeHighlightData)))
					{
						switch (lpFieldInfo->index)
						{
						case 0:
							itoa (ThemeHighlightKey.Refno,Value,10);
							break;
						case 1:
							itoa (ThemeHighlightData.Desc,Value,10);
							break;
						case 2:
							strcpy (Value,ThemeHighlightData.Prefix);
							break;
						case 3:
							strcpy (Value,ThemeHighlightData.UDI);
							break;
						case 4:
							itoa (ThemeHighlightKey.Class,Value,10);
							break;
						case 5:
							ftoa (Value,ThemeHighlightData.Point.x);
							break;
						case 6:
							ftoa (Value,ThemeHighlightData.Point.y);
							break;
						default:
							*Value = 0;
						}
						goto GotData;
					}
				}
				break;

				case DBF_DATAFILE:
		    	case SHAPE_DATAFILE:
				{   
					long	LongVal;
					double	DoubleVal; 
					LPCSTR	pStringVal;
					DBFHandle    pDBF;
					LPDWORD 	 pDBFAddress = (LPDWORD)GlobalLock (FilePtr->FileHandle);
					
					pDBF = (DBFHandle)*pDBFAddress; 
					GlobalUnlock (FilePtr->FileHandle);   
					if (!_fstrnicmp (SQLPtr->SQL,"SHAPEREC =",10))
					{
						_fstrcpy (str,&SQLPtr->SQL[10]);
						ExpandText (str);
						CurrentSHPRec = atol (str);
					}
					else if (!_fstrnicmp(SQLPtr->SQL, "%SHAPEREC=", 10))
					{
						_fstrcpy(str, &SQLPtr->SQL[10]);
						ExpandText(str);
						CurrentSHPRec = atol(str);
					}
					else if (!_fstrnicmp(SQLPtr->SQL, "%SHPRECNO=", 10))
					{
						_fstrcpy(str, &SQLPtr->SQL[10]);
						ExpandText(str);
						CurrentSHPRec = atol(str);
					}
					else if (!_fstrnicmp(SQLPtr->SQL, "%DBFREC=", 8))
					{
						_fstrcpy(str, &SQLPtr->SQL[8]);
						ExpandText(str);
						CurrentDBFRec = atol(str);
					}
					if (FilePtr->Type == SHAPE_DATAFILE)
						CurrentRec = CurrentSHPRec;
					else
						CurrentRec = CurrentDBFRec;
					switch (lpFieldInfo->type)
					{   
						default:   
						case BT_RIGHT_CHAR:
						case BT_CHAR:
							pStringVal = DBFReadStringAttribute(pDBF,CurrentRec, lpFieldInfo->index);  
				        	if (pStringVal)
				        		_fstrcpy (Value,pStringVal);
			        	break;
							        	
			        	case BT_INTEGER:
							LongVal = DBFReadIntegerAttribute(pDBF,CurrentRec, lpFieldInfo->index); 
							ltoa (LongVal,Value,10);
				     	break;
						 			
			 			case BT_REAL:
							DoubleVal = DBFReadDoubleAttribute(pDBF,CurrentRec, lpFieldInfo->index);
			 				sprintf(Value,"%.14lg",DoubleVal);
			 			break;
		        	} 
					goto GotData;
				}
		    	break;

				case LISTVAR_DATAFILE:
				{
					LPLISTVARDATABASE	pDB;   
	
				    pDB = (LPLISTVARDATABASE)GlobalLock (FilePtr->FileHandle);
					strcpy (Value,pDB->CurRow);
					GlobalUnlock (FilePtr->FileHandle);
					goto GotData;
				}
				break;
		    	
		    	case GMTEXT_DATAFILE: 
		    	{
				    if (NeedRead (SQLPtr))
				    {    
				    	if (!_fstrnicmp (SQLPtr->SQL,"%RECORDOFFSET==",15))
						{   
							long	loc;
							
							_fstrcpy (str,&SQLPtr->SQL[15]);
							ExpandText (str);
							loc = atol (str);
					    	GSSillseek (FilePtr->Fid,loc,0);    
					    	fgetstring (str,4090,FilePtr->Fid);
	                        GetDelimTextData(str,FilePtr->FileHandle); 
	                        SQLPtr->st =0;
	                    }
	                    else
	                    {
					    	GSSillseek (FilePtr->Fid,0,0);    
					    	SQLPtr->st =0;
					    	fgetstring (str,4090,FilePtr->Fid);
							ConvertSQLToLogicP (pSQL,SQLPtr->SQL);
					    	do
					    	{
						    	SQLPtr->st =-999;
					    		SQLPtr->Offset = GSSillseek (FilePtr->Fid,0,1);  
					    		if (!fgetstring (str,4090,FilePtr->Fid))
					    			SQLPtr->st = 1;
					    		else
									GetDelimTextData(str,FilePtr->FileHandle);
							    SQLPtr->lastreadtime = NextVarTime ();
					    	}
					    	while (SQLPtr->st <= 0 && !LogicPFile (SQLPtr,pSQL,&err)); 
					    	if (SQLPtr->st == -999)
					    		SQLPtr->st = 0;
				    	}
				    }
	                if (SQLPtr->st > 0)
	                	goto NotFound; 
	                if (WantRecOffset)
	                	ltoa (SQLPtr->Offset,Value,10);
	                else
	                {
						LPSHORT	pnDLTvar=(LPSHORT)GlobalLock (FilePtr->FileHandle);
						short	nDLTvar=abs(*pnDLTvar); 
						LPSTR	DLTDelim = (LPSTR)(pnDLTvar + 1);
						LPHANDLE	DLTVar = (LPHANDLE)(DLTDelim + 1);
						VARPNT	VarPtr; 
						DLTVar += lpFieldInfo->index;  
						
						VarPtr = (VARPNT)GlobalLock (*DLTVar);
				    	_fstrncpy(Value,VarPtr->Value,maxlval); 
				    	GlobalUnlock (*DLTVar);
				    	GlobalUnlock (FilePtr->FileHandle);
				    }
				    
				    goto GotData;
				}
		    	break;
		    	
		    	case COMBO_DATAFILE:
			    {   
					LPCOMBOHEADER		pComboHeader;
				    LPCOMBOFILE  		pComboFile; 
	    			LPCOMBOFIELDINFO    pComboField;  
				    LPWHEREINDEX        pWhereIndex;
				    LPCFIELDINDEX       pCFieldIndex;  
				    LPSTR       		pWhere, pCField, pStr,pStr2;
				    HANDLE				hStr=0;
	    			short	fromfile; 
	    			DWORD	ReadTime;
			                           
					pComboHeader = (LPCOMBOHEADER)GlobalLock (FilePtr->FileHandle); 
					pComboFile = (LPCOMBOFILE)GlobalLock (pComboHeader->hComboFile); 
					CurrentComboFile = pComboFile;
	        		pComboField = (LPCOMBOFIELDINFO)GlobalLock (pComboHeader->hComboFields); 
	        		pComboField += ifield;
	        		fromfile = pComboField->fromfile;
					if (fromfile < 0)
					{   
						hStr = GSSiGlobAlloc ( 235,GMEM_MOVEABLE,4096);
						pStr = GlobalLock (hStr);
						pCFieldIndex =(LPCFIELDINDEX) GlobalLock(pComboHeader->hComputedFields);
						if (pComboFile->Version < 2)
						{
							pCField = (LPSTR)pCFieldIndex + (sizeof(CFIELDINDEX) +
		                        		pCFieldIndex->offset[pComboField->fromfileindex]);
						}
						else
						{
							LPINT	pIndex=(LPINT)pCFieldIndex;
							LPSTR	pFieldDefs = (LPSTR)(pIndex+pComboFile->NumFields);

							pCField = &pFieldDefs[pIndex[pComboField->fromfileindex]];
						}
		                _fstrcpy (pStr,pCField);
		                GlobalUnlock (pComboHeader->hComputedFields); 
		                ExpandText (pStr);
		                _fstrcpy (Value,pStr);   
		                GSSiGlobUlFree (&hStr);
						GlobalUnlock (pComboHeader->hComboFields);
						GlobalUnlock (pComboHeader->hComboFile);  
						CurrentComboFile = 0;
						GlobalUnlock (FilePtr->FileHandle); 
			        	goto GotData;
	                }
	                else
	                {
		                SQLPtr2 = (LPOPENSQLDATA)GlobalLock(pComboFile->hSQL[fromfile]);
		                FilePtr2 = (LPOPENFILEDATA)GlobalLock (SQLPtr2->OFHandle);
		                pFileField = &FilePtr2->FldInfo+pComboField->fromfileindex; 
						hStr = GSSiGlobAlloc ( 236,GMEM_MOVEABLE,1024);
						pStr = GlobalLock (hStr);
						pStr2 = pStr + 512;
						sprintf (pStr,"%s.%s",SQLPtr2->IDName,pFileField->name);
						if (fromfile > 0)
						{   
							pWhere = GlobalLock(pComboHeader->hWhere);
							pWhereIndex = (LPWHEREINDEX)pWhere;  
							pWhere += sizeof(WHEREINDEX) + pWhereIndex->offset[pComboField->WhereID];
							if (FilePtr2->Type == UMIFS_DATAFILE || FilePtr2->Type == ORA_DATAFILE || FilePtr2->Type == GMCENSUS_DATAFILE || FilePtr2->Type == GMTEXT_DATAFILE)
							{   
				                {
				                	LPOPENSQLDATA SQLPtrLink = (LPOPENSQLDATA)GlobalLock(pComboFile->hSQL[fromfile-1]);
				                	LPOPENFILEDATA FilePtrLink = (LPOPENFILEDATA)GlobalLock (SQLPtrLink->OFHandle);

				                	sprintf (pStr2,"[%s]",FilePtrLink->FldInfo.name);
				                	ExpandText (pStr2);
				                	GlobalUnlock (SQLPtrLink->OFHandle);
				                	GlobalUnlock (pComboFile->hSQL[fromfile-1]);
				                }
								_fstrcpy (pStr2,pWhere);
								ExpandText (pStr2); 
								ProcessFileSQL (SQLPtr2,FilePtr2,pStr2);
			                	SQLPtr3 = (LPOPENSQLDATA)GlobalLock(pComboFile->hSQL[0]);
								SQLPtr2->st = SQLPtr3->st; 
								GlobalUnlock(pComboFile->hSQL[0]);
								if (SQLPtr2->st > 0) 
								{
									GlobalUnlock (pComboHeader->hWhere);
									GlobalUnlock (SQLPtr2->OFHandle);
									GlobalUnlock(pComboFile->hSQL[fromfile]); 
					                GSSiGlobUlFree (&hStr);
									GlobalUnlock (pComboHeader->hComboFields);
									GlobalUnlock (pComboHeader->hComboFile);
									CurrentComboFile = 0;
									GlobalUnlock (FilePtr->FileHandle); 
									goto NotFound;
								}
								else
									SQLPtr2->lastreadtime = 0;
							}
							else
							{                                       
								_fstrcpy (SQLPtr2->SQL,pWhere); 
								SQLPtr2->NumGlobals = -1;
							}
							GlobalUnlock (pComboHeader->hWhere);
						} 
						else
							ReadTime = SQLPtr2->lastreadtime;
						GlobalUnlock(pComboFile->hSQL[fromfile]); 
						st = GetValFromOpenFiles (pStr,Value,4096); 
						if (!fromfile)
						{ 
							USHORT	j;
							
							for (j=1;j<pComboFile->NumFiles;j++)
							{ 
				                LPOPENSQLDATA SQLPtr3 = (LPOPENSQLDATA)GlobalLock(pComboFile->hSQL[j]);  
				                
				                if (SQLPtr3->lastreadtime < SQLPtr2->lastreadtime)
				                	SQLPtr3->lastreadtime = 0;	
				                GlobalUnlock (pComboFile->hSQL[j]);
							}
						}
						GlobalUnlock (SQLPtr2->OFHandle);
		                GSSiGlobUlFree (&hStr);
						GlobalUnlock (pComboHeader->hComboFields);
						GlobalUnlock (pComboHeader->hComboFile);
						CurrentComboFile = 0;
						GlobalUnlock (FilePtr->FileHandle); 
			        	goto GotData;
	                }
					GlobalUnlock (pComboHeader->hComboFields);
					GlobalUnlock (pComboHeader->hComboFile);
					CurrentComboFile = 0;
					GlobalUnlock (FilePtr->FileHandle); 
					if (!fromfile)
						goto Exit; 
			    }
		    	break;  
		    	
				case PN_DATAFILE:
		    	case GMCENSUS_DATAFILE:
				case ORA_DATAFILE:
				case UMIFS_DATAFILE:
				{    
					short	FirstCond = BT_EQ;
					
 					lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 
					if (SQLPtr->IndexToUse < 0) goto NotFound;
					hBT = lpGWDHead->BTHandle[SQLPtr->IndexToUse];
				    if (!hBT) 
		        	{ 
		        	    GlobalUnlock (FilePtr->FileHandle); 
		        		goto NotFound;
		        	}
				    
				    if (NeedRead (SQLPtr))
				    {   			    
				        if (lpGWDHead->Version > 1000)
				        {   
						    SQLPtr->lastreadtime = NextVarTime ();
				        	if (!LoadInternalGMD (lpGWDHead,InternalRefno))
				        	{ 
				        	    GlobalUnlock (FilePtr->FileHandle); 
				        		goto NotFound;
				        	}
				            st = 0;  
				           	goto Found;
				        }
					    if (!FetchDBRec (SQLPtr->myhandle))
			        	{ 
			        	    GlobalUnlock (FilePtr->FileHandle); 
			        		goto NotFound;
			        	} 
			        	NumFetch++;
				        /*SQLPtr->lastreadtime = NextVarTime ();  
				        GWDClearSetValues (lpGWDHead);
					    for (i=0,lpSQLField=&SQLPtr->SQLField;i<SQLPtr->NumGlobals;i++,lpSQLField++) 
					    {   
					    	if (lpSQLField->FieldNum >= 0 && lpSQLField->OpCode == OPCODE_EQ)
					    	{
						    	_fstrcpy (str,lpSQLField->String);
						    	ExpandText (str);
						    	lpGWFldInfo=lpGWDHead->pFldInfo + lpSQLField->FieldNum;
								SetFieldValFromChar(lpGWDHead,lpGWFldInfo,str,FALSE); 
							} 
						}
						if (GWDInitUnsetKeyValues (lpGWDHead,SQLPtr->IndexToUse))
							FirstCond = BT_GE;
						GWDFormKey(lpGWDHead,SQLPtr->IndexToUse,TRUE,0);
						if (SQLPtr->IndexToUse) 
							SetReadSecIndex(TRUE);
						SQLPtr->st = BT_FIND (hBT,lpGWDHead->pKeys[SQLPtr->IndexToUse],BT_FIRST,FirstCond, (LPSTR)&SQLPtr->Offset);
						SetReadSecIndex(FALSE); */
					    SQLPtr->lastreadtime = NextVarTime ();
					} 
			        if (lpGWDHead->Version > 1000)
                    	goto Found;
				    if (!SQLPtr->st)
				       	FillGWDData (lpGWDHead,SQLPtr->Offset);
				    else
				    {
		        	    GlobalUnlock (FilePtr->FileHandle); 
				    	goto NotFound;
				    }
	Found:			    
	                if (WantRecOffset)
	                	ltoa (SQLPtr->Offset,Value,10); 
	                else
	                {
						lpGWFldInfo=lpGWDHead->pFldInfo + ifield;
						switch (lpGWFldInfo->Type)
						{   
							default:   
							case BT_RIGHT_CHAR:
							case BT_CHAR:
								{
									int	ln=min(maxlval-1,lpGWFldInfo->Len);
					        		
									memmove (Value,&lpGWDHead->GWDData[lpGWFldInfo->Beg],ln);
					        		ep = &Value[ln]; 
									*ep ='\0';
								}
					        	//Truncate (Value);
				        	break;
								        	
				        	case BT_INTEGER:
				        	    if (lpGWFldInfo->Len == 2)
				        	    	IVal= *(LPSHORT) &lpGWDHead->GWDData[lpGWFldInfo->Beg];
				        	    else
					     	    	IVal = *(LPLONG) &lpGWDHead->GWDData[lpGWFldInfo->Beg];
					     	    ltoa (IVal,Value,10);
					     	break;
							 			
				 			case BT_REAL:
				 			 	if (lpGWFldInfo->Len == 4)
				 					sprintf(Value,"%f",*(LPFLOAT)&lpGWDHead->GWDData[lpGWFldInfo->Beg]);
				 				else
				 					sprintf(Value,"%.14lg",*(LPDOUBLE)&lpGWDHead->GWDData[lpGWFldInfo->Beg]);
				 			break;
			        	} 
			        	if (ValueInExtendedArea && FilePtr->Type == GMCENSUS_DATAFILE)
			        	{   
			        		long	CensusOffset = atol (Value);
			        		
			        		if (CensusOffset < 0) 
			        		{
								GlobalUnlock (FilePtr->FileHandle); 
			        			goto NotFound; 
			        		}
							GSSillseek (lpGWDHead->Fid,CensusOffset,0);
							BigRead (lpGWDHead->Fid,(HPSTR)&len,2); 
							_fstrcpy (Value,"0");
							if (len)
							{   
								HANDLE	hCensusString=GSSiGlobAlloc ( 237,GMEM_MOVEABLE,4096);
								LPSTR	pComma, pEnd, pCensusString = GlobalLock (hCensusString);
								USHORT	field = SFFieldData.Field;
								
								BigRead (lpGWDHead->Fid,pCensusString,len);	
								pCensusString[len] = 0;  
								ExpandCensusString (pCensusString);
								pComma = pCensusString;
								while (field--)
								{
									if (!(pComma = _fstrchr (pComma,',')))
										break;
									pComma++;
								}
								if (pComma)
								{
									if ((pEnd = _fstrchr (pComma,',')))
										*pEnd = 0;
									_fstrcpy (Value,pComma);
								}
								GSSiGlobUlFree (&hCensusString);
							}
			        	}
			        }
		        	GlobalUnlock (FilePtr->FileHandle);  
		        	goto GotData;
				}
				break;
				
				case FGDB_DATAFILE:
				{
				    if (NeedRead (SQLPtr))
				    {     
					    SQLPtr->lastreadtime = NextVarTime ();
				    	if (SQLPtr->hstmt)
				    	{
							FGDBCloseCursor((int)SQLPtr->hstmt);
				        	FGDBFreeStmt((int)SQLPtr->hstmt);
				        	SQLPtr->hstmt = 0;
						    ClearCurVals (FilePtr);
				        } 
				        SQLPtr->st = 0;
	
				    }
	                if (SQLPtr->st)
	                	goto NotFound;
					{
						HANDLE hsql = GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
						LPSTR sql = GlobalLock (hsql);
						strcpy (sql,SQLPtr->SQL);
						ExpandText (sql);
						ValC = (LPSTR) GetFGDBFieldData (FilePtr,
														 sql,&SQLPtr->hstmt,
	                                   		   			 lpFieldInfo, FALSE,0, &irc,FilePtr->NumFields,
	                                   		   			 &FilePtr->FldInfo,0);
						GSSiGlobUlFree (&hsql);
					}
	                if (irc)
	                	goto NotFound;
				    _fstrcpy(Value,ValC);
				    goto GotData;
				}
				break;
				case PGDB_DATAFILE:
				case ODBC_DATAFILE:
				case SQL_DATAFILE:
				case TEXT_DATAFILE:
				{   
				    if (NeedRead (SQLPtr))
				    {     
					    SQLPtr->lastreadtime = NextVarTime ();
				    	if (SQLPtr->hstmt)
				    	{
							SQLCloseCursor(SQLPtr->hstmt);
				        	SQLFreeStmt(SQLPtr->hstmt, SQL_DROP);
				        	SQLPtr->hstmt = 0;
						    ClearCurVals (FilePtr);
				        } 
				        SQLPtr->st = 0;
	
				    }
	                if (SQLPtr->st)
	                	goto NotFound;
					ValC = (LPSTR) GetExternalFieldData (FilePtr,
														 SQLPtr->SQL,&SQLPtr->hstmt,
	                                   		   			 lpFieldInfo, FALSE,0, &irc,FilePtr->NumFields,
	                                   		   			 &FilePtr->FldInfo);
	                if (irc)
	                	goto NotFound;
				    _fstrcpy(Value,ValC);
				    goto GotData;
				
				}
			}
NotFound:	SQLPtr->st = 31;
			ExpandTextDataNotFound=TRUE;
			*Value=0;
Exit:		GlobalUnlock (SQLPtr->OFHandle);
			GlobalUnlock (*lpFileHandle);
		}
	}
	GlobalUnlock (FilePathHandle); 
	}
GetOut:
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (578);
#endif
	return rtn;    
}
	
GotData:
	GlobalUnlock (SQLPtr->OFHandle);
	GlobalUnlock (*lpFileHandle);
	GlobalUnlock (FilePathHandle);
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (578);
#endif
    return (_fstrlen(Value));
}
#if ENABLETRACE
}
#endif
} 

short GetDBType (HANDLE hDB)
{
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	short	Type; 
    
    if (!hDB) 
    	return 0;
	SQLPtr = (LPOPENSQLDATA)GlobalLock(hDB);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	Type = FilePtr->Type;
	GlobalUnlock (SQLPtr->OFHandle);  
	GlobalUnlock (hDB);
    return Type;
}

BOOL GetDBFieldInfo (LPFIELDINFO pFieldInfo,HANDLE hDB)
#if ENABLETRACE
{GSSiEnterProg (579);
#endif
{
	LPFIELDINFO	lpFieldInfo;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr; 
	BOOL	rtn=FALSE;
	short	ifield;
    
    if (!hDB)
{
#if ENABLETRACE
GSSiExitProg (579);
#endif
    	return FALSE;
}
	SQLPtr = (LPOPENSQLDATA)GlobalLock(hDB);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
	lpFieldInfo = &FilePtr->FldInfo;
	for (ifield=0;ifield<FilePtr->NumFields;ifield++,lpFieldInfo++)
	{
		if (!_fstricmp (lpFieldInfo->name,pFieldInfo->name))
		{
			*pFieldInfo = *lpFieldInfo; 
			rtn = TRUE;
			goto Exit;
		}
	}
Exit: 
	GlobalUnlock (SQLPtr->OFHandle);  
	GlobalUnlock (hDB);
{
#if ENABLETRACE
GSSiExitProg (579);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}
	
BOOL GetFieldDefFromOpenFiles (LPSTR VarName,LPFIELDINFO lpField)
#if ENABLETRACE
{GSSiEnterProg (580);
#endif
{   
	LPGWFLDINFO	lpGWFldInfo;
	LPGWDHEADER	lpGWDHead;
	HANDLE		hBT;
	long		Offset, IVal; 
	BOOL		rtn;
	char		str[256];
	int			st, i, j, k, len, isql, ifield, irc; 
	LPSTR		ep, ValC;
	LPFIELDINFO	lpFieldInfo;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	LPSQLFIELD	lpSQLField;
	LPFILEPATH	FilePathPtr;
	LPHANDLE	lpFileHandle;
	 
	rtn = FALSE; 
	if (!FilePathHandle)
{
#if ENABLETRACE
GSSiExitProg (580);
#endif
		return (rtn); 
}
	FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
	lpFileHandle = &FilePathPtr->FileHandle;

	for (isql=0;isql<FilePathPtr->NumFiles;isql++,lpFileHandle++)
	{
		if (*lpFileHandle)
		{   
			SQLPtr = (LPOPENSQLDATA)GlobalLock(*lpFileHandle);
			FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
			lpFieldInfo = &FilePtr->FldInfo;
			for (ifield=0;ifield<FilePtr->NumFields;ifield++,lpFieldInfo++)
			{
				if (!_fstricmp (lpFieldInfo->name,VarName))
					goto GetData;
			}
			goto Next;
	GetData: 
		    rtn = TRUE;  
			*lpField = *lpFieldInfo;
			
Next:		GlobalUnlock (SQLPtr->OFHandle);
			GlobalUnlock (*lpFileHandle); 
			if (rtn) goto Exit;
		}
	}
Exit:
	GlobalUnlock (FilePathHandle);
{
#if ENABLETRACE
GSSiExitProg (580);
#endif
	return (rtn);    
}
#if ENABLETRACE
}
#endif
}   

LPOPENFILEDATA InOpenFileList (LPSTR Name,int Type,int Access)
#if ENABLETRACE
{GSSiEnterProg (581);
#endif
{   
 	int i;
	LPFILEPATH	FilePathPtr;
	LPHANDLE	lpFileHandle,lpFileHandle2; 
	LPOPENSQLDATA	SQLPtr;
	LPSQLFIELD	lpSQLField;   
	LPOPENFILEDATA	FilePtr;
    
    if (!FilePathHandle || !_fstrnicmp (Name,"ODBC|",5))  
{
#if ENABLETRACE
GSSiExitProg (581);
#endif
    	return NULL;
}
    FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
    lpFileHandle = &FilePathPtr->FileHandle; 
    for (i=0;i<FilePathPtr->NumFiles;i++,lpFileHandle++)
    {
		if (*lpFileHandle)
		{
			SQLPtr = (LPOPENSQLDATA)GlobalLock (*lpFileHandle);
            FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
			if (!_fstricmp (FilePtr->fullpath,Name))
			{
				if (Type == UMIFS_DATAFILE && SQLPtr->Access == Access && Access == BT_READ)
				{
					GlobalUnlock (SQLPtr->OFHandle); 
					FilePtr = NULL;
				}
				GlobalUnlock (*lpFileHandle);
				GlobalUnlock (FilePathHandle);
{
#if ENABLETRACE
GSSiExitProg (581);
#endif
				return FilePtr;
}
			}
			GlobalUnlock (SQLPtr->OFHandle);
			GlobalUnlock (*lpFileHandle);
		} 
    }
	GlobalUnlock (FilePathHandle);
{
#if ENABLETRACE
GSSiExitProg (581);
#endif
	return NULL;
}
#if ENABLETRACE
}
#endif
}

void CloseMacroFiles (long ThisMacro)
#if ENABLETRACE
{GSSiEnterProg (582);
#endif
{
 	int i;
	LPFILEPATH	FilePathPtr;
	LPHANDLE	lpFileHandle,lpFileHandle2; 

Top:
    if (!FilePathHandle)  
{
#if ENABLETRACE
GSSiExitProg (582);
#endif
    	return;
}
    FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
    lpFileHandle = &FilePathPtr->FileHandle; 
    for (i=0;i<FilePathPtr->NumFiles;i++,lpFileHandle++)
    {   
   		HANDLE			hSQL = *lpFileHandle;
		LPOPENSQLDATA	SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
		LPOPENFILEDATA	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
		long			MacroID = SQLPtr->MacroID;
        
        GlobalUnlock (SQLPtr->OFHandle);
        GlobalUnlock (hSQL);
    	if (MacroID == ThisMacro)
    	{
			GlobalUnlock (FilePathHandle); 
			CloseDataFile (FALSE,&hSQL);
			goto Top;
		}
    }
	GlobalUnlock (FilePathHandle);
{
#if ENABLETRACE
GSSiExitProg (582);
#endif
	return;    
}
#if ENABLETRACE
}
#endif
}
		
BOOL AddToOpenFileList (HANDLE hSQL)
#if ENABLETRACE
{GSSiEnterProg (583);
#endif
{ 	int i;
	LPFILEPATH	FilePathPtr; 
	HANDLE	SaveHandle;
	LPHANDLE	pHandle;

    if (FilePathHandle)
    {   
	    FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle); 
	    if (FilePathPtr->NumFiles >= MAXFILESINPATH)
	    	BlowOut ("Number of open files exceeds limit","Error");	
	    pHandle = &FilePathPtr->FileHandle;
	    pHandle += FilePathPtr->NumFiles++;
	    *pHandle = hSQL;
    }
    else
    {  
	    SaveHandle = FilePathHandle;
	    FilePathHandle = GSSiGlobAlloc (1756,GHND,sizeof(HANDLE)+sizeof(int)+MAXFILESINPATH*sizeof(HANDLE));
	    FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
	    FilePathPtr->LastPathHandle = SaveHandle;
	    FilePathPtr->FileHandle=hSQL;
	    FilePathPtr->NumFiles=1; 
	}
    GlobalUnlock(FilePathHandle);
{
#if ENABLETRACE
GSSiExitProg (583);
#endif
	return(TRUE);
}
#if ENABLETRACE
}
#endif
}

void RemoveFromOpenFileList (HANDLE hFile)
#if ENABLETRACE
{GSSiEnterProg (584);
#endif
{
 	int i;
	LPFILEPATH	FilePathPtr;
	LPHANDLE	lpFileHandle,lpFileHandle2; 

    if (!FilePathHandle)  
{
#if ENABLETRACE
GSSiExitProg (584);
#endif
    	return;
}
    FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
    lpFileHandle = &FilePathPtr->FileHandle; 
    for (i=0;i<FilePathPtr->NumFiles;i++,lpFileHandle++)
    {
    	if (*lpFileHandle == hFile)
    		goto Remove;
    }
	GlobalUnlock (FilePathHandle);
{
#if ENABLETRACE
GSSiExitProg (584);
#endif
	return;    
}

Remove:	
	if (FilePathPtr->NumFiles==1)
	{   
		HANDLE	LastHandle;
				
		LastHandle = FilePathPtr->LastPathHandle; 

		GSSiGlobUlFree (&FilePathHandle);
		FilePathHandle = LastHandle;  
		if (!FilePathHandle)
			ODBCTerminate (FALSE);
	}
	else if (i == (FilePathPtr->NumFiles-1)) 
	{ 
		FilePathPtr->NumFiles--;
		GlobalUnlock (FilePathHandle);
	}                  
	else
	{
		for (i++,lpFileHandle2=lpFileHandle++;i<FilePathPtr->NumFiles;i++,lpFileHandle++,lpFileHandle2++)
		{
			*lpFileHandle2 = *lpFileHandle;
		}
		FilePathPtr->NumFiles--;
		GlobalUnlock (FilePathHandle);
	}
{
#if ENABLETRACE
GSSiExitProg (584);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void ListGlobals (HWND hWndDlg,WORD Control,BOOL Expand)
#if ENABLETRACE
{GSSiEnterProg (585);
#endif
{
	unsigned short	i = pVarSpace->NumVars;
	VARPNT	VarPtr;  
	char	str[1024]; 
	
	while (i--)
	{   
		VarPtr = (VARPNT)GlobalLock(pVarSpace->VarHandles[i]);
		sprintf (str,"%s\t%s",VarPtr->Name,VarPtr->Value); 
		if (Expand)
			ExpandText (str);  
        SendDlgItemMessage (hWndDlg,Control,LB_ADDSTRING,0,(LPARAM)((LPSTR)str));
		GlobalUnlock(pVarSpace->VarHandles[i]);
	} 
{
#if ENABLETRACE
GSSiExitProg (585);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void dumpvars (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (586);
#endif
{   
	unsigned short	i = pVarSpace->NumVars;
	VARPNT	VarPtr;  
	char	str[128]; 
	OFSTRUCTGM	OFStruct;
	HFILE	Fid;
	
	Fid = GSSiOpenFile (Name,&OFStruct,OF_CREATE);

	while (i--)
	{   
		VarPtr = (VARPNT)GlobalLock(pVarSpace->VarHandles[i]);
		sprintf(str, "%ld %s:%s", (long)pVarSpace->VarHandles[i], VarPtr->Name, VarPtr->Value);
		fputstring (str,Fid);
		GlobalUnlock(pVarSpace->VarHandles[i]);
	} 
	GSSiClose (Fid);
{
#if ENABLETRACE
GSSiExitProg (586);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

HANDLE FindVar (LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (587);
#endif
{   
	short	BegID,MidID,EndID;
	short	i; 
	VARPNT	VarPtr;
	LPVARNAMEINDEXITEM	lpVN; 
	HANDLE	rtn=NULL; 
	HANDLE	hVarSpace=SetVarSpaceFromName(Name);
    
	if (!hVarSpace)
	{
#if ENABLETRACE
		GSSiExitProg(587);
#endif
		return (NULL);
	}
	pVarSpace = GlobalLock(hVarSpace);
	if (!pVarSpace->hVarNameTable)
	{
		GlobalUnlock(hVarSpace);
		{
#if ENABLETRACE
			GSSiExitProg (587);
#endif
			return (NULL);
		}
	}
    if (!_fstrnicmp (Name,"G.",2))
    	Name += 2;
	lpVN = (LPVARNAMEINDEXITEM)GlobalLock(pVarSpace->hVarNameTable);
	BegID = 0;
	EndID = pVarSpace->NumVars - 1;
Start:
	if (EndID < BegID)
		goto Exit;
	MidID = (BegID + EndID)/2;
	i = _fstricmp (Name,lpVN[MidID].Name);
	if (i < 0)
	{ 
		EndID = MidID - 1; 
		goto Start;
	}
	if (i > 0)
	{
		BegID = MidID + 1; 
		goto Start;
	}
	else
		rtn = pVarSpace->VarHandles[lpVN[MidID].id];
Exit:
	GlobalUnlock(pVarSpace->hVarNameTable);
	GlobalUnlock(hVarSpace);
	{
#if ENABLETRACE
GSSiExitProg (587);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

void CloseVars (void)
#if ENABLETRACE
{GSSiEnterProg (588);
#endif
{   int	i,ii;
	VARPNT	VarPtr; 

	for (i = 0; i<pVarSpace->NumVars; i++)
	{   
		if ((VarPtr = (VARPNT)GlobalLock(pVarSpace->VarHandles[i])))
		{
			if (VarPtr->ValueIsHandle)
			{
				HANDLE handle = (HANDLE)atol (VarPtr->Value);
				GSSiGlobFree (&handle);
			}
			GSSiGlobUlFree(&pVarSpace->VarHandles[i]);
		}
		else
			pVarSpace->VarHandles[i] = 0;
	}
	DestroyVarNameTable (); 
	pVarSpace->NumVars = 0;
{
#if ENABLETRACE
GSSiExitProg (588);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

 
COLORREF SetColor (LPSTR pColor)
#if ENABLETRACE
{GSSiEnterProg (589);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (589);
#endif
	return 0;
}
#if ENABLETRACE
}
#endif
}
BOOL OpenGSStreetNames (int mode, int which)
#if ENABLETRACE
{GSSiEnterProg (590);
#endif
{   
	char	str[128]="[%DL][%STATE]\\";
	if (hNames2)
{
#if ENABLETRACE
GSSiExitProg (590);
#endif
		return TRUE;
}
	if (CurState >= 0)
	{   
		ExpandText (str);
		SetGlobalValue("%GEOSPAN_LOC",str);
		if (which >= 2)
		{
			hNames2 = BT_OPEN ("[%DL][%STATE]\\stname2.btr", 0, mode, 0);
			if (!hNames2 && mode == BT_WRITE) 
			{
				CreateStnameFilesUpdate ();
				hNames2 = BT_OPEN ("[%DL][%STATE]\\stname2.btr", 0, mode, 0);
		    }
		}
		if (which%2)
			hNames1 = BT_OPEN ("[%DL][%STATE]\\stname1.btr", 0, mode, 0);
{
#if ENABLETRACE
GSSiExitProg (590);
#endif
		return TRUE; 
}
	}
	else
	{
		hNames2 = BT_OPEN ("[%DL]stname2.btr", 0, mode, 0);
		if (!hNames2) 
		{
			CreateStnameFilesUpdate ();
			hNames2 = BT_OPEN ("[%DL]stname2.btr", 0, mode, 0);
	    }
		hNames1 = BT_OPEN ("[%DL]stname1.btr", 0, mode, 0);
{
#if ENABLETRACE
GSSiExitProg (590);
#endif
		return TRUE; 
}
	}
#if ENABLETRACE
}
#endif
} 

void CloseGSStreetNames(void)
#if ENABLETRACE
{GSSiEnterProg (591);
#endif
{
	BT_CLOSE (hNames1); 
	BT_CLOSE (hNames2);
	hNames1 = 0;
	hNames2 = 0;  
	HaveState = -1;
{
#if ENABLETRACE
GSSiExitProg (591);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

long GetCmdID (LPSTR lpCmdID,LPSTR MenuIDText)
#if ENABLETRACE
{GSSiEnterProg (592);
#endif
{
	char	str[132];
	long	ID;
	LPSTR	lpSpace, lpParen; 
	
	if (!lpCmdID)
	{
{
#if ENABLETRACE
GSSiExitProg (592);
#endif
		return 0;
}
	} 
	Truncate (lpCmdID);
	if (*lpCmdID == '"')
	{   
		LPLONG	pNumCmd, pCmdOffset;  
		LPSTR	pCmd, pEndQuote;
		long	NewOff;   
		int		l;
		
		pEndQuote = lpCmdID;
		pEndQuote++;
		pEndQuote = MatchLev (pEndQuote,'"');
		if (!pEndQuote)
{
#if ENABLETRACE
GSSiExitProg (592);
#endif
			return 0;
}
		*pEndQuote = 0;
		pNumCmd = (LPLONG)GlobalLock (*phWhichCmdList);
		(*pNumCmd)++;
		pCmdOffset = pNumCmd;
		pCmdOffset += *pNumCmd;
		if (*pNumCmd == 1)
		{
			*pCmdOffset = 1024*4 + 4;
		}
		else
		{	
			pCmdOffset--;
			pCmd = (LPSTR)pNumCmd;
			pCmd+=*pCmdOffset;
			l = _fstrlen (pCmd);
			NewOff = *pCmdOffset + l + 1;
			pCmdOffset++;
			*pCmdOffset = NewOff;
		}
		pCmd = (LPSTR)pNumCmd;
		pCmd+=*pCmdOffset;
		if (MenuIDText)
			sprintf (pCmd,"[%%MENUITEM]=%s;%s",MenuIDText,++lpCmdID);
		else
			_fstrcpy (pCmd,++lpCmdID); 
		REPLAC (pCmd,"[%ARG]",IncludeFileArg,_fstrlen(pCmd)+_fstrlen(IncludeFileArg));
		GlobalUnlock (*phWhichCmdList);
		if (phWhichCmdList == &hUserCmd)
{
#if ENABLETRACE
GSSiExitProg (592);
#endif
			return (58000+*pNumCmd);
}
		else			
{
#if ENABLETRACE
GSSiExitProg (592);
#endif
			return (59500+*pNumCmd);
}
	} 
		
	lpParen = _fstrchr (lpCmdID,'(');
	if (lpParen)
		*lpParen = 0;    
	ID = ConvertCMDToNum (lpCmdID);
	if (lpParen)
		*lpParen = '(';
{
#if ENABLETRACE
GSSiExitProg (592);
#endif
	return ID;
}
#if ENABLETRACE
}
#endif
}

BOOL ExecuteUserCmd (int CmdID)
#if ENABLETRACE
{GSSiEnterProg (593);
#endif
{ 
	LPLONG	pNumCmd, pCmdOffset;  
	LPSTR	pCmd, pEndQuote, pCommand;
	long	NewOff, l;
	HANDLE	handle;   
	BOOL	rtn=TRUE;
	
	if (!*phWhichCmdList)
{
#if ENABLETRACE
GSSiExitProg (593);
#endif
		return FALSE;	
}
	pNumCmd = (LPLONG)GlobalLock (*phWhichCmdList);    
	if (!pNumCmd)
{
#if ENABLETRACE
GSSiExitProg (593);
#endif
		return FALSE;  
}
	if (CmdID < 0 || CmdID > *pNumCmd)
{
#if ENABLETRACE
GSSiExitProg (593);
#endif
		return FALSE;
}
	pCmdOffset = pNumCmd;
	pCmdOffset += CmdID;
	pCmd = (LPSTR)pNumCmd;
	pCmd+=*pCmdOffset;
	l = _fstrlen (pCmd);    
	if (!l)
	{
		GlobalUnlock (*phWhichCmdList);
{
#if ENABLETRACE
GSSiExitProg (593);
#endif
		return FALSE;
}
	}
	handle = GSSiGlobAlloc ( 238,GMEM_MOVEABLE,l+10);
	pCommand = GlobalLock (handle); 
	_fstrcpy (pCommand,pCmd);
	GlobalUnlock (*phWhichCmdList);    
	
	if (*pCommand == '|')
		rtn = ExecuteCommandString (++pCommand);
	else
		AddGraphicsCmd (CurView->hWnd,pCommand,FALSE,0); 		
	GSSiGlobUlFree (&handle);         
{
#if ENABLETRACE
GSSiExitProg (593);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}
	
BOOL ExecuteCommandString (LPSTR pCmd)
#if ENABLETRACE
{GSSiEnterProg (594);
#endif
{
	BOOL	MoreCommands,rtn=TRUE; 
	LPSTR	EndCommand, pCommand;
	long	lc;  
	HANDLE	hCmd; 
	LPVIEWPORT	SaveVP =  CurView;
	           
	lc = _fstrlen (pCmd);
	if (!lc)
{
#if ENABLETRACE
GSSiExitProg (594);
#endif
		return TRUE;
}
	hCmd = GSSiGlobAlloc ( 239,GMEM_MOVEABLE,lc+1);
	pCommand = GlobalLock (hCmd);  
	_fstrcpy (pCommand,pCmd);
//	CurView = pViewports[CommandViewport-1]; //tempdebu
   	SetViewport(*pCommandViewport);
	
	MoreCommands =  TRUE;
	while (MoreCommands)
	{
		EndCommand = MatchLev (pCommand,';');
		if (!EndCommand)
		{
			MoreCommands = FALSE;
			EndCommand = _fstrchr (pCommand,0);
		} 
		*EndCommand = 0;
		if (!DoUserCommand (pCommand))  
		{
			rtn = FALSE;
			break;      
		}
		pCommand = EndCommand;
		pCommand++;
	} 
	GSSiGlobUlFree (&hCmd);
	if (ConfigLoaded) 
	{
		if (SaveVP->ID != CurView->ID)
			CurView = SaveVP;  
	}
{
#if ENABLETRACE
GSSiExitProg (594);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

BOOL DoUserCommand (LPSTR Cmd)
#if ENABLETRACE
{GSSiEnterProg (595);
#endif
{   
	UINT	CmdID;
	long	lCmdID; 
	HANDLE	hStr=GSSiGlobAlloc ( 240,GMEM_MOVEABLE,256);
	LPSTR	cmd=GlobalLock (hStr); 
	BOOL	rtn=TRUE;
	
	lCmdID = GetCmdID (Cmd,0);
	if (lCmdID < 0)
	{   
		CmdID = -lCmdID; 
		AddGraphicsFunction (CurView->hWnd, CmdID,0);
	}	
    else if (lCmdID)
    {
    	CmdID = lCmdID; 
		PostMessage(hWndMain, WM_COMMAND, CmdID, CurView->ID);
	}
    else  
    {
    	_fstrcpy (cmd,Cmd);
		ExpandText (cmd); 
		OneSpace (cmd);
		if (!_fstrcmp (cmd,"EXIT"))
			rtn = FALSE;
	}
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (595);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

double GetViewportScale (HDC hDC)
#if ENABLETRACE
{GSSiEnterProg (596);
#endif
{                
	double scale, WDist, IDist, BaseScale;
	int	iLogPixsX;   
	DPOINT	p1,p2; 
	long	OrthRes[10]={1,4,16,16,16,16,16,16,16,16}; 
	int		numOrthoLevs=3;
	short	ires; 
	char	str[256];
	LPSTR	lpComma;

	OrthScale=0;	
	if (!hDC)
{
#if ENABLETRACE
GSSiExitProg (596);
#endif
		return 0;
}
	iLogPixsX = GetDeviceCaps(hDC, LOGPIXELSX);
	if (!iLogPixsX)
{
#if ENABLETRACE
GSSiExitProg (596);
#endif
		return 0;   
}
	IDist = (double)(CurView->DrawRect.right - CurView->DrawRect.left)/(double)iLogPixsX;
    if (!IDist)  
{
#if ENABLETRACE
GSSiExitProg (596);
#endif
    	return 0;
}
	if (!PRJ_TYPE[1])
	{   
		p1.x = CurView->WBounds.xmn;
		p2.x = CurView->WBounds.xmx;
		p1.y = (CurView->WBounds.ymn + CurView->WBounds.ymx) / 2;
		p2.y = p1.y;
		WDist = ArcDistance(p1, p2);
	}
	else
		WDist = CurView->WBounds.xmx - CurView->WBounds.xmn;
    
	scale = WDist/IDist; 
	if (PRJ_TYPE[1] != 2)
		scale *= MFT; 
	scale /= 5280.0; 
    
    if (CurView->DesiredWidth) 
    	BaseScale = WDist/(CurView->DesiredWidth * (1.0-2*(CurView->Margin+max(0,CurView->BorderPct))/100));
    else
    	BaseScale = 0;
    SetGlobalValueReal ("%BASESCALE",BaseScale);
    _fstrcpy (str,"[%ORTHOLEVS]");
    ExpandText (str);
    if (*str)
    { 
		lpComma = str;
    	OrthRes[1]=OrthRes[2]=OrthRes[3]=OrthRes[4]=OrthRes[5]=OrthRes[6]=OrthRes[7]=OrthRes[8]=OrthRes[9]==999999999;
    	OrthRes[0] = atol (str);
		numOrthoLevs = 1;
    	while ((lpComma = _fstrchr (lpComma,',')))   
		{
    		lpComma++;
    		OrthRes[numOrthoLevs++] = atol (lpComma);
		}
    }    
GetScale:
    IDist = (double)(CurView->DrawRect.right - CurView->DrawRect.left);
    if (IDist)
		OrthScale = (WDist/BaseUnitsPerOrthoPixel)/IDist;
	ires = 0;
	if (numOrthoLevs > 1 && OrthScale >= OrthRes[1])
	{
		for (ires=1;ires<numOrthoLevs-1;ires++)
		{
			if (OrthScale <= OrthRes[ires+1]-(OrthRes[ires+1] - OrthRes[ires]) / 4.0)
				break;
		}
	}

	ires = min(numOrthoLevs-1, max(ires + GetGlobalLVal2("[%INCORTHORES]", 0), GetGlobalLVal2("[%MINORTHORES]", 0)));
	SetGlobalValueLong ("%ORTHORES",OrthRes[ires]);
	
{
#if ENABLETRACE
GSSiExitProg (596);
#endif
	return scale;
}
#if ENABLETRACE
}
#endif
}

void SaveGlobalVals (HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (601);
#endif
{
	unsigned short	i = pVarSpace->NumVars;
	LPSTR	pstr;
	VARPNT	VarPtr;
    HANDLE	hMem=GSSiGlobAlloc ( 241,GMEM_MOVEABLE,USHRT_MAX);
    LPSTR	pMem = GlobalLock (hMem);
    long	lMem=0; 
    short	nVars=0,l;
 	short	Length, Version=1, id=OB_SAVEGLOBALS; 
 	HANDLE	hStr=GSSiGlobAlloc ( 242,GMEM_MOVEABLE,USHRT_MAX);
 	LPSTR	str=GlobalLock (hStr); 
 	short	ii;  

 	BigWrite (Fid,(HPSTR)&id,2,-1);  
 	BigWrite (Fid,(HPSTR)&Version,2,-1);
 	BigWrite (Fid,(HPSTR)&Version,2,-1);
	while (i--)
	{   
		VarPtr = (VARPNT)GlobalLock(pVarSpace->VarHandles[i]);
		if (VarPtr)
		{    
	if (!_fstricmp (VarPtr->Name,"%PLOT"))
		ii=1;
			if (VarPtr->Save)
			{
				nVars++;
				_fstrcpy (pMem,VarPtr->Name);
				l = _fstrlen (VarPtr->Name);
				pMem += (l+1);
				lMem += (l+2);  
				*pMem = VarPtr->ContainsGorF;
				pMem++; 
				if (!VarPtr->ContainsGorF)
					GetGlobalVal(pVarSpace->VarHandles[i], str, 0);
				else
					_fstrcpy (str,VarPtr->Value);  
				SubstituteDL (str,TRUE);
				pstr = str;
				if (*str == literalChar)
					pstr++;
				_fstrcpy (pMem,pstr);  
				l = _fstrlen (pMem);
				pMem += (l+1);
				lMem += (l+1);
			}	
			GlobalUnlock(pVarSpace->VarHandles[i]);
		}
	} 
	BigWrite (Fid,(HPSTR)&nVars,2,-1);
	BigWrite (Fid,(HPSTR)&lMem,4,-1);
	GlobalUnlock (hMem);
	pMem = GlobalLock (hMem);
	BigWrite (Fid,(HPSTR)pMem,(size_t)lMem,-1); 
	GSSiGlobUlFree (&hMem);
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (601);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

long FilePos (LPSTR Cmd,LPSTR FileID,LPSTR Pos,int index)
{ 
	//$FILEPOS(GET,FILEID,CURRENT|RECORD|LAST
	//		  (SET,FILEID,FIRST|NEXT|PRIOR|LAST
	LPOPENSQLDATA	SQLPtr;
	LPOPENFILEDATA	FilePtr;
	long	rtn=-1;

   	HANDLE	hSQLPtr = GetDBByIDName (FileID);
	if (!hSQLPtr)
    	return 0; 
	SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQLPtr);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	if (!stricmp (Cmd,"GET"))
	{
		if (!stricmp (Pos,"CURRENT"))
			rtn = GSSillseek (FilePtr->Fid,0,1); 
		else if (!stricmp (Pos,"LAST"))
		{
			long	save=GSSillseek (FilePtr->Fid,0,1); 
			
			rtn = GSSillseek (FilePtr->Fid,0,2);
			GSSillseek (FilePtr->Fid,save,0);
		}			
	}
	if (!stricmp (Cmd,"SET") && FilePtr->Type == UMIFS_DATAFILE)
	{
		LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 
		int	pos;

		if (!stricmp (Pos,"PRIOR"))
			pos = BT_PRIOR;
		else if (!stricmp (Pos,"NEXT"))
			pos = BT_NEXT;
		else if (!stricmp (Pos,"FIRST"))
			pos = BT_FIRST;
		else if (!stricmp (Pos,"LAST"))
			pos = BT_LAST;
		rtn = FALSE;
		if (index)
			SQLPtr->IndexToUse = index - 1;
		if (!BT_FIND (lpGWDHead->BTHandle[SQLPtr->IndexToUse],lpGWDHead->pKeys[SQLPtr->IndexToUse],pos,BT_ANY,(LPSTR)&SQLPtr->Offset))
		{
			FillGWDData (lpGWDHead,SQLPtr->Offset);
			SQLPtr->st = -999;
			rtn = TRUE;
		}
		else
			SQLPtr->st = 1;
		GlobalUnlock (FilePtr->FileHandle);
	}
	GlobalUnlock (hSQLPtr);
	GlobalUnlock (SQLPtr->OFHandle);
	return rtn;
}

int	GetUpdateFieldType (LPSTR SetFieldName)
{
	LPSTR ploc = lpUpdateFieldList;
	int	l = strlen (SetFieldName);

	if (!lpUpdateFieldList)
		return 0;
	while (*ploc)
	{
		if (!strnicmp (SetFieldName,ploc,l))
		{
			char	end = *(ploc + l);

			if (!end || end == ';')
				return 1;
			if (end == '(')
				return 2;
			if (end == '$')
				return 3;
		}
		if (!(ploc = strchr (ploc,';')))
			ploc = strchr (lpUpdateFieldList,0);
		else
			ploc++;
	}
	return 0;
}

BOOL GetUpdateFieldValue (HWND hWndDlg,LPSTR SetFieldName,LPSTR NewValue)
{
	LPSTR ploc = lpUpdateFieldList, pSC;
	int	l = strlen (SetFieldName);
	BOOL	rtn;

	if (!lpUpdateFieldList)
		return FALSE;
	while (*ploc)
	{
		if (!strnicmp (SetFieldName,ploc,l))
		{
			char	end = *(ploc + l);

			if (!end || end == ';')
				return FALSE;
			if (end == '(')
			{
				HMENU	Menu = CreatePopupMenu(); 
				POINT	position;
				int		Choice=1, lv;
				HANDLE	hValues = GSSiGlobAlloc (0,GHND,USHRT_MAX);
				LPSTR	Values = GlobalLock (hValues);
			    
				ploc += l + 1;
				if ((pSC = strchr (ploc,';')))
					*pSC = 0;
				while (ploc && *ploc)
				{
					LPSTR pval = ploc, pEnd = strchr (ploc,',');

					if (!pEnd)
						pEnd = strchr (ploc,')');
					if (pEnd)
					{
						lv = pEnd - ploc;
						if (*pEnd == ')')
							ploc = 0;
						else
							ploc = pEnd + 1;
					}
					else
					{
						lv = strlen (ploc);
						ploc = 0;
					}
					strncpy0 (Values,pval,lv);
					AppendMenu (Menu,MF_ENABLED|MF_STRING,Choice++,Values);
					Values += lv+1;
				}
				if (pSC)
					*pSC = ';';
				GlobalUnlock (hValues);
				AppendMenu (Menu,MF_ENABLED|MF_STRING|MF_SEPARATOR,0,"");
				AppendMenu (Menu,MF_ENABLED|MF_STRING,0,"Cancel");
			   	GetCursorPos (&position);  
		  		Choice = TrackPopupMenu (Menu,TPM_CENTERALIGN|TPM_LEFTBUTTON|TPM_RETURNCMD,position.x,position.y,0,hWndDlg,0);
			  	DestroyMenu (Menu);
				Values = GlobalLock (hValues);
				if (!Choice)
					rtn = FALSE;
				else
				{
					while (Choice-- > 1)
						Values = strchr (Values,0) + 1;
					strcpy (NewValue,Values);
					if (!stricmp (NewValue,"Enter manually"))
					{
						*NewValue = 0;
						rtn = GetTextString (hWndDlg,NewValue,1024,"Enter value",0,0,0,TRUE,TRUE);
					}
					else
						rtn = TRUE;
				}
				GSSiGlobUlFree (&hValues);
				return rtn;
			}
		}
		if (!(ploc = strchr (ploc,';')))
			ploc = strchr (lpUpdateFieldList,0);
		else
			ploc++;
	}
	return FALSE;
}
