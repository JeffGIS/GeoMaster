#include "shr.h"

typedef struct	{
	char	manufacturer;
	char	version;
	char	encoding;
	char	bits_per_pixel;
	int	xmin,ymin;
	int	xmax,ymax;
	int	hres;
	int	vres;
	char	palette[48];
        char	reserved;
	char	colour_planes;
	int	bytes_per_line;
	int	palette_type;
	char	filler[58];
		} PCXHEAD;


HANDLE  BMPFromPCX (LPSTR ImageFile)
{
	HFILE FidPCX;
	HANDLE hloc;
	LPBITMAPINFO pbmi;
	HBITMAP hbm=0;
	PCXHEAD header;			/* where the header lives */
	unsigned int width,depth,i;
	unsigned int bytes;   
	long	filelen,nFileBytes,bmbytes;   
	int		lines,c,rtn;
	
 	char  	*pFileBuf;
 	char  	*pBits;
 	LPSTR		pImage;
 	HANDLE	hFileBuf=0, hBits=0;   
 	OFSTRUCT	OFStruct;  
 	HANDLE	hDibInfo;
 	long	ImageOffset, rowlen; 

	
	RGBQUAD argbq[] = {{ 0, 0, 0, 0 }, 
	                   {255, 255, 255, 0 }};

    FidPCX = GSSiOpenFile (ImageFile,&OFStruct,OF_READ); 

    if (FidPCX == HFILE_ERROR) return 0;
    filelen = _filelength(FidPCX);
	if(BigRead(FidPCX,(HPSTR)&header,sizeof(PCXHEAD))==sizeof(PCXHEAD)) 
	{
		/* check to make sure it's a picture */
		if(header.manufacturer==0x0a) {
			/* allocate a big buffer */
			width = (header.xmax-header.xmin)+1;
			depth = (header.ymax-header.ymin)+1;
			bytes=header.bytes_per_line; 
			bmbytes = bytes;
			if (bytes%4)
				bmbytes += 4 - bytes%4;
			nFileBytes = filelen-sizeof(PCXHEAD);   
			hFileBuf = GSSiGlobAlloc ( 406,GMEM_MOVEABLE,nFileBytes);
			pFileBuf = GlobalLock (hFileBuf);
            hBits = GSSiGlobAlloc ( 407,GHND,sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 2)+
            							(long)bmbytes*(long)depth);
            pbmi = (LPBITMAPINFO)GlobalLock (hBits); 
            pImage = (LPSTR)pbmi; 
			pImage += (sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 2));
            BigRead (FidPCX,pFileBuf,filelen-sizeof(PCXHEAD));
			/* unpack the file */    
			lines = depth;
			while (lines--)
			{   unsigned int	n;
			
				n=0;
				pBits = pImage;
				pBits += bmbytes*(lines); 
				do{
					/* get a key byte */
			        c=*pFileBuf & 0xff; 
			        pFileBuf++;
					/* if it's a run of bytes field */
					if((c & 0xc0) == 0xc0) {
						/* and off the high bits */
						i=c & 0x3f;
						/* get the run byte */
						c=*pFileBuf;
						pFileBuf++;
						/* run the byte */
						while(i--)
						{
							 *pBits=c;
							 pBits++;  
							 n++;
						}
					}
					/* else just store it */
					else
					{
						*pBits=c;
						pBits++;  
						n++;
					} 
				}while (n<bytes);
			}
			
			pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			pbmi->bmiHeader.biWidth = width;
			pbmi->bmiHeader.biHeight = depth;
			pbmi->bmiHeader.biSizeImage = pbmi->bmiHeader.biHeight * bmbytes;
			pbmi->bmiHeader.biPlanes = 1;
			pbmi->bmiHeader.biBitCount = header.bits_per_pixel;
			pbmi->bmiHeader.biCompression = BI_RGB; 
			pbmi->bmiHeader.biClrUsed = 2;
						
			_fmemcpy(pbmi->bmiColors, argbq, sizeof(RGBQUAD) *2);    
			GlobalUnlock (hBits);
		}
	}
	GSSiGlobUlFree (&hFileBuf);
	GSSiClose (FidPCX); 
	return hBits;
}

short  DisplayPCXFileInRect (HDC hDC,LPSTR ImageFile, RECT Rect, BOOL MaintainAspect)
{
	HANDLE hBM;
 	int	rtn=FALSE; 
 	
	hBM = BMPFromPCX (ImageFile); 
//    SaveDIB (hBM,"c:\\tifftest\\test.bmp");
	if (hBM)
	{
		rtn = DisplayBMInRect2 (hDC,hBM, Rect,0,0,0,0);
		GSSiGlobFree (&hBM);
	}
	return rtn;
}  


