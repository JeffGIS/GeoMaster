#include "graphint.h"  
#include "gmextern.h"

#define MAX_MACRO_STACK 64
static int nFunLevs=0, ii;
static BOOL doDebug=FALSE;
static int	breakAt = BA_NEXTLINE;
static char BreakCondition[256]={0};
static char DisplayValue[256]={0};
static char macroStack[MAX_MACRO_STACK][MAX_PATH];
static int macroUse[MAX_MACRO_STACK];
static int lnMacroStack=0;
static int nextMacroUse = 1;
static char currentMacro[MAX_PATH];
static RECT currentRect = { 0 };

#define NUM_DB_CHILDWND	12
#define SNAP_LEFT	-1
#define SNAP_TOP	-1
#define SNAP_BOTTOM	-2
#define SNAP_RIGHT	-2
static UINT childWndID[NUM_DB_CHILDWND] = {IDB_MACROSTACK,IDC_FILEVIEW,IDB_MOVETOMON2,IDCANCEL,IDOK,IDC_STATIC_BP,IDC_STATIC_BC,IDC_STATIC_DV,IDB_BREAKPOINTS,IDB_BREAKCONDITION,IDB_VALUETODISPLAY1,IDB_DISPLAYVALUE1};
static RECT childWndPCT[NUM_DB_CHILDWND] = {SNAP_LEFT,SNAP_TOP,0,0,
											SNAP_LEFT,0,SNAP_RIGHT,0,
											0,SNAP_TOP,0,0,
											0,0,SNAP_RIGHT,0,
											0, SNAP_TOP, SNAP_RIGHT, 0,
											SNAP_LEFT, 0, 0, 0,
											0,0,0,0,
											0,0,0,0,
											SNAP_LEFT,0,0,SNAP_BOTTOM,
											0,0,0,0,
											0,0,0,0,
											0,0,0,0
											};
static int childWndBorder;
static RECT crectDBOrig;

static TCHAR szTitle[]="GMEdit";					// The title bar text
static TCHAR szWindowClass[]="GMEditor";		// the main window class name
LRESULT CALLBACK	WndProcGMEdit(HWND, UINT, WPARAM, LPARAM);

ATOM DBRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	static ATOM c;
	static BOOL first=TRUE;

	if (!first)
		return c;
	first = FALSE;
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

	c=RegisterClassEx(&wcex);
	return c;
}

POINT WindowMidPoint(HWND hWnd)
{
	POINT pt;
	RECT  rect;

	GetWindowRect(hWnd, &rect);
	pt = RectMid(&rect);
	return pt;
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
HWND InitDBInstance(HWND hWndPar,LPRECT pRect)
{
   HWND hWnd;

	//hWndPar=0;
   hWnd = CreateWindow(szWindowClass, szTitle,WS_POPUPWINDOW|WS_VSCROLL|WS_HSCROLL,
      pRect->left,pRect->right,RECTWIDTH(pRect),RECTHEIGHT(pRect),hWndPar, NULL, hInst, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   //MoveWindow (hWnd,0,0,500,500,TRUE);
   return hWnd;
}
HWND CreateDebugFileDisplayWindow (HWND hWnd,LPRECT pRect)
{
	DBRegisterClass(hInst);

	// Perform application initialization:
	return InitDBInstance (hWnd,pRect);
}

LONG FAR PASCAL DebugSubclassProc(HWND hWnd, UINT uiMsg,WORD wParam, LONG lParam)
{
    WNDPROC lpOrgProc;
    LONG    lRtn = 0;
    int		i; 

    lpOrgProc   = ORIGINALPROC(hWnd);
    switch (uiMsg)
    {   
		//case WM_WINDOWPOSCHANGING:
		//case WM_WINDOWPOSCHANGED:
		case WM_MOVE:
            	lRtn = CallWindowProc ((FARPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	    	break;
    	
		case WM_TIMER:
            	lRtn = CallWindowProc ((FARPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
			break;
    	case WM_NCDESTROY:
        {
	        lRtn = CallWindowProc ((FARPROC)lpOrgProc, hWnd, uiMsg,(WPARAM) wParam, (LPARAM)lParam);
	        SetWindowLong (hWnd, GWL_WNDPROC, (LONG) lpOrgProc);
	        RemoveProp (hWnd, "PrHI");
	        RemoveProp (hWnd, "PrLO");
        }
        break;
    
    	case WM_NCHITTEST:
	    {   
	        lRtn = CallWindowProc ((FARPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	    	/*if (hWnd != LastWnd)
	    	{
				Prompt = (UINT)GetProp (hWnd, "GMPr");
				SetPromptDlg (Prompt);
				LastWnd = hWnd; 
			}*/
	    } 
	    break;
	    
   	
    	default:
            lRtn = CallWindowProc ((FARPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
        break;
    }

    return (lRtn);
}

void FAR PASCAL DBSubclassControl(HWND hCtrl, void FAR *Callback)
{
    FARPROC     lpOrgProc; 
    DWORD		PrDat;
    HWND		hWnd;

    lpOrgProc = (FARPROC) SetWindowLong (hCtrl, GWL_WNDPROC,
                       (LONG) (FARPROC) Callback);
    SetProp (hCtrl, "PrHI", (HANDLE) HIWORD (lpOrgProc));
    SetProp (hCtrl, "PrLO", (HANDLE) LOWORD (lpOrgProc));
    }

void SetDBChildWindowParms(HWND hWndDlg)
{
	int i;
	RECT rect, crect;
	float	 w, h;

	GetClientRect(hWndDlg, &crect);
	crectDBOrig = crect;
	w = RECTWIDTH(&crect);
	h = RECTHEIGHT(&crect);

	for (i = 0; i < NUM_DB_CHILDWND; i++)
	{
		GetWindowRect (GetDlgItem (hWndDlg,childWndID[i]), &rect);
		ScreenRectToClientRect(hWndDlg, &rect);
		if (!i)
			childWndBorder = rect.left;
		if (!childWndPCT[i].left)
			childWndPCT[i].left = 1000 * (rect.left / w);
		if (!childWndPCT[i].right)
			childWndPCT[i].right = 1000 * (rect.right / w);
		if (!childWndPCT[i].top)
			childWndPCT[i].top = 1000 * (rect.top / h);
		if (!childWndPCT[i].bottom)
			childWndPCT[i].bottom = 1000 * (rect.bottom / h);
	}
	return;
}

void AdjustDBChildWindows(HWND hWndDlg)
{
	int i;
	RECT crect, rect;
	float	 w, h;
	int border;

	GetClientRect(hWndDlg, &crect);
	w = RECTWIDTH(&crect);
	h = RECTHEIGHT(&crect);
	border = childWndBorder * w / RECTWIDTH(&crectDBOrig);
	for (i = 0; i < NUM_DB_CHILDWND; i++)
	{
		if (childWndPCT[i].left < 0)
			rect.left = crect.left + border;
		else
			rect.left = (w * childWndPCT[i].left) / 1000;
		if (childWndPCT[i].right < 0)
			rect.right = crect.right - border;
		else
			rect.right = (w * childWndPCT[i].right) / 1000;
		if (childWndPCT[i].top < 0)
			rect.top = crect.top + border;
		else
			rect.top = (h * childWndPCT[i].top) / 1000;
		if (childWndPCT[i].bottom < 0)
			rect.bottom = crect.bottom - border;
		else
			rect.bottom = (h * childWndPCT[i].bottom) / 1000;

		MoveWindow(GetDlgItem(hWndDlg, childWndID[i]), rect.left, rect.top, RECTWIDTH(&rect), RECTHEIGHT(&rect), TRUE);
	}
	return;
}

BOOL FAR PASCAL DEBUGGERMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
	static	HANDLE	hSaveBM=0;	
	static	HWND	hDBWnd=0;
	HANDLE	hDisplay;
	LPSTR	pDisplay;
	RECT	rect;
	int		i;
	static	BOOL	firstPaint=TRUE;

 switch(Message)
   {
		 case WM_INITDIALOG:
		 {
			char curFile[MAX_PATH];

			hWndAddEdit = hWndDlg; //does IsDialog processing
			firstPaint = TRUE;
			SetGlobalValueBool("%IGNORESYSMSG", TRUE);
			numMonitors = GetNumMonitors();
			EnableWindow(GetDlgItem(hWndDlg, IDB_MOVETOMON2), numMonitors > 1);
			//DBSubclassControl(hWndDlg, DebugSubclassProc);
			//    	 hSaveBM = EnterBlockingWindow (hWndDlg);
			//		 SetWindowText (hWndDlg,MBHTitle);
			SetDlgItemText(hWndDlg, IDB_BREAKCONDITION, BreakCondition);
			SetDlgItemText(hWndDlg, IDB_VALUETODISPLAY1, DisplayValue);
			hDisplay = GSSiGlobAlloc(0, GMEM_MOVEABLE, 4096);
			pDisplay = GlobalLock(hDisplay);
			strcpy(pDisplay, DisplayValue);
			doDebug = FALSE;
			ExpandText(pDisplay);
			doDebug = TRUE;
			SetDlgItemText(hWndDlg, IDB_DISPLAYVALUE1, pDisplay);
			GSSiGlobUlFree(&hDisplay);
			for (i = 0; i < lnMacroStack; i++)
				SendDlgItemMessage(hWndDlg, IDB_MACROSTACK, LB_ADDSTRING, 0, (LPARAM)macroStack[i]);

			//GMEditSetFile ("[%DL]fundir\\appl1.txt");
			GMEditGetFile(curFile);
			i = SendDlgItemMessage(hWndDlg, IDB_MACROSTACK, LB_FINDSTRING, (WPARAM)-1, (LPARAM)curFile);
			SendDlgItemMessage(hWndDlg, IDB_MACROSTACK, LB_SETCURSEL, i,0);

			if (IsRectEmpty(&currentRect))
			{
				RECT mainrect;

				GetWindowRect(hWndMain, &mainrect);
				SetDBChildWindowParms(hWndDlg);
				MoveWindow(hWndDlg, mainrect.left, mainrect.top, RECTWIDTH(&mainrect), RECTHEIGHT(&mainrect), TRUE);
				GetWindowRect(hWndDlg, &currentRect);
			}
			else
			{
				MoveWindow(hWndDlg, currentRect.left, currentRect.top, RECTWIDTH(&currentRect), RECTHEIGHT(&currentRect), TRUE);
			}
		 }
         break; /* End of WM_INITDIALOG                                 */

	case WM_ACTIVATE:
		 ii=1;
		 break;
	case WM_PAINT:
		 if (!firstPaint)
			 return FALSE;
		 firstPaint = FALSE;
		 GetWindowRect (GetDlgItem (hWndDlg,IDC_FILEVIEW),&rect);
		 //ClientRectToScreenRect (hWndDlg,&rect);
		 hDBWnd = CreateDebugFileDisplayWindow (hWndDlg,&rect);
 		 ii=SetTimer (hDBWnd,1,300,0);
		 PostMessage (hWndDlg,WM_MOVE,0,0);
		 return FALSE;
		 break;

	case WM_ENTERSIZEMOVE:
		if (hDBWnd)
		{
			ShowWindow(hDBWnd, SW_HIDE);
			ShowWindow(GetDlgItem(hWndDlg, IDC_FILEVIEW), SW_SHOW);
		}
		break;

	case WM_MOVE:
	case WM_EXITSIZEMOVE:
		 if (!firstPaint)
		 {
			 RECT wRect;
			 AdjustDBChildWindows(hWndDlg);
			 GetWindowRect(hWndDlg, &wRect);
			 GetWindowRect (GetDlgItem (hWndDlg,IDC_FILEVIEW),&rect);
			 //ScreenRectToClientRect (hWndDlg,&rect);

			 //ShowWindow(hDBWnd,SW_SHOW);
			 if (hDBWnd)
			 {
				 MoveWindow(hDBWnd, rect.left, rect.top, RECTWIDTH(&rect), RECTHEIGHT(&rect), FALSE);
				 ShowWindow(GetDlgItem(hWndDlg, IDC_FILEVIEW), SW_HIDE);
				 ShowWindow(hDBWnd, SW_SHOW);
				 InvalidateRect(hDBWnd, 0, TRUE);
				 UpdateWindow(hDBWnd);
			 }
			 UpdateWindow(hWndDlg);

		 }
		break;
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

	case WM_DESTROY:
		 GetWindowRect(hWndDlg, &currentRect);
		 hWndAddEdit = 0;
		 SendMessage(hDBWnd, WM_CLOSE, 0, 0L);
		 break;

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
				doDebug = FALSE;
				EndDialog(hWndDlg, IDCANCEL);
            break; 
            
            case IDOK:
				GetDlgItemText (hWndDlg,IDB_BREAKCONDITION,BreakCondition,sizeof(BreakCondition)-1);
				GetDlgItemText (hWndDlg,IDB_VALUETODISPLAY1,DisplayValue,sizeof(DisplayValue)-1);
                EndDialog(hWndDlg, TRUE);
            break;

			case IDB_MOVETOMON2:
			{
			   POINT pt = WindowMidPoint(GetDlgItem(hWndDlg, IDB_MOVETOMON2));
			   HMONITOR hMonitor;
			   MONITORINFOEX mi;

			   hMonitor = GetOtherMonitor(pt);

			   if (hMonitor)
			   {
				   mi.cbSize = sizeof(mi);
				   GetMonitorInfo(hMonitor,(LPMONITORINFO) &mi);
				   MoveWindow(hWndDlg, mi.rcWork.left, mi.rcWork.top, RECTWIDTH(&mi.rcWork), RECTHEIGHT(&mi.rcWork), TRUE);
			   }

			}
				break;
			case IDB_VALUETODISPLAY1:
                 switch (HIWORD(wParam))
                 {  case EN_CHANGE:
                        i = GetDlgItemText (hWndDlg,IDB_VALUETODISPLAY1,DisplayValue,sizeof(DisplayValue)-1);
						if (i && DisplayValue[i-1] == '\n')
						{
							 hDisplay = GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
							 pDisplay = GlobalLock (hDisplay);
							 strcpy (pDisplay,DisplayValue);
							 doDebug=FALSE;
							 ExpandText (pDisplay);
							 doDebug=TRUE;
							 SetDlgItemText (hWndDlg,IDB_DISPLAYVALUE1,pDisplay);
							 GSSiGlobUlFree (&hDisplay);
						}
						break;
				 }
			break;
			case IDB_MACROSTACK:
			{
                 switch(HIWORD(wParam))
                 {   
                     case LBN_SELCHANGE: 
					 {
						int item=SendDlgItemMessage(hWndDlg,IDB_MACROSTACK,LB_GETCURSEL,0,0);
						char str[MAX_PATH];

						if (item < 0)
							break;
						SendDlgItemMessage(hWndDlg,IDB_MACROSTACK,LB_GETTEXT,item,(DWORD)str);
	    				if (hDBWnd)
						{
							SendMessage(hDBWnd , WM_CLOSE, 0, 0L);
							hDBWnd = 0;
							//break;
						}
						GMEditSetFile (str,0,0);
						GetWindowRect (GetDlgItem (hWndDlg,IDC_FILEVIEW),&rect);
						hDBWnd = CreateDebugFileDisplayWindow (hWndDlg,&rect);
						PostMessage (hWndDlg,WM_MOVE,0,0);
						ShowWindow (hDBWnd,SW_SHOW);
						InvalidateRect (hDBWnd,0,TRUE);
					 }
				 }
			}
			break;

         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 

void SetDebug (BOOL state)
{
	doDebug = state;
	if (doDebug)
	{
		CloseBufferedMacros();
		DialogBox(hInst, (LPSTR)"DEBUGGER", hWndMain, DEBUGGERMsgProc);
	}
	//CreateDialog (hInst, (LPSTR)"DEBUGGER", hWndMain, DEBUGGERMsgProc);
	return;
}
BOOL GetDebug (void)
{
	return doDebug;
}
void breakAtPos(int pos, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen, int from)
{
	if (pos + bpOffset < bpLen)
	{
		if ((pBrkPt[pos + bpOffset].beginLine &&  from == BA_NEXTPOS && breakAt == BA_NEXTLINE) ||
			(from == BA_FUNCTION && breakAt == BA_FUNCTION) ||
			from == BA_BEGINBLOCK)
		{
			AtBreakPoint("",1+pos + bpOffset);
		}
	}

}

void AtBreakPoint (LPSTR Args,int bploc)
{
	int rtn, maxmacro, maxi, i;
	BOOL err, rc;

	if (!doDebug)
		return;

	if (*BreakCondition)
	{
		HANDLE hCond = GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
		LPSTR pCond = GlobalLock (hCond);

		strcpy (pCond,BreakCondition);
		doDebug = FALSE;
		rc = LogicP (pCond,&err);
		doDebug = TRUE;
		GSSiGlobUlFree (&hCond);
		if (!rc)
			return;
	}
	maxmacro = 0;
	maxi = 0;
	for (i = 0; i<lnMacroStack; i++)
	{
		if (macroUse[i] > maxmacro)
		{
			maxmacro = macroUse[i];
			maxi = i;
		}
	}

	GMEditSetFile (macroStack[maxi],Args,bploc);
	rtn = DialogBox(hInst, (LPSTR)"DEBUGGER", hWndMain, DEBUGGERMsgProc);
	return;
}
void InFunction (int funid,LPSTR inString)
{
	nFunLevs++;
	if (funid == 845)
		ii=1;
	return;
}

void OutFunction (int funid,LPSTR outString)
{
	nFunLevs--;
	if (funid == 845)
		ii=1;
	return;
}

int AddToMacroStack (int from,int iCurrentMacro,LPSTR File,LPHANDLE phArgs,int NumArgs)
{
	int i, mini;
	UINT minUse = UINT_MAX;

	//from: 1=RunMacro, 2=RunGFCommandFromFileAtLoc
	strcpy (currentMacro,File);
	SubstituteDL (currentMacro,FALSE);
	for (i=0;i<lnMacroStack;i++)
	{
		if (!stricmp(currentMacro, macroStack[i]))
		{
			macroUse[i] = nextMacroUse++;
			return i;
		}
		if (macroUse[i] < minUse)
		{
			minUse = macroUse[i];
			mini = i;
		}
	}
	if (lnMacroStack >= MAX_MACRO_STACK - 1)
	{
		strcpy(macroStack[mini], currentMacro);
		macroUse[mini] = nextMacroUse++;
		return mini;
	}
	else
	{	
		strcpy(macroStack[lnMacroStack], currentMacro);
		macroUse[lnMacroStack] = nextMacroUse++;
		return lnMacroStack++;
	}
}

void RemoveFromMacroStack (int macroID)
{
	if (macroID < 0)
		lnMacroStack = 0;
	else
		macroUse[macroID] = -abs(macroUse[macroID]);
	return;
}


