// projtest.cpp : Defines the entry point for the application.
//
#include <windows.h>
#include <winbase.h>
int __stdcall testnad (int i);
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	testnad (1);
	return 0;
}



