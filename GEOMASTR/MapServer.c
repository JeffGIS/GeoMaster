#include "graphint.h"   

#include "gmextern.h"
#include <process.h>

#define MAX_MAPSERVERS	8
static char MapServerFile[MAX_MAPSERVERS][MAX_PATH];
static HWND MapServerWnd[MAX_MAPSERVERS] = { 0 };
static int	nMapServers = 0;
static HANDLE hThread = 0;

BOOL MergeImageIntoViewport2(HBITMAP hNewBitmap,RECT rect,LPSTR title,int textFade);
void __cdecl BackgroundMergeImageIntoViewport(LPHANDLE phArgs);

static int continueBackgroundMerge;

int GetAvailableServerID(void)
{
	int i;

	for (i = 0; i < MAX_MAPSERVERS; i++)
	{
		if (!MapServerWnd[i])
			return i;
	}
	return -1;
}

int GetMapserverIDFromWnd(HWND hWnd)
{
	int serverID;

	for (serverID = 0; serverID < nMapServers; serverID++)
	{
		if (hWnd == MapServerWnd[serverID])
			return serverID;
	}
	return -1;
}

HWND StartBackgroundMapServer(HWND hWnd,LPSTR config,LPSTR command,LPRECT pRect)
{
	char modulePath[MAX_PATH];

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD	CRFlags = 0;
	char	cmd[1024];
	char	startDir[MAX_PATH]="[%DL]";
	HWND	hWndServer = 0;
	int		serverID;
	LPSTR	pDot;

	if (nMapServers >= MAX_MAPSERVERS)
		return 0;
	nMapServers++;
	serverID = GetAvailableServerID();
	CloseAllRequestedFiles(FALSE);
	GetModuleFileName(NULL, modulePath, MAX_PATH);
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	si.dwFlags = STARTF_FORCEONFEEDBACK | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	CRFlags = DETACHED_PROCESS | BELOW_NORMAL_PRIORITY_CLASS;
	sprintf(cmd, "MapServer %s",config);
	GSSiGetTempFileName(0, "gms", 0, (LPSTR)MapServerFile[serverID]);
	pDot = strchr(MapServerFile[serverID], '.');
	if (pDot)
		strcpy(pDot, ".bmp");
	sprintf(strchr(cmd, 0), " /MAPSERVER %i '%s'", (int)hWnd,MapServerFile[serverID]);

	if (pRect)
		sprintf(strchr(cmd, 0), " /RECT %i %i %i %i", pRect->left, pRect->top, pRect->right, pRect->bottom);
	if (command && *command)
		sprintf(strchr(cmd, 0), " %s", command);
	if (*TestFileLocation)
		sprintf(strchr(cmd, 0), " [%%TESTDL]=%s;", TestFileLocation);
	ExpandText(startDir);
	if (*LastChr(startDir) == '\\')
		*LastChr(startDir) = 0;
	ExpandText(cmd);
	if (CreateProcess(modulePath, cmd,
		NULL,             // Process handle not inheritable. 
		NULL,             // Thread handle not inheritable. 
		FALSE,            // Set handle inheritance to FALSE. 
		CRFlags,		  // creation flags. 
		NULL,             // Use parent's environment block. 
		NULL,             // Use parent's starting directory. 
		&si,              // Pointer to STARTUPINFO structure.
		&pi)             // Pointer to PROCESS_INFORMATION structure.
		)
	{
		BOOL TimedOut;
		//int	 MaxWait = atol(Arg[5]);
		DWORD	ProcessID = GetProcessId(pi.hProcess);

		Wait (1000);

		if (!WaitForInputIdle(pi.hProcess, 18000))
		{
			hWndServer = MapServerWnd[serverID] = FindWindowByProcessID(ProcessID, "");
		}
	}
	return hWndServer;
}

void StopBackgroundMapServer(HWND hWndServer)
{
	int	serverID = GetMapserverIDFromWnd(hWndServer);

	if (serverID >= 0)
	{
		PostMessage(hWndServer,WM_CLOSE,0, 0);
		GSSiRemove(MapServerFile[serverID]);
		MapServerWnd[serverID] = 0;
	}
	return;
}

BOOL SendBackgroundMapServerCommand(HWND hWnd, HWND hBackGroundServer, LPSTR cmd,LPARAM id)
{
	HFILE Fid;
	OFSTRUCTGM OFStruct;
	int	serverID = GetMapserverIDFromWnd (hBackGroundServer);
		
	if (serverID < 0)
		return FALSE;
	Fid = OpenFileGM(MapServerFile[serverID], &OFStruct, OF_CREATE);
	_lwrite(Fid, cmd, strlen(cmd) + 1);
	_lclose(Fid);
	PostMessage(hBackGroundServer, GF_MAPSERVER_REQUEST,(WPARAM) hWnd, id);

	return TRUE;
}

BOOL GetMapserverFileName(HWND hBackGroundServer, LPSTR name)
{
	int	serverID = GetMapserverIDFromWnd(hBackGroundServer);

	*name = 0;
	if (serverID < 0)
		return FALSE;
	strcpy(name, MapServerFile[serverID]);
	return TRUE;
}

BOOL CopyMapserverFileToFile(HWND hBackGroundServer, LPSTR File)
{
	int	serverID = GetMapserverIDFromWnd(hBackGroundServer);

	if (serverID < 0)
		return FALSE;
	if (CopyFile(MapServerFile[serverID], File, FALSE))
		return TRUE;
	return FALSE;
}


BOOL MergeImageIntoViewport(HBITMAP hNewBitmap,LPRECT pRect, LPSTR title,int textFade)
{
	static HANDLE hArgs;
	LPSTR arg1;
	LPSTR arg2;
	LPSTR arg3;
	LPSTR arg4;

	if (!hNewBitmap)
	{
		continueBackgroundMerge = 0;
		return TRUE;
	}
	if (hThread)
	{
		int n = 0;
		continueBackgroundMerge = 0;
		while (n < 10 && hThread)
		{
			Sleep(100);
			n++;
		}
	}
	hArgs = GSSiGlobAlloc(9999, GMEM_MOVEABLE, 4096 * 5);
	arg1 = GlobalLock(hArgs);
	arg2 = arg1 + 4096;
	arg3 = arg2 + 4096;
	arg4 = arg3 + 4096;
	itoa((int)hNewBitmap, arg1, 10);
	recttoa(arg2, *pRect);
	strcpy(arg3, title);
	itoa(textFade, arg4, 10);

	GlobalUnlock(hArgs);
	continueBackgroundMerge = 1;

	hThread = (HANDLE)_beginthread(BackgroundMergeImageIntoViewport, 0, &hArgs);
	return TRUE;
}

void __cdecl BackgroundMergeImageIntoViewport(LPHANDLE phArgs)
{
	LPSTR	arg1 = GlobalLock(*phArgs);
	LPSTR	arg2 = arg1 + 4096, arg3 = arg2 + 4096, arg4 = arg3 + 4096;
	HBITMAP hNewBitmap = (HBITMAP)atoi(arg1);
	char	title[256];
	int		textFade=100;
	int		err;
	RECT	rect = atorect (arg2,&err);

	strcpy(title, arg3);
	textFade = atoi(arg4);


	GlobalUnlock(*phArgs);
	GSSiGlobFree(phArgs);
	MergeImageIntoViewport2(hNewBitmap,rect,title,textFade);
	return;
}

BOOL MergeImageIntoViewport2(HBITMAP hNewBitmap,RECT rect,LPSTR title,int textFade)
{
	BLENDFUNCTION bf;
	BOOL	rtn = TRUE;
	static	BOOL blt = FALSE;
	int transParency = 1, inc = 1;
	BOOL first = TRUE;


	if (hNewBitmap)
	{
		HDC hDC = GetDC (hWndMain);
		HDC	TransparentDC = CreateCompatibleDC(hDC);
		HDC	saveDC = CreateCompatibleDC(hDC);
		HDC	tempDC = CreateCompatibleDC(hDC);
		int w;
		int h;
		BITMAP bm;
		HBITMAP hbm=0, hBMSave=0;
		HBITMAP hBMOrig, hBMTemp, hBMTempOld;
		RECT	rect2, textRect;
		HFONT	OldFont;
		int		oldMode;
		int		height;
		float	factor;
		int		minTrans = 0;
		
		HFONT hFont = CreateFont(256, 0, 0, 0, FW_HEAVY, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");

		//GetClientRect(CurView->hWnd, &rect);
		rect2 = rect;
		OldFont = SelectObject(TransparentDC, hFont);
		height = DrawText(TransparentDC, title, -1, &rect2, DT_CALCRECT | DT_SINGLELINE | DT_LEFT);
		textRect.left = textRect.top = 0;
		textRect.bottom = RECTHEIGHT(&rect) - height/2;
		textRect.right = RECTWIDTH(&rect);
		SelectObject(TransparentDC, OldFont);
		DeleteObject(hFont);
		factor = 2*((float)RECTWIDTH(&rect) / (float)RECTWIDTH(&rect2)) / 3;
		hFont = CreateFont(256 * factor, 0, 0, 0, FW_HEAVY, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
		w = RECTWIDTH(&rect);
		h = RECTHEIGHT(&rect);
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = (0xff * transParency) / 100;  // half of 0xff = 50% transparency 
		bf.AlphaFormat = 0;// AC_SRC_ALPHA;   // use source alpha  
		SaveDC(hDC);
		SetGraphicsMode(hDC, GM_COMPATIBLE);
		SetMapMode(hDC, MM_TEXT);
		SetWindowOrgEx(hDC, 0, 0, 0);
		SetViewportOrgEx(hDC, 0, 0, 0);
		SelectClipRgn(hDC, 0);
		SetGraphicsMode(TransparentDC, GM_COMPATIBLE);
		SetMapMode(TransparentDC, MM_TEXT);
		SetWindowOrgEx(TransparentDC, 0, 0, 0);
		SetViewportOrgEx(TransparentDC, 0, 0, 0);
		SelectClipRgn(TransparentDC, 0);
		GetObject(hNewBitmap, sizeof(bm), (LPSTR)&bm);
		if (bm.bmBitsPixel == 32)
		{
			unsigned int irow, icol;
			int lenBits = bm.bmWidthBytes * bm.bmHeight;
			LPBYTE	pBits = malloc(lenBits);
			
			hBMSave = CreateCompatibleBitmap(hDC, bm.bmWidth, bm.bmHeight);
			hBMTemp = CreateCompatibleBitmap(hDC, bm.bmWidth, bm.bmHeight);
			hBMTempOld = SelectObject(tempDC, hBMTemp);
			GetBitmapBits(hNewBitmap, lenBits, pBits);
			SetBitmapBits(hBMSave, lenBits, pBits);
			hbm = SelectObject(saveDC, hBMSave);
			for (irow = max(0, rect.top); irow<min(rect.bottom, bm.bmHeight); irow++)
			{
				LPRGBQUAD pclr = (LPRGBQUAD)(pBits + bm.bmWidthBytes*irow);
				int firstCol = max(0, rect.left);

				pclr += firstCol;

				for (icol = firstCol; icol<min(bm.bmWidth, rect.right); icol++)
				{
					if (pclr->rgbBlue == 0 && pclr->rgbGreen == 0 && pclr->rgbRed == 0)
						pclr->rgbReserved = 0;
					else
						pclr->rgbReserved = 255;
					pclr++;
				}
			}
			//SetBitmapBits(hNewBitmap, lenBits, pBits);
			free(pBits);
		}
		hBMOrig = SelectObject(TransparentDC, hNewBitmap);
		SetTextColor(TransparentDC, RGB(255, 0,0));
		SetBkColor(TransparentDC, 0);
		oldMode = SetBkMode(TransparentDC, TRANSPARENT);
		OldFont = SelectObject(TransparentDC, hFont);
		DrawText(TransparentDC, title, -1, &textRect, DT_SINGLELINE | DT_CENTER | DT_BOTTOM);
		SelectObject(TransparentDC, OldFont);
		SetBkMode(TransparentDC, OPAQUE);
		if (blt)
		{
			if (!BitBlt(hDC, rect.left, rect.top, w, h,
				TransparentDC, rect.left, rect.top, SRCCOPY))
				ii = GetLastError();
		}
		else while (transParency > minTrans && transParency < 100)
		{
			if (continueBackgroundMerge != 1)
				break;
			w = RECTWIDTH(&rect);
			h = RECTHEIGHT(&rect);
			bf.SourceConstantAlpha = (0xff * transParency) / 100; 
			/*{
			HBITMAP hbm = SaveScreen (hDC,rect);
			SaveBitmap (hbm,"c:\\temp\\blendpre.bmp",0,0);
			GSSiDeleteObject (&hbm);
			}*/
			if (!first)
			{
				if (dbug)
				BitBlt(hDC,rect.left, rect.top, w, h,
					TransparentDC, rect.left, rect.top, SRCCOPY);
				BitBlt(tempDC, 0,0, w, h,
					saveDC,0,0, SRCCOPY);
				AlphaBlend(tempDC, 0,0, w, h,
					TransparentDC,
					0,0,
					w, h,
					bf);
				BitBlt(hDC, rect.left, rect.top, w, h,
					tempDC, 0,0, SRCCOPY);
			}
			else
				AlphaBlend(hDC, rect.left, rect.top, w, h,
				TransparentDC,
				0, 0,
				w, h,
				bf);
			/*{
				int er = GetLastError();
				char message[256];
				GetSystemErrMessage(er, message);
				rtn = FALSE;
			}*/
			transParency += inc;
			if (transParency >= 100)
			{
				if (first)
				{
					OldFont = SelectObject(TransparentDC, hFont);
					
					first = FALSE;
					transParency--;
					inc = -3;
					minTrans = textFade;
					FillRect(TransparentDC, &rect, GetStockObject (BLACK_BRUSH));
					if (dbug)
						BitBlt(hDC, rect.left, rect.top, w, h,
						TransparentDC, rect.left, rect.top, SRCCOPY);
					/*hNewBitmap = SelectObject(TransparentDC, hBMOrig);
					GetObject(hNewBitmap, sizeof(bm), (LPSTR)&bm);
					if (bm.bmBitsPixel == 32)
					{
						int lenBits = bm.bmWidthBytes * bm.bmHeight;
						LPBYTE	pBits = malloc(lenBits);
						GetBitmapBits(hNewBitmap, lenBits, pBits);
						memset(pBits, 0, lenBits);
						SetBitmapBits(hNewBitmap, lenBits, pBits);
						free(pBits);
					}
					hBMOrig = SelectObject(TransparentDC, hNewBitmap);*/
					SetTextColor(TransparentDC, RGB(255, 0, 0));
					SetBkColor(TransparentDC, 0);
					DrawText(TransparentDC, title, -1, &textRect, DT_SINGLELINE | DT_CENTER | DT_BOTTOM);
					SelectObject(TransparentDC,OldFont);
					if (dbug)
						BitBlt(hDC, rect.left, rect.top, w, h,
						TransparentDC, rect.left, rect.top, SRCCOPY);


					hNewBitmap = SelectObject(TransparentDC, hBMOrig);
					GetObject(hNewBitmap, sizeof(bm), (LPSTR)&bm);
					if (bm.bmBitsPixel == 32)
					{
						unsigned int irow, icol;
						int lenBits = bm.bmWidthBytes * bm.bmHeight;
						LPBYTE	pBits = malloc(lenBits);
						GetBitmapBits(hNewBitmap, lenBits, pBits);
						for (irow = 0; irow<min(rect.bottom, bm.bmHeight); irow++)
						{
							LPRGBQUAD pclr = (LPRGBQUAD)(pBits + bm.bmWidthBytes*irow);
							int firstCol = 0;

							pclr += firstCol;

							for (icol = firstCol; icol<min(bm.bmWidth, rect.right); icol++)
							{
								if (pclr->rgbBlue == 0 && pclr->rgbGreen == 0 && pclr->rgbRed == 0)
									pclr->rgbReserved = 0;// (0xff * transParency) / 100;
								else
									pclr->rgbReserved = 255;// (0xff * transParency) / 100;
								pclr++;
							}
						}
						SetBitmapBits(hNewBitmap, lenBits, pBits);
						free(pBits);
						bf.AlphaFormat =  AC_SRC_ALPHA;
					}
					hBMOrig = SelectObject(TransparentDC, hNewBitmap);
					if (dbug)
						BitBlt(hDC, rect.left, rect.top, w, h,
						TransparentDC, rect.left, rect.top, SRCCOPY);
				}
			}
			else
				Sleep(15);
		}
		/*			{
		HBITMAP hbm = SaveScreen (hDC,rect);
		SaveBitmap (hbm,"c:\\temp\\blendpost.bmp",0,0);
		GSSiDeleteObject (&hbm);
		}*/
		RestoreDC(hDC, -1);
		SelectObject(TransparentDC, hBMOrig);
		SelectObject(tempDC, hBMTempOld);
		DeleteDC(tempDC);
		DeleteDC (TransparentDC);
		DeleteObject(hNewBitmap);
		DeleteObject(hBMTemp);
		if (hbm)
		{
			SelectObject(saveDC, hbm);
			DeleteObject(hBMSave);
		}
		DeleteDC(saveDC);
		DeleteObject(hFont);
		GdiFlush();
		//wantnextblt();
		hThread = 0;
		//ReleaseDC (CurView->hWnd,hDC);
	}
	return TRUE;
}

HBITMAP GetHiddenWindowBitmap(HWND hwnd)
{
	// Takes a snapshot of the window hwnd, stored in the memory device context hdcMem
	HBITMAP hbitmap = 0;
	HDC hdc = GetWindowDC(hwnd);
	if (hdc)
	{
		HDC hdcMem = CreateCompatibleDC(hdc);
		if (hdcMem)
		{
			RECT rc;
			GetWindowRect(hwnd, &rc);

			hbitmap = CreateCompatibleBitmap(hdc, RECTWIDTH(&rc), RECTHEIGHT(&rc));
			if (hbitmap)
			{
				SelectObject(hdcMem, hbitmap);

				PrintWindow(hwnd, hdcMem, 0);

				DeleteObject(hbitmap);
			}
			DeleteObject(hdcMem);
		}
		ReleaseDC(hwnd, hdc);
	}
	return hbitmap;
}