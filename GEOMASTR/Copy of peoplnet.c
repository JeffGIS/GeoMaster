#include "graphint.h"    
#include "translat.h"
#include "pnet.h"

#define MAXCHANGES	1024

static	char	StateIDs[74][4];  
static	RGBQUAD	ColorChangesFrom[MAXCHANGES],ColorChangesTo[MAXCHANGES]; 
static	RGBQUAD	SavePalette[256];
static	short	NumColorChanges=0, NumSavePaletteColors; 
static	BOOL	HaveSavePalette=FALSE;
static	double	ChangeDist=2;

#include "gmextern.h"

typedef struct {long FromAddL, ToAddL, FromAddR, ToAddR, ZipL, ZipR, CountyL, CountyR, MCDL, MCDR;} PNADDRESSDATA;
typedef PNADDRESSDATA	FAR	*LPPNADDRESSDATA;

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
	return;

}

double GetLocalCorrection (LPMNMXCORD pBounds)
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
	return f; 
} 

BOOL MakeMap (LPSTR Args)
{	double	Scale, InScale;   
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
	LPSTR	CmdMess, pDot;
	HCURSOR	hcurSave;
	BOOL	ReturnMapCmd=GetGlobalBVal2 ("[%RETURNMAPCMD]",FALSE); 
			
   	MemMapStartTime = GetTickCount();
   	TotPointsProcessed = 0;   
	GSSiGlobFree (&hMemMapColorMap);		
	if (!(ParLoc = MatchLev (Args,','))) return FALSE;
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
	    SetMainRect (CurView->hWnd,CurView->hDC,NULL);
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
		InScale = atof (Arg4);
		ExpandText (Arg5);  
		_fstrcpy (MemMapName,Arg5);
	}
	else if (!_fstricmp(Arg1,"CITY")) 
	{    
		DPOINT	CityPoint, p1, p2;
		double	WDist;
		
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
		short	Attempt=0, Err;
		DWORD	SaveMMH=MemMapHeight,SaveMMW=MemMapWidth;
		LPSTR	pDot, pComma;
		
		if (!(ParLoc = MatchLev (Arg3,','))) goto RtnFalse;
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
		pDot = _fstrrchr (MemMapName,'.');
		if (!_fstrnicmp (pDot,".gmd(",4))
		{   
			pDot += 4;
			*pDot++ = 0;
			pComma = _fstrchr (pDot,',');
			if (pComma)
			{
				*pComma++ = 0;
				MemMapSubDir = atoi (pComma);
			}
			else
				MemMapSubDir = 1;
			MemMapID = atol (pDot); 
			if (!ExistFile (MemMapName)) 
			{
				char	DefStr[]="MapID(B4),SubDir(B2),Size(B4),Time(B4),NumPoints(B4),Loc(B4),LocalCorrection(R8),MetersPerDegree(R8),Scale(R8),MNX(R8),MNY(R8),MXX(R8),MXY(R8)";   
				
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
	    SetBounds (CurView->hWnd,NULL);
		CurView->WindowIsZoomed = TRUE; 
		SelectClipRgn (CurView->hDC,NULL);			
		SetDisplayMode (CurView->hDC,GF_TEXTMODE); 
		xmid = (CurView->WBounds.xmn+CurView->WBounds.xmx)/2;
		ymid = (CurView->WBounds.ymn+CurView->WBounds.ymx)/2; 
		degperpixelx = (CurView->WBounds.xmx - CurView->WBounds.xmn)/(double)bmwidth;
		degperpixely = (CurView->WBounds.ymx - CurView->WBounds.ymn)/(double)bmheight;
		CmdMess = GlobalLock (hCmdMess);
		sprintf (CmdMess,"%.14lg;%.14lg;%.14lg;%.14lg;%.14lg",xmid,ymid,Scale,degperpixelx,degperpixely);
		GlobalUnlock (hCmdMess);
		DoPaint = TRUE;	 
		hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
		PaintMap (CurView->hWnd,CurView->hDC,TRUE,NULL); 
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
		short	Err;
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
			    SetMainRect (CurView->hWnd,CurView->hDC,NULL);
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
			PaintMap (CurView->hWnd,hdcMemMap,TRUE,NULL);  
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
		hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
		PaintMap (CurView->hWnd,CurView->hDC,TRUE,NULL); 
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
	GSSiGlobUlFree (&hMem);
	return TRUE;
RtnFalse:
	GSSiGlobUlFree (&hMem);
	return FALSE;
}  

BOOL GetCountyName (short State,short County,LPSTR CountyName)
{   
	static	BOOL	First = TRUE; 
	HANDLE	hBTCountyName;  
	char	str[130]; 
	short	st;
	struct	{short	State, County;} CNKEY;
	
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
		return FALSE;
	hBTCountyName = BT_OPEN ("countynm.btr", 0, BT_READ, 0); 
	if (!hBTCountyName)
		return FALSE;
	CNKEY.State = State;
	CNKEY.County = County;
	st = BT_FIND (hBTCountyName,(LPSTR)&CNKEY,BT_FIRST,BT_EQ,CountyName);
	BT_CLOSE (hBTCountyName);
	if (st)
		return FALSE;
	return TRUE;
} 

BOOL GetFIPSName (short State,long FIPS,LPSTR Name)
{  
	char	SQL[128]; 
	HANDLE	hSQL=0; 
	BOOL	rtn;
	
	if (State > 60)
	{
		char	DBName[]="FIPS=[%DL]ATTRIBUT\\CANMUN.GMD";

		sprintf (SQL,"CANMUNNUM==%ld",FIPS);
		if (!OpenDataFile (DBName,SQL,BT_READ,&hSQL))
			return FALSE;
		rtn = GetValFromOpenFiles ("CANMUNNAME",Name);  
	} 
	else
	{
		char	DBName[]="FIPS=[%DL]attribut\\fips.gmd";

		sprintf (SQL,"FIPS_st_cd==%i&&FIPS_place_cd=%ld",State,FIPS);
		if (!OpenDataFile (DBName,SQL,BT_READ,&hSQL))
			return FALSE;
		rtn = GetValFromOpenFiles ("Feat_name",Name);  
	}
    CloseDataFile (TRUE,&hSQL); 
	return rtn;
}

BOOL WhereAt (LPSTR Args) 
{	double	Scale; 
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
    
	if (!(ParLoc = MatchLev (Args,','))) return FALSE;
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
			_fmemmove (NearPickedStreets,&PickedStreets[0],28);
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
			_fmemmove (NearPickedStreets,&PickedStreets[0],28);
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
			_fmemmove (NearPickedStreets,&PickedStreets[0],28);
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
		_fmemmove (&PickedStreets[0],NearPickedStreets,28);
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
	    
	    _fmemmove (&CurStreetNumbers[0],&PickedStreets[0],16);		
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
	return TRUE;
} 
		
BOOL GetCityCoord (LPSTR Name,LPDPOINT pDPoint,LPMNMXCORD pMinMax)
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
	
	return rtn;
}
  
short GetStateNum (LPSTR ID)
{   
	HFILE	Fid;
	char	StateStr[132];  
	int		istate;  
	OFSTRUCT	OFStruct;
	
	Fid = GSSiOpenFile ("[%INDIR]states.txt",&OFStruct,OF_READ); 
	if (Fid==HFILE_ERROR) return 0;
	while (fgetstring (StateStr,128,Fid))
	{
		istate = atoi (StateStr); 
		if (!_fstricmp (ID,&StateStr[66]))
		{
			GSSiClose (Fid);
			return istate;
		}
	}
	GSSiClose (Fid);
	return 0;
}

BOOL GetStateID (short StateNum, LPSTR ID)
{   
	HFILE	Fid;
	char	StateStr[132];  
	int		istate;
	static	BOOL	First=TRUE;
	
	*ID = 0;
	if (StateNum <1 || StateNum > 73)
		return FALSE;  
	if (First)
	{   
		_fmemset (StateIDs,0,sizeof(StateIDs));
		Fid = GSSiOpenFile ("[%DL]states.txt",NULL,OF_READ); 
		if (Fid==HFILE_ERROR)
			return FALSE;
		while (fgetstring (StateStr,128,Fid))
		{
			istate = atoi (StateStr); 
			_fstrcpy (StateIDs[istate],&StateStr[66]);
		}
		GSSiClose (Fid); 
		First = FALSE;
	} 
	_fstrcpy (ID,StateIDs[StateNum]);
	return TRUE;
} 

void FixCaps (LPSTR str)
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
	return;
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
        CurView->pPickListManual =(LPVISLIST) GlobalLock (hVisList);
        CurView->pPickListManual->hVisList = hVisList;   
    }
    
	SaveMaxPick = MaxPick;
	MaxPick = 1; 
    CurView->pPickListManual->FileIsVisible[0]=0; 
    CurView->pPickListManual->FileIsVisible[1]=0; 
    CurView->pPickListManual->FileIsVisible[2]=0; 
    CurView->pPickListManual->FileIsVisible[3]=0; 
    CurView->pPickListManual->FileIsVisible[4]=0; 
    CurView->pPickListManual->FileIsVisible[5]=0; 
    CurView->pPickListManual->FileIsVisible[6]=0; 
    CurView->pPickListManual->FileIsVisible[7]=0; 
    CurView->pPickListManual->FileIsVisible[8]=0; 
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
   	CurView->pPickListManual->FileIsVisible[9]=1;
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
	return Pop;
} 
short GetIntersectStuff (long TLID, HANDLE hIntersect,LPTIGER1 Tiger1,LPSHORT Stuff)
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
        return 0; 
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
    return TotLen;
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
{   
    short     NumLinks=0, MaxLinks=25;
    TIGER1_MN   Tiger1PN; 
    short TotLen=0;
    LPSHORT   pNum;
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
        return FALSE; 
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
                    return TRUE;
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
        return FALSE; 
    TLID = NextTLID;
    IntLong = NextLong;
    IntLat = NextLat;  
    NumLinks++;
    goto NextLink;
}

void DisplayPNData (long Refno,HANDLE hIntData,HANDLE hPNAddData)
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
	return;
} 

HANDLE DecodePNAddData (HANDLE hPNAddData)
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
	return handle; 
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

BOOL SetPNParms (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
{HDC hDC;
 char key;     
 char	str[128];
 static POINT	MousePoint;
 DPOINT	BasePoint; 
 COLORREF	Color;
 static	BOOL	ColorLocked;

 switch (Message)
   {
   	case GF_INIT:
		PostMessage(hWnd, GF_EXECUTE,0, 0L);
		ColorLocked = FALSE;
   		break;

	case GF_EXECUTE:
		if (!hWndPNParms)
		{
		                  
		  lpfnPNPARMSMsgProc = MakeProcInstance((FARPROC)PNPARMSMsgProc, hInst);
		  hWndTraverseEntry=CreateDialog(hInst,"PNPARMS",hWnd, lpfnPNPARMSMsgProc);
		}
		else
			ShowWindow (hWndPNParms,SW_RESTORE);

		break;
    
    case WM_CHAR:
    {
		switch (wParam)
		{   
			case 'C':
			case 'c': 
				BasePoint =  WinPtToBasePt (MousePoint);       
				ftoa (str,BasePoint.x);
            	SetDlgItemText (hWndPNParms,IDC_LONGITUDE,str);
				ftoa (str,BasePoint.y);
            	SetDlgItemText (hWndPNParms,IDC_LATITUDE,str);
            break;

            default:
            	return FALSE;
            break;
		}  
		return TRUE;
	}
    case WM_MOUSEMOVE:   
    	MousePoint = MAKEPOINT(lParam);   
    	if (ColorLocked)
    		break;
    	hDC = GetDC (hWnd);
    	Color = GetPixel (hDC,MousePoint.x,MousePoint.y); 
    	ReleaseDC (hWnd,hDC);  
    	PostMessage(hWndPNParms, WM_COMMAND, IDC_SHOWCOLOR, Color); 
    	break;  
    case WM_LBUTTONDOWN:
    	break;
    case WM_LBUTTONUP:
    	if (ColorLocked)
    		ColorLocked = FALSE;
    	else
    		ColorLocked = TRUE;
    	break;
    default:
    	return (FALSE);
    }
    return (TRUE);
}

BOOL ChangeImageColors (LPSTR ImageFile,BOOL UseSavedPalette)
{  
	HDIB32 hDib;
	RGBQUAD	PaletteRGB[256];  
	USHORT	i,j; 
	BOOL	rtn=FALSE;   
	long	NumColors; 
	
	AddBMPToCache32 (NULL,0);
	hDib = BMPHandleFromEXT (ImageFile); 
	if (!hDib)
		return FALSE;
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
	return rtn;
}  

BOOL LoadColorChanges (void) 
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
     return rtn;
}

BOOL SaveColorChanges (HWND hWnd) 
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
	return TRUE;
}
 
BOOL FAR PASCAL PNPARMSMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{
	
 int	BRtn;  
 RECT	Rect;   
 char	ImageFile[256], FileName[256]; 
 HDIB32	hDib;   
 static	COLORREF	Color;   
 static	RGBQUAD	ColorRGB;      
 static	short		CurPalIndex; 
 char	txt[520],latitude[32],longitude[32],distance[16],imagefile[128],line[520];
 static	short	NearColorIndex;
 short	i,bmw,bmh;
 BOOL	Error; 
 LPSTR	pTab; 
 HFILE	Fid;  
 static	BOOL	SettingsChanged;
 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
	{    
         hWndPNParms = hWndDlg;    
         SettingsChanged = FALSE;  
         LoadColorChanges ();
         if (NumColorChanges)	
			EnableWindow (GetDlgItem(hWndDlg,IDC_CANCELCHANGE),TRUE); 
         SetDlgItemTextGlobal (hWndDlg,IDC_SERVERIP,"[%SERVERIP]","192.168.0.1");
         SetDlgItemTextGlobal (hWndDlg,IDC_SERVERSOCKET,"[%SERVERSOCKET]","500");
         SetDlgItemTextGlobal (hWndDlg,IDC_SERVERPATH,"[%SERVERPATH]","\\\\Powerspec\\c");
         SetDlgItemTextGlobal (hWndDlg,IDC_MAPFILE,"[%IMAGEFILE_REMOTE]","c:\\testr.tif"); 
         SetDlgItemTextGlobal (hWndDlg,IDC_MAPFILEL,"[%IMAGEFILE_LOCAL]","c:\\testl.tif"); 
         SetDlgItemText (hWndDlg,IDC_LATITUDE,"45.00000");
         SetDlgItemText (hWndDlg,IDC_LONGITUDE,"-93.00000");  
         SetDlgItemText (hWndDlg,IDC_DISTANCE,"2");  
         SetDlgItemText (hWndDlg,IDC_INTENSITY,"100");  
		 SendDlgItemMessage (hWndDlg,IDC_DISPLAYRESULT,BM_SETCHECK,FALSE,0L);
         
	} 

        break; /* End of WM_INITDIALOG                                 */
    case WM_CLOSE: 
         hWndPNParms = 0; 
		 FreeProcInstance(lpfnPNPARMSMsgProc); 
	     break;
	
	case WM_PAINT:
    	 PostMessage(hWndDlg, WM_COMMAND, IDC_SHOWCOLOR, Color);
    	 return FALSE; 
		       
    case WM_COMMAND:
         switch(wParam)
           { 
           	case IDC_INTENSTEST: 
           	{
				short	intens = GetDlgItemInt(hWndDlg,IDC_INTENSITY,&Error,TRUE);   
                
                if (intens < 10 || intens > 100)
                	MessageBox (hWndDlg,"Intensity must be between 10 and 100 percent",NULL,MB_ICONEXCLAMATION);
                else
                {    
					EnableWindow (GetDlgItem(hWndDlg,IDC_CANCELCHANGE),TRUE); 
                	ColorChangesFrom[NumColorChanges].rgbReserved = 1;
                	ColorChangesTo[NumColorChanges++].rgbRed = (100 - intens);
					SetViewport(*pCommandViewport);
					_fstrcpy (ImageFile,CurView->lpFiles[0]);
					ExpandText (ImageFile); 
					ChangeImageColors (ImageFile,TRUE);  
					SaveColorChanges (hWndDlg);
					RedisplayWindow ();
				}
			}
           		break;
           	
           	case IDC_SHOWSCALE:
           	{
           		HFILE Fid=GSSiOpenFile ("[%DL]scale.txt",NULL,OF_READ);
           		
           		if (Fid != HFILE_ERROR)
           		{
           			fgetstring (txt,64,Fid);
           			GSSiClose (Fid);
           			SetDlgItemText (hWndDlg,IDC_MAPSCALE,txt);
           		}  
           	}
           		break;
           			
           	case IDC_CANCELCHANGE:
           		NumColorChanges = max (0,NumColorChanges-1); 
				SaveColorChanges (hWndDlg); 
				if (!NumColorChanges)
					EnableWindow (GetDlgItem(hWndDlg,IDC_CANCELCHANGE),FALSE); 
				SetViewport(*pCommandViewport);
				_fstrcpy (ImageFile,CurView->lpFiles[0]);
				ExpandText (ImageFile); 
				ChangeImageColors (ImageFile,TRUE); 
				RedisplayWindow ();
           		break;
			case IDC_SHOWCOLOR:
			{   
				HWND	hWnd = GetDlgItem (hWndDlg,IDC_COLORFRAME);
				HDC		hDC = GetDC (hWnd);
				
				Color = lParam;   
				ColorRGB = RGBQUADFromCOLORREF (Color);
				GetClientRect (hWnd,&Rect);
				FillRectPoly(hDC, &Rect, Color);
				ReleaseDC (hWnd,hDC);
				break;
			} 
			
			case IDC_SERVERCONNECT: 
				GetDlgItemText (hWndDlg,IDC_SERVERIP,txt,64);
			   	SetGlobalValue("%SERVERIP",txt);
			   	UpdateGlobalFile ("[%DL]global.ini","%SERVERIP",txt); 
				GetDlgItemText (hWndDlg,IDC_SERVERPATH,txt,64);
			   	SetGlobalValue("%SERVERPATH",txt);  
			   	UpdateGlobalFile ("[%DL]global.ini","%SERVERPATH",txt); 
				GetDlgItemText (hWndDlg,IDC_SERVERSOCKET,txt,32);
			   	SetGlobalValue("%SERVERSOCKET",txt);
			   	UpdateGlobalFile ("[%DL]global.ini","%SERVERSOCKET",txt); 
			   	_fstrcpy (txt,"$TCPOPEN([%SERVERIP],[%SERVERSOCKET],SOCKET,,@$MACRO([%DL]macros\\tcpinput.txt),@$MACRO([%DL]macros\\tcpclose.txt))");
                ExpandText (txt);
                if (atob (txt)) 
                {
                	SetDlgItemText (hWndDlg,IDC_MESSAGE,"Server Opened");  
                	EnableWindow (GetDlgItem(hWndDlg,IDC_MAKEMAP),TRUE);
                	EnableWindow (GetDlgItem(hWndDlg,IDC_MAKETEXT),TRUE);
                	EnableWindow (GetDlgItem(hWndDlg,IDC_SERVERDISCONNECT),TRUE);
                	EnableWindow (GetDlgItem(hWndDlg,IDC_SERVERCONNECT),FALSE);
					_fstrcpy (txt,"$SERVERFILE(GET,[SOCKET],@[%DL]zoommac1.txt,[%DL]settings.txt)");
	                ExpandText (txt);
                }
                else
                	MessageBox (hWndDlg,"Failed to open server",NULL,MB_ICONEXCLAMATION);
				break;  
			
			case IDC_SERVERDISCONNECT: 
			   	_fstrcpy (txt,"$TCPCLOSE([SOCKET])");
                ExpandText (txt);
                if (atob (txt)) 
                {
                	SetDlgItemText (hWndDlg,IDC_MESSAGE,"Server Closed");
                	EnableWindow (GetDlgItem(hWndDlg,IDC_MAKEMAP),FALSE);
                	EnableWindow (GetDlgItem(hWndDlg,IDC_MAKETEXT),FALSE);
                	EnableWindow (GetDlgItem(hWndDlg,IDC_SERVERDISCONNECT),FALSE);
                	EnableWindow (GetDlgItem(hWndDlg,IDC_SERVERCONNECT),TRUE);
                }
                else
                	MessageBox (hWndDlg,"Failed to close server",NULL,MB_ICONEXCLAMATION);
				break;  
			
			case IDC_MAKEMAP: 
				SetViewport(*pCommandViewport);
				bmw = CurView->DrawRect.right - CurView->DrawRect.left + 1;
				bmh = CurView->DrawRect.bottom - CurView->DrawRect.top + 1;
				GetDlgItemText (hWndDlg,IDC_LONGITUDE,longitude,32);
				GetDlgItemText (hWndDlg,IDC_LATITUDE,latitude,32);
				GetDlgItemText (hWndDlg,IDC_DISTANCE,distance,16);
				GetDlgItemText (hWndDlg,IDC_MAPFILE,imagefile,128);
				GetDlgItemText (hWndDlg,IDC_MAPFILEL,txt,128);
			   	SetGlobalValue("%IMAGEFILE2",txt); 
			   	SetGlobalValue("%IMAGEFILE_LOCAL",""); 
			   	HaveSavePalette = FALSE;
       			SetDlgItemText (hWndDlg,IDC_MAPSCALE,"Waiting for map");   
       			if (SettingsChanged)
       			{   
       				HFILE	Fid = GSSiOpenFile ("[%DL]settings.txt",NULL,OF_READ);
       				
       				if (Fid != HFILE_ERROR)
       				{  
   						sprintf (txt,"$TCPSEND([SOCKET],$STR($DELETEFILE([%%DL]zoommac1.txt)),T)");
   						ExpandText (txt);
       					while (fgetstring (line,512,Fid))
       					{
       						sprintf (txt,"$TCPSEND([SOCKET],$STR($APPEND([%%DL]zoommac1.txt,$STR(%s))),T)",line);
       						ExpandText (txt);
       					}
       					GSSiClose (Fid);
       				}
       				SettingsChanged = FALSE; 
       			}
				sprintf (txt,"$TCPSEND([SOCKET],$STR([%%ALLOWCACHE]=F;[%%ALLOWCACHE]=T;[%%APPLYCOLORCHANGES]=F;[BITMAP_WIDTH]=%i;[BITMAP_HEIGHT]=%i;[C]=$MAKEMAP(COORD,%s,%s,%s,%s,[%%DL]colormap.bin)),T)",bmw,bmh,longitude,latitude,distance,imagefile);
            	SetGlobalValueBool ("SHOWRESULT",(BOOL)SendDlgItemMessage (hWndDlg,IDC_DISPLAYRESULT,BM_GETCHECK,0,0));
				ExpandText (txt); 
				RedisplayWindow ();
				break; 
				
			case IDC_MAKETEXT: 
				GetDlgItemText (hWndDlg,IDC_LONGITUDE,longitude,32);
				GetDlgItemText (hWndDlg,IDC_LATITUDE,latitude,32);
				sprintf (txt,"$TCPSEND([SOCKET],$STR([%%PNFORMAT]=11111111111;[%C]=$WHEREAT(%s,%s)))",longitude,latitude);
				SetGlobalValueBool ("SHOWRESULT",TRUE);
				ExpandText (txt);
				break; 
				
			case IDC_CHANGECOLOR:
       			ColorChangesFrom[NumColorChanges] = RGBQUADFromCOLORREF ((COLORREF)Color);
				if (!GetColor (hWndDlg,&Color))
					break;  
				EnableWindow (GetDlgItem(hWndDlg,IDC_CANCELCHANGE),TRUE); 
       			ColorChangesTo[NumColorChanges++] = RGBQUADFromCOLORREF ((COLORREF)Color);
				SetViewport(*pCommandViewport);
				_fstrcpy (ImageFile,CurView->lpFiles[0]);
				ExpandText (ImageFile); 
				ChangeImageColors (ImageFile,TRUE);
				EnableWindow (GetDlgItem(hWndDlg,IDC_CANCELCHANGE),TRUE); 
				SetWindowText (GetDlgItem (hWndDlg,IDC_SHOWONLYCOLOR),"Show Usage");
				SaveColorChanges (hWndDlg);
				RedisplayWindow ();
				break;
			
			case IDC_SENDTOSERVER:
       		{   
       				HFILE	Fid = GSSiOpenFile ("[%DL]colorchanges.txt",NULL,OF_READ);
       				
       				if (Fid != HFILE_ERROR)
       				{  
   						sprintf (txt,"$TCPSEND([SOCKET],$STR($DELETEFILE([%%DL]colorchanges.txt)),T)");
   						ExpandText (txt);
       					while (fgetstring (line,512,Fid))
       					{
       						sprintf (txt,"$TCPSEND([SOCKET],$STR($APPEND([%%DL]colorchanges.txt,$STR(%s))),T)",line);
       						ExpandText (txt);
       					}
       					GSSiClose (Fid); 
       				}
       		}
				break;
				
			case IDC_EDITSETTINGS:  
				_fstrcpy (FileName,"[%DL]settings.txt");
				EditTextFile (hWndDlg,FileName);
				SettingsChanged=TRUE;
				break;
					
/*			case IDC_SHOWPALETTE:
			{   
				HWND	hWnd = GetDlgItem (hWndDlg,IDC_COLORFRAME);
				HDC		hDC = GetDC (hWnd); 
				BOOL	Error;
				short	i=GetDlgItemInt(hWndDlg,IDC_PALINDEX,&Error,TRUE);   
				COLORREF	Color2;
				
				Color2 = RGB (SavePalette[i].rgbRed,SavePalette[i].rgbGreen,SavePalette[i].rgbBlue);
				GetClientRect (hWnd,&Rect);
				FillRectPoly(hDC, &Rect, Color2);
				ReleaseDC (hWnd,hDC); 
				sprintf (txt,"%i-%i-%i",GetRValue(Color2),GetGValue(Color2),GetBValue(Color2));
				SetDlgItemText (hWndDlg,IDC_COLORSEP,txt);
				i++;    
				SetDlgItemInt(hWndDlg,IDC_PALINDEX,i,TRUE);  
				
				break;
			}*/ 
			case IDC_SHOWONLYCOLOR: 
			{   
				long	NumColors;
				RGBQUAD	PaletteRGB[256];  
				USHORT	i;
				double	colordif, maxdif=DBL_MAX;
				RGBQUAD	White=RGBQUADFromCOLORREF(RGB(255,255,255));
				
				SetViewport(*pCommandViewport);
				_fstrcpy (ImageFile,CurView->lpFiles[0]);
				ExpandText (ImageFile); 
				GetWindowText (GetDlgItem (hWndDlg,IDC_SHOWONLYCOLOR),txt,32);
				if (!_fstricmp (txt,"Restore"))
				{
					ChangeImageColors (ImageFile,TRUE); 
					SetWindowText (GetDlgItem (hWndDlg,IDC_SHOWONLYCOLOR),"Show Usage");
				}
				else
				{
					SetViewport(*pCommandViewport);
					AddBMPToCache32 (NULL,0);
					_fstrcpy (ImageFile,CurView->lpFiles[0]);  
					ExpandText (ImageFile);
					hDib = BMPHandleFromEXT (ImageFile);
					GetDibPalette (hDib,&NumColors,PaletteRGB);
					for (i=0;i<NumColors;i++)
					{
						colordif = RGBQUADDist  (ColorRGB,PaletteRGB[i]);
						if (colordif <= ChangeDist) 
							PaletteRGB[i] = ColorRGB;
						else
							PaletteRGB[i] = White;
					} 
					SetDibPalette (hDib,NumColors,PaletteRGB); 
					SaveDIB32 (hDib,ImageFile,0,-1);
					DestroyDIB32(hDib,TRUE); 
					SetWindowText (GetDlgItem (hWndDlg,IDC_SHOWONLYCOLOR),"Restore");
				}
				RedisplayWindow ();
            }
            	break;

            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
		         PostMessage(hWndDlg, WM_CLOSE, 0, 0L);
                 break;
           }
         break;  

    default:
        return FALSE;
   }
 return TRUE;
} 


