#include "BASIC.h"    
#include <stdio.h>
#include <math.h>
BOOL SplitMrSidFile (LPSTR File,LPSTR MrSidEx);
int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
 /***********************************************************************/
 /* HANDLE hInstance;       handle for this instance                    */
 /* HANDLE hPrevInstance;   handle for possible previous instances      */
 /* LPSTR  lpszCmdLine;     long pointer to exec command line           */
 /* int    nCmdShow;        Show code for main window display           */
 /***********************************************************************/

 MSG        msg;           /* MSG structure to store your messages        */
 int        nRc;           /* return value from Register Classes          */

 strcpy(szAppName, "BASIC");
 hInst = hInstance;
 if(!hPrevInstance)
   {
    /* register window classes if first instance of application         */
    if ((nRc = nCwRegisterClasses()) == -1)
      {
       /* registering one of the windows failed                         */       
       LoadString(hInst, IDS_ERR_REGISTER_CLASS, szString, sizeof(szString));
       MessageBox(NULL, szString, NULL, MB_ICONEXCLAMATION);
       return nRc;    
      }
   }

 /* create application's Main window                                    */
 hWndMain = CreateWindow(
                szAppName,               /* Window class name           */
                "Basic Windows Application", /* Window's title          */
                WS_CAPTION      |        /* Title and Min/Max           */
                WS_SYSMENU      |        /* Add system menu box         */
                WS_MINIMIZEBOX  |        /* Add minimize box            */
                WS_MAXIMIZEBOX  |        /* Add maximize box            */
                WS_THICKFRAME   |        /* thick sizeable frame        */
                WS_CLIPCHILDREN |         /* don't draw in child windows areas */
                WS_OVERLAPPED,
                CW_USEDEFAULT, 0,        /* Use default X, Y            */
                CW_USEDEFAULT, 0,        /* Use default X, Y            */
                NULL,                    /* Parent window's handle      */
                NULL,                    /* Default to Class Menu       */
                hInst,                   /* Instance of window          */
                NULL);                   /* Create struct for WM_CREATE */


 if(hWndMain == NULL)
   {
    LoadString(hInst, IDS_ERR_CREATE_WINDOW, szString, sizeof(szString));
    MessageBox(NULL, szString, NULL, MB_ICONEXCLAMATION);
    return IDS_ERR_CREATE_WINDOW;
   }

// ShowWindow(hWndMain, nCmdShow);            /* display main window      */

 while(GetMessage(&msg, NULL, 0, 0))        /* Until WM_QUIT message    */
   {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
   }

 /* Do clean up before exiting from the application                     */
 CwUnRegisterClasses();
 return msg.wParam;
} /*  End of WinMain                                                    */
/************************************************************************/
/*                                                                      */   
/* Main Window Procedure                                                */
/*                                                                      */
/* This procedure provides service routines for the Windows events      */
/* (messages) that Windows sends to the window, as well as the user     */
/* initiated events (messages) that are generated when the user selects */
/* the action bar and pulldown menu controls or the corresponding       */
/* keyboard accelerators.                                               */
/*                                                                      */
/************************************************************************/

LONG FAR PASCAL WndProc(HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{
 HMENU      hMenu=0;            /* handle for the menu                 */
 HBITMAP    hBitmap=0;          /* handle for bitmaps                  */                              
 HDC        hDC;                /* handle for the display device       */
 PAINTSTRUCT ps;                /* holds PAINT information             */
 int        nRc=0;              /* return code                         */

 switch (Message)
   {
    case WM_CREATE:
{ 
char	str[128];
char	File[256]={"E:\\naip\\naip03_dakota\\ortho_e1-1_mn037.sid"};
		LPSTR pstr=str;
		hWndMain = hWnd;
		SplitMrSidFile (File,"C:\\MrSID Viewer\\MrSIDViewer.exe");
		return FALSE;
}

         break;       /*  End of WM_CREATE                              */

    case WM_MOVE:     /*  code for moving the window                    */
         break;
    
    case WM_SIZE:     /*  code for sizing client area                   */
         break;       /* End of WM_SIZE                                 */

    case WM_PAINT:    /* code for the window's client area              */
         /* Obtain a handle to the device context                       */
         /* BeginPaint will sends WM_ERASEBKGND if appropriate          */
         memset(&ps, 0x00, sizeof(PAINTSTRUCT));
         hDC = BeginPaint(hWnd, &ps);

         /* Included in case the background is not a pure color         */
         SetBkMode(hDC, TRANSPARENT);

         /* Inform Windows painting is complete                         */
         EndPaint(hWnd, &ps);
         break;       /*  End of WM_PAINT                               */

    case WM_CLOSE:  /* close the window                                 */
         /* Destroy child windows, modeless dialogs, then, this window  */
         DestroyWindow(hWnd);
         if (hWnd == hWndMain)
           PostQuitMessage(0);  /* Quit the application                 */
        break;
    
    case WM_COMMAND:
    {         
    	switch (wParam)
    	{
        }
        break;   	 
    			
    }
    	break;
    default:
         /* For any message for which you don't specifically provide a  */
         /* service routine, you should return the message to Windows   */
         /* for default message processing.                             */
         return DefWindowProc(hWnd, Message, wParam, lParam);
   } 
 return 0L;
}     /* End of WndProc                                         */

/************************************************************************/
/*                                                                      */
/* nCwRegisterClasses Function                                          */
/*                                                                      */
/* The following function registers all the classes of all the windows  */
/* associated with this application. The function returns an error code */
/* if unsuccessful, otherwise it returns 0.                             */
/*                                                                      */
/************************************************************************/

int nCwRegisterClasses(void)
{
 WNDCLASS   wndclass;    /* struct to define a window class             */
 memset(&wndclass, 0x00, sizeof(WNDCLASS));


 /* load WNDCLASS with window's characteristics                         */
 wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNWINDOW;
 wndclass.lpfnWndProc = WndProc;
 /* Extra storage for Class and Window objects                          */
 wndclass.cbClsExtra = 0;
 wndclass.cbWndExtra = 0;
 wndclass.hInstance = hInst;
 wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
 wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
 /* Create brush for erasing background                                 */
 wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
 wndclass.lpszMenuName = szAppName;   /* Menu Name is App Name */
 wndclass.lpszClassName = szAppName; /* Class Name is App Name */
 if(!RegisterClass(&wndclass))
   return -1;


 return(0);
} /* End of nCwRegisterClasses                                          */

/************************************************************************/
/*  CwUnRegisterClasses Function                                        */
/*                                                                      */
/*  Deletes any refrences to windows resources created for this         */
/*  application, frees memory, deletes instance, handles and does       */
/*  clean up prior to exiting the window                                */
/*                                                                      */
/************************************************************************/

void CwUnRegisterClasses(void)
{
 WNDCLASS   wndclass;    /* struct to define a window class             */
 memset(&wndclass, 0x00, sizeof(WNDCLASS));

 UnregisterClass(szAppName, hInst);
}    /* End of CwUnRegisterClasses                                      */

