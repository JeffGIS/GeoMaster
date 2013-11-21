#include "graphint.h"    
#include "translat.h"
#include "pnet.h"

typedef	struct	{int	nrow,ncol;
			int	left,bottom;
			int	cellw,cellh;
			int	ifac;
			int	GridOffset;
			double	fac;
			}STATEGRIDHEADER;
typedef STATEGRIDHEADER	*LPSTATEGRIDHEADER;

static	char	StateIDs[74][4];  
static	STATEGRIDHEADER Header;
//static	DPOINT	CellPoint[16][1024];
static	int		nCellPts[16], LineID[16];

static	BYTE	CellArray[256][256];
typedef struct	{short x,y;}POINT16;    
typedef POINT16	*LPPOINT16;

#include "gmextern.h"

typedef struct {long FromAddL, ToAddL,
					 FromAddR, ToAddR,
					 ZipL, ZipR,
					 CountyL, CountyR,
					 MCDL, MCDR;} PNADDRESSDATA;
typedef PNADDRESSDATA	FAR	*LPPNADDRESSDATA;

int LoadPuertoRicoCities (int i)
{
	HANDLE	hFile, hGPXElem, hWpt,hName,hCoord, hVal;
	LPSTR	pVal, popLoc, cLoc;
	int		loc;
	char	str[256], name[128];
	int		iPop;
	double  lat, lon;

	if ((hFile = LoadXMLFile ("H:\\kml.php")))
	{

			loc = 0;
			while ((hWpt = GetNextXMLElement (hFile,&loc,"Placemark")))
			{
				DPOINT	Wpt;
				int	loc2 = 0;
				if ((hName = GetNextXMLElement (hWpt,&loc2,"name")))
				{
					if ((hVal = GetXMLElementValue (hName)))
					{
						pVal = GlobalLock (hVal);
						strcpy (name,pVal);
						ConvertSpanishText (name);
						GSSiGlobUlFree (&hVal);
					}
					GSSiGlobFree (&hName);
				}
				if ((hName = GetNextXMLElement (hWpt,&loc2,"description")))
				{
					if ((hVal = GetXMLElementValue (hName)))
					{
						pVal = GlobalLock (hVal);
						popLoc = strstr (pVal,"population:");
						popLoc += 11;
						iPop = atoi (popLoc);
						GSSiGlobUlFree (&hVal);
					}
					GSSiGlobFree (&hName);
				}
				if ((hName = GetNextXMLElement (hWpt,&loc2,"coordinates")))
				{
					if ((hVal = GetXMLElementValue (hName)))
					{
						pVal = GlobalLock (hVal);

						cLoc = strchr (pVal,',');
						*cLoc++ = 0;

						lon = atof (pVal);
						lat = atof (cLoc);
						
						GSSiGlobUlFree (&hVal);
					}
					GSSiGlobFree (&hName);
				}

				sprintf (str,"%s\t%i\t%f\t%f",name,iPop,lat,lon);
				AppendFile ("c:\\temp\\prcities.txt",str);
				GSSiGlobFree (&hWpt);
			}
	}
	GSSiGlobFree (&hFile);
	return 0;
}

void ConvertSpanishText (LPBYTE pTxt)
{
	LPBYTE pLoc = pTxt;
	BYTE	last;

	while (*pLoc)
	{
		if (*pLoc == 195)
		{
			pLoc++;
			last = *pLoc;
			switch (*pLoc++)
			{
/*				case 129:
					*pTxt++ = 193;
					break;
				case 161:
					*pTxt++ = 225;
					break;
				case 169:
					*pTxt++ = 233;
					break;
				case 172:
				case 173:
					*pTxt++ = 237;
					break;
				case 178:
				case 179:
					*pTxt++ = 243;
					break;
				case 177:
					*pTxt++ = 241;
					break;
				case 186:
					*pTxt++ = 250;
					break;*/
				default:
					*pTxt++ = last + 64;
			}
		}
		else
			*pTxt++ = *pLoc++;
	}
	*pTxt = 0;
	return;
}

short ProvinceToState (short prov)
{
	switch (prov)
	{
		case 10:		//NEW
			return 65;
		case 11:		//PEI
			return 70;
		case 12:		//NOV
			return 66;
		case 13:		//NBR
			return 64;
		case 24: 		//QUE
			return 68;
		case 35:		//ONT
			return 67;
		case 46:		//MAN
			return 63;
		case 47:		//SAS
			return 69;
		case 48: 		//ALB
			return 61;
		case 59: 		//BC
			return 62;
		case 60:		//YUK
			return 71;
		case 61:		//NWT
			return 72;
	}
	return 0; 
} 

void AZtoDirection2(double az,LPSTR str)
#if ENABLETRACE
{GSSiEnterProg (1401);
#endif
{
	double	dinc;

	az = LTWOPI(az);
	dinc = HALFPI/4;
	if (az < dinc || az > TWOPI - dinc)
		_fstrcpy(str,"E");
	else if (az < dinc*3)
		_fstrcpy(str,"NE");
	else if (az < dinc*5)
		_fstrcpy(str,"N");
	else if (az < dinc*7)
		_fstrcpy(str,"NW");
	else if (az < dinc*9)
		_fstrcpy(str,"W");
	else if (az < dinc*11)
		_fstrcpy(str,"SW");
	else if (az < dinc*13)
		_fstrcpy(str,"S");
	else
		_fstrcpy(str,"SE"); 
{
#if ENABLETRACE
GSSiExitProg (1401);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  

double GetLocalCorrection (LPMNMXCORD pBounds)
#if ENABLETRACE
{GSSiEnterProg (1402);
#endif
{
	double	d1,d2,f, MidX, MidY, Dist;
	DPOINT	p1,p2;                    
	
    MidX = (pBounds->xmn + pBounds->xmx) / 2;
    MidY = (pBounds->ymn + pBounds->ymx) / 2; 
    Dist = (pBounds->xmx - pBounds->xmn + pBounds->ymx - pBounds->ymn) / 2;
	p1.x = MidX - Dist;
	p1.y = MidY;
	p2.x = MidX + Dist;
	p2.y = MidY;
	d1=ArcDistance(p1,p2);
	p1.y = MidY - Dist;
	p1.x = MidX;
	p2.y = MidY + Dist;
	p2.x = MidX;
	d2=ArcDistance(p1,p2);
	f = d1/d2; 
{
#if ENABLETRACE
GSSiExitProg (1402);
#endif
    return f;
}
#if ENABLETRACE
}
#endif
}  

BOOL MakeMap (LPSTR Args)
#if ENABLETRACE
{GSSiEnterProg (1403);
#endif
{
	double	Scale, InScale;   
	int		bmwidth,bmheight,i,j;
	DPOINT	DPoint, InPoint;
	long	hNum; 
	LPSTR	ParLoc, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6;
	HANDLE	hMem;  
	MNMXCORD	Rect;
	BOOL	FromLimits, Immediate; 
	char	Outstring[4][128], str[256], txt[128]; 
	char	InCity[64], NearCity[64],IntName[128],ScaleFile[128];
	long	InPop=0, NearPop=0, NearBigPop=0;
	CITIESKEY1	CitiesKey1;
	CITIESDATA1	CitiesData1;
	HANDLE	hBTCities1; 
	long	idist;
	char	dir[16];
	double	NearDist, NearBigDist, NearAZ, NearBigAZ;
	MNMXCORD	MinMax; 
	BOOL	SaveMemMap=MemMap;   
	DWORD	SaveMMH=MemMapHeight,SaveMMW=MemMapWidth;
	LPSTR	CmdMess, pDot;
	HCURSOR	hcurSave;
	BOOL	ReturnMapCmd=GetGlobalBVal2 ("[%RETURNMAPCMD]",FALSE); 
	double	WDist;
	DPOINT	CityPoint, p1, p2;
	BOOL	OpenBP=FALSE;
	BOOL	rtn=FALSE;
			
   	MemMapStartTime = GetTickCount();
   	TotPointsProcessed = 0;   
	GSSiGlobFree (&hMemMapColorMap);		
	if (!(ParLoc = MatchLev (Args,',')))
		goto Exit;
	hMem = GSSiGlobAlloc ( 625,GMEM_MOVEABLE,2048*6);
	Arg1 = GlobalLock(hMem);
	Arg2 = Arg1 + 2048; 
	Arg3 = Arg2 + 2048; 
	Arg4 = Arg3 + 2048; 
	Arg5 = Arg4 + 2048; 
	Arg6 = Arg5 + 2048; 
			
	_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
	*ParLoc = '\0';
	_fstrcpy (Arg1,Args);
	ExpandText (Arg1);  
	if (_fstricmp(Arg1,"COORD") &&
		_fstricmp(Arg1,"CITY")	&&
		_fstricmp(Arg1,"BOUNDS"))
		goto RtnFalse;
			
	if (!(ParLoc = MatchLev (Arg2,','))) goto RtnFalse;
	_fstrcpy (Arg3,(LPSTR)(ParLoc+1));
	*ParLoc = '\0';
	ExpandText (Arg2); 
	ExpandText (Arg3);  
			
	SetViewport(*pCommandViewport);
	bmwidth = GetGlobalLVal2("[BITMAP_WIDTH]",600); 
	bmheight = GetGlobalLVal2("[BITMAP_HEIGHT]",400); 
    if (bmwidth != MemMapWidth || bmheight != MemMapHeight || !MemMap)
    {   
		MemMap = TRUE;
    	MemMapWidth = bmwidth;
    	MemMapHeight = bmheight; 
    	if (hdcMemMap)
    	{
			HBITMAP hbm = SelectObject(hdcMemMap, hbmpOld);
			DeleteObject (hbm); 
			DeleteDC (hdcMemMap);
			CurView->hDC = OldDC;
    	}	
		hdcMemMap = CreateCompatibleDC(CurView->hDC);    
		hMemBitmap = CreateCompatibleBitmap (CurView->hDC,(int)MemMapWidth,(int)MemMapHeight); 
		OldDC = CurView->hDC;
		CurView->hDC = hdcMemMap;
		hbmpOld = SelectObject(hdcMemMap, hMemBitmap); 
	    SetMainRect (CurView->hWnd,CurView->hDC,NULL,4);
	    SetupViewports (CurView->hWnd,CurView->hDC,0,MainRect,0);
	}
			
	if (!_fstricmp(Arg1,"COORD"))
    {
		if (!(ParLoc = MatchLev (Arg3,','))) goto RtnFalse;
		_fstrcpy (Arg4,(LPSTR)(ParLoc+1));
		*ParLoc = '\0';   
		if (!(ParLoc = MatchLev (Arg4,','))) goto RtnFalse;
		_fstrcpy (Arg5,(LPSTR)(ParLoc+1));
		*ParLoc = '\0';   
		ExpandText (Arg5); 
		if ((ParLoc = MatchLev (Arg5,',')))
		{
			_fstrcpy (Arg6,(LPSTR)(ParLoc+1));
			*ParLoc = '\0';
		}
		else
			*Arg6 = 0;   
		ExpandText (Arg6); 
		if (*Arg6)
		{   
			LPSTR	pName;
			
			hMemMapColorMap = GSSiGlobAlloc (0,GMEM_MOVEABLE,256);  
			pName = GlobalLock (hMemMapColorMap);
			_fstrcpy (pName,Arg6);
			GlobalUnlock (hMemMapColorMap);
		}
		else
		{
			GetGlobalCVal ("[%COLORMAP]",str,NULL);
			if (*str)
			{   
				LPSTR	pName;
				
				hMemMapColorMap = GSSiGlobAlloc (0,GMEM_MOVEABLE,256);  
				pName = GlobalLock (hMemMapColorMap);
				_fstrcpy (pName,str);
				GlobalUnlock (hMemMapColorMap);
			}
		}
		DPoint.x = atof (Arg2);
		DPoint.y = atof (Arg3); 
		InPoint = DPoint;
		ConvertCoord (&DPoint,2,1); 
		ExpandText (Arg4);  
		Scale = InScale = atof (Arg4);
		ExpandText (Arg5);  
		_fstrcpy (MemMapName,Arg5);
	}
	else if (!_fstricmp(Arg1,"CITY")) 
	{    
		
		CmdMess = GlobalLock (hCmdMess);
		_fstrcpy (MemMapName,Arg3);
		if (!GetCityCoord (Arg2,&DPoint,&MinMax))
		{
			_fstrcpy (CmdMess,"ERROR;Unable to find specified city");
			GlobalUnlock (hCmdMess);
			goto RtnFalse;
		}
		p1.x = MinMax.xmn;
		p2.x = MinMax.xmx;
		p1.y = (MinMax.ymn + MinMax.ymx) / 2;
		p2.y = p1.y;
		WDist = ArcDistance(p1, p2);
		InScale = (WDist*MFT / 5280) /2;
		CityPoint = DPoint;
		ConvertCoord (&CityPoint,1,2); 
		sprintf (CmdMess,"%.14lg;%.14lg;%.14lg",CityPoint.y,CityPoint.x,InScale);
		GlobalUnlock (hCmdMess);
	}
	else if (!_fstricmp(Arg1,"BOUNDS")) 
	{   
		DPOINT	PT;
		double	xmid,ymid,degperpixelx,degperpixely, lc, aspect=(double)bmheight/(double)bmwidth;
		short	Attempt=0;
		BOOL	Err;
		LPSTR	pDot, pComma;
		
		_fstrcpy (MemMapName,Arg3);
		if ((ParLoc = MatchLev (Arg3,',')))
		{
			_fstrcpy (Arg4,(LPSTR)(ParLoc+1));
			*ParLoc = '\0';   
			_fstrcpy (MemMapName,Arg3);
			ExpandText (Arg4); 
			if (*Arg4)
			{   
				LPSTR	pName;
				
				hMemMapColorMap = GSSiGlobAlloc (0,GMEM_MOVEABLE,256);  
				pName = GlobalLock (hMemMapColorMap);
				_fstrcpy (pName,Arg4);
				GlobalUnlock (hMemMapColorMap);
			} 
		}
		pDot = _fstrrchr (MemMapName,'.');
		if (!pDot)
			goto RtnFalse;
		if (!_fstrnicmp (pDot,".gmd(",4))
		{   
			LPSTR loc[4];

			pDot += 4;
			*pDot++ = 0;

			GetParmLoc (4,',',pDot,loc);
			MemMapID = atol (loc[0]); 
			MemMapSubDir = atoi (loc[1]);
			MemMapGridID = atol (loc[2]);
			MemMapGridCellID = atol (loc[3]);
			if (!ExistFile (MemMapName)) 
			{
				char	DefStr[]="MapID(B4),SubDir(B4),Format(B2),Size(B4),Time(B4),NumPoints(B4),Loc(B4),LocalCorrection(R8),MetersPerDegree(R8),Scale(R8),GridID(B4),GridCellID(B4),MNX(R8),MNY(R8),MXX(R8),MXY(R8)";   
				
				CreateGWDDatabase (MemMapName,1,FALSE,0,1,DefStr); 
			}
			pDot = _fstrrchr (MemMapName,'.');
			sprintf (pDot,"%i.bin",MemMapSubDir); 
			if (!ExistFile (MemMapName)) 
			{
				HFILE	FidBIN;
			
				FidBIN = GSSiOpenFile (MemMapName,NULL,OF_CREATE);
				GSSiClose (FidBIN); 
			}
			_fstrcpy (pDot,".gmd");
		}
		ZOOMLevelWanted = -1;
TryMapAgain2:
		CurView->NewBounds = atobounds (Arg2,&Err);
		DisplayCycle++;
		if (CurView->OrthoRes >=0 || !CurView->WindowZoomedToOrtho)
		    CurView->WindowZoomedToOrtho = FALSE;
		SetScaleAndMidpointFromBounds (CurView);
	    SetBounds (CurView->hWnd,NULL);
		CurView->WindowIsZoomed = TRUE; 
		SelectClipRgn (CurView->hDC,NULL);			
		SetDisplayMode (CurView->hDC,GF_TEXTMODE); 
		xmid = (CurView->WBounds.xmn+CurView->WBounds.xmx)/2;
		ymid = (CurView->WBounds.ymn+CurView->WBounds.ymx)/2; 
		degperpixelx = (CurView->WBounds.xmx - CurView->WBounds.xmn)/(double)bmwidth;
		degperpixely = (CurView->WBounds.ymx - CurView->WBounds.ymn)/(double)bmheight;
		p1.x = CurView->WBounds.xmn;
		p2.x = CurView->WBounds.xmx;
		p1.y = (CurView->WBounds.ymn + CurView->WBounds.ymx) / 2;
		p2.y = p1.y;
		WDist = GetBaseDist (&p1, &p2);
		Scale = (WDist*MFT / 5280) /2;
		CmdMess = GlobalLock (hCmdMess);
		sprintf (CmdMess,"%.14lg;%.14lg;%.14lg;%.14lg;%.14lg",xmid,ymid,Scale,degperpixelx,degperpixely);
		GlobalUnlock (hCmdMess);
		DoPaint = TRUE;	 
		hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
		PaintMap (CurView->hWnd,CurView->hDC,TRUE,NULL,0); 
		if (!Attempt++ && ZOOMLevelUsed && NumStreetMidPoints < GetGlobalLVal2 ("[%MINSTREETS]",0))
		{
			ZOOMLevelWanted = ZOOMLevelUsed - 1; 
			goto TryMapAgain2;                   
		}
		MemMap = SaveMemMap;
		MemMapWidth = SaveMMW;
		MemMapHeight = SaveMMH; 
		GSSiSetCursor (hcurSave); 
		goto RtnTrue;
	} 
	else 
	{    
		DWORD	SaveMMH=MemMapHeight,SaveMMW=MemMapWidth;
		BOOL	Err;
		BOOL	rtn=TRUE;
				
 		MinMax = atobounds (Arg2,&Err); 
		if ((ParLoc = MatchLev (Arg3,','))) 
		{   
			short	d;
			_fstrcpy (Arg4,(LPSTR)(ParLoc+1));
			*ParLoc = '\0'; 
			ExpandText (Arg4);
			d = atoi (Arg4); 
			if (MinMax.xmx - MinMax.xmn > MinMax.ymx - MinMax.ymn)
			{
				MemMapWidth = d;
				MemMapHeight = d * (MinMax.ymx - MinMax.ymn)/(MinMax.xmx - MinMax.xmn);
			}
			else
			{
				MemMapHeight = d;
				MemMapWidth = d * (MinMax.xmx - MinMax.xmn) / (MinMax.ymx - MinMax.ymn);
			}
	    	if (hdcMemMap)
	    	{
				HBITMAP hbm = SelectObject(hdcMemMap, hbmpOld);
				DeleteObject (hbm); 
				DeleteDC (hdcMemMap);
				CurView->hDC = OldDC;
	    	}	
			hdcMemMap = CreateCompatibleDC(CurView->hDC);    
			if ((hMemBitmap = CreateCompatibleBitmap (CurView->hDC,(int)MemMapWidth,(int)MemMapHeight)))
			{
				OldDC = CurView->hDC;
				CurView->hDC = hdcMemMap;
				hbmpOld = SelectObject(hdcMemMap, hMemBitmap); 
			    SetMainRect (CurView->hWnd,CurView->hDC,NULL,4);
			    SetupViewports (CurView->hWnd,CurView->hDC,0,MainRect,0); 
			}
			else
				rtn = FALSE;
		}
		_fstrcpy (MemMapName,Arg3);
		ExpandText (MemMapName);
		CurView->NewBounds = MinMax;
		if (CurView->OrthoRes >=0 || !CurView->WindowZoomedToOrtho)
		    CurView->WindowZoomedToOrtho = FALSE;
	    SetBounds (CurView->hWnd,NULL);
		CurView->WindowIsZoomed = TRUE; 
		SelectClipRgn (CurView->hDC,NULL);			
		SetDisplayMode (CurView->hDC,GF_TEXTMODE);
		hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
		DoPaint = TRUE;	
		if (rtn) 
			PaintMap (CurView->hWnd,hdcMemMap,TRUE,NULL,0);  
		MemMap = SaveMemMap;
		MemMapWidth = SaveMMW;
		MemMapHeight = SaveMMH; 
		GSSiSetCursor (hcurSave); 
		if (!SaveMemMap)
			RedisplayWindow();
		if (rtn)   
			goto RtnTrue;
		else
			goto RtnFalse;
	}
				
	if (InScale < 0)
		Scale = GetGlobalDVal ("[IN_CITY_SCALE]");
	else
		Scale = InScale;
			
	if (InScale < 0)
	{
		PickItems (CurView->hWnd,DPoint); 
				
		InPop = 0; 
		NearPop = 0; 
				
    	if (NumPicked)
    	{   
    		HANDLE	hBTTiger1;
    		HFILE	FidTiger1;
    		OFSTRUCT	OFStruct; 
    		long	Offset, snum, IntLong, IntLat, IntTLID, ToLong, ToLat;
    		TIGER1_PEOPLENET	Tiger1;  
    		char	Buff[64], OnName[128], TestName[128]; 
    		int		st;
    		double	IntDist, Dist;
    		HANDLE	hIntersect;   
    		DPOINT	IntPoint;
			struct	{
						long Long, Lat, TLID;
					}	IntersectKey;
		    		
		    		
			hBTTiger1 = BT_OPEN ("tiger1.btr", 0, BT_READ, 0);
			FidTiger1 = GSSiOpenFile ("tiger1.dat",&OFStruct,OF_READ);
	    	if (!BT_FIND (hBTTiger1,(LPSTR)&PickList[NumPicked-1].Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset))
	    	{   
	    		GSSillseek (FidTiger1,Offset,0);
	    		BigRead (FidTiger1,(HPSTR)&Tiger1,sizeof(TIGER1_PEOPLENET));   
	    		CitiesKey1.State = Tiger1.STATEL;
	    		CitiesKey1.FIPS = Tiger1.FPLL; 
	    		if (!CitiesKey1.State)
	    		{   
		    		CitiesKey1.State = Tiger1.STATER;
		    		CitiesKey1.FIPS = Tiger1.FPLR; 
	            }    		
	    		hBTCities1 = BT_OPEN ("cities1.btr",0,BT_READ,0); 
	    		if (!BT_FIND (hBTCities1,(LPSTR)&CitiesKey1,BT_FIRST,BT_EQ,(LPSTR)&CitiesData1))
	    		{
	    			_fstrcpy (InCity,CitiesData1.Name);
	    			InPop = CitiesData1.Pop;
	    		}
	    		else
	    			InPop = 0;
	    		BT_CLOSE (hBTCities1);  
	    	} 
	    	GSSiClose (FidTiger1);
	    	BT_CLOSE (hBTTiger1);
	    } 
	    if (InPop < GetGlobalDVal("[IN_CITY_POP]"))  
	    {   
	    	CurState = -1;
	    	NearPop = GetNearCity (DPoint,1,InCity,&NearDist,&NearAZ);
	    	Scale = max (Scale,IDNINT(NearDist / 5280));
	    } 
	}
	if (fabs(Scale)>0.000000001) 
	{   
		DPOINT	PT;
		double	xmid,ymid,degperpixelx,degperpixely, lc, aspect=(double)bmheight/(double)bmwidth;
		short	Attempt=0;
		DWORD	SaveMMH=MemMapHeight,SaveMMW=MemMapWidth;
		
		ZOOMLevelWanted = -1;
TryMapAgain:
		NumStreetMidPoints = 0;		
		PT = NewLatLong(DPoint.y,DPoint.x,Scale*5280*FTM,PY/2);
        Rect.xmx = PT.x;
		PT = NewLatLong(DPoint.y,DPoint.x,Scale*5280*FTM,3*PY/2);
        Rect.xmn = PT.x; 
		PT = NewLatLong(DPoint.y,DPoint.x,Scale*5280*FTM*aspect/2,0);
        Rect.ymx = PT.y;
		PT = NewLatLong(DPoint.y,DPoint.x,-Scale*5280*FTM*aspect/2,0);
        Rect.ymn = PT.y; 
		lc = GetLocalCorrection (&Rect);	            
//        if (bmwidth < bmheight)
        {
        	degperpixelx = (Rect.xmx - Rect.xmn)/ bmwidth;
        	degperpixely = degperpixelx * lc;
        }
/*        else
        {
        	degperpixely = (Rect.ymx - Rect.ymn) / bmheight;
        	degperpixelx = degperpixely / lc;
        }*/
		CurView->NewBounds = Rect; 
		DisplayCycle++;
		if (CurView->OrthoRes >=0 || !CurView->WindowZoomedToOrtho)
		    CurView->WindowZoomedToOrtho = FALSE;
		SetScaleAndMidpointFromBounds (CurView);
	    SetBounds (CurView->hWnd,NULL);
		CurView->WindowIsZoomed = TRUE; 
		SelectClipRgn (CurView->hDC,NULL);			
		SetDisplayMode (CurView->hDC,GF_TEXTMODE); 
		xmid = (CurView->WBounds.xmn+CurView->WBounds.xmx)/2;
		ymid = (CurView->WBounds.ymn+CurView->WBounds.ymx)/2; 
		degperpixelx = (CurView->WBounds.xmx - CurView->WBounds.xmn)/(double)bmwidth;
		degperpixely = (CurView->WBounds.ymx - CurView->WBounds.ymn)/(double)bmheight;
		CmdMess = GlobalLock (hCmdMess); 
		if (ReturnMapCmd)
			sprintf (CmdMess,"MAPRESULT:%.14lg;%.14lg;%.14lg;%.14lg;%.14lg",xmid,ymid,Scale,degperpixelx,degperpixely);
		else
			sprintf (CmdMess,"%.14lg;%.14lg;%.14lg;%.14lg;%.14lg",xmid,ymid,Scale,degperpixelx,degperpixely);
		GlobalUnlock (hCmdMess);
		DoPaint = TRUE;	 
		if (PeopleNet)
			OpenBP = OpenBasePens (); 
		hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
		PaintMap (CurView->hWnd,CurView->hDC,TRUE,NULL,0); 
		CloseBasePens (OpenBP);
		if (!Attempt++ && ZOOMLevelUsed && NumStreetMidPoints < GetGlobalLVal2 ("[%MINSTREETS]",0))
		{
			ZOOMLevelWanted = ZOOMLevelUsed - 1; 
			goto TryMapAgain;                   
		} 
		_fstrcpy (str,"[%VPZOOM]");
		ExpandText (str);
		_fstrcpy (ScaleFile,"[%DL]scale.txt");
		{   
			HFILE	FidScale;
			
			FidScale = GSSiOpenFile (ScaleFile,NULL,OF_CREATE);
			if (FidScale != HFILE_ERROR)
			{
				fputstring (str,FidScale);
				GSSiClose (FidScale);
			}
		} 
		if (GetGlobalBVal2 ("[%APPLYCOLORCHANGES]",TRUE)) 
		{
			LoadColorChanges ();
			ChangeImageColors (MemMapName,FALSE);
		} 
		else
			SetGlobalValueBool ("%APPLYCOLORCHANGES",TRUE);
		MemMap = SaveMemMap;
		MemMapWidth = SaveMMW;
		MemMapHeight = SaveMMH; 
		GSSiSetCursor (hcurSave); 
		if (!SaveMemMap)
			RedisplayWindow();
	} 
RtnTrue:
 
	rtn = TRUE;
RtnFalse:
	MemMap = SaveMemMap;
	MemMapWidth = SaveMMW;
	MemMapHeight = SaveMMH; 
	GSSiGlobUlFree (&hMem);
Exit:
{
#if ENABLETRACE
GSSiExitProg (1403);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}  
 

BOOL GetCountyName (short State,short County,LPSTR CountyName)
#if ENABLETRACE
{GSSiEnterProg (1404);
#endif
{
	static	BOOL	First = TRUE; 
	HANDLE	hBTCountyName;  
	char	str[130]; 
	short	st;
	struct	{short	State, County;} CNKEY;
	BOOL	rtn = FALSE;
	
	*CountyName = 0;  
/*	if (First)
	{
		BTVARDESC	BTVar[2];
		HFILE	Fid;
		OFSTRUCT	OFStruct;
		LPSTR	pCounty;   
		
		if ((Fid = GSSiOpenFile ("countynm.txt",&OFStruct,OF_READ)) == HFILE_ERROR)
			return FALSE;
		BTVar[0].BT_VARTYP=BT_INTEGER;
		BTVar[0].BT_VARLEN=2;
		BTVar[0].BT_VAROFF=0;
		BTVar[1].BT_VARTYP=BT_INTEGER;
		BTVar[1].BT_VARLEN=2;
		BTVar[1].BT_VAROFF=2;
		BT_CREATE ("countynm.btr", 34, FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
		hBTCountyName = BT_OPEN ("countynm.btr", 0, BT_WRITE, 0);
		while(fgetstring (str,128,Fid))
		{   
			CNKEY.State = ldread (str,2);
			CNKEY.County = ldread (&str[5],3);
			str[43]=0;
			pCounty = _fstrstr (&str[11]," County ");
			if (pCounty)
			{
				pCounty += 7;
				*pCounty = 0;
			}
			BT_PUT (hBTCountyName,(LPSTR)&CNKEY,&str[11]);
		}
		BT_CLOSE (hBTCountyName);
		GSSiClose (Fid);
		First = FALSE;
	}*/
	
	if (State < 1 || State > 56 || County < 1)
		goto Exit;
	hBTCountyName = BT_OPEN ("countynm.btr", 0, BT_READ, 0); 
	if (!hBTCountyName)
		goto Exit;
	CNKEY.State = State;
	CNKEY.County = County;
	st = BT_FIND (hBTCountyName,(LPSTR)&CNKEY,BT_FIRST,BT_EQ,CountyName);
	BT_CLOSE (hBTCountyName);
	if (!st)
		rtn = TRUE;
Exit:
{
#if ENABLETRACE
GSSiExitProg (1404);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}  

BOOL GetFIPSName (short State,long FIPS,LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (1405);
#endif
{
	char	SQL[128]; 
	HANDLE	hSQL=0; 
	BOOL	rtn=FALSE;
	
	if (State > 60)
	{
		char	DBName[]="FIPS=[%DL]ATTRIBUT\\CANMUN.GMD";

		sprintf (SQL,"CANMUNNUM==%ld",FIPS);
		if (!OpenDataFile (DBName,SQL,BT_READ,&hSQL))
			goto Exit;
		rtn = GetValFromOpenFiles ("CANMUNNAME",Name,100);  
	} 
	else
	{
		char	DBName[]="FIPS=[%DL]attribut\\fips.gmd";

		sprintf (SQL,"FIPS_st_cd==%i&&FIPS_place_cd=%ld",State,FIPS);
		if (!OpenDataFile (DBName,SQL,BT_READ,&hSQL))
			goto Exit;
		rtn = GetValFromOpenFiles ("Feat_name",Name,100);  
	}
    CloseDataFile (TRUE,&hSQL); 
Exit:
{
#if ENABLETRACE
GSSiExitProg (1405);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}  

BOOL WhereAt (LPSTR Args) 
#if ENABLETRACE
{GSSiEnterProg (1406);
#endif
{
	double	Scale; 
	HANDLE  hMem;
	LPSTR	Arg1, Arg2, ParLoc, CmdMess;  
	int		bmwidth,bmheight,i,j, iii=0;
	DPOINT	DPoint, InPoint, p1,p2;
	long	hNum;   
	MNMXCORD	Rect;
	BOOL	FromLimits, Immediate, WantAddressData=FALSE; 
	char	Outstring[4][128], str[512], PNFormat[16],ZIPC[16]; 
	char	InCity[68], BigCity[64],MedCity[64],SmlCity[64],IntName[128],FIPSName[100];
	long	InPop=0, NearSmlPop=0, NearMedPop=0, NearBigPop=0;
	CITIESKEY1	CitiesKey1;
	CITIESDATA1	CitiesData1;
	HANDLE	hBTCities1, hAddData=0; 
	long	idist;
	char	dir[16];
	double	NearDist, NearBigDist, NearAZ, NearBigAZ, InDist, d, DegreePerMile;
	double	ActualDist=0, BigDist, StreetAZ, BigAZ, MedDist, MedAZ, SmlDist, SmlAZ=0.87453165, MetersPerMile;
	clock_t	starttime, endtime; 
	short	SaveMaxPick; 
	LPVISLIST	SaveVis; 
	BOOL	HaveNearStreet, HaveOnStreet;
	PICKDATA	NearPick;
	short	CityPos1, CityPos2, IntPos;
	long	NearPickedStreets[7], TenthBitStreet; 
	char	OnName[128]="",TenthBitName[128]="";  
	long	FIPSCode;
	BOOL	rtn=FALSE;
    
	if (!(ParLoc = MatchLev (Args,',')))
		goto Exit;
    if (DoTime)
	    starttime=GetTickCount(); 
	if (GetGlobalCVal ("[%PNFORMAT]",PNFormat,NULL))
	{
		if (_fstrcmp (PNFormat,"0"))
			WantAddressData = TRUE;
		else
			_fmemset (PNFormat,0,16);
	}
	hMem = GSSiGlobAlloc ( 626,GMEM_MOVEABLE,2048*6);
	Arg1 = GlobalLock(hMem);
	Arg2 = Arg1 + 2048; 
			
	_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
	*ParLoc = '\0';
	_fstrcpy (Arg1,Args);
	ExpandText (Arg1);  
	ExpandText (Arg2); 
				
	SetViewport(*pCommandViewport);
	CurState = -1;
	DPoint.x = atof (Arg1);
	DPoint.y = atof (Arg2); 
	InPoint = DPoint;
	ConvertCoord (&DPoint,2,1);
			
	InCity[0]=0;  
	IntName[0]=0;
	InPop = 0; 
	NearBigPop = NearMedPop = NearSmlPop = 0;  
	CityPos1 = GetGlobalLVal ("[%CityPos1]")-1;
	CityPos2 = GetGlobalLVal ("[%CityPos2]")-1;
	IntPos = GetGlobalLVal ("[%IntPos]")-1;

	Outstring[0][0]=0;   
	Outstring[1][0]=0;
	Outstring[2][0]=0;
	Outstring[3][0]=0; 
	if (!WantStreets)
		goto NoStreets;
            
/*            p1 = p2 = DPoint;
    p1.y -= 0.5;
    p2.y += 0.5;
    d=GetBaseDist(&p1, &p2);
	UseUserPickAp =FALSE; 
	DegreePerMile = 1.0/((d*MFT)/5280);	*/
	SaveMaxPick = MaxPick;
	MetersPerMile = 5280*FTM;	
	MaxPick=1;  
	SaveVis = CurVis;
	UseUserPickAp =FALSE; 
	SystemPickAp = 0;
	PickLimit = 5 * MetersPerMile;	
	HaveNearStreet=FALSE;
	HaveOnStreet=TRUE;
	if (*PNFormat && PNFormat[9] != '1')
		goto PickAllStreets;
	if (*PNFormat)	
		LoadPickList ("piklists\\intstat2.pik"); 
	else
		LoadPickList ("piklists\\intstate.pik"); 
	CurVis = SaveVis;
	PickItems (CurView->hWnd,DPoint); 
	if (NumPicked)
	{
		ActualDist = GetBaseDist (&DPoint,&PickList[0].PickedPoint)*MFT/5280;
		if (ActualDist <= 1)
		{
			NearPick = PickList[0]; 
			_fmemmove (NearPickedStreets,&PickedStreets[0],28);
			if (PNFormat[9] == '1')
			{ 
			    CurState = PickedStreets[0][4];
				OpenGSStreetNames (BT_READ,1);
				TenthBitStreet = labs (PickedStreets[0][0]);
				*str = 0;				
				BT_FIND (hNames1,(LPSTR)&TenthBitStreet,BT_FIRST,BT_EQ,str); 
				sprintf (TenthBitName,"On %s",str);
				CloseGSStreetNames();
				goto PickAllStreets;
		    }
			goto HaveStreet; 
		}
		if (ActualDist <= 5)
		{
			HaveNearStreet = TRUE;
			NearPick = PickList[0]; 
			_fmemmove (NearPickedStreets,PickedStreets[0],28);
			if (PNFormat[9] == '1')
			{ 
			    CurState = PickedStreets[0][4];
				OpenGSStreetNames (BT_READ,1);
				TenthBitStreet = labs (PickedStreets[0][0]);
				*str = 0;				
				BT_FIND (hNames1,(LPSTR)&TenthBitStreet,BT_FIRST,BT_EQ,str); 
				sprintf (TenthBitName,"Near %s",str);
				CloseGSStreetNames();
		    }
		}
	}  
	PickLimit = 1 * MetersPerMile;	
//	SystemPickAp = (-1 * DegreePerMile)/2;	
	LoadPickList ("piklists\\state.pik"); 
	CurVis = SaveVis;
	PickItems (CurView->hWnd,DPoint); 
	if (NumPicked)
	{
		ActualDist = GetBaseDist (&DPoint,&PickList[0].PickedPoint)*MFT/5280;
		if (ActualDist <= 0.25)
		{
			if (PNFormat[9] == '1')
			{ 
			    CurState = PickedStreets[0][4];
				OpenGSStreetNames (BT_READ,1);
				TenthBitStreet = labs (PickedStreets[0][0]);
				*str = 0;				
				BT_FIND (hNames1,(LPSTR)&TenthBitStreet,BT_FIRST,BT_EQ,str); 
				sprintf (TenthBitName,"On %s",str);
				CloseGSStreetNames();
				goto PickAllStreets;
		    }
			goto HaveStreet; 
		}
		if (!HaveNearStreet && ActualDist <= 1)
		{
			HaveNearStreet = TRUE;  
			PickLimit = ActualDist * MetersPerMile;
			NearPick = PickList[0];
			_fmemmove (NearPickedStreets,PickedStreets[0],28);
			if (PNFormat[9] == '1')
			{ 
			    CurState = PickedStreets[0][4];
				OpenGSStreetNames (BT_READ,1);
				TenthBitStreet = labs (PickedStreets[0][0]);
				*str = 0;				
				BT_FIND (hNames1,(LPSTR)&TenthBitStreet,BT_FIRST,BT_EQ,str); 
				sprintf (TenthBitName,"Near %s",str);
				CloseGSStreetNames();
		    }
		}
	}
	  
PickAllStreets:
	LoadPickList ("piklists\\other.pik"); 
	CurVis = SaveVis;
	PickItems (CurView->hWnd,DPoint); 
	if (NumPicked)
	{
		ActualDist = GetBaseDist (&DPoint,&PickList[0].PickedPoint)*MFT/5280; 
		if ((*PNFormat && PNFormat[9] != '1') || ActualDist <= 0.125) 
		{
			NearPick = PickList[0];
			goto HaveStreet;
		} 
		if (!HaveNearStreet && ActualDist <= 1)
		{
			HaveNearStreet = TRUE;
			NearPick = PickList[0];
			_fmemmove (NearPickedStreets,PickedStreets[0],28);
		}
	}  
	HaveOnStreet=FALSE;
HaveStreet:
	CurVis = SaveVis;
	MaxPick = SaveMaxPick;   
	PickLimit = DBL_MAX;
	UseUserPickAp = TRUE; 
    if (DoTime)  
    {
	    PickTime=GetTickCount()-starttime;
	    starttime=GetTickCount();
	}
	if (!HaveOnStreet && HaveNearStreet)   
	{
		PickList[0]=NearPick;
		_fmemmove (PickedStreets[0],NearPickedStreets,28);
	}
	if (HaveOnStreet || HaveNearStreet)
	{   
		HANDLE	hBTTiger1;
		HFILE	FidTiger1;
		OFSTRUCT	OFStruct; 
		long	Offset, snum, IntLong, IntLat, IntTLID, ToLong, ToLat;
		long	OnRef, OnStreet, IntStreet;
		TIGER1_PEOPLENET	Tiger1;  
		char	Buff[64], TestName[128]; 
		int		st;
		double	IntDist, Dist;
		HANDLE	hIntersect;   
		DPOINT	IntPoint;
		DPOINT	BeginPoint, EndPoint;
		struct	{
					long Long, Lat, TLID;
				}	IntersectKey;
	    
	    _fmemmove (&CurStreetNumbers[0],PickedStreets[0],16);		
	    CurState = PickedStreets[0][4];
	    FromStreet = PickedStreets[0][5];
	    ToStreet = PickedStreets[0][6]; 
	    if (WantAddressData)
	    {   
    	    hPNAddData = GSSiGlobAlloc ( 627,GHND,1024); 
			ProcessPickedItem (0,FALSE);
			hAddData = DecodePNAddData (hPNAddData);
			GSSiGlobFree (&hPNAddData); 
		}
	    		
//				hBTTiger1 = BT_OPEN ("[STATE]\\tiger1.btr", 0, BT_READ, 0);
//				FidTiger1 = GSSiOpenFile ("[STATE]\\tiger1.dat",&OFStruct,OF_READ); 
		OpenGSStreetNames (BT_READ,1);				
/*		    	if (!BT_FIND (hBTTiger1,(LPSTR)&PickList[NumPicked-1].Refno,BT_FIRST,BT_EQ,(LPSTR)&Offset))
    	{   
    		GSSillseek (FidTiger1,Offset,0);
    		BigRead (FidTiger1,&Tiger1,sizeof(TIGER1_PEOPLENET));   
    		CitiesKey1.State = Tiger1.STATEL;
    		CitiesKey1.FIPS = Tiger1.FPLL; 
    		if (!CitiesKey1.State)
    		{   
	    		CitiesKey1.State = Tiger1.STATER;
	    		CitiesKey1.FIPS = Tiger1.FPLR; 
            }
    		hBTCities1 = BT_OPEN ("cities1.btr",0,BT_READ,0); 
    		if (!BT_FIND (hBTCities1,(LPSTR)&CitiesKey1,BT_FIRST,BT_EQ,(LPSTR)&CitiesData1))
    		{   
    			char	StateID[8];
		    			
    			GetStateID (CitiesKey1.State,StateID);
    			sprintf (InCity,"%s, %s",CitiesData1.Name,StateID);
    			InPop = CitiesData1.Pop;
    		}
    		else
    			InPop = 0;
    		BT_CLOSE (hBTCities1); 
*/                  
			OnStreet=0;
			for (i=0;i<4;i++)
				OnStreet = min (OnStreet,CurStreetNumbers[i]);
			if (!OnStreet)
				OnStreet=CurStreetNumbers[0];
			else
				OnStreet = labs (OnStreet); 
			*OnName = 0;
			if (OnStreet)
				BT_FIND (hNames1,(LPSTR)&OnStreet,BT_FIRST,BT_EQ,OnName); 
		    		
    		if (*OnName)
    		{ 
	    		if (HaveOnStreet) 
	    			sprintf (Outstring[0],"On %s",OnName); 
				else 
	    			sprintf (Outstring[0],"Near %s",OnName); 
				OneSpace (Outstring[0]);
	    	} 
	    	else
				Outstring[0][0]=0;
			    	
//		    	} 
		if (PNFormat[4] == '1' || GetGlobalBVal ("[%WANTINTNAME]"))
		{   
			if (PickList[0].PCT > 0.5)
				IntStreet = ToStreet;
			else
				IntStreet = FromStreet;
			if (!IntStreet)
				IntStreet=max (labs(FromStreet),labs(ToStreet)); 
			*IntName=0; 
			if (IntStreet)
				BT_FIND (hNames1,(LPSTR)&IntStreet,BT_FIRST,BT_EQ,IntName); 
		}
		CloseGSStreetNames();
    } 
	if (DoTime)
    	IntersectTime=GetTickCount()-starttime;  
NoStreets:			
	if (DoTime)    
    	starttime=GetTickCount();
	NearBigPop = GetNearCity (DPoint,3,BigCity,&BigDist,&BigAZ);
	NearMedPop = GetNearCity (DPoint,2,MedCity,&MedDist,&MedAZ);
	NearSmlPop = GetNearCity (DPoint,1,SmlCity,&SmlDist,&SmlAZ);
	if (BigDist < GetGlobalLVal ("[%LGE_CITY_DIST1]")) 
	{
	    AZtoDirection (BigAZ,dir);
	    if (!IsInCity (BigDist, NearBigPop)) 
	    	sprintf (Outstring[CityPos1],"%ld miles %s of %s",IDNINT(BigDist),dir,BigCity);
	    else
    		sprintf (Outstring[CityPos1],"In %s",BigCity);
    } 
    else if (MedDist < GetGlobalLVal ("[%MED_CITY_DIST1]"))
	{
	    AZtoDirection (MedAZ,dir); 
	    if (!IsInCity (MedDist, NearMedPop)) 
	    	sprintf (Outstring[CityPos1],"%ld miles %s of %s",IDNINT(MedDist),dir,MedCity);
	    else
    		sprintf (Outstring[CityPos1],"In %s",MedCity);
    } 
    else 
	{
	    AZtoDirection (BigAZ,dir);
    	sprintf (Outstring[CityPos1],"%ld miles %s of %s",IDNINT(BigDist),dir,BigCity);
    }
	if (SmlDist < GetGlobalLVal ("[%SML_CITY_DIST1]")) 
	{
	    AZtoDirection (SmlAZ,dir);
    	sprintf (Outstring[CityPos2],"In %s",SmlCity);
    } 
    else if (MedDist < GetGlobalLVal ("[%MED_CITY_DIST2]"))
	{
	    AZtoDirection (MedAZ,dir);
	    if (!IsInCity (MedDist, NearMedPop)) 
	    	sprintf (Outstring[CityPos2],"%ld miles %s of %s",IDNINT(MedDist),dir,MedCity);
	    else
    		sprintf (Outstring[CityPos2],"In %s",MedCity);
    } 
    else if (SmlDist < GetGlobalLVal ("[%SML_CITY_DIST2]"))
	{
	    AZtoDirection (SmlAZ,dir);
    	sprintf (Outstring[CityPos2],"%ld miles %s of %s",IDNINT(SmlDist),dir,SmlCity);
    }
    else if (MedDist < GetGlobalLVal ("[%MED_CITY_DIST3]"))
	{
	    AZtoDirection (MedAZ,dir);
    	sprintf (Outstring[CityPos2],"%ld miles %s of %s",IDNINT(MedDist),dir,MedCity);
    } 
    if (!_fstricmp (Outstring[CityPos1],Outstring[CityPos2]))
    	*Outstring[CityPos2]=0;
    if (!_fstrnicmp (Outstring[CityPos1],"In ",3) &&
    	!_fstrnicmp (Outstring[CityPos2],"In ",3))
    {
    	_fstrcpy (str,&Outstring[CityPos1][2]);
    	_fstrcpy (Outstring[CityPos1],str);
    }
    if (DoTime)
    	CloseTime=GetTickCount()-starttime;
    if (IntName[0])
    	sprintf (Outstring[IntPos],"Near intersection with %s",IntName);
	OneSpace (Outstring[1]);
	OneSpace (Outstring[2]);
	OneSpace (Outstring[3]);
	CmdMess = GlobalLock (hCmdMess);
    sprintf (CmdMess,"%s;%s;%s;%s",Outstring[0],Outstring[1],Outstring[2],
    										Outstring[3]);
	if (*PNFormat)
	{   
		char	BigDir[16],MedDir[16],SmlDir[16],StreetDir[16],CountyName[128]="",StateID[4]="";
		long	House=0,ZIP=0;
		short	County=0;
		
//						 pAddData->FromAddL, ToAddL, FromAddR, ToAddR, ZipL, ZipR, CountyL, CountyR
		if (hAddData)
		{
			LPPNADDRESSDATA	pAddData=(LPPNADDRESSDATA)GlobalLock (hAddData); 
			
			if (NearPick.OffDist < 0)
			{ 
				House = pAddData->FromAddL + (pAddData->ToAddL - pAddData->FromAddL) * NearPick.PCT;
				ZIP = pAddData->ZipL;
				if (!ZIP)
					ZIP = pAddData->ZipR;
				County = pAddData->CountyL;
				if (!County)
					County = pAddData->CountyR;
				FIPSCode = pAddData->MCDL;
			}
			else 
			{
				House = pAddData->FromAddR + (pAddData->ToAddR - pAddData->FromAddR) * NearPick.PCT;
				ZIP = pAddData->ZipR;
				if (!ZIP)
					ZIP = pAddData->ZipL;
				County = pAddData->CountyR;
				if (!County)
					County = pAddData->CountyL;
				FIPSCode = pAddData->MCDR;
			}
			GlobalUnlock (hAddData);
		}
		if (County && PNFormat[7] == '1')		
			GetCountyName (CurState,County,CountyName);
		if (CurState && PNFormat[8] == '1')		
			GetStateID (CurState, StateID); 
		StreetAZ = getazd (&NearPick.PickedPoint,&DPoint);
	    AZtoDirection2 (StreetAZ,StreetDir);
	    AZtoDirection2 (BigAZ,BigDir);
	    AZtoDirection2 (MedAZ,MedDir);
	    AZtoDirection2 (SmlAZ,SmlDir);
	    if (PNFormat[9] == '1' && !*TenthBitName)
	    	_fstrcpy (TenthBitName,OnName); 
	    *FIPSName = 0; 
	    if (PNFormat[10] == '1')
	    	GetFIPSName (CurState,FIPSCode,FIPSName);
        *ZIPC = 0;
        if (ZIP > 0)
        	ltoa (ZIP,ZIPC,10);
        else
        	CanZipNumToChar (ZIP,ZIPC);
		sprintf (CmdMess,"%s(%.3f%s);%s(%.1f%s%ld);%s(%.1f%s%ld);%s(%.1f%s%ld);%s;%ld;%s;%s;%s;%s;%s",
					 OnName,ActualDist,StreetDir,
					 BigCity,BigDist,BigDir,NearBigPop,
					 MedCity,MedDist,MedDir,NearMedPop,
					 SmlCity,SmlDist,SmlDir,NearSmlPop, 
					 IntName,
					 House,ZIPC,CountyName,StateID,TenthBitName,FIPSName);
	}
	GSSiGlobFree (&hAddData);
	GlobalUnlock (hCmdMess);
	GSSiGlobUlFree (&hMem);
	rtn = TRUE;
Exit:
{
#if ENABLETRACE
GSSiExitProg (1406);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}  
		
BOOL GetCityCoord (LPSTR Name,LPDPOINT pDPoint,LPMNMXCORD pMinMax)
#if ENABLETRACE
{GSSiEnterProg (1407);
#endif
{
	double	Dist; 
	HANDLE	hBTCities1, hBTCities3;
	int		pos=BT_FIRST;  
	CITIESKEY1	CitiesKey1; 
	CITIESDATA1	CitiesData1;
	CITIESKEY3	CitiesKey3; 
	char	Name2[80];  
	int		rtn, istate;  
	HFILE	FidTiger1; 
	TIGER1_PEOPLENET	Tiger1;
	OFSTRUCT	OFStruct;
	LPSTR	lpComma; 
	int		cond;

	hBTCities3 = BT_OPEN ("cities3.btr",0,BT_READ,0);
	if (!hBTCities3)
	{
		BTVARDESC	BTVar[2];   
		
		BTVar[0].BT_VARTYP=BT_CHAR;
		BTVar[0].BT_VARLEN=62;
		BTVar[0].BT_VAROFF=0;
		BTVar[1].BT_VARTYP=BT_INTEGER;
		BTVar[1].BT_VARLEN=2;
		BTVar[1].BT_VAROFF=62;
		BT_CREATE ("cities3.btr", sizeof(CITIESKEY1), FALSE, 2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
		hBTCities3 = BT_OPEN ("cities3.btr", 0, BT_WRITE, 0);
		hBTCities1 = BT_OPEN ("cities1.btr",0,BT_WRITE,0); 
		while(!BT_FIND (hBTCities1,(LPSTR)&CitiesKey1,pos,BT_ANY,(LPSTR)&CitiesData1))
		{   
			pos = BT_NEXT;
			_fstrncpy (CitiesKey3.Name,CitiesData1.Name,62); 
			_strupr (CitiesKey3.Name); 
			CitiesKey3.State = CitiesKey1.State;
			BT_PUT (hBTCities3,(LPSTR)&CitiesKey3,(LPSTR)&CitiesKey1);
		}
		for (istate=1;istate<=61;istate++)
		{   
			char	TName[128];
			
			sprintf (TName,"%2.2i\\tiger1.dat",istate);  
			SetWindowText (hWndMain,TName);
			FidTiger1 = GSSiOpenFile (TName,&OFStruct,OF_READ); 
			if (FidTiger1 == HFILE_ERROR)
				goto NextState;
			while (BigRead(FidTiger1,(HPSTR)&Tiger1,sizeof(TIGER1_PEOPLENET)) == sizeof(TIGER1_PEOPLENET))
			{
				CitiesKey1.State = Tiger1.STATEL;
				CitiesKey1.FIPS = Tiger1.FPLL;  
				if (!BT_FIND (hBTCities1,(LPSTR)&CitiesKey1,BT_FIRST,BT_EQ,(LPSTR)&CitiesData1))
				{   DPOINT	DPoint;
				
					DPoint.x = (double)Tiger1.FRLONG/1000000;
					DPoint.y = (double)Tiger1.FRLAT/1000000;
					CitiesData1.MinMax.xmn = min (CitiesData1.MinMax.xmn,DPoint.x);
					CitiesData1.MinMax.xmx = max (CitiesData1.MinMax.xmx,DPoint.x);
					CitiesData1.MinMax.ymn = min (CitiesData1.MinMax.ymn,DPoint.y);
					CitiesData1.MinMax.ymx = max (CitiesData1.MinMax.ymx,DPoint.y);
					DPoint.x = (double)Tiger1.TOLONG/1000000;
					DPoint.y = (double)Tiger1.TOLAT/1000000;
					CitiesData1.MinMax.xmn = min (CitiesData1.MinMax.xmn,DPoint.x);
					CitiesData1.MinMax.xmx = max (CitiesData1.MinMax.xmx,DPoint.x);
					CitiesData1.MinMax.ymn = min (CitiesData1.MinMax.ymn,DPoint.y);
					CitiesData1.MinMax.ymx = max (CitiesData1.MinMax.ymx,DPoint.y);
					BT_PUT (hBTCities1,(LPSTR)&CitiesKey1,(LPSTR)&CitiesData1);			
				}
			} 
			GSSiClose (FidTiger1);
NextState:; 
		}
		BT_CLOSE (hBTCities1);
	} 
	_fstrcpy (Name2,Name);
	lpComma = _fstrrchr (Name2,' ');
	istate=0;	 
	cond = BT_GE;
	if (lpComma)
	{
		FILE	*Fid; 
		char	StateStr[130];
	
		*lpComma++ = 0;
		Fid = fopen ("states.txt","r"); 
		if (Fid)
		{
			while (fgetss (StateStr,128,Fid))
			{
				if (!_fstrcmp (&StateStr[66],lpComma)) 
				{
					istate = atoi (StateStr);   
					cond = BT_EQ;
					fclose (Fid);
					goto GotState;
				}
			}
			fclose (Fid);
		}
		
	}
GotState:
	_fstrncpy (CitiesKey3.Name,Name2,62);  
	_fstrupr (CitiesKey3.Name);  
	CitiesKey3.State = istate;
	rtn=FALSE;
	
	if (!BT_FIND (hBTCities3,(LPSTR)&CitiesKey3,BT_FIRST,cond,(LPSTR)&CitiesKey1)) 
	{   
		if (!_fstricmp(CitiesKey3.Name,Name2))
		{
			hBTCities1 = BT_OPEN ("cities1.btr",0,BT_READ,0);   
			BT_FIND (hBTCities1,(LPSTR)&CitiesKey1,BT_FIRST,BT_EQ,(LPSTR)&CitiesData1);
			*pMinMax = CitiesData1.MinMax;
			*pDPoint = CitiesData1.LatLong; 
			if (pMinMax->xmn>1000000)
			{
				pMinMax->xmn = pDPoint->x - 0.15;
				pMinMax->xmx = pDPoint->x + 0.15;
				pMinMax->ymn = pDPoint->y - 0.15;
				pMinMax->ymx = pDPoint->y + 0.15; 
			}
			rtn=TRUE;
			BT_CLOSE (hBTCities1);
		}
	}
	BT_CLOSE (hBTCities3);   
	
{
#if ENABLETRACE
GSSiExitProg (1407);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}  
  
short GetStateNum (LPSTR ID)
#if ENABLETRACE
{GSSiEnterProg (1408);
#endif
{
	HFILE	Fid;
	char	StateStr[132];  
	int		istate=0;  
	OFSTRUCT	OFStruct;
	
	Fid = GSSiOpenFile ("[%INDIR]states.txt",&OFStruct,OF_READ); 
	if (Fid==HFILE_ERROR)
		goto Exit;
	while (fgetstring (StateStr,128,Fid))
	{
		istate = atoi (StateStr); 
		if (!_fstricmp (ID,&StateStr[66]))
		{
			GSSiClose (Fid);
			goto Exit;
		}
	}
	GSSiClose (Fid);
Exit:
{
#if ENABLETRACE
GSSiExitProg (1408);
#endif
    return istate;
}
#if ENABLETRACE
}
#endif
}  

BOOL GetStateID (short StateNum, LPSTR ID)
#if ENABLETRACE
{GSSiEnterProg (1409);
#endif
{
	HFILE	Fid;
	char	StateStr[132];  
	int		istate;
	static	BOOL	First=TRUE;
	BOOL	rtn=FALSE;
	
	*ID = 0;
	if (StateNum <1 || StateNum > 73)
		goto Exit;
	if (First)
	{   
		_fmemset (StateIDs,0,sizeof(StateIDs));
		Fid = GSSiOpenFile ("[%DL]states.txt",NULL,OF_READ); 
		if (Fid==HFILE_ERROR)
			goto Exit;
		while (fgetstring (StateStr,128,Fid))
		{
			istate = atoi (StateStr); 
			_fstrcpy (StateIDs[istate],&StateStr[66]);
		}
		GSSiClose (Fid); 
		First = FALSE;
	} 
	_fstrcpy (ID,StateIDs[StateNum]);
	rtn = TRUE;
Exit:
{
#if ENABLETRACE
GSSiExitProg (1409);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}  

void FixCaps (LPSTR str)
#if ENABLETRACE
{GSSiEnterProg (1410);
#endif
{
	BOOL	LastASpace=FALSE;
	
	_fstrlwr (str);
	*str++ = _toupper (*str);
	while (*str)
	{
		if (LastASpace)
			*str = _toupper (*str); 
		if (*str == ' ')
			LastASpace = TRUE;
		else
			LastASpace = FALSE; 
		str++;
	}
{
#if ENABLETRACE
GSSiExitProg (1410);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  

long GetPop (LPSTR UDI)
{
	char	str[66];   
	long	Pop=0;  
	LPSTR	lpSpace;
	
	_fstrcpy (str,UDI); 
	if ((lpSpace = _fstrrchr (str,'|')))
	{   
		*lpSpace = 0;
		lpSpace++; 
		Pop = atol (lpSpace); 
	} 
	return Pop;
}
	

BOOL IsInCity (double Dist, long Pop)
{ 
	double	InDist;
	
	InDist = pow ((double)Pop,0.5);
	InDist *= GetGlobalDVal("[%IN_CITY_DIST_FACTOR]"); 
	InDist += GetGlobalDVal2("[%MIN_CITY_RADIUS]",0);
	if (Dist <= InDist)
		return TRUE;
	else
		return FALSE;
}  

long CanZipCharToNum (LPSTR CZip)
{   
	long	nlong = 0;  
	
	_fmemmove (&nlong,CZip,3); 
	nlong = -nlong;  
	return nlong;
}

LPSTR CanZipNumToChar (long nlong,LPSTR CZip)
{   
	nlong = -nlong;
	_fmemmove (CZip,&nlong,3);
	CZip[3] = 0;
	return CZip; 
}

long GetNearCity (DPOINT DPoint,short Type,LPSTR InCity,LPDOUBLE MinDist, LPDOUBLE MinAZ)
#if ENABLETRACE
{GSSiEnterProg (1411);
#endif
{
	double	Dist; 
	VISLIST	SaveVis;  
	LPSTR	lpSpace; 
	short	SaveMaxPick, item; 
	long	Pop=0, BigestPop;
	char	PLTName[3][8]={"small","medium","large"};
	char	str[12]; 
	static	n=0;
	HANDLE	hVisList; 
	BOOL	PLSaved=TRUE;
	BOOL	SaveUUPA = UseUserPickAp;
    
   	UseUserPickAp =FALSE; 
	SystemPickAp = 0;

	if (CurView->pPickListManual)
 		SaveVis = *CurView->pPickListManual; 
 	else 
 	{
 		PLSaved=FALSE;
        hVisList=GSSiGlobAlloc ( 628,GHND,sizeof(VISLIST));
        CurVis = CurView->pPickListManual =(LPVISLIST) GlobalLock (hVisList);
		InitVis ();
        CurView->pPickListManual->hVisList = hVisList;   
    }
    
	SaveMaxPick = MaxPick;
	MaxPick = 1; 
	memset (CurView->pPickListManual->FileIsVisible,0,sizeof(CurView->pPickListManual->FileIsVisible));
	memset (CurView->pPickListManual->WantType,0,sizeof(CurView->pPickListManual->WantType));
	CurView->pPickListManual->WantType[1] = TRUE;
    if (CurState < 0)
    {   
    	n++;
	   	CurView->pPickListManual->FileIsVisible[10]=1;
		PickItems (CurView->hWnd,DPoint);  
	   	CurView->pPickListManual->FileIsVisible[10]=0; 
	   	if (PickList[0].Desc == 223)
    		CurState=0; 
	   	else if (PickList[0].Desc == 541)
    		CurState=73; 
    	else
    		CurState=60;
    } 
   	CurView->pPickListManual->FileIsVisible[GetGlobalLVal2 ("[%CITYFILEPOS]",9)]=1;
   	_fstrcpy (str,PLTName[Type-1]); 
   	if (CurState < 60)
   		_fstrcat (str,"U"); 
   	else if (CurState == 73)
   		_fstrcat (str,"M");
   	else
   		_fstrcat (str,"C");
   	SetGlobalValue("PICKFILE",str);
	PickItems (CurView->hWnd,DPoint);  
	MaxPick = SaveMaxPick;  
	item = 0; 
	if (PLSaved)
		*CurView->pPickListManual=SaveVis;  
	
	Pop = GetPop (PickList[item].UDI); 
	_fstrcpy (InCity,PickList[item].UDI); 
	if ((lpSpace = _fstrrchr (InCity,'|')))
	{   
		char	State[8];
		short	province;
		
		*lpSpace = 0;
		lpSpace++; 
		Pop = atol (lpSpace);
		if ((lpSpace = _fstrrchr (InCity,'|')))
		{   
			*lpSpace = 0;
			lpSpace++; 
			_fstrcpy (State,lpSpace);
			province = atoi (State);
			switch (province)
			{
				case 48:
					_fstrcpy (State," ALB");
					break; 
				case 59:
					_fstrcpy (State," BC");
					break; 
				case 60:
					_fstrcpy (State," YUK");
					break; 
				case 61:
					_fstrcpy (State," NWT");
					break; 
				case 47:
					_fstrcpy (State," SAS");
					break; 
				case 46:
					_fstrcpy (State," MAN");
					break; 
				case 35:
					_fstrcpy (State," ONT");
					break; 
				case 24:
					_fstrcpy (State," QUE");
					break; 
				case 10:
					_fstrcpy (State," NEW");
					break; 
				case 12:
					_fstrcpy (State," NOV");
					break; 
				case 13:
					_fstrcpy (State," NBR");
					break; 
				case 11:
					_fstrcpy (State," PEI");
					break; 
				default:  // USA cities have type following name - remove
					if ((lpSpace = _fstrrchr (InCity,' ')))
						*lpSpace = 0;
					break;
			}
			FixCaps (InCity);  
			_fstrcat (InCity," ");
			_fstrcat (InCity,State); 
		}
	}
	*MinDist = GetBaseDist (&DPoint,&PickList[item].BeginPoint)*MFT/5280;
	*MinAZ = getazd (&PickList[item].BeginPoint,&DPoint);
	UseUserPickAp = SaveUUPA;
{
#if ENABLETRACE
GSSiExitProg (1411);
#endif
    return Pop;
}
#if ENABLETRACE
}
#endif
}  
short GetIntersectStuff (long TLID, HANDLE hIntersect,LPTIGER1 Tiger1,LPSHORT Stuff)
#if ENABLETRACE
{GSSiEnterProg (1412);
#endif
{
    short TotLen=0;
    LPSHORT   pNum;
    long    IntLong, IntLat, snum; 
    short     st, i;
    struct  {
                long Long, Lat, TLID;
            }   IntersectKey;
    struct  {
                long    Snum;
                long    OPLong, OPLat;
            }   IntersectData;
    
    if (!hIntersect)
        goto Exit; 
    IntLong = ldread (Tiger1->FRLONG,10); 
    IntLat = ldread (Tiger1->FRLAT,9);  
    i=2;
    
    while (i--)
    {
        pNum = Stuff++;
        TotLen++;
        *pNum = 0; 
        IntersectKey.Long = IntLong; 
        IntersectKey.Lat = IntLat; 
        IntersectKey.TLID = 0;  
        st = BT_FIND (hIntersect,(LPSTR)&IntersectKey,BT_FIRST,BT_GT,(LPSTR)&IntersectData);
        while (!st)
        {
            if (IntersectKey.Long != IntLong || IntersectKey.Lat != IntLat)
                st = 1;
            else
            {   
                if (IntersectKey.TLID != TLID)
                {   
                    _fmemmove (Stuff,&IntersectKey.TLID,4);
                    Stuff+=2;
                    _fmemmove (Stuff,&IntersectData.Snum,4);
                    Stuff+=2; 
                    (*pNum)++;  
                    TotLen += 4;
                }
                st = BT_FIND (hIntersect,(LPSTR)&IntersectKey,BT_NEXT,BT_ANY,(LPSTR)&IntersectData);
            }
        }
        IntLong = ldread (Tiger1->TOLONG,10); 
        IntLat = ldread (Tiger1->TOLAT,9);  
    }  
Exit:
{
#if ENABLETRACE
GSSiExitProg (1412);
#endif
    return TotLen;
}
#if ENABLETRACE
}
#endif
}  

short PNAddLen (LPSTR InAdd, LPSTR *pOutAdd)
{
	short l=11, i;
	
	for (i=0;i<11;i++,InAdd++,l--) 
		if (*InAdd != ' ')
			break;
	if (l)
		_fmemmove (*pOutAdd,InAdd,l);
	(*pOutAdd) += l;
	return l;
}

short AddPNExtraData (LPTIGER1 Tiger1,LPSHORT pStuff)
{
	short	TotLen=13, TotAddLen; 
	LPADDRESSLENGTHS pAddLengths; 
	LPSTR	pAdds;
	
//	short i=sizeof(ADDRESSLENGTHS); 
	
	_fmemmove (pStuff,Tiger1->ZIPL,10);
	pStuff += 5;
	_fmemmove (pStuff,Tiger1->COUNTYL,6);
	pStuff += 3;
	_fmemmove (pStuff,Tiger1->FMCDL,10);
	pStuff += 5;
	pAddLengths = (LPADDRESSLENGTHS)pStuff++; 
	pAdds = (LPSTR)pStuff;
	pAddLengths->lFAddL = PNAddLen (Tiger1->FRADDL,&pAdds);
	pAddLengths->lTAddL = PNAddLen (Tiger1->TOADDL,&pAdds);
	pAddLengths->lFAddR = PNAddLen (Tiger1->FRADDR,&pAdds);
	pAddLengths->lTAddR = PNAddLen (Tiger1->TOADDR,&pAdds);
	TotAddLen = 2 + pAddLengths->lFAddL + pAddLengths->lTAddL + pAddLengths->lFAddR + pAddLengths->lTAddR; 
	TotAddLen += TotAddLen%2;
	TotLen += TotAddLen/2;
	return TotLen;
}

BOOL GetNextStreet (long TLID, long Snum, HANDLE hIntersect,LPLONG Long, LPLONG Lat, LPLONG NextSNum)
#if ENABLETRACE
{GSSiEnterProg (1413);
#endif
{
    short     NumLinks=0, MaxLinks=25;
    TIGER1_MN   Tiger1PN; 
    short TotLen=0;
    LPSHORT   pNum;
	BOOL	rtn=FALSE;
    long    IntLong, IntLat, snum, NextTLID, NextLong, NextLat; 
    short     st, i;
    struct  {
                long Long, Lat, TLID;
            }   IntersectKey;
    struct  {
                long    Snum;
                long    OPLong, OPLat;
            }   IntersectData;
        
    *NextSNum = 0;
    if (!hIntersect)
        goto Exit;
    IntLong = *Long; 
    IntLat = *Lat;  

NextLink:   
    IntersectKey.Long = IntLong; 
    IntersectKey.Lat = IntLat; 
    IntersectKey.TLID = 0;    
    NextTLID = 0;
    st = BT_FIND (hIntersect,(LPSTR)&IntersectKey,BT_FIRST,BT_GT,(LPSTR)&IntersectData);
    while (!st)
    {   
        snum = IntersectData.Snum;
        if (IntersectKey.Long != IntLong || IntersectKey.Lat != IntLat)
            st = 1;
        else
        {   
            if (IntersectKey.TLID != TLID)
            {   
                if (snum && (snum != Snum))
                {
                    *Long = IntersectKey.Long;
                    *Lat = IntersectKey.Lat;
                    *NextSNum = snum;
					rtn = TRUE;
					goto Exit;
                   
                } 
                else if (snum == Snum) 
                {
                    NextTLID = IntersectKey.TLID;
                    NextLong = IntersectData.OPLong;
                    NextLat = IntersectData.OPLat;
                }
            }
            st = BT_FIND (hIntersect,(LPSTR)&IntersectKey,BT_NEXT,BT_ANY,(LPSTR)&IntersectData);
        }
    } 
    if (!NextTLID || NumLinks > MaxLinks || !Snum)
	{
        rtn = FALSE; 
		goto Exit;
	}
    TLID = NextTLID;
    IntLong = NextLong;
    IntLat = NextLat;  
    NumLinks++;
    goto NextLink;
Exit:
	{
#if ENABLETRACE
GSSiExitProg (1413);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}  

HANDLE PNOpen (void)
{
	HANDLE DBHandle=0;
    LPGWDHEADER lpGWDHead;
    LPGWFLDINFO lpFieldInfo;
    OFSTRUCT    OFStruct;

	return DBHandle;
}

void DisplayPNData (long Refno,HANDLE hIntData,HANDLE hPNAddData)
#if ENABLETRACE
{GSSiEnterProg (1414);
#endif
{
	LPADDRESSLENGTHS pAddLengths; 
	LPSTR	AddData=GlobalLock (hPNAddData), pAdd;
	char	str[256], adds[128], txt[32]; 
	LPPNADDRESSDATA		pData;   
	HANDLE	handle;
	
	handle = DecodePNAddData (hPNAddData);
	pData =(LPPNADDRESSDATA)GlobalLock (handle);                 
	pAddLengths = (LPADDRESSLENGTHS)(AddData+16);
	pAdd = (LPSTR)pAddLengths;
	pAdd+=2; 
	_fstrcpy (adds,"Adds=");
	if (pAddLengths->lFAddL)
		strncpy0 (_fstrchr(adds,0),pAdd,pAddLengths->lFAddL); 
	pAdd += pAddLengths->lFAddL;
	_fstrcat (adds,",");  
	if (pAddLengths->lTAddL)
		strncpy0 (_fstrchr(adds,0),pAdd,pAddLengths->lTAddL);
	pAdd += pAddLengths->lTAddL;
	_fstrcat (adds,",");  
	if (pAddLengths->lFAddR)
		strncpy0 (_fstrchr(adds,0),pAdd,pAddLengths->lFAddR);
	pAdd += pAddLengths->lFAddR;
	_fstrcat (adds,",");  
	if (pAddLengths->lTAddR)
		strncpy0 (_fstrchr(adds,0),pAdd,pAddLengths->lTAddR);
	pAdd += pAddLengths->lTAddR;
	_fstrcat (adds,",");  
//	sprintf (str,"TLID=%ld,ZipLandR=%.5s - %.5s, CountyLandR=%.3s - %.3s, %s",Refno,&AddData[0],&AddData[5],&AddData[10],&AddData[13],adds);  
	if (CurState > 60)
		sprintf (str,"TLID=%ld,ZipLandR=%s - %s, MunicLandR=%ld - %ld, AddressLandR=(%ld - %ld)-(%ld - %ld)",Refno,
							CanZipNumToChar(pData->ZipL,txt),CanZipNumToChar(pData->ZipR,txt),pData->MCDL,pData->MCDR,pData->FromAddL,pData->ToAddL,pData->FromAddR,pData->ToAddR);
	else	
		sprintf (str,"TLID=%ld,ZipLandR=%ld - %ld, MunicLandR=%ld - %ld, AddressLandR=(%ld - %ld)-(%ld - %ld)",Refno,pData->ZipL,pData->ZipR,pData->MCDL,pData->MCDR,pData->FromAddL,pData->ToAddL,pData->FromAddR,pData->ToAddR);
	GlobalUnlock (hPNAddData);  
	GSSiGlobUlFree (&handle);
	MessageBox (GetFocus(),str,"",MB_OK);
{
#if ENABLETRACE
GSSiExitProg (1414);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  

BOOL GetPNData (HANDLE hDB,long Refno)
{
	LPGWDHEADER lpGWDHead;
	LPADDRESSLENGTHS pAddLengths; 
	LPSTR	AddData, pAdd, pSName;
	char	str[256], adds[128], txt[32]; 
	LPPNADDRESSDATA		pData;   
	HANDLE	handle;
	LPPNSTREETDATA pnData;
	int i, SNum;
	BOOL HaveName = FALSE;
	
	if (!hDB || !hIntData || !hPNAddData)
		return FALSE;

	lpGWDHead = GlobalLock (hDB);
	pnData = (LPPNSTREETDATA)&lpGWDHead->GWDData;
	memset (pnData,0,sizeof (PNSTREETDATA));

	pSName = pnData->STREETNAME;
    for (i=0;i<4;i++,pSName+=sizeof(pnData->STREETNAME))
        if ((SNum=labs(CurStreetNumbers[i])))
            if (GetTrueStreetName (SNum, str, CurState,0))
            {
				if (stricmp (str,"NONE"))
				{
					strncpy (pSName,str,sizeof(pnData->STREETNAME));
					HaveName = TRUE;
				}
            }
    if (!HaveName)
        strncpy (pnData->STREETNAME,"Unnamed Street",sizeof(pnData->STREETNAME));
	GetTrueStreetName (FromStreet, pnData->FROMSTREET, CurState,0);
	GetTrueStreetName (ToStreet, pnData->TOSTREET, CurState,0);
        
    CloseGSStreetNames();
	CloseStreetNameTable();
 

	AddData = GlobalLock (hPNAddData);
	handle = DecodePNAddData (hPNAddData);
	pData =(LPPNADDRESSDATA)GlobalLock (handle); 
	pnData->PNREF = Refno;
	pnData->FROMADDRESSL = pData->FromAddL;
	pnData->FROMADDRESSR = pData->FromAddR;
	pnData->TOADDRESSL = pData->ToAddL;
	pnData->TOADDRESSR = pData->ToAddR;
	pnData->ZIPL = pData->ZipL;
	pnData->ZIPR = pData->ZipR;
	//GetMunicName (pData->MCDL,pnData->CITYL,str);
	//GetMunicName (pData->MCDR,pnData->CITYR,str);
	/*pAddLengths = (LPADDRESSLENGTHS)(AddData+16);
	pAdd = (LPSTR)pAddLengths;
	pAdd+=2; 
	_fstrcpy (adds,"Adds=");
	if (pAddLengths->lFAddL)
		strncpy0 (_fstrchr(adds,0),pAdd,pAddLengths->lFAddL); 
	pAdd += pAddLengths->lFAddL;
	_fstrcat (adds,",");  
	if (pAddLengths->lTAddL)
		strncpy0 (_fstrchr(adds,0),pAdd,pAddLengths->lTAddL);
	pAdd += pAddLengths->lTAddL;
	_fstrcat (adds,",");  
	if (pAddLengths->lFAddR)
		strncpy0 (_fstrchr(adds,0),pAdd,pAddLengths->lFAddR);
	pAdd += pAddLengths->lFAddR;
	_fstrcat (adds,",");  
	if (pAddLengths->lTAddR)
		strncpy0 (_fstrchr(adds,0),pAdd,pAddLengths->lTAddR);
	pAdd += pAddLengths->lTAddR;
	_fstrcat (adds,",");*/  
//	sprintf (str,"TLID=%ld,ZipLandR=%.5s - %.5s, CountyLandR=%.3s - %.3s, %s",Refno,&AddData[0],&AddData[5],&AddData[10],&AddData[13],adds);  
/*	if (CurState > 60)
		sprintf (str,"TLID=%ld,ZipLandR=%s - %s, MunicLandR=%ld - %ld, AddressLandR=(%ld - %ld)-(%ld - %ld)",Refno,
							CanZipNumToChar(pData->ZipL,txt),CanZipNumToChar(pData->ZipR,txt),pData->MCDL,pData->MCDR,pData->FromAddL,pData->ToAddL,pData->FromAddR,pData->ToAddR);
	else	
		sprintf (str,"TLID=%ld,ZipLandR=%ld - %ld, MunicLandR=%ld - %ld, AddressLandR=(%ld - %ld)-(%ld - %ld)",Refno,pData->ZipL,pData->ZipR,pData->MCDL,pData->MCDR,pData->FromAddL,pData->ToAddL,pData->FromAddR,pData->ToAddR);
*/
	GlobalUnlock (hPNAddData);  
	GSSiGlobUlFree (&handle);
	GlobalUnlock (hDB);
    return TRUE;
}  

HANDLE DecodePNAddData (HANDLE hPNAddData)
#if ENABLETRACE
{GSSiEnterProg (1415);
#endif
{
	LPSTR	AddData=GlobalLock (hPNAddData), pAdd;
	LPADDRESSLENGTHS	pAddLengths;  
	HANDLE	handle=GSSiGlobAlloc ( 629,GMEM_MOVEABLE,sizeof(PNADDRESSDATA));
	LPPNADDRESSDATA		pData=(LPPNADDRESSDATA)GlobalLock (handle);
	
	pAddLengths = (LPADDRESSLENGTHS)(AddData+26);
	pAdd = (LPSTR)pAddLengths;
	pAdd+=2;
	if (CurState > 60)
	{ 
		pData->ZipL = CanZipCharToNum (&AddData[0]);
		pData->ZipR = CanZipCharToNum (&AddData[5]); 
	} 
	else
	{ 
		pData->ZipL = ldread (&AddData[0],5);
		pData->ZipR = ldread (&AddData[5],5); 
	}
	pData->CountyL = ldread (&AddData[10],3);
	pData->CountyR = ldread (&AddData[13],3);
	pData->MCDL = ldread (&AddData[16],5);
	pData->MCDR = ldread (&AddData[21],5);
	pData->FromAddL = ldread (pAdd,pAddLengths->lFAddL); 
	pAdd += pAddLengths->lFAddL;
	pData->ToAddL = ldread (pAdd,pAddLengths->lTAddL); 
	pAdd += pAddLengths->lTAddL;
	pData->FromAddR = ldread (pAdd,pAddLengths->lFAddR); 
	pAdd += pAddLengths->lFAddR;
	pData->ToAddR = ldread (pAdd,pAddLengths->lTAddR); 
	GlobalUnlock (hPNAddData);
	GlobalUnlock (handle);
{
#if ENABLETRACE
GSSiExitProg (1415);
#endif
    return handle;
}
#if ENABLETRACE
}
#endif
}  

/*BOOL AddTransCanadaDesignatonToStreetName (short State)
{   
	short	pos=BT_FIRST, len, i,ii;  
	long	Refno, NewStreetNum[4],SNum; 
	HIGHLIGHTDATA	HighlightData; 
	char	Name[128], NewName[128];
	BOOL	Changed;
	float	SizeFactor,TextFactor; 
	BOOL	Prime;  
	long	Tot=BT_NUM_IN_INDEX (hHighlight), Done=0;

	OpenShields ();
	CreateStatusWindow (CurView->hWnd,1,NULL); 
	while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))
	{
		pos = BT_NEXT;
		PickList[0]=HighlightData.PD;
		ProcessPickedItem (0,FALSE);        		
		CurState = State; 
		Changed = FALSE;
		for (i=0;i<3;i++) 
		{    
			if (CurStreetNumbers[i] < 0)
				Prime = TRUE;
			else
				Prime = FALSE;
			SNum = labs (CurStreetNumbers[i]); 
			NewStreetNum[i] = CurStreetNumbers[i]; 
			if (SNum)
			{
				GetTrueStreetName (SNum, Name, CurState); 
	            CloseGSStreetNames();
								
				if (ShieldType (Name,&SizeFactor,&TextFactor))
				{ 
					if (!_fstrnicmp (Name,"TC-",3))
						NewStreetNum[i] = CurStreetNumbers[i]; 
					else  
					{
						_fstrcpy (NewName,"TC-");
						_fstrcat (NewName,Name);
						NewStreetNum[i] = GetGSStreetNum (NewName);  
						Changed = TRUE;
					} 
					if (Prime)
						NewStreetNum[i] *= -1;	
				}
				else
					ii=1;
			}
		}
		if (Changed)
			UpdateEmbeddedStreetNums (0,NewStreetNum);
		StatusWindowUpdate (NULL,NULL,Tot,Done++); 
	} 
	DestroyStatusWindow(0);  
	CloseShields ();
	return TRUE;
}  */



BOOL ChangeImageColors (LPSTR ImageFile,BOOL UseSavedPalette)
#if ENABLETRACE
{GSSiEnterProg (1416);
#endif
{
	HDIB32 hDib;
	RGBQUAD	PaletteRGB[256];  
	USHORT	i,j; 
	BOOL	rtn=FALSE;   
	long	NumColors; 

	
	AddBMPToCache32 (NULL,0);
	hDib = BMPHandleFromEXT (ImageFile); 
	if (!hDib)
		goto Exit;
	if (GetDibPalette (hDib,&NumColors,PaletteRGB))
	{   
		if (!UseSavedPalette)
			HaveSavePalette = FALSE;
		if (!HaveSavePalette)
		{
			NumSavePaletteColors = NumColors;
			HaveSavePalette = TRUE;
			for (i=0;i<NumColors;i++)
				SavePalette[i] = PaletteRGB[i];
		} 
		else
			for (i=0;i<NumColors;i++)
				PaletteRGB[i] = SavePalette[i];
		for (j=0;j<NumColorChanges;j++) 
		{
	        double	HT = (double)ColorChangesTo[j].rgbRed/100;

			for (i=0;i<NumColors;i++) 
			{
				if (ColorChangesFrom[j].rgbReserved)
				{
					PaletteRGB[i].rgbRed = PaletteRGB[i].rgbRed + (255-PaletteRGB[i].rgbRed)*HT;
					PaletteRGB[i].rgbGreen = PaletteRGB[i].rgbGreen + (255-PaletteRGB[i].rgbGreen)*HT;
					PaletteRGB[i].rgbBlue = PaletteRGB[i].rgbBlue + (255-PaletteRGB[i].rgbBlue)*HT;
				}
				else
					if (RGBQUADDist (ColorChangesFrom[j],PaletteRGB[i]) <= ChangeDist)
						PaletteRGB[i] = ColorChangesTo[j];
			}
		}	 
		if (SetDibPalette (hDib,NumColors,PaletteRGB))
			rtn = SaveDIB32 (hDib,ImageFile,0,-1);
	}
	DestroyDIB32(hDib,TRUE);
	rtn = TRUE;
Exit:
{
#if ENABLETRACE
GSSiExitProg (1416);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}  

BOOL LoadColorChanges (void) 
#if ENABLETRACE
{GSSiEnterProg (1417);
#endif
{
	char	txt[130]; 
	BOOL	rtn=FALSE;
    HFILE Fid=GSSiOpenFile ("[%DL]colorchanges.txt",NULL,OF_READ);

   	 NumColorChanges = 0;
     if (Fid != HFILE_ERROR)
     { 
     	while (fgetstring (txt,128,Fid))
     	{
     		LPSTR pTab = _fstrchr (txt,'\t');
     		if (pTab)
     		{
     			*pTab++ = 0;  
     			if (!_fstricmp (txt,"-1"))
     				ColorChangesFrom[NumColorChanges].rgbReserved = 1;
     			else
     				ColorChangesFrom[NumColorChanges] = RGBQUADFromCOLORREF ((COLORREF)atol (txt));
     			ColorChangesTo[NumColorChanges++] = RGBQUADFromCOLORREF ((COLORREF)atol (pTab));
     		}
     	}
     	GSSiClose (Fid);
     	rtn = TRUE;
     } 
{
#if ENABLETRACE
GSSiExitProg (1417);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}  

BOOL SaveColorChanges (HWND hWnd) 
#if ENABLETRACE
{GSSiEnterProg (1418);
#endif
{
	char	txt[128];
	UINT	i;
	HFILE	Fid = GSSiOpenFile ("[%DL]colorchanges.txt",NULL,OF_CREATE);

	if (Fid == HFILE_ERROR)
		MessageBox (hWnd,"Cannot save",NULL,MB_ICONEXCLAMATION);
	else 
	{
		for (i=0;i<NumColorChanges;i++)
		{   
			if (ColorChangesFrom[i].rgbReserved)
				sprintf (txt,"-1\t%ld",COLORREFFromRGBQUAD(ColorChangesTo[i]));  
			else
				sprintf (txt,"%ld\t%ld",COLORREFFromRGBQUAD(ColorChangesFrom[i]),COLORREFFromRGBQUAD(ColorChangesTo[i]));
			fputstring (txt,Fid);
		}
		GSSiClose (Fid);
	} 
{
#if ENABLETRACE
GSSiExitProg (1418);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}  

int	IntersectLineWithBounds (HPDPOINT Line,LPMNMXCORD Bounds,HPDPOINT IntPoints)
#if ENABLETRACE
{GSSiEnterProg (1419);
#endif
{
	int	nint=0, IRC, i,j,nintout=0;
	DPOINT	IntPt[4], Corners[4];
	double	LineAZ = getazd (&Line[0],&Line[1]), BoundsAZ[4];
	double	LineLen = ldistp (Line[0],Line[1]);

//	return 0;
	BoundsToPoints (Bounds,Corners,BoundsAZ);

	for (i=0;i<4;i++)
	{
		IRC = LIN_SEC (Line[0].x,Line[0].y,LineAZ,Corners[i].x,Corners[i].y, BoundsAZ[i],
	    			&IntPt[nint].x, &IntPt[nint].y);
		if (!IRC)
		{
			if (ldistp (IntPt[nint],Line[0]) < LineLen &&
				ldistp (IntPt[nint],Line[1]) < LineLen)
				
			switch (i)
			{

			case 0:
				if (IntPt[nint].y >= Corners[i].y && IntPt[nint].y <= Corners[i+1].y)
					nint++;
				break;
			case 1:
				if (IntPt[nint].x >= Corners[i].x && IntPt[nint].x <= Corners[i+1].x)
					nint++;
				break;
			case 2:
				if (IntPt[nint].y >= Corners[i+1].y && IntPt[nint].y <= Corners[i].y)
					nint++;
				break;
			case 3:
				if (IntPt[nint].x >= Corners[0].x && IntPt[nint].x <= Corners[3].x)
					nint++;
				break;
			}
		}
	}
	
	if (nint)
		IntPoints[nintout++] = IntPt[0];
	for (i=1;i<nint;i++)
	{
		for (j=0;j<i;j++)
		{
			if (SameDPoint (&IntPt[i],&IntPoints[j]))
				goto Next;
		}
		IntPoints[nintout++] = IntPt[j]; //IntPoints[1]
Next:;
	}
	if (nintout > 1)
		i=1;
{
#if ENABLETRACE
GSSiExitProg (1419);
#endif
    return nintout;
}
#if ENABLETRACE
}
#endif
}  

int GetStateGridOffset_old (int irow,int icol,int nrow,int ncol,LPINT pOffsetArray)
{
	int loc = irow * ncol + icol;
	int	RowBeg;
	LPINT	pRun, pState;


	if (irow < 0 || icol < 0 || irow >= nrow || icol >= ncol)
		return -99;
	RowBeg = pOffsetArray[irow];
	pRun   = &pOffsetArray[RowBeg];
	pState = &pOffsetArray[RowBeg+1]; //pOffsetArray[RowBeg+2]
//	icol  -= *pRun;
	while (icol+1 > *pRun)
	{
		icol   -= *pRun;
		pRun   += 2;
		pState += 2;
	}
	return *pState;
}

UINT getdist16 (POINT16 p1,POINT16 p2)
#if ENABLETRACE
{GSSiEnterProg (1420);
#endif
{
	UINT dx = abs(p1.x - p2.x);
	UINT dy = abs(p1.y - p2.y);
	UINT d = dx * dx + dy * dy;
 
{
#if ENABLETRACE
GSSiExitProg (1420);
#endif
    return d;
}
#if ENABLETRACE
}
#endif
}  

UINT getdist16c (POINT16 p1,POINT16 p2,POINT16 p3)
#if ENABLETRACE
{GSSiEnterProg (1421);
#endif
{

	int	midx = (p2.x + p3.x)/2;
	int	midy = (p2.y + p3.y)/2;
	UINT dx = abs(p1.x - midx);
	UINT dy = abs(p1.y - midy);
	UINT d = dx * dx + dy * dy;

{
#if ENABLETRACE
GSSiExitProg (1421);
#endif
    return d;
}
#if ENABLETRACE
}
#endif
}  

int	GetPNCellArrayValue_old (LPBYTE CompressedCell,int subrow,int subcol)
{
	int	Type,irow=0,icol, nRowsInType;
	int	CurRowInType, nRepeats, StateIndex, RowIndex, FromRow;
	BYTE	States[2];
	BYTE	CellArrayRow[256];

	subrow++;
	while (irow < subrow)
	{
		Type = *CompressedCell++;
		nRowsInType = *CompressedCell++ + 1;
		CurRowInType = 0;
		if (Type == 1)
		{
			States[0] = *CompressedCell++;
			States[1] = *CompressedCell++;
		}
		while (CurRowInType < nRowsInType && irow < subrow)
		{
			nRepeats = *CompressedCell++ + 1;
			ExpandRow (Type,CellArrayRow,&CompressedCell,States);
			FromRow = irow++;
			CurRowInType++;
			nRepeats--;
			while (nRepeats--)
			{
				irow++;
				CurRowInType++;
			}
		}
	}
	return CellArrayRow[subcol];
}

void	ExpandPNCellArray (LPBYTE CompressedCell)
#if ENABLETRACE
{GSSiEnterProg (1422);
#endif
{
	int	Type,irow=0,icol, nRowsInType;
	int	CurRowInType, nRepeats, StateIndex, RowIndex, FromRow;
	BYTE	States[2];
//	LPBYTE	pCurRow, pCurLen, pCurRepeat, pRowsInType, pCurStateList=States;
//	BYTE	NextRow[1024];

	while (irow < 256)
	{
		Type = *CompressedCell++;
		nRowsInType = *CompressedCell++ + 1;
		CurRowInType = 0;
		if (Type == 1)
		{
			States[0] = *CompressedCell++;
			States[1] = *CompressedCell++;
		}
		while (CurRowInType < nRowsInType)
		{
			nRepeats = *CompressedCell++ + 1;
			ExpandRow (Type,CellArray[irow],&CompressedCell,States);
			FromRow = irow++;
			CurRowInType++;
			nRepeats--;
			while (nRepeats--)
			{
				memcpy (CellArray[irow++],CellArray[FromRow],256);
				CurRowInType++;
			}
		}
	}
{
#if ENABLETRACE
GSSiExitProg (1422);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  

/*GetPNAreaID (DPOINT Pt,LPSTR DataFile,LPSTR GridFile)
{
	HANDLE	hData;
	LPBYTE	pData;
	int ld = GSSiLength (DataFile), lg = GSSiLength (GridFile);
	int	rtn;
	HFILE	Fid;
	LPSTATEGRIDHEADER	pHead;

	if (ld == 0 || lg == 0)
		return -1;

	hData = GSSiGlobAlloc (0,GMEM_MOVEABLE,ld+lg);
	pData = GlobalLock (hData);
	pHead = (LPSTATEGRIDHEADER)pData;
	Fid = GSSiOpenFile (DataFile,0,OF_READ);
	BigRead (Fid,pData,ld);
	GSSiClose (Fid);
	pHead->GridOffset = ld;
	Fid = GSSiOpenFile (GridFile,0,OF_READ);
	BigRead (Fid,&pData[ld],lg);
	GSSiClose (Fid);
	rtn = GetAreaID (Pt.y,Pt.x,pData);
	GSSiGlobUlFree (&hData);
	return rtn;
}*/
int GetPNAreaID (DPOINT Pt,LPSTR DataFile)
#if ENABLETRACE
{GSSiEnterProg (1423);
#endif
{

	HANDLE	hData;
	LPBYTE	pData;
	int ld = GSSiLength (DataFile);
	int	rtn=-1;
	HFILE	Fid;
	LPSTATEGRIDHEADER	pHead;

	if (ld == 0)
		goto Exit;

	hData = GSSiGlobAlloc (0,GMEM_MOVEABLE,ld);
	pData = GlobalLock (hData);
	pHead = (LPSTATEGRIDHEADER)pData;
	Fid = GSSiOpenFile (DataFile,0,OF_READ);
	BigRead (Fid,pData,ld);
	GSSiClose (Fid);
	rtn = GetAreaID (Pt.y,Pt.x);
	GSSiGlobUlFree (&hData);
Exit:
{
#if ENABLETRACE
GSSiExitProg (1423);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}  
BOOL LoadPNetStateGrid (LPSTR DataFile,LPSTR GridFile,LPSTR OutFile)
#if ENABLETRACE
{GSSiEnterProg (1424);
#endif
{

	HANDLE	hData;
	LPBYTE	pData;
	HANDLE	hGrid, hGridCmp;
	LPINT	pGrid, pGridCmp, pGridRowOffsets;
	LPINT	pCurGrid, pCurRun;
	int ld = GSSiLength (DataFile), lg = GSSiLength (GridFile);
	int	lGridCmp;
	int	irow, icol,ii, nExtraBytes;
	HFILE	Fid;
	DWORD	Bytes=0;
	STATEGRIDHEADER	Head;
	LPSTATEGRIDHEADER	pHead;
	BOOL	rtn=-1;

	if (ld == 0 || lg == 0)
		goto Exit;

	Fid = GSSiOpenFile (DataFile,0,OF_READ);
	BigRead (Fid,&Head,sizeof(Head));
	GSSiClose (Fid);
	hGrid = GSSiGlobAlloc (0,GMEM_MOVEABLE,lg);
	pGrid = GlobalLock (hGrid);
	Fid = GSSiOpenFile (GridFile,0,OF_READ);
	BigRead (Fid,pGrid,lg);
	GSSiClose (Fid);
	hGridCmp = GSSiGlobAlloc (0,GMEM_MOVEABLE,lg);
	pGridCmp = GlobalLock (hGridCmp);
	pGridRowOffsets = pGridCmp;
	lGridCmp = Head.nrow;
	for (irow = 0;irow < Head.nrow; irow++)
	{
		int loc = irow * Head.ncol;

		if (irow == 105)
			ii=1;
		pGridRowOffsets[irow] = lGridCmp;
		pCurRun = &pGridCmp[lGridCmp++];
		pCurGrid = &pGridCmp[lGridCmp++];
		*pCurRun = 0;
		*pCurGrid = pGrid[loc];
		for (icol = 0;icol < Head.ncol;icol++,loc++)
		{
			if (pGrid[loc] == *pCurGrid)
				(*pCurRun)++;
			else
			{
				pCurRun = &pGridCmp[lGridCmp++];
				pCurGrid = &pGridCmp[lGridCmp++];
				*pCurRun = 1;
				*pCurGrid = pGrid[loc];
			}
		}
	}

	hData = GSSiGlobAlloc (0,GMEM_MOVEABLE,ld);
	pData = GlobalLock (hData);
	pHead = (LPSTATEGRIDHEADER)pData;
	Fid = GSSiOpenFile (DataFile,0,OF_READ);
	BigRead (Fid,pData,ld);
	GSSiClose (Fid);
	nExtraBytes = 4 - (ld % 4);
	pHead->GridOffset = ld+nExtraBytes;
	Fid = GSSiOpenFile (OutFile,0,OF_CREATE); 
	BigWrite (Fid,pData,ld,-1);
	if (nExtraBytes)
		BigWrite (Fid,&Bytes,nExtraBytes,-1);
	BigWrite (Fid,pGridRowOffsets,lGridCmp*sizeof(int),-1);
	GSSiClose (Fid);
	GSSiGlobUlFree (&hData);
	GSSiGlobUlFree (&hGrid);
	GSSiGlobUlFree (&hGridCmp);
	rtn = TRUE;
Exit:
{
#if ENABLETRACE
GSSiExitProg (1424);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}  


void DisplayPNStateGrid_old (LPSTR File,LPSTR FileOff)
{
	HFILE		Fid=GSSiOpenFile (File,0,OF_READ);
	HFILE		FidOff=GSSiOpenFile (FileOff,0,OF_READ);
	MNMXCORD	Bounds;
	DPOINT		CellArea[5], MidPoint;
	int			irow, icol, ifac, loff, i,ii;
	short		NumLines, NumPoints, LeftRight;
	HANDLE		hPoints, hPoints16, hOffsets;
	HPDPOINT	Points;
	LPPOINTS	Points16;
	HPLONG		Offsets;
	double	left;
	double	top;
	double	right;
	double	bottom;
	double	fac;

	double	cellw;
	double	cellh;

	if (Fid == HFILE_ERROR)
		return;

	SaveDC (CurView->hDC);
	BigRead (Fid,&Header,sizeof(Header));
	GSSiClose (FidOff);
	ifac = Header.ifac;
	left = ((double)Header.left) / ifac;
	bottom = ((double)Header.bottom) / ifac;
	cellw = ((double)Header.cellw) / ifac;
	cellh = ((double)Header.cellh) / ifac;
	for (irow = 0;irow < Header.nrow-1; irow++)
	{
		for (icol = 0;icol < Header.ncol; icol++)
		{
			Bounds.xmn = left + cellw * icol;
			Bounds.xmx = left + cellw * (icol+1);
			Bounds.ymn = bottom + cellh * irow;
			Bounds.ymx = bottom + cellh * (irow+1);
			BoundsToPoints (&Bounds,CellArea,0);
			MidPoint = MinMaxMidPointD (&Bounds);
			if (DPointInBounds (&MidPoint,&CurView->WBounds))
				ii=1;
			SelectObject (CurView->hDC,GetStockObject(NULL_BRUSH)); 
			SelectObject (CurView->hDC,GetStockObject(BLACK_PEN)); 
			GWPolygonD (CurView->hDC, CellArea,4, 1, 0,0,FALSE,TRUE,0); 
		}
	}
	RestoreDC (CurView->hDC,-1);
	return;
}

void DisplayPNStateGrid (LPSTR File)
#if ENABLETRACE
{GSSiEnterProg (1425);
#endif
{
	HFILE		Fid=GSSiOpenFile (File,0,OF_READ);
	MNMXCORD	Bounds;
	DPOINT		CellArea[5], MidPoint;
	int			irow, icol, ifac, loff, i,ii;
	short		NumLines, NumPoints, LeftRight;
	HANDLE		hPoints, hPoints16, hOffsets;
	HPDPOINT	Points;
	LPPOINTS	Points16;
	HPLONG		Offsets;
	double	left;
	double	top;
	double	right;
	double	bottom;
	double	fac;


	double	cellw;
	double	cellh;

	if (Fid == HFILE_ERROR)
		goto Exit;
	SaveDC (CurView->hDC);
	BigRead (Fid,&Header,sizeof(Header));
	GSSiClose (Fid);
	ifac = Header.ifac;
	fac = Header.fac;
	left = ((double)Header.left) / ifac;
	bottom = ((double)Header.bottom) / ifac;
	cellw = ((double)Header.cellw) / ifac;
	cellh = ((double)Header.cellh) / ifac;
	for (irow = 0;irow < Header.nrow-1; irow++)
	{
		for (icol = 0;icol < Header.ncol; icol++)
		{

			Bounds.xmn = left + cellw * icol;
			Bounds.xmx = left + cellw * (icol+1);
			Bounds.ymn = bottom + cellh * irow;
			Bounds.ymx = bottom + cellh * (irow+1);
			BoundsToPoints (&Bounds,CellArea,0);
			MidPoint = MinMaxMidPointD (&Bounds);
			if (DPointInBounds (&MidPoint,&CurView->WBounds))
				ii=1;
			{
				double Scale = cellw/256;
				POINT	ScreenPt;
				int		srow,scol, State;
				COLORREF	Color;

				CurView->BackGroundColor = 99000;
				//ZoomToPointAndScale (MidPoint,Scale,TRUE);
				SelectObject (CurView->hDC,GetStockObject(NULL_BRUSH)); 
				SelectObject (CurView->hDC,GetStockObject(BLACK_PEN)); 
				GWPolygonD (CurView->hDC, CellArea,4, 1, 0,0,FALSE,TRUE,0);
			}
		}
	}

	RestoreDC (CurView->hDC,-1);
Exit:
{
#if ENABLETRACE
GSSiExitProg (1425);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  

int RemoveShortSegs (int nPntsIn,LPHANDLE hPoly,double Dis)
#if ENABLETRACE
{GSSiEnterProg (1426);
#endif
{

	int	i, nPntsOut=1;
	LPDPOINT	PointsIn = GlobalLock (*hPoly);
	HANDLE		hPolyOut = GSSiGlobAlloc (0,GMEM_MOVEABLE,nPntsIn*sizeof(DPOINT));
	LPDPOINT	PointsOut = GlobalLock (hPolyOut);

	PointsOut[0] = PointsIn[0];
	for (i=1;i<nPntsIn;i++)
	{
		if (ldistp (PointsOut[nPntsOut-1],PointsIn[i]) > Dis)
			PointsOut[nPntsOut++] = PointsIn[i];
	}
	PointsOut[nPntsOut-1] = PointsIn[nPntsIn-1];
	GSSiGlobUlFree (hPoly);
	GlobalUnlock (hPolyOut);
	*hPoly = hPolyOut;
{
#if ENABLETRACE
GSSiExitProg (1426);
#endif
	return nPntsOut;
}
#if ENABLETRACE
}
#endif
}  

/*void CreatePNetStateGrid_old (void)
{
	MNMXCORD	Bounds;
	LPRADIAL	pRadial;  
	long		Refno, nPoly=0;  
	BOOL		SavePAP=PickAllPieces;
	HIGHLIGHTDATA	HighlightData;
	LPTHEME		pTheme;
	int			irow,icol;
	int			npnts,i,j,k;
	short		n;
	HANDLE		hPoly;
	DPOINT		MidPoint, IntPoints[4];
//	HANDLE		hCellPoint = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX);
//	HPDPOINT	CellPoint=GlobalLock (hCellPoint);
	HANDLE		hCellPoint16 = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX);
	HPPOINTS	CellPoint16=GlobalLock (hCellPoint16);
	HFILE		FidOut=GSSiOpenFile ("c:\\pntest.bin",0,OF_CREATE);
	HFILE		FidOutOff=GSSiOpenFile ("c:\\pntestoff.bin",0,OF_CREATE);
	int			TotRecs, nRecs=0;
	int			ifac = 1000000, nWithPoints=0, jfac;
	LPLONG		Offsets;
	HANDLE		hOffsets;
	char		msg[128];
	int			StateProvID=1, loff;
	double		fac;
	int			MaxLines=0, MaxPoints=0;
	short		LeftRight;
	double	OffDis = 0.0003;
	
	int		nrow=100,ncol=(nrow*90)/34.2;
	double	left   = -142;
	double	top	   = 60;
	double	right  = -52;
	double	bottom = 24.4;

	int		nrow=10,ncol=(nrow*90)/34.2;
	double	left   = -95.96;
	double	top	   = 40.6;
	double	right  = -95.35;
	double	bottom = 40.3;

	double	cellw = (right - left)/ncol;
	double	cellh = (top - bottom)/nrow;

	fac = 32000 / max (cellw,cellh);
	hOffsets  = GSSiGlobAlloc (0,GMEM_MOVEABLE,nrow*ncol*4);
	Offsets = GlobalLock (hOffsets);
	Header.ifac = ifac;
	Header.fac  = fac;
	Header.nrow = nrow;
	Header.ncol = ncol;
	Header.left = left * ifac;
	Header.bottom = bottom * ifac;
	Header.cellw = cellw * ifac;
	Header.cellh = cellh * ifac;
	left = ((double)Header.left) / ifac;
	bottom = ((double)Header.bottom) / ifac;
	cellw = ((double)Header.cellw) / ifac;
	cellh = ((double)Header.cellh) / ifac;
	BigWrite (FidOut,&Header,sizeof(Header),-1);
	CreateStatusWindow (CurView->hWnd,1,"Create PNGrid");
	TotRecs = nrow * ncol;
	for (irow = 0;irow < nrow; irow++)
	{
		for (icol = 0;icol < ncol; icol++)
		{
			CurVis->FileIsVisible[0]=0;
			CurVis->FileIsVisible[1]=1;
			Bounds.xmn = left + cellw * icol;
			Bounds.xmx = left + cellw * (icol+1);
			Bounds.ymn = bottom + cellh * irow;
			Bounds.ymx = bottom + cellh * (irow+1);
			MidPoint = MinMaxMidPointD (&Bounds);
			
			//BigWrite (FidOut,&Bounds,sizeof(Bounds),-1);
			loff = irow * ncol + icol;
			Offsets[loff] = GSSillseek (FidOut,0,1);
			ClearHighlightList (FALSE);
			n=HighlightInArea (CurView->hWnd,&Bounds,TRUE,FALSE,0);
			MaxLines = max (MaxLines,n);
			if (n)
			{
				int		NumLines=0, nint,line;
				short	pos = BT_FIRST;
				
				nWithPoints++;
   				while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))  
   				{
					HPDPOINT	DPoints;
					short		nCellPt=0, nCellPt2=0;
					BOOL		In=FALSE;
					LPSTR		pLoc = strrchr (HighlightData.PD.UDI,'-');		

					pos=BT_NEXT;
					if (pLoc)
					{
						pLoc++;
						StateProvID = atoi (pLoc);
					}
					else
						StateProvID = 99;
   					PickList[0]=HighlightData.PD;
    				GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&npnts,&hPoly);
					npnts = RemoveShortSegs (npnts,&hPoly,OffDis);
					DPoints = GlobalLock (hPoly);
					if (PointInBounds (DPoints[0],&Bounds))
					{
						In = TRUE;
						CellPoint[NumLines][nCellPt++] = DPoints[0];
					}
					for (j=0;j<npnts-1;j++)
					{
						nint = IntersectLineWithBounds (&DPoints[j],&Bounds,IntPoints);
						for (k=0;k<nint;k++)
						{
							if (In)
							{
								CellPoint[NumLines][nCellPt++] = IntPoints[k];
								LineID[NumLines] = StateProvID;
								nCellPts[NumLines++] = nCellPt;
								In = FALSE;
							}
							else
							{
								nCellPt =0;
								CellPoint[NumLines][nCellPt++] = IntPoints[k];
								In = TRUE;
							}
						}
						if (PointInBounds (DPoints[j+1],&Bounds))
							CellPoint[NumLines][nCellPt++] = DPoints[j+1];
					}
					if (In)
					{
						LineID[NumLines] = StateProvID;
						nCellPts[NumLines++] = nCellPt;
					}
					GSSiGlobUlFree (&hPoly);
				}
				n = NumLines;
				BigWrite (FidOut,&n,2,-1);
				for (line=0;line<NumLines;line++)
				{
					int	nCellPt2=0;

					LeftRight = LineID[line];
					for (i=1;i<nCellPts[line];i++)
					{
						DPOINT Pt, OffsetPt;
						int	noffpts=0;
						double	dis = ldistp (CellPoint[line][i-1],CellPoint[line][i]);
						double	AZ = getazd (&CellPoint[line][i-1],&CellPoint[line][i]);

						if (dis)
						{
							OffsetPt = dnewpt (CellPoint[line][i-1],AZ,dis/2);
							OffsetPt = dnewpt (OffsetPt,LTWOPI(AZ-HALFPI),OffDis);
							Pt.x = OffsetPt.x - MidPoint.x;
							Pt.y = OffsetPt.y - MidPoint.y;
							CellPoint16[nCellPt2].x = Pt.x * fac;
							CellPoint16[nCellPt2].y = Pt.y * fac;
							if (!nCellPt2)
								nCellPt2++;
							else if (CellPoint16[nCellPt2].x != CellPoint16[nCellPt2-1].x ||
								CellPoint16[nCellPt2].y != CellPoint16[nCellPt2-1].y)
								 nCellPt2++;
						}
					}
					MaxPoints = max (MaxPoints,nCellPt2);
					if (nCellPt2)
					{
						BigWrite (FidOut,&LeftRight,2,-1);
						BigWrite (FidOut,&nCellPt2,2,-1);
						BigWrite (FidOut,CellPoint16,nCellPt2*sizeof(POINTS),-1);
					}
				}
			}
			else
			{
				CurVis->FileIsVisible[0]=1;
				CurVis->FileIsVisible[1]=0;
				UseUserPickAp = FALSE;
				SystemPickAp = -P_TOL;	
				MaxPick=1;  
				SetPickAp(0);  
			    n = PickItems2 (CurView->hWnd,MidPoint,FALSE,TRUE,FALSE);
				if (n)
				{
					LPSTR pLoc = strrchr (PickList[n-1].UDI,'-');

					if (pLoc)
					{
						pLoc++;
						StateProvID = atoi (pLoc);
					}
					else
						StateProvID = 99;
				}
				else
					StateProvID = 99;
				Offsets[loff] = -StateProvID;
			    UseUserPickAp = TRUE;
			}
			StatusWindowUpdate (0,0,TotRecs,++nRecs);
			if (!ContinueProcessing)
				goto Exit;
		}
	}
Exit:
	ContinueProcessing = TRUE;
//	GSSiGlobUlFree (&hCellPoint);
	GSSiGlobUlFree (&hCellPoint16);
	GSSiClose (FidOut);
	BigWrite (FidOutOff,Offsets,nrow*ncol*4,-1);
	GSSiClose (FidOutOff);
	GSSiGlobUlFree (&hOffsets);
	DestroyStatusWindow(0);
	sprintf (msg,"Totcells: %i, NumWithPoints: %i, MaxLines:%i, MaxPoints:%i",TotRecs,nWithPoints,MaxLines,MaxPoints);
	MessageBox (0,msg,0,MB_OK);
	return;
}*/

int GetNextCompressedRow(int irow,LPBYTE NextRow,LPINT pType,LPBYTE States)
#if ENABLETRACE
{GSSiEnterProg (1427);
#endif
{

	int	len=0, col, j;
	int	NumStates=0, CurState;
	LPBYTE	pCurState, pRunLen;

	for (col=0;col<256;col++)
	{
		for (j=0;j<NumStates;j++)
			if (States[j] == CellArray[irow][col])
				goto Next;
		States[NumStates++] = CellArray[irow][col];
Next:;
	}
	if (NumStates < 3)
	{
		*pType = 1;
		NextRow[len] = 0;
		CurState = CellArray[irow][0];
		for (col=1;col<256;col++)
		{
			if (CellArray[irow][col] == CurState)
				NextRow[len]++;
			else
			{
				CurState = CellArray[irow][col];
				NextRow[++len]=0;
			}
		}
	}
	else
	{
		*pType = 2;
		pRunLen = &NextRow[len++];
		pCurState = &NextRow[len++];
		*pRunLen = 0;
		*pCurState = CellArray[irow][0];
		for (col=1;col<256;col++)
		{
			if (CellArray[irow][col] == *pCurState)
				(*pRunLen)++;
			else
			{
				pRunLen = &NextRow[len++];
				pCurState = &NextRow[len++];
				*pRunLen = 0;
				*pCurState = CellArray[irow][col];
			}
		}
		len--;
	}
{
#if ENABLETRACE
GSSiExitProg (1427);
#endif
	return len;
}
#if ENABLETRACE
}
#endif
}  

BOOL CurRowEqNextRow (LPBYTE pCurRow,LPBYTE NextRow,int len)
{
	int	i;

	for (i=0;i<len;i++)
		if (pCurRow[i] != NextRow[i])
			return FALSE;
	return TRUE;
}

int	CompressPNCellArray (int NumStates,LPBYTE CompressedCell)
{
	int	Clen=0, NextRowLen=0, CurType=0,Type,irow=0,i;
	BYTE	States[8];	
	LPBYTE	pCurRow, pCurLen, pCurRepeat, pRowsInType, pCurStateList=States;
	BYTE	NextRow[1024];

	memset (States,0,sizeof(States));
	while (irow < 256)
	{
		NextRowLen = GetNextCompressedRow(irow++,NextRow,&Type,States);
		if (Type != CurType || (Type == 1 && (States[0] != pCurStateList[0] || States[1] != pCurStateList[1])))
		{
			CompressedCell[Clen++] = Type;
			CurType = Type;
			pRowsInType = &CompressedCell[Clen++];
			*pRowsInType = 0;
			if (CurType == 1)
			{
				pCurStateList = &CompressedCell[Clen]; 
				CompressedCell[Clen++] = States[0];
				CompressedCell[Clen++] = States[1];
			}
			pCurRepeat = &CompressedCell[Clen++];
			*pCurRepeat = 0;
			pCurLen = &CompressedCell[Clen++];
			*pCurLen = NextRowLen;
			pCurRow = &CompressedCell[Clen];
			for (i=0;i<NextRowLen+1;i++)
				CompressedCell[Clen++] = NextRow[i];
		}
		else
		{
			(*pRowsInType)++;
			if (NextRowLen == *pCurLen && CurRowEqNextRow (pCurRow,NextRow,NextRowLen+1))
				(*pCurRepeat)++;
			else
			{
				pCurRepeat = &CompressedCell[Clen++];
				*pCurRepeat = 0;
				pCurLen = &CompressedCell[Clen++];
				*pCurLen = NextRowLen;
				pCurRow = &CompressedCell[Clen];
				for (i=0;i<NextRowLen+1;i++)
					CompressedCell[Clen++] = NextRow[i];
			}
		}
	}
	return Clen;
}

void CreatePNetStateGrid (int NumRows)
{
	MNMXCORD	Bounds;
	LPRADIAL	pRadial;  
	long		Refno, nPoly=0;  
	BOOL		SavePAP=PickAllPieces;
	HIGHLIGHTDATA	HighlightData;
	LPTHEME		pTheme;
	int			irow,icol;
	int			npnts,i,j,k;
	short		n;
	HANDLE		hPoly;
	DPOINT		MidPoint, IntPoints[4],	CellArea[5];
//	HANDLE		hCellPoint = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX);
//	HPDPOINT	CellPoint=GlobalLock (hCellPoint);
	HANDLE		hCellPoint16 = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX);
	HPPOINTS	CellPoint16=GlobalLock (hCellPoint16);
	HFILE		FidOut=GSSiOpenFile ("c:\\pntest.bin",0,OF_CREATE);
	HFILE		FidOutOff=GSSiOpenFile ("c:\\pntestoff.bin",0,OF_CREATE);
	int			TotRecs, nRecs=0;
	int			ifac = 10000000, nWithPoints=0, jfac;
	LPLONG		Offsets;
	HANDLE		hOffsets;
	char		msg[128];
	int			StateProvID=1, loff;
	double		fac;
	int			MaxLines=0, MaxPoints=0,ii,lastoff=0;
	short		LeftRight;
	double		OffDis = 0.0003;
	HANDLE		hCompressedCell = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX);
	LPBYTE		CompressedCell = GlobalLock (hCompressedCell);
	char		str[256];
	
	int		nrow=NumRows,ncol=(nrow*90)/34.2;
	double	left   = -142;
	double	top	   = 70.0;
	double	right  = -52;
	double	bottom = 24.4;

/*	int		nrow=10,ncol;
	double	left   = -95.96;
	double	top	   = 40.8;
	double	right  = -95.35;
	double	bottom = 40.3;*/

	double	cellw = (top - bottom)/nrow;
	double	cellh = cellw;
	
	ncol = (right - left - cellw/2) / cellw + 1;

	fac = 32000 / max (cellw,cellh);
	hOffsets  = GSSiGlobAlloc (0,GMEM_MOVEABLE,nrow*ncol*4);
	Offsets = GlobalLock (hOffsets);
	Header.ifac = ifac;
	Header.fac  = fac;
	Header.nrow = nrow;
	Header.ncol = ncol;
	Header.left = left * ifac;
	Header.bottom = bottom * ifac;
	Header.cellw = cellw * ifac;
	Header.cellh = cellh * ifac;
	left = ((double)Header.left) / ifac;
	bottom = ((double)Header.bottom) / ifac;
	cellw = ((double)Header.cellw) / ifac;
	cellh = ((double)Header.cellh) / ifac;
	BigWrite (FidOut,&Header,sizeof(Header),-1);
//	CreateStatusWindow (CurView->hWnd,1,"Create PNGrid");
	TotRecs = nrow * ncol;
	for (irow = 0;irow < nrow; irow++)
//	for (irow = 6;irow < 7; irow++)
	{
		for (icol = 0;icol < ncol; icol++)
//		for (icol = 11;icol < 12; icol++)
		{
			sprintf (str,"Row:%i of %i  Col:%i of %i len:%i",irow,nrow,icol,ncol,lastoff);
			SetWindowText (hWndMain,str);
			if (icol == 13)
				ii=1;
			Bounds.xmn = left + cellw * icol;
			Bounds.xmx = left + cellw * (icol+1);
			Bounds.ymn = bottom + cellh * irow;
			Bounds.ymx = bottom + cellh * (irow+1);
			BoundsToPoints (&Bounds,CellArea,0);
			MidPoint = MinMaxMidPointD (&Bounds);
			
			//BigWrite (FidOut,&Bounds,sizeof(Bounds),-1);
			loff = irow * ncol + icol;
			Offsets[loff] = lastoff = GSSillseek (FidOut,0,1);
			{
				double Scale = cellw/256;
				POINT	ScreenPt;
				int		srow,scol, State,ii,is;
				COLORREF	Color;
				int	NumStates = 0;
				int	States[8];

				States[0]=99;
				CurView->BackGroundColor = 99000;
				ZoomToPointAndScale (MidPoint,Scale,TRUE);
				SelectObject (CurView->hDC,GetStockObject(NULL_BRUSH)); 
				SelectObject (CurView->hDC,GetStockObject(BLACK_PEN)); 
				//GWPolygonD (CurView->hDC, CellArea,4, 1, 0,0,FALSE);
				ScreenPt = BasePtToScreenPt (&CellArea[0]);
				for (srow = 0;srow < 256;srow++)
				{
					for (scol = 0;scol < 256;scol++)
					{
						Color = GetPixel (CurView->hDC,ScreenPt.x+scol,ScreenPt.y-srow);
						State = Color/1000;
						CellArray[srow][scol] = State;
						for (is=0;is<NumStates;is++)
							if (State == States[is])
								goto HaveState;
						States[NumStates++] = State;
HaveState:;
					}
					ii=1;
				}
				if (NumStates < 2)
					Offsets[loff] = -States[0];
				else
				{
					int	Clen = CompressPNCellArray (NumStates,CompressedCell);

					BigWrite (FidOut,CompressedCell,Clen,-1);
				}
			}

			//StatusWindowUpdate (0,0,TotRecs,++nRecs);
			if (!ContinueProcessing)
				goto Exit;
		}
	}
Exit:
	ContinueProcessing = TRUE;
//	GSSiGlobUlFree (&hCellPoint);
	GSSiGlobUlFree (&hCellPoint16);
	GSSiGlobUlFree (&hCompressedCell);
	GSSiClose (FidOut);
	BigWrite (FidOutOff,Offsets,nrow*ncol*4,-1);
	GSSiClose (FidOutOff);
	GSSiGlobUlFree (&hOffsets);
	//DestroyStatusWindow(0);
	sprintf (msg,"Totcells: %i, NumWithPoints: %i, MaxLines:%i, MaxPoints:%i",TotRecs,nWithPoints,MaxLines,MaxPoints);
	MessageBox (0,msg,0,MB_OK);
	return;
}

 



