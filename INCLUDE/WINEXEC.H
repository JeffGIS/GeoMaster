/*
	File: WinExec.h
	Author: Raja
	Date: 07/06/1995.

	Supplies a WinExec like function which returns only when the launched
	application exits. Similar to WinExec, but waits till launched
	application to exit.

	UINT WinExecEx (LPCSTR lpszCmdLine, int nCmdShow);

	Remember: This program will not work correctly when run under the debug
	mode. That is because, NotifyRegister handlers cannot be chained, and
	since most debuggers trap this notification, your application handler 
	will never get called.

	Only one application call WinExecEx, since hWndGlobal is a global
	variable and is shared. Therefore WinExecEx will return an error code of
	31 if another application has already launched an application with
	WinExecEx and waiting for it to terminate.
*/

#ifndef __WINEXECEX__
#define __WINEXECEX__


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
UINT FAR PASCAL WinExecEx (LPCSTR szCmdLine, UINT nCmdShow);
HINSTANCE FAR PASCAL GetCurrentInstance ();
HTASK FAR PASCAL GetTaskFromInstance (HINSTANCE hInst);
HINSTANCE FAR PASCAL GetInstanceFromTask (HTASK hTask);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __WINEXECEX__

// End-of-text.
