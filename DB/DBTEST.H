/* QuickCase:W KNB Version 1.00 */
#include <windows.h>
#include <string.h>
#include "COMPRESS.h"
#define IDM_FILE                  1000
#define IDM_F_NEW                      1050
#define IDM_F_OPEN                     1100
#define IDM_F_DELETE                   1150
#define IDM_F_EXIT                     1250
#define IDM_SHOWRECORDS           2000
#define IDM_ADDRECORDS            3000
#define IDM_IMPORT                4000
#define IDM_TESTLOAD              5000
#define IDM_COMPRESS              6000

#define IDS_ERR_REGISTER_CLASS   1
#define IDS_ERR_CREATE_WINDOW    2

char szString[128];   /* variable to load resource strings         */

char szAppName[20];   /* class name for the window               */
HWND hInst;          
HWND hWndMain;       

void cwCenter(HWND, int);

LONG FAR PASCAL WndProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL TESTLOADMsgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL COMPRESSMsgProc(HWND, WORD, WORD, LONG);
int nCwRegisterClasses(void);
void CwUnRegisterClasses(void);
#define ID_CANCEL                   101
#define TL_LB                       102
#define TL_UP                       103
#define TL_NEXT                     104
#define TL_BACK                     105
#define TL_NRECS                    106
#define TL_ADD                      107
#define TL_FIRST                    109
