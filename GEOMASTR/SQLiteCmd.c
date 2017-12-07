#include "graphint.h"   
#include "gmextern.h"
#include "shapefil.h"
//#include "sqlite3ext.h"
#define INDEX_TYPE_RTREE	1
#define INDEX_TYPE_XY		2
static	char	SQLITERefno[128] = "0";
static	char	SQLITEx[128] = "[SQLITE.x]";
static	char	SQLITEy[128] = "[SQLITE.y]";
static	char	SQLITEStartTime[128] = "[SQLITE.BDate]";
static	char	SQLITEEndTime[128] = "[SQLITE.EDate]";
static  char	SQLITEIndexType = INDEX_TYPE_RTREE;
static	char	SQLITESymbol[128] = "$SYMNUM(WALLPOINT)";
static	char	SQLITESize[128] = "-10";
static	char	SQLITETAG[128] = "CONTROLN:[SQLITE.Wall Id]", SQLITETag[128];//"CASENUM:[SQLITE.CaseNbr]";
static	int		SQLITEXIndex = 1, SQLITEYIndex = 2;
static	int		SQLITEXField = 7, SQLITEYField = 8;
static	DPOINT	SQLITEPoint;
static	int		SQLITEPointSize;
static	long	SQLITEColor = -1;
static char		SQLITEBeginDate[256];
static char		SQLITEEndDate[256];
static time_t	SQLITEParmTime = 0;
static	int		NumSQLITEParms = 0;
static short	HaveSQLITESym = -1;
static BOOL		SQLITEProjectionIsBase;
static char		SQLITEParms[4096] = "";
static char		LastSQLITEFile[MAX_PATH] = "";
static char		SQLITEWhere[256];
static sqlite3	*SQLITEHandle=0;
static LONGLONG	NextSQLITERec = 0, SQLITEBaseRefno = 0;
static MNMXCORD SQLITEFileMNMX;
static sqlite3_stmt *statement = NULL;
static char		cmd[1024];
static char		SQLiteErrorFile[MAX_PATH] = "";

#define BLOB_MAX	USHRT_MAX
#define COORDINATE_FACTOR	10000000
#define INPUTBUFSIZE USHRT_MAX * 32

void SetSQLiteErrFile(LPSTR errFile)
{
	if (errFile)
		strcpy(SQLiteErrorFile, errFile);
	else
		*SQLiteErrorFile = 0;
}

BOOL SQLOK(int sqlReturn, sqlite3* database, char *method, char ** error)
{
	if (sqlReturn != SQLITE_OK)
	{
		char mess[4096];
		if (*method)
			sprintf(mess, "SQLite Error %i = %i:%s\nin:%s",
			sqlReturn, sqlite3_errcode(database), sqlite3_errmsg(database), method);
		else
			sprintf(mess, "SQLite Error %i = %i:%s",
			sqlReturn, sqlite3_errcode(database), sqlite3_errmsg(database));

		if (*SQLiteErrorFile)
			AppendFile(SQLiteErrorFile, mess);
		else
			GSSiMessageBox(3, mess, "SQLite Error", MB_OK, 0);
	}

	return sqlReturn;
}

BOOL SQLOK2(int sqlReturn, sqlite3* database, char *method, char*cmd, char ** error)
{
	if (sqlReturn != SQLITE_OK)
	{
		char * mess = malloc(USHRT_MAX * 4);
		if (method && *method)
			sprintf(mess, "SQLite Error %i = %i:%s\nin:%s",
			sqlReturn, sqlite3_errcode(database), sqlite3_errmsg(database), method);
		else
			sprintf(mess, "SQLite Error %i = %i:%s",
			sqlReturn, sqlite3_errcode(database), sqlite3_errmsg(database));

		if (cmd && *cmd)
			sprintf(strchr(mess, 0), "\n\n%s", cmd);
		GSSiMessageBox(3, mess, "SQLite Error", MB_OK, 0);
		free(mess);
	}

	return sqlReturn;
}


static int maxID(sqlite3 *_database)
{
	int rtn = -1;
	char query[256];
	
	sprintf(query, "SELECT max(id) FROM OFFENSEXY");

	sqlite3_stmt *statement = NULL;

	SQLOK(sqlite3_prepare_v2(_database, query, -1, &statement, 0), _database, "maxID", 0);
	if (sqlite3_step(statement) == SQLITE_ROW)
	{
		rtn = sqlite3_column_int(statement, 0);
	}
	SQLOK(sqlite3_finalize(statement), _database,"maxID",0);
	return rtn;
}

BOOL SLT_StartTrans(sqlite3* db)
{
	BOOL rtn = FALSE;
	if (db)
	{
		int err = SQLOK(sqlite3_exec(db, "BEGIN", NULL, NULL, 0), db, "", 0);
		if (!err)
		{
			rtn = TRUE;
		}
	}
	return rtn;
}
BOOL SLT_EndTrans(sqlite3* db)
{
	BOOL rtn = FALSE;
	if (db)
	{
		int err = SQLOK(sqlite3_exec(db, "COMMIT", NULL, NULL, 0), db, "", 0);
		if (!err)
		{
			rtn = TRUE;
		}
	}
	return rtn;
}
BOOL SLT_Execute(LPSTR cmd,sqlite3* db)
{
	BOOL rtn = FALSE;
	if (db)
	{
		int err = SQLOK(sqlite3_exec(db, cmd, NULL, NULL, 0), db, "", 0);
		if (!err)
		{
			rtn = TRUE;
		}
	}
	return rtn;
}
BOOL SLT_Vacuum(sqlite3* db)
{
	BOOL rtn = FALSE;
	if (db)
	{
		int err = SQLOK(sqlite3_exec(db, "VACUUM", NULL, NULL, 0), db, "", 0);
		if (!err)
		{
			rtn = TRUE;
		}
	}
	return rtn;
}
static int idForCnum(int cnum, int seq, sqlite3 *_database)
{
	int rtn = -1;
	char query[256];
	sprintf (query,"SELECT id FROM OFFENSEXY WHERE ControlNbr = %li AND OffenseOrder = %i", (long)cnum, seq);
	sqlite3_stmt *statement = NULL;

	SQLOK(sqlite3_prepare_v2(_database, query, -1, &statement, 0), _database, "id for cnum", 0);
	if (sqlite3_step(statement) == SQLITE_ROW)
	{
		rtn = sqlite3_column_int(statement, 0);
	}
	SQLOK(sqlite3_finalize(statement), _database, "id for cnum", 0);
	if (rtn < 0)
	{
		rtn = maxID(_database) + 1;
	}

	return rtn;
}

static void getIdForUpdate(LPSTR str, BOOL remove, sqlite3 *_database)
{
	static char fromString[32] = { 0 }, toString[32] = { 0 };

	int cnum;
	int  seq;

	LPSTR pLoc = strstr(str, "OFFENSEXY VALUES(|");
	if (pLoc)
	{
		pLoc += 18;
		LPSTR pEnd = strchr(pLoc, '|');
		if (pEnd)
		{
			*pEnd = 0;
			strncpy(fromString, pLoc - 1, sizeof(fromString));
			strcat(fromString, "|");
			*pEnd++ = '|';
			pLoc = pEnd + 1;
			pEnd = strchr(pLoc, '\'');
			*(pEnd - 1) = 0;
			sscanf(pLoc, "%i,%i", &cnum, &seq);
			*(pEnd - 1) = ',';
			int idnum = cnum;
			if (!remove)
				idnum = idForCnum(cnum,seq,_database);
			sprintf(toString, "%i", idnum);
			REPLAC(str, fromString, toString, INPUTBUFSIZE);
		}

	}
	else
	{
		REPLAC(str, fromString, toString, INPUTBUFSIZE);
	}
}

static LPSTR  DPointsToBlob(HPDPOINT pPoints, int nPnts)
{
	LPSTR pBlob = malloc(nPnts * sizeof(DPOINT)+4);

	return pBlob;
}
static LPSTR  PointsToBlob(HPPOINT pPoints, int nPnts)
{
	LPSTR pBlob = malloc(nPnts * 2 * sizeof(POINT)+4);
	int i, lBlob = 0;

	for (i = 0; i < nPnts; i++)
	{
		int j;
		LPBYTE pInts = (LPBYTE)&pPoints[i];
		unsigned char c;
		for (j = 0; j < 8; j++, pInts++, lBlob += 2)
		{
			c = *pInts;
			sprintf(&pBlob[lBlob], "%2.2x", *pInts);
			//itoa(*pInts, &pBlob[lBlob], 16);
		}
	}
	pBlob[lBlob] = 0;
	return pBlob;
}
static LPSTR  BytesToBlob(LPBYTE pBytes, int nBytes)
{
	LPSTR pBlob = malloc(nBytes * 2 +4);
	int i, lBlob = 0;

	for (i = 0; i < nBytes; i++,lBlob+=2)
	{
		sprintf(&pBlob[lBlob], "%2.2x", pBytes[i]);
	}
	pBlob[lBlob] = 0;
	return pBlob;
}

static LPSTR removePCT(LPSTR name)
{
	static char newName[80];
	LPSTR pChar = name;
	LPSTR pOutChar = newName;

	while (*pChar)
	{
		if (*pChar != '%')
			*pOutChar++ = *pChar;
		pChar++;
	}
	*pOutChar = 0;
	return newName;
}
static int ConvertOffsetsToIDs(LPINT pOffsets, LPGWDHEADER lpGWDHead)
{
	int i;
	int ln = 0;

	for (i = 0; i < 100; i++)
	{
		if (pOffsets[i] > 0)
		{
			FillGWDData(lpGWDHead, pOffsets[i]);
			pOffsets[i] = *(LPINT)&lpGWDHead->GWDData;
			ln = max(ln, (i + 1)*sizeof(int));
		}
	}
	return ln;
}

LONGLONG GetSQLITENumRows(sqlite3 *db,LPSTR tableName,LPSTR where)
{
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX);
	LPSTR  pCmd = GlobalLock(hCmd);
	if (*where)
		sprintf(pCmd, "SELECT COUNT (*) FROM %s WHERE %s", tableName,where);
	else
		sprintf(pCmd, "SELECT COUNT (*) FROM %s", tableName);
	sqlite3_stmt *statement;
	LONGLONG rtn=0;

	if (db)
	{
		if (SQLOK(sqlite3_prepare_v2(db, pCmd, -1, &statement, 0), db, "get num rows", 0) == SQLITE_OK)
		{
			if (sqlite3_step(statement) == SQLITE_ROW)
			{
				rtn = sqlite3_column_int(statement, 0);
			}
		}
		sqlite3_finalize(statement);
	}
	GSSiGlobUlFree(&hCmd);
	return rtn;
}
BOOL GetSQLITEBounds(sqlite3 *db, LPSTR tableName,LPMNMXCORD pfileMNMX)
{
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX);
	LPSTR  pCmd = GlobalLock(hCmd);
	sqlite3_stmt *statement;
	BOOL rtn = FALSE;

	DBoundsInit(pfileMNMX);
	if (db)
	{
		switch (SQLITEIndexType)
		{
		case INDEX_TYPE_RTREE:
			sprintf(pCmd, "SELECT min(minX),max(maxX),min(minY),max(maxY) FROM %s_index", tableName);
			SQLOK(sqlite3_prepare_v2(db, pCmd, -1, &statement, 0), db, "get num rows", 0);

			if (sqlite3_step(statement) == SQLITE_ROW)
			{
				pfileMNMX->xmn = sqlite3_column_double(statement, 0);
				pfileMNMX->xmx = sqlite3_column_double(statement, 1);
				pfileMNMX->ymn = sqlite3_column_double(statement, 2);
				pfileMNMX->ymx = sqlite3_column_double(statement, 3);
				rtn = TRUE;
			}
			SQLOK(sqlite3_finalize(statement), db, "get num rows", NULL);
			break;

		case INDEX_TYPE_XY:
			break;
		}
	}
	GSSiGlobUlFree(&hCmd);
	return rtn;
}

int SQLiteCmd(int nArgs, LPSTR *ARG)
{
	int rtn = 0;
	sqlite3 *db;

	if (!stricmp(ARG[1], "OPEN"))
	{
		rtn = sqlite3_open(ARG[2], &db);
		if (rtn == SQLITE_OK)
		{
			SetGlobalValueLong(ARG[3], (UINT)db);
			rtn = 1;
		}
	}
	else if (!stricmp(ARG[1], "CLOSE"))
	{
		db = (sqlite3*)atoi(ARG[2]);
		rtn = sqlite3_close(db);
	}
	else if (!stricmp(ARG[1], "STARTTRANS"))
	{
		db = (sqlite3*)atoi(ARG[2]);
		if (db)
		{
			int err = SQLOK(sqlite3_exec(db, "BEGIN", NULL, NULL, 0), db, "", 0);
			if (!err)
			{
				rtn = 1;
			}
		}
	}
	else if (!stricmp(ARG[1], "ENDTRANS"))
	{
		char *error = NULL;
		db = (sqlite3*)atoi(ARG[2]);
		int err = SQLOK(sqlite3_exec(db, "COMMIT", NULL, NULL, 0), db, "", 0);
		if (!err)
		{
			rtn = 1;
		}
	}
	else if (!stricmp(ARG[1], "VACUUM"))
	{
		char *error = NULL;
		db = (sqlite3*)atoi(ARG[2]);
		int err = SQLOK(sqlite3_exec(db, "VACUUM", NULL, NULL, 0), db, "", 0);
		if (!err)
		{
			rtn = 1;
		}
	}
	else if (!stricmp(ARG[1], "SPATIALINDEX"))
	{
		char *error = NULL;
		db = (sqlite3*)atoi(ARG[2]);
		if (!stricmp(ARG[3], "CREATE"))
		{
			rtn = SLTSpatialIndexCreate(db, ARG[4]);
		}
		else if (!stricmp(ARG[3], "ADD"))//$SQLITE(SPATIALINDEX,DBHANDLE,ADD,tableName,itemnumber,itemname,bounds)
		{
			BOOL err;
			LONGLONG id = _atoi64(ARG[5]);
			MNMXCORD bounds = atobounds(ARG[7], &err);
			if (!err)
				rtn = SLTSpatialIndexAdd(db, ARG[4], id, ARG[6], &bounds);
		}
	}
	else if (!stricmp(ARG[1], "SPATIALINDEX3D"))
	{
		char *error = NULL;
		db = (sqlite3*)atoi(ARG[2]);
		if (!stricmp(ARG[3], "CREATE"))
		{
			rtn = SLTSpatialIndexCreate3D(db, ARG[4]);
		}
		else if (!stricmp(ARG[3], "ADD"))//$SQLITE(SPATIALINDEX,DBHANDLE,ADD,tableName,itemnumber,itemname,bounds)
		{
			BOOL err;
			LONGLONG id = _atoi64(ARG[5]);
			MNMXCORD3D bounds = atobounds3D(ARG[7], &err);
			if (!err)
				rtn = SLTSpatialIndexAdd3D(db, ARG[4], id, ARG[6], &bounds);
		}
	}
	else if (!stricmp(ARG[1], "CMDFROMFILE"))//$SQLITE(CMDFROMFILE,dbhandle,infile,displaystatus,convertINSERT INTO to INSERT OR REPLACE,skiperrors)
	{
#define MAXSTR 1024 * 1024 * 16
		char *error = NULL;
		HFILE fid = GSSiOpenFile(ARG[3], 0, OF_READ);
		BOOL displayStatus = atob(ARG[4]);
		BOOL convertInsertInto = atob(ARG[5]);
		BOOL skipErrors = atob(ARG[6]);
		int totLen;
		int line = 1;

		if (strstr(ARG[3], "34850-2"))
			ii = 1;
		db = (sqlite3*)atoi(ARG[2]);
		if (fid != HFILE_ERROR)
		{
			HANDLE hstr = GSSiGlobAlloc(0, GMEM_MOVEABLE, MAXSTR);
			LPSTR cmd = GlobalLock(hstr);
			int totLen = GSSifilelength(fid);
			int curPos = 0;
			BOOL keepGoing = TRUE;

			if (displayStatus)
				CreateStatusWind(hWndMain,totLen,"Load SQLite");
			rtn = 1;
			while (keepGoing && fgetstring(cmd, -(MAXSTR - 2), fid))
			{
				int err;
				char errLoc[512];

				sprintf(errLoc, "%s line %i", ARG[3], line++);
				//REPLAC(cmd, "/", "//", MAXSTR-2)
				//REPLAC(cmd, "'", "''", MAXSTR - 2);
				if (convertInsertInto)
					REPLAC(cmd, "INSERT INTO", "INSERT OR REPLACE INTO", MAXSTR - 2);
				getIdForUpdate(cmd, FALSE, db);
				err = SQLOK2(sqlite3_exec(db, cmd, NULL, NULL, &error), db,errLoc,cmd, &error);
				sqlite3_free(error);
				if (err && !skipErrors)
				{
					rtn = 0;
					break;
				}
				if (displayStatus)
				{
					curPos = GSSillseek(fid, 0, 1);
					keepGoing = StatusWindowUpdate(0, "", totLen, curPos);
				}
			}
			if (displayStatus)
					DestroyStatusWindow(0);
			GSSiGlobUlFree(&hstr);
			GSSiClose(fid);
		}
	}
	else if (!stricmp(ARG[1], "EXECUTE"))//$SQLITE(EXECUTE,sqlitehandle,cmd)
	{
		db = (sqlite3*)atoi(ARG[2]);
		sqlite3_stmt *statement;

		if (db)
		{
			if (SQLOK(sqlite3_prepare_v2(db, ARG[3], -1, &statement, 0), db, "", 0) == SQLITE_OK)
			{
				if (sqlite3_step(statement) == SQLITE_ROW)
				{
					LPSTR pName = (LPSTR)sqlite3_column_name(statement, 0);
					LPSTR value = (LPSTR)sqlite3_column_text(statement, 0);
					if (*ARG[4])
						SetGlobalValue(ARG[4], value);
					rtn = TRUE;
				}
				sqlite3_finalize(statement);
			}
		}
	}
	else if (!stricmp(ARG[1], "PREPARE"))//$SQLITE(PREPARE,sqlitehandle,cmd)returns statement address or 0
	{
		db = (sqlite3*)atoi(ARG[2]);
		sqlite3_stmt *statement;

		if (db)
		{
			if (SQLOK(sqlite3_prepare_v2(db, ARG[3], -1, &statement, 0), db, "", 0) == SQLITE_OK)
				rtn = (int)statement;
		}
	}
	else if (!stricmp(ARG[1], "STEP"))//$SQLITE(STEP,statement) returns statement address or 0
	{
		sqlite3_stmt *statement = (sqlite3_stmt *)atoi(ARG[2]);
		if (sqlite3_step(statement) == SQLITE_ROW)
			rtn = TRUE;
	}
	else if (!stricmp(ARG[1], "FINALIZE"))//$SQLITE(STEP,statement) returns statement address or 0
	{
		sqlite3_stmt *statement = (sqlite3_stmt *)atoi(ARG[2]);
		if (sqlite3_finalize(statement) == SQLITE_OK)
			rtn = TRUE;
	}
	else if (!stricmp(ARG[1], "COLUMN"))//$SQLITE(COLUMN,statement,icol,globalvarname)
	{
		sqlite3_stmt *statement = (sqlite3_stmt *)atoi(ARG[2]);
		int irow = atoi(ARG[3]);
		LPSTR pval;

		pval = (LPSTR)sqlite3_column_text(statement, irow);
		if (pval)
		{
			if (*ARG[4])
				SetGlobalValue(ARG[4], pval);
			rtn = TRUE;
		}
	}
	else if (!stricmp(ARG[1], "NUMROWS"))//$SQLITE(NUMROWS,sqlitehandle,tablename,where clause)
	{
		db = (sqlite3*)atoi(ARG[2]);
		rtn = GetSQLITENumRows(db, ARG[3],ARG[4]);
	}
	else if (!stricmp(ARG[1], "FROMGMD"))//$SQLITE(FROMGMD,sqlitehandle,gmdfile,tablename,primkeyisoffset)
	{
		HANDLE hGMDB = OpenGWDatabase(ARG[3], BT_READ);
		char *error=NULL;
		BOOL primKeyIsOffset = atob(ARG[5]);
		if (hGMDB)
		{
			LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock(hGMDB);

			db = (sqlite3*)atoi(ARG[2]);
			if (db)
			{ 
				HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX * 8);
				LPSTR  pCmd = GlobalLock(hCmd);
				LPGWFLDINFO lpFieldInfo;
				char delim[2] = { 0 };
				int i;

				sprintf(pCmd, "DROP TABLE IF EXISTS %s", ARG[4]);
				rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "$SQLITE(FROMGMD drop table", &error);
				sqlite3_free(error);

				if (primKeyIsOffset)
					sprintf(pCmd, "CREATE TABLE %s (OFFSET INTEGER PRIMARY KEY,", ARG[4]);
				else
					sprintf(pCmd, "CREATE TABLE %s (", ARG[4]);

				for ( i = 0, lpFieldInfo = lpGWDHead->pFldInfo; i<lpGWDHead->NumFields; i++, lpFieldInfo++)
				{
					switch (lpFieldInfo->Type)
					{
					case BT_CHAR:
						sprintf(strchr(pCmd, 0), "%s'%s' CHAR(%i)", delim, removePCT(lpFieldInfo->Name), lpFieldInfo->Len);
						break;
					case BT_INTEGER:
						sprintf(strchr(pCmd, 0), "%s'%s' INT", delim, removePCT(lpFieldInfo->Name));
						break;
					case BT_REAL:
						sprintf(strchr(pCmd, 0), "%s'%s' REAL", delim, removePCT(lpFieldInfo->Name));
						break;
					default:
						MessageBox(0, "Bad Type", 0, MB_ICONEXCLAMATION);
						rtn = 1;
						break;
					}
					delim[0] = ',';
				}
				if (!rtn)
				{
					int ifield, index;
					int firstIndex = 1;

					if (primKeyIsOffset)
					{
						firstIndex = 0;
						sprintf(strchr(pCmd, 0), ")");
					}
					else
					{
						sprintf(strchr(pCmd, 0), ",PRIMARY KEY('%s' ASC", lpGWDHead->pFldInfo->Name);
						for (ifield = 1, lpFieldInfo = lpGWDHead->pFldInfo + 1; ifield < lpGWDHead->NumIndexFields[0]; ifield++, lpFieldInfo++)
						{
							sprintf(strchr(pCmd, 0), ",'%s' ASC", removePCT(lpFieldInfo->Name));
						}
						sprintf(strchr(pCmd, 0), "))");
					}
					rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "$SQLITE(FROMGMD create table", &error);
					sqlite3_free(error);
					if (!rtn)
						for (index = firstIndex; index < lpGWDHead->NumIndex; index++)
						{
							sprintf(pCmd, "CREATE INDEX %s_Index%i ON %s ('%s' ASC", ARG[4], index + 1, ARG[4], (lpGWDHead->pFldInfo + lpGWDHead->IndexFields[index][0])->Name);
							for (ifield = 1, lpFieldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[index][1]; ifield<lpGWDHead->NumIndexFields[index]; ifield++, lpFieldInfo++)
							{
								sprintf(strchr(pCmd, 0), ",'%s' ASC", removePCT(lpFieldInfo->Name));
							}
							sprintf(strchr(pCmd, 0), ")");
							rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "$SQLITE(FROMGMD create table", &error);
							sqlite3_free(error);
						}
				}

				if (!rtn)
				{
					int pos = BT_FIRST;
					long Offset;
					HANDLE hVal = GSSiGlobAlloc(1797, GMEM_MOVEABLE, 4096);
					LPSTR val = GlobalLock(hVal);
					int nRecs = BT_NUM_IN_INDEX(lpGWDHead->BTHandle[0]);
					int nLoaded = 0;
					
					sprintf(val, "Load table %s", ARG[4]);
					CreateStatusWind(hWndMain, 1, val);
					rtn = SQLOK(sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error), db, "loadIntersectionTextToDatabase2", &error);
					while (!rtn && !BT_FIND(lpGWDHead->BTHandle[0], lpGWDHead->pKeys[0], pos, BT_ANY, (LPSTR)&Offset))
					{
						pos = BT_NEXT;
						int i;
						FillGWDData(lpGWDHead, Offset);
						if (primKeyIsOffset)
							sprintf(pCmd, "INSERT INTO %s VALUES(%i,",ARG[4],Offset);
						else
							sprintf(pCmd, "INSERT INTO %s VALUES(", ARG[4]);
						delim[0] = 0;
						for (i = 0, lpFieldInfo = lpGWDHead->pFldInfo; i < lpGWDHead->NumFields; i++, lpFieldInfo++)
						{
							GMDGetCharFieldVal(lpGWDHead, i, val);
							switch (lpFieldInfo->Type)
							{
							case BT_CHAR:
								REPLAC(val, "'", "''",4096);
								sprintf(strchr(pCmd, 0), "%s'%s'", delim, val);
								break;
							case BT_INTEGER:
							case BT_REAL:
								sprintf(strchr(pCmd, 0), "%s%s", delim, val);
								break;
							}
							delim[0] = ',';
						}
						sprintf(strchr(pCmd, 0), ")");
						rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "$SQLITE(FROMGMD insert record", &error);
						sqlite3_free(error);
						rtn = !StatusWindowUpdate(NULL, NULL, nRecs, ++nLoaded);
					}
					DestroyStatusWindow(0);
					if (!rtn)
					{
						rtn = SQLOK(sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error), db, "$SQLITE(FROMGMD commit transaction", &error);
					}
					else
					{
						SQLOK(sqlite3_exec(db, "ROLLBACK TRANSACTION", NULL, NULL, &error), db, "$SQLITE(FROMGMD rollback transaction", &error);
					}
					//SQLOK(sqlite3_finalize(self.statement), "loadIntersectionTextToDatabase8");
					GSSiGlobUlFree(&hVal);
					sqlite3_close(db);

				}
				GSSiGlobUlFree(&hCmd);
			}
			if (!rtn)
				rtn = 1;
			GlobalUnlock(hGMDB);
			CloseGWDatabase(hGMDB);
		}
	}
	else if (!stricmp(ARG[1], "CRIMESFROMGMD"))//$SQLITE(CRIMESFROMGMD,outfilename,new/update,gmdfile,tablename,point fields(opt),SQLiteDefFile)
	{
		HANDLE hGMDB = 0;
		char *error = NULL;
		BOOL includesPoint = FALSE;
		BOOL haveDateAndUCR = FALSE;
		HFILE fid;
		HFILE fidDef = HFILE_ERROR;
		char TableName[128];
		char DBName[256];
		char fromFile[256] = { 0 };
		HFILE fromFileFid = HFILE_ERROR;
		char fromFileRec[36];
		char SQL[256] = { 0 };
		BOOL convertToLL = FALSE;
		LPSTR llLoc, pBar;
		HANDLE hFldDefs = 0;
		LPSTR fldDefs;
		BOOL update = FALSE;

		if (*ARG[7])
			fidDef = GSSiOpenFile(ARG[7], 0, OF_READ);
		strcpy(DBName, ARG[4]);
		pBar = strrchr(DBName, '|');
		if (pBar)
		{
			*pBar++ = 0;
			strcpy(SQL, pBar);
		}
		hGMDB = OpenGWDatabase(DBName, BT_READ);
		//OpenDataFile(DBName, SQL, BT_READ, &hGMDB);
		strcpy(TableName, ARG[5]);
		llLoc = strstr(TableName, "_LATLON");
		if (llLoc)
		{
			convertToLL = TRUE;
			*llLoc = 0;
		}

		if (*ARG[6])
		{
			includesPoint = TRUE;
			if (strstr(ARG[6], "$CLK"))
				haveDateAndUCR = TRUE;
		}
		if (hGMDB)
		{
			LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock(hGMDB);
			LPGWDHEADER lpGWDOffConv = 0;
			fid = GSSiOpenFile(ARG[2], 0, OF_CREATE);
			if (atob(ARG[3]))
				update = TRUE;
			if (fid != HFILE_ERROR)
			{
				HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX * 8);
				LPSTR  pCmd = GlobalLock(hCmd);
				HANDLE hCmdIndex = GSSiGlobAlloc(1796, GMEM_MOVEABLE, 4096);
				LPSTR  pCmdIndex = GlobalLock(hCmdIndex);
				LPGWFLDINFO lpFieldInfo;
				char delim[2] = { 0 };
				BOOL HaveBeginDate = FALSE;
				BOOL HaveLastChanged = FALSE;
				BOOL HaveLastChangedID = FALSE;
				BOOL HaveCity = FALSE;
				BOOL HaveZipcode = FALSE;
				int  nextId = -1;
				int i;

				GSSillseek(fid, 0, 2);
				char cmd[256];
				sprintf(cmd, "#1=INSERT INTO %s_index VALUES(", TableName);
				//fputstring(cmd, fid);
				sprintf(cmd, "#2=INSERT INTO %s VALUES(", TableName);
				//fputstring(cmd, fid);
				sprintf(pCmd, "DROP TABLE IF EXISTS %s", TableName);
				if (fidDef == HFILE_ERROR && !update)
					fputstring(pCmd, fid);
				if (includesPoint)
				{
					sprintf(pCmd, "DROP TABLE IF EXISTS %s_index", TableName);
					if (fidDef == HFILE_ERROR && !update)
						fputstring(pCmd, fid);
					if (haveDateAndUCR)
						//sprintf(pCmd, "CREATE VIRTUAL TABLE %s_index USING rtree(id,minX, maxX, minY, maxY, minTime, maxTime, minUCR, maxUCR);", TableName);
						sprintf(pCmd, "CREATE VIRTUAL TABLE %s_index USING rtree(id, minTime, maxTime, minUCR, maxUCR,minX, maxX, minY, maxY);", TableName);
					else
						sprintf(pCmd, "CREATE VIRTUAL TABLE %s_index USING rtree(id,minX, maxX, minY, maxY);", TableName);
					if (fidDef == HFILE_ERROR && !update)
						fputstring(pCmd, fid);
					if (lpGWDHead->NumIndexFields[0] == 1)
						strcpy(lpGWDHead->pFldInfo->Name, "id");
				}

				if (lpGWDHead->NumIndexFields[0] > 1)
				{
					//sprintf(pCmd, "CREATE TABLE %s (id INTEGER PRIMARY KEY,", TableName);
					sprintf(pCmd, "CREATE TABLE %s (", TableName);
					//nextId = 1;
					nextId = 0;
				}
				else
					sprintf(pCmd, "CREATE TABLE %s (", TableName);

				for (i = 0, lpFieldInfo = lpGWDHead->pFldInfo; i<lpGWDHead->NumFields; i++, lpFieldInfo++)
				{
					if (!stricmp(lpFieldInfo->Name, "BeginDate"))//fixes mpls incident table
					{
						if (!HaveBeginDate)
							strcpy(lpFieldInfo->Name, "BeginDate2");
						HaveBeginDate = TRUE;
					}
					if (!stricmp(lpFieldInfo->Name, "LastChanged"))//fixes mpls incident table
					{
						if (!HaveLastChanged)
							strcpy(lpFieldInfo->Name, "LastChanged2");
						HaveLastChanged = TRUE;
					}
					if (!stricmp(lpFieldInfo->Name, "LastChangedID"))//fixes mpls incident table
					{
						if (!HaveLastChangedID)
							strcpy(lpFieldInfo->Name, "LastChangedID2");
						HaveLastChangedID = TRUE;
					}
					if (!stricmp(lpFieldInfo->Name, "City"))//fixes mpls incident table
					{
						if (!HaveCity)
							strcpy(lpFieldInfo->Name, "City2");
						HaveCity = TRUE;
					}
					if (!stricmp(lpFieldInfo->Name, "Zipcode"))//fixes mpls incident table
					{
						if (!HaveZipcode)
							strcpy(lpFieldInfo->Name, "Zipcode2");
						HaveZipcode = TRUE;
					}
					switch (lpFieldInfo->Type)
					{
					case BT_CHAR:
						if (!stricmp(lpFieldInfo->Name, "Offsets") && lpFieldInfo->Len == 400)
							sprintf(strchr(pCmd, 0), "%s'%s' BLOB(%i)", delim, removePCT(lpFieldInfo->Name), lpFieldInfo->Len);
						else
							sprintf(strchr(pCmd, 0), "%s'%s' CHAR(%i)", delim, removePCT(lpFieldInfo->Name), lpFieldInfo->Len);
						break;
					case BT_INTEGER:
						sprintf(strchr(pCmd, 0), "%s'%s' INT", delim, removePCT(lpFieldInfo->Name));
						break;
					case BT_REAL:
						sprintf(strchr(pCmd, 0), "%s'%s' REAL", delim, removePCT(lpFieldInfo->Name));
						break;
					default:
						MessageBox(0, "Bad Type", 0, MB_ICONEXCLAMATION);
						rtn = 1;
						break;
					}
					delim[0] = ',';
				}
				if (haveDateAndUCR)
					sprintf(strchr(pCmd, 0), ",SUNANGLE INT");
				if (!rtn)
				{
					int ifield, index;
					int firstIndex = 1;
					int lastIndex = lpGWDHead->NumIndex;

					if (nextId)
					{
						firstIndex = 0;
						sprintf(strchr(pCmd, 0), ")");
					}
					else
					{
						sprintf(strchr(pCmd, 0), ",PRIMARY KEY('%s' ASC", removePCT(lpGWDHead->pFldInfo->Name));
						for (ifield = 1, lpFieldInfo = lpGWDHead->pFldInfo + 1; ifield < lpGWDHead->NumIndexFields[0]; ifield++, lpFieldInfo++)
						{
							sprintf(strchr(pCmd, 0), ",'%s' ASC", removePCT(lpFieldInfo->Name));
						}
						sprintf(strchr(pCmd, 0), "))");
					}
					if (fidDef == HFILE_ERROR && !update)
						fputstring(pCmd, fid);
					for (index = firstIndex; index < lastIndex; index++)
					{
						if (lpGWDHead->SpatialIndex != index)
						{
							sprintf(pCmd, "CREATE INDEX %s_Index%i ON %s ('%s' ASC", TableName, index + 1, TableName, removePCT((lpGWDHead->pFldInfo + lpGWDHead->IndexFields[index][0])->Name));
							for (ifield = 1, lpFieldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[index][1]; ifield < lpGWDHead->NumIndexFields[index]; ifield++, lpFieldInfo++)
							{
								sprintf(strchr(pCmd, 0), ",'%s' ASC", removePCT(lpFieldInfo->Name));
							}
							sprintf(strchr(pCmd, 0), ");");
							if (fidDef == HFILE_ERROR && !update)
								fputstring(pCmd, fid);
						}
					}
				}
				//CREATE UNIQUE INDEX OFFENSEXYC_Index ON OFFENSEXY('ControlNbr', 'OffenseOrder');

				if (fidDef != HFILE_ERROR)
				{
					int i = 0;
					hFldDefs = GSSiGlobAlloc(1805, GMEM_MOVEABLE, SHRT_MAX + 4);
					fldDefs = GlobalLock(hFldDefs);
					while (i++ < 5 && fgetstring(fldDefs, SHRT_MAX, fidDef))
					{
						if (!update)
							fputstring(fldDefs, fid);
					}
					GSSiClose(fidDef);
				}
				if (!rtn)
				{
					int pos = BT_FIRST, cond = BT_ANY;
					long Offset;
					HANDLE hVal = GSSiGlobAlloc(1797, GMEM_MOVEABLE, 4096);
					LPSTR val = GlobalLock(hVal);
					int nRecs = BT_NUM_IN_INDEX(lpGWDHead->BTHandle[0]);
					int nLoaded = 0;
					char xField[128] = { 0 };
					char yField[128] = { 0 };
					DPOINT pt;
					int indx = 0;
					int wantCNUM = -1;

					sprintf(val, "Load table %s", ARG[4]);
					CreateStatusWind(hWndMain, 1, val);
					if (*SQL)
					{
						LPSTR pSpace;
						if (!strnicmp(SQL, "ControlNbr = ", 13))
						{
							pSpace = SQL + 13;
							wantCNUM = atoi(pSpace);
							cond = BT_GE;
							indx = 0;
							memset(lpGWDHead->pKeys[indx], 0, abs(lpGWDHead->lKeys[indx]));
							*(LPINT)lpGWDHead->pKeys[indx] = wantCNUM;
						}
						else if (!strnicmp(SQL, "FILE=", 5))
						{
							pSpace = SQL + 5;
							strcpy(fromFile, pSpace);
							pos = BT_FIRST;
							cond = BT_GE;
							indx = 0;
							memset(lpGWDHead->pKeys[indx], 0, abs(lpGWDHead->lKeys[indx]));
							fromFileFid = GSSiOpenFile(fromFile, 0, OF_READ);
							if (fromFileFid == HFILE_ERROR)
								return 0;
							fgetstring(fromFileRec, 32, fromFileFid);
							wantCNUM = 0;
							*(LPINT)lpGWDHead->pKeys[indx] = wantCNUM;
						}
						else if (!strnicmp(SQL, "LastChanged >", 13))
						{
							pSpace = SQL + 13;
							cond = BT_GE;
							indx = 2;
							memset(lpGWDHead->pKeys[indx], 0, abs(lpGWDHead->lKeys[indx]));
							strncpy(lpGWDHead->pKeys[indx], pSpace, abs(lpGWDHead->lKeys[indx]));
						}
						else
						{
							pSpace = strrchr(SQL, ' ');
							if (!pSpace)
								pSpace = strrchr(SQL, '>');
							pSpace++;
							if (*pSpace == '\'')
								pSpace++;
							if (*LastChr(pSpace) == '\'')
								*LastChr(pSpace) = 0;
							cond = BT_GE;
							indx = 2;
							strncpy(lpGWDHead->pKeys[indx], pSpace, abs(lpGWDHead->lKeys[indx]));
						}
					}
NextCrimeRec:
					while (!rtn && !BT_FIND(lpGWDHead->BTHandle[indx], lpGWDHead->pKeys[indx], pos, cond, (LPSTR)&Offset))
					{
						int id = Offset;
						int sunAngle = 0;
						pos = BT_NEXT;
						cond = BT_ANY;

						if (wantCNUM >= 0 && *(LPINT)lpGWDHead->pKeys[indx] != wantCNUM)
							break;
						FillGWDData(lpGWDHead, Offset);

						if (includesPoint)
						{
							MNMXCORD bounds;
							BOOL err;
							char cCord[256];

							if (nextId > 0)
								id = nextId++;
							else
								id = *(LPINT)&lpGWDHead->GWDData;
							strcpy(pCmd, ARG[6]);
							strupr(pCmd);
							LPSTR pX = strstr(pCmd, " [X");
							if (pX)
							{
								strncpy(cCord, &pX[2], 255);
								LPSTR pEnd = strchr(cCord, ']');
								LPSTR pY;
								if (pEnd)
								{
									*pEnd++ = 0;
									strcpy(xField, cCord);
									pY = strchr(pEnd, '[');
									if (pY)
									{
										*pY++ = 0;
										pEnd = strchr(pY, ']');
										if (pEnd)
										{
											*pEnd = 0;
											strcpy(yField, pY);
										}
									}
								}
							}
							for (i = 0, lpFieldInfo = lpGWDHead->pFldInfo; i < lpGWDHead->NumFields; i++, lpFieldInfo++)
							{
								char testVar[128];

								sprintf(testVar, "[%s]", lpFieldInfo->Name);
								strupr(testVar);
								if (strstr(pCmd, testVar))
								{
									GMDGetCharFieldVal(lpGWDHead, i, val);
									REPLAC(pCmd, testVar, val, 1024);
								}
							}
							ExpandText(pCmd);
							LPSTR xy = strchr(pCmd, ' ');
							xy = strchr(xy + 1, ' ');
							pt = atopt(xy, &err);
							if (convertToLL)
								ConvertCoord(&pt, 1, 2);
							bounds.xmn = pt.x - 0.00000001;
							bounds.xmx = pt.x + 0.00000001;
							bounds.ymn = pt.y - 0.00000001;
							bounds.ymx = pt.y + 0.00000001;
							//ConvertBounds(&bounds, 1, 2); point field must be lat lon
							if (haveDateAndUCR)
							{
								float ftimebeg = 0, ftimeend = 0, fUCR = 0;
								int itime = atoi(pCmd);
								LPSTR pSpace = strchr(pCmd, ' ');
								if (pSpace)
								{
									pSpace++;
									if (*pSpace)
										fUCR = atoi(pSpace);
									else
										fUCR = 0;
									if (fUCR > 100)
										fUCR = 100;
									pSpace = strchr(pSpace, ' ');
									{
										char daynight[256];
										ftimebeg = itime / 1000 - 1;
										sprintf(daynight, "$SUN(ALT, -93.33 45.0, %i)", itime);
										ExpandText(daynight);
										sunAngle = IDNINT(atof(daynight));
										ftimeend = ftimebeg + 2;
										//pSpace = strchr(pSpace, ' ');
									}
								}
								//sprintf(pCmd, "INSERT INTO %s_index VALUES(%i,%.6f,%.6f,%.6f,%.6f,%.0f,%.0f,%.0f,%.0f);", TableName, id, bounds.xmn, bounds.xmx, bounds.ymn, bounds.ymx, fUCR*10.0, fUCR*10.0, ftimebeg, ftimeend);
								sprintf(pCmdIndex, "INSERT OR REPLACE INTO %s_index VALUES(|%i|,%.0f,%.0f,%.0f,%.0f,%.6f,%.6f,%.6f,%.6f);", TableName, id, ftimebeg, ftimeend, fUCR*10.0, fUCR*10.0, bounds.xmn, bounds.xmx, bounds.ymn, bounds.ymx);
							}
							else
								sprintf(pCmdIndex, "INSERT INTO %s_index VALUES(|%i|,%.6f,%.6f,%.6f,%.6f);", TableName, id, bounds.xmn, bounds.xmx, bounds.ymn, bounds.ymx);
						}

						if (nextId > 0)
							sprintf(pCmd, "INSERT OR REPLACE INTO %s VALUES(|%i|,", TableName, id);
							//sprintf(pCmd, "#2%i,", id);
						else
							sprintf(pCmd, "INSERT OR REPLACE INTO %s VALUES(", TableName);
						delim[0] = 0;

						for (i = 0, lpFieldInfo = lpGWDHead->pFldInfo; i < lpGWDHead->NumFields; i++, lpFieldInfo++)
						{
							char searchStr[128];

							if (!stricmp(lpFieldInfo->Name, "Offsets") && lpFieldInfo->Len == 400)
								GMDGetCharFieldVal(lpGWDHead, -i, val);
							else
								GMDGetCharFieldVal(lpGWDHead, i, val);
							if (!i && !stricmp(val, "BOB"))
								ii = 1;
							if (hFldDefs)
							{
								sprintf(searchStr, "'%s'", lpFieldInfo->Name);
								if (!strstr(fldDefs, searchStr))
									continue;
							}
							if (!stricmp(lpFieldInfo->Name, xField))
							{
								sprintf(strchr(pCmd, 0), "%s%f", delim, pt.x);
							}
							else if (!stricmp(lpFieldInfo->Name, yField))
							{
								sprintf(strchr(pCmd, 0), "%s%f", delim, pt.y);
							}
							else switch (lpFieldInfo->Type)
							{
							case BT_CHAR:
								if (!stricmp(lpFieldInfo->Name, "Offsets") && lpFieldInfo->Len == 400)
								{
									int i;
									LPBYTE pByte = (LPBYTE)val;
									LPBYTE pBlob;

									int ln = ConvertOffsetsToIDs((LPINT)pByte, lpGWDOffConv);
									pBlob = BytesToBlob(pByte, ln);

									sprintf(strchr(pCmd, 0), "%sX'%s'", delim, pBlob);
									free(pBlob);
								}
								else
								{
									REPLAC(val, "'", "''", 4096);
									Truncate(val);
									sprintf(strchr(pCmd, 0), "%s'%s'", delim, val);
								}
								break;
							case BT_INTEGER:
							case BT_REAL:
								sprintf(strchr(pCmd, 0), "%s%s", delim, val);
								break;
							}
							delim[0] = ',';
						}
						if (haveDateAndUCR)
							sprintf(strchr(pCmd, 0), "%s%i", delim, sunAngle);
						sprintf(strchr(pCmd, 0), ")");
						fputstring(pCmd, fid);
						fputstring(pCmdIndex, fid);
						rtn = !StatusWindowUpdate(NULL, NULL, nRecs, ++nLoaded);
					}
					if (fromFileFid != HFILE_ERROR)
					{
						if (fgetstring(fromFileRec, 32, fromFileFid))
						{
							LPINT pCnum = (LPINT)lpGWDHead->pKeys[indx];
							wantCNUM = atoi(fromFileRec);
							*pCnum++ = wantCNUM;
							LPSHORT pOrder = (LPSHORT)pCnum;
							*pOrder = 1;
							pos = BT_FIRST;
							cond = BT_GE;
							goto NextCrimeRec;
						}
						GSSiClose(fromFileFid);
					}
					DestroyStatusWindow(0);
					GSSiClose(fid);
					//SQLOK(sqlite3_finalize(self.statement), "loadIntersectionTextToDatabase8");
					GSSiGlobUlFree(&hVal);
				}
				GSSiGlobUlFree(&hCmd);
				GSSiGlobUlFree(&hCmdIndex);
			}
			if (!rtn)
				rtn = 1;
			GlobalUnlock(hGMDB);
			CloseGWDatabase(hGMDB);
			GSSiGlobUlFree(&hFldDefs);
		}
	}

		else if (!stricmp(ARG[1], "TEXTFROMGMD"))//$SQLITE(TEXTFROMGMD,outfilename,new,gmdfile,tablename,primkeyisoffset,point fields(opt),offsetConversionDB(opt),skipFirst(opt))
		{
			HANDLE hGMDB=0;
			char *error = NULL;
			BOOL primKeyIsOffset = atob(ARG[6]);
			BOOL includesPoint = FALSE;
			BOOL haveDateAndUCR = FALSE;
			HFILE fid;
			char TableName[128];
			char DBName[256];
			char SQL[256] = { 0 };
			BOOL convertToLL = FALSE;
			LPSTR llLoc, pBar;
			BOOL skipFirst = atob(ARG[9]);
			int  firstField = 0;
			char testCondition[256] = { 0 };
			BOOL haveUniqueID = FALSE;

			if (skipFirst) //rowid
				firstField = 1;

			strcpy(DBName, ARG[4]);
			pBar = strrchr(DBName, '|');
			if (pBar)
			{
				*pBar++ = 0;
				strcpy(SQL, pBar);
			}
			hGMDB = OpenGWDatabase(DBName, BT_READ);
			//OpenDataFile(DBName, SQL, BT_READ, &hGMDB);
			strcpy(TableName, ARG[5]);
			llLoc = strstr(TableName, "_LATLON");
			if (llLoc)
			{
				convertToLL = TRUE;
				*llLoc = 0;
			}
					 
			if (*ARG[7])
			{
				includesPoint = TRUE;
				if (strstr(ARG[7], "$CLK"))
					haveDateAndUCR = TRUE;
			}
			if (hGMDB)
			{
				LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock(hGMDB);
				LPGWDHEADER lpGWDOffConv=0;

				if (atob(ARG[3]))
					fid = GSSiOpenFile(ARG[2], 0, OF_CREATE);
				else
					fid = GSSiOpenFile(ARG[2], 0, OF_READWRITE);
				if (fid != HFILE_ERROR)
				{
					HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX * 8);
					LPSTR  pCmd = GlobalLock(hCmd);
					LPGWFLDINFO lpFieldInfo;
					char delim[2] = { 0 };
					HANDLE hOffConvDB = 0;
					HFILE  fidOffConv = HFILE_ERROR;
					int  nextId = -1;
					int i;
					BOOL changeFirstFieldToID = FALSE;

					if (*ARG[8])
					{
						hOffConvDB = OpenGWDatabase(ARG[8], BT_READ);
						lpGWDOffConv = (LPGWDHEADER)GlobalLock(hOffConvDB);
					}
					GSSillseek(fid, 0, 2);
					sprintf(pCmd, "DROP TABLE IF EXISTS %s", TableName);
					fputstring(pCmd, fid);
					if (includesPoint)
					{
						sprintf(pCmd, "DROP TABLE IF EXISTS %s_index", TableName);
						fputstring(pCmd, fid);
						if (haveDateAndUCR)
							//sprintf(pCmd, "CREATE VIRTUAL TABLE %s_index USING rtree(id,minX, maxX, minY, maxY, minTime, maxTime, minUCR, maxUCR);", TableName);
							sprintf(pCmd, "CREATE VIRTUAL TABLE %s_index USING rtree(id, minTime, maxTime, minUCR, maxUCR,minX, maxX, minY, maxY);", TableName);
						else
							sprintf(pCmd, "CREATE VIRTUAL TABLE %s_index USING rtree(id,minX, maxX, minY, maxY);", TableName);
						fputstring(pCmd, fid);
						if (lpGWDHead->NumIndexFields[0] == 1 && lpGWDHead->pFldInfo->Type == BT_INTEGER && lpGWDHead->pFldInfo->Len == 4)
						{
							strcpy(lpGWDHead->pFldInfo->Name, "id");
							changeFirstFieldToID = TRUE;
						}
					}

					if (primKeyIsOffset)
					{
						sprintf(pCmd, "CREATE TABLE %s (OFFSET INTEGER PRIMARY KEY,", TableName);
						haveUniqueID = TRUE;
					}
					else if (lpGWDHead->NumIndexFields[0] > 1 || (includesPoint && !changeFirstFieldToID))
					{
						//sprintf(pCmd, "CREATE TABLE %s (id INTEGER PRIMARY KEY,", TableName);
						//nextId = 1;
						sprintf(pCmd, "CREATE TABLE %s (", TableName);
						nextId = 0;
						//haveUniqueID = TRUE;
					}
					else
						sprintf(pCmd, "CREATE TABLE %s (", TableName);

					lpFieldInfo = lpGWDHead->pFldInfo;
					if (skipFirst)
						lpFieldInfo++;
					for (i = firstField; i<lpGWDHead->NumFields; i++, lpFieldInfo++)
					{
						switch (lpFieldInfo->Type)
						{
						case BT_CHAR:
							if (!stricmp(lpFieldInfo->Name, "Offsets") && lpFieldInfo->Len == 400)
								sprintf(strchr(pCmd, 0), "%s'%s' BLOB(%i)", delim, removePCT(lpFieldInfo->Name), lpFieldInfo->Len);
							else
								sprintf(strchr(pCmd, 0), "%s'%s' CHAR(%i)", delim, removePCT(lpFieldInfo->Name), lpFieldInfo->Len);
							break;
						case BT_INTEGER:
							if (!i && nextId < 0)
							{
								sprintf(strchr(pCmd, 0), "%s%s INTEGER PRIMARY KEY", delim, removePCT(lpFieldInfo->Name));
								haveUniqueID = TRUE;
							}
							else
								sprintf(strchr(pCmd, 0), "%s'%s' INT", delim, removePCT(lpFieldInfo->Name));
							break;
						case BT_REAL:
							sprintf(strchr(pCmd, 0), "%s'%s' REAL", delim, removePCT(lpFieldInfo->Name));
							break;
						default:
							MessageBox(0, "Bad Type", 0, MB_ICONEXCLAMATION);
							rtn = 1;
							break;
						}
						delim[0] = ',';
					}
					if (haveDateAndUCR)
						sprintf(strchr(pCmd, 0), ",SUNANGLE INT");
					if (!rtn)
					{
						int ifield, index;
						int firstIndex = 1;
						int lastIndex = lpGWDHead->NumIndex;

						if (primKeyIsOffset || nextId>0 || haveUniqueID)
						{
							firstIndex = 0;
							sprintf(strchr(pCmd, 0), ")");
						}
						else
						{
							sprintf(strchr(pCmd, 0), ",PRIMARY KEY('%s' ASC", removePCT(lpGWDHead->pFldInfo->Name));
							for (ifield = 1, lpFieldInfo = lpGWDHead->pFldInfo + 1; ifield < lpGWDHead->NumIndexFields[0]; ifield++, lpFieldInfo++)
							{
								sprintf(strchr(pCmd, 0), ",'%s' ASC", removePCT(lpFieldInfo->Name));
							}
							sprintf(strchr(pCmd, 0), "))");
						}
						fputstring(pCmd, fid);
						for (index = firstIndex; index < lastIndex; index++)
						{
							if (lpGWDHead->SpatialIndex != index)
							{
								sprintf(pCmd, "CREATE INDEX %s_Index%i ON %s ('%s' ASC", TableName, index + 1, TableName, removePCT((lpGWDHead->pFldInfo + lpGWDHead->IndexFields[index][0])->Name));
								for (ifield = 1, lpFieldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[index][1]; ifield < lpGWDHead->NumIndexFields[index]; ifield++, lpFieldInfo++)
								{
									sprintf(strchr(pCmd, 0), ",'%s' ASC", removePCT(lpFieldInfo->Name));
								}
								sprintf(strchr(pCmd, 0), ");");
								fputstring(pCmd, fid);
							}
						}
					}

					if (!rtn)
					{
						int pos = BT_FIRST, cond = BT_ANY;
						long Offset;
						HANDLE hVal = GSSiGlobAlloc(1797, GMEM_MOVEABLE, 4096);
						LPSTR val = GlobalLock(hVal);
						int nRecs = BT_NUM_IN_INDEX(lpGWDHead->BTHandle[0]);
						int nLoaded = 0;
						char xField[128] = { 0 };
						char yField[128] = { 0 };
						DPOINT pt;
						int indx = 0;
						int wantCNUM = -1;

						sprintf(val, "Load table %s", ARG[4]);
						CreateStatusWind(hWndMain, 1, val);
						if (*SQL)
						{
							LPSTR pSpace;
							if (!strnicmp(SQL, "ControlNbr = ",13))
							{
								pSpace = SQL + 13;
								wantCNUM = atoi(pSpace);
								cond = BT_GE;
								indx = 0;
								memset(lpGWDHead->pKeys[indx], 0, abs(lpGWDHead->lKeys[indx]));
								*(LPINT)lpGWDHead->pKeys[indx] = wantCNUM;
							}
							else
							{
								pSpace = strrchr(SQL, ' ');
								if (!pSpace)
									pSpace = strrchr(SQL, '>');
								if (pSpace)
								{
									pSpace++;
									if (*pSpace == '\'')
										pSpace++;
									if (*LastChr(pSpace) == '\'')
										*LastChr(pSpace) = 0;
									cond = BT_GE;
									indx = 2;
									strncpy(lpGWDHead->pKeys[indx], pSpace, abs(lpGWDHead->lKeys[indx]));
								}
								else
									strcpy(testCondition, SQL);
							}
						}
						while (!rtn && StatusWindowUpdate(NULL, NULL, nRecs, ++nLoaded) && !BT_FIND(lpGWDHead->BTHandle[indx], lpGWDHead->pKeys[indx], pos, cond, (LPSTR)&Offset))
						{
							int id = Offset;
							int sunAngle=0;
							pos = BT_NEXT;
							cond = BT_ANY;
							int i;

							if (wantCNUM >= 0 && *(LPINT)lpGWDHead->pKeys[indx] != wantCNUM)
								break;
							FillGWDData(lpGWDHead, Offset);
							if (*testCondition)
							{
								char exp[512];
								char testfield[64];
								BOOL err;
								LPSTR pEq = strchr(testCondition, '=');
								if (pEq)
								{
									*pEq = 0;
									strcpy(testfield, testCondition);
									*pEq = '=';
									
									lpFieldInfo = lpGWDHead->pFldInfo;
									if (skipFirst)
										lpFieldInfo++;
									for (i = firstField; i < lpGWDHead->NumFields; i++, lpFieldInfo++)
									{
										if (!stricmp(lpFieldInfo->Name, testfield))
										{
											GMDGetCharFieldVal(lpGWDHead, i, exp);
											break;
										}
									}
									sprintf(strchr(exp, 0), "%s", pEq);
									if (!LogicP(exp, &err))
										continue;
								}
							}

							if (includesPoint)
							{
								MNMXCORD bounds;
								BOOL err;

								if (nextId > 0)
									id = nextId++;
								else if (!primKeyIsOffset)
									id = *(LPINT)&lpGWDHead->GWDData;
								strcpy(pCmd, ARG[7]);
								strupr(pCmd);
								if (*pCmd == '[')
								{
									char cCord[256];
									strncpy(cCord, &pCmd[1], 255);
									LPSTR pEnd = strchr(cCord, ']');
									LPSTR pY;
									if (pEnd)
									{
										*pEnd++ = 0;
										strcpy(xField, cCord);
										pY = strchr(pEnd, '[');
										if (pY)
										{
											*pY++ = 0;
											pEnd = strchr(pY, ']');
											if (pEnd)
											{
												*pEnd = 0;
												strcpy(yField, pY);
											}
										}
									}
								}
								lpFieldInfo = lpGWDHead->pFldInfo;
								if (skipFirst)
									lpFieldInfo++;
								for (i = firstField; i < lpGWDHead->NumFields; i++, lpFieldInfo++)
								{
									char testVar[128];

									sprintf(testVar, "[%s]", lpFieldInfo->Name);
									strupr(testVar);
									if (strstr(pCmd, testVar))
									{
										GMDGetCharFieldVal(lpGWDHead, i, val);
										REPLAC(pCmd, testVar, val, 1024);
									}
								}
								ExpandText(pCmd);
								pt = atopt(pCmd, &err);
								if (convertToLL)
									ConvertCoord(&pt, 1, 2);
								bounds.xmn = pt.x - 0.00000001;
								bounds.xmx = pt.x + 0.00000001;
								bounds.ymn = pt.y - 0.00000001;
								bounds.ymx = pt.y + 0.00000001;
								//ConvertBounds(&bounds, 1, 2); point field must be lat lon
								if (haveDateAndUCR)
								{
									float ftimebeg=0, ftimeend = 0,fUCR=0;
									LPSTR pSpace = strchr(pCmd,' ');
									if (pSpace)
									{
										pSpace = strchr(++pSpace, ' ');
										if (pSpace)
										{
											int itime = atoi(++pSpace);
											char daynight[256];
											ftimebeg = itime / 1000 - 1;
											sprintf(daynight, "$SUN(ALT, -93.33 45.0, %i)", itime);
											ExpandText(daynight);
											sunAngle = IDNINT(atof(daynight));
											ftimeend = ftimebeg + 2;
											pSpace = strchr(pSpace, ' ');
											if (pSpace)
											{
												fUCR = atoi(++pSpace);
											}
										}
									}
									/*{
										if (pSpace)
										{
											int itime = atoi(pCmd);
											ftimebeg = itime / 1000 - 1;
											ftimeend = ftimebeg + 2;
											fUCR = atoi(++pSpace);
										}
									}*/
									//sprintf(pCmd, "INSERT INTO %s_index VALUES(%i,%.6f,%.6f,%.6f,%.6f,%.0f,%.0f,%.0f,%.0f);", TableName, id, bounds.xmn, bounds.xmx, bounds.ymn, bounds.ymx, ftimebeg, ftimeend, fUCR*10.0, fUCR*10.0);
									sprintf(pCmd, "INSERT INTO %s_index VALUES(%i,%.0f,%.0f,%.0f,%.0f,%.6f,%.6f,%.6f,%.6f);", TableName, id, ftimebeg, ftimeend, fUCR*10.0, fUCR*10.0, bounds.xmn, bounds.xmx, bounds.ymn, bounds.ymx);
								}
								else
									sprintf(pCmd, "INSERT INTO %s_index VALUES(%i,%.6f,%.6f,%.6f,%.6f);", TableName, id, bounds.xmn, bounds.xmx, bounds.ymn, bounds.ymx);
								fputstring(pCmd, fid);
							}

							if (nextId > 0)
								sprintf(pCmd, "INSERT INTO %s VALUES(%i,", TableName, nextId++);
							else if (primKeyIsOffset)
								sprintf(pCmd, "INSERT INTO %s VALUES(%i,", TableName, Offset);
							else
								sprintf(pCmd, "INSERT INTO %s VALUES(", TableName);
							delim[0] = 0;

							lpFieldInfo = lpGWDHead->pFldInfo;
							if (skipFirst)
								lpFieldInfo++;
							for (i = firstField; i < lpGWDHead->NumFields; i++, lpFieldInfo++)
							{
								if (!stricmp(lpFieldInfo->Name, "Offsets") && lpFieldInfo->Len == 400)
									GMDGetCharFieldVal(lpGWDHead, -i, val);
								else
									GMDGetCharFieldVal(lpGWDHead, i, val);
								if (!i && !stricmp(val, "BOB"))
									ii = 1;
								if (!stricmp(lpFieldInfo->Name, xField))
								{
									sprintf(strchr(pCmd, 0), "%s%f", delim, pt.x);
								}
								else if (!stricmp(lpFieldInfo->Name, yField))
								{
									sprintf(strchr(pCmd, 0), "%s%f", delim, pt.y);
								}
								else switch (lpFieldInfo->Type)
								{
								case BT_CHAR:
									if (!stricmp(lpFieldInfo->Name, "Offsets") && lpFieldInfo->Len == 400)
									{
										int i;
										LPBYTE pByte = (LPBYTE)val;
										LPBYTE pBlob;
										
										int ln = ConvertOffsetsToIDs((LPINT)pByte, lpGWDOffConv);
										pBlob = BytesToBlob(pByte, ln);

										sprintf(strchr(pCmd, 0), "%sX'%s'", delim,pBlob);
										free(pBlob);
									}
									else
									{
										REPLAC(val, "'", "''", 4096);
										REPLAC(val, "\r", "", 4096);
										REPLAC(val, "\n", "", 4096);
										Truncate(val);
										sprintf(strchr(pCmd, 0), "%s'%s'", delim, val);
									}
									break;
								case BT_INTEGER:
								case BT_REAL:
									sprintf(strchr(pCmd, 0), "%s%s", delim, val);
									break;
								}
								delim[0] = ',';
							}
							if (haveDateAndUCR)
								sprintf(strchr(pCmd, 0), "%s%i", delim, sunAngle);
							sprintf(strchr(pCmd, 0), ")");
							fputstring(pCmd, fid);
						}
						DestroyStatusWindow(0);
						GSSiClose(fid);
						//SQLOK(sqlite3_finalize(self.statement), "loadIntersectionTextToDatabase8");
						GSSiGlobUlFree(&hVal);
					}
					GSSiGlobUlFree(&hCmd);
					if (hOffConvDB)
					{
						GlobalUnlock(hOffConvDB);
						CloseGWDatabase(hOffConvDB);
					}
				}
				if (!rtn)
					rtn = 1;
				GlobalUnlock(hGMDB);
				CloseGWDatabase(hGMDB);
			}
		}
		else if (!stricmp(ARG[1], "TEXTFROMPOLY"))//$SQLITE(TEXTFROMPOLY,outfilename,new,tablename,
												  //UDIFieldNameAndType(i.e PID  CHAR(13)-no spaces in name),
												  //UDIFieldNameAndType2(i.e PID  CHAR(13)-no spaces in name),value,
												  //skipquad(TF),skipconvert(TF),add LastUpdate Field
												  //additional field,additional field val,
												  //...
		{
			short	pos = BT_FIRST;
			long	Refno;
			HIGHLIGHTDATA	HighlightData;
			long	nPnts;
			HANDLE	hPoly;
			HANDLE  hPolyPartLen;
			HPDPOINT	pPoints;
			int nLoops;
			LPINT pPartLen;
			HFILE Fid;
			int nRecs = BT_NUM_IN_INDEX(hHighlight), nLoaded = 0;
			int keepGoing = 1;
			int nCanCompress = 0, nTotal = 0;
			LPSTR pSpace;
			BOOL createFile, createTables=TRUE;
			int wantType = 3;
			LPSTR pUS = strrchr(ARG[4], '_');
			BOOL skipQuadIndex = atob(ARG[8]);
			BOOL skipConvert = atob(ARG[9]);
			double coordFactor = COORDINATE_FACTOR;
			double sqMeters;
			double perimeter;
			double sqMetersCVT;
			double perimeterCVT;
			int maxLineLen = 0;

			if (skipConvert)
				coordFactor /= 1000;
			if (pUS)
			{
				if (!stricmp(pUS, "_LINE"))
					wantType = 2;
			}

			if (*ARG[3] == 'A')
			{
				createFile = FALSE;
				createTables = FALSE;
			}
			else if (*ARG[3] == 'B')
			{
				createFile = FALSE;
				createTables = TRUE;
			}
			else
				createFile = atob(ARG[3]);
			if (createFile)
				Fid = GSSiOpenFile(ARG[2], 0, OF_CREATE);
			else
				Fid = GSSiOpenFile(ARG[2], 0, OF_READWRITE);
			//SetGlobalValue("%ALT_PROJECTION", "[%DL]projections\\hencogrnd.cvt");
			SetGlobalValue("%ALT_PROJECTION", "[%DL]projections\\statepln.cvt");
			ConvertCoordClose();
			ConvertCoordInit();

			if (Fid != HFILE_ERROR)
			{
				HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX * 64*2);
				LPSTR  pCmd = GlobalLock(hCmd);

				GSSillseek(Fid, 0, 2);
				sprintf(pCmd, "Extract Table %s", ARG[4]);
				CreateStatusWind(hWndMain, 1,pCmd);
				if (createTables)
				{
					char addFields[256] = {0};
					
					if (atob(ARG[10]))
						strcpy(addFields, ",LASTUPDATE INT");
					for (int i = 11; i < 16; i+=2)
					{
						if (!*ARG[i])
							break;
						sprintf(strchr(addFields, 0), ",%s", ARG[i]);
					}
					sprintf(pCmd, "DROP TABLE IF EXISTS %s;", ARG[4]);
					fputstring(pCmd, Fid);maxLineLen = max(maxLineLen,strlen(pCmd));
					sprintf(pCmd, "DROP TABLE IF EXISTS %s_index;", ARG[4]);
					fputstring(pCmd, Fid);maxLineLen = max(maxLineLen,strlen(pCmd));

					if (!skipQuadIndex)
					{
						sprintf(pCmd, "CREATE VIRTUAL TABLE %s_index USING rtree(id,minX, maxX, minY, maxY);", ARG[4]);
						fputstring(pCmd, Fid);maxLineLen = max(maxLineLen,strlen(pCmd));
					}
					if (*ARG[6])
						sprintf(pCmd, "CREATE TABLE %s (id INTEGER PRIMARY KEY,%s,%s%s,BasePointX REAL,BasePointY REAL,NumPoints INT,NumLoops INT,PolyPartLen BLOB(%i), Points BLOB(%i),Area DOUBLE,Perimeter DOUBLE);", ARG[4], ARG[5], ARG[6],addFields, BLOB_MAX, BLOB_MAX * 8);
					else
						sprintf(pCmd, "CREATE TABLE %s (id INTEGER PRIMARY KEY,%s%s,BasePointX REAL,BasePointY REAL,NumPoints INT,NumLoops INT,PolyPartLen BLOB(%i), Points BLOB(%i),Area DOUBLE,Perimeter DOUBLE);", ARG[4], ARG[5], addFields,BLOB_MAX, BLOB_MAX * 8);
					fputstring(pCmd, Fid);maxLineLen = max(maxLineLen,strlen(pCmd));
					if ((pSpace = strchr(ARG[5], ' ')))
						*pSpace = 0;
					sprintf(pCmd, "CREATE INDEX %s%s_Index ON %s ('%s' ASC);", ARG[4],ARG[5], ARG[4], ARG[5]);
					fputstring(pCmd, Fid);maxLineLen = max(maxLineLen,strlen(pCmd));
					if (*ARG[6])
					{
						if ((pSpace = strchr(ARG[6], ' ')))
							*pSpace = 0;
						sprintf(pCmd, "CREATE INDEX %s%s_Index ON %s ('%s' ASC)", ARG[4], ARG[6], ARG[4], ARG[6]);
						fputstring(pCmd, Fid);maxLineLen = max(maxLineLen,strlen(pCmd));
					}
				}

				while (keepGoing && !BT_FIND(hHighlight, (LPSTR)&Refno, pos, BT_ANY, (LPSTR)&HighlightData))
				{
					char UDI[80];
					char Arg7Val[256];
					char addFieldVals[1024] = { 0 };

					if (Refno == 80002608)
						ii = 1;
					if (atob(ARG[10]))
						strcpy(addFieldVals, ",0");
					for (int i = 11; i < 16; i += 2)
					{
						if (!*ARG[i])
							break;
						if (strstr(ARG[i], "CHAR("))
							sprintf(strchr(addFieldVals, 0), ",'%s'", ARG[i + 1]);
						else
							sprintf(strchr(addFieldVals, 0), ",%s", ARG[i + 1]);
					}

					strcpy(Arg7Val, ARG[7]);
					if (!stricmp(Arg7Val, "[UDI]"))
					{
						strcpy(Arg7Val, HighlightData.PD.UDI);
						REPLAC(Arg7Val, "'", "''", 80);
					}
					if (!stricmp(ARG[5], "SYMBOLNAME"))
					{
						GetSymbolName(HighlightData.PD.Desc, UDI, 0, 0, 0);
					}
					else
					{
						strcpy(UDI, HighlightData.PD.UDI);
						REPLAC(UDI, "'", "''", 80);
					}
					pos = BT_NEXT;
					if (HighlightData.PD.Type == wantType && strlen(UDI)>0)
					{
						if ((nLoops = GetPolyPointsWithParts((LPPICKDATAHEADER)&HighlightData.PD, &nPnts, &hPoly, &hPolyPartLen)))
						{
							LPMNMXCORD	pBounds = (LPMNMXCORD)GlobalLock(hPoly);
							DPOINT midPt;
							HPDPOINT pDPoints, pPointsCVT;
							HPPOINT  pPoints;
							LPSTR blobPoints,blobParts;
							HANDLE hPoints, hPointsCVT;
							BOOL canCompress=TRUE;
							int  np = nPnts;
							int  nLops;
							double maxd = 0;
							double diffArea, diffPerim;

							if (!skipConvert)
								ConvertBounds(pBounds, 1, 2);
							sprintf(pCmd, "INSERT INTO %s_index VALUES(%i,%.6f,%.6f,%.6f,%.6f);", ARG[4], Refno, pBounds->xmn, pBounds->xmx, pBounds->ymn, pBounds->ymx);
							if (!skipQuadIndex)
								fputstring(pCmd, Fid);maxLineLen = max(maxLineLen,strlen(pCmd));
							if (nLoops > 1)
							{
								pPartLen = GlobalLock(hPolyPartLen);
								pPartLen++;//first is npoly
								blobParts = (LPSTR)BytesToBlob((LPBYTE)pPartLen, nLoops*sizeof(int));
								GlobalUnlock(hPolyPartLen);
							}
							pDPoints = (HPDPOINT)(pBounds + 1);
							sqMeters = ComputeAreaAreaD(pDPoints, nPnts, &perimeter);
							hPoints = GSSiGlobAlloc(0, GMEM_MOVEABLE, nPnts * sizeof(POINT));
							pPoints = GlobalLock(hPoints);
							hPointsCVT = GSSiGlobAlloc(0, GMEM_MOVEABLE, nPnts * sizeof(DPOINT));
							pPointsCVT = GlobalLock(hPointsCVT);
							midPt = MinMaxMidPointD(pBounds);
							nTotal++;
							for (int i = 0; i < nPnts; i++)
							{
								pPointsCVT[i] = pDPoints[i];
								ConvertCoord(&pPointsCVT[i], 1, 3);
								if (!skipConvert)
									ConvertCoord(&pDPoints[i], 1, 2);
								pPoints[i].x = coordFactor * (pDPoints[i].x - midPt.x);
								pPoints[i].y = coordFactor * (pDPoints[i].y - midPt.y);
								if (pPoints[i].x > SHRT_MAX || pPoints[i].x < SHRT_MIN || pPoints[i].y > SHRT_MAX || pPoints[i].y < SHRT_MIN)
									canCompress = FALSE;
							}
							sqMetersCVT = ComputeAreaAreaD(pPointsCVT, nPnts, &perimeterCVT);
							diffArea = 100 * sqMetersCVT / sqMeters;
							diffPerim = 100 * perimeterCVT / perimeter;
							if (diffArea > 101 || diffArea < 99 || diffPerim > 101 || diffPerim < 99)
								ii = 1;
							sqMeters = sqMetersCVT;
							perimeter = perimeterCVT;
							if (canCompress)
							{
								LPPOINTS pShortPoints = malloc(nPnts*sizeof(POINTS)+4);
								for (int i = 0; i<nPnts; i++)
								{
									pShortPoints[i].x = pPoints[i].x;
									pShortPoints[i].y = pPoints[i].y;
								}
								blobPoints = (LPSTR)BytesToBlob((LPBYTE)pShortPoints, nPnts*sizeof(POINTS));
								free(pShortPoints);
								np = -nPnts;
							}
							else
							{
								LPSTR googlestr;
								int googleln, blobln;
								//char str[256];
								LPDPOINT pNewPt = malloc(sizeof(DPOINT)*nPnts + 4);
								maxd = 0;
								blobPoints = PointsToBlob(pPoints, nPnts);
								blobln = strlen(blobPoints);

								/*for (i = 0; i < nPnts; i++)
								{
									sprintf(str, "%f\t%f", pDPoints[i].x, pDPoints[i].y);
									AppendFile("c:\\temp\\compress.txt", str);
								}*/
								googlestr = EncodeString(pDPoints, nPnts);
								//AppendFile("c:\\temp\\compress.txt", googlestr);
								googleln = strlen(googlestr);
								pNewPt = DecodeString(googlestr, &nPnts);
								for (int i = 0; i < nPnts; i++)
								{
									double dist = ArcDistance(pDPoints[i], pNewPt[i]);
									if (dist > 10)
										ii = 1;
									maxd = max(dist, maxd);
								}
								/*for (i = 0; i < nPnts; i++)
								{
									sprintf(str, "%f\t%f", pNewPt[i].x, pNewPt[i].y);
									AppendFile("c:\\temp\\compress.txt", str);
								}
								ii = 1;*/
								free(googlestr);
								free(pNewPt);
							}
							nLops = nLoops;
							if (skipConvert)
								nLops = -nLoops;
							if (nLoops > 1)
							{
								if (*ARG[7])
									sprintf(pCmd, "INSERT INTO %s VALUES(%i,'%s','%s',%.8f,%.8f,%i,%i,X'%s',X'%s',%f,%f);", ARG[4], Refno,UDI,Arg7Val, midPt.x, midPt.y, np, nLops, blobParts, blobPoints,sqMeters,perimeter);
								else
									sprintf(pCmd, "INSERT INTO %s VALUES(%i,'%s',%.8f,%.8f,%i,%i,X'%s',X'%s',%f,%f);", ARG[4], Refno, UDI, midPt.x, midPt.y, np, nLops, blobParts, blobPoints, sqMeters, perimeter);
								free(blobParts);
							}
							else
							{
								if (*ARG[7])
									sprintf(pCmd, "INSERT INTO %s VALUES(%i,'%s','%s'%s,%.8f,%.8f,%i,%i,X'',X'%s',%f,%f);", ARG[4], Refno, UDI, Arg7Val, addFieldVals, midPt.x, midPt.y, np, nLops, blobPoints, sqMeters, perimeter);
								else
									sprintf(pCmd, "INSERT INTO %s VALUES(%i,'%s'%s,%.8f,%.8f,%i,%i,X'',X'%s',%f,%f);", ARG[4], Refno, UDI, addFieldVals, midPt.x, midPt.y, np, nLops, blobPoints, sqMeters, perimeter);
							}
							fputstring(pCmd, Fid);maxLineLen = max(maxLineLen,strlen(pCmd));
							free(blobPoints);
							GSSiGlobUlFree(&hPoints);
							GSSiGlobUlFree(&hPointsCVT);
							GSSiGlobUlFree(&hPoly);
							GSSiGlobFree(&hPolyPartLen);
							if (canCompress)
								nCanCompress++;
						}
					}
					sprintf(pCmd, "Can compress %i (%.1f%%)", nCanCompress, (100.0*nCanCompress) / nTotal);
					keepGoing = StatusWindowUpdate(NULL,pCmd, nRecs, ++nLoaded);
					DestroySavedPolys();
				}
				DestroyStatusWindow(0);
				sprintf(pCmd, "/* maxLineLen=%i */", maxLineLen);
				fputstring(pCmd, Fid);
				GSSiClose(Fid);
				GSSiGlobUlFree(&hCmd);
			}
		}

		else if (!stricmp(ARG[1], "GMMOBILE"))//$SQLITE(GMMOBILE, outfile, datadeffile, dupfile, cenblockfile)
		{
			short	pos = BT_FIRST;
			long	Refno;
			HIGHLIGHTDATA	HighlightData;
			long	nPnts;
			HANDLE	hPoly;
			HANDLE  hPolyPartLen;
			HPDPOINT	pPoints;
			int nLoops;
			LPINT pPartLen;
			HFILE Fid;
			int nRecs = BT_NUM_IN_INDEX(hHighlight), nLoaded = 0;
			int keepGoing = 1;
			int nCanCompress = 0, nTotal = 0;
			LPSTR pSpace;
			BOOL createFile=TRUE, createTables = TRUE;
			int wantType = 3;
			double coordFactor = COORDINATE_FACTOR;
			char TableName[] = "PARCELAREA";

			if (createFile)
				Fid = GSSiOpenFile(ARG[2], 0, OF_CREATE);
			else
				Fid = GSSiOpenFile(ARG[2], 0, OF_READWRITE);
			if (Fid != HFILE_ERROR)
			{
				HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX * 32);
				LPSTR  pCmd = GlobalLock(hCmd);

				GSSillseek(Fid, 0, 2);
				sprintf(pCmd, "Extract Table %s", ARG[4]);
				CreateStatusWind(hWndMain, 1, pCmd);
				if (createTables)
				{
					sprintf(pCmd, "DROP TABLE IF EXISTS %s;", ARG[4]);
					fputstring(pCmd, Fid);
					sprintf(pCmd, "DROP TABLE IF EXISTS %s_index;", ARG[4]);
					fputstring(pCmd, Fid);

					{
						sprintf(pCmd, "CREATE VIRTUAL TABLE %s_index USING rtree(id,minX, maxX, minY, maxY);", ARG[4]);
						fputstring(pCmd, Fid);
					}
					sprintf(pCmd, "CREATE TABLE %s (id INTEGER PRIMARY KEY,PIN,BasePointX REAL,BasePointY REAL,NumPoints INT,NumLoops INT,PolyPartLen BLOB(%i), Points BLOB(%i), Data BLOB(%i));", TableName, BLOB_MAX, BLOB_MAX * 8, BLOB_MAX);
					fputstring(pCmd, Fid);
					sprintf(pCmd, "CREATE INDEX %sPIN_Index ON %s ('PIN' ASC);", TableName, TableName);
					fputstring(pCmd, Fid);
				}

				while (keepGoing && !BT_FIND(hHighlight, (LPSTR)&Refno, pos, BT_ANY, (LPSTR)&HighlightData))
				{
					char UDI[80];
					char Arg7Val[256];
					char addFieldVals[1024] = { 0 };

					if (Refno == 80002608)
						ii = 1;
					if (atob(ARG[10]))
						strcpy(addFieldVals, ",0");
					for (int i = 11; i < 16; i += 2)
					{
						if (!*ARG[i])
							break;
						if (strstr(ARG[i], "CHAR("))
							sprintf(strchr(addFieldVals, 0), ",'%s'", ARG[i + 1]);
						else
							sprintf(strchr(addFieldVals, 0), ",%s", ARG[i + 1]);
					}

					strcpy(Arg7Val, ARG[7]);
					if (!stricmp(Arg7Val, "[UDI]"))
					{
						strcpy(Arg7Val, HighlightData.PD.UDI);
						REPLAC(Arg7Val, "'", "''", 80);
					}
					if (!stricmp(ARG[5], "SYMBOLNAME"))
					{
						GetSymbolName(HighlightData.PD.Desc, UDI, 0, 0, 0);
					}
					else
					{
						strcpy(UDI, HighlightData.PD.UDI);
						REPLAC(UDI, "'", "''", 80);
					}
					pos = BT_NEXT;
					if (HighlightData.PD.Type == wantType && strlen(UDI)>0)
					{
						if ((nLoops = GetPolyPointsWithParts((LPPICKDATAHEADER)&HighlightData.PD, &nPnts, &hPoly, &hPolyPartLen)))
						{
							LPMNMXCORD	pBounds = (LPMNMXCORD)GlobalLock(hPoly);
							DPOINT midPt;
							HPDPOINT pDPoints;
							HPPOINT  pPoints;
							LPSTR blobPoints, blobParts;
							HANDLE hPoints;
							BOOL canCompress = TRUE;
							int  np = nPnts;
							int  nLops;
							double maxd = 0;

							//if (!skipConvert)
								ConvertBounds(pBounds, 1, 2);
							sprintf(pCmd, "INSERT INTO %s_index VALUES(%i,%.6f,%.6f,%.6f,%.6f);", ARG[4], Refno, pBounds->xmn, pBounds->xmx, pBounds->ymn, pBounds->ymx);
							//if (!skipQuadIndex)
								fputstring(pCmd, Fid);
							if (nLoops > 1)
							{
								pPartLen = GlobalLock(hPolyPartLen);
								pPartLen++;//first is npoly
								blobParts = (LPSTR)BytesToBlob((LPBYTE)pPartLen, nLoops*sizeof(int));
								GlobalUnlock(hPolyPartLen);
							}
							pDPoints = (HPDPOINT)(pBounds + 1);
							hPoints = GSSiGlobAlloc(0, GMEM_MOVEABLE, nPnts * sizeof(POINT));
							pPoints = GlobalLock(hPoints);
							midPt = MinMaxMidPointD(pBounds);
							nTotal++;
							for (int i = 0; i < nPnts; i++)
							{
								//if (!skipConvert)
									ConvertCoord(&pDPoints[i], 1, 2);
								pPoints[i].x = coordFactor * (pDPoints[i].x - midPt.x);
								pPoints[i].y = coordFactor * (pDPoints[i].y - midPt.y);
								if (pPoints[i].x > SHRT_MAX || pPoints[i].x < SHRT_MIN || pPoints[i].y > SHRT_MAX || pPoints[i].y < SHRT_MIN)
									canCompress = FALSE;
							}
							if (canCompress)
							{
								LPPOINTS pShortPoints = malloc(nPnts*sizeof(POINTS) + 4);
								for (int i = 0; i<nPnts; i++)
								{
									pShortPoints[i].x = pPoints[i].x;
									pShortPoints[i].y = pPoints[i].y;
								}
								blobPoints = (LPSTR)BytesToBlob((LPBYTE)pShortPoints, nPnts*sizeof(POINTS));
								free(pShortPoints);
								np = -nPnts;
							}
							else
							{
								LPSTR googlestr;
								int googleln, blobln;
								//char str[256];
								LPDPOINT pNewPt = malloc(sizeof(DPOINT)*nPnts + 4);
								maxd = 0;
								blobPoints = PointsToBlob(pPoints, nPnts);
								blobln = strlen(blobPoints);

								/*for (i = 0; i < nPnts; i++)
								{
								sprintf(str, "%f\t%f", pDPoints[i].x, pDPoints[i].y);
								AppendFile("c:\\temp\\compress.txt", str);
								}*/
								googlestr = EncodeString(pDPoints, nPnts);
								//AppendFile("c:\\temp\\compress.txt", googlestr);
								googleln = strlen(googlestr);
								pNewPt = DecodeString(googlestr, &nPnts);
								for (int i = 0; i < nPnts; i++)
								{
									double dist = ArcDistance(pDPoints[i], pNewPt[i]);
									if (dist > 10)
										ii = 1;
									maxd = max(dist, maxd);
								}
								/*for (i = 0; i < nPnts; i++)
								{
								sprintf(str, "%f\t%f", pNewPt[i].x, pNewPt[i].y);
								AppendFile("c:\\temp\\compress.txt", str);
								}
								ii = 1;*/
								free(googlestr);
								free(pNewPt);
							}
							nLops = nLoops;
							//if (skipConvert)
								nLops = -nLoops;
							if (nLoops > 1)
							{
								if (*ARG[7])
									sprintf(pCmd, "INSERT INTO %s VALUES(%i,'%s','%s',%.8f,%.8f,%i,%i,X'%s',X'%s');", ARG[4], Refno, UDI, Arg7Val, midPt.x, midPt.y, np, nLops, blobParts, blobPoints);
								else
									sprintf(pCmd, "INSERT INTO %s VALUES(%i,'%s',%.8f,%.8f,%i,%i,X'%s',X'%s');", ARG[4], Refno, UDI, midPt.x, midPt.y, np, nLops, blobParts, blobPoints);
								free(blobParts);
							}
							else
							{
								if (*ARG[7])
									sprintf(pCmd, "INSERT INTO %s VALUES(%i,'%s','%s'%s,%.8f,%.8f,%i,%i,X'',X'%s');", ARG[4], Refno, UDI, Arg7Val, addFieldVals, midPt.x, midPt.y, np, nLops, blobPoints);
								else
									sprintf(pCmd, "INSERT INTO %s VALUES(%i,'%s'%s,%.8f,%.8f,%i,%i,X'',X'%s');", ARG[4], Refno, UDI, addFieldVals, midPt.x, midPt.y, np, nLops, blobPoints);
							}
							fputstring(pCmd, Fid);
							free(blobPoints);
							GSSiGlobUlFree(&hPoints);
							GSSiGlobUlFree(&hPoly);
							GSSiGlobFree(&hPolyPartLen);
							if (canCompress)
								nCanCompress++;
						}
					}
					sprintf(pCmd, "Can compress %i (%.1f%%)", nCanCompress, (100.0*nCanCompress) / nTotal);
					keepGoing = StatusWindowUpdate(NULL, pCmd, nRecs, ++nLoaded);
					DestroySavedPolys();
				}
				DestroyStatusWindow(0);
				GSSiClose(Fid);
				GSSiGlobUlFree(&hCmd);
			}
		}
		else if (!stricmp(ARG[1], "TEXTFROMPOINT"))//$SQLITE(TEXTFROMPOINT,outfilename,new,tablename,UDIFieldNameAndType(i.e PID  CHAR(13)-no spaces in name),UDIFieldNameAndType2(i.e PID  CHAR(13)-no spaces in name)
		{
			short	pos = BT_FIRST;
			long	Refno;
			HIGHLIGHTDATA	HighlightData;
			HFILE Fid;
			int nRecs = BT_NUM_IN_INDEX(hHighlight), nLoaded = 0;
			int keepGoing = 1;
			LPSTR pSpace;

			if (atob(ARG[3]))
				Fid = GSSiOpenFile(ARG[2], 0, OF_CREATE);
			else
				Fid = GSSiOpenFile(ARG[2], 0, OF_READWRITE);
			if (Fid != HFILE_ERROR)
			{
				HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX * 8);
				LPSTR  pCmd = GlobalLock(hCmd);

				GSSillseek(Fid, 0, 2);
				sprintf(pCmd, "Extact Table %s", ARG[4]);
				CreateStatusWind(hWndMain, 1, pCmd);
				sprintf(pCmd, "DROP TABLE IF EXISTS %s", ARG[4]);
				fputstring(pCmd, Fid);
				sprintf(pCmd, "DROP TABLE IF EXISTS %s_index", ARG[4]);
				fputstring(pCmd, Fid);

				sprintf(pCmd, "CREATE VIRTUAL TABLE %s_index USING rtree(id,minX, maxX, minY, maxY);", ARG[4]);
				fputstring(pCmd, Fid);
				sprintf(pCmd, "CREATE TABLE %s (id INTEGER PRIMARY KEY,%s,LONGITUDE REAL,LATITUDE REAL);", ARG[4],ARG[5]);
				fputstring(pCmd, Fid);
				if ((pSpace = strchr(ARG[5], ' ')))
					*pSpace = 0;
				sprintf(pCmd, "CREATE INDEX %s%s_Index ON %s ('%s' ASC);", ARG[4], ARG[5], ARG[4], ARG[5]);
				fputstring(pCmd, Fid);
				while (keepGoing && !BT_FIND(hHighlight, (LPSTR)&Refno, pos, BT_ANY, (LPSTR)&HighlightData))
				{
					char UDI[80];
					if (!stricmp(ARG[5], "SYMBOLNAME"))
					{
						GetSymbolName(HighlightData.PD.Desc, UDI, 0, 0, 0);
					}
					else
					{
						strcpy(UDI, HighlightData.PD.UDI);
						REPLAC(UDI, "'", "''", 80);
					}
					pos = BT_NEXT;
					if (HighlightData.PD.Type == 1)
					{
						LPMNMXCORD	pBounds = &HighlightData.PD.Rect;
						DPOINT pt;

						ConvertBounds(pBounds, 1, 2);
						sprintf(pCmd, "INSERT INTO %s_index VALUES(%i,%.6f,%.6f,%.6f,%.6f);", ARG[4], Refno, pBounds->xmn, pBounds->xmx, pBounds->ymn, pBounds->ymx);
						fputstring(pCmd, Fid);
						pt = HighlightData.PD.BeginPoint;
						ConvertCoord(&pt, 1, 2);
						sprintf(pCmd, "INSERT INTO %s VALUES(%i,'%s',%.8f,%.8f);", ARG[4], Refno, UDI, pt.x,pt.y);
						fputstring(pCmd, Fid);
					}
					keepGoing = StatusWindowUpdate(NULL, NULL, nRecs, ++nLoaded);
				}
				DestroyStatusWindow(0);
				GSSiClose(Fid);
				GSSiGlobUlFree(&hCmd);
			}
		}
		return rtn;
}

int SQLITEOpenPrj(LPSTR SQLITEFileName, int projectionID)
{
	char	prjFileName[MAX_PATH] = "";
	LPSTR	pDot;
	HFILE	fid;
	int		rtn = 0;

	if (!GetGlobalLVal2("[%USEOSRLIB]", FALSE))
		return 0;
	return 0;
/*	strcpy(prjFileName, SQLITEFileName);
	ExpandText(prjFileName);
	if ((pDot = strrchr(prjFileName, '.')))
	{
		strcpy(pDot, ".prj");
		fid = GSSiOpenFile(prjFileName, 0, OF_READ);
		if (fid != HFILE_ERROR)
		{
			int len = GSSifilelength(fid);
			HANDLE hMem = GSSiGlobAlloc(0, GMEM_MOVEABLE, len + 1);
			LPSTR pMem = GlobalLock(hMem);
			HANDLE hDef = GSSiGlobAlloc(0, GMEM_MOVEABLE, 4096);
			LPSTR proj4def = GlobalLock(hDef);

			BigRead(fid, pMem, len);
			pMem[len] = 0;
			GSSiClose(fid);
			if (!ConvertPRJtoProj4(pMem, proj4def))
			{
				LoadProjection(projectionID, proj4def);
				rtn = 1;
			}
			GSSiGlobUlFree(&hDef);
			GSSiGlobUlFree(&hMem);
		}
	}*/
	return rtn;
}

int OpenSQLITEMapFile(LPSTR FileNameIN, LPMNMXCORD pFileMNMX)
{
	int	i;
	MNMXCORD	FileMNMX, Bounds;
	DPOINT		Points[4];
	int rtnType = 0;
	LPSTR pPar, pEnd;
	char fileName[MAX_PATH], tableName[100];

	DBoundsInit(&FileMNMX);
	CloseTRANS2(&hTranFileToBase);
	CloseTRANS2(&hTranBaseToFile);
	CloseTRANS2(&hTranFileToVP);
	strcpy(fileName, FileNameIN);
	ExpandText(fileName);
	pPar = strrchr(fileName, '(');
	if (pPar)
	{
		*pPar++ = 0;
		if ((pEnd = strchr(pPar, ')')))
		{
			*pEnd = 0;
			strcpy(tableName, pPar);
			if (GSSiLength(fileName) > 0)
			{
				rtnType = SHPT_POINT;
				LoadSQLITEParm(fileName, rtnType, CurView->hWnd);
				if (sqlite3_open(fileName, &SQLITEHandle) == SQLITE_OK)
				{
					if (GetSQLITENumRows(SQLITEHandle, tableName,""))
					{
						if (GetSQLITEBounds(SQLITEHandle, tableName, &SQLITEFileMNMX))
						{
							Points[0].x = ClipCoordToProjection(SQLITEFileMNMX.xmn, 1, 0, 1);
							Points[0].y = ClipCoordToProjection(SQLITEFileMNMX.ymn, 2, 0, 1);
							Points[1].x = ClipCoordToProjection(SQLITEFileMNMX.xmn, 1, 0, 1);
							Points[1].y = ClipCoordToProjection(SQLITEFileMNMX.ymx, 2, 0, 1);
							Points[2].x = ClipCoordToProjection(SQLITEFileMNMX.xmx, 1, 0, 1);
							Points[2].y = ClipCoordToProjection(SQLITEFileMNMX.ymx, 2, 0, 1);
							Points[3].x = ClipCoordToProjection(SQLITEFileMNMX.xmx, 1, 0, 1);
							Points[3].y = ClipCoordToProjection(SQLITEFileMNMX.ymn, 2, 0, 1);
							for (i = 0; i < 4; i++)
							{
								if (ConvertCoord(&Points[i], 0, 1))
								{
									MessageBox(GetFocus(), "Unable to convert coordinates as specified", 0, MB_ICONQUESTION | MB_OK);
									sqlite3_close(SQLITEHandle);
									SQLITEHandle = 0;
									return FALSE;
								}
								AddDPointToMinMax(&Points[i], &FileMNMX);
							}
							if (FileMNMX.xmx - FileMNMX.xmn >
								FileMNMX.ymx - FileMNMX.ymn)
							{
								MinMax.xmn = -32000;
								MinMax.xmx = 32000;
								MinMax.ymn = -32000 * ((FileMNMX.ymx - FileMNMX.ymn) / (FileMNMX.xmx - FileMNMX.xmn));
								MinMax.ymx = -MinMax.ymn;
							}
							else
							{
								MinMax.ymn = -32000;
								MinMax.ymx = 32000;
								MinMax.xmn = -32000 * ((FileMNMX.xmx - FileMNMX.xmn) / (FileMNMX.ymx - FileMNMX.ymn));
								MinMax.xmx = -MinMax.xmn;
							}
							CreateFileTran(&MinMax, &FileMNMX);
							Bounds = CurView->WBounds;
							ConvertBounds(&Bounds, 1, 0);
							sprintf(cmd, "SELECT ALLEYWALLS_NEW.id, [Wall Id],LONGITUDE,LATITUDE FROM ALLEYWALLS_NEW,ALLEYWALLS_NEW_index WHERE ALLEYWALLS_NEW.Current=1 AND ALLEYWALLS_NEW.id=ALLEYWALLS_NEW_index.id AND maxX>=%f AND minX<=%f AND maxY>=%f AND minY<=%f",
								Bounds.xmn, Bounds.xmx, Bounds.ymn, Bounds.ymx);

							if (sqlite3_prepare_v2(SQLITEHandle, cmd, -1, &statement, 0) != SQLITE_OK)
								statement = NULL;
						}
					}
				}
			}
		}
	}
	if (pFileMNMX)
		*pFileMNMX = FileMNMX;
	return rtnType;
}

BOOL GetSQLITERecordBounds(LONGLONG Recno, LPMNMXCORD pBounds)
{
	BOOL rtn = FALSE;
	sprintf(cmd, "SELECT LONGITUDE,LATITUDE FROM ALLEYWALLS_NEW WHERE ALLEYWALLS_NEW.id=%ld",Recno);
	
	if (statement)
		sqlite3_finalize(statement);
	statement = 0;
	if (sqlite3_prepare_v2(SQLITEHandle, cmd, -1, &statement, 0) == SQLITE_OK)
	{
		int st = sqlite3_step(statement);

		if (st == SQLITE_ROW)
		{
			DPOINT BasePt;

			BasePt.x = sqlite3_column_double(statement, 0);
			BasePt.y = sqlite3_column_double(statement, 1);
			ConvertCoord(&BasePt, 0, 1);
			pBounds->xmn = BasePt.x - 1;
			pBounds->ymn = BasePt.y - 1;
			pBounds->xmx = BasePt.x + 1;
			pBounds->ymx = BasePt.y + 1;
			rtn = TRUE;
		}
		sqlite3_finalize(statement);
	}
	statement = NULL;
	return rtn;
}

BOOL GetSQLITERecord(LONGLONG SQLITERec)
{
	BOOL rtn = FALSE;

	sprintf(cmd, "SELECT ALLEYWALLS_NEW.id, [Wall Id],LONGITUDE,LATITUDE FROM ALLEYWALLS_NEW WHERE ALLEYWALLS_NEW.id=%ld", SQLITERec);

	if (sqlite3_prepare_v2(SQLITEHandle, cmd, -1, &statement, 0) != SQLITE_OK)
		statement = NULL;
	else
	{
		int st = sqlite3_step(statement);

		if (st == SQLITE_ROW)
			rtn = TRUE;
		else
		{
			sqlite3_finalize(statement);
			statement = 0;
		}
	}
	return rtn;
}

void CloseSQLITEMapFile(void)
{
	/*GSSiClose(SHPFid);
	OpenSHPFileIndex(0, HFILE_ERROR);*/
	if (SQLITEHandle)
	{
		if (statement)
			sqlite3_finalize(statement);
		statement = 0;
		sqlite3_close(SQLITEHandle);
		SQLITEHandle = 0;
	}
	return;
}

BOOL SetSQLITEVis(HWND hWndDlg, int DlgItemSym, int DlgItemPar, HFILE FidSymList)
{
	char	str[128];
	short	idesc;

	strcpy(str, SQLITESymbol);
	ExpandText(str);
	idesc = atol(str);
	AddSymToList(hWndDlg, DlgItemSym, DlgItemPar, idesc, FidSymList);
	return TRUE;
}
BOOL SetSQLITEParms(void)
{
	LPSTR	pDesc, pTAG, pClause, pC, pColor, pWidth, pRot;
	BOOL	rc;
	char	str[1024];
	BOOL	rtn = FALSE;

	strcpy(str, SQLITERefno);
	ExpandText(str);
	CurrentRefno = atol(str);
	if (!SQLITEHandle)
		return FALSE;
	strcpy(str, SQLITESize);
	ExpandText(str);
	SQLITEPointSize = atol(str);
	SetUseOnlyOneDBHandle(hSHPDBF);
	pTAG = SQLITETAG;
	if (*pTAG && (pC = _fstrchr(pTAG, ':')))
	{
		_fstrcpy(SQLITETag, pTAG);
		ExpandText(SQLITETag);
		pC = _fstrchr(SQLITETag, ':');
		*pC++ = 0;
		strncpy0(CurrentPrefix, SQLITETag, MAX_PREFIX_LEN);
		strncpy0(CurrentUDI, pC--, MAX_UDI_LEN);
		*pC = ':';
		ExpandText(CurrentUDI);
	}
	else
	{
		*SQLITETag = 0;
		*CurrentPrefix = 0;
		*CurrentUDI = 0;
	}
	if (HaveSQLITESym < 0)
	{
/*		LPOPENSQLDATA	SQLPtr = (LPOPENSQLDATA)GlobalLock(SQLITEHandle);
		LPOPENFILEDATA	FilePtr = (LPOPENFILEDATA)GlobalLock(SQLPtr->OFHandle);

		pDesc = SQLITEParms;
		while (*pDesc)
		{
			pClause = _fstrchr(pDesc, 0) + 1;
			pColor = _fstrchr(pClause, 0) + 1;
			pWidth = _fstrchr(pColor, 0) + 1;
			pRot = _fstrchr(pWidth, 0) + 1;
			ConvertSQLToLogicP(str, pClause);
			if (!*pClause || LogicPFile(SQLPtr, str, &rc))
			{
				break;
			}
			else
				pDesc = _fstrchr(pRot, 0) + 1;
		}
		_fstrcpy(str, pDesc);
		ExpandText(str);
		CurrentDesc = GetDictSymbolNumber(str);
		if (!*pDesc)
			goto Exit;
		if (*pColor)
		{
			_fstrcpy(str, pColor);
			ExpandText(str);
			SQLITEColor = ConvertColor(atol(str), CurrentDesc);
		}
		else
			SQLITEColor = -1;
		if (*pWidth)
		{
			_fstrcpy(str, pWidth);
			ExpandText(str);
			SQLITEPointSize = atol(str);
			switch (*LastChr(str))
			{
			case 'P':
			case 'p':
				SQLITEPointSize = -SQLITEPointSize;
				break;
			case 'F':
			case 'f':
				SQLITEPointSize *= FTM;
				break;
			}
			CurPointSize = SQLITEPointSize;
		}
		else
			SQLITEPointSize = 0;
		GlobalUnlock(SQLPtr->OFHandle);
		GlobalUnlock(SQLITEHandle);*/
	}
	else
		CurrentDesc = HaveSQLITESym;
	if (*SQLITEBeginDate)
	{
		_fstrcpy(str, SQLITEBeginDate);
		ExpandText(str);
		GRStartTime = GREndTime = atol(str);
	}
	if (*SQLITEEndDate)
	{
		_fstrcpy(str, SQLITEEndDate);
		ExpandText(str);
		GREndTime = atol(str);
	}
	rtn = TRUE;
Exit:
	SetUseOnlyOneDBHandle(0);
	return rtn;
}

BOOL ProcessSQLITERecord(HDC hDC)
{
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	LPGWDHEADER lpGWDHead;
	BOOL	rtn = FALSE;
	char	str[128], Prefix[10], UDI[64];
	long	Refno;
	short	Symnum;
	int		st;
	LPVIEWPORT	SaveVP = CurView;
	DPOINT BasePt;
	MNMXCORD	RecordBounds;

	//	if (CurView->DisplayInParent && CurView->Parent)            	
	//		SetViewport(CurView->Parent);
	if (CurView->PassID == 2 || !SQLITEHandle)
		goto RtnFalse;
	InitRecord(hDC);
	SetSQLITEParms();
	strcpy(str, SQLITESymbol);
	ExpandText(str);
	CurrentDesc = atol(str);
	if (!GetVisibility(CurrentDesc))
		goto RtnFalse;
	if (*SQLITEWhere)
	{
		BOOL irc;

		if (!LogicP(SQLITEWhere, &irc))
			goto RtnFalse;
	}
	BasePt.x = sqlite3_column_double(statement, 2);
	BasePt.y = sqlite3_column_double(statement, 3);
	rtn = TRUE;
	/*	SQLPtr = (LPOPENSQLDATA)GlobalLock(GMDHandle);
	FilePtr = (LPOPENFILEDATA)GlobalLock(SQLPtr->OFHandle);
	lpGWDHead = (LPGWDHEADER)GlobalLock(FilePtr->FileHandle);
	FillGWDData(lpGWDHead, Offset);
	switch (lpGWDHead->SpatialIndexType)
	{
	case 1:
	case 2:
		GRStartTime = GMDGetIntegerFieldVal(lpGWDHead, lpGWDHead->FromDateField);
		GREndTime = GMDGetIntegerFieldVal(lpGWDHead, lpGWDHead->ToDateField);
		GMDPoint.x = GMDGetRealFieldVal(lpGWDHead, lpGWDHead->XField);
		GMDPoint.y = GMDGetRealFieldVal(lpGWDHead, lpGWDHead->YField);
		break;
	}
	GlobalUnlock(FilePtr->FileHandle);
	GlobalUnlock(SQLPtr->OFHandle);
	GlobalUnlock(GMDHandle);
	if (WantGMDNegGrid)
	{
		if (!GMDPoint.x)
		{
			SelectClipRgn(CurView->hDC, 0);
			GMDPoint = SubVPMidPointWorld;
		}
		else
			goto RtnFalse;
	}*/
	ConvertCoord(&BasePt, 0, 1);
	InGraphicsProcessor = TRUE;
	ShowValue(hDC, FALSE);
	CurrentRefno = sqlite3_column_int(statement, 0);
	ItemSeg = CurrentSQLITERec = CurrentRefno;
	strcpy(str, SQLITERefno);
	ExpandText(str);
	SQLITEBaseRefno = atol(str);
	CurrentRefno += SQLITEBaseRefno;
	PTRot = 0;
	if (PointInWBounds(&BasePt))// && GRStartTime >= TimeRangeBeg && SQLITEStartTime < TimeRangeEnd)
	{
		LPSTR	pTag;
		char	Tag[80];
		short	ltag;
		short	Dummy;
		MNMXCORD bounds;

		//strcpy(Tag, SQLITETAG);
		//ExpandText(Tag);

		pTag = (LPSTR)sqlite3_column_text(statement, 1);
		sprintf (Tag,"ALLYWALL:%s", pTag);
		SetSymNum(CurrentDesc);
		ltag = _fstrlen(Tag);
		if (ProcessRefAndTAG(TRUE, Tag, ltag))
		{
			HiPrecis = TRUE;
			lpDCurPoints = &BasePt;
			CurrentPoint = CurPointLocD = BasePt;
			ItemSeg = CurrentSQLITERec;
			LastElementBeginPoint = LastElementEndPoint = BasePt;
			nPnts = nCurPoints = 1;
			CurPointLoc = BasePtToWinPt(lpDCurPoints);
			if (PointIsBlocked(&CurPointLocD, CurrentDesc))
				goto RtnFalse;
			InGraphicsProcessor = TRUE;
			HaveTXLoc = TRUE;
			CurrentType = GF_POINT;
			CurPointSize = SQLITEPointSize;
			if (CurPointSize < 0)
				CurPointSize = -CurPointSize * DeviceToScreenFactor();
			else
				CurPointSize /= CurView->BaseUnitsPerPixel;
			CurPointSize *= GraphicsPointFactor;
			DBoundsInit(&bounds);
			AddDPointToMinMax(lpDCurPoints, &bounds);
			CurrentItemMinMax = WBoundsToFileBounds(&bounds);
			if ((Pick || PickingByRefno) && GetTypeVisibility(TYPE_POINT))
			{
				CurrentSeg = CurrentRefno;
				PickPointItemD(lpDCurPoints, (CurPointSize*ThemeWidthFactor)*CurView->BaseUnitsPerPixel, PTRot, CurrentDesc);
			}
			else if (GetTypeVisibility(TYPE_POINT))
			{
				short	iDesc = CurrentDesc;

				if (CurrentDesc > 0 && CurrentDesc < 3201)
				{
					if (TSize)
						CurView->CurVisType[CurrentDesc] = 5;
					else
						CurView->CurVisType[CurrentDesc] = 4;
				}
				HighlightPointSym = FALSE;
				if (!GetTypeVisibility(6) && SymbolIsVisible(iDesc))
				{
					CurPointSize = 10 * DeviceToScreenFactor();
					iDesc = InvisiblePointSymbol;
				}
				if (SetDisplayChar(CurView->hDC, GF_POINT, CurrentRefno, CurrentDesc, CurrentPrefix, CurrentUDI) > 0)
				{
					double	size;

					if (ThemePointSym)
					{
						iDesc = ThemePointSym;
						if (ThemePointSize < 0)
							size = -ThemePointSize *DeviceToScreenFactor();
						else
							size = ThemePointSize / CurView->BaseUnitsPerPixel;
						size *= ThemeWidthFactor;
						size = min(max(size*GraphicsPointFactor, 1), MaxPointSize);
					}
					else if (ItemSymbolWidth > 0)
						size = ItemSymbolWidth * CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
					else if (ItemSymbolWidth < 0)
						size = -ItemSymbolWidth * BaseDistToWinDist * CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
					else
						size = CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
					if (iDesc < 0)
					{
						COLORREF	OldColor;

						if (ThemePointColor > -1)
							OldColor = SetTextColor(CurView->hDC, ConvertColor(ThemePointColor, ThemePointUseHalfTone));
						DisplayCharAtLoc(CurView->hDC, CurPointLoc, (short)IDNINT(size), -iDesc);
						if (ThemePointColor > -1)
							SetTextColor(CurView->hDC, OldColor);
					}
					else
					{
						long	DisplayedWidth = 0;

						DisplayPointItem(CurView->hDC, CurPointLoc, size, PTRot, iDesc, &DisplayedWidth);
						CurView->MaxSymbolWidth = max(CurView->MaxSymbolWidth, DisplayedWidth);
						CurView->MaxFileDisplayedPointWidth[FileNum] = max(CurView->MaxFileDisplayedPointWidth[FileNum], (DisplayedWidth / FileDistToWinDist) - (((long)CurrentItemMinMax.xmx) - CurrentItemMinMax.xmn));
					}
					DBoundsInit(&RecordBounds);
					AddDPointToMinMax(lpDCurPoints, &RecordBounds);
					InflateBounds(&RecordBounds, size);
					GetFileMinMax(&CurrentItemMinMax, &RecordBounds);
				}
			}
			InGraphicsProcessor = FALSE;
			TXLoc = CurPointLocD;
			HaveTXLoc = 1;
		}
	}
	ShowValue(hDC, FALSE);
RtnFalse:
	CurView = SaveVP;
	InGraphicsProcessor = FALSE;
	return rtn;
}
BOOL GetNextSQLITERecord(LPMNMXCORD pBounds)
{
	char *error = NULL;
	int rtn = sqlite3_step(statement);

	if (rtn == SQLITE_ROW)
		return TRUE;
	if (rtn == SQLITE_DONE)
		return FALSE;
	SQLOK(rtn, SQLITEHandle,"", &error);
	sqlite3_free(error);
	return FALSE;
/*	if (!hDGN)
		return FALSE;
	if (pBounds)
	{
		MNMXCORD	Bounds = *pBounds;

		if (IgnoreBounds)
			Bounds.xmn = Bounds.ymn = Bounds.xmx = Bounds.ymx = 0;
		pBounds = &Bounds;
		if (_fmemcmp(pBounds, &DGNLastBounds, sizeof(MNMXCORD)))
		{
			MNMXCORD	DGNBounds;

			if (pBounds->xmn || pBounds->xmx || pBounds->ymn || pBounds->ymx)
			{
				if (ConvertRectCoord(&DGNBounds, pBounds, 1, 0))
					DGNLibSetSpatialFilter(hDGN, &DGNBounds);
			}
			else
				DGNLibSetSpatialFilter(hDGN, pBounds);
			DGNLastBounds = *pBounds;
		}
	}
	pElement = (LPDGNElementCore)GlobalLock(hElement);
	rtn = DGNLibReadElement(hDGN, pElement, MaxDGNElementSize, &BaseDistToWinDist, &FillColor, &NumAttributes, Attributes);
	CurrentDGNRec = pElement->element_id;
	GlobalUnlock(hElement);*/
}
BOOL IsSQLITEFileVisible(void)
{
/*	LPSTR	pDesc, pClause, pC, pColor, pWidth, pRot;
	short	idesc, i;
	char	str[128];
	//	return TRUE;
	if (NumIndexSyms)
	{
		for (i = 0; i<NumIndexSyms; i++)
			if (GetVisibility(IndexSyms[i]))
				return TRUE;
	}
	if (!*SHPParms)
		return TRUE;
	pDesc = SHPParms;
	while (*pDesc)
	{
		pClause = _fstrchr(pDesc, 0) + 1;
		pColor = _fstrchr(pClause, 0) + 1;
		pWidth = _fstrchr(pColor, 0) + 1;
		pRot = _fstrchr(pWidth, 0) + 1;
		_fstrcpy(str, pDesc);
		ExpandText(str);
		idesc = GetDictSymbolNumber(str);
		if (GetVisibility(idesc))
			return TRUE;
		pDesc = _fstrchr(pRot, 0) + 1;
	}*/
	return TRUE;
}
BOOL LoadSQLITEParm(LPSTR SQLITEFileName, long Type, HWND hWnd)
#if ENABLETRACE
{
	GSSiEnterProg(1374);
#endif
	{
		
		char	Name[MAX_PATH], str[260], Projection[MAX_PATH + 2], Units[34];
		char	SymName[66]="", cWidth[64]="", cRot[64]="", cColor[64]="", cIF[128]="";
		LPSTR	pDot, pTAG, pWidth, pParm = SQLITEParms;
		short	l;
		int		itype;
		HFILE	Fid;
		BOOL	FileIsIndex;
		BOOL	havePrj = FALSE;
		struct _stati64    statParmFile;

		if (!SQLITEFileName)
		{
			HaveIndexParmFile = FALSE;
			goto RtnTrue;
		}
		*SQLITEBeginDate = 0;
		*SQLITEEndDate = 0;
		SQLITEParmTime = 0;
		NumSQLITEParms = 0;
		SQLITEProjectionIsBase = TRUE;
		_fmemset(SQLITEParms, 0, sizeof(SQLITEParms));
		switch (Type)
		{
		case SHPT_POINT:
		case SHPT_POINTZ:
			GetGlobalCVal("[%DefaultSQLITEPointSymbol]", SQLITEParms, "CIRCLE");
			pWidth = _fstrchr(SQLITEParms, 0) + 4;
			GetGlobalCVal("[%DefaultSQLITEPointSize]", pWidth, "-5");
			break;

		case SHPT_ARC:
		case SHPT_ARCZ:
		case SHPT_ARCM:
			//case shapePolylineM:
			//case shapePolylineZM:
		case shapePolylineZ:
			GetGlobalCVal("[%DefaultSQLITELineSymbol]", SQLITEParms, "PEN1");
			break;

		case 4://personalgeodb area type???
		case SHPT_TEXT:
		case SHPT_POLYGON:
		case SHPT_POLYGONM:
		case SHPT_POLYGONZ:
		case SHPT_PGDB_POLYGONZ:
			//case shapePolygonM:
			//case shapePolygonZM:
			//case shapePolygonZ:
			GetGlobalCVal("[%DefaultSQLITEAreaSymbol]", SQLITEParms, "PARCEL");
			break;
		}
		if (!SQLITEOpenPrj(SQLITEFileName, 0))
		{
			GetGlobalCVal("[%DefaultSQLITEProjection]", Projection, "LATLONG");
			LoadProjection(0, Projection);
		}
		else
			havePrj = TRUE;
		GetGlobalCVal("[%DefaultSQLITEUnits]", Units, "DEGREES");
		if (!_fstricmp(Units, "FEET"))
		{
			PRJ_UNITS[0] = 1;
		}
		else if (!_fstricmp(Units, "METERS"))
			PRJ_UNITS[0] = 2;
		else
			PRJ_UNITS[0] = 4;
		SQLITEBaseRefno = 0;
		*SQLITERefno = 0;
		strcpy(Name, SQLITEFileName);
		pDot = _fstrrchr(Name, '.');
		if (!pDot)
			goto RtnFalse;
		_fstrcpy(pDot, ".slp");
		Fid = GSSiOpenFile(Name, 0, OF_READ);
		if (Fid == HFILE_ERROR)
		{
			DLGPROC lpfnSETSHAPEPARAMMsgProc;

			_fstrcpy(LastSQLITEFile, SQLITEFileName);
			ExpandText(LastSQLITEFile);
			if (!Type || !GetGlobalBVal2("[%AUTOSQLITEPARM]", TRUE))
				goto RtnFalse;
			goto RtnTrue;
			{
				lpfnSETSHAPEPARAMMsgProc = MakeProcInstance((DLGPROC)SETSHAPEPARAMMsgProc, hInst);
				DialogBox(hInst, (LPSTR)"SETSHAPEPARAM", hWnd, lpfnSETSHAPEPARAMMsgProc);
				FreeProcInstance(lpfnSETSHAPEPARAMMsgProc);
				Fid = GSSiOpenFile(Name, 0, OF_READ);
				if (Fid == HFILE_ERROR)
					goto RtnFalse;
			}
		}
		GSSifstat(Fid, &statParmFile);
		SQLITEParmTime = statParmFile.st_mtime;
		fgetstring(Projection, MAX_PATH, Fid);
		if (!havePrj)
		{
			if (!*Projection)
				GetGlobalCVal("[%DefaultShapeProjection]", Projection, "BASEPROJ");
			LoadProjection(0, Projection);
		}
		SQLITEProjectionIsBase = IS_BASE[0];
		fgetstring(Units, 32, Fid);
		if (!*Units)
			GetGlobalCVal("[%DefaultShapeUnits]", Units, "FEET");
		if (!havePrj)
		{
			if (!_fstricmp(Units, "FEET"))
				PRJ_UNITS[0] = 1;
			else if (!_fstricmp(Units, "METERS"))
				PRJ_UNITS[0] = 2;
			else
				PRJ_UNITS[0] = 4;
		}
		fgetstring(SQLITERefno, 255, Fid);
		if (IndexEntryStartRef != LONG_MAX)
			SQLITEBaseRefno = IndexEntryStartRef;
		else
			SQLITEBaseRefno = atol(SQLITERefno);
		fgetstring(SQLITETAG, 99, Fid);
		fgetstring(str, 32, Fid);
		SHPIndexType = atoi(str);
		_fmemset(SQLITEParms, 0, sizeof(SQLITEParms));
		while (fgetstring(str, 256, Fid))
		{
			if (*str == '#')
				break;
			DecodeSHPParam(str, SymName, cIF, cColor, cWidth, cRot);
			_fstrcpy(pParm, SymName);
			l = _fstrlen(SymName);
			pParm += l + 1;
			_fstrcpy(pParm, cIF);
			l = _fstrlen(cIF);
			pParm += l + 1;
			_fstrcpy(pParm, cColor);
			l = _fstrlen(cColor);
			pParm += l + 1;
			_fstrcpy(pParm, cWidth);
			l = _fstrlen(cWidth);
			pParm += l + 1;
			_fstrcpy(pParm, cRot);
			l = _fstrlen(cRot);
			pParm += l + 1;
			NumSQLITEParms++;
			switch (Type)
			{
			default:
			case SHPT_POINT:
			case SHPT_POINTZ:
			case SHPT_MULTIPOINT:
				itype = 1;
				break;
			case SHPT_ARC:
				itype = 2;
				break;
			case SHPT_POLYGON:
				itype = 3;
			}
			if (!GetOrCreateSym(hWnd, SymName, 0, 0, GetGlobalLVal2("[%ALLOWSYMBOLCREATION]", 0), itype))
			{
				ExpandText(SymName);
				MessageBox(0, "Symbol not found", SymName, MB_ICONEXCLAMATION);
			}
		}
		//CreateSHPSymlistFile(SHPFileName, NumSHPParms, SymName); could speed up file gdb and shp processing for multiple symbol files when symbol name contained in a variable
		if (fgetstring(str, 256, Fid))
			_fstrcpy(SQLITEBeginDate, str);
		if (fgetstring(str, 256, Fid))
			_fstrcpy(SQLITEEndDate, str);

		GSSiClose(Fid);
	RtnTrue:
		{
#if ENABLETRACE
			GSSiExitProg(1374);
#endif
			return TRUE;
		}
	RtnFalse:
		{
#if ENABLETRACE
			GSSiExitProg(1374);
#endif
			return FALSE;
		}
#if ENABLETRACE
	}
#endif
}

int TestSQLiteCrimes(LPMNMXCORD pBounds, int fromDate, int toDate, int fromUCR, int toUCR)
{
	sqlite3 *db;
	BOOL rtn;
	char cmd[1024];
	int n = 0;
	char Path[] = "c:\\temp\\crimes_latlon.sql";
	sqlite3_stmt *statement;
	MNMXCORD llBounds = *pBounds;
	char str[4096];

	pBounds = &llBounds;
	ConvertBounds(pBounds, 1, 2);
	rtn = sqlite3_open(Path, &db);
	if (rtn != SQLITE_OK)
		return FALSE;
	sqlite3_exec(db, "BEGIN", NULL, NULL, 0);
	sprintf(cmd, "SELECT ControlNbr,OffenseOrder,Offense FROM OFFENSEXY,OFFENSEXY_index WHERE OFFENSEXY.id=OFFENSEXY_index.id AND \
				 				 OFFENSEXY_index.minX>=%f AND OFFENSEXY_index.maxX<=%f AND OFFENSEXY_index.minY>=%f AND OFFENSEXY_index.maxY<=%f",
								 pBounds->xmn, pBounds->xmx, pBounds->ymn, pBounds->ymx);
	sprintf(cmd, "SELECT ControlNbr,OffenseOrder,Offense FROM OFFENSEXY,OFFENSEXY_index WHERE OFFENSEXY.id=OFFENSEXY_index.id AND OFFENSEXY_index.maxX>=%f AND OFFENSEXY_index.minX<=%f AND OFFENSEXY_index.maxY>=%f AND OFFENSEXY_index.minY<=%f AND OFFENSEXY_index.maxTime>=%f AND OFFENSEXY_index.minTime<=%f AND OFFENSEXY_index.maxUCR>=%f AND OFFENSEXY_index.minUCR<=%f",
		pBounds->xmn, pBounds->xmx, pBounds->ymn, pBounds->ymx, fromDate / 1000.0 - 1, toDate / 1000.0 + 1, fromUCR*10.0 - 1, toUCR*10.0 + 1);
	SQLOK(sqlite3_prepare_v2(db, cmd, -1, &statement, 0), db, "get num rows", 0);

	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		int offenseOrder = sqlite3_column_int(statement, 1);
		LPSTR offense = (LPSTR)sqlite3_column_text(statement, 2);
		long controlNbr = sqlite3_column_int(statement, 0);
		n++;
	}
	sqlite3_finalize(statement);
	HFILE fid = GSSiOpenFile("c:\\temp\\crimeup.txt", 0, OF_READ);
	while (fgetstring(str, 4090, fid))
	{
		rtn = SQLOK(sqlite3_exec(db, str, NULL, NULL, 0), db, "", 0);
	}
	GSSiClose(fid);
	SQLOK(sqlite3_exec(db, "ROLLBACK", NULL, NULL, 0), db, "", 0);
	//sqlite3_exec(db, "COMMIT", NULL, NULL, 0);
	rtn = sqlite3_close(db);
	return n;

}
int TestSQLiteCrimeOffenseOrder(LPMNMXCORD pBounds, int fromDate, int toDate, int fromUCR, int toUCR)
{
	sqlite3 *db;
	BOOL rtn;
	char cmd[1024];
	int n = 0, nbad = 0, nMult = 0;
	char Path[] = "c:\\temp\\crimes_latlon.sql";
	sqlite3_stmt *statement;
	MNMXCORD llBounds = *pBounds;
	long lastcnum = 0;
	int maxoffenseOrder = 0;

	pBounds = &llBounds;
	ConvertBounds(pBounds, 1, 2);
	rtn = sqlite3_open(Path, &db);
	if (rtn != SQLITE_OK)
		return FALSE;
	sqlite3_exec(db, "BEGIN", NULL, NULL, 0);
	sprintf(cmd, "SELECT ControlNbr,OffenseOrder,Offense FROM OFFENSEXY ORDER BY ControlNbr, OffenseOrder");
	SQLOK(sqlite3_prepare_v2(db, cmd, -1, &statement, 0), db, "get num rows", 0);

	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		int offenseOrder = sqlite3_column_int(statement, 1);
		LPSTR offense = (LPSTR)sqlite3_column_text(statement, 2);
		long controlNbr = sqlite3_column_int(statement, 0);
		if (lastcnum != controlNbr)
		{
			lastcnum = controlNbr;
			if (offenseOrder != 1)
				nbad++;
		}
		if (offenseOrder != 1)
			nMult++;
		n++;
		maxoffenseOrder = max(offenseOrder, maxoffenseOrder);
	}

	sqlite3_finalize(statement);
	sqlite3_exec(db, "COMMIT", NULL, NULL, 0);
	rtn = sqlite3_close(db);
	return n;

}
/*
BOOL LoadSQLiteCrimes(LPSTR FromPath, LPSTR ToPath)
{
	BOOL rtn = FALSE;
	sqlite3 *db;
	int err;
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX * 8);
	LPSTR  pCmd = GlobalLock(hCmd);
	
	rtn = sqlite3_open(ToPath, &db);
	if (rtn != SQLITE_OK)
	{
		rtn = FALSE;
	}
	else if (!stricmp(ARG[1], "CLOSE"))
	{
		db = (sqlite3*)atoi(ARG[2]);
		rtn = sqlite3_close(db);
	}
	err = SQLOK(sqlite3_exec(db, "BEGIN", NULL, NULL, 0), db, "", 0);

	HANDLE hGMDB = OpenGWDatabase(ARG[3], BT_READ);
	char *error = NULL;
	BOOL primKeyIsOffset = atob(ARG[5]);
	if (hGMDB)
	{
		LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock(hGMDB);

		db = (sqlite3*)atoi(ARG[2]);
		if (db)
		{
			HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX * 8);
			LPSTR  pCmd = GlobalLock(hCmd);
			LPGWFLDINFO lpFieldInfo;
			char delim[2] = { 0 };

			sprintf(pCmd, "DROP TABLE IF EXISTS %s", ARG[4]);
			rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "$SQLITE(FROMGMD drop table", &error);



	err = SQLOK(sqlite3_exec(db, "COMMIT", NULL, NULL, 0), db, "", 0);
	else if (!stricmp(ARG[1], "CMDFROMFILE"))
	{
#define MAXSTR 1020 * 256
		char *error = NULL;
		HFILE fid = GSSiOpenFile(ARG[3], 0, OF_READ);

		if (strstr(ARG[3], "34850-2"))
			ii = 1;
		db = (sqlite3*)atoi(ARG[2]);
		if (fid != HFILE_ERROR)
		{
			HANDLE hstr = GSSiGlobAlloc(0, GMEM_MOVEABLE, MAXSTR);
			LPSTR cmd = GlobalLock(hstr);
			rtn = 1;
			while (fgetstring(cmd, -(MAXSTR - 2), fid))
			{
				int err;

				//REPLAC(cmd, "/", "//", MAXSTR-2);
				err = SQLOK(sqlite3_exec(db, cmd, NULL, NULL, &error), db, "", &error);
				sqlite3_free(error);
				if (err)
				{
					rtn = 0;
					break;
				}
			}
			GSSiGlobUlFree(&hstr);
			GSSiClose(fid);
		}
	}
	else if (!stricmp(ARG[1], "EXECUTE"))//$SQLITE(EXECUTE,sqlitehandle,cmd)
	{
		db = (sqlite3*)atoi(ARG[2]);
		sqlite3_stmt *statement;

		if (db)
		{
			if (SQLOK(sqlite3_prepare_v2(db, ARG[3], -1, &statement, 0), db, "", 0) == SQLITE_OK)
			{
				if (sqlite3_step(statement) == SQLITE_ROW)
					rtn = TRUE;
				sqlite3_finalize(statement);
			}
		}
	}
	else if (!stricmp(ARG[1], "PREPARE"))//$SQLITE(PREPARE,sqlitehandle,cmd)returns statement address or 0
	{
		db = (sqlite3*)atoi(ARG[2]);
		sqlite3_stmt *statement;

		if (db)
		{
			if (SQLOK(sqlite3_prepare_v2(db, ARG[3], -1, &statement, 0), db, "", 0) == SQLITE_OK)
				rtn = (int)statement;
		}
	}
	else if (!stricmp(ARG[1], "STEP"))//$SQLITE(STEP,statement) returns statement address or 0
	{
		sqlite3_stmt *statement = (sqlite3_stmt *)atoi(ARG[2]);
		if (sqlite3_step(statement) == SQLITE_ROW)
			rtn = TRUE;
	}
	else if (!stricmp(ARG[1], "FINALIZE"))//$SQLITE(STEP,statement) returns statement address or 0
	{
		sqlite3_stmt *statement = (sqlite3_stmt *)atoi(ARG[2]);
		if (sqlite3_finalize(statement) == SQLITE_OK)
			rtn = TRUE;
	}
	else if (!stricmp(ARG[1], "COLUMN"))//$SQLITE(COLUMN,statement,icol,globalvarname)
	{
		sqlite3_stmt *statement = (sqlite3_stmt *)atoi(ARG[2]);
		int irow = atoi(ARG[3]);
		LPSTR pval;

		pval = (LPSTR)sqlite3_column_text(statement, irow);
		if (pval)
		{
			if (*ARG[4])
				SetGlobalValue(ARG[4], pval);
			rtn = TRUE;
		}
	}
	else if (!stricmp(ARG[1], "NUMROWS"))//$SQLITE(ROWS,sqlitehandle,tablename,where clause)
	{
		db = (sqlite3*)atoi(ARG[2]);
		rtn = GetSQLITENumRows(db, ARG[3]);
	}
	else if (!stricmp(ARG[1], "FROMGMD"))//$SQLITE(FROMGMD,sqlitehandle,gmdfile,tablename,primkeyisoffset)
	{
		HANDLE hGMDB = OpenGWDatabase(ARG[3], BT_READ);
		char *error = NULL;
		BOOL primKeyIsOffset = atob(ARG[5]);
		if (hGMDB)
		{
			LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock(hGMDB);

			db = (sqlite3*)atoi(ARG[2]);
			if (db)
			{
				HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX * 8);
				LPSTR  pCmd = GlobalLock(hCmd);
				LPGWFLDINFO lpFieldInfo;
				char delim[2] = { 0 };

				sprintf(pCmd, "DROP TABLE IF EXISTS %s", ARG[4]);
				rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "$SQLITE(FROMGMD drop table", &error);
				sqlite3_free(error);

				if (primKeyIsOffset)
					sprintf(pCmd, "CREATE TABLE %s (OFFSET INT PRIMARY KEY,", ARG[4]);
				else
					sprintf(pCmd, "CREATE TABLE %s (", ARG[4]);

				for (i = 0, lpFieldInfo = lpGWDHead->pFldInfo; i<lpGWDHead->NumFields; i++, lpFieldInfo++)
				{
					switch (lpFieldInfo->Type)
					{
					case BT_CHAR:
						sprintf(strchr(pCmd, 0), "%s'%s' CHAR(%i)", delim, removePCT(lpFieldInfo->Name), lpFieldInfo->Len);
						break;
					case BT_INTEGER:
						sprintf(strchr(pCmd, 0), "%s'%s' INT", delim, removePCT(lpFieldInfo->Name));
						break;
					case BT_REAL:
						sprintf(strchr(pCmd, 0), "%s'%s' REAL", delim, removePCT(lpFieldInfo->Name));
						break;
					default:
						MessageBox(0, "Bad Type", 0, MB_ICONEXCLAMATION);
						rtn = 1;
						break;
					}
					delim[0] = ',';
				}
				if (!rtn)
				{
					int ifield, index;
					int firstIndex = 1;

					if (primKeyIsOffset)
					{
						firstIndex = 0;
						sprintf(strchr(pCmd, 0), ")");
					}
					else
					{
						sprintf(strchr(pCmd, 0), ",PRIMARY KEY('%s' ASC", lpGWDHead->pFldInfo->Name);
						for (ifield = 1, lpFieldInfo = lpGWDHead->pFldInfo + 1; ifield < lpGWDHead->NumIndexFields[0]; ifield++, lpFieldInfo++)
						{
							sprintf(strchr(pCmd, 0), ",'%s' ASC", removePCT(lpFieldInfo->Name));
						}
						sprintf(strchr(pCmd, 0), "))");
					}
					rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "$SQLITE(FROMGMD create table", &error);
					sqlite3_free(error);
					if (!rtn)
						for (index = firstIndex; index < lpGWDHead->NumIndex; index++)
						{
							sprintf(pCmd, "CREATE INDEX %s_Index%i ON %s ('%s' ASC", ARG[4], index + 1, ARG[4], (lpGWDHead->pFldInfo + lpGWDHead->IndexFields[index][0])->Name);
							for (ifield = 1, lpFieldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[index][1]; ifield<lpGWDHead->NumIndexFields[index]; ifield++, lpFieldInfo++)
							{
								sprintf(strchr(pCmd, 0), ",'%s' ASC", removePCT(lpFieldInfo->Name));
							}
							sprintf(strchr(pCmd, 0), ")");
							rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "$SQLITE(FROMGMD create table", &error);
							sqlite3_free(error);
						}
				}


	return rtn;
}
			*/
HANDLE	OpenSLTDatabase(LPSTR NameIN, PSTR SQL, short Access)
{
	HANDLE	hDB;
	LPSQLDATABASE	pDB;
	LPSTR pTable;
	LPSTR pWhere;
	char Name[MAX_PATH + 256];
	char JName[MAX_PATH + 256];
	char lastName[256] = { 0 };
	int rtn;
	sqlite3 *db;

	hDB = GSSiGlobAlloc(1505, GHND, USHRT_MAX);
	pDB = (LPSQLDATABASE)GlobalLock(hDB);
	pDB->hasRowID = TRUE;
	strcpy(Name, NameIN);
	if (!(pTable = strrchr(Name, '(')))
		pTable = strrchr(Name, '|');
	if (pTable)
	{
		char delim = *pTable;
		*pTable++ = 0;
		if (delim == '(')
			*LastChr(pTable) = 0;
		strcpy(pDB->From, pTable);
	}
	strcpy(pDB->DBName, Name);
	if (GSSiLength(pDB->DBName) <= 0)
	{
		GSSiGlobUlFree(&hDB);
		return 0;
	}
	sprintf(JName, "%s-journal", Name);
	if (!ExistFile(JName) && Access == BT_READ)
	{
		OFSTRUCTGM OFStruct;
		HFILE fid = GSSiOpenFile(Name, &OFStruct, OF_READ);
		GSSiClose(fid);
		if (fid != HFILE_ERROR)
			rtn = sqlite3_open_v2(OFStruct.szPathName, &db, SQLITE_OPEN_READONLY, NULL);
		else
			rtn = -1;
	}
	else
		rtn = sqlite3_open(Name, &db);
	if (rtn != SQLITE_OK)
	{
		GSSiGlobUlFree(&pDB);
		return 0;
	}
	pDB->DBHandle = db;
	strcpy(pDB->Where, SQL);
	if (*pDB->From)
	{
		sprintf(pDB->Query, "SELECT rowid,* FROM '%s';", pDB->From);
		if (!SQLOK(sqlite3_prepare_v2(db, pDB->Query, -1, &pDB->statement, 0), db, "get db info", 0))
		{
			int ncols = sqlite3_column_count(pDB->statement);
			pDB->NumFields = 0;
			for (int j = 0; j < ncols; j++)
			{
				char nulltype[2] = { "" };
				int itype = sqlite3_column_type(pDB->statement, j);
				int ibytes = sqlite3_column_bytes(pDB->statement,j);
				LPSTR decl = (LPSTR)sqlite3_column_decltype(pDB->statement,j);
				LPSTR pName = (LPSTR)sqlite3_column_name(pDB->statement,j);
				if (!decl)
					decl = nulltype;
				if (strcmp(pName, lastName))
				{
					strcpy(lastName, pName);
					int nc = 0;
					LPSTR pPar = 0;
					//if (itype != SQLITE_NULL)
					pPar = strchr(decl, '(');
					if (pPar)
					{
						*pPar++ = 0;
						nc = atoi(pPar);
					}
					strncpy(pDB->FldInfo[pDB->NumFields].name, pName, sizeof(pDB->FldInfo[pDB->NumFields].name));
					pDB->FldInfo[pDB->NumFields].index = pDB->NumFields;

					//if (itype != SQLITE_NULL)
					{
						if (!strnicmp(decl, "INT", 3))
						{
							pDB->FldInfo[pDB->NumFields].type = BT_INTEGER;
							pDB->FldInfo[pDB->NumFields].length = 4;
						}
						else if (!stricmp(decl, "REAL") || !stricmp(decl, "FLOAT") || !stricmp(decl, "DOUBLE"))
						{
							pDB->FldInfo[pDB->NumFields].type = BT_REAL;
							pDB->FldInfo[pDB->NumFields].length = 8;
						}
						else if (!stricmp(decl, "BLOB"))
						{
							pDB->FldInfo[pDB->NumFields].type = SQL_LONGVARBINARY;
							pDB->FldInfo[pDB->NumFields].length = nc;
						}
						else if (!stricmp(decl, "CHAR"))
						{
							pDB->FldInfo[pDB->NumFields].type = BT_CHAR;
							pDB->FldInfo[pDB->NumFields].length = nc;
						}
						else if (!stricmp(decl, "TEXT"))
						{
							pDB->FldInfo[pDB->NumFields].type = BT_CHAR;
							pDB->FldInfo[pDB->NumFields].length = nc;
						}
						else
							MessageBox(0, decl, "Invalid type", MB_OK);
					}
					pDB->NumFields++;

				}
				else
					pDB->hasRowID = FALSE;
			}

		}
		sqlite3_finalize(pDB->statement);
		pDB->statement = NULL;
/*		pWhere = malloc(4096);
		strcpy(pWhere, pDB->Where);
		ExpandText(pWhere);
		if (*pWhere)
			sprintf(pDB->Query, "SELECT rowid,* FROM '%s' WHERE %s;", pDB->From, pWhere);
		else
			sprintf(pDB->Query, "SELECT rowid,* FROM '%s';", pDB->From);
		free(pWhere);
		if (SQLOK(sqlite3_prepare_v2(db, pDB->Query, -1, &pDB->statement, 0), db, pDB->Query, 0))
		{
			GSSiGlobUlFree(&hDB);
		}*/

	}
	if (hDB)
		GlobalUnlock(hDB);
	return hDB;
}

BOOL FetchSLTRec(LPSQLDATABASE pSQL)
{
	BOOL rtn = FALSE;
	int st = sqlite3_step(pSQL->statement);
	if (st == SQLITE_ROW)
		rtn = TRUE;
	return rtn;
}

void SLTCloseCursor(LPSQLDATABASE pDB)
{
	if (pDB->statement)
		sqlite3_finalize(pDB->statement);
	pDB->statement = NULL;
}

BOOL SLTPrepareStatement(LPSQLDATABASE	pDB, LPSTR SQL)
{
	sqlite3 *db;
	BOOL rtn = FALSE;
	LPSTR pWhere;
	char getRowID[8] = { 0 };

	db = pDB->DBHandle;
	if (pDB->statement)
		sqlite3_finalize(pDB->statement);
	pDB->statement = NULL;
	pWhere = malloc(4096);
	strcpy(pWhere, SQL);
	ExpandText(pWhere);
	if (pDB->hasRowID)
		strcpy(getRowID, "rowid,");
	if (*pWhere)
		sprintf(pDB->Query, "SELECT %s* FROM '%s' WHERE %s",getRowID, pDB->From, pWhere);
	else
		sprintf(pDB->Query, "SELECT %s* FROM '%s'",getRowID, pDB->From);
	free(pWhere);
	if (!SQLOK(sqlite3_prepare_v2(db, pDB->Query, -1, &pDB->statement, 0), db, "prepare", 0))
	{
		rtn = TRUE;
	}
	return rtn;
}

LPSTR GetSLTFieldData(HANDLE hDB, LPSTR SQL, LPFIELDINFO infield, BOOL SingleVal, LPSHORT irc,LPFIELDINFO FirstField)
{
	static char answer[MAXVARLEN+2] = { 0 };
	LPSTR lpvoid = answer;
	LPSTR pVal = 0;
	LPFIELDINFO field;
	LPCURVAL	pCurVal;
	int WantField = infield->index;

	*irc = 1;
	if (infield->hCurVal)
	{
		pCurVal = (LPCURVAL)GlobalLock(infield->hCurVal);
		int len = min(MAXVARLEN - 2, pCurVal->length);
		_fstrncpy(answer, &pCurVal->Value, len);
		answer[len] = 0;
		GlobalUnlock(infield->hCurVal);
		*irc = 0;
		return lpvoid;
	}
	if (hDB)
	{
		LPSQLDATABASE pDB = (LPSQLDATABASE)GlobalLock(hDB);
		*irc = 0;
		if (SingleVal)
		{
			infield->hCurVal = 0;
			WantField = 0;
		}
		else
		{
			int cols = sqlite3_column_count(pDB->statement);
			char zero[2] = "";
			field = pDB->FldInfo;
			int jstart = 0;
			int i = 0;
			if (cols > pDB->NumFields)
			{
				field++;
				jstart++;
			}
			for (int j = jstart; j < cols;j++, i++,field++)
			{
				int l = sqlite3_column_bytes(pDB->statement, j);
				LPSTR	str = (LPSTR)sqlite3_column_text(pDB->statement, j);

				if (!str)
					str = zero;
				pDB->FldInfo[i].length = l;
				if (field->hCurVal)
					GSSiGlobFree(&field->hCurVal);
				field->hCurVal = GSSiGlobAlloc(153, GMEM_MOVEABLE, sizeof(int)+l + 4);
				pCurVal = (LPCURVAL)GlobalLock(field->hCurVal);
				pCurVal->length = l;
				if (pCurVal->length)
					_fstrncpy(&pCurVal->Value, str, pCurVal->length);
				GlobalUnlock(field->hCurVal);
				if (i == WantField)
				{
					int i = min(MAXVARLEN, l);
					_fstrncpy(answer, str, i);
					answer[i] = 0;
				}
			}
		}
		GlobalUnlock(hDB);
	}
	return lpvoid;
}

void CloseSLTDatabase(LPHANDLE pHandle)
{
	int rtn;
	HANDLE	hDB;
	LPSQLDATABASE	pDB;

	if (*pHandle)
	{
		LPFIELDINFO field;
		pDB = (LPSQLDATABASE)GlobalLock(*pHandle);
		if (*pDB->Query)
			sqlite3_finalize(pDB->statement);
		rtn = sqlite3_close(pDB->DBHandle);
		field = pDB->FldInfo;
		for (int j = 0; j < pDB->NumFields; j++, field++)
		{
			if (field->hCurVal)
				GSSiGlobFree(&field->hCurVal);
		}
		GSSiGlobUlFree(pHandle);
	}
	return;
}

HANDLE	OpenSLTDatabaseQuery(LPSTR Name, LPSTR SQL)
{
	HANDLE	hDB;
	char	str[130];
	HANDLE	hMem;
	LPSTR	pMem;
	sqlite3 *db;
	sqlite3_stmt *statement;
	LPSQLDATABASE	pDB;

	int rtn = sqlite3_open(Name, &db);
	if (rtn != SQLITE_OK)
		return 0;
	hDB = GSSiGlobAlloc(1505, GHND, USHRT_MAX);
	pDB = (LPSQLDATABASE)GlobalLock (hDB);

	if (sqlite3_prepare_v2(db, SQL, -1, &statement, 0) == SQLITE_OK)
	{
		int cols = sqlite3_column_count(statement);
		for (int i = 0; i < cols; i++)
		{
			int itype = sqlite3_column_type(statement, i);
			{
				int type = BT_INTEGER;
				
				switch (itype)
				{
				case SQLITE_INTEGER:
					break;
				case SQLITE_FLOAT:
					type = BT_REAL;
					break;
				case SQLITE_NULL:
				case SQLITE_TEXT:
					type = BT_CHAR;
					break;
				case SQLITE_BLOB:
					break;
				}
			}
			int ibytes = sqlite3_column_bytes(statement, i);
			if (!ibytes)
				ibytes = 4096;
			pDB->FldInfo[pDB->NumFields].length = ibytes;
			LPSTR pName = (LPSTR)sqlite3_column_name(statement, i);
			if (pName)
			{
				strncpy(pDB->FldInfo[pDB->NumFields++].name, pName, sizeof(pDB->FldInfo[pDB->NumFields++].name));
			}

		}
	}
	sqlite3_finalize(statement);

/*	why not just stick "limit 0" on the end of a select statement ? int cols = sqlite3_column_count(stmt); fprintf(stdout, "%d columns\n", cols); for (int i = 0; i<cols; i++) fprintf(stdout, "%d. %s\n", i, sqlite3_column_name(stmt, i)); – Erik Aronesty May 20 '15 at 21:09  





										   &pDB->FldInfo[pDB->NumFields].type,
										   &pDB->FldInfo[pDB->NumFields].index,
										   &pDB->FldInfo[pDB->NumFields].radix,
										   &pDB->FldInfo[pDB->NumFields].scale,
										   &pDB->FldInfo[pDB->NumFields].length,
										   &pDB->FldInfo[pDB->NumFields].precision,
										   pDB->FldInfo[pDB->NumFields].name);
										   */
  	GlobalUnlock (hDB);


	return hDB;
}
void CloseSLTDatabaseQuery (LPHANDLE pHandle)
{
	int rtn;
	if (*pHandle)
		rtn = sqlite3_close(*pHandle);
	return;
}
BOOL SLTSpatialIndexCreate(sqlite3 *db, LPSTR tableName)
{
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX);
	LPSTR  pCmd = GlobalLock(hCmd);
	sqlite3_stmt *statement;
	BOOL rtn = FALSE;
	char * error;

	if (db)
	{
		sprintf(pCmd, "DROP TABLE IF EXISTS %s;DROP TABLE IF EXISTS %s_index;CREATE TABLE %s (id INTEGER PRIMARY KEY, Name CHAR(256));\
					  CREATE VIRTUAL TABLE %s_index USING rtree(id, minX, maxX, minY, maxY);", tableName, tableName, tableName, tableName);

		if (sqlite3_exec(db, pCmd, 0, 0, 0) == SQLITE_OK)
			rtn = TRUE;
	}
	GSSiGlobUlFree(&hCmd);
	return rtn;

}
BOOL SLTSpatialIndexCreate3D(sqlite3 *db, LPSTR tableName)
{
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX);
	LPSTR  pCmd = GlobalLock(hCmd);
	sqlite3_stmt *statement;
	BOOL rtn = FALSE;
	char * error;

	if (db)
	{
		sprintf(pCmd, "DROP TABLE IF EXISTS %s;DROP TABLE IF EXISTS %s_index;CREATE TABLE %s (id INTEGER PRIMARY KEY, Name CHAR(256));\
					  CREATE VIRTUAL TABLE %s_index USING rtree(id, minX, maxX, minY, maxY, minZ, maxZ);", tableName, tableName, tableName, tableName);

		if (sqlite3_exec(db, pCmd, 0, 0, 0) == SQLITE_OK)
			rtn = TRUE;
	}
	GSSiGlobUlFree(&hCmd);
	return rtn;

}

BOOL SLTSpatialIndexAdd(sqlite3 *db, LPSTR tableName, LONGLONG id, LPSTR Name, LPMNMXCORD pBounds)
{
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX);
	LPSTR  pCmd = GlobalLock(hCmd);
	sqlite3_stmt *statement;
	BOOL rtn = FALSE;

	if (db)
	{
		sprintf(pCmd, "INSERT INTO %s_index VALUES(%lli,%f,%f,%f,%f);INSERT INTO %s VALUES(%lli, '%s');",
			tableName, id, pBounds->xmn, pBounds->xmx, pBounds->ymn, pBounds->ymx, tableName, id, Name);
		if (sqlite3_exec(db, pCmd, 0, 0, 0) == SQLITE_OK)
			rtn = TRUE;
	}
	return rtn;
}
BOOL SLTSpatialIndexAdd3D(sqlite3 *db, LPSTR tableName, LONGLONG id, LPSTR Name, LPMNMXCORD3D pBounds)
{
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX);
	LPSTR  pCmd = GlobalLock(hCmd);
	sqlite3_stmt *statement;
	BOOL rtn = FALSE;

	if (db)
	{
		sprintf(pCmd, "INSERT INTO %s_index VALUES(%lli,%f,%f,%f,%f,%f,%f);INSERT INTO %s VALUES(%lli, '%s');",
			tableName, id, pBounds->xmn, pBounds->xmx, pBounds->ymn, pBounds->ymx, pBounds->zmn, pBounds->zmx, tableName, id, Name);
		if (sqlite3_exec(db, pCmd, 0, 0, 0) == SQLITE_OK)
			rtn = TRUE;
	}
	return rtn;
}
MNMXCORD SLTSpatialIndexBounds(sqlite3 *db, LPSTR tableName)
{
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX);
	LPSTR  pCmd = GlobalLock(hCmd);
	sqlite3_stmt *statement;
	MNMXCORD Bounds;
	int n = 0;
	DBoundsInit(&Bounds);

	if (db)
	{
		sprintf(pCmd, "SELECT MinX, MaxX, MinY, MaxY FROM %s_index", tableName);

		SQLOK(sqlite3_prepare_v2(db, pCmd, -1, &statement, 0), db, "get bounds", 0);

		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			MNMXCORD bounds;
			bounds.xmn = sqlite3_column_double(statement, 0);
			bounds.xmx = sqlite3_column_double(statement, 1);
			bounds.ymn = sqlite3_column_double(statement, 2);
			bounds.ymx = sqlite3_column_double(statement, 3);
			n++;
			AddMinMaxD(&Bounds, &bounds);
		}
		sqlite3_finalize(statement);
	}
	return Bounds;
}
MNMXCORD3D SLTSpatialIndexBounds3D(sqlite3 *db, LPSTR tableName)
{
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX);
	LPSTR  pCmd = GlobalLock(hCmd);
	sqlite3_stmt *statement;
	MNMXCORD3D Bounds;
	int n = 0;
	DBoundsInit3D(&Bounds);

	if (db)
	{
		sprintf(pCmd, "SELECT MinX, MaxX, MinY, MaxY, MinZ, MaxZ FROM %s_index", tableName);

		SQLOK(sqlite3_prepare_v2(db, pCmd, -1, &statement, 0), db, "get bounds", 0);

		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			MNMXCORD3D bounds;
			bounds.xmn = sqlite3_column_double(statement, 0);
			bounds.xmx = sqlite3_column_double(statement, 1);
			bounds.ymn = sqlite3_column_double(statement, 2);
			bounds.ymx = sqlite3_column_double(statement, 3);
			bounds.zmn = sqlite3_column_double(statement, 4);
			bounds.zmx = sqlite3_column_double(statement, 5);
			n++;
			AddMinMax3D(&Bounds, &bounds);
		}
		sqlite3_finalize(statement);
	}
	return Bounds;
}
