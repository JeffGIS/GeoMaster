
//************* functions ***************
int pascal WinMain(HANDLE hInstance,HANDLE hPrevInstance,LPSTR lpszCmdParam,int nCmdShow);
long far pascal MainDlgProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
long far pascal WidthDlgProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
long far pascal InformationDlgProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

//************ defines *******************
/*# define ID_SETCOLWIDTHS			122
# define ID_VLINES					121
# define ID_FITTOWINDOW			120
# define ID_BESTFIT				119
# define ID_COLOR_NONE			118
# define ID_COLOR_OUTSTANDING	117
# define ID_COLOR_5				116
# define ID_TEXT_CENTER			114
#define ID_TEXT_RIGHT			113
#define ID_TEXT_LEFT				112
#define ID_MULTIPLE				111
#define ID_EXIT               110
#define ID_RESTORE_WIDTHS		109
#define ID_SAVE_WIDTHS			108
#define ID_GOTO_BOTTOM			107
#define ID_GOTO_TOP				106
#define ID_COLUMN					124
#define ID_WIDTH					125
#define ID_RADPIXELS	  		 	126
#define ID_RADCHARWIDTH			127
#define ID_OK						128
#define ID_ROW						129
#define ID_FONT					130
#define ID_COLWIDTH				132
#define ID_RECTANGLE				133
#define ID_CHARWIDTH				134

#define IDC_STATUS				101
#define IDC_SEARCH				102
#define IDC_TABLEHDG 			103
#define IDC_TABLE 				105
*/

typedef struct{
	char Company[36];
	char Name[36];
	int  Day;
	int  Month;
	int  Year;
	float Balance;
   int  flag;
}DATA;
