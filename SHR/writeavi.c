
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
#include "graphint.h"  
#define AVIIF_KEYFRAME      0x00000010L // this frame is a key frame.
//#include <compman.h>
#include <vfw.h>
//#include <avifile.h>

#define	TYPEAVI	1
#define	TYPEGCI	2
#define TYPEGCO 3

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

#include "gmextern.h"

static short	AVIFileType=0;
static HANDLE	hIC=NULL;
static HFILE	GCIFid=-1;
static PAVIFILE	pfile;
static BOOL	HaveInit=FALSE;
static long	TotCopyTime=0;
static char	CurOrthoFile[128]="";
static AVISTREAMINFO	strhdr;
static PAVISTREAM	pstream=0;
static PGETFRAME	pget=0;  
static BOOL	UseExternalCompression=FALSE;


BOOL InitAVIOut (LPSTR Name, LPBITMAPINFOHEADER alpbi,LPHANDLE phFile);

#define	MAXFRAME 640L * 480L * 3L + 1024L

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
	OFSTRUCTGM	OFStruct;
    LPBITMAPINFOHEADER	lpbiHead;  
    HANDLE	hHeader; 
    DWORD	CompressorID; 
    BOOL	rtn=FALSE; 
    char	str[32],DefaultCompressor[8];
	
	if (lpbi->biBitCount > 8)
		_fstrcpy (DefaultCompressor,"IV50");
	else 
	{
		UseExternalCompression = FALSE;
		_fstrcpy (DefaultCompressor,"IV32"); 
	}
	GetGlobalCVal ("[%GCICOMPRESSOR]",str,DefaultCompressor);
	CompressorID = mmioFOURCC(str[0],str[1],str[2],str[3]);
//   	AVIFileInit();
	pAVIFile = (LPAVIFILE)GlobalLock (hFile);
	pAVIFile->GCIFid = -1;
	pAVIFile->hIC = ICOpen (ICTYPE_VIDEO,CompressorID,ICMODE_COMPRESS);
	if (!pAVIFile->hIC)
	{
		GSSiMessageBox (0,"Failed to open compressor",str,MB_ICONEXCLAMATION,0);
		goto Exit;
	}
	hHeader = GSSiGlobAlloc ( 372,GMEM_MOVEABLE,sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD));
	lpbiHead = (LPBITMAPINFOHEADER)GlobalLock (hHeader);
	ICRtn = ICCompressGetFormat(pAVIFile->hIC,lpbi, lpbiHead); 
	ICRtn = ICCompressBegin(pAVIFile->hIC, lpbi,lpbiHead);  
	if (ICRtn != ICERR_OK)
	{   
		GSSiGlobUlFree (&hHeader);
		GSSiMessageBox (0,"Bad compressor format",str,MB_ICONEXCLAMATION,0);
		goto Exit;            
	}
	pAVIFile->GCIFid = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
	lhead = DIBHeadSize (lpbiHead);
	BigWrite (pAVIFile->GCIFid,(HPSTR)&lhead,2,-1); 
	lpbiHead->biCompression = CompressorID;  
	lpbiHead->biSize = 0; //indicates compression is compressor id
	BigWrite (pAVIFile->GCIFid,(HPSTR)lpbiHead,lhead,-1);
	lhead = DIBHeadSize (lpbi);
	BigWrite (pAVIFile->GCIFid,(HPSTR)&lhead,2,-1);
	BigWrite (pAVIFile->GCIFid,(HPSTR)lpbi,lhead,-1);

	GSSiGlobUlFree (&hHeader); 
	rtn = TRUE; 
Exit:
	if (UseExternalCompression)
	{
		if (pAVIFile->hIC)
		{
			ICCompressEnd(pAVIFile->hIC);
			ICClose (pAVIFile->hIC); 
		}  
		AVIFileExit ();
	}
	GlobalUnlock (hFile);
	return rtn;
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
    
	*phFile = GSSiGlobAlloc ( 373,GHND,sizeof(AVIFILE));
	pAVIFile = (LPAVIFILE)GlobalLock (*phFile); 
    if (!UserDefinedImageQuality)
	    pAVIFile->CompressionFactor = 7200;
	else
	    pAVIFile->CompressionFactor = UserDefinedImageQuality;
	
    if (_fstrstr (Name,".gci")) 
    {
    	GlobalUnlock (*phFile);
    	return (CreateGCIFile (Name,alpbi, *phFile));
    }
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
    SetRect(&strhdr.rcFrame, 0, 0,(int)alpbi->biWidth,(int)alpbi->biHeight);		    // rectangle for stream

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

short AVIOut (LPSTR Name,LPBITMAPINFOHEADER lpbi,LPHANDLE hFile,LPLONG pFrame,BOOL UseExCmp)
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
    HANDLE		hHeader=0;   
    short		rtn=1;
    
/*	HeadLen = sizeof(BITMAPINFOHEADER)+alpbi->biClrUsed*sizeof(RGBQUAD);
	image = (LPSTR)alpbi + HeadLen; 
	OutSize = 0.25 * alpbi->biSizeImage;
    hDIB = ICImageCompress (NULL,0,alpbi,image,alpbi,7000,&OutSize);
    alpbi = GlobalLock(hDIB); */
    if (!lpbi->biSizeImage)
    {
    	long	rowlen = (long)lpbi->biWidth * (long)lpbi->biBitCount/8;
    	
    	if (rowlen % 4)
    		rowlen += (4 - (rowlen % 4));
        lpbi->biSizeImage = rowlen * (long)lpbi->biHeight;
    }
    if (!*hFile)
	{   
		UseExternalCompression = UseExCmp;
		if (!InitAVIOut (Name, lpbi,hFile)) 
		{
			GSSiGlobFree (hFile);
			return FALSE;       
		}
	}
	pAVIFile = (LPAVIFILE)GlobalLock (*hFile);
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
	    if (hr != AVIERR_OK)
	    	rtn = FALSE;
		pAVIFile->iframe++;   
		*pFrame = -1;
	} 
	else
	{   
		HANDLE	hCompData=0;
		DWORD	Flags;
		long	lRec; 
		ULONG	NextLen; 
		short	ii;
		
//		hHeader = GSSiGlobAlloc ( 375,GMEM_MOVEABLE,sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD));
		hHeader = GSSiGlobAlloc ( 374,GMEM_MOVEABLE,sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)+lpbi->biSizeImage);
		lpbiHeadOut = (LPBITMAPINFOHEADER)GlobalLock (hHeader);  
/*		{   
			long	lInRec = lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD) +lpbi->biSizeImage;

			lRec = CompressBinaryRecord ((HPSTR)lpbi,(HPSTR)lpbiHeadOut,lInRec);
			pAVIFile->iframe = GSSillseek (pAVIFile->GCIFid,0,2);  
			*pFrame = pAVIFile->iframe;
			NextLen = (ULONG)*pFrame + (ULONG)lRec;
			if (NextLen < (ULONG)LONG_MAX) 
			{
				BigWrite (pAVIFile->GCIFid,(HPSTR)&lRec,4,-1);
				BigWrite (pAVIFile->GCIFid,(HPSTR)lpbiHeadOut,lRec,-1);
			}
			else
				rtn = -1; 
			goto Exit;
		}*/ 
		if (UseExternalCompression)
		{
			if ((rtn = CompressFrameEX (lpbi,lpbiHeadOut,pAVIFile->CompressionFactor)))
			{
			 	pCompressedData = (LPBYTE) lpbiHeadOut +  
						  lpbiHeadOut->biSize +
						  lpbiHeadOut->biClrUsed * sizeof(RGBQUAD); 
			} 
			else
				goto Exit; 
		}
		else
		{
			hCompData = GSSiGlobAlloc ( 374,GMEM_MOVEABLE,lpbi->biSizeImage);
			pCompressedData = GlobalLock (hCompData);
	 		ICRtn = ICCompressGetFormat(pAVIFile->hIC,lpbi,lpbiHeadOut);
			if (ICRtn != ICERR_OK)
				ii=1;
			ICRtn = ICCompress (pAVIFile->hIC,ICCOMPRESS_KEYFRAME,
											lpbiHeadOut,pCompressedData,
											lpbi,pInData,
											NULL,&Flags,0,0,
											pAVIFile->CompressionFactor,NULL,NULL);  
			if (ICRtn != ICERR_OK)
				rtn = -2; 
		} 
		if (rtn == 1)
		{
			pAVIFile->iframe = GSSillseek (pAVIFile->GCIFid,0,2);  
			*pFrame = pAVIFile->iframe;
			lRec = lpbiHeadOut->biSize + lpbiHeadOut->biSizeImage;
			NextLen = (ULONG)*pFrame + (ULONG)lRec;
			if (NextLen < (ULONG)LONG_MAX) 
			{
				BigWrite (pAVIFile->GCIFid,(HPSTR)&lRec,4,-1);
				BigWrite (pAVIFile->GCIFid,(HPSTR)lpbiHeadOut,(size_t)lpbiHeadOut->biSize,-1);
				if (BigWrite (pAVIFile->GCIFid,(HPSTR)pCompressedData,lpbiHeadOut->biSizeImage,-1) != lpbiHeadOut->biSizeImage)
					rtn = 0;
			}
			else
				rtn = -1; 
		}
Exit:
		GSSiGlobUlFree (&hCompData);   
		GSSiGlobUlFree (&hHeader);   
	} 
	GlobalUnlock (*hFile);
    return rtn;
}
	
void AVIOutClose (LPHANDLE phFile)
{  
	LPAVIFILE	pAVIFile;
	
	if (!*phFile)
		return;
	pAVIFile = (LPAVIFILE)GlobalLock (*phFile); 
	
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
    	if (!UseExternalCompression)
    	{
	    	if (pAVIFile->hIC)
	    	{
	    		ICCompressEnd(pAVIFile->hIC);
				ICClose (pAVIFile->hIC);
			} 
		}
		if (pAVIFile->GCIFid >= 0)
			GSSiClose (pAVIFile->GCIFid);           
	}    
    AVIFileExit();
    HaveInit=FALSE;               
    GSSiGlobUlFree (phFile);

    return;
} 

HANDLE CopyDib (HANDLE hdib)
{
    BYTE HUGE *ps;
    BYTE HUGE *pd;
    HANDLE h;
    DWORD cnt; 
    extern	BOOL IgnoreLock;

    if (h = GSSiGlobAlloc ( 376,GMEM_MOVEABLE, cnt = GlobalSize(hdib)))
    {   
    	IgnoreLock = TRUE;
        ps = GlobalLock(hdib);  
        pd = GlobalLock(h);

        while (cnt-- > 0)
           *pd++ = *ps++;

        GlobalUnlock(hdib);
        GlobalUnlock(h);
        IgnoreLock = FALSE;
    }
    return h;
}

BOOL AVIFrameToDIB (LPSTR File, long frame,LPHANDLE NewDIB,LPSHORT ShouldDeleteBM,
					short BitCount, short width, short height)
{
	HPALETTE	hPal, hPalNew = NULL;	// remember to init these
	HRESULT		hResult; 
	static	BOOL	FirstErr=TRUE;
	typedef	struct	{
			 			BITMAPINFOHEADER biHead;
			 			RGBQUAD Colors[256];}	BIHEADER;  
	typedef BIHEADER	FAR	*LPBIHEADER;
	static	BIHEADER	biHeadOut;
	BIHEADER	biHeadIn; 
	LPBIHEADER	lpbiHeadIn;
	LPBITMAPINFOHEADER  lpbi; 
	BITMAPINFOHEADER  	bi;
	LPSTR		lpDIBBits;
	char		Name[MAX_PATH], str[512];   
	LONG	ICRtn;
	short	lhead; 
	OFSTRUCTGM	OFStruct;
	DWORD	CompressorID=mmioFOURCC('M', 'S', 'V', 'C');




	*NewDIB = 0;
	_fstrcpy (Name,File);
	ExpandText (Name);   
    if (DisplayFiles==1)
    {   
    	sprintf (str,"%s: %ld",Name,frame);
    	SetWindowText (hWndMain,str);   
    }
	if (_fstricmp(Name,CurOrthoFile)) 
	{   
		CloseOrthoAVI(); 
    	if (_fstrstr (Name,".gci"))
    	{
			AVIFileInit(); 
			AVIFileType = TYPEGCI;
			GCIFid = GSSiOpenFile (Name,&OFStruct,OF_READ);
			if (GCIFid == HFILE_ERROR)  
				return FALSE;
			BigRead (GCIFid,(HPSTR)&lhead,2);
			BigRead (GCIFid,(HPSTR)&biHeadIn,lhead);  
			if (!biHeadIn.biHead.biSize)
			{   
				biHeadIn.biHead.biSize = 40;
				CompressorID = biHeadIn.biHead.biCompression;
			}
			BigRead (GCIFid,(HPSTR)&lhead,2);
			BigRead (GCIFid,(HPSTR)&biHeadOut,lhead);
			hIC = ICOpen (ICTYPE_VIDEO,CompressorID,ICMODE_DECOMPRESS);
			if (!hIC)
			{
				GSSiClose (GCIFid);  
				if (FirstErr)
				{
					sprintf(str, "Failed to open decompressor.\r\n\r\nTo fix do the following:\r\n\topen a 'command prompt' using a rightclick\r\n\tselect, 'Run as Administrtor'\r\n\t(32bit users can skip the next step)\r\n\ttype: cd C:\\Windows\\SysWOW64\r\n - press enter\r\r\ttype : regsvr32 ir50_32.dll - press enter");
					GSSiMessageBox (0,str,NULL,MB_ICONEXCLAMATION,0);
				}
				FirstErr = FALSE;

				return FALSE; 
			} 
			ICRtn = ICDecompressGetFormat(hIC,&biHeadIn,&biHeadOut);
			if (biHeadOut.biHead.biBitCount == 16 && (!Display16BitColor || Printing ))
			{
				biHeadOut.biHead.biBitCount=24;
				biHeadOut.biHead.biSizeImage=0;
			}
			ICRtn = ICDecompressBegin(hIC, &biHeadIn,&biHeadOut);  
    	}
    	else if (_fstrstr (Name,".gco"))
    	{
			AVIFileType = TYPEGCO;
			GCIFid = GSSiOpenFile (Name,&OFStruct,OF_READ);
			if (GCIFid == HFILE_ERROR)  
				return FALSE;
    	}
    	else
    	{
			AVIFileInit(); 
			AVIFileType = TYPEAVI;
			hResult = AVIFileOpen(&pfile, Name, OF_READ, 0L);
			if (!pfile) 
			{
				return FALSE;
			}
			hResult = AVIFileGetStream(pfile, &pstream, streamtypeVIDEO, 0);
			
			if (!pstream)
			{
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
		}
		_fstrcpy (CurOrthoFile,Name);
	} 
	
	if (AVIFileType == TYPEGCI || AVIFileType == TYPEGCO)
	{   
		long	len, lRec; 
		HANDLE	hCompressedData;
		HPBYTE	pCompressedData, pData;
		LPBIHEADER	lpbi;
		
		GSSillseek (GCIFid,frame,0);
		BigRead (GCIFid,(HPSTR)&lRec,4); 
		if (lRec <= 0 || lRec > MAXFRAME) 
		{   
			HANDLE	hMess=GSSiGlobAlloc ( 377,GMEM_MOVEABLE,256);
			LPSTR	pMess=GlobalLock (hMess);
			static  BOOL showMessage = TRUE;

			if (showMessage)
			{
				sprintf (pMess,"%s:%ld",File,frame);        
				AppendFile ("[%DL]abends\\ortherr.txt",pMess);
				if (GSSiMessageBox (0,pMess, "Error in Ortho File", MB_OKCANCEL, 0) == IDCANCEL)
					showMessage = FALSE;
				GSSiGlobUlFree (&hMess); 
				ContinueProcessing = FALSE;
			}
			return FALSE;
		}
		hCompressedData = GSSiGlobAlloc ( 378,GMEM_MOVEABLE,lRec+1024);
		lpbiHeadIn = (LPBIHEADER)GlobalLock (hCompressedData);
		if (AVIFileType == TYPEGCI)
		{
			BigRead (GCIFid,(HPSTR)lpbiHeadIn,sizeof(BITMAPINFOHEADER));
			if (lpbiHeadIn->biHead.biClrUsed)
				_fmemmove (lpbiHeadIn->Colors,biHeadOut.Colors,256*sizeof(RGBQUAD)); 
			pCompressedData = (HPBYTE)lpbiHeadIn;
			pCompressedData += DIBHeadSize ((LPBITMAPINFOHEADER)lpbiHeadIn);
			BigRead (GCIFid,pCompressedData,lpbiHeadIn->biHead.biSizeImage); 
			if (!biHeadOut.biHead.biSizeImage)
			{
	    		long	rowlen = (long)biHeadOut.biHead.biWidth * (long)biHeadOut.biHead.biBitCount/8;
		    	
	    		if (rowlen % 4)
	    			rowlen += (4 - (rowlen % 4));
				biHeadOut.biHead.biSizeImage = rowlen * (long)biHeadOut.biHead.biHeight;
			}
			*NewDIB = GSSiGlobAlloc ( 379,GMEM_MOVEABLE,
									 biHeadOut.biHead.biSizeImage+DIBHeadSize((LPBITMAPINFOHEADER)&biHeadOut)); 
									   
			lpbi = (LPBIHEADER)GlobalLock (*NewDIB);
			*lpbi = biHeadOut;
			pData = (HPBYTE)lpbi;
			pData += DIBHeadSize ((LPBITMAPINFOHEADER)&biHeadOut);
			ICRtn = ICDecompress(hIC,0,(LPBITMAPINFOHEADER)lpbiHeadIn, pCompressedData,(LPBITMAPINFOHEADER)lpbi,pData);
			GlobalUnlock (*NewDIB); //ICERR_OK 
		}
		else
		{
			BigRead (GCIFid,(HPSTR)lpbiHeadIn,lRec);
			*NewDIB = LoadDIBFromMem ((LPBYTE)lpbiHeadIn,lRec,FIF_JP2,0);
		}
		
		if (CurView && (CurView->HalfTone || CurView->ConvertToGray))
			AdjustDIBColors (*NewDIB); 
		GSSiGlobUlFree (&hCompressedData);
		*ShouldDeleteBM = TRUE; 
		return TRUE;
	}
	else
	{
		if(pget)
		{  
	
			// Get our own personal copy of the DIB 
			lpbi = AVIStreamGetFrame(pget, frame);
		}
	}
	if (AVIFileType == TYPEAVI && lpbi)  
	{   
			
		if (lpbi->biBitCount == 16 && (!Display16BitColor || Printing ))
		{
			*NewDIB = ConvertBitmap16To24 (lpbi); 
			*ShouldDeleteBM = TRUE;
		}
		else
		{
			//*NewDIB = (HANDLE)HIWORD((DWORD)(lpbi));
			*NewDIB = (HANDLE)(DWORD)(lpbi);
			*ShouldDeleteBM = FALSE;
		}
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
{   
	
	switch (AVIFileType)
	{    
		case 0:
			return;
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
		case TYPEGCO:
		GSSiClose (GCIFid);
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
    BYTE HUGE *startrow; 
    BYTE	*startimage, *color;
    WORD	irow,icol,BytesPerPel, MaxVal;
    long	rowlen; 
    LPBITMAPINFOHEADER	lpbi;
	long	numc=0;  
	short	i;
	
    lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
    if (lpbi->biBitCount != 8)
    	return 0;
    hHistBuf = GSSiGlobAlloc ( 380,GHND,1024);   
    pHist = (LPLONG)GlobalLock (hHistBuf);
     
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

void FullScreenPlay(HWND hWnd, LPSTR Name)
{   
	char	str[256];
	
    wsprintf((LPSTR)str,"open %s alias mov",Name);

	mciSendString((LPSTR)str, NULL, 0, hWnd);
	mciSendString("play mov fullscreen", NULL, 0, hWnd); 
	mciSendString("close mov", NULL, 0, hWnd); 
	return;
} 




