
//#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
//#include <windows.h>
//#include <winbase.h>

// C RunTime Header Files
//#include <stdio.h>
//#include <malloc.h>
//#include <memory.h>
//#include <tchar.h>
//#include <strsafe.h>
#include "graphint.h"  
#include "umio.h"
#include "CDDEMO.h"
#include "extrndb.h"   
#include <sqlext.h>

#include "gmextern.h"

//#include "..\\GEOMASTR\resource.h"
#define MAX_LOADSTRING 100

// Global Variables:
extern HINSTANCE hInst;								// current instance

static TCHAR szTitle[]="GMEdit";					// The title bar text
static TCHAR szWindowClass[]="GMEditor";		// the main window class name

#define MAX_FILE_SIZE	1024*1024*4


typedef struct {char NameAndArgs[128];
				int	 IDNum;
				char Type[32];
				char Desc[255];
}FUNDEFS;
typedef FUNDEFS *LPFUNDEFS;

static	HFONT	hFont;
static	HFONT	hOldFont;
static	BOOL	changesMade;
static	char	fileToEdit[MAX_PATH]="";
static	HANDLE	hFile=0;
static	int		lFile;
static	LPSTR	pFile;
static	int		LINES=0, maxLine=0;
static	float	avCharWidth;
static	int		xChar;       // horizontal scrolling unit 
static	int		yChar;       // vertical scrolling unit 
static	int insertLoc=0, insertLoc2=-1;
static	POINT	insertPoint = {0};
static	POINT	insertPoint2;
static	HANDLE	hInsertOpts=0;
static	char	str[4096];
static	UINT	uFindReplaceMsg=0;
static	HWND	ghFindReplaceDlg=0;
static	LPFINDREPLACECHUNK lpFChunk=0;
static	HANDLE	hFChunk=0;
static	LPFINDREPLACECHUNK lpFRChunk=0;
static	HANDLE	hFRChunk=0;
static	int		firstLine;          // first line in the invalidated area 
static	int		lastLine;           // last line in the invalidated area 
static	int		insertLine;
static  int		currentLine = -1;
static	RECT	currentRect;
static	BOOL	displayOnlyCurrentLine = FALSE;
static	BOOL	setToFind = FALSE;	// set scroll loc to found position
static	BOOL	standAlone = FALSE;
static	char	currentBreakpoint[32]={0};
static  HWND	hWndGMEditReturn = 0;
static	int		breakAtLoc = -1;


static HANDLE hFunDefDB=0;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstanceGM(HINSTANCE hInst, int i);
LRESULT CALLBACK	WndProcGMEdit(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	AboutGMEdit(HWND, UINT, WPARAM, LPARAM);

void GMEditSetFile (LPSTR file,LPSTR bpid,int bploc)
{
	*currentBreakpoint = 0;
	breakAtLoc = -1;
	strcpy(fileToEdit, file);
	if (bploc)
		breakAtLoc = bploc-1;
	else if (bpid)
		strcpy(currentBreakpoint, bpid);
	return;
}

void GMEditGetFile(LPSTR file)
{ 
	strcpy(file, fileToEdit);
	return;
}

void createFunIDFile (void)
{
	char infile[]="g:\\GeomasterSourceCode\\GEOMASTR\\FUNDEFS.C";
	char outfile[]="g:\\GeomasterSourceCode\\GEOMASTR\\FUNDEFS.txt";
	HFILE fidin = GSSiOpenFile (infile,0,OF_READ);
	HFILE fidout = GSSiOpenFile (outfile,0,OF_CREATE);
	char str[300];
	char outstr[300];
	LPSTR pbeg,pend,prtn;
	int ival;

	fputstring ("NAMEANDARGS(C128)\tIDNUM(B4)\tTYPE(C32)\tDESC(C255)",fidout);
	while (fgetstring (str,256,fidin))
	{
		if ((pbeg = strstr (str,"if (!_fstrnicmp (str,\"")))
		{
			pbeg += strlen ("if (!_fstrnicmp (str,\"");
			pend = strchr (pbeg,'"');
			*pend++ = 0;
			prtn = strstr (pend,"return ");
			prtn += 7;
			ival = atoi (prtn);
			sprintf (outstr,"%s\t%i\t?\tUnknown",pbeg,ival);
			fputstring (outstr,fidout);
		}
	}

	GSSiClose (fidin);
	GSSiClose (fidout);

	sprintf (str,"$GMDIMPORT([%DL]fundefs.gmd,N,%s,1)",outfile);
	ProcessText (str);
	return;
}

int APIENTRY WinMainGMEdit(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	SIZE size;
	LPSTR pFile = strstr(lpCmdLine, "/GMEdit ");
	LPSTR pWnd = strstr(lpCmdLine, "/W ");
	//createFunIDFile ();

	//MessageBox(0, lpCmdLine, 0, MB_OK);
	/*{
		HANDLE FileHandle = OpenExternalDatabase("ODBC|MS Access Database;DBQ=C:\\geomas\\projects\\corners\\tables\\Update_ADA_Curb_Ramp_Inventory.mdb|JEFF");
		//CloseDataFile(FALSE,&FileHandle);
	}*/
	if (pWnd)
	{
		*pWnd = 0;
		pWnd += 3;
		hWndGMEditReturn = (HWND)atoi(pWnd);
		ExpandText(pWnd); //sets test env if included
	}
	if (pFile)
	{
		LPSTR pFS;

		strcpy (fileToEdit,pFile+8);
		ExpandText (fileToEdit);
		if ((pFS = strchr (fileToEdit,'/')))
			*pFS++ = 0;
		Truncate (fileToEdit);
	}
		

	// Initialize global strings
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_GMEDIT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstanceGM (hInstance, nCmdShow))
	{
		return FALSE;
	}
	uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_GMEDIT));

        if (!IsDialogMessage(ghPrintingDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
 	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			if (!IsDialogMessage(ghFindReplaceDlg, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	GSSiGlobFree (&hFile);
	if (hWndGMEditReturn)
	{
		//sprintf(str, "send %ld to %ld", GSSI_GMEDITCOMPLETE, (DWORD)hWndGMEditReturn);
		//MessageBox(0, str, 0, MB_OK);
		PostMessage(hWndGMEditReturn, GSSI_GMEDITCOMPLETE, 0, 0);
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
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProcGMEdit;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GMEDIT_LARGE));
	wcex.hCursor		= LoadCursor(NULL, IDC_IBEAM);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_GMEDIT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_GMEDIT_SMALL));

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
BOOL InitInstanceGM(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW|WS_VSCROLL|WS_HSCROLL,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void adjustInsertLocs (void)
{
	if (insertLoc > insertLoc2 && insertLoc2 > -1)
	{
		int i = insertLoc;

		insertLoc = insertLoc2;
		insertLoc2 = i;
	}
	return;
}

void GMEditDisplayText2 (HWND hWnd,HDC hdc,HANDLE hFile,HFONT hFont,RECT rcPaint,int icBeg,int icEnd)
{
	LPSTR pFile, pFileBegin;
	TEXTMETRIC	tm;
	SCROLLINFO si; 
	SIZE size;
	int rtn = 0;
	int yPos, xPos, rowHeight, rowWidth, x, y, tablen, ic;
	HFONT hOldFont;
 
	GetClientRect(hWnd, &currentRect);
	// Get vertical scroll bar position.
    si.cbSize = sizeof (si);
    si.fMask  = SIF_PAGE|SIF_POS|SIF_RANGE|SIF_TRACKPOS;
    GetScrollInfo (hWnd, SB_VERT, &si);
    yPos = si.nPos;

    // Get horizontal scroll bar position.
    GetScrollInfo (hWnd, SB_HORZ, &si);
    xPos = si.nPos;

	hOldFont = SelectObject (hdc,hFont);
  // Find painting limits.
	if (currentLine > -1 && displayOnlyCurrentLine)
	{
		firstLine = lastLine = currentLine;
	}
	else
	{
		firstLine = max (0, yPos + rcPaint.top / yChar);
		lastLine = min (LINES - 1, yPos + rcPaint.bottom / yChar);
	}

	GetTextMetrics(hdc, &tm);
	rowHeight = tm.tmHeight;
	tablen = tm.tmAveCharWidth * 5;
	if (hFile)
	{
		int line = 0;
		
        x = xChar * (1 - xPos);
        y = yChar * (line - yPos);


		pFile = pFileBegin = GlobalLock (hFile);
		while (*pFile)
		{
			switch (*pFile)
			{
				default:
				if (line >= firstLine)
				{
					if (x > rcPaint.right+xChar+1)
					{
						if (!(pFile = strchr (pFile,'\n')))
							goto PastLastLine;
						x = xChar * (1 - xPos);
						y += rowHeight;
						line++;
						break;
					}
					if (x > rcPaint.left-xChar-1)
					{
						ic = (int)(pFile - pFileBegin);
						if (ic == insertLoc || ic == insertLoc-1)
						{
							currentLine = line;
							currentRect.left = xChar;
							//currentRect.right = INT_MAX;
							currentRect.top = y;
							currentRect.bottom = y + rowHeight;
						}
						if (icBeg > -1)
						{
							if (ic >= icBeg && ic < icEnd)
								TextOut (hdc,x,y,pFile,1);
						}
						else
							TextOut (hdc,x,y,pFile,1);
						//GdiFlush();
					}
					GetTextExtentPoint32(hdc,pFile,1,&size);
					x += size.cx;
				}
				break;
				case '\r':
					x = xChar * (1 - xPos);
					break;
				case '\n':
					x = xChar * (1 - xPos);
					y += rowHeight;
					line++;
					break;
				case '\t':
					x += tablen;
					break;
			}
			pFile++;
			if (line > lastLine)
				break;
		}
PastLastLine:
		GlobalUnlock (hFile);
	}
	SelectObject (hdc,hOldFont);
	return;
}

void GMEditDisplayTextBetweenLocs (HWND hWnd,HANDLE hFile,HFONT hFont,int icBeg,int icEnd,BOOL highlight)
{
	RECT rcPaint;
	COLORREF bkColor, oldBColor;
	COLORREF textColor, oldTColor;

	if (icBeg < 0 || icEnd < 0)
		return;
	if (icBeg != icEnd)
	{
		HDC hdc=GetDC (hWnd);
		int saveLine = currentLine;

		currentLine = -1;
		if (icBeg > icEnd)
		{
			int i = icBeg;
			icBeg = icEnd;
			icEnd = i;
		}
		GetClientRect (hWnd,&rcPaint);
		if (highlight)
		{
			textColor = RGB (255,255,255);
			bkColor = RGB (0,0,255);
		}
		else
		{
			textColor = 0;
			bkColor = RGB (255,255,255);
		}
		oldBColor = SetBkColor (hdc,bkColor);
		oldTColor = SetTextColor (hdc,textColor);
		GMEditDisplayText2 (hWnd,hdc,hFile,hFont,rcPaint,icBeg,icEnd);
		SetBkColor (hdc,oldBColor);
		SetTextColor (hdc,oldTColor);
		ReleaseDC (hWnd,hdc);
		currentLine = saveLine;
	}

	return;
}

void GMEditDisplayText (HWND hWnd,HDC hdc,HANDLE hFile,HFONT hFont,RECT rcPaint)
{
	GMEditDisplayText2 (hWnd,hdc,hFile,hFont,rcPaint,-1,0);
	return;
}

int GetInsertLocFromPoint (HWND hWnd,HANDLE hFile,HFONT hFont,LPPOINT pcursorLoc)
{
	LPSTR pFile, pFileBegin;
	TEXTMETRIC	tm;
	SCROLLINFO si; 
	SIZE size;
	RECT rcPaint;
	int rtn = 0;
	int yPos, xPos, FirstLine, LastLine, rowHeight, x, y, tablen;
	HFONT hOldFont;
	HDC hdc=GetDC (hWnd);
 
	GetClientRect (hWnd,&rcPaint);
	// Get vertical scroll bar position.
    si.cbSize = sizeof (si);
    si.fMask  = SIF_PAGE|SIF_POS|SIF_RANGE|SIF_TRACKPOS;
    GetScrollInfo (hWnd, SB_VERT, &si);
    yPos = si.nPos;

    // Get horizontal scroll bar position.
    GetScrollInfo (hWnd, SB_HORZ, &si);
    xPos = si.nPos;

	hOldFont = SelectObject (hdc,hFont);
  // Find painting limits.
    FirstLine = max (0, yPos + rcPaint.top / yChar);
    LastLine = min (LINES - 1, yPos + rcPaint.bottom / yChar);
     
	GetTextMetrics(hdc, &tm);
	rowHeight = tm.tmHeight;
	tablen = tm.tmAveCharWidth * 5;
	pcursorLoc->x -= xChar/4;
	if (!yPos)
		pcursorLoc->y = max (0,pcursorLoc->y);
	insertLine = 0;
	if (hFile)
	{
		int line = 0;
		int ic = 0;
		
        x = xChar * (0 - xPos);// + xChar/2;
        y = yChar * (line - yPos);


		pFile = pFileBegin = GlobalLock (hFile);
		while (*pFile)
		{
			int inc = xChar;

			switch (*pFile)
			{
			case '\n':
				insertLine++;
			default:
				if (*pFile == '\t')
					inc = tablen;
				if (x <= pcursorLoc->x && x + inc > pcursorLoc->x &&
					y <= pcursorLoc->y && y + yChar > pcursorLoc->y)
					{
						//char c=pFile[10];
						rtn = (int)(pFile - pFileBegin);
						pcursorLoc->y = y;
						pcursorLoc->x = x + xChar;
						goto PastLastLine;
					}
					else if (*pFile == '\r')
					{
						if (y <= pcursorLoc->y && y + yChar > pcursorLoc->y)
						{
							rtn = (int)(pFile - pFileBegin);
							pcursorLoc->y = y;
							pcursorLoc->x = x + xChar;
							goto PastLastLine;
						}
						x = xChar * (0 - xPos);// + xChar/2;
					}
					else if (*pFile == '\n')
					{
						y += rowHeight;
						line++;
					}
					else if (*pFile == '\t')
					{
						x += tablen;
					}
					else
					{
						GetTextExtentPoint32(hdc,pFile,1,&size);
						x += size.cx;
					}
			}
			pFile++;
			if (line > LastLine)
				break;
		}
		rtn = (int)(pFile - pFileBegin);
PastLastLine:
		GlobalUnlock (hFile);
	}
	SelectObject (hdc,hOldFont);
	ReleaseDC (hWnd,hdc);
	return rtn;
}
int FileLocToMacroLoc(int fileLoc)
{
	LPSTR pFile = GlobalLock(hFile);
	LPSTR pFileBegin = pFile;
	int locMacro = 0, locFile = 0;
	BOOL lastWasLineTerm = TRUE;

	while (locFile < fileLoc && *pFile)
	{
		if (*pFile == '#' && lastWasLineTerm)
		{
			while (*pFile != '\r' && *pFile != '\n')
			{
				locFile++;
				pFile++;
			}
		}
		if (*pFile != '\r' && *pFile != '\n')
		{
			if (*pFile != '\t')
				locMacro++;
			lastWasLineTerm = FALSE;
		}
		else
			lastWasLineTerm = TRUE;
		locFile++;
		pFile++;
	}
	GlobalUnlock(hFile);
	return locMacro;
}

BOOL FindBreakpoint (LPSTR bp,LPINT pLoc)
{
	int loc = 0;
	char brkp[64];
	int rtn = 0;

	if (breakAtLoc > -1)
	{
		LPSTR pFile = GlobalLock(hFile);
		LPSTR pFileBegin = pFile;
		int locMacro=0, locFile=0;
		BOOL lastWasLineTerm=TRUE;

		while (*pFile)
		{
			if (*pFile == '#' && lastWasLineTerm)
			{
				while (*pFile != '\r' && *pFile != '\n')
				{
					locFile++;
					pFile++;
				}
			}
			if (locMacro >= breakAtLoc)
			{
				while (*pFile == '\r' || *pFile == '\n')
				{
					locFile++;
					pFile++;
					lastWasLineTerm = TRUE;
				}
				if (*pFile == '#' && lastWasLineTerm)
				{
					while (*pFile != '\n')
					{
						locFile++;
						pFile++;
					}
					locFile++;
				}
				while (*pFile == '\t')
				{
					locFile++;
					pFile++;
				}
				rtn = locFile;
				goto Exit;
			}
			if (*pFile != '\r' && *pFile != '\n')
			{
				if (*pFile != '\t')
					locMacro++;
				lastWasLineTerm = FALSE;
			}
			else
				lastWasLineTerm = TRUE;
			locFile++;
			pFile++;
		}
		rtn = locFile;
	Exit:
		GlobalUnlock(hFile);
		//breakAtLoc = -1;
		*pLoc = rtn;
		return TRUE;
	}
	if (*bp)
	{
		sprintf(brkp, "$B(%s", bp);
		if (hFile)
		{
			LPSTR pFile = GlobalLock(hFile);
			LPSTR pFileBegin = pFile;

			while (*pFile)
			{
				if (!strncmp(pFile, brkp, strlen(brkp)) &&
					(*(pFile + strlen(brkp)) == ',' ||
					*(pFile + strlen(brkp)) == ')'))
				{
					loc = (int)(pFile - pFileBegin);
					break;
				}
				pFile++;
			}
			GlobalUnlock(hFile);
		}
		*pLoc = loc;
		return TRUE;
	}
	return FALSE;
}

void verifyLineBreaks (HANDLE hFile)
{
	LPSTR pFile = GlobalLock (hFile);
	LPSTR pFileBeg = pFile;
	int ln;

top:
	pFile = pFileBeg;
	if (*pFile == '\n')
		ii=1;
	pFile++;
	while (*pFile)
	{
		if (*pFile == '\r' && *(pFile+1) != '\n')
		{
			ln = strlen (pFile);
			memmove (pFile+2,pFile+1,ln);
			*(pFile+1) = '\n';
			goto top;
		}
		if (*pFile == '\n' && *(pFile-1) != '\r')
		{
			ln = strlen (pFile);
			memmove (pFile+1,pFile,ln);
			*pFile = '\r';
			goto top;
		}
		pFile++;
	}
	GlobalUnlock (hFile);
	return;
}

int InsertCharAtLoc (HWND hWnd,char key,int insertLoc,int *pinsertLoc2,HANDLE hFile)
{
	if (!hFile)
		return 0;
	else
	{
		LPSTR pFile = GlobalLock(hFile);
		int lFile = strlen(pFile);
		int idiff;

		if (insertLoc < 0)
			return insertLoc;
		if (*pinsertLoc2 < 0)
			*pinsertLoc2 = insertLoc;
		if (insertLoc > *pinsertLoc2)
		{
			int i = insertLoc;
			insertLoc = *pinsertLoc2;
			*pinsertLoc2 = i;
		}
		idiff = *pinsertLoc2 - insertLoc;
		if (idiff)
		{
			memmove(&pFile[insertLoc], &pFile[*pinsertLoc2], lFile - idiff - insertLoc + 1);
			lFile -= idiff;
		}
		if (key == VK_BACK)
		{
			if (lFile && insertLoc > 0)
			{
				if (pFile[insertLoc - 1] == '\n')
				{
					if (insertLoc > 1 && pFile[insertLoc - 2] == '\r')
					{
						memmove(&pFile[insertLoc - 1], &pFile[insertLoc], lFile - insertLoc + 1);
						insertLoc--;
					}
				}
				memmove(&pFile[insertLoc - 1], &pFile[insertLoc], lFile - insertLoc + 1);
				insertLoc--;
			}
		}
		else if (key == '\x18') //delete
		{
			if (insertLoc < lFile && pFile[insertLoc] == '\n')
			{
				memmove(&pFile[insertLoc], &pFile[*pinsertLoc2], lFile - idiff - insertLoc + 1);
				lFile -= idiff;
			}
		}
		else
		{
			if (lFile > insertLoc)
				memmove(&pFile[insertLoc + 1], &pFile[insertLoc], lFile - insertLoc + 1);
			else
				pFile[insertLoc + 1] = 0;
			pFile[insertLoc++] = key;
		}
		GlobalUnlock(hFile);
		*pinsertLoc2 = insertLoc;
		changesMade = TRUE;
		EnableMenuItem(GetMenu(hWnd), IDM_FILE_SAVE, MF_ENABLED);
		return insertLoc;
	}
}

int GetInsertPointFromLoc (HWND hWnd,HANDLE hFile,HFONT hFont,LPPOINT pcursorLoc,int insertLoc,int inc)
{
	LPSTR pFile, pFileBegin;
	TEXTMETRIC	tm;
	SCROLLINFO siH,siV; 
	SIZE size;
	RECT rcPaint;
	int rtn = 0;
	int yPos, xPos, rowHeight, x, y, tablen;
	HFONT hOldFont;
	HDC hdc=GetDC (hWnd);
 
	GetClientRect (hWnd,&rcPaint);
	// Get vertical scroll bar position.
	siV.cbSize = sizeof (siV);
	siV.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	siH.cbSize = sizeof (siH);
	siH.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo(hWnd, SB_VERT, &siV);
    yPos = siV.nPos;

    // Get horizontal scroll bar position.
    GetScrollInfo (hWnd, SB_HORZ, &siH);
    xPos = siH.nPos;

	hOldFont = SelectObject (hdc,hFont);
     
	GetTextMetrics(hdc, &tm);
	rowHeight = tm.tmHeight;
	tablen = tm.tmAveCharWidth * 5;
	//pcursorLoc->x += xChar/2;
	if (hFile)
	{
		int line = 0;
		int ic = 0;
		
        x = xChar * (0 - xPos);
        y = yChar * (line - yPos);


		pFile = pFileBegin = GlobalLock (hFile);
		insertLine = 0;
		while (*pFile)
		{
			switch (*pFile)
			{
			default:
				if (ic >= insertLoc)
				{
					//char c=pFile[10];
					if (*pFile == '\n' && inc > 0)
					{
						x = xChar * (0 - xPos);
						insertLine++;
						y += rowHeight;
						line++;
					}
					rtn = (int)(pFile - pFileBegin);
					pcursorLoc->y = y;
					pcursorLoc->x = x + xChar;
					//pFile[10]=0;
					//SetWindowText (hWnd,pFile);
					//pFile[10]=c;
					goto PastLastLine;
				}
				else if (*pFile == '\r')
				{
					ii = 1;
				}
				else if (*pFile == '\n')
				{
					x = xChar * (0 - xPos);
					insertLine++;
					y += rowHeight;
					line++;
				}
				else if (*pFile == '\t')
				{
					x += tablen;
				}
				else
				{
					GetTextExtentPoint32(hdc,pFile,1,&size);
					x += size.cx;
				}
				ic++;
			}
			pFile++;
		}
		pcursorLoc->y = y;
		pcursorLoc->x = x;
PastLastLine:
		GlobalUnlock (hFile);
	}
	SelectObject (hdc,hOldFont);
	ReleaseDC (hWnd,hdc);
	return rtn;
}

void getLinesAndMaxLine (HDC hdc,HANDLE hFile,HFONT hFont)
{
	HFONT hOldFont = SelectObject (hdc,hFont);

	if (hFile)
	{
		SIZE size;
		LPSTR pFile = GlobalLock (hFile);
		LPSTR pStartLine=pFile;
		int maxWidth = 0;
		int x=0;

		LINES = 1;

		while (*pFile)
		{
			switch (*pFile)
			{
			case '\r':
				maxWidth = max (x,maxWidth);
				x = 0;
				break;
			case '\n':
				LINES++;
				break;
			case '\t':
				x += xChar * 5;
				break;
			default:
				GetTextExtentPoint32(hdc,pFile,1,&size);
				x += size.cx;
				break;
			}
			pFile++;
		}

		maxLine = maxWidth / xChar;
		GlobalUnlock (hFile);
	}
	SelectObject (hdc,hOldFont);
	return;
}
BOOL setScroll (HWND hWnd)
{
	SCROLLINFO six, siy; 
	RECT rect;
	int yClient, xClient;
	int xPos, yPos;

	GetClientRect (hWnd,&rect);
    // Retrieve the dimensions of the client area. 
    yClient = RECTHEIGHT (&rect); 
    xClient = RECTWIDTH (&rect); 

    // Set the vertical scrolling range and page size
	memset(&siy, 0, sizeof(siy));
    siy.cbSize = sizeof(siy); 
    siy.fMask  = SIF_RANGE | SIF_PAGE; 
    siy.nMin   = 0; 
    siy.nMax   = LINES - 1; 
    siy.nPage  = yClient / yChar; 
    SetScrollInfo(hWnd, SB_VERT, &siy, TRUE); 

    // Set the horizontal scrolling range and page size. 
	memset(&six, 0, sizeof(six));
	six.cbSize = sizeof(six);
    six.fMask  = SIF_RANGE | SIF_PAGE; 
    six.nMin   = 0; 
    six.nMax   = maxLine;//2 + 2*xClientMax / xChar; 
    six.nPage  = xClient / xChar; 
    SetScrollInfo(hWnd, SB_HORZ, &six, TRUE); 
    six.fMask = SIF_POS;
    GetScrollInfo (hWnd, SB_HORZ, &six);
    siy.fMask = SIF_POS;
    GetScrollInfo (hWnd, SB_VERT, &siy);
	yPos = insertLine;
	xPos = insertPoint.x/xChar;
	if (setToFind &&
		((yPos < siy.nPos || yPos > siy.nPos+siy.nPage)
		||
 		(xPos < six.nPos || xPos > six.nPos+six.nPage)))
   {
		six.nPos = xPos-six.nPage/2;
		siy.nPos = yPos-siy.nPage/2;
		SetScrollInfo(hWnd, SB_HORZ, &six, TRUE); 
		GetScrollInfo (hWnd, SB_HORZ, &six);
		SetScrollInfo(hWnd, SB_VERT, &siy, TRUE); 
		GetScrollInfo (hWnd, SB_VERT, &siy);
		ScrollWindow(hWnd, xPos - six.nPos,max(0, min(LINES - siy.nPage, yPos - siy.nPos)), NULL, NULL);
		GetInsertPointFromLoc (hWnd,hFile,hFont,&insertPoint,insertLoc,0);
		SetCaretPos(insertPoint.x, insertPoint.y); 
		setToFind = FALSE;

		return TRUE;
    }
	return FALSE;
}

void AutoInsertOpen (void)
{
	char FunDefFile[]="[%DL]fundefs.gmd";
	hFunDefDB = OpenGWDatabase (FunDefFile,BT_READ);

	return;
}
void AutoInsertClose (void)
{
	CloseGWDatabase (hFunDefDB);
	hFunDefDB = 0;
	GSSiGlobFree (&hInsertOpts);
	return;
}
void AutoInsert (HWND hWnd,HANDLE hFile,HMENU *phMenu)
{
	LPGWDHEADER lpGWDHead;
	long	Offset;
	LPFUNDEFS pFunDefs;
	LPSTR	pFile, pBeg;
	char	wantKey[130];
	int		loc, ibeg, iend, keypos, endkey=128, lkey=0, ln;
	char	nextKey[130];
	char	str[1024];
	char testFile[MAX_PATH + 128];
	int iMenuOpt = 60000;
	LPSTR pInsertOpts;

	if (*phMenu)
	    DestroyMenu (*phMenu);  
	*phMenu = 0;
	if (!hFile ||insertLoc < 0)
		return;

	GSSiGlobFree (&hInsertOpts);
	hInsertOpts = GSSiGlobAlloc (1785,GHND,USHRT_MAX);
	pInsertOpts = GlobalLock (hInsertOpts);
	pFile = GlobalLock (hFile);

	if (insertLoc >= 0 && insertLoc2 >= insertLoc)
	{
		strncpy0(testFile, &pFile[insertLoc], min(sizeof(testFile)-1, insertLoc2 - insertLoc));
		pBeg = testFile;
		if (strchr(pBeg, '[') || strchr(pBeg, '$'))
		{
			sprintf(str, "|$EXPAND(%s", pBeg);
			strcpy(pInsertOpts, str);
			pInsertOpts = strchr(pInsertOpts, 0);
			pInsertOpts++;
			sprintf(str, "Expand:%s", pBeg);
			if (!*phMenu)
				*phMenu = CreatePopupMenu();
			AppendMenu(*phMenu, MF_ENABLED | MF_STRING, iMenuOpt++, str);
		}
	}

	if (hFunDefDB) //check for $function matches
	{
		loc = insertLoc-1;
		keypos = endkey;
		wantKey[keypos] = 0;
		wantKey[keypos+1] = 0;
		while (lkey == 0 && loc >= 0 && keypos--)
		{
			switch (*&pFile[loc])
			{
			case '$':
				lkey = endkey - keypos;
				break;
			case '\r':
			case '\n':
				lkey = -1;
				break;
			default:
				wantKey[keypos] = pFile[loc];
				loc--;
				break;
			}
		}
		if (lkey > 1)
		{
			int inc = insertLoc - loc;

			keypos++;
			lkey--;
			lpGWDHead = (LPGWDHEADER)GlobalLock (hFunDefDB); 
			pFunDefs = (LPFUNDEFS)&lpGWDHead->GWDData;
			SetFieldValFromCharAndName(lpGWDHead,"NameAndArgs",(LPSTR)&wantKey[keypos],TRUE);
			GWDFormKey(lpGWDHead,0,TRUE,0,0);
			while (!BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_FIRST,BT_GE, (LPSTR)&Offset))
			{
				FillGWDData (lpGWDHead,Offset); 
				if (strnicmp (pFunDefs->NameAndArgs,&wantKey[keypos],lkey))
					break;
				if (!*phMenu)
					*phMenu = CreatePopupMenu ();
				sprintf (str,"$%s(",lpGWDHead->pKeys[0]);
				strcpy (pInsertOpts,&str[inc]);
				pInsertOpts = strchr (pInsertOpts,0);
				pInsertOpts++;
				AppendMenu (*phMenu,MF_ENABLED|MF_STRING,iMenuOpt++,str);
				ln = strlen (lpGWDHead->pKeys[0]);
				strncpy (nextKey,lpGWDHead->pKeys[0],lpGWDHead->lKeys[0]);
				nextKey[ln]++;
				strncpy (lpGWDHead->pKeys[0],nextKey,lpGWDHead->lKeys[0]);
			}
			GlobalUnlock (hFunDefDB);
		}
	}
	{	//check for file matches

		if (insertLoc == insertLoc2)
		{
			loc = insertLoc-1;
			while (loc)
			{
				switch (*&pFile[loc])
				{
				case '$':
					goto s20;
				case '(':
				case ',':
				case '\r':
				case '\t':
				case '{':
				case '\n':
				case ';':
					goto s10;
				case '=':
					if (loc && *&pFile[loc-1] == ']')
						goto s10;
					//check for @;DBQ=
					if (loc > 6 && !strnicmp (&pFile[loc-5],"@;DBQ=",6))
						loc-=5;
				default:
					loc--;
					break;
				}
			}
	s10:
			loc++;
			iend = strcspn (&pFile[loc],",)};!\r\n\t");
			if (!strnicmp (&pFile[loc+iend],";DBQ=",5))
				iend += 5 + strcspn (&pFile[loc+iend+5],",);!\r\n");
			strncpy0 (testFile,&pFile[loc],min(sizeof(testFile)-1,iend));
		}
		else
		{
			adjustInsertLocs ();
			strncpy0 (testFile,&pFile[insertLoc],min(sizeof(testFile)-1,insertLoc2-insertLoc));
		}
		if ((pBeg = strchr (testFile,'=')))
		{
			if ((int)(pBeg - testFile) > 5 &&
				!strnicmp (pBeg-4,";DBQ=",5))
				pBeg = testFile;
			else
				pBeg++;
		}
		else
			pBeg = testFile;


		if (!strncmp(pBeg, "ODBC|", 5) || FileType(pBeg) == 1 || strstr (pBeg,".GDB"))
		{
			HANDLE hDB=0;
			int itype = OpenDataFile (testFile,"",OF_READ,&hDB);

			if (itype == GMTEXT_DATAFILE ||
				!itype && StrStrI (testFile,".txt"))
			{
				if (hDB && GSSiLength (pBeg) > MAX_FILE_SIZE)
					sprintf (str,"|$EDITFILE(%s)",pBeg);
				else
					sprintf (str,"|$SESSION(CREATE,GMEdit /GMEdit %s)",pBeg);
				strcpy (pInsertOpts,str);
				pInsertOpts = strchr (pInsertOpts,0);
				pInsertOpts++;
				sprintf (str,"Edit file %s",pBeg);
				if (!*phMenu)
					*phMenu = CreatePopupMenu ();
				AppendMenu (*phMenu,MF_ENABLED|MF_STRING,iMenuOpt++,str);
			}
			if (itype == ODBC_DATAFILE && GetNumDBFields(hDB) < 1)
			{
				HMENU hSubMenu=0;
				int	  iSubMenuItem = 1;
				HANDLE DBHandle = GetDBHandleFromSQL(hDB);
				LPSTR lpSTRING = GetTableName(DBHandle, TRUE,itype); // the first table name 
				if (lpSTRING && *lpSTRING)
					hSubMenu = CreatePopupMenu();
				while (lpSTRING && *lpSTRING)
				{
					AppendMenu(hSubMenu, MF_ENABLED | MF_STRING, iSubMenuItem++, lpSTRING);
					lpSTRING = GetTableName(DBHandle, FALSE,itype); // subsequent table names 
				}
				if (hSubMenu)
				{
					POINT menuPoint = insertPoint;
					menuPoint.y += yChar;
					ClientToScreen(hWnd, &menuPoint);
					AppendMenu(hSubMenu, MF_ENABLED | MF_STRING, 0, "Cancel");
					iSubMenuItem = TrackPopupMenu(hSubMenu, TPM_RETURNCMD | TPM_NONOTIFY|TPM_LEFTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN, menuPoint.x, menuPoint.y, 0, hWnd, 0);
					if (iSubMenuItem > 0)
					{
						TCHAR tableName[128];

						if (GetMenuString(hSubMenu, iSubMenuItem, tableName, sizeof(tableName)-1, MF_BYCOMMAND) > 0)
						{
							CloseDataFile(FALSE, &hDB);
							sprintf(strchr(testFile, 0), "|%s", tableName);
							OpenDataFile(testFile, "", OF_READ, &hDB);
						}
					}
					DestroyMenu(hSubMenu);
				}
			}

			if (itype && GetNumDBFields(hDB) > 1)
			{
				sprintf(str, "|$FIELDS(%s)", testFile);
				strcpy(pInsertOpts, str);
				pInsertOpts = strchr(pInsertOpts, 0);
				pInsertOpts++;
				sprintf(str, "Display Fields for %s", pBeg);
				if (!*phMenu)
					*phMenu = CreatePopupMenu();
				AppendMenu(*phMenu, MF_ENABLED | MF_STRING, iMenuOpt++, str);
				sprintf(str, "|$DATADISPLAY(BASIC,%s)", pBeg);
				strcpy(pInsertOpts, str);
				pInsertOpts = strchr(pInsertOpts, 0);
				pInsertOpts++;
				sprintf(str, "Open %s in DATADISPLAY", pBeg);
				AppendMenu(*phMenu, MF_ENABLED | MF_STRING, iMenuOpt++, str);
			}
			CloseDataFile (FALSE,&hDB);
		}
		else if (FileType (pBeg) == 2)
		{
			sprintf (str,"|$WEB(%s)",pBeg);
			strcpy (pInsertOpts,str);
			pInsertOpts = strchr (pInsertOpts,0);
			pInsertOpts++;
			sprintf (str,"Open directory %s",pBeg);
			if (!*phMenu)
				*phMenu = CreatePopupMenu ();
			AppendMenu (*phMenu,MF_ENABLED|MF_STRING,iMenuOpt++,str);
		}
	}
s20: //check for variable matches
	if (*&pFile[insertLoc-1] == '.')
	{
		loc = insertLoc-2;
		while (loc)
		{
			switch (*&pFile[loc])
			{
			case '$':
			case '=':
			case ']':
			case '(':
			case ',':
			case '\r':
			case '\n':
			case ';':
			case '.':
				goto s40;
			case '[':
				goto s30;
			default:
				loc--;
				break;
			}
		}
s30:
		{
			HANDLE hDB;
			char csDBid[64];

			int i;
			loc++;
			strncpy0 (csDBid,&pFile[loc],insertLoc-loc-1);
			if ((hDB = GetOpenDatabaseFromID(csDBid)))
			{
				LPFIELDINFO lpFieldInfo;  
				LPOPENSQLDATA SQLPtr;
				LPOPENFILEDATA FilePtr;

				if (!*phMenu)
					*phMenu = CreatePopupMenu ();
				SQLPtr = (LPOPENSQLDATA)GlobalLock (hDB);
				FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
				lpFieldInfo = &FilePtr->FldInfo; 

				for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++) 
				{   
	        		sprintf (str,"%s]",lpFieldInfo->name); 
					strcpy (pInsertOpts,str);
					pInsertOpts = strchr (pInsertOpts,0);
					pInsertOpts++;
					AppendMenu (*phMenu,MF_ENABLED|MF_STRING,iMenuOpt++,lpFieldInfo->name);
				}           
				GlobalUnlock (SQLPtr->OFHandle);
				GlobalUnlock (hDB);
			}
		}
	}
s40:

	GlobalUnlock (hInsertOpts);
	GlobalUnlock (hFile);
	if (*phMenu)
	{
		POINT menuPoint = insertPoint;
		menuPoint.y += yChar;
		ClientToScreen (hWnd,&menuPoint);
		AppendMenu (*phMenu,MF_ENABLED|MF_STRING,1,"Cancel");
		TrackPopupMenu (*phMenu,TPM_LEFTBUTTON|TPM_TOPALIGN|TPM_LEFTALIGN,menuPoint.x,menuPoint.y,0,hWnd,0);
	}
	return;
}

BOOL CopyToClipboard (HANDLE hWnd,HANDLE hFile,int ib,int ie)
{
 	BOOL	KeepMemLengthSave=KeepMemLength;
	HANDLE	hStr;
	LPSTR	str;
	LPSTR	pFile;
	int		ln;

	if (!hFile)
		return FALSE;
	if (ib < 0 || ie < 0 || ib == ie)
		return FALSE;
	if (ib > ie)
	{
		int i=ib;
		ib = ie;
		ie = i;
	}
	ln = ie-ib;
  	if (!OpenClipboard (hWnd))
		return FALSE;
	EmptyClipboard();
	
	pFile = GlobalLock (hFile);
   	KeepMemLength = TRUE;  
    hStr = GSSiGlobAlloc (1228,GHND,ln+1);  
    KeepMemLength = KeepMemLengthSave;
   	str = GlobalLock (hStr);  
   	strncpy0 (str,&pFile[ib],ln);
	GlobalUnlock (hStr);
	#if CHECKMEM
		GSSiRemoveMem (hStr);
	#endif
	SetClipboardData(CF_TEXT, hStr); 
    CloseClipboard();
	GlobalUnlock (hFile);

	return TRUE;
}

BOOL putInsertPointOnScreen (HWND hWnd)
{
	BOOL rtn = FALSE;

	setToFind = TRUE;
	if (setScroll (hWnd))
	{
		rtn = TRUE;
		InvalidateRect (hWnd,0,TRUE);
	}
	return rtn;
}

BOOL GMEditSaveUpdates (HWND hWnd)
{
	HFILE fid=GSSiOpenFile (fileToEdit,0,OF_CREATE);
	int lnFile;

	if (fid == HFILE_ERROR)
	{
		sprintf (str,"Unable to open file for write access:\r\n%s",fileToEdit);
		MessageBox (hWnd,str,0,MB_ICONEXCLAMATION);
		return FALSE;
	}
	pFile = GlobalLock (hFile);
	lnFile = strlen (pFile);
	BigWrite (fid,pFile,lnFile,-1);
	GSSiClose (fid);
	GlobalUnlock (hFile);
	return TRUE;
}

LRESULT CALLBACK WndProcGMEdit(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
	int fontSize=18;
	int	insertLoc3;
	int cursorWidth=5, cursorHeight = fontSize-2;
	int saveLine;
	POINT mousePoint;
	RECT rect;
	char key;
	static HMENU hMenu=0;
	int loc;


	if (message == 	uFindReplaceMsg)
	{ 
		LPFINDREPLACE lpfr;
	// Get pointer to FINDREPLACE structure from lParam.
		lpfr = (LPFINDREPLACE)lParam;

    // If the FR_DIALOGTERM flag is set, 
    // invalidate the handle that identifies the dialog box. 
		if (lpfr->Flags & FR_DIALOGTERM)
		{ 
			EnableMenuItem(GetMenu(lpfr->hwndOwner), IDM_FIND, MF_ENABLED);
			EnableMenuItem(GetMenu(lpfr->hwndOwner), IDM_FINDANDREPLACE, MF_ENABLED);
			return 0; 
		} 

		// If the FR_FINDNEXT flag is set, 
		// call the application-defined search routine
		// to search for the requested string. 
		if (lpfr->Flags & FR_FINDNEXT && hFile) 
		{
			 LPSTR pFile = GlobalLock (hFile);
			 LPSTR pFound;
			 int  inc=0;

			 if (!strnicmp (&pFile[insertLoc],lpfr->lpstrFindWhat,strlen(lpfr->lpstrFindWhat)) &&
				 insertLoc2 - insertLoc == strlen (lpfr->lpstrFindWhat))
				 inc = 1;
			 if ((pFound = StrStrI(&pFile[insertLoc + inc], lpfr->lpstrFindWhat)))
			 {
				 insertLoc = (int)(pFound - pFile);
				 insertLoc2 = insertLoc + strlen(lpfr->lpstrFindWhat);
			 }
			 else
				 MessageBoxAtPosition(hWnd, "The specified text was not found", "Find", MB_OK,"C");
			 GetInsertPointFromLoc (hWnd,hFile,hFont,&insertPoint,insertLoc,0);
			 SetCaretPos(insertPoint.x, insertPoint.y); 
			 GlobalUnlock (hFile);
			 setToFind = TRUE;
			 setScroll (hWnd);
			 InvalidateRect (hWnd,0,TRUE);
		}
		else if (lpfr->Flags & FR_REPLACE && hFile) 
		{
			 LPSTR pFile = GlobalLock (hFile);
			 LPSTR pFound;
			 int  inc=0;
			 LPSTR pText = lpfr->lpstrReplaceWith;

 			 while (*pText)
				insertLoc = InsertCharAtLoc (hWnd,*pText++,insertLoc,&insertLoc2,hFile);
			 GetInsertPointFromLoc (hWnd,hFile,hFont,&insertPoint,insertLoc,0);
			 SetCaretPos(insertPoint.x, insertPoint.y); 
			 GlobalUnlock (hFile);
			 InvalidateRect (hWnd,0,TRUE);
		}
		
		return 0; 
	}


	switch (message)
	{
    case WM_CREATE: 
        if (!hWndMain)
		{
			hWndMain = hWnd;
			CDInit (hWnd, hInst); /* Initialize Common Dialogs */     
			CurView = (LPVIEWPORT)malloc (sizeof(VIEWPORT));
			CurView->hWnd = hWnd;
			standAlone = TRUE;
		}
		else if (!uFindReplaceMsg)
			uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);

		hdc = GetDC (hWnd); 
 
		changesMade = FALSE;
		hFont = CreateFont(fontSize, 0, 0, 0, FW_MEDIUM,0, 0, 0, 0, OUT_TT_PRECIS, 0, PROOF_QUALITY, 0,"Courier New");   
		hOldFont = SelectObject (hdc,hFont);
        // Extract font dimensions from the text metrics. 
        GetTextMetrics (hdc, &tm); 
        xChar = tm.tmAveCharWidth; 
        xUpper = (tm.tmPitchAndFamily & 1 ? 3 : 2) * xChar/2; 
        yChar = tm.tmHeight + tm.tmExternalLeading; 
 
		insertPoint.x = insertPoint.y = 0;
		insertLoc = insertLoc2 = 0;
        // Set an arbitrary maximum width for client area. 
        // (xClientMax is the sum of the widths of 48 average 
        // lowercase letters and 12 uppercase letters.) 
        xClientMax = 48 * xChar + 12 * xUpper; 
		if (*fileToEdit)
		{
			HFILE fid=GSSiOpenFile (fileToEdit,0,OF_READ);
			
			if (fid != HFILE_ERROR)
			{
				int ln, nchr;
				UINT totWidth=0, totChr=0;
				
				sprintf (str,"GMEdit %s",fileToEdit);
				SetWindowText (hWnd,str);
				LINES = 1;
				lFile = GSSifilelength (fid);
				ln = lFile;
				hFile = GSSiGlobAlloc (1786,GMEM_MOVEABLE,MAX_FILE_SIZE+4);
				pFile = GlobalLock (hFile);
				while (fgetstring (pFile,ln,fid))
				{
					ln = lFile - GSSillseek (fid,0,1);
					nchr = strlen (pFile);
					maxLine = max (maxLine,nchr);
					GetTextExtentPoint32(hdc,pFile,nchr,&size);
					totWidth += size.cx;
					totChr += nchr;
					LINES++;
				}
				GSSillseek (fid,0,0);
				BigRead (fid,pFile,lFile);
				ln = strlen (pFile);
				if (ln < 2 || (pFile[ln-2] != '\r' || pFile[ln-1] != 'n'))
					strcat (pFile,"\r\n");
				GSSiClose (fid);
				GlobalUnlock (hFile);
				avCharWidth = (double)totWidth/(double)totChr;
				verifyLineBreaks (hFile);

			}
		}
         // Free the device context. 
 		SelectObject (hdc,hOldFont);
        ReleaseDC (hWnd, hdc); 
		AutoInsertOpen ();
		GetWindowRect (hWnd,&rect);
		if (FindBreakpoint (currentBreakpoint,&loc))
		{
			 setScroll(hWnd);
			 insertLoc2 = insertLoc = loc;
			 GetInsertPointFromLoc (hWnd,hFile,hFont,&insertPoint,insertLoc,0);
			 SetCaretPos(insertPoint.x, insertPoint.y); 
			 setToFind = TRUE;
			 InvalidateRect (hWnd,0,TRUE);
		}
        return 0; 
	 case WM_DESTROY:
		if (changesMade)
		{
			int st;

			sprintf(str, "Do you want to save changes to: %s?", fileToEdit);
			st = MessageBoxAtPosition(hWnd, str, "GMEdit", MB_YESNOCANCEL, "W");

			if (st == IDYES)
			{
				GMEditSaveUpdates(hWnd);
			}
		}
		GSSiDeleteObject(&hFont);
		AutoInsertClose ();
		GSSiGlobFree (&hFile);
		GSSiGlobFree (&hFChunk);
		GSSiGlobFree (&hFRChunk);
		if (standAlone)
		{
			FreeBigMem ();
			free (CurView);
			PostQuitMessage(0);
		}
		break;

	case WM_CLOSE:
		if (changesMade)
		{
			int st;

			sprintf(str, "Do you want to save changes to: %s?", fileToEdit);
			st = MessageBoxAtPosition(hWnd, str, "GMEdit", MB_YESNOCANCEL, "C");

			if (st == IDCANCEL)
				return 0;
			if (st == IDYES)
			{
				GMEditSaveUpdates(hWnd);
			}
			changesMade = FALSE;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	
	case WM_TIMER:
		ShowWindow (hWnd,SW_SHOW);
		KillTimer (hWnd,1);
		ii=1;
		break;
	case WM_PAINT:
		setScroll(hWnd);
		hdc = BeginPaint(hWnd, &ps);
		GMEditDisplayText (hWnd,hdc,hFile,hFont,ps.rcPaint);
		GMEditDisplayTextBetweenLocs (hWnd,hFile,hFont,insertLoc,insertLoc2,TRUE);
		getLinesAndMaxLine (hdc,hFile,hFont);
		EndPaint(hWnd, &ps);
		DrawMenuBar(hWnd);
		if (FindBreakpoint(currentBreakpoint,&loc))
		{
			insertLoc2 = insertLoc = loc;
			GetInsertPointFromLoc(hWnd, hFile, hFont, &insertPoint, insertLoc, 0);
			SetCaretPos(insertPoint.x, insertPoint.y);
		}
		break;

     case WM_CHAR:
		if (!hFile)
			 break;
		key = wParam;
		displayOnlyCurrentLine = FALSE;
		saveLine = currentLine;
		currentLine = -1;
		switch (key)
		{
		case VK_BACK:
			saveLine = -1;
		default:
			displayOnlyCurrentLine = TRUE;
			currentLine = saveLine;
			insertLoc = InsertCharAtLoc(hWnd, key, insertLoc, &insertLoc2, hFile);
			GetInsertPointFromLoc (hWnd,hFile,hFont,&insertPoint,insertLoc,0);
			SetCaretPos(insertPoint.x, insertPoint.y); 
			if (currentLine > -1)
				InvalidateRect (hWnd,&currentRect,TRUE);
			else
				InvalidateRect(hWnd, 0, TRUE);
			break;
		case 27: //escape
			break;
		case 6: //cntl F
	        PostMessage(hWnd, WM_COMMAND, MAKEWPARAM (IDM_FIND,0),0L); 
			break;
		case 18: //cntl R
	        PostMessage(hWnd, WM_COMMAND, MAKEWPARAM (IDM_FINDANDREPLACE,0),0L); 
			break;
		case '\x16': //paste
  			if (OpenClipboard (hWnd))
			{
				HANDLE hText = GetClipboardData(CF_TEXT);
				int lText, lText2;
			   	KeepMemLength = TRUE; 
				if (hText)
				{
					LPSTR pText;

					IgnoreLock = TRUE;
					pText = GlobalLock (hText);
					lText = strlen (pText);
					GlobalUnlock (hText);
					IgnoreLock = FALSE;
					if (lText)
					{
						HANDLE hText2 = GSSiGlobAlloc (1784,GMEM_MOVEABLE,lText+1);
						LPSTR  pText2 = GlobalLock (hText2);

						IgnoreLock = TRUE;
						pText = GlobalLock (hText);
						lText2 = strlen (pText);
						if (lText2 == lText) //clipboard could have changed
							strcpy (pText2,pText);
						GlobalUnlock (hText);
						IgnoreLock = FALSE;
						if (lText2 == lText)
						{
							while (*pText2)
								insertLoc = InsertCharAtLoc (hWnd,*pText2++,insertLoc,&insertLoc2,hFile);
						}
						GSSiGlobUlFree (&hText2);
						verifyLineBreaks (hFile);
						GetInsertPointFromLoc (hWnd,hFile,hFont,&insertPoint,insertLoc,0);
						SetCaretPos(insertPoint.x, insertPoint.y); 
						InvalidateRect (hWnd,0,TRUE);
					}
				}
			    CloseClipboard();
			}
			break;
		case '\x03': //copy
			CopyToClipboard (hWnd,hFile,insertLoc,insertLoc2);
			break;
		case '\x18': //cut
			CopyToClipboard (hWnd,hFile,insertLoc,insertLoc2);
			insertLoc = InsertCharAtLoc (hWnd,key,insertLoc,&insertLoc2,hFile);
			verifyLineBreaks (hFile);
			SetCaretPos(insertPoint.x, insertPoint.y); 
			if (!putInsertPointOnScreen (hWnd))
				InvalidateRect (hWnd,0,TRUE);
			break;
		case VK_RETURN:
			insertLoc = InsertCharAtLoc (hWnd,'\r',insertLoc,&insertLoc2,hFile);
			insertLoc = InsertCharAtLoc (hWnd,'\n',insertLoc,&insertLoc2,hFile);
			GetClientRect (hWnd,&rect);
			GetInsertPointFromLoc (hWnd,hFile,hFont,&insertPoint,insertLoc,0);
			SetCaretPos(insertPoint.x, insertPoint.y); 
			if (!putInsertPointOnScreen (hWnd))
				InvalidateRect (hWnd,0,TRUE);
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
			currentLine = -1;
	        GetScrollInfo (hWnd, SB_HORZ, &si);
			ScrollWindow(hWnd,- si.nPos,0, NULL, NULL);
			insertPoint.x = -(si.nPos * xChar) + xChar/2;
			insertLoc =  insertLoc2 = GetInsertLocFromPoint (hWnd,hFile,hFont,&insertPoint);
			SetCaretPos(insertPoint.x, insertPoint.y); 
			setToFind = TRUE;
			setScroll (hWnd);
			if (!putInsertPointOnScreen (hWnd))
				InvalidateRect (hWnd,0,TRUE);
			break;
		case VK_END:
			currentLine = -1;
			insertPoint.x = 32000;
			insertLoc =  insertLoc2 = GetInsertLocFromPoint (hWnd,hFile,hFont,&insertPoint);
			SetCaretPos(insertPoint.x, insertPoint.y); 
			setToFind = TRUE;
			setScroll (hWnd);
			putInsertPointOnScreen (hWnd);
			if (!putInsertPointOnScreen (hWnd))
				InvalidateRect (hWnd,0,TRUE);
			break;
		case VK_DELETE:
			currentLine = -1;
			if (hFile)
			{
				int ln;
				pFile = GlobalLock (hFile);
				ln = strlen (pFile);
				if (insertLoc < ln-1)
				{
					if (insertLoc2 <= insertLoc)
						insertLoc2 = insertLoc+1;
					insertLoc = InsertCharAtLoc (hWnd,'\x18',insertLoc,&insertLoc2,hFile);
					SetCaretPos(insertPoint.x, insertPoint.y); 
					InvalidateRect (hWnd,0,TRUE);
				}
				GlobalUnlock (hFile);
			}
			break;
		case VK_F1:
			currentLine = -1;
			AutoInsert(hWnd, hFile, &hMenu);
			break;
		case VK_LEFT:
			currentLine = -1;
			insertLoc = insertLoc2 = max(0, insertLoc - 1);
			GetInsertPointFromLoc (hWnd,hFile,hFont,&insertPoint,insertLoc,-1);
			insertLoc =  insertLoc2 = GetInsertLocFromPoint (hWnd,hFile,hFont,&insertPoint);
			SetCaretPos(insertPoint.x, insertPoint.y); 
			//AutoInsert (hWnd,hFile,insertLoc,&hMenu);
			break;
		case VK_RIGHT:
			currentLine = -1;
			insertLoc = insertLoc2 = min(strlen(pFile) - 1, insertLoc + 1);
			GetInsertPointFromLoc (hWnd,hFile,hFont,&insertPoint,insertLoc,1);
			insertLoc =  insertLoc2 = GetInsertLocFromPoint (hWnd,hFile,hFont,&insertPoint);
			SetCaretPos(insertPoint.x, insertPoint.y); 
			putInsertPointOnScreen (hWnd);
			//AutoInsert (hWnd,hFile,insertLoc,&hMenu);
			break;
		case VK_UP:
			currentLine = -1;
			insertPoint.x -= xChar / 2;
			insertPoint.y -= yChar;
			insertLoc =  insertLoc2 = GetInsertLocFromPoint (hWnd,hFile,hFont,&insertPoint);
			GetInsertPointFromLoc (hWnd,hFile,hFont,&insertPoint,insertLoc,0);
			SetCaretPos(insertPoint.x, insertPoint.y); 
			putInsertPointOnScreen (hWnd);
			//AutoInsert (hWnd,hFile,insertLoc,&hMenu);
			break;
		case VK_DOWN:
			currentLine = -1;
			insertPoint.x -= xChar / 2;
			insertPoint.y += yChar;
			insertLoc =  insertLoc2 = GetInsertLocFromPoint (hWnd,hFile,hFont,&insertPoint);
			GetInsertPointFromLoc (hWnd,hFile,hFont,&insertPoint,insertLoc,0);
			SetCaretPos(insertPoint.x, insertPoint.y); 
			putInsertPointOnScreen (hWnd);
			//AutoInsert (hWnd,hFile,insertLoc,&hMenu);
			break;
		//case 27: //ESC
	    //    PostMessage(hWnd, GF_CLOSE, 0,0L); 
	    //    return TRUE;
		}
		break;
	case WM_RBUTTONDOWN:
		currentLine = -1;
		insertPoint = POINTStoPOINT(MAKEPOINTS(lParam));
		insertLoc = insertLoc2 = GetInsertLocFromPoint(hWnd, hFile, hFont, &insertPoint);
		AddBreakpoint(fileToEdit,FileLocToMacroLoc(insertLoc));
		GetInsertPointFromLoc(hWnd, hFile, hFont, &insertPoint, insertLoc, 0);
		SetCaretPos(insertPoint.x, insertPoint.y);
		break;
	case WM_LBUTTONDOWN:
		currentLine = -1;
		insertPoint = POINTStoPOINT(MAKEPOINTS(lParam));
		GMEditDisplayTextBetweenLocs(hWnd, hFile, hFont, insertLoc, insertLoc2, FALSE);
		insertLoc = insertLoc2 = GetInsertLocFromPoint(hWnd, hFile, hFont, &insertPoint);
		GetInsertPointFromLoc(hWnd, hFile, hFont, &insertPoint, insertLoc, 0);
		SetCaretPos(insertPoint.x, insertPoint.y);
		break;
	case WM_MOUSEMOVE:
		if (wParam != MK_LBUTTON)
			break;
	case WM_LBUTTONUP:
		currentLine = -1;
		insertPoint2 = POINTStoPOINT(MAKEPOINTS(lParam));
		insertLoc3 =  GetInsertLocFromPoint (hWnd,hFile,hFont,&insertPoint2);
		if (insertLoc3 != insertLoc2)
		{
			GMEditDisplayTextBetweenLocs (hWnd,hFile,hFont,insertLoc,insertLoc2,FALSE);
			insertLoc2 = insertLoc3;
			GMEditDisplayTextBetweenLocs (hWnd,hFile,hFont,insertLoc,insertLoc2,TRUE);
		}
		break;
    case WM_SIZE:  
		currentLine = -1;
		GetWindowRect(hWnd, &rect);
		return DefWindowProc(hWnd, message, wParam, lParam);
case WM_SETFOCUS: 
 
    // Create a solid black caret. 
        CreateCaret(hWnd, (HBITMAP) NULL, cursorWidth, cursorHeight); 
 
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
        si.fMask  = SIF_ALL;

		currentLine = -1;
		// Save the position for comparison later on.
        GetScrollInfo (hWnd, SB_HORZ, &si);
        xPos = si.nPos;
        switch (LOWORD (wParam))
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
              
        default :
            break;
        }

        // Set the position and then retrieve it.  Due to adjustments
        // by Windows it may not be the same as the value set.
        si.fMask = SIF_POS;
        SetScrollInfo (hWnd, SB_HORZ, &si, TRUE);
        GetScrollInfo (hWnd, SB_HORZ, &si);
         
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
		short	zDelta = (short) HIWORD(wParam);    // wheel rotation
 
		currentLine = -1;
		si.cbSize = sizeof (si);
        si.fMask  = SIF_ALL;
        GetScrollInfo (hWnd, SB_VERT, &si);
		oldPos = si.nPos;
		if (zDelta < 0)
			si.nPos += 1;
		else
			si.nPos -= 1;
        si.fMask = SIF_POS;
        SetScrollInfo (hWnd, SB_VERT, &si, TRUE);
        GetScrollInfo (hWnd, SB_VERT, &si);
		if (si.nPos != oldPos)
			InvalidateRect (hWnd,0,TRUE);
		break;
	}       
    case WM_VSCROLL:
        // Get all the vertial scroll bar information.
		currentLine = -1;
		si.cbSize = sizeof (si);
        si.fMask  = SIF_ALL;
        GetScrollInfo (hWnd, SB_VERT, &si);

        // Save the position for comparison later on.
        yPos = si.nPos;
        switch (LOWORD (wParam))
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
        SetScrollInfo (hWnd, SB_VERT, &si, TRUE);
        GetScrollInfo (hWnd, SB_VERT, &si);

        // If the position has changed, scroll window and update it.
        if (si.nPos != yPos)
        {                    
            ScrollWindow(hWnd, 0, yChar * (yPos - si.nPos), NULL, NULL);
            UpdateWindow (hWnd);
        }

        return 0;
         
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		currentLine = -1;
		if (wmId >= 60000)
		{
			int insertOpt = wmId - 60000;

			if (hInsertOpts)
			{
				LPSTR pInsertOpts = GlobalLock (hInsertOpts);
				while (insertOpt--)
				{
					pInsertOpts = strchr (pInsertOpts,0);
					pInsertOpts++;
				}
				if (*pInsertOpts == '|')
				{
					if (!strncmp(pInsertOpts + 1, "$EXPAND(", 8))
					{
						LPSTR pTemp = malloc(USHRT_MAX);
						strcpy(pTemp, pInsertOpts + 9);
						ExpandText(pTemp);
						MessageBoxAtPosition(0, pTemp, "", MB_OK,"C");
						free(pTemp);
					}
					else
						ProcessText (pInsertOpts+1);
					GSSiGlobUlFree (&hInsertOpts);
				}
				else
				{
					while (*pInsertOpts)
						insertLoc = InsertCharAtLoc (hWnd,*pInsertOpts++,insertLoc,&insertLoc2,hFile);
					GSSiGlobUlFree (&hInsertOpts);
					GetInsertPointFromLoc (hWnd,hFile,hFont,&insertPoint,insertLoc,0);
					SetCaretPos(insertPoint.x, insertPoint.y); 
					InvalidateRect (hWnd,0,TRUE);
				}
			}
		}
		else switch (wmId)
		{
		case IDM_FILE_EXIT:
			PostMessage (hWnd,WM_CLOSE,0,0);
			break;
		case IDM_FILE_SAVE:
			GMEditSaveUpdates (hWnd);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX_GMEDIT), hWnd, AboutGMEdit);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_FIND:
		{
			int wSize = sizeof(FINDREPLACECHUNK);
         
			EnableMenuItem(GetMenu(ghWnd), IDM_FIND, MF_GRAYED);
			if (!hFChunk)
			{
				if (!(lpFChunk = (LPFINDREPLACECHUNK)AllocAndLockMem(&hFChunk, wSize)))
					break;
				InitializeStruct(IDC_FIND, (LPSTR)lpFChunk);
			}
			else
				lpFChunk = GlobalLock (hFChunk);
			lpFChunk->fr.hwndOwner = hWnd;
			lpFChunk->fr.Flags = FR_ENABLEHOOK;
			if (!(ghFindReplaceDlg = FindText(&(lpFChunk->fr))))
			{
				GSSiGlobUlFree (&hFChunk);
				ghFindReplaceDlg = NULL;
				EnableMenuItem(GetMenu(ghWnd), IDM_FIND, MF_ENABLED);
				ProcessCDError(CommDlgExtendedError());
			}
			else
				GlobalUnlock (hFChunk);

         //Note:  DO NOT call GlobalUnlock() and GlobalFree() here since this is
         //       a modeless dialog.  See default processing below.
		}
         break;

      case IDM_FINDANDREPLACE:
	  {
          int wSize = sizeof(FINDREPLACECHUNK);
		  
		  EnableMenuItem(GetMenu(ghWnd), IDM_FINDANDREPLACE, MF_GRAYED);
		  if (!hFRChunk)
		  {
			if (!(lpFRChunk = (LPFINDREPLACECHUNK)AllocAndLockMem(&hFRChunk, wSize)))
				break;
			InitializeStruct(IDC_FINDREPLACE, (LPSTR)lpFRChunk);
		  }
		  else
			  lpFRChunk = GlobalLock (hFRChunk);
		  lpFRChunk->fr.hwndOwner = hWnd;
		  lpFRChunk->fr.Flags = FR_ENABLEHOOK;
		  if (!(ghFindReplaceDlg = ReplaceText(&(lpFRChunk->fr))))
		  {
			  GSSiGlobUlFree (&hFRChunk);
			  ghFindReplaceDlg = NULL;
 			  ProcessCDError(CommDlgExtendedError());
		  }
			else
				GlobalUnlock (hFRChunk);

         //Note:  DO NOT call GlobalUnlock() and GlobalFree() here since this is
         //       a modeless dialog.  See default processing below.
        
	  }
	  break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;



}



// Message handler for about box.
INT_PTR CALLBACK AboutGMEdit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
