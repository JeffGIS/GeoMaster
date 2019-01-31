#include "graphint.h"
#include "extrndb.h" 

#include "shapefil.h"
#include "gmextern.h"
#include <sqlext.h>               

static	long	debugoff=5368;
static  short CurrentIndex=0; 
static	BOOL	HaveLZOInit=FALSE; 
static	GWDHEADER16	GWDHead16;
static	int updateGMDlenMain  = 0;
static	int	updateGMDlenIndex = 0;

extern char	CacheTitle[256];

BOOL GMDKeyListAdd(LPSTR key);
BOOL GMDCreateKeyList(LPSTR FileName);


int showmessage(int line, char* file,int message)
{

#ifdef CHECKMEM
	char msg[1024];
	static int n = 0;
	sprintf(msg, "Message %d: %s:%d %d\n" , n++, file, line,message);
	OutputDebugStringA(msg);
	return 1;
#else
	return 0;
#endif
}


HANDLE GetFilesToClose (HANDLE hSQL)
{
	HANDLE hList=0;
	LPOPENSQLDATA	SQLPtr;
	LPOPENFILEDATA	FilePtr;   
	short	i;

	SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
	if (FilePtr->Type == UMIFS_DATAFILE)
	{
 		LPGWDHEADER	lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 
		GlobalUnlock (FilePtr->FileHandle); 
	}
	GlobalUnlock (SQLPtr->OFHandle);
	GlobalUnlock (hSQL);
	return hList;
}

BOOL DeleteFilesInList (HANDLE hList)
{
	LPSTR pList;
	
	if (hList)
	{
		pList = GlobalLock (hList);
		GlobalUnlock (hList);
	}
	return TRUE;
}

BOOL NeedSQLValueQuote (HANDLE hSQL,LPSTR FieldName)
#if ENABLETRACE
{GSSiEnterProg (610);
#endif
{   
	BOOL	rtn=FALSE;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	LPFIELDINFO	lpFieldInfo;   
	short	i;

	SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
	lpFieldInfo = &FilePtr->FldInfo; 
	for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++) 
	{
		if (!_fstricmp (lpFieldInfo->name,FieldName))
		{
            switch (lpFieldInfo->type)
            {
            	case SQL_CHAR:
            	case SQL_VARCHAR:
            	case SQL_TIMESTAMP:
				case SQL_UNKCHAR:
					rtn=TRUE; 
				default:
				break; 
			}
		}
	}	
	GlobalUnlock (SQLPtr->OFHandle); 
	GlobalUnlock (hSQL);
{
#if ENABLETRACE
GSSiExitProg (610);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL GWDAddField (LPSTR DBName,LPGWFLDINFO pNewFldInfo)
{
	HANDLE	hDB; 
	LPGWDHEADER	lpGWDHead; 
	LPGWFLDINFO lpFieldInfo;
	long	FirstRecLoc, CurLoc,StartLoc, len, nextlen,Size, dellen,ii;      
	short	i; 
	BOOL	HaveDeletedSegment;
	LPSTR	pEQ;
    
    if ((pEQ = _fstrchr (DBName,'=')))
    	DBName = ++pEQ;
	hDB = OpenGWDatabase (DBName,BT_WRITE);
	if (!hDB)
		return (FALSE);
	Size = GlobalSize (hDB);
	Size += pNewFldInfo->Len;
	hDB = GSSiGlobalReAlloc (0,hDB,Size,GMEM_MOVEABLE);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);        
    if (!lpGWDHead->Compressed)
    { 
ErrOut:
		GlobalUnlock (hDB);
		CloseGWDatabase (hDB);
        return FALSE;
    }
    for (i=0,lpFieldInfo=lpGWDHead->pFldInfo;i<lpGWDHead->NumFields;i++,lpFieldInfo++)
	{
		if (!stricmp (lpFieldInfo->Name,pNewFldInfo->Name))
			goto ErrOut;
	}
	CurLoc = FirstRecLoc =  GSSillseek (lpGWDHead->Fid,0,1);
Next:
    if (BigRead (lpGWDHead->Fid,(HPSTR)&len,4) != 4)
		goto Close; 
	if (len == (-((long)sizeof(GWFLDINFO)-4)))
		goto Close;
	StartLoc = CurLoc;
	if (len < 0)
	{   
		HaveDeletedSegment = TRUE;
		len = -len;
NextDelete:
		if (len > sizeof(GWFLDINFO))
		{
			ii=GSSillseek (lpGWDHead->Fid,FirstRecLoc,0); 
			ii=GSSillseek (lpGWDHead->Fid,sizeof(GWFLDINFO),1); 
			len -= sizeof(GWFLDINFO);
			len = -len;
	        BigWrite (lpGWDHead->Fid,(HPSTR)&len ,4,-1);
	        goto Close;
	    }
	    CurLoc = StartLoc + len + 4;
		GSSillseek (lpGWDHead->Fid,CurLoc,0);
		if (BigRead (lpGWDHead->Fid,(HPSTR)&nextlen,4) != 4)
			goto Close; 
		if (nextlen < 0)
		{
			nextlen = -nextlen;
			len += nextlen;
			goto NextDelete;
		}
	}
	else
		HaveDeletedSegment = FALSE;
    FillGWDData (lpGWDHead,CurLoc);  
	GWDDeleteRecord (lpGWDHead,CurLoc);  
    GWDAddRecord (lpGWDHead,0,NULL);  
    if (HaveDeletedSegment)
    {
	    GSSillseek (lpGWDHead->Fid,CurLoc,0); 
		BigRead (lpGWDHead->Fid,(HPSTR)&dellen,4);
	    len += -dellen + 4;  
	    GSSillseek (lpGWDHead->Fid,FirstRecLoc,0); 
	    len = -len;
		BigWrite (lpGWDHead->Fid,(HPSTR)&len,4,-1);   
	}
    CurLoc = GSSillseek (lpGWDHead->Fid,FirstRecLoc,0);
    goto Next; 
Close:
    GSSillseek (lpGWDHead->Fid,0,0);
	lpGWDHead->NumFields++; 
	pNewFldInfo->Beg = lpGWDHead->Reclen; 
	pNewFldInfo->HasValue = pNewFldInfo->ValueID = pNewFldInfo->fill = 0;
	lpGWDHead->Reclen += pNewFldInfo->Len;
	if (lpGWDHead->StoredAs32)
	    BigWrite (lpGWDHead->Fid,(HPSTR)lpGWDHead ,sizeof(GWDHEADER),-1);
	else
	{
		GWDHead16 = GWDHEADER32toGWDHEADER16 (lpGWDHead);
		BigWrite (lpGWDHead->Fid,(HPSTR)&GWDHead16 ,sizeof(GWDHEADER16),-1);
	}
//	GWDHead16 = GWDHEADER32toGWDHEADER16 (lpGWDHead);
//    BigWrite (lpGWDHead->Fid,(HPSTR)&GWDHead16 ,sizeof(GWDHEADER16),-1);
    for (i=0,lpFieldInfo=lpGWDHead->pFldInfo;i<lpGWDHead->NumFields-1;i++,lpFieldInfo++)
		BigWrite (lpGWDHead->Fid,(HPSTR)lpFieldInfo,sizeof(GWFLDINFO),-1);   
	BigWrite (lpGWDHead->Fid,(HPSTR)pNewFldInfo,sizeof(GWFLDINFO),-1);   
	CurLoc = GSSillseek (lpGWDHead->Fid,0,1);
    GSSiClose (lpGWDHead->Fid);
    for (i=0;i<lpGWDHead->NumIndex;i++)
    {   
        if (lpGWDHead->BTHandle[i])
        {
            BT_SET_TIME_STAMP (lpGWDHead->BTHandle[i], lpGWDHead->TimeStamp);
            BT_CLOSE (lpGWDHead->BTHandle[i]);
            GSSiGlobUlFree (&lpGWDHead->hKeys[i]);
        }
    }
    GSSiGlobUlFree (&lpGWDHead->hFldInfo);
    GSSiGlobUlFree (&hDB);
	return TRUE;
}

long GetCompressedReclen(LPGWDHEADER lpGWDHead, long Offset)
{
	unsigned short	i;
	short	ShortLen;
	int		ii, jj;
	long	len;

	if (Offset < 0)
		return -1;
	if (lpGWDHead->SplitFile)
	{
		if (Offset > lpGWDHead->SplitLength)
		{
			Offset = Offset - lpGWDHead->SplitLength;
			if (GSSillseek(lpGWDHead->Fid, Offset, 0) == HFILE_ERROR)
				return -1;
		}
		else
		{
			Offset = Offset - ((sizeof(GWDHEADER)) + lpGWDHead->NumFields*sizeof(GWFLDINFO));
			if (GSSillseek(lpGWDHead->SplitFid, Offset, 0) == HFILE_ERROR)
				return -1;
		}
	}
	else if (GSSillseek(lpGWDHead->Fid, Offset, 0) == HFILE_ERROR)
		return -1;
	if (lpGWDHead->Compressed)
	{
		if (BigRead(lpGWDHead->Fid, (HPSTR)&len, 4) != 4)
			return -1;
		else
			return abs(len);
	}
	else
	{
		BigRead(lpGWDHead->Fid, (HPSTR)&ShortLen, 2);
		return abs(ShortLen);
	}
}

long FillGWDData (LPGWDHEADER lpGWDHead,long Offset)
#if ENABLETRACE
{GSSiEnterProg (611);
#endif
{   unsigned short	i;
	short	ShortLen; 
	int		ii,jj;
	HANDLE	hCompressedRec;
	HPSTR	CompressedRec;   
	long	len=-1,il;
	HFILE	fid = lpGWDHead->Fid;
	static	db=FALSE;

	if (Offset < 0)
		goto Exit;
	LastGMDRecordLength = 0;    
    if (Offset == GetGWDCurrentOffset (lpGWDHead))
	{
    	len = lpGWDHead->Reclen;
		goto Exit;
	}
    SetGWDCurrentOffset (lpGWDHead,Offset);
	if (lpGWDHead->SplitFile)
	{
		if (Offset >= lpGWDHead->SplitLength)
		{
			Offset = Offset - lpGWDHead->SplitLength +(sizeof(GWDHEADER)) + lpGWDHead->NumFields*sizeof(GWFLDINFO);
			if (GSSillseek(lpGWDHead->Fid, Offset, 0) == HFILE_ERROR)
				goto Exit;
		}
		else
		{
			Offset = Offset - ((sizeof(GWDHEADER)) + lpGWDHead->NumFields*sizeof(GWFLDINFO));
			fid = lpGWDHead->SplitFid;
			if (GSSillseek(fid, Offset, 0) == HFILE_ERROR)
				goto Exit;
		}
	}
	else if (GSSillseek (lpGWDHead->Fid,Offset,0) == HFILE_ERROR)
    	goto Exit;
    if (lpGWDHead->Compressed)
    {
	    if (BigRead (fid,(HPSTR)&len,4) != 4)
		{
			len = -1;
	    	goto Exit;
		}
	    if (len < 0)
	    {
	    	GSSillseek (fid,-len,1);
	    	len = -2;//deleted record  
			goto Exit;
	    }
		if (len > lpGWDHead->Reclen*4)
		{
			len = -1;
			goto Exit;
		}
	    LastGMDRecordLength = len;
	    if (len)
	    {
			HANDLE hDeCompressedRec = GSSiGlobAlloc (1516,GMEM_MOVEABLE,lpGWDHead->Reclen+4096);   
	    	LPSTR  pDeCompressedRec = GlobalLock (hDeCompressedRec);  

	    	hCompressedRec = GSSiGlobAlloc (1516,GMEM_MOVEABLE,len);   
	    	CompressedRec = GlobalLock (hCompressedRec);  
			jj=GSSillseek (fid,0,1);
	    	ii=BigRead (fid,CompressedRec,len);
    		len = DecompressBinaryRecord (pDeCompressedRec,lpGWDHead->Reclen,CompressedRec,len);
			memmove ((HPSTR)&lpGWDHead->GWDData,pDeCompressedRec,min (lpGWDHead->Reclen,len));
			if (lpGWDHead->Reclen < len)
				ii=1;
    		GSSiGlobUlFree (&hCompressedRec);
    		GSSiGlobUlFree (&hDeCompressedRec);
    	}
    	for (il=len;il<lpGWDHead->Reclen;il++)
    		lpGWDHead->GWDData[il]='\0'; 
    }
    else
    {
	    BigRead (fid,(HPSTR)&ShortLen,2); 
	    if (ShortLen < 0)  
	    {
	    	GSSillseek (fid,-ShortLen,1);
	    	len = -2;//deleted record
			goto Exit;
	    }
	    ShortLen = min (ShortLen,lpGWDHead->Reclen);
	    if (ShortLen)
	    	BigRead (fid,lpGWDHead->GWDData,ShortLen);//&lpGWDHead->GWDData[150]
    	for (i=ShortLen;i<lpGWDHead->Reclen;i++)
    		lpGWDHead->GWDData[i]='\0'; 
    	LastGMDRecordLength = len = ShortLen;
    }
Exit:
{
#if ENABLETRACE
GSSiExitProg (611);
#endif
	return len;
}
#if ENABLETRACE
}
#endif
}

BOOL GetGMDUniqueFieldValues (HANDLE hDB, LPCSTR SQL,int fieldindex, HANDLE hDBList)
#if ENABLETRACE
{GSSiEnterProg (612);
#endif
{                             
    int   i, ClassNo, ClassZero=0; 
    LPGWDHEADER lpGWDHead;
    HANDLE      hBT;
    long        Offset;
    char        str[256],Value[256];
    short       len, pos;
    HCURSOR hcurSave;
    
    hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
    hBT = lpGWDHead->BTHandle[0];
    pos = BT_FIRST;    
    while (!BT_FIND (hBT,lpGWDHead->pKeys[0],pos,BT_ANY, (LPSTR)&Offset))
    {    
        pos = BT_NEXT; 
        FillGWDData (lpGWDHead,Offset);
        if (GMDGetCharFieldVal (lpGWDHead,fieldindex,str))
        {
            _fstrncpy (Value,str,(size_t)GetBTKeyLen(hDBList));
            if (BT_FIND(hDBList,Value,BT_FIRST,BT_EQ,(LPSTR)&ClassNo))
                BT_PUT (hDBList,Value,(LPSTR)&ClassZero); 
        }
    }
    GlobalUnlock (hDB);
    GSSiSetCursor (hcurSave); 

{
#if ENABLETRACE
GSSiExitProg (612);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
} 
                                       
 


BOOL CloseComboDatabase (HANDLE hDB)
#if ENABLETRACE
{GSSiEnterProg (615);
#endif
{   
	LPCOMBOHEADER		pComboHeader;
    LPCOMBOFILE  		pComboFile; 
    LPCOMBOFIELDINFO    pComboField;
    LPWHEREINDEX        pWhereIndex;
    LPCFIELDINDEX       pCFieldIndex;  
    LPSTR       pWhere, pCField;
    OFSTRUCTGM    OFStruct;
    HFILE       FidCF; 
    short		i, len;  
    
	if (!hDB)
{
#if ENABLETRACE
GSSiExitProg (615);
#endif
		return FALSE;
}
	pComboHeader = (LPCOMBOHEADER)GlobalLock (hDB); 
	pComboFile = (LPCOMBOFILE)GlobalLock (pComboHeader->hComboFile);
	for (i=0;i<pComboFile->NumFiles;i++)
		CloseDataFile (TRUE,&pComboFile->hSQL[i]);
	GSSiGlobFree (&pComboHeader->hComboFields);
	GSSiGlobFree (&pComboHeader->hWhere);
	GSSiGlobFree (&pComboHeader->hComputedFields);
	GSSiGlobUlFree (&pComboHeader->hComboFile);
	GSSiGlobUlFree (&hDB);
{
#if ENABLETRACE
GSSiExitProg (615);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL CreateComboFile (LPSTR Name,LPSTR FieldDefs,LPSTR RefFile1,LPSTR RefFile2,LPSTR RefFile3,LPSTR RefFile4,LPSTR RefFile5,LPSTR RefFile6,LPSTR RefFile7,LPSTR RefFile8,LPSTR RefFile9)
{
	BOOL rtn=FALSE;
	HFILE	Fid;
	int		i=0;
	char	str[1024+256],Definition[1024],FieldName[66];
	LPSTR	pLoc,pEnd,pPar;
	LPSTR	RefFile[9]={RefFile1,RefFile2,RefFile3,RefFile4,RefFile5,RefFile6,RefFile7,RefFile8,RefFile9};

	Fid = GSSiOpenFile (Name,0,OF_CREATE);   
	if (Fid == HFILE_ERROR)
		return FALSE;
	fputstring ("COMBO FILE DEFINITION",Fid);
	while (i<9 && *RefFile[i])
	{
		sprintf (str,"\tFILE%i=%s",i+1,RefFile[i++]);
		fputstring (str,Fid);
	}
	fputstring ("FIELD DEFINITIONS",Fid);

	if (*FieldDefs == '(')
	{
		FieldDefs++;
		*LastChr (FieldDefs) = 0;
	}
NextField:
	pEnd = strchr (FieldDefs,',');
	if (pEnd)
		*pEnd = 0;
	if (*FieldDefs == '"')
		FieldDefs++;
	if (*LastChr (FieldDefs) == '"')
		*LastChr (FieldDefs) = 0;
	strcpy (FieldName,FieldDefs);
	if ((pPar = strchr (FieldName,'(')))
		*pPar = 0;
	*Definition = 0;
	i = 0;
	while (i<9 && *RefFile[i])
	{
		HANDLE	hDB=0;

		if (OpenDataFile (RefFile[i++],"",BT_READ,&hDB))
		{
			LPOPENSQLDATA	SQLPtr = (LPOPENSQLDATA)GlobalLock(hDB);
			LPOPENFILEDATA	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
			LPFIELDINFO	lpFieldInfo;
			int				ifield;

			lpFieldInfo = &FilePtr->FldInfo; 
			for (ifield=0;ifield<FilePtr->NumFields;ifield++,lpFieldInfo++)
			{
				if (!_fstricmp (lpFieldInfo->name,FieldName))
				{
					sprintf (Definition,"[FILE%i.%s]",i,lpFieldInfo->name);
					break;
				}
			}
			GlobalUnlock (SQLPtr->OFHandle);
			GlobalUnlock (hDB);
			CloseDataFile (FALSE,&hDB);
		}
	}

	sprintf (str,"\t%s=%s",FieldDefs,Definition);
	fputstring (str,Fid);
	if (pEnd)
	{
		FieldDefs = pEnd + 1;
		goto NextField;
	}
	GSSiClose (Fid);
	return rtn;
}
                                       
BOOL FAR PASCAL COMBO_FILEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (616);
#endif
{   char str[300];
    static  HANDLE hDB=0;
    static  LPGWDHEADER lpGWDHead;
    int     TabStops[3]={80,160,170}, i,Choice, len;
    static  short DataFileType, UpdateIndex;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr; 
    static      LPCOMBOFILE     pComboFile; 
    LPFIELDINFO         pField,pFileField;
    LPCOMBOFIELDINFO    pComboField;
    LPWHEREINDEX        pWhereIndex;
    LPCFIELDINDEX       pCFieldIndex;  
    LPSTR       pWhere, pCField;
    static      HANDLE      hSQL,hComboFields, hWhere, hComputedFields, hComboFile; 
    OFSTRUCTGM    OFStruct;
    HFILE       FidCF;
    
 
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (616);
#endif
 	return (BRtn);
}
    
 switch(Message)
   {
    case WM_INITDIALOG:   
        SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_SETTABSTOPS,3,(LPARAM)&TabStops); 
        SendDlgItemMessage (hWndDlg,IDC_CFTYPE,CB_ADDSTRING,0,(LPARAM)"Integer");
        SendDlgItemMessage (hWndDlg,IDC_CFTYPE,CB_ADDSTRING,0,(LPARAM)"Real");
        SendDlgItemMessage (hWndDlg,IDC_CFTYPE,CB_ADDSTRING,0,(LPARAM)"Text");
        hComboFile = GSSiGlobAlloc ( 254,GHND,sizeof(COMBOFILE)+MAXFIELDS*sizeof(FIELDINFO));
        pComboFile = (LPCOMBOFILE)GlobalLock (hComboFile);
        hAddFile = GSSiGlobAlloc ( 255,GHND,128);
        pAddFile = GlobalLock (hAddFile);
        hWndComboFile = hWndDlg;    
        hComboFields = GSSiGlobAlloc ( 256,GHND,MAXFIELDS*sizeof(COMBOFIELDINFO));
        hWhere = GSSiGlobAlloc ( 257,GHND,USHRT_MAX); 
        hComputedFields = GSSiGlobAlloc ( 258,GHND,USHRT_MAX);
        if (hCFName)
        {     
        	LPSTR	pName;
        	
			pName = GlobalLock (hCFName);
			FidCF = GSSiOpenFile (pName,&OFStruct,OF_READ);
			GlobalUnlock (hCFName);
			ReadComboFile16 (FidCF,pComboFile);
			pComboField = (LPCOMBOFIELDINFO)GlobalLock (hComboFields); 
              
			for (i=0;i<pComboFile->NumFields;i++)
			{
				pComboFile->FldInfo[i] = ReadFieldInfo16 (FidCF);
			}
			BigRead (FidCF,(HPSTR)pComboField,pComboFile->NumFields*sizeof(COMBOFIELDINFO));
			GlobalUnlock (hComboFields);
			pWhereIndex = (LPWHEREINDEX) GlobalLock (hWhere); 
			BigRead (FidCF,(HPSTR)pWhereIndex,sizeof(WHEREINDEX));
			len = pWhereIndex->Len;
			pWhereIndex++;
			BigRead (FidCF,(HPSTR)pWhereIndex,len);
			GlobalUnlock (hWhere);
			pCFieldIndex = (LPCFIELDINDEX)GlobalLock(hComputedFields);
			BigRead (FidCF,(HPSTR)pCFieldIndex,sizeof(CFIELDINDEX));
			len = pCFieldIndex->Len;
			pCFieldIndex++;
			BigRead (FidCF,(HPSTR)pCFieldIndex,len);
			GlobalUnlock (hComputedFields);
			GSSiClose (FidCF);
			for (i=0;i<pComboFile->NumFiles;i++)
			{ 
				pComboFile->hSQL[i]=0; 
				OpenDataFile (pComboFile->FileNames[i],"",BT_READ,&pComboFile->hSQL[i]);
			}
         }
   Display:
        SendDlgItemMessage (hWndDlg,IDC_FILES,LB_RESETCONTENT,0,0);
        for (i=0;i<pComboFile->NumFiles;i++)
            SendDlgItemMessage (hWndDlg,IDC_FILES,LB_ADDSTRING,0,(LPARAM)((LPSTR) pComboFile->FileNames[i]));  
        SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_RESETCONTENT,0,0);
        pWhereIndex = (LPWHEREINDEX)GlobalLock (hWhere); 
        pField = pComboFile->FldInfo; 
        pComboField = (LPCOMBOFIELDINFO)GlobalLock (hComboFields);
        for (i=0;i<pComboFile->NumFields;i++,pField++,pComboField++)
        {   
            if (pComboField->fromfile == -1)
            {
                pCFieldIndex =(LPCFIELDINDEX) GlobalLock(hComputedFields);
                pCField = (LPSTR)pCFieldIndex + (sizeof(CFIELDINDEX) +
                        pCFieldIndex->offset[pComboField->fromfileindex]); 
                sprintf (str,"%s\tcomputed\tfrom\t%s",              
                        pField->name,pCField);
                GlobalUnlock (hComputedFields);
            }
            else
            {
                pWhere = (LPSTR)pWhereIndex + (sizeof(WHEREINDEX)+pWhereIndex->offset[pComboField->WhereID]);
                if (pComboFile->hSQL[pComboField->fromfile])
                {
	                SQLPtr = (LPOPENSQLDATA)GlobalLock(pComboFile->hSQL[pComboField->fromfile]);
	                FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	                if (pComboField->fromfileindex < FilePtr->NumFields)
	                {
		                pFileField = &FilePtr->FldInfo+pComboField->fromfileindex; 
		                if (pComboField->fromfile)
		                    sprintf (str,"%s\tfrom %s\tin %i\twhere %s",              
		                            pField->name,pFileField->name,pComboField->fromfile+1,pWhere);
		                else
		                    sprintf (str,"%s\tfrom %s\tin %i",
		                             pField->name,pFileField->name,pComboField->fromfile+1); 
		            }
	                GlobalUnlock (SQLPtr->OFHandle);
	                GlobalUnlock(pComboFile->hSQL[pComboField->fromfile]);   
	            }
	            else
	            	*str = 0;
            }
            SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_ADDSTRING,0,(LPARAM)((LPSTR) str));  
        } 
        GlobalUnlock (hWhere); 
        GlobalUnlock (hComboFields);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
              case 65001:
                    Choice=(short)SendDlgItemMessage(hWndDlg,IDC_FIELDS, LB_GETCURSEL,0,0);
                    pField = pComboFile->FldInfo; 
                    pField += Choice;
                    GetTextString (hWndDlg,pField->name,32,"Enter new field name",NULL,NULL,0,TRUE,TRUE);
                    goto Display;
                    break;   
                    
              case 65004:
                    Choice=(short)SendDlgItemMessage(hWndDlg,IDC_FIELDS, LB_GETCURSEL,0,0);
                    pField = pComboFile->FldInfo; 
                    pField += Choice;
					pComboField = (LPCOMBOFIELDINFO)GlobalLock (hComboFields);
					pComboField += Choice; 
					UpdateIndex = Choice;
		            if (pComboField->fromfile == -1)
		            {
		                pCFieldIndex =(LPCFIELDINDEX) GlobalLock(hComputedFields);
		                pCField = (LPSTR)pCFieldIndex + (sizeof(CFIELDINDEX) +
		                        pCFieldIndex->offset[pComboField->fromfileindex]);
						SetDlgItemText(hWndDlg,IDC_CFNAME,pField->name);
		                SetDlgItemText(hWndDlg,IDC_CFDEF,pCField); 
		                switch (pField->type)
		                {
		                	case BT_INTEGER:
		                		i=0;
		                		break;
		                	case BT_REAL:
		                		i=1;
		                		break;
		                	default:
		                		i=2; 
			                  	SetDlgItemInt (hWndDlg,IDC_CFLEN,(UINT)pField->length,TRUE); 
		                		ShowWindow (GetDlgItem(hWndDlg,IDC_CFLEN),TRUE);
				         		ShowWindow (GetDlgItem(hWndDlg,IDC_CFLEN_TITLE),TRUE);
 		                }
       					SendDlgItemMessage(hWndDlg,IDC_CFTYPE, CB_SETCURSEL,i,0);                 
		                GlobalUnlock (hComputedFields);
		                SetDlgItemText (hWndDlg,IDC_ADD_FIELD,"Update");
		            }
		            
					GlobalUnlock (hComboFields);
                    goto Display;
                    break;   
                    
              case 65002:
              case 65003:
                    break;
              
              case IDOK: 
              {
                 short    SaveDrive, Choice;  
                 BOOL   GotFile; 
                 char	Name[256];   
                 LPSTR  pName;
                 
                  if (hCFName)
                  	pName = GlobalLock (hCFName);
                  else
                  {
                  	pName = Name;
                  	if (!GetSaveName2 (hWndDlg,pName,IDS_FILTERGCF,".GCF",IDS_FILEGCF)) break;   
                  }
                  pComboFile->TotFileLen=0;
                  Choice = 0;
                  while ((len=(short)SendDlgItemMessage(hWndDlg ,IDC_FILES,LB_GETTEXT,Choice++,(LPARAM)str))>=0)
                  {
                    pComboFile->TotFileLen += (len + 1);
                  }
                  FidCF = GSSiOpenFile (pName,&OFStruct,OF_CREATE); 
                  GSSiGlobUlFree (&hCFName);

				  WriteComboFile16 (FidCF,pComboFile);
				  for (i=0;i<pComboFile->NumFields;i++)
				  {
					  WriteFieldInfo16 (FidCF,&pComboFile->FldInfo[i]);
				  }

                  Choice = 0;
                  pComboField = (LPCOMBOFIELDINFO)GlobalLock (hComboFields); 
                  BigWrite (FidCF,(HPSTR)pComboField,pComboFile->NumFields*sizeof(COMBOFIELDINFO),-1);
                  GSSiGlobUlFree (&hComboFields); 
                  pWhereIndex = (LPWHEREINDEX)GlobalLock (hWhere); 
                  BigWrite (FidCF,(HPSTR)pWhereIndex,sizeof(WHEREINDEX) + pWhereIndex->Len,-1);
                  GSSiGlobUlFree (&hWhere);
                   
                  pCFieldIndex = (LPCFIELDINDEX)GlobalLock(hComputedFields);
                  BigWrite (FidCF,(HPSTR)pCFieldIndex,sizeof(CFIELDINDEX) + pCFieldIndex->Len,-1);
                  GSSiGlobUlFree (&hComputedFields);
                  GSSiClose (FidCF);
                  GSSiGlobUlFree (&hAddFile);
                  for (i=0;i<pComboFile->NumFiles;i++)
                    CloseDataFile (TRUE, &pComboFile->hSQL[i]);
                  GSSiGlobUlFree (&hComboFile);
                  EndDialog(hWndDlg, TRUE);
              }
                  break;
                  
              case IDC_HAVE_FILE: 
              {
                  int   nItems, ComboFileID;
                  LPINT lpItems;
                  HANDLE hItems;  
                  
                  ComboFileID=(int)SendDlgItemMessage(hWndDlg,IDC_FILES,LB_GETCURSEL,0,0);
                  if (ComboFileID<0) 
                  {
                    SendDlgItemMessage (hWndDlg,IDC_FILES,LB_ADDSTRING,0,(LPARAM)((LPSTR) pAddFile)); 
                    ComboFileID = pComboFile->NumFiles++;
                    _fstrcpy (pComboFile->FileNames[ComboFileID],pAddFile);
					OpenDataFile (pAddFile,"",BT_READ,&pComboFile->hSQL[ComboFileID]);
                  }
                  nItems=(short)SendDlgItemMessage(hWndAddComboFile ,SV_FIELD_NAME,LB_GETSELCOUNT,0,0);
                  if (nItems + pComboFile->NumFields > MAXFIELDS)
                  {
                  	GSSiMsgBox (hWndAddComboFile,"Too many fields",NULL,MB_ICONEXCLAMATION,0);
                  }
                  else
                  { 
	                  hItems=GSSiGlobAlloc ( 259,GHND,nItems*4);
	                  lpItems=  (LPINT) GlobalLock(hItems);
	                  SendDlgItemMessage(hWndAddComboFile ,SV_FIELD_NAME,LB_GETSELITEMS,nItems,(LPARAM)lpItems);
	                  pField = pComboFile->FldInfo;
	                  pField += pComboFile->NumFields;
	                  pComboField = (LPCOMBOFIELDINFO)GlobalLock (hComboFields);
	                  pComboField += pComboFile->NumFields;
	                  pWhereIndex = (LPWHEREINDEX)GlobalLock (hWhere); 
	                  while (nItems--)
	                  { 
	                    SendDlgItemMessage(hWndAddComboFile ,SV_FIELD_NAME,LB_GETTEXT,*lpItems,(LPARAM)((LPSTR)str)); 
	                    _fstrcpy (pField->name,str);
	                    pField->index = pComboFile->NumFields;
	                    pComboField->fromfile = ComboFileID;
	                    pComboField->fromfileindex = *lpItems++;
	                    pComboField->WhereID = pWhereIndex->Num; 
	                    pField++;   
	                    pComboField++;
	                    pComboFile->NumFields++;
	                  }
	                  GlobalUnlock (hComboFields); 
	                  GSSiGlobUlFree (&hItems);
	                  pWhere = (LPSTR)pWhereIndex + (sizeof(WHEREINDEX) + pWhereIndex->Len); 
	                  pWhereIndex->offset[pWhereIndex->Num] = pWhereIndex->Len;
	                  GetDlgItemText(hWndAddComboFile,IDC_SQL,pWhere,256);
	                  pWhereIndex->Len += _fstrlen(pWhere) + 1;  
	                  pWhereIndex->Num++;
	                  GlobalUnlock (hWhere);
	              }
                  DestroyWindow(hWndAddComboFile); 
                  goto Display;
              }
                  break;
              
              case IDCANCEL: 
                  for (i=0;i<pComboFile->NumFiles;i++)
                    CloseDataFile (TRUE, &pComboFile->hSQL[i]);
                  GSSiGlobUlFree (&hAddFile);
                  GSSiGlobUlFree (&hComboFile);
                  GSSiGlobFree (&hWhere);
                  GSSiGlobFree (&hComboFields);
                  GSSiGlobFree (&hCFName);
                  GSSiGlobFree (&hComputedFields);
                  EndDialog(hWndDlg, FALSE);
                  break; 
                  
              case IDC_ADD_FIELD:
                  pField = pComboFile->FldInfo;
                  GetDlgItemText (hWndDlg,IDC_ADD_FIELD,str,256);
                  if (*str == 'A') 
                  	UpdateIndex = pComboFile->NumFields;
                  pField += UpdateIndex;
                  SetDlgItemText (hWndDlg,IDC_ADD_FIELD,"Add");
                  pComboField = (LPCOMBOFIELDINFO)GlobalLock (hComboFields);
                  pComboField += UpdateIndex;
                  pCFieldIndex = (LPCFIELDINDEX)GlobalLock(hComputedFields);
                  if (!GetDlgItemText(hWndDlg,IDC_CFNAME,pField->name,32))
                  { 
                     GSSiMsgBox( GetFocus(),"No field name",NULL,MB_ICONEXCLAMATION,0); 
                  	 break;
                  }
                  if (!GetDlgItemText(hWndDlg,IDC_CFTYPE,str,32))
                  { 
                     GSSiMsgBox( GetFocus(),"Type not specified",NULL,MB_ICONEXCLAMATION,0); 
                  	 break;
                  }
                  if (!_fstrcmp (str,"Text")) 
                  {  
                  	 BOOL Error;
                  	 
                  	 pField->type = BT_CHAR;
                  	 pField->length = GetDlgItemInt (hWndDlg,IDC_CFLEN,&Error,FALSE); 
                  	 if (!Error || !pField->length)
	                  { 
	                     GSSiMsgBox( GetFocus(),"Length not specified",NULL,MB_ICONEXCLAMATION,0); 
	                  	 break;
	                  }
                  }
                  else if (!_fstrcmp (str,"Real")) 
                  {
                  	 pField->type = BT_REAL;
                  	 pField->length = 8;
                  }
                  else
                  {
                  	 pField->type = BT_INTEGER;
                  	 pField->length = 4;
                  }	 
                  if (UpdateIndex >= pComboFile->NumFields)
                  	pField->index = pComboFile->NumFields++;
                  pComboField->fromfile = -1;
                  pComboField->fromfileindex = pCFieldIndex->Num;
                  pComboField->WhereID = 0; 
                  GlobalUnlock (hComboFields); 
                  pCField = (LPSTR)pCFieldIndex + (sizeof(CFIELDINDEX) + pCFieldIndex->Len); 
                  pCFieldIndex->offset[pCFieldIndex->Num] = pCFieldIndex->Len;
                  GetDlgItemText(hWndDlg,IDC_CFDEF,pCField,256);
//                  pCFieldIndex->Len += _fstrlen(pCField) + 1;  
                  pCFieldIndex->Len += 256;  
                  pCFieldIndex->Num++;
                  GlobalUnlock (hComputedFields);
                  goto Display;
                  
              case IDC_FIELDS:  
              {   
                  HMENU EditMenu;             
                  POINT position;
				  switch(HIWORD(wParam))
                  {
                   case CBN_DBLCLK:
                   case CBN_SELCHANGE: 
                        EditMenu=CreatePopupMenu();             
                        AppendMenu (EditMenu,MF_ENABLED|MF_STRING,65001,IADDR("Change Name",1));
                        AppendMenu (EditMenu,MF_ENABLED|MF_STRING,65004,IADDR("Edit Definiton",1));
                        AppendMenu (EditMenu,MF_ENABLED|MF_STRING,65002,IADDR("Delete",1));
                        AppendMenu (EditMenu,MF_ENABLED|MF_STRING,65003,IADDR("Cancel",1));
                        GetCursorPos (&position);
                        TrackPopupMenu (EditMenu,TPM_LEFTBUTTON,position.x,position.y,0,hWndDlg,0);
                        DestroyMenu (EditMenu); 
                  }
                  break;
              }
              
              case IDC_FILES:  
              {   short   Choice;
              
                  switch(HIWORD(wParam))
                  {
                   case CBN_DBLCLK:
                     Choice=(short)SendDlgItemMessage(hWndDlg,IDC_FILES, LB_GETCURSEL,0,0);
                     SendDlgItemMessage(hWndDlg ,IDC_FILES,LB_GETTEXT,Choice,(LPARAM)pAddFile); 
                     hSQLCombo = pComboFile->hSQL[Choice];
                     goto GetFile; 
                  }
                  break;
              }
              
              case  IDC_EDIT_FILE:
				  Choice=(short)SendDlgItemMessage(hWndDlg,IDC_FILES, LB_GETCURSEL,0,0);
				  if (Choice >= 0)
				  	if (GetTextString (hWndDlg,pComboFile->FileNames[Choice],128,"Edit File Name",NULL,NULL,0,TRUE,TRUE))
	                    goto Display;
              	  break;
              	  
              case  IDC_ADD_FILE:
                  *pAddFile = 0; 
                  hSQLCombo = 0;
         GetFile:
                {
                      DLGPROC   lpfnCOMBO_ADD_FILEMsgProc; 
                            
                      lpfnCOMBO_ADD_FILEMsgProc = MakeProcInstance((DLGPROC)COMBO_ADD_FILEMsgProc, hInst);
                      CreateDialog(hInst, (LPSTR)"COMBO_ADD_FILE", hWndDlg, lpfnCOMBO_ADD_FILEMsgProc);
                }
                  break; 
              case IDC_CFTYPE:  
              {   short   Choice;
              
              switch(HIWORD(wParam))
                  {
                   case CBN_DBLCLK:
                   case CBN_SELCHANGE:
                     Choice=(short)SendDlgItemMessage(hWndDlg,IDC_CFTYPE, CB_GETCURSEL,0,0);
                     SendDlgItemMessage(hWndDlg ,IDC_CFTYPE,CB_GETLBTEXT,Choice,(LPARAM)str); 
                     if (!_fstricmp (str,"Text")) 
                     {
				         ShowWindow (GetDlgItem(hWndDlg,IDC_CFLEN),TRUE);
				         ShowWindow (GetDlgItem(hWndDlg,IDC_CFLEN_TITLE),TRUE);
                     }
                     else	
                     {
				         ShowWindow (GetDlgItem(hWndDlg,IDC_CFLEN),FALSE);
				         ShowWindow (GetDlgItem(hWndDlg,IDC_CFLEN_TITLE),FALSE);
                     }
                  }
                  break;
              }
              
                  
           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (616);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (616);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}   

BOOL FAR PASCAL DATAFILEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam, UINT cntlSQL,
                                UINT cntlSET_FILE, UINT cntlDATABASE_LIST, UINT cntlTABLE_NAMES,UINT cntlTABLE_NAMES_TITLE,
                                LPUINT pcntlFIELD_NAMESIn,int NumFieldLists,
                                LPSTR DataFile, short *DataFileType, HANDLE *hThemeDB,
                                LPBOOL pFieldListIsCBIn, BOOL WantBrackets)
#if ENABLETRACE
{GSSiEnterProg (617);
#endif
{
static short num_tables, LastTableChoice=-2,
                       LastDBChoice=-2, LastFieldChoice=-2;
static  FIELDINFO FIELD;
static  LPFIELDINFO lpFieldInfo = &FIELD;
    short     Choice, rc, i,j, outlen, deslen,ii;
	int		  type;
    static  BOOL ValidTable = FALSE;     
    HANDLE   DBHandle;
    LPSTR   lpSTRING, pBAR;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr; 
    LPBOOL pFieldListIsCB;  
    LPUINT pcntlFIELD_NAMES; 
    HANDLE	hMem=GSSiGlobAlloc ( 260,GMEM_MOVEABLE,4096);
    LPSTR 	str=GlobalLock (hMem);
    LPSTR   szDescription=str+1024;
    LPSTR	names=szDescription+256;
    LPSTR	TableName=names+256;
    LPSTR	cwd=TableName+256;
    LPSTR	AttDir=cwd+256;
    LPSTR	Ext = AttDir+256;
    LPSTR	ExtID=Ext+16;  
                                         
 switch(Message)
   {
    case GSSI_REINITDIALOG:
        ii=1;
    case WM_INITDIALOG:
         LastTableChoice=LastDBChoice=LastFieldChoice=-2;
LoadFields:
         if (DataFile[0])
         {  
            if (!_fstrnicmp (DataFile,"ODBC|",5))
            {   LPSTR   lpDB, lpTABLE;
                             
                SetDlgItemText(hWndDlg,cntlDATABASE_LIST,DataFile);
                lpDB = &DataFile[5];
                lpTABLE = _fstrchr (lpDB,'|');
                if (lpTABLE)
                {
                    *lpTABLE = '\0';
//                    SetDlgItemText(hWndDlg,cntlDATABASE_LIST,lpDB);
                    *lpTABLE++ = '|';
                    SetDlgItemText(hWndDlg,cntlTABLE_NAMES,lpTABLE);
                    ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES),SW_SHOW); 
                    ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES_TITLE),SW_SHOW); 
                }
                else  
                {
                    ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES),SW_HIDE);                   
                    ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES_TITLE),SW_HIDE); 
                    SetDlgItemText(hWndDlg,cntlDATABASE_LIST,lpDB);
                }
            }
            else
            {
                SetDlgItemText(hWndDlg,cntlDATABASE_LIST,DataFile);
                ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES),SW_HIDE);                   
                ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES_TITLE),SW_HIDE); 
            }
            _fstrcpy (ThemeDB,DataFile); 
            for (i=0,pFieldListIsCB=pFieldListIsCBIn,pcntlFIELD_NAMES=pcntlFIELD_NAMESIn;i<NumFieldLists;i++,pFieldListIsCB++,pcntlFIELD_NAMES++)
            {
                if (*pFieldListIsCB)
                    SendDlgItemMessage (hWndDlg,*pcntlFIELD_NAMES,CB_RESETCONTENT,0,0);
                else
                    SendDlgItemMessage (hWndDlg,*pcntlFIELD_NAMES,LB_RESETCONTENT,0,0);
            }
            if ((type = OpenDataFile (DataFile,"",BT_READ,hThemeDB)))
			{
	            SQLPtr = (LPOPENSQLDATA) GlobalLock (*hThemeDB);
	            FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
	            lpFieldInfo = &FilePtr->FldInfo; 
	            for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++) 
	            {
	                for (j=0,pFieldListIsCB=pFieldListIsCBIn,pcntlFIELD_NAMES=pcntlFIELD_NAMESIn;j<NumFieldLists;j++,pFieldListIsCB++,pcntlFIELD_NAMES++)
	                {   
	                    if (WantBrackets)
	                        sprintf (str,"[%s]",lpFieldInfo->name);
	                    else
	                        _fstrcpy (str,lpFieldInfo->name);
	                    if (*pFieldListIsCB)
	                        SendDlgItemMessage (hWndDlg,*pcntlFIELD_NAMES,CB_ADDSTRING,
	                                            0,(LPARAM)((LPSTR) str));
	                    else
	                        SendDlgItemMessage (hWndDlg,*pcntlFIELD_NAMES,LB_ADDSTRING,
	                            0,(LPARAM)((LPSTR) str));
	                }
	            }           
	            GlobalUnlock (SQLPtr->OFHandle);
	            GlobalUnlock (*hThemeDB); 
				*DataFileType = type;
	        }
            PostMessage(hWndDlg, WM_COMMAND, IDC_OPEN_DB, 0L);
            
         }
        goto RtnFalse;
        
    case WM_DESTROY:
        CloseDataFile (FALSE,hThemeDB);      
    	DestroyFieldList ();
    	goto RtnFalse;
    	
    case WM_COMMAND:
         if (LOWORD(wParam) == cntlSET_FILE)
         {
             Choice=(short)SendDlgItemMessage(hWndDlg,cntlDATABASE_LIST, CB_GETCURSEL,0,0);
             if (!Choice && *DataFileType == -UMIFS_DATAFILE)
             	Choice = 3;
//             if(Choice == LastDBChoice && Choice != 0)break;
             LastDBChoice = Choice;
             LastTableChoice = -1;
             LastFieldChoice = -1; 
             CloseDataFile (FALSE,hThemeDB);  
             switch (Choice)
             {
             
             	 case 0:
	             {  
	                    
                    ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES_TITLE),SW_HIDE); 
	                ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES),SW_HIDE);  
	                *AttDir = 0;
	                _fstrcpy (Ext,".TXT;*.CSV"); 
	                _fstrcpy (ExtID,"Comma or tab delimited text files");
	                sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,Ext,_fstrlwr(Ext));
	                if (GetFileName3(hWndDlg,AttDir,0,IDS_FILETXT))   
	                {
	                    _fstrcpy (DataFile,AttDir);
	                    SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_RESETCONTENT,0,0);
	                    SetDlgItemText(hWndDlg,cntlDATABASE_LIST,DataFile);
		                *DataFileType = ODBC_DATAFILE;
//		                OpenDataFile (DataFile,"",BT_READ,hThemeDB); 
	                    goto LoadFields;
	                }
					SendDlgItemMessage(hWndDlg,cntlDATABASE_LIST, CB_SETCURSEL,-1,0);                 
	             } 
	             break;
	             
             	 case 1:
	             {  
	                    
                    ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES_TITLE),SW_HIDE); 
	                ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES),SW_HIDE);  
	                *AttDir = 0;
	                _fstrcpy (Ext,".DBF"); 
	                _fstrcpy (ExtID,"Dbase Files");
	                sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,Ext,_fstrlwr(Ext));
	                if (GetFileName3(hWndDlg,AttDir,0,IDS_FILEDBF))   
	                {
	                    _fstrcpy (DataFile,AttDir);
	                    SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_RESETCONTENT,0,0);
	                    SetDlgItemText(hWndDlg,cntlDATABASE_LIST,DataFile);
		                *DataFileType = DBF_DATAFILE;
	                    goto LoadFields;
	                }
					SendDlgItemMessage(hWndDlg,cntlDATABASE_LIST, CB_SETCURSEL,-1,0);                 
	             } 
	             break;
	             
             	 case 2:
	             {  
	                    
                    ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES_TITLE),SW_HIDE); 
	                ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES),SW_HIDE);  
	                _fstrcpy (AttDir,"[%DATA_LOC]attribut");
	                ExpandText (AttDir); 
	                *AttDir = 0;
	                _fstrcpy (Ext,".GCN"); 
	                _fstrcpy (ExtID,"GeoMaster Census Data Files");
	                sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,Ext,_fstrlwr(Ext));
	                if (GetFileName3(hWndDlg,AttDir,0,IDS_FILEGCN))   
	                {
	                    _fstrcpy (DataFile,AttDir);
	                    SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_RESETCONTENT,0,0);
	                    SetDlgItemText(hWndDlg,cntlDATABASE_LIST,DataFile);
	                    if (StringEndsWith (DataFile,".gcn"))
	                        *DataFileType = GMCENSUS_DATAFILE;
	                    goto LoadFields;
	                } 
					SendDlgItemMessage(hWndDlg,cntlDATABASE_LIST, CB_SETCURSEL,-1,0);                 
	             } 
	             break;

             	 case 3:
	             {  
	                    
                    ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES_TITLE),SW_HIDE); 
	                ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES),SW_HIDE);  
	                _fstrcpy (AttDir,"[%DATA_LOC]attribut");
	                ExpandText (AttDir); 
	                *AttDir = 0;
	                _fstrcpy (Ext,".GCF"); 
	                _fstrcpy (ExtID,"GeoMaster Combo Files");
	                sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,Ext,_fstrlwr(Ext));
	                if (GetFileName3(hWndDlg,AttDir,0,IDS_FILEGCF))   
	                {
	                    _fstrcpy (DataFile,AttDir);
	                    SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_RESETCONTENT,0,0);
	                    SetDlgItemText(hWndDlg,cntlDATABASE_LIST,DataFile);
	                    if (StringEndsWith (DataFile,".gcf"))
	                        *DataFileType = COMBO_DATAFILE;
	                    goto LoadFields;
	                } 
					SendDlgItemMessage(hWndDlg,cntlDATABASE_LIST, CB_SETCURSEL,-1,0);                 
	             } 
	             break;

             	 case 4:
	             {  
	                    
                    ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES_TITLE),SW_HIDE); 
	                ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES),SW_HIDE);  
	                _fstrcpy (AttDir,"[%DATA_LOC]attribut");
	                ExpandText (AttDir); 
	                *AttDir = 0;
	                _fstrcpy (Ext,".GMD"); 
	                _fstrcpy (ExtID,"GeoMaster Data Files");
	                sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,Ext,_fstrlwr(Ext));
	                if (GetFileName3(hWndDlg,AttDir,0,IDS_FILEGMD))   
	                {
	                    _fstrcpy (DataFile,AttDir);
	                    SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_RESETCONTENT,0,0);
	                    SetDlgItemText(hWndDlg,cntlDATABASE_LIST,DataFile);
	                    if (StringEndsWith (DataFile,".gmd"))
	                        *DataFileType = UMIFS_DATAFILE;
	                    goto LoadFields;
	                } 
					SendDlgItemMessage(hWndDlg,cntlDATABASE_LIST, CB_SETCURSEL,-1,0);                 
	             } 
	             break;

				 case 5:
                 {
                    _fstrcpy (DataFile,"[%INDIR]attribut\\graphics.gmd"); 
                    SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_RESETCONTENT,0,0);
                    SetDlgItemText(hWndDlg,cntlDATABASE_LIST,DataFile);
                    *DataFileType = UMIFS_DATAFILE;
                    if (cntlSQL)  
						SetDlgItemText (hWndDlg,cntlSQL,"INT_REFNO = [%INT_REFNO]");	
                    goto LoadFields;
                 } 
				 	             
				 case 6:
                 {
                    _fstrcpy (DataFile,"[%INDIR]attribut\\graphic2.gmd"); 
                    SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_RESETCONTENT,0,0);
                    SetDlgItemText(hWndDlg,cntlDATABASE_LIST,DataFile);
                    *DataFileType = UMIFS_DATAFILE;
                    if (cntlSQL)  
						SetDlgItemText (hWndDlg,cntlSQL,"INT_REFNO = [%INT_REFNO]");	
                    goto LoadFields;
                 } 
				 	             
				 case 7:
                 {
                    if (cntlSQL)  
						SetDlgItemText (hWndDlg,cntlSQL,"");	
                    ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES_TITLE),SW_HIDE); 
	                ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES),SW_HIDE);  
                    ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES_TITLE),SW_HIDE); 
	                *AttDir = 0;
	                _fstrcpy (Ext,".DTM"); 
	                _fstrcpy (ExtID,"GeoMaster Digital Terrain Models");
	                if (GetFileName3(hWndDlg,AttDir,IDS_FILTERSURFACE,IDS_FILEDTM))   
	                {
	                    _fstrcpy (DataFile,AttDir);
	                    SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_RESETCONTENT,0,0);
	                    SetDlgItemText(hWndDlg,cntlDATABASE_LIST,DataFile);
		                *DataFileType = DTM_DATAFILE;
	                    goto LoadFields;
	                }
					SendDlgItemMessage(hWndDlg,cntlDATABASE_LIST, CB_SETCURSEL,-1,0);
                 } 
				 break;                 
				 case 8:
                 {  
                 	if (SelectSHPFile (DataFile,NULL))
                    { 
	                    SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_RESETCONTENT,0,0);
	                    SetDlgItemText(hWndDlg,cntlDATABASE_LIST,DataFile);
	                    *DataFileType = SHAPE_DATAFILE;
	                    if (cntlSQL)   
	                    {
	                    	if (_fstrstr (DataFile,".MDB"))
								SetDlgItemText (hWndDlg,cntlSQL,"OBJECTID = [%SHPRECNO]");	
	                    	else
								SetDlgItemText (hWndDlg,cntlSQL,"SHAPEREC = [%SHPRECNO]");
						}	
	                    goto LoadFields;
	                }
					else 
					{
	                    ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES_TITLE),SW_HIDE); 
		                ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES),SW_HIDE);  
		                *AttDir = 0;
		                _fstrcpy (Ext,".SHP"); 
		                _fstrcpy (ExtID,"Shape Files");
		                sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,Ext,_fstrlwr(Ext));
		                if (GetFileName3(hWndDlg,AttDir,0,IDS_FILESHP))   
		                {
		                    _fstrcpy (DataFile,AttDir);
		                    SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_RESETCONTENT,0,0);
		                    SetDlgItemText(hWndDlg,cntlDATABASE_LIST,DataFile);
			                *DataFileType = SHAPE_DATAFILE;
		                    goto LoadFields;
		                }
		            }
					SendDlgItemMessage(hWndDlg,cntlDATABASE_LIST, CB_SETCURSEL,-1,0);                 
                 } 
				 break;	             
				 	             
				 case 9:
                 {
                    if (cntlSQL)  
						SetDlgItemText (hWndDlg,cntlSQL,"");	
                    ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES_TITLE),SW_HIDE); 
	                ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES),SW_HIDE);  
	                *AttDir = 0;
	                _fstrcpy (Ext,".SQL"); 
	                _fstrcpy (ExtID,"GeoMaster SQL Database");
	                sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,Ext,_fstrlwr(Ext));
	                if (GetFileName3(hWndDlg,AttDir,0,IDS_FILESQL))   
	                {
	                    _fstrcpy (DataFile,AttDir);
	                    SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_RESETCONTENT,0,0);
	                    SetDlgItemText(hWndDlg,cntlDATABASE_LIST,DataFile);
		                *DataFileType = SQL_DATAFILE;
	                    goto LoadFields;
	                }
					SendDlgItemMessage(hWndDlg,cntlDATABASE_LIST, CB_SETCURSEL,-1,0);
                 } 
				 break;                 
				 case 10:
				 {
					 if (cntlSQL)
						 SetDlgItemText(hWndDlg, cntlSQL, "");
					 ShowWindow(GetDlgItem(hWndDlg, cntlTABLE_NAMES_TITLE), SW_HIDE);
					 ShowWindow(GetDlgItem(hWndDlg, cntlTABLE_NAMES), SW_HIDE);
					 *AttDir = 0;
					 _fstrcpy(Ext, ".SLT");
					 _fstrcpy(ExtID, "SQLite Database");
					 sprintf(gszFilter, "%s(*%s)|*%s|", ExtID, Ext, _fstrlwr(Ext));
					 if (GetFileName3(hWndDlg, AttDir, 0, IDS_FILESQL))
					 {
						 _fstrcpy(DataFile, AttDir);
						 SendDlgItemMessage(hWndDlg, cntlDATABASE_LIST, CB_RESETCONTENT, 0, 0);
						 SetDlgItemText(hWndDlg, cntlDATABASE_LIST, DataFile);
						 *DataFileType = SLT_DATAFILE;
						 OpenDataFile(DataFile, "", BT_READ, hThemeDB);
						 ShowWindow(GetDlgItem(hWndDlg, cntlTABLE_NAMES), SW_SHOW);
						 ShowWindow(GetDlgItem(hWndDlg, cntlTABLE_NAMES_TITLE), SW_SHOW);
					 }
					 else
						SendDlgItemMessage(hWndDlg, cntlDATABASE_LIST, CB_SETCURSEL, -1, 0);
				 }
					 break;
				 case 11:
					 break;

             	 default:
	             { // user picked a ODBC Driver
	               _fstrcpy(DataFile, "ODBC|");
	               SendDlgItemMessage(hWndDlg,cntlDATABASE_LIST,CB_GETLBTEXT, 
	                                    Choice,(LPARAM)((LPSTR)&DataFile[5]));
	               SetDlgItemText(hWndDlg,cntlDATABASE_LIST,DataFile);
	               *DataFileType = ODBC_DATAFILE;
				   ODBCTerminate (TRUE);
	               OpenDataFile (DataFile,"",BT_READ,hThemeDB); 
	               ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES),SW_SHOW);  
	               ShowWindow (GetDlgItem(hWndDlg,cntlTABLE_NAMES_TITLE),SW_SHOW);  
	
	                //goto s44;//LoadFields;  
	             }  
	             break;
	         }
             goto RtnTrue;
        }
        else if (LOWORD(wParam) == cntlDATABASE_LIST)
        {
              switch(HIWORD(wParam))
              {
              case CBN_SELCHANGE:
              case CBN_DBLCLK:   
	             Choice=(short)SendDlgItemMessage(hWndDlg,cntlDATABASE_LIST, CB_GETCURSEL,0,0); 
                 if (Choice == 11)    
                 {
					SendDlgItemMessage(hWndDlg,cntlDATABASE_LIST, CB_SETCURSEL,-1,0);                 
                 	break;
                 }
                 PostMessage(hWndDlg, WM_COMMAND, cntlSET_FILE, 0L);
              break;
              case CBN_DROPDOWN:
               SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_RESETCONTENT,0,0);  
               if (*DataFileType == -UMIFS_DATAFILE)
	               SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_ADDSTRING,0,
	                                        (LPARAM)((LPSTR) " GeoMaster data files (*.GMD)")); 
	           else
	           {
	               SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_ADDSTRING,0,
	                                        (LPARAM)((LPSTR) " Comma or tab delimited text files (*.TXT;*.CSV)"));
	               SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_ADDSTRING,0,
	                                        (LPARAM)((LPSTR) " Dbase files (*.DBF)"));
	               SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_ADDSTRING,0,
	                                        (LPARAM)((LPSTR) " GeoMaster census data files (*.GCN)"));
	               SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_ADDSTRING,0,
	                                        (LPARAM)((LPSTR) " GeoMaster combo files (*.GCF)"));
	               SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_ADDSTRING,0,
	                                        (LPARAM)((LPSTR) " GeoMaster data files (*.GMD)"));
	               SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_ADDSTRING,0,
	                                        (LPARAM)((LPSTR) " GeoMaster graphics data"));
	               SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_ADDSTRING,0,
	                                        (LPARAM)((LPSTR) " GeoMaster graphics data (basic)"));
	               SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_ADDSTRING,0,
	                                        (LPARAM)((LPSTR) " GeoMaster terrain model"));
	               SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_ADDSTRING,0,
	                                        (LPARAM)((LPSTR) " Shape Files (*.SHP)"));
				   SendDlgItemMessage(hWndDlg, cntlDATABASE_LIST, CB_ADDSTRING, 0,
											(LPARAM)((LPSTR) " SQL Files (*.SQL)"));
				   SendDlgItemMessage(hWndDlg, cntlDATABASE_LIST, CB_ADDSTRING, 0,
											(LPARAM)((LPSTR) " SQLIte Files (*.SLT)"));
				   SendDlgItemMessage(hWndDlg, cntlDATABASE_LIST, CB_ADDSTRING, 0,
	                                        (LPARAM)((LPSTR) "0 ----------------- ODBC Data Sources -----------------'"));
	               rc = GetDataSource(*hThemeDB, TRUE,
	                    names, 64, &outlen, szDescription, (SWORD) 64, &deslen); 
	                 while (rc != SQL_NO_DATA)
	                 { 
	                  _fstrupr(names);
	                 // if (_fstrcmp (names,"TXT") && _fstrcmp (names,"DBF"))
	                  	i=(short)SendDlgItemMessage (hWndDlg,cntlDATABASE_LIST,CB_ADDSTRING,0,
	                                        (LPARAM)((LPSTR) names));
	                  rc = GetDataSource( *hThemeDB, FALSE,  
	                       names, 64, &outlen, szDescription, (SWORD) 64, &deslen);
	             	} 
                 } 
              } //end of the switch   
             goto RtnTrue;
        }
        else if (LOWORD(wParam) == cntlTABLE_NAMES)  
        {
              i = HIWORD(wParam);
              switch(HIWORD(wParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
				   if (*DataFileType != ODBC_DATAFILE && *DataFileType != SLT_DATAFILE)
                 	break;
                 if (!*hThemeDB)
                 {  
                 	LPSTR	pLoc = DataFile;
                 	
                 	if ((pLoc = _fstrchr (pLoc,'|')))
                 	{
                 		pLoc++;
	                 	if ((pLoc = _fstrchr (pLoc,'|')))
	                 	{
	                 		*pLoc = 0;
                 			OpenDataFile (DataFile,"",BT_READ,hThemeDB);
                 		}
                 	} 
                 }
                 if (!*hThemeDB)
                 	break;
                 DBHandle = GetDBHandleFromSQL (*hThemeDB);
                 Choice=(short)SendDlgItemMessage(hWndDlg,cntlTABLE_NAMES, CB_GETCURSEL,0,0);
//               if(Choice == LastTableChoice) break;
                 LastTableChoice = Choice;
                  LastFieldChoice = -1;
                   ValidTable = TRUE;                           
                   SendDlgItemMessage(hWndDlg,cntlTABLE_NAMES,CB_GETLBTEXT,
                                    Choice,(DWORD)TableName); 
                   if ((pBAR = _fstrchr (DataFile,'|')))
                   		pBAR = _fstrchr (++pBAR,'|');
                   if (pBAR)
                   		*pBAR = 0;
                   _fstrcat(DataFile,"|");
                   _fstrcat(DataFile,(LPCSTR)TableName);
                   CloseDataFile (FALSE,hThemeDB);                  
                    
                   goto LoadFields; 
                case CBN_DROPDOWN:
					if (!*hThemeDB || (*DataFileType != ODBC_DATAFILE && *DataFileType != SLT_DATAFILE)) break;
                   DBHandle = GetDBHandleFromSQL (*hThemeDB);
                   lpSTRING =  GetTableName (DBHandle, TRUE,*DataFileType); // the first table name 
                   if (lpSTRING == 0) break;
                   i=(short)SendDlgItemMessage (hWndDlg,cntlTABLE_NAMES,CB_RESETCONTENT,0,0);
                   i = 0;
                   while (lpSTRING && lpSTRING[0] != 0 )
                   {
                    i=(short)SendDlgItemMessage (hWndDlg,cntlTABLE_NAMES,CB_ADDSTRING,0,(LPARAM)((LPSTR) lpSTRING));
					lpSTRING = GetTableName(DBHandle, FALSE,*DataFileType); // subsequent table names 
                   } 
                     
               } //end of the switch
                break; 
             goto RtnTrue;
           }
    default:  
    RtnFalse:
    	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (617);
#endif
        return FALSE;
}
   }
RtnTrue:
 GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (617);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL SetFieldValFromDlgItem (LPGWDHEADER lpGWDHead,HWND hWndDlg,UINT icntl,LPSTR FieldName)
{
	char	str[260];
	
	*str = 0;
	GetDlgItemText (hWndDlg,icntl,str,256);
	return SetFieldValFromCharAndName(lpGWDHead, FieldName, str, FALSE, TRUE);
}
  
BOOL SetFieldValFromCharAndName(LPGWDHEADER lpGWDHead,LPSTR FieldNameIn,LPSTR CharVal,BOOL BinMode,BOOL ExpandValue)
#if ENABLETRACE
{GSSiEnterProg (621);
#endif
{ 
	LPGWFLDINFO lpGWFldInfo;  
	short	i;
	LPSTR	pLast;   
	BOOL	IncValue=FALSE;
	HANDLE	hMem=GSSiGlobAlloc (1519,GMEM_MOVEABLE,256);
	LPSTR 	FieldName=GlobalLock (hMem);
	
	_fstrcpy (FieldName,FieldNameIn);
	ExpandText (FieldName);  
	pLast=LastChr (FieldName);
	if (*pLast == '+')
	{
		IncValue=TRUE;
		*pLast = 0;
	}
	for (i=0,lpGWFldInfo=lpGWDHead->pFldInfo;i<lpGWDHead->NumFields;i++,lpGWFldInfo++)
	{	
		if (!_fstricmp (lpGWFldInfo->Name,FieldName))  
		{
			GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (621);
#endif
			return SetFieldValFromChar(lpGWDHead,lpGWFldInfo,CharVal,BinMode,IncValue,ExpandValue);
} 
		}
	} 
	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (621);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

BOOL SetFieldValFromChar(LPGWDHEADER lpGWDHead,LPGWFLDINFO lpGWFldInfo,LPSTR InCharVal,BOOL BinMode,BOOL IncrementValue,BOOL ExpandValue)
#if ENABLETRACE
{GSSiEnterProg (622);
#endif
{   
    LPSTR   lpVal;
    HANDLE	hMem=GSSiGlobAlloc (1520,GMEM_MOVEABLE,4096);
    LPSTR	CharVal=GlobalLock (hMem); 
    short     l, Type=lpGWFldInfo->Type,ln;
    BOOL	rtn=TRUE; 
    BOOL	Unset=FALSE;
    
    SetGWDCurrentOffset (lpGWDHead,-1);
    if (BinMode)
    	Type = BT_CHAR; 
    else
    {
	    _fstrcpy (CharVal,InCharVal);
	    if (ExpandValue)
			ExpandText (CharVal); 
	    ln = _fstrlen (CharVal);
    }
    lpGWFldInfo->HasValue = 1;
    lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];
    switch (Type)
    {   
        default:
        case BT_CHAR:   
        	if (BinMode)
            	_fmemmove ((LPSTR)lpVal,InCharVal,lpGWFldInfo->Len);
        	else 
        	{
	        	if (ln && *CharVal == '\'' && CharVal[ln-1] == '\'')
	        	{
	        		CharVal[ln-1] = 0;
	        		CharVal++;  
	        		Unset = TRUE;
	        	}
	            _fstrncpy ((LPSTR)lpVal,CharVal,lpGWFldInfo->Len);
	            if (ln > lpGWFldInfo->Len)
	            	rtn=FALSE; 
	            if (Unset)
	            	CharVal[ln-1] = '\'';
             }
        break;
                                
        case BT_RIGHT_CHAR:
            _fmemset (lpVal,' ', lpGWFldInfo->Len);
            l = _fstrlen (CharVal);
            lpVal+= max(lpGWFldInfo->Len-l,0);
            _fmemmove (lpVal,CharVal,max(l,lpGWFldInfo->Len));
        break;
                                
        case BT_INTEGER:    
        	if (IncrementValue)
        	{
	            if (lpGWFldInfo->Len == 2)
	                *(LPSHORT)lpVal += IDNINT (atof(CharVal));
	            else
	                *(LPLONG)lpVal += IDNINT (atof(CharVal));
	        }
	        else
        	{
	            if (lpGWFldInfo->Len == 2)
	                *(LPSHORT)lpVal = IDNINT (atof(CharVal));
	            else
	                *(LPLONG)lpVal = IDNINT (atof(CharVal));
	        }
        break;
                                
        case BT_REAL:  
       	if (IncrementValue)
        {
            if (lpGWFldInfo->Len == 4)
                *(LPFLOAT)lpVal += (float)atof(CharVal);
            else
                *(LPDOUBLE)lpVal += atof(CharVal);  
        } 
        else
        {
            if (lpGWFldInfo->Len == 4)
                *(LPFLOAT)lpVal = (float)atof(CharVal);
            else
                *(LPDOUBLE)lpVal = atof(CharVal);  
        }
        break;
     } 
     GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (622);
#endif
     return rtn; 
}
#if ENABLETRACE
}
#endif
}

BOOL ConvertFieldValFromChar (LPGWDHEADER lpGWDHead,int FieldNum,LPSTR CharVal,LPSTR lpVal)
#if ENABLETRACE
{GSSiEnterProg (623);
#endif
{   
	LPGWFLDINFO lpGWFldInfo=lpGWDHead->pFldInfo;  
    short     ln = _fstrlen (CharVal);
    BOOL	Unset=FALSE;
    
	lpGWFldInfo+=FieldNum;
    switch (lpGWFldInfo->Type)
    {   
        default:
        case BT_CHAR:
        	if (ln && *CharVal == '\'' && CharVal[ln-1] == '\'')
        	{
        		CharVal[ln-1] = 0;
        		CharVal++;  
        		Unset = TRUE;
        	}
           	_fstrncpy ((LPSTR)lpVal,CharVal,lpGWFldInfo->Len);  
           	if (Unset)
           		CharVal[ln-1] = '\'';
        break;
                                
        case BT_RIGHT_CHAR:
            _fmemset (lpVal,' ', lpGWFldInfo->Len);
            lpVal+= max(lpGWFldInfo->Len-ln,0);
            _fmemmove (lpVal,CharVal,max(ln,lpGWFldInfo->Len));
        break;
                                
        case BT_INTEGER:
            if (lpGWFldInfo->Len == 2)
                *(LPSHORT)lpVal = atoi(CharVal);
            else
                *(LPLONG)lpVal = atol(CharVal);
        break;
                                
        case BT_REAL:
            if (lpGWFldInfo->Len == 4)
                *(LPFLOAT)lpVal = (float)atof(CharVal);
            else
                *(LPDOUBLE)lpVal = atof(CharVal);
        break;
     }
{
#if ENABLETRACE
GSSiExitProg (623);
#endif
     return TRUE; 
}
#if ENABLETRACE
}
#endif
}

short TestFieldVal (LPGWDHEADER lpGWDHead,int FieldNum,LPSTR lpVal,short OpCode)
#if ENABLETRACE
{GSSiEnterProg (624);
#endif
{   
	LPGWFLDINFO lpGWFldInfo=lpGWDHead->pFldInfo;  
    LPSTR   lpVal2; 
    short     l, rtn=0;
    
	lpGWFldInfo+=FieldNum;
    
    lpVal2 = &lpGWDHead->GWDData[lpGWFldInfo->Beg];
    switch (lpGWFldInfo->Type)
    {   
        default:
        case BT_CHAR:
        case BT_RIGHT_CHAR:
           	rtn = _fstrncmp (lpVal,lpVal2,lpGWFldInfo->Len);
        break; 
        
        case BT_INTEGER:
            if (lpGWFldInfo->Len == 2)
            {
                if (*(LPSHORT)lpVal < *(LPSHORT)lpVal2)
                	rtn=-1;
                else if (*(LPSHORT)lpVal > *(LPSHORT)lpVal2)
                	rtn=1; 
            }
            else
            {
                if (*(LPLONG)lpVal < *(LPLONG)lpVal2)
                	rtn=-1;
                else if (*(LPLONG)lpVal > *(LPLONG)lpVal2)
                	rtn=1; 
            }
        break;
                                
        case BT_REAL:
            if (lpGWFldInfo->Len == 4)
            {
                if (*(LPFLOAT)lpVal < *(LPFLOAT)lpVal2)
                	rtn=-1;
                else if (*(LPFLOAT)lpVal > *(LPFLOAT)lpVal2)
                	rtn=1; 
            }
            else
            {
                if (*(LPDOUBLE)lpVal < *(LPDOUBLE)lpVal2)
                	rtn=-1;
                else if (*(LPDOUBLE)lpVal > *(LPDOUBLE)lpVal2)
                	rtn=1; 
            }
        break;
     }
     if (OpCode == OPCODE_GE && rtn < 0)
     	rtn = 0;
     else if (OpCode == OPCODE_LE && rtn > 0)
     	rtn = 0;
{
#if ENABLETRACE
GSSiExitProg (624);
#endif
     return rtn; 
}
#if ENABLETRACE
}
#endif
}

BOOL GetFieldMinValue (LPGWFLDINFO lpGWFldInfo,LPVOID lpVal)
{
    switch (lpGWFldInfo->Type)
    {   
        default:
        case BT_CHAR:
        case BT_RIGHT_CHAR:
            _fmemset (lpVal,0, lpGWFldInfo->Len);
        break;
                                
        case BT_INTEGER:
            if (lpGWFldInfo->Len == 2)
                *(LPSHORT)lpVal = SHRT_MIN;
            else
                *(LPLONG)lpVal = LONG_MIN;
        break;
                                
        case BT_REAL:
            if (lpGWFldInfo->Len == 4)
                *(LPFLOAT)lpVal = -FLT_MAX;
            else
                *(LPDOUBLE)lpVal = -DBL_MAX;
        break;
     } 
	return TRUE;
}

BOOL SetFieldToMinVal(LPGWDHEADER lpGWDHead,int FieldNum)
#if ENABLETRACE
{GSSiEnterProg (625);
#endif
{   
	LPGWFLDINFO lpGWFldInfo=lpGWDHead->pFldInfo;  
    LPSTR   lpVal; 
    short     l;
    
    SetGWDCurrentOffset (lpGWDHead,-1);
	lpGWFldInfo+=FieldNum;
	
    lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];
	GetFieldMinValue (lpGWFldInfo,lpVal);
    lpGWFldInfo->HasValue = 1;
{
#if ENABLETRACE
GSSiExitProg (625);
#endif
     return TRUE; 
}
#if ENABLETRACE
}
#endif
}

BOOL SetFieldValFromLong(LPGWDHEADER lpGWDHead,LPGWFLDINFO lpGWFldInfo,long LVal)
#if ENABLETRACE
{GSSiEnterProg (626);
#endif
{   
    LPVOID  lpVal;
    
    SetGWDCurrentOffset (lpGWDHead,-1);
    lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];
    switch (lpGWFldInfo->Type)
    {   
        default:
        case BT_RIGHT_CHAR:
        case BT_CHAR: 
        {   char    format[8]="%";
            
            itoa (lpGWFldInfo->Len,&format[1],10);
            _fstrcat(format,"ld");
            sprintf ((LPSTR)lpVal,format,LVal);
        }
        break; 
        
        case BT_INTEGER:
            if (lpGWFldInfo->Len == 2)
            {
                if (LVal > SHRT_MAX)
{
#if ENABLETRACE
GSSiExitProg (626);
#endif
                	return FALSE;
}
                *(LPSHORT)lpVal = LVal;              
            }
            else
                *(LPLONG)lpVal = LVal;
        break;
                                
        case BT_REAL:
            if (lpGWFldInfo->Len == 4)
                *(LPFLOAT)lpVal = (float)LVal;
            else
                *(LPDOUBLE)lpVal = LVal;
        break;
     }
{
#if ENABLETRACE
GSSiExitProg (626);
#endif
     return TRUE; 
}
#if ENABLETRACE
}
#endif
}  

BOOL SetFieldValFromReal(LPGWDHEADER lpGWDHead,LPGWFLDINFO lpGWFldInfo,double LVal)
#if ENABLETRACE
{GSSiEnterProg (626);
#endif
{   
    LPVOID  lpVal;
    
    SetGWDCurrentOffset (lpGWDHead,-1);
    lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];
    switch (lpGWFldInfo->Type)
    {   
        default:
        case BT_RIGHT_CHAR:
        case BT_CHAR: 
        {   char    format[8]="%";
            
            itoa (lpGWFldInfo->Len,&format[1],10);
            _fstrcat(format,"f");
            sprintf ((LPSTR)lpVal,format,LVal);
        }
        break; 
        
        case BT_REAL:
            if (lpGWFldInfo->Len == 4)
                *(LPFLOAT)lpVal = (float)LVal;
            else
                *(LPDOUBLE)lpVal = LVal;
        break;
     }
{
#if ENABLETRACE
GSSiExitProg (626);
#endif
     return TRUE; 
}
#if ENABLETRACE
}
#endif
}  

BOOL GWDAddIndex (LPSTR Name,HANDLE hKeyFields,int IndexType,LPMNMXCORD pBounds)
{   
	HANDLE hDB = OpenGWDatabase (Name,BT_WRITE);
    LPGWDHEADER lpGWDHead;
    BOOL	rtn=FALSE;
    LPINT	pFields;   
    BOOL	SaveCheckTimeStamps = CheckTimeStamps;
	int		i, code = SHRT_MAX, mult=1;
	
	if (!hDB)
		return FALSE;  
    lpGWDHead =(LPGWDHEADER) GlobalLock (hDB); 
    if (lpGWDHead->NumIndex >= MAX_GMD_INDEXES) 
    {
		GlobalUnlock (hDB);
    	goto Exit;
    } 
    pFields = (LPINT)GlobalLock (hKeyFields);
	switch (IndexType)
	{
	case -1: //sparse non unique
		mult = -1;
	case 0: //non unique
        code = SHRT_MIN;   
	case 1: //unique
	   	lpGWDHead->lKeys[lpGWDHead->NumIndex]=code;
		lpGWDHead->NumIndexFields[lpGWDHead->NumIndex] = mult * (*pFields++); 
		for (i=0;i<abs(lpGWDHead->NumIndexFields[lpGWDHead->NumIndex]);i++)
			lpGWDHead->IndexFields[lpGWDHead->NumIndex][i] =  *pFields++;
		lpGWDHead->NumIndex++;
		rtn = TRUE;
		break;
	case 2: //spatial index (month,grid)
		if (*pFields++ != 4)
			break;
		lpGWDHead->GridBounds = *pBounds;
		lpGWDHead->SpatialIndex = lpGWDHead->NumIndex;
		lpGWDHead->NumIndexFields[lpGWDHead->NumIndex] = 2;
		lpGWDHead->GridInc = 128;
		lpGWDHead->SpatialIndexType = 1,
		lpGWDHead->FromDateField = *pFields++;
		lpGWDHead->ToDateField = *pFields++;
		lpGWDHead->XField = *pFields++;
		lpGWDHead->YField = *pFields++;
		lpGWDHead->CoordConversion = 0;
		lpGWDHead->NumIndex++;
		rtn = TRUE;
		break;
	case 3: //spatial index (month,code,grid)
		if (*pFields++ != 5)
			break;
		lpGWDHead->GridBounds = lpGWDHead->FileBounds = *pBounds;
		lpGWDHead->SpatialIndex = lpGWDHead->NumIndex;
		lpGWDHead->NumIndexFields[lpGWDHead->NumIndex] = 3;
		lpGWDHead->GridInc = 128;
		lpGWDHead->SpatialIndexType = 2,
		lpGWDHead->FromDateField = *pFields++;
		lpGWDHead->ToDateField = *pFields++;
		lpGWDHead->SymbolField = *pFields++;
		lpGWDHead->XField = *pFields++;
		lpGWDHead->YField = *pFields++;
		lpGWDHead->CoordConversion = 0;
		lpGWDHead->NumIndex++;
		rtn = TRUE;
		break;
	}
	GlobalUnlock (hKeyFields);
	GlobalUnlock (hDB);
	CloseGWDatabase (hDB); 
	CheckTimeStamps = TRUE;
	hDB = OpenGWDatabase (Name,BT_WRITE); 
Exit:  
	CloseGWDatabase (hDB);
	CheckTimeStamps =  SaveCheckTimeStamps;
	return rtn;
}

long GetGWDCurrentOffset (LPGWDHEADER lpGWDHead)
{
	HPSTR pLoc = (HPSTR)lpGWDHead + sizeof (GWDHEADER)+lpGWDHead->Reclen;
	HPLONG	pOffset = (HPLONG)pLoc;
	
	return *pOffset;
}

void SetGWDCurrentOffset (LPGWDHEADER lpGWDHead,long Offset)
{
	HPSTR pLoc = (HPSTR)lpGWDHead + sizeof (GWDHEADER)+lpGWDHead->Reclen;
	HPLONG	pOffset = (HPLONG)pLoc;
	
	*pOffset = Offset;
	return;
}

/*BOOL RecoverBadFile (void)
{   HANDLE DBHandle;
    LPGWDHEADER lpGWDHead;
    LPGWFLDINFO lpFieldInfo;
    OFSTRUCTGM    OFStruct;
    char        IndexName[256];
    short       i, j, ii;
    LPSTR       lpEnd;                     
    unsigned    frequency=1000, duration=100; 
    BTHEAD      BTHead;
    LPSTR   lpDot;
	HFILE	FidFrom,FidTo;
    char	Name[MAX_PATH]="E:\\mpls\\Maplib\\Sewer\\storm_work\\storm_manhole_w.gmd";
    char	FromName[MAX_PATH]="G:\\mpls\\Maplib\\Sewer\\storm_work\\storm_manhole_w.gmd";

    FidFrom =  GSSiOpenFile (FromName,&OFStruct,OF_READ);
    FidTo =  GSSiOpenFile (Name,&OFStruct,OF_READWRITE);
    
    BigRead (FidFrom,(HPSTR)&GWDHead16,sizeof(GWDHEADER16));
	BigWrite (FidTo,(HPSTR)&GWDHead16,sizeof(GWDHEADER16),-1);
    DBHandle = GSSiGlobAlloc ( 264,GHND,sizeof (GWDHEADER)+GWDHead16.Reclen+4);
    lpGWDHead =(LPGWDHEADER) GlobalLock (DBHandle);
    *lpGWDHead = GWDHEADER16toGWDHEADER32 (&GWDHead16);
    lpGWDHead->hFldInfo = GSSiGlobAlloc ( 265,GHND,lpGWDHead->NumFields*sizeof(FIELDINFO));
    lpGWDHead->pFldInfo = (LPGWFLDINFO)GlobalLock(lpGWDHead->hFldInfo);   
    for (i=0,lpFieldInfo=lpGWDHead->pFldInfo;i<lpGWDHead->NumFields;i++,lpFieldInfo++)
    {
         BigRead (FidFrom,(HPSTR)lpFieldInfo,sizeof(GWFLDINFO));
		 BigWrite (FidTo,(HPSTR)lpFieldInfo,sizeof(GWFLDINFO),-1);
    }
	GSSiClose (FidFrom);
	GSSiClose (FidTo);

	return TRUE;
}*/

HANDLE OpenGWDatabase (LPSTR InName, short Mode)
#if ENABLETRACE
{GSSiEnterProg (627);
#endif
{   HANDLE DBHandle;
    LPGWDHEADER lpGWDHead;
    LPGWFLDINFO lpFieldInfo;
    OFSTRUCTGM    OFStruct;
    char        IndexName[256];
    short       i, j, ii;
	HFILE		Fid;
    LPSTR       lpEnd;                     
    unsigned    frequency=1000, duration=100; 
    //BTHEAD      BTHead;
    LPSTR   lpDot;
    char	Name[MAX_PATH];
	char	SplitFileName[MAX_PATH];
	UINT	ofMode;

    updateGMDlenMain = 0;
	updateGMDlenIndex = 0;

	strcpy (Name,InName);
	if ((lpDot = strrchr(Name, '.')))
	{
		if (!stricmp(lpDot, ".ORA"))
			strcpy(lpDot, ".GMD");
	}
	else
		goto Return0;
	if (stricmp (lpDot,".gmd") && stricmp (lpDot,".dtm"))
		goto Return0;
Open:
    if (Mode == BT_READ)
		ofMode = OF_READ;
	else
		ofMode = OF_READWRITE;
    Fid =  GSSiOpenFile (Name,&OFStruct,ofMode);
    if (Fid == HFILE_ERROR)
    {   
{
#if ENABLETRACE
GSSiExitProg (627);
#endif
         return (0);
}
    }
    BigRead (Fid,(HPSTR)&GWDHead16,sizeof(GWDHEADER16));
	if (GWDHead16.Unused)
	{
		int	Size;

		GSSillseek (Fid,0,0);
		DBHandle = GSSiGlobAlloc ( 264,GHND,sizeof (GWDHEADER)+4);
		lpGWDHead =(LPGWDHEADER) GlobalLock (DBHandle);
	    BigRead (Fid,(HPSTR)lpGWDHead,sizeof(GWDHEADER));
		Size = sizeof (GWDHEADER)+ 4 + lpGWDHead->Reclen;
		GlobalUnlock (DBHandle);
		DBHandle = GSSiGlobalReAlloc (0,DBHandle,Size,GMEM_MOVEABLE);
		lpGWDHead =(LPGWDHEADER) GlobalLock (DBHandle);
		if (lpGWDHead->SplitFile) // splitfiles always have the gsf file one directory level up from the gmd and indexes - allows updates without having to resend the .gsf as long as same lenth as previous.
		{
			if (lpGWDHead->SplitLength > 0)
			{
				char drive[32], dir[MAX_PATH], fnam[MAX_PATH];
				LPSTR lpBS;
				BOOL firstTry = TRUE;
				OFSTRUCTGM OFStruct;
				long lenSplitFile;

				strcpy(SplitFileName, Name);
				ExpandText(SplitFileName);
				_splitpath(SplitFileName, drive, dir, fnam, 0);
				if (*LastChr(dir) == '\\')
					*LastChr(dir) = 0;
				lpBS = strrchr(dir, '\\');
				if (lpBS)
					*lpBS = 0;
				sprintf(SplitFileName, "%s%s\\%s_%i.gsf",drive,dir,fnam,lpGWDHead->SplitLengthRequested);
TryAgain:
				lpGWDHead->SplitFid = GSSiOpenFile(SplitFileName, &OFStruct, OF_READ);
				lenSplitFile = GSSifilelength(lpGWDHead->SplitFid);
				if (lpGWDHead->SplitFid == HFILE_ERROR || lenSplitFile != lpGWDHead->SplitLength - (sizeof(GWDHEADER)+lpGWDHead->NumFields*sizeof(GWFLDINFO)))
				{
					char mess[300];
					//might have been interupted during transfer. Close and retry.
					if (firstTry && lpGWDHead->SplitFid != HFILE_ERROR)
					{
						firstTry = FALSE;
						GSSiClose(lpGWDHead->SplitFid);
						GSSiRemove(OFStruct.szPathName);
						goto TryAgain;
					}
					GSSiClose(lpGWDHead->SplitFid);
					GSSiClose(Fid);
					sprintf(mess, "Missing or invalid split file: %s", SplitFileName);
					MessageBox(0, mess, 0, MB_ICONEXCLAMATION);
					GSSiGlobUlFree(&DBHandle);
					goto Return0;
				}
			}
			else
				lpGWDHead->SplitFile = lpGWDHead->SplitLength = 0;
		}
		else
			lpGWDHead->SplitFid = HFILE_ERROR;
	}
	else
	{
		HANDLE DBHandle32 = GSSiGlobAlloc ( 264,GHND,sizeof (GWDHEADER32));
		LPGWDHEADER32 lpGWDHead32 =(LPGWDHEADER32) GlobalLock (DBHandle32);

		DBHandle = GSSiGlobAlloc ( 264,GHND,sizeof (GWDHEADER)+GWDHead16.Reclen+4);
		lpGWDHead =(LPGWDHEADER) GlobalLock (DBHandle);
		*lpGWDHead32 = GWDHEADER16toGWDHEADER32 (&GWDHead16);
		*lpGWDHead = GWDHEADER32toGWDHEADER (lpGWDHead32);
		GSSiGlobUlFree (&DBHandle32);
	}
    lpGWDHead->Fid =  Fid; 
    SetGWDCurrentOffset (lpGWDHead,-1);
	if (lpGWDHead->NumFields <= 0)
	{
		GSSiClose(Fid);
		goto Return0;
	}
    lpGWDHead->hFldInfo = GSSiGlobAlloc ( 265,GHND,lpGWDHead->NumFields*sizeof(FIELDINFO));
    lpGWDHead->pFldInfo = (LPGWFLDINFO)GlobalLock(lpGWDHead->hFldInfo);   
    if (lpGWDHead->StoredAs32)
    	ii=1;
    if (!lpGWDHead->Version)
    {
        lpGWDHead->TimeStamp = 0; 
        ii=GSSillseek (Fid,sizeof (GWDHEADER)-(MAX_GMD_INDEX_FIELDS*sizeof(HANDLE)+
                                            MAX_GMD_INDEX_FIELDS*sizeof(LPSTR)+
                                            MAX_GMD_INDEX_FIELDS*sizeof(short)+
                                            sizeof(time_t)+
                                            sizeof(HANDLE)+
                                            sizeof(LPGWFLDINFO)+2),
                                            0);
    }
    for (i=0,lpFieldInfo=lpGWDHead->pFldInfo;i<lpGWDHead->NumFields;i++,lpFieldInfo++)
    {
         BigRead (Fid,(HPSTR)lpFieldInfo,sizeof(GWFLDINFO));
         lpEnd = LastNonBlank(lpFieldInfo->Name);
         lpEnd++;
         *lpEnd='\0';
    }
	if (lpGWDHead->SpatialIndex)
	{
		lpGWDHead->GridXInc = (lpGWDHead->GridBounds.xmx - lpGWDHead->GridBounds.xmn) / lpGWDHead->GridInc;
		lpGWDHead->GridYInc = (lpGWDHead->GridBounds.ymx - lpGWDHead->GridBounds.ymn) / lpGWDHead->GridInc;
	}
    for (i=0;i<MAX_GMD_INDEXES;i++) lpGWDHead->BTHandle[i]=0;
    for (i=0;i<lpGWDHead->NumIndex;i++)
    {
        _fstrcpy (IndexName,Name);
        ExpandText (IndexName);
        _fstrlwr (IndexName); 
        lpDot = _fstrrchr (IndexName,'.');
        if (!lpDot) 
        {   
        	for (j=0;j<i;j++)
        		BT_CLOSE (lpGWDHead->BTHandle[j]);
            GlobalUnlock (DBHandle);  
			goto Return0;
		}
        _fstrcpy (lpDot,".in");
        itoa (i+1,_fstrchr(IndexName,'\0'),10);
        if (!(lpGWDHead->BTHandle[i] = BT_OPEN (IndexName, lpGWDHead->TimeStamp, Mode, 0))) 
        {   
		    if (Mode == BT_READ) 
		    {   
		    	long	loc = GSSillseek (Fid,0,1);
		    	
		    	GSSiClose (Fid);
		        Fid =  GSSiOpenFile (Name,&OFStruct,OF_READWRITE);
		        GSSillseek (Fid,loc,0);
		        Mode = BT_WRITE;
		    }
            if (!CreateGWDIndex (DBHandle,IndexName,i))
            {   
	            GlobalUnlock (DBHandle);  
			    CloseGWDatabase (DBHandle);
				goto Return0;
			}
            else
            {
	            GlobalUnlock (DBHandle);  
			    CloseGWDatabase (DBHandle);
                goto Open;      
            }
        }
        if (CheckTimeStamps && !BT_CHECK_TIME_STAMP (lpGWDHead->BTHandle[i] ,lpGWDHead->TimeStamp))
        {   
        	BT_CLOSE (lpGWDHead->BTHandle[i]);
            if (!CreateGWDIndex (DBHandle,IndexName,i)) 
            {   
	            GlobalUnlock (DBHandle);  
			    CloseGWDatabase (DBHandle);
				goto Return0;
			}
            else 
            {
	            GlobalUnlock (DBHandle);  
//				lpGWDHead->BTHandle[i] = BT_OPEN (IndexName, lpGWDHead->TimeStamp, Mode, 0);
			    CloseGWDatabase (DBHandle);
                goto Open;      
            }
        }
        //GetBTHeader (lpGWDHead->BTHandle[i],&BTHead); 
        if (!lpGWDHead->lKeys[i] || abs (lpGWDHead->lKeys[i]>256))
            lpGWDHead->lKeys[i] = ComputeGWDKeyLen(lpGWDHead,i);
        lpGWDHead->hKeys[i]=GSSiGlobAlloc ( 266,GHND,abs(lpGWDHead->lKeys[i])+2);
        lpGWDHead->pKeys[i]=GlobalLock(lpGWDHead->hKeys[i]);
        if (!lpGWDHead->pKeys[i])
            ii=0;
    }
         

    GlobalUnlock (DBHandle);
{
#if ENABLETRACE
GSSiExitProg (627);
#endif
    return (DBHandle);
}
Return0:
{
#if ENABLETRACE
	GSSiExitProg(627);
#endif
	return 0;
}

#if ENABLETRACE
}
#endif
}

void CloseGWDatabase (HANDLE DBHandle)
#if ENABLETRACE
{GSSiEnterProg (629);
#endif
{
    LPGWDHEADER lpGWDHead; 
    //BTHEAD  BTHead;
    
    short         i,ii;
    
    if (!DBHandle)
{
#if ENABLETRACE
GSSiExitProg (629);
#endif
    	return;
}
    lpGWDHead = (LPGWDHEADER) GlobalLock (DBHandle);
    //GetBTHeader (lpGWDHead->BTHandle[0], &BTHead);
    if (lpGWDHead->Version && (FileOpenForWrite (lpGWDHead->Fid) || BT_OPEN_FOR_WRITE (lpGWDHead->BTHandle[0])))
    {
        lpGWDHead->TimeStamp = time(0); 
        GSSillseek (lpGWDHead->Fid,0,0);
		if (lpGWDHead->StoredAs32)
	        BigWrite (lpGWDHead->Fid,(HPSTR)lpGWDHead ,sizeof(GWDHEADER),-1);
		else
		{
			GWDHead16 = GWDHEADER32toGWDHEADER16 (lpGWDHead);
			BigWrite (lpGWDHead->Fid,(HPSTR)&GWDHead16 ,sizeof(GWDHEADER16),-1);
		}
    }
    GSSiClose (lpGWDHead->Fid);
	GSSiClose (lpGWDHead->SplitFid);

    for (i=0;i<lpGWDHead->NumIndex;i++)
    {   
        if (lpGWDHead->BTHandle[i])
        {
            BT_SET_TIME_STAMP (lpGWDHead->BTHandle[i], lpGWDHead->TimeStamp);
            BT_CLOSE (lpGWDHead->BTHandle[i]);
            GSSiGlobUlFree (&lpGWDHead->hKeys[i]);
        }
    }
    GSSiGlobUlFree (&lpGWDHead->hFldInfo);
    GSSiGlobUlFree (&DBHandle);
{
#if ENABLETRACE
GSSiExitProg (629);
#endif
    return;
}

#if ENABLETRACE
}
#endif
}

BOOL BasicDataDisplayToDC(LPSTR DBNameIN, HDC hDC, long RecNum, long iref, LPSTR pSQL, int maxline,int FontSize,int maxFontSize, RECT rect, LPSTR title)
#if ENABLETRACE
{
	GSSiEnterProg(630);
#endif
	{
		HANDLE      hSQL;
		double      rtn;
		char        str[300], DBName[MAX_PATH];
		short       ifield, l;
		LPOPENSQLDATA   SQLPtr;
		LPFIELDINFO lpFieldInfo;
		HANDLE      SaveHandle;
		LPOPENFILEDATA  FilePtr;
		LPSTR		str2;
		HANDLE		hStr = 0;
		long SaveSHPRec = CurrentSHPRec;
		HFONT hFont, hFontBold, hOldFont;
		double fontFactor = 1;
		int  Tabs[2] = { RECTWIDTH(&rect) / 4.5, RECTWIDTH(&rect) / 2 };
		int margin = 5.0/100.0 * RECTWIDTH (&rect);
		int lineInc = margin;
		int lineHeight = 16;
		int nRows;

		strcpy(DBName, DBNameIN);
		rtn = FALSE;
		hSQL = 0;
		if (!OpenDataFile(DBName, pSQL, BT_READ, &hSQL))
		{
#if ENABLETRACE
			GSSiExitProg(630);
#endif
			return FALSE;
		}
		CurrentSHPRec = SaveSHPRec;
		SQLPtr = (LPOPENSQLDATA)GlobalLock(hSQL);
		FilePtr = (LPOPENFILEDATA)GlobalLock(SQLPtr->OFHandle);
		lpFieldInfo = &FilePtr->FldInfo;
		fontFactor = (double)RECTHEIGHT (&rect)/(FilePtr->NumFields * FontSize*1.2 + margin*2);
		hFontBold = CreateFont((int)IDNINT(FontSize*fontFactor*1.2), 0, 0, 0, FW_BLACK, 0, 0, 0, 0, 0, 0, 0, 0, "Courier New");
		hFont = CreateFont((int)IDNINT(FontSize*fontFactor), 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 0, 0, "Courier New");
		lineHeight = IDNINT(FontSize*fontFactor*1.2);
		hStr = GSSiGlobAlloc(270, GMEM_MOVEABLE, 4096);
		str2 = GlobalLock(hStr);
		for (ifield = 0; ifield<FilePtr->NumFields; ifield++, lpFieldInfo++)
		{
			BOOL    First = TRUE;

		Display:
			sprintf(str, "%s\t", lpFieldInfo->name);
			if ((l = GetValFromOpenFiles(lpFieldInfo->name, str2, 4096))<0)
			{
				char str[] = "Data record not found";
				TextOut(hDC, 0,0,str , _fstrlen(str));

				goto NotFound;
			}
			else if (First && RecNum>1 && !ifield)
			{
				long    irec;

				for (irec = 1; irec<RecNum; irec++)
					FetchDBRec(hSQL);
				First = FALSE;
				goto Display;
			}
			else
			{
				LPSTR	str3 = str2;
				char	c;

				if (strstr(lpFieldInfo->name, "Slope"))
				{
					double slope = atof(str2);
					sprintf(str2, "%.1f", fabs(slope));
				}
				l = max(l, 1);
				while (l > 0)
				{
					LPSTR	pEnd = _fstrchr(str, 0), pCR;
					short	n = min(l, maxline);
					short	orign = n;

					_fstrncpy(pEnd, str3, n);
					if (n < l)
					{
						while (n && pEnd[n - 1] != ' ')
							n--;
						if (!n)
							n = orign;
					}
					pEnd[n] = 0;
					if ((pCR = _fstrchr(pEnd, '\r')))
					{
						*pCR = 0;
						n = _fstrlen(pEnd) + 2;
					}
					l -= n;
					str3 += n;
					{
						LPSTR pTab = strchr(str, '\t');
						if (pTab)
						{
							*pTab++ = 0;
							if (strlen(str) > 0)
							{
								hOldFont = SelectObject(hDC, hFontBold);
								TabbedTextOut(hDC, rect.left + margin, rect.top + lineInc, str, strlen(str), 1, Tabs, rect.left);
							}
							hOldFont = SelectObject(hDC, hFont);
							TabbedTextOut(hDC, Tabs[0], rect.top + lineInc, pTab, strlen(pTab), 1, Tabs, rect.left);
						}
						else
						{
							hOldFont = SelectObject(hDC, hFont);
						}
						SelectObject(hDC, hOldFont);
						_fstrcpy(str, "\t");
						lineInc += lineHeight;
					}
				}
			}
			First = FALSE;
		}
		rtn = TRUE;
	NotFound:
		GlobalUnlock(SQLPtr->OFHandle);
		GlobalUnlock(hSQL);
		CloseDataFile(TRUE, &hSQL);
		DeleteObject(hFont);
		DeleteObject(hFontBold);
		GSSiGlobUlFree(&hStr);
		{
#if ENABLETRACE
			GSSiExitProg(630);
#endif
			return (int)(rtn);
		}
#if ENABLETRACE
	}
#endif
}

char canEditField(LPSTR FieldName)
{
	char rtn = ' ';
	if (GetUpdateFieldType(FieldName))
		rtn = '*';
	return rtn;
}
BOOL BasicDataDisplay (LPSTR DBNameIN,HWND hWndDlg,short dlgitem,short nextbutton,short priorbutton,long RecNum, long iref, LPSTR pSQL,int maxline)
#if ENABLETRACE
{GSSiEnterProg (630);
#endif
{  
    HANDLE      hSQL;  
    double      rtn;
    char        str[300],DBName[MAX_PATH];
    short       ifield,l;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    HANDLE      SaveHandle;
    LPOPENFILEDATA  FilePtr;  
    LPSTR		str2;
    HANDLE		hStr=0;
	long SaveSHPRec = CurrentSHPRec; 
	
	strcpy (DBName,DBNameIN);
    rtn = FALSE;
    SendDlgItemMessage (hWndDlg,dlgitem,LB_RESETCONTENT,0,0); 
    hSQL=0;
    if (!OpenDataFile (DBName,pSQL,BT_READ,&hSQL))
{
#if ENABLETRACE
GSSiExitProg (630);
#endif
        return FALSE;  
}
	CurrentSHPRec = SaveSHPRec;
    SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
    FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
    lpFieldInfo = &FilePtr->FldInfo; 
    hStr = GSSiGlobAlloc ( 270,GMEM_MOVEABLE,4096); 
    str2 = GlobalLock (hStr);
    for (ifield=0;ifield<FilePtr->NumFields;ifield++,lpFieldInfo++)
    {   
        BOOL    First=TRUE;

Display:        
        sprintf (str,"%s\t %c\t",lpFieldInfo->name,canEditField(lpFieldInfo->name));
        if ((l=GetValFromOpenFiles (lpFieldInfo->name,str2,4096))<0) 
        {
            SetDlgItemText(hWndDlg,IDENTIFY_LINE2,"Data record not found");
            goto NotFound;
        }
        else if (First && RecNum>1 && !ifield) 
        {   
            long    irec;
            
            for (irec=1;irec<RecNum;irec++) 
                FetchDBRec (hSQL);
            First=FALSE;
            goto Display;
        }
        else
        {   
        	LPSTR	str3=str2;
        	char	c; 
        	
        	l = max (l,1);
        	while (l > 0)
        	{   
        		LPSTR	pEnd = _fstrchr(str,0), pCR;
        		short	n=min(l,maxline); 
        		short	orign=n;
        		
	            _fstrncpy(pEnd,str3,n);  
	            if (n < l)
	            {
	            	while (n && pEnd[n-1] != ' ')
		            	n--;  
		            if (!n)
		            	n=orign;
		        }
	            pEnd[n] = 0;   
	            if ((pCR = _fstrchr (pEnd,'\r')))
	            {
	            	*pCR = 0;
	            	n = _fstrlen (pEnd)+2;
	            }
	            l-=n;   
	            str3+=n;
	            SendDlgItemMessage (hWndDlg,dlgitem,LB_ADDSTRING,0,(LPARAM)str); 
	            _fstrcpy (str,"\t");
	        }
        }   
        First=FALSE;
    }
    if (nextbutton)
    {   
        BOOL    More=FetchDBRec (hSQL);
        
        EnableWindow (GetDlgItem(hWndDlg,nextbutton),More);
        EnableWindow (GetDlgItem(hWndDlg,priorbutton),RecNum>1);
        
    }
    rtn = TRUE;
NotFound:
    GlobalUnlock (SQLPtr->OFHandle);
    GlobalUnlock(hSQL); 
    CloseDataFile (TRUE, &hSQL); 
    GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (630);
#endif
    return (int)(rtn);
}
#if ENABLETRACE
}
#endif
} 

BOOL GMDGetNumericKeyVal (LPGWDHEADER lpGWDHead,int Index, short IndexField,double *rtn)
#if ENABLETRACE
{GSSiEnterProg (631);
#endif
{   LPSTR   lpVal;
    LPGWFLDINFO lpGWFldInfo;
    char        str[64];
    short         i;
    LPSTR       ep;  
    BOOL        Status;
    
    Status = TRUE;
    lpVal = lpGWDHead->pKeys[Index];
    for (i=0;i<IndexField;i++)
    {
        lpGWFldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[Index][i];
        lpVal += lpGWFldInfo->Len;
    }
    lpGWFldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[Index][IndexField];
    
    switch (lpGWFldInfo->Type)
    {   
    default:     
    case BT_RIGHT_CHAR:
    case BT_CHAR:
        _fstrncpy (str,lpVal,lpGWFldInfo->Len);
        str[lpGWFldInfo->Len]='\0';
        ep = &str[lpGWFldInfo->Len];
        *ep ='\0';
        *rtn = strtod (str,&ep);
        while (*ep)
            if (*ep++ != ' ')
                Status=FALSE;
    break;
                                            
    case BT_INTEGER:
        if (lpGWFldInfo->Len == 2)
            *rtn = *(LPSHORT)lpVal;
        else
            *rtn = *(LPLONG)lpVal;
    break;
                                            
    case BT_REAL:
        if (lpGWFldInfo->Len == 4)
            *rtn = *(LPFLOAT)lpVal;
        else
            *rtn = *(LPDOUBLE)lpVal;
    break;
    }
{
#if ENABLETRACE
GSSiExitProg (631);
#endif
    return Status; 
}
#if ENABLETRACE
}
#endif
}

BOOL GMDGetCharKeyVal (LPGWDHEADER lpGWDHead,int Index, short IndexField,LPSTR str)
#if ENABLETRACE
{GSSiEnterProg (632);
#endif
{   LPSTR   lpVal;
    LPGWFLDINFO lpGWFldInfo;
    short         i;
    BOOL        Status;
    
    Status = TRUE;
    lpVal = lpGWDHead->pKeys[Index];
    for (i=0;i<IndexField;i++)
    {
        lpGWFldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[Index][i];
        lpVal += lpGWFldInfo->Len;
    }
    lpGWFldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[Index][IndexField];
    
    switch (lpGWFldInfo->Type)
    {   
    default:   
    case BT_RIGHT_CHAR:
    case BT_CHAR:
        _fstrncpy (str,lpVal,lpGWFldInfo->Len);
        str+=lpGWFldInfo->Len;
        *str = '\0';
    break;
                                            
    case BT_INTEGER:
        if (lpGWFldInfo->Len == 2)
            itoa (*(LPSHORT)lpVal,str,10);
        else
            ltoa (*(LPLONG)lpVal,str,10);
    break;
                                            
    case BT_REAL:
        if (lpGWFldInfo->Len == 4)
            sprintf(str,"%f",*(LPFLOAT)lpVal);
        else
            sprintf(str,"%f",*(LPDOUBLE)lpVal);
    break;
    }
{
#if ENABLETRACE
GSSiExitProg (632);
#endif
    return Status; 
}
#if ENABLETRACE
}
#endif
}

double GetNumericFieldData (HANDLE hSQL,LPFIELDINFO lpField, int FunctionID,int MultiValOption,LPSTR CmdString,long iref, int ElemType,LPSHORT irc,LPSTR DataFileID)
#if ENABLETRACE
{GSSiEnterProg (633);
#endif
{   
    double  ValD=0;
    LPGWDHEADER lpGWDHead;
    LPGWFLDINFO lpGWFldInfo;
    HANDLE      hBT, hDB;
    long        Offset; 
    BOOL		HaveData=FALSE;
    double      rtn, Sum=0, MinMax=0, Minv=0, Maxv=0;                  
    HANDLE		hMem = GSSiGlobAlloc ( 271,GMEM_MOVEABLE,4096);
    LPSTR		str=GlobalLock (hMem);
    short         st, i, len;
    LPSTR       ep, ValC; 
    LPVOID      lpVal;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPSQLFIELD  lpSQLField;
	int			FileType, n=0;
    
	*irc = -1;
	if (!hSQL)
		goto Exit;
    SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
    FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
    hDB = FilePtr->FileHandle;
	FileType = FilePtr->Type;
   	GlobalUnlock (SQLPtr->OFHandle); 
   	GlobalUnlock (hSQL);  
    if (FileType == DTM_DATAFILE)
    {   
    	DPOINT	Point1,Point2;
    	double	Elev1, Elev2, dist;  
    	short	ii;
    	
    	ValD = 0;
    	switch (ElemType)
    	{
    		case GF_CURVE: 
    			ii=1;
    		case GF_LINE:
    		case GF_POLYLINE: 
    		{
    			short	ElevUnits=PRJ_UNITS[1]-1;
    			
				Point1 = lpDCurPoints[0];
				Point2 = lpDCurPoints[nPnts-1]; 
				dist = GetPolyLengthD (lpDCurPoints,nPnts);
				if (dist)
				{
    				Elev1 = NGIELV (Point1,hDB,ElevUnits); 
    				Elev2 = NGIELV (Point2,hDB,ElevUnits); 
    				if (Elev1 < DBL_MAX && Elev2 < DBL_MAX)
    				{
    					ValD = 100.0 * fabs(Elev1 - Elev2)/dist;
    					*irc = 0;
    				}
    			}
    		} 
    		break;
    		
    		case GF_POINT:
    		case GF_TEXT:
    			ValD = NGIELV (CurrentPoint,hDB,0); 
		    	if (ValD < DBL_MAX)
		    		*irc = 0; 
		    break;
		    
		    default:
		    break;
		}
        GSSiGlobUlFree (&hMem); 
{
#if ENABLETRACE
GSSiExitProg (633);
#endif
        return (ValD);
}
	}
   	*irc = 0;
    if (FunctionID == 1)
    	_fstrcpy (str,CmdString);
	else
    	sprintf (str,"[%s]",lpField->name);
	switch (MultiValOption)
	{
	case 4:
		MinMax = DBL_MAX;
		break;
	case 5:
		MinMax = -DBL_MAX;
		break;
	case 6:
		Minv = DBL_MAX;
		Maxv = -DBL_MAX;
		break;
	}
	while (FetchDBRec (hSQL))
	{
		n++;
    	ExpandText (str);
		if (MultiValOption != 1)
        {
            ValD = strtod (str,&ep);
			if (*ep == ';')
				ep++;
            while (*ep)
                if (*ep++ != ' ')
				{
                    *irc = -1;
					goto Exit;
				}
			switch (MultiValOption)
			{
			case 0:
			case 7://first class
			case 8://last class
				goto ExitLoop;
			case 1:
				break;
			case 2:
			case 3:
				Sum += ValD;
				break;
			case 4:
				MinMax = min (ValD,MinMax);
				break;
			case 5:
				MinMax = max (ValD,MinMax);
				break;
			case 6:
				Minv = min (ValD,Minv);
				Maxv = max (ValD,Maxv);
				break;
			}
        }
		if (FunctionID == 1)
    		_fstrcpy (str,CmdString);
		else
    		sprintf (str,"[%s]",lpField->name);
	}
ExitLoop:
	if (!n && MultiValOption != 1)
		*irc = 1;                 
	else switch (MultiValOption)
	{
	case 1:
		ValD = n;
		break;
	case 2:
		Sum /= n;
	case 3:
		ValD = Sum;
		break;
	case 4:
	case 5:
		ValD = MinMax;
		break;
	case 6:
		ValD = Maxv - Minv;
		break;
	}
Exit:
    GSSiGlobUlFree (&hMem); 
{
#if ENABLETRACE
GSSiExitProg (633);
#endif
        return (ValD);
}
/*    else if (FilePtr->Type == COMBO_DATAFILE) 
    {   
    	GlobalUnlock (SQLPtr->OFHandle); 
    	GlobalUnlock (hSQL);
    	sprintf (str,"[%s]",lpField->name);
    	goto UseExpand;
    }
    else if (FilePtr->Type == GMTEXT_DATAFILE) 
    {   
    	GlobalUnlock (SQLPtr->OFHandle); 
    	GlobalUnlock (hSQL);
    	sprintf (str,"[%s]",lpField->name);
    	goto UseExpand;
    }
	else if (FilePtr->Type == SHAPE_DATAFILE)
	{   
		long	LongVal;
		double	DoubleVal; 
		LPCSTR	pStringVal;
		DBFHandle    pDBF;
		LPDWORD 	 pDBFAddress = (LPDWORD)GlobalLock (FilePtr->FileHandle);
					
		pDBF = (DBFHandle)*pDBFAddress; 
		GlobalUnlock (FilePtr->FileHandle);   
    	GlobalUnlock (SQLPtr->OFHandle); 
		GlobalUnlock (hSQL);  
		if (CurrentSHPRec < pDBF->nRecords)
			*irc = 0;
		switch (lpField->type)
		{   
			default:   
			case BT_RIGHT_CHAR:
			case BT_CHAR:
				pStringVal = DBFReadStringAttribute(pDBF,CurrentSHPRec, lpField->index);  
	        	ValD = atof (pStringVal);
        	break;
							        	
        	case BT_INTEGER:
				LongVal = DBFReadIntegerAttribute(pDBF,CurrentSHPRec, lpField->index); 
				ValD = LongVal;
	     	break;
						 			
 			case BT_REAL:
				ValD = DBFReadDoubleAttribute(pDBF,CurrentSHPRec, lpField->index);
 			break;
    	} 
        GSSiGlobUlFree (&hMem); 
{
#if ENABLETRACE
GSSiExitProg (633);
#endif
        return (ValD);
}
	}
    else if (FilePtr->Type == UMIFS_DATAFILE || FilePtr->Type == ORA_DATAFILE || FilePtr->Type == GMCENSUS_DATAFILE)
    {   
    	short	FirstCond = BT_EQ; 

    	GlobalUnlock (SQLPtr->OFHandle); 
        lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
        if (lpGWDHead->Version > 1000)
        {   
        	if (!LoadInternalGMD (lpGWDHead,iref))
        		goto NotFound;
            st = 0;  
            HaveData=TRUE;
           	goto Found;
        }
        hBT = lpGWDHead->BTHandle[SQLPtr->IndexToUse];
        if (!hBT) goto NotFound;
        
        GWDClearSetValues (lpGWDHead);
        if (SQLPtr->NumGlobals)
        {
            for (i=0,lpSQLField=&SQLPtr->SQLField;i<SQLPtr->NumGlobals;i++,lpSQLField++) 
            {   
		    	if (lpSQLField->FieldNum >= 0 && lpSQLField->OpCode == OPCODE_EQ)
		    	{
	            	_fstrcpy (str,lpSQLField->String);
	            	ExpandText (str);
	                lpGWFldInfo=lpGWDHead->pFldInfo + lpSQLField->FieldNum;
	                SetFieldValFromChar(lpGWDHead,lpGWFldInfo,str,FALSE,FALSE); 
	            } 
            }
			if (GWDInitUnsetKeyValues (lpGWDHead,SQLPtr->IndexToUse))
				FirstCond = BT_GE;
            GWDFormKey(lpGWDHead,SQLPtr->IndexToUse,TRUE,0); 
            if (SQLPtr->IndexToUse) 
                SetReadSecIndex(TRUE);
            st = BT_FIND (hBT,lpGWDHead->pKeys[SQLPtr->IndexToUse],BT_FIRST,FirstCond, (LPSTR)&SQLPtr->Offset);
            SetReadSecIndex(FALSE);
        }
        else
            st = BT_FIND (hBT,(LPSTR)&iref,BT_FIRST,BT_EQ, (LPSTR)&Offset); 
Found:
        if (!st)
        {   
        	if (!HaveData)
	        	FillGWDData (lpGWDHead,SQLPtr->Offset);
            for (i=0,lpGWFldInfo=lpGWDHead->pFldInfo;i<lpGWDHead->NumFields;i++,lpGWFldInfo++)
            {   if (!_fstrcmp (lpGWFldInfo->Name,lpField->name))
                {
                    *irc = 0;
                    lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];
                    switch (lpGWFldInfo->Type)
                    {   
                        default:
                        case BT_RIGHT_CHAR:
                        case BT_CHAR:
                            _fstrncpy (str,lpVal,lpGWFldInfo->Len);
                            str[lpGWFldInfo->Len]='\0';
                            ep = &str[lpGWFldInfo->Len];
                            *ep ='\0';
                            errno = 0;
                            rtn = strtod (str,&ep);
                            while (*ep)
                                if (*ep++ != ' ')
                                    *irc = -1;
                        break;
                                        
                        case BT_INTEGER:
                            if (lpGWFldInfo->Len == 2)
                                rtn = *(LPSHORT)lpVal;
                            else
                                rtn = *(LPLONG)lpVal;
                        break;
                                        
                        case BT_REAL:
                            if (lpGWFldInfo->Len == 4)
                                rtn = *(LPFLOAT)lpVal;
                            else
                                rtn = *(LPDOUBLE)lpVal;
                        break;
                     } 
					 GlobalUnlock (hSQL);
                     GlobalUnlock (hDB);  
                     GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (633);
#endif
                     return (rtn);
}
                }
            }
            goto NotFound;
        }
        else
NotFound:	*irc = 1;
			GlobalUnlock (hSQL);
            GlobalUnlock (hDB);
            GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (633);
#endif
            return (0);
}
    }
    else
    {
        ValC = (LPSTR) GetExternalFieldData (FilePtr ,SQLPtr->SQL, &SQLPtr->hstmt,
                                             lpField, TRUE,FunctionID, irc,0,0);
    	GlobalUnlock (SQLPtr->OFHandle); 
		GlobalUnlock (hSQL);
        if (!*irc)
        {
            ValD = strtod (ValC,&ep);
            while (*ep)
                if (*ep++ != ' ')
                    *irc = -1;
        }
        GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (633);
#endif
        return (ValD);
}
    }*/
#if ENABLETRACE
}
#endif
}

int GetCharFieldData (HANDLE hSQL,LPFIELDINFO lpField, long iref, short FunctionID,LPSTR CmdString,LPSTR Value,int maxLen,LPSTR DataFileID,int combineOption)
#if ENABLETRACE
{GSSiEnterProg (634);
#endif
{   
	HANDLE		hMem = GSSiGlobAlloc(272, GMEM_MOVEABLE, 4096);
	LPSTR		str = GlobalLock(hMem);
    short       irc=0;
	int			n=0;
	HANDLE		hCombined=0;
	LPSTR		pCombined;
#define MAX_COMBINED_VALUES	16
#define MAX_COMBINED_ELEMENT_LENGTH 32
	int			nValues[MAX_COMBINED_VALUES];
	int i,j;

	*Value = 0;
	if (combineOption == 4)
	{
		memset (nValues,0,sizeof(nValues));
		hCombined = GSSiGlobAlloc (1782,GHND,MAX_COMBINED_VALUES*(MAX_COMBINED_ELEMENT_LENGTH+1));
	}
	while (FetchDBRec (hSQL))
	{
		LPSTR	pEnd;

    	irc = 0;
		if (FunctionID == 1)
    		_fstrcpy (str,CmdString);
		else
    		sprintf (str,"[%s.%s]",DataFileID,lpField->name);
		ExpandText (str);
		pEnd = strchr (str,0);
		if (pEnd-- > str)
		{
			if (*pEnd == ';')
			{
				*pEnd-- = 0;
				if (pEnd > str)
				{
					if (*pEnd == '@')
						*pEnd = ';';
				}
			}
		}
		if (!n)
		{
			strncpy0 (Value,str,maxLen-1);
			if (hCombined)
			{
				pCombined = GlobalLock (hCombined);
				strncpy (pCombined,str,MAX_COMBINED_ELEMENT_LENGTH);
				GlobalUnlock (hCombined);
			}
			nValues[0] = 1;
			n = 1;
		}
		else if (n < MAX_COMBINED_VALUES)
		{
			pCombined = GlobalLock (hCombined);
			for (i=0;i<n;i++)
			{
				int icmp = strcmp (pCombined+i*(MAX_COMBINED_ELEMENT_LENGTH+1),str);
				
				if (icmp < 0)
				{
					ii=1;
				}
				else if (icmp == 0)
				{
					nValues[i]++;
					goto inserted;
				}
				else
				{
					for (j=n;j>i;j--)
					{
						strncpy (pCombined+(j)*(MAX_COMBINED_ELEMENT_LENGTH+1),
								 pCombined+(j-1)*(MAX_COMBINED_ELEMENT_LENGTH+1),
								 MAX_COMBINED_ELEMENT_LENGTH);
						nValues[j] = nValues[j-1];
					}
					strncpy (pCombined+i*(MAX_COMBINED_ELEMENT_LENGTH+1),
						     str,MAX_COMBINED_ELEMENT_LENGTH);
					nValues[i] = 1;
					n++;
					goto inserted;
				}
			}
			strncpy (pCombined+n*(MAX_COMBINED_ELEMENT_LENGTH+1),str,MAX_COMBINED_ELEMENT_LENGTH);
			nValues[n++] = 1;
inserted:
			GlobalUnlock (hCombined);
		}
		else
		{
			pCombined = GlobalLock (hCombined);
			strcpy (pCombined,"Too many combined values");
			GlobalUnlock (hCombined);
			n = 1;
			break;
		}
		if (combineOption < 4)
			break;
	}
	if (!n)
    	irc = 1;
	if (hCombined)
	{
		pCombined = GlobalLock (hCombined);
		if (n)
		{
			sprintf (str,"%s(%i)",pCombined,nValues[0]);
			for (i=1;i<n;i++)
			{
				sprintf (strchr(str,0),"-%s(%i)",pCombined+i*(MAX_COMBINED_ELEMENT_LENGTH+1),nValues[i]);
			}
		}
		GSSiGlobUlFree (&hCombined);
		strncpy0(Value, str, maxLen - 1);
	}
	GSSiGlobUlFree(&hMem);
{
#if ENABLETRACE
GSSiExitProg (634);
#endif
    return irc;
}

#if ENABLETRACE
}
#endif
} 

void GMDUpdateConvertCommas(LPSTR Args)
{
//converts commas in GMDUpdate update values
	if (convertGMDUpdateCommas)
	{
		LPSTR pComma = MatchLev(Args, ',');

		if (pComma)
		{
			pComma = MatchLev(pComma + 1, ',');
			{
				pComma++;
				while ((pComma = MatchLev(pComma, ',')))
				{
					*pComma = '-';
				}
			}
		}
	}
	return;
}

int UpdateGMDFile (LPSTR InFile,LPSTR KeyString,LPSTR UpdateStringIN,char Separator,BOOL Truncate,BOOL updateOnly)
#if ENABLETRACE
{GSSiEnterProg (635);
#endif
{
	LPGWDHEADER lpGWDHead;
	HANDLE	hDBDest; 
	long	Offset;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
    LPFILEPATH  FilePathPtr; 
	LPHANDLE	lpFileHandle; 
	LPSTR	Name,Value,pComma, pEQ;     
	BOOL	DoClose = TRUE; 
	short	isql;
	char	File[MAX_PATH];
	HANDLE	hUpdateString=0;
	LPSTR	UpdateString;
	int		lUp;
	char	OffsetList[MAX_PATH];
	HFILE	FidOffsetList=HFILE_ERROR;
	BOOL	HaveKey=FALSE;
	int		rtn=0;
	int		indexToUse = 0;
	int		maxMatch, idx, ifld;
	int		numIndexMatch[MAX_GMD_INDEXES]={0};
	HANDLE	openedSQL=0;

	strcpy (File,InFile);
	ExpandText (File);
	if (!_fstrchr (File,'.'))
	{
		if (!FilePathHandle)
{
#if ENABLETRACE
GSSiExitProg (635);
#endif
			return FALSE;
}       
		DoClose = FALSE;
		FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
	    
	    isql = FilePathPtr->NumFiles;	
		while (isql--)
		{
			lpFileHandle = &FilePathPtr->FileHandle;  
			lpFileHandle += isql;
			if (*lpFileHandle)
			{   
				SQLPtr = (LPOPENSQLDATA)GlobalLock(*lpFileHandle);
				FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
				hDBDest = FilePtr->FileHandle;
				if (!_fstricmp (File,SQLPtr->IDName))
				{   
					openedSQL = *lpFileHandle;
					GlobalUnlock(*lpFileHandle);
					GlobalUnlock (SQLPtr->OFHandle);
					GlobalUnlock (FilePathHandle);
					goto FoundFile;
				}
				GlobalUnlock(*lpFileHandle);
				GlobalUnlock (SQLPtr->OFHandle);
			}
		}
		GlobalUnlock (FilePathHandle);
{
#if ENABLETRACE
GSSiExitProg (635);
#endif
			return FALSE;
}
	}
	else
	{
		hDBDest = OpenGWDatabase (File,BT_WRITE);   
		if (!hDBDest)
{
#if ENABLETRACE
GSSiExitProg (635);
#endif
		return FALSE;
}   
	}
FoundFile:
	lUp = strlen (UpdateStringIN);
	hUpdateString = GSSiGlobAlloc (0,GMEM_MOVEABLE,lUp+1);
	UpdateString = GlobalLock (hUpdateString);
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest);
	hmemset ((HPSTR)&lpGWDHead->GWDData,0,lpGWDHead->Reclen); 
	if (!stricmp (KeyString,"ALL"))
	{
		int	pos=BT_FIRST;

		GSSiGetTempFileName (0,"gm",0,(LPSTR)OffsetList); 
		FidOffsetList = GSSiOpenFile (OffsetList,0,OF_CREATE);
		while (!BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],pos,BT_ANY,(LPSTR)&Offset)) 
		{
			pos = BT_NEXT;
			BigWrite (FidOffsetList,&Offset,4,-1);
		}
		GSSillseek (FidOffsetList,0,0);
		goto NextRec;
	}
	HaveKey = TRUE;
	pComma = KeyString;
	while (pComma)
	{   
		Name = pComma;
		if ((pEQ = MatchLev (Name,'=')))
		{   
			*pEQ++ = 0;
			Value = pEQ;
			if ((pComma = MatchLev (Value,Separator)))
				*pComma++ = 0;

			for (idx=0;idx<lpGWDHead->NumIndex;idx++)
			{
				if (!lpGWDHead->SpatialIndex || idx != lpGWDHead->SpatialIndex)
				for (ifld=0;ifld<lpGWDHead->NumIndexFields[idx];ifld++)
				{
					LPGWFLDINFO pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[idx][ifld];
	
					if (!stricmp (pFldInfo->Name,Name))
						numIndexMatch[idx]++;
				}
			}


			if (!SetFieldValFromCharAndName(lpGWDHead, Name, Value, FALSE, TRUE))
    		{
    			if (!Truncate)
    				goto Exit;
    			if (Truncate == 2)
    				GSSiMsgBox (0,Value,Name,MB_ICONEXCLAMATION,0);
    		}
		}
		else
			goto Exit;
	}
	maxMatch = numIndexMatch[0];
	for (idx=1;idx<lpGWDHead->NumIndex;idx++)
	{
		if (numIndexMatch[idx] > maxMatch)
		{
			maxMatch = numIndexMatch[idx];
			indexToUse = idx;
		}
	}
	if (!GWDFormKey(lpGWDHead,indexToUse,TRUE,0,0))
    	goto Exit;
NextRec:
	strcpy (UpdateString,UpdateStringIN);
	if (FidOffsetList != HFILE_ERROR)
	{
		if (BigRead (FidOffsetList,&Offset,4) != 4)
			goto Exit;
	   	FillGWDData (lpGWDHead,Offset);
	}
	else if (!BT_FIND (lpGWDHead->BTHandle[indexToUse],lpGWDHead->pKeys[indexToUse],BT_FIRST,BT_EQ,(LPSTR)&Offset))
	{
	   	FillGWDData (lpGWDHead,Offset);
		rtn = 2;
	}
	else
	{
		if (updateOnly)
			goto Exit;
		rtn = 1;
	}
	if (!MatchLev (UpdateString,'='))  
	{
		if (FilePathHandle) 
		{
			LPFILEPATH	FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
    
		    isql = FilePathPtr->NumFiles;	
			while (isql--)
			{
				lpFileHandle = &FilePathPtr->FileHandle;  
				lpFileHandle += isql;
				if (*lpFileHandle)
				{   
					SQLPtr = (LPOPENSQLDATA)GlobalLock(*lpFileHandle);
					FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); //IgnoreLock=FALSE; 
					if (FilePtr->Type != SQL_DATAFILE)
					{
						if (!_fstricmp (UpdateString,SQLPtr->IDName))
						{
							LPFIELDINFO lpFieldInfo = &FilePtr->FldInfo;   
							USHORT	ifield;
							
							for (ifield=0;ifield<FilePtr->NumFields;ifield++,lpFieldInfo++)
							{
								char	Value[100];

								if (HaveKey)
								{
									int	j;
									for (j=0;j<lpGWDHead->NumIndexFields[0];j++)
									{
										if (!stricmp (lpFieldInfo->name,lpGWDHead->pFldInfo[j].Name))
											goto SkipField;
									}
								}
								sprintf (Value,"[%s.%s]",SQLPtr->IDName,lpFieldInfo->name);
								SetFieldValFromCharAndName(lpGWDHead, lpFieldInfo->name, Value, FALSE, TRUE);
SkipField:;
							} 
							GlobalUnlock (SQLPtr->OFHandle);
							GlobalUnlock (*lpFileHandle);
							break;
						}
					}
					GlobalUnlock (SQLPtr->OFHandle);
					GlobalUnlock (*lpFileHandle);
				}
			}
			GlobalUnlock (FilePathHandle); 
		}
	}
	else
	{
		pComma = UpdateString;
		while (pComma)
		{   
			Name = pComma;
			if ((pEQ = MatchLev (Name,'=')))
			{   
				*pEQ++ = 0;
				Value = pEQ;
				if ((pComma = MatchLev (Value,';')))
					*pComma++ = 0;
	    		if (!SetFieldValFromCharAndName(lpGWDHead,Name,Value,FALSE,TRUE))
	    		{
	    			if (!Truncate)
	    				goto Exit;
	    			if (Truncate == 2)
	    				GSSiMsgBox (0,Value,Name,MB_ICONEXCLAMATION,0);
	    		}
			}
			else
				pComma = 0;
		} 
	}
	{
		static n=0,ndb=4,ii;

		n++;
		if (n==ndb)
			ii=1;
	rtn = GWDReplaceRecord (lpGWDHead,0,NULL,-1); 
	if (openedSQL)
	{
		SQLPtr = (LPOPENSQLDATA)GlobalLock(openedSQL);
		SQLPtr->lastreadtime = 0;
		GlobalUnlock (openedSQL);
	}
	}
	if (FidOffsetList != HFILE_ERROR)
		goto NextRec;
Exit:
	if (FidOffsetList != HFILE_ERROR)
	{
		GSSiClose (FidOffsetList);
		GSSiRemove (OffsetList);
	}
	GlobalUnlock (hDBDest);
	GSSiGlobUlFree (&hUpdateString);
	if (DoClose)
		CloseGWDatabase (hDBDest);
	
{
#if ENABLETRACE
GSSiExitProg (635);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL FindGMDRecord (LPSTR File,LPSTR KeyString,LPSTR SetValString, short MinMatchChar,short index)
#if ENABLETRACE
{GSSiEnterProg (636);
#endif
{
	LPGWDHEADER lpGWDHead;
	HANDLE	hDBDest=0; 
	long	Offset;
	LPSTR	Name,Value,pComma, pEQ;     
	BOOL	rtn=FALSE, DoClose = TRUE;
	LPOPENSQLDATA	SQLPtr;
	LPOPENFILEDATA	FilePtr;   
    LPFILEPATH  FilePathPtr; 
	LPHANDLE	lpFileHandle; 
	short	Want, pos=BT_ANY, isql; 
	char	WantKey[256];  
	
	if (!_fstrchr (File,'.'))
	{
		if (!FilePathHandle)
{
#if ENABLETRACE
GSSiExitProg (635);
#endif
			return FALSE;
}       
		FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
	    
	    isql = FilePathPtr->NumFiles;	
		while (isql--)
		{
			lpFileHandle = &FilePathPtr->FileHandle;  
			lpFileHandle += isql;
			if (*lpFileHandle)
			{   
				SQLPtr = (LPOPENSQLDATA)GlobalLock(*lpFileHandle);
				FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
				hDBDest = FilePtr->FileHandle;
				if (!_fstricmp (File,SQLPtr->IDName))
				{   
					hDBDest = *lpFileHandle;
					GlobalUnlock(*lpFileHandle);
					GlobalUnlock (SQLPtr->OFHandle);
					GlobalUnlock (FilePathHandle); 
					DoClose = FALSE;  
					goto FoundFile;
				}
				GlobalUnlock(*lpFileHandle);
				GlobalUnlock (SQLPtr->OFHandle);
			}
		}
		GlobalUnlock (FilePathHandle);
{
#if ENABLETRACE
GSSiExitProg (635);
#endif
			return FALSE;
}
	}
	else
	{
	    if (!OpenDataFile (File, "",BT_READ, &hDBDest))
{
#if ENABLETRACE
GSSiExitProg (636);
#endif
		return FALSE;
} 
	}
FoundFile:
	SQLPtr = (LPOPENSQLDATA) GlobalLock (hDBDest);
    FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
	lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 
	hmemset ((HPSTR)&lpGWDHead->GWDData,0,lpGWDHead->Reclen); 

	if (!_fstricmp (KeyString,"FIRST"))
		Want = BT_FIRST;
	else if (!_fstricmp (KeyString,"LAST"))
		Want = BT_LAST;
	else if (!_fstrnicmp (KeyString,"FIRST(",6))
	{
		Want = BT_FIRST;
		index = max (0,atoi (&KeyString[6]) - 1);
	}
	else if (!_fstrnicmp (KeyString,"LAST(",5))
	{
		Want = BT_LAST;
		index = max (0,atoi (&KeyString[5]) - 1);
	}
	else
	{ 
		Want = BT_FIRST;
		if (MinMatchChar)
			pos = BT_GE;
		else
			pos = BT_EQ;
		pComma = KeyString;
		while (pComma)
		{   
			Name = pComma; 
			if ((pEQ = MatchLev (Name,'=')))
			{   
				*pEQ++ = 0;
				Value = pEQ;
				if ((pComma = MatchLev (Value,';')))
					*pComma++ = 0;
				if (!SetFieldValFromCharAndName(lpGWDHead, Name, Value, FALSE, TRUE))
	    			goto Exit;
			}
		}
	    if (!GWDFormKey(lpGWDHead,index,TRUE,0,0))
	    	goto Exit; 
	}
	if (MinMatchChar)
		_fmemmove (WantKey,lpGWDHead->pKeys[index],abs (lpGWDHead->lKeys[index]));
   	if (BT_FIND (lpGWDHead->BTHandle[index],lpGWDHead->pKeys[index],Want,pos,(LPSTR)&SQLPtr->Offset)) 
   	{
   		if (MinMatchChar) 
   		{  
   			if (BT_FIND (lpGWDHead->BTHandle[index],lpGWDHead->pKeys[index],BT_LAST,BT_ANY,(LPSTR)&SQLPtr->Offset)) 
   				goto Exit;
   		}
   		else
   			goto Exit;
   	}
	if (MinMatchChar)
	{   
		long PriorOffset;
		short nMatched1=0, nMatched2 = NumMatchChar (lpGWDHead->pKeys[index],WantKey,abs (lpGWDHead->lKeys[index]));
		
   		if (!BT_FIND (lpGWDHead->BTHandle[index],lpGWDHead->pKeys[index],BT_PRIOR,BT_ANY,(LPSTR)&PriorOffset)) 
   		{
   			if ((nMatched1 = NumMatchChar (lpGWDHead->pKeys[index],WantKey,abs (lpGWDHead->lKeys[index]))) > nMatched2)
   				SQLPtr->Offset = PriorOffset; 
   		}
		if (max (nMatched1,nMatched2) < MinMatchChar)
			goto Exit;
	}
   	SQLPtr->st = 0;  
   	SQLPtr->lastreadtime = LONG_MAX;
   	SQLPtr->NumGlobals = 0;
   	ExpandText (SetValString);
   	rtn = TRUE;
Exit:
	GlobalUnlock (FilePtr->FileHandle);
	GlobalUnlock (SQLPtr->OFHandle); 
	GlobalUnlock (hDBDest);
	if (DoClose)
		CloseDataFile (TRUE, &hDBDest);	
	
{
#if ENABLETRACE
GSSiExitProg (636);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL GMDCopyRecord (LPSTR File,LPSTR FromKeyString,LPSTR ToKeyString)
#if ENABLETRACE
{GSSiEnterProg (636);
#endif
{
	LPGWDHEADER lpGWDHead;
	HANDLE	hDBDest=0; 
	long	Offset;
	LPSTR	Name,Value,pComma, pEQ;     
	BOOL	rtn=FALSE, DoClose = TRUE;
	LPOPENSQLDATA	SQLPtr;
	LPOPENFILEDATA	FilePtr;   
    LPFILEPATH  FilePathPtr; 
	LPHANDLE	lpFileHandle; 
	short	Want, pos=BT_ANY, isql;
	 
	if (!_fstrchr (File,'.'))
	{
		if (!FilePathHandle)
{
#if ENABLETRACE
GSSiExitProg (635);
#endif
			return FALSE;
}       
		FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
	    
	    isql = FilePathPtr->NumFiles;	
		while (isql--)
		{
			lpFileHandle = &FilePathPtr->FileHandle;  
			lpFileHandle += isql;
			if (*lpFileHandle)
			{   
				SQLPtr = (LPOPENSQLDATA)GlobalLock(*lpFileHandle);
				FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
				hDBDest = FilePtr->FileHandle;
				if (!_fstricmp (File,SQLPtr->IDName))
				{   
					hDBDest = *lpFileHandle;
					GlobalUnlock(*lpFileHandle);
					GlobalUnlock (SQLPtr->OFHandle);
					GlobalUnlock (FilePathHandle); 
					DoClose = FALSE;  
					goto FoundFile;
				}
				GlobalUnlock(*lpFileHandle);
				GlobalUnlock (SQLPtr->OFHandle);
			}
		}
		GlobalUnlock (FilePathHandle);
{
#if ENABLETRACE
GSSiExitProg (635);
#endif
			return FALSE;
}
	}
	else
	{
	    if (!OpenDataFile (File, "",BT_WRITE, &hDBDest))
{
#if ENABLETRACE
GSSiExitProg (636);
#endif
		return FALSE;
} 
	}
FoundFile:
	SQLPtr = (LPOPENSQLDATA) GlobalLock (hDBDest);
    FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
	lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 
	hmemset ((HPSTR)&lpGWDHead->GWDData,0,lpGWDHead->Reclen); 

	Want = BT_FIRST;
	pos = BT_EQ;
	pComma = FromKeyString;
	while (pComma)
	{   
		Name = pComma; 
		if ((pEQ = MatchLev (Name,'=')))
		{   
			*pEQ++ = 0;
			Value = pEQ;
			if ((pComma = MatchLev (Value,';')))
				*pComma++ = 0;
			if (!SetFieldValFromCharAndName(lpGWDHead, Name, Value, FALSE, TRUE))
    			goto Exit;
		} 
		else
			goto Exit;
	}
    if (!GWDFormKey(lpGWDHead,0,TRUE,0,0))
    	goto Exit; 
   	if (!BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],Want,pos,(LPSTR)&SQLPtr->Offset)) 
   	{
	    FillGWDData (lpGWDHead,SQLPtr->Offset);
		pComma = ToKeyString;
		while (pComma)
		{   
			Name = pComma; 
			if ((pEQ = MatchLev (Name,'=')))
			{   
				*pEQ++ = 0;
				Value = pEQ;
				if ((pComma = MatchLev (Value,';')))
					*pComma++ = 0;
				if (!SetFieldValFromCharAndName(lpGWDHead, Name, Value, FALSE, TRUE))
	    			goto Exit;
			}
			else
				goto Exit;
		}
		GWDAddRecord (lpGWDHead,0,NULL);
	   	rtn = TRUE;
	}
Exit:
	GlobalUnlock (FilePtr->FileHandle);
	GlobalUnlock (SQLPtr->OFHandle); 
	GlobalUnlock (hDBDest);
	if (DoClose)
		CloseDataFile (TRUE, &hDBDest);	
	
{
#if ENABLETRACE
GSSiExitProg (636);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

int DeleteGMDRecords (LPSTR File,LPSTR SQL)
#if ENABLETRACE
{GSSiEnterProg (637);
#endif
{
    LPFILEPATH  FilePathPtr; 
	LPHANDLE	lpFileHandle; 
	LPOPENSQLDATA	SQLPtr;
	LPFIELDINFO	lpFieldInfo;
	LPOPENFILEDATA	FilePtr;
	LPGWDHEADER lpGWDHead;
	HANDLE	hDBDest=0;  
	BOOL	DoClose=TRUE;
	int		rtn=0;
	LPSTR	Name,Value,pComma, pEQ;   
	short	isql;
	OPENSQLDATA	SaveSQLData;
	OPENFILEDATA SaveFileData;

	
	if (!_fstrchr (File,'.'))
	{
		if (!FilePathHandle)
{
#if ENABLETRACE
GSSiExitProg (635);
#endif
			return FALSE;
}       
		DoClose = FALSE;
		FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
	    
	    isql = FilePathPtr->NumFiles;	
		while (isql--)
		{
			lpFileHandle = &FilePathPtr->FileHandle;  
			lpFileHandle += isql;
			if (*lpFileHandle)
			{   
				SQLPtr  = (LPOPENSQLDATA)GlobalLock(*lpFileHandle);
				FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
				SaveSQLData  = *SQLPtr;
				SaveFileData = *FilePtr;
				if (!_fstricmp (File,SQLPtr->IDName))
				{   
					hDBDest = *lpFileHandle;
					ProcessFileSQL (SQLPtr,FilePtr,SQL);   
					SQLPtr->lastreadtime = 0;                                     
					GlobalUnlock(*lpFileHandle);  
					GlobalUnlock (SQLPtr->OFHandle);
					GlobalUnlock (FilePathHandle);
					goto FoundFile;
				}
				GlobalUnlock(*lpFileHandle);
				GlobalUnlock (SQLPtr->OFHandle);
			}
		}
		GlobalUnlock (FilePathHandle);
{
#if ENABLETRACE
GSSiExitProg (635);
#endif
			return FALSE;
}
	}
	else
	{
	    if (!OpenDataFile (File,SQL,BT_WRITE,&hDBDest))
{
#if ENABLETRACE
GSSiExitProg (637);
#endif
    	return FALSE;
}
	}
FoundFile:
	SQLPtr = (LPOPENSQLDATA)GlobalLock (hDBDest);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 
	while (FetchDBRec (hDBDest))
	{   
		if (GWDDeleteRecord (lpGWDHead,SQLPtr->Offset))
			rtn++;
		SQLPtr->lastreadtime = 0;
	}
	if (!DoClose)
	{
		*SQLPtr = SaveSQLData;
		*FilePtr = SaveFileData;
	}
	GlobalUnlock (FilePtr->FileHandle);
	GlobalUnlock (SQLPtr->OFHandle); 
	GlobalUnlock (hDBDest);
	if (DoClose)
		CloseDataFile (TRUE, &hDBDest);	
{
#if ENABLETRACE
GSSiExitProg (637);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL GMDGetCharFieldVal (LPGWDHEADER lpGWDHead,int Field,LPSTR str)
#if ENABLETRACE
{GSSiEnterProg (638);
#endif
{   LPSTR   lpVal;
    LPGWFLDINFO lpGWFldInfo;
    BOOL        Status;
	BOOL	binary = FALSE;

	if (Field < 0)
	{
		Field = -Field;
		binary = TRUE;
	}
    
    Status = TRUE;
    lpGWFldInfo = lpGWDHead->pFldInfo + Field;
    lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];
    
    switch (lpGWFldInfo->Type)
    {   
        default: 
        case BT_RIGHT_CHAR:
        case BT_CHAR:
			if (binary)
				memmove(str, lpVal, lpGWFldInfo->Len);
			else
				_fstrncpy(str, lpVal, lpGWFldInfo->Len);
			str += lpGWFldInfo->Len;
            *str = '\0';
        break;
                                                
        case BT_INTEGER:
            if (lpGWFldInfo->Len == 2)
                itoa (*(LPSHORT)lpVal,str,10);
            else
                ltoa (*(LPLONG)lpVal,str,10);
        break;
                                                
        case BT_REAL:
            if (lpGWFldInfo->Len == 4)
                sprintf(str,"%f",*(LPFLOAT)lpVal);
			else
			{
				if (_snprintf(str, 32, "%G", *(LPDOUBLE)lpVal) < 0)
					ii = 1;
				else
					ii = 1;
			}
        break;
    }
{
#if ENABLETRACE
GSSiExitProg (638);
#endif
    return Status; 
}
#if ENABLETRACE
}
#endif
}

int GMDGetIntegerFieldVal (LPGWDHEADER lpGWDHead,int Field)
#if ENABLETRACE
{GSSiEnterProg (638);
#endif
{   LPSTR   lpVal;
    LPGWFLDINFO lpGWFldInfo;
	char	str[128];
	int		val;
    
    lpGWFldInfo = lpGWDHead->pFldInfo + Field;
    lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];
    
    switch (lpGWFldInfo->Type)
    {   
        default: 
        case BT_RIGHT_CHAR:
        case BT_CHAR:
            strncpy0 (str,lpVal,lpGWFldInfo->Len);
			val = atoi (str);
        break;
                                                
        case BT_INTEGER:
            if (lpGWFldInfo->Len == 2)
                val = *(LPSHORT)lpVal;
            else
                val = *(LPLONG)lpVal;
        break;
                                                
        case BT_REAL:
            if (lpGWFldInfo->Len == 4)
                val = IDNINT (*(LPFLOAT)lpVal);
            else
                val = IDNINT (*(LPDOUBLE)lpVal);
        break;
    }
{
#if ENABLETRACE
GSSiExitProg (638);
#endif
    return val; 
}
#if ENABLETRACE
}
#endif
}

double GMDGetRealFieldVal (LPGWDHEADER lpGWDHead,int Field)
#if ENABLETRACE
{GSSiEnterProg (638);
#endif
{   LPSTR   lpVal;
    LPGWFLDINFO lpGWFldInfo;
	char	str[128];
	double	val;
    
    lpGWFldInfo = lpGWDHead->pFldInfo + Field;
    lpVal = &lpGWDHead->GWDData[lpGWFldInfo->Beg];
    
    switch (lpGWFldInfo->Type)
    {   
        default: 
        case BT_RIGHT_CHAR:
        case BT_CHAR:
            strncpy0 (str,lpVal,lpGWFldInfo->Len);
			val = atof (str);
        break;
                                                
        case BT_INTEGER:
            if (lpGWFldInfo->Len == 2)
                val = *(LPSHORT)lpVal;
            else
                val = *(LPLONG)lpVal;
        break;
                                                
        case BT_REAL:
            if (lpGWFldInfo->Len == 4)
                val = *(LPFLOAT)lpVal;
            else
                val = *(LPDOUBLE)lpVal;
        break;
    }
{
#if ENABLETRACE
GSSiExitProg (638);
#endif
    return val; 
}
#if ENABLETRACE
}
#endif
}

BOOL CreateGWDIndex (HANDLE hDB, LPSTR Name, short CreateIndex)
#if ENABLETRACE
{GSSiEnterProg (640);
#endif
{
    LPGWFLDINFO pFldInfo;
    LPGWDHEADER lpGWDHead;
    long        Offset, nRecs, nLoaded; 
    short       i, ibeg, len, nFld; 
    long		st,ii;
    HANDLE      hVars=0;
    BTVARDESC   *pVars;
    char        str[256], mess[256]; 
    short       pos, DupPos;   
    BOOL		rtn=FALSE;

    hVars = GSSiGlobAlloc ( 273,GHND,(MAX_GMD_INDEX_FIELDS+1) * sizeof(BTVARDESC));
    pVars = (BTVARDESC *)GlobalLock(hVars);
    ibeg = 0;

    sprintf(mess,"Index %i",CreateIndex+1);
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
	if (!CreateIndex && lpGWDHead->SplitFile)
	{
		MessageBox(0, "Cannot rebuild primary index on split gmd file", 0, MB_ICONEXCLAMATION);
		goto Exit;
	}
	if (CreateIndex && CreateIndex == lpGWDHead->SpatialIndex)
	{
		switch (lpGWDHead->SpatialIndexType)
		{
		case 1:	//Month,Grid
			pVars->BT_VARLEN=2;
			pVars->BT_VARTYP=BT_INTEGER;
			pVars++->BT_VAROFF=0;
			pVars->BT_VARLEN=2;
			pVars->BT_VARTYP=BT_INTEGER;
			pVars++->BT_VAROFF=2;
			pVars->BT_VARLEN=lpGWDHead->lKeys[0];
			pVars->BT_VARTYP=BT_CHAR;
			pVars->BT_VAROFF=4;
			nFld = 3;
			DupPos = 2;
			lpGWDHead->lKeys[CreateIndex] = -(4 + lpGWDHead->lKeys[0]);
			break;
		case 2:	//Month,Code,Grid
			pVars->BT_VARLEN=2;
			pVars->BT_VARTYP=BT_INTEGER;
			pVars++->BT_VAROFF=0;
			pVars->BT_VARLEN=2;
			pVars->BT_VARTYP=BT_INTEGER;
			pVars++->BT_VAROFF=2;
			pVars->BT_VARLEN=2;
			pVars->BT_VARTYP=BT_INTEGER;
			pVars++->BT_VAROFF=4;
			pVars->BT_VARLEN=lpGWDHead->lKeys[0];
			pVars->BT_VARTYP=BT_CHAR;
			pVars->BT_VAROFF=6;
			nFld = 4;
			DupPos = 3;
			lpGWDHead->lKeys[CreateIndex] = -(6 + lpGWDHead->lKeys[0]);
			break;
		}
	}
	else
	{
		for (i=0;i<abs(lpGWDHead->NumIndexFields[CreateIndex]);i++,pVars++) 
		{
			 pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[CreateIndex][i];
			 pVars->BT_VARLEN=min(128,pFldInfo->Len);
			 pVars->BT_VARTYP=pFldInfo->Type;
			 pVars->BT_VAROFF=ibeg;
			 ibeg += pFldInfo->Len;
		} 
		nFld = abs(lpGWDHead->NumIndexFields[CreateIndex]); 
		DupPos = 0;
		if (lpGWDHead->lKeys[CreateIndex]>0)
			lpGWDHead->lKeys[CreateIndex]=ibeg;
		else
		{   
			DupPos = nFld;
			pVars->BT_VARLEN=lpGWDHead->lKeys[0];
			pVars->BT_VARTYP=BT_CHAR;
			pVars->BT_VAROFF=ibeg; 
			ibeg+=lpGWDHead->lKeys[0];
			lpGWDHead->lKeys[CreateIndex]=-ibeg;  
			nFld++;
		} 
	}
//        lpGWDHead->NumIndexFields[CreateIndex] = nFld;
    lpGWDHead->hKeys[CreateIndex]=GSSiGlobAlloc ( 274,GHND,abs(lpGWDHead->lKeys[CreateIndex]));
    lpGWDHead->pKeys[CreateIndex]=GlobalLock(lpGWDHead->hKeys[CreateIndex]);
                                                 
    _fstrcpy (str,Name); 
    sprintf (_fstrrchr (str,'.'),".in%i",CreateIndex+1);
    GlobalUnlock(hVars);
    pVars = (BTVARDESC *)GlobalLock(hVars);
    rtn = BT_CREATE (str, 4, FALSE,nFld, 1,pVars,FALSE, DupPos, lpGWDHead->TimeStamp, FALSE);
    GSSiGlobUlFree (&hVars);
    if (!rtn)
    	goto Exit;
    lpGWDHead->BTHandle[CreateIndex] = BT_OPEN (str, lpGWDHead->TimeStamp, BT_WRITE, 0);
                            
    nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
    nLoaded = 0;
                
    if (CreateIndex)
    {   
    	if (nRecs)
    	{
			CreateStatusWind (hWndMain,1,"Create Index");
			StatusWindowUpdate ("Create Index",Name, nRecs, 0);
        	pos = BT_FIRST;             
            while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],pos,BT_ANY, (LPSTR)&Offset)) 
            {   
            	pos = BT_NEXT;
				if (FillGWDData(lpGWDHead, Offset) > 0)
				{
					if (CreateIndex && CreateIndex == lpGWDHead->SpatialIndex)
					{
						switch (lpGWDHead->SpatialIndexType)
						{
						case 1:
						case 2:
						{
							LPGWFLDINFO pFldInfo;
							int	Time, FromMonth, ToMonth, Month;

							pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->FromDateField;
							Time = *(LPLONG)&lpGWDHead->GWDData[pFldInfo->Beg];
							FromMonth = SysMonthFromSymTime(Time);
							pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->ToDateField;
							Time = *(LPLONG)&lpGWDHead->GWDData[pFldInfo->Beg];
							ToMonth = SysMonthFromSymTime(Time);
							for (Month = FromMonth; Month <= ToMonth; Month++)
							{
								GWDFormKey(lpGWDHead, CreateIndex, FALSE, 0, Month);
								BT_PUT(lpGWDHead->BTHandle[CreateIndex], lpGWDHead->pKeys[CreateIndex], (LPSTR)&Offset);
							}
						}
							break;
						}
					}
					else
					{
						GWDFormKey(lpGWDHead, CreateIndex, FALSE, 0, 0);
						BT_PUT(lpGWDHead->BTHandle[CreateIndex], lpGWDHead->pKeys[CreateIndex], (LPSTR)&Offset);
					}
				}
				StatusWindowUpdate (NULL,NULL, nRecs, ++nLoaded);
				if (nLoaded == 653700)
					ii = 1;
            }
			DestroyStatusWindow(0);  
        }
    }
    else
    {    
	    SetGWDCurrentOffset (lpGWDHead,-1);
		CreateStatusWind (hWndMain,1,"Create Index");
		StatusWindowUpdate ("Create Index",Name, nRecs, 0);
        Offset = GSSillseek (lpGWDHead->Fid,0,1);
        nRecs = GSSillseek (lpGWDHead->Fid,0,2); 
        GSSillseek (lpGWDHead->Fid,Offset,0);
        while (ContinueProcessing && Offset < nRecs && ((st=FillGWDData (lpGWDHead,Offset))) != -1)
        {   
            if (st > 0)
            {
	            GWDFormKey(lpGWDHead,CreateIndex,FALSE,0,0);
	            BT_PUT (lpGWDHead->BTHandle[CreateIndex],lpGWDHead->pKeys[CreateIndex],(LPSTR)&Offset); 
	        }
            Offset = GSSillseek (lpGWDHead->Fid,0,1);
			StatusWindowUpdate (NULL,NULL, nRecs, Offset);
        }
		DestroyStatusWindow(0);  
    } 
    rtn = ContinueProcessing;
    SetContinueProcessing ( TRUE);
    BT_SET_TIME_STAMP (lpGWDHead->BTHandle[CreateIndex], lpGWDHead->TimeStamp);
    BT_CLOSE (lpGWDHead->BTHandle[CreateIndex]);   
    lpGWDHead->BTHandle[CreateIndex] = BT_OPEN (str, lpGWDHead->TimeStamp, BT_WRITE, 0);
	GSSiGlobUlFree (&lpGWDHead->hKeys[CreateIndex]);
Exit:
	GSSiGlobUlFree(&hVars);
	if (lpGWDHead->hKeys[CreateIndex])
		GlobalUnlock(lpGWDHead->hKeys[CreateIndex]);
    GlobalUnlock (hDB);
{
#if ENABLETRACE
GSSiExitProg (640);
#endif
 return rtn;
}
#if ENABLETRACE
}
#endif
} 

short ComputeGWDKeyLen(LPGWDHEADER lpGWDHead, short KeyID)
#if ENABLETRACE
{GSSiEnterProg (641);
#endif
{   short l, i;
    LPGWFLDINFO pFldInfo;
    
    l=0;
    for (i=0;i<abs(lpGWDHead->NumIndexFields[KeyID]);i++) 
    {
         pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[KeyID][i];
         l += pFldInfo->Len;
    } 

{
#if ENABLETRACE
GSSiExitProg (641);
#endif
    return(l);
}
#if ENABLETRACE
}
#endif
}
                                       
BOOL FAR PASCAL DISPLAY_GWD_DATAMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (642);
#endif
{ 
	BOOL	rtn = FALSE;
    static  LPSTR   pName=0;
    static  HANDLE hDB=0;
    LPGWFLDINFO lpGWFldInfo;
    static  LPGWDHEADER lpGWDHead;
    static  HANDLE      hBT;
    short         st, i, len, pos=BT_FIRST;
    static      short Index=0, DataFileType=0;
    LPVOID      lpVal; 
    LPSTR	str, str2;
    int           TabStops[2]={150,500}; 
    short         Choice;
    static	LPOPENFILEDATA  FilePtr;
    static	LPOPENSQLDATA   SQLPtr; 
    static      HANDLE      hSQL; 
    static      long    CurRec;
    //BTHEAD BTHead;
    BOOL    False=FALSE;
    static	UINT     IDC_FieldName; 
    static	HANDLE	hSaveBM=0; 
	LPFIELDINFO	pFldInfo;
    HANDLE	hMem=0;                  
	int   nItems;
	LPINT lpItems;
	HANDLE hItems;   
	static	HANDLE hNameLocal=0;  
	static	char	CurrentDBName[256];
 
	if (inOpenFileDialog)
		return FALSE;
 short    BRtn;
 if (Message == WM_SETFOCUS)
	 ii = 1;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (642);
#endif
 	return (BRtn);
}
	if (hName && !hNameLocal)
	{
	 	hNameLocal = hName;
	 	hName = 0; 
	 }
 if (Message == WM_INITDIALOG)  
 {   
 	IDC_FieldName=IDC_DATA_LIST2; 
    hSQL = 0;
//	hSaveBM = EnterBlockingWindow (hWndDlg);
 }
 if (hNameLocal)
 {
	 pName=GlobalLock(hNameLocal);
	 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam,0,
	                     SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, &IDC_FieldName,1,
	                     pName, &DataFileType, &hSQL, &False,FALSE))
 	{
 		GlobalUnlock (hNameLocal);
{
#if ENABLETRACE
GSSiExitProg (642);
#endif
                     return TRUE;
}
 	} 
	GlobalUnlock (hNameLocal);
 }
 hMem = GSSiGlobAlloc ( 275,GMEM_MOVEABLE,2*4096);
 str = GlobalLock (hMem);
 str2 = str + 4096;   
 switch(Message)
   {
    case WM_INITDIALOG:  
         /* initialize working variables                                */ 
    /*   CreateCityExtract ();*/
         CurrentIndex=0; 
         *CurrentDBName = 0; 
         SendDlgItemMessage (hWndDlg,IDC_DATA_LIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
         SendDlgItemMessage (hWndDlg,IDC_DATA_LIST2,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
        
OpenDB:          
         if (!hSQL)
         	goto RtnFalse;
         SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
         FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
         hDB = FilePtr->FileHandle;
         GlobalUnlock (SQLPtr->OFHandle);
         GlobalUnlock (hSQL);
         if (!hDB)
            goto RtnFalse;    
         if (FilePtr->Type != 1) break;
         lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
         if (!lpGWDHead->Version)
         	ShowWindow (GetDlgItem(hWndDlg,IDC_CHANGE_FIELD_NAME),SW_HIDE); 
         CurRec = 0;
         st = BT_FIND (lpGWDHead->BTHandle[CurrentIndex],lpGWDHead->pKeys[CurrentIndex],BT_FIRST,BT_ANY, (LPSTR)&SQLPtr->Offset);
         CurRec = 1;  
         GlobalUnlock (hDB);
         
Display: 
         SendDlgItemMessage (hWndDlg,IDC_FieldName,LB_RESETCONTENT,0,0);
		 if (!st && DataFileType == 1)
         {  
	         lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
        	 FillGWDData (lpGWDHead,SQLPtr->Offset);  
	         //GetBTHeader (lpGWDHead->BTHandle[CurrentIndex],&BTHead);  
	         sprintf (str2,"%ld of %ld",CurRec,BT_NUM_IN_INDEX(lpGWDHead->BTHandle[CurrentIndex]));
	         SetDlgItemText (hWndDlg,IDC_RECID,str2);
	         for (i=0,lpGWFldInfo = lpGWDHead->pFldInfo;
	             i<lpGWDHead->NumFields; i++,lpGWFldInfo++)
	         {
	            _fstrcpy (str,lpGWFldInfo->Name);   
	            _fstrcat (str,"\t"); 
	            GetValFromOpenFiles (lpGWFldInfo->Name,str2,4096);
	             _fstrcat(str,str2);
	             SendDlgItemMessage (hWndDlg,IDC_FieldName,LB_ADDSTRING,0,(LPARAM)str);
	         }
			 GlobalUnlock (hDB);
         } 
		 rtn = TRUE;
         break; /* End of WM_INITDIALOG                                 */
    case WM_DESTROY:
         CloseDataFile (TRUE, &hSQL); 
         GSSiGlobFree (&hNameLocal);
		break;
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
		 rtn = TRUE;
         break; /* End of WM_CLOSE                                      */
	case WM_SETFOCUS:
		rtn = FALSE;
		break;
    case WM_COMMAND:
#if WIN32
        switch(LOWORD(wParam))
#else
        switch(wParam)
#endif
           {  
              case IDC_SORTLIST:
		          if (SendDlgItemMessage (hWndDlg,IDC_SORTLIST,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
		          {
		          	ShowWindow (GetDlgItem (hWndDlg,IDC_DATA_LIST),SW_SHOW);
		          	ShowWindow (GetDlgItem (hWndDlg,IDC_DATA_LIST2),SW_HIDE);   
		          	EnableWindow (GetDlgItem (hWndDlg,IDC_CHANGE_FIELD_NAME),FALSE); 
		          	EnableWindow(GetDlgItem(hWndDlg,IDC_INDEXES),FALSE);
				  	IDC_FieldName=IDC_DATA_LIST; 
				  }
				  else
		          {
		          	ShowWindow (GetDlgItem (hWndDlg,IDC_DATA_LIST2),SW_SHOW);
		          	ShowWindow (GetDlgItem (hWndDlg,IDC_DATA_LIST),SW_HIDE); 
		          	if (DataFileType == 1)
		          	{
		          		EnableWindow (GetDlgItem (hWndDlg,IDC_CHANGE_FIELD_NAME),TRUE);
						EnableWindow(GetDlgItem(hWndDlg,IDC_INDEXES),TRUE);
					}
				  	IDC_FieldName=IDC_DATA_LIST2;
				  } 
                  st = 0;
              	  goto Display;
              	  
              case IDC_OPEN_DB:
         		  if (DataFileType == 1 || DataFileType == DBF_DATAFILE)
         		  {
					EnableWindow(GetDlgItem(hWndDlg,IDOK),TRUE);
					EnableWindow(GetDlgItem(hWndDlg,IDC_SEARCH),TRUE);
					EnableWindow(GetDlgItem(hWndDlg,IDC_FIRST),TRUE);
					EnableWindow(GetDlgItem(hWndDlg,IDC_LAST),TRUE);
					EnableWindow(GetDlgItem(hWndDlg,IDC_PRIOR),TRUE);
		          	if (!SendDlgItemMessage (hWndDlg,IDC_SORTLIST,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
					{
						EnableWindow(GetDlgItem(hWndDlg,IDC_CHANGE_FIELD_NAME),TRUE);
						EnableWindow(GetDlgItem(hWndDlg,IDC_INDEXES),TRUE);
					}
         		  }
         		  else
         		  {
					EnableWindow(GetDlgItem(hWndDlg,IDOK),TRUE);
					EnableWindow(GetDlgItem(hWndDlg,IDC_SEARCH),TRUE);
					EnableWindow(GetDlgItem(hWndDlg,IDC_FIRST),TRUE);
					EnableWindow(GetDlgItem(hWndDlg,IDC_LAST),FALSE);
					EnableWindow(GetDlgItem(hWndDlg,IDC_PRIOR),FALSE);
					EnableWindow(GetDlgItem(hWndDlg,IDC_CHANGE_FIELD_NAME),FALSE);
					EnableWindow(GetDlgItem(hWndDlg,IDC_INDEXES),FALSE);
         		  }
         		  if (hNameLocal)
         		  {
         		  	 pName=GlobalLock(hNameLocal);
         		  	 if (_fstricmp (pName,CurrentDBName))
         		  	 	CurrentIndex = 0;
         		  	 _fstrcpy (CurrentDBName,pName);
 					 GlobalUnlock (hNameLocal);
                  }
                  goto OpenDB;
                  
              case IDC_SEARCH: 
         		  if (DataFileType == 1)
				  {              
		    	      lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
					  for (i=0;i<abs(lpGWDHead->NumIndexFields[CurrentIndex]);i++)
					  { 
						  lpGWFldInfo = lpGWDHead->pFldInfo;
						  lpGWFldInfo+=lpGWDHead->IndexFields[CurrentIndex][i];  
						  *str = 0;
						  if (!GetTextString (hWndDlg,str,256,lpGWFldInfo->Name,NULL,NULL,0,TRUE,TRUE))
						  	break; 
						  SetFieldValFromCharAndName(lpGWDHead, lpGWFldInfo->Name, str, FALSE, TRUE);
		  		 	  }
	                  GWDFormKey(lpGWDHead,CurrentIndex,TRUE,0,0);
	                  st = BT_FIND (lpGWDHead->BTHandle[CurrentIndex],lpGWDHead->pKeys[CurrentIndex],BT_FIRST,BT_GE, (LPSTR)&SQLPtr->Offset);
					  GlobalUnlock (hDB);
	              	  goto Display; 
              	  }
              	  else 
              	  {   
              	  	  *str = 0;
                 	  if (GetSQLWhereClause (hWndDlg, hSQL, str)) 
                 	  {   
                 	  	  if (hSQL)
                 	  	  {
			                  CloseDataFile (TRUE, &hSQL); 
			              }
				 		  if (!OpenDataFile (pName,str,BT_READ,&hSQL))
			 		  		break; 
				          PostMessage(hWndDlg, WM_COMMAND, IDC_FIRST, 0L);
			 		  }
              	  } 
				  rtn = TRUE;
              	  break;
              	  
              case IDC_PRIOR:
				  if (DataFileType == DBF_DATAFILE)
				  {
					CurrentDBFRec = max (0,CurrentDBFRec-2);
					goto Display2;
				  }
				  else
				  {
	    			  lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
					  st = BT_FIND (lpGWDHead->BTHandle[CurrentIndex],lpGWDHead->pKeys[CurrentIndex],BT_PRIOR,BT_ANY, (LPSTR)&SQLPtr->Offset);
					  if (!st)
						CurRec--;
					  GlobalUnlock (hDB);
				  }
                  goto Display;

              case IDC_CREATENEWTABLE: 
				  rtn = TRUE;
				nItems = GetLBSelectedItems (hWndDlg,IDC_FieldName,&hItems);
				if (!nItems)
				{
					GSSiMsgBox(GetFocus(),"No fields selected",
					            "Error",MB_OK|MB_ICONEXCLAMATION,0);  
					break;
				}
                lpItems=  (LPINT) GlobalLock(hItems);
				SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
				FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
				*str = 0;
				for (i=0; i<nItems; i++,lpItems++)
				{   
					pFldInfo = &FilePtr->FldInfo + *lpItems;
					if (*str)
						_fstrcat (str,",");
				    CreateGMTextHeader (pFldInfo, _fstrchr(str,0));
				}  
				GSSiGlobUlFree (&hItems);
				GlobalUnlock (SQLPtr->OFHandle);
				GlobalUnlock (hSQL);
              	break;
              	
              case IDC_SHOWTYPE:
				  rtn = TRUE;
			    SendDlgItemMessage (hWndDlg,IDC_FieldName,LB_RESETCONTENT,0,0);  
                if (!hSQL)
                	break;
                CloseDataFile (TRUE, &hSQL); 
                ShowNonStandardFields = TRUE;
				st = OpenDataFile (pName,"",BT_READ,&hSQL);
                ShowNonStandardFields = FALSE;
				if (!st)
			 		break; 
				SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
				FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
				for (i=0,pFldInfo = &FilePtr->FldInfo; i<FilePtr->NumFields; i++,pFldInfo++)
				{
					_fstrcpy (str,pFldInfo->name);   
					_fstrcat (str,"\t"); 
				    CreateGMTextHeader (pFldInfo, str);
					SendDlgItemMessage (hWndDlg,IDC_FieldName,LB_ADDSTRING,0,(LPARAM)str);
				} 
				GlobalUnlock (SQLPtr->OFHandle);
				GlobalUnlock (hSQL);
                CloseDataFile (TRUE, &hSQL);
				*str = 0;
				OpenDataFile (pName,str,BT_READ,&hSQL);
              	break;
              	
              case IDOK: //next
				  rtn = TRUE;
				  switch (DataFileType)
				  {
				  default:
					pos = BT_NEXT;
				  break;
				  }
              
              case IDC_FIRST:
				  rtn = TRUE;
         		  if (DataFileType == 1)
				  {              
		    	      lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	                  st = BT_FIND (lpGWDHead->BTHandle[CurrentIndex],lpGWDHead->pKeys[CurrentIndex],pos,BT_ANY, (LPSTR)&SQLPtr->Offset);
	                  if (!st)
	                  {
	                  	if (pos == BT_FIRST)
	                    	CurRec = 1;
	                    else
	                    	CurRec++;
	                  }
					  GlobalUnlock (hDB);
	                  goto Display; 
                  }
Display2:
                  if (hSQL)
                  {  
                  	 
					 if (DataFileType == DBF_DATAFILE && LOWORD(wParam)==IDC_FIRST)
						CurrentDBFRec = 0;
			         SendDlgItemMessage (hWndDlg,IDC_FieldName,LB_RESETCONTENT,0,0);
				     if (FetchDBRec (hSQL))
				     {
				         SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
				         FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
				         for (i=0,pFldInfo = &FilePtr->FldInfo;
				             i<FilePtr->NumFields; i++,pFldInfo++)
				         {
				            _fstrcpy (str,pFldInfo->name);   
				            _fstrcat (str,"\t"); 
				            GetValFromOpenFiles (pFldInfo->name,str2,4096);
				            _fstrcat(str,str2);
				            SendDlgItemMessage (hWndDlg,IDC_FieldName,LB_ADDSTRING,0,(LPARAM)str);
				         } 
				         GlobalUnlock (SQLPtr->OFHandle);
				         GlobalUnlock (hSQL);
				     }
				     else
				     {
						EnableWindow(GetDlgItem(hWndDlg,IDOK),FALSE);
						//EnableWindow(GetDlgItem(hWndDlg,IDC_FIRST),FALSE);
					 }
			      } 
				  rtn = TRUE;
			      break;
                  
                  
              case IDC_LAST:
				  rtn = TRUE;
				  if (DataFileType == DBF_DATAFILE)
				  {
					CurrentDBFRec = NumDBFRecs-2;
					goto Display2;
				  }
				  else
				  {
		    		  lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
					  st = BT_FIND (lpGWDHead->BTHandle[CurrentIndex],lpGWDHead->pKeys[CurrentIndex],BT_LAST,BT_ANY, (LPSTR)&SQLPtr->Offset);
					  if (!st)
					  {
						 //GetBTHeader (lpGWDHead->BTHandle[CurrentIndex],&BTHead);  
						 CurRec = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[CurrentIndex]);
					  }
					  GlobalUnlock (hDB);
				  }
                  goto Display; 
                  
              case IDCANCEL: 
				  rtn = TRUE;
                  if (Processing)
                  {
                    ContinueProcessing=FALSE;   
                    break;
                  }
                  DestroyWindow (hWndDlg);
                  //GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
                  break; 
                  
              case IDC_CHANGE_FIELD_NAME: 
				  rtn = TRUE;
                  Choice=(short)SendDlgItemMessage(hWndDlg,IDC_FieldName,
                                            LB_GETCURSEL,0,0);
                  if (Choice<0)
                  {
                     GSSiMsgBox( GetFocus(),"No field selected",
                                "Error",MB_ICONEXCLAMATION,0); 
                     break;
                  } 
	    	      lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
                  lpGWFldInfo = lpGWDHead->pFldInfo;
                  lpGWFldInfo+=Choice;
                  SetDlgItemText(hWndDlg,IDC_NEW_FIELD_NAME,lpGWFldInfo->Name);
                  ShowWindow (GetDlgItem(hWndDlg,IDC_NEW_FIELD_NAME),SW_SHOW);
                  ShowWindow (GetDlgItem(hWndDlg,IDC_NEW_FIELD_CANCEL),SW_SHOW);
                  ShowWindow (GetDlgItem(hWndDlg,IDC_NEW_FIELD_COMMIT),SW_SHOW);
                  ShowWindow (GetDlgItem(hWndDlg,IDC_CHANGE_FIELD_NAME),SW_HIDE);
		    	  GlobalUnlock (hDB); 
                  break;
                  
              case IDC_NEW_FIELD_COMMIT:
              {   
                  GWFLDINFO NewField;
                  long  loc;
                   
				  rtn = TRUE;
				  GetDlgItemText(hWndDlg,IDC_NEW_FIELD_NAME,str,34);
                  if (!_fstrlen(str)) break;
                  Choice=(short)SendDlgItemMessage(hWndDlg,IDC_FieldName,
                                            LB_GETCURSEL,0,0);
                  if (Choice<0)
                  {
                     GSSiMsgBox( GetFocus(),"No field selected",
                                "Error",MB_OK,0); 
                     break;
                  } 
                  CloseGWDatabase (hDB);
                  pName = (LPSTR)GlobalLock(hNameLocal);
                  hDB = OpenGWDatabase (pName,BT_WRITE);
                  GlobalUnlock(hNameLocal);
                  if (!hDB)
                  {
                     GSSiMsgBox( GetFocus(),"Cannot open file for write access",
                                "Error",MB_OK,0); 
                     goto OpenDB;
                  }
                   
                  lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
                  loc = GSSillseek (lpGWDHead->Fid,sizeof(GWDHEADER16)+(Choice * sizeof(GWFLDINFO)),0);
                  BigRead (lpGWDHead->Fid,(HPSTR)&NewField,sizeof(GWFLDINFO));
                  if (_fstricmp (str,"just=right")) 
                    _fstrncpy (NewField.Name,str,32);   
                  else
                    NewField.Type = BT_RIGHT_CHAR;
                  GSSillseek (lpGWDHead->Fid,loc,0);
                  BigWrite (lpGWDHead->Fid,(HPSTR)&NewField,sizeof(GWFLDINFO),-1); 
                  
                  SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
                  FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
                  FilePtr->FileHandle = hDB;
                  GlobalUnlock (SQLPtr->OFHandle); 
                  GlobalUnlock (hSQL); 
                  GlobalUnlock (hDB);
                  ShowWindow (GetDlgItem(hWndDlg,IDC_NEW_FIELD_NAME),SW_HIDE);
                  ShowWindow (GetDlgItem(hWndDlg,IDC_NEW_FIELD_CANCEL),SW_HIDE);
                  ShowWindow (GetDlgItem(hWndDlg,IDC_NEW_FIELD_COMMIT),SW_HIDE);
                  ShowWindow (GetDlgItem(hWndDlg,IDC_CHANGE_FIELD_NAME),SW_SHOW);
                  goto OpenDB; 
              }
                  
              case IDC_NEW_FIELD_CANCEL:
				  rtn = TRUE;
				  ShowWindow (GetDlgItem(hWndDlg,IDC_NEW_FIELD_NAME),SW_HIDE);
                  ShowWindow (GetDlgItem(hWndDlg,IDC_NEW_FIELD_CANCEL),SW_HIDE);
                  ShowWindow (GetDlgItem(hWndDlg,IDC_NEW_FIELD_COMMIT),SW_HIDE);
                  ShowWindow (GetDlgItem(hWndDlg,IDC_CHANGE_FIELD_NAME),SW_SHOW);
                  break; 
              case IDC_EDIT_DIALOG_SCREEN: 
				  rtn = TRUE;
				  EditDynamicDialog (hWndDlg, 0,"",NULL);
                 break;
              case IDC_CREATE_DIALOG_SCREEN: 
              {
                  
				  rtn = TRUE;
				  nItems=(short)SendDlgItemMessage(hWndDlg,IDC_FieldName,LB_GETSELCOUNT,0,0);
                  if (!nItems)
                  {
                     GSSiMsgBox(GetFocus(),"No fields selected",
                                    "Error",MB_OK|MB_ICONEXCLAMATION,0);  
                     break;
                  }
                  else if(nItems > 180)
                  {
                     GSSiMsgBox(GetFocus(),"Exceeded Maximum Number of 180 Items",
                                    "Error",MB_OK|MB_ICONEXCLAMATION,0);  
                     break;
                  }
                  hItems=GSSiGlobAlloc ( 276,GHND,nItems*4);
                  lpItems=  (LPINT) GlobalLock(hItems);
                  SendDlgItemMessage(hWndDlg,IDC_FieldName,LB_GETSELITEMS,nItems,(LPARAM)lpItems);
                  pName = GlobalLock(hNameLocal);
//HERE I ADD THE STUFF REQUIRED BY THE DYNAMIC DIALOG PROCESSES
 
                  CreateDynamicDialog (pName,lpGWDHead, nItems, lpItems,
                                hWndDlg,IDC_FieldName, hSQL);
                  GlobalUnlock(hNameLocal);
                  GSSiGlobUlFree (&hItems);
                  break;
              }     
            //  case IDC_DUMP_FIELDS:     
              case IDC_CLOSE_DYNDLG:
				  rtn = TRUE;
#if WIN32
                  DestroyWindow((HWND)lParam);
#else
                  DestroyWindow(wParam);
#endif
                  break;
              
              case IDC_DUMPTOTXT:
              {
                  int   nItems, type;
                  LPINT lpItems;
                  HANDLE hItems;
                  HANDLE	hFile=GSSiGlobAlloc ( 277,GHND,256);
				  HCURSOR hcurSave;
                  LPSTR	pFile=GlobalLock (hFile); 
                  BOOL	GMHeader=FALSE;
                  
				  rtn = TRUE;
				  nItems=(short)SendDlgItemMessage(hWndDlg,IDC_FieldName,LB_GETSELCOUNT,0,0);
                  if (!nItems)
                  {
                     GSSiGlobUlFree (&hFile); 
                     GSSiMsgBox(GetFocus(),"No fields selected",
                                    "Error",MB_OK|MB_ICONEXCLAMATION,0); 
                     break;
                  }
                  if (!GetSaveName2 (hWndDlg,pFile,IDS_FILTERTEXT,".TXT",IDS_FILETXT))
                  {
                     GSSiGlobUlFree (&hFile); 
                 	 break;
                  }   
          	  	  *str = 0;
			 	  if (!OpenDataFile (pName,str,BT_READ,&hSQL))
		 			break; 
             	  if (GetSQLWhereClause (hWndDlg, hSQL, str)) 
             	  {   
		              if (!_fstricmp (str,"All Rows"))
		              	*str = 0;
		 		  }  
		 		  
                  CloseDataFile (TRUE, &hSQL); 
				  hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
                  hItems=GSSiGlobAlloc ( 278,GHND,(nItems+1)*4);
                  lpItems=  (LPINT) GlobalLock(hItems); 
                  *lpItems++ = nItems;
                  SendDlgItemMessage(hWndDlg,IDC_FieldName,LB_GETSELITEMS,nItems,(LPARAM)lpItems);
                  GlobalUnlock (hItems);   
                  if (GSSiMsgBox (hWndDlg,"Use standard header","",MB_YESNO,0) == IDNO)
                  	GMHeader = TRUE;
                  pName = GlobalLock(hNameLocal); 
                  ShowWindow (GetDlgItem(hWndDlg,IDC_CANCELEXPORT),SW_SHOW);   
                  Processing = TRUE;
				  OutputToFile (pFile,TRUE,pName, str, hItems,NULL,0,0,FALSE,FALSE,GMHeader,FALSE,FALSE,0,GetDlgItem(hWndDlg,IDC_STATUS),hWndDlg,TRUE);
				  SetContinueProcessing ( TRUE); 
				  Processing = FALSE;
                  ShowWindow (GetDlgItem(hWndDlg,IDC_CANCELEXPORT),SW_HIDE);
				  GSSiSetCursor (hcurSave); 
                  GlobalUnlock(hNameLocal);
                  GSSiGlobFree (&hItems);
                  GSSiGlobUlFree (&hFile); 
                  break; 
              }
              
              case IDC_DUMPTOGMD:
              {
				  rtn = TRUE;
				  int   nItems, type;
                  LPINT lpItems;
                  HANDLE hItems;
                  HANDLE	hFile=GSSiGlobAlloc ( 279,GHND,256);
				  HCURSOR hcurSave;
                  LPSTR	pFile=GlobalLock (hFile); 
                  BOOL	GMHeader=FALSE;
                  
                  nItems=(short)SendDlgItemMessage(hWndDlg,IDC_FieldName,LB_GETSELCOUNT,0,0); 
                  if (!nItems)
                  {
                     GSSiGlobUlFree (&hFile); 
                     GSSiMsgBox(GetFocus(),"No fields selected",
                                    "Error",MB_OK|MB_ICONEXCLAMATION,0); 
                     break;
                  }
                  if (!GetSaveName2 (hWndDlg,pFile,IDS_FILTERGWD,".GMD",IDS_FILEGMD))
                  {
                     GSSiGlobUlFree (&hFile); 
                 	 break;
                  }   
          	  	  *str = 0;
             	  if (GetSQLWhereClause (hWndDlg, hSQL, str)) 
             	  {   
             	  	  if (hSQL)
             	  	  {
		                  CloseDataFile (TRUE, &hSQL); 
		              }
		              if (!_fstricmp (str,"All Rows"))
		              	*str = 0;
			 		  if (!OpenDataFile (pName,str,BT_READ,&hSQL))
		 		  		break; 
		 		  }  
		 		  
				  hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
                  hItems=GSSiGlobAlloc ( 280,GHND,(nItems+1)*4);
                  lpItems=  (LPINT) GlobalLock(hItems); 
                  *lpItems++ = nItems;
                  SendDlgItemMessage(hWndDlg,IDC_FieldName,LB_GETSELITEMS,nItems,(LPARAM)lpItems);
                  GlobalUnlock (hItems);   
                  //if (GSSiMsgBox (hWndDlg,"Use standard header","",MB_YESNO) == IDNO)
                  GMHeader = 2;
                  pName = GlobalLock(hNameLocal); 
                  ShowWindow (GetDlgItem(hWndDlg,IDC_CANCELEXPORT),SW_SHOW);   
                  Processing = TRUE;
				  OutputToFile (pFile,TRUE,pName, str, hItems,NULL,0,0,FALSE,FALSE,GMHeader,FALSE,FALSE,0,GetDlgItem(hWndDlg,IDC_STATUS),hWndDlg,TRUE);
				  SetContinueProcessing ( TRUE); 
				  Processing = FALSE;
                  ShowWindow (GetDlgItem(hWndDlg,IDC_CANCELEXPORT),SW_HIDE);
				  GSSiSetCursor (hcurSave); 
                  GlobalUnlock(hNameLocal);
                  GSSiGlobFree (&hItems);
                  GSSiGlobUlFree (&hFile); 
                  break; 
              }
              
              case IDC_CANCELEXPORT:
				  rtn = TRUE;
				  SetContinueProcessing (FALSE);
              		break;
              			  
              case IDC_CREATE_REPORT: 
              {
                  int   nItems, type;
                  LPINT lpItems;
                  HANDLE hItems;
                  
				  rtn = TRUE;
				  nItems=(short)SendDlgItemMessage(hWndDlg,IDC_FieldName,LB_GETSELCOUNT,0,0);
                  if (!nItems)
                  {
                     GSSiMsgBox(GetFocus(),"No fields selected",
                                    "Error",MB_OK|MB_ICONEXCLAMATION,0);  
                     break;
                  }
                  hItems=GSSiGlobAlloc ( 281,GHND,nItems*4);
                  lpItems=  (LPINT) GlobalLock(hItems);
                  SendDlgItemMessage(hWndDlg,IDC_FieldName,LB_GETSELITEMS,nItems,(LPARAM)lpItems);
                  pName = GlobalLock(hNameLocal);   
                  if (wParam == IDC_CREATE_REPORT)
                    type = 1;
                  else
                    type = 2;
                  CreateReport (pName,lpGWDHead, nItems, lpItems,hWndDlg,IDC_FieldName, type,hSQL);
                  GlobalUnlock(hNameLocal);
                  GSSiGlobUlFree (&hItems);
                  break; 
              }
                  
              case IDC_INDEXES:
              {
               //   short   nItems, ibeg,i,j;
               //   HANDLE    hItems, hVars;
                //  LPSHORT     lpItems;
                //  BTVARDESC *pVars;
                //  LPGWDHEADER   lpGWDHead;
                //  LPGWFLDINFO   pFldInfo;
                                      
				  rtn = TRUE;
				  CloseDataFile (TRUE, &hSQL);
                   
                 {
                    DLGPROC lpfnGWD_INDEXESMsgProc;
                    short nRc;
                    hName = hNameLocal;        
                    lpfnGWD_INDEXESMsgProc = MakeProcInstance((DLGPROC)GWD_INDEXESMsgProc, hInst);
                    nRc = DialogBox(hInst, (LPSTR)"GWD_INDEXES", hWndMain, lpfnGWD_INDEXESMsgProc);
                    FreeProcInstance(lpfnGWD_INDEXESMsgProc); 
                    hName = 0;
                 }
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);

     
                  break;
            }
            break;
			  default:
				  rtn = FALSE;

           }
         break;    /* End of WM_COMMAND                                 */

    default: 
RtnFalse:
		GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (642);
#endif
//showmessage(__LINE__, __FILE__,Message);
	return rtn;
}
   }
 GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (642);
#endif

return  rtn;
}
#if ENABLETRACE
}
#endif
} 

    
BOOL FAR PASCAL GWD_INDEXESMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (643);
#endif
{ 
    LPSTR   pName, lpDot;
    static  HANDLE hDB;
    LPGWFLDINFO lpGWFldInfo;
    static  LPGWDHEADER lpGWDHead;
    static  HANDLE      hBT;
    static  GWDHEADER   SaveGWDHeader;
    short       i, index;
    static      short InIndex;  
    static      BOOL    OpenForWrite;
    char        str[300], IndexName[256];
    int         TabStops[2]={150,500};
    
    
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (643);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:  
         /* initialize working variables                                */  
         pName = GlobalLock(hName);
         GlobalUnlock(hName);   
         InIndex = CurrentIndex;
         
         hDB = OpenGWDatabase (pName,BT_READ);
         if (!hDB)
{
#if ENABLETRACE
GSSiExitProg (643);
#endif
         	return (FALSE);
}
         OpenForWrite = FALSE;
         lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);  
         SaveGWDHeader = *lpGWDHead;
         ShowWindow (GetDlgItem(hWndDlg,IDC_VARIABLES),SW_HIDE);
         ShowWindow (GetDlgItem(hWndDlg,IDC_ADDMES1),SW_HIDE);
         ShowWindow (GetDlgItem(hWndDlg,IDC_ADDMES2),SW_HIDE);
         ShowWindow (GetDlgItem(hWndDlg,IDC_ADDMES3),SW_HIDE);
SelectIndex:
         SendDlgItemMessage (hWndDlg,IDC_SELECT,LB_RESETCONTENT,0,0);
         SendDlgItemMessage (hWndDlg,IDC_SELECT,LB_ADDSTRING,0,(LPARAM)"Primary Index");
        
         for (i=1;i<lpGWDHead->NumIndex;i++)
         {
            sprintf(str,"Index %i",i+1);
            SendDlgItemMessage (hWndDlg,IDC_SELECT,LB_ADDSTRING,0,(LPARAM)str);
         }
         SendDlgItemMessage (hWndDlg,IDC_SELECT,LB_SETCURSEL,CurrentIndex,0);
         if (CurrentIndex)
             EnableWindow(GetDlgItem(hWndDlg,IDC_DELETE_INDEX),TRUE);
         else
             EnableWindow(GetDlgItem(hWndDlg,IDC_DELETE_INDEX),FALSE);
         if (lpGWDHead->lKeys[CurrentIndex]>0)
            SendDlgItemMessage (hWndDlg,IDC_UNIQUE,(UINT)BM_SETCHECK,TRUE,(LPARAM)0L);
         else
            SendDlgItemMessage (hWndDlg,IDC_UNIQUE,(UINT)BM_SETCHECK,FALSE,(LPARAM)0L);
         if (abs(lpGWDHead->lKeys[CurrentIndex]>9999))
            EnableWindow(GetDlgItem(hWndDlg,IDC_UNIQUE),TRUE);
         else        
            EnableWindow(GetDlgItem(hWndDlg,IDC_UNIQUE),FALSE);      
DisplayIndex:
         SendDlgItemMessage (hWndDlg,IDC_INDEX_DEF,LB_RESETCONTENT,0,0);
         for (i=0;i<abs(lpGWDHead->NumIndexFields[CurrentIndex]);i++)
         {
            lpGWFldInfo = lpGWDHead->pFldInfo;
            lpGWFldInfo+=lpGWDHead->IndexFields[CurrentIndex][i];
            SendDlgItemMessage (hWndDlg,IDC_INDEX_DEF,LB_ADDSTRING,0,(LPARAM)lpGWFldInfo->Name);
         }
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
#if WIN32
        switch(LOWORD(wParam))
#else
        switch(wParam)
#endif
           {
           
              case IDOK:
                 GlobalUnlock (hDB);
                 CloseGWDatabase (hDB);
                 if (OpenForWrite)
                 {
                     pName = GlobalLock(hName);
                     hDB = OpenGWDatabase (pName,BT_WRITE);
                     GlobalUnlock(hName);
                     if (!hDB)
                     {
                         GSSiMsgBox( GetFocus(),"Cannot open file for write access",
                                    "Error",MB_OK,0); 
                         EndDialog(hWndDlg, FALSE);
                         break;
                     }
                     CloseGWDatabase (hDB);
                  }
                  EndDialog(hWndDlg, TRUE); 
                  break;      
                  
              case IDCANCEL:
                  *lpGWDHead = SaveGWDHeader;   
                  CurrentIndex = InIndex;
                  GlobalUnlock (hDB);
                  CloseGWDatabase (hDB);
                  EndDialog(hWndDlg, FALSE);
                  break;  
                  
              case IDC_UNIQUE:    
                  if (SendDlgItemMessage (hWndDlg,IDC_UNIQUE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
                      lpGWDHead->lKeys[CurrentIndex]=SHRT_MAX;
                  else
                      lpGWDHead->lKeys[CurrentIndex]=SHRT_MIN;   
                  break;
                  
              case IDC_SELECT:
              {
              switch(HIWORD(wParam))
                    {
                     case LBN_DBLCLK:
                     case LBN_SELCHANGE:
                         CurrentIndex=SendDlgItemMessage(hWndDlg,IDC_SELECT,
                                                         LB_GETCURSEL,0,0); 
                         ShowWindow (GetDlgItem(hWndDlg,IDC_VARIABLES),SW_HIDE);
                         ShowWindow (GetDlgItem(hWndDlg,IDC_ADDMES1),SW_HIDE);
                         ShowWindow (GetDlgItem(hWndDlg,IDC_ADDMES2),SW_HIDE);
                         ShowWindow (GetDlgItem(hWndDlg,IDC_ADDMES3),SW_HIDE);
                         goto SelectIndex;
                         break;
                    }
               }
                 break; 
                 
              case IDC_VARIABLES:
              {
              switch(HIWORD(wParam))
                    {
                     case LBN_DBLCLK:
                     case LBN_SELCHANGE:
                         lpGWDHead->IndexFields[CurrentIndex][lpGWDHead->NumIndexFields[CurrentIndex]]=
                            SendDlgItemMessage(hWndDlg,IDC_VARIABLES,LB_GETCURSEL,0,0); 
                         lpGWDHead->NumIndexFields[CurrentIndex]++;
                         goto DisplayIndex;
                         break;
                    }
               }
                 break; 
                 
              case IDC_DELETE_INDEX:
              {
                 char   OldName[256];  
                 short    j;
                   
                 if (!CurrentIndex)
                 {
                     GSSiMsgBox( GetFocus(),"Cannot delete the primary index",
                                "Error",MB_OK,0); 
                     EndDialog(hWndDlg, FALSE);
                     break;
                 }
                 pName = GlobalLock(hName);
                 GlobalUnlock(hName);
                 if (!OpenForWrite)
                 {
                     GlobalUnlock (hDB);
                     CloseGWDatabase (hDB);
                     hDB = OpenGWDatabase (pName,BT_WRITE);
                     if (!hDB)
                     {
                         GSSiMsgBox( GetFocus(),"Cannot open file for write access",
                                    "Error",MB_OK,0); 
                         EndDialog(hWndDlg, FALSE);
                         break;
                     }
                     lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);  
                     OpenForWrite = TRUE;  
                 }
                 BT_CLOSE(lpGWDHead->BTHandle[CurrentIndex]); 
                 GSSiGlobUlFree (&lpGWDHead->hKeys[CurrentIndex]);
                 _fstrcpy (IndexName,pName);
                 _fstrlwr (IndexName); 
                 lpDot = _fstrstr (IndexName,".gmd");
                 if (!lpDot) lpDot = _fstrstr (IndexName,".gwd");
                 _fstrcpy (lpDot,".in");
                 itoa (CurrentIndex+1,_fstrchr(IndexName,'\0'),10); 
                 GSSiRemove(IndexName); 
				 if (lpGWDHead->SpatialIndex == CurrentIndex)
					 lpGWDHead->SpatialIndex = 0;
                 for (index=CurrentIndex;index<lpGWDHead->NumIndex-1;index++)
                 {
                     lpGWDHead->NumIndexFields[index]=lpGWDHead->NumIndexFields[index+1]; 
                     lpGWDHead->lKeys[index] = lpGWDHead->lKeys[index+1];
                     lpGWDHead->hKeys[index] = lpGWDHead->hKeys[index+1];
                     lpGWDHead->pKeys[index] = lpGWDHead->pKeys[index+1];
                     lpGWDHead->BTHandle[index] = lpGWDHead->BTHandle[index+1];   
                     BT_CLOSE(lpGWDHead->BTHandle[index]);
                     _fstrcpy (OldName,IndexName);
                     lpDot = _fstrstr (OldName,".in");
                     lpDot+=3;
                     *lpDot = '\0';
                     itoa (index+2,_fstrchr(OldName,'\0'),10); 
                     GSSiRename(OldName,IndexName);
                     lpGWDHead->BTHandle[index] = BT_OPEN (IndexName, lpGWDHead->TimeStamp, BT_WRITE, 0);
                     for (j=0;j<MAX_GMD_INDEX_FIELDS;j++) lpGWDHead->IndexFields[index][j] = lpGWDHead->IndexFields[index+1][j]; 
                     _fstrcpy(IndexName,OldName);
                 }
                 lpGWDHead->NumIndex--; 
                 if (CurrentIndex>=lpGWDHead->NumIndex) CurrentIndex--;
                 GSSillseek(lpGWDHead->Fid,0,0);
				 if (lpGWDHead->StoredAs32)
					BigWrite (lpGWDHead->Fid,(HPSTR)lpGWDHead ,sizeof(GWDHEADER),-1);
				 else
				 {
					GWDHead16 = GWDHEADER32toGWDHEADER16 (lpGWDHead);
					BigWrite (lpGWDHead->Fid,(HPSTR)&GWDHead16 ,sizeof(GWDHEADER16),-1);
				 }
// 				 GWDHead16 = GWDHEADER32toGWDHEADER16 (lpGWDHead);
//				 BigWrite (lpGWDHead->Fid,(HPSTR)&GWDHead16 ,sizeof(GWDHEADER16),-1);
             
                 GlobalUnlock (hDB);
                 CloseGWDatabase (hDB);
                 EndDialog(hWndDlg, TRUE); 
                 break;
              }   
              case IDC_ADD_INDEX: 
                 if (!OpenForWrite)
                 {
                     GlobalUnlock (hDB);
                     CloseGWDatabase (hDB);
                     pName = GlobalLock(hName);
                     hDB = OpenGWDatabase (pName,BT_WRITE);
                     GlobalUnlock(hName);
                     if (!hDB)
                     {
                         GSSiMsgBox( GetFocus(),"Cannot open file for write access",
                                    "Error",MB_OK,0); 
                         EndDialog(hWndDlg, FALSE);
                         break;
                     }
                     lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);  
                     OpenForWrite = TRUE;
                 }  
                 CurrentIndex = lpGWDHead->NumIndex; 
                 pName = GlobalLock(hName);
                 _fstrcpy (IndexName,pName);
                 GlobalUnlock(hName);
                 _fstrlwr (IndexName); 
                 lpDot = _fstrstr (IndexName,".gmd");
                 if (lpDot)
                 {
                 	sprintf (++lpDot,"in%i",CurrentIndex+1); 
                 	GSSiRemove(IndexName);
                 } 
                 lpGWDHead->NumIndex++;
                 lpGWDHead->NumIndexFields[CurrentIndex]=0; 
                 lpGWDHead->lKeys[CurrentIndex]=SHRT_MAX;
                 ShowWindow (GetDlgItem(hWndDlg,IDC_VARIABLES),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ADDMES1),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ADDMES2),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ADDMES3),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ADD_INDEX),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_DELETE_INDEX),SW_HIDE);
                 SendDlgItemMessage (hWndDlg,IDC_VARIABLES,LB_RESETCONTENT,0,0);
                 for (i=0,lpGWFldInfo = lpGWDHead->pFldInfo;
                      i<lpGWDHead->NumFields; i++,lpGWFldInfo++)
                     SendDlgItemMessage (hWndDlg,IDC_VARIABLES,LB_ADDSTRING,0,(LPARAM)lpGWFldInfo->Name); 
                 goto SelectIndex;
                 break;
              
                  

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (643);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (643);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

void CreateBinaryNonUniqueKey (LPGWDHEADER lpGWDHead,LPSTR loc)
{
	LPGWFLDINFO pFldInfo; 
	LPSTR		lpVal;   
	short	i;
	
	if (lpGWDHead->NonUniqueSortedBinary)
	{
	    for (i=0;i<abs(lpGWDHead->NumIndexFields[0]);i++) 
	    {
	        pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[0][i];
        	lpVal = &loc[pFldInfo->Beg]; 
	        switch (pFldInfo->Type)
	        {
	        	default:
	        		break;
	        	case BT_INTEGER:
	        		flip (lpVal,pFldInfo->Len);
	        		break;
	        }
	    } 
	}
	return;
}

int GridFromPoint (LPGWDHEADER lpGWDHead,LPDPOINT pPoint)
{
	int Grid=0;
	int	Gridx, Gridy;

	if (WantGMDNegGrid)
		return -1;
	if (pPoint->x < lpGWDHead->GridBounds.xmn || pPoint->x > lpGWDHead->GridBounds.xmx)
		return -1;
	if (pPoint->y < lpGWDHead->GridBounds.ymn || pPoint->y > lpGWDHead->GridBounds.ymx)
		return -1;
	Gridx = (pPoint->x - lpGWDHead->GridBounds.xmn) / lpGWDHead->GridXInc;
	Gridx = min (Gridx,lpGWDHead->GridInc);
	Gridy = (pPoint->y - lpGWDHead->GridBounds.ymn) / lpGWDHead->GridYInc;
	Gridy = min (Gridy,lpGWDHead->GridInc);

	Grid = Gridy * lpGWDHead->GridInc + Gridx;
	return Grid;
}

int SysMonthFromSymTime (time_t systime)
{
	struct	tm	tmtime;
	int		Month=-1;
	
	if (systime >= 0)
	{
		tmtime = *localtime (&systime);
		Month = tmtime.tm_year*12 + tmtime.tm_mon;
	}
	return Month;
}


BOOL GMDGetFileMinMax (LPSTR FileName,LPMNMXCORD pBounds,LPINT pMinTime,LPINT pMaxTime)
{
	LPGWDHEADER lpGWDHead;
	int	nRecs, nLoaded=0;
	long	Offset;
	HANDLE	hDB = OpenGWDatabase (FileName,BT_READ);
	BOOL	ContinueProcessing;
	DPOINT	DPoint;
	BOOL	rtn = FALSE;
	int		pos=BT_FIRST, len;
    LPGWFLDINFO pFldInfo;  
	long	Time;

	*pMinTime = LONG_MAX;
	*pMaxTime = 0;
	DBoundsInit (pBounds);

	if (hDB)
	{
		lpGWDHead = GlobalLock (hDB);
		CreateStatusWind (hWndMain,1,"Getting File MinMax"); 
		nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
		while ((ContinueProcessing= StatusWindowUpdate(NULL, NULL, nRecs, ++nLoaded)) && !BT_FIND(lpGWDHead->BTHandle[0], lpGWDHead->pKeys[0], pos, BT_ANY, (LPSTR)&Offset))
		{   
    		pos = BT_NEXT;
			len = FillGWDData (lpGWDHead,Offset);
			pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->XField;
			memmove (&DPoint.x,&lpGWDHead->GWDData[pFldInfo->Beg],8); 
			pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->YField;
			memmove (&DPoint.y,&lpGWDHead->GWDData[pFldInfo->Beg],8); 
			AddDPointToMinMax (&DPoint,pBounds);
			if (lpGWDHead->FromDateField)
			{
				pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->FromDateField;
				Time = *(LPLONG)&lpGWDHead->GWDData[pFldInfo->Beg];
				if (Time > 0)
					*pMinTime = min (Time,*pMinTime);
				if (lpGWDHead->ToDateField)
				{
					pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->ToDateField;
					Time = *(LPLONG)&lpGWDHead->GWDData[pFldInfo->Beg];
					if (Time > 0)
						*pMaxTime = max (Time,*pMaxTime);
				}
			}
		}
		if (ContinueProcessing)
		{
			GlobalUnlock (hDB);
			CloseGWDatabase (hDB); 
			ProcessText ("[%ALLOWJOURNAL]=T");
			hDB = OpenGWDatabase (FileName,BT_WRITE);
			if (hDB)
			{
				lpGWDHead = GlobalLock (hDB);
				lpGWDHead->FileBounds = *pBounds;
				if (*pMinTime < LONG_MAX)
				{
					lpGWDHead->MinTime = *pMinTime;
					lpGWDHead->MaxTime = *pMaxTime;
				}
				rtn = TRUE; 
			}
		}
		GlobalUnlock (hDB);
		CloseGWDatabase (hDB); 
		ProcessText ("[%ALLOWJOURNAL]=F");
		DestroyStatusWindow(0);  
	}
	return rtn;
}

BOOL GMDUpdateMinMax (LPGWDHEADER lpGWDHead,LPDPOINT pDPoint)
{
	BOOL rtn=FALSE;
	long	Time;
    LPGWFLDINFO pFldInfo;  

	AddDPointToMinMax (pDPoint,&lpGWDHead->FileBounds);
	if (lpGWDHead->FromDateField)
	{
		pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->FromDateField;
		Time = *(LPLONG)&lpGWDHead->GWDData[pFldInfo->Beg];
		if (Time > 0)
			lpGWDHead->MinTime = min (Time,lpGWDHead->MinTime);
		if (lpGWDHead->ToDateField)
		{
			pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->ToDateField;
			Time = *(LPLONG)&lpGWDHead->GWDData[pFldInfo->Beg];
			if (Time > 0)
				lpGWDHead->MaxTime = max (Time,lpGWDHead->MaxTime);
		}
		rtn = TRUE;
	}

	return rtn;
}

BOOL GMDKeyListAdd(LPSTR key)
{
	BOOL rtn = FALSE;
	static UINT iSeq = 0;
	HANDLE hKey = BT_FormKey(hGMDKeyList,key);
	LPSTR  pKey = GlobalLock(hKey);

	BT_PUT(hGMDKeyList, pKey,(LPSTR) &iSeq);
	iSeq++;
	GSSiGlobUlFree(&hKey);

	return rtn;
}

BOOL GMDCreateKeyList(LPSTR FileName)
{
	BOOL rtn = FALSE;
	HANDLE	hDB = OpenGWDatabase(FileName, BT_READ);

	BT_CLOSEANDDELETE(&hGMDKeyList);
	if (hDB)
	{
		LPGWDHEADER lpGWDHead = GlobalLock(hDB);
		int NumIndexFields = lpGWDHead->NumIndexFields[0];
		//GWFLDINFO GWFldInfo;
		LPGWFLDINFO pFldInfo;
		//LPGWFLDINFO	lpGWFldInfo;
		//LPGWFLDINFO	lpFieldInfo;
		short	i, ifield;
		UINT	ibeg;
		HANDLE	hVars;
		LPBTVARDESC pVars;
		char	tmpName[MAX_PATH];

		GSSiGetTempFileName(0, "gm", 0, (LPSTR)tmpName);
		hVars = LocalAlloc(LHND, NumIndexFields * sizeof(BTVARDESC));
		pVars = (BTVARDESC *)LocalLock(hVars);
		ibeg = 0;
		for (i = 0; i<NumIndexFields; i++, pVars++)
		{
			pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[0][i];
			pVars->BT_VARLEN = pFldInfo->Len;
			pVars->BT_VARTYP = pFldInfo->Type;
			pVars->BT_VAROFF = ibeg;
			ibeg += pFldInfo->Len;
		}
		LocalUnlock(hVars);
		pVars = (BTVARDESC *)LocalLock(hVars);
		BT_CREATE(tmpName, 4, FALSE, NumIndexFields, 1, pVars, FALSE, 0, lpGWDHead->TimeStamp, FALSE);
		LocalUnlock(hVars);
		LocalFree(hVars);

		GlobalUnlock(hDB);
		CloseGWDatabase(hDB);
		hGMDKeyList = BT_OPEN(tmpName, lpGWDHead->TimeStamp, BT_WRITE, 0);
		GMDKeyListPos = BT_FIRST;
		rtn = TRUE;
	}
	return rtn;
}

BOOL GWDFormKey (LPGWDHEADER lpGWDHead, int Index, BOOL Search,long length,int Month)
#if ENABLETRACE
{GSSiEnterProg (644);
#endif
{   LPSTR   loc, lpVal;
    short   ibeg, i, Grid;
    LPGWFLDINFO pFldInfo;  
    //BTHEAD  BTHead;
	DPOINT	DPoint;
    
    //GetBTHeader (lpGWDHead->BTHandle[0],&BTHead); 
    ibeg = 0;
	if (Index && Index == lpGWDHead->SpatialIndex)
	{
		switch (lpGWDHead->SpatialIndexType)
		{
		case 1:	//Month,Grid
			{
				LPSPATIALINDEXTYPE1	pSIIndex1 = (LPSPATIALINDEXTYPE1)lpGWDHead->pKeys[lpGWDHead->SpatialIndex];

        		pSIIndex1->Month = Month;   
				pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->XField;
				memmove (&DPoint.x,&lpGWDHead->GWDData[pFldInfo->Beg],8); 
				pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->YField;
				memmove (&DPoint.y,&lpGWDHead->GWDData[pFldInfo->Beg],8); 
				GMDUpdateMinMax (lpGWDHead,&DPoint);
				pSIIndex1->Grid = GridFromPoint (lpGWDHead,&DPoint);
				if (Search)
					_fmemset(&pSIIndex1->PrimeIndex,'\0',lpGWDHead->lKeys[0]);         
				else
				{
					pFldInfo = lpGWDHead->pFldInfo;
					lpVal = &lpGWDHead->GWDData[pFldInfo->Beg];
					_fmemmove(&pSIIndex1->PrimeIndex,lpVal,abs(lpGWDHead->lKeys[0])); 
					CreateBinaryNonUniqueKey (lpGWDHead,(LPSTR)&pSIIndex1->PrimeIndex);
				}
			}
			break;
		case 2:	//Month,Code,Grid
			{
				LPSPATIALINDEXTYPE2	pSIIndex2 = (LPSPATIALINDEXTYPE2)lpGWDHead->pKeys[lpGWDHead->SpatialIndex];

        		pSIIndex2->Month = Month; 
				pSIIndex2->Code = GMDGetIntegerFieldVal (lpGWDHead,lpGWDHead->SymbolField);
				pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->XField;
				memmove (&DPoint.x,&lpGWDHead->GWDData[pFldInfo->Beg],8); 
				pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->YField;
				memmove (&DPoint.y,&lpGWDHead->GWDData[pFldInfo->Beg],8); 
				//GMDUpdateMinMax(lpGWDHead, &DPoint);
				pSIIndex2->Grid = GridFromPoint(lpGWDHead, &DPoint);
				if (pSIIndex2->Grid < 0)
					return FALSE;
				if (Search)
					pSIIndex2->PrimeIndex = LONG_MIN;         
				else
				{
					pFldInfo = lpGWDHead->pFldInfo;
					lpVal = &lpGWDHead->GWDData[pFldInfo->Beg];
					memmove(&pSIIndex2->PrimeIndex,lpVal,4); 
				}
			}
			break;
		}
	}
	else
	{
		for (i=0;i<abs(lpGWDHead->NumIndexFields[Index]);i++) 
		{ 
			loc = lpGWDHead->pKeys[Index] + ibeg;
			pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[Index][i];
			if (length && (pFldInfo->Beg + pFldInfo->Len) > length)
	{
#if ENABLETRACE
GSSiExitProg (644);
#endif
        		return FALSE;
	}
			lpVal = &lpGWDHead->GWDData[pFldInfo->Beg]; 
			switch (pFldInfo->Type)
			{
        		default:
        			_fmemmove(loc,lpVal,pFldInfo->Len);   
        			break;
        		case BT_CHAR:
        			_fstrncpy (loc,lpVal,pFldInfo->Len);
        			break;
			}
			ibeg += pFldInfo->Len;
		}
		if (lpGWDHead->lKeys[Index]<0)
		{    
			loc = lpGWDHead->pKeys[Index] + ibeg;
			if (Search)
				_fmemset(loc,'\0',lpGWDHead->lKeys[0]);         
			else
			{
				pFldInfo = lpGWDHead->pFldInfo;
				lpVal = &lpGWDHead->GWDData[pFldInfo->Beg];
				_fmemmove(loc,lpVal,abs(lpGWDHead->lKeys[0])); 
				CreateBinaryNonUniqueKey (lpGWDHead,loc);
			}
		}
	}
                 
{
#if ENABLETRACE
GSSiExitProg (644);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}
 
LPFIELDINFO GetFieldInfo (HANDLE TBLHandle, BOOL First, short FieldType,LPBOOL pHaveNonStandardFields)
#if ENABLETRACE
{GSSiEnterProg (645);
#endif
{   static icount;
    static FIELDINFO    Finfo;
    LPFIELDINFO lpFinfo=&Finfo;
    LPGWDHEADER lpGWDHead;
    LPGWFLDINFO lpGWFldInfo;
	LPCOMBOHEADER		pComboHeader;
    LPCOMBOFILE  		pComboFile; 
    LPFIELDINFO         pField;
	LPSHORT	pnDLTvar;
	short	nDLTvar;
	LPHANDLE	DLTVar;   
	VARPNT	VarPtr;  
	LPSTR	DLTDelim;   
	LPSHORT	DLTStart, DLTLen,DLTType;
	static	nFields;
	LPSTR	pDot;

    if (First)
        icount=-1;
    icount++;
    if (FieldType == GMTEXT_DATAFILE)
    {
        pnDLTvar = (LPSHORT)GlobalLock (TBLHandle); 
        nDLTvar = abs (*pnDLTvar); 
        DLTDelim = (LPSTR)(pnDLTvar + 1);
		DLTVar = (LPHANDLE)(DLTDelim + 1);
		DLTStart = (LPSHORT)(DLTVar + MAXDLTVAR);
		DLTLen = (LPSHORT)(DLTStart + MAXDLTVAR);
		DLTType = (LPSHORT)(DLTLen + MAXDLTVAR);  
        if (icount >= nDLTvar)
        {
			GlobalUnlock (TBLHandle);
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        	return (0);
}
        }
		VarPtr = (VARPNT)GlobalLock (DLTVar[icount]);
		if ((pDot = strchr(VarPtr->Name, '.')))
			pDot++;
		else
			pDot = VarPtr->Name;
        _fstrcpy (lpFinfo->name,pDot); 
        if (DLTType[icount])
        {
	        lpFinfo->type = DLTType[icount];  
        	lpFinfo->length = DLTLen[icount];
        }
        else
        { 
	        lpFinfo->type = BT_CHAR;  
	        if (DLTLen[icount] > 0)
	        	lpFinfo->length = DLTLen[icount];
	        else
	        	lpFinfo->length = 255;
	    } 
        GlobalUnlock (DLTVar[icount]);
        GlobalUnlock (TBLHandle);
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        return (lpFinfo);
}
    }
    if (FieldType == UMIFS_DATAFILE || FieldType == ORA_DATAFILE || FieldType == PN_DATAFILE)
    {
        lpGWDHead = (LPGWDHEADER)GlobalLock (TBLHandle);
        if (icount >= lpGWDHead->NumFields)
        {
			GlobalUnlock (TBLHandle);
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        	return (0);
}
        }
        lpGWFldInfo = lpGWDHead->pFldInfo;
        lpGWFldInfo += icount;
        _fstrcpy (lpFinfo->name,lpGWFldInfo->Name);  
        lpFinfo->type = lpGWFldInfo->Type;
        lpFinfo->length = lpGWFldInfo->Len;
        GlobalUnlock (TBLHandle);
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        return (lpFinfo);
}
    }
    if (FieldType == IMAGE_DATAFILE)
    {
        if (icount >= 7)
        {
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        	return (0);
}
        }
		switch (icount)
		{
		case 0:
			strcpy (lpFinfo->name,"WIDTH");
			lpFinfo->type = BT_INTEGER;
			lpFinfo->length = 4;
			break;
		case 1:
			strcpy (lpFinfo->name,"HEIGHT");
			lpFinfo->type = BT_INTEGER;
			lpFinfo->length = 4;
			break;
		case 2:
			strcpy (lpFinfo->name,"BITSPERPIXEL");
			lpFinfo->type = BT_INTEGER;
			lpFinfo->length = 2;
			break;
		case 3:
			strcpy (lpFinfo->name,"X");
			lpFinfo->type = BT_REAL;
			lpFinfo->length = 8;
			break;
		case 4:
			strcpy (lpFinfo->name,"Y");
			lpFinfo->type = BT_REAL;
			lpFinfo->length = 8;
			break;
		case 5:
			strcpy (lpFinfo->name,"DATETAKEN");
			lpFinfo->type = BT_CHAR;
			lpFinfo->length = 20;
			break;
		case 6:
			strcpy (lpFinfo->name,"AZIMUTH");
			lpFinfo->type = BT_REAL;
			lpFinfo->length = 8;
			break;
		}
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        return (lpFinfo);
}
	}
    if (FieldType == HLTLIST_DATAFILE)
    {
		//BTHEAD BTHead;
		
		//GetBTHeader (TBLHandle,&BTHead);   
        if (icount >= 4)
        {
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        	return (0);
}
        }
		switch (icount)
		{
		case 0:
			strcpy (lpFinfo->name,"REFNO");
			lpFinfo->type = BT_INTEGER;
			lpFinfo->length = 4;
			break;
		case 1:
			strcpy (lpFinfo->name,"SYMBOLNUM");
			lpFinfo->type = BT_INTEGER;
			lpFinfo->length = 2;
			break;
		case 2:
			strcpy (lpFinfo->name,"PREFIX");
			lpFinfo->type = BT_CHAR;
			lpFinfo->length = 8;
			break;
		case 3:
			strcpy (lpFinfo->name,"UDI");
			lpFinfo->type = BT_CHAR;
			lpFinfo->length = 64;
			break;
		}
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        return (lpFinfo);
}
	}
    if (FieldType == THEME_HLTFILE)
    {
		//BTHEAD BTHead;
		
		//GetBTHeader (TBLHandle,&BTHead);   
        if (icount >= 7)
        {
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        	return (0);
}
        }
		switch (icount)
		{
		case 0:
			strcpy (lpFinfo->name,"REFNO");
			lpFinfo->type = BT_INTEGER;
			lpFinfo->length = 4;
			break;
		case 1:
			strcpy (lpFinfo->name,"SYMBOLNUM");
			lpFinfo->type = BT_INTEGER;
			lpFinfo->length = 2;
			break;
		case 2:
			strcpy (lpFinfo->name,"PREFIX");
			lpFinfo->type = BT_CHAR;
			lpFinfo->length = 8;
			break;
		case 3:
			strcpy (lpFinfo->name,"UDI");
			lpFinfo->type = BT_CHAR;
			lpFinfo->length = 64;
			break;
		case 4:
			strcpy (lpFinfo->name,"CLASSNO");
			lpFinfo->type = BT_INTEGER;
			lpFinfo->length = 2;
			break;
		case 5:
			strcpy (lpFinfo->name,"X");
			lpFinfo->type = BT_REAL;
			lpFinfo->length = 8;
			break;
		case 6:
			strcpy (lpFinfo->name,"Y");
			lpFinfo->type = BT_REAL;
			lpFinfo->length = 8;
			break;
		}
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        return (lpFinfo);
}
    }
	if (FieldType == SQL_DATAFILE)
	{
		LPSQLDATABASE pSQLDatabase = (LPSQLDATABASE)GlobalLock(TBLHandle);
		if (icount >= pSQLDatabase->NumFields)
		{
			GlobalUnlock(TBLHandle);
			{
#if ENABLETRACE
				GSSiExitProg (645);
#endif
				return (0);
			}
		}
		pField = &pSQLDatabase->FldInfo[0];
		pField += icount;
		*lpFinfo = *pField;
		GlobalUnlock(TBLHandle);
		{
#if ENABLETRACE
			GSSiExitProg (645);
#endif
			return (lpFinfo);
		}
	}
	if (FieldType == SLT_DATAFILE)
	{
		LPSQLDATABASE pSQLDatabase = (LPSQLDATABASE)GlobalLock(TBLHandle);
		if (icount >= pSQLDatabase->NumFields)
		{
			GlobalUnlock(TBLHandle);
			{
#if ENABLETRACE
				GSSiExitProg(645);
#endif
				return (0);
			}
		}
		pField = &pSQLDatabase->FldInfo[0];
		pField += icount;
		*lpFinfo = *pField;
		GlobalUnlock(TBLHandle);
		{
#if ENABLETRACE
			GSSiExitProg(645);
#endif
			return (lpFinfo);
		}
	}
	if (FieldType == FGDB_DATAFILE)
    {
		int FieldWidth, FieldType;
		
		if (First)
			nFields = FGDBGetFieldCount((int)TBLHandle);
        if (icount >= nFields)
        {
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        	return (0);
}
        }
    	FGDBGetFieldInfo((int)TBLHandle, icount, lpFinfo->name, &FieldWidth,&FieldType);
	    lpFinfo->length = FieldWidth;
	    lpFinfo->type = FieldType;
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        return (lpFinfo);
}
    }
    if (FieldType == SHAPE_DATAFILE || FieldType == DBF_DATAFILE)
    {
		int		FieldWidth, FieldDecimals;
		DBFHandle    pDBF;
		LPDWORD	pDBFAddress = (LPDWORD)GlobalLock (TBLHandle);
        
        pDBF = (DBFHandle)*pDBFAddress;
        GlobalUnlock (TBLHandle);
        if (icount >= DBFGetFieldCount(pDBF))
        {
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        	return (0);
}
        }
    	DBFGetFieldInfo(pDBF, icount, lpFinfo->name, &FieldWidth,&FieldDecimals );
        if (FieldDecimals)
        {
	        lpFinfo->length = 8;
	        lpFinfo->type = BT_REAL;
        }
        else
        {
	        lpFinfo->length = FieldWidth;
	        lpFinfo->type = BT_CHAR;
	    } 
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        return (lpFinfo);
}
    }
    if (FieldType == GMCENSUS_DATAFILE)
    {
        lpGWDHead = (LPGWDHEADER)GlobalLock (TBLHandle);
        if (icount >= lpGWDHead->NumFields)
        {
			GlobalUnlock (TBLHandle);
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        	return (0);
}
        }
        lpGWFldInfo = lpGWDHead->pFldInfo;
        lpGWFldInfo += icount;
        _fstrcpy (lpFinfo->name,lpGWFldInfo->Name);  
        lpFinfo->type = lpGWFldInfo->Type;
        lpFinfo->length = lpGWFldInfo->Len;
        GlobalUnlock (TBLHandle);
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        return (lpFinfo);
}
    }
    if (FieldType == DTM_DATAFILE)
    {
/*		LPDTMINFO	pDTMInfo = (LPDTMINFO)GlobalLock (TBLHandle);
		
        lpGWDHead = (LPGWDHEADER)GlobalLock (pDTMInfo->hDB); 
        if (icount >= lpGWDHead->NumFields)*/  
        if (icount > 1)
{
//		GlobalUnlock (pDTMInfo->hDB);
//		GlobalUnlock (TBLHandle);
#if ENABLETRACE
GSSiExitProg (645);
#endif
        	return (0);
}
//        lpGWFldInfo = lpGWDHead->pFldInfo;
//        lpGWFldInfo += icount;
//        _fstrcpy (lpFinfo->name,lpGWFldInfo->Name);
		if (icount == 0)
			_fstrcpy (lpFinfo->name,"ELEVATION");
		else
			_fstrcpy (lpFinfo->name,"PERCENTSLOPE");
        lpFinfo->type = BT_REAL;
        lpFinfo->length = 8;  
//        GlobalUnlock (pDTMInfo->hDB);
//        GlobalUnlock (TBLHandle);
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        return (lpFinfo);
}
    }
    if (FieldType == LISTVAR_DATAFILE)
    {
		LPLISTVARDATABASE pDB = (LPLISTVARDATABASE)GlobalLock (TBLHandle);
        if (icount >= pDB->NumFields)
        {
			GlobalUnlock (TBLHandle);
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        	return (0);
}
        }
		switch (icount)
		{
		case 0:
			*lpFinfo = pDB->FldInfo[0];
			break;
		}
		GlobalUnlock (TBLHandle);
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        return (lpFinfo);
}
	}
    else if (FieldType == COMBO_DATAFILE)
    {
		pComboHeader = (LPCOMBOHEADER)GlobalLock (TBLHandle);
		pComboFile = (LPCOMBOFILE)GlobalLock (pComboHeader->hComboFile);
        if (icount >= pComboFile->NumFields)
        {
			GlobalUnlock (pComboHeader->hComboFile);
	        GlobalUnlock (TBLHandle);
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        	return (0);
}
        }
        pField = pComboFile->FldInfo;
        pField += icount;
        _fstrcpy (lpFinfo->name,pField->name);  
        lpFinfo->type = pField->type;
        lpFinfo->length = pField->length;
		GlobalUnlock (pComboHeader->hComboFile);
        GlobalUnlock (TBLHandle);
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        return (lpFinfo);
}
    }
    else
{
#if ENABLETRACE
GSSiExitProg (645);
#endif
        return (GetExternalFieldInfo (TBLHandle,First,pHaveNonStandardFields));
}
#if ENABLETRACE
}
#endif
}

BOOL CreateReport (LPSTR DBName, LPGWDHEADER lpGWDHead, int NumSelect, LPINT Selected,
                    HWND hWndDlg, UINT DlgItem, int type,HGLOBAL hSQL)
#if ENABLETRACE
{GSSiEnterProg (646);
#endif
{   
    short j;
    LPSTR  lpBar;
    HFILE Fid; 
    LPINT   pSelected;
    OFSTRUCTGM    OFStruct;
    char    str[256],str2[64], FieldName[300];
    char    File[256], Title[128];
    
    *File = 0;
    if (!GetSaveName2 (hWndDlg,File,IDS_FILTERTEXT,".TXT",IDS_FILERPT))
{
#if ENABLETRACE
GSSiExitProg (646);
#endif
    	return FALSE;
}
    Fid = GSSiOpenFile (File,&OFStruct,OF_CREATE);
    if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (646);
#endif
        return FALSE;
}
    if (type == 1)
    { 
        Title[0]=0;
        if (!GetTextString (hWndDlg,Title,128,"Enter the report description",NULL,NULL,0,TRUE,TRUE))
        	goto Cancelled;
        sprintf (str,"GMREPORT %s",Title);
        fputstring (str,Fid);
        fputstring ("[%JUST]=C",Fid);
        fputstring ("[%MARGINS]=0.05",Fid);
        fputstring ("[%NUM_FONTS]=3",Fid);
        fputstring ("Arial,B,16,BLACK",Fid);
        fputstring ("Arial,B,12,BLACK",Fid);
        fputstring ("Arial,N,12,BLACK",Fid);
        fputstring ("[%NUM_FILES]=1",Fid);
        str[0]=0;
        if (!GetTextString (hWndDlg,Title,128,"Enter the report title",NULL,NULL,0,TRUE,TRUE)) 
        	goto Cancelled;
        FieldName[0]=0;
        if (!GetSQLWhereClause (hWndDlg, hSQL, FieldName))  
        	goto Cancelled;
        sprintf (str,"%s,%s",DBName,FieldName);
        fputstring (str,Fid);
        fputstring ("[%WRAP]=Y",Fid);
        fputstring ("[%NUM_TABS]=1",Fid);
        fputstring ("AUTO",Fid); 
        sprintf (str,"[%%NUM_ROWS]=%i",NumSelect+1);
        fputstring (str,Fid);
        sprintf (str,"[%%FONT]=1;%s",Title);
        fputstring (str,Fid);  
    } 
    else
        fputstring (DBName,Fid);
    for (j=0,pSelected=Selected;j<NumSelect;j++,pSelected++)
    {
        SendDlgItemMessage(hWndDlg,DlgItem,LB_GETTEXT, 
                                *pSelected,(LPARAM)((LPSTR)FieldName));
         
        if ((lpBar = _fstrchr(FieldName,'\t')))
            *lpBar = '\0';    
        if (type == 1)
            sprintf (str,"[%%FONT]=2;[%%JUST]=R;%s$TAB( )[%%FONT]=3;[%%JUST]=L;[%s]",FieldNameToTitle(FieldName,str2),FieldName);
        else
            sprintf (str,"[%s]",FieldName);
        fputstring (str,Fid);
    }
    GSSiClose (Fid);  
    sprintf (str,"Report %s has been created",File);
    GSSiMsgBox( GetFocus(),str,"",MB_OK,0);  
{
#if ENABLETRACE
GSSiExitProg (646);
#endif
    return TRUE;
}
Cancelled:       
	GSSiClose (Fid);
	GSSiRemove (File);
{
#if ENABLETRACE
GSSiExitProg (646);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 

LPSTR FieldNameToTitle (LPSTR FName,LPSTR OutStr)
#if ENABLETRACE
{GSSiEnterProg (647);
#endif
{                               
    short len,i;
    LPSTR   str; 
    BOOL    lastaspace=FALSE;
    
    len = _fstrlen(FName);
    _fstrcpy (OutStr,FName);
    REPLAC (OutStr,"_"," ",len);
    
    str = OutStr;
    str++;
    for (i=1;i<len;i++,str++)
    {   
        if (!lastaspace)
            *str = tolower(*str); 
        if (*str == ' ')
            lastaspace = TRUE; 
        else
            lastaspace = FALSE;
    }
{
#if ENABLETRACE
GSSiExitProg (647);
#endif
    return OutStr;
}
#if ENABLETRACE
}
#endif
}

BOOL GWD_MergeDBs (LPSTR ToDB, LPSTR FromDB, BOOL Replace)
#if ENABLETRACE
{GSSiEnterProg (648);
#endif
{
    GWDHEADER GWDHeadTo, GWDHeadFrom; 
    LPGWDHEADER lpGWDHeadTo, lpGWDHeadFrom;
    HANDLE  hDBTo, hDBFrom;   
    long	Offset, nLoaded=0, nRecs;  
    short	len, pos=BT_FIRST,i;
	BOOL	rtn=FALSE; 
	char	mess[300];

	hDBTo = OpenGWDatabase (ToDB,BT_WRITE); 
	if (!hDBTo)
{
#if ENABLETRACE
GSSiExitProg (648);
#endif
		return FALSE;
}
    lpGWDHeadTo = (LPGWDHEADER)GlobalLock (hDBTo);
	hDBFrom = OpenGWDatabase (FromDB,BT_READ); 
	if (!hDBFrom)
	{
		GlobalUnlock (hDBTo);
	    CloseGWDatabase (hDBTo); 
{
#if ENABLETRACE
GSSiExitProg (648);
#endif
		return FALSE;
}
	}
    lpGWDHeadFrom = (LPGWDHEADER)GlobalLock (hDBFrom);
    if (lpGWDHeadFrom->NumFields != lpGWDHeadTo->NumFields)
    {   
ErrMess:
		sprintf (mess,"%s and %s have different structures and cannot be merged",ToDB,FromDB);
    	GSSiMsgBox (GetFocus(),mess,NULL,MB_ICONEXCLAMATION,0);  
    	goto Exit;
    }
    for (i=0;i<lpGWDHeadFrom->NumFields;i++) 
    {
    	short l = _fstrlen (lpGWDHeadFrom->pFldInfo[i].Name);
    	if (_fmemcmp (&lpGWDHeadFrom->pFldInfo[i],&lpGWDHeadTo->pFldInfo[i],l+1+6))
    		goto ErrMess;
    }
	CreateStatusWind (hWndMain,1,"Merging Databases"); 
    nRecs = BT_NUM_IN_INDEX (lpGWDHeadFrom->BTHandle[0]);
    while (ContinueProcessing && !BT_FIND (lpGWDHeadFrom->BTHandle[0],lpGWDHeadFrom->pKeys[0],pos,BT_ANY, (LPSTR)&Offset))
    {   
    	pos = BT_NEXT;
        len = FillGWDData (lpGWDHeadFrom,Offset);
    	hmemmove ((HPSTR)&lpGWDHeadTo->GWDData,(HPSTR)&lpGWDHeadFrom->GWDData,lpGWDHeadFrom->Reclen);
		GWDAddRecord (lpGWDHeadTo,0,NULL);
		StatusWindowUpdate (NULL,NULL, nRecs, ++nLoaded);
	}
	if (ContinueProcessing)
		rtn = TRUE; 
Exit:
	GlobalUnlock (hDBFrom);
    CloseGWDatabase (hDBFrom); 
	GlobalUnlock (hDBTo);
    CloseGWDatabase (hDBTo); 
	DestroyStatusWindow(0);  
{
#if ENABLETRACE
GSSiExitProg (648);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}
 
BOOL GMDSwitchToMemFile (HANDLE hDB,int MaxMemMain,int MaxMemIndex)
{
	BOOL rtn=FALSE;
    LPGWDHEADER lpGWDHead = GlobalLock (hDB);
	int		i;
	HFILE	Fid;

	/*ConvertToMemFile (lpGWDHead->Fid,MaxMemMain);
	for (i=0;i<lpGWDHead->NumIndex;i++)
	{
		Fid = GetBTFid (lpGWDHead->BTHandle[i]);
		ConvertToMemFile (Fid,MaxMemIndex);
	}*/
	GlobalUnlock (hDB);
	return rtn;
}

BOOL GMDOpenJournal (LPSTR FileName)
{
	BOOL rtn=FALSE;

	return rtn;
}

BOOL GMDCloseJournal (LPSTR FileName)
{
	BOOL rtn=FALSE;

	return rtn;
}

BOOL TransferCacheBlocks (int BlockID,HFILE FidFrom,HFILE FidTo,HFILE FidNetTransfer,int BlockSize,HWND hWndProgress,int TotBlocksToGet,int nseqblocks,int nblocks)
{
	HANDLE	hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,BlockSize*nseqblocks);
	LPBYTE	pBlockData = GlobalLock (hMem);
	int		FromLoc, ToLoc;
	long	nBytes;
	long	BlockLoc = BlockID * BlockSize;

	FromLoc = _llseek (FidFrom,BlockLoc,0);
	if (FidNetTransfer == HFILE_ERROR)
		ToLoc = _llseek (FidTo,BlockLoc,0);
	nBytes = _lread (FidFrom,pBlockData,BlockSize*nseqblocks);
	if (nBytes > 0)
	{
		if (FidNetTransfer == HFILE_ERROR)
			_lwrite (FidTo,pBlockData,nBytes);
		else
		{
			LONGLONG	blockLoc = BlockLoc;

			BigWrite (FidNetTransfer,(LPSTR)&blockLoc,sizeof(LONGLONG),-1);
			BigWrite (FidNetTransfer,(LPSTR)&nBytes,4,-1);
			BigWrite (FidNetTransfer,pBlockData,nBytes,-1);
		}
	}
	GSSiGlobUlFree (&hMem);
	if (hWndCache)
		PctBox (hWndProgress,TotBlocksToGet,nblocks,0);
	return  TRUE;
}

BOOL GetCacheBlock (int BlockID,HFILE FidFrom,HFILE FidTo,HFILE FidNetTransfer,int BlockSize,HWND hWndProgress,int TotBlocksToGet,int opt)
{
	static	int nblocks;
	static	int	nseqblocks;
	static	int	startblockid;


	if (opt == 1)
	{
		nblocks = 0;
		nseqblocks = 0;
		return TRUE;
	}
	if (opt == 2)
	{
		nblocks += nseqblocks;
		if (nseqblocks)
			TransferCacheBlocks (startblockid,FidFrom,FidTo,FidNetTransfer,BlockSize,hWndProgress,TotBlocksToGet,nseqblocks,nblocks);
		return TRUE;
	}
	if (nseqblocks)
	{
		if (nseqblocks < 31 && startblockid == BlockID + 1)
		{
			nseqblocks++;
			startblockid = BlockID;
			return TRUE;
		}
		else
		{
			nblocks += nseqblocks;
			TransferCacheBlocks (startblockid,FidFrom,FidTo,FidNetTransfer,BlockSize,hWndProgress,TotBlocksToGet,nseqblocks,nblocks);
		}
	}
	startblockid = BlockID;
	nseqblocks = 1;
	return TRUE;
}

int UpdateGMDFromCheckPointLog2 (LPSTR CacheFile, LPSTR FromFile,int UpdateFromCheckPointID,HFILE FidNetTransfer)
{
	int		rtn=0; //0=updated OK,1=cant find cpl file,2=file too old to update,3=no need to update
	OFSTRUCTGM	OFStruct;
	HFILE	Fid, FidFrom, FidTo=HFILE_ERROR;
	CHECKPNTLOGHEADER CheckPntLogHeader;
	CHECKPNTLOGRECORD CheckPntLogRecord;
	long	loc,CompressedLength,FullLength,AllLength,SaveAllLength,lFullRec,CheckPointID,nBytes;
	long	AllNumBlocks, MaxNumBlocks, BlockNum;
	HANDLE	hCompressedRec, hFullRec, hAllRecs=0;
	LPBYTE	pCompressedRec, pFullRec, pAllRecs;
	long	ii;
	int		ifile=0;
    GWDHEADER GWDHead;
	char	CPLFile[MAX_PATH], FromIndexFile[MAX_PATH], ToIndexFile[MAX_PATH];
	LPSTR	pDot;

	strcpy (CPLFile,FromFile);
	pDot = strrchr (CPLFile,'.');
	strcpy (pDot,"_gmd.cpl");

	Fid  = OpenFileGM (CPLFile,&OFStruct,OF_READ);
	if (Fid != HFILE_ERROR)
	{
		_lread (Fid,&CheckPntLogHeader,sizeof(CHECKPNTLOGHEADER));
		if (UpdateFromCheckPointID >= CheckPntLogHeader.FirstCheckPointID)
		{
			char	Info[1024];

			sprintf (Info,"From %i to %i",UpdateFromCheckPointID,CheckPntLogHeader.LastCheckPointID);

			while (CheckPntLogHeader.LastCheckPointLoc[ifile] >= 0)
			{
				BOOL	First=TRUE;
				int		chksmf,chksmt;

				if (!ifile)
				{
					strcpy (FromIndexFile,FromFile);
					FidFrom = OpenFileGM (FromFile,&OFStruct,OF_READ);
					if (FidNetTransfer == HFILE_ERROR)
						FidTo = OpenFileGM (CacheFile,&OFStruct,OF_READWRITE);
					else
					{
						int	lnf = strlen(CacheFile)+1;

						BigWrite (FidNetTransfer,&lnf,sizeof(int),-1);
						BigWrite (FidNetTransfer,CacheFile,lnf,-1);
					}
				}
				else
				{
					strcpy (FromIndexFile,FromFile);
					pDot = strrchr (FromIndexFile,'.');
					sprintf (pDot,".in%i",ifile);
					FidFrom = OpenFileGM (FromIndexFile,&OFStruct,OF_READ);
					strcpy (ToIndexFile,CacheFile);
					pDot = strrchr (ToIndexFile,'.');
					sprintf (pDot,".in%i",ifile);
					if (FidNetTransfer == HFILE_ERROR)
					{
						FidTo = OpenFileGM (ToIndexFile,&OFStruct,OF_READWRITE);
						CacheAlreadyChecked (ToIndexFile,_fstrlen(CachePathnameTo),CHECKTIMESTAMP);  
					}
					else
					{
						int	lnf = strlen(ToIndexFile)+1;

						BigWrite (FidNetTransfer,&lnf,sizeof(int),-1);
						BigWrite (FidNetTransfer,ToIndexFile,lnf,-1);
					}
				}
				if (UpdateFromCheckPointID < CheckPntLogHeader.LastCheckPointID)
				{
					int TotBlocksToGet = 0, nblocks=0;

					AllNumBlocks = (_llseek(FidFrom,0,2)-1)/CheckPntLogHeader.BlockSize + 1;
					MaxNumBlocks = 0;
					AllLength = (AllNumBlocks - 1) / 8 + 1;
					hAllRecs = GSSiGlobAlloc (1676,GHND,AllLength);
					CheckPointID = CheckPntLogHeader.LastCheckPointID;
					loc = CheckPntLogHeader.LastCheckPointLoc[ifile];
					while (loc >= 0 && CheckPointID-- > UpdateFromCheckPointID)
					{
						_llseek (Fid,loc,0);
						_lread (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD));
						CompressedLength = CheckPntLogRecord.Reclen -sizeof(CHECKPNTLOGRECORD) + 4;
						hCompressedRec = GSSiGlobAlloc (1671,GMEM_MOVEABLE,CompressedLength);
						_llseek (Fid,loc+(sizeof(CHECKPNTLOGRECORD)-4),0);
						pCompressedRec = GlobalLock (hCompressedRec);
						_lread (Fid,pCompressedRec,CompressedLength);
						MaxNumBlocks = max (MaxNumBlocks,CheckPntLogRecord.NumBlocks);
						FullLength = (CheckPntLogRecord.NumBlocks - 1) / 8 + 1;
						hFullRec = GSSiGlobAlloc (1672,GMEM_MOVEABLE,FullLength+32);
						pFullRec = GlobalLock (hFullRec);
						lFullRec = DecompressBinaryRecordUnsafe (pFullRec,pCompressedRec,CompressedLength);
						pAllRecs = GlobalLock (hAllRecs);
						while (lFullRec--)
							*pAllRecs++ = *pAllRecs | *pFullRec++;
						GlobalUnlock (hAllRecs);
						GSSiGlobUlFree (&hCompressedRec);
						GSSiGlobUlFree (&hFullRec);
						loc = CheckPntLogRecord.PreviousRec;
					}
					pAllRecs = (LPBYTE)GlobalLock (hAllRecs);
					for (BlockNum = MaxNumBlocks;BlockNum < AllNumBlocks;BlockNum++)
						SetBit (BlockNum,pAllRecs,TRUE);
					BlockNum = 0;
					SaveAllLength = AllLength;
					while (AllLength--)
					{
						if (pAllRecs)
						{
							int	i;
							for (i=0;i<8;i++)
							{
								if (GetBit (i,pAllRecs))
									TotBlocksToGet++;
							}
						}
						pAllRecs++;
						BlockNum+=8;
					}
					AllLength = SaveAllLength;
					GlobalUnlock (hAllRecs);
					pAllRecs = (LPBYTE)GlobalLock (hAllRecs) + (AllLength - 1);
					BlockNum = AllLength * 8;
 					if (FidNetTransfer == HFILE_ERROR && TotBlocksToGet > 32)
					{
						sprintf (CacheTitle,"Incremental update from ver %ld to %ld",UpdateFromCheckPointID,CheckPntLogHeader.LastCheckPointID);
						hWndCache = CreateDialog(hInst, "CACHEFILE", hWndMain, (DLGPROC)CACHEFILEMsgProc); 
						DoPCTPeek = hWndCache;
						SetDlgItemText (hWndCache,IDC_FILEBEINGCACHED,FromIndexFile);
					}
					else
						hWndCache = 0;
					GetCacheBlock (0,0,0,0,0,GetDlgItem(hWndCache,IDC_CACHEPROGRESS),TotBlocksToGet,1);
					nblocks = 0;
					while (AllLength--)
					{
						if (!AllLength)
							ii=1;
						if (pAllRecs)
						{
							int	i;
							for (i=7;i>=0;i--)
							{
								if (GetBit (i,pAllRecs))
								{
									nCheckPointUpdateBlocks++;
									GetCacheBlock (BlockNum - (8-i),FidFrom,FidTo,FidNetTransfer,CheckPntLogHeader.BlockSize,GetDlgItem(hWndCache,IDC_CACHEPROGRESS),TotBlocksToGet,0);
								}
							}
						}
						pAllRecs--;
						BlockNum-=8;
					}
					GSSiGlobUlFree (&hAllRecs);
					GetCacheBlock (0,FidFrom,FidTo,FidNetTransfer,CheckPntLogHeader.BlockSize,GetDlgItem(hWndCache,IDC_CACHEPROGRESS),TotBlocksToGet,2);

			/*	compare from and to files	{
						HANDLE	hTest=GSSiGlobAlloc (0,GMEM_MOVEABLE,CheckPntLogHeader.BlockSize);
						LPBYTE	pTest=GlobalLock (hTest);

						_llseek (FidFrom,0,0);
						_llseek (FidTo,0,0);
						while ((nBytes = _lread (FidFrom,pBlockData,CheckPntLogHeader.BlockSize))>0)
						{
							long nBytes2 = _lread (FidTo,pTest,CheckPntLogHeader.BlockSize);
							if (nBytes != nBytes2)
								ii=1;
							else if (memcmp (pBlockData,pTest,CheckPntLogHeader.BlockSize))
								ii=1;
						}
						GSSiGlobUlFree (&hTest);
					}*/
				}
				else
					rtn = 3;
				if (FidNetTransfer != HFILE_ERROR)
				{
					LONGLONG	blockLoc = -1;

					BigWrite (FidNetTransfer,(LPSTR)&blockLoc,sizeof(LONGLONG),-1);
				}
				if (hWndCache)
				{
					DoPCTPeek = 0;
					DestroyWindow (hWndCache);
				}

				ifile++;
				/*chksmf = GetOpenFileChecksum2 (FidFrom,0,4096*2);
				chksmt = GetOpenFileChecksum2 (FidTo,0,4096*2);
				if (chksmf == chksmt)
					sprintf (strchr (Info,0),":File %i check 1 OK",ifile);
				else
					sprintf (strchr (Info,0),":File %i check 1 FAILS",ifile);
				chksmf = GetOpenFileChecksum2 (FidFrom,10000000,10000000+4096*2);
				chksmt = GetOpenFileChecksum2 (FidTo,10000000,10000000+4096*2);
				if (chksmf == chksmt)
					sprintf (strchr (Info,0),":File %i check 2 OK",ifile);
				else
					sprintf (strchr (Info,0),":File %i check 2 FAILS",ifile);
				chksmf = GetOpenFileChecksum2 (FidFrom,-(4096*2),0);
				chksmt = GetOpenFileChecksum2 (FidTo,-(4096*2),0);
				if (chksmf == chksmt)
					sprintf (strchr (Info,0),":File %i check 3 OK",ifile);
				else
					sprintf (strchr (Info,0),":File %i check 3 FAILS",ifile);
				chksmf = _llseek (FidFrom,0,2);
				chksmt = _llseek (FidTo,0,2);
				if (chksmf == chksmt)
					sprintf (strchr (Info,0),":File %i length OK",ifile);
				else
					sprintf (strchr (Info,0),":File %i length FAILS(%i %i)",ifile,chksmf,chksmt);
				*/
				_lclose (FidFrom);
				if (FidTo != HFILE_ERROR)
					_lclose (FidTo);
			}
			SetGlobalValue ("%GMDCPUPDATEINFO",Info);
		
		}
		else
			rtn = 2;
		_lclose (Fid);
	}
	else
		rtn = 1;
	return rtn;
}

BOOL UpdateGMDFromCheckPointLog (HFILE FidCache,LPSTR ToFile, LPSTR FromFile)
{
	BOOL rtn=FALSE;
	LPSTR	pDot = strrchr (FromFile,'.');
	HFILE	Fid;
	OFSTRUCTGM  OFStruct;
    GWDHEADER GWDHead, GWDHeadFrom;

	if (pDot && !stricmp (pDot,".gmd"))
	{
		if (FidCache != HFILE_ERROR)
		{
			BigRead (FidCache,(HPSTR)&GWDHead,sizeof(GWDHEADER));
			if (GWDHead.StoredAs32)
			{
				int	CheckPointIDCache = GWDHead.CheckPointID;

				if (GWDHead.CheckPointID)
				{
					Fid = OpenFileGM (FromFile,&OFStruct,OF_READ);
					if (Fid != HFILE_ERROR)
					{
						ii=_lread (Fid,(HPSTR)&GWDHeadFrom,sizeof(GWDHEADER));
						_lclose (Fid);
						if (GWDHeadFrom.StoredAs32)
						{
							if (GWDHeadFrom.CheckPointID >= CheckPointIDCache)
							{
								int	rc;

								GSSiClose (FidCache);
								FidCache = HFILE_ERROR;
								rc = UpdateGMDFromCheckPointLog2 (ToFile,FromFile,CheckPointIDCache,HFILE_ERROR);
								if (rc == 0 || rc == 3)
									rtn = TRUE;
							}
						}
					}
				}
			}
		}
	}
	GSSiClose (FidCache);
	return rtn;
}

int UpdateGMDFromCheckPointLog_net (int CheckPointIDCache,HFILE FidNetTransfer,LPSTR FromFileIN)
{
	int rtn=10;//0=file updated,10=not checkpointed gmd file,1=cant find cpl file,2=file too old to update,3=no need to update
	LPSTR	pDot;
	HFILE	Fid;
	OFSTRUCTGM  OFStruct;
    GWDHEADER GWDHead;
	char	fromFile[MAX_PATH];
	char	cacheFile[MAX_PATH];

	strcpy (fromFile,FromFileIN);
	if (strnicmp (fromFile,"[%DL]",5))
		return 2;
	sprintf (cacheFile,"[%%CACHEDIRACTUAL]%s",&fromFile[5]);
	ExpandText (fromFile);
	pDot  = strrchr (fromFile,'.');
	if (pDot && !stricmp (pDot,".gmd"))
	{
		Fid = OpenFileGM (fromFile,&OFStruct,OF_READ);
		if (Fid != HFILE_ERROR)
		{
			_lread (Fid,(HPSTR)&GWDHead,sizeof(GWDHEADER));
			_lclose (Fid);
			if (GWDHead.StoredAs32)
			{
				if (GWDHead.CheckPointID >= CheckPointIDCache)
					rtn = UpdateGMDFromCheckPointLog2 (cacheFile,fromFile,CheckPointIDCache,FidNetTransfer);
			}
		}
	}
	return rtn;
}

int checkcpl (int i)
{
//	char	File[]="C:\\Users\\Jeff\\Downloads\\inc_comments_gmd.cpl";
	char	File[]="C:\\Users\\Jeff\\Downloads\\mgv2\\incident_gmd.cpl";
	OFSTRUCTGM	OFStruct;
	HFILE	Fid;
	CHECKPNTLOGHEADER CheckPntLogHeader;
	CHECKPNTLOGRECORD CheckPntLogRecord;
	long	loc,CompressedLength,FullLength,AllLength,lFullRec;
	HANDLE	hCompressedRec, hFullRec, hAllRecs=0;
	LPBYTE	pCompressedRec, pFullRec, pAllRecs;
	long	nb = 0;
	int		ifile=0;
    GWDHEADER GWDHead;
return 1;
	Fid = OpenFileGM ("E:\\GMMGVPOL_Test\\attribut\\Incident.gmd",&OFStruct,OF_READ);
	_lread (Fid,&GWDHead,sizeof(GWDHEADER));
	_lclose (Fid);
	Fid  = OpenFileGM (File,&OFStruct,OF_READ);
	_lread (Fid,&CheckPntLogHeader,sizeof(CHECKPNTLOGHEADER));
	while (CheckPntLogHeader.LastCheckPointLoc[ifile] >= 0)
	{
		BOOL	First=TRUE;

		loc = CheckPntLogHeader.LastCheckPointLoc[ifile++];
		while (loc >= 0)
		{
			_llseek (Fid,loc,0);
			_lread (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD));
			CompressedLength = CheckPntLogRecord.Reclen -sizeof(CHECKPNTLOGRECORD) + 4;
			hCompressedRec = GSSiGlobAlloc (1673,GMEM_MOVEABLE,CompressedLength);
			_llseek (Fid,loc+(sizeof(CHECKPNTLOGRECORD)-4),0);
			pCompressedRec = GlobalLock (hCompressedRec);
			_lread (Fid,pCompressedRec,CompressedLength);
			FullLength = (CheckPntLogRecord.NumBlocks - 1) / 8 + 1;
			hFullRec = GSSiGlobAlloc (1674,GMEM_MOVEABLE,FullLength+32);
			if (First)
			{
				AllLength = FullLength;
				hAllRecs = GSSiGlobAlloc (1677,GHND,AllLength);
				First = FALSE;
			}
			pFullRec = GlobalLock (hFullRec);
			lFullRec = DecompressBinaryRecordUnsafe (pFullRec,pCompressedRec,CompressedLength);
			pAllRecs = GlobalLock (hAllRecs);
			while (lFullRec--)
				*pAllRecs++ = *pAllRecs || *pFullRec++;
			GlobalUnlock (hAllRecs);
			GSSiGlobUlFree (&hCompressedRec);
			GSSiGlobUlFree (&hFullRec);
			loc = CheckPntLogRecord.PreviousRec;
		}
		pAllRecs = GlobalLock (hAllRecs);
		while (AllLength--)
		{
			if (pAllRecs)
			{
				int	i;
				for (i=0;i<8;i++)
				{
					if (GetBit (i,pAllRecs))
						nb++;
				}
			}
			pAllRecs++;
		}
		GSSiGlobUlFree (&hAllRecs);
	}
	_lclose (Fid);
	return 1;
}


BOOL GMDCreateCheckPointLog (LPSTR FileName)
{
	BOOL rtn=FALSE;
	CHECKPNTLOGHEADER CheckPntLogHeader;
    LPGWDHEADER lpGWDHead;
	char	CheckPntLogFileName[MAX_PATH];
	int		i,ln;
	HFILE	Fid;

	HANDLE	hDB = OpenGWDatabase (FileName,BT_WRITE);
	if (hDB)
	{
		lpGWDHead = GlobalLock (hDB);
		if (lpGWDHead->CheckPointID <= 0)
			lpGWDHead->CheckPointID = 1;
		else
			lpGWDHead->CheckPointID++;
		memset (&CheckPntLogHeader,0,sizeof(CheckPntLogHeader));
		for (i=0;i<MAX_GMD_INDEXES+1;i++)
		{
			CheckPntLogHeader.FirstCheckPointLoc[i] = -1;
			CheckPntLogHeader.LastCheckPointLoc[i]  = -1;
		}
		CheckPntLogHeader.FirstFreeBlock    = -1;
		CheckPntLogHeader.LastFreeBlock     = -1;
		CheckPntLogHeader.FirstCheckPointID = lpGWDHead->CheckPointID;
		CheckPntLogHeader.LastCheckPointID  = lpGWDHead->CheckPointID;
		CheckPntLogHeader.BlockSize = 1024;
		CheckPntLogHeader.nCheckPoints = 1024;
		strcpy (CheckPntLogFileName,FileName);
		strlwr (CheckPntLogFileName);
		ln = strlen (CheckPntLogFileName);
		if (!strcmp (&CheckPntLogFileName[ln-4],".gmd"))
		{
			strcpy (&CheckPntLogFileName[ln-4],"_gmd.cpl");
			Fid = GSSiOpenFile (CheckPntLogFileName,0,OF_CREATE);   
			if (Fid != HFILE_ERROR)
			{
				BigWrite (Fid,&CheckPntLogHeader,sizeof(CHECKPNTLOGHEADER),-1);
				GSSiClose (Fid);
				rtn = TRUE;
			}
		}
		GlobalUnlock (hDB);
		CloseGWDatabase (hDB);
	}
	return rtn;
}

void tracecpl (int id,int loc,LPCHECKPNTLOGRECORD prec)
{
	char	str[256];

	return;

	sprintf (str,"%i:%i - %i,%i",id,loc,prec->PreviousRec,prec->NextRec);
	AppendFile2 ("c:\\temp\\cpllog.txt",str);
	return;
}

BOOL AddCPLRecordToFreeSpace (HFILE Fid,LPCHECKPNTLOGHEADER pCheckPntLogHeader,int index,int AddLoc)
{
	CHECKPNTLOGRECORD CheckPntLogRecordAdd, CheckPntLogRecordNext, CheckPntLogRecord;
	int	loc = pCheckPntLogHeader->FirstFreeBlock, Lastloc=-1;
	int	ii;

	GSSillseek (Fid,AddLoc,0);
	BigRead (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4);
	CheckPntLogRecord.NextRec = -1;
	pCheckPntLogHeader->MaxFreeBlockSize = 0;
	if (pCheckPntLogHeader->FirstFreeBlock < 0)
	{
		pCheckPntLogHeader->FirstFreeBlock = AddLoc;
		CheckPntLogRecord.PreviousRec = -1;
		CheckPntLogRecordAdd = CheckPntLogRecord;
	}
	else
	{
		int len;

		len = CheckPntLogRecord.Reclen;
		CheckPntLogRecordAdd = CheckPntLogRecord;
		loc = pCheckPntLogHeader->FirstFreeBlock;
		while (loc >= 0)
		{
			GSSillseek (Fid,loc,0);
			if (BigRead (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4) != sizeof(CHECKPNTLOGRECORD)-4)
				return FALSE;
			//if (CheckPntLogRecord.PreviousRec != Lastloc)
			//	break;
			if (loc + CheckPntLogRecord.Reclen == CheckPntLogRecord.NextRec)
			{
				GSSillseek (Fid,CheckPntLogRecord.NextRec,0);
				BigRead (Fid,&CheckPntLogRecordNext,sizeof(CHECKPNTLOGRECORD)-4);
				CheckPntLogRecord.Reclen += CheckPntLogRecordNext.Reclen;
				pCheckPntLogHeader->MaxFreeBlockSize = max (pCheckPntLogHeader->MaxFreeBlockSize,CheckPntLogRecord.Reclen);
				CheckPntLogRecord.NextRec = CheckPntLogRecordNext.NextRec;
				GSSillseek (Fid,loc,0);
				if (loc == 1517)
					ii= CheckPntLogRecord.NextRec;
				tracecpl (1,loc,&CheckPntLogRecord);
				BigWrite (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4,-1);
				if (CheckPntLogRecord.NextRec > -1)
				{
					GSSillseek (Fid,CheckPntLogRecord.NextRec,0);
					BigRead (Fid,&CheckPntLogRecordNext,sizeof(CHECKPNTLOGRECORD)-4);
					CheckPntLogRecordNext.PreviousRec = loc;
					GSSillseek (Fid,CheckPntLogRecord.NextRec,0);
				tracecpl (11,CheckPntLogRecord.NextRec,&CheckPntLogRecordNext);
					BigWrite (Fid,&CheckPntLogRecordNext,sizeof(CHECKPNTLOGRECORD)-4,-1);
				}
				else
					pCheckPntLogHeader->LastFreeBlock = loc;
			}
			else
				pCheckPntLogHeader->MaxFreeBlockSize = max (pCheckPntLogHeader->MaxFreeBlockSize,CheckPntLogRecord.Reclen);
			Lastloc = loc;
			loc = CheckPntLogRecord.NextRec;
		}
		CheckPntLogRecord.NextRec = AddLoc;
		GSSillseek (Fid,Lastloc,0);
				if (Lastloc == 1517)
					ii= CheckPntLogRecord.NextRec;
				tracecpl (2,Lastloc,&CheckPntLogRecord);
		BigWrite (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4,-1);
	}
	GSSillseek (Fid,AddLoc,0);
	CheckPntLogRecordAdd.PreviousRec = Lastloc;
				tracecpl (3,AddLoc,&CheckPntLogRecordAdd);
	BigWrite (Fid,&CheckPntLogRecordAdd,sizeof(CHECKPNTLOGRECORD)-4,-1);
	pCheckPntLogHeader->LastFreeBlock = AddLoc;
	pCheckPntLogHeader->MaxFreeBlockSize = max (pCheckPntLogHeader->MaxFreeBlockSize,CheckPntLogRecordAdd.Reclen);

	return TRUE;
}

void chekcforloop (HFILE Fid,LPCHECKPNTLOGHEADER pCheckPntLogHeader)
{
	CHECKPNTLOGRECORD CheckPntLogRecord;
	int loc,n=0,lastloc=-1,totfree=0;

	return;
	loc = pCheckPntLogHeader->FirstFreeBlock;
	while (loc >= 0)
	{
		n++;
		if (n > 1000)
			n=0;
		GSSillseek (Fid,loc,0);
		BigRead (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4);
		totfree += CheckPntLogRecord.Reclen;
		if (lastloc != CheckPntLogRecord.PreviousRec)
			n=0;
		lastloc = loc;
		loc = CheckPntLogRecord.NextRec;
	}
	return;
}

int	GetCPLNewRecordLoc (HFILE Fid,LPCHECKPNTLOGHEADER pCheckPntLogHeader,int len)
{
	CHECKPNTLOGRECORD CheckPntLogRecord, CheckPntLogRecord2;
	int loc, Lastloc;
	int	nPieces=0;
	int prRec, nxRec;

Top:
	if (len <= pCheckPntLogHeader->MaxFreeBlockSize)
	{
		chekcforloop (Fid,pCheckPntLogHeader);
		loc = pCheckPntLogHeader->FirstFreeBlock;
		Lastloc = -1;
		while (loc >= 0)
		{
			GSSillseek (Fid,loc,0);
			BigRead (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4);
			nPieces++;
			if (len == CheckPntLogRecord.Reclen)
			{
				nxRec = CheckPntLogRecord.NextRec;
				if (pCheckPntLogHeader->MaxFreeBlockSize == len)
					pCheckPntLogHeader->MaxFreeBlockSize = 0;

				if (Lastloc > -1)
				{
					GSSillseek (Fid,Lastloc,0);
					BigRead (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4);
					CheckPntLogRecord.NextRec = nxRec;
					GSSillseek (Fid,Lastloc,0);
					tracecpl (4,Lastloc,&CheckPntLogRecord);
					BigWrite (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4,-1);
				}
				else
					pCheckPntLogHeader->FirstFreeBlock = CheckPntLogRecord.NextRec;
				if (nxRec > -1)
				{
					GSSillseek (Fid,nxRec,0);
					BigRead (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4);
					CheckPntLogRecord.PreviousRec = Lastloc;
					GSSillseek (Fid,nxRec,0);
					tracecpl (13,nxRec,&CheckPntLogRecord);
					BigWrite (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4,-1);
				}
				else
					pCheckPntLogHeader->LastFreeBlock = Lastloc;
		chekcforloop (Fid,pCheckPntLogHeader);
				return loc;
			}
			if (len < (CheckPntLogRecord.Reclen - (sizeof(CHECKPNTLOGRECORD)-4)))
			{
				int	Rtnloc = loc;

				if (pCheckPntLogHeader->MaxFreeBlockSize == CheckPntLogRecord.Reclen)
					pCheckPntLogHeader->MaxFreeBlockSize = 0;
				loc += len;
				CheckPntLogRecord.Reclen -= len;
				GSSillseek (Fid,loc,0);
					tracecpl (5,loc,&CheckPntLogRecord);
				BigWrite (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4,-1);
				nxRec = CheckPntLogRecord.NextRec;
				if (Lastloc > -1)
				{
					GSSillseek (Fid,Lastloc,0);
					BigRead (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4);
					CheckPntLogRecord.NextRec = loc;
					GSSillseek (Fid,Lastloc,0);
					tracecpl (6,Lastloc,&CheckPntLogRecord);
					BigWrite (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4,-1);
				}
				else
					pCheckPntLogHeader->FirstFreeBlock = loc;
				if (nxRec > -1)
				{
					GSSillseek (Fid,nxRec,0);
					BigRead (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4);
					CheckPntLogRecord.PreviousRec = loc;
					GSSillseek (Fid,nxRec,0);
					tracecpl (12,nxRec,&CheckPntLogRecord);
					BigWrite (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4,-1);
				}
				else
					pCheckPntLogHeader->LastFreeBlock = Lastloc;
		chekcforloop (Fid,pCheckPntLogHeader);
				return Rtnloc;
			}
			if (loc >= 0 && loc < Lastloc)
			{
				nxRec = CheckPntLogRecord.NextRec;
				GSSillseek (Fid,Lastloc,0);
				BigRead (Fid,&CheckPntLogRecord2,sizeof(CHECKPNTLOGRECORD)-4);
				CheckPntLogRecord2.NextRec = CheckPntLogRecord.NextRec;
				prRec = CheckPntLogRecord2.PreviousRec;
				CheckPntLogRecord2.PreviousRec = loc;
				GSSillseek (Fid,Lastloc,0);
					tracecpl (7,Lastloc,&CheckPntLogRecord2);
				BigWrite (Fid,&CheckPntLogRecord2,sizeof(CHECKPNTLOGRECORD)-4,-1);
				if (prRec >= 0)
				{
					GSSillseek (Fid,prRec,0);
					BigRead (Fid,&CheckPntLogRecord2,sizeof(CHECKPNTLOGRECORD)-4);
					CheckPntLogRecord2.NextRec = loc;
					GSSillseek (Fid,prRec,0);
					tracecpl (8,prRec,&CheckPntLogRecord2);
					BigWrite (Fid,&CheckPntLogRecord2,sizeof(CHECKPNTLOGRECORD)-4,-1);
				}
				else
					pCheckPntLogHeader->FirstFreeBlock = loc;
				GSSillseek (Fid,loc,0);
				CheckPntLogRecord.PreviousRec = prRec;
				CheckPntLogRecord.NextRec = Lastloc;
					tracecpl (9,loc,&CheckPntLogRecord);
				BigWrite (Fid,&CheckPntLogRecord,sizeof(CHECKPNTLOGRECORD)-4,-1);
				if (nxRec >= 0)
				{
					GSSillseek (Fid,nxRec,0);
					BigRead (Fid,&CheckPntLogRecord2,sizeof(CHECKPNTLOGRECORD)-4);
					CheckPntLogRecord2.PreviousRec = Lastloc;
					GSSillseek (Fid,nxRec,0);
					tracecpl (10,nxRec,&CheckPntLogRecord2);
					BigWrite (Fid,&CheckPntLogRecord2,sizeof(CHECKPNTLOGRECORD)-4,-1);
				}
				else
					pCheckPntLogHeader->LastFreeBlock = Lastloc;
				goto Top;
			}
			else
			{
				Lastloc = loc;
				loc = CheckPntLogRecord.NextRec;
			}
		}
		pCheckPntLogHeader->LastFreeBlock = Lastloc;
	}
	loc	= GSSillseek (Fid,0,2);
	chekcforloop (Fid,pCheckPntLogHeader);
	return loc;
}

BOOL GMDUpdateCheckPointLog (LPSTR FileName)
{
	BOOL rtn=FALSE;
	CHECKPNTLOGHEADER CheckPntLogHeader;
	CHECKPNTLOGRECORD CheckPntLogRecord, CheckPntLogRecord2;
    LPGWDHEADER lpGWDHead;
	char	CheckPntLogFileName[MAX_PATH];
	int		indx,j,ln,loc;
	HFILE	Fid, Fid2;
	BOOL	SaveAllowJournal = AllowJournal;
	int		IncFirstCP=0;

	strcpy (CheckPntLogFileName,FileName);
	strlwr (CheckPntLogFileName);
	ln = strlen (CheckPntLogFileName);
	if (!strcmp (&CheckPntLogFileName[ln-4],".gmd"))
	{
		strcpy (&CheckPntLogFileName[ln-4],"_gmd.cpl");
		Fid = GSSiOpenFile (CheckPntLogFileName,0,OF_READWRITE);   
		if (Fid != HFILE_ERROR)
		{
			HANDLE	hDB;

			AllowJournal = TRUE;
			hDB = OpenGWDatabase (FileName,BT_WRITE);
			AllowJournal = FALSE;
			if (hDB)
			{
				lpGWDHead = GlobalLock (hDB);
				BigRead (Fid,&CheckPntLogHeader,sizeof(CHECKPNTLOGHEADER));
				for (indx=0;indx<lpGWDHead->NumIndex+1;indx++)
				{
					if (indx)
						Fid2 = GetBTFid (lpGWDHead->BTHandle[indx-1]);
					else
						Fid2 = lpGWDHead->Fid;
					if (JournalFileIndex[Fid2])
					{
						LPLONG	pNumIndexBlocks = GlobalLock (JournalFileIndex[Fid2]);
						LPJOURNALINDEXRECORD pIndexRecord = (LPJOURNALINDEXRECORD)(pNumIndexBlocks+1);
						HANDLE	hBytes;
						LPBYTE	pBytes;

						CheckPntLogRecord.NumBlocks = (OpenFileLength[Fid2] -1) / CheckPntLogHeader.BlockSize + 1;
						if (CheckPntLogRecord.NumBlocks > 0)
						{
							int	len = CheckPntLogRecord.NumBlocks / 8 + 1;
							HANDLE	hCompressedRec = GSSiGlobAlloc (1513,GMEM_MOVEABLE,len+32);
							HPSTR	pCompressedRec = GlobalLock (hCompressedRec);
							long	CompressedLength;

							hBytes = GSSiGlobAlloc (1601,GHND,len);
							pBytes = GlobalLock (hBytes);
							for (j=0;j<*pNumIndexBlocks;j++,pIndexRecord++)
							{
								int	iblock = pIndexRecord->StartBlock;
								int	nblocks = pIndexRecord->NumBlocks;

								while (nblocks--)
									SetBit (iblock++, pBytes,TRUE);
							}
						    CompressedLength = CompressBinaryRecord (pBytes,pCompressedRec,len);
							CheckPntLogRecord.PreviousRec = CheckPntLogHeader.LastCheckPointLoc[indx];
							CheckPntLogRecord.Reclen = CompressedLength+sizeof(CHECKPNTLOGRECORD)-4;
							CheckPntLogRecord.NextRec = -1;
							if (CheckPntLogHeader.LastCheckPointID - CheckPntLogHeader.FirstCheckPointID >= CheckPntLogHeader.nCheckPoints)
							{
								GSSillseek (Fid,CheckPntLogHeader.FirstCheckPointLoc[indx],0);
								BigRead (Fid,&CheckPntLogRecord2,sizeof(CHECKPNTLOGRECORD));
								AddCPLRecordToFreeSpace (Fid,&CheckPntLogHeader,indx,CheckPntLogHeader.FirstCheckPointLoc[indx]);
								CheckPntLogHeader.FirstCheckPointLoc[indx] = CheckPntLogRecord2.NextRec;
								IncFirstCP =1;
							}
							loc = GetCPLNewRecordLoc (Fid,&CheckPntLogHeader,CheckPntLogRecord.Reclen);
							if (CheckPntLogHeader.LastCheckPointLoc[indx] > -1)
							{
								GSSillseek (Fid,CheckPntLogHeader.LastCheckPointLoc[indx],0);
								BigRead (Fid,&CheckPntLogRecord2,sizeof(CHECKPNTLOGRECORD));
								CheckPntLogRecord2.NextRec = loc;
								GSSillseek (Fid,CheckPntLogHeader.LastCheckPointLoc[indx],0);
								BigWrite (Fid,&CheckPntLogRecord2,sizeof(CHECKPNTLOGRECORD),-1);
							}
							else
								CheckPntLogHeader.FirstCheckPointLoc[indx] = loc;
							CheckPntLogHeader.LastCheckPointLoc[indx] = loc;
							GSSillseek (Fid,loc,0);
							BigWrite (Fid,&CheckPntLogRecord,sizeof(CheckPntLogRecord)-4,-1);
							BigWrite (Fid,pCompressedRec,CompressedLength,-1);
							GSSiGlobUlFree (&hCompressedRec);
							GSSiGlobUlFree (&hBytes);
						}
						GlobalUnlock (JournalFileIndex[Fid2]);
					}
				}
				lpGWDHead->CheckPointID++;
				CheckPntLogHeader.LastCheckPointID  = lpGWDHead->CheckPointID;
				CheckPntLogHeader.FirstCheckPointID += IncFirstCP;
				GSSillseek (Fid,0,0);
				BigWrite (Fid,&CheckPntLogHeader,sizeof(CHECKPNTLOGHEADER),-1);
				GSSiClose (Fid);
				GlobalUnlock (hDB);
				AllowJournal = TRUE;
				CloseGWDatabase (hDB);
				rtn = TRUE;
			}
		}
	}
	AllowJournal = SaveAllowJournal;
	return rtn;
}

int GetSplitLengthFromRequest(LPGWDHEADER lpGWDHead)
{
	int pos = BT_FIRST;
	long offset, maxOffset = -1;
	long totLen, nRead = 0;
	long endoff = lpGWDHead->SplitLengthRequested;// +sizeof(GWDHEADER)+lpGWDHead->NumFields*sizeof(GWFLDINFO);

	CreateStatusWind(hWndMain, 1, "Find Split Point");

	totLen = BT_NUM_IN_INDEX(lpGWDHead->BTHandle[0]);
	while (!BT_FIND(lpGWDHead->BTHandle[0], lpGWDHead->pKeys[0], pos, BT_ANY, (LPSTR)&offset))
	{
		pos = BT_NEXT;
		if (offset < endoff && offset > maxOffset)
			maxOffset = offset;
		StatusWindowUpdate(NULL, NULL, totLen, nRead++);
	}
	maxOffset += 4 + GetCompressedReclen(lpGWDHead, maxOffset);
	DestroyStatusWindow(0);
	return maxOffset;
}

BOOL GMDFunctions (int nArgs,LPSTR *Arg,LPSTR OutLoc)
{
	HANDLE	hDB, hDB1, hDB2;
    LPGWDHEADER lpGWDHead,lpGWDHead1,lpGWDHead2;
    LPGWFLDINFO lpFieldInfo;
	GWFLDINFO	FieldInfo;
	int		i, loc;
	BOOL	rtn=FALSE;
	long	Offset1, Offset2;
	char	DefStr[1024];
	int		NumDiffs=0;
    
	*OutLoc = 0;
	if (!stricmp (Arg[1],"RENAMEFIELD"))
	{
		hDB = OpenGWDatabase (Arg[2],BT_WRITE);
		if (!hDB)
			return -1;
		lpGWDHead = GlobalLock (hDB);
		GSSillseek (lpGWDHead->Fid,0,0);
		BigRead (lpGWDHead->Fid,(HPSTR)&GWDHead16,sizeof(GWDHEADER16));

		if (GWDHead16.Unused)
		{
			int	Size;
			HANDLE DBHandle = GSSiGlobAlloc ( 264,GHND,sizeof (GWDHEADER)+4);

			lpGWDHead2 =(LPGWDHEADER) GlobalLock (DBHandle);
			GSSillseek (lpGWDHead->Fid,0,0);
			BigRead (lpGWDHead->Fid,(HPSTR)lpGWDHead2,sizeof(GWDHEADER));
			Size = sizeof (GWDHEADER)+ 4 + lpGWDHead2->Reclen;
			GlobalUnlock (DBHandle);
			DBHandle = GSSiGlobalReAlloc (0,DBHandle,Size,GMEM_MOVEABLE);
			lpGWDHead2 =(LPGWDHEADER) GlobalLock (DBHandle);
			GSSiGlobUlFree (&DBHandle);
		}
		/*else
		{
			HANDLE DBHandle32 = GSSiGlobAlloc ( 264,GHND,sizeof (GWDHEADER32));
			LPGWDHEADER32 lpGWDHead32 =(LPGWDHEADER32) GlobalLock (DBHandle32);

			DBHandle = GSSiGlobAlloc ( 264,GHND,sizeof (GWDHEADER)+GWDHead16.Reclen+4);
			lpGWDHead =(LPGWDHEADER) GlobalLock (DBHandle);
			*lpGWDHead32 = GWDHEADER16toGWDHEADER32 (&GWDHead16);
			*lpGWDHead = GWDHEADER32toGWDHEADER (lpGWDHead32);
			GSSiGlobUlFree (&DBHandle32);
		}*/


		for (i=0;i<lpGWDHead->NumFields;i++)
		{
			loc = GSSillseek (lpGWDHead->Fid,0,1);
			BigRead (lpGWDHead->Fid,(HPSTR)&FieldInfo,sizeof(GWFLDINFO));
			if (!stricmp (FieldInfo.Name,Arg[3]))
			{
				GSSillseek (lpGWDHead->Fid,loc,0);
				strcpy (FieldInfo.Name,Arg[4]);
				BigWrite (lpGWDHead->Fid,&FieldInfo,sizeof(GWFLDINFO),-1);
				rtn = TRUE;
				break;
			}
		}
		GlobalUnlock (hDB);
		CloseGWDatabase (hDB); 
	}
	else if (!stricmp(Arg[1], "SETLIMITS"))
	{
		MNMXCORD Bounds;
		long	MinTime, MaxTime;

		rtn = GMDGetFileMinMax(Arg[2], &Bounds, &MinTime, &MaxTime);
	}
	else if (!stricmp(Arg[1], "KEYLIST"))
	{
		if (!stricmp(Arg[2], "CREATE"))
		{
			rtn = GMDCreateKeyList(Arg[3]);
		}
		else if (!stricmp(Arg[2], "DELETE"))
		{
			BT_CLOSEANDDELETE(&hGMDKeyList);
		}
		if (!stricmp(Arg[2], "ADD"))
		{
			rtn = GMDKeyListAdd(Arg[3]);
		}

	}
	else if (!stricmp(Arg[1], "GETCPID"))
	{
		hDB1 = OpenGWDatabase (Arg[2],BT_READ);
		if (!hDB1)
			return FALSE;
		lpGWDHead1 = GlobalLock (hDB1);
		itoa (lpGWDHead1->CheckPointID,OutLoc,10);
		GlobalUnlock (hDB1);
		CloseGWDatabase (hDB1); 
		return TRUE;
	}
	else if (!stricmp(Arg[1], "SPLIT"))//$GMD(SPLIT,infile,outfile,splitlen)
	{
		int splitLen = atoi(Arg[4]);
		char SplitFile[MAX_PATH];
		char drive[32], dir[MAX_PATH], fname[MAX_PATH];
		char OutFilePath[MAX_PATH];
		char txt[128];
		LPSTR pDot;

		if (splitLen <= 0)
			return FALSE;
		hDB = OpenGWDatabase(Arg[2], BT_READ);
		if (!hDB)
			return FALSE;
		lpGWDHead = GlobalLock(hDB);
		if (splitLen > GSSifilelength(lpGWDHead->Fid))
		{
			GlobalUnlock(hDB);
			CloseGWDatabase(hDB);
			MessageBox(0, "Split length cannot exceed file length", 0, MB_ICONEXCLAMATION);
			return FALSE;
		}
		if (GSSifilelength(lpGWDHead->Fid) > splitLen + lpGWDHead->Reclen * 2)
		{
			if (!lpGWDHead->SplitFile)
			{
				HFILE FidOut = GSSiOpenFile(Arg[3], 0, OF_CREATE);

				if (FidOut != HFILE_ERROR)
				{
					LPSTR pBS;
					HFILE FidOut2;

					strcpy(SplitFile, Arg[3]);
					ExpandText(SplitFile);
					_splitpath(SplitFile, drive, dir, fname, 0);
					if (*LastChr(dir) == '\\')
						*LastChr(dir) = 0;
					pBS = strrchr(dir, '\\');
					if (pBS)
						*pBS = 0;
					sprintf(SplitFile, "%s%s\\%s_%i.gsf", drive, dir, fname, splitLen);
					FidOut2 = GSSiOpenFile(SplitFile, 0, OF_CREATE);
					if (FidOut2 != HFILE_ERROR)
					{
						long nRead, didRead, totLen, curLoc, lastLoc,index;
#define BUFFERSIZE USHRT_MAX
						HANDLE hbuf = GSSiGlobAlloc(0, GMEM_MOVEABLE, BUFFERSIZE);
						LPSTR pbuf = GlobalLock(hbuf);

						lpGWDHead->SplitLengthRequested = splitLen;
						lpGWDHead->SplitLength = GetSplitLengthFromRequest(lpGWDHead);
						lpGWDHead->SplitFile = 1;
						BigWrite(FidOut, lpGWDHead, sizeof(GWDHEADER), -1);
						GSSillseek(lpGWDHead->Fid, sizeof(GWDHEADER), 0);
						didRead = BigRead(lpGWDHead->Fid, pbuf, lpGWDHead->NumFields*sizeof(GWFLDINFO));
						BigWrite(FidOut, pbuf,didRead, -1);
						nRead = lpGWDHead->SplitLength - GSSillseek(lpGWDHead->Fid,0,1);
						CreateStatusWind(hWndMain, 1, "Write split file");
						while (nRead > 0)
						{
							didRead = BigRead(lpGWDHead->Fid, pbuf, min(nRead, BUFFERSIZE));

							BigWrite(FidOut2, pbuf, didRead, -1);
							nRead -= didRead;
							StatusWindowUpdate(NULL, NULL, lpGWDHead->SplitLength, nRead);
						}
						GSSiClose(FidOut2);
						curLoc = GSSillseek(lpGWDHead->Fid, 0, 1);
						lastLoc = GSSillseek(lpGWDHead->Fid, 0, 2);
						GSSillseek(lpGWDHead->Fid, curLoc, 0);
						totLen = lastLoc - curLoc;
						didRead = BigRead(lpGWDHead->Fid, pbuf, BUFFERSIZE);
						ii = *(LPINT)pbuf;
						while (didRead > 0)
						{
							BigWrite(FidOut, pbuf, didRead, -1);
							StatusWindowUpdate(NULL, NULL, totLen, nRead+=didRead);
							didRead = BigRead(lpGWDHead->Fid, pbuf, BUFFERSIZE);
						}
						GSSiClose(FidOut);
						for (index = 0; index < lpGWDHead->NumIndex; index++)
						{
							HFILE IndexFid = GetBTFid(lpGWDHead->BTHandle[index]);
							HFILE FidOut;

							sprintf(txt, "Copy Index %i", index + 1);
							strcpy(OutFilePath, Arg[3]);
							pDot = strrchr(OutFilePath, '.');
							sprintf(pDot, ".in%i", index + 1);
							FidOut = GSSiOpenFile(OutFilePath, 0, OF_CREATE);
							totLen = GSSifilelength(IndexFid);
							GSSillseek(IndexFid, 0, 0);
							didRead = BigRead(IndexFid, pbuf, BUFFERSIZE);
							nRead = 0;
							while (didRead > 0)
							{
								BigWrite(FidOut, pbuf, didRead, -1);
								didRead = BigRead(IndexFid, pbuf, BUFFERSIZE);
								StatusWindowUpdate(NULL, txt, totLen, nRead += didRead);
							}
							GSSiClose(IndexFid);
							GSSiClose(FidOut);
						}
						*LastChr(Arg[2]) = 'p';
						if (FileType(Arg[2]))
						{
							strcpy(OutFilePath, Arg[3]);
							*LastChr(Arg[3]) = 'p';
							CopyFile(Arg[2], Arg[3], FALSE);
						}

						DestroyStatusWindow(0);
						GSSiGlobUlFree(&hbuf);
						rtn = TRUE;
					}
				}
			}
		}
		GlobalUnlock(hDB);
		CloseGWDatabase(hDB);
		return rtn;
	}
	else if (!stricmp(Arg[1], "COMPARE"))
	{
		int	NumIndexFields;

		hDB1 = OpenGWDatabase (Arg[2],BT_READ);
		if (!hDB1)
			return FALSE;
		hDB2 = OpenGWDatabase (Arg[3],BT_READ);
		if (!hDB2)
		{
			CloseGWDatabase (hDB1); 
			return -1;
		}
		lpGWDHead1 = GlobalLock (hDB1);
		NumIndexFields = lpGWDHead1->NumIndexFields[0] + 1;
		*DefStr = 0;
		for (i=0,lpFieldInfo=lpGWDHead1->pFldInfo;i<lpGWDHead1->NumIndexFields[0];i++,lpFieldInfo++)
		{
			FIELDINFO FieldInfo;

			strcpy (FieldInfo.name,lpFieldInfo->Name);
			FieldInfo.type = lpFieldInfo->Type;
			FieldInfo.length = lpFieldInfo->Len;
			CreateGMTextHeader (&FieldInfo, DefStr);
			_fstrcpy (_fstrchr (DefStr,0),",");
		}
		sprintf (strchr(DefStr,0),"FieldName(C64),Value1(C255),Value2(C255)");	
		GlobalUnlock (hDB1);
		rtn = CreateGWDDatabase (Arg[4],1,FALSE,0,NumIndexFields,DefStr);  
		hDB = OpenGWDatabase (Arg[4],BT_WRITE);
		if (hDB)
		{
			int	pos=BT_FIRST;
		    LPGWFLDINFO lpGWFldInfo;
			GWFLDINFO FieldInfo;
			short	index;
			char	Val1[256], Val2[256];
			int		nRecs, nChecked=0;

			lpGWDHead = GlobalLock (hDB);
			lpGWDHead1 = GlobalLock (hDB1);
			lpGWDHead2 = GlobalLock (hDB2);
			CreateStatusWind (hWndMain,1,"Compare GMD files");
			nRecs = BT_NUM_IN_INDEX (lpGWDHead1->BTHandle[0]);
			while (!BT_FIND (lpGWDHead1->BTHandle[0],lpGWDHead1->pKeys[0],pos,BT_ANY,(LPSTR)&Offset1)) 
			{
				pos = BT_NEXT;
		        FillGWDData (lpGWDHead1,Offset1);
				if (!BT_FIND (lpGWDHead2->BTHandle[0],lpGWDHead1->pKeys[0],BT_FIRST,BT_EQ,(LPSTR)&Offset2))
				{
			        FillGWDData (lpGWDHead2,Offset2);
    
					for (i=0,lpGWFldInfo=lpGWDHead1->pFldInfo;i<lpGWDHead1->NumFields;i++,lpGWFldInfo++)
					{
						GMDGetCharFieldVal (lpGWDHead1,i,Val1);
						if (i < lpGWDHead1->NumIndexFields[0])
							SetFieldValFromCharAndName(lpGWDHead, lpGWFldInfo->Name, Val1, FALSE, TRUE);
                		if (GWDGetFieldInfoFromName (hDB2,lpGWFldInfo->Name, &FieldInfo,&index))
						{
							GMDGetCharFieldVal (lpGWDHead2,index,Val2);
							if (strcmp (Val1,Val2))
							{
								SetFieldValFromCharAndName(lpGWDHead, "FieldName", lpGWFldInfo->Name, FALSE, TRUE);
								SetFieldValFromCharAndName(lpGWDHead, "Value1", Val1, FALSE, TRUE);
								SetFieldValFromCharAndName(lpGWDHead, "Value2", Val2, FALSE, TRUE);
							    NumDiffs++;
						        GWDAddRecord (lpGWDHead,0,NULL);
							}
						}
					}
				}
				StatusWindowUpdate (NULL,NULL, nRecs, ++nChecked);
			}
			GlobalUnlock (hDB1);
			GlobalUnlock (hDB2);
			GlobalUnlock (hDB);
			CloseGWDatabase (hDB); 
			DestroyStatusWindow(0); 
		}
		CloseGWDatabase (hDB1); 
		CloseGWDatabase (hDB2); 
		rtn = TRUE;
		ltoa (NumDiffs,OutLoc,10);
	}
	else if (!stricmp(Arg[1], "COMPAREKEYS"))//$GMD(COMPAREKEYS,db1,db2,outfile) checks to see if all keys in db1 are in db2. Writes list of those which are not to outfile. Returns number of nomatch or -1 if error
	{
		int	NoMatch = 0;
		HFILE fidOut;

		hDB1 = OpenGWDatabase(Arg[2], BT_READ);
		if (!hDB1)
			return -1;
		hDB2 = OpenGWDatabase(Arg[3], BT_READ);
		if (!hDB2)
		{
			CloseGWDatabase(hDB1);
			return -1;
		}
		fidOut = GSSiOpenFile(Arg[4], 0, OF_CREATE);
		if (fidOut != HFILE_ERROR)
		{
			int	pos = BT_FIRST;
			int ID;
			char cID[16];
			LPGWFLDINFO lpGWFldInfo;
			GWFLDINFO FieldInfo;
			short	index;
			char	Val1[256], Val2[256];
			int		nRecs, nChecked = 0;

			lpGWDHead1 = GlobalLock(hDB1);
			lpGWDHead2 = GlobalLock(hDB2);
			CreateStatusWind(hWndMain, 1, "Compare GMD Keys");
			nRecs = BT_NUM_IN_INDEX(lpGWDHead1->BTHandle[0]);
			fputstring("KEY", fidOut);

			while (!BT_FIND(lpGWDHead1->BTHandle[0], lpGWDHead1->pKeys[0], pos, BT_ANY, (LPSTR)&Offset1))
			{
				pos = BT_NEXT;
				if (Offset1 >= 0 && BT_FIND(lpGWDHead2->BTHandle[0], lpGWDHead1->pKeys[0], BT_FIRST, BT_EQ, (LPSTR)&Offset2))
				{
					NoMatch++;
					ID = *(LPINT)lpGWDHead1->pKeys[0];
					itoa(ID, cID, 10);
					fputstring(cID,fidOut);
				}
				if (!(nChecked++ % 100))
					StatusWindowUpdate(NULL, NULL, nRecs, nChecked);
			}
			GlobalUnlock(hDB1);
			GlobalUnlock(hDB2);
			DestroyStatusWindow(0);
			GSSiClose(fidOut);
			rtn = TRUE;
		}
		CloseGWDatabase(hDB1);
		CloseGWDatabase(hDB2);
		ltoa(NoMatch, OutLoc, 10);
	}
	else if (!stricmp(Arg[1], "CHECKPOINTLOG"))
	{
		if (!stricmp (Arg[2],"CREATE"))
		{
			rtn = GMDCreateCheckPointLog (Arg[3]);
		}
		else if (!stricmp (Arg[2],"OPEN"))
		{
			rtn = GMDOpenJournal (Arg[3]);
		}
		else if (!stricmp (Arg[2],"CLOSE"))
		{
			rtn = GMDCloseJournal (Arg[3]);
		}
		ltoa (rtn,OutLoc,10);
	}
	else if (!stricmp(Arg[1], "CHECKINDEX"))
	{
		rtn = ValidateGMDIndexes(Arg[2],atoi(Arg[3]),atob(Arg[4]));
		ltoa(rtn, OutLoc, 10);
	}

	else if (!stricmp (Arg[1],"DUMPTOINDEX"))
	{
		hDB = OpenGWDatabase (Arg[2],BT_READ);
		if (hDB)
		{
			GWFLDINFO FieldInfo;
			int		nRecs;
			short	nLevels=2, nperLevel=64, nlast;
			int		iOffsetField;
			short	index;

           	if (GWDGetFieldInfoFromName (hDB,Arg[4],&FieldInfo,&index))
			{
				HFILE	FidIndex = GSSiOpenFile (Arg[3],0,OF_CREATE);

				if (FidIndex != HFILE_ERROR)
				{
					int pos = BT_FIRST;
					int	Offset, iOffset;
					typedef struct  {char key[10];short maxElev;int loc;}TINDEX;
					TINDEX tiTop[64], tiBot[64];
					short	ntop=0, nbot=0;
					int loctiTop;

					lpGWDHead = GlobalLock (hDB);
					nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
					nlast = nRecs % nperLevel;
					BigWrite (FidIndex,&nRecs,4,-1);
					BigWrite (FidIndex,&nperLevel,2,-1);
					BigWrite (FidIndex,&nLevels,2,-1);
					loctiTop = GSSillseek (FidIndex,0,1);
					BigWrite (FidIndex,&ntop,sizeof(ntop),-1);
					BigWrite (FidIndex,tiTop,sizeof(tiTop),-1);
					
					while (!BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],pos,BT_ANY,(LPSTR)&Offset)) 
					{
						if (pos == BT_FIRST)
						{
							pos = BT_NEXT;
							memmove (&tiTop[0].key,lpGWDHead->pKeys[0],10);
							ntop = 1;
							nbot = 0;
							tiTop[0].loc = GSSillseek (FidIndex,0,1);
						}
						FillGWDData (lpGWDHead,Offset);
						iOffset = GMDGetIntegerFieldVal (lpGWDHead,index);
						if (nbot == nperLevel)
						{
							BigWrite (FidIndex,&nbot,sizeof(nbot),-1);
							BigWrite (FidIndex,tiBot,sizeof(tiBot),-1);
							memmove (&tiTop[ntop].key,lpGWDHead->pKeys[0],10);
							tiTop[ntop++].loc = GSSillseek (FidIndex,0,1);
							nbot = 0;
						}
						memmove (&tiBot[nbot].key,lpGWDHead->pKeys[0],10);
						tiBot[nbot++].loc = iOffset;
					}
					if (nbot)
					{
						LPINT pLevX;

						BigWrite (FidIndex,&nbot,sizeof(nbot),-1);
						BigWrite (FidIndex,tiBot,sizeof(tiBot),-1);
						pLevX = (LPINT)&lpGWDHead->pKeys[0][6];
						(*pLevX)++;
						memmove (&tiTop[ntop].key,lpGWDHead->pKeys[0],10);
						tiTop[ntop++].loc = GSSillseek (FidIndex,0,1);
						nbot = 0;
					}
					GSSillseek (FidIndex,loctiTop,0);
					BigWrite (FidIndex,&ntop,sizeof(ntop),-1);
					BigWrite (FidIndex,tiTop,sizeof(tiTop),-1);
					GlobalUnlock (hDB);
					GSSiClose (FidIndex);
				}
			}
			CloseGWDatabase (hDB); 
			rtn = TRUE;
		}
	}
	return rtn;
}

BOOL GMDReorg (LPSTR Name,int IndexToReorgOn,BOOL Compress,BOOL Verify,HWND hWnd,LPSTR AddFieldDefs)
{
	HANDLE	hDB = OpenGWDatabase (Name,BT_WRITE); 
	HANDLE	hDBNew;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
    LPGWDHEADER lpGWDHead, lpGWDHeadNew;
    short		pos=BT_FIRST,NumNewIndex,i;  
    long		Offset, Offset2; 
    char		NewName[256], IndexName[256], NewIndexName[256];
    LPSTR		pDot=_fstrrchr (Name,'.');
    long		nRecs, nLoaded=0;  
    BOOL		rtn=FALSE;  
    LPSTR		str,lpTab;
	LPGWFLDINFO NewFieldInfo,lpGWFldInfo;
	HANDLE		hNewFields=0;   
	LPHANDLE	hSetClause;
	short		nNewFields=0, ibeg=0;   
	char		setclause[256];
		
	if (!hDB)
		return FALSE; 
	if (!pDot)  
	{
		CloseGWDatabase (hDB); 
		return FALSE;
	} 
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
    hDBNew = GSSiGlobAlloc (1509,GHND,sizeof (GWDHEADER)+lpGWDHead->Reclen+4);
    lpGWDHeadNew =(LPGWDHEADER) GlobalLock (hDBNew);
	*lpGWDHeadNew = *lpGWDHead;
	lpGWDHeadNew->hFldInfo = 0;
	if (*AddFieldDefs)
	{        
		hNewFields = GSSiGlobAlloc (1510,GHND,USHRT_MAX);  
		NewFieldInfo = (LPGWFLDINFO)GlobalLock (hNewFields);
		hSetClause = (LPHANDLE)(NewFieldInfo + 256);
		str = AddFieldDefs;
	 	if (*str == '(')
	 	{
	 		str++;
	 		if (*LastChr (str) == ')')
	 			*LastChr (str) = 0;
		}
		lpTab = str;	 		   
		ibeg = lpGWDHead->pFldInfo[lpGWDHead->NumFields-1].Beg + lpGWDHead->pFldInfo[lpGWDHead->NumFields-1].Len;  
		while (str)
		{
		 	if (!(lpTab = _fstrchr (str,'(')) || !GetFieldTypeAndLenFromChar (lpTab+1,&NewFieldInfo[nNewFields],&hSetClause[nNewFields]))
			{
				HANDLE	hMem = GSSiGlobAlloc (1511,GMEM_MOVEABLE,4096);
				LPSTR	pMess=GlobalLock (hMem);
		
				sprintf (pMess,"Error in Table Definition String:Field (%s)",lpTab); 	
				GSSiMessageBox (0,pMess,Name,MB_ICONEXCLAMATION,0); 
				GSSiGlobUlFree (&hMem);
				GSSiGlobUlFree (&hNewFields);
			    GlobalUnlock (hDB);
				CloseGWDatabase (hDB); 
				goto Exit;
			}
			*lpTab++ = 0;
			if (*str == '"')   
				str++;
			_fstrcpy (NewFieldInfo[nNewFields].Name,str);
			NewFieldInfo[nNewFields].Beg = ibeg; 
			ibeg += NewFieldInfo[nNewFields++].Len;  
			lpGWDHeadNew->NumFields++;  
			if ((str = MatchLev (lpTab,',')))
				str++;
		}    
		lpGWDHeadNew->Reclen = ibeg;
	    lpGWDHeadNew->hFldInfo = GSSiGlobAlloc (1512,GHND,lpGWDHeadNew->NumFields*sizeof(FIELDINFO));
	    lpGWDHeadNew->pFldInfo = (LPGWFLDINFO)GlobalLock(lpGWDHeadNew->hFldInfo); 
        for (i=0;i<lpGWDHead->NumFields;i++)
        	lpGWDHeadNew->pFldInfo[i] = lpGWDHead->pFldInfo[i];
        for (i=0;i<nNewFields;i++)
        	lpGWDHeadNew->pFldInfo[lpGWDHead->NumFields + i] = NewFieldInfo[i];
	}
	*pDot = 0; 
	sprintf (NewName,"%s__REORG__.gmd",Name);  
	*pDot = '.';
	CreateGWDDatabase (NewName,0,Compress,-1,0,(LPSTR) lpGWDHeadNew);  
	GSSiGlobUlFree (&lpGWDHeadNew->hFldInfo);
	GSSiGlobUlFree (&hDBNew);
    GlobalUnlock (hDB);
	CloseGWDatabase (hDB); 
    hDB = 0;
	OpenDataFile (Name,"",BT_READ,&hDB);
    SQLPtr = (LPOPENSQLDATA) GlobalLock (hDB);
    FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
	GMDSwitchToMemFile (FilePtr->FileHandle,30000000,5000000);
	lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 

	hDBNew = OpenGWDatabase (NewName,BT_WRITE);
	lpGWDHeadNew = (LPGWDHEADER)GlobalLock (hDBNew);   
	NumNewIndex = lpGWDHeadNew->NumIndex;
	if (!hWnd)
		hWnd = hWndMain; 
	CreateStatusWind (hWnd,1,"Reorganize GMD file");
    nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[IndexToReorgOn]);
	StatusWindowUpdate (NULL,Name, nRecs, 0);  
    while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[IndexToReorgOn],lpGWDHead->pKeys[IndexToReorgOn],pos,BT_ANY,(LPSTR)&Offset))
    {    
        pos = BT_NEXT;  
        SQLPtr->st = 0;
        SQLPtr->Offset = Offset;
        SQLPtr->lastreadtime = LONG_MAX;  
        SQLPtr->NumGlobals = 0;
        FillGWDData (lpGWDHead,Offset);
        hmemmove ((HPSTR)&lpGWDHeadNew->GWDData,(HPSTR)&lpGWDHead->GWDData,lpGWDHead->Reclen); 
        for (i=0;i<nNewFields;i++)
        {
        	if (hSetClause[i])
        	{   
        		LPSTR pStr=GlobalLock (hSetClause[i]);
        		
        		_fstrcpy (setclause,pStr);
        		GlobalUnlock (hSetClause[i]);
        		ExpandText (setclause);
				SetFieldValFromCharAndName(lpGWDHeadNew, NewFieldInfo[i].Name, setclause, FALSE, TRUE);
        	}
        }  
        GWDAddRecord (lpGWDHeadNew,0,NULL);
		SetContinueProcessing(StatusWindowUpdate(NULL, NULL, nRecs, ++nLoaded));
    }
	GlobalUnlock (FilePtr->FileHandle); 
	GlobalUnlock (SQLPtr->OFHandle);
    GlobalUnlock (hDB);
    CloseDataFile (TRUE, &hDB);
    GlobalUnlock (hDBNew);
	CloseGWDatabase (hDBNew); 
    if (ContinueProcessing)
    {
	    for (i=0;i<NumNewIndex;i++)
	    {  
	    	LPSTR	lpDot;
	    	
	        _fstrcpy (IndexName,NewName);
	        ExpandText (IndexName);
	        _fstrlwr (IndexName); 
	        lpDot = _fstrrchr (IndexName,'.');
	        sprintf (lpDot,".in%i",i+1);
			StatusWindowUpdate ("Restructure Index",IndexName, nRecs, 0);
			if (!ReorgBTree (IndexName, GetDlgItem(PrintMsgWnd,PRINT_VIEW)))
			{
				SetContinueProcessing (FALSE);
				break;
			}
		}
	}  
	rtn = ContinueProcessing;
    SetContinueProcessing ( TRUE);  
    if (rtn && Verify)
    {   
    	hDB = OpenGWDatabase (Name,BT_WRITE);
		hDBNew = OpenGWDatabase (NewName,BT_WRITE);
		lpGWDHeadNew = (LPGWDHEADER)GlobalLock (hDBNew);   
		lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
		pos = BT_FIRST;
		nLoaded = 0;   
		StatusWindowUpdate ("Verifying Compression",Name, nRecs, 0);
	    while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],pos,BT_ANY,(LPSTR)&Offset)) 
	    {
	    	if (BT_FIND (lpGWDHeadNew->BTHandle[0],lpGWDHeadNew->pKeys[0],pos,BT_ANY,(LPSTR)&Offset2))
	    		SetContinueProcessing (FALSE);
	    	else
	    	{  
	            FillGWDData (lpGWDHead,Offset);
		        FillGWDData (lpGWDHeadNew,Offset2);
                if (_fmemcmp (&lpGWDHead->GWDData,&lpGWDHeadNew->GWDData,(size_t)lpGWDHead->Reclen))
                	SetContinueProcessing (FALSE);
	    	} 
	    	pos = BT_NEXT;
			StatusWindowUpdate (NULL,NULL, nRecs, ++nLoaded);
	    }
	    rtn = ContinueProcessing;
	    SetContinueProcessing ( TRUE);  
	    GlobalUnlock (hDB);
	    GlobalUnlock (hDBNew);
		CloseGWDatabase (hDB); 
		CloseGWDatabase (hDBNew); 
    }
    if (rtn)
    {
	    for (i=0;i<NumNewIndex;i++)
	    {  
	    	LPSTR	lpDot;
	    	
	        _fstrcpy (IndexName,Name);
	        ExpandText (IndexName);
	        _fstrlwr (IndexName); 
	        lpDot = _fstrrchr (IndexName,'.');
	        sprintf (lpDot,".in%i",i+1);
	        _fstrcpy (NewIndexName,NewName);
	        ExpandText (NewIndexName);
	        _fstrlwr (NewIndexName); 
	        lpDot = _fstrrchr (NewIndexName,'.');
	        sprintf (lpDot,".in%i",i+1); 
	        GSSiRemove (IndexName);
	        GSSiRename (NewIndexName,IndexName);
		}
        GSSiRemove (Name);
        GSSiRename (NewName,Name);
    }
	DestroyStatusWindow(0); 
Exit: 
    for (i=0;i<nNewFields;i++)
    	GSSiGlobFree (&hSetClause[i]);
	GSSiGlobUlFree (&hNewFields);
	return rtn;
}
    
BOOL GMDCopyFile (LPSTR ToFile,LPSTR FromFile,LPSTR SQL)
{
	HANDLE	hDB = OpenGWDatabase (ToFile,BT_WRITE); 
	HANDLE	hDBFrom=0;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
    LPGWDHEADER lpGWDHead, lpGWDHeadFrom;
    long		Offset, Offset2; 
    BOOL		rtn=FALSE;  
    LPSTR		str,lpTab;
		
	if (!hDB)
		return FALSE; 
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
	if (!OpenDataFile (FromFile,SQL,BT_READ,&hDBFrom))
		goto Exit;
    SQLPtr = (LPOPENSQLDATA) GlobalLock (hDBFrom);
    FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
	lpGWDHeadFrom = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 

    while (FetchDBRec (hDBFrom))
    {    
        hmemmove ((HPSTR)&lpGWDHead->GWDData,(HPSTR)&lpGWDHeadFrom->GWDData,lpGWDHead->Reclen); 
        GWDAddRecord (lpGWDHead,0,NULL); 
    }
	GlobalUnlock (FilePtr->FileHandle); 
	GlobalUnlock (SQLPtr->OFHandle);
    GlobalUnlock (hDBFrom);
    CloseDataFile (TRUE, &hDBFrom); 
Exit:
    GlobalUnlock (hDB);
	CloseGWDatabase (hDB); 
	return rtn;
}
    
int GWDAddRecord (LPGWDHEADER lpGWDHead,long length,LPSHORT IndexArray)//returns 1 if new rec or 2 if replace. Always writes to end of primary file.
#if ENABLETRACE
{GSSiEnterProg (649);
#endif
{
    long    Offset,ii;  
    USHORT	ShortLength;
    short   Index; 
    BOOL	CreateIndexEntry;   
	int		rtn = 1;
    
    Offset = GSSillseek (lpGWDHead->Fid,0,2);  
    if (Offset == debugoff)
    	ii=1;
    if (!length)
	{
        length = lpGWDHead->Reclen; 
		while (length > 1 && lpGWDHead->GWDData[length-1] == 0)
			length--;
	}
    if (lpGWDHead->Compressed)
    {
	    HANDLE	hCompressedRec = GSSiGlobAlloc (1513,GMEM_MOVEABLE,length+32);
	    HPSTR	pCompressedRec = GlobalLock (hCompressedRec); 
	    long	CompressedLength = CompressBinaryRecord ((HPSTR)&lpGWDHead->GWDData,pCompressedRec,length);   
	    
	    BigWrite (lpGWDHead->Fid,(HPSTR)&CompressedLength,4,-1);
	    BigWrite (lpGWDHead->Fid,(HPSTR)pCompressedRec,CompressedLength,-1);
	    GSSiGlobUlFree (&hCompressedRec); 
	}
	else 
	{   
		ShortLength = length;
	    BigWrite (lpGWDHead->Fid,(HPSTR)&ShortLength,2,-1);
	    BigWrite (lpGWDHead->Fid,(HPSTR)&lpGWDHead->GWDData,length,-1); //&lpGWDHead->GWDData[150]
	}
	if (lpGWDHead->SplitFile)
		Offset += lpGWDHead->SplitLength;
    SetGWDCurrentOffset (lpGWDHead,-1);
    for (Index=0;Index<lpGWDHead->NumIndex;Index++)
    {   
    	if (IndexArray)
    		CreateIndexEntry = *IndexArray++;
    	else
    		CreateIndexEntry = TRUE;
    		    
		if (Index && Index == lpGWDHead->SpatialIndex)
		{
			switch (lpGWDHead->SpatialIndexType)
			{
				case 1:
				case 2:
				{
					LPGWFLDINFO pFldInfo;
					int	Time, FromMonth, ToMonth, Month;

					pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->FromDateField;
 					Time = *(LPLONG)&lpGWDHead->GWDData[pFldInfo->Beg];
					FromMonth = SysMonthFromSymTime (Time);
					pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->ToDateField;
 					Time = *(LPLONG)&lpGWDHead->GWDData[pFldInfo->Beg];
					ToMonth = SysMonthFromSymTime (Time);
					for (Month = FromMonth;Month <= ToMonth;Month++)
					{
						if (GWDFormKey(lpGWDHead,Index,FALSE,0,Month))
							BT_PUT (lpGWDHead->BTHandle[Index],lpGWDHead->pKeys[Index],(LPSTR)&Offset);
					}
				}
				continue;
			default:
				break;
			}
		}
        if ((!Index || CreateIndexEntry) && GWDFormKey(lpGWDHead,Index,FALSE,lpGWDHead->Reclen,0))
        {
        	
        	if (!BT_PUT (lpGWDHead->BTHandle[Index],lpGWDHead->pKeys[Index],(LPSTR)&Offset))
			{
				if (!Index)
					rtn = 2;
			}
        }
    }

{
#if ENABLETRACE
GSSiExitProg (649);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

int GWDReplaceRecord (LPGWDHEADER lpGWDHead,long UnCompressedLength,LPSHORT IndexArray, long Offset)
#if ENABLETRACE
{GSSiEnterProg (650);
#endif
{
    short   Index, ShortLength; 
    BOOL	CreateIndexEntry,WriteDeleteLen=FALSE;
    HANDLE	hSaveRec; 
    HPSTR	pSaveRec, pWriteRec; 
    long	oldlength, length; 
    long	OldOffset,ii;
    HANDLE	hCompressedRec=0;
    HPSTR	pCompressedRec; 
    long	CompressedLength;  
	int		rtn=0;
    
    if (Offset == debugoff)         //lpGWDHead->pFldInfo[7]
    	ii=1;
    if (!UnCompressedLength)
        UnCompressedLength = lpGWDHead->Reclen;  
    length = UnCompressedLength;
    SetGWDCurrentOffset (lpGWDHead,-1);
    if (Offset < 0) //find the record
    {
		if (GWDFormKey(lpGWDHead, 0, FALSE, length, 0))
		{
			if (BT_FIND(lpGWDHead->BTHandle[0], lpGWDHead->pKeys[0], BT_FIRST, BT_EQ, (LPSTR)&Offset))
			{
				rtn = GWDAddRecord(lpGWDHead, length, IndexArray);
				goto Exit;
			}
		}
		else
			goto Exit;
    }  
	rtn = 2;
    hSaveRec = GSSiGlobAlloc ( 282,GMEM_MOVEABLE,length);
    pSaveRec = GlobalLock (hSaveRec);
    hmemmove (pSaveRec,(HPSTR)lpGWDHead->GWDData,length);
    SetGWDCurrentOffset (lpGWDHead,-1);
	oldlength = FillGWDData (lpGWDHead,Offset);

	if (oldlength > 0)
	{
		for (Index=1;Index<lpGWDHead->NumIndex;Index++)
		{   
			if (Index && Index == lpGWDHead->SpatialIndex)
			{
				switch (lpGWDHead->SpatialIndexType)
				{
					case 1:
					case 2:
					{
						LPGWFLDINFO pFldInfo;
						int	Time, FromMonth, ToMonth, Month;

						pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->FromDateField;
 						Time = *(LPLONG)&lpGWDHead->GWDData[pFldInfo->Beg];
						FromMonth = SysMonthFromSymTime (Time);
						pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->ToDateField;
 						Time = *(LPLONG)&lpGWDHead->GWDData[pFldInfo->Beg];
						ToMonth = SysMonthFromSymTime (Time);
						for (Month = FromMonth;Month <= ToMonth;Month++)
						{
							if (GWDFormKey(lpGWDHead,Index,FALSE,oldlength,Month))
							{
        						if (!BT_FIND (lpGWDHead->BTHandle[Index],lpGWDHead->pKeys[Index],BT_FIRST,BT_EQ,(LPSTR)&OldOffset))
								{
	        						BT_DELETE (lpGWDHead->BTHandle[Index],lpGWDHead->pKeys[Index],(LPSTR)&OldOffset,FALSE);
								}
							}
						}
					}
					continue;
				default:
					break;
				}
			}
			if (GWDFormKey(lpGWDHead,Index,FALSE,oldlength,0))
			{
        		if (!BT_FIND (lpGWDHead->BTHandle[Index],lpGWDHead->pKeys[Index],BT_FIRST,BT_EQ,(LPSTR)&OldOffset))
				{
	        		BT_DELETE (lpGWDHead->BTHandle[Index],lpGWDHead->pKeys[Index],(LPSTR)&OldOffset,FALSE);
				}
			}
		}
    }
    hmemmove ((HPSTR)&lpGWDHead->GWDData,pSaveRec,length); 
    GSSiGlobUlFree (&hSaveRec);
    pWriteRec = lpGWDHead->GWDData;
    if (lpGWDHead->Compressed)
    {
	    hCompressedRec = GSSiGlobAlloc (1514,GMEM_MOVEABLE,length+32);
	    pCompressedRec = GlobalLock (hCompressedRec); 
	    CompressedLength = CompressBinaryRecord ((HPSTR)&lpGWDHead->GWDData,pCompressedRec,length); 
	    pWriteRec = pCompressedRec;
	    length = CompressedLength;
	}  
	else //for uncompresses split files
	{
		pCompressedRec = (HPSTR)&lpGWDHead->GWDData;
		CompressedLength = length;
	}
    if (lpGWDHead->SplitFile || length > LastGMDRecordLength || (length < LastGMDRecordLength && length > LastGMDRecordLength -4))
    {
		if (!lpGWDHead->SplitFile && lpGWDHead->Compressed)
	    {    
	    	long	dellength;
	    	
		    GSSillseek (lpGWDHead->Fid,Offset,0);
		    if (BigRead (lpGWDHead->Fid,(HPSTR)&dellength,4) != 4)
		    {   
		    	MessageBox (0,"Error writing to gmd file",NULL,MB_ICONEXCLAMATION);
			    GSSiGlobUlFree (&hCompressedRec);
		    	goto Exit;
		    } 
		    dellength = -dellength;   
		    GSSillseek (lpGWDHead->Fid,Offset,0);
		    BigWrite (lpGWDHead->Fid,(HPSTR)&dellength,4,-1);
	    }
	    Offset = GSSillseek (lpGWDHead->Fid,0,2);  
	}
	else 
	{
		GSSillseek (lpGWDHead->Fid,Offset,0); 
		if (length < LastGMDRecordLength)
			WriteDeleteLen = TRUE;
	}
	if (lpGWDHead->SplitFile || lpGWDHead->Compressed)
    {
	    BigWrite (lpGWDHead->Fid,(HPSTR)&CompressedLength,4,-1);
	    BigWrite (lpGWDHead->Fid,(HPSTR)pCompressedRec,CompressedLength,-1);
	    GSSiGlobUlFree (&hCompressedRec);
	    if (WriteDeleteLen)
	    {
	    	long NumDeleteBytes = LastGMDRecordLength - CompressedLength - 4; 
            
            if (NumDeleteBytes)
            	NumDeleteBytes = -NumDeleteBytes;
		    BigWrite (lpGWDHead->Fid,(HPSTR)&NumDeleteBytes,4,-1);
		}
	}
	else 
	{   
		ShortLength = length;
	    BigWrite (lpGWDHead->Fid,(HPSTR)&ShortLength,2,-1);
	    BigWrite (lpGWDHead->Fid,(HPSTR)&lpGWDHead->GWDData,length,-1); 
	}

    for (Index=0;Index<lpGWDHead->NumIndex;Index++)
    {   
    	if (IndexArray)
    		CreateIndexEntry = *IndexArray++;
    	else
    		CreateIndexEntry = TRUE;
    		    
		if (Index && Index == lpGWDHead->SpatialIndex)
		{
			switch (lpGWDHead->SpatialIndexType)
			{
				case 1:
				case 2:
				{
					LPGWFLDINFO pFldInfo;
					int	Time, FromMonth, ToMonth, Month;

					pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->FromDateField;
 					Time = *(LPLONG)&lpGWDHead->GWDData[pFldInfo->Beg];
					FromMonth = SysMonthFromSymTime (Time);
					pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->ToDateField;
 					Time = *(LPLONG)&lpGWDHead->GWDData[pFldInfo->Beg];
					ToMonth = SysMonthFromSymTime (Time);
					for (Month = FromMonth;Month <= ToMonth;Month++)
					{
						GWDFormKey(lpGWDHead,Index,FALSE,0,Month);
						BT_PUT (lpGWDHead->BTHandle[Index],lpGWDHead->pKeys[Index],(LPSTR)&Offset);
					}
				}
				continue;
			default:
				break;
			}
		}
        if ((!Index || CreateIndexEntry) && GWDFormKey(lpGWDHead,Index,FALSE,UnCompressedLength,0))
        	BT_PUT (lpGWDHead->BTHandle[Index],lpGWDHead->pKeys[Index],(LPSTR)&Offset);
    }
Exit:
{
#if ENABLETRACE
GSSiExitProg (650);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL GWDDeleteRecord (LPGWDHEADER lpGWDHead,long Offset)
#if ENABLETRACE
{GSSiEnterProg (651);
#endif
{
	short   Index;
	BOOL	CreateIndexEntry;
	long	length;
	short	ShortLen;
	long	OldOffset, ii, SaveOffset;
	BOOL	rtn = FALSE;

	SetGWDCurrentOffset(lpGWDHead, -1);
	length = FillGWDData(lpGWDHead, Offset);
	if (length == -2)
	{
		rtn = TRUE;
		goto Exit;
	}
	if (length == -1)
		goto Exit;
	SaveOffset = GSSillseek(lpGWDHead->Fid, 0, 1);
    if (GSSillseek (lpGWDHead->Fid,Offset,0) == HFILE_ERROR)
		goto Exit;
	if (lpGWDHead->Compressed)
    {    
    	long	dellength;
    	
		if (BigRead(lpGWDHead->Fid, (HPSTR)&dellength, 4) != 4)
			goto Exit;
	    dellength = -dellength;   
	    GSSillseek (lpGWDHead->Fid,Offset,0);
	    BigWrite (lpGWDHead->Fid,(HPSTR)&dellength,4,-1);
    }
    else
    {
	    BigRead (lpGWDHead->Fid,(HPSTR)&ShortLen,2); 
        ShortLen = -ShortLen;
	    BigWrite (lpGWDHead->Fid,(HPSTR)&ShortLen,2,-1);  
	}
	GSSillseek (lpGWDHead->Fid,SaveOffset,0);
    for (Index=0;Index<lpGWDHead->NumIndex;Index++)              // *(LPLONG)lpGWDHead->pKeys[Index]
    {   
		if (Index && Index == lpGWDHead->SpatialIndex)
		{
			switch (lpGWDHead->SpatialIndexType)
			{
				case 1:
				case 2:
				{
					LPGWFLDINFO pFldInfo;
					int	Time, FromMonth, ToMonth, Month;

					pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->FromDateField;
 					Time = *(LPLONG)&lpGWDHead->GWDData[pFldInfo->Beg];
					FromMonth = SysMonthFromSymTime (Time);
					pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->ToDateField;
 					Time = *(LPLONG)&lpGWDHead->GWDData[pFldInfo->Beg];
					ToMonth = SysMonthFromSymTime (Time);
					for (Month = FromMonth;Month <= ToMonth;Month++)
					{
						if (GWDFormKey(lpGWDHead,Index,FALSE,length,Month))
						{
        					if (!BT_FIND (lpGWDHead->BTHandle[Index],lpGWDHead->pKeys[Index],BT_FIRST,BT_EQ,(LPSTR)&OldOffset))
							{
	        					BT_DELETE (lpGWDHead->BTHandle[Index],lpGWDHead->pKeys[Index],(LPSTR)&OldOffset,FALSE);
							}
						}
					}
				}
				continue;
			default:
				break;
			}
		}
        if (GWDFormKey(lpGWDHead,Index,FALSE,length,0))
        {
        	if (!BT_FIND (lpGWDHead->BTHandle[Index],lpGWDHead->pKeys[Index],BT_FIRST,BT_EQ,(LPSTR)&OldOffset))
	        	BT_DELETE (lpGWDHead->BTHandle[Index],lpGWDHead->pKeys[Index],(LPSTR)&OldOffset,FALSE);
	    }
    }
	rtn = TRUE;
Exit:  
{
#if ENABLETRACE
GSSiExitProg (651);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
} 

BOOL SelectFieldType (LPSTR Name,LPSTR ftype)
{   
	MessageBox (0,"Invalid field",Name,MB_ICONEXCLAMATION);
	return FALSE;
}

BOOL CreateGMTextHeader (LPFIELDINFO lpFieldInfo, LPSTR lpHead)
#if ENABLETRACE
{GSSiEnterProg (652);
#endif
{   
	short	l,ii;
	
	switch (lpFieldInfo->type)
	{
        default:
    		sprintf (_fstrchr(lpHead,0),"\"%s(NS|%i|%i)\"",lpFieldInfo->name,lpFieldInfo->type,lpFieldInfo->length); 
    		break; 
		case SQL_MSSHAPE:
    		lpFieldInfo->length = 4;
			break;
    	case SQL_LONGVARBINARY:
    		lpFieldInfo->length = 512;
    		break;	
    	case SQL_LONGVARCHAR:
    		lpFieldInfo->length = 512;	
		case SQL_GUID:
        case SQL_CHAR: 
        case SQL_VARCHAR:
		case SQL_UNKCHAR:
		case BT_RIGHT_CHAR:
		case BT_CHAR: 
		case SQL_BIGINT:
			if (lpFieldInfo->precision)
				l = min (lpFieldInfo->length,lpFieldInfo->precision);
			else
				l = lpFieldInfo->length; 
			if (!l)
			{   
				char	ftype[8];
				
				if (!SelectFieldType (lpFieldInfo->name,ftype))
{
#if ENABLETRACE
GSSiExitProg (652);
#endif
    				return FALSE;
}
    			sprintf (_fstrchr(lpHead,0),"\"%s(%s)\"",lpFieldInfo->name,ftype);
			}
			else
    			sprintf (_fstrchr(lpHead,0),"\"%s(C%i)\"",lpFieldInfo->name,l);
    		break;
    	case SQL_TIMESTAMP:
    		l = 19; 
    		sprintf (_fstrchr(lpHead,0),"\"%s(C%i)\"",lpFieldInfo->name,l);
    		break; 
		case SQL_NUMERIC:
		case SQL_DECIMAL:
    		sprintf (_fstrchr(lpHead,0),"\"%s(C%i)\"",lpFieldInfo->name,lpFieldInfo->length);
    		break;
        case SQL_INTEGER:
        case SQL_TINYINT:
        case SQL_SMALLINT:
    	case BT_INTEGER:
    	case BT_INT2:
    	case BT_INT4: 
    	case SQL_BIT:
    		sprintf (_fstrchr(lpHead,0),"\"%s(B%i)\"",lpFieldInfo->name,max(2,lpFieldInfo->length));
    		break;
    	case BT_REAL:
    	case BT_REAL4:
    	case BT_REAL8:
        case SQL_FLOAT:
		case SQL_REAL:
		case SQL_DOUBLE:
    		sprintf (_fstrchr(lpHead,0),"\"%s(R%i)\"",lpFieldInfo->name,lpFieldInfo->length);
    		break;
    }
{
#if ENABLETRACE
GSSiExitProg (652);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}  

short GMDHeaderToTableDef (LPSTR DefStr,LPSTR TableDef)
{
	 LPSTR	str = DefStr, lpTab;
	 short	Len, Type;   
	 short	NumFields = 0; 
	 char	Name[66], CType[66];
     
     if (*str == '(')
     {
     	str++;
     	*LastChr(str) = 0;
     }
	 while ((lpTab = _fstrchr (str,'(')))
 	 {   
 		*lpTab++=0;
 		switch (*lpTab)
 		{
			default:
	 		case 'C':
				Type = BT_CHAR;
			break;
			case 'B':
				Type = BT_INTEGER; 
			break;
			case 'R':
				Type = BT_REAL;
			break;
			case 'J':
				Type = BT_RIGHT_CHAR;
			break;
		}
		lpTab++;
		Len = atoi(lpTab);
		if (*str == '"')   
			str++;
		_fstrcpy (Name,str);
		NumFields++;  
		str = _fstrchr (lpTab,',');
		if (!str)
			str = _fstrchr (lpTab,0);
		else
			str++;
        if(*TableDef)
        	_fstrcat (TableDef,",");
	    switch (Type)
	    {   
	    	case SQL_LONGVARBINARY:
	        	Len = 512;
	            sprintf (CType,"varchar(%i)",min(254,Len));
    		break;
	    	case SQL_MSSHAPE:
	        	Len = 512;
	            sprintf (CType,"msshape(%i)",min(254,Len));
    		break;
	        default: 
	        case SQL_LONGVARCHAR:
	        	Len = 512;
	        case SQL_CHAR: 
	        case SQL_VARCHAR:
			case SQL_UNKCHAR:
	        case BT_RIGHT_CHAR:
	        case BT_CHAR:   
	        case SQL_BIGINT:
	            sprintf (CType,"varchar(%i)",min(254,Len));
	        break;
			
			case SQL_TINYINT:			        
	        case SQL_INTEGER:
	        case SQL_SMALLINT:    
	        case SQL_BIT:
	        case BT_INTEGER:
	            if (Len == 2)
	                _fstrcpy (CType,"smallint");
	            else
	                _fstrcpy (CType,"integer");
	        break;
						        
	        case SQL_NUMERIC:
	        case SQL_FLOAT:
			case SQL_REAL:
			case SQL_DOUBLE:
	        case BT_REAL:
	            _fstrcpy (CType,"float");
	        break;
	    }
   		sprintf (_fstrchr(TableDef,0),"\"%s\" %s",Name,CType);
	}
	return NumFields;
}

BOOL SetGMDFileLength (int mainLen,int indexLen)
{
	updateGMDlenMain = mainLen;
	updateGMDlenIndex = indexLen;
	return TRUE;
}

BOOL CreateGWDDatabase (LPSTR InName,int Version,BOOL Compress,int NumFields,int NumIndexFields,LPSTR DefStr)
#if ENABLETRACE
{GSSiEnterProg (653);
#endif
{
	LPGWDHEADER	lpGWDHead; 
	GWDHEADER GWDHead; 
	GWFLDINFO GWFldInfo;
    LPGWFLDINFO pFldInfo;
	LPGWFLDINFO	lpGWFldInfo;
	LPGWFLDINFO	lpFieldInfo;   
	OFSTRUCTGM	OFStruct;
	short	i, ifield;     
	UINT	ibeg;
	HFILE	FidData;  
	HANDLE	hVars;
	LPBTVARDESC pVars; 
	LPSTR	str, lpTab, pDot; 
	char	Name[260];
	BOOL	DoClose;
	HANDLE	hDB;

	memset (&GWDHead,0,sizeof(GWDHEADER));
	DBoundsInit (&GWDHead.FileBounds);
    _fstrcpy (Name,InName);
    _fstrlwr (Name);
    if (!StringEndsWith (Name,".gmd"))
{
#if ENABLETRACE
GSSiExitProg (653);
#endif
    	return FALSE;
}
    if ((FidData = GSSiOpenFile (Name,&OFStruct,OF_CREATE))==HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (653);
#endif
    	return FALSE;
}   
	if (NumFields == -1)
	{
		GWDHead = *(LPGWDHEADER) DefStr;  
//		GWDHead16 = GWDHEADER32toGWDHEADER16 (&GWDHead);
//        BigWrite (FidData,(HPSTR)&GWDHead16 ,sizeof(GWDHEADER16),-1);
		GWDHead.StoredAs32 = 1;
        BigWrite (FidData,(HPSTR)&GWDHead ,sizeof(GWDHEADER),-1);
		NumIndexFields = GWDHead.NumIndexFields[0];
		hVars = LocalAlloc (LHND,NumIndexFields * sizeof(BTVARDESC));
		pVars = (BTVARDESC *) LocalLock(hVars);
		ibeg = 0;
		for (i=0;i<NumIndexFields;i++,pVars++) 
		{
			pFldInfo = GWDHead.pFldInfo + GWDHead.IndexFields[0][i];
			pVars->BT_VARLEN=pFldInfo->Len;
			pVars->BT_VARTYP=pFldInfo->Type;
			pVars->BT_VAROFF=ibeg;
			ibeg += pFldInfo->Len;
		}
	    for (i=0,lpFieldInfo=GWDHead.pFldInfo;i<GWDHead.NumFields;i++,lpFieldInfo++)
			BigWrite (FidData,(HPSTR)lpFieldInfo,sizeof(GWFldInfo),-1);   
	}
	else if (NumIndexFields == 0)
	{
		LPFILEPATH  FilePathPtr; 
		LPHANDLE	lpFileHandle; 
		LPOPENFILEDATA	FilePtr;
		LPOPENSQLDATA	SQLPtr;
		int			isql;
		
		if (!_fstrchr (DefStr,'.'))
		{
			if (!FilePathHandle)
{
#if ENABLETRACE
GSSiExitProg (635);
#endif
				return FALSE;
}       
			DoClose = FALSE;
			FilePathPtr = (LPFILEPATH)GlobalLock (FilePathHandle);
			
			isql = FilePathPtr->NumFiles;	
			while (isql--)
			{
				lpFileHandle = &FilePathPtr->FileHandle;  
				lpFileHandle += isql;
				if (*lpFileHandle)
				{   
					SQLPtr = (LPOPENSQLDATA)GlobalLock(*lpFileHandle);
					FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
					hDB = FilePtr->FileHandle;
					if (!_fstricmp (DefStr,SQLPtr->IDName))
					{   
						GlobalUnlock(*lpFileHandle);
						GlobalUnlock (SQLPtr->OFHandle);
						GlobalUnlock (FilePathHandle);
						goto FoundFile;
					}
					GlobalUnlock(*lpFileHandle);
					GlobalUnlock (SQLPtr->OFHandle);
				}
			}
			GlobalUnlock (FilePathHandle);
{
#if ENABLETRACE
GSSiExitProg (635);
#endif
			return FALSE;
}
	}
	else
	{
		hDB = OpenGWDatabase (DefStr,BT_READ);

		if (!hDB)
		{
			GSSiClose (FidData);
			GSSiRemove (Name);
			
{
#if ENABLETRACE
GSSiExitProg (653);
#endif
			return FALSE;
}       
		}
	}
		DoClose = TRUE;
FoundFile:
		lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);        
		GWDHead = *lpGWDHead;  
//		GWDHead16 = GWDHEADER32toGWDHEADER16 (&GWDHead);
//		Compress = GWDHead16.Compressed;
//        BigWrite (FidData,(HPSTR)&GWDHead16 ,sizeof(GWDHEADER16),-1);
		Compress = GWDHead.Compressed;
		GWDHead.StoredAs32 = 1;
        BigWrite (FidData,(HPSTR)&GWDHead ,sizeof(GWDHEADER),-1);
		NumIndexFields = GWDHead.NumIndexFields[0];
		hVars = LocalAlloc (LHND,NumIndexFields * sizeof(BTVARDESC));
		pVars = (BTVARDESC *) LocalLock(hVars);
		ibeg = 0;
		for (i=0;i<NumIndexFields;i++,pVars++) 
		{
			pFldInfo = GWDHead.pFldInfo + GWDHead.IndexFields[0][i];
			pVars->BT_VARLEN=pFldInfo->Len;
			pVars->BT_VARTYP=pFldInfo->Type;
			pVars->BT_VAROFF=ibeg;
			ibeg += pFldInfo->Len;
		}
	    for (i=0,lpFieldInfo=GWDHead.pFldInfo;i<GWDHead.NumFields;i++,lpFieldInfo++)
			BigWrite (FidData,(HPSTR)lpFieldInfo,sizeof(GWFldInfo),-1);   
		GlobalUnlock (hDB); 
		if (DoClose)
			CloseGWDatabase (hDB);
	}
	else
	{
		GWDHead.NumFields=0;
		GWDHead.NumIndex=1;
	 	GWDHead.Version=Version;
		GWDHead.FileVersion = 1;
		GWDHead.NumIndexFields[0]=NumIndexFields;
		for (i=0;i<NumIndexFields;i++)
			GWDHead.IndexFields[0][i]=i;
//		GWDHead16 = GWDHEADER32toGWDHEADER16 (&GWDHead);
//        BigWrite (FidData,(HPSTR)&GWDHead16 ,sizeof(GWDHEADER16),-1);
		GWDHead.StoredAs32 = 1;
        BigWrite (FidData,(HPSTR)&GWDHead ,sizeof(GWDHEADER),-1);
		ibeg = 0;
						 
		 hVars = LocalAlloc (LHND,NumIndexFields * sizeof(BTVARDESC));
		 pVars = (BTVARDESC *) LocalLock(hVars);
					     
		 GWFldInfo.Len = 4;
		 GWFldInfo.Beg = ibeg;
		 GWFldInfo.Type = BT_INTEGER;
		 pVars->BT_VARLEN=4;
		 pVars->BT_VARTYP=BT_INTEGER;
		 pVars->BT_VAROFF=0;
		 ifield=0;
		 str = DefStr;
		 if (*str == '(')
		 	str++;
		 
		 NumFields = 4096;		         
		 for (i=0;i<NumFields;i++,ifield++)
		 {  
		 	if ((lpTab = _fstrchr (str,'(')))
		 	{   
		 		*lpTab++=0;
		 		if (!GetFieldTypeAndLenFromChar (lpTab,&GWFldInfo,0))
				{
					HANDLE	hMem = GSSiGlobAlloc (1515,GMEM_MOVEABLE,4096);
					LPSTR	pMess=GlobalLock (hMem);

					sprintf (pMess,"Error in Table Definition String:Field %i,(%s)",i,lpTab); 	
					GSSiMessageBox (0,pMess,Name,MB_ICONEXCLAMATION,0); 
					GSSiGlobUlFree (&hMem);
					goto Exit;
				}
				lpTab++;
				GWFldInfo.Beg = ibeg; 
				if (ifield<NumIndexFields)
				{    
					 if (GWFldInfo.Len > 128)  
					 {
					 	GSSiMessageBox (0,Name,"Key field exceeds max length of 128",MB_ICONEXCLAMATION,0); 
					 	goto Exit;
					 }
					 pVars->BT_VARLEN=GWFldInfo.Len;
					 pVars->BT_VARTYP=GWFldInfo.Type;
					 pVars->BT_VAROFF=ibeg; 
					 pVars++;
					 GWDHead.lKeys[0]=ibeg+GWFldInfo.Len;
				}
								
				ibeg += GWFldInfo.Len;  
				if (*str == '"')   
					str++;
				strncpy0 (GWFldInfo.Name,str,32);
				BigWrite (FidData,(HPSTR)&GWFldInfo,sizeof(GWFldInfo),-1);
				GWDHead.NumFields++;  
				str = _fstrchr (lpTab,',');
				if (!str)
					break;
				str++;
			}
			else
			{
				HANDLE	hMem = GSSiGlobAlloc (1516,GMEM_MOVEABLE,4096);
				LPSTR	pMess=GlobalLock (hMem);

				sprintf (pMess,"Error in Table Definition String:Field %i,(%s)",i,lpTab); 	
				GSSiMessageBox (0,Name,pMess,MB_ICONEXCLAMATION,0); 
				GSSiGlobUlFree (&hMem);
	Exit:
				LocalUnlock(hVars);
				LocalFree(hVars);
				GSSiClose (FidData);
				GSSiRemove (Name);
			
{
#if ENABLETRACE
GSSiExitProg (653);
#endif
			return FALSE;
}
			}
		 }
		 GWDHead.Reclen=ibeg; 
	 }
	 GWDHead.TimeStamp = time(NULL); 
	 GWDHead.Compressed = Compress; 
	 GWDHead.NonUniqueSortedBinary = TRUE;
	 GWDHead.StoredAs32 = 1;
	 GWDHead.FileVersion = 1;
	 GSSillseek (FidData,0,0);
// 	 GWDHead16 = GWDHEADER32toGWDHEADER16 (&GWDHead);
//     BigWrite (FidData,(HPSTR)&GWDHead16 ,sizeof(GWDHEADER16),-1);
     BigWrite (FidData,(HPSTR)&GWDHead ,sizeof(GWDHEADER),-1);
	 GSSillseek (FidData,0,2);
	
	 pDot = _fstrrchr (Name,'.');
	 _fstrcpy (pDot,".in1");
	 LocalUnlock(hVars);
	 pVars =(BTVARDESC *)  LocalLock(hVars);
	 BT_CREATE (Name, 4, FALSE, NumIndexFields, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
	 LocalUnlock(hVars);
	 LocalFree(hVars);
	 GSSiClose (FidData);

	 for (i=2;i<9;i++)
	 {
		 sprintf (pDot,".in%i",i);
		 GSSiRemove (Name);
	 }
	 hDB = OpenGWDatabase (InName,BT_WRITE);
	 CloseGWDatabase (hDB);
{
#if ENABLETRACE
GSSiExitProg (653);
#endif
	return TRUE;			 	 
}
#if ENABLETRACE
}
#endif
}

void GWDClearSetValues (LPGWDHEADER lpGWDHead)
#if ENABLETRACE
{GSSiEnterProg (654);
#endif
{   
	UINT	i;
    LPGWFLDINFO lpGWFldInfo;

    for (i=0,lpGWFldInfo=lpGWDHead->pFldInfo;i<lpGWDHead->NumFields;i++,lpGWFldInfo++)
    	lpGWFldInfo->HasValue = 0;
	
{
#if ENABLETRACE
GSSiExitProg (654);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL GWDGetFieldInfoFromName (HANDLE hDB,LPSTR FieldName,LPGWFLDINFO pFieldInfo,LPSHORT Index)
{   
	UINT	i;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);  
    
    for (i=0,lpGWFldInfo=lpGWDHead->pFldInfo;i<lpGWDHead->NumFields;i++,lpGWFldInfo++)
    	if (!_fstricmp (lpGWFldInfo->Name,FieldName))
    	{
    		*pFieldInfo = *lpGWFldInfo;
    		*Index = i;  
    		GlobalUnlock (hDB);
    		return TRUE;
    	}
	GlobalUnlock (hDB);
	return FALSE;
}


BOOL GWDInitKeyValues (LPGWDHEADER lpGWDHead,int IndexToUse)
#if ENABLETRACE
{GSSiEnterProg (655);
#endif
{
    LPGWFLDINFO pFldInfo;
	BOOL	rtn=FALSE;
	UINT	i;
	
	if (IndexToUse && IndexToUse == lpGWDHead->SpatialIndex)
	{
		memset (lpGWDHead->pKeys[lpGWDHead->SpatialIndex],0,abs(lpGWDHead->lKeys[lpGWDHead->SpatialIndex]));
		goto Exit;
	}
    for (i=0;i<abs(lpGWDHead->NumIndexFields[IndexToUse]);i++)
    { 
    	pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[IndexToUse][i];
		SetFieldToMinVal(lpGWDHead,lpGWDHead->IndexFields[IndexToUse][i]);
    }
Exit:
{
#if ENABLETRACE
GSSiExitProg (655);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL GWDInitUnsetKeyValues (LPGWDHEADER lpGWDHead,int IndexToUse)
#if ENABLETRACE
{GSSiEnterProg (655);
#endif
{
    LPGWFLDINFO pFldInfo;
	BOOL	rtn=FALSE;
	UINT	i;
	
    for (i=0;i<abs(lpGWDHead->NumIndexFields[IndexToUse]);i++)
    { 
    	pFldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[IndexToUse][i];
    	if (!pFldInfo->HasValue)
    	{
    		rtn = TRUE;
			SetFieldToMinVal(lpGWDHead,lpGWDHead->IndexFields[IndexToUse][i]);
    	}
    }
{
#if ENABLETRACE
GSSiExitProg (655);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

int NumBytesDifferent (LPSTR File1,LPSTR File2)
{
	int		nDiff = -1, nTot, nChecked=0;
	int		nBlocks, BlockSize=USHRT_MAX;
	int		iBlock;
	char	mess[128];
	HFILE	Fid1, Fid2;
	UINT	i;
	int		nRead1, nRead2, minRead;
	BOOL	SaveAllowCache = AllowCache;
	HANDLE	hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,BlockSize*2);
	LPBYTE	pBuf1 = GlobalLock (hMem);
	LPBYTE	pBuf2 = pBuf1 + BlockSize;

	AllowCache = FALSE;
	Fid1 = GSSiOpenFile (File1,0,OF_READ);
	if (Fid1 != HFILE_ERROR)
	{
		Fid2 = GSSiOpenFile (File2,0,OF_READ);
		if (Fid2 != HFILE_ERROR)
		{
			nDiff = 0;
			nTot = max (GSSifilelength (Fid1),GSSifilelength (Fid2));
			nBlocks = (nTot - 1)/BlockSize + 1;
			CreateStatusWind (hWndMain,1,"Compare Files");
			{
				for (iBlock=0;iBlock < nBlocks;iBlock++)
				{
					nRead1 = BigRead (Fid1,pBuf1,BlockSize);
					nRead2 = BigRead (Fid2,pBuf2,BlockSize);
					minRead = min (nRead1,nRead2);
					nDiff += nRead1 - minRead;
					nDiff += nRead2 - minRead;
					for (i=0;i<BlockSize;i++)
						if (pBuf1[i] != pBuf2[i])
							nDiff++;
					sprintf (mess,"%i bytes different",nDiff);
					if (!StatusWindowUpdate (NULL,mess, nBlocks, ++nChecked))
						break;
				}
			}
			DestroyStatusWindow(0);
			GSSiClose (Fid2);
		}
		GSSiClose (Fid1);
	}
	GSSiGlobUlFree (&hMem);
	AllowCache = SaveAllowCache;
	return nDiff;
}
BOOL ValidateGMDIndexes(LPSTR FilePath,int wantIndex,BOOL displayAfterEachIndex)
{
	BOOL rtn2 = TRUE;
	HANDLE hDB = OpenGWDatabase(FilePath, BT_READ);
	HANDLE hDB2;
	LPGWDHEADER lpGWDHead;
	LPGWDHEADER lpGWDHead2;
	long offset, offset2;
	int index, startIndex, stopIndex;
	char txt[128];

	if (!hDB)
		return FALSE;
	lpGWDHead = (LPGWDHEADER)GlobalLock(hDB);
	hDB2 = OpenGWDatabase(FilePath, BT_READ);
	lpGWDHead2 = (LPGWDHEADER)GlobalLock(hDB2);
	if (wantIndex)
	{
		startIndex = wantIndex - 1;
		stopIndex = wantIndex;
	}
	else
	{
		startIndex = 0;
		stopIndex = lpGWDHead->NumIndex;
	}
	for (index = startIndex; index < stopIndex; index++)
	{
		int pos = BT_FIRST;
		int keySt;
		int rtn = 1;
		int numErr = 0;
		int nRecs = BT_NUM_IN_INDEX(lpGWDHead2->BTHandle[index]), curRec = 0;

		sprintf(txt, "Check Index %i", index + 1);
		
		CreateStatusWind(hWndMain, 1, txt);

		while (StatusWindowUpdate(NULL, txt, nRecs, curRec) && !BT_FIND(lpGWDHead->BTHandle[index], lpGWDHead->pKeys[index], pos, BT_ANY, (LPSTR)&offset))
		{
			int month = 0;
			pos = BT_NEXT;
			FillGWDData(lpGWDHead, offset);
			if (lpGWDHead->SpatialIndexType == 2)
			{
				int time = GMDGetIntegerFieldVal(lpGWDHead, lpGWDHead->FromDateField);
				if (time)
					month = SysMonthFromSymTime(time);
			}

			keySt = GWDFormKey(lpGWDHead, index, FALSE, 0, month);
			{
				if (!BT_FIND(lpGWDHead2->BTHandle[index], lpGWDHead->pKeys[index], BT_FIRST, BT_EQ, (LPSTR)&offset2))
				{
					if (offset != offset2)
						numErr++;
				}
				else if (keySt)
					rtn = FALSE;
			}
			if (!(curRec++ % 16))
			{
				sprintf(txt, "Index:%i rtn=%i numErr=%i", index+1,rtn, numErr);
				
			}
		}
		if (!rtn)
			rtn2 = FALSE;
		if (displayAfterEachIndex)
		{
			sprintf(txt, "Index:%i rtn=%i numErr=%i", index + 1, rtn, numErr);
			MessageBox(0, txt, "", MB_OK);
		}
		DestroyStatusWindow(0);

	}
	GlobalUnlock(hDB);
	GlobalUnlock(hDB2);
	CloseGWDatabase(hDB);
	CloseGWDatabase(hDB2);
	return rtn2;
}

BOOL FILEFunctions(int nArgs, LPSTR *Arg, LPSTR OutLoc)
{
	HFILE fid;
	BYTE bytes[2];
	BOOL rtn = FALSE;
	*OutLoc = 0;
	if (!stricmp(Arg[1], "UPDATE"))
	{
		if (ExistFile(Arg[2]))
		{
			fid = GSSiOpenFile(Arg[2], 0, OF_READWRITE);
			if (fid != HFILE_ERROR)
			{
				BigRead(fid, bytes, 1);
				GSSillseek(fid, 0, 0);
				BigWrite(fid, bytes, 1, -1);
				GSSiClose(fid);
				CloseAllRequestedFiles(FALSE);
				rtn = TRUE;
			}
		}
	}
	else if (!stricmp(Arg[1], "OFFSET"))
	{
		long off = -1;
		HANDLE	hSQLPtr = GetDBByIDName(Arg[2]);
		if (hSQLPtr)
		{
			LPOPENSQLDATA SQLPtr = (LPOPENSQLDATA)GlobalLock(hSQLPtr);
			off = SQLPtr->Offset;
			GlobalUnlock(hSQLPtr);
		}
		itoa(off, OutLoc, 10);
	}
	return rtn;
}
