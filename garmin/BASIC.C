/* QuickCase:W KNB Version 1.00 */
#include "BASIC.h" 
#include <stdio.h>  
#include <ctype.h>
#include <math.h>    
#include "garmin.h" 
#include "resource.h"
short	Stream=-1; 
char	Input[2048], InBuf[2][2048]={"",""};
short	lbuf[2]={0,1};
BOOL FAR PASCAL IMPORTWAYPMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
BOOL FAR PASCAL ABOUTMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
BOOL FAR PASCAL DIALOGSTYLEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
BOOL ProcessCOMMNotification( HWND hWnd, WORD wParam, LONG lParam );
BOOL SetupSIOConnection(short Stream,short flowtype);
BOOL CloseSIOConnection(short Stream);
BOOL WriteCommByte( HWND hWnd, BYTE bByte ,short Stream);
short OpenSIOConnection( HWND hWnd ,LPSTR ID,short flowtype);
void DumpDCB (LPSTR ComID,int Stream,LPSTR File);
void LogIO (int Stream,LPSTR Input,short len, HWND hWnd);
void AppendFile (LPSTR File,LPSTR Line); 

#define MOREDATA 10000
#define IDM_OPENSIO 10001
UINT	GPSTimeOut=2000;
    
#define GPSTIMER	10 
#define INVALIDPACKET	0

enum { EOM=-4, CHK_SUM, START_CHK_SUM, USE_ARGV };  // Must not be 0..255

int send_init[]={0x10,START_CHK_SUM,0xfe,0,CHK_SUM,0x10,3,EOM};
int send_init_spare[]={START_CHK_SUM,0x10,0xfe,0,2,0x10,3,EOM};
int send_alminac[]={0x10,START_CHK_SUM,0xa,2,1,0,CHK_SUM,0x10,3,EOM};
int send_begin_ack[]={0x10,START_CHK_SUM,0x6,2,0xfe,0,CHK_SUM,0x10,3,EOM};
int send_clock[]={0x10,START_CHK_SUM,0xa,2,5,0,CHK_SUM,0x10,3,EOM};// clock
int send_position[]={0x10,START_CHK_SUM,0xa,2,2,0,CHK_SUM,0x10,3,EOM};// clock
//int send_bad_position[]={0x10,START_CHK_SUM,0x11,0x20,0,0,0,0,0,0,0xf8,0x3f,
//	0,0,0,0,0,0,0xf8,0x3f, CHK_SUM,0x10,3,EOM1};// faked position
int send_request[]={0x10,START_CHK_SUM,Pid_Command_Data,2,USE_ARGV,0,CHK_SUM,0x10,3,EOM};
int send_ack[]={0x10,START_CHK_SUM,Pid_Ack_Byte,2,USE_ARGV,0,CHK_SUM,0x10,3,EOM};
int send_nak[]={0x10,START_CHK_SUM,Pid_Nak_Byte,2,USE_ARGV,0,CHK_SUM,0x10,3,EOM};
HBRUSH	hBackBrush1; 

BOOL GetGPSIdentity(LPSTR Identity);
void CreatePacket (LPINT msg,short Arg,LPBYTE Packet );
short	PacketLength (LPBYTE Packet);
short	HaveCompletePacket (LPBYTE Packet,short Length);

DWORD GetDlgItemPrompt (HWND hWnd){return 0;}
void SetPromptDlg (UINT PromptID){return;}

BOOL ValidChecksum (LPBYTE Packet)
{
	return TRUE;
}

void SendACK (int Stream,short PacketID)
{  
	BYTE	Packet[512];
	short	ii;
	
	CreatePacket (send_ack,PacketID,Packet);   
	ii=WriteComm(Stream,Packet,PacketLength(Packet));
	return;
}

void SendNAK (int Stream,short PacketID)
{  
	BYTE	Packet[512];
	short	ii;
	
	CreatePacket (send_nak,PacketID,Packet);   
	ii=WriteComm(Stream,Packet,PacketLength(Packet));
	return;
}

void CreatePacket (LPINT msg,short Arg,LPBYTE Packet )
{   
	short	chksum;
	
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

short	PacketLength (LPBYTE Packet)
{
	short	len=0;
	char	l=Packet[2];
	
	return l+6;
}   

short	HaveCompletePacket (LPBYTE Packet,short Length)
{   
	short	PacketID, plength;
	
	if (Length < 6)
		return FALSE;
    if (Packet[0] != 16)
    	return FALSE; 
    PacketID = Packet[1];
    plength = Packet[2];
    if (plength + 6 > Length)
    	return FALSE;
    if (Packet[4+plength] != 16 || Packet[5+plength] != 3)
    	return FALSE;
	return PacketID;
	
}

BOOL SendPacket (int Stream,LPBYTE Packet)
{
	int        nError, nLength=0,ii, LenInput ;
	COMSTAT    ComStat ;
	MSG        msg ; 
	BOOL	HaveMsg;  
	BYTE	InPacket[1024]; 
	BOOL	Continue=TRUE;
	short	InLength=0, PacketID; 
   
	nLength = ReadComm( Stream, &InPacket[InLength], 512 );
	ii=WriteComm(Stream,Packet,PacketLength(Packet));
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
				KillTimer (hWndMain,GPSTIMER);
				return FALSE;
				
			default:
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			    break;
			    
			case WM_COMMNOTIFY:
      			if (CN_EVENT & LOWORD( msg.lParam ) != CN_EVENT)
      				break;
      		
            case MOREDATA:
			     GetCommEventMask( Stream, EV_RXFLAG ) ;
				 if ((nLength = ReadComm( Stream, &InPacket[InLength], 512 ))>0)
				 { 
				 	InLength+=nLength; 
				 	if ((PacketID = HaveCompletePacket (InPacket,InLength)))
				 	{
				 		if (PacketID == Pid_Ack_Byte)
				 		{
							KillTimer (hWndMain,GPSTIMER);
				 			return TRUE;
				 		}
				 	} 
				 }
			}
      }
	KillTimer (hWndMain,GPSTIMER);
	return FALSE;
}
 

short GetPacket (int Stream,LPBYTE Packet)
{
	int        nError, nLength=1,ii, LenInput ;
	COMSTAT    ComStat ;
	MSG        msg ; 
	BOOL	HaveMsg;  
	BYTE	InPacket[1024]; 
	BOOL	Continue=TRUE;
	short	InLength=0, PacketID; 
   
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
				KillTimer (hWndMain,GPSTIMER);
				return FALSE;
				
			default:
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			    break;
			    
			case WM_COMMNOTIFY:
      			if (CN_EVENT & LOWORD( msg.lParam ) != CN_EVENT)
      				break;
			     GetCommEventMask( Stream, EV_RXFLAG ) ;
      		
            case MOREDATA:
				 if (InLength < 512 && (nLength = ReadComm( Stream, &Packet[InLength], 512 ))>0)
				 { 
				 	InLength+=nLength; 
				 	if ((PacketID = HaveCompletePacket (Packet,InLength)))
				 	{
						KillTimer (hWndMain,GPSTIMER);
						if (PacketID == INVALIDPACKET || !ValidChecksum (Packet))
						{
							SendNAK (Stream,PacketID);
							goto Top;
						}
						SendACK (Stream,PacketID);
				 		return PacketID;
				 	} 
				 }
				 else
				 	nLength = -1;
			}
      }
	KillTimer (hWndMain,GPSTIMER);
	return FALSE;
}
 
 
int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
 /***********************************************************************/
 /* HANDLE hInstance;       handle for this instance                    */
 /* HANDLE hPrevInstance;   handle for possible previous instances      */
 /* LPSTR  lpszCmdLine;     long pointer to exec command line           */
 /* int    nCmdShow;        Show code for main window display           */
 /***********************************************************************/

 MSG        msg;           /* MSG structure to store your messages        */
 int        nRc;           /* return value from Register Classes          */

 strcpy(szAppName, "GarminGPS");
 hInst = hInstance;
 if(!hPrevInstance)
   {
    /* register window classes if first instance of application         */
    if ((nRc = nCwRegisterClasses()) == -1)
      {
       /* registering one of the windows failed                         */       
       LoadString(hInst, IDS_ERR_REGISTER_CLASS, szString, sizeof(szString));
       MessageBox(NULL, szString, NULL, MB_ICONEXCLAMATION);
       return nRc;    
      }
   }

 /* create application's Main window                                    */
 hWndMain = CreateWindow(
                szAppName,               /* Window class name           */
                "Garmin GPS Interlink", /* Window's title          */
                WS_CAPTION      |        /* Title and Min/Max           */
                WS_SYSMENU      |        /* Add system menu box         */
                WS_MINIMIZEBOX  |        /* Add minimize box            */
                WS_MAXIMIZEBOX  |        /* Add maximize box            */
                WS_THICKFRAME   |        /* thick sizeable frame        */
                WS_CLIPCHILDREN |         /* don't draw in child windows areas */
                WS_OVERLAPPED,
                CW_USEDEFAULT, 0,        /* Use default X, Y            */
                CW_USEDEFAULT, 0,        /* Use default X, Y            */
                NULL,                    /* Parent window's handle      */
                NULL,                    /* Default to Class Menu       */
                hInst,                   /* Instance of window          */
                NULL);                   /* Create struct for WM_CREATE */


 if(hWndMain == NULL)
   {
    LoadString(hInst, IDS_ERR_CREATE_WINDOW, szString, sizeof(szString));
    MessageBox(NULL, szString, NULL, MB_ICONEXCLAMATION);
    return IDS_ERR_CREATE_WINDOW;
   }

 ShowWindow(hWndMain, nCmdShow);            /* display main window      */

 while(GetMessage(&msg, NULL, 0, 0))        /* Until WM_QUIT message    */
   {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
   }

 /* Do clean up before exiting from the application                     */
 CwUnRegisterClasses();
 return msg.wParam;
} /*  End of WinMain                                                    */
/************************************************************************/
/*                                                                      */   
/* Main Window Procedure                                                */
/*                                                                      */
/* This procedure provides service routines for the Windows events      */
/* (messages) that Windows sends to the window, as well as the user     */
/* initiated events (messages) that are generated when the user selects */
/* the action bar and pulldown menu controls or the corresponding       */
/* keyboard accelerators.                                               */
/*                                                                      */
/************************************************************************/

LONG FAR PASCAL WndProc(HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{
 HMENU      hMenu=0;            /* handle for the menu                 */
 HBITMAP    hBitmap=0;          /* handle for bitmaps                  */                              
 HDC        hDC;                /* handle for the display device       */
 PAINTSTRUCT ps;                /* holds PAINT information             */
 int        nRc=0;              /* return code                         */
 char		str[256];
 
 switch (Message)
   {
    case WM_COMMNOTIFY:
         ProcessCOMMNotification( hWnd, (WORD) wParam, (LONG) lParam ) ;
         break ;

    case WM_CREATE:
         PostMessage (hWnd,IDM_OPENSIO,0,0);
         
         break;
         
    case IDM_OPENSIO:
		 if (GetGPSIdentity (str))
		 {  
		 	char	mess[150];
		 	
		 	sprintf (mess,"Garmin GPS Interlink connected to: %s",str);
		 	SetWindowText (hWnd,mess);
		 } 
		 else
	         PostMessage (hWnd,WM_CLOSE,0,0);
		 
         break;       /*  End of WM_CREATE                              */      
         
    case WM_COMMAND:
         /* The Windows messages for action bar and pulldown menu items */
         /* are processed here.                                         */ 
         
        switch (wParam)
        {
		    case IDM_IMPORTWP:
		         {
		          FARPROC lpfnIMPORTWAYPMsgProc;
		
		          lpfnIMPORTWAYPMsgProc = MakeProcInstance((FARPROC)IMPORTWAYPMsgProc, hInst);
		          nRc = DialogBox(hInst, (LPSTR) "IMPORTWAYP", hWnd, lpfnIMPORTWAYPMsgProc);
		          FreeProcInstance(lpfnIMPORTWAYPMsgProc);
		
		         }
		    	 break;  
		}
		break;
            	 


    case WM_MOVE:     /*  code for moving the window                    */
         break;
    
    case WM_SIZE:     /*  code for sizing client area                   */
         break;       /* End of WM_SIZE                                 */

    case WM_PAINT:    /* code for the window's client area              */
         /* Obtain a handle to the device context                       */
         /* BeginPaint will sends WM_ERASEBKGND if appropriate          */
         memset(&ps, 0x00, sizeof(PAINTSTRUCT));
         hDC = BeginPaint(hWnd, &ps);

         /* Included in case the background is not a pure color         */
         SetBkMode(hDC, TRANSPARENT);

         /* Inform Windows painting is complete                         */
         EndPaint(hWnd, &ps); 
         break;       /*  End of WM_PAINT                               */

    case WM_CLOSE:  /* close the window                                 */
         /* Destroy child windows, modeless dialogs, then, this window  */   
         CloseSIOConnection (Stream);
//         CloseSIOConnection (SIO2Stream);
//		 LogIO (-1,0,0,0); 
         DestroyWindow(hWnd);
         if (hWnd == hWndMain)
           PostQuitMessage(0);  /* Quit the application                 */
        break;

    default:
         /* For any message for which you don't specifically provide a  */
         /* service routine, you should return the message to Windows   */
         /* for default message processing.                             */
         return DefWindowProc(hWnd, Message, wParam, lParam);
   } 
 return 0L;
}     /* End of WndProc                                         */

/************************************************************************/
/*                                                                      */
/* nCwRegisterClasses Function                                          */
/*                                                                      */
/* The following function registers all the classes of all the windows  */
/* associated with this application. The function returns an error code */
/* if unsuccessful, otherwise it returns 0.                             */
/*                                                                      */
/************************************************************************/

int nCwRegisterClasses(void)
{
 HBITMAP	hBmp;
 WNDCLASS   wndclass;    /* struct to define a window class             */
 memset(&wndclass, 0x00, sizeof(WNDCLASS));   


 /* load WNDCLASS with window's characteristics                         */
 wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_BYTEALIGNWINDOW;
 wndclass.lpfnWndProc = WndProc;
 /* Extra storage for Class and Window objects                          */
 wndclass.cbClsExtra = 0;
 wndclass.cbWndExtra = 0;
 wndclass.hInstance = hInst;
 wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
 wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
 hBmp = LoadBitmap (hInst,"BACKGROUND_1");
 hBackBrush1 = CreatePatternBrush (hBmp); 
 DeleteObject (hBmp);
 wndclass.hbrBackground = hBackBrush1 ;
 wndclass.lpszMenuName = szAppName;   /* Menu Name is App Name */
 wndclass.lpszClassName = szAppName; /* Class Name is App Name */
 if(!RegisterClass(&wndclass))
   return -1;


 return(0);
} /* End of nCwRegisterClasses                                          */

/************************************************************************/
/*  CwUnRegisterClasses Function                                        */
/*                                                                      */
/*  Deletes any refrences to windows resources created for this         */
/*  application, frees memory, deletes instance, handles and does       */
/*  clean up prior to exiting the window                                */
/*                                                                      */
/************************************************************************/

void CwUnRegisterClasses(void)
{
 WNDCLASS   wndclass;    /* struct to define a window class             */
 memset(&wndclass, 0x00, sizeof(WNDCLASS));

 UnregisterClass(szAppName, hInst);
}    /* End of CwUnRegisterClasses                                      */        

BOOL CheckForNMEA (void)
{
	return FALSE;
}

BOOL GetGPSIdentity(LPSTR Identity)
{
	char       szError[ 10 ] ;
	int        nError, nLength,ii, LenInput ;
	COMSTAT    ComStat ;
	MSG        msg ;    
	BYTE	Packet[270];  
	LPBYTE	pstr=Packet; 
	HANDLE	hInput;
	LPSTR	pInput, bInput, pCmd;
	BOOL		rtn=TRUE;  
	POINT	CursorPoint;
	short	ProdID, SVer;
	LPINT	pInt;  
	LPSTR	pDesc;
	   
Top:       
	CloseSIOConnection(Stream);
	Stream = OpenSIOConnection(hWndMain ,"COM1",0);
	if (Stream < 0)
		return FALSE;
//	DumpDCB ("COM1",SIO1Stream,"c:\\siodump.txt"); 
	FlushComm(Stream, 0);
	FlushComm(Stream, 1);  
	if (CheckForNMEA ())
		goto Top;
	*Identity = 0;
	CreatePacket (send_init,0,pstr);   
	if (!SendPacket (Stream,pstr))  
	{
		if (MessageBox (GetFocus(),"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
			NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
			goto Top;
	   	return FALSE; 
	}
	if (!GetPacket (Stream,pstr))
   		goto Top;
    pInt = &Packet[3];
    ProdID = *pInt++;
    SVer = *pInt++;
    pDesc = pInt;
    _fstrcpy (Identity,pDesc); 
    while (GetPacket (Stream,pstr)); //Get protocol capabilities array if exists
	return TRUE;
}

BOOL ProcessCOMMNotification( HWND hWnd, WORD wParam, LONG lParam )
{
   char       szError[ 10 ] ;
   int        nError, nLength,ii, LenInput ;
   COMSTAT    ComStat ;
   MSG        msg ;    
   char	str[256];   
   HANDLE	hInput;
   LPSTR	pInput, bInput, pCmd;
   BOOL		rtn=TRUE;  
   POINT	CursorPoint;
   short	Stream=wParam; 
   static	short	n=0; 
   
   
      // verify that it is a COMM event specified by our mask

      if (CN_EVENT & LOWORD( lParam ) != CN_EVENT)
//      if (CN_RECEIVE & LOWORD( lParam ) != CN_RECEIVE)
      {
         rtn=FALSE;
         goto Exit;
      }

      // reset the event word so we are notified
      // when the next event occurs

      GetCommEventMask( Stream, EV_RXFLAG ) ;

      // We loop here since it is highly likely that the buffer
      // can been filled while we are reading this block.  This
      // is especially true when operating at high baud rates
      // (e.g. >= 9600 baud).

      do
      {  
	     GetCommEventMask( Stream, EV_RXCHAR ) ;
		 if ((nLength = ReadComm( Stream, Input, 2048 ))<0)
		 	ii=1;
		 else
		 {   
		 	 if (nLength)
		 	 {  
//			 	LogIO (Stream,Input,nLength,hWnd); 
//			 	WriteComm( OpStream,Input,nLength);
		 	 }
		 }
      }
      while (PeekMessage( &msg, NULL, WM_COMMNOTIFY,WM_COMMNOTIFY, PM_REMOVE) ||
             (nLength > 0)) ;
Exit:
   return ( rtn ) ;

} 

void LogIO (int Stream,LPSTR Input,short len,HWND hWnd)
{
	short	id=0, Opid=1;
	char	str[1024];
	static	short	y=5;
	
	if (Stream < 0)  
	{   
		for (Opid=0;Opid<2;Opid++)
		if (lbuf[Opid])
		{
			sprintf (str,"%i:%s",Opid,InBuf[Opid]);
			AppendFile ("c:\\siolog.txt",str);
			lbuf[Opid]=0;
		}
		return;
	}          
/*	if (Stream == SIO2Stream)
	{
		id=1;
		Opid = 0;
	} */
	
	if (lbuf[Opid])
	{
	 	HDC	hDC = GetDC (hWnd);
		sprintf (str,"%i:%s\r\n",Opid,InBuf[Opid]);
		AppendFile ("c:\\siolog.txt",str);  
		TextOut (hDC,5,y,str,_fstrlen(str));
		y+=15;
		if (y>600)
		{   
			RECT	ClientRect;
			
			GetClientRect (hWnd,&ClientRect);
			y=5;
			FillRect (hDC,&ClientRect,GetStockObject(WHITE_BRUSH));
		}
	 	ReleaseDC (hWnd,hDC);
		lbuf[Opid]=0;
		*InBuf[Opid]=0;
	}
	while (len--)
	{
		if (iscntrl (*Input))
			sprintf (_fstrchr(InBuf[id],0),"\\%3.3i",(int)*Input);
		else
			sprintf (_fstrchr(InBuf[id],0),"%c",*Input); 
		Input++;
	}
	lbuf[id] = _fstrlen (InBuf[id]);
	return;
}
 
short OpenSIOConnection( HWND hWnd ,LPSTR ID,short flowtype)
{
   BOOL       fRetVal ;
   BOOL		rtn=FALSE;
   int		ii;
   LPSTR	lpComma, lpStart;    
   char		title[144],mess[256];
   short	Stream;
   
   if ((Stream = OpenComm( ID, 4096, 256)) < 0)  
   {     
   		switch (Stream)
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
      	} 
      	sprintf(title,"Error opening com port %s",ID);
      	MessageBox (hWnd,mess,title,MB_ICONEXCLAMATION);
        return ( Stream) ;
    }
	if (SetupSIOConnection(Stream,flowtype))
	{
      // set up notifications from COMM.DRV

         // In this case we really are only using the notifications
         // for the received characters - it could be expanded to
         // cover the changes in CD or other status lines.

		SetCommEventMask( Stream, EV_RXFLAG ) ;

         // Enable notifications for events only.

         // NB:  This method does not use the specific
         // in/out queue triggers.

        ii=EnableCommNotification(Stream, hWnd, -1, -1 ) ;
      // assert DTR

      ii=EscapeCommFunction( Stream, SETDTR ) ;  
      ii=EscapeCommFunction( Stream, SETRTS ) ;  
      
   }
   else
   {  
   	  MessageBox (hWnd,"Error setting digitizer parameters","Unable to open digitizer",MB_ICONEXCLAMATION);
      CloseComm(Stream) ;
      Stream=-1;
   }

   return ( Stream ) ;

}

BOOL SetupSIOConnection(short Stream,short flowtype)
{
   int		rtn,i ;
   BYTE       bSet ;
   DCB        dcb ;
   char			str[128]; 
   LPSTR	pCmd;

	_fmemset (&dcb,0,sizeof(DCB));	
	GetCommState (Stream,&dcb);
	dcb.BaudRate = CBR_9600;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.fOutxCtsFlow = dcb.fRtsflow = 0;
	dcb.CtsTimeout = 0;
    dcb.fOutxDsrFlow = dcb.fDtrflow = 0;
	dcb.DsrTimeout = 0;
	dcb.fInX = dcb.fOutX = 0; 
	if (flowtype==1)
	{
		dcb.fDtrDisable = TRUE ; 
		dcb.fOutxCtsFlow = dcb.fRtsflow = 1;
		dcb.CtsTimeout = 500;  
	}
	else if (flowtype==2) 
	{
		dcb.fRtsDisable = TRUE;
	    dcb.fOutxDsrFlow = dcb.fDtrflow = 1;
		dcb.DsrTimeout = 500;
   	}
	dcb.fBinary = TRUE ;
	dcb.fParity = FALSE ;
	dcb.fChEvt = TRUE;  
	dcb.EvtChar = 3;
   rtn = SetCommState( &dcb );
   
   if (rtn < 0)
   		return FALSE;
   return TRUE;  
   

} 
BOOL CloseSIOConnection(short Stream)
{  
	short	i;
	
   if (Stream < 0)
   		return FALSE;
   EnableCommNotification( Stream, NULL, -1, -1 ) ;

   // kill the focus


   EscapeCommFunction(Stream, CLRDTR ) ;

   // close comm connection

   CloseComm( Stream ) ;

   return ( TRUE ) ;

} 
 
BOOL WriteCommByte( HWND hWnd, BYTE bByte ,short Stream)
{

	MSG	msg;

   WriteComm( Stream, (LPSTR) &bByte, 1 ) ;
   while (!PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE));

   return ( TRUE ) ;

}

BOOL fputstring(LPSTR lpStr, HFILE Fid)
{   
    UINT    len;
    
    len=_fstrlen(lpStr);
    if (len)
    {
        if (_lwrite (Fid,(char *)lpStr,len)!=len)
            return FALSE;
    }
    _lwrite (Fid,"\r\n",2);
    return TRUE;
}

void AppendFile (LPSTR File,LPSTR Line)
{
	OFSTRUCT	OFStruct;
	HFILE		Fid;
	
	Fid = OpenFile (File,&OFStruct,OF_READWRITE);
	if (Fid == HFILE_ERROR)
		Fid = OpenFile (File,&OFStruct,OF_CREATE);
	_llseek (Fid,0,2);
	fputstring (Line,Fid);
	_lclose (Fid);
	return;
}


void DumpDCB (LPSTR ComID,int Stream,LPSTR File)
{   
	DCB		dcb;
	LPDCB	pDCB=&dcb;
	char	str[128];
	    
	    sprintf (str,"Dump DCB for %s",ComID);
		AppendFile (File,str);
	    
		GetCommState (Stream,&dcb);
		sprintf (str,"%s:%u","Id",(UINT)pDCB->Id);
		AppendFile (File,str);
		sprintf (str,"%s:%u","BaudRate",(UINT)pDCB->BaudRate);
		AppendFile (File,str);
		sprintf (str,"%s:%u","ByteSize",(UINT)pDCB->ByteSize);
		AppendFile (File,str);
		sprintf (str,"%s:%u","Parity",(UINT)pDCB->Parity);
		AppendFile (File,str);
		sprintf (str,"%s:%u","StopBits",(UINT)pDCB->StopBits);
		AppendFile (File,str);
		sprintf (str,"%s:%u","RlsTimeout",(UINT)pDCB->RlsTimeout);
		AppendFile (File,str);
		sprintf (str,"%s:%u","CtsTimeout",(UINT)pDCB->CtsTimeout);
		AppendFile (File,str);
		sprintf (str,"%s:%u","DsrTimeout",(UINT)pDCB->DsrTimeout);
		AppendFile (File,str);
		
		sprintf (str,"%s:%u","fBinary",(UINT)pDCB->fBinary);
		AppendFile (File,str);
		sprintf (str,"%s:%u","fRtsDisable",(UINT)pDCB->fRtsDisable);
		AppendFile (File,str);
		sprintf (str,"%s:%u","fParity",(UINT)pDCB->fParity);
		AppendFile (File,str);
		sprintf (str,"%s:%u","fOutxCtsFlow",(UINT)pDCB->fOutxCtsFlow);
		AppendFile (File,str);
		sprintf (str,"%s:%u","fOutxDsrFlow",(UINT)pDCB->fOutxDsrFlow);
		AppendFile (File,str);
		sprintf (str,"%s:%u","fDummy",(UINT)pDCB->fDummy);
		AppendFile (File,str);
		sprintf (str,"%s:%u","fDtrDisable",(UINT)pDCB->fDtrDisable);
		AppendFile (File,str);
	
	
		sprintf (str,"%s:%u","fOutX",(UINT)pDCB->fOutX);
		AppendFile (File,str);
		sprintf (str,"%s:%u","fInX",(UINT)pDCB->fInX);
		AppendFile (File,str);
		sprintf (str,"%s:%u","fPeChar",(UINT)pDCB->fPeChar);
		AppendFile (File,str);
		sprintf (str,"%s:%u","fNull",(UINT)pDCB->fNull);
		AppendFile (File,str);
		sprintf (str,"%s:%u","fChEvt",(UINT)pDCB->fChEvt);
		AppendFile (File,str);
		sprintf (str,"%s:%u","fDtrflow",(UINT)pDCB->fDtrflow);
		AppendFile (File,str);
		sprintf (str,"%s:%u","fRtsflow",(UINT)pDCB->fRtsflow);
		AppendFile (File,str);
		sprintf (str,"%s:%u","fDummy2",(UINT)pDCB->fDummy2);
		AppendFile (File,str);
	
		sprintf (str,"%s:%u","XonChar",(UINT)pDCB->XonChar);
		AppendFile (File,str);
		sprintf (str,"%s:%u","XoffChar",(UINT)pDCB->XoffChar);
		AppendFile (File,str);
		sprintf (str,"%s:%u","XonLim",(UINT)pDCB->XonLim);
		AppendFile (File,str);
		sprintf (str,"%s:%u","XoffLim",(UINT)pDCB->XoffLim);
		AppendFile (File,str);
		sprintf (str,"%s:%u","PeChar",(UINT)pDCB->PeChar);
		AppendFile (File,str);
		sprintf (str,"%s:%u","EofChar",(UINT)pDCB->EofChar);
		AppendFile (File,str);
		sprintf (str,"%s:%u","EvtChar",(UINT)pDCB->EvtChar);
		AppendFile (File,str);
		sprintf (str,"%s:%u","TxDelay",(UINT)pDCB->TxDelay);
		AppendFile (File,str);
	return;
}

BOOL FAR PASCAL ABOUTMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
	char	str[128], cUID[32];	

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
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break;
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} /* End of ABOUTMsgProc                                      */


BOOL FAR PASCAL IMPORTWAYPMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
	char	str[256], cUID[32];	 
	BYTE	Packet[270];  
	short	PacketID=0;
	D103_Wpt_Type	*pWpt;
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
Top:    
	CreatePacket (send_request,Cmnd_Transfer_Wpt,Packet);   
	if (!SendPacket (Stream,Packet))  
	{ 
ErMsg:
		if (MessageBox (GetFocus(),"The GPS unit is not responding. Be sure it is turned on, the cable is connected and the baud rate and output format are properly set.",
			NULL,MB_ICONEXCLAMATION|MB_RETRYCANCEL)==IDRETRY)
			goto Top;
	   	return FALSE; 
	} 
	while (PacketID != Pid_Xfer_Cmplt)
	{
		PacketID = GetPacket (Stream,Packet);
		switch (PacketID)
		{ 
			case 0:
				goto ErMsg; 
			case Pid_Records:
			case Pid_Xfer_Cmplt:
				break;
			case Pid_Wpt_Data:
			{   
				double lat,lon;
				pWpt = &Packet[3]; 
				sprintf (str,"%.6s :%f %f %.40s",pWpt->ident,pWpt->posn.lat*SEMICIRCLETODEGREE,
															 pWpt->posn.lon*SEMICIRCLETODEGREE,
															 pWpt->cmnt);
		 		SendDlgItemMessage (hWndDlg,IDC_WPLIST,LB_ADDSTRING,(WPARAM)NULL,(LPARAM) str); 
		 	}
				break;
		}
	}
//		 SetDlgItemText (hWndDlg,IDC_VERSION,GMVersion);	
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
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} /* End of ABOUTMsgProc                                      */



