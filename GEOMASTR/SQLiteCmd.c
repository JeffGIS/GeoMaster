#include "graphint.h"   
#include "gmextern.h"
//#include "sqlite3ext.h"

#define BLOB_MAX	USHRT_MAX
int i;

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
	else if (!stricmp(ARG[1], "NUMROWS"))//$SQLITE(ROWS,sqlitehandle,tablename,where clause)
	{
		HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX);
		LPSTR  pCmd = GlobalLock(hCmd);
		db = (sqlite3*)atoi(ARG[2]);
		sprintf (pCmd,"SELECT COUNT (*) FROM %s",ARG[3]);
		sqlite3_stmt *statement;

		SQLOK(sqlite3_prepare_v2(db, pCmd, -1, &statement, NULL), "get num rows",NULL);

		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			rtn = sqlite3_column_int(statement, 0);
		}
		SQLOK(sqlite3_finalize(statement), "get num rows",NULL);
		GSSiGlobUlFree(&hCmd);
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

				sprintf(pCmd, "DROP TABLE IF EXISTS %s", ARG[4]);
				rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), "$SQLITE(FROMGMD drop table", &error);
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
					rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), "$SQLITE(FROMGMD create table", &error);
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
		else if (!stricmp(ARG[1], "TEXTFROMGMD"))//$SQLITE(TEXTFROMGMD,outfilename,new,gmdfile,tablename,primkeyisoffset)
		{
			HANDLE hGMDB = OpenGWDatabase(ARG[4], BT_READ);
			char *error = NULL;
			BOOL primKeyIsOffset = atob(ARG[6]);
			HFILE fid;
			if (hGMDB)
			{
				LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock(hGMDB);

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

					GSSillseek(fid, 0, 2);
					sprintf(pCmd, "DROP TABLE IF EXISTS %s", ARG[5]);
					fputstring(pCmd, fid);

					if (primKeyIsOffset)
						sprintf(pCmd, "CREATE TABLE %s (OFFSET INT PRIMARY KEY,", ARG[5]);
					else
						sprintf(pCmd, "CREATE TABLE %s (", ARG[5]);

					for (i = 0, lpFieldInfo = lpGWDHead->pFldInfo; i<lpGWDHead->NumFields; i++, lpFieldInfo++)
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
							sprintf(strchr(pCmd, 0), ",PRIMARY KEY('%s' ASC", removePCT(lpGWDHead->pFldInfo->Name));
							for (ifield = 1, lpFieldInfo = lpGWDHead->pFldInfo + 1; ifield < lpGWDHead->NumIndexFields[0]; ifield++, lpFieldInfo++)
							{
								sprintf(strchr(pCmd, 0), ",'%s' ASC", removePCT(lpFieldInfo->Name));
							}
							sprintf(strchr(pCmd, 0), "))");
						}
						fputstring(pCmd, fid);
						for (index = firstIndex; index < lpGWDHead->NumIndex; index++)
						{
							sprintf(pCmd, "CREATE INDEX %s_Index%i ON %s ('%s' ASC", ARG[5], index + 1, ARG[5], removePCT((lpGWDHead->pFldInfo + lpGWDHead->IndexFields[index][0])->Name));
							for (ifield = 1, lpFieldInfo = lpGWDHead->pFldInfo + lpGWDHead->IndexFields[index][1]; ifield<lpGWDHead->NumIndexFields[index]; ifield++, lpFieldInfo++)
							{
								sprintf(strchr(pCmd, 0), ",'%s' ASC", removePCT(lpFieldInfo->Name));
							}
							sprintf(strchr(pCmd, 0), ")");
							fputstring(pCmd, fid);
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
						while (!rtn && !BT_FIND(lpGWDHead->BTHandle[0], lpGWDHead->pKeys[0], pos, BT_ANY, (LPSTR)&Offset))
						{
							pos = BT_NEXT;
							FillGWDData(lpGWDHead, Offset);
							if (primKeyIsOffset)
								sprintf(pCmd, "INSERT INTO %s VALUES(%i,", ARG[5], Offset);
							else
								sprintf(pCmd, "INSERT INTO %s VALUES(", ARG[5]);
							delim[0] = 0;
							for (i = 0, lpFieldInfo = lpGWDHead->pFldInfo; i < lpGWDHead->NumFields; i++, lpFieldInfo++)
							{
								GMDGetCharFieldVal(lpGWDHead, i, val);
								switch (lpFieldInfo->Type)
								{
								case BT_CHAR:
									if (!stricmp(lpFieldInfo->Name, "Offsets") && lpFieldInfo->Len == 400)
									{
										int i;
										LPBYTE pByte = (LPBYTE)val;
										LPBYTE pBlob = BytesToBlob(pByte, lpFieldInfo->Len);

										sprintf(strchr(pCmd, 0), "%sX'%s'", delim,pBlob);
										free(pBlob);
									}
									else
									{
										REPLAC(val, "'", "''", 4096);
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
							sprintf(strchr(pCmd, 0), ")");
							fputstring(pCmd, fid);
							rtn = !StatusWindowUpdate(NULL, NULL, nRecs, ++nLoaded);
						}
						DestroyStatusWindow(0);
						GSSiClose(fid);
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
		else if (!stricmp(ARG[1], "TEXTFROMPOLY"))//$SQLITE(TEXTFROMPOLY,outfilename,new,tablename,projection)
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

			if (atob(ARG[3]))
				Fid = GSSiOpenFile(ARG[2], 0, OF_CREATE);
			else
				Fid = GSSiOpenFile(ARG[2], 0, OF_READWRITE);
			if (Fid != HFILE_ERROR)
			{
				HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX * 8);
				LPSTR  pCmd = GlobalLock(hCmd);

				sprintf(pCmd, "Extact Table %s", ARG[4]);
				CreateStatusWind(hWndMain, 1,pCmd);
				sprintf(pCmd, "DROP TABLE IF EXISTS %s", ARG[4]);
				fputstring(pCmd, Fid);
				sprintf(pCmd, "DROP TABLE IF EXISTS %s_index", ARG[4]);
				fputstring(pCmd, Fid);

				sprintf(pCmd, "CREATE VIRTUAL TABLE %s_index USING rtree(id,minX, maxX, minY, maxY);", ARG[4]);
				fputstring(pCmd, Fid);
				sprintf(pCmd, "CREATE TABLE %s (id INT PRIMARY KEY,PID CHAR(13),BasePointX REAL,BasePointY REAL,NumPoints INT,NumLoops INT,PolyPartLen BLOB(%i), Points BLOB(%i));", ARG[4], BLOB_MAX,BLOB_MAX*8);
				fputstring(pCmd, Fid);
				while (keepGoing && !BT_FIND(hHighlight, (LPSTR)&Refno, pos, BT_ANY, (LPSTR)&HighlightData))
				{
					pos = BT_NEXT;
					if (HighlightData.PD.Type == 3)
					{

						if ((nLoops = GetPolyPointsWithParts((LPPICKDATAHEADER)&HighlightData.PD, &nPnts, &hPoly, &hPolyPartLen)))
						{
							LPMNMXCORD	pBounds = (LPMNMXCORD)GlobalLock(hPoly);
							DPOINT midPt;
							HPDPOINT pDPoints;
							HPPOINT  pPoints;
							LPSTR blobPoints,blobParts;
							HANDLE hPoints;
							BOOL canCompress=TRUE;
							int  np = nPnts;

							ConvertBounds(pBounds, 1, 2);
							sprintf(pCmd, "INSERT INTO %s_index VALUES(%i,%.6f,%.6f,%.6f,%.6f);", ARG[4], Refno, pBounds->xmn, pBounds->xmx, pBounds->ymn, pBounds->ymx);
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
							for (i = 0; i < nPnts; i++)
							{
								ConvertCoord(&pDPoints[i], 1, 2);
								pPoints[i].x = 100000000 * (pDPoints[i].x - midPt.x);
								pPoints[i].y = 100000000 * (pDPoints[i].y - midPt.y);
								if (pPoints[i].x > SHRT_MAX || pPoints[i].x < SHRT_MIN || pPoints[i].y > SHRT_MAX || pPoints[i].y < SHRT_MIN)
									canCompress = FALSE;
							}
							if (canCompress)
							{
								LPPOINTS pShortPoints = malloc(nPnts*sizeof(POINTS)+4);
								for (i = 0; i<nPnts; i++)
								{
									pShortPoints[i].x = pPoints[i].x;
									pShortPoints[i].y = pPoints[i].y;
								}
								blobPoints = (LPSTR)BytesToBlob((LPBYTE)pShortPoints, nPnts*sizeof(POINTS));
								free(pShortPoints);
								np = -nPnts;
							}
							else
								blobPoints = PointsToBlob(pPoints, nPnts);
							if (nLoops > 1)
							{
								sprintf(pCmd, "INSERT INTO %s VALUES(%i,'%s',%.8f,%.8f,%i,%i,X'%s',X'%s');", ARG[4], Refno, HighlightData.PD.UDI, midPt.x, midPt.y, np, nLoops, blobParts, blobPoints);
								free(blobParts);
							}
							else
								sprintf(pCmd, "INSERT INTO %s VALUES(%i,'%s',%.8f,%.8f,%i,%i,X'',X'%s');", ARG[4], Refno, HighlightData.PD.UDI, midPt.x, midPt.y, np, nLoops, blobPoints);
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
					keepGoing = StatusWindowUpdate(NULL,pCmd, nRecs, ++nLoaded);
				}
				DestroyStatusWindow(0);
				GSSiClose(Fid);
				GSSiGlobUlFree(&hCmd);
			}
		}
	return rtn;
}

