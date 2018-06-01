#include "graphint.h"   
#include "gmextern.h"
#include "RampCompliance.h"
#include "MPIntersection.h"

#define CURRENT_INTERSECTION_VERSION "3.0"

static sqlite3 *database = NULL;

BOOL getMPIntersectionFromDB(int intID, BOOL wantRamps, MPINTERSECTION * pMPInt);
void convertVersion(LPSTR str, int fromVer, int toVer);
void convertVersion_1_to_2(LPSTR str);
void convertVersion_2_to_3(LPSTR str);
BOOL createIntersectionsTable(BOOL dropExistingTables);

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
	GSSiClose(FidOld);
	GSSiClose(FidNew);
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
							GSSiClose(fid);
						}
					}
					first = FALSE;
				}
			}
			Execute("COMMIT",0);
			GSSiClose(FidList);

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
			GSSiClose(fidTemp);
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
			GSSiClose(fidTemp);
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
					FormatStreets(pNames,FormattedStreets);
					sprintf(line, "%i\t%f\t%f\t%s", intersectionID, lat, lon, FormattedStreets);
					fputstring(line, fid);
				}
				SQLOK(sqlite3_finalize(statement), database, "get mpint", 0);
			}
			rtn++;
		}
		SQLOK(sqlite3_finalize(statement), database, "get mpint", 0);
		rc = sqlite3_close(database);
		GSSiClose(fid);
	}
	return rtn;
}

BOOL OutputRampsForIntersectionsInListToFile(LPSTR List, LPSTR OutFile, LPSTR NVCRISDataBase, int codeSystem, int headerType)
{
	BOOL rtn = FALSE;
	int rc;
	ToleranceValues tolerances;
	setStandardToleranceValues(&tolerances);

	rc = sqlite3_open(NVCRISDataBase, &database);
	if (rc == SQLITE_OK)
	{
		HFILE FidList = GSSiOpenFile(List, 0, OF_READ);
		if (FidList != HFILE_ERROR)
		{
			HFILE FidOut = GSSiOpenFile(OutFile, 0, OF_CREATE);
			if (FidOut != HFILE_ERROR)
			{
				LPSTR rampHeader = (LPSTR)rampToTextHeader(headerType);
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
								LPSTR detailCode;
								LPSTR ccode = rampComplianceCode(pRamp, &detailCode, &tolerances, codeSystem);
								LPSTR rampText = rampToText(pmpInt->intID, pRamp);
								sprintf(line, "%s\t%s\t%s", rampText, detailCode, ccode);
								fputstring(line, FidOut);
								free(ccode);
								free(detailCode);
								free(rampText);
							}
						}
					}
					else
						ii = 1;
				}
				free(line);
				free(pmpInt);
				GSSiClose(FidOut);
				rtn = TRUE;
			}
			GSSiClose(FidList);
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
		GSSiClose(fid);
	}
	return rtn;
}

BOOL OutputRampsToFile(LPSTR OutFile, LPSTR NVCRISDataBase, int codeSystem, int headerType, int completionCode)
{
	BOOL rtn = FALSE;
	int rc;
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
				LPSTR rampHeader = (LPSTR)rampToTextHeader(headerType);
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
								LPSTR detailCode;
								LPSTR ccode = rampComplianceCode(pRamp, &detailCode, &tolerances, codeSystem);
								LPSTR rampText = rampToText(pmpInt->intID, pRamp);
								sprintf(line, "%s\t'%s'\t'%s'", rampText, detailCode, ccode);
								fputstring(line, FidOut);
								free(ccode);
								free(detailCode);
								free(rampText);
							}
						}
					}
					else
						ii = 1;
				}
				free(line);
				free(pmpInt);
				GSSiClose(FidOut);
				rtn = TRUE;
			}
			GSSiClose(FidList);
		}
		rc = sqlite3_close(database);
	}
	return rtn;
}
BOOL OutputRampForIntersectionAndRampnumToFile(int intID, int rampNum, LPSTR OutFile, LPSTR NVCRISDataBase, int codeSystem, int headerType)
{
	BOOL rtn = FALSE;
	OFSTRUCTGM OFStruct;
	HFILE fid;
	int rc;
	ToleranceValues tolerances;
	setStandardToleranceValues(&tolerances);

	rampNum = fixRampNum(rampNum);
	if (rampNum < 0 || rampNum > 12)
		return FALSE;

	fid = GSSiOpenFile(NVCRISDataBase, &OFStruct, OF_READ);
	GSSiClose(fid);
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
					LPSTR rampHeader = (LPSTR)rampToTextHeader(headerType);
					fputstring(rampHeader, FidOut);
				}
				LPSTR line = malloc(4096);
				MPINTERSECTION *pmpInt = malloc(sizeof(MPINTERSECTION)+4);
				if (getMPIntersectionFromDB(intID, TRUE, pmpInt))
				{
					RampStruct * pRamp = &pmpInt->ramps[rampNum];
					if (pRamp->rampExists)
					{
						LPSTR detailCode;
						LPSTR ccode = rampComplianceCode(pRamp, &detailCode, &tolerances, codeSystem);
						LPSTR rampText = rampToText(pmpInt->intID, pRamp);
						sprintf(line, "%s\t%s\t%s", rampText, detailCode, ccode);
						fputstring(line, FidOut);
						free(ccode);
						free(detailCode);
						free(rampText);
					}
					else
						ii = 1;
				}
				else
					ii = 1;
				free(line);
				free(pmpInt);
				GSSiClose(FidOut);
				rtn = TRUE;
			}
			rc = sqlite3_close(database);

		}
	}
	return rtn;
}

BOOL ComplianceCodeForRamp(int intID, int rampNum, LPSTR NVCRISDataBase, int codeSystem, LPSTR OutLoc)
{
	BOOL rtn = FALSE;
	OFSTRUCTGM OFStruct;

	*OutLoc = 0;
	rampNum = fixRampNum(rampNum);
	HFILE fid = GSSiOpenFile(NVCRISDataBase, &OFStruct, OF_READ);
	GSSiClose(fid);
	if (fid != HFILE_ERROR)
	{
		int rc = sqlite3_open_v2(OFStruct.szPathName, &database, SQLITE_OPEN_READONLY, NULL);
		if (rc == SQLITE_OK)
		{
			MPINTERSECTION *pMPInt = malloc(sizeof(MPINTERSECTION)+4);

			if (getMPIntersectionFromDB(intID, TRUE, pMPInt))
			{
				if (rampNum > 0 && rampNum < 13)
				{
					ToleranceValues tolerances;
					setStandardToleranceValues(&tolerances);

					RampStruct * pRamp = &pMPInt->ramps[rampNum];
					if (pRamp->rampExists)
					{
						LPSTR detailCode;
						LPSTR ccode = rampComplianceCode(pRamp, &detailCode, &tolerances, codeSystem);
						strcpy(OutLoc, ccode);
						free(ccode);
						free(detailCode);
					}
				}
			}
			free(pMPInt);
			rc = sqlite3_close(database);
		}
	}
	return rtn;
}

int getMiddleRampIDFromRampID(int rampID)
{
	int rtn = 0;

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
		rtn = 23;
		break;
	case 10:
		rtn = 45;
		break;
	case 11:
		rtn = 67;
		break;
	default:
		rtn = 81;
		break;
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

void MPIntersectionInit(MPINTERSECTION * mpint)
{
	memset(mpint, 0, sizeof(MPINTERSECTION));
}
BOOL getMPIntersectionFromDB(int intID, BOOL wantRamps,MPINTERSECTION * pMPInt)
{
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
		//int nStreets = sqlite3_column_int(statement, 27);
		streets = (LPSTR)sqlite3_column_text(statement, 29);
		if (streets)
			strcpy(streetString, streets);
		//ReplaceChar(streetString, '|', '\n');
		comment = (LPSTR)sqlite3_column_text(statement, 33);
		if (comment)
			strcpy(mpint.intersectionComment, comment);
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
	}

	SQLOK(sqlite3_finalize(statement), database, "get mpint", 0);
	if (wantRamps && haveIntersection)
	{
		sprintf(query, "SELECT rowid,* FROM Ramps WHERE intID=%i", intID);
		SQLOK(sqlite3_prepare_v2(database, query, -1, &statement, NULL), database, "get mpint", 0);
		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			int i = 0;
			int uniqueID = sqlite3_column_int(statement, i++);
			int intersectionID = sqlite3_column_int(statement, i++);
			int rampNum = sqlite3_column_int(statement, i++);
			rampNum = fixRampNum(rampNum);
			RampStruct ramp = { 0 };

			ramp.uniqueID = uniqueID;

			ramp.rampNum = rampNum;
			LPSTR rampID = (LPSTR)sqlite3_column_text(statement, i++);
			strncpy0(ramp.rampID, rampID,sizeof(ramp.rampID)-1);
			Strip(ramp.rampID, ' ');
			ramp.yearRebuilt = sqlite3_column_int(statement, i++);
			ramp.timeComplete = sqlite3_column_int(statement, i++);
			ramp.rampExists = sqlite3_column_int(statement, i++);
			ramp.isComplete = sqlite3_column_int(statement, i++);
			ramp.approximateHeading = sqlite3_column_double(statement, i++);
			ramp.adjustedRot = sqlite3_column_int(statement, i++);
			ramp.rampInXWalk = sqlite3_column_int(statement, i++);
			ramp.xWalkisComplete = sqlite3_column_int(statement, i++);
			ramp.signalisComplete = sqlite3_column_int(statement, i++);
			ramp.texture = sqlite3_column_int(statement, i++);
			ramp.upperLandingObstruction = sqlite3_column_int(statement, i++);
			ramp.lowerLandingObstruction = sqlite3_column_int(statement, i++);
			ramp.rampObstruction = sqlite3_column_int(statement, i++);
			ramp.hasRampCracks = sqlite3_column_int(statement, i++);
			ramp.hasUpperLandingCracks = sqlite3_column_int(statement, i++);
			ramp.hasStreetLandingCracks = sqlite3_column_int(statement, i++);
			ramp.rampWidth = sqlite3_column_int(statement, i++);
			ramp.rampDepth = sqlite3_column_int(statement, i++);
			ramp.rampSlopeFront = sqlite3_column_double(statement, i++);
			ramp.rampSlopeSide = sqlite3_column_double(statement, i++);
			ramp.rampSlopeHeading = sqlite3_column_double(statement, i++);
			ramp.upperLandingSlopeFront = sqlite3_column_double(statement, i++);
			ramp.upperLandingSlopeSide = sqlite3_column_double(statement, i++);
			ramp.upperLandingSlopeHeading = sqlite3_column_double(statement, i++);
			ramp.streetLandingSlopeFront = sqlite3_column_double(statement, i++);
			ramp.streetLandingSlopeSide = sqlite3_column_double(statement, i++);
			ramp.streetLandingSlopeHeading = sqlite3_column_double(statement, i++);
			ramp.flareLeftSlopeFront = sqlite3_column_double(statement, i++);
			ramp.flareLeftSlopeSide = sqlite3_column_double(statement, i++);
			ramp.flareLeftSlopeHeading = sqlite3_column_double(statement, i++);
			ramp.flareRightSlopeFront = sqlite3_column_double(statement, i++);
			ramp.flareRightSlopeSide = sqlite3_column_double(statement, i++);
			ramp.flareRightSlopeHeading = sqlite3_column_double(statement, i++);
			ramp.swkLeftSlopeFront = sqlite3_column_double(statement, i++);
			ramp.swkLeftSlopeSide = sqlite3_column_double(statement, i++);
			ramp.swkLeftSlopeHeading = sqlite3_column_double(statement, i++);
			ramp.swkRightSlopeFront = sqlite3_column_double(statement, i++);
			ramp.swkRightSlopeSide = sqlite3_column_double(statement, i++);
			ramp.swkRightSlopeHeading = sqlite3_column_double(statement, i++);
			ramp.PEDSignalType = sqlite3_column_int(statement, i++);
			ramp.PEDButtonType = sqlite3_column_int(statement, i++);
			ramp.PEDButtonHeight = sqlite3_column_int(statement, i++);
			ramp.PEDButtonDist = sqlite3_column_int(statement, i++);
			ramp.SteepTopOfCurb = sqlite3_column_double(statement, i++);
			ramp.PedRampLip = sqlite3_column_double(statement, i++);
			ramp.lev21x = sqlite3_column_int(statement, i++);
			ramp.lev21y = sqlite3_column_int(statement, i++);
			PixelXYToLatLong(ramp.lev21x, ramp.lev21y, 21, &ramp.latitude, &ramp.longitude);
			ramp.rampType = sqlite3_column_int(statement, i++);
			LPSTR comment = (LPSTR)sqlite3_column_text(statement, i++);
			if (comment && *comment)
				strcpy(ramp.rampComment, comment);
			else
				getCornerComment (intID,rampNum,ramp.rampComment);
			ramp.awi = sqlite3_column_int(statement, i++);
			ramp.hasLocatorTone = sqlite3_column_int(statement, i++);
			ramp.hasInfoSign = sqlite3_column_int(statement, i++);
			ramp.hasBraille = sqlite3_column_int(statement, i++);
			ramp.hasTactileArrow = sqlite3_column_int(statement, i++);
			ramp.locatorToneVolume = sqlite3_column_int(statement, i++);
			ramp.audibleWalkIndicationVolume = sqlite3_column_int(statement, i++);
			CrackWidth cw;
			cw.rampCrackWidth = sqlite3_column_double(statement, i++);
			cw.upperLandingCrackWidth = sqlite3_column_double(statement, i++);
			cw.streetLandingCrackWidth = sqlite3_column_double(statement, i++);
			cw.leftSidewalkCrackWidth = sqlite3_column_double(statement, i++);
			cw.rightSidewalkCrackWidth = sqlite3_column_double(statement, i++);
			ramp.crackWidth = cw;
			ramp.curbCutDistance = sqlite3_column_double(statement, i++);
			ramp.bumpWidth = sqlite3_column_double(statement, i++);
			ramp.bumpHeight = sqlite3_column_double(statement, i++);
			ramp.dwWidth = sqlite3_column_double(statement, i++);
			ramp.dwDepth = sqlite3_column_double(statement, i++);
			LPSTR fileID = (LPSTR)sqlite3_column_text(statement, i++);
			strncpy0(ramp.fileID, fileID, sizeof(ramp.fileID) - 1);

			if (ramp.bumpWidth > 0 || ramp.bumpHeight > 0)
				ii = 1;
			if (mpint.timeComplete >= mpint.ramps[rampNum].timeComplete)
				mpint.ramps[rampNum] = ramp;
		}
		SQLOK(sqlite3_finalize(statement), database, "get mpint", 0);
	}
	free(query);

	*pMPInt = mpint;
	return TRUE;
}
int FormatStreets(LPSTR from, LPSTR outtext)
{
	int nStreets = 0;
	int lfrom = strlen(from);
	int istreet = 0;

	if (lfrom > 0)
	{
		LPSTR streets = malloc(lfrom + 8);
		LPSTR pBar = streets;
		LPSTR pStreet[16];
		strcpy(streets, from);
		pStreet[0] = streets;
		nStreets++;
		while ((pBar = strchr(pBar, '|')))
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
	char *fromText[] = { "Home", "Ramp", "Signal", "Texture", "Obstruction", "Steep TOC", "Crack", "Curb Cut", "Bump Width", "Bump Height", "Manual Slope" };

	if (from > 0 && from < 12)
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
	char searchFor[] = "CREATE TABLE IF NOT EXISTS Ramps (";
	char searchFor2[] = "INSERT OR REPLACE INTO Ramps VALUES(";
	int lenSearch = strlen(searchFor);
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
				convertVersion(str, fromVer, toVer);
			if (insertFileID)
			{
				if (!strnicmp(str, searchFor, lenSearch))
				{
					REPLAC(str, "PRIMARY KEY", "FromFileID CHAR(12), PRIMARY KEY", maxLineLen);
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
					*(ptotErrors)++;
			}
		}
	}
	GSSiClose(fid);
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

void convertVersion(LPSTR str, int fromVer, int toVer)
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
}

/*BOOL adjustToLatestVersion
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
}*/
BOOL NVCreateDB(LPSTR path,BOOL Delete)
{
	BOOL rc;
	
	int type = FileType(path);

	if (type && !Delete)
		return FALSE;
	else if (type)
		GSSiRemove(path);
	else
	{
		HFILE fid = GSSiOpenFile(path, 0, OF_CREATE);
		GSSiClose(fid);
		GSSiRemove(path);
	}

	rc = sqlite3_open(path, &database);
	createIntersectionsTable(TRUE);
	rc = sqlite3_close(database);
	return rc;
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
FromFileID CHAR(12), PRIMARY KEY (intID,rampNum ASC));";
rtn = executeCmd(createcmd2);

char createcmd3[] = "CREATE TABLE IF NOT EXISTS CURBRAMP_PICTURES ('id' INTEGER PRIMARY KEY,'iPadNum' INT,'pictNum' INT,'intID' INT,'rampNum' INT, 'type' INT, 'heading' INT, 'latitude' REAL, 'longitude' REAL, 'time' INT)";
rtn = executeCmd(createcmd3);
char createcmd4[] = "CREATE TABLE IF NOT EXISTS CURBRAMP_NOTES ('id' INTEGER PRIMARY KEY,'intID' INT,'corner' INT,'rampID' INT,'type' INT, 'note' CHAR(4096))";
rtn = executeCmd(createcmd4);
char createcmd5[] = "CREATE TABLE IF NOT EXISTS CURBRAMP_STANDARD_TEXT ('textID' INTEGER PRIMARY KEY,'type' INT,'text' CHAR(4096))";
rtn = executeCmd(createcmd5);
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

