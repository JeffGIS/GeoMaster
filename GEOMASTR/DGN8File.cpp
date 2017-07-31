#include <windows.h>
#include <ctime>
#include <cmath>
#include <iostream>
#include <string>
#include <fstream>
#include "gmlimits.h"
#include "gssitype.h"
#include "gmextern.h"

using namespace std;


extern "C" BOOL OpenDGN8File(LPSTR InName, LPMNMXCORD pBounds)
{
	return TRUE;
}

extern "C" BOOL CloseDGN8File(void)
{
	BOOL	st = FALSE;
	return st;
}

extern "C" BOOL GetDGN8RecordBounds(DWORD Recno, LPMNMXCORD pBounds)
{
	MNMXCORD	TempBounds;
	BOOL rtn = FALSE;
	return rtn;
}

 BOOL IsDGN8FileVisible(void)
{
	return FALSE;
}

extern "C" BOOL SetDGN8Vis(HWND hWndDlg, int DlgItemSym, int DlgItemPar, HFILE FidSymList)
{
	return TRUE;
}

extern "C" BOOL SetDGN8Parms()
{
	BOOL rtn = FALSE;
	return rtn;
}

extern "C" BOOL ProcessDGN8Record(HDC hDC, long Recno)
{
	return FALSE;
}
extern "C" BOOL ReadNextDGN8Record(LPMNMXCORD pBounds)
{
	BOOL	rtn = FALSE;

	return rtn;
}

