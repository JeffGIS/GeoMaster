#if ! defined XPVERSION && ! defined NVSERVER
/**
 * Sample: ExecutingSQL
 *
 * Demonstrates how to use ExecuteSQL to perform standard SQL requests on a file geodatabase.
 * Prior to running this example, copy the ExecuteSQL.gdb from the data
 * directory to the working directory.
 */

#include <windows.h>
#include <ctime>
#include <cmath>
#include <iostream>
#include <string>
#include <fstream>

//#include <glut.h>
//#include <atlbase.h>
//#include <atlstr.h>

#include "FileGDBAPI.h"

#define LONGWSDEF (L"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    ");

typedef long *LPLONG;
typedef short *LPSHORT;
typedef float *LPFLOAT;
typedef double *LPDOUBLE;
#pragma pack(2)
typedef struct
   {    double  xmn;
        double  ymn;
        double  xmx;
        double  ymx;
    } MNMXCORD;
typedef MNMXCORD    *LPMNMXCORD; 

typedef struct {
	int		shapeType;
	int		geometryType;
	int		inUseLength;
	BYTE    isEmpty;
	BYTE	hasZs;
	BYTE	hasMs;
	BYTE	hasIDs;
	BYTE	hasCurves;
	BYTE	hasNormals;
	BYTE	hasTextures;
	BYTE	hasMaterials;
}FILEGDBRECHEADER;
typedef FILEGDBRECHEADER	*LPFILEGDBRECHEADER;
#pragma pack()

#if CHECKMEM        
#define	GlobalLock	GSSiGLOBALLOCK
#define	GlobalUnlock	GSSiGLOBALUNLOCK
#define	GlobalAlloc	GSSiGLOBALALLOC
#define	GlobalFree	GSSiGLOBALFREE
#endif

#define	GlobalSize	GSSiGLOBALSIZE
#define GlobalReAlloc GSSiGLOBALREALLOC  
#define PostMessageA GSSiPOSTMESSAGE 
extern "C" BOOL    WINAPI GSSiPOSTMESSAGE(HWND, UINT, WPARAM, LPARAM);
extern "C" void LogMemAlloc (unsigned short MemID,long MemLen);
extern "C" LPVOID GSSiGLOBALLOCK (HANDLE hglb);
extern "C" DWORD GSSiGLOBALSIZE (HANDLE hglb);
extern "C" BOOL GSSiGLOBALUNLOCK (HANDLE hglb);
extern "C" HGLOBAL GSSiGLOBALFREE (HANDLE hglb);
extern "C" HGLOBAL WINAPI GSSiGLOBALREALLOC (HGLOBAL hglb, DWORD cbAlloc, UINT fuAlloc);  
extern "C" void GSSiRemoveMem (HGLOBAL hglb);  
extern "C" HGLOBAL GSSiGLOBALALLOC(UINT fuAlloc, DWORD cbAlloc);
extern "C" MNMXCORD atobounds (LPSTR Value,LPBOOL err);
extern "C" LPSTR strncpy0(LPSTR Buff,LPSTR str, size_t n);
extern "C" LPSTR ExpandText(LPSTR str);


extern "C" HGLOBAL GSSiGlobAlloc (int From,UINT fuAlloc, long cbAlloc);
extern "C" HGLOBAL GSSiGlobalReAlloc (USHORT From,HGLOBAL hGlob, long cbAlloc,UINT fuAlloc);
extern "C" LPSTR ftoa (LPSTR Value,double DVal);
extern "C" int FGDBGetChildList (int iDB,LPCTSTR Under,int Type,int MaxElementSize,LPHANDLE phList);

using namespace std;
using namespace FileGDBAPI;

#define esriShapeBasicTypeMask 255

#define MAXOPENFGDB	32

static	char		openGDBName[MAXOPENFGDB][MAX_PATH]={0};
static	int			openGDBid[MAXOPENFGDB];
static	int			numOpens[MAXOPENFGDB] = {0};
static	Geodatabase geodatabase[MAXOPENFGDB];
static	Table		table[MAXOPENFGDB];
static	int			gdbInUse[MAXOPENFGDB]={0};
static	EnumRows	attributeQueryRows[MAXOPENFGDB];
static	Row			row[MAXOPENFGDB];
static	int			ii=0,not=0,nod=0,noq=0;

static	wstring fieldName LONGWSDEF;

#include "gmlimits.h"
#pragma pack(2)
typedef struct
    { short  type,
             index,
             radix,
             scale;
      long   length,
             precision;       
      char   name[62]; 
      HANDLE    hCurVal;
    } FIELDINFO;
typedef FIELDINFO FAR  *LPFIELDINFO;

typedef struct
    {
        HANDLE  hGlobal;
        short   FieldNum;
        short	OpCode;
        char    String[256];
    }   SQLFIELD;
typedef SQLFIELD    FAR *LPSQLFIELD;

typedef struct
    {   
		HANDLE  myhandle,
			FileHandle,
			BufferHandle;
		short   Type,
			NumSQLs,
			NumFields;
		long    FirstLineOffset;
		HFILE	Fid;
		HANDLE  SQLHandles[MAXSQLPERFILE];
		char    fullpath[_MAX_PATH];
		char	table[256];
		short	HaveNonStandardFields;
		FIELDINFO   FldInfo;
}OPENFILEDATA;
typedef OPENFILEDATA    FAR *LPOPENFILEDATA;    
typedef struct
    {   
        HANDLE  myhandle,
                OFHandle; 
        short   OpenFileID;
		short	Access;
        DWORD   lastreadtime;  
        short   st;
        long    Offset,
        		MacroID;
        LPVOID  hstmt;
        char    IDName[34];
		char	myhandleC[16];
        char    SQL[4096]; 
        short   IndexToUse; 
        short	Unique;
        short   NumGlobals;
		short	singleValID;
		SQLFIELD    SQLField;
    }OPENSQLDATA; 
typedef OPENSQLDATA FAR *LPOPENSQLDATA;    
typedef struct {int   length; char Value;} CURVAL;
typedef CURVAL  FAR *LPCURVAL;
#pragma pack()
#define SHPT_NULL	0
#define SHPT_POINT	1
#define SHPT_ARC	3   
#define SHPT_TEXT	99	//added by GSSi  
#define SHPT_POLYGON_PGDB 4//added by GSSi
#define SHPT_POLYGON	5 
#define SHPT_POLYLINE_WITHCURVES	536870962	//added by GSSi
#define SHPT_POLYGON_WITHCURVES	536870963	//added by GSSi
#define SHPT_MULTIPOINT	8
#define SHPT_POINTZ	11
#define SHPT_ARCZ	13
#define SHPT_POLYGONZ	15
#define SHPT_MULTIPOINTZ 18 
#define SHPT_PGDB_POLYGONZ 19 //added by GSSi
#define SHPT_POINTM	21
#define SHPT_ARCM	23
#define SHPT_POLYGONM	25
#define SHPT_MULTIPOINTM 28
#define SHPT_MULTIPATCH 31

static	char	sqlstr[4096];
static	char	FunctionName[5][6]={"COUNT","AVG","SUM","MIN","MAX"};

extern "C" HANDLE GetNextXMLElement (HANDLE FileHandle,LPINT pFileLoc,LPSTR TagID);
extern "C" HANDLE GetXMLElementValue (HANDLE hElement);
extern "C" void ClearCurVals (LPOPENFILEDATA FilePtr);

//#include <algorithm>

typedef short	BOOL16;
#include "bt.h"

extern "C" BOOL FGDBCheck(void)
{
	if (not || noq || nod)
		return FALSE;
	return TRUE;
}



/*
// Prototype for conversion functions
std::wstring StringToWString(const std::string& s);
std::string WStringToString(const std::wstring& s);

std::wstring StringToWString(const std::string& s)
{
std::wstring temp(s.length(),L' ');
std::copy(s.begin(), s.end(), temp.begin());
return temp; 
}

*/


std::string WStringToString(const std::wstring& s)
{
	wchar_t *wstr = new wchar_t [s.length()+1];
	char * cstr = new char [s.length()+1];
	

	wcscpy (wstr,s.c_str());
	for (int i=0;i<s.length();i++)
		cstr[i] = wstr[i];
	cstr[s.length()] = 0;
//string str;
//std::string temp(s.length(), ' ');
//int i;
//int l = s.length();
//for (i=0;i<l;i++)
//	temp.c_str[i] = s.c_str[i];
//std::copy(s.begin(), s.end(), temp.begin());
std::string str(cstr);
delete []wstr;
delete []cstr;
return str; 
}

/*int StringToWString(std::wstring &ws, const std::string &s)
{
    std::wstring; wsTmp(s.begin(), s.end());

    ws = wsTmp;

    return 0;
}*/
HANDLE stringToMem (string str)
{
	HANDLE	hMem = GlobalAlloc (GMEM_MOVEABLE,str.size()+1);
	LPSTR	pMem = (LPSTR)GlobalLock (hMem);

	strcpy (pMem,str.c_str());
	GlobalUnlock (hMem);
	return hMem;
}

int OpenGDB (LPCTSTR DBName)
{
	long	hr;
	char	fullName[MAX_PATH];
	int		openID = -1;

	_fullpath (fullName,DBName,MAX_PATH-1);

	for (int idb=0;idb<MAXOPENFGDB;idb++)
	{
		if (!stricmp (fullName,openGDBName[idb]))
		{
			numOpens[idb]++;
			return idb;
		}
		if (!*openGDBName[idb] && openID < 0)
			openID = idb;
	}
	if (openID < 0)
		return -1;
	string	dbname = LPCTSTR(DBName);
	wstring wdbname (dbname.begin(),dbname.end());
    if ((hr = OpenGeodatabase(wdbname, geodatabase[openID])) != S_OK)
	    return -1;
	nod++;
	strcpy (openGDBName[openID],fullName);
	numOpens[openID] = 1;
	return openID;
}

void CloseGDBid (int id)
{
	numOpens[id]--;
	if (!numOpens[id])
	{
		CloseGeodatabase(geodatabase[id]);
		*openGDBName[id] = 0;
		nod--;
	}
	return;
}

extern "C" int OpenFGDB2 (LPCTSTR DBName,LPSTR Table,LPSTR SQL)
{   
	int		rtn=-1;
	long	hr;
	int		idb;
	vector<wstring> childList(1); 
	
	for (idb=0;idb<MAXOPENFGDB;idb++)
		if (!gdbInUse[idb])
			goto HaveDB;
	return 0;

HaveDB:

	int openID = OpenGDB (DBName);
	if (openID < 0)
		return 0;
	openGDBid[idb] = openID;
	gdbInUse[idb] = 1;
	if (Table)
	{
		string	tablename = LPCTSTR(Table);
		wstring wtablename(tablename.begin(), tablename.end());
		if (*Table)
		{
			if ((hr = geodatabase[openGDBid[idb]].OpenTable(wtablename, table[idb])) != S_OK)
			{
				CloseGDBid(openGDBid[idb]);
				gdbInUse[idb] = 0;
				return 0;
			}
			gdbInUse[idb] = 2;
			not++;
		}
	}
    return idb + 1;
}
extern "C" BOOL FGDBGetTableInfo (int iDB,LPCTSTR TablePath,LPINT pType,LPINT pnRows,LPMNMXCORD pBounds)
{   
	long	hr, rc;
	int		n;
	BOOL	rtn=FALSE;
	string undr = TablePath;
	wstring	under (undr.begin(),undr.end());
	Table	table;
	EnumRows	attributeQueryRows;
	Envelope	extent;
	ShapeBuffer shapebuf;
	ShapeType	shapeType;
	GeometryType geometryType;
	FieldType fieldType;
	FieldInfo	fieldInfo;
	//wstring fieldName;

	*pType = 0;
	if (iDB < 1)
		return 0;
	hr = geodatabase[openGDBid[iDB-1]].OpenTable(under, table);
	if (!hr)
	{
		Row row;
		EnumRows	attributeQueryRows;

		not++;
		if (pnRows)
			hr = table.GetRowCount(*pnRows);
		if (pBounds)
		{
			hr = table.GetExtent(extent);
			if (!hr)
			{
				pBounds->xmn = extent.xMin;
				pBounds->xmx = extent.xMax;
				pBounds->ymn = extent.yMin;
				pBounds->ymx = extent.yMax;
			}
		}
		if ((hr = table.Search(L"*", L"", true, attributeQueryRows)) == S_OK)
		{
			noq++;
			if ((rc = attributeQueryRows.Next(row)) == S_OK)
			{
				int			nFields;
				bool		isNull=FALSE;
				int			nAnnoMarkers = 0;
				//char		fName[256];

				attributeQueryRows.GetFieldInformation(fieldInfo);
				fieldInfo.GetFieldCount(nFields);
				for (long fieldNumber = 0; fieldNumber < nFields; fieldNumber++)
				{
				  fieldInfo.GetFieldName(fieldNumber, fieldName);
				 // strcpy (fName,WStringToString(fieldName).c_str());
				  if (!wcsicmp (fieldName.c_str(),L"TextString") ||
					  !wcsicmp (fieldName.c_str(),L"FontName") ||
					  !wcsicmp (fieldName.c_str(),L"FontSize"))
					  nAnnoMarkers++;
				}
				for (long fieldNumber = 0; fieldNumber < nFields; fieldNumber++)
				{
				  fieldInfo.GetFieldType(fieldNumber, fieldType);
				  fieldInfo.GetFieldName(fieldNumber, fieldName);
				  row.IsNull(fieldName, isNull);
				  if (!isNull)
				  {
					    rtn = TRUE;
						switch (fieldType)
						{
						  case fieldTypeGeometry:
							  {

								row.GetGeometry(shapebuf);
								shapebuf.GetShapeType(shapeType);
								geometryType = shapebuf.GetGeometryType(shapeType);
								if ((geometryType == 4 || geometryType == 5) && nAnnoMarkers == 3)
									*pType = 99;
								else switch (geometryType)
								{
									default:
									case geometryMultiPatch:
										*pType = 0;
										break;
									case  geometryPoint:
										*pType = SHPT_POINT;
										break;
									case geometryMultipoint:
										*pType = SHPT_MULTIPOINT;
										break;
									case  geometryPolyline:
										*pType = SHPT_ARC;
										break;
									case  geometryPolygon:
										*pType = SHPT_POLYGON;
										break;
								}
							  }
							  break;
						  default:
							  ii=1;
							  break;
						}
				  }
				}
			}
			ii=2;
			attributeQueryRows.Close();
			noq--;
		}
		geodatabase[openGDBid[iDB-1]].CloseTable(table);
		not--;
	}
    return rtn;
}
extern "C" int FGDBGetChildList (int iDB,LPCTSTR Under,int Type,int MaxElementSize,LPHANDLE phList)
{   
	long	hr;
	int		n;
	string undr = Under;
	wstring	type[3]= {L"Table",L"Feature Class",L"Feature Dataset"};
	vector<wstring> childList(5); 
	wstring	under (undr.begin(),undr.end());
	
	
	if (iDB < 1)
		return 0;
	if (Type < 1 || Type > 3)
		return 0;
	hr = geodatabase[openGDBid[iDB-1]].GetChildDatasets(under, type[Type-1], childList);
	if (!hr)
	{
		n = childList.size();
		if (n)
		{
			LPSTR pList;
			int	i;

			*phList = GSSiGlobAlloc (0,GMEM_MOVEABLE,n*MaxElementSize+32);
			pList = (LPSTR)GlobalLock (*phList);
			for (i=0;i<n;i++,pList+=MaxElementSize)
				strcpy (pList,WStringToString(childList[i]).c_str());

			GlobalUnlock (*phList);
		}
	}

    return n;
}
extern "C" BOOL CloseFGDB (int iDB)
{
	long hr;

	if (!iDB--)
		return FALSE;
	  
	if (gdbInUse[iDB] == 3)
	{
		attributeQueryRows[iDB].Close(); 
		gdbInUse[iDB] = 2;
		noq--;
	}
	if (gdbInUse[iDB] == 2)
	{
		hr = geodatabase[openGDBid[iDB]].CloseTable(table[iDB]);
		not--;
	}
	if (gdbInUse[iDB])
		CloseGDBid (openGDBid[iDB]);
	gdbInUse[iDB] = 0;
	return TRUE;
}

int GetFGDBCVal (LPFIELDINFO field,LPSTR answer,int lMaxAnswer)
{
	int	l=0;
	
	*answer = 0;
	if (field->hCurVal)
	{
		LPCURVAL pCurVal= (LPCURVAL)GlobalLock (field->hCurVal);

		if (pCurVal->length)
		switch (field->type)
		{
		case BT_INTEGER:
			if (field->length == 2)
				itoa (*(LPSHORT)&pCurVal->Value,answer,10);
			else
				itoa (*(LPLONG)&pCurVal->Value,answer,10);
			l = strlen (answer);
			break;
		case BT_REAL:
			if (field->length == 4)
				ftoa (answer,*(LPFLOAT)&pCurVal->Value);
			else
				ftoa (answer,*(LPDOUBLE)&pCurVal->Value);
			l = strlen (answer);
			break;
		case BT_CHAR:
		default:
			if (pCurVal->length < 0) //handle
				itoa ((int)field->hCurVal,answer,10);
			else
			{
				l = min (lMaxAnswer-1,pCurVal->length);
				if (l)
					strncpy (answer,&pCurVal->Value,l);
				answer[l] = 0;
			}
		}
		GlobalUnlock (field->hCurVal);
	}
	return l;
}


extern "C" BOOL FGDBGetFieldInfo(int iDB,int icount, LPSTR FieldName, LPINT pFieldWidth,LPINT pFieldType)
{
	long hr;
	BOOL	rtn=FALSE;

	string tableDef;
	static	int nFields;
	static	EnumRows attrQueryRows;
	static	FieldInfo fieldInfo;
	FieldType fieldType;
	//wstring   fieldName;
	int		fieldLength;

  // Iterate through the returned rows printing out all field values.
	short     shortField;
	int32     longField;
	float     floatField;
	double    doubleField;
	string    stringField;
	wstring   wstringField;
	tm        dateTimeField;
	char      datetime[80];
	GUID      globalIDField;
	GUID      guidField;
	wchar_t   strGuid[40];
	wchar_t   strGlobalID[40];
	bool      isNull;
	int		  ii;
 
	icount++;

	if (iDB-- > 0 && gdbInUse[iDB])
	{

	   //wstring fieldName;
		if (icount == 1)
		{
			if ((hr = table[iDB].Search(L"*", L"", true, attrQueryRows)) != S_OK)
			{
				return FALSE;
			}
			noq++;
			attrQueryRows.GetFieldInformation(fieldInfo);
			fieldInfo.GetFieldCount(nFields);
		}
		fieldInfo.GetFieldType(icount-1, fieldType);
		fieldInfo.GetFieldName(icount-1, fieldName);
		fieldInfo.GetFieldLength(icount-1, fieldLength);
		strcpy (FieldName,WStringToString(fieldName).c_str());

		switch (fieldType)
		{
		  case fieldTypeSmallInteger:
			  *pFieldWidth = 2;
			  *pFieldType = BT_INTEGER;
		  break;
          
		  case fieldTypeInteger:
			  *pFieldWidth = 4;
			  *pFieldType = BT_INTEGER;
		  break;
          
		  case fieldTypeSingle:
			  *pFieldWidth = 4;
			  *pFieldType = BT_REAL;
		  break;
          
		  case fieldTypeDouble:
			  *pFieldWidth = 8;
			  *pFieldType = BT_REAL;
		  break;
          
		  case fieldTypeString:
			  *pFieldWidth = fieldLength;
			  *pFieldType = BT_CHAR;
		  break;
          
		  case fieldTypeDate:
			  *pFieldWidth = 4;
			  *pFieldType = BT_INTEGER;
		  break;
          
		  case fieldTypeOID:
			  *pFieldWidth = fieldLength;
			  *pFieldType = BT_INTEGER;
		  break;
          
		  case fieldTypeGeometry:
			  *pFieldWidth = 64;
			  *pFieldType = BT_CHAR;
		  break;
          
		  case fieldTypeBlob:
			  *pFieldWidth = fieldLength;
			  *pFieldType = BT_CHAR;
		  break;
          
		  case fieldTypeGUID:
			  *pFieldWidth = fieldLength;
			  *pFieldType = BT_CHAR;
		  break;
          
		  case fieldTypeGlobalID:
			  *pFieldWidth = fieldLength;
			  *pFieldType = BT_CHAR;
		  break;
		  
		  case fieldTypeXML:
			  *pFieldWidth = fieldLength;
			  *pFieldType = BT_CHAR;
		  break;
          
		  default:
			  MessageBox (0,"Invalid field type",0,MB_ICONEXCLAMATION);
		  break;
		}
		if (icount == nFields)
		{
			attrQueryRows.Close();
			noq--;
		}
	}
	return rtn;
}
extern "C"  BOOL FGDBCloseCursor(int iDB)
{
	if (gdbInUse[iDB] == 3)
	{
		attributeQueryRows[iDB].Close(); 
		gdbInUse[iDB] = 2;
		noq--;
	}
	return TRUE;
}
extern "C"  BOOL FGDBFreeStmt(int iSt)
{
	return TRUE;
}

extern "C" int  FGDBGetFieldCount(int iDB)
{
	long hr;

	string tableDef;
	int nFields=0;
	FieldInfo	fieldInfo;

	if (iDB-- > 0 && gdbInUse[iDB])
	{
		EnumRows attrQueryRows;

		if ((hr = table[iDB].Search(L"*", L"", true, attrQueryRows)) != S_OK)
		{
		    return -1;
		}
		noq++;
		attrQueryRows.GetFieldInformation(fieldInfo);
		fieldInfo.GetFieldCount(nFields);
		attrQueryRows.Close();
		noq--;
 	}
	return nFields;
}
extern "C" int  NumRowsInFGDBTable(int iDB)
{
	long hr;

	int nRows=0;

	if (iDB-- > 0 && gdbInUse[iDB] > 1)
	{

		if ((hr = table[iDB].GetRowCount(nRows)) != S_OK)
		{
		    return -1;
		}
	}
	return nRows;
}

extern "C" int FetchFGDBRecord (LPOPENFILEDATA	FilePtr,int singleValID)
{	
	int	rc=-1;
	if (FilePtr)
	{
		int	iDB = (int)FilePtr->FileHandle - 1;
   		LPFIELDINFO	pField=&FilePtr->FldInfo;

		ClearCurVals (FilePtr);
		if ((rc = attributeQueryRows[iDB].Next(row[iDB])) == S_OK)
		{
			FieldInfo	fieldInfo;
			int			nFields, fieldLength;
			short     shortField;
			int32     longField;
			float     floatField;
			double    doubleField;
			string    stringField;
			tm        dateTimeField;
			char      datetime[80];
			GUID	  globalIDField;
			Guid      guidField;
			wchar_t	  strGuid[40];
			wchar_t   strGlobalID[40];
			bool      isNull;
			FieldType fieldType;
			LPCURVAL  pCurVal;
			int		  ii,iii;
			LONGLONG  sysTime;
			//wstring fieldName;

			attributeQueryRows[iDB].GetFieldInformation(fieldInfo);
			fieldInfo.GetFieldCount(nFields);
			for (long fieldNumber = 0; fieldNumber < nFields; fieldNumber++,pField++)
			{
				if (singleValID && fieldNumber != singleValID - 1)
					continue;
			  fieldInfo.GetFieldType(fieldNumber, fieldType);
			  fieldInfo.GetFieldName(fieldNumber, fieldName);
			  fieldInfo.GetFieldLength(fieldNumber, fieldLength);
			  row[iDB].IsNull(fieldName, isNull);
			  fieldLength = max (1,pField->length);
			  pField->hCurVal = GSSiGlobAlloc (1731,GMEM_MOVEABLE,fieldLength+sizeof(int)+32);
			  pCurVal = (LPCURVAL)GlobalLock (pField->hCurVal);
			  pCurVal->length = fieldLength;
			  if (!isNull)
			  {
				switch (fieldType)
				{
				  case fieldTypeSmallInteger:
					row[iDB].GetShort(fieldName, shortField);
					memmove ((LPSTR)&pCurVal->Value,&shortField,fieldLength);
				  break;
		          
				  case fieldTypeInteger:
					row[iDB].GetInteger(fieldName, longField);
					memmove ((LPSTR)&pCurVal->Value,&longField,fieldLength);
				  break;
		          
				  case fieldTypeSingle:
					row[iDB].GetFloat(fieldName, floatField);
					memmove ((LPSTR)&pCurVal->Value,&floatField,fieldLength);
				  break;
		          
				  case fieldTypeDouble:
					row[iDB].GetDouble(fieldName, doubleField);
					memmove ((LPSTR)&pCurVal->Value,&doubleField,fieldLength);
					if (doubleField == 18839.0)
						ii = 1;
				  break;
		          
				  case fieldTypeString:
					  {
						wstring   wstringField (L"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        ");
					row[iDB].GetString(fieldName, wstringField);
					std::string stringField = WStringToString(wstringField);
					iii = stringField.length();
					strncpy0 ((LPSTR)&pCurVal->Value,(LPSTR)stringField.c_str(),stringField.length());
					//if (!stricmp((LPSTR)&pCurVal->Value, "283401320222"))
					//	ii = 1;
					  }
				  break;
		          
				  case fieldTypeDate:
					row[iDB].GetDate(fieldName, dateTimeField);
					sysTime = mktime (&dateTimeField); 
//					strftime(datetime,80,"%a %b %d %I:%M:%S%p %Y", &dateTimeField);
					memmove ((LPSTR)&pCurVal->Value,&sysTime,fieldLength);

				  break;
		          
				  case fieldTypeOID:
					row[iDB].GetOID(longField);
					memmove ((LPSTR)&pCurVal->Value,&longField,fieldLength);
				  break;
		          
				  case fieldTypeGeometry:
					  {
						    size_t          inUseLength;

  
						ShapeBuffer shapebuf;
						MultiPartShapeBuffer mpShapebuf;
						LPFILEGDBRECHEADER	pFGDBShapeHeader;
						LPBYTE	shpBuf;
						ShapeType	shapeType;
						GeometryType geometryType;
						double* zArray=NULL;

						row[iDB].GetGeometry(shapebuf);
						GlobalUnlock (pField->hCurVal);
						fieldLength = sizeof (FILEGDBRECHEADER) + shapebuf.inUseLength;
						pField->hCurVal = GSSiGlobalReAlloc (1734,pField->hCurVal,fieldLength+sizeof(int),GMEM_MOVEABLE);
						pCurVal = (LPCURVAL)GlobalLock (pField->hCurVal);
						pCurVal->length = -fieldLength;
						pFGDBShapeHeader = (LPFILEGDBRECHEADER)&pCurVal->Value;
						shpBuf = (LPBYTE)(pFGDBShapeHeader + 1);
						pFGDBShapeHeader->inUseLength = (int)shapebuf.inUseLength;
						shapebuf.GetShapeType(shapeType);
						if ((shapeType & esriShapeBasicTypeMask) == shapeGeneralPolyline)
						{
							pFGDBShapeHeader->shapeType = (int)shapeGeneralPolyline;
						}
						else
							pFGDBShapeHeader->shapeType = (int)shapeType;
						pFGDBShapeHeader->hasZs = shapebuf.HasZs(shapeType);
						pFGDBShapeHeader->hasMs = shapebuf.HasMs(shapeType);
						pFGDBShapeHeader->hasIDs = shapebuf.HasIDs(shapeType);
						pFGDBShapeHeader->hasNormals = shapebuf.HasNormals(shapeType);
						pFGDBShapeHeader->hasTextures = shapebuf.HasTextures(shapeType);
						pFGDBShapeHeader->hasCurves = shapebuf.HasCurves(shapeType);
						pFGDBShapeHeader->hasMaterials = shapebuf.HasMaterials(shapeType);
						pFGDBShapeHeader->geometryType = shapebuf.GetGeometryType(shapeType);
						pFGDBShapeHeader->isEmpty = shapebuf.IsEmpty();
						if (pFGDBShapeHeader->hasZs)
						{
							row[iDB].GetGeometry(mpShapebuf);
							mpShapebuf.GetZs(zArray);
						}
						if (pFGDBShapeHeader->hasCurves)
						{
							int numCurves;
							byte *curves;

							row[iDB].GetGeometry(mpShapebuf);
							mpShapebuf.GetNumCurves(numCurves);
							mpShapebuf.GetCurves(curves);
							for (int i = 0; i < numCurves; i++)
							{
								//curveType = curves[i].GetCurveType;
							}

						}
						memmove (shpBuf,(LPBYTE)shapebuf.shapeBuffer,shapebuf.inUseLength);
					  }
				  break;
		          
				  case fieldTypeBlob:
					  {
						ByteArray binaryBuf;

						rc = row[iDB].GetBinary(fieldName, binaryBuf);
						GlobalUnlock (pField->hCurVal);
						fieldLength = binaryBuf.inUseLength;
						pField->hCurVal = GSSiGlobalReAlloc (1733,pField->hCurVal,fieldLength+sizeof(int),GMEM_MOVEABLE);
						pCurVal = (LPCURVAL)GlobalLock (pField->hCurVal);
						pCurVal->length = -fieldLength;
						memmove ((LPSTR)&pCurVal->Value,binaryBuf.byteArray,fieldLength);
					  }
				  break;
		          
				  case fieldTypeGUID:
				  {
					int i;
				 }
				  break;
		          
				  case fieldTypeGlobalID:
				  {
					int i;
					LPSTR	pStr=&pCurVal->Value,pWstr;
					Guid	gid;

					row[iDB].GetGlobalID(gid);
					break;
					//globalIDField = (GUID)gid;
					//sprintf ((LPSTR)&pCurVal->Value,"%ld-%i-%i-%s",globalIDField.Data1,globalIDField.Data2,globalIDField.Data3,globalIDField.Data4);
					//::StringFromGUID2(gid, strGlobalID, 40);
					pWstr = (LPSTR)strGlobalID;
					for (i=0;i<fieldLength;i++,pWstr++)
						*pStr++ = *pWstr++;
				  }
				  break;

				  case fieldTypeXML:
					row[iDB].GetXML(fieldName, stringField);
					GlobalUnlock (pField->hCurVal);
					fieldLength = (int)stringField.length() + 1;
					pField->hCurVal = GSSiGlobalReAlloc (1735,pField->hCurVal,fieldLength+sizeof(int),GMEM_MOVEABLE);
				    pCurVal = (LPCURVAL)GlobalLock (pField->hCurVal);
					pCurVal->length = fieldLength;
					memmove ((LPSTR)&pCurVal->Value,stringField.c_str(),fieldLength);
					break;
		          
				  default:
					  ii=1;
				  break;
				}
			}
			  else
			  {
				  pCurVal->length = 0;
				  pCurVal->Value = 0;
			  }
			GlobalUnlock (pField->hCurVal);
		  }
		}
		else
			ii=1;
	}
    return rc;
}
extern "C" LPVOID GetFGDBFieldData ( LPOPENFILEDATA FilePtr, LPCSTR indexIN, LPVOID *hstmt,
									 LPFIELDINFO infield, BOOL SingleVal, short FunctionID,short *irc,
									 int NumFields,LPFIELDINFO FirstField,int singleValID)
{
	static char answer[4096];
	string	sQL;
	EnumRows attrQueryRows;
	long	hr;
	int		WantField, startfield, j, FldNum, NumResultCols;
	LPFIELDINFO field;
	LPCURVAL	pCurVal;
	LPVOID	lpvoid=answer;
	int		iDB = (int)FilePtr->FileHandle - 1;
	LPSTR	index=0;

	if (!gdbInUse[iDB])
	{
		*irc = 1;
		return lpvoid;
	}
	*irc = 0;
	if (infield->hCurVal)
	{ 
		GetFGDBCVal (infield,answer,4096);
	    return lpvoid;
	}

	index = (LPSTR)malloc(4096);
	strcpy(index, indexIN);
	ExpandText(index);
	if (gdbInUse[iDB] == 2)
	{
		BOOL	haveEnvelope=FALSE;
		Envelope envelope;
		LPSTR	pBounds, pEnd;

		pBounds = (LPSTR)strstr (index,"BOUNDS=(");
		if (pBounds)
		{
			MNMXCORD	bounds;
			BOOL		err;

			*pBounds = 0;
			pBounds += 8;
			if ((pEnd = strchr (pBounds,')')))
				*pEnd = 0;
			bounds = atobounds (pBounds,&err);
			envelope.xMin = bounds.xmn;
			envelope.yMin = bounds.ymn;
			envelope.xMax = bounds.xmx;
			envelope.yMax = bounds.ymx;
			haveEnvelope = TRUE;
		}
		sQL = LPCTSTR(index);
		wstring sql (sQL.begin(),sQL.end());
	    if (SingleVal)
	    {  
			if (FunctionID)
				sprintf(sqlstr, "%s(\"%s\")", FunctionName[FunctionID - 1], infield->name);
			else
				sprintf(sqlstr, "%s", infield->name);
			wstring wsqlstr(sqlstr, sqlstr + strlen(sqlstr));
			if (haveEnvelope)
			{

				if ((hr = table[iDB].Search(wsqlstr, sql, envelope, true, attributeQueryRows[iDB])) != S_OK)
					*irc = 1;
				else
					noq++;
			}
			else
			{
				if ((hr = table[iDB].Search(wsqlstr, sql, true, attributeQueryRows[iDB])) != S_OK)
					*irc = 1;
				else
					noq++;
			}
				
	    }
	    else if (haveEnvelope)
		{
			if ((hr = table[iDB].Search(L"*",sql, envelope,true, attributeQueryRows[iDB])) != S_OK)
				*irc = 1;
			else
				noq++;
		}
		else
		{
			if ((hr = table[iDB].Search(L"*",sql, true, attributeQueryRows[iDB])) != S_OK)
				*irc = 1;
			else
				noq++;
		}
		if (*irc)
			goto Exit;
		else
			gdbInUse[iDB] = 3;
		*irc = FetchFGDBRecord (FilePtr,singleValID);
	};
 	GetFGDBCVal (infield,answer,4096);
	if (SingleVal)
	{
		ClearCurVals (FilePtr);
		FGDBCloseCursor(iDB);
	}
Exit:
	if (index)
		free(index);
    return lpvoid;
}

#endif