#include "gmlimits.h"
#include <stdlib.h>  
#include <time.h>    
#include <commdlg.h>
#include <ctype.h>      
#if WIN32
#define HUGE 
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
#define _fstrcspn strcspn
#define _fstrncat strncat
#define _fstrspn strspn
#define _fstrpbrk strpbrk
#define _fmemccpy memccpy
#define _fmemmove memmove
#define hmemmove memmove
#define _fmemcpy memcpy
#define _fstrset strset
#define _fmemicmp memicmp 
#define _fmemcmp memcmp
#define hmemcpy memcpy
#define _fmemset memset

#define HANDLE16	unsigned short
#define HFILE16		HANDLE16   
#define HPEN16		HANDLE16
#define HBRUSH16	HANDLE16    
#define BOOL16		short
#define HCURSOR16	HANDLE16
#define HWND16		HANDLE16
#define HDC16		HANDLE16
#define HRGN16		HANDLE16
#define HBITMAP16	HANDLE16

typedef RGBTRIPLE	FAR *LPRGBTRIPLE;
typedef HFILE		FAR *LPHFILE;    
typedef struct
{
    short left;
    short top;
    short right;
    short bottom;
} RECT16;
typedef RECT16	FAR *LPRECT16;
#else
#define HANDLE16	HANDLE
#define HFILE16		HFILE
#define HPEN16		HPEN
#define HBRUSH16	HBRUSH      
#define POINT16		POINT
#define RECT16		RECT
#endif 

#define WM_F1DOWN	    0x0500  
#define	INDCOD	31100 
#define	NULL_ELV	-100.0    

#define CHECKTIMESTAMP	4
#define CHECKFOREXIST	3
#define DONTEXIST		2
#define	DOESEXIST		1

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
#define SQL_UNKCHAR				(-9)
#define SQL_TYPE_DRIVER_START	(-80)       
#define SQL_MSSHAPE				(-151)

#define	TYPE_AREA		0
#define TYPE_POLYLINE	1
#define	TYPE_CONTOUR	3  
#define TYPE_TEXT		4
#define TYPE_SYMBOL		6
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

#define VPFILETYPE_FORMAT	1
#define VPFILETYPE_SINGLEPLOT	2
#define VPFILETYPE_IMAGE	3
#define VPFILETYPE_PLOTDIR	4
#define VPFILETYPE_ORTHODIR	5
#define VPFILETYPE_HLTLIST	6
#define VPFILETYPE_MACRO	7
#define VPFILETYPE_SUBVP	8
#define VPFILETYPE_DTM	9
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
#define HFILE_GMD	-3
#define HFILE_GPX	-4
#define HFILE_FGDB	-5
#define HFILE_KML	-6

#define SYMTYPEPARENT	0
#define SYMTYPEPOINT	1
#define SYMTYPELINE		2    
#define SYMTYPEAREA		3

#define SVPRESYM	-1              
#define SVPOSTSYM	-3 
#define SVEND		-2             
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
#define TCPREPLAYTIMER	99
#define USERTIMER	100    
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
#define MT_GMD	10
#define MT_HIGHLIGHTLIST	11
#define MT_HGF	12
#define MT_GPX	13
#define MT_FILE_GEO_DB	14 
#define MT_KML	15



#define ORAT_NULL	0
#define ORAT_POINT	1
#define ORAT_ARC	3  
#define ORAT_TEXT	4
#define ORAT_POLYGON	5
#define ORAT_MULTIPOINT	8
#define ORAT_MIXED	10 

#define	DEST_UPDATE	1
#define	DEST_TEST	2
#define DEST_WORKING	3

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
#define		ORA_DATAFILE		15
#define		HLTLIST_DATAFILE	16
#define		THEME_HLTFILE		17
#define		IMAGE_DATAFILE		18
#define		LISTVAR_DATAFILE	19
#define		FGDB_DATAFILE		20
#define		POLY_DATAFILE		21
#define		PN_DATAFILE			22

#define	FT_POLYGON	1
#define FT_CIRCLE	2
#define FT_ROUTE	3

#define DTM_RENDER_RAW_POINTS		1                   
#define DTM_RENDER_GRID_POINTS		2
#define DTM_RENDER_SLOPE_VECTORS	3                   
#define DTM_RENDER_SLOPE_POLYGONS	4                   
#define DTM_RENDER_CONTOURS			5                  

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
typedef POINTS				*HPPOINTS;
typedef struct{double x,y,z;} DPOINT3D;
typedef DPOINT3D  			FAR *LPDPOINT3D;
typedef DPOINT3D  			HUGE *HPDPOINT3D;
typedef DPOINT				FPOINT;
typedef FPOINT  			FAR *LPFPOINT;
typedef FPOINT  			HUGE *HPFPOINT;
typedef struct{float x,y;}  FLTPOINT;
typedef FLTPOINT			*LPFLTPOINT;
typedef POINT				HUGE *HPPOINT;
typedef void				HUGE* LPHUGE;
typedef struct{long x,y;} LPOINT;
typedef LPOINT  			FAR *LPLPOINT;         
typedef LPOINT  			HUGE *HPLPOINT;         
typedef HANDLE				HDIB32; 
typedef HDIB32				FAR *LPHDIB32; 
typedef RECT				HUGE *HPRECT;

#pragma pack(2)
typedef struct 
{
    short      lfHeight;
    short      lfWidth;
    short      lfEscapement;
    short      lfOrientation;
    short      lfWeight;
    BYTE      lfItalic;
    BYTE      lfUnderline;
    BYTE      lfStrikeOut;
    BYTE      lfCharSet;
    BYTE      lfOutPrecision;
    BYTE      lfClipPrecision;
    BYTE      lfQuality;
    BYTE      lfPitchAndFamily;
    CHAR      lfFaceName[LF_FACESIZE];
} LOGFONT16,  FAR *LPLOGFONT16;
#pragma pack()

typedef struct
{
	short x;
	short y;
}  svec2;
typedef struct
{
	short x;
	short y;
	short z;
}  svec3;

typedef struct
{
	int x;
	int y;
}  ivec2;
typedef struct
{
	int x;
	int y;
	int z;
}  ivec3;

typedef struct
{
	float x;
	float y;
}  vec2;
typedef struct
{
	double x;
	double y;
}  dvec2;
typedef struct
{
	float x;
	float y;
	float z;
}  vec3;

typedef struct
{
	double x;
	double y;
	double z;
}  dvec3;

typedef struct
{
	float x;
	float y;
	float z;
	float w;
}  vec4;

struct PlaneVec2 {
	vec2 point;
	vec2 normal;
};

struct PlaneVec3 {
	vec3 point;
	vec3 normal;
};

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
				 double		Factor;
				 double		WOffset;
				 short		HaveInPoints;
				 short		Width,
				 			Height;
				 double		AveX,
				 			AveY; 
				 short		Offset;
				 short		Type;//0 if world 1 if window
				 short		Compression;
				 short		unused;
				 int		arraylen;
				 BYTE		barray[];
				} PIAAStruct;
typedef PIAAStruct	HUGE	*LPPIAAStruct;

typedef struct {
				 MNMXCORD	Bounds;
				 double		Factor;
				 short		Width,
				 			Height,
							Offset;
				 double		WOffset[2];
				 short		Type;//0 if world 1 if window
				 short		unused;
				 HBITMAP	hBitMap[2];
				} PCTIAStruct;
typedef PCTIAStruct	*LPPCTIAStruct;

typedef struct
{
    long left;
    long top;
    long right;
    long bottom;
} RECT32;
typedef RECT32 FAR*  LPRECT32;

typedef struct
     {
        MNMXCORD	Bounds, TriBounds; 
        double  A1, A2, B1, B2, C1, C2, BASX, BASY;
        HANDLE  TriHandle;
        BOOL    FIXEDP, ONE_SCALE;
        short   NSETPT; 
        short	LastTri;
		double	FitPointAZFrom, FitPointAZTo;
     } TRANDATA;
typedef TRANDATA    FAR *LPTRANDATA;

typedef struct	{
				 MNMXCORD FromMNMX, ToMNMX; 
				 DPOINT FromPT[4];
				 DPOINT	ToPT[4];
				 HANDLE	hFromTran, hToTran;
				 short	NumTri;
				 char	spacer[58];//aligns on 256 for huge ptr
				}TRANTRI; 
typedef	TRANTRI	HUGE	*HPTRANTRI;
    
typedef struct	{char Word[8];
				 short Seq;
				 short nOffsets;
				 long Offsets[MAXWIOFFSETS];} WORDINDEX;
typedef WORDINDEX	FAR	*LPWORDINDEX;

typedef	struct	{
					short	PenNum, Style;
					float	Width;
					long	Color; 
				}	PENDESC;
typedef	PENDESC	FAR	*LPPENDESC; 

typedef struct {
		unsigned short	Transparent	:1;
		unsigned short	BGOpt		:1;
		unsigned short	Pattern		:6;
		} PATBYTE; 
typedef PATBYTE	FAR	*LPPATBYTE;

typedef struct {
		unsigned short	r	:5;
		unsigned short	g	:5;
		unsigned short	b	:5;
		unsigned short	x	:1;
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

typedef struct {int   length; char Value;} CURVAL;
typedef CURVAL  FAR *LPCURVAL;

typedef	struct	{long	Refno;
				 float	PCT;
				 } STREETMP;
typedef STREETMP	FAR	*LPSTREETMP;   
#pragma pack(2)
typedef struct
    { short  type,
             index,
             radix,
             scale;
      long   length,
             precision;       
      char   name[62]; 
      HANDLE16    hCurVal;
    } FIELDINFO16;
typedef FIELDINFO16 FAR  *LPFIELDINFO16;
#pragma pack()

typedef struct
	{   
		short	NumFields;  
		HANDLE16	DBHandle;
		char	DBName[128];
		char	Select[8192]; 
		char	From[1024];
		char	Where[1024]; 
		FIELDINFO16	FldInfo[1];
	}SQLDATABASE16;
typedef	SQLDATABASE16 FAR	*LPSQLDATABASE16;

#pragma pack(2)
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
#pragma pack()

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
		short	NumFields;
		int		NumRows;
		int		Pos;
		int		CurRowID;
		char	CurRow[256];
		char	Value[4096]; 
		FIELDINFO	FldInfo[1];
	}LISTVARDATABASE;
typedef	LISTVARDATABASE FAR	*LPLISTVARDATABASE;

typedef struct
    {   
        HANDLE  myhandle,
                FileHandle,
                BufferHandle; 
        short   Type,
                NumSQLs,
                NumFields; 
        long    FirstLineOffset;
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
        short   OpenFileID;
		short	Access;
        DWORD   lastreadtime;  
        short   st;
        long    Offset,
        		MacroID;
        LPVOID  hstmt;
        char    IDName[34];
		char	myhandleC[16];
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
				HWND	hWnd;
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
        short   Type;
        BYTE	ValueIsHandle;
        short	NumLinkedVars;  
        DWORD   changetime; 
        BOOL	ContainsGorF;
        BOOL	Save;
        char    Name[34];
        char    Value[MAXVARLEN]; 
        HANDLE	LinkedVar[1];
    } VARINFO;
typedef VARINFO FAR *VARPNT;     

typedef	struct
	{	LONG	Segment;
		WORD	Offset, Element;
		LONG	Refno;   
		short	ViewID,
				ConfigID;
		short	FileNum;
		USHORT	FileInIndex;
		short	SubFile;
		unsigned	short	Type:8, 
						HasText:1,
						HasTextPointer:1, 
						HiPrecis:1, 
						Blocked:1,
						IsDispersed:1,
						IsDeleted:1,
						filler2:2;
		short	Desc,
		 		PolyID;
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
		char	Prefix[33], UDI[65];
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
		short	Desc,
		 		PolyID;
	} PICKDATAHEADER;   
typedef	PICKDATAHEADER	FAR	*LPPICKDATAHEADER;

typedef struct	{short	Number;
			 HANDLE	Handle;} SYMDESC;
typedef SYMDESC	FAR	*LPSYMDESC; 

#pragma pack(1)
typedef struct {
				unsigned int	Type:4,
								InVisible:1, 
								SolidLine:1,
								WidthIsMeters:1,
								Width:9,
								DisplayPos:3,
								Layered:1,
								Reversed:1,
								Shadow:1;
				COLORREF		Color;  
				RECT16	Rect;
				} SYMBOLATTRIBUTE;
typedef SYMBOLATTRIBUTE FAR *LPSYMBOLATTRIBUTE;
#pragma pack()
		
typedef	struct
	{	short	SymNum,	Parent, Type;
	} CHILDLIST;
typedef CHILDLIST FAR *LPCHILDLIST;

typedef struct {
		double	Dist,
				AZM;
		short	Type;						// 0=begin/end, 1=poc, 2=(x:y)
		} VECTOR;
typedef VECTOR FAR *LPVECTOR;


#pragma pack(2)
typedef struct {
		short	Number,
				Parent;
		char	Name[34];
		char	Desc[61]; 
		char	InVisible;
		float	BaseSize;
		unsigned	short	Type:3,    		// 0=parent, 1=point, 2=line, 3=area      
						DisplayPos:3,		// 1,2 = after area pass, 3,4=after line pass, 5,6=after text pass
						BlockRotation:1,	//block rot for points - reverse for lines
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
		short		Width; //if type is border or XHatch abs (width) is embedded symbol num if negative 
		short		Type; 					// 1=embedded text, 2=polyline, 3=polygon, 4=border, 5=XHatch, -1=begin sym, -2=varline, -3=endsymbol
		unsigned short		Style:4,
		 			LineColorType:2,
					FillColorType:2, 
					Squared:1,
					Reverse:1,
					Shadow:1,
					Dummy:5;
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
typedef struct	{   
					long	SecondsRepresented;
					float	Radius;
					unsigned	short	DecayOpt:4, 
									ColorOpt:6,
									PassThrough:2,
									PromptForSavex:2,
									filler:2;
					short	GridWidth,
							GridHeight,
							MaskWidth;
					long	TotalIncidents,
							MaxGridValue;
					HANDLE16	hMask,
							hGrid,
							hTranBaseToHotSpot; 
				}HOTSPOTDATA16;
typedef HOTSPOTDATA16	FAR	*LPHOTSPOTDATA16;
#pragma pack()


typedef struct {
				char	Name[64],
						Command[32],
						SymName[34],
						SubName[34];
						short	SymNum, Type;
						float	Size,Width;
						COLORREF	Color;
				} LAYERDESC;
typedef LAYERDESC	FAR	*LPLAYERDESC;
typedef struct {
				long	WayPointID,
						LakeID;  
				DPOINT	LatLong;
				short	Type,Year,Month,Day,
						FromTime, ToTime,
						Depth, WTemp, ATemp,
						Weather, WindSpeed, WindDir,
						BarState, BarValue;
				char	Comment[82]; 
				char	WPName[16];
				} WAYPOINTDATA;
typedef WAYPOINTDATA	FAR	*LPWAYPOINTDATA;

typedef struct {
				long	Refno;
				char	ID[8];
				char	Symbol[8];
				char	WPName[16];
				} WAYPOINTGPSDATA;
typedef WAYPOINTGPSDATA	FAR	*LPWAYPOINTGPSDATA;

typedef struct {
				long	WayPointID;
				short	FishID,
						Type, Num, Size, Tackle, LiveBait;
				char	Comment[30];
				short	Length;
				} FISHDATA;
typedef FISHDATA	FAR	*LPFISHDATA;  

typedef struct {long	ID;
		short	Depth;
		DPOINT	LatLong;  
		char	WPName[16];
		char	Text[256];} FOUNDWPDATA; 
typedef FOUNDWPDATA	FAR	*LPFOUNDWPDATA;

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
					char	Filler[38];
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
					unsigned	short	DecayOpt:4, 
									ColorOpt:6,
									PassThrough:2,
									PromptForSavex:2,
									Granularity:2;
					short	GridWidth,
							GridHeight,
							MaskWidth;
					long	TotalIncidents,
							MaxGridValue;
					HANDLE	hMask,
							hGrid,
							hTranBaseToHotSpot; 
				}HOTSPOTDATA;
typedef HOTSPOTDATA	FAR	*LPHOTSPOTDATA;

typedef struct {
        			long	Refno;
        			long	NumPoints;
        			long	NumPointsInArea; 
        			long	NumPointsInBounds;
        			double	TotalValueInArea;
        			HANDLE	hPoints; 
        			MNMXCORD	Bounds;
        			HANDLE	hAccelerator;
        		} THEMEAREAHEADER;
typedef THEMEAREAHEADER	FAR *LPTHEMEAREAHEADER;

typedef struct {
        			long	Refno;
        			long	TotCount;
        			double	TotValue;
        		} PIADB;
typedef PIADB	FAR *LPPIADB;

typedef struct {
		short 	Class;
		long	Refno;
		} THEMEHIGHLIGHTKEY;
typedef THEMEHIGHLIGHTKEY *LPTHEMEHIGHLIGHTKEY;

typedef struct
    {
        short	Seq;
        long	Ref;
        char	Prefix[8],
        		UDI[32];
        char	Type;
        char	SymbolName[32];   
        char	File[255];
		char	Material[32];
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
		char	Prefix[10],UDI[64];
		short	Type;
		short	Desc;
		MNMXCORD	Bounds;
		char	Value[MAX_THEME_VALUE_LEN];
		}THEMEHIGHLIGHTDATA;
typedef THEMEHIGHLIGHTDATA *LPTHEMEHIGHLIGHTDATA;
		
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


typedef	struct	{
					short	nColors;
					RGBTRIPLE	Colors[MAXSIGNATURECOLORS];
					short	ColorsN[MAXSIGNATURECOLORS];
				}FILLSIGNATURE;  
typedef FILLSIGNATURE FAR	*LPFILLSIGNATURE;
 

typedef struct {char	ID[32],   
						Desc[128],
						ID2[32],
						Driver[64],
						Macro[32];
				COLORREF	Color,TrackColor;     
				BOOL	ComputeAZ, ComputeSpeed;
				long	ComputeTimeSpan;
				short	nLoc,
						Size[2],
						Symbol,
						StatusSymbol;
				double	Speed,AZ;
				long	SaveScreenID[MAX_VIEWPORTS];
				HANDLE	hSaveScreen[MAX_VIEWPORTS];
				DPOINT	Loc[MAXVEHLOC];         
				time_t	LocTime[MAXVEHLOC]; 
				short	Speeds[MAXVEHLOC];
				short	Heading[MAXVEHLOC]; 
				short	Status;
				short	FenceStatus;
				long	NumHist;
				HANDLE	hHist;
				BOOL	Display;
				RECT	Rect, MenuRect,LastDrawRect,LastFlagRect;
				int		index;
				} VEHLOCATION;
typedef	VEHLOCATION	FAR	*LPVEHLOCATION;

typedef struct {int		Type;
				char	Name[42];
				double	Offset;
				double	BaseOffset;
				int		NumPoints;
				HANDLE	hAccelerator;
				DPOINT	Point;
				char	OffsetUnits;
				char	filler;
				} FENCEHEADER;
typedef FENCEHEADER *LPFENCEHEADER;

typedef struct {DPOINT Point;
				double Elev;} RADIAL;
typedef RADIAL	FAR	*LPRADIAL;

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
				long	VPID;
				long	Refno;
				}SKIPREF;
typedef SKIPREF	*LPSKIPREF;

typedef struct {
		long	Refno;
		long	dupnum;
		} DUPREFKEY; 
typedef	struct	{
				 USHORT	FileInIndex; 
				 unsigned	long	Segment:31,
				 					Deleted:1;
				 WORD	Offset; 
				 RECT16	MinMax;
				 short	FileNum;
				 short	SubFile;
				 short	Desc;
		 		}	DUPREFDATA; 

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
				Symbol;
		int		NumPoints;
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
				int		NumLinks,
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
			long	SubDir;
			short	Format;
 			long	Size,Time,NumPoints;
			DWORD	Loc;     
			double	LLNormFactor;
			double	MetersPerDegree;
			double	Scale;    
			long	GridID,GridCellID;
			MNMXCORD	Bounds;
		}	MAPFILE;
typedef MAPFILE	FAR	*LPMAPFILE;

typedef	struct	{long	GridID;
				 long	Seq;
				 short	CharNo;
				 char	chr[2];
				 short	x,y;
				 short	size;
				 short	rot;
				 short  isym;} SAVECONTTEXT;
typedef	SAVECONTTEXT	*LPSAVECONTEXT;

typedef	struct	{char	QuadKey[20];
				 long	Seq;
				 short	depth;
				 short	x,y;
				 double	latitude,longitude;
				 short	size;
				 short	rot;
				 float	az;
				 } SAVECONTTEXT2;
typedef	SAVECONTTEXT2	*LPSAVECONTEXT2;
 typedef	struct	{char	QuadKey[20];
				 long	Seq;
				 } SAVECONTTEXT2KEY;
typedef	SAVECONTTEXT2KEY	*LPSAVECONTEXT2KEY;

typedef	struct	{
				 short	CharNo;
				 char	chr[2];
				 short	x,y;
				 short	size;
				 short	rot;} CTEXTOUTREC;
typedef	CTEXTOUTREC	*LPCTEXTOUTREC;

typedef	struct	{
				 long	ref;
				 short	symno,hasextendedtext;
				 short	x,y;
				 char	PointText[64];
				 } MAPPOINTOUTREC;
typedef	MAPPOINTOUTREC	*LPMAPPOINTOUTREC;

typedef	struct	{
				 long	Population;
				 short	type;
				 short	nchar;
				 short	x,y;
				 char	State[4];
				 char	Name[68];
				 } CITYLAKENAMEOUTREC;
typedef	CITYLAKENAMEOUTREC	*LPCITYLAKENAMEOUTREC;

typedef	struct	{
				 long	CellID, Population,ref;
				 short	symnum;
				 double	x,y;
				 double	pctx,pcty;
				 char	State[2];
				 char	Name[100];
				 } CITYLAKENAMES;
typedef	CITYLAKENAMES	*LPCITYLAKENAMES;

typedef	struct	{
				 long	CellID, ref;
				 short	symnum;
				 double	x,y;
				 double	pctx,pcty;
				 short	hasextendedtext;
				 char	PointText[64];
				 } MAPPOINTS;
typedef	MAPPOINTS	*LPMAPPOINTS;

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
				DPOINT	Point;
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

typedef struct	{
					RECT	Rect;
					int		Offset;
					int		MacroLen;
					DPOINT	WPoint;
				}DATADISPLAYRECT;
typedef DATADISPLAYRECT FAR *LPDATADISPLAYRECT;

typedef struct {short	ID,
					  	VPID;
				RECT	Rect;
				}INFOBOXRECT;
typedef	INFOBOXRECT	FAR	*LPINFOBOXRECT;

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
				unsigned	short	FlipForEasyReading:1, 
									UltiMapStyle:1,
									italic:1, 
									weight:2, 
									Opaque:1,
									Shadow:1,
									Filler:9;
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
				unsigned	short	style1:4, 
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
				 short	FileNum;  
				 short	SubFile;
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
				 
typedef struct {char Name[256+32];MNMXCORD MinMax;long StartRefno;} FILELISTENTRY;
typedef FILELISTENTRY	HUGE	*LPFILELISTENTRY;  

typedef	struct {long ref;
				short Layer, State, width, height, symnum;
				short unusedspacer;
				POINT Point;
				float AZ,Length;
				long Refno;
				COLORREF TextColor, ShadowColor;} MIDPOINT;
typedef	MIDPOINT	HUGE	*LPMIDPOINT;

#pragma pack(2)
typedef struct {short	Desc; long Num;} REORG;
typedef REORG	FAR	*LPREORG;
typedef struct {short	Desc; long Offset;} DESCBLOCK;
typedef DESCBLOCK	HUGE	*LPDESCBLOCK;
#pragma pack()

typedef	struct	{
				long	Path;
				double	StartMP;
				}	NETMARKERSTHEMEKEY;  
				
typedef struct	{  
				short		Code101;
				long	UsedDescOffset;
				short		Code102;
				long	ColorPaletteOffset;
				short		Code103;
				long	TranPointOffset;
				short		Code200;
				long	GraphicsOffset; 
				short		Code201;
				long	DescBlockOffset; 
				short		Code202;
				long	MinTime,MaxTime; 
				short		Term;
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
	{   short		Marker;
		mnmxCor	MinMax;
		short		Len;
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
				 RECT16	MinMax;
				 }	REFINDEXDATA; 
typedef	REFINDEXDATA	FAR	*LPREFINDEXDATA;

typedef struct {
				unsigned	short		lText:9, 
									FontNum:5,
				 					Weight:2;
				unsigned	short		vJust:2,      //0=above,1=baseline,2=center,3=below
									hJust:2,      //0=left,1=center,2=right
									italic:1,
									shadow:1,  
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
	{	short		Parent;
		HANDLE	Next;
	} PARLIST;
typedef PARLIST FAR *LPPARLIST;

#pragma pack(1)
typedef struct
	{
		short		ID;
		HANDLE16	handle;
		short		TargetViewport;
		BOOL16	IsActive;
		short		DisplayViewport;
		short		CoordinateSystem,
				Units,
				Format;
       	LOGFONT16	Font;
    }	COORDINATEDISPLAY_V0;

typedef struct
	{
		short	ID;
		HANDLE16	handle;
		short	Version;
		short	TargetViewport;
		short	DisplayViewport;
		BOOL16	DisplayLine[3];
		char	AltCVTFile[14],
				LineID[3][34];
		short	Units[3],
				Precision[3];
		BOOL16	Commas[3];
       	LOGFONT16	Font;  
       	COLORREF	FontColor;  
       	BOOL16	Refresh; 
       	char	PrintText[100];
       	char	Space[22];
    }	COORDINATEDISPLAY_V101;
typedef	COORDINATEDISPLAY_V101	FAR	*LPCOORDINATEDISPLAY_V101;

typedef struct
	{
		short	ID;
		HANDLE16	handle;
		short	Version;
		short	TargetViewport;
		short	DisplayViewport;
		BOOL16	DisplayLine[4];
		char	AltCVTFile[14],
				LineID[4][34];
		short	Units[4],
				Precision[4];
		BOOL16	Commas[4];
       	LOGFONT16	Font;  
       	COLORREF	FontColor;  
       	BOOL16	Refresh; 
       	char	PrintText[100];
       	char	Space[22];
    }	COORDINATEDISPLAY_V102;
typedef	COORDINATEDISPLAY_V102	FAR	*LPCOORDINATEDISPLAY_V102; 

typedef struct
	{
		short	ID;
		HANDLE16	handle;
		short	Version;
		short	TargetViewport;
		short	DisplayViewport;
		BOOL16	DisplayLine[4];
		char	AltCVTFile[14],
				LineID[4][34];
		short	Units[4],
				Precision[4];
		BOOL16	Commas[4];
       	LOGFONT16	Font;  
       	COLORREF	FontColor;  
       	BOOL16	Refresh; 
       	char	PrintText[100]; 
       	char	ElevSurface[128]; 
       	HANDLE16	hSurf; 
       	short	ElevUnits,
       			ElevPrecision;
       	BOOL16	ElevCommas;
       	BOOL16	DisplayElevation;
       	char	ElevationID[34];
       	char	Space[470];
    }	COORDINATEDISPLAY_V103;
typedef	COORDINATEDISPLAY_V103	FAR	*LPCOORDINATEDISPLAY_V103; 

typedef struct {
		short	Number,
				Parent;
		char	Name[34];
		char	Desc[61]; 
		float	BaseSize;
		char	InVisible;
		unsigned	short	Type:3,    		// 0=parent, 1=point, 2=line, 3=area      
						DisplayPos:3,		// 1,2 = after area pass, 3,4=after line pass, 5,6=after text pass
						BlockRotation:1,
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
		HANDLE16	hElement;
	} SYMBOL_V1;
typedef SYMBOL_V1	FAR	*LPSYMBOL_V1; 

#pragma pack()

#pragma pack(1)
typedef struct    		
	{
		short		ID;
		HANDLE16	handle; 
		short		Version;
		short		TargetViewport,
				DisplayViewport;
		BOOL16	IsActive,
				WantDataPass,
				ComputeClassBoundaries,
				DisplayScatterDiagram,
				Recompute,
				ReScan;  		// also holds network opened flag, TargetViewport2
		HANDLE16	hThemeDB,
				hScatterFile;
		char	DataFile[128];
		char	SQL[256];
		short		DataFileType,
				DataType;
		FIELDINFO16	Field[1]; 
		short	ClassSymbol[MAX_THEME_CLASSES_v0];      
		char	SymSizeC[32];
		BYTE	ClassIsSelected[MAX_THEME_CLASSES_v0];
		LPVOID	Statement;
		char	ScatterFile[128];
		short		NumClass;
		short		NumDesiredClass;
        short		ClassType;
        short		ValConv;  
        short		MissOpt;
        BOOL16	MarkInvalid;
        double	XLimit,
        		YLimit,
        		RoundTo;
        long	NumVals;
        double	Xmin,Ymin,Xmax,Ymax;
        double	ClassMin[MAX_THEME_CLASSES_v0],
        		ClassMax[MAX_THEME_CLASSES_v0];
        COLORREF	ClassColor[MAX_THEME_CLASSES_v0]; 
        HPEN16		ClassPen[MAX_THEME_CLASSES_v0];
        HBRUSH16		ClassBrush[MAX_THEME_CLASSES_v0],
        		NoDataBrush,
        		InvalidDataBrush;
        long	ClassCount[MAX_THEME_CLASSES_v0];
        DPOINT	ClassPnt[MAX_THEME_CLASSES_v0];
        COLORREF	BGColor, ScatterColor, ScatterBoxBG, TitleBoxBG;
        RECT16	Rect, TitleBox, ScatterBox, ColorsBox, RangesBox, InfoBox;
        RECT16	ClassClrBox[MAX_THEME_CLASSES_v0];
        short		Margin, ScatterWidth, ColorsWidth, InnerMargin, TitleHeight;
        char	ClassBM[MAX_THEME_CLASSES_v0][128];
        char	Title[256]; 
        char	Contents[10];
        short		SymNum;
       	LOGFONT16	TitleFont,
       			ClassFont1,
       			ClassFont2;
       	short		Xmove, Ymove;  	// Xmove also used to hold NetworkID, DisplayOpt
       	BOOL16	ZeroIsMissing,
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
       	HANDLE16	hVisList;
       	short	DispersePoints;
       	HANDLE16	hDisperseFileName,
       			hDisperseFile;
       	short	MaxDispersion;
       	HANDLE16	hHighlightFileName,
       			hHighlightFile;
       	BYTE	AccumPointSymbolOpt;
       	BYTE	AccumPointSizeOpt; 
       	float	AccumPointBaseSize;  
       	BYTE	AccumPointLink; 
       	BYTE	AccumPointText;
       	BOOL16	ZeroBased;
       	BOOL16	PCTByArea;
       	long	NumNonMask;
       	long	PCTDisplayCycle; 
       	short	LayerID;
       	BOOL16	InvertLegend;
       	short	NumCols;
       	BOOL16	HiPrecis;
       	short	NumMidpoint;  
       	short	AllValueClass; 
		unsigned	short	ClearIfNoCount:1,
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
		HANDLE16	handle; 
		short	Version;
		short	TargetViewport,
				DisplayViewport;
		BOOL16	IsActive,
				WantDataPass,
				ComputeClassBoundaries,
				DisplayScatterDiagram,
				Recompute,
				ReScan;  		// also holds network opened flag, TargetViewport2
		HANDLE16	hThemeDB,
				hScatterFile;
		char	DataFile[128];
		char	SQL[256];
		short	DataFileType,
				DataType;
		FIELDINFO16	Field; 
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
        HPEN16		ClassPen[MAX_THEME_CLASSES];
        HBRUSH16		ClassBrush[MAX_THEME_CLASSES],
        		NoDataBrush,
        		InvalidDataBrush;
        long	ClassCount[MAX_THEME_CLASSES];
        DPOINT	ClassPnt[MAX_THEME_CLASSES];
        COLORREF	BGColor, ScatterColor, ScatterBoxBG, TitleBoxBG;
        RECT16	Rect, TitleBox, ScatterBox, ColorsBox, RangesBox, InfoBox;
        RECT16	ClassClrBox[MAX_THEME_CLASSES];
        short	Margin, ScatterWidth, ColorsWidth, InnerMargin, TitleHeight;
        char	ClassBM[MAX_THEME_CLASSES][128];
        char	Title[256]; 
        char	Contents[34];
        short	SymNum;
       	LOGFONT16	TitleFont,
       			ClassFont1,
       			ClassFont2;
       	short	Xmove, Ymove;  	// Xmove also used to hold NetworkID, DisplayOpt
       	BOOL16	ZeroIsMissing,
       			AddCommas,
       			ShowValue,
       			VPDisplayed,
       			DisplayPCT; 
       	char	RefValChar[36]; 
       	double	RefValDbl;  
       	short	FieldFun, 
       			ValueLen;
       	short	FieldCorrection,
       			ShortDummy2;   
       	COLORREF	IBBGColor, TitleTextColor, IBTextColor[2]; 
       	short	RefIsPCT; 
       	char	Value[128]; 
       	HANDLE16	hVisList;
       	short	DispersePoints;
       	HANDLE16	hDisperseFileName,
       			hDisperseFile;
       	short	MaxDispersion;
       	HANDLE16	hHighlightFileName,
       			hHighlightFile;
       	BYTE	AccumPointSymbolOpt;
       	BYTE	AccumPointSizeOpt; 
       	float	AccumPointBaseSize;  
       	BYTE	AccumPointLink; 
       	BYTE	AccumPointText;
       	BOOL16	ZeroBased;
       	BOOL16	PCTByArea;
       	long	NumNonMask;
       	long	PCTDisplayCycle; 
       	short	LayerID;
       	BOOL16	InvertLegend;
       	short	NumCols;
       	BOOL16	HiPrecis;
       	short	NumMidpointnotused;  
       	short	AllValueClass; 
		unsigned	short	ClearIfNoCount:1,
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
		HOTSPOTDATA16 HotSpotData; 
		float	ClassFactor[MAX_THEME_CLASSES]; 
		char	HotSpotCompareTo[128];
		char	HotSpotSaveTo[128];  
		double	CompareHotSpotFactor;
		char	IconLibrary[128];//also used for hotspot weight 
		short	ActualXMargin;
		short	ActualYMargin; 
		LOGFONT16	ShowValueFont;
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
		HANDLE16	hHotSpotBitmap; 
		MNMXCORD	HotSpotBounds;  
		unsigned	short	ComputeStoredCounts:1,
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
		POINTS	RadiusPoint;
		double	Radius; 
		double	DynSegMaxFixedIncrement;
		double	DynSegFixedIncrement; 
		double	ShowValAZ;  
		short	ProfileUnits;
        char	Contents2[34];
        short	SymNum2;
       	HANDLE16	hVisList2;
       	HANDLE16	hAreas;
       	USHORT	NumAreas;
       	short	ACCDisperse; 
       	short	Pass;
       	HPEN16	PointInAreaPen;
       	HBRUSH16	PointInAreaBrush; 
       	COLORREF	PointInAreaColor;
       	HFILE16	FidAreas; 
       	short	PointInAreaPointThemeVPID;  
       	long	NumMidpoint; 
       	char	ShowValMacro[128]; 
       	HFILE16	FidDelayedText; 
       	HANDLE16	hDelayedFont;
		char	filler[22];
	}	THEME_V1;
typedef THEME_V1	FAR	*LPTHEME_V1;
typedef struct
	{
		short	NameSource;
		short	LayerID;
		float	MinTextSize,MaxTextSize;
		char	Macro[256];
		BOOL16	ShowCities;
		short	NameLength; 
		HANDLE16	hNameFile1, hNameFile2;
		char	NameFile1[128];
		char	NameFile2[128]; 
		BOOL16	Italic, Shadow;
		short	BackgroundOpt; //0=transparent,1=opaque,2=opaque if ortho on
		COLORREF	TextColor, ShadowColor;
		BOOL16	ShowAllElements; 
		BOOL16	SavePAE;  
		short	VJust; 
		BOOL16	HorizontalText;
		BOOL16	PrimeNameOnly;
		char	Expression[256]; 
		char	ListFile[128]; 
		char	ListData[128]; 
		short	ListLen;   
		BOOL16	SortOnData;
		HANDLE16	hListDB; 
		short	MinSize;
       	LOGFONT16	StreetTextFont;
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

	}	STREETTEXTDATA16;
typedef STREETTEXTDATA16	FAR	*LPSTREETTEXTDATA16;

#pragma pack()

typedef struct {
				long	Population;
				short	Unique;
				} CITY_NAME_KEY;
typedef CITY_NAME_KEY	*LPCITY_NAME_KEY;

typedef struct {
				char	Text[MAX_CITYNAME_LENGTH];
				POINT	Point;
				int		SymNum;
				} CITY_NAME_DATA;
typedef CITY_NAME_DATA	*LPCITY_NAME_DATA;

#pragma pack(2)
typedef struct
	{
		short	ID;
		HANDLE16	handle16;
		short	Version;
		HANDLE	handle;
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
		BOOL	MultiLine;
       	char	Space[466];
    }	COORDINATEDISPLAY;
typedef	COORDINATEDISPLAY	FAR	*LPCOORDINATEDISPLAY; 

typedef struct    		
	{
		short	ID;
		HANDLE16	handle16; 
		short	Version;
		HANDLE	handle;
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
		char	DataFile[MAX_PATH];
		char	SQL[256];
		short	DataFileType,
				DataType;
		FIELDINFO	Field; 
		short	ClassSymbol[MAX_THEME_CLASSES];      
		char	SymSizeC[32];
		BYTE	ClassIsSelected[MAX_THEME_CLASSES];
		LPVOID	Statement_dummy;
		char	ScatterFile[MAX_PATH];
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
        char	ClassBM[MAX_THEME_CLASSES][256];
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
       	short	FieldFun, 
       			ValueLen;
       	short	FieldCorrection,
       			CityUniqueInc;   
       	COLORREF	IBBGColor, TitleTextColor, IBTextColor[2]; 
       	short	RefIsPCT; 
       	char	Value[256]; 
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
		unsigned	short	ClearIfNoCount:1,
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
						FlatEndOffsetLine:1; 
		char	ClassDefDB[MAX_PATH], 
				ClassDefSQL[256],
				ClassDefKeyField[34],
				ClassDefSymField[34],
				ClassDefTitleField[34],
				ClassDefValDB[MAX_PATH],    
				ClassDefValSQL[256],   //also used for hotspot radius
				ClassDefValField[34];
		char	SymbolFont[4][64];  
		BYTE	ClassStatus[MAX_THEME_CLASSES];
		HOTSPOTDATA HotSpotData; 
		float	ClassFactor[MAX_THEME_CLASSES]; 
		char	HotSpotCompareTo[MAX_PATH];
		char	HotSpotSaveTo[MAX_PATH];  
		double	CompareHotSpotFactor;
		char	IconLibrary[MAX_PATH];//also used for hotspot weight 
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
		short	MultiValOption; 
		short	ColorScheme; 
		HANDLE	hHotSpotBitmap; 
		MNMXCORD	HotSpotBounds;  
		unsigned	short	ComputeStoredCounts:1,
						UseStoredCounts:1,
						UseCheckmark:1, 
						DelayTextDisplay:1,
						ShowValStyle:3,//0=original,1=yellowtb
						ProfileAlignmentOption:2,
						ComputeAreaAndLength:1,
						UseShadowColor:1,
						ShowDirection:2,
						ExpressionConverted:1,
						CompareAttributes:1,// also used for auto city max pop option
						ShowCityCircle:1;  
		double	DistanceBetweenProfilePoints; 
		double	CrossSectionSpacing;
		double	CrossSectionWidth[2]; 
		POINT	RadiusPoint;
		double	Radius; 
		double	DynSegMaxFixedIncrement;
		double	DynSegFixedIncrement; 
		double	ShowValAZ;  
		short	ProfileUnits;
        char	Contents2[64];
        short	SymNum2;
       	HANDLE	hVisList2;
       	HANDLE	hAreas;
       	USHORT	SortOption;
       	short	ACCDisperse; 
       	short	Pass;
       	HPEN	PointInAreaPen;
       	HBRUSH	PointInAreaBrush; 
       	COLORREF	PointInAreaColor;
       	HFILE	FidAreas; 
       	short	PointInAreaPointThemeVPID;  
       	long	NumMidpoint; 
       	char	ShowValMacro[256]; 
       	HFILE	FidDelayedText; 
       	HFONT	hDelayedFont;
		long	NumAreas;
		char	GraphicsAttributesMacro[MAX_PATH];
		char	DataDisplayMacro[512];
		float	AbsLineWidth[MAX_THEME_CLASSES];
		COLORREF ShowValueShadowColor; 
		char	CurValue[256];
		char	DataFileID[34];
		HDC		CompareDC;
		HBITMAP	CompareBitmap,CompareBitmapOld;
		BYTE	SortOrder[MAX_THEME_CLASSES];
		float	ScaleBarTextFactor;
		int		nLabelLines;
		HANDLE	hhLabelLines;
		long	MinCityPop;
		int		CityTextRectInflateFactor;
		int		CityTextMinSize, CityTextMaxSize;
		short	CityTextSizeOpt;
		USHORT	MaxCitiesToDisplay;
		int		MaxCityPopOnScreen;
		short	GridID;//0=latlon,1=Google
		short	GridZoom;
		int		numPreloadedValues;
		char	filler[682];
	}	THEME;
typedef THEME	FAR *LPTHEME;

typedef struct
	{
		short	NameSource;
		short	LayerID;
		float	MinTextSize,MaxTextSize;
		char	Macro[256];
		BOOL	ShowCities;
		short	NameLength; 
		HANDLE	hNameFile1, hNameFile2;
		char	NameFile1[MAX_PATH];
		char	NameFile2[MAX_PATH]; 
		BOOL	Italic, Shadow;
		short	BackgroundOpt; //0=transparent,1=opaque,2=opaque if ortho on
		COLORREF	TextColor, ShadowColor;
		BOOL	ShowAllElements; 
		BOOL	SavePAE;  
		short	VJust; 
		BOOL	HorizontalText;
		BOOL	PrimeNameOnly;
		char	Expression[256]; 
		char	ListFile[MAX_PATH]; 
		char	ListData[MAX_PATH]; 
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
							RotateToScreen:1,
							NonAntialiased:1,
							Dummy:4;
       	char	VisMacro[256],
       			ColorMacro[256],
       			DisplayNameMacro[256];

	}	STREETTEXTDATA;
typedef STREETTEXTDATA	FAR	*LPSTREETTEXTDATA;

typedef struct
	{	short	Len;
		short	BMWidth, BMHeight,BMBitCount;
		MNMXCORD	Bounds;
		char	Name[128];
	}	FILEINDEXENTRY;
typedef FILEINDEXENTRY	FAR		*LPFILEINDEXENTRY;

typedef struct
	{	short	Type;
		short	NumFiles;
		long	Length;
		short	CurFile;
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
		char	FileName[MAX_PATH];
		char	FirstIndex;
	}	FILEINDEX;
typedef FILEINDEX	FAR *LPFILEINDEX; 
#pragma pack()

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
		float		Width;
		unsigned	char	Type:2;   //if Type = 0 Width = text factor
		unsigned	char	Style:3; 
		unsigned	char	ProPen:3;
		unsigned	char	R,G,B;
	}	NEWOBJECT;
typedef NEWOBJECT FAR  *LPNEWOBJECT;

typedef struct	{long	iX,iY,Ref;} DUPHEADER;
typedef DUPHEADER	FAR	*LPDUPHEADER;

typedef	struct	{DPOINT Point;
				 int	iDesc;
				 char	Prefix[8],UDI[64];
				} DUPDATA;
typedef DUPDATA	FAR	*LPDUPDATA;

typedef struct	{double	XorY;
				 long	Ref;} NPHEADER;
typedef NPHEADER	FAR	*LPNPHEADER;

typedef struct
	{
		short	Show; 
		long	Sequence; 
		PICKDATAHEADER	PD; 
	} HIGHLIGHTDATA_SHORT;
	
typedef struct
	{
		short	Show; 
		long	Sequence; 
		PICKDATA	PD; 
		ULONG	HLTGraphicsFilePos;
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

typedef struct{long PNREF;
			   char STREETNAME[34],
				    ALTSTREETNAME1[34],
				    ALTSTREETNAME2[34],
				    ALTSTREETNAME3[34],
					FROMSTREET[34],
					TOSTREET[34],
					CITYL[64],CITYR[64];
			   long FROMADDRESSL,TOADDRESSL,
				    FROMADDRESSR,TOADDRESSR,
					ZIPL,ZIPR;
				}PNSTREETDATA;
typedef PNSTREETDATA	*LPPNSTREETDATA;

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


typedef struct {
				unsigned	short	AutoEdit:1, 
									AutoMove:1,
									ExpandText:1,
									FixedSize:1,
									UseOrigSize:1,
									CloseIcon:1,
									ShadowText:1;
				}IBFLAGS;
#pragma pack(2)
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
	short		incx;
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
	COLORREF	InnerColor;
	short		Shape;
	long		Refno;
	HANDLE		hReport;
	char		Prefix[10],UDI[34];
	char		DataFile[128];
	char		SQL[256];
	char		text[858];
	short		incy;
	COLORREF	FontShadowColor;
	MNMXCORD	BlowUpBounds;  
	char		PickMacroFile[128];
	
} TAGBOX;
typedef TAGBOX	FAR *LPTAGBOX;
#pragma pack()
typedef struct
{	MNMXCORD	FromBounds;
	MNMXCORD	ToBounds; 
	RECT		ToWinRect;
}	BLOWUPREC;


#pragma pack(2)
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

typedef struct
     {
       long		Offset;
       long		ImageLen;
	   RECT		ClientRect;
	   RECT		Rect;
	   MNMXCORD	Bounds;
     } SAVEDIMAGEDATA;
typedef SAVEDIMAGEDATA    *LPSAVEDIMAGEDATA;
  
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
	 	short		WidthType; 		/* 1 = fixed, 2 = relative, 3=pixels								 */
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
									UsePanZoomControl:1,
									MinMaxIcon:1,
									HaveFixedProfileRoute:1;
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
		char		FileID[MAX_VIEWPORT_FILES][MAX_VPFILE_ID];/* File identifier displayed to user      */
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
		int		NumMaskPoints;
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
		char	RButFunction[MAX_RBUTFUN];
		short	OnPrintAddSpaceToVP; 
		char	unusedspace[16];
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
		short	NumMaskAreaParts; 
		short	numBackgroundAreaParts;
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
		HWND	hWndDlg;
		char	DlgInitCmd[256];
		POINT	PanZoomControlPoint;
		char	BeginDisplayCmd[256];
		char	EndDisplayCmd[256];
		RECT	MinMaxIconRect;
		ULONG	DisplayCycle;
		RECT	ProfileRect;
		MNMXCORD	ProfileBounds;
		BYTE	LabelAllContours[MAX_VIEWPORT_FILES];
		RECT	DisplayRect;
		double	DisplayRectFactor;
		HBITMAP	hVPBitmap;
		double	MaxOffsetDist;
		HANDLE	hProfileRoute[MAXPROFILEROUTES];
		long	nProfileRoute[MAXPROFILEROUTES];
		HANDLE	hProfileElev[MAXPROFILEROUTES];
		long	nProfileElev[MAXPROFILEROUTES]; 
		short	nProfileRoutes;
		short	FileProjectionType;
		HANDLE	hTranProjectionToScreen;
		HANDLE	hTranScreenToProjection;
		HANDLE	hBackgroundArea;
		int		numBackgroundAreaPoints;
		COLORREF	BackgroundAreaColor;
		int		transParency;
		HBITMAP hTransparencyBitmap, origTransparencyBitmap;
		HDC		hTransparentDC, originalDC;
		HRGN	hTransparentNullRgn;
		int		transparencyBitmapWidth, transparencyBitmapHeight;
		unsigned short WantDisplayHighlight:1,
					   unused:15;
		char	GrowSpace[2898-16*MAXPROFILEROUTES-2*sizeof(short)-2*sizeof(HANDLE)
						  -sizeof(HANDLE)-sizeof(int)-sizeof(COLORREF)
						  -sizeof(int)-2*sizeof(HBITMAP)-2*sizeof(HDC)
						  -sizeof(HRGN)-2*sizeof(int)-sizeof(short)];
        char		EndOfViewport;     
        
	}	VIEWPORT;
typedef VIEWPORT	FAR	*LPVIEWPORT;    
#pragma pack()

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
	short	MaxPriority;
	}	UserInfo;

typedef	struct
	{ long	TLID,
			StreetNum[4],
			faddl, faddr, taddl, taddr,
			ZIPL,
			ZIPR;
	  short	
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
	  long	UpdateTime;
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
		 }	SEGDATAGS;  
typedef	SEGDATAGS	FAR	*LPSEGDATAGS;	

typedef struct {
		long	TLID;
   		long	House;
 	  } POINTADDKEY; 
typedef POINTADDKEY	FAR	*LPPOINTADDKEY;

typedef struct {
		long	Refno;
   		short	Desc;
		long	nPnts;
		double	Length;
 	  } STREETPOLYHEADER; 
typedef STREETPOLYHEADER	FAR	*LPSTREETPOLYHEADER;

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
		char	JustC[2];
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
		COLORREF	FontShadowColor[MAXREPORTFONTS];        
		short	FontSize[MAXREPORTFONTS],
				FontWeight[MAXREPORTFONTS],
				FontItalic[MAXREPORTFONTS],
				FontUnderline[MAXREPORTFONTS],
				FontShadow[MAXREPORTFONTS];
		char	FontName[MAXREPORTFONTS][LF_FACESIZE]; 
		BOOL	WantSize;
		RECT	SizeRect;
		int		curLineHeight;
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
        int     NumFiles;
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
                HANDLE  BTHandle[MAX_GMD_INDEXES16];
                short   NumIndexFields[MAX_GMD_INDEXES16], //neg indicates sparse index
                        IndexFields[MAX_GMD_INDEXES16][MAX_GMD_INDEX_FIELDS],
                        lKeys[MAX_GMD_INDEXES16]; 
                HANDLE  hKeys[MAX_GMD_INDEXES16];
                LPSTR   pKeys[MAX_GMD_INDEXES16];
                long  TimeStamp;
                HANDLE  hFldInfo;                       
                LPGWFLDINFO pFldInfo;
                char    GWDData[2];
                }   GWDHEADER32;
typedef GWDHEADER32 FAR  *LPGWDHEADER32;  

typedef struct {
				unsigned short
                		Version:13,
						Compressed:1, 
						NonUniqueSortedBinary:1,
						StoredAs32:1;
				short	FileVersion;
				short	Unused1;
                short   NumFields,
                        NumIndex;
                long    Fid;
                long    Reclen;
                HANDLE  BTHandle[MAX_GMD_INDEXES];
                short   NumIndexFields[MAX_GMD_INDEXES], //neg indicates sparse index
                        IndexFields[MAX_GMD_INDEXES][MAX_GMD_INDEX_FIELDS],
                        lKeys[MAX_GMD_INDEXES]; 
                HANDLE  hKeys[MAX_GMD_INDEXES];
                LPSTR   pKeys[MAX_GMD_INDEXES];
                long  TimeStamp;
                HANDLE  hFldInfo;                       
                LPGWFLDINFO pFldInfo;
				MNMXCORD	FileBounds;
				MNMXCORD	GridBounds;
				int		GridInc;
				double	GridXInc, GridYInc;
				long	MinTime,
						MaxTime;
				short	SpatialIndex,
						SpatialIndexType,
						FromDateField,
						ToDateField,
						XField,
						YField,
						SymbolField,
						CoordConversion;
				BYTE	Contents[400];
				ULONG	CheckPointID;
				HANDLE	CheckPointHandle;
				char	Unused2[248];
                char    GWDData[2];
                }   GWDHEADER;
typedef GWDHEADER FAR  *LPGWDHEADER;  

typedef struct {
				ULONG	FirstCheckPointID;
				ULONG	LastCheckPointID;
				int		FirstCheckPointLoc[MAX_GMD_INDEXES+1];
				int		LastCheckPointLoc[MAX_GMD_INDEXES+1];
				int		nCheckPoints;
				int		BlockSize;
				int		FirstFreeBlock;
				int		LastFreeBlock;
				int		MaxFreeBlockSize;
				char	unused[1020];
				}	CHECKPNTLOGHEADER;
typedef CHECKPNTLOGHEADER	*LPCHECKPNTLOGHEADER;

typedef struct {
				int		PreviousRec;
				int		NextRec;
				int		Reclen;
				int		NumBlocks;
				BYTE	BlockChangedBits[4];
				}	CHECKPNTLOGRECORD;
typedef CHECKPNTLOGRECORD	*LPCHECKPNTLOGRECORD;

typedef struct {
				ULONG	CheckPointID;
				int		BlockSize;
				int		NumBlocks;
				int		FileLength;
				int		OrigFileLength;
				int		BlockIndexLoc;
				BOOL	HasBeenDeleted;
				}	JOURNALHEADER;
typedef JOURNALHEADER	*LPJOURNALHEADER;

typedef struct {
				int		LastRec,NextRec;
				int		BlockSize;
				int		StartBlock;
				int		NumBlocks;
				}	JOURNALRECORDHEADER;
typedef JOURNALRECORDHEADER	*LPJOURNALRECORDHEADER;

typedef struct {
				int		StartBlock;
				USHORT	NumBlocks;
				int		JournalFileOffset;
				}	JOURNALINDEXRECORD;
typedef JOURNALINDEXRECORD	*LPJOURNALINDEXRECORD;

typedef struct {
                short   Version,
                        NumFiles,
                        NumFields, 
                        TotFileLen;  
                HANDLE  hSQL[5];
                char    FileNames[5][MAX_PATH];
                FIELDINFO   FldInfo[1];     
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
	  short	
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
	  short	
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

#pragma pack(1)
typedef struct {
				DWORD SRID;
				BYTE  Version, SerializationProperties;
				DWORD  numPoints;
				}MSGEOGRAPHY;
typedef MSGEOGRAPHY *LPMSGEOGRAPHY;
typedef struct {
				DWORD parentOffset,
					  figureOffset;
				BYTE  OpenGISType;
				}MSSHAPE;
typedef MSSHAPE *LPMSSHAPE;
#pragma pack()

typedef struct {
	int		shapeType;
	int		geometryType;
	int		inUseLength;
	BYTE    isEmpty;
	BYTE	hasZs;
	BYTE	hasMs;
	BYTE	hasIDs;
	BYTE	hasCurves;
	BYTE	hasNormals;
	BYTE	hasTextures;
	BYTE	hasMaterials;
}FILEGDBRECHEADER;
typedef FILEGDBRECHEADER	*LPFILEGDBRECHEADER;

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
	  DPOINT	Point;
	} SHPPOINTZREC;
typedef SHPPOINTZREC FAR  *LPSHPPOINTZREC;   

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
		short	State;
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
		short	State;
		long	FIPS;
	}	CITIESKEY2; 
	
typedef struct
	{	
		char	Name[62];
		short	State;
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
				unsigned short	lFAddL:4,
								lTAddL:4,
								lFAddR:4,
								lTAddR:4;
			}ADDRESSLENGTHS;
typedef ADDRESSLENGTHS FAR *LPADDRESSLENGTHS;

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

typedef struct {
	    short	VPID;
		short	Type;
		long	Refno;
		double	Offset;
		int	np;
		int	nPoly;
		MNMXCORD	Bounds;
		}HIGHLIGHTAREAHEADER;
typedef HIGHLIGHTAREAHEADER *LPHIGHLIGHTAREAHEADER;

typedef struct {
	    short	VPID;
		long	Offset;
		}HAINDEX;
typedef HAINDEX *LPHAINDEX;

typedef	struct	{short Month, Grid;
				 char  PrimeIndex[1];
				} SPATIALINDEXTYPE1;
typedef	SPATIALINDEXTYPE1	*LPSPATIALINDEXTYPE1;
typedef	struct	{short Month, Code, Grid;
				 long  PrimeIndex;
				} SPATIALINDEXTYPE2;
typedef	SPATIALINDEXTYPE2	*LPSPATIALINDEXTYPE2;

typedef struct {int		nMenus;
				int		currentMenu;
				int		displayOpt;		//0=tabbed,1=tiled
				int		MaxWidth;
				double	Factor;
				HWND	hWnd;
				HWND	hWndMenu;
				HWND	hWndDisplay;
				HWND	hWndTab;
				HANDLE	hSaveScreen;
				BOOL	Float;
				BOOL	Verticle;
				BOOL	Flat;
				short	xpad,ypad;
				POINT	ScreenStartPoint;
				RECT	FixedRect;
				BOOL	HaveTrackMouseEvent;
				COLORREF	BackgroundColor;
				COLORREF	FontColorTab;
				int		AlphaBlendFactor;
				LOGFONT	LogFontTab;
				HFONT	hFontTab;
				int		menuType[MAXMENUSPERWINDOW];
				int		isDocked;
				int		dockWidth;
				HANDLE	menuHandle[MAXMENUSPERWINDOW];
				char	menuTitle[MAXMENUSPERWINDOW][32];
				char	appliesToVP[40];
				LPVIEWPORT	pVP;
				}WINDOWMENUHEADER;
typedef	WINDOWMENUHEADER	*LPWINDOWMENUHEADER; 

typedef	struct {POINT Point;
		double	AZ; 
		short	Type; 
		int		nPnts;
		HANDLE	hPoints; 
		int		nPoly;
		HANDLE	hPolyPartLen;
		LPTHEME pTheme;  
		long	Refno;
		short	Just;
		DPOINT	WPoint;
		char	Text[1024];} SHOWVAL;
typedef SHOWVAL *LPSHOWVAL;

void SetOldStructSizes (void);

#define ExtCreatePen GSSiEXTCREATEPEN
HPEN WINAPI GSSiEXTCREATEPEN(DWORD iPenStyle,
									DWORD cWidth,
                                    CONST LOGBRUSH *plbrush,
                                    DWORD cStyle,
                                    DWORD *pstyle);

#define CreatePen GSSiCREATEPEN  
HPEN    WINAPI GSSiCREATEPEN (int style, int width, COLORREF color);
#define CreateSolidBrush GSSiCREATESOLIDBRUSH
HBRUSH  WINAPI GSSiCREATESOLIDBRUSH(COLORREF color);
#define CreateHatchBrush GSSiCREATEHATCHBRUSH
HBRUSH  WINAPI GSSiCREATEHATCHBRUSH(int style, COLORREF color);  
#define CreatePatternBrush GSSiCREATEPATTERNBRUSH
HBRUSH  WINAPI GSSiCREATEPATTERNBRUSH(HBITMAP bitmap); 
#define CreateFontA GSSiCREATEFONT
HFONT   WINAPI GSSiCREATEFONT(int i1, int i2, int i3, int i4, int i5, BYTE b1, BYTE b2, BYTE b3, BYTE b4, BYTE b5, BYTE b6, BYTE b7, BYTE b8, LPCSTR name);
#define CreateFontIndirectA GSSiCREATEFONTINDIRECT
HFONT   WINAPI GSSiCREATEFONTINDIRECT (const LOGFONT FAR* lf);
#define CreateEllipticRgn GSSiCREATEELLIPTICRGN
HRGN    WINAPI GSSiCREATEELLIPTICRGN (int i1, int i2, int i3, int i4);
#define CreateRectRgn GSSiCREATERECTRGN
HRGN    WINAPI GSSiCREATERECTRGN (int i1, int i2, int i3, int i4);
#define CreateRoundRectRgn GSSiCREATEROUNDRECTRGN
HRGN    WINAPI GSSiCREATEROUNDRECTRGN(int x1,int y1,int x2,int y2, int w, int h);
#define CreateRectRgnIndirect GSSiCREATERECTRGNINDIRECT
HRGN    WINAPI GSSiCREATERECTRGNINDIRECT (const RECT FAR* pr);
#define CreatePolygonRgn GSSiCREATEPOLYGONRGN
HRGN    WINAPI GSSiCREATEPOLYGONRGN (const POINT FAR* pp, int i1, int i2);
#define CreateBrushIndirect GSSiCREATEBRUSHINDIRECT
HBRUSH  WINAPI GSSiCREATEBRUSHINDIRECT(LOGBRUSH FAR* logbrush);
#define DeleteObject GSSiDELETEOBJCT   
BOOL GSSiDELETEOBJCT (HGDIOBJ hobj);
#define SelectObject GSSiSELECTOBJECT  
HGDIOBJ GSSiSELECTOBJECT (HDC hdc,HGDIOBJ hobj);
#define SelectClipRgn GSSiSELECTCLIPRGN
int  WINAPI GSSiSELECTCLIPRGN(__in HDC hdc, __in_opt HRGN hrgn);
#define Polygon GSSiPolygon
BOOL  WINAPI GSSiPolygon(__in HDC hdc, __in_ecount(cpt) CONST POINT *apt, __in int cpt);
#define Polyline GSSiPolyline
BOOL  WINAPI GSSiPolyline(__in HDC hdc, __in_ecount(cpt) CONST POINT *apt, __in int cpt);
#define SetTextColor GSSiSetTextColor
COLORREF WINAPI GSSiSetTextColor(__in HDC hdc, __in COLORREF color);
#define StretchBlt GSSiStretchBlt
BOOL	 WINAPI GSSiStretchBlt(__in HDC hdcDest, __in int xDest, __in int yDest, __in int wDest, __in int hDest, __in HDC hdcSrc, __in int xSrc, __in int ySrc, __in int wSrc, __in int hSrc, __in DWORD rop);
#define BitBlt GSSiBitBlt
BOOL	WINAPI GSSiBitBlt( __in HDC hdc, __in int x, __in int y, __in int cx, __in int cy, __in_opt HDC hdcSrc, __in int x1, __in int y1, __in DWORD rop);
#define SetStretchBltMode GSSiSetStretchBltMode
int   WINAPI GSSiSetStretchBltMode(__in HDC hdc, __in int mode);
#define SetBrushOrgEx GSSiSetBrushOrgEx
BOOL  WINAPI GSSiSetBrushOrgEx( __in HDC hdc, __in int x, __in int y, __out_opt LPPOINT lppt);
#define SetGraphicsMode GSSiSetGraphicsMode
int   WINAPI GSSiSetGraphicsMode(__in HDC hdc, __in int iMode);
#define SetMapMode	GSSiSetMapMode
int   WINAPI GSSiSetMapMode(__in HDC hdc, __in int iMode);
#define SetViewportExtEx	GSSiSetViewportExtEx
BOOL  WINAPI GSSiSetViewportExtEx( __in HDC hdc, __in int x, __in int y, __out_opt LPSIZE lpsz);
#define SetViewportOrgEx	GSSiSetViewportOrgEx
BOOL  WINAPI GSSiSetViewportOrgEx( __in HDC hdc, __in int x, __in int y, __out_opt LPPOINT lppt);
#define SetWindowExtEx	GSSiSetWindowExtEx
BOOL  WINAPI GSSiSetWindowExtEx( __in HDC hdc, __in int x, __in int y, __out_opt LPSIZE lpsz);
#define SetWindowOrgEx	GSSiSetWindowOrgEx
BOOL  WINAPI GSSiSetWindowOrgEx( __in HDC hdc, __in int x, __in int y, __out_opt LPPOINT lppt);
#define SetWorldTransform	GSSiSetWorldTransform
BOOL WINAPI GSSiSetWorldTransform( __in HDC hdc, __in CONST XFORM * lpxf);
#define MoveWindow	GSSiMoveWindow
BOOL WINAPI GSSiMoveWindow(__in HWND hWnd,__in int X,__in int Y,__in int nWidth,__in int nHeight,__in BOOL bRepaint);
#define SetWindowPos GSSiSetWindowPos
BOOL WINAPI GSSiSetWindowPos(__in HWND hWnd,__in_opt HWND hWndInsertAfter,__in int X,__in int Y,__in int cx,__in int cy,__in UINT uFlags);
#define SetTimer	GSSiSetTimer
UINT_PTR WINAPI GSSiSetTimer( __in_opt HWND hWnd,__in UINT_PTR nIDEvent,__in UINT uElapse,__in_opt TIMERPROC lpTimerFunc);
#define KillTimer	GSSiKillTimer
BOOL WINAPI GSSiKillTimer(__in_opt HWND hWnd,__in UINT_PTR uIDEvent);
              
#if CHECKMEM        
#define PostMessageA GSSiPOSTMESSAGE 
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
void GSSiRemoveMem (HGLOBAL hglb);  

#endif

#define LoadBitmapA GSSiLOADBITMAP
HBITMAP WINAPI LoadBitmap(HINSTANCE hInst, LPCSTR Name);  
#define CreateBitmap GSSiCREATEBITMAP
HBITMAP WINAPI GSSiCREATEBITMAP (int i1, int i2, UINT i3, UINT i4, const void FAR* p);
#define CreateCompatibleBitmap GSSiCREATECOMPATIBLEBITMAP
HBITMAP WINAPI GSSiCREATECOMPATIBLEBITMAP (HDC hDC, int i1, int i2);
#define CreateDIBitmap GSSiCREATEDIBITMAP
HBITMAP WINAPI GSSiCREATEDIBITMAP(HDC hDC, BITMAPINFOHEADER FAR* pbi, DWORD i1, const void FAR* p, BITMAPINFO FAR* pb, UINT i2);
#define CreatePalette GSSiCREATEPALETTE
HPALETTE WINAPI GSSiCREATEPALETTE(const LOGPALETTE FAR*);
#define CreateDIBSection GSSiCREATEDIBSECTION
HBITMAP WINAPI GSSiCREATEDIBSECTION (HDC hDC,BITMAPINFO *lpbmi,UINT usage, VOID **ppvBits,HANDLE hSection,DWORD offset);

 

void GSSiGLOBALLOCCLOSE (void);

#pragma pack(1)
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
                HANDLE16  BTHandle[MAX_GMD_INDEXES16];
                short   NumIndexFields[MAX_GMD_INDEXES16], //neg indicates sparse index
                        IndexFields[MAX_GMD_INDEXES16][MAX_GMD_INDEX_FIELDS],
                        lKeys[MAX_GMD_INDEXES16]; 
                HANDLE16  hKeys[MAX_GMD_INDEXES16];
                LPSTR   pKeys[MAX_GMD_INDEXES16];
                long  TimeStamp;
                HANDLE16  hFldInfo;                       
                LPGWFLDINFO pFldInfo;
                char    GWDData[2];
                }   GWDHEADER16;
typedef GWDHEADER16 FAR  *LPGWDHEADER16;   
#pragma pack()
int checkvp(int i);

#include "TileGraphics.h"