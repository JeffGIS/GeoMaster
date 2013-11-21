#include "resource.h"
#include "graphint.h"  
#include "dict.h"   
#include "gps.h"
#include "p_tol.h"

#define	MAXBLOCK	4096       
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D     

#include "gmextern.h"

#define MAX_SEGMENTS	1024 

static	long	MaxSegments = MAX_SEGMENTS;
static	BOOL	DoTrack = TRUE, Spline=FALSE; 
static	long	nTempPoints=0; 
static  HANDLE	hTempPoints=0;   
static	BOOL	HaveRedefineData=FALSE;
static	PICKDATA	RedefineData; 
static	UINT	NumPolyCopies=0;  
static	double	PolyCopyOffdist=0;
static	BOOL	HavePOC=FALSE, GPSInput=FALSE, DisplayNMEA=FALSE;     
static	DPOINT	TrackLineBegin, TrackLineEnd;   
static	DPOINT	LastEP; 
static	BOOL	HaveLastEP=FALSE;
static	char	IncompleteDigInput[4096]="";
static	char	ButtonCodes[17][4];   
static	HANDLE	hDigButtonCmd[17]; 
static HPSEGMENTDATA	SegData;
static BOOL	FirstMoveSinceRedraw=FALSE;
static BOOL	DigHiPrecis=TRUE;
static short	TempLineType=0;
static BOOL	WantDigDigCtlPnt=FALSE;
static HANDLE	hTranDigToWorld=0;
static short	nDigControlPoints=0;
static DPOINT	CurrentDigPoint;
static short	DigXBegin;
static short	DigXLen;
static short	DigYBegin;
static short	DigYLen;
static short	ButtonBegin;
static short	ButtonLen;
static BOOL	DigEventMode=TRUE;
static UINT	RxQueSize;
static HANDLE	hDigQue=0;
static UINT	DigQueLen=0;
static char	EndOfRecChar=ASCII_CR;
static short	LastButton;
static short	NumDigButtons;

 
BOOL AutoAreaID(UINT NumNewPolyPoints,HANDLE hNewPolyPoints,LPSTR SaveUDI)
{   
	char	UDI[66]; 
	HPDPOINT	lpDPoint; 
	long	TotNum, Refno;   
	HIGHLIGHTDATA	HighlightData;
	
	*SaveUDI = 0;
	GetGlobalCVal ("[%NEW_AREA_UDI]",UDI,NULL);
	if (_fstricmp (UDI,"%AUTO")) 
		return TRUE;  
	ClearPolyOff ();
	lpDPoint = (HPDPOINT)GlobalLock (hNewPolyPoints);   
	AddAreaToOffsetFile (NumNewPolyPoints, lpDPoint);
	GlobalUnlock (hNewPolyPoints);
    ClearHighlightList (FALSE); 
	HighlightOnlyPoints = TRUE; 
	HighlightInArea (CurView->hWnd,NULL,TRUE,FALSE);   
	HighlightOnlyPoints = FALSE;
	ClearPolyOff (); 
	TotNum = BT_NUM_IN_INDEX (hHighlight);
	if (TotNum != 1) 
	{    
		char	Msg[32]="Multiple points picked";
		
	    ClearHighlightList (FALSE); 
		if (!TotNum)
			_fstrcpy (Msg,"No points picked");
		GSSiMessageBox (Msg,"Error in Auto Area Identifier",MB_ICONEXCLAMATION);
		return FALSE;
	}
	BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&HighlightData);
    ClearHighlightList (FALSE); 
    SetGlobalValue ("%NEW_AREA_UDI",HighlightData.PD.UDI); 
    _fstrcpy (SaveUDI,"%AUTO");
	return TRUE;
}               

BOOL GetColorName (COLORREF Color,LPSTR Name)
{
	if (!GetListValue ("[%DL]colordef.txt", Color, Name))
		*Name = 0;
	return TRUE;
}

COLORREF GetColorFromName (LPSTR Name)
{
	COLORREF	Color=0;
	long		lColor;  

	lColor = GetListNum ("[%DL]colordef.txt", Name);  
	if (lColor == -1)
		Color = 0;
	else
		Color = lColor;
	return Color;
}
	
BOOL GPSTracking (short Opt)//0=off,1=on,2=toggle
{   
	BOOL	rtn=TRUE;
	
	if (Opt == 2)
		Opt = 1-TrackingStatus;
	switch (Opt)
	{
		case 0:
			CloseDigConnection ();
			TrackingStatus = 0; 
			if (DisplayNMEA)
				SetWindowText (hWndMain,"GPS Tracking Disabled");
			break;
		case 1:			 
			TrackingStatus = 0;
   			if (!OpenDigConnection(hWndMain))
   				rtn = FALSE;
   			TrackingStatus = 1; 
			SetWindowText (hWndMain,"GPS Tracking Enabled");  
			break;
   	} 
   	if (TrackingStatus)
	{
	    if (GetGlobalBVal2 ("[%DIGISGPS]",FALSE))
   			SetGlobalValue ("%AVLFOLLOW","GPS"); 
	}   	
   	else
   		SetGlobalValue ("%AVLFOLLOW","");
 	return rtn;       
} 
 
void ExtendPoly (long npnts, HPDPOINT pPoints,double indist)
{   
	double AZ, dist=fabs (indist);
	DPOINT	NewBPoint, NewEPoint;
	HPDPOINT	pBPoint= pPoints, pEPoint;
	 
	if (npnts <2 || !dist)
		return;
	AZ = getazd (pPoints,(pPoints+1));
	NewBPoint = dnewpt (*pPoints,AZ,-dist);
	pPoints += (npnts-1);
	pEPoint = pPoints;
	pPoints--;
	AZ = getazd (pPoints,pEPoint);
	NewEPoint = dnewpt (*pEPoint,AZ,dist);
	if (indist > 0) 
		*pBPoint = NewBPoint;
	*pEPoint = NewEPoint;
	return;
}

DPOINT GetButtonWorldCoord (POINT MousePoint,WORD wParam)
{
	if (wParam & MK_DIGITIZER_BUTTON)
		return CurrentPoint;
	else
		return (WinPtToBasePt(MousePoint));
}

short GetDigButtonID (LPSTR bInput)
{   
	short	i;
	
	for (i=0;i<NumDigButtons+1;i++)
	{
		if (!_fstrncmp (bInput,ButtonCodes[i],ButtonLen))
			return i;
	}
	return 0;
}  

void UnlockCursor (void)
{  
	if (CursorIsLocked)
	{
		CursorIsLocked = FALSE; 
		CreateDigCursor (CurView->hDC); 
	} 
	EnlargeScreen (0,0);
	return;
} 

void LockCursor (LPDPOINT lpDPoint)
{   
	CurrentPoint = *lpDPoint;
	CursorIsLocked = TRUE;  
	return;
} 

void SetDigWorldControlPoint (DPOINT DPoint)
{
	if (WantDigWorldCtlPnt && hWndDigControl)
	{   
		DigWorldControlPoint = DPoint;
		PostMessage(hWndDigControl, WM_COMMAND, IDC_SETWORLDCOORD, 0L);
	}
	return;
}

BOOL ProcessCOMMNotification( HWND hWnd, WORD wParam, LONG lParam )
{
   char       szError[ 10 ] ;
   int        nError, nLength,ii, LenInput ;
   COMSTAT    ComStat ;
   MSG        msg ;    
   char	str[256];   
   HANDLE	hInput;
   LPSTR	pInput, bInput, pCmd,pID,EndID;
   BOOL		rtn=TRUE;
   DPOINT	BasePoint; 
   POINT	CursorPoint;
   
   hInput = GSSiGlobAlloc (GMEM_MOVEABLE,RxQueSize+8); 
   bInput = pInput = GlobalLock (hInput);
   _fstrcpy (pInput,IncompleteDigInput); 
   pInput = _fstrchr (pInput,0);  
   LenInput = _fstrlen (IncompleteDigInput)+1;
   if (DigEventMode)
   {
      // verify that it is a COMM event specified by our mask

/*      if (CN_EVENT & LOWORD( lParam ) != CN_EVENT)
      {
         rtn=FALSE;
         goto Exit;
      }
*/
      // reset the event word so we are notified
      // when the next event occurs

      //GetCommEventMask( DigitizerStream, EV_RXFLAG|EV_ERR ) ;

      // We loop here since it is highly likely that the buffer
      // can been filled while we are reading this block.  This
      // is especially true when operating at high baud rates
      // (e.g. >= 9600 baud).

      do
      {  
      	UINT ComEvent = GetCommEventMask( DigitizerStream, EV_RXFLAG|EV_ERR|EV_BREAK );
      	if (EV_RXFLAG & ComEvent)
      		ii=1;
      	if (EV_ERR & ComEvent || EV_BREAK & ComEvent)
      	{   
      		short ier = GetCommError (DigitizerStream,NULL); 
      		FlushComm (DigitizerStream,1);
      		goto NextRec;
      	}
		 if ((nLength = ReadComm( DigitizerStream, pInput, RxQueSize-LenInput ))<0)
		 	ii=1;
		 else
		 {
			 LenInput += nLength; 
			 pInput += nLength;  
			 *pInput = 0; 
			 if (nLength)
			 {
			 	LPSTR	pEndRec;  
			 	short	lRec, x,y, Button;  
			 	POINT	WindowPoint;
			 	
			 	while ((pEndRec = _fstrchr (bInput,EndOfRecChar)))
			 	{   
//				    SetViewport(*pCommandViewport);
			 	  *pEndRec=0;//temp for setwindowtext
			 	  	pEndRec++;
			 		lRec = pEndRec - bInput;            
			 		LenInput -= lRec; 
			 		if (_fstrlen (bInput) > 0 && DisplayNMEA)
			 			SetWindowText (hWndMain,bInput); 
			 		if (GPSInput)
			 		{   
						KillTimer (hWndMain,GPSTIMER);
			 			if (lRec > 5)
			 			{   
		 					LPSTR	pLat,pLon,pElev;
		 					double Elev = DBL_MAX;
		 					short	i;   
		 					long	iElev;
		 					char	cElev[16];
		 					
							pLat=bInput+7;
			 				if (!_fstrnicmp (bInput,"$GPGGA,",7))
			 				{
			 					pLat=_fstrchr (pLat,',');
			 					pLat++;  
			 					pElev = _fstrrchr (bInput,','); 
			 					i=5;            
			 					
			 					while (i)
			 					{ 
			 						pElev--;
			 						if (*pElev == ',')
			 							i--;
			 					}
			 					pElev++;
			 					Elev = atof (pElev) * MFT; 
			 					iElev = IDNINT (Elev);
			 					sprintf (cElev,"%6.6ld",iElev);
			 					SetWindowText (hWndMain,cElev);
			 ;              }
			 				else if (_fstrnicmp (bInput,"$GPGLL,",7))
			 					goto Next;
			 				{   
			 					double	lat,lon, latdeg, latmin, londeg, lonmin; 
			 					short	ii;  
			 					DPOINT	DPoint;
			 					
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
			 					ii=1;            
			 					DPoint.x = -lon;
			 					DPoint.y = lat;
			 					ConvertCoord (&DPoint,2,1);
 								SetVehicleLoc ("GPS",DPoint,0,0); 

			 				} 
			 		Next:;
			 			}
				 		SetTimer (hWndMain,GPSTIMER,5000,NULL);
			 				
			 		}
			 		else
			 		{ 
				 		CurrentDigPoint.x = ldread (IADDR(bInput,DigXBegin),DigXLen);
				 		CurrentDigPoint.y = ldread (IADDR(bInput,DigYBegin),DigYLen); 
				 		Button = GetDigButtonID (bInput);
				 		if (!Button || (Button != LastButton))
				 		{   
				 			LastButton = Button;
				 			switch (Button)
				 			{   
				 				case 1:
									if (WantDigDigCtlPnt && hWndDigControl)
									{   
										PostMessage(hWndDigControl, WM_COMMAND, IDC_SETDIGITIZERCOORD, 0L);
										return TRUE;
									}
									if (!hTranDigToWorld)
										break; 
									if (!CursorIsLocked)
										CurrentPoint = TranPoint (&CurrentDigPoint,hTranDigToWorld);   
									WindowPoint = BasePtToWinPt (&CurrentPoint);  
	       	 						SendMessage (hWndMain,WM_LBUTTONDOWN,MK_DIGITIZER_BUTTON,
			    							  			  MAKELPARAM(WindowPoint.x,WindowPoint.y));  
	       	 						SendMessage (hWndMain,WM_LBUTTONUP,MK_DIGITIZER_BUTTON,
			    							  			  MAKELPARAM(WindowPoint.x,WindowPoint.y));  
									break; 
								
				 				case 2:
									if (!hTranDigToWorld)
										break;  
									if (!CursorIsLocked)
										CurrentPoint = TranPoint (&CurrentDigPoint,hTranDigToWorld);   
									WindowPoint = BasePtToWinPt (&CurrentPoint);  
	       	 						SendMessage (hWndMain,WM_MBUTTONDOWN,MK_DIGITIZER_BUTTON,
			    							  			  MAKELPARAM(WindowPoint.x,WindowPoint.y));  
	       	 						SendMessage (hWndMain,WM_MBUTTONUP,MK_DIGITIZER_BUTTON,
			    							  			  MAKELPARAM(WindowPoint.x,WindowPoint.y));  
									break;  
									
				 				case 3:
									if (!hTranDigToWorld)
										break; 
									if (!CursorIsLocked)
										CurrentPoint = TranPoint (&CurrentDigPoint,hTranDigToWorld); 
									WindowPoint = BasePtToWinPt (&CurrentPoint);  
	       	 						SendMessage (hWndMain,WM_RBUTTONDOWN,MK_DIGITIZER_BUTTON,
			    							  			  MAKELPARAM(WindowPoint.x,WindowPoint.y));  
	       	 						SendMessage (hWndMain,WM_RBUTTONUP,MK_DIGITIZER_BUTTON,
			    							  			  MAKELPARAM(WindowPoint.x,WindowPoint.y));  
									break;  
									
								case 0:
									if (!hTranDigToWorld)
										break; 
									BasePoint = TranPoint (&CurrentDigPoint,hTranDigToWorld); 
									WindowPoint = BasePtToWinPt (&BasePoint);
									CursorPoint = WindowPoint;
									ClientToScreen (hWndMain,&CursorPoint);  
									if (!CursorIsLocked)
							 			SetCursorPosGM (CursorPoint.x,CursorPoint.y,0);
						    		SendMessage(hWndMain, WM_MOUSEMOVE, MK_DIGITIZER_BUTTON,
						    							  MAKELPARAM(WindowPoint.x,WindowPoint.y));  
								    break;
								
								default:
									if (!hDigButtonCmd[Button])
										break; 
									pCmd = GlobalLock (hDigButtonCmd[Button]);
									HaveCurrentLBUTTON = 2;    
									if (!CursorIsLocked)
										CurrentPoint = TranPoint (&CurrentDigPoint,hTranDigToWorld); 
									WindowPoint = BasePtToWinPt (&CurrentPoint);
							 		CurrentLBUTDOWNLoc = WindowPoint;
									CursorPoint = WindowPoint;
									ClientToScreen (hWndMain,&CursorPoint);  
									if (!CursorIsLocked)
							 			SetCursorPosGM (CursorPoint.x,CursorPoint.y,0);
								    AddGraphicsCmd (hWndMain,pCmd,TRUE,0);  
								    GlobalUnlock (hDigButtonCmd[Button]);  
								    break;
				 			}
				 		} 
				 	}
			 		_fmemmove (bInput,pEndRec,LenInput);  
			 		pInput -= lRec;  
			 	}
			 } 
		}
NextRec:;
      }
      while (/*PeekMessage( &msg, NULL, WM_COMMNOTIFY,WM_COMMNOTIFY, PM_REMOVE) ||*/
             (nLength > 0)) ;
      if (LenInput)
      	_fstrcpy (IncompleteDigInput,bInput);
   }
   else
   {
      // verify that it is a receive event

      if (CN_RECEIVE & LOWORD( lParam ) != CN_RECEIVE)  
      {
         rtn=FALSE;
         goto Exit;
      }
      do
      {
         if (nLength = ReadComm( DigitizerStream, str, sizeof(str) ))
         {
         }
         if (nError = GetCommError( DigitizerStream, &ComStat ))
         {
         }
      }
      while ((!PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE )) ||
            (ComStat.cbInQue >= MAXBLOCK)) ;
   } 
Exit:
	GSSiGlobUlFree (&hInput);
   return ( rtn ) ;

}  

BOOL OpenDigConnection( HWND hWnd )
{
   char       DigPort[ 10 ], szTemp[ 10 ], DigName[32], DigInitString[256] ;
   BOOL       fRetVal ;
   char		IniName[128]="geomastr.ini";
   BOOL		rtn=FALSE;
   int		ii;
   LPSTR	lpComma, lpStart;
    
    GPSClose ();
	CloseDigConnection();  
	LastButton = 0;
   	ExpandText (IniName);
    if (GetGlobalBVal2 ("[%DIGISGPS]",FALSE))
    {   
    	GPSInput = TRUE;
		GetPrivateProfileString ("GPS","Port","",DigPort,32,IniName);   
		GetPrivateProfileString ("GPS","DisplayNMEA","Y",DigInitString,2,"geomastr.ini"); 
		if (*DigInitString == 'Y')
			DisplayNMEA = TRUE;
		else
			DisplayNMEA = FALSE;
		_fstrcpy (DigName,"GPS");
	}
    else
    {   
    	GPSInput = FALSE;
		GetPrivateProfileString ("CurrentDigitizer","Name","",DigName,sizeof(DigName),IniName);   
		sprintf (DigInitString,"DigName %s",DigName);
		GSSiTrace (DigInitString);
		GetPrivateProfileString ("CurrentDigitizer","Port","",DigPort,sizeof(DigPort),IniName);  
		sprintf (DigInitString,"Port %s",DigPort);
		GSSiTrace (DigInitString);
	}
	RxQueSize=GetPrivateProfileInt ("CurrentDigitizer","ReceiveQueSize",4096,IniName);  
    DigitizerStream = OpenComm( DigPort, RxQueSize, 256);  
	sprintf (DigInitString,"DigStream %i",DigitizerStream);
	GSSiTrace (DigInitString);
    if (DigitizerStream < 0)  
   {    
    
   		switch (DigitizerStream)
   		{
   		
			case IE_BADID:
				_fstrcpy(DigInitString,"The device identifier is invalid or unsupported.");
				break;
			case IE_BAUDRATE:
				_fstrcpy(DigInitString,"The device's baud rate is unsupported.");
				break;
			case IE_BYTESIZE:
				_fstrcpy(DigInitString,"The specified byte size is invalid.");
				break;
			case IE_DEFAULT:
				_fstrcpy(DigInitString,"The default parameters are in error."); 
				break;
			case IE_HARDWARE:
				_fstrcpy(DigInitString,"The hardware is not available (is locked by another device).");
				break;
			case IE_MEMORY:
				_fstrcpy(DigInitString,"The function cannot allocate the queues."); 
				break;
			case IE_NOPEN:
				_fstrcpy(DigInitString,"The device is not open."); 
				break;
			case IE_OPEN:
				_fstrcpy(DigInitString,"The device is already open."); 
                break;
      	} 
      	sprintf (IniName,"Error opening digitizer on %s",DigPort);
      	MessageBox (hWnd,DigInitString,IniName,MB_ICONEXCLAMATION);
        return ( FALSE ) ;
    } 
   	FlushComm(DigitizerStream, 0);
	FlushComm(DigitizerStream, 1);  
	GetCommError (DigitizerStream,NULL);
    GetCommEventMask( DigitizerStream, EV_RXFLAG|EV_ERR|EV_BREAK );
	if (SetupDigConnection(DigName, IniName))
	{
      // set up notifications from COMM.DRV

         // In this case we really are only using the notifications
         // for the received characters - it could be expanded to
         // cover the changes in CD or other status lines.

		SetCommEventMask( DigitizerStream, EV_RXFLAG|EV_ERR|EV_BREAK ) ;

         // Enable notifications for events only.

         // NB:  This method does not use the specific
         // in/out queue triggers.

        ii=EnableCommNotification( DigitizerStream, hWnd, -1, -1 ) ;
      // assert DTR

      ii=EscapeCommFunction( DigitizerStream, SETDTR ) ;  
      if (GPSInput)
		SetTimer (hWndMain,GPSTIMER,5000,NULL);
	  GetPrivateProfileString (DigName,"InitString","",DigInitString,sizeof(DigInitString),IniName); 
	  ExpandText (DigInitString);
	  GSSiTrace (DigInitString); 
	  lpStart = DigInitString;
	  rtn = TRUE; 
	  if (_fstrlen(lpStart))
	  while (lpStart)
	  {
		  lpComma = _fstrchr (lpStart,','); 
		  if (lpComma)
		  	*lpComma++=0;
	      if ((ii=WriteComm( DigitizerStream, lpStart,_fstrlen(lpStart)))<0 ) 
	      {
	      	MessageBox (GetFocus(),"Unable to send digitizer initialization string",NULL,MB_ICONEXCLAMATION);
	      	rtn=FALSE; 
	      }
      	  lpStart = lpComma; 
	      Sleep (500);
	  } 
	  hDigQue=GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX);
	  DigQueLen=0; 
	  *IncompleteDigInput=0;
   }
   else
   {  
   	  MessageBox (hWnd,"Error setting digitizer parameters","Unable to open digitizer",MB_ICONEXCLAMATION);
      CloseComm(DigitizerStream) ;
      DigitizerStream=-1;
   }

   return ( rtn ) ;

}

BOOL OpenDigControlDialog (HWND hWnd)
{
    FARPROC	lpfnDIGCONTROLMsgProc; 

	if (DigitizerStream < 0)
	{
		MessageBox (GetFocus(),"The digitizer is not open",NULL,MB_ICONEXCLAMATION);
		return FALSE;
	}
    
    if (hWndDigControl)
    	return TRUE;
	lpfnDIGCONTROLMsgProc = MakeProcInstance((FARPROC)DIGCONTROLMsgProc, hInst);
	CreateDialog(hInst, (LPSTR)"DIGCONTROL", hWnd, lpfnDIGCONTROLMsgProc);
	return TRUE;
}

BOOL FAR PASCAL DIGCONTROLMsgProc(HWND hWndDlg, WORD Message, WPARAM wParam, LPARAM lParam)
{ 
	static	short	Choice;
	static	BOOL	AutoDig=FALSE;
	BOOL	Show; 
	LPDIGCNTLPOINT pDigCtlPnt;	
	short	TabStops[4]={20,30,40,1000}, i, npts;
	char	str[128];  
	LPDOUBLE XDIG,YDIG,XBASE,YBASE;   
	HANDLE	hCoord;   
	float	RSQMIN;   
	OFSTRUCT	OFStruct;
	HFILE	FidCntl;
    
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
    	hWndDigControl = hWndDlg;
		SendDlgItemMessage (hWndDlg,IDC_CONTROLPNTS,LB_SETTABSTOPS,4,(LPARAM)&TabStops);
		FidCntl = GSSiOpenFile ("digsetup.bin",&OFStruct,OF_READ);
		if (FidCntl != HFILE_ERROR)
		{
			_lread (FidCntl,&nDigControlPoints,2);
			hDigControlPoints = GSSiGlobAlloc (GHND,UINT_MAX);  
	        pDigCtlPnt = (LPDIGCNTLPOINT)GlobalLock (hDigControlPoints); 
	        _lread (FidCntl,pDigCtlPnt,nDigControlPoints*sizeof(DIGCNTLPOINT));
	        GSSiClose (FidCntl);  
	        GlobalUnlock (hDigControlPoints);
	    }
    case GSSI_REINITDIALOG:
		SendDlgItemMessage (hWndDlg,IDC_CONTROLPNTS,LB_RESETCONTENT,0,0); 
		if (!hDigControlPoints)
		{
			hDigControlPoints = GSSiGlobAlloc (GHND,UINT_MAX);  
			nDigControlPoints = 0;
			Choice = -1;  
            EnableWindow (GetDlgItem(hWndDlg,IDC_SETDIGITIZERCOORDAUTO),FALSE); 
		}
		else
		{ 
            EnableWindow (GetDlgItem(hWndDlg,IDC_SETDIGITIZERCOORDAUTO),TRUE); 
			hDigControlPoints = GlobalReAlloc (hDigControlPoints,UINT_MAX,GHND);
		} 
        pDigCtlPnt = (LPDIGCNTLPOINT)GlobalLock (hDigControlPoints);  
        hCoord = GSSiGlobAlloc (GMEM_MOVEABLE,(1+nDigControlPoints)*4*sizeof(double));
        XDIG = (LPDOUBLE)GlobalLock (hCoord);
        YDIG = XDIG + nDigControlPoints; 
        XBASE = YDIG + nDigControlPoints; 
        YBASE = XBASE + nDigControlPoints; 
        npts = 0;
		DisplayMarkers = TRUE;
		for (i=0;i<nDigControlPoints;i++,pDigCtlPnt++)
		{	
			sprintf (str,"%i\t%c\t%c",i+1,pDigCtlPnt->HaveWorld,pDigCtlPnt->HaveDig);
			SendDlgItemMessage (hWndDlg,IDC_CONTROLPNTS,LB_ADDSTRING,NULL,(LPARAM)str);   
			if (pDigCtlPnt->HaveWorld != ' ' && pDigCtlPnt->HaveDig != ' ')
			{   
				*(XDIG+npts) = pDigCtlPnt->DigitizerPoint.x;
				*(YDIG+npts) = pDigCtlPnt->DigitizerPoint.y;
				*(XBASE+npts) = pDigCtlPnt->WorldPoint.x;
				*(YBASE+npts) = pDigCtlPnt->WorldPoint.y;
				npts++;
			}
			*_fstrchr (str,'\t') = 0;  
			if (pDigCtlPnt->HaveWorld != ' ')
				DisplayMarker (pDigCtlPnt->WorldPoint,1,str,0.20,0,0,FALSE,FALSE,NULL,NULL);
		}
		DisplayMarkers = FALSE;           
		GlobalUnlock (hDigControlPoints); 
		CloseTRANS2 (&hTranDigToWorld);  
		if (npts > 2)  
		{
			hTranDigToWorld = STRAN2 (XDIG,YDIG,XBASE,YBASE,npts,&RSQMIN,1,NULL); 
			sprintf (str,"RSQ = %f",RSQMIN);
			SetDlgItemText (hWndDlg,IDC_MESS,str);
		}
		else
			SetDlgItemText (hWndDlg,IDC_MESS,"Digitizer transformation not set");
		GSSiGlobUlFree (&hCoord);
 		SendDlgItemMessage (hWndDlg,IDC_CONTROLPNTS,LB_SETCURSEL,(WPARAM)Choice,(LPARAM)NULL); 
		if (Choice < 0)
		{
			EnableWindow (GetDlgItem(hWndDlg,IDC_SETWORLDCOORD),FALSE);
			EnableWindow (GetDlgItem(hWndDlg,IDC_SETDIGITIZERCOORD),FALSE);
			EnableWindow (GetDlgItem(hWndDlg,IDC_DELETE_CP),FALSE);  
		} 
		
        break;  
   
    case WM_DESTROY:
		 if (!nDigControlPoints)
		 {
			GSSiGlobFree (&hDigControlPoints);
			GSSiRemove ("digsetup.bin");
		 }
		 else 
		 {
			FidCntl = GSSiOpenFile ("digsetup.bin",&OFStruct,OF_CREATE);
			BigWrite (FidCntl,(HPSTR)&nDigControlPoints,2,-1);
	        pDigCtlPnt = (LPDIGCNTLPOINT)GlobalLock (hDigControlPoints); 
	        BigWrite (FidCntl,(HPSTR)pDigCtlPnt,nDigControlPoints*sizeof(DIGCNTLPOINT),-1);
	        GSSiClose (FidCntl);  
	        GSSiGlobUlFree (&hDigControlPoints);
	     }
		 	   
         hWndDigControl = 0; 
         WantDigWorldCtlPnt = FALSE;
         WantDigDigCtlPnt = FALSE;
    	 return 0;
    	 
    case WM_CLOSE:
		 DestroyWindow (hWndDlg);
         break;  

    case WM_COMMAND:
#if WIN32
         switch(LOWORD(wParam))
#else
         switch(wParam)
#endif
         {  
            case IDOK: 
		         PostMessage(hWndDlg, WM_CLOSE, 0, 0L);
            	 break; 
            	 
            case IDC_ADD_CP: 
	             pDigCtlPnt = (LPDIGCNTLPOINT)GlobalLock (hDigControlPoints);
	             pDigCtlPnt+= nDigControlPoints; 
	             pDigCtlPnt->HaveWorld = pDigCtlPnt->HaveDig = ' ';  
	             GlobalUnlock (hDigControlPoints);
            	 Choice = nDigControlPoints++; 
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
            	 
            	 break;
            
            case IDC_DELETE_CP: 
		         pDigCtlPnt = (LPDIGCNTLPOINT)GlobalLock (hDigControlPoints); 
				 for (i=0;i<nDigControlPoints-1;i++,pDigCtlPnt++)
		  		 {
		  		 	if (i >= Choice)
		  		 		*pDigCtlPnt = *(pDigCtlPnt+1);
				 }
            	 nDigControlPoints--;
				 Choice = -1;
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
            	 
            	 break;
            
            case IDC_SETWORLDCOORD: 
            	 if (WantDigWorldCtlPnt)
            	 {
	            	 pDigCtlPnt = (LPDIGCNTLPOINT)GlobalLock (hDigControlPoints);
	            	 pDigCtlPnt+= (Choice);
	            	 pDigCtlPnt->WorldPoint = DigWorldControlPoint;      
	            	 pDigCtlPnt->HaveWorld = 'W';
	            	 GlobalUnlock (hDigControlPoints);
                     EnableWindow (GetDlgItem(hWndDlg,IDC_SETWORLDCOORD),TRUE); 
                     WantDigWorldCtlPnt = FALSE;
                 	 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
				 }
				 else
				 { 
					 SetDlgItemText (hWndDlg,IDC_MESS,"Waiting for world control point");
				 	 WantDigWorldCtlPnt = TRUE;
                     EnableWindow (GetDlgItem(hWndDlg,IDC_SETWORLDCOORD),FALSE);
				 }
            	 break;
            	 
            case IDC_SETDIGITIZERCOORDAUTO:
            	 Choice = 0;
            	 AutoDig = TRUE; 
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);  
				 PostMessage(hWndDigControl, WM_COMMAND, IDC_SETDIGITIZERCOORD, 0L);
            	 break;
            	 
            case IDC_SETDIGITIZERCOORD: 
            	 if (WantDigDigCtlPnt)
            	 {
	            	 pDigCtlPnt = (LPDIGCNTLPOINT)GlobalLock (hDigControlPoints);
	            	 pDigCtlPnt+= (Choice);
	            	 pDigCtlPnt->DigitizerPoint = CurrentDigPoint;      
	            	 pDigCtlPnt->HaveDig = 'D';
	            	 GlobalUnlock (hDigControlPoints);
                     EnableWindow (GetDlgItem(hWndDlg,IDC_SETDIGITIZERCOORD),TRUE); 
                     WantDigDigCtlPnt = FALSE;
                 	 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);  
                 	 if (AutoDig)
                 	 {
                 	 	Choice++;
                 	 	if (Choice < nDigControlPoints) 
                 	 	{   
                 	 		sprintf (str,"Locate digitizer point %i",Choice+1);
                 	 		SetDlgItemText (hWndDlg,IDC_MESS2,str);
				 			PostMessage(hWndDigControl, WM_COMMAND, IDC_SETDIGITIZERCOORD, 0L);
				 		}
                 	 	else
                 	 	{
                 	 		SetDlgItemText (hWndDlg,IDC_MESS2,"");
                 	 		AutoDig = FALSE;
                 	 		Choice = -1;  
                 	 	}
                 	 }
				 }
				 else
				 { 
					 SetDlgItemText (hWndDlg,IDC_MESS,"Waiting for digitizer control point");
				 	 WantDigDigCtlPnt = TRUE;
                     EnableWindow (GetDlgItem(hWndDlg,IDC_SETDIGITIZERCOORD),FALSE);
				 }
            	 break;
            
            case IDC_CLEAR_CP:
				 if (MessageBox( GetFocus(), "Are you sure you wish to clear the current digitizer setup?","Verify Clear",
				     MB_OKCANCEL) == IDCANCEL) break;
            	 GSSiGlobUlFree (&hDigControlPoints);
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
            	 break;
            	 	 
            case IDC_CONTROLPNTS:
                 switch(HIWORD(lParam))
                 {   
                     case LBN_SELCHANGE: 
                         Choice=(short)SendDlgItemMessage(hWndDlg,wParam,LB_GETCURSEL,0,0);
                         if (Choice >= 0)
                         	Show=TRUE;
                         else
                         	Show=FALSE;
                         EnableWindow (GetDlgItem(hWndDlg,IDC_SETWORLDCOORD),Show);
                         EnableWindow (GetDlgItem(hWndDlg,IDC_SETDIGITIZERCOORD),Show);
                         EnableWindow (GetDlgItem(hWndDlg,IDC_DELETE_CP),Show);
	                     WantDigWorldCtlPnt = FALSE;
                         WantDigDigCtlPnt = FALSE;	
		        		 break;
		         }
		         break;
                 
                              
                 
          }
          break;

    default:
        return FALSE;
   }
 return TRUE;    
} 

UINT GetBaud (LPSTR str)
{   
	UINT BaudRate=0;
	
	if (!_fstricmp (str,"2400")) 
		BaudRate = CBR_2400;
	else if (!_fstricmp (str,"4800")) 
		BaudRate = CBR_4800;
	else if (!_fstricmp (str,"9600")) 
		BaudRate = CBR_9600;
	else if (!_fstricmp (str,"19200")) 
		BaudRate = CBR_19200;
	else
		GSSiMessageBox (str,"Unsupported baud rate",MB_ICONEXCLAMATION);
    return BaudRate;
}

BOOL SetupDigConnection(LPSTR DigName, LPSTR IniName)
{
   int		rtn,i ;
   BYTE       bSet, SaveID ;
   DCB        dcb ;
   char			str[128]; 
   LPSTR	pCmd;

	GetCommState (DigitizerStream,&dcb);
	SaveID = dcb.Id;
	_fmemset (&dcb,0,sizeof(DCB));	  
	dcb.Id = SaveID;
	GetPrivateProfileString (DigName,"Baud","9600",str,32,"geomastr.ini");  
	dcb.BaudRate = GetBaud (str); 
	dcb.ByteSize = GetPrivateProfileInt (DigName,"ByteSize",8,IniName);
	dcb.Parity = GetPrivateProfileInt (DigName,"Parity",NOPARITY,IniName);
	dcb.StopBits = GetPrivateProfileInt (DigName,"StopBits",ONESTOPBIT,IniName);
	DigXBegin = GetPrivateProfileInt (DigName,"XBegin",3,IniName);
	DigXLen = GetPrivateProfileInt (DigName,"XLength",5,IniName);
	DigYBegin = GetPrivateProfileInt (DigName,"YBegin",8,IniName);
	DigYLen = GetPrivateProfileInt (DigName,"YLength",5,IniName);
	ButtonBegin = GetPrivateProfileInt (DigName,"ButtonBegin",1,IniName);
	ButtonLen = GetPrivateProfileInt (DigName,"ButtonLength",2,IniName);
	dcb.fOutxCtsFlow = dcb.fRtsflow = 0;
	dcb.CtsTimeout = 0;
    dcb.fOutxDsrFlow = dcb.fDtrflow = 0;
	dcb.DsrTimeout = 0;
	dcb.fInX = dcb.fOutX = 0;
	GetPrivateProfileString (DigName,"FlowControl","RTSCTS",str,sizeof(str),IniName); 
	switch (*str)
	{   
		case 'H':
		case 'h':
   		case 'R':
   		case 'r':
   			dcb.fOutxCtsFlow = dcb.fRtsflow = 1;
   			dcb.CtsTimeout = 100;
   			break;    
   		case 'D':
   		case 'd':
		    dcb.fOutxDsrFlow = dcb.fDtrflow = 1;
   			dcb.DsrTimeout = 100;
   		    break;
   		case 'X':
   		case 'x':
			dcb.fInX = dcb.fOutX = 1 ;
			dcb.XonChar = ASCII_XON ;
			dcb.XoffChar = ASCII_XOFF ;
			dcb.XonLim = 100 ;
			dcb.XoffLim = 100 ; 
			break; 
		case 'N':
		case 'n':
		default:
			break;
	}
	dcb.XonChar = ASCII_XON ;
	dcb.XoffChar = ASCII_XOFF ;
	dcb.XonLim = 100 ;
	dcb.XoffLim = 100 ; 
	NumDigButtons = GetPrivateProfileInt (DigName,"NumButtons",0,IniName);
	NumDigButtons = min(16,NumDigButtons);
	GetPrivateProfileString (DigName,"MoveCode","",ButtonCodes[0],4,IniName); 
	for (i=1;i<NumDigButtons+1;i++)
	{
		sprintf (str,"ButtonCode%i",i); 
		GetPrivateProfileString (DigName,str,"",ButtonCodes[i],4,IniName);
		sprintf (str,"ButtonCommand%i",i); 
		hDigButtonCmd[i] = GSSiGlobAlloc (GHND,256);
		pCmd = GlobalLock (hDigButtonCmd[i]); 
		if (!GetPrivateProfileString (DigName,str,"",pCmd,255,IniName))
			GSSiGlobUlFree (&hDigButtonCmd[i]); 
		else
			GlobalUnlock (hDigButtonCmd[i]);
	}

   // other various settings

   dcb.fBinary = TRUE ;
   dcb.fParity = TRUE ;
   dcb.fRtsDisable = FALSE ;
   dcb.fDtrDisable = FALSE ; 
   dcb.fChEvt = TRUE;  
   
   GetPrivateProfileString (DigName,"EndOfRecChar","LF",str,sizeof(str),IniName); 
   switch (*str)
   {
   		case 'C':
   		case 'c':
    
		   EndOfRecChar=ASCII_CR; 
		   dcb.EvtChar = ASCII_CR;
		   break;
		default:
		   EndOfRecChar=ASCII_LF; 
		   dcb.EvtChar = ASCII_LF;  
		   break;
   }

   rtn = SetCommState( &dcb );
   
   if (rtn < 0)
   		return FALSE;  
//   DumpDCB (DigitizerStream);
   return TRUE;  
   

} 

BOOL CloseDigConnection( void)
{  
	short	i;
	
   if (DigitizerStream < 0)
   		return FALSE; 
   if (GPSInput)
		KillTimer (hWndMain,GPSTIMER);
   EnableCommNotification( DigitizerStream, NULL, -1, -1 ) ;

   // kill the focus


   EscapeCommFunction( DigitizerStream, CLRDTR ) ;

   // close comm connection

   CloseComm( DigitizerStream ) ;
   DigitizerStream = -1; 
   GSSiGlobFree (&hDigQue);
   for (i=1;i<NumDigButtons+1;i++)
		GSSiGlobUlFree (&hDigButtonCmd[i]); 
   GSSiTrace ("Dig Closed");
   return ( TRUE ) ;

} 
 
BOOL WriteCommByte( HWND hWnd, BYTE bByte )
{

	MSG	msg;

   WriteComm( DigitizerStream, (LPSTR) &bByte, 1 ) ;
   while (!PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE));

   return ( TRUE ) ;

}  

BOOL WMASetUp (void)
{
	char	File[128]="c:\\wma\\submaps\\[PRIMMAP].txt";
	char	str[132]="[MAPPIECE]";
	char	TRS[16]; 
	long	Refno;
	HFILE	Fid, FidCntl, FidOut;
	short	Piece, WantPiece, i, NumPieces;          
	LPDIGCNTLPOINT pDigCtlPnt;	
	OFSTRUCT	OFStruct;
	MNMXCORD Bounds; 
	double	InflateValue; 
	HANDLE	hDLT;
	
	DBoundsInit (&Bounds);
/*	Fid = GSSiOpenFile (File,&OFStruct,OF_READ);
	ExpandText (str);
	WantPiece = atoi (str); 
	ProcessDelimTextHeader("\"PIECE\",\"SECT1\",\"SECT2\",\"SECT3\",\"SECT4\"");
	while (fgetstring (str,128,Fid))
	{
		Piece = atoi (str);
		if (Piece == WantPiece)
		{   
			GSSiClose (Fid);
			nDigControlPoints = 4;
 			hDigControlPoints = GSSiGlobAlloc (GHND,UINT_MAX); 
	        pDigCtlPnt = GlobalLock (hDigControlPoints); 
            GetDelimTextData(str);
            for (i=0;i<4;i++)
            {  
            	sprintf (str,"[SECT%i]",i+1);
	            ExpandText (str);
	            _fstrcpy (TRS,str);
	            TRS[3]='0';
	            _fstrcpy (&TRS[4],&str[3]);
	            if (!PickByRefno (Refno,"PLSS",TRS,-1))
	            	break;   
	            pDigCtlPnt->WorldPoint = UpperLeftCorner (0);
				AddDPointToMinMax (&pDigCtlPnt->WorldPoint,&Bounds);
	            pDigCtlPnt->HaveDig = ' '; 
	            pDigCtlPnt++->HaveWorld = 'W'; 
	        }
	        GlobalUnlock (hDigControlPoints);
	        pDigCtlPnt = GlobalLock (hDigControlPoints); 
			FidCntl = GSSiOpenFile ("digsetup.bin",&OFStruct,OF_CREATE);
			BigWrite (FidCntl,&nDigControlPoints,2);
	        pDigCtlPnt = GlobalLock (hDigControlPoints); 
	        BigWrite (FidCntl,pDigCtlPnt,nDigControlPoints*sizeof(DIGCNTLPOINT));
	        GSSiClose (FidCntl);  
	        GSSiGlobUlFree (&hDigControlPoints);  
	        InflateValue = 0.5 * max (Bounds.xmx - Bounds.xmn,Bounds.ymx - Bounds.ymn);
			InflateBounds (&Bounds,InflateValue);
			CreateNewMap ("[UPDATEMAP]",&Bounds,0,NULL,0,NULL); */
			CreateNewMap ("[UPDATEMAP]",&CurView->WBounds,0,NULL,0,NULL,0,0,TRUE); 
			_fstrcpy (File,"c:\\wma\\maps\\[PRIMMAP].txt");
			Fid = GSSiOpenFile (File,&OFStruct,OF_READ);
			_fstrcpy (File,"[WMALISTFILE]");
			FidOut = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	        fputstring ("* Not in List *",FidOut);
			ProcessDelimTextHeader("\"WMA\",\"ACRES\",\"NUMPIECES\"",&hDLT);
			while (fgetstring (str,128,Fid))
			{
	            GetDelimTextData(str,hDLT);
	            NumPieces = GetGlobalLVal ("[NUMPIECES]");
	            for (i=0;i<NumPieces;i++)
	            {
	            	sprintf (str,"[WMA](%i)",i+1);
	            	ExpandText (str);
	            	fputstring (str,FidOut);
	            }
			}
			GSSiGlobFree (&hDLT);
			GSSiClose (Fid);	
			GSSiClose (FidOut);
            PostMessage(hWndMain, WM_COMMAND, IDM_Z_EDITLIMITS, 0L);
	        return TRUE;
/*		}	
	}
	GSSiClose (Fid);
	MessageBox (GetFocus(),"Control not found",NULL,MB_ICONEXCLAMATION);
    GSSiGlobUlFree (&hDigControlPoints);
	return FALSE;  */
}  

DPOINT	UpperLeftCorner (short item)
{
   	LPTHEME		pTheme;    
   	DPOINT		RtnPoint;
   	double		midx;
   	short		i, nleft, nright;

    RtnPoint.x = PickList[item].Rect.xmn;   
    RtnPoint.y = PickList[item].Rect.ymx;   
    SetViewport (PickList[item].ViewID);
	pTheme = AddTheme (GF_SAVEPOLY_THEME);
	CurView->PassID = 2;
	ProcessSelectedTheme = CurView->NumThemes;
	ProcessPickedItem (item,FALSE);        		
	ProcessSelectedTheme = 0;
	DeleteTheme (pTheme);
	while (GetSavedPolys ())
	if (hSavePoly)
	{   LPMNMXCORD lpRect;
		LPDPOINT	lpDpoint;  
		short		nPnts;   
		DPOINT		leftPoint[4];
    		    
        nPnts = nSavePoly; 
        lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);  
        midx = (lpRect->xmn + lpRect->xmx)/2.0;
        lpRect++;
        lpDpoint = (LPDPOINT) lpRect; 
        nleft=nright=0; 
        for (i=0;i<4;i++,lpDpoint++)
        {
        	if (lpDpoint->x < midx)
        		leftPoint[nleft++] = *lpDpoint;
        }
	  	GSSiGlobUlFree (&hSavePoly); 
	  	if (nleft == 2)
	  	{
	  		if (leftPoint[0].y > leftPoint[1].y)
	  			RtnPoint = leftPoint[0];
	  		else
	  			RtnPoint = leftPoint[1];
	  	}		
    }
    return RtnPoint; 
}

POINT DrawCurPoints (BOOL Spline,long StartSpline,short Mode,LPPOINT Points)
{
	HPDPOINT	lpDPoint;
	HPPOINT	lpPoints, lpPoint; 
	HANDLE	hPoints;
	ULONG	i;
	long	nDisplayPoints;
	short	OldMode; 
	POINT	EndPoint;
	
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	CurView->hRgn = CreateVPRgn (FALSE,FALSE);
  	SelectClipRgn (CurView->hDC,CurView->hRgn);
  	DeleteObject(CurView->hRgn);
	nDisplayPoints = nCurPolyPoints;  
	if (Spline)
		nDisplayPoints = StartSpline;
	hPoints = GSSiGlobAlloc(GMEM_MOVEABLE,(long)(nDisplayPoints+1)*sizeof(POINT));
	lpPoints = (HPPOINT)GlobalLock(hPoints);
	lpPoint = lpPoints;
	lpDPoint = (HPDPOINT)GlobalLock(hCurPolyPoints); 
	for (i=0;i<nDisplayPoints;i++,lpPoint++,lpDPoint++)
	{
		*lpPoint = BasePtToWinPt(lpDPoint);
	}
	GlobalUnlock (hCurPolyPoints); 
	if (Points)
	{
		nCurPolyPoints++;
		*lpPoint = Points[1];
	} 
	else
		lpPoint--;
	EndPoint = *lpPoint;
	OldMode = SetROP2(CurView->hDC,Mode);
	if (TempLineType != 1) 
		TempPolyline (CurView->hDC,lpPoints,(short)(nDisplayPoints),NULL,NULL);  
	SetROP2(CurView->hDC,OldMode);
	GSSiGlobUlFree (&hPoints); 
	return EndPoint;
}  

BOOL CreatePoly (HWND hWnd, WORD Message, WORD wParam, LONG lParam,short Function)
{HDC hDC;
 char key;
 static	POINT	MousePoint;
 static	long	StartSpline;    
 static	POINT	POCScreen;    
 static	DPOINT	POC;
 HPPOINT	Points;
 HPDPOINT	lpDPoint;
 HPPOINT	lpPoints, lpPoint;
 HPDPOINT	lpNewPoint;  
 HANDLE	hPoints;
 DPOINT	BasePoint, EndPoint, AtPoint, LastPoint;
 static	BOOL	HaveStart;
 int	OldMode; 
 long	i;
 BOOL	EndPointConnected;
 long	NewTLID;
 static	BOOL	MapEdit=TRUE; 
 HPEN	hTrackPen, hOldPen; 
 double	Dist;  
 static	double	TotDist;  
 static	HCURSOR	InCursor;
 static	BOOL	HaveDownButton;
 char	txt[128];
 
 if (idTimer) return FALSE;    
 if (hTempPoints)
 	Points = (HPPOINT)GlobalLock (hTempPoints);
 switch (Message)
   {
   	case GF_INIT:
   		TotDist=0; 
   		InCreatePoly=TRUE;
   		HaveDownButton = FALSE; 
   		HavePOC = FALSE; 
   		HaveStart = FALSE;
   		Spline = GetGlobalBVal2 ("[%SPLINE]",FALSE);
   		if (!hCurPolyPoints)
   		{
   			hCurPolyPoints = GSSiGlobAlloc(GHND,MAX_DIGPOINTS*(long)sizeof(DPOINT));
   			nCurPolyPoints = 0;  
   			CurPolyVP = CurView;
   		}
   		else 
   		{
	 		MessageBox(GetFocus(), "Must complete current polyline before beginning a new one", NULL,MB_ICONEXCLAMATION|MB_OK);
	 		goto RtnFalse;
   		}
		InCursor=CurView->hCursor;
		SetCurs (hDigCursor,FALSE);
		hTempPoints = GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX); 
	 	Points = (HPPOINT)GlobalLock (hTempPoints);
   		nTempPoints = 0;
   		StartSpline = 1;  
   		GSSiGlobFree (&hNewPolyPoints);
   		NumNewPolyPoints = 0;
   		break;  
   		
   	case GF_COMPLETE:
       	HaveDownButton = FALSE;  
   		if (CursorIsLocked) 
   		{
			IgnoreMouseMove = FALSE;
			GetCursorPos (&MousePoint);
			ScreenToClient (hWnd,&MousePoint);  
		    PostMessage(hWnd, WM_MOUSEMOVE,0, MAKELPARAM (MousePoint.x,MousePoint.y));
		} 
		else
			SetCurs (2,FALSE); 
		if (wParam == GF_UNDO)
			goto DoBackspace;
		if (wParam == GF_LBUTTON)
		{
			lParam = MAKELONG(RLButLoc.x,RLButLoc.y);
	    	MousePoint = MAKEPOINT(lParam); 
	    	HaveDownButton = TRUE;
			goto LButUp; 
		}
		if (wParam == GF_RBUTTON)
		{
			lParam = MAKELONG(RLButLoc.x,RLButLoc.y);
	    	MousePoint = MAKEPOINT(lParam);
	    	HaveDownButton = TRUE;
			goto RButUp;
		}
		if (wParam == GF_SNAP_UNLOCK && hCurPolyPoints)
		{
			lpDPoint = (LPDPOINT)GlobalLock(hCurPolyPoints)+(nCurPolyPoints-1); 
			CurrentPoint = *lpDPoint; 
			GlobalUnlock (hCurPolyPoints);
		} 
		if (wParam == GF_SET_POC)
		{	
			if (!hCurPolyPoints/* || (NewPolyType == 1 && nCurPolyPoints > 1)*/) 
				break;
			POC = CurrentPoint;
			POCScreen = BasePtToWinPt(&CurrentPoint); 
			HavePOC = TRUE;
			HaveDownButton=FALSE;  
			//if (!NewPolyType)
				break;  
			lpDPoint = (LPDPOINT)GlobalLock(hCurPolyPoints)+nCurPolyPoints; 
			*lpDPoint = CurrentPoint;  
			GlobalUnlock (hCurPolyPoints);
			nCurPolyPoints++;
			break;			
		}
		if (hNewPolyPoints && hCurPolyPoints)
		{   
			if (!nTempPoints)
				HaveStart = FALSE;
	    	if (HaveStart && DoTrack)
				NotPolyline (CurView->hDC,Points,(short)nTempPoints,NULL,NULL);  
			nTempPoints = 2;
			lpDPoint = (LPDPOINT)GlobalLock(hCurPolyPoints)+nCurPolyPoints; 
			nCurPolyPoints+=NumNewPolyPoints;
			lpNewPoint = (HPDPOINT)GlobalLock (hNewPolyPoints); 
			while (NumNewPolyPoints--)
			{   
				if (HaveStart)
				{
					Points[1] = BasePtToWinPt (lpNewPoint);
					TempPolyline (CurView->hDC,Points,2,NULL,NULL);
		    		Points[0] = Points[1];
				}
				else
				{
					HaveStart = TRUE;
					Points[0] = BasePtToWinPt (lpNewPoint);
				}	  
				*lpDPoint++ = *lpNewPoint++;
			}
			GlobalUnlock (hCurPolyPoints);
        }    
		GSSiGlobUlFree (&hNewPolyPoints);
		NumNewPolyPoints = 0;
       	if (wParam == GF_AREA_FROM_HLT)  
       	{
	    	HaveDownButton = TRUE;
			goto RButUp;
       	}
   		break;	
   		
   	case GF_REDRAW:
    	FirstMoveSinceRedraw = TRUE;  
		HaveDownButton = FALSE;
    	if (nCurPolyPoints)
    	{   
    		Points[0]=Points[1]=DrawCurPoints (Spline,StartSpline,R2_NOT,NULL);
    		
/*			SetDisplayMode (CurView->hDC, GF_TEXTMODE);
			CurView->hRgn = CreateVPRgn (FALSE);
		  	SelectClipRgn (CurView->hDC,CurView->hRgn);
		  	DeleteObject(CurView->hRgn);
   			hPoints = GSSiGlobAlloc(GMEM_MOVEABLE,nCurPolyPoints*sizeof(POINT));
   			lpPoints = (HPPOINT)GlobalLock(hPoints);
   			lpPoint = lpPoints;
   			lpDPoint = (HPDPOINT)GlobalLock(hCurPolyPoints); 
   			for (i=0;i<nCurPolyPoints;i++,lpPoint++,lpDPoint++)
   			{
   				*lpPoint = BasePtToWinPt(lpDPoint);
   			}
   			GlobalUnlock (hCurPolyPoints);
	    	lpPoint--;
	    	if (nCurPolyPoints > 1)
	    	{
		    	TempPolyline (CurView->hDC,lpPoints,(short)nCurPolyPoints,NULL,NULL);  
		    	Points[1]=*lpPoint;
		    }
	    	Points[0]=*lpPoint;
           	GSSiGlobUlFree (&hPoints);*/  
   		}
   		if (!CursorIsLocked)
			SetCurs (2,FALSE);
   		break;
    
    case WM_RBUTTONDOWN: 
    	if (wParam & MK_LBUTTON)
    		goto RtnFalse;
    case WM_LBUTTONDOWN: 
    	HaveDownButton=TRUE;
    	MousePoint = MAKEPOINT(lParam);
    	break;
    	
    case WM_MBUTTONDOWN:   
    	break;
    	
    case WM_MBUTTONUP:
       	HaveCurrentLBUTTON=TRUE;
		CurrentLBUTDOWNLoc = MAKEPOINT (lParam);
		IgnoreLbutton = FALSE;   
		AddGraphicsFunction (hWnd, GF_SET_POC,0); 
/*		if (!CursorIsLocked)
		{
			IgnoreLbutton = FALSE;   
			IgnoreSelectVP = TRUE;
			PostMessage(hWnd, WM_LBUTTONDOWN,wParam, lParam); 
			PostMessage(hWnd, WM_LBUTTONUP,wParam, lParam);
		}*/
    	break;
/*    	if (HaveLastEP)
    	{ 
			CursorIsLocked = TRUE;
	    	CurrentPoint = LastEP; 
	    	HaveDownButton = TRUE;
			if (DoTrack)
			{
				NotPolyline (CurView->hDC,Points,nTempPoints,NULL,NULL);
				Points[1] = BasePtToWinPt (&CurrentPoint);
				TempPolyline (CurView->hDC,Points,2,NULL,NULL);
			}
	    	goto LButUp;
    	}   
    	break; */
    	
    case WM_LBUTTONUP:
LButUp: 
		if (!HaveDownButton)
			break;
/*    	if (HavePOC && NewPolyType == 1)
    		goto RButUp; */
		HaveDownButton=FALSE;  
    	if (HaveStart && !CursorIsLocked)
    	{
			if (DoTrack)
				NotPolyline (CurView->hDC,Points,(short)nTempPoints,NULL,NULL);
			if (!Spline && TempLineType != 1) 
				TempPolyline (CurView->hDC,Points,(short)nTempPoints,NULL,NULL);
		}
		SetDisplayMode (CurView->hDC, GF_TEXTMODE);
		CurView->hRgn = CreateVPRgn (FALSE,FALSE);
	  	SelectClipRgn (CurView->hDC,CurView->hRgn);
	  	DeleteObject(CurView->hRgn);
	    if (!CurView->hTranWinToBase) break;
   		if (!hCurPolyPoints)
   		{
   			hCurPolyPoints = GSSiGlobAlloc(GHND,MAX_DIGPOINTS*(long)sizeof(DPOINT));
   			nCurPolyPoints = 0;   
   			CurPolyVP = CurView;
   		}
	    if (CursorIsLocked)
	    { 
	    	BasePoint = CurrentPoint;  
	    	UnlockCursor ();
//			CursorIsLocked = FALSE; 
           	CreateDigCursor (CurView->hDC);
			SetCurs (2,FALSE);
	    }
	    else
        	BasePoint=GetButtonWorldCoord (MousePoint,wParam);  
        EnlargeScreen (0,0);
        CurrentPoint = BasePoint;
		lpDPoint = (LPDPOINT)GlobalLock(hCurPolyPoints)+nCurPolyPoints; 
        if (HaveStart && nCurPolyPoints)
        {   
   			double	AZ;
   			long	nNewPnts;
   			 
        	lpDPoint--;
        	if (ldistp (CurrentPoint,*lpDPoint) > P_TOL || HavePOC)
        	{   
		    	Points[0]=Points[1];
		    	Points[1] = MousePoint;
				if (Spline)
					nTempPoints = 0;
				else
					nTempPoints = 2;
        		ShowTempLineType (&BasePoint,lpDPoint,&TotDist);
				if (HavePOC)
				{   
					DPOINT	PC=*lpDPoint;
					double	BackAZ;
					
					lpDPoint++;
	                CurvePointsD(&PC,&POC,&CurrentPoint, &nCurPolyPoints, &lpDPoint,&BackAZ,UINT_MAX-4,CurveChordDist,1);
					HavePOC = FALSE; 
					lpDPoint--;
		    		Points[0]=BasePtToWinPt(lpDPoint);
		    		Points[1]=Points[0];   
					SetCurAZ (BackAZ);
				}
				else
				{
					nCurPolyPoints++; 
					AZ = getazd (lpDPoint,&BasePoint);
					SetCurAZ (AZ);
					lpDPoint++;
					*lpDPoint = BasePoint; 
				}   
			}
        }
        else
        {
    		HaveStart = TRUE;  
    		*lpDPoint = BasePoint;
    		Points[0]=BasePtToWinPt(lpDPoint);
    		Points[1]=Points[0];  
    		if (Spline)
    			nTempPoints = 0;
    		else
	    		nTempPoints = 2;
    		nCurPolyPoints = 1;  
	    }
		GlobalUnlock (hCurPolyPoints);
		if (Message == WM_MBUTTONUP) 
			PostMessage(hWnd, WM_MOUSEMOVE,wParam, lParam); 
		break; 
		
//    case WM_RBUTTONDOWN:
//    	if (wParam!=2) goto RtnFalse;
    	/* keeps from being passed to identify function */
//		break;
		
    case WM_RBUTTONUP:
RButUp: 
//    	MousePoint = MAKEPOINT(lParam);
		if (!HaveDownButton)
			break;
		HaveDownButton=FALSE;  
	    if (!CurView->hTranWinToBase) break;
	    if (CursorIsLocked)
	    { 
	    	BasePoint = CurrentPoint;  
	    	UnlockCursor ();
//			CursorIsLocked = FALSE; 
//			SetCurs (hDigCursor,FALSE);
	    }
	    else
        	BasePoint=GetButtonWorldCoord (MousePoint,wParam);  
        EnlargeScreen (0,0);
        LastEP = BasePoint;
        HaveLastEP = TRUE;
        if (HaveStart && hCurPolyPoints)
        {   
        	BOOL	st; 
        	DPOINT	FirstPoint;
        	
			if (DoTrack)
				NotPolyline (CurView->hDC,Points,(short)nTempPoints,NULL,NULL); 
			lpDPoint = (HPDPOINT)GlobalLock(hCurPolyPoints);
			FirstPoint = *lpDPoint;
			lpDPoint += nCurPolyPoints; 
        	lpDPoint--;
       		st = ShowTempLineType (&BasePoint,lpDPoint,&TotDist);    
       		lpDPoint++;
//			OldMode = SetROP2(CurView->hDC,R2_NOT); 
//			if (!st)
//				TempPolyline (CurView->hDC,Points,nTempPoints,NULL,NULL);
	    	Points[1]=MousePoint;
//	    	SetROP2(CurView->hDC,OldMode);
        	EndPointConnected=FALSE;
        	if (NewPolyType && nCurPolyPoints == 1 && HavePOC)
        	{
				nCurPolyPoints++;
				*lpDPoint++ = POC;
			}
			else if (HavePOC && !NewPolyType && !Spline)
			{   
				DPOINT	PC=*(lpDPoint-1);
				double	BackAZ;
					
                CurvePointsD(&PC,&POC,&FirstPoint, &nCurPolyPoints, &lpDPoint,&BackAZ,UINT_MAX-4,CurveChordDist,1);
				HavePOC = FALSE; 
				lpDPoint--;
				nCurPolyPoints--;
	    		Points[0]=BasePtToWinPt(lpDPoint);
	    		Points[1]=Points[0];   
				SetCurAZ (BackAZ);
			}
			nCurPolyPoints++;
			*lpDPoint = BasePoint; 
			GlobalUnlock (hCurPolyPoints);

			if (Spline)
			{   
				double	Thin = GetGlobalDVal2 ("[%SPLINETHIN]",0.1); 
				double	EndDist;
				
				lpDPoint = (HPDPOINT)GlobalLock(hCurPolyPoints);
				if (wParam == 10)
				{   
					lpDPoint[nCurPolyPoints++] = lpDPoint[1];
					EndDist = ldistp (lpDPoint[0],lpDPoint[1]);
				}
				else
					EndDist = 0;	  
		        SplinePointsD (StartSpline,nCurPolyPoints,lpDPoint, &StartSpline, lpDPoint+StartSpline,CurView->BaseUnitsPerPixel,Thin,EndDist,UINT_MAX/4);   
	        	GlobalUnlock (hCurPolyPoints); 
	        	nCurPolyPoints = StartSpline;
				lpDPoint = (HPDPOINT)GlobalLock(hCurPolyPoints)+nCurPolyPoints; 
				*lpDPoint = BasePoint;
				GlobalUnlock (hCurPolyPoints);
				nCurPolyPoints++;
				Spline = FALSE;
			}

            NumNewPolyPoints = nCurPolyPoints;
            hNewPolyPoints = hCurPolyPoints;
            hCurPolyPoints = 0;
           	HaveStart=FALSE;
            nCurPolyPoints = 0;
           	CreateDigCursor (CurView->hDC);
		    PostMessage(hWnd, GF_CLOSE,Function,0); 
        } 
        break; 
        
    case GF_CLOSE:
    	if (HaveStart && !CursorIsLocked && DoTrack)
			NotPolyline (CurView->hDC,Points,(short)nTempPoints,NULL,NULL); 
    	if (nCurPolyPoints > 1)
    	{   
    		DrawCurPoints (Spline,StartSpline,R2_NOT,Points);
	    	nTempPoints = 0;
   		}
//        RemoveGraphicsFunction (hWnd); 
//        NumNewPolyPoints = 0;
//        hNewPolyPoints = 0;
       	GSSiGlobFree(&hCurPolyPoints);
       	HaveStart=FALSE;
       	nCurPolyPoints=0; 
       	GSSiGlobUlFree (&hTempPoints);
       	SetCurs (InCursor,FALSE);
       	InCreatePoly = FALSE;  
       	goto RtnFalse;
    	break;
        
    case WM_MOUSEMOVE: 
    	if (!HaveStart || HaveDownButton || !hCurPolyPoints) break;
		if (DoTrack)
			NotPolyline (CurView->hDC,Points,(short)nTempPoints,NULL,NULL);
    	if (CursorIsLocked) 
    	{
    		AtPoint = CurrentPoint;
    		Points[1] = BasePtToWinPt (&CurrentPoint); 
    	}
    	else 
    	{
    		Points[1] = MAKEPOINT(lParam); 
    		AtPoint = WinPtToBasePt (Points[1]);
    	}
		lpDPoint = (HPDPOINT)GlobalLock(hCurPolyPoints)+nCurPolyPoints; 
    	lpDPoint--; 
    	LastPoint = TrackLineBegin = *lpDPoint; 
    	TrackLineEnd = AtPoint;
    	GlobalUnlock (hCurPolyPoints);
		if (TempLineType==1)
    		ShowCurrentDist (2,&LastPoint,&AtPoint,TotDist);
    	if (HavePOC)
    	{   
    		DPOINT	BP=PointToDPoint(Points[0]),POC=PointToDPoint(POCScreen),EP=PointToDPoint(Points[1]);
    		nTempPoints = 0;
	        CurvePoints(&BP,&POC,&EP, &nTempPoints,  &Points, 1,UINT_MAX/4); 
	        GlobalUnlock (hTempPoints);
         	Points = (HPPOINT)GlobalLock (hTempPoints);
    	} 
    	else if (Spline && (nCurPolyPoints - StartSpline))
    	{   
			lpDPoint = (HPDPOINT)GlobalLock(hCurPolyPoints); 
    		lpDPoint[nCurPolyPoints] = TrackLineEnd;
    		if (ldistp (lpDPoint[nCurPolyPoints-1],lpDPoint[nCurPolyPoints]))
    			nCurPolyPoints++;
        	GlobalUnlock (hCurPolyPoints);
    		nTempPoints = 0;
	        SplinePoints (StartSpline,nCurPolyPoints,hCurPolyPoints, &nTempPoints,  Points,CurView->BaseUnitsPerPixel*2,UINT_MAX/4);   
	        nCurPolyPoints--;
	        GlobalUnlock (hTempPoints);
         	Points = (HPPOINT)GlobalLock (hTempPoints);
    	}
    	else
    		nTempPoints = 2; 
    	
    	if (DoTrack)
			NotPolyline (CurView->hDC,Points,(short)nTempPoints,NULL,NULL);
		break;
		
    case WM_CHAR:
		key = wParam; 
		if (lParam & KF_UP)
			break;
		switch (key)
		{   
			case 2: //CNTL_B 
				if (nCurPolyPoints && hCurPolyPoints)
				{   
					lpDPoint = (LPDPOINT)GlobalLock(hCurPolyPoints); 
					LockCursor (lpDPoint);
					GlobalUnlock (hCurPolyPoints);
					HaveDownButton = TRUE; 
					IgnoreLbutton = FALSE;   
					IgnoreSelectVP = TRUE;
//		        	SendMessage(hWnd, WM_LBUTTONUP,0, 0L); 
					HaveDownButton = TRUE;
		        	PostMessage(hWnd, WM_RBUTTONUP,10, 0L); 
		        }
		        goto RtnTrue;
		        
   			case 19: //CNTL_S 
   				if (Spline)
   				{   
					if (DoTrack)
			    	{   
						NotPolyline (CurView->hDC,Points,(short)nTempPoints,NULL,NULL);
			    		nTempPoints = 0;
				        SplinePoints (StartSpline,nCurPolyPoints,hCurPolyPoints, &nTempPoints,  Points,CurView->BaseUnitsPerPixel*2,UINT_MAX/4);   
				        GlobalUnlock (hTempPoints);
			         	Points = (HPPOINT)GlobalLock (hTempPoints);
						NotPolyline (CurView->hDC,Points,(short)nTempPoints,NULL,NULL);
			    	} 
                    {
						HPDPOINT pPoints = (HPDPOINT)GlobalLock(hCurPolyPoints);
						HPDPOINT pEndPoint = pPoints + (nCurPolyPoints-1);
						DPOINT	 EndPoint = *pEndPoint;
						double	Thin = GetGlobalDVal2 ("[%SPLINETHIN]",GetGlobalDVal2("[%BLACKLINEWIDTH]",2)/10);
	
						lpDPoint = pPoints;  
				        SplinePointsD (StartSpline,nCurPolyPoints,lpDPoint, &StartSpline, lpDPoint+StartSpline,CurView->BaseUnitsPerPixel,Thin,0,UINT_MAX/4);   
			        	GlobalUnlock (hCurPolyPoints); 
			        	nCurPolyPoints = StartSpline;
						pPoints = (HPDPOINT)GlobalLock(hCurPolyPoints)+nCurPolyPoints++;
						*pPoints = EndPoint;
			        	GlobalUnlock (hCurPolyPoints); 
	   					Spline = FALSE; 
	   					nTempPoints = 2;
		    			Points[0]=Points[1]=BasePtToWinPt(&EndPoint); 
		    		}
   				}
   				else 
   				{
   					Spline = TRUE;
   					StartSpline = max(nCurPolyPoints,1);
   				}
   				break;
   				
   			case 20: //CNTL_T
   				if (HaveStart) 
					NotPolyline (CurView->hDC,Points,(short)nTempPoints,NULL,NULL);
   				if (DoTrack) 
   					DoTrack = FALSE;
   				else
   					DoTrack = TRUE;
		        goto RtnTrue;
   				
		    case '\b': //Backspace 
	DoBackspace:
		    	if (!nCurPolyPoints) break;
		    	if (!FirstMoveSinceRedraw)
		    	{
					OldMode = SetROP2(CurView->hDC,R2_NOT); 
					if (DoTrack) 
						TempPolyline (CurView->hDC,Points,(short)nTempPoints,NULL,NULL);   
			    	SetROP2(CurView->hDC,OldMode);   
				}
				lpDPoint = (LPDPOINT)GlobalLock(hCurPolyPoints)+(nCurPolyPoints-1); 
	    		Points[1]=BasePtToWinPt(lpDPoint);
		    	nCurPolyPoints--; 
		    	if (Spline && nCurPolyPoints < StartSpline)
		    		Spline = FALSE;
		    	if (!nCurPolyPoints)
		    		HaveStart=FALSE;  
		    	else if (Spline)
		    		nTempPoints = 0;
		    	else
		    	{
					lpDPoint--; 
					CurrentPoint = *lpDPoint;
		    		Points[0]=BasePtToWinPt(lpDPoint); 
			    	if (HavePOC)
			    	{  
				    	HavePOC = FALSE;    
				    	Points[1] = Points[0];
				    	nTempPoints = 2;
				    }
		    	}
		    	GlobalUnlock (hCurPolyPoints);
		    	FirstMoveSinceRedraw = FALSE; 
		    	if (nCurPolyPoints)
		    	{
	    			SendMessage(hWnd, WM_MOUSEMOVE, 0,
	    							  MAKELPARAM(Points[1].x,Points[1].y)); 
	    		} 
		    	
		    	goto RtnTrue;
		}
       	goto RtnFalse;

    default:
    	goto RtnFalse;
    }
RtnTrue:
	if (hTempPoints)
		GlobalUnlock (hTempPoints);

    return (TRUE);   
RtnFalse:
	if (hTempPoints)
		GlobalUnlock (hTempPoints);

    return (FALSE);   
    
}

BOOL AutoIncTAG (LPSTR Prefix, LPSTR UDI,double IncVal)
{   
	BOOL	rtn=FALSE;
	short	i, len, l, ndp; 
	LPSTR	pBeg, pEnd;
	double	CurVal; 
	
	if (!IncVal)
		return FALSE;
	LoadTAGDef ();   
	if (NumTAGDef)
	{ 
		LPTAGDEF    lpTAGDef;
		char	SaveEnd;
						            
		lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
		for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
		{
		    if (!_fstricmp (Prefix,lpTAGDef->Prefix))
		    {
		    	if (lpTAGDef->IncLen)
		    	{
		    		l=_fstrlen (UDI); 
		    		if (lpTAGDef->IncBeg)
		    		{
		    			pBeg = UDI + (lpTAGDef->IncBeg-1);
		    			pEnd = pBeg + lpTAGDef->IncLen; 
		    			len = lpTAGDef->IncLen;
		    		}
		    		else
		    		{   
		    			len = min (l,lpTAGDef->IncLen);
		    			pEnd = _fstrchr (UDI,0);
		    			pBeg = UDI + (l - len);
		    		}         
		    		SaveEnd = *pEnd;
		    		*pEnd = 0;      
		    		CurVal = atof (pBeg);
		    		if (lpTAGDef->IncNDP)
		    		;
		    		else
		    			IWRITEZ (IDNINT(CurVal+IncVal),pBeg,len);
		    		*pEnd = SaveEnd;  
		    		UDI[lpTAGDef->Len] = 0;
		    		rtn = TRUE;
		    	}
		    }
		}
		GlobalUnlock (hTAGDef);
	}
	if (!rtn)
	{
		//find last n numeric values 
		if (rread (UDI,&CurVal,&ndp))
		{
			l = _fstrlen (UDI);
			CurVal += IncVal;
			if (ndp > 1)
				ndp--;
			else
				ndp = 0;
			RWRITEZ (CurVal,ndp,l,UDI); 
			rtn = TRUE;
		}
	}
	return rtn; 
} 

short AutoIncType (LPSTR Prefix)
{   
	short	rtn=0;
	
	LoadTAGDef ();   
	if (NumTAGDef)
	{ 
		LPTAGDEF    lpTAGDef;
		short	i, len, l; 
		LPSTR	pBeg, pEnd;
		double	CurVal; 
		char	SaveEnd;
						            
		lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
		for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
		{
		    if (!_fstricmp (Prefix,lpTAGDef->Prefix))
		    {
		    	rtn = lpTAGDef->IncLen;
		    	break;
		    }
		}
		GlobalUnlock (hTAGDef);
	}
	return rtn; 
}   

long GetPointsFromList (LPSTR PointList,LPHANDLE phList)
{
	long		nPoints=0; 
	HPDPOINT	Points; 
	LPSTR		pSpace;

	*phList = 0;
	OneSpace (PointList);
	if (!*PointList)
		return 0;         
	*phList = GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX);
	Points = (HPDPOINT)GlobalLock (*phList);
	while (PointList)
	{
		if (*PointList == ' ')
			PointList++;
		if ((pSpace = _fstrchr (PointList,' ')))
			*pSpace++ = 0;
		else
		{
			GSSiGlobUlFree (phList);
			return 0;
		}
		Points[nPoints].x = atof (PointList);
		Points[nPoints++].y = atof (pSpace);
		PointList = _fstrchr (pSpace,' ');
	}
	GlobalUnlock (*phList);	
	return nPoints;
}

long GetIntsFromList (LPSTR PointList,LPHANDLE phList)
{
	long		nPoints=0; 
	HPSHORT		Vals;
	LPSTR		pSpace;

	*phList = 0;
	OneSpace (PointList);
	if (!*PointList)
		return 0;         
	*phList = GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX);
	Vals = (HPSHORT)GlobalLock (*phList);
	while (PointList)
	{
		if (*PointList == ' ')
			PointList++;
		Vals[nPoints++] = atof (PointList);
		PointList = _fstrchr (PointList,' ');
	}
	GlobalUnlock (*phList);	
	return nPoints;
}

BOOL AddAreaToMap (LPSTR EditName,LPSTR TAG,LPSTR SymName,LPSTR PointList,short Type)
{ 
	short	SymNum, NumSyms=0; 
	HANDLE	hSymDesc=0; 
	long	Refno;
	char	Prefix[10], UDI[66]; 
	HANDLE	hPoints;
	HPDPOINT	pPoints;
	USHORT	nPoints=0;  
	BOOL	rtn=FALSE;
	
	if (!_fstricmp (EditName,"EDIT"))
	{
   		if (!CurView->UpdateFile)
   			return FALSE;
   		else
   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]); 
	}  
    SymNum = GetOrCreateSym (SymName,&NumSyms,&hSymDesc,FALSE,0); 
    if (!SymNum)
    	return FALSE;
    nPoints = GetPointsFromList (PointList,&hPoints);
    if (!nPoints)
    	return FALSE;
    if (Type && nPoints > 1 || !Type && nPoints > 2)
    {   
    	LPSTR	pColon = _fstrchr (TAG,':');
    	
    	if (pColon)  
    	{
    		strncpy0 (Prefix,TAG,(short)((long)pColon-(long)TAG));
    		pColon++;
    		_fstrcpy (UDI,pColon);
    	}
    	else
    		*Prefix = 0;
		Refno = GetNewRefno (EditName,NULL,NULL,NULL,NULL);
		AddToSymList (SymNum,&NumSyms,&hSymDesc); 
		_fstrcpy (PltName,EditName);
		PltType = 2;
		OpenMap (CurView->hWnd,CurView->hDC);
		EditBounds = CurView->FileMNMX; 
		CloseMap (FALSE);
		rtn = AddPolyToMap (1,&nPoints, &hPoints,Type,Refno,NULL,-1,SymNum,NULL,Prefix,UDI,-1,-1,-1,0,0,0,0,TRUE);
		GSSiGlobFree (&hPoints);
	    CloseMap(TRUE);  
		AddSymToMap (NumSyms,hSymDesc,0,NULL); 
	    DestroySymList (&NumSyms,&hSymDesc);
	}
	GSSiGlobFree (&hPoints);  
	return rtn;
}

BOOL CreateNewPolyline (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{  
 char key;   
 short	rtn=1;
 static	BOOL	Inited=FALSE;    
 static short	NewSymbol; 
 static	HCURSOR	InCursor;
 
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
   	
   		if (Inited)
   		{
	 		MessageBox(GetFocus(), "Must complete current line before beginning a new one", NULL,MB_ICONEXCLAMATION|MB_OK);
			PostMessage(hWnd, GF_CLOSE,0, 0L); 
            break;
   		}
   		if (NewPolyType)
   		{  
	   		if (!SetLineSymbol (&NewSymbol,NULL))
	   		{
		 		MessageBox(GetFocus(), "Linear symbol not set", NULL,MB_ICONEXCLAMATION|MB_OK);
				PostMessage(hWnd, GF_CLOSE,0, 0L); 
	            break;
	   		} 
	   	}
	   	else 
   		{  
	   		if (!SetAreaSymbol (&NewSymbol,NULL))
	   		{
		 		MessageBox(GetFocus(), "Area symbol not set", NULL,MB_ICONEXCLAMATION|MB_OK);
				PostMessage(hWnd, GF_CLOSE,0, 0L); 
	            break;
	   		}  
	   	} 
	   	InCursor = CurView->hCursor;
		CreateDigCursor (CurView->hDC);
		SetCurs (2,FALSE);
   		Inited = TRUE; 
		if (NewPolyType) 
	   		SetPrompt (PRMT_POLYLINE_INIT,TRUE);
	   	else
	   		SetPrompt (PRMT_POLYGON_INIT,TRUE);

		AddGraphicsFunction (hWnd, GF_DIGITIZE_POLYLINE,0);
		if (CurView->CurrentFunction != GF_DIGITIZE_POLYLINE)
			break;  
		if (NewPolyType) 
	   		SetFunStackPrompt (PRMT_POLYLINE_NEXT);
	   	else
	   		SetFunStackPrompt (PRMT_POLYGON_NEXT); 
   		break;
   		
    case WM_MBUTTONDOWN:   
    	break;
    	
    case WM_MBUTTONUP:
    	if (HaveLastEP)
    	{ 
			CursorIsLocked = TRUE;
	    	CurrentPoint = LastEP;
	    	goto LButUp;
    	}   
    	break;
    	
    case WM_LBUTTONUP: 
LButUp:
   		if (!EditName[0]) break;
		AddGraphicsFunction (hWnd, GF_DIGITIZE_POLYLINE,0);
		if (CurView->CurrentFunction != GF_DIGITIZE_POLYLINE)
			break;  
		if (NewPolyType) 
	   		SetFunStackPrompt (PRMT_POLYLINE_NEXT);
	   	else
	   		SetFunStackPrompt (PRMT_POLYGON_NEXT); 
/*	   	if (hCurPolyPoints) Added to allow enlarge to be used on first point - causes problems with dig mult lines
	   		break;*/
		PostMessage(hWnd, WM_LBUTTONDOWN,wParam, lParam); 
		PostMessage(hWnd, WM_LBUTTONUP,wParam, lParam);
		if (Message == WM_MBUTTONUP) 
			PostMessage(hWnd, WM_MOUSEMOVE,wParam, lParam); 
		break;
		
    case GF_DISPLAYMESS: 
		if (NewPolyType) 
	   		SetPrompt (PRMT_POLYLINE_INIT,TRUE);
	   	else
	   		SetPrompt (PRMT_POLYGON_INIT,TRUE);
        break;
        
    case GF_COMPLETE:
		if (wParam == GF_LBUTTON)
		{
			lParam = MAKELONG(RLButLoc.x,RLButLoc.y);
			goto LButUp; 
		}
		if (wParam == GF_RBUTTON)
			break;
		if (NewPolyType) 
	   		SetPrompt (PRMT_POLYLINE_INIT,TRUE);
	   	else
	   	{   
	   		if (NumNewPolyPoints)
	   			NumNewPolyPoints--; //remove right button point
	   		SetPrompt (PRMT_POLYGON_INIT,TRUE);                     
	   	}
    	if (NumNewPolyPoints)
    	{   
    		LPDPOINT	lpDPoints;
    		long		NewRefno; 
    		int			st;  
    		char		Prefix[128],UDI[128];
    		LPSTR		pStuff=0; 
    		HANDLE		hStuff=0;  
    		BOOL		status, Redefine=FALSE;   
    		char		SymName[34]; 
    		COLORREF	NewLineColor, NewAreaColor;
    		float		NewLineWidth;
    		short		NewLineSymbol, NewAreaSymbol; 
    		HANDLE		hText=0, hTextTPL=0, hTime=0;
		 
    		if (!EditName[0]) break;
    		_fstrcpy (PltName,EditName);
    		PltType = 2;
//			GetTAGForSymbol ("PEN1",Prefix,UDI); 
			if (NewPolyType) 
			{   
				short	Type=NewPolyType; 
				DPOINT	FirstPoint;
				
				lpDPoints =(HPDPOINT)GlobalLock (hNewPolyPoints);
				FirstPoint = *lpDPoints;
				GlobalUnlock (hNewPolyPoints);
				if (HavePOC)
					Type = 3;   
				NewRefno = GetNewRefnoNoUpdate (EditName);
				if (!SetNewMacro (2))
					goto ExitLine;
				if (!SetLineSymbol (&NewLineSymbol,NULL))
					goto ExitLine;
				if (!SetLineWidth (&NewLineWidth,NULL))
					goto ExitLine;
				if (!SetLineColor (&NewLineColor,NULL))
					goto ExitLine;
				if (!SetNewText (2,&hText,&hTextTPL,&FirstPoint))
					goto ExitLine;
				if (!SetNewTime (2,&hTime))
					goto ExitLine;
				_fstrcpy (Prefix,"[%NEW_LINE_PREFIX]");
				_fstrcpy (UDI,"[%NEW_LINE_UDI]"); 
				ExpandText (Prefix);
				ExpandText (UDI); 
				NewRefno = GetNewRefno (EditName,Prefix,UDI,&NewLineSymbol,&Redefine);
				if (!SetNewStuff (2,&hStuff))
					goto ExitLine;
				if (hStuff)
					pStuff = GlobalLock (hStuff);
				if (GetGlobalBVal2 ("[%LINESONLY]",FALSE) )
				{   
					WORD	nPnts2=2, i;
					HANDLE	hPnts2=GSSiGlobAlloc (GMEM_MOVEABLE,2*sizeof(DPOINT));
					HPDPOINT	pPoint2=(HPDPOINT)GlobalLock (hPnts2), pNewPP=(HPDPOINT)GlobalLock (hNewPolyPoints);
					
					for (i=1;i<NumNewPolyPoints;i++)
					{   
						pPoint2[0] = pNewPP[i-1];
						pPoint2[1] = pNewPP[i];
						status=AddPolyToMap (1,&nPnts2, &hPnts2,Type,NewRefno,hTime,-1,NewLineSymbol,(LPSHORT)pStuff,Prefix,UDI,-1,NewLineColor,(short)NewLineWidth,hText,hTextTPL,0,0,DigHiPrecis);
						NewRefno = GetNewRefno (EditName,Prefix,UDI,&NewLineSymbol,&Redefine);
					} 
					GlobalUnlock (hNewPolyPoints);
					GSSiGlobUlFree (&hPnts2);
				}
				else
					status=AddPolyToMap (1,&NumNewPolyPoints, &hNewPolyPoints,Type,NewRefno,hTime,-1,NewLineSymbol,(LPSHORT)pStuff,Prefix,UDI,-1,NewLineColor,(short)NewLineWidth,hText,hTextTPL,0,0,DigHiPrecis);
				NewSymbol = NewLineSymbol;
				if (status && *Prefix)
				{   
					double	IncVal = GetGlobalDVal2 ("[%NEW_LINE_AUTOINC]",0);
					
					if (AutoIncTAG (Prefix,UDI,IncVal))
						SetGlobalValue ("%NEW_LINE_UDI",UDI);
				}
			}
			else  
			{   
				LPVIEWPORT SaveVP=CurView; 
				
				NewRefno = GetNewRefnoNoUpdate (EditName);
				if (!SetNewMacro (3))
					goto ExitLine;
				if (!SetAreaSymbol (&NewAreaSymbol,NULL))
					goto ExitLine;
				if (!SetAreaColor (&NewAreaColor,NULL))
					goto ExitLine;   
				if (!SetNewTime (3,&hTime))
					goto ExitLine;
				_fstrcpy (Prefix,"[%NEW_AREA_PREFIX]");
				_fstrcpy (UDI,"[%NEW_AREA_UDI]"); 
				ExpandText (Prefix);
				ExpandText (UDI);
				SetCurView (SaveVP);
				NewRefno = GetNewRefno (EditName,Prefix,UDI,&NewAreaSymbol,&Redefine);
				if (!SetNewStuff (3,&hStuff))
					goto ExitLine;
				if (hStuff)
					pStuff = GlobalLock (hStuff); 
				if (Redefine)
					DeleteRedefinedItem (EditName);
				status=AddPolyToMap (1,&NumNewPolyPoints, &hNewPolyPoints,NewPolyType,NewRefno,hTime,-1,NewAreaSymbol,(LPSHORT)pStuff,Prefix,UDI,NewAreaColor,-1,0,0,0,0,0,DigHiPrecis);
			                                                                                                                                                                                                                                               