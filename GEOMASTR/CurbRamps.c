#include "graphint.h"   
#include "gmextern.h"
#include "RampCompliance.h"
#include "MPIntersection.h"

#define CURRENT_INTERSECTION_VERSION "2.0"

static sqlite3 *database = NULL;

BOOL getMPIntersectionFromDB(int intID, BOOL wantRamps, MPINTERSECTION * pMPInt);
void convertVersion(LPSTR str, int fromVer, int toVer);
void convertVersion_1_to_2(LPSTR str);
static BOOL Execute(LPSTR cmd);
BOOL UpdateFromFile(LPSTR file, BOOL convertInsert);


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
BOOL LoadFilesInListInChronologicalSequence(LPSTR List,LPSTR DataBase,BOOL showProgress)
{
#define LINELEN	USHRT_MAX
	BOOL rtn = FALSE;
	int rc;
	int nTot=0, nDone = 0;
	LPSTR line = malloc(LINELEN);

	rc = sqlite3_open(DataBase, &database);
	if (rc == SQLITE_OK)
	{
		HFILE FidList = GSSiOpenFile(List, 0, OF_READ);
		if (FidList != HFILE_ERROR)
		{
			LPSTR file = malloc(260);
			Execute("BEGIN");

			sprintf(line, "DROP TABLE IF EXISTS SORTEDFILES;CREATE TABLE SORTEDFILES (TIME INT,FILEPATH CHAR(256));");
			if (Execute(line))
			{
				fgetstring(file, 258, FidList);
				while (fgetstring(file, 258, FidList))
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
							Execute(line);
							nTot++;
						}
						GSSiClose(fid);
					}
				}
			}
			Execute("COMMIT");
			GSSiClose(FidList);
			sprintf(line, "SELECT FILEPATH FROM SORTEDFILES ORDER BY TIME ASC;");
			sqlite3_stmt *statement = NULL;
			if (showProgress)
			{
				CreateStatusWind(hWndMain, 1, "Loading Data");
			}
			if (sqlite3_prepare_v2(database,line, -1, &statement, 0) == SQLITE_OK)
			{
				while (sqlite3_step(statement) == SQLITE_ROW)
				{
					LPSTR filePath = (LPSTR) sqlite3_column_text(statement, 0);
					BOOL st = UpdateFromFile(filePath,TRUE);
					if (showProgress)
						StatusWindowUpdate(0, 0, nTot, ++nDone);

				}
				sqlite3_finalize(statement);
			}
			if (showProgress)
				DestroyStatusWindow(0);

			free(file);
		}
		rc = sqlite3_close(database);
	}
	free(line);
	return rtn;
}

BOOL OutputRampsForIntersectionsInListToFile(LPSTR List, LPSTR OutFile, LPSTR NVCRISDataBase, int codeSystem)
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
				LPSTR rampHeader = (LPSTR)rampToTextHeader();
				fputstring(rampHeader, FidOut);
				LPSTR line = malloc(4096);
				while (fgetstring(line, sizeof(line)-2, FidList))
				{
					int intID = atoi(line);
					MPINTERSECTION mpInt;
					if (getMPIntersectionFromDB(intID, TRUE, &mpInt))
					{
						for (int i = 1; i < 13; i++)
						{
							RampStruct * pRamp = &mpInt.ramps[i];
							if (pRamp->rampExists)
							{
								LPSTR detailCode;
								LPSTR ccode = rampComplianceCode(pRamp, &detailCode, &tolerances,codeSystem);
								LPSTR rampText = rampToText(mpInt.intID, pRamp);
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
				GSSiClose(FidOut);
				rtn = TRUE;
			}
			GSSiClose(FidList);
		}
		rc = sqlite3_close(database);

	}
	return rtn;
}
BOOL OutputRampForIntersectionAndRampnumToFile(int intID, int rampNum, LPSTR OutFile, LPSTR NVCRISDataBase, int codeSystem)
{
	BOOL rtn = FALSE;
	int rc;
	ToleranceValues tolerances;
	setStandardToleranceValues(&tolerances);

	rc = sqlite3_open(NVCRISDataBase, &database);
	if (rc == SQLITE_OK)
	{
		HFILE FidOut = GSSiOpenFile(OutFile, 0, OF_CREATE);
		if (FidOut != HFILE_ERROR)
		{
			LPSTR rampHeader = (LPSTR)rampToTextHeader();
			fputstring(rampHeader, FidOut);
			LPSTR line = malloc(4096);
			MPINTERSECTION mpInt;
			if (getMPIntersectionFromDB(intID, TRUE, &mpInt))
			{
				RampStruct * pRamp = &mpInt.ramps[rampNum];
				if (pRamp->rampExists)
				{
					LPSTR detailCode;
					LPSTR ccode = rampComplianceCode(pRamp, &detailCode, &tolerances,codeSystem);
					LPSTR rampText = rampToText(mpInt.intID, pRamp);
					sprintf(line, "%s\t%s\t%s", rampText, detailCode, ccode);
					fputstring(line, FidOut);
					free(ccode);
					free(detailCode);
					free(rampText);
				}
			}
			else
				ii = 1;
			free(line);
			GSSiClose(FidOut);
			rtn = TRUE;
		}
		rc = sqlite3_close(database);

	}
	return rtn;
}

BOOL ComplianceCodeForRamp(int intID, int rampNum, LPSTR NVCRISDataBase, int codeSystem, LPSTR OutLoc)
{
	BOOL rtn = FALSE;
	*OutLoc = 0;
	int rc = sqlite3_open(NVCRISDataBase, &database);
	if (rc == SQLITE_OK)
	{
		MPINTERSECTION MPInt;

		if (getMPIntersectionFromDB(intID, TRUE, &MPInt))
		{
			if (rampNum > 0 && rampNum < 13)
			{
				ToleranceValues tolerances;
				setStandardToleranceValues(&tolerances);

				RampStruct * pRamp = &MPInt.ramps[rampNum];
				if (pRamp->rampExists)
				{
					LPSTR detailCode;
					LPSTR ccode = rampComplianceCode(pRamp, &detailCode, &tolerances,codeSystem);
					strcpy(OutLoc, ccode);
					free(ccode);
					free(detailCode);
				}
			}
		}
		rc = sqlite3_close(database);
	}
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
			if (comment)
				strcpy(ramp.rampComment, comment);
			else
				*ramp.rampComment = 0;
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
			if (ramp.bumpWidth > 0 || ramp.bumpHeight > 0)
				ii = 1;
			mpint.ramps[rampNum] = ramp;
		}
		SQLOK(sqlite3_finalize(statement), database, "get mpint", 0);
	}
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

static BOOL Execute(LPSTR cmd)
{
	BOOL rtn = !SQLOK(sqlite3_exec(database, cmd, 0, 0, 0), database, "", 0);
	return rtn;
}

BOOL UpdateFromFile(LPSTR file,BOOL convertInsert)
{
	BOOL rtn = TRUE;
	int line = 0;
	HFILE fid = GSSiOpenFile(file, 0, OF_READ);
	int totLen = GSSifilelength(fid);
	int maxLineLen = totLen + 2;
	LPSTR str = malloc(totLen + 4096);
	int err = Execute("BEGIN");
	if (fgetstring(str, maxLineLen, fid))
	{
		int fromVer = atoi(CURRENT_INTERSECTION_VERSION);
		int toVer = atoi(CURRENT_INTERSECTION_VERSION);
		LPSTR vloc = strstr(str, "DBVer ");
		if (vloc)
			fromVer = atoi(vloc + 6);
		while (rtn && fgetstring(str, maxLineLen, fid))
		{
			convertVersion(str, fromVer, toVer);
			if (convertInsert)
				REPLAC(str, "INSERT INTO", "INSERT OR REPLACE INTO", maxLineLen + 4090);
			rtn = Execute(str);
			line++;
		}
	}
	GSSiClose(fid);
	if (!rtn) //file has to be edited on server (by GSSi) before more data can be loaded
	{
		LPSTR filename = strrchr(file, '\\');
		if (!filename)
			filename = file;
		err = Execute("ROLLBACK");
		LPSTR mess = malloc(USHRT_MAX);
		sprintf(mess, " in file %s at line %i\n%s", filename, line, str);
		GSSiMessageBox(2, mess, "Data load failure", MB_ICONEXCLAMATION, 0);
	}
	else
		err = Execute("COMMIT");
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

