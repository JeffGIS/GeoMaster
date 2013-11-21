#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sql.h>
#include <sqlext.h>

typedef	struct
	{ short  type,
             index,
             radix,
             scale;
      long   length,
             precision;       
      char   name[64];
	} FIELDINFO;
typedef FIELDINFO FAR  *LPFIELDINFO;


HANDLE OpenExternalDatabase ( LPSTR Name);

HANDLE OpenDatabaseTable (HANDLE DBHandle, LPSTR TableName);
/*	Opens the specified table						*/

LPFIELDINFO GetExternalFieldInfo (HANDLE TBLHandle, BOOL First);
/*	Returns a pointer to a FIELDINFO struct.		*/
/*	If First is TRUE returns the first table.		*/
/*	Otherwise returns the next or NULL if at end	*/  

void CloseExternalDatabase (HANDLE DBHandle);
void main(void);     
          
#define MAX_STMT_LEN 100 
int FindType(char *szTypeName); 
UDWORD display_size(SWORD coltype, UDWORD collen, UCHAR *colname);
int i;  // watch this variable before and after call SQLColumns
static int  DBType[64]; 
static HWND DBS[64]; 
static HENV HENVS[64];
static HDBC HDBCS[64];
static HSTMT hstmts[64];
static char TableNames[64][32];
static char DBNames[64][64];
static char DBDataSource[64][64];
static char indexes[64][32];

 
void main(void)     
{
  FIELDINFO mess;
  LPFIELDINFO lpmess = &mess;
  HANDLE DBHandle = 0; 
  double spacers[818];  // take this out and it works ok
  spacers[0] = 0;       // gotta comment this out too
   DBHandle = OpenExternalDatabase( "TEST_ACCESS");
   DBHandle = OpenDatabaseTable ( DBHandle, "COMBOTEST"); 
   // the next call is where the trouble occurs.
   lpmess =   GetExternalFieldInfo( DBHandle, TRUE); 
   CloseExternalDatabase (DBHandle);     
}

//********************************************************************//
LPFIELDINFO GetExternalFieldInfo( HANDLE DBhandle, BOOL first)
{
#define STR_LEN 128+1 
#define REM_LEN 254+1
static FIELDINFO mine;
static FIELDINFO FAR *lpmine = &mine; 
SDWORD cbTableQual, cbTableOwner, cbTableName, cbColumnName;
UCHAR  szQual[STR_LEN+1],       szTableName[STR_LEN+1],
       szTableOwner[STR_LEN+1], szTypeName[STR_LEN+1],  
       szRemarks[STR_LEN+1];
SDWORD cbRemarks, cbDataType, cbPrecision, cbLength,
       cbRadix, cbNullable, cbTypeName,cbScale;
RETCODE rc;       
SWORD Nullable;
HSTMT hstmt;
  for(i = 1; i < 64; i++) if(DBS[i] == DBhandle)break; 
  if(first)
  {
    mine.precision = 0;
    mine.scale = 0;
    mine.radix = 0;
    mine.length = 0; 
    SQLAllocStmt(HDBCS[i], &hstmt);  
    // the variable i just got set to 1.  the next call trashes it!
    rc = SQLColumns(hstmt, NULL, 0, NULL, 0, DBNames[i], SQL_NTS, NULL, 0);
    if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)  goto s44;
    i = 1;  // gotta reset it after the previous call blew it away
    hstmts[i] = hstmt;
    rc = SQLBindCol(hstmts[i],  1, SQL_C_CHAR,   szQual, STR_LEN, &cbTableQual); 
    rc = SQLBindCol(hstmts[i],  2, SQL_C_CHAR,   szTableOwner, STR_LEN, &cbTableOwner); 
    rc = SQLBindCol(hstmts[i],  3, SQL_C_CHAR,   szTableName, STR_LEN, &cbTableName); 
    rc = SQLBindCol(hstmts[i],  4, SQL_C_CHAR,   mine.name, STR_LEN, &cbColumnName); 
    rc = SQLBindCol(hstmts[i],  5, SQL_C_SHORT, &mine.type, 0, &cbDataType); 
    rc = SQLBindCol(hstmts[i],  6, SQL_C_CHAR,   szTypeName, STR_LEN, &cbTypeName); 
    rc = SQLBindCol(hstmts[i],  7, SQL_C_LONG,  &mine.precision, 0, &cbPrecision); 
    rc = SQLBindCol(hstmts[i],  8, SQL_C_LONG,  &mine.length, 0, &cbLength); 
    rc = SQLBindCol(hstmts[i],  9, SQL_C_SHORT, &mine.scale, 0, &cbScale); 
    rc = SQLBindCol(hstmts[i], 10, SQL_C_SHORT, &mine.radix, 0, &cbRadix); 
    rc = SQLBindCol(hstmts[i], 11, SQL_C_SHORT, &Nullable, 0, &cbNullable); 
    rc = SQLBindCol(hstmts[i], 12, SQL_C_CHAR,   szRemarks, REM_LEN, &cbRemarks); 
   } //end of if (first) 
    mine.type = 0;
    rc = SQLFetch(hstmts[i]);
    while(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
    { 
      // if(mine.type == 0)lpmine->type = FindType(szTypeName); 
       return lpmine;
    
    }
s44: SQLFreeStmt(hstmts[i], SQL_DROP);
     return NULL;
}

//***********************************************************************//
HANDLE OpenExternalDatabase( LPSTR name)
{
static BOOL first_entry = TRUE;
int  i, next_one;
HENV henv;
HDBC hdbc;
RETCODE rc; 
SDWORD nerr;
SWORD mlen = 253, Moutlen, maxoutlen = 255, outlen;
char errmess[254];
char tnames[256], cstring[100], uid[]="admin", pwd[]="";
UWORD fDirection = SQL_FETCH_FIRST; //SQL_FETCH_NEXT
   if(first_entry)
   {
     for (i = 0; i < 64; DBType[i] = -1, i++);
     first_entry = FALSE;
   }
   for(i = 1; i < 64; i++)if(DBType[i] == -1)break;
   strcpy(cstring,"DSN=");
   strcat(cstring,name);                                        
   next_one = i;
   SQLAllocEnv(&henv);
   SQLAllocConnect(henv, &hdbc);
  // rc = SQLConnect(hdbc, name, SQL_NTS,NULL,0, NULL,0); // uid, SQL_NTS, pwd, SQL_NTS);
   tnames[0] = '\0';
   rc = SQLDriverConnect(hdbc, NULL, cstring, SQL_NTS, tnames, maxoutlen, &Moutlen, SQL_DRIVER_COMPLETE_REQUIRED);
   if(rc ==  SQL_ERROR)
   {
     SQLError(henv,hdbc, SQL_NULL_HSTMT, tnames, &nerr, errmess, mlen, &outlen);
   }  

   if(rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)  goto s44; 
   DBS[next_one] = next_one;
   DBType[next_one] = 'O';
   HENVS[next_one] = henv;
   HDBCS[next_one] = hdbc;
   _fstrcpy (&DBDataSource[next_one][0],name);
   return (HANDLE) next_one;
  
  
s44:  SQLDisconnect(hdbc);
      SQLFreeConnect(hdbc);
      SQLFreeEnv(henv);
      return NULL;

   
}


/******************************************************************/
HANDLE OpenDatabaseTable (HANDLE DBHandle, LPSTR TableName)
{ 

  _fstrcpy(&DBNames[DBHandle][0], TableName);
  return DBHandle;
  
} 
 
/******************************************************************/
void CloseExternalDatabase (HANDLE ClientDDE)
{
      if(ClientDDE == 0)return;
      DBType[ClientDDE] = -1;
      DBS[ClientDDE] = 0;
      SQLDisconnect(HDBCS[ClientDDE]);
      SQLFreeConnect(HDBCS[ClientDDE]);
      SQLFreeEnv(HENVS[ClientDDE]);
      return;
}
