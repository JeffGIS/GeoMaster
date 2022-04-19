
/* dyndlg.h */
/* defines and structs for Dynamic Dialog Box */

#ifndef DYNDLG_H
#define DYNDLG_H


#define ID_FNAME 3
#define IDM_OPEN 41
#define DYNM_CLOSE WM_USER+1
#define DYNM_ENTER WM_USER+2
#define DYNM_NEWFILE WM_USER+3

#define DLG_COUNT 5
#define MAX_STRING 80

#define PROP_DLGINFO "DLGINFO"
#define WINDOW_NAME "Dynamic Dialogs"



typedef struct{
		int   diiLength;
		int   diiPosition;  // in output
	      } DlgItemInfo;

typedef struct{
		GLOBALHANDLE dihDlgTmp;
		int diRecSize;
		DlgItemInfo diItems[1];
	      } DlgInfo;
typedef DlgInfo	FAR *LPDLGINFO;
typedef struct
     { 
       FIELDINFO  FI;
       char FldContents[128],
            MinVal[32],
            MaxVal[32],
            DefVal[32];
unsigned int  FldFormat,
              FldID,
              FldType,
              FldNumDigits,
              TempFldLen;
HGLOBAL       hCurrValue;                   
     } FldInfo;
typedef FldInfo FAR *lpFldInfo;
// function prototypes

int Lookup(char far *key,char far *table[]);
GLOBALHANDLE NewDlgTemplate(DWORD Style,WORD X,WORD Y,
		WORD CX,WORD CY,LPSTR Menu,
		LPSTR Class,LPSTR Text,
		LPSTR TypeFace,WORD PointSize);
int AddDlgItem(GLOBALHANDLE *DlgTmp,DWORD Style,
		WORD X,WORD Y,WORD CX,WORD CY,
		LPSTR Class,LPSTR Text,BYTE DataLen,
		LPSTR Data,BYTE Type);
int TemplateSize(GLOBALHANDLE hDlgTmp);
GLOBALHANDLE ReadDialogDef(char *filename);
HWND OpenModelessDialog(char *filename);
int FindDlgBox(HWND hDlg);

BOOL FAR PASCAL DialogProc(HWND,WORD,WPARAM,LPARAM);
BOOL FAR PASCAL FOpenProc(HWND,WORD,WPARAM,LPARAM);
BOOL  FAR PASCAL EnterProc(HWND hWnd,LPARAM Record);
void ShowDynWindows (void);
void CloseDynWindows (void);
void DynDlgOn (BOOL On);


extern DLGPROC lpDlgProc;
extern HINSTANCE hinstance;

// parser tables

enum {type_unused,TYPE_CALC,TYPE_SQL,TYPE_FIELD,FLD_TYPES};
      
enum{TYPE_Text,TYPE_INT,TYPE_REAL,TYPE_2DIGITS,
     TYPE_4DIGITS,TYPE_DOLLARS,TYPE_TIME,TYPE_MMDDYY,
     TYPE_DDMMMYY,TYPE_YYMMDD,TYPE_MMMDDYYYY,
     TYPE_FORMATS};
      
enum {TYPE_UNUSED,TYPE_EDIT,TYPE_CHECKBOX,
      TYPE_LTEXT, TYPE_ENTERBUTTON,TYPE_NUMERIC,
      TYPE_LISTBOX, TYPE_COMBOBOX,TYPE_RADIO,TYPE_NAME_COUNT};

#ifdef DEFINE_GLOBALS

char far *TypeNames[]={"UNUSED","EDIT","CHECKBOX",
                       "LTEXT","ENTERBUTTON", "NUMERIC",
                       "LISTBOX","COMBOBOX","RADIOBUTTON",NULL};

char far *ClassNames[]={"UNUSED","EDIT","BUTTON",
                        "STATIC","BUTTON","EDIT",
                        "LISTBOX","COMBOBOX","BUTTON"};

long FieldStyles[]=
           {
            0,
            WS_TABSTOP | WS_BORDER| WS_TABSTOP,
            WS_TABSTOP | BS_AUTOCHECKBOX| WS_TABSTOP,
            SS_LEFT, //WS_TABSTOP, //SS_LEFT,
            BS_PUSHBUTTON | WS_TABSTOP| WS_TABSTOP,
            WS_TABSTOP | WS_BORDER| WS_TABSTOP,
            LBS_STANDARD|LBS_EXTENDEDSEL |WS_TABSTOP,
            CBS_DROPDOWNLIST | WS_TABSTOP|CBS_SORT|CBS_AUTOHSCROLL,
            BS_AUTORADIOBUTTON};

BOOL	PopDynDlg;
RECT	MainRectDynDlg;

HINSTANCE hInstanceGlobal;
HWND hDlgBox[DLG_COUNT];
DLGPROC lpDlgProc=NULL;


#else
extern char far * TypeNames[];
extern char far *ClassNames[];
extern long FieldStyles[];

#endif

#endif
