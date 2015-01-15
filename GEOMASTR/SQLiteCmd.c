#include "graphint.h"   
#include "sqlite3.h"
#include "gmextern.h"
//#include "sqlite3ext.h"

int i;

int SQLOK(int sqlReturn, char *method,char ** error)
{
	if (sqlReturn != SQLITE_OK)
	{
		char mess[256];

		if (*error)
			sprintf(mess, "SQLite error %s at %s", *error,method);
		else
			sprintf(mess, "SQLite error at %s", method);
		MessageBox(0, mess, 0, MB_ICONEXCLAMATION);
	}

	return sqlReturn;
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
		}
	}
	else if (!stricmp(ARG[1], "CLOSE"))
	{
		db = (sqlite3*)atoi(ARG[2]);
		rtn = sqlite3_close(db);
	}
	else if (!stricmp(ARG[1], "FROMGMD"))//$SQLITE(FROMGMD,sqlitehandle,gmdfile,tablename)
	{
		HANDLE hGMDB = OpenGWDatabase(ARG[3], BT_READ);
		char *error=NULL;

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
				rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), "$SQLITE(FROMGMD drop table", &error);
				sqlite3_free(error);

				sprintf(pCmd, "CREATE TABLE %s (", ARG[4]);

				for (i = 0, lpFieldInfo = lpGWDHead->pFldInfo; i<lpGWDHead->NumFields; i++, lpFieldInfo++)
				{
					switch (lpFieldInfo->Type)
					{
					case BT_CHAR:
						sprintf(strchr(pCmd, 0), "%s'%s' CHAR(%i)", delim, lpFieldInfo->Name, lpFieldInfo->Len);
						break;
					case BT_INTEGER:
						sprintf(strchr(pCmd, 0), "%s'%s' INT", delim, lpFieldInfo->Name);
						break;
					case BT_REAL:
						sprintf(strchr(pCmd, 0), "%s'%s' REAL", delim, lpFieldInfo->Name);
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
					sprintf(strchr(pCmd, 0), ",PRIMARY KEY('%s' ASC", lpGWDHead->pFldInfo->Name);
					for (ifield = 1, lpFieldInfo = lpGWDHead->pFldInfo + 1; ifield<lpGWDHead->NumIndexFields[0]; ifield++, lpFieldInfo++)
					{
						sprintf(strchr(pCmd, 0), ",'%s' ASC", lpFieldInfo->Name);
					}
					sprintf(strchr(pCmd, 0), "))");
					rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), "$SQLITE(FROMGMD create table", &error);
					sqlite3_free(error);
					if (!rtn)
					for (index = 1; index < lpGWDHead->NumIndex; index++)
					{
						sprintf(pCmd, "CREATE INDEX Index%i ON %s ('%s' ASC", index+1,ARG[4], (lpGWDHead->pFldInfo+lpGWDHead->IndexFields[1][0])->Name);
						for (ifield = 1, lpFieldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[index][1]; ifield<lpGWDHead->NumIndexFields[index]; ifield++, lpFieldInfo++)
						{
							sprintf(strchr(pCmd, 0), ",'%s' ASC", lpFieldInfo->Name);
						}
						sprintf(strchr(pCmd, 0), ")");
						rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), "$SQLITE(FROMGMD create table", &error);
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
					rtn = SQLOK(sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error), "loadIntersectionTextToDatabase2", &error);
					while (!rtn && !BT_FIND(lpGWDHead->BTHandle[0], lpGWDHead->pKeys[0], pos, BT_ANY, (LPSTR)&Offset))
					{
						pos = BT_NEXT;
						FillGWDData(lpGWDHead, Offset);
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
						rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), "$SQLITE(FROMGMD insert record", &error);
						sqlite3_free(error);
						rtn = !StatusWindowUpdate(NULL, NULL, nRecs, ++nLoaded);
					}
					DestroyStatusWindow(0);
					if (!rtn)
					{
						rtn = SQLOK(sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, &error), "$SQLITE(FROMGMD commit transaction", &error);
					}
					else
					{
						SQLOK(sqlite3_exec(db, "ROLLBACK TRANSACTION", NULL, NULL, &error), "$SQLITE(FROMGMD rollback transaction", &error);
					}
					//SQLOK(sqlite3_finalize(self.statement), "loadIntersectionTextToDatabase8");
					GSSiGlobUlFree(&hVal);
				}
				GSSiGlobUlFree(&hCmd);
			}
			if (!rtn)
				rtn = 1;
			GlobalUnlock(hGMDB);
			CloseGWDatabase(hGMDB);
		}
	}
	return rtn;
}