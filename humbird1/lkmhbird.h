#include "ChartLib.h"
#include "fs.h"
#include "types.h"   // NOT humminbird types header

#define	MAX_PATH	128
#define PY		  3.141592653589793e0
#define TWOPI     6.283185307179586e0
#define	HALFPI    1.570796326794896e0 
#define RADtoDEG 57.295779513082322
#define DEGtoRAD 0.017453292519943296
#define MFT  3.280833333333333e0
#define FTM 3.04800609601219e-1
#define SQMTOACRES	0.000247e0
#define SQFTTOACRES	0.000022956841138e0
#define SFTSM     9.2903411613274e-2
#define  SMSFT     1.076386736111111e1

#define IMAGE_MODE_NORTHUP	1
#define IMAGE_MODE_COURSEUP 2
#define IMAGE_MODE_MAXMEM 3

#define	TRUE		1
#define FALSE		0
#define MAX_RECS_IN_BLOCK	65
#define MAX_TEXT_IN_IMAGE	2048
#define MAX_SUBFILE	32

#define	MAXBUFFEREDTILES	5
// #define ENABLE_LKM_BUFFERING
#ifdef ENABLE_LKM_BUFFERING
//#define	MAXBUFFEREDIMAGETILES	42
//#define	CTEXTBUFFERSIZE	(1024 * 64)
#define	CTEXTBUFFERSIZE	(1024 * 256)  // size is increased from 64K to 256K
#else
#define  CTEXTBUFFERSIZE   (4)
#endif

#define IMAGETILESIZE	(sizeof (LKMTILE)+ 256*256 + 1024 + 54)   // 67746 bytes
#define IMAGETILESIZE_ALIGNED	(IMAGETILESIZE + IMAGETILESIZE%4)
//typedef void	*HANDLE;
//typedef char	*LPSTR;
typedef int   BOOL;
#ifndef linux
typedef unsigned long       DWORD;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
#endif
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
typedef struct {
        unsigned char    rgbBlue;
        unsigned char    rgbGreen;
        unsigned char    rgbRed;
        unsigned char    rgbReserved;
} CL_RGBQUAD;
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
typedef struct {
				int  x;
				int  y;
				} CL_POINT;
typedef CL_POINT	*LPCL_POINT;
typedef	unsigned long ULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
typedef int		HFILE;

typedef struct	{
				 int	Type;
				 int	BitmapID;
				 int	Size;
				 int	nPoints;
				 CL_POINT	WorldPoint;
				 CL_POINT	ImagePoint;
				 int	nChar;
				 char	Text[];	
				}LKMOBJECT;
typedef LKMOBJECT	*LPLKMOBJECT;

typedef	struct	{long GridID,GridCellID;} GRIDCELLDEF;
typedef struct	{long GridCellID, loc;} GRIDINDEXREC;
typedef struct	{long TileWidth,TileHeight,NumPerIndexRec,NumIndexLevs,FirstIndexLoc,NumRecs;} GRIDFILEHEADER;
typedef struct	{long GridID,NumPerIndexRec,NumIndexLevs,FirstIndexLoc,NumRecs;} CTEXTFILEHEADER;
typedef struct	{long GridID,NumPerIndexRec,NumIndexLevs,FirstIndexLoc,NumRecs;} OBJECTFILEHEADER;
typedef struct	{CL_POINT	TilePoint;
				 CL_POINT	CenterPoint;
				 double	RangeInMeters;
				 double Scale;
				 int	RangeInPixels;
				 int	iScale;
				 CL_POINT	pickedPoint;
				 int	pickedCellID;
				 int	pickedCellX;
				 int	pickedCellY;
				 int	pickedType;
				 CL_RGBQUAD	pickedColor;
				 CL_BOOL	First, Done;
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
typedef struct {int width, height;
				int	xoff,  yoff;
				int rectw, recth;
				int	destx, desty;
				int	destw, desth;
				int	CellID;
				int	iScale;
				char SubFileID;
				char filler[3];
				unsigned int	lastUse;
				MNMXCORL	CellBounds;
				MNMXCORL	CellBounds64;
				CL_POINT	tilePoint;
				int	PaletteLen;
				int	TrackColorPaletteIndex[2];
				CL_RGBQUAD	Palette[256];
				LPBYTE Image;
				BYTE winBitmap[4];
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
				 char	SubFileID;
				 char	filler[3];
				 int	Numrecs;
				 CL_POINT	tilePoint;
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
				CL_POINT	CenterWorld;
				CLFILE	*Fid;
				MNMXCORL	Bounds;
				CL_POINT	TilePoint;
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

typedef	struct {int ID, Length;}IDLength;


//typedef void	*HANDLE;
//typedef char	*LPSTR;
typedef struct
{
	// Caller supplied values
	int	iOpt;						// 1 for initialization
									// 2 to display route choices
									// 3 to create route track
									// 4 to highlight selected contour in current image
									// 5 to continue the route starting at the end of the current track
									// 6 to reverse the route starting at the end of the current track
	DPOINT		pickPointUser;		// world coordinate point selected by the user to start the track	
	int			pickAperature;		// width of rectangle (in screen pixels) in which pick will be done
	CL_RGBQUAD	directionColors[2];	// colors for the 2 highlighted directions
	int			lineWidth;			// width of highlighted contour line
	CL_BOOL		doDash;				// dash second choice line
	int			maxTileCrossingPointArray;// max length of the following array in number of points.
	LPCL_POINT	pTileCrossingPointArray;// points at which selected contour crosses tile boundaries
	int			iSelect;			// contour selected by user (1 or 2, 0 if user canceled selection.
	double		trackPointSpacing;	// distance in meters between track points
	double		offsetInMeters;		// Offset in meters of tracking points from selected contour.
									// Positive value for right side, negative for left side
	int			knotRemovalRange;	// indicates how many points forward or backward to look for knot intersections
	int			nSmoothPass;		// 0 for no smoothing of offset track line 1 or greater for smoothing
	CL_BOOL		showSelectedContour;// displays selected contour in first color if iOpt==4
	int			maxTrackPoints;		// max number of track points
	int			lenTrackArrayOut;	// length of the ouput track and depth arrays
	LPCL_POINT	pTrackArrayOut;		// iPilot tracking points
	LPINT		   pTrackArrayDepth;	// actual depth of each track point in feet
	// Returned values
   LPINT       pTrackArrayDepthOut;  // depth of each track point in feet * 10 with water depth offset adjustment
	int			iError;				// returned error status. 0 if no error, 1 if no contour within pick aperature
	char		pickText[32];		// returned if iOpt==1. Text description of contour picked
	CL_BOOL		trackIsContinuous;	// TRUE if track is a continuous loop
	int			azimuthFromStart[2];// Azimuth in degrees for the 2 direction options (0=East,90=North,180=West,270=South)
	int			deeperFlag[2];		// Flag indicating which side of each direction option is deapest (-1 for left, 1 for right)
	int			userPicked[2];		// Flag indicating which side of each direction the original user pick point is on (-1 for left, 1 for right)
	int			lenTileCrossingPointArray;// number of crossing points currently in the following array
	DPOINT		pickPointD;			// world coordinate point start point on the selected contour	
	DPOINT		endPoint[2];		// world coordinate point at which the tracking ended
	// the following are for internal use
	CL_RGBQUAD	pickColor;			// internal depth color value of selected contour
	DPOINT		directionPoint[2];
	LPDPOINT	pTrackArray;	
	LPDPOINT	pTrackArrayTemp;	
	int			lenTrackArray;		// number of track points created
	int			lenTrackArrayTemp;	
	int			iScale;						
	int			pickedCellID;
	int			pickedCellX;
	int			pickedCellY;
	CL_POINT	first8Steps[2][8];
	int			first8CellID[2][8];
	BYTE		first8PixelVals[2][8];
	int			nFirstSteps[2];
	int			lenSelectionTrack[2];
	LPDPOINT	pSelectionTrack[2];
	LPCL_POINT	pSelectionTrackOut[2];
	int			lenSelectedContour;
	LPDPOINT	pSelectedContour;
	LPCL_POINT	pSelectedContourOut;
	int			switchPoint;		// crossing point array index at which color switch occures
	CL_POINT	breakPoint;
} IPILOTSTRUCT;
typedef IPILOTSTRUCT *LPIPILOTSTRUCT;


CL_BOOL LKMGetCardName (char *PathToLKMData,char *CardName, int MaxLen );
int LKMCheckCardCID (char *PathToLKMData,char *CID);
int LKMToHBInit (char *PathToLKMData,char *CID,char *CardName, unsigned long nScreenSize);
void SetLKMImageMode (int mode);
void LKMToHBClose (void);
CL_BOOL LakeMasterToHBirdImage (HANDLE HBImageHandle,int HBImageWidth,int HBImageHeight,int CenterX,int CenterY,double *pScale,
								int DepthOff, int HighlightLMLakes, int SeamLess,int Rotation,
								int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange,
								CL_BOOL ShowHazardAreas, int HazDepth,
								CL_BOOL AdjustScale,
								ClChartRenderCb pfCallBack, void *pCallBackData,LPINT pRc );
CL_BOOL LKMBeginEnumObjectsText (int WorldPtX,int WorldPtY,double RangeInMeters,double Scale,HANDLE *enumHandle );
CL_BOOL LKMEnumNextObjectText(HANDLE enumHandle, char *text , int maxtextlen );
void LKMEndEnumObjectsText(HANDLE enumHandle );
CL_BOOL LKMBeginEnumObjects (int CenterX, int CenterY, double RangeInMeters,double Scale,int WantType,HANDLE *penumHandle);
CL_BOOL LKMEnumNextName (HANDLE enumHandle, HANDLE *ObjectHandle  );	
void LKMEndEnumObjects (HANDLE enumHandle );
CL_BOOL LKMEnumNextObject(HANDLE enumHandle, HANDLE *ObjectHandle  );	
CL_BOOL LKMGetNavaidData (HANDLE ObjectHandle,int *x,int *y,int *BitmapID);
int LKMGetChartText (HANDLE ObjectHandle,int WantType,int *x,int *y, char *text , int maxtextlen );
CL_BOOL LKMGetHighwayShieldData (HANDLE ObjectHandle,int *x,int *y,int *ShieldID, char *text); 
CL_BOOL LKMGetObjectNameFromID (int ID,char *Name);
void LKMSelectMap (char * MapID);
CL_BOOL LKMPostRotationProcessing(HANDLE HBImageHandle,int HBImageWidth,int HBImageHeight,int CenterX,int CenterY,double Scale,
								int DepthOff, int HighlightLMLakes,
								int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange,
								CL_BOOL DisplayHazardAreas, int HazDepth,
								ClChartRenderCb pfCallBack, void *pCallBackData );
void HBDisplayBitmap (HANDLE HBImageHandle,char * BitmapPathName,int PCTSize,int WorldX,int WorldY);
int GetLakeOffset (CLFILE *pFID,CL_BOOL useBufferedRead);
double AdjustToClosestScale (double Scale);

void OffsetTrackPoints (LPIPILOTSTRUCT piPilot);
double distp(CL_POINT Point1, CL_POINT Point2);
DPOINT CL_POINTtoDPOINT (CL_POINT point);
CL_POINT DPOINTtoCL_POINT (LPDPOINT dpt);
double distpd(LPDPOINT Point1, LPDPOINT Point2);
int ReSampleToOriginalSpacing (int nPnts,LPDPOINT *pPoints,int MaxPoints,double TrackPointSpacing);
int GetAZMInDegrees (LPDPOINT Point1,LPDPOINT Point2);
double GetDepthAtPoint (LPDPOINT Point, int nPickAperature);
double getazmd (LPDPOINT Point1,LPDPOINT Point2);
DPOINT dnewpt (LPDPOINT OldPoint, double AZM, double DIS);
double DeltaAZ (double *AZ1, double *AZ2);
void ConvertUTMtoLatLon (LPDPOINT pPoint,int iZone);
void ConvertLatLontoUTM (LPDPOINT pPoint,int iUTMZone, char *ActualZone);
long Convert_Geodetic_To_UTM (double Lat_Degrees,
                              double Long_Degrees,
                              int    Zone_Override,
                              long   *ActualZone,
                              char   *Hemisphere,
                              double *Easting,
                              double *Northing);

#include "FollowTheContour.h"