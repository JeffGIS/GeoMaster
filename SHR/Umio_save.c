/* QuickCase:W KNB Version 1.00 */
typedef unsigned short int u_short;
typedef unsigned long u_long;
typedef	char *	caddr_t;
#include "graphint.h"   
#include "umio.h"   

#define MAX_SOCKET_BUFFER_SIZE	4*USHRT_MAX-2
#define MAX_FT_SEG_SIZE			SHRT_MAX-512
#define MAX_PENDING_CONNECTS 8  /* The backlog allowed for listen() */
#define UMPort (u_short) 13760

#include "gmextern.h"

static	BOOL	InProcessTCPData=FALSE;
static	BOOL	BlockSocketInput=FALSE;
static	char	ErrMsg[128], IPAddress[32];  
static	BOOL	FirstSocketCall=TRUE,WinsockOpened=FALSE;
static	SOCKET	OpenSockets[MAXOPENSOCKETS]; 
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
static BOOL		LogSocketIO[MAXOPENSOCKETS];
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
static	int		NumLogCommands=0;
static	int		NumConnections=0;


BOOL SaveFileTransferRecord (void);
void SetSocketTimer (SOCKET socket);
SOCKET	connectToServer ( char *serverName, u_short port,LPINT perr );

int		ReadUM (LPSTR Message);
BOOL	WriteUM (LPSTR Message, int lmes);
BOOL ProcessFTSegment (SOCKET socket,long ID,LPBYTE pSeg,short SegLen);
HWND	TraceWnd2=0;

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
		ExtTextOut (hDC,x,y,0,0,Last32Commands[i],strlen(Last32Commands[i]),0);
		GetTextExtentPoint32(hDC,Last32Commands[i],strlen(Last32Commands[i]),&txsize);
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

	if (!InServerMode)
		return;

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
			memmove (Last32Commands[0],Last32Commands[1],31*256);
			NumLogCommands--;
		}
		strncpy0 (Last32Commands[NumLogCommands++],Mess,255);
	}
	if (GetGlobalCVal ("[%SERVERACTIVITYLOGFILE]",ServerLogFile,0))
	{
		char	TimeAndDate[128]="$CAL([%SYS_CLOCK])";
	
		ExpandText (TimeAndDate);
		AppendFile2 (ServerLogFile,TimeAndDate);
		AppendFile2 (ServerLogFile,Mess);
	}
	InvalidateRect (hWndMain,0,TRUE);
	return;
}

BOOL GetReplayHeader (void)
{
	 if (FidReplay == HFILE_ERROR || !fgetstring (ReplayHeader,127,FidReplay))
	 {
		 GSSiClose (FidReplay);
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
			GSSiClose (FidReplay);
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
	CreateDialog(hInst, (LPSTR)"VEHICLE_REPLAY", hWnd, VEHICLE_TIMEMsgProc);
	return TRUE;
}

int SetReplayPosition (int pos)
{
	LPSTR		nRead;
	char	str[256];

	pos = max (pos,MinReplayPos);
	GSSillseek (FidReplay,pos,0);
	*str = 0;
	nRead =	fgetstring (str,254,FidReplay);

	while (nRead && strncmp (str,"I(0): ",6))
	{
		pos = GSSillseek (FidReplay,0,1);
		nRead = fgetstring (str,254,FidReplay);
	}
	if (nRead)
	{
		strcpy (ReplayHeader,str);
		pos = GSSillseek (FidReplay,0,1);
		if (fgetstring (str,254,FidReplay))
		{
			if (!strnicmp (str,">Plot:",6))
			{
				LPSTR pEnd,pLoc = strchr (str,',');
				
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
		GSSillseek (FidReplay,pos,0);
		return pos;
	}
	return 0;
}

BOOL FAR PASCAL VEHICLE_TIMEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
	BOOL	Err,ii;
	RECT	Rect;
	static	int	lineinc,pageinc;

 switch(Message)
   {
    case WM_INITDIALOG:
		{
			int	top,left;
			POINT	MidPoint;

			hWndVehTime = hWndDlg;
			SetDlgItemInt (hWndDlg,IDC_REPLAYDELAY,(10000-ReplayDelay)/100,FALSE);
			GetWindowRect (hWndDlg,&Rect);
			MidPoint = RectMid (&CurView->ScreenRect);
			top = MidPoint.y - RECTHEIGHT(&Rect)/2;
			left = MidPoint.x - RECTWIDTH(&Rect)/2;
 			MoveWindow(hWndDlg,left,top,RECTWIDTH(&Rect),RECTHEIGHT(&Rect),TRUE);
			if (FidReplay != HFILE_ERROR)
			{
				int	range = GSSifilelength (FidReplay);

				pageinc = range/10;
				lineinc = range/100;
				MinReplayPos = GSSillseek (FidReplay,0,1);
				SetScrollRange(GetDlgItem(hWndDlg,IDC_SCROLLTIME),SB_CTL, 0, range, FALSE);  
			}
		}
        break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */
	case WM_DESTROY:
		hWndVehTime = 0;
		break;
	case WM_NOTIFY:
		{
			LPNMHDR	pnmh;

			pnmh = (LPNMHDR) lParam; 
			switch (wParam)
			{

				case IDC_SPIN1:
				case IDC_REPLAYDELAY:
					ReplayDelay = 10000 - (GetDlgItemInt (hWndDlg,IDC_REPLAYDELAY,&Err,FALSE))*100;
					break;
			}
		}
		break;
  case WM_HSCROLL:
	   {
		int nScrollCode = (int) LOWORD(wParam);  // scroll bar value 
		int	nPos = (short int) HIWORD(wParam);   // scroll box position 
		SCROLLINFO	si;
		HWND	hWnd = GetDlgItem (hWndDlg,IDC_SCROLLTIME);

		if (hWnd != (HWND)lParam)
			break;
		if (HaveReplayTimer)
			KillTimer (hWndMain,HaveReplayTimer);
		HaveReplayTimer = 0;
		switch (nScrollCode)
		{
		  case SB_LINEDOWN: 
			CurReplayPos += lineinc;
		  break;

		  case SB_LINEUP:
			CurReplayPos -= lineinc;
	  	  break;

		  case SB_PAGEDOWN:
			CurReplayPos += pageinc;
	  	  break;

		  case SB_PAGEUP:
			CurReplayPos -= pageinc;
	  	  break;

		  case SB_THUMBPOSITION:
			//CurReplayPos = nPos;
		  break;  
		  
		  case SB_THUMBTRACK:
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
			  CurReplayPos = si.nPos;
			CurReplayPos = SetReplayPosition (CurReplayPos);
			HaveReplayTimer = SetTimer(hWndMain, TCPREPLAYTIMER,max(1,ReplayDelay), (FARPROC) NULL);
			SetScrollPos((HWND)lParam, SB_CTL,CurReplayPos, TRUE);
			return TRUE;
			break;

		  default:
			  return TRUE;
		}
		CurReplayPos = SetReplayPosition (CurReplayPos);
		SetScrollPos((HWND)lParam, SB_CTL,CurReplayPos, TRUE);
	   }
	   break;
    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break; 

			case IDC_SPIN1:
			case IDC_REPLAYDELAY:
				ReplayDelay = 10000 - GetDlgItemInt (hWndDlg,IDC_REPLAYDELAY,&Err,FALSE)*100;
				break;
            case IDC_VEHSTOPSTART: 
            {
				if (ReplayDelay == LONG_MAX)
				{
					SetDlgItemText (hWndDlg,IDC_VEHSTOPSTART,"Halt");
					ReplayDelay = 10000 - GetDlgItemInt (hWndDlg,IDC_REPLAYDELAY,&Err,FALSE)*100;
					if (HaveReplayTimer)
						KillTimer (hWndMain,HaveReplayTimer);
					HaveReplayTimer = SetTimer(hWndMain, TCPREPLAYTIMER, max(1,ReplayDelay), (FARPROC) NULL);
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
						KillTimer (hWndMain,HaveReplayTimer);
					HaveReplayTimer = SetTimer(hWndMain, TCPREPLAYTIMER, max(1,ReplayDelay), (FARPROC) NULL);
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
				HaveReplayTimer = SetTimer(hWndMain, TCPREPLAYTIMER, max(1,ReplayDelay), (FARPROC) NULL);
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
	char	str[32]=" ";
	HDC		hDC=GetDC (hWndMain);
	RECT	Rect={0,0,46,18};
	//char	block[16]=" ";
	int		ln;

	if (FirstSocketCall)
		return; 
//	if (BlockSocketInput)
//		itoa (BlockSocketInput,block,10);
	SaveDC (hDC);
	SetBkMode (hDC,TRANSPARENT);
//	SetBkColor (hDC,RGB(255,255,255));
	if (BlockSocketInput)
		//strcpy (str,"B");
		sprintf (str,"B%i",BlockSocketInput);
	else if (i > -1)
		sprintf (str,"%i",LenSocketBuffer[i]);
/*	if (i > -1)
		ln = LenSocketBuffer[i];
	else
		ln = 0;
	sprintf (str,"%s|%i|%i|%i|%i|%i",block,InDisplayProcessing,InVehicleDisplay,BlockVehicleDisplay,HavePendingDisplay,ln);*/
	SelectClipRgn (hDC,0);
	FillRect (hDC,&Rect,GetStockObject (LTGRAY_BRUSH));
	if (i > -2)
		TextOut (hDC,0,0,str,strlen(str));
	RestoreDC (hDC,-1);
	ReleaseDC (hWndMain,hDC);
	return;
}

BOOL BlockSocketProcessing (BOOL Block)
{
	int	i;

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
	SocketTimerDelay[i] = Seconds * 1000;    
	l = _fstrlen (TimerString);
	hSocketTimerString[i] = GlobalAlloc (GMEM_MOVEABLE,l+1);
	pString = GlobalLock (hSocketTimerString[i]); 
	_fstrcpy (pString,TimerString);
	GlobalUnlock (hSocketTimerString[i]);   
	SetSocketTimer (socket);	

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
		SetTimer(OpenSockethWnd[i], TimerID, SocketTimerDelay[i], (FARPROC) NULL);
	}
	return;
}

void LogSocketError (LPSTR Error,LPSTR Input)
{   
	char	TimeAndDate[128]="$CAL([%SYS_CLOCK])";
	
	ExpandText (TimeAndDate);
	AppendFile2 ("$DIRPATH(ALLUSERAPPDATA,GeoMaster)\\gmsocketerrors.txt",TimeAndDate);
	AppendFile2 ("$DIRPATH(ALLUSERAPPDATA,GeoMaster)\\gmsocketerrors.txt",Error);
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
		for (i=0;i<len;i++)
			if (!Input[i])
				Input[i] = '~';
		AppendFile2 ("$DIRPATH(ALLUSERAPPDATA,GeoMaster)\\socketlog.txt",Header);
		AppendFile2 ("$DIRPATH(ALLUSERAPPDATA,GeoMaster)\\socketlog.txt",Input);
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
		AppendFile2 ("$DIRPATH(ALLUSERAPPDATA,GeoMaster)\\socketlog.txt",Header);
		AppendFile2 ("$DIRPATH(ALLUSERAPPDATA,GeoMaster)\\socketlog.txt",Input); 
	}
	return;
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
		for (i=0;i<MAXOPENSOCKETS;i++)
		{
			OpenSockets[i] = INVALID_SOCKET;
			hSocketProcessString[i] = 0;
			hSocketCloseString[i] = 0; 
			hSocketRestartString[i] = 0; 
			hSocketTimerString[i] = 0;
			hSocketBuffer[i] = 0;
			BlockedSocket[i]=0;
		}  
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

SOCKET connectToServer ( char *serverName, u_short port, LPINT perr )
{
	SOCKET toServerSocket;
	int		err;

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
	WaitCursor (1);
	err =connect( toServerSocket, (LPSOCKADDR)&serverSockAddr,sizeof(serverSockAddr) );
	WaitCursor (-1);
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
    if (UMIODebug) SetWindowText(hWndMain,"Connected to server");
    return toServerSocket;
} /* connectToServer */

short CheckForRegistrationServer (HWND hWnd,LPSTR SerialNum)
{   
	int		err;
	short	rtn=0;
	
	_fstrcpy (SerialNumber,SerialNum);
	{
      FARPROC lpfnREGCONNECTMsgProc; 
      short	nRc;
		
      lpfnREGCONNECTMsgProc = MakeProcInstance((FARPROC)REGCONNECTMsgProc, hInst);
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
	_fstrcpy (IPAddress,ServerIP); 
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

    myAddr = inet_addr(IPAddress);

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
	sprintf (str,"Server opened on %s port %i",IPAddress,port);
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
	err = WSAAsyncSelect (OpenSockets[i],hWnd,GF_TCPIPMESSAGE,FD_READ|FD_CLOSE);
	hSocketBuffer[i] = GSSiGlobAlloc (0,GHND,MAX_SOCKET_BUFFER_SIZE);  
	LenSocketInputTerminator[i] = GetGlobalLVal2 ("[%ServerInputTerminatorLen]",0);
	GetGlobalCVal ("[%ServerInputTerminator]",SocketInputTerminator[i],"");  
	NumConnections++;
	sprintf (mess,"Open accepted on socket %i",(int)sock);
	LogServerActivity (mess);

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
		sock = connectToServer (TCPAddress,Port,&err);
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
			hSocketProcessString[i] = GlobalAlloc (GMEM_MOVEABLE,l+1);
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
			hSocketCloseString[i] = GlobalAlloc (GMEM_MOVEABLE,l+1);
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
			hSocketRestartString[i] = GlobalAlloc (GMEM_MOVEABLE,l+1);
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
			{
				recv (OpenSockets[i],(LPSTR)&Dummy,4,MSG_PEEK);//re-enable messages
				goto RtnFalse;
			}
		BlockedSocket[NumBlockedSockets++] = sock;
		recv (OpenSockets[i],(LPSTR)&Dummy,4,MSG_PEEK);//re-enable messages
		goto RtnFalse;
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
		LogSocketInput (i,SocketInputBuffer,err);     
		LenSocketBuffer[i]+=err; 
		DisplayLenSocketBuffer (i);
	}
	if (!InDisplayProcessing && LenSocketBuffer[i])
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
			    	SetWindowText(hWndMain,Cmd);
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
        		short	lCmd = strlen (Cmd);;
        		
				*CmdMess = 0;  
				GlobalUnlock (hCmdMess);
				{
					HANDLE	hCmd = GSSiGlobAlloc (0,GMEM_MOVEABLE,lCmd+1);   
					LPSTR	pCmd = GlobalLock ((HANDLE)hCmd);

					strcpy (pCmd,Cmd);  
    				GlobalUnlock (hCmd);  
					PostMessage(hWndMain, GF_PRCESSTCPCMD, (WPARAM)sock, (LPARAM)hCmd); 
				}
			} 
		}
	}
	if (!InDisplayProcessing && LenSocketBuffer[i])
		PostMessage(hWndMain, GF_PRCESSTCPCMD, (WPARAM)sock, 0); 
	else
	{
		if (BlockVehicleDisplay > 1)
		{
			BlockVehicleDisplay = 0;
			UpdateVehicleStatusDlg ();
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
	
	ContinueProcessing = TRUE;
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
	ProcessText (Cmd); 
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
	GSSiClose (Fid);  
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
	GSSiClose (Fid);
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
	GSSiClose (Fid);
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
			GSSiClose (Fid); 
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
			sprintf (CmdMess,"$SERVERFILE(SEGMENT,-1,%s,%s,%lu,%i)",ServerFile,Arg1,Loc,SegLen);
		}   
		else
		{   
			hSegRec = GSSiGlobAlloc (0,GMEM_MOVEABLE,SegLen);
			pRec = GlobalLock (hSegRec);
			GSSillseek (Fid,Loc,0);
			BigRead (Fid,pRec,SegLen); 
			GlobalUnlock (hSegRec); 
			GSSiClose (Fid);
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
    	GSSiClose (Fid);
    	FTRecord.Loc += SegLen;
    	if (FTRecord.Loc >= FTRecord.FileLength)
    	{   
    		HANDLE	hMacro;
    		long	lMacro; 
    		LPSTR	pMacro;
    		
    		HaltMapDisplay (TRUE);
		    CloseSymDict();  
			CloseOrthos ();
			AddBMPToCache (NULL,0);
			AddBMPToCache32 (NULL,0);
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