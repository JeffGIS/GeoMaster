#include <windows.h>
#include <winbase.h>
#include <math.h>

#define  RADDEG    1.74532925199433e-2
#define  HALFPI    1.570796326794896e0

extern HWND hWndMain;
extern BOOL	displayGrids;
extern int	currentCellID;

POINT WorldPointToImagePoint (int TileWorldX, int TileWorldY);
POINT WorldPointToScreenPoint (int WorldX, int WorldY);
POINT inewpt (POINT OldPoint, double AZM, double DIS);

void setscreenpixel (POINT pt,COLORREF color)
{
	HDC	hDC = GetDC (hWndMain);

	SetPixel (hDC,pt.x,pt.y,color);
	SetPixel (hDC,pt.x-1,pt.y,color);
	SetPixel (hDC,pt.x+1,pt.y,color);
	SetPixel (hDC,pt.x,pt.y-1,color);
	SetPixel (hDC,pt.x,pt.y+1,color);
	SetPixel (hDC,pt.x+1,pt.y+1,color);
	SetPixel (hDC,pt.x+1,pt.y-1,color);
	SetPixel (hDC,pt.x-1,pt.y+1,color);
	SetPixel (hDC,pt.x-1,pt.y-1,color);
	ReleaseDC (hWndMain,hDC);
	return;
}

int StretchHBBits (HANDLE HBImageHandle,
				   int XDest, int YDest, int nDestWidth, int nDestHeight,
				   int XSrc,  int YSrc,  int nSrcWidth,  int nSrcHeight, 
				   BYTE *lpBytes,
				   int TileHeight, int TileWidth,
				   int TilePaletteLen,
				   RGBQUAD *TilePalette)
{
	LPBITMAPINFO pbi = malloc (sizeof (BITMAPINFO)+1024);
	static	int wantCellID=-1;
 
	memset (pbi,0,sizeof (BITMAPINFO)+1024);
	pbi->bmiHeader.biHeight = TileHeight;
	pbi->bmiHeader.biWidth  = TileWidth;
	pbi->bmiHeader.biPlanes = 1;
	pbi->bmiHeader.biSize   = 40;
	pbi->bmiHeader.biBitCount = 8;
	pbi->bmiHeader.biClrUsed = max (256,TilePaletteLen);
	pbi->bmiHeader.biClrImportant = pbi->bmiHeader.biClrUsed;
	memmove (pbi->bmiColors,TilePalette,TilePaletteLen*4);
	if (currentCellID == wantCellID || wantCellID == -1)
		StretchDIBits ((HDC)HBImageHandle,
				   XDest, YDest, nDestWidth, nDestHeight,
				   XSrc,  YSrc,  nSrcWidth,  nSrcHeight, 
				   lpBytes,
				   pbi,
				   DIB_RGB_COLORS,
				   SRCCOPY);
	if (displayGrids)
	{
		POINT points[5];
		HPEN hPen,hOldPen;
		char str[32];
		int	x,y;

		x = XDest + nDestWidth /2;
		y = YDest + nDestHeight/2;

		points[0].x = XDest;
		points[0].y = YDest;
		points[1].x = XDest + nDestWidth;
		points[1].y = YDest;
		points[2].x = XDest + nDestWidth;
		points[2].y = YDest + nDestHeight;
		points[3].x = XDest;
		points[3].y = YDest + nDestHeight;
		points[4].x = XDest;
		points[4].y = YDest;
		hPen = CreatePen (PS_SOLID,0,RGB(255,255,0));
		hOldPen = SelectObject ((HDC)HBImageHandle,hPen);
		Polyline ((HDC)HBImageHandle,points,5);
		SelectObject ((HDC)HBImageHandle,hOldPen);
		DeleteObject (hPen);
		itoa (currentCellID,str,10);
		TextOut ((HDC)HBImageHandle,x,y,str,strlen(str));
	}
	free (pbi);
	return 0;
}



POINT AdjustTextPoint (HDC hDC,char *chr,int nchr,POINT CenterPoint,int Angle)
{
	SIZE	txSize;
	double	az=((double)Angle)/10 * RADDEG;
	double		twidth, theight;
	POINT	Point;
/*	RECT	rect;

	rect.left  = CenterPoint.x-1;
	rect.right = CenterPoint.x+1;
	rect.top = CenterPoint.y-1;
	rect.bottom = CenterPoint.y+1;

	FillRect (hDC,&rect,GetStockObject (BLACK_BRUSH));*/
	
	GetTextExtentPoint32 (hDC,chr,nchr,&txSize); 
	twidth = txSize.cx;
	theight = txSize.cy;
	Point = inewpt (CenterPoint,-az,-twidth/2);
	Point = inewpt (Point,-az-HALFPI,theight/2);
	return Point;
}

void HBDisplayTextChar (HANDLE HBImageHandle,char *chr, int nchar,int WorldX, int WorldY,int Rotation,int Size,int TextColor,int ShadowColor)
{
	LOGFONT	LogFont;    
   	HFONT	OldFont,hFont;
	HDC		hDC = (HDC)HBImageHandle;
	POINT	AdjustedPoint;
	POINT	CenterPoint;

	CenterPoint = WorldPointToScreenPoint (WorldX,WorldY);
	memset (&LogFont,0,sizeof(LOGFONT));
	LogFont.lfHeight = Size-1; 
	LogFont.lfEscapement = LogFont.lfOrientation = Rotation; 
	LogFont.lfWeight = FW_NORMAL;    
	LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
	LogFont.lfQuality = PROOF_QUALITY; 
	strcpy (LogFont.lfFaceName,"Arial Rounded MT Bold");  
	hFont = CreateFontIndirect((LPLOGFONT)&LogFont);
	OldFont = SelectObject (hDC,hFont);
	AdjustedPoint = AdjustTextPoint (hDC,chr,nchar,CenterPoint,Rotation);
	SetBkMode (hDC,TRANSPARENT);
	if (ShadowColor)
	{
		int	i;
		int	inc=3;

		SetTextColor (hDC,ShadowColor); 
		for (i=1;i<inc;i++)
		{
			TextOut (hDC,AdjustedPoint.x+i,AdjustedPoint.y+i,chr,nchar);
			TextOut (hDC,AdjustedPoint.x-i,AdjustedPoint.y+i,chr,nchar);
			TextOut (hDC,AdjustedPoint.x-i,AdjustedPoint.y-i,chr,nchar);
			TextOut (hDC,AdjustedPoint.x+i,AdjustedPoint.y-i,chr,nchar);
		}
	}
	SetTextColor (hDC,TextColor);
	TextOut (hDC,AdjustedPoint.x,AdjustedPoint.y,chr,nchar);	
	SelectObject (hDC,OldFont);
	DeleteObject (hFont);
	return;
}

