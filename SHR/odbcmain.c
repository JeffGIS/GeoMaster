
#include "graphint.h"  
#include "extrndb.h"   
#include <sqlext.h>
 
#define	MAXPARAMLENGTH	128 
#define MAXODBCPARMS	32
#define MAX_OPEN_DATABASES 64
#define STR_LEN 256+1 
#define REM_LEN 512+1

typedef struct {char File[64],User[32],PW[32];} PWINFO;
typedef PWINFO	FAR	*LPPWINFO;

#include "shapefil.h"
#include "gmextern.h"

static	short	nODBCParms=0;
static	HANDLE	hODBCParms[MAXODBCPARMS];
static	HWND	SQLWherehWnd;
static	HANDLE	SQLhDB;  
static	char	SQLString[1024];
static	int		SQLCurFieldType=-1;  
static	char	SQLValOp[16];
static UCHAR  szQual[STR_LEN+1],       szTableName[STR_LEN+1],
       szTableOwner[STR_LEN+1], szTypeName[STR_LEN+1],  
       szRemarks[REM_LEN+1];
static short DBType[MAX_OPEN_DATABASES];
static	short OpenDBType[MAX_OPEN_DATABASES];
static short DBNamePtr[MAX_OPEN_DATABASES];
static HDBC HDBCS[MAX_OPEN_DATABASES];    
static BOOL	StandardSQL[MAX_OPEN_DATABASES]; 
static char	QuoteChar[MAX_OPEN_DATABASES][2];
static HANDLE	hTableNames=0; 
static char	CurTable[128];
static char TableNames[MAX_OPEN_DATABASES][128];
static char	OpenDBNames[MAX_OPEN_DATABASES][256];
static HDBC OpenhDBs[MAX_OPEN_DATABASES];
static short DBOpenCount[MAX_OPEN_DATABASES]; 
static	int	NumOpenDBs=0;   
static HENV	henv=0;
static FIELDINFO	mine;
static LPFIELDINFO	lpmine;
static SDWORD	cbTableQual;
static SDWORD	cbTableOwner;
static SDWORD	cbTableName;
static SDWORD	cbColumnName;
static SDWORD	cbRemarks;
static SDWORD	cbDataType;
static SDWORD	cbPrecision;
static SDWORD	cbLength;
static SDWORD	cbRadix;
static SDWORD	cbNullable;
static SDWORD	cbTypeName;
static SDWORD	cbScale;
          
int FindType(LPSTR szTypeName); 
UDWORD display_size(SWORD coltype, UDWORD collen, UCHAR *colname);

int FindType(char *szTypeName)
{
if(_fstrnicmp (szTypeName,"Char",4) ==0)          return SQL_VARCHAR;
if(_fstrnicmp (szTypeName,"TEXT",4) ==0)          return SQL_VARCHAR;
if(_fstrnicmp (szTypeName,"logical",7) == 0)      return SQL_CHAR;
if(_fstrnicmp (szTypeName,"BIT",3) == 0)          return SQL_CHAR;
if(_fstrnicmp (szTypeName,"numeric",7) == 0)      return SQL_DOUBLE;
if(_fstrnicmp (szTypeName,"DOUBLEFLOAT",11) == 0) return SQL_DOUBLE;
if(_fstrnicmp (szTypeName,"LONG",4) == 0)         return SQL_INTEGER;
if(_fstrnicmp (szTypeName,"date",4) == 0)         return SQL_VARCHAR;
if(_fstrnicmp (szTypeName,"DATETIME",8) == 0)     return SQL_DATE;
if(_fstrnicmp (szTypeName,"float",5) == 0)        return SQL_DOUBLE;
if(_fstrnicmp (szTypeName,"MEMO",4) == 0)         return SQL_VARCHAR;
if(_fstrnicmp (szTypeName,"general",7) == 0)      return SQL_VARCHAR;
if(_fstrnicmp (szTypeName,"byte",4) == 0)         return SQL_CHAR;
if(_fstrnicmp (szTypeName,"CURRENCY",8) == 0)     return SQL_VARCHAR;

return 0;
}

UDWORD display_size(SWORD coltype, UDWORD collen, UCHAR *colname)
{
   switch (coltype)
   {
      case SQL_LONGVARCHAR:
      case SQL_CHAR:
      case SQL_VARCHAR:
	  case SQL_UNKCHAR:
      case SQL_DATE: 
      case SQL_TIMESTAMP:
	  case SQL_BIGINT:
	  case SQL_GUID:
         return(max(collen, _fstrlen(colname)));
	  case SQL_TINYINT:
      case SQL_SMALLINT:
         return(max(6,_fstrlen(colname)));  
      case SQL_BIT:
      	 return 1;
      case SQL_INTEGER:
         return(max(11, _fstrlen(colname)));
      case SQL_DECIMAL:         
      case SQL_NUMERIC:         
      case SQL_REAL:         
      case SQL_FLOAT:         
      case SQL_DOUBLE:
         return(max(15,_fstrlen(colname)));
      default:
         return 0;
   } // end of the switch                  
return 0;
}  

HANDLE CreateUniqueList (int length, LPSTR Name)
{

	BTVARDESC	BTVar[2];
	int	i, ifield;
	OFSTRUCTGM	OFStruct;
	HANDLE	hBT; 
	HANDLE	hMem=0;
	
	if (!Name)
	{
		hMem = GSSiGlobAlloc (0,GHND,256);
		Name = GlobalLock (hMem);
	}					
	GSSiGetTempFileName (0,"gmu",0,(LPSTR)Name);
	BTVar[0].BT_VARTYP=BT_CHAR;
	BTVar[0].BT_VARLEN=min(200,length);
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (Name, 4, FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (Name, 0, BT_WRITE, 0);
    GSSiGlobUlFree (&hMem);
	return hBT;
}  

BOOL GetODBCUniqueFieldValues (int DBhandle, LPCSTR SQL,LPSTR name,int length, 
                               HANDLE hDBList)
{                             
char answer[256], Value[260];
int   i, ClassNo, ClassZero=0; 
RETCODE rc;
HDBC hdbc;
UDWORD collen;
SDWORD  lenanswer;
char sqlstr[256],str[256];
LPSTR	lpsqlstr=sqlstr, lpBrack, lpEndBrack, lpParam;
HSTMT	hstmt; 
BOOL	rtn=FALSE;
HCURSOR hcurSave; 
char	qc=QuoteChar[DBNamePtr[DBhandle]][0];
char	qs[2]="";
		
    hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
    i=DBhandle;
    hdbc = HDBCS[i]; 
    SQLAllocStmt(hdbc, &hstmt);
	if (_fstrchr (TableNames[i],' '))
		_fstrcpy (qs,QuoteChar[DBNamePtr[DBhandle]]);
    if (qc)
	    sprintf (sqlstr," Select DISTINCT %c%s%c From %s%s%s",qc,name,qc,qs,TableNames[i],qs);
	else  
	    sprintf (sqlstr," Select DISTINCT %s From %s",name,TableNames[i]);  
    if (SQL)
    	if (*SQL)
    		sprintf (_fstrchr(sqlstr,0)," Where (%s)",SQL);
    
//    rc = SQLPrepare(hstmt, lpsqlstr, SQL_NTS);
//    if(rc == SQL_SUCCESS)   
		rc = SQLExecDirect(hstmt, lpsqlstr, SQL_NTS);
    if(rc ==  SQL_ERROR)
    {    SDWORD	ierrno;
    	 SWORD lmes;
    	 char	cError[128], Mess[SQL_MAX_MESSAGE_LENGTH], DispStr[SQL_MAX_MESSAGE_LENGTH+300];
	    	 
		 lmes = SQL_MAX_MESSAGE_LENGTH-1; 
	     SQLError(SQL_NULL_HENV,SQL_NULL_HDBC,hstmt,cError,&ierrno,Mess,SQL_MAX_MESSAGE_LENGTH-2,&lmes);
	     sprintf (DispStr,"%s   %s",lpsqlstr,Mess);
	     setDoPaint( FALSE);
	     if (GSSiMsgBox(hWndMain,DispStr,"Error in SQL Query",MB_OKCANCEL|MB_ICONQUESTION,0)
	     	 == IDCANCEL)
	     	 HaltReport=TRUE;  
	     setDoPaint( TRUE);
	     goto Exit;
    }  
	if(rc != SQL_SUCCESS) goto Exit;
	rc = SQLFetch(hstmt);
	while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
	{
	    SQLGetData(hstmt, 1, SQL_C_CHAR, answer, 254, &lenanswer); 
	    if (lenanswer == -1)
	    	answer[0]=0;
	    if (lenanswer >= -1)
	    {  
		    _fstrncpy (Value,answer,length);
		    if (BT_FIND(hDBList,Value,BT_FIRST,BT_EQ,(LPSTR)&ClassNo))
		    	BT_PUT (hDBList,Value,(LPSTR)&ClassZero); 
		}
		rc = SQLFetch(hstmt);
    } 
    rtn = TRUE;
Exit:
	 SQLCloseCursor(hstmt);
	 SQLFreeStmt(hstmt, SQL_DROP);
     GSSiSetCursor (hcurSave); 
     return rtn;
}

RETCODE FetchODBCRecord (LPOPENSQLDATA	SQLPtr)
{	
	RETCODE	rc;
	LPOPENFILEDATA	FilePtr;

	if (!SQLPtr)
		goto Err;

	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	if (!FilePtr)
		goto Err;

	ClearCurVals (FilePtr);
    rc = SQLFetch(SQLPtr->hstmt); 
    GlobalUnlock (SQLPtr->OFHandle);
    return rc;
Err:
	MessageBox (0,"Invalid handle in FetchODBCRecord",0,MB_ICONEXCLAMATION);
	return -1;
}

SWORD NumSQLCols (LPOPENSQLDATA	SQLPtr)
{	
	RETCODE	rc;
	LPOPENFILEDATA	FilePtr;
	SWORD	Ncols;

	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
    rc = SQLNumResultCols(SQLPtr->hstmt,&Ncols); 
    GlobalUnlock (SQLPtr->OFHandle); 
    if (rc != SQL_SUCCESS)
    	Ncols = 0;
    return Ncols;
}
                              
DWORD NumSQLRows (HANDLE hSQL)
{	
char answer[256];
RETCODE rc;
HDBC hdbc;
UDWORD collen;
SDWORD  lenanswer;
HANDLE	hStr=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
LPSTR str=GlobalLock(hStr);
LPSTR sqlstr=str+1024;
LPSTR	lpsqlstr=sqlstr, lpBrack, lpEndBrack, lpParam, lpOrder, Value, lpFrom;
HSTMT	hstmt; 
LPOPENFILEDATA	FilePtr;
DWORD	rtn=0;
//BTHEAD BTHead;
LPOPENSQLDATA	SQLPtr;  
HANDLE	hSTR = 0;

    SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
	FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
    
    switch (FilePtr->Type)
    {
    	case UMIFS_DATAFILE:   
		case ORA_DATAFILE:
    	case GMCENSUS_DATAFILE:
	    {   
	    	LPGWDHEADER	lpGWDHead;
	    	
	        lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);
         	//GetBTHeader (lpGWDHead->BTHandle[0],&BTHead);
         	if (*SQLPtr->SQL)
         	{  
		    	while (FetchDBRec (hSQL))
		    		rtn++;
		    }
		    else
		    	rtn = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
         	GlobalUnlock (FilePtr->FileHandle);
         	GlobalUnlock (SQLPtr->OFHandle);
         	SQLPtr->lastreadtime = 0; 
         	GlobalUnlock (hSQL); 
         	GSSiGlobUlFree (&hStr);
         	return rtn;
		} 
		break;
		
    	case COMBO_DATAFILE:
	    {   
			LPCOMBOHEADER		pComboHeader;
		    LPCOMBOFILE  		pComboFile; 
	    
			pComboHeader = (LPCOMBOHEADER)GlobalLock (FilePtr->FileHandle); 
			pComboFile = (LPCOMBOFILE)GlobalLock (pComboHeader->hComboFile);
			rtn = NumSQLRows (pComboFile->hSQL[0]);
			GlobalUnlock (pComboHeader->hComboFile);
			GlobalUnlock (FilePtr->FileHandle); 
         	GlobalUnlock (SQLPtr->OFHandle); 
         	GlobalUnlock (hSQL);
         	GSSiGlobUlFree (&hStr);
			return rtn;
	    } 
	    break; 
	    
	    case GMTEXT_DATAFILE: 
	    { 
	    	rtn = NumRowsInTxtFile (FilePtr->Fid);
         	GlobalUnlock (SQLPtr->OFHandle); 
        	GlobalUnlock (hSQL);
         	GSSiGlobUlFree (&hStr);
	    	return rtn; 
	    }

		case FGDB_DATAFILE:
		{
			rtn = NumRowsInFGDBTable ((int)FilePtr->FileHandle);
         	GlobalUnlock (SQLPtr->OFHandle); 
        	GlobalUnlock (hSQL);
         	GSSiGlobUlFree (&hStr);
	    	return rtn;
	    }

		case DBF_DATAFILE:
	    case SHAPE_DATAFILE:
        {   
			DBFHandle    pDBF;
			LPDWORD 	 pDBFAddress = (LPDWORD)GlobalLock (FilePtr->FileHandle);
			
			pDBF = (DBFHandle)*pDBFAddress; 
		    if (!pDBF)
		    	break;  
		    rtn = pDBF->nRecords;
			GlobalUnlock (FilePtr->FileHandle);
         	GlobalUnlock (SQLPtr->OFHandle); 
        	GlobalUnlock (hSQL);
         	GSSiGlobUlFree (&hStr);
	    	return rtn;
	    }
            break;
		case SQL_DATAFILE:
		{
			LPSQLDATABASE	pDB = (LPSQLDATABASE)GlobalLock(FilePtr->FileHandle);

			rtn = NumSQLRows(pDB->DBHandle);
			GlobalUnlock(FilePtr->FileHandle);
			GlobalUnlock(SQLPtr->OFHandle);
			GlobalUnlock(hSQL);
			GSSiGlobUlFree(&hStr);
			return rtn;
		}
			break;
		case SLT_DATAFILE:
		{
			LPSQLDATABASE	pDB = (LPSQLDATABASE)GlobalLock(FilePtr->FileHandle);

			rtn = NumSQLRows(pDB->DBHandle);
			GlobalUnlock(FilePtr->FileHandle);
			GlobalUnlock(SQLPtr->OFHandle);
			GlobalUnlock(hSQL);
			GSSiGlobUlFree(&hStr);
			return rtn;
		}
			break;
		case IMAGE_DATAFILE:
			rtn = 1;
			return rtn;
			break;
		case HLTLIST_DATAFILE:
			rtn = BT_NUM_IN_INDEX (FilePtr->FileHandle);
         	GlobalUnlock (SQLPtr->OFHandle); 
        	GlobalUnlock (hSQL);
         	GSSiGlobUlFree (&hStr);
	    	return rtn;
		break;
	    default:
	    break;
	}
    hdbc = HDBCS[(int)FilePtr->FileHandle]; 
    SQLAllocStmt(hdbc, &hstmt);
    hSTR = GSSiGlobAlloc ( 142,GMEM_MOVEABLE,4096);
    Value = GlobalLock (hSTR);   
    _fstrcpy (Value,SQLPtr->SQL);  
    if (*Value == '$')
    	ExpandText (Value);
    lpOrder = _fstrstr (Value," ORDER BY");
    if (lpOrder)
    	*lpOrder = 0;
    lpFrom = _fstrstr (Value," FROM ");
    if (lpFrom)
   		sprintf (sqlstr," Select COUNT(*) %s", lpFrom);
    else
	{
		char	qc=QuoteChar[DBNamePtr[(int)FilePtr->FileHandle]][0];
		char	qs[2]="";
		
		if (_fstrchr(TableNames[(int)FilePtr->FileHandle], ' ') || strchr(TableNames[(int)FilePtr->FileHandle], '$'))
			_fstrcpy (qs,QuoteChar[DBNamePtr[(int)FilePtr->FileHandle]]);
    
	    if (qc)
    		sprintf (sqlstr," Select COUNT(*) From %s%s%s", qs,TableNames[(int)FilePtr->FileHandle],qs);
		else  
	    	sprintf (sqlstr," Select COUNT(*) From %s",TableNames[(int)FilePtr->FileHandle]); 
	    if (*Value)
	    	sprintf (_fstrchr(sqlstr,0)," WHERE %s",Value); 
	}
    ExpandText (sqlstr);
//    rc = SQLPrepare(hstmt, lpsqlstr, SQL_NTS);
//    if(rc == SQL_SUCCESS)  
		rc = SQLExecDirect(hstmt, lpsqlstr, SQL_NTS);
    if(rc ==  SQL_ERROR)
    {    SDWORD	ierrno;
    	 SWORD lmes;
    	 char	cError[128], Mess[SQL_MAX_MESSAGE_LENGTH], DispStr[SQL_MAX_MESSAGE_LENGTH+300];
	    	 
		 lmes = SQL_MAX_MESSAGE_LENGTH-1; 
	     SQLError(SQL_NULL_HENV,SQL_NULL_HDBC,hstmt,cError,&ierrno,Mess,SQL_MAX_MESSAGE_LENGTH-2,&lmes);
	     sprintf (DispStr,"%s   %s",lpsqlstr,Mess);
	     setDoPaint( FALSE);
	     if (GSSiMsgBox(hWndMain,DispStr,"Error in SQL Query",MB_OKCANCEL|MB_ICONQUESTION,0)
	     	 == IDCANCEL)
	     	 HaltReport=TRUE; 
	     setDoPaint( TRUE);
       	 GlobalUnlock (SQLPtr->OFHandle); 
       	 GlobalUnlock (hSQL); 
       	 GSSiGlobUlFree (&hSTR);
       	 GSSiGlobUlFree (&hStr);
	     return FALSE;
    }  
	if(rc != SQL_SUCCESS) goto Exit;
	rc = SQLFetch(hstmt);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
	{
	    SQLGetData(hstmt, 1, SQL_C_CHAR, answer, 254, &lenanswer); 
	    if (lenanswer == -1)
	    	answer[0]=0; 
	    rtn = atol (answer);
    }
    else 
    	rtn = 0;
Exit: 
	 SQLCloseCursor(hstmt);
	 SQLFreeStmt(hstmt, SQL_DROP);
	 GlobalUnlock (SQLPtr->OFHandle); 
	 GlobalUnlock (hSQL);
   	 GSSiGlobUlFree (&hSTR);
   	 GSSiGlobUlFree (&hStr);
     return rtn;
}
                              
void ClearCurVals (LPOPENFILEDATA FilePtr)
{
	int	i;
	LPFIELDINFO	lpField;
	
	for (i=0,lpField=&FilePtr->FldInfo;i<FilePtr->NumFields;i++,lpField++)
	{
		if (lpField->hCurVal)
		{   
			if (lpField->type == SQL_LONGVARBINARY || lpField->type == SQL_MSSHAPE)
			{ 
				LPCURVAL pCurVal = (LPCURVAL)GlobalLock (lpField->hCurVal);   
				HANDLE	hBinVal = (HANDLE)atol (&pCurVal->Value);
				
				GlobalUnlock (lpField->hCurVal);
				GSSiGlobFree (&hBinVal);
			} 
			GSSiGlobFree (&lpField->hCurVal);
		}
	}
	return;
} 
BOOL ExternalSQLDirectBatch (int hDB, LPSTR SQL)
{ 
	static HANDLE	BatchHandle=0;
	int		CurBatchID=0;  
	BOOL	rtn=TRUE;  
	LPSTR	lpSQL;
	
	if (!SQL)
	{
		if (!BatchHandle)
			return TRUE;
		lpSQL = GlobalLock (BatchHandle);
		rtn = ExternalSQLDirect (CurBatchID, lpSQL);
		GlobalUnlock (BatchHandle);
		GSSiGlobFree (&BatchHandle); 
		CurBatchID = 0;
		return rtn;                 
	}
	if (!BatchHandle)
		BatchHandle = GSSiGlobAlloc ( 143,GMEM_MOVEABLE,4096);
	lpSQL = GlobalLock (BatchHandle);
	if (((long)_fstrlen (lpSQL) + (long)_fstrlen (SQL) + 16) > (long)4000 ||
		 ((hDB != CurBatchID) && CurBatchID))
	{
		rtn = ExternalSQLDirect (CurBatchID, lpSQL);
		*lpSQL = 0;
	}
	CurBatchID = hDB;
	_fstrcat (lpSQL,SQL);
	_fstrcat (lpSQL,";");
	GlobalUnlock (BatchHandle);
	return rtn;
}

BOOL ExternalSQLDirect (int hDB, LPSTR SQL)
{
	RETCODE rc;
	HDBC hdbc;
	HSTMT	hstmt;
	BOOL	rtn=TRUE; 
	LPSTR	DBFLoc, pSQL=0;
	HANDLE	handle=0; 
	short	SaveDrive, drivenum;
	char	cwd[128], NewDir[128];


	SaveDrive = _getdrive();
	_getcwd (cwd,128); 
	
    hdbc = HDBCS[hDB]; 
    SQLAllocStmt(hdbc, &hstmt); 
    DBFLoc = _fstrstr (SQL,".DBF");
    if (DBFLoc)
    {   
    	LPSTR	pBegName, pEndName;
    	char	SaveEndName;
    	char	FullName[128], Name[32], Dir[128], Drive[8], Ext[6];
    	
    	pEndName = DBFLoc + 4;
    	SaveEndName = *pEndName;
    	*pEndName = 0;
    	pBegName = _fstrrchr (SQL,' '); 
    	if (!pBegName)
    	{
    		*pEndName = SaveEndName;
    		goto Next; 
    	}
    	*pBegName++=0;
    	if (!_fstrchr (pBegName,'\\'))
    	{
    		*pEndName = SaveEndName; 
    		pBegName--;
    		*pBegName = ' ';
    		goto Next;      
    	} 
    	pEndName++;
    	_fullpath (FullName,pBegName,sizeof(FullName));  
    	_splitpath (FullName,Drive,Dir,Name,Ext);
    	handle = GSSiGlobAlloc ( 144,GMEM_MOVEABLE,USHRT_MAX);
    	pSQL = GlobalLock (handle);  
    	sprintf (pSQL,"%s %s.DBF %s",SQL,Name,pEndName);  
    	SQL = pSQL;
    	sprintf (NewDir,"%s%s",Drive,Dir); 
    	drivenum = GetDriveNum(*Drive);
		_chdrive (drivenum);
	 	_chdir (NewDir); 
    	
    }
Next:
	rc = SQLExecDirect(hstmt, SQL, SQL_NTS);
    if(rc ==  SQL_ERROR)
    {    SDWORD	ierrno;
    	 SWORD lmes; 
    	 LPSTR	cError, Mess, DispStr;
    	 HANDLE	hMem;
	    
	     rtn = FALSE;	 
//    	 hMem = GSSiGlobAlloc ( 145,GMEM_MOVEABLE,SQL_MAX_MESSAGE_LENGTH*2+300+10+128+32);     
    	 hMem = GSSiGlobAlloc ( 146,GMEM_MOVEABLE,USHRT_MAX);     
    	 cError = GlobalLock (hMem);
    	 Mess = cError + 130;
    	 DispStr = Mess + SQL_MAX_MESSAGE_LENGTH+10+4;
	    	 
		 lmes = SQL_MAX_MESSAGE_LENGTH-1; 
	     SQLError(SQL_NULL_HENV,SQL_NULL_HDBC,hstmt,cError,&ierrno,Mess,SQL_MAX_MESSAGE_LENGTH-2,&lmes);
	     sprintf (DispStr,"%s\r\n%s",SQL,Mess); 
	     setDoPaint( FALSE);
	     if (GSSiMsgBox(hWndMain,DispStr,"Error in SQL Execution",MB_OKCANCEL|MB_ICONQUESTION,0)
	     	 == IDCANCEL)
	     {
	     	HaltReport = TRUE;
	     	rtn=FALSE;        
	     }
	     setDoPaint( TRUE);
	     GSSiGlobUlFree (&hMem);
    }  
	SQLCloseCursor(hstmt);
	SQLFreeStmt(hstmt, SQL_DROP);
	if (pSQL)
		GlobalUnlock (handle); 
	_chdrive (SaveDrive);
 	_chdir (cwd); 
	GSSiGlobFree (&handle); 
	return rtn;
} 

BOOL SelectAllStatement (LPOPENFILEDATA FilePtr,LPSTR sqlstr)
{
	
	LPFIELDINFO	lpFieldInfo = &FilePtr->FldInfo;    
	short	i;
	
	_fstrcpy (sqlstr,"Select");
	if (FilePtr->HaveNonStandardFields) 
	{  
		char	qc=QuoteChar[DBNamePtr[(int)FilePtr->FileHandle]][0];
		char	delim=' ';
		
		for (i=0;i<FilePtr->NumFields;i++) 
		{   
			if (qc)
				sprintf (_fstrchr(sqlstr,0),"%c%c%s%c",delim,qc,lpFieldInfo[i].name,qc); 
			else
				sprintf (_fstrchr(sqlstr,0),"%c%s",delim,lpFieldInfo[i].name); 
			delim = ',';
		} 
		return TRUE;
	}
	else if (hSelectFields)
	{
		LPSTR pstr = GlobalLock (hSelectFields);

		strcat (sqlstr," ");
		strcat (sqlstr,pstr);
		GSSiGlobUlFree (&hSelectFields);
		FilePtr->NumFields = 0;
	}
	else
		_fstrcat(sqlstr, " *");
    return FALSE;
} 

/******************************************************************/
LPVOID GetExternalFieldData ( LPOPENFILEDATA FilePtr, LPCSTR indexIN, LPVOID *hstmt,
                              LPFIELDINFO infield, BOOL SingleVal, short FunctionID,LPSHORT irc,
                              int NumFields,LPFIELDINFO FirstField)
                                 
{  // index[] = "pidno", keydata[] = "0102824110013" ;                            
static char answer[4096];
#define MAX_COLS 5
LPVOID lpvoid = answer;
short   j,i, startfield, WantField; 
double danswer;
short shorti, ii;
RETCODE rc;  
HDBC hdbc;
UDWORD collen;
static	SDWORD  lenanswer;
SWORD nresultcols, scale, coltype, colnamelen, nullable;
//UCHAR colname[32];          
long  long_i, irow=-1, nrow, ltemp, l;
//static char szItem[] = "SQL";
HANDLE	hSTR;
LPSTR	sqlstr,index, pBuf, str;  
static	char	FunctionName[5][6]={"COUNT","AVG","SUM","MIN","MAX"};
LPSTR	lpBrack, lpEndBrack, lpParam;
short	nparam;
short	FldNum;
BOOL	ExSQL=TRUE;
int DBhandle; 
BOOL	UseRecNum=FALSE; 
UWORD	RowStatus;   
SQLSMALLINT	NumResultCols;
static	char	CName[32], BlankStr[2]="";
BOOL	SkipFirstField = FALSE;   
LPFIELDINFO field;
LPCURVAL	pCurVal;

    *irc = 0;
	if (infield->hCurVal)
	{ 
		pCurVal = (LPCURVAL)GlobalLock (infield->hCurVal);
		_fstrncpy (answer,&pCurVal->Value,pCurVal->length);
		answer[pCurVal->length]=0;   
		GlobalUnlock (infield->hCurVal);
	    return lpvoid;
	}
	DBhandle = (int)FilePtr->FileHandle;  
	
    hSTR = GSSiGlobAlloc ( 147,GHND,USHRT_MAX); 
    str = GlobalLock (hSTR);
    index = str + 4096;  
    sqlstr = index + 4096;
    _fstrcpy (index,indexIN);
    if (*index == '$')
    	ExpandText (index);

	if (!_fstrnicmp (index,"%RECNUM=",8))
	{  
    	UseRecNum = TRUE; 
    	ExpandText (&index[8]);
    	irow = atol (&index[8]);
    } 
	if (*hstmt && (!SingleVal || UseRecNum))
		ExSQL = FALSE;
   // for(i = 0; i < 128; szValue[i] = ' ',i++);
    i=DBhandle;
    hdbc = HDBCS[i]; 
    if (!hODBCParams)
    	hODBCParams = GSSiGlobAlloc ( 148,GPTR,4096);
    if (!*hstmt)
    {
	    rc = SQLAllocStmt(hdbc, hstmt); 
	    if(rc ==  SQL_ERROR)
	    	goto ErrMes;
	    if (*TableNames[i])
	    {
			char	qc=QuoteChar[DBNamePtr[DBhandle]][0]; 
			
		    if (SingleVal)
		    {   
		    	if (FunctionID)
		    		sprintf (sqlstr,"Select %s(\"%s\")",FunctionName[FunctionID-1],infield->name);
		    	else if (qc)
		    		sprintf (sqlstr,"Select %c%s%c",qc,infield->name,qc);   
//					sprintf (sqlstr,"%s%s%s",QuoteChar[i],field->name,QuoteChar[i]); 
		    	else
		    		sprintf (sqlstr,"Select %s",infield->name);
		    }
		    else
		    	SkipFirstField = SelectAllStatement (FilePtr,sqlstr);
			if (qc && (_fstrchr(TableNames[i], ' ') || strchr(TableNames[i], '$')))
		    	sprintf (_fstrchr(sqlstr,0)," From %c%s%c",qc,TableNames[i],qc);
		    else
		    	sprintf (_fstrchr(sqlstr,0)," From %s",TableNames[i]);
	    	if (!_fstrnicmp (index," ORDER BY",9))
		    	_fstrcat(sqlstr,index); 
	    	else if (UseRecNum)  
	    	{
		    	sprintf (CName,"CURSOR%8.8lx",(long)*hstmt); 
				SQLSetCursorName(*hstmt, (UCHAR FAR *)CName, SQL_NTS);  
				SQLSetStmtOption(*hstmt,SQL_ROWSET_SIZE,1);
				SQLSetStmtOption(*hstmt,SQL_CURSOR_TYPE,SQL_CURSOR_DYNAMIC);
				SQLSetStmtOption(*hstmt,SQL_CONCURRENCY,SQL_CONCUR_READ_ONLY);

		    }
		    else if (index[0])
		    {
		    	_fstrcat(sqlstr, " Where ");
		    	_fstrcpy(str, index);
			    AllVarEqQuestionMark=1;
			    ExpandText (str);
			    AllVarEqQuestionMark=0;   
			    REPLAC (str,"'?'","?",sizeof(str));
			    _fstrcat(sqlstr,str);
		    } 
	    }
	    else 
	    {
	    	 _fstrcpy(sqlstr,index);
	    	 ExpandText (sqlstr);
	    }
	   // outlen = sizeof(sqlstr);
		if (UseRecNum || SingleVal) 
		{   
			if (!FilePtr->BufferHandle)
				FilePtr->BufferHandle = GSSiGlobAlloc ( 149,GPTR,4096);
			pBuf = GlobalLock (FilePtr->BufferHandle);
		    if ((rc = SQLBindCol(*hstmt,  1, SQL_C_CHAR,   pBuf, 4095, &lenanswer)) == SQL_ERROR)
		    	goto ErrMes;  
		    GlobalUnlock (FilePtr->BufferHandle);
		}
	    rc = SQLPrepare(*hstmt, sqlstr, SQL_NTS);
	    if(rc ==  SQL_ERROR)
	    	goto ErrMes;
	    if(rc != SQL_SUCCESS) goto s44;  
	    nparam = 0;   
	    lpParam = GlobalLock (hODBCParams);
		lpBrack = str; 
		if (UseRecNum)
			lpBrack = BlankStr;
		while ((lpBrack = _fstrchr (lpBrack,'?')))
		{
			SWORD	scale;
			UDWORD	precision;
			short	type = GetSQLType(str,lpBrack,FilePtr->NumFields,&FilePtr->FldInfo,&precision,&scale);
			
			++nparam; 
				
			rc = SQLSetParam(*hstmt, nparam, SQL_C_CHAR,
							 type,
							 precision,scale,lpParam, 0);
			lpParam += MAXPARAMLENGTH;   
			lpBrack++;
		} 
		GlobalUnlock (hODBCParams);
	  /*  SQLBindCol(*hstmt, 1, SQL_C_CHAR, answer, 254, &lenanswer);*/
	}
	if (ExSQL)
	{   
		HANDLE	hTemp=GSSiGlobAlloc ( 150,GMEM_MOVEABLE,4096*2); 
		LPSTR	pTemp, pParm, tmp;
		        
		AddToODBCParms (0,0); 
		pTemp = GlobalLock (hTemp); 
		tmp = pTemp + 4096;
		_fstrcpy (pTemp,index);
	    AllVarEqQuestionMark=2;
		ExpandText (pTemp);
		AllVarEqQuestionMark=0;   
		lpParam = pTemp;
		*pTemp = 0;
		for (i=0;i<nODBCParms;i++)
		{
			LPSTR	pODBCParm = GlobalLock (hODBCParms[i]);
			
			_fstrcpy (tmp,pODBCParm);
			ExpandText (tmp);
			_fstrcpy (lpParam,tmp);
			lpParam += MAXPARAMLENGTH; 
			GlobalUnlock (hODBCParms[i]);
		}  
		pParm = GlobalLock (hODBCParams);
		_fmemmove (pParm,pTemp,4096);
		GlobalUnlock (hODBCParams);
		GSSiGlobUlFree (&hTemp); 
	    /*rc = SQLExecDirect(hstmt, lpsqlstr, SQL_NTS);*/
	    rc = SQLExecute(*hstmt);
ErrMes: 
	    if(rc ==  SQL_ERROR)
	    {    SDWORD	ierrno;
	    	 SWORD lmes; 
	    	 LPSTR	cError, Mess, DispStr;
	    	 HANDLE	hMem; 
	    	 HANDLE	hParms = GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
	    	 LPSTR	pParms = GlobalLock (hParms);
	    	 RETCODE	rcer;
	    	 BOOL	FirstErr=TRUE;
	    	 
			 //MessageBox (0,"Have ODBC Error",0,MB_ICONEXCLAMATION|MB_TASKMODAL);
	    	 if (ShowSQLErrors)
	    	 {
				 numSQLErrors++;
		    	 *pParms = 0;
				 for (i=0;i<nODBCParms;i++)
				 {
					LPSTR	pODBCParm = GlobalLock (hODBCParms[i]);
					
					sprintf (_fstrchr (pParms,0),"\"%s\" ",pODBCParm);
					GlobalUnlock (hODBCParms[i]);
				 }  
				 AddToODBCParms (0,0); 
		    	 ExpandText (pParms);
		    	 hMem = GSSiGlobAlloc ( 151,GMEM_MOVEABLE,USHRT_MAX);     
		    	 cError = GlobalLock (hMem);
		    	 Mess = cError + 130;
		    	 DispStr = Mess + SQL_MAX_MESSAGE_LENGTH+10+4;
		    	 
		NextErr:
				 lmes = SQL_MAX_MESSAGE_LENGTH-1; 
			     rcer = SQLError(SQL_NULL_HENV,SQL_NULL_HDBC,*hstmt,cError,&ierrno,Mess,SQL_MAX_MESSAGE_LENGTH-1,&lmes);
			     if (!FirstErr && rcer == SQL_NO_DATA_FOUND)
			     	goto EndErr;    
			     FirstErr = FALSE;
			     sprintf (DispStr,"Query:%s\r\nParms:%s\r\nError Num: %ld(%i)\r\nError Message:%s",sqlstr,pParms,ierrno,rcer,Mess); 
			     setDoPaint( FALSE);
				 if (ShowSQLErrors == 1)
				 {
					 if (GSSiMsgBox(hWndMain, DispStr, "Error in SQL Query", MB_OKCANCEL | MB_ICONQUESTION, 0)
						 == IDCANCEL)
					 {
						 HaltReport = TRUE;
						 ContinueProcessing = FALSE;
					 }
				 }
				 else if (*SQLErrorLog)
				 {
					 AppendFile(SQLErrorLog, DispStr);
				 }
			     goto NextErr;  
		EndErr:
			     setDoPaint( TRUE);
			     GSSiGlobUlFree (&hMem);
			 }  
		     GSSiGlobUlFree (&hParms);
		     goto ErrExit;
	    }
  		AddToODBCParms (0,0); 
		if(rc != SQL_SUCCESS) goto s44;
		if (!UseRecNum)    
    		rc = SQLFetch(*hstmt);  
    		
	    if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
			goto s44;
		if (rc == SQL_SUCCESS_WITH_INFO)
			ii=1;
	} 
    ClearCurVals (FilePtr);
	if (UseRecNum)
	{
		rc = SQLExtendedFetch(*hstmt, SQL_FETCH_ABSOLUTE, irow, &nrow, &RowStatus);  
	    if(rc ==  SQL_ERROR)
	    	goto ErrMes;
	    if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)goto s44;
    }
/*    SQLNumResultCols(*hstmt, &nresultcols);
    SQLDescribeCol(*hstmt,1,colname, (SWORD) sizeof(colname),
                     &colnamelen, &coltype, &collen, &scale,
                     &nullable); */
   // outlen = display_size(coltype, collen, colname);
    //SQLGetData(hstmt,1, SQL_C_CHAR, answer, outlen, &lenanswer);
/*	hstmt = NULL;*/
    SQLNumResultCols (*hstmt,&NumResultCols); 
    if (!FilePtr->NumFields)
    {
    	short	j;    
    	SWORD	nlen,type,scale,nullable;
    	SDWORD	precis; 
    	LPFIELDINFO	pField=&FilePtr->FldInfo; //pField[1]
    	
    	for (j=0;j<NumResultCols;j++)
    	{ 
    		SQLDescribeCol (*hstmt,(short)(j+1),pField[j].name,62,&nlen,&type,&precis,&scale,&nullable);  
    		pField[j].type = type;
			pField[j].index = j;
			pField[j].scale = scale;
			pField[j].precision = precis;
	    	switch (type)
	    	{
	       		case SQL_DATE:
				case SQL_TIME:
				case SQL_TIMESTAMP:
				case SQL_GUID:
	//				lpmine->type = SQL_VARCHAR;
					pField[j].length = precis;
				break;
	            case SQL_LONGVARBINARY:
					pField[j].length = 8;
					break;
				case SQL_CHAR: 
				case SQL_VARCHAR:
				case SQL_UNKCHAR:
				case BT_RIGHT_CHAR:
				case BT_CHAR: 
					pField[j].length = precis;
					break;
				case SQL_NUMERIC:
				case SQL_FLOAT:
				case SQL_REAL:
				case SQL_DOUBLE:
		    		pField[j].type = BT_REAL;
					if (precis > 10)
						pField[j].length = 8;
					else
						pField[j].length = 4;
					break;
				case SQL_INTEGER:
				case SQL_TINYINT:
				case SQL_SMALLINT:
    			case BT_INTEGER:
    			case BT_INT2:
    			case BT_INT4: 
    			case SQL_BIT:
		    		pField[j].type = BT_INTEGER;
					if (precis > 8)
						pField[j].length = 4;
					else
						pField[j].length = 2;
					break;
	       		default:
	       		break;
	       	} 
    	} 
    	FilePtr->NumFields = NumResultCols;
    }
    if (SingleVal) 
    {
    	startfield = 1;
    	NumFields = 1;
    	WantField = 1;
		field = infield;
    }
    else  
    {
	    WantField = infield->index+1;  
	    field = FirstField;
    	startfield = 1; 
/*    	if (!_fstricmp (field->name,"GEOM"))
    	{
			field->hCurVal = GSSiGlobAlloc ( 152,GHND,2);
			pCurVal = (LPCURVAL)GlobalLock (field->hCurVal); 
			pCurVal->length=0;   
			GlobalUnlock (field->hCurVal);  
    		field++; 
    		WantField--;
    	}*/
    } 
    for (j=0,FldNum = startfield;j<NumResultCols;j++,FldNum++,field++)
    {   
    	HANDLE	hBinVal=0;
    	HPSTR	binval;
    	
    	if (!SingleVal || FldNum > 1)
    	{   
    		if (field->type == SQL_LONGVARBINARY) 
    		{   
    			LPSHPPOLYHEADER	pShp; 
    			BYTE	Dummy[2];
    			long	size=32000;
	    		char	mess[64];
				HPSTR	pTemp;
    			
    			//hBinVal=GSSiGlobAlloc (0,GMEM_MOVEABLE,size);
    			//binval=GlobalLock (hBinVal);
				pTemp = malloc (lCommonMem);
				pTemp += 1024;
		    	//rc = SQLGetData(*hstmt, FldNum, SQL_C_BINARY, pCommonMem, lCommonMem, &lenanswer);   
		    	rc = SQLGetData(*hstmt, FldNum, SQL_C_BINARY, pTemp, lCommonMem-1024, &lenanswer);   
				pShp = (LPSHPPOLYHEADER)pTemp;
		    	//rc = SQLGetData(*hstmt, FldNum, SQL_C_BINARY, binval, size, &lenanswer);   
		    	if (rc == SQL_ERROR)// || rc == SQL_SUCCESS_WITH_INFO)
		    	{   
		    		sprintf (mess,"rc = %i",rc);
		    		MessageBox (0,mess,0,MB_OK|MB_SYSTEMMODAL);
		    	} 
				else if (rc == SQL_SUCCESS_WITH_INFO)
		    	{   
		    		sprintf (mess,"rc = %i",rc);
		    		MessageBox (0,mess,0,MB_OK|MB_SYSTEMMODAL);
		    	} 
		    	else if (lenanswer ==  SQL_NULL_DATA)
				{
					lenanswer = 0;
					*pTemp = 0;
				}
		    	else if (lenanswer == SQL_NO_TOTAL)
		    	{   
		    		sprintf (mess,"len = %ld",lenanswer);
		    		MessageBox (0,mess,"SQL error",MB_OK|MB_SYSTEMMODAL);
		    	}  
		    	//else if (lenanswer > 64000)
		    	//{   
		    	//	sprintf (mess,"len = %ld",lenanswer);
		    	//	MessageBox (0,mess,0,MB_OK|MB_SYSTEMMODAL);
		    	//}
				else if (lenanswer <= 0)
		    	{   
		    		sprintf (mess,"len = %ld",lenanswer);
		    		MessageBox (0,mess,0,MB_OK|MB_SYSTEMMODAL);
		    	}
		    	
		    	BinSize = lenanswer+32; 
    			hBinVal=GSSiGlobAlloc (0,GMEM_MOVEABLE,BinSize);
    			binval=GlobalLock (hBinVal);
    			hmemmove (binval,pTemp,BinSize-1);
		    	BinSize = lenanswer; 
				pTemp -= 1024;
				free (pTemp);
		    /*	if (BinSize > size)
		    	{   
		    		GlobalUnlock (hBinVal);
				   	hBinVal = GlobalReAlloc (hBinVal,BinSize,GMEM_MOVEABLE);
	    			binval=GlobalLock (hBinVal);
			    	rc = SQLGetData(*hstmt, FldNum, SQL_C_BINARY, &binval[size], BinSize, &lenanswer);  
		    	} */
    			pShp = (LPSHPPOLYHEADER)binval;   //sizeof(SHPPOLYHEADER)
		    	ltoa ((long)hBinVal,str,10); 
		    	lenanswer = _fstrlen (str)+1;  
				str[lenanswer] = 0;
		    	GlobalUnlock (hBinVal);
		    }
    		else if (field->type ==  SQL_MSSHAPE) 
    		{   
    			LPMSGEOGRAPHY	pShp; 
    			BYTE	Dummy[2];
    			long	size=32000;
	    		char	mess[64];
				HPSTR	pTemp;
    			
    			//hBinVal=GSSiGlobAlloc (0,GMEM_MOVEABLE,size);
    			//binval=GlobalLock (hBinVal);
				pTemp = malloc (lCommonMem);
		    	//rc = SQLGetData(*hstmt, FldNum, SQL_C_BINARY, pCommonMem, lCommonMem, &lenanswer);   
		    	rc = SQLGetData(*hstmt, FldNum, SQL_C_BINARY, pTemp, lCommonMem, &lenanswer);   
				pShp = (LPMSGEOGRAPHY)pTemp;
		    	//rc = SQLGetData(*hstmt, FldNum, SQL_C_BINARY, binval, size, &lenanswer);   
		    	if (rc == SQL_ERROR)// || rc == SQL_SUCCESS_WITH_INFO)
		    	{   
		    		sprintf (mess,"rc = %i",rc);
		    		MessageBox (0,mess,0,MB_OK|MB_SYSTEMMODAL);
		    	} 
				else if (rc == SQL_SUCCESS_WITH_INFO)
		    	{   
		    		sprintf (mess,"rc = %i",rc);
		    		MessageBox (0,mess,0,MB_OK|MB_SYSTEMMODAL);
		    	} 
		    	else if (lenanswer ==  SQL_NULL_DATA)
				{
					lenanswer = 0;
					*pTemp = 0;
				}
		    	else if (lenanswer == SQL_NO_TOTAL)
		    	{   
		    		sprintf (mess,"len = %ld",lenanswer);
		    		MessageBox (0,mess,"SQL error",MB_OK|MB_SYSTEMMODAL);
		    	}  
		    	//else if (lenanswer > 64000)
		    	//{   
		    	//	sprintf (mess,"len = %ld",lenanswer);
		    	//	MessageBox (0,mess,0,MB_OK|MB_SYSTEMMODAL);
		    	//}
				else if (lenanswer <= 0)
		    	{   
		    		sprintf (mess,"len = %ld",lenanswer);
		    		MessageBox (0,mess,0,MB_OK|MB_SYSTEMMODAL);
		    	}
		    	
		    	BinSize = lenanswer+1; 
    			hBinVal=GSSiGlobAlloc (0,GMEM_MOVEABLE,BinSize);
    			binval=GlobalLock (hBinVal);
    			hmemmove (binval,pTemp,BinSize-1);
		    	BinSize = lenanswer; 
				free (pTemp);
		    /*	if (BinSize > size)
		    	{   
		    		GlobalUnlock (hBinVal);
				   	hBinVal = GlobalReAlloc (hBinVal,BinSize,GMEM_MOVEABLE);
	    			binval=GlobalLock (hBinVal);
			    	rc = SQLGetData(*hstmt, FldNum, SQL_C_BINARY, &binval[size], BinSize, &lenanswer);  
		    	} */
    			pShp = (LPMSGEOGRAPHY)binval;   //sizeof(SHPPOLYHEADER)
				{
					BOOL hasZ = pShp->SerializationProperties & 0x01;
					BOOL hasM = pShp->SerializationProperties & 0x02;
					BOOL isValid = pShp->SerializationProperties & 0x04;
					BOOL isPoint = pShp->SerializationProperties & 0x08;
					BOOL isLine = pShp->SerializationProperties & 0x10;
					LPDWORD	pNumFigures, pNumShapes, pNumPoints;
					LPDPOINT pPoint = (LPDPOINT)&pShp->numPoints;
					LPDOUBLE pZ;

		    		ltoa ((long)hBinVal,str,10); 
					lenanswer = _fstrlen (str); 
					if (isPoint)
					{
						pNumFigures = (LPDWORD)((LPBYTE)&pShp->numPoints + sizeof (DPOINT));
						pZ = (LPDOUBLE)(pPoint+1);
					}
					else if (isLine)
					{
						pNumFigures = (LPDWORD)((LPBYTE)&pShp->numPoints + sizeof (DPOINT)*2);
						pPoint++;
						pZ = (LPDOUBLE)(pPoint+1);
						pZ++;
					}
					else
					{
						pNumPoints = &pShp->numPoints;
						pNumFigures = (LPDWORD)(pNumPoints + 1);
						pPoint = (LPDPOINT)(pNumPoints + 1);
						pNumFigures = (LPDWORD)((LPBYTE)(pNumPoints+1) + pShp->numPoints * 16);
						if (hasZ)
							pNumFigures = (LPDWORD)((LPBYTE)pNumFigures + pShp->numPoints * 8);
						if (hasM)
							pNumFigures = (LPDWORD)((LPBYTE)pNumFigures + pShp->numPoints * 8);
					}
					pNumShapes = (LPDWORD)((LPBYTE)pNumFigures + *pNumFigures * 5);
					ii=1;
				}
		    	GlobalUnlock (hBinVal);
		    }
			else if (field->type == SQL_REAL || field->type == SQL_FLOAT)
			{
				float rval;
				rc = SQLGetData(*hstmt, FldNum, SQL_C_FLOAT, &rval, 4, &lenanswer);
				ftoa(str, rval);
				lenanswer = strlen(str);
			}
			else if (field->type == SQL_DOUBLE)
			{
				double rval;
				rc = SQLGetData(*hstmt, FldNum, SQL_C_DOUBLE, &rval, 8, &lenanswer);
				ftoa(str, rval);
				lenanswer = strlen(str);
			}
			else
		    	rc = SQLGetData(*hstmt, FldNum, SQL_C_CHAR, str, 4095, &lenanswer);
		    if(rc ==  SQL_ERROR) 
		    	goto ErrMes;
		    if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
				goto s44;   
			if (rc == SQL_SUCCESS_WITH_INFO)
				ii=1;
		}
		else
		{
			pBuf = GlobalLock (FilePtr->BufferHandle);
			_fstrcpy (str,pBuf);
			GlobalUnlock (FilePtr->BufferHandle);
		}   
		l=min(4095,lenanswer);
		if (l<0)
		{
			l=0;	 
			str[0]=0;
		}
		if (SingleVal)
			field->hCurVal = 0;
		else
		{   short	ii;
			if (l>255)
				ii=1;
			field->hCurVal = GSSiGlobAlloc ( 153,GMEM_MOVEABLE,sizeof(int)+l+4);
			pCurVal = (LPCURVAL)GlobalLock (field->hCurVal); 
			pCurVal->length=l;   
			if (pCurVal->length)
				_fstrncpy (&pCurVal->Value,str,pCurVal->length);
			GlobalUnlock (field->hCurVal);  
		} 
		if (FldNum == WantField)
		{
			_fstrncpy (answer,str,(size_t)l);
			answer[l]=0;
		} 
	}
	if (SingleVal && !UseRecNum) 
	{
		SQLCloseCursor(*hstmt);
//		SQLFreeStmt(*hstmt, SQL_CLOSE);
//		hstmt = 0;
	}
	GSSiGlobUlFree (&hSTR);
    return lpvoid;
    
s44: if (*hstmt && SingleVal)
	 {
		SQLCloseCursor(*hstmt);
	 	SQLFreeStmt(*hstmt, SQL_CLOSE);
		hstmt = 0;
		/*SQLFreeStmt(*hstmt,SQL_UNBIND);*/
	 } 
ErrExit:	
	if (ShowSQLErrors)
     *irc = 1;
	GSSiGlobUlFree (&hSTR); 
	*answer = 0;
    return lpvoid;

}   
BOOL AddToODBCParms (LPSTR Begin,LPSTR End)
{    
	short	l; 
	LPSTR	pODBCParm;
	
	if (!Begin)
	{
		while (nODBCParms--)
			GSSiGlobFree (&hODBCParms[nODBCParms]);   
		nODBCParms = 0;
	}
	else if (!End)
	{   
		if (nODBCParms == MAXODBCPARMS)
		{
ErrExit:
			while (nODBCParms--)
				GSSiGlobFree (&hODBCParms[nODBCParms]);   
			BlowOut ("Reached limit of ODBC Parms","Fatal Error");
		}
		l = _fstrlen (Begin)+2;
		hODBCParms[nODBCParms]=GSSiGlobAlloc ( 154,GMEM_MOVEABLE,l+1);
		pODBCParm = GlobalLock (hODBCParms[nODBCParms]);
		sprintf (pODBCParm,"[%s]",Begin);
		GlobalUnlock (hODBCParms[nODBCParms++]);
	}
	else 
	{
		char	save=*++End;
		
		if (nODBCParms == MAXODBCPARMS)
			goto ErrExit;
		*End = 0; 
		l = _fstrlen (Begin);
		hODBCParms[nODBCParms]=GSSiGlobAlloc ( 155,GMEM_MOVEABLE,l+1);
		pODBCParm = GlobalLock (hODBCParms[nODBCParms]);
		_fstrcpy (pODBCParm,Begin); 
		*End = save;
		GlobalUnlock (hODBCParms[nODBCParms++]);
	}
	return TRUE;
}

int UpdateExternalFieldData ( LPOPENFILEDATA FilePtr, LPSTR index,LPSTR UpdateString)
{                                 
HSTMT	hstmt=0;
int   i,l; 
short shorti;
RETCODE rc;
HDBC hdbc;
UDWORD collen;
static	SDWORD  lenanswer;
SWORD nresultcols, scale, coltype, colnamelen, nullable;
//UCHAR colname[32];          
long  long_i, nrow;
//static char szItem[] = "SQL";
char str[512];
LPSTR	lpsqlstr, lpBrack, lpEndBrack, lpParam;
short	nparam, FldNum;
BOOL	ExSQL=TRUE;
HANDLE DBhandle, hstr; 
LPCURVAL	pCurVal;  
long	irow;
UWORD	RowStatus; 
BOOL	UseRecNum = FALSE;  
static	char	CName[32];  
LPSTR	pBuf; 
short	lCName;

    hstr = GSSiGlobAlloc ( 156,GMEM_MOVEABLE,4096);
    lpsqlstr = GlobalLock (hstr);
    *lpsqlstr = 0;                                       
	DBhandle = FilePtr->FileHandle;    
    
    i=(int)DBhandle;
    hdbc = HDBCS[i]; 
    if (!hODBCParams)
    	hODBCParams = GSSiGlobAlloc ( 157,GHND,4096);  
    	
	if (!_fstrnicmp (index,"%RECNUM=",8))
	{  
    	GSSiMessageBox ("Cannot update using %RECNUM",0,MB_ICONEXCLAMATION,0);
    	goto s44;
    } 
    SQLAllocStmt(hdbc, &hstmt); 
    sprintf (lpsqlstr,"UPDATE %s SET %s WHERE ",TableNames[i],UpdateString);  
/*    if (UseRecNum)  
    {
    	_fstrcat (lpsqlstr,"CURRENT OF "); 
    	sprintf (CName,"CURSOR%8.8lx",(long)hstmt); 
    	_fstrcat (lpsqlstr,CName); 
    	SQLGetCursorName(hstmt,CName,32,&lCName);
		SQLSetCursorName(hstmt, (UCHAR FAR *)CName, SQL_NTS);  
		SQLSetStmtOption(hstmt,SQL_ROWSET_SIZE,1);
		SQLSetStmtOption(hstmt,SQL_CURSOR_TYPE,SQL_CURSOR_KEYSET_DRIVEN);
		if (!FilePtr->BufferHandle)
			FilePtr->BufferHandle = GSSiGlobAlloc ( 158,GPTR,4096);
		pBuf = GlobalLock (FilePtr->BufferHandle);
	    if ((rc = SQLBindCol(hstmt,  1, SQL_C_CHAR,   pBuf, 4095, &lenanswer)) == SQL_ERROR)
	    	goto ErrMes; 
	    GlobalUnlock (FilePtr->BufferHandle);
		sprintf (str,"SELECT * FROM %s",TableNames[i]);
	    if (SQLPrepare(hstmt, str, SQL_NTS) == SQL_ERROR)
	    	goto ErrMes;
	    if (SQLExecute(hstmt) == SQL_ERROR)
	    	goto ErrMes;
		if (SQLExtendedFetch(hstmt, SQL_FETCH_ABSOLUTE, irow, &nrow, &RowStatus) == SQL_ERROR)
	    	goto ErrMes; 
	    if (SQLExecDirect(hstmt, lpsqlstr, SQL_NTS) == SQL_ERROR)
	    	goto ErrMes;
	    if (SQLPrepare(hstmt, lpsqlstr, SQL_NTS) == SQL_ERROR)
	    	goto ErrMes;
    }
    else */
    {
    	_fstrcpy(str, index);
	    AllVarEqQuestionMark=1;
	    ExpandText (str);
	    AllVarEqQuestionMark=0;
	    _fstrcat(lpsqlstr,str);
	    rc = SQLPrepare(hstmt, lpsqlstr, SQL_NTS);
	    if(rc != SQL_SUCCESS) goto s44;  
	    nparam = 0;   
	    lpParam = GlobalLock (hODBCParams);
		lpBrack = index;
		while ((lpBrack = _fstrchr (lpBrack,'[')))
		{
			if (lpEndBrack = _fstrchr (lpBrack,']'))
			{
				SWORD	scale;
				UDWORD	precision;
				short	type = GetSQLType(str,lpBrack,FilePtr->NumFields,&FilePtr->FldInfo,&precision,&scale);
				
				++nparam; 
					
				rc = SQLSetParam(hstmt, nparam, SQL_C_CHAR,
								 type,
								 precision,scale,lpParam, 0);
				lpParam += MAXPARAMLENGTH;   
				lpBrack++;
			} 
			else
				lpBrack++;
				
		} 
		GlobalUnlock (hODBCParams);
	
		{   
			HANDLE	hTemp=GSSiGlobAlloc ( 159,GMEM_MOVEABLE,4096); 
			LPSTR	pTemp, pParm;
			        
			lpBrack = index; 
			lpParam = pTemp = GlobalLock (hTemp);
			while ((lpBrack = _fstrchr (lpBrack,'[')))
			{
				if (lpEndBrack = _fstrchr (lpBrack,']'))
				{   char	SaveChar, tmp[260];
				
					SaveChar = *++lpEndBrack;
					*lpEndBrack = '\0';
					_fstrcpy (tmp,lpBrack);
					ExpandText (tmp);
					_fstrcpy (lpParam,tmp);
					*lpEndBrack = SaveChar;
					lpBrack = lpEndBrack;
					lpParam += MAXPARAMLENGTH; 
				}
				else
					lpBrack++;
				
			}  
			pParm = GlobalLock (hODBCParams);
			_fmemmove (pParm,pTemp,4096);
			GlobalUnlock (hODBCParams);
			GSSiGlobUlFree (&hTemp);
	    }
	}
	
    /*rc = SQLExecDirect(hstmt, lpsqlstr, SQL_NTS);*/
    rc = SQLExecute(hstmt);
    if(rc ==  SQL_ERROR)
    {    SDWORD	ierrno;
    	 SWORD lmes; 
    	 LPSTR	cError, Mess, DispStr;
    	 HANDLE	hMem;
	    	 
ErrMes: 
    	 hMem = GSSiGlobAlloc ( 160,GMEM_MOVEABLE,SQL_MAX_MESSAGE_LENGTH*2+300+10+128+32);     
    	 cError = GlobalLock (hMem);
    	 Mess = cError + 130;
    	 DispStr = Mess + SQL_MAX_MESSAGE_LENGTH+10+4;
	    	 
		 lmes = SQL_MAX_MESSAGE_LENGTH-1; 
	     SQLError(SQL_NULL_HENV,SQL_NULL_HDBC,hstmt,cError,&ierrno,Mess,SQL_MAX_MESSAGE_LENGTH-2,&lmes);
	     sprintf (DispStr,"%s   %s",lpsqlstr,Mess); 
	     setDoPaint( FALSE);
	     if (GSSiMsgBox(hWndMain,DispStr,"Error in SQL Query",MB_OKCANCEL|MB_ICONQUESTION,0)
	     	 == IDCANCEL)
	     	 HaltReport=TRUE;  
	     setDoPaint( TRUE);
	     GlobalUnlock (hMem);
	     GSSiGlobUlFree (&hMem);
	     goto s44;
    }  
    ClearCurVals (FilePtr);
	if(rc != SQL_SUCCESS) goto s44;
	SQLCloseCursor(hstmt);
	SQLFreeStmt(hstmt, SQL_CLOSE); 
	GSSiGlobUlFree (&hstr);
    return TRUE;
    
    
s44: if (hstmt)
	 {
		SQLCloseCursor(hstmt);
	 	SQLFreeStmt(hstmt, SQL_CLOSE);
	 }
	if (HaltReport)
		ContinueProcessing = FALSE;
	GSSiGlobUlFree (&hstr);
    return FALSE;
}   

BOOL InsertExternalFieldData (LPOPENFILEDATA FilePtr, LPSTR InsertString)
{                                 
int   i,l; 
RETCODE rc;
HDBC hdbc;
LPSTR	lpsqlstr;
HANDLE hstr; 
BOOL	rtn;

    hstr = GSSiGlobAlloc ( 161,GMEM_MOVEABLE,USHRT_MAX);
    lpsqlstr = GlobalLock (hstr);
    *lpsqlstr = 0;                                       
	i = (int)FilePtr->FileHandle;    
    
    sprintf (lpsqlstr,"INSERT INTO %s %s",TableNames[i],InsertString);
    ExpandText (lpsqlstr);
    ClearCurVals (FilePtr);
	rtn = ExternalSQLDirect (i,lpsqlstr);
	GSSiGlobUlFree (&hstr);
	
    return rtn;
}   

USHORT GetSQLType(LPSTR pstr,LPSTR lpBrack,int NumFields,LPFIELDINFO lpFldInfo,LPDWORD pPrecision,LPUSHORT pScale)
{   
	char	Name[64];  
	LPSTR	lpEq;
	
	*pPrecision = MAXPARAMLENGTH; 
	*pScale = 0;
	if (lpBrack <= pstr)
		return SQL_VARCHAR;
	lpBrack--; 
	while (*lpBrack == ' ' && lpBrack > pstr)
		lpBrack--;
	if (*lpBrack != '=')
		return SQL_VARCHAR;   
	lpEq = lpBrack;
	*(lpBrack--)=0;	
	while (*lpBrack == ' ' && lpBrack > pstr)
		lpBrack--;
	while (lpBrack > pstr && (*lpBrack != ' ' && *lpBrack != '('))
		lpBrack--;
	if (lpBrack > pstr) lpBrack++;
	_fstrcpy (Name,lpBrack); 
	Truncate (Name);
	*lpEq = '=';
	while (NumFields--)
	{
		if (!_fstrcmp (lpFldInfo->name,Name))
		{   
			if (lpFldInfo->type == SQL_DECIMAL || lpFldInfo->type == SQL_NUMERIC)
			{
				*pPrecision = lpFldInfo->precision;
				*pScale = lpFldInfo->scale; 
			}
			return (lpFldInfo->type);
		}
		else
			lpFldInfo++;
	}
	return SQL_CHAR;
}

/******************************************************************/
LPFIELDINFO GetExternalFieldInfo( HANDLE DBhandle, BOOL first, LPBOOL pHaveNonStandardFields)
{
RETCODE rc;       
SWORD Nullable;
static HSTMT hstmt;
short	i, drivenum,ii;  
HANDLE	hMem=GSSiGlobAlloc ( 162,GMEM_MOVEABLE,4096);  
LPSTR	TableName=GlobalLock (hMem);
LPSTR	str = TableName+256;
LPSTR	dir = str + 256;
LPSTR	file = dir + 256;
LPSTR	drive = file + 128;
LPSTR	ext = drive + 32;
static  char	cwd[MAX_PATH];   
static	short	SaveDrive;
LPSTR	pOwner, pTable;   
short	lOwner;
static	char	FirstName[128];


	lpmine = &mine;

  i=(int)DBhandle;  
  if (!*TableNames[i]) 
  {
    GSSiGlobUlFree (&hMem);	
  	return NULL; 
  }
  if(first)
  { 
  	*FirstName = 0;
	SaveDrive = _getdrive();
	_getcwd (cwd,MAX_PATH); 
    mine.precision = 0;
    mine.scale = 0;
    mine.radix = 0;
    mine.length = 0; 
    SQLAllocStmt(HDBCS[i], &hstmt); 
    if (_fstrstr (OpenDBNames[DBNamePtr[i]],"DBF|"))
    {
    	_fullpath (str,TableNames[i],256);
    	_splitpath (str,drive,dir,TableName,ext); 
    	sprintf (str,"%s%s",drive,dir); 
    	drivenum = GetDriveNum(*drive);
		_chdrive (drivenum);
	 	_chdir (str); 
		_getcwd (str,256);    
		pOwner = 0;
		lOwner = 0;
    }
    else
    {  
    	_fstrcpy (TableName,TableNames[i]);
    	if (DBType[i] != 'T' &&
    		!_fstrstr(TableName,".TXT") && !_fstrstr(TableName,".DBF") && (pTable = _fstrchr (TableName,'.')))
    	{
    		pOwner = TableName;
    		*pTable++ = 0;    
    		lOwner = _fstrlen (pOwner);
    	}
    	else 
    	{
    		pTable = TableName;
    		pOwner = 0;
    		lOwner = 0;
    	}
    	
    }
    rc = SQLColumns(hstmt, 0, 0, pOwner,lOwner, pTable, SQL_NTS, 0, 0);
    if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)  goto s44;
    SQLBindCol(hstmt,  1, SQL_C_CHAR,   szQual, STR_LEN, &cbTableQual); 
    SQLBindCol(hstmt,  2, SQL_C_CHAR,   szTableOwner, STR_LEN, &cbTableOwner); 
    SQLBindCol(hstmt,  3, SQL_C_CHAR,   szTableName, STR_LEN, &cbTableName); 
    SQLBindCol(hstmt,  4, SQL_C_CHAR,   mine.name, 62, &cbColumnName); 
    SQLBindCol(hstmt,  5, SQL_C_SHORT, &mine.type, 2, &cbDataType); 
    SQLBindCol(hstmt,  6, SQL_C_CHAR,   szTypeName, STR_LEN, &cbTypeName); 
    SQLBindCol(hstmt,  7, SQL_C_LONG,  &mine.precision, 4, &cbPrecision); 
    SQLBindCol(hstmt,  8, SQL_C_LONG,  &mine.length, 4, &cbLength); 
    SQLBindCol(hstmt,  9, SQL_C_SHORT, &mine.scale, 2, &cbScale); 
    SQLBindCol(hstmt, 10, SQL_C_SHORT, &mine.radix, 2, &cbRadix); 
    SQLBindCol(hstmt, 11, SQL_C_SHORT, &Nullable, 2, &cbNullable); 
    SQLBindCol(hstmt, 12, SQL_C_CHAR,   szRemarks, REM_LEN, &cbRemarks); 
   } //end of if (first) 
    mine.type = 0;
    mine.index = 0;
Next:
    rc = SQLFetch(hstmt);
    while(rc == SQL_SUCCESS && _fstrcmp (FirstName,mine.name))
    {    
    	if (!*FirstName)
    		_fstrcpy (FirstName,mine.name); 
    	if (mine.type < 0)
    		ii=1;
    	if (mine.type < 0 && mine.type != SQL_BIT && mine.type != SQL_TINYINT && mine.type != SQL_GUID
    					  && mine.type != SQL_BIGINT && mine.type != SQL_LONGVARCHAR && mine.type != SQL_UNKCHAR && mine.type != SQL_MSSHAPE) 
    	{   
			if (mine.type != SQL_LONGVARBINARY || !UseLongVarBinary)
			{
				if (!stricmp(mine.name, "SHAPE"))
				{
					mine.type = SQL_MSSHAPE;
				}
				else
				{
					*pHaveNonStandardFields = TRUE;
					if (!ShowNonStandardFields)
						goto Next;
				}
    		} 
    	}
    	switch (mine.type)
    	{
//       		lpmine->type = FindType(szTypeName); 
//       		break;
       		
       		case SQL_DATE:
			case SQL_TIME:
			case SQL_TIMESTAMP:
			case SQL_GUID:
//				lpmine->type = SQL_VARCHAR;
				lpmine->length = max (lpmine->length,lpmine->precision);
			break;
            case SQL_LONGVARBINARY:
            	ii=1;
       		default:
       		break;
       	} 
       GSSiGlobUlFree (&hMem);	
       return lpmine;
    
    }
s44:SQLCloseCursor(hstmt);
	SQLFreeStmt(hstmt, SQL_DROP);
	_chdrive (SaveDrive);
 	_chdir (cwd); 
     GSSiGlobUlFree (&hMem);	
     return NULL;
}  

void SetODBCPassword2 (LPSTR str) 
{   
	LPSTR	UserID, Password, pColon;
	
	if (!(UserID = _fstrstr (str,"UID=")))
		return;
	UserID += 4;
	if (!(pColon = _fstrchr (UserID,';')))
		return; 
	*pColon++ = 0;
	if (!(Password = _fstrstr (pColon,"PWD=")))
		return;
	Password += 4;
	if ((pColon = _fstrchr (UserID,';')))
		*pColon++ = 0;
	SetODBCPassword (UserID,Password);        
	return;
}

void CloseODBCPasswordFile (void)
{
    GSSiGlobFree (&hODBCPW);
	return;
}

void SetODBCPassword (LPSTR UserID,LPSTR Password)
{   
	LPPWINFO	pPWInfo;  
	int	i;       
	LPSHORT	pnumPW; 
	LPSTR	pSC = _fstrchr (Password,';'); 
		
	if (!hODBCPW)  
	{
		hODBCPW=GSSiGlobAlloc ( 163,GHND,2+64*sizeof(PWINFO));    
	}
	pnumPW = (LPSHORT)GlobalLock (hODBCPW);
	pnumPW++;
	pPWInfo = (LPPWINFO)pnumPW;
	pnumPW--;
	for (i=0;i<*pnumPW;i++,pPWInfo++)
	{
		if (!_fstricmp (CurODBCFile,pPWInfo->File))
			goto Exit;
	} 
	(*pnumPW)++;                
Exit:
	_fstrcpy (pPWInfo->File,CurODBCFile);
	_fstrcpy (pPWInfo->User,UserID); 
	if (pSC)
		*pSC = 0;
	_fstrcpy (pPWInfo->PW,Password);
	GlobalUnlock (hODBCPW);
	return;
}
BOOL AddPWtoODBCFile (LPSTR lpcstring)
{
	LPPWINFO	pPWInfo;  
	int	i;
	LPSHORT	pnumPW; 
	BOOL	rtn=FALSE;
	char	str[MAX_PATH+2];
    
    if (!hODBCPW)
    {
		if (GetGlobalCVal ("[%ODBCPASSWORDFILE]",str,0))
		{   
			char		SaveCurODBCFile[MAX_PATH];
			OFSTRUCTGM	OFStruct;
			HFILE		Fid=GSSiOpenFile (str,&OFStruct,OF_READ);
			LPSTR		pSpace; 
			
			if (Fid == HFILE_ERROR)
				return FALSE;   
			_fstrcpy (SaveCurODBCFile,CurODBCFile);

			while (fgetstring (str,MAX_PATH,Fid))
			{   
				if ((pSpace = _fstrstr (str," UID=")))
				{   
					*pSpace ++ = 0;
					_fstrcpy (CurODBCFile,str);
					SetODBCPassword2 (pSpace);  
				}
			}
			GSSiClose (Fid);
			_fstrcpy (CurODBCFile,SaveCurODBCFile);
		    if (!hODBCPW)
		    	return FALSE;
		}
		else
    		return FALSE;
    }
	pnumPW = (LPSHORT)GlobalLock (hODBCPW);
	pnumPW++;
	pPWInfo = (LPPWINFO)pnumPW;
	pnumPW--;
	for (i=0;i<*pnumPW;i++,pPWInfo++)
	{
		if (!_fstricmp (CurODBCFile,pPWInfo->File))  
		{
			sprintf (_fstrchr(lpcstring,0),";UID=%s;PWD=%s",pPWInfo->User,pPWInfo->PW);   
			rtn=TRUE;
			goto Exit;
		}
	}                 
Exit:
	GlobalUnlock (hODBCPW);
	return rtn;
}

void SubstituteDBQ (LPSTR str,LPSTR pDBQ)
{
	LPSTR	pDBQstr,pEnd;
	HANDLE	hDBQ;
	LPSTR	DBQ;
	LPSTR	pDB,pTable,pBAR;

	if (!pDBQ)
		return;
	if (!*pDBQ)
		return;
	hDBQ = GSSiGlobAlloc (0,GMEM_MOVEABLE,2048);
	DBQ = GlobalLock (hDBQ);
	strcpy (DBQ,pDBQ);
	pDB = DBQ + 1024;
	pTable = pDB + 512;
	if ((pEnd = strchr (DBQ+1,';')))
		*pEnd = 0;
	if (!strstr (DBQ,"[%DL]"))
	{
		SubstituteDL (DBQ+5,FALSE);
	}
	strcpy (pDB,str);
	if ((pBAR = strchr (pDB,'|')))
	{
		if ((pBAR = strchr (pBAR+1,'|')))
		{
			*pBAR++ = 0;
			strcpy (pTable,pBAR);
		}
		else
			*pTable = 0;
	}
	if (!(pDBQstr = strstr(str, ";DBQ=")))
		pDBQstr = strstr(str, ";DBQ@=");
	if (!pDBQstr)
		strcat (pDB,DBQ);
	else
	{
		HANDLE	hNewStr=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
		LPSTR	pNewStr=GlobalLock (hNewStr);
		LPSTR	pEndStr;

		*pDBQstr++ = 0;
		pEndStr = strchr (pDBQstr,';');
		strcpy (pNewStr,str);
		strcat (pNewStr,DBQ);
		if (pEndStr)
			strcat (pNewStr,pEndStr);
		GSSiGlobUlFree (&hNewStr);
	}
	if (*pTable)
		sprintf (str,"%s|%s",pDB,pTable);
	else
		strcpy (str,pDB);
	GSSiGlobUlFree (&hDBQ);
	return;
}

LRESULT CALLBACK WndProcEXTDB(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
}


ATOM EXTDBRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	static ATOM c;
	static BOOL first = TRUE;

	if (!first)
		return c;
	first = FALSE;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProcEXTDB;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(NULL, IDC_IBEAM);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_GMEDIT);
	wcex.lpszClassName = "EXTDBWindow";
	wcex.hIconSm = 0;

	c = RegisterClassEx(&wcex);
	return c;
}

HWND InitEXTDBInstance(HWND hWndPar, LPRECT pRect)
{
	HWND hWnd;

	//hWndPar=0;
	hWnd = CreateWindow("EXTDBWindow", "External DB", WS_POPUPWINDOW | WS_VSCROLL | WS_HSCROLL,
		pRect->left, pRect->right, RECTWIDTH(pRect), RECTHEIGHT(pRect), hWndPar, NULL, hInst, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	//MoveWindow (hWnd,0,0,500,500,TRUE);
	return hWnd;
}


/******************************************************************/
HANDLE OpenExternalDatabase( LPSTR Inname)
{
static BOOL first_entry = TRUE;
int  i, next_one, j;  
BOOL	SaveDoPaint, DoPrompt=FALSE;
HDBC hdbc;
RETCODE rc; 
HANDLE	htnames=GSSiGlobAlloc ( 164,GHND,2048+256+1024+256+256+256+1024);
SDWORD nerr;
SWORD mlen = 253, Moutlen, maxoutlen = 1024, outlen;
LPSTR cptr;
char  uid[]="admin", pwd[]=""; 
LPSTR	lptnames, lpcstring, lpUID, pPassWord;
UWORD fDirection = SQL_FETCH_FIRST; //SQL_FETCH_NEXT
short	SaveDrive;
LPSTR tnames = GlobalLock (htnames);
LPSTR errmess = tnames + (1024+512);  
LPSTR DBAndTable = tnames + 2048;
LPSTR cstring = tnames + (2048+256);    
LPSTR cwd = tnames + (2048+256+1024);
LPSTR File = tnames + (2048+256+1024+256);  
LPSTR DriverName = tnames + (2048+256+1024+256+256);  
LPSTR name = DriverName+256;
LPSTR pDBQ, pSC;
short	qclen;
char	additional[256]={0};

	strcpy (name,Inname);
	ExpandText (name);
//	MessageBox (0,name,"Open",MB_OK);
	SaveDrive = _getdrive();
	_getcwd (cwd,256); 
   if(first_entry)
   {
     for (i = 0; i < MAX_OPEN_DATABASES; DBType[i] = -1, i++);
     first_entry = FALSE;
   }
   for(i = 1; i < MAX_OPEN_DATABASES; i++)
   		if(DBType[i] == -1)
   			goto FoundOne; 
   MessageBox (0,"Open database limit reached",0,MB_ICONEXCLAMATION);
   return 0;  
FoundOne:
   cptr = _fstrchr(name,'|');
   if(cptr)
   {
      _fstrcpy(tnames,&name[5]);
      cptr = _fstrchr(tnames,'|');
   }
   else
      _fstrcpy(tnames,name);
 
   if(cptr) *cptr = 0;
   next_one = i;
   
   for (i=0;i<NumOpenDBs;i++)
   {
   		if (OpenhDBs[i] && !_fstrcmp (OpenDBNames[i],tnames))
   		{
		   DBType[next_one] = OpenDBType[i];
		   HDBCS[next_one] = OpenhDBs[i]; 
		   DBOpenCount[i]++;
		   DBNamePtr[next_one] = i;
			_chdrive (SaveDrive);
		 	_chdir (cwd);
		   GSSiGlobUlFree (&htnames); 
		   return (HANDLE) next_one;
   		}
   } 
    if ((pSC = strchr (tnames,';')))
   {
	   strcpy (additional,pSC);
 	   *pSC = 0;
   }
   _fstrcpy (DBAndTable,tnames);  
   cptr = _fstrstr (tnames,"(CHAN");
   if(cptr) *cptr = 0;
   _fstrcpy (CurODBCFile,tnames);   
   sprintf (DriverName,"[%%STANDARDSQL_%s",tnames);   
   cptr = _fstrchr (DriverName,';');
   if(cptr) *cptr = 0;
   _fstrcat (DriverName,"]");
   
   _fstrcpy(cstring,"DSN=");
   _fstrcat(cstring,tnames);                                        
   if (!henv)
   		SQLAllocEnv(&henv);
   SQLAllocConnect(henv, &hdbc);
   SQLSetConnectOption(hdbc,SQL_ODBC_CURSORS,SQL_CUR_USE_IF_NEEDED);
   tnames[0] = '\0';
   lptnames = (LPSTR)tnames;
   lpcstring = (LPSTR)cstring; 
   SaveDoPaint = DoPaint();
TryAgain:
//testvalue(1);
   setDoPaint( FALSE);  
   lpUID = _fstrstr (lpcstring,";UID=");
   if (lpUID) *lpUID=0;
   if (!AddPWtoODBCFile (lpcstring) && lpUID)
   		*lpUID = ';';
   
//testvalue(1);
   //strcat (lpcstring,";DATABASE=dbtgis_pwSWS");
   strcat (lpcstring,additional);
   if (DoPrompt)
	   rc = SQLDriverConnect(hdbc, hWndMain, lpcstring, SQL_NTS, lptnames, maxoutlen, &Moutlen,SQL_DRIVER_PROMPT);
   else
   {
       HCURSOR hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	   //HDC hDC = GetDC(hWndMain);
	   HWND hWnd;
	   RECT rect = { 0, 0, 100, 100 };
	   
	   //SaveDC(hDC);

//testvalue(1); 
		SaveCurView (0);
//		MessageBox (0,lpcstring,"Connect",MB_OK);
		//EXTDBRegisterClass(hInst);
		//hWnd = InitEXTDBInstance(0, &rect);
//calling this with MS Access Database driver messes up font in main window
		rc = SQLDriverConnect(hdbc, 0, lpcstring, strlen(lpcstring), lptnames, maxoutlen, &Moutlen, SQL_DRIVER_COMPLETE_REQUIRED);
//testvalue(1); 
	   SaveCurView (1);
	   GSSiSetCursor (hcurSave);
	  // RestoreDC(hDC, -1);
	  // ReleaseDC(hWndMain, hDC);
   }
   setDoPaint( SaveDoPaint);
//testvalue(1);
   if(rc ==  SQL_ERROR)
   {
     SQLError(henv,hdbc,SQL_NULL_HSTMT,tnames, &nerr, errmess,mlen,&outlen); 
     if (!_fstricmp (tnames,"28000") || !_fstricmp (tnames,"IM008"))
     {
		DLGPROC lpfnDBLOGINMsgProc;
		int	nRc;
			
		lpfnDBLOGINMsgProc = MakeProcInstance((DLGPROC)DBLOGINMsgProc, hInst);
		nRc = DialogBox(hInst, (LPSTR)"DBLOGIN", hWndMain, lpfnDBLOGINMsgProc);
		FreeProcInstance(lpfnDBLOGINMsgProc);
		if (nRc)
	     	goto TryAgain; 
	    goto s44;
     } 
     if ((pPassWord = _fstrstr (lpcstring,"PWD=")))
     {  
     	pPassWord += 4;
     	while (*pPassWord && *pPassWord != ';')
     		*pPassWord++ = '*';
     }
	 if (GetGlobalBVal ("[%BLOCKODBCERROR]"))
		 HaltReport=TRUE;  
	 else if (GSSiMessageBox(errmess,lpcstring,MB_OKCANCEL|MB_ICONQUESTION|MB_TASKMODAL,0) == IDCANCEL)
	     	 HaltReport=TRUE;  
   }  

   if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)  goto s44;
   SetODBCPassword2 (lptnames);  
   if (_fstrstr (lptnames,";FIL=text;"))                                                      
   	DBType[next_one] = 'T';
   else
   	DBType[next_one] = 'O';
   if ((pDBQ=strstr (lptnames,";DBQ=")))
   {
	   LPSTR	pEnd = strchr (pDBQ+1,';');

	   if (pEnd)
		   *pEnd = 0;
	   SubstituteDBQ (Inname,pDBQ);
   }
   HDBCS[next_one] = hdbc;
   for (i=0;i<NumOpenDBs;i++)
   {
   		if (!OpenhDBs[i])
   			goto s43;
   }
   i=NumOpenDBs++;
s43:
   _fstrcpy (OpenDBNames[i],DBAndTable);   
   strcat (OpenDBNames[i],additional);
   StandardSQL[i] = GetGlobalBVal2 (DriverName,StanSQL);
   SQLGetInfo (hdbc,SQL_IDENTIFIER_QUOTE_CHAR,(PTR)QuoteChar[i],2,&qclen); 
   if (!qclen)
   	*QuoteChar[i] = 0;
   DBNamePtr[next_one] = i;
   DBOpenCount[i]=1;
   OpenhDBs[i]=hdbc; 
   OpenDBType[i] = DBType[next_one];
	_chdrive (SaveDrive);
   _chdir (cwd); 
   GSSiGlobUlFree (&htnames); 
   return (HANDLE) next_one;
  
  
s44:  SQLDisconnect(hdbc);
      SQLFreeConnect(hdbc);
	_chdrive (SaveDrive);
 	_chdir (cwd); 
    GSSiGlobUlFree (&htnames); 
    return NULL;

   
} 

void ODBCTerminate (BOOL Quit)
{   
	if (!Quit && !GetGlobalBVal ("[%CLOSEODBC]"))
		return;
    while (NumOpenDBs--)
    { 
    	if (OpenhDBs[NumOpenDBs])
    	{
			SQLDisconnect(OpenhDBs[NumOpenDBs]);
			SQLFreeConnect(OpenhDBs[NumOpenDBs]);
		}
    }  
    NumOpenDBs = 0;
	if (!Quit)
		return;
    GSSiGlobFree (&hODBCParams);
	if (henv)
    	SQLFreeEnv(henv);
    henv = 0;
	return;
}	

/******************************************************************/
int NumDatabaseTables( char *type, HWND  DBhandle,int itype)
{
#define STR_RMK 254
SDWORD cbTableQual, cbTableOwner, cbTableName, cbTableType, cbRemarks;
UCHAR  szTableQual[STR_LEN+1],   szTableName[STR_LEN+1],
       szTableOwner[STR_LEN+1],  szTableType[STR_LEN+1],
       szRemarks[STR_RMK];  
       char	Owner[64]; 
	   char	TableTypes[34] = "'TABLE','VIEW'";
       short	lOwner, ln; 
       LPSTR	pTables;
	   char buffer[1024] = { 0 };
	   SQLSMALLINT	lbuf;
static HSTMT hstmt;      
		long	TotLen=0;
HDBC hdbc;           
RETCODE rc;
int icount; 
short	i;
	
    GSSiGlobFree (&hTableNames);
    i=(int)DBhandle;
    hdbc = HDBCS[i];
    SQLAllocStmt(hdbc, &hstmt);

	rc = SQLGetInfo(hdbc, SQL_DRIVER_NAME, buffer, sizeof(buffer) - 1, &lbuf);
	rc = SQLGetInfo(hdbc, SQL_DATA_SOURCE_NAME, buffer, sizeof(buffer) - 1, &lbuf);

//	if (itype == 4)
//		strcpy(TableTypes, "'SYSTEM TABLE','TABLE','VIEW'");

//    rc = SQLTables(hstmt,0,0,0,0,0,0,0,0); 
	GetGlobalCVal ("[%TABLEOWNER]",Owner,0);
	lOwner = _fstrlen (Owner);  
	if (lOwner)
    	rc = SQLTables(hstmt,0,0,Owner,lOwner,0,0,TableTypes,(short)_fstrlen(TableTypes));
    else
		rc = SQLTables(hstmt, 0, 0, 0, 0, 0, 0, TableTypes, (short)_fstrlen(TableTypes));
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO) goto s44;
	SQLBindCol(hstmt, 1, SQL_C_CHAR, szTableQual, STR_LEN, &cbTableQual);
	SQLBindCol(hstmt, 2, SQL_C_CHAR, szTableOwner, STR_LEN, &cbTableOwner);
	SQLBindCol(hstmt, 3, SQL_C_CHAR, szTableName, STR_LEN, &cbTableName);
	SQLBindCol(hstmt, 4, SQL_C_CHAR, szTableType, STR_LEN, &cbTableType);
	SQLBindCol(hstmt, 5, SQL_C_CHAR, szRemarks, STR_RMK, &cbRemarks);
	rc = SQLFetch(hstmt);
	if (!(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO))
	{
		strcpy(TableTypes, "'SYSTEM TABLE','TABLE','VIEW'");
		SQLCloseCursor(hstmt);
		SQLFreeStmt(hstmt, SQL_DROP);
		SQLAllocStmt(hdbc, &hstmt);
		rc = SQLTables(hstmt, 0, 0, 0, 0, 0, 0, TableTypes, (short)_fstrlen(TableTypes));
		SQLBindCol(hstmt, 1, SQL_C_CHAR, szTableQual, STR_LEN, &cbTableQual);
		SQLBindCol(hstmt, 2, SQL_C_CHAR, szTableOwner, STR_LEN, &cbTableOwner);
		SQLBindCol(hstmt, 3, SQL_C_CHAR, szTableName, STR_LEN, &cbTableName);
		SQLBindCol(hstmt, 4, SQL_C_CHAR, szTableType, STR_LEN, &cbTableType);
		SQLBindCol(hstmt, 5, SQL_C_CHAR, szRemarks, STR_RMK, &cbRemarks);
		rc = SQLFetch(hstmt);
	}
    icount = 1; 
    hTableNames = GSSiGlobAlloc ( 165,GHND,USHRT_MAX);
    pTables = GlobalLock (hTableNames);
    while ((rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) && TotLen < USHRT_MAX - 256)
    {   
    	short	ii;
    	if (!_fstrnicmp (szTableName,"JS",2))
    		ii=1;
     cbTableName = max (0,min (cbTableName,STR_LEN)); 
     szTableName[cbTableName] = 0;
     if (cbTableOwner > 0)
     	sprintf (pTables,"%s.%s",szTableOwner,szTableName);   
     else
     	_fstrcpy(pTables, szTableName);
     ln = _fstrlen (pTables);
     pTables += ln + 1;
     TotLen += ln + 1;
     rc = SQLFetch(hstmt);
     icount++;
    } 
    *pTables = 0;
    GlobalUnlock (hTableNames);
s44:SQLCloseCursor(hstmt);
	SQLFreeStmt(hstmt, SQL_DROP);
	return icount-1;
}  

long FindTableName (LPSTR DBName,LPSTR TablePartialName,LPSTR OutFile)
{   
	long	n=0;  
	HANDLE	hDB=0;  
	LPSTR	lpSTRING;   
	short	NS;   
	HANDLE	DBHandle;
	char	TableName[128];
	short	itype;
	
    if (!(itype=OpenDataFile (DBName,"",BT_READ,&hDB)))
    	return 0; 
    _fstrupr (TablePartialName);
	DBHandle = GetDBHandleFromSQL (hDB);
	lpSTRING =  GetTableName (DBHandle, TRUE,itype); 
	while (lpSTRING && *lpSTRING) 
	{   
		_fstrupr (lpSTRING);
		if (_fstrstr (lpSTRING,TablePartialName))
		{
			n++;
			AppendFile (OutFile,lpSTRING); 
		}
		lpSTRING =  GetTableName (DBHandle,FALSE,itype); 
	}
    CloseDataFile (TRUE, &hDB);
	return n;
}

long FindFieldName (LPSTR DBName,LPSTR FldPartialName,LPSTR OutFile)
{   
	long	n=0;  
	HANDLE	hDB=0;  
	LPSTR	lpSTRING;   
	short	NS;   
	HANDLE	DBHandle;
	char	TableName[128];
	short	itype;
	
    if (!(itype=OpenDataFile (DBName,"",BT_READ,&hDB)))
    	return 0; 
    _fstrupr (FldPartialName);
	DBHandle = GetDBHandleFromSQL (hDB);
	lpSTRING =  GetTableName (DBHandle, TRUE,itype); 
	while (lpSTRING && *lpSTRING) 
	{ 
		BOOL	First=TRUE;
		LPFIELDINFO pFld; 
		HANDLE	hDB2=0;
		
		sprintf (TableName,"%s|%s",DBName,lpSTRING,itype);
	    if (OpenDataFile (TableName,"",BT_READ,&hDB2)) 
	    {
			LPFIELDINFO	lpFieldInfo;
			LPOPENFILEDATA	FilePtr;
			LPOPENSQLDATA	SQLPtr;
			short	ifield;

			SQLPtr = (LPOPENSQLDATA)GlobalLock(hDB2);
			FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
			  
			lpFieldInfo = &FilePtr->FldInfo; 
			for (ifield=0;ifield<FilePtr->NumFields;ifield++,lpFieldInfo++)
			{   
				char	testname[66];  
				short	ii;
				
				_fstrcpy (testname,lpFieldInfo->name);
				_fstrupr (testname);
				if (_fstrstr (testname,FldPartialName)) 
				{
					char mess[520];
					
					n++;
					sprintf (mess,"%s in %s",lpFieldInfo->name,lpSTRING); 
					AppendFile (OutFile,mess); 
				}
			} 
			GlobalUnlock (SQLPtr->OFHandle);
			GlobalUnlock (hDB2);
		    CloseDataFile (TRUE, &hDB2);
		}
		lpSTRING =  GetTableName (DBHandle,FALSE,itype); 
	}
    CloseDataFile (TRUE, &hDB);
	return n;
}

/******************************************************************/
LPSTR GetTableName (HANDLE DBhandle, BOOL first,int itype)
{
	static int icount, num_of_em;
	LPSTR	pTable;
	short	i;
	
    if(first)
    {
      num_of_em = NumDatabaseTables( "TABLE", DBhandle,itype);
      icount = 1;
    }  
    else
    {
      icount++;  
      if(!hTableNames || icount > num_of_em)
      {  
      	 GSSiGlobFree (&hTableNames);
      	 return NULL;
      }
    } 
    if(!hTableNames)
    	return NULL;                   
	pTable = GlobalLock (hTableNames);
	for (i=1;i<icount;i++)
		pTable = _fstrchr (pTable,0)+1;
	_fstrcpy (CurTable,pTable);
	GlobalUnlock (hTableNames);
	return &CurTable[0];
}


/******************************************************************/
int OpenDatabaseTable (int DBHandle, LPSTR TableName)
{ 

  _fstrcpy(&TableNames[DBHandle][0], TableName);
  return DBHandle;
  
} 

/*******************************************************************/ 
RETCODE GetDataSource(HANDLE CThemeDB, BOOL first, LPSTR names,
           short maxnamelen, short *namelen, LPSTR desc, short maxdesclen,
           short *desclen)
{
RETCODE rc;
static HDBC hdbc;
             if(first)
             {  
             	if (!henv)
                   SQLAllocEnv(&henv);
                 SQLAllocConnect(henv, &hdbc);
                 rc = (SQLDataSources(henv, SQL_FETCH_FIRST,
                       names, maxnamelen, namelen, desc,  maxdesclen,
                        desclen));
                 if(rc == SQL_NO_DATA_FOUND) goto s44;       
             }
             else        
             { 
                 rc = (SQLDataSources(henv, SQL_FETCH_NEXT,
                       names, maxnamelen, namelen, desc,  maxdesclen,
                        desclen));
                 if(rc == SQL_NO_DATA_FOUND) goto s44;       
             }
             return rc;
s44:         SQLDisconnect(hdbc);
             SQLFreeConnect(hdbc);
             return rc;    
}              
/******************************************************************/

/*
void GetExternalReport ( HANDLE DBhandle, LPCSTR index, LPVOID keydata,
                              LPFIELDINFO field,LPCSTR R_or_F_Name)
// this doesn't work because vendor's don't support SQLProcedures yet.  lda Aug 94

                                 
{  // index[] = "pidno", keydata[] = "0102824110013" ;                            
int   i,j, k; 
RETCODE rc;
HSTMT hstmt;
HENV  henv;
HDBC hdbc;
UDWORD collen;
UWORD functions[101];
UCHAR szProcName[32],szRemark[128];
short ProcType, count;
SDWORD cbProcName, cbRemark, cbProcType;
SWORD errmesslen;
char sqlstr[128], byte, *point, tnames[128], errmess[128];
    for(i = 1; i < 64; i++) if(DBS[i] == DBhandle)break;
    hdbc = HDBCS[i];
    SQLAllocStmt(hdbc, &hstmt); 
    SQLBindCol(hstmt,  3, SQL_C_CHAR,    szProcName, 32, &cbProcName); 
    SQLBindCol(hstmt,  7, SQL_C_CHAR,    szRemark,  127, &cbRemark); 
    SQLBindCol(hstmt,  8, SQL_C_SSHORT,  &ProcType,   0, &cbProcType); 
   rc = SQLGetFunctions(hdbc,SQL_API_ALL_FUNCTIONS,functions); 
 //  rc = SQLProcedures(hstmt,NULL,0,NULL,0,NULL,0); 
   if(rc ==  SQL_ERROR)
   {
     SQLError(SQL_NULL_HENV,SQL_NULL_HDBC, hstmt, tnames,
      &cbRemark , errmess, 127, &errmesslen);
     goto s44;
   }  
    count = 0;
    if(rc != SQL_SUCCESS) goto s44;
    rc = SQLFetch(hstmt);
    while (rc)
    {
      count++;
    }
   // rc = SQLExecDirect(hstmt, sqlstr, SQL_NTS);

   // if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)goto s44;

  
    
s44: SQLFreeStmt(hstmts, SQL_DROP);
     return ;
}
*/
/******************************************************************/
void CloseExternalDatabase (int ClientDDE)
{     
	 short	i; 
	 RETCODE	rc;
	 
      if(ClientDDE == 0)
      	return;
      DBType[ClientDDE] = -1;
      *TableNames[ClientDDE] = 0; 
      i = DBNamePtr[ClientDDE]; 
      DBOpenCount[i]--;
      if (DBOpenCount[i])
      	return;
      if (!GetGlobalBVal2 ("[%CLOSEODBC]",FALSE))
      	return;  
	  if (OpenhDBs[i])
	  {
		rc = SQLDisconnect(OpenhDBs[i]);
		rc = SQLFreeConnect(OpenhDBs[i]);
	  }
      OpenhDBs[i] = 0; 
      if (i+1 == NumOpenDBs)
      	NumOpenDBs--;
	  if (!NumOpenDBs)
	  {
		if (henv)
    		SQLFreeEnv(henv);
		henv = 0;
	  }
      return;
} 
  // The following is a prototype that isn't ready yet... lda  
/********************************************************************/
/*LPFIELDINFO XXXGetExternalFieldInfo( HANDLE DBhandle, BOOL first)
{
#define STR_LEN 128+1 
#define REM_LEN 254+1
static FIELDINFO mine;
static FIELDINFO FAR *lpmine = &mine;
static UWORD icount; 
UCHAR szTypeName[32];
SDWORD  pfDesc ;
RETCODE rc;       
SWORD cbDescMax = 254, pcbDesc;
HSTMT hstmt;
char space[255];
PTR rgbDesc = space;
  if(first)
  {
    for(i = 1; i < 64; i++) if(DBS[i] == DBhandle)break; 
    mine.precision = 0;
    mine.scale = 0;
    mine.radix = 0;
    mine.length = 0;
    icount = 0; 
    SQLAllocStmt(HDBCS[i], &hstmt);
   } 
    mine.type = 0;
    icount++;
    rc = SQLColAttributes(hstmt,icount,SQL_COLUMN_NAME, rgbDesc, cbDescMax, &pcbDesc, &pfDesc);
    while(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    { 
       if(mine.type == 0)lpmine->type = FindType(szTypeName); 
       return lpmine;
    
    }
     SQLFreeStmt(hstmt, SQL_DROP);
     return NULL;
}*/             

BOOL GetSQLWhereClause (HWND hWnd, HANDLE hDB,  LPSTR Where)
{
	DLGPROC lpfnSQLWHEREMsgProc; 
	BOOL	nRc; 
	
	if (!hDB)
		return FALSE;
	SQLhDB = hDB;  
	_fstrcpy (SQLString,Where);
	lpfnSQLWHEREMsgProc = MakeProcInstance((DLGPROC)SQLWHEREMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"SQLWHERE", hWnd, lpfnSQLWHEREMsgProc);
	FreeProcInstance(lpfnSQLWHEREMsgProc); 
	if (nRc)
		_fstrcpy (Where,SQLString);    
	return nRc;
}
                             
BOOL FAR PASCAL SQLWHEREMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
	char	str[128], cUID[32];	
	int		Choice, ibeg,ii; 
	LPSTR	lpstr;
	LPSTR	lpTAB;
	char	OPCODE[17][16]={" = "," <> "," > "," !> "," < "," !< "," >= "," <= "," IS NULL "," IS NOT NULL ","","","",""," AND "," OR "," NOT "};
	RECT	Rect;  
	int		BRtn;

 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
    {    
#define	NUMOPS	17
#define MAXOP	28
    	char	OPS[NUMOPS][MAXOP]={"Equal to","Not equal to","Greater than","Not greater than","Less than","Not less than","Greater than or equal to","Less than or equal to","IS NULL","IS NOT NULL","BETWEEN","LIKE","IN","","AND","OR","NOT"};
    	int i=0;  
		LPOPENFILEDATA	FilePtr;
		LPOPENSQLDATA	SQLPtr;
		LPFIELDINFO	lpFieldInfo; 
		int		TabStops=1000;   
    	
    	if (!SQLhDB)
    	{
    		PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L); 
    		break;
    	} 
                     
		SQLWherehWnd = hWndDlg;
		
    	SetDlgItemText (hWndDlg,IDC_SQL,SQLString);   
		SendDlgItemMessage (hWndDlg,IDC_FIELDLIST,LB_SETTABSTOPS,1,(LPARAM)&TabStops);
		SQLPtr = (LPOPENSQLDATA)GlobalLock (SQLhDB);
		FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
		lpFieldInfo = &FilePtr->FldInfo; 
		for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++) 
		{
  			sprintf (str,"%s\t%i",lpFieldInfo->name,lpFieldInfo->type);
		 	SendDlgItemMessage (hWndDlg,IDC_FIELDLIST,LB_ADDSTRING,0,(LPARAM)str);
		}			
		GlobalUnlock (SQLPtr->OFHandle); 
		GlobalUnlock (SQLhDB);
		
		for (i=0;i<NUMOPS;i++)
		 	SendDlgItemMessage (hWndDlg,IDC_OPLIST,LB_ADDSTRING,0,(LPARAM)OPS[i]);
		SendDlgItemMessage (hWndDlg,IDC_GLOBALLIST,LB_ADDSTRING,0,(LPARAM)"[%UDI]");
		SendDlgItemMessage (hWndDlg,IDC_GLOBALLIST,LB_ADDSTRING,0,(LPARAM)"[%INT_REFNO]");
		SendDlgItemMessage (hWndDlg,IDC_GLOBALLIST,LB_ADDSTRING,0,(LPARAM)"[%MSLINK]");
    	PostMessage(GetDlgItem(hWndDlg, IDC_SQL), EM_SETSEL, 0,  -1);
			
        break; /* End of WM_INITDIALOG                                 */
    }
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDC_OPLIST: 
	            switch(HIWORD(wParam))
	            {
	                 case LBN_SELCHANGE:
	                 case LBN_DBLCLK:
		                 Choice=SendDlgItemMessage(hWndDlg,IDC_OPLIST,LB_GETCURSEL,0,0); 
					 switch (Choice)
					 {  
					 	default: 
					 	{ 
							DLGPROC lpfnMsgProc; 
							BOOL	nRc;    
							
					    	GetWindowRect (GetDlgItem(hWndDlg,IDC_GLOBALLIST),&Rect); 
					        DisplayFieldList (hWndDlg,0,&Rect,1,0);
							_fstrcpy (SQLValOp,OPCODE[Choice]);
							lpfnMsgProc = MakeProcInstance((DLGPROC)SQL_VALUEMsgProc, hInst);
							nRc = DialogBox(hInst, (LPSTR)"SQL_VALUE", hWndDlg, lpfnMsgProc);
							FreeProcInstance(lpfnMsgProc); 
							hFldAppendWnd=0;
							DestroyFieldList();    
					 	}
					 	break;
					 	case 8:
					 	case 9:  
					 	case 14:
					 	case 15:
					 	case 16: 
					 		SendDlgItemMessage (hWndDlg,IDC_SQL,EM_REPLACESEL,0,(LPARAM)OPCODE[Choice]);
					 	break;  
					 	case 10:
					 	case 11:
					 	case 12:
					 	{
							DLGPROC lpfnMsgProc; 
							BOOL	nRc;    
							int		i=Choice-10;
							char	DiaName[3][16]={"SQL_BETWEEN","SQL_LIKE","SQL_IN"};
							FARPROC	MsgProc[3]={SQL_BETWEENMsgProc,SQL_LIKEMsgProc,SQL_INMsgProc};
							
					    	GetWindowRect (GetDlgItem(hWndDlg,IDC_GLOBALLIST),&Rect); 
					        DisplayFieldList (hWndDlg,0,&Rect,1,0);
							lpfnMsgProc = MakeProcInstance((DLGPROC)MsgProc[i], hInst);
							nRc = DialogBox(hInst, (LPSTR)DiaName[i], hWndDlg, lpfnMsgProc);
							FreeProcInstance(lpfnMsgProc);     
							hFldAppendWnd=0;
							DestroyFieldList();    
						} 
						case 13:
						break;
					 }
				}
		        break;
		    case IDC_ALLROWS:  
		    	_fstrcpy (SQLString,"ALL ROWS");
                EndDialog(hWndDlg, TRUE);
		    	break;    
            case IDC_GLOBALLIST: 
            case IDC_FIELDLIST: 
	            switch(HIWORD(wParam))
	            {
	                 case LBN_SELCHANGE:
	                 case LBN_DBLCLK:
	                 {
	                 	char	str2[40];
	                 	
		                 Choice=SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0); 
				         SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETTEXT,Choice,(DWORD)str);
				         if ((lpTAB = _fstrchr (str,'\t')))
				         {
				         	*lpTAB++=0;
				         	SQLCurFieldType = atoi (lpTAB);
				         }
				         if (_fstrchr (str,' '))
				         	sprintf (str2,"\"%s\"",str);
				         else
				         	_fstrcpy (str2,str);
					 	 SendDlgItemMessage (hWndDlg,IDC_SQL,EM_REPLACESEL,0,(LPARAM)str2); 
					 }
					 break;    
				}
		        break;
		    
		    case IDOK:
		    	GetDlgItemText (hWndDlg,IDC_SQL,SQLString,1024);
                EndDialog(hWndDlg, TRUE);
		        break;
		        
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break;
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} /* End of SQLWHEREMsgProc                                      */

BOOL FAR PASCAL SQL_LIKEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
	char	str[128];	

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
    
         cwCenter(hWndDlg, -1);
		 hFldAppendWnd=GetDlgItem (hWndDlg,IDC_LIKE);			
			
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);
                break;
                
            case IDOK: 
            {
            	HANDLE	hMem;
            	LPSTR	lpstr, lpstr2;    
            	
            	hMem = GSSiGlobAlloc ( 166,GMEM_MOVEABLE,2048);
            	
            	lpstr = GlobalLock (hMem);  
            	if (!GetDlgItemText (hWndDlg,IDC_LIKE,lpstr,256))
            	{   
			        GSSiMsgBox(hWndDlg, "Missing or invalid values", 0, MB_ICONEXCLAMATION,0);
              		goto Invalid;
            	}
		 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)" LIKE ");
            	if ((SQLCurFieldType == SQL_CHAR || SQLCurFieldType == SQL_VARCHAR || SQLCurFieldType == SQL_UNKCHAR) && *lpstr != '\'')
            	{
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)"'");
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)lpstr);
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)"'");
            	}
            	else 
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)lpstr);
                EndDialog(hWndDlg, TRUE);
         Invalid:  
         		GSSiGlobUlFree (&hMem);  
         		break;
         	}
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL SQL_INMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
	char	str[128];	

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
    
         cwCenter(hWndDlg, -1);
		 hFldAppendWnd=GetDlgItem (hWndDlg,IDC_IN);			
			
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break;
            case IDOK: 
            {
            	HANDLE	hMem;
            	LPSTR	lpstr, lpEnd, lpBeg, lpCom;  
            	BOOL	more,first;  
            	
            	hMem = GSSiGlobAlloc ( 167,GMEM_MOVEABLE,1024);
            	
            	lpstr = GlobalLock (hMem);  
            	if (!GetDlgItemText (hWndDlg,IDC_IN,lpstr,1024))
            	{   
			        GSSiMsgBox(hWndDlg, "Missing or invalid values", 0, MB_ICONEXCLAMATION,0);
              		goto Invalid;
            	}
            	if (*lpstr == '(')
            		lpBeg = lpstr+1; 
				else
					lpBeg = lpstr;
		 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)" IN(");
            	lpEnd = _fstrchr (lpstr,0);
            	lpEnd--;  
            	if (*lpEnd == ')') 
            		*lpEnd = 0; 
            	else
            		lpEnd++;
            	more = TRUE; 
            	first = TRUE;
            	while (more)
            	{   
            		if (!first)
			 			SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)",");
            		first = FALSE;
            		lpCom = _fstrchr (lpBeg,','); 
            		if (!lpCom)
            		{
            			more = FALSE;
            			lpCom = lpEnd;
            		} 
            		*lpCom = 0;
            		if (*lpBeg != '\'' &&
            			 (SQLCurFieldType == SQL_CHAR || SQLCurFieldType == SQL_VARCHAR || SQLCurFieldType == SQL_UNKCHAR))
				 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)"'");
				 	SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)lpBeg);
				 	lpBeg = lpCom-- + 1;
            		if (*lpCom != '\'' &&
            			 (SQLCurFieldType == SQL_CHAR || SQLCurFieldType == SQL_VARCHAR || SQLCurFieldType == SQL_UNKCHAR))
				 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)"'");
				} 	
				SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)")");
                EndDialog(hWndDlg, TRUE);
         Invalid:  
         		GlobalUnlock (hMem);
         		GSSiGlobUlFree (&hMem);  
         		break;
         	}
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}


BOOL FAR PASCAL SQL_BETWEENMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
	char	str[128];	

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
    
         cwCenter(hWndDlg, -1);
		 hFldAppendWnd=GetDlgItem (hWndDlg,IDC_BETWEEN1);			
			
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break; 
            
            case IDOK: 
            {
            	HANDLE	hMem;
            	LPSTR	lpstr, lpstr2;    
            	
            	hMem = GSSiGlobAlloc ( 168,GMEM_MOVEABLE,2048);
            	
            	lpstr = GlobalLock (hMem);  
            	lpstr2 = lpstr + 1024;
            	_fstrcpy (lpstr," BETWEEN ");
            	if (!GetDlgItemText (hWndDlg,IDC_BETWEEN1,lpstr2,256))
            	{   
			        GSSiMsgBox(hWndDlg, "Missing or invalid values", 0, MB_ICONEXCLAMATION,0);
              		goto Invalid;
            	}
            	if ((SQLCurFieldType == SQL_CHAR || SQLCurFieldType == SQL_VARCHAR || SQLCurFieldType == SQL_UNKCHAR) && *lpstr2 != '\'')
            	{
            		_fstrcat (lpstr,"'");
            		_fstrcat (lpstr,lpstr2);
            		_fstrcat (lpstr,"' AND ");
            	}
            	else 
            	{
            		_fstrcat (lpstr,lpstr2);
            		_fstrcat (lpstr," AND ");
            	}
            	if (!GetDlgItemText (hWndDlg,IDC_BETWEEN2,lpstr2,256))
            	{   
			        GSSiMsgBox(hWndDlg, "Missing or invalid values", 0, MB_ICONEXCLAMATION,0);
              		goto Invalid;
            	}
            	if ((SQLCurFieldType == SQL_CHAR || SQLCurFieldType == SQL_VARCHAR || SQLCurFieldType == SQL_UNKCHAR) && *lpstr2 != '\'')
            	{
            		_fstrcat (lpstr,"'");
            		_fstrcat (lpstr,lpstr2);
            		_fstrcat (lpstr,"'");
            	}
            	else 
            		_fstrcat (lpstr,lpstr2);
		 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)lpstr);
                EndDialog(hWndDlg, TRUE);
         Invalid:  
         		GlobalUnlock (hMem);
         		GSSiGlobUlFree (&hMem);  
         		break;
         	}
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL SQL_VALUEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
	char	str[128];	

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
    
         cwCenter(hWndDlg, -1);
		 hFldAppendWnd=GetDlgItem (hWndDlg,IDC_SQLVALUE);			
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break; 
            
            case IDOK: 
            {
            	HANDLE	hMem;
            	LPSTR	lpstr, lpstr2;    
            	
            	hMem = GSSiGlobAlloc ( 169,GMEM_MOVEABLE,2048);
            	
            	lpstr = GlobalLock (hMem);  
            	if (!GetDlgItemText (hWndDlg,IDC_SQLVALUE,lpstr,256))
            	{   
			        GSSiMsgBox(hWndDlg, "Missing or invalid values", 0, MB_ICONEXCLAMATION,0);
              		goto Invalid;
            	}     
		 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)SQLValOp);
            	if ((SQLCurFieldType == SQL_CHAR || SQLCurFieldType == SQL_VARCHAR || SQLCurFieldType == SQL_UNKCHAR) && *lpstr != '\'')
            	{
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)"'");
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)lpstr);
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)"'");
            	}
            	else 
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,(LPARAM)lpstr);
                EndDialog(hWndDlg, TRUE);
         Invalid:  
         		GSSiGlobUlFree (&hMem);  
         		break;
         	}
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL DBLOGINMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
	char	UserID[64], Password[32];
	static	HANDLE	hSaveBM=0;	

 switch(Message)
   {
    case WM_INITDIALOG:  
    
    	 hSaveBM = EnterBlockingWindow (hWndDlg);
		 SetDlgItemText(hWndDlg, IDC_DBNAME, CurODBCFile);
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
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);  
            break; 
            
            case IDOK: 
            	GetDlgItemText (hWndDlg,IDC_USERID,UserID,sizeof(UserID));
            	GetDlgItemText (hWndDlg,IDC_PASSWORD,Password,sizeof(Password)); 
            	SetODBCPassword (UserID,Password);
                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
            break;
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 
BOOL doesDSNExist (LPSTR dsnName,BOOL localMachine) 
{
	HKEY	hKeyhw,hKeyd,hKeys,hKeycp,hKeyz;
	HKEY	hKey = HKEY_CURRENT_USER;
	int		lValue;
	BOOL	rtn = FALSE;
	if (localMachine)
		hKey = HKEY_LOCAL_MACHINE;
	if (RegOpenKeyEx (hKey,"SOFTWARE",0,KEY_READ,&hKeyhw) == ERROR_SUCCESS)
	{
		if (RegOpenKeyEx (hKeyhw,"ODBC",0,KEY_READ,&hKeyd) == ERROR_SUCCESS)
		{
			if (RegOpenKeyEx (hKeyd,"ODBC.ini",0,KEY_READ,&hKeys) == ERROR_SUCCESS)
			{
				if (RegOpenKeyEx (hKeys,dsnName,0,KEY_READ,&hKeycp) == ERROR_SUCCESS)
				{
					RegCloseKey (hKeycp);
					rtn = TRUE;
				}
				RegCloseKey (hKeys);
			}
			RegCloseKey (hKeyd);
		}
		RegCloseKey (hKeyhw);
	}
	return rtn;
}

BOOL createDSN (LPSTR dsnName,BOOL localMachine,LPSTR type,LPSTR config)
{
	HKEY	hKeyhw,hKeyd,hKeys,hKeycp,hKeyz;
	HKEY	hKey = HKEY_CURRENT_USER;
	DWORD	Disp;
	int		lValue;
	BOOL	rtn = FALSE;
	char	message[256];
	int		rc=0;

	if (localMachine)
		hKey = HKEY_LOCAL_MACHINE;
	if (RegOpenKeyEx (hKey,"SOFTWARE",0,KEY_READ,&hKeyhw) == ERROR_SUCCESS)
	{
		if ((rc=RegCreateKeyEx (hKeyhw,"ODBC",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&hKeyd,&Disp)) == ERROR_SUCCESS)
		//if (RegOpenKeyEx (hKeyhw,"ODBC",0,KEY_READ,&hKeyd) == ERROR_SUCCESS)
		{
			if (RegCreateKeyEx (hKeyd,"ODBC.ini",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&hKeys,&Disp) == ERROR_SUCCESS)
			//if (RegOpenKeyEx (hKeyd,"ODBC.ini",0,KEY_READ,&hKeys) == ERROR_SUCCESS)
			{
				if (!(RegOpenKeyEx (hKeys,dsnName,0,KEY_READ,&hKeycp) == ERROR_SUCCESS))
				{
					LPSTR pName, pValue, pNext;

					if ((rc=RegCreateKeyEx (hKeys,dsnName,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&hKeycp,&Disp)) != ERROR_SUCCESS)
						goto Exit;
					pNext = config;
					while (pNext && *pNext)
					{
						pName = pNext;
						if ((pNext = strchr (pName,'\n')))
							*pNext++ = 0;
						pValue = strchr (pName,'\t');
						if (!pValue)
						{
							RegCloseKey (hKeycp);
							goto Exit;
						}
						*pValue++ = 0;
						if ((rc=RegSetValueEx (hKeycp,pName,0,REG_SZ,pValue,strlen(pValue)+1)) != ERROR_SUCCESS)
						{
							RegCloseKey (hKeycp);
							goto Exit;
						}
					}
					RegCloseKey (hKeycp);
					if (RegOpenKeyEx (hKeys,"ODBC Data Sources",0,KEY_WRITE,&hKeycp) != ERROR_SUCCESS)
					{
						if ((rc=RegCreateKeyEx (hKeys,"ODBC Data Sources",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&hKeycp,&Disp)) != ERROR_SUCCESS)
						{
							RegCloseKey (hKeycp);
							goto Exit;
						}
					}
					rc = RegSetValueEx (hKeycp,dsnName,0,REG_SZ,type,strlen(type)+1);
					RegCloseKey (hKeycp);
					if (rc == ERROR_SUCCESS)
						rtn = TRUE;
Exit:
					if (rc != ERROR_SUCCESS)
					{
						GetSystemErrMessage (rc,message);
						MessageBox (0,message,"",MB_ICONEXCLAMATION);
					}

				}
				RegCloseKey (hKeys);
			}
			RegCloseKey (hKeyd);
		}
		RegCloseKey (hKeyhw);
	}
	return rtn;
}



