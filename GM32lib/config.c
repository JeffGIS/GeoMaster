#include <windows.h>
#include <winbase.h>
#include <shlobj.h>
#include <commdlg.h>
#include <stdio.h>
#include <direct.h>
#include <winnt.h> 
#include <Wownt32.h>
#include "minilzo.h"
#include <vfw.h>
#include <string.h>
#include <assert.h>
#include <io.h>
#include <memory.h>
#include <stdlib.h>
#include "FreeImage.h"
#include <winreg.h>
#include <errno.h>
#include <urlmon.h>
#include <wingdi.h>
#include "resource.h"
#include "dibapi.h"
#include "dibutil.h"
#include "gm32lib.h"
#include <dlgs.h>


static	short	CfgImageFormat;
static	long	CfgImageLen; 
static 	long	CfgImageLoc;
static	long	CfgDescLoc;  
static	LPSTR	VPAutoFile=NULL;
static	struct	{unsigned	short	HaveImage:1,
	 								Unused1:1, 
	 								Unused2:1, 
	 								Unused3:1,
	 								Len:12;
	 			}ConfigDesc;
HANDLE	hConfigDescription=0;  
HANDLE	hCfgImage=0;
short	destX;
short	destY;
short	destW;
short	destH;
short	StretchMode=STRETCH_DELETESCANS;
short	rect_width;
short	rect_height;
WORD	bmX;
WORD	bmY;

#define		CURRENT_CFG_VERSION	6 



LPSTR FAR FindDIBBits(LPSTR lpDIB)
{
   return (lpDIB + *(LPDWORD)lpDIB + PaletteSize(lpDIB));
}

void ComputeBMLoc (RECT Rect,LPBITMAPINFO pDibInfo,short MaintainAspect)
#if ENABLETRACE
{GSSiEnterProg (397);
#endif
{    float  pw, ph, fac;

     if (!pDibInfo)
{
#if ENABLETRACE
GSSiExitProg (397);
#endif
     	return;
}    
	 if (abs(MaintainAspect) == 2)   
	 {
	     rect_width = Rect.right - Rect.left;
	     rect_height = Rect.bottom - Rect.top;
	     ph =  rect_height / (float)pDibInfo->bmiHeader.biHeight;
	     pw =  rect_width / (float)pDibInfo->bmiHeader.biWidth;
	     ph = min (ph,pw);
     	 fac = 1;
	     
	     if (ph > 1) 
	     	fac = (long)ph;
	     else if (ph > 0) 
	     {
	     	while (ph*fac < 1) 
	     		fac *= 2;
	     	fac = 1/fac;
	     }	
         destH = pDibInfo->bmiHeader.biHeight * fac;
         destW = pDibInfo->bmiHeader.biWidth * fac;
         destX = (short)(Rect.left + (rect_width - destW) / 2); 
         if (MaintainAspect < 0) 
         	destY = Rect.top;
         else
         	destY =  Rect.top + (rect_height - destH) / 2;   
	 }
	 else
	 {
	 
	     rect_width = Rect.right - Rect.left + 1;
	     rect_height = Rect.bottom - Rect.top + 1;
	     ph = (float)pDibInfo->bmiHeader.biHeight / rect_height;
	     pw = (float)pDibInfo->bmiHeader.biWidth / rect_width;
	     if (ph > pw)
	        {destY = (short)Rect.top;
	         destH = (short)rect_height;
	         destW = (short)(IDNINT(destH * ((double)pDibInfo->bmiHeader.biWidth /
	                          (double)pDibInfo->bmiHeader.biHeight)));
	         destX = (short)(Rect.left + (rect_width - destW) / 2);}
	     else
	        {destX = (short)Rect.left;
	         destW = (short)rect_width;
	         destH = (short)(IDNINT(destW * ((double)pDibInfo->bmiHeader.biHeight /
	                          (double)pDibInfo->bmiHeader.biWidth)));
	         destY = (short)(Rect.top + (rect_height - destH) / 2);}   
     }
     bmX = bmY = 0;
{
#if ENABLETRACE
GSSiExitProg (397);
#endif
     return;
}
#if ENABLETRACE
}
#endif
}  
void GSSiGlobFree (LPHANDLE pHandle)
{
	if (*pHandle)
		GlobalFree (*pHandle);
	*pHandle = 0;
}

void GSSiGlobUlFree (LPHANDLE pHandle)
{
	GlobalUnlock (*pHandle);
	if (*pHandle)
		GlobalFree (*pHandle);
	*pHandle = 0;
}

HGLOBAL GSSiGlobAlloc (USHORT From,UINT fuAlloc, long cbAlloc)
{
	HGLOBAL handle;
    handle = GlobalAlloc (fuAlloc, (DWORD)cbAlloc);
	return handle;
}

BOOL DisplayConfigPreview (HWND hDlg,HDC hDC,LPSTR Name,LPRECT ImageRect, UINT TextCntl)
{  
	short	Signature, Version;
	long	ii;
	BOOL	SetDC=FALSE;
	HFILE	FidConfig = GSSiOpenFile (Name,NULL,OF_READ);
//	HWND	hWnd=GetDlgItem(hDlg,IDC_PREVGMC);
	
	if (FidConfig == HFILE_ERROR)
		return FALSE;
		
    _llseek(FidConfig,(LONG)-(6),2);

    _lread (FidConfig,&ConfigDesc,2);
    _lread (FidConfig,&Signature,2);
    _lread (FidConfig,&Version,2);
    if (Signature != 28052 || Version < 5 || Version > CURRENT_CFG_VERSION)
    	goto Exit;
	if (Version > 5)
		FidConfig = DecompressCfgFile (FidConfig);
    if (!hDC)
	{
		SetDC = TRUE;
		hDC = GetDC (hDlg); 
	}
    //SelectClipRgn (hDC,0);
	SetMapMode    (hDC, MM_TEXT );
	SetWindowOrgEx  (hDC, 0,   0,0 );
	SetViewportOrgEx(hDC, 0, 0,0 ); 
    if (ConfigDesc.Len)
    {   
    	LPSTR	pConfigDescription; 
    	short	id; 
    	HFONT	hFont, OldFont;
    	
	    GSSiGlobFree (&hConfigDescription);
    	hConfigDescription = GSSiGlobAlloc (1315,GMEM_MOVEABLE,512);
    	pConfigDescription = GlobalLock (hConfigDescription);
	    GSSillseek(FidConfig,(LONG)-(6+4+4),2);
	    GSSilread (FidConfig,&CfgDescLoc,4);
	    GSSillseek(FidConfig,CfgDescLoc,0);
	    GSSilread (FidConfig,&id,2); 
	    GSSilread (FidConfig,pConfigDescription,ConfigDesc.Len);
//		hFont = CreateFont(12, 0, 0, 0, FW_BOLD, 
//	    					0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
//	    OldFont = SelectObject (hDC,hFont);     
		if (hDlg)
			SetDlgItemText (hDlg,TextCntl,pConfigDescription);
//		DrawText (hDC,pConfigDescription,_fstrlen(pConfigDescription),TextRect,DT_LEFT); 
//		SelectObject (hDC,OldFont);
//		DeleteObject (hFont);
//		RestoreDC (hDC,-1);	
	    GlobalUnlock (hConfigDescription);
    }
    
    if (ConfigDesc.HaveImage)   
    {   
    	LPSTR	pCfgImage;  
    	short	id;
    	                               
	   ii=GSSillseek(FidConfig,-(long)(6+4),2);
	   ii= GSSilread (FidConfig,&CfgImageLoc,4); 
	    GSSillseek(FidConfig,CfgImageLoc,0);
	    GSSilread (FidConfig,&id,2); 
	    GSSilread (FidConfig,&CfgImageFormat,2); 
	    GSSilread (FidConfig,&CfgImageLen,4);
	    if (CfgImageLen)
	    { 
		    hCfgImage = GSSiGlobAlloc (0,GMEM_MOVEABLE,CfgImageLen);
		    pCfgImage = GlobalLock (hCfgImage); 
		    GSSilread (FidConfig,(HPSTR)pCfgImage,CfgImageLen); 
		    GlobalUnlock (hCfgImage); 
   			DisplayBMInRect2 (hDC,hCfgImage, *ImageRect,0,0,0);
			GSSiGlobFree (&hCfgImage); 
		}
	}
	if (SetDC)
		ReleaseDC (hDlg,hDC);
Exit:
	GSSiClose (FidConfig);
	return TRUE;
}

HFILE DecompressCfgFile (HFILE FidConfig)
{   
    long	lenCfg = _llseek (FidConfig,0,2);
	HANDLE	handle=GSSiGlobAlloc (0,GMEM_MOVEABLE,MAX_CFG_SIZE);
	HANDLE	hTemp = GSSiGlobAlloc (0,GMEM_MOVEABLE,lenCfg*2);
	HPSTR	pTemp=GlobalLock (hTemp);
	HPSTR	pCfg = GlobalLock (handle);  
	short	ConfigDesc,Signature,Version;   
	
    GSSillseek(FidConfig,lenCfg-6,0);
    GSSilread (FidConfig,&ConfigDesc,2);
    GSSilread (FidConfig,&Signature,2);
    GSSilread (FidConfig,&Version,2);
    GSSillseek(FidConfig,0,0);
    _lread (FidConfig,pTemp,lenCfg-6);
    GSSiClose (FidConfig);
	lenCfg = GM32DecompressBinaryRecord (pCfg,pTemp,lenCfg-6); 
	pCfg += lenCfg; 
	BufWrite (&pCfg,&lenCfg,(HPSTR)&ConfigDesc,2);
	BufWrite (&pCfg,&lenCfg,(HPSTR)&Signature,2);
	BufWrite (&pCfg,&lenCfg,(HPSTR)&Version,2);
	GlobalUnlock (handle);
	GSSiGlobUlFree (&hTemp);
	FidConfig = SetMemFile (handle,lenCfg);  
	return FidConfig;
}

BOOL  DisplayBMInRect2 (HDC hDC, HANDLE hBM, RECT Rect, double Factor,double VPct, double HPct)
#if ENABLETRACE
{GSSiEnterProg (394);
#endif
{   short   i=0;
	LPBITMAPINFOHEADER pDibInfo;
	LPSTR pImage;  
	short	xoff=IDNINT(HPct),yoff=IDNINT(VPct);    
	BOOL	Use32=TRUE;
    
    if (!hBM)
{
#if ENABLETRACE
GSSiExitProg (394);
#endif
    	return FALSE;
}
    pDibInfo = (LPBITMAPINFOHEADER)GlobalLock (hBM);
    if (!pDibInfo)
{
#if ENABLETRACE
GSSiExitProg (394);
#endif
    	return FALSE;
}
    pImage = FindDIBBits ((LPSTR)pDibInfo);
    if (!Factor)
		i=DisplayBMInRect (hDC,pDibInfo,pImage, Rect, TRUE);
    else if (Factor < 0)
		i=DisplayBMInRect (hDC,pDibInfo,pImage, Rect, FALSE);
	else
	{
        destX = (short)Rect.left;
        destY = (short)Rect.top;
        destW = (short)(Rect.right - Rect.left + 1);
        destH = (short)(Rect.bottom - Rect.top + 1); 
        if (!Use32)
        {
		    SetStretchBltMode(hDC, StretchMode); 
		    i=StretchDIBits (hDC,destX,destY,
		                       destW, destH,
		                       xoff,yoff,
		                       (short) IDNINT(min(destW*Factor,(pDibInfo->biWidth-xoff)*Factor)),
		                       (short) IDNINT(min(destH*Factor,(pDibInfo->biHeight-yoff)*Factor)),
		                       pImage,
		                      (LPBITMAPINFO)pDibInfo,
		                      (UINT)DIB_RGB_COLORS,
		                      (DWORD) SRCCOPY); 
	    } 
	    else                 
		    i=GM32StretchDIBits (hDC,destX,destY,
		                       destW, destH,
		                       xoff,yoff,
		                       IDNINT(min(destW*Factor,(pDibInfo->biWidth-xoff)*Factor)),
		                       IDNINT(min(destH*Factor,(pDibInfo->biHeight-yoff)*Factor)),
		                      (LPBYTE) pImage,
		                      (LPBITMAPINFOHEADER)pDibInfo,
		                      (UINT)DIB_RGB_COLORS,
		                      (DWORD) SRCCOPY, 
		                      &Factor);
	                      
    }  
    GlobalUnlock (hBM); 
    if (i)
{
#if ENABLETRACE
GSSiExitProg (394);
#endif
    	return TRUE;
}
    else
{
#if ENABLETRACE
GSSiExitProg (394);
#endif
    	return FALSE;
}
#if ENABLETRACE
}
#endif
}

short  DisplayBMInRect (HDC hDC,LPBITMAPINFOHEADER pDibInfo,LPSTR pImage, RECT Rect, short MaintainAspect)
#if ENABLETRACE
{GSSiEnterProg (393);
#endif
{   short   i;
    UINT	ColorOpt;   
    double	Factor=1;
    
    if (MaintainAspect)
        ComputeBMLoc (Rect,(LPBITMAPINFO) pDibInfo,MaintainAspect);
    else
    { 
        destX = (short)Rect.left;
        destY = (short)Rect.top;
        destW = (short)(Rect.right - Rect.left + 1);
        destH = (short)(Rect.bottom - Rect.top + 1);
    }
    SetStretchBltMode(hDC, StretchMode); 
/*    if (pDibInfo->biClrUsed)
    	ColorOpt = DIB_PAL_COLORS;
    else*/
    	ColorOpt = DIB_RGB_COLORS;
    i=StretchDIBits32 (hDC,destX,destY,
                       destW, destH,
                       0,0,
                       (short) pDibInfo->biWidth,
                       (short) pDibInfo->biHeight,
                      (LPBITMAPINFOHEADER)pDibInfo,
                       pImage,
                      (UINT)ColorOpt,
                      (DWORD) SRCCOPY,&Factor);
/*  GlobalUnlock (hImage);
    GlobalUnlock (hDibInfo);
    GlobalFree (hImage);
    GlobalFree (hDibInfo);*/

{
#if ENABLETRACE
GSSiExitProg (393);
#endif
    return (destH);
}
#if ENABLETRACE
}
#endif
}

