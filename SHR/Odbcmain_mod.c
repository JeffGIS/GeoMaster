not used
#include <windows.h>
#include "dde.h"
#include <ddeml.h>
#include "client.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h> 
#include <direct.h> 
#include <errno.h>
#include <sql.h>
#include <sqlext.h> 
#include "shr.h" 
#include "gwd.h" 
#include "bt.h" 
#include "\gssi\prog\geomastr\resource.h"

#define	MAXPARAMLENGTH	255

typedef struct {char File[64],User[32],PW[32];} PWINFO;
typedef PWINFO	FAR	*LPPWINFO;
HANDLE	hODBCPW=0;  
BOOL	StandardSQL=TRUE;
extern	HWND	hWndMain, hInst;
extern	BOOL	DoPaint, HaltReport,ContinueProcessing;                
extern	AllVarEqQuestionMark; 

static	HWND	SQLWherehWnd;
static	HANDLE	SQLhDB;  
static	char	SQLString[1024];
static	int		SQLCurFieldType=-1;  
static	char	SQLValOp[16];
HENV henv=0;
char	CurODBCFile[64]="";
FIELDINFO mine;
HANDLE	hODBCParams=0;
#define STR_LEN 128+1 
#define REM_LEN 254+1
LPFIELDINFO lpmine; 
SDWORD cbTableQual, cbTableOwner, cbTableName, cbColumnName;
UCHAR  szQual[STR_LEN+1],       szTableName[STR_LEN+1],
       szTableOwner[STR_LEN+1], szTypeName[STR_LEN+1],  
       szRemarks[REM_LEN+1];
SDWORD cbRemarks, cbDataType, cbPrecision, cbLength,
       cbRadix, cbNullable, cbTypeName,cbScale;
          
#define MAX_STMT_LEN 100 
int FindType(char *szTypeName); 
UDWORD display_size(SWORD coltype, UDWORD collen, UCHAR *colname);
static short DBType[64];
static short DBNamePtr[64];
static HDBC HDBCS[64];
static char CurTables[1024][64];
static char TableNames[1024][64];
static char	OpenDBNames[128][64];
static HDBC OpenhDBs[64];
static short DBOpenCount[64]; 
static	int	NumOpenDBs=0;   

extern	HWND	hFldAppendWnd;

BOOL FAR PASCAL DBLOGINMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
BOOL FAR PASCAL SQL_LIKEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
BOOL FAR PASCAL SQL_INMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
BOOL FAR PASCAL SQL_VALUEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);

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
      case SQL_CHAR:
      case SQL_VARCHAR:
      case SQL_DATE:
         return(max(collen, _fstrlen(colname)));
      case SQL_SMALLINT:
         return(max(6,_fstrlen(colname)));
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

HANDLE CreateUniqueList (short length, LPSTR Name)
{

	BTVARDESC	BTVar[3];
	int	i, ifield;
	OFSTRUCT	OFStruct;
	HANDLE	hBT;
						
	if (*Name)
		GSSiOpenFile (Name,(LPOFSTRUCT)&OFStruct,OF_DELETE);
	else
		GetTempFileName (NULL,"unq",NULL,(LPSTR)Name);

	BTVar[0].BT_VARTYP=BT_CHAR;
	BTVar[0].BT_VARLEN=min(200,length);
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (Name, 2, FALSE, 1, 1, (LPBTVARDESC)&BTVar, FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (Name, 0, BT_WRITE, 0);

	return hBT;
}  

BOOL GetODBCUniqueFieldValues (HANDLE DBhandle, LPCSTR SQL,LPSTR name,short length, 
                               HANDLE hDBList)
{                             
char answer[256], Value[256];
int   i, ClassNo, ClassZero=0; 
RETCODE rc;
HDBC hdbc;
UDWORD collen;
SDWORD  lenanswer;
char sqlstr[256],str[256];
LPSTR	lpsqlstr=&sqlstr, lpBrack, lpEndBrack, lpParam;
HSTMT	hstmt; 
BOOL	rtn=FALSE;
HCURSOR hcurSave;
    
    hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
    i=DBhandle;
    hdbc = HDBCS[i]; 
    SQLAllocStmt(hdbc, &hstmt);
    sprintf (sqlstr," Select DISTINCT \"%s\" From %s",name,TableNames[i]);  
    if (SQL)
    	sprintf (_fstrchr(sqlstr,0)," Where (%s)",SQL);
    
    rc = SQLPrepare(hstmt, lpsqlstr, SQL_NTS);
    if(rc == SQL_SUCCESS)   
		rc = SQLExecDirect(hstmt, lpsqlstr, SQL_NTS);
    if(rc ==  SQL_ERROR)
    {    SDWORD	errno;
    	 SWORD lmes;
    	 char	cError[128], Mess[SQL_MAX_MESSAGE_LENGTH], DispStr[SQL_MAX_MESSAGE_LENGTH+300];
	    	 
		 lmes = SQL_MAX_MESSAGE_LENGTH-1; 
	     SQLError(SQL_NULL_HENV,SQL_NULL_HDBC,hstmt,cError,&errno,Mess,SQL_MAX_MESSAGE_LENGTH-2,&lmes);
	     sprintf (DispStr,"%s   %s",lpsqlstr,Mess);
	     DoPaint = FALSE;
	     if (MessageBox(hWndMain,DispStr,"Error in SQL Query",MB_OKCANCEL|MB_ICONQUESTION)
	     	 == IDCANCEL) HaltReport=TRUE;  
	     DoPaint = TRUE;
	     goto Exit;
    }  
	if(rc |= SQL_SUCCESS) goto Exit;
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
	 SQLFreeStmt(hstmt, SQL_DROP);
     GSSiSetCursor (hcurSave); 
     return rtn;
}

RETCODE FetchODBCRecord (LPOPENSQLDATA	SQLPtr)
{	
	RETCODE	rc;
	LPOPENFILEDATA	FilePtr;

	FilePtr = GlobalLock (SQLPtr->OFHandle); 
	ClearCurVals (FilePtr);
    rc = SQLFetch(SQLPtr->hstmt); 
    GlobalUnlock (SQLPtr->OFHandle);
    return rc;
}

SWORD NumSQLCols (LPOPENSQLDATA	SQLPtr)
{	
	RETCODE	rc;
	LPOPENFILEDATA	FilePtr;
	SWORD	Ncols;

	FilePtr = GlobalLock (SQLPtr->OFHandle); 
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
char sqlstr[256],str[256];
LPSTR	lpsqlstr=&sqlstr, lpBrack, lpEndBrack, lpParam, lpOrder, Value;
HSTMT	hstmt; 
LPOPENFILEDATA	FilePtr;
DWORD	rtn=0;
BTHEAD BTHead;
LPOPENSQLDATA	SQLPtr;   
HANDLE	hSTR=0;

    SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
	FilePtr = GlobalLock (SQLPtr->OFHandle); 
    
    if (FilePtr->Type == UMIFS_DATAFILE)
    {   
    	LPGWDHEADER	lpGWDHead;
    	
	    if (FilePtr->Type == UMIFS_DATAFILE)
	    {
	        lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);
         	GetBTHeader (lpGWDHead->BTHandle[0],&BTHead);  
         	GlobalUnlock (FilePtr->FileHandle);
         	GlobalUnlock (SQLPtr->OFHandle); 
         	GlobalUnlock (hSQL);
         	return (BTHead.BT_NUMRECS);
	    } 
	}
    else if (FilePtr->Type == COMBO_DATAFILE)
    {   
		LPCOMBOHEADER		pComboHeader;
	    LPCOMBOFILE  		pComboFile; 
    
		pComboHeader = GlobalLock (FilePtr->FileHandle); 
		pComboFile = GlobalLock (pComboHeader->hComboFile);
		rtn = NumSQLRows (pComboFile->hSQL[0]);
		GlobalUnlock (pComboHeader->hComboFile);
		GlobalUnlock (FilePtr->FileHandle); 
		return rtn;
    } 
    hdbc = HDBCS[FilePtr->FileHandle]; 
    SQLAllocStmt(hdbc, &hstmt);   
    hSTR = GSSiGlobAlloc (GMEM_MOVEABLE,4096);
    Value = GlobalLock (hSTR);
    _fstrcpy (Value,SQLPtr->SQL);
    if (*Value == '$')
	   	ExpandText (Value);
    lpOrder = _fstrstr (Value," ORDER BY");
    if (lpOrder)
    	*lpOrder = 0;
    if (*Value)
    	sprintf (sqlstr," Select COUNT('%s') From %s WHERE %s",FilePtr->FldInfo.name,TableNames[FilePtr->FileHandle],Value); 
    else
    	sprintf (sqlstr," Select COUNT('%s') From %s",FilePtr->FldInfo.name,TableNames[FilePtr->FileHandle]); 
    ExpandText (sqlstr);
    rc = SQLPrepare(hstmt, lpsqlstr, SQL_NTS);
    if(rc == SQL_SUCCESS)  
		rc = SQLExecDirect(hstmt, lpsqlstr, SQL_NTS);
    if(rc ==  SQL_ERROR)
    {    SDWORD	errno;
    	 SWORD lmes;
    	 char	cError[128], Mess[SQL_MAX_MESSAGE_LENGTH], DispStr[SQL_MAX_MESSAGE_LENGTH+300];
	    	 
		 lmes = SQL_MAX_MESSAGE_LENGTH-1; 
	     SQLError(SQL_NULL_HENV,SQL_NULL_HDBC,hstmt,cError,&errno,Mess,SQL_MAX_MESSAGE_LENGTH-2,&lmes);
	     sprintf (DispStr,"%s   %s",lpsqlstr,Mess);
	     DoPaint = FALSE;
	     if (MessageBox(hWndMain,DispStr,"Error in SQL Query",MB_OKCANCEL|MB_ICONQUESTION)
	     	 == IDCANCEL) HaltReport=TRUE; 
	     DoPaint = TRUE;
       	 GlobalUnlock (SQLPtr->OFHandle); 
       	 GlobalUnlock (hSQL);
 		 GSSiGlobUlFree (&hSTR);
 	     return FALSE;
    }  
	if(rc |= SQL_SUCCESS) goto Exit;
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
	 SQLFreeStmt(hstmt, SQL_DROP);
	 GlobalUnlock (SQLPtr->OFHandle); 
	 GlobalUnlock (hSQL);
 	 GSSiGlobUlFree (&hSTR);
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
			GlobalFree (lpField->hCurVal);
			lpField->hCurVal=0;
		}
	}
	return;
} 
BOOL ExternalSQLDirectBatch (HANDLE hDB, LPSTR SQL)
{ 
	static HANDLE	BatchHandle=0, CurBatchID=0;  
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
		BatchHandle = GSSiGlobAlloc (GMEM_MOVEABLE,4096);
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

BOOL ExternalSQLDirect (HANDLE hDB, LPSTR SQL)
{
	RETCODE rc;
	HDBC hdbc;
	HSTMT	hstmt;
	BOOL	rtn=TRUE; 
	LPSTR	DBFLoc, pSQL=NULL;
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
    	handle = GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX);
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
    {    SDWORD	errno;
    	 SWORD lmes; 
    	 LPSTR	cError, Mess, DispStr;
    	 HANDLE	hMem;
	    	 
//    	 hMem = GSSiGlobAlloc (GMEM_MOVEABLE,SQL_MAX_MESSAGE_LENGTH*2+300+10+128+32);     
    	 hMem = GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX);     
    	 cError = GlobalLock (hMem);
    	 Mess = cError + 130;
    	 DispStr = Mess + SQL_MAX_MESSAGE_LENGTH+10+4;
	    	 
		 lmes = SQL_MAX_MESSAGE_LENGTH-1; 
	     SQLError(SQL_NULL_HENV,SQL_NULL_HDBC,hstmt,cError,&errno,Mess,SQL_MAX_MESSAGE_LENGTH-2,&lmes);
	     sprintf (DispStr,"%s\r\n%s",SQL,Mess); 
	     DoPaint = FALSE;
	     if (MessageBox(hWndMain,DispStr,"Error in SQL Execution",MB_OKCANCEL|MB_ICONQUESTION)
	     	 == IDCANCEL) rtn=FALSE; 
	     DoPaint = TRUE;
	     GlobalUnlock (hMem);
	     GlobalFree (hMem);
    }  
	SQLFreeStmt(hstmt, SQL_DROP);
	if (pSQL)
		GlobalUnlock (handle); 
	_chdrive (SaveDrive);
 	_chdir (cwd); 
	GSSiGlobFree (&handle); 
	return rtn;
}

/******************************************************************/
LPVOID GetExternalFieldData ( LPOPENFILEDATA FilePtr, LPCSTR indexIN, LPVOID *hstmt,
                              LPFIELDINFO field, BOOL SingleVal, short FunctionID,LPINT irc,
                              int NumFields,LPFIELDINFO FirstField)
                                 
{  // index[] = "pidno", keydata[] = "0102824110013" ;                            
static char answer[4096];
#define MAX_COLS 5
LPVOID lpvoid = answer;
int   i,l, startfield, WantField; 
double danswer;
short shorti;
RETCODE rc;
HDBC hdbc;
UDWORD collen;
SDWORD  lenanswer;
SWORD nresultcols, scale, coltype, colnamelen, nullable;
UCHAR colname[32];          
long  long_i;
//static char szItem[] = "SQL";
HANDLE	hSTR;
LPSTR	sqlstr,str,index;  
char	FunctionName[5][6]={"COUNT","AVG","SUM","MIN","MAX"};
LPSTR	lpBrack, lpEndBrack, lpParam;
int		nparam, FldNum;
BOOL	ExSQL=TRUE;
HANDLE DBhandle; 
LPCURVAL	pCurVal; 

    *irc = 0;
	if (field->hCurVal)
	{ 
		pCurVal = GlobalLock (field->hCurVal);
		_fstrncpy (answer,&pCurVal->Value,pCurVal->length);
		answer[pCurVal->length]=0;   
		GlobalUnlock (field->hCurVal);
	    return &answer;
	}
	DBhandle = FilePtr->FileHandle;  
	  
    hSTR = GSSiGlobAlloc (GHND,4096+1024+4096);
    str = GlobalLock (hSTR);
    sqlstr = str + 4096;
    index = sqlstr + 1024;
    _fstrcpy (index,indexIN);
    if (*index == '$')
       	ExpandText (index);
	if (*hstmt && !SingleVal)
		ExSQL = FALSE;

   // for(i = 0; i < 128; szValue[i] = ' ',i++);
    i=DBhandle;
    hdbc = HDBCS[i]; 
    if (!hODBCParams)
    	hODBCParams = GSSiGlobAlloc (GPTR,4096);
    if (!*hstmt)
    {
	    SQLAllocStmt(hdbc, hstmt); 
	    if (*TableNames[i])
	    {
		    _fstrcpy(sqlstr, "Select *");
		    if (SingleVal)
		    {   
		    	if (FunctionID)
		    		sprintf (sqlstr,"Select %s(\"%s\")",FunctionName[FunctionID-1],field->name);
		    	else if (StandardSQL || _fstrchr (field->name,' '))
		    		sprintf (sqlstr,"Select \"%s\"",field->name);   
		    	else
		    		sprintf (sqlstr,"Select %s",field->name);
		    }
		    else
		    	_fstrcpy(sqlstr, "Select *");
		    _fstrcat(sqlstr," From ");
		    _fstrcat(sqlstr, TableNames[i]);
	    	if (!_fstrnicmp (index," ORDER BY",9))
		    	_fstrcat(sqlstr,index);
		    else if (index[0])
		    {
		    	_fstrcat(sqlstr, " Where ");
		    	_fstrcpy(str, index);
			    AllVarEqQuestionMark=TRUE;
			    ExpandText (str);
			    AllVarEqQuestionMark=FALSE;   
			    REPLAC (str,"'?'","?",sizeof(str));
			    _fstrcat(sqlstr,str);
		    } 
	    }
	    else
	    	 _fstrcpy(sqlstr,index);
	   // outlen = sizeof(sqlstr);
	    rc = SQLPrepare(*hstmt, sqlstr, SQL_NTS);
	    if(rc ==  SQL_ERROR)
	    	goto ErrMes;
	    if(rc |= SQL_SUCCESS) goto s44;  
	    nparam = 0;   
	    lpParam = GlobalLock (hODBCParams);
		lpBrack = index;
		while ((lpBrack = _fstrchr (lpBrack,'[')))
		{
			if (lpEndBrack = _fstrchr (lpBrack,']'))
			{   
				++nparam; 
				
				rc = SQLSetParam(*hstmt, nparam, SQL_C_CHAR,
								 GetSQLType(index,lpBrack,FilePtr->NumFields,&FilePtr->FldInfo),
								 MAXPARAMLENGTH, 0,lpParam, NULL);
				lpParam += MAXPARAMLENGTH;   
				lpBrack = ++lpEndBrack;
			}
			else
				lpBrack++;
			
		} 
		GlobalUnlock (hODBCParams);
	  /*  SQLBindCol(*hstmt, 1, SQL_C_CHAR, answer, 254, &lenanswer);*/
	}
	if (ExSQL)
	{   
		HANDLE	hTemp=GSSiGlobAlloc (GMEM_MOVEABLE,4096); 
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
	    /*rc = SQLExecDirect(hstmt, lpsqlstr, SQL_NTS);*/
	    rc = SQLExecute(*hstmt);
ErrMes: 
	    if(rc ==  SQL_ERROR)
	    {    SDWORD	errno;
	    	 SWORD lmes; 
	    	 LPSTR	cError, Mess, DispStr;
	    	 HANDLE	hMem;
	    	 
	    	 hMem = GSSiGlobAlloc (GMEM_MOVEABLE,SQL_MAX_MESSAGE_LENGTH*2+300+10+128+32);     
	    	 cError = GlobalLock (hMem);
	    	 Mess = cError + 130;
	    	 DispStr = Mess + SQL_MAX_MESSAGE_LENGTH+10+4;
	    	 
			 lmes = SQL_MAX_MESSAGE_LENGTH-1; 
		     SQLError(SQL_NULL_HENV,SQL_NULL_HDBC,*hstmt,cError,&errno,Mess,SQL_MAX_MESSAGE_LENGTH-2,&lmes);
		     sprintf (DispStr,"%s   %s",sqlstr,Mess); 
		     DoPaint = FALSE;
		     if (MessageBox(hWndMain,DispStr,"Error in SQL Query",MB_OKCANCEL|MB_ICONQUESTION)
		     	 == IDCANCEL) HaltReport=TRUE; 
		     DoPaint = TRUE;
		     GlobalUnlock (hMem);
		     GlobalFree (hMem);
		     goto ErrExit;
	    }  
	    ClearCurVals (FilePtr);
    	if(rc |= SQL_SUCCESS) goto s44;
	    rc = SQLFetch(*hstmt);
	    if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)goto s44;
    }
/*    SQLNumResultCols(*hstmt, &nresultcols);
    SQLDescribeCol(*hstmt,1,colname, (SWORD) sizeof(colname),
                     &colnamelen, &coltype, &collen, &scale,
                     &nullable); */
   // outlen = display_size(coltype, collen, colname);
    //SQLGetData(hstmt,1, SQL_C_CHAR, answer, outlen, &lenanswer);
/*	hstmt = NULL;*/
    if (SingleVal) 
    {
    	startfield = 1;
    	NumFields = 1;
    	WantField = 1;
    }
    else  
    {
	    WantField = field->index+1;  
	    field = FirstField;
    	startfield = 1; 
    }
    for (FldNum = startfield;FldNum<=NumFields;FldNum++,field++)
    {
	    rc = SQLGetData(*hstmt, FldNum, SQL_C_CHAR, str, 4095, &lenanswer);
	    if(rc ==  SQL_ERROR) 
	    	goto ErrMes;
	    if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)goto s44;  
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
			field->hCurVal = GSSiGlobAlloc (GMEM_MOVEABLE,sizeof(int)+l);
			pCurVal = GlobalLock (field->hCurVal); 
			pCurVal->length=l;   
			if (pCurVal->length)
				_fstrncpy (&pCurVal->Value,str,pCurVal->length);
			GlobalUnlock (field->hCurVal);  
		} 
		if (FldNum == WantField)
		{
			_fstrncpy (answer,str,l);
			answer[l]=0;
		}
	}
	if (SingleVal) 
	{
		SQLFreeStmt(*hstmt, SQL_CLOSE);
	} 
	GSSiGlobUlFree (&hSTR);
    return &answer;
    
s44: if (*hstmt && SingleVal)
	 {
	 	SQLFreeStmt(*hstmt, SQL_CLOSE);
		/*SQLFreeStmt(*hstmt,SQL_UNBIND);*/
	 } 
ErrExit:	
     *irc = 1;
	GSSiGlobUlFree (&hSTR);
    return NULL;

}   

int UpdateExternalFieldData ( LPOPENFILEDATA FilePtr, LPCSTR index,LPSTR UpdateString)
{                                 
HSTMT	hstmt;
int   i,l; 
short shorti;
RETCODE rc;
HDBC hdbc;
UDWORD collen;
SWORD nresultcols, scale, coltype, colnamelen, nullable;
UCHAR colname[32];          
long  long_i;
//static char szItem[] = "SQL";
char str[260];
LPSTR	lpsqlstr, lpBrack, lpEndBrack, lpParam;
int		nparam, FldNum;
BOOL	ExSQL=TRUE;
HANDLE DBhandle, hstr; 
LPCURVAL	pCurVal;

    hstr = GSSiGlobAlloc (GMEM_MOVEABLE,4096);
    lpsqlstr = GlobalLock (hstr);
    *lpsqlstr = 0;                                       
	DBhandle = FilePtr->FileHandle;    
    
    i=DBhandle;
    hdbc = HDBCS[i]; 
    if (!hODBCParams)
    	hODBCParams = GlobalAlloc (GHND,4096);  
    	
    SQLAllocStmt(hdbc, &hstmt); 
    sprintf (lpsqlstr,"UPDATE %s SET %s WHERE ",TableNames[i],UpdateString);
    _fstrcpy(str, index);
    AllVarEqQuestionMark=TRUE;
    ExpandText (str);
    AllVarEqQuestionMark=FALSE;
    _fstrcat(lpsqlstr,str);
    rc = SQLPrepare(hstmt, lpsqlstr, SQL_NTS);
    if(rc |= SQL_SUCCESS) goto s44;  
    nparam = 0;   
    lpParam = GlobalLock (hODBCParams);
	lpBrack = index;
	while ((lpBrack = _fstrchr (lpBrack,'[')))
	{
		if (lpEndBrack = _fstrchr (lpBrack,']'))
		{   
			++nparam; 
				
			rc = SQLSetParam(hstmt, nparam, SQL_C_CHAR,
							 GetSQLType(index,lpBrack,FilePtr->NumFields,&FilePtr->FldInfo),
							 MAXPARAMLENGTH, 0,lpParam, NULL);
			lpParam += MAXPARAMLENGTH;   
			lpBrack = ++lpEndBrack;
		}
		else
			lpBrack++;
			
	} 
	GlobalUnlock (hODBCParams);

	{   
		HANDLE	hTemp=GSSiGlobAlloc (GMEM_MOVEABLE,4096); 
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

    /*rc = SQLExecDirect(hstmt, lpsqlstr, SQL_NTS);*/
    rc = SQLExecute(hstmt);
ErrMes: 
    if(rc ==  SQL_ERROR)
    {    SDWORD	errno;
    	 SWORD lmes; 
    	 LPSTR	cError, Mess, DispStr;
    	 HANDLE	hMem;
	    	 
    	 hMem = GSSiGlobAlloc (GMEM_MOVEABLE,SQL_MAX_MESSAGE_LENGTH*2+300+10+128+32);     
    	 cError = GlobalLock (hMem);
    	 Mess = cError + 130;
    	 DispStr = Mess + SQL_MAX_MESSAGE_LENGTH+10+4;
	    	 
		 lmes = SQL_MAX_MESSAGE_LENGTH-1; 
	     SQLError(SQL_NULL_HENV,SQL_NULL_HDBC,hstmt,cError,&errno,Mess,SQL_MAX_MESSAGE_LENGTH-2,&lmes);
	     sprintf (DispStr,"%s   %s",lpsqlstr,Mess); 
	     DoPaint = FALSE;
	     if (MessageBox(hWndMain,DispStr,"Error in SQL Query",MB_OKCANCEL|MB_ICONQUESTION)
	     	 == IDCANCEL) HaltReport=TRUE;  
	     DoPaint = TRUE;
	     GlobalUnlock (hMem);
	     GlobalFree (hMem);
	     goto s44;
    }  
    ClearCurVals (FilePtr);
	if(rc |= SQL_SUCCESS) goto s44;
	SQLFreeStmt(hstmt, SQL_DROP); 
	GlobalUnlock (hstr);
	GlobalFree (hstr);
    return TRUE;
    
    
s44: if (&hstmt)
	 	SQLFreeStmt(&hstmt, SQL_CLOSE);
	if (HaltReport)
		ContinueProcessing = FALSE;
	GlobalUnlock (hstr);
	GlobalFree (hstr);
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

    hstr = GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX);
    lpsqlstr = GlobalLock (hstr);
    *lpsqlstr = 0;                                       
	i = FilePtr->FileHandle;    
    
    sprintf (lpsqlstr,"INSERT INTO %s %s",TableNames[i],InsertString);
    ExpandText (lpsqlstr);
    ClearCurVals (FilePtr);
	rtn = ExternalSQLDirect (i,lpsqlstr);
	GlobalUnlock (hstr);
	GlobalFree (hstr);
    return rtn;
}   

SWORD GetSQLType(LPSTR pstr,LPSTR lpBrack,int NumFields,LPFIELDINFO lpFldInfo)
{   
	char	Name[64];  
	LPSTR	lpEq;
	
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
			return (lpFldInfo->type);
		else
			lpFldInfo++;
	}
	return SQL_CHAR;
}

/******************************************************************/
LPFIELDINFO GetExternalFieldInfo( HANDLE DBhandle, BOOL first)
{
RETCODE rc;       
SWORD Nullable;
HSTMT hstmt;
short	i, drivenum;    
char	TableName[128], str[128], drive[4], dir[128], file[64], ext[8];
static	short	SaveDrive;
static  char	cwd[128];   
LPSTR	pOwner, pTable;   
short	lOwner;


	lpmine = &mine;

  i=DBhandle;  
  if (!*TableNames[i])
  	return NULL;
  if(first)
  {
	SaveDrive = _getdrive();
	_getcwd (cwd,128); 
    mine.precision = 0;
    mine.scale = 0;
    mine.radix = 0;
    mine.length = 0; 
    SQLAllocStmt(HDBCS[i], &hstmt); 
    if (_fstrstr (OpenDBNames[DBNamePtr[i]],"DBF|"))
    {
    	_fullpath (str,TableNames[i],128);
    	_splitpath (str,drive,dir,TableName,ext); 
    	sprintf (str,"%s%s",drive,dir); 
    	drivenum = GetDriveNum(*drive);
		_chdrive (drivenum);
	 	_chdir (str); 
		_getcwd (str,128);    
		pOwner = 0;
		lOwner = 0;
    }
    else
    {  
    	_fstrcpy (TableName,TableNames[i]);
    	if (!_fstrstr(TableName,".TXT") && !_fstrstr(TableName,".DBF") && (pTable = _fstrchr (TableName,'.')))
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
    rc = SQLColumns(hstmt, NULL, 0, pOwner,lOwner, pTable, SQL_NTS, NULL, 0);
    if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)  goto s44;
    SQLBindCol(hstmt,  1, SQL_C_CHAR,   szQual, STR_LEN, &cbTableQual); 
    SQLBindCol(hstmt,  2, SQL_C_CHAR,   szTableOwner, STR_LEN, &cbTableOwner); 
    SQLBindCol(hstmt,  3, SQL_C_CHAR,   szTableName, STR_LEN, &cbTableName); 
    SQLBindCol(hstmt,  4, SQL_C_CHAR,   mine.name, 64, &cbColumnName); 
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
    rc = SQLFetch(hstmt);
    while(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {  
    	switch (mine.type)
    	{
       		lpmine->type = FindType(szTypeName); 
       		break;
       		
       		case SQL_DATE:
			case SQL_TIME:
			case SQL_TIMESTAMP:
				lpmine->type = SQL_VARCHAR;
				lpmine->length = 20;
			break;

       		default:
       		break;
       	}
       return lpmine;
    
    }
s44: SQLFreeStmt(hstmt, SQL_DROP);
	_chdrive (SaveDrive);
 	_chdir (cwd); 
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
	LPINT	pnumPW; 
	LPSTR	pSC = _fstrchr (Password,';'); 
		
	if (!hODBCPW)  
	{
		hODBCPW=GSSiGlobAlloc (GHND,2+64*sizeof(PWINFO));    
	}
	pnumPW = GlobalLock (hODBCPW);
	pnumPW++;
	pPWInfo = pnumPW;
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
	LPINT	pnumPW; 
	BOOL	rtn=FALSE;
	char	str[128];
    
    if (!hODBCPW)
    {
		if (GetGlobalCVal ("[%ODBCPASSWORDFILE]",str,NULL))
		{   
			char		SaveCurODBCFile[64];
			OFSTRUCT	OFStruct;
			HFILE		Fid=GSSiOpenFile (str,&OFStruct,OF_READ);
			LPSTR		pSpace; 
			
			if (Fid == HFILE_ERROR)
				return FALSE;   
			_fstrcpy (SaveCurODBCFile,CurODBCFile);

			while (fgetstring (str,120,Fid))
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
	pnumPW = GlobalLock (hODBCPW);
	pnumPW++;
	pPWInfo = pnumPW;
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

/******************************************************************/
HANDLE OpenExternalDatabase( LPSTR name)
{
static BOOL first_entry = TRUE;
int  i, next_one, j;  
BOOL	SaveDoPaint, DoPrompt=FALSE;
HDBC hdbc;
RETCODE rc; 
HANDLE	htnames=GSSiGlobAlloc (GHND,2048+256+256+256+256);
SDWORD nerr;
SWORD mlen = 253, Moutlen, maxoutlen = 255, outlen;
LPSTR cptr;
char  uid[]="admin", pwd[]=""; 
LPSTR	lptnames, lpcstring, lpUID;
UWORD fDirection = SQL_FETCH_FIRST; //SQL_FETCH_NEXT
short	SaveDrive;
LPSTR tnames = GlobalLock (htnames);
LPSTR errmess = tnames + (1024+512);  
LPSTR DBAndTable = tnames + 2048;
LPSTR cstring = tnames + (2048+256);    
LPSTR cwd = tnames + (2048+256+256);
LPSTR File = tnames + (2048+256+256+256);


	SaveDrive = _getdrive();
	_getcwd (cwd,128); 
   if(first_entry)
   {
     for (i = 0; i < 64; DBType[i] = -1, i++);
     first_entry = FALSE;
   }
   for(i = 1; i < 64; i++)if(DBType[i] == -1)break;
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
   		   DBType[next_one] = DBType[i];
		   DBType[next_one] = 'O';
		   HDBCS[next_one] = OpenhDBs[i]; 
		   DBOpenCount[i]++;
		   DBNamePtr[next_one] = i;
			_chdrive (SaveDrive);
		 	_chdir (cwd);
		   GSSiGlobUlFree (&htnames); 
		   return (HANDLE) next_one;
   		}
   } 
   _fstrcpy (DBAndTable,tnames);  
   cptr = _fstrstr (tnames,"(CHAN");
   if(cptr) *cptr = 0;
   _fstrcpy (CurODBCFile,tnames);   
   
   _fstrcpy(cstring,"DSN=");
   _fstrcat(cstring,tnames);                                        
   if (!henv)
   		SQLAllocEnv(&henv);
   SQLAllocConnect(henv, &hdbc);
   tnames[0] = '\0';
   lptnames = (LPSTR)tnames;
   lpcstring = (LPSTR)cstring; 
   SaveDoPaint = DoPaint;
TryAgain:
   DoPaint = FALSE;  
   lpUID = _fstrstr (lpcstring,";UID=");
   if (lpUID) *lpUID=0;
   if (!AddPWtoODBCFile (lpcstring) && lpUID)
   		*lpUID = ';';
   
   if (DoPrompt)
	   rc = SQLDriverConnect(hdbc, hWndMain, lpcstring, SQL_NTS, lptnames, maxoutlen, &Moutlen,SQL_DRIVER_PROMPT);
   else
	   rc = SQLDriverConnect(hdbc, hWndMain, lpcstring, SQL_NTS, lptnames, maxoutlen, &Moutlen,SQL_DRIVER_COMPLETE_REQUIRED);

   DoPaint = SaveDoPaint;
   if(rc ==  SQL_ERROR)
   {
     SQLError(henv,
     hdbc,
     SQL_NULL_HSTMT, 
     tnames, 
     &nerr, 
     errmess,
     mlen, 
     &outlen); 
     if (!_fstricmp (tnames,"28000") || !_fstricmp (tnames,"IM008"))
     {
		FARPROC lpfnDBLOGINMsgProc;
		int	nRc;
			
		lpfnDBLOGINMsgProc = MakeProcInstance((FARPROC)DBLOGINMsgProc, hInst);
		nRc = DialogBox(hInst, (LPSTR)"DBLOGIN", hWndMain, lpfnDBLOGINMsgProc);
		FreeProcInstance(lpfnDBLOGINMsgProc);
		if (nRc)
	     	goto TryAgain; 
	    goto s44;
     }
     MessageBox(NULL,errmess,lpcstring,MB_OK|MB_ICONQUESTION|MB_TASKMODAL);
   }  

   if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)  goto s44;
   SetODBCPassword2 (lptnames);                                                        
   DBType[next_one] = 'O';
   HDBCS[next_one] = hdbc;
   for (i=0;i<NumOpenDBs;i++)
   {
   		if (!OpenhDBs[i])
   			goto s43;
   }
   i=NumOpenDBs++;
s43:
   _fstrcpy (OpenDBNames[i],DBAndTable);   
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
int NumDatabaseTables( char *type, HWND  DBhandle)
{
#define STR_RMK 254
SDWORD cbTableQual, cbTableOwner, cbTableName, cbTableType, cbRemarks;
UCHAR  szTableQual[STR_LEN+1],   szTableName[STR_LEN+1],
       szTableOwner[STR_LEN+1],  szTableType[STR_LEN+1],
       szRemarks[STR_RMK];
static HSTMT hstmt;
HDBC hdbc;           
RETCODE rc;
int icount; 
short	i;
    i=DBhandle;
    hdbc = HDBCS[i];
    SQLAllocStmt(hdbc, &hstmt);
    rc = SQLTables(hstmt,NULL,0,NULL,0,NULL,0,NULL,0);
    if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)  goto s44;
    SQLBindCol(hstmt,  1, SQL_C_CHAR,   szTableQual, STR_LEN, &cbTableQual); 
    SQLBindCol(hstmt,  2, SQL_C_CHAR,   szTableOwner, STR_LEN, &cbTableOwner); 
    SQLBindCol(hstmt,  3, SQL_C_CHAR,   szTableName, STR_LEN, &cbTableName); 
    SQLBindCol(hstmt,  4, SQL_C_CHAR,   szTableType, STR_LEN, &cbTableType); 
    SQLBindCol(hstmt,  5, SQL_C_CHAR,   szRemarks, STR_RMK, &cbRemarks);
    icount = 1;
    rc = SQLFetch(hstmt);
    while (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    {
     if(cbTableName > 254) cbTableName = 253;
     _fmemcpy(&CurTables[icount][0], szTableName, (size_t) cbTableName);
     CurTables[icount][cbTableName] = '\0';
     if (cbTableOwner > 0)
     	sprintf (CurTables[icount],"%s.%s",szTableOwner,szTableName);
     rc = SQLFetch(hstmt);
     icount++;
    }
s44:  SQLFreeStmt(hstmt, SQL_DROP);
 return icount-1;
}

/******************************************************************/
LPSTR GetTableName (HANDLE DBhandle, BOOL first)
{
static int icount, num_of_em;
    if(first)
    {
      num_of_em = NumDatabaseTables( "TABLE", DBhandle );
      icount = 1;
    }  
    else
    {
      icount++;  
      if(icount > num_of_em) return NULL;
    }  
return &CurTables[icount][0];
}


/******************************************************************/
HANDLE OpenDatabaseTable (HANDLE DBHandle, LPSTR TableName)
{ 

  _fstrcpy(&TableNames[DBHandle][0], TableName);
  return DBHandle;
  
} 

/*******************************************************************/ 
RETCODE GetDataSource(HANDLE CThemeDB, BOOL first, LPSTR names,
           int maxnamelen, int *namelen, LPSTR desc, int maxdesclen,
           int *desclen)
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
    if(rc |= SQL_SUCCESS) goto s44;
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
void CloseExternalDatabase (HANDLE ClientDDE)
{     
	 short	i;
	 
      if(ClientDDE == 0)return;
      DBType[ClientDDE] = -1;
      *TableNames[ClientDDE] = 0; 
      i = DBNamePtr[ClientDDE]; 
      DBOpenCount[i]--;
      if (DBOpenCount[i])
      	return;
      if (!GetGlobalBVal ("[%CLOSEODBC]"))
      	return;  
	  if (OpenhDBs[i])
	  {
		SQLDisconnect(OpenhDBs[i]);
		SQLFreeConnect(OpenhDBs[i]);
	  }
      OpenhDBs[i] = 0; 
      if (i == NumOpenDBs)
      	NumOpenDBs--;
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
	FARPROC lpfnSQLWHEREMsgProc; 
	BOOL	nRc; 
	
	if (!hDB)
		return FALSE;
	SQLhDB = hDB;  
	_fstrcpy (SQLString,Where);
	lpfnSQLWHEREMsgProc = MakeProcInstance((FARPROC)SQLWHEREMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"SQLWHERE", hWnd, lpfnSQLWHEREMsgProc);
	FreeProcInstance(lpfnSQLWHEREMsgProc); 
	if (nRc)
		_fstrcpy (Where,SQLString);    
	return nRc;
}
                             
BOOL FAR PASCAL SQLWHEREMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
	char	str[128], cUID[32];	
	int		Choice, ibeg; 
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
		SQLPtr = GlobalLock (SQLhDB);
		FilePtr = GlobalLock (SQLPtr->OFHandle);
		lpFieldInfo = &FilePtr->FldInfo; 
		for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++) 
		{
  			sprintf (str,"%s\t%i",lpFieldInfo->name,lpFieldInfo->type);
		 	SendDlgItemMessage (hWndDlg,IDC_FIELDLIST,LB_ADDSTRING,NULL,(LPARAM)str);
		}			
		GlobalUnlock (SQLPtr->OFHandle); 
		GlobalUnlock (SQLhDB);
		
		for (i=0;i<NUMOPS;i++)
		 	SendDlgItemMessage (hWndDlg,IDC_OPLIST,LB_ADDSTRING,NULL,(LPARAM)OPS[i]);
		SendDlgItemMessage (hWndDlg,IDC_GLOBALLIST,LB_ADDSTRING,NULL,(LPARAM)"[%UDI]");
			
        break; /* End of WM_INITDIALOG                                 */
    }
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
         {  
            case IDC_OPLIST: 
	            switch(HIWORD(lParam))
	            {
	                 case LBN_SELCHANGE:
	                 case LBN_DBLCLK:
		                 Choice=SendDlgItemMessage(hWndDlg,IDC_OPLIST,LB_GETCURSEL,NULL,NULL); 
					 switch (Choice)
					 {  
					 	default: 
					 	{ 
							FARPROC lpfnMsgProc; 
							BOOL	nRc;    
							
					    	GetWindowRect (GetDlgItem(hWndDlg,IDC_GLOBALLIST),&Rect); 
					        DisplayFieldList (hWndDlg,0,&Rect,1);
							_fstrcpy (SQLValOp,OPCODE[Choice]);
							lpfnMsgProc = MakeProcInstance((FARPROC)SQL_VALUEMsgProc, hInst);
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
					 		SendDlgItemMessage (hWndDlg,IDC_SQL,EM_REPLACESEL,0,OPCODE[Choice]);
					 	break;  
					 	case 10:
					 	case 11:
					 	case 12:
					 	{
							FARPROC lpfnMsgProc; 
							BOOL	nRc;    
							int		i=Choice-10;
							char	DiaName[3][16]={"SQL_BETWEEN","SQL_LIKE","SQL_IN"};
							FARPROC	MsgProc[3]={SQL_BETWEENMsgProc,SQL_LIKEMsgProc,SQL_INMsgProc};
							
					    	GetWindowRect (GetDlgItem(hWndDlg,IDC_GLOBALLIST),&Rect); 
					        DisplayFieldList (hWndDlg,0,&Rect,1);
							lpfnMsgProc = MakeProcInstance((FARPROC)MsgProc[i], hInst);
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
	            switch(HIWORD(lParam))
	            {
	                 case LBN_SELCHANGE:
	                 case LBN_DBLCLK:
	                 {
	                 	char	str2[40];
	                 	
		                 Choice=SendDlgItemMessage(hWndDlg,wParam,LB_GETCURSEL,NULL,NULL); 
				         SendDlgItemMessage(hWndDlg,wParam,LB_GETTEXT,Choice,(DWORD)str);
				         if ((lpTAB = _fstrchr (str,'\t')))
				         {
				         	*lpTAB++=0;
				         	SQLCurFieldType = atoi (lpTAB);
				         }
				         if (_fstrchr (str,' '))
				         	sprintf (str2,"\"%s\"",str);
				         else
				         	_fstrcpy (str2,str);
					 	 SendDlgItemMessage (hWndDlg,IDC_SQL,EM_REPLACESEL,0,str2); 
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

BOOL FAR PASCAL SQL_LIKEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
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
         switch(wParam)
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);
                break;
                
            case IDOK: 
            {
            	HANDLE	hMem;
            	LPSTR	lpstr, lpstr2;    
            	
            	hMem = GSSiGlobAlloc (GMEM_MOVEABLE,2048);
            	
            	lpstr = GlobalLock (hMem);  
            	if (!GetDlgItemText (hWndDlg,IDC_LIKE,lpstr,256))
            	{   
			        MessageBox(hWndDlg, "Missing or invalid values", NULL, MB_ICONEXCLAMATION);
              		goto Invalid;
            	}
		 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0," LIKE ");
            	if ((SQLCurFieldType == SQL_CHAR || SQLCurFieldType == SQL_VARCHAR) && *lpstr != '\'')
            	{
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,"'");
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,lpstr);
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,"'");
            	}
            	else 
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,lpstr);
                EndDialog(hWndDlg, TRUE);
         Invalid:  
         		GlobalUnlock (hMem);
         		GlobalFree (hMem);  
         		break;
         	}
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL SQL_INMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
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
         switch(wParam)
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break;
            case IDOK: 
            {
            	HANDLE	hMem;
            	LPSTR	lpstr, lpEnd, lpBeg, lpCom;  
            	BOOL	more,first;  
            	
            	hMem = GSSiGlobAlloc (GMEM_MOVEABLE,1024);
            	
            	lpstr = GlobalLock (hMem);  
            	if (!GetDlgItemText (hWndDlg,IDC_IN,lpstr,1024))
            	{   
			        MessageBox(hWndDlg, "Missing or invalid values", NULL, MB_ICONEXCLAMATION);
              		goto Invalid;
            	}
            	if (*lpstr == '(')
            		lpBeg = lpstr+1; 
				else
					lpBeg = lpstr;
		 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0," IN(");
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
			 			SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,",");
            		first = FALSE;
            		lpCom = _fstrchr (lpBeg,','); 
            		if (!lpCom)
            		{
            			more = FALSE;
            			lpCom = lpEnd;
            		} 
            		*lpCom = 0;
            		if (*lpBeg != '\'' &&
            			 (SQLCurFieldType == SQL_CHAR || SQLCurFieldType == SQL_VARCHAR))
				 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,"'");
				 	SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,lpBeg);
				 	lpBeg = lpCom-- + 1;
            		if (*lpCom != '\'' &&
            			 (SQLCurFieldType == SQL_CHAR || SQLCurFieldType == SQL_VARCHAR))
				 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,"'");
				} 	
				SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,")");
                EndDialog(hWndDlg, TRUE);
         Invalid:  
         		GlobalUnlock (hMem);
         		GlobalFree (hMem);  
         		break;
         	}
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}


BOOL FAR PASCAL SQL_BETWEENMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
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
         switch(wParam)
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break; 
            
            case IDOK: 
            {
            	HANDLE	hMem;
            	LPSTR	lpstr, lpstr2;    
            	
            	hMem = GSSiGlobAlloc (GMEM_MOVEABLE,2048);
            	
            	lpstr = GlobalLock (hMem);  
            	lpstr2 = lpstr + 1024;
            	_fstrcpy (lpstr," BETWEEN ");
            	if (!GetDlgItemText (hWndDlg,IDC_BETWEEN1,lpstr2,256))
            	{   
			        MessageBox(hWndDlg, "Missing or invalid values", NULL, MB_ICONEXCLAMATION);
              		goto Invalid;
            	}
            	if ((SQLCurFieldType == SQL_CHAR || SQLCurFieldType == SQL_VARCHAR) && *lpstr2 != '\'')
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
			        MessageBox(hWndDlg, "Missing or invalid values", NULL, MB_ICONEXCLAMATION);
              		goto Invalid;
            	}
            	if ((SQLCurFieldType == SQL_CHAR || SQLCurFieldType == SQL_VARCHAR) && *lpstr2 != '\'')
            	{
            		_fstrcat (lpstr,"'");
            		_fstrcat (lpstr,lpstr2);
            		_fstrcat (lpstr,"'");
            	}
            	else 
            		_fstrcat (lpstr,lpstr2);
		 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,lpstr);
                EndDialog(hWndDlg, TRUE);
         Invalid:  
         		GlobalUnlock (hMem);
         		GlobalFree (hMem);  
         		break;
         	}
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL SQL_VALUEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
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
         switch(wParam)
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break; 
            
            case IDOK: 
            {
            	HANDLE	hMem;
            	LPSTR	lpstr, lpstr2;    
            	
            	hMem = GSSiGlobAlloc (GMEM_MOVEABLE,2048);
            	
            	lpstr = GlobalLock (hMem);  
            	if (!GetDlgItemText (hWndDlg,IDC_SQLVALUE,lpstr,256))
            	{   
			        MessageBox(hWndDlg, "Missing or invalid values", NULL, MB_ICONEXCLAMATION);
              		goto Invalid;
            	}
		 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,SQLValOp);
            	if ((SQLCurFieldType == SQL_CHAR || SQLCurFieldType == SQL_VARCHAR) && *lpstr != '\'')
            	{
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,"'");
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,lpstr);
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,"'");
            	}
            	else 
			 		SendDlgItemMessage (SQLWherehWnd,IDC_SQL,EM_REPLACESEL,0,lpstr);
                EndDialog(hWndDlg, TRUE);
         Invalid:  
         		GlobalUnlock (hMem);
         		GlobalFree (hMem);  
         		break;
         	}
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

short DupDLLs (void)
{ 	
	struct	_find_t	FileInfo; 
	char	str[144], Name[144], ToDir[144], ToName[144], WDir[144], DeleteFile[144], mess[256];
	short	rtn=0, NumErr=0,st;
	BTVARDESC	BTVar[3];
	int	i, ifield;  
	HFILE		hMessFile;
	OFSTRUCT	OFStruct;
	HANDLE	hBT;  
	struct {unsigned short date, time;} dant;
						
	GetTempFileName (NULL,"gm",NULL,(LPSTR)Name);

	BTVar[0].BT_VARTYP=BT_CHAR;
	BTVar[0].BT_VARLEN=16;
	BTVar[0].BT_VAROFF=0;
	BT_CREATE (Name, 4, FALSE, 1, 1, (LPBTVARDESC)&BTVar, FALSE, 0, 0, FALSE);
	hBT = BT_OPEN (Name, 0, BT_WRITE, 0);
	
	_fstrcpy (ToDir,"[%INDIR]olddlls");
	ExpandText (ToDir);
	_mkdir (ToDir);	
	
	sprintf (Name,"%s\\readme.txt",ToDir);
	hMessFile = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
	GetWindowsDirectory (str,144);
	_fstrcpy (WDir,str);
	_fstrcat (str,"\\*.dll");
	st = _dos_findfirst (str,_A_RDONLY|_A_SYSTEM|_A_HIDDEN,&FileInfo);
	while (!st)
	{   
		_fstrncpy (Name,FileInfo.name,16); 
		dant.date = FileInfo.wr_date;
		dant.time = FileInfo.wr_time;
		BT_PUT (hBT,(LPSTR)Name,(LPSTR)&dant);
		rtn++;
		st = _dos_findnext (&FileInfo);
	}
	rtn = 0;
	GetWindowsDirectory (str,144);
	_fstrcat (str,"\\system\\*.dll");
	st = _dos_findfirst (str,_A_RDONLY|_A_SYSTEM|_A_HIDDEN,&FileInfo);
	while (!st)
	{   
		_fstrncpy (Name,FileInfo.name,16); 
		if (!BT_FIND (hBT,(LPSTR)Name,BT_FIRST,BT_EQ,(LPSTR)&dant))
		{
			if (dant.date < FileInfo.wr_date)
				sprintf (DeleteFile,"%s\\%s",WDir,Name);
			else
				sprintf (DeleteFile,"%s\\system\\%s",WDir,Name); 
			sprintf (ToName,"%s\\%s",ToDir,Name);
			copyfile (ToName,DeleteFile,FALSE,0,0,0,0); 
			if (remove (DeleteFile))
			{
				NumErr++;
				sprintf (mess,"Unable to remove %s",DeleteFile);
			}                                                   
			else
				sprintf (mess,"Successfully removed %s",DeleteFile);
			rtn++;
			fputstring (mess,hMessFile);  
		}
		st = _dos_findnext (&FileInfo);
	} 
	BT_CLOSEANDDELETE (&hBT);     
	GSSiClose (hMessFile);
	sprintf (mess,"%i duplicates found - unable to remove %i",rtn,NumErr);
	MessageBox (GetFocus(),mess,"",MB_OK);
	return rtn;
}



