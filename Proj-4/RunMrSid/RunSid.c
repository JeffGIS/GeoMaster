#include <windows.h>
#include <stdio.h>

HWND	MrSidWnd=0;
extern	HWND	hWndMain;

BOOL CALLBACK EnumChildProc(HWND hCtrl,LONG lParam)
{
    char        str[256];
//    long	lUserData;
//    POINT	Loc; 
    RECT	Rect;
    
    GetWindowText (hCtrl,str,200);  
    GetWindowRect (hCtrl,&Rect);
    return (TRUE);
}

BOOL CALLBACK EnumWndProc(HWND hCtrl,LONG lParam)
{
	char        str[130];
//	long	lUserData; 
	HWND	hPar;
	    
	GetWindowText (hCtrl,str,128); 
	hPar = GetParent (hCtrl); 
	if (!strnicmp (str,"MrSid Viewer",12))
		MrSidWnd = hCtrl; 
	return TRUE;
}
void Wait (long MicroSeconds)
{   
	MSG	msg; 
	UINT	MS=(UINT)MicroSeconds;
	
	SetTimer (hWndMain,9999,MS,0);
	msg.wParam = 0; 
	while (msg.wParam != 9999)
		GetMessage (&msg,hWndMain,WM_TIMER,WM_TIMER); 
	KillTimer (hWndMain,9999);
	return;
}


BOOL SplitMrSidFile (LPSTR File,LPSTR MrSidEx)
{   
	char	str[256], mess[128];
	DWORD	WVer;
	UINT	ierr;
	BOOL	rtn = TRUE;
    FARPROC lpfnEnumWndProc;
    
    {							      	 
		sprintf (str,"%s %s",MrSidEx,File); 
		if ((ierr = WinExec (str,SW_SHOW)) < 32)
		{  
			sprintf (mess,"Error loading MrSid: %i",(int) ierr);
			MessageBox (GetFocus(),mess,NULL,0);  
			return FALSE;
		}  
		MrSidWnd = 0;
		Wait (2000);
		lpfnEnumWndProc = MakeProcInstance((FARPROC)EnumWndProc, hInst);
		EnumWindows (lpfnEnumWndProc,0);   
		FreeProcInstance(lpfnEnumWndProc); 
		SetWindowText (MrSidWnd,"MrSid 1");
//		PostMessage (MrSidWnd, WM_CLOSE, 0, 0L);
    }
	return TRUE;
}
