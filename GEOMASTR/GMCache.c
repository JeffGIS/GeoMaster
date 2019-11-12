#include "graphint.h"  
#include "umio.h"
#include "CDDEMO.h"
#include "extrndb.h"   
#include <sqlext.h>

#include "gmextern.h"

int APIENTRY WinMainGMCache(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{

	// TODO: Place code here.
	MSG msg = { 0 };
	/*
	HACCEL hAccelTable;
	SIZE size;
	LPSTR pFile = strstr(lpCmdLine, "/GMEdit ");
	LPSTR pEndFile = strchr(pFile, 0);
	LPSTR pWnd = strstr(lpCmdLine, "/W ");
	if (pWnd)
	{
		*pWnd = 0;
		pWnd += 3;
		hWndGMEditReturn = (HWND)atoi(pWnd);
		ExpandText(pWnd); //sets test env if included
	}
	if (pFile)
	{
		LPSTR pFS;

		pFile += 8;
		if (*pFile == '\'')
		{
			pFile++;
			pEndFile = strrchr(pFile, '\'');
			if (pEndFile)
				*pEndFile++ = 0;
		}
		else
			pEndFile = strchr(pFile, ' ');
		if (pEndFile && *pEndFile)
			*pEndFile++ = 0;
		SavePosition = atob(pEndFile);
		strcpy(fileToEdit, pFile);
		ExpandText(fileToEdit);
		if ((pFS = strchr(fileToEdit, '/')))
			*pFS++ = 0;
		Truncate(fileToEdit);
	}


	// Initialize global strings
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_GMEDIT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstanceGM(hInstance, nCmdShow))
	{
		return FALSE;
	}
	uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_GMEDIT));

	if (!IsDialogMessage(ghPrintingDlg, &msg)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			if (!IsDialogMessage(ghFindReplaceDlg, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	GSSiGlobFree(&hFile);
	if (hWndGMEditReturn)
	{
		//sprintf(str, "send %ld to %ld", GSSI_GMEDITCOMPLETE, (DWORD)hWndGMEditReturn);
		//MessageBox(0, str, 0, MB_OK);
		PostMessage(hWndGMEditReturn, GSSI_GMEDITCOMPLETE, 0, 0);
	}
	*/
	return (int)msg.wParam;
}

void NeedToStartBackgroundCache(void)
{
	char LastCacheStartDateFile[MAX_PATH];
	char LastDataUpdateFile[MAX_PATH];
	OFSTRUCTGM	OFStruct;
	char cDate[24];
	time_t currentTime;
	time_t lastDataUpdateTime = 0;
	time_t lastCacheCompleteTime = 0;

	time(&currentTime);
	sprintf(LastCacheStartDateFile,"%sLastCacheStartDate.txt", CachePathnameTo);
	ExpandText(LastCacheStartDateFile);
	strcpy (LastDataUpdateFile,"[%DL]lastdataupdate.txt");
	ExpandText(LastDataUpdateFile);

	HFILE	Fid = OpenFileGM(LastDataUpdateFile, &OFStruct, OF_READ);

	if (Fid != HFILE_ERROR)
	{
		int ln = min(22, _lread(Fid, cDate, 22));
		cDate[ln] = 0;

		lastDataUpdateTime = atol(cDate);
		_lclose(Fid);
	}
	Fid = OpenFileGM(LastCacheStartDateFile, &OFStruct, OF_READ);

	if (Fid != HFILE_ERROR)
	{
		int ln = min(22, _lread(Fid, cDate, 22));
		cDate[ln] = 0;

		lastCacheCompleteTime = atol(cDate);
		_lclose(Fid);
	}

	if (lastDataUpdateTime > lastCacheCompleteTime)
	{
		sprintf(cDate, "%lli", currentTime);
		Fid = OpenFileGM(LastCacheStartDateFile, &OFStruct, OF_CREATE);
		int ln = strlen(cDate);
		_lwrite(Fid, cDate, ln + 1);
		_lclose(Fid);
	}
	return;
}