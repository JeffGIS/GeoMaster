#include "graphint.h"  
#include "gmextern.h"

#define MAX_MACRO_STACK 64
static int nFunLevs=0, ii;
static BOOL doDebug=FALSE;
static char BreakCondition[256]={0};
static char DisplayValue[256]={0};
static char macroStack[MAX_MACRO_STACK][MAX_PATH];
static int lnMacroStack=0;
static char currentMacro[MAX_PATH];



static TCHAR szTitle[]="GMEdit";					// The title bar text
static TCHAR szWindowClass[]="GMEditor";		// the main window class name
LRESULT CALLBACK	WndProcGMEdit(HWND, UINT, WPARAM, LPARAM);
void GMEditSetFile (LPSTR file,LPSTR bpid);

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
		 hWndAddEdit = hWndDlg; //does IsDialog processing
		 firstPaint = TRUE;
		 //DBSubclassControl(hWndDlg, DebugSubclassProc);
//    	 hSaveBM = EnterBlockingWindow (hWndDlg);
//		 SetWindowText (hWndDlg,MBHTitle);
		 SetDlgItemText (hWndDlg,IDB_BREAKCONDITION,BreakCondition);
		 SetDlgItemText (hWndDlg,IDB_VALUETODISPLAY1,DisplayValue);
		 hDisplay = GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
		 pDisplay = GlobalLock (hDisplay);
		 strcpy (pDisplay,DisplayValue);
		 doDebug=FALSE;
		 ExpandText (pDisplay);
		 doDebug=TRUE;
		 SetDlgItemText (hWndDlg,IDB_DISPLAYVALUE1,pDisplay);
		 GSSiGlobUlFree (&hDisplay);
		 for (i=0;i<lnMacroStack;i++)
			 SendDlgItemMessage (hWndDlg,IDB_MACROSTACK,LB_ADDSTRING,0,(LPARAM)macroStack[i]); 

		 //GMEditSetFile ("[%DL]fundir\\appl1.txt");
         cwCenter(hWndDlg, 0);
			
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

	case WM_MOVE:
		 if (!firstPaint)
		 {
			 GetWindowRect (GetDlgItem (hWndDlg,IDC_FILEVIEW),&rect);
			 //ScreenRectToClientRect (hWndDlg,&rect);
			 MoveWindow (hDBWnd,rect.left,rect.top,RECTWIDTH(&rect),RECTHEIGHT(&rect),FALSE);

			 //ShowWindow(hDBWnd,SW_SHOW);
			 UpdateWindow(hDBWnd);
		 }
		break;
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 SendMessage(hDBWnd , WM_CLOSE, 0, 0L);
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

	case WM_DESTROY:
		 hWndAddEdit = 0;
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
						GMEditSetFile (str,0);
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
		DialogBox(hInst, (LPSTR)"DEBUGGER", hWndMain, DEBUGGERMsgProc);
	//CreateDialog (hInst, (LPSTR)"DEBUGGER", hWndMain, DEBUGGERMsgProc);
	return;
}
BOOL GetDebug (void)
{
	return doDebug;
}
void AtBreakPoint (LPSTR Args)
{
	int rtn;
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
	GMEditSetFile (macroStack[lnMacroStack-1],Args);
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
	int i;

	//from: 1=RunMacro, 2=RunGFCommandFromFileAtLoc
	strcpy (currentMacro,File);
	SubstituteDL (currentMacro,FALSE);
	for (i=0;i<lnMacroStack;i++)
	{
		if (!stricmp (currentMacro,macroStack[i]))
			return i;
	}
	if (lnMacroStack >= MAX_MACRO_STACK-1)
		return -1;
	strcpy (macroStack[lnMacroStack],currentMacro);
	
	return lnMacroStack++;
}

void RemoveFromMacroStack (int macroID)
{
	if (macroID < 0)
		lnMacroStack = 0;
	else
		lnMacroStack--;
	return;
}


