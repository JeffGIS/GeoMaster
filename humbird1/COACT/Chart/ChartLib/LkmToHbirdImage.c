#include	<stdlib.h>
#if !defined(HUMMINBIRD)
#include	<malloc.h>
#include	<fcntl.h>
#endif
#include	<stdio.h>
#include	<string.h>
#include	<time.h>
#define _TIME_T
#include	"lzoconf.h"
#include	"minilzo.h"
#include  "text.h"
#include "chart.h"
#include "ClMalloc.h"
#include "fs.h"
#include "ClAssert.h"
#include "ByteSwap.h"
#include "ChartLib.h"

#if defined(HUMMINBIRD)
#include "fs_api.h"
#if !defined(HBIRDDEMO)
#define HBIRDDEMO 1
#endif
#endif

 
// FILE NAME CONSTANTS
#define SYMLISTBIN "symlst.bin"
#define LKMTEXTBIN "text.bin"
#define LKMPOINTSBIN "pnts.bin"
#define LKMNAMESBIN "cities.bin"
#define LKMLAKESBIN "lnames.bin"
#define OBJECTLISTTXT "objlst.txt"
#define GRIDDEFBIN "grddef.bin"

//PARAMETER CONSTANTS
#define DBUGPARM "DBUG="
#define SDFPARM "SDF="
#define DDFPARM "DDF="
#define CHARTTSPARM "CHARTTS="
#define CITYTSPARM "CITYTS="
#define LAKETSPARM "LAKETS="
#define CHARTTCPARM "CHARTTC="
#define CITYTCPARM "CITYTC="
#define LAKETCPARM "LAKETC="
#define HDCPARM "HDC="
#define DOCPARM "DOC="
#define SRCPARM "SRC="
#define PMCPARM "PMC="
#define MXCOSPARM "MXCOS="
#define MXCTSPARM "MXCTS="
#define MLTSPARM "MNLTS="
#define MNCTYTSPARM "MNCTYTS="
#define MXCTLSPARM "MXCTLS="
#define MXCTTSPARM "MXCTTS="

#define ALLOWCONTOURTEXTROTATION 1

static int MAX_CHARTOBJECT_SCALE=10;
static int MAX_CHARTTEXT_SCALE=20;
static int MIN_LAKENAME_TEXT_SCALE=10;
static int MIN_CITY_TEXT_SCALE=20;
static int MAX_CONTOURLINE_SCALE=10;
static int MAX_CONTOURTEXT_SCALE=5;


#define LOWORD(l)           ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l)           ((WORD)((DWORD_PTR)(l) >> 16))
#define LOBYTE(w)           ((BYTE)((DWORD_PTR)(w) & 0xff))
#define HIBYTE(w)           ((BYTE)((DWORD_PTR)(w) >> 8))

#define	TRUE		1
#define FALSE		0
#define MAX_RECS_IN_BLOCK	33
#define MAX_TEXT_IN_IMAGE	2048
typedef void	*HANDLE;
typedef char	*LPSTR;
typedef unsigned long       DWORD;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef BYTE	*LPBYTE;
typedef int		*LPINT;
typedef DWORD   COLORREF;
typedef struct{double x,y;} DPOINT;
typedef DPOINT  			*LPDPOINT;
typedef struct
   {	long	xmn;
   		long	ymn;
   		long	xmx;
   		long	ymx;
   	} MNMXCORL;
typedef MNMXCORL *LPMNMXCORL;
typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
typedef struct {
				int  x;
				int  y;
				} POINT;
typedef POINT	*LPPOINT;
typedef	unsigned long ULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
typedef int		HFILE;

typedef struct	{
				 int	Type;
				 int	BitmapID;
				 int	Size;
				 int	nPoints;
				 POINT	WorldPoint;
				 POINT	ImagePoint;
				 int	nChar;
				 char	Text[];	
				}LKMOBJECT;
typedef LKMOBJECT	*LPLKMOBJECT;


#include	"lkmhbird.h"

typedef	struct	{long GridID,GridCellID;} GRIDCELLDEF;
typedef struct	{long GridCellID, loc;} GRIDINDEXREC;
typedef struct	{long TileWidth,TileHeight,NumPerIndexRec,NumIndexLevs,FirstIndexLoc,NumRecs;} GRIDFILEHEADER;
typedef struct	{long GridID,NumPerIndexRec,NumIndexLevs,FirstIndexLoc,NumRecs;} CTEXTFILEHEADER;
typedef struct	{long GridID,NumPerIndexRec,NumIndexLevs,FirstIndexLoc,NumRecs;} OBJECTFILEHEADER;
typedef struct	{POINT	TilePoint;
				 POINT	CenterPoint;
				 double	RangeInMeters;
				 double Scale;
				 BOOL	First, Done;
				 HANDLE	PointEnumHandle;
				}ENUMSTRUCT;
typedef ENUMSTRUCT	*LPENUMSTRUCT;

static int	StartTime;
static int	NumImageCalls=0;
static int	TotStretchTime, TotImageTime, NumStretchCalls, TotPRTime;
int numseek,numreads,totread, totreadtime,maxseektime,numopen,maxopentime,totdecomptime;


#if defined(HUMMINBIRD)
unsigned int H_TICK_u32ReadFreeRunningTimerInMilliSeconds(void);
#elif 0
unsigned int GetTick (void)
{
  return GetTickCount();
}
#else
// Dummy function
unsigned int GetTick (void)
{ 
  return 0; 
}
#endif

typedef struct {int width, height;
				int	xoff,  yoff;
				int rectw, recth;
				int	destx, desty;
				int	destw, desth;
				int	CellID;
				MNMXCORL	CellBounds;
				int	PaletteLen;
				RGBQUAD	Palette[256];
				BYTE Image[4];
				}LKMTILE;

typedef LKMTILE *PLKMTILE;

typedef	struct	{
				 short	CharNo;
				 char 	chr;
				 short	x,y;
				 short	size;
				 short	rot;} CTEXTOUTREC;
typedef	CTEXTOUTREC	*LPCTEXTOUTREC;

typedef	struct	{
				 int	CharNo_Chr;
				 int	x_y;
				 int	size_rot;
				 } CTEXTREC;
typedef	CTEXTREC	*LPCTEXTREC;

typedef	struct	{
				 int	ref;	
				 short	symno,hasextendedtext;
				 short	type;
				 short	x,y;
				 char	PointText[64];
				 } MAPPOINTOUTREC;
typedef	MAPPOINTOUTREC	*LPMAPPOINTOUTREC;

typedef	struct	{
				 int	ref;
				 int	symno_hasextendedtext;
				 int	x_y;
				 char	PointText[64];
				 } MAPPOINTREC;
typedef	MAPPOINTREC	*LPMAPPOINTREC;

typedef	struct	{
				 short	CharNo;
				 int	chr;
				 short	x,y;
				 short	size;
				 short	rot;
				 char	ShieldText[4];} POINTOBJECTOUTREC;
typedef	POINTOBJECTOUTREC	*LPPOINTOBJECTOUTREC;

typedef	struct	{
				 int	CharNo_Chr;
				 int	x_y;
				 int	size_rot;
				 } POINTOBJECTREC;
typedef	POINTOBJECTREC	*LPPOINTOBJECTREC;

typedef	struct	{
				 int	PopulationOrAcres;
				 int	type_nchar;
				 int	x_y;
				 char	State[4];
				 char	Name[68];
				 } CITYLAKENAMEREC;
typedef	CITYLAKENAMEREC	*LPCITYLAKENAMEREC;

typedef	struct	{
				 long	PopulationOrAcres;
				 short	type;
				 short	nchar;
				 short	x,y;
				 char	State[4];
				 char	Name[68];
				 } CITYLAKENAMEOUTREC;
typedef	CITYLAKENAMEOUTREC	*LPCITYLAKENAMEOUTREC;

typedef struct {
				 int	CellID;
				 int	Numrecs;
				 MNMXCORL	CellBounds;
				 CTEXTREC	TextDef[];
				}LKMCTEXTTILE;
typedef LKMCTEXTTILE *PLKMCTEXTTILE;

typedef struct {
				 int	CellID;
				 int	Numrecs;
				 int	Nextrec;
				 MNMXCORL		CellBounds;
				 MAPPOINTREC	PointDef[];
				}LKMPOINTTILE;
typedef LKMPOINTTILE *PLKMPOINTTILE;

typedef struct {
				 int	CellID;
				 int	Numrecs;
				 int	Nextrec;
				 int	NumSelected;
				 MNMXCORL		CellBounds;
				 CITYLAKENAMEREC	PointDef[];
				}LKMNAMETILE;
typedef LKMNAMETILE *PLKMNAMETILE;

typedef struct {
				POINT	CenterWorld;
				CLFILE	*Fid;
				MNMXCORL	Bounds;
				POINT	TilePoint;
				int		GridID;
				int		CellID;
				int		WantType;
				OBJECTFILEHEADER ObjectFileHeader;
				HANDLE	RecordHandle;
				}ENUMOBJECT;
typedef ENUMOBJECT	*LPENUMOBJECT;

typedef struct {
				MNMXCORL	Bounds;
				int			Size;
				int			TextSize;
				int			TextColor;
				int			TextShadowColor;
				int			BitmapID;
				char		Name[68];
				}NAMESONSCREEN;
typedef NAMESONSCREEN	*LPNAMESONSCREEN;

typedef struct {
				int x,y;
				int	depth;
				}TEXT_IN_IMAGE_RECORD;
typedef TEXT_IN_IMAGE_RECORD	*LPTEXT_IN_IMAGE_RECORD;

typedef struct {int		UTMZone;
				int		MaxDepthLayer;
				char	MapPrefix_SufFile_dek[4];
				char	CardName[12];
				} SETUPDATA;

static	struct {int ID, Length;}ID_Length;

char	LKMPath[CL_MAX_PATH];

static	char	ImageLibraryIndex[CL_MAX_PATH];
static	CLFILE	*FidIndex=NULL;
static	CLFILE	*FidTiles=NULL;
static	BYTE	SymDepth[2000][2];
static	short	minsym, maxsym;
static	BOOL	ShowContourLines;
static	BOOL	ShowDepthColors;
static	BOOL	DisplayContourText;
static	int		HighlightDepth=0;
static	int		HighlightDepthRange=5;
static	GRIDFILEHEADER	FileHeader;
static	CTEXTFILEHEADER CTextFileHeader;
static	MNMXCORL	HBImageBounds;
static	MNMXCORL	GridCellBounds;
static	POINT	HBImageCenterWorld;
static	POINT	HBImageCenterWorld_100;
static	POINT	HBImageCenter;
static	int		ImageWidth, ImageHeight;
static	double	MetersPerPixel;
static	int	MetersPerPixel_100000; // = MetersPerPixel * 100000;
static	POINT	TilePoint;
static	int		nTextInImage;
static	LPTEXT_IN_IMAGE_RECORD	TextInImageBuffer;
static	int		nNamesOnScreen;
static	LPNAMESONSCREEN	NamesOnScreenBuffer;
static	int		SameDepthFilter, SameDepthFilterBase=300;
static	int		DiffDepthFilter, DiffDepthFilterBase=48;
static	int		LastFullCoverage = 5;
static	int		FirstFullCoverage = 1;
static	BOOL	GrayMap = FALSE;
static	int		DepthOffset = 0; 
static	int		HazardDepth = 5;
static	BOOL	ShowDepthOffsetArea=TRUE;
static	BOOL	ShowHazardAreas=TRUE;
static	RGBQUAD	HighlightDepthColor={0,243,8,0};
static	RGBQUAD	HazardDepthColor={5,0,236,0};
static	RGBQUAD	DepthOffsetColor={99,165,178,0};
static	RGBQUAD StandardResColorIn={240,240,0,0};
static	RGBQUAD PromapColorIn={251,128,0,0};
static	RGBQUAD StandardResColor={229,196,106,0};
static	RGBQUAD PromapColor={255,128,0,0};
static	int		ChartTextColor=RGB(0,0,0);
static	int		ChartTextShadowColor=RGB(255,255,255);
static	int		ChartTextSize=14;
static	int		CityTextColor=RGB(112,112,112);
static	int		CityTextShadowColor=RGB(255,255,255);
static	int		CityTextSize=16;
static	int		LakeTextColor=RGB(0,0,255);
static	int		LakeTextShadowColor=RGB(255,255,255);
static	int		LakeTextSize=14;
static	BOOL	ByteSwapNeeded;
static	char	MaxSubFileID='D';
static	int		nreads=0, numread=0;
static	char	MapPrefix[4]="mn";
static	int		MaxNamesPerTile;
static	int		MaxNamesOnScreen;
static	int		MinPopulation; 
static	int		MinDistBetweenShields;
static	int		UTMZone;
static	int		MaxDepthLayer=7;
static	int		LevUsed;
static	int		InitCalled=0;

char	CIDValue[66], dek;
BOOL	DoFilter=TRUE, FlipText=TRUE; int ii;//for debugging only
int		Dbug=0, AllowDbug=0;

static	GRIDINDEXREC	IndexRecordBlock[MAX_RECS_IN_BLOCK];

#define	MAX_GRIDS	18
typedef struct {long GridID,GridMinX,GridMinY,GridMaxX,GridMaxY,GridWidth,GridHeight,nGridCol,nGridRow;
				double	MetersPerPixel;} GRIDDEF;
static	GRIDDEF	GridDefs[MAX_GRIDS];

int		NumGrids;
int		nGridCol;
int		nGridRow;
int		GridMinX;
int		GridMinY;
int		GridMaxX;
int		GridMaxY;
int		GridWidth;
int		GridHeight;
int		GridPixelWidth;
static	int	PixelsPerScreen;

static void cl_freadGRIDDEF(GRIDDEF *grid,CLFILE *pFID);
static void cl_freadGRIDFILEHEADER(GRIDFILEHEADER *header,CLFILE *pFID);
static void cl_freadGRIDINDEXREC(GRIDINDEXREC *rec,CLFILE *pFID);
static void swapGRIDINDEXREC(GRIDINDEXREC *rec);
static void cl_freadTEXTFILEHEADER( CTEXTFILEHEADER *header, CLFILE *pFID );
static void cl_freadOBJECTFILEHEADER( OBJECTFILEHEADER *header, CLFILE *pFID );
static void cl_freadCTEXTREC( CTEXTREC *rec , CLFILE *pFID );
static void swapCTEXTREC( CTEXTREC *rec );
static void cl_freadMAPPOINTREC( MAPPOINTREC *rec , CLFILE *pFID );
static void swapMAPPOINTREC( MAPPOINTREC *rec , CLFILE *pFID );
static void swapCITYLAKENAMEREC( CITYLAKENAMEREC *rec , CLFILE *pFID );

RGBQUAD DepthColor (int Depth,int HighlightDepth,int HighlightDepthRange,BOOL ShowDepthColors);
BOOL ConvertHBirdColors (int ClrUsed,RGBQUAD *pal);
BOOL LoadHBSymList (LPSTR Path);
BOOL LoadHBParms (LPSTR Path);

int StretchHBBits (HANDLE HBImageHandle,
				   int XDest, int YDest, int nDestWidth, int nDestHeight,
				   int XSrc,  int YSrc,  int nSrcWidth,  int nSrcHeight, 
				   BYTE *lpBits,
				   int TileHeight, int TileWidth,
				   int TilePaletteLen,
				   RGBQUAD *TilePalette);

void GetLKMImageInit (int HBImageWidth,int HBImageHeight,int CenterX, int CenterY, double Scale, int ForceLevel,char SubFileID,int Rotation,
					  int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange,LPPOINT pTilePoint);
void GetLKMImageTerminate (void);
PLKMTILE GetLKMImageTile (POINT *Point,LPINT pRc);
int DecompressBinaryRecord (LPBYTE pDecompressedRec,LPBYTE pCompressedRec,int CompressedLen);		
int DeCompressByteArray (LPBYTE pMemCmp,LPBYTE pMem,int lMem);
BOOL DecompressTile (BYTE *Image,int compressedlen);
void GetLKMCTextTerminate (void);
PLKMCTEXTTILE GetLKMCTextTile (LPPOINT pTilePoint);
void GetLKMCTextInit (int HBImageWidth,int HBImageHeight,int CenterX, int CenterY, double Scale, LPPOINT pTilePoint,char SubFile);
POINT WorldToScreen (POINT WorldPoint);

#if !defined(HUMMINBIRD)
#pragma warning(disable : 4996) 
#endif

int GetClockTicks (void)
{
   unsigned int clkval;

   if (!Dbug)
	   return 0;
#if defined(HUMMINBIRD)
   clkval = H_TICK_u32ReadFreeRunningTimerInMilliSeconds();
#else
   clkval = GetTick();  
#endif
   return clkval;
}

void LKMTrace (LPSTR TraceMessage)
{
	return;
}
static BOOL cl_ByteSwapNeeded()
{
   long one= 1;
   return !(*((char *)(&one)));
}

void LKMSelectMap (char * MapID)
{
	strcpy (MapPrefix, MapID);
	return;
}

void SetGridVals (int i)
{
	nGridCol = GridDefs[i].nGridCol;
	nGridRow = GridDefs[i].nGridRow;
	GridMinX = GridDefs[i].GridMinX;
	GridMinY = GridDefs[i].GridMinY;
	GridMaxX = GridDefs[i].GridMaxX;
	GridMaxY = GridDefs[i].GridMaxY;
	GridWidth = GridDefs[i].GridWidth;
	GridHeight = GridDefs[i].GridHeight;
	SameDepthFilter = SameDepthFilterBase;
	DiffDepthFilter = DiffDepthFilterBase;
	return;
}

POINT TilePointToImagePoint (int TileX, int TileY,LPMNMXCORL pCellBounds)
{
	POINT pt;
	int	WorldX, WorldY;

	WorldX = pCellBounds->xmn*100 + (TileX * (pCellBounds->xmx - pCellBounds->xmn))/100;
	WorldY = pCellBounds->ymx*100 - (TileY * (pCellBounds->ymx - pCellBounds->ymn))/100;
	pt.x = HBImageCenter.x - (1000*(HBImageCenterWorld_100.x - WorldX)) / MetersPerPixel_100000;
	pt.y = HBImageCenter.y + (1000*(HBImageCenterWorld_100.y - WorldY)) / MetersPerPixel_100000;
	return pt;
}

POINT WorldPointToImagePoint (int TileWorldX, int TileWorldY)
{
	POINT pt;

	pt.x = HBImageCenter.x - (int)((HBImageCenterWorld.x - TileWorldX) / MetersPerPixel);
	pt.y = HBImageCenter.y + (int)((HBImageCenterWorld.y - TileWorldY) / MetersPerPixel);
	return pt;
}

POINT WorldPointToScreenPoint (int TileWorldX, int TileWorldY)
{
	POINT pt;

	pt.x = HBImageCenter.x - (int)((HBImageCenterWorld.x - TileWorldX) / MetersPerPixel);
	pt.y = HBImageCenter.y + (int)((HBImageCenterWorld.y - TileWorldY) / MetersPerPixel);
	return pt;
}

/*POINT TilePointToWorldPoint (int TileX, int TileY,LPMNMXCORL pCellBounds)
{
	POINT WorldPt;

	WorldPt.x = pCellBounds->xmn + (TileX * (pCellBounds->xmx - pCellBounds->xmn)) / 10000;
	WorldPt.y = pCellBounds->ymn + ((10000-TileY) * (pCellBounds->ymx - pCellBounds->ymn)) / 10000;

	return WorldPt;
}*/
POINT TilePointToWorldPoint (int TileX, int TileY,LPMNMXCORL pCellBounds)
{
	POINT WorldPt;

	WorldPt.x = pCellBounds->xmn + ((TileX/10) * (pCellBounds->xmx - pCellBounds->xmn)) / 1000;
	WorldPt.y = pCellBounds->ymn + (((10000-TileY)/10) * (pCellBounds->ymx - pCellBounds->ymn)) / 1000;

	return WorldPt;
}

POINT WorldPointToTilePoint (int WorldX, int WorldY,PLKMTILE pLKMTile)
{
	POINT pt;

	pt.x = (pLKMTile->width * (WorldX - pLKMTile->CellBounds.xmn))/(pLKMTile->CellBounds.xmx - pLKMTile->CellBounds.xmn);
	pt.y = (pLKMTile->height * (WorldY - pLKMTile->CellBounds.ymn))/(pLKMTile->CellBounds.ymx - pLKMTile->CellBounds.ymn);
	return pt;
}

BOOL PointInBound (POINT Point,LPMNMXCORL pBounds)
{   
	if (Point.x < pBounds->xmn ||
		Point.x > pBounds->xmx ||
		Point.y < pBounds->ymn ||
		Point.y > pBounds->ymx)
		return FALSE;
	return TRUE;
}

BOOL BoundsInBound (LPMNMXCORL pBounds1,LPMNMXCORL pBounds2,short Opt)
{   
	BOOL InBounds=TRUE;
	
	switch (Opt)
	{
		case 0: //bounds 1 completely in bounds2
	    if (pBounds1->xmn < pBounds2->xmn ||   
	        pBounds1->xmx > pBounds2->xmx ||
	        pBounds1->ymn < pBounds2->ymn ||
	        pBounds1->ymx > pBounds2->ymx)
	        InBounds=FALSE; 
	    break;
	    
	    case 1: //bounds 1 at least partially in bounds2   
	    if (pBounds1->xmx < pBounds2->xmn ||   
	        pBounds1->xmn > pBounds2->xmx ||
	        pBounds1->ymx < pBounds2->ymn ||
	        pBounds1->ymn > pBounds2->ymx)
	        InBounds=FALSE; 
	    break;
	}
    return InBounds;
}
void InflateMnMxL (LPMNMXCORL pBounds, int Value)
{      
	pBounds->xmn -= Value;
	pBounds->xmx += Value;
	pBounds->ymn -= Value;
	pBounds->ymx += Value;
	return;
} 

BOOL RoomForNameOnScreen (LPSTR Name,int nChar,int BitmapID,int Size,int TextSize,int TextColor,int TextShadowColor,LPMNMXCORL pBounds)
{
	int	i;

	for (i=0;i<nNamesOnScreen;i++)
	{
		if (BoundsInBound (pBounds,&NamesOnScreenBuffer[i].Bounds,1))
		{
			if (Size > NamesOnScreenBuffer[i].Size)
			{
				NamesOnScreenBuffer[i].BitmapID = BitmapID;
				NamesOnScreenBuffer[i].Size = Size;
				NamesOnScreenBuffer[i].TextSize = TextSize;
				NamesOnScreenBuffer[i].TextColor = TextColor;
				NamesOnScreenBuffer[i].TextShadowColor = TextShadowColor;
				NamesOnScreenBuffer[i].Bounds = *pBounds;
				strncpy (NamesOnScreenBuffer[i].Name,Name,nChar+1);
			}
			return FALSE;
		}
		if (NamesOnScreenBuffer[i].BitmapID == BitmapID &&
			!strcmp (NamesOnScreenBuffer[i].Name,Name))
		{
			MNMXCORL Bounds1 = NamesOnScreenBuffer[i].Bounds;
			MNMXCORL Bounds2 = *pBounds;

			InflateMnMxL (&Bounds1,MinDistBetweenShields);
			InflateMnMxL (&Bounds2,MinDistBetweenShields);
			if (BoundsInBound (&Bounds1,&Bounds2,1))
				return FALSE;
		}
	}
	if (nNamesOnScreen < MaxNamesOnScreen)
	{
		NamesOnScreenBuffer[nNamesOnScreen].BitmapID = BitmapID;
		NamesOnScreenBuffer[nNamesOnScreen].Size = Size;
		NamesOnScreenBuffer[nNamesOnScreen].TextSize = TextSize;
		NamesOnScreenBuffer[nNamesOnScreen].TextColor = TextColor;
		NamesOnScreenBuffer[nNamesOnScreen].TextShadowColor = TextShadowColor;
		strncpy (NamesOnScreenBuffer[nNamesOnScreen].Name,Name,nChar+1);
		NamesOnScreenBuffer[nNamesOnScreen++].Bounds = *pBounds;
		return TRUE;
	}
	return FALSE;
}

void SetShowContourLines (BOOL In)
{
	ShowContourLines = In;
	return;
}
void SetShowDepthColors (BOOL In)
{
	ShowDepthColors = In;
	return;
}
void SetHighlightDepth (int In)
{
	HighlightDepth = In;
	return;
}

int GridCellID (LPPOINT pPoint)
{
	int ID, Row, Col;

	Col = (int) ((pPoint->x + 1) - GridMinX) / GridWidth;
	Row = (int) ((pPoint->y + 1) - GridMinY) / GridHeight;
	if (Col >= nGridCol || Row >= nGridRow)
		return -1;
	ID = Row * nGridCol + Col;
	return ID;
}
int GridCellRow (LPPOINT pPoint)
{
	int Row;

	Row = (int) (pPoint->y - GridMinY - 1) / GridHeight;
	return Row;
}

BOOL GetGridBounds (int GridID,LPMNMXCORL pGridBounds)
{
	int	Row, Col;

	Row = GridID / nGridCol;
	Col = GridID % nGridCol;
	pGridBounds->xmn = GridMinX + GridWidth * Col;
	pGridBounds->xmx = GridMinX + GridWidth * (Col+1);
	pGridBounds->ymn = GridMinY + GridHeight * Row;
	pGridBounds->ymx = GridMinY + GridHeight * (Row+1);
	return TRUE;
}

POINT GetGridPoint (int GridID)
{
	int	Row, Col;
	POINT	pt;

	Row = GridID / nGridCol;
	Col = GridID % nGridCol;
	pt.x = GridMinX + GridWidth * Col;
	pt.y = GridMinY + GridHeight * Row;
	return pt;
}

BOOL LKMGetCardName (char *PathToLKMData,LPSTR CardName, int MaxLen )
{

	char	File[CL_MAX_PATH];
	CLFILE	*Fid;
	SETUPDATA	setupdata;

	*CardName = 0;
	sprintf (File,"%ssetup.bin",PathToLKMData);
	Fid = db_fopen (File,"rb");
	if (!Fid)
		return 0;
	db_fread (&setupdata,1,sizeof(setupdata),Fid);
	strncpy (CardName,setupdata.CardName,MaxLen);
  CardName[MaxLen - 1] = '\0';
	cl_fclose (Fid);
	return 1;
}

int LKMCheckCardCID (char *PathToLKMData,LPSTR CID)
{
	char	File[CL_MAX_PATH];
	CLFILE	*Fid;
	int		ioff;
	SETUPDATA	setupdata;
	char	SaveCID[sizeof(CIDValue)];
	char	Savedek = dek;
	int		rtn=0;

	strncpy (SaveCID,CIDValue,sizeof(CIDValue));
	sprintf (File,"%ssetup.bin",PathToLKMData);
	Fid = db_fopen (File,"rb");
	if (!Fid)
		return 0;
	strcpy (CIDValue,CID);
	db_fread (&setupdata,1,sizeof(setupdata),Fid);
	dek = setupdata.MapPrefix_SufFile_dek[3];
	sprintf (File,"%s%s%s",PathToLKMData,MapPrefix,GRIDDEFBIN);
	Fid = db_fopen (File,"rb");
	if (Fid)
	{
		if ((ioff = GetLakeOffset (Fid)) >= 0)
			rtn = 1;
		cl_fclose (Fid);
	}
	strncpy (CIDValue,SaveCID,sizeof(CIDValue));
	dek = Savedek;
	return rtn;
}

int LKMToHBInit (char *PathToLKMData,LPSTR CID,LPSTR CardName)
{
	char	File[CL_MAX_PATH];
	CLFILE	*Fid;
	int		i, ioff;
	SETUPDATA	setupdata;

	InitCalled = 1;
	ByteSwapNeeded = cl_ByteSwapNeeded();
	strcpy (CIDValue,CID);
	strcpy (LKMPath,PathToLKMData);
	sprintf (File,"%ssetup.bin",LKMPath);
	Fid = db_fopen (File,"rb");
	if (!Fid)
		return 0;
	db_fread (&setupdata,1,sizeof(setupdata),Fid);
	swap32 (&setupdata.UTMZone);
	swap32 (&setupdata.MaxDepthLayer);
	strncpy (MapPrefix,setupdata.MapPrefix_SufFile_dek,2);
	MapPrefix[2] = 0;
	UTMZone = setupdata.UTMZone;
	MaxDepthLayer = setupdata.MaxDepthLayer;
	MaxSubFileID = setupdata.MapPrefix_SufFile_dek[2];
	dek = setupdata.MapPrefix_SufFile_dek[3];
	strcpy (CardName,setupdata.CardName);

	strcpy (LKMPath,PathToLKMData);
	sprintf (File,"%s%s%s",LKMPath,MapPrefix,GRIDDEFBIN);
	Fid = db_fopen (File,"rb");
	if (!Fid)
		return 0;
	if ((ioff = GetLakeOffset (Fid)) < 0)
		return 0;
	db_fseek (Fid,0,SEEK_SET);
	NumGrids = cl_freadS32(Fid);
	for (i=0;i<NumGrids;i++)
		cl_freadGRIDDEF(&GridDefs[i],Fid);

	cl_fclose (Fid);

	LoadHBSymList (PathToLKMData);
	LoadHBParms (PathToLKMData);
	return UTMZone;
}

int GetClosestScaleIndex (double Scale)
{
	int	i;

	for (i=0;i<NumGrids;i++)
		if (Scale >= GridDefs[i].MetersPerPixel)
			return i;
	return NumGrids-1;
}

double AdjustToClosestScale (double Scale)
{
	int	i;


	for (i=0;i<NumGrids;i++)
		if (Scale >= GridDefs[i].MetersPerPixel)
			goto HaveGrid;
	return GridDefs[NumGrids-1].MetersPerPixel;

HaveGrid:
	if (i > 0)
	{
		if (Scale - GridDefs[i].MetersPerPixel > GridDefs[i-1].MetersPerPixel - Scale)
			Scale = GridDefs[i-1].MetersPerPixel;
		else
			Scale = GridDefs[i].MetersPerPixel;
	}
	return Scale;
}

void DecodeNameRec (LPCITYLAKENAMEOUTREC pLMNOutRec,LPCITYLAKENAMEREC pLMNRec)
{
	pLMNOutRec->PopulationOrAcres = pLMNRec->PopulationOrAcres;
	pLMNOutRec->y = HIWORD (pLMNRec->x_y);
	pLMNOutRec->x = LOWORD (pLMNRec->x_y);
	pLMNOutRec->nchar = HIWORD (pLMNRec->type_nchar);
	pLMNOutRec->type = LOWORD (pLMNRec->type_nchar);
	strncpy (pLMNOutRec->Name,pLMNRec->Name,sizeof(pLMNOutRec->Name));
	strncpy (pLMNOutRec->State,pLMNRec->State,2);
	return;
}
void DecodeTextRec (LPCTEXTOUTREC pTextOutRec,LPCTEXTREC pTextRec)
{
	pTextOutRec->CharNo = LOWORD (pTextRec->CharNo_Chr);
	pTextOutRec->chr = (char) HIWORD (pTextRec->CharNo_Chr);
	pTextOutRec->y = HIWORD (pTextRec->x_y);
	pTextOutRec->x = LOWORD (pTextRec->x_y);
	pTextOutRec->size = LOWORD (pTextRec->size_rot);
	pTextOutRec->rot = HIWORD (pTextRec->size_rot);
	return;
}

void DecodePointRec (LPMAPPOINTOUTREC pPointOutRec,LPMAPPOINTREC pPointRec)
{
	pPointOutRec->ref = pPointRec->ref;
	pPointOutRec->y = HIWORD (pPointRec->x_y);
	pPointOutRec->x = LOWORD (pPointRec->x_y);
	pPointOutRec->symno = LOWORD (pPointRec->symno_hasextendedtext);
	pPointOutRec->hasextendedtext = HIWORD (pPointRec->symno_hasextendedtext);
	pPointOutRec->type = 3;
	memcpy (pPointOutRec->PointText,pPointRec->PointText,64);
	if (*pPointOutRec->PointText || pPointOutRec->hasextendedtext == 1)
		pPointOutRec->type = 4;
	if (pPointOutRec->symno < 0)
	{
		pPointOutRec->type = 13;
		pPointOutRec->symno = -pPointOutRec->symno;
	}
	return;
}

int GetObjectText (LPMAPPOINTOUTREC pPointOutRec,HANDLE *TextHandle)
{
	int ObjectID;
	int	nChar=0;
	int		i, block, pos, loc, ID, storedlen;
	char	TextFile[CL_MAX_PATH];
	static	CLFILE	*Fid=0;
	CTEXTFILEHEADER	TextFileHeader;
	LPSTR	pText;

	if (!pPointOutRec)
	{
		if (Fid)
			cl_fclose (Fid);
		Fid = 0;
		return 0;
	}
	ObjectID = pPointOutRec->ref;
	if (*pPointOutRec->PointText)
	{
		*TextHandle = cl_malloc (66);
  ClAssert( NULL != *TextHandle );
		pText = (LPSTR)*TextHandle;
		strncpy (pText,pPointOutRec->PointText,64);
		pText[65] = 0;
		return (strlen (pText));
	}
	if (!pPointOutRec->hasextendedtext)
		return 0;
	if (!Fid)
	{
		sprintf (TextFile,"%s%s%s",LKMPath,MapPrefix,LKMTEXTBIN);

		Fid = db_fopen (TextFile,"rb");

		if (!Fid)
			return 0;
	}
	db_fseek (Fid,-(int)sizeof(CTEXTFILEHEADER),SEEK_END);
	cl_freadTEXTFILEHEADER( &TextFileHeader, Fid );

	db_fseek (Fid,TextFileHeader.FirstIndexLoc,SEEK_SET);
	for (i=0;i<TextFileHeader.NumIndexLevs;i++)
	{
		db_fread (&IndexRecordBlock,1,(TextFileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC),Fid);
		for (block=0; block<TextFileHeader.NumPerIndexRec+1; block++) {
			swapGRIDINDEXREC(&(IndexRecordBlock[block]));
		}
		for (pos=0;pos<TextFileHeader.NumPerIndexRec;pos++)
		{
			if (ObjectID < IndexRecordBlock[pos].GridCellID)
			{
				if (!pos)
					return 0;
				break;
			}
			else if (IndexRecordBlock[pos].loc < 0)
				break;
			else
				loc = IndexRecordBlock[pos].loc;
		}
		db_fseek (Fid,loc,SEEK_SET);
	}

	do
	{
		db_fseek (Fid,loc,SEEK_SET);		
		ID = cl_freadS32(Fid);
		storedlen = cl_freadS32(Fid);
		if (storedlen < 0)
			return 0;
		loc += 8 + storedlen;
	}while (ObjectID != ID);
	
	nChar = storedlen;
	*TextHandle = cl_malloc (nChar+1);
  ClAssert( NULL != *TextHandle );
	pText = (LPSTR)*TextHandle;
	db_fread (pText, 1, nChar, Fid);
	pText[nChar] = 0;

	cl_fclose (Fid);
	Fid = 0;
	return nChar;
}

BOOL FilterNearbyText (int Depth,int x, int y)
{
	int	i;
	LPTEXT_IN_IMAGE_RECORD	TIImageBuffer = TextInImageBuffer;

	if (DoFilter)
	{
		if (x < 0 || y < 0)
			return TRUE;
		if (x > ImageWidth || y > ImageHeight)
			return TRUE;
		for (i=0;i<nTextInImage;i++,TIImageBuffer++)
		{
			if (TIImageBuffer->depth == Depth)
			{
				if ((abs (x-TIImageBuffer->x) + abs (y-TIImageBuffer->y)) < SameDepthFilter)
					return TRUE;
			}
			else if ((abs (x-TIImageBuffer->x) + abs (y-TIImageBuffer->y)) < DiffDepthFilter)
				return TRUE;
		}
		if (nTextInImage < MAX_TEXT_IN_IMAGE)
		{
			TIImageBuffer->depth = Depth;
			TIImageBuffer->x = x;
			TIImageBuffer->y = y;
			nTextInImage++;
		}
		else
			ii=1;
	}
	return FALSE;
}

void DisplayTextChar (HANDLE HBImageHandle,char *chr, int nchar,int WorldX, int WorldY,int Rotation,int Size,BOOL DepthPoint)
{
	char	txt[8];
	int		i, Depth, ntxt;
	RGBQUAD	c;
	int		ShadowColor=0;
	POINT	BitmapPt=WorldPointToImagePoint (WorldX,WorldY);

	static	int	nDisplayed=0;

	nDisplayed++;
	if (!ALLOWCONTOURTEXTROTATION)
		Rotation = 0;
	if (nchar == 1 && *chr == '0')
		return;

	for (i=0;i<nchar;i++)
		txt[i] = chr[i];
	txt[i] = 0;
	Depth = atoi (txt) + DepthOffset;
	if (Depth <= 0 && ShowDepthOffsetArea)
		return;
	Depth = CL_MAX (0,Depth); 
	if (!DepthPoint && FilterNearbyText (Depth,BitmapPt.x,BitmapPt.y))
		return;
	sprintf (txt,"%i",Depth);
	ntxt = strlen (txt);
	if (ShowHazardAreas && Depth <= HazardDepth)
		c = HazardDepthColor;
	else
		c = DepthColor (Depth,HighlightDepth,HighlightDepthRange,ShowDepthColors);
	ShadowColor = RGB(c.rgbRed,c.rgbGreen,c.rgbBlue);
	if (!FlipText || (Rotation >= 0 && Rotation <= 900 || Rotation >= 2700))
		HBDisplayTextChar (HBImageHandle,txt,ntxt,WorldX,WorldY,Rotation,Size,0,ShadowColor);
	else
		HBDisplayTextChar (HBImageHandle,txt,ntxt,WorldX,WorldY,1800+Rotation,Size,0,ShadowColor);
	return;
}
CL_BOOL LakeMasterToHBirdImage2 (HANDLE HBImageHandle,int HBImageWidth,int HBImageHeight,int CenterX,int CenterY,double *pScale,int ForceLevel,int Rotation,
							int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange,BOOL AdjustScale,int *NumScreenPixelsCovered,
              ClChartRenderCb pfCallBack, void *pCallBackData,LPINT pRc )
{
	PLKMTILE	pLKMTile; 
	char	SubFileID='A';
	CL_BOOL bIsAbort = CL_FALSE;
	int	nTiles=0, RowsOnScreen, ColsOnScreen;
	int	StartStretchTime;

Top:
    GetLKMImageInit (HBImageWidth,HBImageHeight,CenterX, CenterY, *pScale,ForceLevel,SubFileID, Rotation,
					 WantDepthColors, WantContourLines, HighlightDepth, HighlightDepthRange,&TilePoint);
	while ( !bIsAbort && (pLKMTile = GetLKMImageTile (&TilePoint,pRc)))
	{
		nTiles++;
		ConvertHBirdColors (pLKMTile->PaletteLen,pLKMTile->Palette);
		
		LKMTrace ("Entering StretchHBBits");
		StartStretchTime = GetClockTicks ();
		NumStretchCalls++;
		if (pLKMTile->desty < 0)
			RowsOnScreen = CL_MAX (0,pLKMTile->desty + pLKMTile->desth);
		else if (pLKMTile->desty + pLKMTile->desth > ImageHeight)
			RowsOnScreen = CL_MAX (0,ImageHeight - pLKMTile->desty);
		else
			RowsOnScreen = pLKMTile->desth;
		if (pLKMTile->destx < 0)
			ColsOnScreen = CL_MAX (0,pLKMTile->destx + pLKMTile->destw);
		else if (pLKMTile->destx + pLKMTile->destw > ImageWidth)
			ColsOnScreen = CL_MAX (0,ImageWidth - pLKMTile->destx);
		else
			ColsOnScreen = pLKMTile->destw;
		
		*NumScreenPixelsCovered += RowsOnScreen * ColsOnScreen;
		StretchHBBits (HBImageHandle,
					   pLKMTile->destx,pLKMTile->desty,
	                   pLKMTile->destw, pLKMTile->desth,
	                   pLKMTile->xoff,pLKMTile->yoff,
	                   pLKMTile->rectw,pLKMTile->recth,
					   pLKMTile->Image,
	                   pLKMTile->width,pLKMTile->height,
					   pLKMTile->PaletteLen,
	                   pLKMTile->Palette);
		cl_free (pLKMTile);

		TotStretchTime += (GetClockTicks () - StartStretchTime);
		LKMTrace ("Exiting StretchHBBits");

		if (pfCallBack)
			bIsAbort = pfCallBack( HBImageHandle , pCallBackData );
		else
			bIsAbort = CL_FALSE;
	}
	GetLKMImageTerminate ();
	if (!bIsAbort && SubFileID < MaxSubFileID)
	{
		SubFileID++;
		goto Top;
	}
	return !bIsAbort;
}

BOOL LKMPostRotationProcessing (HANDLE HBImageHandle,int HBImageWidth,int HBImageHeight,int CenterX,int CenterY,double Scale,
								int DepthOff, int HighlightLMLakes,
								int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange,
								BOOL DisplayHazardAreas, int HazDepth,
								ClChartRenderCb pfCallBack, void *pCallBackData )
{
	PLKMCTEXTTILE	pLKMCTextTile;
	CL_BOOL bIsAbort = CL_FALSE;
	int	i;
	POINT	pt, WorldPt;
	MNMXCORL	TextBounds;
	double RangeInMeters = (Scale * CL_MAX (HBImageWidth,HBImageHeight))/2;
	HANDLE	enumHandle, ObjectHandle;
	char	SubFileID='A';
	char	BitmapPathName[CL_MAX_PATH];
	int		StartPRTime = GetClockTicks ();

	LKMTrace ("Entering PostRotationProcessing");
	TotPRTime = 0;

	ImageWidth  = HBImageWidth;
	ImageHeight = HBImageHeight;
	HBImageCenter.x = HBImageWidth / 2;
	HBImageCenter.y = HBImageHeight / 2;
	HBImageCenterWorld.x = CenterX;
	HBImageCenterWorld.y = CenterY;
	HBImageCenterWorld_100.x = CenterX * 100;
	HBImageCenterWorld_100.y = CenterY * 100;
	MetersPerPixel = Scale;
	MetersPerPixel_100000 = (int)(MetersPerPixel * 100000);
	HBImageBounds.xmn = CenterX - (int)(HBImageWidth/2 * Scale);
	HBImageBounds.xmx = CenterX + (int)(HBImageWidth/2 * Scale);
	HBImageBounds.ymn = CenterY - (int)(HBImageHeight/2 * Scale);
	HBImageBounds.ymx = CenterY + (int)(HBImageHeight/2 * Scale);
	MinDistBetweenShields = (int)(MetersPerPixel * 100);
	//Process contour text
	if ( !bIsAbort && WantContourLines && DisplayContourText && Scale < MAX_CONTOURTEXT_SCALE)
	{
NextCTextFile:
		GetLKMCTextInit (HBImageWidth,HBImageHeight,CenterX, CenterY, Scale,&TilePoint,SubFileID);
		while (!bIsAbort && (pLKMCTextTile = GetLKMCTextTile (&TilePoint)))
		{
			LPCTEXTREC	pTextRec = pLKMCTextTile->TextDef;
			CTEXTOUTREC	TextOutRec;
			char	txt[4], chr;
			int		nchr=0, x, y;
			int		rot;
			BOOL	DepthPoint;

			for (i=0;i<pLKMCTextTile->Numrecs && !bIsAbort ;i++,pTextRec++)
			{
				DecodeTextRec (&TextOutRec,pTextRec);
				if (!TextOutRec.CharNo)
				{
					if (nchr)
					{
						x /= nchr;
						y /= nchr;
						rot /= nchr;
						pt = TilePointToWorldPoint (x,y,&pLKMCTextTile->CellBounds);
						DisplayTextChar (HBImageHandle,txt,nchr,pt.x,pt.y,rot,TextOutRec.size,DepthPoint);
						nchr = 0;
					}
					rot = 0;
					x = 0;
					y = 0;
				}
				chr = TextOutRec.chr;
				if (chr == '!')
					DepthPoint = TRUE;
				else
				{
					x += TextOutRec.x;
					y += TextOutRec.y;
					rot += TextOutRec.rot;
					txt[nchr++] = chr;
					DepthPoint = FALSE;
				}
			}
			if (nchr)
			{
				x /= nchr;
				y /= nchr;
				rot /= nchr;
				pt = TilePointToWorldPoint (x,y,&pLKMCTextTile->CellBounds);
				DisplayTextChar (HBImageHandle,txt,nchr,pt.x,pt.y,rot,TextOutRec.size,DepthPoint);
				nchr = 0;
			}
			cl_free (pLKMCTextTile);

    	if (pfCallBack)
      		bIsAbort = pfCallBack( HBImageHandle , pCallBackData );
		else
			bIsAbort = FALSE;

		}
		GetLKMCTextTerminate ();
		if (!bIsAbort && SubFileID < MaxSubFileID)
		{
			SubFileID++;
			goto NextCTextFile;
		}
	}
	 //process chart objects
	if (!bIsAbort && Scale < MAX_CHARTOBJECT_SCALE)
	{

		if (LKMBeginEnumObjects (CenterX,CenterY,RangeInMeters,Scale, 3,&enumHandle ))
		{
			while(!bIsAbort && LKMEnumNextObject( enumHandle, &ObjectHandle  ))
			{
				int		BitmapID;

				LKMGetNavaidData (ObjectHandle,&WorldPt.x,&WorldPt.y,&BitmapID);
				sprintf (BitmapPathName,"%s%s%i.bmp",LKMPath,MapPrefix,BitmapID);
				HBDisplayBitmap (HBImageHandle,BitmapPathName,100,WorldPt.x,WorldPt.y);
				cl_free (ObjectHandle);
    			if (pfCallBack)
      				bIsAbort = pfCallBack( HBImageHandle , pCallBackData );
				else
					bIsAbort = FALSE;

			}
			LKMEndEnumObjects( enumHandle );
		}
	}
	MaxNamesOnScreen = (int) (((double)HBImageWidth * (double)HBImageHeight) / 2500);
	NamesOnScreenBuffer = cl_malloc (MaxNamesOnScreen * sizeof(NAMESONSCREEN));
  ClAssert( NULL != NamesOnScreenBuffer );
	nNamesOnScreen = 0;

	//Display chart text
	if (!bIsAbort && Scale < MAX_CHARTTEXT_SCALE)
	{
		if (LKMBeginEnumObjects (CenterX,CenterY,RangeInMeters,Scale, 4,&enumHandle ))
		{
			while(!bIsAbort && LKMEnumNextObject( enumHandle, &ObjectHandle  ))
			{
				char	text[1024];
				int		nChar;

				nChar = LKMGetChartText (ObjectHandle,4,&WorldPt.x,&WorldPt.y, text ,sizeof(text)-1 );
				TextBounds.xmn = (int) (WorldPt.x - MetersPerPixel * ChartTextSize/2 * (nChar/2 + 1));
				TextBounds.xmx = (int) (WorldPt.x + MetersPerPixel * ChartTextSize/2 * (nChar/2 + 1));
				TextBounds.ymn = (int) (WorldPt.y - MetersPerPixel * (ChartTextSize + 1));
				TextBounds.ymx = (int) (WorldPt.y + MetersPerPixel * (ChartTextSize + 1));
				RoomForNameOnScreen (text,nChar,0,0,ChartTextSize,ChartTextColor,ChartTextShadowColor,&TextBounds);
				cl_free (ObjectHandle);
    			if (pfCallBack)
      				bIsAbort = pfCallBack( HBImageHandle , pCallBackData );
				else
					bIsAbort = FALSE;

			}
			LKMEndEnumObjects( enumHandle );
		}
	}

	
//Display lake names
	if (!bIsAbort && Scale > MIN_LAKENAME_TEXT_SCALE)
	{
		if (LKMBeginEnumObjects (CenterX,CenterY,RangeInMeters,Scale,12,&enumHandle ))
		{
			LPENUMOBJECT	pEnumObject=(LPENUMOBJECT)enumHandle;
			int	iscale = CL_MAX (0,(int)(Scale - 50));

			MaxNamesPerTile = 1 + (int)(1024 * (((double)GridWidth * (double)GridHeight)/((double)(HBImageWidth * Scale)*(double)(HBImageHeight * Scale))));
			MinPopulation = (iscale * iscale)/7;
			while(!bIsAbort && LKMEnumNextName( enumHandle, &ObjectHandle  ))
			{
				char	text[1024];
				int		nChar;
				LPLKMOBJECT pObject=(LPLKMOBJECT)ObjectHandle;

				nChar = LKMGetChartText (ObjectHandle,0,&WorldPt.x,&WorldPt.y, text ,sizeof(text)-1 );
				TextBounds.xmn = (int) (WorldPt.x - MetersPerPixel * LakeTextSize/2 * (nChar/2 + 1));
				TextBounds.xmx = (int) (WorldPt.x + MetersPerPixel * LakeTextSize/2 * (nChar/2 + 1));
				TextBounds.ymn = (int) (WorldPt.y - MetersPerPixel * (LakeTextSize + 1));
				TextBounds.ymx = (int) (WorldPt.y + MetersPerPixel * (LakeTextSize + 1));
				RoomForNameOnScreen (text,nChar,0,pObject->Size,LakeTextSize,LakeTextColor,LakeTextShadowColor,&TextBounds);
				cl_free (ObjectHandle);
    			if (pfCallBack)
      				bIsAbort = pfCallBack( HBImageHandle , pCallBackData );
				else
					bIsAbort = FALSE;
			}
			LKMEndEnumObjects( enumHandle );
		}
	}

	//Display city names
	if (!bIsAbort && Scale > MIN_CITY_TEXT_SCALE)
	{
		if (LKMBeginEnumObjects (CenterX,CenterY,RangeInMeters,Scale,11,&enumHandle ))
		{
			LPENUMOBJECT	pEnumObject=(LPENUMOBJECT)enumHandle;
			int	iscale = CL_MAX (0,(int)(Scale - 50));

			MaxNamesPerTile = 1 + (int)(1024 * (((double)GridWidth * (double)GridHeight)/((double)(HBImageWidth * Scale)*(double)(HBImageHeight * Scale))));
			MinPopulation = (iscale * iscale)/50;
			while(!bIsAbort && LKMEnumNextName( enumHandle, &ObjectHandle  ))
			{
				char	text[1024];
				int		nChar;
				LPLKMOBJECT pObject=(LPLKMOBJECT)ObjectHandle;

				nChar = LKMGetChartText (ObjectHandle,11,&WorldPt.x,&WorldPt.y, text ,sizeof(text)-1 );
				TextBounds.xmn = (int) (WorldPt.x - MetersPerPixel * CityTextSize/2 * (nChar/2 + 1));
				TextBounds.xmx = (int) (WorldPt.x + MetersPerPixel * CityTextSize/2 * (nChar/2 + 1));
				TextBounds.ymn = (int) (WorldPt.y - MetersPerPixel * (CityTextSize + 1));
				TextBounds.ymx = (int) (WorldPt.y + MetersPerPixel * (CityTextSize + 1));
				RoomForNameOnScreen (text,nChar,0,pObject->Size,CityTextSize,CityTextColor,CityTextShadowColor,&TextBounds);
				cl_free (ObjectHandle);
    			if (pfCallBack)
      				bIsAbort = pfCallBack( HBImageHandle , pCallBackData );
				else
					bIsAbort = FALSE;
			}
			LKMEndEnumObjects( enumHandle );
		}
	}
	//Display highway shields
	if (!bIsAbort)
	{
		if (LKMBeginEnumObjects (CenterX,CenterY,RangeInMeters,Scale, 13,&enumHandle ))
		{
			int	id=1;
			while(!bIsAbort && LKMEnumNextObject( enumHandle, &ObjectHandle  ))
			{
				char	text[1024];
				int		nChar;
				LPLKMOBJECT	  pObject=(LPLKMOBJECT)ObjectHandle;

				if ((nChar = LKMGetChartText (ObjectHandle,13,&WorldPt.x,&WorldPt.y, text ,sizeof(text)-1 )))
				{
					TextBounds.xmn = (int) (WorldPt.x - MetersPerPixel * ChartTextSize/2 * (nChar/2 + 1));
					TextBounds.xmx = (int) (WorldPt.x + MetersPerPixel * ChartTextSize/2 * (nChar/2 + 1));
					TextBounds.ymn = (int) (WorldPt.y - MetersPerPixel * (ChartTextSize + 1));
					TextBounds.ymx = (int) (WorldPt.y + MetersPerPixel * (ChartTextSize + 1));
					RoomForNameOnScreen (text,nChar,pObject->BitmapID,0,ChartTextSize,ChartTextColor,ChartTextShadowColor,&TextBounds);
				}
				cl_free (ObjectHandle);
    			if (pfCallBack)
      				bIsAbort = pfCallBack( HBImageHandle , pCallBackData );
				else
					bIsAbort = FALSE;
			}
			LKMEndEnumObjects( enumHandle );
		}
	}

	if (!bIsAbort) for (i=0;i<nNamesOnScreen;i++)
	{
		int	nchr = strlen(NamesOnScreenBuffer[i].Name);

		WorldPt.x = NamesOnScreenBuffer[i].Bounds.xmn + (NamesOnScreenBuffer[i].Bounds.xmx - NamesOnScreenBuffer[i].Bounds.xmn)/2;
		WorldPt.y = NamesOnScreenBuffer[i].Bounds.ymn + (NamesOnScreenBuffer[i].Bounds.ymx - NamesOnScreenBuffer[i].Bounds.ymn)/2;
		if (NamesOnScreenBuffer[i].BitmapID)
		{
			char	width = 'a';

			if (nchr > 2)
				width = 'b';
			sprintf (BitmapPathName,"%ssh%i%c.bmp",LKMPath,NamesOnScreenBuffer[i].BitmapID,width);
			HBDisplayBitmap (HBImageHandle,BitmapPathName,100,WorldPt.x,WorldPt.y);
			switch (NamesOnScreenBuffer[i].BitmapID)
			{
			case 5001:
				NamesOnScreenBuffer[i].TextColor = RGB(255,255,255);
				NamesOnScreenBuffer[i].TextShadowColor = RGB(0,0,255);
			case 5002:
			case 5100:
			case 5200:
				NamesOnScreenBuffer[i].TextShadowColor = 0;
				break;
			default:
				break;
			}
		}
		HBDisplayTextChar (HBImageHandle,NamesOnScreenBuffer[i].Name,nchr,WorldPt.x,WorldPt.y,0,
			NamesOnScreenBuffer[i].TextSize,NamesOnScreenBuffer[i].TextColor,NamesOnScreenBuffer[i].TextShadowColor);
	}
	cl_free (NamesOnScreenBuffer);

	TotPRTime = GetClockTicks() - StartPRTime;
	if (Dbug)
	{
		char	dbmess[128];

		sprintf (dbmess,"PARM  X-%i  Y-%i  IW-%i  IH-%i  SCL-%f LEV-%i AB-%i IC-%i",HBImageCenterWorld.x, HBImageCenterWorld.y, ImageWidth,ImageHeight, Scale,LevUsed, bIsAbort,InitCalled);
		HBDisplayTextChar (HBImageHandle,dbmess,strlen(dbmess),HBImageCenterWorld.x,(int)(HBImageCenterWorld.y-30*MetersPerPixel),0,16,RGB(164,0,0),RGB(255,255,255));
		sprintf (dbmess,"DBTIMES  NC-%i  TI-%i  NS-%i  TS-%i  PR-%i RD-%i EX-%i",NumImageCalls, TotImageTime, NumStretchCalls,TotStretchTime, TotPRTime, totreadtime,totdecomptime);
		HBDisplayTextChar (HBImageHandle,dbmess,strlen(dbmess),HBImageCenterWorld.x,(int)(HBImageCenterWorld.y-60*MetersPerPixel),0,16,RGB(164,0,0),RGB(255,255,255));
		sprintf (dbmess,"IO  opens %i maxopentime %i seeks %i  maxtime %i  reads %i  totread %i",numopen,maxopentime,numseek,maxseektime,numreads,totread);
		HBDisplayTextChar (HBImageHandle,dbmess,strlen(dbmess),HBImageCenterWorld.x,(int)(HBImageCenterWorld.y-90*MetersPerPixel),0,16,RGB(164,0,0),RGB(255,255,255));

	}
	InitCalled = 0;
	LKMTrace ("Exiting PostRotationProcessing");
	return !bIsAbort;
}

CL_BOOL LakeMasterToHBirdImage (HANDLE HBImageHandle,int HBImageWidth,int HBImageHeight,int CenterX,int CenterY,double *pScale,
								int DepthOff, int HighlightLMLakes, int SeamLess,int Rotation,
								int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange,
								BOOL DisplayHazardAreas, int HazDepth,
								BOOL AdjustScale,
              ClChartRenderCb pfCallBack, void *pCallBackData ,LPINT pRc)
{
	int	i;
	CL_BOOL	Continue = CL_TRUE;
	int		StartImageTime = GetClockTicks ();
	int		NumScreenPixelsCovered;

	*pRc = 0;
	TotStretchTime = 0;
	TotImageTime = 0;
	NumStretchCalls = 0;
	NumImageCalls++;
	numseek = maxseektime = numreads = totread = totreadtime = totdecomptime = maxopentime = numopen = 0;

	HighlightLMLakes = CL_FALSE;
	if (AllowDbug && DepthOff == -1)
	{
		Dbug = 1;
		DepthOff = 0;
	}

	LKMTrace ("Entering LakeMasterToHBirdImage");
	
	ShowHazardAreas = DisplayHazardAreas;
	HazardDepth = HazDepth;
	DepthOffset = DepthOff;
	GrayMap = HighlightLMLakes;
	if (HighlightLMLakes)
		WantContourLines = FALSE;
	else if (WantContourLines && *pScale > MAX_CONTOURLINE_SCALE)
	{
		if (*pScale/2 > MAX_CONTOURLINE_SCALE)
			WantContourLines = FALSE;
		else
			WantContourLines = 5;
	}
	if (AdjustScale)
		*pScale = AdjustToClosestScale (*pScale);
	if (*pScale > MAX_CONTOURTEXT_SCALE || !WantContourLines)
		DisplayContourText = FALSE;
	else
		DisplayContourText = TRUE;
	for (i=0;i<NumGrids;i++)
		if (*pScale >= GridDefs[i].MetersPerPixel)
			goto HaveGrid;
	i--;
HaveGrid:
	SetGridVals (i);
	PixelsPerScreen = (HBImageWidth - 5) * (HBImageHeight - 5);
	NumScreenPixelsCovered = 0;
	if (SeamLess && i > LastFullCoverage && i < NumGrids - 1)
		Continue = LakeMasterToHBirdImage2 (HBImageHandle,HBImageWidth,HBImageHeight,CenterX,CenterY,pScale,i-1,Rotation,
											WantDepthColors,FALSE,HighlightDepth,HighlightDepthRange,FALSE,&NumScreenPixelsCovered,
						pfCallBack, pCallBackData,pRc );
	if (Continue)
	{
		do
		{
			NumScreenPixelsCovered = 0;
			Continue = LakeMasterToHBirdImage2 (HBImageHandle,HBImageWidth,HBImageHeight,CenterX,CenterY,pScale,i,Rotation,
												WantDepthColors,WantContourLines,HighlightDepth,HighlightDepthRange,FALSE,&NumScreenPixelsCovered,
							pfCallBack, pCallBackData,pRc );
			i--;
		}
		while (Continue && i > LastFullCoverage && NumScreenPixelsCovered < PixelsPerScreen);
	}
	
	TotImageTime = GetClockTicks () - StartImageTime;
	LKMTrace ("Exiting LakeMasterToHBirdImage");

	return Continue;
}

void GetLKMImageInit (int HBImageWidth,int HBImageHeight,int CenterX, int CenterY, double Scale, int ForceLevel,char SubFileID,int Rotation,
					  int WantDepthColors,int WantContourLines,int HltDepth,int HltDepthRange,LPPOINT pTilePoint)
{
	int		i, ioff;

	ImageWidth  = HBImageWidth;
	ImageHeight = HBImageHeight;
	HBImageCenter.x = HBImageWidth / 2;
	HBImageCenter.y = HBImageHeight / 2;
	HBImageCenterWorld.x = CenterX;
	HBImageCenterWorld.y = CenterY;
	HBImageCenterWorld_100.x = CenterX * 100;
	HBImageCenterWorld_100.y = CenterY * 100;
	MetersPerPixel = Scale;
	MetersPerPixel_100000 = (int)(MetersPerPixel * 100000);
	ShowContourLines    = WantContourLines;
	ShowDepthColors     = WantDepthColors;
	HighlightDepth	    = HltDepth;
	HighlightDepthRange = HltDepthRange;
	
	if (ForceLevel >= 0)
		i = ForceLevel;
	else
	{
		for (i=0;i<NumGrids;i++)
			if (Scale >= GridDefs[i].MetersPerPixel)
				goto HaveGrid;
		i--;
	}
HaveGrid:
	i = CL_MAX (FirstFullCoverage,i);
	SetGridVals (i);

#if !defined(HUMMINBIRD)
//	DisplayCurrentLayer (GridDefs[i].GridID);
#endif

	HBImageBounds.xmn = CenterX - (int)(HBImageWidth/2 * Scale);
	HBImageBounds.xmx = CenterX + (int)(HBImageWidth/2 * Scale);
	HBImageBounds.ymn = CenterY - (int)(HBImageHeight/2 * Scale);
	HBImageBounds.ymx = CenterY + (int)(HBImageHeight/2 * Scale);
	pTilePoint->x = HBImageBounds.xmn - GridWidth;
	pTilePoint->y = HBImageBounds.ymn;
	strcpy (ImageLibraryIndex,LKMPath);
	strcat (ImageLibraryIndex,MapPrefix);
	sprintf (strchr(ImageLibraryIndex,0),"%i%c.bin",GridDefs[i].GridID,SubFileID);
	LevUsed = GridDefs[i].GridID;
	FidIndex = db_fopen (ImageLibraryIndex,"rb");

	if (!FidIndex)
	{
		nGridCol = 0;
		return;
	}
	if ((ioff = GetLakeOffset (FidIndex)) < 0)
	{
		nGridCol = 0;
		return;
	}
	db_fseek (FidIndex,-(int)(ioff+sizeof(FileHeader)),SEEK_END);
	cl_freadGRIDFILEHEADER(&FileHeader,FidIndex);
	return;
}

void GetLKMImageTerminate (void)
{
	if (FidIndex)
		cl_fclose (FidIndex);
	FidIndex = NULL;
	return;
}
	
PLKMTILE GetLKMImageTile (LPPOINT pTilePoint,LPINT pRc)
{
	PLKMTILE pLKMTile;
	int	len, storedlen, compressedlen;
	static	int	nRead=0;
	int	CellID, ID, NextID, LastID;
	int	loc=-1,i,pos,block;
	MNMXCORL	CellBounds;
	POINT	pt;
static	int	calls=0;

	if (!nGridCol)
		return 0;
Top:
	pTilePoint->x += GridWidth;
	CellID = GridCellID (pTilePoint);
	if (CellID > -1)
	{
		GetGridBounds (CellID,&CellBounds);
		if (BoundsInBound (&CellBounds,&HBImageBounds,1))
			goto HaveTile;
	}
	pTilePoint->x = CL_MAX (GridMinX,HBImageBounds.xmn);
	pTilePoint->y += GridHeight;
	pTilePoint->y = CL_MAX (pTilePoint->y,GridMinY);
	CellID = GridCellID (pTilePoint);
	if (CellID < 0)
		return 0;
	GetGridBounds (CellID,&CellBounds);
	if (!BoundsInBound (&CellBounds,&HBImageBounds,1))
		return 0;
	
HaveTile:
	NextID = -1;
	db_fseek (FidIndex,FileHeader.FirstIndexLoc,SEEK_SET);
	for (i=0;i<FileHeader.NumIndexLevs;i++)
	{
		db_fread (&IndexRecordBlock,1,(FileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC),FidIndex);
calls++;
		for (block=0; block<FileHeader.NumPerIndexRec+1; block++) {
			swapGRIDINDEXREC(&(IndexRecordBlock[block]));
		}
		for (pos=0;pos<FileHeader.NumPerIndexRec;pos++)
		{
			if (CellID < IndexRecordBlock[pos].GridCellID)
			{
				if (!pos)
				{
					CellID = IndexRecordBlock[pos].GridCellID;
					GetGridBounds (CellID,&CellBounds);
					if (CellBounds.ymx < HBImageBounds.ymn)
					{
						pTilePoint->y = HBImageBounds.ymn;
						pTilePoint->x = CL_MAX (GridMinX,HBImageBounds.xmn);
					}
					else
					{
						pTilePoint->y = CellBounds.ymn;
						if (CellBounds.xmx < HBImageBounds.xmn)
							pTilePoint->x = HBImageBounds.xmn;
						else
							pTilePoint->x = CellBounds.xmn;
					}
					pTilePoint->x -= GridWidth;
					goto Top;
				}
				NextID = IndexRecordBlock[pos].GridCellID;
				break;
			}
			else if (IndexRecordBlock[pos].loc < 0)
				break;
			else
				loc = IndexRecordBlock[pos].loc;
		}
		if (loc < 0)
			return 0;
		db_fseek (FidIndex,loc,SEEK_SET);
	}
	LastID = -1;
	do
	{
		db_fseek (FidIndex,loc,SEEK_SET);
		db_fread (&ID_Length,1,8,FidIndex);
		swap32 (&ID_Length.ID);
		swap32 (&ID_Length.Length);
		ID = ID_Length.ID;
		storedlen = ID_Length.Length;
		if (storedlen < 0)
		{
			if (NextID > -1)
			{
				CellID = NextID;
				GetGridBounds (CellID,&CellBounds);
				if (CellBounds.ymx < HBImageBounds.ymn)
				{
					pTilePoint->y = HBImageBounds.ymn;
					pTilePoint->x = CL_MAX (GridMinX,HBImageBounds.xmn);
				}
				else
				{
					pTilePoint->y = CellBounds.ymn;
					if (CellBounds.xmx < HBImageBounds.xmn)
						pTilePoint->x = HBImageBounds.xmn;
					else
						pTilePoint->x = CellBounds.xmn;
				}
				pTilePoint->x -= GridWidth;
			}
			goto Top;
		}
		LastID = ID;
		loc += 8 + storedlen;
	}while (ID != CellID);
	
	len = sizeof (LKMTILE)+FileHeader.TileWidth*FileHeader.TileHeight + 1024 + 54;

	pLKMTile = (PLKMTILE)cl_malloc (len);
  ClAssert( NULL != pLKMTile );
	nRead += db_fread (pLKMTile->Image,sizeof(BYTE),storedlen,FidIndex);
	compressedlen = storedlen;// - sizeof(LKMTILE) + 4;
	if (DecompressTile (pLKMTile->Image,compressedlen))
	{
		pLKMTile->PaletteLen = 256;
		pLKMTile->width = FileHeader.TileWidth;
		pLKMTile->height = FileHeader.TileHeight;
		memmove (pLKMTile->Palette,&pLKMTile->Image[54],1024);
		memmove (pLKMTile->Image,&pLKMTile->Image[54+1024],pLKMTile->width*pLKMTile->height);

		pt = WorldPointToImagePoint (CellBounds.xmn,CellBounds.ymx);
		pLKMTile->destx = pt.x;
		pLKMTile->desty = pt.y;
		pt = WorldPointToImagePoint (CellBounds.xmx,CellBounds.ymn);
		pLKMTile->destw = pt.x - pLKMTile->destx;
		pLKMTile->desth = pt.y - pLKMTile->desty;
		pLKMTile->xoff = 0;
		pLKMTile->yoff = 0;
		pLKMTile->rectw = FileHeader.TileHeight;
		pLKMTile->recth = FileHeader.TileHeight;
		pLKMTile->CellID = CellID;
		pLKMTile->CellBounds = CellBounds;
	}
	else
	{
		*pRc = 1;
		cl_free (pLKMTile);
		pLKMTile = 0;
	}

	return pLKMTile;
}

BOOL DecompressTile (LPBYTE Image,int compressedlen)
{
	LPBYTE pCompressedImage = cl_malloc (compressedlen);
	LPBYTE pCompressedByteArray;
	int	lbin, lImage;
	int	StartTime = GetClockTicks();

  ClAssert( NULL != pCompressedImage );
   if (!pCompressedImage)
	  return FALSE;

	memmove (pCompressedImage,Image,compressedlen);
	lbin = DecompressBinaryRecord (Image,pCompressedImage,compressedlen); 
	pCompressedByteArray = (LPBYTE)cl_malloc (lbin);
  ClAssert( NULL != pCompressedImage );
	if (!pCompressedByteArray )
	  return FALSE;
	memmove (pCompressedByteArray,Image,lbin);
	lImage = DeCompressByteArray (pCompressedByteArray,Image,lbin);
	cl_free (pCompressedByteArray);
	cl_free (pCompressedImage);
	totdecomptime += (GetClockTicks()-StartTime);
	return TRUE;
}

#if HBIRDDEMO
BOOL InitLZO(void)
{   
	static BOOL	HaveLZOInit=FALSE;
	if (HaveLZOInit)
		return TRUE;
	if (lzo_init() != 0)
    	return FALSE;
    HaveLZOInit = TRUE;	
    return TRUE;
}
#endif

int DecompressBinaryRecord (LPBYTE pDecompressedRec,LPBYTE pCompressedRec,int CompressedLen)
{
	int		DCLen;
	int		rtn;

	if (InitLZO())
	{    
		rtn = lzo1x_decompress(pCompressedRec,(lzo_uint) CompressedLen,pDecompressedRec,(lzo_uint *) &DCLen,NULL);
		if (rtn == 0)
        	return DCLen;
	}
	return 0;
}

int DeCompressByteArray (LPBYTE pMemCmp,LPBYTE pMem,int lMem)
{
	int	lDeCmp=0;
	int	lNonRun;
	int	nRun=0;

	while (lMem > 0)
	{
		signed char	*plen2 = (signed char	*)pMemCmp++;
		int	len2 = *plen2;

		if (len2 > 0)
		{
			lNonRun = len2;
			memmove (pMem,pMemCmp,lNonRun);
			pMem += lNonRun;
			pMemCmp += lNonRun;
			lMem -= (lNonRun + 1);
			lDeCmp += lNonRun;
		}
		else
		{
			nRun = -len2;
			lDeCmp += nRun;
			while (nRun--)
			*pMem++ = *pMemCmp;
			lMem -= 2;
			pMemCmp++;
		}
	}
	return lDeCmp;
}

RGBQUAD ColorOfDepth (int Depth)
{
	RGBQUAD c;
#define COLORREDUCER	2

	c.rgbBlue = 255;
	Depth /= COLORREDUCER;
	Depth *= COLORREDUCER;
	Depth = CL_MIN (127,Depth);
	c.rgbGreen = c.rgbRed = CL_MAX (0,255 - Depth*2);
	return c;
}

RGBQUAD DepthColor (int Depth,int HighlightDepth,int HighlightDepthRange,BOOL ShowDepthColors)
{
	RGBQUAD c;
	RGBQUAD	ClassicDepthColor = {255,180,180,0};
	RGBQUAD	ClassicHighlightDepthColor = {255,128,128,0};

	c.rgbReserved = 0;
	if (!ShowDepthColors && (!HighlightDepth || !HighlightDepthRange))
		c = ClassicDepthColor;
	else if (!HighlightDepth)
		c = ColorOfDepth (Depth);
	else if (abs (Depth - HighlightDepth) <= HighlightDepthRange)
		c = HighlightDepthColor;
	else if (!ShowDepthColors)
		c = ClassicDepthColor;
	else
		c = ColorOfDepth (Depth);
	return c;
}

int GetTileElevAtPoint (int TilePtX,int TilePtY,PLKMTILE pLKMTile,LPSTR celev)
{
	int		rtn=0;

	if (!(TilePtX < 0 || TilePtX >= pLKMTile->width || TilePtY < 0 || TilePtY >= pLKMTile->height))
	{
		int		i = TilePtY * pLKMTile->width + TilePtX;
		int		PalletIndex = (int)*(LPBYTE)(pLKMTile->Image + i);
		RGBQUAD	color = pLKMTile->Palette[PalletIndex];
		int		Depth, Depth1, Depth2;
		char	Resolution[6]="";

		*celev = 0;
		if (color.rgbBlue > 253 && (color.rgbGreen != 255 || color.rgbRed != 255))
		{
			long	c=RGB(color.rgbRed,color.rgbGreen,color.rgbBlue);
			long	SymbolNumberColor = RGB(0,128,color.rgbBlue);	
			long	SymNum = c - SymbolNumberColor;

			if (Dbug)
			{
				if (color.rgbBlue == 254)
					strcpy (Resolution,"(SR)");
				else
					strcpy (Resolution,"(PM)");
			}
			if (SymNum >= minsym && SymNum <= maxsym)
			{
				SymNum -= minsym;
				if (!SymDepth[SymNum][1])
				{
					Depth = CL_MAX (0,SymDepth[SymNum][0] + DepthOffset);
					sprintf (celev,"%i feet %s",Depth,Resolution);
					rtn = 2;
				}
				else
				{
					Depth1 = CL_MAX (0,SymDepth[SymNum][0] + DepthOffset);
					Depth2 = CL_MAX (0,SymDepth[SymNum][1] + DepthOffset);
					if (Depth1 == Depth2)
						sprintf (celev,"%i feet %s",Depth1,Resolution);
					else
						sprintf (celev,"%i to %i feet %s",Depth1,Depth2,Resolution);
					rtn = 1;
				}
			}
		}
	}
	return rtn;
}

BOOL ConvertHBirdColors (int ClrUsed,RGBQUAD *pal)
{
	RGBQUAD	Black = {0,0,0,0};
	int		Depth, Depth1, Depth2;
	int		i;

	for (i=0;i<ClrUsed;i++,pal++)
	{
		if (*(LPINT)pal == *(LPINT)&StandardResColorIn)
			*pal = StandardResColor;
		else if (*(LPINT)pal == *(LPINT)&PromapColorIn)
			*pal = PromapColor;
/*		else if (GrayMap)
		{
			int luminance=(30L*pal->rgbRed+59L*pal->rgbGreen+11L*pal->rgbBlue)/100;  
			
			pal->rgbRed = pal->rgbGreen = pal->rgbBlue = luminance + (4*(255-luminance))/8;
		}*/

		else if (pal->rgbBlue > 253	&& (pal->rgbGreen != 255 || pal->rgbRed != 255))
		{
			long	c=RGB(pal->rgbRed,pal->rgbGreen,pal->rgbBlue);
			long	SymbolNumberColor = RGB(0,128,pal->rgbBlue);	
			long	SymNum = c - SymbolNumberColor;

			if (GrayMap)
			{
				if (pal->rgbBlue == 254)
					*pal = StandardResColor;
				else
					*pal = PromapColor;
			}
			else if (SymNum >= minsym && SymNum <= maxsym)
			{
				SymNum -= minsym;
				if (!SymDepth[SymNum][1])	//contour line symbol
				{
					Depth = SymDepth[SymNum][0] + DepthOffset;
					if (ShowContourLines)
					{
						if (Depth == HighlightDepth && !HighlightDepthRange)
							*pal = HighlightDepthColor;
						else if (Depth < 0 && ShowDepthOffsetArea)
							*pal = DepthOffsetColor;
						else if (ShowContourLines > 1 && Depth % (ShowContourLines-1))
							*pal = DepthColor (Depth,HighlightDepth,HighlightDepthRange,ShowDepthColors);
						else 
							*pal = Black;
					}
					else					
					{
						if (Depth < 0 && ShowDepthOffsetArea)
							*pal = DepthOffsetColor;
						else if (ShowHazardAreas && Depth < HazardDepth)
							*pal = HazardDepthColor;
						else
						{
							Depth = CL_MAX (0,Depth);
							*pal = DepthColor (Depth,HighlightDepth,HighlightDepthRange,ShowDepthColors);
						}
					}
				}
				else					//contour area symbol	
				{
					Depth1 = SymDepth[SymNum][0] + DepthOffset;
					Depth2 = SymDepth[SymNum][1] + DepthOffset;
					if (ShowDepthOffsetArea && (Depth1 < 0 || Depth2 < 0))
						*pal = DepthOffsetColor;
					else if (ShowHazardAreas && (Depth1 < HazardDepth || Depth2 < HazardDepth))
						*pal = HazardDepthColor;
					else
					{
						Depth = CL_MAX(0,(Depth1 + Depth2) / 2);
						*pal = DepthColor (Depth,HighlightDepth,HighlightDepthRange,ShowDepthColors);
					}
				}
			}
		}

	}
	return TRUE;
}

BOOL LoadHBSymList (LPSTR Path)
{
	char	SymListFile[CL_MAX_PATH];
	CLFILE	*Fid;
	sprintf (SymListFile,"%s%s%s",Path,MapPrefix,SYMLISTBIN);
	Fid = db_fopen (SymListFile,"rb");

	if (!Fid)
		return FALSE;
	minsym = cl_freadS16(Fid);
	maxsym = cl_freadS16(Fid);
	db_fread (SymDepth,1,(maxsym-minsym+1)*2,Fid);
	cl_fclose (Fid);
	return TRUE;
}

BOOL LoadHBParms (LPSTR Path)
{
	char	ParmFile[CL_MAX_PATH];
	CLFILE	*Fid;
	int		len;
	LPSTR	pCmd;
	LPSTR	pLoc;
	int		R,G,B;

	sprintf (ParmFile,"%sHBParm.txt",Path);
	Fid = db_fopen (ParmFile,"rb");

	if (!Fid)
		return FALSE;
	db_fseek (Fid,0,SEEK_END);
	len = cl_ftell (Fid)+1;
	db_fseek (Fid,0,SEEK_SET);
	pCmd = cl_malloc (len);
  ClAssert( NULL != pCmd );
	db_fread (pCmd,1,len,Fid);
	pCmd[len] = 0;
	cl_fclose (Fid);
	if ((pLoc = strstr (pCmd,DBUGPARM)))
		AllowDbug = atoi (pLoc+strlen(DBUGPARM));
	if ((pLoc = strstr (pCmd,SDFPARM)))
		SameDepthFilterBase = atoi (pLoc+strlen(SDFPARM));
	if ((pLoc = strstr (pCmd,DDFPARM)))
		DiffDepthFilterBase = atoi (pLoc+strlen(DDFPARM));
	if ((pLoc = strstr (pCmd,CHARTTSPARM)))
		ChartTextSize = atoi (pLoc+strlen(CHARTTSPARM));
	if ((pLoc = strstr (pCmd,CITYTSPARM)))
		CityTextSize = atoi (pLoc+strlen(CHARTTSPARM));
	if ((pLoc = strstr (pCmd,LAKETSPARM)))
		LakeTextSize = atoi (pLoc+strlen(LAKETSPARM));
	if ((pLoc = strstr (pCmd,CHARTTCPARM)))
	{
		sscanf (pLoc+strlen(CHARTTCPARM),"%i %i %i",&R,&G,&B);
		ChartTextColor = RGB(R,G,B);
	}
	if ((pLoc = strstr (pCmd,CITYTCPARM)))
	{
		sscanf (pLoc+strlen(CITYTCPARM),"%i %i %i",&R,&G,&B);
		CityTextColor = RGB(R,G,B);
	}
	if ((pLoc = strstr (pCmd,LAKETCPARM)))
	{
		sscanf (pLoc+strlen(LAKETCPARM),"%i %i %i",&R,&G,&B);
		LakeTextColor = RGB(R,G,B);
	}
	if ((pLoc = strstr (pCmd,HDCPARM)))
	{
		sscanf (pLoc+strlen(HDCPARM),"%i %i %i",&R,&G,&B);
		HazardDepthColor.rgbBlue = B;
		HazardDepthColor.rgbGreen = G;
		HazardDepthColor.rgbRed = R;
		HazardDepthColor.rgbReserved = 0;
	}
	if ((pLoc = strstr (pCmd,DOCPARM)))
	{
		sscanf (pLoc+strlen(DOCPARM),"%i %i %i",&R,&G,&B);
		DepthOffsetColor.rgbBlue = B;
		DepthOffsetColor.rgbGreen = G;
		DepthOffsetColor.rgbRed = R;
		DepthOffsetColor.rgbReserved = 0;
	}
	if ((pLoc = strstr (pCmd,SRCPARM)))
	{
		sscanf (pLoc+strlen(SRCPARM),"%i %i %i",&R,&G,&B);
		StandardResColor.rgbBlue = B;
		StandardResColor.rgbGreen = G;
		StandardResColor.rgbRed = R;
		StandardResColor.rgbReserved = 0;
	}
	if ((pLoc = strstr (pCmd,PMCPARM)))
	{
		sscanf (pLoc+strlen(PMCPARM),"%i %i %i",&R,&G,&B);
		PromapColor.rgbBlue = B;
		PromapColor.rgbGreen = G;
		PromapColor.rgbRed = R;
		PromapColor.rgbReserved = 0;
	}
	if ((pLoc = strstr (pCmd,MXCOSPARM)))
		MAX_CHARTOBJECT_SCALE = atoi (pLoc+strlen(MXCOSPARM));
	if ((pLoc = strstr (pCmd,MXCTSPARM)))
		MAX_CHARTTEXT_SCALE = atoi (pLoc+strlen(MXCTSPARM));
	if ((pLoc = strstr (pCmd,MLTSPARM)))
		MIN_LAKENAME_TEXT_SCALE = atoi (pLoc+strlen(MLTSPARM));
	if ((pLoc = strstr (pCmd,MNCTYTSPARM)))
		MIN_CITY_TEXT_SCALE = atoi (pLoc+strlen(MNCTYTSPARM));
	if ((pLoc = strstr (pCmd,MXCTLSPARM)))
		MAX_CONTOURLINE_SCALE = atoi (pLoc+strlen(MXCTLSPARM));
	if ((pLoc = strstr (pCmd,MXCTTSPARM)))
		MAX_CONTOURTEXT_SCALE = atoi (pLoc+strlen(MXCTTSPARM));

	return TRUE;
}

BOOL LKMBeginEnumObjectsText (int WorldPtX,int WorldPtY,double RangeInMeters,double Scale,HANDLE *enumHandle )
{
	LPENUMSTRUCT	pEnum;

	pEnum = cl_malloc (sizeof(ENUMSTRUCT));
  ClAssert( NULL != pEnum );
	*enumHandle = pEnum;
	pEnum->First = TRUE;
	pEnum->Done = FALSE;
	pEnum->RangeInMeters = RangeInMeters;
	pEnum->Scale = Scale;
	pEnum->CenterPoint.x = WorldPtX;
	pEnum->CenterPoint.y = WorldPtY;
	pEnum->PointEnumHandle = 0;

	return TRUE;
}

BOOL LKMEnumNextObjectText(HANDLE enumHandle, LPSTR text , int maxtextlen )
{
	LPENUMSTRUCT	pEnum=(LPENUMSTRUCT)enumHandle;
	LPENUMOBJECT	pEnumObject;
	HANDLE	ObjectHandle;
	char	SubFileID='A';
	int		i;
	int		NextType;
	int		Rc;

	BOOL			rtn=FALSE;

	MinPopulation = 0;
	if (pEnum->First)
	{
		PLKMTILE	pLKMTile;

		pEnum->First = FALSE;
		i = GetClosestScaleIndex (pEnum->Scale);
		if (i >= MaxDepthLayer)
		{
Top:
			GetLKMImageInit ((int)pEnum->RangeInMeters,(int)pEnum->RangeInMeters,
							 (int)(pEnum->CenterPoint.x + pEnum->RangeInMeters/2),(int)(pEnum->CenterPoint.y + pEnum->RangeInMeters/2),
							 1,-1,SubFileID,0,-1,0,0,0,&pEnum->TilePoint);
			if ((pLKMTile = GetLKMImageTile (&pEnum->TilePoint,&Rc)))
			{
				POINT	TilePt = WorldPointToTilePoint (pEnum->CenterPoint.x,pEnum->CenterPoint.y,pLKMTile);
				int		Offset=5, incx, incy;

				rtn = TRUE;
				for (incx=1;incx < Offset;incx++)
				{
					int	x = TilePt.x + incx;

					for (incy=1;incy < Offset;incy++)
					{
						int	y = TilePt.y + incy;

						if (GetTileElevAtPoint (x,y,pLKMTile,text) == 2)
							goto HaveContourLine;
						y = TilePt.y - incy;
						if (GetTileElevAtPoint (x,y,pLKMTile,text) == 2)
							goto HaveContourLine;
					}
					x = TilePt.x - incx;
					for (incy=1;incy < Offset;incy++)
					{
						int	y = TilePt.y + incy;

						if (GetTileElevAtPoint (x,y,pLKMTile,text) == 2)
							goto HaveContourLine;
						y = TilePt.y - incy;
						if (GetTileElevAtPoint (x,y,pLKMTile,text) == 2)
							goto HaveContourLine;
					}
				}
				rtn = GetTileElevAtPoint (TilePt.x,TilePt.y,pLKMTile,text);
	HaveContourLine:
				cl_free (pLKMTile);
			}
			GetLKMImageTerminate ();
			if (!rtn && SubFileID < MaxSubFileID)
			{
				SubFileID++;
				goto Top;
			}
			NextType = 3;
		}
		else
			NextType = 12;
		LKMBeginEnumObjects (pEnum->CenterPoint.x,pEnum->CenterPoint.y,
							 pEnum->RangeInMeters,pEnum->Scale, NextType,&pEnum->PointEnumHandle );
		if (rtn)
			return TRUE;
	}
	pEnumObject = (LPENUMOBJECT)pEnum->PointEnumHandle;
	if (pEnumObject->WantType == 12)
	{
		int	MaxSize=0;
		while (LKMEnumNextName(pEnum->PointEnumHandle, &ObjectHandle  ))
		{
			LPLKMOBJECT	  pObject = ObjectHandle;

			if (pObject->nChar && pObject->Size > MaxSize)
			{
				int	n = CL_MIN (pObject->nChar,maxtextlen);

				strncpy (text,pObject->Text,n);
				text[n] = 0;
				MaxSize = pObject->Size;
				rtn = TRUE;
			}
		}
	}
	else if (LKMEnumNextObject(pEnum->PointEnumHandle, &ObjectHandle  ))
	{
		LPLKMOBJECT	  pObject = ObjectHandle;

		if (pObject->nChar)
		{
			int	n = CL_MIN (pObject->nChar,maxtextlen);

			strncpy (text,pObject->Text,n);
			text[n] = 0;
		}
		else
		{
			LKMGetObjectNameFromID (pObject->BitmapID,text);
		}
		cl_free (ObjectHandle);
		rtn = TRUE;
	}
	return rtn;
}

void LKMEndEnumObjectsText(HANDLE enumHandle )
{
	if (enumHandle)
	{
		LPENUMSTRUCT	pEnum=(LPENUMSTRUCT)enumHandle;

		if (pEnum->PointEnumHandle)
			LKMEndEnumObjects(pEnum->PointEnumHandle);
		pEnum->PointEnumHandle = 0;
		cl_free (enumHandle);
	}
	return;
}

void GetLKMCTextInit (int HBImageWidth,int HBImageHeight,int CenterX, int CenterY, double Scale,LPPOINT pTilePoint,char SubFile)
{
	int		i, ioff;

	nGridCol = 0;
	ImageWidth  = HBImageWidth;
	ImageHeight = HBImageHeight;
	HBImageCenter.x = HBImageWidth / 2;
	HBImageCenter.y = HBImageHeight / 2;
	HBImageCenterWorld.x = CenterX;
	HBImageCenterWorld.y = CenterY;
	MetersPerPixel = Scale;

	sprintf (ImageLibraryIndex,"%s%sctext%c.bin",LKMPath,MapPrefix,SubFile);

	FidIndex = db_fopen (ImageLibraryIndex,"rb");

	if (!FidIndex)
		return;
	if ((ioff = GetLakeOffset (FidIndex)) < 0)
		return;
	db_fseek (FidIndex,-(int)(ioff+sizeof(CTextFileHeader)),SEEK_END);
	cl_freadTEXTFILEHEADER( &CTextFileHeader, FidIndex );
	for (i=0;i<NumGrids;i++)
		if (CTextFileHeader.GridID == GridDefs[i].GridID)
			goto HaveGrid;
	return;
HaveGrid:
	SetGridVals (i);
	CTextFileHeader.GridID = i;
	HBImageBounds.xmn = CenterX - (int)(HBImageWidth/2 * Scale);
	HBImageBounds.xmx = CenterX + (int)(HBImageWidth/2 * Scale);
	HBImageBounds.ymn = CenterY - (int)(HBImageHeight/2 * Scale);
	HBImageBounds.ymx = CenterY + (int)(HBImageHeight/2 * Scale);
	pTilePoint->x = HBImageBounds.xmn - GridWidth;
	pTilePoint->y = HBImageBounds.ymn;
	nTextInImage = 0;
	TextInImageBuffer = cl_malloc (MAX_TEXT_IN_IMAGE * sizeof(TEXT_IN_IMAGE_RECORD));
  ClAssert( NULL != TextInImageBuffer );
	return;
}

void GetLKMCTextTerminate (void)
{
	if (FidIndex)
		cl_fclose (FidIndex);
	FidIndex = NULL;
	if (TextInImageBuffer)
		cl_free (TextInImageBuffer);
	TextInImageBuffer = 0;
	return;
}
	
PLKMCTEXTTILE GetLKMCTextTile (LPPOINT pTilePoint)
{
	PLKMCTEXTTILE pLKMTile;
	int	len, storedlen;
	int	CellID, ID, NextID, LastID;
	int	loc,i,block,pos;
	MNMXCORL	CellBounds;
	static	int	nCellsRead=0;

	if (!nGridCol)
		return 0;
	nCellsRead++;

Top:
	SetGridVals (CTextFileHeader.GridID);
	pTilePoint->x += GridWidth;
	CellID = GridCellID (pTilePoint);
	if (CellID > -1)
	{
		GetGridBounds (CellID,&CellBounds);
		if (BoundsInBound (&CellBounds,&HBImageBounds,1))
			goto HaveTile;
	}
	pTilePoint->x = CL_MAX (GridMinX,HBImageBounds.xmn);
	pTilePoint->y += GridHeight;
	pTilePoint->y = CL_MAX (pTilePoint->y,GridMinY);
	CellID = GridCellID (pTilePoint);
	if (CellID < 0)
		return 0;
	GetGridBounds (CellID,&CellBounds);
	if (!BoundsInBound (&CellBounds,&HBImageBounds,1))
		return 0;
	
HaveTile:
	NextID = -1;
	db_fseek (FidIndex,CTextFileHeader.FirstIndexLoc,SEEK_SET);
	for (i=0;i<CTextFileHeader.NumIndexLevs;i++)
	{
		db_fread (&IndexRecordBlock,1,(CTextFileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC),FidIndex);
		nreads++;
		numread+=(CTextFileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC);

		for (block=0; block<CTextFileHeader.NumPerIndexRec+1; block++) {
			swapGRIDINDEXREC(&(IndexRecordBlock[block]));
		}
		for (pos=0;pos<CTextFileHeader.NumPerIndexRec;pos++)
		{
			if (CellID < IndexRecordBlock[pos].GridCellID)
			{
				if (!pos)
				{
					CellID = IndexRecordBlock[pos].GridCellID;
					GetGridBounds (CellID,&CellBounds);
					if (CellBounds.ymx < HBImageBounds.ymn)
					{
						pTilePoint->y = HBImageBounds.ymn;
						pTilePoint->x = CL_MAX (GridMinX,HBImageBounds.xmn);
					}
					else
					{
						pTilePoint->y = CellBounds.ymn;
						if (CellBounds.xmx < HBImageBounds.xmn)
							pTilePoint->x = HBImageBounds.xmn;
						else
							pTilePoint->x = CellBounds.xmn;
					}
					pTilePoint->x -= GridWidth;
					goto Top;
				}
				NextID = IndexRecordBlock[pos].GridCellID;
				break;
			}
			else if (IndexRecordBlock[pos].loc < 0)
				break;
			else
				loc = IndexRecordBlock[pos].loc;
		}
		db_fseek (FidIndex,loc,SEEK_SET);
	}
	LastID = -1;
	do
	{
		db_fseek (FidIndex,loc,SEEK_SET);
		db_fread (&ID_Length,1,8,FidIndex);
		swap32 (&ID_Length.ID);
		swap32 (&ID_Length.Length);
		ID = ID_Length.ID;
		storedlen = ID_Length.Length;

		if (storedlen < 0)
		{
			if (NextID > -1)
			{
				CellID = NextID;
				GetGridBounds (CellID,&CellBounds);
				if (CellBounds.ymx < HBImageBounds.ymn)
				{
					pTilePoint->y = HBImageBounds.ymn;
					pTilePoint->x = CL_MAX (GridMinX,HBImageBounds.xmn);
				}
				else
				{
					pTilePoint->y = CellBounds.ymn;
					if (CellBounds.xmx < HBImageBounds.xmn)
						pTilePoint->x = HBImageBounds.xmn;
					else
						pTilePoint->x = CellBounds.xmn;
				}
				pTilePoint->x -= GridWidth;
			}
			goto Top;
		}
		LastID = ID;
		loc += 8 + storedlen;
	}while (ID != CellID);
	
	len = sizeof (LKMCTEXTTILE)+storedlen;

	pLKMTile = (PLKMCTEXTTILE)cl_malloc (len);
  ClAssert( NULL != pLKMTile );
	
	pLKMTile->CellID = CellID;
	pLKMTile->Numrecs = storedlen / sizeof(CTEXTREC);
	pLKMTile->CellBounds = CellBounds;

	db_fread (pLKMTile->TextDef,1,pLKMTile->Numrecs*sizeof(CTEXTREC),FidIndex);
	nreads++;
	numread+=pLKMTile->Numrecs*sizeof(CTEXTREC);

	for (block=0; block < pLKMTile->Numrecs ; block++) {
		swapCTEXTREC( &(pLKMTile->TextDef[block]));
	}

	return pLKMTile;
}

BOOL LKMBeginEnumObjects (int CenterX, int CenterY, double RangeInMeters,double Scale,int WantType,HANDLE *penumHandle)
{
	char	ObjectLibrary[CL_MAX_PATH];
	int		i, iScale;
	LPENUMOBJECT	pEnumObject=cl_malloc (sizeof(ENUMOBJECT));

  ClAssert( NULL != pEnumObject );

	*penumHandle = 0;
	nGridCol = 0;

	pEnumObject->CenterWorld.x = CenterX;
	pEnumObject->CenterWorld.y = CenterY;
	pEnumObject->WantType = WantType;

	switch (WantType)
	{
	case 11: //city names
		sprintf (ObjectLibrary,"%s%s%s",LKMPath,MapPrefix,LKMNAMESBIN);
		break;
	case 12: //lake names
		sprintf (ObjectLibrary,"%s%s%s",LKMPath,MapPrefix,LKMLAKESBIN);
		break;
	case 13: //highway shields
		i = GetClosestScaleIndex (CL_MIN (800,Scale));
		iScale = GridDefs[i].GridID;
		sprintf (ObjectLibrary,"%s%s%iS.bin",LKMPath,MapPrefix,iScale);
		break;
	default:
		sprintf (ObjectLibrary,"%s%s%s",LKMPath,MapPrefix,LKMPOINTSBIN);
		break;
	}

	pEnumObject->Fid = db_fopen(ObjectLibrary,"rb");

	if (!pEnumObject->Fid)
	{
		cl_free (pEnumObject);
		return FALSE;
	}
	db_fseek (pEnumObject->Fid,-(int)sizeof(OBJECTFILEHEADER),SEEK_END);
	cl_freadOBJECTFILEHEADER( &pEnumObject->ObjectFileHeader, pEnumObject->Fid );
	for (i=0;i<NumGrids;i++)
		if (pEnumObject->ObjectFileHeader.GridID == GridDefs[i].GridID)
			goto HaveGrid;
	cl_free (pEnumObject);
	return FALSE;
HaveGrid:
	pEnumObject->GridID = i;
	SetGridVals (pEnumObject->GridID);

	pEnumObject->Bounds.xmn = CenterX - (int)(RangeInMeters+0.5);
	pEnumObject->Bounds.xmx = CenterX + (int)(RangeInMeters+0.5);
	pEnumObject->Bounds.ymn = CenterY - (int)(RangeInMeters+0.5);
	pEnumObject->Bounds.ymx = CenterY + (int)(RangeInMeters+0.5);
	pEnumObject->TilePoint.x = pEnumObject->Bounds.xmn - GridWidth;
	pEnumObject->TilePoint.y = pEnumObject->Bounds.ymn;
	pEnumObject->RecordHandle = 0;
	*penumHandle = pEnumObject;
	return TRUE;
}

void LKMEndEnumObjects (HANDLE enumHandle )
{
	LPENUMOBJECT	pEnumObject=(LPENUMOBJECT)enumHandle;

	GetObjectText (0,0);
	if (pEnumObject->Fid)
		cl_fclose (pEnumObject->Fid);
	if (pEnumObject->RecordHandle)
		cl_free (pEnumObject->RecordHandle);
	cl_free (enumHandle);
	return;
}

BOOL LKMEnumNextObject(HANDLE enumHandle, HANDLE *ObjectHandle  )	
{
	LPENUMOBJECT	pEnumObject=(LPENUMOBJECT)enumHandle;
	PLKMPOINTTILE pLKMTile;
	LPMAPPOINTREC pPointRec;
	MAPPOINTOUTREC	PointOutRec;
	LPMAPPOINTOUTREC pPointOutRec=&PointOutRec;
	LPLKMOBJECT	  pObject;
static	int nTiles=0;

	int	len, storedlen;
	int	ID, LastID, NextID;
	int	loc,i,block,pos;
	MNMXCORL	CellBounds;

	if (!enumHandle)
		return FALSE;

GetRec:
	if (pEnumObject->RecordHandle)
	{
		pLKMTile = pEnumObject->RecordHandle;
		while (pLKMTile->Nextrec < pLKMTile->Numrecs)
		{
			int	nTextChar=0;
			HANDLE	TextHandle=0;

			pPointRec = &pLKMTile->PointDef[pLKMTile->Nextrec++];

			DecodePointRec (pPointOutRec,pPointRec);
			if (pEnumObject->WantType >= 3)
				nTextChar = GetObjectText (pPointOutRec,&TextHandle);
			*ObjectHandle = cl_malloc (sizeof(LKMOBJECT)+nTextChar+1);
      ClAssert( NULL != *ObjectHandle );
			pObject = *ObjectHandle;
			pObject->Type = pPointOutRec->type;
			pObject->BitmapID = pPointOutRec->symno;
			pObject->nPoints = 1;
			pObject->Size  = 16;
			pObject->nChar = nTextChar;
			if (nTextChar)
			{
				LPSTR	pText = TextHandle;

				strcpy (pObject->Text,pText);
				cl_free (TextHandle);
			}
			pObject->WorldPoint = TilePointToWorldPoint (pPointOutRec->x,10000-pPointOutRec->y,&GridCellBounds);
			if (pObject->Type == pEnumObject->WantType && PointInBound (pObject->WorldPoint,&pEnumObject->Bounds))
				return TRUE;
			cl_free (*ObjectHandle);
		}
		cl_free (pEnumObject->RecordHandle);
		pEnumObject->RecordHandle = 0;
	}
Top:
	pEnumObject->TilePoint.x += GridWidth;
	pEnumObject->CellID = GridCellID (&pEnumObject->TilePoint);
	if (pEnumObject->CellID > -1)
	{
		GetGridBounds (pEnumObject->CellID,&CellBounds);
		if (BoundsInBound (&CellBounds,&pEnumObject->Bounds,1))
			goto HaveTile;
	}
	pEnumObject->TilePoint.x = CL_MAX (GridMinX,HBImageBounds.xmn);
	pEnumObject->TilePoint.y += GridHeight;
	pEnumObject->TilePoint.y = CL_MAX (pEnumObject->TilePoint.y,GridMinY);
	pEnumObject->CellID = GridCellID (&pEnumObject->TilePoint);
	if (pEnumObject->CellID < 0)
		return FALSE;
	GetGridBounds (pEnumObject->CellID,&CellBounds);
	if (!BoundsInBound (&CellBounds,&pEnumObject->Bounds,1))
		return FALSE;
	
HaveTile:
	NextID = -1;
	db_fseek (pEnumObject->Fid,pEnumObject->ObjectFileHeader.FirstIndexLoc,SEEK_SET);
	for (i=0;i<pEnumObject->ObjectFileHeader.NumIndexLevs;i++)
	{
		db_fread (&IndexRecordBlock,1,(pEnumObject->ObjectFileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC),pEnumObject->Fid);
		for (block=0; block<pEnumObject->ObjectFileHeader.NumPerIndexRec+1; block++) {
			swapGRIDINDEXREC(&(IndexRecordBlock[block]));
		}
		for (pos=0;pos<pEnumObject->ObjectFileHeader.NumPerIndexRec;pos++)
		{
			if (pEnumObject->CellID < IndexRecordBlock[pos].GridCellID)
			{
				if (!pos)
				{
					pEnumObject->CellID = IndexRecordBlock[pos].GridCellID;
					GetGridBounds (pEnumObject->CellID,&CellBounds);
					if (CellBounds.ymx < HBImageBounds.ymn)
					{
						pEnumObject->TilePoint.y = HBImageBounds.ymn;
						pEnumObject->TilePoint.x = CL_MAX (GridMinX,HBImageBounds.xmn);
					}
					else
					{
						pEnumObject->TilePoint.y = CellBounds.ymn;
						if (CellBounds.xmx < HBImageBounds.xmn)
							pEnumObject->TilePoint.x = HBImageBounds.xmn;
						else
							pEnumObject->TilePoint.x = CellBounds.xmn;
					}
					pEnumObject->TilePoint.x -= GridWidth;
					goto Top;
				}
				NextID = IndexRecordBlock[pos].GridCellID;
				break;
			}
			else if (IndexRecordBlock[pos].loc < 0)
				break;
			else
				loc = IndexRecordBlock[pos].loc;
		}
		db_fseek (pEnumObject->Fid,loc,SEEK_SET);
	}
	LastID = -1;
	do
	{
		db_fseek (pEnumObject->Fid,loc,SEEK_SET);
		db_fread (&ID_Length,1,8,pEnumObject->Fid);
		swap32 (&ID_Length.ID);
		swap32 (&ID_Length.Length);
		ID = ID_Length.ID;
		storedlen = ID_Length.Length;
		if (storedlen < 0)
		{
			if (NextID > -1)
			{
				pEnumObject->CellID = NextID;
				GetGridBounds (pEnumObject->CellID,&CellBounds);
				if (CellBounds.ymx < HBImageBounds.ymn)
				{
					pEnumObject->TilePoint.y = HBImageBounds.ymn;
					pEnumObject->TilePoint.x = CL_MAX (GridMinX,HBImageBounds.xmn);
				}
				else
				{
					pEnumObject->TilePoint.y = CellBounds.ymn;
					if (CellBounds.xmx < HBImageBounds.xmn)
						pEnumObject->TilePoint.x = HBImageBounds.xmn;
					else
						pEnumObject->TilePoint.x = CellBounds.xmn;
				}
				pEnumObject->TilePoint.x -= GridWidth;
			}
			goto Top;
		}
		LastID = ID;
		loc += 8 + storedlen;
	}while (ID != pEnumObject->CellID);
	
	len = sizeof (LKMPOINTTILE)+storedlen;

	pEnumObject->RecordHandle = (PLKMPOINTTILE)cl_malloc (len);
  ClAssert( NULL != pEnumObject->RecordHandle );
	pLKMTile = (PLKMPOINTTILE)pEnumObject->RecordHandle;

	pLKMTile->CellID = pEnumObject->CellID;
	GetGridBounds (pEnumObject->CellID,&GridCellBounds);
	pLKMTile->Numrecs = storedlen / sizeof(MAPPOINTREC);
	pLKMTile->Nextrec = 0;
	pLKMTile->CellBounds = CellBounds;
	db_fread (pLKMTile->PointDef,1,pLKMTile->Numrecs*sizeof(MAPPOINTREC),pEnumObject->Fid);

	for (block=0; block < pLKMTile->Numrecs ; block++) {
		swapMAPPOINTREC( &(pLKMTile->PointDef[block]) , pEnumObject->Fid );
	}

nTiles++;
	goto GetRec;
}

BOOL LKMEnumNextName (HANDLE enumHandle, HANDLE *ObjectHandle  )	
{
	LPENUMOBJECT	pEnumObject=(LPENUMOBJECT)enumHandle;
	PLKMNAMETILE pLKMTile;
	LPCITYLAKENAMEREC pPointRec;
	CITYLAKENAMEOUTREC	PointOutRec;
	LPCITYLAKENAMEOUTREC pPointOutRec=&PointOutRec;
	LPLKMOBJECT	  pObject;
static	int nTiles=0;

	int	len, storedlen;
	int	CellID, ID, LastID, NextID;
	int	loc,i,block,pos;
	MNMXCORL	CellBounds;

	if (!enumHandle)
		return FALSE;

GetRec:
	if (pEnumObject->RecordHandle)
	{
		pLKMTile = pEnumObject->RecordHandle;
		while (pLKMTile->Nextrec < pLKMTile->Numrecs && pLKMTile->NumSelected < MaxNamesPerTile)
		{
			int	Pop;

			pPointRec = &pLKMTile->PointDef[pLKMTile->Nextrec++];

			DecodeNameRec (pPointOutRec,pPointRec);
			*ObjectHandle = cl_malloc (sizeof(LKMOBJECT)+pPointOutRec->nchar+1);
      ClAssert( NULL != *ObjectHandle );
			pObject = *ObjectHandle;
			pObject->Type = pPointOutRec->type + 10;
			pObject->BitmapID = pPointOutRec->type;
			if (pPointOutRec->type == 2)
				pObject->nPoints = atoi (pPointOutRec->State);
			else
				pObject->nPoints = 1;
			pObject->Size  = pPointOutRec->PopulationOrAcres;
			pObject->nChar = pPointOutRec->nchar;
			strncpy (pObject->Text,pPointOutRec->Name,pPointOutRec->nchar+1);
			pObject->WorldPoint = TilePointToWorldPoint (pPointOutRec->x,10000-pPointOutRec->y,&GridCellBounds);
			Pop = pPointOutRec->PopulationOrAcres;
			if (pObject->Type == pEnumObject->WantType && Pop >= MinPopulation && PointInBound (pObject->WorldPoint,&pEnumObject->Bounds))
			{
				pLKMTile->NumSelected++;
				return TRUE;
			}
			cl_free (*ObjectHandle);
			if (Pop < MinPopulation)
				break;
		}
		cl_free (pEnumObject->RecordHandle);
		pEnumObject->RecordHandle = 0;
	}
Top:
	pEnumObject->TilePoint.x += GridWidth;
	CellID = GridCellID (&pEnumObject->TilePoint);
	if (CellID > -1)
	{
		GetGridBounds (CellID,&CellBounds);
		if (BoundsInBound (&CellBounds,&pEnumObject->Bounds,1))
			goto HaveTile;
	}
	pEnumObject->TilePoint.x = CL_MAX (GridMinX,HBImageBounds.xmn);
	pEnumObject->TilePoint.y += GridHeight;
	pEnumObject->TilePoint.y = CL_MAX (pEnumObject->TilePoint.y,GridMinY);
	CellID = GridCellID (&pEnumObject->TilePoint);
	if (CellID < 0)
		return FALSE;
	GetGridBounds (CellID,&CellBounds);
	if (!BoundsInBound (&CellBounds,&pEnumObject->Bounds,1))
		return FALSE;
	
HaveTile:
	NextID = -1;
	db_fseek (pEnumObject->Fid,pEnumObject->ObjectFileHeader.FirstIndexLoc,SEEK_SET);
	for (i=0;i<pEnumObject->ObjectFileHeader.NumIndexLevs;i++)
	{
		db_fread (&IndexRecordBlock,1,(pEnumObject->ObjectFileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC),pEnumObject->Fid);
		for (block=0; block<pEnumObject->ObjectFileHeader.NumPerIndexRec+1; block++) {
			swapGRIDINDEXREC(&(IndexRecordBlock[block]));
		}
		for (pos=0;pos<pEnumObject->ObjectFileHeader.NumPerIndexRec;pos++)
		{
			if (CellID < IndexRecordBlock[pos].GridCellID)
			{
				if (!pos)
				{
					CellID = IndexRecordBlock[pos].GridCellID;
					GetGridBounds (CellID,&CellBounds);
					if (CellBounds.ymx < HBImageBounds.ymn)
					{
						pEnumObject->TilePoint.y = HBImageBounds.ymn;
						pEnumObject->TilePoint.x = CL_MAX (GridMinX,HBImageBounds.xmn);
					}
					else
					{
						pEnumObject->TilePoint.y = CellBounds.ymn;
						if (CellBounds.xmx < HBImageBounds.xmn)
							pEnumObject->TilePoint.x = HBImageBounds.xmn;
						else
							pEnumObject->TilePoint.x = CellBounds.xmn;
					}
					pEnumObject->TilePoint.x -= GridWidth;
					goto Top;
				}
				NextID = IndexRecordBlock[pos].GridCellID;
				break;
			}
			else if (IndexRecordBlock[pos].loc < 0)
				break;
			else
				loc = IndexRecordBlock[pos].loc;
		}
		db_fseek (pEnumObject->Fid,loc,SEEK_SET);
	}
	LastID = -1;
	do
	{
		db_fseek (pEnumObject->Fid,loc,SEEK_SET);
		db_fread (&ID_Length,1,8,pEnumObject->Fid);
		swap32 (&ID_Length.ID);
		swap32 (&ID_Length.Length);
		ID = ID_Length.ID;
		storedlen = ID_Length.Length;
		if (storedlen < 0)
		{
			if (NextID > -1)
			{
				CellID = NextID;
				GetGridBounds (CellID,&CellBounds);
				if (CellBounds.ymx < HBImageBounds.ymn)
				{
					pEnumObject->TilePoint.y = HBImageBounds.ymn;
					pEnumObject->TilePoint.x = CL_MAX (GridMinX,HBImageBounds.xmn);
				}
				else
				{
					pEnumObject->TilePoint.y = CellBounds.ymn;
					if (CellBounds.xmx < HBImageBounds.xmn)
						pEnumObject->TilePoint.x = HBImageBounds.xmn;
					else
						pEnumObject->TilePoint.x = CellBounds.xmn;
				}
				pEnumObject->TilePoint.x -= GridWidth;
			}
			goto Top;
		}
		LastID = ID;
		loc += 8 + storedlen;
	}while (ID != CellID);
	
	len = sizeof (LKMNAMETILE)+storedlen;

	pEnumObject->RecordHandle = (PLKMPOINTTILE)cl_malloc (len);
  ClAssert( NULL != pEnumObject->RecordHandle );
	pLKMTile = (PLKMNAMETILE)pEnumObject->RecordHandle;

	pLKMTile->CellID = CellID;
	pLKMTile->Numrecs = storedlen / sizeof(CITYLAKENAMEREC);
	GetGridBounds (CellID,&GridCellBounds);
	pLKMTile->NumSelected = 0;
	pLKMTile->Nextrec = 0;
	pLKMTile->CellBounds = CellBounds;
	db_fread (pLKMTile->PointDef,1,pLKMTile->Numrecs*sizeof(CITYLAKENAMEREC),pEnumObject->Fid);

	for (block=0; block < pLKMTile->Numrecs ; block++) {
		swapCITYLAKENAMEREC( &(pLKMTile->PointDef[block]) , pEnumObject->Fid );
	}

nTiles++;
	goto GetRec;
}

BOOL LKMGetNavaidData (HANDLE ObjectHandle,int *x,int *y,int *BitmapID)
{
	LPLKMOBJECT	  pObject=(LPLKMOBJECT)ObjectHandle;

	if (pObject->Type != 3)
		return FALSE;
	*x = pObject->WorldPoint.x;
	*y = pObject->WorldPoint.y;
	*BitmapID = pObject->BitmapID;
	return TRUE;
}

int LKMGetChartText (HANDLE ObjectHandle,int WantType,int *x,int *y, char *text , int maxtextlen )
{
	int	rtn=0;
	LPLKMOBJECT	  pObject=(LPLKMOBJECT)ObjectHandle;

	if (WantType && pObject->Type != WantType)
		return FALSE;
	*x = pObject->WorldPoint.x;
	*y = pObject->WorldPoint.y;
	rtn = CL_MIN(pObject->nChar,maxtextlen);
	strncpy (text,pObject->Text,rtn+1);
	return rtn;
}

BOOL LKMGetHighwayShieldData (HANDLE ObjectHandle,int *x,int *y,int *ShieldID, char *text)
{
	return TRUE;
}

BOOL LKMGetObjectNameFromID (int ID,char *Name)
{
	char	ObjectList[CL_MAX_PATH];
	char	list[1024];
	char	searchstr[8]="|";
	CLFILE	*Fid;
	LPSTR	pLoc, pEnd;

	sprintf (ObjectList,"%s%s%s",LKMPath,MapPrefix,OBJECTLISTTXT);

	Fid = db_fopen (ObjectList,"rb");

	if (!Fid)
		return FALSE;
	db_fread (list, 1, sizeof(list), Fid);
	cl_fclose (Fid);
	sprintf (&searchstr[1],"%i",ID);
	strcat (searchstr,",");
	pLoc = strstr (list,searchstr);
	if (pLoc)
	{
		pLoc += strlen (searchstr);
		pEnd = strchr (pLoc,'\r');
		strncpy (Name,pLoc,(int)(pEnd-pLoc));
		Name[(int)(pEnd-pLoc)] = 0;
		return TRUE;
	}
	return FALSE;
}

static void swap64(double *dval)
{
   unsigned char tempChar;
   union {
      unsigned char u8[8];
      double f64;
   } buf;


   if (cl_ByteSwapNeeded()) {
	  buf.f64 = *dval;
      tempChar = buf.u8[7];
      buf.u8[7] = buf.u8[0];
      buf.u8[0] = tempChar;
      tempChar = buf.u8[6];
      buf.u8[6] = buf.u8[1];
      buf.u8[1] = tempChar;
      tempChar = buf.u8[5];
      buf.u8[5] = buf.u8[2];
      buf.u8[2] = tempChar;
      tempChar = buf.u8[4];
      buf.u8[4] = buf.u8[3];
      buf.u8[3] = tempChar;
	  *dval = buf.f64;
   }

   return;
}


static void cl_freadGRIDDEF(GRIDDEF *grid,CLFILE *pFid)
{
   db_fread (grid,1,36,pFid);
   swap32 (&grid->GridID);
   swap32 (&grid->GridMinX);
   swap32 (&grid->GridMinY);
   swap32 (&grid->GridMaxX);
   swap32 (&grid->GridMaxY);
   swap32 (&grid->GridWidth);
   swap32 (&grid->GridHeight);
   swap32 (&grid->nGridCol);
   swap32 (&grid->nGridRow);
   db_fread (&grid->MetersPerPixel,1,8,pFid);
   swap64 (&grid->MetersPerPixel);

   return;
}

static void cl_freadGRIDFILEHEADER(GRIDFILEHEADER *header,CLFILE *pFid)
{
   db_fread (header,1,sizeof(GRIDFILEHEADER),pFid);
   swap32 (&header->TileWidth);
   swap32 (&header->TileHeight);
   swap32 (&header->NumPerIndexRec);
   swap32 (&header->NumIndexLevs);
   swap32 (&header->FirstIndexLoc);
   swap32 (&header->NumRecs);

   return;
}

static void swapGRIDINDEXREC(GRIDINDEXREC *rec)
{
   swap32 (&rec->GridCellID);
   swap32 (&rec->loc);

   return;
}

static void cl_freadGRIDINDEXREC(GRIDINDEXREC *rec,CLFILE *pFID)
{
   rec->GridCellID = cl_freadS32(pFID);
   rec->loc = cl_freadS32(pFID);

   return;
}

static void cl_freadTEXTFILEHEADER( CTEXTFILEHEADER *header, CLFILE *pFid )
{  
  db_fread (header,1,sizeof(CTEXTFILEHEADER),pFid);
  swap32 (&header->GridID);
  swap32 (&header->NumPerIndexRec);
  swap32 (&header->NumIndexLevs);
  swap32 (&header->FirstIndexLoc);
  swap32 (&header->NumRecs);
  return;
}

static void cl_freadOBJECTFILEHEADER( OBJECTFILEHEADER *header, CLFILE *pFid )
{  
  db_fread (header,1,sizeof(OBJECTFILEHEADER),pFid);
  swap32 (&header->GridID);
  swap32 (&header->NumPerIndexRec);
  swap32 (&header->NumIndexLevs);
  swap32 (&header->FirstIndexLoc);
  swap32 (&header->NumRecs);
  return;
}

static void swapCTEXTREC( CTEXTREC *rec)
{
  swap32 (&rec->CharNo_Chr);
  swap32 (&rec->x_y);
  swap32 (&rec->size_rot);
  return;
}

static void cl_freadCTEXTREC( CTEXTREC *rec , CLFILE *pFID )
{
  rec->CharNo_Chr = cl_freadS32(pFID);
  rec->x_y = cl_freadS32(pFID);
  rec->size_rot = cl_freadS32(pFID);
}

static void cl_freadMAPPOINTREC( MAPPOINTREC *rec , CLFILE *pFID )
{
  rec->ref = cl_freadS32(pFID);
  rec->symno_hasextendedtext = cl_freadS32(pFID);
  rec->x_y = cl_freadS32(pFID);
  db_fread ( rec->PointText, sizeof(rec->PointText[0]), sizeof( rec->PointText ), pFID );
}

 void swapMAPPOINTREC( MAPPOINTREC *rec , CLFILE *pFID )
{
  swap32 (&rec->ref);
  swap32 (&rec->symno_hasextendedtext);
  swap32 (&rec->x_y);
}

 void swapCITYLAKENAMEREC( CITYLAKENAMEREC *rec , CLFILE *pFID )
{
  swap32 (&rec->PopulationOrAcres);
  swap32 (&rec->type_nchar);
  swap32 (&rec->x_y);
}

#if !defined(HUMMINBIRD)
#pragma warning(default : 4996) 
#endif
