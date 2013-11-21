// humbird1.cpp : Defines the entry point for the application.
//
#include	<windows.h>
#include	<winbase.h>
#include	<stdlib.h>
#include	<malloc.h>
#include	<io.h>
#include	<fcntl.h>
#include	<stdio.h>
#include	<string.h>
#include	<wingdi.h>
#include "stdafx.h"
#include "resource.h"
#include "types.h"
#include "lkmhbird.h"
#include "proj_api.h"

static    projPJ pj_google, pj_waypoint, pj_latlon, pj_utm;


#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)

#define MAX_LOADSTRING 100
#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))

typedef struct
   {    double  xmn;
        double  ymn;
        double  xmx;
        double  ymx;
    } MNMXCORD;
typedef MNMXCORD    FAR *LPMNMXCORD; 


HWND	hWndMain;
RECT	rt;
//int CenterX = 272326;
//int CenterY = 5169665;
//int CenterX = 304326;
//int CenterY = 5146865;
int CenterX = 167793;//545788;;
int CenterY = 3836679;//5305846;
//int CenterX = 440178;//218964;//166885;//1094726; //1094726,3187265
//int CenterY = 4968705;//3785503;//3837056;//3187265;
//int	ScreenDim[2]={1280, 1280}; CenterX = 163961;CenterY =  3836098;double Scale= 0.097656;
//int	ScreenDim[2]={1280, 1280}; CenterX = 168262;CenterY =  3836556;double Scale= 0.097656;
//int	ScreenDim[2]={114,114}; 
int	ScreenDim[2]={640,960};
POINT	ScreenCenterPoint;
double Scale=0.25;//0.5;
double ZoomFactor=2;
typedef struct {int	mode;	//0 display selected contour if it exists
							//1 pick new contour, highlight both directions currently on screen in 2 colors
							//3 clear selected contour
				int	pickAperature;
				double pickx;
				double picky;
				int	errcode;
}PICKCONTOURSTRUCT;
typedef PICKCONTOURSTRUCT *LPPICKCONTOURSTRUCT;

CL_BOOL	AdjustScale=TRUE;
extern	BOOL FlipText;
extern	BOOL DoFilter;
static	BOOL HighlightLMLakes;
static	BOOL Seamless=FALSE;
static	int	 DepthOff;
static	int	 HazDepth=5;
static	BOOL ShowHaz=FALSE;
static	int iPilotCreate=0;
static	BOOL displayPointNumbers=FALSE;
double	RangeInMeters;
static	int	PickAp=10;
static	double	trackDistInMiles = 0.5;
static	int	trackSpacing = 10;
static	int	numTrackPoints = 322/10;
static	double offsetInMeters;
static	FTC_RGBQUAD directionColors[2];
static	int lastAz=0;

static	CL_POINT	CrossingPoints[1024];
static	CL_POINT	TrackingPoints[10000];
static	int			TrackingPointDepth[10000];
static	CL_POINT	screenPoints[10000];
int	ii;
int	Shrink=0;
BOOL	displayGrids=FALSE;
BOOL	ShowNext;

extern char	LKMPath[MAX_PATH];

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];								// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];								// The title bar text

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	Choose(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	SelectLake(HWND, UINT, WPARAM, LPARAM);

DPOINT	ContourPickPt;
DPOINT ScreenPointToWorldPoint (int ScreenX, int ScreenY);
double distp(CL_POINT Point1, CL_POINT Point2);

static	LPIPILOTSTRUCT	piPilot[8]={0};
static	int	niPilotStruct=0;
static	int	iip;

int		maxTrack=200;
CL_BOOL	useDash=TRUE;
CL_BOOL showSelectedContour=TRUE;
int		lineWidth=2;
int		nSmoothPass=1;
int		knotRemovalRange=25;
CL_BOOL	doResample=TRUE;
double	trackOffset=0;

CL_POINT captureMousePoint;
DPOINT captureWorldPt;
POINT captureScreenCenterPoint1;
RECT captureWindowRect1;
POINT captureScreenCenterPoint2;
RECT captureWindowRect2;
double	captureScale1;
double	captureScale2;
int	captureCenterX1, captureCenterY1;
int	captureCenterX2, captureCenterY2;
int	iReplayID=0;
int autoZoom=0;
int	pickDepth;

CL_BOOL DisplaySelectionTracks (HDC hDC);
CL_BOOL DisplaySelectedContour (HDC hdc,int i);

DPOINT FTC_POINTtoDPOINT (FTC_POINT point)
{
	DPOINT dpoint;

	dpoint.x = point.x;
	dpoint.y = point.y;
	return dpoint;
}

int iDeltaAZ (int iaz1,int iaz2)
{
	int iDaz;

	double az1 = DEGtoRAD * iaz1, az2 = DEGtoRAD * iaz2;
	iDaz = (int)(DeltaAZ (&az1,&az2) * RADtoDEG);
	return abs(iDaz);
}
BOOL InitCoordConv (int utmZone)
{
	char prjDef[256];

	sprintf (prjDef,"+proj=utm +zone=%i +ellps=WGS84 +datum=WGS84 +units=m +no_defs ",utmZone);
	if (!(pj_google = pj_init_plus("+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs")) )
       return FALSE;
    if (!(pj_latlon = pj_init_plus("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs")) )
       return FALSE;
 //   if (!(pj_utm = pj_init_plus("+proj=utm +zone=15 +ellps=WGS84 +datum=WGS84 +units=m +no_defs ")) )
    if (!(pj_utm = pj_init_plus(prjDef)) )
       return FALSE;
/*        while (scanf("%lf %lf", &x, &y) == 2) {
               x *= DEG_TO_RAD;
               y *= DEG_TO_RAD;
               p = pj_transform(pj_latlong, pj_merc, 1, 1, &x, &y, NULL );
               printf("%.2f\t%.2f\n", x, y);
            }*/
	return TRUE;
}

int RestorePoints (LPDPOINT *Points)
{
	OFSTRUCT OFStruct;
	LPDPOINT	pPoints;
	HFILE fid = OpenFile ("c:\\temp\\savepoints.bin",&OFStruct,OF_READ);
	int	ln = _llseek (fid,0,2);
	int np = ln / sizeof(DPOINT);

	_llseek (fid,0,0);
	pPoints = malloc (ln);
	_lread (fid,pPoints,ln);
	*Points = pPoints;
	return np;
}

void SavePoints (int np,LPDPOINT p)
{
	OFSTRUCT OFStruct;
	HFILE fid = OpenFile ("c:\\temp\\savepoints.bin",&OFStruct,OF_CREATE);

	_lwrite (fid,(LPCCH)p,np*sizeof(DPOINT));
	_lclose (fid);
	return;
}

void sleep (i)
{
	Sleep (i);
	return;
}

void WRITE_TO_ERROR_LOG2(int i)
{
	return;
}
void    WRITE_TO_ERROR_LOG( LPSTR x )
{
	return;
}

unsigned int GetTick (void)
{
 
	static __int64	fVal=0;
	LARGE_INTEGER val;
	
 // return GetTickCount(); 
	QueryPerformanceCounter(&val);
	if (!fVal)
		fVal = val.QuadPart;
	return (unsigned int)(val.QuadPart - fVal);

}


int DisplayCurrentLayer (int ID)
{
	char	txt[128];
return 1;
	sprintf (txt,"Current Layer is %i",ID);
	SetWindowText (hWndMain,txt);
	return 1;
}

DPOINT ScreenToWorld (CL_POINT ScreenPoint)
{
	DPOINT WorldPt;

	WorldPt.x = CenterX - (((rt.right-rt.left)/2 - (ScreenPoint.x-rt.left)) * Scale);
	WorldPt.y = CenterY + (((rt.bottom-rt.top)/2 - (ScreenPoint.y-rt.top)) * Scale);

	return WorldPt;
}

CL_POINT WorldToScreen (LPDPOINT WorldPoint)
{
	CL_POINT ScreenPt;

	ScreenPt.x = ScreenCenterPoint.x - (int)((CenterX - WorldPoint->x)/Scale);
	ScreenPt.y = ScreenCenterPoint.y + (int)((CenterY - WorldPoint->y)/Scale);

	return ScreenPt;
}

HBITMAP GetBitmapFromID (int ID)
{
	HBITMAP hBM;
	char	Name[32];
	
	LKMGetObjectNameFromID (ID,Name);
	hBM = LoadBitmap (hInst,Name);

	return hBM;
}

WORD FAR DIBNumColors(LPSTR lpDIB)
{
   WORD wBitCount;  // DIB bit count

   /*  If this is a Windows-style DIB, the number of colors in the
    *  color table can be less than the number of bits per pixel
    *  allows for (i.e. lpbi->biClrUsed can be set to some value).
    *  If this is the case, return the appropriate value.
    */

   if (IS_WIN30_DIB(lpDIB))
   {
      DWORD dwClrUsed;

      dwClrUsed = ((LPBITMAPINFOHEADER)lpDIB)->biClrUsed;
      if (dwClrUsed)
     return (WORD)dwClrUsed;
   }

   /*  Calculate the number of colors in the color table based on
    *  the number of bits per pixel for the DIB.
    */
   if (IS_WIN30_DIB(lpDIB))
      wBitCount = ((LPBITMAPINFOHEADER)lpDIB)->biBitCount;
   else
      wBitCount = ((LPBITMAPCOREHEADER)lpDIB)->bcBitCount;

   /* return number of colors based on bits per pixel */
   switch (wBitCount)
      {
   case 1:
      return 2;

   case 4:
      return 16;

   case 8:
      return 256;

   default:
      return 0;
      }
}
WORD FAR PaletteSize(LPSTR lpDIB)
{
   return (DIBNumColors(lpDIB) * sizeof(RGBQUAD));
}

LPSTR FAR FindDIBBits(LPSTR lpDIB)
{
   return (lpDIB + *(LPDWORD)lpDIB + PaletteSize(lpDIB));
}


HBITMAP FAR DIBToBitmap(HANDLE hDIB, HPALETTE hPal)
{
   LPSTR lpDIBHdr, lpDIBBits;  // pointer to DIB header, pointer to DIB bits
   HBITMAP hBitmap;            // handle to device-dependent bitmap
   HDC hDC;                    // handle to DC
   HPALETTE hOldPal = NULL;    // handle to a palette

   /* if invalid handle, return NULL */

   if (!hDIB)
      return NULL;

   /* lock memory block and get a pointer to it */
   lpDIBHdr = GlobalLock(hDIB);

   /* get a pointer to the DIB bits */
   lpDIBBits = FindDIBBits(lpDIBHdr);

   /* get a DC */
   hDC = GetDC(NULL);
   if (!hDC)
   {
      /* clean up and return NULL */
      GlobalUnlock(hDIB);
      return NULL;
   }

   /* select and realize palette */
   if (hPal)
      hOldPal = SelectPalette(hDC, hPal, FALSE);
   RealizePalette(hDC);

   /* create bitmap from DIB info. and bits */
   hBitmap = CreateDIBitmap(hDC, (LPBITMAPINFOHEADER)lpDIBHdr, CBM_INIT,
                lpDIBBits, (LPBITMAPINFO)lpDIBHdr, DIB_RGB_COLORS);

   /* restore previous palette */
   if (hOldPal)
      SelectPalette(hDC, hOldPal, FALSE);

   /* clean up */
   ReleaseDC(NULL, hDC);
   GlobalUnlock(hDIB);

   /* return handle to the bitmap */
   return hBitmap;
}

HANDLE ReadDIBFile(int hFile)
{
   BITMAPFILEHEADER bmfHeader;
   DWORD dwBitsSize;
   UINT nNumColors;   // Number of colors in table
   HANDLE hDIB;        
   HANDLE hDIBtmp;    // Used for GlobalRealloc() //MPB
   LPBITMAPINFOHEADER lpbi; 
   DWORD offBits;

   /*
    * get length of DIB in bytes for use when reading
    */

   dwBitsSize = _llseek (hFile,0,2);
   _llseek (hFile,0,0);

   // Allocate memory for header & color table.	We'll enlarge this
   // memory as needed.

   hDIB = GlobalAlloc(GMEM_MOVEABLE,
       (DWORD)(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)));
   
   if (!hDIB) return NULL;

   lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);
   if (!lpbi) 
   {
     GlobalFree(hDIB);
     return NULL;
   }

   // read the BITMAPFILEHEADER from our file

   if (sizeof (BITMAPFILEHEADER) != _lread (hFile, (LPSTR)&bmfHeader, sizeof (BITMAPFILEHEADER)))
     goto ErrExit;

   if (bmfHeader.bfType != 0x4d42)	/* 'BM' */
     goto ErrExit;

   // read the BITMAPINFOHEADER

   if (sizeof(BITMAPINFOHEADER) != _lread (hFile, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER)))
     goto ErrExit;

   // Check to see that it's a Windows DIB -- an OS/2 DIB would cause
   // strange problems with the rest of the DIB API since the fields
   // in the header are different and the color table entries are
   // smaller.
   //
   // If it's not a Windows DIB (e.g. if biSize is wrong), return NULL.

   if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
     goto ErrExit;

   // Now determine the size of the color table and read it.  Since the
   // bitmap bits are offset in the file by bfOffBits, we need to do some
   // special processing here to make sure the bits directly follow
   // the color table (because that's the format we are susposed to pass
   // back)

   if (!(nNumColors = (UINT)lpbi->biClrUsed))
    {
      // no color table for 24-bit, default size otherwise
      if (lpbi->biBitCount != 24)
        nNumColors = 1 << lpbi->biBitCount; /* standard size table */
    }

   // fill in some default values if they are zero
   if (lpbi->biClrUsed == 0)
     lpbi->biClrUsed = nNumColors;

   if (lpbi->biSizeImage == 0)
   {
     lpbi->biSizeImage = ((((lpbi->biWidth * (DWORD)lpbi->biBitCount) + 31) & ~31) >> 3)
			 * lpbi->biHeight;
   }

   // get a proper-sized buffer for header, color table and bits   
   GlobalUnlock(hDIB);
   hDIBtmp = GlobalReAlloc(hDIB, lpbi->biSize +
                        nNumColors * sizeof(RGBQUAD) +
                        lpbi->biSizeImage, GMEM_MOVEABLE);

   if (!hDIBtmp) // can't resize buffer for loading
     goto ErrExitNoUnlock; //MPB
   else
     hDIB = hDIBtmp;

   lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

   // read the color table 
   if (nNumColors)
   		_lread (hFile, (LPSTR)(lpbi) + lpbi->biSize, nNumColors * sizeof(RGBQUAD));

   // offset to the bits from start of DIB header
   offBits = lpbi->biSize + nNumColors * sizeof(RGBQUAD);

   // If the bfOffBits field is non-zero, then the bits might *not* be
   // directly following the color table in the file.  Use the value in
   // bfOffBits to seek the bits.

   if (bmfHeader.bfOffBits != 0L)
      _llseek(hFile, bmfHeader.bfOffBits, SEEK_SET);
   
   if (_lread(hFile, (LPSTR)lpbi + offBits, lpbi->biSizeImage))
     goto OKExit;


ErrExit:
    GlobalUnlock(hDIB);    
ErrExitNoUnlock:    
    GlobalFree(hDIB);
    return NULL;

OKExit:
    GlobalUnlock(hDIB);
    return hDIB;
}


void HBDisplayBitmap (HDC hDC,char * BitmapPathName,int PCTSize,int WorldX,int WorldY)
{
/*	return TRUE;
}

void ProcessChartObject (HDC hDC,HANDLE ObjectHandle)
{*/
	HBITMAP hBitmap, hBMOld;
	HDC		hdcMem;
    BITMAP	bm;
	DPOINT	WorldPt;
	CL_POINT	ScreenPt;
	int		destx,desty;
	int		destw,desth;
	HFILE	hFile;
	OFSTRUCT	ofs;
	HANDLE	hDIB;
	BOOL	rtn;

	//LKMGetNavaidData (ObjectHandle,&WorldPt.x,&WorldPt.y,&BitmapID);
    // Create monochrome (1 bit) mask bitmap.  

	WorldPt.x = WorldX;
	WorldPt.y = WorldY;
	ScreenPt = WorldToScreen (&WorldPt);

    if ((hFile = OpenFile(BitmapPathName, &ofs, OF_READ)) != -1)
    {
      hDIB = ReadDIBFile(hFile);
      _lclose(hFile);  
	}
	else
		return;
//	hPal = CreateDIBPalette (hDIB);
	hBitmap = DIBToBitmap (hDIB,0); 
//	hBitmap = GetBitmapFromID (2);
    GetObject(hBitmap, sizeof(BITMAP), &bm);
	hdcMem = CreateCompatibleDC(hDC);
    hBMOld = SelectObject (hdcMem, hBitmap);

//	BitBlt(hDC, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);


	destw = min (bm.bmWidth,(int)((PCTSize * bm.bmWidth)/100));
	desth = min (bm.bmHeight,(int)((PCTSize * bm.bmHeight)/100));
	PickAp = max (PickAp,destw);
	PickAp = max (PickAp,desth);
	destx = ScreenPt.x - destw/2;
	desty = ScreenPt.y - desth/2;
	rtn = TransparentBlt(hDC, destx,desty,destw,desth, 
				   hdcMem, 0, 0,bm.bmWidth, bm.bmHeight,RGB(255,255,255));

	SelectObject (hdcMem, hBMOld);
	DeleteDC (hdcMem);
	DeleteObject (hBitmap);
	return;
}

BOOL LoadSkin (void)
{
	OFSTRUCT	OFStruct;
	int			nBytes;
	BITMAPFILEHEADER bmfHead;
	BITMAPINFOHEADER bih;
	HFILE		Fid = OpenFile ("Skin.bmp",&OFStruct,OF_READ); 

    if (Fid == HFILE_ERROR)
       	return FALSE;
    _lread (Fid,(HPSTR)&bmfHead,sizeof(BITMAPFILEHEADER));
    nBytes = bmfHead.bfSize-sizeof(BITMAPFILEHEADER);
    _lread (Fid,&bih,sizeof(BITMAPINFOHEADER));
    _lclose (Fid); 
	return TRUE;
}

void DisplaySkin (HDC hDC,LPRECT pRect)
{
    //BITMAP	bm;
	int		destx,desty, rectw, recth;
	int		destw,desth;
	double	bmFac;
	OFSTRUCT	OFStruct;
	int			nBytes;
	LPBYTE	pBits;
	BITMAPFILEHEADER bmfHead;
	BITMAPINFOHEADER bih;
	HFILE		Fid = OpenFile ("Skin.bmp",&OFStruct,OF_READ); 

    if (Fid == HFILE_ERROR)
       	return;
    _lread (Fid,(HPSTR)&bmfHead,sizeof(BITMAPFILEHEADER));
    nBytes = bmfHead.bfSize-sizeof(BITMAPFILEHEADER);
    _lread (Fid,&bih,sizeof(BITMAPINFOHEADER));
	pBits = malloc (bih.biSizeImage);
	_lread (Fid,pBits,bih.biSizeImage);
    _lclose (Fid); 

//	hBitmap = LoadBitmap (hInst,"HBSkin");
//    GetObject(hBitmap, sizeof(BITMAP), &bm);
//	LoadSkin ();
	bmFac = (double)bih.biWidth / (double)bih.biHeight;

	destw = rectw = pRect->right - pRect->left;
	desth = recth = pRect->bottom - pRect->top;
	destx = 0;
	desty = 0;

	if (bmFac * desth > destw)
		desth = (int)(destw / bmFac);
	else
		destw = (int)(desth * bmFac);

	destx = (rectw - destw) / 2;
	desty = (recth - desth) / 2;
//	hdcMem = CreateCompatibleDC(hDC);
//    hBMOld = SelectObject (hdcMem, hBitmap);
	pRect->left = destx + (65 * destw) / bih.biWidth;
	pRect->right = destx + (372 * destw) / bih.biWidth;
	pRect->top = desty + (47 * desth) / bih.biHeight;
	pRect->bottom = desty + (239 * desth) / bih.biHeight;
		
//	TransparentBlt(hDC, destx,desty,destw,desth, 
//				   hdcMem, 0, 0,bm.bmWidth, bm.bmHeight,RGB(255,255,255));

//	StretchBlt(hDC, destx,desty,destw,desth, 
//				   hdcMem, 0, 0,bm.bmWidth, bm.bmHeight,SRCCOPY);
	StretchDIBits (hDC, destx,desty,destw,desth, 
				   0, 0,bih.biWidth, bih.biHeight,
				   pBits,(BITMAPINFO *)&bih,DIB_RGB_COLORS,SRCCOPY);
	free (pBits);
//	SelectObject (hdcMem, hBMOld);
//	DeleteDC (hdcMem);
//	DeleteObject (hBitmap);
	return;
}


void ProcessHighwayShield (HDC hDC,HANDLE ObjectHandle)
{
	HBITMAP hBitmap, hBMOld;
	HDC		hdcMem;
    BITMAP	bm;
	DPOINT	WorldPt;
	CL_POINT	iWorldPt,ScreenPt;
	int		BitmapID;
	int		destx,desty;
	int		destw,desth;

	LKMGetNavaidData (ObjectHandle,&iWorldPt.x,&iWorldPt.y,&BitmapID);
    // Create monochrome (1 bit) mask bitmap.  

	WorldPt = CL_POINTtoDPOINT (iWorldPt);
	ScreenPt = WorldToScreen (&WorldPt);

	hBitmap = GetBitmapFromID (BitmapID);
    GetObject(hBitmap, sizeof(BITMAP), &bm);
	hdcMem = CreateCompatibleDC(hDC);
    hBMOld = SelectObject (hdcMem, hBitmap);

//	BitBlt(hDC, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);


	destw = min (bm.bmWidth,(int)((2 * bm.bmWidth)/Scale));
	desth = min (bm.bmHeight,(int)((2 * bm.bmHeight)/Scale));
	PickAp = max (PickAp,destw);
	PickAp = max (PickAp,desth);
	destx = ScreenPt.x - destw/2;
	desty = ScreenPt.y - desth/2;
	TransparentBlt(hDC, destx,desty,destw,desth, 
				   hdcMem, 0, 0,bm.bmWidth, bm.bmHeight,RGB(255,255,255));

	SelectObject (hdcMem, hBMOld);
	DeleteDC (hdcMem);
	DeleteObject (hBitmap);
	return;
}

void ProcessChartText (HDC hDC,HANDLE ObjectHandle)
{
	char	text[1024];
	DPOINT	WorldPt;
	CL_POINT	iWorldPt, ScreenPt;
	int		nChar;

	nChar = LKMGetChartText (ObjectHandle,4,&iWorldPt.x,&iWorldPt.y, text ,sizeof(text)-1 );
    // Create monochrome (1 bit) mask bitmap.  

	WorldPt = CL_POINTtoDPOINT (iWorldPt);
	ScreenPt = WorldToScreen (&WorldPt);
	SetBkMode (hDC,TRANSPARENT);

	SetTextColor (hDC,RGB(255,255,255));
	TextOut (hDC,ScreenPt.x-1,ScreenPt.y-1,text,nChar);
	TextOut (hDC,ScreenPt.x+1,ScreenPt.y-1,text,nChar);
	TextOut (hDC,ScreenPt.x-1,ScreenPt.y+1,text,nChar);
	TextOut (hDC,ScreenPt.x+1,ScreenPt.y+1,text,nChar);
	SetTextColor (hDC,0);
	TextOut (hDC,ScreenPt.x,ScreenPt.y,text,nChar);
	return;
}

CL_BOOL AbortRoutine (LPVOID pData)
{
	static	int	i=1;

	i++;
	return FALSE;
	if (!(i++%153))
		return TRUE;
	return FALSE;
}

int TestHBird (HDC hDC,int ShowCon,int ShowDepth,int HltDepth,int HltDepthRange)
{
	char	MapName[12];
	static	CL_BOOL	First=TRUE;
	int		rc;
	long	AbortData;
	int		rtn=1;
	char	path[MAX_PATH];
	int		utmZone;
//	HANDLE	enumHandle, ObjectHandle;

//	LKMToHBInit ("c:\\temp\\hbtest_prod\\","1c53565344432020100000010e007c79");413432534434474220223008a200972f
	_fullpath (path,"",MAX_PATH);
	strcat (path,"\\");
	if (First && !(utmZone = LKMToHBInit (path,"413432534434474220223008a200972f",MapName,800)))
	{
		MessageBox (0,"Failed to pass security check",0,MB_ICONEXCLAMATION);
		return 0;
	}
	if (First)
		InitCoordConv(utmZone);
	First = FALSE;
	PickAp = 10;
	//Display The image
	if (LakeMasterToHBirdImage ((HANDLE)hDC,rt.right-rt.left,rt.bottom-rt.top,CenterX,CenterY,&Scale,DepthOff,0,Seamless,0,ShowDepth,ShowCon,HltDepth,HltDepthRange,ShowHaz,HazDepth,AdjustScale,
								AbortRoutine,&AbortData,&rc))
		LKMPostRotationProcessing ((HANDLE)hDC,rt.right-rt.left,rt.bottom-rt.top,CenterX,CenterY,Scale,
								    DepthOff, HighlightLMLakes,
									ShowDepth,ShowCon,HltDepth,HltDepthRange,ShowHaz,HazDepth,
									AbortRoutine,&AbortData);
	//piPilot = NULL;
/*	if (piPilot && piPilot->iOpt == 2)
		return 2;
	if (piPilot && piPilot->iOpt == 4)
		return 4;*/
	//Display chart objects
	return 1;
/*	RangeInMeters = Scale * rt.right;
	if (LKMBeginEnumObjects (CenterX,CenterY,RangeInMeters,Scale, 3,&enumHandle ))
	{
		while(Scale < 20 && LKMEnumNextObject( enumHandle, &ObjectHandle  ))
		{
			ProcessChartObject (hDC,ObjectHandle);
			free (ObjectHandle);
		}
		LKMEndEnumObjects( enumHandle );
	}

	//Display chart text
	if (Scale < 20 && LKMBeginEnumObjects (CenterX,CenterY,RangeInMeters,Scale, 4,&enumHandle ))
	{
		while(LKMEnumNextObject( enumHandle, &ObjectHandle  ))
		{
			ProcessChartText (hDC,ObjectHandle);
			free (ObjectHandle);
		}
		LKMEndEnumObjects( enumHandle );
	}

	//Display highway shields
	if (Scale < 20 && LKMBeginEnumObjects (CenterX,CenterY,RangeInMeters,Scale, 5,&enumHandle ))
	{
		while(LKMEnumNextObject( enumHandle, &ObjectHandle  ))
		{
			ProcessHighwayShield (hDC,ObjectHandle);
			free (ObjectHandle);
		}
		LKMEndEnumObjects( enumHandle );
	}
	return 1;*/
}

MNMXCORD atobounds (LPSTR Value,LPBOOL err)
{                 
	MNMXCORD	Bounds;
	
	if (sscanf (Value,"%Flf %Flf %Flf %Flf",&Bounds.xmn,&Bounds.ymn,&Bounds.xmx,&Bounds.ymx) != 4)  
	{
		Bounds.xmn=Bounds.ymn=Bounds.xmx=Bounds.ymx = 0;
		*err = TRUE;
	}
	else
		*err = FALSE;
	return Bounds;
}

void testarray (int in[5])
{
	in[3] = 321;
	return;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	DPOINT	testCoord, outCoord, outCoord2, outCoord3, testCoord2;
	int		iZone, zone;
	char	hemi;
	double	d;
	char	actualZone[4];

	int tarray[5]={1,2,3,4,5};

/*	testarray (tarray);
	testCoord.y = 45.0;
	testCoord.x = -93.0;
	iZone = 15;
	testCoord.y = 27.3392230;
	testCoord.x = -81.4110020;
	iZone = 17;
	outCoord2 = testCoord;
	outCoord3 = testCoord;

	ii = Convert_Geodetic_To_UTM (testCoord.y,testCoord.x,iZone,&zone,&hemi,&outCoord.x,&outCoord.y);

	InitCoordConv();

//	pj_transform( pj_utm,pj_latlon, 1, 1, &outCoord.x, &outCoord.y, NULL );

	outCoord2.x *= DEGtoRAD;
	outCoord2.y *= DEGtoRAD;

	pj_transform( pj_latlon,pj_utm, 1, 1, &outCoord2.x, &outCoord2.y, NULL );
	ConvertLatLontoUTM (&outCoord3,17,actualZone);
	testCoord2 = outCoord3;
	ConvertUTMtoLatLon (&testCoord2,17);
	d = distpd (&outCoord,&outCoord2);
	d = distpd (&outCoord,&outCoord3);
	outCoord.x /= DEGtoRAD;
	outCoord.y /= DEGtoRAD;
	*/
	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_HUMBIRD1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_HUMBIRD1);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	LKMToHBClose ();
#if _DEBUG
	_CrtDumpMemoryLeaks();
#endif
	return msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_HUMBIRD1);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_HUMBIRD1;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      0,0,ScreenDim[0],ScreenDim[1], NULL, NULL, hInstance, NULL);
//   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
//      0,0,800,600, NULL, NULL, hInstance, NULL);
//   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
//      0,0,1436,1436, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   hWndMain = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
				   InvalidateRect (hWnd,0,1);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR szHello[MAX_LOADSTRING];
	char	text[1024];
	CL_POINT	MousePoint;
	static	int	ShowCon=1, ShowDepth=4, HltDepth=0, HltDepthRange=5;
	char cmd[256];
	OFSTRUCT OFStruct;
	HFILE fid;

	LoadString(hInst, IDS_HELLO, szHello, MAX_LOADSTRING);

	switch (message) 
	{
		case WM_MOUSEMOVE:
			break;	
		case WM_LBUTTONDOWN:
			{
				int	x = HIWORD(lParam);
				int	y = LOWORD(lParam);


				ContourPickPt = ScreenPointToWorldPoint (x, y);

				ii=1;
			}
			break;
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			autoZoom = 0;

			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_SELECTLAKE:
				   ShowNext = FALSE;
				   DialogBox(hInst, (LPCTSTR)IDD_SELECTLAKE, hWnd, (DLGPROC)SelectLake);
				   InvalidateRect (hWnd,0,1);
				   break;
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;
				case ID_CAPTURE:
					{
						int	itim = time(0);
						int	irc;
						
						iReplayID = itim;
						sprintf (cmd,"iPilotCapture\\%i.bin",itim);
						fid = OpenFile (cmd,&OFStruct,OF_CREATE);
						_lwrite (fid,&captureMousePoint,sizeof(captureMousePoint));
						_lwrite (fid,&captureWorldPt,sizeof(captureWorldPt));
						_lwrite (fid,&captureScreenCenterPoint1,sizeof(captureScreenCenterPoint1));
						_lwrite (fid,&captureWindowRect1,sizeof(captureWindowRect1));
						_lwrite (fid,&captureScreenCenterPoint2,sizeof(captureScreenCenterPoint2));
						_lwrite (fid,&captureWindowRect2,sizeof(captureWindowRect2));
						_lwrite (fid,&captureScale1,sizeof(captureScale1));
						_lwrite (fid,&captureScale2,sizeof(captureScale2));
						_lwrite (fid,&captureCenterX1,sizeof(captureCenterX1));
						_lwrite (fid,&captureCenterY1,sizeof(captureCenterY1));
						_lwrite (fid,&captureCenterX2,sizeof(captureCenterX2));
						_lwrite (fid,&captureCenterY2,sizeof(captureCenterY2));

						_lclose (fid);
						sprintf (cmd,"screendump.exe iPilotCapture\\%i.jpg",itim);
						irc = WinExec (cmd,SW_HIDE);
						irc = 0;
					}
					break;

				case ID_REDISPLAY:
					InvalidateRect (hWnd,0,0);
					break;

				case ID_REPLAY:
					{
						sprintf (cmd,"iPilotCapture\\%i.bin",iReplayID);
						fid = OpenFile (cmd,&OFStruct,OF_READ);
						_lread (fid,&captureMousePoint,sizeof(captureMousePoint));
						_lread (fid,&captureWorldPt,sizeof(captureWorldPt));
						_lread (fid,&captureScreenCenterPoint1,sizeof(captureScreenCenterPoint1));
						_lread (fid,&captureWindowRect1,sizeof(captureWindowRect1));
						_lread (fid,&captureScreenCenterPoint2,sizeof(captureScreenCenterPoint2));
						_lread (fid,&captureWindowRect2,sizeof(captureWindowRect2));
						_lread (fid,&captureScale1,sizeof(captureScale1));
						_lread (fid,&captureScale2,sizeof(captureScale2));
						_lread (fid,&captureCenterX1,sizeof(captureCenterX1));
						_lread (fid,&captureCenterY1,sizeof(captureCenterY1));
						_lread (fid,&captureCenterX2,sizeof(captureCenterX2));
						_lread (fid,&captureCenterY2,sizeof(captureCenterY2));
						_lclose (fid);
						ScreenCenterPoint = captureScreenCenterPoint1;
						Scale = captureScale1;
						CenterX = captureCenterX1;
						CenterY = captureCenterY1;
						SetWindowPos (hWnd,0,captureWindowRect1.left,captureWindowRect1.top,RECTWIDTH(&captureWindowRect1),RECTHEIGHT(&captureWindowRect1),SWP_NOZORDER|SWP_SHOWWINDOW);
						InvalidateRect (hWnd,0,1);
					}
					break;
				case ID_RESELECT:
					{
						sprintf (cmd,"iPilotCapture\\%i.bin",iReplayID);
						fid = OpenFile (cmd,&OFStruct,OF_READ);
						_lread (fid,&captureMousePoint,sizeof(captureMousePoint));
						_lread (fid,&captureWorldPt,sizeof(captureWorldPt));
						_lread (fid,&captureScreenCenterPoint1,sizeof(captureScreenCenterPoint1));
						_lread (fid,&captureWindowRect1,sizeof(captureWindowRect1));
						_lread (fid,&captureScreenCenterPoint2,sizeof(captureScreenCenterPoint2));
						_lread (fid,&captureWindowRect2,sizeof(captureWindowRect2));
						_lread (fid,&captureScale1,sizeof(captureScale1));
						_lread (fid,&captureScale2,sizeof(captureScale2));
						_lread (fid,&captureCenterX1,sizeof(captureCenterX1));
						_lread (fid,&captureCenterY1,sizeof(captureCenterY1));
						_lread (fid,&captureCenterX2,sizeof(captureCenterX2));
						_lread (fid,&captureCenterY2,sizeof(captureCenterY2));
						_lclose (fid);
						PostMessage (hWnd,WM_LBUTTONUP,0,MAKELPARAM(captureMousePoint.x,captureMousePoint.y));
					}
					break;
				case ID_OPTIONS_DISPLAYGRID:
					displayGrids = !displayGrids;
					if (displayGrids)
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					else
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_UNCHECKED);
					InvalidateRect (hWnd,0,1);
					break;	
				case ID_NEW_OFFSET:
					piPilot[niPilotStruct++] = LKMiPilotRequestTrack (piPilot[niPilotStruct-1],5,0,0,0,0,offsetInMeters*2,
									knotRemovalRange,nSmoothPass,TRUE);
					offsetInMeters *= 2;
					InvalidateRect (hWnd,0,1);
					break;
				case ID_IPILOT_CONTINUE:
					piPilot[niPilotStruct++] = LKMiPilotRequestTrack (piPilot[niPilotStruct-1],3,0,0,0,0,0,
									knotRemovalRange,nSmoothPass,TRUE);
					//LKMiPilotEnableSelectedContourHighlight (piPilot[niPilotStruct-1],directionColors[1],1);
					InvalidateRect (hWnd,0,1);
					break;
				case ID_IPILOT_REVERSE:
					piPilot[niPilotStruct++] = LKMiPilotRequestTrack (piPilot[niPilotStruct-1],4,0,0,0,0,0,
									knotRemovalRange,nSmoothPass,TRUE);
					//LKMiPilotEnableSelectedContourHighlight (piPilot[niPilotStruct-1],directionColors[1],1);
					InvalidateRect (hWnd,0,1);
					break;
				case ID_OPTIONS_SHOWSELECTEDCONTOUR:
					showSelectedContour = !showSelectedContour;
					if (showSelectedContour)
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					else
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_UNCHECKED);
					break;	
				case ID_TRACKLINELIMIT_1MILE:
					trackDistInMiles = 0.5;
					numTrackPoints = (trackDistInMiles * 5280 * FTM) / trackSpacing;
					CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_TRACKLINELIMIT_2MILES, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_TRACKLINELIMIT_4MILES, MF_BYCOMMAND | MF_UNCHECKED);
					break;
				case ID_TRACKLINELIMIT_2MILES:
					trackDistInMiles = 2;
					numTrackPoints = (trackDistInMiles * 5280 * FTM) / trackSpacing;
					CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_TRACKLINELIMIT_1MILE, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_TRACKLINELIMIT_4MILES, MF_BYCOMMAND | MF_UNCHECKED);
					break;
				case ID_TRACKLINELIMIT_4MILES:
					trackDistInMiles = 4;
					numTrackPoints = (trackDistInMiles * 5280 * FTM) / trackSpacing;
					CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_TRACKLINELIMIT_1MILE, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_TRACKLINELIMIT_2MILES, MF_BYCOMMAND | MF_UNCHECKED);
					break;
				case ID_TRACKLINESPACING_5METERS:
					trackSpacing = 5;
					CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_TRACKLINESPACING_10METERS, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_TRACKLINESPACING_15METERS, MF_BYCOMMAND | MF_UNCHECKED);
					break;
				case ID_TRACKLINESPACING_10METERS:
					trackSpacing = 10;
					CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_TRACKLINESPACING_5METERS, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_TRACKLINESPACING_15METERS, MF_BYCOMMAND | MF_UNCHECKED);
					break;
				case ID_TRACKLINESPACING_15METERS:
					trackSpacing = 15;
					CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_TRACKLINESPACING_5METERS, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_TRACKLINESPACING_10METERS, MF_BYCOMMAND | MF_UNCHECKED);
					break;
				case ID_OPTIONS_USEDASHLINE:
					useDash = !useDash;
					if (useDash)
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					else
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_UNCHECKED);
					break;		
				case ID_OPTIONS_RESAMPLE:
					doResample = !doResample;
					if (doResample)
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					else
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_UNCHECKED);
					break;	
				case ID_OPTIONS_SHOWPOINTS:
					displayPointNumbers = !displayPointNumbers;
					if (displayPointNumbers)
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					else
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_UNCHECKED);
					InvalidateRect (hWnd,0,1);
					break;	
					
				case ID_KNOTREMOVAL_0:
					knotRemovalRange = 0;
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_KNOTREMOVAL_25, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_KNOTREMOVAL_100, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_KNOTREMOVAL_ALL, MF_BYCOMMAND | MF_UNCHECKED);
					break;
				case ID_KNOTREMOVAL_25:
					knotRemovalRange = 25;
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_KNOTREMOVAL_0, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_KNOTREMOVAL_100, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_KNOTREMOVAL_ALL, MF_BYCOMMAND | MF_UNCHECKED);
					break;
				case ID_KNOTREMOVAL_100:
					knotRemovalRange = 100;
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_KNOTREMOVAL_25, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_KNOTREMOVAL_0, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_KNOTREMOVAL_ALL, MF_BYCOMMAND | MF_UNCHECKED);
					break;
				case ID_KNOTREMOVAL_ALL:
					knotRemovalRange = 1000000;
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_KNOTREMOVAL_25, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_KNOTREMOVAL_0, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_KNOTREMOVAL_100, MF_BYCOMMAND | MF_UNCHECKED);
					break;
				case ID_OPTIONS_SMOOTH:
					if (nSmoothPass == 1)
						nSmoothPass--;
					else
						nSmoothPass++;
					if (nSmoothPass == 1)
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					else
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_UNCHECKED);
					break;		
				case ID_OPTIONS_LINEWIDTH2:
					lineWidth = 1;
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_LINEWIDTH3, MF_BYCOMMAND | MF_UNCHECKED);
					break;		
				case ID_OPTIONS_LINEWIDTH3:
					lineWidth = 2;
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_LINEWIDTH2, MF_BYCOMMAND | MF_UNCHECKED);
					break;	
				case ID_OPTIONS_POINTLIMIT2000:
					maxTrack = 2000;
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_POINTLIMIT200, MF_BYCOMMAND | MF_UNCHECKED);
					break;	
				case ID_OPTIONS_POINTLIMIT200:
					maxTrack = 200;
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_POINTLIMIT2000, MF_BYCOMMAND | MF_UNCHECKED);
					break;	
				case ID_OPTIONS_OFFSET0:
					trackOffset = 0;
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET15LEFT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET30LEFT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET25RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET50RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET100RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
					break;	
				case ID_OPTIONS_OFFSET15LEFT:
					trackOffset = -15.0/3.280833;
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET0, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET30LEFT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET25RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET50RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET100RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
					break;	
				case ID_OPTIONS_OFFSET30LEFT:
					trackOffset = -30.0/3.2808333;
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET15LEFT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET25RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET50RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET0, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET100RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
						break;
				case ID_OPTIONS_OFFSET25RIGHT:
					trackOffset = 25.0/3.2808333;
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET15LEFT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET30LEFT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET50RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET0, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET100RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
						break;
				case ID_OPTIONS_OFFSET50RIGHT:
					trackOffset = 50.0/3.2808333;
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET15LEFT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET30LEFT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET25RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET0, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET100RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
					break;	
				case ID_OPTIONS_OFFSET100RIGHT:
					trackOffset = 100.0/3.2808333;
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET15LEFT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET30LEFT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET25RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET0, MF_BYCOMMAND | MF_UNCHECKED);
						CheckMenuItem(GetMenu(hWnd), ID_OPTIONS_OFFSET50RIGHT, MF_BYCOMMAND | MF_UNCHECKED);
					break;	
				case ID_HAZARDDEPTH_3FEET:
					HazDepth=3;
					CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_HAZARDDEPTH_5FEET, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_HAZARDDEPTH_7FEET, MF_BYCOMMAND | MF_UNCHECKED);
					InvalidateRect (hWnd,0,1);
					break;
				case ID_HAZARDDEPTH_5FEET:
					HazDepth=5;
					CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_HAZARDDEPTH_3FEET, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_HAZARDDEPTH_7FEET, MF_BYCOMMAND | MF_UNCHECKED);
					InvalidateRect (hWnd,0,1);
					break;
				case ID_HAZARDDEPTH_7FEET:
					HazDepth=7;
					CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_HAZARDDEPTH_3FEET, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_HAZARDDEPTH_5FEET, MF_BYCOMMAND | MF_UNCHECKED);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_SHOWHAZARD:
					ShowHaz = !ShowHaz;
					if (ShowHaz)
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					else
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_UNCHECKED);
					InvalidateRect (hWnd,0,1);
					break;		

				case IDM_IPILOT:
					if (iPilotCreate)
						iPilotCreate = 0;
					else
						iPilotCreate = 1;
					if (iPilotCreate)
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					else
					{
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_UNCHECKED);
						InvalidateRect (hWnd,0,1);
						for (iip=0;iip<niPilotStruct;iip++)
						{
							LKMiPilotClear (piPilot[iip]);
							piPilot[iip] = 0;
						}
						niPilotStruct = 0;

					}
					CheckMenuItem(GetMenu(hWnd), ID_IPILOT_RESTART, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_IPILOT_RESTARTREVERSE, MF_BYCOMMAND | MF_UNCHECKED);
					break;		
				case ID_IPILOT_RESTART:
					iPilotCreate = 2;
					CheckMenuItem(GetMenu(hWnd), IDM_IPILOT, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_IPILOT_RESTARTREVERSE, MF_BYCOMMAND | MF_UNCHECKED);
					break;		
				case ID_IPILOT_RESTARTREVERSE:
					iPilotCreate = 3;
					CheckMenuItem(GetMenu(hWnd), IDM_IPILOT, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_IPILOT_RESTART, MF_BYCOMMAND | MF_UNCHECKED);
					break;		
				case IDM_SEAMLESS:
					Seamless = !Seamless;
					if (Seamless)
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					else
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_UNCHECKED);
					InvalidateRect (hWnd,0,1);
					break;		

				case IDM_ADJUSTSCALE:
					AdjustScale = !AdjustScale;
					if (AdjustScale)
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					else
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_UNCHECKED);
					InvalidateRect (hWnd,0,1);
					break;		

				case IDM_FLIPTEXT:
					FlipText = !FlipText;
					if (FlipText)
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_CHECKED);
					else
						CheckMenuItem(GetMenu(hWnd), wmId, MF_BYCOMMAND | MF_UNCHECKED);
					InvalidateRect (hWnd,0,1);
					break;		

				case IDM_ZOOMBY2:
					ZoomFactor = 2;
					CheckMenuItem(GetMenu(hWnd), IDM_ZOOMBY2, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_ZOOMBY1P5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_ZOOMBY1P2, MF_BYCOMMAND | MF_UNCHECKED);
					break;		

				case IDM_ZOOMBY1P2:
					ZoomFactor = 1.2;
					CheckMenuItem(GetMenu(hWnd), IDM_ZOOMBY1P2, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_ZOOMBY1P5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_ZOOMBY2, MF_BYCOMMAND | MF_UNCHECKED);
					break;		

				case IDM_ZOOMBY1P5:
					ZoomFactor = 1.5;
					CheckMenuItem(GetMenu(hWnd), IDM_ZOOMBY2, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_ZOOMBY1P5, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_ZOOMBY1P2, MF_BYCOMMAND | MF_UNCHECKED);
					break;		

				case IDM_ZOOM_IN:
					Scale /= ZoomFactor;
					InvalidateRect (hWnd,0,1);
					break;

				case IDM_ZOOM_OUT:
					Scale *= ZoomFactor;
					InvalidateRect (hWnd,0,1);
					break;
				
				case IDM_PAN_LEFT:
					CenterX -= (int)(Scale * rt.right/4);
					InvalidateRect (hWnd,0,1);
					break;
				
				case IDM_PAN_RIGHT:
					CenterX += (int)(Scale * rt.right/4);
					InvalidateRect (hWnd,0,1);
					break;
				
				case IDM_PAN_UP:
					CenterY += (int)(Scale * rt.bottom/4);
					InvalidateRect (hWnd,0,1);
					break;
				
				case IDM_PAN_DOWN:
					CenterY -= (int)(Scale * rt.bottom/4);
					InvalidateRect (hWnd,0,1);
					break;
				
				case IDM_SHOWCON:
					if (ShowCon)
					{
						CheckMenuItem(GetMenu(hWnd), IDM_SHOWCON, MF_BYCOMMAND | MF_UNCHECKED);
						ShowCon = 0;
					}
					else
					{
						CheckMenuItem(GetMenu(hWnd), IDM_SHOWCON, MF_BYCOMMAND | MF_CHECKED);
						ShowCon = 1;
					}
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_DOFILTER:
					if (DoFilter)
					{
						CheckMenuItem(GetMenu(hWnd), IDM_DOFILTER, MF_BYCOMMAND | MF_UNCHECKED);
						DoFilter = 0;
					}
					else
					{
						CheckMenuItem(GetMenu(hWnd), IDM_DOFILTER, MF_BYCOMMAND | MF_CHECKED);
						DoFilter = 1;
					}
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_SHOWDEPTH:
					if (ShowDepth)
					{
						CheckMenuItem(GetMenu(hWnd), IDM_SHOWDEPTH, MF_BYCOMMAND | MF_UNCHECKED);
						ShowDepth = 0;
					}
					else
					{
						CheckMenuItem(GetMenu(hWnd), IDM_SHOWDEPTH, MF_BYCOMMAND | MF_CHECKED);
						ShowDepth = 1;
					}
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_GRAYMAP:
					if (HighlightLMLakes)
					{
						CheckMenuItem(GetMenu(hWnd), IDM_GRAYMAP, MF_BYCOMMAND | MF_UNCHECKED);
						HighlightLMLakes = 0;
					}
					else
					{
						CheckMenuItem(GetMenu(hWnd), IDM_GRAYMAP, MF_BYCOMMAND | MF_CHECKED);
						HighlightLMLakes = 1;
					}
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_COLORFAC0:
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC2, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC4, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC6, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC8, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC1, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC0, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC9, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC10, MF_BYCOMMAND | MF_UNCHECKED);
					ShowDepth = 0;
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_COLORFAC1:
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC2, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC4, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC6, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC8, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC1, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC9, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC10, MF_BYCOMMAND | MF_UNCHECKED);
					ShowDepth = 1;
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_COLORFAC2:
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC1, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC4, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC6, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC8, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC2, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC9, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC10, MF_BYCOMMAND | MF_UNCHECKED);
					ShowDepth = 2;
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_COLORFAC3:
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC1, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC2, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC4, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC6, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC8, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC3, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC9, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC10, MF_BYCOMMAND | MF_UNCHECKED);
					ShowDepth = 3;
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_COLORFAC4:
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC1, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC2, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC6, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC8, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC4, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC9, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC10, MF_BYCOMMAND | MF_UNCHECKED);
					ShowDepth = 4;
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_COLORFAC5:
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC1, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC2, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC4, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC6, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC8, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC5, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC9, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC10, MF_BYCOMMAND | MF_UNCHECKED);
					ShowDepth = 5;
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_COLORFAC6:
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC1, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC2, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC4, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC8, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC6, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC9, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC10, MF_BYCOMMAND | MF_UNCHECKED);
					ShowDepth = 6;
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_COLORFAC7:
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC1, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC2, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC4, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC6, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC8, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC7, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC9, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC10, MF_BYCOMMAND | MF_UNCHECKED);
					ShowDepth = 7;
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_COLORFAC8:
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC1, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC2, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC4, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC6, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC8, MF_BYCOMMAND | MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC9, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC10, MF_BYCOMMAND | MF_UNCHECKED);
					ShowDepth = 8;
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_COLORFAC9:
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC1, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC2, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC4, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC6, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC8, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC10, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC9, MF_BYCOMMAND | MF_CHECKED);
					ShowDepth = 9;
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_COLORFAC10:
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC1, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC2, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC4, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC6, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC8, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC9, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_COLORFAC10, MF_BYCOMMAND | MF_CHECKED);
					ShowDepth = 10;
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_OFFSET5HIGH:
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET1, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET55, MF_BYCOMMAND | MF_UNCHECKED);
					DepthOff = 5;
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET0, MF_BYCOMMAND | MF_CHECKED);
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_OFFSET0:
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET1, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET55, MF_BYCOMMAND | MF_UNCHECKED);
					DepthOff = 0;
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET0, MF_BYCOMMAND | MF_CHECKED);
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_OFFSET1:
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET55, MF_BYCOMMAND | MF_UNCHECKED);
					DepthOff = -1;
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET1, MF_BYCOMMAND | MF_CHECKED);
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_OFFSET3:
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET1, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET55, MF_BYCOMMAND | MF_UNCHECKED);
					DepthOff = -3;
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET3, MF_BYCOMMAND | MF_CHECKED);
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_OFFSET5:
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET1, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET7, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET55, MF_BYCOMMAND | MF_UNCHECKED);
					DepthOff = -5;
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET5, MF_BYCOMMAND | MF_CHECKED);
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_OFFSET7:
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET55, MF_BYCOMMAND | MF_UNCHECKED);
					DepthOff = -7;
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET7, MF_BYCOMMAND | MF_CHECKED);
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_OFFSET55:
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET3, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET5, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET0, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET7, MF_BYCOMMAND | MF_UNCHECKED);
					DepthOff = -55;
					CheckMenuItem(GetMenu(hWnd), IDM_OFFSET7, MF_BYCOMMAND | MF_CHECKED);
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_HIGHLIGHT:
					CheckMenuItem(GetMenu(hWnd), IDM_HIGHLIGHT2, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_HIGHLIGHT3, MF_BYCOMMAND | MF_UNCHECKED);
					if (HltDepth == 25)
					{
						CheckMenuItem(GetMenu(hWnd), IDM_HIGHLIGHT, MF_BYCOMMAND | MF_UNCHECKED);
						HltDepth = 0;
					}
					else
					{
						CheckMenuItem(GetMenu(hWnd), IDM_HIGHLIGHT, MF_BYCOMMAND | MF_CHECKED);
						HltDepth = 25;
						HltDepthRange = 5;
					}
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_HIGHLIGHT2:
					CheckMenuItem(GetMenu(hWnd), IDM_HIGHLIGHT, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_HIGHLIGHT3, MF_BYCOMMAND | MF_UNCHECKED);
					if (HltDepth == 17)
					{
						CheckMenuItem(GetMenu(hWnd), IDM_HIGHLIGHT2, MF_BYCOMMAND | MF_UNCHECKED);
						HltDepth = 0;
					}
					else
					{
						CheckMenuItem(GetMenu(hWnd), IDM_HIGHLIGHT2, MF_BYCOMMAND | MF_CHECKED);
						HltDepth = 17;
						HltDepthRange = 3;
					}
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				case IDM_HIGHLIGHT3:
					CheckMenuItem(GetMenu(hWnd), IDM_HIGHLIGHT, MF_BYCOMMAND | MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), IDM_HIGHLIGHT2, MF_BYCOMMAND | MF_UNCHECKED);
					if (HltDepth == 15)
					{
						CheckMenuItem(GetMenu(hWnd), IDM_HIGHLIGHT3, MF_BYCOMMAND | MF_UNCHECKED);
						HltDepth = 0;
					}
					else
					{
						CheckMenuItem(GetMenu(hWnd), IDM_HIGHLIGHT3, MF_BYCOMMAND | MF_CHECKED);
						HltDepth = 15;
						HltDepthRange = 0;
					}
					DrawMenuBar (hWnd);
					InvalidateRect (hWnd,0,1);
					break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_RBUTTONUP:
			ShowNext = TRUE;
		    DialogBox(hInst, (LPCTSTR)IDD_SELECTLAKE, hWnd, (DLGPROC)SelectLake);
		    InvalidateRect (hWnd,0,0);
			//Shrink+=16;
		//	InvalidateRect (hWnd,0,1);
			break;
		case WM_LBUTTONUP:
			{
 				DPOINT	WorldPt;
				char	pickText[32];
				int azimuthFromStart[2],deeperFlag[2],userPicked[2];
				int	itrack;

   				MousePoint.x = LOWORD(lParam);
				MousePoint.y = HIWORD(lParam);
				WorldPt = ScreenToWorld (MousePoint);
				if (iPilotCreate == 2)
				{
					DPOINT llPoint = WorldPt;
					ConvertUTMtoLatLon (&llPoint,17);
					piPilot[niPilotStruct] = LKMiPilotRequestTrack (piPilot[niPilotStruct-1],6,llPoint.x,llPoint.y,0,0,offsetInMeters,
												knotRemovalRange,nSmoothPass,TRUE);
					for (iip=0;iip<niPilotStruct;iip++)
						LKMiPilotClear (piPilot[iip]);
					piPilot[0] = piPilot[niPilotStruct];
					niPilotStruct = 1;
					//LKMiPilotEnableSelectedContourHighlight (piPilot[niPilotStruct-1],directionColors[1],1);
					InvalidateRect (hWnd,0,1);
				}
				else if (iPilotCreate == 3)
				{
					ConvertUTMtoLatLon (&WorldPt,17);
					piPilot[niPilotStruct] = LKMiPilotRequestTrack (piPilot[niPilotStruct-1],7,WorldPt.x,WorldPt.y,0,0,offsetInMeters,
												knotRemovalRange,nSmoothPass,TRUE);
					for (iip=0;iip<niPilotStruct;iip++)
						LKMiPilotClear (piPilot[iip]);
					piPilot[0] = piPilot[niPilotStruct];
					niPilotStruct = 1;
					//LKMiPilotEnableSelectedContourHighlight (piPilot[niPilotStruct-1],directionColors[1],1);
					InvalidateRect (hWnd,0,1);
				}
				else if (iPilotCreate)
				{
					captureMousePoint = MousePoint;
					captureWorldPt = WorldPt;
					captureScale1 = Scale;
					captureScreenCenterPoint1 = ScreenCenterPoint;
					captureCenterX1 = CenterX;
					captureCenterY1 = CenterY;
					GetWindowRect (hWnd,&captureWindowRect1);
/*					memset (&iPilotStruct,0,sizeof (iPilotStruct));
					iPilotStruct.pickPointD = WorldPt;
					iPilotStruct.iOpt = 1;
					iPilotStruct.pickAperature = 16;
					iPilotStruct.trackPointSpacing = trackSpacing;
					iPilotStruct.maxTrackPoints = numTrackPoints;
					iPilotStruct.directionColors[1].rgbRed = 220;
					iPilotStruct.directionColors[1].rgbGreen = 220;
					iPilotStruct.directionColors[1].rgbBlue= 0;
					iPilotStruct.directionColors[0].rgbReserved = 0;
					iPilotStruct.directionColors[0].rgbRed = 0;
					iPilotStruct.directionColors[0].rgbGreen = 128;
					iPilotStruct.directionColors[0].rgbBlue= 0;
					iPilotStruct.directionColors[1].rgbReserved = 0;
					piPilot =  &iPilotStruct;
					piPilot->pTileCrossingPointArray = CrossingPoints;
					piPilot->lenTileCrossingPointArray = 0;
					piPilot->maxTileCrossingPointArray = 1024;
					piPilot->pTrackArrayOut = TrackingPoints;
					piPilot->pTrackArrayDepth = TrackingPointDepth;
					piPilot->lineWidth = lineWidth;
					piPilot->doDash = useDash;
					piPilot->offsetInMeters = trackOffset;
					piPilot->ReSample = doResample;
					piPilot->showSelectedContour = showSelectedContour;
					piPilot->nSmoothPass = nSmoothPass;
					piPilot->knotRemovalRange = knotRemovalRange;*/
					for (iip=0;iip<niPilotStruct;iip++)
						LKMiPilotClear (piPilot[iip]);
					niPilotStruct = 0;
					SetFTCMaxSelectionTrack (256);
					SetFTCMaxSelectedContour (1024);
					piPilot[0] = (LPIPILOTSTRUCT)LKMiPilotInit (WorldPt.x,WorldPt.y,12,
												numTrackPoints,trackSpacing,
												pickText,&pickDepth,
												azimuthFromStart,deeperFlag,userPicked);
					InvalidateRect (hWnd,0,0);
					if (piPilot[0])
					{
						int	iSelect;
						HDC hdc = GetDC (hWnd);
						
						niPilotStruct = 1;
						if (iDeltaAZ (lastAz,azimuthFromStart[0]) <
							iDeltaAZ (lastAz,azimuthFromStart[1]))
						{
							directionColors[1].rgbRed = 220;
							directionColors[1].rgbGreen = 220;
							directionColors[1].rgbBlue= 0;
							directionColors[1].rgbReserved = 0;
							directionColors[0].rgbRed = 0;
							directionColors[0].rgbGreen = 128;
							directionColors[0].rgbBlue= 0;
							directionColors[0].rgbReserved = 0;
							lastAz = azimuthFromStart[0];
						}
						else
						{
							directionColors[0].rgbRed = 220;
							directionColors[0].rgbGreen = 220;
							directionColors[0].rgbBlue= 0;
							directionColors[0].rgbReserved = 0;
							directionColors[1].rgbRed = 0;
							directionColors[1].rgbGreen = 128;
							directionColors[1].rgbBlue= 0;
							directionColors[1].rgbReserved = 0;
							lastAz = azimuthFromStart[1];
						}
						piPilot[0]->knotRemovalRange = knotRemovalRange;
					//	if (LKMiPilotEnableSelectionImage (piPilot[0],directionColors,lineWidth,TRUE))
					//		InvalidateRect (hWnd,0,0);
						iSelect = DialogBox(hInst, (LPCTSTR)IDD_CHOOSE_DIALOG, hWndMain, (DLGPROC)Choose);
						if (iSelect)
						{
							ClearFTCSelectionContours (piPilot[0]);
							piPilot[1] = LKMiPilotRequestTrack (piPilot[0],iSelect,0,0,0,0,offsetInMeters,
																knotRemovalRange,nSmoothPass,TRUE);
							LKMiPilotClear (piPilot[0]);
							piPilot[0] = piPilot[1];
							//LKMiPilotEnableSelectedContourHighlight (piPilot[0],directionColors[1],1);
							InvalidateRect (hWnd,0,0);
						}
						else
						{
							LKMiPilotClear (piPilot[0]);
							niPilotStruct = 0;
						}
					}
				
				}	
				else
				{
					HANDLE	enumHandle;
					double	RangeInMeters = Scale * (max(10,PickAp)/2 +1);

					SetWindowText (hWnd,"No objects selected");
					LKMBeginEnumObjectsText (WorldPt.x,WorldPt.y,RangeInMeters, Scale,&enumHandle );
					while(LKMEnumNextObjectText( enumHandle, text , 256  ))
					{
						SetWindowText (hWnd,text);
						//MessageBox (hWnd,text,"Returned Text",MB_OK);
					}
					LKMEndEnumObjectsText( enumHandle );
				}
			}
			break;

		case WM_PAINT:
			{
				HRGN hRgn;
				HDC	hdcMem;
				HBITMAP	hNewBM,hbmPrev;
				BITMAP	bm;
				int	rtn;

				hdc = BeginPaint(hWnd, &ps);
				// TODO: Add any drawing code here...
				GetClientRect(hWnd, &rt);
				InflateRect (&rt,-Shrink,-Shrink);
				SelectClipRgn (hdc,0);
				//DisplaySkin (hdc, &rt);
				DisplaySelectionTracks (hdc);
				for (iip=0;iip<niPilotStruct;iip++)
					DisplaySelectedContour (hdc,iip);
				//ScreenDim[0] = RECTWIDTH(&rt);
				//ScreenDim[1] = RECTHEIGHT(&rt);
				hdcMem = CreateCompatibleDC(hdc);
				hNewBM = CreateCompatibleBitmap(hdc,ScreenDim[0],ScreenDim[1]);
			//	hNewBM = CreateCompatibleBitmap(hdc,1436,1436);
			//	hNewBM = CreateCompatibleBitmap(hdc,1024,768);

				GetObject(hNewBM, sizeof(bm), (LPSTR)&bm);
				rt.left = rt.top = 0;
				rt.right = bm.bmWidth;
				rt.bottom = bm.bmHeight;
				hbmPrev = SelectObject(hdcMem, hNewBM);
				hRgn = CreateRectRgn (rt.left,rt.top,rt.right,rt.bottom);
				SelectClipRgn (hdcMem,hRgn);
				DeleteObject (hRgn);
				SetMapMode(hdcMem, MM_TEXT);
				SetWindowOrgEx  ( hdcMem, 0, 0,0 );
				SetViewportOrgEx( hdcMem, rt.left, rt.top,0 );    
				ScreenCenterPoint.x = (rt.right - rt.left)/2;
				ScreenCenterPoint.y = (rt.bottom - rt.top)/2;
				captureScreenCenterPoint2 = ScreenCenterPoint;
				captureCenterX2 = CenterX;
				captureCenterY2 = CenterY;
				GetWindowRect (hWnd,&captureWindowRect2);
				captureScale2 = Scale;
				FillRect (hdc,&rt,GetStockObject (BLACK_BRUSH));
				rtn = TestHBird (hdc/*hdcMem*/,ShowCon,ShowDepth,HltDepth,HltDepthRange);
				if (!IsFTCCompatible())
					EnableMenuItem(GetMenu(hWnd), 3, MF_BYPOSITION | MF_DISABLED);
				DrawMenuBar (hWnd);
				if (rtn)
				{
					//BitBlt(hdc, 0, 0, rt.right-rt.left+1,rt.bottom-rt.top+1,
					//	   hdcMem, rt.left,rt.top, SRCCOPY);
					SelectObject(hdcMem, hbmPrev);

					if (niPilotStruct)
						DisplaySelectionTracks (hdc);

					for (iip=0;iip<niPilotStruct;iip++)
					{
						DisplaySelectedContour (hdc,iip);
						if (piPilot[iip]->lenTrackArrayOut > 0)
						{
							int	i;
							HPEN	hRedPen=CreatePen (PS_SOLID,3,RGB(255,128,128));
							HPEN	hGreenPen=CreatePen (PS_SOLID,3,RGB(0,196,0));
							HPEN	hOldPen = SelectObject (hdc,hRedPen);
							double  totDist = 0;
							char	str[128];
							RECT	bounds;
							HFONT	hFont = CreateFont(8, 0, 0, 0, 0, 
    								0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
							HFONT	OldFont = SelectObject (hdc,hFont);
							DPOINT	lastWorldPt;

							bounds.left = bounds.right = piPilot[iip]->pTrackArrayOut[0].x;
							bounds.top = bounds.bottom = piPilot[iip]->pTrackArrayOut[0].y;
							for (i=0;i<piPilot[iip]->lenTrackArrayOut;i++)
							{
								DPOINT worldPt = CL_POINTtoDPOINT (piPilot[iip]->pTrackArrayOut[i]);
								
								bounds.left = min (bounds.left,piPilot[iip]->pTrackArrayOut[i].x);
								bounds.right = max (bounds.right,piPilot[iip]->pTrackArrayOut[i].x);
								bounds.top = min (bounds.top,piPilot[iip]->pTrackArrayOut[i].y);
								bounds.bottom = max (bounds.bottom,piPilot[iip]->pTrackArrayOut[i].y);
								worldPt.x = DEGtoRAD * (worldPt.x / 1000000);
								worldPt.y = DEGtoRAD * (worldPt.y / 1000000);
								pj_transform(pj_latlon, pj_utm, 1, 1, &worldPt.x, &worldPt.y, NULL );
								if (i)
									totDist += distpd (&lastWorldPt,&worldPt);
								lastWorldPt = worldPt;
								screenPoints[i] = WorldToScreen (&worldPt);
							}
							for (i=0;i<piPilot[iip]->lenTrackArrayOut-1;i++)
							{
								if (piPilot[iip]->pTrackArrayDepth[i]+piPilot[iip]->pTrackArrayDepth[i+1] > HazDepth*20)
									SelectObject (hdc,hGreenPen);
								else
									SelectObject (hdc,hRedPen);
								Polyline (hdc,(POINT *)&screenPoints[i],2);
							}
							for (i=0;i<piPilot[iip]->lenTrackArrayOut;i++)
							{
								itoa (i,str,10);
								if (displayPointNumbers)
									TextOut (hdc,screenPoints[i].x,screenPoints[i].y,str,strlen(str));
							}
							SelectObject (hdc,OldFont);
							DeleteObject (hFont);
							SelectObject (hdc,hOldPen);
							DeleteObject (hRedPen);
							DeleteObject (hGreenPen);
							if (IsFTCTrackContinuous(piPilot[0]))
								sprintf (str,"%i points: %f miles (loop)",piPilot[0]->lenTrackArray,(totDist*3.2808333)/5280);
							else
								sprintf (str,"%i points: %f miles",piPilot[0]->lenTrackArray,(totDist*3.2808333)/5280);
							SetWindowText (hWndMain,str);
							if (autoZoom == 1)
							{
								CenterX = (bounds.left + bounds.right) / 2;
								CenterY = (bounds.top + bounds.bottom) / 2;
								InvalidateRect (hWnd,0,0);
								autoZoom = 0;
							}
						}
					}
						
				}
				DeleteDC(hdcMem);
				DeleteObject (hNewBM);
				EndPaint(hWnd, &ps);
			}
			break;
		case WM_DESTROY:
			for (iip=0;iip<niPilotStruct;iip++)
				LKMiPilotClear (piPilot[iip]);
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Mesage handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char	CardName[16], mess[64];

	switch (message)
	{
		case WM_INITDIALOG:
		//	LKMGetCardName ("c:\\LkMaster\\",CardName,15);
			LKMGetCardName ("",CardName,15);
			SetDlgItemText (hDlg,IDC_CARDNAME,CardName);
			//if (LKMCheckCardCID ("","1c53565344432020100000010e007c79"))
			if (LKMCheckCardCID ("","413432534434474220223008a200972f"))
				sprintf (mess,"CID check OK");
			else
				sprintf (mess,"CID check failed");
			SetDlgItemText (hDlg,IDC_CIDMESSAGE,mess);
			return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}
void GetCompassHeadingFromAZ (LPSTR text,int AZInDegrees)
{
	if (AZInDegrees < 0)
		AZInDegrees = 360 + AZInDegrees;
	AZInDegrees = (AZInDegrees%360) * 10;

	if (AZInDegrees < 225 || AZInDegrees > 3600 - 225)
		strcpy (text,"East");
	else if (AZInDegrees >= 450 - 225 && AZInDegrees < 450 + 225)
		strcpy (text,"NorthEast");
	else if (AZInDegrees >= 900 - 225 && AZInDegrees < 900 + 225)
		strcpy (text,"North");
	else if (AZInDegrees >= 1350 - 225 && AZInDegrees < 1350 + 225)
		strcpy (text,"NorthWest");
	else if (AZInDegrees >= 1800 - 225 && AZInDegrees < 1800 + 225)
		strcpy (text,"West");
	else if (AZInDegrees >= 2250 - 225 && AZInDegrees < 2250 + 225)
		strcpy (text,"SouthWest");
	else if (AZInDegrees >= 2700 - 225 && AZInDegrees < 2700 + 225)
		strcpy (text,"South");
	else
		strcpy (text,"SouthEast");
	return;
}

LRESULT CALLBACK Choose(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	POINT	point;
	RECT	rect;
	char	text[128];
	static	int	iOff = 0;

	switch (message)
	{
		case WM_INITDIALOG:
			GetCursorPos (&point);
			GetWindowRect (hDlg,&rect);
			if (point.y-RECTHEIGHT(&rect)-64 < 16)
				MoveWindow (hDlg,rect.left,point.y+RECTHEIGHT(&rect)+64,RECTWIDTH(&rect),RECTHEIGHT(&rect),TRUE);
			else
				MoveWindow (hDlg,rect.left,point.y-RECTHEIGHT(&rect)-64,RECTWIDTH(&rect),RECTHEIGHT(&rect),TRUE);
			GetCompassHeadingFromAZ (text,piPilot[0]->azimuthFromStart[0]);
			strcat (text," (solid green)");
			SetWindowText (GetDlgItem(hDlg,IDC_DIRECTION1),text);
			GetCompassHeadingFromAZ (text,piPilot[0]->azimuthFromStart[1]);
			strcat (text," (dashed yellow)");
			SetWindowText (GetDlgItem(hDlg,IDC_DIRECTION2),text);
			SetDlgItemInt (hDlg,IDC_OFFSETVAL,abs(iOff),FALSE);
			SetDlgItemInt (hDlg,IDC_NSMOOTH,piPilot[0]->nSmoothPass,FALSE);
			SetDlgItemInt (hDlg,IDC_KNOT,piPilot[0]->knotRemovalRange,FALSE);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_DIRECTION1:
				SendDlgItemMessage (hDlg,IDC_DIRECTION2,BM_SETCHECK,FALSE,0);
				GetCompassHeadingFromAZ (text,piPilot[0]->azimuthFromStart[0]+90);
				if (piPilot[0]->deeperFlag[0] < 0)
					sprintf (strchr (text,0),"-Deeper-Left");
				else
					sprintf (strchr (text,0),"-Shallower-Left");
				SetWindowText (GetDlgItem(hDlg,IDC_OFFSETLEFT),text);
				GetCompassHeadingFromAZ (text,piPilot[0]->azimuthFromStart[0]-90);
				if (piPilot[0]->deeperFlag[1] < 0)
					sprintf (strchr (text,0),"-Deeper-Right");
				else
					sprintf (strchr (text,0),"-Shallower-Right");
				SetWindowText (GetDlgItem(hDlg,IDC_OFFSETRIGHT),text);
				SendDlgItemMessage (hDlg,IDC_DIRECTION2,BM_SETCHECK,FALSE,0);
				SendDlgItemMessage (hDlg,IDC_OFFSETLEFT,BM_SETCHECK,piPilot[0]->userPicked[0]<0,0);
				SendDlgItemMessage (hDlg,IDC_OFFSETRIGHT,BM_SETCHECK,piPilot[0]->userPicked[0]>0,0);
				EnableWindow (GetDlgItem(hDlg,IDOK),TRUE);
				break;
			case IDC_DIRECTION2:
				SendDlgItemMessage (hDlg,IDC_DIRECTION1,BM_SETCHECK,FALSE,0);
				GetCompassHeadingFromAZ (text,piPilot[0]->azimuthFromStart[1]+90);
				if (piPilot[0]->deeperFlag[1] < 0)
					sprintf (strchr (text,0),"-Deeper-Left");
				else
					sprintf (strchr (text,0),"-Shallower-Left");
				SetWindowText (GetDlgItem(hDlg,IDC_OFFSETLEFT),text);
				GetCompassHeadingFromAZ (text,piPilot[0]->azimuthFromStart[1]-90);
				if (piPilot[0]->deeperFlag[1] < 0)
					sprintf (strchr (text,0),"-Shallower-Right");
				else
					sprintf (strchr (text,0),"-Deeper-Right");
				SetWindowText (GetDlgItem(hDlg,IDC_OFFSETRIGHT),text);
				EnableWindow (GetDlgItem(hDlg,IDOK),TRUE);
				SendDlgItemMessage (hDlg,IDC_OFFSETLEFT,BM_SETCHECK,piPilot[0]->userPicked[1]<0,0);
				SendDlgItemMessage (hDlg,IDC_OFFSETRIGHT,BM_SETCHECK,piPilot[0]->userPicked[1]>0,0);
				EnableWindow (GetDlgItem(hDlg,IDOK),TRUE);
				break;
			case IDC_OFFSETLEFT:
				SendDlgItemMessage (hDlg,IDC_OFFSETRIGHT,BM_SETCHECK,FALSE,0);
				break;
			case IDC_OFFSETRIGHT:
				SendDlgItemMessage (hDlg,IDC_OFFSETLEFT,BM_SETCHECK,FALSE,0);
				break;
			case IDOK:
			{
				int	rtn = 1;
				int	err;

				if (SendDlgItemMessage (hDlg,IDC_DIRECTION2,BM_GETCHECK,0,0))
					rtn = 2;
				iOff = GetDlgItemInt (hDlg,IDC_OFFSETVAL,&err,FALSE);
				if (iOff && SendDlgItemMessage (hDlg,IDC_OFFSETLEFT,BM_GETCHECK,0,0))
					iOff = -iOff;
				offsetInMeters = iOff * FTM;
				nSmoothPass = GetDlgItemInt (hDlg,IDC_NSMOOTH,&err,FALSE);
				knotRemovalRange = GetDlgItemInt (hDlg,IDC_KNOT,&err,FALSE);
				EndDialog(hDlg, rtn);
				return TRUE;
			}
			case IDCANCEL:
			{
				EndDialog(hDlg, 0);
				return TRUE;
			}
			break;
			}
	}
    return FALSE;
}
LPSTR fgetss (LPSTR lpStr, short len, FILE *Fid)
{   LPSTR i, r;

    r = fgets (lpStr,len, Fid);
    if (r)
    {
        i = strchr (lpStr,'\r');
        if (i) *i = '\0';
        i = strchr (lpStr,'\n');
        if (i) *i = '\0';
    }
    return (r);
}
// Mesage handler for about box.
LRESULT CALLBACK SelectLake(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static	int	choice=-1;
	char	str[256];
	int		ln, pos;
	int		tabs[2]={2000,3000};
	switch (message)
	{
		case WM_INITDIALOG:
			{
				char	LakeList[MAX_PATH];
				HFILE	Fid;

				SetDlgItemInt (hDlg,IDC_REPLAYID,iReplayID,FALSE);
				SendDlgItemMessage (hDlg,IDC_LIST1,LB_SETTABSTOPS,2,(LPARAM)tabs);
				strcpy (LakeList,LKMPath);
				strcat (LakeList,"lakebounds.txt");
				Fid = _open (LakeList,_O_RDONLY|_O_TEXT);
				pos = 0;
				while (_read(Fid,str,255))
				{
					LPSTR pEnd = strchr (str,'\n');

					if (pEnd)
						*pEnd = 0;
					ln=strlen (str);
					pos += ln+2;
					_lseek (Fid,pos,SEEK_SET);
					SendDlgItemMessage (hDlg,IDC_LIST1,LB_ADDSTRING,0,(LPARAM)str);
				}
				_close (Fid);
				if (ShowNext)
				{
					choice++;
					goto NextItem;
				}
			}
				return TRUE;
		case WM_COMMAND:
			if (LOWORD(wParam) == IDCANCEL)
				EndDialog(hDlg, FALSE);

			else if (LOWORD(wParam) == IDOK) 
			{
				LPSTR	pTab;
				BOOL	err;
				
				choice=SendDlgItemMessage (hDlg,IDC_LIST1,LB_GETCURSEL,0,0);
				
NextItem:				
				err = SendDlgItemMessage (hDlg,IDC_LIST1,LB_GETTEXT,(WPARAM)choice,(LPARAM)str);
				if (err >= 0 && (pTab = strrchr (str,'\t')))
				{
					MNMXCORD bnds;
					double	Scale1,Scale2;

					*pTab++ = 0;
					SetWindowText (hWndMain,str);

					bnds = atobounds (pTab,&err);
					CenterX = (int)(bnds.xmn + bnds.xmx)/2;
					CenterY = (int)(bnds.ymn + bnds.ymx)/2;
					Scale1 = (bnds.xmx - bnds.xmn) / (rt.right - rt.left);
					Scale2 = (bnds.ymx - bnds.ymn) / (rt.bottom - rt.top);
					Scale = max (Scale1,Scale2);
				}
				iReplayID = GetDlgItemInt (hDlg,IDC_REPLAYID,&err,FALSE);
				EndDialog(hDlg, LOWORD(wParam));
			}
			return TRUE;
			break;
	}
    return FALSE;
}

void AppendFile (LPSTR file,LPSTR txt)
{
	OFSTRUCT OFStruct;
	HFILE Fid=OpenFile (file,&OFStruct,OF_EXIST);

	if (Fid != HFILE_ERROR)
		Fid=OpenFile (file,&OFStruct,OF_READWRITE);
	else
		Fid=OpenFile (file,&OFStruct,OF_CREATE);
	_llseek (Fid,0,2);
	_lwrite (Fid,txt,strlen(txt));
	_lwrite (Fid,"\r\n",2);
	_lclose (Fid);
	return;
}

CL_BOOL DisplaySelectionTracks (HDC hdc)
{
	int itrack;
	int numPoints, i;

	if (!niPilotStruct)
		return CL_FALSE;
	if (!GetFTCSelectionTrack (piPilot[0],0,&numPoints) && 
		!GetFTCSelectionTrack (piPilot[1],0,&numPoints))
		return CL_FALSE;
	for (itrack=0;itrack<2;itrack++)
	{
		LPFTC_POINT pTrack = GetFTCSelectionTrack (piPilot[0],itrack,&numPoints);

		if (numPoints > 0)
		{
			HPEN hPen, hOldPen;

			hPen = CreatePen (PS_SOLID,5,RGB(directionColors[itrack].rgbRed,directionColors[itrack].rgbGreen,directionColors[itrack].rgbBlue));

			for (i=0;i<numPoints;i++)
			{
				DPOINT worldPt = FTC_POINTtoDPOINT (pTrack[i]);
				
				worldPt.x = DEGtoRAD * (worldPt.x / 1000000);
				worldPt.y = DEGtoRAD * (worldPt.y / 1000000);
				pj_transform(pj_latlon, pj_utm, 1, 1, &worldPt.x, &worldPt.y, NULL );
				screenPoints[i] = WorldToScreen (&worldPt);
			}
			hOldPen = SelectObject (hdc,hPen);
			Polyline (hdc,(POINT *)screenPoints,numPoints);
			SelectObject (hdc,hOldPen);
			DeleteObject (hPen);
		}
	}

	return CL_TRUE;

}

CL_BOOL DisplaySelectedContour (HDC hdc,int ip)
{
	int itrack;
	int numPoints, i;
	LPFTC_POINT pTrack = GetFTCSelectedContourTrack (piPilot[ip],&numPoints);
	HPEN hPen, hOldPen;

	if (!numPoints)
		return CL_FALSE;

	hPen = CreatePen (PS_SOLID,3,RGB(directionColors[1].rgbRed,directionColors[1].rgbGreen,directionColors[1].rgbBlue));

	for (i=0;i<numPoints;i++)
	{
		DPOINT worldPt = FTC_POINTtoDPOINT (pTrack[i]);
		
		worldPt.x = DEGtoRAD * (worldPt.x / 1000000);
		worldPt.y = DEGtoRAD * (worldPt.y / 1000000);
		pj_transform(pj_latlon, pj_utm, 1, 1, &worldPt.x, &worldPt.y, NULL );
		screenPoints[i] = WorldToScreen (&worldPt);
	}
	hOldPen = SelectObject (hdc,hPen);
	Polyline (hdc,(POINT *)screenPoints,numPoints);
	SelectObject (hdc,hOldPen);
	DeleteObject (hPen);
	
	return CL_TRUE;
}

void  WriteParamToErrorLog(LPSTR mess, int LZOerror )
{
	return;
}
void  WriteInformationToErrorLog(LPSTR mess)
{
	return;
}
