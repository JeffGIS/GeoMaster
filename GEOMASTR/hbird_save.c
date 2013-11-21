#include "graphint.h" 
#include "FreeImage.h"
#include "gmextern.h"   
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

static	char	Symnames[2048][16];

int AddToPalette (LPINT plPalette,RGBQUAD *prgbpal,RGBTRIPLE *prgb)
{
	UINT		i;
	COLORREF	color=RGB(prgb->rgbtRed,prgb->rgbtGreen,prgb->rgbtBlue);
	RGBQUAD		qcolor;
	LPLONG		pcolor=(LPLONG)&qcolor;

	qcolor.rgbBlue = prgb->rgbtBlue;
	qcolor.rgbGreen = prgb->rgbtGreen;
	qcolor.rgbRed = prgb->rgbtRed;
	qcolor.rgbReserved = 0;
	for (i=0;i<*plPalette;i++)
	{
		if (prgb->rgbtBlue == 255)
		{
			if (*(LPLONG)&prgbpal[i] == *pcolor)
				return i;
		}
		else
		{
			if (*(LPLONG)&prgbpal[i] == *pcolor || RGBQUADDist (prgbpal[i],qcolor) < 10)
				return i;
		}
	}
	if (*plPalette <1024)
	{
		prgbpal[*plPalette] = qcolor;
		return ((*plPalette)++);
	}
	return -1;
}

int CompressByteArray (LPBYTE pMem,LPBYTE pMemCmp,int lMem)
{
	int	lCmp=0;
	int	lNonRun;
	USHORT	nRun=0;
	signed char	len2;
	int	nNonRun=0;
	LPBYTE	pShort = (LPBYTE)pMem;
	LPBYTE	pEnd = pShort + lMem -1;
	LPBYTE  pEndSeg = min ((int)(pShort + 128 - 1),(int)pEnd);
	LPBYTE	pCmpShort = (LPBYTE)pMemCmp;
	LPBYTE	pStart=pShort;
	LPBYTE	pStartRun=pShort;

	do
	{
		do
		{
			nRun++;
			pShort++;
		}while (pShort < pEndSeg && nRun < 128 && *pShort == *pStartRun);
		if (nRun > 2)
		{
			lNonRun = (int)pStartRun - (int)pStart;
			if (lNonRun > 0)
			{
				len2 = lNonRun;
				memcpy (pMemCmp,&len2,1);
				pMemCmp ++;
				lCmp ++;
				memcpy (pMemCmp,pStart,lNonRun);
				pMemCmp += lNonRun;
				lCmp += lNonRun;
			}
			len2 = -nRun;
			memcpy (pMemCmp,&len2,1);
			pMemCmp ++;
			lCmp ++;
			memcpy (pMemCmp,pStartRun,1);
			pMemCmp ++;
			lCmp ++;
			pStart = pStartRun = pShort;
			pEndSeg = min ((int)(pShort + 128 - 1),(int)pEnd);
			nRun = 0;
		}
		else if (pShort < pEndSeg)
		{
			nRun = 0;
			pStartRun = pShort;
		}
		else
		{
			lNonRun = (int)pShort - (int)pStart;
			if (lNonRun > 0)
			{
				len2 = lNonRun;
				memcpy (pMemCmp,&len2,1);
				pMemCmp ++;
				lCmp ++;
				memcpy (pMemCmp,pStart,lNonRun);
				pMemCmp += lNonRun;
				lCmp += lNonRun;
			}
			pStart = pStartRun = pShort;
			pEndSeg = min ((int)(pShort + 128 - 1),(int)pEnd);
			nRun = 0;
		}
	}while (pStart < pEnd);
	return lCmp;
}
int CompressByteArray2 (LPBYTE pMem,LPBYTE pMemCmp,int lMem)
{
	int	lCmp=0;
	int	lNonRun;
	USHORT	nRun=0;
	SHORT	len2;
	int	nNonRun=0;
	LPSHORT	pShort = (LPSHORT)pMem;
	LPSHORT	pEnd = pShort + (lMem/2 -1);
	LPSHORT pEndSeg = min ((int)(pShort + SHRT_MAX - 1),(int)pEnd);
	LPSHORT	pCmpShort = (LPSHORT)pMemCmp;
	LPSHORT	pStart=pShort;
	LPSHORT	pStartRun=pShort;

	do
	{
		do
		{
			nRun++;
			pShort++;
		}while (pShort < pEndSeg && nRun < SHRT_MAX && *pShort == *pStartRun);
		if (nRun > 2)
		{
			lNonRun = (int)pStartRun - (int)pStart;
			if (lNonRun > 0)
			{
				len2 = lNonRun/2;
				memcpy (pMemCmp,&len2,2);
				pMemCmp += 2;
				lCmp += 2;
				memcpy (pMemCmp,pStart,lNonRun);
				pMemCmp += lNonRun;
				lCmp += lNonRun;
			}
			len2 = -nRun;
			memcpy (pMemCmp,&len2,2);
			pMemCmp += 2;
			lCmp += 2;
			memcpy (pMemCmp,pStartRun,2);
			pMemCmp += 2;
			lCmp += 2;
			pStart = pStartRun = pShort;
			pEndSeg = min ((int)(pShort + SHRT_MAX - 1),(int)pEnd);
			nRun = 0;
		}
		else if (pShort < pEndSeg)
		{
			nRun = 0;
			pStartRun = pShort;
		}
		else
		{
			lNonRun = (int)pShort - (int)pStart;
			if (lNonRun > 0)
			{
				len2 = lNonRun/2;
				memcpy (pMemCmp,&len2,2);
				pMemCmp += 2;
				lCmp += 2;
				memcpy (pMemCmp,pStart,lNonRun);
				pMemCmp += lNonRun;
				lCmp += lNonRun;
			}
			pStart = pStartRun = pShort;
			pEndSeg = min ((int)(pShort + SHRT_MAX - 1),(int)pEnd);
			nRun = 0;
		}
	}while (pStart < pEnd);
	return lCmp;
}

BOOL ConvertHBirdImages (LPSTR InName, LPSTR OutName)
{
	char	OutFile[MAX_PATH]="c:\\temp\\temp1.bmp";
	char	MapImageFile[MAX_PATH];
	char	MapImageFileOut[MAX_PATH];
	char	bmpname[MAX_PATH];
	HANDLE	hDB = OpenGWDatabase (InName,BT_READ);  
	HANDLE	hDBOut;
	LPSTR	pDot,pDot2; 
	long	Offsetlong, MapID, Offset;  
	BOOL	rtn=FALSE;
	HFILE	FidOut;
	UINT	row, col, i;
	LPBITMAPINFOHEADER pbi;
	BITMAPFILEHEADER bifh;
	HFILE	Fid;
	char	str[256];
	long	SymbolNumberColor = RGB(0,128,255);	
	int		nSymColors;
	int		Flag=TIFF_ADOBE_DEFLATE;
	int		TotTifLen=0;
	int		TotBMPLen=0;
	int		loc;

	strcpy (MapImageFile,InName);
	strcpy (MapImageFileOut,OutName);
	pDot = _fstrrchr (MapImageFile,'.');
	pDot2 = _fstrrchr (MapImageFileOut,'.');
	CreateGWDDatabase (OutName,1,FALSE,0,0,InName);
	hDBOut = OpenGWDatabase (OutName,BT_WRITE);  
	if (hDB)
	{
	    LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
	    LPMAPFILE   pMAPFILE = (LPMAPFILE)&lpGWDHead->GWDData;  
		LPGWDHEADER lpGWDHeadOut = (LPGWDHEADER)GlobalLock (hDBOut);
		LPMAPFILE   pMAPFILEOut = (LPMAPFILE)&lpGWDHeadOut->GWDData;  
		int	pos=BT_FIRST;
		int	nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
		int	Curloc = 0;

		CreateStatusWindow (hWndMain,1,"Convert Images");
		    
        while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&MapID,pos,BT_ANY, (LPSTR)&Offset)) 
        {
			pos = BT_NEXT;
//Offset = 105488;
//Offset = 1817328;
        	FillGWDData (lpGWDHead,Offset);
			*pMAPFILEOut = *pMAPFILE;
			
		    sprintf (pDot,"%i.bin",pMAPFILE->SubDir);
		    sprintf (pDot2,"%i.bin",pMAPFILE->SubDir);
			{
				HFILE	FidBIN=GSSiOpenFile (MapImageFile,0,OF_READ);
				
				_fstrcpy (pDot,".gmd");	 
				if (FidBIN != HFILE_ERROR)
				{
					HANDLE	hMem=GSSiGlobAlloc (0,GMEM_MOVEABLE,pMAPFILE->Size);
					HPSTR	pMem=GlobalLock (hMem);
					
					GSSillseek2 (FidBIN,pMAPFILE->Loc,0);
					BigRead (FidBIN,pMem,pMAPFILE->Size);
					FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);
					if (FidOut != HFILE_ERROR)
					{
						BigWrite (FidOut,pMem,pMAPFILE->Size,-1);
						GSSiClose (FidOut);
						{
							HDIB32 dibin = BMPHandleFromEXT (OutFile);
							int	Width=FreeImage_GetWidth (dibin);
							int	Height=FreeImage_GetHeight (dibin);
							int	bpp=FreeImage_GetBPP (dibin);
							int BitsPerPixel=8, rowlen;
							BOOL rtn=FALSE;
							FIBITMAP *dib = FreeImage_Allocate (Width,Height,BitsPerPixel,0,0,0);
							//FIBITMAP *dibt = FreeImage_Allocate (16,16,BitsPerPixel,0,0,0);
							RGBQUAD	*rgbpalx = FreeImage_GetPalette (dib);
							RGBTRIPLE	*rgb;
							LPBYTE	pByte = (LPBYTE)FreeImage_GetBits (dib);
							int	lPalette=0, iPalette;
							RGBQUAD	rgbpal[1024];
							double	mind,maxd;
							UINT	i,j,ii;
							int		nless10=0;

							pbi = FreeImage_GetInfoHeader (dibin);
							rowlen = pbi->biSizeImage/Height;
							for (row = 0;row < Height; row++)
							{
								rgb = (RGBTRIPLE*)FreeImage_GetScanLine(dibin,row);
								pByte = (LPBYTE)FreeImage_GetScanLine(dib,row);
								for (col = 0; col < Width; col++)
								{
									iPalette = AddToPalette (&lPalette,rgbpal,(RGBTRIPLE*)&rgb[col]);
									pByte[col] = iPalette;
								}
							}
							nSymColors = 0;
							for (i=0;i<lPalette;i++)
								if (rgbpal[i].rgbBlue == 255 && rgbpal[i].rgbGreen != 255 && rgbpal[i].rgbRed != 255)
								{
									long	c=RGB(rgbpal[i].rgbRed,rgbpal[i].rgbGreen,rgbpal[i].rgbBlue);
									long	SymNum = c - SymbolNumberColor;
									nSymColors++;
								}
/*							mind=DBL_MAX;
							maxd=0;
							for (i=0;i<lPalette-1;i++)
								for (j=i+1;j<lPalette;j++)
								{
									if ((rgbpal[i].rgbBlue == 255 && rgbpal[i].rgbGreen != 255 && rgbpal[i].rgbRed != 255) ||
										(rgbpal[i].rgbBlue == 255 && rgbpal[i].rgbGreen != 255 && rgbpal[i].rgbRed != 255))
									{
										ii=1;
									}
									else
									{
										double d=RGBQUADDist (rgbpal[i],rgbpal[j]);

										if (d < 10)
											nless10++;
										sprintf (str,"%i %i %i %i %i %i %i",IDNINT(d),rgbpal[i].rgbRed,rgbpal[j].rgbRed,rgbpal[i].rgbGreen,rgbpal[j].rgbGreen,rgbpal[i].rgbBlue,rgbpal[j].rgbBlue);
										AppendFile ("c:\\temp\\colordist.txt",str);
										mind = min (d,mind);
										maxd = max (d,maxd);
									}
								}

							sprintf (str,"%f %f",mind,maxd);
							AppendFile ("c:\\temp\\colordist.txt",str);*/

								
							pbi = FreeImage_GetInfoHeader (dib);
							sprintf (str,"%i\t%i\t%i",lPalette,nSymColors,Offset);
							AppendFile ("c:\\temp\\palettelen.txt",str);
							if (lPalette > 256)
							{
								sprintf (bmpname,"c:\\temp\\temp%i.bmp",Offset);
								GMFIBMPHandleToEXT (bmpname,dibin,0);
							}
							else if (nSymColors > 256)
							{
							/*GMFIBMPHandleToEXT ("c:\\temp\\temp.bmp",dib,0);
							Fid = GSSiOpenFile ("c:\\temp\\temp5.bmp",0,OF_CREATE);
							pbi->biSizeImage =  WIDTHBYTES((DWORD)pbi->biWidth * pbi->biBitCount) * pbi->biHeight;
							memset (&bifh,0,sizeof(BITMAPFILEHEADER));
							memcpy (&bifh.bfType,"BM",2);
							bifh.bfSize = sizeof(BITMAPINFOHEADER) + 1024 + pbi->biSizeImage;
							BigWrite (Fid,&bifh,sizeof(BITMAPFILEHEADER),-1);
							BigWrite (Fid,pbi,sizeof(BITMAPINFOHEADER),-1);
							BigWrite (Fid,rgbpal,1024,-1);
							pByte = (LPBYTE)FreeImage_GetBits (dib);
							BigWrite (Fid,pByte,pbi->biSizeImage,-1);
							GSSiClose (Fid);*/
								sprintf (bmpname,"c:\\temp\\temp%i.bmp",Offset);
								GMFIBMPHandleToEXT (bmpname,dibin,0);
							/*GMFIBMPHandleToEXT ("c:\\temp\\temp3.bmp",dibt,0);*/
							//FreeImage_Unload (dibt);
							}
							else
							{
								HANDLE	hMem, hMemCmp;
								LPSTR	pMem, pMemCmp;
								int		l, lfe, lbin,lMemCmp;
								HFILE	FidBINOut;

								memset (rgbpalx,0,256*sizeof(RGBQUAD));
								for (i=0;i<lPalette;i++)
									rgbpalx[i] = rgbpal[i];
								pbi->biClrImportant = 256;
								//sprintf (bmpname,"c:\\temp\\temp.tif",Offset);
								//GMFIBMPHandleToEXT (bmpname,dib,TIFF_ADOBE_DEFLATE);
								//TotTifLen += GSSiLength (bmpname);
								sprintf (bmpname,"c:\\temp\\temp.tif",Offset);
								GMFIBMPHandleToEXT (bmpname,dib,TIFF_ADOBE_DEFLATE);
								lfe = GSSiLength (bmpname);
								//Fid = GSSiOpenFile (bmpname,0,OF_READ);
								//l = GSSillseek (Fid,0,2);
								//GSSillseek (Fid,0,0);
								l = pbi->biHeight * pbi->biWidth;
								hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,l);
								pMem = GlobalLock (hMem);
								hMemCmp = GSSiGlobAlloc (0,GMEM_MOVEABLE,l+1024);
								pMemCmp = GlobalLock (hMem);
								for (row = 0;row < Height; row++)
								{
									pByte = (LPBYTE)FreeImage_GetScanLine(dib,row);
									memcpy (pMem,pByte,pbi->biWidth);
									pMem += pbi->biWidth;
								}
								GlobalUnlock (pMem);
								pMem = GlobalLock (hMem);
							 	lbin = CompressBinaryRecord (pMem,pMemCmp,l); 	    
								lMemCmp = CompressByteArray (pMem,pMemCmp,l);
								lMemDeCmp = DeCompressByteArray (pMemCmp,pMem,lMemCmp);
							 	lbin = CompressBinaryRecord (pMemCmp,pMem,lMemCmp); 	    
								//BigRead (Fid,pMem,l);
								//GSSiClose (Fid);
								if ((FidBINOut=GSSiOpenFile (MapImageFileOut,0,OF_READWRITE)) == HFILE_ERROR)
									FidBINOut=GSSiOpenFile (MapImageFileOut,0,OF_CREATE);
								loc = GSSillseek (FidBINOut,0,2);
								pMAPFILEOut->Loc = loc;
								pMAPFILEOut->Size = l;
								pMAPFILEOut->Format = 1;
								BigWrite (FidBINOut,pMem,l,-1);
								GSSiClose (FidBINOut);
								GSSiGlobUlFree (&hMem);
								GWDReplaceRecord (lpGWDHeadOut,0,NULL,-1); 
								//TotBMPLen += (GSSiLength (bmpname)-256*sizeof(RGBQUAD)+lPalette*sizeof(RGBQUAD)-sizeof(BITMAPFILEHEADER)-sizeof(BITMAPINFOHEADER));
								
							}
							FreeImage_Unload (dib);
							FreeImage_Unload (dibin);
						}
						rtn=TRUE;
					}
					GSSiClose (FidBIN);  
					GSSiGlobUlFree (&hMem);
				}
			} 
			if (!StatusWindowUpdate (NULL,NULL, nRecs,Curloc++))
				break;
		}
		DestroyStatusWindow(0);  
		ContinueProcessing = TRUE;
		GlobalUnlock (hDB);
	    CloseGWDatabase (hDB);   
		GlobalUnlock (hDBOut);
	    CloseGWDatabase (hDBOut);   
	}
	return rtn;
}	

BOOL ConvertHBirdColors (HDIB32 hDib)
{
	RGBQUAD	*pal, Black = {0,0,0,0};
	LPBITMAPINFOHEADER pbi;
	long	SymbolNumberColor = RGB(0,128,255);	
	static	BOOL	Loaded=FALSE;
	int		symnum;
	int		i, ii;

	if (!hDib)
		return FALSE;
	if (FreeImage_GetBPP (hDib) != 8)
		return FALSE;
	if (!Loaded)
	{
		HFILE Fid = GSSiOpenFile ("[%DL]\\symdump.txt",0,OF_READ);
		char	str[130];

		while (fgetstring (str,128,Fid))
		{
			LPSTR ptab2 = strrchr (str,'\t');
			LPSTR ptab1 = strchr (str,'\t');

			*ptab1 = 0;
			ptab2++;
			symnum = atoi (ptab2);
			if (symnum > 0 && symnum < 2048)
				strcpy (Symnames[symnum],str);
		}
		GSSiClose (Fid);
		Loaded = TRUE;
	}

	pbi = FreeImage_GetInfoHeader (hDib);
	pal = FreeImage_GetPalette (hDib);
	for (i=0;i<pbi->biClrUsed;i++,pal++)
	{
		if (pal->rgbBlue == 255	&& pal->rgbGreen != 255 && pal->rgbRed != 255)
		{
			long	c=RGB(pal->rgbRed,pal->rgbGreen,pal->rgbBlue);
			long	SymNum = c - SymbolNumberColor;

			if (SymNum >0 && SymNum < 2048 && !strnicmp (Symnames[SymNum],"LKC",3))
				ii=1;//*pal = Black;

		}
	}
	return TRUE;
}
