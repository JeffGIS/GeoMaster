#include "graphint.h"  
#include "umio.h"
#include "CDDEMO.h"
#include "extrndb.h"   
#include <sqlext.h>

#include "gmextern.h"
#include <psapi.h>

static	BYTE	Cachebuf[USHRT_MAX];
static	UINT	lCachebuf = USHRT_MAX;
static  int		nCalls = 0;
int CreateFilesToCacheFile(LPSTR cachFileList, HFILE fidOut);
void CacheFileInBackground(LPSTR FromFileIN, LPSTR CacheDir, LPSTR DataLocDir, LONGLONG StartPos);

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

		GetGlobalCVal("[%BackgroundCacheFilelist]", BackgroundCacheFilelist, "[%DL]BackgroundCacheFilelist.txt");
		sprintf(FilesToCacheFile, "%sFilesToCacheFile.txt", CachePathnameTo);
		ExpandText(FilesToCacheFile);
		sprintf(CachingPidFile, "%sCachingPid.txt", CachePathnameTo);
		ExpandText(CachingPidFile);

		HFILE fidFilesToCache = OpenFileGM(FilesToCacheFile, &OFStruct, OF_CREATE);
		int nFiles = CreateFilesToCacheFile(BackgroundCacheFilelist, fidFilesToCache);
		_lclose(fidFilesToCache);
		if (!nFiles)
		{
			sprintf(FilesToCacheFile, "%sCACHE_IS_COMPLETE.tbr", CachePathnameTo);
			HFILE fid = OpenFileGM(FilesToCacheFile, &OFStruct, OF_CREATE);
			_lwrite(fid, "NoFiles", 6);
			_lclose(fid);

			DestroyWindow(hWnd);
		}
		else
		{
			DWORD	Pid = _getpid();
			char cPid[32];
			sprintf(cPid,"%lli", (LONGLONG)Pid);
			HFILE Fid = OpenFileGM(CachingPidFile, &OFStruct, OF_CREATE);
			_lwrite(Fid, (LPSTR)cPid, strlen(cPid));
			_lclose(Fid);
			SetTimer(hWnd, GMCACHE_TIMER, 20, 0);
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
			KillTimer(hWnd, wParam);
			if (StartCachingFiles ())
				PostQuitMessage(0);
			return 0;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_SIZE:
		return DefWindowProc(hWnd, message, wParam, lParam);
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

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
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
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GMEDIT_LARGE));
	wcex.hCursor = LoadCursor(NULL, IDC_IBEAM);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_GMEDIT);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_GMEDIT_SMALL));

	return RegisterClassEx(&wcex);
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
				TranslateMessage(&msg);
				DispatchMessage(&msg);
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
	char RenameCompleteFile[MAX_PATH];
	char TrustedCacheFiles[MAX_PATH];
	char cDate[24];
	time_t currentTime;
	time_t lastDataUpdateTime = 0;
	time_t lastCacheStartTime = 0;
	time_t lastCacheCompleteTime = 0;

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
	strcpy(RenameCompleteFile, "[%DL]renameComplete.txt");
	ExpandText(LastDataUpdateFile);

	HFILE	Fid = OpenFileGM(LastDataUpdateFile, &OFStruct, OF_READ);

	if (Fid != HFILE_ERROR)
	{
		int ln = _lread(Fid, cDate, 22);
		ln = min(22,ln);
		cDate[ln] = 0;

		lastDataUpdateTime = atol(cDate);
		_lclose(Fid);
	}
	Fid = OpenFileGM(LastCacheStartTimeFile, &OFStruct, OF_READ);

	if (Fid != HFILE_ERROR)
	{
		int ln = _lread(Fid, cDate, 22);
		ln = min(22, ln);
		cDate[ln] = 0;

		lastCacheStartTime = atol(cDate);
		_lclose(Fid);
	}
	Fid = OpenFileGM(LastCacheCompleteTimeFile, &OFStruct, OF_READ);

	if (Fid != HFILE_ERROR)
	{
		int ln = _lread(Fid, cDate, 22);
		ln = min(22, ln);
		cDate[ln] = 0;

		lastCacheCompleteTime = atol(cDate);
		_lclose(Fid);
	}
	//lastCacheCompleteTime = 0;
	if (lastDataUpdateTime > lastCacheCompleteTime)
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
			_lwrite(Fid, cDate, ln + 1);
			_lclose(Fid);
			GSSiRemove(TrustedCacheFiles);
			GSSiGlobFree(&hTrustedCacheFiles);
			StartGMCache();
		}
	}
	if (!ExistFile(RenameCompleteFile) && !GMCacheTimer)
		GMCacheTimer = SetTimer(hWndMain, CACHE_FILE_RENAME_TIMER, 15000, 0);
	return;
}

void UpdateLastDataUpdate(void)
{
	char LastDataUpdateFile[MAX_PATH];
	time_t currentTime;
	OFSTRUCTGM	OFStruct = { 0 };
	char cDate[32];

	time(&currentTime);

	strcpy(LastDataUpdateFile, "[%DL]lastdataupdate.txt");
	ExpandText(LastDataUpdateFile);
	sprintf(cDate, "%lli", currentTime);
	HFILE Fid = OpenFileGM(LastDataUpdateFile, &OFStruct, OF_CREATE);
	int ln = strlen(cDate);
	_lwrite(Fid, cDate, ln + 1);
	_lclose(Fid);
}

void FreeTrustedFiles(void)
{
	GSSiGlobFree(&hTrustedCacheFiles);
	nCalls++;
}
BOOL StartCachingFiles(void)
{
	char DataLocDir[MAX_PATH];
	char CacheDir[MAX_PATH];
	char FilesToCacheFile[MAX_PATH];
	OFSTRUCTGM	OFStruct = { 0 };
	double PctDone = 0;
	LONGLONG StartPos = 0;
	char FromFile[MAX_PATH + 2];
	char mess[MAX_PATH*2];
	int totLen;

	strcpy(DataLocDir, "[%DL]");
	ExpandText(DataLocDir);
	strcpy(CacheDir, CachePathnameTo);
	ExpandText(CacheDir);
	sprintf(FilesToCacheFile, "%sFilesToCacheFile.txt", CacheDir);
	ExpandText(FilesToCacheFile);

	HFILE fid = OpenFileGM(FilesToCacheFile, &OFStruct, OF_READ);
	totLen = _llseek(fid, 0, 2);
	_llseek(fid, 0, 0);
	while (fgetstring2(FromFile, MAX_PATH, fid))
	{
		int nDir = 3;
		int loc = _llseek(fid,0,1);
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

		CacheFileInBackground(FromFile, CacheDir, DataLocDir, StartPos);
		PctDone = 100.0 * (double)loc / (double)totLen;
		sprintf(mess, "%s (%.1f %% complete)",pName, PctDone);
		BackgroundUpdateMessage (mess);
	}
	_lclose(fid);
	sprintf(FromFile, "%sCACHE_IS_COMPLETE.tbr", CacheDir);
	fid = OpenFileGM(FromFile, &OFStruct, OF_CREATE);
	_lwrite(fid, "Done", 4);
	_lclose(fid);
	return TRUE;
}
int CreateFilesToCacheFile(LPSTR cachFileList,HFILE fidOut)
{
	FILE* FidFilelist;
	//char    File[MAX_PATH];
	//char	CacheDir[MAX_PATH];
	char	DataLocDir[MAX_PATH];
	//char	CachingPidFile[MAX_PATH];
	//char	cachFileList[MAX_PATH];
	char	tempFile[MAX_PATH];
	char	str[MAX_PATH + 2];
	HFILE	Fid, Fid2;
	OFSTRUCTGM	OFStruct;
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
					_llseek(Fid2, 0, 0);
					while (fgetstring2(str, MAX_PATH, Fid2))
					{
							REPLAC(str, "[%DL]", DataLocDir, MAX_PATH);
							fputstring2(str, fidOut);
							totFiles++;
					}
					_lclose(Fid2);
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

BOOL UseTrustedCacheFile(LPSTR FileName)
{
	BOOL rtn = FALSE;
	LPSTR pTrustedFiles;
	if (!hTrustedCacheFiles)
	{
		char TrustedCacheFiles[MAX_PATH];
		OFSTRUCTGM OFStruct = { 0 };
		sprintf(TrustedCacheFiles, "%sTrustedCacheFiles.txt", CachePathnameTo);
		ExpandText(TrustedCacheFiles);
		HFILE fid = OpenFileGM(TrustedCacheFiles, &OFStruct, OF_READ);
		if (fid != HFILE_ERROR)
		{
			int ln = _llseek(fid, 0, 2);
			_llseek(fid, 0, 0);
			hTrustedCacheFiles = GSSiGlobAlloc(0,GMEM_MOVEABLE,ln+2);
			LPSTR pTrustedFiles = GlobalLock(hTrustedCacheFiles);
			_lread(fid, pTrustedFiles, ln);
			pTrustedFiles[ln] = 0;
			GlobalUnlock(hTrustedCacheFiles);
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
	char	SearchString[32] = "*.tbr";
	HFILE	Fid, Fid2;
	long	TotFiles = 0;
	LPSTR	pDot;
	HFILE	FidCachedFiles = HFILE_ERROR;
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
		if (FidCachedFiles == HFILE_ERROR)
		{
			FidCachedFiles = OpenFileGM(TrustedCacheFiles, &OFStruct, OF_CREATE);
			fputstring2("", FidCachedFiles);
		}
		else
			_llseek(FidCachedFiles, 0, 2);
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
							sprintf(cDate, "%lli", currentTime);
							sprintf(LastCacheCompleteTimeFile, "%sLastCacheCompleteTime.txt", CachePathnameTo);
							ExpandText(LastCacheCompleteTimeFile);
							HFILE Fid = OpenFileGM(LastCacheCompleteTimeFile, &OFStruct, OF_CREATE);
							int ln = strlen(cDate);
							_lwrite(Fid, cDate, ln + 1);
							_lclose(Fid);
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
			}
		}
		GSSiGlobFree(&hTrustedCacheFiles);
	}
	if (FidCachedFiles != HFILE_ERROR)
		_lclose (FidCachedFiles);
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

void CacheFileInBackground(LPSTR FromFileIN, LPSTR CacheDir, LPSTR DataLocDir, LONGLONG StartPos)
{
	int	dtime;
	BY_HANDLE_FILE_INFORMATION fifrom, fito;
	HFILE	FidFrom, FidTo;
	int		lDL = 5;
	OFSTRUCTGM	OFStruct = { 0 };
	char	FromFile[MAX_PATH];
	char	ToFile[MAX_PATH];
	char	ToFileIntermediate[MAX_PATH];
	int		st;
	LONGLONG	FromSize, ToSize, TotRead;
	UINT	OFMode = OF_CREATE;
	DWORD	lRead;
	LPSTR	pDot;
	char	CacheFromName[MAX_PATH];
	char	altDir[MAX_PATH];
	int		icpf, l;
	char mess[256];

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
	return;

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
	strcat(ToFileIntermediate, ".tbr");
	FidFrom = OpenFileGM(FromFile, &OFStruct, OF_READ);
	FidTo = OpenFileGM(ToFileIntermediate, &OFStruct, OF_READ);
	if (FidTo == HFILE_ERROR)
		FidTo = OpenFileGM(ToFile, &OFStruct, OF_READ);
	if (FidFrom == HFILE_ERROR)
	{
		if (FidTo != HFILE_ERROR)
		{
			_lclose(FidTo);
			OpenFileGM(ToFile, &OFStruct, OF_DELETE);
			OpenFileGM(ToFileIntermediate, &OFStruct, OF_DELETE);
		}
		return;
	}
	strcpy(ToFile, ToFileIntermediate);
	st = GetFileInformationByHandle((HANDLE)FidFrom, &fifrom);
	FromSize = (LONGLONG)fifrom.nFileSizeLow + ULONG_MAX * (LONGLONG)fifrom.nFileSizeHigh;
	if (FidTo == HFILE_ERROR)
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
		_lclose(FidTo);
		FidTo = HFILE_ERROR;
	}
	if (dtime > 0)
	{
		BOOL	OkToCache = makedirectories(ToFile, FALSE, FALSE);

		if (OkToCache)
		{
			LONGLONG FreeSpace = GetDriveFreeSpace(ToFile);

			FreeSpace -= FromSize;
			if (FreeSpace < MinCacheDriveFreeSpace)
				OkToCache = FALSE;
			if (OkToCache)
			{
				FidTo = OpenFileGM(ToFile, &OFStruct, OFMode);
				if (FidTo != HFILE_ERROR)
				{
					BOOL st;
					llFileSeek((HANDLE)FidTo, StartPos, 0);
					llFileSeek((HANDLE)FidFrom, StartPos, 0);
					TotRead = StartPos;
					st = ReadFile((HANDLE)FidFrom, Cachebuf, lCachebuf, &lRead, 0);
					while (ContinueBackgroundCache && st && lRead > 0)
					{
						if (lRead != _lwrite(FidTo, Cachebuf, lRead))
						{
							_lclose(FidTo);
							FidTo = HFILE_ERROR;
							OpenFileGM(ToFile, &OFStruct, OF_DELETE);
							break;
						}
						TotRead += lRead;
						st = ReadFile((HANDLE)FidFrom, Cachebuf, lCachebuf, &lRead, 0);
					}
					if (!ContinueBackgroundCache && TotRead != FromSize)
					{
						HFILE	FidRestart;
						char	str[600];
						char	RestartFile[MAX_PATH];

						if (TotRead)
						{
							SubstituteDL(FromFileIN, FALSE);
							sprintf(str, "%s\t%s\t%lld", FromFileIN, DataLocDir, TotRead);
							sprintf(RestartFile, "%sRestartCache.txt", CacheDir);
							FidRestart = OpenFileGM(RestartFile, &OFStruct, OF_CREATE);
							_lwrite(FidRestart, str, 600);
							_lclose(FidRestart);
						}
					}
				}
			}
		}
	}
	else
	{
		LPSTR pDollar = strrchr(ToFileIntermediate, '$');
		if (pDollar)
		{
			OpenFileGM(ToFileIntermediate, &OFStruct, OF_DELETE);
			*pDollar = '#';
			HFILE FidIsGood = OpenFileGM(ToFileIntermediate, &OFStruct, OF_CREATE);
			_lwrite(FidIsGood, "IsGood", 6);
			_lclose(FidIsGood);
		}
	}
	_lclose(FidFrom);
	if (FidTo != HFILE_ERROR)
		_lclose(FidTo);

	return;
}

void ContinueInteruptedCache(LPSTR CacheDir)
{
	char	str[600];
	char	File[MAX_PATH];
	char	RestartFile[MAX_PATH];
	HFILE	FidRestart;
	LONGLONG	RestartPos;
	LPSTR	DataLocDir;
	LPSTR	pTab;
	OFSTRUCTGM	OFStruct;

	sprintf(RestartFile, "%sRestartCache.txt", CacheDir);
	FidRestart = OpenFileGM(RestartFile, &OFStruct, OF_READ);
	if (FidRestart != HFILE_ERROR)
	{
		_lread(FidRestart, str, 600);
		_lclose(FidRestart);
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
	DWORD	CachingPid;
	OFSTRUCTGM	OFStruct;
	char	cPid[32];
	HFILE	Fid = OpenFileGM(ProcessIDFile, &OFStruct, OF_READ);

	if (Fid == HFILE_ERROR)
		return FALSE;

	int ln=_lread(Fid, cPid, sizeof(cPid));
	if (ln > -1)
	{
		cPid[ln] = 0;
		CachingPid = atol(cPid);
	}
	_lclose(Fid);
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
