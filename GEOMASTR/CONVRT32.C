#include "graphint.h" 
#include "dibapi.h"

#include "gmextern.h"

static short	NumIB=0;
static LPCOORDINATEDISPLAY	CD;
static HANDLE	handle=NULL;


static	struct	{unsigned	short	HaveImage:1,
	 								Unused1:1, 
	 								Unused2:1, 
	 								Unused3:1,
	 								Len:12;
	 			}ConfigDesc;


#pragma pack(1)			
typedef	struct
	{	
		short	ID;
		HANDLE16	Handle;
		short	DisplayVP, TargetVP;
		short	Type;
		RECT16	SavedRect;
		HANDLE16 SavedScreen;
		POINTS	BoxPoints[7];  
		long	CurAreaRef;
		char	CurAreaRefGlobal[34];  
		BOOL16	IgnoreClear; 
		short	VPBoundsOpt; //1=keepcurrentbounds,other=usevislim
		char	BoundsGlobal[34];  
	} BOUNDSDISPLAY16;
typedef BOUNDSDISPLAY16 FAR  *LPBOUNDSDISPLAY16;

typedef struct
	{	HANDLE16		Handle;
		unsigned	char	Type:2; 
		unsigned	char	Style:3;
		unsigned	char	Width:3;
		unsigned	char	R,G,B;
	}	NEWOBJECT_v0;
			
typedef struct
	{	HANDLE16		Handle;  
		float		Width;
		unsigned	char	Type:2;   //if Type = 0 Width = text factor
		unsigned	char	Style:3; 
		unsigned	char	ProPen:3;
		unsigned	char	R,G,B;
	}	NEWOBJECT16;
typedef NEWOBJECT16 FAR  *LPNEWOBJECT16;
typedef struct
	{	HANDLE16	hVisList;
		LPSTR	LastVisList,
				NextVisList;
		double	MinScale,
				MaxScale;
		short	WantType[10];
		short	FileIsVisible[MAX_VIEWPORT_FILES16];
		short		MinPointSize;
		short		MaxPointSize;
		BYTE	VisBits[400];

	}	VISLIST16;
typedef VISLIST16	FAR	*LPVISLIST16;


typedef struct {
                short   Version,
                        NumFiles,
                        NumFields, 
                        TotFileLen;  
                HANDLE16  hSQL[5];
                char    FileNames[5][128];
                FIELDINFO16   FldInfo;     
                }   COMBOFILE16;
typedef COMBOFILE16   FAR *LPCOMBOFILE16;

typedef struct
	{
		short 		ID,  
					Version,
					Parent,
					Type,			/* 0 = Format, 1 = Plan, 2 = Profile, 3 = Cross Section,
									   4 = 3D,   5 = Menu,    6 = Data, 7= Global View		 */
					ZoomTarget;     /* For type 7 only. Indicates which viewport is
									   zoomed/panned when zoom/pan commands entered in this
									   viewport.											 */  
		BOOL		Active;
		HWND		hWnd;
		HDC			hDC;
		BOOL		Display;
		char		Name[32];		/* Name of window to appear on Viewports menu. If NULL  
									   this viewport will not appear on Viewports menu		 */
		USHORT 		ActivateMenuID; /* Viewports menu id of this viewport					 */
		COLORREF	BackGroundColor,
					BorderColor;
		BOOL		Shadow;
		float		BorderPct;      /* Width of vp border line                               */
		float		DesiredHeight,	/* Establishes aspect ratio and plot					 */
					DesiredWidth;	/* size. On screen or plotters smaller
								       than desired size, aspect ratio will
								       be preserved. Applies only to Viewport 1				.*/
	 	short 		TagPointID;		/* 1 = lower left, 2 = upper left, 3 = upper right,
	 								   4 = lower right										 */
	 	short 		TagPointType;	/* 1 = % of largest dimension, 2 = % of x dim if x, y 
	 								   dim if y												 */
	 	DPOINT		TagPoint;		/* if relative coords represent percent of parent 		 */
	 	POINT		TagPointActual;	/* actual Window coordinates							 */
	 	short 		WidthType, 		/* 1 = fixed, 2 = relative								 */
	 				HeightType;
	 	float		Width,
	 				Height;
	 	float		Margin;			/* inner margin as a percent of the max dimension		 */
	 	BOOL		MarginPan;		/* if true picking in the margin pans map				 */
	 	RECT		Rect;			/* The current screen rect for this viewport 			 */
	 	RECT		DrawRect;
	 	RECT		ZBRect;			/* The relative dimensions of the zoom box - diff from
	 								   Rect if target of zoom is another VP					 */
	 	HRGN		hRgn;      
	 	HBITMAP		Bitmap;			/* The saved bitmap for type 7 viewports                 */ 
	 	RECT		BitmapRect;		/* The dimensions of the saved bitmap					 */
		MNMXCORD	WBounds,		/* Current world coordinate bounds of viewport			 */
					NewBounds,		/* Desired new bounds. May be adjusted to fit viewport.  */
					FileBounds,     /* Current file coordinate bounds of viewport			 */
					FileMNMX;		/* World coordinate bounds of current file				 */
		MNMXCORL	Bounds;			/* Current file coordinate bounds of viewport			 */
		HANDLE		hMaskArea,
					hTranWinToBase,
					hTranBaseToWin;
		double		BaseUnitsPerPixel,
					VisScale;
		short 		FileFactor;
		BOOL		HaveBounds;
		short 		BoundsDisplayID;/* The Viewport ID in which this vps bounds are displayed*/
		LPBOUNDSDISPLAY		lpBoundsDisplay;
		BOOL        WindowIsZoomed,
					WindowZoomedToOrtho;
		short 		wOrigX, wOrigY, wExtX, wExtY, vExtX, vExtY;
		LPTHEME		pTheme;
		short 		NumThemes;
		LPTHEME		pThemes[MAX_VIEWPORT_THEMES_v7]; /* Pointers to themes which this viewport
													 is a target of */
		short 		BoundsDisplayVP;			  /* If this VP is used as a bounds display
													 VP for another VP this is the others VP id.*/
		BOOL		BoundsDisplayed;			  /* TRUE if bounds currently displayed in BoundsDisplayVP */
		short 		PassID;			/* Current pass - 0 = Theme pass, 1 = Display pass; */
		BOOL		WantPass[2];
		short 		NumFiles;
		short 		CurFile; 
		short 		SubFile;
		char		OrigFile[128];
		BOOL		FirstFile;
		short 		FileType[MAX_VIEWPORT_FILES16];  /* 1 = Format (Viewport Coord),
													  2 = Standard (World Coord),
													  3 = Bitmap file,
													  4 = Plot file directory (World Coord)
													  5 = Ortho Photo directory              */
		LPSTR		lpFiles[MAX_VIEWPORT_FILES16];   /* Files or file sets associated with viewport			 */
		char		FileID[MAX_VIEWPORT_FILES16][16];/* File identifier displayed to user      */
		HANDLE		hlpIndex[MAX_VIEWPORT_FILES16];   /* HANDLES to file indexes if type 4 or 5     */
/*		LPFILEINDEX	lpIndex[MAX_VIEWPORT_FILES16];    */
		short 		NumVisList;
		LPVISLIST16	pVisList1;		/* Points to first (top) visibility list in chain 		 */
		LPVISLIST16	pVisListManual;	/* Points to manual vis list if in manual mode else NULL */ 
		LPVISLIST16	CurVis;
		short 		NumPickList;
		LPVISLIST16	pPickList1;		/* Points to first (top) pick list in chain 		 */
		LPVISLIST16	pPickListManual;/* Points to manual pick list if in manual mode else NULL */ 
		char		VisName[128];
		char		PickName[128];
		short 		FunctionStack[MAX_FUNCTION_STACK];
		short 		LenFunctionStack;
		char		FunctionFile[128]; /* File containing active function list */
		char		FunctionDir[128];   /* Current directory of functions */  
		char		DisplayRedefFile[128];
		HANDLE		hReport,
					hTAGList;
		BOOL		ShrinkToFit;
		char		Prefix[10],
					UDI[34];
		long		ReportRefno;
		double		ReportFactor;
		short		CurVisType[3201];	//1=line, 2=linewithtext,3=area, 4=Point, 5=Pointwithtext
		HANDLE		hPenRedef;
		short 		NumNewObjects;
		short 		NewObjectMap[3201];
		NEWOBJECT_v0	NewObject[MAX_NEW_OBJECTS];    /* redefines display characteristics for this view*/
        char		EndOfViewport;
	}	VIEWPORT_v0;
typedef VIEWPORT_v0	FAR	*LPVIEWPORT_v0;  
typedef struct
	{
		short		ID,  
					Version,
					Parent,
					Type,			/* 0 = Format, 1 = Plan, 2 = Profile, 3 = Cross Section,
									   4 = 3D,   5 = Menu,    6 = Data, 7= Global View		 */
					ZoomTarget;     /* For type 7 only. Indicates which viewport is
									   zoomed/panned when zoom/pan commands entered in this
									   viewport.											 */  
		BOOL		Active;
		HWND		hWnd;
		HDC			hDC;
		BOOL		Display;
		char		Name[32];		/* Name of window to appear on Viewports menu. If NULL  
									   this viewport will not appear on Viewports menu		 */
		USHORT		ActivateMenuID; //no longer used
		COLORREF	BackGroundColor,
					BorderColor;
		BOOL		Shadow;
		float		BorderPct;      /* Width of vp border line                               */
		float		DesiredHeight,	/* Establishes aspect ratio and plot					 */
					DesiredWidth;	/* size. On screen or plotters smaller
								       than desired size, aspect ratio will
								       be preserved. Applies only to Viewport 1				.*/
	 	short		TagPointID;		/* 1 = lower left, 2 = upper left, 3 = upper right,
	 								   4 = lower right										 */
	 	short		TagPointType;	/* 1 = % of largest dimension, 2 = % of x dim if x, y 
	 								   dim if y												 */
	 	DPOINT		TagPoint;		/* if relative coords represent percent of parent 		 */
	 	POINT		TagPointActual;	/* actual Window coordinates							 */
	 	short		WidthType, 		/* 1 = fixed, 2 = relative								 */
	 				HeightType;
	 	float		Width,
	 				Height;
	 	float		Margin;			/* inner margin as a percent of the max dimension		 */
	 	BOOL		MarginPan;		/* if true picking in the margin pans map				 */
	 	RECT		Rect;			/* The current screen rect for this viewport 			 */
	 	RECT		DrawRect;
	 	RECT		ZBRect;			/* The relative dimensions of the zoom box - diff from
	 								   Rect if target of zoom is another VP					 */
	 	HRGN		hRgn;      
	 	HBITMAP		Bitmap;			/* The saved bitmap for type 7 viewports                 */ 
	 	RECT		BitmapRect;		/* The dimensions of the saved bitmap					 */
		MNMXCORD	WBounds,		/* Current world coordinate bounds of viewport			 */
					NewBounds,		/* Desired new bounds. May be adjusted to fit viewport.  */
					FileBounds,     /* Current file coordinate bounds of viewport			 */
					FileMNMX;		/* World coordinate bounds of current file				 */
		MNMXCORL	Bounds;			/* Current file coordinate bounds of viewport			 */
		HANDLE		hMaskArea,
					hTranWinToBase,
					hTranBaseToWin;
		double		BaseUnitsPerPixel,
					VisScale;
		short		FileFactor;
		BOOL		HaveBounds;
		short		BoundsDisplayID;/* The Viewport ID in which this vps bounds are displayed*/
		LPBOUNDSDISPLAY		lpBoundsDisplay;
		BOOL        WindowIsZoomed,
					WindowZoomedToOrtho;
		short		wOrigX, wOrigY, wExtX, wExtY, vExtX, vExtY;
		LPTHEME		pTheme;
		short		NumThemes;
		LPTHEME		pThemes[MAX_VIEWPORT_THEMES_v7]; /* Pointers to themes which this viewport
													 is a target of */
		short		BoundsDisplayVP;			  /* If this VP is used as a bounds display
													 VP for another VP this is the others VP id.*/
		BOOL		BoundsDisplayed;			  /* TRUE if bounds currently displayed in BoundsDisplayVP */
		short		PassID;			/* Current pass - 0 = Theme pass, 1 = Display pass; */
		BOOL		WantPass[2];
		short		NumFiles;
		short		CurFile; 
		short		SubFile;
		char		OrigFile[128];
		BOOL		FirstFile;
		short		FileType[MAX_VIEWPORT_FILES16];  /* 1 = Format (Viewport Coord),
													  2 = Standard (World Coord),
													  3 = Editable file (World Coord)
													  4 = Plot file directory (World Coord)
													  5 = Ortho Photo directory              */
		LPSTR		lpFiles[MAX_VIEWPORT_FILES16];   /* Files or file sets associated with viewport			 */
		char		FileID[MAX_VIEWPORT_FILES16][16];/* File identifier displayed to user      */
		HANDLE		hlpIndex[MAX_VIEWPORT_FILES16];   /* HANDLES to file indexes if type 4 or 5     */
/*		LPFILEINDEX	lpIndex[MAX_VIEWPORT_FILES16];    */
		short		NumVisList;
		LPVISLIST16	pVisList1;		/* Points to first (top) visibility list in chain 		 */
		LPVISLIST16	pVisListManual;	/* Points to manual vis list if in manual mode else NULL */ 
		LPVISLIST16	CurVis;
		short		NumPickList;
		LPVISLIST16	pPickList1;		/* Points to first (top) pick list in chain 		 */
		LPVISLIST16	pPickListManual;/* Points to manual pick list if in manual mode else NULL */ 
		char		VisName[128];
		char		PickName[128];
		short		FunctionStack[MAX_FUNCTION_STACK];
		short		LenFunctionStack;
		char		FunctionFile[128]; /* File containing active function list */
		char		FunctionDir[128];   /* Current directory of functions */  
		char		DisplayRedefFile[128];
		HANDLE		hReport,
					hTAGList;
		BOOL		ShrinkToFit;
		char		Prefix[10],
					UDI[34];
		long		ReportRefno;
		double		ReportFactor;
		short		CurVisType[3201]; 
		HANDLE		hPenRedef;
		short		NumNewObjects;
		short		NewObjectMap[3201];
		NEWOBJECT_v0	NewObject[MAX_NEW_OBJECTS];    /* redefines display characteristics for this view*/
		char		PickMacroFile[128];
		BOOL		DisplayFunStack; 
		DPOINT		FunStackLoc;
		FARPROC lpfnFUNSTACKMsgProc; 
		HWND	FunStackWnd; 
		short		LinkedTo; 
		double		OrthoRes; 
		char		Space[1010];
        char		EndOfViewport;     
        
	}	VIEWPORT_v2;
typedef VIEWPORT_v2	FAR	*LPVIEWPORT_v2;    
typedef struct
{	
	short		Version;  
	char	Desc[34];
    BOOL	AutoEdit;
	RECT	rect;
	DPOINT	TAGPoint;
	POINT	TAGPointScr;
	POINT	ConnectPoint;
	POINT		RestorePoint;
	DPOINT		center; 
	short			ViewportID;
	short			CoordStyle;
	short			BGstyle;
	short			PLwidth;
	short			PLstyle;
	short			BorderStyle, Margin;
	float		TXheight;
	short			Just;
	short			inc;
	COLORREF	BGcolor;
	COLORREF	TXcolor;
	COLORREF	BorderColor;
	COLORREF	PointerColor;
	HBITMAP		before, after;
	double		bmWidthD,bmHeightD;
	short			bmWidth, bmHeight; 
	BOOL		Shadow;
	LOGFONT		LogFont;
	short			DataFileType; 
	HANDLE		hTAGDB;
	short			SymNum;
	char		Contents[10];
	long		Refno;
	HANDLE		hReport;
	char		DataFile[128];
	char		SQL[256];
	char		text[1024];   
	
} TAGBOX_v0;
typedef struct
{	
	short		Version;  
	char		Desc[34];
    IBFLAGS		Flags;
	RECT16		rect;
	DPOINT		TAGPoint;
	POINTS		TAGPointScr;
	POINTS		ConnectPoint;
	POINTS		RestorePoint;
	DPOINT		center; 
	short		ViewportID;
	short		CoordStyle;
	short		BGstyle;
	short		PLwidth;
	short		PLstyle;
	short		BorderStyle, Margin;
	float		TXheight;
	short		Just;
	short		inc;
	COLORREF	BGcolor;
	COLORREF	TXcolor;
	COLORREF	BorderColor;
	COLORREF	PointerColor;
	HBITMAP16		before, after;
	double		bmWidthD,bmHeightD;
	short		bmWidth, bmHeight; 
	BOOL16		Shadow;
	LOGFONT16		LogFont;
	short		DataFileType; 
	HANDLE16	hTAGDB;
	short		SymNum;
	float		Factor;
	char		dummy1[6];
	long		Refno;
	HANDLE16	hReport;
	char		Prefix[10],UDI[34];
	char		DataFile[128];
	char		SQL[256];
	char		text[864];
	MNMXCORD	BlowUpBounds;  
	char		PickMacroFile[128];
	
} TAGBOX_V1;
typedef TAGBOX_V1	FAR *LPTAGBOX_V1;
typedef struct
	{
		short		ID,  
					Version,
					Parent,
					Type,			/* 0 = Format, 1 = Plan, 2 = Profile, 3 = Cross Section,
									   4 = 3D,   5 = Menu,    6 = Legend Viewport, 7= Global View, 8=LegendImage,9=SubViewport		 */
					ZoomTarget;     /* For type 7 only. Indicates which viewport is
									   zoomed/panned when zoom/pan commands entered in this
									   viewport.											 */  
		BOOL16		Active;
		HWND16		hWnd;
		HDC16			hDC;
		BOOL16		Display;
		char		Name[32];		/* Name of window to appear on Viewports menu. If NULL  
									   this viewport will not appear on Viewports menu		 */
		USHORT 		ActivateMenuID; //no longer used
		COLORREF	BackGroundColor,
					BorderColor;
		BOOL16		Shadow;
		float		BorderPct;      /* Width of vp border line                               */
		float		DesiredHeight,	/* Establishes aspect ratio and plot					 */
					DesiredWidth;	/* size. On screen or plotters smaller
								       than desired size, aspect ratio will
								       be preserved. Applies only to Viewport 1				.*/
	 	short		TagPointID;		/* 1 = lower left, 2 = upper left, 3 = upper right,
	 								   4 = lower right										 */
	 	short		TagPointType;	/* 1 = % of largest dimension, 2 = % of x dim if x, y 
	 								   dim if y												 */
	 	DPOINT		TagPoint;		/* if relative coords represent percent of parent 		 */
	 	POINTS		TagPointActual;	/* actual Window coordinates							 */
	 	short		WidthType; 		/* 1 = fixed, 2 = relative								 */
	 	unsigned	short			DisplayInInches:1,
	 								DisplayedFullScreen:1, 
	 								AutoSize:1, 
	 								LinkToScale:1,
	 								ShowFullScreen:1,
	 								ConvertToGray:1, 
	 								CloseIcon:1, 
	 								ProfileInCrossSection:1,    
	 								ShrinkParent:1,
	 								DisableZoomMacro:1,
	 								AutoVisIcon:1,
									unusedflags:5;
	 	float		Width,
	 				Height;
	 	float		Margin;			/* inner margin as a percent of the max dimension		 */
	 	BOOL16		MarginPan;		/* if true picking in the margin pans map				 */
	 	RECT16		Rect;			/* The current screen rect for this viewport 			 */
	 	RECT16		DrawRect;
	 	RECT16		ZBRect;			/* The relative dimensions of the zoom box - diff from
	 								   Rect if target of zoom is another VP					 */
	 	HRGN16		hRgn;      
	 	HBITMAP16		Bitmap;			/* The saved bitmap for type 7 viewports                 */ 
	 	RECT16		BitmapRect;		/* The dimensions of the saved bitmap					 */
		MNMXCORD	WBounds,		/* Current world coordinate bounds of viewport			 */
					NewBounds,		/* Desired new bounds. May be adjusted to fit viewport.  */
					FileBounds,     /* Current file coordinate bounds of viewport			 */
					FileMNMX;		/* World coordinate bounds of current file				 */
		MNMXCORL	Bounds;			/* Current file coordinate bounds of viewport			 */
		HANDLE16		hMaskArea,
					hTranWinToBase,
					hTranBaseToWin;
		double		BaseUnitsPerPixel,
					VisScale;
		short		FileFactor;
		BOOL16		HaveBounds;
		short		BoundsDisplayID;/* The Viewport ID in which this vps bounds are displayed*/
		LPBOUNDSDISPLAY		lpBoundsDisplay;
		BOOL16        WindowIsZoomed,
					WindowZoomedToOrtho;
		short		wOrigX, wOrigY, wExtX, wExtY, vExtX, vExtY;
		LPTHEME		pTheme;
		short		NumThemes;
		LPTHEME		pThemes[MAX_VIEWPORT_THEMES_v7]; /* Pointers to themes which this viewport
													 is a target of */
		short		BoundsDisplayVP;			  /* If this VP is used as a bounds display
													 VP for another VP this is the others VP id.*/
		BOOL16		BoundsDisplayed;			  /* TRUE if bounds currently displayed in BoundsDisplayVP */
		short		PassID;			/* Current pass - 0 = Theme pass, 1 = Display pass; */
		BOOL16		WantPass[2];
		short		NumFiles;
		short		CurFile; 
		short		SubFile;
		char		OrigFile[128];
		BOOL16		FirstFile;
		short		FileType[MAX_VIEWPORT_FILES16];  /* 1 = Format (Viewport Coord),
													  2 = Standard (World Coord),
													  3 = TXT, BMP, JPG, TIF, PCX or SBM file
													  4 = Plot file directory (World Coord)
													  5 = Ortho Photo directory
													  6 = Highlight List 
													  7 = Display Macro
													  8 = Sub Viewport
													  9 = DTM*/
		LPSTR		lpFiles[MAX_VIEWPORT_FILES16];   /* Files or file sets associated with viewport			 */
		char		FileID[MAX_VIEWPORT_FILES16][16];/* File identifier displayed to user      */
		HANDLE16		hlpIndex[MAX_VIEWPORT_FILES16];   /* HANDLES to file indexes if type 4 or 5     */
/*		LPFILEINDEX	lpIndex[MAX_VIEWPORT_FILES16];    */
		short		NumVisList;
		LPVISLIST16	pVisList1;		/* Points to first (top) visibility list in chain 		 */
		LPVISLIST16	pVisListManual;	/* Points to manual vis list if in manual mode else NULL */ 
		LPVISLIST16	CurVis;
		short		NumPickList;
		LPVISLIST16	pPickList1;		/* Points to first (top) pick list in chain 		 */
		LPVISLIST16	pPickListManual;/* Points to manual pick list if in manual mode else NULL */ 
		char		VisName[128];
		char		PickName[128];
		short		OldFunctionStact0;
		short		StartupFunction;
		HANDLE16		FunStackHandle;  
		short		CurrentFunction;
		short		HalfTone; 
		BYTE		NumRanColor, //0=use default globals
		 			PrimeColorMin[3],
		 			PrimeColorMax[3],
		 			Space4; 
		RECT16		AutoVisIconRect;
		double		ZMScale;
		char		FunctionFile[128]; /* File containing active function list */
		char		FunctionDir[128];   /* Current directory of functions */  
		char		DisplayRedefFile[128];
		HANDLE16		hReport,
					hTAGList;
		BYTE		ShrinkToFit,FitToWindow;
		char		Prefix[10],
					UDI[34];
		long		ReportRefno;
		double		ReportFactor;
		BYTE		CurVisType[3201];  
		COLORREF	DarkContourColor[MAX_VIEWPORT_FILES16];  
		COLORREF	ContourTextColor[MAX_VIEWPORT_FILES16];    
		long		MaxFileDisplayedPointWidth[MAX_VIEWPORT_FILES16]; 
		double		MetersPerPixel;
		double		MetersPerDegreeX;
		char		space3[3201-800-70-4*MAX_VIEWPORT_FILES16-4*MAX_VIEWPORT_FILES16-4*MAX_VIEWPORT_FILES16-2*8]; 
		double		LastProfileAZ;   
		double		LastProfileDist; 
		DPOINT		LastProfilePoint;    
		DPOINT		ProfileCrossSection[2]; 
		HANDLE16		hProfileRouteSave;
		long		nProfileRouteSave;
		float		NewObjectTextFactor[MAX_NEW_OBJECTS];
		RGBTRIPLE	NewObjectTextColor[MAX_NEW_OBJECTS];
		BYTE		NewObjectSetTextColor[MAX_NEW_OBJECTS];
		HANDLE16		hPenRedef;
		short		NumNewObjects;
		short		NewObjectMap[3201];
		NEWOBJECT16	NewObject[MAX_NEW_OBJECTS];    /* redefines display characteristics for this view*/
		char		PickMacroFile[128];
		BOOL16		DisplayFunStack; 
		DPOINT		FunStackLoc;
		long lpfnFUNSTACKMsgProc; 
		HWND16	FunStackWnd; 
		short		LinkedTo; 
		double		OrthoRes;  
       	HANDLE16	hBinFileList;
       	HCURSOR16 hCursor, LastCursor;  
       	short	UpdateFile;  
       	BOOL16	HaveOrthos; 
       	HANDLE16	hEditRect;
       	BOOL16	HaveEditRect;
		short	EditRectInfoSize;
		short	EditRectSize;  
		USHORT	MaxEditRect;
		USHORT	NumEditRects;
		char	OrthoDisplayName[32]; 
		MNMXCORD	PriorBounds; 
		USHORT	NumMaskPoints;
		HANDLE16	LinkedCursorHandle;
		short	RestoreFile;
		HANDLE16	hFileTransIn, hFileTransOut;
		long	BoundsDisplayCycle; 
		long	BitmapID;
		BOOL16	Transparent;   
		HANDLE16	ToolbarHandle; 
		short	CurrentIcon;
		RECT16	CurrentIconRect; 
		char	RButFunction[128];
		short	OnPrintAddSpaceToVP; 
		HANDLE16	hProfileRoute;
		long	nProfileRoute;
		HANDLE16	hProfileElev;
		long	nProfileElev; 
		double	LLNormFactor; 
		short	DisplayInParent; 
		long	CurZoomAreaRef;
		char	CurVisibilityID[36]; 
//99b		
		DPOINT	TagPointSave;
		float	SaveWidth, SaveHeight;
        float	ScaleDist;
        short	ScaleDistUnits, ScaleBase; 
        BYTE	HalfToneVisBits[400];
        short	HalfToneNewObjectStart;
		LPVISLIST16	CurPick; 
		POINTS	LastCursorPos; 
		long	MaxSymbolWidth;    
		RECT16	CloseIconRect;
		BYTE	HaveLayerColor[MAX_VIEWPORT_FILES16];  
		COLORREF	LayerColor[MAX_VIEWPORT_FILES16];  
		MNMXCORD	SaveWBounds; 
		short	PreviousCommandVP;
		BYTE	DTMRenderAs[MAX_VIEWPORT_FILES16]; 
		MNMXCORD	WBoundsWhenSaved;
		signed	char	DTMContourIntOrSlopeFactor[MAX_VIEWPORT_FILES16];  
		BYTE	StretchImage[MAX_VIEWPORT_FILES16]; 
		HANDLE16	hMaskAccelerator[3];   
		HANDLE16	hDistanceLine; 
		long	MaskAreaRefno;    
		short	NumMaskAreaParts;   
		double	Rotation,
				Scale;
		DPOINT	MidPointW;  
		HANDLE16	hProfileDataRectangles;
		short	nProfileDataRectangles;
		short	ProfileDistUnits;  
        char		EndOfViewport;     
        
	}	VIEWPORT_V6;
typedef VIEWPORT_V6	FAR	*LPVIEWPORT_V6;    
#pragma	pack()

#pragma pack(2)
typedef struct
	{
		short		ID,  
					Version,
					Parent,
					Type,			/* 0 = Format, 1 = Plan, 2 = Profile, 3 = Cross Section,
									   4 = 3D,   5 = Menu,    6 = Legend Viewport, 7= Global View, 8=LegendImage,9=SubViewport		 */
					ZoomTarget;     /* For type 7 only. Indicates which viewport is
									   zoomed/panned when zoom/pan commands entered in this
									   viewport.											 */  
		BOOL		Active;
		HWND		hWnd;
		HDC			hDC;
		BOOL		Display;
		char		Name[32];		/* Name of window to appear on Viewports menu. If NULL  
									   this viewport will not appear on Viewports menu		 */
		USHORT 		ActivateMenuID; //no longer used
		COLORREF	BackGroundColor,
					BorderColor;
		BOOL		Shadow;
		float		BorderPct;      /* Width of vp border line                               */
		float		DesiredHeight,	/* Establishes aspect ratio and plot					 */
					DesiredWidth;	/* size. On screen or plotters smaller
								       than desired size, aspect ratio will
								       be preserved. Applies only to Viewport 1				.*/
	 	short		TagPointID;		/* 1 = lower left, 2 = upper left, 3 = upper right,
	 								   4 = lower right										 */
	 	short		TagPointType;	/* 1 = % of largest dimension, 2 = % of x dim if x, y 
	 								   dim if y												 */
	 	DPOINT		TagPoint;		/* if relative coords represent percent of parent 		 */
	 	POINT		TagPointActual;	/* actual Window coordinates							 */
	 	short		WidthType; 		/* 1 = fixed, 2 = relative								 */
	 	unsigned	short			DisplayInInches:1,
	 								DisplayedFullScreen:1, 
	 								AutoSize:1, 
	 								LinkToScale:1,
	 								ShowFullScreen:1,
	 								ConvertToGray:1, 
	 								CloseIcon:1, 
	 								ProfileInCrossSection:1,    
	 								ShrinkParent:1,
	 								DisableZoomMacro:1,
	 								AutoVisIcon:1,
									ZoomLocked:1,
									AlwaysUseHalfTone:1,
									unusedflags:3;
	 	float		Width,
	 				Height;
	 	float		Margin;			/* inner margin as a percent of the max dimension		 */
	 	BOOL		MarginPan;		/* if true picking in the margin pans map				 */
	 	RECT		Rect;			/* The current screen rect for this viewport 			 */
	 	RECT		DrawRect;
	 	RECT		ZBRect;			/* The relative dimensions of the zoom box - diff from
	 								   Rect if target of zoom is another VP					 */
		RECT		ScreenRect;		// Viewport Screen rect - differs from drawrect if rotated
	 	HRGN		hRgn;      
	 	HBITMAP		Bitmap;			/* The saved bitmap for type 7 viewports                 */ 
	 	RECT		BitmapRect;		/* The dimensions of the saved bitmap					 */
		MNMXCORD	WBounds,		/* Current world coordinate bounds of viewport			 */
					NewBounds,		/* Desired new bounds. May be adjusted to fit viewport.  */
					BoundsBeforeRotation,
					FileBounds,     /* Current file coordinate bounds of viewport			 */
					FileMNMX;		/* World coordinate bounds of current file				 */
		MNMXCORL	Bounds;			/* Current file coordinate bounds of viewport			 */
		HANDLE		hMaskArea,
					hTranVPToBase,
					hTranBaseToVP,
					hTranVPToScreen,
					hTranScreenToVP;
		double		BaseUnitsPerPixel,
					VisScale;
		short		FileFactor;
		BOOL		HaveBounds;
		short		BoundsDisplayID;/* The Viewport ID in which this vps bounds are displayed*/
		LPBOUNDSDISPLAY		lpBoundsDisplay;
		BOOL        WindowIsZoomed,
					WindowZoomedToOrtho;
		int		wOrigX, wOrigY, wExtX, wExtY, vExtX, vExtY;
		LPTHEME		pTheme;
		short		NumThemes;
		LPTHEME		pThemes[MAX_VIEWPORT_THEMES_v7]; /* Pointers to themes which this viewport
													 is a target of */
		short		BoundsDisplayVP;			  /* If this VP is used as a bounds display
													 VP for another VP this is the others VP id.*/
		BOOL		BoundsDisplayed;			  /* TRUE if bounds currently displayed in BoundsDisplayVP */
		short		PassID;			/* Current pass - 0 = Theme pass, 1 = Display pass; */
		BOOL		WantPass[2];
		short		NumFiles;
		short		CurFile; 
		short		SubFile;
		char		OrigFile[MAX_PATH];
		BOOL		FirstFile;
		short		FileType[MAX_VIEWPORT_FILES];  /* 1 = Format (Viewport Coord),
													  2 = Standard (World Coord),
													  3 = TXT, BMP, JPG, TIF, PCX or SBM file
													  4 = Plot file directory (World Coord)
													  5 = Ortho Photo directory
													  6 = Highlight List 
													  7 = Display Macro
													  8 = Sub Viewport
													  9 = DTM*/
		LPSTR		lpFiles[MAX_VIEWPORT_FILES];   /* Files or file sets associated with viewport			 */
		char		FileID[MAX_VIEWPORT_FILES][MAX_VPFILE_ID_v7];/* File identifier displayed to user      */
		HANDLE		hlpIndex[MAX_VIEWPORT_FILES];   /* HANDLES to file indexes if type 4 or 5     */
		short		NumVisList;
		LPVISLIST	pVisList1;		/* Points to first (top) visibility list in chain 		 */
		LPVISLIST	pVisListManual;	/* Points to manual vis list if in manual mode else NULL */ 
		LPVISLIST	CurVis;
		short		NumPickList;
		LPVISLIST	pPickList1;		/* Points to first (top) pick list in chain 		 */
		LPVISLIST	pPickListManual;/* Points to manual pick list if in manual mode else NULL */ 
		char		VisName[MAX_PATH];
		char		PickName[MAX_PATH];
		short		OldFunctionStact0;
		short		StartupFunction;
		HANDLE		FunStackHandle;  
		short		CurrentFunction;
		short		HalfTone; 
		BYTE		NumRanColor, //0=use default globals
		 			PrimeColorMin[3],
		 			PrimeColorMax[3],
		 			Space4; 
		RECT		AutoVisIconRect;
		double		ZMScale;
		char		FunctionFile[MAX_PATH]; /* File containing active function list */
		char		FunctionDir[MAX_PATH];   /* Current directory of functions */  
		char		DisplayRedefFile[MAX_PATH];
		HANDLE		hReport,
					hTAGList;
		BYTE		ShrinkToFit,FitToWindow;
		char		Prefix[10],
					UDI[34];
		long		ReportRefno;
		double		ReportFactor;
		BYTE		CurVisType[3202];  
		COLORREF	DarkContourColor[MAX_VIEWPORT_FILES];  
		COLORREF	ContourTextColor[MAX_VIEWPORT_FILES];    
		long		MaxFileDisplayedPointWidth[MAX_VIEWPORT_FILES]; 
		double		MetersPerPixel;
		double		MetersPerDegreeX;
		double		LastProfileAZ;   
		double		LastProfileDist; 
		DPOINT		LastProfilePoint;    
		DPOINT		ProfileCrossSection[2]; 
		HANDLE		hProfileRouteSave;
		long		nProfileRouteSave;
		float		NewObjectTextFactor[MAX_NEW_OBJECTS];
		RGBTRIPLE	NewObjectTextColor[MAX_NEW_OBJECTS];
		BYTE		NewObjectSetTextColor[MAX_NEW_OBJECTS];
		HANDLE		hPenRedef;
		short		NumNewObjects;
		short		NewObjectMap[3202];
		NEWOBJECT	NewObject[MAX_NEW_OBJECTS];    /* redefines display characteristics for this view*/
		char		PickMacroFile[MAX_PATH];
		BOOL		DisplayFunStack; 
		DPOINT		FunStackLoc;
		FARPROC lpfnFUNSTACKMsgProc; 
		HWND	FunStackWnd; 
		short		LinkedTo; 
		double		OrthoRes;  
       	HANDLE	hBinFileList;
       	HCURSOR hCursor, LastCursor;  
       	short	UpdateFile;  
       	BOOL	HaveOrthos; 
       	HANDLE	hEditRect;
       	BOOL	HaveEditRect;
		int		EditRectInfoSize;
		int		EditRectSize;  
		UINT	MaxEditRect;
		UINT	NumEditRects;
		char	OrthoDisplayName[32]; 
		MNMXCORD	PriorBounds; 
		UINT	NumMaskPoints;
		HANDLE	LinkedCursorHandle;
		short	RestoreFile;
		HANDLE	hFileTransIn, hFileTransOut;
		short	hFileTransInStatusx;
		long	BoundsDisplayCycle; 
		long	BitmapID;
		BOOL	Transparent;   
		HANDLE	ToolbarHandle; 
		HANDLE	CurrentIcon;
		RECT	CurrentIconRect; 
		char	RButFunction[MAX_RBUTFUN_v7];
		short	OnPrintAddSpaceToVP; 
		HANDLE	hProfileRoute;
		long	nProfileRoute;
		HANDLE	hProfileElev;
		long	nProfileElev; 
		double	LLNormFactor; 
		short	DisplayInParent; 
		long	CurZoomAreaRef;
		char	CurVisibilityID[36]; 
		DPOINT	TagPointSave;
		float	SaveWidth, SaveHeight;
        float	ScaleDist;
        short	ScaleDistUnits, ScaleBase; 
        BYTE	HalfToneVisBits[400];
        short	HalfToneNewObjectStart;
		LPVISLIST	CurPick; 
		POINT	LastCursorPos; 
		long	MaxSymbolWidth;    
		RECT	CloseIconRect;
		BYTE	HaveLayerColor[MAX_VIEWPORT_FILES];  
		COLORREF	LayerColor[MAX_VIEWPORT_FILES];  
		MNMXCORD	SaveWBounds; 
		short	PreviousCommandVP;
		BYTE	DTMRenderAs[MAX_VIEWPORT_FILES]; 
		MNMXCORD	WBoundsWhenSaved;
		signed	char	DTMContourIntOrSlopeFactor[MAX_VIEWPORT_FILES];  
		BYTE	StretchImage[MAX_VIEWPORT_FILES]; 
		HANDLE	hMaskAccelerator[3];   
		HANDLE	hDistanceLine; 
		long	MaskAreaRefno;    
		int	NumMaskAreaParts;   
		double	Rotation,
				Scale;
		XFORM	xForm;
		DPOINT	MidPointW;  
		HANDLE	hProfileDataRectangles;
		int	nProfileDataRectangles;
		int	ProfileDistUnits; 
		char	MeterInit[1024];
		char	RButPickInit[1024];
		int		LastWidth,
				LastHeight;
		char	DataDisplayRectFile[MAX_PATH];
		int		CurDataRectID;
		int		nDataDisplayRect;
		HANDLE	hDataDisplayRect;
		short	SaveParent;
		char	GrowSpace[3814];
        char		EndOfViewport;     
        
	}	VIEWPORT_V7;
typedef VIEWPORT_V7	FAR	*LPVIEWPORT_V7;  
#pragma pack()

void ConvertVP_V7_to_V8 (LPVIEWPORT CurView,LPVIEWPORT_V7 pVP7);
void ConvertVP_V6_to_V7 (LPVIEWPORT_V7 CurView,LPVIEWPORT_V6 pVP6);
void ConvertSymbol_V1_to_V2 (LPSYMBOL pSym,LPSYMBOL_V1 pSym1);
void ConvertSymbol_V2_to_V1 (LPSYMBOL_V1 pSym,LPSYMBOL pSym1);
void ConvertTB_V1_to_V2 (LPTAGBOX pTB,LPTAGBOX_V1 pTB16);
LOGFONT ConvertLF16_to_32 (LPLOGFONT16 pLF16);
FIELDINFO Field16toField32 (LPFIELDINFO16 pField);
FIELDINFO16 Field32toField16 (LPFIELDINFO pField);
HOTSPOTDATA Hotspot16toHotspot32 (LPHOTSPOTDATA16 pHS16);
NEWOBJECT NewObject16_to_NewObject (LPNEWOBJECT16 pNO16);
void ConvertThemeV1toV2 (LPTHEME CurTheme,LPTHEME_V1 pTheme_v1);
void ConvertCD_V103_to_V104 (LPCOORDINATEDISPLAY pCD,LPCOORDINATEDISPLAY_V103 pCD103);
void ComboFile16ToComboFile32 (LPCOMBOFILE p32,LPCOMBOFILE16 p16);
void ComboFile32ToComboFile16 (LPCOMBOFILE16 p32,LPCOMBOFILE p16);
void STREETTEXTDATA16ToSTREETTEXTDATA32 (LPSTREETTEXTDATA p32,LPSTREETTEXTDATA16 p16);
void ConvertVislist16to32 (LPVISLIST p32,LPVISLIST16 p16);

extern	int		SizeSYMBOL_V1;
extern	int		SizeBOUNDSDISPLAY16;
extern	int		SizeVISLIST16;

void SetOldStructSizes (void)
{
	SizeSYMBOL_V1 = sizeof(SYMBOL_V1);
	SizeBOUNDSDISPLAY16 = sizeof(BOUNDSDISPLAY16);
	SizeVISLIST16 = sizeof(VISLIST16);
	return;
}

BOOL ConvertCDV101To102 (HFILE Fid,LPCOORDINATEDISPLAY CD)
{
    COORDINATEDISPLAY_V101 CD101; 
    UINT	i;

   	GSSilread (Fid,&CD101,sizeof(COORDINATEDISPLAY_V101));

	CD->ID = CD101.ID;
	CD->Version = 102;
	CD->TargetViewport = CD101.TargetViewport;
	CD->DisplayViewport = CD101.DisplayViewport; 
	_fmemmove (CD->AltCVTFile,CD101.AltCVTFile,14);
	CD->Font = ConvertLF16_to_32(&CD101.Font);
	CD->FontColor = CD101.FontColor;
	CD->Refresh = CD101.Refresh;
	_fmemmove (CD->PrintText,CD101.PrintText,100);
	for (i=0;i<3;i++)
	{
		CD->DisplayLine[i] = CD101.DisplayLine[i];  
		_fmemmove (CD->LineID[i],CD101.LineID[i],34); 
		CD->Units[i] = CD101.Units[i];
		CD->Precision[i] = CD101.Precision[i];
		CD->Commas[i] = CD101.Commas[i];
	}
	return TRUE;
}

BOOL ConvertCDV102To103 (HFILE Fid,LPCOORDINATEDISPLAY CD)
{
    COORDINATEDISPLAY_V102 CD102; 
    UINT	i;
	COORDINATEDISPLAY_V103 CD103;

	memset (&CD103,0,sizeof(COORDINATEDISPLAY_V103));
   	GSSilread  (Fid,&CD103,sizeof(COORDINATEDISPLAY_V102));
	ConvertCD_V103_to_V104 (CD,&CD103);

	return TRUE;
}

void ConvertVislist16to32 (LPVISLIST p32,LPVISLIST16 p16)
{
	int	i;
	_fmemset (p32,0,sizeof(VISLIST));  
	if (p16)
	{
		p32->MinScale = p16->MinScale;
		p32->MaxScale = p16->MaxScale;
		for (i=0;i<10;i++){
			p32->WantType[i] = p16->WantType[i];
		}
		for (i=0;i<MAX_VIEWPORT_FILES16;i++){
			p32->FileIsVisible[i] = p16->FileIsVisible[i];
		}
		p32->MinPointSize = p16->MinPointSize;
		p32->MaxPointSize = p16->MaxPointSize;
		for (i=0;i<400;i++){
			p32->VisBits[i] = p16->VisBits[i];
		}
	}
	return;
}

void ConvertSymbol_V1_to_V2 (LPSYMBOL pSym,LPSYMBOL_V1 pSym1)
{
	int	i;

	if (!pSym || !pSym1)
		return;
	_fmemset (pSym,0,sizeof(SYMBOL));  

	pSym->Number = pSym1->Number;
	pSym->Parent = pSym1->Parent;
	strncpy (pSym->Name,pSym1->Name,sizeof(pSym1->Name));
	strncpy (pSym->Desc,pSym1->Desc,sizeof(pSym1->Desc));
	pSym->BaseSize = pSym1->BaseSize;
	pSym->InVisible = pSym1->InVisible;
	pSym->Type = pSym1->Type;
	pSym->DisplayPos = pSym1->DisplayPos;
	pSym->BlockRotation = pSym1->BlockRotation;
	pSym->HasBitmap = pSym1->HasBitmap;
	pSym->BaseScale = pSym1->BaseScale;
	pSym->GetDimensionsFromSizePoints = pSym1->GetDimensionsFromSizePoints;
	pSym->NoSizeLimit = pSym1->NoSizeLimit;
	pSym->BaseSizeSet = pSym1->BaseSizeSet;
	pSym->Layered = pSym1->Layered;
	pSym->NumElements = pSym1->NumElements;
	pSym->TiePoint = pSym1->TiePoint;
	for (i=0;i<2;i++){
		pSym->SizePointV[i] = pSym1->SizePointV[i];
	}
	for (i=0;i<2;i++){
		pSym->SizePointH[i] = pSym1->SizePointH[i];
	}
	pSym->VSize = pSym1->VSize;
	pSym->HSize = pSym1->HSize;
	pSym->hElement = (HANDLE)pSym1->hElement;
	return;
}

void ConvertSymbol_V2_to_V1 (LPSYMBOL_V1 pSym,LPSYMBOL pSym1)
{
	int	i;
	_fmemset (pSym,0,sizeof(SYMBOL_V1));  

	pSym->Number = pSym1->Number;
	pSym->Parent = pSym1->Parent;
	strncpy (pSym->Name,pSym1->Name,sizeof(pSym1->Name));
	strncpy (pSym->Desc,pSym1->Desc,sizeof(pSym1->Desc));
	pSym->BaseSize = pSym1->BaseSize;
	pSym->InVisible = pSym1->InVisible;
	pSym->Type = pSym1->Type;
	pSym->DisplayPos = pSym1->DisplayPos;
	pSym->BlockRotation = pSym1->BlockRotation;
	pSym->HasBitmap = pSym1->HasBitmap;
	pSym->BaseScale = pSym1->BaseScale;
	pSym->GetDimensionsFromSizePoints = pSym1->GetDimensionsFromSizePoints;
	pSym->NoSizeLimit = pSym1->NoSizeLimit;
	pSym->BaseSizeSet = pSym1->BaseSizeSet;
	pSym->Layered = pSym1->Layered;
	pSym->NumElements = pSym1->NumElements;
	pSym->TiePoint = pSym1->TiePoint;
	for (i=0;i<2;i++){
		pSym->SizePointV[i] = pSym1->SizePointV[i];
	}
	for (i=0;i<2;i++){
		pSym->SizePointH[i] = pSym1->SizePointH[i];
	}
	pSym->VSize = pSym1->VSize;
	pSym->HSize = pSym1->HSize;
	pSym->hElement = (HANDLE16)pSym1->hElement;
	return;
}
HANDLE	BufferToSymbol (HANDLE hBuf)
{
	HANDLE	hSym;
	HPSTR	pBuf=GlobalLock (hBuf), pStr; 
	long	lBuf=0; 
	LPSYMBOL	pSymDesc; 
	SYMBOL_V1	SymV1;
	LPHANDLE	phElement;
	short	i;
	
	hSym = AllocateNewSymbol ();
	pSymDesc = (LPSYMBOL)GlobalLock(hSym);  
	hmemmove ((HPSTR)&SymV1,pBuf,SizeSYMBOL_V1); 
	ConvertSymbol_V1_to_V2 (pSymDesc,&SymV1);
	pBuf += SizeSYMBOL_V1;
	if (pSymDesc->NumElements > 1)
	{   
		short	n=pSymDesc->NumElements;
		GlobalUnlock (hSym);
		hSym = GSSiGlobalReAlloc (0,hSym,sizeof(SYMBOL)+n*sizeof(HANDLE),GMEM_MOVEABLE);
		pSymDesc = (LPSYMBOL)GlobalLock (hSym); 
	}
	for (i=0,phElement=&pSymDesc->hElement;i<pSymDesc->NumElements;i++,phElement++)
	{ 
		LPELEMENT	pElement;
		
		*phElement = GSSiGlobAlloc ( 462,GMEM_MOVEABLE,sizeof(ELEMENT)+ (long)(MAX_ELEMENT_VECTORS-1)*sizeof(VECTOR));
		pElement = (LPELEMENT)GlobalLock (*phElement); 
		hmemmove ((HPSTR)pElement,pBuf,sizeof(ELEMENT));
		pBuf += sizeof(ELEMENT);
		switch (pElement->NumVectors)
		{
		case 0:
			pBuf -= sizeof(VECTOR);
			break;
		case 1:
			break;
		default:
			pStr = (HPSTR)pElement;
			pStr += sizeof(ELEMENT);
			hmemmove (pStr,pBuf,(pElement->NumVectors-1)*(long)sizeof(VECTOR)); 
			pBuf += (pElement->NumVectors-1)*(long)sizeof(VECTOR);
			break;
		}
		GlobalUnlock (*phElement); 
		*phElement = GSSiGlobalReAlloc (0,*phElement,sizeof(ELEMENT)+(max(0,pElement->NumVectors-1))*sizeof(VECTOR),GMEM_MOVEABLE);
	}   
	GlobalUnlock (hSym);	
	GlobalUnlock (hBuf);
	return hSym;
} 

long SymbolToBuffer (LPSYMBOL CurSymbol, LPHANDLE phBuf,short nDeleteElement,LPINT pDeleteElement)
{   
	long	lBuf=0;
	HPSTR	pBuf; 
	short	i, j, NumElements=CurSymbol->NumElements;
	LPHANDLE	phElement;   
	double	MaxLength=0;
	SYMBOL_V1	SymV1;
	
	*phBuf = GSSiGlobAlloc ( 463,GMEM_MOVEABLE,MAX_SYMBOL_SIZE);
	pBuf = GlobalLock (*phBuf); 
	if (CurSymbol->Type == SVLINE)  
	{
		for (i=0,phElement=&CurSymbol->hElement;i<NumElements;i++,phElement++)
		{ 
			LPELEMENT	pElement; 
			
			for (j=0;j<nDeleteElement;j++)
				if (i == pDeleteElement[j])
					goto SkipElement;
			pElement = (LPELEMENT)GlobalLock (*phElement);
			{ 
				LPVECTOR	pVector=&pElement->Vector;  
				 
				for (j=0;j<pElement->NumVectors;j++,pVector++)
					MaxLength = max (MaxLength,pVector->Dist);
			}
			GlobalUnlock (*phElement);   
	SkipElement:;
		}
		if (!CurSymbol->TiePoint.x)
			CurSymbol->TiePoint.x = MaxLength;
	} 
	if (CurSymbol->Type == 1 && !CurSymbol->GetDimensionsFromSizePoints)  
	{   
		CurSymbol->SizePointH[0].x = 0;
		CurSymbol->SizePointH[1].x = CurSymbol->HSize;
		CurSymbol->SizePointV[0].y = 0;
		CurSymbol->SizePointV[1].y = CurSymbol->VSize;  
		CurSymbol->GetDimensionsFromSizePoints = 1;  
		CurSymbol->HSize = 1;
		CurSymbol->VSize = 1;
	}
	CurSymbol->NumElements -= nDeleteElement;
	ConvertSymbol_V2_to_V1 (&SymV1,CurSymbol);
	BufWrite(&pBuf,&lBuf,(HPSTR)&SymV1,SizeSYMBOL_V1); 
	CurSymbol->NumElements = NumElements;
	for (i=0,phElement=&CurSymbol->hElement;i<NumElements;i++,phElement++)
	{ 
		LPELEMENT	pElement; 
		
		for (j=0;j<nDeleteElement;j++)
			if (i == pDeleteElement[j])
				goto DeleteElement;
		pElement = (LPELEMENT)GlobalLock (*phElement); 
		BufWrite(&pBuf,&lBuf,(HPSTR)pElement,sizeof(ELEMENT)+(pElement->NumVectors-1)*(long)sizeof(VECTOR));
		GlobalUnlock (*phElement);   
DeleteElement:;
	}
	GlobalUnlock (*phBuf);   
	return lBuf;
}


void ConvertVP_V7_to_V8 (LPVIEWPORT pCurView,LPVIEWPORT_V7 pVP7)
{
	int	l,i;

	memset (pCurView,0,sizeof(VIEWPORT)); 

	l = (LPSTR)pVP7->pThemes - (LPSTR)pVP7;
	memmove (pCurView,pVP7,l);
	l = (LPSTR)pVP7->FileID - (LPSTR)&pVP7->BoundsDisplayVP;
	memmove (&pCurView->BoundsDisplayVP,&pVP7->BoundsDisplayVP,l);
	for (i=0;i<MAX_VIEWPORT_FILES;i++)
		strcpy (pCurView->FileID[i],pVP7->FileID[i]);
	l = (LPSTR)pVP7->RButFunction - (LPSTR)pVP7->hlpIndex;
	memmove (pCurView->hlpIndex,pVP7->hlpIndex,l);
	strcpy (pCurView->RButFunction,pVP7->RButFunction);
	l = (LPSTR)&pVP7->EndOfViewport - (LPSTR)&pVP7->OnPrintAddSpaceToVP;
	memmove (&pCurView->OnPrintAddSpaceToVP,&pVP7->OnPrintAddSpaceToVP,l);
	if (pCurView->HaveBounds)
	{
		pCurView->ScreenRect = pCurView->DrawRect;
		SetScaleAndMidpointFromBounds (pCurView);
	}
	pCurView->Version = 8;
	return;
}

void ConvertVP_V6_to_V7 (LPVIEWPORT_V7 pCurView,LPVIEWPORT_V6 pVP6)
{
	int	i;
	_fmemset (pCurView,0,sizeof(VIEWPORT_V7));  
	
	pCurView->ID = pVP6->ID;
	pCurView->Version = 7;
	pCurView->Parent = pVP6->Parent;
	pCurView->Type = pVP6->Type;
	pCurView->ZoomTarget = pVP6->ZoomTarget;
	pCurView->Active = pVP6->Active;
	pCurView->hWnd = (HWND)pVP6->hWnd;
	pCurView->hDC = (HDC)pVP6->hDC;
	pCurView->Display = pVP6->Display;
	strncpy (pCurView->Name,pVP6->Name,sizeof(pVP6->Name));
	pCurView->ActivateMenuID = pVP6->ActivateMenuID;
	pCurView->BackGroundColor = pVP6->BackGroundColor;
	pCurView->BorderColor = pVP6->BorderColor;
	pCurView->Shadow = pVP6->Shadow;
	pCurView->BorderPct = pVP6->BorderPct;
	pCurView->DesiredHeight = pVP6->DesiredHeight;
	pCurView->DesiredWidth = pVP6->DesiredWidth;
	pCurView->TagPointID = pVP6->TagPointID;
	pCurView->TagPointType = pVP6->TagPointType;
	pCurView->TagPoint = pVP6->TagPoint;
	pCurView->TagPointActual = POINTStoPOINT(pVP6->TagPointActual);
	pCurView->WidthType = pVP6->WidthType;
	pCurView->DisplayInInches = pVP6->DisplayInInches;
	pCurView->DisplayedFullScreen = pVP6->DisplayedFullScreen;
	pCurView->AutoSize = pVP6->AutoSize;
	pCurView->LinkToScale = pVP6->LinkToScale;
	pCurView->ShowFullScreen = pVP6->ShowFullScreen;
	pCurView->ConvertToGray = pVP6->ConvertToGray;
	pCurView->CloseIcon = pVP6->CloseIcon;
	pCurView->ProfileInCrossSection = pVP6->ProfileInCrossSection;
	pCurView->ShrinkParent = pVP6->ShrinkParent;
	pCurView->DisableZoomMacro = pVP6->DisableZoomMacro;
	pCurView->AutoVisIcon = pVP6->AutoVisIcon;
	pCurView->unusedflags = pVP6->unusedflags;
	pCurView->Width = pVP6->Width;
	pCurView->Height = pVP6->Height;
	pCurView->Margin = pVP6->Margin;
	pCurView->MarginPan = pVP6->MarginPan;
	pCurView->Rect = Rect16ToRect32 (pVP6->Rect);
	pCurView->DrawRect = Rect16ToRect32 (pVP6->DrawRect);
	pCurView->ZBRect = Rect16ToRect32 (pVP6->ZBRect);
	pCurView->hRgn = (HRGN)pVP6->hRgn;
	pCurView->Bitmap = (HANDLE)pVP6->Bitmap;
	pCurView->BitmapRect = Rect16ToRect32 (pVP6->BitmapRect);
	pCurView->WBounds = pVP6->WBounds;
	pCurView->NewBounds = pVP6->NewBounds;
	pCurView->FileBounds = pVP6->FileBounds;
	pCurView->FileMNMX = pVP6->FileMNMX;
	pCurView->Bounds = pVP6->Bounds;
	pCurView->hMaskArea = (HANDLE)pVP6->hMaskArea;
	pCurView->hTranVPToBase = (HANDLE)pVP6->hTranWinToBase;
	pCurView->hTranBaseToVP = (HANDLE)pVP6->hTranBaseToWin;
	pCurView->BaseUnitsPerPixel = pVP6->BaseUnitsPerPixel;
	pCurView->VisScale = pVP6->VisScale;
	pCurView->FileFactor = pVP6->FileFactor;
	pCurView->HaveBounds = pVP6->HaveBounds;
	pCurView->BoundsDisplayID = pVP6->BoundsDisplayID;
	pCurView->lpBoundsDisplay = pVP6->lpBoundsDisplay;
	pCurView->WindowIsZoomed = pVP6->WindowIsZoomed;
	pCurView->WindowZoomedToOrtho = pVP6->WindowZoomedToOrtho;
	pCurView->wOrigX = pVP6->wOrigX;
	pCurView-> wOrigY = pVP6-> wOrigY;
	pCurView-> wExtX = pVP6-> wExtX;
	pCurView-> wExtY = pVP6-> wExtY;
	pCurView-> vExtX = pVP6-> vExtX;
	pCurView-> vExtY = pVP6-> vExtY;
	pCurView->pTheme = pVP6->pTheme;
	pCurView->NumThemes = pVP6->NumThemes;
	for (i=0;i<MAX_VIEWPORT_THEMES_v7;i++){
		pCurView->pThemes[i] = pVP6->pThemes[i];
	}
	pCurView->BoundsDisplayVP = pVP6->BoundsDisplayVP;
	pCurView->BoundsDisplayed = pVP6->BoundsDisplayed;
	pCurView->PassID = pVP6->PassID;
	for (i=0;i<2;i++){
		pCurView->WantPass[i] = pVP6->WantPass[i];
	}
	pCurView->NumFiles = pVP6->NumFiles;
	pCurView->CurFile = pVP6->CurFile;
	pCurView->SubFile = pVP6->SubFile;
	strncpy (pCurView->OrigFile,pVP6->OrigFile,sizeof(pVP6->OrigFile));
	pCurView->FirstFile = pVP6->FirstFile;
	for (i=0;i<MAX_VIEWPORT_FILES16;i++){
		pCurView->FileType[i] = pVP6->FileType[i];
	}
	for (i=0;i<MAX_VIEWPORT_FILES16;i++){
		pCurView->lpFiles[i] = pVP6->lpFiles[i];
	}
	for (i=0;i<MAX_VIEWPORT_FILES16;i++){
		strncpy (pCurView->FileID[i],pVP6->FileID[i],sizeof(pVP6->FileID[i]));
	}
	for (i=0;i<MAX_VIEWPORT_FILES16;i++){
		pCurView->hlpIndex[i] = (HANDLE)pVP6->hlpIndex[i];
	}
	pCurView->NumVisList = pVP6->NumVisList;
	pCurView->pVisList1 = (LPVISLIST)pVP6->pVisList1;
	pCurView->pVisListManual = (LPVISLIST)pVP6->pVisListManual;
	//ConvertVislist16to32 (pCurView->CurVis,pVP6->CurVis);
	pCurView->NumPickList = pVP6->NumPickList;
	pCurView->pPickList1 = (LPVISLIST)pVP6->pPickList1;
	pCurView->pPickListManual = (LPVISLIST)pVP6->pPickListManual;
	strncpy (pCurView->VisName,pVP6->VisName,sizeof(pVP6->VisName));
	strncpy (pCurView->PickName,pVP6->PickName,sizeof(pVP6->PickName));
	pCurView->OldFunctionStact0 = pVP6->OldFunctionStact0;
	pCurView->StartupFunction = pVP6->StartupFunction;
	pCurView->FunStackHandle = (HANDLE)pVP6->FunStackHandle;
	pCurView->CurrentFunction = pVP6->CurrentFunction;
	pCurView->HalfTone = pVP6->HalfTone;
	pCurView->NumRanColor = pVP6->NumRanColor;
	for (i=0;i<3;i++){
		pCurView->PrimeColorMin[i] = pVP6->PrimeColorMin[i];
	}
	for (i=0;i<3;i++){
		pCurView->PrimeColorMax[i] = pVP6->PrimeColorMax[i];
	}
	pCurView->Space4 = pVP6->Space4;
	pCurView->AutoVisIconRect = Rect16ToRect32 (pVP6->AutoVisIconRect);
	pCurView->ZMScale = pVP6->ZMScale;
	strncpy (pCurView->FunctionFile,pVP6->FunctionFile,sizeof(pVP6->FunctionFile));
	strncpy (pCurView->FunctionDir,pVP6->FunctionDir,sizeof(pVP6->FunctionDir));
	strncpy (pCurView->DisplayRedefFile,pVP6->DisplayRedefFile,sizeof(pVP6->DisplayRedefFile));
	pCurView->hReport = (HANDLE)pVP6->hReport;
	pCurView->hTAGList = (HANDLE)pVP6->hTAGList;
	pCurView->ShrinkToFit = pVP6->ShrinkToFit;
	pCurView->FitToWindow = pVP6->FitToWindow;
	strncpy (pCurView->Prefix,pVP6->Prefix,sizeof(pVP6->Prefix));
	strncpy (pCurView->UDI,pVP6->UDI,sizeof(pVP6->UDI));
	pCurView->ReportRefno = pVP6->ReportRefno;
	pCurView->ReportFactor = pVP6->ReportFactor;
	for (i=0;i<3201;i++){
		pCurView->CurVisType[i] = pVP6->CurVisType[i];
	}
	for (i=0;i<MAX_VIEWPORT_FILES16;i++){
		pCurView->DarkContourColor[i] = pVP6->DarkContourColor[i];
	}
	for (i=0;i<MAX_VIEWPORT_FILES16;i++){
		pCurView->ContourTextColor[i] = pVP6->ContourTextColor[i];
	}
	for (i=0;i<MAX_VIEWPORT_FILES16;i++){
		pCurView->MaxFileDisplayedPointWidth[i] = pVP6->MaxFileDisplayedPointWidth[i];
	}
	pCurView->MetersPerPixel = pVP6->MetersPerPixel;
	pCurView->MetersPerDegreeX = pVP6->MetersPerDegreeX;
	pCurView->LastProfileAZ = pVP6->LastProfileAZ;
	pCurView->LastProfileDist = pVP6->LastProfileDist;
	pCurView->LastProfilePoint = pVP6->LastProfilePoint;
	for (i=0;i<2;i++){
		pCurView->ProfileCrossSection[i] = pVP6->ProfileCrossSection[i];
	}
	pCurView->hProfileRouteSave = (HANDLE)pVP6->hProfileRouteSave;
	pCurView->nProfileRouteSave = pVP6->nProfileRouteSave;
	for (i=0;i<MAX_NEW_OBJECTS;i++){
		pCurView->NewObjectTextFactor[i] = pVP6->NewObjectTextFactor[i];
	}
	for (i=0;i<MAX_NEW_OBJECTS;i++){
		pCurView->NewObjectTextColor[i] = pVP6->NewObjectTextColor[i];
	}
	for (i=0;i<MAX_NEW_OBJECTS;i++){
		pCurView->NewObjectSetTextColor[i] = pVP6->NewObjectSetTextColor[i];
	}
	pCurView->hPenRedef = (HANDLE)pVP6->hPenRedef;
	pCurView->NumNewObjects = pVP6->NumNewObjects;
	for (i=0;i<3201;i++){
		pCurView->NewObjectMap[i] = pVP6->NewObjectMap[i];
	}
	for (i=0;i<MAX_NEW_OBJECTS;i++){
		pCurView->NewObject[i] = NewObject16_to_NewObject (&pVP6->NewObject[i]);
	}
	strncpy (pCurView->PickMacroFile,pVP6->PickMacroFile,sizeof(pVP6->PickMacroFile));
	pCurView->DisplayFunStack = pVP6->DisplayFunStack;
	pCurView->FunStackLoc = pVP6->FunStackLoc;
	pCurView->lpfnFUNSTACKMsgProc = (FARPROC)pVP6->lpfnFUNSTACKMsgProc;
	pCurView->FunStackWnd = (HWND)pVP6->FunStackWnd;
	pCurView->LinkedTo = pVP6->LinkedTo;
	pCurView->OrthoRes = pVP6->OrthoRes;
	pCurView->hBinFileList = (HANDLE)pVP6->hBinFileList;
	pCurView->hCursor = (HANDLE)pVP6->hCursor;
	pCurView-> LastCursor = (HCURSOR)pVP6-> LastCursor;
	pCurView->UpdateFile = pVP6->UpdateFile;
	pCurView->HaveOrthos = pVP6->HaveOrthos;
	pCurView->hEditRect = (HANDLE)pVP6->hEditRect;
	pCurView->HaveEditRect = pVP6->HaveEditRect;
	pCurView->EditRectInfoSize = pVP6->EditRectInfoSize;
	pCurView->EditRectSize = pVP6->EditRectSize;
	pCurView->MaxEditRect = pVP6->MaxEditRect;
	pCurView->NumEditRects = pVP6->NumEditRects;
	strncpy (pCurView->OrthoDisplayName,pVP6->OrthoDisplayName,sizeof(pVP6->OrthoDisplayName));
	pCurView->PriorBounds = pVP6->PriorBounds;
	pCurView->NumMaskPoints = pVP6->NumMaskPoints;
	pCurView->LinkedCursorHandle = (HANDLE)pVP6->LinkedCursorHandle;
	pCurView->RestoreFile = pVP6->RestoreFile;
	pCurView->hFileTransIn = (HANDLE)pVP6->hFileTransIn;
	pCurView-> hFileTransOut = (HANDLE)pVP6-> hFileTransOut;
	pCurView->BoundsDisplayCycle = pVP6->BoundsDisplayCycle;
	pCurView->BitmapID = pVP6->BitmapID;
	pCurView->Transparent = pVP6->Transparent;
	pCurView->ToolbarHandle = (HANDLE)pVP6->ToolbarHandle;
	pCurView->CurrentIcon = (HANDLE)pVP6->CurrentIcon;
	pCurView->CurrentIconRect = Rect16ToRect32 (pVP6->CurrentIconRect);
	strncpy (pCurView->RButFunction,pVP6->RButFunction,sizeof(pVP6->RButFunction));
	pCurView->OnPrintAddSpaceToVP = pVP6->OnPrintAddSpaceToVP;
	pCurView->hProfileRoute = (HANDLE)pVP6->hProfileRoute;
	pCurView->nProfileRoute = pVP6->nProfileRoute;
	pCurView->hProfileElev = (HANDLE)pVP6->hProfileElev;
	pCurView->nProfileElev = pVP6->nProfileElev;
	pCurView->LLNormFactor = pVP6->LLNormFactor;
	pCurView->DisplayInParent = pVP6->DisplayInParent;
	pCurView->CurZoomAreaRef = pVP6->CurZoomAreaRef;
	strncpy (pCurView->CurVisibilityID,pVP6->CurVisibilityID,sizeof(pVP6->CurVisibilityID));
	pCurView->TagPointSave = pVP6->TagPointSave;
	pCurView->SaveWidth = pVP6->SaveWidth;
	pCurView-> SaveHeight = pVP6-> SaveHeight;
	pCurView->ScaleDist = pVP6->ScaleDist;
	pCurView->ScaleDistUnits = pVP6->ScaleDistUnits;
	pCurView-> ScaleBase = pVP6-> ScaleBase;
	for (i=0;i<400;i++){
		pCurView->HalfToneVisBits[i] = pVP6->HalfToneVisBits[i];
	}
	pCurView->HalfToneNewObjectStart = pVP6->HalfToneNewObjectStart;
	//ConvertVislist16to32 (pCurView->CurPick,pVP6->CurPick);
	pCurView->LastCursorPos = POINTStoPOINT (pVP6->LastCursorPos);
	pCurView->MaxSymbolWidth = pVP6->MaxSymbolWidth;
	pCurView->CloseIconRect = Rect16ToRect32 (pVP6->CloseIconRect);
	for (i=0;i<MAX_VIEWPORT_FILES16;i++){
		pCurView->HaveLayerColor[i] = pVP6->HaveLayerColor[i];
	}
	for (i=0;i<MAX_VIEWPORT_FILES16;i++){
		pCurView->LayerColor[i] = pVP6->LayerColor[i];
	}
//	pCurView->SaveWBounds = pVP6->SaveWBounds;
	pCurView->PreviousCommandVP = pVP6->PreviousCommandVP;
	for (i=0;i<MAX_VIEWPORT_FILES16;i++){
		pCurView->DTMRenderAs[i] = pVP6->DTMRenderAs[i];
	}
	pCurView->WBoundsWhenSaved = pVP6->WBoundsWhenSaved;
	strncpy (pCurView->DTMContourIntOrSlopeFactor,pVP6->DTMContourIntOrSlopeFactor,sizeof(pVP6->DTMContourIntOrSlopeFactor));
	for (i=0;i<MAX_VIEWPORT_FILES16;i++){
		pCurView->StretchImage[i] = pVP6->StretchImage[i];
	}
	for (i=0;i<3;i++){
		pCurView->hMaskAccelerator[i] = (HANDLE)pVP6->hMaskAccelerator[i];
	}
	pCurView->hDistanceLine = (HANDLE)pVP6->hDistanceLine;
	pCurView->MaskAreaRefno = pVP6->MaskAreaRefno;
	pCurView->NumMaskAreaParts = pVP6->NumMaskAreaParts;
	pCurView->Rotation = 0;
	pCurView->Scale = pVP6->Scale;
	pCurView->MidPointW = pVP6->MidPointW;
	pCurView->hProfileDataRectangles = (HANDLE)pVP6->hProfileDataRectangles;
	pCurView->nProfileDataRectangles = pVP6->nProfileDataRectangles;
	pCurView->ProfileDistUnits = pVP6->ProfileDistUnits;
	return;
}

void ConvertTB_V1_to_V2 (LPTAGBOX pTB,LPTAGBOX_V1 pTB16)
{
	int	i;
	_fmemset (pTB,0,sizeof(TAGBOX));  
	pTB->Version = 2;
	strncpy (pTB->Desc,pTB16->Desc,sizeof(pTB16->Desc));
	pTB->Flags = pTB16->Flags;
	pTB->rect = Rect16ToRect32(pTB16->rect);
	pTB->TAGPoint = pTB16->TAGPoint;
	pTB->TAGPointScr = POINTStoPOINT(pTB16->TAGPointScr);
	pTB->ConnectPoint = POINTStoPOINT(pTB16->ConnectPoint);
	pTB->RestorePoint = POINTStoPOINT(pTB16->RestorePoint);
	pTB->center = pTB16->center;
	pTB->ViewportID = pTB16->ViewportID;
	pTB->CoordStyle = pTB16->CoordStyle;
	pTB->BGstyle = pTB16->BGstyle;
	pTB->PLwidth = pTB16->PLwidth;
	pTB->PLstyle = pTB16->PLstyle;
	pTB->BorderStyle = pTB16->BorderStyle;
	pTB-> Margin = pTB16-> Margin;
	pTB->TXheight = pTB16->TXheight;
	pTB->Just = pTB16->Just;
	pTB->incx = pTB16->inc;
	pTB->BGcolor = pTB16->BGcolor;
	pTB->TXcolor = pTB16->TXcolor;
	pTB->BorderColor = pTB16->BorderColor;
	pTB->PointerColor = pTB16->PointerColor;
	pTB->before = (HANDLE)pTB16->before;
	pTB-> after = (HANDLE)pTB16-> after;
	pTB->bmWidthD = pTB16->bmWidthD;
	pTB->bmHeightD = pTB16->bmHeightD;
	pTB->bmWidth = pTB16->bmWidth;
	pTB-> bmHeight = pTB16-> bmHeight;
	pTB->Shadow = pTB16->Shadow;
	pTB->LogFont = ConvertLF16_to_32(&pTB16->LogFont);
	pTB->DataFileType = pTB16->DataFileType;
	pTB->hTAGDB = (HANDLE)pTB16->hTAGDB;
	pTB->SymNum = pTB16->SymNum;
	pTB->Factor = pTB16->Factor;
	pTB->Refno = pTB16->Refno;
	pTB->hReport = (HANDLE)pTB16->hReport;
	strncpy (pTB->Prefix,pTB16->Prefix,sizeof(pTB16->Prefix));
	strncpy (pTB->UDI,pTB16->UDI,sizeof(pTB16->UDI));
	strncpy (pTB->DataFile,pTB16->DataFile,sizeof(pTB16->DataFile));
	strncpy (pTB->SQL,pTB16->SQL,sizeof(pTB16->SQL));
	strncpy (pTB->text,pTB16->text,sizeof(pTB->text));
	pTB->BlowUpBounds = pTB16->BlowUpBounds;
	strncpy (pTB->PickMacroFile,pTB16->PickMacroFile,sizeof(pTB16->PickMacroFile));
	return;
}

LOGFONT ConvertLF16_to_32 (LPLOGFONT16 pLF16)
{
	LOGFONT LF;
	LPLOGFONT pLF=&LF;

	pLF->lfHeight = pLF16->lfHeight;
	pLF->lfWidth = pLF16->lfWidth;
	pLF->lfEscapement = pLF16->lfEscapement;
	pLF->lfOrientation = pLF16->lfOrientation;
	pLF->lfWeight = pLF16->lfWeight;
	pLF->lfItalic = pLF16->lfItalic;
	pLF->lfUnderline = pLF16->lfUnderline;
	pLF->lfStrikeOut = pLF16->lfStrikeOut;
	pLF->lfCharSet = pLF16->lfCharSet;
	pLF->lfOutPrecision = pLF16->lfOutPrecision;
	pLF->lfClipPrecision = pLF16->lfClipPrecision;
	pLF->lfQuality = pLF16->lfQuality;
	pLF->lfPitchAndFamily = pLF16->lfPitchAndFamily;
	strncpy (pLF->lfFaceName,pLF16->lfFaceName,sizeof(pLF16->lfFaceName));

	return LF;
}

FIELDINFO Field16toField32 (LPFIELDINFO16 pField)
{
	FIELDINFO	Field;

	memmove (&Field,pField,78);
	Field.hCurVal = 0;

	return Field;
}

FIELDINFO16 Field32toField16 (LPFIELDINFO pField)
{
	FIELDINFO16	Field;

	memmove (&Field,pField,78);
	Field.hCurVal = 0;

	return Field;
}

HOTSPOTDATA Hotspot16toHotspot32 (LPHOTSPOTDATA16 pHS16)
{
	HOTSPOTDATA	hsdata;
	LPHOTSPOTDATA pHS=&hsdata;

	pHS->SecondsRepresented = pHS16->SecondsRepresented;
	pHS->Radius = pHS16->Radius;
	pHS->DecayOpt = pHS16->DecayOpt;
	pHS->ColorOpt = pHS16->ColorOpt;
	pHS->PassThrough = pHS16->PassThrough;
	pHS->PromptForSavex = pHS16->PromptForSavex;
//	pHS->filler = pHS16->filler;
	pHS->GridWidth = pHS16->GridWidth;
	pHS->GridHeight = pHS16->GridHeight;
	pHS->MaskWidth = pHS16->MaskWidth;
	pHS->TotalIncidents = pHS16->TotalIncidents;
	pHS->MaxGridValue = pHS16->MaxGridValue;
	pHS->hMask = (HANDLE)pHS16->hMask;
	pHS->hGrid = (HANDLE)pHS16->hGrid;
	pHS->hTranBaseToHotSpot =(HANDLE) pHS16->hTranBaseToHotSpot;

	return hsdata;
}

NEWOBJECT NewObject16_to_NewObject (LPNEWOBJECT16 pNO16)
{
	NEWOBJECT NOBJ;
	
	NOBJ.Handle = (HANDLE)pNO16->Handle;
	NOBJ.Width = pNO16->Width;
	NOBJ.Type = pNO16->Type;
	NOBJ.Style = pNO16->Style;
	NOBJ.ProPen = pNO16->ProPen;
	NOBJ.R = pNO16->R;
	NOBJ.G = pNO16->G;
	NOBJ.B = pNO16->B;

	return NOBJ;
}

void ConvertThemeV1toV2 (LPTHEME pTheme,LPTHEME_V1 pTheme16)
{
	int	i;
	HANDLE	hSave=NULL;

	switch (pTheme16->ID)
	{
		case GF_STREET_TEXT_THEME:
		{
		    LPSTREETTEXTDATA16	pStreetData16=(LPSTREETTEXTDATA16)pTheme16->ClassBM;
			LPSTREETTEXTDATA	pSDSave;   
		    
		    hSave = GSSiGlobAlloc (1139,GMEM_MOVEABLE,sizeof(STREETTEXTDATA));
		    pSDSave = (LPSTREETTEXTDATA)GlobalLock(hSave);
			STREETTEXTDATA16ToSTREETTEXTDATA32 (pSDSave,pStreetData16);
		    GlobalUnlock (hSave);
		    break;
		}
		default:
			break;
	}
	pTheme->ID = pTheme16->ID;
	pTheme->handle = (HANDLE)pTheme16->handle;
	pTheme->Version = 2;
	pTheme->TargetViewport = pTheme16->TargetViewport;
	pTheme->DisplayViewport = pTheme16->DisplayViewport;
	pTheme->IsActive = pTheme16->IsActive;
	pTheme->WantDataPass = pTheme16->WantDataPass;
	pTheme->ComputeClassBoundaries = pTheme16->ComputeClassBoundaries;
	pTheme->DisplayScatterDiagram = pTheme16->DisplayScatterDiagram;
	pTheme->Recompute = pTheme16->Recompute;
	pTheme->ReScan = pTheme16->ReScan;
	pTheme->hThemeDB = (HANDLE)pTheme16->hThemeDB;
	pTheme->hScatterFile = (HANDLE)pTheme16->hScatterFile;
	strncpy (pTheme->DataFile,pTheme16->DataFile,sizeof(pTheme16->DataFile));
	strncpy (pTheme->SQL,pTheme16->SQL,sizeof(pTheme16->SQL));
	pTheme->DataFileType = pTheme16->DataFileType;
	pTheme->DataType = pTheme16->DataType;
	pTheme->Field = Field16toField32 (&pTheme16->Field);
	for (i=0;i<MAX_THEME_CLASSES;i++){
		pTheme->ClassSymbol[i] = pTheme16->ClassSymbol[i];
	}
	strncpy (pTheme->SymSizeC,pTheme16->SymSizeC,sizeof(pTheme16->SymSizeC));
	for (i=0;i<MAX_THEME_CLASSES;i++){
		pTheme->ClassIsSelected[i] = pTheme16->ClassIsSelected[i];
	}
	pTheme->Statement_dummy = 0;//pTheme16->Statement;
	strncpy (pTheme->ScatterFile,pTheme16->ScatterFile,sizeof(pTheme16->ScatterFile));
	pTheme->NumClass = pTheme16->NumClass;
	pTheme->NumDesiredClass = pTheme16->NumDesiredClass;
	pTheme->ClassType = pTheme16->ClassType;
	pTheme->ValConv = pTheme16->ValConv;
	pTheme->MissOpt = pTheme16->MissOpt;
	pTheme->MarkInvalid = pTheme16->MarkInvalid;
	pTheme->XLimit = pTheme16->XLimit;
	pTheme->YLimit = pTheme16->YLimit;
	pTheme->RoundTo = pTheme16->RoundTo;
	pTheme->NumVals = pTheme16->NumVals;
	pTheme->Xmin = pTheme16->Xmin;
	pTheme->Ymin = pTheme16->Ymin;
	pTheme->Xmax = pTheme16->Xmax;
	pTheme->Ymax = pTheme16->Ymax;
	for (i=0;i<MAX_THEME_CLASSES;i++){
		pTheme->ClassMin[i] = pTheme16->ClassMin[i];
	}
	for (i=0;i<MAX_THEME_CLASSES;i++){
		pTheme->ClassMax[i] = pTheme16->ClassMax[i];
	}
	for (i=0;i<MAX_THEME_CLASSES;i++){
		pTheme->ClassColor[i] = pTheme16->ClassColor[i];
	}
	for (i=0;i<MAX_THEME_CLASSES;i++){
		pTheme->ClassPen[i] = (HPEN)pTheme16->ClassPen[i];
	}
	for (i=0;i<MAX_THEME_CLASSES;i++){
		pTheme->ClassBrush[i] = (HBRUSH)pTheme16->ClassBrush[i];
	}
	pTheme->NoDataBrush = (HBRUSH)pTheme16->NoDataBrush;
	pTheme->InvalidDataBrush = (HBRUSH)pTheme16->InvalidDataBrush;
	for (i=0;i<MAX_THEME_CLASSES;i++){
		pTheme->ClassCount[i] = pTheme16->ClassCount[i];
	}
	for (i=0;i<MAX_THEME_CLASSES;i++){
		pTheme->ClassPnt[i] = pTheme16->ClassPnt[i];
	}
	pTheme->BGColor = pTheme16->BGColor;
	pTheme-> ScatterColor = pTheme16-> ScatterColor;
	pTheme-> ScatterBoxBG = pTheme16-> ScatterBoxBG;
	pTheme-> TitleBoxBG = pTheme16-> TitleBoxBG;
	pTheme->Rect = Rect16ToRect32(pTheme16->Rect);
	pTheme-> TitleBox = Rect16ToRect32(pTheme16-> TitleBox);
	pTheme-> ScatterBox = Rect16ToRect32(pTheme16-> ScatterBox);
	pTheme-> ColorsBox = Rect16ToRect32(pTheme16-> ColorsBox);
	pTheme-> RangesBox = Rect16ToRect32(pTheme16-> RangesBox);
	pTheme-> InfoBox = Rect16ToRect32(pTheme16-> InfoBox);
	for (i=0;i<MAX_THEME_CLASSES;i++){
		pTheme->ClassClrBox[i] = Rect16ToRect32(pTheme16->ClassClrBox[i]);
	}
	pTheme->Margin = pTheme16->Margin;
	pTheme-> ScatterWidth = pTheme16-> ScatterWidth;
	pTheme-> ColorsWidth = pTheme16-> ColorsWidth;
	pTheme-> InnerMargin = pTheme16-> InnerMargin;
	pTheme-> TitleHeight = pTheme16-> TitleHeight;
	for (i=0;i<MAX_THEME_CLASSES;i++){
		strncpy (pTheme->ClassBM[i],pTheme16->ClassBM[i],sizeof(pTheme16->ClassBM[i]));
	}
	strncpy (pTheme->Title,pTheme16->Title,sizeof(pTheme16->Title));
	strncpy (pTheme->Contents,pTheme16->Contents,sizeof(pTheme16->Contents));
	pTheme->SymNum = pTheme16->SymNum;
	pTheme->TitleFont = ConvertLF16_to_32(&pTheme16->TitleFont);
	pTheme->ClassFont1 = ConvertLF16_to_32(&pTheme16->ClassFont1);
	pTheme->ClassFont2 = ConvertLF16_to_32(&pTheme16->ClassFont2);
	pTheme->Xmove = pTheme16->Xmove;
	pTheme-> Ymove = pTheme16-> Ymove;
	pTheme->ZeroIsMissing = pTheme16->ZeroIsMissing;
	pTheme->AddCommas = pTheme16->AddCommas;
	pTheme->ShowValue = pTheme16->ShowValue;
	pTheme->VPDisplayed = pTheme16->VPDisplayed;
	pTheme->DisplayPCT = pTheme16->DisplayPCT;
	strncpy (pTheme->RefValChar,pTheme16->RefValChar,sizeof(pTheme16->RefValChar));
	pTheme->RefValDbl = pTheme16->RefValDbl;
	pTheme->FieldFun = pTheme16->FieldFun;
	pTheme->ValueLen = pTheme16->ValueLen;
	pTheme->FieldCorrection = pTheme16->FieldCorrection;
	pTheme->IBBGColor = pTheme16->IBBGColor;
	pTheme-> TitleTextColor = pTheme16-> TitleTextColor;
	for (i=0;i<2;i++){
		pTheme-> IBTextColor[i] = pTheme16-> IBTextColor[i];
	}
	pTheme->RefIsPCT = pTheme16->RefIsPCT;
	strncpy (pTheme->Value,pTheme16->Value,sizeof(pTheme16->Value));
	pTheme->hVisList = (HANDLE)pTheme16->hVisList;
	pTheme->DispersePoints = pTheme16->DispersePoints;
	pTheme->hDisperseFileName = (HANDLE)pTheme16->hDisperseFileName;
	pTheme->hDisperseFile = (HANDLE)pTheme16->hDisperseFile;
	pTheme->MaxDispersion = pTheme16->MaxDispersion;
	pTheme->hHighlightFileName = (HANDLE)pTheme16->hHighlightFileName;
	pTheme->hHighlightFile = (HANDLE)pTheme16->hHighlightFile;
	pTheme->AccumPointSymbolOpt = pTheme16->AccumPointSymbolOpt;
	pTheme->AccumPointSizeOpt = pTheme16->AccumPointSizeOpt;
	pTheme->AccumPointBaseSize = pTheme16->AccumPointBaseSize;
	pTheme->AccumPointLink = pTheme16->AccumPointLink;
	pTheme->AccumPointText = pTheme16->AccumPointText;
	pTheme->ZeroBased = pTheme16->ZeroBased;
	pTheme->PCTByArea = pTheme16->PCTByArea;
	pTheme->NumNonMask = pTheme16->NumNonMask;
	pTheme->PCTDisplayCycle = pTheme16->PCTDisplayCycle;
	pTheme->LayerID = pTheme16->LayerID;
	pTheme->InvertLegend = pTheme16->InvertLegend;
	pTheme->NumCols = pTheme16->NumCols;
	pTheme->HiPrecis = pTheme16->HiPrecis;
	pTheme->NumMidpointnotused = pTheme16->NumMidpointnotused;
	pTheme->AllValueClass = pTheme16->AllValueClass;
	pTheme->ClearIfNoCount = pTheme16->ClearIfNoCount;
	pTheme->DisplayPointsOnly = pTheme16->DisplayPointsOnly;
	pTheme->FlipLegend = pTheme16->FlipLegend;
	pTheme->FactorLegend = pTheme16->FactorLegend;
	pTheme->DisplayCount = pTheme16->DisplayCount;
	pTheme->ShowOnlySelectedClasses = pTheme16->ShowOnlySelectedClasses;
	pTheme->HideNullClasses = pTheme16->HideNullClasses;
	pTheme->NotSetColor = pTheme16->NotSetColor;
	pTheme->AppendCount = pTheme16->AppendCount;
	pTheme->CompressNullClasses = pTheme16->CompressNullClasses;
	pTheme->DisplayDistance = pTheme16->DisplayDistance;
	pTheme->AutoClassDef = pTheme16->AutoClassDef;
	pTheme->UseFirstSymbol = pTheme16->UseFirstSymbol;
	pTheme->FillRow = pTheme16->FillRow;
	pTheme->CenterText = pTheme16->CenterText;
	pTheme->FlatEndOffsetLine = pTheme16->Unusedbit;
	strncpy (pTheme->ClassDefDB,pTheme16->ClassDefDB,sizeof(pTheme16->ClassDefDB));
	strncpy (pTheme->ClassDefSQL,pTheme16->ClassDefSQL,sizeof(pTheme16->ClassDefSQL));
	strncpy (pTheme->ClassDefKeyField,pTheme16->ClassDefKeyField,sizeof(pTheme16->ClassDefKeyField));
	strncpy (pTheme->ClassDefSymField,pTheme16->ClassDefSymField,sizeof(pTheme16->ClassDefSymField));
	strncpy (pTheme->ClassDefTitleField,pTheme16->ClassDefTitleField,sizeof(pTheme16->ClassDefTitleField));
	strncpy (pTheme->ClassDefValDB,pTheme16->ClassDefValDB,sizeof(pTheme16->ClassDefValDB));
	strncpy (pTheme->ClassDefValSQL,pTheme16->ClassDefValSQL,sizeof(pTheme16->ClassDefValSQL));
	strncpy (pTheme->ClassDefValField,pTheme16->ClassDefValField,sizeof(pTheme16->ClassDefValField));
	for (i=0;i<4;i++){
		strncpy (pTheme->SymbolFont[i],pTheme16->SymbolFont[i],sizeof(pTheme16->SymbolFont[i]));
	}
	for (i=0;i<MAX_THEME_CLASSES;i++){
		pTheme->ClassStatus[i] = pTheme16->ClassStatus[i];
	}
	pTheme->HotSpotData = Hotspot16toHotspot32 (&pTheme16->HotSpotData);
	for (i=0;i<MAX_THEME_CLASSES;i++){
		pTheme->ClassFactor[i] = pTheme16->ClassFactor[i];
	}
	strncpy (pTheme->HotSpotCompareTo,pTheme16->HotSpotCompareTo,sizeof(pTheme16->HotSpotCompareTo));
	strncpy (pTheme->HotSpotSaveTo,pTheme16->HotSpotSaveTo,sizeof(pTheme16->HotSpotSaveTo));
	pTheme->CompareHotSpotFactor = pTheme16->CompareHotSpotFactor;
	strncpy (pTheme->IconLibrary,pTheme16->IconLibrary,sizeof(pTheme16->IconLibrary));
	pTheme->ActualXMargin = pTheme16->ActualXMargin;
	pTheme->ActualYMargin = pTheme16->ActualYMargin;
	pTheme->ShowValueFont = ConvertLF16_to_32(&pTheme16->ShowValueFont);
	pTheme->ShowValueTextColor = pTheme16->ShowValueTextColor;
	pTheme->Config = pTheme16->Config;
	pTheme->UseHalfTone = pTheme16->UseHalfTone;
	for (i=0;i<2;i++){
		pTheme->HaveVP[i] = pTheme16->HaveVP[i];
	}
	pTheme->NextValueColor = pTheme16->NextValueColor;
	pTheme->ValueColor = pTheme16->ValueColor;
	pTheme->NumMissing = pTheme16->NumMissing;
	pTheme->NumInvalid = pTheme16->NumInvalid;
	pTheme->SkipInvalid = pTheme16->SkipInvalid;
	pTheme->MultiValOption = pTheme16->GetFirstClass;
	pTheme->ColorScheme = pTheme16->ColorScheme;
	pTheme->hHotSpotBitmap = (HANDLE)pTheme16->hHotSpotBitmap;
	pTheme->HotSpotBounds = pTheme16->HotSpotBounds;
	pTheme->ComputeStoredCounts = pTheme16->ComputeStoredCounts;
	pTheme->UseStoredCounts = pTheme16->UseStoredCounts;
	pTheme->UseCheckmark = pTheme16->UseCheckmark;
	pTheme->DelayTextDisplay = pTheme16->DelayTextDisplay;
	pTheme->DistanceBetweenProfilePoints = pTheme16->DistanceBetweenProfilePoints;
	pTheme->CrossSectionSpacing = pTheme16->CrossSectionSpacing;
	for (i=0;i<2;i++){
		pTheme->CrossSectionWidth[i] = pTheme16->CrossSectionWidth[i];
	}
	pTheme->RadiusPoint = POINTStoPOINT(pTheme16->RadiusPoint);
	pTheme->Radius = pTheme16->Radius;
	pTheme->DynSegMaxFixedIncrement = pTheme16->DynSegMaxFixedIncrement;
	pTheme->DynSegFixedIncrement = pTheme16->DynSegFixedIncrement;
	pTheme->ShowValAZ = pTheme16->ShowValAZ;
	pTheme->ProfileUnits = pTheme16->ProfileUnits;
	strncpy (pTheme->Contents2,pTheme16->Contents2,sizeof(pTheme16->Contents2));
	pTheme->SymNum2 = pTheme16->SymNum2;
	pTheme->hVisList2 = (HANDLE)pTheme16->hVisList2;
	pTheme->hAreas = (HANDLE)pTheme16->hAreas;
	pTheme->NumAreas = pTheme16->NumAreas;
	pTheme->ACCDisperse = pTheme16->ACCDisperse;
	pTheme->Pass = pTheme16->Pass;
	pTheme->PointInAreaPen = (HPEN)pTheme16->PointInAreaPen;
	pTheme->PointInAreaBrush = (HBRUSH)pTheme16->PointInAreaBrush;
	pTheme->PointInAreaColor = pTheme16->PointInAreaColor;
	pTheme->FidAreas = pTheme16->FidAreas;
	pTheme->PointInAreaPointThemeVPID = pTheme16->PointInAreaPointThemeVPID;
	pTheme->NumMidpoint = pTheme16->NumMidpoint;
	strncpy (pTheme->ShowValMacro,pTheme16->ShowValMacro,sizeof(pTheme16->ShowValMacro));
	pTheme->FidDelayedText = pTheme16->FidDelayedText;
	pTheme->hDelayedFont = (HFONT)pTheme16->hDelayedFont;
	strncpy (pTheme->filler,pTheme16->filler,sizeof(pTheme16->filler));
	switch (pTheme16->ID)
	{
		case GF_STREET_TEXT_THEME:
		{
			LPSTREETTEXTDATA	pSDSave=GlobalLock (hSave);   
		    
		    memmove (&pTheme->ClassBM,pSDSave,sizeof(STREETTEXTDATA));
		    GSSiGlobUlFree (&hSave);
		    break;
		}
		default:
			break;
	}
	return;
}

void ConvertCD_V103_to_V104 (LPCOORDINATEDISPLAY pCD,LPCOORDINATEDISPLAY_V103 pCD16)
{
	int	i;
	_fmemset (pCD,0,sizeof(COORDINATEDISPLAY));  

	pCD->Version = 2;
	pCD->ID = pCD16->ID;
	pCD->handle = (HANDLE)pCD16->handle;
	pCD->Version = 104;
	pCD->TargetViewport = pCD16->TargetViewport;
	pCD->DisplayViewport = pCD16->DisplayViewport;
	for (i=0;i<4;i++){
		pCD->DisplayLine[i] = pCD16->DisplayLine[i];
	}
	strncpy (pCD->AltCVTFile,pCD16->AltCVTFile,sizeof(pCD16->AltCVTFile));
	for (i=0;i<4;i++){
		strncpy (pCD->LineID[i],pCD16->LineID[i],sizeof(pCD16->LineID[i]));
	}
	for (i=0;i<4;i++){
		pCD->Units[i] = pCD16->Units[i];
	}
	for (i=0;i<4;i++){
		pCD->Precision[i] = pCD16->Precision[i];
	}
	for (i=0;i<4;i++){
		pCD->Commas[i] = pCD16->Commas[i];
	}
	pCD->Font = ConvertLF16_to_32(&pCD16->Font);
	pCD->FontColor = pCD16->FontColor;
	pCD->Refresh = pCD16->Refresh;
	strncpy (pCD->PrintText,pCD16->PrintText,sizeof(pCD16->PrintText));
	strncpy (pCD->ElevSurface,pCD16->ElevSurface,sizeof(pCD16->ElevSurface));
	pCD->hSurf = (HANDLE)pCD16->hSurf;
	pCD->ElevUnits = pCD16->ElevUnits;
	pCD->ElevPrecision = pCD16->ElevPrecision;
	pCD->ElevCommas = pCD16->ElevCommas;
	pCD->DisplayElevation = pCD16->DisplayElevation;
	strncpy (pCD->ElevationID,pCD16->ElevationID,sizeof(pCD16->ElevationID));
	strncpy (pCD->Space,pCD16->Space,sizeof(pCD->Space));
	return;
}

BOUNDSDISPLAY BD16ToBD32 (LPVOID p16v)
{
	LPBOUNDSDISPLAY16 p16=(LPBOUNDSDISPLAY16)p16v;
	BOUNDSDISPLAY bd;
	LPBOUNDSDISPLAY p32=&bd;
	int	i;

	p32->ID = p16->ID;
	p32->Handle = (HANDLE)p16->Handle;
	p32->DisplayVP = p16->DisplayVP;
	p32-> TargetVP = p16-> TargetVP;
	p32->Type = p16->Type;
	p32->SavedRect = Rect16ToRect32(p16->SavedRect);
	p32->SavedScreen = (HANDLE)p16->SavedScreen;
	for (i=0;i<7;i++){
		p32->BoxPoints[i] = POINTStoPOINT(p16->BoxPoints[i]);
	}
	p32->CurAreaRef = p16->CurAreaRef;
	strncpy (p32->CurAreaRefGlobal,p16->CurAreaRefGlobal,sizeof(p16->CurAreaRefGlobal));
	p32->IgnoreClear = p16->IgnoreClear;
	p32->VPBoundsOpt = p16->VPBoundsOpt;
	strncpy (p32->BoundsGlobal,p16->BoundsGlobal,sizeof(p16->BoundsGlobal));
	return bd;
}

GWDHEADER16 GWDHEADER32toGWDHEADER16 (LPGWDHEADER p16)
{
	GWDHEADER16 gwdhead;
	LPGWDHEADER16	p32=&gwdhead;
	int i,j;

	p32->Version = p16->Version;
	p32->Compressed = p16->Compressed;
	p32->NonUniqueSortedBinary = p16->NonUniqueSortedBinary;
	p32->Unused = 0;
	p32->NumFields = p16->NumFields;
	p32->NumIndex = p16->NumIndex;
	p32->Fid = p16->Fid;
	p32->Reclen = p16->Reclen;
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p32->BTHandle[i] = 0;
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p32->NumIndexFields[i] = p16->NumIndexFields[i];
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		for (j=0;j<MAX_GMD_INDEX_FIELDS;j++){
			p32->IndexFields[i][j] = p16->IndexFields[i][j];
		}
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p32->lKeys[i] = p16->lKeys[i];
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p32->hKeys[i] = (HANDLE16)p16->hKeys[i];
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p32->pKeys[i] = p16->pKeys[i];
	}
	p32->TimeStamp = p16->TimeStamp;
	p32->hFldInfo = (HANDLE16)p16->hFldInfo;
	p32->pFldInfo = p16->pFldInfo;
	strncpy (p32->GWDData,p16->GWDData,sizeof(p16->GWDData));

	return gwdhead;
}



GWDHEADER GWDHEADER32toGWDHEADER (LPGWDHEADER32 p32)
{
	GWDHEADER gwdhead;
	LPGWDHEADER	p=&gwdhead;
	int i,j;

	memset (p,0,sizeof(GWDHEADER));
	p->Version = p32->Version;
	p->Compressed = p32->Compressed;
	p->NonUniqueSortedBinary = p32->NonUniqueSortedBinary;
	p->StoredAs32 = p32->Unused;
	p->NumFields = p32->NumFields;
	p->NumIndex = p32->NumIndex;
	p->Fid = p32->Fid;
	p->Reclen = p32->Reclen;
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p->BTHandle[i] = (HANDLE)p32->BTHandle[i];
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p->NumIndexFields[i] = p32->NumIndexFields[i];
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		for (j=0;j<MAX_GMD_INDEX_FIELDS;j++){
			p->IndexFields[i][j] = p32->IndexFields[i][j];
		}
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p->lKeys[i] = p32->lKeys[i];
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p->hKeys[i] = (HANDLE)p32->hKeys[i];
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p->pKeys[i] = p32->pKeys[i];
	}
	p->TimeStamp = p32->TimeStamp;
	p->hFldInfo = (HANDLE)p32->hFldInfo;
	p->pFldInfo = p32->pFldInfo;
	strncpy (p->GWDData,p32->GWDData,sizeof(p32->GWDData));

	return gwdhead;
}

GWDHEADER32 GWDHEADER16toGWDHEADER32 (LPGWDHEADER16 p16)
{
	GWDHEADER32 gwdhead;
	LPGWDHEADER32	p32=&gwdhead;
	int i,j;

	p32->Version = p16->Version;
	p32->Compressed = p16->Compressed;
	p32->NonUniqueSortedBinary = p16->NonUniqueSortedBinary;
	p32->Unused = p16->Unused;
	p32->NumFields = p16->NumFields;
	p32->NumIndex = p16->NumIndex;
	p32->Fid = p16->Fid;
	p32->Reclen = p16->Reclen;
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p32->BTHandle[i] = (HANDLE)p16->BTHandle[i];
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p32->NumIndexFields[i] = p16->NumIndexFields[i];
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		for (j=0;j<MAX_GMD_INDEX_FIELDS;j++){
			p32->IndexFields[i][j] = p16->IndexFields[i][j];
		}
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p32->lKeys[i] = p16->lKeys[i];
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p32->hKeys[i] = (HANDLE)p16->hKeys[i];
	}
	for (i=0;i<MAX_GMD_INDEXES16;i++){
		p32->pKeys[i] = p16->pKeys[i];
	}
	p32->TimeStamp = p16->TimeStamp;
	p32->hFldInfo = (HANDLE)p16->hFldInfo;
	p32->pFldInfo = p16->pFldInfo;
	strncpy (p32->GWDData,p16->GWDData,sizeof(p16->GWDData));

	return gwdhead;
}

void STREETTEXTDATA16ToSTREETTEXTDATA32 (LPSTREETTEXTDATA p32,LPSTREETTEXTDATA16 p16)
{
	int	i;
	_fmemset (p32,0,sizeof(STREETTEXTDATA));  

	p32->NameSource = p16->NameSource;
	p32->LayerID = p16->LayerID;
	p32->MinTextSize = p16->MinTextSize;
	p32->MaxTextSize = p16->MaxTextSize;
	strncpy (p32->Macro,p16->Macro,sizeof(p16->Macro));
	p32->ShowCities = p16->ShowCities;
	p32->NameLength = p16->NameLength;
	p32->hNameFile1 = (HANDLE)p16->hNameFile1;
	p32-> hNameFile2 = (HANDLE)p16-> hNameFile2;
	strncpy (p32->NameFile1,p16->NameFile1,sizeof(p16->NameFile1));
	strncpy (p32->NameFile2,p16->NameFile2,sizeof(p16->NameFile2));
	p32->Italic = p16->Italic;
	p32-> Shadow = p16-> Shadow;
	p32->BackgroundOpt = p16->BackgroundOpt;
	p32->TextColor = p16->TextColor;
	p32-> ShadowColor = p16-> ShadowColor;
	p32->ShowAllElements = p16->ShowAllElements;
	p32->SavePAE = p16->SavePAE;
	p32->VJust = p16->VJust;
	p32->HorizontalText = p16->HorizontalText;
	p32->PrimeNameOnly = p16->PrimeNameOnly;
	strncpy (p32->Expression,p16->Expression,sizeof(p16->Expression));
	strncpy (p32->ListFile,p16->ListFile,sizeof(p16->ListFile));
	strncpy (p32->ListData,p16->ListData,sizeof(p16->ListData));
	p32->ListLen = p16->ListLen;
	p32->SortOnData = p16->SortOnData;
	p32->hListDB = (HANDLE)p16->hListDB;
	p32->MinSize = p16->MinSize;
	p32->StreetTextFont = ConvertLF16_to_32(&p16->StreetTextFont);
	p32->TextSize = p16->TextSize;
	p32->IgnoreShields = p16->IgnoreShields;
	p32->AllowOverlap = p16->AllowOverlap;
	p32->ScaleText = p16->ScaleText;
	p32->UseFont = p16->UseFont;
	p32->AllowHollow = p16->AllowHollow;
	p32->Dummy = p16->Dummy;
	strncpy (p32->VisMacro,p16->VisMacro,sizeof(p16->VisMacro));
	strncpy (p32->ColorMacro,p16->ColorMacro,sizeof(p16->ColorMacro));
	strncpy (p32->DisplayNameMacro,p16->DisplayNameMacro,sizeof(p16->DisplayNameMacro));
	return;
}

void ComboFile16ToComboFile32 (LPCOMBOFILE p32,LPCOMBOFILE16 p16)
{
	int	i;
	_fmemset (p32,0,sizeof(COMBOFILE));  

	p32->Version = p16->Version;
	p32->NumFiles = p16->NumFiles;
	p32->NumFields = p16->NumFields;
	p32->TotFileLen = p16->TotFileLen;
	for (i=0;i<5;i++){
		p32->hSQL[i] = (HANDLE)p16->hSQL[i];
	}
	for (i=0;i<5;i++){
		strncpy (p32->FileNames[i],p16->FileNames[i],sizeof(p16->FileNames[i]));
	}
	return;
}

void ComboFile32ToComboFile16 (LPCOMBOFILE16 p32,LPCOMBOFILE p16)
{
	int	i;
	_fmemset (p32,0,sizeof(COMBOFILE16));  

	p32->Version = p16->Version;
	p32->NumFiles = p16->NumFiles;
	p32->NumFields = p16->NumFields;
	p32->TotFileLen = p16->TotFileLen;
//	for (i=0;i<5;i++){
//		p32->hSQL[i] = (HANDLE)p16->hSQL[i];
//	}
	for (i=0;i<5;i++){
		strncpy (p32->FileNames[i],p16->FileNames[i],sizeof(p32->FileNames[i]));
	}
	return;
}

int	ReadTAGBOX (HFILE Fid,int l,LPTAGBOX pTAGBOX)
{
	int	rtn;

	if (l == sizeof(TAGBOX_V1))
	{
		TAGBOX_V1	TB_V1;
	
		rtn = GSSilread (Fid,&TB_V1,sizeof(TAGBOX_V1));
		ConvertTB_V1_to_V2 (pTAGBOX,&TB_V1);
	}
	else
		rtn = BigRead (Fid,(HPSTR)pTAGBOX,sizeof(TAGBOX)); 
	return rtn;
}

HANDLE OpenComboDatabase (LPSTR Name,LPSTR SQL)
#if ENABLETRACE
{GSSiEnterProg (614);
#endif
{   
	LPCOMBOHEADER		pComboHeader;
    LPCOMBOFILE16  		pComboFile16; 
    LPCOMBOFILE  		pComboFile; 
    LPCOMBOFIELDINFO    pComboField;
    LPFIELDINFO         pField,pFileField;
    LPWHEREINDEX        pWhereIndex;
    LPCFIELDINDEX       pCFieldIndex;  
    LPOPENFILEDATA		FilePtr;
    LPOPENSQLDATA		SQLPtr; 
    LPSTR       pWhere, pWhere2, pCField, pSQL=SQL;
    HANDLE      hDB; 
	OFSTRUCTGM    OFStruct;
    HFILE       FidCF; 
    short		i;     
    char		Blanks[]="";
    long		size=0, len;
	LPSTR		str;
	HANDLE		hStr=0;
	COMBOFILE16	ComboFile16;
	LPINT		pFieldIndex;
	LPSTR		pFieldDefs;
	int			loc;
	LPSTR		pPar,pLoc;
	HANDLE		hFieldDefs=0,hFieldIndex=0;

	FidCF = GSSiOpenFile (Name,&OFStruct,OF_READ);
	if (FidCF == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (614);
#endif
		return 0;
}
	hStr = GSSiGlobAlloc (1517,GMEM_MOVEABLE,4096);
	str = GlobalLock (hStr);
	hDB = GSSiGlobAlloc ( 249,GHND,sizeof(COMBOHEADER));
	pComboHeader = (LPCOMBOHEADER)GlobalLock (hDB);
	pComboHeader->hComboFile = GSSiGlobAlloc ( 250,GHND,USHRT_MAX);
	pComboFile = (LPCOMBOFILE)GlobalLock (pComboHeader->hComboFile);
	pComboHeader->hWhere = GSSiGlobAlloc ( 252,GHND,USHRT_MAX);
	pWhereIndex = (LPWHEREINDEX) GlobalLock (pComboHeader->hWhere); 
	pWhere2 = (LPSTR)pWhereIndex;
	pWhere2 += sizeof(WHEREINDEX);
	BigRead (FidCF,str,22);
	GSSillseek (FidCF,0,0);
	if (!strncmp (str,"COMBO FILE DEFINITION",21))
	{
		pComboFile->Version = 2;
		fgetstring (str,4090,FidCF);
		while (fgetstring (str,4090,FidCF))
		{
			LPSTR pStart = FirstNonBlank (str);

			if (!strcmp (str,"FIELD DEFINITIONS"))
				break;
			if (pStart)
			{
				if ((pWhere = strchr (pStart,';')))
					*pWhere++ = 0;
				strcpy (pComboFile->FileNames[pComboFile->NumFiles],pStart);
				if (pWhere)
				{
					int	l=strlen (pWhere);

					strcpy (pWhere2,pWhere);
					pWhere2 += (l+1);
					pWhereIndex->offset[pComboFile->NumFiles] = pWhereIndex->Len;
					pWhereIndex->Num++;
					pWhereIndex->Len += (l+1);
				}
				pComboFile->NumFiles++;
			}
		}
	    pField = pComboFile->FldInfo; 
		hFieldDefs = GSSiGlobAlloc ( 253,GHND,USHRT_MAX);
		pFieldDefs = GlobalLock (hFieldDefs);
		hFieldIndex = GSSiGlobAlloc (254,GHND,SHRT_MAX);
		pFieldIndex = GlobalLock (hFieldIndex);
		loc = 0;
		while (fgetstring (str,4090,FidCF))
		{
			LPSTR pStart = FirstNonBlank (str);

			if (pStart)
			{
				LPSTR	pEQ = strchr (pStart,'=');

				if (pEQ)
				{
					GWFLDINFO	FieldInfo;

					*pEQ++ = 0;
					if ((pPar = strchr (pStart,'(')))
					{
						*pPar++ = 0;
						memset (pField,0,sizeof(FIELDINFO));
						strcpy (pField->name,pStart);
						if (GetFieldTypeAndLenFromChar (pPar,&FieldInfo,0)) 
						{
							pField->type = FieldInfo.Type;
							pField->length = FieldInfo.Len;
							pField++;
							strcpy (&pFieldDefs[loc],pEQ);
							pFieldIndex[pComboFile->NumFields++] = loc;
							loc += strlen (pEQ) + 1;
						}
					}
				}
			}
		}
		pComboHeader->hComboFields = GSSiGlobAlloc ( 251,GHND,pComboFile->NumFields*sizeof(COMBOFIELDINFO));
		pComboHeader->hComputedFields = GSSiGlobAlloc (1518,GMEM_MOVEABLE,pComboFile->NumFields*sizeof(int)+loc);
		pLoc = GlobalLock (pComboHeader->hComputedFields);
		memmove (pLoc,pFieldIndex,pComboFile->NumFields*sizeof(int));
		pLoc += pComboFile->NumFields*sizeof(int);
		memmove (pLoc,pFieldDefs,loc);
		GlobalUnlock (pComboHeader->hComputedFields);
		pComboField = (LPCOMBOFIELDINFO)GlobalLock (pComboHeader->hComboFields); 
		for (i=0;i<pComboFile->NumFields;i++)
		{
			pComboField->fromfile = -1;
			pComboField++->fromfileindex = i;
		}
		GlobalUnlock (pComboHeader->hComboFields);
		GSSiGlobUlFree (&hFieldDefs);
		GSSiGlobUlFree (&hFieldIndex);
	}
	else
	{
		pComboFile->Version = 1;
		BigRead (FidCF,(HPSTR)&ComboFile16,sizeof(COMBOFILE16)-sizeof(FIELDINFO16));
		ComboFile16ToComboFile32 (pComboFile,&ComboFile16);
		for (i=0;i<pComboFile->NumFields;i++)
		{
			FIELDINFO16 FieldInfo16;

			BigRead (FidCF,(HPSTR)&FieldInfo16,sizeof(FIELDINFO16));
			pComboFile->FldInfo[i] = Field16toField32 (&FieldInfo16);
		}
		pComboHeader->hComboFields = GSSiGlobAlloc ( 251,GHND,pComboFile->NumFields*sizeof(COMBOFIELDINFO));
		pComboField = (LPCOMBOFIELDINFO)GlobalLock (pComboHeader->hComboFields); 
		BigRead (FidCF,(HPSTR)pComboField,pComboFile->NumFields*sizeof(COMBOFIELDINFO)); //pComboField[19]
		GlobalUnlock (pComboHeader->hComboFields);
		BigRead (FidCF,(HPSTR)pWhereIndex,sizeof(WHEREINDEX));
		len = pWhereIndex->Len;
		pWhereIndex++;
		BigRead (FidCF,(HPSTR)pWhereIndex,(UINT)len);
		pComboHeader->hComputedFields = GSSiGlobAlloc ( 253,GHND,USHRT_MAX);
		pCFieldIndex = (LPCFIELDINDEX)GlobalLock(pComboHeader->hComputedFields);
		BigRead (FidCF,(HPSTR)pCFieldIndex,sizeof(CFIELDINDEX));
		len = pCFieldIndex->Len;
		pCFieldIndex++;
		BigRead (FidCF,(HPSTR)pCFieldIndex,(UINT)len);
		GlobalUnlock (pComboHeader->hComputedFields);
		pComboHeader->hComputedFields = GSSiGlobalReAlloc (0,pComboHeader->hComputedFields,sizeof(CFIELDINDEX)+len,GMEM_MOVEABLE);
	}
	len = pWhereIndex->Len;
	GlobalUnlock (pComboHeader->hWhere);
	pComboHeader->hWhere = GSSiGlobalReAlloc (0,pComboHeader->hWhere,sizeof(WHEREINDEX)+len,GMEM_MOVEABLE);
	GSSiClose2 (&FidCF);
	for (i=0;i<pComboFile->NumFiles;i++)
		pComboFile->hSQL[i]=0; 
	for (i=0;i<pComboFile->NumFiles;i++)
	{   
		char	Name[256];
		
		if (strchr (pComboFile->FileNames[i],'='))
			strcpy (Name,pComboFile->FileNames[i]);
		else
			sprintf (Name,"CF%i=%s",i,pComboFile->FileNames[i]);
		if (!OpenDataFile (Name,pSQL,BT_READ,&pComboFile->hSQL[i]))
		{   
			GlobalUnlock (pComboHeader->hComboFile);
			GlobalUnlock (hDB);
			CloseComboDatabase (hDB);
			GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (614);
#endif
			return 0;
}
		}
		pSQL = (LPSTR)&Blanks;
	}
    pField = pComboFile->FldInfo; 
    pComboField = (LPCOMBOFIELDINFO)GlobalLock (pComboHeader->hComboFields);
    for (i=0;i<pComboFile->NumFields;i++,pField++,pComboField++)
    {   
        if (pComboField->fromfile > -1)
        {
            SQLPtr = (LPOPENSQLDATA)GlobalLock(pComboFile->hSQL[pComboField->fromfile]);
            FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);
            pFileField = &FilePtr->FldInfo+pComboField->fromfileindex; 
            pField->type = pFileField->type;
            pField->length = pFileField->length;
            GlobalUnlock (SQLPtr->OFHandle);
            GlobalUnlock(pComboFile->hSQL[pComboField->fromfile]); 
        }
    }
	size = sizeof(COMBOFILE)+pComboFile->NumFields*sizeof(FIELDINFO);
	GlobalUnlock (pComboHeader->hComboFields); 
	GlobalUnlock (pComboHeader->hComboFile); 
	pComboHeader->hComboFile = GSSiGlobalReAlloc (0,pComboHeader->hComboFile,size,GMEM_MOVEABLE);
	GlobalUnlock (hDB);
	GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (614);
#endif
	return hDB;
}
#if ENABLETRACE
}
#endif
}

int ReadVISLIST16 (HFILE Fid,LPVISLIST CurVis)
{
	int	nread;

	HANDLE	hVL16 = GSSiGlobAlloc (0,GMEM_MOVEABLE,sizeof(VISLIST16));
	LPVISLIST16 pVL16 = (LPVISLIST16)GlobalLock (hVL16);
	nread = BigRead (Fid,pVL16,sizeof(VISLIST16));
	if (nread == sizeof(VISLIST16))
	{
		nread = sizeof(VISLIST);
		ConvertVislist16to32 (CurVis,pVL16);
	}
	GSSiGlobUlFree (&hVL16);
	return nread;
}

int ReadComboFile16 (HFILE FidCF,LPCOMBOFILE pComboFile)
{
	COMBOFILE16	ComboFile16;

	int	rtn = BigRead (FidCF,(HPSTR)&ComboFile16,sizeof(COMBOFILE16)-sizeof(FIELDINFO16));
	ComboFile16ToComboFile32 (pComboFile,&ComboFile16);
	return rtn;
}

FIELDINFO ReadFieldInfo16 (HFILE FidCF)
{
	FIELDINFO FieldInfo;
	FIELDINFO16 FieldInfo16;

	BigRead (FidCF,(HPSTR)&FieldInfo16,sizeof(FIELDINFO16));
	FieldInfo = Field16toField32 (&FieldInfo16);
	return FieldInfo;
}

int WriteComboFile16 (HFILE FidCF,LPCOMBOFILE pComboFile)
{
	COMBOFILE16	ComboFile16;
	int	rtn;

	ComboFile32ToComboFile16 (&ComboFile16,pComboFile);
    rtn = BigWrite (FidCF,(HPSTR)&ComboFile16,sizeof(COMBOFILE16)-sizeof(FIELDINFO16),-1);
	return rtn;
}

int WriteFieldInfo16 (HFILE FidCF,LPFIELDINFO pFieldInfo)
{
	FIELDINFO16 FieldInfo16;
	int	rtn;

	FieldInfo16 = Field32toField16 (pFieldInfo);
	rtn = BigWrite (FidCF,(HPSTR)&FieldInfo16,sizeof(FIELDINFO16),-1);
	return rtn;
}

BOOL LoadInfoBox (LPSTR File,LPTAGBOX pTagBox)
{
    HFILE	Fid;
	int l;

 	Fid = GSSiOpenFile (File,0,OF_READ);
 	if (Fid == HFILE_ERROR)
 		return FALSE;
 	SetCurVal (File,IDS_FILEINB);
	l = GSSifilelength (Fid);
	if (l == sizeof(TAGBOX_V1))
	{
		TAGBOX_V1	TB_V1;
	
		GSSilread (Fid,&TB_V1,sizeof(TAGBOX_V1));
		ConvertTB_V1_to_V2 (pTagBox,&TB_V1);
	}
	else
		BigRead (Fid,(HPSTR)pTagBox,sizeof(TAGBOX)); 
	*pTagBox->Prefix = 0;
	*pTagBox->UDI = 0;
	if (!pTagBox->Flags.FixedSize)   
		pTagBox->bmWidthD = pTagBox->bmHeightD = 0;
 	GSSiClose2 (&Fid);
	return TRUE;
}

void ReadTAGBox (HFILE Fid,LPTAGBOX pTB)
{
	short version;
	TAGBOX_V1	TB_V1;
	
	GSSilread (Fid,&version,2);
	GSSillseek (Fid,-2,1);
	if (version == 1)
	{
		GSSilread (Fid,&TB_V1,sizeof(TAGBOX_V1));
		ConvertTB_V1_to_V2 (pTB,&TB_V1);
	}
	else
		GSSilread(Fid,pTB,sizeof(TAGBOX));
	return;
}
short LoadCGFViewports (LPSTR Name,long Offset,HFILE FidConfig,LPHANDLE hViewports,LPSHORT pCommandViewport,short Version)
#if ENABLETRACE
{GSSiEnterProg (1006);
#endif
{   
	LPSTR	pStr;
	short	NumIB, NumViews, NumMenuMask, MenuMask, imenu, iv, len, i;  
	long	WindowColor;
	LPTAGBOX	pTAGBox;
	HANDLE	hVisList, hTAG, hSTR;
	LPVISLIST	CurVis;
	LPVIEWPORT	SaveCurView=CurView;
	int		lVisList;
	long	ii;
	char	str[128];
	
	if (!Offset)
	{
		FidConfig = GetCfgFid (Name,&Version); 
		if (FidConfig == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1006);
#endif
		return 0;
}   
	    if (Version < 4)
	    {
	    	GSSiClose2 (&FidConfig);
{
#if ENABLETRACE
GSSiExitProg (1006);
#endif
    	return 0;
}
	    }  
	}
    else  
    {
		if (FidConfig == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1006);
#endif
		return 0;
}    
		GSSillseek (FidConfig,Offset,0);
    }
    hSTR=GSSiGlobAlloc ( 780,GMEM_MOVEABLE,1024);
    pStr =(LPSTR) GlobalLock (hSTR);
    hVisList=GSSiGlobAlloc ( 781,GMEM_MOVEABLE,sizeof(VISLIST));
    CurVis =(LPVISLIST) GlobalLock (hVisList);
    hTAG=GSSiGlobAlloc ( 782,GMEM_MOVEABLE,sizeof(TAGBOX));
    pTAGBox =(LPTAGBOX) GlobalLock (hTAG);

    ii=GSSilread (FidConfig,pStr,256);
    ii=GSSilread (FidConfig,pStr,128);
    ii=GSSilread (FidConfig,&NumIB,sizeof(NumIB));  
    while (NumIB--)
    {
        GSSilread(FidConfig,pTAGBox,sizeof(TAGBOX));
    }  
    ii=GSSilread (FidConfig,&NumViews,2);
    ii=GSSilread (FidConfig,pCommandViewport,2); 
    ii=GSSilread (FidConfig,&NumMenuMask,sizeof(NumMenuMask)); 
    for (imenu=0;imenu<NumMenuMask;imenu++)
    {
        GSSilread (FidConfig,&MenuMask,sizeof(MenuMask));  
    }
    ii=GSSilread (FidConfig,&WindowColor,4);
	ReadTAGBox (FidConfig,&TAGBox);  
//	sprintf (str,"Numv %i",NumViews);
//	MessageBox (0,str,0,MB_OK);
    for (iv=0;iv<NumViews;iv++)
    {
        hViewports[iv]=GSSiGlobAlloc ( 783,GHND,sizeof(VIEWPORT)+MAX_VIEWPORT_FILES*MAX_PATH);
        CurView = (LPVIEWPORT)GlobalLock (hViewports[iv]);
        if (Version < 8)
        {  
        	HANDLE	hVP6 = GSSiGlobAlloc (1529,GMEM_MOVEABLE,sizeof(VIEWPORT_V6));
        	LPVIEWPORT_V6 pVP6 = (LPVIEWPORT_V6)GlobalLock (hVP6);
        	HANDLE	hVP7 = GSSiGlobAlloc (1530,GMEM_MOVEABLE,sizeof(VIEWPORT_V7));
        	LPVIEWPORT_V7 pVP7 = (LPVIEWPORT_V7)GlobalLock (hVP7);
        	
            i=GSSilread (FidConfig,pVP6,sizeof(VIEWPORT_V6)); 
            ConvertVP_V6_to_V7 (pVP7,pVP6);  
            ConvertVP_V7_to_V8 (CurView,pVP7);  
            GSSiGlobUlFree (&hVP6);
            GSSiGlobUlFree (&hVP7);
        }
        else if (Version < 10)
        {  
        	HANDLE	hVP7 = GSSiGlobAlloc (1531,GMEM_MOVEABLE,sizeof(VIEWPORT_V7)); //sizeof(VIEWPORT)
        	LPVIEWPORT_V7 pVP7 = (LPVIEWPORT_V7)GlobalLock (hVP7);
        	
            i=GSSilread (FidConfig,pVP7,sizeof(VIEWPORT_V7)); 
            ConvertVP_V7_to_V8 (CurView,pVP7);  
            GSSiGlobUlFree (&hVP7);
        }
		else
            i=GSSilread (FidConfig,CurView,sizeof(VIEWPORT)); 
//		sprintf (str,"VP%i",CurView->ID);
//		MessageBox (0,str,0,MB_OK);
        ClearVPFields (CurView);  
        for (i=0;i<CurView->NumFiles;i++)
        {
            GSSilread (FidConfig,&len,2);
            CurView->lpFiles[i]=(LPSTR) &CurView->EndOfViewport + i * MAX_PATH;
            GSSilread (FidConfig,CurView->lpFiles[i],len); 
        } 
		lVisList = sizeof(VISLIST);
		if (Version < 8)
			lVisList = sizeof(VISLIST16);
        for (i=0;i<CurView->NumVisList;i++)
        {
            GSSilread (FidConfig,CurVis,lVisList);
        } 
        CurView->NumVisList = 0;
        for (i=0;i<CurView->NumPickList;i++)
        {
            GSSilread (FidConfig,CurVis,lVisList);
        } 
        CurView->NumPickList = 0;
        if (CurView->pVisListManual)
        {
            GSSilread (FidConfig,CurVis,lVisList);
        }
        CurView->pVisListManual = 0;
        if (CurView->pPickListManual)
        {
            GSSilread (FidConfig,CurVis,lVisList);
        } 
        CurView->pPickListManual = 0;
        ReadObject(&FidConfig,FALSE,0,OB_SAVEMASK);
        ReadObject(&FidConfig,FALSE,0,OB_SAVEBACKGROUND);
        if (CurView->pTheme)
        {
            ReadObject(&FidConfig,FALSE,&CurView->pTheme,0);
        }
        if (CurView->BoundsDisplayID)
        {
            BoundsDisplayRead (&CurView->lpBoundsDisplay,FidConfig);
        }
		GlobalUnlock (hViewports[iv]);
   }

	if (!Offset)
		GSSiClose2 (&FidConfig);

	GSSiGlobUlFree (&hVisList);
	GSSiGlobUlFree (&hTAG);
	GSSiGlobUlFree (&hSTR);
	CurView = SaveCurView;
{
#if ENABLETRACE
GSSiExitProg (1006);
#endif
	return NumViews;
}
#if ENABLETRACE
}
#endif
}	

void SaveLocalConfigToNetwork (HFILE FidConfig,LPSTR ConfigPath)
{
	if (*SaveLocalConfigNetDir)
	{
		char	dlDir[MAX_PATH];

		if (FidConfig == HFILE_ERROR)
			return;
		GetGlobalCVal ("[%DL]",dlDir,0);
		if (strnicmp (ConfigPath,dlDir,strlen(dlDir)))
		{
			char	ToPath[MAX_PATH];
			char	User[64];
			BOOL	SaveAllowCache = AllowCache;
			long	loc;

			loc = GSSillseek (FidConfig,0,1);
			GSSillseek (FidConfig,0,0);
			AllowCache = FALSE;
			GetGlobalCVal ("[%USERNAME]",User,0);
			sprintf (ToPath,"%s%s\\%s",SaveLocalConfigNetDir,User,ConfigPath);
			REPLAC (&ToPath[3], ":","", MAX_PATH);
			{
				OFSTRUCTGM	OFStruct;
				HFILE FidTo = GSSiOpenFile (ToPath,&OFStruct,OF_CREATE);

				if (FidTo != HFILE_ERROR)

				{
					int	lr;

					while ((lr = BigRead (FidConfig,pCommonMem,lCommonMem))>0)
						BigWrite (FidTo,pCommonMem,lr,-1);
					GSSiClose2 (&FidTo);
				}
			}
			AllowCache = SaveAllowCache;
			GSSillseek (FidConfig,loc,0);
		}
	}
	return;
}

BOOL OpenConfig (HWND hWnd,HDC hDC)
#if ENABLETRACE
{GSSiEnterProg (100);
#endif
{   short     Signature, Version, n;
	OFSTRUCTGM    OFStruct;
    HANDLE		hVisList;
    short     len, iv,MenuMask;
	int	i;
    LPVIEWPORT  SaveView;
    LPSTR   LastVisList;
    LPVISLIST   SaveVis;
    short     imenu,   NumIB;
    short     NumViews;
    LPSHORT   lpMenuMask; 
    long    ii;   
    char    Cfg[128], str[512], FullPath[MAX_PATH], Suffix[8];
    LPSTR	pStr; 
    static	BOOL	First = TRUE;
    long	lenCfg;
	int		MaxFileLength = MAX_PATH;
	LPSTR	pTE, pTEend;
	char	setTestEnv[MAX_PATH + 64] = { 0 };

	if (FidConfig != HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (100);
#endif
    	return (TRUE);
}
    if (!CfgName[0])
{
#if ENABLETRACE
GSSiExitProg (100);
#endif
    	return (FALSE);    
}  
	ClearPolyOff(FALSE);
	CloseAllRequestedFiles (FALSE);
	memset (&InitWindowRect,0,sizeof(RECT));
	SetProcessStatusTitle (0);
    ExpandText (CfgName);  
    if (!_fstrchr (CfgName,'.'))
    	_fstrcat (CfgName,".gmc");
    _fullpath (FullPath,CfgName,sizeof(FullPath)); 
    _fstrlwr (FullPath);
	if ((pTE = strstr(FullPath, "\\testenvironments\\")))
	{
		pTE += strlen("\\testenvironments\\");
		if ((pTEend = strchr(pTE, '\\')))
		{
			*pTEend = 0;
			sprintf (setTestEnv,"[%%TESTDL]=[%%DL]testenvironments\\%s", pTE);
			*pTEend = '\\';
		}
	}
    _splitpath (FullPath,0,0,Cfg,Suffix);
    SetGlobalValue ("%CONFIGPATH",FullPath); 
    SetGlobalValue ("%CONFIG",Cfg);
	strcpy (str,"[%WT]");
	ExpandText (str);
	if (!*str)
	{
		sprintf (str,"GeoMaster: %s",FullPath);
		if (!App)
			SetGlobalValue ("%WT",str); 
	}
//7/16/04 moved from bottomto allow colors settings saved in config to stay	
	LoadGlobalInit ("global.ini",FALSE);
	GetPrivateProfileString ("Install","InDir","c:\\geomastr",str,sizeof(str),"geomastr.ini"); 
	_fstrcat (str,"\\"); 
	SetGlobalValue("%INDIR",str);
	if (GetGlobalCVal ("[%USERDIR]",str,0))
	 	LoadGlobalInit ("[%USERDIR]global.ini",FALSE); 
    if (CreateConfig)
    	CreateTestConfig ();   
    CreateConfig = FALSE;  
    CurPassiveFun = 0;
    if (!ConfigLevel)   
    {
     	_fstrcpy (Lev0CfgName,CfgName); 
	    if (SetConfig(0))
			UnallocateConfig ();
	    SetConfig(1);
	    if (!IgnoreSavedMenu)
			*MenuCFGName = 0;
		ConfigDisplayRect.left=ConfigDisplayRect.right = 0; 
	}
    else
    	SetConfig(1); 
    EmbededMenuLoc = 0;
    nAVLVP = 0;
    LastVP = LastBoxVP = CurView = 0;  
    ConfigID++;
    RezoomRect.xmn = 0;
    RezoomRect.xmx = -1;
    *pNumViewports = 0;  
    _fmemset (pViewports,0,MAX_VIEWPORTS*sizeof(LPVIEWPORT)); 
    GSSiGlobFree (&hConfigDescription);
    FidConfig = GSSiOpenFile (FullPath,(LPOFSTRUCTGM)&OFStruct,OF_READ);
    if (FidConfig == HFILE_ERROR) 
    {   char    DLName[MAX_PATH];
    
        if (!strchr (CfgName,':'))
		{
			_fstrcpy (DLName,"[%DL]");
			_fstrcat (DLName,CfgName);
			FidConfig = GSSiOpenFile (DLName,(LPOFSTRUCTGM)&OFStruct,OF_READ);
		}
        if (FidConfig == HFILE_ERROR)
        {
            GSSiMessageBox (0,"Configuration file not found",CfgName, MB_OK,0);
			GetGlobalCVal ("[%DEFAULTCONFIG]",CfgName,"basic1.gmc");
			ShowWindow(hWndMain, SW_SHOW);
{
#if ENABLETRACE
GSSiExitProg (100);
#endif
            return (FALSE);
}
        } 
    } 
	SaveLocalConfigToNetwork (FidConfig,FullPath);
	ProcessText(setTestEnv);
	GSSifstat(FidConfig, &OpenConfigStat);
    SetGlobalValue ("%CONFIG",Cfg);
    SetCurVal (FullPath,IDS_FILEGMC);
    lenCfg = GSSillseek (FidConfig,0,2);
    ii = GSSillseek(FidConfig,lenCfg-6,0);

    GSSilread (FidConfig,&ConfigDesc,2);
    GSSilread (FidConfig,&Signature,2);
    GSSilread (FidConfig,&Version,2);
	if (Version < 8)
		MaxFileLength = 128;
	ConfigVersion = Version;
    if (Signature != 28052)
    {   CloseConfig ();
        GSSiMessageBox (0,"This is not a valid configuration file",CfgName, MB_OK,0);
{
#if ENABLETRACE
GSSiExitProg (100);
#endif
        return (FALSE);
}
    } 
    if (Version < 1 || Version > CURRENT_CFG_VERSION)
    {   CloseConfig ();
        GSSiMessageBox (0,"This configuration file version is not recognized",CfgName, MB_OK,0);
{
#if ENABLETRACE
GSSiExitProg (100);
#endif
        return(FALSE);
}
    }
    if (Version > 5)
    	FidConfig = DecompressCfgFile (FidConfig,Version);
    if (Version > 4 && ConfigDesc.Len>1)   
    {   
    	LPSTR	pConfigDescription;
    	short	id;
    	
	    GSSiGlobFree (&hConfigDescription);
    	hConfigDescription = GSSiGlobAlloc (1315,GMEM_MOVEABLE,512);
    	pConfigDescription = GlobalLock (hConfigDescription);
	    GSSillseek(FidConfig,(LONG)-(6+4+4),2);
	    GSSilread (FidConfig,&CfgDescLoc,4);
	    GSSillseek(FidConfig,CfgDescLoc,0);
	    GSSilread (FidConfig,&id,2); 
	    GSSilread (FidConfig,pConfigDescription,ConfigDesc.Len);
	    SetGlobalValue ("%CFGDESC",pConfigDescription);
	    GlobalUnlock (hConfigDescription);
    }
    else
    {
    	ConfigDesc.Len = 0;
	    SetGlobalValue ("%CFGDESC","");
    }	
    NumSavedImages = 0;
    if (Version > 4 && ConfigDesc.HaveImage)   
    {   
    	LPSTR	pCfgImage;  
    	short	id;
 		HDIB32 hDib;
   	                               
	   ii=GSSillseek(FidConfig,-(long)(6+4),2);
	   ii= GSSilread (FidConfig,&CfgImageLoc,4); 
	    GSSillseek(FidConfig,CfgImageLoc,0);
	    GSSilread (FidConfig,&id,2); 
	    GSSilread (FidConfig,&CfgImageFormat,2); 
	    GSSilread (FidConfig,&CfgImageLen,4);
		switch (id)
		{
			case OB_CONFIGIMAGES:
				{
					int	i=0;
					int TotImageLen=0;
					
					SaveImagesOffset = GSSillseek (FidConfig,0,1);
					BigRead (FidConfig,(HPSTR)&NumSavedImages,sizeof(int)); 
					BigRead (FidConfig,(HPSTR)&LastBounds,sizeof(MNMXCORD)); 
					BigRead (FidConfig,(HPSTR)SavedImageData,NumSavedImages*sizeof(SAVEDIMAGEDATA)); 
					for (i=0;i<NumSavedImages;i++)
						TotImageLen += SavedImageData[i].ImageLen;
					hCfgImage = GSSiGlobAlloc (1534,GMEM_MOVEABLE,TotImageLen);
					pCfgImage = GlobalLock (hCfgImage); 
					BigRead (FidConfig,(HPSTR)pCfgImage,TotImageLen);  
					GlobalUnlock (hCfgImage);
					//CfgImageFormat = 1;
				}
				break;
			default:
				if (CfgImageLen)
				{ 
					hCfgImage = GSSiGlobAlloc (1534,GMEM_MOVEABLE,CfgImageLen);
					pCfgImage = GlobalLock (hCfgImage); 
					BigRead (FidConfig,(HPSTR)pCfgImage,CfgImageLen);  
					if (CfgImageFormat != 1)
					{
						hDib = LoadDIBFromMem (pCfgImage,CfgImageLen,CfgImageFormat,0);
						GSSiGlobUlFree (&hCfgImage);
						hCfgImage = hDib;
					}
					else
						GlobalUnlock (hCfgImage);
					//SaveBitmap (hCfgImage,"c:\\testitem.bmp",0,0);
				}
				break;
			}
	}
	else
		hCfgImage = 0;
    GSSillseek(FidConfig,0,0);
    if (Version >= 3)
    {   
    	LPSTR	pVB;
    	
    	pStr = GlobalLock (hStartupCommand);
        GSSilread (FidConfig,pStr,256);
        GlobalUnlock (hStartupCommand);
    	pStr = GlobalLock (hStartupMenu);
        GSSilread (FidConfig,pStr,128);
        OneSpace (pStr);
        GlobalUnlock (hStartupMenu);
        GSSilread (FidConfig,&NumIB,sizeof(NumIB));  
        while (NumIB--)
        {
			ReadTAGBox (FidConfig,&TAGBox);
			TAGBox.hReport = 0;
            SaveTAG(-1);
        }  
    }   
    GSSilread (FidConfig,&NumViews,2);
    GSSilread (FidConfig,&*pCommandViewport,2); 
    if (!*pCommandViewport)
    	*pCommandViewport = 1;
    GSSilread (FidConfig,&NumMenuMask,sizeof(NumMenuMask)); 
    GSSiGlobFree (&hMenuMask);
    if (NumMenuMask)
    { 
	    hMenuMask=GSSiGlobAlloc (1316,GMEM_MOVEABLE,(NumMenuMask+1)*sizeof(short));
	    lpMenuMask=(LPSHORT)GlobalLock(hMenuMask); 
	    for (imenu=0;imenu<NumMenuMask;imenu++,lpMenuMask++)
	    {
	        GSSilread (FidConfig,lpMenuMask,sizeof(MenuMask));  
	        if (hWnd)
	        {
	            if (!DeleteMenu (GetMenu(hWnd),*lpMenuMask,MF_BYCOMMAND))
	                ii=1;
	        }
	    }
	    GlobalUnlock(hMenuMask);
	} 
    //if (hWnd) DrawMenuBar(hWnd); 
    GSSilread (FidConfig,&WindowColor,4);
	ReadTAGBox (FidConfig,&TAGBox);  
    ii=sizeof(VIEWPORT);
	ii=sizeof(VIEWPORT_V6);
    ii=sizeof(NEWOBJECT);
	ii=sizeof(NEWOBJECT16);
	ii=sizeof(LOGFONT);
	ii=sizeof(LOGFONT16);
	ii=sizeof(IBFLAGS);
	ii=sizeof(TAGBOX_V1);
    for (iv=0;iv<NumViews;iv++)
    {
        hViewports[iv]=GSSiGlobAlloc (1317,GHND,sizeof(VIEWPORT)+8+MAX_VIEWPORT_FILES*MAX_PATH);   
        SetCurView ( (LPVIEWPORT) GlobalLock (hViewports[iv]));
        pViewports[iv]=CurView;
        pViewportsD[iv]=CurView;
        if (Version == 1)
        {
            i=GSSilread (FidConfig,CurView,sizeof(VIEWPORT_v0));
            _fstrcpy (CurView->PickMacroFile,"pikmacro.txt"); 
        } 
        else if (Version < 4)
        {
            i=GSSilread (FidConfig,CurView,sizeof(VIEWPORT_v2)); 
            CurView->NumNewObjects=0; 
            _fmemset (CurView->NewObjectMap,0,sizeof(CurView->NewObjectMap));
        }
        else if (Version < 8)
        {  
        	HANDLE	hVP6 = GSSiGlobAlloc (1535,GMEM_MOVEABLE,sizeof(VIEWPORT_V6));
        	LPVIEWPORT_V6 pVP6 = (LPVIEWPORT_V6)GlobalLock (hVP6);
        	HANDLE	hVP7 = GSSiGlobAlloc (1536,GMEM_MOVEABLE,sizeof(VIEWPORT_V7));
        	LPVIEWPORT_V7 pVP7 = (LPVIEWPORT_V7)GlobalLock (hVP7);
        	
            i=GSSilread (FidConfig,pVP6,sizeof(VIEWPORT_V6)); 
            ConvertVP_V6_to_V7 (pVP7,pVP6);  
            ConvertVP_V7_to_V8 (CurView,pVP7);  
            GSSiGlobUlFree (&hVP6);
            GSSiGlobUlFree (&hVP7);
        }
        else if (Version < 10)
        {  
        	HANDLE	hVP7 = GSSiGlobAlloc (1537,GMEM_MOVEABLE,sizeof(VIEWPORT_V7)); //sizeof(VIEWPORT)
        	LPVIEWPORT_V7 pVP7 = (LPVIEWPORT_V7)GlobalLock (hVP7);
        	
            i=GSSilread (FidConfig,pVP7,sizeof(VIEWPORT_V7)); 
            ConvertVP_V7_to_V8 (CurView,pVP7);  
            GSSiGlobUlFree (&hVP7);
        }
		else
            i=GSSilread (FidConfig,CurView,sizeof(VIEWPORT));  
        if (CurView->ID < 1 || CurView->ID > MAX_VIEWPORTS)
        {
			BlowOut ("Corrupted configuration file",FullPath);
	        break;
{
#if ENABLETRACE
GSSiExitProg (100);
#endif
	        return FALSE;
}
        }
        CurView->Version=CURRENT_VP_VERSION; 
        ClearVPFields (CurView);  
        CurView->PriorBounds.xmn = CurView->PriorBounds.xmx = 0;
        CurView->hDC = hDC;
        CurView->hWnd = hWnd;   
        if (!_fstricmp (CurView->Name,"Format"))
        	CurView->Type = 0;
        if (CurView->UpdateFile > CurView->NumFiles)
        	CurView->UpdateFile = 0;
        CurView->NumNewObjects = min (CurView->NumNewObjects,100);
        if (CurView->OldFunctionStact0)
        {
        	CurView->StartupFunction = CurView->OldFunctionStact0;
        	CurView->OldFunctionStact0 = 0; 
        }        	
        if (!_fstricmp (CurView->FunctionDir,"fundir"))
            _fstrcpy (CurView->FunctionDir,"index.txt");
        if (CurView->Type != 7)
			CurView->ZoomTarget = 0;

        for (i=0;i<CurView->NumFiles;i++)
        {
            GSSilread (FidConfig,&len,2);
            CurView->lpFiles[i]=(LPSTR) &CurView->EndOfViewport + i * MAX_PATH;
            GSSilread (FidConfig,CurView->lpFiles[i],len); 
            _fstrupr (CurView->lpFiles[i]);

        } 
        for (i=0;i<CurView->NumVisList;i++)
        {
            hVisList=GSSiGlobAlloc (1318,GHND,sizeof(VISLIST));
            CurVis =(LPVISLIST) GlobalLock (hVisList);
			if (Version < 8)
			{
        		HANDLE	hVL16 = GSSiGlobAlloc (1538,GMEM_MOVEABLE,sizeof(VISLIST16));
        		LPVISLIST16 pVL16 = (LPVISLIST16)GlobalLock (hVL16);
	            GSSilread (FidConfig,pVL16,sizeof(VISLIST16));
				ConvertVislist16to32 (CurVis,pVL16);
				GSSiGlobUlFree (&hVL16);
			}
			else
	            GSSilread (FidConfig,CurVis,sizeof(VISLIST));
            CurVis->hVisList = hVisList;
            if (!i)
            {
                CurView->pVisList1=CurVis;
                CurVis->LastVisList = 0;
            }
            else
            {   
                CurVis->LastVisList = LastVisList;
                SaveVis = CurVis;
                CurVis =(LPVISLIST) LastVisList;
                CurVis->NextVisList = (LPSTR)SaveVis;
                CurVis = SaveVis;
            }
            CurVis->NextVisList = 0;
            LastVisList = (LPSTR)CurVis;
        } 
        for (i=0;i<CurView->NumPickList;i++)
        {
            hVisList=GSSiGlobAlloc (1319,GHND,sizeof(VISLIST));
            CurVis =(LPVISLIST) GlobalLock (hVisList);
			if (Version < 8)
			{
        		HANDLE	hVL16 = GSSiGlobAlloc (1539,GMEM_MOVEABLE,sizeof(VISLIST16));
        		LPVISLIST16 pVL16 = (LPVISLIST16)GlobalLock (hVL16);
	            GSSilread (FidConfig,pVL16,sizeof(VISLIST16));
				ConvertVislist16to32 (CurVis,pVL16);
				GSSiGlobUlFree (&hVL16);
			}
			else
				GSSilread (FidConfig,CurVis,sizeof(VISLIST));
            CurVis->hVisList = hVisList;
            if (!i)
            {
                CurView->pPickList1=CurVis;
                CurVis->LastVisList = 0;
            }
            else
            {   
                CurVis->LastVisList = LastVisList;
                SaveVis = CurVis;
                CurVis =(LPVISLIST) LastVisList;
                CurVis->NextVisList = (LPSTR)SaveVis;
                CurVis = SaveVis;
            }
            CurVis->NextVisList = 0;
            LastVisList = (LPSTR)CurVis;
        } 
        if (CurView->pVisListManual)
        {
            hVisList=GSSiGlobAlloc (1320,GHND,sizeof(VISLIST));
            CurVis =(LPVISLIST) GlobalLock (hVisList);
			if (Version < 8)
			{
        		HANDLE	hVL16 = GSSiGlobAlloc (1540,GMEM_MOVEABLE,sizeof(VISLIST16));
        		LPVISLIST16 pVL16 = (LPVISLIST16)GlobalLock (hVL16);
	            GSSilread (FidConfig,pVL16,sizeof(VISLIST16));
				ConvertVislist16to32 (CurVis,pVL16);
				GSSiGlobUlFree (&hVL16);
			}
			else
				GSSilread (FidConfig,CurVis,sizeof(VISLIST));
            CurVis->hVisList = hVisList;   
            CurView->pVisListManual = CurVis;
        }
        if (CurView->pPickListManual)
        {
            hVisList=GSSiGlobAlloc (1321,GHND,sizeof(VISLIST));
            CurVis =(LPVISLIST) GlobalLock (hVisList);
 			if (Version < 8)
			{
        		HANDLE	hVL16 = GSSiGlobAlloc (1541,GMEM_MOVEABLE,sizeof(VISLIST16));
        		LPVISLIST16 pVL16 = (LPVISLIST16)GlobalLock (hVL16);
	            GSSilread (FidConfig,pVL16,sizeof(VISLIST16));
				ConvertVislist16to32 (CurVis,pVL16);
				GSSiGlobUlFree (&hVL16);
			}
			else
				GSSilread (FidConfig,CurVis,sizeof(VISLIST));
            CurVis->hVisList = hVisList;   
            CurView->pPickListManual = CurVis;
        }
        if (CurView->VisName[0])
            LoadVisList (CurView->VisName);
        if (CurView->PickName[0])
            LoadPickList (CurView->PickName);
        LoadDisplayRedefFile(CurView->DisplayRedefFile);   

        ReadObject(&FidConfig,FALSE,0,OB_SAVEMASK);
        ReadObject(&FidConfig,FALSE,0,OB_SAVEBACKGROUND);
        
        if (CurView->pTheme)
        {
            if (ReadObject(&FidConfig,FALSE,&CurView->pTheme,0))
            	CurView->pTheme->DisplayViewport = CurView->ID;
        }
        if (CurView->BoundsDisplayID)
        {
            BoundsDisplayRead (&CurView->lpBoundsDisplay,FidConfig);
        }
        if (CurView->ZoomTarget && CurView->ZoomTarget < CurView->ID)
        {
            pViewportsD[iv] = pViewportsD[CurView->ZoomTarget-1];
            pViewportsD[CurView->ZoomTarget-1]=CurView;
        } 
		*CurView->CurrentGoogleImage = 0;
//          AddActivateSwitch();
// 		sprintf (str,"VP %i %i %i",CurView->ID,CurView->NumPickList,CurView->NumFiles);
//		MessageBox (0,str,0,MB_OK);
    }
    n = 0; 
    for (iv=0;iv<NumViews;iv++)
    {   
        if (pViewports[iv]->Type == 7)
            pViewportsD[n++] = pViewports[iv];
    }
    for (iv=0;iv<NumViews;iv++)
    {   
        if (pViewports[iv]->Type != 7 && !pViewports[iv]->pTheme)
            pViewportsD[n++] = pViewports[iv];
    }
    for (iv=0;iv<NumViews;iv++)
    {   
        if (pViewports[iv]->Type != 7 && pViewports[iv]->pTheme)
            pViewportsD[n++] = pViewports[iv];
    }
    
    for (iv=0;iv<NumViews;iv++)
    {
        SetCurView ( pViewports[iv]);  
//        sprintf (str,"%i %i",iv);
//        SetWindowText (hWndMain,str);
		if (CurView->lpBoundsDisplay)
		{
			HANDLE h=CurView->lpBoundsDisplay->Handle;

			if (CurView->lpBoundsDisplay->TargetVP > 0 && CurView->lpBoundsDisplay->TargetVP <= NumViews)
			{
				if (pViewports[CurView->lpBoundsDisplay->DisplayVP-1]->pTheme &&
					pViewports[CurView->lpBoundsDisplay->DisplayVP-1]->pTheme->ID == GF_BOUNDS_DISPLAY_THEME &&
					pViewports[CurView->lpBoundsDisplay->DisplayVP-1]->pTheme->TargetViewport == CurView->ID)
				goto BDOK;
			}
			GSSiGlobUlFree (&h);
			CurView->lpBoundsDisplay = 0;
		}
BDOK:
        if (CurView->pTheme)
        {
            SaveView = CurView;

            CurTheme = CurView->pTheme; 
            if (CurTheme->ID == GF_BOUNDS_DISPLAY_THEME)
            	CurTheme->NumVals = -1;
            if (CurTheme->TargetViewport)
            { 
	            SetCurView ( pViewports[CurTheme->TargetViewport-1]); //pViewports[3]
	            if (CurView)
	            {
					AddThemeToVP (CurView,CurTheme);
		            if (CurTheme->ID == GF_BOUNDS_DISPLAY_THEME) 
		            {
	                    BoundsDisplayDestroy (CurView->lpBoundsDisplay); 
				        CurView->lpBoundsDisplay=0;
				    }
	            }
	            SetCurView (SaveView);  
	        }
        }
    }
    LoadedHLTFromConfig = FALSE;    
    *pNumViewports = NumViews; 
    while (ReadObject (&FidConfig, FALSE,0,0));
	if (!IsRectEmpty(&InitWindowRect))
	{
		showWindowCmd = SW_SHOW;
	}
	if (!MapServer && !IsWindowVisible(hWndMain))
	{
		ShowWindow(hWndMain, showWindowCmd);
	}
	DisplayAllToolbars(4);
	if (hStartupMenu)
	{
        pStr = GlobalLock (hStartupMenu);
        if (*pStr)
        {   
            if (!GMLoadMenu (hWndMain,pStr)) 
            {   
                char    mess[256];
                
                sprintf (mess,"Failed to load menu '%s'",pStr);
                GSSiMessageBox (0,mess,0, MB_OK|MB_ICONEXCLAMATION,0);
            }
        }
        else if (hUserMenu && hWnd) 
        	LoadFullMenu (hWnd);
        GlobalUnlock (hStartupMenu);  
        SetMenuDefaultValues (hWnd);
	}
    if (ExistFile ("startup.bmp") && FirstSchmooze && hWnd)
    { 
        FirstSchmooze=FALSE;
        {
#if WIN32
			DialogBox(hInst, (LPSTR)"SCHMOOZ", hWnd, (DLGPROC)SCHMOOZMsgProc);
#else
          FARPROC lpfnSCHMOOZMsgProc;
          lpfnSCHMOOZMsgProc = MakeProcInstance((FARPROC)SCHMOOZMsgProc, hInst);
          DialogBox(hInst, (LPSTR)"SCHMOOZ", hWnd, lpfnSCHMOOZMsgProc);
          FreeProcInstance(lpfnSCHMOOZMsgProc);
#endif
        }
    }
    *pNumViewports = NumViews; 
	DisplayProfileLoc (0,0,0,TRUE);
	DisplayProfileLink (0);
    SetGlobalValue ("%CONFIG",Cfg);
    SetCurVal (FullPath,IDS_FILEGMC);
    if (!*pNumViewports)
{
#if ENABLETRACE
GSSiExitProg (100);
#endif
    	return FALSE;
}
    TransferSavedZoom ();  
    DetermineVPDisplaySequence ();
    SetCurView (pViewports[*pCommandViewport-1]);  
    if (ForceBounds)
    {
		if (CfgImageFormat == 1)
			GSSiGlobFree (&hCfgImage);
		else
		{
			GMDestroyDIB32 (hCfgImage);
			hCfgImage = 0;
		} 
    	CurView->NewBounds = CurView->WBounds;
   		CurView->HaveBounds=TRUE; 
	}
	else if (ValidBounds (&StartupZoom) && First)
    {
		if (CfgImageFormat == 1)
			GSSiGlobFree (&hCfgImage);
		else
		{
			GMDestroyDIB32 (hCfgImage);
			hCfgImage = 0;
		} 
    	CurView->WBounds = CurView->NewBounds = StartupZoom;
   		CurView->HaveBounds=TRUE; 
		SetScaleAndMidpointFromBounds (CurView);
		CurView->WindowIsZoomed = TRUE;
	}
    else if (!CurView->HaveBounds && hDC)
    {
    	if (GetVisBounds (&CurView->NewBounds,hDC))
    	{   
    		CurView->HaveBounds=TRUE; 
    		CurView->WindowIsZoomed = TRUE;
    		CurView->WBounds = CurView->NewBounds;
//           	SetBounds(CurView->hWnd,hDC); 
        }
    }
    else
    	CurView->NewBounds = CurView->WBounds;
    ResetToViewport = 0;
    ConfigLoaded = TRUE;
	//RunStartupCommand (1); moved 2010_06_10
    for (iv=0;iv<NumViews;iv++)
    {
        SetCurView ( pViewports[iv]);  
        if (ValidStartupFunction(CurView->StartupFunction))
        {
           	AddLBUTTON = TRUE;
			AddGraphicsFunction (hWnd, CurView->StartupFunction,0);        
		}
		else
			CurView->StartupFunction = 0;
    }
    SetCurView (pViewports[*pCommandViewport-1]);
    if (!ConfigLevel)   
    {
	    if (!LoadedHLTFromConfig)  
			ClearHighlightList (FALSE); 
	    if (GetGlobalBVal ("[%PROMPTS]"))   
	    	DoScreenPrompt=TRUE;
		else
	    	DoScreenPrompt=FALSE;

//	    if (GetGlobalBVal ("[%OPENVIDEOWINDOW]"))
//	    	GEOSPANVideo(hWndMain);
	    SetGlobalValue ("%CONFIGPATH",FullPath); 
	    SetGlobalValue ("%CONFIG",Cfg);
	    FirstDisplayOfConfig = TRUE;
	    if (CFGOpenTrackingStatus)
	    	GPSTracking (1);	
	    CFGOpenTrackingStatus = FALSE;      
		if (*MenuCFGName)
		{
			char	FullCFG[MAX_PATH];
			char	FullMenu[MAX_PATH];

			strcpy (FullCFG,CfgName);
			ExpandText (FullCFG);
			_fullpath (FullCFG,FullCFG,MAX_PATH);
			strcpy (FullMenu,MenuCFGName);
			ExpandText (FullMenu);
			_fullpath (FullMenu,FullMenu,MAX_PATH);
			if (stricmp (FullCFG,FullMenu))
				LoadMenuConfig (MenuCFGName,0,HFILE_ERROR,Version); 
			else
				*MenuCFGName = 0;
		}
		else if (EmbededMenuLoc)
			LoadMenuConfig (FullPath,EmbededMenuLoc,FidConfig,Version);
	    if (First)
	    {
	    	//First = FALSE;
	    	RunSportMapStartupCommand ();
	    }
	    SetConfig (1); 
	    if (!pViewportsD[0]->Type && pViewportsD[0]->ShowFullScreen)
			UseFullScreen (hWnd,0);
	}
    GSSiClose (FidConfig);
	RunStartupCommand (1);  
	DisplayAllToolbars (0);
	LogUsageInfo (1,FullPath);
/*    for (iv=0;iv<NumViews;iv++)
    {
        SetCurView ( pViewports[iv]);
		if (CurView->UsePanZoomControl)
		{
			CreatePanZoomRotTool (CurView->hWnd,CurView->PanZoomControlPoint);
			break;
		}
	}*/
	ConfigChangesMade = FALSE;
{
#if ENABLETRACE
GSSiExitProg (100);
#endif
    return(TRUE); 
}
#if ENABLETRACE
}
#endif
} 
BOOL SaveConfig (LPSTR Name,BOOL UseCompression)
#if ENABLETRACE
{GSSiEnterProg (101);
#endif
{   short     Signature, Version=CURRENT_CFG_VERSION;  
    short     len, iv,  i;
    short     imenu;
    LPSHORT   lpMenuMask;
    short     NumViews; 
    LPVISLIST   SaveVisList, SavePickList;  
    LPVISLIST	SaveInVis=CurVis;
    HBITMAP SaveBitmap;  
	HDIB32		hDib32;
    short     NumIB; 
    LPSTR	pStr;  
    LPTHEME	SaveTheme;  
    LPVIEWPORT	SaveVP=CurView;   
    char	TempName[MAX_PATH]=""; 
    
    if (CurrentConfig && EmbedMenus)
    {   
        BOOL SSaveCfgSizePos = SaveCfgSizePos;
        BOOL SSaveCfgSizePosAll = SaveCfgSizePosAll;
    	BOOL SSaveGlobals = SaveGlobals;
    	BOOL SSaveHLTList = SaveHLTList;
    	BOOL SSaveInfoBox = SaveInfoBox;   
    	BOOL SSaveCfgImage = SaveCfgImage;
    	BOOL SSavePrintSetup = SavePrintSetup;
//		HDIB32	SSavehWindowDIB = hWindowDib32;
    	
    	SaveMenuName = FALSE;
    	SaveHLTList = FALSE; 
    	SaveCfgSizePos = FALSE;
    	SaveCfgSizePosAll = FALSE;
        SaveGlobals = FALSE;  
        EmbedMenus = FALSE;  
        SaveInfoBox = FALSE; 
        SaveCfgImage = FALSE; 
	    GMDestroyDIB32 (hWindowDib32);
		hWindowDib32 = 0;
        SavePrintSetup = FALSE;
    	SetConfig (0);
    	GSSiGetTempFileName (0,"gmc",0,TempName); 
    	SaveConfig (TempName,FALSE);
    	SetConfig (1);  
    	EmbedMenus = TRUE;
    	SaveInfoBox = SSaveInfoBox;
    	SaveHLTList = SSaveHLTList; 
    	SaveCfgSizePos = SSaveCfgSizePos;
    	SaveCfgSizePosAll = SSaveCfgSizePosAll;
        SaveGlobals = SSaveGlobals; 
        SaveCfgImage = SSaveCfgImage;
        SavePrintSetup = SSavePrintSetup;
//		hWindowDib32 = SSavehWindowDIB;
    }
    IgnoreSavedMenu = FALSE;
    FidConfig = GSSiOpenFile (Name,0,OF_CREATE);
    if (FidConfig == HFILE_ERROR) 
    {   HANDLE	hMem=GSSiGlobAlloc (1322,GMEM_MOVEABLE,256);
    	LPSTR	str = GlobalLock (hMem);
        
        sprintf (str,"Unable to create config file: %s",Name);
        GSSiMessageBox (0,str,0, MB_OK|MB_ICONEXCLAMATION,0); 
        GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (101);
#endif
        return (FALSE);
}
    } 
	SetVarSaveStatus ("%CFGDESC",FALSE);
    SetGlobalValue("%P","");	 
    pStr = GlobalLock (hStartupCommand);
    BigWrite (FidConfig,(HPSTR)pStr,256,-1);
    GlobalUnlock (hStartupCommand);
    pStr = GlobalLock (hStartupMenu);
    BigWrite (FidConfig,(HPSTR)pStr,128,-1); 
    GlobalUnlock (hStartupMenu);
    if (SaveInfoBox)
    	NumIB = GetNumInfoBox ();
    else
    	NumIB = 0;
    BigWrite (FidConfig,(HPSTR)&NumIB,sizeof(NumIB),-1); 
    if (NumIB)
        WriteInfoBoxes (FidConfig);
    NumViews=*pNumViewports; 
    BigWrite (FidConfig,(HPSTR)pNumViewports,2,-1);
    BigWrite (FidConfig,(HPSTR)&*pCommandViewport,2,-1);               
    BigWrite (FidConfig,(HPSTR)&NumMenuMask,sizeof(NumMenuMask),-1);
    if (NumMenuMask)
    { 
	    lpMenuMask = (LPSHORT)GlobalLock (hMenuMask); 
	    for (imenu=0;imenu<NumMenuMask;imenu++,lpMenuMask++)
	        BigWrite (FidConfig,(char *)lpMenuMask,sizeof(short),-1);  
	    GlobalUnlock (hMenuMask);
	}
    BigWrite (FidConfig,(HPSTR)&WindowColor,4,-1);
    BigWrite(FidConfig,(HPSTR)&TAGBox,sizeof(TAGBOX),-1);
    for (iv=0;iv<NumViews;iv++)
    {
        SetCurView ( pViewports[iv]); 
        SaveBitmap = CurView->Bitmap;
        CurView->Bitmap = 0;
		if (CurView->DisplayedFullScreen)
		{
			CurView->TagPoint = CurView->TagPointSave;
			CurView->Width = CurView->SaveWidth;
			CurView->Height = CurView->SaveHeight;
			CurView->DisplayedFullScreen = 0;
		}
		if (!CurView->lpBoundsDisplay)
        	CurView->BoundsDisplayID = 0;
		CurView->NumVisList = 0;  
        CurVis = CurView->pVisList1;
        while (CurVis)
        {  
            CurView->NumVisList++;
            CurVis =(LPVISLIST) CurVis->NextVisList;
        } 
		CurView->NumPickList = 0;  
        CurVis = CurView->pPickList1;
        while (CurVis)
        {  
            CurView->NumPickList++;
            CurVis =(LPVISLIST) CurVis->NextVisList;
        } 
        SavePickList = CurView->pPickList1;
        SaveVisList = CurView->pVisList1;
        CurView->pPickList1 = 0;
        CurView->pVisList1 = 0;   
        CurView->WBoundsWhenSaved = CurView->WBounds;
        if (!SaveZoom)
        {
            CurView->WindowIsZoomed = FALSE; 
            CurView->HaveBounds = FALSE;
        }
        CurView->NumNewObjects = min (CurView->NumNewObjects,100);
        CloseNewObjects(); 
        SaveTheme = CurView->pTheme;
//        if (CurView->pTheme && CurView->Type != 7)
//        	CurView->pTheme = 0;

		ExpandText (CurView->FunctionDir);
		SubstituteDL (CurView->FunctionDir,FALSE);  
		ExpandText (CurView->FunctionFile);
		SubstituteDL (CurView->FunctionFile,FALSE);  
        i=BigWrite (FidConfig,(HPSTR)CurView,sizeof(VIEWPORT),-1);
       	CurView->pTheme = SaveTheme; 
        CurView->pPickList1 = SavePickList;
        CurView->pVisList1 = SaveVisList;
        for (i=0;i<CurView->NumFiles;i++)
        {   
            len = _fstrlen(CurView->lpFiles[i])+1;
            BigWrite (FidConfig,(HPSTR)&len,2,-1);
            BigWrite (FidConfig,(HPSTR)CurView->lpFiles[i],len,-1);
        } 
        CurVis = CurView->pVisList1;
        while (CurVis)
        {  
            BigWrite (FidConfig,(HPSTR)CurVis,sizeof(VISLIST),-1);
            CurVis =(LPVISLIST) CurVis->NextVisList;
        } 
        CurVis = CurView->pPickList1;
        while (CurVis)
        {  
            BigWrite (FidConfig,(HPSTR)CurVis,sizeof(VISLIST),-1);
            CurVis =(LPVISLIST) CurVis->NextVisList;
        } 
        if (CurView->pVisListManual)
            BigWrite (FidConfig,(HPSTR)CurView->pVisListManual,sizeof(VISLIST),-1);
        if (CurView->pPickListManual)
            BigWrite (FidConfig,(HPSTR)CurView->pPickListManual,sizeof(VISLIST),-1); 
        if (CurView->hMaskArea && SaveZoom) 
        	WriteMaskArea (FidConfig);
        if (CurView->hBackgroundArea) 
			WriteBackgroundArea (FidConfig);
        if (CurView->pTheme/* && CurView->Type == 7*/)
        {
            CurTheme = CurView->pTheme;
            WriteCurTheme(FidConfig);
        }  
        if (!CurView->lpBoundsDisplay)
        	CurView->BoundsDisplayID = 0;
        if (CurView->BoundsDisplayID)
        {
            BoundsDisplayWrite (CurView->lpBoundsDisplay,FidConfig);
        } 
        CurView->Bitmap = SaveBitmap;
    } 
    
    if (SaveCfgSizePos)
    	SaveWindowPos (FidConfig); 
    if (SaveCfgSizePosAll)
    	SaveAllSizes (FidConfig); 
    if (SaveGlobals)
    	SaveGlobalVals (FidConfig);   
    if (SavePrintSetup)
    	SavePrintSetupData (FidConfig,0);
    if (SaveMenuName)
    	SaveMenusInConfig (FidConfig,1);  
    if (EmbedMenus)
    	EmbedMenusInConfig (FidConfig,TempName);
	SaveToolbarsInConfig (FidConfig);
    if (*TempName)
    	GSSiRemove (TempName);
    if (SaveHLTList)
    	FidConfig = SaveHighlightListToConfig (FidConfig); 
//    if (SaveInfoBox)
//    	SaveInfoBoxes (FidConfig);
    Signature = 28052; 
   	ConfigDesc.Len = 0;
	if (SaveCfgSizePosAll && hSaveCfgImagesFileName)
	{
    	HPSTR	pCfgImage;
    	short	id = OB_CONFIGIMAGES;
		HANDLE	hTempMem;
		LPSTR	pTempName=GlobalLock (hSaveCfgImagesFileName);
        HFILE	FidTemp = GSSiOpenFile (pTempName,0,OF_READ);

		if (FidTemp != HFILE_ERROR)
		{
			CfgImageLen = GSSifilelength (FidTemp);
			if (hConfigDescription)
			{
	    		LPSTR	pConfigDescription = GlobalLock (hConfigDescription);
	    		short	id = OB_CONFIGDESCRIPTION;
	    		
				CfgDescLoc = GSSillseek(FidConfig,0,1);
		 		BigWrite (FidConfig,(HPSTR)&id,2,-1);  
	    		ConfigDesc.Len = _fstrlen (pConfigDescription) + 1;
				BigWrite (FidConfig,(HPSTR)pConfigDescription,ConfigDesc.Len,-1);
				GSSiGlobUlFree (&hConfigDescription);
			}
			else  
	    		CfgDescLoc = 0;
			CfgImageLoc = GSSillseek(FidConfig,0,1);
	 		BigWrite (FidConfig,(HPSTR)&id,2,-1);  
	 		BigWrite (FidConfig,(HPSTR)&CfgImageFormat,2,-1);  
	 		BigWrite (FidConfig,(HPSTR)&CfgImageLen,4,-1); 
			hTempMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,CfgImageLen);
	 		pCfgImage = GlobalLock (hTempMem);
			BigRead (FidTemp,pCfgImage,CfgImageLen);
			GSSiClose2 (&FidTemp);
			GSSiRemove (pTempName);
			GlobalUnlock (hSaveCfgImagesFileName);
	 		BigWrite (FidConfig,(HPSTR)pCfgImage,CfgImageLen,-1); 
			GSSiGlobUlFree (&hTempMem);
	 		BigWrite (FidConfig,(HPSTR)&CfgDescLoc,4,-1); 
	 		BigWrite (FidConfig,(HPSTR)&CfgImageLoc,4,-1); 
			ConfigDesc.HaveImage = 1;    
		}
		GSSiGlobFree (&hSaveCfgImagesFileName);
	}
    else if (SaveCfgImage)   
    {   
    	HPSTR	pCfgImage;
    	short	id = OB_CONFIGIMAGE;
		RECT	WindowRect; 
		long	BMSize;
		int		Flags;
		HANDLE	hWindowDIB;
        
		CheckForContinue(TRUE,0);
		Sleep (500);
		CheckForContinue(TRUE, 0);
	    if (hConfigDescription)
	    {
	    	LPSTR	pConfigDescription = GlobalLock (hConfigDescription);
	    	short	id = OB_CONFIGDESCRIPTION;
	    	
	        CfgDescLoc = GSSillseek(FidConfig,0,1);
		 	BigWrite (FidConfig,(HPSTR)&id,2,-1);  
	    	ConfigDesc.Len = _fstrlen (pConfigDescription) + 1;
		    BigWrite (FidConfig,(HPSTR)pConfigDescription,ConfigDesc.Len,-1);
		    GSSiGlobUlFree (&hConfigDescription);
	    }
	    else  
	    	CfgDescLoc = 0;
        CfgImageLoc = GSSillseek(FidConfig,0,1);
	 	BigWrite (FidConfig,(HPSTR)&id,2,-1);  
    	CfgImageFormat = FIF_JPEG;
    	CfgImageFormat = FIF_TIFF;
		Flags = 0;
		if (CfgImageFormat == FIF_TIFF)
			Flags = TIFF_ADOBE_DEFLATE;
		else if (CfgImageFormat == FIF_JPEG)
			Flags = JPEG_QUALITYGOOD;//JPEG_QUALITYSUPERB;
	 	BigWrite (FidConfig,(HPSTR)&CfgImageFormat,2,-1); 
	 	if (!hWindowDib32)
	 	{ 
			//GetClientRect (hWndMain,&WindowRect);
			WindowRect = MainRect;   
			ClientRectToScreenRect (hWndMain,&WindowRect);
			hWindowDib32 = CopyScreenToDIB32 (&WindowRect); 
//				   		SaveBitmap (hOverViewBitmap,OutFile,FIF_TIFF,TIFF_ADOBE_DEFLATE); 
//			SaveDIB32 (hDib32,"c:\\test.tif",FIF_TIFF,TIFF_ADOBE_DEFLATE);
		}
		hWindowDIB = WriteDIBToMem (hWindowDib32,CfgImageFormat,Flags,&CfgImageLen);
	 	BigWrite (FidConfig,(HPSTR)&CfgImageLen,4,-1); 
	 	pCfgImage = GlobalLock (hWindowDIB); 
		ii = GSSillseek(FidConfig, 0, 1);
	 	BigWrite (FidConfig,(HPSTR)pCfgImage,CfgImageLen,-1); 
	 	BigWrite (FidConfig,(HPSTR)&CfgDescLoc,4,-1); 
	 	BigWrite (FidConfig,(HPSTR)&CfgImageLoc,4,-1); 
		ConfigDesc.HaveImage = 1;    
		GSSiGlobUlFree (&hWindowDIB);
	}
	else
		ConfigDesc.HaveImage = 0;
    GMDestroyDIB32 (hWindowDib32);
	hWindowDib32 = 0;
    if (UseCompression)
    	FidConfig = CompressConfig (FidConfig,Name);   
    BigWrite (FidConfig,(HPSTR)&ConfigDesc,sizeof(ConfigDesc),-1);
    BigWrite (FidConfig,(HPSTR)&Signature,2,-1);
    BigWrite (FidConfig,(HPSTR)&Version,2,-1);
    GSSiClose (FidConfig);  
    CurVis = SaveInVis;     
    CurView = SaveVP;
	ConfigChangesMade = FALSE;
{
#if ENABLETRACE
GSSiExitProg (101);
#endif
    return(TRUE); 
}
#if ENABLETRACE
}
#endif
}
BOOL DisplayConfigPreview (HWND hWnd,HDC hDC,LPSTR Name,LPRECT ImageRect, UINT TextCntl)
{  
	short	Signature, Version;
	long	ii;
	HFILE	FidConfig = GSSiOpenFile (Name,0,OF_READ);
	HFONT	hFont, OldFont;
	char	NoPrevMess[64] = "\rNo Preview Available";
	
	if (FidConfig == HFILE_ERROR)
		return FALSE;
		
    GSSillseek(FidConfig,(LONG)-(6),2);

    BigRead (FidConfig,(HPSTR)&ConfigDesc,2);
    BigRead (FidConfig,(HPSTR)&Signature,2);
    BigRead (FidConfig,(HPSTR)&Version,2);
    if (Signature != 28052 || Version < 5 || Version > CURRENT_CFG_VERSION)
    	goto Exit;
	if (Version > 5)
		FidConfig = DecompressCfgFile (FidConfig,Version);
//    hDC = GetDC (hWnd); 
    if (ConfigDesc.Len)
    {   
    	LPSTR	pConfigDescription; 
    	short	id; 
    	HFONT	hFont, OldFont;
    	
	    GSSiGlobFree (&hConfigDescription);
    	hConfigDescription = GSSiGlobAlloc (1315,GMEM_MOVEABLE,512);
    	pConfigDescription = GlobalLock (hConfigDescription);
	    GSSillseek(FidConfig,(LONG)-(6+4+4),2);
	    GSSilread (FidConfig,&CfgDescLoc,4);
	    GSSillseek(FidConfig,CfgDescLoc,0);
	    GSSilread (FidConfig,&id,2); 
	    GSSilread (FidConfig,pConfigDescription,ConfigDesc.Len);
//		hFont = CreateFont(12, 0, 0, 0, FW_BOLD, 
//	    					0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
//	    OldFont = SelectObject (hDC,hFont);     
		SetDlgItemText (hWnd,TextCntl,pConfigDescription);
//		DrawText (hDC,pConfigDescription,_fstrlen(pConfigDescription),TextRect,DT_LEFT); 
//		SelectObject (hDC,OldFont);
//		DeleteObject (hFont);
//		RestoreDC (hDC,-1);	
	    GlobalUnlock (hConfigDescription);
    }
    
    if (ConfigDesc.HaveImage)   
    {   
    	LPSTR	pCfgImage;  
    	short	id;
		HDIB32 hDib;
    	                               
	   ii=GSSillseek(FidConfig,-(long)(6+4),2);
	   ii= GSSilread (FidConfig,&CfgImageLoc,4); 
	    GSSillseek(FidConfig,CfgImageLoc,0);
	    GSSilread (FidConfig,&id,2); 
	    GSSilread (FidConfig,&CfgImageFormat,2); 
	    GSSilread (FidConfig,&CfgImageLen,4);
		switch (id)
		{
			case OB_CONFIGIMAGES:
				{
					int	i=0, nearimage=0;
					int TotImageLen=0;
					double	NearAspect, neardiff;
					double	ImageAspect = ((double)RECTWIDTH(ImageRect))/RECTHEIGHT(ImageRect);
					
					SaveImagesOffset = GSSillseek (FidConfig,0,1);
					BigRead (FidConfig,(HPSTR)&NumSavedImages,sizeof(int)); 
					BigRead (FidConfig,(HPSTR)&LastBounds,sizeof(MNMXCORD)); 
					BigRead (FidConfig,(HPSTR)SavedImageData,NumSavedImages*sizeof(SAVEDIMAGEDATA)); 
					for (i=0;i<NumSavedImages;i++)
					{
						TotImageLen += SavedImageData[i].ImageLen;
						if (i)
						{
							double Aspect = ((double)RECTWIDTH(&SavedImageData[i].Rect))/RECTHEIGHT(&SavedImageData[i].Rect);
							
							if (fabs (Aspect - ImageAspect) < neardiff)
							{
								nearimage = i;
								NearAspect = Aspect;
								neardiff = fabs (Aspect - ImageAspect);
							}
						}
						else
						{
							NearAspect = ((double)RECTWIDTH(&SavedImageData[i].Rect))/RECTHEIGHT(&SavedImageData[i].Rect);
							neardiff = fabs (NearAspect - ImageAspect);
						}
					}
					hCfgImage = GSSiGlobAlloc (1534,GMEM_MOVEABLE,TotImageLen);
					pCfgImage = GlobalLock (hCfgImage); 
					BigRead (FidConfig,(HPSTR)pCfgImage,TotImageLen);  
					CfgImageLen = SavedImageData[nearimage].ImageLen;
					pCfgImage += SavedImageData[nearimage].Offset;
					hDib = LoadDIBFromMem (pCfgImage,CfgImageLen,CfgImageFormat,0);
					GSSiGlobUlFree (&hCfgImage);
					DisplayBMInRect32 (hDC,hDib,  *ImageRect,TRUE);
				}
				break;
			default:
				if (CfgImageLen)
				{ 

					hCfgImage = GSSiGlobAlloc (1544,GMEM_MOVEABLE,CfgImageLen);
					pCfgImage = GlobalLock (hCfgImage); 
					BigRead (FidConfig,(HPSTR)pCfgImage,CfgImageLen);  
					if (CfgImageFormat != 1)
					{
						hDib = LoadDIBFromMem (pCfgImage,CfgImageLen,CfgImageFormat,0);
						GSSiGlobUlFree (&hCfgImage);
						DisplayBMInRect32 (hDC,hDib,  *ImageRect,TRUE);
	   					//DisplayBMInRect2 (hDC,hDib,  *ImageRect,-1,0,0);
					}
					else
					{
						GlobalUnlock (hCfgImage); 
   						DisplayBMInRect2 (hDC,hCfgImage, *ImageRect,0,0,0,0);
						GSSiGlobFree (&hCfgImage);
					}
				break;
			}
		}
	}
	else
	{
		SaveDC (hDC);
		SetTextColor (hDC,RGB(200,200,200));
		SetBkMode (hDC,TRANSPARENT);
		hFont = CreateFont((ImageRect->bottom-ImageRect->top)/4, 0, 0, 0, FW_BOLD, 
		    				0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
		OldFont = SelectObject (hDC,hFont);     
		DrawText (hDC,NoPrevMess,-1,ImageRect,DT_CENTER|DT_VCENTER|DT_WORDBREAK); 
		SelectObject (hDC,OldFont);
		DeleteObject (hFont);
		RestoreDC (hDC,-1);	
	}
//	ReleaseDC (hWnd,hDC);
Exit:
	GSSiClose2 (&FidConfig);
	return TRUE;
}

BOOL FindConfigFile (HWND hWnd)
{
#if WIN32
	DialogBox(hInst, (LPSTR)"CONFIGLIST_WIDE", hWnd, (DLGPROC)CONFIGLISTMsgProc);
#else
          FARPROC lpfnCONFIGLISTMsgProc;
          lpfnCONFIGLISTMsgProc = MakeProcInstance((FARPROC)CONFIGLISTMsgProc, hInst);
          DialogBox(hInst, (LPSTR)"CONFIGLIST_WIDE", hWnd, lpfnCONFIGLISTMsgProc);
          FreeProcInstance(lpfnCONFIGLISTMsgProc);
#endif 
	return TRUE;
}

NEWOBJECT ReadNewObject16 (HFILE Fid)
{
	NEWOBJECT16	NewObject16;
	NEWOBJECT	NewObject;

	BigRead (Fid,(HPSTR)&NewObject16,sizeof(NEWOBJECT16)); 
	NewObject = NewObject16_to_NewObject (&NewObject16);
	return NewObject;
}

NEWOBJECT ReadNewObject_V0(HFILE Fid)
{
	NEWOBJECT_v0	OldNewObject;
	NEWOBJECT	NewObject;

	BigRead (Fid,(HPSTR)&OldNewObject,sizeof(NEWOBJECT_v0)); 
	NewObject.Type =OldNewObject.Type;
	NewObject.Style =OldNewObject.Style;
	NewObject.R =OldNewObject.R;
	NewObject.G =OldNewObject.G;
	NewObject.B =OldNewObject.B;
	NewObject.Width =OldNewObject.Width; 
	NewObject.Handle=0;  
	return NewObject;
}
int	CreateTestConfig (void)
{	short		Signature, Version=4;
	short		len, i;
	OFSTRUCTGM	OFStruct;
	char	File[128];
	HFILE		FidConfig;
	char	CfgName[256];
	COLORREF	WindowColor;
	short		CommandVpt; 
	HANDLE	hVisList=NULL;
	short		NumMenuMask, MenuMask;
	char	StartupCommand[256]="", StartupMenu[128]="";
	LPVIEWPORT_V6	CurView6;
	
	InfoBoxInit (&TAGBox);
	
                  
	_fstrcpy (CfgName,"basic1.gmc");
	GSSiRemove (CfgName);
	FidConfig = GSSiOpenFile (CfgName,(LPOFSTRUCTGM)&OFStruct,OF_CREATE); 
	
    BigWrite (FidConfig,(HPSTR)StartupCommand,sizeof(StartupCommand),-1);
    BigWrite (FidConfig,(HPSTR)StartupMenu,sizeof(StartupMenu),-1);
    BigWrite (FidConfig,(HPSTR)&NumIB,sizeof(NumIB),-1);  
	
	*pNumViewports = 4;
	CommandVpt = 1;
	WindowColor = RGB(239,239,239);
    BigWrite (FidConfig,(HPSTR)pNumViewports,2,-1);
    BigWrite (FidConfig,(HPSTR)&CommandVpt,2,-1);
	NumMenuMask=0;
    BigWrite (FidConfig,(HPSTR)&NumMenuMask,sizeof(NumMenuMask),-1);
    BigWrite (FidConfig,(HPSTR)&WindowColor,4,-1);
   	BigWrite (FidConfig,(HPSTR)&TAGBox,sizeof(TAGBOX),-1);

	hViewports[0]=GSSiGlobAlloc ( 128,GHND,sizeof(VIEWPORT_V6));
	CurView6 = (LPVIEWPORT_V6)GlobalLock (hViewports[0]);
	
	CurView6->ID = 1;  
	_fstrcpy(CurView6->Name,"Primary Viewport");
	CurView6->Version = Version;
	CurView6->Parent = 0;
	CurView6->Type = 1;
	CurView6->Active = TRUE; 
	CurView6->DesiredHeight = 0;
	CurView6->DesiredWidth = 0;
	CurView6->BackGroundColor = RGB(255,255,255);
	CurView6->Shadow = FALSE;
	CurView6->Margin = 2;
	CurView6->MarginPan=TRUE;
	CurView6->TagPointID = 1;
	CurView6->TagPointType = 2;
	CurView6->TagPoint.x = 1;
	CurView6->TagPoint.y = 1;
	CurView6->WidthType = 2;
	CurView6->Width = 98;
	CurView6->Height = 98;
	CurView6->NewBounds.xmn = -10000;
	CurView6->NewBounds.xmx =  10000;
	CurView6->NewBounds.ymn = -10000;
	CurView6->NewBounds.ymx =  10000;
	CurView6->HaveBounds = FALSE;
    CurView6->WindowIsZoomed = FALSE; 
    CurView6->BoundsDisplayID = 1;
    CurView6->lpBoundsDisplay = BoundsDisplayInit (1,2,1);
	_fstrcpy (CurView6->PickMacroFile,"pikmacro.txt");
    CurView6->pTheme = 0;
	CurView6->NumThemes = 0;
	CurView6->NumFiles = 1;
	CurView6->FileType[0]=2;
	CurView6->lpFiles[0]=0;
	CurView6->NumVisList = 1;
	
	GSSiGlobUlFree (&hVisList);
	hVisList=GSSiGlobAlloc ( 129,GHND,sizeof(VISLIST));
	CurVis = (LPVISLIST)GlobalLock (hVisList); 
	CurVis->hVisList=hVisList;
	InitVis ();

	CurView6->StartupFunction=GF_PAN_TO_POINT;
	_fstrcpy(CurView6->FunctionFile,"fundir\\appl1.txt");
	_fstrcpy(CurView6->FunctionDir,"index.txt");
	
	
	i=BigWrite (FidConfig,(HPSTR)CurView6,sizeof(VIEWPORT_V6),-1);
	_fstrcpy (File,"[%PLOT]");
	len = _fstrlen (File)+1;
	BigWrite (FidConfig,(HPSTR)&len,2,-1);
	BigWrite (FidConfig,(HPSTR)&File,len,-1);
	
	BigWrite (FidConfig,(HPSTR)CurVis,sizeof(VISLIST),-1); 
	
	if (CurView6->BoundsDisplayID)
		BoundsDisplayWrite (CurView6->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView6->lpBoundsDisplay); 
	

	CurView6->ID = 2;
	CurView6->Active = FALSE; 
	_fstrcpy(CurView6->Name,"Index Map");
	CurView6->Parent = 0;
	CurView6->Type = 7;
	CurView6->ZoomTarget = 1;
	CurView6->Bitmap = 0;
	CurView6->DesiredHeight = 0;
	CurView6->DesiredWidth = 0;
	CurView6->BackGroundColor = RGB(255,255,255);
	CurView6->Shadow = TRUE; 
	CurView6->Margin = 0;
	CurView6->TagPointID = 3;
	CurView6->TagPointType = 2;
	CurView6->TagPoint.x = 2;
	CurView6->TagPoint.y = 2;
	CurView6->WidthType = 2;
	CurView6->Width = 22;
	CurView6->Height = 47;
	CurView6->NewBounds.xmn = -10000;
	CurView6->NewBounds.xmx =  10000;
	CurView6->NewBounds.ymn = -10000;
	CurView6->NewBounds.ymx =  10000;
	CurView6->HaveBounds = FALSE;
    CurView6->WindowIsZoomed = FALSE; 
    CurView6->BoundsDisplayID = 0;
    CurView6->lpBoundsDisplay = NULL;
    CurView6->pTheme = 0;
	CurView6->NumThemes = 0;
	CurView6->NumFiles = 1;
	CurView6->FileType[0]=2;
	CurView6->lpFiles[0]=0;
	CurView6->NumVisList = 1;
	_fstrcpy (CurView6->VisName,"vislists\\index.vis");
	
	GSSiGlobUlFree (&hVisList);
	hVisList=GSSiGlobAlloc ( 130,GHND,sizeof(VISLIST));
	CurVis = (LPVISLIST)GlobalLock (hVisList); 
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	CurVis->WantType[1]=0;
	CurVis->WantType[2]=0;

	
	CurView6->StartupFunction=GF_PAN_ZOOM_TARGET;
	_fstrcpy(CurView6->FunctionFile,"fundir\\viewport.txt");
	_fstrcpy(CurView6->FunctionDir,"index.txt");
	
	
	i=BigWrite (FidConfig,(HPSTR)CurView6,sizeof(VIEWPORT_V6),-1);
	_fstrcpy (File,"\\newbase.plt");
	_fstrcpy (File,"[%PLOT]");

	len = _fstrlen (File)+1;
	BigWrite (FidConfig,(HPSTR)&len,2,-1);
	BigWrite (FidConfig,(HPSTR)&File,len,-1);
	
	BigWrite (FidConfig,(HPSTR)CurVis,sizeof(VISLIST),-1); 
	
	if (CurView6->BoundsDisplayID)
		BoundsDisplayWrite (CurView6->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView6->lpBoundsDisplay);
	
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GSSiGlobFree (&handle);
    	handle=NULL;
    }

	handle=GSSiGlobAlloc ( 131,GHND,sizeof(THEME));
	CurTheme = (LPTHEME)GlobalLock(handle);
	CurView6->pTheme = CurTheme;
	CurTheme->ID=GF_SINGLE_VALUE_THEME; 
	CurTheme->Version = 1;
	CurTheme->TargetViewport=1;
	CurTheme->DisplayViewport=CurView6->ID+1;
	_fstrcpy (CurTheme->DataFile,"attribut\\pid02.gmd");
	_fstrcpy (CurTheme->SQL,"REFNO=[%INT_REFNO]");
	_fstrcpy (CurTheme->Field.name,"MKT_VAL_200");
	_fstrcpy (CurTheme->Field.name,"BLDG_MKT");
	_fstrcpy (CurTheme->Contents,"PARCEL");
	CurTheme->SymNum=99;
	CurTheme->DataFileType = UMIFS_DATAFILE;
   	CurTheme->IsActive = FALSE;
	CurTheme->WantDataPass=FALSE;
	CurTheme->ComputeClassBoundaries=TRUE;
	CurTheme->DisplayScatterDiagram=TRUE;
	CurTheme->NumDesiredClass=5;

	CurTheme->ClassType=1;
	CurTheme->XLimit = 500000;
	CurTheme->YLimit = 500000;
	CurTheme->ClassColor[0]=RGB(50,150,250);
	CurTheme->ClassColor[1]=RGB(150,250,250);
	CurTheme->ClassColor[2]=RGB(250,150,250);
	CurTheme->ClassColor[3]=RGB(250,50,150);
	CurTheme->ClassColor[4]=RGB(50,250,150);
	CurTheme->YLimit = LONG_MAX;
	CurTheme->Margin = 2;
	CurTheme->ScatterWidth=15;
	CurTheme->ColorsWidth=15;
	CurTheme->InnerMargin=2;
	CurTheme->TitleHeight=15;
	CurTheme->BGColor=RGB(255,255,255);
	CurTheme->ScatterColor=RGB(0,0,0);
	CurTheme->ScatterBoxBG=RGB(255,255,255);
	CurTheme->TitleBoxBG=RGB(255,255,255);
	_fstrcpy (CurTheme->Title,"Market Value");
	_fstrcpy (CurTheme->TitleFont.lfFaceName,"Arial Rounded MT Bold");
	_fstrcpy (CurTheme->ClassFont1.lfFaceName,"Arial");
	_fstrcpy (CurTheme->ClassFont2.lfFaceName,"Arial");


	CurView6->ID = 3;
	CurView6->Active = FALSE;
	_fstrcpy(CurView6->Name,"Legend");
	CurView6->Parent = 0;
	CurView6->Type = 1; 
	CurView6->ZoomTarget = 0;
	CurView6->DesiredHeight = 0;
	CurView6->DesiredWidth = 0;
	CurView6->BackGroundColor = RGB(225,225,225);
	CurView6->Shadow = TRUE;
	CurView6->TagPointID = 4;
	CurView6->TagPointType = 2;
	CurView6->TagPoint.x = 2;
	CurView6->TagPoint.y = 2;
	CurView6->WidthType = 2;
	CurView6->Width = 22;
	CurView6->Height = 47;
	CurView6->NewBounds.xmn = -10000;
	CurView6->NewBounds.xmx =  10000;
	CurView6->NewBounds.ymn = -10000;
	CurView6->NewBounds.ymx =  10000;
    CurView6->BoundsDisplayID = 0;
    CurView6->lpBoundsDisplay = NULL;
	CurView6->HaveBounds = FALSE;
    CurView6->WindowIsZoomed = FALSE;
	CurView6->NumThemes = 0;
	CurView6->NumFiles = 0;
	CurView6->FileType[0]=2;
	CurView6->lpFiles[0]=0;
	CurView6->NumVisList = 1;
	CurView6->VisName[0]='\0';
	
	GSSiGlobUlFree (&hVisList);
	hVisList=GSSiGlobAlloc ( 132,GHND,sizeof(VISLIST));
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	
	CurView6->CurrentFunction=0;
	_fstrcpy(CurView6->FunctionFile,"fundir\\viewport.txt");
	_fstrcpy(CurView6->FunctionDir,"index.txt");
	
	
	i=BigWrite (FidConfig,(HPSTR)CurView6,sizeof(VIEWPORT_V6),-1);
	
	BigWrite (FidConfig,(HPSTR)CurVis,sizeof(VISLIST),-1);
	
	BigWrite (FidConfig,(HPSTR)CurTheme,sizeof(THEME),-1);
	
	if (CurView6->BoundsDisplayID) BoundsDisplayWrite (CurView6->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView6->lpBoundsDisplay);


	CurView6->ID = 4;
	CurView6->Parent = 0;
	CurView6->Type = 1; 
	CurView6->Active = FALSE; 
	_fstrcpy(CurView6->Name,"Coordinate Display");
	CurView6->DesiredHeight = 0;
	CurView6->DesiredWidth = 0;
	CurView6->BackGroundColor = RGB(255,255,255);
	CurView6->Shadow = TRUE;
	CurView6->TagPointID = 1;
	CurView6->TagPointType = 2;
	CurView6->TagPoint.x = 3;
	CurView6->TagPoint.y = 3;
	CurView6->WidthType = 2;
	CurView6->Width = 30;
	CurView6->Height = 5;
	CurView6->NewBounds.xmn = -10000;
	CurView6->NewBounds.xmx =  10000;
	CurView6->NewBounds.ymn = -10000;
	CurView6->NewBounds.ymx =  10000;
    CurView6->BoundsDisplayID = 0;
    CurView6->lpBoundsDisplay = NULL;
	CurView6->HaveBounds = FALSE;
    CurView6->WindowIsZoomed = FALSE;
	CurView6->NumThemes = 0;
	CurView6->NumFiles = 0;
	CurView6->NumVisList = 0;
	
	CurView6->CurrentFunction=0;
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GSSiGlobFree (&handle);
    	handle=NULL;
    }

	handle=GSSiGlobAlloc ( 133,GHND,sizeof(COORDINATEDISPLAY));
	CD = (LPCOORDINATEDISPLAY)GlobalLock(handle);
	CurView6->pTheme = (LPTHEME)CD; 
	CD->Version = 103;
	CD->TargetViewport=1;
	CD->DisplayViewport=4;
	CD->ID = PF_COORD_DISPLAY;
	CD->DisplayLine[0] = TRUE;
	CD->Units[0] = 1;
	CD->Precision[0] = 3;
	CD->Commas[0] = TRUE;

	i=BigWrite (FidConfig,(HPSTR)CurView6,sizeof(VIEWPORT_V6),-1);
	
	BigWrite (FidConfig,(HPSTR)CD,sizeof(COORDINATEDISPLAY),-1);
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GSSiGlobFree (&handle);
    	handle=NULL;
    }
	
	
	
	Signature = 28052;
    BigWrite (FidConfig,(HPSTR)&Signature,2,-1);
    BigWrite (FidConfig,(HPSTR)&Version,2,-1);
    
	GlobalUnlock (hViewports[0]);
	GSSiGlobFree (&hViewports[0]);	
    GSSiClose2 (&FidConfig);
    FidConfig = 0;

                  

	_fstrcpy (CfgName,"basic2.gmc");
	GSSiRemove (CfgName);
	FidConfig = GSSiOpenFile (CfgName,(LPOFSTRUCTGM)&OFStruct,OF_CREATE);
    BigWrite (FidConfig,(HPSTR)StartupCommand,sizeof(StartupCommand),-1);
    BigWrite (FidConfig,(HPSTR)StartupMenu,sizeof(StartupMenu),-1);
    BigWrite (FidConfig,(HPSTR)&NumIB,sizeof(NumIB),-1);  
	*pNumViewports = 4; 
	CommandVpt = 1;
	WindowColor = RGB(239,239,239);
    BigWrite (FidConfig,(HPSTR)pNumViewports,2,-1);
    BigWrite (FidConfig,(HPSTR)&CommandVpt,2,-1);
    
    NumMenuMask=0;
    BigWrite (FidConfig,(HPSTR)&NumMenuMask,sizeof(NumMenuMask),-1);
    BigWrite (FidConfig,(HPSTR)&WindowColor,4,-1);
   	BigWrite (FidConfig,(HPSTR)&TAGBox,sizeof(TAGBOX),-1);
	hViewports[0]=GSSiGlobAlloc ( 134,GHND,sizeof(VIEWPORT_V6));
	CurView6 = (LPVIEWPORT_V6)GlobalLock (hViewports[0]);
	
	CurView6->ID = 1;
	CurView6->Version = Version;
	_fstrcpy(CurView6->Name,"Primary Viewport");
	CurView6->Active = TRUE;
	CurView6->Parent = 0;
	CurView6->Type = 1; 
	CurView6->DesiredHeight = 0;
	CurView6->DesiredWidth = 0;
	CurView6->BackGroundColor = RGB(255,255,255);
	CurView6->Shadow = FALSE;  
	CurView6->MarginPan=TRUE;
	CurView6->Margin = 2;
	CurView6->TagPointID = 1;
	CurView6->TagPointType = 2;
	CurView6->TagPoint.x = 0;
	CurView6->TagPoint.y = 0;
	CurView6->WidthType = 2;
	CurView6->Width =100;
	CurView6->Height = 100;
	CurView6->NewBounds.xmn = -10000;
	CurView6->NewBounds.xmx =  10000;
	CurView6->NewBounds.ymn = -10000;
	CurView6->NewBounds.ymx =  10000;
	CurView6->HaveBounds = FALSE;
    CurView6->WindowIsZoomed = FALSE; 
    CurView6->BoundsDisplayID = 1;
    CurView6->lpBoundsDisplay = BoundsDisplayInit (1,2,1);
    CurView6->pTheme = 0;
	CurView6->NumThemes = 0;
	CurView6->NumFiles = 5;
	CurView6->FileType[0]=2;
	CurView6->lpFiles[0]=0; 
	_fstrcpy (CurView6->FileID[0],"City Base");
	CurView6->FileType[1]=4;
	CurView6->lpFiles[1]=0;
	_fstrcpy (CurView6->FileID[1],"Parcel Base");
	CurView6->FileType[2]=4;
	CurView6->lpFiles[2]=0;
	_fstrcpy (CurView6->FileID[2],"Parcel Text");
	CurView6->FileType[3]=4;
	CurView6->lpFiles[3]=0;
	_fstrcpy (CurView6->FileID[3],"Features");
	CurView6->FileType[4]=4;
	CurView6->lpFiles[4]=0;
	_fstrcpy (CurView6->FileID[4],"Topo");
	CurView6->NumVisList = 2;
	_fstrcpy (CurView6->VisName,"vislists\\[%CONFIG].vis");
	_fstrcpy (CurView6->PickName,"piklists\\[%CONFIG].pik");
	_fstrcpy (CurView6->DisplayRedefFile,"[%CONFIG].rdf");
	_fstrcpy (CurView6->PickMacroFile,"pikmacro.txt");
	
	CurView6->StartupFunction=GF_PAN_TO_POINT;
	_fstrcpy(CurView6->FunctionFile,"fundir\\appl1.txt");
	_fstrcpy(CurView6->FunctionDir,"index.txt");
	i=BigWrite (FidConfig,(HPSTR)CurView6,sizeof(VIEWPORT_V6),-1);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\basemap.plt");
	len = _fstrlen (File)+1;
	BigWrite (FidConfig,(HPSTR)&len,2,-1);
	BigWrite (FidConfig,(HPSTR)&File,len,-1);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\baselayr\\index");
	len = _fstrlen (File)+1;
	BigWrite (FidConfig,(HPSTR)&len,2,-1);
	BigWrite (FidConfig,(HPSTR)&File,len,-1); 
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\textlayr\\index");
	len = _fstrlen (File)+1;
	BigWrite (FidConfig,(HPSTR)&len,2,-1);
	BigWrite (FidConfig,(HPSTR)&File,len,-1);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\features\\index");
	len = _fstrlen (File)+1;
	BigWrite (FidConfig,(HPSTR)&len,2,-1);
	BigWrite (FidConfig,(HPSTR)&File,len,-1);
	
	_fstrcpy (File,"[%DATA_LOC]maplib\\topolayr\\index");
	len = _fstrlen (File)+1;
	BigWrite (FidConfig,(HPSTR)&len,2,-1);
	BigWrite (FidConfig,(HPSTR)&File,len,-1);
	
	
	GSSiGlobUlFree (&hVisList);
	hVisList=GSSiGlobAlloc ( 135,GHND,sizeof(VISLIST));
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0; 
	CurVis->MinPointSize = 6; 
	CurVis->MaxScale=10.0;
	CurVis->FileIsVisible[0]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	BigWrite (FidConfig,(HPSTR)CurVis,sizeof(VISLIST),-1);
	
	InitVis ();
	CurVis->MinScale=10.0;
	CurVis->FileIsVisible[2]=FALSE;
	CurVis->FileIsVisible[3]=FALSE;
	CurVis->FileIsVisible[4]=FALSE;
	
	BigWrite (FidConfig,(HPSTR)CurVis,sizeof(VISLIST),-1);

	if (CurView6->BoundsDisplayID) BoundsDisplayWrite (CurView6->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView6->lpBoundsDisplay);

	CurView6->ID = 2;
	CurView6->Active = TRUE; 
	_fstrcpy(CurView6->Name,"Index Map");
	CurView6->Parent = 0;
	CurView6->Type = 7;
	CurView6->ZoomTarget = 1;
	CurView6->Bitmap = 0;
	CurView6->DesiredHeight = 0;
	CurView6->DesiredWidth = 0;
	CurView6->BackGroundColor = RGB(255,255,255);
	CurView6->Shadow = TRUE; 
	CurView6->Margin = 1.5;   
	CurView6->MarginPan = FALSE;
	CurView6->TagPointID = 3;
	CurView6->TagPointType = 2;
	CurView6->TagPoint.x = 2;
	CurView6->TagPoint.y = 2;
	CurView6->WidthType = 2;
	CurView6->Width = 22;
	CurView6->Height = 47;
	CurView6->NewBounds.xmn = -10000;
	CurView6->NewBounds.xmx =  10000;
	CurView6->NewBounds.ymn = -10000;
	CurView6->NewBounds.ymx =  10000;
	CurView6->HaveBounds = FALSE;
    CurView6->WindowIsZoomed = FALSE; 
    CurView6->BoundsDisplayID = 0;
    CurView6->lpBoundsDisplay = NULL;
    CurView6->pTheme = 0;
	CurView6->NumThemes = 0;
	CurView6->NumFiles = 1;
	CurView6->FileType[0]=2;
	CurView6->lpFiles[0]=0;
	CurView6->NumVisList = 1;
	_fstrcpy (CurView6->VisName,"vislists\\index.vis");
	CurView6->DisplayRedefFile[0]='\0';	
	_fstrcpy (CurView6->PickMacroFile,"pikmacro.txt");
	CurView6->PickMacroFile[0]=0;
	GSSiGlobUlFree (&hVisList);
	hVisList=GSSiGlobAlloc ( 136,GHND,sizeof(VISLIST));
	CurVis = (LPVISLIST)GlobalLock (hVisList); 
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	CurVis->WantType[1]=0;
	CurVis->WantType[2]=0;

	
	CurView6->StartupFunction=GF_PAN_ZOOM_TARGET;
	_fstrcpy(CurView6->FunctionFile,"fundir\\viewport.txt");
	_fstrcpy(CurView6->FunctionDir,"index.txt");
	
	
	i=BigWrite (FidConfig,(HPSTR)CurView6,sizeof(VIEWPORT_V6),-1);
	_fstrcpy (File,"\\newbase.plt");
	_fstrcpy (File,"[%DATA_LOC]maplib\\index.plt");

	len = _fstrlen (File)+1;
	BigWrite (FidConfig,(HPSTR)&len,2,-1);
	BigWrite (FidConfig,(HPSTR)&File,len,-1);
	
	BigWrite (FidConfig,(HPSTR)CurVis,sizeof(VISLIST),-1); 
	
	if (CurView6->BoundsDisplayID) BoundsDisplayWrite (CurView6->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView6->lpBoundsDisplay);
	
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GSSiGlobFree (&handle);
    	handle=NULL;
    }
	handle=GSSiGlobAlloc ( 137,GHND,sizeof(THEME));
	CurTheme = (LPTHEME)GlobalLock(handle);
	CurView6->pTheme = CurTheme;
	CurTheme->ID=GF_SINGLE_VALUE_THEME; 
	CurTheme->Version = 1;
	CurTheme->TargetViewport=1;
	CurTheme->DisplayViewport=CurView6->ID+1;
	_fstrcpy (CurTheme->DataFile,"[%DATA_LOC]attribut\\pid02.gmd");
	_fstrcpy (CurTheme->SQL,"REFNO=[%INT_REFNO]");
	_fstrcpy (CurTheme->Field.name,"MKT_VAL_200");
	_fstrcpy (CurTheme->Field.name,"BLDG_MKT");
	_fstrcpy (CurTheme->Contents,"PARCEL");
	CurTheme->SymNum=99;
	CurTheme->DataFileType = UMIFS_DATAFILE;
   	CurTheme->IsActive = FALSE;
	CurTheme->WantDataPass=FALSE;
	CurTheme->ComputeClassBoundaries=TRUE;
	CurTheme->DisplayScatterDiagram=TRUE;
	CurTheme->NumDesiredClass=5;

	CurTheme->ClassType=1;
	CurTheme->XLimit = 500000;
	CurTheme->YLimit = 500000;
	CurTheme->ClassColor[0]=RGB(50,150,250);
	CurTheme->ClassColor[1]=RGB(150,250,250);
	CurTheme->ClassColor[2]=RGB(250,150,250);
	CurTheme->ClassColor[3]=RGB(250,50,150);
	CurTheme->ClassColor[4]=RGB(50,250,150);
	CurTheme->YLimit = LONG_MAX;
	CurTheme->Margin = 0;
	CurTheme->ScatterWidth=15;
	CurTheme->ColorsWidth=15;
	CurTheme->InnerMargin=2;
	CurTheme->TitleHeight=15;
	CurTheme->BGColor=RGB(255,255,255);
	CurTheme->ScatterColor=RGB(0,0,0);
	CurTheme->ScatterBoxBG=RGB(255,255,255);
	CurTheme->TitleBoxBG=RGB(255,255,255);
	_fstrcpy (CurTheme->Title,"Market Value");
	_fstrcpy (CurTheme->TitleFont.lfFaceName,"Arial Rounded MT Bold");
	_fstrcpy (CurTheme->ClassFont1.lfFaceName,"Arial");
	_fstrcpy (CurTheme->ClassFont2.lfFaceName,"Arial");


	CurView6->ID = 3;
	CurView6->Active = TRUE;
	_fstrcpy(CurView6->Name,"Legend");
	CurView6->Parent = 0;
	CurView6->Type = 1; 
	CurView6->ZoomTarget = 0;
	CurView6->DesiredHeight = 0;
	CurView6->DesiredWidth = 0;
	CurView6->BackGroundColor = RGB(225,225,225);
	CurView6->Shadow = TRUE;
	CurView6->TagPointID = 4;
	CurView6->TagPointType = 2;
	CurView6->TagPoint.x = 2;
	CurView6->TagPoint.y = 2;
	CurView6->WidthType = 2;
	CurView6->Width = 22;
	CurView6->Height = 47;
	CurView6->NewBounds.xmn = -10000;
	CurView6->NewBounds.xmx =  10000;
	CurView6->NewBounds.ymn = -10000;
	CurView6->NewBounds.ymx =  10000;
    CurView6->BoundsDisplayID = 0;
    CurView6->lpBoundsDisplay = NULL;
	CurView6->HaveBounds = FALSE;
    CurView6->WindowIsZoomed = FALSE;
	CurView6->NumThemes = 0;
	CurView6->NumFiles = 0;
	CurView6->FileType[0]=2;
	CurView6->lpFiles[0]=0;
	CurView6->NumVisList = 1;
	CurView6->VisName[0]='\0';
	_fstrcpy (CurView6->PickMacroFile,"pikmacro.txt");
	CurView6->PickMacroFile[0]=0;
	
	GSSiGlobUlFree (&hVisList);
	hVisList=GSSiGlobAlloc ( 138,GHND,sizeof(VISLIST));
	CurVis = (LPVISLIST)GlobalLock (hVisList);
	CurVis->hVisList=hVisList;
	InitVis ();
	CurVis->LastVisList=0;
	CurVis->NextVisList=0;
	
	CurView6->StartupFunction=0;
	_fstrcpy(CurView6->FunctionFile,"fundir\\themes.txt");
	_fstrcpy(CurView6->FunctionDir,"index.txt");
	
	
	i=BigWrite (FidConfig,(HPSTR)CurView6,sizeof(VIEWPORT_V6),-1);
	
	BigWrite (FidConfig,(HPSTR)CurVis,sizeof(VISLIST),-1);
	
	BigWrite (FidConfig,(HPSTR)CurTheme,sizeof(THEME),-1);
	
	if (CurView6->BoundsDisplayID) BoundsDisplayWrite (CurView6->lpBoundsDisplay,FidConfig);
	BoundsDisplayDestroy (CurView6->lpBoundsDisplay);


	CurView6->ID = 4;
	CurView6->Parent = 0;
	CurView6->Type = 1; 
	CurView6->Active = FALSE; 
	_fstrcpy(CurView6->Name,"Coordinate Display");
	CurView6->DesiredHeight = 0;
	CurView6->DesiredWidth = 0;
	CurView6->BackGroundColor = RGB(255,255,255);
	CurView6->Shadow = TRUE;
	CurView6->TagPointID = 1;
	CurView6->TagPointType = 2;
	CurView6->TagPoint.x = 3;
	CurView6->TagPoint.y = 3;
	CurView6->WidthType = 2;
	CurView6->Width = 30;
	CurView6->Height = 5;
	CurView6->NewBounds.xmn = -10000;
	CurView6->NewBounds.xmx =  10000;
	CurView6->NewBounds.ymn = -10000;
	CurView6->NewBounds.ymx =  10000;
    CurView6->BoundsDisplayID = 0;
    CurView6->lpBoundsDisplay = NULL;
	CurView6->HaveBounds = FALSE;
    CurView6->WindowIsZoomed = FALSE;
	CurView6->NumThemes = 0;
	CurView6->NumFiles = 0;
	CurView6->NumVisList = 0;
	_fstrcpy (CurView6->PickMacroFile,"pikmacro.txt");
	CurView6->PickMacroFile[0]=0;
	
	CurView6->CurrentFunction=0;
	
    if (handle)
    {
    	GlobalUnlock(handle);
    	GSSiGlobFree (&handle);
    	handle=NULL;
    }
	handle=GSSiGlobAlloc ( 139,GHND,sizeof(COORDINATEDISPLAY));
	CD = (LPCOORDINATEDISPLAY)GlobalLock(handle);
	CurView6->pTheme = (LPTHEME)CD;
	CD->Version = 103;
	CD->TargetViewport=1;
	CD->DisplayViewport=4;
	CD->ID = PF_COORD_DISPLAY;
	CD->DisplayLine[0] = TRUE;
	CD->Units[0] = 1;
	CD->Precision[0] = 3;
	CD->Commas[0] = TRUE;

	i=BigWrite (FidConfig,(HPSTR)CurView6,sizeof(VIEWPORT_V6),-1);
	
	BigWrite (FidConfig,(HPSTR)CD,sizeof(COORDINATEDISPLAY),-1);
	

    GSSiGlobUlFree (&handle);
	
	Signature = 28052;
    BigWrite (FidConfig,(HPSTR)&Signature,2,-1);
    BigWrite (FidConfig,(HPSTR)&Version,2,-1);
    
	GSSiGlobUlFree (&hViewports[0]);
    GSSiClose2 (&FidConfig);  
    FidConfig = 0;
	GSSiGlobUlFree (&hVisList);

	_fstrcpy (CfgName,"formats\\format1.gmc");
	makedirectories (CfgName,FALSE,FALSE);
	GSSiRemove (CfgName);
	FidConfig = GSSiOpenFile (CfgName,(LPOFSTRUCTGM)&OFStruct,OF_CREATE);
    BigWrite (FidConfig,(HPSTR)StartupCommand,sizeof(StartupCommand),-1);
    BigWrite (FidConfig,(HPSTR)StartupMenu,sizeof(StartupMenu),-1);
    BigWrite (FidConfig,(HPSTR)&NumIB,sizeof(NumIB),-1);  
	*pNumViewports = 1; 
	CommandVpt = 1;
	WindowColor = RGB(239,239,239);
    BigWrite (FidConfig,(HPSTR)pNumViewports,2,-1);
    BigWrite (FidConfig,(HPSTR)&CommandVpt,2,-1);
    
    NumMenuMask=0;
    BigWrite (FidConfig,(HPSTR)&NumMenuMask,sizeof(NumMenuMask),-1);
    BigWrite (FidConfig,(HPSTR)&WindowColor,4,-1);
   	BigWrite (FidConfig,(HPSTR)&TAGBox,sizeof(TAGBOX),-1);
	hViewports[0]=GSSiGlobAlloc ( 140,GHND,sizeof(VIEWPORT_V6));
	CurView6 = (LPVIEWPORT_V6)GlobalLock (hViewports[0]);
	
	CurView6->ID = 1;
	CurView6->Version = Version;
	_fstrcpy(CurView6->Name,"Format");
	CurView6->Active = TRUE;
	CurView6->Parent = 0;
	CurView6->Type = 1; 
	CurView6->DesiredHeight = 8;
	CurView6->DesiredWidth = 10.5;
	CurView6->BackGroundColor = RGB(255,255,255);
	CurView6->Shadow = FALSE;  
	CurView6->MarginPan=FALSE;
	CurView6->Margin = 0;
	CurView6->BorderPct = -1;
	CurView6->TagPointID = 1;
	CurView6->TagPointType = 2;
	CurView6->TagPoint.x = 0;
	CurView6->TagPoint.y = 0;
	CurView6->WidthType = 1; 
	CurView6->DisplayInInches = TRUE;
	CurView6->Width =8;
	CurView6->Height = 10.5;
	CurView6->NewBounds.xmn = -10000;
	CurView6->NewBounds.xmx =  10000;
	CurView6->NewBounds.ymn = -10000;
	CurView6->NewBounds.ymx =  10000;
	CurView6->HaveBounds = FALSE;
    CurView6->WindowIsZoomed = FALSE; 
    CurView6->BoundsDisplayID = 0;
    CurView6->lpBoundsDisplay = NULL;
    CurView6->pTheme = 0;
	CurView6->NumThemes = 0;
	CurView6->NumFiles = 0;
	CurView6->NumVisList = 0;
	_fstrcpy (CurView6->VisName,"");
	_fstrcpy (CurView6->PickName,"");
	_fstrcpy (CurView6->DisplayRedefFile,"");
	_fstrcpy (CurView6->PickMacroFile,"");
	
	CurView6->StartupFunction=0;
	_fstrcpy(CurView6->FunctionFile,"fundir\\appl1.txt");
	_fstrcpy(CurView6->FunctionDir,"index.txt");
	i=BigWrite (FidConfig,(HPSTR)CurView6,sizeof(VIEWPORT_V6),-1);
	Signature = 28052;
    BigWrite (FidConfig,(HPSTR)&Signature,2,-1);
    BigWrite (FidConfig,(HPSTR)&Version,2,-1);
    
	GSSiGlobUlFree (&hViewports[0]);
    GSSiClose2 (&FidConfig);  


	return (1);
} 
  
long BoundsDisplayRead (LPVOID *lpV,int Fid)
#if ENABLETRACE
{GSSiEnterProg (122);
#endif
{	LPBOUNDSDISPLAY lpBoundsDisplay;
	long	iread;
	HANDLE	Handle;
#pragma pack(2)
	struct {
			long	nbytes;
			short		version;
			} BV;
#pragma pack()
	Handle = GSSiGlobAlloc (  69,GHND,sizeof(BOUNDSDISPLAY));
	lpBoundsDisplay = (LPBOUNDSDISPLAY)GlobalLock(Handle);
	*lpV = lpBoundsDisplay;
	iread = 0;
	iread += GSSilread (Fid,&BV,sizeof(BV));
	if (BV.version == 1)
	{
		HANDLE	hMem=GSSiGlobAlloc (0,GMEM_MOVEABLE,SizeBOUNDSDISPLAY16);
		LPSTR	pbd16 = GlobalLock (hMem);

		iread += GSSilread (Fid,pbd16,(UINT)BV.nbytes-6);
		*lpBoundsDisplay = BD16ToBD32 (pbd16);
		GSSiGlobUlFree (&hMem);
	}
	else
		iread += GSSilread (Fid,lpBoundsDisplay,(UINT)BV.nbytes-6);
	lpBoundsDisplay->Handle = Handle;
{
#if ENABLETRACE
GSSiExitProg (122);
#endif
	return (iread);
}
#if ENABLETRACE
}
#endif
}
long BoundsDisplayWrite (LPVOID lpV,HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (123);
#endif
{	LPBOUNDSDISPLAY lpBoundsDisplay;
	long	iread;
#pragma pack(2)
	struct {
			long	nbytes;
			short		version;
			} BV;
#pragma pack()

	lpBoundsDisplay = lpV;
	iread = 0;
	BV.nbytes = sizeof(BV) + sizeof(BOUNDSDISPLAY);
	BV.version = 2;
	iread += BigWrite (Fid,(HPSTR)&BV,sizeof(BV),-1);
	iread += BigWrite (Fid,(HPSTR)lpBoundsDisplay,sizeof(BOUNDSDISPLAY),-1);
{
#if ENABLETRACE
GSSiExitProg (123);
#endif
	return (iread);
}

#if ENABLETRACE
}
#endif
}




