#include "gmlimits.h"
#include <stdlib.h>  
#include <time.h>    
#include <commdlg.h>
#include <ctype.h>      
#ifndef _WIN32
#define HUGE __huge  
#else
#define HUGE 
//#define FAR
#define LONG	long
#endif
#if WIN32
#define _fstrncpy strncpy
#define _fstrncmp strncmp
#define _fstrnicmp strnicmp
#define _fstrlen strlen
#define _fstrcmp strcmp
#define _fstrcpy strcpy
#define _fstrstr strstr
#define _fstrlwr strlwr
#define _fstrcat strcat
#define _fstrupr strupr
#define _fstrchr strchr
#define _fstrrchr strrchr
#define _fstricmp stricmp
#define _fmemmove memmove
#define _fmemcpy memcpy
#define _fstrset strset
#define _fmemicmp memicmp 
#define _fmemcmp memcmp
#define _fmemset memset
#endif 

#define WM_F1DOWN	    0x0500  
#define	INDCOD	31100 
#define	NULL_ELV	-100.0  
          

#define SQL_DATE				 9
#define SQL_TIME				10
#define SQL_TIMESTAMP			11
#define SQL_LONGVARCHAR 		(-1)
#define SQL_BINARY				(-2)
#define SQL_VARBINARY			(-3)
#define SQL_LONGVARBINARY		(-4)
#define SQL_BIGINT				(-5)
#define SQL_TINYINT 			(-6)
#define SQL_BIT 				(-7)
#define SQL_TYPE_DRIVER_START	(-80)       

#define	TYPE_AREA		0
#define TYPE_POLYLINE	1
#define	TYPE_CONTOUR	3  
#define TYPE_TEXT		4
#define TYPE_LINECURVE	101
#define TYPE_POINT		1

#define VPTYPE_FORMAT	0
#define VPTYPE_PLAN		1
#define VPTYPE_PROFILE	2
#define VPTYPE_MENU		5
#define VPTYPE_LEGEND	6
#define VPTYPE_INDEX	7
#define VPTYPE_LEGENDIMAGE	8
#define VPTYPE_SUBVIEWPORT	9

#ifdef	_WIN32_WCE
typedef long clock_t;
/* OpenFile() Structure */
typedef struct tagOFSTRUCT
{
    BYTE cBytes;
    BYTE fFixedDisk;
    UINT nErrCode;
    BYTE reserved[4];
    char szPathName[128];
} OFSTRUCT;
typedef OFSTRUCT *LPOFSTRUCT;

#define GF_MESSAGE_RANGE	50000

/* OpenFile() Flags */
#define OF_READ 	    0x0000
#define OF_WRITE	    0x0001
#define OF_READWRITE	    0x0002
#define OF_SHARE_COMPAT	    0x0000
#define OF_SHARE_EXCLUSIVE  0x0010
#define OF_SHARE_DENY_WRITE 0x0020
#define OF_SHARE_DENY_READ  0x0030
#define OF_SHARE_DENY_NONE  0x0040
#define OF_PARSE	    0x0100
#define OF_DELETE	    0x0200
#define OF_VERIFY	    0x0400      /* Used with OF_REOPEN */
#define OF_SEARCH	    0x0400	/* Used without OF_REOPEN */
#define OF_CANCEL	    0x0800
#define OF_CREATE	    0x1000
#define OF_PROMPT	    0x2000
#define OF_EXIST	    0x4000
#define OF_REOPEN	    0x8000
#endif

#define HFILE_DGN	-2

#define SYMTYPEPARENT	0
#define SYMTYPEPOINT	1
#define SYMTYPELINE		2    
#define SYMTYPEAREA		3

#define SVTEXT	1              
#define	SVLINE	2
#define SVAREA	3    
#define SVVARCOLOR	1   
#define SVNULLCOLOR 2

#define	NETMARKERTIMERID	4
#define	STREETEDITTIMERID	5
#define INFOBOXEDITTIMERID	7
#define	OKTOCONTINUETIMER	8
#define GPSTIMER	10 
#define TCPTIMER	21     
#define	MAXOPENSOCKETS	32

#define MT_PLT	0
#define MT_SHP	1
#define MT_ORA	2
#define MT_DGN7	3
#define MT_INDEX	4
#define MT_MACRO	5	
#define MT_IMAGE	6 
#define MT_DTM	7  
#define MT_SID	8
#define MT_PERSONAL_GEO_DB	9  

#define ORAT_NULL	0
#define ORAT_POINT	1
#define ORAT_ARC	3  
#define ORAT_TEXT	4
#define ORAT_POLYGON	5
#define ORAT_MULTIPOINT	8
#define ORAT_MIXED	10 

#define     UMIFS_DATAFILE      1
#define     FOXPRO_DATAFILE     2
#define     MSACCESS_DATAFILE   3
#define     ODBC_DATAFILE       4
#define     COMBO_DATAFILE      5   
#define     TEXT_DATAFILE       6 
#define		DBF_DATAFILE		7  
#define     DTM_DATAFILE 	    8 
#define		GMTEXT_DATAFILE		9 
#define		THEME_MEMFILE		10 
#define		GMCENSUS_DATAFILE	11  
#define		SHAPE_DATAFILE		12      
#define		SQL_DATAFILE		13  
#define		PGDB_DATAFILE		14


#define DTM_RENDER_RAW_POINTS		1                   
#define DTM_RENDER_GRID_POINTS		2
#define DTM_RENDER_SLOPE_VECTORS	3                   
#define DTM_RENDER_SLOPE_POLYGONS	4                   
#define DTM_RENDER_CONTOURS			5                  

#define BMP_DEFAULT         0
#define BMP_SAVE_RLE        1
#define CUT_DEFAULT         0
#define DDS_DEFAULT			0
#define ICO_DEFAULT         0
#define ICO_MAKEALPHA		1		// convert to 32bpp and create an alpha channel from the AND-mask when loading
#define IFF_DEFAULT         0
#define JPEG_DEFAULT        0
#define JPEG_FAST           1
#define JPEG_ACCURATE       2
#define JPEG_QUALITYSUPERB  0x80
#define JPEG_QUALITYGOOD    0x100
#define JPEG_QUALITYNORMAL  0x200
#define JPEG_QUALITYAVERAGE 0x400
#define JPEG_QUALITYBAD     0x800
#define KOALA_DEFAULT       0
#define LBM_DEFAULT         0
#define MNG_DEFAULT         0
#define PCD_DEFAULT         0
#define PCD_BASE            1		// load the bitmap sized 768 x 512
#define PCD_BASEDIV4        2		// load the bitmap sized 384 x 256
#define PCD_BASEDIV16       3		// load the bitmap sized 192 x 128
#define PCX_DEFAULT         0
#define PNG_DEFAULT         0
#define PNG_IGNOREGAMMA		1		// avoid gamma correction
#define PNM_DEFAULT         0
#define PNM_SAVE_RAW        0       // If set the writer saves in RAW format (i.e. P4, P5 or P6)
#define PNM_SAVE_ASCII      1       // If set the writer saves in ASCII format (i.e. P1, P2 or P3)
#define PSD_DEFAULT         0
#define RAS_DEFAULT         0
#define TARGA_DEFAULT       0
#define TARGA_LOAD_RGB888   1       // If set the loader converts RGB555 and ARGB8888 -> RGB888.
#define TIFF_DEFAULT        0
#define TIFF_CMYK			0x0001	// reads/stores tags for separated CMYK (use | to combine with compression flags)
#define TIFF_PACKBITS       0x0100  // save using PACKBITS compression
#define TIFF_DEFLATE        0x0200  // save using DEFLATE compression (a.k.a. ZLIB compression)
#define TIFF_ADOBE_DEFLATE  0x0400  // save using ADOBE DEFLATE compression
#define TIFF_NONE           0x0800  // save without any compression      
#define WBMP_DEFAULT        0
#define XBM_DEFAULT			0
#define XPM_DEFAULT			0

/* Handle to a DIB */
#define HDIB HANDLE

typedef signed long 		SLONG;
typedef signed short		SSHORT;
typedef unsigned long		ULONG;
typedef HFILE				FAR *LPHFILE;    
typedef ULONG				FAR *LPULONG;
typedef ULONG				HUGE *HPULONG;
typedef unsigned short		USHORT; 
typedef USHORT				FAR	*LPUSHORT;
typedef USHORT				HUGE *HPUSHORT;
typedef char				HUGE *HPSTR;
typedef short				HUGE *HPSHORT;
typedef long				HUGE *HPLONG; 
typedef WORD				HUGE *HPWORD;  
typedef unsigned char		HUGE *HPBYTE; 
typedef HBRUSH				FAR	*LPHBRUSH;
typedef HPEN				FAR	*LPHPEN;
typedef short				FAR *LPSHORT;
typedef float			 	FAR *LPFLOAT;
typedef float   			HUGE *HPFLOAT;
typedef double  			FAR *LPDOUBLE;
typedef double  			HUGE *HPDOUBLE;
typedef struct{double x,y;} DPOINT;
typedef DPOINT  			FAR *LPDPOINT;
typedef DPOINT  			HUGE *HPDPOINT;
typedef struct{double x,y,z;} DPOINT3D;
typedef DPOINT3D  			FAR *LPDPOINT3D;
typedef DPOINT3D  			HUGE *HPDPOINT3D;
typedef struct{float x,y;} FPOINT;
typedef FPOINT  			FAR *LPFPOINT;
typedef FPOINT  			HUGE *HPFPOINT;
typedef POINT				HUGE *HPPOINT;
typedef void				HUGE* LPHUGE;
typedef struct{long x,y;} LPOINT;
typedef LPOINT  			FAR *LPLPOINT;         
typedef LPOINT  			HUGE *HPLPOINT;         
typedef DWORD				HDIB32; 
typedef HDIB32				FAR *LPHDIB32; 
typedef RECT				HUGE *HPRECT;

typedef WORD (CALLBACK* FARHOOK)(HWND,UINT,WPARAM, LPARAM);

typedef	struct	{RECT	Rect;
				 long	Refno;   
				 USHORT	AZ;
				 short  Width, Height;
				 POINT	MidPoint;
			 	 short AddID;} EDITRECTINFO; 
typedef EDITRECTINFO	FAR	*LPEDITRECTINFO;

typedef struct
   {    double  xmn;
        double  ymn;
        double  xmx;
        double  ymx;
    } MNMXCORD;
typedef MNMXCORD    FAR *LPMNMXCORD; 
typedef MNMXCORD	HUGE *HPMNMXCORD;
typedef struct
   {	short	xmn;
   		short	ymn;
   		short	xmx;
   		short	ymx;
   	} mnmxCor;
typedef mnmxCor FAR	*LPMINMAX;
typedef struct
   {	LONG	xmn;
   		LONG	ymn;
   		LONG	xmx;
   		LONG	ymx;
   	} MNMXCORL;
typedef MNMXCORL FAR  *LPMNMXCORL;

typedef struct
     {  
     	short	Version;
     	MNMXCORD	Bounds;
     	double	CellSpacing; 
     	float	MinElev, MaxElev;
     	long	NumRows, NumCols;
     } LIDARFILEHEADER;
typedef struct
     {  
     	short	xoff,yoff;
     	float	Elevation;
     } LIDARPNT;
typedef struct
     {
       short	NumPoints;
       LIDARPNT	LidarPnt[MAXLIDARPERREC];
     } LIDARREC;
typedef LIDARREC    HUGE *LPLIDARREC;

typedef struct
     {
       short	NumPoints;
       DPOINT	XY[MAXLIDARPERREC];
       double	Z[MAXLIDARPERREC];
     } LIDARCELL;
typedef LIDARCELL    HUGE *LPLIDARCELL;

typedef struct
	{
		short	Over;
		short	Up;
		short	TriNum;
	}TRIANGLEID; 

typedef struct
	{
		TRIANGLEID	ID;
		float	Elev[3];
		short	SlopesToward;
		float	SlopePCT;
		float	SlopeAZ;
		short	InBasin;
	} TRIANGLEDATA;
typedef TRIANGLEDATA    FAR *LPTRIANGLEDATA;

typedef struct 
     {
       unsigned short rec, desc, type;
       long    ref;
       double  azm,   lngth,  eazm, rad,
               x1,    y1,     x2,    y2,    rx,   ry;      
       float   XMN,   XMX,  YMN,  YMX;
     } LINE; 
      
typedef LINE far *lpLine;
      
typedef struct {
				 MNMXCORD	Bounds;
				 MNMXCORD	InBounds; 
				 short		HaveInPoints;
				 double		Factor;
				 short		Width,
				 			Height;
				 double		AveX,
				 			AveY; 
				 short		Offset;
				 short		Type;
				 BYTE		array[];
				} PIAAStruct;
typedef PIAAStruct	HUGE	*LPPIAAStruct;

typedef struct
{
    long left;
    long top;
    long right;
    long bottom;
} RECT32;
typedef RECT32 FAR*  LPRECT32;

typedef struct
     {  HANDLE  TriHandle;
        double  A1, A2, B1, B2, C1, C2, BASX, BASY;
        BOOL    FIXEDP, ONE_SCALE;
        short   NSETPT; 
        MNMXCORD	Bounds, TriBounds; 
        short	LastTri;
     } TRANDATA;
typedef TRANDATA    FAR *LPTRANDATA;

typedef struct	{DPOINT FromPT[4];
				 DPOINT	ToPT[4];
				 MNMXCORD FromMNMX, ToMNMX; 
				 HANDLE	hFromTran, hToTran;
				 short	NumTri;
				 char	spacer[58];//aligns on 256 for huge ptr
				}TRANTRI; 
typedef	TRANTRI	HUGE	*HPTRANTRI;
    
typedef	struct	{
					int		PenNum, Style;
					float	Width;
					long	Color; 
				}	PENDESC;
typedef	PENDESC	FAR	*LPPENDESC; 

typedef struct {
		unsigned int	Transparent	:1;
		unsigned int	BGOpt		:1;
		unsigned int	Pattern		:6;
		} PATBYTE; 
typedef PATBYTE	FAR	*LPPATBYTE;

typedef struct {
		unsigned int	r	:5;
		unsigned int	g	:5;
		unsigned int	b	:5;
		unsigned int	x	:1;
		} GSSiCOLOR16;
typedef GSSiCOLOR16 FAR  *LPGSSiCOLOR16;
			
typedef struct  {char   Prefix[10];
                 short  Len,IncBeg,IncLen,IncNDP,NumVar;
                 char   VarName[8][10];
                 short  VarStart[8], VarLen[8]; 
                 char	CurUDI[66];
                 double	CurINC;
                } TAGDEF;
typedef TAGDEF FAR  *LPTAGDEF;

typedef struct {short   length; char Value;} CURVAL;
typedef CURVAL  FAR *LPCURVAL;

typedef	struct	{long	Refno;
				 float	PCT;
				 } STREETMP;
typedef STREETMP	FAR	*LPSTREETMP;   
typedef struct
    { short  type,
             index,
             radix,
             scale;
      long   length,
             precision;       
      char   name[62]; 
      HANDLE    hCurVal;
    } FIELDINFO;
typedef FIELDINFO FAR  *LPFIELDINFO;

typedef struct
	{   
		short	NumFields;  
		HANDLE	DBHandle;
		char	DBName[128];
		char	Select[8192]; 
		char	From[1024];
		char	Where[1024]; 
		FIELDINFO	FldInfo[1];
	}SQLDATABASE;
typedef	SQLDATABASE FAR	*LPSQLDATABASE;

typedef struct
    {   
        HANDLE  myhandle,
                FileHandle,
                BufferHandle; 
        short   Type,
                NumSQLs,
                NumFields; 
        long    Offsetx; 
        HFILE	Fid;
        HANDLE  SQLHandles[MAXSQLPERFILE];
        char    fullpath[_MAX_PATH];   
        short	HaveNonStandardFields;
        FIELDINFO   FldInfo;
    }OPENFILEDATA;
typedef OPENFILEDATA    FAR *LPOPENFILEDATA;    

typedef struct
	{
		USHORT	OpCode;
		char	Arg1[2048],
				Arg2[2048];
	}	LOGICPSTATEMENT;
typedef	LOGICPSTATEMENT	FAR	*LPLOGICPSTATEMENT;
 
typedef struct
    {
        HANDLE  hGlobal;
        short   FieldNum;
        short	OpCode;
        char    String[256];
    }   SQLFIELD;
typedef SQLFIELD    FAR *LPSQLFIELD;
    
typedef struct
    {   
        HANDLE  myhandle,
                OFHandle; 
        short     OpenFileID;
        DWORD   lastreadtime;  
        short     st;
        long    Offset,
        		FirstLineOffset,
        		MacroID;
        LPVOID  hstmt;
        char    IDName[34];
        char    SQL[4096]; 
        short   IndexToUse; 
        short	Unique;
        short   NumGlobals;
        SQLFIELD    SQLField; 
    }OPENSQLDATA; 
typedef OPENSQLDATA FAR *LPOPENSQLDATA;    

typedef struct {
                char    Prefix[2], Suffix[2];
                double  Value;
                }   MARKERVAL;
typedef MARKERVAL   FAR *LPMARKERVAL;  

typedef struct {
				HBITMAP	hBM;
				RECT	Rect;
				LPVOID	pVP; 
				long	ID; 
				short	UserID;
				} SAVESCREEN;
typedef SAVESCREEN	FAR	*LPSAVESCREEN;


typedef struct
   {
      long FromId, ToId;
      double X, Y;
      long Irc;
   } TranP;
typedef TranP FAR *lpTranP;

typedef struct
   {
     long id,
          UofM,  //1 = feet, 2  = meters, 4 = degrees, 0 = get info elsewhere
          irc;
   }   LoadP;
typedef LoadP FAR *lpLoadP;

typedef struct
    {   
        HANDLE  Handle;
        short   Len;     
        short    Type;
        BYTE	ValueIsHandle;
        short	NumLinkedVars;  
        DWORD   changetime; 
        BOOL	ContainsGorF;
        BOOL	Save;
        char    Name[34];
        char    Value[256]; 
        HANDLE	LinkedVar[1];
    } VARINFO;
typedef VARINFO FAR *VARPNT;     

typedef	struct
	{	LONG	Segment;
		WORD	Offset, Element;
		LONG	Refno;   
		short	ViewID,
				filler1;
		short	FileNum;
		USHORT	FileInIndex;
		short	SubFile;
		unsigned	int	Type:8, 
						HasText:1,
						HasTextPointer:1, 
						HiPrecis:1, 
						Blocked:1,
						IsDispersed:1,
						filler2:3;
		short	Desc,
		 		ipen;
		UINT	NearPoint;
		short	NearSeg,
		 		UDILen;
		UINT	NumPoints;
		double	Length,
		 		PCT,
		 		OffDist,
		 		BPAZ,PPAZ,EPAZ,
		 		Area;
		DPOINT	BeginPoint, EndPoint, PickedPoint, NodePoint, FromPoint, ToPoint; //for curves NodePoint = POC 
		POINT	BeginPointFile, EndPointFile, PickedPointFile;
		MNMXCORD Rect;
		float	Elev; 
		char	Prefix[9], UDI[65];
		ULONG	MSLink;
	} PICKDATA;   
typedef	PICKDATA	FAR	*LPPICKDATA;

typedef	struct
	{	LONG	Segment;
		WORD	Offset, Element;
		LONG	Refno;   
		LPVOID	View;
		short	FileNum;
		USHORT	FileInIndex;
		short	SubFile,
		 		Type;
	} PICKDATAHEADER;   
typedef	PICKDATAHEADER	FAR	*LPPICKDATAHEADER;

typedef struct	{int	Number;
			 HANDLE	Handle;} SYMDESC;
typedef SYMDESC	FAR	*LPSYMDESC; 

typedef struct {
				unsigned short	Type:4,
								InVisible:1, 
								SolidLine:1,
								WidthIsMeters:1,
								Width:9; 
				COLORREF		Color;  
				RECT	Rect;
				} SYMBOLATTRIBUTE;
typedef SYMBOLATTRIBUTE FAR *LPSYMBOLATTRIBUTE;
		
typedef	struct
	{	short	SymNum,	Parent, Type;
	} CHILDLIST;
typedef CHILDLIST FAR *LPCHILDLIST;

typedef struct {
		double	Dist,
				AZM;
		int		Type;						// 0=begin/end, 1=poc, 2=(x:y)
		} VECTOR;
typedef VECTOR FAR *LPVECTOR;
		
typedef struct {
		short	Number,
				Parent;
		char	Name[34];
		char	Desc[61]; 
		float	BaseSize;
		char	InVisible;
		unsigned	int	Type:7,    					// 0=parent, 1=point, 2=line, 3=area      
						HasBitmap:1,
						BaseScale:4,
						GetDimensionsFromSizePoints:1, 
						NoSizeLimit:1,
						BaseSizeSet:1,
	 					Layered:1;
		short	NumElements; 
		DPOINT	TiePoint,      				//for line items x is default scale (200ft/in if 0)
				SizePointV[2],
				SizePointH[2];
		double	VSize,HSize;
		HANDLE	hElement;
	} SYMBOL;
typedef SYMBOL	FAR	*LPSYMBOL; 

typedef struct {
		short		NumVectors;
		long		LineColor, FillColor;
		short		Width; 
		short		Type; 					// 1=embedded text, 2=polyline, 3=polygon, -1=begin sym, -2=varline, -3=endsymbol
		unsigned short		Style:4,
		 			LineColorType:2,
					FillColorType:2, 
					Squared:1,
					Dummy:7;
		VECTOR		Vector;                 //first vector must have min x value for linear syms
		}	ELEMENT;  
		
typedef ELEMENT	FAR	*LPELEMENT;  

typedef struct
	{
		char	Name[128];
		char	OrigName[68];
		BOOL	DIBColorsArePalleteEntries;
		long	Frame;
		HDIB32	hDib;
		short	DeleteBM;
		MNMXCORD	Bounds;
		double	Res;
		short	Width, Height,BitCount;
		long	LastUsed;
	}	ORTHO;
typedef ORTHO	HUGE	*LPORTHO;

typedef struct
	{	HANDLE	hVisList;
		LPSTR	LastVisList,
				NextVisList;
		double	MinScale,
				MaxScale;
		short	WantType[10];
//CurVis->WantType[0] = area
//					1	= PLC
//					2	= Text
//					3	= Contours
//					4	=  
//					5	= Orthos 
//					6	= Symbols 
//					7	= Area hatch,fill,border 
//					8	= Invis 
//					9	= Width 
		short	FileIsVisible[MAX_VIEWPORT_FILES];
		short		MinPointSize;
		short		MaxPointSize;
		BYTE	VisBits[400];

	}	VISLIST;
typedef VISLIST	FAR	*LPVISLIST;

typedef struct {                                        
				HANDLE	hDB;                            //handle of file if .dtm or viewport if TIN Plt files
				short	Type;
				HFILE	Fid;							//1 = dtm grid, 2 = TIN plt files, 3 = Lidar points
				double	NULLElv;
				double	GridSpace; 
				short	ElevUnits; 						//0=feet,1=feet*100,2=decimeters,3=meters     
				short	CoordUnits;                     //0=meters, 1=feet
				DPOINT	SouthWestNode; 
				MNMXCORD	Bounds; 
				double	MaxDistToRawPoint;
				long	MaxRawPointsToUse;
				long	NumRows,
						NumCols;
				HANDLE	hCell[MAXDTMCELLBUFFERS];
				long	CellID[MAXDTMCELLBUFFERS];
				long	CellUse[MAXDTMCELLBUFFERS];  
				char	TINIndex[256];
				VISLIST	VisList; //location of data for TIN surface
				} DTMINFO;
typedef DTMINFO	FAR	*LPDTMINFO;

typedef struct {
				double	NULLElv;
				double	GridSpace; 
				short	ElevUnits; 						//0=feet,1=feet*100,2=decimeters
				short	CoordUnits;						//0=meters,1=feet
				DPOINT	SouthWestNode;
				MNMXCORD	Bounds; 
				} DTMDATA;
typedef DTMDATA	FAR	*LPDTMDATA;
typedef struct	{	DPOINT	Point;
					double	Dist;
				} BLOCKINGPOINT;
typedef BLOCKINGPOINT	FAR	*LPBLOCKINGPOINT;

typedef struct	{   
					char	Name[34],
							Image[130],
							InfoFile[130],
							HistoryFile[130];
					HANDLE	hPoly,
							hMoveList;  
					short	nMoveList;
					long	nPnts;
					double	RouteLength;
				}DUMMYVEHICLE;
typedef DUMMYVEHICLE	FAR	*LPDUMMYVEHICLE;  

typedef struct	{   
					short	Command; 
					long	StartTime,EndTime;
					double	StartMilePoint,EndMilePoint,
							Value;
				}MOVELIST;
typedef MOVELIST	FAR	*LPMOVELIST;

 typedef struct {
 					DPOINT	WorldPoint;
 					DPOINT	DigitizerPoint; 
 					char	HaveWorld, HaveDig;
 				}	DIGCNTLPOINT;
 typedef DIGCNTLPOINT	FAR	*LPDIGCNTLPOINT; 

typedef struct {
				unsigned	long	GEOSEG_ROW:12, 
									GEOSEG_COL:12,
									SUBCEL_ROW:4,
									SUBCEL_COL:4;
				} DTMKEY;
typedef DTMKEY	FAR	*LPDTMKEY;

typedef struct {
				unsigned	short	LENGTH:12, 
									TYPE:3,
									INDT:1;
				} SUBCELLINFO_v1;

typedef struct {
				unsigned	short	LENGTH:13, 
									TYPE:2,
									INDT:1;
				} SUBCELLINFO;
typedef SUBCELLINFO	FAR	*LPSUBCELLINFO;  

typedef struct {
					long	FromSeg;
					double	AtDist, FromDist, Length;
					BOOL	Reverse;
					long	FirstPointID;
					MNMXCORD Bounds; 
					PICKDATAHEADER   PD;  
					char	Filler[42];
				} SEGMENTDATA;
typedef SEGMENTDATA	HUGE	*HPSEGMENTDATA;

typedef struct {long	Segno;
				double	AtDist, TurnAZ;
				long	NextSeg;} INTERSECTIONKEY;
typedef INTERSECTIONKEY	FAR	*LPINTERSECTIONKEY;   
 
typedef struct {long OnRef;
				DPOINT	IntPoint;}INTERSECTIONDATA;
typedef struct	{   
					long	SecondsRepresented;
					float	Radius;
					unsigned	int	DecayOpt:4, 
									ColorOpt:6,
									PassThrough:2,
									PromptForSavex:2,
									filler:2;
					short	 
							//Granularity,//no longer used
							GridWidth,
							GridHeight,
							MaskWidth;
					long	TotalIncidents,
							MaxGridValue;
					//		ColorMax,
					//short	ColorFactor;
					HANDLE	hMask,
							hGrid,
							hTranBaseToHotSpot; 
				}HOTSPOTDATA;
typedef HOTSPOTDATA	FAR	*LPHOTSPOTDATA;

typedef struct {
		short 	Class;
		long	Refno;
		} THEMEHIGHLIGHTKEY;

typedef struct
    {
        short	Seq;
        long	Ref;
        char	Prefix[8],
        		UDI[32];
        char	Type;
        char	SymbolName[32];   
        char	File[255];
        double	Width,
        		BPX,
        		BPY,
        		BPZ,
        		EPX,
        		EPY,
        		EPZ;
    }   PROFILEFILEDATA;
typedef PROFILEFILEDATA	FAR	*LPPROFILEFILEDATA;

typedef struct
    {	POINT	Points[4];
    	long	Offset;
    }	PROFILEDATARECTANGLE;
typedef PROFILEDATARECTANGLE	FAR *LPPROFILEDATARECTANGLE;  

typedef struct {
		short 	FileNum,
				SubFile;
		USHORT	FileInIndex;
		long	Segment;
		WORD	Offset, Element; 
		DPOINT	Point; 
		double	Length, Area; 
		char	Prefix[10],UDI[34];
		char	Value[128];
		}THEMEHIGHLIGHTDATA;
		
typedef struct {
				short	Type;   
				DPOINT	Loc[3];  
				double	size,ascent,intleading,avecharwidth;       
				short	Font;
				short	lText; 
				char	Text[256];
				} EXPORTTEXT;
typedef	EXPORTTEXT	FAR	*LPEXPORTTEXT;

typedef struct {
				short	Type;
				short	nPoints;   
				DPOINT	Loc[3];  
				} EXPORTTEXTPOINTER;
typedef	EXPORTTEXTPOINTER	FAR	*LPEXPORTTEXTPOINTER;   

typedef struct {
				COLORREF Color;
				short	Width;   
				short	Style;  
				} LINEDESCRIPTION;
typedef	LINEDESCRIPTION	FAR	*LPLINEDESCRIPTION;   

typedef struct {
				COLORREF ForeColor,
						 BackColor;
				short	Pattern;   
				} AREADESCRIPTION;
typedef	AREADESCRIPTION	FAR	*LPAREADESCRIPTION;   

#define	MAXSIGNATURECOLORS	16 
#define MAXLEGENDS	64
typedef	struct	{
					short	nColors;
					RGBTRIPLE	Colors[MAXSIGNATURECOLORS];
					short	ColorsN[MAXSIGNATURECOLORS];
				}FILLSIGNATURE;  
typedef FILLSIGNATURE FAR	*LPFILLSIGNATURE;
 

typedef struct {char	ID[32],   
						Desc[128],
						Macro[32];
				COLORREF	Color;     
				BOOL	ComputeAZ, ComputeSpeed;
				long	ComputeTimeSpan;
				short	nLoc,
						Size[2],
						Symbol,
						StatusSymbol;
				double	Speed;
				long	SaveScreenID[MAX_VIEWPORTS];
				HANDLE	hSaveScreen[MAX_VIEWPORTS];
				DPOINT	Loc[MAXVEHLOC];         
				time_t	LocTime[MAXVEHLOC]; 
				short	Speeds[MAXVEHLOC];
				short	Heading[MAXVEHLOC]; 
				short	Status;
				} VEHLOCATION;
typedef	VEHLOCATION	FAR	*LPVEHLOCATION;

typedef struct {char	ID[8];
				DPOINT	Point[2];} EDGE;
typedef	EDGE	FAR	*LPEDGE;

typedef struct {char	ID[8];
				DPOINT	Point;} CORNER;
typedef	CORNER	FAR	*LPCORNER;

typedef struct {char PREFIX[8],
					 UDI[32];
				long Refno;} TAGKEY;
typedef TAGKEY	FAR	*LPTAGKEY;

typedef struct {
				short	Type;
				long	nSavePoly;
				HANDLE	hSavePoly; 
				HANDLE	hSavePolyParts; 
				HANDLE	hSavePolyElev;
				} SAVEPOLY;
typedef SAVEPOLY	FAR	*LPSAVEPOLY;

typedef struct {
		long	Ref, LastRef, StartRef,Sequence; 
		short	SymNum, Type; 
		short	NumNext;
		long	NextRef;
		short	FromPCT, ToPCT;
		double	Cost, LastCost;  
		} ROUTEATFILE;
typedef	ROUTEATFILE	FAR	*LPROUTEATFILE;

#define	TRACK_LEFT	1
#define TRACK_RIGHT 2
typedef struct	{
				LPOINT	Point1;
				LPOINT	Point2;
				long	Refno;
				long	Mnx,Mxx;
				long	filler;
				} LINEINT;  
typedef LINEINT	HUGE	*LPLINEINT; 
typedef struct	{
				double	Size;
				long	LoopID;
				}	LOOPKEY;    

typedef struct	{
					long	LinkID;
					short	WhichEnd; 
					DPOINT	Point;
				}	NODESDATA;
typedef NODESDATA	FAR	*LPNODESDATA;

typedef struct {
		long	LinkRef;
		short	Dir;
	   }	LOOPREC;   
typedef LOOPREC	FAR	*LPLOOPREC;			

typedef struct {
		long	Ref;
		LPOINT	BPFile,EPFile; 
		short	ConnectStatus,
				BPConnections, EPConnections,
				NumPoints,
				Symbol;
		DPOINT	BPBase, EPBase;               
		double	Length;
		} REFCONNECT;
typedef	REFCONNECT	FAR	*LPREFCONNECT;   

typedef struct	{
					long	LeftPoly,RightPoly;  
					DPOINT	FromPoint, POCPoint,ToPoint; 
					short	IsCurve;
				}	LINESDATA;
typedef LINESDATA	FAR	*LPLINESDATA;

typedef struct	{
			LPOINT	FromNode,POCNode,ToNode;
		}	LINESKEY;
typedef LINESKEY	FAR	*LPLINESKEY;

typedef struct {
				MNMXCORD MinMax; 
				long	Offset;
				DPOINT	Point1;
				short	NumLinks,
						NumSides; 
				BOOL	Reverse; 
				long	StoredAsInclusionRef;
				}	LOOPDATA;

typedef struct {
 					short	Symbol,
 							TextStart;
 					float	SizeFactor; 
 					float	TextFactor;
 					COLORREF TextColor;
 					char	IDText[32]; 
 					char	ProcessText[128];
 				}	SHIELDS;
typedef SHIELDS	FAR	*LPSHIELDS; 

typedef struct	{
			long	ID;
			short	SubDir;
			long	Size,Time,NumPoints;
			DWORD	Loc;     
			double	LLNormFactor;
			double	MetersPerDegree;
			double	Scale;    
			MNMXCORD	Bounds;
		}	MAPFILE;
typedef MAPFILE	FAR	*LPMAPFILE;

typedef struct {
		long	AreaRefno; 
		long	PointRefno;
		char	ID[34];   
		char	AreaUDI[66];
		char	PointUDI[66];
		long	MultipleID;
	   }	POLYID;   
typedef POLYID	FAR	*LPPOLYID;			

 typedef struct {
 					LPOINT	NodeID;
 					double	AZ;
 					long	Refno;
 					short	WhichEnd;
 				}	NODELINK;
 typedef NODELINK	FAR	*LPNODELINK;
 
 typedef struct {
 					LPOINT	NodeID;
 					DPOINT	BasePoint;
 					short	MatchPointStatus;
 				}	NODES;
 typedef NODES	FAR	*LPNODES;  
 
 typedef struct {
 					LPOINT	FromNode,
 							ToNode; 
 					double	FromAZ,
 							ToAZ,
 							DeltaAZ;
 					short	TrackedLeft,
 							TrackedRight;
 					long	Offset;
 				}	LINKDATA;
 typedef LINKDATA	FAR	*LPLINKDATA;

 
typedef struct {char Name[32];
                MNMXCORD    Bounds;
                BOOL    HaveBounds;
                HANDLE	hMask; 
                HANDLE	hMaskAccelerator[3];
                long	NumMaskPoints;
                short	NumMaskAreaParts;} SAVEDZOOM;
typedef SAVEDZOOM FAR   *LPSAVEDZOOM;


typedef struct {HANDLE	MyHandle,
						PrevHandle;
				short	VP; 
				unsigned short	CurLoc,
								EndLoc,
								CurFun,
								CommandLimit,
								CommandSuccessCount; 
				BOOL	AddLBUTTON;  
				HCURSOR	hCursor; 
				HANDLE	hWhile; 
				HANDLE	hError;
				UINT	Prompt;
				char	PromptText[128];
				char	Cmd[];
			   }	CMDSTRING;
typedef CMDSTRING	FAR	*LPCMDSTRING;

typedef struct {
				short	version, length;
				MNMXCORD MinMax;
				} IMPORTLIMITS;
typedef IMPORTLIMITS	FAR	*LPIMPORTLIMITS;

typedef struct {
				short	version, length;
				char	Filter[1024];
				} IMPORTFILTER;
typedef IMPORTFILTER	FAR	*LPIMPORTFILTER;

typedef struct {
				short	version, length; 
				short	lcmdstring;
				char	CmdString[256];
				} GRCOMMAND;
typedef GRCOMMAND	FAR	*LPGRCOMMAND;

typedef struct {
				short	version, length; 
				char	StartTime[256], EndTime[256];
				} TIMESTAMP;
typedef TIMESTAMP	FAR	*LPTIMESTAMP;

typedef struct {
				short	version, length; 
				char	SetRefno[256];
				} SETREFNO;
typedef SETREFNO	FAR	*LPSETREFNO;

typedef struct {
				short	version, length;
				short	FontNum;
       			char	spacer[48]; 
       			char	cHeight[64];
       			char	cColor[64];
       			short	hJust,
       					vJust;
				unsigned	int		FlipForEasyReading:1, 
									UltiMapStyle:1,
									italic:1, 
									weight:2, 
									Opaque:1,
									filler:10;
				short	ltext;
				char	Text[256];
				} GRTEXT;
typedef GRTEXT	FAR	*LPGRTEXT;

typedef struct {
				short	style;
				DPOINT	points[2];
				char	spacer[4];
				} TEXTTPL;
typedef TEXTTPL	FAR	*LPTEXTTPL;

typedef struct {
				unsigned	int		style1:4, 
									style2:4,
									lorc1:2,
									lorc2:2,
									filler:4;
				FPOINT	points[6];
				} UMTEXTTPL;
typedef UMTEXTTPL	FAR	*LPUMTEXTTPL;

typedef	struct	{long	LastRef, StartRef,Sequence;
				 double	Cost, LastCost;
				 short	FromPCT, ToPCT;
				 LONG	Segment;
				 WORD	Offset;
				 int	FileNum;  
				 int	SubFile;
		         USHORT	FileInIndex; 
		         long	StreetNums[4];
		         long	NextRef;
		         short	SymNum;
		         short	Type; 
		         short	NumNext;
				 } ATDATA;  
typedef	ATDATA	FAR	*LPATDATA;    
typedef struct {long	Ref; 
				short	RouteID;
				long	Sequence;
			   }	ATKEY;
typedef ATKEY	FAR	*LPATKEY;
// Note: the fields in ATDATA and NEXTDATA down to StreetNum must match for SetRouteStreetNum to work
typedef	struct	{long	AtRef, StartRef,Sequence; 
				 double	AtCost, NextCost;
				 short	FromPCT, ToPCT;
				 LONG	Segment;
				 WORD	Offset;
				 short	FileNum;  
				 short	SubFile;
		         USHORT	FileInIndex;
		         long	StreetNums[4];
				 DPOINT	OpenEnd; 
				 long	Path;
				 double	MP;  
				 short	Dir; 
				 long	WhenAssigned; 
				 short	SymNum;
				 short	Type;
				 } NEXTDATA;  
typedef	NEXTDATA	FAR	*LPNEXTDATA;
typedef struct	{double	Cost;
				 long	Refno;
				} NEXTKEY;         
typedef NEXTKEY	FAR	*LPNEXTKEY;   

typedef struct	{short	Type;
				 COLORREF	Color;
				 short	Width;
				 } SPECIALDATA;
				 
typedef struct {char Name[256-32];MNMXCORD MinMax;} FILELISTENTRY;
typedef FILELISTENTRY	HUGE	*LPFILELISTENTRY;  

typedef	struct {long ref;
				short Layer, State, width, height, symnum;
				short unusedspacer;
				POINT Point;
				float AZ,Length;
				long Refno;} MIDPOINT;
typedef	MIDPOINT	HUGE	*LPMIDPOINT;

typedef struct {int	Desc; long Num;} REORG;
typedef REORG	FAR	*LPREORG;
typedef struct {int	Desc; long Offset;} DESCBLOCK;
typedef DESCBLOCK	HUGE	*LPDESCBLOCK;

typedef	struct	{
				long	Path;
				double	StartMP;
				}	NETMARKERSTHEMEKEY;  
				
typedef struct	{  
				int		Code101;
				long	UsedDescOffset;
				int		Code102;
				long	ColorPaletteOffset;
				int		Code103;
				long	TranPointOffset;
				int		Code200;
				long	GraphicsOffset; 
				int		Code201;
				long	DescBlockOffset; 
				int		Code202;
				long	MinTime,MaxTime; 
				int		Term;
				} PRIMEOFFSETS;  
typedef	PRIMEOFFSETS	FAR	*LPPRIMEOFFSETS;

typedef struct {
				char	QuadID[12];
				char	VolLabel[16];
				char	Date[8];
				long	FltHeight;
				char	GSSiCD[6]; 
				short	GSSiFile;
				short	Status; 
				short	Datum, ProjectionCode;
				long	ZoneCode;
				double	Res;
				long	nRows, nCols; 
				RECT	ClipRect;  
				DPOINT	SWClipPoint,
						NWClipPoint,
						NEClipPoint,
						SEClipPoint;
				} QUADDATA;
typedef QUADDATA	FAR	*LPQUADDATA;				

typedef struct {
				char	QuadID[12];
				char	GSSiCD[6]; 
				short	GSSiFile;
				short	Status; 
				long	DateCreated;
				} CDDATA;
typedef CDDATA	FAR	*LPCDDATA;				

typedef struct
	{   int		Marker;
		mnmxCor	MinMax;
		int		Len;
	} ITEM;
typedef ITEM	FAR *LPITEM;

typedef struct
{	mnmxCor MinMax;
	long offset;
}	SegHeader;

typedef	struct	{USHORT	FileInIndex;
				 unsigned	long	Segment:31,
				 					Deleted:1;
				 WORD	Offset; 
				 RECT	MinMax;
				 }	REFINDEXDATA; 
typedef	REFINDEXDATA	FAR	*LPREFINDEXDATA;

typedef struct {
				unsigned	int		lText:9, 
									FontNum:5,
				 					Weight:2;
				unsigned	int		vJust:2,      //0=above,1=baseline,2=center,3=below
									hJust:2,      //0=left,1=center,2=right
									italic:1,
									underline:1,  
									HeightPrecision:2, 
									Opaque:1, 
									HeightIsPixels:1,
									hJust2:4,		//fractional movement back toward center 
									UltiMapStyle:1,
									FlipForEasyReading:1;
				unsigned	short	Height;			//base units*10 (or *100000 if degree units)
                } GRTEXTHEADER;
typedef	GRTEXTHEADER	FAR	*LPGRTEXTHEADER;

typedef struct
{	mnmxCor MinMax;
	USHORT Next[2];
	long TypeOffset[10];
}	QUAD;
typedef QUAD HUGE *LPQUAD;

typedef	struct
	{	int		Parent;
		HANDLE	Next;
	} PARLIST;
typedef PARLIST FAR *LPPARLIST;


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
    }	COORDINATEDISPLAY_V0;

typedef struct
	{
		short	ID;
		HANDLE	handle;
		short	Version;
		short	TargetViewport;
		short	DisplayViewport;
		BOOL	DisplayLine[3];
		char	AltCVTFile[14],
				LineID[3][34];
		short	Units[3],
				Precision[3];
		BOOL	Commas[3];
       	LOGFONT	Font;  
       	COLORREF	FontColor;  
       	BOOL	Refresh; 
       	char	PrintText[100];
       	char	Space[22];
    }	COORDINATEDISPLAY_V101;
typedef	COORDINATEDISPLAY_V101	FAR	*LPCOORDINATEDISPLAY_V101;

typedef struct
	{
		short	ID;
		HANDLE	handle;
		short	Version;
		short	TargetViewport;
		short	DisplayViewport;
		BOOL	DisplayLine[4];
		char	AltCVTFile[14],
				LineID[4][34];
		short	Units[4],
				Precision[4];
		BOOL	Commas[4];
       	LOGFONT	Font;  
       	COLORREF	FontColor;  
       	BOOL	Refresh; 
       	char	PrintText[100];
       	char	Space[22];
    }	COORDINATEDISPLAY_V102;
typedef	COORDINATEDISPLAY_V102	FAR	*LPCOORDINATEDISPLAY_V102; 

typedef struct
	{
		short	ID;
		HANDLE	handle;
		short	Version;
		short	TargetViewport;
		short	DisplayViewport;
		BOOL	DisplayLine[4];
		char	AltCVTFile[14],
				LineID[4][34];
		short	Units[4],
				Precision[4];
		BOOL	Commas[4];
       	LOGFONT	Font;  
       	COLORREF	FontColor;  
       	BOOL	Refresh; 
       	char	PrintText[100]; 
       	char	ElevSurface[128]; 
       	HANDLE	hSurf; 
       	short	ElevUnits,
       			ElevPrecision;
       	BOOL	ElevCommas;
       	BOOL	DisplayElevation;
       	char	ElevationID[34];
       	char	Space[470];
    }	COORDINATEDISPLAY;
typedef	COORDINATEDISPLAY	FAR	*LPCOORDINATEDISPLAY; 

typedef struct
	{
		PICKDATA	PD;
		HANDLE		ItemHandle; 
    }	SAVELISTDATA;

typedef struct
	{
		int		NumItems; 
		SAVELISTDATA	SLD[]; 
    }	SAVELIST;
typedef	SAVELIST	FAR	*LPSAVELIST;
typedef struct    		
	{
		int		ID;
		HANDLE	handle; 
		int		Version;
		int		TargetViewport,
				DisplayViewport;
		BOOL	IsActive,
				WantDataPass,
				ComputeClassBoundaries,
				DisplayScatterDiagram,
				Recompute,
				ReScan;  		// also holds network opened flag, TargetViewport2
		HANDLE	hThemeDB,
				hScatterFile;
		char	DataFile[128];
		char	SQL[256];
		int		DataFileType,
				DataType;
		FIELDINFO	Field[1]; 
		short	ClassSymbol[MAX_THEME_CLASSES_v0];      
		char	SymSizeC[32];
		BYTE	ClassIsSelected[MAX_THEME_CLASSES_v0];
		LPVOID	Statement;
		char	ScatterFile[128];
		int		NumClass;
		int		NumDesiredClass;
        int		ClassType;
        int		ValConv;  
        int		MissOpt;
        BOOL	MarkInvalid;
        double	XLimit,
        		YLimit,
        		RoundTo;
        long	NumVals;
        double	Xmin,Ymin,Xmax,Ymax;
        double	ClassMin[MAX_THEME_CLASSES_v0],
        		ClassMax[MAX_THEME_CLASSES_v0];
        COLORREF	ClassColor[MAX_THEME_CLASSES_v0]; 
        HPEN		ClassPen[MAX_THEME_CLASSES_v0];
        HBRUSH		ClassBrush[MAX_THEME_CLASSES_v0],
        		NoDataBrush,
        		InvalidDataBrush;
        long	ClassCount[MAX_THEME_CLASSES_v0];
        DPOINT	ClassPnt[MAX_THEME_CLASSES_v0];
        COLORREF	BGColor, ScatterColor, ScatterBoxBG, TitleBoxBG;
        RECT	Rect, TitleBox, ScatterBox, ColorsBox, RangesBox, InfoBox;
        RECT	ClassClrBox[MAX_THEME_CLASSES_v0];
        int		Margin, ScatterWidth, ColorsWidth, InnerMargin, TitleHeight;
        char	ClassBM[MAX_THEME_CLASSES_v0][128];
        char	Title[256]; 
        char	Contents[10];
        int		SymNum;
       	LOGFONT	TitleFont,
       			ClassFont1,
       			ClassFont2;
       	int		Xmove, Ymove;  	// Xmove also used to hold NetworkID, DisplayOpt
       	BOOL	ZeroIsMissing,
       			AddCommas,
       			ShowValue,
       			VPDisplayed,
       			DisplayPCT; 
       	char	RefValChar[36]; 
       	double	RefValDbl;  
       	short	FieldFun[1], 
       			ShortDummy1;
       	short	FieldCorrection[1],
       			ShortDummy2;   
       	COLORREF	IBBGColor, TitleTextColor, IBTextColor[2]; 
       	short	RefIsPCT; 
       	char	Value[128]; 
       	HANDLE	hVisList;
       	short	DispersePoints;
       	HANDLE	hDisperseFileName,
       			hDisperseFile;
       	short	MaxDispersion;
       	HANDLE	hHighlightFileName,
       			hHighlightFile;
       	BYTE	AccumPointSymbolOpt;
       	BYTE	AccumPointSizeOpt; 
       	float	AccumPointBaseSize;  
       	BYTE	AccumPointLink; 
       	BYTE	AccumPointText;
       	BOOL	ZeroBased;
       	BOOL	PCTByArea;
       	long	NumNonMask;
       	long	PCTDisplayCycle; 
       	short	LayerID;
       	BOOL	InvertLegend;
       	short	NumCols;
       	BOOL	HiPrecis;
       	short	NumMidpoint;  
       	short	AllValueClass; 
		unsigned	int	ClearIfNoCount:1,
						DisplayPointsOnly:1,
						FlipLegend:1,
						FactorLegend:1,
						DisplayCount:1,
						ShowOnlySelectedClasses:1,
						HideNullClasses:1,
						DummyFlag8:1,
						DummyFlag9:1,
						DummyFlag10:1,
						DummyFlag11:1,
						DummyFlag12:1,
						DummyFlag13:1,
						DummyFlag14:1,
						DummyFlag15:1,
						DummyFlag16:1;
	}	THEME_v0;
typedef THEME_v0	FAR	*LPTHEME_v0;

typedef struct    		
	{
		short	ID;
		HANDLE	handle; 
		short	Version;
		short	TargetViewport,
				DisplayViewport;
		BOOL	IsActive,
				WantDataPass,
				ComputeClassBoundaries,
				DisplayScatterDiagram,
				Recompute,
				ReScan;  		// also holds network opened flag, TargetViewport2
		HANDLE	hThemeDB,
				hScatterFile;
		char	DataFile[128];
		char	SQL[256];
		short	DataFileType,
				DataType;
		FIELDINFO	Field[1]; 
		short	ClassSymbol[MAX_THEME_CLASSES];      
		char	SymSizeC[32];
		BYTE	ClassIsSelected[MAX_THEME_CLASSES];
		LPVOID	Statement;
		char	ScatterFile[128];
		short		NumClass;
		short		NumDesiredClass;
        short		ClassType;
        short		ValConv;  
        short		MissOpt;
        short	MarkInvalid;
        double	XLimit,
        		YLimit,
        		RoundTo;
        long	NumVals;
        double	Xmin,Ymin,Xmax,Ymax;
        double	ClassMin[MAX_THEME_CLASSES],
        		ClassMax[MAX_THEME_CLASSES];
        COLORREF	ClassColor[MAX_THEME_CLASSES]; 
        HPEN		ClassPen[MAX_THEME_CLASSES];
        HBRUSH		ClassBrush[MAX_THEME_CLASSES],
        		NoDataBrush,
        		InvalidDataBrush;
        long	ClassCount[MAX_THEME_CLASSES];
        DPOINT	ClassPnt[MAX_THEME_CLASSES];
        COLORREF	BGColor, ScatterColor, ScatterBoxBG, TitleBoxBG;
        RECT	Rect, TitleBox, ScatterBox, ColorsBox, RangesBox, InfoBox;
        RECT	ClassClrBox[MAX_THEME_CLASSES];
        short	Margin, ScatterWidth, ColorsWidth, InnerMargin, TitleHeight;
        char	ClassBM[MAX_THEME_CLASSES][128];
        char	Title[256]; 
        char	Contents[34];
        short	SymNum;
       	LOGFONT	TitleFont,
       			ClassFont1,
       			ClassFont2;
       	short	Xmove, Ymove;  	// Xmove also used to hold NetworkID, DisplayOpt
       	BOOL	ZeroIsMissing,
       			AddCommas,
       			ShowValue,
       			VPDisplayed,
       			DisplayPCT; 
       	char	RefValChar[36]; 
       	double	RefValDbl;  
       	short	FieldFun[1], 
       			ValueLen;
       	short	FieldCorrection[1],
       			ShortDummy2;   
       	COLORREF	IBBGColor, TitleTextColor, IBTextColor[2]; 
       	short	RefIsPCT; 
       	char	Value[128]; 
       	HANDLE	hVisList;
       	short	DispersePoints;
       	HANDLE	hDisperseFileName,
       			hDisperseFile;
       	short	MaxDispersion;
       	HANDLE	hHighlightFileName,
       			hHighlightFile;
       	BYTE	AccumPointSymbolOpt;
       	BYTE	AccumPointSizeOpt; 
       	float	AccumPointBaseSize;  
       	BYTE	AccumPointLink; 
       	BYTE	AccumPointText;
       	BOOL	ZeroBased;
       	BOOL	PCTByArea;
       	long	NumNonMask;
       	long	PCTDisplayCycle; 
       	short	LayerID;
       	BOOL	InvertLegend;
       	short	NumCols;
       	BOOL	HiPrecis;
       	short	NumMidpointnotused;  
       	short	AllValueClass; 
		unsigned	int	ClearIfNoCount:1,
						DisplayPointsOnly:1,
						FlipLegend:1,
						FactorLegend:1,
						DisplayCount:1,
						ShowOnlySelectedClasses:1,
						HideNullClasses:1,
						NotSetColor:1,
						AppendCount:1,
						CompressNullClasses:1,
						DisplayDistance:1,
						AutoClassDef:1,
						UseFirstSymbol:1,
						FillRow:1,
						CenterText:1,
						Unusedbit:1; 
		char	ClassDefDB[128], 
				ClassDefSQL[256],
				ClassDefKeyField[34],
				ClassDefSymField[34],
				ClassDefTitleField[34],
				ClassDefValDB[128],    
				ClassDefValSQL[256],   //also used for hotspot radius
				ClassDefValField[34];
		char	SymbolFont[4][32];  
		BYTE	ClassStatus[MAX_THEME_CLASSES];
		HOTSPOTDATA HotSpotData; 
		float	ClassFactor[MAX_THEME_CLASSES]; 
		char	HotSpotCompareTo[128];
		char	HotSpotSaveTo[128];  
		double	CompareHotSpotFactor;
		char	IconLibrary[128];//also used for hotspot weight 
		short	ActualXMargin;
		short	ActualYMargin; 
		LOGFONT	ShowValueFont;
		COLORREF ShowValueTextColor; 
		short	Config;  
		short	UseHalfTone; 
		short	HaveVP[2];  
		long	NextValueColor; 
		long	ValueColor; 
		long	NumMissing;
		long	NumInvalid; 
		short	SkipInvalid;
		short	GetFirstClass; 
		short	ColorScheme; 
		HANDLE	hHotSpotBitmap; 
		MNMXCORD	HotSpotBounds;  
		unsigned	int	ComputeStoredCounts:1,
						UseStoredCounts:1,
						UseCheckmark:1, 
						DelayTextDisplay:1,
						Unusedbit4:1,
						Unusedbit5:1,
						Unusedbit6:1,
						Unusedbit7:1,
						Unusedbit8:1,
						Unusedbit9:1,
						Unusedbit10:1,
						Unusedbit11:1,
						Unusedbit12:1,
						Unusedbit13:1,
						Unusedbit14:1,
						Unusedbit15:1;  
		double	DistanceBetweenProfilePoints; 
		double	CrossSectionSpacing;
		double	CrossSectionWidth[2]; 
		POINT	RadiusPoint;
		double	Radius; 
		double	DynSegMaxFixedIncrement;
		double	DynSegFixedIncrement; 
		double	ShowValAZ;  
		short	ProfileUnits;
        char	Contents2[34];
        short	SymNum2;
       	HANDLE	hVisList2;
       	HANDLE	hAreas;
       	USHORT	NumAreas;
       	short	ACCDisperse; 
       	short	Pass;
       	HPEN	PointInAreaPen;
       	HBRUSH	PointInAreaBrush; 
       	COLORREF	PointInAreaColor;
       	HFILE	FidAreas; 
       	short	PointInAreaPointThemeVPID;  
       	long	NumMidpoint; 
       	char	ShowValMacro[128]; 
       	HFILE	FidDelayedText; 
       	HFONT	hDelayedFont;
		char	filler[22];
	}	THEME;
typedef THEME	FAR	*LPTHEME;

typedef struct
	{
		short	NameSource;
		short	LayerID;
		float	MinTextSize,MaxTextSize;
		char	Macro[256];
		BOOL	ShowCities;
		short	NameLength; 
		HANDLE	hNameFile1, hNameFile2;
		char	NameFile1[128];
		char	NameFile2[128]; 
		BOOL	Italic, Shadow;
		short	BackgroundOpt; //0=transparent,1=opaque,2=opaque if ortho on
		COLORREF	TextColor, ShadowColor;
		BOOL	ShowAllElements; 
		BOOL	SavePAE;  
		short	VJust; 
		BOOL	HorizontalText;
		BOOL	PrimeNameOnly;
		char	Expression[256]; 
		char	ListFile[128]; 
		char	ListData[128]; 
		short	ListLen;   
		BOOL	SortOnData;
		HANDLE	hListDB; 
		short	MinSize;
       	LOGFONT	StreetTextFont;
       	double	TextSize;
       	BYTE	IgnoreShields, 
       			AllowOverlap,
       			ScaleText;
		unsigned	char	UseFont:1,  
							AllowHollow:1,
							Dummy:6;
       	char	VisMacro[256],
       			ColorMacro[256],
       			DisplayNameMacro[256];

	}	STREETTEXTDATA;
typedef STREETTEXTDATA	FAR	*LPSTREETTEXTDATA;

typedef struct
	{	int		Len;
		int		BMWidth, BMHeight,BMBitCount;
		MNMXCORD	Bounds;
		char	Name[128];
	}	FILEINDEXENTRY;
typedef FILEINDEXENTRY	FAR		*LPFILEINDEXENTRY;

typedef struct
	{	int		Type;
		int		NumFiles;
		long	Length;
		int		CurFile;
		HANDLE	Handle;
		double	OrthoRes;
		LPFILEINDEXENTRY	CurrentEntry;
		char	FirstIndex;
	}	FILEINDEXver1;
typedef FILEINDEXver1	FAR *LPFILEINDEXver1; 

typedef struct
	{	BYTE	Type;
		BYTE	UsesTimes;
		USHORT	NumFiles;
		long	Length;
		double	OrthoRes; 
		MNMXCORD	Bounds;
		LPFILEINDEXENTRY	CurrentEntry;
		long	FirstIndexFileOffset,
				NextHeaderOffset,
				EndOffset,
				CurrentEntryOffset; 
		USHORT	FileInIndex; 
		BOOL	FirstFoundFile;
		char	FileName[128];
		char	FirstIndex;
	}	FILEINDEX;
typedef FILEINDEX	FAR *LPFILEINDEX; 

#define MAXINDEXLENGTH		32000       
#define STOREDINDEXLENGTH	2*sizeof(short)+sizeof(long)+sizeof(double)+sizeof(MNMXCORD)
typedef	struct
	{	
		short	ID;
		HANDLE	Handle;
		short	DisplayVP, TargetVP;
		short	Type;
		RECT	SavedRect;
		HBITMAP SavedScreen;
		POINT	BoxPoints[7];  
		long	CurAreaRef;
		char	CurAreaRefGlobal[34];  
		BOOL	IgnoreClear; 
		short	VPBoundsOpt; //1=keepcurrentbounds,other=usevislim
		char	BoundsGlobal[34];  
	} BOUNDSDISPLAY;
typedef BOUNDSDISPLAY FAR  *LPBOUNDSDISPLAY;

typedef struct
	{	HANDLE		Handle;
		unsigned	char	Type:2; 
		unsigned	char	Style:3;
		unsigned	char	Width:3;
		unsigned	char	R,G,B;
	}	NEWOBJECT_v0;
			
typedef struct
	{	HANDLE		Handle;  
		float		Width;
		unsigned	char	Type:2;   //if Type = 0 Width = text factor
		unsigned	char	Style:3; 
		unsigned	char	ProPen:3;
		unsigned	char	R,G,B;
	}	NEWOBJECT;
typedef NEWOBJECT FAR  *LPNEWOBJECT;
			
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
		LPTHEME		pThemes[MAX_VIEWPORT_THEMES]; /* Pointers to themes which this viewport
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
		short 		FileType[MAX_VIEWPORT_FILES];  /* 1 = Format (Viewport Coord),
													  2 = Standard (World Coord),
													  3 = Bitmap file,
													  4 = Plot file directory (World Coord)
													  5 = Ortho Photo directory              */
		LPSTR		lpFiles[MAX_VIEWPORT_FILES];   /* Files or file sets associated with viewport			 */
		char		FileID[MAX_VIEWPORT_FILES][16];/* File identifier displayed to user      */
		HANDLE		hlpIndex[MAX_VIEWPORT_FILES];   /* HANDLES to file indexes if type 4 or 5     */
/*		LPFILEINDEX	lpIndex[MAX_VIEWPORT_FILES];    */
		short 		NumVisList;
		LPVISLIST	pVisList1;		/* Points to first (top) visibility list in chain 		 */
		LPVISLIST	pVisListManual;	/* Points to manual vis list if in manual mode else NULL */ 
		LPVISLIST	CurVis;
		short 		NumPickList;
		LPVISLIST	pPickList1;		/* Points to first (top) pick list in chain 		 */
		LPVISLIST	pPickListManual;/* Points to manual pick list if in manual mode else NULL */ 
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
		int			ID,  
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
		UINT		ActivateMenuID; //no longer used
		COLORREF	BackGroundColor,
					BorderColor;
		BOOL		Shadow;
		float		BorderPct;      /* Width of vp border line                               */
		float		DesiredHeight,	/* Establishes aspect ratio and plot					 */
					DesiredWidth;	/* size. On screen or plotters smaller
								       than desired size, aspect ratio will
								       be preserved. Applies only to Viewport 1				.*/
	 	int			TagPointID;		/* 1 = lower left, 2 = upper left, 3 = upper right,
	 								   4 = lower right										 */
	 	int			TagPointType;	/* 1 = % of largest dimension, 2 = % of x dim if x, y 
	 								   dim if y												 */
	 	DPOINT		TagPoint;		/* if relative coords represent percent of parent 		 */
	 	POINT		TagPointActual;	/* actual Window coordinates							 */
	 	int			WidthType, 		/* 1 = fixed, 2 = relative								 */
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
		int			FileFactor;
		BOOL		HaveBounds;
		int			BoundsDisplayID;/* The Viewport ID in which this vps bounds are displayed*/
		LPBOUNDSDISPLAY		lpBoundsDisplay;
		BOOL        WindowIsZoomed,
					WindowZoomedToOrtho;
		int			wOrigX, wOrigY, wExtX, wExtY, vExtX, vExtY;
		LPTHEME		pTheme;
		int			NumThemes;
		LPTHEME		pThemes[MAX_VIEWPORT_THEMES]; /* Pointers to themes which this viewport
													 is a target of */
		int			BoundsDisplayVP;			  /* If this VP is used as a bounds display
													 VP for another VP this is the others VP id.*/
		BOOL		BoundsDisplayed;			  /* TRUE if bounds currently displayed in BoundsDisplayVP */
		int			PassID;			/* Current pass - 0 = Theme pass, 1 = Display pass; */
		BOOL		WantPass[2];
		int			NumFiles;
		int			CurFile; 
		int			SubFile;
		char		OrigFile[128];
		BOOL		FirstFile;
		int			FileType[MAX_VIEWPORT_FILES];  /* 1 = Format (Viewport Coord),
													  2 = Standard (World Coord),
													  3 = Editable file (World Coord)
													  4 = Plot file directory (World Coord)
													  5 = Ortho Photo directory              */
		LPSTR		lpFiles[MAX_VIEWPORT_FILES];   /* Files or file sets associated with viewport			 */
		char		FileID[MAX_VIEWPORT_FILES][16];/* File identifier displayed to user      */
		HANDLE		hlpIndex[MAX_VIEWPORT_FILES];   /* HANDLES to file indexes if type 4 or 5     */
/*		LPFILEINDEX	lpIndex[MAX_VIEWPORT_FILES];    */
		int			NumVisList;
		LPVISLIST	pVisList1;		/* Points to first (top) visibility list in chain 		 */
		LPVISLIST	pVisListManual;	/* Points to manual vis list if in manual mode else NULL */ 
		LPVISLIST	CurVis;
		int			NumPickList;
		LPVISLIST	pPickList1;		/* Points to first (top) pick list in chain 		 */
		LPVISLIST	pPickListManual;/* Points to manual pick list if in manual mode else NULL */ 
		char		VisName[128];
		char		PickName[128];
		int			FunctionStack[MAX_FUNCTION_STACK];
		int			LenFunctionStack;
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
		int			CurVisType[3201]; 
		HANDLE		hPenRedef;
		int			NumNewObjects;
		int			NewObjectMap[3201];
		NEWOBJECT_v0	NewObject[MAX_NEW_OBJECTS];    /* redefines display characteristics for this view*/
		char		PickMacroFile[128];
		BOOL		DisplayFunStack; 
		DPOINT		FunStackLoc;
		FARPROC lpfnFUNSTACKMsgProc; 
		HWND	FunStackWnd; 
		int			LinkedTo; 
		double		OrthoRes; 
		char		Space[1010];
        char		EndOfViewport;     
        
	}	VIEWPORT_v2;
typedef VIEWPORT_v2	FAR	*LPVIEWPORT_v2;    


typedef struct
	{
		short	Show; 
		long	Sequence; 
		PICKDATA	PD; 
	} HIGHLIGHTDATA;
	
typedef HIGHLIGHTDATA	FAR *LPHIGHLIGHTDATA;

typedef	struct
	{	short	TargetVP;
		char	StreetNameFile[256];
		char	StreetSegFile[256];
		char	SegmentAddressFile[256];
		char	AddressPlotFile[256];
	}	ADDRESSLOCATION;
typedef	ADDRESSLOCATION	FAR	*LPADDRESSLOCATION;

typedef	struct
	{	long	MinSeq,
				MaxSeq;
	}	PROFILETHEMEDATA;
typedef	PROFILETHEMEDATA	FAR	*LPPROFILETHEMEDATA;

typedef struct {DPOINT	Point;
				short	idesc;//neg symbols indicates elev comes from item, not surface
				float	SurfElev;  
				float	Size;
			   }PROFILESYMBOLS;
typedef PROFILESYMBOLS	FAR	*LPPROFILESYMBOLS;

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

typedef struct {
				unsigned	int		AutoEdit:1, 
									AutoMove:1,
									ExpandText:1,
									FixedSize:1,
									UseOrigSize:1;
				}IBFLAGS;

typedef struct
{	
	short		Version;  
	char		Desc[34];
    IBFLAGS		Flags;
	RECT		rect;
	DPOINT		TAGPoint;
	POINT		TAGPointScr;
	POINT		ConnectPoint;
	POINT		RestorePoint;
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
	HBITMAP		before, after;
	double		bmWidthD,bmHeightD;
	short		bmWidth, bmHeight; 
	BOOL		Shadow;
	LOGFONT		LogFont;
	short		DataFileType; 
	HANDLE		hTAGDB;
	short		SymNum;
	float		Factor;
	char		dummy1[6];
	long		Refno;
	HANDLE		hReport;
	char		Prefix[10],UDI[34];
	char		DataFile[128];
	char		SQL[256];
	char		text[864];
	MNMXCORD	BlowUpBounds;  
	char		PickMacroFile[128];
	
} TAGBOX;
typedef TAGBOX	FAR *LPTAGBOX;

typedef struct
{	MNMXCORD	FromBounds;
	MNMXCORD	ToBounds; 
	RECT		ToWinRect;
}	BLOWUPREC;


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
									unusedflags:5;
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
		LPTHEME		pThemes[MAX_VIEWPORT_THEMES]; /* Pointers to themes which this viewport
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
		char		FileID[MAX_VIEWPORT_FILES][16];/* File identifier displayed to user      */
		HANDLE		hlpIndex[MAX_VIEWPORT_FILES];   /* HANDLES to file indexes if type 4 or 5     */
/*		LPFILEINDEX	lpIndex[MAX_VIEWPORT_FILES];    */
		short		NumVisList;
		LPVISLIST	pVisList1;		/* Points to first (top) visibility list in chain 		 */
		LPVISLIST	pVisListManual;	/* Points to manual vis list if in manual mode else NULL */ 
		LPVISLIST	CurVis;
		short		NumPickList;
		LPVISLIST	pPickList1;		/* Points to first (top) pick list in chain 		 */
		LPVISLIST	pPickListManual;/* Points to manual pick list if in manual mode else NULL */ 
		char		VisName[128];
		char		PickName[128];
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
		char		FunctionFile[128]; /* File containing active function list */
		char		FunctionDir[128];   /* Current directory of functions */  
		char		DisplayRedefFile[128];
		HANDLE		hReport,
					hTAGList;
		BYTE		ShrinkToFit,FitToWindow;
		char		Prefix[10],
					UDI[34];
		long		ReportRefno;
		double		ReportFactor;
		BYTE		CurVisType[3201];  
		COLORREF	DarkContourColor[MAX_VIEWPORT_FILES];  
		COLORREF	ContourTextColor[MAX_VIEWPORT_FILES];    
		long		MaxFileDisplayedPointWidth[MAX_VIEWPORT_FILES]; 
		double		MetersPerPixel;
		double		MetersPerDegreeX;
		char		space3[3201-800-70-4*MAX_VIEWPORT_FILES-4*MAX_VIEWPORT_FILES-4*MAX_VIEWPORT_FILES-2*8]; 
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
		short		NewObjectMap[3201];
		NEWOBJECT	NewObject[MAX_NEW_OBJECTS];    /* redefines display characteristics for this view*/
		char		PickMacroFile[128];
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
		short	EditRectInfoSize;
		short	EditRectSize;  
		USHORT	MaxEditRect;
		USHORT	NumEditRects;
		char	OrthoDisplayName[32]; 
		MNMXCORD	PriorBounds; 
		USHORT	NumMaskPoints;
		HANDLE	LinkedCursorHandle;
		short	RestoreFile;
		HANDLE	hFileTransIn, hFileTransOut;
		long	BoundsDisplayCycle; 
		long	BitmapID;
		BOOL	Transparent;   
		HANDLE	ToolbarHandle; 
		short	CurrentIcon;
		RECT	CurrentIconRect; 
		char	RButFunction[128];
		short	OnPrintAddSpaceToVP; 
		HANDLE	hProfileRoute;
		long	nProfileRoute;
		HANDLE	hProfileElev;
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
		short	NumMaskAreaParts;   
		double	Rotation,
				Scale;
		DPOINT	MidPointW;  
		HANDLE	hProfileDataRectangles;
		short	nProfileDataRectangles;
		short	ProfileDistUnits;  
        char		EndOfViewport;     
        
	}	VIEWPORT;
typedef VIEWPORT	FAR	*LPVIEWPORT;    

typedef struct	{
			 long	TLID;
			 long	street_num;
			 long	faddl, faddr, taddl, taddr;
			 long	fframe, tframe;
			 long	frlong, frlat, tolong, tolat;
			 char	CTBNAL[6], CTBNAR[6];
			 short	BGL, BGR;
			 long	FMCDL, FMCDR;
			 short	DeleteFlag, ChangedFlag;
			 char	CFCC[3];
			 char	vdir;
			 char	Run[8];
			 short	FromFrameCycle;
			 short	DINumerator;
			 long	LoopFrame; 
			 char	CycleID,
			 		Spacer;
			 long	MALeft,
			 		MARight;
			 long	ZIPLeft,
			 		ZIPRight;
		 }	SEGDATA;  
		 
typedef	SEGDATA	FAR *LPSEGDATA;

typedef struct {
	char	Userid[6];
	char	svrName[32];
	char	HomeDir[128];
	int		MaxPriority;
	}	UserInfo;

typedef	struct
	{ long	TLID,
			StreetNum[4],
			faddl, faddr, taddl, taddr,
			ZIPL,
			ZIPR;
	  int	
			STATEL,
			STATER,
			COUNTYL,
			COUNTYR;
	  long
			FPLL,
			FPLR;
	  char
			CTBNAL[6],
			CTBNAR[6],
			BLKL[4],
			BLKR[4];
	  long
			FMCDL, FMCDR;
	  long	TrafVol; 
	  long	FromTLID;
	  short Lanes;
	  time_t	UpdateTime;
	  short	UpdateID;
	  short	ChangedFlag;
	  float	Width;
	  short	Speed,
	  		OneWay;
	  char	CFCC[3]; 
	  char	Filler2;
	  long	LowAddrL, LowAddrR;
	  long	HighAddrL, HighAddrR; 
	  short	LowOffL, LowOffR;
	  short	HighOffL, HighOffR;
	} SEGDATAGM; 
typedef	SEGDATAGM	FAR	*LPSEGDATAGM;	
	
typedef struct	{
			 long	TLID;
			 long	street_num;
			 long	faddl, faddr, taddl, taddr;
			 long	fframe, tframe;
			 long	frlong, frlat, tolong, tolat;
			 char	CTBNAL[6], CTBNAR[6];
			 int	BGL, BGR;
			 long	FMCDL, FMCDR;
			 int	DeleteFlag, ChangedFlag;
			 char	CFCC[3];
			 char	vdir;
			 char	Run[8];
			 int	FromFrameCycle;
			 int	DINumerator;
			 long	LoopFrame; 
			 char	CycleID,
			 		Spacer;
			 long	MALeft,
			 		MARight;
			 long	ZIPLeft,
			 		ZIPRight;
		 }	SEGDATAGS;  
typedef	SEGDATAGS	FAR	*LPSEGDATAGS;	

typedef struct {
		long	TLID;
   		long	House;
 	  } POINTADDKEY; 
typedef POINTADDKEY	FAR	*LPPOINTADDKEY;

typedef struct
	{
		char	Name[62];
		short	id;
	}VARNAMEINDEXITEM;
typedef	VARNAMEINDEXITEM	FAR	*LPVARNAMEINDEXITEM;

typedef struct
	{   short	Type,
				NumFiles,
				NumRows, 
				NumTabs, 
				NumFonts, 
				Height,
				Width,
				Just; 
		BOOL	FilesAreOpen, First;
		HDC		hDC;
		short	x,y;
		float	Margin;
		short	TabLen[MAXREPORTTABS];
		short	FileTypes[MAXREPORTFILES]; 
		RECT	Rect;
		HANDLE	FileHandles[MAXREPORTFILES];
		HANDLE	hRows;
		HANDLE	hFiles;   
		HANDLE	hScrollLine;
		HFONT	hFonts[MAXREPORTFONTS]; 
		COLORREF	FontColor[MAXREPORTFONTS];        
		short	FontSize[MAXREPORTFONTS],
				FontWeight[MAXREPORTFONTS],
				FontItalic[MAXREPORTFONTS],
				FontUnderline[MAXREPORTFONTS];
		char	FontName[MAXREPORTFONTS][LF_FACESIZE]; 
		BOOL	WantSize;
		RECT	SizeRect;
	} REPORT;
typedef REPORT	FAR *LPREPORT;

typedef struct
    {
        short     Num, 
                Len,
                offset[MAXWHEREINCOMBO];
    }   WHEREINDEX;
typedef WHEREINDEX  FAR *LPWHEREINDEX;

typedef struct
    {
        short     Num, 
                Len,
                offset[MAXCFIELDINCOMBO];
    }   CFIELDINDEX;
typedef CFIELDINDEX FAR *LPCFIELDINDEX;

typedef struct
    {
        HANDLE  LastPathHandle;
        short     NumFiles;
        HANDLE  FileHandle;
    }   FILEPATH;
typedef FILEPATH    FAR *LPFILEPATH;


typedef struct
    { short  fromfile,
             fromfileindex,
             WhereID;
    } COMBOFIELDINFO;
typedef COMBOFIELDINFO FAR  *LPCOMBOFIELDINFO;
typedef struct {short     Beg;
                short     Len;
                short     Type;
                char	  Name[33];
				BYTE	  HasValue:1, 
						  filler:7;
                } GWFLDINFO;
typedef GWFLDINFO FAR  *LPGWFLDINFO;
typedef struct {
				unsigned short
                		Version:13,
						Compressed:1, 
						NonUniqueSortedBinary:1,
						Unused:1;
                short
                        NumFields,
                        NumIndex,
                        Fid;
                long    Reclen;
                HANDLE  BTHandle[MAX_GMD_INDEXES];
                short   NumIndexFields[MAX_GMD_INDEXES], //neg indicates sparse index
                        IndexFields[MAX_GMD_INDEXES][MAX_GMD_INDEX_FIELDS],
                        lKeys[MAX_GMD_INDEXES]; 
                HANDLE  hKeys[MAX_GMD_INDEXES];
                LPSTR   pKeys[MAX_GMD_INDEXES];
                time_t  TimeStamp;
                HANDLE  hFldInfo;                       
                LPGWFLDINFO pFldInfo;
                char    GWDData[2];
                }   GWDHEADER;
typedef GWDHEADER FAR  *LPGWDHEADER;   

typedef struct {
                short   Version,
                        NumFiles,
                        NumFields, 
                        TotFileLen;  
                HANDLE  hSQL[5];
                char    FileNames[5][128];
                FIELDINFO   FldInfo;     
                }   COMBOFILE;
typedef COMBOFILE   FAR *LPCOMBOFILE;

typedef struct {
				HANDLE	hComboFields, hWhere, hComputedFields, hComboFile;
				} COMBOHEADER;
typedef COMBOHEADER	FAR	*LPCOMBOHEADER;

typedef struct {USHORT	ControlID; 
				short	Type;
				HWND	hWnd;
				short	DefLoc,
						DefaultLoc, 
						InitialLoc,
						ValidLoc,
						ValidMsgLoc;
				BYTE	ValueChanged;} CONTROLDEF;
typedef	CONTROLDEF	FAR	*LPCONTROLDEF;

typedef	struct {long	ID;
				char	Prefix[8], UDI[32], SymName[32];
				} TRAVIDDATA;
typedef TRAVIDDATA	FAR	*LPTRAVIDDATA;
typedef	struct {
				short	Type;
				DPOINT	Points[3];
				double	Dist;
				} LEGDATA;
typedef LEGDATA	FAR	*LPLEGDATA; 
typedef	struct {long	ID;
				short	LegNum;
				short	Type;
				long	SnapRef;
				short	SnapPos;
				char	Parameters[250];
				} TRAVLEGDATA;
typedef TRAVLEGDATA	FAR	*LPTRAVLEGDATA; 

typedef	struct
	{ char	RT,
			VERSION[4],
			TLID[10],
			ONESIDE,
			SOURCE,
			FEDIRP[2],
			FENAME[30],
			FETYPE[4],
			FEDIRS[2],
			CFCC[3],
			FRADDL[11],
			TOADDL[11],
			FRADDR[11],
			TOADDR[11],
			FRIADDL,
			TOIADDL,
			FRIADDR,
			TOIADDR,
			ZIPL[5],
			ZIPR[5],
			FAIRL[5],
			FAIRR[5],
			ANRCL[2],
			ANRCR[2],
			STATEL[2],
			STATER[2],
			COUNTYL[3],
			COUNTYR[3],
			FMCDL[5],
			FMCDR[5],
			FSMCDL[5],
			FSMCDR[5],
			FPLL[5],
			FPLR[5],
			CTBNAL[6],
			CTBNAR[6],
			BLKL[4],
			BLKR[4],
			FRLONG[10],
			FRLAT[9],
			TOLONG[10],
			TOLAT[9];
	  char	CRLF[2];
	} TIGER1;
typedef TIGER1 FAR  *LPTIGER1;     

typedef	struct
	{ char	RT,
			VERSION[4],
			TLID[10],
			RTSQ[3],
			FEAT[5][8],
	  		CRLF[2];
	} TIGER4;
typedef TIGER4 FAR  *LPTIGER4;     

typedef	struct
	{ char	RT, 
			VER[4],
			STATE[2],
			COUNTY[3],
			FEAT[8], 
			FEDIRP[2],
			FENAME[30],
			FETYPE[4],
			FEDIRS[2],
	  		CRLF[2];
	} TIGER5;
typedef TIGER5 FAR  *LPTIGER5;     

typedef	struct
	{ long	TLID,
			StreetNum,
			ZIPL,
			ZIPR;
	  int	
			STATEL,
			STATER,
			COUNTYL,
			COUNTYR;
	  long
			FPLL,
			FPLR;
	  char
			CTBNAL[6],
			CTBNAR[6],
			BLKL[4],
			BLKR[4];
	  long
			FRLONG,
			FRLAT,
			TOLONG,
			TOLAT;
	} TIGER1_PEOPLENET;
typedef TIGER1_PEOPLENET FAR  *LPTIGER1_PEOPLENET;     

typedef	struct
	{ long	TLID,
			StreetNum[4],
			faddl, faddr, taddl, taddr,
			ZIPL,
			ZIPR;
	  int	
			STATEL,
			STATER,
			COUNTYL,
			COUNTYR;
	  long
			FPLL,
			FPLR;
	  char
			CTBNAL[6],
			CTBNAR[6],
			BLKL[4],
			BLKR[4];
	  long
			FMCDL, FMCDR,
			FRLONG,FRLAT,
			TOLONG,TOLAT;
	  short	ChangedFlag;
	  float	Width;
	  short	Speed,
	  		OneWay;
	  char	CFCC[3];
	} TIGER1_MN;
typedef TIGER1_MN FAR  *LPTIGER1_MN;     

typedef struct	{
			 long	TLID;
			 long	street_num;
			 long	faddl, faddr, taddl, taddr;
			 long	fframe, tframe;
			 long	frlong, frlat, tolong, tolat;
			 char	CTBNAL[6], CTBNAR[6];
			 int	BGL, BGR;
			 long	FMCDL, FMCDR;
			 int	DeleteFlag, ChangedFlag;
			 char	CFCC[3];
			 char	vdir;
			 char	Run[8];
			 int	FromFrameCycle;
			 int	DINumerator;
			 long	LoopFrame; 
			 char	CycleID,
			 		Spacer;
			 long	MALeft,
			 		MARight;
			 long	ZIPLeft,
			 		ZIPRight;
		 }	TIGER1_GEOSPAN;  
typedef TIGER1_GEOSPAN FAR  *LPTIGER1_GEOSPAN;     

typedef struct
	{ char	Longitude[10],
			Latitude[9];
	}LNGLAT;	
typedef	struct
	{ char	RT,
			VERSION[4],
			TLID[10],  
			RTSQ[3];
	  LNGLAT	LONGLAT[10]; 
	  char	CRLF[2];
	} TIGER2;
typedef TIGER2 FAR  *LPTIGER2;   

typedef	struct
	{
		long	FileCode,
				Space1[5],
				FileLength,
				Version,
				ShapeType;
	  double	Xmin,
				Ymin,
				Xmax,
				Ymax, 
				Zmin,
				Zmax,
				Mmin,
				Mmax;
	} SHPHEADER;
typedef SHPHEADER FAR  *LPSHPHEADER;   

typedef struct
	 {
		long	RecNum,
				Length;
	} SHPRECHEADER;
typedef SHPRECHEADER FAR  *LPSHPRECHEADER;   

typedef struct
	 {
	  long		Type;
	  double	Xmin,
				Ymin,
				Xmax,
				Ymax;   
	  long		NumParts,
	  			NumPoints;
	} SHPPOLYHEADER;
typedef SHPPOLYHEADER FAR  *LPSHPPOLYHEADER;   

typedef struct
	 {
	  long		Type;
	  DPOINT	Point;
	} SHPPOINTREC;
typedef SHPPOINTREC FAR  *LPSHPPOINTREC;   

typedef struct
	 {
	  long		Type;
	  double	Xmin,
				Ymin,
				Xmax,
				Ymax;   
	  long		NumPoints;
	} SHPMULTIPOINTHEADER;
typedef SHPMULTIPOINTHEADER FAR  *LPMULTIPOINTHEADER;   

typedef struct
	{	
		int		State;
		long	FIPS;
	}	CITIESKEY1;
typedef struct
	{	
		DPOINT	LatLong; 
		MNMXCORD MinMax;
		long	Pop;  
		char	Name[64];
	}	CITIESDATA1;
typedef struct
	{	
		long	Pop;
		int		State;
		long	FIPS;
	}	CITIESKEY2; 
	
typedef struct
	{	
		char	Name[62];
		int		State;
	}	CITIESKEY3;  
typedef struct
	{	
		DPOINT	LatLong; 
		long	Pop; 
		char	State[4]; 
		char	Name[64];
	}	CITIESDATA4;   
typedef	CITIESDATA4	FAR	*LPCITIESDATA4;
	
typedef	struct
	{ HANDLE	hBT;
	  HFILE		hData;
	} TIGERHANDLES;
typedef TIGERHANDLES FAR  *LPTIGERHANDLES;    

typedef struct{
				unsigned	int	lFAddL:4,
								lTAddL:4,
								lFAddR:4,
								lTAddR:4;
			}ADDRESSLENGTHS;
typedef ADDRESSLENGTHS FAR *LPADDRESSLENGTHS;


              
#if CHECKMEM        
#define PostMessage GSSiPOSTMESSAGE 
BOOL    WINAPI GSSiPOSTMESSAGE(HWND, UINT, WPARAM, LPARAM);
#define	GlobalLock	GSSiGLOBALLOCK
void LogMemAlloc (unsigned short MemID,long MemLen);
LPVOID GSSiGLOBALLOCK (HANDLE hglb);
#define	GlobalSize	GSSiGLOBALSIZE
DWORD GSSiGLOBALSIZE (HANDLE hglb);
#define	GlobalUnlock	GSSiGLOBALUNLOCK
BOOL GSSiGLOBALUNLOCK (HANDLE hglb);
#define	GlobalAlloc	GSSiGLOBALALLOC
HGLOBAL GSSiGLOBALALLOC(UINT fuAlloc, DWORD cbAlloc);
#define	GlobalFree	GSSiGLOBALFREE
HGLOBAL GSSiGLOBALFREE (HANDLE hglb);
#define GlobalReAlloc GSSiGLOBALREALLOC  
HGLOBAL WINAPI GSSiGLOBALREALLOC (HGLOBAL hglb, DWORD cbAlloc, UINT fuAlloc);  
#define CreatePen GSSiCREATEPEN  
HPEN    WINAPI GSSiCREATEPEN (int style, int width, COLORREF color);
#define CreateSolidBrush GSSiCREATESOLIDBRUSH
HBRUSH  WINAPI GSSiCREATESOLIDBRUSH(COLORREF color);
#define CreateHatchBrush GSSiCREATEHATCHBRUSH
HBRUSH  WINAPI GSSiCREATEHATCHBRUSH(int style, COLORREF color);  
#define CreatePatternBrush GSSiCREATEPATTERNBRUSH
HBRUSH  WINAPI GSSiCREATEPATTERNBRUSH(HBITMAP bitmap); 
#define CreateBrushIndirect GSSiCREATEBRUSHINDIRECT
HBRUSH  WINAPI GSSiCREATEBRUSHINDIRECT(LOGBRUSH FAR* logbrush);
#define LoadBitmap GSSiLOADBITMAP
HBITMAP WINAPI LoadBitmap(HINSTANCE hInst, LPCSTR Name);  
#define CreateFont GSSiCREATEFONT
HFONT   WINAPI GSSiCREATEFONT(int i1, int i2, int i3, int i4, int i5, BYTE b1, BYTE b2, BYTE b3, BYTE b4, BYTE b5, BYTE b6, BYTE b7, BYTE b8, LPCSTR name);
#define CreateFontIndirect GSSiCREATEFONTINDIRECT
HFONT   WINAPI GSSiCREATEFONTINDIRECT (const LOGFONT FAR* lf);
#define CreateRectRgn GSSiCREATERECTRGN
HRGN    WINAPI GSSiCREATERECTRGN (int i1, int i2, int i3, int i4);
#define CreateRectRgnIndirect GSSiCREATERECTRGNINDIRECT
HRGN    WINAPI GSSiCREATERECTRGNINDIRECT (const RECT FAR* pr);
#define CreatePolygonRgn GSSiCREATEPOLYGONRGN
HRGN    WINAPI GSSiCREATEPOLYGONRGN (const POINT FAR* pp, int i1, int i2);
#define CreateBitmap GSSiCREATEBITMAP
HBITMAP WINAPI GSSiCREATEBITMAP (int i1, int i2, UINT i3, UINT i4, const void FAR* p);
#define CreateCompatibleBitmap GSSiCREATECOMPATIBLEBITMAP
HBITMAP WINAPI GSSiCREATECOMPATIBLEBITMAP (HDC hDC, int i1, int i2);
#define CreateDIBitmap GSSiCREATEDIBITMAP
HBITMAP WINAPI GSSiCREATEDIBITMAP(HDC hDC, BITMAPINFOHEADER FAR* pbi, DWORD i1, const void FAR* p, BITMAPINFO FAR* pb, UINT i2);
#define CreatePalette GSSiCREATEPALETTE
HPALETTE WINAPI GSSiCREATEPALETTE(const LOGPALETTE FAR*);
 
void GSSiRemoveMem (HGLOBAL hglb);  

#define DeleteObject GSSiDELETEOBJCT   
BOOL GSSiDELETEOBJCT (HGDIOBJ hobj);
#define SelectObject GSSiSELECTOBJECT  
HGDIOBJ GSSiSELECTOBJECT (HDC hdc,HGDIOBJ hobj);
#endif

void GSSiGLOBALLOCCLOSE (void);

