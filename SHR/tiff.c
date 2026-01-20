#include "shr.h" 

/* TIFF object sizes */
#define	TIFFbyte		1
#define	TIFFascii		2
#define	TIFFshort		3
#define	TIFFlong		4
#define	TIFFrational		5

/* TIFF tag names */
#define	NewSubFile		254
#define	SubfileType		255
#define	ImageWidth		256
#define	ImageLength		257
#define	RowsPerStrip		278
#define	StripOffsets		273
#define	StripByteCounts		279
#define	SamplesPerPixel		277
#define	BitsPerSample		258
#define	Compression		259
#define	PlanarConfiguration	284
#define	Group3Options		292
#define	Group4Options	 	293
#define	FillOrder		266
#define	Threshholding		263
#define	CellWidth		264
#define	CellLength		265
#define	MinSampleValue		280
#define	MaxSampleValue		281
#define	PhotometricInterp	262
#define	GrayResponseUnit	290
#define	GrayResponseCurve	291
#define	ColorResponseUnit	300
#define	ColorResponseCurves	301
#define	XResolution		282
#define	YResolution		283
#define	ResolutionUnit		296
#define	Orientation		274
#define	DocumentName		269
#define	PageName		285
#define	XPosition		286
#define	YPosition		287
#define	PageNumber		297
#define	ImageDescription	270
#define	Make			271
#define	Model			272
#define	FreeOffsets		288
#define	FreeByteCounts		289
#define	ColorMap		320
#define Artist  		315
#define DateTime                306
#define HostComputer            316
#define ImageDescription        270
#define Software                305

void flip2 (LPSTR val,short n,BOOL doit)
{
	if (doit)
		 flip (val,n);
	return;
}

BOOL SetupTIFHeader (HFILE Fid, LPHANDLE phDibInfo,LPSHORT pNumStrips, LPHANDLE phImageOffset,LPHANDLE phByteCounts,
					 LPSHORT pRowsPerStrip,LPSHORT pPhotoInterp,
					 LPSHORT pSamplesPerPixel,LPSHORT pPlanarConfig,LPSHORT TIFFCompression,
					 LPSHORT pBitsPerSample,LPSHORT pSkipBits)
{   UINT nRead;
    BITMAPFILEHEADER bmfHead;
    LPBITMAPINFO    pDibInfo; 
    HPULONG	pOffsets; 
    long	RowLen,i; 
    BOOL	HaveMap=FALSE,FlipTif=FALSE;
    DWORD	Value; 
    BYTE	Type1Val;
    LPBYTE	pType1Val;
    long	rtn, TotBytes;  
    HANDLE	hByteCounts;
    short	NumTags,nColors;
    UINT	icolor;
    UINT	*pType3Val, Type3Val; 
    DWORD	Type4Val;  
    LPSHORT pBitsPerSampleOrig=pBitsPerSample;
    struct {
    	unsigned short	numbertype, version;
    	unsigned long	offset;
    		}TIFFHEAD;
    struct {
    	unsigned short	tag, type;
    	unsigned long	length, offset;
    		}TIFFTAG;
    
    if (pSkipBits)
    	*pSkipBits = 0;
    _fmemset (pBitsPerSample,0,3*2);
    *pRowsPerStrip = 0; 
    *pNumStrips = 1;
    *TIFFCompression = 1;
    *pSamplesPerPixel = 0;
    *pPlanarConfig = 0;
    if (!Fid || Fid==HFILE_ERROR) return (FALSE);
    nRead = BigRead (Fid,(HPSTR)&TIFFHEAD,sizeof(TIFFHEAD));
    if (TIFFHEAD.numbertype == 0x4D4D)
    	FlipTif = TRUE;
    else if (TIFFHEAD.numbertype != 0x4949)
    	return FALSE;   
    flip2 ((LPSTR)&TIFFHEAD.version,2,FlipTif);
    flip2 ((LPSTR)&TIFFHEAD.offset,4,FlipTif);
    GSSillseek (Fid,TIFFHEAD.offset,0);
    BigRead (Fid,(HPSTR)&NumTags,2);
   	flip2 ((LPSTR)&NumTags,2,FlipTif);
    *phImageOffset=0;
    *phDibInfo = GSSiGlobAlloc(GAIDNO 1102,GHND,1024+sizeof(BITMAPINFO));
    pDibInfo = (LPBITMAPINFO) GlobalLock(*phDibInfo);  
    pDibInfo->bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
    pDibInfo->bmiHeader.biPlanes = 1;
    pDibInfo->bmiHeader.biCompression = BI_RGB;
    while (NumTags--)
    {
    	BigRead (Fid,(HPSTR)&TIFFTAG,sizeof(TIFFTAG));
    	rtn = GSSillseek (Fid,0,1);      
    	flip2 ((LPSTR)&TIFFTAG.tag,sizeof(TIFFTAG.tag),FlipTif); 
    	flip2 ((LPSTR)&TIFFTAG.type,sizeof(TIFFTAG.type),FlipTif);
    	flip2 ((LPSTR)&TIFFTAG.length,sizeof(TIFFTAG.length),FlipTif);
//    	flip2 (&TIFFTAG.offset,TIFFTAG.length,FlipTif);
    	
    	switch (TIFFTAG.type)
    	{   
    		case 1:
    			pType1Val = (LPBYTE)&TIFFTAG.offset;
    			Value = *pType1Val; 
    			TotBytes = TIFFTAG.length;
    			break;
    		case 3:
    			pType3Val = (UINT *)&TIFFTAG.offset;
    			Value = *pType3Val;        
    			flip2 ((LPSTR)&Value,2,FlipTif);
    			TotBytes = 2*TIFFTAG.length;
    			break;
    		case 4:  
    			Value = TIFFTAG.offset; 
    			flip2 ((LPSTR)&Value,4,FlipTif);
    			TotBytes = 4*TIFFTAG.length;
    			break;
    		default:
    			Value = 0; 
    			TotBytes = 0;
    			break;
    	}
		flip2 ((LPSTR)&TIFFTAG.offset,4,FlipTif);
		switch(TIFFTAG.tag) {
			case SubfileType:
				break;
			case ImageWidth:
				pDibInfo->bmiHeader.biWidth = Value;
				break;
			case ImageLength:
				pDibInfo->bmiHeader.biHeight = Value;
				break;
			case RowsPerStrip:
//				if(type==TIFFlong) fi->rowsperstrip=offset;
//				else fi->rowsperstrip=offset & 0xffffL;   
				*pRowsPerStrip = Value;
				break;
			case StripOffsets:
				if(TIFFTAG.type==TIFFlong)
				{
					*phImageOffset = GSSiGlobAlloc(GAIDNO 1103,GMEM_MOVEABLE,4*TIFFTAG.length);
					pOffsets = (HPLONG)GlobalLock (*phImageOffset);
					if (TIFFTAG.length > 1)
					{
						GSSillseek (Fid,TIFFTAG.offset,0);
						BigRead (Fid,(HPSTR)pOffsets,4*TIFFTAG.length); 
						*pNumStrips = TIFFTAG.length;
						while (TIFFTAG.length--)
							flip2 ((LPSTR)pOffsets++,4,FlipTif);
					}
					else
						*pOffsets = TIFFTAG.offset;
					GlobalUnlock (*phImageOffset);
				} 
				else
					return FALSE;
//				else fi->offset=offset & 0xffffL;
//				fi->count=(int)length;
				break;
			case StripByteCounts:
				if(TIFFTAG.type==TIFFlong)
				{
					*phByteCounts = GSSiGlobAlloc(GAIDNO 1104,GMEM_MOVEABLE,4*TIFFTAG.length); 
					pOffsets = (HPULONG)GlobalLock (*phByteCounts);
					if (TIFFTAG.length > 1)
					{
						GSSillseek (Fid,TIFFTAG.offset,0);
						BigRead (Fid,(HPSTR)pOffsets,4*TIFFTAG.length);
						while (TIFFTAG.length--)
							flip2 ((LPSTR)pOffsets++,4,FlipTif);
					}
					else
						*pOffsets = TIFFTAG.offset;
					GlobalUnlock (*phByteCounts);   
//					GSSiGlobFree (phByteCounts);
				} 
//				if(type==TIFFlong) fi->bytecount=offset;
//				else fi->bytecount= offset & 0xffffL;
				break;
			case SamplesPerPixel:
//				fi->samples=(int)offset; 
				*pSamplesPerPixel = Value;
				break;
			case BitsPerSample: 
				if (TotBytes > 4)
				{
					GSSillseek (Fid,TIFFTAG.offset,0);
					while (TIFFTAG.length--)
					{ 
				    	switch (TIFFTAG.type)
				    	{   
				    		case 1:
				    			BigRead (Fid,&Type1Val,1);
				    			Value = Type1Val; 
				    			break;
				    		case 3:
				    			BigRead (Fid,(HPSTR)&Type3Val,2); 
    			    			flip2 ((LPSTR)&Type3Val,2,FlipTif);
				    			Value = Type3Val; 
				    			break;
				    		case 4:
				    			BigRead (Fid,(HPSTR)&Type4Val,4);
				    			flip2 ((LPSTR)&Type4Val,4,FlipTif);
				    			Value = Type4Val; 
				    			break;
				    		default:
				    			Value = 0; 
				    			break;
				    	}
		    			*pBitsPerSample++=Value;
			    	}
				}
				else
		    		*pBitsPerSample++=Value;
				break;
			case PlanarConfiguration:
//				fi->planarconfig=(int)offset;
				*pPlanarConfig = Value;
				break;
			case Compression:
//				fi->compression=(int)offset; 
				if (Value == 32773)
					*TIFFCompression = 3;  
				else if (Value == 4)
					*TIFFCompression = 4;  
				else if (Value != 1)
				{
					MessageBox (GetFocus(),"Compressed TIFF not supported",NULL,MB_ICONEXCLAMATION);
					goto Fail;
				}
				break;
			case Group3Options:
				break;
			case Group4Options:
				break;
			case FillOrder:
				break;
			case Threshholding:
				break;
			case CellWidth:
				break;
			case CellLength:
				break;
			case MinSampleValue:
				break;
			case MaxSampleValue:
				break;
			case PhotometricInterp: 
				*pPhotoInterp = TIFFTAG.offset;
				break;
			case GrayResponseUnit:
				break;
			case GrayResponseCurve:
				break;
			case ColorResponseUnit:
				break;
			case ColorResponseCurves:
				break;
			case XResolution:
				break;
			case YResolution:
				break;
			case ResolutionUnit:
				break;
			case Orientation:
				break;
			case DocumentName:
				break;
			case PageName:
				break;
			case XPosition:
				break;
			case YPosition:
				break;
			case PageNumber:
				break;
			case ImageDescription:
				break;
			case Make:
				break;
			case Model:
				break;
			case FreeOffsets:
				break;
			case FreeByteCounts:
				break;
			case ColorMap:
				HaveMap=TRUE; 
				nColors = IDNINT (pow (2,*pBitsPerSampleOrig)); 
				GSSillseek (Fid,TIFFTAG.offset,0);
        		for (i=0;i<nColors;i++)
        		{
        			BigRead (Fid,(HPSTR)&icolor,2);  
        			flip2 ((LPSTR)&icolor,2,FlipTif);
        			pDibInfo->bmiColors[i].rgbRed = icolor / 256;
        		}
        		for (i=0;i<nColors;i++)
        		{
        			BigRead (Fid,(HPSTR)&icolor,2);  
        			flip2 ((LPSTR)&icolor,2,FlipTif);
        			pDibInfo->bmiColors[i].rgbGreen = icolor / 256;
        		}
        		for (i=0;i<nColors;i++)
        		{
        			BigRead (Fid,(HPSTR)&icolor,2);  
        			flip2 ((LPSTR)&icolor,2,FlipTif);
        			pDibInfo->bmiColors[i].rgbBlue = icolor / 256;
        		}
//				pos=ftell(fp);
//				fseek(fp,offset,SEEK_SET);
//				for(i=0;i<(1<<fi->bitspersample);++i) {
//					if(i >= 256) break;
//					fi->palette[i*RGB_SIZE+RGB_RED]=
//					    fgetword(fp) >> 8;
//				}
//				for(i=0;i<(1<<fi->bitspersample);++i) {
//					if(i >= 256) break;
//					fi->palette[i*RGB_SIZE+RGB_GREEN]=
//					    fgetword(fp) >> 8;
//				}
//				for(i=0;i<(1<<fi->bitspersample);++i) {
//					if(i >= 256) break;
//					fi->palette[i*RGB_SIZE+RGB_BLUE]=
//					    fgetword(fp) >> 8;
//				}
//				fseek(fp,pos,SEEK_SET);
				break;
		}    
		GSSillseek (Fid,rtn,0);

    } 
    if (!*pRowsPerStrip)
    	*pRowsPerStrip = pDibInfo->bmiHeader.biHeight;
	if (*pPhotoInterp == 2 && *pSamplesPerPixel != 3)
	{
		MessageBox (GetFocus(),"Unsupported TIFF format",NULL,MB_ICONEXCLAMATION);
		goto Fail;
	}
	for (i=0;i<3;i++)
		pDibInfo->bmiHeader.biBitCount += *pBitsPerSampleOrig++;
    
    RowLen = (long)pDibInfo->bmiHeader.biBitCount * (long)pDibInfo->bmiHeader.biWidth;
    if (RowLen%8)
    	RowLen = RowLen/8 + 1;
    else
    	RowLen = RowLen/8;
    if (RowLen%4)
    	RowLen += 4 - RowLen%4;
    pDibInfo->bmiHeader.biSizeImage= (long)pDibInfo->bmiHeader.biHeight * RowLen;
    if (pSkipBits && pDibInfo->bmiHeader.biBitCount == 1 && pDibInfo->bmiHeader.biSizeImage > 16000000L)
    {
    	*pSkipBits = 1;
    	RowLen /= 2;
    	pDibInfo->bmiHeader.biHeight /= 2;
    	pDibInfo->bmiHeader.biWidth /= 2;
	    RowLen = (long)pDibInfo->bmiHeader.biBitCount * (long)pDibInfo->bmiHeader.biWidth;
	    if (RowLen%8)
	    	RowLen = RowLen/8 + 1;
	    else
	    	RowLen = RowLen/8;
	    if (RowLen%4)
	    	RowLen += 4 - RowLen%4;
	    pDibInfo->bmiHeader.biSizeImage= (long)pDibInfo->bmiHeader.biHeight * RowLen;
    }
 	if (!pDibInfo->bmiHeader.biSizeImage)
 		goto Fail;
    if (pDibInfo->bmiHeader.biBitCount > 8) 
    {
    	pDibInfo->bmiHeader.biClrUsed=0;
    	pDibInfo->bmiHeader.biClrImportant=0; 
    }
    if (! pDibInfo->bmiHeader.biClrUsed)
    {
        if (pDibInfo->bmiHeader.biBitCount==1)
        	pDibInfo->bmiHeader.biClrUsed=2;
        else if (pDibInfo->bmiHeader.biBitCount==4)
        	pDibInfo->bmiHeader.biClrUsed=16;
        else
        {
	        if (pDibInfo->bmiHeader.biBitCount==8)
	        {
	        	pDibInfo->bmiHeader.biClrUsed=256;
	        	if (!HaveMap)
	        	for (i=0;i<256;i++)
	        		pDibInfo->bmiColors[i].rgbRed = pDibInfo->bmiColors[i].rgbGreen = pDibInfo->bmiColors[i].rgbBlue = i;	
	        }
	    }  
    }
//    nRead = BigRead (Fid,pDibInfo->bmiColors,(UINT)pDibInfo->bmiHeader.biClrUsed*4); 
    GlobalUnlock (*phDibInfo);
    return (TRUE); 
Fail:
    GSSiGlobUlFree (phDibInfo);
    GSSiGlobUlFree (phImageOffset);
    return FALSE;
}

BOOL DecompressTIFF (HPSTR Data,long length,long explen,short type)
{   
	short n=*Data; 
	HANDLE	handle;
	HPSTR	pData, StartData=Data;   
	long	ldata=0;
	
	if (type != 3)
		return TRUE;  
	handle = GSSiGlobAlloc(GAIDNO 1105,GMEM_MOVEABLE,explen);
	pData = GlobalLock (handle);
	while (length > 0)
	{   
		if (*Data == -128)
			Data++;
		else if (*Data < 0)
		{
			n = -(*Data) + 1;  
			Data++;
			length -= 2;
			while (n--) 
			{
				*pData++ = *Data;
				ldata++;
			}
			Data++;
		}
		else 
		{
			n = (*Data++) + 1;  
			length--;
			while (n--)
			{
				*pData++ = *Data++; 
				length--; 
				ldata++;
			}
		}	
	} 
	GlobalUnlock (handle); 
	pData = GlobalLock (handle);
	hmemmove (StartData,pData,explen);
	GSSiGlobUlFree (&handle);
	return TRUE;
}

HANDLE  BMPFromTIF (LPSTR TiffFile,BOOL Check)
{
	HFILE FidTiff;
	HANDLE hloc, hTIFFOffsets, hTIFFLengths;
	LPBITMAPINFO pDibInfo; 
	LPBITMAPINFOHEADER	lpbi;
	HBITMAP hbm=0;
	unsigned int width,depth,i,j;
	unsigned int bytes;   
	long	filelen,nFileBytes,bmbytes;   
	int		lines,c,rtn;
	
 	char  	*pFileBuf;
 	char  	*pBits;
 	HPSTR		pImage;
 	HANDLE	hFileBuf=0;   
	OFSTRUCTGM	OFStruct;
 	HANDLE	hDibInfo;
 	long	ImageOffset, rowlen, outrowlen, row; 
    short	FirstBufRow, ThisHeight,RowsPerStrp=1,PhotoInterp,SamplesPerPix,PlanarConfig,BitsPerSmple[8];
    short	TIFFCompression=0;
    DWORD	nNumColors=0;
    short	NumStrips, SkipBits;

	
	RGBQUAD argbq[] = {{ 0, 0, 0, 0 }, 
	                   {255, 255, 255, 0 }};

    FidTiff = GSSiOpenFile (TiffFile,&OFStruct,OF_READ); 

    if (FidTiff == HFILE_ERROR) return 0;
    filelen = _filelength(FidTiff);
	if (!SetupTIFHeader (FidTiff, &hDibInfo,&NumStrips,&hTIFFOffsets,&hTIFFLengths,
						 &RowsPerStrp,&PhotoInterp,
						 &SamplesPerPix,&PlanarConfig,&TIFFCompression,
						 BitsPerSmple,&SkipBits)) 
	{   
		GSSiClose2 (&FidTiff);
			return 0;
	}  
				
    pDibInfo = (LPBITMAPINFO)GlobalLock (hDibInfo);  
    lpbi = (LPBITMAPINFOHEADER)pDibInfo;  
    if (Check)
    {   
    	HANDLE	rtn=(HANDLE)1;
    	
		GSSiClose2 (&FidTiff);
    	if (SkipBits)
    		rtn = 0;
    	GSSiGlobUlFree (&hDibInfo);  
    	GSSiGlobFree (&hTIFFOffsets);
    	GSSiGlobFree (&hTIFFLengths);
    	return rtn;
    }
   	nNumColors = lpbi->biClrUsed; 
   	if (nNumColors == 2)
   	{
   		lpbi->biClrUsed = 0;  //????
   	}
    GlobalUnlock (hDibInfo); 
    hDibInfo = GSSiGlobalReAlloc (0,hDibInfo, lpbi->biSize +
                        nNumColors * sizeof(RGBQUAD) +
                        lpbi->biSizeImage, GHND);
    pDibInfo = (LPBITMAPINFO)GlobalLock (hDibInfo);
    lpbi = (LPBITMAPINFOHEADER)pDibInfo;   
    rowlen = lpbi->biSizeImage/lpbi->biHeight;
    if (nNumColors == 2) 
    {
    	if (PhotoInterp == 1)
    		_fmemcpy (pDibInfo->bmiColors,argbq,2*sizeof(RGBQUAD));  
    	else  
    	{
    		pDibInfo->bmiColors[0] = argbq[1];
    		pDibInfo->bmiColors[1] = argbq[0];
    	} 
    	pDibInfo->bmiHeader.biClrUsed = 2;
    }
	ImageOffset = lpbi->biSize + nNumColors * sizeof(RGBQUAD);
//    if (RowsPerStrp == pDibInfo->bmiHeader.biHeight && TIFFCompression < 2)
    {   
		HPLONG	pOffset=(HPLONG)GlobalLock (hTIFFOffsets);  
		HPLONG	pLength=(HPLONG)GlobalLock (hTIFFLengths);  
        HPSTR	pImage=(HPSTR)lpbi + ImageOffset;
        
        long	dboff=0; 
        
        pImage += rowlen * (lpbi->biHeight - 1) + dboff;   		
		row = lpbi->biHeight;
		for (i=0;i<NumStrips;i++,pOffset++,pLength++) 
		{
			long	OutLen=*pLength;
			HANDLE	hDeCompressedData = GSSiGlobAlloc(GAIDNO 1106,GMEM_MOVEABLE,RowsPerStrp * rowlen);
			HPBYTE	pDeCompressedData = GlobalLock (hDeCompressedData);

			GSSillseek (FidTiff,*pOffset,0);
			switch (TIFFCompression)
			{   
				
				case 4: 
				{
					HANDLE	hCompressedData = GSSiGlobAlloc(GAIDNO 1107,GMEM_MOVEABLE,*pLength);
					HPBYTE	pCompressedData = GlobalLock (hCompressedData);
					
					BigRead (FidTiff,pCompressedData,*pLength);  
//					grp4decomp(pCompressedData,*pLength,pDibInfo->bmiHeader.biWidth,RowsPerStrp,pDeCompressedData,&OutLen,SkipBits); 
					outrowlen = OutLen/RowsPerStrp;
					GSSiGlobUlFree (&hCompressedData); 
				}
				break;
				default: 
					outrowlen = pDibInfo->bmiHeader.biSizeImage / pDibInfo->bmiHeader.biHeight;
					BigRead (FidTiff,pDeCompressedData,*pLength);  
				break;
			}
			for (j=0;j<RowsPerStrp;j++)
			{
				if (row) 
				{
					hmemmove (pImage,pDeCompressedData,outrowlen);
					pImage -= rowlen;
					pDeCompressedData += outrowlen;
					row--;
				}
			}
			GSSiGlobUlFree (&hDeCompressedData); 
		} 
		GlobalUnlock (hTIFFOffsets);
		GlobalUnlock (hTIFFLengths);
    }  
	GSSiGlobFree (&hTIFFOffsets);
	GSSiGlobFree (&hTIFFLengths);
    GlobalUnlock (hDibInfo);     
						
	GSSiClose2 (&FidTiff); 
	return hDibInfo;
}

short DisplayTIFFileInRect (HDC hDC,LPSTR ImageFile, RECT Rect, BOOL MaintainAspect)
{
	HANDLE hBM;
 	int	rtn=FALSE; 

	hBM = BMPFromEXT (ImageFile);
//	hBM = BMPFromTIF (ImageFile,FALSE); 
//    SaveDIB (hBM,"c:\\tifftest\\test.bmp");
	if (hBM)
	{
		rtn = DisplayBMInRect2 (hDC,hBM, Rect,0,0,0,0);
		GSSiGlobFree (&hBM);
	}
	return rtn;
}  

/*short DisplayTIFFileInRect (HDC hDC,LPSTR ImageFile, RECT Rect, BOOL MaintainAspect)
{ 
    OFSTRUCTGM    fStruct;
    LPOFSTRUCT  pStruct = &fStruct;
    LPBITMAPINFOHEADER  pDibInfo;
    LPSTR pImage;
    short     rtn;
    HDIB  hDibInfo;
    char	Name[128];
    
    _fstrcpy (Name,ImageFile);
    ExpandText (Name);
    if (!Name[0]) return FALSE;  
    _fstrlwr (Name);
    hDibInfo=BMPFromTIF (Name); 
    SaveDIB (hDibInfo,"c:\\test.bmp");
    if (!hDibInfo)
    {   
    	GSSiMessageBox (0,"Error loading tiff file",Name,MB_ICONEXCLAMATION);
    	return FALSE;
    }
//   if (!LoadBitMap (ImageFile, &hDibInfo, &hImage)) return FALSE;
    pDibInfo = (LPBITMAPINFOHEADER) GlobalLock (hDibInfo);
//  pImage = GlobalLock (hImage);
    
    pImage =(LPSTR) pDibInfo;
    pImage += sizeof(BITMAPINFOHEADER) + pDibInfo->biClrUsed * 4;     
    rtn = DisplayBMInRect (hDC,pDibInfo,pImage, Rect, MaintainAspect); 
    GlobalUnlock (hDibInfo);
    DestroyDIB (hDibInfo); 
    return rtn;
}*/

