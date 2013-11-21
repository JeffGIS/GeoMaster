/****************************************************************************
*                                                                           *
*                             GLOBAL VARIABLES                              *
*                                                                           *
****************************************************************************/




/****************************************************************************
*                                                                           *
*                             FUNCTION PROTOTYPES                           *
*                                                                           *
****************************************************************************/

long FAR PASCAL MainWndProc(HWND, UINT, WPARAM,LPARAM);
//int      PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);
BOOL FAR        InitInstance(HANDLE, int);
void FAR        ProcessCDError(DWORD);
void FAR        ReportError(WORD);
BOOL FAR PASCAL About(HWND, UINT, WPARAM,LPARAM);
UINT CALLBACK  FileOpenHook(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK  FindHook(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK  FindReplaceHook(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK  PrintSetupHook(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK  PrintDlgProc(HWND, UINT, WPARAM, LPARAM);
UINT CALLBACK  ColorHook (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
/*BOOL FAR PASCAL AbortProc(HDC, int);*/

/****************************************************************************
*                                                                           *
*                             MENU CONSTANTS                                *
*                                                                           *
****************************************************************************/

/*#define IDM_ABOUT          100
#define IDM_FILEOPEN       102
#define IDM_FILESAVE       103
#define IDM_FIND           104
#define IDM_FINDANDREPLACE 105
#define IDM_COLORS         106
#define IDM_FONTS          107
#define IDM_EXIT           109*/

/****************************************************************************
*                                                                           *
*                             LOADSTRING CONSTANTS                          *
*                                                                           *
****************************************************************************/

#define IDS_DIALOGFAILURE     5001
#define IDS_STRUCTSIZE        5002
#define IDS_INITIALIZATION    5003
#define IDS_NOTEMPLATE        5004
#define IDS_NOHINSTANCE       5005
#define IDS_LOADSTRFAILURE    5006
#define IDS_FINDRESFAILURE    5007
#define IDS_LOADRESFAILURE    5008
#define IDS_LOCKRESFAILURE    5009
#define IDS_MEMALLOCFAILURE  5010
#define IDS_MEMLOCKFAILURE   5011
#define IDS_NOHOOK           5012
#define IDS_SETUPFAILURE     5013
#define IDS_PARSEFAILURE     5014
#define IDS_RETDEFFAILURE    5015
#define IDS_LOADDRVFAILURE   5016
#define IDS_GETDEVMODEFAIL   5017
#define IDS_INITFAILURE      5018
#define IDS_NODEVICES        5019
#define IDS_NODEFAULTPRN     5020
#define IDS_DNDMMISMATCH     5021
#define IDS_CREATEICFAILURE  5022
#define IDS_PRINTERNOTFOUND  5023
#define IDS_NOFONTS          5024
#define IDS_SUBCLASSFAILURE  5025
#define IDS_INVALIDFILENAME  5026
#define IDS_BUFFERTOOSMALL   5027

#define IDS_SEARCHUP         5028
#define IDS_SEARCHDOWN       5029
#define IDS_CASESENSITIVE    5030
#define IDS_IGNORECASE       5031
#define IDS_WHOLEWORD        5032
#define IDS_WHOLEANDSUB      5033

#define IDS_FILTERSTRING     5034

/****************************************************************************
*                                                                           *
*                             OTHER CONSTANTS                               *
*                                                                           *
****************************************************************************/

#define IDC_OPENFILE            1
#define IDC_SAVEFILE            2
#define IDC_FIND                3
#define IDC_FINDREPLACE         4
#define IDC_COLORS              5
#define IDC_FONT                6
#define IDC_PRINTDLG            7

#define IDC_ALLOCFAIL           1
#define IDC_LOCKFAIL            2
#define IDC_LOADSTRINGFAIL      3

#define IDC_HELPMSG          2000
#define IDC_FINDREPLACEMSG   2001

#define MAXFILTERBUF          256
#define MAXFILENAMELEN        256
#define MAXFILETITLELEN       256
#define MAXFINDWHATLEN         64
#define MAXREPLACEWITHLEN      64
#define MAXFINDDIRECTIONLEN    24
#define MAXFINDCASELEN         24
#define MAXFINDWORDLEN         26

/****************************************************************************
*                                                                           *
*                          COMMOND DIALOG NOTIFICATION CODES                *
*                                                                           *
****************************************************************************/

#define CDN_FIND             1
#define CDN_FINDREPLACE      2


/****************************************************************************
*                                                                           *
*                          FILE OPEN STRUCTURE                              *
*                                                                           *
****************************************************************************/
typedef struct tagFOCHUNK
   {
      OPENFILENAME of;
      char szFile[MAXFILENAMELEN];
      char szFileTitle[MAXFILETITLELEN];
   }
FOCHUNK;

typedef FOCHUNK FAR *LPFOCHUNK;
typedef FOCHUNK FAR *LPFSCHUNK;


/****************************************************************************
*                                                                           *
*                          COLORS STRUCTURE                                 *
*                                                                           *
****************************************************************************/
typedef struct tagCOLORSCHUNK
   {
      CHOOSECOLOR chsclr;
      DWORD dwColor;
      DWORD dwCustClrs[16];
   }
COLORSCHUNK;

typedef COLORSCHUNK FAR *LPCOLORSCHUNK;


/****************************************************************************
*                                                                           *
*                          FIND STRUCTURE                                   *
*                                                                           *
****************************************************************************/

typedef struct tagFINDREPLACECHUNK
   {
      FINDREPLACE fr;
      char szFindWhat[MAXFINDWHATLEN];
      char szReplaceWith[MAXREPLACEWITHLEN];
   }
FINDREPLACECHUNK;

typedef FINDREPLACECHUNK FAR *LPFINDREPLACECHUNK;


#define IDD_ICON                    34
#define myicon                      35

LPSTR AllocAndLockMem (HANDLE *hChunk, WORD wSize);
short FormatFilterString (void);
void InitializeStruct (WORD wCommDlgType, LPSTR lpStruct);
/*****************************************************************************\
*                                                                             *
* colordlg.h -  Common dialog color dialog's control id numbers               *
*                                                                             *
*               Version 1.0                                                   *
*                                                                             *
*               Copyright (c) 1992, Microsoft Corp.  All rights reserved.     *
*                                                                             *
\*****************************************************************************/

#ifndef _INC_COLORDLG
#define _INC_COLORDLG

#define DLG_COLOR          10

#define COLOR_HUESCROLL   700	       /* color dialog */
#define COLOR_SATSCROLL   701
#define COLOR_LUMSCROLL   702
#define COLOR_HUE         703
#define COLOR_SAT         704
#define COLOR_LUM         705
#define COLOR_RED         706
#define COLOR_GREEN       707
#define COLOR_BLUE        708
#define COLOR_CURRENT     709
#define COLOR_RAINBOW     710
#define COLOR_SAVE        711
#define COLOR_ADD         712
#define COLOR_SOLID       713
#define COLOR_TUNE        714
#define COLOR_SCHEMES     715
#define COLOR_ELEMENT     716
#define COLOR_SAMPLES     717
#define COLOR_PALETTE     718
#define COLOR_MIX         719
#define COLOR_BOX1        720
#define COLOR_CUSTOM1     721

#define COLOR_HUEACCEL    723
#define COLOR_SATACCEL    724
#define COLOR_LUMACCEL    725
#define COLOR_REDACCEL    726
#define COLOR_GREENACCEL  727
#define COLOR_BLUEACCEL   728

#define IDC_SOLID	  730
#define IDC_50PCT	  731
#define IDC_25PCT	  732
#define IDC_12PCT	  733
#define IDC_50PCT2	  734
#define IDC_25PCT2	  735
#define IDC_12PCT2	  736
#define IDC_OUTCOLOR	  737
#define IDC_XHATCH	  738 
#define IDC_XHATCH2   739

#define NUM_BASIC_COLORS   48
#define NUM_CUSTOM_COLORS  16

#endif  /* !_INC_COLORDLG */
