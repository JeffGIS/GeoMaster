#include "graphint.h"              
#include "gps.h"

#define	MAXBLOCK	4096       
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D     

#include "gmextern.h"


static BOOL	DigEventMode=TRUE;
static UINT	RxQueSize;
static HANDLE	hDigQue=0;
static UINT	DigQueLen=0;
static char	EndOfRecChar=ASCII_CR;
static short	LastButton;
static short	NumDigButtons;  

 
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

	Color = GetListNum ("[%DL]colordef.txt", Name);  
	return Color;
}
	
BOOL GPSTracking (short Opt)//-1 returns current status,0=off,1=on,2=toggle,3=?
{   
	BOOL	rtn=TRUE;  
	char	VehID[64];
	int		iview;
	
	if (Opt < 0)
		return TrackingStatus;
	if (Opt == 3)
	{
		UpdateAllVehicles(TRUE);
		RedisplayWindow ();
		TrackingStatus = 1; 
		return TRUE;
	}
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
			if (GetGlobalBVal2 ("[%DIGISGPS]",TRUE))
		   		SetGlobalValue ("%AVLFOLLOW","GPS"); 
			GetGlobalCVal ("[%AVLFOLLOW]",VehID,0); 

   			if (!_fstricmp (VehID,"GPS") && !OpenDigConnection(hWndMain))
   				rtn = FALSE;
   			else
   				TrackingStatus = 1; 
			SetWindowText (hWndMain,"GPS Tracking Enabled");  
			//UpdateAllVehicles (TRUE);  
			SetViewport(*pCommandViewport);
			for (iview=0;iview<nAVLVP;iview++)
			{
				if (CurView == AVLViewports[iview])
					goto HaveVP;
			}
			AVLViewports[nAVLVP++] = CurView;
			DisplayAllVehicles (TRUE,TRUE); 
HaveVP:
			break;
   	} 
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

DPOINT GetButtonWorldCoord (POINT MousePoint,WPARAM wParam)
{
	if (wParam & MK_DIGITIZER_BUTTON)
		return CurrentPoint;
	else
		return (ScreenPtToBasePt(MousePoint));
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
	LockedCursorVP = CurView->ID; 
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

BOOL CloseDigConnection( void)
{  
	short	i;
	
	if (GPSInputWnd)
   {	
		PostMessage( GPSInputWnd, WM_CLOSE, 0, 0L ) ; 
		GPSInputWnd = 0;
		return TRUE;
   }
   if (DigitizerStream == (HANDLE)-1)
   		return FALSE; 
   if (GPSInput)
		KillTimer (hWndMain,GPSTIMER);
   //EnableCommNotification( DigitizerStream, NULL, -1, -1 ) ;

   // kill the focus


   EscapeCommFunction( DigitizerStream, CLRDTR ) ;

   // close comm connection

   CloseComm( DigitizerStream ) ;
   DigitizerStream = (HANDLE)-1; 
   GSSiGlobFree (&hDigQue);
   for (i=1;i<NumDigButtons+1;i++)
		GSSiGlobUlFree (&hDigButtonCmd[i]); 
   GSSiTrace ("Dig Closed",0);
   return ( TRUE ) ;

} 
 

void StopExistingGPSInput (void)
{   
	HWND	hWndGPS;
	
	if ((hWndGPS = FindWindow ("GPSInputWndClass",NULL))) 
	{
		PostMessage(hWndGPS, WM_CLOSE, 0, 0L ) ; 
		Wait (1000);
	}
	return;
}

BOOL OpenDigConnection( HWND hWnd )
{
   BOOL		rtn=FALSE;
   char       DigPort[ 10 ], szTemp[ 10 ], DigName[32], DigInitString[256],Baud[32], cmd[256] ;
   BOOL       fRetVal ;
   char		IniName[128]="[%DL]geomastr.ini";
   int		ii;
   LPSTR	lpComma, lpStart;  
   BOOL		UseExternalGPSInput=GetGlobalBVal2("[%USEGPSINPUTEXE]",TRUE);
    
    GPSClose (&GPSStream);
	CloseDigConnection();  
	LastButton = 0;
   	ExpandText (IniName);
    if (GetGlobalBVal2 ("[%DIGISGPS]",TRUE))
    {   
    	GPSInput = TRUE;
		GetPrivateProfileString ("GPS","Port","COM1",DigPort,32,IniName);   
		GetPrivateProfileString ("GPS","Baud","9600",Baud,32,IniName);  
		GetPrivateProfileString ("GPS","DisplayNMEA","Y",DigInitString,2,IniName); 
		if (*DigInitString == 'Y')
			DisplayNMEA = TRUE;
		else
			DisplayNMEA = FALSE;
		_fstrcpy (DigName,"GPS"); 
		if (UseExternalGPSInput)
		{ 
			char	GPSReplayFile[MAX_PATH];

			StopExistingGPSInput ();
			GetGlobalCVal ("[%GPSReplayFile]",GPSReplayFile,0);
			sprintf (cmd,"[%DL]GPSInput.exe %ld %s %s %i %s",(long)hWndMain,&DigPort[3],Baud,DisplayNMEA,GPSReplayFile); 
			ExpandText (cmd);
			WinExec (cmd,SW_HIDE);
			return TRUE;
		}
	}
    else
    {   
    	GPSInput = FALSE;
		GetPrivateProfileString ("CurrentDigitizer","Name","",DigName,sizeof(DigName),IniName);   
		sprintf (DigInitString,"DigName %s",DigName);
		GSSiTrace (DigInitString,0);
		GetPrivateProfileString ("CurrentDigitizer","Port","",DigPort,sizeof(DigPort),IniName);  
		sprintf (DigInitString,"Port %s",DigPort);
		GSSiTrace (DigInitString,0);
	}
/*	RxQueSize=GetPrivateProfileInt ("CurrentDigitizer","ReceiveQueSize",4096,IniName);  
    DigitizerStream = OpenComm( DigPort, RxQueSize, 256);  
	sprintf (DigInitString,"DigStream %i",DigitizerStream);
	GSSiTrace (DigInitString,0);
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
	ClearCommBreak (DigitizerStream);
	GetCommError (DigitizerStream,0);
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
		SetTimer (hWndMain,GPSTIMER,5000,0);
	  GetPrivateProfileString (DigName,"InitString","",DigInitString,sizeof(DigInitString),IniName); 
	  ExpandText (DigInitString);
	  GSSiTrace (DigInitString,0); 
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
	      	MessageBox (GetFocus(),"Unable to send digitizer initialization string",0,MB_ICONEXCLAMATION);
	      	rtn=FALSE; 
	      }
      	  lpStart = lpComma; 
	      Sleep (500);
	  } 
	  hDigQue=GSSiGlobAlloc ( 596,GMEM_MOVEABLE,UINT_MAX);
	  DigQueLen=0; 
	  *IncompleteDigInput=0;
   }
   else
   {  
   	  MessageBox (hWnd,"Error setting digitizer parameters","Unable to open digitizer",MB_ICONEXCLAMATION);
      CloseComm(DigitizerStream) ;
      DigitizerStream=-1;
   }
*/
   return ( rtn ) ;

}

BOOL OpenDigControlDialog (HWND hWnd)
{
    FARPROC	lpfnDIGCONTROLMsgProc; 

	if (DigitizerStream < 0)
	{
		MessageBox (GetFocus(),"The digitizer is not open",0,MB_ICONEXCLAMATION);
		return FALSE;
	}
    
    if (hWndDigControl)
    	return TRUE;
	lpfnDIGCONTROLMsgProc = MakeProcInstance((FARPROC)DIGCONTROLMsgProc, hInst);
	CreateDialog(hInst, (LPSTR)"DIGCONTROL", hWnd, lpfnDIGCONTROLMsgProc);
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
		GSSiMessageBox (str,"Unsupported baud rate",MB_ICONEXCLAMATION,0);
    return BaudRate;
}



DPOINT	UpperLeftCorner (short item)
{
   	LPTHEME		pTheme;    
   	DPOINT		RtnPoint;
   	double		midx;
   	short		i, nleft, nright;

    RtnPoint.x = PickList[item].Rect.xmn;   
    RtnPoint.y = PickList[item].Rect.ymx;   
	SetConfig (PickList[item].ConfigID);
    SetViewport (PickList[item].ViewID);
	pTheme = AddTheme (GF_SAVEPOLY_THEME);
	CurView->PassID = 4;
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
    GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn (FALSE,FALSE);
  	SelectClipRgn (CurView->hDC,CurView->hRgn);
  	GSSiDeleteObject(&CurView->hRgn);
	nDisplayPoints = nCurPolyPoints;  
	if (Spline)
		nDisplayPoints = StartSpline;
	hPoints = GSSiGlobAlloc ( 602,GMEM_MOVEABLE,(long)(nDisplayPoints+1)*sizeof(POINT));
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
		TempPolyline (CurView->hDC,lpPoints,(short)(nDisplayPoints),0,0);  
	SetROP2(CurView->hDC,OldMode);
	GSSiGlobUlFree (&hPoints); 
	return EndPoint;
}  



BOOL AutoIncTAG (LPSTR Prefix, LPSTR UDI,double IncVal)
{   
	BOOL	rtn=FALSE;
	short	i, len, l;
	int		ndp; 
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

long GetPointsFromList (LPSTR PointList,LPHANDLE phList)
{
	long		nPoints=0,i; 
	HPDPOINT	Points, Points2; 
	LPSTR		pSpace;  
	HANDLE		hList;

	*phList = 0;
	OneSpace (PointList);
	if (!*PointList)
		return 0;
	if (!_fstrchr (PointList,' '))
	{
		if ((hList = GetPointListPoints (PointList,&nPoints)))
		{
			*phList = hList;
			return nPoints;
		}
		if (!IsInteger (PointList))
			return 0;
		hList = (HANDLE)atol (PointList);
		if (!hList)
			return 0;
		nPoints = GlobalSize (hList) / sizeof(DPOINT); 
		Points2 = (HPDPOINT)GlobalLock (hList);
		*phList = GSSiGlobAlloc ( 607,GMEM_MOVEABLE,nPoints * sizeof(DPOINT));
		Points = (HPDPOINT)GlobalLock (*phList);
		for (i=0;i<nPoints;i++)  
		{
			if (Points2[i].x < DBL_MAX)
				Points[i] = Points2[i]; 
			else
			{
				nPoints = i;
				break;
			}
		}
		GlobalUnlock (hList);
	}
	else
	{
		*phList = GSSiGlobAlloc ( 607,GMEM_MOVEABLE,USHRT_MAX);
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
	}
	GlobalUnlock (*phList);	
	return nPoints;
}

long GetIntsFromList (LPSTR PointList,LPHANDLE phList)
{
	long		nPoints=0; 
	HPLONG		Vals;
	LPSTR		pSpace;

	*phList = 0;
	OneSpace (PointList);
	if (!*PointList)
		return 0;         
	*phList = GSSiGlobAlloc ( 608,GMEM_MOVEABLE,USHRT_MAX);
	Vals = (HPLONG)GlobalLock (*phList);
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

long ConvertRectToArea (LPHANDLE phPoints)
{
	HANDLE		hPoints2 = GSSiGlobAlloc ( 609,GMEM_MOVEABLE,sizeof(DPOINT)*4);
	HPDPOINT	Point1 = (HPDPOINT)GlobalLock (*phPoints); 
	HPDPOINT	Point2 = (HPDPOINT)GlobalLock (hPoints2);
    	
	Point2[0] = Point2[1] = Point1[0];
	Point2[2] = Point2[3] = Point1[1];
	Point2[1].y = Point2[2].y;
	Point2[3].y = Point2[0].y;
	GlobalUnlock (hPoints2);
	GSSiGlobUlFree (phPoints);
	*phPoints = hPoints2;
	return 4;
}


BOOL AddAreaToMap (LPSTR EditName,LPSTR TAG,LPSTR SymNameIn,LPSTR PointList,short Type,double size,double rot,LPSTR Text,double TextSize,long TextColor,BOOL OpaqueText,BOOL Shadow,LPSHORT Stuff,HANDLE hTime)
{ 
	short	SymNum;
	static	short	NumSyms=0; 
	static	HANDLE	hSymDesc=0; 
	long	Refno;
	char	Prefix[10], UDI[66]; 
	HANDLE	hPoints=0;
	HPDPOINT	pPoints;
	int		nPoints=0;  
	BOOL	rtn=FALSE;
	short	PickFile;
	static	BOOL	DoClose=TRUE;
	long	fillColor=-1, penColor=-1, iColor;
	char	SymName[66],CSize[32],CRot[32],CColor[32];
	
	Processing = TRUE;
	if (Type == 4)
	{
		CloseMap(TRUE);  
		AddSymToMap (NumSyms,hSymDesc,0,0); 
		DestroySymList (&NumSyms,&hSymDesc);
		DoClose = TRUE;
		Processing = FALSE;
		return TRUE;
	}
	if (Type == 3 || DoClose)
	{
		CloseMap (FALSE);
		DestroySymList (&NumSyms,&hSymDesc);
		if (!_fstricmp (EditName,"EDIT"))
		{
   			if (!CurView->UpdateFile)
			{
				Processing = FALSE;
   				return FALSE;
			}
   			else
   				_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]); 
		}  
		_fstrcpy (PltName,EditName);
		PltType = 2;
	    ForceRefIndex = ForceTAGIndex = TRUE; 
		if (!OpenMap (CurView->hWnd,0))
			goto Exit;
	    ForceRefIndex = ForceTAGIndex = FALSE; 
		EditBounds = CurView->FileMNMX; 
		if (Type == 3)
		{
			rtn = TRUE;
			DoClose = FALSE;
			Processing = FALSE;
			return rtn;
		}
		if (Type == 6)
			Type = 3;
	}
    nPoints = GetPointsFromList (PointList,&hPoints);
    if (!nPoints)
    	goto Exit;
	DecodePointSym (SymNameIn, SymName,CSize,CRot,CColor);
	iColor = atoi (CColor);
	if (iColor > 0)
		fillColor = iColor;
    SymNum = GetOrCreateSym (hWndMain,SymName,&NumSyms,&hSymDesc,FALSE,0); 
    if (!SymNum)
    	goto Exit;
	AddToSymList (SymNum,&NumSyms,&hSymDesc); 
    if (!Type && nPoints == 2)
    	nPoints = ConvertRectToArea (&hPoints);
	if (Type == 5)
		Type = 0;
    if (Type == 1 && nPoints > 1 ||
		Type == 2 && nPoints == 1 ||
		Type == 3 && (nPoints == 2 || nPoints == 3) ||
		!Type && nPoints > 1)
    {   
    	LPSTR	pColon = _fstrchr (TAG,':');
    	LPSTR	pVB	= strchr (TAG,'|');

		if (pVB)
			*pVB++ = 0;
    	if (pColon)  
    	{
    		strncpy0 (Prefix,TAG,(short)((long)pColon-(long)TAG));
    		pColon++;
    		_fstrcpy (UDI,pColon);
    	}
    	else
    		*Prefix = 0;
		//PickFile = GetPickFile (-1);
		PickFile = -1;
		if (pVB)
		{
			if (*pVB)
				Refno = atol (pVB);
			else
				Refno = GetNewRefno (EditName,0,0,0,0);
		}
		else if (*Prefix && PickByRefno (0,Prefix,UDI,PickFile))
			Refno = PickList[0].Refno;
		else
			Refno = GetNewRefno (EditName,0,0,0,0);
		if (Type < 2 || Type == 3)
			rtn = AddPolyToMap (1,&nPoints, &hPoints,Type,Refno,hTime,-1,SymNum,Stuff,Prefix,UDI,-1,-1,-1,0,0,0,0,TRUE,0);
		else 
		{
			LPDPOINT	pPoint=(LPDPOINT)GlobalLock (hPoints);     
			HANDLE		hText=0;
			LPGRTEXT	lpGRText; 
			LPUMTEXTTPL	lpTextTPL;
			DPOINT		DPoint; 
			char		str[256];
			int			vj = 2;
			int			hj = 1;
			BOOL		flip=TRUE;
			LPSTR		pBar;
				
			if (*Text)
			{
				if ((pBar=strrchr (Text,'|')))
				{
					char	tmp[2]=" ";

					tmp[0] = *(pBar+1);
					vj = atoi (tmp);
					tmp[0] = *(pBar+2);
					hj = atoi (tmp);
					tmp[0] = *(pBar+3);
					flip = atob (tmp);
					*pBar = 0;
				}
				if (TextColor < 0)
				{   
					char	Desc[80];
					
					if (GetDictSymDescription (GetDictSymbolNumber (SymName),Desc))
					{
						ExpandText (Desc);
						{
							pBar=_fstrchr (Desc,'|');
							
							if (pBar)
							{   
								*pBar++ = 0; 
								OpaqueText = atob (pBar);
								TextColor = atol(Desc);
							}
						} 
					} 
				}
				hText = GSSiGlobAlloc ( 343,GHND,sizeof(GRTEXT));
			   	lpGRText = (LPGRTEXT)GlobalLock (hText); 
				lpGRText->version = 1;    
				lpGRText->length = sizeof(GRTEXT);   
			   	lpGRText->vJust = vj;
			   	lpGRText->hJust = hj;
			    lpGRText->ltext = _fstrlen (Text); 
			    _fstrcpy (lpGRText->Text,Text);
			    lpGRText->weight = 2;  
			    lpGRText->Shadow = Shadow;
			    lpGRText->italic = FALSE; 
			    lpGRText->FontNum = 0;
			    lpGRText->Opaque=OpaqueText; 
			    sprintf (lpGRText->cHeight,"%f",TextSize);
			    sprintf (lpGRText->cColor,"%ld",TextColor);
			    lpGRText->FlipForEasyReading = flip; 
		    	GlobalUnlock (hText);
			}  
			rtn = AddPointToMap (*pPoint,Refno,hTime,SymNum,size, rot,Stuff,hText,0,
								 Prefix,UDI,fillColor,penColor,-1,FALSE,HiPrecis,0,0); 
			GlobalUnlock (hPoints); 
			GSSiGlobFree (&hText);
		}
       	GSSiGlobFree (&hPoints);
	}
Exit:
	if (DoClose)
	{
		CloseMap(TRUE);  
		AddSymToMap (NumSyms,hSymDesc,0,0); 
		DestroySymList (&NumSyms,&hSymDesc);
	}
    ForceRefIndex = ForceTAGIndex = FALSE; 
	GSSiGlobFree (&hPoints);  
	Processing = FALSE;
	return rtn;
}

BOOL FillRectWithGrid (LPSTR EditName,LPSTR SymName,LPMNMXCORD pRect,double GridSpace)
{ 
	short	SymNum, NumSyms=0,i,j; 
	HANDLE	hSymDesc=0; 
	long	Refno=0;
	HANDLE	hPoints;
	HPDPOINT	pPoints;
	int		nPointsx, nPointsy;  
	BOOL	rtn=FALSE;
	short	PickFile; 
	double	RectWidth = pRect->xmx - pRect->xmn;
	double	RectHeight = pRect->ymx - pRect->ymn;   
	double	GridSpaceX=GridSpace, GridSpaceY=GridSpace;
	
	if (GridSpace < 0)
	{
		GridSpaceX = -RectWidth / GridSpace;
		GridSpaceY = -RectHeight / GridSpace;
	}
	if (!_fstricmp (EditName,"EDIT"))
	{
   		if (!CurView->UpdateFile)
   			return FALSE;
   		else
   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]); 
	}  
    SymNum = GetOrCreateSym (hWndMain,SymName,&NumSyms,&hSymDesc,FALSE,0); 
    if (!SymNum)
    	return FALSE;
	AddToSymList (SymNum,&NumSyms,&hSymDesc); 
	_fstrcpy (PltName,EditName);
	PltType = 2;
	OpenMap (CurView->hWnd,CurView->hDC);
	EditBounds = CurView->FileMNMX; 
	CloseMap (FALSE);
    nPointsx = RectWidth/GridSpaceX+2;
    nPointsy = RectHeight/GridSpaceY+2;
    for (i = 0;i<nPointsy;i++)
    {
    	hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,nPointsx*sizeof(DPOINT));
    	pPoints = (HPDPOINT)GlobalLock (hPoints);
    	for (j=0;j<nPointsx;j++) 
    	{
    		pPoints[j].x = pRect->xmn + j*GridSpaceX;   
    		pPoints[j].y = pRect->ymn + i*GridSpaceY;
    	} 
		Refno = GetNewRefno (EditName,0,0,0,0);
		rtn = AddPolyToMap (1,&nPointsx, &hPoints,1,Refno,0,-1,SymNum,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
		GSSiGlobUlFree (&hPoints);
    }
    for (i = 0;i<nPointsx;i++)
    {
    	hPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,nPointsy*sizeof(DPOINT));
    	pPoints = (HPDPOINT)GlobalLock (hPoints);
    	for (j=0;j<nPointsy;j++) 
    	{
    		pPoints[j].x = pRect->xmn + i*GridSpaceX;   
    		pPoints[j].y = pRect->ymn + j*GridSpaceY;
    	} 
		Refno = GetNewRefno (EditName,0,0,0,0);
		rtn = AddPolyToMap (1,&nPointsy, &hPoints,1,Refno,0,-1,SymNum,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
		GSSiGlobUlFree (&hPoints);
    }
    CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,0); 
    DestroySymList (&NumSyms,&hSymDesc);
	return rtn;
}

 
DPOINT	GetAreaCenter (HPDPOINT Points,long nPnts)
{
	DPOINT Center={0,0}, FirstPoint=*Points, LastPoint=*Points++;
	double	TotDist=0, Dist;
	
	nPnts--;
    while (nPnts--)
    {
    	Dist = ldistp (LastPoint,*Points); 
    	LastPoint = *Points; 
    	TotDist+= Dist;
        Center.x += Points->x * Dist;
        Center.y += Points->y * Dist;
        Points++;
    }
    Center.x /= TotDist;
    Center.y /= TotDist;
    return Center;
} 

BOOL DisplayDistanceLine (BOOL DisplayDist)  
{
	if (!CurView->hDistanceLine)
		return FALSE;    
	SaveDC (CurView->hDC);  
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	{
		LOGBRUSH    NDB={BS_HATCHED,RGB(255,255,0),HS_DIAGCROSS}; 		
	    HBRUSH		hBrush =  CreateBrushIndirect(&NDB);
		double		size,Perim,SQMiles, Acres,SQFeet;
		HBRUSH		hOldBrush = SelectObject (CurView->hDC,hBrush);  
		DPOINT		Center;  
		POINT		CenterWindow;
		short		x, y;
		char		str[256];
		DWORD		TextExt;
		short		w,h; 
		short		OldMode = SetBkMode(CurView->hDC, TRANSPARENT); 
		COLORREF	OldColor = SetTextColor (CurView->hDC,0);  
		LPLONG		pNumDistPoints;
		HPDPOINT	pDistPoints;  
		long		nPoints;
		BOOL		ShowArea=TRUE;
		
		pNumDistPoints = (LPLONG)GlobalLock (CurView->hDistanceLine);
		pDistPoints = (HPDPOINT)(pNumDistPoints+1);     
		nPoints = *pNumDistPoints;
		if (nPoints < 0)
		{
			ShowArea = FALSE;
			nPoints = -nPoints;
		}
		GSSiDeleteObject(&CurView->hRgn);
		CurView->hRgn = CreateVPRgn (FALSE,FALSE);
		SelectClipRgn (CurView->hDC,CurView->hRgn);
		GSSiDeleteObject(&CurView->hRgn);  
		
		if (DisplayDist)
		{   
			UINT	i;
			double	TotDist=0; 
			int		SaveTLT = TempLineType;
			
			TempLineType = 1;
			for (i=1;i<nPoints;i++)
		    	ShowTempLineType (&pDistPoints[i-1],&pDistPoints[i],&TotDist);  
		    TempLineType = SaveTLT;
		}
		if (ShowArea)
		{   
			LOGFONT	LogFont;  
			HFONT	hFont,OldFont;  
			SIZE	txSize;
			
			_fmemset (&LogFont,0,sizeof(LOGFONT));
			_fstrcpy (LogFont.lfFaceName,"Arial"); 
			LogFont.lfHeight = -14 *DeviceToScreenFactor; 
			hFont = CreateFontIndirect((LPLOGFONT)&LogFont); 
			OldFont = SelectObject (CurView->hDC,hFont);
			GWPolygonD (CurView->hDC,pDistPoints,nPoints,1,0,0,FALSE,TRUE,0);
			size = ComputeAreaAreaD (pDistPoints,nPoints,&Perim);
			SQFeet = ConvertArea (size,1);  
			SQMiles = ConvertArea (size,4);  
			Acres = ConvertArea (size,6);
			Center = GetAreaCenter (pDistPoints,*pNumDistPoints);  
			CenterWindow = BasePtToWinPt (&Center);
			SetBkMode (CurView->hDC,OPAQUE);
			sprintf (str,"%.0f SqFt",SQFeet);  
			GetTextExtentPoint32 (CurView->hDC,str,_fstrlen(str),&txSize);
			w = txSize.cx; 
			h = txSize.cy; 
			x = CenterWindow.x - w/2;
			y = CenterWindow.y - h;
			TextOut (CurView->hDC,x,y,str,_fstrlen(str));
			sprintf (str,"%.3f Acres",Acres);  
			GetTextExtentPoint32 (CurView->hDC,str,_fstrlen(str),&txSize);
			w = txSize.cx; 
			h = txSize.cy; 
			x = CenterWindow.x - w/2;
			y = CenterWindow.y;
			TextOut (CurView->hDC,x,y,str,_fstrlen(str));
			sprintf (str," %.3f Square Miles",SQMiles); 
			GetTextExtentPoint32 (CurView->hDC,str,_fstrlen(str),&txSize);
			w = txSize.cx; 
			h = txSize.cy; 
			x = CenterWindow.x - w/2;
			y = CenterWindow.y + h;
			TextOut (CurView->hDC,x,y,str,_fstrlen(str));  
			SetBkMode(CurView->hDC, OldMode); 
			SetTextColor (CurView->hDC,OldColor);
			SelectObject (CurView->hDC,hOldBrush);   
			SelectObject (CurView->hDC,OldFont);
			GSSiDeleteObject (&hFont);
			GSSiDeleteObject (&hBrush); 
		} 
		GlobalUnlock (CurView->hDistanceLine);   
	}
	RestoreDC (CurView->hDC,-1);    
	return TRUE;
}

void NotPolyline (HDC hDC, LPPOINT Points, short nPnts, LPSTR TopText, LPSTR BottomText)
{   
	short	OldMode;
	HRGN	hRgn;
	
	SaveDC (hDC);
	SetDisplayMode (hDC, GF_TEXTMODE);
	hRgn = CreateVPRgn (FALSE,FALSE);
  	SelectClipRgn (hDC,hRgn);
  	GSSiDeleteObject(&hRgn);
	if (!FirstMoveSinceRedraw)
	{
		OldMode = SetROP2(hDC,R2_NOT); 
		TempPolyline (hDC,Points,nPnts,0,0); 
		SetROP2(hDC,OldMode); 
	}
	FirstMoveSinceRedraw=FALSE;
	RestoreDC (hDC,-1);
	return;
}

void NotPolylineScreen (HDC hDC, LPPOINT Points, short nPnts, LPSTR TopText, LPSTR BottomText)
{   
	short	OldMode;
	
	SaveDC (hDC);
	SetDisplayMode (hDC, GF_SCREENMODE);
  	SelectClipRgn (hDC,0);
	if (!FirstMoveSinceRedraw)
	{
		OldMode = SetROP2(hDC,R2_NOT); 
		TempPolyline (hDC,Points,nPnts,0,0); 
		SetROP2(hDC,OldMode); 
	}
	FirstMoveSinceRedraw=FALSE;
	RestoreDC (hDC,-1);
	return;
}

BOOL ShowTempLineType (LPDPOINT pBasePoint,LPDPOINT lpDPoint,LPDOUBLE pTotDist)
{   
	char	txt[128]; 
	COLORREF	Color;
	HPEN	hDistPen, hOldPen; 
	short	width, LineWidth, TextWidth, nc; 
	POINT	Points[3];
	DWORD	TextExt;  
	double	AZ;
	    
	if (TempLineType==1)
	{   
		DPOINT	MidPoint;
		POINT	point;
		extern	short OutDistUnits; 
		short	DistUnits = OutDistUnits,i;
		double	Dist,AZ, Size=GetGlobalDVal2 ("[%DISTLINETEXTSIZE]",0.15);
		
		Color = ConvertColor(GetGlobalLVal2 ("[%DISTLINECOLOR]",RGB(255,0,0)),0);						
		width = GetGlobalLVal2 ("[%DISTLINEWIDTH]",2);
		if (PRJ_UNITS[1] == 4)	
			Dist = ArcDistance(*lpDPoint,*pBasePoint);
		else
			Dist = ldistp (*lpDPoint,*pBasePoint);
		AZ = getazd (lpDPoint,pBasePoint);
		MidPoint = MidPointD(*lpDPoint,*pBasePoint);
		for (i=0;i<CurView->NumThemes;i++)
		{
			if (CurView->pThemes[i]->ID  == GF_DISTANCE_THEME) 
			{   
//				if (CurView->pThemes[i]->ClassType != 1)
					DistUnits = CurView->pThemes[i]->ValConv;
				break;
			}
		}
		if (DistUnits <= 0)
		{   
			if (ConvertDist (*pTotDist+Dist,4) < GetGlobalDVal2 ("[%SWITCHTOMILESAT]",1.0))
				DistUnits = 1;
			else
				DistUnits = 4;
		}
		if (*pTotDist > 0) 
		{
			*pTotDist += Dist;
			sprintf (txt,"%.1f (%.1f)", ConvertDist(Dist,DistUnits),
										ConvertDist(*pTotDist,DistUnits)); 
		}
		else
		{
			sprintf (txt,"%.1f",ConvertDist(Dist,DistUnits)); 
			*pTotDist = Dist;
		}
		SaveDC (CurView->hDC);  
		SetTextColor (CurView->hDC,Color);
		SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    	SetBkMode (CurView->hDC,OPAQUE);
		hDistPen = CreatePen (PS_SOLID,(int)IDNINT(DeviceToScreenFactor*width),Color);
		hOldPen = SelectObject (CurView->hDC,hDistPen); 
		Points[0]=BasePtToWinPt (lpDPoint);
		Points[1]=BasePtToWinPt (pBasePoint);
		point = BasePtToWinPt (&MidPoint);
		TextExt = DispText (CurView->hDC,TRUE,point.x,point.x, -point.y,0, 2,2,
  					Size,1,1,100, FALSE,AZ,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0); 
  		nc = _fstrlen (txt);
  		TextWidth = LOWORD (TextExt);
  		if (nc)
  			TextWidth += TextWidth/nc;
  		LineWidth = idist (Points[0],Points[1]);
  		if (TextWidth < LineWidth)
  		{ 
			DispText (CurView->hDC,FALSE,point.x,point.x, point.y,0, 2,2,
	  					Size,1,1,100, FALSE,AZ,txt,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0);   
	  		Points[2] = Points[1];
	  		AZ = getaz (Points[0],Points[1]);
	  		Points[1] = newpt (Points[0],AZ,(LineWidth-TextWidth)/2);
			Polyline (CurView->hDC,Points,2);  
	  		Points[1] = newpt (Points[2],AZ,-(LineWidth-TextWidth)/2);
			Polyline (CurView->hDC,&Points[1],2);  
		}
		else
		{
			Polyline (CurView->hDC,Points,2);  
		}
		SelectObject (CurView->hDC,hOldPen);
		DeleteObject (hDistPen);
		RestoreDC (CurView->hDC,-1);    
		return TRUE;
	}
	return FALSE;
}

void TempPolyline (HDC hDC, LPPOINT lpPoints, short nPnts, LPSTR TopText, LPSTR BottomText)
{   
	HPEN hTrackPen, hOldPen; 
	double	AZ, dist; 
	char	txt[32];
	POINT	Point1, Point2, point;
	LPPOINT	pPoint1, pPoint2;
	DPOINT	DPoint1, DPoint2;  
	float	size;
	
	hTrackPen = CreatePen (PS_SOLID,TrackWidth,AutoYellow(TrackColor));
	hOldPen = SelectObject (CurView->hDC,hTrackPen);
	Polyline (CurView->hDC,lpPoints,nPnts);  
	SelectObject (CurView->hDC,hOldPen);
	DeleteObject (hTrackPen);
	if (TempLineType==1 && TopText)
	{   
		size = -0.2;
		AZ=0;
		nPnts--;
		pPoint2 = lpPoints;
		pPoint2 += nPnts; 
		pPoint1 = pPoint2;
		pPoint1--; 
		Point1 = *pPoint1;
		Point2 = *pPoint2;
		DPoint1 = WinPtToBasePt(Point1);
		DPoint2 = WinPtToBasePt(Point2);  
		point.x = Point1.x + (Point2.x - Point1.x)/2;
		point.y = Point1.y + (Point2.y - Point1.y)/2;
		AZ = getazd  (&DPoint1,&DPoint2);
		DispText (hDC,FALSE,point.x,point.x, point.y,0, 4,2,size,1,1,100, FALSE,AZ,TopText,0,FALSE,0,0,-1,0,0,0,0,0,0,0,0,0,0,0,0); 
	}
	return;
}

UINT AddNewPoly (short Type,int nPnts,HANDLE hPoints,BOOL Display)
{   
	LPDPOINT	lpDPoints;
	long		NewRefno; 
	int			st;  
	char		Prefix[128], UDI[128], SymName[34], txt[128];
	BOOL		Redefine; 
	UINT		rtn=0;
	COLORREF	color;
	LPSTR		pStuff; 
	HANDLE		hStuff=0,hText=0, hTime=0, hTextTPL=0, hBuf=0;   
	long		lBuf=0;
	COLORREF	NewLineColor, NewAreaColor;
	float		NewLineWidth;
	double		NewRouteWidth;
	short		NewLineSymbol, NewAreaSymbol, NewSymbol; 
    		
	if (CurView->UpdateFile)
		_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
	if (!EditName[0]) return FALSE;
	_fstrcpy (PltName,EditName);
	PltType = 2;

	if (Type) 
	{    
		DPOINT	FirstPoint;
		
		lpDPoints =(HPDPOINT)GlobalLock (hPoints);
		FirstPoint = *lpDPoints;
		GlobalUnlock (hPoints);
		NewRefno = GetNewRefnoNoUpdate (EditName);
		if (!SetNewMacro (2))
			goto ExitLine;
		if (!SetLineSymbol (&NewLineSymbol,0))
			goto ExitLine;
		if (!SetLineWidth (&NewLineWidth,0))
			goto ExitLine;
		if (!SetRouteWidth (&NewRouteWidth,0))
			goto ExitLine;
		if (!SetLineColor (&NewLineColor,0))
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
		if (NewRouteWidth)
			rtn=AddPolyToMap (1,&nPnts, &hPoints,Type,NewRefno,hTime,-1,NewLineSymbol,(LPSHORT)pStuff,Prefix,UDI,-1,NewLineColor,(short)NewLineWidth,hText,hTextTPL,0,0,DigHiPrecis,&NewRouteWidth);
		else
			rtn=AddPolyToMap (1,&nPnts, &hPoints,Type,NewRefno,hTime,-1,NewLineSymbol,(LPSHORT)pStuff,Prefix,UDI,-1,NewLineColor,(short)NewLineWidth,hText,hTextTPL,0,0,DigHiPrecis,0);
		NewSymbol = NewLineSymbol;
	}
	else  
	{   
		LPVIEWPORT SaveVP=CurView; 
					
		NewRefno = GetNewRefnoNoUpdate (EditName);
		if (!SetNewMacro (3))
			goto ExitLine;
		if (!SetAreaSymbol (&NewAreaSymbol,0))
			goto ExitLine;
		if (!SetAreaColor (&NewAreaColor,0))
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
		rtn=AddPolyToMap (1,&nPnts, &hPoints,Type,NewRefno,hTime,-1,NewAreaSymbol,(LPSHORT)pStuff,Prefix,UDI,NewAreaColor,-1,0,0,0,0,0,DigHiPrecis,0);
		NewSymbol = NewAreaSymbol;   
	}
	CloseMap(TRUE);
	CloseRefIndex(FALSE);    		
	ForceRefIndex = ForceTAGIndex = FALSE;

	if (rtn)
	{
		if (!GetSymbolName (NewSymbol,SymName,0,2,0))
		{   
			HANDLE	hSymDesc=0;
			short	NumSyms=0;
					
    		_fstrcpy (PltName,EditName);
    		PltType = 2;
			AddToSymList (NewSymbol,&NumSyms,&hSymDesc); 
			AddSymToMap (NumSyms,hSymDesc,0,0); 
            DestroySymList (&NumSyms,&hSymDesc);  
        }
    	_fstrcpy (PickName,EditName);
		PltType = 2;
    	PickList[0].Segment = ItemSeg;
    	PickList[0].Offset = CurrentItem;  
    	PickList[0].FileNum=CurView->UpdateFile-1;
		PickList[0].SubFile = CurView->SubFile;
    	PickList[0].ViewID = CurView->ID; 
		PickList[0].ConfigID = CurrentConfig;
    	if (!HaveBlockingWindow && Display)
    		ProcessPickedItem(0,TRUE); 
    	rtn = GF_INCREASE_SUCCESS_COUNT; 
    }
ExitLine:
    GSSiGlobUlFree (&hStuff);
    GSSiGlobFree (&hText); 
    GSSiGlobFree (&hTextTPL); 
    GSSiGlobFree (&hTime); 
	GetNewRefnoNoUpdate (0);
    return rtn; 
}

UINT AddNewPoint (DPOINT Point,BOOL Display,LPSTR FileToEdit)
{   
	LPDPOINT	lpDPoints;
	long		NewRefno; 
	int			st;  
	char		Prefix[128], UDI[128], SymName[34], txt[128];
	double		size,rot;
	BOOL		Redefine; 
	UINT		rtn=0;
	COLORREF	color;
	double		AZ;
	short		NewPointSymbol;   
	LPSTR		pStuff; 
	HANDLE		hStuff=0,hText=0, hTime=0, hTextTPL=0, hBuf=0;   
	long		lBuf=0;
    
    *EditName = 0;
    if (FileToEdit && *FileToEdit)
    	_fstrcpy (EditName,FileToEdit);
    else
    {		
		if (CurView->UpdateFile)
		{
   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
			sprintf (txt,"#%i",CurView->UpdateFile);
			SetLayerVisibility (txt,1);
		}
	}
	if (!EditName[0])
		return FALSE;
	NewPointLoc = Point;
	_fstrcpy (PltName,EditName);
	PltType = 2;
	NewRefno = GetNewRefnoNoUpdate (EditName);  
	SetIntRefno (NewRefno); 
	rtn = 1;		
	if (!SetNewMacro (1)) 
		goto ExitPt;
	if (!SetPointSymbol (&NewPointSymbol,0))
		goto ExitPt;
	if (!SetPointSize (&size,0))
		goto ExitPt;
	if (!SetPointRot (&rot,0))
		goto ExitPt;
	if (!SetPointColor (&color,0))
		goto ExitPt;
	if (!SetNewText (1,&hText,&hTextTPL,&Point))
		goto ExitPt;
	if (!SetNewTime (1,&hTime))
		goto ExitPt;
	_fstrcpy (PltName,EditName);
	PltType = 2;
	_fstrcpy (Prefix,"[%NEW_POINT_PREFIX]");
	_fstrcpy (UDI,"[%NEW_POINT_UDI]"); 
	ExpandText (Prefix);
	ExpandText (UDI);
	NewRefno = GetNewRefno (EditName,Prefix,UDI,&NewPointSymbol,&Redefine);
	if (!SetNewStuff (1,&hStuff))
		goto ExitPt; 
	if (Redefine)
		DeleteRedefinedItem (EditName);
	pStuff = GlobalLock (hStuff);
	SelectVisList (FALSE);
    rtn=AddPointToMap (Point,NewRefno,hTime,NewPointSymbol,size,rot,(LPSHORT)pStuff,hText,hTextTPL,
    					Prefix,UDI,color,color,-1,FALSE,DigHiPrecis,&hBuf,&lBuf);
	if (rtn)
		SetGlobalValueLong("%LASTNEWPOINTREF", NewRefno);
	if (rtn && *Prefix)
	{   
		double	IncVal = GetGlobalDVal2 ("[%NEW_POINT_AUTOINC]",0);
					
		if (AutoIncTAG (Prefix,UDI,IncVal))
			SetGlobalValue ("%NEW_POINT_UDI",UDI);
	}
	CloseMap(TRUE);
	CloseRefIndex(FALSE);    		
	ForceRefIndex = ForceTAGIndex = FALSE;
	if (rtn)
	{
		if (!GetSymbolName (NewPointSymbol,SymName,0,2,0))
		{   
			HANDLE	hSymDesc=0;
			short	NumSyms=0;
					
    		_fstrcpy (PltName,EditName);
    		PltType = 2;
			AddToSymList (NewPointSymbol,&NumSyms,&hSymDesc); 
			AddSymToMap (NumSyms,hSymDesc,0,0); 
            DestroySymList (&NumSyms,&hSymDesc);  
        }
		if (!GetVisibility (NewPointSymbol))
			ToggleVisibility (NewPointSymbol);
	    _fstrcpy (PickName,EditName);
	    PltType = 2;  
		PickList[0].Refno = NewRefno;
		PickList[0].Desc = NewPointSymbol;
    	PickList[0].Segment = ItemSeg;
    	PickList[0].Offset = CurrentItem;  
    	PickList[0].FileNum=CurView->UpdateFile-1;
		PickList[0].SubFile = CurView->SubFile;
    	PickList[0].ViewID = CurView->ID;  
		PickList[0].ConfigID = CurrentConfig;
    	PickList[0].Desc = NewPointSymbol;
    	_fstrcpy (PickList[0].Prefix,Prefix);
    	_fstrcpy (PickList[0].UDI,UDI);
    	if (!HaveBlockingWindow && Display)  
    	{
    		ProcessPickedItem(0,TRUE); 
    		SaveFullWindowBitmap (CurView->hWnd);
		}
    	rtn = GF_INCREASE_SUCCESS_COUNT; 
    }
ExitPt:
    GSSiGlobUlFree (&hStuff);
    GSSiGlobFree (&hText); 
    GSSiGlobFree (&hTextTPL); 
    GSSiGlobFree (&hTime); 
	GetNewRefnoNoUpdate (0);
    return rtn; 
}



BOOL BlackLineMidPoint (POINT BitPoint,double AZ,LPPOINT LinePoints,double MaxDist,LPDPOINT pMidPoint,LPDOUBLE pWidth)
{
	DPOINT	MidPoint, DPoint,BitPointD, LeftEdge, RightEdge; 
	POINT	Point;
	double	inc=0.25, dist=0;    
	BOOL	HaveEdge;
	
	AZ = LTWOPI (AZ + HALFPI); 
	
	BitPointD = PointToDPoint (BitPoint);
	HaveEdge = FALSE; 
	LeftEdge = BitPointD;
	while (!HaveEdge)
	{   
		dist += inc; 
		if (MaxDist && dist > MaxDist)
			return FALSE;
		DPoint = dnewpt (BitPointD,AZ,dist); 
		Point.x = IDNINT (DPoint.x);
		Point.y = IDNINT (DPoint.y); 
		if (!PtInRect (&CurView->DrawRect,Point) || GetPixel (CurView->hDC,Point.x,Point.y))
			HaveEdge = TRUE;
		else
			LeftEdge = DPoint; 
	}
	HaveEdge = FALSE; 
	RightEdge = BitPointD;  
	dist = 0;
	while (!HaveEdge)
	{   
		dist -= inc;  
		if (MaxDist && fabs (dist) > MaxDist)
			return FALSE;
		DPoint = dnewpt (BitPointD,AZ,dist); 
		Point.x = IDNINT (DPoint.x);
		Point.y = IDNINT (DPoint.y); 
		if (!PtInRect (&CurView->DrawRect,Point) || GetPixel (CurView->hDC,Point.x,Point.y))
			HaveEdge = TRUE;
		else
			RightEdge = DPoint; 
	}
	*pMidPoint = MidPointD (LeftEdge,RightEdge);
	if (pWidth)
		*pWidth = ldistp (LeftEdge,RightEdge);
	return TRUE;
}

BOOL PickColor (POINT MousePoint,COLORREF Color,LPPOINT pSnapPoint, BOOL RemoveTrack,int Opt)
{   
// Opt=0	find nearest black pixel within Pickap
// Opt=1	find nearest black edge pixel within Pickap using CurrentAZ as current direction
// Opt=2	find nearest black midpoint withing Pickap using CurrentAZ as current direction
	POINT	Point, minpoint;
	double	dist, AZ, mindist = DBL_MAX, LineWidth = GetGlobalDVal2 ("[%BLACKLINEWIDTH]",2)/2 * BaseDistToWinDist;
	short	Pickap = SetPickAp(0)+2;
	short	x,y;
	BOOL	rtn=FALSE;
	DPOINT	DPoint;
	
	if (RemoveTrack && DoTrack && hTempPoints && nTempPoints)
	{
	 	HPPOINT	Points = (HPPOINT)GlobalLock (hTempPoints);
		NotPolyline (CurView->hDC,Points,(short)nTempPoints,0,0); 
		GlobalUnlock (hTempPoints);
	} 
	for (x = MousePoint.x - Pickap;x <= MousePoint.x + Pickap; x++)
		for (y = MousePoint.y - Pickap;y <= MousePoint.y + Pickap; y++)
			if (GetPixel (CurView->hDC,x,y) == Color)
			{   
				Point.x = x;
				Point.y = y; 
				if (PtInRect (&CurView->DrawRect,Point))
				{
					rtn = TRUE;
					dist = idist (Point,MousePoint);
					if (dist < mindist)
					{
						mindist = dist;
						minpoint = Point;
					}
				}
			} 
	if (rtn)
		*pSnapPoint = minpoint;  
	else
		return FALSE;
	switch (Opt)
	{
		case 0: 
			return TRUE;
		break;
		
		case 1:
			return TRUE;
		break;
		
		case 2:
		{
			double	AZ=0, Dist,MinDist=DBL_MAX;
			POINT	StartPoint=*pSnapPoint;
			short	i;
			
			for (i=0;i<8;i++,AZ+=HALFPI/2)
			{
				if (BlackLineMidPoint (StartPoint,AZ,0,0,&DPoint,&Dist))
				{   
					if (Dist >= LineWidth && Dist < MinDist)
					{
						MinDist = Dist;
						*pSnapPoint = DPointToPoint (DPoint);
					}
				}
			}           
			if (Dist < DBL_MAX)
				return TRUE;
		}
		break; 
	}
	return FALSE;
}  




 

BOOL SnapToPoint (DPOINT Point)
{   
	POINT	CursorPoint;
	
	LockCursor (&Point);
	if (!PtInWBounds (&CurrentPoint) && AutoPan)
	    CenterWindow (CurrentPoint,TRUE);
	CursorPoint = BasePtToWinPt (&CurrentPoint);
	ClientToScreen (CurView->hWnd,(LPPOINT)&CursorPoint);
	CreateDigCursor (CurView->hDC); 
	SetCurs ((HCURSOR)2,FALSE); 
	SetCursorPosGM (CursorPoint.x,CursorPoint.y,0);
	DisplayCoordinate2 (&CurrentPoint,CursorPoint); 
	return TRUE;
}

 

BOOL AutoAreaID(int NumNewPolyPoints,HANDLE hNewPolyPoints,LPSTR SaveUDI)
{   
	char	UDI[66]; 
	HPDPOINT	lpDPoint; 
	long	TotNum, Refno;   
	HIGHLIGHTDATA	HighlightData;
	
	*SaveUDI = 0;
	GetGlobalCVal ("[%NEW_AREA_UDI]",UDI,0);
	if (_fstricmp (UDI,"%AUTO")) 
		return TRUE;  
	ClearPolyOff (FALSE);
	lpDPoint = (HPDPOINT)GlobalLock (hNewPolyPoints);   
	AddAreaToOffsetFile (0,3,NumNewPolyPoints, lpDPoint,1,0,0);
	GlobalUnlock (hNewPolyPoints);
    ClearHighlightList (FALSE); 
	HighlightOnlyPoints = TRUE; 
	HighlightInArea (CurView->hWnd,0,TRUE,FALSE,0);   
	HighlightOnlyPoints = FALSE;
	ClearPolyOff (FALSE); 
	TotNum = BT_NUM_IN_INDEX (hHighlight);
	if (TotNum != 1) 
	{    
		char	Msg[32]="Multiple points picked";
		
	    ClearHighlightList (FALSE); 
		if (!TotNum)
			_fstrcpy (Msg,"No points picked");
		GSSiMessageBox (Msg,"Error in Auto Area Identifier",MB_ICONEXCLAMATION,0);
		return FALSE;
	}
	BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)&HighlightData);
    ClearHighlightList (FALSE); 
    SetGlobalValue ("%NEW_AREA_UDI",HighlightData.PD.UDI); 
    _fstrcpy (SaveUDI,"%AUTO");
	return TRUE;
}               

                                     

BOOL FindIntersectionsWithRef (long Segno,HANDLE hInt,double Tol,
        					   HPSEGMENTDATA SegData,LPLONG pNumSegs)
{
    short	pos, i, type[2];  
    HIGHLIGHTDATA	HighlightData;
	long	npnts[2], IntRefno, Refno=SegData[Segno].PD.Refno, NewSegno; 
 	HPDPOINT	pPolyPoints[2];
 	long	MaxNewSegs=1024;
 	short	NumNewSegs=0;
	HANDLE	hPoly[2]={0,0}, hNewSegs=GSSiGlobAlloc ( 618,GMEM_MOVEABLE,MaxNewSegs*4);  
	LPLONG	NewSegs=(LPLONG)GlobalLock (hNewSegs);
	BOOL	rtn=TRUE;  
	DPOINT	IntPoint;
	MNMXCORD	Bounds=SegData[Segno].Bounds; 
	double	IntDist[2], AtDist;
	INTERSECTIONKEY	IntKey;
	INTERSECTIONDATA	IntData; 
	BOOL	OutReverse[4], FlipReverse;
	short	WhichPoly[4];
	double	InAZ, OutAZ[4], AZ;                                             
	short	n;
	long	iseg;
    
    ClearHighlightList (FALSE);    
    ExpandBounds (&Bounds,Tol+P_TOL*4);
	type[0] = PickList[0].Type;  
	AtDist = SegData[Segno].FromDist;
   	if (!GetPolyPoints (&SegData[Segno].PD,SegData[Segno].Reverse,&npnts[0],&hPoly[0]))
   	{
   		GSSiGlobUlFree (&hNewSegs);
   		return TRUE;               
   	}
	if (HighlightInArea (CurView->hWnd,&Bounds,TRUE,FALSE,0) < 0)
   	{
   		GSSiGlobUlFree (&hNewSegs);
		return FALSE;
	}
   	pPolyPoints[0] = (HPDPOINT)GlobalLock (hPoly[0]);
   	if (type[0] == 2) 
   		ExtendPoly (npnts[0],pPolyPoints[0],-Tol); 
	pos = BT_FIRST;
  	while (rtn && !BT_FIND (hHighlight,(LPSTR)&IntRefno,pos,BT_ANY,(LPSTR)&HighlightData))  
   	{
   		pos=BT_NEXT; 
   		n = 0;
   		FlipReverse = FALSE;
   		if (IntRefno != Refno || fabs (SegData[Segno].Length - HighlightData.PD.Length) > P_TOL)         
   		{
	 		type[1] = HighlightData.PD.Type;
	   		if (GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npnts[1],&hPoly[1]))
	   		{   
	   			pPolyPoints[1] = (HPDPOINT)GlobalLock (hPoly[1]);
			   	if (type[1] == 2) 
			   		ExtendPoly (npnts[1],pPolyPoints[1],Tol); 
			}
	   	}
	   	else if (npnts[0] > 2 && !SegData[Segno].Reverse && ldistp (pPolyPoints[0][0],pPolyPoints[0][1]) < AtDist+P_TOL)
	   	{ 
	   		npnts[1] = 2;
	   		hPoly[1] = GSSiGlobAlloc ( 619,GMEM_MOVEABLE,2*sizeof(DPOINT));
   			pPolyPoints[1] = (HPDPOINT)GlobalLock (hPoly[1]);
	   		pPolyPoints[1][0] = pPolyPoints[0][0];
	   		pPolyPoints[1][1] = pPolyPoints[0][1];  
	   		HighlightData.PD.Length = ldistp (pPolyPoints[1][0],pPolyPoints[1][1]); 
	   	}
	   	else if (npnts[0] > 2 && !SegData[Segno].Reverse && AtDist-P_TOL > 0)
	   	{ 
	   		npnts[1] = 2;
	   		hPoly[1] = GSSiGlobAlloc ( 620,GMEM_MOVEABLE,2*sizeof(DPOINT));
   			pPolyPoints[1] = (HPDPOINT)GlobalLock (hPoly[1]);
	   		pPolyPoints[1][0] = pPolyPoints[0][0]; 
	   		AZ = getazd (&pPolyPoints[0][0],&pPolyPoints[0][1]);
	   		pPolyPoints[1][1] = dnewpt (pPolyPoints[1][0],AZ,AtDist);  
	   		HighlightData.PD.Length = ldistp (pPolyPoints[1][0],pPolyPoints[1][1]); 
	   	}
	   	else if (npnts[0] > 2 && SegData[Segno].Reverse &&
	   			(SegData[Segno].Length - 
	   			ldistp (pPolyPoints[0][npnts[0]-2],pPolyPoints[0][npnts[0]-1])) > AtDist+P_TOL)
	   	{ 
	   		npnts[1] = 2;
	   		hPoly[1] = GSSiGlobAlloc ( 621,GMEM_MOVEABLE,2*sizeof(DPOINT));
   			pPolyPoints[1] = (HPDPOINT)GlobalLock (hPoly[1]);
	   		pPolyPoints[1][0] = pPolyPoints[0][npnts[0]-1];
	   		pPolyPoints[1][1] = pPolyPoints[0][npnts[0]-2];  
	   		HighlightData.PD.Length = ldistp (pPolyPoints[1][0],pPolyPoints[1][1]); 
	   		FlipReverse = TRUE;
	   	}
	   	else if (npnts[0] > 2 && SegData[Segno].Reverse && AtDist-P_TOL > 0)
	   	{ 
	   		npnts[1] = 2;
	   		hPoly[1] = GSSiGlobAlloc ( 622,GMEM_MOVEABLE,2*sizeof(DPOINT));
   			pPolyPoints[1] = (HPDPOINT)GlobalLock (hPoly[1]);
	   		pPolyPoints[1][0] = pPolyPoints[0][npnts[0]-1];
	   		AZ = getazd (&pPolyPoints[0][npnts[0]-1],&pPolyPoints[0][npnts[0]-2]);
	   		pPolyPoints[1][1] = dnewpt (pPolyPoints[1][0],AZ,SegData[Segno].Length-AtDist);  
	   		HighlightData.PD.Length = ldistp (pPolyPoints[1][0],pPolyPoints[1][1]); 
	   		FlipReverse = TRUE;
	   	}
	   	if (hPoly[1])
	   	{
			n = IntersectPolys2 (npnts[0],pPolyPoints[0],
								 npnts[1],pPolyPoints[1],
								 AtDist,IntDist,&IntPoint,&InAZ,
								 OutAZ,OutReverse,WhichPoly,FALSE);
		}	   	
		while (n--)
		{
			for (iseg=0;iseg<NumNewSegs;iseg++)
			{            
				if (fabs (IntDist[0] - SegData[NewSegs[iseg]].AtDist) < P_TOL)
					IntDist[0] = SegData[NewSegs[iseg]].AtDist;	
				if (IntDist[0] < SegData[NewSegs[iseg]].AtDist)
				{
					_fmemmove (&NewSegs[iseg+1],&NewSegs[iseg],(size_t)((NumNewSegs-iseg)*4));
					break;
				}
			}
			IntKey.Segno = Segno;
			IntKey.AtDist = IntDist[0];  
			IntKey.TurnAZ = DeltaAZ (InAZ,OutAZ[n]); 
			NewSegno = (*pNumSegs)++;
			if (NewSegno < MaxSegments)
			{  
				NewSegs[iseg] = NewSegno;
				NumNewSegs++;
				SegData[NewSegno].Reverse = OutReverse[n];   
				if (!WhichPoly[n])
					SegData[NewSegno] = SegData[Segno];
				SegData[NewSegno].AtDist = IntDist[0];
				SegData[NewSegno].FromSeg = Segno;
				if (WhichPoly[n])
				{
			        SegData[NewSegno].PD.Refno = IntRefno;
        			SegData[NewSegno].Length = HighlightData.PD.Length + Tol*2; 
        			if (SegData[NewSegno].Reverse)
        				SegData[NewSegno].FromDist = SegData[NewSegno].Length - (IntDist[1]-Tol);
        			else
        				SegData[NewSegno].FromDist = IntDist[1]-Tol;
        			if (FlipReverse)
        			{
        				if (SegData[NewSegno].Reverse)
        					SegData[NewSegno].Reverse = FALSE;
        				else
        					SegData[NewSegno].Reverse = TRUE;
        			}
        			SegData[NewSegno].Bounds = HighlightData.PD.Rect;  
        			SegData[NewSegno].PD = *(LPPICKDATAHEADER)&HighlightData.PD;
        		} 
        		else
        			SegData[NewSegno].FromDist = IntDist[0];
        		if (SegData[NewSegno].Length-SegData[NewSegno].FromDist < Tol + P_TOL)
        			(*pNumSegs)--;	
        		else
        		{
	                IntData.OnRef = Refno;
	                IntData.IntPoint = IntPoint;
					IntKey.NextSeg = NewSegno;
					BT_PUT (hInt,(LPSTR)&IntKey,(LPSTR)&IntData);
				}
			}
			else 
				rtn = FALSE;
		}
		GSSiGlobUlFree (&hPoly[1]);
    }
	GSSiGlobUlFree (&hPoly[0]);
	GSSiGlobUlFree (&hNewSegs);    
    ClearHighlightList (FALSE);    
	return rtn;
}

void SetRedefineData (LPPICKDATA pRedefineData)
{
	if (pRedefineData)
	{
		HaveRedefineData = TRUE;
		RedefineData = *pRedefineData;
	}
	else
		HaveRedefineData = FALSE;
	return;
}

short SetNewElementValues (HWND hWnd,int Type)
{
    short nRc=-1;
    
	switch (Type)
	{
		case 1: //points
		{
			FARPROC lpfnNEWPOINTSETMsgProc;
			
			lpfnNEWPOINTSETMsgProc = MakeProcInstance((FARPROC)NEWPOINTSETMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"NEWPOINTSET", hWnd, lpfnNEWPOINTSETMsgProc);
			FreeProcInstance(lpfnNEWPOINTSETMsgProc);
		}
		break;
		            	 
        case 2: //lines
		{
			FARPROC lpfnNEWLINESETMsgProc;
			
			lpfnNEWLINESETMsgProc = MakeProcInstance((FARPROC)NEWLINESETMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"NEWLINESET", hWnd, lpfnNEWLINESETMsgProc);
			FreeProcInstance(lpfnNEWLINESETMsgProc);
		}
		break;
		            	 
        case 3:	//areas
		{
			FARPROC lpfnNEWAREASETMsgProc;
			
			lpfnNEWAREASETMsgProc = MakeProcInstance((FARPROC)NEWAREASETMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"NEWAREASET", hWnd, lpfnNEWAREASETMsgProc);
			FreeProcInstance(lpfnNEWAREASETMsgProc);
		}
       	break; 
    }
    return nRc;
}
