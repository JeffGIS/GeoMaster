#include "graphint.h"  
#include "umio.h"
#include "CDDEMO.h"
#include "extrndb.h"   
#include <sqlext.h>

#include "gmextern.h"
#include <psapi.h>

static  int		nCalls = 0;
static  HWND	hWndGMCacheDialog = 0;
static	int		cacheDelayBetweenReads = 100;
static	time_t lastCacheStartTime = 0;

int CreateFilesToCacheFile(LPSTR cachFileList, HANDLE fidOut);
LONGLONG CacheFileInBackground(LPSTR FromFileIN, LPSTR CacheDir, LPSTR DataLocDir, LONGLONG StartPos);
void UpdateCacheMessage(LPSTR mess);

static TCHAR szTitle[] = "GMCache";					// The title bar text
static TCHAR szWindowClass[] = "GMCache";		// the main window class name
static HANDLE hTrustedCacheFiles = 0;
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
static void SetHoldMessage(HWND hWndDlg)
{
	char mess[256];
	static char lastmess[256] = "";
	if (hWndDlg)
	{
		EnableWindow(GetDlgItem(hWndDlg, IDC_RELEASEHOLD), GMCacheOnHoldUntil);
		if (GMCacheOnHoldUntil)
		{
			char onHoldUntil[128];
			sprintf(onHoldUntil, "$CAL(%i,7)", GMCacheOnHoldUntil);
			ExpandText(onHoldUntil);
			sprintf(mess, "Caching is on hold until %s\r\nYou may release the hold with the above button.", onHoldUntil);
		}
		else
		{
			strcpy(mess, "To speed up your computer you may place the caching on hold using one of the above buttons");
		}
		if (strcmp(mess, lastmess))
		{
			SetDlgItemText(hWndDlg, IDC_CACHEHOLDMESSAGE, mess);
			strcpy(lastmess, mess);
		}
	}
	else
		*lastmess = 0;
}
BOOL FAR PASCAL GMCacheMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
	char cmd[1024];
	int	BRtn;
	if ((BRtn = DIALOGSTYLEMsgProc(hWndDlg, Message, wParam, lParam)))
		return (BRtn);
	switch (Message)
	{
	case WM_INITDIALOG:
		hWndGMCacheDialog = hWndDlg;
		cwCenter(hWndDlg, 0);
		SetHoldMessage(0);
		SetHoldMessage(hWndDlg);
		UpdateCacheMessage("");
		break; /* End of WM_INITDIALOG                                 */

	case WM_CLOSE:
		/* Closing the Dialog behaves the same as Cancel               */
		PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
		break; /* End of WM_CLOSE                                      */

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			hWndGMCacheDialog = 0;
			EndDialog(hWndDlg, FALSE);
			break;
		case IDOK:
			EndDialog(hWndDlg, TRUE);
			break;
		case IDC_HOLD_ONE_HOUR:
			KillTimer(hWndMain, GMCACHE_TIMER);
			sprintf(cmd, "$INT([%%SYS_CLOCK]+3600)");
			ExpandText(cmd);
			GMCacheOnHoldUntil = atol(cmd);
			SetTimer(hWndMain, GMCACHE_TIMER, 60*60*1000, 0);
			SetHoldMessage(hWndDlg);
			break;
		case IDC_HOLD_UNTIL_5:
		{
			char cmd[] = "$CACHE(HOLD,17:00)";
			KillTimer(hWndMain, GMCACHE_TIMER);
			ProcessText(cmd);
			SetHoldMessage(hWndDlg);
		}
		break;
		case IDC_RELEASEHOLD:
			GMCacheOnHoldUntil = 0;
			SetTimer(hWndMain, GMCACHE_TIMER, 1000, 0);
			SetHoldMessage(hWndDlg);

			break;
		}
		break;    /* End of WM_COMMAND                                 */

	default:
		return FALSE;
	}
	return TRUE;
}

int removeFilesBeingCached(LPSTR cachedir)
{
	char tempFile[MAX_PATH];
	OFSTRUCTGM	OFStruct = { 0 };
	int nFiles = 0;
	char str[MAX_PATH*2];
	int rtn = 0;

	sprintf(tempFile, "%sremovbc.txt",cachedir);
	strcpy(str, cachedir);
	ExpandText(str);
	ExpandText(tempFile);
	HANDLE Fid2 = OpenFileGM(tempFile, &OFStruct, OF_CREATE);
	ii = SearchFilesInDirBC(str, 0, Fid2, &nFiles, "*.beingcached", 1, TRUE);
	llFileSeek(Fid2, 0, 0);
	while (fgetstring2(str, MAX_PATH, Fid2))
	{
		GSSiRemove(str);
		rtn++;
	}
	GSSiClose64(&Fid2);
	return rtn;
}

LRESULT CALLBACK WndProcGMCache(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	TEXTMETRIC	tm;
	SCROLLINFO si;
	SIZE size;
	int rowHeight;
	HDC hdc;
	int tablen;
	static HFILE fidFilesToCache = HFILE_ERROR;
	static char CachingPidFile[MAX_PATH] = { 0 };
	switch (message)
	{
	case WM_CREATE:
	{
		char FilesToCacheFile[MAX_PATH];
		OFSTRUCTGM	OFStruct = { 0 };
		char	BackgroundCacheFilelist[MAX_PATH];

		hWndMain = hWnd;
		SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);

		sprintf(FilesToCacheFile, "%sCACHE_IS_COMPLETE.tbr", CachePathnameTo);
		GSSiRemove(FilesToCacheFile);

		removeFilesBeingCached(CachePathnameTo);
		GetGlobalCVal("[%BackgroundCacheFilelist]", BackgroundCacheFilelist, "[%DL]BackgroundCacheFilelist.txt");
		sprintf(FilesToCacheFile, "%sFilesToCacheFile.txt", CachePathnameTo);
		ExpandText(FilesToCacheFile);
		sprintf(CachingPidFile, "%sCachingPid.txt", CachePathnameTo);
		ExpandText(CachingPidFile);

		HANDLE fidFilesToCache = OpenFileGM(FilesToCacheFile, &OFStruct, OF_CREATE);
		int nFiles = CreateFilesToCacheFile(BackgroundCacheFilelist, fidFilesToCache);
		GSSiClose64(&fidFilesToCache);
		if (!nFiles)
		{
			sprintf(FilesToCacheFile, "%sCACHE_IS_COMPLETE.tbr", CachePathnameTo);
			HANDLE fid = OpenFileGM(FilesToCacheFile, &OFStruct, OF_CREATE);
			BigWrite64(fid, "NoFiles", 6,-1);
			GSSiClose64(&fid);

			DestroyWindow(hWnd);
		}
		else
		{
			DWORD	Pid = _getpid();
			char cPid[32];
			char LastDataUpdateFile[MAX_PATH];
			sprintf(cPid,"%lli", (LONGLONG)Pid);
			HANDLE Fid = OpenFileGM(CachingPidFile, &OFStruct, OF_CREATE);
			BigWrite64(Fid, (LPSTR)cPid, strlen(cPid),-1);
			GSSiClose64(&Fid);

			strcpy(LastDataUpdateFile, "[%DL]lastdataupdate.txt");
			ExpandText(LastDataUpdateFile);

			Fid = OpenFileGM(LastDataUpdateFile, &OFStruct, OF_READ);
			SetTimer(hWnd, GMCACHE_TIMER, 20, 0);

			if (Fid != INVALID_HANDLE_VALUE)
			{
				char cmd[MAX_PATH + 2];
				while (fgetstring2(cmd, 254, Fid))
				{
					ExpandText(cmd);
				}
				GSSiClose64(&Fid);
			}

		}
	}
		return 0;
	case WM_DESTROY:
		GSSiRemove32(CachingPidFile);
		PostQuitMessage(0);
		break;

	case WM_CLOSE:
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_TIMER:
		if (wParam == GMCACHE_TIMER)
		{
			GMCacheOnHoldUntil = 0;
			SetHoldMessage(hWndGMCacheDialog);
			if (!StartCachingFiles())
			{
				KillTimer(hWnd, wParam);
				PostQuitMessage(0);
			}
			else
			{
				KillTimer(hWnd, wParam);
				SetTimer(hWndMain, GMCACHE_TIMER, cacheReadDelayInMicrosecs, 0);
			}
			return 0;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_SHOWWINDOW:
		if (wParam)
		{
			DialogBox(hInst, (LPCTSTR)"GMCACHE_DIALOG", hWnd, (DLGPROC)GMCacheMsgProc);
		}
		else
		{
			if (hWndGMCacheDialog)
				PostMessage (hWndGMCacheDialog, WM_COMMAND, IDCANCEL, 0L);
		}
		break;
	case WM_SIZE:
		switch (wParam)
		{
		case SIZE_MINIMIZED:
			if (hWndGMCacheDialog)
				PostMessage(hWndGMCacheDialog, WM_COMMAND, IDCANCEL, 0L);
			break;
		case SIZE_MAXIMIZED:
			if (!hWndGMCacheDialog)
			{
				DialogBox(hInst, (LPCTSTR)"GMCACHE_DIALOG", hWnd, (DLGPROC)GMCacheMsgProc);
				ShowWindow(hWnd, SW_MINIMIZE);
			}
			break;
		case SIZE_RESTORED:
			ShowWindow(hWnd, SW_MAXIMIZE);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	case WM_SETFOCUS:
		break;
	case WM_KILLFOCUS:
		break;

	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;

}
BOOL InitInstanceGMCache(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_DLGFRAME,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

static ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProcGMCache;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, "GMCache");
	wcex.hCursor = LoadCursor(NULL, IDC_IBEAM);
	wcex.hbrBackground = (HBRUSH)(COLOR_BTNSHADOW + 1);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_GMEDIT_SMALL));

	return RegisterClassEx(&wcex);
}
/*ATOM MyRegisterClass_test(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MONITORTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MONITORTEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}*/
//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 0,0, hInstance,0);

   if (!hWnd)
   {
      return FALSE;
   }
   hWndMain = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

int APIENTRY WinMainGMCache(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{

	// TODO: Place code here.
	MSG msg = { 0 };
	
	HACCEL hAccelTable;
	SIZE size;
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstanceGMCache(hInstance, nCmdShow))
	{
		return FALSE;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (hWndGMCacheDialog && IsDialogMessage(hWndGMCacheDialog, &msg))
			ii = 1;
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

BOOL StartGMCache(void)
{
	BOOL rtn = FALSE;
	char cmd[] = "$SESSION(CREATE,GeoMaster /GMCache)";

	ExpandText(cmd);
	rtn = atob(cmd);
	return rtn;
}

BOOL GMCacheIsRunning(void)
{
	char cachingPidFile[MAX_PATH];
	BOOL rtn = FALSE;
	sprintf(cachingPidFile, "%sCachingPid.txt", CachePathnameTo);
	ExpandText(cachingPidFile);
	rtn = AnotherProcessIsCaching(cachingPidFile);

	return rtn;
}
void CacheFilesInList(LPSTR supplementalList, LPSTR CacheDir)
{
	HANDLE Fid = 0;
	OFSTRUCTGM OFStruct = { 0 };
	char fromFile[MAX_PATH+2], toFile[MAX_PATH];
	char dataDir[MAX_PATH] = "[%DL]";

	ExpandText(dataDir);
	int ldataDir = strlen(dataDir);
	Fid = OpenFileGM(supplementalList, &OFStruct, OF_READ);
	while (fgetstring2(fromFile, MAX_PATH, Fid))
	{
		if (!strnicmp(fromFile, dataDir, ldataDir))
		{
			sprintf(toFile, "%s%s", CacheDir, &fromFile[ldataDir]);
			ExpandText(fromFile);
			ExpandText(toFile);
			makedirectories(toFile, FALSE, FALSE);
			CopyFile(fromFile, toFile, FALSE);
		}
	}
	GSSiClose64(&Fid);

	return;
}
BOOL LoadSupplementalCacheFiles(LPSTR SupplementalCacheDir,LPSTR CacheDir)
{
	char value[128];
	char supplementalList[MAX_PATH+2];
	char tempFile[MAX_PATH+2];
	HANDLE Fid = 0;
	OFSTRUCTGM OFStruct = { 0 };
	BOOL rtn = FALSE;
	int nFiles = 0;
	int lastProcessed;
	int maxID = 0;

	GSSiGetTempFileName(0, "gm", 0, tempFile);
	GetPrivateProfileString("User", "LastSupplementalCache", "0", value, sizeof(value), GMIni);
	lastProcessed = atoi(value);
	maxID = lastProcessed;
	Fid = OpenFileGM(tempFile, &OFStruct, OF_CREATE);
	ii = SearchFilesInDirBC(SupplementalCacheDir, 0, Fid, &nFiles, "*.txt", 1, FALSE);
	if (nFiles > 0)
	{
		llFileSeek(Fid, 0, 0);
		while (fgetstring2(supplementalList, MAX_PATH, Fid))
		{
			LPSTR pID = strrchr(supplementalList, '\\');
			if (pID)
			{
				int dirID;
				dirID = atoi(++pID);
				if (dirID > lastProcessed)
				{
					maxID = max(dirID, maxID);
					CacheFilesInList(supplementalList,CacheDir);
				}
			}
		}
		if (maxID > lastProcessed)
		{
			itoa(maxID, value, 10);
			WritePrivateProfileString("User", "LastSupplementalCache", value, GMIni);
		}
	}
	GSSiClose64(&Fid);
	OpenFileGM(tempFile, &OFStruct, OF_DELETE);

	return rtn;
}
void NeedToStartBackgroundCache(void)
{
	char	BackgroundCacheFilelist[MAX_PATH];
	OFSTRUCTGM	OFStruct = { 0 };

	GetGlobalCVal("[%BackgroundCacheFilelist]", BackgroundCacheFilelist, "[%DL]BackgroundCacheFilelist.txt");
	if (!*BackgroundCacheFilelist || !AllowCache || !ExistFile(BackgroundCacheFilelist))
		return;

	char FilesToCacheFile[MAX_PATH];
	char LastCacheStartTimeFile[MAX_PATH];
	char LastCacheCompleteTimeFile[MAX_PATH];
	char LastDataUpdateFile[MAX_PATH];
	char TrustedCacheFiles[MAX_PATH];
	char CacheIsCompleteFile[MAX_PATH];
	char SupplementalCacheDir[MAX_PATH] = "[%DL]SupplementalCacheFiles";
	char cDate[24];
	time_t currentTime;
	time_t lastDataUpdateTime = 0;
	time_t lastCacheCompleteTime = 0;

	ExpandText(SupplementalCacheDir);
	LoadSupplementalCacheFiles(SupplementalCacheDir, CachePathnameTo);
	time(&currentTime);
	sprintf(LastCacheStartTimeFile, "%sLastCacheStartTime.txt", CachePathnameTo);
	ExpandText(LastCacheStartTimeFile);
	sprintf(LastCacheCompleteTimeFile, "%sLastCacheCompleteTime.txt", CachePathnameTo);
	ExpandText(LastCacheCompleteTimeFile);
	sprintf(FilesToCacheFile, "%sFilesToCacheFile.txt", CachePathnameTo);
	ExpandText(FilesToCacheFile);
	sprintf(TrustedCacheFiles, "%sTrustedCacheFiles.txt", CachePathnameTo);
	ExpandText(TrustedCacheFiles);
	strcpy(LastDataUpdateFile, "[%DL]lastdataupdate.txt");
	ExpandText(LastDataUpdateFile);
	sprintf(CacheIsCompleteFile, "%sCACHE_IS_COMPLETE.tbr", CachePathnameTo);
	ExpandText(CacheIsCompleteFile);

	HANDLE	Fid = OpenFileGM(LastDataUpdateFile, &OFStruct, OF_READ);

	if (Fid != INVALID_HANDLE_VALUE)
	{
		char cmd[MAX_PATH+2];
		fgetstring2(cmd, MAX_PATH, Fid);
		lastDataUpdateTime = atol(cmd);
		while (fgetstring2(cmd, 254, Fid))
		{
			ExpandText(cmd);
		}
		GSSiClose64(&Fid);
	}
	Fid = OpenFileGM(LastCacheStartTimeFile, &OFStruct, OF_READ);

	if (Fid != INVALID_HANDLE_VALUE)
	{
		int ln = BigRead64(Fid, cDate, 22);
		ln = min(22, ln);
		cDate[ln] = 0;

		lastCacheStartTime = atol(cDate);
		GSSiClose64(&Fid);
	}
	Fid = OpenFileGM(LastCacheCompleteTimeFile, &OFStruct, OF_READ);

	if (Fid != INVALID_HANDLE_VALUE)
	{
		int ln = BigRead64(Fid, cDate, 22);
		ln = min(22, ln);
		cDate[ln] = 0;

		lastCacheCompleteTime = atol(cDate);
		GSSiClose64(&Fid);
	}
//	lastCacheCompleteTime = 0;
	if (lastDataUpdateTime > lastCacheCompleteTime)
	{
		if (!ExistFile(CacheIsCompleteFile))
		{
			if (!GMCacheIsRunning())
			{
				char DataLocDir[MAX_PATH];
				char CacheDir[MAX_PATH];

				strcpy(DataLocDir, "[%DL]");
				ExpandText(DataLocDir);
				strcpy(CacheDir, CachePathnameTo);
				ExpandText(CacheDir);
				sprintf(cDate, "%lli", currentTime);
				Fid = OpenFileGM(LastCacheStartTimeFile, &OFStruct, OF_CREATE);
				int ln = strlen(cDate);
				BigWrite64(Fid, cDate, ln + 1,-1);
				GSSiClose64(&Fid);
				GSSiRemove(TrustedCacheFiles);
				GSSiGlobFree(&hTrustedCacheFiles);
				StartGMCache();
			}
		}
		if (!GMCacheTimer)
			GMCacheTimer = SetTimer(hWndMain, CACHE_FILE_RENAME_TIMER, 15000, 0);
	}
	return;
}

void UpdateLastDataUpdate(LPSTR holdUntil,int bufferSize)
{
	char LastDataUpdateFile[MAX_PATH];
	time_t currentTime;
	OFSTRUCTGM	OFStruct = { 0 };
	char cDate[64];

	time(&currentTime);

	strcpy(LastDataUpdateFile, "[%DL]lastdataupdate.txt");
	ExpandText(LastDataUpdateFile);
	sprintf(cDate, "%lli", currentTime);
	HANDLE Fid = OpenFileGM(LastDataUpdateFile, &OFStruct, OF_CREATE);
	int ln = strlen(cDate);
	BigWrite64(Fid, cDate, ln + 1,-1);
	if (*holdUntil)
	{
		BigWrite64(Fid, "\r\n", 2,-1);
		sprintf(cDate, "$CACHE(HOLD,%s)", holdUntil);
		BigWrite64(Fid, cDate, strlen(cDate),-1);
	}
	if (bufferSize)
	{
		BigWrite64(Fid, "\r\n", 2,-1);
		sprintf(cDate, "[%%CACHEBUFFERSIZE]=%i", bufferSize);
		BigWrite64(Fid, cDate, strlen(cDate),-1);
	}
	GSSiClose64(&Fid);
}

void FreeTrustedFiles(void)
{
	GSSiGlobFree(&hTrustedCacheFiles);
	nCalls++;
}

void UpdateCacheMessage(LPSTR mess)
{
	static char lastmess[512] = "";

	if (strcmp(mess, lastmess))
	{
		if (hWndGMCacheDialog)
		{
			SetDlgItemText(hWndGMCacheDialog, IDC_CACHEMESSAGE, mess);
		}
		else if (hWndMain)
			SetWindowText(hWndMain, mess);
		strcpy(lastmess, mess);
	}
}

BOOL StartCachingFiles(void)
{
	static char DataLocDir[MAX_PATH];
	static char CacheDir[MAX_PATH] = { 0 };
	static char FilesToCacheFile[MAX_PATH] = { 0 };
	OFSTRUCTGM	OFStruct = { 0 };
	double PctDone = 0;
	static LONGLONG StartPos = 0;
	char FromFile[MAX_PATH + 2];
	char mess[MAX_PATH*2];
	static int totLen;
	static int loc = 0;
#define NEACH_LOOP	10
	int nProcessed = NEACH_LOOP;

	HANDLE fid = INVALID_HANDLE_VALUE;
	static first = TRUE;
	BOOL rtn = TRUE;
	BOOL completed = FALSE;

	if (first)
	{
		first = FALSE;
		strcpy(DataLocDir, "[%DL]");
		ExpandText(DataLocDir);
		strcpy(CacheDir, CachePathnameTo);
		ExpandText(CacheDir);
		sprintf(FilesToCacheFile, "%sFilesToCacheFile.txt", CacheDir);
		ExpandText(FilesToCacheFile);

		fid = OpenFileGM(FilesToCacheFile, &OFStruct, OF_READ);
		totLen = llFileSeek(fid, 0, 2);
		llFileSeek(fid, 0, 0);
	}
	else
	{
		fid = OpenFileGM(FilesToCacheFile, &OFStruct, OF_READ);
		llFileSeek(fid, loc, 0);
	}
	PctDone = 100.0 * (double)loc / (double)totLen;
	if (fgetstring2(FromFile, MAX_PATH, fid))
	{
		int nDir = 3;
		LPSTR	pName = strrchr(FromFile, '\\');
		LPSTR	pNameLast = FromFile;
		while (pName && nDir--)
		{
			pNameLast = pName;
			*pName = 0;
			pName = strrchr(FromFile, '\\');
			*pNameLast = '\\';
		}
		if (pName)
			pName++;
		else
			pName = FromFile;

		sprintf(mess, "%s\r\n(%.2f %% complete)", pName, PctDone);
		UpdateCacheMessage(mess);
		StartPos = CacheFileInBackground(FromFile, CacheDir, DataLocDir, StartPos);
		if (!StartPos)
		{
			loc = llFileSeek(fid, 0, 1);

			PctDone = 100.0 * (double)loc / (double)totLen;
			sprintf(mess, "%s\r\n(%.2f %% complete)", pName, PctDone);
			UpdateCacheMessage(mess);
		}
	}
	else
		completed = TRUE;
	GSSiClose64(&fid);
	if (completed)
	{
		sprintf(FromFile, "%sCACHE_IS_COMPLETE.tbr", CacheDir);
		fid = OpenFileGM(FromFile, &OFStruct, OF_CREATE);
		BigWrite64(fid, "Done", 4,-1);
		GSSiClose64(&fid);
		rtn = FALSE;
	}
	return rtn;
}
int CreateFilesToCacheFile(LPSTR cachFileList,HANDLE fidOut)
{
	FILE* FidFilelist;
	//char    File[MAX_PATH];
	//char	CacheDir[MAX_PATH];
	char	DataLocDir[MAX_PATH];
	//char	CachingPidFile[MAX_PATH];
	//char	cachFileList[MAX_PATH];
	char	tempFile[MAX_PATH];
	char	str[MAX_PATH + 2];
	HANDLE	Fid, Fid2;
	OFSTRUCTGM	OFStruct = { 0 };
	int totFiles = 0;
	BOOL	skip = FALSE;

	strcpy(DataLocDir, "[%DL]");
	ExpandText(DataLocDir);
	FidFilelist = fopen(cachFileList, "rt");
	if (FidFilelist != NULL)
	{
		GSSiGetTempFileName(0, "gm", 0, tempFile);
		while (fgetss(str, MAX_PATH, FidFilelist))
		{
			if (!strnicmp(str, "IF(", 3))
			{
				*LastChr(str) = 0;
				BOOL irc;

				BOOL val = LogicP(&str[3], &irc);
				if (!val)
				{
					skip = TRUE;
				}
				continue;
			}
			else if (!stricmp(str, "ENDIF"))
			{
				skip = FALSE;
			}
			else if (skip)
				continue;
			REPLAC(str, "[%DL]", DataLocDir, MAX_PATH);
			ExpandText(str);
			if (strchr(str, '*'))
			{
				int		nFiles = 0;
				LPSTR	pWild = strrchr(str, '\\');
				BOOL	wantSub = FALSE;

				if (pWild)
				{
					if (*(pWild - 1) == '\\')
					{
						wantSub = TRUE;
						*(pWild - 1) = 0;
					}
					*pWild++ = 0;
					Fid2 = OpenFileGM(tempFile, &OFStruct, OF_CREATE);
					ii = SearchFilesInDirBC(str, 0, Fid2, &nFiles, pWild, 1, wantSub);
					llFileSeek(Fid2, 0, 0);
					while (fgetstring2(str, MAX_PATH, Fid2))
					{
							REPLAC(str, "[%DL]", DataLocDir, MAX_PATH);
							fputstring2(str, fidOut);
							totFiles++;
					}
					GSSiClose64(&Fid2);
					OpenFileGM(tempFile, &OFStruct, OF_DELETE);
				}
			}
			else
			{
				totFiles++;
				fputstring2(str, fidOut);
			}
		}
 		fclose(FidFilelist);
	}
	return totFiles;

}

static BOOL Execute(LPSTR cmd, sqlite3* database)
{
	BOOL rtn = !SQLOK(sqlite3_exec(database, cmd, 0, 0, 0), database, "", 0);

	return rtn;
}

BOOL SortAndReduceCachedFiles (LPSTR List)
{
#define LINELEN	USHRT_MAX
	sqlite3* database = NULL;
	BOOL rtn = FALSE;
	int rc, iseq=0;
	int nTot = 0, nDone = 0, totErrors = 0;
	LPSTR line = malloc(LINELEN);
	char tempFile[MAX_PATH];
	HFILE fidOut;

	GSSiGetTempFileName(0, "slt", 0, tempFile);

	LPSTR file = malloc(1024);
	rc = sqlite3_open(tempFile, &database);
	if (rc == SQLITE_OK)
	{
		HFILE FidList = GSSiOpenFile(List, 0, OF_READ);
		if (FidList != HFILE_ERROR)
		{
			BOOL first = TRUE;
			Execute("BEGIN", database);

			sprintf(line, "CREATE TABLE SORTEDFILES (FILEPATH CHAR(256) PRIMARY KEY,OrigSequence INT);");
			if (Execute(line, database))
			{
				while (fgetstring(file, 258, FidList))
				{
					sprintf(line, "INSERT OR REPLACE INTO SORTEDFILES VALUES('%s',%i);", file, iseq++);
					Execute(line, database);
				}
			}
			Execute("COMMIT", database);
			GSSiClose2(&FidList);

			fidOut = GSSiOpenFile("c:\\temp\\sortedfiles.txt", 0, OF_CREATE);
			sprintf(line, "SELECT FILEPATH FROM SORTEDFILES ORDER BY OrigSequence;");
			sqlite3_stmt* statement = NULL;
			if (sqlite3_prepare_v2(database, line, -1, &statement, 0) == SQLITE_OK)
			{
				while (sqlite3_step(statement) == SQLITE_ROW)
				{
					LPSTR filePath = (LPSTR)sqlite3_column_text(statement, 0);
					fputstring(filePath, fidOut);
				}
			}
			sqlite3_finalize(statement);
			GSSiClose2(&fidOut);
		}
		rc = sqlite3_close(database);
	}
	GSSiRemove(tempFile);
	free(line);
	free(file);
	return rtn;
}

BOOL UseTrustedCacheFile(LPSTR FileName)
{
	static int logUncachedFiles = 0;
	static FILE* uncachedFileFile = 0;
	BOOL rtn = FALSE;
	LPSTR pTrustedFiles;

	if (!FileName)
	{
		if (logUncachedFiles == 1)
		{
			fclose(uncachedFileFile);
			SortAndReduceCachedFiles("c:\\temp\\uncachedfiles.txt");
		}
		return TRUE;
	}
	if (!logUncachedFiles)
	{
		if (GetGlobalBVal2("[%LogUncachedFiles]", FALSE))
		{
			logUncachedFiles = 1;
			uncachedFileFile = fopen("c:\\temp\\uncachedfiles.txt", "w");
		}
		else
			logUncachedFiles = 2;
	}
	if (!hTrustedCacheFiles)
	{
		char TrustedCacheFiles[MAX_PATH];
		OFSTRUCTGM OFStruct = { 0 };
		sprintf(TrustedCacheFiles, "%sTrustedCacheFiles.txt", CachePathnameTo);
		ExpandText(TrustedCacheFiles);
		HANDLE fid = OpenFileGM(TrustedCacheFiles, &OFStruct, OF_READ);
		if (fid != INVALID_HANDLE_VALUE)
		{
			int ln = llFileSeek(fid, 0, 2);
			llFileSeek(fid, 0, 0);
			hTrustedCacheFiles = GSSiGlobAlloc(0,GMEM_MOVEABLE,ln+2);
			LPSTR pTrustedFiles = GlobalLock(hTrustedCacheFiles);
			BigRead64(fid, pTrustedFiles, ln);
			pTrustedFiles[ln] = 0;
			GlobalUnlock(hTrustedCacheFiles);
			GSSiClose64(&fid);
		}
		else
			hTrustedCacheFiles = GSSiGlobAlloc(0, GHND, 4);
	}
	pTrustedFiles = GlobalLock(hTrustedCacheFiles);
	char searchFile[MAX_PATH + 2];

	sprintf(searchFile, "*%s\r\n", FileName);
	strupr(searchFile);
	if (strstr(pTrustedFiles, searchFile))
		rtn = TRUE;
	else if (logUncachedFiles == 1)
	{
		char str[MAX_PATH + 2];
		LPSTR pFile = searchFile;
		if (*pFile == '*')
		{
			pFile++;
			sprintf(str, "[%%DL]%s", pFile);
		}
		else
			strcpy(str, pFile);
		LPSTR pEnd = strrchr(str, '\n');
		if (pEnd)
			*pEnd = 0;
		//ExpandText(str);
		fwrite(str, strlen(str),1, uncachedFileFile);
	}
	GlobalUnlock(hTrustedCacheFiles);
	nCalls++;
	return rtn;
}

#define MAX_FILES_TO_RENAME 4096
void RenameCachedFiles(void)
{
	LPSTR CacheDir = CachePathnameTo;
	char	CacheListDir[MAX_PATH];
	char	TempName[MAX_PATH];
	char	FileName[MAX_PATH];
	char	FromName[MAX_PATH+2];
	char	TrustedCacheFiles[MAX_PATH];
	char	CacheRenameFile[MAX_PATH + 2];
	char	SearchString[32] = "*.tb*";
	HFILE	Fid, Fid2;
	long	TotFiles = 0;
	LPSTR	pDot;
	HANDLE	FidCachedFiles = INVALID_HANDLE_VALUE;
	OFSTRUCTGM	OFStruct = { 0 };
	char dir[MAX_PATH];
	strcpy(dir, CacheDir);
	ExpandText(dir);
	*LastChr(dir) = 0;

	if (isGMCache)
		return;
	if (!*CacheDir)
		return;
	GSSiGetTempFileName(0, "gm", 0, TempName);
	Fid = GSSiOpenFile(TempName, 0, OF_CREATE);
	SearchFilesInDir2(dir, "", Fid, &TotFiles, SearchString, 1, TRUE,TRUE, MAX_FILES_TO_RENAME);
	if (TotFiles)
	{
		char	CacheDirectory[MAX_PATH + 2];
		strcpy(CacheDirectory, CacheDir);
		ExpandText(CacheDirectory);
		strupr(CacheDirectory);
		int lnCacheDirectory = strlen(CacheDirectory);
		sprintf(TrustedCacheFiles, "%sTrustedCacheFiles.txt", CacheDirectory);
		FidCachedFiles = OpenFileGM(TrustedCacheFiles, &OFStruct, OF_WRITE);
		if (FidCachedFiles == INVALID_HANDLE_VALUE)
		{
			FidCachedFiles = OpenFileGM(TrustedCacheFiles, &OFStruct, OF_CREATE);
			fputstring2("", FidCachedFiles);
		}
		else
			llFileSeek(FidCachedFiles, 0, 2);
		CloseAllRequestedFiles(FALSE);
		GSSillseek(Fid, 0, 0);
		while (fgetstring(FromName, MAX_PATH, Fid))
		{
			strcpy(FileName, FromName);
			LPSTR pDot = strrchr(FileName, '.');

			if (pDot)
			{
				if (!stricmp (pDot,".tbr"))
				{
					LPSTR pBS = strrchr(FileName, '\\');
					*pDot = 0;
					if (pBS && !stricmp(pBS + 1, "CACHE_IS_COMPLETE"))
					{
						if (TotFiles == 1)
						{
							time_t currentTime;
							char cDate[24];

							char LastCacheCompleteTimeFile[MAX_PATH];
							GSSiRemove(FromName);
							time(&currentTime);
							sprintf(cDate, "%lli", lastCacheStartTime);
							sprintf(LastCacheCompleteTimeFile, "%sLastCacheCompleteTime.txt", CachePathnameTo);
							ExpandText(LastCacheCompleteTimeFile);
							HANDLE Fid = OpenFileGM(LastCacheCompleteTimeFile, &OFStruct, OF_CREATE);
							int ln = strlen(cDate);
							BigWrite64(Fid, cDate, ln + 1,-1);
							GSSiClose64(&Fid);
							KillTimer(hWndMain, GMCacheTimer);
						}
						continue;
					}
					LPSTR pDollar = strrchr(FileName, '$');
					LPSTR pIsGood;
					if (pDollar)
					{
						*pDollar = '.';
						GSSiRemove(FileName);
						GSSiRename(FromName, FileName);
					}
					else if ((pIsGood = strrchr(FileName, '#')))
					{
						GSSiRemove(FromName);
						*pIsGood = '.';
					}
					else
					{
						GSSiRemove(FileName);
						GSSiRename(FromName, FileName);
					}
					strupr(FileName);
					LPSTR pName = FileName;
					if (!strncmp(FileName, CacheDirectory, lnCacheDirectory))
					{
						FileName[lnCacheDirectory - 1] = '*';
						pName = &FileName[lnCacheDirectory - 1];
					}
					fputstring2(pName, FidCachedFiles);
				}
				else if (!stricmp(pDot, ".tbd"))
				{
					GSSiRemove(FileName);
					strupr(FileName);
					REPLAC(FileName, ".TBD", "", MAX_PATH);
					REPLAC(FileName, "$", ".", MAX_PATH);
					LPSTR pName = FileName;
					if (!strncmp(FileName, CacheDirectory, lnCacheDirectory))
					{
						FileName[lnCacheDirectory - 1] = '*';
						pName = &FileName[lnCacheDirectory - 1];
					}
					fputstring2(pName, FidCachedFiles);
				}
			}
		}
		GSSiGlobFree(&hTrustedCacheFiles);
	}
	if (FidCachedFiles != INVALID_HANDLE_VALUE)
		GSSiClose64 (&FidCachedFiles);
	GSSiClose (Fid);
	GSSiRemove(TempName);
	return;
}

void __cdecl BackgroundCache2(LPSTR* Args)
{
	char    File[MAX_PATH];
	char	CacheDir[MAX_PATH];
	HFILE	FidFilelist;
	HFILE	Fid;

	strcpy(CacheDir, Args[2]);
	Sleep(20000);
	MessageBox(0, Args[0], "Starting Caching", MB_OK);
	FidFilelist = GSSiOpenFile(Args[1], 0, OF_READ);
	GSSiClose2(&FidFilelist);
	return;
}

LONGLONG CacheFileInBackground(LPSTR FromFileIN, LPSTR CacheDir, LPSTR DataLocDir, LONGLONG StartPos)
{
	int	dtime;
	BY_HANDLE_FILE_INFORMATION fifrom, fito;
	HANDLE	FidFrom, FidTo;
	int		lDL = 5;
	OFSTRUCTGM	OFStruct = { 0 };
	char	FromFile[MAX_PATH];
	char	ToFile[MAX_PATH];
	char	ToFileIntermediate[MAX_PATH];
	int		st;
	static  LONGLONG	FromSize = 0;
	LONGLONG ToSize, TotRead, pos=0;
	UINT	OFMode = OF_CREATE;
	DWORD	lRead;
	LPSTR	pDot;
	char	CacheFromName[MAX_PATH];
	char	altDir[MAX_PATH];
	int		icpf, l;
	char mess[256];
	BOOL	toIsRenameFile = FALSE;

	GMCacheOnHoldUntil = 0;
	for (icpf = 0; icpf < NumCachePathnameFrom; icpf++)
	{
		strcpy(CacheFromName, CachePathnameFrom[icpf]);
		if (!stricmp(CacheFromName, "[%DL]"))
			strcpy(CacheFromName, DataLocDir);
		l = _fstrlen(CacheFromName);
		if (l)
		{
			if (!strnicmp(FromFileIN, CacheFromName, l))
				goto Next;
		}
	}
	return 0;

Next:
	if (icpf)
	{
		strcpy(altDir, CacheFromName);
		REPLAC(altDir, ":\\", "_", MAX_PATH);
	}
	else
		*altDir = 0;
	sprintf(FromFile, "%s%s", CacheFromName, &FromFileIN[l]);
	sprintf(ToFile, "%s%s%s", CacheDir, altDir, &FromFileIN[l]);
	strcpy(ToFileIntermediate, ToFile);
	if ((pDot = strrchr(ToFileIntermediate, '.')))
		*pDot = '$';
	FidFrom = OpenFileEX(FromFile, &OFStruct, OF_READ, 0);
	if (StartPos > 0)
	{
		strcat(ToFileIntermediate, ".beingcached");
		strcpy(ToFile, ToFileIntermediate);
		OFMode = OF_READWRITE;
		dtime = 1;
	}
	else
	{
		strcat(ToFileIntermediate, ".tbr");
		FidTo = OpenFileGM(ToFileIntermediate, &OFStruct, OF_READ);
		if (FidTo == INVALID_HANDLE_VALUE)
			FidTo = OpenFileGM(ToFile, &OFStruct, OF_READ);
		else
		{
			toIsRenameFile = TRUE;
			GSSiRemove(ToFile);
		}
		if (FidFrom == INVALID_HANDLE_VALUE)
		{
			if (FidTo != INVALID_HANDLE_VALUE)
			{
				GSSiClose64(&FidTo);
				GSSiRemove(ToFile);
				GSSiRemove(ToFileIntermediate);
			}
			REPLAC(ToFileIntermediate, ".tbr", ".tbd", MAX_PATH);
			FidTo = OpenFileGM(ToFileIntermediate, &OFStruct, OF_CREATE);
			BigWrite64(FidTo, "Does not exist", 6, -1);
			GSSiClose64(&FidTo);
			return 0;
		}
		strcpy(ToFile, ToFileIntermediate);
		st = GetFileInformationByHandle((HANDLE)FidFrom, &fifrom);
		FromSize = (LONGLONG)fifrom.nFileSizeLow + ULONG_MAX * (LONGLONG)fifrom.nFileSizeHigh;
		if (FidTo == INVALID_HANDLE_VALUE)
			dtime = 1;
		else
		{
			st = GetFileInformationByHandle((HANDLE)FidTo, &fito);
			ToSize = (LONGLONG)fito.nFileSizeLow + ULONG_MAX * (LONGLONG)fito.nFileSizeHigh;
			if (StartPos > 0)
			{
				dtime = 1;
				if (StartPos != ToSize)
					StartPos = 0;
				else
					OFMode = OF_READWRITE;
			}
			else if (FromSize != ToSize)
				dtime = 1;
			else
				dtime = CompareFileTime(&fifrom.ftLastWriteTime, &fito.ftLastWriteTime);
			GSSiClose64(&FidTo);
		}
	}
	if (dtime > 0)
	{
		BOOL OkToCache = TRUE;
		if (StartPos == 0)
		{
			OkToCache = makedirectories(ToFile, FALSE, FALSE);

			if (OkToCache)
			{
				LONGLONG FreeSpace = GetDriveFreeSpace(ToFile);

				FreeSpace -= FromSize;
				if (FreeSpace < MinCacheDriveFreeSpace)
					OkToCache = FALSE;
			}
		}
		if (OkToCache)
		{
			LPSTR Cachebuf = malloc(lCachebuf + 4);
			LPSTR pDot = strrchr(ToFile, '.');
			if (!stricmp(pDot, ".tbr"))
				strcpy(pDot, ".beingcached");
			FidTo = OpenFileGM(ToFile, &OFStruct, OFMode);
			if (FidTo != INVALID_HANDLE_VALUE)
			{
				llFileSeek((HANDLE)FidTo, StartPos, 0);
				llFileSeek((HANDLE)FidFrom, StartPos, 0);
				TotRead = StartPos;
				lRead = BigRead64((HANDLE)FidFrom, Cachebuf, lCachebuf);
				if (lRead > 0)
				{
					pos = llFileSeek((HANDLE)FidFrom, 0, 1);
					if (lRead != BigWrite64(FidTo, Cachebuf, lRead,-1))
					{
						GSSiClose64(&FidTo);
						OpenFileGM(ToFile, &OFStruct, OF_DELETE);
					}
					else
						TotRead += lRead;
					//SleepEx(cacheDelayBetweenReads, 0);
				}
				if (TotRead == FromSize)
					pos = -2;
			}
			free(Cachebuf);
		}
	}
	else if (!toIsRenameFile)
	{
		LPSTR pDollar = strrchr(ToFileIntermediate, '$');
		if (pDollar)
		{
			OpenFileGM(ToFileIntermediate, &OFStruct, OF_DELETE);
			*pDollar = '#';
			HANDLE FidIsGood = OpenFileGM(ToFileIntermediate, &OFStruct, OF_CREATE);
			BigWrite64(FidIsGood, "IsGood", 6,-1);
			GSSiClose64(&FidIsGood);
		}
	}
	else
	{
		LPSTR pDollar = strrchr(ToFileIntermediate, '$');
		if (pDollar)
		{
			*pDollar = '#';
			GSSiRemove(ToFileIntermediate);
		}
	}
	GSSiClose64(&FidFrom);
	if (FidTo != INVALID_HANDLE_VALUE)
	{
		ii=FlushFileBuffers(FidTo);
		GSSiClose64(&FidTo);
		if (pos == -2)
		{
			char name[MAX_PATH + 32];
			strcpy(name, ToFile);
			LPSTR pDot = strrchr(name, '.');
			if (pDot && !stricmp(pDot, ".beingcached"))
			{
				*pDot = 0;
				strcat(name, ".tbr");
				GSSiRename(ToFile, name);
			}
			pos = 0;
		}
	}

	return pos;
}

void ContinueInteruptedCache(LPSTR CacheDir)
{
	char	str[600];
	char	File[MAX_PATH];
	char	RestartFile[MAX_PATH];
	HANDLE	FidRestart;
	LONGLONG	RestartPos;
	LPSTR	DataLocDir;
	LPSTR	pTab;
	OFSTRUCTGM	OFStruct = { 0 };

	sprintf(RestartFile, "%sRestartCache.txt", CacheDir);
	FidRestart = OpenFileGM(RestartFile, &OFStruct, OF_READ);
	if (FidRestart != INVALID_HANDLE_VALUE)
	{
		BigRead64(FidRestart, str, 600);
		GSSiClose64(&FidRestart);
		FidRestart = OpenFileGM(RestartFile, &OFStruct, OF_DELETE);
		DataLocDir = strchr(str, '\t');
		*DataLocDir++ = 0;
		pTab = strchr(DataLocDir, '\t');
		*pTab++ = 0;
		RestartPos = _atoi64(pTab);
		strcpy(File, str);
		REPLAC(File, "[%DL]", DataLocDir, 600);
		CacheFileInBackground(File, CacheDir, DataLocDir, RestartPos);
	}
	return;
}

BOOL AnotherProcessIsCaching(LPSTR ProcessIDFile)
{
	LPDWORD pPid = malloc(4096 * sizeof(DWORD));
	DWORD	nBytes, nPid;
	BOOL	rtn = FALSE;
	DWORD	CachingPid=0;
	OFSTRUCTGM	OFStruct = { 0 };
	char	cPid[32];
	HANDLE	Fid = OpenFileGM(ProcessIDFile, &OFStruct, OF_READ);

	if (Fid == INVALID_HANDLE_VALUE)
		return FALSE;

	int ln=BigRead64(Fid, cPid, sizeof(cPid));
	if (ln > -1)
	{
		cPid[ln] = 0;
		CachingPid = atol(cPid);
	}
	GSSiClose64(&Fid);
	EnumProcesses(pPid, 4096 * sizeof(DWORD), &nBytes);
	nPid = nBytes / sizeof(DWORD);
	while (nPid--)
	{
		if (pPid[nPid] == CachingPid)
		{
			HMODULE hMods[1024];
			HANDLE hProcess;
			DWORD cbNeeded;
			unsigned int i;

			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, CachingPid);
			if (NULL == hProcess)
			{
				ii = GetLastError();
				goto Exit;
			}
			if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
			{
				for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
				{
					TCHAR szModName[MAX_PATH];

					// Get the full path to the module's file.

					if (GetModuleFileNameEx(hProcess, hMods[i], szModName,
						sizeof(szModName) / sizeof(TCHAR)))
					{
						ii = 1;
					}
				}
			}

			CloseHandle(hProcess);

			rtn = TRUE;
			break;
		}
	}
Exit:
	free(pPid);
	return rtn;
}
