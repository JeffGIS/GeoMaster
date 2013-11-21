/***************************************************
Ultimate Grid 3.0 for MFC
Dundas Software Ltd. 1994-1996

  Standard Defines
****************************************************/

//movement defines
#define UG_LINEUP	0
#define UG_LINEDOWN	1
#define UG_PAGEUP	2
#define UG_PAGEDOWN	3
#define UG_TOP		4
#define UG_BOTTOM	5

//common return codes  - error and warning codes are positive values
#define UG_SUCCESS		0
#define UG_NA			-1
#define UG_PROCESSED	-2

//data types
#define UG_NODATA		-1
#define UG_TEXTDATA		0
#define UG_BOOLDATA		1
#define UG_NUMBERDATA	2
#define UG_DATEDATA		3
#define UG_TIMEDATA		4

//data source
#define	UG_MAXDATASOURCES 5

//scrolling
#define UG_SCROLLNORMAL		0
#define UG_SCROLLTRACKING	1
#define UG_SCROLLJOYSTICK	2

//alignment defines
#define UG_ALIGNLEFT		1
#define UG_ALIGNRIGHT		2
#define UG_ALIGNCENTER		4
#define UG_ALIGNTOP			8
#define UG_ALIGNBOTTOM		16
#define UG_ALIGNVCENTER		32

//border style defines
#define UG_BDR_LTHIN		1
#define UG_BDR_TTHIN		2
#define UG_BDR_RTHIN		4
#define UG_BDR_BTHIN		8
#define UG_BDR_LMEDIUM		16
#define UG_BDR_TMEDIUM		32
#define UG_BDR_RMEDIUM		64
#define UG_BDR_BMEDIUM		128
#define UG_BDR_LTHICK		256
#define UG_BDR_TTHICK		512
#define UG_BDR_RTHICK		1024
#define UG_BDR_BTHICK		2048
#define UG_BDR_RECESSED		4096
#define UG_BDR_RAISED		8192


//printing defines
#define UG_PRINT_TOPHEADING		1
#define UG_PRINT_SIDEHEADING	2
#define UG_PRINT_LEFTMARGIN		3
#define UG_PRINT_TOPMARGIN		4
#define UG_PRINT_RIGHTMARGIN	5
#define UG_PRINT_BOTTOMMARGIN	6


//bit defines
#define BIT0	1
#define BIT1	2
#define BIT2	4
#define BIT3	8
#define BIT4	16
#define BIT5	32
#define BIT6	64
#define BIT7	128
#define BIT8	256
#define BIT9	512
#define BIT10	1024
#define BIT11	2048
#define BIT12	4096
#define BIT13	8192
#define BIT14	16384
#define BIT15	32768
#define BIT16	65536l
#define BIT17	131072l
#define BIT18	262144l
#define BIT19	524288l
#define BIT20	1048576l
#define BIT21	2097152l
#define BIT22	4194304l
#define BIT23	8388608l
#define BIT24	16777216l
#define BIT25	33554432l
#define BIT26	67108864l
#define BIT27	134217728l
#define BIT28	268435456l
#define BIT29	536870912l
#define BIT30	1073741824l
#define BIT31	2147483648l


//Standard Cell Type ID defines
#define UGCT_NORMAL			0
#define UGCT_CHECKBOX		1
#define UGCT_DROPLIST		2
#define UGCT_RADIOBUTTON	3
#define UGCT_AUTOFONT		4
#define UGCT_PROGRESS		5
#define UGCT_SPINBUTTON		6
#define UGCT_MULTILINE		7
#define UGCT_ARROW			8


//Normal CellTypeEx Values
#define UGCT_NORMALSINGLELINE	1 (default)
#define UGCT_NORMALMULTILINE	2

//CheckBox CellTypeEx values
#define UGCT_CHECKBOX3DRECESS	BIT8
#define UGCT_CHECKBOX3DRAISED	BIT9
#define UGCT_CHECKBOXFLAT		0
#define UGCT_CHECKBOXCHECKMARK	BIT12
#define UGCT_CHECKBOXCROSS		0


//Arrow CellTypeEx values
#define UGCT_ARROWRIGHT		16
#define UGCT_ARROWLEFT		17
#define	UGCT_ARROWDRIGHT	18
#define	UGCT_ARROWDLEFT		19

//SpinButton CellTypeEx Values
//Spinbutton Notification messages
#define UGCT_SPINBUTTONUP	16
#define UGCT_SPINBUTTONDOWN	17


//Droplist cell type notification messages
#define UGCT_DROPLISTSTART	100
#define UGCT_DROPLISTSELECT 101


//Grid Sections - used by menu commands,
#define UG_GRID				1
#define UG_TOPHEADING		2
#define UG_SIDEHEADING		3
#define UG_CORNERBUTTON		4
#define UG_HSCROLL  		5
#define UG_VSCROLL  		6

//printing defines
#define UG_PRINTTOPHEADING	1
#define UG_PRINTSIDEHEADING	2
#define UG_PRINTLANDSCAPE	4
#define UG_PRINTSMALL		8
#define UG_PRINTLARGE		16

