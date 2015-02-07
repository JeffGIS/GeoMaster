#include "graphint.h"
#include "extrndb.h"

#include "gmextern.h"

BOOL SaveAreasToFile(LPSTR FileName)
{
	short	pos = BT_FIRST;
	long	Refno;
	HIGHLIGHTDATA	HighlightData;
	long	nPnts;
	HANDLE	hPoly;
	HANDLE  hPolyPartLen;
	HPDPOINT	pPoints;
	HFILE	Fid;
	BOOL rtn = FALSE;
	int nLoops;
	LPINT pPartLen;

	Fid = GSSiOpenFile(FileName, 0, OF_CREATE);
	if (Fid != HFILE_ERROR)
	{
		while (!BT_FIND(hHighlight, (LPSTR)&Refno, pos, BT_ANY, (LPSTR)&HighlightData))
		{
			pos = BT_NEXT;
			if (HighlightData.PD.Type == 3)
			{
				if ((nLoops = GetPolyPointsWithParts((LPPICKDATAHEADER)&HighlightData.PD, &nPnts, &hPoly, &hPolyPartLen)))
				{
					LPMNMXCORD	pBounds = (LPMNMXCORD)GlobalLock(hPoly);
					BigWrite(Fid, &Refno, 4, -1);
					BigWrite(Fid, HighlightData.PD.UDI, 65, -1);
					BigWrite(Fid, pBounds, sizeof(MNMXCORD), -1);
					BigWrite(Fid, &nLoops, sizeof(int), -1);
					if (nLoops > 1)
					{
						pPartLen = GlobalLock(hPolyPartLen);
						BigWrite(Fid, pPartLen, sizeof(int)*nLoops, -1);
						GlobalUnlock(hPolyPartLen);
					}
					pPoints = (HPDPOINT)(pBounds + 1);
					BigWrite(Fid, (HPSTR)&nPnts, 4, -1);
					BigWrite(Fid, (HPSTR)pPoints, nPnts*sizeof(DPOINT), -1);
					GSSiGlobUlFree(&hPoly);
					GSSiGlobFree(&hPolyPartLen);
				}
			}
		}
		GSSiClose(Fid);
		rtn = TRUE;
	}
	return rtn;
}

static BOOL GetAreaFromFile(HFILE Fid,LPMNMXCORD pBounds,LPINT pnPnts,LPHANDLE phDPoints)
{
	int Refno;
	int nLoops;
	char UDI[66];
	HPDPOINT pPoints;

	if (Fid == HFILE_ERROR)
		return FALSE;
	if (!BigRead(Fid, &Refno, 4))
		return FALSE;
	BigRead (Fid,UDI, 65);
	BigRead (Fid, pBounds, sizeof(MNMXCORD));
	BigRead(Fid, &nLoops, sizeof(int));
	if (nLoops > 1)
	{
		HANDLE hPartLen = GSSiGlobAlloc(0, GMEM_MOVEABLE, nLoops*sizeof(int));
		LPINT pPartLen = GlobalLock(hPartLen);
		BigRead (Fid, pPartLen, sizeof(int)*nLoops);
		GSSiGlobUlFree (&hPartLen);
	}
	BigRead(Fid, (HPSTR)pnPnts, 4);
	*phDPoints = GSSiGlobAlloc(1798, GMEM_MOVEABLE, *pnPnts * sizeof(DPOINT)+4);
	pPoints = GlobalLock(*phDPoints);
	BigRead(Fid, (HPSTR)pPoints, *pnPnts*sizeof(DPOINT));
	GlobalUnlock(*phDPoints);
	return TRUE;
}

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
			MNMXCORD bounds, BMbounds, mareaBounds;
			int width, height;
			int maxdim = 1024;
			double fac;
			HBITMAP hBM, hBMOld;
			BITMAP	bm;
			HDC hDC, hDCMain;
			RECT rect;
			HBRUSH hOldBrush;
			HANDLE hTranWtoBM, hTranBMtoW;
			HANDLE hPoly;
			LPPOINT pPoly;
			BOOL savebm = TRUE;
			int i;
			static int nTest = 1;
			int nMareaPoints;
			HANDLE hMareaPoints;
			COLORREF blue = RGB(0, 0, 255);
			HPEN hBluePen;
			HBRUSH hBlueBrush;
			HFILE Fid = GSSiOpenFile(CurTheme->DataFile, 0, OF_READ);
			
			if (Fid != HFILE_ERROR)
			{
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
				rect.left = rect.bottom = 0;
				rect.right = width + 4;
				rect.top = height + 4;
				GetObject(hBM, sizeof(bm), (LPSTR)&bm);
				ReleaseDC(CurView->hWnd, hDCMain);
				hBMOld = SelectObject(hDC, hBM);
				SetMapMode(hDC, MM_ISOTROPIC);
				SetWindowOrgEx(hDC, 0, 0, 0);
				SetViewportOrgEx(hDC, 0, 0, 0);
				SetWindowExtEx(hDC, 1024, 1024, 0);
				SetViewportExtEx(hDC, 1024, 1024, 0);
				FillRect(hDC, &rect, GetStockObject(WHITE_BRUSH));
				BMbounds.xmn = 2;
				BMbounds.ymn = 2;
				BMbounds.xmx = 2 + width;
				BMbounds.ymx = 2 + height;
				hTranWtoBM = STRANBoundsToBounds(&bounds, &BMbounds);
				hTranBMtoW = STRANBoundsToBounds(&BMbounds, &bounds);

				SetROP2(hDC, 5);//or 10 for both
				hOldBrush = SelectObject(hDC, GetStockObject(BLACK_BRUSH));
				hOldPen = SelectObject(hDC, GetStockObject(BLACK_PEN));

				while (GetAreaFromFile(Fid, &mareaBounds, &nMareaPoints, &hMareaPoints))
				{
					if (IntersectBounds(&mareaBounds, &bounds, 0))
					{
						LPDPOINT pMareaPoints = GlobalLock(hMareaPoints);
						hPoly = GSSiGlobAlloc(0, GMEM_MOVEABLE, nMareaPoints * sizeof(POINT));
						pPoly = GlobalLock(hPoly);
						for (i = 0; i < nMareaPoints; i++)
							pPoly[i] = TRANDPointToPoint(&pMareaPoints[i], hTranWtoBM);
						Polygon(hDC, pPoly, nMareaPoints);
						GSSiGlobUlFree(&hPoly);
						GlobalUnlock(hMareaPoints);
					}
					GSSiGlobFree(&hMareaPoints);
				}
				GSSiClose(Fid);

				hBluePen = CreatePen(PS_SOLID, 1, blue);
				hBlueBrush = CreateSolidBrush(blue);
				SelectObject(hDC,hBlueBrush);
				SelectObject(hDC, hBluePen);
				hPoly = GSSiGlobAlloc(0, GMEM_MOVEABLE, nPnts * sizeof(POINT));
				pPoly = GlobalLock(hPoly);
				for (i = 0; i < nPnts; i++)
					pPoly[i] = TRANDPointToPoint(&lpDCurPoints[i], hTranWtoBM);
				Polygon(hDC, pPoly, nPnts);
				SelectObject(hDC, hOldBrush);
				SelectObject(hDC, hOldPen);
				DeleteObject(hBluePen);
				DeleteObject(hBlueBrush);
				GSSiGlobUlFree(&hPoly);
				SelectObject(hDC, hBMOld);
				DeleteDC(hDC);

				if (savebm)
				{
					char file[MAX_PATH];

					sprintf(file, "c:\\temp\\AreaTests\\test%i.bmp", nTest++);
					SaveBitmap(hBM, file, 0, 0);
				}
				GSSiDeleteObject(&hBM);
				CloseTRANS2(&hTranWtoBM);
				CloseTRANS2(&hTranBMtoW);

				//nPnts /= 2;
				rtn = TRUE;
			}
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
