#include "graphint.h"   
#include "gmextern.h"
#include "RampCompliance.h"
#include "MPIntersection.h"

#define FIRSTYEAR	2004
#define LASTYEAR	2015
#define NYEARS	12
#define NUMVARS 64
#define MAXLINELEN 2048

static LPSTR pValues[NYEARS];
static HANDLE hIndexYear[NYEARS] = { 0 };
static HFILE fidYear[NYEARS];

static void test(LPSTR INDir);

int LoadMultPropertyDB(LPSTR INDir)
{
	int rtn = 0;
	BTVARDESC   BTVar[2];
	HANDLE	hIndex = 0;
	int year = 2004;
	int offset;
	short	st;
	char inFile[MAX_PATH];
	char indexFile[MAX_PATH];
	int nRecs;
	HFILE fid;
	char line[SHRT_MAX];
	int maxLineLen = SHRT_MAX - 2;
	int mxlnlen = 0;

	test(INDir);
	while (year < 2016)
	{
		BTVar[0].BT_VARTYP = BT_CHAR;
		BTVar[0].BT_VARLEN = 17;
		BTVar[0].BT_VAROFF = 0;
		sprintf(inFile, "%s\\%i.txt", INDir, year);
		fid = GSSiOpenFile(inFile, 0, OF_READ);
		nRecs = GSSifilelength(fid);
		sprintf(indexFile, "%s\\%i.index", INDir, year);
		BT_CREATE(indexFile, sizeof(offset), FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
		hIndex = BT_OPEN(indexFile, 0, BT_WRITE, 0);
		offset = 0;

		sprintf(line, "Create Index %i %i", year,mxlnlen);
		CreateStatusWind(hWndMain, 1,line);
		while (StatusWindowUpdate(NULL, NULL, nRecs, offset) && fgetstring(line, maxLineLen, fid))
		{
			LPSTR pTab = strchr(line, '\t');
			mxlnlen = max(mxlnlen, strlen(line));
			if (pTab)
			{
				LPSTR pPid = ++pTab;
				pTab = strchr(pPid, '\t');
				*pTab++ = 0;
				offset += (pTab - line);
				BT_PUT(hIndex, pPid, (LPSTR)&offset);
				offset = GSSillseek(fid, 0, 1);
			}
		}
		GSSiClose(fid);
		BT_CLOSE(hIndex);
		DestroyStatusWindow(0);
		year++;
	}
	rtn = mxlnlen;

	return rtn;
}

static void test(LPSTR INDir)
{
	char pid[18] = "003-263425110003";
	int year = 2004;
	int offset;
	short	st;
	char inFile[MAX_PATH];
	char indexFile[MAX_PATH];
	int nRecs;
	HFILE fid;
	HANDLE hIndex;
	char line[USHRT_MAX];
	int maxLineLen = USHRT_MAX - 2;

	sprintf(inFile, "%s\\%i.txt", INDir, year);
	fid = GSSiOpenFile(inFile, 0, OF_READ);
	nRecs = GSSifilelength(fid);
	sprintf(indexFile, "%s\\%i.index", INDir, year);
	hIndex = BT_OPEN(indexFile, 0, BT_READ, 0);
	st = BT_FIND(hIndex, pid, BT_FIRST, BT_EQ,(LPSTR) &offset);

	GSSillseek(fid, offset, 0);
	fgetstring(line, maxLineLen, fid);
	BT_CLOSE(hIndex);
	GSSiClose(fid);
	return;

}

static void FindAllRecords(LPSTR pid)
{
	int offset;

	for (int i = 0; i < NYEARS; i++)
	{
		if (!BT_FIND(hIndexYear[i], pid, BT_FIRST, BT_EQ, (LPSTR)&offset))
		{
			GSSillseek(fidYear[i], offset, 0);
			fgetstring(pValues[i], MAXLINELEN - 2, fidYear[i]);
		}
		else
			*pValues[i] = 0;
	}

}
static void GetVarValue(int ivar, LPSTR line, LPSTR value)
{
	LPSTR pTab=line;
	LPSTR pEnd;

	*value = 0;
	while (ivar--)
	{
		pTab = strchr(pTab, '\t');
		if (!pTab)
			return;
		pTab++;
	}
	pEnd = strchr(pTab, '\t');
	if (pEnd)
		*pEnd = 0;
	strcpy(value, pTab);
	if (pEnd)
		*pEnd = '\t';
	return;
}
void OpenYearFiles(LPSTR INDir)
{
	char inFile[MAX_PATH];
	char indexFile[MAX_PATH];
	for (int i = 0; i < NYEARS; i++)
	{
		sprintf(inFile, "%s\\%i.txt", INDir, LASTYEAR-i);
		fidYear[i] = GSSiOpenFile(inFile, 0, OF_READ);
		sprintf(indexFile, "%s\\%i.index", INDir, LASTYEAR - i);
		hIndexYear[i] = BT_OPEN(indexFile, 0, BT_READ, 0);
	}
}
void CloseYearFiles(void)
{
	for (int i = 0; i < NYEARS; i++)
	{
		BT_CLOSE(hIndexYear[i]);
		GSSiClose(fidYear[i]);
	}
}
int CreateMultValueFile(LPSTR INDir)
{
	BOOL rtn = FALSE;
	int offset;
	short	st;
	char inFile[MAX_PATH];
	char indexFile[MAX_PATH];
	int year = 2016;
	HANDLE hIndex;
	char line[1024];
	int maxLineLen = 1020;
	char pid[18];
	HFILE fid;
	int pos = BT_FIRST;
	char curValue[1024];
	char value[1024];
	LPSTR pChangeValues = malloc(USHRT_MAX);
	LPSTR pChangeValuesCompressed = malloc(USHRT_MAX);
	char valueTerminator = 1;
	char valuesTerminator = 2;
	int lnChangeValues;
	int lnChangeValuesCompressed;
	int totlnChangeValues = 0;
	int totlnChangeValuesCompressed = 0;
	int nRecs;
	int iRec = 0;
	for (int i = 0; i < NYEARS; i++)
	{
		pValues[i] = malloc(MAXLINELEN + 4);
	}
	sprintf(inFile, "%s\\%i.txt", INDir, year);
	fid = GSSiOpenFile(inFile, 0, OF_READ);
	sprintf(indexFile, "%s\\%i.index", INDir, year);
	hIndex = BT_OPEN(indexFile, 0, BT_READ, 0);
	OpenYearFiles(INDir);
	CreateStatusWind(hWndMain, 1, 0);
	nRecs = BT_NUM_IN_INDEX(hIndex);
	while (StatusWindowUpdate(NULL, NULL, nRecs, iRec++) && !BT_FIND(hIndex, pid, pos, BT_ANY, (LPSTR)&offset))
	{
		*pChangeValues = 0;
		pos = BT_NEXT;
		if (*pid)
		{
			GSSillseek(fid, offset, 0);
			fgetstring(line, maxLineLen, fid);
			FindAllRecords(pid);
			//for (int ivar = 0; ivar < NUMVARS; ivar++)
			for (int ivar = 32; ivar < 38; ivar++)
			{
				GetVarValue(ivar, line, curValue);
				for (int iyear = LASTYEAR; iyear>FIRSTYEAR; iyear--)
				{
					GetVarValue(ivar, pValues[iyear - FIRSTYEAR], value);
					if (strcmp(curValue, value))
					{
						sprintf(strchr(pChangeValues, 0), "%s%c", value, valueTerminator);
						strcpy(curValue, value);
					}
				}
				sprintf(strchr(pChangeValues, 0), "%c", valuesTerminator);
			}
			lnChangeValues = strlen(pChangeValues);
			lnChangeValuesCompressed = CompressBinaryRecord(pChangeValues, pChangeValuesCompressed, lnChangeValues);
			totlnChangeValues += lnChangeValues;
			totlnChangeValuesCompressed += lnChangeValuesCompressed;
		}
	}
	BT_CLOSE(hIndex);
	DestroyStatusWindow(0);
	GSSiClose(fid);
	CloseYearFiles();
	free(pChangeValues);
	free(pChangeValuesCompressed);
	for (int i = 0; i < NYEARS; i++)
	{
		free(pValues[i]);
	}

	return totlnChangeValuesCompressed;
}
BOOL AssignMultValues(LPSTR indexFile, LPSTR dataFile)
{
	BOOL rtn = FALSE;
	HANDLE hIndex;
	HFILE  fidBin;
	char	pid[32];
	int nRecs;
	int iRec = 0;
	int pos = BT_FIRST;
	int offset;
	char KeyString[256], UpdateString[256];

	hIndex = BT_OPEN(indexFile, 0, BT_READ, 0);
	if (!hIndex)
		return FALSE;

	CreateStatusWind(hWndMain, 1, 0);
	nRecs = BT_NUM_IN_INDEX(hIndex);
	while (StatusWindowUpdate(NULL, NULL, nRecs, iRec++) && !BT_FIND(hIndex, pid, pos, BT_ANY, (LPSTR)&offset))
	{
		pos = BT_NEXT;
		sprintf(KeyString, "PIN=%s", pid);
		sprintf(UpdateString, "MULTIYEAROFFSET=%i",offset);
		rtn = UpdateGMDFile(dataFile, KeyString, UpdateString, ';', 1, FALSE);
	}
	BT_CLOSE(hIndex);
	DestroyStatusWindow(0);

	return rtn;
}
