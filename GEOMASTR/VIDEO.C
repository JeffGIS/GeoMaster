
#include <windows.h>
#include <commdlg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
#include <mmsystem.h>
#include <vfw.h>
#include <avifile.h>		
#include <shellapi.h> 
#include <dibapi.h> 
#include <float.h>
#include "graphint.h"
#include "resource.h" 
#include "mci.h"
#include "winexec.h"
#include "geospan.h" 
#include "address.h" 

#define	MPTOL	0.0000001

BOOL	AVInit=FALSE;

BOOL	NoVideo=FALSE;  
#include "gmextern.h"
long	GSTLID, GSFrame;
int		GSView;

BOOL	ShowInitBM=TRUE;
BOOL 	DriveHousesOnly=FALSE; 
FARPROC lpfnGEOSPANVIDEOMsgProc;  
BOOL FAR PASCAL VIDEO_CONTROLSMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
BOOL FAR PASCAL GEOSPANVIDEOMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
int	GVFind(long TLID, int View, long Frame, int mode,
		   LPSTR File, LPLONG jpegframe);
BOOL DisplayVideoAtFrame (long TLID,long Frame,LPVIEWPORT VideoVP);
BOOL GetSegData (long TLID,LPVOID Segdata);
void SetViewProfile(void);
void ReadSegData (long Offset, LPVOID pSegdata);
int	GetLogicalView (int RealView);

/*BOOL GEOSPANAtPoint (DPOINT PickedPoint,short Layer, LPSTR Viewport)
{
	long	lHouse, TLID, Frame; 
	char	cStreet[64], cHouse[32], cCity[64], mess[256];   
	BOOL	rtn=TRUE; 
	int		RealView=3, SaveMaxPick;
	LPVIEWPORT	VideoVP=NULL; 
	short	LeftView,RightView,FrontView,i;
	LPVISLIST SaveVis, SaveCurVis;
	HANDLE	hVisList;
    
    VideoVP = GetVPByName (Viewport);

	UseUserPickAp =FALSE; 
	SaveMaxPick = MaxPick;
	SystemPickAp = 0;	
	MaxPick=1;  
	SaveVis = CurView->pPickListManual; 
    hVisList=GSSiGlobAlloc ( 399,GHND,sizeof(VISLIST));
    CurView->pPickListManual = (LPVISLIST)GlobalLock (hVisList);    
    SaveCurVis = CurVis;
    CurVis = CurView->pPickListManual;
    CurVis->hVisList=hVisList;
    InitVis ();
    for (i=0;i<MAX_VIEWPORT_FILES;i++)
    	if (i != Layer)
    		CurVis->FileIsVisible[i]=FALSE;
	
    PickItems (CurView->hWnd,PickedPoint); 
	GlobalUnlock (hVisList);
	GlobalFree (hVisList);
    MaxPick = SaveMaxPick; 
    CurView->pPickListManual = SaveVis;  
    CurVis = SaveCurVis;
	UseUserPickAp =TRUE;
    if (!NumPicked)
    	return FALSE;
    TLID = PickList[0].Refno;
			    
	OpenAddressFiles (hWndMain);
//    CloseAddressFiles(FALSE);
	if (!GetSegData (TLID,&Segdata))
		return FALSE;
	Frame = Segdata.fframe + (Segdata.tframe - Segdata.fframe) * PickList[0].PCT;
	if (Segdata.vdir == '0')
	{
		LeftView = 8;
		RightView = 3;
		FrontView = 1;
	}
	else
	{
		LeftView = 3;
		RightView = 8;
		FrontView = 1;
	}
    if (fabs(PickList[NumPicked-1].OffDist) < 5)
    	RealView = FrontView;    	
	else if (PickList[NumPicked-1].OffDist <=0)
		RealView = LeftView;
	else
		RealView = RightView; 
	if (DriveDir < 0 && RealView == FrontView)
		RealView = OppositeView(RealView); 
    HaveTurnArrows = FALSE;
    CloseAddressFiles(FALSE);
	return DisplayVideoAtFrame (TLID,Frame,VideoVP);
}

BOOL GEOSPANAtAddress (LPSTR DBName,LPSTR pSQL, LPSTR House, LPSTR Street, LPSTR City, LPSTR Viewport)
{   
	long	lHouse, TLID, Frame; 
	char	cStreet[64], cHouse[32], cCity[64], mess[256];   
	BOOL	rtn=TRUE; 
	int		RealView=3;
	LPVIEWPORT	VideoVP=NULL;
	 
	HANDLE hDB;
	HANDLE		hBT, hSQL;
	LPOPENFILEDATA	FilePtr;
	LPOPENSQLDATA	SQLPtr;
	HANDLE		SaveHandle;
	MSG		msg;   
    
    VideoVP = GetVPByName (Viewport);
    hSQL=0;
	if (!OpenDataFile (DBName,pSQL,BT_READ,&hSQL))  
	{   
		sprintf (mess,"Failed to open address database %s",DBName);
		MessageBox(GetFocus(), mess, NULL,MB_ICONEXCLAMATION|MB_OK);
		return FALSE;  
	}
	_fstrcpy (cHouse,House);
	_fstrcpy (cStreet,Street);
	_fstrcpy (cCity,City);
	ExpandText (cHouse);
	ExpandText (cStreet);
	ExpandText (cCity); 
	lHouse = atol (cHouse);
	CloseDataFile (TRUE, &hSQL); 
	sprintf (mess,"Address: %s %ld %c",cStreet,lHouse,cCity);
	GSSiTrace (mess);
	if (!GetTLIDAndFrameFromAdd (cStreet,lHouse,cCity,&TLID, &Frame))
	{   
		sprintf (mess,"Failed to match address for  %ld %s,%s",lHouse,cStreet,cCity);
		MessageBox(GetFocus(), mess, NULL,MB_ICONEXCLAMATION|MB_OK);
		return FALSE;  
	} 
	return DisplayVideoAtFrame (TLID,Frame,VideoVP);
}
*/	
BOOL DisplayVideoAtFrame (long TLID,long Frame,LPVIEWPORT VideoVP)
{   
	char	mess[128];
	
	sprintf (mess,"TLID and Frame:%ld %ld",TLID,Frame);
	GSSiTrace (mess);  
	if (!VideoWnd && !VideoVP) 
	{
		GEOSPANVideo(hWndMain);
    }
    if (Frame < 0)
    	RealView = 8;
    else
    	RealView = 3;
    GSView = RealView;
    GSTLID = TLID;
    GSFrame = labs(Frame); 
    if (VideoCntlWnd && !VideoVP)
    	PostMessage(VideoCntlWnd, WM_COMMAND, IDOK, 0L); 
    else if (VideoVP)
    {   
    	int	rc;
    	
    	if (*VideoVP->lpFiles[0] && VideoVP->FileType[0] == 3)
    	{
	    	rc = GVFind(GSTLID, GSView, GSFrame, 3, CurMovie, &CurAVIFrame); 
	    	if (!rc)
	    	{ 
				HDIB	hDIB;
				HPALETTE	hPal;  
				LPBITMAPINFO	pDIB;
				HANDLE	hVideoBM;
	    
			    hDIB = FrameToDIB (VideoWnd);
			    if (!hDIB)
			    	return FALSE; 
				hPal = CreateDIBPalette (hDIB);
			    hVideoBM = DIBToBitmap (hDIB,hPal); 
			    pDIB=(LPBITMAPINFO)GlobalLock(hDIB);
			    GlobalUnlock(hDIB);
			    DeleteObject (hPal); 
			    SaveDIB (hDIB,VideoVP->lpFiles[0]);
			    DestroyDIB (hDIB);
				DeleteObject (hVideoBM); 
				SetDisplayMode (CurView->hDC, GF_TEXTMODE);
				SelectClipRgn (CurView->hDC,NULL);
				DisplayBMFileInRect (VideoVP->hDC,VideoVP->lpFiles[0],VideoVP->DrawRect,TRUE);
	    	} 
	    	else
			{   
				sprintf (mess,"Failed to find video for TLID %ld, View %i, Frame %ld",GSTLID,GSView,GSFrame);
				MessageBox(GetFocus(), mess, NULL,MB_ICONEXCLAMATION|MB_OK);
			}
	    	
	    }
    }
    
//	rtn = DisplayVideo (TLID,Frame,RealView);
	return TRUE;
}

BOOL GEOSPANAtNetwork (LPLONG PreferredPath,long Refno, double PCT, double Length,double Offdist,BOOL AtInt)
{   
	long	TLID, Frame; 
	BOOL	rtn=TRUE; 
	 
	MSG		msg;  
	NETLINKSKEY		NetLinksKey;
	NETLINKSDATA	NetLinksData;  
	NETMARKERSKEY1	NetMarkersKey1;
	NETMARKERSKEY2	NetMarkersKey2_1,NetMarkersKey2_2;
	NETMARKERSDATA1 NetMarkersData1;
	NETMARKERSDATA2	NetMarkersData2_1,NetMarkersData2_2;
	NETREFSKEY		NetRefsKey,NetRefsKey2;
	NETREFSDATA		NetRefsData,NetRefsData2;  
	BOOL	Opened, Opened2, Opened3;
	double		MPinc, MPValue, MP;  
	int			st, st2, pos;
	NETVIDEOKEY1	NetVidKey1_1, NetVidKey1_2;
	NETVIDEODATA1	NetVidData1_1, NetVidData1_2; 
	MARKERVAL NetMarker;
	char	TrueName[34]; 
	int		dir,ii; 
	short	Dir;  
	long	Path;
	
	CurNetRef = Refno;
	CurNetPCT = PCT;
	CurNetLength = Length;
	CurNetOffdist = Offdist;
	NetVideo=TRUE;
    SetViewport(*pCommandViewport);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	SelectClipRgn (CurView->hDC,NULL);
	OpenNetLinkAndRef (NetworkID,FALSE,&Opened);  
	OpenNetMarkers (NetworkID,FALSE,&Opened2);  
	OpenNetVideoIndex (NetworkID,FALSE,&Opened3);
	NetLinksKey.Ref = Refno;
	NetLinksKey.NetID = NetworkID;  
	NetLinksKey.Path = *PreferredPath;
	st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData);
	if ( st || 
		(NetLinksKey.Ref != Refno) ||
		(NetLinksKey.Path != *PreferredPath && *PreferredPath))
	{
		NetLinksKey.Ref = Refno;
		NetLinksKey.NetID = NetworkID;  
		NetLinksKey.Path = 0;
		st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData);
	}
	if (!st && NetLinksKey.Ref == Refno)
	{   
		*PreferredPath = NetRefsKey.Path =  NetLinksKey.Path;
		NetRefsKey.MP = NetLinksData.MP; 
		st = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GE,(LPSTR)&NetRefsData);
		BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey2,BT_NEXT,BT_ANY,(LPSTR)&NetRefsData2);
		if (NetRefsData.Dir == 2)
			PCT = 1.0 - PCT;
		MPinc = PCT * Length;
		
		MP = NetLinksData.MP += MPinc;  
		if (!GetMilePointFromMP (NetRefsKey.Path,1,MP, &NetMarker,&Dir))
		{
			rtn = FALSE; 
			goto Exit;
		} 
		if (AtInt)
		{   
			NetDirection = 1;
//			DriveDir = 1;
			if (PCT < 0.5 && Dir < 0 || PCT > 0.5 && Dir > 0) 
			{
//				DriveDir = -1;
				NetDirection = 2;
			
			}
		}
		MPValue = NetMarker.Value;			
/*		NetMarkersKey2_2.MarkerID=1;
		NetMarkersKey2_2.Path = NetLinksKey.Path;
		NetMarkersKey2_2.MP = NetLinksData.MP;

		st2 = BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2_2,BT_FIRST,BT_GE,(LPSTR)&NetMarkersData2_2); 
		if (st2)
			pos = BT_LAST;
		else
			pos = BT_PRIOR;
		st2 = BT_FIND (hBTNetMarkers2,(LPSTR)&NetMarkersKey2_1,pos,BT_ANY,(LPSTR)&NetMarkersData2_1);	
		MPValue = NetMarkersData2_1.Value +
				  (NetMarkersData2_2.Value - NetMarkersData2_1.Value) * (NetLinksData.MP - NetMarkersKey2_1.MP)/(NetMarkersKey2_2.MP - NetMarkersKey2_1.MP);
*/      
		Path = NetLinksKey.Path;
		if (GetTrueStreetName (Path,TrueName,0))
		{   
			if (*TrueName)
			{
				LPSTR lpEnd;
				
				lpEnd = _fstrchr (TrueName,0); 
				lpEnd--;
				switch (*lpEnd)
				{
					case 'E':
						NetDirection = 1;
					break;
					case 'W':
						NetDirection = 2;
					break;
					case 'N':
						NetDirection = 1;
					break;
					case 'S':
						NetDirection = 2;
					break; 
					default:
						goto NoDir;
				}
				*lpEnd = 0;
                Path = GetStreetNumFromName (TrueName,1,BT_FIRST,TrueName); 
			}
			
		} 
NoDir:
		NetVidKey1_2.Path = Path;
		NetVidKey1_2.Dir = NetDirection;  
		NetVidKey1_2.View = 0;
		if (BT_FIND (hBTNetVideo1,(LPSTR)&NetVidKey1_2,BT_FIRST,BT_GE,(LPSTR)&NetVidData1_2))
			rtn = FALSE;
		else
		{
			if (NetVidKey1_2.Path != Path)
				rtn = FALSE;
			else
			{
				NetVidKey1_2.View = RealView;
				_fmemset (NetVidKey1_2.Prefix,' ',2);
				_fmemset (NetVidKey1_2.Suffix,' ',2);
				NetVidKey1_2.Value = MPValue;   
		/*		st=0;
				st = BT_FIND (hBTNetVideo1,(LPSTR)&NetVidKey1_1,BT_FIRST,BT_ANY,(LPSTR)&NetVidData1_1);
				while (!st)
				{
					st = BT_FIND (hBTNetVideo1,(LPSTR)&NetVidKey1_1,BT_NEXT,BT_ANY,(LPSTR)&NetVidData1_1);
					ii=0;
				} */
				st2=BT_FIND (hBTNetVideo1,(LPSTR)&NetVidKey1_2,BT_FIRST,BT_GE,(LPSTR)&NetVidData1_2);
				if (st2)
					pos = BT_LAST;
				else
					pos = BT_PRIOR;
				if (fabs (NetVidKey1_2.Value - MPValue) < MPTOL && NetVidKey1_2.View == RealView) 
				{
					Frame = NetVidData1_2.Frame;
					NetVidKey1_1 = NetVidKey1_2;
				} 
				else
				{
					st2 = BT_FIND (hBTNetVideo1,(LPSTR)&NetVidKey1_1,pos,BT_ANY,(LPSTR)&NetVidData1_1);
					if (!st2 && NetVidKey1_2.View != RealView)
					{ 
						Frame = NetVidData1_1.Frame;
						NetVidData1_2 = NetVidData1_1;
					}
					else if (st2 || NetVidKey1_1.View != RealView)
					{ 
						Frame = NetVidData1_2.Frame;
					}
					else
						Frame = IDNINT(NetVidData1_1.Frame +
							((MPValue - NetVidKey1_1.Value)/(NetVidKey1_2.Value - NetVidKey1_1.Value)) *
							(NetVidData1_2.Frame - NetVidData1_1.Frame));
				}
			}
		}
	}
	else
		rtn = FALSE;  
Exit:					          		
	CloseNetLinkAndRef (Opened);
	CloseNetMarkers (Opened2);
	CloseNetVideoIndex (Opened3);
	if (!rtn)
		return rtn;
	_fstrcpy (DiskID,NetVidData1_2.DiskID); 
	if (!VideoWnd) 
	{   int	ii;
	
		GEOSPANVideo(hWndMain);
    }  
    GSView = NetVidKey1_1.View;
    GSTLID = TLID;
    GSFrame = Frame;
    PostMessage(VideoCntlWnd, WM_COMMAND, IDOK, 0L);
    
	return rtn;
}


BOOL DisplayUnlinkedVideo (long Path,short Dir,short View,MARKERVAL NetMarker)
{
	NETVIDEOKEY1	NetVidKey1_1, NetVidKey1_2;
	NETVIDEODATA1	NetVidData1_1, NetVidData1_2;  
	long	Frame;   
	BOOL	Opened;
	
	OpenNetVideoIndex (1,FALSE,&Opened);
	_fmemset (&NetVidKey1_2,0,sizeof(NETVIDEOKEY1));
	NetVidKey1_2.Path = Path;
	NetVidKey1_2.Dir = NetDirection;  
	NetVidKey1_2.View = View; 
	NetVidKey1_2.MarkerID = 1;
	_fmemcpy (&NetVidKey1_2.Prefix,&NetMarker,sizeof(NetMarker));
	
	if (BT_FIND (hBTNetVideo1,(LPSTR)&NetVidKey1_2,BT_FIRST,BT_GE,(LPSTR)&NetVidData1_2))
		return FALSE;
	BT_FIND (hBTNetVideo1,(LPSTR)&NetVidKey1_1,BT_PRIOR,BT_ANY,(LPSTR)&NetVidData1_1);
	_fstrcpy (DiskID,NetVidData1_2.DiskID);     
	Frame = IDNINT(NetVidData1_1.Frame +
		((NetMarker.Value - NetVidKey1_1.Value)/(NetVidKey1_2.Value - NetVidKey1_1.Value)) *
		(NetVidData1_2.Frame - NetVidData1_1.Frame));
	if (!VideoWnd) 
	{   int	ii;
	
		GEOSPANVideo(hWndMain);
    }  
    GSView = NetVidKey1_1.View;
    GSTLID = 0;
    GSFrame = Frame;
	CloseNetVideoIndex(Opened); 
    PostMessage(VideoCntlWnd, WM_COMMAND, IDOK, 0L);
    return TRUE;
}

void FullScreenPlay(HWND hWnd, LPSTR Name)
{   
	char	str[256];
	
    wsprintf((LPSTR)str,"open %s alias mov",Name);

	mciSendString((LPSTR)str, NULL, 0, hWnd);
	mciSendString("play mov fullscreen", NULL, 0, hWnd); 
	mciSendString("close mov", NULL, 0, hWnd); 
	return;
} 

BOOL initAVI(void)
{   int dw;
	char	achCommand[128];
    
    dw=mciSendString("open avivideo", NULL, 0, NULL);              
    if (dw)
    {
		 mciGetErrorString(dw, achCommand,sizeof(achCommand));
		 MessageBox(GetFocus(), achCommand, "Video for Windows Error",MB_ICONEXCLAMATION|MB_OK);
		 return FALSE;
	}
	else
		return TRUE;
}

void termAVI(void)
{    
	if (!AVInit) return; 
	AVInit=FALSE;
    mciSendString("close avivideo", NULL, 0, NULL); 
    return;
}

BOOL GEOSPANVideo(HWND hWnd)
{HDC hDC;
 char key;     
 int	st;
    
    DoPaint = FALSE;
    GSinitAVI();
	lpfnGEOSPANVIDEOMsgProc = MakeProcInstance((FARPROC)GEOSPANVIDEOMsgProc, hInst);
	CreateDialog(hInst, (LPSTR)"GEOSPANVIDEO", hWnd, lpfnGEOSPANVIDEOMsgProc);
//	PostMessage(hWndMain,WM_USER, NULL, 0L);        
	SetViewProfile();   
	DoPaint = TRUE;
    return (TRUE);
}

BOOL FAR PASCAL GEOSPANVIDEOMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 	int		isDir, nchar;
	static	int	width,height;
	char	Name[128], str[16];
	RECT			Rect;    
	static	RECT	LastRect;
	HMENU			hMenuSystem;
	int				iLoop;
	long            status, i, holder;
	FARPROC			fpfn;
	NPLOGPALETTE	pLogPal;
	HDC				hDC;
	double r8;
	int sign, dec, k;
	char testtlid[22],RomToUse[64];
	long testframe, errcode;
	int *sptr = &sign, *dptr = &dec;
	static	HANDLE	hPal;
	static	HBRUSH	hBrushTrans=NULL;
	static	int		TransColourIndex;
    static BOOL RDOWN = FALSE, LDOWN = FALSE, Startup = TRUE;
	int		x, Fid;
	BOOL	Left;
	OFSTRUCT	OFStruct;
	PAINTSTRUCT ps;  
	int	MaxDimension, ShadowInc;  
	HWND	hwnd;
	MINMAXINFO FAR* lpmmi;
	WINDOWPOS	FAR* wp;    
	static	BOOL	IsMin=FALSE, First=TRUE; 
	HDIB	hDIB;

if (GetCDWnd) return FALSE;

if (Message == WM_INITDIALOG)
{
//		case WM_CREATE:   
			IsMin = FALSE; 
			First = TRUE;
			GetWindowRect(hWndDlg,&Rect);
			height = Rect.bottom - Rect.top;  
			width = 330;
			MoveWindow(hWndDlg, Rect.left,Rect.top,320+10,height, FALSE);  
			hwnd = GetDlgItem(hWndDlg,IDC_VIDEO);
			GetClientRect(hWndDlg,&Rect);
//			Rect.left = Rect.left - PRect.left;
//			Rect.top = Rect.top - PRect.top;
			MoveWindow(hwnd, Rect.left+2,Rect.top+2,320-1,240, FALSE);  
//			status = GVInit();
            VideoWnd = hwnd;
            VideoCntlWnd = hWndDlg;    
		    PostMessage(hWndDlg, WM_COMMAND, IDM_VAN_MEDIUM, 0L); 
			
} 
//else if (Message == WM_CLOSE)
//	CloseGEOSPANVideo ();


 if (VIDEO_CONTROLSMsgProc(hWndDlg, Message, wParam,lParam))
 	return (TRUE);
 switch(Message)
   {
	    case WM_PAINT: 
return DefWindowProc(hWndDlg,  Message,  wParam, lParam);
        	 
        case WM_WINDOWPOSCHANGED: 
	    	 GetWindowRect (hWndDlg,&Rect);
	    	 if (!EqualRect (&Rect,&LastRect))
	    	 {
	    	 	LastRect = Rect;
		     	PostMessage(hWndMain, WM_COMMAND, IDM_Z_REDRAW, 0L); 
		     }
		     return FALSE;
		     break;
		     
		case WM_GETMINMAXINFO:
		    lpmmi = (MINMAXINFO FAR*) lParam;
		    lpmmi->ptMaxTrackSize.x = width;
		    lpmmi->ptMaxTrackSize.y = height;  
		    return FALSE;
   		 	break;

	    case WM_SIZE:
	    {
	    	BOOL	rtn;
	    	  
	         switch(wParam)
	         {  
	         	case SIZE_MINIMIZED:
	         		IsMin=TRUE; 
	         		rtn = FALSE;
	         		break;
	         	case SIZE_MAXIMIZED:
	         		if (IsMin)
	         			IsMin=FALSE; 
	         		rtn = FALSE;
			 		break;
			 }
		}    
			return FALSE;
	    	 break;
	    	 
	    case WM_COMMAND: 
	    {
	         switch(wParam)
	         {  
	            case IDM_VAN_SMALL: 
	            	 VehLen = 35; 
	            	 VehSizeOpt = 1;
					 CheckMenuItem(GetMenu(hWndDlg), IDM_VAN_SMALL, MF_BYCOMMAND | MF_CHECKED);
					 CheckMenuItem(GetMenu(hWndDlg), IDM_VAN_MEDIUM, MF_BYCOMMAND | MF_UNCHECKED);
					 CheckMenuItem(GetMenu(hWndDlg), IDM_VAN_LARGE, MF_BYCOMMAND | MF_UNCHECKED);
	                 break;
	            	 
	            case IDM_VAN_MEDIUM: 
	            	 VehLen = 35 * 5;
	            	 VehSizeOpt = 2;
					 CheckMenuItem(GetMenu(hWndDlg), IDM_VAN_SMALL, MF_BYCOMMAND | MF_UNCHECKED);
					 CheckMenuItem(GetMenu(hWndDlg), IDM_VAN_MEDIUM, MF_BYCOMMAND | MF_CHECKED);
					 CheckMenuItem(GetMenu(hWndDlg), IDM_VAN_LARGE, MF_BYCOMMAND | MF_UNCHECKED);
	                 break;
	            	 
	            case IDM_VAN_LARGE: 
	            	 VehLen = 35 * 10;
	            	 VehSizeOpt = 3;
					 CheckMenuItem(GetMenu(hWndDlg), IDM_VAN_SMALL, MF_BYCOMMAND | MF_UNCHECKED);
					 CheckMenuItem(GetMenu(hWndDlg), IDM_VAN_MEDIUM, MF_BYCOMMAND | MF_UNCHECKED);
					 CheckMenuItem(GetMenu(hWndDlg), IDM_VAN_LARGE, MF_BYCOMMAND | MF_CHECKED);
	                 break;
	            	 
	         
	         	case IDM_FULL_SCREEN:
       				PostMessage(hWndMain, WM_COMMAND, IDM_FULL_SCREEN, 0L);  
       				break;   
       			case IDM_CLIP_IMAGE:
	         	    ClipImage();								
	         	    break;
       			case IDM_PRINT_IMAGE:
					hDIB = FrameToDIB (VideoWnd);
					GSinitAVI();
					ReOpenMovie(VideoWnd);
					SaveDIB(hDIB,"print.bmp");
				    DestroyDIB (hDIB);
	         	    PrintImage(hWndDlg,"print.bmp",1);								
	         	    break;
       			case IDM_SAVE_IMAGE:
            {    
            	 char SaveName[128], Ext[8], CurDir[128];
            	 char InitDir[128];
				 HDIB	hDIB;
				 RECT	VideoRect; 
				 LPRECT	pRect;  
				 int	SaveDrive;
            	 
            	 SetFilterString (IDS_FILTERBMP);
				 _getcwd (CurDir,128);
				 _getcwd (InitDir,128);
           	  	 SaveDrive = _getdrive();
				 _fstrcpy (Ext,".BMP");
				 EnableWindow (hWndMain,FALSE);
            	 if (GetSaveFileCD (hWndMain,SaveName,InitDir))
            	 { 
				 	if (!_fstrstr(SaveName,"."))
				 		_fstrcat(SaveName,Ext);
				 	if (!_fstrstr(SaveName,Ext))
				 	{  
				 	 	sprintf (SaveName,"Must have %s extension",Ext);
						MessageBox( GetFocus(), SaveName,"Invalid Name", MB_OK);
				 	 	goto RestoreDir;
				 	}
				 	if (wParam == IDM_SAVE_IMAGE)
				 	{
				 		pRect = &VideoRect;
						GetWindowRect(VideoWnd,pRect);
						hDIB = FrameToDIB (VideoWnd);
						GSinitAVI();
						ReOpenMovie(VideoWnd);
					}
				 	else                           
				 	{    
				 		SetViewport(1);
				 		pRect = &CurView->DrawRect; 
						ClientRectToScreenRect (hWndMain,pRect);
						hDIB = CopyScreenToDIB(pRect);
				 	}

					SaveDIB(hDIB,SaveName);
					DestroyDIB(hDIB);
            	 }
RestoreDir:      _chdir (CurDir);
            	 _chdrive (SaveDrive);
				 EnableWindow (hWndMain,TRUE);
            }
	         	    break;
	         	case IDC_DRIVE:
	         		break;
	         	case IDOK:
					DisplayVideo (GSTLID,GSFrame,GSView);
				    if (First)
				    	PostMessage(hWndMain, WM_COMMAND, IDM_Z_REDRAW, 0L); 
				    First = FALSE;
			 		break;
	            case IDCANCEL:
	                 /* Ignore data values entered into the controls        */
	                 /* and dismiss the dialog window returning FALSE       */
					 CloseGEOSPANVideo ();
	                 break;
			  }  
			  return FALSE;
			  break;
		}
/*		case WM_ERASEBKGND:
			if (!hBrushTrans) break;
			GetClientRect(hWndDlg, &Rect);
			SelectObject((HDC) wParam, hBrushTrans);
			FillRect((HDC) wParam, &Rect, hBrushTrans);
			break; 
		case WM_SIZE:
			i=LOWORD(lParam);
			i=HIWORD(lParam);
			break;
        case WM_LBUTTONDOWN:
	           if (RealView != 1 && RealView != 5) break;
	           GetClientRect (hWndDlg,&Rect);
	           x = LOWORD(lParam);
	           if (x > (Rect.left + Rect.right)/2)
	           		Left=FALSE;
	           else
	           		Left=TRUE;
	           NextTLID = GetTurnTLID (AddRefno,CurFrame,Left,FALSE);
	           if (NextTLID)
	           {
	   	           SetDlgItemText (VideoCntlWnd,IDC_DRIVE,"Continue");
		           PostMessage(VideoCntlWnd,WM_COMMAND, IDC_DRIVE, 0L);
               }

	        break;  */
		case WM_LBUTTONUP:
		       LDOWN = FALSE;
		    break;
		case WM_RBUTTONUP:
		       RDOWN = FALSE;
		    break;
        case WM_DESTROY:
			EndDialog(hWndDlg, FALSE);
			break;

        case WM_CLOSE:
            /* Closing the Dialog behaves the same as Cancel */
			/* delete the palette created at WM_CREATE time */
//			 fileCloseMovie(VideoWnd);		   
			CloseGEOSPANVideo ();
//	         EndDialog(hWndDlg, FALSE);
         break; /* End of WM_CLOSE                                      */



    default:  
    	return FALSE;
//        return DefWindowProc(hWndDlg,  Message,  wParam, lParam);
   }
 return TRUE;
} 
BOOL FAR PASCAL GEOSPANVIDEOMsgProc2(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
	int			TabStops[3]={50,100,150};
    BOOL		Opened;
	double		MP;
	int			st;
	char		str[256];
    
 int	BRtn;
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
         	case IDOK:
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
		  }
		  break;

    default:
        return FALSE;
   }
 return TRUE;
}
BOOL GoToNextSubFrame(int wDirection)
{
	return FALSE;
}
void EnableVideoControls (BOOL On)
{
	return;
} 
void TimeFrame (long Fram, LPSTR str)
{
	return;
}
DWORD WINAPI mciSendString2 (LPSTR lpstrCommand,
    LPSTR lpstrReturnString, UINT uReturnLength, HWND hwndCallback)
{ 
DWORD	rtn;  
rtn=mciSendString (lpstrCommand,
    lpstrReturnString, uReturnLength, hwndCallback);
    return rtn;
} 

void SetReplaceHouse(long House){
	return;
}
 
long GetNextTLID(long TLID, long CurFrame){
	return 0;
}

void AddFrameToList (long CurFrame){
	return;
}

void SaveDummyAddr (long CurFrame){
	return;
}
  
void DisplayAddList (long TLID, int OddEven){
	return;
}

void ReplaceActAddr (long CurFrame, long AddFrame, int Flags){
	return;
}

void DuplicateActAddr (long CurFrame){
	return;
}

BOOL MoveToNextStreet (long NextTLID,long CurTLID, long CurFrame)
{   int		stSeg, stInt, Street, PrevDriveDir, SaveLastArrow;
	long	Offset, IntLong, IntLat, Frame, fframe, tframe;
	BOOL	irc;
	char	CDRomToUse[64], PrevVdir;

	OpenAddressFiles (hWndMain);
    PrevDriveDir = DriveDir;
    if (BT_FIND (hSegData,(LPSTR)&CurTLID,BT_FIRST,BT_EQ,(LPSTR)&Offset))
    	return FALSE;
	ReadSegData (Offset, &Segdata); 
	if (!Segdata.Run[0])
		return FALSE;
    PrevVdir = Segdata.vdir;
	if (labs (CurFrame - Segdata.fframe) <
		labs (CurFrame - Segdata.tframe))
	{
   		IntLong = Segdata.frlong;
   		IntLat  = Segdata.frlat;
   	}
   	else
   	{
   		IntLong = Segdata.tolong;
   		IntLat = Segdata.tolat;
   	}
    if (BT_FIND (hSegData,(LPSTR)&NextTLID,BT_FIRST,BT_EQ,(LPSTR)&Offset))
    	return FALSE;
	ReadSegData (Offset, &Segdata);
	if (!Segdata.Run[0])
		return FALSE;
	if (Segdata.frlong == IntLong && Segdata.frlat == IntLat)
	{           
   		Frame = Segdata.fframe;
   		DriveDir = 1;
   	}
   	else
   	{
   		Frame = Segdata.tframe;
   		DriveDir = -1;
   	}
   	if (Segdata.vdir == '1') DriveDir = -DriveDir;
/*    if (DriveDir != PrevDriveDir)
    	LogicalView = OppositeView(LogicalView);  */
    RealView = GetLogicalView(LogicalView);
	if (Segdata.vdir == '1')
		TLIDView = OppositeView(RealView);
	else
		TLIDView = RealView;
    
    AddRefno = NextTLID;
    SaveLastArrow=LastArrow;   
    irc = DisplayVideo (AddRefno,Frame,RealView);
    LastArrow=SaveLastArrow;
	return(irc);
}

long GetNextRouteTLID (long AddRefno,long CurFrame)
{   
	HIGHLIGHTDATA	HighlightData;
	long	Refno;
	
	if (BT_FIND (hHighlight,(LPSTR)&AddRefno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData))
		return 0;
	if (BT_FIND (hHighlight2,(LPSTR)&HighlightData.Sequence,BT_FIRST,BT_GT,(LPSTR)&Refno))
	{ 
		if (BT_FIND (hHighlight2,(LPSTR)&HighlightData.Sequence,BT_FIRST,BT_ANY,(LPSTR)&Refno))
			Refno=0; 
	}
	return Refno;
}


              
/*
void fileCloseMovie(HWND hWnd);
BOOL fileOpenMovie(HWND hWnd, LPSTR Name);
void positionMovie(HWND hWnd);
void playMovie(HWND hWnd, WORD wDirection);
void seekMovie(HWND hWnd, WORD wAction);
void stepMovie(HWND hWnd, int wDirection);
long GetAVIFrame (void);
void SetAVIFrame (long Frame);
void FullScreenFrame(HWND hWnd, long Frame);
void GSFullScreenPlay(HWND hWnd, LPSTR Name);
void ReOpenMovie (HWND hWnd);
void SetAVIStatus (BOOL On);
void SetAVIFrameNotify (long Frame, HWND hWndNotify);
void DisplayCurrentFrame (void);
void PlayVideo(HWND hWnd, LPSTR Name);
void PlayVideoFrame0(HWND hWnd, LPSTR Name);
HANDLE FrameToDIB (HWND hWnd);
HANDLE FrameToDIB2 (LPSTR File, long Frame);
*/
