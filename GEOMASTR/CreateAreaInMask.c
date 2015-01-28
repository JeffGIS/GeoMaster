#include "graphint.h"
#include "extrndb.h"

#include "gmextern.h"

BOOL ThemeCreateAreaInMask(int from)
{
	BOOL rtn = FALSE;
	LPVIEWPORT CurViewSave = CurView;

	if (CurTheme->TargetViewport)
		SetViewport(CurTheme->TargetViewport);

	if (CurrentType == GF_AREA && !from && CurView->PassID == 2)
	{
		nPnts /= 2;
		rtn = TRUE;
	}
	CurView = CurViewSave;
	return rtn;
}

BOOL FAR PASCAL AreaInMaskThemeMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	int	BRtn;
	if ((BRtn = DIALOGSTYLEMsgProc(hWndDlg, Message, wParam, lParam)))
		return (BRtn);
	if (ThemeCommonCode(hWndDlg, Message, wParam, lParam, CurTheme->hThemeDB))
	{
		return TRUE;
	}
	switch (Message)
	{
	case WM_INITDIALOG:

		cwCenter(hWndDlg, 0);
		break; /* End of WM_INITDIALOG                                 */

	case WM_CLOSE:
		/* Closing the Dialog behaves the same as Cancel               */
		PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
		break; /* End of WM_CLOSE                                      */

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hWndDlg, FALSE);
			break;
		case IDOK:
			EndDialog(hWndDlg, TRUE);
			break;
		}
		break;    /* End of WM_COMMAND                                 */

	default:
		return FALSE;
	}
	return TRUE;
}
