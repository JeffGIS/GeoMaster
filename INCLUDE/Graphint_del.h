#include <windows.h>
#include <string.h>
#include <stdlib.h>
#ifndef	_WIN32_WCE
#include <wmf.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "Graphics.h"
#include "shr.h"
#include "bt.h"
#include "extrndb.h"

#define		MAX_NEW_OBJECTS		100
#define		MAX_FUNCTION_STACK	16 
#define		MAX_VIEWPORT_FILES	4
#define     MAX_VIEWPORT_THEMES 16
#define		UMIFS_DATAFILE		1
#define		FOXPRO_DATAFILE		2
#define		MSACCESS_DATAFILE	3
//lda addition
#define     ODBC_DATAFILE       4
//end of lda addition
typedef struct
   {	int	xmn;
   		int	ymn;
   		int xmx;
   		int	ymx;
   	} mnmxCor;
mnmxCor		MinMax, *pMinMax;
typedef mnmxCor FAR	*LPMINMAX;

typedef struct
	{   int		Marker;
		mnmxCor	MinMax;
		int		Len;
	} ITEM;

typedef struct
{	mnmxCor MinMax;
	long offset;
}	SegHeader;

typedef struct
   {	LONG	xmn;
   		LONG	ymn;
   		LONG	xmx;
   		LONG	ymx;
   	} MNMXCORL;

typedef struct
{	mnmxCor MinMax;
	int Next[2];
	long TypeOffset[10];
}	QUAD;

typedef	struct
	{	LONG	Segment;
		WORD	Offset;
		LONG	Refno;
		int		Desc;
		int		Type;
	} PickData;
typedef MNMXCORL FAR  *LPMNMXCORL;

typedef	struct
	{	int		Parent;
		HANDLE	Next;
	} PARLIST;
               
typedef struct
	{	HANDLE	hVisList;
		LPSTR	LastVisList,
				NextVisList;
		double	MinScale,
				MaxScale;
		BOOL	WantType[10];
		BOOL	FileIsVisible[MAX_VIEWPORT_FILES];
		int		MinPointSize;
		int		MaxPointSize;
		BYTE	VisBits[400];

	}	VISLIST;
typedef VISLIST	FAR	*LPVISLIST; 


typedef struct
	{	
		int		ID;
		HANDLE	handle;
		int		TargetViewport; 
		BOOL	IsActive;
		int		DisplayViewport;
		int		CoordinateSystem,
				Units,
				Format;
       	LOGFONT	Font;
    }	COORDINATEDISPLAY;
typedef	COORDINATEDISPLAY	FAR	*LPCOORDINATEDISPLAY;

typedef struct
	{	
		int		NumItems;
		LPVOID	ItemHandle[];
    }	SAVELIST;
typedef	SAVELIST	FAR	*LPSAVELIST;

typedef struct
	{	
		int		ID;
		HANDLE	handle;
		int		TargetViewport; 
		BOOL	IsActive;
		BOOL	WantDataPass;
		BOOL	ComputeClassBoundaries;
		BOOL	DisplayScatterDiagram;
		HANDLE	hThemeDB,
				hScatterFile;
		char	DataFile[256];
		int		DataFileType;
		char	Index[34];
		FIELDINFO	Field[2];
		char	ScatterFile[128];
		int		NumClass;
        int		ClassType;
        int		ValConv;
        BOOL	LogScale;
        long	XLimit,
        		YLimit;   
        double	Xmin,Ymin,Xmax,Ymax;
        long	ClassMin[16],
        		ClassMax[16];
        COLORREF	ClassColor[16];
        HBRUSH		ClassBrush[16],
        		NoDataBrush;
        long	ClassCount[16];
        DPOINT	ClassPnt[16];
        COLORREF	BGColor, ScatterColor, ScatterBoxBG, TitleBoxBG;
        RECT	Rect, TitleBox, ScatterBox, ColorsBox, RangesBox, InfoBox;
        RECT	ClassClrBox[16];
        int		Margin, ScatterWidth, ColorsWidth, InnerMargin, TitleHeight;
        char	ClassBM[16][128];
        char	Title[256];
       	LOGFONT	TitleFont, 
       			ClassFont1,
       			ClassFont2;
       	int		Xmove, Ymove;
        
	}	THEME;
typedef THEME	FAR	*LPTHEME; 

typedef struct
	{	char	Name[256];
		HANDLE	hDibInfo, hImage;
		MNMXCORD	Bounds;
		double	Res;
		int		Width, Height;
		long	LastUsed;
	}	ORTHO;
typedef ORTHO	FAR	*LPORTHO;

typedef struct
	{	int		Len;
		int		BMWidth, BMHeight;
		MNMXCORD	Bounds;
		char	Name[256];
	}	FILEINDEXENTRY;
typedef FILEINDEXENTRY	FAR	*LPFILEINDEXENTRY; 

typedef struct
	{	int		Type,
				NumFiles,
				CurFile;
		HANDLE	Handle;
		double	OrthoRes;
		LPFILEINDEXENTRY	CurrentEntry;
		char	FirstIndex;
	}	FILEINDEX;
typedef FILEINDEX	FAR	*LPFILEINDEX;


typedef struct
	{	
		int			ID,
					Parent,
					Type,			/* 1 = Plan, 2 = Profile, 3 = Cross Section,
									   4 = 3D,   5 = Menu,    6 = Data, 7= Global View		 */
					GlobalChild;    /* For type 7 only. Indicates which viewport is
									   zoomed/panned when zoom/pan commands entered in this
									   viewport.											 */ 
		HWND		hWnd;
		HDC			hDC;
		BOOL		Display;
		COLORREF	BackGroundColor;
		BOOL		Shadow;
		float		DesiredHeight,	/* Establishes aspect ratio and plot					 */
					DesiredWidth;	/* size. On screen or plotters smaller
								       than desired size, aspect ratio will
								       be preserved. Applies only to Viewport 1				.*/
	 	int			TagPointID;		/* 1 = lower left, 2 = upper left, 3 = upper right,
	 								   4 = lower right										 */
	 	int			TagPointType;	/* 1 = fixed coordinates, 2 = relative coordinates		 */
	 	POINT		TagPoint;		/* if relative coords represent percent of parent 		 */
	 	POINT		TagPointActual;	/* actual Window coordinates							 */
	 	int			WidthType, 		/* 1 = fixed, 2 = relative								 */
	 				HeightType;
	 	int			Width,
	 				Height;
	 	float		Margin;			/* inner margin as a percent of the max dimension		 */
	 	RECT		Rect;			/* The current screen rect for this viewport 			 */ 
	 	RECT		DrawRect;
	 	RECT		ZBRect;			/* The relative dimensions of the zoom box - diff from
	 								   Rect if target of zoom is another VP					 */
	 	HRGN		hRgn;
		MNMXCORD	WBounds,		/* Current world coordinate bounds of viewport			 */
					NewBounds,		/* Desired new bounds. May be adjusted to fit viewport.  */
					FileBounds;     /* Current file coordinate bounds of viewport			 */
		MNMXCORL	Bounds;			/* Current file coordinate bounds of viewport			 */ 
		LPVOID		TranWinToBase,
					TranBaseToWin; 
		double		BaseUnitsPerPixel;
		BOOL		HaveBounds;
		int			BoundsDisplayID;
		LPVOID		lpBoundsDisplay;
		BOOL        WindowIsZoomed,
					WindowZoomedToOrtho;
		int			wOrigX, wOrigY, wExtX, wExtY, vExtX, vExtY;
		LPTHEME		pTheme;
		int			NumThemes;
		LPTHEME		pThemes[MAX_VIEWPORT_THEMES]; /* Pointers to themes which this viewport
													 is a target of */
		int			BoundsDisplayVP;			  /* If this VP is used as a bounds display
													 VP for another VP this is the others VP id.*/
		int			PassID;			/* Current pass - 0 = Theme pass, 1 = Display pass; */
		BOOL		WantPass[2];		
		int			NumFiles;
		int			CurFile;
		int			FileType[MAX_VIEWPORT_FILES];  /* 1 = Format (Viewport Coord),
													  2 = Standard (World Coord),
													  3 = Editable file (World Coord) 		 
													  4 = Plot file directory (World Coord)   
													  5 = Ortho Photo directory              */
		LPSTR		lpFiles[MAX_VIEWPORT_FILES];   /* Files or file sets associated with viewport			 */
		LPFILEINDEX	lpIndex[MAX_VIEWPORT_FILES];   /* Pointers to file indexes if type 3     */
		int			NumVisList;
		LPVISLIST	pVisList1;		/* Points to first (top) visibility list in chain 		 */ 
		int			FunctionStack[MAX_FUNCTION_STACK];
		int			LenFunctionStack;
		char		FunctionFile[128]; /* File containing active function list */
		char		FunctionDir[128];   /* Current directory of functions */
		int			NumNewObjects;		/* NewObject allows items to be redefined by class */
		int			NewObjectMap[3201];
		HANDLE		NewObject[MAX_NEW_OBJECTS];
		int			NewObjectType[MAX_NEW_OBJECTS], NewObjectWidth[MAX_NEW_OBJECTS];
		COLORREF	NewObjectColor[MAX_NEW_OBJECTS];
        char		EndOfViewport;
	}	VIEWPORT;
typedef VIEWPORT	FAR	*LPVIEWPORT;

typedef	struct
	{	HANDLE	Handle;
		int		DisplayVP, TargetVP;
		int		Type;      
		RECT	SavedRect;
		HBITMAP SavedScreen;
		POINT	BoxPoints[7];
	} BOUNDSDISPLAY;
typedef BOUNDSDISPLAY FAR  *LPBOUNDSDISPLAY;

typedef	struct
	{	int		TargetVP;
		char	StreetNameFile[256];
		char	StreetSegFile[256];
		char	SegmentAddressFile[256];
		char	AddressPlotFile[256];
	}	ADDRESSLOCATION;
typedef	ADDRESSLOCATION	FAR	*LPADDRESSLOCATION;



POINT		CursRtnPnt;


typedef struct
{	RECT	rect;
	DPOINT	TAGPoint;
	POINT	TAGPointScr;
	POINT	ConnectPoint;
	POINT		RestorePoint;
	DPOINT		center;
	char	text[256];
	int			PLwidth;
	int			PLstyle;
	int			TXheight;
	COLORREF	BGcolor;
	COLORREF	TXcolor;
	COLORREF	BorderColor;
	HBITMAP		before, after;
	int			bmWidth, bmHeight;
	LOGFONT		LogFont;
} TAGBOX;
TAGBOX	TAGBox;

typedef struct
{	MNMXCORD	FromBounds;
	MNMXCORD	ToBounds;
}	BLOWUPREC;
BLOWUPREC BlowUpRec;


char		CurrentSymName[9];


BOOL OpenThemeDataFile (void);
BOOL CloseThemeDataFile (BOOL Final);
void WriteSavedItems (HANDLE hSaveList);
BOOL PointInPickArea (POINT	Point);
void MovePickListToSaveList (HANDLE hSaveList);
void DrawSavedItems (HANDLE hSaveList, HANDLE hNewList);
HANDLE CreateSaveList (int NumItems);
void DeleteSavedItems (HANDLE hSaveList);
BOOL MovePolyLine (void);
HRGN CreateVPRgn (void);
LPTHEME	AddTheme (int ThemeID);
void DeleteTheme (LPTHEME pTheme);
double ComputeRes (int h, int w, MNMXCORD bounds);
void AddPassiveFun (LPVOID Ptr);
LPVOID	ReadObject (int	Fid);
void ProcessPassiveFunctions (HWND hWnd,WORD Message, WORD wParam,LONG lParam);
void DisplayCursorCoordinate (HWND hWnd,WORD Message, WORD wParam,LONG lParam);
void SelectVisList (void);
void DuplicateDescInit(void);
BOOL DuplicateDesc(int idesc, BOOL DupSet);
void DuplicateDescClose(void);
BOOL ThemeEdit (HWND hWnd, WORD Message, WORD wParam, LONG lParam); 
BOOL ThemeActivate (HWND hWnd, WORD Message, WORD wParam, LONG lParam);
BOOL ThemeDeActivate (HWND hWnd, WORD Message, WORD wParam, LONG lParam);
void SetNewBoundsToOrtho(void);
void ShowFullBM(void);
void OpenOrthos (void);
void CloseOrthos (void);
BOOL OrthoInBuffer(LPSTR Name);
BOOL RectInWBounds (MNMXCORD MinMax);
BOOL DisplayBMInVP (HDC hDC, HANDLE hDibInfo, HANDLE hImage, BOOL Stretch);
BOOL LoadBitMap (LPSTR ImageFile, LPHANDLE phDibInfo, LPHANDLE phImage);
void IncrementFile (void);
void ShowZoomBoxClear (HDC hDC, BOOL *HaveBox);
void RestoreScreen (HDC hDC, HBITMAP hSavedBM, RECT Rect);
HBITMAP SaveScreen (HDC hDC, RECT Rect); 
int CenterWindow (HWND hWnd, DPOINT CenterPoint, BOOL Imediate);
POINT BasePtToWinPt (DPOINT WPoint);
DPOINT WinPtToBasePt (POINT Point); 
DPOINT FilePtToBasePt (POINT Point);

void ShowZoomArea (HDC hDC,POINT StartPoint,POINT LastPoint,
				           HBITMAP *SavedScreen, RECT *SavedRect, POINT Points[7]);
LPVOID BoundsDisplayInit (int Type, int DisplayVP, int TargetVP);
void BoundsDisplayShow (LPVOID lpBoundsDisplay,BOOL First);
long BoundsDisplayRead (LPVOID lpBoundsDisplay,int Fid);
long BoundsDisplayWrite (LPVOID lpBoundsDisplay,int Fid);
void BoundsDisplayDestroy (LPVOID lpBoundsDisplay);
BOOL SetFileBounds ();
void SetBMDisplay (LPSTR File, DPOINT SegPoint);
void ShowBitmap (long iref,DPOINT BasePoint);
void ComputeBMLoc (RECT Rect, LPBITMAPINFO pDibInfo);
void ThemePickImage (POINT MousePoint);
BOOL PickImage (HWND hWnd, WORD Message, WORD wParam, LONG lParam);
BOOL DisplayImage (HWND hWnd, WORD Message, WORD wParam, LONG lParam);
BOOL PtInBounds (POINT SegPoint);
void DisplayBMThemeLegend(void);
void DrawPointerLine (HDC hDC,POINT beginpoint,POINT endpoint,HPEN LinePen, HPEN TipPen);
void LinkScatterPointToMap (int X,int Y,POINT MapPoint);
POINT CurrentMidPoint (int Type);
void ThemeDisplayScatterDiagram ();
RECT PctRect (RECT Rect, float Pct);
void DisplaySVThemeLegend(void);
void DisplayTwoVThemeLegend(void);
void ThemeDisplayLegend(void);
void ThemeBeginDisplayPass(void);
void ThemeEndDisplayPass(void);
void ThemeEndDataPass (void);
BOOL ThemeSetData (int Type, long iref, int desc);
BOOL ThemeSetChar (int Type, long iref, int desc);
void ThemeEndDataPass (void);
BOOL ThemeNeedsDataPass (void);
void ResetViewport (BOOL WantDisplayPass);
int  SetCharsFromTheme (HDC hDC, int Type, long iref, int desc);
void SelectViewport (LONG lParam);
void InitVis (void);
void UnallocateConfig (void);
void SetupViewports (HWND hWnd, HDC hDC, int CurParent,RECT rect);
void SetupViewport (RECT rect);
BOOL GetNextViewportFile (void);
void CloseConfig (void);
BOOL DisplayViewport (HWND hWnd, HDC hDC);
void RedisplayViewport (BOOL Imediate);
void RedisplayViewports (BOOL Imediate);
int	 BlockInWindow (mnmxCor MinMax);
int  FindNextSegment(void);
WORD FAR PASCAL		ComputeWMFChecksum( LPWMFHEADER );
BOOL ItemInRegion (HRGN hRgn,LPPOINT lpPoints, int npnts);
BOOL ProcessPrimarySeg (HWND hWndDlg, int DlgItemSym, int DlgItemPar);
BOOL LoadQuadTree(void);
LPINT PickPolygon (LPPOINT lpPoints2,int nPnts,LPINT ipnt);
LPINT PickPolyline (LPPOINT lpPoints2,int nPnts,LPINT ipnt);
void PickLine (int x1, int y1, int x2, int y2);
void ProcessGraphicsRec (HDC hDC, LPINT ipnt);
int	PickItems (HWND hWnd,DPOINT PickPoint);
void SetLineHighlight(HDC hDC);
void SetAreaHighlight(HDC hDC);
POINT WinCoordToFileCoord(POINT WinPoint);
POINT FileCoordToWinCoord(POINT SegPoint);
DPOINT WinCoordToVPCoord(POINT WinPoint);
void SetDisplayMode (HDC hDC,int Mode);
BOOL LineInBounds (int NPNTS, LPPOINT POINTS, LPMNMXCORL BOUNDS);
void	GetSymbolName (int Desc, LPSTR Name);
BOOL	GetTAG (LONG Refno, LPSTR TAG);
void DisplayTAGs(HDC hDC);
void RestoreTAG(HDC hDC);
void SaveTAG(void);
HANDLE AddParToList(int Parent);
BOOL GetTextVisibility (int TSize);
POINT AdjustPoint (HWND hWnd,POINT StartPoint,POINT LastPoint,int Xmove,int Ymove);
BOOL PanZoomTarget (HWND hWnd, WORD Message, WORD wParam, LONG lParam);
BOOL ZoomRectangle (HWND hWnd, WORD Message, WORD wParam, LONG lParam);
BOOL MoveTAG (HWND hWnd, WORD Message, WORD wParam, LONG lParam);
BOOL WindowZoom (HWND hWnd, WORD Message, WORD wParam, LONG lParam);
BOOL BlowUp (HWND hWnd, WORD Message, WORD wParam, LONG lParam); 
BOOL PanToPoint (HWND hWnd, WORD Message, WORD wParam, LONG lParam); 
BOOL ShowItem (HWND hWnd, WORD Message, WORD wParam, LONG lParam); 
BOOL ColorClass (HWND hWnd, WORD Message, WORD wParam, LONG lParam); 
BOOL CreateTAG (HWND hWnd, WORD Message, WORD wParam, LONG lParam); 
BOOL PickScatterPoint (HWND hWnd, WORD Message, WORD wParam, LONG lParam); 
BOOL ThemeSVChangeColor (HWND hWnd, WORD Message, WORD wParam, LONG lParam); 
BOOL MoveIntersection (HWND hWnd, WORD Message, WORD wParam, LONG lParam); 
void ThemeSVChangeClr (POINT MousePoint);
void ShowZoomBox (HDC hDC,POINT StartPoint,POINT LastPoint, BOOL *HaveBox);
void BuildRefIndex (void);
void OpenRefIndex (void);
void CloseRefIndex (void);
void OpenHighlightList(void);
void CloseHighlightList(void);
void AddToHighlightList (long Refno);
void DrawTAG (HWND hWnd, HDC hDC, BOOL MoveMode, BOOL Restore);
void ShowPointerLine (HWND hWnd,RECT rect,POINT endpoint, POINT begpoint, BOOL *HavePL);
BOOL PointInRect (POINT point, RECT rect);
int	SelectPickedItem (void);
void GetItemText (int item, LPSTR text);
void CreateTAGBox (HDC hDC, DPOINT TagPoint, LPSTR text);
void AddMinMax (LPMINMAX mm1, LPMINMAX mm2);
void ResetTAGBox (HDC hDC, DPOINT TagPoint, LPSTR text);
