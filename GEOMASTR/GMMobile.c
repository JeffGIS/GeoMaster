#include "graphint.h"   
#include "gmextern.h"

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
	int nDup=0;
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
		if (!memcmp(&pData->NP, &lastDupData.NP, sizeof(DUPDATA) - 24))
		{
			rtn++;
			if (!nDup)
			{
				baseRef = lastDupData.Refno;
				strncpy(baseUDI, pData->UDI, 20);
			}
			nDup++;
			dupLink.ref = baseRef;
			strncpy(dupLink.dupUDI, baseUDI, 20);
			strncpy(dupLink.UDI, pData->UDI, 20);
			dupLink.dupNum = nDup;
			BT_PUT(hBTDupLink, (LPSTR)&pData->Refno, (LPSTR)&dupLink);
		}
		else
		{
			if (baseRef)
			{
				dupLink.ref = baseRef;
				dupLink.dupNum = -nDup;
				BT_PUT(hBTDupLink, (LPSTR)&baseRef, (LPSTR)&dupLink);
				rtn++;
			}
			baseRef = 0;
			nDup = 0;
		}
		lastDupData = *pData;
	}
	if (nDup && baseRef)
	{
		dupLink.ref = baseRef;
		strncpy(dupLink.dupUDI, baseUDI, 20);
		strncpy(dupLink.UDI, lastDupData.UDI, 20);
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
			strncpy(pOutData->UDI, dupLink.UDI, 20);
			strncpy(pOutData->dupUDI, dupLink.dupUDI, 20);
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