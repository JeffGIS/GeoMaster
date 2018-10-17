#include "graphint.h"  
#include "umio.h"
#include "CDDEMO.h"
#include "extrndb.h"   
#include <sqlext.h>

#include "gmextern.h"

static TCHAR szTitleGMDoc[] = "GMDoc";					// The title bar text
static TCHAR szWindowClassGMDoc[] = "GMDocumenter";		// the main window class name
static int continueBackgroundMerge;
static HANDLE hThread = 0;
static	HFONT	hFont;
static	HFONT	hOldFont;
static	BOOL	changesMade;
static	char	GMDocFile[MAX_PATH] = "";
static  char	GMDocDir[MAX_PATH];
static	HANDLE	hFile = 0;
static	int		lFile;
static	LPSTR	pFile;
static	int		LINES = 0, maxLine = 0;
static	float	avCharWidth;
static	int		xChar;       // horizontal scrolling unit 
static	int		yChar;       // vertical scrolling unit 
static	int insertLoc = 0, insertLoc2 = -1;
static	POINT	insertPoint = { 0 };
static	POINT	insertPoint2;
static	HANDLE	hInsertOpts = 0;
static	BOOL	standAlone = FALSE;
static	BOOL	displayOnlyCurrentLine = FALSE;
static	int		currentLine = 0;
static	HWND	hWndGMDocReturn = 0;
static	BOOL	RestartFromLastPos = FALSE;
static  int		DocPos = 0;
static	int		FadeSpeed = 10;
static	int		timerValue = 100;
static	int		saveTimerValue = 0;
static	BOOL	inManualMode = TRUE;
static	BOOL	productionMode = FALSE;

void __cdecl BackgroundMergeDocImageIntoViewport(LPHANDLE phArgs);

INT_PTR CALLBACK	AboutGMDoc(HWND, UINT, WPARAM, LPARAM);
ATOM MyRegisterClassGMDoc(HINSTANCE hInstance);
LRESULT CALLBACK WndProcGMDoc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void DisplayDocImage(LPSTR ImagePath, RECT rect, int Fade, BOOL Transparent, COLORREF TranColor);

// Message handler for about box.
INT_PTR CALLBACK AboutGMDoc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		char	value[128];
		char ImagePath[MAX_PATH] = "[%DL]GMDocumenter\\GMDocHelp.png";
		ExpandText(ImagePath);
		HDIB32 hDib32 = GMFIBMPHandleFromEXT(ImagePath);
		HBITMAP hBitmap = DIB32ToBitmap(hDib32, (HPALETTE)0);
		DestroyDIB32(hDib32, FALSE);

		cwCenter(hDlg, 0);

		SendDlgItemMessage(hDlg, IDC_GMDOCTEXT, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);
		ii = GetPrivateProfileString("User", "DontShowGMDocHelpAtStartup", "0", value, sizeof(value), GMIni);
		SendDlgItemMessage(hDlg, IDC_CHECK_DONOTSHOW, BM_SETCHECK, !atob(value), 0);
	}
		return (INT_PTR)TRUE;

	case WM_DESTROY:
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == IDC_CHECK_DONOTSHOW)
		{
			if (SendDlgItemMessage(hDlg, IDC_CHECK_DONOTSHOW, BM_GETCHECK, 0, 0L))
				ii = WritePrivateProfileString("User", "DontShowGMDocHelpAtStartup", "1", GMIni);
			else
				ii = WritePrivateProfileString("User", "DontShowGMDocHelpAtStartup", "0", GMIni);

		}
		break;
	}
	return (INT_PTR)FALSE;
}
int APIENTRY WinMainGMDoc(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{

	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable = 0;
	SIZE size;
	LPSTR pFile = strstr(lpCmdLine, "/GMDoc ");
	LPSTR pEndFile = strchr(pFile, 0);
	LPSTR pWnd = strstr(lpCmdLine, "/W ");
	LPSTR pMode = strstr(lpCmdLine, "/PROD");
	if (pMode)
	{
		productionMode = TRUE;
	}
	ExpandDL();
	if (pWnd)
	{
		*pWnd = 0;
		pWnd += 3;
		hWndGMDocReturn = (HWND)atoi(pWnd);
		ExpandText(pWnd); //sets test env if included
	}
	if (pFile)
	{
		LPSTR pFS;

		pFile += 7;
		if (*pFile == '\'')
		{
			pFile++;
			pEndFile = strrchr(pFile, '\'');
			if (pEndFile)
				*pEndFile++ = 0;
		}
		else
			pEndFile = strchr(pFile, ' ');
		if (pEndFile && *pEndFile)
			*pEndFile++ = 0;
		RestartFromLastPos = atob(pEndFile);
		strcpy(GMDocDir, pFile);
		if ((pFS = strchr(GMDocDir, '/')))
			*pFS++ = 0;
		Truncate(GMDocDir);
		if (productionMode)
			strcat(GMDocDir, "\\composited");
		else
			strcat(GMDocDir, "\\raw");
		ExpandText(GMDocDir);
		sprintf(GMDocFile,"%s\\filelist.csv", GMDocDir);
	}


	// Initialize global strings
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_GMEDIT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClassGMDoc(hInstance);

	// Perform application initialization:
	if (!InitInstanceGMDoc(hInstance, nCmdShow))
	{
		return FALSE;
	}
	//hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_GMDOC));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			//if (!IsDialogMessage(ghFindReplaceDlg, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	GSSiGlobFree(&hFile);
	if (hWndGMDocReturn)
	{
		//sprintf(str, "send %ld to %ld", GSSI_GMEDITCOMPLETE, (DWORD)hWndGMEditReturn);
		//MessageBox(0, str, 0, MB_OK);
		PostMessage(hWndGMDocReturn, GSSI_GMDOCCOMPLETE, 0, 0);
	}
	return (int)msg.wParam;
}



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
ATOM MyRegisterClassGMDoc(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProcGMDoc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GMEDIT_LARGE));
	wcex.hCursor = LoadCursor(NULL, IDC_IBEAM);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_GMDOC);
	wcex.lpszClassName = szWindowClassGMDoc;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_GMEDIT_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstanceGMDoc(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClassGMDoc, szTitleGMDoc, WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL | CS_OWNDC,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

void DoDelay(int seconds)
{
	Wait(seconds * 1000);
	return;
}
BOOL ProcessGMDocItem(HWND hWnd)
{
	BOOL rtn = FALSE;
	char	itemName[66];
	char	itemFile[MAX_PATH];
	char	line[1024];
	char	imageName[34];
	char	imagePath[MAX_PATH];
	int		item = 0;
	HFILE fidList = GSSiOpenFile(GMDocFile, 0, OF_READ);
	HFILE fidItem;
	RECT	clientRect;
	BOOL	Err;
	int		fadeIn, delay;
	int		icursor=0;

	if (fidList == HFILE_ERROR)
		return FALSE;
	GetClientRect(hWndMain, &clientRect);
	while (item < DocPos && fgetstring(itemName, 64, fidList))
	{
		item++;
	}
	GSSiClose(fidList);
	if (item < DocPos)
		return rtn;
	rtn = TRUE;
	strcpy(itemFile, GMDocFile);
	LPSTR pEndDir = strrchr(itemFile, '\\') + 1;
	*pEndDir = 0;
	strcpy(imagePath, itemFile);
	strcat(itemFile, itemName);
	fidItem = GSSiOpenFile(itemFile, 0, OF_READ);
	fgetstring(line, 1022, fidItem);
	if (!stricmp(line, "CAPSCREEN"))
	{
		RECT ImageRect;
		int	 transparent;
		COLORREF tranColor = 0;
		fgetstring(imageName, 32, fidItem);
		if (!strnicmp(imageName, "Icons\\", 6))
		{
			LPSTR pEnd;
			strcpy(imagePath, GMDocDir);
			pEnd = strrchr(imagePath, '\\');
			*++pEnd = 0;
		}
		strcat(imagePath, imageName);
		ExpandText(imagePath);
		fgetstring(line, 64, fidItem);
		ImageRect = atorect(line, &Err);
		if (IsRectEmpty(&ImageRect))
		{
			int width = 0;
			int height = 0;
			HDIB32	hDib32 = LoadDIB32(imagePath, FALSE);
			if (hDib32)
			{
				width = FreeImage_GetWidth(hDib32);
				height = FreeImage_GetHeight(hDib32);
				DestroyDIB32(hDib32, FALSE);
				ImageRect.right = ImageRect.left + width;
				ImageRect.bottom = ImageRect.top + height;
			}
		}
		fgetstring(line, 64, fidItem);
		fadeIn = atoi(line);
		fgetstring(line, 64, fidItem);
		delay = atoi(line);
		fgetstring(line, 64, fidItem);
		transparent = atoi(line);
		fgetstring(line, 64, fidItem);
		ExpandText(line);
		tranColor = atoi(line);
		KillTimer(hWnd, 1);
		DisplayDocImage(imagePath, ImageRect, fadeIn, transparent, tranColor);
		HDC hDC = GetDC(hWndMain);

		//FrameRect(hDC, &ImageRect, GetStockObject(BLACK_BRUSH));
		ReleaseDC(hWndMain, hDC);

		timerValue = delay * 1000;
		if (!inManualMode)
			SetTimer(hWnd, 1, timerValue, 0);
	}
	else if (!stricmp(line, "CAPSCRIPT"))
	{
		RECT ImageRect;
		int	 transparent;
		COLORREF tranColor = 0;

		fgetstring(line, 64, fidItem);
		ImageRect = atorect(line, &Err);
		fgetstring(imageName, 32, fidItem);
		{
			LPSTR pEnd;
			strcpy(imagePath, GMDocDir);
			pEnd = strrchr(imagePath, '\\');
			*++pEnd = 0;
		}
		strcat(imagePath, "scripts\\");
		strcat(imagePath, imageName);
		ExpandText(imagePath);
		if (IsRectEmpty(&ImageRect))
		{
			int width = 0;
			int height = 0;
			HDIB32	hDib32 = LoadDIB32(imagePath, FALSE);
			if (hDib32)
			{
				width = FreeImage_GetWidth(hDib32);
				height = FreeImage_GetHeight(hDib32);
				DestroyDIB32(hDib32, FALSE);
				ImageRect.right = ImageRect.left + width;
				ImageRect.bottom = ImageRect.top + height;
			}
		}
		fgetstring(line, 64, fidItem);
		fadeIn = atoi(line);
		fgetstring(line, 64, fidItem);
		delay = atoi(line);
		fgetstring(line, 64, fidItem);
		transparent = atoi(line);
		fgetstring(line, 64, fidItem);
		ExpandText(line);
		tranColor = atoi(line);
		KillTimer(hWnd, 1);
		DisplayDocImage(imagePath, ImageRect, fadeIn, transparent, tranColor);
		HDC hDC = GetDC(hWndMain);

		//FrameRect(hDC, &ImageRect, GetStockObject(BLACK_BRUSH));
		ReleaseDC(hWndMain, hDC);

		timerValue = delay * 1000;
		if (!inManualMode)
			SetTimer(hWnd, 1, timerValue, 0);
	}
	else if (!stricmp(line, "CAPRIGHTCLICK") || !stricmp(line, "CAPLEFTCLICK"))
	{
		RECT ImageRect;
		int	 transparent;
		COLORREF tranColor = 0;
		char offsetC[32];
		LPSTR pOffset;
		int offsetX = 0, offsetY = 0;

		fgetstring(line, 64, fidItem);
		ImageRect = atorect(line, &Err);
		fgetstring(line, 64, fidItem);
		icursor = atoi(line);
		fgetstring(imageName, 32, fidItem);
		{
			LPSTR pEnd;
			strcpy(imagePath, GMDocDir);
			pEnd = strrchr(imagePath, '\\');
			*pEnd = 0;
			pEnd = strrchr(imagePath, '\\');
			*pEnd = 0;
		}
		strcat(imagePath, "\\icons\\");
		strcat(imagePath, imageName);
		ExpandText(imagePath);
		pOffset = strchr(imageName, '_');
		if (pOffset)
		{
			pOffset++;
			strcpy(offsetC, pOffset);
			pOffset = strchr(offsetC, '_');
			if (pOffset)
				*pOffset++ = 0;
			offsetX = atoi(offsetC);
			offsetY = atoi(pOffset);
		}
		ImageRect.left -= offsetX;
		ImageRect.top -= offsetY;
		if (IsRectEmpty(&ImageRect))
		{
			int width = 0;
			int height = 0;
			HDIB32	hDib32 = LoadDIB32(imagePath, FALSE);
			if (hDib32)
			{
				width = FreeImage_GetWidth(hDib32);
				height = FreeImage_GetHeight(hDib32);
				DestroyDIB32(hDib32, FALSE);
				ImageRect.right = ImageRect.left + width;
				ImageRect.bottom = ImageRect.top + height;
			}
		}
		fgetstring(line, 64, fidItem);
		fadeIn = atoi(line);
		fgetstring(line, 64, fidItem);
		delay = atoi(line);
		fgetstring(line, 64, fidItem);
		transparent = 1;
		if (!fgetstring(line, 64, fidItem))
			strcpy(line, "$RGB(255,255,255)");
		ExpandText(line);
		tranColor = atoi(line);
		KillTimer(hWnd, 1);
		DisplayDocImage(imagePath, ImageRect, fadeIn, transparent, tranColor);
		HDC hDC = GetDC(hWndMain);

		//FrameRect(hDC, &ImageRect, GetStockObject(BLACK_BRUSH));
		ReleaseDC(hWndMain, hDC);

		timerValue = delay * 1000;
		if (!inManualMode)
			SetTimer(hWnd, 1, timerValue, 0);
	}
	else if (!stricmp(line, "POINTER"))
	{
		RECT ImageRect;
		int	 transparent;
		COLORREF tranColor = 0;
		char offsetC[32];
		LPSTR pOffset;
		int offsetX = 0, offsetY = 0;

		fgetstring(line, 64, fidItem);
		ImageRect = atorect(line, &Err);
		fgetstring(imageName, 32, fidItem);
		{
			LPSTR pEnd;
			strcpy(imagePath, GMDocDir);
			pEnd = strrchr(imagePath, '\\');
			*pEnd = 0;
			pEnd = strrchr(imagePath, '\\');
			*pEnd = 0;
		}
		strcat(imagePath, "\\icons\\");
		strcat(imagePath, imageName);
		ExpandText(imagePath);
		pOffset = strchr(imageName, '_');
		if (pOffset)
		{
			pOffset++;
			strcpy(offsetC, pOffset);
			pOffset = strchr(offsetC, '_');
			if (pOffset)
				*pOffset++ = 0;
			offsetX = atoi(offsetC);
			offsetY = atoi(pOffset);
		}
		ImageRect.left -= offsetX;
		ImageRect.top -= offsetY;
		if (IsRectEmpty(&ImageRect))
		{
			int width = 0;
			int height = 0;
			HDIB32	hDib32 = LoadDIB32(imagePath, FALSE);
			if (hDib32)
			{
				width = FreeImage_GetWidth(hDib32);
				height = FreeImage_GetHeight(hDib32);
				DestroyDIB32(hDib32, FALSE);
				ImageRect.right = ImageRect.left + width;
				ImageRect.bottom = ImageRect.top + height;
			}
		}
		fgetstring(line, 64, fidItem);
		fadeIn = atoi(line);
		fgetstring(line, 64, fidItem);
		delay = atoi(line);
		fgetstring(line, 64, fidItem);
		transparent = 1;
		if (!fgetstring(line, 64, fidItem))
			strcpy(line, "$RGB(255,255,255)");
		ExpandText(line);
		tranColor = atoi(line);
		KillTimer(hWnd, 1);
		DisplayDocImage(imagePath, ImageRect, fadeIn, transparent, tranColor);
		HDC hDC = GetDC(hWndMain);

		//FrameRect(hDC, &ImageRect, GetStockObject(BLACK_BRUSH));
		ReleaseDC(hWndMain, hDC);

		timerValue = delay * 1000;
		if (!inManualMode)
			SetTimer(hWnd, 1, timerValue, 0);
	}
	GSSiClose(fidItem);
	DocPos++;
	return rtn;

}
LRESULT CALLBACK WndProcGMDoc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	TEXTMETRIC	tm;
	SCROLLINFO si;
	SIZE size;
	int rowHeight;
	HDC hdc;
	int tablen;

	// These variables are required to display text. 
	static int xClient;     // width of client area 
	static int yClient;     // height of client area 
	static int xClientMax;  // maximum width of client area 

	static int xUpper;      // average width of uppercase letters 

	static int xPos;        // current horizontal scrolling position 
	static int yPos;        // current vertical scrolling position 

	int i;                  // loop counter 
	int x, y;               // horizontal and vertical coordinates

	HRESULT hr;
	size_t abcLength;        // length of an abc[] item 
	int fontSize = 18;
	int	insertLoc3;
	int cursorWidth = 5, cursorHeight = fontSize - 2;
	int saveLine;
	POINT mousePoint;
	RECT rect;
	char key;
	static HMENU hMenu = 0;
	int loc;
	static haveFile = FALSE;


	switch (message)
	{
	case WM_CREATE:
	{
		char value[128];

		if (!hWndMain)
		{
			hWndMain = hWnd;
			CDInit(hWnd, hInst); /* Initialize Common Dialogs */
			CurView = (LPVIEWPORT)malloc(sizeof(VIEWPORT));
			CurView->hWnd = hWnd;
			standAlone = TRUE;
		}

		hdc = GetDC(hWnd);

		hFont = CreateFont(fontSize, 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, OUT_TT_PRECIS, 0, PROOF_QUALITY, 0, "Courier New");
		hOldFont = SelectObject(hdc, hFont);
		// Extract font dimensions from the text metrics. 
		GetTextMetrics(hdc, &tm);
		xChar = tm.tmAveCharWidth;
		xUpper = (tm.tmPitchAndFamily & 1 ? 3 : 2) * xChar / 2;
		yChar = tm.tmHeight + tm.tmExternalLeading;

		insertPoint.x = insertPoint.y = 0;
		insertLoc = insertLoc2 = 0;
		// Set an arbitrary maximum width for client area. 
		// (xClientMax is the sum of the widths of 48 average 
		// lowercase letters and 12 uppercase letters.) 
		xClientMax = 48 * xChar + 12 * xUpper;
		haveFile = FALSE;
		if (*GMDocFile)
		{
			HFILE fid = GSSiOpenFile(GMDocFile, 0, OF_READ);

			if (fid != HFILE_ERROR)
			{
				haveFile = TRUE;
				DocPos = 1;
				GSSiClose(fid);
			}
		}
		// Free the device context. 
		SelectObject(hdc, hOldFont);
		ReleaseDC(hWnd, hdc);
		GetWindowRect(hWnd, &rect);
		ii = GetPrivateProfileString("User", "DontShowGMDocHelpAtStartup", "0", value, sizeof(value), GMIni);
		if (!atob(value))
			PostMessage(hWnd, WM_COMMAND, IDM_ABOUT, 0);
		InvalidateRect(hWnd, 0, TRUE);
	}
		return 0;
	case WM_DESTROY:
		KillTimer(hWnd, 1);
		continueBackgroundMerge = FALSE;
		GSSiDeleteObject(&hFont);
		AutoInsertClose();
		if (standAlone)
		{
			FreeBigMem();
			free(CurView);
			PostQuitMessage(0);
		}
		break;

	case WM_CLOSE:
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_TIMER:
		if (inManualMode)
			KillTimer(hWnd, 1);
		if (!ProcessGMDocItem(hWnd))
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	case WM_PAINT:
		KillTimer(hWnd, 1);
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		DrawMenuBar(hWnd);
		DocPos = max(1, DocPos - 1);
		SetTimer(hWnd, 1, timerValue, 0);
		break;

	case WM_CHAR:
		if (!haveFile)
			break;
		key = wParam;
		switch (key)
		{
		case VK_BACK:
			saveLine = -1;
		default:
			displayOnlyCurrentLine = TRUE;
			currentLine = saveLine;
			InvalidateRect(hWnd, 0, TRUE);
			break;
		case 27: //escape
			PostMessage(hWnd, GF_CLOSE, 0, 0L);
			break;
		case VK_RETURN:
			InvalidateRect(hWnd, 0, TRUE);
			break;
		case 'b':
		case 'B':
			DocPos = 0;
			InvalidateRect(hWnd, 0, TRUE);
			break;
		case 'h':
		case 'H':
			PostMessage(hWnd, WM_COMMAND, IDM_ABOUT, 0);
		case 'n':
		case 'N':
		case '>':
			SetTimer(hWnd, 1, 50, 0);

			break;
		}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
		case VK_HOME:
			InvalidateRect(hWnd, 0, TRUE);
			break;
		case VK_END:
			InvalidateRect(hWnd, 0, TRUE);
			break;
		case VK_DELETE:
			break;
		case VK_F1:
			break;
		case VK_LEFT:
			break;
		case VK_RIGHT:
			break;
		case VK_UP:
			break;
		case VK_DOWN:
			break;
		}
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_MOUSEMOVE:
		break;
	case WM_LBUTTONUP:
		break;
	case WM_SIZE:
		GetWindowRect(hWnd, &rect);
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_SETFOCUS:

		// Create a solid black caret. 
		CreateCaret(hWnd, (HBITMAP)NULL, cursorWidth, cursorHeight);

		// Adjust the caret position, in client coordinates. 
		SetCaretPos(insertPoint.x, insertPoint.y);

		// Display the caret. 
		ShowCaret(hWnd);

		break;
	case WM_KILLFOCUS:

		// The window is losing the keyboard focus, so destroy the caret. 

		DestroyCaret();

		break;
	case WM_HSCROLL:
		// Get all the vertial scroll bar information.
		si.cbSize = sizeof (si);
		si.fMask = SIF_ALL;

		currentLine = -1;
		// Save the position for comparison later on.
		GetScrollInfo(hWnd, SB_HORZ, &si);
		xPos = si.nPos;
		switch (LOWORD(wParam))
		{
			// User clicked the left arrow.
		case SB_LINELEFT:
			si.nPos -= 1;
			break;

			// User clicked the right arrow.
		case SB_LINERIGHT:
			si.nPos += 1;
			break;

			// User clicked the scroll bar shaft left of the scroll box.
		case SB_PAGELEFT:
			si.nPos -= si.nPage;
			break;

			// User clicked the scroll bar shaft right of the scroll box.
		case SB_PAGERIGHT:
			si.nPos += si.nPage;
			break;

			// User dragged the scroll box.
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;

		default:
			break;
		}

		// Set the position and then retrieve it.  Due to adjustments
		// by Windows it may not be the same as the value set.
		si.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
		GetScrollInfo(hWnd, SB_HORZ, &si);

		// If the position has changed, scroll the window.
		if (si.nPos != xPos)
		{
			ScrollWindow(hWnd, xChar * (xPos - si.nPos), 0, NULL, NULL);
		}

		return 0;
	case WM_MOUSEWHEEL:
	{
		int		fwKeys = LOWORD(wParam);    // key flags
		int		oldPos;
		short	zDelta = (short)HIWORD(wParam);    // wheel rotation

		currentLine = -1;
		si.cbSize = sizeof (si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_VERT, &si);
		oldPos = si.nPos;
		if (zDelta < 0)
			si.nPos += 1;
		else
			si.nPos -= 1;
		si.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hWnd, SB_VERT, &si);
		if (si.nPos != oldPos)
			InvalidateRect(hWnd, 0, TRUE);
		break;
	}
	case WM_VSCROLL:
		// Get all the vertial scroll bar information.
		currentLine = -1;
		si.cbSize = sizeof (si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_VERT, &si);

		// Save the position for comparison later on.
		yPos = si.nPos;
		switch (LOWORD(wParam))
		{

			// User clicked the HOME keyboard key.
		case SB_TOP:
			si.nPos = si.nMin;
			break;

			// User clicked the END keyboard key.
		case SB_BOTTOM:
			si.nPos = si.nMax;
			break;

			// User clicked the top arrow.
		case SB_LINEUP:
			si.nPos -= 1;
			break;

			// User clicked the bottom arrow.
		case SB_LINEDOWN:
			si.nPos += 1;
			break;

			// User clicked the scroll bar shaft above the scroll box.
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;

			// User clicked the scroll bar shaft below the scroll box.
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;

			// User dragged the scroll box.
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;

		default:
			break;
		}

		// Set the position and then retrieve it.  Due to adjustments
		// by Windows it may not be the same as the value set.
		si.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hWnd, SB_VERT, &si);

		// If the position has changed, scroll window and update it.
		if (si.nPos != yPos)
		{
			ScrollWindow(hWnd, 0, yChar * (yPos - si.nPos), NULL, NULL);
			UpdateWindow(hWnd);
		}

		return 0;

	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_FILE_EXIT:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
		case IDM_FILE_SAVE:
			GMEditSaveUpdates(hWnd);
			break;
		case IDM_ABOUT:
		{
			KillTimer(hWnd, 1);
			saveTimerValue = timerValue;
			timerValue = -1;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_GMDOCHELP), hWnd, AboutGMDoc);
			timerValue = saveTimerValue;
			InvalidateRect(hWnd, 0, TRUE);
		}
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;

}

void DisplayDocImage(LPSTR ImagePath, RECT rect, int fade, BOOL Transparent,COLORREF TranColor)
{
	if (Transparent)
	{
		HDC hDC = GetDC(hWndMain);
		HDIB32 hDib32 = GMFIBMPHandleFromEXT(ImagePath);
		POINT tiePoint = { rect.left + RECTWIDTH(&rect) / 2, rect.top + RECTHEIGHT(&rect) / 2 };
		DisplayTransparentBitmap(hDC, &hDib32, tiePoint, 0, &rect, 0, &TranColor);
		//DisplayTransparentBitmapInRect(hDC, hDib32, &rect, TRUE);
		DestroyDIB32(hDib32, FALSE);
		ReleaseDC(hWndMain, hDC);
	}
	else if (!fade)
	{
		HDC hDC = GetDC(hWndMain);

		DisplayBMFileInRect(hDC, ImagePath, rect, 1);
		ReleaseDC(hWndMain, hDC);
	}
	else
	{
		HDIB32 hDib32 = GMFIBMPHandleFromEXT(ImagePath);

		FadeSpeed = fade;
		if (hDib32)
		{
			BITMAP bm;
			HDIB32 hDib24 = FreeImage_ConvertTo24Bits(hDib32);
			HBITMAP hBM = DIB32ToBitmap(hDib32, (HPALETTE)0);
			GetObject(hBM, sizeof(bm), (LPSTR)&bm);

			DestroyDIB32(hDib32, FALSE);
			DestroyDIB32(hDib24, FALSE);
			//timerID = SetTimer(hWnd, MAKELPARAM(GF_DISPLAY_DATED_ORTHOS, CurView->ID), timeBetweenDates, 0);
			MergeDocImageIntoViewport(hBM, &rect, "", 0);
		}
	}
}


BOOL MergeDocImageIntoViewport(HBITMAP hNewBitmap, LPRECT pRect, LPSTR title, int textFade)
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

	GlobalUnlock (hArgs);
	continueBackgroundMerge = 1;

	hThread = (HANDLE)_beginthread(BackgroundMergeDocImageIntoViewport, 0, &hArgs);
	return TRUE;
}

void __cdecl BackgroundMergeDocImageIntoViewport(LPHANDLE phArgs)
{
	LPSTR	arg1 = GlobalLock(*phArgs);
	LPSTR	arg2 = arg1 + 4096, arg3 = arg2 + 4096, arg4 = arg3 + 4096;
	HBITMAP hNewBitmap = (HBITMAP)atoi(arg1);
	char	title[256];
	int		textFade = 100;
	int		err;
	RECT	rect = atorect(arg2, &err);

	strcpy(title, arg3);
	textFade = atoi(arg4);


	GSSiGlobUlFree(phArgs);
	MergeDocImageIntoViewport2(hNewBitmap, rect, title, textFade);
	return;
}

BOOL MergeDocImageIntoViewport2(HBITMAP hNewBitmap, RECT rect, LPSTR title, int textFade)
{
	BLENDFUNCTION bf;
	BOOL	rtn = TRUE;
	static	BOOL blt = FALSE;
	int transParency = 1, inc = 1;
	BOOL first = TRUE;


	if (hNewBitmap)
	{
		HDC hDC = GetDC(hWndMain);
		HDC	TransparentDC = CreateCompatibleDC(hDC);
		HDC	saveDC = CreateCompatibleDC(hDC);
		HDC	tempDC = CreateCompatibleDC(hDC);
		int w;
		int h;
		BITMAP bm;
		HBITMAP hbm = 0, hBMSave = 0;
		HBITMAP hBMOrig=0, hBMTemp=0, hBMTempOld=0;
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
		textRect.bottom = RECTHEIGHT(&rect) - height / 2;
		textRect.right = RECTWIDTH(&rect);
		SelectObject(TransparentDC, OldFont);
		DeleteObject(hFont);
		factor = 2 * ((float)RECTWIDTH(&rect) / (float)RECTWIDTH(&rect2)) / 3;
		hFont = CreateFont(256 * factor, 0, 0, 0, FW_HEAVY, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
		w = RECTWIDTH(&rect);
		h = RECTHEIGHT(&rect);
		bf.BlendOp = AC_SRC_OVER;
		bf.BlendFlags = 0;
		bf.SourceConstantAlpha = (0xff * transParency) / 100;  // half of 0xff = 50% transparency 
		bf.AlphaFormat = 0;// AC_SRC_ALPHA;   // use source alpha  
		//SaveDC(hDC);
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
					if (pclr->rgbBlue == 255 && pclr->rgbGreen == 255 && pclr->rgbRed == 255)
						pclr->rgbReserved = 0;
					else
						pclr->rgbReserved = 255;
					pclr++;
				}
			}
			SetBitmapBits(hNewBitmap, lenBits, pBits);
			free(pBits);
		}
		hBMOrig = SelectObject(TransparentDC, hNewBitmap);
		SetTextColor(TransparentDC, RGB(255, 0, 0));
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
				//if (dbug)
				//	BitBlt(hDC,rect.left, rect.top, w, h,TransparentDC, rect.left, rect.top, SRCCOPY);
				BitBlt(tempDC, 0, 0, w, h,
					saveDC, 0, 0, SRCCOPY);
				AlphaBlend(tempDC, 0, 0, w, h,
					TransparentDC,
					0, 0,
					w, h,
					bf);
				BitBlt(hDC, rect.left, rect.top, w, h,
					tempDC, 0, 0, SRCCOPY);
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
					FillRect(TransparentDC, &rect, GetStockObject(BLACK_BRUSH));
					//if (dbug)
					//	BitBlt(hDC, rect.left, rect.top, w, h,TransparentDC, rect.left, rect.top, SRCCOPY);
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
					SelectObject(TransparentDC, OldFont);
					//if (dbug)
					//	BitBlt(hDC, rect.left, rect.top, w, h,TransparentDC, rect.left, rect.top, SRCCOPY);


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
						bf.AlphaFormat = AC_SRC_ALPHA;
					}
					hBMOrig = SelectObject(TransparentDC, hNewBitmap);
					//if (dbug)
					//	BitBlt(hDC, rect.left, rect.top, w, h,TransparentDC, rect.left, rect.top, SRCCOPY);
				}
			}
			else
				Sleep(FadeSpeed);
		}
		/*			{
		HBITMAP hbm = SaveScreen (hDC,rect);
		SaveBitmap (hbm,"c:\\temp\\blendpost.bmp",0,0);
		GSSiDeleteObject (&hbm);
		}*/
		//RestoreDC(hDC, -1);
		SelectObject(TransparentDC, hBMOrig);
		SelectObject(tempDC, hBMTempOld);
		DeleteDC(tempDC);
		DeleteDC(TransparentDC);
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
