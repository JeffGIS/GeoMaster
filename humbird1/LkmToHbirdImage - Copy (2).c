#include	<stdlib.h>
#include	<math.h>
#if !defined(HUMMINBIRD)
#include	<malloc.h>
#include	<fcntl.h>
#endif
#include	<stdio.h>
#include	<string.h>
#include <ctype.h>
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
#define SYMLIST2BIN "symls2.bin"
#define LKMTEXTBIN "text.bin"
#define LKMPOINTSBIN "pnts.bin"
#define LKMNAMESBIN "cities.bin"
#define LKMLAKESBIN "lnames.bin"
#define OBJECTLISTTXT "objlst.txt"
#define GRIDDEFBIN "grddef.bin"
#define CTEXTEXTENTSBIN "ctextX.bin"

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

#define TWOPI     6.283185307179586e0
#define	HALFPI    1.570796326794896e0 

#define	TRUE		1
#define FALSE		0
#define MAX_RECS_IN_BLOCK	65
#define MAX_TEXT_IN_IMAGE	2048
#define MAX_SUBFILE	32
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
				 int	RangeInPixels;
				 int	iScale;
				 POINT	pickedPoint;
				 int	pickedCellID;
				 int	pickedCellX;
				 int	pickedCellY;
				 int	pickedType;
				 RGBQUAD	pickedColor;
				 BOOL	First, Done;
				 HANDLE	PointEnumHandle;
				}ENUMSTRUCT;
typedef ENUMSTRUCT	*LPENUMSTRUCT;

typedef struct	{int	iFile;
				 int	iPos;
				 int	lReadAttempt;
				 int	lReadActual;
				 int	offset;
				}READBUFFERINDEX;
typedef	READBUFFERINDEX	*LPREADBUFFERINDEX;
#define	MAX_READ_BUFFER_INDEX_ENTRIES 1024
#define READ_BUFFER_SIZE  3145728
static	unsigned int	nInBuffer=0, ReadBufferLength=0;

static	LPREADBUFFERINDEX	pReadBufferIndex;
static	LPBYTE				pReadBuffer;
static int	StartTime;
static int	NumImageCalls=0;
static int	nPRTAborts=0, nImageAborts=0;
static int	TotStretchTime, TotImageTime, NumStretchCalls, TotPRTime;
int numseek,numreads,totread,NotNeededReads, totreadtime,maxseektime,numopen,maxopentime,totdecomptime;

int	currentCellID; //debug display of grid boundaries


#if defined(HUMMINBIRD)
unsigned int H_TICK_u32ReadFreeRunningTimerInMilliSeconds(void);
#elif 0
unsigned int GetTick (void);
{
  return GetTickCount();
}
#else
// Dummy function
unsigned int GetTick (void);


#endif

typedef struct {int width, height;
				int	xoff,  yoff;
				int rectw, recth;
				int	destx, desty;
				int	destw, desth;
				int	CellID;
				MNMXCORL	CellBounds;
				MNMXCORL	CellBounds64;
				int	PaletteLen;
				int	TrackColorPaletteIndex[2];
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
static	short	SymDep[2000][2];
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
static	int		DepthFactor=1;
static	MNMXCORL	SubFileBounds[MAX_SUBFILE];
static	int		nSubFiles;
static	int		CurSubFileLev=-1;
static	BOOL	WantExtendedText;

static	int	xoff1[8]={-1,0,1, 0,-1, 1, 1,-1};
static	int	yoff1[8]={ 0,1,0,-1, 1, 1,-1,-1};
static	int	xoff2[16]={-2,-2,-2,-1,0,1,2,2,2, 2, 2, 1, 0,-1,-2,-2};
static	int yoff2[16]={ 0, 1, 2, 2,2,2,2,1,0,-1,-2,-2,-2,-2,-2,-1};
static	int	xoff3[24]={-3,-3,-3,-3,-2,-1,0,1,2,3,3,3,3, 3, 3, 3, 2, 1, 0,-1,-2,-3,-3,-3};
static	int yoff3[24]={ 0, 1, 2, 3, 3, 3,3,3,3,3,2,1,0,-1,-2,-3,-3,-3,-3,-3,-3,-3,-2,-1};

char	CIDValue[66], dek;
BOOL	DoFilter=TRUE, FlipText=TRUE; int ii;//for debugging only
int		readloc[36];
int		readtim[36];
int		Dbug=0, AllowDbug=0;
#define MAXOPENEDFILES	32
static	char	OpenedFiles[MAXOPENEDFILES+1][10];
int		FileLength[MAXOPENEDFILES+1];
int		FilePos[MAXOPENEDFILES+1];
static	int		NumOpenedFiles;
int		CurrentOpenFile=0;
int		filetim[MAXOPENEDFILES+1];
int		filecnt[MAXOPENEDFILES+1];
static	int	ord[8];

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

static void swap32(long *lval);
static void cl_freadGRIDDEF(GRIDDEF *grid,CLFILE *pFID);
static void cl_freadGRIDFILEHEADER(GRIDFILEHEADER *header,CLFILE *pFID);
static void cl_freadGRIDINDEXREC(GRIDINDEXREC *rec,CLFILE *pFID);
static void swapGRIDINDEXREC(GRIDINDEXREC *rec);
static BOOL cl_freadTEXTFILEHEADER( CTEXTFILEHEADER *header, CLFILE *pFID );
static BOOL cl_freadOBJECTFILEHEADER( OBJECTFILEHEADER *header, CLFILE *pFID );
static BOOL cl_freadCTEXTREC( CTEXTREC *rec , CLFILE *pFID );
static void swapCTEXTREC( CTEXTREC *rec );
static void cl_freadMAPPOINTREC( MAPPOINTREC *rec , CLFILE *pFID );
static void swapMAPPOINTREC( MAPPOINTREC *rec , CLFILE *pFID );
static void swapCITYLAKENAMEREC( CITYLAKENAMEREC *rec , CLFILE *pFID );

RGBQUAD DepthColor (int Depth1,int Depth2,int HighlightDepth,int HighlightDepthRange,BOOL ShowDepthColors);
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
void setscreenpixel (POINT pt,COLORREF color);

void GetLKMImageInit (int HBImageWidth,int HBImageHeight,int CenterX, int CenterY, double Scale, int ForceLevel,char SubFileID,int Rotation,
					  int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange,LPPOINT pTilePoint);
void GetLKMImageTerminate (void);
PLKMTILE GetLKMImageTile (POINT *Point,LPIPILOTSTRUCT piPilot,LPINT pRc);
PLKMTILE GetLKMImageTileFromScaleAndID (int iScale,int TileID,LPIPILOTSTRUCT piPilot,LPINT pRc);
int DecompressBinaryRecord (LPBYTE pDecompressedRec,LPBYTE pCompressedRec,int CompressedLen);		
int DeCompressByteArray (LPBYTE pMemCmp,LPBYTE pMem,int lMem);
BOOL DecompressTile (BYTE *Image,int compressedlen);
void GetLKMCTextTerminate (void);
PLKMCTEXTTILE GetLKMCTextTile (LPPOINT pTilePoint);
void GetLKMCTextInit (int HBImageWidth,int HBImageHeight,int CenterX, int CenterY, double Scale, LPPOINT pTilePoint,char SubFile);
POINT WorldToScreen (POINT WorldPoint);
int ProcessiPilotCommand (LPIPILOTSTRUCT piPilot,double Scale);
void sleep (int i);

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

long    ININT (double X)
{
    if (X < 0)
        return ((long) (X - 0.5));
    return ((long) (X + 0.5));
}

double distp(POINT Point1, POINT Point2)
{
    return sqrt ((double)(Point1.x-Point2.x)*(double)(Point1.x-Point2.x) +
                 (double)(Point1.y-Point2.y)*(double)(Point1.y-Point2.y));
}

POINT inewpt (POINT OldPoint, double AZM, double DIS)
{
	  POINT NewPoint;

      NewPoint.x= ININT((OldPoint.x+DIS*cos(AZM)));
      NewPoint.y= ININT((OldPoint.y+DIS*sin(AZM)));
      return (NewPoint);
}

double BTWNTWOPI (double AZ1)
{
      if (AZ1<0)
      	return (TWOPI + fmod(AZ1,TWOPI));
      if (AZ1 == 0)
      	return (AZ1);
      return (fmod(AZ1,TWOPI));
}
double getazm (POINT Point1, POINT Point2)
{   
	double	rtn=BTWNTWOPI(atan2(((double)Point2.y-(double)Point1.y),((double)Point2.x-(double)Point1.x)));
	return rtn;
}

void OffsetTrackPoints (LPIPILOTSTRUCT piPilot)
{
	int	i;
	double	az;
	POINT	prevpt, newpnt;

	if (!piPilot->lenTrackArray || piPilot->offsetInMeters == 0)
		return;
	prevpt = piPilot->pTrackArray[0];
	for (i=1;i<piPilot->lenTrackArray;i++)
	{
		az = getazm (prevpt,piPilot->pTrackArray[i]);
		newpnt = inewpt (piPilot->pTrackArray[i],az-HALFPI,piPilot->offsetInMeters);
		prevpt = piPilot->pTrackArray[i];
		piPilot->pTrackArray[i] = newpnt;
	}
	return;
}

int ClearReadBuffer (BOOL Final,int iFile)
{
	nInBuffer = 0;
	ReadBufferLength = 0;
	if (Final)
	{
		cl_free (pReadBufferIndex);
		cl_free (pReadBuffer);
	}
	else if (NumOpenedFiles && iFile >= 0)
	{
		memmove (&OpenedFiles[0],&OpenedFiles[iFile],10);
		FileLength[0] = FileLength[iFile];
		FilePos[0] = FilePos[iFile];
		NumOpenedFiles = 1;
	}
	else
		NumOpenedFiles = 0;

	return 0;
}

int BufferedRead (CLFILE *Fid,int iFile,int lRead,LPBYTE pData)
{
	static	BOOL	first=TRUE;
	int		rtn=-1;
	static	unsigned int	i, nRepeats=0, nReads=0;

//	if (Dbug)
	{
		int		iPos;
		
		if (iFile == -1)
		{
			int	iPCT = (100 * nRepeats)/nReads;
			nRepeats=0;
			nReads=0;

			return iPCT;
		}
		nReads+=lRead;
		iPos = FilePos[iFile];
		if (first)
		{
			first = FALSE;
			pReadBufferIndex = cl_malloc (sizeof(READBUFFERINDEX)*MAX_READ_BUFFER_INDEX_ENTRIES);
			pReadBuffer		 = cl_malloc (READ_BUFFER_SIZE);
		}
		for (i=0;i<nInBuffer;i++)
		{
			if (iFile == pReadBufferIndex[i].iFile &&
				iPos == pReadBufferIndex[i].iPos &&
				lRead == pReadBufferIndex[i].lReadAttempt)
			{
				nRepeats+=lRead;
				//cl_fread (pData,1,lRead,Fid);
				if (pReadBufferIndex[i].lReadActual > 0)
				{
					memmove (pData,&pReadBuffer[pReadBufferIndex[i].offset],pReadBufferIndex[i].lReadActual);
					FilePos[iFile] += pReadBufferIndex[i].lReadActual;
				}
				return pReadBufferIndex[i].lReadActual;
			}
		}
		if (nInBuffer >= MAX_READ_BUFFER_INDEX_ENTRIES || ReadBufferLength + lRead >= READ_BUFFER_SIZE)
			iFile = ClearReadBuffer (FALSE,iFile);
		pReadBufferIndex[nInBuffer].iFile = iFile;
		pReadBufferIndex[nInBuffer].lReadAttempt = lRead;
		pReadBufferIndex[nInBuffer].iPos = iPos;
		pReadBufferIndex[nInBuffer++].offset = ReadBufferLength;
		cl_fseek (Fid,iPos,SEEK_SET);
		rtn = pReadBufferIndex[i].lReadActual = cl_fread (pData,1,lRead,Fid);
		if (rtn > 0)
		{
			memmove (&pReadBuffer[ReadBufferLength],pData,rtn);
			ReadBufferLength += rtn;
			FilePos[iFile] += rtn;
		}	
	}
	return rtn;
}

CLFILE *db_fopen2( char *filename, const char *mode )
{
	CLFILE *fid;
//   if (Dbug)
   {
	   LPSTR pFile = filename;
	   LPSTR pBS = strrchr (pFile,'\\');
	   int	i;
	   char	name[10];

	   if (pBS)
		   pFile = pBS;
	   if ((pBS = strrchr (pFile,'.')))
		   *pBS = 0;
	   strncpy (name,pFile,9);
	   name[9] = 0;
	   if (pBS)
		   *pBS = '.';
	   for (i=0;i<10;i++)
	   {
		   name[i] = tolower(name[i]);
	   }
	   for (i=0;i<NumOpenedFiles;i++)
	   {
		   if (!strcmp (name,OpenedFiles[i]))
		   {
			   CurrentOpenFile = i;
			   goto Exit;
		   }
	   }
	   if (NumOpenedFiles == MAXOPENEDFILES)
		   ClearReadBuffer (FALSE,-1);
	   CurrentOpenFile = NumOpenedFiles;
	   strncpy (OpenedFiles[NumOpenedFiles++],name,9);
  }
Exit:
   fid = db_fopen (filename,mode);
   FilePos[CurrentOpenFile] = 0;
   if (fid == NULL)
	   FileLength[CurrentOpenFile] = -1;
   else
   {
	   cl_fseek (fid,0,SEEK_END);
	   FileLength[CurrentOpenFile] = cl_ftell (fid);
   	   cl_fseek (fid,0,SEEK_SET);
   }
   return fid;
}

int FileNotInOrd (int i)
{
	int	j;

	for (j=0;j<8;j++)
		if (ord[j] == i)
			return 0;
	return 1;
}

void SortFileTimes (void)
{
	int	i,j;

	for (i=0;i<8;i++)
		ord[i] = NumOpenedFiles;

	for (j=0;j<8;j++)
	{
		int	slowesttim=-1;
		int	slowesti = NumOpenedFiles;

		for(i=0;i<NumOpenedFiles;i++)
		{
			if (FileNotInOrd (i) && filetim[i] > slowesttim)
			{
				slowesti = i;
				slowesttim = filetim[i];
			}
		}
		ord[j] = slowesti;
	}
	return;
}


CL_BOOL db_fread2(void * pbuf, int size,int count, CLFILE * fid)
{

	if (db_fread(pbuf,size,count,fid) <= 0)
		return CL_FALSE;
	return CL_TRUE;
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

int GetLeastUsedPaletteEntry (PLKMTILE pLKMTile)
{
	unsigned int i = pLKMTile->height * pLKMTile->width;
	LPBYTE	pImage = pLKMTile->Image;
	int	freq[256]={0}, minFreq;

	while (i--)
		freq[*pImage++]++;

	i = 256;
	while (i--)
	{
		if (freq[i])
			break;
	}

	if (i < 254)
	{
		pLKMTile->TrackColorPaletteIndex[0] = i+1;
		pLKMTile->TrackColorPaletteIndex[1] = i+2;
		return i+3;
	}
	minFreq = 1000000000;
	for (i=0;i<256;i++)
	{
		if (freq[i] < minFreq)
		{
			pLKMTile->TrackColorPaletteIndex[0] = i;
			minFreq = freq[i];
		}
	}
	freq[pLKMTile->TrackColorPaletteIndex[0]] = 2000000000;
	minFreq = 1000000000;
	for (i=0;i<256;i++)
	{
		if (freq[i] < minFreq)
		{
			pLKMTile->TrackColorPaletteIndex[1] = i;
			minFreq = freq[i];
		}
	}
	return 256;
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
	SameDepthFilter = (int)(SameDepthFilterBase * MetersPerPixel);
	DiffDepthFilter = (int)(DiffDepthFilterBase * MetersPerPixel);
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

POINT TilePointToImagePoint2 (int TileX, int TileY,PLKMTILE pLKMTile)
{
	POINT pt;
	double	WorldX, WorldY, x, y;

	WorldX = pLKMTile->CellBounds.xmn + (TileX * (double)(pLKMTile->CellBounds.xmx - pLKMTile->CellBounds.xmn)) / pLKMTile->width;
	WorldY = pLKMTile->CellBounds.ymn + (TileY * (double)(pLKMTile->CellBounds.ymx - pLKMTile->CellBounds.ymn)) / pLKMTile->height;
	x = HBImageCenter.x - (HBImageCenterWorld.x - WorldX) / MetersPerPixel;
	y = HBImageCenter.y + (HBImageCenterWorld.y - WorldY) / MetersPerPixel;
	if (x > 0)
		pt.x = (int)(x + 0.5);
	else
		pt.x = (int)(x - 0.5);
	if (y > 0)
		pt.y = (int)(y + 0.5);
	else
		pt.y = (int)(y - 0.5);
	return pt;
}

DPOINT ScreenPointToWorldPoint (int ScreenX, int ScreenY)
{
	DPOINT pt;

	pt.x = HBImageCenterWorld.x - ((HBImageCenter.x - ScreenX) * MetersPerPixel);
	pt.y = HBImageCenterWorld.y + ((HBImageCenter.y - ScreenY) * MetersPerPixel);
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

POINT TilePointToWorldPoint2 (int TileX, int TileY,PLKMTILE pLKMTile)
{
	POINT WorldPt;
	double	x,y;

	x = pLKMTile->CellBounds.xmn + (TileX * (double)(pLKMTile->CellBounds.xmx - pLKMTile->CellBounds.xmn)) / pLKMTile->width;
	y = pLKMTile->CellBounds.ymn + (TileY * (double)(pLKMTile->CellBounds.ymx - pLKMTile->CellBounds.ymn)) / pLKMTile->height;

	if (x > 0)
		WorldPt.x = (int)(x + 0.5);
	else
		WorldPt.x = (int)(x - 0.5);
	if (y > 0)
		WorldPt.y = (int)(y + 0.5);
	else
		WorldPt.y = (int)(y - 0.5);
	return WorldPt;
}
POINT TilePointToWorldPoint64 (int TileX, int TileY,PLKMTILE pLKMTile)
{
	POINT WorldPt;
	double	x,y;

	x = pLKMTile->CellBounds64.xmn + (TileX * (double)(pLKMTile->CellBounds64.xmx - pLKMTile->CellBounds64.xmn)) / pLKMTile->width;
	y = pLKMTile->CellBounds64.ymn + (TileY * (double)(pLKMTile->CellBounds64.ymx - pLKMTile->CellBounds64.ymn)) / pLKMTile->height;

	if (x > 0)
		WorldPt.x = (int)(x + 0.5);
	else
		WorldPt.x = (int)(x - 0.5);
	if (y > 0)
		WorldPt.y = (int)(y + 0.5);
	else
		WorldPt.y = (int)(y - 0.5);
	return WorldPt;
}

POINT WorldPoint64ToTilePoint (POINT WorldPt,PLKMTILE pLKMTile)
{
	POINT pt;

	pt.x = (pLKMTile->width * (WorldPt.x - pLKMTile->CellBounds64.xmn))/(pLKMTile->CellBounds64.xmx - pLKMTile->CellBounds64.xmn);
	pt.y = (pLKMTile->height * (WorldPt.y - pLKMTile->CellBounds64.ymn))/(pLKMTile->CellBounds64.ymx - pLKMTile->CellBounds64.ymn);
	return pt;
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

POINT GetGridPoint (int GridCellID)
{
	int	Row, Col;
	POINT	pt;

	Row = GridCellID / nGridCol;
	Col = GridCellID % nGridCol;
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
	Fid = db_fopen2 (File,"rb");
	if (!Fid)
		return 0;
	db_fread (&setupdata,5,sizeof(setupdata),Fid);
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
	Fid = db_fopen2 (File,"rb");
	if (!Fid)
		return 0;
	strcpy (CIDValue,CID);
	db_fread (&setupdata,6,sizeof(setupdata),Fid);
	dek = setupdata.MapPrefix_SufFile_dek[3];
	sprintf (File,"%s%s%s",PathToLKMData,MapPrefix,GRIDDEFBIN);
	Fid = db_fopen2 (File,"rb");
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
	Fid = db_fopen2 (File,"rb");
	if (!Fid)
		return 0;
	db_fread (&setupdata,7,sizeof(setupdata),Fid);
	cl_fclose (Fid);
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
	Fid = db_fopen2 (File,"rb");
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
	if (!pPointOutRec->hasextendedtext || !WantExtendedText)
		return 0;
	if (!Fid)
	{
		sprintf (TextFile,"%s%s%s",LKMPath,MapPrefix,LKMTEXTBIN);

		Fid = db_fopen2 (TextFile,"rb");

		if (!Fid)
			return 0;
	}
	db_fseek (Fid,-(int)sizeof(CTEXTFILEHEADER),SEEK_END);
	if (!cl_freadTEXTFILEHEADER( &TextFileHeader, Fid ))
		return 0;

	db_fseek (Fid,TextFileHeader.FirstIndexLoc,SEEK_SET);
	for (i=0;i<TextFileHeader.NumIndexLevs;i++)
	{
		if (!db_fread2 (&IndexRecordBlock,8,(TextFileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC),Fid))
			return 0;
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
	if (!db_fread2 (pText, 9, nChar, Fid))
		nChar = 0;
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
		if (x < HBImageBounds.xmn || y < HBImageBounds.ymn)
			return TRUE;
		if (x > HBImageBounds.xmx || y > HBImageBounds.ymx)
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

	static	int	nDisplayed=0;

	nDisplayed++;
	if (!ALLOWCONTOURTEXTROTATION)
		Rotation = 0;
	if (nchar < 1)
		return;
	if (nchar > 7)
		return;
	nchar = CL_MIN (nchar,7);
	if (nchar == 1 && *chr == '0')
		return;

	for (i=0;i<nchar;i++)
		txt[i] = chr[i];
	txt[i] = 0;
	Depth = atoi (txt) + DepthOffset;
	if (Depth <= 0 && ShowDepthOffsetArea)
		return;
	Depth = CL_MAX (0,Depth); 
	if (!DepthPoint && FilterNearbyText (Depth,WorldX,WorldY))
		return;
	sprintf (txt,"%i",Depth);
	ntxt = strlen (txt);
	if (ShowHazardAreas && Depth <= HazardDepth)
		c = HazardDepthColor;
	else
		c = DepthColor (Depth,Depth,HighlightDepth,HighlightDepthRange,ShowDepthColors);
	ShadowColor = RGB(c.rgbRed,c.rgbGreen,c.rgbBlue);
	if (!FlipText || (Rotation >= 0 && Rotation <= 900 || Rotation >= 2700))
		HBDisplayTextChar (HBImageHandle,txt,ntxt,WorldX,WorldY,Rotation,Size,0,ShadowColor);
	else
		HBDisplayTextChar (HBImageHandle,txt,ntxt,WorldX,WorldY,1800+Rotation,Size,0,ShadowColor);
	return;
}

CL_BOOL ImageBoundsInFileBounds (int Level,char *SubFileID,LPMNMXCORL ImageBounds)
{
	CL_BOOL In=CL_TRUE;
	int	nread;
	int	i;
	int	iSubFileID = *SubFileID - 'A';

	if (Level != CurSubFileLev)
	{
		CurSubFileLev = Level;
		nSubFiles = 0;
		strcpy (ImageLibraryIndex,LKMPath);
		strcat (ImageLibraryIndex,MapPrefix);
		if (Level == -2)
			strcat (ImageLibraryIndex,CTEXTEXTENTSBIN);
		else
			sprintf (strchr(ImageLibraryIndex,0),"%iX.bin",GridDefs[Level].GridID);
		FidIndex = db_fopen2 (ImageLibraryIndex,"rb");
		if (FidIndex != 0)
		{
			nread = db_fread (SubFileBounds,33,MAX_SUBFILE*sizeof(MNMXCORL),FidIndex);
			cl_fclose (FidIndex);
			nSubFiles = CL_MAX (0,nread / sizeof(MNMXCORL));
			for (i=0;i<nSubFiles;i++)
			{
				  swap32 (&SubFileBounds[i].xmn);
				  swap32 (&SubFileBounds[i].xmx);
				  swap32 (&SubFileBounds[i].ymn);
				  swap32 (&SubFileBounds[i].ymx);
			}		
		}
	}
	if (nSubFiles)
	{
		if (iSubFileID >= nSubFiles)
		{
			*SubFileID = MaxSubFileID;
			In = CL_FALSE;
		}
		else if (!BoundsInBound (&HBImageBounds,&SubFileBounds[iSubFileID],1))
			In = CL_FALSE;
	}

	return In;
}

CL_BOOL LakeMasterToHBirdImage2 (HANDLE HBImageHandle,int HBImageWidth,int HBImageHeight,int CenterX,int CenterY,double *pScale,int ForceLevel,int Rotation,
							int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange,BOOL AdjustScale,int *NumScreenPixelsCovered,
							LPIPILOTSTRUCT piPilot,
              ClChartRenderCb pfCallBack, void *pCallBackData,LPINT pRc )
{
	PLKMTILE	pLKMTile; 
	char	SubFileID='A';
	CL_BOOL bIsAbort = CL_FALSE;
	int	nTiles=0, RowsOnScreen, ColsOnScreen;
	int	StartStretchTime;
	int	StartImageCalls, StartNumReads;
	int	inc;
	
	inc = (int) (*pScale * HBImageWidth / 2);
	HBImageBounds.xmn = CenterX - inc;
	HBImageBounds.xmx = CenterX + inc;
	inc = (int) (*pScale * HBImageHeight / 2);
	HBImageBounds.ymn = CenterY - inc;
	HBImageBounds.ymx = CenterY + inc;
	if (piPilot != NULL)
	{
		GetLKMImageInit (HBImageWidth,HBImageHeight,CenterX, CenterY, *pScale,ForceLevel,SubFileID, Rotation,
						 WantDepthColors, WantContourLines, HighlightDepth, HighlightDepthRange,&TilePoint);
		if (ProcessiPilotCommand (piPilot,*pScale))
		{
			inc = (int) (*pScale * HBImageWidth / 2);
			HBImageBounds.xmn = CenterX - inc;
			HBImageBounds.xmx = CenterX + inc;
			inc = (int) (*pScale * HBImageHeight / 2);
			HBImageBounds.ymn = CenterY - inc;
			HBImageBounds.ymx = CenterY + inc;
			GetLKMImageInit (HBImageWidth,HBImageHeight,CenterX, CenterY, *pScale,ForceLevel,SubFileID, Rotation,
							 WantDepthColors, WantContourLines, HighlightDepth, HighlightDepthRange,&TilePoint);
		}
	}
Top:
	if (!ImageBoundsInFileBounds (ForceLevel,&SubFileID,&HBImageBounds))
		goto NextFile;
	StartImageCalls = NumStretchCalls;
	StartNumReads = numreads;
    GetLKMImageInit (HBImageWidth,HBImageHeight,CenterX, CenterY, *pScale,ForceLevel,SubFileID, Rotation,
					 WantDepthColors, WantContourLines, HighlightDepth, HighlightDepthRange,&TilePoint);

	while ( !bIsAbort && (pLKMTile = GetLKMImageTile (&TilePoint,piPilot,pRc)))
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
		currentCellID = pLKMTile->CellID;
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
	if (StartImageCalls == NumStretchCalls)
		NotNeededReads += (numreads - StartNumReads);
NextFile:
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
	int		NumCalls, StartNumReads;

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
	WantExtendedText = CL_FALSE;
	//Process contour text
	if ( !bIsAbort && WantContourLines && DisplayContourText && Scale < MAX_CONTOURTEXT_SCALE)
	{
NextCTextFile:
		if (!ImageBoundsInFileBounds (-2,&SubFileID,&HBImageBounds))
			goto SkipCTextFile;
		StartNumReads = numreads;
		NumCalls = 0;
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
						NumCalls++;
						nchr = 0;
					}
					rot = 0;
					x = 0;
					y = 0;
				}
				chr = TextOutRec.chr;
				if (chr == '!')
					DepthPoint = TRUE;
				else if (nchr < 4)
				{
					x += TextOutRec.x;
					y += TextOutRec.y;
					rot += TextOutRec.rot;
					txt[nchr++] = chr;
					DepthPoint = FALSE;
				}
				else
					nchr = 0;
			}
			if (nchr)
			{
				x /= nchr;
				y /= nchr;
				rot /= nchr;
				pt = TilePointToWorldPoint (x,y,&pLKMCTextTile->CellBounds);
				DisplayTextChar (HBImageHandle,txt,nchr,pt.x,pt.y,rot,TextOutRec.size,DepthPoint);
				NumCalls++;
				nchr = 0;
			}
			cl_free (pLKMCTextTile);

    	if (pfCallBack)
      		bIsAbort = pfCallBack( HBImageHandle , pCallBackData );
		else
			bIsAbort = FALSE;

		}
		GetLKMCTextTerminate ();
		if (!NumCalls)
			NotNeededReads += (numreads - StartNumReads);
SkipCTextFile:
		if (!bIsAbort && SubFileID < MaxSubFileID)
		{
			SubFileID++;
			goto NextCTextFile;
		}
	}
	 //process chart objects
	if (!bIsAbort && Scale < MAX_CHARTOBJECT_SCALE)
	{

		if (LKMBeginEnumObjects (CenterX,CenterY,RangeInMeters,Scale, -3,&enumHandle ))
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

			MaxNamesPerTile = 1 + (int)(2048 * (((double)GridWidth * (double)GridHeight)/((double)(HBImageWidth * Scale)*(double)(HBImageHeight * Scale))));
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

			MaxNamesPerTile = 1 + (int)(2048 * (((double)GridWidth * (double)GridHeight)/((double)(HBImageWidth * Scale)*(double)(HBImageHeight * Scale))));
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
		HBDisplayTextChar (HBImageHandle,dbmess,strlen(dbmess),HBImageCenterWorld.x,(int)(HBImageCenterWorld.y+30*MetersPerPixel),0,16,RGB(164,0,0),RGB(255,255,255));
		sprintf (dbmess,"DBTIMES  NC-%i  TI-%i  NS-%i  TS-%i  PR-%i RD-%i EX-%i",NumImageCalls, TotImageTime, NumStretchCalls,TotStretchTime, TotPRTime, totreadtime,totdecomptime);
		HBDisplayTextChar (HBImageHandle,dbmess,strlen(dbmess),HBImageCenterWorld.x,(int)(HBImageCenterWorld.y-0*MetersPerPixel),0,16,RGB(164,0,0),RGB(255,255,255));
		sprintf (dbmess,"IO  opens %i maxopentime %i seeks %i  maxtime %i  reads %i  totread %i  nnread %i no %i",numopen,maxopentime,numseek,maxseektime,numreads,totread,NotNeededReads,NumOpenedFiles);
		HBDisplayTextChar (HBImageHandle,dbmess,strlen(dbmess),HBImageCenterWorld.x,(int)(HBImageCenterWorld.y-30*MetersPerPixel),0,16,RGB(164,0,0),RGB(255,255,255));
		sprintf (dbmess,"%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i ",
			readloc[1],readloc[2],readloc[3],readloc[4],readloc[5],readloc[6],readloc[7],readloc[8],readloc[9],readloc[10],
			readloc[11],readloc[12],readloc[13],readloc[14],readloc[15],readloc[16],readloc[17],readloc[18],readloc[19],readloc[20],
			readloc[21],readloc[22],readloc[23],readloc[24],readloc[25],readloc[26],readloc[27],readloc[28],readloc[29],readloc[30],
			readloc[31],readloc[32],readloc[33],readloc[34],readloc[35]);
		HBDisplayTextChar (HBImageHandle,dbmess,strlen(dbmess),HBImageCenterWorld.x,(int)(HBImageCenterWorld.y-60*MetersPerPixel),0,16,RGB(164,0,0),RGB(255,255,255));
		sprintf (dbmess,"%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i ",
			readtim[1],readtim[2],readtim[3],readtim[4],readtim[5],readtim[6],readtim[7],readtim[8],readtim[9],readtim[10],
			readtim[11],readtim[12],readtim[13],readtim[14],readtim[15],readtim[16],readtim[17],readtim[18],readtim[19],readtim[20],
			readtim[21],readtim[22],readtim[23],readtim[24],readtim[25],readtim[26],readtim[27],readtim[28],readtim[29],readtim[30],
			readtim[31],readtim[32],readtim[33],readtim[34],readtim[35]);
		HBDisplayTextChar (HBImageHandle,dbmess,strlen(dbmess),HBImageCenterWorld.x,(int)(HBImageCenterWorld.y-90*MetersPerPixel),0,16,RGB(164,0,0),RGB(255,255,255));
		SortFileTimes ();
		sprintf (dbmess,"%s-%i-%i   %s-%i-%i   %s-%i-%i   %s-%i-%i   %s-%i-%i",
			OpenedFiles[ord[0]],filetim[ord[0]],filecnt[ord[0]],
			OpenedFiles[ord[1]],filetim[ord[1]],filecnt[ord[1]],
			OpenedFiles[ord[2]],filetim[ord[2]],filecnt[ord[2]],
			OpenedFiles[ord[3]],filetim[ord[3]],filecnt[ord[3]],
			OpenedFiles[ord[4]],filetim[ord[4]],filecnt[ord[4]]);
		HBDisplayTextChar (HBImageHandle,dbmess,strlen(dbmess),HBImageCenterWorld.x,(int)(HBImageCenterWorld.y-120*MetersPerPixel),0,16,RGB(164,0,0),RGB(255,255,255));
		sprintf (dbmess,"IAborts: %i   PAborts: %i",nImageAborts,nPRTAborts);
		HBDisplayTextChar (HBImageHandle,dbmess,strlen(dbmess),HBImageCenterWorld.x,(int)(HBImageCenterWorld.y-150*MetersPerPixel),0,16,RGB(164,0,0),RGB(255,255,255));
	}
	InitCalled = 0;
	LKMTrace ("Exiting PostRotationProcessing");
	if (bIsAbort)
		nPRTAborts++;
	return !bIsAbort;
}

BOOL GetNextDepthPixel (LPPOINT pPoint,BYTE wantPixelValue,PLKMTILE pLKMTile,LPIPILOTSTRUCT piPilot,int iStep)
{
	BOOL	rtn = FALSE;
	POINT	nextPoint;
	int	i, iPixel;

	if (piPilot)
	{
		if (piPilot->iOpt == 3 && iStep < piPilot->nFirstSteps[piPilot->iSelect-1])
		{
			*pPoint = piPilot->first5Steps[piPilot->iSelect-1][iStep];
			return TRUE;
		}
	}
	for (i=0;i<8;i++)
	{
		nextPoint.x = pPoint->x + xoff1[i];
		nextPoint.y = pPoint->y + yoff1[i];
		if (nextPoint.x >= 0 && nextPoint.x < pLKMTile->width &&
			nextPoint.y >= 0 && nextPoint.y < pLKMTile->height)
		{
			iPixel = nextPoint.y * pLKMTile->width + nextPoint.x;
			if (pLKMTile->Image[iPixel] == wantPixelValue)
			{
				*pPoint = nextPoint;
				return TRUE;
			}
		}
	}
	for (i=0;i<16;i++)
	{
		nextPoint.x = pPoint->x + xoff2[i];
		nextPoint.y = pPoint->y + yoff2[i];
		if (nextPoint.x >= 0 && nextPoint.x < pLKMTile->width &&
			nextPoint.y >= 0 && nextPoint.y < pLKMTile->height)
		{
			iPixel = nextPoint.y * pLKMTile->width + nextPoint.x;
			if (pLKMTile->Image[iPixel] == wantPixelValue)
			{
				*pPoint = nextPoint;
				return TRUE;
			}
		}
	}
	for (i=0;i<24;i++)
	{
		nextPoint.x = pPoint->x + xoff3[i];
		nextPoint.y = pPoint->y + yoff3[i];
		if (nextPoint.x >= 0 && nextPoint.x < pLKMTile->width &&
			nextPoint.y >= 0 && nextPoint.y < pLKMTile->height)
		{
			iPixel = nextPoint.y * pLKMTile->width + nextPoint.x;
			if (pLKMTile->Image[iPixel] == wantPixelValue)
			{
				*pPoint = nextPoint;
				return TRUE;
			}
		}
	}
	return FALSE;
}

void showpoint (POINT nextPoint,COLORREF color,int saveImageWidth,int saveImageHeight,POINT saveHBImageCenter,POINT saveHBImageCenterWorld,POINT saveHBImageCenterWorld_100,PLKMTILE pLKMTile,LPIPILOTSTRUCT piPilot)
{
 int	resetImageWidth = ImageWidth, resetImageHeight = ImageHeight;
 POINT	resetHBImageCenter = HBImageCenter, resetHBImageCenterWorld = HBImageCenterWorld, resetHBImageCenterWorld_100 = HBImageCenterWorld_100;
 static int i=1;

 if (i)
	 return;
 ImageWidth = saveImageWidth;
 ImageHeight = saveImageHeight;
 HBImageCenter = saveHBImageCenter;
 HBImageCenterWorld = saveHBImageCenterWorld;
 HBImageCenterWorld_100 = saveHBImageCenterWorld_100;

 {
	 POINT screenpt = TilePointToImagePoint2 (nextPoint.x,nextPoint.y,pLKMTile);

 //POINT worldpt = TilePointToWorldPoint2 (nextPoint.x,nextPoint.y,pLKMTile);
 //POINT screenpt = WorldPointToScreenPoint (worldpt.x,worldpt.y);
 setscreenpixel (screenpt,color);
 }
 HBImageCenter = resetHBImageCenter;
 HBImageCenterWorld = resetHBImageCenterWorld;
 HBImageCenterWorld_100 = resetHBImageCenterWorld_100;
 ImageWidth = resetImageWidth;
 ImageHeight = resetImageHeight;
 return;
}

int GetPaletteIndexForColor (RGBQUAD Color,PLKMTILE pLKMTile)
{
	int i;
	
	for (i=0;i<pLKMTile->PaletteLen;i++)
		if (Color.rgbBlue == pLKMTile->Palette[i].rgbBlue &&
			Color.rgbGreen == pLKMTile->Palette[i].rgbGreen &&
			Color.rgbRed == pLKMTile->Palette[i].rgbRed)
			return i;

	return 0;
}

BOOL TilePointOnScreen (POINT TilePt,PLKMTILE pLKMTile)
{
	POINT pt = TilePointToWorldPoint2 (TilePt.x,TilePt.y,pLKMTile);

	if (pt.x < HBImageBounds.xmn || pt.x > HBImageBounds.xmx ||
		pt.y < HBImageBounds.ymn || pt.y > HBImageBounds.ymx)
		return FALSE;
	return TRUE;
}

BOOL GetAdjoiningTile (LPPOINT pTilePt,LPINT pcornerIndex,int iScale,PLKMTILE *pLKMTile,PLKMTILE pLKMTileStart,LPIPILOTSTRUCT piPilot)
{
	int	TileID = (*pLKMTile)->CellID;
	int iErr;

	if (pTilePt->x <= 0)
	{
		TileID -= 1;
		pTilePt->x = (*pLKMTile)->width - 1;
	}
	else if (pTilePt->x >= (*pLKMTile)->width - 1)
	{
		TileID++;
		pTilePt->x = 0;
	}
	if (pTilePt->y <= 0)
	{
		TileID -= nGridCol;
		pTilePt->y = (*pLKMTile)->height - 1;
	}
	else if (pTilePt->y >= (*pLKMTile)->height - 1)
	{
		TileID += nGridCol;
		pTilePt->y = 0;
	}

	if (TileID == (*pLKMTile)->CellID)
		return FALSE;

	if (*pLKMTile != pLKMTileStart)
		cl_free (*pLKMTile);
	
	*pLKMTile = GetLKMImageTileFromScaleAndID (iScale,TileID,piPilot,&iErr);
	if (!*pLKMTile)
		return FALSE;
	return TRUE;
}

int ProcessiPilotCommand (LPIPILOTSTRUCT piPilot,double Scale)
{
	int	rtn=0;
	POINT	saveHBImageCenter = HBImageCenter, saveHBImageCenterWorld = HBImageCenterWorld,saveHBImageCenterWorld_100 = HBImageCenterWorld_100;
	int		saveImageWidth = ImageWidth, saveImageHeight = ImageHeight;
	MNMXCORL saveHBImageBounds = HBImageBounds;
	HANDLE	enumHandle;
	double	RangeInMeters;
	int	iErr;
	int	minStepsBetweenTrackPoints;
	int stepsSinceLastTrackPoint=0;

	if (piPilot == NULL)
		return 0;
	switch (piPilot->iOpt)
	{
	case 1:
	{

		RangeInMeters = Scale * piPilot->pickAperature + 1;
		Scale = AdjustToClosestScale (0.5);
		piPilot->iError = 1;
		LKMBeginEnumObjectsText (piPilot->pickPoint.x,piPilot->pickPoint.y,RangeInMeters, Scale,&enumHandle );
		while(LKMEnumNextObjectText( enumHandle, piPilot->pickText , sizeof(piPilot->pickText)  ))
		{
			LPENUMSTRUCT	pEnum=(LPENUMSTRUCT)enumHandle;

			if (pEnum->pickedType == 1)
			{
				piPilot->pickedCellID = pEnum->pickedCellID;
				piPilot->pickedCellX = pEnum->pickedCellX;
				piPilot->pickedCellY = pEnum->pickedCellY;
				piPilot->pickColor = pEnum->pickedColor;
				piPilot->iScale = pEnum->iScale;
				rtn = 1;
			}
		}
		LKMEndEnumObjectsText( enumHandle );
		HBImageBounds = saveHBImageBounds;
	}
	goto StartTracking;

	case 3:
	SetGridVals (piPilot->iScale);
	Scale = GridDefs[piPilot->iScale].MetersPerPixel;
	RangeInMeters = 10000 * Scale;
	minStepsBetweenTrackPoints = (int)((piPilot->trackPointDist / Scale)/1.4);
	piPilot->lenTrackArray = 1;
	piPilot->pTrackArray[0] = piPilot->pickPoint;
StartTracking:
	LKMBeginEnumObjectsText (piPilot->pickPoint.x,piPilot->pickPoint.y,RangeInMeters, Scale,&enumHandle );
	LKMEnumNextObjectText( enumHandle, piPilot->pickText , 0);
	HBImageBounds = saveHBImageBounds;
	if (piPilot->iOpt == 3)
	{
		HBImageBounds.xmn -= 200000;
		HBImageBounds.xmx += 200000;
		HBImageBounds.ymn -= 200000;
		HBImageBounds.ymx += 200000;
	}
	{
		PLKMTILE pLKMTile = GetLKMImageTileFromScaleAndID (piPilot->iScale,piPilot->pickedCellID,piPilot,&iErr);
		PLKMTILE pLKMTileStart = pLKMTile;
		if (pLKMTile)
		{
			POINT	nextPoint;
			BYTE	wantPixelValue;
			int		iPixel;
			int		cornerIndex = 0;
			BOOL	foundPixel;
			int		nsteps;
			int		breakRing=-1;

SplitRing:
			nsteps = 0;
			nextPoint.x = piPilot->pickedCellX;
			nextPoint.y = piPilot->pickedCellY;
			iPixel = nextPoint.y * pLKMTile->width + nextPoint.x;
			do
			{
				if (piPilot->lenTileCrossingPointArray >= piPilot->maxTileCrossingPointArray)
					break;
				piPilot->pTileCrossingPointArray[piPilot->lenTileCrossingPointArray++] = TilePointToWorldPoint64 (nextPoint.x,nextPoint.y,pLKMTile);
				wantPixelValue = GetPaletteIndexForColor (piPilot->pickColor,pLKMTile);
				pLKMTile->Palette[pLKMTile->TrackColorPaletteIndex[0]] = piPilot->directionColors[0];
				pLKMTile->Palette[pLKMTile->TrackColorPaletteIndex[1]] = piPilot->directionColors[1];
				foundPixel = FALSE;
				do
				{
					iPixel = nextPoint.y * pLKMTile->width + nextPoint.x;
					if (pLKMTile->Image[iPixel] == wantPixelValue)
						foundPixel = TRUE;
					pLKMTile->Image[iPixel] = pLKMTile->TrackColorPaletteIndex[0];
					if (nsteps < 5 && piPilot->iOpt == 1)
					{
						piPilot->first5Steps[0][nsteps] = nextPoint;
						piPilot->first5CellID[0][nsteps] = pLKMTile->CellID;
						piPilot->nFirstSteps[0] = nsteps+1;
					}
					if (nsteps == breakRing)
						break;
					if (piPilot->iOpt == 3 && stepsSinceLastTrackPoint++ > minStepsBetweenTrackPoints)
					{
						POINT wpoint = TilePointToWorldPoint2 (nextPoint.x,nextPoint.y,pLKMTile);

						if (distp (wpoint,piPilot->pTrackArray[piPilot->lenTrackArray-1]) >= piPilot->trackPointDist)
						{
							piPilot->pTrackArray[piPilot->lenTrackArray++] = wpoint;
							stepsSinceLastTrackPoint = 0;
							if (piPilot->lenTrackArray >= piPilot->maxTrackPoints)
								break;
						}
					}

					showpoint (nextPoint,RGB(255,255,0),saveImageWidth,saveImageHeight,saveHBImageCenter,saveHBImageCenterWorld,saveHBImageCenterWorld_100,pLKMTile,piPilot);
				} while (GetNextDepthPixel (&nextPoint,wantPixelValue,pLKMTile,piPilot,nsteps++));
				if (piPilot->lenTileCrossingPointArray >= piPilot->maxTileCrossingPointArray)
					break;
				if (foundPixel)
					piPilot->pTileCrossingPointArray[piPilot->lenTileCrossingPointArray++] = TilePointToWorldPoint64 (nextPoint.x,nextPoint.y,pLKMTile);
			}
			while (foundPixel && TilePointOnScreen (nextPoint,pLKMTile) && GetAdjoiningTile (&nextPoint,&cornerIndex,piPilot->iScale,&pLKMTile,pLKMTileStart,piPilot));
			
			if (piPilot->iOpt == 3)
			{
				OffsetTrackPoints (piPilot);
				goto Exit;
			}
			if (pLKMTile && pLKMTile->CellID == piPilot->pickedCellID && (abs(nextPoint.x-piPilot->pickedCellX)+abs(nextPoint.y-piPilot->pickedCellY) < 7))
			{
				breakRing = nsteps/2;
				piPilot->lenTileCrossingPointArray = 0;
				cl_free (pLKMTile);
				pLKMTile = GetLKMImageTileFromScaleAndID (piPilot->iScale,piPilot->pickedCellID,piPilot,&iErr);
				goto SplitRing;
			}
			piPilot->switchPoint = piPilot->lenTileCrossingPointArray;
			nextPoint.x = piPilot->pickedCellX;
			nextPoint.y = piPilot->pickedCellY;
			if (pLKMTile && pLKMTile != pLKMTileStart)
				cl_free (pLKMTile);
			pLKMTile = pLKMTileStart;
			if (piPilot->lenTileCrossingPointArray < piPilot->maxTileCrossingPointArray)
				piPilot->pTileCrossingPointArray[piPilot->lenTileCrossingPointArray++] = TilePointToWorldPoint64 (nextPoint.x,nextPoint.y,pLKMTile);
			nsteps = 0;
			
			do
			{
				wantPixelValue = GetPaletteIndexForColor (piPilot->pickColor,pLKMTile);
				foundPixel = FALSE;
				do
				{
					iPixel = nextPoint.y * pLKMTile->width + nextPoint.x;
					if (pLKMTile->Image[iPixel] == wantPixelValue)
						foundPixel = TRUE;
					pLKMTile->Image[iPixel] = pLKMTile->TrackColorPaletteIndex[1];
					if (nsteps < 5 && piPilot->iOpt == 1)
					{
						piPilot->first5Steps[1][nsteps] = nextPoint;
						piPilot->first5CellID[1][nsteps] = pLKMTile->CellID;
						piPilot->nFirstSteps[1] = nsteps+1;
					}
					showpoint (nextPoint,RGB(255,0,0),saveImageWidth,saveImageHeight,saveHBImageCenter,saveHBImageCenterWorld,saveHBImageCenterWorld_100,pLKMTile,piPilot);
				} while (GetNextDepthPixel (&nextPoint,wantPixelValue,pLKMTile,piPilot,nsteps++));
				if (piPilot->lenTileCrossingPointArray >= piPilot->maxTileCrossingPointArray)
					break;
				if (foundPixel)
					piPilot->pTileCrossingPointArray[piPilot->lenTileCrossingPointArray++] = TilePointToWorldPoint64 (nextPoint.x,nextPoint.y,pLKMTile);
			}
			while (foundPixel && TilePointOnScreen (nextPoint,pLKMTile) && GetAdjoiningTile (&nextPoint,&cornerIndex,piPilot->iScale,&pLKMTile,0,piPilot));

Exit:
			cl_free (pLKMTile);
			piPilot->iError = 0;
			//sleep (2000);
		}
		piPilot->iOpt++;
		rtn = 1;
	}
	LKMEndEnumObjectsText( enumHandle );
	break;
	case 2:
		break;
	case 4:
		break;
	}
	return rtn;
}

CL_BOOL LakeMasterToHBirdImage (HANDLE HBImageHandle,int HBImageWidth,int HBImageHeight,int CenterX,int CenterY,double *pScale,
								int DepthOff, int HighlightLMLakes, int SeamLess,int Rotation,
								int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange,
								BOOL DisplayHazardAreas, int HazDepth,
								BOOL AdjustScale,
								LPIPILOTSTRUCT piPilot,
              ClChartRenderCb pfCallBack, void *pCallBackData ,LPINT pRc)
{
	int	i;
	CL_BOOL	Continue = CL_TRUE;
	int		StartImageTime = GetClockTicks ();
	int		NumScreenPixelsCovered;

	SeamLess = 0;
	DepthFactor = CL_MAX (1,WantDepthColors);
	*pRc = 0;
	TotStretchTime = 0;
	TotImageTime = 0;
	NumStretchCalls = 0;
	NumImageCalls++;
	numseek = maxseektime = numreads = NotNeededReads = totread = totreadtime = totdecomptime = maxopentime = numopen = 0;
	memset (readloc,0,sizeof(readloc));
	memset (readtim,0,sizeof(readtim));
	memset (filetim,0,sizeof(filetim));
	memset (filecnt,0,sizeof(filecnt));
	if (AllowDbug && DepthOff == -1)
	{
		Dbug = 1;
		DepthOff = 0;
	}

	LKMTrace ("Entering LakeMasterToHBirdImage");
	
	ShowHazardAreas = DisplayHazardAreas;
	HazardDepth = HazDepth;
	DepthOffset = DepthOff;
	if (WantContourLines && *pScale > MAX_CONTOURLINE_SCALE)
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
	do
	{
		NumScreenPixelsCovered = 0;
		Continue = LakeMasterToHBirdImage2 (HBImageHandle,HBImageWidth,HBImageHeight,CenterX,CenterY,pScale,i,Rotation,
											WantDepthColors,WantContourLines,HighlightDepth,HighlightDepthRange,FALSE,&NumScreenPixelsCovered,
											piPilot,
						pfCallBack, pCallBackData,pRc );
		i--;
		//NumScreenPixelsCovered = PixelsPerScreen;
	}
	while (Continue && i > LastFullCoverage && NumScreenPixelsCovered < PixelsPerScreen);
	
	TotImageTime = GetClockTicks () - StartImageTime;
	LKMTrace ("Exiting LakeMasterToHBirdImage");

	if (!Continue)
		nImageAborts++;
	return Continue;
}

void GetLKMImageInit (int HBImageWidth,int HBImageHeight,int CenterX, int CenterY, double Scale, int ForceLevel,char SubFileID,int Rotation,
					  int WantDepthColors,int WantContourLines,int HltDepth,int HltDepthRange,LPPOINT pTilePoint)
{
	int		i, ioff;
	static	int	lastLevel=-1;

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
	if (i != lastLevel)
	{
		ClearReadBuffer (FALSE,-1);
		lastLevel = i;
	}
	SetGridVals (i);

#if !defined(HUMMINBIRD)
//	DisplayCurrentLayer (GridDefs[i].GridID);
#endif

	pTilePoint->x = HBImageBounds.xmn - GridWidth;
	pTilePoint->y = HBImageBounds.ymn;
	strcpy (ImageLibraryIndex,LKMPath);
	strcat (ImageLibraryIndex,MapPrefix);
	sprintf (strchr(ImageLibraryIndex,0),"%i%c.bin",GridDefs[i].GridID,SubFileID);
	LevUsed = GridDefs[i].GridID;
	FidIndex = db_fopen2 (ImageLibraryIndex,"rb");

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

void SetWidePoint (LPBYTE image,POINT pt,BYTE color,int iWidth,PLKMTILE pLKMTile)
{
	int i, iPixel = pt.y * pLKMTile->width + pt.x;
	
	image[iPixel] = color;

	for (i=0;i<8;i++)
	{
		int	ix = pt.x + xoff1[i];
		int	iy = pt.y + yoff1[i];

		if (ix >= 0 && ix < pLKMTile->width &&
			iy >= 0 && iy < pLKMTile->height)
		{
			iPixel = iy * pLKMTile->width + ix;
			image[iPixel] = color;
		}
	}
	if (iWidth > 1)
	for (i=0;i<16;i++)
	{
		int	ix = pt.x + xoff2[i];
		int	iy = pt.y + yoff2[i];

		if (ix >= 0 && ix < pLKMTile->width &&
			iy >= 0 && iy < pLKMTile->height)
		{
			iPixel = iy * pLKMTile->width + ix;
			image[iPixel] = color;
		}
	}
	if (iWidth > 2)
	for (i=0;i<24;i++)
	{
		int	ix = pt.x + xoff3[i];
		int	iy = pt.y + yoff3[i];

		if (ix >= 0 && ix < pLKMTile->width &&
			iy >= 0 && iy < pLKMTile->height)
		{
			iPixel = iy * pLKMTile->width + ix;
			image[iPixel] = color;
		}
	}
	return;
}

BOOL IsPixelValue (int x,int y,BYTE wantPixelValue,PLKMTILE pLKMTile)
{
	int iPixel = y * pLKMTile->width + x;
	
	if (pLKMTile->Image[iPixel] == wantPixelValue)
		return TRUE;
	return FALSE;
}

void FixTopAndRightEdge (PLKMTILE pLKMTile,BYTE wantPixelValue,BYTE setPixelValue)
{
	int nextx=1, nexty=0, lowerx=0, lowery=-1, x, y, iPixel;
	BOOL lastWas, nextIs, thisIs=FALSE, lowerIs;

	y = pLKMTile->height-1;
	nextIs = IsPixelValue (0,y,wantPixelValue,pLKMTile);
	for (x=0;x<pLKMTile->width-1;x++)
	{
		lastWas = thisIs;
		thisIs = nextIs;
		nextIs = IsPixelValue (x+1,y,wantPixelValue,pLKMTile);
		lowerIs = IsPixelValue (x,y-1,wantPixelValue,pLKMTile);
		if (lastWas && thisIs && nextIs && !lowerIs)
		{
			iPixel = y * pLKMTile->width + x;
			pLKMTile->Image[iPixel] = setPixelValue;
		}
	}
	x = pLKMTile->width-1;
	nextIs = IsPixelValue (x,0,wantPixelValue,pLKMTile);
	thisIs = FALSE;
	for (y=0;y<pLKMTile->height-1;y++)
	{
		lastWas = thisIs;
		thisIs = nextIs;
		nextIs = IsPixelValue (x,y+1,wantPixelValue,pLKMTile);
		lowerIs = IsPixelValue (x-1,y,wantPixelValue,pLKMTile);
		if (lastWas && thisIs && nextIs && !lowerIs)
		{
			iPixel = y * pLKMTile->width + x;
			pLKMTile->Image[iPixel] = setPixelValue;
		}
	}
	return;
}

void ProcessiPilotCrossingPoints (LPIPILOTSTRUCT piPilot,PLKMTILE pLKMTile)
{
	if (piPilot != NULL)
	{
		int		iCP;
		POINT	nextPoint;
		int		wantPixelValue;
		int		iColor = 0;
		int		iPixel;
		POINT	switchPoint={-1,-1};
		int		lenImage = pLKMTile->width * pLKMTile->height;
		LPBYTE	displayImage = pLKMTile->Image;
		int		ndash=0;
		int		saveLineWidth = piPilot->lineWidth;

		pLKMTile->Palette[pLKMTile->TrackColorPaletteIndex[0]] = piPilot->directionColors[0];
		pLKMTile->Palette[pLKMTile->TrackColorPaletteIndex[1]] = piPilot->directionColors[1];
		wantPixelValue = GetPaletteIndexForColor (piPilot->pickColor,pLKMTile);
		//FixTopAndRightEdge (pLKMTile,wantPixelValue,pLKMTile->TrackColorPaletteIndex[0]);
		if (piPilot->iOpt != 3 && piPilot->lineWidth)
		{
			displayImage = (LPBYTE)cl_malloc (lenImage);
  ClAssert( NULL != pLKMTile );
			memmove (displayImage,pLKMTile->Image,lenImage);
		}
		if (piPilot->iOpt == 3)
			piPilot->lineWidth = 0;
/*		if (piPilot->iOpt == 3 && piPilot->iSelect)
		{
			if (PointInBound (piPilot->blockingPoints[piPilot->iSelect-1][0],&pLKMTile->CellBounds64))
			{
				nextPoint = WorldPoint64ToTilePoint (piPilot->blockingPoints[piPilot->iSelect-1][0],pLKMTile);
				SetWidePoint (pLKMTile->Image,nextPoint,pLKMTile->TrackColorPaletteIndex[0],2,pLKMTile);
			}
			if (PointInBound (piPilot->blockingPoints[piPilot->iSelect-1][1],&pLKMTile->CellBounds64))
			{
				nextPoint = WorldPoint64ToTilePoint (piPilot->blockingPoints[piPilot->iSelect-1][1],pLKMTile);
				SetWidePoint (pLKMTile->Image,nextPoint,pLKMTile->TrackColorPaletteIndex[0],2,pLKMTile);
			}
		}*/
		if (piPilot->lenTileCrossingPointArray > 0)
		{
			//first CP always pick point, put a block here
			if (PointInBound (piPilot->pTileCrossingPointArray[0],&pLKMTile->CellBounds64))
			{
				nextPoint = WorldPoint64ToTilePoint (piPilot->pTileCrossingPointArray[0],pLKMTile);
				SetWidePoint (pLKMTile->Image,nextPoint,pLKMTile->TrackColorPaletteIndex[0],3,pLKMTile);
			}
		}
		if (piPilot->iOpt == 2 && piPilot->switchPoint)
			switchPoint = WorldPoint64ToTilePoint (piPilot->pTileCrossingPointArray[piPilot->switchPoint],pLKMTile);
		for (iCP=0;iCP < piPilot->lenTileCrossingPointArray;iCP++)
		{
			if (piPilot->iOpt == 2 && iCP >= piPilot->switchPoint)
				iColor = 1;
			if (PointInBound (piPilot->pTileCrossingPointArray[iCP],&pLKMTile->CellBounds64))
			{
				nextPoint = WorldPoint64ToTilePoint (piPilot->pTileCrossingPointArray[iCP],pLKMTile);
				if (nextPoint.x >= 0 && nextPoint.x < pLKMTile->width &&
					nextPoint.y >= 0 && nextPoint.y < pLKMTile->height)
				{
					do
					{
						iPixel = nextPoint.y * pLKMTile->width + nextPoint.x;
						pLKMTile->Image[iPixel] = pLKMTile->TrackColorPaletteIndex[iColor];
						if (piPilot->lineWidth)
						{
							displayImage[iPixel] = pLKMTile->TrackColorPaletteIndex[iColor];
							if (!iColor || !piPilot->doDash)
								SetWidePoint (displayImage,nextPoint,pLKMTile->TrackColorPaletteIndex[iColor],piPilot->lineWidth,pLKMTile);
							else if (ndash++/8 % 2)
								SetWidePoint (displayImage,nextPoint,pLKMTile->TrackColorPaletteIndex[iColor],piPilot->lineWidth,pLKMTile);
						}
					} while (!(nextPoint.x == switchPoint.x && nextPoint.y == switchPoint.y) && GetNextDepthPixel (&nextPoint,wantPixelValue,pLKMTile,0,0));
				}
			}
		}
		if (piPilot->iOpt != 3 && piPilot->lineWidth)
		{
			memmove (pLKMTile->Image,displayImage,lenImage);
			cl_free (displayImage);
		}
		piPilot->lineWidth = saveLineWidth;
	}
	return;
}
	
PLKMTILE GetLKMImageTile (LPPOINT pTilePoint,LPIPILOTSTRUCT piPilot,LPINT pRc)
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
		db_fread2 (&IndexRecordBlock,10,(FileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC),FidIndex);
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
		if (!db_fread2 (&ID_Length,11,8,FidIndex))
			return 0;
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
	if (!db_fread2 (pLKMTile->Image,12,storedlen,FidIndex))
	{
		cl_free (pLKMTile);
		return 0;
	}
	nRead += storedlen;
	compressedlen = storedlen;// - sizeof(LKMTILE) + 4;
	if (DecompressTile (pLKMTile->Image,compressedlen))
	{
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
		pLKMTile->CellBounds64.xmn = CellBounds.xmn * 64;
		pLKMTile->CellBounds64.xmx = CellBounds.xmx * 64;
		pLKMTile->CellBounds64.ymn = CellBounds.ymn * 64;
		pLKMTile->CellBounds64.ymx = CellBounds.ymx * 64;
		pLKMTile->PaletteLen = GetLeastUsedPaletteEntry (pLKMTile);
		ProcessiPilotCrossingPoints (piPilot,pLKMTile);
	}
	else
	{
		*pRc = 1;
		cl_free (pLKMTile);
		pLKMTile = 0;
	}

	return pLKMTile;
}

PLKMTILE GetLKMImageTileFromScaleAndID (int iScale,int CellID,LPIPILOTSTRUCT piPilot,LPINT pRc)
{
	PLKMTILE pLKMTile = NULL;
	char	SubFileID='A';
	int	len, storedlen, compressedlen;
	static	int	nRead=0;
	int	ID, NextID, LastID;
	int	loc=-1,pos,block,i,ioff;
	MNMXCORL	GridCellBounds;
	POINT	pt;
static	int	calls=0;

	SetGridVals (iScale);
	if (!nGridCol)
		return 0;
	GetGridBounds (CellID,&GridCellBounds);

Top:
	if (!ImageBoundsInFileBounds (iScale,&SubFileID,&GridCellBounds))
		goto NextFile;
	strcpy (ImageLibraryIndex,LKMPath);
	strcat (ImageLibraryIndex,MapPrefix);
	sprintf (strchr(ImageLibraryIndex,0),"%i%c.bin",GridDefs[iScale].GridID,SubFileID);
	LevUsed = GridDefs[iScale].GridID;
	FidIndex = db_fopen2 (ImageLibraryIndex,"rb");
	if (!FidIndex)
		goto NextFile;

	if ((ioff = GetLakeOffset (FidIndex)) < 0)
	{
		nGridCol = 0;
		goto NextFile;
	}
	db_fseek (FidIndex,-(int)(ioff+sizeof(FileHeader)),SEEK_END);
	cl_freadGRIDFILEHEADER(&FileHeader,FidIndex);
	NextID = -1;
	db_fseek (FidIndex,FileHeader.FirstIndexLoc,SEEK_SET);
	for (i=0;i<FileHeader.NumIndexLevs;i++)
	{
		db_fread2 (&IndexRecordBlock,10,(FileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC),FidIndex);
calls++;
		for (block=0; block<FileHeader.NumPerIndexRec+1; block++) {
			swapGRIDINDEXREC(&(IndexRecordBlock[block]));
		}
		for (pos=0;pos<FileHeader.NumPerIndexRec;pos++)
		{
			if (CellID < IndexRecordBlock[pos].GridCellID)
			{
				if (!pos)
					goto NextFile;
				NextID = IndexRecordBlock[pos].GridCellID;
				break;
			}
			else if (IndexRecordBlock[pos].loc < 0)
				break;
			else
				loc = IndexRecordBlock[pos].loc;
		}
		if (loc < 0)
			goto NextFile;
		db_fseek (FidIndex,loc,SEEK_SET);
	}
	LastID = -1;
	do
	{
		db_fseek (FidIndex,loc,SEEK_SET);
		if (!db_fread2 (&ID_Length,11,8,FidIndex))
			return 0;
		swap32 (&ID_Length.ID);
		swap32 (&ID_Length.Length);
		ID = ID_Length.ID;
		storedlen = ID_Length.Length;
		if (storedlen < 0)
			goto NextFile;
		LastID = ID;
		loc += 8 + storedlen;
	}while (ID != CellID);
	
	len = sizeof (LKMTILE)+FileHeader.TileWidth*FileHeader.TileHeight + 1024 + 54;

	pLKMTile = (PLKMTILE)cl_malloc (len);
  ClAssert( NULL != pLKMTile );
	if (!db_fread2 (pLKMTile->Image,12,storedlen,FidIndex))
	{
		cl_free (pLKMTile);
		return 0;
	}
	nRead += storedlen;
	compressedlen = storedlen;// - sizeof(LKMTILE) + 4;
	if (DecompressTile (pLKMTile->Image,compressedlen))
	{
		pLKMTile->width = FileHeader.TileWidth;
		pLKMTile->height = FileHeader.TileHeight;
		memmove (pLKMTile->Palette,&pLKMTile->Image[54],1024);
		memmove (pLKMTile->Image,&pLKMTile->Image[54+1024],pLKMTile->width*pLKMTile->height);

		pt = WorldPointToImagePoint (GridCellBounds.xmn,GridCellBounds.ymx);
		pLKMTile->destx = pt.x;
		pLKMTile->desty = pt.y;
		pt = WorldPointToImagePoint (GridCellBounds.xmx,GridCellBounds.ymn);
		pLKMTile->destw = pt.x - pLKMTile->destx;
		pLKMTile->desth = pt.y - pLKMTile->desty;
		pLKMTile->xoff = 0;
		pLKMTile->yoff = 0;
		pLKMTile->rectw = FileHeader.TileHeight;
		pLKMTile->recth = FileHeader.TileHeight;
		pLKMTile->CellID = CellID;
		pLKMTile->CellBounds = GridCellBounds;
		pLKMTile->CellBounds64.xmn = GridCellBounds.xmn * 64;
		pLKMTile->CellBounds64.xmx = GridCellBounds.xmx * 64;
		pLKMTile->CellBounds64.ymn = GridCellBounds.ymn * 64;
		pLKMTile->CellBounds64.ymx = GridCellBounds.ymx * 64;
		pLKMTile->PaletteLen = GetLeastUsedPaletteEntry (pLKMTile);
		ProcessiPilotCrossingPoints (piPilot,pLKMTile);
	}
	else
	{
		*pRc = 1;
		cl_free (pLKMTile);
		pLKMTile = 0;
	}
NextFile:
	GetLKMImageTerminate ();
	if (pLKMTile == NULL && SubFileID < MaxSubFileID)
	{
		SubFileID++;
		goto Top;
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
	if (DepthFactor < 4)
		Depth *= (5-DepthFactor);
	else if (DepthFactor > 4)
		Depth /= (DepthFactor-3);
	Depth /= COLORREDUCER;
	Depth *= COLORREDUCER;
	Depth = CL_MIN (127,Depth);
	c.rgbGreen = c.rgbRed = CL_MAX (0,255 - Depth*2);
	return c;
}

RGBQUAD DepthColor (int Depth1,int Depth2,int HighlightDepth,int HighlightDepthRange,BOOL ShowDepthColors)
{
	RGBQUAD c;
	RGBQUAD	ClassicDepthColor = {255,180,180,0};
	RGBQUAD	ClassicHighlightDepthColor = {255,128,128,0};

	c.rgbReserved = 0;
	if (!ShowDepthColors && (!HighlightDepth || !HighlightDepthRange))
		c = ClassicDepthColor;
	else if (!HighlightDepth)
		c = ColorOfDepth ((Depth1+Depth2)/2);
	else if (HighlightDepth && !HighlightDepthRange && Depth1 == HighlightDepth && Depth2 == HighlightDepth)
		c = HighlightDepthColor;
	else if (!(Depth1 >= HighlightDepth + HighlightDepthRange || Depth2 <= HighlightDepth - HighlightDepthRange))
		c = HighlightDepthColor;
	else if (!ShowDepthColors)
		c = ClassicDepthColor;
	else
		c = ColorOfDepth ((Depth1+Depth2)/2);
	return c;
}

int GetTileElevAtPoint (int TilePtX,int TilePtY,PLKMTILE pLKMTile,RGBQUAD *pcolor,LPSTR celev)
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
		*pcolor = color;
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
				if (!SymDep[SymNum][1])
				{
					Depth = CL_MAX (0,SymDep[SymNum][0] + DepthOffset);
					sprintf (celev,"%i feet %s",Depth,Resolution);
					rtn = 2;
				}
				else
				{
					Depth1 = CL_MAX (0,SymDep[SymNum][0] + DepthOffset);
					Depth2 = CL_MAX (0,SymDep[SymNum][1] + DepthOffset);
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
				if (!SymDep[SymNum][1])	//contour line symbol
				{
					Depth = SymDep[SymNum][0] + DepthOffset;
					if (ShowContourLines)
					{
						if (Depth == HighlightDepth && !HighlightDepthRange)
							*pal = HighlightDepthColor;
						else if (Depth < 0 && ShowDepthOffsetArea)
							*pal = DepthOffsetColor;
						else if (ShowContourLines > 1 && Depth % (ShowContourLines-1))
							*pal = DepthColor (Depth,Depth,HighlightDepth,HighlightDepthRange,ShowDepthColors);
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
							*pal = DepthColor (Depth,Depth,HighlightDepth,HighlightDepthRange,ShowDepthColors);
						}
					}
				}
				else					//contour area symbol	
				{
					Depth1 = SymDep[SymNum][0] + DepthOffset;
					Depth2 = SymDep[SymNum][1] + DepthOffset;
					if (ShowDepthOffsetArea && (Depth1 < 0 || Depth2 < 0))
						*pal = DepthOffsetColor;
					else if (ShowHazardAreas && (Depth1 < HazardDepth || Depth2 < HazardDepth))
						*pal = HazardDepthColor;
					else
						*pal = DepthColor (Depth1,Depth2,HighlightDepth,HighlightDepthRange,ShowDepthColors);
				}
			}
		}

	}
	return TRUE;
}

BOOL LoadHBSymList (LPSTR Path)
{
	char	SymListFile[CL_MAX_PATH];
	int		type=1, nsym, i;
	CLFILE	*Fid;

	sprintf (SymListFile,"%s%s%s",Path,MapPrefix,SYMLIST2BIN);
	Fid = db_fopen2 (SymListFile,"rb");
	if (!Fid)
	{
		sprintf (SymListFile,"%s%s%s",Path,MapPrefix,SYMLISTBIN);
		Fid = db_fopen2 (SymListFile,"rb");

		if (!Fid)
			return FALSE;
	}
	else
		type = 2;
	minsym = cl_freadS16(Fid);
	maxsym = cl_freadS16(Fid);
	nsym = (maxsym-minsym+1);
	if (type == 1)
	{
		db_fread (SymDepth,13,nsym*2,Fid);
		for (i=0;i<nsym;i++)
		{
			SymDep[i][0] = SymDepth[i][0];
			SymDep[i][1] = SymDepth[i][1];
		}
	}
	else
   {
		db_fread (SymDep,13,nsym*4,Fid);
		for (i=0;i<nsym;i++)
		{
			swap16(&(SymDep[i][0]));
			swap16(&(SymDep[i][1]));
		}
   }

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
	Fid = db_fopen2 (ParmFile,"rb");

	if (!Fid)
		return FALSE;
	db_fseek (Fid,0,SEEK_END);
	len = cl_ftell (Fid)+1;
	db_fseek (Fid,0,SEEK_SET);
	pCmd = cl_malloc (len);
  ClAssert( NULL != pCmd );
	db_fread (pCmd,14,len,Fid);
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
	int	inc;
	
	inc = (int) (0.5 + RangeInMeters);
	HBImageBounds.xmn =  WorldPtX - inc;
	HBImageBounds.xmx =  WorldPtX + inc;
	HBImageBounds.ymn =  WorldPtY - inc;
	HBImageBounds.ymx =  WorldPtY + inc;

	pEnum = cl_malloc (sizeof(ENUMSTRUCT));
  ClAssert( NULL != pEnum );
	*enumHandle = pEnum;
	pEnum->First = TRUE;
	pEnum->Done = FALSE;
	pEnum->RangeInMeters = RangeInMeters;
	pEnum->Scale = Scale;
	pEnum->RangeInPixels = (int)(RangeInMeters/Scale);
	pEnum->CenterPoint.x = WorldPtX;
	pEnum->CenterPoint.y = WorldPtY;
	pEnum->PointEnumHandle = 0;
	WantExtendedText = CL_TRUE;

	return TRUE;
}

BOOL LKMEnumNextObjectText(HANDLE enumHandle, LPSTR text , int maxtextlen )
{
	LPENUMSTRUCT	pEnum=(LPENUMSTRUCT)enumHandle;
	LPENUMOBJECT	pEnumObject;
	HANDLE	ObjectHandle;
	char	SubFileID='A';
	int		iScale;
	int		NextType;
	int		Rc;
	BOOL	rtn=FALSE;
	char	depthAreaText[32]="";
	char	minDistText[32]="";
	int		minDist, dist, minCellID, minX, minY;
	RGBQUAD	color, minColor;

	MinPopulation = 0;
	if (pEnum->First)
	{
		PLKMTILE	pLKMTile;
		double Scale;

		pEnum->pickedType = 0;
		pEnum->First = FALSE;
		iScale = GetClosestScaleIndex (pEnum->Scale);
		Scale = GridDefs[iScale].MetersPerPixel;
		if (iScale >= MaxDepthLayer)
		{
			rtn = FALSE;
			minDist = 1000000000;
Top:
			if (!ImageBoundsInFileBounds (iScale,&SubFileID,&HBImageBounds))
				goto NextFile;
			GetLKMImageInit ((int)pEnum->RangeInMeters,(int)pEnum->RangeInMeters,
							 (int)pEnum->CenterPoint.x,(int)pEnum->CenterPoint.y,
							 Scale,iScale,SubFileID,0,-1,0,0,0,&pEnum->TilePoint);
			while (maxtextlen && minDist > 0 && (pLKMTile = GetLKMImageTile (&pEnum->TilePoint,0,&Rc)))
			{
				POINT	TilePt = WorldPointToTilePoint (pEnum->CenterPoint.x,pEnum->CenterPoint.y,pLKMTile);
				int		Offset=pEnum->RangeInPixels/2, incx, incy;

				if ((rtn = GetTileElevAtPoint (TilePt.x,TilePt.y,pLKMTile,&color,text)) == 2)
				{
					minDist = 0;
					minCellID = pLKMTile->CellID;
					minX = TilePt.x;
					minY = TilePt.y;
					minColor = color;
					strncpy (minDistText,text,32);
					goto HaveContourLine;
				}
				if (rtn == 1) //depth area
					strncpy (depthAreaText,text,32);
				for (incx=1;incx < Offset;incx++)
				{
					int	x = TilePt.x + incx;

					for (incy=1;incy < Offset;incy++)
					{
						int	y = TilePt.y + incy;

						if ((rtn = GetTileElevAtPoint (x,y,pLKMTile,&color,text) == 2))
						{
							dist = incx + incy;
							if (dist < minDist)
							{
								minDist = dist;
								minCellID = pLKMTile->CellID;
								minX = x;
								minY = y;
								minColor = color;
								strncpy (minDistText,text,32);
							}
						}
						y = TilePt.y - incy;
						if ((rtn = GetTileElevAtPoint (x,y,pLKMTile,&color,text) == 2))
						{
							dist = incx + incy;
							if (dist < minDist)
							{
								minDist = dist;
								minCellID = pLKMTile->CellID;
								minX = x;
								minY = y;
								minColor = color;
								strncpy (minDistText,text,32);
							}
						}
					}
					x = TilePt.x - incx;
					for (incy=1;incy < Offset;incy++)
					{
						int	y = TilePt.y + incy;

						if ((rtn = GetTileElevAtPoint (x,y,pLKMTile,&color,text) == 2))
						{
							dist = incx + incy;
							if (dist < minDist)
							{
								minDist = dist;
								minCellID = pLKMTile->CellID;
								minX = x;
								minY = y;
								minColor = color;
								strncpy (minDistText,text,32);
							}
						}
						y = TilePt.y - incy;
						if ((rtn = GetTileElevAtPoint (x,y,pLKMTile,&color,text) == 2))
						{
							dist = incx + incy;
							if (dist < minDist)
							{
								minDist = dist;
								minCellID = pLKMTile->CellID;
								minX = x;
								minY = y;
								minColor = color;
								strncpy (minDistText,text,32);
							}
						}
					}
				}
	HaveContourLine:
				cl_free (pLKMTile);
			}
			GetLKMImageTerminate ();
NextFile:
			if (minDist && SubFileID < MaxSubFileID)
			{
				SubFileID++;
				goto Top;
			}
			if (*minDistText)
			{
				MNMXCORL GridCellBounds;

				strcpy (text,minDistText);
				GetGridBounds (minCellID,&GridCellBounds);
				pEnum->pickedPoint = TilePointToWorldPoint (minX,minY,&GridCellBounds);
				pEnum->pickedCellID = minCellID;
				pEnum->pickedCellX = minX;
				pEnum->pickedCellY = minY;
				pEnum->pickedColor = minColor;
				pEnum->iScale = iScale;
				pEnum->pickedType = 1;
				rtn = TRUE;
			}
			else if (*depthAreaText)
			{
				strcpy (text,depthAreaText);
				pEnum->pickedCellID = minCellID;
				pEnum->pickedCellX = minX;
				pEnum->pickedCellY = minY;
				pEnum->pickedColor = minColor;
				pEnum->pickedType = 2;
				rtn = TRUE;
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
	if (pEnumObject)
	{
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

	FidIndex = db_fopen2 (ImageLibraryIndex,"rb");

	if (!FidIndex)
		return;
	if ((ioff = GetLakeOffset (FidIndex)) < 0)
		return;
	db_fseek (FidIndex,-(int)(ioff+sizeof(CTextFileHeader)),SEEK_END);
	if (cl_freadTEXTFILEHEADER( &CTextFileHeader, FidIndex ))
	{
		for (i=0;i<NumGrids;i++)
			if (CTextFileHeader.GridID == GridDefs[i].GridID)
				goto HaveGrid;
	}
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
		if (!db_fread2 (&IndexRecordBlock,15,(CTextFileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC),FidIndex))
			return 0;
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
		if (!db_fread2 (&ID_Length,16,8,FidIndex))
			return 0;
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

	if (!db_fread2 (pLKMTile->TextDef,17,pLKMTile->Numrecs*sizeof(CTEXTREC),FidIndex))
	{
		cl_free (pLKMTile);
		return 0;
	}
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

	switch (abs (WantType))
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

	pEnumObject->Fid = db_fopen2 (ObjectLibrary,"rb");

	if (!pEnumObject->Fid)
	{
		cl_free (pEnumObject);
		return FALSE;
	}
	db_fseek (pEnumObject->Fid,-(int)sizeof(OBJECTFILEHEADER),SEEK_END);
	if (cl_freadOBJECTFILEHEADER( &pEnumObject->ObjectFileHeader, pEnumObject->Fid ))
	{
		for (i=0;i<NumGrids;i++)
			if (pEnumObject->ObjectFileHeader.GridID == GridDefs[i].GridID)
				goto HaveGrid;
	}
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
	POINT		WorldPoint;
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
			WorldPoint = TilePointToWorldPoint (pPointOutRec->x,10000-pPointOutRec->y,&GridCellBounds);
			if (!PointInBound (WorldPoint,&pEnumObject->Bounds))
				continue;
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
			pObject->WorldPoint = WorldPoint;
			if (pObject->Type == abs(pEnumObject->WantType))
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
		if (!db_fread2 (&IndexRecordBlock,18,(pEnumObject->ObjectFileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC),pEnumObject->Fid))
			return 0;
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
		if (!db_fread2 (&ID_Length,19,8,pEnumObject->Fid))
			return FALSE;
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
	if (!db_fread2 (pLKMTile->PointDef,20,pLKMTile->Numrecs*sizeof(MAPPOINTREC),pEnumObject->Fid))
	{
		cl_free (pEnumObject->RecordHandle);
		pEnumObject->RecordHandle = 0;
		return FALSE;
	}

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
//		if (!stricmp ("SANBURN",pObject->Text))
//			ii=1;
			pObject->WorldPoint = TilePointToWorldPoint (pPointOutRec->x,10000-pPointOutRec->y,&GridCellBounds);
			Pop = pPointOutRec->PopulationOrAcres;
			if (pObject->Type == abs(pEnumObject->WantType) && Pop >= MinPopulation && PointInBound (pObject->WorldPoint,&pEnumObject->Bounds))
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
		if (!db_fread2 (&IndexRecordBlock,21,(pEnumObject->ObjectFileHeader.NumPerIndexRec+1)*sizeof(GRIDINDEXREC),pEnumObject->Fid))
			return FALSE;
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
		if (!db_fread2 (&ID_Length,22,8,pEnumObject->Fid))
			return FALSE;
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
	if (!db_fread2 (pLKMTile->PointDef,23,pLKMTile->Numrecs*sizeof(CITYLAKENAMEREC),pEnumObject->Fid))
	{
		cl_free (pEnumObject->RecordHandle);
		pEnumObject->RecordHandle = 0;
		return FALSE;
	}

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

	Fid = db_fopen2 (ObjectList,"rb");

	if (!Fid)
		return FALSE;
	db_fread (list,24, sizeof(list), Fid);
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
   db_fread (grid,25,36,pFid);
   swap32 (&grid->GridID);
   swap32 (&grid->GridMinX);
   swap32 (&grid->GridMinY);
   swap32 (&grid->GridMaxX);
   swap32 (&grid->GridMaxY);
   swap32 (&grid->GridWidth);
   swap32 (&grid->GridHeight);
   swap32 (&grid->nGridCol);
   swap32 (&grid->nGridRow);
   db_fread (&grid->MetersPerPixel,26,8,pFid);
   swap64 (&grid->MetersPerPixel);

   return;
}

static void cl_freadGRIDFILEHEADER(GRIDFILEHEADER *header,CLFILE *pFid)
{
   db_fread (header,27,sizeof(GRIDFILEHEADER),pFid);
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

static BOOL cl_freadTEXTFILEHEADER( CTEXTFILEHEADER *header, CLFILE *pFid )
{  
  if (!db_fread2 (header,28,sizeof(CTEXTFILEHEADER),pFid))
	  return FALSE;
  swap32 (&header->GridID);
  swap32 (&header->NumPerIndexRec);
  swap32 (&header->NumIndexLevs);
  swap32 (&header->FirstIndexLoc);
  swap32 (&header->NumRecs);
  return TRUE;
}

static BOOL cl_freadOBJECTFILEHEADER( OBJECTFILEHEADER *header, CLFILE *pFid )
{  
  if (!db_fread2 (header,29,sizeof(OBJECTFILEHEADER),pFid))
	  return FALSE;
  swap32 (&header->GridID);
  swap32 (&header->NumPerIndexRec);
  swap32 (&header->NumIndexLevs);
  swap32 (&header->FirstIndexLoc);
  swap32 (&header->NumRecs);
  return TRUE;
}

static void swapCTEXTREC( CTEXTREC *rec)
{
  swap32 (&rec->CharNo_Chr);
  swap32 (&rec->x_y);
  swap32 (&rec->size_rot);
  return;
}

/*static BOOL cl_freadCTEXTREC( CTEXTREC *rec , CLFILE *pFID )
{
  rec->CharNo_Chr = cl_freadS32(pFID);
  rec->x_y = cl_freadS32(pFID);
  rec->size_rot = cl_freadS32(pFID);
}*/

/*static void cl_freadMAPPOINTREC( MAPPOINTREC *rec , CLFILE *pFID )
{
  rec->ref = cl_freadS32(pFID);
  rec->symno_hasextendedtext = cl_freadS32(pFID);
  rec->x_y = cl_freadS32(pFID);
  db_fread ( rec->PointText, 30, sizeof( rec->PointText ), pFID );
}*/

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
