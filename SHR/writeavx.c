
/****************************************************************************
 *
 *  WRITEAVI.C
 *
 *  Creates the file OUTPUT.AVI, an AVI file consisting of a rotating clock
 *  face.  This program demonstrates using the functions in AVIFILE.DLL
 *  to make writing AVI files simple.
 *
 *  This is a stripped-down example; a real application would have a user
 *  interface and check for errors.
 *
 ***************************************************************************/
/**************************************************************************
 *
 *  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *  PURPOSE.
 *
 *  Copyright (c) 1992, 1993  Microsoft Corporation.  All Rights Reserved.
 * 
 **************************************************************************/


#include <windows.h>
#include <mmsystem.h>
#define AVIIF_KEYFRAME      0x00000010L // this frame is a key frame.
#include <vfw.h>
#include <avifile.h>
#include <dibapi.h>  
#include <compman.h>
#include "shr.h"  

#define	TYPEAVI	1
#define	TYPEGCI	2

short	AVIFileType=0;
HANDLE	hIC=NULL;
HFILE	GCIFid=-1;
PAVIFILE pfile;
extern	BOOL	FastOrthos;
BOOL	HaveInit=FALSE;
long	TotCopyTime=0;
short	UserDefinedImageQuality=0;
extern	BOOL	Display16BitColor;
extern	BOOL	TraceOn,Printing;  
extern	short	MAXORTHOBUFS;
char	CurOrthoFile[128]="";
typedef struct {
				short	Type;  
				short	CompressionFactor;
				long	iframe;
				HANDLE	hIC;
				HFILE	GCIFid;
				PAVIFILE pfile;
				PAVISTREAM psCompressed, ps;
				}	AVIFILE;
typedef	AVIFILE	FAR	*LPAVIFILE;
AVISTREAMINFO strhdr;
PAVISTREAM 	pstream=0;
PGETFRAME	pget=0;
BOOL CopyDib2 (HANDLE hdib,LPHANDLE phDibInfo, LPHANDLE phImage);
void AdjustDIBColors (HANDLE hDib);
BOOL InitAVIOut (LPSTR Name, LPBITMAPINFOHEADER alpbi,LPHANDLE phFile);


#ifndef WIN32
#define CODE  _based(_segname("_CODE"))
#define STACK _based(_segname("_STACK"))
#else
#define CODE
#define STACK
#endif

short DIBHeadSize (LPBITMAPINFOHEADER lpbi)
{
	return (lpbi->biSize+lpbi->biClrUsed*sizeof(RGBQUAD));
}


BOOL CreateGCIFile (LPSTR Name,LPBITMAPINFOHEADER lpbi, HANDLE hFile)
{         
	HANDLE 	hIC;
	LONG	ICRtn;
	short	lhead;
    LPAVIFILE	pAVIFile;  
    OFSTRUCT	OFStruct;  
    LPBITMAPINFOHEADER	lpbiHead;  
    HANDLE	hHeader;
	
	pAVIFile = GlobalLock (hFile);
	pAVIFile->GCIFid = -1;
	pAVIFile->hIC = ICOpen (ICTYPE_VIDEO,mmioFOURCC('M', 'S', 'V', 'C'),ICMODE_COMPRESS);
	if (!pAVIFile->hIC)
		return FALSE; 
	hHeader = GSSiGlobAlloc (GMEM_MOVEABLE,sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD));
	lpbiHead = GlobalLock (hHeader);
	ICRtn = ICCompressGetFormat(pAVIFile->hIC,lpbi, lpbiHead); 
	ICRtn = ICCompressBegin(pAVIFile->hIC, lpbi,lpbiHead);  
	if (ICRtn != ICERR_OK)
	{
		GSSiGlobUlFree (&hHeader);
		return FALSE;             
	}
	pAVIFile->GCIFid = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
	lhead = DIBHeadSize (lpbiHead);
	_lwrite (pAVIFile->GCIFid,&lhead,2);
	_lwrite (pAVIFile->GCIFid,lpbiHead,lhead);
	lhead = DIBHeadSize (lpbi);
	_lwrite (pAVIFile->GCIFid,&lhead,2);
	_lwrite (pAVIFile->GCIFid,lpbi,lhead);

	GSSiGlobUlFree (&hHeader);
	return TRUE;
}

BOOL InitAVIOut (LPSTR Name, LPBITMAPINFOHEADER alpbi,LPHANDLE phFile)
{
    int			i;
    char		ach[50];
    int			iLen;
    AVICOMPRESSOPTIONS	opts;
    HRESULT		hr;
    DWORD		dwTextFormat;
    WORD	wVer;  
    CLSID 	ClsID;
    LPAVIFILE	pAVIFile;

    /* first let's make sure we are running on 1.1 */
    wVer = HIWORD(VideoForWindowsVersion());
    if (wVer < 0x010a){
	    /* oops, we are too old, blow out of here */
	    MessageBeep(MB_ICONHAND);
	    MessageBox(NULL, "Video for Windows version is too old",
		       "WriteAVI Error", MB_OK|MB_ICONSTOP);
	    return FALSE;
    }
    
    
    if (!HaveInit)
    	AVIFileInit();
    HaveInit = TRUE;
    
	*phFile = GSSiGlobAlloc (GHND,sizeof(AVIFILE));
	pAVIFile = GlobalLock (*phFile); 
    if (!UserDefinedImageQuality)
	    pAVIFile->CompressionFactor = 7200;
	else
	    pAVIFile->CompressionFactor = UserDefinedImageQuality;
	GlobalUnlock (*phFile);
	
    if (_fstrstr (Name,".gci"))
    	return (CreateGCIFile (Name,alpbi, *phFile));

   	pAVIFile->Type = TYPEAVI;
    hr = AVIFileOpen(&pAVIFile->pfile,			    // returned file pointer
		       Name,		    // file name
		       OF_WRITE | OF_CREATE,	    // mode to open file with
		       NULL);			    // use handler determined
						    // from file extension....
    if (hr != AVIERR_OK)
		return FALSE;

    // Fill in the header for the video stream....

    // The video stream will run in 15ths of a second....
    
    _fmemset(&strhdr, 0, sizeof(strhdr));
    strhdr.fccType                = streamtypeVIDEO;// stream type
    strhdr.fccHandler             = 0;
    strhdr.dwScale                = 1;
    strhdr.dwRate                 = 1;		    // 1 fps
    strhdr.dwSuggestedBufferSize  = alpbi->biSizeImage;
    SetRect(&strhdr.rcFrame, 0, 0,alpbi->biWidth,alpbi->biHeight);		    // rectangle for stream

    // And create the stream;
    hr = AVIFileCreateStream(pAVIFile->pfile,		    // file pointer
			       &pAVIFile->ps,		    // returned stream pointer
			       &strhdr);	    // stream header
    if (hr != AVIERR_OK) 
    {
    	GSSiGlobUlFree (phFile);
		return FALSE;
    }

    _fmemset(&opts, 0, sizeof(opts));

    opts.dwFlags = AVICOMPRESSF_VALID;
    opts.fccType = streamtypeVIDEO;
    opts.fccHandler = mmioFOURCC('M', 'S', 'V', 'C');
	opts.dwQuality = pAVIFile->CompressionFactor;   
    opts.dwKeyFrameEvery = 1;
    hr = AVIMakeCompressedStream(&pAVIFile->psCompressed, pAVIFile->ps, &opts, NULL); 
    if (hr != AVIERR_OK) 
    {
    	GSSiGlobUlFree (phFile);
		return FALSE;
    }
    
    hr = AVIStreamSetFormat(pAVIFile->psCompressed, 0, 
			       alpbi,	    // stream format
			       alpbi->biSize +   // format size
			       alpbi->biClrUsed * sizeof(RGBQUAD));
    if (hr != AVIERR_OK) 
    {
    	GSSiGlobUlFree (phFile);
		return FALSE;
    }
    GlobalUnlock (*phFile);
	return TRUE;
}

BOOL AVIOut (LPSTR Name,LPBITMAPINFOHEADER lpbi,LPHANDLE hFile,LPLONG pFrame)
{   
    HRESULT		hr;
    HDIB		hDIB;
    LPSTR		image;
    int			HeadLen;  
    long		OutSize;  
    LPAVIFILE	pAVIFile;
    LONG		ICRtn;
    HPBYTE		pInData, pCompressedData;       
    LPBITMAPINFOHEADER	lpbiHeadOut;     
    HANDLE		hHeader;
    
/*	HeadLen = sizeof(BITMAPINFOHEADER)+alpbi->biClrUsed*sizeof(RGBQUAD);
	image = (LPSTR)alpbi + HeadLen; 
	OutSize = 0.25 * alpbi->biSizeImage;
    hDIB = ICImageCompress (NULL,0,alpbi,image,alpbi,7000,&OutSize);
    alpbi = GlobalLock(hDIB); */
    
    if (!*hFile)
	{   
		if (!InitAVIOut (Name, lpbi,hFile)) 
		{
			GSSiGlobFree (hFile);
			return FALSE;       
		}
	}
	pAVIFile = GlobalLock (*hFile);
	pInData = (LPBYTE) lpbi +  
			  lpbi->biSize +
			  lpbi->biClrUsed * sizeof(RGBQUAD);
 
	if (pAVIFile->Type == TYPEAVI)
	{
		hr = AVIStreamWrite(pAVIFile->psCompressed,	    // stream pointer
				      pAVIFile->iframe,		    // time of this frame
				      1,		    // number to write
				      pInData,
				      lpbi->biSizeImage,// size of this frame
				      AVIIF_KEYFRAME,	    // flags....
				      NULL, NULL);
		pAVIFile->iframe++;   
		*pFrame = -1;
	} 
	else
	{   
		HANDLE	hCompData;
		DWORD	Flags;
		long	lRec;
		
		hCompData = GSSiGlobAlloc (GMEM_MOVEABLE,lpbi->biSizeImage);
		pCompressedData = GlobalLock (hCompData);
		hHeader = GSSiGlobAlloc (GMEM_MOVEABLE,sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD));
		lpbiHeadOut = GlobalLock (hHeader);
		ICRtn = ICCompressGetFormat(pAVIFile->hIC,lpbi,lpbiHeadOut);
		ICRtn = ICCompress (pAVIFile->hIC,ICCOMPRESS_KEYFRAME,
										lpbiHeadOut,pCompressedData,
										lpbi,pInData,
										NULL,&Flags,0,0,
										pAVIFile->CompressionFactor,NULL,NULL);  
		pAVIFile->iframe = _llseek (pAVIFile->GCIFid,0,2);  
		*pFrame = pAVIFile->iframe;
		lRec = lpbiHeadOut->biSize+lpbiHeadOut->biClrUsed*sizeof(RGBQUAD) + lpbiHeadOut->biSizeImage;
		_lwrite (pAVIFile->GCIFid,&lRec,4);
		_lwrite (pAVIFile->GCIFid,lpbiHeadOut,DIBHeadSize(lpbiHeadOut));
		_hwrite (pAVIFile->GCIFid,pCompressedData,lpbiHeadOut->biSizeImage);
		GSSiGlobUlFree (&hCompData);   
		GSSiGlobUlFree (&hHeader);   
		hr = AVIERR_OK;
	} 
	GlobalUnlock (*hFile);
    if (hr != AVIERR_OK)
    	return FALSE;
    return TRUE;
}
	
void AVIOutClose (LPHANDLE phFile)
{  
	LPAVIFILE	pAVIFile;
	
	if (!*phFile)
		return;
	pAVIFile = GlobalLock (*phFile); 
	
	if (pAVIFile->Type == TYPEAVI)
	{
	    if (pAVIFile->ps)
			AVIStreamClose(pAVIFile->ps);
	
	    if (pAVIFile->psCompressed)
			AVIStreamClose(pAVIFile->psCompressed);
	
	    if (pAVIFile->pfile)
			AVIFileClose(pAVIFile->pfile); 
    }
    else
    {   
    	if (pAVIFile->hIC)
    	{
    		ICCompressEnd(pAVIFile->hIC);
			ICClose (pAVIFile->hIC);
		}
		if (pAVIFile->GCIFid >= 0)
			_lclose (pAVIFile->GCIFid);           
	}    
    AVIFileExit();
    HaveInit=FALSE;               
    GSSiGlobUlFree (phFile);

    return;
}

BOOL AVIFrameToDIB (LPSTR File, long frame,LPHANDLE NewDIB,LPSHORT ShouldDeleteBM,
					int BitCount, int width, int height)
{
	HPALETTE	hPal, hPalNew = NULL;	// remember to init these
	HRESULT		hResult; 
	typedef	struct	{
			 			BITMAPINFOHEADER biHead;
			 			RGBQUAD Colors[256];}	BIHEADER;  
	typedef BIHEADER	FAR	*LPBIHEADER;
	static	BIHEADER	biHeadOut;
	BIHEADER	biHeadIn;
	LPBITMAPINFOHEADER  lpbi, lpbiHeadIn; 
	BITMAPINFOHEADER  	bi;
	LPSTR		lpDIBBits;
	char		Name[128],str[128];   
	LONG	ICRtn;
	short	lhead; 
	OFSTRUCT	OFStruct;
	                      
	_fstrcpy (Name,File);
	ExpandText (Name);   
	if (_fstricmp(Name,CurOrthoFile)) 
	{   
		CloseOrthoAVI(); 
		AVIFileInit(); 
    	if (_fstrstr (Name,".gci"))
    	{
			AVIFileType = TYPEGCI;
			hIC = ICOpen (ICTYPE_VIDEO,mmioFOURCC('M', 'S', 'V', 'C'),ICMODE_DECOMPRESS);
			if (!hIC)
				return FALSE;  
			GCIFid = GSSiOpenFile (Name,&OFStruct,OF_READ);
			if (GCIFid == HFILE_ERROR)  
			{
				ICClose (hIC);
				return FALSE; 
			}  
			_lread (GCIFid,&lhead,2);
			_lread (GCIFid,&biHeadIn,lhead);
			_lread (GCIFid,&lhead,2);
			_lread (GCIFid,&biHeadOut,lhead);
			ICRtn = ICDecompressGetFormat(hIC,&biHeadIn,&biHeadOut);
			if (biHeadOut.biHead.biBitCount == 16 && (!Display16BitColor || Printing ))
			{
				biHeadOut.biHead.biBitCount=24;
				biHeadOut.biHead.biSizeImage=0;
			}
			ICRtn = ICDecompressBegin(hIC, &biHeadIn,&biHeadOut);  
    	}
    	else
    	{
			AVIFileType = TYPEAVI;
			hResult = AVIFileOpen(&pfile, Name, OF_READ, 0L);
			if (!pfile) 
			{
				GSSiTrace ("AVI File Open failed"); 
				return FALSE;
			}
			hResult = AVIFileGetStream(pfile, &pstream, streamtypeVIDEO, 0);
			
			if (!pstream)
			{
				GSSiTrace ("AVI File Get Stream failed");
				CloseOrthoAVI(); 
				return FALSE;
			}
			_fmemset (&bi,0,sizeof(BITMAPINFOHEADER));
			bi.biSize = sizeof(BITMAPINFOHEADER);
			bi.biWidth = width;
			bi.biHeight = height;
			bi.biPlanes = 1;
			bi.biBitCount =BitCount; 
			bi.biCompression = BI_RGB;
			bi.biSizeImage = (long) bi.biWidth * (long) bi.biHeight * bi.biBitCount/8;  
			if (FastOrthos) 
				pget = AVIStreamGetFrameOpen(pstream,NULL); 
			else
	//			pget = AVIStreamGetFrameOpen(pstream,&bi);
				pget = AVIStreamGetFrameOpen(pstream,NULL);
	/*		sprintf (str,"pget=%ld %ld %i %i %i %ld",(long)pget,(long)pstream,
			(int)bi.biWidth,(int)bi.biHeight,(int)bi.biBitCount,(long)bi.biSizeImage);
			GSSiTrace (str);  */
			if (!pget)
				GSSiTrace ("Failed to open compressor");
		}
		_fstrcpy (CurOrthoFile,Name);
	} 
	
	if (AVIFileType == TYPEGCI)
	{   
		long	len, lRec; 
		HANDLE	hCompressedData;
		HPBYTE	pCompressedData, pData;
		LPBIHEADER	lpbi;
		
		_llseek (GCIFid,frame,0);
		_lread (GCIFid,&lRec,4);            
		hCompressedData = GSSiGlobAlloc (GMEM_MOVEABLE,lRec);
		lpbiHeadIn = GlobalLock (hCompressedData);
		_hread (GCIFid,lpbiHeadIn,lRec); 
		pCompressedData = lpbiHeadIn;
		pCompressedData += DIBHeadSize (lpbiHeadIn);
		*NewDIB = GSSiGlobAlloc (GMEM_MOVEABLE,
								 biHeadOut.biHead.biSizeImage+DIBHeadSize(&biHeadOut));
		lpbi = GlobalLock (*NewDIB);
		*lpbi = biHeadOut;
		pData = lpbi;
		pData += DIBHeadSize (&biHeadOut);
		ICRtn = ICDecompress(hIC,0,lpbiHeadIn, pCompressedData,lpbi,pData);	 
		GlobalUnlock (*NewDIB);
	}
	else
	{
		if(pget)
		{  
	
			// Get our own personal copy of the DIB 
			GSSiTrace("Getting frame");
			lpbi = AVIStreamGetFrame(pget, frame);
		}
	}
	if (TraceOn)		
		SaveDIB ((HANDLE)HIWORD((DWORD)(lpbi)),"c:\\ortho.bmp");
	sprintf (str,"lpbi=%ld",(long)lpbi);
	GSSiTrace(str);
	if (lpbi)  
	{   
			
		if (AVIFileType == TYPEAVI && lpbi->biBitCount == 16 && (!Display16BitColor || Printing ))
		{
			*NewDIB = ConvertBitmap (lpbi); 
			*ShouldDeleteBM = TRUE;
		}
		else if (AVIFileType == TYPEAVI)
		{
			*NewDIB = (HANDLE)HIWORD((DWORD)(lpbi));	
			*ShouldDeleteBM = FALSE;
		}
		else
			*ShouldDeleteBM = TRUE;
/*			if (TraceOn)		
			SaveDIB (*NewDIB,"c:\\ortho2.bmp");*/ 
		if ((!FastOrthos || MAXORTHOBUFS > 1) && !*ShouldDeleteBM)
		{
			*NewDIB = CopyDib (*NewDIB);
			*ShouldDeleteBM = TRUE;
		}	
//			CopyDib2(NewDIB,phDibInfo, phImage);
		AdjustDIBColors (*NewDIB); 
	}
	else
	{
	 	CloseOrthoAVI();
		return FALSE;
	}
	 if (!FastOrthos && AVIFileType == TYPEAVI) 
	 	CloseOrthoAVI();
	return TRUE;
}

void CloseOrthoAVI (void)
{   char	str[256];
	
	switch (AVIFileType)
	{
		case TYPEAVI:
		if (pget)                                               
			AVIStreamGetFrameClose(pget);
		if (pstream) 
			AVIStreamRelease( pstream ); 
		if (pfile)
		{
			AVIFileRelease( pfile ); 
			AVIFileExit();
		}
		break;   
		
		case TYPEGCI:
		ICDecompressEnd(hIC);
		ICClose (hIC);
		_lclose (GCIFid);
		break;
		
		default:
		break;
	}
	pget = 0;
	pstream = 0;
	pfile = 0;
	CurOrthoFile[0]=0;
	AVIFileType = 0;
	return;
}

long NumDIBColors (HANDLE hDib)
{   
	HANDLE	hHistBuf;
   	LPLONG	pHist, pColor;
    BYTE __huge *startrow; 
    BYTE	*startimage, *color;
    WORD	irow,icol,BytesPerPel, MaxVal;
    long	rowlen; 
    LPBITMAPINFOHEADER	lpbi;
	long	numc=0;  
	short	i;
	
    lpbi = GlobalLock(hDib);
    if (lpbi->biBitCount != 8)
    	return 0;
    hHistBuf = GSSiGlobAlloc (GHND,1024);   
    pHist = GlobalLock (hHistBuf);
     
    BytesPerPel = 1;

	startimage = (LPSTR) lpbi + (lpbi->biSize+lpbi->biClrUsed*sizeof(COLORREF));
	startrow = startimage;
	irow = lpbi->biHeight;
	rowlen = lpbi->biWidth*BytesPerPel;
	if (rowlen%4) rowlen += (4-rowlen%4);
	while (irow--)
	{                      
		icol = lpbi->biWidth;    
		color = startrow;
		while (icol--)
		{
			pColor = pHist + *color++; 
			(*pColor)++;  
 		}
		startrow += rowlen;
	}
	for (i=0;i<256;i++,pHist++)
		if (*pHist)
			numc++;
    GSSiGlobUlFree (&hHistBuf);
    GlobalUnlock (hDib);
	return numc;
}

void AdjustDIBColors (HANDLE hDib)

{
    RGBTRIPLE	color;
    BYTE __huge *startrow; 
    BYTE	*startimage;
    HANDLE h;
    DWORD cnt;                         
    WORD	irow,icol,BytesPerPel, MaxVal;
    long	rowlen; 
	short	RFac, GFac, BFac, Intensity;
    RGBTRIPLE	_huge	*rgb24;             
    LPBITMAPINFOHEADER	lpbi;
	COLOR16 __huge *rgb16; 
     
    lpbi = GlobalLock(hDib);
	if (lpbi->biBitCount <= 8)
		goto Exit;
	RFac = GetGlobalLVal ("[%ORTHORED]");
	GFac = GetGlobalLVal ("[%ORTHOGREEN]");
	BFac = GetGlobalLVal ("[%ORTHOBLUE]");
	Intensity = GetGlobalLVal ("[%ORTHOINTENSITY]");
    if (RFac == 0 && GFac == 0 && BFac == 0 && Intensity == 0) return; 
    if (RFac<0)
    	RFac = -(RFac + 100);
    else if (RFac>0)
    	RFac *= log((double)RFac)/log(100);
    if (GFac<0)
    	GFac = -(GFac + 100);
    else
    	GFac *= log((double)GFac)/log(100);
    if (BFac<0)
    	BFac = -(BFac + 100);
    else
    	BFac *= log((double)BFac)/log(100);
    if (Intensity<0)
    	Intensity = -(Intensity + 100); 
    else
    	Intensity *= log((double)Intensity)/log(100);
    BytesPerPel = lpbi->biBitCount/8;
    MaxVal = lpbi->biBitCount;
	switch (lpbi->biBitCount)
    {
    	case 24:
			MaxVal = 255;
			break;
		case 16:
			MaxVal = 31;
			break;
	}
    if (lpbi->biBitCount>8)
    {
    	startimage = (LPSTR) lpbi + (lpbi->biSize+lpbi->biClrUsed*sizeof(COLORREF));
    	startrow = startimage;
    	irow = lpbi->biHeight;
    	rowlen = lpbi->biWidth*BytesPerPel;
    	if (rowlen%4) rowlen += (4-rowlen%4);
    	while (irow--)
    	{                      
			rgb24 = startrow; 
			rgb16 = startrow; 
    		icol = lpbi->biWidth;
    		while (icol--)
    		{
		    switch (lpbi->biBitCount)
			    {
			    	case 24:
	    				color = *rgb24;
	    				break;
	    			case 16:
	    				color.rgbtBlue = rgb16->b;
	    				color.rgbtRed = rgb16->r;
	    				color.rgbtGreen = rgb16->g;
	    				break;
	    		}
    		    if (BFac > 0)
    		    	color.rgbtBlue += ((int)(MaxVal-color.rgbtBlue) * BFac)/100;
    		    else
    		    {
    		    	if (BFac < 0)
    		    		color.rgbtBlue = (color.rgbtBlue * (-BFac))/100; 
    		    }
    		    if (GFac > 0)
    		    	color.rgbtGreen += ((int)(MaxVal-color.rgbtGreen) * GFac)/100;
    		    else
    		    {
    		    	if (GFac < 0)
    		    		color.rgbtRed = (color.rgbtGreen * (-GFac))/100;
    		    }
    		    if (RFac > 0)
    		    	color.rgbtRed += ((int)(MaxVal-color.rgbtRed) * RFac)/100;
    		    else 
    		    {
    		    	if (RFac < 0)
    		    		color.rgbtRed = (color.rgbtRed * (-RFac))/100; 
    		    }
    		    if (Intensity>0)
    		    {   
    		    	color.rgbtBlue += ((int)(MaxVal-color.rgbtBlue) * Intensity)/100;
    		    	color.rgbtGreen += ((int)(MaxVal-color.rgbtGreen) * Intensity)/100;
    		    	color.rgbtRed += ((int)(MaxVal-color.rgbtRed) * Intensity)/100;
    		    }
    		    else if (Intensity < 0)
    		    {
    		    	color.rgbtBlue = (color.rgbtBlue * (-Intensity))/100;
    		    	color.rgbtGreen = (color.rgbtGreen * (-Intensity))/100;
    		    	color.rgbtRed = (color.rgbtRed * (-Intensity))/100;
    		    }
			    switch (lpbi->biBitCount)
			    {
			    	case 24:
	    				*rgb24++=color; 
	    				break;
	    			case 16: 
	    				rgb16->b = color.rgbtBlue;
	    				rgb16->g = color.rgbtGreen;
	    				rgb16++->r = color.rgbtRed;
	    				break;
	    		}
    		}
    		startrow += rowlen;
    	}
    }
Exit:
    GlobalUnlock (hDib);
	return;
}

HANDLE ConvertBitmap (LPBITMAPINFOHEADER  lpbi)
{
	LPBITMAPINFOHEADER  lpNewbi;
	HDIB	NewDIB; 
	unsigned short		ipixel, irow,i;
	char	__huge *StartImageOrig;
	char	__huge *StartImageNew;
	long	RowLenNew, RowLenOrig, Sizeimage;
	COLOR16 __huge *lp16; 
	unsigned	char	__huge *icolor;
	                   
	RowLenNew = lpbi->biWidth * 3;
	if (RowLenNew % 4)
		RowLenNew += 4 - (RowLenNew % 4);
		
	RowLenOrig = lpbi->biWidth * 2;
	if (RowLenOrig % 4)
		RowLenOrig += 4 - (RowLenOrig % 4);
		
	Sizeimage =	lpbi->biHeight * RowLenNew;
	NewDIB = GlobalAlloc (GHND, sizeof(BITMAPINFOHEADER) + Sizeimage);
	lpNewbi = GlobalLock (NewDIB);
	*lpNewbi = *lpbi;  
	lpNewbi->biPlanes=1;
	lpNewbi->biBitCount = 24;
	lpNewbi->biSizeImage = Sizeimage;
	StartImageOrig = (LPSTR)lpbi + sizeof (BITMAPINFOHEADER);
	StartImageNew = (LPSTR)lpNewbi + sizeof (BITMAPINFOHEADER);
	
	for (irow=0;irow<lpNewbi->biHeight;irow++)
	{
		icolor = (StartImageNew + (irow * RowLenNew));
		lp16 = (StartImageOrig + (irow * RowLenOrig));
		ipixel = lpNewbi->biWidth;
		while (ipixel--)
		{   
			*icolor++ = lp16->r * 8;
			*icolor++ = lp16->g * 8;
			*icolor++ = lp16++->b * 8;
		}
	}
	
	GlobalUnlock (NewDIB);
	return NewDIB;
	
}

BOOL CopyDib2 (HANDLE hdib,LPHANDLE phDibInfo, LPHANDLE phImage)

{
    BYTE __huge *ps;
    BYTE __huge *pd; 
    BYTE	*startimage;
    HANDLE h;
    DWORD cnt;  
    LPBITMAPINFOHEADER	lpbi; 
    clock_t	starttime, endtime;

//    starttime=clock();
    ps = GlobalLock(hdib);
    lpbi = (LPBITMAPINFOHEADER) ps; 
    cnt = lpbi->biSize+lpbi->biClrUsed*sizeof(COLORREF); 
    startimage = ps + cnt;
    h = GSSiGlobAlloc (GMEM_MOVEABLE,cnt);
    pd = GlobalLock(h);

    while (cnt--)
      *pd++ = *ps++;
    *phDibInfo = h;
    GlobalUnlock (h);
    cnt = lpbi->biSizeImage;
    h = GSSiGlobAlloc (GMEM_MOVEABLE,cnt);
    pd = GlobalLock(h);
    ps = startimage;
    while (cnt--)
      *pd++ = *ps++;
    *phImage = h;
    GlobalUnlock(h); 
    GlobalUnlock(hdib);
//    endtime=clock();  
//    TotCopyTime+=(endtime-starttime);
    return TRUE;
}




