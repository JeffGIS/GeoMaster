#include "graphint.h"
#include <CommCtrl.h>
#include "gmextern.h"

#define MAX_DATED_ORTHOS	64

static  char		OrthoDateFile[MAX_PATH];
static	char		datedOrthoFile[MAX_DATED_ORTHOS][MAX_PATH];
static	char		orthoDates[MAX_DATED_ORTHOS][32];
static	char		orthoTitles[MAX_DATED_ORTHOS][32];
static  int			nDatesSelected = 0;
static  int			nDatesReturned, iDate;
#define TIME_MANUAL	USER_TIMER_MAXIMUM
#define TIME_FAST	5000
#define TIME_MEDIUM 7500
#define TIME_SLOW	10000
static  int			timeBetweenDates = TIME_MEDIUM;
#define FADE_FULL	0
#define FADE_PARTIAL 35
#define FADE_NONE	100
static	int			textFade = FADE_FULL;
static UINT			timerID = 0;
static HWND			hWndDatedOrthos;
static BOOL			loopDates = TRUE;
static BOOL			sequentialMethod = TRUE;


BOOL FAR PASCAL PlayOrthosMESSAGEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	SCROLLINFO scrollInfo;
	int	BRtn;
	int i;
	static int leaveCounter;
	static int vpid;
#define NBITMAPS	5
	static HANDLE hBM[NBITMAPS] = { 0 };
	if ((BRtn = DIALOGSTYLEMsgProc(hWndDlg, Message, wParam, lParam)))
		return (BRtn);
	if ((BRtn = CloseWhenCursorLeavesMsgProc(hWndDlg, Message, wParam, lParam, &leaveCounter)))
		return (BRtn);
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		POINT pt;
		RECT  rect;

		vpid = CurView->ID;
		cwCenter(hWndDlg, -4);
		GetWindowRect(hWndDlg, &rect);
		pt = RectMid(&rect);
		SetCursorPos(pt.x, pt.y);
		if (timeBetweenDates == TIME_MANUAL)
		{
			ShowWindow(GetDlgItem(hWndDlg, IDC_PLAY), SW_SHOW);
			ShowWindow(GetDlgItem(hWndDlg, IDC_PAUSE), SW_HIDE);
			ShowWindow(GetDlgItem(hWndDlg, IDC_SLIDER1), SW_SHOW);
			EnableWindow(GetDlgItem(hWndDlg, IDC_SLIDER1), FALSE);
		}
		else
		{
			ShowWindow(GetDlgItem(hWndDlg, IDC_PLAY), SW_HIDE);
			ShowWindow(GetDlgItem(hWndDlg, IDC_PAUSE), SW_SHOW);
			ShowWindow(GetDlgItem(hWndDlg, IDC_SLIDER1), SW_SHOW);
			EnableWindow(GetDlgItem(hWndDlg, IDC_SLIDER1), TRUE);
		}
		hBM[0] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LEFT_ARROW));
		SetBitmapSizeToButton(GetDlgItem(hWndDlg, IDC_PRIOR),(HBITMAP*) &hBM[0]);
		hBM[1] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_RIGHT_ARROW));
		SetBitmapSizeToButton(GetDlgItem(hWndDlg, IDC_NEXT), (HBITMAP*)&hBM[1]);
		hBM[2] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PLAY_START));
		SetBitmapSizeToButton(GetDlgItem(hWndDlg, IDC_PLAY), (HBITMAP*)&hBM[2]);
		hBM[3] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PLAY_PAUSE));
		SetBitmapSizeToButton(GetDlgItem(hWndDlg, IDC_PAUSE), (HBITMAP*)&hBM[3]);
		hBM[4] = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_CANCEL));
		SetWindowSizeToBitmap(GetDlgItem(hWndDlg, IDCANCEL), (HBITMAP)hBM[4]);
		SendDlgItemMessage(hWndDlg, IDC_PRIOR, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBM[0]);
		SendDlgItemMessage(hWndDlg, IDC_NEXT, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBM[1]);
		SendDlgItemMessage(hWndDlg, IDC_PLAY, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBM[2]);
		SendDlgItemMessage(hWndDlg, IDC_PAUSE, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBM[3]);
		SendDlgItemMessage(hWndDlg, IDCANCEL, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBM[4]);
		SendDlgItemMessage(hWndDlg, IDC_SLIDER1, TBM_SETPOS, TRUE, 50);

	}
		break; /* End of WM_INITDIALOG                                 */

	case WM_DESTROY:
		for (i = 0; i < NBITMAPS; i++)
			GSSiDeleteObject(&hBM[i]);
		break;
	case WM_CLOSE:
		/* Closing the Dialog behaves the same as Cancel               */
		PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
		break; /* End of WM_CLOSE                                      */

		case  WM_HSCROLL:
			if (LOWORD(wParam) == SB_ENDSCROLL)
			{
				timeBetweenDates = 2000 + 100 * (100 - SendDlgItemMessage(hWndDlg, IDC_SLIDER1, TBM_GETPOS, 0, 0));
				timerID = SetTimer(hWndDatedOrthos, MAKELPARAM(GF_DISPLAY_DATED_ORTHOS, vpid), 1, 0);
			}
		break;


	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			//DestroyWindow(hWndDlg);
			SetViewport(vpid);
			PostMessage(hWndMain, GF_CLOSE, 0, 0L);

			break;

		case IDC_PRIOR:
			if (iDate > 1)
				iDate -= 2;
			else if (iDate)
				iDate = nDatesSelected - 1;
			else
				iDate = nDatesSelected - 2;
			timerID = SetTimer(hWndDatedOrthos, MAKELPARAM(GF_DISPLAY_DATED_ORTHOS, vpid), 1, 0);
			break;
		case IDC_NEXT:
			if (iDate >= nDatesSelected)
				iDate = 0;
			timerID = SetTimer(hWndDatedOrthos, MAKELPARAM(GF_DISPLAY_DATED_ORTHOS, vpid), 1, 0);
			break;
		case IDC_PLAY:
			timerID = SetTimer(hWndDatedOrthos, MAKELPARAM(GF_DISPLAY_DATED_ORTHOS, vpid), 1, 0);
			ShowWindow(GetDlgItem(hWndDlg, IDC_PLAY), SW_HIDE);
			ShowWindow(GetDlgItem(hWndDlg, IDC_PAUSE), SW_SHOW);
			EnableWindow(GetDlgItem(hWndDlg, IDC_SLIDER1), TRUE);
			timeBetweenDates = 2000 + 100 * (100 - SendDlgItemMessage(hWndDlg, IDC_SLIDER1, TBM_GETPOS, 0, 0));
			break;
		case IDC_PAUSE:
			KillTimer(hWndDatedOrthos, timerID);
			ShowWindow(GetDlgItem(hWndDlg, IDC_PLAY), SW_SHOW);
			ShowWindow(GetDlgItem(hWndDlg, IDC_PAUSE), SW_HIDE);
			EnableWindow(GetDlgItem(hWndDlg, IDC_SLIDER1), FALSE);
			break;
		}
		break;    /* End of WM_COMMAND                                 */

	default:
		return FALSE;
	}
	return TRUE;
}

BOOL FAR PASCAL SelectOrthosMESSAGEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	HFILE	fid;
	char	str[1024];
	int		BRtn, i;
	int		TabStops[2] = { 2000, 2100 };
	if ((BRtn = DIALOGSTYLEMsgProc(hWndDlg, Message, wParam, lParam)))
		return (BRtn);

	switch (Message)
	{
	case WM_INITDIALOG:

		cwCenter(hWndDlg, 0);
		SendDlgItemMessage(hWndDlg, IDC_SEQUENTIAL, BM_SETCHECK, sequentialMethod, 0L);
		switch (timeBetweenDates)
		{
		case TIME_MANUAL:
			SendDlgItemMessage(hWndDlg, IDC_SPEED_0, BM_SETCHECK, TRUE, 0L);
			break;
		case TIME_SLOW:
			SendDlgItemMessage(hWndDlg, IDC_SPEED_1, BM_SETCHECK, TRUE, 0L);
			break;
		default:
		case TIME_MEDIUM:
			SendDlgItemMessage(hWndDlg, IDC_SPEED_2, BM_SETCHECK, TRUE, 0L);
			break;
		case TIME_FAST:
			SendDlgItemMessage(hWndDlg, IDC_SPEED_3, BM_SETCHECK, TRUE, 0L);
			break;
		}
		switch (textFade)
		{
		case FADE_FULL:
			SendDlgItemMessage(hWndDlg, IDC_FADE_FULL, BM_SETCHECK, TRUE, 0L);
			break;
		case FADE_PARTIAL:
			SendDlgItemMessage(hWndDlg, IDC_FADE_PARTIAL, BM_SETCHECK, TRUE, 0L);
			break;
		case FADE_NONE:
			SendDlgItemMessage(hWndDlg, IDC_FADE_NONE, BM_SETCHECK, TRUE, 0L);
			break;
		}
		SendDlgItemMessage(hWndDlg, IDC_LOOP, BM_SETCHECK, loopDates, 0L);

		fid = GSSiOpenFile(OrthoDateFile, 0, OF_READ);
		if (fid == HFILE_ERROR)
		{
			sprintf(str, "Unable to open aerial photo date file:\r%s", OrthoDateFile);
			MessageBox(hWndDlg, str, 0, MB_ICONEXCLAMATION);
			PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
		}
		SendDlgItemMessage(hWndDlg, IDC_LIST, LB_SETTABSTOPS, 2, (LPARAM)&TabStops);
		while (fgetstring(str, 255, fid))
		{
			LPSTR pEnd = strchr(str, '|');
			if (pEnd)
			{
				*pEnd = '\t';
				SendDlgItemMessage(hWndDlg, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)str);
			}
		}
		GSSiClose(fid);
		for (i = 0; i < nDatesSelected; i++)
		{
			int item = SendDlgItemMessage(hWndDlg, IDC_LIST, LB_FINDSTRING, -1, (LPARAM)orthoTitles[i]);
			SendDlgItemMessage(hWndDlg, IDC_LIST, LB_SETSEL, TRUE, (LPARAM)item);
		}
		EnableWindow(GetDlgItem(hWndDlg, IDOK), SendDlgItemMessage(hWndDlg, IDC_LIST, LB_GETSELCOUNT, 0, 0) > 0);

		break; /* End of WM_INITDIALOG                                 */


	case WM_CLOSE:
		/* Closing the Dialog behaves the same as Cancel               */
		PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
		break; /* End of WM_CLOSE                                      */

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_LIST:
			EnableWindow(GetDlgItem(hWndDlg, IDOK), SendDlgItemMessage(hWndDlg, IDC_LIST, LB_GETSELCOUNT, 0, 0) > 0);
			break;

		case IDC_SELECT_ALL:
			SendDlgItemMessage(hWndDlg, IDC_LIST, LB_SETSEL, SendDlgItemMessage(hWndDlg, IDC_SELECT_ALL, BM_GETCHECK, 0, 0L), (LPARAM)-1);
			EnableWindow(GetDlgItem(hWndDlg, IDOK), SendDlgItemMessage(hWndDlg, IDC_LIST, LB_GETSELCOUNT, 0, 0) > 0);
			break;

		case IDCANCEL:
			EndDialog(hWndDlg, FALSE);
			break;
		case IDOK:
		{
			HANDLE	hItems;
			LPINT	pItems;
			LPSTR	pTab;

			nDatesSelected = GetLBSelectedItems(hWndDlg, IDC_LIST, &hItems);
			if (!nDatesSelected)
				break;
			pItems = (LPINT)GlobalLock(hItems);
			for (i = 0; i < nDatesSelected; i++)
			{
				SendDlgItemMessage(hWndDlg, IDC_LIST, LB_GETTEXT, pItems[i], (LPARAM)str);
				pTab = strchr(str, '\t');
				*pTab++ = 0;
				strcpy(orthoDates[i], pTab);
				strcpy(orthoTitles[i], str);
			}
			GSSiGlobUlFree(&hItems);
			if (SendDlgItemMessage(hWndDlg, IDC_SPEED_1, BM_GETCHECK, 0, 0L))
				timeBetweenDates = TIME_SLOW;
			if (SendDlgItemMessage(hWndDlg, IDC_SPEED_2, BM_GETCHECK, 0, 0L))
				timeBetweenDates = TIME_MEDIUM;
			if (SendDlgItemMessage(hWndDlg, IDC_SPEED_3, BM_GETCHECK, 0, 0L))
				timeBetweenDates = TIME_FAST;
			if (SendDlgItemMessage(hWndDlg, IDC_FADE_FULL, BM_GETCHECK, 0, 0L))
				textFade = FADE_FULL;
			if (SendDlgItemMessage(hWndDlg, IDC_FADE_PARTIAL, BM_GETCHECK, 0, 0L))
				textFade = FADE_PARTIAL;
			if (SendDlgItemMessage(hWndDlg, IDC_FADE_NONE, BM_GETCHECK, 0, 0L))
				textFade = FADE_NONE;
			loopDates = SendDlgItemMessage(hWndDlg, IDC_LOOP, BM_GETCHECK, 0, 0L);
			sequentialMethod = SendDlgItemMessage(hWndDlg, IDC_SEQUENTIAL, BM_GETCHECK, 0, 0L);
			EndDialog(hWndDlg, TRUE);
		}
			break;
		}
		break;    /* End of WM_COMMAND                                 */

	default:
		return FALSE;
	}
	return TRUE;
}


BOOL DisplayDatedOrthos(HWND hWnd, int Message, WPARAM wParam, LPARAM lParam, short Function)
{
	static	BOOL	Inited = FALSE;
	static HCURSOR     OldCursor;
	static	UINT	CurrentPrompt;
	static HWND		hBackGroundServer = 0;
	static BOOL		ignoreHalt, serverIsReady;
	static HWND		hWndPlayer = 0;
	static HBITMAP	saveFWBM = 0;
	static BOOL		haveScreenBuf;
	char	txt[260];
	int		i;
	RECT	rect;

	switch (Message)
	{
	case GF_INIT:
		hWndDatedOrthos = hWnd;
		//GetClientRect(hWnd, &rect);
		haveScreenBuf = (BOOL)HaveScreenBuffer(0);
		if (haveScreenBuf)
			ProcessText("[%BUFFERSCREEN]=0");
		rect.left = rect.top = 0;
		rect.right = RECTWIDTH (&CurView->DrawRect);
		rect.bottom = RECTHEIGHT(&CurView->DrawRect);
		serverIsReady = FALSE;
		hBackGroundServer = StartBackgroundMapServer(hWnd, "[%DL]configs\\orthoserver.gmc", "", &rect);
		if (!hBackGroundServer)
		{
			MessageBox(hWnd, "Failed to start background map server", 0, MB_ICONEXCLAMATION);
			PostMessage(hWnd, GF_CLOSE, 0, 0L);
			break;
		}
	case GF_REINIT:
	{
		memset(datedOrthoFile, 0, sizeof(datedOrthoFile));
		ignoreHalt = TRUE;
		MergeImageIntoViewport(0, 0, 0, 0);
		KillTimer(hWnd, timerID);
		if (!hBackGroundServer || !GetGlobalCVal("[%ORTHODATEFILE]", OrthoDateFile, 0))
			PostMessage(hWnd, GF_CLOSE, 0, 0L);
		else
		{
			{
				int nRc = DialogBox(hInst, (LPSTR)"SELECTORTHOS", hWnd, (DLGPROC)SelectOrthosMESSAGEMsgProc);

				if (!nRc)
					PostMessage(hWnd, GF_CLOSE, 0, 0L);
				else
				{
					iDate = 0;
					Inited = TRUE;
					saveFWBM = hFullWindowBitMap;
					hFullWindowBitMap = (HBITMAP)-1;
					//ClearFullWindowBitmap(0);

					//SaveFullWindowBitmap(hWnd);
					CurrentPrompt = PRMT_PANZOOM2;
					SetPrompt(CurrentPrompt, TRUE);
					SetCurs(0, FALSE);
					sprintf(txt, "[ORTHODATE]=%s;$ZOOM(POINTANDSCALE,%f %f,%f,-1);$REDISPLAY(T)", orthoDates[0], CurView->MidPointW.x, CurView->MidPointW.y, CurView->Scale);
					SendBackgroundMapServerCommand(hWnd, hBackGroundServer, txt, MAKELPARAM(1,CurView->ID));
				}

			}
		}
	}
		break;

	case WM_TIMER:
		if (wParam != GF_DISPLAY_DATED_ORTHOS)
			return FALSE;
		else if (iDate < nDatesSelected)
		{
			if (iDate < nDatesReturned)
			{
				HDIB32 hDib32 = GMFIBMPHandleFromEXT(datedOrthoFile[iDate]);


				if (hDib32)
				{
					HBITMAP hBM = DIB32ToBitmap(hDib32, (HPALETTE)0);
					DestroyDIB32(hDib32, FALSE);
					timerID = SetTimer(hWnd, MAKELPARAM(GF_DISPLAY_DATED_ORTHOS, CurView->ID), timeBetweenDates, 0);
					MergeImageIntoViewport(hBM,&CurView->DrawRect, orthoTitles[iDate], textFade);
				}
				ignoreHalt = FALSE;
				iDate++;
			}
		}
		else if (loopDates)
		{
			iDate = 0;
			timerID = SetTimer(hWnd, MAKELPARAM(GF_DISPLAY_DATED_ORTHOS, CurView->ID), 1, 0);
		}
		else
			PostMessage(hWnd, GF_CLOSE, 0, 0L);

		break;

	case  GF_MAPSERVER_READY:
		serverIsReady = TRUE;
		break;

	case GF_MAPSERVER_FAILED:
		MessageBox(hWnd, "MapServer failed", 0, MB_ICONEXCLAMATION);
		break;

	case GF_MAPSERVER_RESPONSE://wParam HIWORD is return type, LOWORD is requestID;lParam is server wnd
		if (wParam == MAPSERVER_RETURNED_IMAGE)
		{
			int id = lParam;

			GSSiGetTempFileName(0, "gmo", 0, datedOrthoFile[id - 1]);
			CopyMapserverFileToFile(hBackGroundServer, datedOrthoFile[id - 1]);
			nDatesReturned = id;
			if (id < nDatesSelected)
			{
				//sprintf(txt, "[ORTHODATE]=%s;$REDISPLAY(T)", orthoDates[id-1]);
				sprintf(txt, "[ORTHODATE]=%s;$ZOOM(POINTANDSCALE,%f %f,%f,-1);$REDISPLAY(T)", orthoDates[id], CurView->MidPointW.x, CurView->MidPointW.y, CurView->Scale);
				SendBackgroundMapServerCommand(hWnd, hBackGroundServer, txt, MAKELPARAM(id+1, CurView->ID));
			}
			if (id == 1)
			{
				hWndPlayer = CreateDialog(hInst, (LPSTR)"PLAYER_DATED", hWnd, (DLGPROC)PlayOrthosMESSAGEMsgProc);
				AnimateWindow(hWndPlayer, 500, AW_BLEND | AW_ACTIVATE);
				timerID = SetTimer(hWnd, MAKELPARAM(GF_DISPLAY_DATED_ORTHOS, CurView->ID), 1, 0);
			}

		}
		break;

	case GF_EXECUTE:
	case GF_USEPICKED:
		/*		if (!HaveSizeFactor)
		{
		char	str[64];

		ftoa(str, NewPointSizeFactor);
		if (!GetTextString(hWnd, str, 32, "Enter size factor", 0, 0, 0, TRUE, TRUE))
		return GF_EXECUTE_CANCELED;
		NewPointSizeFactor = atof(str);
		if (!NewPointSizeFactor)
		return GF_EXECUTE_CANCELED;
		HaveSizeFactor = TRUE;
		}
		if (Message == GF_USEPICKED)
		NumPicked = 1;
		if (NumPicked > 0)
		{
		UpdateItem = 202;
		UpdateRecord(0, PickList[NumPicked - 1].Desc, PickList[NumPicked - 1].Prefix, PickList[NumPicked - 1].UDI, 0, 0, 1, -1);
		}
		if (Message == GF_USEPICKED)
		PostMessage(hWnd, GF_CLOSE, 0, 0L);
		*/
		return GF_INCREASE_SUCCESS_COUNT;

	case GF_ENTER_VIEWPORT:
		SetPrompt(CurrentPrompt, TRUE);
	case GF_REDRAW:
	case GF_REDRAW_CMD:
	case GF_CLEAR:
	case GF_CLEAR_CMD:
	case GF_INCREASE_SUCCESS_COUNT:
	case GF_DECREASE_SUCCESS_COUNT:
	case GF_DISPLAYMESS:
	case GF_READY_TO_PROCESS:
	case GF_EXECUTE_FINISHED:
	case GF_EXIT_VIEWPORT:
		break;
	case WM_LBUTTONDOWN:
		if (timerID)
		{
			KillTimer(hWnd, timerID);
			timerID = 0;
		}
		else
		{
			timerID = SetTimer(hWnd, MAKELPARAM(GF_DISPLAY_DATED_ORTHOS, CurView->ID), 1, 0);
		}
		break;
	case WM_LBUTTONUP:
	{
		DPOINT		BasePoint;
		POINT		MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));


		if (CursorIsLocked)
			BasePoint = CurrentPoint;
		else
			BasePoint = ScreenPtToBasePt(MousePoint);
	}
		break;

	case WM_RBUTTONUP:
	{
	}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case 27:  //ESC   
			PostMessage(hWnd, GF_CLOSE, 0, 0L);
			break;
		case VK_F9:
		default:
			return FALSE;
		}
		break;

	case WM_CHAR:
	{
		switch (wParam)
		{
		case 'x':
		case 'X':
			PostMessage(hWnd, GF_COMPLETE, 0, 0L);
			break;
		default:
			return FALSE;
		}
	}
		break;
	case GF_COMPLETE:
		PostMessage(hWnd, GF_CLOSE, 0, 0L);
		break;

	case GF_HALTDISPLAY:
		if (!ignoreHalt)
		{
			RestoreFullWindowBitmap();
			ClearFullWindowBitmap(0);
			PostMessage(hWnd, GF_CLOSE, 0, 0L);
		}
		break;
	case GF_CLOSE:
		if (hWndPlayer)
		{
			DestroyWindow(hWndPlayer);
		}
		KillTimer(hWnd, timerID);
		ClearFullWindowBitmap(0);
		if ((int)saveFWBM != -1)
			hFullWindowBitMap = saveFWBM;
		RestoreFullWindowBitmap();
		Inited = FALSE;
		MergeImageIntoViewport(0, 0, 0, 0);
		StopBackgroundMapServer(hBackGroundServer);
		for (i = 0; i < nDatesReturned; i++)
			GSSiRemove(datedOrthoFile[i]);
		if (haveScreenBuf)
			ProcessText("[%BUFFERSCREEN]=1");

		return FALSE;
		break;

	default:
		return (FALSE);
	}
	return (TRUE);
}

