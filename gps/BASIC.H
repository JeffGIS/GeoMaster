/* QuickCase:W KNB Version 1.00 */
#include <windows.h>
#include <string.h>    
#include <stdlib.h>

#define IDS_ERR_REGISTER_CLASS   1
#define IDS_ERR_CREATE_WINDOW    2

char szString[128];   /* variable to load resource strings         */

char szAppName[20];   /* class name for the window               */
HWND hInst;          
HWND hWndMain;       


LONG FAR PASCAL WndProc(HWND, WORD, WORD, LONG);
int nCwRegisterClasses(void);
void CwUnRegisterClasses(void);
