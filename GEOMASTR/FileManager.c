#include "graphint.h"   
#include "gmextern.h"

static char manFile[MAX_PATH];
static char deleteFile[MAX_PATH];
static char IncludedFilesPath[MAX_PATH]="c:\\temp\\FileManagerIncludedFiles.txt";
static sqlite3* db = NULL;
static HFILE	fidDeleteList = HFILE_ERROR;
static int	currentDisplay = 0;
static int	currentGroup = 1;
static char currentSource[34] = "[%DL]";
static char currentDestination[34] = "";
static LONGLONG  totLen = 0;
static LONGLONG	 totIncludedLen = 0;
static int totFiles = 0;
static int totIncludedFiles = 0;
static HFILE fidIncludedFiles = HFILE_ERROR;

#define FM_INCLUDE	1
#define FM_LOCAL	2
#define FM_DELETED	3

#define MAX_ENTRY	MAX_PATH*5+80

static LPSTR FMFixPath(LPSTR path)
{
	LPSTR fixed = malloc(strlen(path) * 2 + 2);

	strcpy(fixed, path);
	REPLAC(fixed, "'", "''", strlen(path) * 2);
	return fixed;
}
static void AddFMSource(LPSTR Name)
{
	int groupID;
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, 4096);
	LPSTR  pCmd = GlobalLock(hCmd);
	char *error = NULL;
	sqlite3_stmt *statement;

	sprintf(pCmd, "INSERT INTO SOURCE VALUES ('%s',0)", Name);
	SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error),db, "Add source", &error);
	sqlite3_free(error);

	GSSiGlobUlFree(&hCmd);
}
static void ListFMSource(HWND hWndDlg)
{
	int groupID;
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, 4096);
	LPSTR  pCmd = GlobalLock(hCmd);
	char *error = NULL;

	SendDlgItemMessage(hWndDlg, IDC_FMSOURCELIST, LB_RESETCONTENT, 0, 0);

	sprintf(pCmd, "SELECT SourceName FROM SOURCE WHERE Removed = 0 ");
	sqlite3_stmt *statement;

	SQLOK(sqlite3_prepare_v2(db, pCmd, -1, &statement, NULL),db, "ListSource", &error);

	strcpy(pCmd, "[%DL]");
	SendDlgItemMessage(hWndDlg, IDC_FMSOURCELIST, LB_ADDSTRING, 0, (LPARAM)((LPSTR)pCmd));

	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		LPSTR Name = (LPSTR)sqlite3_column_text(statement, 0);
		SendDlgItemMessage(hWndDlg, IDC_FMSOURCELIST, LB_ADDSTRING, 0, (LPARAM)((LPSTR)Name));
	}

	SQLOK(sqlite3_finalize(statement), db, "finalize ListSource", 0);
	GSSiGlobUlFree(&hCmd);
	SendDlgItemMessage(hWndDlg, IDC_FMSOURCELIST, LB_SELECTSTRING, -1, (LPARAM)((LPSTR)currentSource));
}

static void AddGroup(LPSTR Name)
{
	int groupID=0;
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, 4096);
	LPSTR  pCmd = GlobalLock(hCmd);
	char *error = NULL;
	sqlite3_stmt *statement;

	sprintf(pCmd, "SELECT MAX(GroupID) FROM GROUPS");

	SQLOK(sqlite3_prepare_v2(db, pCmd, -1, &statement, NULL), db, "AddGroups", &error);

	if (sqlite3_step(statement) == SQLITE_ROW)
	{
		groupID = max(2, sqlite3_column_int(statement, 0));
	}
	SQLOK(sqlite3_finalize(statement), db, "finalize ListGroup", 0);
	groupID++;
	sprintf(pCmd, "INSERT INTO GROUPS VALUES (%i,'%s',0)", groupID, Name);
	SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "Add group", &error);
	sqlite3_free(error);

	GSSiGlobUlFree(&hCmd);
}
static void ListGroups(HWND hWndDlg)
{
	int groupID;
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, 4096);
	LPSTR  pCmd = GlobalLock(hCmd);
	char *error = NULL;

	SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTDISPLAY, LB_RESETCONTENT, 0, 0);
	SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTASSIGN, LB_RESETCONTENT, 0, 0);

	sprintf(pCmd, "SELECT GroupName,GroupID FROM GROUPS WHERE Removed = 0");
	sqlite3_stmt *statement;

	SQLOK(sqlite3_prepare_v2(db, pCmd, -1, &statement, NULL), db, "ListGroups", &error);

	strcpy(pCmd, "Unassigned\t0");
	SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTDISPLAY, LB_ADDSTRING, 0, (LPARAM)((LPSTR)pCmd));
	strcpy(pCmd, "CORE\t1");
	SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTDISPLAY, LB_ADDSTRING, 0, (LPARAM)((LPSTR)pCmd));
	SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTASSIGN, LB_ADDSTRING, 0, (LPARAM)((LPSTR)pCmd));
	strcpy(pCmd, "LOCAL\t2");
	SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTDISPLAY, LB_ADDSTRING, 0, (LPARAM)((LPSTR)pCmd));
	SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTASSIGN, LB_ADDSTRING, 0, (LPARAM)((LPSTR)pCmd));

	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		LPSTR Name = (LPSTR)sqlite3_column_text(statement, 0);
		groupID = sqlite3_column_int(statement, 1);
		sprintf(pCmd, "%s\t%i", Name, groupID);
		SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTDISPLAY, LB_ADDSTRING, 0, (LPARAM)((LPSTR)pCmd));
		SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTASSIGN, LB_ADDSTRING, 0, (LPARAM)((LPSTR)pCmd));
	}

	SQLOK(sqlite3_finalize(statement), db, "finalize ListGroup", 0);
	GSSiGlobUlFree(&hCmd);
}
static int FMGroup2(LPSTR path)
{
	int st = 0;
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, 4096);
	LPSTR  pCmd = GlobalLock(hCmd);
	char *error = NULL;
	LPSTR fixedPath = FMFixPath(path);

	sprintf(pCmd, "SELECT GroupID FROM PATHS WHERE path ='%s%s'", currentSource, fixedPath);
	free(fixedPath);
	sqlite3_stmt *statement;

	SQLOK(sqlite3_prepare_v2(db, pCmd, -1, &statement, NULL), db, "get Group", &error);

	if (sqlite3_step(statement) == SQLITE_ROW)
		st = sqlite3_column_int(statement, 0);

	SQLOK(sqlite3_finalize(statement), db, "finalize get Group", 0);
	GSSiGlobUlFree(&hCmd);

	return st;
}
static int FMGroup(LPSTR path)
{
	int st = 0;
	HANDLE hTestPath = GSSiGlobAlloc(1796, GMEM_MOVEABLE, 512);
	LPSTR  pTestPath = GlobalLock(hTestPath);
	LPSTR  pBS;

	strcpy(pTestPath, path);
	st = FMGroup2(pTestPath);
	pBS = strrchr(pTestPath, '\\');
	while (!st && pBS)
	{
		LPSTR pAt = pBS;

		*pBS = 0;
		pBS = strrchr(pTestPath, '\\');
		*pAt++ = '\\';
		*pAt = 0;
		st = FMGroup2(pTestPath);
	}
	GSSiGlobUlFree(&hTestPath);
	return st;
}
BOOL OpenFileManagerDB(LPSTR path)
{
	char *error = NULL;
	BOOL rtn = !SQLOK(sqlite3_open(path, &db), db, "open(Intersections)", &error);

	if (rtn)
	{
		HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX);
		LPSTR  pCmd = GlobalLock(hCmd);

		sprintf(pCmd, "CREATE TABLE IF NOT EXISTS PATHS (Path CHAR(256) PRIMARY KEY,GroupID INT,UpdateTime INT,Size INT)");
		rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "Create FILEMANAGER database", &error);
		sqlite3_free(error);
		sprintf(pCmd, "CREATE TABLE IF NOT EXISTS GROUPS (GroupID INT PRIMARY KEY,GroupName CHAR(64),Removed INT)");
		rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "Create FILEMANAGER database", &error);
		sqlite3_free(error);
		sprintf(pCmd, "CREATE TABLE IF NOT EXISTS SOURCE (SourceName CHAR(64),Removed INT)");
		rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "Create FILEMANAGER database", &error);
		sqlite3_free(error);
		if (!rtn)
		{
			rtn = !SQLOK(sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error), db, "Start transaction", &error);
			sqlite3_free(error);
			GSSiGetTempFileName(0, "gm", 0, deleteFile);
			fidDeleteList = GSSiOpenFile(deleteFile, 0, OF_CREATE);
		}
		else
			rtn = FALSE;
		GSSiGlobUlFree(&hCmd);
	}
	return rtn;
}

BOOL FMClose()
{
	BOOL rtn = !SQLOK(sqlite3_close(db), db, "Close FILEMANAGER database", 0);
	return rtn;
}

static BOOL SetFMCode(int code, LPSTR path)
{
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, 1024);
	LPSTR  pCmd = GlobalLock(hCmd);
	char *error = NULL;
	LPSTR fixedPath = FMFixPath(path);
	BOOL  rtn;

	sprintf(pCmd, "INSERT INTO PATHS VALUES('%s%s',%i,0,0)", currentSource, fixedPath, code);
	free(fixedPath);
	rtn = !SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "SetFMCode", &error);
	sqlite3_free(error);
	GSSiGlobUlFree(&hCmd);
	return rtn;
}
BOOL FMIncludeFile(LPSTR path,int group, long lastUpdateTime, long fileSize)
{
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, 1024);
	LPSTR  pCmd = GlobalLock(hCmd);
	char *error = NULL;
	LPSTR fixedPath = FMFixPath(path);
	BOOL  rtn;

	sprintf(pCmd, "INSERT INTO PATHS VALUES('%s%s',%i,%i,%i)", currentSource,fixedPath, group, lastUpdateTime, fileSize);
	free(fixedPath);
	rtn = !SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "Include file", &error);
	sqlite3_free(error);
	GSSiGlobUlFree(&hCmd);
	return rtn;
}

BOOL FMDeleteItem(LPSTR path)
{
	BOOL rtn=SetFMCode (FM_DELETED,path);

	if (rtn)
	{
		HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, 1024);
		LPSTR  pCmd = GlobalLock(hCmd);
		sprintf(pCmd, "%s%s", currentSource,path);
		fputstring(pCmd, fidDeleteList);
		GSSiGlobUlFree(&hCmd);
		rtn = TRUE;
	}
	else
		rtn = FALSE;
	return rtn;
}

static BOOL FMRollBack(HWND hWnd)
{
	BOOL rtn=FALSE;
	UINT opt = MessageBox(hWnd, "Do you really want to cancel all changes?", "", MB_YESNO);
	if (opt == IDYES)
	{
		HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX);
		LPSTR  pCmd = GlobalLock(hCmd);
		char *error = NULL;

		sprintf(pCmd, "ROLLBACK TRANSACTION");
		rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "Rollback", &error);
		sqlite3_free(error);
		if (!rtn)
		{
			GSSiClose(fidDeleteList);
			fidDeleteList = GSSiOpenFile(deleteFile, 0, OF_CREATE);
			rtn = TRUE;
			SQLOK(sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error), db, "Start transaction", &error);
			sqlite3_free(error);
		}
		else
			rtn = FALSE;
		GSSiGlobUlFree(&hCmd);
	}
	return rtn;
}

static BOOL FMCommit(void)
{
	BOOL rtn;
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, USHRT_MAX);
	LPSTR  pCmd = GlobalLock(hCmd);
	char *error = NULL;

	sprintf(pCmd, "COMMIT TRANSACTION");
	rtn = SQLOK(sqlite3_exec(db, pCmd, NULL, NULL, &error), db, "Commit", &error);
	sqlite3_free(error);
	if (!rtn)
	{
		GSSillseek(fidDeleteList, 0, 0);
		while (fgetstring(pCmd, 260, fidDeleteList))
		{
			if (*LastChr(pCmd) == '\\')
			{
				*LastChr(pCmd) = 0;
				DeleteDirAndContents(pCmd);
			}
			else
			{
				GSSiRemove(pCmd);
			}
		}
		GSSiClose(fidDeleteList);
		fidDeleteList = GSSiOpenFile(deleteFile, 0, OF_CREATE);
		rtn = TRUE;
		SQLOK(sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &error), db, "Start transaction", &error);
		sqlite3_free(error);
	}
	else
		rtn = FALSE;
	GSSiGlobUlFree(&hCmd);
	return rtn;
}

static BOOL CreateFMExtract(HWND hWndDlg)
{
	BOOL rtn = FALSE;
	char str[512]="";
	char fromFile[MAX_PATH], toFile[MAX_PATH];
	LPSTR pTab=str;
	int totLen, curPos = 0;

	fidIncludedFiles = GSSiOpenFile(IncludedFilesPath, 0, OF_READ);
	if (fidIncludedFiles != HFILE_ERROR)
	{
		totLen = GSSifilelength(fidIncludedFiles);
		CreateStatusWind(CurView->hWnd, 1, "Create Extract");
		while (StatusWindowUpdate(0,pTab, totLen, curPos) && fgetstring(str, sizeof(str)-2, fidIncludedFiles))
		{
			pTab = strchr(str, '\t');
			*pTab++ = 0;
			sprintf(fromFile, "%s%s", str, pTab);
			sprintf(toFile, "%s\\%s", currentDestination, pTab);
			GSSiCopyFile(fromFile, toFile, FALSE);
			curPos = GSSillseek(fidIncludedFiles, 0, 1);
		}
		DestroyStatusWindow(0);
		GSSiClose(fidIncludedFiles);
	}

	return rtn;
}
static BOOL FMScan(HWND hWndDlg)
{
	int pos;
	long offset;
	char SearchLoc[128] = "[%DL]";
	char prefix[128];
	char TempFile[MAX_PATH];
	char copyPath[MAX_PATH];
	char entry[MAX_ENTRY];
	char str[MAX_ENTRY];
	char CtotSize[32];
	char CtotIncludedSize[32];
	int nfiles, lprefix;
	HFILE Fid;
	long  fileLen, lastWrite;
	
	totFiles = 0;
	totLen = 0;
	totIncludedFiles = 0;
	totIncludedLen = 0;

	if (GetDlgItemText(hWndDlg, IDC_FMDEST, currentDestination, sizeof(currentDestination)-1))
	{
		fidIncludedFiles = GSSiOpenFile(IncludedFilesPath, 0, OF_CREATE);
	}
	else
		fidIncludedFiles = HFILE_ERROR;
	WaitCursor(1);
	strcpy(SearchLoc, currentSource);
	SendDlgItemMessage(hWndDlg, IDC_FMFILELIST, LB_RESETCONTENT, 0, 0);
	ExpandText(SearchLoc);
	strcpy(prefix, SearchLoc);
	strlwr(prefix);
	lprefix = strlen(prefix);
	GSSiGetTempFileName(0, "gm", 0, TempFile);
	nfiles = GetFileList(TempFile, TRUE, SearchLoc, "*.*", TRUE, FALSE,FALSE);
	Fid = GSSiOpenFile(TempFile, 0, OF_READ);
	fgetstring(entry, MAX_ENTRY - 2, Fid);
	while (fgetstring(entry, MAX_ENTRY-2, Fid))
	{
		LPSTR pTab = strchr(entry, '\t');
		LPSTR pEntry = entry;
		if (pTab)
		{
			LPSTR pBeg;
			*pTab++ = 0;
			pBeg = pTab;
			pTab = strrchr(pBeg, '\t');
			*pTab-- = 0;
			pTab = strrchr(pBeg, '\t');
			fileLen = atoi(pTab+1);
			*pTab-- = 0;
			pTab = strrchr(pBeg, '\t');
			lastWrite = atoi(pTab + 1);
		}
		strlwr(entry);
		if (!strncmp(entry, prefix, lprefix))
			pEntry += lprefix;
		switch (FMGroup(pEntry))
		{
			case FM_INCLUDE:
				totIncludedFiles++;
				totIncludedLen += fileLen;
				if (fidIncludedFiles != HFILE_ERROR)
				{
					sprintf(copyPath, "%s\t%s", currentSource,pEntry);
					ExpandText(copyPath);
					fputstring(copyPath, fidIncludedFiles);
				}
				break;
			case FM_LOCAL:
			case FM_DELETED:
				break;
			default:
			{
				char cLastWrite[64];
				sprintf(cLastWrite, "$CAL(%i,3)", lastWrite);
				ExpandText(cLastWrite);
				sprintf(str, "%s\t%s  %i\t%i %i", pEntry, cLastWrite, fileLen, lastWrite, fileLen);
				SendDlgItemMessage(hWndDlg, IDC_FMFILELIST, LB_ADDSTRING, 0, (LPARAM)((LPSTR)str));
				totFiles++;
				totLen += fileLen;
				break;
			}
		}
	}
	GSSiClose(Fid);
	GSSiRemove(TempFile);
	itoa(IDNINT(totLen / (1024.0*1024.0)), CtotSize, 10);
	AddCommas(CtotSize);
	itoa(IDNINT(totIncludedLen / (1024.0*1024.0)), CtotIncludedSize, 10);
	AddCommas(CtotIncludedSize);
	sprintf(str, "Total Files: %i Total Size: %s MB      Included Files: %i Included Size: %s MB", totFiles, CtotSize, totIncludedFiles, CtotIncludedSize);
	SetDlgItemText(hWndDlg, IDC_FMTOTALS,str);
	WaitCursor(-1);
	if (fidIncludedFiles != HFILE_ERROR)
	{
		GSSiClose(fidIncludedFiles);
		EnableWindow(GetDlgItem(hWndDlg, IDC_FMCREATEEXTRACT),TRUE);
	}
	else
		EnableWindow(GetDlgItem(hWndDlg, IDC_FMCREATEEXTRACT), FALSE);

	return TRUE;
};
static void ListFMSavedFiles(HWND hWndDlg)
{
	HANDLE hCmd = GSSiGlobAlloc(1796, GMEM_MOVEABLE, 4096);
	LPSTR  pCmd = GlobalLock(hCmd);
	char *error = NULL;
	char str[MAX_ENTRY];
	char CtotSize[32];
	long  fileLen, lastWrite;

	totFiles = 0, totLen = 0;
	WaitCursor(1);
	sprintf(pCmd, "SELECT Path,UpdateTime,Size FROM PATHS WHERE GroupID = %i", currentDisplay);
	sqlite3_stmt *statement;

	SQLOK(sqlite3_prepare_v2(db, pCmd, -1, &statement, NULL), db, "ListFiles", &error);

	while (sqlite3_step(statement) == SQLITE_ROW)
	{
		LPSTR path = (LPSTR)sqlite3_column_text(statement, 0);
		int lastWrite = sqlite3_column_int(statement, 1);
		int fileLen = sqlite3_column_int(statement, 2);
		char cLastWrite[64];
		sprintf(cLastWrite, "$CAL(%i,3)", lastWrite);
		ExpandText(cLastWrite);
		sprintf(str, "%s\t%s  %i\t%i %i", path, cLastWrite, fileLen, lastWrite, fileLen);
		SendDlgItemMessage(hWndDlg, IDC_FMFILELIST, LB_ADDSTRING, 0, (LPARAM)((LPSTR)str));
		totFiles++;
		totLen += fileLen;
	}

	SQLOK(sqlite3_finalize(statement), db, "finalize ListGroup", 0);
	GSSiGlobUlFree(&hCmd);
	itoa(IDNINT(totLen / (1024.0*1024.0)), CtotSize, 10);
	AddCommas(CtotSize);
	sprintf(str, "Total Files: %i Total Size: %s MB      Selected Files: 0   Selected Size: 0 MB", totFiles, CtotSize);
	SetDlgItemText(hWndDlg, IDC_FMTOTALS, str);
	WaitCursor(-1);
}

static void DisplayFMFiles(HWND hWndDlg,BOOL force)
{
	if (force || SendDlgItemMessage(hWndDlg, IDC_FMAUTOSCAN,BM_GETCHECK, 0, 0))
	{
		SendDlgItemMessage(hWndDlg, IDC_FMFILELIST, LB_RESETCONTENT, 0, 0);
		switch (currentDisplay)
		{
		case 0:
			FMScan(hWndDlg);
			break;
		default:
			ListFMSavedFiles(hWndDlg);
			break;
		}
	}
}
static void selectFMDirectory(HWND hWndDlg, LPSTR dir)
{
	int ibeg, iend;
	int ldir = strlen(dir);
	char txt[260];
	ibeg = iend = SendDlgItemMessage(hWndDlg, IDC_FMFILELIST, LB_FINDSTRING, -1, (LPARAM)dir);
	while (SendDlgItemMessage(hWndDlg, IDC_FMFILELIST, LB_GETTEXT, ++iend, (LPARAM)txt) >= 0)
	{
		if (strncmp(dir, txt,ldir))
			break;
	}
	iend--;
	SendDlgItemMessage(hWndDlg, IDC_FMFILELIST, LB_SELITEMRANGEEX, ibeg, iend);
}

static void setSelectedSize(HWND hWndDlg)
{
	LONGLONG selectedLen = 0;
	char txt[256];
	HANDLE	hItems;
	int n = GetLBSelectedItems(hWndDlg, IDC_FMFILELIST, &hItems);
	LPINT	pItem;
	LPSTR	pTab;
	int selectedFiles = n;
	char CtotSize[32], CselSize[32];

	if (hItems)
	{
		pItem = (LPINT)GlobalLock(hItems);
		while (n--)
		{
			SendDlgItemMessage(hWndDlg, IDC_FMFILELIST, LB_GETTEXT, *pItem++, (DWORD)txt);
			if ((pTab = strrchr(txt, ' ')))
				*pTab++ = 0;
			selectedLen += atoi(pTab);
		}
		GSSiGlobUlFree(&hItems);
	}
	itoa(IDNINT(totLen / (1024.0*1024.0)), CtotSize,10);
	AddCommas(CtotSize);
	itoa(IDNINT(selectedLen / (1024.0*1024.0)), CselSize,10);
	AddCommas(CselSize);
	sprintf(txt, "Total Files: %i Total Size: %s MB      Selected Files: %i   Selected Size: %s MB", totFiles, CtotSize, selectedFiles,CselSize);
	SetDlgItemText(hWndDlg, IDC_FMTOTALS, txt);
}

BOOL FAR PASCAL FileManagerMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	char txt[1024];
	char filePath[MAX_PATH];
	int	BRtn, i;
	int TabStops[2] = { 620, 2100 };
	LPSTR pTab;

	if ((BRtn = DIALOGSTYLEMsgProc(hWndDlg, Message, wParam, lParam)))
		return (BRtn);
	switch (Message)
	{
	case WM_INITDIALOG:
		EscapeFunction(TRUE);
		AllowCache = FALSE;
		CloseAllRequestedFiles(FALSE); 
		SendDlgItemMessage(hWndDlg, IDC_FMAUTOSCAN, BM_SETCHECK, FALSE, 0);
		SendDlgItemMessage(hWndDlg, IDC_FMFILELIST, LB_SETTABSTOPS, 2, (LPARAM)&TabStops);
		SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTDISPLAY, LB_SETTABSTOPS, 2, (LPARAM)&TabStops);
		SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTASSIGN, LB_SETTABSTOPS, 2, (LPARAM)&TabStops);
		TabStops[0] = 90;
		SendDlgItemMessage(hWndDlg, IDC_FMSITELIST, LB_SETTABSTOPS, 2, (LPARAM)&TabStops);
		cwCenter(hWndDlg, 0);
		if (OpenFileManagerDB(manFile))
		{
			ListGroups(hWndDlg);
			DisplayFMFiles(hWndDlg,FALSE);
		}
		else
			MessageBox(hWndDlg, "Unable to open FILEMANAGER database", 0, MB_ICONEXCLAMATION);
		SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTDISPLAY, LB_SETCURSEL,0, 0);
		SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTASSIGN, LB_SETCURSEL, 0, 0);
		SendDlgItemMessage(hWndDlg, IDC_FMSITELIST, LB_ADDSTRING, 0,(LPARAM) "Engineering\tLocal");
		SendDlgItemMessage(hWndDlg, IDC_FMSITELIST, LB_ADDSTRING, 0, (LPARAM) "Laptops\tDual");
		SendDlgItemMessage(hWndDlg, IDC_FMSITELIST, LB_ADDSTRING, 0, (LPARAM)"Parks\tDetatched");
		SendDlgItemMessage(hWndDlg, IDC_FMSITELIST, LB_ADDSTRING, 0, (LPARAM)"Police\tDetatched");
		ListFMSource(hWndDlg);
		break; /* End of WM_INITDIALOG                                 */

	case WM_CLOSE:
		/* Closing the Dialog behaves the same as Cancel               */
		PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
		break; /* End of WM_CLOSE                                      */

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			if (FMRollBack(hWndDlg))
			{
				FMClose();
				GSSiClose(fidDeleteList);
				GSSiRemove(deleteFile);
				EndDialog(hWndDlg, FALSE);
			}
			break;
		case IDOK:
			FMCommit();
			FMClose();
			GSSiClose(fidDeleteList);
			GSSiRemove(deleteFile);
			EndDialog(hWndDlg, TRUE);
			break;
		case IDC_FMUNDO:
			if (FMRollBack(hWndDlg))
				DisplayFMFiles(hWndDlg, FALSE);
			break;
		case IDC_FMCOMMIT:
			FMCommit();
			//DisplayFMFiles(hWndDlg);
			break;
		case IDC_FMCREATEEXTRACT:
			CreateFMExtract(hWndDlg);
			break;
		case IDC_FMFILELIST:
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
			{
				HANDLE	hItems;
				int n = GetLBSelectedItems(hWndDlg, IDC_FMFILELIST, &hItems);
				LPINT	pItem = (LPINT)GlobalLock(hItems);

				SetDlgItemText(hWndDlg, IDC_FMFILE, "");
				if (n > 0)
				{
					EnableWindow(GetDlgItem(hWndDlg, IDC_FMINCLUDEFILE), TRUE);
					EnableWindow(GetDlgItem(hWndDlg, IDC_FMEXCLUDEFILE), TRUE);
					EnableWindow(GetDlgItem(hWndDlg, IDC_FMDELETEFILES), TRUE);

					EnableWindow(GetDlgItem(hWndDlg, IDC_FMINCLUDEDIR), FALSE);
					EnableWindow(GetDlgItem(hWndDlg, IDC_FMEXCLUDEDIR), FALSE);
				}
				if (n == 1)
				{
					SendDlgItemMessage(hWndDlg, IDC_FMFILELIST, LB_GETTEXT, *pItem++, (DWORD)txt);
					if ((pTab = strrchr(txt, '\t')))
						*pTab++ = 0;
					if ((pTab = strchr(txt, '\t')))
						*pTab++ = 0;
					SetDlgItemText(hWndDlg, IDC_FMFILE, txt);
				}
				GSSiGlobUlFree(&hItems);
				setSelectedSize(hWndDlg);
			}
				break;
			}
			break;
		case IDC_FMRESCAN:
			DisplayFMFiles(hWndDlg, TRUE);
			break;

		case IDC_FMSOURCELIST:
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
			{
				i = SendDlgItemMessage(hWndDlg, IDC_FMSOURCELIST, LB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hWndDlg, IDC_FMSOURCELIST, LB_GETTEXT, i, (DWORD)txt);
				strcpy (currentSource,txt);
				sprintf(txt, "@%s (%s)", currentSource, currentSource);
				ExpandText(txt);
				SetDlgItemText(hWndDlg, IDC_FMHEADER, txt);
				DisplayFMFiles(hWndDlg, TRUE);
			}
				break;
			}
			break;
		case IDC_FMGROUPLISTDISPLAY:
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
			{
				i = SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTDISPLAY, LB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTDISPLAY, LB_GETTEXT, i, (DWORD)txt);
				pTab = strrchr(txt, '\t');
				currentDisplay = atoi(++pTab);
				DisplayFMFiles(hWndDlg, FALSE);
			}
				break;
			}
			break;
		case IDC_FMGROUPLISTASSIGN:
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
			{
				i = SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTASSIGN, LB_GETCURSEL, 0, 0);
				SendDlgItemMessage(hWndDlg, IDC_FMGROUPLISTASSIGN, LB_GETTEXT, i, (DWORD)txt);
				pTab = strrchr(txt, '\t');
				currentGroup = atoi(++pTab);
			}
				break;
			}
			break;
		case IDC_FMFILE:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
			{
				strcpy(txt, currentSource);
				if (GetDlgItemText(hWndDlg, IDC_FMFILE, strchr(txt, 0), MAX_PATH))
				{
					ExpandText(txt);

					EnableWindow(GetDlgItem(hWndDlg, IDC_FMINCLUDEDIR), FALSE);
					EnableWindow(GetDlgItem(hWndDlg, IDC_FMEXCLUDEDIR), FALSE);
					EnableWindow(GetDlgItem(hWndDlg, IDC_FMOPENDIR), FALSE);
					EnableWindow(GetDlgItem(hWndDlg, IDC_FMSELECTDIR), FALSE);
					EnableWindow(GetDlgItem(hWndDlg, IDC_FMDELETEDIR), FALSE);
					switch (FileType(txt))
					{
					case 1:
						break;
					case 2:
						EnableWindow(GetDlgItem(hWndDlg, IDC_FMINCLUDEDIR), TRUE);
						EnableWindow(GetDlgItem(hWndDlg, IDC_FMEXCLUDEDIR), TRUE);
						EnableWindow(GetDlgItem(hWndDlg, IDC_FMOPENDIR), TRUE);
						EnableWindow(GetDlgItem(hWndDlg, IDC_FMSELECTDIR), TRUE);
						EnableWindow(GetDlgItem(hWndDlg, IDC_FMDELETEDIR), TRUE);
						break;
					}
					break;
				}
			}
			}
			break;
		case IDC_FMDELETEFILES:
		{
			HANDLE	hItems;
			int n = GetLBSelectedItems(hWndDlg, IDC_FMFILELIST, &hItems);
			LPINT	pItem;

			if (hItems)
			{
				pItem = (LPINT)GlobalLock(hItems);
				while (n--)
				{
					SendDlgItemMessage(hWndDlg, IDC_FMFILELIST, LB_GETTEXT, *pItem++, (DWORD)txt);
					if ((pTab = strrchr(txt, '\t')))
						*pTab++ = 0;
					if ((pTab = strchr(txt, '\t')))
						*pTab++ = 0;
					FMDeleteItem(txt);
				}
				GSSiGlobUlFree(&hItems);
				DisplayFMFiles(hWndDlg, FALSE);
			}
		}
			break;
		case IDC_FMINCLUDEFILE:
		case IDC_FMEXCLUDEFILE:
		{
			HANDLE	hItems;
			int n = GetLBSelectedItems(hWndDlg, IDC_FMFILELIST, &hItems);
			LPINT	pItem;
			long	lastUpdateTime, fileSize;

			if (hItems)
			{
				pItem = (LPINT)GlobalLock(hItems);
				while (n--)
				{
					SendDlgItemMessage(hWndDlg, IDC_FMFILELIST, LB_GETTEXT, *pItem++, (DWORD)txt);
					if ((pTab = strrchr(txt, '\t')))
						*pTab++ = 0;
					lastUpdateTime = atoi(pTab);
					pTab = strchr(pTab, ' ');
					fileSize = atoi(pTab);
					if ((pTab = strchr(txt, '\t')))
						*pTab++ = 0;
					if (LOWORD(wParam) == IDC_FMINCLUDEFILE)
						FMIncludeFile(txt, currentGroup, lastUpdateTime, fileSize);
					else
						SetFMCode(FM_LOCAL, txt);
				}
				GSSiGlobUlFree(&hItems);
				DisplayFMFiles(hWndDlg, FALSE);
			}
		}
			break;
		case IDC_FMUPTODIR:
			GetDlgItemText(hWndDlg, IDC_FMFILE, txt, MAX_PATH);
			if ((pTab = strrchr(txt, '\\')))
			{
				*pTab = 0;
				SetDlgItemText(hWndDlg, IDC_FMFILE, txt);
			}
			break;
		case IDC_FMOPENDIR:
		{
			char cmd[512];

			GetDlgItemText(hWndDlg, IDC_FMFILE, txt, MAX_PATH);
			if (*LastChr(txt) == '\\')
				*LastChr(txt) = 0;
			sprintf(cmd, "$WEB(%s%s)", currentSource, txt);
			ExpandText(cmd);
		}
			break;
		case IDC_FMSELECTDIR:
		{
			GetDlgItemText(hWndDlg, IDC_FMFILE, txt, MAX_PATH);
			if (*LastChr(txt) != '\\')
				strcat(txt, "\\");
			selectFMDirectory(hWndDlg, txt);
			setSelectedSize(hWndDlg);
		}
			break;
		case IDC_FMDELETEDIR:
		{
			GetDlgItemText(hWndDlg, IDC_FMFILE, txt, MAX_PATH);
			if (*LastChr(txt) != '\\')
				strcat(txt, "\\");
			FMDeleteItem(txt);
			DisplayFMFiles(hWndDlg, FALSE);
		}
			break;
		case IDC_FMINCLUDEDIR:
			GetDlgItemText(hWndDlg, IDC_FMFILE, txt, MAX_PATH);
			if (*LastChr(txt) != '\\')
				strcat(txt, "\\");
			FMIncludeFile(txt, currentGroup, 0,0);
			DisplayFMFiles(hWndDlg, FALSE);
			break;
		case IDC_FMEXCLUDEDIR:
			GetDlgItemText(hWndDlg, IDC_FMFILE, txt, MAX_PATH);
			if (*LastChr(txt) != '\\')
				strcat(txt, "\\");
			SetFMCode(FM_LOCAL, txt);
			DisplayFMFiles(hWndDlg, FALSE);
			break;
		case IDC_FMNEWGROUP:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				if (GetDlgItemText(hWndDlg, IDC_FMNEWGROUP, txt, 32))
					EnableWindow(GetDlgItem(hWndDlg, IDC_FMADDGROUP), TRUE);
				else
					EnableWindow(GetDlgItem(hWndDlg, IDC_FMADDGROUP), FALSE);
				break;
			}
			break;
		case IDC_FMADDGROUP:
			GetDlgItemText(hWndDlg, IDC_FMNEWGROUP, txt, 32);
			AddGroup(txt);
			SetDlgItemText(hWndDlg, IDC_FMNEWGROUP, "");
			ListGroups(hWndDlg);
			break;
		case IDC_FMNEWSOURCE:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				if (GetDlgItemText(hWndDlg, IDC_FMNEWSOURCE, txt, 32))
					EnableWindow(GetDlgItem(hWndDlg, IDC_FMADDSOURCE), TRUE);
				else
					EnableWindow(GetDlgItem(hWndDlg, IDC_FMADDSOURCE), FALSE);
				break;
			}
			break;
		case IDC_FMADDSOURCE:
			GetDlgItemText(hWndDlg, IDC_FMNEWSOURCE, txt, 32);
			AddFMSource(txt);
			SetDlgItemText(hWndDlg, IDC_FMNEWSOURCE, "");
			ListFMSource(hWndDlg);
			break;
		}
		break;    /* End of WM_COMMAND                                 */
	default:
		return FALSE;
	}
	return TRUE;
}
void GMFileManager(LPSTR ManagerFile)
{
	strcpy(manFile, ManagerFile);
	int nRc = DialogBox(hInst, (LPSTR)"FILEMANAGER", hWndMain,(DLGPROC) FileManagerMsgProc);
}
