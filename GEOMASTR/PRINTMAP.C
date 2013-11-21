#include "graphint.h"

BOOL		ForceRefIndex=FALSE;
BOOL		CreateConfig=FALSE; 
BOOL		NoRefIndex = FALSE;
BOOL		ShowFileBounds = FALSE;
HWND		hWndMain;
LPORTHO		CurOrtho, OrthoBuffers[8]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
HANDLE		hOrthos=NULL; 
long		OrthoUse=LONG_MIN;
BOOL		Printing=FALSE;
BOOL		SolidAreas=TRUE; 
BOOL		UpdateSegment=FALSE;
char		str[256];
char		PltName[256];
char		PickName[256];
extern		char ViewName[256];
int			PltType;
char		CfgName[128];
char		PlotName[9];
static int 	idTimer=0, Fid=0, FidConfig=0;
int			FileNum, FileInIndex;
LPVOID		TranFileToBase=NULL, TranBaseToFile=NULL, TranBMToBase=NULL, TranBaseToBM=NULL;
BOOL		FirstError, DoPaint=TRUE, HavePaint=FALSE;
BOOL		Highlight=FALSE;
static BOOL		FileMode=FALSE, HaveBasePens;
HANDLE		hDupDesc;
BOOL		*pDupDesc;
LONG		pu=0; 
RECT		MainRect;
LONG		UsedDescOffset,TranPointOffset, ColorPaletteOffset, GraphicsOffset;
int			nPnts, nRead, i, nBytes;
int			i2, Pcode;
int			PickAp=4;
HCURSOR		hCursor, curCursor;
int			ipen, WidthFactor;
RGBQUAD 	rgb;
HPEN		hOldPen, hSavePen, hNullPen, hRedPen, pens[266], HighlightPen;
HPEN		CurrentPen;
HBRUSH		hOldBrush, brushes[266], hRedBrush, hGreenBrush, hBlueBrush, HighlightBrush;
static int			maxbrush;
static POINT	Points[4];
static HANDLE		hPoints, hQuadTree=0, hQuadOffset=0;
LPSTR		pQuadTree, pQuadSeg, pQuadOffset;
LPSTR		BeginSeg;
LONG		LenQuadSeg;
LONG		LenQuad, MaxQuadLevel, MaxQuadType;
int			QuadLevelAt, QuadTypeNext;
LONG		ContinuationOffset;
BOOL		Wincap=TRUE, Pick=FALSE;
int			SetVisByPar=-1;
BOOL		ParIsVisible;
BOOL		CreateRefIndex;
BOOL		AutoPan=TRUE;
HANDLE		hRefIdx=0;
int	ShadowInc;
float	ShadowPct = 1.0F, BorderPct = 0.5F;
int	MaxDimension;

#ifdef	Bradly
BOOL	Brad=FALSE;
#else
BOOL	Brad=FALSE;
#endif
BYTE	Mask[8] = {128, 64, 32, 16, 8, 4, 2, 1};

float	PixelsPerHInch;

HANDLE	CurParList, LastParList;

LPPOINT lpPoints, lpPoints2, lpPoints3;
HRGN		hRgn;
MNMXCORD	ZoomBoxRect;

static	 long	PrimeOffset;

PickData	PickList[256];
int		NumPicked, MaxPick=256;
mnmxCor	PickMinMax;
LONG	CurrentSeg, CurrentRefno;
int		CurrentDesc;
WORD	CurrentItem;
RECT	CurrentRect, ClipRect;
HANDLE	hVisList;
POINT	ZBPoints[5];

struct	{int	FileInIndex;
		 long	Segment;
		 WORD	Offset;
		 }	RefIdxData;
		 
LPINT		CurElementPnt;
		 
LPVISLIST	CurVis=0;
LPVIEWPORT	CurView=0, pViewports[16];
HANDLE		hViewports[16];
int			NumViewports=0, CurViewID=0, DisplayViewID, NumViewportsToDisplay;
COLORREF	WindowColor; 
int			CommandViewport;
LPTHEME		CurTheme=0;

extern	int	NumPassiveFun;
extern	BOOL	AddLBUTTON;
extern	HANDLE	hInst;
extern	char	FullBM[256];

