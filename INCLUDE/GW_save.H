/* QuickCase:W KNB Version 1.00 */
#include <windows.h>
#include <string.h>
#define IDM_BUILD                 1000
#define IDM_SUBMIT                2000
#define IDM_STATUS                3000
#define IDM_SEP                    500
#define IDM_VIEW                  5000
#define IDM_PAN                   6000
#define IDM_P_LEFT                     6050
#define IDM_P_RIGHT                    6100
#define IDM_P_UP                       6150
#define IDM_P_DOWN                     6200
#define IDM_ZOOM                  7000
#define IDM_Z_IN                       7050
#define IDM_Z_OUT                      7100
#define IDM_Z_WINDOW                   7150
#define IDM_Z_ORTHO                    7200
#define IDM_Z_REZOOM                   7250
#define IDM_Z_REDRAW                   7251
#define IDM_LOCATE                8000
#define IDM_L_PARCEL                   8050
#define IDM_L_ADDRESS                  8100
#define IDM_L_INTERSECTION             8150
#define IDM_VISIBILITY            9000
#define IDM_CLEAR                 10000
#define IDM_PRINT                 12000
#define IDM_EXPORT                13000
#define IDM_E_TOCLIPBOARD              13050
#define IDM_E_TOFILE                   13100
#define IDM_HELP                  14000
#define IDM_H_BASICS                   14050
#define IDM_HLT_CLEAR			  15000
#define IDM_HLT_SAVE				   15010
#define IDM_TAG_CLEAR				   15020

#define IDS_ERR_REGISTER_CLASS   1
#define IDS_ERR_CREATE_WINDOW    2

char szString[128];   /* variable to load resource strings         */

char szAppName[20];   /* class name for the window               */

void cwCenterBelowCursor(HWND hWnd, int top);
void cwCenter(HWND, int);

LONG FAR PASCAL WndProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL BUILDMsgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL REQUESTMsgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL STATUSMsgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL VIEWMsgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL VISIBLEMsgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL CBTYPEMsgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL CLASSICMsgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL OUTPUTMsgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL PRINTINGMsgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL USERCBMsgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL IDENTIFYMsgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL RBUTOPSMsgProc(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL FILEDESCMsgProc(HWND, WORD, WORD, LONG);
int nCwRegisterClasses(void);
void CwUnRegisterClasses(void);
#define IDHELP                      103
#define CBB_TYPES_LB                108
#define CBB_DIR_CB                  113
#define CBB_CONTENTS_LB             114
#define CBR_DIR_CB                  101
#define CBS_LB                      102
#define CBV_CB                      112
#define CBV_DELETE                  104
#define CBB_DIR                     105
#define CBV_DIR                     106
#define CB_CLASSIC_TITLE            107
#define CB_CLASSIC_BASEMAP_LB       109
#define CB_CLASSIC_OPTIONS          110
#define CBR_PRIORITY_LB             111
#define OUTPUT_LB                   115
#define CBS_HELP                    116
#define CBS_REFRESH                 117
#define CBS_VIEW_OUTPUT             118
#define CBS_VIEW_PLOT               119
#define CBS_CHANGE_PRIORITY         120
#define CBS_DELETE                  121
#define CB_CLASSIC_APP              122
#define CB_CLASSIC_HELP             124
#define CB_TYPE_LB                  123
#define CB_CLASSIC_BMTYPE           125
#define CB_USER_DATAFILE            127
#define CB_USER_DATADIR             129
#define CB_CLASSIC_PROJECT          128
#define CBR_DIR                     126
#define CB_CLASSIC_PLTNAME          131
#define CB_USER_DESC                133
#define CBS_VIEW_REPORT             130
#define CB_USER_                    132
#define CB_USER_EDIT_DF             134
#define CB_USER_DESC_ICON           136
#define FILE_DESC_COLUMN_MARKER     135
#define FILE_DESC_A                 137
#define FILE_DESC_B                 138
#define FILE_DESC_C                 139
#define FILE_DESC_D                 140
#define FILE_DESC_E                 141
#define FILE_DESC_F                 142
#define FILE_DESC_G                 143
#define FILE_DESC_H                 144
#define FILE_DESC_I                 145
#define FILE_DESC_HSB               146
#define FILE_DESC_VSB               147
#define CB_CLASSIC_SAVE             148
#define CB_CLASSIC_SUBMIT           149
#define ID_CANCEL                   150
#define UNLOCK                      151
#define PNT_LOC_OPTS                152
#define SYM_VIS_LB                  153
#define AREA_VIS                    154
#define PLC_VIS                     155
#define TEXT_VIS                    156
#define TPOINT_MIN                  157
#define TPOINT_MAX                  158
#define REDRAW_AF_OK                159
#define LEFTBUT_CENTER              160
#define LEFTBUT_IDENTIFY            161
#define LEFTBUT_TOGGLEVIS           162
#define END_MACRO                   163
#define IDENTIFY_NEXT               164
#define IDENTIFY_LINE1              165
#define IDENTIFY_LINE2              166
#define IDENTIFY_LINE3              167
#define IDENTIFY_LINE4              168
#define PAR_VIS_LB                  169
#define VIS_ZOOM_LEV                171
#define ADD_ZOOM_LEVEL              173
#define SAVE_VIS                    174
#define VIS_HELP                    175
#define WIDTH_VIS                   177
#define GB_OFFSET_UNITS             170
#define GB_AREA                     172
#define GB_CONTENTS                 176
#define GB_OFFSET_VAL               178
#define GB_NAME                     179
#define GB_AREA_TYPES               180
#define GB_AREA_OF_TYPE             181
#define CB_ADDRESS                  182
#define CB_ADDRESS_MULT             183
#define CB_ADDRESS_MESSAGE          184
#define CB_STREET2                  185
#define CB_STREET1                  187
#define GB_VIEW_AREA                186
#define GB_SCALE_VALUE              188
#define GB_SCALE_UNITS              190
#define GB_SCALE_TITLE              191
#define CB_INT_DELETE               189
#define GEO_PID                     192
#define GEO_ADDRESS                 193
#define GB_DEST_COOKBOOK            194
#define GB_DEST_PC                  195
#define IDD_SV_THEME                108
#define IDD_ACTIVATE_THEME          109

