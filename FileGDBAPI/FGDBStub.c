#if (defined XPVERSION) || (defined NVSERVER)

#include "graphint.h"

LPSTR FGDBVersion(void)
{
	static char version[] = { "File Geodatabase Version 0.0" };
	return version;
}


int  OpenFGDB2 (LPSTR DBName,LPSTR Table,LPSTR SQL)
{
	MessageBox(0, "This version of GeoMaster (XP) does not support filegeodatabases", 0, MB_ICONEXCLAMATION);
	return FALSE;
}
int FGDBGetChildList (int iDB,LPCTSTR Under,int Type,int MaxElementSize,LPHANDLE phList)
{
	return FALSE;
}
BOOL FGDBGetTableInfo (int iDB,LPCTSTR TablePath,LPINT pType,LPINT pnRows,LPMNMXCORD pBounds, LPMNMXCORD pSetBounds)
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
int FetchFGDBRecord (LPOPENFILEDATA	FilePtr,int singleValID)
{
	return FALSE;
}
LPVOID GetFGDBFieldData ( LPOPENFILEDATA FilePtr, LPCSTR indexIN, LPVOID *hstmt,
									 LPFIELDINFO infield, BOOL SingleVal, short FunctionID,short *irc,
									 int NumFields,LPFIELDINFO FirstField,int singleValID)
{
	return 0;
}
BOOL FGDBCheck(void)
{
	return FALSE;
}
#endif