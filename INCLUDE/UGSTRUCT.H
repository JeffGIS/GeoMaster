/*******************************************************

*******************************************************/


/*******************************************************
Cell Information Structure
*******************************************************/
typedef struct UG_CellInfoTag{

	
	int			DataType;		// text,number,date,etc.
								// -1: not specified 0:text 1:Bool 
								// 2:number 3:Date 4:Time
	
	//string info
	CString	String;
	CString	Mask;

	CString LabelString;

	//number info
	double  Number;
	short	CommasFlag;
	short	Decimals;
	short	CurrencyFlag;

	//date and time
	short   D_S,M_M,Y_H;  //day-second, month-minute, year-hour
	short   MinD_S,MinM_M,MinY_H;
	short   MaxD_S,MaxM_M,MaxY_H;

	//min max
	double  Min;
	double	Max;
	short	MinMaxFlag;

	//format for dates, time, bool, etc
	short	DisplayFormat;	

	//color and style information
	COLORREF 	TextColor,BackColor;
	COLORREF 	HTextColor,HBackColor;
	
	short		Alignment;		//UG_LEFT, UG_RIGHT,UG_CENTER,UG_TOP,UG_BOTTOM,UG_VCENTER
	short		BorderStyle;	//Thin:		bit	0-left 1-top 2-right 3-bottom
								//Medium:	bit	4-left 5-top 6-right 7-bottom 
								//Thick:	bit	8-left 9-top 10-right 11-bottom 
								//3DRecess:	bit	12
								//3DRaised:	bit	13
								
	HPEN		BorderColor;
	HFONT		hFont;
	HBITMAP		hBitmap;

	short		ReadOnlyFlag;
					
	long		CellType;		//0-normal
	long		CellTypeEx;		//0-normal
	
	//cell extra memory
	BOOL		UseExtraMem;
	long		ExtraMemSize;
	void *		ExtraMem;

	//multi cell block
	BOOL		UseJoinInfo;
	BOOL		JoinOrigin;
	int			JoinCol;
	long		JoinRow;

	//data source and column translation
	short		DataCol;		
	short		DataSource;

}UG_CellInfo;




/*******************************************************
General Information Structure
*******************************************************/
typedef struct {

	//column info
	int NumberCols;
	int CurrentCol;
	int LastCol;
	int LeftCol;
	int LastLeftCol;
	int MaxLeftCol;
	int *ColWidths;
	int DefColWidth;

	//row info
	long NumberRows;
	long CurrentRow;
	long LastRow;
	long TopRow;
	long LastTopRow;
	long MaxTopRow;
	long *RowHeights;
	long DefRowHeight;
	int  UniformRowHeightFlag;	//true or false

	int  SizingMode;	//0 -off 1-normal 2-update on the fly

	//defaults
	CUGCell *GridDefaults;
	CUGCell **ColDefaults;
	CUGCell *HdgDefaults;

	//top heading info
	CUGCell **TH_CellInfo;
	//corner button info
	CUGCell *CB_CellInfo;
	//current cell
	CUGCell	*CurrentCell;

	//sizes
	int TopHdgHeight;
	int SideHdgWidth;
	int VScrollWidth;
	int HScrollHeight;
	int GridHeight;
	int GridWidth;
	int	ShowHScroll;
	int ShowVScroll;

	//highlighting
	int  HighlightRowFlag;
	int  MultiSelectFlag;
	int	 CurrentCellMode;  //mode(bits) 1:focus rect 2:highlight

	
	int  MovedBy;			// 1-keyboard 2-lbuttondown 3-mousemove
							//used by the Moved member, for internal use


	//other options
	int  MouseScrollFlag;
	int  AccessStyleSHFlag;
	int	 ThreeDHeight;

	//defaults
	HFONT DefaultFont;

	int PaintMode;

	int EnablePopupMenu;

	int EnableHints;
	int EnableVScrolHints;
	int EnableHScrolHints;

	//scrollbars
	int VScrollMode;		// 0-normal 2- tracking 3-joystick
	int HScrollMode;		// 0-normal 2- tracking 


	//editing
	long	EditRow;
	int		EditCol;

	//data translation functions
	BOOL PerformTranslations;

}UG_GridInfo;




