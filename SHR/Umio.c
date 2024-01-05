/* QuickCase:W KNB Version 1.00 */
typedef unsigned short int u_short;
typedef unsigned long u_long;
typedef	char *	caddr_t;
#include "graphint.h"   
#include <commctrl.h>

#define MAX_SOCKET_BUFFER_SIZE	4*USHRT_MAX-2
#define MAX_FT_SEG_SIZE			SHRT_MAX-512
#define MAX_PENDING_CONNECTS 8  /* The backlog allowed for listen() */
#define UMPort (u_short) 13760

#define MAX_THREADS	256
static	HANDLE hBGFileTranThread[MAX_THREADS]={0};
static  char LogFile[MAX_PATH] = { 0 };

#include "gmextern.h"

static	BOOL	InProcessTCPData=FALSE;
static	BOOL	BlockSocketInput=FALSE;
static	char	ErrMsg[128];  
static	BOOL	FirstSocketCall=TRUE,WinsockOpened=FALSE;
static	SOCKET	OpenSockets[MAXOPENSOCKETS] = { INVALID_SOCKET };
static	HWND	OpenSockethWnd[MAXOPENSOCKETS];
static	SOCKET	BlockedSocket[MAXOPENSOCKETS];  
static	int		SocketTimerDelay[MAXOPENSOCKETS];
static	HANDLE	hSocketProcessString[MAXOPENSOCKETS];
static	HANDLE	hSocketCloseString[MAXOPENSOCKETS];
static	HANDLE	hSocketRestartString[MAXOPENSOCKETS];
static	HANDLE	hSocketTimerString[MAXOPENSOCKETS];
static	HANDLE	hSocketBuffer[MAXOPENSOCKETS];    
static	long	LenSocketBuffer[MAXOPENSOCKETS]; 
static	USHORT	SocketFileTranLength[MAXOPENSOCKETS]; 
static	long	SocketFileTranID[MAXOPENSOCKETS]; 
static	char	SocketInputTerminator[MAXOPENSOCKETS][4];
static	short	LenSocketInputTerminator[MAXOPENSOCKETS];
static u_short	bufLen;
static struct	hostent*serverHostEnt;
static u_long	hostAddr;
static BOOL	ConnectBySIO=FALSE;
static SOCKET	UMSocket=INVALID_SOCKET;
static short	err;
static USHORT	ServerPort=0;
static struct	sockaddr_in serverSockAddr;
static struct	sockaddr_in mySockAddr; 
static SOCKET	WaitForInputOnSocket = -1;
static short	WaitForInputOnSocketStatus;    
static char		SocketInputBuffer[MAX_SOCKET_BUFFER_SIZE+2]; 
static int		NumBlockedSockets=0;
static BOOL		LogSocketIO[MAXOPENSOCKETS] = { 0 };
static char		Cmd[4096];     
static struct	{
					long	ID;
					char	ServerFile[256],
							LocalFile[256],
							TempFile[256],
							StatusMacro[4096],
							CompletionMacro[4096];
					DWORD	LastWriteTimeLow,
						 	LastWriteTimeHigh,
							FileLength,
							Loc; 
				}FTRecord;  
static HANDLE	hSegRec=0;
static short	SegLen;
static	HFILE	FidReplay=HFILE_ERROR;
static	char	ReplayHeader[128];
static	char	ReplayInput[4096];
static	char	ReplayOutput[1024];
static	int		ReplayOutputLen,ReplayInputLen;
static	int		MinReplayPos,CurReplayPos;
static	char	LogConnect[64];
static	char	Last32Commands[32][256];
static  char	Last32CommandsTime[32][128];
static	int		NumLogCommands=0;
static	int		NumConnections=0;
static	HWND	hWndVehTimer;


BOOL SaveFileTransferRecord (void);
void KillSocketTimer (SOCKET socket);
void SetSocketTimer (SOCKET socket);

int		ReadUM (LPSTR Message);
BOOL	WriteUM (LPSTR Message, int lmes);
BOOL ProcessFTSegment (SOCKET socket,long ID,LPBYTE pSeg,short SegLen);
HWND	TraceWnd2=0;

void DoTCPTrace (int InOut,LPSTR str,int len)
{
	HANDLE hStr = GSSiGlobAlloc (0,GHND,len*5+3);
	LPSTR  pStr = GlobalLock (hStr);
	int		i,j=2;
	char	str2[32];

	if (InOut)
		strcpy (pStr,"O ");
	else
		strcpy (pStr,"I ");
	for (i=0;i<len;i++)
	{
		if (str[i] > 31 && str[i] < 127)
			pStr[j++] = str[i];
		else
		{
			sprintf (str2,"|%i|",(int)str[i]);
			strcat (pStr,str2);
			j += strlen (str2);
		}
	}
	pStr[j] = 0;
	SetWindowText(hWndMain,pStr);
	GSSiGlobUlFree (&hStr);
	Sleep (500);
	return;
}

BOOL PaintServerInfo (HDC hDC,LPRECT pRect)
{
	int	x,y,i;
	SIZE	txsize;
	HFONT	hFont, OldFont;
	char	str[128];

	if (!InServerMode)
		return FALSE;

//    memset(&ps, 0x00, sizeof(PAINTSTRUCT));
//    hDC = BeginPaint(hWnd, &ps);
	SetDisplayMode (hDC,GF_TEXTMODE); 

	FillRect (hDC,pRect,GetStockObject (WHITE_BRUSH));
	x = 2;
	y = 24;

    OldFont = SelectObject(hDC, GetStockObject(SYSTEM_FONT));
	SetTextColor (hDC,0);

	GetGlobalCVal ("[SN]",str,"GeoMaster Server");

	ExtTextOut (hDC,x,y,0,0,str,strlen(str),0);
	GetTextExtentPoint32(hDC,str,strlen(str),&txsize);
	y += txsize.cy*3 + 2;
	ExtTextOut (hDC,x,y,0,0,LogConnect,strlen(LogConnect),0);
	GetTextExtentPoint32(hDC,LogConnect,strlen(LogConnect),&txsize);
	y += txsize.cy + 2;
	sprintf (str,"Number of connections: %i",NumConnections);
	ExtTextOut (hDC,x,y,0,0,str,strlen(str),0);
	GetTextExtentPoint32(hDC,str,strlen(str),&txsize);
	y += txsize.cy*3 + 2;

	for (i=NumLogCommands-1;i>=0;i--)
	{
		char str[512];

		sprintf(str, "%s: %s", Last32CommandsTime[i], Last32Commands[i]);
		ExtTextOut (hDC,x,y,0,0,str,strlen(str),0);
		GetTextExtentPoint32(hDC, str, strlen(str), &txsize);
		y += txsize.cy + 2;
	}

    SelectObject (hDC,OldFont); 
   // EndPaint(hWnd, &ps);  

	return TRUE;
}

void LogServerActivity (LPSTR Mess)
{
	static	BOOL	First=TRUE;
	char	ServerLogFile[MAX_PATH];
	HDC		hDCScreen;
	RECT	rect;
	char	TimeAndDate[128] = "$CAL([%SYS_CLOCK])";

	if (!InServerMode)
		return;

	ExpandText(TimeAndDate);
	if (First)
	{
		memset (Last32Commands,0,sizeof(Last32Commands));
		strcpy (LogConnect,Mess);
		First = FALSE;
	}
	else
	{
		if (NumLogCommands == 32)
		{
			memmove(Last32Commands[0], Last32Commands[1], 31 * 256);
			memmove(Last32CommandsTime[0], Last32CommandsTime[1], 31 * 128);
			NumLogCommands--;
		}
		strcpy(Last32CommandsTime[NumLogCommands], TimeAndDate);
		strncpy0 (Last32Commands[NumLogCommands++],Mess,255);
	}
	if (GetGlobalCVal ("[%SERVERACTIVITYLOGFILE]",ServerLogFile,0))
	{
		AppendFile2 (ServerLogFile,TimeAndDate);
		AppendFile2 (ServerLogFile,Mess);
	}
	hDCScreen = GetDC (hWndMain);
	GetClientRect(hWndMain,&rect);
	PaintServerInfo (hDCScreen,&rect);
	ReleaseDC (hWndMain,hDCScreen);
//	InvalidateRect (hWndMain,0,TRUE);
	return;
}
void RepaintServerInfo(void)
{
	HDC hDCScreen = GetDC(hWndMain);
	RECT rect;

	GetClientRect(hWndMain, &rect);
	PaintServerInfo(hDCScreen, &rect);
	ReleaseDC(hWndMain, hDCScreen);
	return;
}
BOOL GetReplayHeader (void)
{
	 if (FidReplay == HFILE_ERROR || !fgetstring (ReplayHeader,127,FidReplay))
	 {
		 GSSiClose2 (&FidReplay);
		 FidReplay = HFILE_ERROR;
		 *ReplayHeader = 'E';
		 return FALSE;
	 }
	 else
		 return TRUE;
}

BOOL OpenTCPReplay (LPSTR ReplayFile)
{
	LPSTR	pLen;

	if (!ReplayFile)
	{
		if (FidReplay != HFILE_ERROR)
			GSSiClose2 (&FidReplay);
		FidReplay = HFILE_ERROR;
		ReplayTCP = FALSE;
		return TRUE;
	}
	FidReplay = GSSiOpenFile (ReplayFile,0,OF_READ);
	ReplayTCP = TRUE;
	if (FidReplay != HFILE_ERROR)
	{
		if (GetReplayHeader ())
		{
			if ((pLen = strrchr (ReplayHeader,'|')))
			{
				ReplayOutputLen = atoi (pLen+1);
				BigRead (FidReplay,ReplayOutput,ReplayOutputLen+2);
			}
		}
	}
	return ReplayTCP;
}

BOOL StartVehTimeMenu (HWND hWnd)
{
	if (!ReplayTCP)
		return FALSE;
	CreateDialog(hInst, (LPSTR)"VEHICLE_REPLAY", hWnd,(DLGPROC) VEHICLE_TIMEMsgProc);
	hWndVehTimer = hWndMain;
	return TRUE;
}

int SetReplayPosition (int pos,HFILE FidReplay2)
{
	LPSTR		nRead;
	char	str[256];
	HFILE	Fid = FidReplay;

	if (FidReplay2 != HFILE_ERROR)
		Fid = FidReplay2;
	pos = max (pos,MinReplayPos);
	GSSillseek (Fid,pos,0);
	*str = 0;
	nRead =	fgetstring (str,254,Fid);

	while (nRead &&
			((FidReplay2 != HFILE_ERROR && strncmp (str,">Plot:",6)) ||
			 (FidReplay2 == HFILE_ERROR && strncmp (str,"I(0): ",6))))
	{
		pos = GSSillseek (Fid,0,1);
		nRead = fgetstring (str,254,Fid);
	}
	if (nRead)
	{
		strcpy (ReplayHeader,str);
		pos = GSSillseek (Fid,0,1);
		if (FidReplay2 != HFILE_ERROR || fgetstring (str,254,Fid))
		{
			if (!strnicmp (str,">Plot:",6))
			{
				LPSTR pEnd,pLoc = strchr (str,',');

				if (FidReplay2 != HFILE_ERROR)
					SetGlobalValue ("%TCPINPUT",str);
				
				if (pLoc)
				{
					pLoc++;
					if ((pEnd = strchr (pLoc,',')))
						*pEnd = 0;
					if (hWndVehTime)
						SetDlgItemText (hWndVehTime,IDC_VEHTIME,pLoc);
				}
			}
		}
		GSSillseek (Fid,pos,0);
		return pos;
	}
	return 0;
}

BOOL FAR PASCAL VEHICLE_TIMEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
	BOOL	Err,ii;
	RECT	Rect;
	char	str[260];
	static	int	lineinc,pageinc,lastCurReplayPos;
	static	char	saveHistDir[MAX_PATH];

 switch(Message)
   {
    case WM_INITDIALOG:
		{
			int	top,left;
			POINT	MidPoint;

			hWndVehTime = hWndDlg;
			hWndVehTimer = hWndDlg;
			ProcessText ("$MACRO([%DL]macros\\HistoryReplay.txt,START)");
			lastCurReplayPos = 0;
			ShowWindow (hWndVehHist,SW_HIDE);
			SetDlgItemInt (hWndDlg,IDC_REPLAYDELAY,(1000-ReplayDelay)/10,FALSE);
			GetWindowRect(hWndDlg,&Rect);
			MidPoint = RectMid (&CurView->ScreenRect);
			top = MidPoint.y - RECTHEIGHT(&Rect)/2;
			left = MidPoint.x - RECTWIDTH(&Rect)/2;
			*saveHistDir = 0;
			if (VehReplayFid != HFILE_ERROR)
			{
				FidReplay = VehReplayFid;
				MinReplayPos = CurReplayPos = 0;
				GetGlobalCVal ("%VEHHISTDIR",saveHistDir,0);
				strcat (saveHistDir,"x");
				SetGlobalValue ("%VEHHISTDIR",saveHistDir);
				*LastChr (saveHistDir) = 0;
			}
 			//MoveWindow(hWndDlg,left,top,RECTWIDTH(&Rect),RECTHEIGHT(&Rect),TRUE);
			if (FidReplay != HFILE_ERROR)
			{
				int	range = GSSifilelength (FidReplay);
				int	nRows = NumRowsInTxtFile (FidReplay);

				pageinc = range/10;
				if (nRows)
					lineinc = 1.2 * range / nRows;
				else
					lineinc = range/100;
				MinReplayPos = GSSillseek (FidReplay,0,1);
				SetScrollRange(GetDlgItem(hWndDlg,IDC_SCROLLTIME),SB_CTL, 0, range, FALSE);  
			}
			SendMessage(GetDlgItem (hWndDlg,IDC_SPEEDSLIDER), TBM_SETRANGE,(WPARAM) TRUE,(LPARAM) MAKELONG(1, 10)); 
        
			SendMessage(GetDlgItem (hWndDlg,IDC_SPEEDSLIDER), TBM_SETPOS, 
				(WPARAM) TRUE,                   // redraw flag 
				(LPARAM) 5); 
			ReplayDelay = 500;
			if (VehReplayFid != HFILE_ERROR)
			{
				ClearVehicleHistory (INT_MAX);
				RedisplayViewports (TRUE);
				HaveReplayTimer = SetTimer(hWndVehTimer, TCPREPLAYTIMER,max(1,ReplayDelay), (TIMERPROC) NULL);
			}
			UpdateAllVehicles(TRUE);
		}
        break; /* End of WM_INITDIALOG                                 */

	case WM_TIMER:
		if ((CurReplayPos = SetReplayPosition (CurReplayPos,VehReplayFid)) && hSocketProcessString[0])
		{ 
			HANDLE	hTmp;
			LPSTR	pTmp, pCommand;
			int		lTmp;

			if (CurReplayPos < lastCurReplayPos)
			{
				ClearVehicleHistory (INT_MAX);
				UpdateAllVehicles(TRUE);
			}
			lastCurReplayPos = CurReplayPos;
			SetScrollPos(GetDlgItem(hWndDlg,IDC_SCROLLTIME), SB_CTL,CurReplayPos, TRUE);
			pCommand = GlobalLock (hSocketProcessString[0]);
			lTmp = strlen (pCommand);
			hTmp = GSSiGlobAlloc (0,GMEM_MOVEABLE,lTmp+1);
			pTmp = GlobalLock (hTmp);
			strcpy (pTmp,pCommand);
			GlobalUnlock (hSocketProcessString[0]);
			ProcessText (pTmp);
			GSSiGlobUlFree (&hTmp);
		}
		if (ReplayDelay == INT_MAX)
		{
			KillTimer (hWndVehTimer,HaveReplayTimer);
			HaveReplayTimer = 0;
		}
		break;

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */
	case WM_DESTROY:
		GSSiClose2 (&FidReplay);
		if (HaveReplayTimer)
			KillTimer (hWndVehTimer,HaveReplayTimer);
		HaveReplayTimer = 0;
		break;
	case WM_NOTIFY:
		{
			LPNMHDR	pnmh;

			pnmh = (LPNMHDR) lParam; 
			switch (wParam)
			{

				case IDC_SPEEDSLIDER:
				{
					int ival = SendMessage(GetDlgItem(hWndDlg,IDC_SPEEDSLIDER), TBM_GETPOS, 0, 0); 
					
					if (HaveReplayTimer)
						KillTimer (hWndVehTimer,HaveReplayTimer);
					ReplayDelay = 1000 - ival*100;
					HaveReplayTimer = SetTimer(hWndVehTimer, TCPREPLAYTIMER, max(1,ReplayDelay), (TIMERPROC) NULL);
				}
					break;
			}
		}
		break;
	case WM_VSCROLL:
		ii=1;
		break;

	case WM_HSCROLL:
	   {
		int nScrollCode = (int) LOWORD(wParam);  // scroll bar value 
		int	nPos = (short int) HIWORD(wParam);   // scroll box position 
		SCROLLINFO	si;
		HWND	hWnd = GetDlgItem (hWndDlg,IDC_SCROLLTIME);

		if ((HWND)lParam == GetDlgItem (hWndDlg,IDC_SPEEDSLIDER))
		{
			int ival = SendMessage(GetDlgItem(hWndDlg,IDC_SPEEDSLIDER), TBM_GETPOS, 0, 0); 

			if (HaveReplayTimer)
				KillTimer (hWndVehTimer,HaveReplayTimer);
			ReplayDelay = 1000 - ival*100;
			HaveReplayTimer = SetTimer(hWndVehTimer, TCPREPLAYTIMER, max(1,ReplayDelay), (TIMERPROC) NULL);
			break;
		}
//		if (HaveReplayTimer)
//			KillTimer (hWndVehTimer,HaveReplayTimer);
//		HaveReplayTimer = 0;
		switch (nScrollCode)
		{
		  case SB_LINEDOWN: 
			//CurReplayPos += lineinc;
		  break;

		  case SB_LINEUP:
			CurReplayPos -= lineinc*2;
	  	  break;

		  case SB_PAGEDOWN:
			CurReplayPos += pageinc;
	  	  break;

		  case SB_PAGEUP:
			CurReplayPos -= pageinc;
	  	  break;

		  case SB_THUMBTRACK:
		  case SB_THUMBPOSITION:
			CurReplayPos = nPos;
		  break;  
		  
/*		  case SB_THUMBTRACK:
			ZeroMemory(&si, sizeof(SCROLLINFO));
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_TRACKPOS;
 
            if (GetScrollInfo((HWND)lParam, SB_CTL, &si) )
			  CurReplayPos = si.nTrackPos;
			  break;
		  case SB_ENDSCROLL:
			 // CurReplayPos = GetScrollPos ((HWND)lParam,SB_CTL);
			ZeroMemory(&si, sizeof(SCROLLINFO));
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_ALL;
 
 
            if (GetScrollInfo((HWND)lParam, SB_CTL, &si) )
			  CurReplayPos = si.nTrackPos;
			CurReplayPos = SetReplayPosition (CurReplayPos,VehReplayFid);
			if (ReplayDelay == INT_MAX)
				HaveReplayTimer = SetTimer(hWndVehTimer, TCPREPLAYTIMER,1, (TIMERPROC) NULL);
			else
				HaveReplayTimer = SetTimer(hWndVehTimer, TCPREPLAYTIMER,max(1,ReplayDelay), (TIMERPROC) NULL);
			SetScrollPos((HWND)lParam, SB_CTL,CurReplayPos, TRUE);
			return TRUE;
			break;
*/
		  default:
			  return TRUE;
		}
//		CurReplayPos = SetReplayPosition (CurReplayPos,VehReplayFid);
//		SetScrollPos((HWND)lParam, SB_CTL,CurReplayPos, TRUE);
		if (ReplayDelay == INT_MAX)
		{
			if (HaveReplayTimer)
				KillTimer (hWndVehTimer,HaveReplayTimer);
			HaveReplayTimer = SetTimer(hWndVehTimer, TCPREPLAYTIMER,1, (TIMERPROC) NULL);
		}
	   }
	   break;
    case WM_COMMAND:
         switch(LOWORD(wParam))
         { 
			case  IDC_SPEEDSLIDER:
				ii=1;
				break;

            case IDCANCEL: 
				if (*saveHistDir)
					SetGlobalValue ("%VEHHISTDIR",saveHistDir);
				VehicleInReplayList (0);

				hWndVehTime = 0;
				ProcessText ("$MACRO([%DL]macros\\HistoryReplay.txt,STOP)");
				ShowWindow (hWndVehHist,SW_SHOW);
				RedisplayViewports (FALSE);
                DestroyWindow (hWndDlg);
            break; 

//			case IDC_SPIN1:
//			case IDC_REPLAYDELAY:
//				ReplayDelay = 1000 - GetDlgItemInt (hWndDlg,IDC_REPLAYDELAY,&Err,FALSE)*10;
//				break;
            case IDC_VEHSTOPSTART: 
            {
				if (HaveReplayTimer)
					KillTimer (hWndVehTimer,HaveReplayTimer);
				HaveReplayTimer = 0;
				if (ReplayDelay == LONG_MAX)
				{
					int ival = SendMessage(GetDlgItem(hWndDlg,IDC_SPEEDSLIDER), TBM_GETPOS, 0, 0); 
					ReplayDelay = 1000 - ival*100;
					SetDlgItemText (hWndDlg,IDC_VEHSTOPSTART,"Stop");
					//ReplayDelay = 1000 - GetDlgItemInt (hWndDlg,IDC_REPLAYDELAY,&Err,FALSE)*10;
					HaveReplayTimer = SetTimer(hWndVehTimer, TCPREPLAYTIMER, max(1,ReplayDelay), (TIMERPROC) NULL);
				}
				else
				{
					SetDlgItemText (hWndDlg,IDC_VEHSTOPSTART,"Start");
					ReplayDelay = LONG_MAX;
				}
        		break;
         	}
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

int GetTCPInput (SOCKET sock,LPSTR Buffer,int maxlen,int flags)
{
	int	err = 0;
	LPSTR	pLen;
	int	ii;
	
Top:
	if (ReplayTCP)
	{
		if (*ReplayHeader == 'O')
		{
			if ((pLen = strrchr (ReplayHeader,'|')))
			{
				ReplayOutputLen = atoi (pLen+1);
				BigRead (FidReplay,ReplayOutput,ReplayOutputLen+2);
			}
			if (!strnicmp (ReplayOutput,">Ping:",6) || !strnicmp (ReplayOutput,">Pong:",6))
			{
				if (GetReplayHeader ())
					goto Top;
			}
			err = SOCKET_ERROR;
			WSASetLastError (WSAEWOULDBLOCK);
			*ReplayHeader = 'E';
		}
		else if (*ReplayHeader == 'E')
		{
			err = SOCKET_ERROR;
			WSASetLastError (WSAEWOULDBLOCK);
		}
		else if ((pLen = strrchr (ReplayHeader,'|')))
		{
			ReplayInputLen = atoi (pLen+1);
			BigRead (FidReplay,ReplayInput,ReplayInputLen+2);
			if (ReplayInputLen > maxlen)
				ii=1;
			else
			{
				if (!strnicmp (ReplayInput,">Ping:",6) || !strnicmp (ReplayInput,">Pong:",6))
				{
					if (GetReplayHeader ())
						goto Top;
				}
				memcpy (Buffer,ReplayInput,ReplayInputLen);
				if (!strnicmp (Buffer,">Plot:",6))
					ii=1;
				err = ReplayInputLen;
				GetReplayHeader ();
Next:
				if (*ReplayHeader == 'I')
				{
					if (HaveReplayTimer)
						KillTimer (hWndVehTimer,HaveReplayTimer);
					HaveReplayTimer = SetTimer(hWndVehTimer, TCPREPLAYTIMER, max(1,ReplayDelay), (TIMERPROC) NULL);
				}
				else if (*ReplayHeader == 'O')
				{
					if ((pLen = strrchr (ReplayHeader,'|')))
					{
						ReplayOutputLen = atoi (pLen+1);
						BigRead (FidReplay,ReplayOutput,ReplayOutputLen+2);
						*ReplayHeader = 'E';
					}
					if (!strnicmp (ReplayOutput,">Ping:",6) || !strnicmp (ReplayOutput,">Pong:",6))
					{
						if (GetReplayHeader ())
							goto Next;
					}
				}
			}
		}
		if (hWndVehTime)
		{
			CurReplayPos = GSSillseek (FidReplay,0,1);
			SetScrollPos(GetDlgItem(hWndVehTime,IDC_SCROLLTIME),SB_CTL,CurReplayPos , TRUE); 
		}
	}
	else
		err = recv (sock,Buffer,maxlen,flags);
	return err;
}

void ProcessReplayTCPTimer (void)
{
	PostMessage(hWndMain, GF_TCPIPMESSAGE, (WPARAM)FidReplay, MAKELPARAM (FD_READ,0)); 
	KillTimer (hWndMain,TCPREPLAYTIMER);
	return;
}

int SendTCPOutput (SOCKET sock,LPSTR pRec,int len,int flags)
{
	int err = 0;
	LPSTR	pLen;

    if (UMIODebug)
		DoTCPTrace (1,pRec,len);

	if (ReplayTCP)
	{
		if (!strnicmp (pRec,">Subscribe:",11))
			err=0;
		if (!strnicmp (pRec,">Ping:",6) || !strnicmp (pRec,">Pong:",6))
			err = len;
		else if (!strncmp (pRec,ReplayOutput,len))
		{
			GetReplayHeader ();
			if (*ReplayHeader == 'I')
			{
				if (HaveReplayTimer)
					KillTimer (hWndMain,HaveReplayTimer);
				HaveReplayTimer = SetTimer(hWndMain, TCPREPLAYTIMER, max(1,ReplayDelay), (TIMERPROC) NULL);
			}
		}
	}
	else
		err = send (sock,pRec,len,flags);
	return err;
}

void SetLogSocketIO (BOOL set)
{
	int		i;
	
	OpenWinSock ();
	i = GetSocketID (CurrentServerSocket);
	if (i < 0)
	{
		for (i=0;i<MAXOPENSOCKETS;i++)
			LogSocketIO[i] = set;
	}
	else
		LogSocketIO[i] = set;
			
	return;
}

BOOL RestartSocket (SOCKET sock)
{
	int	i = GetSocketID (sock);

	if (InServerMode)
		return FALSE;
	if (i >= 0 && hSocketRestartString[i])
	{
		LPSTR pRestartString = GlobalLock (hSocketRestartString[i]);
    	int lMacro = _fstrlen (pRestartString);   
    	if (lMacro)
    	{
    		HANDLE	hMacro = GSSiGlobAlloc (0,GMEM_MOVEABLE,lMacro+1);   
    		LPSTR	pMacro = GlobalLock (hMacro);
    		_fstrcpy (pMacro,pRestartString);  
			GlobalUnlock (hSocketRestartString[i]);
    		GlobalUnlock (hMacro);  
			CloseWinSock (FALSE);
			Sleep (2000);
			OpenWinSock ();
			PostMessage(hWndMain, WM_COMMAND, IDM_PROCESSTEXT, (LPARAM)hMacro); 
			return TRUE;
		}
		else
			GlobalUnlock (hSocketRestartString[i]);
	}
	return FALSE;
}

void DisplayLenSocketBuffer (int i)
{
	char	str[32]=":  ";
	HDC		hDC;
	RECT	Rect={0,0,46,18};
	//char	block[16]=" ";
	int		ln;

	if (FirstSocketCall)
		return; 
	hDC = GetDC (hWndMain);
//	if (BlockSocketInput)
//		itoa (BlockSocketInput,block,10);
//	SaveDC (hDC);
//	SetBkMode (hDC,TRANSPARENT);
//	SetBkColor (hDC,RGB(255,255,255));
	if (BlockSocketInput)
		//strcpy (str,"B");
		sprintf (str,":B%i",BlockSocketInput);
	else if (i > -1)
		sprintf (str,":%i",LenSocketBuffer[i]);
/*	if (i > -1)
		ln = LenSocketBuffer[i];
	else
		ln = 0;
	sprintf (str,"%s|%i|%i|%i|%i|%i",block,InDisplayProcessing,InVehicleDisplay,BlockVehicleDisplay,HavePendingDisplay,ln);*/
//	SelectClipRgn (hDC,0);
//	FillRect (hDC,&Rect,GetStockObject (LTGRAY_BRUSH));
	if (i > -2)
		BackgroundUpdateMessage (str);
	else
		BackgroundUpdateMessage (":        ");

//		TextOut (hDC,0,0,str,strlen(str));
//	RestoreDC (hDC,-1);
	ReleaseDC (hWndMain,hDC);
	return;
}

BOOL BlockSocketProcessing (BOOL Block)
{
	int	i;

	if (InServerMode)
		return FALSE;
	if (!Block)
	{
		if (BlockSocketInput)
		{
			BlockSocketInput = FALSE;
			TraceWnd2 = 0;
			for (i=0;i<NumBlockedSockets;i++)
				ProcessTCPData (BlockedSocket[i]);
		}
		else
		{
			for (i=0;i<MAXOPENSOCKETS;i++)
			{
				if (OpenSockets[i] != INVALID_SOCKET)
					ProcessTCPData (OpenSockets[i]);
			}
		}
		NumBlockedSockets = 0;
		DisplayLenSocketBuffer (-2);
	}
	else
	{
		BlockSocketInput = Block;
		DisplayLenSocketBuffer (-1);
		if (Block == 4)
			TraceWnd2 = TraceWnd;
	}
	return TRUE;
}

int GetSocketID (SOCKET sock)
{
	int i;
	
	if (sock != INVALID_SOCKET)
		for (i=0;i<MAXOPENSOCKETS;i++)
		{
			if (OpenSockets[i] == sock)
				return i;
		}
	return -1;
}

BOOL SetupSocketTimer (SOCKET socket,long Seconds,LPSTR TimerString)
{
	int		l;  
	int		i = GetSocketID (socket);
	LPSTR	pString;
	
	if (FirstSocketCall)
		return FALSE;  
	if (socket == INVALID_SOCKET)
		return FALSE;
	if (i < 0)
		return FALSE;  
	if (Seconds <= 0)
	{
		GSSiGlobFree (&hSocketTimerString[i]);
		KillSocketTimer (socket);
	}
	else
	{
		SocketTimerDelay[i] = Seconds * 1000;    
		l = _fstrlen (TimerString);
		GSSiGlobFree (&hSocketTimerString[i]);
		hSocketTimerString[i] = GSSiGlobAlloc (1738,GMEM_MOVEABLE,l+1);
		pString = GlobalLock (hSocketTimerString[i]); 
		_fstrcpy (pString,TimerString);
		GlobalUnlock (hSocketTimerString[i]);   
		SetSocketTimer (socket);	
	}
	return TRUE;
} 

BOOL SetupSocketTerminator (SOCKET socket,LPSTR Term)
{
	int		l;  
	int		i = GetSocketID (socket);
	LPSTR	pString;
	
	if (FirstSocketCall)
		return FALSE;  
	if (socket == INVALID_SOCKET)
		return FALSE;
	if (i < 0)
		return FALSE;   
	if (!stricmp (Term,"NULL"))
	{
		strcpy (SocketInputTerminator[i],"");   
		LenSocketInputTerminator[i] = 1;
	}
	else
	{
		strcpy (SocketInputTerminator[i],"\r\n");   
		LenSocketInputTerminator[i] = 2;
	}
	return TRUE;
} 

void ProcessSocketTimer (int TimerID)
{   
	int	i=TimerID-TCPTIMER-1;
	
	if (InDisplayProcessing || ReplayTCP)
		return;
	if (i < 0 || i >= MAXOPENSOCKETS || OpenSockets[i] == INVALID_SOCKET)
		return;
	if (hSocketTimerString[i])
	{
		LPSTR pString = GlobalLock (hSocketTimerString[i]); 
		
		_fstrcpy (Cmd,pString);
		GlobalUnlock (hSocketTimerString[i]);   
        ProcessText (Cmd);
	}
	ProcessTCPData (OpenSockets[i]);
	return;
}

void KillSocketTimer (SOCKET socket)
{
	int	TimerID, i;

	if (FirstSocketCall)
		return;  
	if (socket == INVALID_SOCKET)
		return;
	for (i=0;i<MAXOPENSOCKETS;i++)
	{
		if (OpenSockets[i] == socket)
			goto GotID;
	}
	return;   
GotID:
	TimerID = TCPTIMER + i + 1;
	KillTimer (OpenSockethWnd[i],TimerID);
	return;
}

void SetSocketTimer (SOCKET socket)
{
	short	i, err;    
	int		TimerID;
	
	if (FirstSocketCall)
		return;  
	if (socket == INVALID_SOCKET)
		return;
	for (i=0;i<MAXOPENSOCKETS;i++)
	{
		if (OpenSockets[i] == socket)
			goto GotID;
	}
	return;   
GotID:
	TimerID = TCPTIMER + i + 1;
	if (OpenSockethWnd[i] && SocketTimerDelay[i])
	{
		KillSocketTimer (socket);
		SetTimer(OpenSockethWnd[i], TimerID, SocketTimerDelay[i], (TIMERPROC) NULL);
	}
	return;
}

void LogSocketError (LPSTR Error,LPSTR Input)
{   
	char	TimeAndDate[128]="$CAL([%SYS_CLOCK])";
	
	ExpandText (TimeAndDate);
	AppendFile2 ("$DIRPATH(ALLUSERAPPDATA,GeoMaster)\\gmsocketerrors.txt",TimeAndDate);
	AppendFile2 ("$DIRPATH(ALLUSERAPPDATA,GeoMaster)\\gmsocketerrors.txt",Error);
	if (strlen(Input) > 1020)
		Input[1020] = 0;
	AppendFile2 ("$DIRPATH(ALLUSERAPPDATA,GeoMaster)\\gmsocketerrors.txt",Input);
	return;
}

void ProcessSocketError (SOCKET sock,int err,LPSTR Info)
{   
	char	mess[256],title[]="Communication lost with server";
	int	i = GetSocketID (sock);
	static	BOOL	IgnoreErrors=FALSE;
	
	sprintf (mess,"Message number %i\r\n%s",err,Info);
	LogSocketError (title,mess);   
//	FirstSocketCall = TRUE;
	if (InServerMode && (err == WSAECONNRESET || err == WSAECONNABORTED))
	{
		if (!RestartSocket (sock))
			CloseTCPIPSocket (sock,TRUE);
		return;
	}
	if (err == WSAECONNRESET || err == WSAECONNABORTED || err == WSAENOTSOCK)
	{
Restart:
			if (i >= 0 && hSocketRestartString[i])
			{
				LPSTR pRestartString = GlobalLock (hSocketRestartString[i]);
    			int lMacro = _fstrlen (pRestartString);   
    			if (lMacro)
    			{
    				HANDLE	hMacro = GSSiGlobAlloc (0,GMEM_MOVEABLE,lMacro+1);   
    				LPSTR	pMacro = GlobalLock (hMacro);
    				_fstrcpy (pMacro,pRestartString);  
					GlobalUnlock (hSocketRestartString[i]);
    				GlobalUnlock (hMacro);  
					CloseWinSock (FALSE);
					Sleep (5000);
					OpenWinSock ();
					PostMessage(hWndMain, WM_COMMAND, IDM_PROCESSTEXT, (LPARAM)hMacro); 
					LogSocketError ("Restart issued","");   
					return;
				}
				else
					GlobalUnlock (hSocketRestartString[i]);
			}
	}
	CloseWinSock (FALSE);
	OpenWinSock ();
	if (!IgnoreErrors)
	{
		if (GetGlobalBVal2 ("%AutoTCPRestart",FALSE))
		{
			Wait2 (1000);
			goto Restart;
		}
		else switch (MessageBox (0,mess,title,MB_ICONEXCLAMATION|MB_ABORTRETRYIGNORE))
		{
		case IDRETRY:
			goto Restart;
		case IDABORT:
			BlowOut(0,0);
		case IDIGNORE:
			IgnoreErrors = TRUE;
			break;
		}
	}
	return;
}

void LogSocketInput (int i,LPSTR Input,int len)
{   
	
	LogServerActivity (Input);
 
	if (LogSocketIO[i])
	{	
		char	Header[64];

		sprintf (Header,"I(%i): $CAL([%%SYS_CLOCK])|%i",i,len);
		ExpandText (Header);
		Input[len]=0;
		for (i = 0; i < len; i++)
		{
			if (!Input[i])
				Input[i] = '~';
		}
		if (!*LogFile)
			GetGlobalCVal("%SOCKETLOGFILE", LogFile, "$DIRPATH(ALLUSERAPPDATA,GeoMaster)\\socketlog.txt");
		AppendFile2 (LogFile,Header);
		AppendFile2 (LogFile,Input);
	}
	return;
}  

void LogSocketOutput (int i,LPSTR Input,int len)
{

	if (LogSocketIO[i])
	{	
		char	Header[64];

		sprintf (Header,"O(%i): $CAL([%%SYS_CLOCK])|%i",i,len);
		ExpandText (Header);
		Input[len]=0;
		for (i=0;i<len;i++)
			if (!Input[i])
				Input[i] = '~';
		if (!*LogFile)
			GetGlobalCVal("%SOCKETLOGFILE", LogFile, "$DIRPATH(ALLUSERAPPDATA,GeoMaster)\\socketlog.txt");
		AppendFile2(LogFile, Header);
		AppendFile2(LogFile, Input);
	}
	return;
}  

void ResetSocketLog(void)
{
	GetGlobalCVal("%SOCKETLOGFILE", LogFile, "$DIRPATH(ALLUSERAPPDATA,GeoMaster)\\socketlog.txt");
}

BOOL GetMyIPNetAddress (LPSTR inetAddr)
{
	char	HostName[128];
	struct hostent	hent;
	WORD	wVersionRequested = 256 + 1;
	WSADATA	wsaData;
	ULONG	localAddr;
	LPBYTE	ptr, pLocAddr;

    if (!WinsockOpened)
		if (WSAStartup(wVersionRequested,&wsaData))
			return FALSE;
	gethostname (HostName,128);
    localAddr = inet_addr( HostName );
	hent = *gethostbyname (HostName);
	pLocAddr = inet_ntoa (*(struct in_addr*)*hent.h_addr_list);
	strcpy (inetAddr,pLocAddr);
	if (!WinsockOpened)
		WSACleanup();
	return TRUE;
}

void InitSockets()
{
	for (int i = 0; i < MAXOPENSOCKETS; i++)
	{
		OpenSockets[i] = INVALID_SOCKET;
		hSocketProcessString[i] = 0;
		hSocketCloseString[i] = 0;
		hSocketRestartString[i] = 0;
		hSocketTimerString[i] = 0;
		hSocketBuffer[i] = 0;
		BlockedSocket[i] = 0;
	}

}
BOOL OpenWinSock (void)
{
	WORD	wVersionRequested = 256 + 1;
	WSADATA	wsaData;
	short	i;
    
    if (WinsockOpened)
    	return TRUE;
    err =WSAStartup(wVersionRequested,&wsaData);
    if (err)
    {
		return FALSE;
	}

    WinsockOpened = TRUE;
	if (FirstSocketCall)
	{
		FirstSocketCall = FALSE;
		NumBlockedSockets=0;
		InitSockets();
	} 
	WSASetLastError (0);
    return TRUE;
}

void CloseWinSock (BOOL ProcessCloseString)
{   
	UINT	i;
	
	if (!FirstSocketCall)
		for (i=0;i<MAXOPENSOCKETS;i++)
		{
			if (OpenSockets[i] != INVALID_SOCKET)
				CloseTCPIPSocket (OpenSockets[i],ProcessCloseString);
		}
	if (WinsockOpened)
		WSACleanup();
	WinsockOpened = FALSE;
	return;
}

int WaitCursor (int iWait)
{   static int state = 0;
	int	laststate; 
	static HCURSOR	hcurSave; 
	HCURSOR	hCursor;
    
    
	state += iWait;
	state = max (state,0);
	if (!state)
	{
		if (iWait<0) 
		{ 
			GSSiSetCursor (hcurSave);
			SetCurs (hcurSave,FALSE);
			hCursor=0;
		}
	}
	else if (state==1)
	{
		if (iWait>0)  
		{
			hCursor = LoadCursor (NULL,IDC_WAIT);
			hcurSave = GSSiSetCursor (hCursor);  
			SetCurs (hCursor,TRUE);
		}
	}
     return (state);
}

SOCKET connectToServer ( char *serverName, u_short port,BOOL UpdateServerOpen, LPINT perr )
{
	SOCKET toServerSocket;
	int		err;
	char	str[32];

    _fmemset ( &serverSockAddr,0, sizeof(serverSockAddr));

    hostAddr = inet_addr( serverName );

    if( (long)hostAddr != INADDR_NONE)
     { /* we've got an address */
/*       _fmemmove( &serverSockAddr.sin_addr,&hostAddr,  sizeof(hostAddr));*/
			serverSockAddr.sin_addr.S_un.S_addr = hostAddr;
     } else
     { /* ask the host database/name server for host entry */
       serverHostEnt = gethostbyname( serverName );
       if( NULL == serverHostEnt )
       {
    	GSSiMsgBox( GetFocus(), "Cannot find Server","Fatal Error", MB_OK,0);
		return (INVALID_SOCKET);
       }
       /* copy address from host entry to socket structure */
       _fmemmove( &serverSockAddr.sin_addr, serverHostEnt->h_addr,
              serverHostEnt->h_length);
    } /* else */

    serverSockAddr.sin_family = PF_INET;
    serverSockAddr.sin_port = htons( port );

    /* Create a socket */
    toServerSocket = socket( PF_INET, SOCK_STREAM, 0);
    if (toServerSocket == INVALID_SOCKET)
    {	err = WSAGetLastError();
        if (err)
	    {
			wsprintf(ErrMsg, "Error Code: %d ",err);
	    	GSSiMsgBox( GetFocus(), ErrMsg,
	    				"Unable to Allocate Socket", MB_OK,0); 
	    	WSASetLastError (0);
			return (INVALID_SOCKET);
		}
    }
//	WaitCursor (1);
	err =connect( toServerSocket, (LPSOCKADDR)&serverSockAddr,sizeof(serverSockAddr) );
//	WaitCursor (-1);
    if (err == SOCKET_ERROR)
    {	err = WSAGetLastError(); 
    	WSASetLastError (0);
        if (perr)  
        	*perr = err;
        else
	    {
			wsprintf(ErrMsg, "Error Code: %d ",err);
	    	GSSiMsgBox( GetFocus(), ErrMsg,
	    				"Unable to Connect to Server", MB_OK,0);
		}
    	closesocket (toServerSocket);
		return (-1);
    } 
	if (UpdateServerOpen)
	{
		err = recv (toServerSocket,str,4,0);
		if (err == SOCKET_ERROR)
		{
			err = WSAGetLastError(); 
    		WSASetLastError (0);
			err = 0;
		}
		if (!err)
			toServerSocket = INVALID_SOCKET;
	}
	if (UMIODebug) SetWindowText(hWndMain,"Connected to server");
    return toServerSocket;
} /* connectToServer */

short CheckForRegistrationServer (HWND hWnd,LPSTR SerialNum)
{   
	int		err;
	short	rtn=0;
	
	_fstrcpy (SerialNumber,SerialNum);
	{
      DLGPROC lpfnREGCONNECTMsgProc; 
      short	nRc;
		
      lpfnREGCONNECTMsgProc = MakeProcInstance((DLGPROC)REGCONNECTMsgProc, hInst);
      nRc = DialogBox(hInst, (LPSTR)"REGCONNECT", hWnd, lpfnREGCONNECTMsgProc);
      FreeProcInstance(lpfnREGCONNECTMsgProc); 
      if (!nRc)
      	return 2; 
      if (nRc == 2)
      	return 1;  
      if (nRc == 3 || nRc == 4)
      	return nRc;
	} 
	return 0;
}


BOOL OpenTCPIPServer (LPSTR ServerIP,USHORT port)
{   
	char	str[256];
	sprintf (str,"%s : %ld",ServerIP,(long)port);
//	MessageBox (0,str,0,MB_OK);
	_fstrcpy(CurrentIPAddress, ServerIP);
	ServerPort = port;
	return TRUE;
}

BOOL OpenTCPIPServer2 (HWND hWnd)
{   
	short	i, err;    
	SOCKET	sock;
	long	l;
	u_long myAddr;
	SOCKET ServerSocket; 
	char	str[256]; 
	USHORT	port = ServerPort;
	
	if (!port)
		return FALSE;
	if (!OpenWinSock ()) 
	{
    	GSSiMsgBox( GetFocus(), "Cannot find Windows Socket Interface","Error", MB_OK,0);
		return FALSE;
	} 
    _fmemset ( &mySockAddr,0, sizeof(mySockAddr));

	myAddr = inet_addr(CurrentIPAddress);

    if( (long)myAddr != INADDR_NONE)
     { /* we've got an address */
/*       _fmemmove( &serverSockAddr.sin_addr,&hostAddr,  sizeof(hostAddr));*/
			mySockAddr.sin_addr.S_un.S_addr = myAddr;
     }
     else
     { 	GSSiMsgBox( GetFocus(), "Invalid Server IP Address","Fatal Error", MB_OK,0);
		return FALSE;
     }

    mySockAddr.sin_family = PF_INET;
    mySockAddr.sin_port = htons( port );

    /* Create a socket */
    ServerSocket = socket( PF_INET, SOCK_STREAM, 0);
    if (ServerSocket == INVALID_SOCKET)
    {	err = WSAGetLastError();
        if (err)
	    {   
	    	WSASetLastError (0);
			wsprintf(str, "Error Code: %d ",err);
	    	GSSiMsgBox( GetFocus(), str,"Unable to Allocate Socket", MB_ICONEXCLAMATION,0);
			return FALSE;
		}
    }
	if (bind(ServerSocket, (struct sockaddr FAR *) &mySockAddr, sizeof(mySockAddr)) == SOCKET_ERROR) {
		sprintf(str, "%d is the error", WSAGetLastError());
		
		WSASetLastError (0);
		GSSiMsgBox(GetFocus(),str, "bind(sock) failed", MB_ICONEXCLAMATION,0);
    	closesocket (ServerSocket);
		return FALSE;
	}
	err = WSAAsyncSelect (ServerSocket,hWnd,GF_TCPIPMESSAGE,FD_ACCEPT);
    if (err == SOCKET_ERROR)
    {	err = WSAGetLastError();
        if (err)
	    {   
	    	WSASetLastError (0);
			wsprintf(str, "Error Code: %d ",err);
	    	GSSiMsgBox( GetFocus(), str,
	    				"Call to AsyncSelect failed", MB_ICONEXCLAMATION,0);
	    	closesocket (ServerSocket);
			return FALSE;
		}
    } 
    err =listen( ServerSocket, MAX_PENDING_CONNECTS );
    if (err == SOCKET_ERROR)
    {	err = WSAGetLastError();
        if (err)
	    {
			wsprintf(str, "Error Code: %d ",err);
	    	GSSiMsgBox( GetFocus(), str,
	    				"Call to listen failed", MB_ICONEXCLAMATION,0);
	    	closesocket (ServerSocket);
			return FALSE;
		}
    } 
	for (i=0;i<MAXOPENSOCKETS;i++)
	{
		if (OpenSockets[i] == INVALID_SOCKET)
			goto GotID;
	}
	GSSiMsgBox( GetFocus(), "No available sockets",
				"Call to listen failed", MB_ICONEXCLAMATION,0);
	return FALSE;
GotID:
	OpenSockets[i] = ServerSocket;	
	OpenSockethWnd[i] = 0;//keeps timer from being created
	sprintf(str, "Server opened on %s port %i", CurrentIPAddress, port);
	LogServerActivity (str);
	return TRUE;
}

BOOL AcceptTCPConnection (HWND hWnd,SOCKET sock)
{   
	short	i;
	char	mess[64];
	
	for (i=0;i<MAXOPENSOCKETS;i++)
	{
		if (OpenSockets[i] == INVALID_SOCKET)
			goto GotID;
	}
	GSSiMsgBox( GetFocus(), "No available sockets",
				"Call to accept failed", MB_ICONEXCLAMATION,0);
	return FALSE;
GotID:	
	OpenSockets[i] = accept( sock,NULL,NULL);
	if (OpenSockets[i] == INVALID_SOCKET)
    	return FALSE;
	if (!AllowingSocketConnections)
	{
		err = shutdown (OpenSockets[i],2);
		err = closesocket (OpenSockets[i]);
		OpenSockets[i] = INVALID_SOCKET;
		return FALSE;
	}

	err = WSAAsyncSelect (OpenSockets[i],hWnd,GF_TCPIPMESSAGE,FD_READ|FD_CLOSE);
	hSocketBuffer[i] = GSSiGlobAlloc (0,GHND,MAX_SOCKET_BUFFER_SIZE);  
	if (UpdateServer)
	{
		LenSocketInputTerminator[i] = 1;
		strcpy (SocketInputTerminator[i],"");  
		send (OpenSockets[i],"ACPT",4,0); //verify acceptence
	}
	else
	{
		LenSocketInputTerminator[i] = GetGlobalLVal2 ("[%ServerInputTerminatorLen]",0);
		GetGlobalCVal ("[%ServerInputTerminator]",SocketInputTerminator[i],"");  
	}
	NumConnections++;
	sprintf (mess,"Open accepted on socket %i",(int)OpenSockets[i]);
	LogServerActivity (mess);
	if (UpdateServer)
	{
		err = send (OpenSockets[i],"Connected to GeoMaster Update Server",36,0);
		if (err == SOCKET_ERROR)
		{
    		err = WSAGetLastError();   
    		WSASetLastError (0); 
			if (err == WSAEWOULDBLOCK)
				err = 0;
		}
	}

	return TRUE;
}	

int OpenTCPIPSocket (HWND hWnd,LPSTR TCPAddress,USHORT Port,SOCKET *pSocket,LPSTR InputTerminator,LPSTR InputProcessString,LPSTR CloseProcessString,LPSTR RestartProcessString,BOOL Notify)
{   
	int	i, err;    
	SOCKET	sock;
	long	l;
	static	BOOL	First=TRUE;
	char	mess[64];
	
	if (!OpenWinSock ())
		return -1;
	for (i=0;i<MAXOPENSOCKETS;i++)
	{
		if (OpenSockets[i] == INVALID_SOCKET)
			goto GotID;
	}
	GSSiMsgBox( GetFocus(), "No available sockets",
				"Call to open failed", MB_ICONEXCLAMATION,0);
	return FALSE;
GotID:	
	if (ReplayTCP && First)
		sock = FidReplay;
	else
		sock = connectToServer (TCPAddress,Port,FALSE,&err);
	First = FALSE;
	if (sock == INVALID_SOCKET)
		return -max (1,err); 
	*pSocket = sock;
	OpenSockets[i] = sock;
	SocketTimerDelay[i] = 0;
	OpenSockethWnd[i] = hWnd;
	if (Notify)
		err = WSAAsyncSelect (sock,hWnd,GF_TCPIPMESSAGE,FD_READ|FD_CLOSE);
	hSocketBuffer[i] = GSSiGlobAlloc (0,GHND,MAX_SOCKET_BUFFER_SIZE);  
	_fstrcpy (SocketInputTerminator[i],InputTerminator);   
	LenSocketInputTerminator[i] = _fstrlen (SocketInputTerminator[i]);
	if (InputProcessString)
	{   
		LPSTR	pString;
		
		l=_fstrlen (InputProcessString);
		if (l)
		{
			hSocketProcessString[i] = GSSiGlobAlloc (1749,GMEM_MOVEABLE,l+1);
			pString = GlobalLock (hSocketProcessString[i]); 
			_fstrcpy (pString,InputProcessString);
			GlobalUnlock (hSocketProcessString[i]);   
		}
	}
	if (CloseProcessString)
	{   
		LPSTR	pString;
		
		l=_fstrlen (CloseProcessString);
		if (l)
		{
			hSocketCloseString[i] = GSSiGlobAlloc (1750,GMEM_MOVEABLE,l+1);
			pString = GlobalLock (hSocketCloseString[i]); 
			_fstrcpy (pString,CloseProcessString);
			GlobalUnlock (hSocketCloseString[i]);   
		}
	}
	if (RestartProcessString)
	{   
		LPSTR	pString;
		
		l=_fstrlen (RestartProcessString);
		if (l)
		{
			hSocketRestartString[i] = GSSiGlobAlloc (1751,GMEM_MOVEABLE,l+1);
			pString = GlobalLock (hSocketRestartString[i]); 
			_fstrcpy (pString,RestartProcessString);
			GlobalUnlock (hSocketRestartString[i]);   
		}
	}
	sprintf (mess,"Socket %i opened",(int)sock);
	LogServerActivity (mess);
	return TRUE;
}

short SendTCPIPString (SOCKET socket,LPSTR string,BOOL SendNullTerm,UINT WaitSeconds)
{    
	int	l=_fstrlen (string), err=0, rtn=1,ii;
	int	attempt=0;

	if (FirstSocketCall)
		return FALSE; 
	if (GetSocketID (socket) < 0)
		return FALSE;
	if (SendNullTerm)
		l++;
Again:
	if (l)
		err = SendTCPOutput (socket,string,l,0); 
	if (err == SOCKET_ERROR)
    {
    	err = WSAGetLastError();   
    	WSASetLastError (0); 
		if (err == WSAEWOULDBLOCK && !attempt)
		{
			attempt++;
			Sleep (2000);
			goto Again;
		}
    	ProcessSocketError (socket,err,"Sending TCPIP string");
    	return FALSE;
    }
    if (err != l)
    	ii=1;
    if (l)
    	LogSocketOutput (GetSocketID (socket),string,l);
	if (WaitSeconds)
	{   
		MSG	msg;
		
		rtn = 0;                               
 		SetTimer (hWndMain,TCPTIMER,WaitSeconds*1000,NULL); 
 		WaitForInputOnSocket = socket;
 		WaitForInputOnSocketStatus = 0;
		while (ContinueProcessing && !rtn && !WaitForInputOnSocketStatus && GSSiGetMessage(&msg,0,0,0))
		{   
			switch (msg.message)
			{   
				case WM_TIMER:
					if (msg.wParam == TCPTIMER)
						rtn = 2;
					break;
					
				case GF_TCPIPMESSAGE:
					TranslateMessage(&msg);
					DispatchMessage(&msg);
					break;
				
				case WM_KEYDOWN:
					if (msg.wParam == 27) 
						rtn = 3; 
					break; 
					 
				default: 
					if (PrintMsgWnd && IsDialogMessage(PrintMsgWnd, &msg)) 
					{ 
					}  
					break;
			}
		} 
		if (!rtn && WaitForInputOnSocketStatus)
			rtn = 1;
		WaitForInputOnSocket = -1;
		KillSocketTimer (socket);
	}
	else
		SetSocketTimer (socket);	
	return rtn;
}
 
BOOL CloseTCPIPSocket (SOCKET socket,BOOL ProcessCloseString)
{   
	short	i, n, err, TimerID;
	char	mess[64];
	
	if (FirstSocketCall)
		return FALSE;  
	if (socket == INVALID_SOCKET)
		return FALSE;
	i = GetSocketID (socket);
	if (i < 0)
		return FALSE;
	KillSocketTimer (socket);
	GSSiGlobFree (&hSocketProcessString[i]);
	if (ProcessCloseString && hSocketCloseString[i])
	{	
		LPSTR	pCommand = GlobalLock (hSocketCloseString[i]);
		ProcessText (pCommand);
		GlobalUnlock (hSocketCloseString[i]);
	}
	err = shutdown (socket,2);
	err = closesocket (socket);
	OpenSockets[i] = INVALID_SOCKET;  
	GSSiGlobFree (&hSocketCloseString[i]);
	GSSiGlobFree (&hSocketRestartString[i]);
	GSSiGlobFree (&hSocketTimerString[i]);
	GSSiGlobFree (&hSocketBuffer[i]);
	LenSocketBuffer[i] = 0; 
	SocketFileTranLength[i] = 0;
	n = NumBlockedSockets;
	NumBlockedSockets = 0;
	for (i=0;i<n;i++)
		if (BlockedSocket[i] != socket)
			BlockedSocket[NumBlockedSockets++] = socket;
	if (!PeopleNet)
		CloseAllRequestedFiles(FALSE);  
	NumConnections--;
	sprintf (mess,"Socket %i closed",(int)socket);
	LogServerActivity (mess);
	return TRUE;
}  
void __cdecl BackgroundFileSend (LPHANDLE phArgs)
{
    char    File[MAX_PATH];
	char	str[MAX_PATH+2];
	HANDLE	Fid;
	LPSTR	arg1=GlobalLock (*phArgs);
	LPSTR	arg2 = arg1 + MAX_PATH, arg3 = arg2 + MAX_PATH, arg4 = arg3 + MAX_PATH;
	OFSTRUCTGM	OFStruct = { 0 };
	int		iThread, ier;
	long	startLoc, lenFile, totRead=0;
	SOCKET	sock;
	long	lenBuf;
	LPSTR	pBuf = malloc (SHRT_MAX);
	u_long	iMode=0;

	strcpy (File,arg1);
	startLoc = atoi (arg2);
	sock = (SOCKET)atoi (arg3);
	iThread = atoi (arg4);
	GSSiGlobUlFree (phArgs);
	Fid = OpenFileGM (File,&OFStruct,OF_READ);
	lenFile = llFileSeek (Fid,0,2);
	llFileSeek (Fid,startLoc,0);
	while (totRead < lenFile && ContinueBackgroundCache && (lenBuf = BigRead64 (Fid,pBuf,SHRT_MAX))>0)
	{
		LPBYTE pCompressedRec = malloc (lenBuf+1024);
		int	   lCompressedRec = CompressBinaryRecord (pBuf,pCompressedRec+sizeof(int),lenBuf);

		*(LPINT)pCompressedRec = lenBuf;
		lCompressedRec += sizeof(int);
		if (send (sock,(LPSTR)&lCompressedRec,4,0) != 4)
		{
			free (pCompressedRec);
			break;
		}
		if (send (sock,pCompressedRec,lCompressedRec,0) != lCompressedRec)
		{
			free (pCompressedRec);
			break;
		}
		ReadNTBlock (sock,str,6);
		if (strncmp (str,"GOT IT",6))
		{
			free (pCompressedRec);
			break;
		}
		totRead += lenBuf;
		free (pCompressedRec);
	}
	GSSiClose64 (&Fid);
	free (pBuf);
	if (totRead == lenFile)
		Fid = OpenFileGM (File,&OFStruct,OF_DELETE);
	CloseTCPIPSocket (sock,FALSE);
	hBGFileTranThread[iThread] = 0;
	return;
}
BOOL StartBackgroundFileSend (LPSTR FilePath,LONGLONG StartLoc,SOCKET sock)
{
	static	HANDLE hArgs;
	LPSTR	arg1,arg2,arg3,arg4;//filepath,startloc,socket,iThread
	BOOL	rtn = FALSE;
	int		iThread, ier;
	u_long	iMode=0;

	
	hArgs = GSSiGlobAlloc (9999,GMEM_MOVEABLE,MAX_PATH*4);
	arg1=GlobalLock (hArgs);
	arg2=arg1+MAX_PATH;
	arg3=arg2+MAX_PATH;
	arg4=arg3+MAX_PATH;

	strcpy (arg1,FilePath);
	_i64toa (StartLoc,arg2,10);
	itoa ((int)sock,arg3,10);
	for (iThread=0;iThread<MAX_THREADS;iThread++)
		if (!hBGFileTranThread[iThread])
			goto HaveThread;
	MessageBox (0,"Max threads reached",0,MB_ICONEXCLAMATION);
	GSSiGlobUlFree (&hArgs);
	return FALSE;

HaveThread:
	itoa (iThread,arg4,10);
	GlobalUnlock (hArgs);
	ier = WSAAsyncSelect (sock,hWndMain,0,0);
	if (ier == SOCKET_ERROR)
	{	
		int	err = WSAGetLastError();
		WSASetLastError (0);
	}
	ier = ioctlsocket (sock,FIONBIO,&iMode);
	if (ier == SOCKET_ERROR)
	{	
		int	err = WSAGetLastError();
		WSASetLastError (0);
	}
	hBGFileTranThread[iThread] = (HANDLE)_beginthread(BackgroundFileSend, 0, &hArgs);
	SetThreadPriority (hBGFileTranThread[iThread],THREAD_PRIORITY_LOWEST);
	rtn = TRUE;
	return rtn;
}
BOOL ProcessUpdateServerRequest (SOCKET sock,int ln,LPSTR SocketInputBuffer)
{
	LPINT plBuf = (LPINT)SocketInputBuffer;
	LPINT plastUp = plBuf + 1;
	LPINT pnFiles = plastUp + 1;
	LPINT plFile = pnFiles + 1;
	LPSTR pFile = (LPSTR)(plFile + 1);
	LPSTR pLoc;
	char	fileName[MAX_PATH];
	char	msg[128];
	int	i, netCheckPointID, ierr=0;
	BOOL	rtn=FALSE;
	HFILE	FidNetTransfer;
	time_t	lTime = time (0);
	char	netTransferFile[MAX_PATH];
	LONGLONG	lenFile;
	long	lnMsg, iEnd=-1;
	int		version=1, lnc;

	GetGlobalCVal ("[%LASTNETUPDATE]",msg,0);
	lnc = strlen (msg)+1;
	sprintf (netTransferFile,"[%%DL]NetTransferFiles\\%I64i.bin",lTime);
	ExpandText (netTransferFile);
	FidNetTransfer = GSSiOpenFile (netTransferFile,0,OF_CREATE);
	BigWrite (FidNetTransfer,&lenFile,sizeof(LONGLONG),-1);
	BigWrite (FidNetTransfer,&version,sizeof(int),-1);
	BigWrite (FidNetTransfer,&lnc,sizeof(int),-1);
	BigWrite (FidNetTransfer,msg,lnc,-1);

	for (i=0; i < *pnFiles; i++)
	{
		strcpy (fileName,pFile);
		plFile = (LPINT)(pFile + *plFile);
		pFile = (LPSTR)(plFile + 1);
		pLoc = strstr (fileName,".gmd(");
		if (pLoc)
		{
			pLoc += 4;
			*pLoc++ = 0;
			netCheckPointID = atoi (pLoc);
			switch (UpdateGMDFromCheckPointLog_net (netCheckPointID,FidNetTransfer, fileName))
			{
			case 0://updated OK
			case 3://no update needed
				rtn = TRUE;
				break;
			case 1://can't find cpl file
				ierr=1;
				goto Failed;
			case 2://file too old for update
				ierr=2;
				goto Failed;
			case 10://non checkpointed gmd file
				ierr=10;
				break;
			}
		}
	}
	BigWrite (FidNetTransfer,&iEnd,4,-1);
	lenFile = GSSillseek (FidNetTransfer,0,2);
	GSSillseek (FidNetTransfer,0,0);
	BigWrite (FidNetTransfer,&lenFile,sizeof(LONGLONG),-1);
	GSSiClose2 (&FidNetTransfer);
	sprintf (msg,"OK:%s|%lli",netTransferFile,lenFile);
	lnMsg = strlen (msg) + 1;
	if (send (sock,(LPSTR)&lnMsg,4,0) == 4)
	{
		if (send (sock,msg,lnMsg,0) == lnMsg)
			rtn = StartBackgroundFileSend (netTransferFile,0,sock);
	}
	return rtn;
Failed:
	GSSiClose2 (&FidNetTransfer);
	GSSiRemove (netTransferFile);
	sprintf (msg,"ERROR:%i",ierr);
	lnMsg = strlen (msg) + 1;
	if (send (sock,(LPSTR)&lnMsg,4,0) == 4)
		send (sock,msg,lnMsg,0);
	return FALSE;
}

BOOL ProcessTCPData (SOCKET sock)
#if ENABLETRACE
{GSSiEnterProg (1399);
#endif
{
	int		i,err,inc;     
	LPSTR	pCommand, pBuffer, pTerm;  
	long	Dummy;
	
	if (InProcessTCPData)
{
#if ENABLETRACE
GSSiExitProg (1399);
#endif
		return FALSE;
}
	InProcessTCPData = TRUE;
	if (FirstSocketCall)
		goto RtnFalse;
	i = GetSocketID (sock);
	if (i<0)
		goto RtnFalse;
	if (BlockSocketInput)
	{
		for (i=0;i<NumBlockedSockets;i++)
			if (sock == BlockedSocket[i])
				goto Top;
/*			{
				recv (OpenSockets[i],(LPSTR)&Dummy,4,MSG_PEEK);//re-enable messages
				goto RtnFalse;
			}*/
		BlockedSocket[NumBlockedSockets++] = sock;
/*		recv (OpenSockets[i],(LPSTR)&Dummy,4,MSG_PEEK);//re-enable messages
		goto RtnFalse;*/
	}
Top:
	i = GetSocketID (sock);
	if (i<0)
		goto RtnFalse;
	if (!hSocketBuffer[i])
		goto RtnFalse;
	pBuffer = GlobalLock (hSocketBuffer[i]); 
	if (LenSocketInputTerminator[i] && _fstrlen (pBuffer) < LenSocketBuffer[i])
	{
		LogSocketError ("NULL value in non-null terminated buffer",pBuffer);
		LenSocketBuffer[i] = _fstrlen (pBuffer);
		DisplayLenSocketBuffer (i);
	}
	err = GetTCPInput (OpenSockets[i],SocketInputBuffer,MAX_SOCKET_BUFFER_SIZE,0);
	if (err == SOCKET_ERROR)    
	{
		err = WSAGetLastError();
		WSASetLastError (0);	
		if (err == WSAEWOULDBLOCK)
			err = 0;
		else
		{
			GlobalUnlock (hSocketBuffer[i]);
			ProcessSocketError (sock,err,"In ProcessTCPData-1");
			goto RtnFalse;
		}
	}
	if (err && UpdateServer)
	{
		ProcessUpdateServerRequest (OpenSockets[i],err,SocketInputBuffer);
		GlobalUnlock (hSocketBuffer[i]);
		goto RtnFalse;
	}
	SetSocketTimer (OpenSockets[i]);	
/*	if (!err)
	{
		GlobalUnlock (hSocketBuffer[i]);
		goto RtnFalse;  
	}*/
	if (LenSocketBuffer[i]+err > (long)MAX_SOCKET_BUFFER_SIZE)  
	{
		LogSocketError ("Buffer overrun",SocketInputBuffer);
		LenSocketBuffer[i] = 0;
	}
	if (!SocketFileTranLength[i] && (LenSocketInputTerminator[i]&& _fstrlen (SocketInputBuffer)+1 < err))
	{
		if (*SocketInputTerminator[i]) 
		{
			LogSocketError ("NULL value in non-null terminated input",SocketInputBuffer);
			err = _fstrlen (SocketInputBuffer); 
		}
	}
	if (err)
	{
		memmove (&pBuffer[LenSocketBuffer[i]],SocketInputBuffer,err);
		*&pBuffer[LenSocketBuffer[i]+err] = 0; 
		SocketInputBuffer[err] = 0;
		LogSocketInput (i,SocketInputBuffer,err);     
		LenSocketBuffer[i]+=err; 
		DisplayLenSocketBuffer (i);
	}
	if (!BlockSocketInput && !InDisplayProcessing && LenSocketBuffer[i])
	{   
		if (SocketFileTranLength[i])
		{   
			short l =  SocketFileTranLength[i];
			
			if (LenSocketBuffer[i] < SocketFileTranLength[i])
				goto RtnTrue;
			SocketFileTranLength[i] = 0;
			ProcessFTSegment (sock,SocketFileTranID[i],pBuffer,l);   
			LenSocketBuffer[i] -= l;   
			DisplayLenSocketBuffer (i);
			pTerm = pBuffer + l;
			if (LenSocketBuffer[i]>0) 
				_fmemmove (pBuffer,pTerm,(int)LenSocketBuffer[i]);  
			goto RtnTrue;
		} 
		if (*SocketInputTerminator[i])
			pTerm=_fstrstr(pBuffer,SocketInputTerminator[i]);
		else
			pTerm = _fstrchr (pBuffer,0); 
		if (!pTerm || pTerm > &pBuffer[LenSocketBuffer[i]])
			goto RtnTrue;
		{
			char	SaveChr = *(pTerm);  
			short	lRec;
			
			*pTerm = 0;    
			lRec = _fstrlen (pBuffer);  
			_fstrcpy (Cmd,pBuffer);
//			if (!LenSocketInputTerminator[i])
//				lRec++;
			*pTerm = SaveChr;
			if (sock == WaitForInputOnSocket)
				WaitForInputOnSocketStatus = 1;
			inc = max (1,LenSocketInputTerminator[i]);
			pTerm += inc;
			LenSocketBuffer[i]-=(lRec + inc);
			DisplayLenSocketBuffer (i);
			
			if (LenSocketBuffer[i]>0) 
				_fmemmove (pBuffer,pTerm,(int)LenSocketBuffer[i]); 
			else
				LenSocketBuffer[i]=0;
			*&pBuffer[LenSocketBuffer[i]] = 0;
			CurrentServerSocket = sock;
			if (hSocketProcessString[i])
			{ 
				HANDLE	hTmp;
				LPSTR	pTmp;
				int		lTmp;

			    if (UMIODebug)
					DoTCPTrace (0,Cmd,strlen(Cmd));
				SetGlobalValue ("%TCPINPUT",Cmd);
				pCommand = GlobalLock (hSocketProcessString[i]);
				lTmp = strlen (pCommand);
				hTmp = GSSiGlobAlloc (0,GMEM_MOVEABLE,lTmp+1);
				pTmp = GlobalLock (hTmp);
				strcpy (pTmp,pCommand);
				GlobalUnlock (hSocketProcessString[i]);
				GlobalUnlock (hSocketBuffer[i]);
				ProcessText (pTmp);
				GSSiGlobUlFree (&hTmp);
				if (!hSocketBuffer[i])//socket restarted
					goto RtnFalse;
				pBuffer = GlobalLock (hSocketBuffer[i]); 
			}
			else if (hCmdMess) 
			{   
        		LPSTR CmdMess = GlobalLock (hCmdMess);
        		short	lCmd = strlen (Cmd);
        		
				*CmdMess = 0;  
				GlobalUnlock (hCmdMess);
				{
					HANDLE	hCmd = GSSiGlobAlloc (0,GMEM_MOVEABLE,lCmd+1);   
					LPSTR	pCmd = GlobalLock ((HANDLE)hCmd);

					strcpy (pCmd,Cmd);  
    				GlobalUnlock (hCmd);  
					PostMessage(hWndMain, GF_PROCESSTCPCMD, (WPARAM)sock, (LPARAM)hCmd); 
				}
			} 
		}
	}
	if (!BlockSocketInput)
	{
		if (!InDisplayProcessing && LenSocketBuffer[i])
			PostMessage(hWndMain, GF_PROCESSTCPCMD, (WPARAM)sock, 0); 
		else
		{
			if (BlockVehicleDisplay > 1)
			{
				BlockVehicleDisplay = 0;
				UpdateVehicleStatusDlg ();
			}
		}
	}
RtnTrue:
	*&pBuffer[LenSocketBuffer[i]] = 0;
	GlobalUnlock (hSocketBuffer[i]);
//	if (ContinueProcessing) 
//		goto Top;
	InProcessTCPData = FALSE;
{
#if ENABLETRACE
GSSiExitProg (1399);
#endif
		return TRUE;
}
RtnFalse:  
	
	SetContinueProcessing ( TRUE);
	InProcessTCPData = FALSE;
{
#if ENABLETRACE
GSSiExitProg (1399);
#endif
		return FALSE;
}
#if ENABLETRACE
}
#endif
}

BOOL ProcessTCPCmd (SOCKET sock,LPSTR Cmd)
{
	LPSTR	CmdMess;
	int		lCmd;
	int		i = GetSocketID (sock);

	CmdMess = GlobalLock (hCmdMess);
	*CmdMess = 0;
	GlobalUnlock (hCmdMess);
	if (*Cmd == '$')
		ProcessText (Cmd); 
	else
	{
		CloseTCPIPSocket(sock, FALSE);
		LogSocketError("Invalid Input", Cmd);
		return FALSE;
	}
	CmdMess = GlobalLock (hCmdMess); 
	lCmd=_fstrlen(CmdMess);
	if (!*SocketInputTerminator[i])
		lCmd++;
	if (lCmd)
	{
		err = SendTCPOutput (OpenSockets[i],CmdMess,lCmd,0); 
		LogSocketOutput (i,CmdMess,lCmd);
	}
	if (hSegRec)
	{   
		LPBYTE	pRec = GlobalLock (hSegRec);
		
		err = SendTCPOutput (OpenSockets[i],pRec,SegLen,0);  
		GSSiGlobUlFree (&hSegRec);
	}
	GlobalUnlock (hCmdMess);
	return TRUE;
}

long GetNewFileTransferRequest (LPSTR ServerFile,LPSTR LocalFile,LPSTR StatusMacro,LPSTR CompletionMacro)
{
	char	FTRecFile[256];   
	HFILE	Fid;   
	long	NextID;  
	LPSTR	pDot;
	
	_fstrcpy (FTRecFile,"[%DL]FTDir\\next.bin"); 
	if (ExistFile (FTRecFile)) 
	{
		Fid = GSSiOpenFile (FTRecFile,NULL,OF_READWRITE);
		BigRead (Fid,(HPSTR)&NextID,4); 
	}
	else
	{
		Fid = GSSiOpenFile (FTRecFile,NULL,OF_CREATE);
		NextID = 0;
	}
	NextID++; 
	GSSillseek (Fid,0,0);
	BigWrite (Fid,(HPSTR)&NextID,4,-1);
	GSSiClose2 (&Fid);  
	_fmemset (&FTRecord,0,sizeof(FTRecord));
	FTRecord.ID = NextID;
	_fstrcpy (FTRecord.ServerFile,ServerFile);
	_fstrcpy (FTRecord.LocalFile,LocalFile);
	_fstrcpy (FTRecord.TempFile,LocalFile);
	if ((pDot = _fstrrchr (FTRecord.TempFile,'.')))
		_fstrcpy (pDot,".ftf");
	else
		_fstrcat (FTRecord.TempFile,".ftf");
	_fstrcpy (FTRecord.StatusMacro,StatusMacro);
	_fstrcpy (FTRecord.CompletionMacro,CompletionMacro);
	SaveFileTransferRecord ();
	return NextID;
}

BOOL LoadFileTransferRecord (long ID)
{   
	char	FTRecFile[256];   
	BOOL	rtn=FALSE;  
	HFILE	Fid;
	
	sprintf (FTRecFile,"[%%DL]FTDir\\%ld.bin",ID);
	Fid = GSSiOpenFile (FTRecFile,NULL,OF_READ);
	if (Fid == HFILE_ERROR)
		return FALSE;
	if (BigRead (Fid,(HPSTR)&FTRecord,sizeof(FTRecord)) == sizeof(FTRecord))
		rtn=TRUE;
	GSSiClose2 (&Fid);
	return rtn;
}

BOOL SaveFileTransferRecord (void)
{
	char	FTRecFile[256];   
	BOOL	rtn=FALSE;
	HFILE	Fid;
	
	sprintf (FTRecFile,"[%%DL]FTDir\\%ld.bin",FTRecord.ID);
	Fid = GSSiOpenFile (FTRecFile,NULL,OF_CREATE);
	if (Fid == HFILE_ERROR)
		return FALSE;
	if (BigWrite (Fid,(HPSTR)&FTRecord,sizeof(FTRecord),-1) == sizeof(FTRecord))
		rtn=TRUE;
	GSSiClose2 (&Fid);
	return rtn;
}

BOOL DeleteFileTransferRecord (long ID)
{
	char	FTRecFile[256];   
	BOOL	rtn=FALSE;
	
	sprintf (FTRecFile,"[%%DL]FTDir\\%ld.bin",FTRecord.ID);
	rtn = !GSSiRemove (FTRecFile);
	return rtn;
}

BOOL RequestNextFileTransferSegment (long ID,SOCKET socket)
{
	HANDLE	hStr=GSSiGlobAlloc (0,GMEM_MOVEABLE,1024);
	LPSTR	str=GlobalLock (hStr);   
	long	Len=MAX_FT_SEG_SIZE;
	
	LoadFileTransferRecord (ID); 
	if ((long)Len + FTRecord.Loc > FTRecord.FileLength)
		Len = FTRecord.FileLength - FTRecord.Loc;
	sprintf (str,"$SERVERFILE(GETSEG,-1,%s,%ld,%lu,%ld)",FTRecord.ServerFile,ID,FTRecord.Loc,Len);
	SendTCPIPString (socket,str,TRUE,0); 
	
	GSSiGlobUlFree (&hStr);
	return TRUE;
} 


BOOL ServerFile (LPSTR Option,SOCKET socket,LPSTR ServerFile,LPSTR Arg1,LPSTR Arg2,LPSTR Arg3)
{    
	HFILE	Fid;
	MSG	msg;   
	UINT	WaitSeconds=20;      
	short	rtn=0;  
	HANDLE	hStr=GSSiGlobAlloc (0,GMEM_MOVEABLE,1024);
	LPSTR	str=GlobalLock (hStr);   
	long	FileTransferRequestID; 
	DWORD	Flen, Loc;
	LPBYTE	pRec;
	
	if (socket == -1)
		socket = CurrentServerSocket;	
	if (!_fstricmp (Option,"GET"))
	{   
		FileTransferRequestID = GetNewFileTransferRequest (ServerFile,Arg1,Arg2,Arg3);
		sprintf (str,"$SERVERFILE(GETHEADER,-1,%s,%ld)",ServerFile,FileTransferRequestID);
		SendTCPIPString (socket,str,TRUE,0);  
	}
	if (!_fstricmp (Option,"GETHEADER") && hCmdMess)
	{   
   		LPSTR CmdMess = GlobalLock (hCmdMess);
        DWORD	LastWriteTimeLow=0, LastWriteTimeHigh=0, SFileLength=0;		
		Fid = GSSiOpenFile (ServerFile,NULL,OF_READ);
		if (Fid == HFILE_ERROR)
			sprintf (CmdMess,"$SERVERFILE(HEADER,-1,%s,%s,%lu-%lu,%lu)",ServerFile,Arg1,LastWriteTimeLow,LastWriteTimeHigh,SFileLength);   
		else
		{
			SFileLength = GSSifilelength (Fid);
			sprintf (CmdMess,"$SERVERFILE(HEADER,-1,%s,%s,%lu-%lu,%lu)",ServerFile,Arg1,LastWriteTimeLow,LastWriteTimeHigh,SFileLength);  
			GSSiClose2 (&Fid); 
		}
		GlobalUnlock (hCmdMess);
	} 
	if (!_fstricmp (Option,"HEADER"))
	{   
        FileTransferRequestID = atol (Arg1);  
        Flen = (DWORD)atol (Arg3);
        if (!Flen)
        	DeleteFileTransferRecord (FileTransferRequestID);
        else if (LoadFileTransferRecord (FileTransferRequestID))
        {   
        	LPSTR pDash = _fstrchr (Arg2,'-');
        	
        	if (pDash)
        	{
        		*pDash++ = 0;
        		FTRecord.LastWriteTimeLow = (DWORD)atol (Arg2);
        		FTRecord.LastWriteTimeHigh = (DWORD)atol (pDash);
        	}
       		FTRecord.FileLength = Flen;
       		SaveFileTransferRecord ();
       		RequestNextFileTransferSegment (FileTransferRequestID,socket);
        }
	} 
	if (!_fstricmp (Option,"GETSEG") && hCmdMess)
	{   
   		LPSTR CmdMess = GlobalLock (hCmdMess);
   		
   		Loc = (DWORD)atol (Arg2);
   		SegLen = atoi (Arg3);
		Fid = GSSiOpenFile (ServerFile,NULL,OF_READ);
		if (Fid == HFILE_ERROR) 
		{
			SegLen = 0;
			if (strlen(ServerFile) + strlen(Arg1) < MAX_CMDMESSAGE)
				sprintf(CmdMess, "$SERVERFILE(SEGMENT,-1,%s,%s,%lu,%i)", ServerFile, Arg1, Loc, SegLen);
			else
				*CmdMess = 0;
		}   
		else
		{   
			hSegRec = GSSiGlobAlloc (0,GMEM_MOVEABLE,SegLen);
			pRec = GlobalLock (hSegRec);
			GSSillseek (Fid,Loc,0);
			BigRead (Fid,pRec,SegLen); 
			GlobalUnlock (hSegRec); 
			GSSiClose2 (&Fid);
			sprintf (CmdMess,"$SERVERFILE(SEGMENT,-1,%s,%s,%lu,%i)",ServerFile,Arg1,Loc,SegLen);  
		}
		GlobalUnlock (hCmdMess);
	} 
	if (!_fstricmp (Option,"SEGMENT"))
	{   
        FileTransferRequestID = atol (Arg1);  
        Loc = (DWORD)atol (Arg2);
        SegLen = (DWORD)atoi (Arg3);
        if (!SegLen)
        	DeleteFileTransferRecord (FileTransferRequestID); 
        else
        {   
        	short	i;
        	
			for (i=0;i<MAXOPENSOCKETS;i++)
			{
				if (OpenSockets[i] == socket)
					goto GotID;
			}
				return FALSE;  
GotID:
        	SocketFileTranLength[i] = SegLen;
        	SocketFileTranID[i] = FileTransferRequestID;
        }
	} 
	GSSiGlobUlFree (&hStr);
	return TRUE;
}

BOOL ProcessFTSegment (SOCKET socket,long ID,LPBYTE pSeg,short SegLen)
{   
	HFILE	Fid; 
	UINT	OpenOpt=OF_READWRITE;
	
    if (LoadFileTransferRecord (ID))
    {   
    	if (!FTRecord.Loc)
    		OpenOpt = OF_CREATE;
    	Fid = GSSiOpenFile (FTRecord.TempFile,NULL,OpenOpt);
        	
    	GSSillseek (Fid,0,2);
    	BigWrite (Fid,pSeg,SegLen,-1);
    	GSSiClose2 (&Fid);
    	FTRecord.Loc += SegLen;
    	if (FTRecord.Loc >= FTRecord.FileLength)
    	{   
    		HANDLE	hMacro;
    		long	lMacro; 
    		LPSTR	pMacro;
    		
    		HaltMapDisplay (TRUE,FALSE);
		    CloseSymDict();  
			CloseOrthos(TRUE);
			AddBMPToCache (NULL,0);
			AddBMPToCache32 (NULL,0, 0);
    		CloseAllRequestedFiles(FALSE); 
    		GSSiRemove (FTRecord.LocalFile);
    		GSSiRename (FTRecord.TempFile,FTRecord.LocalFile);  
    		lMacro = _fstrlen (FTRecord.CompletionMacro);   
    		if (lMacro)
    		{
    			hMacro = GSSiGlobAlloc (0,GMEM_MOVEABLE,lMacro+1);   
    			pMacro = GlobalLock (hMacro);
    			_fstrcpy (pMacro,FTRecord.CompletionMacro);  
    			GlobalUnlock (hMacro);  
			    PostMessage(hWndMain, WM_COMMAND, IDM_PROCESSTEXT, (LPARAM)hMacro); 
			}
        	DeleteFileTransferRecord (ID);
    	}
    	else
    	{
       		SaveFileTransferRecord ();
       		RequestNextFileTransferSegment (ID,socket);
       	}
    }
    return TRUE;
}

BOOL ReadNTBlock (SOCKET sock,LPSTR pBuf,long lBuf)
{
	BOOL rtn = FALSE;
	long	ln, lnTot=0, err;

WaitAgain:
	ln = recv (sock,&pBuf[lnTot],lBuf-lnTot,0);
	if (ln == SOCKET_ERROR)
	{	
		err = WSAGetLastError();
		WSASetLastError (0);
		if (ContinueBackgroundCache && err == WSAETIMEDOUT)
			goto WaitAgain;
		else
			return FALSE;
	}
	else if (!ln)
		return FALSE;
	else
		lnTot += ln;

	if (lnTot == lBuf)
		return TRUE;
	goto WaitAgain;
}

BOOL SendNTBlock (SOCKET sock,LPSTR pBuf,long lBuf)
{
	BOOL rtn = FALSE;
	long	ln, lnTot=0, err;

WaitAgain:
	ln = send (sock,&pBuf[lnTot],lBuf-lnTot,0);
	if (ln == SOCKET_ERROR)
	{	
		err = WSAGetLastError();
		WSASetLastError (0);
		if (ContinueBackgroundCache && err == WSAETIMEDOUT)
			goto WaitAgain;
		else
			return FALSE;
	}
	else if (!ln)
		return FALSE;
	else
		lnTot += ln;

	if (lnTot == lBuf)
		return TRUE;
	goto WaitAgain;
}

#include <stdio.h>      /* for printf() and fprintf() */
//#include <sys/socket.h> /* for socket() and bind() */
//#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
//#include <unistd.h>     /* for close() */

#define ECHOMAX 1024     /* Longest string to echo */

void DieWithError(char *errorMessage);  /* External error handling function */

void DieWithError(LPSTR mess)
{
	char str[32];

	int err = WSAGetLastError();
	WSASetLastError(0);
	itoa(err, str, 10);

	MessageBox(0, mess,str, MB_ICONEXCLAMATION);
	return;
}
int UDPmain(int port)
{
	int sock;                        /* Socket */
	struct sockaddr_in echoServAddr; /* Local address */
	struct sockaddr_in echoClntAddr; /* Client address */
	unsigned int cliAddrLen;         /* Length of incoming message */
	char echoBuffer[ECHOMAX+1];        /* Buffer for echo string */
	unsigned short echoServPort;     /* Server port */
	int recvMsgSize;                 /* Size of received message */
	char str[1024];

	//if (argc != 2)         /* Test for correct number of parameters */
	//{
	//	fprintf(stderr, "Usage:  %s <UDP SERVER PORT>\n", argv[0]);
	//	exit(1);
	//}

	echoServPort = port;  /* First arg:  local port */
	{
		u_long myAddr;
		SOCKET ServerSocket;
		if (!OpenWinSock())
		{
			GSSiMsgBox(GetFocus(), "Cannot find Windows Socket Interface", "Error", MB_OK, 0);
			return FALSE;
		}
		_fmemset(&mySockAddr, 0, sizeof(mySockAddr));

		myAddr = inet_addr(CurrentIPAddress);
		ii = 1;
	}

	/* Create socket for sending/receiving datagrams */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket() failed");

	/* Construct local address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
	echoServAddr.sin_family = AF_INET;                /* Internet address family */
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	echoServAddr.sin_port = htons(echoServPort);      /* Local port */

	/* Bind to the local address */
	if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError("bind() failed");

	for (;;) /* Run forever */
	{
		/* Set the size of the in-out parameter */
		cliAddrLen = sizeof(echoClntAddr);

		/* Block until receive message from a client */
		if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0,
			(struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
			DieWithError("recvfrom() failed");
		else
		{
			echoBuffer[recvMsgSize] = 0;
			sprintf(str, "%s\n", echoBuffer);

			AppendFile("c:\\temp\\sierratest.txt", str);
		}
	}
	/* NOT REACHED */
}

