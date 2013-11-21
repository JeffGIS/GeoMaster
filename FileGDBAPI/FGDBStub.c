#include "graphint.h"

int  OpenFGDB2 (LPSTR DBName,LPSTR Table,LPSTR SQL)
{
	return FALSE;
}
int FGDBGetChildList (int iDB,LPCTSTR Under,int Type,int MaxElementSize,LPHANDLE phList)
{
	return FALSE;
}
BOOL FGDBGetTableInfo (int iDB,LPCTSTR TablePath,LPINT pType,LPINT pnRows,LPMNMXCORD pBounds)
{
	return FALSE;
}
BOOL CloseFGDB (int iDB)
{
	return FALSE;
}
BOOL FGDBGetFieldInfo(int iDB,int icount, LPSTR FieldName, LPINT pFieldWidth,LPINT pFieldType)
{
	return FALSE;
}
int  FGDBGetFieldCount(int iDB)
{
	return FALSE;
}
int  NumRowsInFGDBTable(int iDB)
{
	return FALSE;
}
BOOL FGDBCloseCursor(int iSt)
{
	return FALSE;
}
BOOL FGDBFreeStmt(int iSt)
{
	return FALSE;
}
int FetchFGDBRecord (LPOPENFILEDATA	FilePtr)
{
	return FALSE;
}
LPVOID GetFGDBFieldData ( LPOPENFILEDATA FilePtr, LPCSTR indexIN, LPVOID *hstmt,
									 LPFIELDINFO infield, BOOL SingleVal, short FunctionID,short *irc,
									 int NumFields,LPFIELDINFO FirstField)
{
	return 0;
}
BOOL SendEmail (LPSTR From,LPSTR To,LPSTR Subject,LPSTR Message,LPSTR Attach,LPSTR Response)
{

	return FALSE;
}
