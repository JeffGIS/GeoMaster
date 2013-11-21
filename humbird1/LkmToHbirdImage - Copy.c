#include	<stdlib.h>
#if !defined(HUMMINBIRD)
#include	<malloc.h>
#include	<fcntl.h>
#endif
#include	<stdio.h>
#include	<string.h>
#include	"lzoconf.h"
#include	"minilzo.h"
#include  "text.h"
#include "ClMalloc.h"
#include "ClAssert.h"

#if defined(HUMMINBIRD)
#include "fs_api.h"
#if !defined(HBIRDDEMO)
#define HBIRDDEMO 1
#endif
#endif

// FILE NAME CONSTANTS
#define SYMLISTBIN "symlst.bin"
#define LKMTEXTBIN "text.bin"
#define LKMCTEXTBIN "ctext.bin"
#define LKMPOINTSBIN "pnts.bin"
#define OBJECTLISTTXT "objlst.txt"
#define GRIDDEFBIN "grddef.bin"

#define ALLOWCONTOURTEXTROTATION 1

#define CL_MIN(x,y) ( (x)<(y) ? (x) : (y) )
#define CL_MAX(x,y) ( (x)>(y) ? (x) : (y) )

#if defined(HUMMINBIRD)
typedef FS_FILE CLFILE;
#define cl_fopen FS_FOpen
#define cl_fread FS_FRead
#define cl_fclose FS_FClose
#define cl_fseek FS_FSeek
#else
typedef FILE      CLFILE;
#define cl_fopen  fopen
#define cl_fread  fread
#define cl_fclose fclose
#define cl_fseek  fseek
#endif


#define LOWORD(l)           ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l)           ((WORD)((DWORD_PTR)(l) >> 16))
#define LOBYTE(w)           ((BYTE)((DWORD_PTR)(w) & 0xff))
#define HIBYTE(w)           ((BYTE)((DWORD_PTR)(w) >> 8))

#define	TRUE		1
#define FALSE		0
#define MAX_RECS_IN_BLOCK	33
#define MAX_TEXT_IN_IMAGE	256
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

typedef struct {int width, height;
				int	xoff,  yoff;
				int rectw, recth;
				int	destx, desty;
				int	destw, desth;
				int	CellID;
				MNMXCORL	CellBounds;
				int	PaletteLen;
				RGBQUAD	Palette[1024];
				BYTE Image[4];
				}LKMTILE;

typedef LKMTILE *PLKMTILE;

typedef	struct	{
				 short	CharNo;
				 int	chr;
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
				 short	symno,hastext;
				 short	type;
				 short	x,y;
				 char	ShieldText[4];
				 } MAPPOINTOUTREC;
typedef	MAPPOINTOUTREC	*LPMAPPOINTOUTREC;

typedef	struct	{
				 int	ref;
				 int	symno_hastext;
				 int	x_y;
				 char	ShieldText[4];
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
				POINT	CenterWorld;
				CLFILE	*Fid;
				MNMXCORL	Bounds;
				POINT	TilePoint;
				int		GridID;
				int		WantType;
				OBJECTFILEHEADER ObjectFileHeader;
				HANDLE	RecordHandle;
				}ENUMOBJECT;
typedef ENUMOBJECT	*LPENUMOBJECT;

typedef struct {
				int x,y;
				int	depth;
				}TEXT_IN_IMAGE_RECORD;
typedef TEXT_IN_IMAGE_RECORD	*LPTEXT_IN_IMAGE_RECORD;

char	LKMPath[MAX_PATH];

static	CLFILE	*FidIndex=NULL;
static	CLFILE	*FidTiles=NULL;
static	BYTE	SymDepth[1200][2];
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
static	int		SameDepthFilter, SameDepthFilterBase=60;
static	int		DiffDepthFilter, DiffDepthFilterBase=10;
static	int		LastFullCoverage = 4;
static	BOOL	GrayMap = FALSE;
static	int		DepthOffset = 0; 
static	int		HazardDepth = 5;
static	BOOL	ShowDepthOffsetArea=TRUE;
static	BOOL	ShowHazardAreas=TRUE;
static	RGBQUAD	HighlightDepthColor={0,243,8,0};
static	RGBQUAD	HazardDepthColor={5,0,236,0};
static	RGBQUAD	DepthOffsetColor={99,165,178,0};
static	RGBQUAD StandardResColor={240,240,0,0};
static	RGBQUAD PromapColor={255,128,0,0};
static	int		ChartTextColor=RGB(128,255,128);
static	int		ChartTextShadowColor=RGB(255,255,255);
static	int		ChartTextSize=14;
static	BOOL	ByteSwapNeeded;
static	int		MaxContourTextScale=5;
static	char	MaxSubFileID='D';
BOOL	DoFilter=TRUE, FlipText=TRUE; //for debugging only
static	int		nreads=0, numread=0;
static	char	MapPrefix[4]="mn";



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

static void swap32(long *lval);
static short cl_freadS16(CLFILE *pFID);
static long cl_freadS32(CLFILE *pFID);
static double cl_freadF64(CLFILE *pFID);
static void cl_freadGRIDDEF(GRIDDEF *grid,CLFILE *pFID);
static void cl_freadGRIDFILEHEADER(GRIDFILEHEADER *header,CLFILE *pFID);
static void cl_freadGRIDINDEXREC(GRIDINDEXREC *rec,CLFILE *pFID);
static void swapGRIDINDEXREC(GRIDINDEXREC *rec);
static void cl_freadTEXTFILEHEADER( CTEXTFILEHEADER *header, CLFILE *pFID );
static void cl_freadOBJECTFILEHEADER( OBJECTFILEHEADER *header, CLFILE *pFID );
static void cl_freadCTEXTREC( CTEXTREC *rec , CLFILE *pFID );
static void swapCTEXTREC( CTEXTREC *rec );
static void cl_freadMAPPOINTREC( MAPPOINTREC *rec , CLFILE *pFID );

RGBQUAD DepthColor (int Depth,int HighlightDepth,int HighlightDepthRange,BOOL ShowDepthColors);
BOOL ConvertHBirdColors (int ClrUsed,RGBQUAD *pal);
BOOL LoadHBSymList (LPSTR Path);

int StretchHBBits (HANDLE HBImageHandle,
				   int XDest, int YDest, int nDestWidth, int nDestHeight,
				   int XSrc,  int YSrc,  int nSrcWidth,  int nSrcHeight, 
				   BYTE *lpBits,
				   int TileHeight, int TileWidth,
				   int TilePaletteLen,
				   RGBQUAD *TilePalette);
void HBDisplayTextChar (HANDLE HBImageHandle,char *chr, int nchar,int WorldX, int WorldY,int Rotation,int Size,int TextColor,int ShadowColor);

void GetLKMImageInit (int HBImageWidth,int HBImageHeight,int CenterX, int CenterY, double Scale, int ForceLevel,char SubFileID,int Rotation,
					  int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange,LPPOINT pTilePoint);
void GetLKMImageTerminate (void);
PLKMTILE GetLKMImageTile (POINT *Point);
int DecompressBinaryRecord (LPBYTE pDecompressedRec,LPBYTE pCompressedRec,int CompressedLen);		
int DeCompressByteArray (LPBYTE pMemCmp,LPBYTE pMem,int lMem);
void DecompressTile (BYTE *Image,int compressedlen);
void GetLKMCTextTerminate (void);
PLKMCTEXTTILE GetLKMCTextTile (LPPOINT pTilePoint);
void GetLKMCTextInit (int HBImageWidth,int HBImageHeight,int CenterX, int CenterY, double Scale, LPPOINT pTilePoint);
POINT WorldToScreen (POINT WorldPoint);

#if !defined(HUMMINBIRD)
#pragma warning(disable : 4996) 
#endif

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
	SameDepthFilter = SameDepthFilterBase * (NumGrids-i);
	DiffDepthFilter = DiffDepthFilterBase * (NumGrids-i);
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

POINT TilePointToWorldPoint (int TileX, int TileY,LPMNMXCORL pCellBounds)
{
	POINT WorldPt;

	WorldPt.x = pCellBounds->xmn + (TileX * (pCellBounds->xmx - pCellBounds->xmn)) / 10000;
	WorldPt.y = pCellBounds->ymn + (TileY * (pCellBounds->ymx - pCellBounds->ymn)) / 10000;

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

int LKMToHBInit (char *PathToLKMData)
{
	char	File[MAX_PATH];
	CLFILE	*Fid;
	int		i;
	
	strcpy (LKMPath,PathToLKMData);
	sprintf (File,"%s%s%s",LKMPath,MapPrefix,GRIDDEFBIN);
	Fid = cl_fopen (File,"rb");
	if (!Fid)
		return 0;
	NumGrids = cl_freadS32(Fid);
	for (i=0;i<NumGrids;i++)
		cl_freadGRIDDEF(&GridDefs[i],Fid);

	cl_fclose (Fid);

	LoadHBSymList (PathToLKMData);
	ByteSwapNeeded = cl_ByteSwapNeeded();
	return 1;
}

double AdjustToClosestScale (double Scale)
{
	int	i;


	for (i=0;i<NumGrids;i++)
		if (Scale >= GridDefs[i].MetersPerPixel)
			goto HaveGrid;
	return GridDefs[NumGrids-1].MetersPerPixel;

HaveGrid:
/*	if (i == 0)
		Scale = GridDefs[0].MetersPerPixel;
	else*/
	if (i > 0)
	{
		if (Scale - GridDefs[i].MetersPerPixel > GridDefs[i-1].MetersPerPixel - Scale)
			Scale = GridDefs[i-1].MetersPerPixel;
		else
			Scale = GridDefs[i].MetersPerPixel;
	}
	return Scale;
}
/*typedef	struct	{
				 short	CharNo;
				 short	chr;
				 short	x,y;
				 short	size;
				 short	rot;} CTEXTOUTREC;
typedef	struct	{
				 int	CharNo_Chr;
				 int	x_y;
				 int	size_rot;
				 } CTEXTREC;*/

void DecodeTextRec (LPCTEXTOUTREC pTextOutRec,LPCTEXTREC pTextRec)
{
	pTextOutRec->CharNo = LOWORD (pTextRec->CharNo_Chr);
	pTextOutRec->chr = HIWORD (pTextRec->CharNo_Chr);
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
	pPointOutRec->symno = LOWORD (pPointRec->symno_hastext);
	pPointOutRec->hastext = HIWORD (pPointRec->symno_hastext);
	pPointOutRec->type = 3;
	memcpy (pPointOutRec->ShieldText,pPointRec->ShieldText,4);
	if (pPointOutRec->hastext == 1)
		pPointOutRec->type = 4;
	if (pPointOutRec->symno < 0)
	{
		pPointOutRec->type = 5;
		pPointOutRec->symno = -pPointOutRec->symno;
	}
	return;
}

int GetObjectText (LPMAPPOINTOUTREC pPointOutRec,HANDLE *TextHandle)
{
	int ObjectID = pPointOutRec->ref;
	int	nChar=0;
	int		i, block, pos, loc, ID, storedlen;
	char	TextFile[MAX_PATH];
	CLFILE	*Fid;
	CTEXTFILEHEADER	TextFileHeader;
	LPSTR	pText;

	if (*pPointOutRec->ShieldText)
	{
		*TextHandle = cl_malloc (5);
		pText = (LPSTR)*TextHandle;
		strncpy (pText,pPointOutRec->ShieldText,4);
		pText[4] = 0;
		return (strlen (pText));
	}
	sprintf (TextFile,"%s%s%s",LKMPath,MapPrefix,LKMTEXTBIN);

	Fid = cl_fopen (TextFile,"rb");

	if (!Fid)
		return 0;
	cl_fseek (Fid,-(int)sizeof(CTEXTFILEHEADER),SEEK_END);
  cl_freadTEXTFILEHEADER( &TextFileHeader, Fid );

	cl_fseek (Fid,TextFileHeader.FirstIndexLoc,SEEK_SET);
	for (i=0;i<TextFileHeader.NumIndexLevs;i++)
	{
		for (block=0; block<TextFileHeader.NumPerIndexRec+1; block++) {
			cl_freadGRIDINDEXREC(&(IndexRecordBlock[block]),Fid);
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
		cl_fseek (Fid,loc,SEEK_SET);
	}

	do
	{
		cl_fseek (Fid,loc,SEEK_SET);		
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
	cl_fread (pText, 1, nChar, Fid);
	pText[nChar] = 0;

	cl_fclose (Fid);
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
							int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange,BOOL AdjustScale,
              ClChartRenderCb pfCallBack, void *pCallBackData )
{
	PLKMTILE	pLKMTile;
	char	SubFileID='A';
  CL_BOOL bIsAbort = CL_FALSE;

Top:
    GetLKMImageInit (HBImageWidth,HBImageHeight,CenterX, CenterY, *pScale,ForceLevel,SubFileID, Rotation,
					 WantDepthColors, WantContourLines, HighlightDepth, HighlightDepthRange,&TilePoint);
	while ( !bIsAbort && (pLKMTile = GetLKMImageTile (&TilePoint)))
	{
		ConvertHBirdColors (pLKMTile->PaletteLen,pLKMTile->Palette);
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
	double RangeInMeters = Scale * HBImageWidth;
	HANDLE	enumHandle, ObjectHandle;
	
	HBImageCenter.x = HBImageWidth / 2;
	HBImageCenter.y = HBImageHeight / 2;
	HBImageCenterWorld.x = CenterX;
	HBImageCenterWorld.y = CenterY;
	HBImageCenterWorld_100.x = CenterX * 100;
	HBImageCenterWorld_100.y = CenterY * 100;
	MetersPerPixel = Scale;
	MetersPerPixel_100000 = (int)(MetersPerPixel * 100000);
	//Process contour text
	if ( !bIsAbort && WantContourLines && DisplayContourText && Scale < MaxContourTextScale)
	{
		GetLKMCTextInit (HBImageWidth,HBImageHeight,CenterX, CenterY, Scale,&TilePoint);
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
				chr = *(LPSTR)&TextOutRec.chr;
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
	}
	if (!bIsAbort) //process chart objects
	{

		if (LKMBeginEnumObjects (CenterX,CenterY,RangeInMeters,Scale, 3,&enumHandle ))
		{
			while(Scale < 20 && LKMEnumNextObject( enumHandle, &ObjectHandle  ))
			{
				int		BitmapID;
				char	BitmapPathName[MAX_PATH];

				LKMGetNavaidData (ObjectHandle,&WorldPt.x,&WorldPt.y,&BitmapID);
				sprintf (BitmapPathName,"%s%s%i.bmp",LKMPath,MapPrefix,BitmapID);
				HBDisplayBitmap (HBImageHandle,BitmapPathName,100,WorldPt.x,WorldPt.y);
				free (ObjectHandle);
			}
			LKMEndEnumObjects( enumHandle );
		}
	}
	//Display chart text
	if (!bIsAbort && Scale < 20 && LKMBeginEnumObjects (CenterX,CenterY,RangeInMeters,Scale, 4,&enumHandle ))
	{
		while(LKMEnumNextObject( enumHandle, &ObjectHandle  ))
		{
			char	text[1024];
			int		nChar;

			nChar = LKMGetChartText (ObjectHandle,&WorldPt.x,&WorldPt.y, text ,sizeof(text)-1 );
			HBDisplayTextChar (HBImageHandle,text,nChar,WorldPt.x,WorldPt.y,0,ChartTextSize,ChartTextColor,ChartTextShadowColor);
			free (ObjectHandle);
		}
		LKMEndEnumObjects( enumHandle );
	}

	//Display highway shields
	if (!bIsAbort && LKMBeginEnumObjects (CenterX,CenterY,RangeInMeters,Scale, 5,&enumHandle ))
	{
		while(LKMEnumNextObject( enumHandle, &ObjectHandle  ))
		{
			//ProcessHighwayShield (HBImageHandle,ObjectHandle);
			free (ObjectHandle);
		}
		LKMEndEnumObjects( enumHandle );
	}
	return bIsAbort;
}

CL_BOOL LakeMasterToHBirdImage (HANDLE HBImageHandle,int HBImageWidth,int HBImageHeight,int CenterX,int CenterY,double *pScale,
								int DepthOff, int HighlightLMLakes, int SeamLess,int Rotation,
							int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange,
							BOOL DisplayHazardAreas, int HazDepth,
							BOOL AdjustScale,
              ClChartRenderCb pfCallBack, void *pCallBackData )
{
	int	i;
	CL_BOOL	Continue = CL_TRUE;

	ShowHazardAreas = DisplayHazardAreas;
	HazardDepth = HazDepth;
	DepthOffset = DepthOff;
	GrayMap = HighlightLMLakes;
	if (HighlightLMLakes)
		WantContourLines = FALSE;
	if (AdjustScale)
		*pScale = AdjustToClosestScale (*pScale);
	if (*pScale > 10 || !WantContourLines)
		DisplayContourText = FALSE;
	else
		DisplayContourText = TRUE;
	for (i=0;i<NumGrids;i++)
		if (*pScale >= GridDefs[i].MetersPerPixel)
			goto HaveGrid;
	i--;
HaveGrid:
	SetGridVals (i);
	if (SeamLess && i > LastFullCoverage && i < NumGrids - 1)
		Continue = LakeMasterToHBirdImage2 (HBImageHandle,HBImageWidth,HBImageHeight,CenterX,CenterY,pScale,i-1,Rotation,
											WantDepthColors,FALSE,HighlightDepth,HighlightDepthRange,FALSE,
						pfCallBack, pCallBackData );
	if (Continue)
	{
		if (i == NumGrids - 1)
			i--;
		Continue = LakeMasterToHBirdImage2 (HBImageHandle,HBImageWidth,HBImageHeight,CenterX,CenterY,pScale,i,Rotation,
											WantDepthColors,WantContourLines,HighlightDepth,HighlightDepthRange,FALSE,
						pfCallBack, pCallBackData );
	}
	return Continue;
}

void GetLKMImageInit (int HBImageWidth,int HBImageHeight,int CenterX, int CenterY, double Scale, int ForceLevel,char SubFileID,int Rotation,
					  int WantDepthColors,int WantContourLines,int HltDepth,int HltDepthRange,LPPOINT pTilePoint)
{
	char	ImageLibraryIndex[MAX_PATH];
	int		i;

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
	SetGridVals (i);

	HBImageBounds.xmn = CenterX - (int)(HBImageWidth/2 * Scale);
	HBImageBounds.xmx = CenterX + (int)(HBImageWidth/2 * Scale);
	HBImageBounds.ymn = CenterY - (int)(HBImageHeight/2 * Scale);
	HBImageBounds.ymx = CenterY + (int)(HBImageHeight/2 * Scale);
	pTilePoint->x = HBImageBounds.xmn - GridWidth;
	pTilePoint->y = HBImageBounds.ymn;
	strcpy (ImageLibraryIndex,LKMPath);
	strcat (ImageLibraryIndex,MapPrefix);
	sprintf (strchr(ImageLibraryIndex,0),"%i%c.bin",GridDefs[i].GridID,SubFileID);

	FidIndex = cl_fopen (ImageLibraryIndex,"rb");

	if (!FidIndex)
	{
		nGridCol = 0;
		return;
	}
	cl_fseek (FidIndex,-(int)sizeof(FileHeader),SEEK_END);
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
	
PLKMTILE GetLKMImageTile (LPPOINT pTilePoint)
{
	PLKMTILE pLKMTile;
	int	len, storedlen, compressedlen;
	static	int	nRead=0;
	int	CellID, ID;
	int	loc=-1,i,pos,block;
	MNMXCORL	CellBounds;
	POINT	pt;

	if (!nGridCol)
		return 0;
Top:
	pTilePoint->x += GridWidth;
	CellID = GridCellID (pTilePoint);
	GetGridBounds (CellID,&CellBounds);
	if (BoundsInBound (&CellBounds,&HBImageBounds,1))
		goto HaveTile;
	pTilePoint->x = HBImageBounds.xmn;
	pTilePoint->y += GridHeight;
	CellID = GridCellID (pTilePoint);
	GetGridBounds (CellID,&CellBounds);
	if (!BoundsInBound (&CellBounds,&HBImageBounds,1))
		return 0;
	
HaveTile:
	cl_fseek (FidIndex,FileHeader.FirstIndexLoc,SEEK_SET);
	for (i=0;i<FileHeader.NumIndexLevs;i++)
	{
		cl_fread (&IndexRecordBlock,1,(FileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC),FidIndex);
		for (block=0; block<FileHeader.NumPerIndexRec+1; block++) {
			swapGRIDINDEXREC(&(IndexRecordBlock[block]));
		}
		for (pos=0;pos<FileHeader.NumPerIndexRec;pos++)
		{
			if (CellID < IndexRecordBlock[pos].GridCellID)
			{
				if (!pos)
					goto Top;
				break;
			}
			else if (IndexRecordBlock[pos].loc < 0)
				break;
			else
				loc = IndexRecordBlock[pos].loc;
		}
		if (loc < 0)
			return 0;
		cl_fseek (FidIndex,loc,SEEK_SET);
	}

	do
	{
		cl_fseek (FidIndex,loc,SEEK_SET);
		ID = cl_freadS32(FidIndex);
		storedlen = cl_freadS32(FidIndex);
		if (storedlen < 0)
			goto Top;
		loc += 8 + storedlen;
	}while (ID != CellID);
	
	len = sizeof (LKMTILE)+FileHeader.TileWidth*FileHeader.TileHeight + 1024 + 54;

	pLKMTile = (PLKMTILE)cl_malloc (len);
  ClAssert( NULL != pLKMTile );
	nRead += cl_fread (pLKMTile->Image,sizeof(BYTE),storedlen,FidIndex);
	compressedlen = storedlen;// - sizeof(LKMTILE) + 4;
	DecompressTile (pLKMTile->Image,compressedlen);
	pLKMTile->PaletteLen = 1024;
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

	return pLKMTile;
}

void DecompressTile (LPBYTE Image,int compressedlen)
{
	LPBYTE pCompressedImage = cl_malloc (compressedlen);
	LPBYTE pCompressedByteArray;
	int	lbin, lImage;

  ClAssert( NULL != pCompressedImage );

	memmove (pCompressedImage,Image,compressedlen);
	lbin = DecompressBinaryRecord (Image,pCompressedImage,compressedlen); 
	pCompressedByteArray = (LPBYTE)cl_malloc (lbin);
  ClAssert( NULL != pCompressedByteArray );
	memmove (pCompressedByteArray,Image,lbin);
	lImage = DeCompressByteArray (pCompressedByteArray,Image,lbin);
	cl_free (pCompressedByteArray);
  cl_free(pCompressedImage);
	return;
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
	int	ii;

	while (lMem > 0)
	{
		signed char	*plen2 = (signed char	*)pMemCmp++;
		int	len2 = *plen2;

		if (len2 == 0)
			ii=1;
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

RGBQUAD DepthColor (int Depth,int HighlightDepth,int HighlightDepthRange,BOOL ShowDepthColors)
{
	RGBQUAD c;
	RGBQUAD	ClassicDepthColor = {255,180,180,0};
	RGBQUAD	ClassicHighlightDepthColor = {255,128,128,0};

	c.rgbReserved = 0;
	if (!ShowDepthColors && (!HighlightDepth || !HighlightDepthRange))
		c = ClassicDepthColor;
	else if (!HighlightDepth)
	{
		c.rgbBlue = 255;
		c.rgbGreen = c.rgbRed = CL_MAX (0,255 - Depth*2);
	}
	else if (abs (Depth - HighlightDepth) <= HighlightDepthRange)
		c = HighlightDepthColor;
	else if (!ShowDepthColors)
		c = ClassicDepthColor;
	else
	{
		c.rgbBlue = 255;
		c.rgbGreen = c.rgbRed = CL_MAX (0,255 - Depth*2);
	}
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
		char	Resolution[6];

		*celev = 0;
		if (color.rgbBlue > 253 && (color.rgbGreen != 255 || color.rgbRed != 255))
		{
			long	c=RGB(color.rgbRed,color.rgbGreen,color.rgbBlue);
			long	SymbolNumberColor = RGB(0,128,color.rgbBlue);	
			long	SymNum = c - SymbolNumberColor;

			if (color.rgbBlue == 254)
				strcpy (Resolution,"(SR)");
			else
				strcpy (Resolution,"(PM)");
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
		if (*(LPINT)&pal == *(LPINT)&StandardResColor || *(LPINT)&pal == *(LPINT)&PromapColor)
			;
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
	char	SymListFile[MAX_PATH];
	CLFILE	*Fid;
	static	BOOL	Loaded=FALSE;

	if (Loaded)
		return TRUE;
	sprintf (SymListFile,"%s%s%s",Path,MapPrefix,SYMLISTBIN);
	Fid = cl_fopen (SymListFile,"rb");

	if (!Fid)
		return FALSE;
	minsym = cl_freadS16(Fid);
	maxsym = cl_freadS16(Fid);
	cl_fread (SymDepth,1,(maxsym-minsym+1)*2,Fid);
	cl_fclose (Fid);
	Loaded = TRUE;
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
	HANDLE	ObjectHandle;
	char	SubFileID='A';

	BOOL			rtn=FALSE;

	if (pEnum->First)
	{
		PLKMTILE	pLKMTile;

		pEnum->First = FALSE;
Top:
		GetLKMImageInit ((int)pEnum->RangeInMeters,(int)pEnum->RangeInMeters,
						 (int)(pEnum->CenterPoint.x + pEnum->RangeInMeters/2),(int)(pEnum->CenterPoint.y + pEnum->RangeInMeters/2),
						 1,-1,SubFileID,0,-1,0,0,0,&pEnum->TilePoint);
		if ((pLKMTile = GetLKMImageTile (&pEnum->TilePoint)))
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

		LKMBeginEnumObjects (pEnum->CenterPoint.x,pEnum->CenterPoint.y,
							 pEnum->RangeInMeters,pEnum->Scale, 3,&pEnum->PointEnumHandle );
		if (rtn)
			return TRUE;
	}

	if (LKMEnumNextObject(pEnum->PointEnumHandle, &ObjectHandle  ))
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

void GetLKMCTextInit (int HBImageWidth,int HBImageHeight,int CenterX, int CenterY, double Scale,LPPOINT pTilePoint)
{
	char	ImageLibraryIndex[MAX_PATH];
	int		i;

	nGridCol = 0;
	ImageWidth  = HBImageWidth;
	ImageHeight = HBImageHeight;
	HBImageCenter.x = HBImageWidth / 2;
	HBImageCenter.y = HBImageHeight / 2;
	HBImageCenterWorld.x = CenterX;
	HBImageCenterWorld.y = CenterY;
	MetersPerPixel = Scale;

	sprintf (ImageLibraryIndex,"%s%s%s",LKMPath,MapPrefix,LKMCTEXTBIN);

	FidIndex = cl_fopen (ImageLibraryIndex,"rb");

	if (!FidIndex)
		return;
	cl_fseek (FidIndex,-(int)sizeof(CTextFileHeader),SEEK_END);
	cl_freadTEXTFILEHEADER( &CTextFileHeader, FidIndex );
	for (i=0;i<NumGrids;i++)
		if (CTextFileHeader.GridID == GridDefs[i].GridID)
			goto HaveGrid;
	return;
HaveGrid:
	SetGridVals (i);

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
	cl_free (TextInImageBuffer);
	return;
}
	
PLKMCTEXTTILE GetLKMCTextTile (LPPOINT pTilePoint)
{
	PLKMCTEXTTILE pLKMTile;
	int	len, storedlen;
	int	CellID, ID;
	int	loc,i,block,pos;
	MNMXCORL	CellBounds;
	static	int	nCellsRead=0;

	if (!nGridCol)
		return 0;
	nCellsRead++;

Top:
	pTilePoint->x += GridWidth;
	CellID = GridCellID (pTilePoint);
	GetGridBounds (CellID,&CellBounds);
	if (BoundsInBound (&CellBounds,&HBImageBounds,1))
		goto HaveTile;
	pTilePoint->x = HBImageBounds.xmn;
	pTilePoint->y += GridHeight;
	CellID = GridCellID (pTilePoint);
	GetGridBounds (CellID,&CellBounds);
	if (!BoundsInBound (&CellBounds,&HBImageBounds,1))
		return 0;
	
HaveTile:
	cl_fseek (FidIndex,CTextFileHeader.FirstIndexLoc,SEEK_SET);
	for (i=0;i<CTextFileHeader.NumIndexLevs;i++)
	{
		cl_fread (&IndexRecordBlock,1,(CTextFileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC),FidIndex);
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
					goto Top;
				break;
			}
			else if (IndexRecordBlock[pos].loc < 0)
				break;
			else
				loc = IndexRecordBlock[pos].loc;
		}
		cl_fseek (FidIndex,loc,SEEK_SET);
	}

	do
	{
		cl_fseek (FidIndex,loc,SEEK_SET);
		ID = cl_freadS32(FidIndex);
		storedlen = cl_freadS32(FidIndex);
		if (storedlen < 0)
			goto Top;
		loc += 8 + storedlen;
	}while (ID != CellID);
	
	len = sizeof (LKMCTEXTTILE)+storedlen;

	pLKMTile = (PLKMCTEXTTILE)cl_malloc (len);
  ClAssert( NULL != pLKMTile );
	
	pLKMTile->CellID = CellID;
	pLKMTile->Numrecs = storedlen / sizeof(CTEXTREC);
	pLKMTile->CellBounds = CellBounds;

	cl_fread (pLKMTile->TextDef,1,pLKMTile->Numrecs*sizeof(CTEXTREC),FidIndex);
	nreads++;
	numread+=pLKMTile->Numrecs*sizeof(CTEXTREC);

	for (block=0; block < pLKMTile->Numrecs ; block++) {
		swapCTEXTREC( &(pLKMTile->TextDef[block]));
	}

	return pLKMTile;
}

BOOL LKMBeginEnumObjects (int CenterX, int CenterY, double RangeInMeters,double Scale,int WantType,HANDLE *penumHandle)
{
	char	ObjectLibrary[MAX_PATH];
	int		i;
	LPENUMOBJECT	pEnumObject=cl_malloc (sizeof(ENUMOBJECT));

  ClAssert( NULL != pEnumObject );

	*penumHandle = 0;
	nGridCol = 0;

	pEnumObject->CenterWorld.x = CenterX;
	pEnumObject->CenterWorld.y = CenterY;
	pEnumObject->WantType = WantType;

	sprintf (ObjectLibrary,"%s%s%s",LKMPath,MapPrefix,LKMPOINTSBIN);

	pEnumObject->Fid = cl_fopen(ObjectLibrary,"rb");

	if (!pEnumObject->Fid)
	{
		cl_free (pEnumObject);
		return FALSE;
	}
	cl_fseek (pEnumObject->Fid,-(int)sizeof(OBJECTFILEHEADER),SEEK_END);
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


	int	len, storedlen;
	int	CellID, ID;
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
			if (pEnumObject->WantType >= 3 && pPointOutRec->hastext)
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
			pObject->WorldPoint = TilePointToWorldPoint (pPointOutRec->x,pPointOutRec->y,&GridCellBounds);
			if (pObject->Type == pEnumObject->WantType && PointInBound (pObject->WorldPoint,&pEnumObject->Bounds))
				return TRUE;
			cl_free (*ObjectHandle);
		}
		cl_free (pEnumObject->RecordHandle);
		pEnumObject->RecordHandle = 0;
	}
Top:
	SetGridVals (pEnumObject->GridID);
	pEnumObject->TilePoint.x += GridWidth;
	CellID = GridCellID (&pEnumObject->TilePoint);
	GetGridBounds (CellID,&CellBounds);
	if (BoundsInBound (&CellBounds,&pEnumObject->Bounds,1))
		goto HaveTile;
	pEnumObject->TilePoint.x = HBImageBounds.xmn;
	pEnumObject->TilePoint.y += GridHeight;
	CellID = GridCellID (&pEnumObject->TilePoint);
	GetGridBounds (CellID,&CellBounds);
	if (!BoundsInBound (&CellBounds,&pEnumObject->Bounds,1))
		return FALSE;
	
HaveTile:
	cl_fseek (pEnumObject->Fid,pEnumObject->ObjectFileHeader.FirstIndexLoc,SEEK_SET);
	for (i=0;i<pEnumObject->ObjectFileHeader.NumIndexLevs;i++)
	{
		cl_fread (&IndexRecordBlock,1,(pEnumObject->ObjectFileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC),pEnumObject->Fid);
		for (block=0; block<pEnumObject->ObjectFileHeader.NumPerIndexRec+1; block++) {
			swapGRIDINDEXREC(&(IndexRecordBlock[block]));
		}
		for (pos=0;pos<pEnumObject->ObjectFileHeader.NumPerIndexRec;pos++)
		{
			if (CellID < IndexRecordBlock[pos].GridCellID)
			{
				if (!pos)
					goto Top;
				break;
			}
			else if (IndexRecordBlock[pos].loc < 0)
				break;
			else
				loc = IndexRecordBlock[pos].loc;
		}
		cl_fseek (pEnumObject->Fid,loc,SEEK_SET);
	}

	do
	{
		cl_fseek (pEnumObject->Fid,loc,SEEK_SET);
		ID = cl_freadS32(pEnumObject->Fid);
		storedlen = cl_freadS32(pEnumObject->Fid);
		if (storedlen < 0)
			goto Top;
		loc += 8 + storedlen;
	}while (ID != CellID);
	
	len = sizeof (LKMPOINTTILE)+storedlen;

	pEnumObject->RecordHandle = (PLKMPOINTTILE)cl_malloc (len);
  ClAssert( NULL != pEnumObject->RecordHandle );
	pLKMTile = (PLKMPOINTTILE)pEnumObject->RecordHandle;
	for (block=0; block < storedlen / (int)sizeof(MAPPOINTREC) ; block++) {
		cl_freadMAPPOINTREC( &(pLKMTile->PointDef[block]) , pEnumObject->Fid );
	}

	pLKMTile->CellID = CellID;
	GetGridBounds (CellID,&GridCellBounds);
	pLKMTile->Numrecs = storedlen / sizeof(MAPPOINTREC);
	pLKMTile->Nextrec = 0;
	pLKMTile->CellBounds = CellBounds;

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

int LKMGetChartText (HANDLE ObjectHandle,int *x,int *y, char *text , int maxtextlen )
{
	int	rtn=0;
	LPLKMOBJECT	  pObject=(LPLKMOBJECT)ObjectHandle;

	if (pObject->Type != 4)
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
	char	ObjectList[MAX_PATH];
	char	list[1024];
	char	searchstr[8]="|";
	CLFILE	*Fid;
	LPSTR	pLoc, pEnd;

	sprintf (ObjectList,"%s%s%s",LKMPath,MapPrefix,OBJECTLISTTXT);

	Fid = cl_fopen (ObjectList,"rb");

	if (!Fid)
		return FALSE;
	cl_fread (list, 1, sizeof(list), Fid);
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

static short cl_freadS16(CLFILE *pFID)
{
   unsigned char tempChar;
   const size_t numBytesToRead = 2;
   size_t numBytesRead = 0;
   union {
      unsigned char u8[2];
      short s16;
   } buf;

   buf.s16 = 0;

   numBytesRead = cl_fread(&(buf.u8),1,numBytesToRead,pFID);
   if (numBytesRead != numBytesToRead) return buf.s16;

   if (cl_ByteSwapNeeded()) {
      tempChar = buf.u8[1];
      buf.u8[1] = buf.u8[0];
      buf.u8[0] = tempChar;
   }

   return buf.s16;
}

static void swap32(long *lval)
{
   unsigned char tempChar;
   union {
      unsigned char u8[4];
      long s32;
   } buf;

   if (ByteSwapNeeded) {
	  buf.s32 = *lval;
      tempChar = buf.u8[3];
      buf.u8[3] = buf.u8[0];
      buf.u8[0] = tempChar;
      tempChar = buf.u8[2];
      buf.u8[2] = buf.u8[1];
      buf.u8[1] = tempChar;
	  *lval = buf.s32;
   }
   return;
}

static long cl_freadS32(CLFILE *pFID)
{
   unsigned char tempChar;
   const size_t numBytesToRead = 4;
   size_t numBytesRead = 0;
   union {
      unsigned char u8[4];
      long s32;
   } buf;

   buf.s32 = 0;

   numBytesRead = cl_fread(&(buf.u8),1,numBytesToRead,pFID);
   if (numBytesRead != numBytesToRead) return buf.s32;

   if (cl_ByteSwapNeeded()) {
      tempChar = buf.u8[3];
      buf.u8[3] = buf.u8[0];
      buf.u8[0] = tempChar;
      tempChar = buf.u8[2];
      buf.u8[2] = buf.u8[1];
      buf.u8[1] = tempChar;
   }
   nreads++;
   numread+=4;
   return buf.s32;
}

static double cl_freadF64(CLFILE *pFID)
{
   unsigned char tempChar;
   const size_t numBytesToRead = 8;
   size_t numBytesRead = 0;
   union {
      unsigned char u8[8];
      double f64;
   } buf;

   buf.f64 = 0;

   numBytesRead = cl_fread(&(buf.u8),1,numBytesToRead,pFID);
   if (numBytesRead != numBytesToRead) return buf.f64;

   if (cl_ByteSwapNeeded()) {
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
   }

   return buf.f64;
}

static void cl_freadGRIDDEF(GRIDDEF *grid,CLFILE *pFID)
{
   grid->GridID = cl_freadS32(pFID);
   grid->GridMinX = cl_freadS32(pFID);
   grid->GridMinY = cl_freadS32(pFID);
   grid->GridMaxX = cl_freadS32(pFID);
   grid->GridMaxY = cl_freadS32(pFID);
   grid->GridWidth = cl_freadS32(pFID);
   grid->GridHeight = cl_freadS32(pFID);
   grid->nGridCol = cl_freadS32(pFID);
   grid->nGridRow = cl_freadS32(pFID);
   grid->MetersPerPixel = cl_freadF64(pFID);

   return;
}

static void cl_freadGRIDFILEHEADER(GRIDFILEHEADER *header,CLFILE *pFID)
{
   header->TileWidth = cl_freadS32(pFID);
   header->TileHeight = cl_freadS32(pFID);
   header->NumPerIndexRec = cl_freadS32(pFID);
   header->NumIndexLevs = cl_freadS32(pFID);
   header->FirstIndexLoc = cl_freadS32(pFID);
   header->NumRecs = cl_freadS32(pFID);

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

static void cl_freadTEXTFILEHEADER( CTEXTFILEHEADER *header, CLFILE *pFID )
{  
  header->GridID = cl_freadS32(pFID);
  header->NumPerIndexRec = cl_freadS32(pFID);
  header->NumIndexLevs = cl_freadS32(pFID);
  header->FirstIndexLoc = cl_freadS32(pFID);
  header->NumRecs = cl_freadS32(pFID);
}

static void cl_freadOBJECTFILEHEADER( OBJECTFILEHEADER *header, CLFILE *pFID )
{  
  header->GridID = cl_freadS32(pFID);
  header->NumPerIndexRec = cl_freadS32(pFID);
  header->NumIndexLevs = cl_freadS32(pFID);
  header->FirstIndexLoc = cl_freadS32(pFID);
  header->NumRecs = cl_freadS32(pFID);
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
  rec->symno_hastext = cl_freadS32(pFID);
  rec->x_y = cl_freadS32(pFID);
  cl_fread( rec->ShieldText, sizeof(rec->ShieldText[0]), sizeof( rec->ShieldText ), pFID );
}

#if !defined(HUMMINBIRD)
#pragma warning(default : 4996) 
#endif
