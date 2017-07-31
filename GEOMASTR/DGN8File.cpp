#include "graphint.h"
#include "gmextern.h"

BOOL OpenDGN8File(LPSTR InName, LPMNMXCORD pBounds)
{
	return TRUE;
}

BOOL CloseDGN8File(void)
{
	BOOL	st = FALSE;
	return st;
}


BOOL ReadNextDGN8Record(LPMNMXCORD pBounds)
{
	BOOL	rtn = FALSE;

	return rtn;
}

BOOL GetDGN8RecordBounds(DWORD Recno, LPMNMXCORD pBounds)
{
	MNMXCORD	TempBounds;
	BOOL rtn = FALSE;
	return rtn;
}

BOOL IsDGN8FileVisible(void)
{
	return FALSE;
}

BOOL SetDGN8Vis(HWND hWndDlg, int DlgItemSym, int DlgItemPar, HFILE FidSymList)
{
	return TRUE;
}

BOOL SetDGN8Parms()
{
	BOOL rtn = FALSE;
	return rtn;
}

BOOL ProcessDGN8Record(HDC hDC, long Recno)
{
	return FALSE;
}
