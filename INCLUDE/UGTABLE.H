/****************************************************************
	Ultimate Grid
	Dundas Software Ltd.
	Table Header File               version 1.3
	(C)opyright 1994-1995

	Note: This file may not be redistributed.
*****************************************************************/

//#include <windows.h>

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

//*********************** defines **************************
// date styles
#define DATE_CENTURY    16
#define DATE_DDMMYY     1
#define DATE_MMDDYY     2
#define DATE_DDMONYY    3
#define DATE_MONDDYY    4
#define DATE_DDMONTHYY  5
#define DATE_MONTHDDYY  6

// window styles
#define TS_HEADINGWINDOW    0x0010l    //Heading Window Style
#define TS_HEADING3D        0x0020l    //heading window with 3D button look

//************************* Messages *************************

// messages that retrieve information

#define TB_GETCHARWIDTH WM_USER+131    //returns the width of an average char for
													//the current table font (in pixels)
#define TB_GETCOLWIDTH  WM_USER+128    //returns the widths of a column in pixels
													//wParam=col
#define TB_GETFONT      WM_USER+136    //returns the current font handle (NULL for the sys font)
#define TB_GETHDGHWND   WM_USER+135 	//returns the hwnd of the tables heading window
#define TB_GETNUMCOLS   WM_USER+134 	//returns the number of cols on the entire table
#define TB_GETNUMROWS   WM_USER+133 	//returns the number of rows on the entire table
#define TB_GETRECT      WM_USER+130 	//gets the rect of a specific row-col
													//lParam =far * GETRECT
#define TB_GETSELROW    WM_USER+115    //returns the currently selected row number
#define TB_GETSELCOL    WM_USER+127    //returns the currently selected col number
#define TB_STRINGWIDTH  WM_USER+132    //returns the width of a string using the table font
													//in pixels   lParam= char *
#define TB_GETHDC       WM_USER+138 	//returns the tables HDC handle and also
													//sets the tables font

//messages that set information

#define TB_FSETCOLWIDTH WM_USER+137    //sets the width of a column by ave char width
													//wParam=column  lParam=width
#define TB_GOTOCOL      WM_USER+114    //moves to the col specified  wParam=col
#define TB_GOTOROW      WM_USER+113    //moves to the row specified  lParam=row
#define TB_SETCOLWIDTH  WM_USER+112    //sets the width of a column in pixels
													//wParam=column  lParam=width
#define TB_SETHDGHWND   WM_USER+119    //gives the table the handle to the heading window
													//lParam=hwnd
#define TB_SETINTERSPACE WM_USER+118   //sets the spacing between rows
													//wParam= number of pixels
#define TB_SETNUMCOLS   WM_USER+111    //sets the number of colums available
													//wParam=num cols up to 100
#define TB_SETNUMROWS   WM_USER+110    //sets the number of rows available:
													//lParam=num rows up to 2,100,000,000
#define TB_USERRESIZE   WM_USER+116    //turns user column risizing on and off
													// wParam= 1 or 0  (On/Off)
#define TB_SETVLINES    WM_USER+140    //turns vertical lines on or off
													//wParam = TRUE or FALSE
#define TB_GETADDRESS   WM_USER+141    //returns the TABLEINFO address

#define TB_SETHIGHLIGHT WM_USER+142    //sets the highlight flag
													//wParam = TRUE or FALSE

#define TB_SETHLINES    WM_USER+143    //turns horizontal lines on/off
													//wParam = TRUE or FALSE

#define TB_SETMARGIN		WM_USER+144		//sets the margin width (1 by default)
													// wParam = margin width in pixels
//notify messages (sent from the table or table heading)
//NOTE: all notify messages use the TABLEINFO structure

#define TBN_COLCHANGE   WM_USER+122    //notifies the parent of a col change
													//lParam= TABLEINFO *
#define TBN_KEYBOARD    WM_USER+124  	//returns the value of a key that was pressed
#define TBN_ROWCHANGE   WM_USER+121    //notifies the parent of a row change
													//lParam= TABLEINFO *
#define TBN_ROWSELECTED WM_USER+123    //notifies the parent of a row selection
													//lParam= TABLEINFO *
#define TBN_WANTHDG     WM_USER+125  	//notifies the main window that it must supply
													//the string for the current col heading
													//lParam= TABLEINFO *
#define TBN_WANTFTR     WM_USER+145  	//notifies the main window that it must supply
													//the string for the current col heading
													//lParam= TABLEINFO *

#define TBN_WANTTEXT    WM_USER+120  	//notifies the main window that it must supply
													//the string for the current row
													//lParam= TABLEINFO *
#define TBN_RBUTTONDOWN WM_USER+126 	//notifies the main window that the right mouse
													//button is down

//************************* Structures *************************

// this structure is sent to the parent in the lParam whenever it
// receives a NOTIFICATION message
typedef struct{
	UINT msg;               //notification being sent
	WPARAM wParam;          //general purpose variable

	WPARAM col;             //column
	LPARAM row;             //row

	COLORREF textcolor;     //text color extended option
	COLORREF backcolor;     //background color extended option
	WPARAM  alignment;      //text alignment extended option
	char buf[1024];         //text string

}TABLEINFO;

// this is used when retrieving the co-ords of a cell within the table
typedef struct{
	WPARAM col;             //col to return
	LPARAM  row;            //row to return
	RECT rect;              //structure to return the co-ords in
	int isvisible;  			//visibility flag  0-not visible 1- visible
									//                 2-partial visibility
}TABLEGETRECT;


//************************* Functions *************************



#ifdef __cplusplus
}
#endif  /* __cplusplus */

