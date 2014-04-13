/* QuickCase:W KNB Version 1.00 */
#include "graphint.h"  
#include "gps.h"
#include "garmin.h" 
#define GPSTIMER	10 

#include "gmextern.h"

#pragma	pack(1)

typedef long	tJulian;
typedef struct
    {
    unsigned char  mPacketType;
    unsigned char  mReserved1;
    unsigned short mReserved2;
    unsigned short mPacketId;
    unsigned short mReserved3;
    unsigned long  mDataSize;
    BYTE           mData[1];
    } Packet_t;
typedef Packet_t	FAR	*LPPACKET_T;

static	short	WptFormat=103, RteHdrFormat=201, RteWptFormat=103, RouteProtocol=200; 
static	char	Input[2048], InBuf[2][2048]={"",""};
static	short	lbuf[2]={0,1};     
static	short	TrkLogFormat;  

#define MOREDATA 10000
#define IDM_OPENSIO 10001
#define INVALIDPACKET	0   
#define VALIDPACKET		1 
#define	UNABLEPACKET	2  
#define USBStream		(HANDLE)-2

enum { EOM=-5, CHK_SUM, START_CHK_SUM, USE_ARGV, INSERT_LEN };  // Must not be 0..255

static short send_init[]={0x10,START_CHK_SUM,0xfe,0,CHK_SUM,0x10,3,EOM};
static short send_init_spare[]={START_CHK_SUM,0x10,0xfe,0,2,0x10,3,EOM};
static short send_alminac[]={0x10,START_CHK_SUM,0xa,2,1,0,CHK_SUM,0x10,3,EOM};
static short send_begin_ack[]={0x10,START_CHK_SUM,0x6,2,0xfe,0,CHK_SUM,0x10,3,EOM};
static short send_clock[]={0x10,START_CHK_SUM,0xa,2,5,0,CHK_SUM,0x10,3,EOM};// clock
static short send_position[]={0x10,START_CHK_SUM,0xa,2,2,0,CHK_SUM,0x10,3,EOM};// clock
//int send_bad_position[]={0x10,START_CHK_SUM,0x11,0x20,0,0,0,0,0,0,0xf8,0x3f,
//	0,0,0,0,0,0,0xf8,0x3f, CHK_SUM,0x10,3,EOM1};// faked position
static short send_request[]={0x10,START_CHK_SUM,Pid_Command_Data,2,USE_ARGV,0,CHK_SUM,0x10,3,EOM};
static short send_ack[]={0x10,START_CHK_SUM,Pid_Ack_Byte,2,USE_ARGV,0,CHK_SUM,0x10,3,EOM};
static short send_nak[]={0x10,START_CHK_SUM,Pid_Nak_Byte,2,USE_ARGV,0,CHK_SUM,0x10,3,EOM};
static short send_records[]={0x10,START_CHK_SUM,Pid_Records,2,USE_ARGV,0,CHK_SUM,0x10,3,EOM};
static short send_complete[]={0x10,START_CHK_SUM,Pid_Xfer_Cmplt,2,USE_ARGV,0,CHK_SUM,0x10,3,EOM};
static short send_wpt[]={0x10,START_CHK_SUM,Pid_Wpt_Data,INSERT_LEN,USE_ARGV,CHK_SUM,0x10,3,EOM};
static short send_route[]={0x10,START_CHK_SUM,Pid_Rte_Hdr,INSERT_LEN,USE_ARGV,CHK_SUM,0x10,3,EOM};
static short send_route_wpt[]={0x10,START_CHK_SUM,Pid_Rte_Wpt_Data,INSERT_LEN,USE_ARGV,CHK_SUM,0x10,3,EOM};
static short send_route_link_data[]={0x10,START_CHK_SUM,Pid_Rte_Link_Data,INSERT_LEN,USE_ARGV,CHK_SUM,0x10,3,EOM};

//Lowrance Stuff    
typedef	struct	{
					WORD	PreAmble,
							Command,
							Count;
					BYTE	ID,
							CHKSUM;
				}LSI100HEADER;
typedef	LSI100HEADER	FAR	*LPLSI100HEADER;
				
typedef	struct	{   BYTE	Reserved;
					WORD	Number;
					BYTE	Status,
							Symbol;
					LONG	Lat,Lon;
					char    Name[13];
					LONG	Date;
				}LSI100WAYPOINT;
typedef	LSI100WAYPOINT	FAR	*LPLSI100WAYPOINT;
				
typedef	struct	{   BYTE	Reserved;
					WORD	Number;
					BYTE	nWP;
					char    Name[13];
					WORD	WPNum[];
				}LSI100ROUTE;
typedef	LSI100ROUTE	FAR	*LPLSI100ROUTE;

typedef	struct	{   BYTE	Reserved;
					BYTE	PlotTrailNumber;
					LONG	OriginY,
							OriginX;
					WORD	NumberOfDeltas;
				}LSI100PLOTTRAIL;
typedef LSI100PLOTTRAIL	FAR	*LPLSI100PLOTTRAIL;  

typedef struct	{BYTE	PlotTrailNumber;
				 WORD	NumberOfDeltas;} LSI100PLOTTRAILREQUEST;

typedef struct	{short	DeltaY, DeltaX;} DELTA; 
typedef DELTA	FAR	*LPDELTA;
typedef	struct	{   BYTE	Reserved;
					BYTE	PlotTrailNumber;
					WORD	NumberOfDeltas;  
					DELTA	Deltas[];
				}LSI100PLOTTRAILDELTAS;
typedef LSI100PLOTTRAILDELTAS	FAR	*LPLSI100PLOTTRAILDELTAS;

#define LSI100PreAmble		0x8155
#define	LSI100WakeUp		0x000D
#define	LSI100GetProdInfo	0x030E  
#define LSI100GetWaypoint	0x0303
#define LSI100SendWaypoint	0x0304 
#define LSI100SendRoute		0x0306  
#define LSI100GetPlotTrail	0x0312
#define LSI100GetPlotTrailDeltas	0x0313
#define SecondsFrom1970to1992	694252800  
#define MAXDELTATRANSFER	100

/*0x0008 Read from a memory location
0x0009 Write to a memory location
0x000D Wake up external serial port
0x010A Change NMEA Baud Rate
0x0301 Request Screen Pointer to download the current Screen
0x0302 unfreeze the current screen (happens after downloading screen)
0x0303 Get a Waypoint
0x0304 Send a Waypoint
0x0305 Get a Route
0x0306 Send a Route
0x0307 Get plot trail Pointer
0x0308 Get number of icons
0x0309 Get icon symbol
0x030A Set number of icons
0x030B Send icon
0x030C Get number of Graphical symbols
0x030D Get Icon Graphical symbol
0x030E Get product information
0x0312 Get Plot Trail Origin (Protocol version 2.0)
0x0313 Get Plot Trail Deltas (Protocol version 2.0)
0x0314 Set Plot Trail Origin (Protocol version 2.0)
0x0315 Set Plot Trail Deltas (Protocol version 2.0)*/    
typedef struct{ BYTE	Reserved;
				WORD	ProductID,
						ProtocolVersion,
						ScreenType,
						ScreenWidth,
						ScreenHeight,
						NumOfWaypoints,
						NumOfIcons,
						NumOfRoutes,
						NumOfWaypointsPerRoute;
				BYTE	NumOfPlotTrails,
						NumOfIconSym,
						ScreenRotateAngle;
				LONG	RunTime;
				}LSI100ProductID;
/*
Item Description
ProductID This value will present the product identifier. These values are
defined as follows:
1 - GlobalMap
2 - AirMap
3 - AccuMap
4 - GlobalNav 310
5 - Eagle View
6 - Eagle Explorer
7 - GlobalNav 200
8 - Expedition II
9 - GlobalNav 212
10 - GlobalMap 12
12 - AccuMap 12
Protocol Version
0 - Version 1.0
1 - Version 2.0      */

static BOOL	TrackLogTransferSupported=FALSE;
static char LSI100ProductName[12][16]={
"GlobalMap",
"AirMap",
"AccuMap",
"GlobalNav 310",
"Eagle View",
"Eagle Explorer",
"GlobalNav 200",
"Expedition II",
"GlobalNav 212",
"",
"GlobalMap 12",
"AccuMap 12"};

//
short CheckForNMEA (HANDLE Stream);
void CreatePacket (LPSHORT msg,int Arg,LPBYTE Packet );
void CreateLSI100Packet (WORD cmd,int Arg,LPBYTE Packet );
short	PacketLength (LPBYTE Packet);
short	HaveCompletePacket (LPBYTE Packet,LPBYTE InPacket,LPSHORT pLength);
short StuffPacket (LPBYTE OutPacket,LPBYTE InPacket);
void UnStuffPacket (LPBYTE Packet);
BOOL GPSImportGarminWP (HWND hWndDlg,UINT Control,UINT StatusControl);
BOOL OpenGarmin (HWND hWnd,LPSTR Identity);
BOOL OpenMagellan (HWND hWnd,LPSTR Identity);
BOOL OpenLowrance (HWND hWnd,LPSTR Identity);
void CreateLSI100Header (WORD cmd,int lArg,LPLSI100HEADER pHead );
BOOL SendLSI100Header (HANDLE Stream,LPLSI100HEADER pHeader,LPBYTE pDataSnd,LPBYTE pDataRcv);
BOOL GPSImportLSI100WP (HWND hWndDlg,UINT Control,UINT StatusControl,BOOL SearchALL);
BOOL HaveCompleteLSI100Packet (LPBYTE InPacket,short InLength);
BOOL GetLSI100Data (LPBYTE InPacket,short InLength,WORD Command,LPBYTE pData);
BYTE ComputeNMEACheckSum (LPBYTE rec,int l);
short GPSExportGarminWP (HWND hWndDlg,UINT Control,UINT StatusControl);
short GPSExportLSI100WP (HWND hWndDlg,UINT Control,UINT StatusControl,BOOL SearchALL,LPHANDLE phWPNums,short RouteNum);
short GPSExportLSI100Route (HWND hWndDlg,UINT Control,UINT StatusControl,BOOL SearchALL);
short GPSExportGarminRoute (HWND hWndDlg,UINT Control,UINT StatusControl);
short GPSExportMagellanRoute (HWND hWndDlg,UINT Control,UINT StatusControl);
int SendMagellanPacket (LPSTR Packet);
short HaveCompleteMagellanPacket (LPBYTE InPacket,LPSHORT InLength,LPSTR Packet,LPBYTE pChkSum);
short HaveMagellanCheckSumPacket (LPBYTE InPacket,LPSHORT InLength,LPBYTE pChkSum);
short GetMagellanPacket (LPBYTE Packet);
BOOL GPSImportMagellanWP (HWND hWndDlg,UINT Control,UINT StatusControl);
short GPSExportMagellanWP (HWND hWndDlg,UINT Control,UINT StatusControl,BOOL Route);
BOOL GPSImportUSRWP (HWND hWndDlg,UINT Control,UINT StatusControl);
BOOL GPSImportGPXWP (HWND hWndDlg,UINT Control,UINT StatusControl);
short GPSExportUSRWP (HWND hWndDlg,UINT Control,UINT StatusControl);
BOOL GPSImportRaytheon (HWND hWndDlg,UINT Control,UINT StatusControl);
short GPSExportRaytheon (HWND hWndDlg,UINT Control,UINT StatusControl);
BOOL GPSImportTrackGarmin (HWND hWndDlg,UINT ListControl,UINT StatusControl);
BOOL GPSImportTrackMagellan (HWND hWndDlg,UINT Control,UINT StatusControl);
BOOL GPSImportTrackLSI100 (HWND hWndDlg,UINT ListControl,UINT StatusControl);
BOOL ConvertUSBPacketToSerialPacket (LPBYTE Packet,LPPACKET_T USBPacket);
BOOL ConvertSerialPacketToUSBPacket (LPBYTE Packet,LPPACKET_T USBPacket);
void CreateDataPacket (LPSHORT msg,LPBYTE pArg,int len,int PadZeros,LPBYTE Packet );


static	BYTE	InPacket[4096], USBPacket[1024];
static	short	InLength=0,MaxLowranceWP=0,MaxLowranceRouteWP,MaxLowrancePlotTrails,LowranceProtocolVersion ;
static	HANDLE	hLowranceWPList=0; 
static	short	LowranceScanOption; 
static	short	LowranceReplaceOpt=0;
static	char	USRFileName[256]="";
static	int		CommTimeout=200;

#define RADtoDEG 57.295779513082322
#define DEGtoRAD 0.017453292519943296
typedef struct
{
	double Lat;
	double Lon;
}LSTRUC; 
typedef struct  
{
	long Latitude;
	long Longitude;
}tMCoord;

typedef struct
{
	long TextLength;
	char String[1];
} tText;

enum tWaypointType
{
	WAYPOINT_TYPE_USER,
	WAYPOINT_TYPE_TEMPORARY,
	WAYPOINT_TYPE_POINT_OF_INTEREST,

	NUM_WAYPOINT_TYPES
};

#define SYMBOL_ALIAS_START 10000
enum SymbolTypes {
  SYMBOL_ALIAS_WAYPOINT_1 = SYMBOL_ALIAS_START, 
  SYMBOL_ALIAS_WAYPOINT_2,
  SYMBOL_ALIAS_WAYPOINT_3,
  SYMBOL_ALIAS_X_1,
  SYMBOL_ALIAS_X_2,
  SYMBOL_ALIAS_X_3,
  SYMBOL_ALIAS_CROSS,
  SYMBOL_ALIAS_HOUSE,
  SYMBOL_ALIAS_CAR,
  SYMBOL_ALIAS_STORE,
  SYMBOL_ALIAS_GAS,
  SYMBOL_ALIAS_FORK_SPOON,
  SYMBOL_ALIAS_PHONE,
  SYMBOL_ALIAS_PLANE,
  SYMBOL_ALIAS_EXIT,
  SYMBOL_ALIAS_STOP,
  SYMBOL_ALIAS_EXCLAMATION,
  SYMBOL_ALIAS_TRAFFIC_LIGHT,
  SYMBOL_ALIAS_FLAG,
  SYMBOL_ALIAS_MAN,
  SYMBOL_ALIAS_RESTROOMS,
  SYMBOL_ALIAS_TREE,
  SYMBOL_ALIAS_MOUNTAINS,
  SYMBOL_ALIAS_CAMPSITE,
  SYMBOL_ALIAS_PICNIC_TABLE,
  SYMBOL_ALIAS_DEER,
  SYMBOL_ALIAS_DEER_TRACKS,
  SYMBOL_ALIAS_TURKEY_TRACKS,
  SYMBOL_ALIAS_TREE_STAND,
  SYMBOL_ALIAS_BRIDGE,
  SYMBOL_ALIAS_SKULL,
  SYMBOL_ALIAS_FISH,
  SYMBOL_ALIAS_TWO_FISH,
  SYMBOL_ALIAS_DIVE_FLAG,
  SYMBOL_ALIAS_WRECK,
  SYMBOL_ALIAS_ANCHOR,
  SYMBOL_ALIAS_BOAT,
  SYMBOL_ALIAS_BOAT_RAMP,
  SYMBOL_ALIAS_FLAG_BUOY,
  SYMBOL_ALIAS_DAM,
  SYMBOL_ALIAS_SWIMMER,
  SYMBOL_ALIAS_PIER,

  SYMBOL_ALIAS_BROKEN,
  SYMBOL_ALIAS_TEMPORARY,

  SYMBOL_HUNT_ATV,
  SYMBOL_HUNT_GROUNDBLIND,
  SYMBOL_HUNT_OAKTREE,
  SYMBOL_HUNT_WINDMILL,
  SYMBOL_HUNT_CAMERA,
  SYMBOL_HUNT_RUB,
  SYMBOL_HUNT_SCRAPE,
  SYMBOL_HUNT_DROPPING,
  SYMBOL_HUNT_ROOSTTREE,
  SYMBOL_HUNT_ANIMALBED,
  SYMBOL_HUNT_FEEDER,
  SYMBOL_HUNT_FOODPLOT,
  SYMBOL_HUNT_TURKEY,
  SYMBOL_HUNT_DUCK,
  SYMBOL_HUNT_UPLAND,
  SYMBOL_HUNT_RABBIT,
  SYMBOL_HUNT_PAW,
  SYMBOL_HUNT_BLOOD,

  SYMBOL_GEOCACHE_EARTH_CACHE,
  SYMBOL_GEOCACHE_EARTH_FLAG,
  SYMBOL_GEOCACHE_EVENT,
  SYMBOL_GEOCACHE_GHOST,
  SYMBOL_GEOCACHE_LETTER,
  SYMBOL_GEOCACHE_MULTI,
  SYMBOL_GEOCACHE_QUESTION,
  SYMBOL_GEOCACHE_TRADITION,
  SYMBOL_GEOCACHE_WEBCAM,

  SYMBOL_ALIAS_SUN,
  SYMBOL_ALIAS_NOTE,
  SYMBOL_ALIAS_CAMERA,
  SYMBOL_ALIAS_STAR,
  SYMBOL_ALIAS_MUG,
  SYMBOL_ALIAS_BOOKS,
  SYMBOL_ALIAS_HISTORICAL,
  SYMBOL_ALIAS_TOOLS,
  SYMBOL_ALIAS_HEART,
  SYMBOL_ALIAS_ARENA,
  SYMBOL_ALIAS_GOLF,
  SYMBOL_ALIAS_MONEY
}; // Waypoint Symbols enumerate.


typedef struct 
{
	tMCoord Position;
	long Altitude;
	tText Name;

	// A comment associated with the waypoint - may be shown in the arrival alarm when the waypoint is 
	// reached while navigating a route (not yet used by head unit software)
	tText Comment;

	tJulian Time;
	int SymbolID;
	unsigned short Type;	
} tWaypoint;


typedef struct
{
	tMCoord Position;
	int SymbolID;
} tIcon;

typedef struct
{
	tText RouteName;
	short NumLegs;
	BOOL RouteReversed;

	tWaypoint Legs[1];
} tRoute;

typedef struct
{
	tText TrailName;
	BOOL TrailVisible;
	short NumTrailPoints;
	short MaxTrailSize;

//	tTrailSection TrailSections[...];
} tTrail;

typedef struct
{
	tMCoord Point;

	// If true, this trail point is connected to the previous point. If false, then there was a position
	// loss in between this point and the one previous to it, so they should not be connected with a 
	// straight line.
	BOOL Continuous;
} tTrailPoint;

typedef struct
{
	short NumSectionPoints;
	tTrailPoint Points[1];
} tTrailSection;

#pragma	pack()

HANDLE OpenComm (LPSTR CommID,int Inbuf,int Outbuf)
{
	HANDLE Stream;
	DWORD	Err=0;
	COMMTIMEOUTS	CommTimeOuts;
	char	Port[16];

	sprintf (Port,"\\.\\%s",CommID);
	
    Stream = CreateFile(Port, GENERIC_READ | GENERIC_WRITE,
                  0,                    // exclusive access
                  NULL,                 // no security attrs
                  OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL,// | FILE_FLAG_OVERLAPPED, // overlapped I/O
                  NULL );
	if (Stream != (HANDLE)-1)
	{
      // get any early notifications

        SetCommMask(Stream, EV_RXCHAR ) ;
		SetupComm (Stream, Inbuf,Outbuf ) ;
      // purge any information in the buffer

		PurgeComm(Stream, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
		InLength = 0;
      // set up for overlapped I/O

		CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF ;
		CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
		CommTimeOuts.ReadTotalTimeoutConstant = CommTimeout ;
		// CBR_9600 is approximately 1byte/ms. For our purposes, allow
		// double the expected time per character for a fudge factor.
//		CommTimeOuts.WriteTotalTimeoutMultiplier = 2*CBR_9600/BAUDRATE( npTTYInfo ) ;
		CommTimeOuts.WriteTotalTimeoutConstant = 0 ;
		SetCommTimeouts(Stream, &CommTimeOuts ) ;

	}
	else
		Err = GetLastError ();
	return Stream;
}
/*      // get any early notifications

      SetCommMask( COMDEV( npTTYInfo ), EV_RXCHAR ) ;

      // setup device buffers

      SetupComm( COMDEV( npTTYInfo ), 4096, 4096 ) ;

      // purge any information in the buffer

      PurgeComm( COMDEV( npTTYInfo ), PURGE_TXABORT | PURGE_RXABORT |
                                      PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

      // set up for overlapped I/O

      CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF ;
      CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
      CommTimeOuts.ReadTotalTimeoutConstant = 1000 ;
      // CBR_9600 is approximately 1byte/ms. For our purposes, allow
      // double the expected time per character for a fudge factor.
      CommTimeOuts.WriteTotalTimeoutMultiplier = 2*CBR_9600/BAUDRATE( npTTYInfo ) ;
      CommTimeOuts.WriteTotalTimeoutConstant = 0 ;
      SetCommTimeouts( COMDEV( npTTYInfo ), &CommTimeOuts ) ;
   }

   fRetVal = SetupConnection( hWnd ) ;

   if (fRetVal)
   {
      CONNECTED( npTTYInfo ) = TRUE ;

      // Create a secondary thread
      // to watch for an event.

      if (NULL == (hCommWatchThread =
                      CreateThread( (LPSECURITY_ATTRIBUTES) NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE) CommWatchProc,
                                    (LPVOID) npTTYInfo,
                                    0, &dwThreadID )))
      {
         CONNECTED( npTTYInfo ) = FALSE ;
         CloseHandle( COMDEV( npTTYInfo ) ) ;
         fRetVal = FALSE ;
      }
      else
      {
         THREADID( npTTYInfo ) = dwThreadID ;
         HTHREAD( npTTYInfo ) = hCommWatchThread ;

         // assert DTR

         EscapeCommFunction( COMDEV( npTTYInfo ), SETDTR ) ;

*/
BOOL CloseComm (HANDLE Stream)
{
	if (Stream != (HANDLE)-1)
		CloseHandle (Stream);
	return TRUE;
}
	
BOOL FoundModem (HANDLE Stream)
{   

	char	AreYouAModem[6]="ate1\r\n"; 
	char	Mess[64]="";
	short	ln;

	WriteComm(Stream,AreYouAModem,_fstrlen(AreYouAModem)); 
	Sleep (500);  
	if ((ln=ReadComm( Stream,Mess, 64))>0)
	{
		Mess[ln]=0;
		if (strstr (Mess,"OK\r\n"))
		return TRUE; 
	}
	
	return FALSE;
} 

LSTRUC Merc_2_Degree(long lat, long lon)
{
	LSTRUC T;
	double temp;
	temp = (double)lat / (double) 6356752.3142;
	temp = exp(temp);
	temp = (2 * atan(temp)) - (PY/2);
	T.Lat = temp * RADtoDEG;
	T.Lon = (double)lon / (double) 6356752.3142;
	T.Lon = T.Lon * RADtoDEG;
	return T;
};

typedef struct {long Lat,Lon;} Merc;
Merc Degree_2_Merc(double lat, double lon)
{
	Merc T;
	double Temp;
	Temp = ((lat * DEGtoRAD) + (PY/2))/2;
	Temp = tan(Temp);
	Temp = log(Temp);
	Temp = Temp * 6356752.3142;
	T.Lat = (long) Temp;
	T.Lon = ( long) ((6356752.3142) * (lon * DEGtoRAD));
	return T;
}; 

BOOL GPSImportWP (HWND hWndDlg,UINT Control,UINT StatusControl,BOOL SearchALL)
{
	switch (GPSFormat)
	{
		case 1000:
			return GPSImportGarminWP (hWndDlg,Control,StatusControl);
		case 2000:
			return GPSImportMagellanWP (hWndDlg,Control,StatusControl);
		case 3000:
			return GPSImportLSI100WP (hWndDlg,Control,StatusControl,SearchALL);
//		case 4000:
//			return GPSImportRaytheon (hWndDlg,Control,StatusControl);
		case 5000:
			return GPSImportUSRWP (hWndDlg,Control,StatusControl);
		case 6000:
			return GPSImportGPXWP (hWndDlg,Control,StatusControl);
	}
	return FALSE;
}	

short GPSExportWP (HWND hWndDlg,UINT Control,UINT StatusControl,BOOL SearchALL)
{
	switch (GPSFormat)
	{
		case 1000:
			return GPSExportGarminWP (hWndDlg,Control,StatusControl);
		case 2000:
			return GPSExportMagellanWP (hWndDlg,Control,StatusControl,FALSE);
		case 3000:
			return GPSExportLSI100WP (hWndDlg,Control,StatusControl,SearchALL,NULL,0);
//		case 4000:
//			return GPSExportRaytheon (hWndDlg,Control,StatusControl);
		case 5000:
			return GPSExportUSRWP (hWndDlg,Control,StatusControl);
	}
	return FALSE;
}

short GPSExportRoute (HWND hWndDlg,UINT Control,UINT StatusControl,BOOL SearchALL)
{
	switch (GPSFormat)
	{
		case 1000:
			return GPSExportGarminRoute (hWndDlg,Control,StatusControl);
		case 2000:
			return GPSExportMagellanRoute (hWndDlg,Control,StatusControl);
		case 3000:
			return GPSExportLSI100Route (hWndDlg,Control,StatusControl,SearchALL);
		default:
			GSSiMessageBox ("Route transfer not yet supported with your GPS",NULL,MB_ICONEXCLAMATION,0);
	}
	return FALSE;
}

short HaveMagellanCheckSumPacket (LPBYTE InPacket,LPSHORT InLength,LPBYTE pChkSum)
{   
	LPSTR	pBeg=InPacket, pEnd;   
	short	l;
	int	iChk;
	
	InPacket[*InLength] = 0; 
Top:
	pBeg = _fstrchr (pBeg,'$');
	if (!pBeg)
		return INVALIDPACKET;
	pEnd = _fstrchr (pBeg,'\n');  
	if (!pEnd)
		return INVALIDPACKET; 
	if (!_fstrncmp(pBeg,"$PMGNCMD,UNABLE",15))
		return UNABLEPACKET;
	if (_fstrnicmp (pBeg,"$PMGNCSM,",9)) 
		return INVALIDPACKET;
	pBeg +=9;    
	sscanf (pBeg,"%X",&iChk);   
	*pChkSum = iChk;
	pEnd++;
	l = pEnd - InPacket;
	if (l>0)
	{
		(*InLength) -= l;
		_fmemmove (InPacket,pEnd,*InLength);
	}
	return  VALIDPACKET;
}
	
short HaveCompleteMagellanPacket (LPBYTE InPacket,LPSHORT InLength,LPSTR Packet,LPBYTE pChkSum)
{   
	LPSTR	pBeg=InPacket, pEnd, pCk;
	short	l;
	int		iChk;
	
	InPacket[*InLength] = 0; 
Top:
	pBeg = _fstrchr (pBeg,'$');
	if (!pBeg)
		return INVALIDPACKET;
	pEnd = _fstrchr (pBeg,'\n');  
	if (!pEnd)
		return INVALIDPACKET;
	if (!_fstrnicmp (pBeg,"$PMGNCSM,",9)) 
	{
		pBeg = pEnd + 1; 
		goto Top;
	}  
	if ((pCk = _fstrrchr (pBeg,'*'))) 
	{
		*pCk++ = 0;   
		sscanf (pCk,"%X",&iChk); 
		*pChkSum = iChk;
	}
	_fstrcpy (Packet,pBeg);   
	pEnd++;
	l = pEnd - InPacket;
	if (l>0)
	{
		(*InLength) -= l;
		_fmemmove (InPacket,pEnd,*InLength);
	}
	return  VALIDPACKET;
}
	
BOOL HaveCompleteLSI100Packet (LPBYTE InPacket,short InLength)
{   
	LPLSI100HEADER pHeader=(LPLSI100HEADER)InPacket; 
	LPBYTE	pData=&InPacket[8];
	short	lData; 
	
	if (InLength < 8)
		return FALSE;
	if (ComputeCheckSum (InPacket,7) != pHeader->CHKSUM)
		return FALSE; 
	lData = InLength - 8 - 1;
	if (lData < (short)pHeader->Count)
		return FALSE; 
	if (ComputeCheckSum (pData,pHeader->Count) != pData[pHeader->Count])
		return FALSE;
	return TRUE;
}

BOOL GetLSI100Data (LPBYTE InPacket,short InLength,WORD Command,LPBYTE pDataOut)
{
	LPLSI100HEADER pHeader=(LPLSI100HEADER)InPacket; 
	LPBYTE	pData=&InPacket[8];
    
    _fmemmove (pDataOut,pData,pHeader->Count);
	return TRUE;
}

void GPSClose(LPHANDLE pStream)
{
	   
	if (*pStream == USBStream)
		CloseGarminUSB(0);
	else
		CloseSIOConnection(*pStream);
	*pStream = (HANDLE)-1;
	return;
}

BYTE ComputeNMEACheckSum (LPBYTE rec,int l)
{
	BYTE	csum=0;	
	
	rec++;
	l--;       
	while(l--) 
	{
		if (*rec == '*')
			break;
		csum ^= *rec++;
	}
	return csum; 
}

BOOL ValidChecksum (LPBYTE Packet)
{
	short	chksum=Packet[1], l=Packet[2]+1;  
	BYTE	csum;	
	Packet+=2;
	
	while(l--)
		chksum += *Packet++;
		
	csum = 256-(chksum&0xff); 
	if (csum == *Packet)
		return TRUE;
	return FALSE;
}

void SendACK (HANDLE Stream,short PacketID)
{  
	BYTE	Packet[512];
	short	ii;
	
	CreatePacket (send_ack,PacketID,Packet);   
	ii=WriteComm(Stream,Packet,PacketLength(Packet));

	return;
}

void SendNAK (HANDLE Stream,short PacketID)
{  
	BYTE	Packet[512];
	short	ii;
	
	CreatePacket (send_nak,PacketID,Packet);   
	ii=WriteComm(Stream,Packet,PacketLength(Packet));

	return;
}

void CreatePacket (LPSHORT msg,int Arg,LPBYTE Packet )
{   
	short	chksum=0;
	
	while(*msg !=EOM)
	{
		switch (*msg)
		{
			case USE_ARGV:
				chksum+=Arg;
				*Packet++ = Arg;
				break;  
				
			case CHK_SUM:
				*Packet++ = 256-(chksum&0xff); 
				break;
				
			case START_CHK_SUM:
				chksum=0;
				break;
				
			default:  
				*Packet++ = *msg;
				chksum+=*msg;
		} 
		msg++;
	}
	return;
}          

void CreateDataPacket (LPSHORT msg,LPBYTE pArg,int len,int PadZeros,LPBYTE Packet )
{   
	short	chksum=0, l=len+PadZeros,i;
	
	for (i=0;i<PadZeros;i++)
		pArg[len+i] = 0;
	
	while(*msg !=EOM)
	{
		switch (*msg)
		{   
			case INSERT_LEN: 
				*Packet = len+PadZeros;
				chksum += *Packet++;
				break;
				
			case USE_ARGV:
				while (l--)
				{
					chksum+=*pArg;
					*Packet++ = *pArg++; 
				}
				break;  
				
			case CHK_SUM:
				*Packet++ = 256-(chksum&0xff); 
				break;
				
			case START_CHK_SUM:
				chksum=0;
				break;
				
			default:  
				*Packet++ = *msg;
				chksum+=*msg;
		} 
		msg++;
	}
	return;
}          

short	PacketLength (LPBYTE Packet)
{
	short	l=Packet[2];
	
	return l+6;
}

short StuffPacket (LPBYTE OutPacket,LPBYTE InPacket)
{
	short	l,n=0;
	
	*OutPacket++ = *InPacket++;
	*OutPacket++ = *InPacket++;
	l = *InPacket + 2;
	while (l--)
	{
		if (*InPacket == 16)
		{
			n++;
			*OutPacket++ = 16; 
		}
		*OutPacket++ = *InPacket++;
	}
	*OutPacket++ = *InPacket++;
	*OutPacket++ = *InPacket++;
	return n;
}   

void UnStuffPacket (LPBYTE InPacket)
{
	short	l,n=0;
	LPBYTE	OutPacket=InPacket+2;
	
	InPacket = OutPacket;
	l = *InPacket + 4;
	while (l--)
	{
		if (*InPacket == 16)
			InPacket++; 
		*OutPacket++ = *InPacket++;
	}
	return;
}   

short GetnStuff (LPBYTE InPacket,int Length)
{
	short	n=0, last=0; 
	
	while (Length--)
	{
		if (*InPacket == 16 && last == 16)  
		{
			n++; 
			//Length++;
			last = 0;
		}
		else
			last = *InPacket;
		InPacket++;
	}
	return n;
}   

short	HaveCompletePacket (LPBYTE Packet,LPBYTE InPacket,LPSHORT pLength) //also unstuffs packet
{   
	short	PacketID, plength, nstuff,l;
	
	if (*pLength < 6)
		return 0;
    if (InPacket[0] != 16)
    	return 0; 
    PacketID = InPacket[1];
    plength = InPacket[2]; 
    if (plength + 6 > *pLength)
    	return 0;
	nstuff = GetnStuff (InPacket,plength+6);
	if (Packet)
		memmove (Packet,InPacket,plength+6+nstuff);
	l = *pLength - (plength+6+nstuff);
	if (l)
		memmove (InPacket,&InPacket[plength+6],l);
	*pLength = l;
	if (Packet)
	{
		if (Packet[4+plength+nstuff] != 16 || Packet[5+plength+nstuff] != 3)
    		return 0;
	}
	return PacketID;

}

BOOL ConvertSerialPacketToUSBPacket (LPBYTE Packet,LPPACKET_T USBPacket)
{   
	USHORT	i;
	
	USBPacket->mPacketType = 20;
	USBPacket->mReserved1 = 0;
	USBPacket->mReserved2 = 0;
	USBPacket->mReserved3 = 0;
	USBPacket->mPacketId = Packet[1];
	USBPacket->mDataSize = Packet[2]; 
	for (i=0;i<USBPacket->mDataSize;i++)
		USBPacket->mData[i] = Packet[i+3]; 

	return TRUE;
}

BOOL ConvertUSBPacketToSerialPacket (LPBYTE Packet,LPPACKET_T USBPacket)
{   
	USHORT	i;
	
	Packet[1] = (BYTE)USBPacket->mPacketId;
	Packet[2] = (BYTE)USBPacket->mDataSize;   
	if (!USBPacket->mDataSize)
		return FALSE;
	for (i=0;i<USBPacket->mDataSize;i++)
		Packet[i+3] = USBPacket->mData[i];   // USBPacket->mData[66]
	return TRUE;
}

BOOL SendPacket (HANDLE Stream,LPBYTE Packet)
{   
	BOOL	rtn;
	
	if (Stream == USBStream) 
	{
		ConvertSerialPacketToUSBPacket (Packet,(LPPACKET_T)USBPacket);
		rtn = SendPacketUSB (USBPacket);
	}
	else
		rtn = SendPacketSerial (Stream,Packet);
	return rtn;
}

BOOL SendPacketSerial (HANDLE Stream,LPBYTE Packet)
{
    BOOL	rtn=FALSE;
 	int        nError, nLength=0,ii, LenInput ;
	COMSTAT    ComStat ;
	MSG        msg ; 
	BOOL	HaveMsg;  
	BOOL	Continue=TRUE;
	short	PacketID, numstuff; 
    HANDLE	hMem=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
    LPBYTE	OutPacket=GlobalLock (hMem);
	int		TotTime = 0;
   
	//ReadComm( Stream, &InPacket[InLength], 512 ); 
	numstuff = StuffPacket (OutPacket,Packet);
	InLength = 0;
	ii=WriteComm(Stream,OutPacket,PacketLength(OutPacket)+numstuff);
//	SetTimer (hWndMain,GPSTIMER,GPSTimeOut,NULL);
    HaveMsg = TRUE;
	while (TotTime < GPSTimeOut)
	{
		nLength = ReadComm(GPSStream, &InPacket[InLength], 1024);  
		if (nLength > 0)
		{
			InLength += nLength;
			if (InLength > 1024*3)
				goto Exit;
			TotTime = 0;
		}
		if ((PacketID = HaveCompletePacket (0,InPacket,&InLength)))
		{
			if (PacketID == Pid_Ack_Byte)
			{
				rtn = TRUE; 
				goto Exit;
			}
		} 
		TotTime += CommTimeout;
    }
Exit:
	GSSiGlobUlFree (&hMem);
	return rtn;
}
 
short GetPacket (HANDLE Stream,LPBYTE Packet)
{
	short	rtn;
	
	if (Stream == USBStream) 
	{
		if (GetPacketUSB (USBPacket))  
		{
			if (ConvertUSBPacketToSerialPacket (Packet,(LPPACKET_T)USBPacket)) 
				rtn = Packet[1]; 
			else
				return FALSE;
		}
		else
			return FALSE;
	}
	else
		rtn = GetPacketSerial (Stream,Packet);
	return rtn;
}

short GetPacketSerial (HANDLE Stream,LPBYTE Packet)
{
	int        nError, nLength=1,ii, LenInput ;
	COMSTAT    ComStat ;
	MSG        msg ; 
	BOOL	HaveMsg;  
	BOOL	Continue=TRUE;
	short	PacketID;   
    short	rtn=FALSE;
	int		TotTime;
 
Top:
	TotTime = 0;
	while (TotTime < GPSTimeOut)
	{
		if ((PacketID = HaveCompletePacket (Packet,InPacket,&InLength)))
		{
//			KillTimer (hWndMain,GPSTIMER); 
			UnStuffPacket (Packet);
			if (PacketID == INVALIDPACKET || !ValidChecksum (Packet))
			{
				SendNAK (Stream,PacketID);
				goto Top;
			}
			SendACK (Stream,PacketID);
			rtn = PacketID;
			goto Exit;
		} 
		nLength = ReadComm(GPSStream, &InPacket[InLength], 1024); 
		if (nLength > 0)
		{
			InLength += nLength;
			TotTime = 0;
		}
		TotTime += CommTimeout;
	}
/*    
Top:
	SetTimer (hWndMain,GPSTIMER,GPSTimeOut,NULL);
    HaveMsg = TRUE;
    while (Continue)
    {   
    	if (nLength>=0)
    		msg.message = MOREDATA;
    	else
			Continue = GetMessage( &msg, NULL, 0,0);
		switch (msg.message)
		{   
			case WM_TIMER:
				if (msg.wParam != GPSTIMER)
					break;
				KillTimer (hWndMain,GPSTIMER);
				goto Exit;
				
			default:
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			    break;
			    
			case WM_COMMNOTIFY:
      			//if (CN_EVENT & LOWORD( msg.lParam ) != CN_EVENT)
      				break;
      		
            case MOREDATA:
			     GetCommEventMask( Stream, EV_RXFLAG ) ;
				 if (InLength < 512 && (nLength = ReadComm( Stream, &Packet[InLength], 512 ))>0)
				 { 
				 	InLength+=nLength; 
				 	if ((PacketID = HaveCompletePacket (Packet,InLength)))
				 	{
						KillTimer (hWndMain,GPSTIMER); 
						UnStuffPacket (Packet);
						if (PacketID == INVALIDPACKET || !ValidChecksum (Packet))
						{
							SendNAK (Stream,PacketID);
							goto Top;
						}
						SendACK (Stream,PacketID);
				 		rtn = PacketID;
				 		goto Exit;
				 	} 
				 }
				 else
				 	nLength = -1;
			}
      }
	KillTimer (hWndMain,GPSTIMER); 
*/
Exit:
	return rtn;
}

int LogCommIO (int From,LPBYTE Buffer,int BytesRead)
{
	static	BOOL	TraceComm=FALSE;

	if (TraceComm)
	{
		char	Buff[1024],str[1024];
		int	i=0;

		if (From != 3 && BytesRead > 0)
			for (i=0;i<BytesRead;i++)
				Buff[i] = Buffer[i];
		Buff[i] = 0;
		switch (From)
		{
		case 1:
			sprintf (str,"Read %i:%s",BytesRead,Buff);
			AppendFile ("c:\\commtrace.txt",str);
			break;
		case 2:
			sprintf (str,"Write %i:%s",BytesRead,Buff);
			AppendFile ("c:\\commtrace.txt",str);
			break;
		case 3:
			sprintf (str,"Baud rate:%i",BytesRead);
			AppendFile ("c:\\commtrace.txt",str);
			break;
		}

	}
	return BytesRead;
}

int ReadComm (HANDLE Stream,LPBYTE Buffer,int MaxRead)
{
	int BytesRead;

	if (Stream == (HANDLE)-1)
		return -1;
	if (!CheckForContinue (TRUE,0))
	{
	//MessageBox (0,"Checkforcont return",0,MB_OK);
		return 0;
	}
	if (ReadFile(Stream,Buffer,MaxRead,&BytesRead,0))
		return LogCommIO (1,Buffer,BytesRead);
	MessageBox (0,"Read returns -1",0,MB_OK);
	return -1;
};
 
int WriteComm (HANDLE Stream,LPBYTE Buffer,int MaxRead)
{
	int BytesRead;

	if (Stream == (HANDLE)-1)
		return -1;
	if (WriteFile(Stream,Buffer,MaxRead,&BytesRead,0))
		return LogCommIO (2,Buffer,BytesRead);
	MessageBox (0,"Write returns -1",0,MB_OK);
	return -1;
};

int GetCommEventMask(HANDLE Stream,int EvtToClear)
{
	return 0;
}
 
short CheckForNMEA (HANDLE Stream)
{
	BYTE	ChkSum; 
	BOOL	HaveMsg;  
	BOOL	Continue=TRUE, HaveHeader=FALSE;
	short	InLength=0, PacketID, numstuff; 
	HCURSOR	hcurSave;   
	short	rtn=0;   
	int		nRead=0; 
	int		st;
	HANDLE	hMem=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
	LPBYTE	InPacket = GlobalLock (hMem); 
	
    
//    SetWindowText (hWndMain,"Checking for NMEA");
//	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 

	Sleep (1500);
//	GSSiSetCursor (0);
Top:
	InLength = ReadComm( Stream,InPacket, 1000 ); 
	if (InLength < 1 || nRead > 3)   
		goto Exit;
	nRead++; 
	if (!_fstrstr (InPacket,"$GP"))
		goto Top;
	st = GSSiMessageBox ("The GPS receiver is transmitting NMEA data.\r\nPlease switch to data transfer mode.",NULL,MB_ICONEXCLAMATION|MB_OKCANCEL,0);
	if (st == IDOK)
		rtn = 1;
	else
		rtn = 2;
Exit:
	GSSiGlobUlFree (&hMem);
	
	return rtn;	
}

void GetGPSModels (HWND hWndDlg,UINT Control)
{ 
	FillCBList (hWndDlg,Control,"[%DL]gpsunits.txt",-1,0);

//	SendDlgItemMessage (hWndDlg,Control,CB_ADDSTRING,0,(LPARAM)"Garmin GPS 12 XL"); 
//	SendDlgItemMessage (hWndDlg,Control,CB_ADDSTRING,0,(LPARAM)"Magellan GPS ColorTRAK"); 
    return;
} 

short GetGPSFormat (LPSTR Model)
{
 	short fmt = GetListNum ("[%DL]gpsunits.txt",Model);
	return fmt;
}

void GetGPSPorts (HWND hWndDlg,UINT Control)
{   
	char	str[8];
	short	i;
	
	SendDlgItemMessage (hWndDlg,Control,CB_ADDSTRING,0,(LPARAM)"USB");
	for (i=1;i<49;i++)  
	{
		sprintf (str,"COM%i",i);
		SendDlgItemMessage (hWndDlg,Control,CB_ADDSTRING,0,(LPARAM)str);
	} 
    return;
}

void GetGPSBaud (HWND hWndDlg,UINT Control)
{
	SendDlgItemMessage (hWndDlg,Control,CB_ADDSTRING,0,(LPARAM)"4800"); 
	SendDlgItemMessage (hWndDlg,Control,CB_ADDSTRING,0,(LPARAM)"9600"); 
	SendDlgItemMessage (hWndDlg,Control,CB_ADDSTRING,0,(LPARAM)"19200"); 
    return;
}  

BOOL ProcessPidProtocolArray (LPBYTE Packet)
{   
	short	lData = Packet[2]; 
	char	TAG;
	LPSHORT	pVal;
	LPBYTE	pData = Packet + 3; 
	LPBYTE	pCheckSum = Packet + lData; 
	short	AType=0,PType,LType;
	
	while (pData < pCheckSum) 
	{
		TAG = *pData++;
		pVal = (LPSHORT)pData++;
		pData++;   
		switch (TAG)
		{   
			case 'P':
				PType = *pVal; 
			break;
			case 'L':
				LType = *pVal; 
			break;
			case 'A':
				AType = *pVal; 
				switch (AType)
				{
					case 200:
						RouteProtocol = 200;
						break;
					case 201:
						RouteProtocol=201;
						AType = 200;
						break;
					case 300:
						TrackLogTransferSupported = TRUE; 
						break;
				}
			break;
			
			case 'D':  
				if (*pVal >= 100 && *pVal <  200) 
				{
					if (AType == 100)
						WptFormat = *pVal;
					else if (AType == 200)
						RteWptFormat = *pVal;   
					else if (AType == 300)
						TrkLogFormat = *pVal;
				}
				else if (AType == 200 && *pVal >= 200 && *pVal <  210)
					RteHdrFormat = *pVal;
			break;    
			
			default:  
			break;
		}					
			
	}
	return TRUE;
} 

BOOL GPSOpen(HWND hWnd,LPSTR Identity,int Format,LPSTR ForcePort,LPSTR ForceBaud,BOOL ShowError)
{
	char	Port[32], Model[128]; 
	BOOL	UseEvtChr=TRUE, rtn=FALSE;
	BYTE	EvtChr=3;
	HCURSOR	hcurSave;     
	short	ii;

	KillTimer (hWndMain,GF_PAN_ZOOM_TARGET);
	
	if (hWnd && ShowError)
		EnableWindow (hWnd,FALSE);
//	hWndMain = hWnd;  
	CloseDigConnection(); 
Top:  
	InLength = 0;
	GPSClose (&GPSStream);      
	if (Format)
	{
		_fstrcpy (Port,ForcePort);
		GPSFormat = Format;       
	}
	else
	{
		GetPrivateProfileString ("GPS","Model","",Model,100,GMIni);
		GetPrivateProfileString ("GPS","Port","",Port,32,GMIni); 
		GPSFormat = GetGPSFormat (Model); 
	}
	switch (GPSFormat)
	{   
		default:
		case 1000:
			break; 
		case 4000:   
		case 2000:
			EvtChr = '\n';
			break;
		case 3000:
			UseEvtChr=FALSE;
			break; 
		case 5000:  
			_fstrcpy (Identity,"Lowrance USR Input");
			rtn = TRUE;  
			goto Exit;
		case 6000:  
			_fstrcpy (Identity,"GPX Input");
			rtn = TRUE;  
			goto Exit;
	}
	if (!_fstricmp (Port,"USB"))
	{
		GPSStream = USBStream;
//		if (!OpenGarminUSB (Identity))
//			ii=1;
	}
	else 
	{
		GPSStream = OpenSIOConnection(hWnd,Port,0,UseEvtChr,EvtChr,ForceBaud,ShowError);
		if (GPSStream == (HANDLE)-1)
			goto Exit;
		PurgeComm(GPSStream, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
//		GetCommError (GPSStream,NULL);
//	    GetCommEventMask(GPSStream, EV_RXFLAG|EV_ERR|EV_BREAK );
		*Identity = 0;  
		switch (CheckForNMEA (GPSStream))
		{
			case 1:
				goto Top;
			case 2: 
				rtn = -1;
				goto Exit;
		}
	}
    if (!Identity)
    	return TRUE;
//	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT));   
	

	switch (GPSFormat)
	{   
		default:
		case 1000:
			rtn = OpenGarmin (hWnd,Identity);    
			break;
		case 2000:
			rtn = OpenMagellan (hWnd,Identity);
			break;
		case 3000:
			rtn = OpenLowrance (hWnd,Identity);   
			break; 
		case 4000: 
		case 5000:
		case 6000:
			rtn = TRUE;
			break;
	}
	if (!rtn && ShowError)
	{
		if (MessageBox (hWnd,"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
			NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
		goto Top; 
	}
Exit: 
	if (hWnd)
		EnableWindow (hWnd,TRUE);
//	GSSiSetCursor (0); 
	if (rtn <=0)
		GPSClose (&GPSStream);  

	return rtn;
}  

BOOL OpenMagellan (HWND hWnd,LPSTR Identity)
{   
	BYTE	ChkSum; 
	short	l=0, PacketID; 
    LPSTR	lpCmd, lpVer;
	HCURSOR	hcurSave; 
	int		ii;
    HANDLE	hMem=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
    LPBYTE	Mess=GlobalLock (hMem);   
    BOOL	rtn=FALSE;

	Sleep (500);  
Top:
	ii=ReadComm( GPSStream,Mess, 1000 ); 
	WriteComm(GPSStream,"$$",2); 
	ii=ReadComm( GPSStream,Mess, 1000 ); 
	SendMagellanPacket ("HANDON");
	Sleep (500);  
	//MessageBox (0,"Sending",0,MB_OK);
	if ((PacketID = SendMagellanPacket ("VERSION")) != VALIDPACKET)
		goto Exit; 
	//MessageBox (0,"Got",0,MB_OK);
	Sleep (500);  
	if (!(l=GetMagellanPacket (Mess)))
	{ 
		goto Exit;
	} 
	SetWindowText (hWndMain,Mess);
	if ((lpCmd=_fstrstr(Mess,"$PMGNVER")))
	{
		UINT	SaveT = GPSTimeOut; 
		
		lpVer = _fstrrchr (lpCmd,',');
		_fstrcpy (Identity,++lpVer);
		Sleep (500);
		GPSTimeOut=400;  
		GetMagellanPacket (Mess); 
		GPSTimeOut=SaveT;
	}   
	rtn = TRUE;   
Exit:
	GSSiGlobUlFree (&hMem);
	
	return rtn;
} 

int SendMagellanPacket (LPSTR Packet)
{
	int        nError, nLength=1,ii, ierr,LenInput ;
	COMSTAT    ComStat ;
	MSG        msg ; 
	BOOL	HaveMsg;  
	BOOL	Continue=TRUE;
	COMSTAT	cStat; 
	BYTE	ChkSum, ChkSumMG;
	int		rtn=0; 
	short	PacketID, numstuff; 
	HCURSOR	hcurSave; 
	HANDLE	hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);
	LPBYTE	OutPacket=GlobalLock (hMem);
	int		TotTime=0;
	//LPBYTE	InPacket = OutPacket + 2048;
    
    InLength = 0;
//	while (ReadComm( GPSStream, &InPacket[InLength], 512 )>0);  
//	nLength = ReadComm( GPSStream, &InPacket[InLength], 512 );  
//	if (nLength > 0)
//		InLength += nLength;
	nLength = 1;
	if (*Packet == '$')
		_fstrcpy (OutPacket,Packet);
	else 
		sprintf (OutPacket,"$PMGNCMD,%s",Packet);
	ChkSum = ComputeNMEACheckSum (OutPacket,_fstrlen(OutPacket));
	sprintf (_fstrchr(OutPacket,0),"*%2.2X\r\n",ChkSum);
Top:
	PurgeComm(GPSStream, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
//    ii=FlushComm (GPSStream,0);
//    ii=FlushComm (GPSStream,1);
	InLength = 0;
	ii=WriteComm(GPSStream,OutPacket,_fstrlen(OutPacket));
    if (!_fstrnicmp (OutPacket,"$PMGNCSM,",9))   
    {
    	rtn = VALIDPACKET;
    	goto Exit; 
    }
   
	while (TotTime < GPSTimeOut)
	{
		nLength = ReadComm(GPSStream, &InPacket[InLength], 1024);  
		if (nLength > 0)
		{
			InLength += nLength;
			TotTime = 0;
		}
		if ((PacketID = HaveMagellanCheckSumPacket (InPacket,&InLength,&ChkSumMG)))
		{  
			if (PacketID == UNABLEPACKET)
			{   
				MessageBox (GetFocus(),"GPS returned UNABLE response to above command",Packet,MB_ICONEXCLAMATION);
				return 0;
			}
			if (ChkSum == ChkSumMG) 
			{
				rtn = VALIDPACKET;
				goto Exit;
			}
			else
				MessageBox (0,"Invalid checksum",0,MB_OK);
		}
		TotTime += CommTimeout;
	}

Exit: 
	GSSiGlobUlFree (&hMem);
	
	return rtn;
}

/*	SetTimer (hWndMain,GPSTIMER,GPSTimeOut,NULL);
    HaveMsg = TRUE;  
//Sleep (4000);
    while (Continue)
    {   
    	if (nLength>0)
    		msg.message = MOREDATA;
    	else
			Continue = GetMessage( &msg, NULL, 0,0);
		switch (msg.message)
		{   
			case WM_TIMER:
				if (msg.wParam != GPSTIMER)
					break;
				KillTimer (hWndMain,GPSTIMER);
				goto Exit;
				
			default:  
				if (msg.message == WM_SETCURSOR)
					break;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			    break;
			    
			case WM_COMMNOTIFY:
      			//if (CN_EVENT & LOWORD( msg.lParam ) != CN_EVENT)
      				break;
      		
            case MOREDATA:
			     GetCommEventMask( GPSStream, EV_RXFLAG ) ;
				 if ((nLength = ReadComm( GPSStream, &InPacket[InLength], 1024-InLength))>0)
				 	InLength+=nLength; 
			 	 if ((PacketID = HaveMagellanCheckSumPacket (InPacket,&InLength,&ChkSumMG)))
				 {  
					KillTimer (hWndMain,GPSTIMER); 
				 	if (PacketID == UNABLEPACKET)
				 	{   
				 		MessageBox (GetFocus(),"GPS returned UNABLE response to above command",Packet,MB_ICONEXCLAMATION);
				 		return 0;
				 	}
					if (ChkSum == ChkSumMG) 
					{
						rtn = TRUE;
						goto Exit;
					}
					else
						goto Top;
				 }
			}
      }
	KillTimer (hWndMain,GPSTIMER); 
*/
 

short GetMagellanPacket (LPBYTE Packet)
{
	int        nError, nLength=1,ii, ierr,LenInput ;
	COMSTAT    ComStat ;
	MSG        msg ; 
	BOOL	HaveMsg;  
	BYTE	OutPacket[64], ChkSum, ChkSumMG; 
	BOOL	Continue=TRUE;
	short	PacketID=0;
	COMSTAT	cStat; 
	HCURSOR	hcurSave;
	int		TotTime=0;
   
	while (TotTime < GPSTimeOut)
	{
		nLength = ReadComm(GPSStream, &InPacket[InLength], 1024); 
		if (nLength > 0)
		{
			InLength += nLength;
			TotTime = 0;
		}
		if ((PacketID = HaveCompleteMagellanPacket (InPacket,&InLength,Packet,&ChkSumMG)))
		{      
			//KillTimer (hWndMain,GPSTIMER); 
			ChkSum = ComputeNMEACheckSum (Packet,_fstrlen(Packet));
			sprintf (OutPacket,"$PMGNCSM,%2.2X",ChkSum);  
			SendMagellanPacket (OutPacket);  
			if (ChkSum == ChkSumMG)
				goto Exit;
			
		} 
		TotTime += CommTimeout;
	}
Exit:

	return PacketID;
}

/*	nLength = 1; 
Top:
	SetTimer (hWndMain,GPSTIMER,GPSTimeOut,NULL);
    HaveMsg = TRUE;
    while (Continue)
    {   
    	if (nLength>0)
    		msg.message = MOREDATA;
    	else
			Continue = GetMessage( &msg, NULL, 0,0);
		switch (msg.message)
		{   
			case WM_TIMER:
				if (msg.wParam != GPSTIMER)
					break;
				KillTimer (hWndMain,GPSTIMER);  
				goto Exit;
				
			default:  
//				if (msg.message != WM_PAINT)
//					break;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			    break;
			    
			case WM_COMMNOTIFY:
      			//if (CN_EVENT & LOWORD( msg.lParam ) != CN_EVENT)
      				break;
      		
            case MOREDATA:
			     GetCommEventMask( GPSStream, EV_RXFLAG ) ;
			 	 if ((PacketID = HaveCompleteMagellanPacket (InPacket,&InLength,Packet,&ChkSumMG)))
				 {      
						KillTimer (hWndMain,GPSTIMER); 
						ChkSum = ComputeNMEACheckSum (Packet,_fstrlen(Packet));
						sprintf (OutPacket,"$PMGNCSM,%2.2X",ChkSum);  
						SendMagellanPacket (OutPacket);  
						if (ChkSum == ChkSumMG)
				 			goto Exit;
				 		else
				 			goto Top;
				 } 
				 if ((nLength = ReadComm( GPSStream, &InPacket[InLength], 1024-InLength))>0)
				 	InLength+=nLength; 
			 	 if ((PacketID = HaveCompleteMagellanPacket (InPacket,&InLength,Packet,&ChkSumMG)))
				 {      
						KillTimer (hWndMain,GPSTIMER); 
						ChkSum = ComputeNMEACheckSum (Packet,_fstrlen(Packet));
						sprintf (OutPacket,"$PMGNCSM,%2.2X",ChkSum);  
						SendMagellanPacket (OutPacket);  
						if (ChkSum == ChkSumMG)
				 			goto Exit;
				 		else
				 			goto Top;
				 } 
			}
      }
	KillTimer (hWndMain,GPSTIMER);
*/
 
BOOL OpenLowrance (HWND hWnd,LPSTR Identity)
{
	BYTE	Packet[270], Data[100],SessionID; 
	LPBYTE	pstr=Packet; 
	LSI100ProductID	ProdID;
	LSI100HEADER	Header;

	while (ReadComm( GPSStream,InPacket, 32 )>0); 

Top:
	CreateLSI100Header (LSI100WakeUp,0,&Header);   
	if (!SendLSI100Header (GPSStream,&Header,NULL,&SessionID))  
	{
Error:
	   	return FALSE; 
	}
	CreateLSI100Header (LSI100GetProdInfo,0,&Header);  
	if (!SendLSI100Header (GPSStream,&Header,NULL,(LPBYTE)&ProdID))
		goto Error;  
//    _fstrcpy (Identity,pDesc);     
	MaxLowranceWP = ProdID.NumOfWaypoints;   //sizeof(LSI100ProductID)
	MaxLowranceRouteWP = ProdID.NumOfWaypointsPerRoute; 
	MaxLowrancePlotTrails = ProdID.NumOfPlotTrails;  
	LowranceProtocolVersion = ProdID.ProtocolVersion;
	
	if (ProdID.ProductID < 13)
		sprintf (Identity,"%s",LSI100ProductName[ProdID.ProductID-1]);
	else
		sprintf (Identity,"Lowrance/Eagle (Product ID = %i)",ProdID.ProductID);
	return TRUE;
} 

void CreateLSI100Header (WORD cmd,int lArg,LPLSI100HEADER pHead )
{   
	pHead->PreAmble = LSI100PreAmble;
	pHead->Command = cmd;
	pHead->Count = lArg;
	pHead->ID = 0;
	pHead->CHKSUM = ComputeCheckSum ((LPBYTE)pHead,sizeof(LSI100HEADER)-1);
	
	return;
}          

BOOL SendLSI100Header (HANDLE Stream,LPLSI100HEADER pHeader,LPBYTE pDataSnd,LPBYTE pDataRcv)
{
	
	int        nError, nLength=1,ii, LenInput ;
	COMSTAT    ComStat ;
	MSG        msg ;   
	BYTE	ChkSum;  
	BOOL	HaveMsg;  
	BOOL	Continue=TRUE, HaveHeader=FALSE;
	short	PacketID, numstuff; 
	int		TotTime = 0;
  
	InLength = 0;
	ii=WriteComm(Stream,(LPBYTE)pHeader,sizeof(LSI100HEADER)); 
	if (pHeader->Count)
	{
		WriteComm(Stream,pDataSnd,pHeader->Count);
		ChkSum = ComputeCheckSum (pDataSnd,pHeader->Count);
		WriteComm(Stream,&ChkSum,1);
	} 
	if (!pDataRcv) 
	{   
		Sleep (100);
		while (ReadComm( Stream,InPacket, 32 )>0); 
		return TRUE;     
	}
//	SetTimer (hWndMain,GPSTIMER,GPSTimeOut,NULL);

   
	while (TotTime < GPSTimeOut)
	{
		nLength = ReadComm(GPSStream, &InPacket[InLength], 1024);  
		if (nLength > 0)
		{
			InLength += nLength;
			if (InLength > 1024*3)
				goto Exit;
			TotTime = 0;
		}
		if ((HaveCompleteLSI100Packet (InPacket,InLength)))
		{
			if (GetLSI100Data (InPacket,InLength,pHeader->Command,pDataRcv))
				return TRUE;
		} 
		TotTime += CommTimeout;
    }
	
Exit:  
	
	return FALSE;
}
 
BOOL OpenGarmin (HWND hWnd,LPSTR Identity)
{
	int        nError, nLength,ii, LenInput ;
	COMSTAT    ComStat ;
	MSG        msg ;  
	HANDLE	hMem=GSSiGlobAlloc (0,GMEM_MOVEABLE,4096);  
	LPBYTE	Packet=GlobalLock (hMem); 
	LPBYTE	pstr=Packet; 
	HANDLE	hInput;
	LPSTR	pInput, bInput, pCmd;
	POINT	CursorPoint;
	short	ProdID, SVer, PacketID,n=0, attempts=0;
	LPSHORT	pInt;  
	LPSTR	pDesc;
	BOOL	rtn = FALSE; 
    
    if (GPSStream == USBStream) 
		if (!OpenGarminUSB (Identity))
			goto Exit;
	
Top:
	if (attempts > 2)
		goto Exit;
	CreatePacket (send_init,0,pstr);   
	if (!SendPacket (GPSStream,pstr))  
	{   
	   	goto Exit;
	}
	attempts++;
	if (!(PacketID = GetPacket (GPSStream,pstr)))
   		goto Top;
   	do
   	{
   		switch (PacketID)
   		{
			case Pid_Product_Data: 
			    pInt = (LPSHORT)&Packet[3];
			    ProdID = *pInt++;
			    SVer = *pInt++;
			    pDesc = (LPSTR)pInt;
			    _fstrcpy (Identity,pDesc);
			    break;
   			case  Pid_Protocol_Array:
	    		ProcessPidProtocolArray (pstr); 
	    		if (n) 
	    		{
	    			rtn=TRUE;
	    			goto Exit;
	    		}
	    		n++;
	    	default:    
    		break; 
    	}
    }while((PacketID = GetPacket (GPSStream,pstr)));//Get protocol capabilities array if exists   
	rtn = TRUE;
Exit:
	GSSiGlobUlFree (&hMem);
	return rtn;
}

BOOL GetSystemErrMessage (DWORD errorcode,LPSTR Mess)
{
	LPVOID lpMsgBuf;

	BOOL	rtn = FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &lpMsgBuf,
		0,
		NULL 
	);
	if (rtn)
		strcpy (Mess,lpMsgBuf);
	else
		*Mess = 0;
	LocalFree( lpMsgBuf );
	return rtn;
}

HANDLE OpenSIOConnection( HWND hWnd ,LPSTR ID,short flowtype,BOOL UseEvtChr,BYTE EvtChr,LPSTR ForceBaud,BOOL ShowError)
{
   BOOL       fRetVal ;
   BOOL		rtn=FALSE;
   HANDLE	Stream=0;
  int		ii;
   LPSTR	lpComma, lpStart;    
   char		title[144],mess[256];
   DWORD	ierr;
   
   if ((Stream = OpenComm( ID, 4096, 256)) == (HANDLE)-1)  
   {     
 /*  		switch (Stream)
   		{
   		
			case IE_BADID:
				_fstrcpy(mess,"The device identifier is invalid or unsupported.");
				break;
			case IE_BAUDRATE:
				_fstrcpy(mess,"The device's baud rate is unsupported.");
				break;
			case IE_BYTESIZE:
				_fstrcpy(mess,"The specified byte size is invalid.");
				break;
			case IE_DEFAULT:
				_fstrcpy(mess,"The default parameters are in error."); 
				break;
			case IE_HARDWARE:
				_fstrcpy(mess,"The hardware is not available (is locked by another device).");
				break;
			case IE_MEMORY:
				_fstrcpy(mess,"The function cannot allocate the queues."); 
				break;
			case IE_NOPEN:
				_fstrcpy(mess,"The device is not open."); 
				break;
			case IE_OPEN:
				_fstrcpy(mess,"The device is already open."); 
                break;
      	} */
	    GetSystemErrMessage (GetLastError(),mess);
      	sprintf(title,"Error opening com port %s",ID);
      	if (ShowError)
      		MessageBox (hWnd,mess,title,MB_ICONEXCLAMATION);
        return ( Stream) ;
    } 
//   DumpDCB (Stream);
//    ii=CloseComm (Stream);
	if (SetupSIOConnection(Stream,flowtype,UseEvtChr,EvtChr,ForceBaud))
	{
      // set up notifications from COMM.DRV

         // In this case we really are only using the notifications
         // for the received characters - it could be expanded to
         // cover the changes in CD or other status lines.


         // Enable notifications for events only.

         // NB:  This method does not use the specific
         // in/out queue triggers.
        if (UseEvtChr)  
        {
 //      		SetCommEventMask( Stream, EV_RXFLAG ) ;
 //       	ii=EnableCommNotification(Stream, hWnd, -1, -1 ) ;  
        }
        else 
        {
//			SetCommEventMask( Stream, EV_RXCHAR ) ;
 //      		ii=EnableCommNotification(Stream, hWnd, -1, -1 ) ; 
       	}
      // assert DTR

      ii=EscapeCommFunction( Stream, SETDTR ) ;  
      ii=EscapeCommFunction( Stream, SETRTS ) ;  
//   DumpDCB (Stream);
      
   }
   else
   {  
   	  if (ShowError)
   	  	MessageBox (hWnd,"Error setting GPS parameters","Unable to open GPS",MB_ICONEXCLAMATION);
      CloseComm(Stream) ;
      Stream=(HANDLE)-1;
   }

   return ( Stream ) ;

}

BOOL SetupSIOConnection(HANDLE Stream,short flowtype,BOOL UseEvtChr,BYTE EvtChr,LPSTR ForceBaud)
{
   int		rtn,i ;
   BYTE       bSet , SaveID;
   DCB        dcb ,savedcb;
   char			str[128]; 
   LPSTR	pCmd;

	rtn = GetCommState (Stream,&dcb); 
   rtn = SetCommState(Stream, &dcb ); 
   savedcb = dcb;
	//SaveID = dcb.Id;
//	_fmemset (&dcb,0,sizeof(DCB));	  
//	dcb.Id = SaveID;      
	dcb.XonChar = 17;
	dcb.XoffChar = 19;
	dcb.XonLim = 10;
	dcb.XoffLim = 10;
	if (ForceBaud)
		_fstrcpy (str,ForceBaud);
	else
		GetPrivateProfileString ("GPS","Baud","4800",str,32,GMIni);
	if (!*str)
		_fstrcpy (str,"4800");  
	dcb.BaudRate = GetBaud (str);
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fOutxCtsFlow = 0;
//	dcb.CtsTimeout = 0;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fOutxDsrFlow = 0;
//	dcb.DsrTimeout = 0;
	dcb.fInX = dcb.fOutX = 0; 
	if (flowtype==1)
	{
		dcb.fDtrControl = DTR_CONTROL_ENABLE ; 
		dcb.fOutxCtsFlow = 1;
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
//		dcb.CtsTimeout = 500;  
	}
	else if (flowtype==2) 
	{
//		dcb.fRtsDisable = TRUE;
	    dcb.fOutxDsrFlow = 1;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
//.		dcb.DsrTimeout = 500;
   	}
	dcb.fBinary = TRUE ;
	dcb.fParity = FALSE ;
//	dcb.fChEvt = UseEvtChr;  
	dcb.EvtChar = EvtChr;
//	dcb = savedcb;
   rtn = SetCommState(Stream, &dcb );
   LogCommIO (3,0,dcb.BaudRate);
   if (rtn < 0)
   		return FALSE;
   return TRUE;  
   

} 
BOOL CloseSIOConnection(HANDLE Stream)
{  
	short	i;
	
   if (Stream == (HANDLE)-1)
   		return FALSE;
	PurgeComm(Stream, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
   //i=FlushComm (Stream,0);
   //i=FlushComm (Stream,1);
   //EnableCommNotification( Stream, NULL, -1, -1 ) ;

   // kill the focus


   EscapeCommFunction(Stream, CLRDTR ) ;

   // close comm connection

   i=CloseComm( Stream ) ;
   if (i<0)
   	return FALSE;
	
   return TRUE;

}   

short GPSExportGarminRoute (HWND hWndDlg,UINT Control,UINT StatusControl)
{
	char	str[256], cUID[32];	 
	long	nRecs,nLoaded;
	BYTE	Packet[270];  
	short	PacketID=0, nItems, lID=0,lCMT=0, RouteNumber;
	D200_Rte_Hdr_Type	RteD200;      
	D201_Rte_Hdr_Type	RteD201;      
	D202_Rte_Hdr_Type	RteD202;      
	D100_Wpt_Type	WptD100;      
	D101_Wpt_Type	WptD101;      
	D102_Wpt_Type	WptD102;      
	D103_Wpt_Type	WptD103;      
	D104_Wpt_Type	WptD104;      
	D107_Wpt_Type	WptD107;      
	D108_Wpt_Type	WptD108; 
	D109_Wpt_Type	WptD109; 
	D110_Wpt_Type	WptD110; 
	D210_Rte_Link_Type	LinkData;
	
	LPBYTE	Wpt, Rte,Lnk=(LPBYTE)&LinkData,Wpt_ident, Wpt_cmnt=0, Wpt_smbl, Wpt_attr=0,Wpt_subclass=0; 
	LPFLOAT	Wpt_alt=0, Wpt_dpth=0;
	Semicircle_Type	FAR	*Wpt_posn;    
	HCURSOR	hcurSave;    
	HANDLE	hItems;
	LPINT	lpItems; 
	float	Depth;
	BOOL	AltIsDepth, HaveDepth; 
	LPSTR	pLat,pLon, pTAB, pOpts, pSym, pDesc, pRef,pEnd; 
	BYTE	SubClassInit[18]={0x00,0x00,0x00,0x00,0x00,0x00,
							  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	long	Ref;
	double	lat,lon;
	BOOL	Err;
	int		i,n;     
	short	sizeWpt=sizeof(D103_Wpt_Type), sizecomment=40, PadZeros=0,sizeRte=sizeof(D201_Rte_Hdr_Type),sizeLnk=sizeof(LinkData);
	
	_fmemset (Lnk,0,sizeLnk);
	
	switch (RteHdrFormat)
	{
		default:  
			sprintf (str,"GPS route format %i is not supported",RteHdrFormat);
			GSSiMessageBox (str,NULL,MB_ICONEXCLAMATION,0);
			return FALSE;
		case 200:
			Rte = &RteD200;  
			*str=0;
			if (!GetTextString (hWndDlg,str,2,"Enter Route Number",NULL,NULL,0,TRUE,TRUE))
				return FALSE; 
			RouteNumber = RteD200 = atoi (str);
			sizeRte = sizeof(D200_Rte_Hdr_Type);   
			break;
		case 201:
			Rte = (LPBYTE)&RteD201; 
			*str=0;
			if (!GetTextString (hWndDlg,str,2,"Enter Route Number",NULL,NULL,0,TRUE,TRUE))
				return FALSE; 
			RouteNumber = RteD201.nmbr=atoi (str);
			*str=0;
			if (!GetTextString (hWndDlg,str,20,"Enter Route Comment",NULL,NULL,0,TRUE,TRUE))
				return FALSE; 
			_fstrupr (str); 
			_fstrncpy (RteD201.cmnt,str,20); 
			PadString (RteD201.cmnt,' ',19);    
			RteD201.cmnt[19] = ' ';
			sizeRte = sizeof(D201_Rte_Hdr_Type); 
			break; 
		case 202:
			*str=0;
			if (!GetTextString (hWndDlg,str,2,"Enter Route Number",NULL,NULL,0,TRUE,TRUE))
				return FALSE; 
			RouteNumber = atoi (str);
			*str=0;
			if (!GetTextString (hWndDlg,str,20,"Enter Route Comment",NULL,NULL,0,TRUE,TRUE))
				return FALSE; 
			_fstrupr (str); 
			Rte = str;
			sizeRte = _fstrlen (str) + 1;
			break; 
	}
	switch (RteWptFormat)
	{
		default:  
			sprintf (str,"GPS format %i is not supported",WptFormat);
			GSSiMessageBox (str,NULL,MB_ICONEXCLAMATION,0);
			return FALSE;
		case 100:
			Wpt = (LPBYTE)&WptD100;
			Wpt_ident = (LPBYTE)&WptD100.ident;
			Wpt_cmnt = (LPBYTE)&WptD100.cmnt;
			Wpt_smbl = 0; 
			Wpt_posn = &WptD100.posn;
			sizeWpt = sizeof(D100_Wpt_Type);   
			sizecomment=40;
			break;
		case 101:
			Wpt = (LPBYTE)&WptD101;
			Wpt_ident = (LPBYTE)&WptD101.ident;
			Wpt_cmnt = (LPBYTE)&WptD101.cmnt;
			Wpt_smbl = (LPBYTE)&WptD101.smbl; 
			Wpt_posn = &WptD101.posn;
			sizeWpt = sizeof(D101_Wpt_Type); 
			sizecomment=40;
			break;
		case 102:
			Wpt = (LPBYTE)&WptD102;
			Wpt_ident = (LPBYTE)&WptD102.ident;
			Wpt_cmnt = (LPBYTE)&WptD102.cmnt;
			Wpt_smbl = (LPBYTE)&WptD102.smbl; 
			Wpt_posn = &WptD102.posn;
			sizeWpt = sizeof(D102_Wpt_Type); 
			sizecomment=40;
			break;
		case 103:
			Wpt = (LPBYTE)&WptD103;
			Wpt_ident = (LPBYTE)&WptD103.ident;
			Wpt_cmnt = (LPBYTE)&WptD103.cmnt;
			Wpt_smbl = (LPBYTE)&WptD103.smbl; 
			Wpt_posn = &WptD103.posn;
			sizeWpt = sizeof(D103_Wpt_Type); 
			sizecomment=40;
			break;
		case 104:
			Wpt = (LPBYTE)&WptD104;
			Wpt_ident = (LPBYTE)&WptD104.ident;
			Wpt_cmnt = (LPBYTE)&WptD104.cmnt;
			Wpt_smbl = (LPBYTE)&WptD104.smbl; 
			Wpt_posn = &WptD104.posn;
			sizeWpt = sizeof(D104_Wpt_Type); 
			sizecomment=40;
			break;
		case 107:
			Wpt = (LPBYTE)&WptD107;
			Wpt_ident =(LPBYTE) &WptD107.ident;
			Wpt_cmnt = (LPBYTE)&WptD107.cmnt;
			Wpt_smbl = (LPBYTE)&WptD107.smbl; 
			Wpt_posn = &WptD107.posn;
			sizeWpt = sizeof(D107_Wpt_Type); 
			sizecomment=40;
			break;
		case 108:
			Wpt = (LPBYTE)&WptD108;
			Wpt_ident =(LPBYTE) &WptD108.ident;
			Wpt_smbl = (LPBYTE)&WptD108.smbl; 
			Wpt_posn = &WptD108.posn;   
			Wpt_alt = &WptD108.alt;
			Wpt_dpth = &WptD108.dpth;
			Wpt_attr =  &WptD108.attr; 
			Wpt_subclass = (LPBYTE)&WptD108.subclass;
			sizeWpt = sizeof(D108_Wpt_Type)-265;      
			sizecomment=0;
			break;
		case 109:
			Wpt = (LPBYTE)&WptD109;
			Wpt_ident = (LPBYTE)&WptD109.ident;
			Wpt_smbl = (LPBYTE)&WptD109.smbl; 
			Wpt_posn = &WptD109.posn;   
			Wpt_alt = &WptD109.alt;
			Wpt_dpth = &WptD109.dpth;
			Wpt_attr =  &WptD109.attr; 
			Wpt_subclass = (LPBYTE)&WptD109.subclass;
			sizeWpt = sizeof(D109_Wpt_Type)-265;      
			sizecomment=0;
			break;
		case 110:
			Wpt = (LPBYTE)&WptD110;
			Wpt_ident = (LPBYTE)&WptD110.ident;
			Wpt_smbl = (LPBYTE)&WptD110.smbl; 
			Wpt_posn = &WptD110.posn;   
			Wpt_alt = &WptD110.alt;
			Wpt_dpth = &WptD110.dpth;
			Wpt_attr =  &WptD110.attr; 
			Wpt_subclass = (LPBYTE)&WptD110.subclass;
			sizeWpt = sizeof(D110_Wpt_Type)-265;      
			sizecomment=0;
			break;
	}  
	nItems=(short)SendDlgItemMessage(hWndDlg ,Control,LB_GETSELCOUNT,0,0); 
	hItems=GSSiGlobAlloc (1036,GMEM_MOVEABLE,nItems*4);
	lpItems=  (LPINT) GlobalLock(hItems);
	SendDlgItemMessage(hWndDlg ,Control,LB_GETSELITEMS,nItems,(LPARAM)lpItems);
	GlobalUnlock (hItems);
Top:    
//	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
	lpItems=  (LPINT) GlobalLock(hItems);
	CreatePacket (send_records,nItems+1,Packet);   
	if (!SendPacket (GPSStream,Packet))  
	{ 
ErMsg:  
//		GSSiSetCursor (hcurSave);   
		GlobalUnlock (hItems);
		if (MessageBox (hWndDlg,"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
			NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
			goto Top;
		GSSiGlobFree (&hItems);
	   	return FALSE; 
	}  
	CreateDataPacket (send_route,Rte,sizeRte,0,Packet);   
	if (!SendPacket (GPSStream,Packet))
		goto ErMsg;  
	n = nRecs = nItems;
	nLoaded = 0;
	while (n--)
	{   
		_fmemset (Wpt,0,sizeWpt);
		if (WptFormat == 109 || WptFormat == 110)
		{
			WptD109.dtyp = 0x01;   
			WptD109.ete = 0xffffffff;
			WptD109.dspl_color = 7;
		}		
		if (WptFormat == 110)
		{
			WptD110.dtyp = 0x01;   
			WptD110.ete = 0xffffffff;
			WptD110.dspl_color = 7;
		}		
		SendDlgItemMessage (hWndDlg,Control,LB_GETTEXT,*lpItems++,(LPARAM)str); 
		if ((pRef = _fstrrchr (str,'\t'))) 
		{
			*pRef++ = 0;
			Ref = atol (pRef);
		} 
		else
			Ref=1;
		pTAB = _fstrchr (str,'\t');
		*pTAB++ = 0;
//		_fstrncpy (Wpt_ident,str,6); 
		sprintf (Wpt_ident,"RT%i%ld",RouteNumber,nLoaded+1);
		if (WptFormat == 108 || WptFormat == 109 || WptFormat == 110) 
		{
			Wpt_ident[6] = 0; 
			lID = _fstrlen(Wpt_ident)+1; 
		}
		else
			for (i=0;i<6;i++)
				if (!Wpt_ident[i])
					Wpt_ident[i] = ' ';
		pSym = pTAB;
		pTAB = _fstrchr (pSym,'\t');
		*pTAB++ = 0;
		pDesc = pTAB;
		pTAB = _fstrchr (pDesc,'\t');
		*pTAB++ = 0;   
		Depth = FTM*strtod (pDesc,&pEnd);
		if (pEnd > pDesc)
			HaveDepth = TRUE;
		else
			HaveDepth = FALSE;
		AltIsDepth = FALSE;
		if (Wpt_cmnt)
		{ 
			_fstrncpy (Wpt_cmnt,pDesc,sizecomment);
			Wpt_cmnt[sizecomment-1]=0;
			_fstrupr(Wpt_cmnt);  
			PadString (Wpt_cmnt,' ',sizecomment-1);
			Wpt_cmnt[sizecomment-1]=' '; 
		}
		else if (WptFormat == 108 || WptFormat == 109 || WptFormat == 110)
		{
			LPSTR pCmt=_fstrchr(Wpt_ident,0) + 1;
			_fstrcpy (pCmt,pDesc);
			lCMT = _fstrlen(pCmt)+1; 
			if (WptFormat == 108)
				AltIsDepth = TRUE;
		}
		if (Wpt_alt)
		{
			*Wpt_alt = 1.0e25;
			if (AltIsDepth && HaveDepth) 
				*Wpt_alt =  Depth;
		}
		if (Wpt_dpth) 
		{
			if ((WptFormat == 109 || WptFormat == 110) && HaveDepth) 
				*Wpt_dpth = Depth;
			else
				*Wpt_dpth = 1.0e25; 
		}
		if (Wpt_attr) 
		{
			if (WptFormat == 109)
				*Wpt_attr = 0x70;
			else if (WptFormat == 110)
				*Wpt_attr = 0x80;
			else
				*Wpt_attr = 0x60;   
		}
		if (Wpt_subclass)
			_fmemcpy (Wpt_subclass,SubClassInit,18);
		pOpts = pTAB;
		pTAB = _fstrchr (pOpts,'\t');
		*pTAB++ = 0;
		pLat = pTAB;
		pTAB = _fstrchr (pLat,'\t');
		*pTAB++ = 0;
		pLon = pTAB;
		lat = DecDegFromDMS (pLat,&Err); 
		Wpt_posn->lat = lat/SEMICIRCLETODEGREE;
		lon = DecDegFromDMS (pLon,&Err);
		Wpt_posn->lon = lon/SEMICIRCLETODEGREE;
		if (Wpt_smbl)
        	*Wpt_smbl = smbl_fish;
		CreateDataPacket (send_route_wpt,Wpt,sizeWpt+lID+lCMT,PadZeros,Packet);   
		if (!SendPacket (GPSStream,Packet))
			goto ErMsg;
		if (RouteProtocol==201)
		{
			CreateDataPacket (send_route_link_data,Lnk,sizeLnk,0,Packet);   
			if (!SendPacket (GPSStream,Packet))
				goto ErMsg;
		}
        PctBox (GetDlgItem(hWndDlg,StatusControl),nRecs,++nLoaded,-1);
	}
	CreatePacket (send_complete,Cmnd_Transfer_Rte,Packet);   
	if (!SendPacket (GPSStream,Packet))
		goto ErMsg;  
//	GSSiSetCursor (hcurSave); 
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete");
	GSSiGlobUlFree (&hItems);
	return nItems;
}
 
short GPSExportGarminWP (HWND hWndDlg,UINT Control,UINT StatusControl)
{
	char	str[256], cUID[32];	 
	long	nRecs,nLoaded;
	BYTE	Packet[270];  
	short	PacketID=0, nItems, lID=0,lCMT=0, MaxWPIDLength=6;
	D100_Wpt_Type	WptD100;      
	D101_Wpt_Type	WptD101;      
	D102_Wpt_Type	WptD102;      
	D103_Wpt_Type	WptD103;      
	D104_Wpt_Type	WptD104;      
	D107_Wpt_Type	WptD107;      
	D108_Wpt_Type	WptD108; 
	LPD109_Wpt_Type	pWptD109; 
	LPD110_Wpt_Type	pWptD110; 
	LPBYTE	Wpt, Wpt_ident, Wpt_cmnt=0, Wpt_smbl, Wpt_attr=0,Wpt_subclass=0; 
	LPFLOAT	Wpt_alt=0, Wpt_dpth=0;
	Semicircle_Type	FAR	*Wpt_posn;    
	HCURSOR	hcurSave;    
	HANDLE	hItems;
	LPINT	lpItems; 
	float	Depth;
	BOOL	AltIsDepth, HaveDepth; 
	LPSTR	pLat,pLon, pTAB, pOpts, pSym, pDesc, pRef,pEnd; 
	BYTE	SubClassInit[18]={0x00,0x00,0x00,0x00,0x00,0x00,
							  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	long	Ref;
	double	lat,lon;
	int		i,n;     
	BOOL	Err;
	short	sizeWpt=sizeof(D103_Wpt_Type), sizecomment=40,PadZeros=0;  
	HANDLE	hWPMem=GSSiGlobAlloc (0,GHND,1024);
	LPSTR	pWP=GlobalLock (hWPMem);
	
	switch (WptFormat)
	{
		default:  
			sprintf (str,"GPS format %i is not supported",WptFormat);
			GSSiMessageBox (str,NULL,MB_ICONEXCLAMATION,0);
			return FALSE;
		case 100:
			Wpt = (LPBYTE)&WptD100;
			Wpt_ident = (LPBYTE)&WptD100.ident;
			Wpt_cmnt = (LPBYTE)&WptD100.cmnt;
			Wpt_smbl = 0; 
			Wpt_posn = &WptD100.posn;
			sizeWpt = sizeof(D100_Wpt_Type);   
			sizecomment=40;
			break;
		case 101:
			Wpt = (LPBYTE)&WptD101;
			Wpt_ident = (LPBYTE)&WptD101.ident;
			Wpt_cmnt = (LPBYTE)&WptD101.cmnt;
			Wpt_smbl = (LPBYTE)&WptD101.smbl; 
			Wpt_posn = &WptD101.posn;
			sizeWpt = sizeof(D101_Wpt_Type); 
			sizecomment=40;
			break;
		case 102:
			Wpt = (LPBYTE)&WptD102;
			Wpt_ident = (LPBYTE)&WptD102.ident;
			Wpt_cmnt = (LPBYTE)&WptD102.cmnt;
			Wpt_smbl = (LPBYTE)&WptD102.smbl; 
			Wpt_posn = &WptD102.posn;
			sizeWpt = sizeof(D102_Wpt_Type); 
			sizecomment=40;
			break;
		case 103:
			Wpt = (LPBYTE)&WptD103;
			Wpt_ident = (LPBYTE)&WptD103.ident;
			Wpt_cmnt = (LPBYTE)&WptD103.cmnt;
			Wpt_smbl = (LPBYTE)&WptD103.smbl; 
			Wpt_posn = &WptD103.posn;
			sizeWpt = sizeof(D103_Wpt_Type); 
			sizecomment=40;
			break;
		case 104:
			Wpt = (LPBYTE)&WptD104;
			Wpt_ident = (LPBYTE)&WptD104.ident;
			Wpt_cmnt = (LPBYTE)&WptD104.cmnt;
			Wpt_smbl = (LPBYTE)&WptD104.smbl; 
			Wpt_posn = &WptD104.posn;
			sizeWpt = sizeof(D104_Wpt_Type); 
			sizecomment=40;
			break;
		case 107:
			Wpt = (LPBYTE)&WptD107;
			Wpt_ident = (LPBYTE)&WptD107.ident;
			Wpt_cmnt = (LPBYTE)&WptD107.cmnt;
			Wpt_smbl = (LPBYTE)&WptD107.smbl; 
			Wpt_posn = &WptD107.posn;
			sizeWpt = sizeof(D107_Wpt_Type); 
			sizecomment=40;
			break;
		case 108:
			Wpt = (LPBYTE)&WptD108;
			Wpt_ident = (LPBYTE)&WptD108.ident;
			Wpt_smbl = (LPBYTE)&WptD108.smbl; 
			Wpt_posn = &WptD108.posn;   
			Wpt_alt = &WptD108.alt;
			Wpt_dpth = &WptD108.dpth;
			Wpt_attr =  &WptD108.attr; 
			Wpt_subclass = (LPBYTE)&WptD108.subclass;
			sizeWpt = sizeof(D108_Wpt_Type)-265;      
			sizecomment=0;
			break;
		case 109: 
			pWptD109 = (LPD109_Wpt_Type)pWP;
			Wpt = (LPBYTE)pWptD109;
			Wpt_ident = (LPBYTE)&pWptD109->ident;
			Wpt_smbl = (LPBYTE)&pWptD109->smbl; 
			Wpt_posn = &pWptD109->posn;   
			Wpt_alt = &pWptD109->alt;
			Wpt_dpth = &pWptD109->dpth;
			Wpt_attr =  &pWptD109->attr; 
			Wpt_subclass = (LPBYTE)&pWptD109->subclass; 
			sizeWpt = sizeof(D109_Wpt_Type)-265;      
			sizecomment=0; 
			PadZeros = 4;
			break;
		case 110: 
			pWptD110 = (LPD110_Wpt_Type)pWP;
			Wpt = (LPBYTE)pWptD110;
			Wpt_ident = (LPBYTE)&pWptD110->ident;
			Wpt_smbl = (LPBYTE)&pWptD110->smbl; 
			Wpt_posn = &pWptD110->posn;   
			Wpt_alt = &pWptD110->alt;
			Wpt_dpth = &pWptD110->dpth;
			Wpt_attr =  &pWptD110->attr; 
			Wpt_subclass = (LPBYTE)&pWptD110->subclass; 
			sizeWpt = sizeof(D110_Wpt_Type)-265;      
			sizecomment=0; 
			PadZeros = 4;   
			MaxWPIDLength = 12;
			break;
	}  
	nItems = GetLBSelectedItems (hWndDlg,Control,&hItems);
Top:    
	lpItems=  (LPINT) GlobalLock(hItems);
//	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
	CreatePacket (send_records,nItems,Packet);   
	if (!SendPacket (GPSStream,Packet))  
	{ 
ErMsg:  
//		GSSiSetCursor (hcurSave);   
		GlobalUnlock (hItems);
		if (MessageBox (hWndDlg,"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
			NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
			goto Top;
		GSSiGlobFree (&hItems);
		GSSiGlobUlFree (&hWPMem);
	   	return FALSE; 
	}  
	n = nRecs = nItems;
	nLoaded = 0;
	while (n--)
	{   
		_fmemset (Wpt,0,sizeWpt);   
		if (WptFormat == 109)
		{
			pWptD109->dtyp = 0x01;   
			pWptD109->ete = 0xffffffff;
			pWptD109->dspl_color = 7;
		}		
		if (WptFormat == 110)
		{
			pWptD110->dtyp = 0x01;   
			pWptD110->ete = 0xffffffff;
			pWptD110->dspl_color = 7;  
			pWptD110->temp = 1.0e25;
			pWptD110->time = 0xFFFFFFFF;
		}		
		SendDlgItemMessage (hWndDlg,Control,LB_GETTEXT,*lpItems++,(LPARAM)str); 
		if ((pRef = _fstrrchr (str,'\t'))) 
		{
			*pRef++ = 0;
			Ref = atol (pRef);
		} 
		else
			Ref=1;
		pTAB = _fstrchr (str,'\t');
		*pTAB++ = 0;
		_fstrncpy (Wpt_ident,str,MaxWPIDLength);
		if (WptFormat == 108 || WptFormat == 109 || WptFormat == 110) 
		{
			Wpt_ident[MaxWPIDLength] = 0; 
			lID = _fstrlen(Wpt_ident)+1; 
		}
		else
			for (i=0;i<6;i++)
				if (!Wpt_ident[i])
					Wpt_ident[i] = ' ';
		pSym = pTAB;
		pTAB = _fstrchr (pSym,'\t');
		*pTAB++ = 0;
		pDesc = pTAB;
		pTAB = _fstrchr (pDesc,'\t');
		*pTAB++ = 0;   
		Depth = FTM*strtod (pDesc,&pEnd);
		if (pEnd > pDesc)
			HaveDepth = TRUE;
		else
			HaveDepth = FALSE;
		AltIsDepth = FALSE;
		if (Wpt_cmnt)
		{ 
			_fstrncpy (Wpt_cmnt,pDesc,sizecomment);
			Wpt_cmnt[sizecomment-1]=0;
			_fstrupr(Wpt_cmnt);  
			PadString (Wpt_cmnt,' ',sizecomment-1);
			Wpt_cmnt[sizecomment-1]=' '; 
		}
		else if (WptFormat == 108 || WptFormat == 109 || WptFormat == 110)
		{
			LPSTR pCmt=_fstrchr(Wpt_ident,0) + 1;
			_fstrcpy (pCmt,pDesc);
			lCMT = _fstrlen(pCmt)+1; 
			if (WptFormat == 108)
				AltIsDepth = TRUE;
		}
		if (Wpt_alt)
		{
			*Wpt_alt = 1.0e25;
			if (AltIsDepth && HaveDepth) 
				*Wpt_alt =  Depth;
		}
		if (Wpt_dpth) 
		{
			if ((WptFormat == 109 || WptFormat == 110) && HaveDepth) 
				*Wpt_dpth = Depth;
			else
				*Wpt_dpth = 1.0e25; 
		}
		if (Wpt_attr) 
		{
			if (WptFormat == 109)
				*Wpt_attr = 0x70;
			else if (WptFormat == 110)
				*Wpt_attr = 0x80;
			else
				*Wpt_attr = 0x60;   
		}
		if (Wpt_subclass)
			_fmemcpy (Wpt_subclass,SubClassInit,18); 
//		if (Ref != -1) //indicate track log
		{ 
			pOpts = pTAB;
			pTAB = _fstrchr (pOpts,'\t');
			*pTAB++ = 0; 
		}
		pLat = pTAB;
		pTAB = _fstrchr (pLat,'\t');
		*pTAB++ = 0;
		pLon = pTAB;
		lat = DecDegFromDMS (pLat,&Err); 
		Wpt_posn->lat = lat/SEMICIRCLETODEGREE;
		lon = DecDegFromDMS (pLon,&Err);
		Wpt_posn->lon = lon/SEMICIRCLETODEGREE;
		if (Wpt_smbl)
        	*Wpt_smbl = smbl_fish;
		CreateDataPacket (send_wpt,Wpt,sizeWpt+lID+lCMT,PadZeros,Packet);   
		if (!SendPacket (GPSStream,Packet))
			goto ErMsg;  
        PctBox (GetDlgItem(hWndDlg,StatusControl),nRecs,++nLoaded,-1);
	}
	CreatePacket (send_complete,Cmnd_Transfer_Wpt,Packet);   
	if (!SendPacket (GPSStream,Packet))
		goto ErMsg;  
//	GSSiSetCursor (hcurSave); 
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete");
	GSSiGlobUlFree (&hItems);   
	GSSiGlobUlFree (&hWPMem);
	return nItems;
}

BOOL GPSImportGarminWP (HWND hWndDlg,UINT Control,UINT StatusControl)
{ 
	char	str[256], cUID[32], Wpt_depth[8];	 
	BYTE	Packet[270];  
	short	PacketID=0;
	D100_Wpt_Type	WptD100;      
	D101_Wpt_Type	WptD101;      
	D102_Wpt_Type	WptD102;      
	D103_Wpt_Type	WptD103;      
	D104_Wpt_Type	WptD104;      
	D107_Wpt_Type	WptD107;      
	D108_Wpt_Type	WptD108; 
	LPD109_Wpt_Type	pWptD109; 
	LPD110_Wpt_Type	pWptD110; 
	LPBYTE	pWpt;     
	char	zero=0;
	LPSTR	Wpt_ident, Wpt_cmnt;     
	LPSEMICIRCLETYPE	Wpt_posn;    
	HCURSOR	hcurSave;
	long	nLoaded, nRecs;  
	HANDLE	hWPMem=GSSiGlobAlloc (0,GHND,1024);
	LPSTR	pWP=GlobalLock (hWPMem);
	short	ii=0,nerr;

	Sleep (500);  
Top:    
//	hCursor = LoadCursor(NULL, IDC_WAIT);
//	hcurSave = GSSiSetCursor(hCursor); 
	CreatePacket (send_request,Cmnd_Transfer_Wpt,Packet);   
	if (!SendPacket (GPSStream,Packet))  
	{ 
ErMsg:  
//		GSSiSetCursor (hcurSave);
		if (MessageBox (hWndDlg,"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
			NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
			goto Top;
		GSSiGlobUlFree (&hWPMem);
	   	return FALSE; 
	} 
	nerr = 0;
	while (PacketID != Pid_Xfer_Cmplt && ContinueProcessing)
	{
		PacketID = GetPacket (GPSStream,Packet);
		switch (PacketID)
		{ 
			case 0: 
//				if (!nerr++)
					break;
				goto ErMsg; 
			case Pid_Records:
				nLoaded = 0;
				nRecs = *(LPSHORT)&Packet[3];  
			case Pid_Xfer_Cmplt:
				break;
			case Pid_Wpt_Data:
			{   
				double lat,lon; 
				char	LatC[32],LonC[32], savec;
				short	Deg, Min,ndp=1, index;
				double	Sec;    
				short	lData=Packet[2];

				pWpt = &Packet[3];  
				_fstrcpy (Wpt_depth,"Unknown");
				switch (WptFormat)
				{
					case 100:
						Wpt_posn = (LPSEMICIRCLETYPE)(pWpt + ((LPBYTE)&WptD100.posn - (LPBYTE)&WptD100));    
						Wpt_ident = pWpt + ((LPBYTE)&WptD100.ident - (LPBYTE)&WptD100);    
						Wpt_cmnt = pWpt + ((LPBYTE)&WptD100.cmnt - (LPBYTE)&WptD100);    
						break; 
					case 101: 
						Wpt_posn = (LPSEMICIRCLETYPE)(pWpt + ((LPBYTE)&WptD101.posn - (LPBYTE)&WptD101));    
						Wpt_ident = pWpt + ((LPBYTE)&WptD101.ident - (LPBYTE)&WptD101);    
						Wpt_cmnt = pWpt + ((LPBYTE)&WptD101.cmnt - (LPBYTE)&WptD101);    
						break;
					case 102: 
						Wpt_posn = (LPSEMICIRCLETYPE)(pWpt + ((LPBYTE)&WptD102.posn - (LPBYTE)&WptD102));    
						Wpt_ident = pWpt + ((LPBYTE)&WptD102.ident - (LPBYTE)&WptD102);    
						Wpt_cmnt = pWpt + ((LPBYTE)&WptD102.cmnt - (LPBYTE)&WptD102);    
						break;
					case 103: 
						Wpt_posn = (LPSEMICIRCLETYPE)(pWpt + ((LPBYTE)&WptD103.posn - (LPBYTE)&WptD103));    
						Wpt_ident = pWpt + ((LPBYTE)&WptD103.ident - (LPBYTE)&WptD103);    
						Wpt_cmnt = pWpt + ((LPBYTE)&WptD103.cmnt - (LPBYTE)&WptD103);    
						break;
					case 104: 
						Wpt_posn = (LPSEMICIRCLETYPE)(pWpt + ((LPBYTE)&WptD104.posn - (LPBYTE)&WptD104));    
						Wpt_ident = pWpt + ((LPBYTE)&WptD104.ident - (LPBYTE)&WptD104);    
						Wpt_cmnt = pWpt + ((LPBYTE)&WptD104.cmnt - (LPBYTE)&WptD104);    
						break;
					case 107: 
						Wpt_posn = (LPSEMICIRCLETYPE)(pWpt + ((LPBYTE)&WptD107.posn - (LPBYTE)&WptD107));    
						Wpt_ident = pWpt + ((LPBYTE)&WptD107.ident - (LPBYTE)&WptD107);    
						Wpt_cmnt = pWpt + ((LPBYTE)&WptD107.cmnt - (LPBYTE)&WptD107);    
						break;
					case 108:
						Wpt_posn = (LPSEMICIRCLETYPE)(pWpt + ((LPBYTE)&WptD108.posn - (LPBYTE)&WptD108));    
						Wpt_ident = pWpt + ((LPBYTE)&WptD108.ident - (LPBYTE)&WptD108);    
						Wpt_cmnt = &zero;    
						break;
					case 109: 
					{
						HFILE	FidWP;
						    
						pWptD109 = (LPD109_Wpt_Type)pWP;
						_fmemmove (pWptD109,pWpt,lData); 
						Wpt_posn = (LPSEMICIRCLETYPE)&pWptD109->posn;    
						Wpt_ident = pWptD109->ident;    
						Wpt_cmnt = _fstrchr (Wpt_ident,0);
						Wpt_cmnt++; 
						if (pWptD109->dpth*MFT < 10000)
							sprintf (Wpt_depth,"%4.0f",pWptD109->dpth*MFT);
						else
							_fstrcpy (Wpt_depth,"Unknown"); 
						break;
					}  
					case 110: 
					{
						HFILE	FidWP;   //sizeof(D110_Wpt_Type)
						    
						pWptD110 = (LPD110_Wpt_Type)pWP;
						_fmemmove (pWptD110,pWpt,lData); 
						Wpt_posn = (LPSEMICIRCLETYPE)&pWptD110->posn;    
						Wpt_ident = pWptD110->ident;    
						Wpt_cmnt = _fstrchr (Wpt_ident,0);
						Wpt_cmnt++; 
						if (pWptD110->dpth*MFT < 10000)
							sprintf (Wpt_depth,"%4.0f",pWptD110->dpth*MFT);
						else
							_fstrcpy (Wpt_depth,"Unknown"); 
					}  
						break;
					default:  
						sprintf (str,"GPS format %i is not supported",WptFormat);
						GSSiMessageBox (str,NULL,MB_ICONEXCLAMATION,0);
						return FALSE;
					
				}
  
				GetDMS (Wpt_posn->lat*SEMICIRCLETODEGREE,&Deg,&Min,&Sec); 
				sprintf (LatC,"N%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
				GetDMS (Wpt_posn->lon*SEMICIRCLETODEGREE,&Deg,&Min,&Sec); 
				sprintf (LonC,"W%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);

				sprintf (str,"%.12s\t%.8s\t%.7s\t%.30s\t%s\t%s\t0",Wpt_ident,"Fish",Wpt_depth,Wpt_cmnt,
															 LatC,LonC);  
		 		ConvertGPSLatLon (str);
				savec = str[12];   
				str[12]=0;
		 		index = SendDlgItemMessage (hWndDlg,Control,LB_FINDSTRING,(WPARAM)-1,(LPARAM) str); 
		 		str[12] = savec;
		 		if (index != LB_ERR)
		 		{
		 			SendDlgItemMessage (hWndDlg,Control,LB_DELETESTRING,(WPARAM)index,(LPARAM)0); 
		 			SendDlgItemMessage (hWndDlg,Control,LB_INSERTSTRING,(WPARAM)index,(LPARAM) str); 
		 		}
		 		else
		 			index = SendDlgItemMessage (hWndDlg,Control,LB_ADDSTRING,(WPARAM)0,(LPARAM) str); 
		 		SendDlgItemMessage (hWndDlg,Control,LB_SETTOPINDEX,(WPARAM)index,(LPARAM)0); 
                PctBox (GetDlgItem(hWndDlg,StatusControl),nRecs,++nLoaded,-1);
		 	}
				break;
		}
	} 
	ContinueProcessing = TRUE;
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete"); 
	GSSiGlobUlFree (&hWPMem);
//	SetCursor (hcurSave);
	return TRUE;
}


BOOL GPSImportLSI100WP (HWND hWndDlg,UINT Control,UINT StatusControl,BOOL SearchALL)
{ 
	LSI100HEADER	Header; 
	LSI100WAYPOINT	WayPoint;
	WORD			WPNum=1;
	HCURSOR	hcurSave; 
	char	str[256], CTime[32];
	long	nLoaded=0, nSearched=0,nRecs,n;
	
	nRecs = n =MaxLowranceWP;  
Top:  
	WPNum = 0;  
//	hCursor = LoadCursor(NULL, IDC_WAIT);
//	hcurSave = SetCursor(hCursor); 
	
	SetDlgItemText (hWndDlg,StatusControl,"Requesting waypoints");   
	WayPoint.Status = 1; 
	if (SearchALL)
	{
		GSSiGlobFree (&hLowranceWPList);
		hLowranceWPList = GSSiGlobAlloc (0,GHND,(MaxLowranceWP+1)*18);
	}  
	
	while (n-- && ContinueProcessing)
	{
		CreateLSI100Header (LSI100GetWaypoint,2,&Header);   
		if (!SendLSI100Header (GPSStream,&Header,(LPBYTE)&WPNum,(LPBYTE)&WayPoint))  
		{ 
	ErMsg:  
//			GSSiSetCursor (hcurSave);
			if (MessageBox (hWndDlg,"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
				NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
				goto Top;
		   	return FALSE; 
		}
		if (WayPoint.Status)
		{ 
			LSTRUC	LatLon; 
			char	LatC[32],LonC[32], Name[14],savec;
			short	Deg, Min,ndp=1, index;
			double	Sec;    
			
			if (SearchALL)
			{ 
				LPSTR	pList = GlobalLock (hLowranceWPList);
							
				pList += (WayPoint.Number*18);
				strncpy0 (pList,WayPoint.Name,12);
				Truncate (pList);  
				*(LPLONG)(pList+14) = WayPoint.Date;
				GlobalUnlock (hLowranceWPList);
			}    
			sprintf (Name,"WP%4.4i      ",(short)(WayPoint.Number+1));
			LatLon = Merc_2_Degree (WayPoint.Lat,WayPoint.Lon);	
			GetDMS (LatLon.Lat,&Deg,&Min,&Sec); 
			sprintf (LatC,"N%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
			GetDMS (LatLon.Lon,&Deg,&Min,&Sec); 
			sprintf (LonC,"W%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
            sprintf (CTime,"$CAL(%ld,3)",WayPoint.Date+SecondsFrom1970to1992); 
            ExpandText (CTime);
			sprintf (str,"%.12s\t%.8s\t%.7s\t%.30s\t%s\t%s\t0",WayPoint.Name,"Fish","",CTime,
														 LatC,LonC);  
		 	ConvertGPSLatLon (str);
			savec = str[12];   
			str[12]=0;
	 		index = SendDlgItemMessage (hWndDlg,Control,LB_FINDSTRING,(WPARAM)-1,(LPARAM) str); 
	 		str[12] = savec;
	 		if (index != LB_ERR)
	 		{
	 			SendDlgItemMessage (hWndDlg,Control,LB_DELETESTRING,(WPARAM)index,(LPARAM)0); 
	 			SendDlgItemMessage (hWndDlg,Control,LB_INSERTSTRING,(WPARAM)index,(LPARAM) str); 
	 		}
	 		else
	 			index = SendDlgItemMessage (hWndDlg,Control,LB_ADDSTRING,(WPARAM)0,(LPARAM) str); 
	 		SendDlgItemMessage (hWndDlg,Control,LB_SETTOPINDEX,(WPARAM)index,(LPARAM)0); 
	 		sprintf (str,"%ld points loaded",nLoaded++);
	 		SetDlgItemText (hWndDlg,StatusControl,str);
	 	} 
	 	else if (!SearchALL)
	 		break;
        WPNum++;
        PctBox (GetDlgItem(hWndDlg,StatusControl),nRecs,++nSearched,-1);
	} 
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete");
	ContinueProcessing = TRUE;
//	SetCursor (hcurSave);
	return TRUE;
}  

BOOL GetLowranceWPList (BOOL Delete)
{
	if (Delete)
		GSSiGlobFree (&hLowranceWPList);
	else if (hLowranceWPList)
		return TRUE;
	else
	{
		if (!ScanLowrance (1))
			return FALSE;
	}
	return TRUE;
}

BOOL HaveFreeLowranceWPSlots (int nItems,HANDLE hItems,HWND hWndDlg,UINT Control)
{   
	short	n=0,i, j, EmptySlots, nWP, nSlotsNeeded=nItems, nToFree; 
	LPSTR	pList, pTAB; 
	LPINT	pItems;    
	char	str[256];
	
	if (!GetLowranceWPList(FALSE))
		return FALSE;
	pList = GlobalLock (hLowranceWPList);
	for (i=0;i<MaxLowranceWP;i++)
	{
		if (*pList)
			n++;
		pList += 18;
	}
	GlobalUnlock (hLowranceWPList);
	EmptySlots = MaxLowranceWP - n;
	if (EmptySlots >= nSlotsNeeded)
		return TRUE;
	pItems = (LPINT)GlobalLock (hItems);
	for (j=0;j<nItems;j++)
	{
		SendDlgItemMessage (hWndDlg,Control,LB_GETTEXT,pItems[j],(LPARAM)str);  
		if ((pTAB = _fstrchr (str,'\t')))
			*pTAB = 0; 
		Truncate (str);
		pList = GlobalLock (hLowranceWPList);
		for (i=0;i<MaxLowranceWP;i++)
		{
			if (!_fstricmp (str,pList))
			{
				nSlotsNeeded--;
				break;
			}
			pList += 18;
		}
		GlobalUnlock (hLowranceWPList);
	} 
	GlobalUnlock (hItems);
	if (EmptySlots >= nSlotsNeeded)
		return TRUE;
	nToFree = nSlotsNeeded - EmptySlots;   
	if (nToFree > MaxLowranceWP)
	{ 
		MessageBox (0,"You have selected more waypoints than your GPS receiver can hold",NULL,MB_ICONEXCLAMATION);
		return FALSE;
	}
	if (!LowranceReplaceOpt)
		if (!ScanLowrance (3))
			return FALSE;
	switch (LowranceReplaceOpt)
	{   
		case 1: //first
		
		for (j=0;j<nToFree;j++)
		{
			pList = GlobalLock (hLowranceWPList);
			for (i=0;i<MaxLowranceWP;i++)
			{
				if (*pList)
				{
					*pList = 0;
					break;
				}
				pList += 18;
			}
			GlobalUnlock (hLowranceWPList);
		}
		break;
		
		case 2://oldest
		for (j=0;j<nToFree;j++)
		{   
			long	Oldest=LONG_MAX;
			LPSTR	pOldest;

			pList = GlobalLock (hLowranceWPList);
			for (i=0;i<MaxLowranceWP;i++)
			{
				if (*pList)
				{   
					long	date = *(LPLONG)(pList+14);
					
					if (date < Oldest)
					{
						Oldest = date;
						pOldest = pList;
					}
				}
				pList += 18;
			} 
			*pOldest = 0;
			GlobalUnlock (hLowranceWPList);
		}
		break;
	}
	return TRUE;
}

short GetLowranceWPNumber (LPSTR WPNameIn,long date)
{ 
	short	i; 
	LPSTR	pList; 
	char	WPName[32];
	
	_fstrcpy (WPName,WPNameIn);
	Truncate (WPName);
	pList = GlobalLock (hLowranceWPList);
	for (i=0;i<MaxLowranceWP;i++)
	{
		if (!*pList || !_fstricmp (pList,WPName))
		{
			strncpy0 (pList,WPName,12); 
			Truncate (pList);
			*(LPLONG)(pList+14) = date;
			GlobalUnlock (hLowranceWPList);
			return i;
		}
		pList += 18;
	}
	GlobalUnlock (hLowranceWPList);
    
    return 0;
}

short GPSExportLSI100WP (HWND hWndDlg,UINT Control,UINT StatusControl,BOOL SearchALL,LPHANDLE phWPNums,short RouteNum)
{
	LSI100HEADER	Header; 
	LSI100WAYPOINT	Wpt;   
	BYTE	Cmd;
	Merc	T;
	char	str[256];	 
	HCURSOR	hcurSave;    
	HANDLE	hItems=0;
	LPINT	lpItems,lpWPNums; 
	LPSTR	pLat,pLon, pTAB, pOpts, pSym, pDesc, pRef;
	double	lat,lon;
	int		n, nItems;    
	BOOL	Err;
	long	nRecs, nLoaded=0; 
	LPBYTE	Wpt_ident, Wpt_cmnt=0, Wpt_smbl, Wpt_attr=0; 
	long	Ref; 
	short	WPInRouteNumber=1;
	
	nItems=GetLBSelectedItems (hWndDlg,Control,&hItems);  
	if (!nItems)
		return FALSE;
	if (!HaveFreeLowranceWPSlots (nItems,hItems,hWndDlg,Control)) 
	{
		GSSiGlobFree (&hItems);
		return FALSE; 
	}
	if (phWPNums)
	{
		*phWPNums = GSSiGlobAlloc (1039,GMEM_MOVEABLE,nItems*4);
		lpWPNums=  (LPINT) GlobalLock(*phWPNums);  
	}
	lpItems=  (LPINT) GlobalLock(hItems);
Top:    
//	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
	Wpt_ident = (LPBYTE)&Wpt.Name;
	Wpt_cmnt = 0;
	Wpt_smbl = &Wpt.Symbol; 
	
	n = nRecs = nItems;
	nLoaded = 0;
	_fmemset (&Wpt,0,sizeof(Wpt));    
	while (n--)
	{   
		SendDlgItemMessage (hWndDlg,Control,LB_GETTEXT,*lpItems++,(LPARAM)str);
			Ref=1;
		if ((pRef = _fstrrchr (str,'\t'))) 
		{  
			if (IsInteger(pRef+1))
			{
				*pRef++ = 0;
				Ref = atol (pRef); 
			}
		} 
		Wpt.Status = TRUE; 
		pTAB = _fstrchr (str,'\t');
		*pTAB++ = 0;   
		_fstrncpy (Wpt_ident,str,12);   
		if (phWPNums)
			sprintf (Wpt_ident,"RT%i%i",RouteNum+1,WPInRouteNumber++);
		Wpt.Date = time(NULL)-SecondsFrom1970to1992;
		Wpt.Number = GetLowranceWPNumber (Wpt.Name,Wpt.Date);
		PadString (Wpt_ident,' ',12);
		Wpt.Name[12] = ' ';
		pSym = pTAB;
		pTAB = _fstrchr (pSym,'\t');
		*pTAB++ = 0;
		pDesc = pTAB;
		pTAB = _fstrchr (pDesc,'\t');
		*pTAB++ = 0; 
//		if (Ref != -1) //indicate track log
		{ 
			pOpts = pTAB;
			pTAB = _fstrchr (pOpts,'\t');
			*pTAB++ = 0; 
		} 
		pLat = pTAB;
		pTAB = _fstrchr (pLat,'\t');
		*pTAB++ = 0;
		pLon = pTAB;
		lat = DecDegFromDMS (pLat,&Err); 
		lon = DecDegFromDMS (pLon,&Err);
		T = Degree_2_Merc(lat,lon); 
		Wpt.Lat = T.Lat;
		Wpt.Lon = T.Lon;  

		if (Wpt_smbl)
        	*Wpt_smbl = 0;
		CreateLSI100Header (LSI100SendWaypoint,sizeof(Wpt)-1,&Header);   
		if (!SendLSI100Header (GPSStream,&Header,(LPBYTE)&Wpt.Number,0))  
		{ 
	ErMsg:  
//			GSSiSetCursor (hcurSave);
			if (MessageBox (hWndDlg,"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
				NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
				goto Top; 
			if (phWPNums)
				GSSiGlobUlFree (phWPNums);
		   	return FALSE; 
		}
		if (phWPNums)  
			*lpWPNums++ = Wpt.Number; 
        PctBox (GetDlgItem(hWndDlg,StatusControl),nRecs,++nLoaded,-1);
	}
//	GSSiSetCursor (hcurSave); 
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete");
	GSSiGlobUlFree (&hItems);
	if (phWPNums)
		GlobalUnlock (*phWPNums);
	return nItems;
} 

short GPSExportLSI100Route (HWND hWndDlg,UINT Control,UINT StatusControl,BOOL SearchALL)
{
	LSI100HEADER	Header; 
	LPLSI100ROUTE	pRte;   
	BYTE	Cmd;
	char	str[256];	 
	HCURSOR	hcurSave;    
	WORD	RouteNumber;    
	long	nRecs, nLoaded=0; 
	HANDLE	hWPNums=0, hRoute=0;
	short	nItems, sizeRte,i;  
	LPSHORT	pWPNums;
	
	nItems=(short)SendDlgItemMessage(hWndDlg ,Control,LB_GETSELCOUNT,0,0); 
	if (nItems > MaxLowranceRouteWP)
	{   
		sprintf (str,"This GPS unit has a limit of %i waypoints per route./r/nYou have selected %i waypoints",
				     MaxLowranceRouteWP,nItems);
		MessageBox (hWndDlg,str,NULL,MB_ICONEXCLAMATION);
   		return FALSE; 
	}
	*str=0;
	if (!GetTextString (hWndDlg,str,5,"Enter Route Number",NULL,NULL,0,TRUE,TRUE))
		return FALSE; 
	sizeRte = sizeof(LSI100ROUTE)+nItems*sizeof(WORD);
	hRoute = GSSiGlobAlloc (1040,GMEM_MOVEABLE,sizeRte);
	pRte = (LPLSI100ROUTE)GlobalLock (hRoute);
	pRte->Number = max (0,atoi (str)-1); 
	pRte->nWP = nItems;
	*str=0;
	if (!GetTextString (hWndDlg,str,20,"Enter Route Name",NULL,NULL,0,TRUE,TRUE))
		goto RtnFalse; 
	_fstrupr (str); 
	_fstrncpy (pRte->Name,str,12); 
	PadString (pRte->Name,' ',12);    
	pRte->Name[12] = ' ';

	if (!(nItems = GPSExportLSI100WP (hWndDlg,Control,StatusControl,SearchALL,&hWPNums,pRte->Number)))
		goto RtnFalse; 
	pWPNums = (LPSHORT)GlobalLock (hWPNums);
	for (i=0;i<nItems;i++)
		pRte->WPNum[i] = pWPNums[i];
	GSSiGlobUlFree (&hWPNums);
Top:		
	CreateLSI100Header (LSI100SendRoute,sizeRte-1,&Header);   
	if (!SendLSI100Header (GPSStream,&Header,(LPBYTE)&pRte->Number,0))  
	{ 
//		GSSiSetCursor (hcurSave);
		if (MessageBox (hWndDlg,"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
			NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
			goto Top;
	   	goto RtnFalse;  
	}  
//	GSSiSetCursor (hcurSave); 
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete");
RtnFalse:
	GSSiGlobUlFree (&hWPNums);  
	GSSiGlobUlFree (&hRoute);
	return nItems;
} 

BOOL GPSImportMagellanWP (HWND hWndDlg,UINT Control,UINT StatusControl)
{ 
	LSI100HEADER	Header; 
	LSI100WAYPOINT	WayPoint;
	WORD			WPNum=1;
	HCURSOR	hcurSave; 
	char	str[256],Packet[512],Depth[32];  
	short	l,ii;
	long	nLoaded=0, nRecs;  
Top:  
	WPNum = 0;  
//	hCursor = LoadCursor(NULL, IDC_WAIT);
//	hcurSave = SetCursor(hCursor); 
	
	SetDlgItemText (hWndDlg,StatusControl,"Requesting waypoints");   
	if (!SendMagellanPacket ("WAYPOINT"))
		return FALSE; 
	Sleep (500);
Next:
	if (!(l=GetMagellanPacket (Packet)))
	{
		if (MessageBox (GetFocus(),"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
			NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
			goto Top;
		return FALSE; 
	} 
	if (!_fstrnicmp (Packet,"$PMGNVER",8))
		goto Next;
	WayPoint.Status = 1;
	while (!_fstrnicmp (Packet,"$PMGNWPL,",9) && ContinueProcessing)
	{
		LPSTR	pLat,pLon,pName,pDesc,pSym,pDash;
		double	lat,lon, latdeg, latmin, londeg, lonmin; 
		short	ii;  
		char	LatC[32],LonC[32], Name[12],savec;
		short	Deg, Min,ndp=1, index;
		double	Sec;    
					 					
		pLat=&Packet[9];
		pLon=_fstrchr (pLat,',');
		pLon++;
		pLon=_fstrchr (pLon,',');
		pLon++;      
		latdeg = ldread (pLat,2); 
		pLat+=2;
		latmin = atof (pLat);
		lat = latdeg + latmin/60;
		londeg = ldread (pLon,3); 
		pLon+=3;
		lonmin = atof (pLon);
		lon = londeg + lonmin/60;
		pDesc = _fstrchr (pLon,',');
		pDesc++;
		pDesc = _fstrchr (pDesc,',');
		pDesc++;
		pDesc = _fstrchr (pDesc,',');
		pDesc++;
		pDesc = _fstrchr (pDesc,',');
		pDesc++; 
		pName = pDesc;
		pDesc = _fstrchr (pDesc,',');
		*pDesc++ = 0;    
		pSym = pDesc;
		pSym = _fstrchr (pDesc,',');
		*pSym++ = 0;
			
		GetDMS (lat,&Deg,&Min,&Sec); 
		sprintf (LatC,"N%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
		GetDMS (lon,&Deg,&Min,&Sec); 
		sprintf (LonC,"W%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec); 
		if ((pDash = _fstrchr (pDesc,'-')))
		{
			*pDash++=0;
			_fstrcpy (Depth,pDesc);
			if (*Depth == '?')
				_fstrcpy (Depth,"Unknown");
			pDesc = pDash;
		} 
		else
			*Depth = 0;
		sprintf (str,"%.12s\t%.8s\t%.7s\t%.30s\t%s\t%s\t0",pName,"Fish",Depth,pDesc,
													 LatC,LonC);  
	 	ConvertGPSLatLon (str);
		savec = str[12];   
		str[12]=0;
 		index = SendDlgItemMessage (hWndDlg,Control,LB_FINDSTRING,(WPARAM)-1,(LPARAM) str); 
 		str[12] = savec;
 		if (index != LB_ERR)
 		{
 			SendDlgItemMessage (hWndDlg,Control,LB_DELETESTRING,(WPARAM)index,(LPARAM)0); 
 			SendDlgItemMessage (hWndDlg,Control,LB_INSERTSTRING,(WPARAM)index,(LPARAM) str); 
 		}
 		else
 			index = SendDlgItemMessage (hWndDlg,Control,LB_ADDSTRING,(WPARAM)0,(LPARAM) str); 
 		SendDlgItemMessage (hWndDlg,Control,LB_SETTOPINDEX,(WPARAM)index,(LPARAM)0); 
 		sprintf (str,"%ld points loaded",++nLoaded);
 		SetDlgItemText (hWndDlg,StatusControl,str);
        WPNum++;
		CheckForContinue (TRUE,0);    
		if (!(l=GetMagellanPacket (Packet)))
		{
			if (MessageBox (GetFocus(),"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
				NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
				goto Top;
			return FALSE; 
		} 
	} 
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete");
	ContinueProcessing = TRUE;
//	SetCursor (hcurSave);
	return TRUE;
} 

short GPSExportMagellanWP (HWND hWndDlg,UINT Control,UINT StatusControl,BOOL Route)
{
	BYTE	Cmd;
	Merc	T;
	char	str[256],Packet[256],dirlat,dirlon,Sym='a', Name[32],DepAndDesc[128];	 
	HCURSOR	hcurSave;    
	HANDLE	hItems;
	LPINT	lpItems; 
	LPSTR	pLat,pLon, pTAB, pOpts, pSym, pDesc, pRef, pDepth;
	double	lat,lon, minlat,minlon,decpart;
	BOOL	Err;
	int		n, nItems, deglat,deglon;    
	long	nRecs, nLoaded=0; 
	long	Ref;
	
	nItems = GetLBSelectedItems (hWndDlg,Control,&hItems);
	lpItems=  (LPINT) GlobalLock(hItems);
Top:    
//	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
	
	n = nRecs = nItems;
	nLoaded = 0;
	while (n--)
	{   
		SendDlgItemMessage (hWndDlg,Control,LB_GETTEXT,*lpItems++,(LPARAM)str);
		if ((pRef = _fstrrchr (str,'\t'))) 
		{
			*pRef++ = 0;
			Ref = atol (pRef);
		} 
		else
			Ref=1;
		pTAB = _fstrchr (str,'\t');
		*pTAB++ = 0;  
		if (Route)
			sprintf (Name,"RT%i%ld",Route,nLoaded+1);
		else
			_fstrcpy (Name,str);
//		_fstrncpy (Wpt_ident,str,6);
//		PadString (Wpt_ident,' ',6);
		pSym = pTAB;
		pTAB = _fstrchr (pSym,'\t');
		*pTAB++ = 0;
		pDepth = pTAB;
		pTAB = _fstrchr (pDepth,'\t');
		*pTAB++ = 0; 
//		if (Ref != -1) //indicate track log
		{ 
			pDesc = pTAB;
			pTAB = _fstrchr (pDesc,'\t');
			*pTAB++ = 0; 
		} 
		pLat = pTAB;
		pTAB = _fstrchr (pLat,'\t');
		*pTAB++ = 0;
		pLon = pTAB;
		lat = DecDegFromDMS (pLat,&Err);
		deglat = fabs (lat);    
		if (lat < 0)
			dirlat = 'S';
		else
			dirlat = 'N';
		decpart = fmod (fabs(lat),1.0);
		minlat = 60.0 * decpart;
		lon = DecDegFromDMS (pLon,&Err); 
		deglon = fabs (lon);     
		if (lon < 0)
			dirlon = 'W';
		else
			dirlon = 'E';
		decpart = fmod (fabs(lon),1.0);
		minlon = 60.0 * decpart;
		if (!_fstricmp (pDepth,"Unknown"))  
			sprintf (DepAndDesc,"?-%s",pDesc); 
		else
			sprintf (DepAndDesc,"%s-%s",pDepth,pDesc);
		_fstrupr (DepAndDesc);
		sprintf (Packet,"$PMGNWPL,%2.2i%06.3f,%c,%3.3i%06.3f,%c,,,%s,%s,%c",
						deglat,minlat,dirlat,
						deglon,minlon,dirlon,
						Name,DepAndDesc,Sym);
		if (!SendMagellanPacket (Packet))  
		{ 
	ErMsg:  
//			GSSiSetCursor (hcurSave);
			if (MessageBox (hWndDlg,"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
				NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
				goto Top;
		   	return FALSE; 
		}  
        PctBox (GetDlgItem(hWndDlg,StatusControl),nRecs,++nLoaded,-1);
	} 
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete");
//	GSSiSetCursor (hcurSave); 
	GSSiGlobUlFree (&hItems);
	return nItems;
}  

short GPSExportMagellanRoute (HWND hWndDlg,UINT Control,UINT StatusControl)
{
	BYTE	Cmd;
	Merc	T;
	char	str[256],Packet[256],dirlat,dirlon,Sym='a',Name[32];	 
	HCURSOR	hcurSave;    
	LPINT	lpItems; 
	LPSTR	pLat,pLon, pTAB, pOpts, pSym, pDesc, pRef;
	double	lat,lon, minlat,minlon,decpart;
	short	Err, n,i,j, nItems, deglat,deglon,RouteNumber, nRecs;    
	long	nLoaded=0; 
	long	Ref;
	
	*str=0;
	if (!GetTextString (hWndDlg,str,20,"Enter Route Number",NULL,NULL,0,TRUE,TRUE))
		return FALSE;
	RouteNumber = max (1,atoi (str));
	nItems=(short)SendDlgItemMessage(hWndDlg ,Control,LB_GETSELCOUNT,0,0); 
	if (!GPSExportMagellanWP (hWndDlg,Control,StatusControl,RouteNumber))
		return FALSE;
Top:    
//	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
	nRecs = (nItems-1)/10+1; 
	j=1;
	for (i=0;i<nRecs;i++) 
	{
		sprintf (Packet,"$PMGNRTE,%i,%i,c,%i",nRecs,i+1,RouteNumber);
		for (n = i*10;n<min(i*10+10,nItems);n++)
			sprintf (_fstrchr(Packet,0),",RT%i%i",RouteNumber,j++);
		if (!SendMagellanPacket (Packet))  
		{ 
ErMsg:  
//			GSSiSetCursor (hcurSave);
			if (MessageBox (hWndDlg,"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
				NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
				goto Top;
		   	return FALSE; 
		 }
	}  
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete");
//	GSSiSetCursor (hcurSave); 
	return nItems;
}  

BOOL GPSImportTrack (HWND hWndDlg,UINT Control,UINT StatusControl) 
{
	switch (GPSFormat)
	{
		case 1000:
			return GPSImportTrackGarmin (hWndDlg,Control,StatusControl);
		case 2000:
			return GPSImportTrackMagellan (hWndDlg,Control,StatusControl);
		case 3000:
			return GPSImportTrackLSI100 (hWndDlg,Control,StatusControl);
	} 
	return FALSE;
}

BOOL GPSImportTrackMagellan (HWND hWndDlg,UINT Control,UINT StatusControl)
{ 
	LSI100HEADER	Header; 
	LSI100WAYPOINT	WayPoint;
	WORD			WPNum=1; 
	LPSTR	pTime, pValid;
	HCURSOR	hcurSave; 
	char	str[256],Packet[512], WPName[12]; 
	short	l,ii;
	long	nLoaded=0, nRecs;  
Top:  
	WPNum = 0;  
//	hCursor = LoadCursor(NULL, IDC_WAIT);
//	hcurSave = SetCursor(hCursor); 
	
	SetDlgItemText (hWndDlg,StatusControl,"Requesting waypoints");   
	if (!SendMagellanPacket ("TRACK"))
		return FALSE; 
	if (!(l=GetMagellanPacket (Packet)))
	{
		if (MessageBox (GetFocus(),"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
			NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
			goto Top;
		return FALSE; 
	} 
	WayPoint.Status = 1;
	while (!_fstrnicmp (Packet,"$PMGNTRK,",9) && ContinueProcessing)
	{
		LPSTR	pLat,pLon,pName,pDesc,pSym;
		double	lat,lon, latdeg, latmin, londeg, lonmin; 
		short	ii;  
		char	LatC[32],LonC[32], Name[12],savec;
		short	Deg, Min,ndp=1, index;
		double	Sec;    
					 					
		pLat=&Packet[9];
		pLon=_fstrchr (pLat,',');
		pLon++;
		pLon=_fstrchr (pLon,',');
		pLon++;      
		latdeg = ldread (pLat,2); 
		pLat+=2;
		latmin = atof (pLat);
		lat = latdeg + latmin/60;
		londeg = ldread (pLon,3); 
		pLon+=3;
		lonmin = atof (pLon);
		lon = londeg + lonmin/60;
		pDesc = _fstrchr (pLon,',');
		pDesc++;
		pDesc = _fstrchr (pDesc,',');
		pDesc++;
		pDesc = _fstrchr (pDesc,',');
		pDesc++;
		pDesc = _fstrchr (pDesc,',');
		pDesc++; 
		pTime = pDesc;
		pDesc = _fstrchr (pDesc,',');
		*pDesc++ = 0;    
		pValid = pDesc;
		
		sprintf (WPName,"TL%i",WPNum+1);	
		GetDMS (lat,&Deg,&Min,&Sec); 
		sprintf (LatC,"N%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
		GetDMS (lon,&Deg,&Min,&Sec); 
		sprintf (LonC,"W%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
		sprintf (str,"%.6s\t%.8s\t%.20s\t%s\t%s\t%s\t0",WPName,"",pTime,pValid,
													 LatC,LonC);  
		index = SendDlgItemMessage (hWndDlg,Control,LB_ADDSTRING,(WPARAM)0,(LPARAM) str); 
 		SendDlgItemMessage (hWndDlg,Control,LB_SETTOPINDEX,(WPARAM)index,(LPARAM)0); 
 		sprintf (str,"%ld points loaded",++nLoaded);
 		SetDlgItemText (hWndDlg,StatusControl,str);
        WPNum++;
		CheckForContinue (TRUE,0);    
		if (!(l=GetMagellanPacket (Packet)))
		{
			if (MessageBox (GetFocus(),"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
				NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
				goto Top;
			return FALSE; 
		} 
	} 
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete");
	ContinueProcessing = TRUE;
//	SetCursor (hcurSave);
	return TRUE;
} 

BOOL GPSImportTrackLSI100 (HWND hWndDlg,UINT ListControl,UINT StatusControl)
{
	LSI100HEADER	Header; 
	LSI100PLOTTRAIL	PlotTrail;
	LPLSI100PLOTTRAILDELTAS	pPlotTrailDeltas; 
	LSI100PLOTTRAILREQUEST	PTRequest;
	HANDLE	hPTD=0, hDeltas=0;
	HCURSOR	hcurSave;  
	BYTE	PTNum=0;
	char	str[256];
	long	nLoaded=0, nSearched=0,nRecs,n;  
	LPDELTA	pDeltas;
	short	i,nTot=0;
	
	if (!LowranceProtocolVersion || !MaxLowrancePlotTrails)
		return FALSE;
Top:
	nTot = 0;  
	CreateLSI100Header (LSI100GetPlotTrail,2,&Header);   
	if (!SendLSI100Header (GPSStream,&Header,(LPBYTE)&PTNum,(LPBYTE)&PlotTrail))  
	{ 
	ErMsg:  
		GSSiGlobUlFree (&hPTD);
		if (MessageBox (hWndDlg,"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
			NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
			goto Top;
	   	return FALSE; 
	} 
	hPTD = GSSiGlobAlloc (1042,GHND,sizeof(LSI100PLOTTRAILDELTAS)*MAXDELTATRANSFER*sizeof(DELTA)); 
	pPlotTrailDeltas = (LPLSI100PLOTTRAILDELTAS)GlobalLock (hPTD);
	hDeltas = GSSiGlobAlloc (1042,GHND,PlotTrail.NumberOfDeltas*sizeof(DELTA)); 
	pDeltas = (LPDELTA)GlobalLock (hDeltas); 
	while (nTot < PlotTrail.NumberOfDeltas)
	{
		CreateLSI100Header (LSI100GetPlotTrailDeltas,sizeof(LSI100PLOTTRAILREQUEST),&Header); 
		PTRequest.PlotTrailNumber = PlotTrail.PlotTrailNumber;
		PTRequest.NumberOfDeltas = min (PlotTrail.NumberOfDeltas-nTot,MAXDELTATRANSFER);
		if (!SendLSI100Header (GPSStream,&Header,(LPBYTE)&PTRequest,(LPBYTE)pPlotTrailDeltas))
		{
			GSSiGlobUlFree (&hPTD);
			GSSiGlobUlFree (&hDeltas);
			goto ErMsg;  
		}
		for (i=0;i<pPlotTrailDeltas->NumberOfDeltas;i++)
			pDeltas[nTot++] = pPlotTrailDeltas->Deltas[i]; 
	}
	GSSiGlobUlFree (&hPTD);
	for (i=0;i<PlotTrail.NumberOfDeltas;i++) 
	{ 
		LSTRUC	LatLon; 
		char	LatC[32],LonC[32], Name[12],savec;
		short	Deg, Min,ndp=1, index;
		double	Sec;    
			
		sprintf (Name,"TL%4.4i      ",i+1);
		LatLon = Merc_2_Degree (PlotTrail.OriginY+pDeltas[i].DeltaY,
								PlotTrail.OriginX+pDeltas[i].DeltaX);	
		GetDMS (LatLon.Lat,&Deg,&Min,&Sec); 
		sprintf (LatC,"N%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
		GetDMS (LatLon.Lon,&Deg,&Min,&Sec); 
		sprintf (LonC,"W%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);

		sprintf (str,"%.12s\t%.8s\t%.7s\t%.30s\t%s\t%s\t0",Name,"","","",
													 LatC,LonC);  
		index = SendDlgItemMessage (hWndDlg,ListControl,LB_ADDSTRING,(WPARAM)0,(LPARAM) str); 
 		SendDlgItemMessage (hWndDlg,ListControl,LB_SETTOPINDEX,(WPARAM)index,(LPARAM)0); 
 		sprintf (str,"%ld points loaded",nLoaded++);
 		SetDlgItemText (hWndDlg,StatusControl,str);
 	} 
	
//		Deltas[i] = pPlotTrailDeltas->Deltas[i];
	GSSiGlobUlFree (&hDeltas);
	return TRUE;
}

BOOL GPSImportTrackGarmin (HWND hWndDlg,UINT ListControl,UINT StatusControl)
{ 
	char	str[256], TimeC[32], New;	 
	BYTE	Packet[270];  
	short	PacketID=0,i;  
	static	DWORD	BeginOfTime=0;
	D300_Trk_Point_Type	*pTrk;      
	HCURSOR	hcurSave;  
	long	nLoaded, nRecs,TPRef=0;  
	short	nerr;
	
	struct	tm btm;
	btm.tm_year=89;
	btm.tm_mon=11;
	btm.tm_mday=31; 
	btm.tm_hour=0;
	btm.tm_min=0;
	btm.tm_sec=0;
	btm.tm_isdst = -1;
	BeginOfTime = mktime (&btm);
Top:    
//	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
	CreatePacket (send_request,Cmnd_Transfer_Trk,Packet);   
	if (!SendPacket (GPSStream,Packet))  
	{ 
ErMsg:  
//		GSSiSetCursor (hcurSave);
		if (MessageBox (hWndDlg,"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
			NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
			goto Top;
	   	return FALSE; 
	} 
	i=1;  
	nerr=0;
	while (PacketID != Pid_Xfer_Cmplt && ContinueProcessing)
	{   
		PacketID = GetPacket (GPSStream,Packet);
		switch (PacketID)
		{ 
			case 0: 
				//if (!nerr++)
					break;
				goto ErMsg; 
			case Pid_Records:
				nLoaded = 0;
				nRecs = *(LPSHORT)&Packet[3];
				break;  
			case Pid_Xfer_Cmplt:
                PctBox (GetDlgItem(hWndDlg,StatusControl),nRecs,++nLoaded,-1);
				break;
			case Pid_Trk_Data:
			{   
				double lat,lon; 
				char	LatC[32],LonC[32],savec;
				short	Deg, Min,ndp=1, index;        
				double	Sec;   
				LPSTR	ptime;
				DWORD	gpstime;  
				time_t	t;
				
				pTrk = (LPD300_Trk_Point_Type)&Packet[3];  
				GetDMS (pTrk->posn.lat*SEMICIRCLETODEGREE,&Deg,&Min,&Sec); 
				sprintf (LatC,"N%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
				GetDMS (pTrk->posn.lon*SEMICIRCLETODEGREE,&Deg,&Min,&Sec); 
				sprintf (LonC,"W%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
                gpstime = pTrk->time + BeginOfTime;
				t = gpstime;
                ptime = ctime(&t);
                if (ptime)              
					_fstrncpy (TimeC,ptime,24);
				else
					*TimeC = 0;
				TimeC[24]=0;
				if (pTrk->new_trk)
					New = 'X';
				else	
					New = ' '; 
				sprintf (str,"TP%i\t%s\t\t%s\t%s\t-1",i++,TimeC,LatC,LonC);  
	 			index = SendDlgItemMessage (hWndDlg,ListControl,LB_ADDSTRING,(WPARAM)0,(LPARAM) str); 
                PctBox (GetDlgItem(hWndDlg,StatusControl),nRecs,++nLoaded,-1);
		 	}
				break;
		}
	} 
	ContinueProcessing = TRUE;
//	GSSiSetCursor (hcurSave);
	return TRUE;
} 




BOOL FAR PASCAL LOWRANCESCANMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{
    short	Choice;
	char	str[128];   
	int		ii;    
	BOOL	rtn;
		
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
       	cwCenter(hWndDlg, 0);   
       	switch (LowranceScanOption)
       	{
       		case 1: 
       			SetWindowText (hWndDlg,"Scanning for existing waypoints");
         		PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
         		break;
       		case 2:
		        {
	       			SetWindowText (hWndDlg,"Deleting existing waypoints");
	         		PostMessage(hWndDlg, WM_COMMAND, IDC_DELETEALLWP, 0L);
	         	}
         		break;
         }
       	
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {

            case IDOK:
			{ 
				LSI100HEADER	Header; 
				LSI100WAYPOINT	WayPoint;
				WORD			WPNum=1;
				long	nLoaded=0,nRecs,n;
					
				nRecs = n =MaxLowranceWP;  
			Top:  
				WPNum = 1;  
			//	hCursor = LoadCursor(NULL, IDC_WAIT);
			//	hcurSave = SetCursor(hCursor); 
					
				SetDlgItemText (hWndDlg,IDC_STATUS,"Requesting waypoints");   
				WayPoint.Status = 1; 
				GSSiGlobFree (&hLowranceWPList);
				hLowranceWPList = GSSiGlobAlloc (0,GHND,(MaxLowranceWP+1)*18);
				Processing = TRUE;	
				while (n-- && ContinueProcessing)
				{
					CreateLSI100Header (LSI100GetWaypoint,2,&Header);   
					if (!SendLSI100Header (GPSStream,&Header,(LPBYTE)&WPNum,(LPBYTE)&WayPoint))  
					{ 
				ErMsg:  
//						GSSiSetCursor (hcurSave);
						if (MessageBox (hWndDlg,"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
							NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
							goto Top;
					   	goto Exit; 
					}
					if (WayPoint.Status)
					{ 
						LPSTR	pList = GlobalLock (hLowranceWPList);
							
						pList += (WayPoint.Number*18);
						strncpy0 (pList,WayPoint.Name,12); 
						Truncate (pList);  
						*(LPLONG)(pList+14) = WayPoint.Date;
						GlobalUnlock (hLowranceWPList);
					}    
			        PctBox (GetDlgItem(hWndDlg,IDC_STATUS),nRecs,WPNum++,-1);
				}
				rtn = ContinueProcessing; 
				Processing = FALSE;	
				SetDlgItemText (hWndDlg,IDC_STATUS,"Transfer complete");
				ContinueProcessing = TRUE;
			//	SetCursor (hcurSave);
			}  
            EndDialog(hWndDlg, rtn); 
        	break;
        	
        	case IDC_LOWREPLACEFIRST:
        		LowranceReplaceOpt = 1;
                EndDialog(hWndDlg, TRUE);
        		break;
        	
        	case IDC_LOWREPLACEOLDEST:
        		LowranceReplaceOpt = 2;
                EndDialog(hWndDlg, TRUE);
        		break;
        	
        	case IDC_DELETEALLWP:
			{ 
				LSI100HEADER	Header; 
				LSI100WAYPOINT	Wpt;
				WORD			WPNum=1;
				long	nLoaded=0,nRecs,n;
					
		 		if (MessageBox (hWndDlg,"Are you sure you wish to delete all waypoints from your GPS receiver?","Verify delete",MB_YESNO) == IDNO)
		 		{
		        	if (LowranceScanOption == 2)
		        		PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);  
		        	break;
		        }
				nRecs = n =MaxLowranceWP;  
			Top2:  
				WPNum = 0;  
			//	hCursor = LoadCursor(NULL, IDC_WAIT);
			//	hcurSave = SetCursor(hCursor); 
					
				SetDlgItemText (hWndDlg,IDC_STATUS,"Requesting waypoints");   
				Wpt.Status = 1; 
				GSSiGlobFree (&hLowranceWPList);
				hLowranceWPList = GSSiGlobAlloc (0,GHND,(MaxLowranceWP+1)*18);
				Processing = TRUE;	
				while (n-- && ContinueProcessing)
				{   
					Wpt.Status = FALSE;  
					Wpt.Number = WPNum;
					CreateLSI100Header (LSI100SendWaypoint,sizeof(Wpt)-1,&Header);   
					if (!SendLSI100Header (GPSStream,&Header,(LPBYTE)&Wpt.Number,0))  
					{ 
//						GSSiSetCursor (hcurSave);
						if (MessageBox (hWndDlg,"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
							NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
							goto Top2;
					   	goto Exit; 
					}
			        PctBox (GetDlgItem(hWndDlg,IDC_STATUS),nRecs,WPNum++,-1);
				} 
				rtn = ContinueProcessing; 
				Processing = FALSE;	
				SetDlgItemText (hWndDlg,IDC_STATUS,"All waypoints deleted");
				ContinueProcessing = TRUE;
			//	SetCursor (hcurSave);
			}  
            EndDialog(hWndDlg, rtn); 
        	break;

            case IDCANCEL: 
           Exit: 
				GSSiGlobFree (&hLowranceWPList);
            	if (Processing)
            	{
					Processing = FALSE;	
            		ContinueProcessing = FALSE; 
            	}
            	else
	                EndDialog(hWndDlg, FALSE);
                break; 
                 
           }
    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL ScanLowrance (short Option)
{   
	FARPROC	lpfnLOWRANCESCANMsgProc;
	BOOL	nRc;
	
	LowranceScanOption = Option;
	lpfnLOWRANCESCANMsgProc = MakeProcInstance((FARPROC)LOWRANCESCANMsgProc, hInst); 
	if (Option < 3)
		nRc = DialogBox(hInst, (LPSTR)"LOWRANCESCAN", hWndMain, lpfnLOWRANCESCANMsgProc); 
	else
		nRc = DialogBox(hInst, (LPSTR)"LOWRANCEREPLACE", hWndMain, lpfnLOWRANCESCANMsgProc);
	FreeProcInstance(lpfnLOWRANCESCANMsgProc);
	return nRc;   
}

BOOL GPSImportUSRWP (HWND hWndDlg,UINT Control,UINT StatusControl)
{ 
	WORD	WPNum=1;
	char	str[256], CTime[32],IconName[12];
	long	nLoaded=0, nSearched=0,nRecs,n;
	char	title[]="Load USR Waypoint File";
	long i;
	short MajorVersion;
	short MinorVersion;
	short NumWaypoints;
	short NumRoutes;
	short NumIcons;
	short NumTrails;
	short ObjectNum;
	float Depth;
    HFILE	Fid;
	tWaypoint Waypoint;
	tText ObjectName;
	tMCoord Position;
	long Altitude;  
	long	ltext;
	char	Text[256];  
	long	Time; 
	short	Time2;
	short	SymbolID;
	short	WaypointType;
//    char	temp[1024];  
	LSTRUC	LatLon; 

#pragma pack(2)
	struct{
	 unsigned short ObjectID;          // Numeric identifier of the Waypoint..
	 long           Latitude;           // Latitude component of a position..
	 long           Longitude;          // Longitude component of a position..
	 long           Altitude;          // Waypoint's altitude..
	 char*          Name;              // Waypoint's name..
	 char*          Comment;           // Comments about the Waypoint..
	 unsigned long  Time;              // Creation date of the Waypoint..
	 unsigned long  SymbolID;          // Graphic symbol associated with the Waypoint 
	 unsigned short WaypointType;      // Type of Waypoint, among USER CREATED,                         
									   //  TEMPORARY and POINT OF INTEREST (see 
									   //  description of these types below)..
	 float          Depth;             // Waypoint's depth (new in Version 3.0)
	} nWaypoint ; 
#pragma pack()


	WPNum = 0;  
	strcpy (OFTitle,title); 
	if (!GetFileName3(hWndDlg,USRFileName,IDS_FILTERUSR,IDS_FILEUSR)) 
		return FALSE;  
	
	Fid=GSSiOpenFile (USRFileName,NULL,OF_READ); 
	if (Fid == HFILE_ERROR)
		return FALSE;
	SetDlgItemText (hWndDlg,StatusControl,"Loading waypoints");   
	
	BigRead (Fid,(HPSTR)&MajorVersion,2);
	BigRead (Fid,(HPSTR)&MinorVersion,2);
	BigRead (Fid,(HPSTR)&NumWaypoints,2);
	for (i=0;i<NumWaypoints;i++)
	{ 
		char	LatC[32],LonC[32], Name[32],savec;
		short	Deg, Min,ndp=1, index;
		double	Sec;    
		
		if (!ContinueProcessing)
			break;	
//		BigRead (Fid,(HPSTR)&nWaypoint,sizeof(nWaypoint));
		BigRead (Fid,(HPSTR)&ObjectNum,2);
		BigRead (Fid,(HPSTR)&Position,sizeof(Position)); 
		LatLon = Merc_2_Degree(Position.Latitude,Position.Longitude);

		BigRead (Fid,(HPSTR)&Altitude,sizeof(Altitude));
		BigRead (Fid,(HPSTR)&ltext,sizeof(ltext)); 
		if (ltext)
		{
			BigRead (Fid,Name,(short)ltext);
			Name[ltext]=0;
		}
		BigRead (Fid,(HPSTR)&ltext,sizeof(ltext)); 
		if (ltext)
			BigRead (Fid,Text,(short)ltext);
		BigRead (Fid,(HPSTR)&Time,sizeof(Time));
		BigRead (Fid,(HPSTR)&Time2,sizeof(Time2));
		BigRead (Fid,(HPSTR)&SymbolID,sizeof(SymbolID));
		BigRead (Fid,(HPSTR)&WaypointType,sizeof(WaypointType));
		if (MajorVersion > 2)
			BigRead (Fid,(HPSTR)&Depth,sizeof(Depth));
		else
			Depth = 0;

//		sprintf (Name,"WP%4.4i      ",(short)(WayPoint.Number+1));
//		LatLon = Merc_2_Degree (WayPoint.Lat,WayPoint.Lon);	
		GetDMS (LatLon.Lat,&Deg,&Min,&Sec); 
		sprintf (LatC,"N%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
		GetDMS (LatLon.Lon,&Deg,&Min,&Sec); 
		sprintf (LonC,"W%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
        sprintf (CTime,"$CAL(%ld,3)",Time+SecondsFrom1970to1992); 
        ExpandText (CTime);
		sprintf (str,"%.12s\t%.8s\t%7.1f\t%.30s\t%s\t%s\t0",Name,IconName,Depth,CTime,
													 LatC,LonC);  
	 	ConvertGPSLatLon (str);
		savec = str[12];   
		str[12]=0;
 		index = SendDlgItemMessage (hWndDlg,Control,LB_FINDSTRING,(WPARAM)-1,(LPARAM) str); 
 		str[12] = savec;
 		if (index != LB_ERR)
 		{
 			SendDlgItemMessage (hWndDlg,Control,LB_DELETESTRING,(WPARAM)index,(LPARAM)0); 
 			SendDlgItemMessage (hWndDlg,Control,LB_INSERTSTRING,(WPARAM)index,(LPARAM) str); 
 		}
 		else
 			index = SendDlgItemMessage (hWndDlg,Control,LB_ADDSTRING,(WPARAM)0,(LPARAM) str); 
 		SendDlgItemMessage (hWndDlg,Control,LB_SETTOPINDEX,(WPARAM)index,(LPARAM)0); 
 		sprintf (str,"%ld points loaded",nLoaded++);
 		SetDlgItemText (hWndDlg,StatusControl,str);
 	}
 	 
 	i=BigRead (Fid,(HPSTR)&NumRoutes,sizeof(NumRoutes));
 	i=BigRead (Fid,(HPSTR)&NumIcons,sizeof(NumIcons));
 	i=BigRead (Fid,(HPSTR)&NumTrails,sizeof(NumTrails));
	GSSiClose (Fid);
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete");
	ContinueProcessing = TRUE;
	return TRUE;
}  

short GPSExportUSRWP (HWND hWndDlg,UINT Control,UINT StatusControl)
{
	LSI100WAYPOINT	Wpt;   
	BYTE	Cmd;
	Merc	T;
	char	str[256]="";	 
	HCURSOR	hcurSave;    
	HANDLE	hItems=0;
	LPINT	lpItems,lpWPNums; 
	LPSTR	pLat,pLon, pTAB, pOpts, pSym, pDesc, pRef;
	double	lat,lon;
	int		n, nItems;    
	BOOL	Err;
	long	nRecs, nLoaded=0; 
	LPBYTE	Wpt_ident, Wpt_cmnt=0, Wpt_smbl, Wpt_attr=0; 
	long	Ref; 
	short	WPInRouteNumber=1;
	short MajorVersion=2;
	short MinorVersion=0;
	short NumWaypoints;
	short NumRoutes=0;
	short NumIcons=0;
	short NumTrails=0;
	short ObjectNum;
    HFILE	Fid;
	tWaypoint Waypoint;
	tText ObjectName;
	tMCoord Position;
	long Altitude=0;  
	long	ltext;
	char	Text[256];  
	long	Time; 
	short	Time2=10000;
	short	SymbolID=0;
	short	WaypointType=0;
//    char	temp[1024];  
	LSTRUC	LatLon;  
	short	i; 
	char	Ext[6]=".usr";
	
   	if (!GetSaveName2 (hWndDlg,str,IDS_FILTERUSR,Ext,IDS_FILEUSR)) 
   		goto Exit;
	nItems=GetLBSelectedItems (hWndDlg,Control,&hItems);  
	if (!nItems)
		return FALSE;
	lpItems=  (LPINT) GlobalLock(hItems);
	Fid = GSSiOpenFile (str,NULL,OF_CREATE);
	if (Fid == HFILE_ERROR)
		goto Exit;    
	Wpt_ident = (LPBYTE)&Wpt.Name;
	Wpt_cmnt = 0;
	Wpt_smbl = &Wpt.Symbol; 
	
	n = NumWaypoints = nRecs = nItems;
	nLoaded = 0;
	_fmemset (&Wpt,0,sizeof(Wpt));    
	BigWrite (Fid,(HPSTR)&MajorVersion,2,-1);
	BigWrite (Fid,(HPSTR)&MinorVersion,2,-1);
	BigWrite (Fid,(HPSTR)&NumWaypoints,2,-1);
	for (i=0;i<NumWaypoints;i++)
	{ 
		char	LatC[32],LonC[32], Name[32],savec;
		short	Deg, Min,ndp=1, index;
		double	Sec;    
		
		if (!ContinueProcessing)
			break;
		ObjectNum = i;	
		BigWrite (Fid,(HPSTR)&ObjectNum,2,-1);
		SendDlgItemMessage (hWndDlg,Control,LB_GETTEXT,*lpItems++,(LPARAM)str);
			Ref=1;
		if ((pRef = _fstrrchr (str,'\t'))) 
		{  
			if (IsInteger(pRef+1))
			{
				*pRef++ = 0;
				Ref = atol (pRef); 
			}
		} 
		Wpt.Status = TRUE; 
		pTAB = _fstrchr (str,'\t');
		*pTAB++ = 0;   
		_fstrncpy (Wpt_ident,str,12);   
		Time = Wpt.Date = time(NULL)-SecondsFrom1970to1992;
		pSym = pTAB;
		pTAB = _fstrchr (pSym,'\t');
		*pTAB++ = 0;
		pDesc = pTAB;
		pTAB = _fstrchr (pDesc,'\t');
		*pTAB++ = 0; 
//		if (Ref != -1) //indicate track log
		{ 
			pOpts = pTAB;
			pTAB = _fstrchr (pOpts,'\t');
			*pTAB++ = 0; 
		} 
		pLat = pTAB;
		pTAB = _fstrchr (pLat,'\t');
		*pTAB++ = 0;
		pLon = pTAB;
		lat = DecDegFromDMS (pLat,&Err); 
		lon = DecDegFromDMS (pLon,&Err);
		T = Degree_2_Merc(lat,lon); 
		Wpt.Lat = T.Lat;
		Wpt.Lon = T.Lon;  

		if (Wpt_smbl)
        	*Wpt_smbl = 0;
		BigWrite (Fid,(HPSTR)&T,sizeof(Position),-1); 

		BigWrite (Fid,(HPSTR)&Altitude,sizeof(Altitude),-1); 
		ltext = _fstrlen (Wpt_ident);
		BigWrite (Fid,(HPSTR)&ltext,sizeof(ltext),-1); 
		if (ltext)
		{
			BigWrite (Fid,(HPSTR)Wpt_ident,(short)ltext,-1);
			Name[ltext]=0;
		}
		BigWrite (Fid,(HPSTR)&ltext,sizeof(ltext),-1); 
		if (ltext)
			BigWrite (Fid,(HPSTR)&Text,(short)ltext,-1);
		BigWrite (Fid,(HPSTR)&Time,sizeof(Time),-1);
		BigWrite (Fid,(HPSTR)&Time2,sizeof(Time2),-1);
		BigWrite (Fid,(HPSTR)&SymbolID,sizeof(SymbolID),-1);
		BigWrite (Fid,(HPSTR)&WaypointType,sizeof(WaypointType),-1);

        PctBox (GetDlgItem(hWndDlg,StatusControl),nRecs,++nLoaded,-1);
	}
 	BigWrite (Fid,(HPSTR)&NumRoutes,sizeof(NumRoutes),-1);
 	BigWrite (Fid,(HPSTR)&NumIcons,sizeof(NumIcons),-1);
 	BigWrite (Fid,(HPSTR)&NumTrails,sizeof(NumTrails),-1);
	GSSiClose (Fid);
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete");   
Exit:
	GSSiGlobUlFree (&hItems);
	return nItems;
} 

BOOL GPSImportGPXWP (HWND hWndDlg,UINT Control,UINT StatusControl)
{ 
	WORD	WPNum=1;
	char	str[256], CTime[32],IconName[12];
	long	nLoaded=0, nSearched=0,nRecs,n;
	char	title[]="Load GPX Waypoint File";
	long i;
	short MajorVersion;
	short MinorVersion;
	short NumWaypoints;
	short NumRoutes;
	short NumIcons;
	short NumTrails;
	short ObjectNum;
	float Depth;
    HFILE	Fid;
	tWaypoint Waypoint;
	tText ObjectName;
	tMCoord Position;
	long Altitude;  
	long	ltext;
	char	Text[256];  
	long	Time; 
	short	Time2;
	short	SymbolID;
	short	WaypointType;
//    char	temp[1024];  
	LSTRUC	LatLon; 

#pragma pack(2)
	struct{
	 unsigned short ObjectID;          // Numeric identifier of the Waypoint..
	 long           Latitude;           // Latitude component of a position..
	 long           Longitude;          // Longitude component of a position..
	 long           Altitude;          // Waypoint's altitude..
	 char*          Name;              // Waypoint's name..
	 char*          Comment;           // Comments about the Waypoint..
	 unsigned long  Time;              // Creation date of the Waypoint..
	 unsigned long  SymbolID;          // Graphic symbol associated with the Waypoint 
	 unsigned short WaypointType;      // Type of Waypoint, among USER CREATED,                         
									   //  TEMPORARY and POINT OF INTEREST (see 
									   //  description of these types below)..
	 float          Depth;             // Waypoint's depth (new in Version 3.0)
	} nWaypoint ; 
#pragma pack()


	WPNum = 0;  
	strcpy (OFTitle,title); 
	if (!GetFileName3(hWndDlg,USRFileName,IDS_FILTERUSR,IDS_FILEUSR)) 
		return FALSE;  
	
	Fid=GSSiOpenFile (USRFileName,NULL,OF_READ); 
	if (Fid == HFILE_ERROR)
		return FALSE;
	SetDlgItemText (hWndDlg,StatusControl,"Loading waypoints");   
	
	BigRead (Fid,(HPSTR)&MajorVersion,2);
	BigRead (Fid,(HPSTR)&MinorVersion,2);
	BigRead (Fid,(HPSTR)&NumWaypoints,2);
	for (i=0;i<NumWaypoints;i++)
	{ 
		char	LatC[32],LonC[32], Name[32],savec;
		short	Deg, Min,ndp=1, index;
		double	Sec;    
		
		if (!ContinueProcessing)
			break;	
//		BigRead (Fid,(HPSTR)&nWaypoint,sizeof(nWaypoint));
		BigRead (Fid,(HPSTR)&ObjectNum,2);
		BigRead (Fid,(HPSTR)&Position,sizeof(Position)); 
		LatLon = Merc_2_Degree(Position.Latitude,Position.Longitude);

		BigRead (Fid,(HPSTR)&Altitude,sizeof(Altitude));
		BigRead (Fid,(HPSTR)&ltext,sizeof(ltext)); 
		if (ltext)
		{
			BigRead (Fid,Name,(short)ltext);
			Name[ltext]=0;
		}
		BigRead (Fid,(HPSTR)&ltext,sizeof(ltext)); 
		if (ltext)
			BigRead (Fid,Text,(short)ltext);
		BigRead (Fid,(HPSTR)&Time,sizeof(Time));
		BigRead (Fid,(HPSTR)&Time2,sizeof(Time2));
		BigRead (Fid,(HPSTR)&SymbolID,sizeof(SymbolID));
		BigRead (Fid,(HPSTR)&WaypointType,sizeof(WaypointType));
		if (MajorVersion > 2)
			BigRead (Fid,(HPSTR)&Depth,sizeof(Depth));
		else
			Depth = 0;

//		sprintf (Name,"WP%4.4i      ",(short)(WayPoint.Number+1));
//		LatLon = Merc_2_Degree (WayPoint.Lat,WayPoint.Lon);	
		GetDMS (LatLon.Lat,&Deg,&Min,&Sec); 
		sprintf (LatC,"N%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
		GetDMS (LatLon.Lon,&Deg,&Min,&Sec); 
		sprintf (LonC,"W%2.2iD %2.2iM %04.1fS",abs(Deg),Min,Sec);
        sprintf (CTime,"$CAL(%ld,3)",Time+SecondsFrom1970to1992); 
        ExpandText (CTime);
		sprintf (str,"%.12s\t%.8s\t%7.1f\t%.30s\t%s\t%s\t0",Name,IconName,Depth,CTime,
													 LatC,LonC);  
	 	ConvertGPSLatLon (str);
		savec = str[12];   
		str[12]=0;
 		index = SendDlgItemMessage (hWndDlg,Control,LB_FINDSTRING,(WPARAM)-1,(LPARAM) str); 
 		str[12] = savec;
 		if (index != LB_ERR)
 		{
 			SendDlgItemMessage (hWndDlg,Control,LB_DELETESTRING,(WPARAM)index,(LPARAM)0); 
 			SendDlgItemMessage (hWndDlg,Control,LB_INSERTSTRING,(WPARAM)index,(LPARAM) str); 
 		}
 		else
 			index = SendDlgItemMessage (hWndDlg,Control,LB_ADDSTRING,(WPARAM)0,(LPARAM) str); 
 		SendDlgItemMessage (hWndDlg,Control,LB_SETTOPINDEX,(WPARAM)index,(LPARAM)0); 
 		sprintf (str,"%ld points loaded",nLoaded++);
 		SetDlgItemText (hWndDlg,StatusControl,str);
 	}
 	 
 	i=BigRead (Fid,(HPSTR)&NumRoutes,sizeof(NumRoutes));
 	i=BigRead (Fid,(HPSTR)&NumIcons,sizeof(NumIcons));
 	i=BigRead (Fid,(HPSTR)&NumTrails,sizeof(NumTrails));
	GSSiClose (Fid);
	SetDlgItemText (hWndDlg,StatusControl,"Transfer complete");
	ContinueProcessing = TRUE;
	return TRUE;
}  






