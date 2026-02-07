#include "graphint.h"   
#include "gmextern.h"

#define SAME_AS_LAST 6
#define USE_FIRST_N	 7
#define REPLACE_LAST_N 8
#define USE_AS_IS 9

static LPSTR getFieldVal(LPSTR fld)
{
	int ln;
	LPSTR pOut;
	LPSTR pTab = strchr(fld, '\t');
	if (pTab)
		*pTab = 0;
	ln = strlen(fld) + 1;
	pOut = calloc(ln,1);
	strcpy(pOut, fld);
	if (pTab)
		*pTab = '\t';
	return pOut;
}
static LPSTR getNewField(LPSTR pCurField, LPSTR pPreField)
{
	LPSTR pNewField = malloc(1024);

	LPSTR pCurF = getFieldVal(pCurField);
	LPSTR pPreF = getFieldVal(pPreField);
	int lcur = strlen(pCurF);
	int lpre = strlen(pPreF);
	int nsame = 0;

	if (!lcur)
	{
		*pNewField = USE_AS_IS;
		pNewField[1] = 0;
	}
	else if (!strcmp(pCurF, pPreF))
	{
		*pNewField = SAME_AS_LAST;
		pNewField[1] = 0;
	}
	else if (lcur == lpre)
	{
		while (pCurF[nsame] ==  pPreF[nsame])
		{
			nsame++;
		}
		if (nsame > 1)
		{
			*pNewField = REPLACE_LAST_N;
			strcpy(&pNewField[1], &pCurF[nsame]);
		}
		else
		{
			*pNewField = USE_AS_IS;
			strcpy(&pNewField[1], pCurF);
		}
	}
	else if (lcur > 2 && lpre > 2 && pCurF[0] == pPreF[0] && pCurF[1] == pPreF[1])
	{
		nsame = 2;
		while (pCurF[nsame] && pCurF[nsame] == pPreF[nsame])
		{
			nsame++;
		}
		if (nsame > 1 && nsame < 240)
		{
			*pNewField = USE_FIRST_N;
			pNewField[1] = nsame + 10;
			strcpy(&pNewField[2], &pCurF[nsame]);
		}
		else
		{
			*pNewField = USE_AS_IS;
			strcpy(&pNewField[1], pCurF);
		}
	}
	else
	{
		*pNewField = USE_AS_IS;
		strcpy(&pNewField[1], pCurF);
	}
	return pNewField;
}
int GMMCompression(LPSTR INFile, LPSTR OUTFile)
{
	int rtn = 0;
	int flen = GSSiLength(INFile);
	HANDLE hMem = GSSiGlobAlloc(GAIDNO 2018, GMEM_MOVEABLE, 4096*8);
	LPSTR pFile = GlobalLock(hMem);
	HANDLE hMemCmp = GSSiGlobAlloc(GAIDNO 2019, GMEM_MOVEABLE, 4096*8);
	LPSTR pFileCmp = GlobalLock(hMemCmp);
	HFILE fid = GSSiOpenFile(INFile, 0, OF_READ);
	int flenCmp=0;
	int n = -1;
	int reclen;
	
	while (n < 0)
	{
		n = 8;
		reclen = 0;
		while (fgetstring(&pFile[reclen], 4000, fid) && n--)
		{
			int flen = strlen(&pFile[reclen]);
			reclen += flen;
		}
		flenCmp += CompressBinaryRecord(pFile, pFileCmp, reclen);
	}

	GSSiClose2 (&fid);
	GSSiGlobUlFree(&hMem);
	GSSiGlobUlFree(&hMemCmp);
	rtn = (100.0 * flenCmp) / flen;

	/*	int rtn = 0;
	int flen = GSSiLength(INFile);
	HANDLE hMem = GSSiGlobAlloc(GAIDNO 0, GMEM_MOVEABLE, flen + 4);
	LPSTR pFile = GlobalLock(hMem);
	HANDLE hMemCmp = GSSiGlobAlloc(GAIDNO 0, GMEM_MOVEABLE, flen * 2);
	LPSTR pFileCmp = GlobalLock(hMemCmp);
	HFILE fid = GSSiOpenFile(INFile, 0, OF_READ);
	int flenCmp;
	BigRead(fid, pFile, flen);
	GSSiClose2 (&fid);

	flenCmp = CompressBinaryRecord(pFile, pFileCmp, flen);
	GSSiGlobUlFree(&hMem);
	GSSiGlobUlFree(&hMemCmp);
	rtn = (100.0 * flenCmp) / flen;
	return rtn;
}*/
/*{
	int rtn = 0;
	int flen = GSSiLength(INFile);
	HANDLE hMem = GSSiGlobAlloc(GAIDNO 0, GMEM_MOVEABLE, flen + 4);
	LPSTR pFile = GlobalLock(hMem);
	HANDLE hMemCmp = GSSiGlobAlloc(GAIDNO 0, GMEM_MOVEABLE, flen * 2);
	LPSTR pFileCmp = GlobalLock(hMemCmp);
	HFILE fid = GSSiOpenFile(INFile, 0, OF_READ);
	int flenCmp=0;
	int lfile = 0;
	int nlines = 0;
	int lineBeg[1024] = { 0 };
	BigRead(fid, pFile, flen);
	flenCmp = CompressBinaryRecord(pFile, pFileCmp, flen);
	GSSillseek(fid, 0, 0);
	flenCmp = 0;
	while (fgetstring(&pFile[lfile], 4096, fid))
	{
		lfile += strlen(&pFile[lfile]) + 1;
		nlines++;
		lineBeg[nlines] = lfile;
	}
	GSSiClose2 (&fid);

	for (int il = nlines - 1; il > 0; il--)
	{
		int ifcur = lineBeg[il];
		int ifpre = lineBeg[il - 1];
		LPSTR pcurField = &pFile[ifcur];
		LPSTR ppreField = &pFile[ifpre];
		while (pcurField)
		{
			LPSTR pNewField = getNewField(pcurField, ppreField);
			strcpy(&pFileCmp[flenCmp], pNewField);
			flenCmp += strlen(pNewField);
			free(pNewField);
			pcurField = strchr(pcurField, '\t');
			ppreField = strchr(ppreField, '\t');
			if (pcurField)
			{
				pcurField++;
				ppreField++;
			}
			else
				break;
		}
	}
	flenCmp = CompressBinaryRecord(pFileCmp, pFile, flenCmp);
	GSSiGlobUlFree(&hMem);
	GSSiGlobUlFree(&hMemCmp);
	rtn = (100.0 * flenCmp) / flen;*/
	return rtn;
}
int FindDupParcels(LPSTR OUTFile)
{
	int rtn = 0;
	char	DefStr[] = "Refno(B4),UDI(C20),NP(B4),AREA(B4),MNX(B4),MNY(B4),MXX(B4),MXY(B4)";
	char	DefStr2[] = "Refno(B4),UDI(C20),DupRef(B4),dupUDI(C20),DupNum(B4)";
	LPGWDHEADER lpGWDHead;
	HIGHLIGHTDATA	HighlightData;
	long	Refno, Offset;
	short	pos = BT_FIRST;
	typedef struct {
		long	Refno;
		char	UDI[20];
		int	NP, AREA, MNX, MNY, MXX, MXY;
	}DUPDATA;
	typedef DUPDATA	FAR	*LPDUPDATA;
	LPDUPDATA	pData;
	typedef struct {
		long	Refno;
		char	UDI[20];
		int		DupRef;
		char	dupUDI[20];
		int		DupNum;
	}OUTDATA;
	typedef OUTDATA	FAR	*LPOUTDATA;
	LPOUTDATA	pOutData;
	struct { int ref, dupNum;
			char	UDI[20];
			char	dupUDI[20];
	}dupLink;
	DUPDATA lastDupData;
	HANDLE hDUPFile = 0;
	HANDLE hOUTFile = 0;
	int nTot = BT_NUM_IN_INDEX(hHighlight);
	int nComplete = 0;
	int ref, offset;
	int nDup=-1;
	int baseRef=0;
	char baseUDI[20] = { 0 };
	HANDLE hBTDupLink;
	BTVARDESC	BTVar[2];
	char		TempName[MAX_PATH];
	char		DUPFile[MAX_PATH];
	LPSTR pDot;

	GSSiGetTempFileName(0, "gmd", 0, (LPSTR)DUPFile);
	pDot = strrchr(DUPFile, '.');
	strcpy(pDot, ".gmd");
	if (CreateGWDDatabase(DUPFile, 1, FALSE, 0, 1, DefStr))
	{
		HANDLE	hKeyFields;

		GetFieldIDsFromNames(DUPFile, &hKeyFields, 0, "NP;AREA;MNX;MNY;MXX;MXY", 0);
		GWDAddIndex(DUPFile, hKeyFields, FALSE, 0);
		GSSiGlobFree(&hKeyFields);
	}
	else
		return -1;
	hDUPFile = OpenGWDatabase(DUPFile, BT_WRITE);
	lpGWDHead = (LPGWDHEADER)GlobalLock(hDUPFile);
	pData = (LPDUPDATA)&lpGWDHead->GWDData;
	CreateStatusWind(CurView->hWnd, 1, "Build dup file");
	while (StatusWindowUpdate(0, 0, nTot, nComplete++) && !BT_FIND(hHighlight, (LPSTR)&Refno, pos, BT_ANY, (LPSTR)&HighlightData))
	{
		pos = BT_NEXT;
		PickList[0] = HighlightData.PD;
		if (PickList[0].Type == 3)
		{
			pData->AREA = PickList[0].Area;
			pData->Refno = Refno;
			_fstrncpy(pData->UDI, PickList[0].UDI, 20);
			pData->NP = PickList[0].NumPoints;
			pData->MNX = PickList[0].Rect.xmn;
			pData->MXX = PickList[0].Rect.xmx;
			pData->MNY = PickList[0].Rect.ymn;
			pData->MXY = PickList[0].Rect.ymx;
			GWDAddRecord(lpGWDHead, 0, 0);
		}
	}
	DestroyStatusWindow(0);

	GlobalUnlock(hDUPFile);
	CloseGWDatabase(hDUPFile);


	BTVar[0].BT_VARLEN = 4;
	BTVar[0].BT_VARTYP = BT_INTEGER;
	BTVar[0].BT_VAROFF = 0;
	GSSiGetTempFileName(0, "btr", 0, (LPSTR)TempName);
	BT_CREATE(TempName, sizeof(dupLink), FALSE, 1, 1, BTVar, FALSE, 0, 0, FALSE);
	hBTDupLink = BT_OPEN(TempName, 0, BT_WRITE, 0);


	hDUPFile = OpenGWDatabase(DUPFile, BT_READ);
	lpGWDHead = (LPGWDHEADER)GlobalLock(hDUPFile);
	pData = (LPDUPDATA)&lpGWDHead->GWDData;
	memset(&lastDupData, 0, sizeof(DUPDATA));
	CreateStatusWind(CurView->hWnd, 1, "Build dup file");
	pos = BT_FIRST;
	nTot = BT_NUM_IN_INDEX(lpGWDHead->BTHandle[0]);
	nComplete = 0;
	while (StatusWindowUpdate(0, 0, nTot, nComplete++) && !BT_FIND(lpGWDHead->BTHandle[1], lpGWDHead->pKeys[1], pos, BT_ANY, (LPSTR)&offset))
	{
		pos = BT_NEXT;
		FillGWDData(lpGWDHead, offset);
		if (nDup >= 0 && !memcmp(&pData->NP, &lastDupData.NP, sizeof(DUPDATA) - 24))
		{
			rtn++;
			nDup++;
			dupLink.ref = baseRef;
			strncpy0(dupLink.dupUDI, baseUDI, 19);
			strncpy0(dupLink.UDI, pData->UDI, 19);
			dupLink.dupNum = nDup;
			BT_PUT(hBTDupLink, (LPSTR)&pData->Refno, (LPSTR)&dupLink);
		}
		else
		{
			if (baseRef && nDup > 0)
			{
				dupLink.ref = baseRef;
				dupLink.dupNum = -nDup;
				strncpy0(dupLink.UDI, baseUDI, 19);
				strncpy0(dupLink.dupUDI, baseUDI, 19);
				BT_PUT(hBTDupLink, (LPSTR)&baseRef, (LPSTR)&dupLink);
				rtn++;
			}
			baseRef = pData->Refno;
			strncpy0(baseUDI, pData->UDI, 19);
			nDup = 0;
		}
		lastDupData = *pData;
	}
	if (nDup && baseRef)
	{
		dupLink.ref = baseRef;
		strncpy0(dupLink.dupUDI, baseUDI, 19);
		strncpy0(dupLink.UDI, lastDupData.UDI, 19);
		dupLink.dupNum = -nDup;
		BT_PUT(hBTDupLink, (LPSTR)&baseRef, (LPSTR)&dupLink);
	}
	DestroyStatusWindow(0);
	nTot = BT_NUM_IN_INDEX(hBTDupLink);

	GlobalUnlock(hDUPFile);
	CloseGWDatabase(hDUPFile);
	GSSiRemove(DUPFile);
	if (CreateGWDDatabase(OUTFile, 1, FALSE, 0, 1, DefStr2))
	{
		HANDLE	hKeyFields;

		GetFieldIDsFromNames(OUTFile, &hKeyFields, 0, "UDI", 0);
		GWDAddIndex(OUTFile, hKeyFields, 1, 0);
		GSSiGlobFree(&hKeyFields);
		hOUTFile = OpenGWDatabase(OUTFile, BT_WRITE);
		lpGWDHead = (LPGWDHEADER)GlobalLock(hOUTFile);
		pOutData = (LPOUTDATA)&lpGWDHead->GWDData;
		pos = BT_FIRST;
		while (!BT_FIND(hBTDupLink, (LPSTR)&ref, pos, BT_ANY, (LPSTR)&dupLink))
		{
			pos = BT_NEXT;
			pOutData->Refno = ref;
			strncpy0(pOutData->UDI, dupLink.UDI, 19);
			strncpy0(pOutData->dupUDI, dupLink.dupUDI, 19);
			pOutData->DupRef = dupLink.ref;
			pOutData->DupNum = dupLink.dupNum;
			GWDAddRecord(lpGWDHead, 0, 0);
		}
		GlobalUnlock(hOUTFile);
		CloseGWDatabase(hOUTFile);
	}
	BT_CLOSEANDDELETE(&hBTDupLink);
	return rtn;
}