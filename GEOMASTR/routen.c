#include <windows.h>   
#include <commdlg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
#include <mmsystem.h>
#include <vfw.h>
#include <avifile.h>		
#include "winexec.h" 
#include <shellapi.h>  
#include <float.h>
#include "graphint.h"
#include "dyndlg.h"
#include "resource.h"  
#include "address.h"  
#include "dibapi.h"   
#include "STD.H"  
#include "translat.h" 
#include "dict.h"   
#include "p_tol.h"
#include <malloc.h>

#define	TRACK_LEFT	1
#define TRACK_RIGHT 2
typedef struct	{
				POINT	Point1;
				POINT	Point2;
				long	Refno;
				short	Mnx,Mxx;
				} LINEINT;  
typedef LINEINT	huge	*LPLINEINT; 
typedef struct	{
				double	Size;
				long	LoopID;
				}	LOOPKEY;    
LOOPKEY	LoopKey;
typedef struct {
				MNMXCORD MinMax; 
				long	Offset;
				DPOINT	Point1;
				short	NumLinks,
						NumSides; 
				BOOL	Reverse;
				}	LOOPDATA;
LOOPDATA	LoopData;
 
struct	{
			LPOINT	NodeID;
			double	AZ;
		}	NodesKey;
typedef struct	{
					long	LinkID;
					short	WhichEnd; 
					DPOINT	Point;
				}	NODESDATA;
typedef NODESDATA	FAR	*LPNODESDATA;
NODESDATA	NodesData;

typedef struct {
		long	LinkRef;
		short	Dir;
	   }	LOOPREC;   
typedef LOOPREC	FAR	*LPLOOPREC;			

typedef struct {
		long	Ref;
		POINT	BPFile,EPFile; 
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
					DPOINT	FromPoint, ToPoint;
				}	LINESDATA;
typedef LINESDATA	FAR	*LPLINESDATA;
LINESDATA	LinesData;

typedef struct	{
			LPOINT	FromNode,ToNode;
		}	LINESKEY;
typedef LINESKEY	FAR	*LPLINESKEY;


HANDLE	hNodes=0, hBTLinks=0, hBTInclusion=0, hBTExclusion=0, hDBPolyID, hLines=0;  
HFILE	FidLoop,FidLinks;
double	AddNodeFactor;
DPOINT	AddNodeBias;
BOOL	LimitRouteToScreen=TRUE;
HANDLE	hBTAt, hBTNext,hDBRefConnect=0,hBTDups=0;  
BOOL	TraceNet=FALSE; 
double	UniqueCost, UniqueCostInc=0.0000000001;  
long	WhenAssigned;              
BOOL	RouteByNet=FALSE, DebugRoute=FALSE;
ATDATA		AtData, LastAtData;
NEXTDATA	NextData;       
NEXTKEY		NextKey;  
int		RouteType=TWOPOINTROUTE,WalkOutRouteOpt; 
HANDLE	hBTChained=0, hIntRef=0;   
HFILE	FidChain=HFILE_ERROR;
long	IntersectionID; 
HFILE	FidDupList;

extern  BOOL    DisableHalt, ContinueProcessing, Processing;
extern	HCURSOR	hCursor;
extern	MNMXCORD	EditBounds; 
extern	char	EditName[128];
extern	POINT	SnapPointFile;
extern	short	SnapEnd;
extern	double	MaxSpeed, MinSpeed;
extern	long	FastPickFileNum; 
extern	double FileDistToBaseDist;
extern	long	HighlightSequence;
extern	HANDLE	hDBStreetNames, hBTNetLinks, hBTNetRefs, hBTNetMarkers1,hBTNetMarkers2,hBTNetIntRef, hBTNetIntMP;  
extern	HANDLE	hBTNetVideo1, hBTNetVideo2;
extern  HANDLE  hSavePoly,hSavePolyParts;
extern	short	nSavePoly;
extern	long	StreetNum,DoNotPickThisRefno,TotHLTPoints; 
extern	MNMXCORD HLTBounds;                
extern	char	CurrentStreetName[34]; 
extern	BOOL	IgnoreBounds,AddLBUTTON;  
extern	int		CommandViewport;
extern	int		NetworkID;
extern	BOOL	DisplayMarkers,UseUserPickAp, Pick,PickingByRefno;
extern	int		MaxPick;  
extern	HANDLE	hHighlight,hHighlight2; 
extern	char    PickName[128], PltName[128];
extern	int		NumPicked;
extern	PICKDATA	PickList[MAXPICKITEMS];
extern	long	PickedStreets[MAXPICKITEMS][4];
extern	LPVISLIST	CurVis;
extern	LPVIEWPORT	CurView, pViewports[MAX_VIEWPORTS], pViewportsD[MAX_VIEWPORTS];
extern	HANDLE		hViewports[MAX_VIEWPORTS];
extern	int			NumViewports, NumViewportsToDisplay, DisplayViewID;
extern	LPTHEME		CurTheme;
extern	LPVOID		TranFileToBase, TranBaseToFile, TranBMToBase, TranBaseToBM;
extern	HANDLE		hTranFileToBase, hTranBaseToFile, hTranBMToBase, hTranBaseToBM;
extern	int			OrthoRedAdjust,OrthoGreenAdjust,OrthoBlueAdjust,OrthoIntensity;
extern	HANDLE		hInst;   
extern	HWND		hWndMain;
extern	double		WidthFactor;
extern	RGBQUAD 	rgb;
extern	HPEN		hSavePen, hNullPen, hRedPen, pens[MAXPENS], HighlightPen;
extern	HPEN		CurrentPen;
extern	HBRUSH		brushes[MAXPENS], hRedBrush, hGreenBrush, hBlueBrush, HighlightBrush;
extern	int			maxbrush; 
extern	BOOL		DoPaint, NetUpdate;
extern	double		OrthoAdjustX, OrthoAdjustY;
extern	char        gszFilter[256];
extern	int	 NetDirection;                           
extern	int	 FileNum;
extern	USHORT	FileInIndex;  
extern	LONG	CurrentSeg, ItemSeg, CurrentRefno;
extern	int		CurrentDesc, CurrentiPen, CurrentType;  
extern	WORD	CurrentItem;
extern	double	RouteTOL;
extern	double	NetTOL;  
extern	double	SystemPickAp;  
extern	HCURSOR	hDigCursor; 
extern	BOOL	CursorIsLocked;
extern	DPOINT	CurrentPoint;
extern	HANDLE	hDBStreetSegments;

void DecompAddLine (LPDPOINT FromPoint, LPDPOINT ToPoint, long Poly);
BOOL AddNodeInit (LPMNMXCORD Rect);
BOOL CreateRefConnectionFile (LPSTR Name);
BOOL OpenRefConnectionFile (LPSTR Name);
void UpdateConnectFile (long Refno,LPGWDHEADER lpGWDHead,DPOINT BP, DPOINT EP, POINT BPFile, POINT EPFile);
short TraceLink (long Refno,long LinkRef,long StartRef,LPGWDHEADER lpGWDHead,short WhichEndNext,long Offset, HPDPOINT *pPoints,LPLPOINT pToNode,LPDOUBLE pNodeAZ);
LPOINT AddNode (short InOut,short OpEnd,DPOINT ConnectPointBase,long LinkRef,LPREFCONNECT pRC,LPDOUBLE pNodeAZ);
void CloseNodes (void);
BOOL GetNextLink (LPLPOINT AtNode,LPLONG AtLink,LPSHORT AtEnd,LPDOUBLE TotAZ,short Track,LPDOUBLE pAtAZ);
HANDLE LoadLoopSides (LPSHORT pNumSides,short NumLinks,long Offset,BOOL Reverse);
BOOL PointInLoop (DPOINT Point,short NumLoopPoints,HANDLE hLoopPoints);
void CalculateLoopSize (HFILE FidLoop,long Offset,short NumLinks,BOOL Reverse,
            			LPMNMXCORD pMinMax, LPDOUBLE pSize,LPSHORT pNumSides, LPDPOINT pPoint1);
double GetLinkDeltaAZ (short NumLinkPoints,HPDPOINT pLinkPoints);
double DeltaAZ (double AZ1, double AZ2);
BOOL SplitPoly (short Item, LPLONG pNewRef1, LPLONG pNewRef2);
long CheckForDupLines (long refno,POINT BP,POINT EP);
void CloseRefConnectFile (void);
void DumpLinkData (LPSTR InName); 
BOOL OpenPolyIDFile (LPSTR Name);
void ClosePolyIDFile (void);
BOOL LineInt (LPLINEINT lInt1, LPLINEINT lInt2);
short LoadTranFilePoints (LPSTR Name,LPDOUBLE XFROM, LPDOUBLE YFROM, LPDOUBLE XTO, LPDOUBLE YTO);
BOOL DecompInit (LPMNMXCORD Rect);
void DecompClose (void);

BOOL HighlightSequentialItems (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{HDC hDC;
 char key;     
 int	st;
 long	TLID;
 HIGHLIGHTDATA	HighlightData;
 POINT	MousePoint;
 static	char RouteFile1[128]="", RouteFile2[128]="",ChainedFile[128]="";  
 static	long		FromRef, ToRef, FromPath; 
 static	double		FromMP1, FromMP2; 
 static short		FromDir1, FromDir2;  
 static	DPOINT	FromPoint1, FromPoint2, FromPoint;   
 static	BOOL	NetOpened=FALSE, StreetOpened=FALSE;
 long	Sequence;    

 switch (Message)
   {
   	case GF_INIT:
		{	BTVARDESC	BTVar[2]; 
			HIGHLIGHTDATA	HighlightData1, HighlightData2; 
			long	Refno;
		    
		    if (WalkOutRouteOpt == GF_BUILD_NETWORK)
		    {
		    	if (!NetUpdate)
			    {   
			    	MessageBox (GetFocus(),"You must be in network update mode to run this command",NULL,MB_ICONEXCLAMATION);
					PostMessage(hWnd, GF_CLOSE,0, 0L); 
			     	break;
			    }  
				if (MessageBox( GetFocus(),"This command will replace your current street network - do you wish to continue?",
						"Verify Rebuild", MB_OKCANCEL) == IDCANCEL) 
				{
					PostMessage(hWnd, GF_CLOSE,0, 0L); 
					break;
				}
				DeleteStreetNetwork();
			}
		    if (RouteType == TWOPOINTROUTENET || WalkOutRouteOpt == GF_WALKOUT_ROUTE)
		    	RouteByNet = TRUE;         
		    else
		    	RouteByNet = FALSE;
		    NetOpened = StreetOpened = FALSE;
			if (RouteByNet || TraceNet)
				OpenNetLinkAndRef (1,FALSE,&NetOpened);
			if (RouteByNet) 
			{
		    	StartFastPick (65);
				StreetOpened = OpenStreetSegmentTable (FALSE); 
			}
		    if (RouteType == TWOPOINTROUTE)
		    {
			    st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_LAST,BT_ANY,(LPSTR)&FromRef); 
			    if (st)
			    {
			 		MessageBox(GetFocus(), "No start segment", NULL,MB_ICONEXCLAMATION|MB_OK);
			        PostMessage(hWnd, GF_CLOSE,0, 0L);
			    	break;
			    }  
			    BT_FIND (hHighlight,(LPSTR)&FromRef,BT_FIRST,BT_EQ,(LPSTR)&HighlightData1); 
			    PickList[0] = HighlightData1.PD; 
			    if (RouteByNet)
			    {   
			    	PickList[0].PCT = 0;
   		    		GetNetLocFromPick (0,&FromPath, &FromMP1, &FromDir1);
   		    		FromDir1 *= -1;
			    	PickList[0].PCT = 1;
   		    		GetNetLocFromPick (0,&FromPath, &FromMP2, &FromDir2);
   		    	}
			    FromPoint1 = HighlightData1.PD.BeginPoint;
			    FromPoint2 = HighlightData1.PD.EndPoint;
			    st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_PRIOR,BT_ANY,(LPSTR)&Refno); 
			    if (!st)
			    {
			    	BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData2);
			    	if (ldistp(HighlightData1.PD.BeginPoint,HighlightData2.PD.BeginPoint) < RouteTOL)
			    		FromPoint1 = FromPoint2;
			    	else if (ldistp(HighlightData1.PD.BeginPoint,HighlightData2.PD.EndPoint) < RouteTOL)
			    		FromPoint1 = FromPoint2;
			    	else if (ldistp(HighlightData1.PD.EndPoint,HighlightData2.PD.BeginPoint) < RouteTOL)
			    		FromPoint2 = FromPoint1;
			    	else if (ldistp(HighlightData1.PD.EndPoint,HighlightData2.PD.EndPoint) < RouteTOL)
			    		FromPoint2 = FromPoint1;
			    } 
			    FromPoint.x = (FromPoint1.x + FromPoint2.x) / 2;
			    FromPoint.y = (FromPoint1.y + FromPoint2.y) / 2;    
				GetTempFileName (NULL,"hl1",NULL,(LPSTR)&RouteFile1);
			} 
			else if (RouteType == TWOPOINTROUTENET)
			{
				if (BT_NUM_IN_INDEX (hHighlight)<2)  
				{
			 		MessageBox(GetFocus(), "Insufficient segments highlighted", NULL,MB_ICONEXCLAMATION|MB_OK);
			        PostMessage(hWnd, GF_CLOSE,0, 0L);
			    	break;
			    }
				GetTempFileName (NULL,"hl1",NULL,(LPSTR)&RouteFile1);
			}
			else
			{
				FromPoint.x = 0;
				FromPoint.y = 0;  
				FromPoint1 = FromPoint;
				FromPoint2 = FromPoint;  
				FromRef = ZERO$;
				switch (WalkOutRouteOpt)
				{   
					case GF_BUILD_NETWORK:
					{
					OFSTRUCT	OFStruct;
					
						GetTempFileName (NULL,"hl3",NULL,(LPSTR)&ChainedFile);
						BTVar[0].BT_VARTYP=BT_INTEGER;
						BTVar[0].BT_VARLEN=4;
						BTVar[0].BT_VAROFF=0;
						BT_CREATE (ChainedFile, 4, FALSE, 1, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
						hBTChained= BT_OPEN (ChainedFile, 0, BT_WRITE, 0);
						FidChain = GSSiOpenFile ("NetChain.bin",&OFStruct,OF_CREATE);
						BTVar[0].BT_VARTYP=BT_INTEGER;
						BTVar[0].BT_VARLEN=4;
						BTVar[0].BT_VAROFF=0;
						BTVar[1].BT_VARTYP=BT_INTEGER;
						BTVar[1].BT_VARLEN=4;
						BTVar[1].BT_VAROFF=4;
						BT_CREATE ("intref.btr", sizeof(NETINTREFDATA), FALSE, 2, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
						hIntRef= BT_OPEN ("intref.btr", 0, BT_WRITE, 0);   
						IntersectionID=0; 
						GetTempFileName (NULL,"hl1",NULL,(LPSTR)&RouteFile1);
					}
					break;
					case GF_CONNECTIVITY_TEST: 
				    	StartFastPick (65);
						_fstrcpy (RouteFile1,"contest.wor");
					break;
					case GF_WALKOUT_ROUTE:
						GetTempFileName (NULL,"hl1",NULL,(LPSTR)&RouteFile1);
					break;   
				} 
			}
			GetTempFileName (NULL,"hl2",NULL,(LPSTR)&RouteFile2);
			BTVar[0].BT_VARTYP=BT_INTEGER;
			BTVar[0].BT_VARLEN=4;
			BTVar[0].BT_VAROFF=0;
			BT_CREATE (RouteFile1, sizeof(ATDATA), FALSE, 1, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
			hBTAt= BT_OPEN (RouteFile1, 0, BT_WRITE, 0);
			BTVar[0].BT_VARTYP=BT_REAL;
			BTVar[0].BT_VARLEN=8;
			BTVar[0].BT_VAROFF=0;
			BTVar[1].BT_VARTYP=BT_INTEGER;
			BTVar[1].BT_VARLEN=4;
			BTVar[1].BT_VAROFF=8;
			BT_CREATE (RouteFile2, sizeof(NEXTDATA), FALSE, 2, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
			hBTNext= BT_OPEN (RouteFile2, 0, BT_WRITE, 0); 
		}
       	AddLBUTTON = TRUE;
   		
   		break;  
   		
   	case GF_CLOSE:  
   		EndFastPick();
   		CloseNetLinkAndRef (NetOpened);
   		if (StreetOpened)
   			CloseStreetSegmentTable();
   		StreetOpened=FALSE;
   		if (hBTChained)
   		{
   			BT_CLOSE (hBTChained);
   			hBTChained = 0;         
   			BT_CLOSE (hIntRef);
   			hIntRef = 0;
   			GSSiRemove (ChainedFile);  
   			_lclose (FidChain); 
	        FidChain = HFILE_ERROR;
   		}
   		if (hBTAt)
   		{   
   			if (DebugRoute)
   				SaveRouteNextFile (hBTNext,"routnext.gmd");
			SaveRouteAtFile (hBTAt,"routat.gmd");
   			BT_CLOSE(hBTAt);
   			BT_CLOSE(hBTNext);
   			hBTAt=hBTNext=0;
			if (WalkOutRouteOpt==GF_BUILD_NETWORK || RouteType == TWOPOINTROUTE || RouteType == TWOPOINTROUTENET)
				GSSiRemove(RouteFile1);
   			GSSiRemove(RouteFile2); 
			if (!DisplayMarkers && RouteType == WALKOUTROUTE)
		   		RedisplayViewport(FALSE,FALSE);
   		} 
   		else
   			Sound(BAD_SOUND);
        RemoveGraphicsFunction (hWnd);
   		break;

    case WM_LBUTTONUP: 
    {   
    	VISLIST	SaveVis; 
    	DPOINT	ToPoint1, ToPoint2, ToPoint, TPoint; 
    	double	MaxCost = MAX_COST;  
   // 	double	Cost;
    	long	LinkRef = LONG_MIN, LinkToRef, LastRef, NextRef;
    	NEXTKEY		SaveNextKey;
    	NEXTDATA	SaveNextData;
		HCURSOR	hcurSave; 
		long	ToPath;
		double	ToMP1, ToMP2;
		short	ToDir1, ToDir2;
		int		ii, TolFac=1,SaveMaxPick; 
		BOOL	Opened1, Opened2; 
		short	pos=BT_FIRST;
	
   		if (!hBTAt) 
   		{
	        RemoveGraphicsFunction (hWnd);
   			break;
    	}
//AddSpeed (); 
		if (RouteByNet)
		{
			OpenNetLinkAndRef (NetworkID,TRUE,&Opened1); 
			OpenNetIntersect (NetworkID,FALSE,&Opened2); 
		} 
    	
    	UniqueCost = 0; 
    	WhenAssigned=0;
    	MousePoint.x = LOWORD(lParam);
	    MousePoint.y = HIWORD(lParam);
	    if (!CurView->hTranWinToBase) break; 
	    hCursor = LoadCursor(NULL, IDC_WAIT);
		hcurSave = GSSiSetCursor(hCursor); 
	    ToPoint=WinPtToBasePt(MousePoint);
		if (RouteType == WALKOUTROUTE)  
			FromPoint = ToPoint;
		SelectVisList (TRUE); 
		SaveVis = *CurVis;
		CurVis->WantType[0]=FALSE;
		CurVis->WantType[2]=FALSE;
		SaveMaxPick = MaxPick;
        NumPicked = 0;
		UseUserPickAp =FALSE;
		SystemPickAp = 0;	
		MaxPick=8;  

		if (WalkOutRouteOpt == GF_BUILD_NETWORK || RouteType == TWOPOINTROUTE)
		{
		    PickItems (hWnd,ToPoint); 
		    if (!NumPicked)
		    	st = 1;
		    else
		    {
		    	NumPicked--;
		    	st = 0;     
		    }
		}
		else if (RouteType == TWOPOINTROUTENET)
		{ 
		    st = BT_FIND (hHighlight2,(LPSTR)&Sequence,pos,BT_ANY,(LPSTR)&FromRef); 
		    pos = BT_NEXT;
		    BT_FIND (hHighlight,(LPSTR)&FromRef,BT_FIRST,BT_EQ,(LPSTR)&HighlightData); 
		    PickList[0] = HighlightData.PD; 
		    if (RouteByNet)
		    {   
		    	PickList[0].PCT = 0;
	    		GetNetLocFromPick (0,&FromPath, &FromMP1, &FromDir1);
	    		FromDir1 *= -1;
		    	PickList[0].PCT = 1;
	    		GetNetLocFromPick (0,&FromPath, &FromMP2, &FromDir2);
	    	}
		    FromPoint1 = HighlightData.PD.BeginPoint;
		    FromPoint2 = HighlightData.PD.EndPoint;
		    FromPoint.x = (FromPoint1.x + FromPoint2.x) / 2;
		    FromPoint.y = (FromPoint1.y + FromPoint2.y) / 2;    
		    st = BT_FIND (hHighlight2,(LPSTR)&Sequence,pos,BT_ANY,(LPSTR)&PickList[NumPicked].Refno); 
	    	st = BT_FIND (hHighlight,(LPSTR)&PickList[NumPicked].Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData); 
		    PickList[0] = HighlightData.PD; 
		}
		else  
		{
		    st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_FIRST,BT_ANY,(LPSTR)&PickList[NumPicked].Refno); 
	    	if (!st)
	    		BT_FIND (hHighlight,(LPSTR)&PickList[NumPicked].Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData); 
	    	PickList[NumPicked] = HighlightData.PD;
	    }
	    RouteTOL = TolFac * FileDistToBaseDist;
		SetDisplayMode (CurView->hDC, GF_TEXTMODE);
		CurView->hRgn = CreateVPRgn ();
	  	SelectClipRgn (CurView->hDC,CurView->hRgn);
	  	DeleteObject(CurView->hRgn);  
	  	pos = BT_FIRST;
	  	while (!st)
	  	{
	    	if (RouteByNet) 
	    	{   
	    		PickList[NumPicked].PCT = 0;
	    		GetNetLocFromPick (NumPicked,&ToPath, &ToMP1, &ToDir1);
	    		ToDir1 *= -1; 
	    		PickList[NumPicked].PCT = 1;
	    		GetNetLocFromPick (NumPicked,&ToPath, &ToMP2, &ToDir2); 
	    	}
			AtData.Sequence = 0;
	    	AtData.LastRef = NULLREF;  
	    	ToRef = PickList[NumPicked].Refno;
	    	AtData.StartRef = ToRef;    
			AtData.Segment = PickList[NumPicked].Segment;
			AtData.Offset = PickList[NumPicked].Offset;
			AtData.FileNum = PickList[NumPicked].FileNum;  
			AtData.SubFile = CurView->SubFile;
	        AtData.FileInIndex = PickList[NumPicked].FileInIndex;  
	        _fmemmove (AtData.StreetNums,PickedStreets[NumPicked],16);
		        
	    	ToPoint1 = PickList[NumPicked].BeginPoint;
	    	ToPoint2 = PickList[NumPicked].EndPoint;
	    	BT_PUT(hBTAt,(LPSTR)&ToRef,(LPSTR)&AtData);
	    	AtData.LastRef = NULLREF;
	    	AtData.NextRef = NULLREF;
	    	AtData.StartRef = FromRef;
			AtData.Segment = PickList[NumPicked].Segment;
			AtData.Offset = PickList[NumPicked].Offset;
			AtData.FileNum = PickList[NumPicked].FileNum;
			AtData.SubFile = PickList[NumPicked].SubFile;  
	        AtData.FileInIndex = PickList[NumPicked].FileInIndex;  
	        _fmemmove (AtData.StreetNums,PickedStreets[NumPicked],16);
	        if (FromRef !=ZERO$)
	        {
		    	BT_PUT(hBTAt,(LPSTR)&FromRef,(LPSTR)&AtData);
		    	AddNextRefs(FromRef,FromPoint1,FromPath,FromMP1,FromDir1,FromRef,0,ToPoint,0);
		    	if (ldistp(FromPoint1,FromPoint2) > RouteTOL) 
		    		AddNextRefs(FromRef,FromPoint2,FromPath,FromMP2,FromDir2,FromRef,0,ToPoint,0);  
		    }
	    	AddNextRefs(ToRef,ToPoint1,ToPath,ToMP1,ToDir1,ToRef,0,FromPoint,0); 
	    	AddNextRefs(ToRef,ToPoint2,ToPath,ToMP2,ToDir2,ToRef,0,FromPoint,0);
			if (WalkOutRouteOpt == GF_BUILD_NETWORK || (RouteType == TWOPOINTROUTE && !RouteByNet) )
				st = 1;
			else
			{   
				NumPicked=0;
		    	st = BT_FIND (hHighlight2,(LPSTR)&Sequence,BT_NEXT,BT_ANY,(LPSTR)&PickList[NumPicked].Refno); 
		    	if (!st)
		    		BT_FIND (hHighlight,(LPSTR)&PickList[NumPicked].Refno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData); 
		    	PickList[NumPicked] = HighlightData.PD;
		    }
	   	}
    	if (BT_FIND (hBTNext,(LPSTR)&NextKey,BT_FIRST,BT_ANY,(LPSTR)&NextData))
    		NextKey.Cost = INFINITE_COST; 
    	while (NextKey.Cost < MaxCost && CheckForContinue ())   
    	{   
    		SaveNextKey = NextKey;
    		SaveNextData = NextData;
    		st = BT_FIND (hBTAt,(LPSTR)&NextKey.Refno,BT_FIRST,BT_EQ,(LPSTR)&AtData);
    		if (!st && RouteType != WALKOUTROUTE && AtData.StartRef != NextData.StartRef)
    		{   double	Cost;
		    		
    			Cost = NextData.AtCost + AtData.Cost; 
    			if (Cost < MaxCost)
    			{
    				MaxCost = Cost;
	    			LinkRef = NextKey.Refno;
	    			LinkToRef = NextData.AtRef;
	    			DisplayMarker(NextData.OpenEnd,6,NULL,0,0,0,TRUE); 
	    		}
    		}
    		else
    		{   
    			if (!st)
    			{
    				if (AtData.Cost <= NextData.NextCost)
    					goto S10;
    				BT_DELETE (hBTAt,(LPSTR)&NextKey.Refno,(LPSTR)&AtData,FALSE);
    			}
    			DisplayMarker(NextData.OpenEnd,1,NULL,0,0,0,TRUE);
    			if (NextData.StartRef == FromRef)
    				TPoint = ToPoint;
    			else
    				TPoint = FromPoint; 
    			AtData.NextRef = NULLREF;
		    	AtData.LastRef = NextData.AtRef;
		    	AtData.StartRef = NextData.StartRef;
		    	AtData.Cost = NextData.NextCost;
		    	AtData.Sequence = NextData.Sequence;
				AtData.Segment = NextData.Segment;
				AtData.Offset = NextData.Offset;
				AtData.FileNum = NextData.FileNum; 
				AtData.SubFile = NextData.SubFile;  
		        AtData.FileInIndex = NextData.FileInIndex;  
		        _fmemmove (AtData.StreetNums,NextData.StreetNums,16);   
		        LastRef = NextData.AtRef; 
		        NextRef = NextKey.Refno;
				    	
    	//		Cost = NextKey.Cost - ldistp(NextData.OpenEnd,TPoint);
		    	BT_PUT(hBTAt,(LPSTR)&NextRef,(LPSTR)&AtData); 
    			if (AddNextRefs(NextKey.Refno,NextData.OpenEnd,NextData.Path,NextData.MP,NextData.Dir,
    							NextData.StartRef,AtData.Cost,TPoint,AtData.Sequence+1))  
    			{
    				AtData.NextRef = LONG_MIN;
			    	BT_PUT(hBTAt,(LPSTR)&NextRef,(LPSTR)&AtData); 
                }
		    	BT_FIND(hBTAt,(LPSTR)&LastRef,BT_FIRST,BT_EQ,(LPSTR)&LastAtData);
		    	LastAtData.NextRef = NextRef;
		    	BT_PUT(hBTAt,(LPSTR)&LastRef,(LPSTR)&LastAtData); 
    		}
    	S10: 
    		if (BT_DELETE (hBTNext,(LPSTR)&SaveNextKey,(LPSTR)&SaveNextData,FALSE))
    			ii=1;  
    		if (DebugRoute)
    		{
    			SaveNextKey.Cost = INFINITE_COST;
    			BT_PUT (hBTNext,(LPSTR)&SaveNextKey,(LPSTR)&SaveNextData);  
    		}
	    	if (BT_FIND (hBTNext,(LPSTR)&NextKey,BT_FIRST,BT_ANY,(LPSTR)&NextData))
	    		NextKey.Cost = INFINITE_COST;
    	}
	    if (LinkRef > LONG_MIN)
	    {   
	    	long	NumFrom,NumTo,Sequence,StartSeq,Ref1,Ref2,Ref; 
	    	double	ActualCost=0; 
	    	char	str[144]; 
	    	BOOL	SavePick;
	    	
		    ii=BT_FIND (hBTAt,(LPSTR)&LinkRef,BT_FIRST,BT_EQ,(LPSTR)&AtData);
		    if (AtData.StartRef == FromRef) 
		    {   
		    	Ref1 = LinkRef;
		    	NumFrom = AtData.Sequence;
		    }
		    else  
		    {
		    	Ref2 = LinkRef;
		    	NumTo = AtData.Sequence;
		    }
		    ii=BT_FIND (hBTAt,(LPSTR)&LinkToRef,BT_FIRST,BT_EQ,(LPSTR)&AtData);
		    if (AtData.StartRef == FromRef) 
		    {
		    	Ref1 = LinkToRef;
		    	NumFrom = AtData.Sequence;
		    }
		    else  
		    {
		    	Ref2 = LinkToRef;
		    	NumTo = AtData.Sequence;
		    } 
		    Pick=PickingByRefno=TRUE;
		    Ref = Ref1;
		    st = BT_FIND (hBTAt,(LPSTR)&Ref,BT_FIRST,BT_EQ,(LPSTR)&AtData);
		    StartSeq = HighlightSequence + NumFrom * 1000; 
		    Sequence = StartSeq;  
		    while (!st && Ref != FromRef)
		    {   
		    	PickList[0].Segment = AtData.Segment;
		    	PickList[0].Offset = AtData.Offset;
		    	PickList[0].View = CurView;
		    	PickList[0].FileNum = AtData.FileNum;
		    	PickList[0].SubFile = AtData.SubFile;
		    	PickList[0].FileInIndex = AtData.FileInIndex;
		    	NumPicked=0; 
		    	IgnoreBounds = TRUE;
		    	ProcessPickedItem(0,FALSE);
		    	IgnoreBounds = FALSE; 
		    	PickList[0].Segment = AtData.Segment;
		    	PickList[0].Offset = AtData.Offset;
		    	ActualCost+=PickList[0].Length;
				AddToHighlightListSeq (Ref,&PickList[0],Sequence,TRUE); 
				SavePick=Pick;
				Pick=FALSE;   
		    	ProcessPickedItem(0,TRUE);
		    	Pick=SavePick;
				Sequence -= 1000; 
				Ref = AtData.LastRef;
		    	st = BT_FIND (hBTAt,(LPSTR)&Ref,BT_FIRST,BT_EQ,(LPSTR)&AtData);
			}
		    Ref = Ref2;
		    st = BT_FIND (hBTAt,(LPSTR)&Ref,BT_FIRST,BT_EQ,(LPSTR)&AtData);
		    Sequence = StartSeq;  
		    Ref1 = FromRef;
		    while (!st && Ref1 != ToRef)
		    {   
				Sequence += 1000; 
		    	PickList[0].Segment = AtData.Segment;
		    	PickList[0].Offset = AtData.Offset;
		    	PickList[0].View = CurView;
		    	PickList[0].FileNum = AtData.FileNum;  
		    	PickList[0].SubFile = AtData.SubFile;
		    	PickList[0].FileInIndex = AtData.FileInIndex;
		    	NumPicked=0;
		    	IgnoreBounds = TRUE;
		    	ProcessPickedItem(0,FALSE); 
		    	PickList[0].Segment = AtData.Segment;
		    	PickList[0].Offset = AtData.Offset;
		    	IgnoreBounds = FALSE;
				AddToHighlightListSeq (Ref,&PickList[0],Sequence,TRUE);
				SavePick=Pick;
				Pick=FALSE;   
		    	ProcessPickedItem(0,TRUE);
		    	Pick=SavePick;
				Ref1 = Ref;  
				Ref = AtData.LastRef; 
				if (Ref1 != ToRef)
			    	ActualCost+=PickList[0].Length;
		    	st = BT_FIND (hBTAt,(LPSTR)&Ref,BT_FIRST,BT_EQ,(LPSTR)&AtData);
			}
		    sprintf (str,"MaxCost=%f  ActualCost=%f",MaxCost,ActualCost);
			if (DisplayMarkers)
			    SetWindowText(hWndMain,str);
	    }
	    *CurVis = SaveVis;  
	    MaxPick = SaveMaxPick;  
		UseUserPickAp =TRUE;
	    Pick = PickingByRefno = FALSE;
	    CloseNetLinkAndRef (Opened1);
		CloseNetIntersect(Opened2); 
	    GSSiSetCursor(hcurSave);
	    hCursor = 0; 
        PostMessage(hWnd, GF_CLOSE,0, 0L); 
    }
		break;
    case WM_KEYDOWN:
		key = wParam;
		switch (key)
		{
			case 27: //ESC
		        PostMessage(hWnd, GF_CLOSE, 0,0L); 
		        return TRUE;
		}
       	return FALSE;

    default:
    	return (FALSE);
    }
    return (TRUE);
} 

int AddNextRefs (long AtRef,DPOINT AtPoint,long AtPath,double AtMP,short AtDir,
				 long Start, double StartCost, DPOINT ToPoint,long Sequence)
{   int	n=0, OrigNumPicked, np, NumInt=0,ii; 
	double	dist1, dist2, IntAZ[64], IntPCT[64], IntLength[64];
	long	IntRefs[64], IntStreets[64], JoinRef;
	BOOL	Join=FALSE, HaveNonNet=FALSE, SkipNetCheck=FALSE;
	ATDATA		AtData;
	NEXTDATA	NextData;       
	NEXTKEY		NextKey;  
	NETINTREFKEY	NetIntRefKey;
	NETINTREFDATA	NetIntRefData;   
	NETREFSKEY	NetRefsKey;
	NETREFSDATA NetRefsData; 
	NETINTMPKEY NetIntMPKey;
	NETLINKSKEY NetLinksKey;
	NETLINKSDATA NetLinksData;
	SEGDATAGM SegdataNext, SegdataAt;
	long	AtInt;
	static	long	DebugRef=-2147436847;
	int	st, pos, NumAtInt; 
	long	PickPaths[64];
	short	PickDirs[64];
	double	PickMPs[64]; 
	double	Speed=30, IntersectionCost=0, SegmentCost, MinCostToEnd, DistFactor=(MFT*60)/5280;

        if (AtRef == DebugRef)
        	ii=1;
        if (AtRef == -2147436773)
        	ii=1;
Top:        
        if (LimitRouteToScreen && !PtInWBounds(AtPoint))
        	return 0;
        if (RouteByNet)
        {   
        	NumPicked = 0;
			if (!NetworkID || !hBTNetRefs)
				goto DonePick;  
			if ((AtInt = AtNetIntersection (AtPath, AtMP-NetTOL, AtMP+NetTOL,FALSE)))
			{   
				NumAtInt = 0;
				NetIntRefKey.IntID = AtInt;
				NetIntRefKey.Refno = LONG_MIN;
				st = BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,BT_FIRST,BT_GE,(LPSTR)&NetIntRefData); 
				while (!st && NetIntRefKey.IntID == AtInt)
				{   
					if (AtRef != NetIntRefKey.Refno)
					{   
						NetLinksKey.Ref = NetIntRefKey.Refno;
						NetLinksKey.NetID = NetworkID;  
						NetLinksKey.Path = 0;  
						st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData);
						if (!st && NetLinksKey.Ref == NetIntRefKey.Refno && NetLinksKey.NetID == NetworkID)
						{
							if (NumAtInt)
							{   
								PickMPs[NumAtInt] = PickMPs[0];
								PickPaths[NumAtInt] = PickPaths[0];
								PickDirs[NumAtInt] = PickDirs[0];
								PickList[NumAtInt] = PickList[0]; 
							}
							NumAtInt++;
							if ((NetLinksData.Length < 0 && NetIntRefData.WhichEnd == 1) ||  
								(NetLinksData.Length > 0 && NetIntRefData.WhichEnd == 2))
							{
								PickDirs[0] = -1;
								PickMPs[0] = NetLinksData.MP; 
							}
							else
							{
								PickDirs[0] = 1;
								PickMPs[0] = NetLinksData.MP + fabs(NetLinksData.Length);
							}
//							PickMPs[0] += PickDirs[0] * fabs(NetLinksData.Length);
							PickPaths[0] = NetLinksKey.Path;  
							PickByRefno (NetIntRefKey.Refno,NULL,NULL,FastPickFileNum); 
						}
					}
					st = BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,BT_NEXT,BT_ANY,(LPSTR)&NetIntRefData); 
				} 
				NumPicked = NumAtInt;
			} 
			else
			{
				NetRefsKey.Path = AtPath;
				NetRefsKey.MP = AtMP-NetTOL;
				st = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GT,(LPSTR)&NetRefsData); 
				if (st || NetRefsKey.Path != AtPath)
					goto DonePick;
				if (NetRefsData.Ref == AtRef) 
				{ 
					if (AtDir < 0)  
						pos = BT_PRIOR;
					else
						pos = BT_NEXT;
					st = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,pos,BT_ANY,(LPSTR)&NetRefsData); 
					if (st || NetRefsKey.Path != AtPath)
						goto DonePick;
					if (NetRefsData.Ref == AtRef)
					{
						AtMP = NetRefsKey.MP;
						goto Top;
					}
				} 
				PickPaths[0] = NetRefsKey.Path;
				PickMPs[0] = NetRefsKey.MP;
				PickDirs[0] = AtDir;
				PickByRefno (NetRefsData.Ref,NULL,NULL,FastPickFileNum); 
				if (PickDirs[0] > 0)
					PickMPs[0] += PickList[0].Length;
			}			
        }
        else
	    	PickItems2 (hWndMain,AtPoint);  

DonePick:
	    OrigNumPicked = NumPicked; 
	    if (NumPicked)
   			GetSegDataGM (AtRef,&SegdataAt);
	    
	    while (NumPicked--)
	    {  
 	    	if (PickList[NumPicked].Length <=0)
	    		goto Next; 
	    	if (PickList[NumPicked].Refno == AtRef)
	    	{
	    		IntAZ[NumInt] = PickList[NumPicked].AZ;  
	    		IntPCT[NumInt] = PickList[NumPicked].PCT;  
	    		IntLength[NumInt] = PickList[NumPicked].Length; 
	    		IntStreets[NumInt] = PickedStreets[NumPicked][0];
	    		IntRefs[NumInt++] = PickList[NumPicked].Refno;
	    		goto Next; 
	    	}
	    	if (!SkipNetCheck)
	    	{
		    	if (TraceNet && !RefInNet (PickList[NumPicked].Refno))
		    	{
		    		HaveNonNet=TRUE;
		    		goto Next;      
		    	} 
		    }
	    	dist1 = ldistp(AtPoint,PickList[NumPicked].BeginPoint);
	    	dist2 = ldistp(AtPoint,PickList[NumPicked].EndPoint);
	    	if (dist1 < dist2 && dist1 <= RouteTOL)
	    	{   
	    		np = OrigNumPicked;
	    		while (np--)
	    		{
	    			if (np != NumPicked)
	    			{
	    				if (ldistp (PickList[NumPicked].BeginPoint,
	    							PickList[np].BeginPoint) < dist1 ||
	    					ldistp (PickList[NumPicked].BeginPoint,
	    							PickList[np].EndPoint) < dist1)
	    					goto Next;
	    			}
	    		}
		    	if (!BT_FIND(hBTAt,(LPSTR)&PickList[NumPicked].Refno,BT_FIRST,BT_EQ,(LPSTR)&AtData))
		    	{
		    		if (AtData.StartRef == Start) 
		    		{
		    			Join = TRUE; 
		    			JoinRef = PickList[NumPicked].Refno;
		    			goto Next; 
		    		}
		    	}  
	    		NextData.OpenEnd = PickList[NumPicked].EndPoint; 
				goto AddNext;
	    	}
	    	else if (dist2 <= dist1 && dist2 <= RouteTOL)
	    	{ 
	    		np = OrigNumPicked;
	    		while (np--)
	    		{
	    			if (np != NumPicked)
	    			{
	    				if (ldistp (PickList[NumPicked].EndPoint,
	    							PickList[np].BeginPoint) < dist2 ||
	    					ldistp (PickList[NumPicked].EndPoint,
	    							PickList[np].EndPoint) < dist2)
	    					goto Next;
	    			}
	    		}
		    	if (!BT_FIND(hBTAt,(LPSTR)&PickList[NumPicked].Refno,BT_FIRST,BT_EQ,(LPSTR)&AtData))
		    	{
		    		if (AtData.StartRef == Start) 
		    		{
		    			Join = TRUE;
		    			JoinRef = PickList[NumPicked].Refno;
		    			goto Next; 
		    		}
		    	}  
	    		NextData.OpenEnd = PickList[NumPicked].BeginPoint; 
	AddNext: 
		    	NextData.AtRef = AtRef;
	    		NextData.StartRef = Start;
	    		NextData.Sequence = Sequence;
				NextData.Segment = PickList[NumPicked].Segment;
				NextData.Offset = PickList[NumPicked].Offset;
				NextData.FileNum = PickList[NumPicked].FileNum;
				NextData.SubFile = PickList[NumPicked].SubFile;
		        NextData.FileInIndex = PickList[NumPicked].FileInIndex;  
	        	_fmemmove (NextData.StreetNums,PickedStreets[NumPicked],16);
	    		NextData.AtCost = StartCost; 
	    		NextKey.Refno = PickList[NumPicked].Refno; 
	    		if (GetSegDataGM (NextKey.Refno,&SegdataNext))
	    		{
	    			Speed = GetSegmentSpeed (&SegdataNext,NextData.AtCost);  
	    			IntersectionCost = GetIntersectionCost (&SegdataAt,&SegdataNext);
	    		}
	    		else 
	    		{
	    			Speed = MinSpeed;
	    			IntersectionCost = 0;
	    		}

	    		if (!Speed)
	    			SegmentCost = MAX_COST;
	    		else
	    			SegmentCost = (PickList[NumPicked].Length*DistFactor)/Speed;
				if (RouteType != WALKOUTROUTE)  
		    		MinCostToEnd = ldistp(NextData.OpenEnd,ToPoint)*DistFactor/MaxSpeed; 
		    	else
		    		MinCostToEnd = 0;
	    		NextData.NextCost = StartCost + IntersectionCost + SegmentCost; 
	    		NextKey.Cost = NextData.NextCost + MinCostToEnd; 
	    		NextKey.Cost += (UniqueCost += UniqueCostInc);
	    		NextData.MP = PickMPs[NumPicked];
	    		NextData.Path = PickPaths[NumPicked];
	    		NextData.Dir = PickDirs[NumPicked];  
	    		NextData.WhenAssigned = WhenAssigned++;
	    		BT_PUT (hBTNext,(LPSTR)&NextKey,(LPSTR)&NextData); 
	    		IntAZ[NumInt] = PickList[NumPicked].AZ;  
	    		IntPCT[NumInt] = PickList[NumPicked].PCT; 
	    		IntLength[NumInt] = PickList[NumPicked].Length;  
	    		IntStreets[NumInt] = NextData.StreetNums[0];
	    		IntRefs[NumInt++] = PickList[NumPicked].Refno;
	    		n++;  
	    	}
Next:;
	    }
    
	if (n>1 || (n==1 && IntStreets[0] != IntStreets[1]))
	{
		DisplayMarker(AtPoint,3,NULL,0,0,0,TRUE); 
		if (!Join)
			AddIntersect (AtPoint,AtRef,NumInt,IntRefs,IntAZ,IntPCT,IntLength);
		AddChain (AtRef,NULL);
	}  
	else if (!n && !Join)
	{
		if (HaveNonNet && !SkipNetCheck)
		{
			SkipNetCheck=TRUE;
			goto Top;
		}
		DisplayMarker(AtPoint,3,NULL,0,0,RGB(255,0,0),TRUE);
		AddChain (AtRef,NULL);
	}  
	else if (!n && Join) 
	{
		DisplayMarker(AtPoint,3,NULL,0,0,RGB(0,255,0),TRUE);  
		AddChain (AtRef,&JoinRef);
		n=1;
	}
	else if (n && Join) 
	{
		DisplayMarker(AtPoint,3,NULL,0,0,RGB(0,0,0),TRUE);  
		AddChain (AtRef,NULL);
	}
	
	return n;
}

BOOL FAR PASCAL BTREE_REORGMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
	char	Name[128]="", NewName[128], mess[256];	  
	HANDLE	hBT, hBT2;
   	LPGWDHEADER	lpGWDHead;
	BTHEAD BTHead; 
	long	TotRecs,Done;
	short	pos;
	LPSTR	pKey, pData, lpDot;
	HANDLE	hKey, hData;   
	BTVARDESC   BTVar[8];
	
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
    
			
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);
            break;  
            
            case IDC_FIND_FILE: 
            	 
				 if (!GetFileName2 (hWndDlg,Name,".btr",0))
				 	break; 
				 SetDlgItemText (hWndDlg,IDC_FILE_NAME,Name);
				 hBT = BT_OPEN (Name, 0, BT_READ, 0);
         		 GetBTHeader (hBT,&BTHead);  
         		 sprintf (mess,"%ld bytes for %ld records in %i levels",BTHead.BT_LENGTH,BTHead.BT_NUMRECS,BTHead.BT_NUMLEVS);  
         		 SetDlgItemText (hWndDlg,IDC_MESS1,mess);
         		 BT_CLOSE (hBT);
         		 break;   
         		 
            case IDOK: 
            	 
				 if (!GetDlgItemText (hWndDlg,IDC_FILE_NAME,Name,sizeof(Name)))
				 	break; 
				 BT_SET_PARMS (8,16,7,15);
				 hBT = BT_OPEN (Name, 0, BT_READ, 0);
         		 GetBTHeader (hBT,&BTHead);   
         		 GetBTVarDesc (hBT,BTVar);
         		 
         		 _fstrcpy (NewName,Name); 
         		 lpDot = _fstrrchr (NewName,'.');
         		 *lpDot = 0;
         		 _fstrcat (NewName,".rbt");
			     BT_CREATE (NewName, BTHead.BT_DATLEN, FALSE, BTHead.BT_NVARS, 1,(LPBTVARDESC) BTVar,FALSE, 0, 0, FALSE);
				 hBT2 = BT_OPEN (NewName, 0, BT_WRITE, 0); 
				 pos = BT_FIRST;
				 Done = 0; 
				 hKey  = GSSiGlobAlloc (GHND,1024);
				 pKey = GlobalLock (hKey);
				 hData  = GSSiGlobAlloc (GHND,1024);
				 pData = GlobalLock (hData);    
				 TotRecs = BTHead.BT_NUMRECS;
				 while (!BT_FIND (hBT,pKey,pos,BT_ANY,pData))
				 {  
				 	pos = BT_NEXT;
				 	BT_PUT (hBT2,pKey,pData); 
				 	PctBox (GetDlgItem(hWndDlg,IDC_STATUS),TotRecs, Done++,0);
				 } 
				 GlobalUnlock (hKey);
				 GlobalUnlock (hData);
				 GlobalFree (hKey);
				 GlobalFree (hData);
         		 BT_CLOSE (hBT); 
         		 GetBTHeader (hBT2,&BTHead);  
         		 sprintf (mess,"%ld bytes for %ld records in %i levels",BTHead.BT_LENGTH,BTHead.BT_NUMRECS,BTHead.BT_NUMLEVS);  
         		 SetDlgItemText (hWndDlg,IDC_MESS2,mess);
         		 BT_CLOSE (hBT2); 
         		 GSSiRemove (Name);
         		 rename (NewName,Name);
         		 break;
            
         }
         break;   

    default:
        return FALSE;
   }
 return TRUE;
}  

long CheckForDupLines (long Refno,POINT BP,POINT EP)
{   
	long	dupref;
	
	struct {
			POINT Point1, Point2;
			}	DupLineKey;
	
	if (BP.x == EP.x)
	{   
		if (BP.y < EP.y) 
		{
			DupLineKey.Point1 = BP;
			DupLineKey.Point2 = EP;
		}
		else            
		{
			DupLineKey.Point1 = EP;
			DupLineKey.Point2 = BP;
		}
	}
	else
	{   
		if (BP.x < EP.x) 
		{
			DupLineKey.Point1 = BP;
			DupLineKey.Point2 = EP;
		}
		else            
		{
			DupLineKey.Point1 = EP;
			DupLineKey.Point2 = BP;
		}
	}
	if (!BT_FIND (hBTDups,(LPSTR)&DupLineKey,BT_FIRST,BT_EQ,(LPSTR)&dupref))
		return dupref;
	BT_PUT (hBTDups,(LPSTR)&DupLineKey,(LPSTR)&Refno); 
	return 0;
}    

void CloseRefConnectFile (void)
{
	if (hDBRefConnect)
    	CloseGWDatabase (hDBRefConnect);  
    hDBRefConnect=0;
	BT_CLOSEANDDELETE (&hBTDups);
    _lclose (FidDupList);
    return;
}
   
void DumpLinkData (LPSTR InName) 
{
	short	pos=BT_FIRST;
	long	LinkRef;    	
    BTVARDESC  *pVars;  
    short       NumFields, Reclen, len;
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hVars, hDB;
    short       FidData,ibeg,NumVars;
    OFSTRUCT    OFStruct;
    GWFLDINFO FldInfo; 
    char	Name[128];   
    char	PrimeIndex[128];
    LPSTR	lpDot, lpEnd;
    typedef struct {
    				long	Refno;
 					LPOINT	FromNode,
 							ToNode; 
 					double	FromAZ,
 							ToAZ;
 				}	LINKDB;
    typedef	LINKDB	FAR	*LPLINKDB;   
    LINKDATA	LinkData;
    
    LPLINKDB	pLinkDB;
    _fstrcpy (Name,InName);
    ExpandText (Name); 
    _fstrcpy (PrimeIndex,Name);
    lpDot = _fstrrchr (PrimeIndex,'.');
    *lpDot = 0;
    _fstrcat (lpDot,".in1");
    lpGWDHead = &GWDHead; 
    
    FidData = GSSiOpenFile (Name,&OFStruct,OF_CREATE);   
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER));
    GWDHead.NumIndex=1;
    GWDHead.Version=1;
    GWDHead.NumIndexFields[0]=1;
    GWDHead.IndexFields[0][0]=0;
    _lwrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER));
    ibeg = 0;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"INT_REFNO");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"FromNodeX");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"FromNodeY");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"ToNodeX");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"ToNodeY");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"FromAZ");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"ToAZ");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

	GWDHead.Reclen=ibeg; 
	GWDHead.TimeStamp = time(0);
	_llseek (FidData,0,0);
	_lwrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER));
	_llseek (FidData,0,2);
	                 
	NumVars = 1;
	            
	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
	pVars =(LPBTVARDESC) LocalLock(hVars);
	            
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (PrimeIndex, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
	LocalUnlock(hVars);
	LocalFree(hVars); 
	_lclose (FidData);
	
	hDB = OpenGWDatabase (Name,BT_WRITE);
	if (!hDB) return;
    lpGWDHead = GlobalLock (hDB);   
	pLinkDB = &lpGWDHead->GWDData; 
	while (!BT_FIND (hBTLinks,(LPSTR)&LinkRef,pos,BT_ANY,(LPSTR)&LinkData)) 
	{   
		pos = BT_NEXT;
		pLinkDB->Refno = LinkRef;
		pLinkDB->FromNode=LinkData.FromNode;
		pLinkDB->ToNode=LinkData.ToNode;
		pLinkDB->FromAZ=LinkData.FromAZ;
		pLinkDB->ToAZ=LinkData.ToAZ;
		GWDAddRecord (lpGWDHead,0,NULL);
	}
	GlobalUnlock (hDB);
	CloseGWDatabase (hDB); 
	return;  
}


BOOL OpenRefConnectionFile (LPSTR Name)
{   
	HANDLE	hDB; 
	OFSTRUCT	OFStruct;
	
	hDBRefConnect = OpenGWDatabase (Name,BT_WRITE);
	if (!hDBRefConnect)                          
	{
		CreateRefConnectionFile (Name);
		hDBRefConnect = OpenGWDatabase (Name,BT_WRITE);
	}  
	hBTDups = BT_OPEN ("dupcheck.btr", 0, BT_WRITE, 0);   
	FidDupList = GSSiOpenFile ("duplist.bin",&OFStruct,OF_CREATE);
	return TRUE;
}

BOOL CreateRefConnectionFile (LPSTR InName)
{
	ATDATA		AtData;       
	long		AtKey;   
	short		pos=BT_FIRST;
    BTVARDESC  *pVars;  
    BTVARDESC	Vars[3];
    short       NumFields, Reclen, len;
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hVars, hDB;
    short       FidData,ibeg,NumVars;
    OFSTRUCT    OFStruct;
    GWFLDINFO FldInfo; 
    char	Name[128];   
    char	PrimeIndex[128];
    LPSTR	lpDot, lpEnd;
   
    _fstrcpy (Name,InName);
    ExpandText (Name); 
    _fstrcpy (PrimeIndex,Name);
    lpDot = _fstrrchr (PrimeIndex,'.');
    *lpDot = 0;
    _fstrcat (lpDot,".in1");
    lpGWDHead = &GWDHead; 
    
    FidData = GSSiOpenFile (Name,&OFStruct,OF_CREATE);   
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER));
    GWDHead.NumIndex=3;
    GWDHead.Version=1;
    GWDHead.NumIndexFields[0]=1;
    GWDHead.IndexFields[0][0]=0;
    GWDHead.NumIndexFields[1]=3;
    GWDHead.NumIndexFields[2]=3;
    GWDHead.IndexFields[1][0]=1;
    GWDHead.IndexFields[1][1]=2;
    GWDHead.IndexFields[1][2]=2;
    GWDHead.IndexFields[1][3]=0;
    GWDHead.IndexFields[2][0]=3;
    GWDHead.IndexFields[2][1]=4;
    GWDHead.IndexFields[2][2]=0;
    GWDHead.IndexFields[2][3]=0;
    _lwrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER));
    ibeg = 0;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"INT_REFNO");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"BPXFile");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"BPYFile");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"EPXFile");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"EPYFile");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"ConnectionStatus");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"BPConnections");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"EPConnections");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"NumPoints");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"Symbol");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"BPXBase");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"BPYBase");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"EPXBase");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"EPYBase");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 8;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_REAL;
    _fstrcpy (FldInfo.Name,"Length");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

	GWDHead.Reclen=ibeg; 
	GWDHead.TimeStamp = time(0);
	_llseek (FidData,0,0);
	_lwrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER));
	_llseek (FidData,0,2);
	                 
	NumVars = 1;
	            
	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
	pVars =(LPBTVARDESC) LocalLock(hVars);
	            
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (PrimeIndex, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
	LocalUnlock(hVars);
	LocalFree(hVars); 
	_lclose (FidData);
	lpEnd = _fstrchr (PrimeIndex,0);
	lpEnd--;
	*lpEnd = '2';
	GSSiRemove (PrimeIndex);
	*lpEnd = '3';
	GSSiRemove (PrimeIndex);
	
	hDB = OpenGWDatabase (Name,BT_WRITE);
	if (!hDB) return (FALSE);
	CloseGWDatabase (hDB); 
	 
	Vars[0].BT_VARLEN=4;
	Vars[0].BT_VARTYP=BT_INTEGER;
	Vars[0].BT_VAROFF=0;
	Vars[1].BT_VARLEN=4;
	Vars[1].BT_VARTYP=BT_INTEGER;
	Vars[1].BT_VAROFF=4;
	BT_CREATE ("dupcheck.btr", 4, FALSE, 2, 1,Vars,FALSE, 0, GWDHead.TimeStamp, FALSE);
	
	
	return TRUE;
} 

BOOL CreateRefConnectTable (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{HDC hDC;
 int	st, BPConnect,EPConnect,SaveMaxPick;
 LPGWDHEADER lpGWDHead;
 BOOL	HaveMidHit;
 HIGHLIGHTDATA	HighlightData;         
 LPREFCONNECT	pRC;
 short	pos, pos2, len;  
 long	Refno, Offset, NumDups=0, dupref;  
 HCURSOR	hcurSave;
 double	SnapTol=0.001;   
 POINT	Point; 
 char	mess[128];
 struct {
		POINT	Point; 
		long	Ref;
		short	End;
		} ConnectKey;
   
 switch (Message)
   {
   	case GF_INIT:
		PostMessage(hWnd, GF_EXECUTE,0, 0L); 
   		break;
    
    case GF_EXECUTE:
    {   
    	LPLINEINT	hpLineInt, hpLineIntStart, hpLineInt1, hpLineInt2;
    	long		maxlines, nHighlight;  
    	DWORD		il, jl, NumLines=0; 
    	POINT		LastPoint;  
    	LPTHEME		pTheme; 
    	MNMXCORD	IntBounds;
    	
		hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
		GSSiRemove ("[%CONSTATFILE].gmd");
		OpenRefConnectionFile ("[%CONSTATFILE].gmd");
	    lpGWDHead = GlobalLock (hDBRefConnect);   
	    pRC = &lpGWDHead->GWDData; 
	    pos = BT_FIRST;

		
		while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
		{   
			pos = BT_NEXT; 
			if (!HighlightData.PD.Length)
				goto NextRec;
			if (HighlightData.PD.NumPoints == 2)
			{   
				if ((dupref=CheckForDupLines (Refno,HighlightData.PD.BeginPointFile,HighlightData.PD.EndPointFile)))
				{
					NumDups ++; 
					_lwrite (FidDupList,&Refno,4);
					sprintf (mess,"%ld dup with %ld",Refno,dupref);
					SetWindowText (hWndMain,mess); 
				}
			}
			
			BPConnect=EPConnect=0;  
			ConnectKey.Point = HighlightData.PD.BeginPointFile;
			ConnectKey.Ref = ConnectKey.End = 0;  
	        while (!BT_FIND (lpGWDHead->BTHandle[1],&ConnectKey,BT_FIRST,BT_GT,(LPSTR)&Offset))
	        {    
	        	if (ConnectKey.Point.x == HighlightData.PD.BeginPointFile.x &&
	        		ConnectKey.Point.y == HighlightData.PD.BeginPointFile.y)
	        	{
	        		len = FillGWDData (lpGWDHead,Offset); 
	        		pRC->BPConnections++;
					pRC->ConnectStatus=min(pRC->BPConnections,1)+min(pRC->EPConnections,1);
					GWDReplaceRecord (lpGWDHead,len,NULL,Offset);  
					BPConnect++; 
				}
				else break;
        	}
			ConnectKey.Point = HighlightData.PD.BeginPointFile;
			ConnectKey.Ref = ConnectKey.End = 0;
	        while (!BT_FIND (lpGWDHead->BTHandle[2],&ConnectKey,BT_FIRST,BT_GT,(LPSTR)&Offset))
	        {    
	        	if (ConnectKey.Point.x == HighlightData.PD.BeginPointFile.x &&
	        		ConnectKey.Point.y == HighlightData.PD.BeginPointFile.y)
	        	{
	        		len = FillGWDData (lpGWDHead,Offset); 
	        		pRC->EPConnections++;
					pRC->ConnectStatus=min(pRC->BPConnections,1)+min(pRC->EPConnections,1);
					GWDReplaceRecord (lpGWDHead,len,NULL,Offset);  
					BPConnect++; 
				}
				else break;
        	}
			ConnectKey.Point = HighlightData.PD.EndPointFile;
			ConnectKey.Ref = ConnectKey.End = 0;
	        while (!BT_FIND (lpGWDHead->BTHandle[1],&ConnectKey,BT_FIRST,BT_GT,(LPSTR)&Offset))
	        {    
	        	if (ConnectKey.Point.x == HighlightData.PD.EndPointFile.x &&
	        		ConnectKey.Point.y == HighlightData.PD.EndPointFile.y)
	        	{
	        		len = FillGWDData (lpGWDHead,Offset); 
	        		pRC->BPConnections++;
					pRC->ConnectStatus=min(pRC->BPConnections,1)+min(pRC->EPConnections,1);
					GWDReplaceRecord (lpGWDHead,len,NULL,Offset);  
					EPConnect++;  
				}
				else break;
        	}
			ConnectKey.Point = HighlightData.PD.EndPointFile;
			ConnectKey.Ref = ConnectKey.End = 0;
	        while (!BT_FIND (lpGWDHead->BTHandle[2],&ConnectKey,BT_FIRST,BT_GT,(LPSTR)&Offset))
	        {    
	        	if (ConnectKey.Point.x == HighlightData.PD.EndPointFile.x &&
	        		ConnectKey.Point.y == HighlightData.PD.EndPointFile.y)
	        	{
	        		len = FillGWDData (lpGWDHead,Offset); 
	        		pRC->EPConnections++;
					pRC->ConnectStatus=min(pRC->BPConnections,1)+min(pRC->EPConnections,1);
					GWDReplaceRecord (lpGWDHead,len,NULL,Offset);  
					EPConnect++;
				}
				else break;
        	}
			pRC->Ref = Refno;
			pRC->BPFile = HighlightData.PD.BeginPointFile;
			pRC->EPFile = HighlightData.PD.EndPointFile; 
			pRC->BPConnections = BPConnect;
			pRC->EPConnections = EPConnect;  
			pRC->ConnectStatus=min(pRC->BPConnections,1)+min(pRC->EPConnections,1);
			pRC->BPBase = HighlightData.PD.BeginPoint;
			pRC->EPBase = HighlightData.PD.EndPoint; 
			pRC->Length = HighlightData.PD.Length;
			pRC->NumPoints = HighlightData.PD.NumPoints;
			pRC->Symbol = HighlightData.PD.Desc;  
		    GWDAddRecord (lpGWDHead,0,NULL);    
NextRec:;
		} 

		UseUserPickAp =FALSE;
		SystemPickAp = -1;	
		SaveMaxPick=MaxPick;
		MaxPick=16;
		SetPickAp();
	    Refno = LONG_MIN;
		while (!BT_FIND (lpGWDHead->BTHandle[0],&Refno,BT_FIRST,BT_GT,(LPSTR)&Offset)) 
		{   
			pos = BT_NEXT; 
       		len = FillGWDData (lpGWDHead,Offset);   
       		HaveMidHit=FALSE;
       		if (pRC->BPConnections>1)
       		{
				PickItems2 (CurView->hWnd,pRC->BPBase); 
				while (NumPicked--)
				{   
					if (((PickList[NumPicked].PCT * PickList[NumPicked].Length) > SnapTol) &&
						(((1.0-PickList[NumPicked].PCT) * PickList[NumPicked].Length) > SnapTol)) 
						HaveMidHit=TRUE;
				} 
       		} 
       		if (pRC->EPConnections>1)
       		{
				PickItems2 (CurView->hWnd,pRC->EPBase); 
				while (NumPicked--)
				{   
					if (((PickList[NumPicked].PCT * PickList[NumPicked].Length) > SnapTol) &&
						(((1.0-PickList[NumPicked].PCT) * PickList[NumPicked].Length) > SnapTol)) 
						HaveMidHit=TRUE;
				} 
       		} 
       		if (HaveMidHit)
       		{
       			pRC->ConnectStatus = 3;
				GWDReplaceRecord (lpGWDHead,len,NULL,Offset);
			}  
       	}

		UseUserPickAp =TRUE;
		MaxPick=SaveMaxPick;
		GlobalUnlock (hDBRefConnect);  
		IntBounds = HLTBounds;
		ClearHighlightList ();
		_llseek (FidDupList,0,0);
		while (_lread (FidDupList,&Refno,4) == 4)
		{ 
			if (PickByRefno(Refno,NULL,NULL,-1))
				DeletePickedItem (0,12,92);
		}    
		

        if (GetGlobalBVal ("[%CHECKINTS]"))
        {   
        	MNMXCORD Bounds[16];
        	short	iblock, irow, icol;  
        	double	rowheight, colwidth, colx, rowy;
        	
        	rowheight = (IntBounds.ymx - IntBounds.ymn)/4;
        	colwidth = (IntBounds.xmx - IntBounds.xmn)/4;   
        	
        	iblock=0;
        	for (irow=0,rowy=IntBounds.ymn;irow<4;irow++,rowy+=rowheight)
	        	for (icol=0,colx=IntBounds.xmn;icol<4;icol++,colx+=colwidth)
	        	{ 
	        		Bounds[iblock].xmn = colx;
	        		Bounds[iblock].ymn = rowy;
	        		Bounds[iblock].xmx = colx + colwidth;
	        		Bounds[iblock++].ymx = rowy + rowheight;
	        	}
        	
        	for (iblock=0;iblock<16;iblock++)
        	{
				ClearHighlightList ();
				HighlightInArea (CurView->hWnd,&Bounds[iblock]); 
				if (TotHLTPoints)
				{       
			   		hpLineInt = hpLineIntStart = (LPLINEINT)_halloc(TotHLTPoints, sizeof(LINEINT) );
				    pos = BT_FIRST;
			        NumLines=0;
					
					while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
					{   
						pos = BT_NEXT; 
						if (HighlightData.PD.NumPoints == 2)
						{   
							hpLineInt->Point1 = HighlightData.PD.BeginPointFile;
							hpLineInt->Point2 = HighlightData.PD.EndPointFile;
							hpLineInt->Refno = Refno;
							hpLineInt->Mnx = min (HighlightData.PD.BeginPointFile.x,HighlightData.PD.EndPointFile.x);
							hpLineInt++->Mxx = max (HighlightData.PD.BeginPointFile.x,HighlightData.PD.EndPointFile.x);
							NumLines++;
						}
						else
						{   
							PickList[0] = HighlightData.PD;
							pTheme = AddTheme (GF_SAVEPOLYFILE_THEME);
							CurView->PassID = 4;
							ProcessPickedItem (0,FALSE);        		
							DeleteTheme (pTheme);
					   		if (hSavePoly)
							{   LPMINMAX lpRect;
								LPINT	npt;
								short	nPnts; 
								LPPOINT	lppoint;
						    		    
					            nPnts = nSavePoly; 
					            lpRect = (LPMINMAX) GlobalLock (hSavePoly);
					            lpRect++;
					            lppoint = (LPPOINT) lpRect; 
					            LastPoint = *lppoint++; 
					            nPnts--;
					            while (nPnts--)
					            {
									hpLineInt->Point1 = LastPoint;
									hpLineInt->Point2 = *lppoint;
									hpLineInt->Refno = Refno;
									hpLineInt->Mnx = min (hpLineInt->Point1.x,hpLineInt->Point2.x );
									hpLineInt++->Mxx = max (hpLineInt->Point1.x,hpLineInt->Point2.x );
									NumLines++;  
									LastPoint = *lppoint++;
					            }  
					            GlobalUnlock (hSavePoly);
					            GlobalFree (hSavePoly);
					         }
							
						}  
					}	
			        for (il = 0, hpLineInt1=hpLineIntStart; il<NumLines-1; il++,hpLineInt1++) 
			        	for (jl = il+1,hpLineInt2=hpLineInt1+1;jl < NumLines;jl++,hpLineInt2++)
			        		if (LineInt (hpLineInt1,hpLineInt2))
			        		{
								BT_FIND (lpGWDHead->BTHandle[0],&hpLineInt1->Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset);
					       		len = FillGWDData (lpGWDHead,Offset);   
								pRC->ConnectStatus = 4;
								GWDReplaceRecord (lpGWDHead,len,NULL,Offset);
								BT_FIND (lpGWDHead->BTHandle[0],&hpLineInt2->Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset);
					       		len = FillGWDData (lpGWDHead,Offset);   
								pRC->ConnectStatus = 4;
								GWDReplaceRecord (lpGWDHead,len,NULL,Offset);
			                 }
			        if (hpLineInt)
			        	_hfree( hpLineInt); 
			    }
		  	}
		}
		
		ClearHighlightList ();
		
		CloseRefConnectFile ();
	    GSSiSetCursor(hcurSave); 
        RemoveGraphicsFunction (hWnd);  
   }
	
		break;
/*typedef struct {
		long	Ref;
		POINT	BPFile,EPFile; 
		short	ConnectStatus,
				BPConnections, EPConnections;
		DPOINT	BPBase, EPBase;
		} REFCONNECT;  */
		
    default:
    	return (FALSE);
    }
    return (TRUE);
}   

BOOL RefConnectFix (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{HDC hDC;
 int	st, BPConnect,EPConnect;
 LPGWDHEADER lpGWDHead;
 HIGHLIGHTDATA	HighlightData;
 LPREFCONNECT	pRC;
 short	pos, pos2, len,ii, SnapItem, FixItem, SaveMaxPick;  
 long	Refno, Offset, NearOpRef, NumRemoved=0, Newref1, Newref2;  
 HCURSOR	hcurSave;   
 POINT	Point;  
 DPOINT	SnapPointBase;
 double	FixTol=GetGlobalDVal("[%FIXTOL]"), NearOpDist, MinSegLength=FixTol*0.5;
 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		break;
    
    case WM_LBUTTONUP:
		hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
		OpenRefConnectionFile ("[%CONSTATFILE].gmd");
	    lpGWDHead = GlobalLock (hDBRefConnect);
		UseUserPickAp =FALSE;
		SystemPickAp = -FixTol-0.5;	
		SaveMaxPick=MaxPick;
	    pRC = &lpGWDHead->GWDData;   
	    Refno = LONG_MIN;
		while (!BT_FIND (lpGWDHead->BTHandle[0],&Refno,BT_FIRST,BT_GT,(LPSTR)&Offset)) 
		{   
       		len = FillGWDData (lpGWDHead,Offset);
       		if (pRC->Length < MinSegLength)			//eliminates very short segments
       		{
		       	DoNotPickThisRefno=LONG_MAX;
				SnapPointFile = AveragePoint (pRC->BPFile,pRC->EPFile);
				SnapPointBase = AverageDPoint (pRC->BPBase,pRC->EPBase); 
		       	MaxPick=64;
				PickItems (CurView->hWnd,pRC->BPBase); 
				while (NumPicked--)
				{
					if (PickList[NumPicked].Refno == Refno)  
					{
						NumRemoved++;
						DeletePickedItem (NumPicked,12,92); 
					}
					else
					{   
						if (ldistp (pRC->BPBase,PickList[NumPicked].BeginPoint) < FixTol/2)
						{   
							SnapEnd = 3;
							SnapPickedItem (NumPicked); 
							UpdateConnectFile (PickList[NumPicked].Refno,lpGWDHead,
												SnapPointBase,PickList[NumPicked].EndPoint,
												SnapPointFile,PickList[NumPicked].EndPointFile);
						}
						else if (ldistp (pRC->BPBase,PickList[NumPicked].EndPoint) < FixTol/2)
						{   
							SnapEnd = 4;
							SnapPickedItem (NumPicked);
							UpdateConnectFile (PickList[NumPicked].Refno,lpGWDHead,
												PickList[NumPicked].BeginPoint,SnapPointBase,
												PickList[NumPicked].BeginPointFile,SnapPointFile);
						}
					}
				}
				PickItems (CurView->hWnd,pRC->EPBase); 
				while (NumPicked--)
				{
					if (ldistp (pRC->BPBase,PickList[NumPicked].BeginPoint) < FixTol/2)
					{   
						SnapEnd = 3;
						SnapPickedItem (NumPicked);
						UpdateConnectFile (PickList[NumPicked].Refno,lpGWDHead,
												SnapPointBase,PickList[NumPicked].EndPoint,
												SnapPointFile,PickList[NumPicked].EndPointFile);
					}
					else if (ldistp (pRC->BPBase,PickList[NumPicked].EndPoint) < FixTol/2)
					{   
						SnapEnd = 4;
						SnapPickedItem (NumPicked);
						UpdateConnectFile (PickList[NumPicked].Refno,lpGWDHead,
												PickList[NumPicked].BeginPoint,SnapPointBase,
												PickList[NumPicked].BeginPointFile,SnapPointFile);
					}
				}
       			goto NextItem;
       		} 
       		if (!pRC->BPConnections)
       		{
    			SnapEnd = 3;
		       	DoNotPickThisRefno=Refno;  
		       	MaxPick=1;
				PickItems (CurView->hWnd,pRC->EPBase); 
				if (NumPicked)
				{  
					NearOpRef = PickList[0].Refno;
					NearOpDist = PickList[0].OffDist;
				}
				else
					NearOpRef = LONG_MAX; 
		       	DoNotPickThisRefno=LONG_MAX;
		       	MaxPick=3;
				PickItems (CurView->hWnd,pRC->BPBase); 
				if (NumPicked >1)
				{   
					NumPicked--;
					if (PickList[NumPicked].Refno == Refno)
					{
						FixItem=NumPicked;
						SnapItem=NumPicked-1;
					}
					else if (PickList[NumPicked-1].Refno == Refno) 
					{
						FixItem=NumPicked-1;
						SnapItem=NumPicked;
					}             
					else
						goto NextEnd;
					if (PickList[SnapItem].Refno == NearOpRef &&
						PickList[SnapItem].OffDist > NearOpDist)
					{
						if (NumPicked == 1)
							goto NextEnd; 
						SnapItem = 0;
					}
					if (((PickList[SnapItem].PCT * PickList[SnapItem].Length) < FixTol) &&
						((PickList[SnapItem].PCT * PickList[SnapItem].Length) <
						 (1.0-PickList[SnapItem].PCT) * PickList[SnapItem].Length))					
					{
						SnapPointFile = PickList[SnapItem].BeginPointFile;
						SnapPointBase = PickList[SnapItem].BeginPoint;
	    				SnapPickedItem (FixItem);
						UpdateConnectFile (PickList[FixItem].Refno,lpGWDHead,
												SnapPointBase,PickList[FixItem].EndPoint,
												SnapPointFile,PickList[FixItem].EndPointFile);
					}
					else if (((1.0-PickList[SnapItem].PCT) * PickList[SnapItem].Length) < FixTol) 
					{
						SnapPointFile = PickList[SnapItem].EndPointFile; 
						SnapPointBase = PickList[SnapItem].EndPoint; 
						SnapPickedItem (FixItem);
						UpdateConnectFile (PickList[FixItem].Refno,lpGWDHead,
												SnapPointBase,PickList[FixItem].EndPoint,
												SnapPointFile,PickList[FixItem].EndPointFile);
					}
					else
					{   
						SnapPointFile = PickList[SnapItem].PickedPointFile;
						SnapPointBase = PickList[SnapItem].PickedPoint;
						SnapPickedItem (FixItem);
						UpdateConnectFile (PickList[FixItem].Refno,lpGWDHead,
												SnapPointBase,PickList[FixItem].EndPoint,
												SnapPointFile,PickList[FixItem].EndPointFile); 
						GetPickName (SnapItem); 
						_fstrcpy (EditName,PickName);
						SplitPoly (SnapItem,&Newref1,&Newref2);
					}
				} 
       		}
NextEnd:
       		if (!pRC->EPConnections)
       		{
   				SnapEnd = 4;
		       	DoNotPickThisRefno=Refno;  
		       	MaxPick=1;
				PickItems (CurView->hWnd,pRC->BPBase); 
				if (NumPicked)
				{  
					NearOpRef = PickList[0].Refno;
					NearOpDist = PickList[0].OffDist;
				}
				else
					NearOpRef = LONG_MAX; 
		       	DoNotPickThisRefno=LONG_MAX;   
		       	MaxPick=3;
				PickItems (CurView->hWnd,pRC->EPBase); 
				if (NumPicked >1)
				{   
					NumPicked--;
					if (PickList[NumPicked].Refno == Refno)
					{
						FixItem=NumPicked;
						SnapItem=NumPicked-1;
					}
					else if (PickList[NumPicked-1].Refno == Refno) 
					{
						FixItem=NumPicked-1;
						SnapItem=NumPicked;
					}             
					else
						goto NextItem;
					if (PickList[SnapItem].Refno == NearOpRef &&
						PickList[SnapItem].OffDist > NearOpDist)
					{
						if (NumPicked == 1)
							goto NextItem; 
						SnapItem = 0;
					}
					if (((PickList[SnapItem].PCT * PickList[SnapItem].Length) < FixTol) &&
						((PickList[SnapItem].PCT * PickList[SnapItem].Length) <
						 (1.0-PickList[SnapItem].PCT) * PickList[SnapItem].Length))					
					{
						SnapPointFile = PickList[SnapItem].BeginPointFile;
						SnapPointBase = PickList[SnapItem].BeginPoint;
	    				SnapPickedItem (FixItem);
						UpdateConnectFile (PickList[FixItem].Refno,lpGWDHead,
												PickList[FixItem].BeginPoint,SnapPointBase,
												PickList[FixItem].BeginPointFile,SnapPointFile);
					}					
					else if (((1.0-PickList[SnapItem].PCT) * PickList[SnapItem].Length) < FixTol)
					{
						SnapPointFile = PickList[SnapItem].EndPointFile; 
						SnapPointBase = PickList[SnapItem].EndPoint;
    					SnapPickedItem (FixItem);
						UpdateConnectFile (PickList[FixItem].Refno,lpGWDHead,
												PickList[FixItem].BeginPoint,SnapPointBase,
												PickList[FixItem].BeginPointFile,SnapPointFile);
    				}
					else
					{   
						SnapPointFile = PickList[SnapItem].PickedPointFile;
						SnapPointBase = PickList[SnapItem].PickedPoint;
    					SnapPickedItem (FixItem);
						UpdateConnectFile (PickList[FixItem].Refno,lpGWDHead,
												PickList[FixItem].BeginPoint,SnapPointBase,
												PickList[FixItem].BeginPointFile,SnapPointFile);
						GetPickName (SnapItem); 
						_fstrcpy (EditName,PickName);
						SplitPoly (SnapItem,&Newref1,&Newref2);
					}
				} 
       		}
NextItem:;
       	}   
       	DoNotPickThisRefno=LONG_MAX;
		UseUserPickAp =TRUE;
		MaxPick = SaveMaxPick;
		GlobalUnlock (hDBRefConnect); 
		CloseRefConnectFile ();
	    GSSiSetCursor(hcurSave); 
        SetGlobalValueLong ("%NumRemoved",NumRemoved);
        RemoveGraphicsFunction (hWnd);  
	
		break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}  

BOOL DeletePickedItem (short Item,short OldType,short NewType)
{   
	HFILE	Fid; 
	OFSTRUCT	OFStruct;  
	long	Loc;
	struct	{BYTE	Type, Pen;} ID;   
	
	GetPickName (Item);
	Fid = GSSiOpenFile (PickName,(LPOFSTRUCT)&OFStruct,OF_READWRITE);
	if (Fid != HFILE_ERROR)
	{   
		Loc = PickList[Item].Segment + PickList[Item].Offset + 2;
		if (_llseek (Fid,Loc,0)==Loc)
		{
			if (_lread (Fid,&ID,2)==2)
			{
				if (ID.Type == OldType)
				{
					_llseek (Fid,Loc,0);
					ID.Type =NewType;
					_lwrite (Fid,&ID,2);
				}  
			}
		}
		_lclose (Fid);
	}
}

void UpdateConnectFile (long Refno,LPGWDHEADER lpGWDHead,DPOINT BP, DPOINT EP, POINT BPFile, POINT EPFile)
{ 
	LPREFCONNECT	pRC; 
	REFCONNECT		RCSave;
	short	len;  
	long	Offset;  

    pRC = &lpGWDHead->GWDData; 
    RCSave = *pRC;  
	if (BT_FIND (lpGWDHead->BTHandle[0],&Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset))
		return;
	len = FillGWDData (lpGWDHead,Offset);
    pRC->Length = ldistp (BP,EP);         
    if (pRC->Length == 0)
		GWDDeleteRecord (lpGWDHead,Offset);  
    else
    {
	    pRC->BPBase = BP;
	    pRC->EPBase = EP;
	    pRC->BPFile = BPFile;
	    pRC->EPFile = EPFile;		 
		GWDReplaceRecord (lpGWDHead,len,NULL,Offset);  
	}
	*pRC = RCSave;
	return;
}

short TraceLink (long Refno,long LinkRef,long StartRef,LPGWDHEADER lpGWDHead,short AtEnd,long Offset, HPDPOINT *pPoints,LPLPOINT pToNode,LPDOUBLE pNodeAZ)
{ 
	LPREFCONNECT	pRC; 
	REFCONNECT		RCSave; 
	POINT			StartPoint, ConnectPointFile;
	LPDPOINT		pOpPointBase;
	DPOINT			ConnectPointBase; 
	LPSHORT			pConnections;
	LPTHEME			pTheme;
	short	len, NumPoints=0, st,ii;  
	short 	WhichEndNext=3-AtEnd, End;
	long	Ref; 
	static	long	debugrefno=1062189981, debuglink=1508; 
	BOOL	First=TRUE;
	struct {
			POINT	Point; 
			long	Ref; 
			short	End;
			} ConnectKey;
	
    if (LinkRef == debuglink)
    	ii=1;
    pRC = &lpGWDHead->GWDData; 
Next: 
	Ref = pRC->Ref;
	End = WhichEndNext; 
	if (pRC->Ref == debugrefno)
		ii=1;
    RCSave = *pRC;
	if (WhichEndNext == 1)
	{
		StartPoint = pRC->BPFile;
		pOpPointBase = &pRC->BPBase;
		ConnectPointBase = pRC->BPBase; 
		pConnections = &RCSave.BPConnections;
	}
	else
	{
		StartPoint = pRC->EPFile;
		pOpPointBase = &pRC->EPBase;
		ConnectPointBase = pRC->EPBase; 
		pConnections = &RCSave.EPConnections;
	} 
	if (pRC->NumPoints > 2)
	{
		if (!PickByRefno(pRC->Ref,NULL,NULL,-1))
		{
	    	MessageBox (GetFocus(),"refindex needs to be recreated",NULL,MB_ICONEXCLAMATION);
	        PostMessage(hWndMain, WM_CLOSE, 0, 0L);  
	        exit(1);
	    }
		pTheme = AddTheme (GF_SAVEPOLY_THEME);
		CurView->PassID = 4;
		ProcessPickedItem (0,FALSE);        		
		DeleteTheme (pTheme);
   		if (hSavePoly)
		{   LPMNMXCORD lpRect;
			short	nPnts; 
			LPDPOINT	lpDpoint;
    		    
            nPnts = nSavePoly-2; 
            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
            lpRect++;
            lpDpoint = (LPDPOINT) lpRect;   
            if (WhichEndNext == 2)
            {
	            lpDpoint++;   
	            while (nPnts--)
	            {
					**pPoints = *lpDpoint++;
					(*pPoints)++; 
					NumPoints++;
	            }
	        }
	        else 
	        {   
	        	lpDpoint+=nPnts;
	            while (nPnts--)
	            {
					**pPoints = *lpDpoint--;
					(*pPoints)++; 
					NumPoints++;
	            }
	        }
            GlobalUnlock (hSavePoly);
            GlobalFree (hSavePoly);
         }
	}
	**pPoints = *pOpPointBase;
	(*pPoints)++; 
	NumPoints++;  
	if (StartRef == LONG_MAX || !First)
		GWDDeleteRecord (lpGWDHead,Offset);  
	if (*pConnections > 1)
	{
	    *pToNode = AddNode (1,WhichEndNext,ConnectPointBase,LinkRef,&RCSave,pNodeAZ); 
	    return NumPoints;
	} 
	ConnectKey.Point = StartPoint;
	ConnectKey.Ref = ConnectKey.End = 0;
	
	WhichEndNext = 2; 
    st = BT_FIND (lpGWDHead->BTHandle[1],&ConnectKey,BT_FIRST,BT_GT,(LPSTR)&Offset);
    if (!st && Ref == StartRef && ConnectKey.Ref == Ref && First)
	    st = BT_FIND (lpGWDHead->BTHandle[1],&ConnectKey,BT_NEXT,BT_ANY,(LPSTR)&Offset);
	if (st || ConnectKey.Point.x != StartPoint.x ||
        		ConnectKey.Point.y != StartPoint.y) 
    {
		WhichEndNext = 1;
		ConnectKey.Point = StartPoint;
		ConnectKey.Ref = ConnectKey.End = 0;
	    st = BT_FIND (lpGWDHead->BTHandle[2],&ConnectKey,BT_FIRST,BT_GT,(LPSTR)&Offset);
	    if (!st && Ref == StartRef && ConnectKey.Ref == Ref && First)
		    st = BT_FIND (lpGWDHead->BTHandle[2],&ConnectKey,BT_NEXT,BT_ANY,(LPSTR)&Offset);
		if (st || ConnectKey.Point.x != StartPoint.x ||
	        		ConnectKey.Point.y != StartPoint.y)
	    {   
	    	char	mess[128];
	    	sprintf (mess,"Dead end link %ld, NumPoints = %i, Refno = %ld, End = %i",LinkRef,NumPoints,pRC->Ref,End);
	    	MessageBox (GetFocus(),mess,NULL,MB_ICONEXCLAMATION);
   			DisplayMarker (*pOpPointBase,5,NULL,0,0,0,FALSE);
	    	return -1;
	    } 
	}
	First = FALSE;
	len = FillGWDData (lpGWDHead,Offset);
	if (pRC->Ref == StartRef)
	{
	    *pToNode = AddNode (1,3-WhichEndNext,ConnectPointBase,LinkRef,pRC,pNodeAZ); 
	    GWDDeleteRecord (lpGWDHead,Offset); 
	    return NumPoints;
	} 
	 
	goto Next;
}
	

BOOL RefConnectOutput (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{HDC hDC;
 int	st, BPConnect,EPConnect;
 LPGWDHEADER lpGWDHead;
 HIGHLIGHTDATA	HighlightData;
 LPREFCONNECT	pRC;
 short	pos, pos2, cond,len,ii, SnapItem, FixItem, SaveMaxPick;  
 long	Refno, Offset, NearOpRef, NumRemoved=0, TotLoops, MinRef, nlast,n;  
 HCURSOR	hcurSave;   
 POINT	Point;  
 DPOINT	SnapPointBase;
 LINKDATA	LinkData; 
 short	NumLoopPoints[256], NumLoops;
 HANDLE	hLoopPoints[256];  
LOOPREC	LoopRec;    
static	long	debuglinkref=184,debugarearef=2, debugloops=146, debuglink=185;
 switch (Message)
   {
   	case GF_INIT:
   		if (!CurView->UpdateFile)
   		{
	 		MessageBox(GetFocus(), "No update file in this viewport", NULL,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		} 
   		else
   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	
       	AddLBUTTON = FALSE;
		PostMessage(hWnd, GF_EXECUTE,0, 0L); 
   		break;
    
    case GF_EXECUTE:  
    {
    	short	Pass=0, NumConnect, NumLinkPoints,AtEnd, NewAreaSymbol, NewLinkPoints;
    	long	LinkRef=0, LastRef,ii,n=0,NumLinks,StartRef, MinLinkRef, LoopID=1;
    	POINT	ConnectPointFile,OpEndFile;
    	DPOINT	ConnectPointBase,OpEndBase;
    	HANDLE	hLinkPoints = GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX);
    	HPDPOINT	pLinkPoints;    
    	OFSTRUCT	OFStruct;  
   	    BTVARDESC  Vars;  
   	    BOOL	JustDebugRec=FALSE,NoIslands=FALSE;
   	    char	mess[128]; 
		long	AreaRef=1;   
		COLORREF	AreaColor;
		static	long	debugref=1061980080;

    	NewAreaSymbol = GetDictSymbolNumber ("SOILAREA");
		hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT));  
		AddNodeInit (&CurView->FileMNMX);
		OpenRefConnectionFile ("[%CONSTATFILE].gmd");
	    lpGWDHead = GlobalLock (hDBRefConnect);
	    pRC = &lpGWDHead->GWDData; 
	    LastRef = LONG_MAX;   
	    MinRef = LONG_MIN;
NextLink:
	    pos = BT_FIRST; 
	    cond = BT_GT;
	    NumLinks = 0; 
	    Refno = MinRef;
		while (!BT_FIND (lpGWDHead->BTHandle[0],&Refno,pos,cond,(LPSTR)&Offset)) 
		{    
			pos = BT_NEXT;  
			cond = BT_ANY;
			NumLinks++;
			if (Refno == LastRef)
				ii=1;    
			LastRef = Refno;
			if (Refno == debugref)
				ii=1;    
			n++;
			if (!(n%1000))
				ii=1;
       		len = FillGWDData (lpGWDHead,Offset); 
       		StartRef = LONG_MAX; 
       		switch (Pass)
       		{            
       			case 0:
       			NumConnect = pRC->BPConnections;
       			ConnectPointFile = pRC->BPFile;
       			ConnectPointBase = pRC->BPBase;
       			OpEndBase = pRC->EPBase;
       			OpEndFile = pRC->EPFile;
       			AtEnd = 1;
       			break; 
       			
       			case 1:
       			NumConnect = pRC->EPConnections;
       			ConnectPointFile = pRC->EPFile;
       			ConnectPointBase = pRC->EPBase;
       			OpEndBase = pRC->BPBase;
       			OpEndFile = pRC->BPFile;
       			AtEnd = 2; 
       			break; 
       			
       			case 2: 
       			StartRef = Refno;
       			NumConnect = 2;
       			ConnectPointFile = pRC->BPFile;
       			ConnectPointBase = pRC->BPBase;
       			OpEndBase = pRC->EPBase;
       			OpEndFile = pRC->EPFile;
       			AtEnd = 1;
       			break;
       		}
       		if (NumConnect > 1)
       		{   
       			LinkRef++;
       			if (LinkRef == debuglinkref)
       				ii=1;
       			LinkData.FromNode = AddNode (2,AtEnd,ConnectPointBase,LinkRef,pRC,&LinkData.FromAZ);  
       			NumLinkPoints = 1;
       			pLinkPoints = GlobalLock (hLinkPoints);
       			*pLinkPoints++=ConnectPointBase; 
       			NewLinkPoints = TraceLink (Refno,LinkRef,StartRef,lpGWDHead,AtEnd,Offset,&pLinkPoints,&LinkData.ToNode,&LinkData.ToAZ);
       			if (NewLinkPoints < 0)
       			{
	       			GlobalUnlock (hLinkPoints);
					GlobalUnlock (hDBRefConnect);
					CloseRefConnectFile (); 
					goto Exit;
       			}
       			NumLinkPoints += NewLinkPoints;
       			GlobalUnlock (hLinkPoints);
       			pLinkPoints = GlobalLock (hLinkPoints);  
       			LinkData.Offset = _llseek (FidLinks,0,2); 
       			LinkData.DeltaAZ = GetLinkDeltaAZ (NumLinkPoints,pLinkPoints);
       			LinkData.TrackedLeft=LinkData.TrackedRight=0;
       			BT_PUT (hBTLinks,(LPSTR)&LinkRef,(LPSTR)&LinkData);
       			_lwrite (FidLinks,&LinkRef,4);
       			_lwrite (FidLinks,&NumLinkPoints,2);
				BigWrite (FidLinks,pLinkPoints,(long)NumLinkPoints * sizeof(DPOINT));       			
       			GlobalUnlock (hLinkPoints);
	       		goto NextLink;
       		}
       		else
       			MinRef = Refno;
       	}                 
       	if (!Pass)
       	{
       		Pass = 1;   
       		MinRef = LONG_MIN;
       		goto NextLink;
       	} 
       	else if (Pass == 1)
       	{
       		Pass = 2;  
       		MinRef = LONG_MIN;
       		goto NextLink;
       	}
       	else if (NumLinks)
       		ii=1;
		GlobalUnlock (hDBRefConnect);
		CloseRefConnectFile ();
	    
	    {
	    	short pos, numlinks; 
	    	long	TotNodes,BadNodes;
	    	LPOINT	LastNode;
		    
		    pos = BT_FIRST; 
		    LastNode.x = LastNode.y = LONG_MAX;
		    numlinks = 2; 
		    TotNodes=BadNodes=0;
		    while (!BT_FIND (hNodes,(LPSTR)&NodesKey,pos,BT_ANY,(LPSTR)&NodesData))
		    {
		    	pos = BT_NEXT; 
		    	if (NodesKey.NodeID.x != LastNode.x || NodesKey.NodeID.y != LastNode.y)
		    	{
		    		if (numlinks < 2) 
		    		{
		    			BadNodes++; 
		    			DisplayMarker (NodesData.Point,4,NULL,0,0,0,FALSE);
		    		}
		    		numlinks = 1;
		    		TotNodes++;
		    		LastNode = NodesKey.NodeID;
		    	} 
		    	else
		    		numlinks++;
		    }
		    ii=1;  
		}

	    if (GetGlobalBVal ("[%CREATELINKMAP]")) 
	    {   
	    
	    	DumpLinkData ("linkdata.gmd");

		    CreateNewMap ("testlink.plt",&CurView->WBounds,0,NULL,0,NULL);
			EditBounds = CurView->FileMNMX;
		    _fstrcpy (PltName,"testlink.plt");   
		    _llseek (FidLinks,128,0);
		    while (_lread (FidLinks,&LinkRef,4)==4)
		    {
		    	short NewLineSymbol=3, rtn;
		    	long	NewRefno=1;
		        
				_lread (FidLinks,&NumLinkPoints,2);
	   			pLinkPoints = GlobalLock (hLinkPoints);  
				BigRead (FidLinks,pLinkPoints,(long)NumLinkPoints * sizeof(DPOINT)); 
				GlobalUnlock (hLinkPoints);      			
				rtn=AddPolyToMap (1,&NumLinkPoints, &hLinkPoints,1,LinkRef,2,NewLineSymbol,NULL,NULL,NULL,-1,-1,-1);
		    }
		    CloseMap();  
		}

	    TotLoops=0;
	    MinLinkRef = LinkRef = LONG_MIN;
	       
	    
		while (!BT_FIND (hBTLinks,(LPSTR)&LinkRef,BT_FIRST,BT_GT,(LPSTR)&LinkData))
		{   
			short	Track, AtEnd;			   
			double	TotAZ, StartAZ, AzChange, InAZ;  
			long	AtLink; 
			HANDLE	hBT; 
			
			LPOINT	StartNode, AtNode;
				                                   
			if (!LinkData.TrackedLeft) 
			{
				LinkData.TrackedLeft = 1;
				Track = TRACK_LEFT;  
			}
			else if (!LinkData.TrackedRight) 
			{
				LinkData.TrackedRight = 1;
				Track = TRACK_RIGHT;  
			}
			else
			{
				MinLinkRef = LinkRef;
				goto FindNextLink;
			}  
			TotLoops++;
			sprintf (mess,"%ld loops",TotLoops);  
			if (TotLoops == debugloops)
				ii=1;
			SetWindowText (hWndMain,mess);
			StartNode = LinkData.FromNode;
			StartAZ = LinkData.FromAZ; 
			AtNode = LinkData.ToNode;
			AtLink = LinkRef;
			AtEnd = 2;
			LoopData.Offset = _llseek (FidLoop,0,2);
			LoopRec.LinkRef = LinkRef;
			LoopRec.Dir = 1;
			LoopData.NumLinks = 1;
			_lwrite (FidLoop,&LoopRec,sizeof(LoopRec)); 
			TotAZ = LinkData.DeltaAZ; 
			BT_PUT (hBTLinks,(LPSTR)&LinkRef,(LPSTR)&LinkData);
			while (AtNode.x != StartNode.x || AtNode.y != StartNode.y)
			{   
				if (AtLink == debuglink)
					ii=1;
				if (!GetNextLink (&AtNode,&AtLink,&AtEnd,&TotAZ,Track,&InAZ))
				{
			    	sprintf (mess,"Problem at link %ld end %i",AtLink,AtEnd);
			    	MessageBox (GetFocus(),mess,NULL,MB_ICONEXCLAMATION);
					goto FindNextLink;
				}
				LoopRec.LinkRef = AtLink;  
				LoopRec.Dir = 3-AtEnd;  
				LoopData.NumLinks++;
				_lwrite (FidLoop,&LoopRec,sizeof(LoopRec));
			}
			AzChange = DeltaAZ (InAZ,StartAZ); 
			TotAZ += AzChange;
			if (Track == TRACK_LEFT && TotAZ > 0)
			{
				LoopData.Reverse = TRUE;
				hBT = hBTInclusion;  
			}
			else if (Track == TRACK_RIGHT && TotAZ < 0) 
			{
				LoopData.Reverse = FALSE;
				hBT = hBTInclusion;  
			}
			else if (Track == TRACK_LEFT && TotAZ < 0)
			{
				LoopData.Reverse = TRUE;
				hBT = hBTExclusion;  
			}
			else if (Track == TRACK_RIGHT && TotAZ > 0) 
			{
				LoopData.Reverse = FALSE;
				hBT = hBTExclusion;  
			}
            CalculateLoopSize (FidLoop,LoopData.Offset,LoopData.NumLinks,LoopData.Reverse,
            					   &LoopData.MinMax,&LoopKey.Size,
            					   &LoopData.NumSides,&LoopData.Point1);
            LoopKey.LoopID = LoopID++;
			BT_PUT (hBT,(LPSTR)&LoopKey,(LPSTR)&LoopData);
FindNextLink:
			LinkRef = MinLinkRef;
		} 
		
		
	    CreateNewMap (EditName,&CurView->WBounds,0,NULL,0,NULL);
		EditBounds = CurView->FileMNMX;
	    _fstrcpy (PltName,EditName);   
		while (!BT_FIND (hBTInclusion,(LPSTR)&LoopKey,BT_FIRST,BT_ANY,(LPSTR)&LoopData))
		{   
			double	MaxExclusionSize = LoopKey.Size * 0.999999; 
			short	pos2;
			
			AreaRef = LoopKey.LoopID;
			if (AreaRef == debugarearef)
				ii=1;   
			sprintf (mess,"Storing area %ld",AreaRef);
			SetWindowText (hWndMain,mess);
			NumLoopPoints[0]=LoopData.NumSides;
			hLoopPoints[0] = LoadLoopSides (&LoopData.NumSides,LoopData.NumLinks,LoopData.Offset,LoopData.Reverse);
			NumLoops = 1; 
			BT_DELETE (hBTInclusion,(LPSTR)&LoopKey,(LPSTR)&LoopData,FALSE);
			if (NoIslands)
				MaxExclusionSize = 0;
			pos2 = BT_FIRST;
			while (!BT_FIND (hBTExclusion,(LPSTR)&LoopKey,pos2,BT_ANY,(LPSTR)&LoopData))
			{   
				pos2 = BT_NEXT;
				if (LoopKey.Size >= MaxExclusionSize)
					goto NoMoreExclusions; 
				if (PointInLoop (LoopData.Point1,NumLoopPoints[0]-1,hLoopPoints[0]))
				{
					NumLoopPoints[NumLoops]=LoopData.NumSides;
					hLoopPoints[NumLoops++] = LoadLoopSides (&LoopData.NumSides,LoopData.NumLinks,LoopData.Offset,LoopData.Reverse);
					BT_DELETE (hBTExclusion,(LPSTR)&LoopKey,(LPSTR)&LoopData,FALSE);
				}
			}
NoMoreExclusions:
            AreaColor = RGB(127+64+IDNINT(((double)rand()/RAND_MAX)*32),
                            127+64+IDNINT(((double)rand()/RAND_MAX)*32),
                            127+64+IDNINT(((double)rand()/RAND_MAX)*32)); 
            if (!GetGlobalBVal ("[%REFISLOOPID]"))
				AreaRef = GetNewRefno(PltName);
			if (!JustDebugRec || AreaRef == debugarearef)
				AddPolyToMap (NumLoops,&NumLoopPoints,&hLoopPoints,0,AreaRef,2,NewAreaSymbol,NULL,NULL,NULL,AreaColor,-1,-1);
			while (NumLoops--)
				GlobalFree (hLoopPoints[NumLoops]);
		} 
		
		{   
			HANDLE	hSymDesc=0;
			short	NumSyms=0;
				
			AddToSymList (NewAreaSymbol,&NumSyms,&hSymDesc); 
			AddSymToMap (NumSyms,hSymDesc,0,NULL); 
            DestroySymList (&NumSyms,&hSymDesc);
		}
		nlast = BT_NUM_IN_INDEX (hBTExclusion); 
		if (nlast > 1)
		{   
			char	mess[128];
			
			sprintf (mess,"There are %ld unattached exclusions\r\nShould these be added to the map?",nlast-1);
	        if (MessageBox( GetFocus(),mess,NULL,MB_YESNO|MB_ICONQUESTION)==IDYES) 
			{
		    	NewAreaSymbol = GetDictSymbolNumber ("CENTRACT");
				AreaRef=1000; 
				pos = BT_FIRST;
				n=1;
				while (!BT_FIND (hBTExclusion,(LPSTR)&LoopKey,pos,BT_ANY,(LPSTR)&LoopData))
				{   
					pos = BT_NEXT;
		            AreaColor = 0;
					NumLoops=0; 
					hLoopPoints[NumLoops++] = LoadLoopSides (&LoopData.NumSides,LoopData.NumLinks,LoopData.Offset,ReverseBOOL(LoopData.Reverse));
					NumLoopPoints[0]=LoopData.NumSides;   
					if (n++ < nlast)
						AddPolyToMap (NumLoops,&NumLoopPoints,&hLoopPoints,0,LoopKey.LoopID,2,NewAreaSymbol,NULL,NULL,NULL,AreaColor,-1,-1);
					while (NumLoops--)
						GlobalFree (hLoopPoints[NumLoops]);
				}
				{   
					HANDLE	hSymDesc=0;
					short	NumSyms=0;
						
					AddToSymList (NewAreaSymbol,&NumSyms,&hSymDesc); 
					AddSymToMap (NumSyms,hSymDesc,0,NULL); 
		            DestroySymList (&NumSyms,&hSymDesc);
				}
			}
		}
Exit:	
	    GlobalFree (hLinkPoints);
		CloseNodes ();
	    GSSiSetCursor(hcurSave);
        RemoveGraphicsFunction (hWnd);  
	}
		break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}  

BOOL GetNextLink (LPLPOINT AtNode,LPLONG AtLink,LPSHORT AtEnd,LPDOUBLE TotAZ,short Track,LPDOUBLE pAtAZ)
{   
	NODESDATA	LastLink, NextLink, FirstLink, PriorLink, GotLink; 
	double		LastAZ, NextAZ, FirstAZ, AtAZ, GotAZ, PriorAZ, AzChange, InAZ, OutAZ;
	short		PastAt = 0, pos, cond,ii;
	BOOL		First=TRUE;
	LINKDATA	LinkData; 
	
	LastLink.WhichEnd = PriorLink.WhichEnd = NextLink.WhichEnd = FirstLink.WhichEnd =-1;	
	NodesKey.AZ = -10; 
	NodesKey.NodeID = *AtNode;   
	pos = BT_FIRST;
	cond = BT_GT; 
	
	while (!BT_FIND (hNodes,(LPSTR)&NodesKey,pos,cond,(LPSTR)&NodesData))
	{   
		if (NodesKey.NodeID.x != AtNode->x || NodesKey.NodeID.y != AtNode->y)
		{
			if (Track == TRACK_LEFT) 
			{
				GotLink = LastLink;
				GotAZ = LastAZ;
			}
			else
			{
				GotLink = FirstLink;
				GotAZ = FirstAZ;
			}
			goto Exit;
		}
		if (NodesData.LinkID == *AtLink && NodesData.WhichEnd == *AtEnd) 
		{   
			AtAZ = NodesKey.AZ; 
			InAZ = LTWOPI (AtAZ + PY);
			PastAt = 1;
			if (Track == TRACK_LEFT && PriorLink.WhichEnd > 0)
			{
				GotLink = PriorLink;
				GotAZ = PriorAZ;
				goto Exit;	
			}
		}
		else if (PastAt==1)
		{   
			if (Track == TRACK_RIGHT)
			{
				GotLink = NodesData;
				GotAZ = NodesKey.AZ;
				goto Exit;	
			}
			NextAZ = LastAZ = NodesKey.AZ;
			NextLink = LastLink = NodesData;
			PastAt++; 
		}
		else if (PastAt) 
		{
			LastAZ = NodesKey.AZ;
			LastLink = NodesData; 
		}
		else
		{ 
			PriorAZ = NodesKey.AZ;
			PriorLink = NodesData;
		} 
		if (First)
		{
			pos = BT_NEXT;
			cond = BT_ANY;
			First = FALSE;
			if (!PastAt)
			{	
				FirstAZ = NodesKey.AZ;
				FirstLink = NodesData;
			}
		}
		
	} 
	if (Track == TRACK_LEFT) 
	{
		GotLink = LastLink;
		GotAZ = LastAZ;
	}
	else
	{
		GotLink = FirstLink;
		GotAZ = FirstAZ;
	}
Exit:   
	if (!PastAt)
		return FALSE;   
		
	DisplayMarker (GotLink.Point,3,NULL,0,0,0,FALSE);			
	*AtLink = GotLink.LinkID;
	AzChange = DeltaAZ (InAZ,GotAZ); 
	*TotAZ += AzChange;
	BT_FIND (hBTLinks,(LPSTR)AtLink,BT_FIRST,BT_EQ,(LPSTR)&LinkData);
	if (GotLink.WhichEnd == 1)
	{
		if (Track == TRACK_LEFT)
			LinkData.TrackedLeft = 1;
		else
			LinkData.TrackedRight = 1;
		*AtNode = LinkData.ToNode;
		*AtEnd = 2;  
		AzChange = LinkData.DeltaAZ;
		*pAtAZ = LTWOPI (LinkData.ToAZ + PY);
	}
	else
	{
		if (Track == TRACK_LEFT)
			LinkData.TrackedRight = 1;
		else
			LinkData.TrackedLeft = 1;
		*AtNode = LinkData.FromNode;
		*AtEnd = 1;  
		AzChange = -LinkData.DeltaAZ; 
		*pAtAZ = LTWOPI (LinkData.FromAZ + PY);
	}
	*TotAZ += AzChange;
	BT_PUT (hBTLinks,(LPSTR)AtLink,(LPSTR)&LinkData);
	
	return TRUE;
}

LPOINT AddNode (short InOut,short AtEnd,DPOINT ConnectPointBase,long LinkRef,LPREFCONNECT pRC,LPDOUBLE pNodeAZ)
{   
	static	long	debuglinkref=2212;
	short	ii;
	
	if (LinkRef == debuglinkref)
		ii=1;   
	NodesKey.NodeID.x = IDNINT ((ConnectPointBase.x - AddNodeBias.x) * AddNodeFactor);
	NodesKey.NodeID.y = IDNINT ((ConnectPointBase.y - AddNodeBias.y) * AddNodeFactor);
	if (pRC->NumPoints > 2)
	{   
		LPTHEME	pTheme;
		
		PickByRefno(pRC->Ref,NULL,NULL,-1);  
		pTheme = AddTheme (GF_SAVEPOLY_THEME);
		CurView->PassID = 4;
		ProcessPickedItem (0,FALSE);        		
		DeleteTheme (pTheme);
   		if (hSavePoly)
		{   LPMNMXCORD lpRect;
			short	nPnts; 
			LPDPOINT	lpDpoint;
			DPOINT	Point1, Point2;
    		    
            nPnts = nSavePoly; 
            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
            lpRect++;
            lpDpoint = (LPDPOINT) lpRect; 
            if (AtEnd == 1)
            {
            	Point1 = *lpDpoint++; 
            	Point2 = *lpDpoint; 
            }
            else 
            {
            	lpDpoint += (nPnts-2);
            	Point2 = *lpDpoint++; 
            	Point1 = *lpDpoint;
            }
           	NodesKey.AZ = getazd (Point1,Point2);
            GlobalUnlock (hSavePoly);
            GlobalFree (hSavePoly);
         }
	}
	else if (AtEnd == 2)
		NodesKey.AZ = getazd (pRC->EPBase,pRC->BPBase);
	else
		NodesKey.AZ = getazd (pRC->BPBase,pRC->EPBase); 
	*pNodeAZ = NodesKey.AZ; 
	NodesData.LinkID = LinkRef;
	NodesData.Point = ConnectPointBase;
	NodesData.WhichEnd = 3 - InOut;
	BT_PUT (hNodes,(LPSTR)&NodesKey,(LPSTR)&NodesData);
	return NodesKey.NodeID;
}    

void CloseNodes (void)
{   
	char	File[128];
	
	BT_CLOSEANDDELETE (&hNodes);
	_llseek (FidLoop,0,0);
	_lread (FidLoop,File,128);
	_lclose (FidLoop);
	GSSiRemove (File); 
	_llseek (FidLinks,0,0);
	_lread (FidLinks,File,128);
	_lclose (FidLinks);
	GSSiRemove (File); 
	BT_CLOSEANDDELETE (&hBTLinks);
	BT_CLOSEANDDELETE (&hBTInclusion);
	BT_CLOSEANDDELETE (&hBTExclusion);
	return;
}

BOOL AddNodeInit (LPMNMXCORD Rect)
{   
	double	MaxDist;
	short	ld;
	BTVARDESC	BTVar[3];
	char	File[128];  
	OFSTRUCT	OFStruct;

	MaxDist = max (Rect->xmx - Rect->xmn,Rect->ymx - Rect->ymn); 
	AddNodeBias.x =  (Rect->xmx + Rect->xmn)/2;
	AddNodeBias.y =  (Rect->ymx + Rect->ymn)/2; 
	ld = log10 (MaxDist);
	
	AddNodeFactor = pow (10,8 - ld);
	GetTempFileName (NULL,"nod",NULL,(LPSTR)File);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BTVar[2].BT_VARTYP=BT_REAL;
	BTVar[2].BT_VARLEN=8;
	BTVar[2].BT_VAROFF=8;
	BT_CREATE (File,sizeof(NodesData), FALSE, 3, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
	hNodes= BT_OPEN (File, 0, BT_WRITE, 0); 
	
	GetTempFileName (NULL,"lop",NULL,(LPSTR)File); 
	FidLoop = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	_lwrite (FidLoop,File,128);
	GetTempFileName (NULL,"lkd",NULL,(LPSTR)File); 
	FidLinks = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	_lwrite (FidLinks,File,128);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	GetTempFileName (NULL,"lki",NULL,(LPSTR)File); 
	BT_CREATE (File, sizeof(LINKDATA), FALSE, 1, 1,&BTVar,FALSE, 0, 0, FALSE); 
	hBTLinks = BT_OPEN (File,0, BT_WRITE, 0);
    
	GetTempFileName (NULL,"bti",NULL,(LPSTR)File); 
	BTVar[0].BT_VARTYP=BT_REAL;
	BTVar[0].BT_VARLEN=8;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=8;
	BT_CREATE (File,sizeof(LOOPDATA), FALSE, 2, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
	hBTInclusion = BT_OPEN (File, 0, BT_WRITE, 0); 
    
	GetTempFileName (NULL,"bte",NULL,(LPSTR)File); 
	BT_CREATE (File,sizeof(LOOPDATA), FALSE, 2, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
	hBTExclusion = BT_OPEN (File, 0, BT_WRITE, 0); 
    
	return TRUE;
}  

HANDLE LoadLoopSides (LPSHORT pNumSides,short NumLinks,long Offset,BOOL Reverse)
{   
	HANDLE	hLoop, hLoop2;
	HPDPOINT	pPoints, pPoints2;
	LOOPREC	LoopRec; 
	LINKDATA	LinkData;
	short		np, np2;
	long	MemLen=(long)*pNumSides*(long)sizeof(DPOINT);
	
	if (*pNumSides < 0)
	{   
		MemLen = (long)sizeof(DPOINT)*(long)32767;
		hLoop=GSSiGlobAlloc (GMEM_MOVEABLE,MemLen);
	}
	else
		hLoop=GSSiGlobAlloc (GMEM_MOVEABLE,MemLen);
	if (Reverse)
	{
		hLoop2=GSSiGlobAlloc (GMEM_MOVEABLE,MemLen);
		pPoints = GlobalLock (hLoop2);
	}
	else
		pPoints = GlobalLock (hLoop);
	*pNumSides = 0;	
	_llseek (FidLoop,Offset,0); 
	while (NumLinks--)
	{
		_lread (FidLoop,&LoopRec,sizeof(LoopRec));
		BT_FIND (hBTLinks,(LPSTR)&LoopRec.LinkRef,BT_FIRST,BT_EQ,(LPSTR)&LinkData); 
		_llseek (FidLinks,LinkData.Offset+4,0);
		_lread (FidLinks,&np,2); 
		(*pNumSides)+=(np-1); 
		if (LoopRec.Dir == 1)
			BigRead (FidLinks,pPoints,(long)np*sizeof(DPOINT)); 
		else
		{   
			HANDLE	Handle = GSSiGlobAlloc (GMEM_MOVEABLE,(long)np*sizeof(DPOINT)); 
			HPDPOINT	pPointsSave = pPoints;
			
			pPoints2 = GlobalLock (Handle);	
			BigRead (FidLinks,pPoints2,(long)np*sizeof(DPOINT)); 
			np2 = np;
			pPoints2+=(np-1);
			while (np2--)
				*pPoints++ = *pPoints2--;
			GlobalUnlock (Handle);
			GlobalFree (Handle); 
			pPoints = pPointsSave;
		}
		pPoints += (np-1);
	}
	(*pNumSides)++;
	if (Reverse)
    {   
    	pPoints2 = GlobalLock (hLoop); 
    	np = *pNumSides;
    	while (np--)
    		*pPoints2++=*pPoints--;
    	GlobalUnlock (hLoop2);
    	GlobalFree (hLoop2);
    }
    GlobalUnlock (hLoop);
	return hLoop;
}

BOOL PointInLoop (DPOINT Point,short NumLoopPoints,HANDLE hLoopPoints)
{
	BOOL	rtn;  
	HPDPOINT	pLoopPoints;
	
	pLoopPoints = GlobalLock (hLoopPoints);
	rtn = POINT_IN_AREAD (Point, NumLoopPoints, pLoopPoints);
	GlobalUnlock (hLoopPoints);
	return rtn;
}

void CalculateLoopSize (HFILE FidLoop,long Offset,short NumLinks,BOOL Reverse,
            			LPMNMXCORD pMinMax, LPDOUBLE pSize,LPSHORT pNumSides, LPDPOINT pPoint1)
{   
	LOOPREC	LoopRec;
	BOOL	First=TRUE; 
	HPDPOINT	lpDPoints;
	HANDLE	Handle; 
	double	Perim; 
	short	np;
	
	*pNumSides = -1;
	pMinMax->xmn = DBL_MAX;
    pMinMax->xmx = -DBL_MAX; 
    pMinMax->ymn = DBL_MAX;
    pMinMax->ymx = -DBL_MAX; 
	Handle = LoadLoopSides (pNumSides,NumLinks,Offset,Reverse); 
	lpDPoints = GlobalLock (Handle);
	*pPoint1 = *lpDPoints;
	*pSize = fabs (ComputeAreaAreaD (lpDPoints,*pNumSides,&Perim));
	np = *pNumSides;
	while (np--)
		AddToMinMaxD (pMinMax, lpDPoints++);
	GlobalUnlock (Handle);
	GlobalFree (Handle);
	return;
}  

double GetLinkDeltaAZ (short NumLinkPoints,HPDPOINT pLinkPoints)
{
	double AZ1,AZ2,LinkDeltaAZ=0;
	DPOINT	Point1, Point2;
	
	Point1 = *pLinkPoints++; 
	AZ1 = getazd (Point1,*pLinkPoints);  
	Point1 = *pLinkPoints++;
	NumLinkPoints-=2;
	while (NumLinkPoints--)
	{
		Point2 = *pLinkPoints++;
		AZ2 = getazd (Point1,Point2);
		LinkDeltaAZ += DeltaAZ (AZ1,AZ2);
		Point1 = Point2;
		AZ1 = AZ2;
	} 
	return LinkDeltaAZ;
} 

double DeltaAZ (double AZ1, double AZ2)
{         
	double daz;
	
	daz =  AZ2 - AZ1;
	if (daz < -PY)
		daz += TWOPI;
	else if (daz > PY)
		daz -= TWOPI;
	return daz;
}   

short LoadSoilData (short idum)
{
	short		pos=BT_FIRST;
    BTVARDESC  *pVars;  
    short       NumFields, Reclen, len;
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hVars, hDB;
    short       FidData,ibeg,NumVars;
    OFSTRUCT    OFStruct;
    GWFLDINFO FldInfo; 
    char	Name[128];   
    char	PrimeIndex[128];
    LPSTR	lpDot, lpEnd;
    HFILE	FidSP;
    typedef	struct	{long	IntRef;
    		 		 char	SoilID[16];
    		 		}	SOILPNTS;
    typedef SOILPNTS	FAR	*LPSOILPNTS;
    LPSOILPNTS	pSP;
    char	str[128];		
   
    _fstrcpy (Name,"soilpnts.gmd");
    ExpandText (Name); 
    _fstrcpy (PrimeIndex,Name);
    lpDot = _fstrrchr (PrimeIndex,'.');
    *lpDot = 0;
    _fstrcat (lpDot,".in1");
    lpGWDHead = &GWDHead; 
    
    FidData = GSSiOpenFile (Name,&OFStruct,OF_CREATE);   
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER));
    GWDHead.NumIndex=1;
    GWDHead.Version=1;
    GWDHead.NumIndexFields[0]=1;
    GWDHead.IndexFields[0][0]=0;
    _lwrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER));
    ibeg = 0;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"INT_REFNO");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 16;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"SoilType");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

	GWDHead.Reclen=ibeg; 
	GWDHead.TimeStamp = time(0);
	_llseek (FidData,0,0);
	_lwrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER));
	_llseek (FidData,0,2);
	                 
	NumVars = 1;
	            
	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
	pVars =(LPBTVARDESC) LocalLock(hVars);
	            
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (PrimeIndex, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
	LocalUnlock(hVars);
	LocalFree(hVars); 
	_lclose (FidData);
	
	hDB = OpenGWDatabase (Name,BT_WRITE);
    lpGWDHead = GlobalLock (hDB);   
	pSP = &lpGWDHead->GWDData;
	FidSP = GSSiOpenFile ("d:\\soilproj\\soilpnts.txt",&OFStruct,OF_READ);
	while (fgetstring (str,64,FidSP)) 
	{   
		str[10]=0;
		pSP->IntRef = atol (str);
		_fstrncpy (pSP->SoilID,&str[11],16);
	    GWDAddRecord (lpGWDHead,0,NULL);
	}
	_lclose (FidSP);    
	GlobalUnlock (hDB);
	CloseGWDatabase (hDB); 
	
	return TRUE;
} 

BOOL IdentifyPolygons (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{HDC hDC;
 int	st,SaveMaxPick;
 LPGWDHEADER lpGWDHead;
 HIGHLIGHTDATA	HighlightData;         
 short	pos, len;  
 long	Refno, Offset;  
 HCURSOR	hcurSave;
 POINT	Point; 
 char	mess[128], PointID[128]; 
 LPPOLYID	pPI;

 switch (Message)
   {
   	case GF_INIT:
		PostMessage(hWnd, GF_EXECUTE,0, 0L); 
   		break;
    
    case GF_EXECUTE:
		hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
		GSSiRemove ("[%POLYIDFILE].gmd");
		OpenPolyIDFile ("[%POLYIDFILE].gmd");
	    lpGWDHead = GlobalLock (hDBPolyID);   
	    pPI = &lpGWDHead->GWDData; 
		UseUserPickAp =FALSE;
		SystemPickAp = 1;	
		SaveMaxPick=MaxPick;
		MaxPick=1;
		SetPickAp();
	    pos = BT_FIRST;
		while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
		{   
			pos = BT_NEXT; 
			PickList[0] = HighlightData.PD;
			ProcessPickedItem (0,FALSE);
			_fstrcpy (PointID,"[%POLYID]");
			ExpandText (PointID);  
			PickItems2 (CurView->hWnd,HighlightData.PD.BeginPoint);
			if (NumPicked)
			{ 
	        	if (!BT_FIND (lpGWDHead->BTHandle[0],&PickList[0].Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset))
		        {    
	        		len = FillGWDData (lpGWDHead,Offset);
	        		if (_fstrcmp (pPI->ID,PointID))
	        		{   
	        			pPI->MultipleID=1;
						GWDReplaceRecord (lpGWDHead,len,NULL,Offset); 
					}
				}
				else
				{   
					pPI->Refno = PickList[0].Refno;
					pPI->MultipleID=0;
					_fstrncpy (pPI->ID,PointID,sizeof(pPI->ID));
			    	GWDAddRecord (lpGWDHead,0,NULL);   
			    }
			} 
NextRec:;
		} 

		UseUserPickAp =TRUE;
		MaxPick=SaveMaxPick;
		GlobalUnlock (hDBRefConnect);  
		
		ClearHighlightList ();
		ClosePolyIDFile ();
	    GSSiSetCursor(hcurSave); 
        RemoveGraphicsFunction (hWnd);
	
		break;
		
    default:
    	return (FALSE);
    }
    return (TRUE);
}   


BOOL IdentifyPolygonsTemp (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{HDC hDC;
 int	st,SaveMaxPick;
 LPGWDHEADER lpGWDHead;
 HIGHLIGHTDATA	HighlightData;         
 short	pos, len;  
 long	Refno, Offset;  
 HCURSOR	hcurSave;
 POINT	Point; 
 char	mess[128], PointID[128]; 
 LPPOLYID	pPI;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		break;
    
    case WM_LBUTTONUP:
		hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
		OpenPolyIDFile ("[%POLYIDFILE].gmd");
	    lpGWDHead = GlobalLock (hDBPolyID);   
	    pPI = &lpGWDHead->GWDData; 
		UseUserPickAp =FALSE;
		SystemPickAp = 1;	
		SaveMaxPick=MaxPick;
		MaxPick=1;
		SetPickAp();
	    pos = BT_FIRST;
		while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
		{   
			pos = BT_NEXT; 
			PickList[0] = HighlightData.PD;
			ProcessPickedItem (0,FALSE);
			_fstrcpy (PointID,"[SOILTYPE]");
			ExpandText (PointID);  
			PickItems2 (CurView->hWnd,HighlightData.PD.BeginPoint);
			if (NumPicked)
			{ 
	        	if (!BT_FIND (lpGWDHead->BTHandle[0],&PickList[0].Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset))
		        {    
	        		len = FillGWDData (lpGWDHead,Offset);
	        		if (_fstrcmp (pPI->ID,PointID))
	        		{   
	        			pPI->MultipleID=1;
						GWDReplaceRecord (lpGWDHead,len,NULL,Offset); 
					}
				}
				else
				{   
					pPI->Refno = PickList[0].Refno;
					pPI->MultipleID=0;
					_fstrncpy (pPI->ID,PointID,sizeof(pPI->ID));
			    	GWDAddRecord (lpGWDHead,0,NULL);   
			    }
			} 
NextRec:;
		} 

		UseUserPickAp =TRUE;
		MaxPick=SaveMaxPick;
		GlobalUnlock (hDBRefConnect);  
		
		ClearHighlightList ();
		ClosePolyIDFile ();
	    GSSiSetCursor(hcurSave); 
        RemoveGraphicsFunction (hWnd);
	
		break;
		
    default:
    	return (FALSE);
    }
    return (TRUE);
}   

void ClosePolyIDFile (void)
{
	if (hDBPolyID)
    	CloseGWDatabase (hDBPolyID);  
    hDBPolyID=0;
    return;
}
BOOL OpenPolyIDFile (LPSTR Name)
{   
	HANDLE	hDB; 
	OFSTRUCT	OFStruct;
	
	CreatePolyIDFile (Name);
	hDBPolyID = OpenGWDatabase (Name,BT_WRITE);
	return TRUE;
}

BOOL CreatePolyIDFile (LPSTR InName)
{
	short		pos=BT_FIRST;
    BTVARDESC  *pVars;  
    short       NumFields, Reclen, len;
    GWDHEADER GWDHead; 
    LPGWDHEADER lpGWDHead;
    HANDLE  hVars, hDB;
    short       FidData,ibeg,NumVars;
    OFSTRUCT    OFStruct;
    GWFLDINFO FldInfo; 
    char	Name[128];   
    char	PrimeIndex[128];
    LPSTR	lpDot, lpEnd;
    POLYID	PolyID;
   
    _fstrcpy (Name,InName);
    ExpandText (Name); 
    _fstrcpy (PrimeIndex,Name);
    lpDot = _fstrrchr (PrimeIndex,'.');
    *lpDot = 0;
    _fstrcat (lpDot,".in1");
    lpGWDHead = &GWDHead; 
    
    FidData = GSSiOpenFile (Name,&OFStruct,OF_CREATE);   
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER));
    GWDHead.NumIndex=1;
    GWDHead.Version=1;
    GWDHead.NumIndexFields[0]=1;
    GWDHead.IndexFields[0][0]=0;
    _lwrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER));
    ibeg = 0;

    FldInfo.Len = 4;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"INT_REFNO");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = sizeof(PolyID.ID);
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_CHAR;
    _fstrcpy (FldInfo.Name,"ID");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;

    FldInfo.Len = 2;
    FldInfo.Beg = ibeg;
    ibeg += FldInfo.Len;
    FldInfo.Type = BT_INTEGER;
    _fstrcpy (FldInfo.Name,"MultipleIDs");
    _lwrite (FidData,(char *)&FldInfo,sizeof(FldInfo));
    GWDHead.NumFields++;
	GWDHead.Reclen=ibeg; 
	GWDHead.TimeStamp = time(0);
	_llseek (FidData,0,0);
	_lwrite (FidData,(char *)&GWDHead,sizeof(GWDHEADER));
	_llseek (FidData,0,2);
	                 
	NumVars = 1;
	            
	hVars = LocalAlloc (LMEM_MOVEABLE|LMEM_ZEROINIT,NumVars * sizeof(BTVARDESC));
	pVars =(LPBTVARDESC) LocalLock(hVars);
	            
	pVars->BT_VARLEN=4;
	pVars->BT_VARTYP=BT_INTEGER;
	pVars->BT_VAROFF=0;
	BT_CREATE (PrimeIndex, 4, FALSE, 1, 1,pVars,FALSE, 0, GWDHead.TimeStamp, FALSE);
	LocalUnlock(hVars);
	LocalFree(hVars); 
	_lclose (FidData);
	
	hDB = OpenGWDatabase (Name,BT_WRITE);
	if (!hDB) return (FALSE);
	CloseGWDatabase (hDB); 
	 
	return TRUE;
} 

BOOL LineInt (LPLINEINT lInt1, LPLINEINT lInt2)
{	
	short	rc;
	double	X1, Y1, X2, Y2, A1, A2, X3, Y3; 
	short	Mnx1, Mxx1, Mny1, Mxy1;
	short	Mnx2, Mxx2, Mny2, Mxy2;
	
	if (lInt1->Mxx < lInt2->Mnx ||
		lInt1->Mnx > lInt2->Mxx	||
		lInt2->Mxx < lInt1->Mnx ||
		lInt2->Mnx > lInt1->Mxx)
		return FALSE; 
	
	Mny1 = min (lInt1->Point1.y,lInt1->Point2.y);
	Mxy1 = max (lInt1->Point1.y,lInt1->Point2.y);
	Mny2 = min (lInt2->Point1.y,lInt2->Point2.y); 
	Mxy2 = max (lInt2->Point1.y,lInt2->Point2.y); 
	if (Mny1 > Mxy2 ||
		Mny2 > Mxy1 ||
		Mxy1 < Mny2 ||
		Mxy2 < Mny1)
		return FALSE; 
	if (SamePoint (lInt1->Point1,lInt2->Point1))
		return FALSE;
	if (SamePoint (lInt1->Point2,lInt2->Point1))
		return FALSE;
	if (SamePoint (lInt1->Point1,lInt2->Point2))
		return FALSE;
	if (SamePoint (lInt1->Point2,lInt2->Point2))
		return FALSE;
	X1 = lInt1->Point1.x;	
	Y1 = lInt1->Point1.y;
	A1 = getaz (lInt1->Point1,lInt1->Point2);	
	X2 = lInt2->Point1.x;	
	Y2 = lInt2->Point1.y;
	A2 = getaz (lInt2->Point1,lInt2->Point2);	
    rc = LINSEC_TOL (X1, Y1, A1, X2, Y2, A2, &X3, &Y3, P_TOL);
	if (rc != 1)
		return FALSE;
	if (X3 < lInt1->Mnx ||
		X3 > lInt1->Mxx ||
		X3 < lInt2->Mnx ||
		X3 > lInt2->Mxx)
		return FALSE;
	if (Y3 < Mny1 ||
		Y3 > Mxy1 ||
		Y3 < Mny2 ||
		Y3 > Mxy2)
		return FALSE;
	return TRUE;
} 

/*BOOL ComputeNewMapControlPoint (LPDPOINT pPoint)
{   
	HANDLE	hTran; 
	char	TranFile[128]; 
	double	XOldSP[32],YOldSP[32],XNewSP[32],YNewSP[32],XNewUTM[32],YNewUTM[32];
	double	XBM[32],YBM[32];
	DPOINT	Point;
	short	npnts=0, TotPnts=0,i; 
	float	RSQMIN; 
	BOOL	First=TRUE;
	HFILE Fid;
	OFSTRUCT OFStruct;
	char	str[260]; 

Top:	
	_fstrcpy (TranFile,"d:\\soilbmps\\[SOILMAP].dpt");
	ExpandText (TranFile);
	if (!(hTran=LoadTranFile(TranFile,1)))
	{ 
		if (!First)
			return FALSE;
		First=FALSE;
		_fstrcpy (TranFile,"d:\\soilbmps\\[SOILMAP]e.mpt");
		ExpandText (TranFile);
		if ((npnts=LoadTranFilePoints(TranFile,XBM,YBM,XOldSP,YOldSP)))
		{
			_fstrcpy (TranFile,"d:\\soilbmps\\[SOILMAP]e.cpt");
			ExpandText (TranFile);
			if (!(hTran=LoadTranFile(TranFile,1)))
			{
				return FALSE;
			} 
			for (i=0;i<npnts;i++) 
			{
		    	TRANS2 (XBM[i],YBM[i],&XNewUTM[i],&YNewUTM[i],hTran);  
		    	Point.x = XNewUTM[i];
		    	Point.y = YNewUTM[i]; 
		    	ConvertPoint ("usgsdoq",&Point,1);
		    	XNewSP[i] = Point.x;
		    	YNewSP[i] = Point.y;
		    }
			CloseTRANS2 (&hTran); 
		}
		TotPnts = npnts;
		_fstrcpy (TranFile,"d:\\soilbmps\\[SOILMAP]w.mpt");
		ExpandText (TranFile);
		if ((npnts=LoadTranFilePoints(TranFile,XBM,YBM,&XOldSP[TotPnts],&YOldSP[TotPnts])))
		{
			_fstrcpy (TranFile,"d:\\soilbmps\\[SOILMAP]w.cpt");
			ExpandText (TranFile);
			if (!(hTran=LoadTranFile(TranFile,1)))
			{
				return FALSE;
			} 
			for (i=0;i<npnts;i++) 
			{
		    	TRANS2 (XBM[i],YBM[i],&XNewUTM[i],&YNewUTM[i],hTran);  
		    	Point.x = XNewUTM[i];
		    	Point.y = YNewUTM[i]; 
		    	ConvertPoint ("usgsdoq",&Point,1);
		    	XNewSP[TotPnts+i] = Point.x;
		    	YNewSP[TotPnts+i] = Point.y;
		    }
			CloseTRANS2 (&hTran);
		} 
		TotPnts += npnts; 
		_fstrcpy (TranFile,"d:\\soilbmps\\[SOILMAP].dpt");
		ExpandText (TranFile);
		Fid = GSSiOpenFile (TranFile,&OFStruct,OF_CREATE);
		for (i=0;i<TotPnts;i++)
		{
			sprintf (str,"%f %f %f %f",XOldSP[i],YOldSP[i],XNewSP[i],YNewSP[i]);
			fputstring (str,Fid);
		}
		_lclose (Fid);
		goto Top;
	} 
    TRANS2 (pPoint->x,pPoint->y,&pPoint->x,&pPoint->y,hTran); 
	CloseTRANS2 (&hTran);
	
	return TRUE;
}*/
   
BOOL ComputeNewMapControlPoint (LPDPOINT pPoint)
{   
	HANDLE	hTran; 
	char	TranFile[128]; 

Top:	
	_fstrcpy (TranFile,"[%MAPTRANFILE]");
	ExpandText (TranFile);
	if ((hTran=LoadTranFile(TranFile,2)))
	{ 
	    TRANS2 (pPoint->x,pPoint->y,&pPoint->x,&pPoint->y,hTran); 
		CloseTRANS2 (&hTran);
	} 
	
	return TRUE;
}
   
short LoadTranFilePoints (LPSTR Name,LPDOUBLE XFROM, LPDOUBLE YFROM, LPDOUBLE XTO, LPDOUBLE YTO)
{
	HFILE Fid;
	OFSTRUCT OFStruct;
	char	str[260]; 
	HANDLE	handle=0, hcoord=GSSiGlobAlloc (GMEM_MOVEABLE,4096); 
	float	RSQMIN;
	short	N=0;
	
	Fid = GSSiOpenFile (Name,&OFStruct,OF_READ);
	if (Fid == HFILE_ERROR)
		return 0;
	while (fgetstring (str,256,Fid))
	{ 
		if (sscanf (str,"%Flf %Flf %Flf %Flf",XFROM++,YFROM++,XTO++,YTO++) == 4)
			N++;
		else
			goto Exit;
	}  
Exit:
	_lclose (Fid);
	return N;
}


BOOL SplitSegment (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{
 char key;     
 switch (Message)
   {
    case GF_CLOSE: 
        RemoveGraphicsFunction (hWnd);
    	break;
        
   	case GF_INIT:
   		if (!CurView->UpdateFile)
   		{
	 		MessageBox(GetFocus(), "No update file in this viewport", NULL,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		} 
   		else
   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONUP:
    {
		short	Item; 
		POINT	MousePoint, CursorPos;
		DPOINT	BasePoint;
    	long	Ref; 


    	MousePoint=MAKEPOINT(lParam);
	    if (!CurView->hTranWinToBase) break;
	    BasePoint=WinPtToBasePt(MousePoint);
		ClearHighlightList ();
	    PickItems (hWnd,BasePoint);
	    if (!NumPicked)
	    	break;  
	    Item = NumPicked-1;  
		AddToHighlightList (PickList[Item].Refno,&PickList[Item],TRUE);
	    ShowPickedItem (hWndMain,Item); 
        GetCursorPos (&CursorPos);
		if (MessageBox (GetFocus(),"Do you wish to split this segment?","Verify split",MB_YESNO) == IDYES)
		{   
			long	Newref1, Newref2;
			
			SetCursorPos (CursorPos.x,CursorPos.y);
		    RemoveFromHighlightList (PickList[Item].Refno,2);
		    ShowPickedItem (hWndMain,Item);  
			SplitPoly (Item,&Newref1,&Newref2);
			ClearHighlightList ();
			PickByRefno(Newref1,NULL,NULL,-1);
			ShowPickedItem (hWndMain,0); 
			ClearHighlightList ();
			PickByRefno(Newref2,NULL,NULL,-1);
			ShowPickedItem (hWndMain,0); 
            RemoveGraphicsFunction (hWnd);  
       	}
       	else
       	{
		    RemoveFromHighlightList (PickList[Item].Refno,2);
		    ShowPickedItem (hWndMain,Item);  
		}
		ClearHighlightList ();
    }
    	break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}

/*BOOL SplitPoly (short Item, LPLONG pNewRef1, LPLONG pNewRef2)
{   
	short	OldType=12,NewType=92; 
	short	NewPolyType=1,NumNewPolyPoints=2,NewLineSymbol=PickList[Item].Desc;
	long	NewRefno; 
	LPSTR	pStuff=NULL, Prefix=NULL,UDI=NULL;
	BOOL	rtn;
	HANDLE	hNewPolyPoints; 
	LPDPOINT	pNewPoints;
	
	GetPickName (Item); 
	_fstrcpy (PltName,PickName);
	DeletePickedItem (Item,OldType,NewType);
	hNewPolyPoints = GSSiGlobAlloc (GMEM_MOVEABLE,sizeof(DPOINT)*(long)NumNewPolyPoints);
	pNewPoints = GlobalLock (hNewPolyPoints);
	*pNewPoints++ = PickList[Item].BeginPoint;
	*pNewPoints = PickList[Item].PickedPoint;
	GlobalUnlock (hNewPolyPoints);
	NewRefno = GetNewRefno (PickName); 
	rtn=AddPolyToMap (1,&NumNewPolyPoints, &hNewPolyPoints,NewPolyType,NewRefno,2,NewLineSymbol,pStuff,Prefix,UDI,-1,-1,-1);
	pNewPoints = GlobalLock (hNewPolyPoints);
	*pNewPoints++ = PickList[Item].PickedPoint;
	*pNewPoints = PickList[Item].EndPoint;
	GlobalUnlock (hNewPolyPoints);
	NewRefno = GetNewRefno (PickName); 
	rtn=AddPolyToMap (1,&NumNewPolyPoints, &hNewPolyPoints,NewPolyType,NewRefno,2,NewLineSymbol,pStuff,Prefix,UDI,-1,-1,-1);
	GlobalFree (hNewPolyPoints);
	CloseMap();
	return rtn;
}  */

BOOL SplitPoly (short Item, LPLONG pNewRef1, LPLONG pNewRef2)
{   
	HANDLE	hSeg1, hSeg2;   
	LPTHEME	pTheme;  
	short	npt1=1,npt2=1;
	LPDPOINT	pDpoint;
	long	lmem=(long)PickList[Item].NumPoints*sizeof(DPOINT);  
	double	PickDist, Dist=0;
	long	Newref1, Newref2;
			
	hSeg1 = GSSiGlobAlloc (GMEM_MOVEABLE,lmem);
	hSeg2 = GSSiGlobAlloc (GMEM_MOVEABLE,lmem);  
	PickDist = PickList[Item].Length * PickList[Item].PCT;
	pTheme = AddTheme (GF_SAVEPOLY_THEME);
	CurView->PassID = 4;
	ProcessPickedItem (Item,FALSE);        		
	DeleteTheme (pTheme);
	if (hSavePoly)
	{   LPMNMXCORD lpRect;
		short	nPnts; 
		LPDPOINT	lpDpoint; 
		DPOINT	LastPoint;  
		BOOL 	First=TRUE;
	    		    
        nPnts = nSavePoly; 
        lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
        lpRect++;
        lpDpoint = (LPDPOINT) lpRect;
        pDpoint = GlobalLock (hSeg1);
        LastPoint = *lpDpoint;
        *pDpoint++ = *lpDpoint++;   
        nPnts--;
        while (nPnts--)
        {   
        	if (!First)
        	{
            	*pDpoint++ = *lpDpoint; 
            	npt2++;
        	}
        	else
        	{
            	Dist += ldistp (LastPoint,*lpDpoint);  
            	if (fabs (Dist - PickDist) < P_TOL) 
            	{
            		*pDpoint = *lpDpoint;
            		npt1++;
            		GlobalUnlock (hSeg1);
		            pDpoint = GlobalLock (hSeg2);  
		            *pDpoint++ = *lpDpoint;
		            First = FALSE;
            	}
            	else if (Dist > PickDist)
            	{
            		*pDpoint = PickList[Item].PickedPoint;
            		npt1++;
            		GlobalUnlock (hSeg1);
		            pDpoint = GlobalLock (hSeg2);  
            		*pDpoint++ = PickList[Item].PickedPoint;
	            	*pDpoint++ = *lpDpoint; 
	            	npt2++;
		            First = FALSE;
            	} 
            	else
            	{ 
            		LastPoint = *lpDpoint;
            		*pDpoint++ = *lpDpoint;
            		npt1++;
            	}
			}  
			lpDpoint++;
        }
        GlobalUnlock (hSavePoly);
        GlobalFree (hSavePoly);    
        GlobalUnlock (hSeg2);
    }
	_fstrcpy (PltName,EditName);
    Newref1=GetNewRefno(PltName);
    Newref2=GetNewRefno(PltName);
	AddPolyToMap (1,&npt1, &hSeg1,1,Newref1,2,PickList[Item].Desc,NULL,NULL,NULL,-1,-1,-1);
	AddPolyToMap (1,&npt2, &hSeg2,1,Newref2,2,PickList[Item].Desc,NULL,NULL,NULL,-1,-1,-1); 
	GlobalFree (hSeg1);
	GlobalFree (hSeg2);
	DeletePickedItem (Item,12,92);
	GetPickName (Item); 
	_fstrcpy (PltName,PickName);
	*pNewRef1 = Newref1;  
	*pNewRef2 = Newref2;  
	return TRUE;
}

BOOL FAR PASCAL DECOMPPOLYMsgProc(HWND hWndDlg, WORD Message, WPARAM wParam, LPARAM lParam)
{ 
 char	str[128], txt[128];   
 short    BRtn,i;
 long	NumItems; 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    
    case WM_INITDIALOG: 
        NumItems = BT_NUM_IN_INDEX(hHighlight);  
        if (!NumItems)
        { 
            MessageBox( GetFocus(),"No items highlighted",NULL, MB_ICONEXCLAMATION);
            EndDialog(hWndDlg, FALSE); 
            break;
        }  
        sprintf(str,"%ld items selected",NumItems);
        SetDlgItemText(hWndDlg,IDC_PROCESS_MESS,str);  
        break; /* End of WM_INITDIALOG                                 */
    
   
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */  
         if (Processing)
         	break;
         PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
#if WIN32
         switch(LOWORD(wParam))
#else
         switch(wParam)
#endif
         {  
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 ContinueProcessing = FALSE;
                 break;
                 
            case IDC_LOCATE_DESTMAP: 
                 *str=0;
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
                 {
                    if (!GetSaveName2 (hWndDlg,str,IDS_FILTERPLT,".PLT",IDS_FILEPLT)) break;   
                 }
                 else  
                 {
                    if (!GetFileName3 (hWndDlg,str,IDS_FILTERPLT,IDS_FILEPLT)) break;   
                 }
                 SetDlgItemText (hWndDlg,IDC_DESTMAP,str);
                 break;
            case IDC_GET_INTERIOR_SYM:
            	 *str = 0;
                 if (!SelectLineSymbol (hWndDlg,str,NULL,NULL))
                    break;  
                 SetDlgItemText (hWndDlg,IDC_INTERIOR_LINES,str); 
                 break;  
            case IDC_GET_EXTERIOR_SYM: 
            	 *str = 0;
                 if (!SelectLineSymbol (hWndDlg,str,NULL,NULL))
                    break;  
                 SetDlgItemText (hWndDlg,IDC_EXTERIOR_LINES,str); 
                 break;  
                
            case IDOK:
            {
            	LPTHEME	pTheme;
            	short	pos, nareas, n, npoints, LineSymbol, IntLineSym, ExtLineSym;  
            	long	iref, CurItem=0;
                LPSHORT	pPolyParts;
            	HIGHLIGHTDATA	HighlightData; 
				LINESKEY	LinesKey, LinesKey2;
				LINESDATA	LinesData;   
				LPOINT	LastNode, NodePoint;
            	BOOL	First, FirstLoop;
            	HANDLE	hPoints=GSSiGlobAlloc (GMEM_MOVEABLE,(long)16000*(long)sizeof(DPOINT));
            	HPDPOINT	pDPoint;
                short    NumSyms=0;  
                HANDLE hSymDesc=0;  
                DPOINT	ToPoint;
                char	NewPltName[128]; 
                long	NewRef = 1,ii;
            	
            	if (!GetDlgItemText (hWndDlg,IDC_DESTMAP,NewPltName,sizeof(NewPltName)))
            	{
                    MessageBox(GetFocus(),"No destination map", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
            	}  
            	if (!GetDlgItemText (hWndDlg,IDC_INTERIOR_LINES,str,sizeof(str)))
            	{
                    MessageBox(GetFocus(),"No interior symbol", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
            	}  
                IntLineSym = GetDictSymbolNumber (str);
                AddToSymList (IntLineSym,&NumSyms,&hSymDesc);
            	if (!GetDlgItemText (hWndDlg,IDC_EXTERIOR_LINES,str,sizeof(str)))
            	{
                    MessageBox(GetFocus(),"No exterior symbol", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
            	}  
                ExtLineSym = GetDictSymbolNumber (str);
                AddToSymList (ExtLineSym,&NumSyms,&hSymDesc);
            	Processing = TRUE;  
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE); 

				DecompInit (&HLTBounds);
		        NumItems = TotHLTPoints;  
                SetDlgItemText(hWndDlg,IDC_PROCESS_MESS,"Step1: decomposing polygons");
                pos = BT_FIRST;
                while (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData)&&ContinueProcessing)
                {   
                    pos = BT_NEXT; 
                    PickList[0]=HighlightData.PD;
                    if (PickList[0].Type != 3)
                        goto NextHlt; 
                    pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME);
                    CurView->PassID = 4;
                    ProcessPickedItem (0,FALSE);                
                    DeleteTheme (pTheme);
                    if (hSavePoly)
                    {   LPMNMXCORD lpRect;
                        short   nPnts,i; 
                        long	Totp;
                        HPDPOINT    lpDpoint;
						LPSHORT	pPolyParts; 
						DPOINT	FirstPoint;

                        if (hSavePolyParts)
                        {
                        	pPolyParts = GlobalLock (hSavePolyParts);
                        	nareas = *pPolyParts++; 
                        }
                        else
                        { 
                            nareas=1;  
                            pPolyParts = &nSavePoly;
                        }
                            
                        lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
                        lpRect++;
                        lpDpoint = (LPDPOINT) lpRect;  
                        FirstLoop = TRUE;
                        while (nareas--)
                        {   
                        	DPOINT	FirstPoint, LastPoint;
                        	
                        	First = TRUE;
	                        while ((*pPolyParts)--)
	                        {   
	                                
	                            if (First)
	                                FirstPoint = LastPoint = *lpDpoint;
	                            else 
	                            {
									DecompAddLine (&LastPoint, lpDpoint,iref);
									LastPoint = *lpDpoint;
								}
	                            First = FALSE;   
	                            lpDpoint++;  
	                        }
							if (nareas)
							{
								DecompAddLine (&LastPoint, lpDpoint,LONG_MIN);
								if (!FirstLoop)
								{
									LastPoint = *lpDpoint++;
									DecompAddLine (&LastPoint, lpDpoint,LONG_MIN);
								}
								FirstLoop = FALSE;
								pPolyParts++; 
							}
	                    } 
                        if (hSavePolyParts)
                        {
                        	GlobalUnlock (hSavePolyParts);
                        	GSSiGlobFree (&hSavePolyParts);
                        }
                        GlobalUnlock (hSavePoly);
                        GlobalFree (hSavePoly);
                    }
                    
            NextHlt:
            		CurItem+=HighlightData.PD.NumPoints;                    
                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem,0);
                } 
                
                SetDlgItemText(hWndDlg,IDC_PROCESS_MESS,"Step 2: locating nodes");
			    CloseMap();  
                _fstrcpy (PltName,NewPltName);
		    	CreateNewMap (PltName,&HLTBounds,NumSyms,hSymDesc,0,NULL);
				EditBounds = CurView->FileMNMX;
                DestroySymList (&NumSyms,&hSymDesc);
                pos = BT_FIRST;
                n=0;
        		LastNode.x = LONG_MAX;
        		LastNode.y = LONG_MIN;
        		CurItem = 0;
        		NumItems = BT_NUM_IN_INDEX(hLines);  
                while (!BT_FIND (hLines,(LPSTR)&LinesKey,pos,BT_ANY,(LPSTR)&LinesData)&&ContinueProcessing)
                {   
                    pos = BT_NEXT; 
                	if (LinesKey.FromNode.x == LastNode.x && LinesKey.FromNode.y == LastNode.y)
                		n++;
                	else
                	{
                		if (n>2)
                			BT_PUT (hNodes,(LPSTR)&LastNode,(LPSTR)&n); 
                		n=1;
                		LastNode = LinesKey.FromNode;
                	}
                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0);
                }
        		if (n>2)
        			BT_PUT (hNodes,(LPSTR)&LastNode,(LPSTR)&n); 
       			
                SetDlgItemText(hWndDlg,IDC_PROCESS_MESS,"Step 3: creating output file");
        		NumItems = BT_NUM_IN_INDEX(hNodes);  
	NextNode:   
				if (!ContinueProcessing)
					goto Exit;
				CurItem = NumItems - BT_NUM_IN_INDEX(hNodes);  
                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem,0);
				if (BT_FIND (hNodes,(LPSTR)&LinesKey,BT_FIRST,BT_ANY,(LPSTR)&n))
					goto Exit;
				n--;
				if (n)
                	BT_PUT (hNodes,(LPSTR)&LinesKey,(LPSTR)&n); 
                else
                	BT_DELETE (hNodes,(LPSTR)&LinesKey,(LPSTR)&n,FALSE);
				pDPoint = GlobalLock (hPoints); 
				npoints = 0;
	NextPoint:
				LinesKey.ToNode.x = LinesKey.ToNode.y = LONG_MIN;
				if (BT_FIND (hLines,(LPSTR)&LinesKey,BT_FIRST,BT_GE,(LPSTR)&LinesData))
					goto NextNode; 
				*pDPoint++ = LinesData.FromPoint; 
				ToPoint = LinesData.ToPoint; 
				npoints++; 
				BT_DELETE (hLines,(LPSTR)&LinesKey,(LPSTR)&LinesData,FALSE);
				LinesKey2.FromNode = LinesKey.ToNode;
				LinesKey2.ToNode = LinesKey.FromNode; 			
				BT_DELETE (hLines,(LPSTR)&LinesKey2,(LPSTR)&LinesData,FALSE);
				NodePoint = LinesKey.ToNode;
				if (!BT_FIND (hNodes,(LPSTR)&NodePoint,BT_FIRST,BT_EQ,(LPSTR)&n))
				{ 
					n--;
					if (n)
	                	BT_PUT (hNodes,(LPSTR)&NodePoint,(LPSTR)&n); 
	                else
	                	BT_DELETE (hNodes,(LPSTR)&NodePoint,(LPSTR)&n,FALSE);
                    
					*pDPoint++ = ToPoint;
					npoints++; 
                    GlobalUnlock (hPoints);
                    if (LinesData.LeftPoly == LONG_MIN || LinesData.RightPoly == LONG_MIN)
						goto NextNode;
                    LineSymbol = IntLineSym;
                    if (LinesData.LeftPoly == LONG_MAX || LinesData.RightPoly == LONG_MAX)
                    	LineSymbol = ExtLineSym;
                    if (NewRef >72)
                    	ii=1;
					AddPolyToMap (1,&npoints, &hPoints,1,NewRef++,2,LineSymbol,NULL,NULL,NULL,-1,-1,-1);
					goto NextNode;
				}
				else
				{
					LinesKey.FromNode = LinesKey.ToNode;
					goto NextPoint;
				}
					
	Exit:	    
				if (ContinueProcessing)
		        	SetDlgItemText(hWndDlg,IDC_PROCESS_MESS,"Processing finished");
		        else
		        	SetDlgItemText(hWndDlg,IDC_PROCESS_MESS,"Processing aborted");
			    CloseMap();  
				GlobalUnlock (hPoints);
				GlobalFree (hPoints);
                DecompClose();
            	
            	Processing = FALSE;
            	ContinueProcessing = TRUE;
                EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);
            } 
                break;
                 
            case IDC_EXIT: 
                EndDialog(hWndDlg, TRUE); 
                break;
                 
          }
          break;

    default:
        return FALSE;
   }
 return TRUE;    
}  

BOOL DecompInit (LPMNMXCORD Rect)
{   
	double	MaxDist;
	short	ld;
	BTVARDESC	BTVar[4];
	char	File[128];  
	OFSTRUCT	OFStruct;

	MaxDist = max (Rect->xmx - Rect->xmn,Rect->ymx - Rect->ymn); 
	AddNodeBias.x =  (Rect->xmx + Rect->xmn)/2;
	AddNodeBias.y =  (Rect->ymx + Rect->ymn)/2; 
	ld = log10 (MaxDist);
	
	AddNodeFactor = pow (10,8 - ld);
	GetTempFileName (NULL,"gm",NULL,(LPSTR)File);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=4;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=4;
	BTVar[2].BT_VARTYP=BT_INTEGER;
	BTVar[2].BT_VARLEN=4;
	BTVar[2].BT_VAROFF=8;
	BTVar[3].BT_VARTYP=BT_INTEGER;
	BTVar[3].BT_VARLEN=4;
	BTVar[3].BT_VAROFF=12;
	BT_CREATE (File,sizeof(LinesData), FALSE, 4, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
	hLines= BT_OPEN (File, 0, BT_WRITE, 0); 
    
	GetTempFileName (NULL,"gm",NULL,(LPSTR)File);
	BT_CREATE (File,2, FALSE, 2, 1,(LPBTVARDESC)&BTVar,FALSE, 0, 0, FALSE);
	hNodes= BT_OPEN (File, 0, BT_WRITE, 0); 
    
	return TRUE;
} 

void DecompClose (void)
{
	BT_CLOSEANDDELETE (&hLines);
	BT_CLOSEANDDELETE (&hNodes);
} 

void DecompAddLine (LPDPOINT FromPoint, LPDPOINT ToPoint, long Poly)
{
	LINESKEY	LinesKey1, LinesKey2;
	
	LinesKey1.FromNode.x = LinesKey2.ToNode.x = IDNINT ((FromPoint->x - AddNodeBias.x) * AddNodeFactor);
	LinesKey1.FromNode.y = LinesKey2.ToNode.y = IDNINT ((FromPoint->y - AddNodeBias.y) * AddNodeFactor);
	LinesKey1.ToNode.x = LinesKey2.FromNode.x = IDNINT ((ToPoint->x - AddNodeBias.x) * AddNodeFactor);
	LinesKey1.ToNode.y = LinesKey2.FromNode.y = IDNINT ((ToPoint->y - AddNodeBias.y) * AddNodeFactor);
	if (LinesKey1.FromNode.x == LinesKey1.ToNode.x && LinesKey1.FromNode.y == LinesKey1.ToNode.y)
		return;
	if (BT_FIND (hLines,(LPSTR)&LinesKey1,BT_FIRST,BT_EQ,(LPSTR)&LinesData))
		LinesData.LeftPoly = LONG_MAX;
	LinesData.RightPoly = Poly; 
	LinesData.FromPoint = *FromPoint;
	LinesData.ToPoint = *ToPoint;
    BT_PUT (hLines,(LPSTR)&LinesKey1,(LPSTR)&LinesData); 
	if (BT_FIND (hLines,(LPSTR)&LinesKey2,BT_FIRST,BT_EQ,(LPSTR)&LinesData))
		LinesData.RightPoly = LONG_MAX;
	LinesData.LeftPoly = Poly;
	LinesData.FromPoint = *ToPoint;
	LinesData.ToPoint = *FromPoint;
    BT_PUT (hLines,(LPSTR)&LinesKey2,(LPSTR)&LinesData); 
	return;
}


