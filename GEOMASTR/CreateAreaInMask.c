#include "graphint.h"
#include "extrndb.h"

#include "gmextern.h"
static	int	xoff1[8] = { -1, 0, 1, 0, -1, 1, 1, -1 };
static	int	yoff1[8] = { 0, 1, 0, -1, 1, 1, -1, -1 };
static	int	xoff2[16] = { -2, -2, -2, -1, 0, 1, 2, 2, 2, 2, 2, 1, 0, -1, -2, -2 };
static	int yoff2[16] = { 0, 1, 2, 2, 2, 2, 2, 1, 0, -1, -2, -2, -2, -2, -2, -1 };
static	int	xoff3[24] = { -3, -3, -3, -3, -2, -1, 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3 };
static	int yoff3[24] = { 0, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -3, -3, -3, -3, -2, -1 };
static	int	xoff4[32] = { -4, -4, -4, -4, -4, -3, -2, -1, 0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0, -1, -2, -3, -4, -4, -4, -4 };
static	int yoff4[32] = { 0, 1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0, -1, -2, -3, -4, -4, -4, -4, -4, -4, -4, -4, -4, -3, -2, -1 };
static	int	xoff5[40] = { -5, -5, -5, -5, -5, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -5, -5, -5, -5 };
static	int yoff5[40] = { 0, 1, 2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 3, 2, 1, 0, -1, -2, -3, -4, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -4, -3, -2, -1 };

#define MAX_NEW_POLYGONS 64
#define MAX_NEW_POLY_POINTS USHRT_MAX * 4
static int GetNewPolygon(HBITMAP hBM, LPINT pnumNewPoints, LPHANDLE phNewPoints);

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

static int getMaxBitmapDimension(LPTHEME CurTheme)
{
	int rtn;
	switch (CurTheme->DataType)
	{
	case 1:
		rtn = 1024;
		break;
	default:
		rtn = 2048;
	case 3:
		rtn = 4096;
		break;
	}
	return rtn;
}
static saveBitmap(HDC hDC, HBITMAP hBMOld)
{
	static BOOL savebm = FALSE;
	static int nTest = 1;
	if (savebm)
	{
		char file[MAX_PATH];
		HBITMAP hBM = SelectObject(hDC, hBMOld);
		sprintf(file, "c:\\temp\\AreaTests\\test%i.bmp", nTest++);
		SaveBitmap(hBM, file, 0, 0);
		SelectObject(hDC, hBM);
	}
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
			int maxdim = getMaxBitmapDimension(CurTheme);
			int margin = 4;
			double fac;
			HBITMAP hBM, hBMOld;
			BITMAP	bm;
			HDC hDC, hDCMain;
			RECT rect;
			HBRUSH hOldBrush;
			HANDLE hTranWtoBM, hTranBMtoW;
			HANDLE hPoly;
			LPPOINT pPoly;
			int i;
			static int nTest = 1;
			int nMareaPoints;
			HANDLE hMareaPoints;
			COLORREF blue = RGB(0, 0, 255);
			HPEN hBluePen;
			HBRUSH hBlueBrush;
			int numNewPoints[MAX_NEW_POLYGONS];
			HANDLE hNewPoints[MAX_NEW_POLYGONS];
			int nNewPoly = 0;
			LPDPOINT pPixelPoints;
			HFILE Fid = GSSiOpenFile(CurTheme->DataFile, 0, OF_READ);
			
			if (Fid != HFILE_ERROR)
			{
				GetPolyBoundsD2(lpDCurPoints, nPnts, &bounds, TYPE_AREA);
				fac = BoundsWidth(&bounds) / BoundsHeight(&bounds);
				if (fac > 1)
				{
					width = maxdim - margin*2;
					height = width / fac;
				}
				else
				{
					height = maxdim - margin * 2;
					width = height * fac;
				}
				hDCMain = GetDC(CurView->hWnd);
				hDC = CreateCompatibleDC(hDCMain);
				//hBM = CreateBitmap(width+4, height+4, 1, 1, 0);
				hBM = CreateCompatibleBitmap(hDCMain, width + margin * 2, height + margin * 2);
				rect.left = rect.bottom = 0;
				rect.right = width + margin * 2;
				rect.top = height + margin * 2;
				GetObject(hBM, sizeof(bm), (LPSTR)&bm);
				ReleaseDC(CurView->hWnd, hDCMain);
				hBMOld = SelectObject(hDC, hBM);
				SetMapMode(hDC, MM_ISOTROPIC);
				SetWindowOrgEx(hDC, 0, 0, 0);
				SetViewportOrgEx(hDC, 0, 0, 0);
				SetWindowExtEx(hDC, width, width, 0);
				SetViewportExtEx(hDC, width, width, 0);
				FillRect(hDC, &rect, GetStockObject(WHITE_BRUSH));
				BMbounds.xmn = margin;
				BMbounds.ymn = margin;
				BMbounds.xmx = margin + width;
				BMbounds.ymx = margin + height;
				hTranWtoBM = STRANBoundsToBounds(&bounds, &BMbounds);
				hTranBMtoW = STRANBoundsToBounds(&BMbounds, &bounds);

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

				saveBitmap(hDC, hBMOld);

				SetROP2(hDC, 5);//or 10 for both
				hBluePen = CreatePen(PS_SOLID, 1, blue);
				hBlueBrush = CreateSolidBrush(blue);
				SelectObject(hDC,hBlueBrush);
				//SelectObject(hDC, hBluePen);
				SelectObject(hDC, GetStockObject(NULL_PEN));
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
				saveBitmap(hDC, hBMOld);
				SelectObject(hDC, hBMOld);
				DeleteDC(hDC);
				nNewPoly = GetNewPolygon(hBM, numNewPoints, hNewPoints);
				GSSiDeleteObject(&hBM);
				GSSiGlobUlFree(&hPolyBuffer);
				GSSiGlobFree(&hPolyPartLen);
				if (nNewPoly)
				{
					int totPoints = nNewPoly;
					int np = 0, j;
					for (i = 0; i < nNewPoly; i++)
						totPoints += numNewPoints[i];
					hPolyBuffer = GSSiGlobAlloc(1799, GMEM_MOVEABLE, sizeof(DPOINT)*totPoints + 4);
					lpDCurPoints = GlobalLock(hPolyBuffer);
					for (i = 0; i < nNewPoly; i++)
					{
						if (i)
							lpDCurPoints[np++] = lpDCurPoints[0];
						pPixelPoints = GlobalLock(hNewPoints[i]);
						for (j = 0; j < numNewPoints[i]; j++)
						{
							lpDCurPoints[np++] = TranPoint(&pPixelPoints[j], hTranBMtoW);
						}
						GlobalUnlock(hNewPoints[i]);
					}
					nPnts = np;
				}
				else
					nPnts = 0;
				nPolyPoints = nPnts;

				for (i = 0; i < nNewPoly; i++)
					GSSiGlobFree(&hNewPoints[i]);
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

static int bitIndex(BITMAP *pbm, int row, int col)
{
	int index = -1;
	
	if (row < 0 || row >= pbm->bmHeight)
		return -1;
	if (col < 0 || col > pbm->bmWidth)
		return -1;
	index = row * pbm->bmWidthBytes / 4;
	index += col;
	return index;
}
static BOOL edgeNode(int row, int col, BITMAP *pbm, LPCOLORREF pbits)
{
	BOOL rtn = FALSE;
	COLORREF blue = 255;
	int left = bitIndex(pbm, row, col - 1);
	int right= bitIndex(pbm, row, col + 1);
	int up   = bitIndex(pbm, row + 1, col);
	int down = bitIndex(pbm, row - 1, col);

	if (left < 0 || right < 0 || up < 0 || down < 0)
		rtn = TRUE;
	else
	{
		if (pbits[left] != pbits[right] &&
			(pbits[left] == blue || pbits[right] == blue))
			rtn = TRUE;
		if (pbits[up] != pbits[down] &&
			(pbits[up] == blue || pbits[down] == blue))
			rtn = TRUE;
	}
	return rtn;
}
static int findStartNode(LPINT prow, LPINT pcol, BITMAP *pbm, LPCOLORREF pbits)
{
	int width = pbm->bmWidthBytes / 4;
	int indx;

	for (*prow = 0; *prow < pbm->bmHeight; (*prow)++)
	{
		indx = *prow * width;
		for (*pcol = 0; *pcol < pbm->bmWidth; (*pcol)++, indx++)
			if (!pbits[indx])
				return indx;
	}
	return -1;
}

static int findNextNode(LPINT prow, LPINT pcol, BITMAP *pbm, LPCOLORREF pbits)
{
	int i;
	int indx;
	for (i = 0; i < 8; i++)
	{
		indx = bitIndex(pbm, *prow + yoff1[i], *pcol + xoff1[i]);
		if (indx >= 0 && !pbits[indx])
		{
			(*pcol) += xoff1[i];
			(*prow) += yoff1[i];
			return indx;
		}
	}
	//return -1;
	for (i = 0; i < 16; i++)
	{
		indx = bitIndex(pbm, *prow + yoff2[i], *pcol + xoff2[i]);
		if (indx >= 0 && !pbits[indx])
		{
			(*pcol) += xoff2[i];
			(*prow) += yoff2[i];
			return indx;
		}

	}

	for (i = 0; i < 24; i++)
	{
		indx = bitIndex(pbm, *prow + yoff3[i], *pcol + xoff3[i]);
		if (indx >= 0 && !pbits[indx])
		{
			(*pcol) += xoff3[i];
			(*prow) += yoff3[i];
			return indx;
		}

	}

	return -1;

}
static int GetNewPolygon(HBITMAP hBM,LPINT pnumNewPoints, LPHANDLE phNewPoints)
{
	BITMAP bm;
	LPRGBQUAD pbit;
	LPCOLORREF pbits, pbits2;
	int row, col;
	COLORREF white = RGB(255, 255, 255);
	COLORREF black = 0;
	COLORREF blue = 255;
	COLORREF green = RGB(0, 255, 0);
	int bmsize;
	int nPoly = 0;

		GetObject(hBM, sizeof(BITMAP), &bm);
		bmsize = bm.bmHeight * bm.bmWidthBytes;
		pbits = (LPCOLORREF)malloc(bmsize);
		pbits2 = (LPCOLORREF)malloc(bmsize);
		GetBitmapBits(hBM, bmsize, pbits);
		//find the edge points
		for (row = 0; row < bm.bmHeight; row++)
		{
			LPCOLORREF pRow = pbits + (row * bm.bmWidthBytes / 4);
			LPCOLORREF pRow2 = pbits2 + (row * bm.bmWidthBytes / 4);
			for (col = 0; col < bm.bmWidthBytes / 4; col++, pRow++, pRow2++)
			{
				*pRow2 = white;
				if (*pRow == blue)
				{
					if (edgeNode(row, col, &bm, pbits))
						*pRow2 = black;
				}
			}
		}
		//create the polygons
		{
			int indx;
			double Area;
			while ((indx = findStartNode(&row, &col, &bm, pbits2)) >= 0)
			{
				int nNodes = 0;
				int startrow = row, startcol = col;
				LPDPOINT pNewPoints;
				pbits2[indx] = blue;
				phNewPoints[nPoly] = GSSiGlobAlloc(0, GMEM_MOVEABLE, sizeof(DPOINT)*MAX_NEW_POLY_POINTS);
				pNewPoints = GlobalLock(phNewPoints[nPoly]);
				pNewPoints[nNodes].x = col;
				pNewPoints[nNodes++].y = row;
				while ((indx = findNextNode(&row, &col, &bm, pbits2)) >= 0)
				{
					pbits2[indx] = blue;
					pNewPoints[nNodes].x = col;
					pNewPoints[nNodes++].y = row;
				}
				Area = ComputeAreaAreaD(pNewPoints, nNodes, 0);
				if (fabs(Area) > 10 && nNodes > 2 && max(abs(startrow - row), abs(startcol - col)) < 3)
				{
					GlobalUnlock(phNewPoints[nPoly]);
					phNewPoints[nPoly] = GSSiGlobalReAlloc(0, phNewPoints[nPoly], nNodes*sizeof(DPOINT), GMEM_MOVEABLE);

					pnumNewPoints[nPoly++] = nNodes;
				}
				else
					GSSiGlobUlFree(&phNewPoints[nPoly]);
			}
		}
		free(pbits);
		free(pbits2);
		return nPoly;
}

void testConvertBitmapToPoly(LPSTR file)
{
	char outFile[MAX_PATH];
	HDIB32 hDib32 = GMFIBMPHandleFromEXT(file);
	BITMAP bm;
	LPRGBQUAD pbit;
	LPCOLORREF pbits, pbits2;
	int row, col;
	COLORREF white = RGB(255, 255, 255);
	COLORREF black = 0;
	COLORREF blue = 255;
	COLORREF green = RGB(0, 255, 0);
	int bmsize;
	int nPoly = 0;
	if (hDib32)
	{
		HBITMAP hBM = DIB32ToBitmap(hDib32, (HPALETTE)0);
		DestroyDIB32(hDib32, FALSE);
		GetObject(hBM,sizeof(BITMAP) , &bm);
		bmsize = bm.bmHeight * bm.bmWidthBytes;
		pbits = (LPCOLORREF)malloc(bmsize);
		pbits2 = (LPCOLORREF)malloc(bmsize);
		GetBitmapBits(hBM, bmsize, pbits);
		//find the edge points
		for (row = 0; row < bm.bmHeight; row++)
		{
			LPCOLORREF pRow = pbits + (row * bm.bmWidthBytes / 4);
			LPCOLORREF pRow2 = pbits2 + (row * bm.bmWidthBytes / 4);
			for (col = 0; col < bm.bmWidthBytes / 4; col++, pRow++,pRow2++)
			{
				*pRow2 = white;
				if (*pRow == blue)
				{
					if (edgeNode(row,col,&bm,pbits))
						*pRow2 = black;
				}
			}
		}
		SetBitmapBits(hBM, bmsize, pbits2);
		strcpy(outFile, file);
		REPLAC(outFile, ".bmp", "edge.bmp",MAX_PATH);
		SaveBitmap(hBM, outFile, 0, 0);
		//create the polygons
		{
			int indx;
			while ((indx = findStartNode(&row, &col, &bm, pbits2))>=0)
			{
				int nNodes = 0;
				int startrow = row, startcol = col;
				pbits2[indx] = blue;
				while ((indx = findNextNode(&row, &col, &bm, pbits2)) >= 0)
				{
					pbits2[indx] = blue;
					nNodes++;
				}
				SetBitmapBits(hBM, bmsize, pbits2);
				strcpy(outFile, file);
				REPLAC(outFile, ".bmp", "poly.bmp", MAX_PATH);
				SaveBitmap(hBM, outFile, 0, 0);
				if (nNodes > 2 && max(abs(startrow - row), abs(startcol - col)) < 3)
					nPoly++;
			}
		}
		free(pbits);
		free(pbits2);
	}
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
		default:
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
