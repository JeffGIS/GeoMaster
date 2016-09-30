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
	HANDLE	hIndex = 0;
	int year = 2016;
	int offset;
	short	st;
	char inFile[MAX_PATH];
	char indexFile[MAX_PATH];
	int nRecs;
	HFILE fid;
	char line[SHRT_MAX];
	int maxLineLen = SHRT_MAX - 2;
	int mxlnlen = 0;

	//test(INDir);
	while (year < 2017)
	{
		BTVARDESC   BTVar[2];
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
	char cvFile[MAX_PATH];
	char indexFile[MAX_PATH];
	char cvindexFile[MAX_PATH];
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
	HFILE fidChangeValues;
	HANDLE hChangeValueIndex;
	BOOL homestead, taxexempt;
	typedef struct { int EMV_LAND, EMV_BLDG, EMV_TOTAL, TAX_CAPACITY, TOTAL_TAX, SPEC_ASSES;
					}YEARLYVALUES;
	typedef YEARLYVALUES *LPYEARLYVALUES;
	LPYEARLYVALUES pYearly;
	BTVARDESC   BTVar[2];
	BTVar[0].BT_VARTYP = BT_CHAR;
	BTVar[0].BT_VARLEN = 17;
	BTVar[0].BT_VAROFF = 0;
	sprintf(cvFile, "%s\\changeValues.bin", INDir);
	fidChangeValues = GSSiOpenFile(cvFile, 0, OF_CREATE);
	sprintf(cvindexFile, "%s\\changeValues.index", INDir);
	BT_CREATE(cvindexFile, sizeof(offset), FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
	hChangeValueIndex = BT_OPEN(cvindexFile, 0, BT_WRITE, 0);

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
			/*for (int ivar = 31; ivar < 39; ivar++)
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
			}*/
			lnChangeValues = 0;
			pYearly = (LPYEARLYVALUES)pChangeValues;
			for (int iyear = 0; iyear < NYEARS; iyear++)
			{
				GetVarValue(31, pValues[iyear], value);
				homestead = atob(value);
				GetVarValue(32, pValues[iyear], value);
				pYearly->EMV_LAND = atoi(value);
				GetVarValue(33, pValues[iyear], value);
				pYearly->EMV_BLDG = atoi(value);
				GetVarValue(34, pValues[iyear], value);
				pYearly->EMV_TOTAL = atoi(value);
				GetVarValue(35, pValues[iyear], value);
				pYearly->TAX_CAPACITY = atoi(value);
				GetVarValue(36, pValues[iyear], value);
				pYearly->TOTAL_TAX = atoi(value);
				GetVarValue(37, pValues[iyear], value);
				pYearly->SPEC_ASSES = atoi(value);
				GetVarValue(38, pValues[iyear], value);
				taxexempt = atob(value);
				pYearly->EMV_LAND++;
				if (homestead) pYearly->EMV_LAND *= -1;
				pYearly->EMV_BLDG++;
				if (taxexempt) pYearly->EMV_BLDG *= -1;
				pYearly++;
			}
			//lnChangeValues = strlen(pChangeValues);
			lnChangeValues = NYEARS * sizeof(YEARLYVALUES);
			offset = GSSillseek(fidChangeValues, 0, 1);
			BT_PUT(hChangeValueIndex, pid, (LPSTR)&offset);
			lnChangeValuesCompressed = CompressBinaryRecord(pChangeValues, pChangeValuesCompressed, lnChangeValues);
			BigWrite(fidChangeValues, &lnChangeValuesCompressed, 4, -1);
			BigWrite(fidChangeValues, pChangeValuesCompressed, lnChangeValuesCompressed, -1);
			totlnChangeValues += lnChangeValues;
			totlnChangeValuesCompressed += lnChangeValuesCompressed;
		}
	}
	BT_CLOSE(hIndex);
	BT_CLOSE(hChangeValueIndex);
	GSSiClose(fidChangeValues);
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