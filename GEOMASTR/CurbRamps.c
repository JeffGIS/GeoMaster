#include "graphint.h"   
#include "gmextern.h"
#include "CRAPI.h"
#include "CurbRamps.h"

#define CURRENT_INTERSECTION_VERSION "7.0"

typedef struct {
	int intID, rampNum, retired;
} RAMPID;

static BOOL _ignoreErrorLog = FALSE;
static sqlite3 *database = NULL;

int getMPIntersectionFromDB(int intID, BOOL wantRamps, MPINTERSECTION * pMPInt);
void convertVersion(LPSTR str, int fromVer, int toVer,LPSTR fileID);
void convertVersion_1_to_2(LPSTR str);
void convertVersion_2_to_3(LPSTR str);
void convertVersion_3_to_4(LPSTR str, LPSTR fileID);
void convertVersion_4_to_5(LPSTR str, LPSTR fileID);
void convertVersion_5_to_6(LPSTR str, LPSTR fileID);
void convertVersion_6_to_7(LPSTR str, LPSTR fileID);

BOOL createIntersectionsTable(BOOL dropExistingTables);
int GetRampData(RampStruct * pRamp, sqlite3_stmt *statement);
int getAllRampIDs(LPSTR OutFile);
BOOL getRampFromDB(RAMPID * pRampID, RampStruct * pRamp, LPSTR wantPhotos);

static BOOL Execute(LPSTR cmd,LPSTR errFile);
BOOL UpdateFromFile(LPSTR file, BOOL convertInsert,BOOL insertFileID,int dbType,LPSTR errFile,LPINT ptotErrors,int checkPointOpt);


/*int getOffsetCoord:(MPIntersection *)mpint
x : (int)x
y : (int)y
{
	int offCoord = 0;

	short offx = (x - mpint.lev21x);
	short offy = (y - mpint.lev21y);

	offCoord = (int)MAKELONG(offx, offy);
	return offCoord;
}

-(int)getRampCoord:(MPIntersection *)mpint
ramp : (int)r
{
	int icoord = 0;
	MPRamp *ramp = mpint.ramps[r];

	if (ramp.rampExists)
	{
		icoord = [self getOffsetCoord : mpint
		x : ramp.lev21x
		y : ramp.lev21y];
	}

	return icoord;

}

-(void)setRampCoord:(MPIntersection *)mpint
ramp : (int)r
   from : (sqlite3_stmt *)statement
{
	int icol = (8 + (r - 1));
	int icoord = sqlite3_column_int(statement, icol);

	short offsetx = LOWORD(icoord);
	short offsety = HIWORD(icoord);
	MPRamp *ramp = mpint.ramps[r];
	int lev21x = (mpint.lev21x + offsetx);
	int lev21y = (mpint.lev21y + offsety);

	if (icoord)
	{
		ramp.lev21x = lev21x;
		ramp.lev21y = lev21y;
		ramp.rampExists = YES;
		ramp.isComplete = NO;
	}

	else
	{
		ramp.rampExists = NO;
		ramp.isComplete = YES;
	}
}

-(int)getXWalkCoord:(MPIntersection *)mpint
xWalk : (char)xw
{
	int icoord = 0;

	switch (xw)
	{
	case 'A':
		if (mpint.xWalkAExists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.xWalkAlev21x
			y : mpint.xWalkAlev21y];
		}

		break;

	case 'B':
		if (mpint.xWalkBExists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.xWalkBlev21x
			y : mpint.xWalkBlev21y];
		}

		break;

	case 'C':
		if (mpint.xWalkCExists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.xWalkClev21x
			y : mpint.xWalkClev21y];
		}

		break;

	case 'D':
		if (mpint.xWalkDExists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.xWalkDlev21x
			y : mpint.xWalkDlev21y];
		}

		break;
	}

	return icoord;
}

-(void)setXWalkCoord:(MPIntersection *)mpint
xWalk : (char)xw
	from : (sqlite3_stmt *)statement
{
	int icol = (20 + (xw - 'A'));
	int icoord = sqlite3_column_int(statement, icol);

	short offsetx = LOWORD(icoord);
	short offsety = HIWORD(icoord);

	switch (xw)
	{
	case 'A':
		if (icoord)
		{
			mpint.xWalkAlev21x = mpint.lev21x + offsetx;
			mpint.xWalkAlev21y = mpint.lev21y + offsety;
			mpint.xWalkAExists = YES;
		}

		else
			mpint.xWalkAExists = NO;

		break;

	case 'B':
		if (icoord)
		{
			mpint.xWalkBlev21x = mpint.lev21x + offsetx;
			mpint.xWalkBlev21y = mpint.lev21y + offsety;
			mpint.xWalkBExists = YES;
		}

		else
			mpint.xWalkBExists = NO;

		break;

	case 'C':
		if (icoord)
		{
			mpint.xWalkClev21x = mpint.lev21x + offsetx;
			mpint.xWalkClev21y = mpint.lev21y + offsety;
			mpint.xWalkCExists = YES;
		}

		else
			mpint.xWalkCExists = NO;

		break;

	case 'D':
		if (icoord)
		{
			mpint.xWalkDlev21x = mpint.lev21x + offsetx;
			mpint.xWalkDlev21y = mpint.lev21y + offsety;
			mpint.xWalkDExists = YES;
		}

		else

			mpint.xWalkDExists = NO;
		break;
	}
}

-(int)getSignalCoord:(MPIntersection*)mpint
signal : (int)sig
{
	int icoord = 0;

	switch (sig)
	{
	case 81:
		if (mpint.signal81Exists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.signal81lev21x
			y : mpint.signal81lev21y];
		}

		break;

	case 23:
		if (mpint.signal23Exists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.signal23lev21x
			y : mpint.signal23lev21y];
		}

		break;

	case 45:
		if (mpint.signal45Exists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.signal45lev21x
			y : mpint.signal45lev21y];
		}

		break;

	case 67:
		if (mpint.signal67Exists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.signal67lev21x
			y : mpint.signal67lev21y];
		}

		break;
	}

	return icoord;
}

-(void)setSignalCoord:(MPIntersection *)mpint
signal : (int)s
	 from : (sqlite3_stmt *)statement
{
	int icol = 24;

	switch (s)
	{
	case 81:
		break;

	case 23:
		icol++;
		break;

	case 45:
		icol += 2;
		break;

	case 67:
		icol += 3;
		break;
	}

	int icoord = sqlite3_column_int(statement, icol);

	short offsetx = LOWORD(icoord);
	short offsety = HIWORD(icoord);

	switch (s)
	{
	case 81:
		if (icoord)
		{
			mpint.signal81lev21x = mpint.lev21x + offsetx;
			mpint.signal81lev21y = mpint.lev21y + offsety;
			mpint.signal81Exists = YES;
		}

		else
			mpint.signal81Exists = NO;

		break;

	case 23:
		if (icoord)
		{
			mpint.signal23lev21x = mpint.lev21x + offsetx;
			mpint.signal23lev21y = mpint.lev21y + offsety;
			mpint.signal23Exists = YES;
		}

		else
			mpint.signal23Exists = NO;

		break;

	case 45:
		if (icoord)
		{
			mpint.signal45lev21x = mpint.lev21x + offsetx;
			mpint.signal45lev21y = mpint.lev21y + offsety;
			mpint.signal45Exists = YES;
		}

		else
			mpint.signal45Exists = NO;

		break;

	case 67:
		if (icoord)
		{
			mpint.signal67lev21x = mpint.lev21x + offsetx;
			mpint.signal67lev21y = mpint.lev21y + offsety;
			mpint.signal67Exists = YES;
		}

		else
			mpint.signal67Exists = NO;

		break;
	}
}*/
/*-(BOOL)storeIntersection:(MPIntersection *)currentIntersection
{
	BOOL rtn = FALSE;

	for (int i = 1; i < 13; i++)
	{
		MPRamp *ramp = [currentIntersection.ramps objectAtIndex : i];

		switch (i)
		{
		case 1:
			ramp.xWalkisComplete = ramp.rampExists ? !currentIntersection.xWalkAExists : YES;
			ramp.signalisComplete = !currentIntersection.signal23Exists;
			break;

		case 2:
			ramp.xWalkisComplete = ramp.rampExists ? !currentIntersection.xWalkAExists : YES;
			ramp.signalisComplete = !currentIntersection.signal81Exists;
			break;

		case 3:
			ramp.xWalkisComplete = ramp.rampExists ? !currentIntersection.xWalkBExists : YES;
			ramp.signalisComplete = !currentIntersection.signal45Exists;
			break;

		case 4:
			ramp.xWalkisComplete = ramp.rampExists ? !currentIntersection.xWalkBExists : YES;
			ramp.signalisComplete = !currentIntersection.signal23Exists;
			break;

		case 5:
			ramp.xWalkisComplete = ramp.rampExists ? !currentIntersection.xWalkCExists : YES;
			ramp.signalisComplete = !currentIntersection.signal67Exists;
			break;

		case 6:
			ramp.xWalkisComplete = ramp.rampExists ? !currentIntersection.xWalkCExists : YES;
			ramp.signalisComplete = !currentIntersection.signal45Exists;
			break;

		case 7:
			ramp.xWalkisComplete = ramp.rampExists ? !currentIntersection.xWalkDExists : YES;
			ramp.signalisComplete = !currentIntersection.signal81Exists;
			break;

		case 8:
			ramp.xWalkisComplete = ramp.rampExists ? !currentIntersection.xWalkDExists : YES;
			ramp.signalisComplete = !currentIntersection.signal67Exists;
			break;

		default:
			ramp.xWalkisComplete = YES;
			ramp.signalisComplete = YES;
			break;
		}
	}

	rtn = [self storeMPIntersectionToDB : currentIntersection];

	return rtn;
}

-(int)incrementLastDataFileNum
{
	BOOL opened = [self open];
	int rtn = [self.curbRampData incrementLastDataFileNum];
	[self close : opened];
	return rtn;
}

-(NSArray*)setCurrentRamps:(int)intID
{
	NSMutableArray * array = [NSMutableArray new];

	for (int i = 0; i<13; i++)
	{
		NSNumber * num = [[NSNumber alloc]initWithBool:NO];
		[array addObject:num];
	}

	BOOL opened = [self open];
	sqlite3_stmt *statement = NULL;

	NSString *cmd = [NSString stringWithFormat : @"SELECT rampNum FROM ramps WHERE intID = %i", intID];
	SQLOK(2222, sqlite3_prepare_v2(database, cmd.UTF8String, -1, &statement, NULL), database);

	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		int rampNum = sqlite3_column_int(statement, 0);
		if (rampNum && rampNum < 13)
		{
			NSNumber * num = [[NSNumber alloc]initWithBool:YES];
			[array replaceObjectAtIndex:rampNum withObject : num];
		}
	}

	SQLOK(223, sqlite3_finalize(statement), database);
	[self close : opened];
	return array;
}*/
BOOL UpdatePictureID(LPSTR PathName, int oldSequence, int newSequence)
{
	BOOL rtn = FALSE;
	char NewFile[MAX_PATH];
	LPSTR pDot;
	HFILE FidOld, FidNew;
#define MAX_LINE_LEN	8000
	char line[MAX_LINE_LEN + 4];
	char searchfor[] = "INSERT OR REPLACE INTO CURBRAMP_PICTURES VALUES(";
	int ln = strlen(searchfor);

	strcpy(NewFile, PathName);
	pDot = strrchr(NewFile, '.');
	if (!pDot || stricmp(pDot, ".sql"))
		return FALSE;
	strcpy(pDot, ".new");
	FidOld = GSSiOpenFile(PathName, 0, OF_READ);
	if (FidOld == HFILE_ERROR)
		return FALSE;
	FidNew = GSSiOpenFile(NewFile, 0, OF_CREATE);
	while (fgetstring(line, MAX_LINE_LEN, FidOld))
	{
		if (!strnicmp(line, searchfor,ln))
		{
			char newline[MAX_LINE_LEN];
			LPSTR pLoc = line;
			LPSTR pComma;
			int pictID;

			pLoc += ln;
			pComma = strchr(pLoc, ',');
			*pComma++ = 0;
			pictID = atoi(pLoc);
			pictID /= 100;
			pictID *= 100;
			pictID += newSequence;
			pComma = strchr(pComma, ',');
			*pComma++ = 0;
			sprintf(newline, "%s%i,%i,%s", searchfor, pictID, newSequence,pComma);

			fputstring(newline, FidNew);
		}
		else
			fputstring(line, FidNew);
	}
	GSSiClose2 (&FidOld);
	GSSiClose2 (&FidNew);
	GSSiRemove(PathName);
	rtn = GSSiRename(NewFile, PathName);
	return rtn;
}

BOOL LoadFilesInListInChronologicalSequence(LPSTR List, LPSTR DataBase, BOOL showProgress, int dbType, BOOL convertInsert,LPSTR errFile,BOOL addFileID,int checkPointOpt )
{
#define LINELEN	USHRT_MAX
	BOOL rtn = FALSE;
	int rc;
	int nTot=0, nDone = 0, totErrors = 0;
	LPSTR line = malloc(LINELEN);
	char tempFile[MAX_PATH];
	HFILE fidTemp;

	LPSTR file = malloc(1024);
	rc = sqlite3_open(DataBase, &database);
	if (rc == SQLITE_OK)
	{
		HFILE FidList = GSSiOpenFile(List, 0, OF_READ);
		if (FidList != HFILE_ERROR)
		{
			BOOL first = TRUE;
			Execute("BEGIN",0);

			sprintf(line, "DROP TABLE IF EXISTS SORTEDFILES;CREATE TABLE SORTEDFILES (TIME INT,FILEPATH CHAR(256));");
			if (Execute(line,0))
			{
				while (fgetstring(file, 258, FidList))
				{
					if (!first || !strchr(file, '\t'))
					{
						HFILE fid = GSSiOpenFile(file, 0, OF_READ);
						if (fid != HFILE_ERROR)
						{
							int time = 0;
							fgetstring(line, 1022, fid);
							LPSTR tloc = strstr(line, " Time ");
							if (tloc)
							{
								time = atoi(tloc + 6);
								sprintf(line, "INSERT INTO SORTEDFILES VALUES(%i,'%s');", time, file);
								Execute(line,0);
								nTot++;
							}
							GSSiClose2 (&fid);
						}
					}
					first = FALSE;
				}
			}
			Execute("COMMIT",0);
			GSSiClose2 (&FidList);

			GSSiGetTempFileName(0, "txt", 0, tempFile);
			fidTemp = GSSiOpenFile(tempFile, 0, OF_CREATE);
			sprintf(line, "SELECT FILEPATH FROM SORTEDFILES ORDER BY TIME ASC;");
			sqlite3_stmt *statement = NULL;
			if (sqlite3_prepare_v2(database, line, -1, &statement, 0) == SQLITE_OK)
			{
				while (sqlite3_step(statement) == SQLITE_ROW)
				{
					LPSTR filePath = (LPSTR)sqlite3_column_text(statement, 0);
					fputstring(filePath, fidTemp);
				}
			}
			sqlite3_finalize(statement);
			rc = sqlite3_close(database);
			GSSiClose2 (&fidTemp);
			rc = sqlite3_open(DataBase, &database);
			if (showProgress)
			{
				CreateStatusWind(hWndMain, 1, "Loading Data");
			}
			if (checkPointOpt)
				Execute("BEGIN", 0);
			fidTemp = GSSiOpenFile(tempFile, 0, OF_READ);
			{
				while (fgetstring (file,MAX_PATH,fidTemp))
				{
					BOOL st = UpdateFromFile(file, convertInsert, addFileID, dbType, errFile,&totErrors,checkPointOpt);
					if (showProgress)
					{
						char mess[128];
						sprintf(mess, "%i errors", totErrors);
						StatusWindowUpdate(0, mess, nTot, ++nDone);
					}

				}
			}
			if (showProgress)
				DestroyStatusWindow(0);
			GSSiClose2 (&fidTemp);
			GSSiRemove(tempFile);
		}
		if (checkPointOpt)
			Execute("COMMIT", 0);
		rc = sqlite3_close(database);
	}
	free(line);
	free(file);
	return rtn;
}
int OutputIntsWithRampsToFile(LPSTR OutFile, LPSTR NVCRISDataBase, int opt,int header)
{//opt=0 ALL opt=1 with ramps opt=2 complete opt=3 paid only
	//header=0 no header only int ID header=1 include header and street names and coord header=2 same with types
	int rtn = 0;
	char line[2048];
	char query[256];
	HFILE fid;

	int rc = sqlite3_open(NVCRISDataBase, &database);
	if (rc == SQLITE_OK)
	{
		sqlite3_stmt *statement = NULL;
		switch (opt)
		{
		default:
			sprintf(query, "SELECT DISTINCT intID FROM Intersections");
			break;
		case 1:
			sprintf(query, "SELECT DISTINCT intID FROM ramps");
			break;
		case 2:
			sprintf(query, "SELECT DISTINCT intID FROM ramps WHERE isComplete >= 1");
			break;
		case 3:
			sprintf(query, "SELECT DISTINCT intID FROM ramps WHERE isComplete = 2");
			break;
		}
		SQLOK(sqlite3_prepare_v2(database, query, -1, &statement, NULL), database, "get mpint", 0);
		fid = GSSiOpenFile(OutFile, 0, OF_CREATE);
		if (header == 1)
		{
			sprintf(line, "IntersectionNum\tLatitude\tLongitude\tStreet Names");
			fputstring(line, fid);
		}
		else if (header == 2)
		{
			sprintf(line, "IntersectionNum(B4)\tLatitude(R8)\tLongitude(R8)\tStreet Names(C254)");
			fputstring(line, fid);
		}
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			int intersectionID = sqlite3_column_int(statement, 0);
			if (intersectionID > 0)
			{
				if (!header)
				{
					itoa(intersectionID, line, 10);
					fputstring(line, fid);
				}
				else
				{
					char intquery[256];
					char FormattedStreets[1024];
					sqlite3_stmt *statement = NULL;
					sprintf(intquery, "SELECT streetNames,latitude,longitude FROM Intersections WHERE intID=%i", intersectionID);
					SQLOK(sqlite3_prepare_v2(database, intquery, -1, &statement, NULL), database, "get mpint", 0);
					if (sqlite3_step(statement) == SQLITE_ROW)
					{
						LPSTR pNames = (LPSTR)sqlite3_column_text(statement, 0);
						double lat = sqlite3_column_double(statement, 1);
						double lon = sqlite3_column_double(statement, 2);
						FormatStreets(pNames, FormattedStreets);
						sprintf(line, "%i\t%f\t%f\t%s", intersectionID, lat, lon, FormattedStreets);
						fputstring(line, fid);
					}
					SQLOK(sqlite3_finalize(statement), database, "get mpint", 0);
				}
				rtn++;
			}
		}
		SQLOK(sqlite3_finalize(statement), database, "get mpint", 0);
		rc = sqlite3_close(database);
		GSSiClose2 (&fid);
	}
	return rtn;
}

BOOL GetIntersectionStreetNames(LPSTR NVCRISDataBase, int intnum, LPSTR OutLoc,LPINT pAltIntNum)
{
	BOOL rtn = FALSE;
	int rc = SQLITE_OK;

	*OutLoc = 0;
	if (*NVCRISDataBase)
		rc = sqlite3_open(NVCRISDataBase, &database);
	if (rc == SQLITE_OK)
	{
		MPINTERSECTION *pmpInt = malloc(sizeof(MPINTERSECTION)+4);
		rtn = getMPIntersectionFromDB(intnum, FALSE, pmpInt);
		if (rtn)
			strcpy(OutLoc, pmpInt->name);
		free(pmpInt);
		if (pAltIntNum)
		{
			*pAltIntNum = GetAlternateIntersectionNum(intnum);
		}
		if (*NVCRISDataBase)
			sqlite3_close(database);
	}
	return rtn;
}

BOOL OutputRampsForIntersectionsInListToFile(LPSTR List, LPSTR OutFile, LPSTR NVCRISDataBase, int codeSystem, int headerType,BOOL wantPhotos)
{
	BOOL rtn = FALSE;
	int rc;

	rc = sqlite3_open(NVCRISDataBase, &database);
	if (rc == SQLITE_OK)
	{
		HFILE FidList = GSSiOpenFile(List, 0, OF_READ);
		if (FidList != HFILE_ERROR)
		{
			HFILE FidOut = GSSiOpenFile(OutFile, 0, OF_CREATE);
			if (FidOut != HFILE_ERROR)
			{
				LPSTR rampHeader = (LPSTR)rampToTextHeader(headerType,wantPhotos);
				fputstring(rampHeader, FidOut);
				LPSTR line = malloc(4096);
				MPINTERSECTION *pmpInt = malloc(sizeof(MPINTERSECTION)+4);
				while (fgetstring(line, sizeof(line)-2, FidList))
				{
					int intID = atoi(line);
					if (getMPIntersectionFromDB(intID, TRUE, pmpInt))
					{
						for (int i = 1; i < 13; i++)
						{
							RampStruct * pRamp = &pmpInt->ramps[i];
							if (pRamp->rampExists)
							{
								LPSTR rampText = rampToText(intID, pRamp, codeSystem,0);
								sprintf(line, "%s", rampText);
								fputstring(line, FidOut);
								free(rampText);
							}
						}
					}
					else
						ii = 1;
				}
				free(line);
				free(pmpInt);
				GSSiClose2 (&FidOut);
				rtn = TRUE;
			}
			GSSiClose2 (&FidList);
		}
		rc = sqlite3_close(database);

	}
	return rtn;
}
BOOL OutputRampToFile(int intNum, int rampNum, int retired, LPSTR OutFile, LPSTR NVCRISDataBase, int codeSystem, int headerType,BOOL wantPhotos)
{
	BOOL rtn = FALSE;
	char line[4096 * 2];
	int rc;
	LPSTR photos = 0;
	ToleranceValues tolerances;
	setStandardToleranceValues(&tolerances);

	rc = sqlite3_open(NVCRISDataBase, &database);
	if (rc == SQLITE_OK)
	{
		HFILE FidOut;
		
		if (headerType >= 0)
			FidOut = GSSiOpenFile(OutFile, 0, OF_CREATE);
		else
		{
			FidOut = GSSiOpenFile(OutFile, 0, OF_READWRITE);
			GSSillseek(FidOut,0,2);
		}
		if (FidOut != HFILE_ERROR)
		{
			char tempRampIDs[MAX_PATH];
			if (headerType >= 0)
			{
				LPSTR rampHeader = (LPSTR)rampToTextHeader(headerType,wantPhotos);
				fputstring(rampHeader, FidOut);
			}
			RAMPID rampID;
			rampID.intID = intNum;
			rampID.rampNum = rampNum;
			rampID.retired = retired;
			RampStruct ramp = { 0 };
			RampStruct * pRamp = &ramp;
			if (wantPhotos)
				photos = malloc(1024);
			if (getRampFromDB(&rampID, pRamp,photos))
			{
				if (pRamp->rampExists)
				{
					if (pRamp->rampType > 100)
					{
						//get bump data
						RampStruct rampBump = { 0 };
						RampStruct * pRampBump = &rampBump;
						RAMPID rampIDBump = rampID;
						rampIDBump.rampNum = getMiddleRampNumFromRampNum(rampID.rampNum);
						if (getRampFromDB(&rampIDBump, pRampBump,0))
						{
							pRamp->bumpHeight = pRampBump->bumpHeight;
							pRamp->bumpWidth = pRampBump->bumpWidth;
						}
					}
					LPSTR rampText = rampToText(intNum, pRamp, codeSystem,photos);
					sprintf(line, "%s", rampText);
					fputstring(line, FidOut);
					free(rampText);
				}
			}
			GSSiFree(&photos);
			GSSiClose2 (&FidOut);
			rtn = TRUE;
		}
		rc = sqlite3_close(database);

	}
	return rtn;
}

int getAllRampIDs (LPSTR OutFile)
{
	int nRamps = 0;
	MPINTERSECTION mpint;
	sqlite3_stmt *statement = NULL;
	LPSTR query = malloc(4096);
	RAMPID rampID;

	HFILE FidOut = GSSiOpenFile(OutFile, 0, OF_CREATE);
	if (FidOut != HFILE_ERROR)
	{
		sprintf(query, "SELECT intID,rampNum,Retired FROM Ramps WHERE rampExists > 0");

		SQLOK(sqlite3_prepare_v2(database, query, -1, &statement, NULL), database, "get mpint", 0);

		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			rampID.intID = sqlite3_column_int(statement, 0);
			rampID.rampNum = sqlite3_column_int(statement, 1);
			rampID.retired = sqlite3_column_int(statement, 2);
			BigWrite(FidOut, &rampID, sizeof(RAMPID), -1);
			nRamps++;
		}
		SQLOK(sqlite3_finalize(statement), database, "get mpint", 0);
		GSSiClose2 (&FidOut);
	}
	free(query);

	return nRamps;
}
BOOL getRampFromDB(RAMPID * pRampID, RampStruct * pRamp,LPSTR wantPhotos)
{
	BOOL rtn = FALSE;
	char query[1024];
	sqlite3_stmt *statement = NULL;

	sprintf(query, "SELECT rowid,* FROM Ramps WHERE intID=%i AND rampNum = %i AND retired = %i", pRampID->intID, pRampID->rampNum,pRampID->retired);
	SQLOK(sqlite3_prepare_v2(database, query, -1, &statement, NULL), database, "get mpint", 0);
	if (sqlite3_step(statement) == SQLITE_ROW)
	{
		int rampNum = GetRampData(pRamp, statement);
		rtn = TRUE;
	}
	SQLOK(sqlite3_finalize(statement), database, "get mpint", 0);
	if (wantPhotos)
	{
		int cornerID = getMiddleRampIDFromRampID(pRampID->rampNum);
		*wantPhotos = 0;

		sprintf(query, "SELECT rampNum,iPadNum, pictNum FROM CURBRAMP_PICTURES WHERE intID=%i", pRampID->intID);
		LPSTR pPos = wantPhotos;
		SQLOK(sqlite3_prepare_v2(database, query, -1, &statement, NULL), database, "get mpint", 0);
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			char pictName[32];
			int i = 0;
			int rampNum = sqlite3_column_int(statement, i++);
			int iPadNum = sqlite3_column_int(statement, i++);
			int pictNum = sqlite3_column_int(statement, i++);

			if (rampNum == pRampID->rampNum)
			{
				sprintf(pictName, "%i_%i.jpg",iPadNum, pictNum);
				strcpy(pPos, pictName);
				pPos = strchr(pPos, 0);
				pPos++;
				rtn = TRUE;
			}
		}
		SQLOK(sqlite3_finalize(statement), database, "get mpint", 0);
		SQLOK(sqlite3_prepare_v2(database, query, -1, &statement, NULL), database, "get mpint", 0);
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			char pictName[32];
			int i = 0;
			int rampNum = sqlite3_column_int(statement, i++);
			int iPadNum = sqlite3_column_int(statement, i++);
			int pictNum = sqlite3_column_int(statement, i++);

			if (rampNum != pRampID->rampNum && cornerID == getMiddleRampIDFromRampID(rampNum))
			{
				sprintf(pictName, "%i_%i.jpg", iPadNum, pictNum);
				strcpy(pPos, pictName);
				pPos = strchr(pPos, 0);
				pPos++;
				rtn = TRUE;
			}
		}
		*pPos++ = 0;
		*pPos = 0;
		SQLOK(sqlite3_finalize(statement), database, "get mpint", 0);

	}
	return rtn;
}
BOOL OutputAllRampsToFile(LPSTR OutFile, LPSTR NVCRISDataBase, int codeSystem, int headerType,BOOL wantPhotos)
{
	BOOL rtn = FALSE;
	char line[4096 * 2];
	int rc;
	LPSTR photos = 0;
	ToleranceValues tolerances;
	setStandardToleranceValues(&tolerances);

	rc = sqlite3_open(NVCRISDataBase, &database);
	if (rc == SQLITE_OK)
	{
		HFILE FidOut = GSSiOpenFile(OutFile, 0, OF_CREATE);
		if (FidOut != HFILE_ERROR)
		{
			char tempRampIDs[MAX_PATH];
			GSSiGetTempFileName(0, "gmc", 0, tempRampIDs);

			int nRamps = getAllRampIDs(tempRampIDs);
			HFILE FidList = GSSiOpenFile(tempRampIDs, 0, OF_READ);
			if (FidList != HFILE_ERROR)
			{
				LPSTR rampHeader = (LPSTR)rampToTextHeader(headerType,wantPhotos);
				fputstring(rampHeader, FidOut);
				RAMPID rampID;
				while (BigRead(FidList, &rampID, sizeof(RAMPID)))
				{
					RampStruct ramp = { 0 };
					RampStruct * pRamp = &ramp;
					if (wantPhotos)
						photos = malloc(1024);
					if (getRampFromDB(&rampID,pRamp,photos))
					{
						if (pRamp->rampExists)
						{
							LPSTR rampText = rampToText(rampID.intID, pRamp, codeSystem,photos);
							sprintf(line, "%s", rampText);
							fputstring(line, FidOut);
							free(rampText);
						}
					}
					GSSiFree(&photos);
				}
				GSSiClose2 (&FidList);
			}
			GSSiClose2 (&FidOut);
			rtn = TRUE;
		}
		rc = sqlite3_close(database);

	}
	return rtn;
}

BOOL OutputPriorityLocToFile(LPSTR OutFile, LPSTR NVCRISDataBase, int header)
{
	BOOL rtn = FALSE;
	char line[2048];
	char query[256];
	HFILE fid;
	//locationID(C0) Name(C0) Type(C0) Category(C0) Latitude(C0) Longitude(C0) Radius(C0) Offset(C0)
	int rc = sqlite3_open(NVCRISDataBase, &database);
	if (rc == SQLITE_OK)
	{
		sqlite3_stmt *statement = NULL;
		sprintf(query, "CREATE TABLE IF NOT EXISTS PriorityLocations ('locationID' INTEGER PRIMARY KEY,'Name' CHAR(256),'Type' INT, 'Category' CHAR(32),'Latitude' DOUBLE, 'Longitude' DOUBLE, 'Radius' DOUBLE, 'Offset' INT);SELECT * FROM PriorityLocations");
		SQLOK(sqlite3_prepare_v2(database, query, -1, &statement, NULL), database, "get mpint", 0);
		fid = GSSiOpenFile(OutFile, 0, OF_CREATE);
		if (header == 1)
		{
			sprintf(line, "LocationID\tName\tCategory\tLatitude\tLongitude\tRadius\tOffset");
			fputstring(line, fid);
		}
		else if (header == 2)
		{
			sprintf(line, "LocationID(B4)\tName(C128)\tCategory(C64)\tLatitude(R8)\tLongitude(R8)\tRadius(R8)\tOffset(R8)");
			fputstring(line, fid);
		}
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			int plID = sqlite3_column_int(statement, 0);
			LPSTR pName = (LPSTR)sqlite3_column_text(statement, 1);
			int type = sqlite3_column_int(statement, 2);
			LPSTR category = (LPSTR)sqlite3_column_text(statement, 3);
			double lat = sqlite3_column_double(statement, 4);
			double lon = sqlite3_column_double(statement, 5);
			double radius = sqlite3_column_double(statement, 6);
			double offset = sqlite3_column_double(statement, 7);
			sprintf(line, "%i\t%s\t%s\t%f\t%f\t%f\t%f", plID, pName, category, lat, lon, radius,offset);
			fputstring(line, fid);
			rtn++;
		}
		SQLOK(sqlite3_finalize(statement), database, "get pl", 0);
		rc = sqlite3_close(database);
		GSSiClose2 (&fid);
	}
	return rtn;
}

BOOL OutputRampsToFile(LPSTR OutFile, LPSTR NVCRISDataBase, int codeSystem, int headerType, int completionCode,BOOL wantPhotos)
{
	BOOL rtn = FALSE;
	int rc;
	int n = 0;
	int totRamps = 0;
	int nIntsNoRamps = 0;
	ToleranceValues tolerances;
	char tempfile[MAX_PATH];

	setStandardToleranceValues(&tolerances);
	GSSiGetTempFileName(0, "gmc", 0, tempfile);
	if (OutputIntsWithRampsToFile(tempfile, NVCRISDataBase, completionCode+1, 0))
	{
		HFILE FidList = GSSiOpenFile(tempfile, 0, OF_READ);

		rc = sqlite3_open(NVCRISDataBase, &database);
		if (rc == SQLITE_OK)
		{
			HFILE FidOut = GSSiOpenFile(OutFile, 0, OF_CREATE);
			if (FidOut != HFILE_ERROR)
			{
				LPSTR rampHeader = (LPSTR)rampToTextHeader(headerType,wantPhotos);
				fputstring(rampHeader, FidOut);
				LPSTR line = malloc(4096);
				MPINTERSECTION *pmpInt = malloc(sizeof(MPINTERSECTION)+4);
				while (fgetstring(line, 4094, FidList))
				{
					int intID = atoi(line);
					int nRamps = getMPIntersectionFromDB(intID, TRUE, pmpInt);
					if (nRamps > 0)
					{
						totRamps += nRamps;
						for (int i = 1; i < 13; i++)
						{
							RampStruct * pRamp = &pmpInt->ramps[i];
							if (pRamp->rampExists)
							{
								LPSTR rampText = rampToText(pmpInt->intID, pRamp, codeSystem,0);
								sprintf(line, "%s", rampText);
								fputstring(line, FidOut);
								free(rampText);
								n++;
							}
						}
					}
					else
						nIntsNoRamps++;
				}
				free(line);
				free(pmpInt);
				GSSiClose2 (&FidOut);
				rtn = TRUE;
			}
			GSSiClose2 (&FidList);
			GSSiRemove(tempfile);
		}
		rc = sqlite3_close(database);
	}
	return rtn;
}
BOOL OutputRampForIntersectionAndRampnumToFile(int intID, int rampNum, LPSTR OutFile, LPSTR NVCRISDataBase, int codeSystem, int headerType,BOOL wantPhotos)
{
	BOOL rtn = FALSE;
	OFSTRUCTGM OFStruct;
	HFILE fid;
	int rc;
	LPSTR photos = 0;
	ToleranceValues tolerances;
	setStandardToleranceValues(&tolerances);

	rampNum = fixRampNum(rampNum);
	if (rampNum < 0 || rampNum > 12)
		return FALSE;

	fid = GSSiOpenFile(NVCRISDataBase, &OFStruct, OF_READ);
	GSSiClose (fid);
	if (fid != HFILE_ERROR)
	{
		rc = sqlite3_open_v2(OFStruct.szPathName, &database, SQLITE_OPEN_READONLY, NULL);
		if (rc == SQLITE_OK)
		{
			HFILE FidOut;
			if (headerType < 0)
			{
				FidOut = GSSiOpenFile(OutFile, 0, OF_READWRITE);
				GSSillseek(FidOut, 0, 2);
			}
			else
				FidOut = GSSiOpenFile(OutFile, 0, OF_CREATE);
			if (FidOut != HFILE_ERROR)
			{
				if (headerType >= 0)
				{
					LPSTR rampHeader = (LPSTR)rampToTextHeader(headerType,wantPhotos);
					fputstring(rampHeader, FidOut);
				}
				LPSTR line = malloc(4096);
				MPINTERSECTION *pmpInt = malloc(sizeof(MPINTERSECTION)+4);
				if (getMPIntersectionFromDB(intID, TRUE, pmpInt))
				{
					RampStruct * pRamp = &pmpInt->ramps[rampNum];
					if (pRamp->rampExists)
					{
						LPSTR rampText = rampToText(pmpInt->intID, pRamp, codeSystem,photos);
						sprintf(line, "%s", rampText);
						fputstring(line, FidOut);
						free(rampText);
					}
					else
						ii = 1;
				}
				else
					ii = 1;
				free(line);
				free(pmpInt);
				GSSiClose2 (&FidOut);
				rtn = TRUE;
			}
			rc = sqlite3_close(database);

		}
	}
	return rtn;
}

BOOL ComplianceCodeForRamp(int intNum, int rampNum, int retired, LPSTR NVCRISDataBase, int codeSystem, LPSTR OutLoc)
{
	BOOL rtn = FALSE;
	OFSTRUCTGM OFStruct;
	ToleranceValues tolerances;
	setStandardToleranceValues(&tolerances);

	*OutLoc = 0;
	rampNum = fixRampNum(rampNum);
	HFILE fid = GSSiOpenFile(NVCRISDataBase, &OFStruct, OF_READ);
	GSSiClose (fid);
	if (fid != HFILE_ERROR)
	{

		int rc = sqlite3_open_v2(OFStruct.szPathName, &database, SQLITE_OPEN_READONLY, NULL);
		if (rc == SQLITE_OK)
		{
			RAMPID rampID;
			rampID.intID = intNum;
			rampID.rampNum = rampNum;
			rampID.retired = retired;
			RampStruct ramp = { 0 };
			RampStruct * pRamp = &ramp;
			if (getRampFromDB(&rampID, pRamp,0))
			{
				if (pRamp->rampExists)
				{
					LPSTR detailCode;
					LPSTR ccode = rampComplianceCode(pRamp, &detailCode, &tolerances, codeSystem);
					strcpy(OutLoc, ccode);
					free(ccode);
					free(detailCode);
				}
			}
			rc = sqlite3_close(database);
		}
	}
	return rtn;
}
int getMiddleRampNumFromRampNum(int rampNum)
{
	int rtn = rampNum;

	switch (rampNum)
	{
	case 1:
	case 8:
		rtn = 12;
		break;
	case 3:
	case 2:
		rtn = 9;
		break;
	case 5:
	case 4:
		rtn = 10;
		break;
	case 7:
	case 6:
		rtn = 11;
		break;
	}
	return rtn;
}

int getMiddleRampIDFromRampID(int rampID)
{
	int rtn = rampID;

	switch (rampID)
	{
	case 1:
	case 8:
		rtn = 12;
		break;
	case 3:
	case 2:
		rtn = 9;
		break;
	case 5:
	case 4:
		rtn = 10;
		break;
	case 7:
	case 6:
		rtn = 11;
		break;
	}
	switch (rtn)
	{
	case 9:
	case 23:
		rtn = 23;
		break;
	case 10:
	case 45:
		rtn = 45;
		break;
	case 11:
	case 67:
		rtn = 67;
		break;
	case 12:
	case 81:
		rtn = 81;
		break;
	default:
		rtn = 0;
	}
	return rtn;
}

static BOOL getCornerComment(int intID, int rampNum, LPSTR rampComment)
{
	BOOL rtn = FALSE;
	int cornerID = getMiddleRampIDFromRampID(rampNum);
	sqlite3_stmt *statement = NULL;
	
	*rampComment = 0;
	LPSTR query = malloc(4096);
	sprintf(query, "SELECT note FROM CURBRAMP_NOTES WHERE intID=%i AND corner=%i", intID,cornerID);

	SQLOK(sqlite3_prepare_v2(database, query, -1, &statement, NULL), database, "get mpint", 0);

	if (sqlite3_step(statement) == SQLITE_ROW)
	{
		LPSTR note = (LPSTR)sqlite3_column_text(statement,0);
		if (note)
			strncpy0(rampComment,note,254);
	}
	SQLOK(sqlite3_finalize(statement), database, "get mpint", 0);
	free(query);

	return rtn;
}

BOOL RampIDFromRampNum(int rampNum,LPSTR rampID)
{
	BOOL rtn = FALSE;
	int i = fixRampNum (rampNum);

	*rampID = 0;
	
	switch (i)
	{
	default:
	if (i % 2)
		sprintf (rampID,"%i(Left)", i);

	else
		sprintf(rampID, "%i(Right)", i);
	break;

	case 9:
		sprintf(rampID, "23 (Middle)");
	break;

	case 10:
		sprintf(rampID, "45 (Middle)");
	break;

	case 11:
		sprintf(rampID, "67 (Middle)");
	break;

	case 12:
		sprintf(rampID, "81 (Middle)");
	break;
	}
	return rtn;
}

void MPIntersectionInit(MPINTERSECTION * mpint)
{
	memset(mpint, 0, sizeof(MPINTERSECTION));
}
int getMPIntersectionFromDB(int intID, BOOL wantRamps,MPINTERSECTION * pMPInt)
{
	int nRamps = 0;
	MPINTERSECTION mpint;
	MPIntersectionInit(&mpint);
	sqlite3_stmt *statement = NULL;
	double lat = 0.0;
	double lon = 0.0;
	BOOL haveIntersection = FALSE;

	LPSTR query = malloc(4096);
	sprintf(query, "SELECT * FROM Intersections WHERE intID=%i", intID);

	SQLOK(sqlite3_prepare_v2(database, query, -1, &statement, NULL), database, "get mpint", 0);

	if (sqlite3_step(statement) == SQLITE_ROW)
	{
		LPSTR streets, comment;
		char streetString[1024] = { 0 };
		int recID;

		haveIntersection = TRUE;
		mpint.intID = intID;
		recID = sqlite3_column_int(statement, 0);
		mpint.lastUpdate = sqlite3_column_int(statement, 1);
		mpint.status = sqlite3_column_int(statement, 2);
		mpint.assignedPrelim = sqlite3_column_int(statement, 3);
		mpint.assignedDetail = sqlite3_column_int(statement, 4);
		mpint.lev21x = sqlite3_column_int(statement, 5);
		mpint.lev21y = sqlite3_column_int(statement, 6);
		mpint.intRotation = sqlite3_column_int(statement, 7);
		PixelXYToLatLong(mpint.lev21x, mpint.lev21y, 21, &lat, &lon);
		mpint.lat = lat;
		mpint.lon = lon;
		int nStreets = sqlite3_column_int(statement, 28);
		streets = (LPSTR)sqlite3_column_text(statement, 29);
		if (streets && nStreets)
			//strcpy(streetString, streets);
			FormatStreets(streets, streetString);
		//ReplaceChar(streetString, '|', '\n');
		strncpy0(mpint.name, streetString, sizeof(mpint.name) - 1);
		comment = (LPSTR)sqlite3_column_text(statement, 33);
		if (comment && *comment)
			strncpy0(mpint.intersectionComment, comment, sizeof(mpint.intersectionComment) - 1);
		mpint.doLater = sqlite3_column_int(statement, 34);
		mpint.zoomLevel = sqlite3_column_int(statement, 35);
		/*[self setRampCoord : mpint ramp : 1 from : statement];
		[self setRampCoord : mpint ramp : 2 from : statement];
		[self setRampCoord : mpint ramp : 3 from : statement];
		[self setRampCoord : mpint ramp : 4 from : statement];
		[self setRampCoord : mpint ramp : 5 from : statement];
		[self setRampCoord : mpint ramp : 6 from : statement];
		[self setRampCoord : mpint ramp : 7 from : statement];
		[self setRampCoord : mpint ramp : 8 from : statement];
		[self setRampCoord : mpint ramp : 9 from : statement];
		[self setRampCoord : mpint ramp : 10 from : statement];
		[self setRampCoord : mpint ramp : 11 from : statement];
		[self setRampCoord : mpint ramp : 12 from : statement];
		[self setXWalkCoord : mpint xWalk : 'A' from : statement];
		[self setXWalkCoord : mpint xWalk : 'B' from : statement];
		[self setXWalkCoord : mpint xWalk : 'C' from : statement];
		[self setXWalkCoord : mpint xWalk : 'D' from : statement];
		[self setSignalCoord : mpint signal : 81 from : statement];
		[self setSignalCoord : mpint signal : 23 from : statement];
		[self setSignalCoord : mpint signal : 45 from : statement];
		[self setSignalCoord : mpint signal : 67 from : statement];
		[mpint setRampsXWalkAndSignal];*/
		nRamps = -1;
	}
	SQLOK(sqlite3_finalize(statement), database, "get mpint", 0);
	if (wantRamps && haveIntersection)
	{
		sprintf(query, "SELECT rowid,* FROM Ramps WHERE intID=%i", intID);
		SQLOK(sqlite3_prepare_v2(database, query, -1, &statement, NULL), database, "get mpint", 0);
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			RampStruct ramp = { 0 };
			if (nRamps == -1)
				nRamps = 0;
			int rampNum = GetRampData(&ramp, statement);
//			if (mpint.timeComplete >= mpint.ramps[rampNum].timeComplete)
			if (rampNum > 0 && rampNum < 13)
				mpint.ramps[rampNum] = ramp;
			nRamps++;
		}
		SQLOK(sqlite3_finalize(statement), database, "get mpint", 0);
	}
	free(query);

	*pMPInt = mpint;
	if (nRamps < 1)
	{
		ii = 1;
	}
	return nRamps;
}

int GetRampData(RampStruct * pRamp, sqlite3_stmt *statement)
{
	int i = 0;
	int uniqueID = sqlite3_column_int(statement, i++);
	int intersectionID = sqlite3_column_int(statement, i++);
	int rampNum = sqlite3_column_int(statement, i++);
	rampNum = fixRampNum(rampNum);

	pRamp->uniqueID = uniqueID;

	pRamp->rampNum = rampNum;
	LPSTR rampID = (LPSTR)sqlite3_column_text(statement, i++);
	strncpy0(pRamp->rampID, rampID, sizeof(pRamp->rampID) - 1);
	Strip(pRamp->rampID, ' ');
	pRamp->yearRebuilt = sqlite3_column_int(statement, i++);
	pRamp->timeComplete = sqlite3_column_int(statement, i++);
	pRamp->rampExists = sqlite3_column_int(statement, i++);
	pRamp->isComplete = sqlite3_column_int(statement, i++);
	pRamp->approximateHeading = sqlite3_column_double(statement, i++);
	pRamp->adjustedRot = sqlite3_column_int(statement, i++);
	pRamp->rampInXWalk = sqlite3_column_int(statement, i++);
	pRamp->xWalkisComplete = sqlite3_column_int(statement, i++);
	pRamp->signalisComplete = sqlite3_column_int(statement, i++);
	pRamp->texture = sqlite3_column_int(statement, i++);
	pRamp->upperLandingObstruction = sqlite3_column_int(statement, i++);
	pRamp->lowerLandingObstruction = sqlite3_column_int(statement, i++);
	pRamp->rampObstruction = sqlite3_column_int(statement, i++);
	pRamp->hasRampCracks = sqlite3_column_int(statement, i++);
	pRamp->hasUpperLandingCracks = sqlite3_column_int(statement, i++);
	pRamp->hasStreetLandingCracks = sqlite3_column_int(statement, i++);
	pRamp->rampWidth = sqlite3_column_int(statement, i++);
	pRamp->rampDepth = sqlite3_column_int(statement, i++);
	pRamp->rampSlopeFront = sqlite3_column_double(statement, i++);
	pRamp->rampSlopeSide = sqlite3_column_double(statement, i++);
	pRamp->rampSlopeHeading = sqlite3_column_double(statement, i++);
	pRamp->upperLandingSlopeFront = sqlite3_column_double(statement, i++);
	pRamp->upperLandingSlopeSide = sqlite3_column_double(statement, i++);
	pRamp->upperLandingSlopeHeading = sqlite3_column_double(statement, i++);
	pRamp->streetLandingSlopeFront = sqlite3_column_double(statement, i++);
	pRamp->streetLandingSlopeSide = sqlite3_column_double(statement, i++);
	pRamp->streetLandingSlopeHeading = sqlite3_column_double(statement, i++);
	pRamp->flareLeftSlopeFront = sqlite3_column_double(statement, i++);
	pRamp->flareLeftSlopeSide = sqlite3_column_double(statement, i++);
	pRamp->flareLeftSlopeHeading = sqlite3_column_double(statement, i++);
	pRamp->flareRightSlopeFront = sqlite3_column_double(statement, i++);
	pRamp->flareRightSlopeSide = sqlite3_column_double(statement, i++);
	pRamp->flareRightSlopeHeading = sqlite3_column_double(statement, i++);
	pRamp->swkLeftSlopeFront = sqlite3_column_double(statement, i++);
	pRamp->swkLeftSlopeSide = sqlite3_column_double(statement, i++);
	pRamp->swkLeftSlopeHeading = sqlite3_column_double(statement, i++);
	pRamp->swkRightSlopeFront = sqlite3_column_double(statement, i++);
	pRamp->swkRightSlopeSide = sqlite3_column_double(statement, i++);
	pRamp->swkRightSlopeHeading = sqlite3_column_double(statement, i++);
	pRamp->PEDSignalType = sqlite3_column_int(statement, i++);
	pRamp->PEDButtonType = sqlite3_column_int(statement, i++);
	pRamp->PEDButtonHeight = sqlite3_column_int(statement, i++);
	pRamp->PEDButtonDist = sqlite3_column_int(statement, i++);
	pRamp->SteepTopOfCurb = sqlite3_column_double(statement, i++);
	pRamp->PedRampLip = sqlite3_column_double(statement, i++);
	pRamp->lev21x = sqlite3_column_int(statement, i++);
	pRamp->lev21y = sqlite3_column_int(statement, i++);
	PixelXYToLatLong(pRamp->lev21x, pRamp->lev21y, 21, &pRamp->latitude, &pRamp->longitude);
	pRamp->rampType = sqlite3_column_int(statement, i++);
	LPSTR comment = (LPSTR)sqlite3_column_text(statement, i++);
	if (comment && *comment)
		strcpy(pRamp->rampComment, comment);
	else
		getCornerComment(intersectionID, rampNum, pRamp->rampComment);
	pRamp->awi = sqlite3_column_int(statement, i++);
	pRamp->hasLocatorTone = sqlite3_column_int(statement, i++);
	pRamp->hasInfoSign = sqlite3_column_int(statement, i++);
	pRamp->hasBraille = sqlite3_column_int(statement, i++);
	pRamp->hasTactileArrow = sqlite3_column_int(statement, i++);
	pRamp->locatorToneVolume = sqlite3_column_int(statement, i++);
	pRamp->audibleWalkIndicationVolume = sqlite3_column_int(statement, i++);
	CrackWidth cw;
	cw.rampCrackWidth = sqlite3_column_double(statement, i++);
	cw.upperLandingCrackWidth = sqlite3_column_double(statement, i++);
	cw.streetLandingCrackWidth = sqlite3_column_double(statement, i++);
	cw.leftSidewalkCrackWidth = sqlite3_column_double(statement, i++);
	cw.rightSidewalkCrackWidth = sqlite3_column_double(statement, i++);
	if (!pRamp->hasRampCracks && cw.rampCrackWidth > 0)
		pRamp->hasRampCracks = 1;
	if (!pRamp->hasUpperLandingCracks && cw.upperLandingCrackWidth > 0)
		pRamp->hasUpperLandingCracks = 1;
	if (!pRamp->hasStreetLandingCracks && cw.streetLandingCrackWidth > 0)
		pRamp->hasStreetLandingCracks = 1;

	pRamp->hasLeftSidewalkCracks = cw.leftSidewalkCrackWidth > 0;
	pRamp->hasRightSidewalkCracks = cw.rightSidewalkCrackWidth > 0;
	pRamp->crackWidth = cw;
	pRamp->curbCutDistance = sqlite3_column_double(statement, i++);
	pRamp->bumpWidth = sqlite3_column_double(statement, i++);
	pRamp->bumpHeight = sqlite3_column_double(statement, i++);
	pRamp->dwWidth = sqlite3_column_double(statement, i++);
	pRamp->dwDepth = sqlite3_column_double(statement, i++);
	LPSTR fileID = (LPSTR)sqlite3_column_text(statement, i++);
	if (fileID)
		strncpy0(pRamp->fileID, fileID, sizeof(pRamp->fileID) - 1);
	pRamp->cornerID = sqlite3_column_int(statement, i++);
	pRamp->retired = sqlite3_column_int(statement, i++);
	LPSTR rampStatus = (LPSTR)sqlite3_column_text(statement, i++);
	if (rampStatus)
		strncpy0(pRamp->rampStatus, rampStatus, sizeof(pRamp->rampStatus) - 1);
	pRamp->rampCode = sqlite3_column_int(statement, i++);
 	pRamp->proximityScore = sqlite3_column_int(statement, i++);
	LPSTR rampNotes = (LPSTR)sqlite3_column_text(statement, i++);
	if (rampNotes && *rampNotes)
		strcpy(pRamp->rampNotes, rampNotes);
	LPSTR ccode = (LPSTR)sqlite3_column_text(statement, i++);
	LPSTR ccodedetail = (LPSTR)sqlite3_column_text(statement, i++);
	LPSTR lastupdate = (LPSTR)sqlite3_column_text(statement, i++);
	pRamp->proximityValue = sqlite3_column_int(statement, i++);
	if (lastupdate && *lastupdate)
		strcpy(pRamp->lastUpdate, lastupdate);
	if (pRamp->bumpWidth > 0 || pRamp->bumpHeight > 0)
		ii = 1;
	return rampNum;

}
int GetAlternateIntersectionNum(int intID)
{
	sqlite3_stmt *statement = NULL;
	int altInt = 0;
	LPSTR query = malloc(4096);
	sprintf(query, "SELECT TrafficIntID FROM IntersectionXRef WHERE NVCRISIntID=%i", intID);
	if (sqlite3_prepare_v2(database, query, -1, &statement, NULL) == SQLITE_OK)
	{
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			altInt = sqlite3_column_int(statement, 0);
		}
		SQLOK(sqlite3_finalize(statement), database, "get altint", 0);
	}
	free(query);
	return altInt;
}
int FormatStreets(LPSTR from, LPSTR outtext)
{
	int nStreets = 0;
	int lfrom = strlen(from);
	int istreet = 0;

	if (lfrom > 0)
	{
		LPSTR streets = malloc(lfrom + 32);
		LPSTR pBar = streets;
		LPSTR pStreet[16];
		strcpy(streets, from);
		pStreet[0] = streets;
		nStreets++;
		while ((pBar = strchr(pBar, '|')) && nStreets < 4)
		{
			*pBar++ = 0;
			pStreet[nStreets++] = pBar;
		}
		strcpy(outtext, pStreet[istreet++]);
		while (istreet < nStreets)
		{
			for (int i = 0; i < istreet; i++)
			{
				if (!stricmp(pStreet[istreet], pStreet[i]))
					goto skip;
			}
			sprintf(strchr(outtext,0), " and %s", pStreet[istreet]);
		skip:
			istreet++;
		}
		free(streets);
	}
	return nStreets;
}
BOOL GetFromCodeText(int from, LPSTR text)
{
	char *fromText[] = { "Ramp", "Ramp", "Signal", "Texture", "Obstruction", "Steep TOC", "Crack", "Curb Cut", "Bump Width", "Bump Height", "Manual Slope","","","","","","","Ground Front","Ground Back","Top Down" };

	if (from > 0 && from < 20)
	{
		strcpy(text, fromText[from - 1]);
		return TRUE;
	}
	*text = 0;
	return FALSE;
}

static BOOL Execute(LPSTR cmd,LPSTR errFile)
{
	SetSQLiteErrFile(errFile);
	BOOL rtn = !SQLOK(sqlite3_exec(database, cmd, 0, 0, 0), database, "", 0);
	SetSQLiteErrFile("");

	return rtn;
}

BOOL UpdateFromFile(LPSTR file,BOOL convertInsert,BOOL insertFileID,int dbType,LPSTR errFile,LPINT ptotErrors,int checkPointOpt)
{
	BOOL rtn = TRUE;
	int line = 0;
	char fileID[32] = { "" };
	char searchFor[] = "CREATE TABLE";
	char searchFor1[] = "DROP TABLE";
	char searchFor2[] = "INSERT OR REPLACE INTO Ramps VALUES(";
	int lenSearch = strlen(searchFor);
	int lenSearch1 = strlen(searchFor1);
	int lenSearch2 = strlen(searchFor2);
	LPSTR pBS = strrchr(file, '\\');
	if (pBS)
	{
		LPSTR pDot;
		strcpy(fileID, ++pBS);
		pDot = strrchr(fileID, '.');
		if (pDot)
			*pDot = 0;
	}
	HFILE fid = GSSiOpenFile(file, 0, OF_READ);
	int totLen = GSSifilelength(fid);
	int maxLineLen = totLen + 2;
	LPSTR str = malloc(totLen + 4096);
	int err;
	LPSTR filename = strrchr(file, '\\');
	if (!filename)
		filename = file;
	if (!checkPointOpt)
		err = Execute("BEGIN", errFile);
	if (fgetstring(str, maxLineLen, fid))
	{
		int fromVer = atoi(CURRENT_INTERSECTION_VERSION);
		int toVer = atoi(CURRENT_INTERSECTION_VERSION);
		LPSTR vloc = strstr(str, "DBVer ");
		if (vloc)
			fromVer = atoi(vloc + 6);
		line++;
		while (rtn && fgetstring(str, maxLineLen, fid))
		{
			line++;
			if (!dbType)
				convertVersion(str, fromVer, toVer,fileID);
			if (insertFileID && toVer < 4)
			{
				if (!strnicmp(str, searchFor, lenSearch))
				{
					*str = 0;
				}
				else if (!strnicmp(str, searchFor1, lenSearch1))
				{
					*str = 0;
				}
				else if (!strnicmp(str, searchFor2, lenSearch2))
				{
					LPSTR pLoc = strchr(str,0);

					pLoc -= 2;
					if (*pLoc == ')')
						sprintf(pLoc, ",'%s');", fileID);
				}
			}
			if (convertInsert)
				REPLAC(str, "INSERT INTO", "INSERT OR REPLACE INTO", maxLineLen + 4090);
			rtn = Execute(str, errFile);
			if (!rtn && *errFile)
			{
				LPSTR mess = malloc(USHRT_MAX);
				sprintf(mess, " in file %s at line %i\r\n%s", filename, line, str);
				AppendFile(errFile, mess);
				free(mess);
				rtn = TRUE;
				if (ptotErrors)
					*ptotErrors = *ptotErrors + 1;
			}
		}
	}
	GSSiClose2 (&fid);
	if (!rtn && !*errFile && !checkPointOpt) //file has to be edited on server (by GSSi) before more data can be loaded
	{
		err = Execute("ROLLBACK", errFile);
		LPSTR mess = malloc(USHRT_MAX);
		sprintf(mess, " in file %s at line %i\r\n%s", filename, line, str);
		if (*errFile)
			AppendFile(errFile, mess);
		else
			GSSiMessageBox(2, mess, "Data load failure", MB_ICONEXCLAMATION, 0);
		free(mess);
	}
	else if (!checkPointOpt)
		err = Execute("COMMIT", errFile);
	free(str);
	return rtn;
}

void convertVersion(LPSTR str, int fromVer, int toVer,LPSTR fileID)
{
	int version = fromVer;

	while (version < toVer)
	{
		switch (version++)
		{
		case 1:
			convertVersion_1_to_2(str);
			break;
		case 2:
			convertVersion_2_to_3(str);
			break;
		case 3:
			convertVersion_3_to_4(str, fileID);
			break;
		case 4:
			convertVersion_4_to_5(str, fileID);
			break;
		case 5:
			convertVersion_5_to_6(str, fileID);
			break;
		case 6:
			convertVersion_6_to_7(str, fileID);
			break;
		}
	}
	return;
}
void convertVersion_1_to_2(LPSTR str)
{
	LPSTR ploc = strstr(str, "INSERT OR REPLACE INTO Ramps VALUES(");
	if (ploc)
	{
		LPSTR pEnd = strrchr(ploc, ')');
		sprintf(pEnd, ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);");
	}
}

void convertVersion_2_to_3(LPSTR str)
{
	LPSTR ploc = strstr(str, "INSERT OR REPLACE INTO Ramps VALUES(");
	if (ploc)
	{
		LPSTR pEnd = strrchr(ploc, ')');
		sprintf(pEnd, ",0,0);");
	}
	else
	{
		ploc = strstr(str, "INSERT INTO RAMPS VALUES(");
		if (ploc)
		{
			LPSTR pEnd = strrchr(ploc, ')');
			sprintf(pEnd, ",0,0);");
		}
	}
}

void convertVersion_3_to_4(LPSTR str, LPSTR fileID)
{
	char searchStr[] = "INSERT OR REPLACE INTO Ramps VALUES(";
	LPSTR ploc = strstr(str, searchStr);
	if (ploc)
	{
		LPSTR pid = ploc + strlen(searchStr);
		LPSTR prn = strchr(pid, ',');
		if (prn)
		{
			int rampNum = atoi(++prn);
			if (rampNum > 12)
			{
				char rampNumC[4];
				rampNum = fixRampNum(rampNum);
				sprintf(rampNumC, "%2i", rampNum);
				memmove(prn, rampNumC, 2);
			}
			int cornerID = getMiddleRampIDFromRampID(rampNum);
			LPSTR pEnd = strrchr(ploc, ')');
			sprintf(pEnd, ",'%s',%i,0);", fileID, cornerID);
		}
	}
	else
	{
		char searchStr[] = "INSERT INTO RAMPS VALUES(";
		ploc = strstr(str, searchStr);
		if (ploc)
		{
			LPSTR pid = ploc + strlen(searchStr);
			LPSTR prn = strchr(pid, ',');
			if (prn)
			{
				int rampNum = atoi(++prn);
				if (rampNum > 12)
				{
					char rampNumC[4];
					rampNum = fixRampNum(rampNum);
					sprintf(rampNumC, "%2i", rampNum);
					memmove(prn, rampNumC, 2);
				}
				int cornerID = getMiddleRampIDFromRampID(rampNum);
				LPSTR pEnd = strrchr(ploc, ')');
				sprintf(pEnd, ",'%s',%i,0);", fileID, cornerID);
			}
		}
	}
}
void convertVersion_4_to_5(LPSTR str, LPSTR fileID)
{

}
void convertVersion_5_to_6(LPSTR str, LPSTR fileID)
{
	{
		LPSTR ploc = strstr(str, "INSERT OR REPLACE INTO Ramps VALUES(");
		if (ploc)
		{
			LPSTR pEnd = strrchr(ploc, ')');
			sprintf(pEnd, ",'',0,0);");
		}
	}
}
void convertVersion_6_to_7(LPSTR str, LPSTR fileID)
{
	{
		LPSTR ploc = strstr(str, "INSERT OR REPLACE INTO Ramps VALUES(");
		if (ploc)
		{
			LPSTR pEnd = strrchr(ploc, ')');
			sprintf(pEnd, ",'','','','',0);");
		}
	}
}

int getDatasetVersion(void)
{
	int rtn = -1;

	if (database)
	{
		char query[256] = "SELECT max(VersionID) FROM VERSION";

		sqlite3_stmt *statement = NULL;

		SQLOK(sqlite3_prepare_v2(database, query, -1, &statement, NULL), database, "get version", 0);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			LPSTR version = (LPSTR)sqlite3_column_text(statement, 0);

			if (version)
				rtn = atoi(version);
		}
		sqlite3_finalize(statement);
	}

	return rtn;
}
BOOL ComputeCCCodes(int intID, int rampNum, int retired, int which, LPSTR OutLoc) // retrieves both summary and detail sep by |, if which 0 retrieves current , 1 computes new
{
	BOOL rtn = FALSE;
	RAMPID rampID;
	ToleranceValues tolerances;

	*OutLoc = 0;
	if (which)
	{
		setStandardToleranceValues(&tolerances);

		rampID.intID = intID;
		rampID.rampNum = rampNum;
		rampID.retired = retired;
		RampStruct ramp = { 0 };
		RampStruct * pRamp = &ramp;
		if (getRampFromDB(&rampID, pRamp,0))
		{
			if (pRamp->rampExists)
			{
				LPSTR detailCode;
				LPSTR ccode = rampComplianceCode(pRamp, &detailCode, &tolerances, 1);
				sprintf(OutLoc, "%s|%s", ccode, detailCode);
				free(ccode);
				free(detailCode);
				rtn = TRUE;
			}
		}
	}
	else
	{
		sqlite3_stmt *statement;
		char cmd[256];
		sprintf(cmd, "SELECT CCSUMMARY,CCDETAIL FROM RAMPS WHERE intID=%i AND rampNum=%i AND retired=%i", intID, rampNum, retired);
		SQLOK(SQLitePrepare(database, cmd, -1, &statement, 0), database, "getcc", 0);
		if (sqlite3_step(statement) == SQLITE_ROW)
		{
			LPSTR ccode = (LPSTR)sqlite3_column_text(statement, 0);
			LPSTR detailCode = (LPSTR)sqlite3_column_text(statement, 1);
			sprintf(OutLoc, "%s|%s", ccode, detailCode);
			rtn = TRUE;
		}
		SQLOK(SQLiteFinalize(statement), database, "updatedb", 0);
	}
	return rtn;
}

BOOL adjustToLatestVersion(LPSTR fromPath)
{
	BOOL rtn = TRUE;
	if (!database)
		return FALSE;
	int currentVersion = getDatasetVersion();
	int latestVersion = atoi (CURRENT_INTERSECTION_VERSION);
	char cmd[1024];
	BOOL needToReload = FALSE;

	for (int version = currentVersion; version < latestVersion;version++)
	{
		if (rtn)
		switch (version)
		{
		case 6://convert version 6 to version 7
		{
			BOOL st = TRUE;
			rtn = FALSE;
			SLT_StartTrans(database);
			strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN RampNotes CHAR(1024);");
			if (st) st = executeCmd(cmd);
			strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN CCSummary CHAR(32);");
			if (st) st = executeCmd(cmd);
			strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN CCDetail CHAR(64);");
			if (st) st = executeCmd(cmd);
			strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN LastUpdate CHAR(32);");
			if (st) st = executeCmd(cmd);
			strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN ProximityValue INT;");
			if (st) st = executeCmd(cmd);
			sprintf(cmd, "UPDATE Version SET VersionID = '%0.1f' WHERE vid = 1;", (double)(version + 1));
			if (st) st = executeCmd(cmd);
			if (st)
			{
				char cmd[256] = "SELECT intID,rampNum,retired FROM RAMPS";
				sqlite3_stmt *statement;
				RAMPID rampID;
				ToleranceValues tolerances;
				setStandardToleranceValues(&tolerances);

				SQLOK(SQLitePrepare(database, cmd, -1, &statement, 0), database, "updatedb", 0);
				while (sqlite3_step(statement) == SQLITE_ROW)
				{
					rampID.intID = sqlite3_column_int(statement, 0);
					rampID.rampNum = sqlite3_column_int(statement, 1);
					rampID.retired = sqlite3_column_int(statement, 2);
					RampStruct ramp = { 0 };
					RampStruct * pRamp = &ramp;
					if (getRampFromDB(&rampID, pRamp,0))
					{
						if (pRamp->rampExists)
						{
							LPSTR detailCode;
							LPSTR ccode = rampComplianceCode(pRamp, &detailCode, &tolerances, 1);
							sprintf(cmd, "UPDATE RAMPS SET CCSummary = '%s',CCDetail='%s' WHERE intID=%i AND rampNum=%i AND retired=%i", ccode, detailCode, rampID.intID, rampID.rampNum, rampID.retired);
							free(ccode);
							free(detailCode);
							st = executeCmd(cmd);
						}
					}
				}
				SQLOK(SQLiteFinalize(statement), database, "updatedb", 0);

			}

			if (st)
			{
				SLT_EndTrans(database);
				rtn = TRUE;
			}
			else
				SLT_AbortTrans(database);
		}
		break;

		case 5://convert version 5 to version 6
		{
			BOOL st = TRUE;
			rtn = FALSE;
			SLT_StartTrans(database);
			strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN RampStatus CHAR(256);");
			if (st) st = executeCmd(cmd);
			strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN RampScore INT;");
			if (st) st = executeCmd(cmd);
			strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN ProximityScore INT;");
			if (st) st = executeCmd(cmd);
			sprintf(cmd, "UPDATE Version SET VersionID = '%0.1f' WHERE vid = 1;", (double)(version + 1));
			if (st) st = executeCmd(cmd);
			strcpy(cmd, "UPDATE Ramps SET RampStatus = '', RampScore = 0, ProximityScore = 0");
			if (st) st = executeCmd(cmd);
			if (st)
			{
				SLT_EndTrans(database);
				rtn = TRUE;
			}
			else
				SLT_AbortTrans(database);
		}
		break;

		case 4://convert version 4 to version 5
			{
				BOOL st = TRUE;
				needToReload = TRUE;
				{
					SLT_StartTrans(database);
					strcpy(cmd, "UPDATE Ramps SET rampNum = 9 WHERE rampNum = 23");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET rampNum = 10 WHERE rampNum = 45");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET rampNum = 11 WHERE rampNum = 67");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET rampNum = 12 WHERE rampNum = 81");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET CornerID = 81 WHERE rampNum = 1");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET CornerID = 23 WHERE rampNum = 2");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET CornerID = 23 WHERE rampNum = 3");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET CornerID = 45 WHERE rampNum = 4");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET CornerID = 45 WHERE rampNum = 5");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET CornerID = 67 WHERE rampNum = 6");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET CornerID = 67 WHERE rampNum = 7");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET CornerID = 81 WHERE rampNum = 8");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET CornerID = 23 WHERE rampNum = 9  OR rampNum = 23");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET CornerID = 45 WHERE rampNum = 10 OR rampNum = 45");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET CornerID = 67 WHERE rampNum = 11 OR rampNum = 67");
					if (st) st = executeCmd(cmd);
					strcpy(cmd, "UPDATE Ramps SET CornerID = 81 WHERE rampNum = 12 OR rampNum = 81");
					if (st) st = executeCmd(cmd);
					if (st)
						SLT_EndTrans(database);
					else
						SLT_AbortTrans(database);
				}
				rtn = st;
			}
			break;
			case 3://convert version 3 to version 4
			{
				BOOL st = TRUE;
				rtn = FALSE;
				SLT_StartTrans(database);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN FromFileID CHAR(12);");
				if (st) st = sqlite3_exec(database, cmd, 0, 0, 0);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN CornerID INT;");
				st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN Retired INT;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET rampNum = 9 WHERE rampNum = 23");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET Retired = 0");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET rampNum = 10 WHERE rampNum = 45");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET rampNum = 11 WHERE rampNum = 67");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET rampNum = 12 WHERE rampNum = 81");
				if (st) st = executeCmd(cmd);
				
				strcpy(cmd, "UPDATE Ramps SET CornerID = 81 WHERE rampNum = 1");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET CornerID = 23 WHERE rampNum = 2");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET CornerID = 23 WHERE rampNum = 3");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET CornerID = 45 WHERE rampNum = 4");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET CornerID = 45 WHERE rampNum = 5");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET CornerID = 67 WHERE rampNum = 6");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET CornerID = 67 WHERE rampNum = 7");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET CornerID = 81 WHERE rampNum = 8");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET CornerID = 23 WHERE rampNum = 9  OR rampNum = 23");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET CornerID = 45 WHERE rampNum = 10 OR rampNum = 45");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET CornerID = 67 WHERE rampNum = 11 OR rampNum = 67");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "UPDATE Ramps SET CornerID = 81 WHERE rampNum = 12 OR rampNum = 81");
				if (st) st = executeCmd(cmd);
				sprintf(cmd, "UPDATE Version SET VersionID = '%0.1f' WHERE vid = 1;", (double)(version + 1));
				if (st) st = executeCmd(cmd);
				if (st)
				{
					SLT_EndTrans(database);
					rtn = TRUE;
				}
				else
					SLT_AbortTrans(database);
			}
			break;
			case 2://convert version 2 to version 3
			{
				BOOL st = TRUE;
				rtn = FALSE;
				SLT_StartTrans(database);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN detectableWidth INT;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN detectableDepth INT;");
				if (st) st = executeCmd(cmd);
				sprintf(cmd, "UPDATE Version SET VersionID = '%0.1f' WHERE vid = 1;", (double)(version + 1));
				if (st) st = executeCmd(cmd);
				if (st)
				{
					SLT_EndTrans(database);
					rtn = TRUE;
				}
				else
					SLT_AbortTrans(database);
			}
			break;
			case 1://convert version 1 to version 2
			{
				BOOL st = TRUE;
				rtn = FALSE;
				SLT_StartTrans(database);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN pedButtonAudibleWalkIndicationType INT;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN pedButtonHasLocatorTone INT;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN pedButtonHasInfoSign INT;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN pedButtonHasBraille INT;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN pedButtonHasTactileArrow INT;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN pedButtonLocatorToneVolume INT;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN pedButtonAudibleWalkIndicationVolume INT;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN rampCrackWidth REAL;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN upperLandingCrackWidth REAL;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN streetLandingCrackWidth REAL;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN leftSidewalkCrackWidth REAL;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN rightSidewalkCrackWidth REAL;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN curbCutDist REAL;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN bumpWidth REAL;");
				if (st) st = executeCmd(cmd);
				strcpy(cmd, "ALTER TABLE Ramps ADD COLUMN bumpHeight REAL;");
				if (st) st = executeCmd(cmd);
				sprintf(cmd, "UPDATE Version SET VersionID = '%0.1f' WHERE vid = 1;", (double)(version + 1));
				if (st) st = executeCmd(cmd);
				if (st)
				{
					SLT_EndTrans(database);
					rtn = TRUE;
				}
				else
					SLT_AbortTrans(database);
			}
			break;
		}
	}
	if (needToReload && rtn)
	{
		char toPath[MAX_PATH];
		BOOL st = TRUE;
		
		rtn = FALSE;
		sprintf(toPath, "%s.new", fromPath);
		st = sqlite3_close(database);
		st = NVCreateDB(toPath, TRUE);
		if (NVCopyDB(fromPath, toPath))
		{
			st = !sqlite3_open(toPath, &database);
			if (st)
				sqlite3_close(database);
			if (st)
			{
				GSSiRemove(fromPath);
				GSSiRename(toPath, fromPath);
				st = !sqlite3_open(fromPath, &database);
			}
			if (st)
				rtn = TRUE;
		}
	}
	return rtn;
}

BOOL NVCopyDB(LPSTR fromPath, LPSTR toPath)
{
	BOOL rc,rtn = FALSE;
	sqlite3 *database = NULL;
	char cmd[512];


	rc = sqlite3_open(fromPath, &database);
	if (!rc)
	{
		sprintf(cmd, "ATTACH DATABASE '%s' AS new_db;", toPath);
		rc = !SQLOK(sqlite3_exec(database, cmd, 0, 0, 0), database, "", 0);
		if (rc)
		{
			sprintf(cmd, "INSERT INTO new_db.CURBRAMP_NOTES SELECT * FROM CURBRAMP_NOTES;");
			rtn = !SQLOK(sqlite3_exec(database, cmd, 0, 0, 0), database, "", 0);
			sprintf(cmd, "INSERT INTO new_db.CURBRAMP_PICTURES SELECT * FROM CURBRAMP_PICTURES;");
			if (rtn)
				rtn = !SQLOK(sqlite3_exec(database, cmd, 0, 0, 0), database, "", 0);
			sprintf(cmd, "INSERT INTO new_db.CURBRAMP_STANDARD_TEXT SELECT * FROM CURBRAMP_STANDARD_TEXT;");
			if (rtn)
				rtn = !SQLOK(sqlite3_exec(database, cmd, 0, 0, 0), database, "", 0);
			sprintf(cmd, "INSERT INTO new_db.CURBRAMP_UPDATES SELECT * FROM CURBRAMP_UPDATES;");
			if (rtn)
				rtn = !SQLOK(sqlite3_exec(database, cmd, 0, 0, 0), database, "", 0);
			sprintf(cmd, "INSERT INTO new_db.Intersections SELECT * FROM Intersections;");
			if (rtn)
				rtn = !SQLOK(sqlite3_exec(database, cmd, 0, 0, 0), database, "", 0);
			sprintf(cmd, "INSERT INTO new_db.RAMPS SELECT * FROM RAMPS;");
			if (rtn)
				rtn = !SQLOK(sqlite3_exec(database, cmd, 0, 0, 0), database, "", 0);
			sprintf(cmd, "INSERT INTO new_db.PriorityLocations SELECT * FROM PriorityLocations;");
			if (rtn)
				rtn = !SQLOK(sqlite3_exec(database, cmd, 0, 0, 0), database, "", 0);
		}
		rc = sqlite3_close(database);
	}
	return rtn;
}

BOOL NVCreateDB(LPSTR path,BOOL Delete)
{
	BOOL rc;
	BOOL rtn = FALSE;
	
	int type = FileType(path);

	if (type && !Delete)
		return FALSE;
	else if (type)
		GSSiRemove(path);
	else
	{
		HFILE fid = GSSiOpenFile(path, 0, OF_CREATE);
		GSSiClose2 (&fid);
		GSSiRemove(path);
	}

	rc = sqlite3_open(path, &database);
	if (!rc)
		rtn = createIntersectionsTable(TRUE);
	rc = sqlite3_close(database);
	return rtn;
}

sqlite3 * getNVDBHandle(int databaseID,BOOL *opened)
{
	*opened = FALSE;

	if (database)
		return database;
	if (NVOpenDB(CRAPI->sharedInstance.currentCurbRampDB, FALSE, 0))
	{
		*opened = TRUE;
		return database;
	}
	return NULL;
}
int NVOpenDB(LPSTR path, BOOL CreateIfNotExists, LPSTR varnameforhandle)
{
	int rtn = 0;

	if (!ExistFile(path))
	{
		if (CreateIfNotExists)
		{
			rtn = NVCreateDB(path, FALSE);
		}
	}
	else
		rtn = TRUE;
	if (rtn)
	{
		rtn = sqlite3_open(path, &database);
		if (rtn == SQLITE_OK)
		{
			CRAPI_Init();
			strcpy(CRAPI->sharedInstance.currentCurbRampDB, path);
			if (varnameforhandle && *varnameforhandle)
				SetGlobalValueLong(varnameforhandle, (UINT)database);
			rtn = adjustToLatestVersion(path);
		}
	}
	return rtn;
}

int NVCloseDB(long handle)
{
	int rtn = 0;
	sqlite3 *db;
	if (handle)
	{
		db = (sqlite3*)handle;
		rtn = sqlite3_close(db);
		if (db == database)
			database = 0;
	}
	else
	{
		rtn = sqlite3_close(database);
		database = 0;
	}
	return rtn;
}

BOOL executeCmd(LPSTR cmd)
{
	return  Execute(cmd,0);
}
BOOL createIntersectionsTable(BOOL dropExistingTables)
{
	BOOL rtn = NO;

	char dropcmd[] = "DROP TABLE IF EXISTS Intersections;DROP TABLE IF EXISTS Intersections_index;DROP TABLE IF EXISTS Ramps;DROP TABLE IF EXISTS VERSION;DROP TABLE IF EXISTS CURBRAMP_UPDATES;DROP TABLE IF EXISTS CURBRAMP_PICTURES;DROP TABLE IF EXISTS CURBRAMP_NOTES;DROP TABLE IF EXISTS CURBRAMP_STANDARD_TEXT";

	if (dropExistingTables)
	{
		rtn = executeCmd(dropcmd);

	}
	char cmd[] = "CREATE TABLE IF NOT EXISTS CURBRAMP_UPDATES ('iPad' INTEGER PRIMARY KEY,'LastDataUpdate' INT,'LastPictUpdate' INT)";
	rtn = executeCmd(cmd);
	//create intersection table
	char createcmd[4096];
	
sprintf(createcmd, "CREATE TABLE IF NOT EXISTS VERSION (vid INTEGER PRIMARY KEY,VersionID CHAR(6));\
INSERT OR REPLACE INTO VERSION VALUES(1,'%s');\
CREATE TABLE IF NOT EXISTS Intersections (\
intID INTEGER PRIMARY KEY,\
lastUpdateTime INTEGER,\
status INTEGER,\
assignedToPrelim INTEGER,\
assignedToDetail INTEGER,\
intX INTEGER,\
intY INTEGER,\
rotation INTEGER,\
ramp1 INTEGER,\
ramp2 INTEGER,\
ramp3 INTEGER,\
ramp4 INTEGER,\
ramp5 INTEGER,\
ramp6 INTEGER,\
ramp7 INTEGER,\
ramp8 INTEGER,\
ramp81 INTEGER,\
ramp23 INTEGER,\
ramp45 INTEGER,\
ramp67 INTEGER,\
xWalkA INTEGER,\
xWalkB INTEGER,\
xWalkC INTEGER,\
xWalkD INTEGER,\
signal81 INTEGER,\
signal23 INTEGER,\
signal45 INTEGER,\
signal67 INTEGER,\
nStreets INTEGER,\
streetNames CHAR(512),\
streetAZMs CHAR(128),\
latitude REAL,\
longitude REAL,\
intersectionComment CHAR(256),\
doLater INTEGER,\
zoomLevel INTEGER);\
CREATE VIRTUAL TABLE IF NOT EXISTS Intersections_index USING rtree(id,minX, maxX, minY, maxY);", CURRENT_INTERSECTION_VERSION);

rtn = executeCmd(createcmd);
//create ramp table
char	createcmd2[] = "CREATE VIRTUAL TABLE IF NOT EXISTS Ramps_index USING rtree(id,minX, maxX, minY, maxY);\
CREATE TABLE IF NOT EXISTS Ramps (\
intID INT,\
rampNum INT,\
rampID CHAR(16),\
yearRebuilt INT,\
timeComplete INT,\
rampExists INT,\
isComplete INT,\
approximateHeading REAL,\
adjustedRot INT,\
rampInXWalk INT,\
xWalkisComplete INT,\
signalisComplete INT,\
texture INT,\
upperLandingObstruction INT,\
lowerLandingObstruction INT,\
rampObstruction INT,\
hasRampCracks INT,\
hasUpperLandingCracks INT,\
hasStreetLandingCracks INT,\
rampWidth INT,\
rampDepth INT,\
rampSlopeFront REAL,\
rampSlopeSide REAL,\
rampSlopeHeading REAL,\
upperLandingSlopeFront REAL,\
upperLandingSlopeSide REAL,\
upperLandingSlopeHeading REAL,\
streetLandingSlopeFront REAL,\
streetLandingSlopeSide REAL,\
streetLandingSlopeHeading REAL,\
flareLeftSlopeFront REAL,\
flareLeftSlopeSide REAL,\
flareLeftSlopeHeading REAL,\
flareRightSlopeFront REAL,\
flareRightSlopeSide REAL,\
flareRightSlopeHeading REAL,\
swkLeftSlopeFront REAL,\
swkLeftSlopeSide REAL,\
swkLeftSlopeHeading REAL,\
swkRightSlopeFront REAL,\
swkRightSlopeSide REAL,\
swkRightSlopeHeading REAL,\
PEDSignalType INT,\
PEDButtonType INT,\
PEDButtonHeight INT,\
PEDButtonDist INT,\
SteepTopOfCurb REAL,\
PedRampLip REAL,\
lev21x INT, lev21y INT,\
rampType INT,\
rampComment CHAR(256),\
pedButtonAudibleWalkIndicationType INT,\
pedButtonHasLocatorTone INT,\
pedButtonHasInfoSign INT,\
pedButtonHasBraille INT,\
pedButtonHasTactileArrow INT,\
pedButtonLocatorToneVolume INT,\
pedButtonAudibleWalkIndicationVolume INT,\
rampCrackWidth REAL,\
upperLandingCrackWidth REAL,\
streetLandingCrackWidth REAL,\
leftSidewalkCrackWidth REAL,\
rightSidewalkCrackWidth REAL,\
curbCutDist REAL,\
bumpWidth REAL,\
bumpHeight REAL,\
detectableWidth INT,\
detectableDepth INT,\
FromFileID CHAR(12),\
CornerID INT,\
Retired INT,\
RampStatus CHAR(256),\
RampCode INT,\
ProximityScore INT, \
RampNotes CHAR(1024), \
CCSummary CHAR(32), \
CCDetail CHAR(64), \
LastUpdate CHAR(32), \
ProximityValue INT, \
PRIMARY KEY (intID,rampNum,Retired ASC));";
rtn = executeCmd(createcmd2);

char createcmd3[] = "CREATE TABLE IF NOT EXISTS CURBRAMP_PICTURES ('id' INTEGER PRIMARY KEY,'iPadNum' INT,'pictNum' INT,'intID' INT,'rampNum' INT, 'type' INT, 'heading' INT, 'latitude' REAL, 'longitude' REAL, 'time' INT)";
rtn = executeCmd(createcmd3);
char createcmd4[] = "CREATE TABLE IF NOT EXISTS CURBRAMP_NOTES ('id' INTEGER PRIMARY KEY,'intID' INT,'corner' INT,'rampID' INT,'type' INT, 'note' CHAR(4096))";
rtn = executeCmd(createcmd4);
char createcmd5[] = "CREATE TABLE IF NOT EXISTS CURBRAMP_STANDARD_TEXT ('textID' INTEGER PRIMARY KEY,'type' INT,'text' CHAR(4096))";
rtn = executeCmd(createcmd5);
char createcmd6[] = "CREATE TABLE IF NOT EXISTS PriorityLocations ('locationID' INTEGER PRIMARY KEY,'Name' CHAR(256),'Type' INT, 'Category' CHAR(32),'Latitude' DOUBLE, 'Longitude' DOUBLE, 'Radius' DOUBLE, 'Offset' INT);";
rtn = executeCmd(createcmd6);
char createcmd7[] = "CREATE INDEX rampsIntersectionIndex ON ramps (intID);CREATE INDEX picturesIntersectionIndex ON CURBRAMP_PICTURES (intID);";
rtn = executeCmd(createcmd7);


	return rtn;
}
/*
-(BOOL)adjustToLatestVersion
{
	BOOL rtn = TRUE;

	int currentVersion = [self datasetVersion];
	int latestVersion = atoi(CURRENT_INTERSECTION_VERSION);

	for (int version = currentVersion; version < latestVersion; version++)
	{
		switch (version)
		{
		case 2://convert version 2 to version 3
		{
			BOOL st = TRUE;
			rtn = FALSE;
			[self startTransaction];
			NSString * cmd = @"";
			cmd = @"ALTER TABLE Ramps ADD COLUMN detectableWidth INT;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN detectableDepth INT;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = [NSString stringWithFormat : @"UPDATE Version SET VersionID = '%s' WHERE vid = 1;", CURRENT_INTERSECTION_VERSION];
			st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			if (st)
			{
				[self commitTransaction];
				rtn = TRUE;
			}
			else
				[self cancelTransaction];
		}
			break;
		case 1://convert version 1 to version 2
		{
			BOOL st = TRUE;
			rtn = FALSE;
			[self startTransaction];
			NSString * cmd = @"ALTER TABLE Ramps ADD COLUMN pedButtonAudibleWalkIndicationType INT;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN pedButtonHasLocatorTone INT;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN pedButtonHasInfoSign INT;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN pedButtonHasBraille INT;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN pedButtonHasTactileArrow INT;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN pedButtonLocatorToneVolume INT;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN pedButtonAudibleWalkIndicationVolume INT;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN rampCrackWidth REAL;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN upperLandingCrackWidth REAL;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN streetLandingCrackWidth REAL;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN leftSidewalkCrackWidth REAL;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN rightSidewalkCrackWidth REAL;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN curbCutDist REAL;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN bumpWidth REAL;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = @"ALTER TABLE Ramps ADD COLUMN bumpHeight REAL;";
			if (st) st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			cmd = [NSString stringWithFormat : @"UPDATE Version SET VersionID = '%s' WHERE vid = 1;", CURRENT_INTERSECTION_VERSION];
			st = [self executeCmd : cmd.UTF8String sendToServer : NO from : 0];
			if (st)
			{
				[self commitTransaction];
				rtn = TRUE;
			}
			else
				[self cancelTransaction];
		}
			break;
		}
	}
	return rtn;
}
*/
int getRampOffsetCoord(POINT ramp21,POINT int21)
{
	int offCoord = 0;

	short offx = (ramp21.x - int21.x);
	short offy = (ramp21.y - int21.y);

	offCoord = (int)MAKELONG(offx, offy);
	return offCoord;
}
/*
-(int)getRampCoord:(MPIntersection *)mpint
ramp : (int)r
{
	int icoord = 0;
	MPRamp *ramp = mpint.ramps[r];

	if (ramp.rampExists)
	{
		icoord = [self getOffsetCoord : mpint
		x : ramp.lev21x
		y : ramp.lev21y];
	}

	return icoord;

}

-(void)setRampCoord:(MPIntersection *)mpint
ramp : (int)r
   from : (sqlite3_stmt *)statement
{
	int icol = (8 + (r - 1));
	int icoord = sqlite3_column_int(statement, icol);

	short offsetx = LOWORD(icoord);
	short offsety = HIWORD(icoord);
	MPRamp *ramp = mpint.ramps[r];
	int lev21x = (mpint.lev21x + offsetx);
	int lev21y = (mpint.lev21y + offsety);

	if (icoord)
	{
		ramp.lev21x = lev21x;
		ramp.lev21y = lev21y;
		ramp.rampExists = YES;
		ramp.isComplete = NO;
	}

	else
	{
		ramp.rampExists = NO;
		ramp.isComplete = YES;
	}
}

-(int)getXWalkCoord:(MPIntersection *)mpint
xWalk : (char)xw
{
	int icoord = 0;

	switch (xw)
	{
	case 'A':
		if (mpint.xWalkAExists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.xWalkAlev21x
			y : mpint.xWalkAlev21y];
		}

		break;

	case 'B':
		if (mpint.xWalkBExists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.xWalkBlev21x
			y : mpint.xWalkBlev21y];
		}

		break;

	case 'C':
		if (mpint.xWalkCExists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.xWalkClev21x
			y : mpint.xWalkClev21y];
		}

		break;

	case 'D':
		if (mpint.xWalkDExists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.xWalkDlev21x
			y : mpint.xWalkDlev21y];
		}

		break;
	}

	return icoord;
}

-(void)setXWalkCoord:(MPIntersection *)mpint
xWalk : (char)xw
	from : (sqlite3_stmt *)statement
{
	int icol = (20 + (xw - 'A'));
	int icoord = sqlite3_column_int(statement, icol);

	short offsetx = LOWORD(icoord);
	short offsety = HIWORD(icoord);

	switch (xw)
	{
	case 'A':
		if (icoord)
		{
			mpint.xWalkAlev21x = mpint.lev21x + offsetx;
			mpint.xWalkAlev21y = mpint.lev21y + offsety;
			mpint.xWalkAExists = YES;
		}

		else
			mpint.xWalkAExists = NO;

		break;

	case 'B':
		if (icoord)
		{
			mpint.xWalkBlev21x = mpint.lev21x + offsetx;
			mpint.xWalkBlev21y = mpint.lev21y + offsety;
			mpint.xWalkBExists = YES;
		}

		else
			mpint.xWalkBExists = NO;

		break;

	case 'C':
		if (icoord)
		{
			mpint.xWalkClev21x = mpint.lev21x + offsetx;
			mpint.xWalkClev21y = mpint.lev21y + offsety;
			mpint.xWalkCExists = YES;
		}

		else
			mpint.xWalkCExists = NO;

		break;

	case 'D':
		if (icoord)
		{
			mpint.xWalkDlev21x = mpint.lev21x + offsetx;
			mpint.xWalkDlev21y = mpint.lev21y + offsety;
			mpint.xWalkDExists = YES;
		}

		else

			mpint.xWalkDExists = NO;
		break;
	}
}

-(int)getSignalCoord:(MPIntersection*)mpint
signal : (int)sig
{
	int icoord = 0;

	switch (sig)
	{
	case 81:
		if (mpint.signal81Exists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.signal81lev21x
			y : mpint.signal81lev21y];
		}

		break;

	case 23:
		if (mpint.signal23Exists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.signal23lev21x
			y : mpint.signal23lev21y];
		}

		break;

	case 45:
		if (mpint.signal45Exists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.signal45lev21x
			y : mpint.signal45lev21y];
		}

		break;

	case 67:
		if (mpint.signal67Exists)
		{
			icoord = [self getOffsetCoord : mpint
			x : mpint.signal67lev21x
			y : mpint.signal67lev21y];
		}

		break;
	}

	return icoord;
}

-(void)setSignalCoord:(MPIntersection *)mpint
signal : (int)s
	 from : (sqlite3_stmt *)statement
{
	int icol = 24;

	switch (s)
	{
	case 81:
		break;

	case 23:
		icol++;
		break;

	case 45:
		icol += 2;
		break;

	case 67:
		icol += 3;
		break;
	}

	int icoord = sqlite3_column_int(statement, icol);

	short offsetx = LOWORD(icoord);
	short offsety = HIWORD(icoord);

	switch (s)
	{
	case 81:
		if (icoord)
		{
			mpint.signal81lev21x = mpint.lev21x + offsetx;
			mpint.signal81lev21y = mpint.lev21y + offsety;
			mpint.signal81Exists = YES;
		}

		else
			mpint.signal81Exists = NO;

		break;

	case 23:
		if (icoord)
		{
			mpint.signal23lev21x = mpint.lev21x + offsetx;
			mpint.signal23lev21y = mpint.lev21y + offsety;
			mpint.signal23Exists = YES;
		}

		else
			mpint.signal23Exists = NO;

		break;

	case 45:
		if (icoord)
		{
			mpint.signal45lev21x = mpint.lev21x + offsetx;
			mpint.signal45lev21y = mpint.lev21y + offsety;
			mpint.signal45Exists = YES;
		}

		else
			mpint.signal45Exists = NO;

		break;

	case 67:
		if (icoord)
		{
			mpint.signal67lev21x = mpint.lev21x + offsetx;
			mpint.signal67lev21y = mpint.lev21y + offsety;
			mpint.signal67Exists = YES;
		}

		else
			mpint.signal67Exists = NO;

		break;
	}
}

*/

LPSTRD getSendToServerFile(int databaseID, int iPad)
{
	int pictNum;
	int currentFileNum = getLastDataUpdateNumber (databaseID,iPad, &pictNum);

	LPSTRD file = malloc(MAX_PATH);
	sprintf (file,"%s/%i_%i.sql",CRAPI_sharedInstance_sharedOutputDirectory("CurbRamps"), iPad, currentFileNum);
	return file;
}

BOOL executeAndSendCmd (int databaseID, LPSTR cmd,BOOL sendToServer)
{

	BOOL rtn = Execute(cmd, CRAPI->sharedInstance.errFile);
	if (rtn && sendToServer)
	{
		int iPad = CRAPI->sharedInstance.currentiPadWithinManager;
		LPSTRD sendToServerFile = getSendToServerFile (databaseID, iPad);


		if (!FileType (sendToServerFile))
		{
			char versions[256];
			__time32_t iTime;
			_time32(&iTime);

			sprintf (versions,"/*ApVer %s DBVer %s Time %i*/", appAndVersion(), CURRENT_INTERSECTION_VERSION, iTime);

			rtn = appendStringToFile(versions, sendToServerFile);
		}

		rtn = appendStringToFile(cmd, sendToServerFile);
		free(sendToServerFile);
	}
	else if (sendToServer)
	{
		LPSTRD logmsg = malloc(strlen(cmd) + 16);
		sprintf(logmsg, "COMMAND:%s", cmd);
		logToErrorFile (logmsg);
		free(logmsg);
	}

	return rtn;
}
int SQLitePrepare(sqlite3 *db,            /* Database handle */
	const char *zSql,       /* SQL statement, UTF-8 encoded */
	int nByte,              /* Maximum length of zSql in bytes. */
	sqlite3_stmt **ppStmt,  /* OUT: Statement handle */
	const char **pzTail     /* OUT: Pointer to unused portion of zSql */)

{
	int rtn = sqlite3_prepare_v2(db, zSql, nByte, ppStmt, pzTail);
	int qlen = (int)strlen(zSql);
	//if (pdb->lastQuery)
	//	free(pdb->lastQuery);
	//pdb->lastQuery = malloc(qlen + 4);
	//strcpy(pdb->lastQuery, zSql);
	return rtn;
}
int SQLiteFinalize(sqlite3_stmt *pStmt)
{
	int rtn = sqlite3_finalize(pStmt);
	//    if (lastQuery)
	//        free (lastQuery);
	//    lastQuery = 0;
	return rtn;
}
int SQLiteExec(
	sqlite3 *db,                                  /* An open database */
	const char *sql,                           /* SQL to be evaluated */
	int(*callback)(void*, int, char**, char**),  /* Callback function */
	void * arg,                                    /* 1st argument to callback */
	char **errmsg                              /* Error msg written here */)
{
	int qlen = (int)strlen(sql);
//	if (pdb->lastQuery)
//		free(pdb->lastQuery);
//	pdb->lastQuery = malloc(qlen + 4);
//	strcpy(pdb->lastQuery, sql);
	return sqlite3_exec(db, sql, callback, arg, errmsg);
}

int getLastDataUpdateNumber(int databaseID,int iPad,int *pictNum)
{
	int lastNum = 0;
	int lastPictNum = 0;
	BOOL opened = openDatabaseID(databaseID);
	char cmd[256];
	sprintf (cmd,"SELECT LastDataUpdate, LastPictUpdate FROM CURBRAMP_UPDATES WHERE iPad = %i",iPad);
	sqlite3_stmt *statement;

	SQLOK(SQLitePrepare(database, cmd, -1, &statement, 0), database,"getLastDataUpdateNumber", 0);
	if (sqlite3_step(statement) == SQLITE_ROW)
	{
		lastNum = sqlite3_column_int(statement, 0);
		lastPictNum = sqlite3_column_int(statement, 1);
	}
	SQLOK(SQLiteFinalize(statement), database, "getLastDataUpdateNumber", 0);
	if (!lastNum)
	{
		if (iPad == CRAPI->sharedInstance.currentiPadWithinManager)
			lastNum = 1;
		sprintf(cmd, "INSERT OR REPLACE INTO CURBRAMP_UPDATES VALUES(%i, %i, 0); ",iPad,lastNum);
		executeAndSendCmd(databaseID, cmd, NO);
	}
	closeDatabaseID(databaseID,opened);
	*pictNum = lastPictNum;
	return lastNum;
}

BOOL openDatabaseID(int databaseID)
{
	if (database)
		return FALSE;
	return NVOpenDB(CRAPI->sharedInstance.currentCurbRampDB, FALSE, 0);
}

void closeDatabaseID(int databaseID, BOOL opened)
{
	if (opened && database)
		NVCloseDB(0);
}
BOOL setLastDataAndPictUpdateNums(int databaseID, int lastNum,int pictNum,int iPad)
{
	BOOL opened = openDatabaseID(databaseID);
	char cmd[256];
	sprintf (cmd,"INSERT OR REPLACE INTO CURBRAMP_UPDATES VALUES(%i, %i, %i); ",iPad,lastNum,pictNum);
	BOOL rtn = executeAndSendCmd(databaseID, cmd, NO);
	closeDatabaseID(databaseID, opened);
	return rtn;
}

BOOL loadUpdate(int databaseID, LPSTR file)
{
		BOOL rtn = TRUE;
		char fileID[32] = "";
		BOOL opened = openDatabaseID(databaseID);
		int line = 0;
		HFILE fid = GSSiOpenFile(file, 0, OF_READ);
		int totLen = GSSifilelength(fid);
		int maxLineLen = totLen + 2;
		LPSTR str = malloc(totLen + 4096);
		LPSTR pBS = strrchr(file, '\\');
		if (pBS)
		{
			LPSTR pDot;
			strcpy(fileID, ++pBS);
			pDot = strrchr(fileID, '.');
			if (pDot)
				*pDot = 0;
		}

		SLT_StartTrans(database);
		if (fgetstring(str, maxLineLen, fid))
		{
			int fromVer = atoi(CURRENT_INTERSECTION_VERSION);
			int toVer = atoi(CURRENT_INTERSECTION_VERSION);
			LPSTR vloc = strstr(str, "DBVer ");
			if (vloc)
				fromVer = atoi(vloc + 6);
			while (rtn && fgetstring(str, maxLineLen, fid))
			{
				convertVersion(str, fromVer, toVer,fileID);
				rtn = executeAndSendCmd(databaseID, str, NO);
				line++;
			}
		}
		GSSiClose2 (&fid);
		if (!rtn) //file has to be edited on server (by GSSi) before more data can be loaded
		{
			SLT_AbortTrans(database);
			LPSTRD header = malloc(1024);
			LPSTRD name = lastPathComponent(file);
			sprintf (header,"Data load failure in file %s at line %i",name,line);
			logToErrorFileWithHeader(str, header);
			free(header);
			free(name);
		}
		else
			SLT_EndTrans(database);
		free(str);
		closeDatabaseID(databaseID, opened);
		return rtn;
}

BOOL setLastDataUpdateNum (int databaseID, int lastNum,int iPad)
{
	BOOL rtn = FALSE;
	BOOL opened = openDatabaseID(databaseID);
	char cmd[256];
	sprintf (cmd,"SELECT * FROM CURBRAMP_UPDATES WHERE iPad = %i",iPad);
	sqlite3_stmt *statement;

	SQLOK(SQLitePrepare(database, cmd, -1, &statement, 0),database,"setLastDataUpdateNum",0);
	BOOL haveRecord = (sqlite3_step(statement) == SQLITE_ROW);
	SQLOK(SQLiteFinalize(statement), database, "setLastDataUpdateNum", 0);

	if (!haveRecord)
	{
		sprintf (cmd, "INSERT OR REPLACE INTO CURBRAMP_UPDATES VALUES(%i, 1, 0);",iPad);
		rtn = executeAndSendCmd(databaseID, cmd, NO);
	}
	sprintf (cmd,"UPDATE CURBRAMP_UPDATES SET LastDataUpdate = %i WHERE iPad = %i",lastNum,iPad);
	rtn = executeAndSendCmd(databaseID, cmd, NO);
	closeDatabaseID(databaseID, opened);
	return rtn;
}

BOOL setLastPictUpdateNum(int databaseID, int lastNum,int iPad)
{
	BOOL rtn = FALSE;
	BOOL opened = openDatabaseID(databaseID);
	char cmd[256];
	sprintf (cmd,"SELECT * FROM CURBRAMP_UPDATES WHERE iPad = %i",iPad);
	sqlite3_stmt *statement;

	SQLOK(SQLitePrepare(database, cmd, -1, &statement, 0), database,"setLastPictUpdateNum",0);
	BOOL haveRecord = (sqlite3_step(statement) == SQLITE_ROW);
	SQLOK(SQLiteFinalize(statement),database,"setLastPictUpdateNum",0);

	if (!haveRecord)
	{
		sprintf (cmd ,"INSERT OR REPLACE INTO CURBRAMP_UPDATES VALUES(%i, 1, 0); ",iPad);
		rtn = executeAndSendCmd(databaseID, cmd, NO);
	}
	sprintf (cmd ,"UPDATE CURBRAMP_UPDATES SET LastPictUpdate = %i WHERE iPad = %i",lastNum,iPad);
	rtn = executeAndSendCmd(databaseID, cmd, NO);
	closeDatabaseID(databaseID, opened);
	return rtn;
}

int getLastPictUpdateNumber (int databaseID, int iPad)
{
	int LastNum = 0;
	BOOL opened = openDatabaseID(databaseID);
	char query[256];
	sprintf (query,"SELECT LastPictUpdate FROM CURBRAMP_UPDATES WHERE iPad = %i",iPad);
	sqlite3_stmt *statement;

	SQLOK( SQLitePrepare(database, query, -1, &statement, 0), database,"getLastPictUpdateNumber",0);
	if (sqlite3_step(statement) == SQLITE_ROW)
	{
		LastNum = sqlite3_column_int(statement, 0);
	}
	SQLOK(SQLiteFinalize(statement), database, "getLastPictUpdateNumber", 0);
	closeDatabaseID(databaseID, opened);
	return LastNum;
}

int incrementLastDataFileNum (int databaseID)
{
	int pictNum;
	int iPad = CRAPI->sharedInstance.currentiPadWithinManager;
	int lastNum = getLastDataUpdateNumber(databaseID,iPad,&pictNum);
	int nextNum = max(1, lastNum);
	LPSTRD file = outputDataFile (databaseID,nextNum,iPad);

	int fileSize = GSSiLength(file);
	while (fileSize > 0)
	{
		nextNum++;
		free(file);
		file = outputDataFile(databaseID,nextNum, iPad);
		fileSize = GSSiLength(file);
	}
	if (nextNum > lastNum)
		setLastDataUpdateNum(databaseID,nextNum, iPad);
	if (nextNum - lastNum > 1)
	{
		char logmess[32];
		strcpy(logmess, "Curbramp data file num error");
		logToErrorFile(logmess);
	}
	return nextNum;
}
/*{
	int pictNum;
	int iPad = CRAPI.sharedInstance.currentiPadWithinManager;
	int lastNum = [self getLastDataUpdateNumber : iPad
		lastPictNum : &pictNum];
	int nextNum = max(1, lastNum);
	NSString* file = [self outputDataFile : nextNum iPad : iPad];

	int fileSize = (int)[NSFileManager.defaultManager attributesOfItemAtPath : file
		error : nil].fileSize;
	while (fileSize > 0)
	{
		nextNum++;
		file = [self outputDataFile : nextNum iPad : iPad];
		fileSize = (int)[NSFileManager.defaultManager attributesOfItemAtPath : file
			error : nil].fileSize;
	}
	if (nextNum > lastNum)
		[self setLastDataUpdateNum : nextNum pictNum : pictNum foriPad : iPad];
	if (nextNum - lastNum > 1)
	{
		NSString *logmess = @"Curbramp data file num error";
			[CRAPI.sharedInstance logToErrorFile : logmess];
	}
	return nextNum;
}*/


int incrementLastDataFileNumber(int databaseID)
{
	return incrementLastDataFileNum(databaseID);
}
int incrementLastPictFileNumber(int databaseID)
{
	return incrementLastPictFileNum(databaseID);
}
int incrementLastPictFileNum (int databaseID)
{
	int iPad = CRAPI->sharedInstance.currentiPadWithinManager;
	int lastNum = getLastPictUpdateNumber(databaseID,iPad);
	LPSTR file = outputDataFile(databaseID,lastNum, iPad);

	int fileSize = GSSiLength(file);
	if (fileSize > 0)
	{
		setLastPictUpdateNum(databaseID,++lastNum,iPad);
	}
	return lastNum;
}
LPSTRD outputDataFile(int databaseID,int fileNum,int iPad)
{
	LPSTRD file = malloc(MAX_PATH);
	sprintf (file,"%s\\%i_%i.sql",CRAPI_sharedInstance_sharedOutputDirectory("CurbRamps"), iPad, fileNum);
	return file;
}

LPSTRD outputPictFile(int databaseID,int fileNum,int iPad)
{
	LPSTRD file = malloc(MAX_PATH);
	sprintf (file,"%s\\i_%i.jpg", CRAPI_sharedInstance_sharedOutputDirectory("CurbRamps"), iPad, fileNum);
	return file;
}

BOOL appendStringToFile(LPSTR str, LPSTR filePath)
{
	return AppendFile(filePath, str);
}
LPSTRD stringByDeletingLastPathComponent(LPSTR path)
{
	LPSTRD rtn = malloc(MAX_PATH);
	LPSTR last = strrchr(path, '\\');
	if (!last)
		last = strrchr(path, '/');
	if (last)
	{
		char save = *last;
		*last = 0;
		strcpy(rtn, path);
		*last = save;
	}
	else
		strcpy(rtn, path);
	return rtn;
}
LPSTRD lastPathComponent(LPSTR path)
{
	LPSTRD rtn = malloc(MAX_PATH);
	LPSTR last = strrchr(path, '\\');
	if (!last)
		last = strrchr(path, '/');
	if (last)
	{
		last++;
		strcpy(rtn, last);
	}
	else
		strcpy(rtn, path);

	return rtn;

}

LPSTRD errorLogPath (void)
{
	LPSTRD path = malloc(MAX_PATH);
	sprintf (path,"%s\\%s", CRAPI->sharedInstance.sharedFilePath, "NVErrorLog.txt");
	return path;
}
void logToErrorFileIgnore(BOOL ignore)
{
	_ignoreErrorLog = ignore;
}
LPSTRD CRAPI_sharedInstance_errorLogPath(void)
{
	return errorLogPath();
}
BOOL CRAPI_sharedInstance_haveErrorLog(void)
{
	BOOL rtn = FALSE;
	LPSTRD logPath = errorLogPath();
	int fileSize = GSSiLength(logPath);
	if (fileSize > 0)
		rtn = TRUE;
	free(logPath);
	return rtn;
}

void logToErrorFilewithHeader ( LPSTR error,LPSTR header)
{
	LPSTR mess = malloc(strlen(error) + strlen(header) + 32);
	sprintf (mess,"%s:%s",header, error);
	logToErrorFile(mess);
	free(mess);
}

void NSLog(LPSTR fmt, LPSTR str)
{
	LPSTR mess = malloc(strlen(str) * 2 + 32);

	sprintf(mess, fmt, str);
	free(mess);
}
LPSTR timeStamp(void)
{
	static char ts[64];
	LPSTR rtn = ts;

	strcpy(ts, "$CAL([%SYS_CLOCK],3)");
	ExpandText(ts);
	return rtn;
}
void logToErrorFile (LPSTR error)
{
	if (_ignoreErrorLog)
		return;
	LPSTRD errorFile = errorLogPath();
	LPSTRD idString = malloc(1024);
	sprintf (idString,"%s %i %i %s %i %i %i %i",
		timeStamp(),
		CRAPI->sharedInstance.currentModule,
		CRAPI->sharedInstance.currentSubModule,
		CRAPI->sharedInstance.appVersionBuild,
		CRAPI->sharedInstance.GSSiPadNumber,
		CRAPI->sharedInstance.currentManageriPad,
		CRAPI->sharedInstance.currentiPadWithinManager,
		CRAPI->sharedInstance.currentGeoid);
	NSLog("%s", error);
	appendStringToFile (idString, errorFile);
	appendStringToFile (error, errorFile);
	free(errorFile);
	free(idString);
}
void logToErrorFileWithHeader (LPSTR error,LPSTR header)
{
	LPSTRD mess = malloc(strlen(error) + strlen(header) + 32);
	sprintf (mess,"%s:%s",header, error);
	logToErrorFile(mess);
	free(mess);
}

void clearErrorFile (void)
{
	LPSTRD errorFile = errorLogPath();
	GSSiRemove(errorFile);
	free(errorFile);
}

BOOL PushFileToServer(LPSTR fromFileName, LPSTR toFileName, LPSTR fromDir, LPSTR toDir, BOOL deleteWhenDone, BOOL appendTempExtension, HWND *popoverView, LPSTR title,double showAfter,LPSTR errorVar)
{
	BOOL rtn = FALSE;
	HANDLE hFTPStruct = OpenServerFTP(toDir, 3, errorVar);
	char localFile[MAX_PATH];

	if (hFTPStruct)
	{
		LPFTPSTRUCT pFTPStruct = GlobalLock(hFTPStruct);
		sprintf(localFile, "%s\\%s", fromDir, fromFileName);
		LPSTRD toFile = malloc(MAX_PATH);

		if (appendTempExtension)
			sprintf(toFile, "%s.upload", toFileName);
		else
			strcpy(toFile, toFileName);
		rtn = FTPPutFile(pFTPStruct->hFTP,toFile,localFile, TRUE, TRUE, errorVar);
		GlobalUnlock(hFTPStruct);
		free(toFile);

		CloseServerFTP(hFTPStruct);
	}
	return rtn;
}

BOOL GetFileFromServer(LPSTR fromFile, LPSTR toFileName, LPSTR fromDir, LPSTR toDir, HWND *popoverView, LPSTR title, double showAfter, LPSTR errorVar)
{
	BOOL rtn = FALSE;
	HANDLE hFTPStruct = OpenServerFTP(fromDir, 3, errorVar);
	char localFile[MAX_PATH];

	if (hFTPStruct)
	{
		LPFTPSTRUCT pFTPStruct = GlobalLock(hFTPStruct);
		sprintf(localFile, "%s\\%s", toDir, toFileName);
		LPSTRD toFile = malloc(MAX_PATH);

		rtn = FTPGetFile(pFTPStruct->hFTP, fromFile, localFile, TRUE, TRUE, errorVar);
		GlobalUnlock(hFTPStruct);
		free(toFile);

		CloseServerFTP(hFTPStruct);
	}
	return rtn;
}
HANDLE OpenServerFTP (LPSTR serverDir, int serverNumber, LPSTR errorVar)
{
	HANDLE hFTPStruct = GSSiGlobAlloc(1781, GHND, sizeof(FTPSTRUCT));
	LPFTPSTRUCT pFTPStruct = GlobalLock(hFTPStruct);
	char server[32];
	char loginID[] = "GSSiProfessionalServer";
	char pswd[] = "GSSi";
	sprintf(server, "www.gssiserver%i.com", serverNumber);
	pFTPStruct->hFTP = FTPOpen(server, loginID, pswd, serverDir, errorVar, 0, TRUE);
	if (!pFTPStruct->hFTP)
	{
		GSSiGlobUlFree(&hFTPStruct);
	}
	else
	{
		pFTPStruct->reopenAttempts = 1;
		strcpy(pFTPStruct->ServerName,server);
		strcpy(pFTPStruct->Username, loginID);
		strcpy(pFTPStruct->Password, pswd);
		strcpy(pFTPStruct->directory,serverDir);
		GlobalUnlock(hFTPStruct);
	}
	return hFTPStruct;
}

BOOL CloseServerFTP (HANDLE hFTPStruct)
{
	BOOL rtn = FALSE;
	if (hFTPStruct)
	{
		SIZE_T l = GlobalSize(hFTPStruct);
		if (l > 0)
		{
			LPFTPSTRUCT pFTPStruct = GlobalLock(hFTPStruct);
			rtn = FTPClose(pFTPStruct->hFTP);
			GSSiGlobUlFree(&hFTPStruct);
		}
	}
	return rtn;
}

BOOL CreateSharedZoomList(LPSTR name)
{
	BOOL rtn = FALSE;
	sqlite3 *db;
	BOOL opened = FALSE;
	if (GetGlobalBVal2("[%ALLOWSHAREDLISTS]", FALSE))
	{
		CRAPI_Init();
		db = getNVDBHandle(CRAPI->sharedInstance.currentDatabaseID, &opened);
		if (db)
		{
			char cmd[512];

			sprintf(cmd, "CREATE TABLE ZOOMLIST_%s (Name CHAR(256) PRIMARY KEY,xmin DOUBLE,ymin DOUBLE,xmax DOUBLE,ymax DOUBLE, STATUS CHAR(256))", name);
			if (executeAndSendCmd(CRAPI->sharedInstance.currentDatabaseID, cmd, TRUE))
				rtn = TRUE;
			if (opened)
				NVCloseDB((long)db);
		}
	}
	return rtn;
}

BOOL SaveZoomToCurrentSharedList(LPMNMXCORD pBounds, LPSTR Name,LPSTR Status)
{
	BOOL rtn = FALSE;
	BOOL opened = FALSE;
	if (GetGlobalBVal2("[%ALLOWSHAREDLISTS]", FALSE))
	{
		sqlite3 *db;
		CRAPI_Init();
		db = getNVDBHandle(CRAPI->sharedInstance.currentDatabaseID, &opened);
		if (db)
		{
			char cmd[1024];

			sprintf(cmd, "INSERT INTO ZOOMLIST_%s VALUES('%s',%f,%f,%f,%f,'%s')", CurrentZoomList,Name,pBounds->xmn,pBounds->ymn,pBounds->xmx,pBounds->ymx,Status);
			if (executeAndSendCmd(CRAPI->sharedInstance.currentDatabaseID, cmd, TRUE))
				rtn = TRUE;
			if (opened)
				NVCloseDB((long)db);
		}
	}

	return rtn;
}
BOOL CreateNewZoomList(HWND hWndDlg, LPSTR Name)
{
	BOOL rtn = FALSE;
	char listFile[MAX_PATH];
	char name[256] = { 0 };
	char str[512];
	char mess[512];
	HFILE Fid;
	BOOL listIsShared = FALSE;

	if (!hWndDlg)
		hWndDlg = GetFocus();
	if (!Name)
	{
		if (!GetTextString(hWndDlg, name, 250, "Enter List Name:", "", 0, 0, 1, 0))
		{
			return FALSE;
		}
		if (GetGlobalBVal2("[%ALLOWSHAREDLISTS]", FALSE))
		{
			short opt = MessageBox(hWndDlg, "Is this a shared list?", "", MB_YESNOCANCEL);
			switch (opt)
			{
			case IDCANCEL:
				return FALSE;
			case IDYES:
				listIsShared = TRUE;
			}
		}
	}
	else
		strncpy0(name, Name, 250);
	sprintf(mess, "%s is an existing list.\nDo you wish to delete its contents?", name);
	sprintf(listFile, "%s\\%s.txt", ZOOMLISTDIR, name);
	if (FileType(listFile))
	{
		short opt = MessageBox(hWndDlg, mess, "", MB_YESNOCANCEL);
		switch (opt)
		{
		case IDCANCEL:
		case IDNO:
			return FALSE;
		case IDYES:
			GSSiRemove(listFile);
			break;
		}

	}
	strcpy(CurrentZoomList, name);
	if (listIsShared)
	{
		CRAPI_Init();
		sprintf(strchr(name, 0), "_%2.2i", CRAPI->sharedInstance.currentiPadWithinManager);
		if (SharedZoomListExists(name))
		{
			short opt = MessageBox(hWndDlg, mess, "", MB_YESNOCANCEL);
			switch (opt)
			{
			case IDCANCEL:
			case IDNO:
				return FALSE;
			case IDYES:
				break;
			}

		}
		rtn = CreateSharedZoomList(name);
	}
	else
	{
		Fid = GSSiOpenFile(listFile, 0, OF_CREATE);
		if (Fid != HFILE_ERROR)
		{
			strcpy(str, "[%DEFAULTZOOMOFFSET]=00000100.0;[%NEXTZOOMRECORD]=0000000000;");
			fputstring(str, Fid);
			rtn = TRUE;
			GSSiClose2(&Fid);
		}
	}
	return rtn;
}

int GetSharedZoomListEntries(HWND hWndDlg, UINT ListID)
{
	int nItems = 0;
	MNMXCORD bounds;
	LPMNMXCORD pBounds = &bounds;
	char line[1024];
	char blank[2] = "";
	int fileLoc=0;
	CRAPI_Init();

	BOOL opened = openDatabaseID(CRAPI->sharedInstance.currentDatabaseID);
	char query[256];
	sprintf(query, "SELECT rowid, * FROM ZOOMLIST_%s", CurrentZoomList);
	sqlite3_stmt *statement;

	SQLOK(SQLitePrepare(database, query, -1, &statement, 0), database, "GetSharedZoomListEntries", 0);
	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		int i = 0;
		fileLoc = sqlite3_column_int(statement, i++);
		LPSTR pName = (LPSTR)sqlite3_column_text(statement, i++);
		bounds.xmn = sqlite3_column_double(statement, i++);
		bounds.ymn = sqlite3_column_double(statement, i++);
		bounds.xmx = sqlite3_column_double(statement, i++);
		bounds.ymx = sqlite3_column_double(statement, i++);
		LPSTR pStatus = (LPSTR)sqlite3_column_text(statement, i++);
		if (!pStatus)
			pStatus = blank;
		sprintf(line, "%s\t(%f %f %f %f)", pName, pBounds->xmn, pBounds->ymn, pBounds->xmx, pBounds->ymx);

		int item = SendDlgItemMessage(hWndDlg, IDC_ZOOMLISTCONTENTS, LB_ADDSTRING, (WPARAM)0, (LPARAM)line);
		SendDlgItemMessage(hWndDlg, IDC_ZOOMLISTCONTENTS, LB_SETITEMDATA, (WPARAM)item, (LPARAM)fileLoc);
	}
	SQLOK(SQLiteFinalize(statement), database, "getLastPictUpdateNumber", 0);
	closeDatabaseID(CRAPI->sharedInstance.currentDatabaseID, opened);

	return nItems;
}
