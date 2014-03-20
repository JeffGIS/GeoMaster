#include "shr.h"
#include "proj.h"       
#include "P_TOL.h"
#include "..\\GEOMASTR\resource.h"

void SetBounds (HWND hWnd,HDC hDC);


#define Sqr(number)(number*number)

#include "bigmemln.h"
#include "bigmem.h"
#include "gmextern.h"

static  Init=FALSE;
static	BOOL FirstCvtCoordErr = TRUE;        
static 	HANDLE	hTranRotation=0;//not being used

void SetVPRotation (double AZ)
{   
	float	RSQMIN;
	double	X1[2]={0,1000},Y1[2]={0,0},X2[2],Y2[2];
	DPOINT	Point1={0,0}, Point2;
	
    CloseTRANS2 (&CurView->hTranBaseToVP);
    CloseTRANS2 (&hTranRotation);
	DisplayCycle++;
	AdjustBoundsAndDrawRectToRotation ();
	SetBounds(CurView->hWnd,CurView->hDC);
	SetPZRControlRotation (AZ);
	return;
    if (AZ)
    {
		Point2 = dnewpt (Point1,AZ,1000);  
		X2[0] = 0;
		X2[1] = Point2.x;
		Y2[0] = 0;
		Y2[1] = Point2.y;
		hTranRotation = STRAN2 (1604,X1,Y1,X2,Y2,2,&RSQMIN,1,NULL);
    } 
	return;
}
  
void FreePROJ(int id)
{
	int i;
	projPJ pjsave;

	if (!PRJ_PROJ4DEF[id])
		return;
	pjsave = PRJ_PROJ4DEF[id];
	PRJ_PROJ4DEF[id] = 0;
	for (i = 0; i < MAX_PROJ; i++)
	{
		if (pjsave == PRJ_PROJ4DEF[i])
			return;
	}
	pj_free(pjsave);
	return;
}

BOOL InitProj4CoordConv (BOOL Delete)
{
	int	id;

	if (Delete)
	{
		for (id = 0; id < MAX_PROJ; id++)
			FreePROJ(id);
		return TRUE;
	}
//	if (!(projdef[1] = pj_init_plus("+proj=merc +lon_0=0 +k=1 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs ")) )
//	if (!(projdef[1] = pj_init_plus("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs")) )

	if (!(PRJ_PROJ4DEF[GOOGLEMAPSPROJECTION] = pj_init_plus("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs")))
       return FALSE;
	strcpy (projid[1],"GoogleMaps");
	if (!(PRJ_PROJ4DEF[LATLONPROJECTION] = pj_init_plus("+proj=latlong +datum=WGS84")))
       return FALSE;
	strcpy (projid[0],"LATLON");
	return TRUE;
}

long ConvertCoordInit (void)
#if ENABLETRACE
{GSSiEnterProg (1333);
#endif
{    long ok, ID; 
     char Name[MAX_PATH];   
     
     if (Init)
{
#if ENABLETRACE
GSSiExitProg (1333);
#endif
     	return 0;
} 
   	 _chdir (CurDir);
  	 _chdrive (SaveDrive);
/*     if(!LoadGCTPLibrary32( ))
     {
     //  GSSiMsgBox(0,"Unable to Open GCTP Library",
     //    "Coordinate Conversion Error",MB_ICONSTOP);
{
#if ENABLETRACE
GSSiExitProg (1333);
#endif
         return -4;
}
     
     }  
     if(!OpenGCTP32(&ok))
     {
    //   GSSiMsgBox(0,"Unable to Open GCTP",
    //     "Coordinate Conversion Error",MB_ICONSTOP);
         CloseGCTPLibrary32();
{
#if ENABLETRACE
GSSiExitProg (1333);
#endif
         return -5;
}
     }       
      
*/
     _fstrcpy(Name, "baseproj");
     ID = 1;
     ok = LoadProjection(ID,Name);
     if (ok != 0)
     {         
//        CloseGCTPLibrary32();
{
#if ENABLETRACE
GSSiExitProg (1333);
#endif
        return ok;
}
     } 
     if (PRJ_UNITS[1] == 4)
     	P_TOL = 0.000000001;
     else
     	P_TOL = 0.0001;
     _fstrcpy(Name,"latlongs");
     ID = 2;
     ok = LoadProjection(ID,Name);
     if (ok != 0)
     {         
//        CloseGCTPLibrary32();
{
#if ENABLETRACE
GSSiExitProg (1333);
#endif
        return ok;
}
     } 
     ID = 3; 
     _fstrcpy(Name,"[%ALT_PROJECTION]");
     ExpandText (Name);
     if (!Name[0] || Name[0] == '[') 
	     _fstrcpy(Name, "baseproj");
     
     ok = LoadProjection(ID,Name);
     if (ok != 0)
     {         
//        CloseGCTPLibrary32();
{
#if ENABLETRACE
GSSiExitProg (1333);
#endif
        return ok;
}
     } 
     Init=TRUE;
{
#if ENABLETRACE
GSSiExitProg (1333);
#endif
    return 0;
}
#if ENABLETRACE
}
#endif
} 

void ConvertUnits (LPDPOINT DPoint, long from, long to)     
#if ENABLETRACE
{GSSiEnterProg (1334);
#endif
{
	if (from == to)
{
#if ENABLETRACE
GSSiExitProg (1334);
#endif
		return; 
}
	if (from == 1 && to == 2)
	{
		DPoint->x *= FTM;
		DPoint->y *= FTM;
	}
	else if (from == 2 && to == 1)
	{
		DPoint->x *= MFT;
		DPoint->y *= MFT;
	}
	else if (from == 4 && to == 5)
	{
		DPoint->x *= 100000;
		DPoint->y *= 100000;
	}
	else if (from == 5 && to == 4)
	{
		DPoint->x /= 100000;
		DPoint->y /= 100000;
	}
{
#if ENABLETRACE
GSSiExitProg (1334);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL NeedToConvertCoord (int from, int to)
{
	if (from == SphericalMercatorPROJECTION)
		return TRUE;
	if (to == SphericalMercatorPROJECTION)
	{
		if (PRJ_TYPE[from] == SphericalMercatorPROJECTION)
			return FALSE;
		return TRUE;
	}
	if (PRJ_UNITS[from] != PRJ_UNITS[to])
		return TRUE;

	if (from == 1 && IS_BASE[to])
   		return FALSE;
    if (to == 1 && IS_BASE[from])
   		return FALSE;
	return TRUE;
}

double ClipCoordToProjection (double val,int xory,int idFrom,int idTo)
{
	double rtn=val;

	if (PRJ_TYPE[idFrom] == LATLONPROJECTION && PRJ_TYPE[idTo] == SphericalMercatorPROJECTION)
	{
		if (xory == 2)
		{
			if (val < -85.05112878)
				rtn = -85.05112878;
			else if (val > 85.05112878)
				rtn = 85.05112878;
		}
	}

	return rtn;
}

long ConvertCoord (LPDPOINT DPoint, int from, int to)
#if ENABLETRACE
{GSSiEnterProg (1335);
#endif
{
TranP TP;
lpTranP lpTP; 
long ok;     
     extern HWND    hWndMain; 
     long	SaveFromUnits;
     long	SaveToUnits;
	 int	saveto = 0;
     
     ok = ConvertCoordInit();
	 if (!ok && !NeedToConvertCoord (from,to))
		 return ok;
	 if (!ok && PRJ_TYPE[from] == PROJ4PROJECTION) // indicates proj4 projection
	 {
		 if (pj_is_latlong(PRJ_PROJ4DEF[from]))
		 {
			 DPoint->x *= DEG_TO_RAD;
			 DPoint->y *= DEG_TO_RAD;
		 }
		 if (PRJ_TYPE[to] == PROJ4PROJECTION)
		 {
			 ok = pj_transform(PRJ_PROJ4DEF[from], PRJ_PROJ4DEF[to], 1, 1, &DPoint->x, &DPoint->y, NULL);
			 {
#if ENABLETRACE
				 GSSiExitProg(1335);
#endif
				 return ok;//unable to properly open the files   
			 }
		 }
		 ok = pj_transform(PRJ_PROJ4DEF[from], PRJ_PROJ4DEF[LATLONPROJECTION], 1, 1, &DPoint->x, &DPoint->y, NULL);
		 DPoint->x *= RAD_TO_DEG;
		 DPoint->y *= RAD_TO_DEG;
		 from = 2;
	 }
     if (ok != 0)
{
#if ENABLETRACE
GSSiExitProg (1335);
#endif
     	return ok;//unable to properly open the files   
}
	 if (PRJ_TYPE[to] == PROJ4PROJECTION)
	 {
		 saveto = to;
		 to = 2;
	 }
     SaveFromUnits=PRJ_UNITS[from];
     SaveToUnits=PRJ_UNITS[to];
     ConvertUnits (DPoint,PRJ_UNITS[from],PRJ_BASEUNITS[from]);
     PRJ_UNITS[from] = PRJ_BASEUNITS[from];
     PRJ_UNITS[to] = PRJ_BASEUNITS[to];
      
     lpTP = &TP;
     ok = TranProjection((long)from, (long)to, &DPoint->x, &DPoint->y);
     if (ok == 22)
     { 
     	DPOINT	TestPoint,TestPoint2, NearPoint;
     	UINT	i=16, pass=0;
     	double	dinc=0.001, fac=2, dist=1, dx, dy, mind=99999999, div=2;
     	
     	while (i--)
     	{   
     		TestPoint=*DPoint;
     		TestPoint.x += dinc;
     		TestPoint.y += dinc;
     		if (!TranProjection((long)from, (long)to, &TestPoint.x, &TestPoint.y))
     			goto GotNearPoint; 
     		fac *= 2;
     		dinc = fac * 0.001;
     	}                         
     	ok = 22;  
     	goto Exit;
GotNearPoint:
		i=16;  
		dx = dy = 0;  
		while (i-- && dist > 0.0001)
		{
			TestPoint.x += dx/div;
			TestPoint.y += dy/div;
			TestPoint2 = TestPoint;  
			if ((ok=TranProjection((long)to, (long)from, &TestPoint2.x, &TestPoint2.y)))	
				goto Exit;
			dist = ldistp (TestPoint2,*DPoint); 
			if (dist < mind)
			{
				mind = dist;
				NearPoint = TestPoint;
			}
			dx = DPoint->x - TestPoint2.x;
			dy = DPoint->y - TestPoint2.y;
		}
		if (pass)
			*DPoint = NearPoint; 
		else
		{
			pass=1;
			div = 8;          
			TestPoint = NearPoint;
			goto GotNearPoint;
		}
		ok = 0;
     }
     ConvertUnits (DPoint,PRJ_UNITS[to],SaveToUnits);
Exit:      
     PRJ_UNITS[from] = SaveFromUnits;
     PRJ_UNITS[to] = SaveToUnits; 
	 if (saveto)
	 {
		 DPoint->x *= DEG_TO_RAD;
		 DPoint->y *= DEG_TO_RAD;
		 ok = pj_transform(PRJ_PROJ4DEF[LATLONPROJECTION], PRJ_PROJ4DEF[saveto], 1, 1, &DPoint->x, &DPoint->y, NULL);
	 }
     *DPoint = TranPoint (DPoint,hTranRotation);
     
{
#if ENABLETRACE
GSSiExitProg (1335);
#endif
     return ok;
}
#if ENABLETRACE
}
#endif
}

BOOL ConvertCoordClose (void)
#if ENABLETRACE
{GSSiEnterProg (1336);
#endif
{   
	short	ID;
	
	for (ID=1;ID<4;ID++)
	{
		if (PRJ_TYPE[ID] == 200)
           PRJ_TYPE[ID] = 0;
        CloseTRANS2 (&PRJ_TRAN[ID][1]); 
        CloseTRANS2 (&PRJ_TRAN[ID][2]); 
    } 

/*    if (Init)
        CloseGCTPLibrary32(); */
    Init=FALSE;
{
#if ENABLETRACE
GSSiExitProg (1336);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}  
   
void ConvertCoordError(long ErrNum)
#if ENABLETRACE
{GSSiEnterProg (1337);
#endif
{ 
 char szString[64];
  if(!FirstCvtCoordErr)
{
#if ENABLETRACE
GSSiExitProg (1337);
#endif
  	return;
}
  FirstCvtCoordErr = FALSE;
 switch (ErrNum)
 {
    case 0:
     break;
    case -1:
       LoadString(hInst, IDS_COORD_NEG1, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case -2:
       LoadString(hInst, IDS_COORD_NEG2, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case -3:
       LoadString(hInst, IDS_COORD_NEG3, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case -4:
       LoadString(hInst, IDS_COORD_NEG4, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case -5:
       LoadString(hInst, IDS_COORD_NEG5, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case -6:
       LoadString(hInst, IDS_COORD_NEG6, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case -7:
       LoadString(hInst, IDS_COORD_NEG7, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case -8:
       LoadString(hInst, IDS_COORD_NEG8, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case 1:
       LoadString(hInst, IDS_COORD1, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case 2:
       LoadString(hInst, IDS_COORD2, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case 3:
       LoadString(hInst, IDS_COORD3, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case 4:
       LoadString(hInst, IDS_COORD4, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case 5:
       LoadString(hInst, IDS_COORD5, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case 6:
       LoadString(hInst, IDS_COORD6, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case 7:
       LoadString(hInst, IDS_COORD7, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case 8:
       LoadString(hInst, IDS_COORD8, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case 9:
       LoadString(hInst, IDS_COORD9, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case 10:
       LoadString(hInst, IDS_COORD10, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case 11:
       LoadString(hInst, IDS_COORD11, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case -27:
       LoadString(hInst, IDS_COORD_NEG27, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case -83:
       LoadString(hInst, IDS_COORD_NEG83, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case -31:
       LoadString(hInst, IDS_COORD_NEG31, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
    case -32:
       LoadString(hInst, IDS_COORD_NEG32, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    break;
 
    default:
       LoadString(hInst, IDS_COORD11, szString, sizeof(szString));
       GSSiMsgBox(NULL, szString, NULL, MB_ICONEXCLAMATION,0);
    
 }//end of the switch 
  
{
#if ENABLETRACE
GSSiExitProg (1337);
#endif
 return;
}
#if ENABLETRACE
}
#endif
}
