#include "graphint.h"   
#include "dibapi.h"
#include <commctrl.h>
//#include <windowsx.h>

#include "gmextern.h"

// Global variables 
 
static TCHAR g_achTemp[256];  // Temporary buffer for strings. 
static HWND  g_hwndTab=0;       // Tab control.
HWND  g_hwndDisplay=0;   // Static control in tab control's display area.
static WNDPROC g_OldProc;
WNDPROC	g_OrigTabProc;

void DisplayTabbedMenu (HWND hWnd);
LRESULT CALLBACK TabSubclassProc (HWND hwnd, UINT message, 
                             WPARAM wParam, LPARAM lParam);

int SwitchTabbedMenu (HWND hWnd,int ipage);

// The following function creates the static control that occupies the tab control's display area.
// The application's initialization function calls this function after creating the main window and the tab control.

// DoCreateDisplayWindow - creates a child window (a static 
//     control) to occupy the tab control's display area. 
// Returns the handle to the static control. 
// hwndParent - parent window (the application's main window, or a dialog box). 
 
HWND WINAPI DoCreateDisplayWindow(HWND hwndParent) 
{ 
	RECT	parClientRect;
	HWND	hwndStatic;

	GetClientRect (hwndParent,&parClientRect);
    hwndStatic = CreateWindow(TEXT("STATIC"), TEXT(""), 
        WS_CHILD | WS_VISIBLE | WS_BORDER, 
		100,100,100,100,
        //parClientRect.left, parClientRect.top, parClientRect.right,parClientRect.bottom,      // Position and dimensions; example only.
        hwndParent, NULL, hInst, NULL); 
	g_hwndDisplay = hwndStatic;
    return hwndStatic; 
}
// Following are the relevant portions of the application's window procedure.
// The application processes the WM_SIZE message to position and size the tab control and the static control.
// To determine the appropriate position and size for the static control, this example sends the tab control a TCM_ADJUSTRECT message (by using the TabCtrl_AdjustRect macro).

// When a tab is selected, the tab control sends a WM_NOTIFY message, specifying the TCN_SELCHANGE notification code.
// The application processes this notification code by setting the text of the static control.


// MainWindowProc - processes the message for the main window class. 
// The return value depends on the message. 
// hwnd - handle to the window. 
// uMsg - identifier for the message. 
// wParam - message-specific parameter. 
// lParam - message-specific parameter. 
 
BOOL CALLBACK TabbedWindowProc( 
        HWND hwnd, 
        UINT uMsg, 
        WPARAM wParam, 
        LPARAM lParam 
        ) 
{
	int	ii;
	BOOL st;
	LPWINDOWPOS	lpwp;

	ii=uMsg;
    switch (uMsg) { 
/*        case WM_SIZE: { 
                HDWP hdwp; 
                RECT rc; 
 
                // Calculate the display rectangle, assuming the 
                // tab control is the size of the client area. 
                SetRect(&rc, 0, 0, 
                        GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)); 
                TabCtrl_AdjustRect(g_hwndTab, FALSE, &rc); 
 
                // Size the tab control to fit the client area. 
                //hdwp = BeginDeferWindowPos(2); 
               // hdwp = DeferWindowPos(hdwp,
					SetWindowPos (g_hwndTab, NULL, 0, 0, 
                    GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 
                    SWP_NOMOVE | SWP_NOZORDER 
                    ); 
 
                // Position and size the static control to fit the 
                // tab control's display area, and make sure the 
                // static control is in front of the tab control. 
               // hdwp = DeferWindowPos(hdwp, 
                    SetWindowPos(g_hwndDisplay, HWND_TOP, rc.left, rc.top, 
                    rc.right - rc.left, rc.bottom - rc.top, 0 
                    ); 
                //st = EndDeferWindowPos(hdwp); 
            } 
            break; 
 
		case WM_COMMAND:
			ii=1;
			break;*/
        case WM_NOTIFY:
			ii=1;
            switch (((LPNMHDR)lParam)->code)
            {
            case TCN_SELCHANGE:
                { 
                    int iPage = TabCtrl_GetCurSel(((LPNMHDR)lParam)->hwndFrom); 
/*					LRESULT result;

			  		 sprintf (g_achTemp,"Tab %i",iPage);
                     result = SendMessage(g_hwndDisplay, WM_SETTEXT, 0,
                        (LPARAM) g_achTemp); 
			 		InvalidateRect (g_hwndDisplay,0,TRUE);*/
					SwitchTabbedMenu (hwnd,iPage);
		            return TRUE;
                } 
            }
			return FALSE;
/*		case WM_WINDOWPOSCHANGED:
			lpwp = (LPWINDOWPOS) lParam;
			return DefWindowProc(hwnd,uMsg, wParam, lParam);
		case WM_WINDOWPOSCHANGING:
			lpwp = (LPWINDOWPOS) lParam;
			return DefWindowProc(hwnd,uMsg, wParam, lParam);
		case WM_PAINT:
			break;*/
        default: 
			return FALSE;
			CallWindowProc (g_OldProc,hwnd,uMsg, wParam, lParam );

    } 
    return TRUE; 
} 
 
// The following function creates the tab control and adds a tab for each day of the week.
// The names of the days are defined as string resources, consecutively numbered starting with IDS_FIRSTDAY (defined in the application's resource header file). 
// Both the parent window and the tab control must have the WS_CLIPSIBLINGS window style. The application's initialization function calls this function after creating the main window.

// DoCreateTabControl - creates a tab control, sized to fit the 
//     specified parent window's client area, and adds some tabs. 
// Returns the handle to the tab control. 
// hwndParent - parent window (the application's main window). 
 
HWND WINAPI DoCreateTabControl(HWND hwndParent,int nTabs,LPSTR TabItems,int TabItemLen,LPRECT pRect,BOOL Verticle,BOOL Flat,int xPad,int yPad) 
{ 
    RECT rcClient; 
    HWND hwndTab; 
    TCITEM tie; 
    int i; 
	DWORD	Flags = WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE|TCS_MULTILINE|TCS_HOTTRACK;
 
 
    // Get the dimensions of the parent window's client area, and 
    // create a tab control child window of that size. 
    //GetClientRect(hwndParent, &rcClient);
	
	if (Verticle)
		Flags = Flags | TCS_VERTICAL| TCS_TABS;
	else
	{
		if (Flat)
			Flags = Flags | TCS_FLATBUTTONS;
		else
			Flags = Flags | TCS_BUTTONS;
	}

    hwndTab = CreateWindow(WC_TABCONTROL, "", Flags,
        pRect->left,pRect->top,pRect->right,pRect->bottom,
        hwndParent, NULL, hInst, NULL); 
    if (hwndTab == NULL)
    { 
        return NULL; 
    }
 	g_OrigTabProc = (WNDPROC)SetWindowLong(hwndTab, GWL_WNDPROC, (LONG)TabSubclassProc);

	g_hwndTab = hwndTab;
    tie.mask = TCIF_TEXT | TCIF_IMAGE; 
    tie.iImage = -1; 
    tie.pszText = g_achTemp; 

	TabCtrl_SetPadding (hwndTab,xPad,yPad);
//	TabCtrl_SetToolTips(hwndTab,hWndTT);

    for (i = 0; i < nTabs; i++) 
    { 
		strcpy (g_achTemp,(TabItems+(i*TabItemLen)));
        if (TabCtrl_InsertItem(hwndTab, i, &tie) == -1) 
        { 
            DestroyWindow(hwndTab); 
            return NULL; 
        } 
    } 
	g_OldProc = (WNDPROC)GetWindowLong (hwndTab,GWL_WNDPROC);
//	SetWindowLong (hwndTab,GWL_WNDPROC,TabbedWindowProc);
    return hwndTab; 
} 
 