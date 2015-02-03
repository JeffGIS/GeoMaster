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
		if (HiPrecis)
		{
			MNMXCORD bounds, BMbounds;
			int width, height;
			int maxdim = 1024;
			double fac;
			HBITMAP hBM, hBMOld;
			BITMAP	bm;
			HDC hDC, hDCMain;
			HBRUSH hOldBrush;
			HANDLE hTranWtoBM, hTranBMtoW;
			HANDLE hPoly;
			LPPOINT pPoly;
			BOOL savebm = TRUE;
			int i;

			GetPolyBoundsD2(lpDCurPoints, nPnts, &bounds, TYPE_AREA);
			fac = BoundsWidth(&bounds) / BoundsHeight(&bounds);
			if (fac > 1)
			{
				width = maxdim - 4;
				height = width / fac;
			}
			else
			{
				height = maxdim - 4;
				width = height * fac;
			}
			hDCMain = GetDC(CurView->hWnd);
			hDC = CreateCompatibleDC(hDCMain);
			//hBM = CreateBitmap(width+4, height+4, 1, 1, 0);
			hBM = CreateCompatibleBitmap(hDCMain, width + 4, height + 4);
			GetObject(hBM, sizeof(bm), (LPSTR)&bm);
			ReleaseDC(CurView->hWnd, hDCMain);
			hBMOld = SelectObject(hDC, hBM);
			SetMapMode(hDC, MM_ISOTROPIC);
			SetWindowOrgEx(hDC, 0, 0, 0);
			SetViewportOrgEx(hDC, 0, 0, 0);
			SetWindowExtEx(hDC, 1024, 1024, 0);
			SetViewportExtEx(hDC, 1024, 1024, 0);
			hOldBrush = SelectObject(hDC, hRedBrush);
			hOldPen = SelectObject(hDC, hRedPen);
			BMbounds.xmn = 2;
			BMbounds.ymn = 2;
			BMbounds.xmx = 2 + width;
			BMbounds.ymx = 2 + height;
			hTranWtoBM = STRANBoundsToBounds(&bounds, &BMbounds);
			hTranBMtoW = STRANBoundsToBounds(&BMbounds, &bounds);
			hPoly = GSSiGlobAlloc(0, GMEM_MOVEABLE, nPnts * sizeof(POINT));
			pPoly = GlobalLock(hPoly);
			for (i = 0; i < nPnts; i++)
				pPoly[i] = TRANDPointToPoint(&lpDCurPoints[i], hTranWtoBM);
			Polygon(hDC, pPoly, nPnts);
			GSSiGlobUlFree(&hPoly);
			SelectObject(hDC, hBMOld);
			DeleteDC(hDC);

			if (savebm)
				SaveBitmap(hBM, "c:\\temp\\test.bmp", 0, 0);
			GSSiDeleteObject(&hBM);
			CloseTRANS2(&hTranWtoBM);
			CloseTRANS2(&hTranBMtoW);

			//nPnts /= 2;
			rtn = TRUE;
		}
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
		SetDlgItemText(hWndDlg, IDC_MASKEDAREASFILE,CurTheme->DataFile);
		switch (CurTheme->DataType)
		{
		case 1:
			SendDlgItemMessage(hWndDlg, IDC_CMAP_LOW, (UINT)BM_SETCHECK, TRUE, (LPARAM)0L);
			break;
		case 2:
			SendDlgItemMessage(hWndDlg, IDC_CMAP_MEDIUM, (UINT)BM_SETCHECK, TRUE, (LPARAM)0L);
			break;
		case 3:
			SendDlgItemMessage(hWndDlg, IDC_CMAP_HIGH, (UINT)BM_SETCHECK, TRUE, (LPARAM)0L);
			break;
		}
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
			GetDlgItemText(hWndDlg, IDC_MASKEDAREASFILE, CurTheme->DataFile, MAX_PATH);
			if (SendDlgItemMessage(hWndDlg, IDC_CMAP_LOW, (UINT)BM_GETCHECK, (WPARAM)0, (LPARAM)0L))
				CurTheme->DataType = 1;
			else if (SendDlgItemMessage(hWndDlg, IDC_CMAP_MEDIUM, (UINT)BM_GETCHECK, (WPARAM)0, (LPARAM)0L))
				CurTheme->DataType = 2;
			else
				CurTheme->DataType = 3;
			EndDialog(hWndDlg, TRUE);
			break;
		}
		break;    /* End of WM_COMMAND                                 */

	default:
		return FALSE;
	}
	return TRUE;
}
