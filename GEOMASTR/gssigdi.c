#include <windows.h> 
#include <limits.h>
#include "TileGraphics.h"

HGLOBAL GSSiGlobAlloc(int From,UINT fuAlloc, long cbAlloc);
HGLOBAL GSSiGlobalReAlloc (USHORT From,HGLOBAL hGlob, long cbAlloc,UINT fuAlloc);
void GSSiGlobFree (LPHANDLE pHandle); 
void GSSiGlobUlFree (LPHANDLE pHandle);    
long BigWrite (HFILE Fid,LPVOID pMF,DWORD isize,long loc);
long BigRead (HFILE Fid,LPVOID pBuf,long isize);
int   WINAPI GSSiSetGraphicsMode(__in HDC hdc, __in int iMode);
int   WINAPI GSSiSetMapMode(__in HDC hdc, __in int iMode);
BOOL  WINAPI GSSiSetViewportExtEx( __in HDC hdc, __in int x, __in int y, __out_opt LPSIZE lpsz);
BOOL  WINAPI GSSiSetViewportOrgEx( __in HDC hdc, __in int x, __in int y, __out_opt LPPOINT lppt);
BOOL  WINAPI GSSiSetWindowExtEx( __in HDC hdc, __in int x, __in int y, __out_opt LPSIZE lpsz);
BOOL  WINAPI GSSiSetWindowOrgEx( __in HDC hdc, __in int x, __in int y, __out_opt LPPOINT lppt);
BOOL WINAPI GSSiSetWorldTransform( __in HDC hdc, __in CONST XFORM * lpxf);
void SetSavedGraphicsFid (int Type);
void SaveTileGraphics (HFILE Fid,HDC hDC,int type,LPPOINT points,int np);


#if CHECKMEM    
void MEMERR(LPSTR Mess);

#define PostMessageA GSSiPOSTMESSAGE 
BOOL    WINAPI GSSiPOSTMESSAGE(HWND, UINT, WPARAM, LPARAM);
#define	GlobalLock	GSSiGLOBALLOCK
void LogMemAlloc (unsigned short MemID,long MemLen);
LPVOID GSSiGLOBALLOCK (HANDLE hglb);
#define	GlobalSize	GSSiGLOBALSIZE
DWORD GSSiGLOBALSIZE (HANDLE hglb);
#define	GlobalUnlock	GSSiGLOBALUNLOCK
BOOL GSSiGLOBALUNLOCK (HANDLE hglb);
#define	GlobalAlloc	GSSiGLOBALALLOC
HGLOBAL GSSiGLOBALALLOC(UINT fuAlloc, DWORD cbAlloc);
#define	GlobalFree	GSSiGLOBALFREE
HGLOBAL GSSiGLOBALFREE (HANDLE hglb);
#define GlobalReAlloc GSSiGLOBALREALLOC  
HGLOBAL WINAPI GSSiGLOBALREALLOC (HGLOBAL hglb, DWORD cbAlloc, UINT fuAlloc);  
void GSSiRemoveMem (HGLOBAL hglb);  

#endif

extern HFILE SavedGraphicsFid;

#define	MAXLAST	1024    
#define MAXTRACK	4096
#define MAXTRACKTYPE	18

static	short	SGid;
#define SG_POLYLINE			1
#define SG_LOGPEN			2
#define SG_EXTLOGPEN		3
#define SG_BITBLT			4
#define SG_SETBRUSHORDEX	5
#define SG_SETSTRETCHBLTMODE	6
#define SG_SETTEXTCOLOR		7
#define SG_STRETCHBLT1		8
#define SG_STRETCHBLT2		9
#define SG_SETGRAPHICSMODE		10
#define SG_SETMAPMODE		11
#define SG_SETVPEXTEX		12
#define SG_SETVPORGEX		13
#define SG_SETWINEXTEX		14
#define SG_SETWINORGEX		15
#define SG_SETWORLDTRANSFORM	16
#define SG_LOGBRUSH			17
#define SG_POLYGON			18

typedef struct {int Style;
				int	Width;
				COLORREF Color;} CREATEPENSTRUCT;
static	CREATEPENSTRUCT	CreatePenStruct;

extern	BOOL	InDebug;
extern	HWND	hWndMain;
extern	LPSTR	CurrentPrefix;
extern	LPSTR	CurrentUDI;
extern	BOOL	CaptureIndexGraphics;
extern	char	CIGPrefix[10];
extern	HFILE	FidCIG;
extern	HFILE	FidCIGIndex;

static	UINT	nLast=0;
static	HGDIOBJ	LastObj[MAXLAST];
static	UINT	nLastd=0;
static	HGDIOBJ	LastObjd[MAXLAST];
static	HGDIOBJ TrackObj[MAXTRACK]; 
static	BYTE	TrackObjType[MAXTRACK];
static	int		TrackObjCount[MAXTRACK];
static	int		NextObjectCount=0;
static	BOOL	FirstTrack=TRUE;
static	char	TrackTypeName[MAXTRACKTYPE][20] = {"Pen","Solid Brush","Hatch Brush","Pat Brush","Ind Brush","Load Bitmap",
													"Create Font","Create FontInd","Create RectRgn","Create RectRgnInd",
													"Create PolygonRgn","Create Bitmap","Create CompatBitmap","Create DIBitmap",
													"Create Palette","Create EllipticRgn","Create DIBSection","CreateRoundRectRgn"};
static	int		CurSGFType=0;
static	HFILE	SavedGraphicsFids[7];
static	char	SavedGraphicsFileName[7][MAX_PATH];
static	HDC		hDCSG=0;
static	short	ln2;
static	LOGPEN	LogPen, CurLogPen[7];
static	EXTLOGPEN	ExtLogPen;
static	LOGBRUSH	LogBrush, CurLogBrush[7];
static	HPEN	CurPen[7];
static	HBRUSH	CurBrush[7];
static	int		CurGraphicsMode=-1;
static	int		CurMapMode=-1;
static	int		CurVPExtEx_x, CurVPExtEx_y;
static	int		CurVPOrgEx_x, CurVPOrgEx_y;
static	int		CurWINExtEx_x, CurWINExtEx_y;
static	int		CurWINOrgEx_x, CurWINOrgEx_y;
static	HFILE	FidSTG=HFILE_ERROR;
static	BOOL	wantnextbltblt = FALSE;

#pragma pack(2)
static	struct {short opt;
				int	xDest,yDest,wDest,hDest;
				int xSrc,ySrc,wSrc,hSrc;
				DWORD	rop;
				BITMAP	bm;
				}BitBltStruct;
#pragma pack()

void InitTileGraphics (HFILE Fid)
{
	FidSTG = Fid;
	return;
}

void ResetObjectCount (void)
{
	NextObjectCount=0;
	return;
}

void SetSavedGraphicsDC (HDC hDC)
{
	hDCSG = hDC;
	return;
}

void DestroySavedGraphicsFile (int Type)
{
	SetSavedGraphicsFid (0);
	if (Type && SavedGraphicsFids[Type-1] != HFILE_ERROR)
	{
		GSSiClose (SavedGraphicsFids[Type-1]);
		SavedGraphicsFids[Type-1] = HFILE_ERROR;
		GSSiRemoveAndClear (SavedGraphicsFileName[Type-1]);
	}
	else if (!Type)
	{
		int	i;

		for (i=0;i<7;i++)
		{
			if (SavedGraphicsFids[i] != HFILE_ERROR)
			{
				GSSiClose (SavedGraphicsFids[i]);
				SavedGraphicsFids[i] = HFILE_ERROR;
				GSSiRemoveAndClear (SavedGraphicsFileName[i]);
			}
		}
	}
	return;
}

void CreateSavedGraphicsFiles (int Type)
{
	GSSiGetTempFileName(0,"gms",0,SavedGraphicsFileName[Type]);
	SavedGraphicsFids[Type] = GSSiOpenFile (SavedGraphicsFileName[Type],0,OF_CREATE);
	memset (&CurLogPen[Type],0,sizeof(LOGPEN));
	memset (&CurLogBrush[Type],0,sizeof(LOGBRUSH));
	CurPen[Type]=0;
	CurBrush[Type]=0;
	return;
}

void DisplaySavedGraphicsFile (HDC hDC,int Type)
{
	int npt;
	HANDLE	hpt;
	LPPOINT	ppt;
	HPEN	hPen, hPen2, hRestorePen=0;
	HPEN	hBrush, hBrush2, hRestoreBrush=0;
	COLORREF	color;
	int		mode;
	int	x,y;
	int	PrevSGid=-1;
	XFORM	xf;

	if (SavedGraphicsFids[Type-1] != HFILE_ERROR)
	{
		HFILE	Fid = SavedGraphicsFids[Type-1];

		SaveDC (hDC);
		GSSillseek (Fid,0,0);
		while (BigRead (Fid,&SGid,2) == 2)
		{
			switch (SGid)
			{
			case SG_POLYLINE:
				BigRead (Fid,&npt,4);
				if (npt)
				{
					hpt = GSSiGlobAlloc (0,GMEM_MOVEABLE,npt*sizeof(POINT));
					ppt = (LPPOINT)GlobalLock (hpt);
					BigRead (Fid,ppt,npt*sizeof(POINT));
					Polyline (hDC,ppt,npt);
					GSSiGlobUlFree (&hpt);
				}
				break;
			case SG_POLYGON:
				BigRead (Fid,&npt,4);
				if (npt)
				{
					hpt = GSSiGlobAlloc (0,GMEM_MOVEABLE,npt*sizeof(POINT));
					ppt = (LPPOINT)GlobalLock (hpt);
					BigRead (Fid,ppt,npt*sizeof(POINT));
					Polygon (hDC,ppt,npt);
					GSSiGlobUlFree (&hpt);
				}
				break;
			case SG_LOGPEN:
				BigRead (Fid,&ln2,2);
				BigRead (Fid,&LogPen,sizeof(LOGPEN));
				hPen = CreatePenIndirect(&LogPen);
				hPen2 = SelectObject (hDC,hPen);
				if (!hRestorePen)
					hRestorePen = hPen2;
				else
					DeleteObject (hPen2);
				break;
			case SG_LOGBRUSH:
				BigRead (Fid,&ln2,2);
				BigRead (Fid,&LogBrush,sizeof(LOGBRUSH));
				hBrush = CreateBrushIndirect(&LogBrush);
				hBrush2 = SelectObject (hDC,hBrush);
				if (!hRestoreBrush)
					hRestoreBrush = hBrush2;
				else
					DeleteObject (hBrush2);
				break;
			case SG_EXTLOGPEN:
				BigRead (Fid,&ln2,2);
				BigRead (Fid,&ExtLogPen,ln2);
				hPen = ExtCreatePen(ExtLogPen.elpPenStyle,
									ExtLogPen.elpWidth,
									&LogBrush,
									ExtLogPen.elpNumEntries,
									ExtLogPen.elpStyleEntry);
				hPen2 = SelectObject (hDC,hPen);
				if (!hRestorePen)
					hRestorePen = hPen2;
				else
					DeleteObject (hPen2);
				break;
			case SG_SETTEXTCOLOR:
				BigRead (Fid,&color,sizeof(COLORREF));
				SetTextColor (hDC,color);
				break;
			case SG_SETSTRETCHBLTMODE:
				BigRead (Fid,&mode,sizeof(int));
				SetStretchBltMode(hDC,mode);
				break;
			case SG_SETBRUSHORDEX:
				BigRead (Fid,&x,sizeof(int));
				BigRead (Fid,&y,sizeof(int));
				SetBrushOrgEx( hDC, x, y,0);
				break;
			case SG_STRETCHBLT1:
				{
					BITMAPINFO	bmi;
					DWORD	flag;
					HBITMAP	hBMP;
					int	lnBits;
					HANDLE	hBits;
					LPBYTE	pBits;

					BigRead (Fid,&BitBltStruct.xDest,sizeof(BitBltStruct)-2);
					lnBits = BitBltStruct.bm.bmWidthBytes*BitBltStruct.bm.bmHeight;
					hBits = GSSiGlobAlloc (0,GMEM_MOVEABLE,lnBits);
					pBits = GlobalLock (hBits);
					BigRead (Fid,pBits,lnBits);
					memset (&bmi,0,sizeof(BITMAPINFO));
					bmi.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
					bmi.bmiHeader.biWidth = BitBltStruct.wDest;
					bmi.bmiHeader.biHeight = BitBltStruct.hDest;
					bmi.bmiHeader.biPlanes = BitBltStruct.bm.bmPlanes;
					bmi.bmiHeader.biBitCount = BitBltStruct.bm.bmBitsPixel;
					StretchDIBits(hDC,
                               BitBltStruct.xDest,
                               BitBltStruct.yDest+BitBltStruct.hDest,
                               BitBltStruct.wDest,
                               -BitBltStruct.hDest,
							   0,0,
                               BitBltStruct.wDest,
                               BitBltStruct.hDest,
                               pBits,
                               &bmi,
                               DIB_RGB_COLORS, 
                               BitBltStruct.rop); 
					GSSiGlobUlFree (&hBits);

				}
				break;
			case SG_STRETCHBLT2:
				{
					BITMAPINFO	bmi;
					DWORD	flag;
					HBITMAP	hBMP;
					int	lnBits;
					HANDLE	hBits;
					LPBYTE	pBits;

					BigRead (Fid,&BitBltStruct.xDest,sizeof(BitBltStruct)-2);
					lnBits = BitBltStruct.bm.bmWidthBytes*BitBltStruct.bm.bmHeight;
					hBits = GSSiGlobAlloc (0,GMEM_MOVEABLE,lnBits);
					pBits = GlobalLock (hBits);
					BigRead (Fid,pBits,lnBits);
					memset (&bmi,0,sizeof(BITMAPINFO));
					bmi.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
					bmi.bmiHeader.biWidth = BitBltStruct.wSrc;
					bmi.bmiHeader.biHeight = BitBltStruct.hSrc;
					bmi.bmiHeader.biPlanes = BitBltStruct.bm.bmPlanes;
					bmi.bmiHeader.biBitCount = BitBltStruct.bm.bmBitsPixel;
					StretchDIBits(hDC,
                               BitBltStruct.xDest,
                               BitBltStruct.yDest+BitBltStruct.hDest,
                               BitBltStruct.wDest,
                               -BitBltStruct.hDest,
							   0,0,
                               BitBltStruct.wSrc,
                               BitBltStruct.hSrc,
                               pBits,
                               &bmi,
                               DIB_RGB_COLORS, 
                               BitBltStruct.rop); 
					GSSiGlobUlFree (&hBits);
				}
				break;
			case SG_SETGRAPHICSMODE:
				BigRead (Fid,&mode,sizeof(int));
				SetGraphicsMode (hDC,mode);
				break;
			case SG_SETMAPMODE:
				BigRead (Fid,&mode,sizeof(int));
				SetMapMode (hDC,mode);
				break;
			case SG_SETVPEXTEX:
				BigRead (Fid,&x,sizeof(int));
				BigRead (Fid,&y,sizeof(int));
				SetViewportExtEx (hDC,x,y,0);
				break;
			case SG_SETVPORGEX:
				BigRead (Fid,&x,sizeof(int));
				BigRead (Fid,&y,sizeof(int));
				SetViewportOrgEx (hDC,x,y,0);
				break;
			case SG_SETWINEXTEX:
				BigRead (Fid,&x,sizeof(int));
				BigRead (Fid,&y,sizeof(int));
				SetWindowExtEx (hDC,x,y,0);
				break;
			case SG_SETWINORGEX:
				BigRead (Fid,&x,sizeof(int));
				BigRead (Fid,&y,sizeof(int));
				SetWindowOrgEx (hDC,x,y,0);
				break;
			case SG_SETWORLDTRANSFORM:
				BigRead (Fid,&xf,sizeof(XFORM));
				SetWorldTransform (hDC,&xf);
				break;
			default:
				MessageBox (0,"bad code",0,MB_ICONEXCLAMATION);
				break;

			}
			PrevSGid = SGid;
		}
		if (hRestorePen)
		{
			hPen2 = SelectObject (hDC,hRestorePen);
			DeleteObject (hPen2);
		}
		if (hRestoreBrush)
		{
			hBrush2 = SelectObject (hDC,hRestoreBrush);
			DeleteObject (hBrush2);
		}
		RestoreDC (hDC,-1);
		DestroySavedGraphicsFile (Type);
	}
	return;
}
void SetSavedGraphicsFid (int Type)
{
	static	BOOL	First=TRUE;
	static	int		CurrentType=0;
	int		i;
	int		mode, x, y;
	SIZE	size;
	XFORM	xf;
	POINT	pt;

	if (First)
	{
		First = FALSE;
		for (i=0;i<7;i++)
			SavedGraphicsFids[i] = HFILE_ERROR;
	}

	if (Type == CurrentType)
		return;
	if (Type)
	{
		CurSGFType = Type - 1;
		if (SavedGraphicsFids[CurSGFType] == HFILE_ERROR)
			CreateSavedGraphicsFiles(CurSGFType);
		SavedGraphicsFid = SavedGraphicsFids[CurSGFType];
		CurGraphicsMode = -1;
		CurMapMode = -1;
		CurVPExtEx_x = CurVPExtEx_y = INT_MAX;
		CurWINExtEx_x = CurWINExtEx_y = INT_MAX;
		CurVPOrgEx_x = CurVPOrgEx_y = INT_MAX;
		CurWINOrgEx_x = CurWINOrgEx_y = INT_MAX;
		mode = GetMapMode (hDCSG);
		GSSiSetMapMode (hDCSG,mode);
		mode = GetGraphicsMode (hDCSG);
		GSSiSetGraphicsMode (hDCSG,mode);
		GetViewportExtEx (hDCSG,&size);
		GSSiSetViewportExtEx (hDCSG,size.cx,size.cy,0);
		GetViewportOrgEx (hDCSG,&pt);
		GSSiSetViewportOrgEx (hDCSG,pt.x,pt.y,0);
		GetWindowExtEx (hDCSG,&size);
		GSSiSetWindowExtEx (hDCSG,size.cx,size.cy,0);
		GetWindowOrgEx (hDCSG,&pt);
		GSSiSetWindowOrgEx (hDCSG,pt.x,pt.y,0);
		GetWorldTransform (hDCSG,&xf);
		GSSiSetWorldTransform(hDCSG,&xf);

	}
	else
		SavedGraphicsFid = HFILE_ERROR;
	CurrentType = Type;
	return;
}


void TrackObject (HGDIOBJ hObj,short Type)
{   
	unsigned short	i,ii;
	
	if (!InDebug)
		return;
	if (FirstTrack)
	{
		FirstTrack = FALSE;
		memset (TrackObj,0,sizeof(TrackObj));
	}
	if (Type == -100)
	{   
		char	mess[64];
        short	n, itype;
        
        for (itype=0;itype < MAXTRACKTYPE;itype++)
        {
        	n=0;
			for (i=0;i<MAXTRACK;i++)
			{   
				if (TrackObj[i] && TrackObjType[i] == itype+1)
				{
					n++;
					ii=TrackObjCount[i];
				}
			}
			if (n)
			{
				sprintf (mess,"%i %s not deleted",n,TrackTypeName[itype]);
				MessageBox (0,mess,NULL,MB_ICONEXCLAMATION);
			}
		} 
		return;
	}
	if (Type > 0)
	{
		for (i=0;i<MAXTRACK;i++)
		{   
			if (!TrackObj[i])
			{
				TrackObj[i] = hObj;
				TrackObjType[i] = Type; 
				TrackObjCount[i] = NextObjectCount++; 
				if (TrackObjCount[i] == 9401 || TrackObjCount[i] == 9611)
					ii=1;
				if (i==49 && Type == 8)
					ii=1;
				return;
			}
		} 
		MEMERR ("Tracking Limit");
	}
	else
	{
		for (i=0;i<MAXTRACK;i++)
		{   
			if (TrackObj[i] == hObj)
			{
				TrackObj[i] = 0;
				TrackObjType[i] = 0;
				TrackObjCount[i] = 0;
				return;
			}
		} 
		MEMERR ("Deleting Invalid Object");
	}
	return;
}

COLORREF WINAPI GSSiSetTextColor(__in HDC hdc, __in COLORREF color)
{
	if (hdc == hDCSG && SavedGraphicsFid != HFILE_ERROR)
	{
		SGid = SG_SETTEXTCOLOR;
		BigWrite (SavedGraphicsFid,&SGid,2,-1);
		BigWrite (SavedGraphicsFid,&color,sizeof(COLORREF),-1);
	}
	return SetTextColor (hdc,color);
}

int   WINAPI GSSiSetStretchBltMode(__in HDC hdc, __in int mode)
{
	if (hdc == hDCSG && SavedGraphicsFid != HFILE_ERROR)
	{
		SGid = SG_SETSTRETCHBLTMODE;
		BigWrite (SavedGraphicsFid,&SGid,2,-1);
		BigWrite (SavedGraphicsFid,&mode,sizeof(int),-1);
	}
	return SetStretchBltMode(hdc,mode);
}
BOOL  WINAPI GSSiSetBrushOrgEx( __in HDC hdc, __in int x, __in int y, __out_opt LPPOINT lppt)
{
	if (hdc == hDCSG && SavedGraphicsFid != HFILE_ERROR)
	{
		SGid = SG_SETBRUSHORDEX;
		BigWrite (SavedGraphicsFid,&SGid,2,-1);
		BigWrite (SavedGraphicsFid,&x,sizeof(int),-1);
		BigWrite (SavedGraphicsFid,&y,sizeof(int),-1);
	}
	return SetBrushOrgEx( hdc, x, y,lppt);
}
int   WINAPI GSSiSetGraphicsMode(__in HDC hdc, __in int iMode)
{
	if (hdc == hDCSG && SavedGraphicsFid != HFILE_ERROR && iMode != CurGraphicsMode)
	{
		CurGraphicsMode = iMode;
		SGid = SG_SETGRAPHICSMODE;
		BigWrite (SavedGraphicsFid,&SGid,2,-1);
		BigWrite (SavedGraphicsFid,&iMode,sizeof(int),-1);
	}
	return SetGraphicsMode (hdc,iMode);
}
int   WINAPI GSSiSetMapMode(__in HDC hdc, __in int iMode)
{
	if (hdc == hDCSG && SavedGraphicsFid != HFILE_ERROR && iMode != CurMapMode)
	{
		CurMapMode = iMode;
		SGid = SG_SETMAPMODE;
		BigWrite (SavedGraphicsFid,&SGid,2,-1);
		BigWrite (SavedGraphicsFid,&iMode,sizeof(int),-1);
	}
	return SetMapMode (hdc,iMode);
}
BOOL  WINAPI GSSiSetViewportExtEx( __in HDC hdc, __in int x, __in int y, __out_opt LPSIZE lpsz)
{
	if (hdc == hDCSG && SavedGraphicsFid != HFILE_ERROR && (x != CurVPExtEx_x || y != CurVPExtEx_y))
	{
		CurVPExtEx_x = x;
		CurVPExtEx_y = y;
		SGid = SG_SETVPEXTEX;
		BigWrite (SavedGraphicsFid,&SGid,2,-1);
		BigWrite (SavedGraphicsFid,&x,sizeof(int),-1);
		BigWrite (SavedGraphicsFid,&y,sizeof(int),-1);
	}
	return SetViewportExtEx (hdc,x,y,lpsz);
}
BOOL  WINAPI GSSiSetViewportOrgEx( __in HDC hdc, __in int x, __in int y, __out_opt LPPOINT lppt)
{
	if (hdc == hDCSG && SavedGraphicsFid != HFILE_ERROR && (x != CurVPOrgEx_x || y != CurVPOrgEx_y))
	{
		CurVPOrgEx_x = x;
		CurVPOrgEx_y = y;
		SGid = SG_SETVPORGEX;
		BigWrite (SavedGraphicsFid,&SGid,2,-1);
		BigWrite (SavedGraphicsFid,&x,sizeof(int),-1);
		BigWrite (SavedGraphicsFid,&y,sizeof(int),-1);
	}
	return SetViewportOrgEx (hdc,x,y,lppt);
}
BOOL  WINAPI GSSiSetWindowExtEx( __in HDC hdc, __in int x, __in int y, __out_opt LPSIZE lpsz)
{
	if (hdc == hDCSG && SavedGraphicsFid != HFILE_ERROR && (x != CurWINExtEx_x || y != CurWINExtEx_y))
	{
		CurWINExtEx_x = x;
		CurWINExtEx_y = y;
		SGid = SG_SETWINEXTEX;
		BigWrite (SavedGraphicsFid,&SGid,2,-1);
		BigWrite (SavedGraphicsFid,&x,sizeof(int),-1);
		BigWrite (SavedGraphicsFid,&y,sizeof(int),-1);
	}
	return SetWindowExtEx (hdc,x,y,lpsz);
}
BOOL  WINAPI GSSiSetWindowOrgEx( __in HDC hdc, __in int x, __in int y, __out_opt LPPOINT lppt)
{
	if (hdc == hDCSG && SavedGraphicsFid != HFILE_ERROR && (x != CurWINOrgEx_x || y != CurWINOrgEx_y))
	{
		CurWINOrgEx_x = x;
		CurWINOrgEx_y = y;
		SGid = SG_SETWINORGEX;
		BigWrite (SavedGraphicsFid,&SGid,2,-1);
		BigWrite (SavedGraphicsFid,&x,sizeof(int),-1);
		BigWrite (SavedGraphicsFid,&y,sizeof(int),-1);
	}
	return SetWindowOrgEx (hdc,x,y,lppt);
}

BOOL WINAPI GSSiSetWorldTransform( __in HDC hdc, __in CONST XFORM * lpxf)
{
	if (hdc == hDCSG && SavedGraphicsFid != HFILE_ERROR)
	{
		SGid = SG_SETWORLDTRANSFORM;
		BigWrite (SavedGraphicsFid,&SGid,2,-1);
		BigWrite (SavedGraphicsFid,(LPVOID)lpxf,sizeof(XFORM),-1); 
	}
	return SetWorldTransform (hdc,lpxf);
}


BOOL	 WINAPI GSSiStretchBlt(__in HDC hdcDest, __in int xDest, __in int yDest, __in int wDest, __in int hDest, __in HDC hdcSrc, __in int xSrc, __in int ySrc, __in int wSrc, __in int hSrc, __in DWORD rop)
{
	if (hdcDest == hDCSG && SavedGraphicsFid != HFILE_ERROR)
	{
		BITMAP	bm;
		HDC		hDCTemp;
		HBITMAP	hBMTemp, hBM;
		int		lnBits;
		LPBYTE	pBits;
		HANDLE	hBits;

		BitBltStruct.xDest = xDest;
		BitBltStruct.yDest = yDest;
		BitBltStruct.wDest = wDest;
		BitBltStruct.hDest = hDest;
		BitBltStruct.xSrc  = xSrc;
		BitBltStruct.ySrc  = ySrc;
		BitBltStruct.wSrc  = wSrc;
		BitBltStruct.hSrc  = hSrc;
		BitBltStruct.rop   = rop;
		hDCTemp = CreateCompatibleDC (hdcDest);
		if (wSrc * hSrc > wDest * hDest)
		{
			BitBltStruct.opt   = SG_STRETCHBLT1;
			hBMTemp = CreateCompatibleBitmap (hdcDest,wDest,hDest);
			hBM = SelectObject (hDCTemp,hBMTemp);
			StretchBlt (hDCTemp,0,0,wDest,hDest,hdcSrc,xSrc,ySrc,wSrc,hSrc,SRCCOPY);
		}
		else
		{
			BitBltStruct.opt   = SG_STRETCHBLT2;
			hBMTemp = CreateCompatibleBitmap (hdcDest,wSrc,hSrc);
			hBM = SelectObject (hDCTemp,hBMTemp);
			StretchBlt (hDCTemp,0,0,wSrc,hSrc,hdcSrc,xSrc,ySrc,wSrc,hSrc,SRCCOPY);
		}
		SelectObject (hDCTemp,hBM);
		GetObject (hBMTemp,sizeof(BITMAP),&BitBltStruct.bm);
		lnBits = BitBltStruct.bm.bmWidthBytes*BitBltStruct.bm.bmHeight;
		hBits = GSSiGlobAlloc (0,GMEM_MOVEABLE,lnBits);
		pBits = GlobalLock (hBits);
		GetBitmapBits (hBMTemp,lnBits,pBits);
		BigWrite (SavedGraphicsFid,&BitBltStruct,sizeof(BitBltStruct),-1);
		BigWrite (SavedGraphicsFid,pBits,lnBits,-1);
		GSSiGlobUlFree (&hBits);
		return TRUE;
	}
	return StretchBlt(hdcDest,xDest, yDest,wDest, hDest, hdcSrc,xSrc, ySrc, wSrc,hSrc, rop);
}

void wantnextblt(void)
{
	wantnextbltblt = TRUE;
}
BOOL	WINAPI GSSiBitBlt( __in HDC hdc, __in int x, __in int y, __in int cx, __in int cy, __in_opt HDC hdcSrc, __in int x1, __in int y1, __in DWORD rop)
{
	if (wantnextbltblt)
		wantnextbltblt = FALSE;
	return BitBlt(hdc, x, y, cx, cy,  hdcSrc, x1, y1, rop);
}


BOOL  WINAPI GSSiPolygon(__in HDC hdc, __in_ecount(cpt) CONST POINT *apt, __in int cpt)
{
	if (FidSTG != HFILE_ERROR)
		SaveTileGraphics (FidSTG,hdc,TGPOLYGON,(LPPOINT)apt,cpt);
	if (CaptureIndexGraphics)
	{
		int	loc = _llseek (FidCIG,0,1);

		if (!stricmp (CurrentPrefix,CIGPrefix))
		{
			_lwrite (FidCIGIndex,CurrentUDI,64);
			_lwrite (FidCIGIndex,(LPSTR)&loc,4);
		}
		_lwrite (FidCIG,(LPSTR)&cpt,4);
		_lwrite (FidCIG,(LPSTR)apt,cpt*sizeof(POINT));
	}
	if (hdc == hDCSG && SavedGraphicsFid != HFILE_ERROR)
	{
		HPEN	hPen=GetCurrentObject (hdc,OBJ_PEN);
		HBRUSH	hBrush=GetCurrentObject (hdc,OBJ_BRUSH);

		if (hPen != CurPen[CurSGFType])
		{
			CurPen[CurSGFType] = hPen;
			ln2 = GetObject (hPen,sizeof(EXTLOGPEN),&ExtLogPen);
			if (ln2 == sizeof (LOGPEN))
			{
				if (!memcmp (&CurLogPen[CurSGFType],&ExtLogPen,ln2))
					goto Next;
				CurLogPen[CurSGFType] = *(LPLOGPEN)&ExtLogPen;
				SGid = SG_LOGPEN;
			}
			else
				SGid = SG_EXTLOGPEN;
			BigWrite (SavedGraphicsFid,&SGid,2,-1);
			BigWrite (SavedGraphicsFid,&ln2,2,-1);
			BigWrite (SavedGraphicsFid,&ExtLogPen,ln2,-1);
		}
Next:
		if (hBrush != CurBrush[CurSGFType])
		{
			CurBrush[CurSGFType] = hBrush;
			ln2 = GetObject (hBrush,sizeof(LOGBRUSH),&LogBrush);
			if (ln2 == sizeof (LOGBRUSH))
			{
				if (!memcmp (&CurLogBrush[CurSGFType],&LogBrush,ln2))
					goto WritePoly;
				CurLogBrush[CurSGFType] = *(LPLOGBRUSH)&LogBrush;
				SGid = SG_LOGBRUSH;
			}
			BigWrite (SavedGraphicsFid,&SGid,2,-1);
			BigWrite (SavedGraphicsFid,&ln2,2,-1);
			BigWrite (SavedGraphicsFid,&LogBrush,ln2,-1);
		}
WritePoly:
		SGid = SG_POLYGON;
		BigWrite (SavedGraphicsFid,&SGid,2,-1);
		BigWrite (SavedGraphicsFid,&cpt,4,-1);
		BigWrite (SavedGraphicsFid,(LPVOID)apt,cpt*sizeof(POINT),-1);
		return TRUE;
	}
	return Polygon (hdc, apt, cpt);
}

BOOL  WINAPI GSSiPolyline(__in HDC hdc, __in_ecount(cpt) CONST POINT *apt, __in int cpt)
{
	
	if (FidSTG != HFILE_ERROR)
		SaveTileGraphics (FidSTG,hdc,TGPOLYLINE,(LPPOINT)apt,cpt);
	if (hdc == hDCSG && SavedGraphicsFid != HFILE_ERROR)
	{
		HPEN	hPen=GetCurrentObject (hdc,OBJ_PEN);

		if (hPen != CurPen[CurSGFType])
		{
			CurPen[CurSGFType] = hPen;
			ln2 = GetObject (hPen,sizeof(EXTLOGPEN),&ExtLogPen);
			if (ln2 == sizeof (LOGPEN))
			{
				if (!memcmp (&CurLogPen[CurSGFType],&ExtLogPen,ln2))
					goto WritePoly;
				CurLogPen[CurSGFType] = *(LPLOGPEN)&ExtLogPen;
				SGid = SG_LOGPEN;
			}
			else
				SGid = SG_EXTLOGPEN;
			BigWrite (SavedGraphicsFid,&SGid,2,-1);
			BigWrite (SavedGraphicsFid,&ln2,2,-1);
			BigWrite (SavedGraphicsFid,&ExtLogPen,ln2,-1);
		}
WritePoly:
		SGid = SG_POLYLINE;
		BigWrite (SavedGraphicsFid,&SGid,2,-1);
		BigWrite (SavedGraphicsFid,&cpt,4,-1);
		BigWrite (SavedGraphicsFid,(LPVOID)apt,cpt*sizeof(POINT),-1);
		return TRUE;
	}
	return Polyline (hdc, apt, cpt);
}

BOOL ObjectInUse (HGDIOBJ hobj) 
{ 
	return FALSE;
}
BOOL ObjectInUsex (HGDIOBJ hobj) 
{   
	BOOL	rtn = FALSE;
	HDC		hdc=GetDC (hWndMain);
	HPEN	hOldPen=SelectObject (hdc,GetStockObject(NULL_PEN));
	HBRUSH	hOldBrush=SelectObject (hdc,GetStockObject(NULL_BRUSH));
	HFONT	hOldFont=SelectObject (hdc,GetStockObject(SYSTEM_FONT));
    
    if (hOldPen == hobj)
    	rtn = TRUE; 
    else
    	SelectObject (hdc,hOldPen);	
    if (hOldBrush == hobj)
    	rtn = TRUE; 
    else
    	SelectObject (hdc,hOldBrush);	
    if (hOldFont == hobj)
    	rtn = TRUE; 
    else
    	SelectObject (hdc,hOldFont);	
	ReleaseDC (hWndMain,hdc);
	return rtn;
}


BOOL GSSiDELETEOBJCT (HGDIOBJ hobj)
{
	BOOL rtn=1;  
	short	ii;
	static	HGDIOBJ debugobj=0;
	
	if (InDebug)
	{
		if (hobj == debugobj)
			ii=1;
		if (ObjectInUse (hobj))
			MEMERR ("Deleting in-use object");
		TrackObject (hobj,-1);
	}
	//if (IsGDIObject (hobj))
	rtn = DeleteObject (hobj);
	//else 
	//	MEMERR ("Deleting invalid object");  
	if (!InDebug)
		return rtn;
	if (!rtn)
	{
		DWORD Err = GetLastError ();
		MEMERR ("Error deleting object"); 
	}
	if (nLastd == MAXLAST)
	{
		memmove (LastObjd,&LastObjd[1],sizeof(HGDIOBJ)*(MAXLAST-1));
		LastObjd[MAXLAST-1] = hobj;
	}
	else
		LastObjd[nLastd++] = hobj;
	return rtn;
}

HGDIOBJ GSSiSELECTOBJECT (HDC hdc,HGDIOBJ hobj)
{
	HGDIOBJ holdobj;    
	static	HGDIOBJ	debugobject=0;
	int		ii;
	
	if (GetObject (hobj,0,0) == sizeof (LOGPEN))
		ii=1;
//	if (hobj == debugobject)
//		ii=1;
	//if (IsGDIObject (hobj))
		holdobj = SelectObject (hdc,hobj); 
	//else
	//	MEMERR ("Selecting invalid object");   
	if (InDebug)
	{
		if (nLast == MAXLAST)
		{
			memmove (LastObj,&LastObj[1],sizeof(HGDIOBJ)*(MAXLAST-1));
			LastObj[MAXLAST-1] = hobj;
		}
		else
			LastObj[nLast++] = hobj;
	}
	return holdobj;
}
HPEN    WINAPI GSSiCREATEPEN (int style, int width, COLORREF color)
{
	HPEN	rtn = CreatePen (style,width,color);

/*	if (SavedGraphicsFid != HFILE_ERROR)
	{
		SGid = SG_CREATEPEN;
		CreatePenStruct.Style = style;
		CreatePenStruct.Width = width;
		CreatePenStruct.Color = color;
		BigWrite (SavedGraphicsFid,&SGid,2);
		BigWrite (SavedGraphicsFid,&CreatePenStruct,sizeof(CreatePenStruct));
		return rtn;
	}*/
	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreatePen Failed");
	else
		TrackObject (rtn,1);
	return rtn;
}

HPEN WINAPI GSSiEXTCREATEPEN( DWORD iPenStyle,
									DWORD cWidth,
                                    CONST LOGBRUSH *plbrush,
                                    DWORD cStyle,
                                    DWORD *pstyle)
{
	HPEN	rtn = ExtCreatePen (iPenStyle,cWidth,plbrush,cStyle,pstyle);
	
	if (!InDebug)
		return rtn;
	if (!rtn)
	{
		char Mess[1024];

		GetSystemErrMessage (Mess);
		MEMERR ("ExtCreatePen Failed");
	}
	else
		TrackObject (rtn,1);
	return rtn;
}


HBRUSH  WINAPI GSSiCREATESOLIDBRUSH(COLORREF color)
{
	HBRUSH	rtn = CreateSolidBrush (color);
	
	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreateSolidBrush Failed");
	else
		TrackObject (rtn,2);
	return rtn;
}

HBRUSH  WINAPI GSSiCREATEHATCHBRUSH(int style, COLORREF color)  
{
	HBRUSH	rtn = CreateHatchBrush (style,color);
	
	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreateHatchBrush Failed");
	else
		TrackObject (rtn,3);
	return rtn;
}

HBRUSH  WINAPI GSSiCREATEPATTERNBRUSH(HBITMAP bitmap) 
{
	HBRUSH	rtn = CreatePatternBrush (bitmap);
	
	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreatePatternBrush Failed");
	else
		TrackObject (rtn,4);
	return rtn;
}

HBRUSH  WINAPI GSSiCREATEBRUSHINDIRECT(LOGBRUSH FAR* logbrush)
{
	HBRUSH	rtn = CreateBrushIndirect (logbrush);
	
	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreateBrushIndirect Failed");
	else
		TrackObject (rtn,5);
	return rtn;
} 

HBITMAP WINAPI GSSiLOADBITMAP(HINSTANCE hInst, LPCSTR Name)
{
	HBRUSH	rtn = LoadBitmap (hInst,Name);
	
	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("LoadBitmap Failed");
	else
		TrackObject (rtn,6);
	return rtn;
} 

HFONT   WINAPI GSSiCREATEFONT(int i1, int i2, int i3, int i4, int i5, BYTE b1, BYTE b2, BYTE b3, BYTE b4, BYTE b5, BYTE b6, BYTE b7, BYTE b8, LPCSTR name)
{
	HFONT	rtn = CreateFont(i1,i2,i3,i4,i5,b1,b2,b3,b4,b5,b6,b7,b8,name);
	
	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreateFont Failed");
	else
		TrackObject (rtn,7);
	return rtn;
}
 
HFONT   WINAPI GSSiCREATEFONTINDIRECT (const LOGFONT FAR* lf)
{
	HFONT	rtn = CreateFontIndirect (lf);
	
	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreateFontIndirect Failed");
	else
		TrackObject (rtn,8);
	return rtn;
} 

int  WINAPI GSSiSELECTCLIPRGN(__in HDC hdc, __in_opt HRGN hrgn)
{
	int	rtn = SelectClipRgn (hdc,hrgn);

	if (!InDebug)
		return rtn;

	if (rtn == ERROR)
	{
		char str[40];
		sprintf (str,"SelectClipRgn Failed:%x",(int)hdc);
		MEMERR (str);
	}
	return rtn;
}

HRGN    WINAPI GSSiCREATEELLIPTICRGN (int i1, int i2, int i3, int i4)
{
	HRGN    rtn = CreateEllipticRgn(i1,i2,i3,i4);

	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreateEllipticRgn Failed");
	else
		TrackObject (rtn,16);
	return rtn;
} 

HRGN    WINAPI GSSiCREATERECTRGN (int i1, int i2, int i3, int i4)
{
	HRGN    rtn = CreateRectRgn(i1,i2,i3,i4);

	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreateRectRgn Failed");
	else
		TrackObject (rtn,9);
	return rtn;
} 

HRGN    WINAPI GSSiCREATEROUNDRECTRGN(int x1,int y1,int x2,int y2, int w, int h)
{
	HRGN    rtn = CreateRoundRectRgn (x1,y1,x2,y2,w,h);

	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreateRoundRectRgn Failed");
	else
		TrackObject (rtn,18);
	return rtn;
} 

HRGN    WINAPI GSSiCREATERECTRGNINDIRECT (const RECT FAR* pr)
{
	HRGN    rtn = CreateRectRgnIndirect (pr);

	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreateRectRgnIndirect Failed");
	else
		TrackObject (rtn,10);
	return rtn;
} 

HRGN    WINAPI GSSiCREATEPOLYGONRGN (const POINT FAR* pp, int i1, int i2)
{
	HRGN    rtn = CreatePolygonRgn (pp,i1,i2);

	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreatePolygonRgn Failed");
	else
		TrackObject (rtn,11);
	return rtn;
} 

HBITMAP WINAPI GSSiCREATEBITMAP (int i1, int i2, UINT i3, UINT i4, const void FAR* p)
{
	HBITMAP    rtn = CreateBitmap (i1,i2,i3,i4,p);


	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreateBitmap Failed");
	else
		TrackObject (rtn,12);
	return rtn;
} 

HBITMAP WINAPI GSSiCREATECOMPATIBLEBITMAP (HDC hDC, int i1, int i2)
{
	HBITMAP    rtn = CreateCompatibleBitmap (hDC,i1,i2);

	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreateCompatibleBitmap Failed");
	else
		TrackObject (rtn,13);
	return rtn;
} 

HBITMAP WINAPI GSSiCREATEDIBITMAP(HDC hDC, BITMAPINFOHEADER FAR* pbi, DWORD i1, const void FAR* p, BITMAPINFO FAR* pb, UINT i2)
{
	HBITMAP    rtn = CreateDIBitmap (hDC,pbi,i1,p,pb,i2);

	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreateDIBitmap Failed");
	else
		TrackObject (rtn,14);
	return rtn;
} 

HPALETTE WINAPI GSSiCREATEPALETTE(const LOGPALETTE FAR* lp)
{
	HPALETTE    rtn = CreatePalette (lp);

	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreatePalette Failed");
	else
		TrackObject (rtn,15);
	return rtn;
} 

HBITMAP WINAPI GSSiCREATEDIBSECTION (HDC hDC,BITMAPINFO *lpbmi,UINT usage, VOID **ppvBits,HANDLE hSection,DWORD offset)
{
	HBITMAP    rtn = CreateDIBSection (hDC,lpbmi,usage,ppvBits,hSection,offset);

	if (!InDebug)
		return rtn;
	if (!rtn)
		MEMERR ("CreateDIBSection Failed");
	else
		TrackObject (rtn,17);
	return rtn;
} 

BOOL WINAPI GSSiMoveWindow(__in HWND hWnd,__in int X,__in int Y,__in int nWidth,__in int nHeight,__in BOOL bRepaint)
{
	return MoveWindow (hWnd,X,Y,nWidth,nHeight,bRepaint);
}

BOOL WINAPI GSSiSetWindowPos(__in HWND hWnd,__in_opt HWND hWndInsertAfter,__in int X,__in int Y,__in int cx,__in int cy,__in UINT uFlags)
{
	return SetWindowPos (hWnd,hWndInsertAfter,X,Y,cx,cy,uFlags);
}
UINT_PTR WINAPI GSSiSetTimer( __in_opt HWND hWnd,__in UINT_PTR nIDEvent,__in UINT uElapse,__in_opt TIMERPROC lpTimerFunc)
{
	return SetTimer (hWnd,nIDEvent,uElapse,lpTimerFunc);
}

BOOL WINAPI GSSiKillTimer(__in_opt HWND hWnd,__in UINT_PTR uIDEvent)
{
	return KillTimer (hWnd,uIDEvent);
}
