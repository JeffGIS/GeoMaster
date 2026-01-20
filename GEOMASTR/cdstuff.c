#include "graphint.h"


#include "gmextern.h"


static	HANDLE	hCancelledCDList=0, hCDLookUpTable=0;    
static	char	CurrentCD[32]="";


BOOL FileOnSystem (LPSTR Name)
{   
	char	NewName[MAX_PATH];
	
	_fstrcpy (NewName,Name);
	ExpandText (NewName);
//    if (ConvertNameToCDName (NewName))
//    	return TRUE;
	return ExistFile(Name);
} 

BOOL AddToCDList (LPSTR Prefix,LPSTR VolLabel)
{   
	int	num=0, l=1;
	char	str[128], addstr[128], CDnum[32]; 
	
	sprintf (addstr,"%s(%s)",Prefix,VolLabel); 
	do
	{   
		num++;
		sprintf (CDnum,"CD%i",num);
		l = GetPrivateProfileString ("CDList",CDnum,"",str,128,"geomastr.ini"); 
		if (!_fstricmp (addstr,str))
			return FALSE; 
	}
	while (l);
	WritePrivateProfileString ("CDList",CDnum,addstr,GMIni); 
	return TRUE;
}

BOOL CDIsRegistered  (LPSTR VolLabel)
{   
	int	l=1,num=0;
	char	str[128], CDnum[32];    
	LPSTR	pPar;
	
	do
	{   
		num++;
		sprintf (CDnum,"CD%i",num);
		l = GetPrivateProfileString ("CDList",CDnum,"",str,128,"geomastr.ini");
		if ((pPar = _fstrchr (str,'(')))
		{
			*pPar = 0;
			if ((pPar = _fstrrchr (str,'\\')))
			{
				*pPar++ = 0; 
				if (!_fstricmp (VolLabel,pPar))
					return TRUE;
			}
		}
	}
	while (l);
	return FALSE;
}

BOOL RegisterCD (LPSTR FromPath,LPSTR ToPath,LPSTR SearchString)
{ 
	HCURSOR	hcurSave;  
	char	Name[MAX_PATH], str2[150], file[MAX_PATH], lastpath[MAX_PATH], VolLabel[34], Prefix[128];
	HFILE	OutFileFID, Fid;  
	long	TotFiles=0;   
	OFSTRUCTGM	OFStruct;
	LPSTR	lpIdx, pRes, pBS;
	short	l=_fstrlen(FromPath), dnum;

Retry:	
//	dnum = GetDriveNum (*FromPath)-1;
	if (!GetVolumeLabel(FromPath,VolLabel)) 
	{
		sprintf (str2,"Unable to read CD in drive %s",FromPath);  
		if (GSSiMessageBox (0,str2,NULL,MB_RETRYCANCEL,0) == IDRETRY)
			goto Retry;
		return FALSE;
	}
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
	mkdir (ToPath);
	sprintf (file,"%s\\filelist.txt",ToPath); 
	if (ExistFile(file))   
	{
		OutFileFID = GSSiOpenFile (file,&OFStruct,OF_READWRITE);    
		GSSillseek (OutFileFID,0,2);
	}
	else
		OutFileFID = GSSiOpenFile (file,&OFStruct,OF_CREATE);
	if (OutFileFID == HFILE_ERROR)
		return FALSE;
	GSSiGetTempFileName(0,"gm",0,Name);
	Fid =	GSSiOpenFile (Name,(LPOFSTRUCTGM)&OFStruct,OF_CREATE);
	SearchFilesInDir (FromPath, "", Fid,&TotFiles,SearchString,1,TRUE,FALSE); 
	GSSillseek (Fid,0,0);
	*lastpath = 0; 
	while (fgetstring (str2,144,Fid))
	{   
		LPSTR pTAB = strchr (str2,'\t');

		if (pTAB)
			*pTAB = 0;
		sprintf (file,"%s\\%s.idx",ToPath,&str2[l]);
		makedirectories (file,FALSE,FALSE);
		lpIdx = _fstrstr (file,".idx");
		*lpIdx = 0;
		copyfile (file,str2,FALSE,0,0,0,0,0,0);
    	if (!_fstrchr(file,'.'))
    	{   
    		pRes = LastChr(file)-1;
    		if (*pRes == 'x' || *pRes == 'X')
    			pRes++;
  			_fstrcpy (pRes,"[%ORTHORES]"); 
			SubstituteDL (file,TRUE);
  			if (_fstricmp (file,lastpath)) 
  			{   
  				LPSTR	pFile=file;
  				if (*pFile == '@')
  					pFile++;
  				fputstring (pFile,OutFileFID);
				{   	
					_fstrcpy (Prefix,ToPath);
					if ((pBS = _fstrchr (&str2[l],'\\')))
						*pBS = 0;
					sprintf (Prefix,"%s\\%s",ToPath,&str2[l]);
					SubstituteDL (Prefix,TRUE);  
					pFile = Prefix;
	  				if (*pFile == '@')
	  					pFile++;
					AddToCDList (pFile,VolLabel);
				}
  			}	 
  			_fstrcpy (lastpath,file);
  		}
	}  
	GSSiClose2 (&Fid);
	GSSiClose2 (&OutFileFID);
	GSSiRemove (Name);
	GSSiSetCursor(hcurSave); 
	sprintf (str2,"CD %s successfully registered",VolLabel);
	GSSiMessageBox (0,str2,"",MB_OK,0);
	CloseCDLookUpTable ();    
	return TRUE;
} 


void OpenCDLookUpTable (void)
{
	int		num=0;
	long	l=0;
	char	str[128], CDnum[32]="CD1";   
	LPSTR	Prefix, VolLabel, pBegin, pPar, pEndPar;
	
	hCDLookUpTable = GSSiGlobAlloc(GAIDNO 1279,GHND,USHRT_MAX);
	Prefix = pBegin = GlobalLock (hCDLookUpTable);
	while (GetPrivateProfileString ("CDList",CDnum,"",str,128,"geomastr.ini"))
	{   
		num++;
		sprintf (CDnum,"CD%i",num);     
		ExpandText (str);
		pPar = _fstrchr (str,'(');
		*pPar++ = 0;
		pEndPar = _fstrchr (pPar,')');
		*pEndPar = 0;
		_fstrcpy (Prefix,str);
		VolLabel = _fstrchr (Prefix,0) + 1;
		_fstrcpy (VolLabel,pPar);
		Prefix = _fstrchr (VolLabel,0) + 1;
	}                                      
	if (num)
	{
		l = (long)VolLabel - (long)pBegin + 34;
		GlobalUnlock (hCDLookUpTable);
		hCDLookUpTable = GSSiGlobalReAlloc (0,hCDLookUpTable,l,GMEM_MOVEABLE);
	} 
	else
	{
		GSSiGlobUlFree (&hCDLookUpTable);
		hCDLookUpTable = (HANDLE)1;
	}
	return;
} 

int testvalue (int i)
{
	if (hCDLookUpTable == (HANDLE)3)
		return TRUE;
	else
		return FALSE;
}                          

void CloseCDLookUpTable (void)
{
	if (hCDLookUpTable > (HANDLE)1)
		GSSiGlobFree (&hCDLookUpTable);
	else
		hCDLookUpTable = 0;
	return;
}

BOOL ConvertNameToCDName (LPSTR Name)
{   
	char	Drive[34], NewName[MAX_PATH], Mess[MAX_PATH]; 
	short	n;
	BOOL	SaveDP = DoPaint (), SaveDH = DisableHalt;
	short	Response=1;
	LPSTR	VolLabel, FromName, pBS;
	
	if (!hCDLookUpTable)
		OpenCDLookUpTable ();
	if (hCDLookUpTable == (HANDLE)1)
		return FALSE;
	if (hCDLookUpTable == (HANDLE)3)
		return FALSE;
	ExpandText (Name);  
	setDoPaint( FALSE); 
	DisableHalt = TRUE;
	FromName = VolLabel = GlobalLock (hCDLookUpTable);
	while (*FromName)
	{
		n = _fstrlen (FromName);
		VolLabel += n+1;
		if (!_fstrnicmp (Name,FromName,n))
		{   
			char	savechr = Name[n];
			
			Name[n] = 0;
			pBS = _fstrrchr (Name,'\\');
			Name[n] = savechr;
			while (Response > 0)
			{
				if (GetCDDriveForVol (VolLabel,Drive)) 
				{  
					if (!_fstrchr (VolLabel,':'))
						_fstrcpy (CurrentCD,VolLabel);
					_fstrcpy (NewName,Drive);
					_fstrcat (NewName,pBS);
					_fstrcpy (Name,NewName);
					GlobalUnlock (hCDLookUpTable);
					setDoPaint( SaveDP);
					DisableHalt = SaveDH;
					return TRUE; 
				}
				if (*CurrentCD || Pick || CDInCancelledList (VolLabel))
					Response = 0;
				else 
				{
					Response = GetCDDriveForVolWait (VolLabel,Drive);
					if (!Response)
						AddCDToCancelledList (VolLabel);
				}
/*				{
					sprintf (Mess,"Please load the CD named %s",VolLabel);
					Response = GSSiMessageBox (0,Mess,"",MB_OKCANCEL);  
					if (Response == IDCANCEL)
						AddCDToCancelledList (VolLabel);
				} */
			} 
			goto Exit;
		}
		FromName = _fstrchr (VolLabel,0) + 1;
		VolLabel = FromName;
	}
Exit: 
	GlobalUnlock (hCDLookUpTable);
	setDoPaint( SaveDP);
	DisableHalt = SaveDH;
	return FALSE;
}

BOOL CDIsInstalled (LPSTR CDName)
{   
	short	n;
	LPSTR	VolLabel, FromName, pBS;    
	BOOL	rtn = FALSE;
	
	if (!hCDLookUpTable)
		OpenCDLookUpTable ();
	if (hCDLookUpTable == (HANDLE)1)
		return FALSE;
	FromName = VolLabel = GlobalLock (hCDLookUpTable);
	while (*FromName)
	{
		n = _fstrlen (FromName);
		VolLabel += n+1;
		if ((pBS = _fstrrchr (FromName,'\\')))
			pBS++;
		else
			pBS = FromName;
		if (!_fstricmp (CDName,pBS))
		{   
			if (_fstricmp (CDName,VolLabel))
				rtn = TRUE;
			GlobalUnlock (hCDLookUpTable);
			return rtn; 
		}
		FromName = _fstrchr (VolLabel,0) + 1;
		VolLabel = FromName;
	}
	GlobalUnlock (hCDLookUpTable);
	return rtn;
}

void AddCDToCancelledList (LPSTR CD)
{   
	LPSTR	pList;
	
	if (!hCancelledCDList)
		hCancelledCDList = GSSiGlobAlloc(GAIDNO 1280,GHND,1024);
	pList = GlobalLock (hCancelledCDList);
	while (*pList)
	{
		pList = _fstrchr (pList,0);
		pList++;
	} 
	_fstrcpy (pList,CD);
	GlobalUnlock (hCancelledCDList);
	return;
}

BOOL CDInCancelledList (LPSTR CD)
{   
	LPSTR	pList;
	
	if (!hCancelledCDList)
		return FALSE; 
	pList = GlobalLock (hCancelledCDList);
	while (*pList)
	{
		if (!_fstricmp (CD,pList)) 
		{
			GlobalUnlock (hCancelledCDList); 
			return TRUE;
		}
		pList = _fstrchr (pList,0);
		pList++;
	}
	GlobalUnlock (hCancelledCDList);
	return FALSE;
} 

void ClearCurrentCD (void)
{
	GSSiGlobFree (&hCancelledCDList);
	*CurrentCD = 0;
	return;
}


BOOL InstallCD (LPSTR FromPath,LPSTR ToPath,LPSTR Exclude,HWND hWndDlg,UINT MsgCntl,UINT StatusCntl)
{ 
	HCURSOR	hcurSave;  
	char	TempName[144], str2[150], file[144], str[144], VolLabel[34], Prefix[128];
	HFILE	OutFileFID, Fid;  
	long	TotFiles=0, TotLen=0, CurLoc=0;   
	OFSTRUCTGM	OFStruct;
	LPSTR	lpIdx, pRes, pBS;
	short	l=_fstrlen(FromPath), dnum; 
	char	SearchString[32]="*.*";
	double FreeSpace=GetDriveFreeSpace (ToPath), mb = FreeSpace/((double)1024*(double)1024);
    
    _fstrlwr (FromPath);
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
	SetDlgItemText (hWndDlg,MsgCntl,"Preparing to copy files");
	GSSiGetTempFileName(0,"gm",0,TempName);
	Fid = GSSiOpenFile (TempName,(LPOFSTRUCTGM)&OFStruct,OF_CREATE);
	SearchFilesInDir (FromPath, "", Fid,&TotFiles,SearchString,1,TRUE,FALSE); 
	GSSillseek (Fid,0,0);
	while (fgetstring (str2,144,Fid))
	{   
		LPSTR pTAB = strchr (str2,'\t');

		if (pTAB)
			*pTAB = 0;
		_fstrlwr (str2);
		if (!_fstrstr (str2,Exclude))
			TotLen += GSSiLength (str2);
	}
	if (TotLen > FreeSpace)
	{
		sprintf (str,"Insufficient space on drive: %.1fMB available - %.1fMB required\r\nContinue anyway?",
				 mb,(double)TotLen/((double)1024*(double)1024));
		if (MessageBox (hWndDlg,str,"Disk Space Warning",MB_ICONEXCLAMATION|MB_YESNO) == IDNO)
			return FALSE;
	} 
	GSSillseek (Fid,0,0);  
	if (mkdir (ToPath))
	{   
		sprintf (str,"Unable to create dirctory %s",ToPath);
		MessageBox (hWndDlg,str,NULL,MB_ICONEXCLAMATION);
		return FALSE;
	}
	while (fgetstring (str2,144,Fid))
	{   
		_fstrlwr (str2);
		if (!_fstrstr (str2,Exclude))
		{   
			char	FromFile[144];
			
			_fstrcpy (FromFile,str2); 
			sprintf (str,"Copying %s",FromFile); 
			SetDlgItemText (hWndDlg,MsgCntl,str);
			REPLAC (str2, FromPath,"", 144);
			sprintf (file,"%s%s",ToPath,str2);
			makedirectories (file,FALSE,FALSE);
			copyfile (file,FromFile,FALSE,0,0,hWndDlg,StatusCntl,TotLen,&CurLoc);
	        PctBox (GetDlgItem(hWndDlg,StatusCntl),TotLen,CurLoc,0);
	  	}
	}  
	GSSiClose2 (&Fid);
	GSSiRemove (TempName);
	GSSiSetCursor(hcurSave); 
	return TRUE;
} 

  

short SaveCurrentCD (HWND hWndDlg)
{   
	HFILE		FidTemp, FidOrder;
	OFSTRUCTGM	OFStruct;
	char		str[260], str2[270];
	int			Choice, CDNum, i;
	
	GetDlgItemText (hWndDlg,IDC_CDID,str,255);
	CDNum = atoi (LastChr (str));
	FidTemp = GSSiOpenFile ("[%DL]orders\\temp",&OFStruct,OF_CREATE);
	FidOrder = GSSiOpenFile ("[%DL]orders\\current.txt",&OFStruct,OF_READWRITE);
	if (FidOrder != HFILE_ERROR)
	{
		while (fgetstring (str,255,FidOrder))
		{
			i = atoi (str);
			if (i != CDNum)
				fputstring (str,FidTemp);
		} 
		GSSiClose2 (&FidOrder);
	} 
	Choice = 0;
	while (SendDlgItemMessage(hWndDlg,IDC_CDCONTENTS,LB_GETTEXT,Choice++,(DWORD)str) != LB_ERR)
	{   
		sprintf (str2,"%i\t%s",CDNum,str);
		fputstring (str2,FidTemp);
	}
	GSSiClose2 (&FidTemp);
	GSSiRemove ("[%DL]orders\\current.txt");
	GSSiRename ("[%DL]orders\\temp","[%DL]orders\\current.txt");
	return CDNum;
}

long GetCurrentCD (HWND hWndDlg,short NumCDs,LPLONG pOrderFreeSpace)
{   
	short	CDNum;
	HFILE		FidTemp, FidOrder;
	OFSTRUCTGM	OFStruct;
	char		str[260], str2[270];
	short		Choice, i; 
	LPSTR		pBeg, pLastTab;
	long		CDUsed=0, OrderUsed=0, size;
	
    SendDlgItemMessage (hWndDlg,IDC_CDCONTENTS,LB_RESETCONTENT,0,0);
	GetDlgItemText (hWndDlg,IDC_CDID,str,255);
	CDNum = atoi (LastChr (str));
	FidOrder = GSSiOpenFile ("[%DL]orders\\current.txt",&OFStruct,OF_READWRITE);
	if (FidOrder != HFILE_ERROR)
	{
		while (fgetstring (str,255,FidOrder))
		{
		    pLastTab = _fstrrchr (str,'\t');
		    *pLastTab =0;
		    pBeg = _fstrrchr (str,'\t');
		    pBeg++;
		    size = atol (pBeg);
		    *pLastTab = '\t'; 
		    OrderUsed += size;
			i = atoi (str);
			if (i == CDNum)
			{   
				pBeg = _fstrchr (str,'\t');
				pBeg++;
			    SendDlgItemMessage (hWndDlg,IDC_CDCONTENTS,LB_ADDSTRING,0,(LPARAM)pBeg); 
			    CDUsed += size;
			}
		} 
		GSSiClose2 (&FidOrder);
	} 
	*pOrderFreeSpace = max (0,NumCDs * CDSize - OrderUsed);
	sprintf (str,"%ld megabytes free space remain on this CD",CDSize - CDUsed);
	SetDlgItemText (hWndDlg,IDC_MESS,str);
	PctBox (GetDlgItem(hWndDlg,IDC_STATUS),CDSize,CDUsed,0);   
	return CDSize - CDUsed;
} 

BOOL GetCDDriveForVol (LPSTR VolID,LPSTR Drive)
#if ENABLETRACE
{GSSiEnterProg (416);
#endif
{
	short	i; 
	char	VolLabel[32];
	UINT	PrevErrMode;
	
	if (_fstrchr (VolID,':'))
	{
		_fstrcpy (Drive,VolID);
{
#if ENABLETRACE
GSSiExitProg (416);
#endif
		return TRUE;
}
	}
	PrevErrMode = SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);

	for (i=3;i<26;i++)
	{   
//		if (GetDriveType (i) == DRIVE_REMOTE)
		{                 
			if (GetVolumeLabel(Drive,VolLabel))
			{   
				Truncate (VolID);
                if (!_fstricmp(VolLabel,VolID))
                {
                	sprintf (Drive,"%c:",(char)('A'+i)); 
					SetErrorMode(PrevErrMode);
{
#if ENABLETRACE
GSSiExitProg (416);
#endif
                	return TRUE;  
}
                }
            } 
        }
	} 
	SetErrorMode(PrevErrMode);
{
#if ENABLETRACE
GSSiExitProg (416);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 


