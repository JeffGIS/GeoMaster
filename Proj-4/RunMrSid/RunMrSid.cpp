// RunMrSid.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	char	cmd[256]={"C:\\MrSID Viewer\\MrSIDViewer.exe"};
	WinExec (cmd,SW_SHOWNORMAL);
	WinExec (cmd,SW_SHOWNORMAL);

	return 0;
}



