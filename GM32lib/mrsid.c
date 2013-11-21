#include "graphint.h"
#include <ltic_api.h>
#include <lt_status.h>
#include <lt_utilstatus.h>
#include <lt_utilstatusstrings.h>
#include "FreeImage.h"

#if USEMRSID
HDIB32 GMMrSidOpen (LPSTR MrSidFile)
{
	LTICImageH	imageHandle=0;

	LT_STATUS st;
	LPSTR	pDot=strrchr (MrSidFile,'.');

	if (!pDot || !stricmp (pDot,".SID"))
		st = ltic_openMrSIDImageFile ( &imageHandle,  MrSidFile);
	else
		st = ltic_openJP2ImageFile	( &imageHandle,  MrSidFile);

	return imageHandle;
} 

DWORD GMMrSidClose (HDIB32 ImageHandle)
{
	LT_STATUS st=ltic_closeImage((LTICImageH)ImageHandle);

	return (DWORD)st;
}

DWORD GMMrSidVersion (LPSTR Version)
{
	DWORD	major,minor,rev,build;
	char	branch[256];
	const	char	*branchadd=branch;
	LT_STATUS st = ltic_getVersion(&major,&minor,&rev,&build,&branchadd);

	*Version = 0;
	if (!st)
	{
		sprintf (Version,"%ld.%ld.%ld.%ld",major,minor,rev,build);
		return TRUE;
	}
	return FALSE;
}

HDIB32 GMMrSidGetImage (HDIB32 ImageHandle,
					   LPDOUBLE pxUpperLeft,
					   LPDOUBLE pyUpperLeft,
					   LPDWORD pWidth,
					   LPDWORD pHeight,
					   LPDOUBLE pMagnification,
					   DWORD	ConvertToGray,
					   DWORD	Intensity,
					   DWORD DisplayErrorMessage)
{
	double xUpperLeft=*(double *)pxUpperLeft;
	double yUpperLeft=*(double *)pyUpperLeft;
	DWORD width=*(DWORD *)pWidth;
	DWORD height=*(DWORD *)pHeight;
	double magnification=*(double *)pMagnification;
	LT_STATUS st;
	LPBYTE	buffers0, buffers1, buffers2,buffers3,pStartImage;
	DWORD	size = width * height;
	long	imagesize, rowlen, NumBands;
	DWORD	row,col;
	HDIB32	rtn=0;
	RGBTRIPLE	*pTriColor;
	RGBQUAD		*pQuadColor;
	FIBITMAP *hDib = NULL;
	BITMAPINFOHEADER *pbi;
	DWORD	i=0;
	void** bufs = (void**)malloc(sizeof(LPBYTE) * 4);
	char	str[256];
	const char*	statusstr;
	double	HT = ((double)Intensity*10)/100;
	BOOL	TrueColor=GetGlobalBVal2 ("[%TRUECOLOR]",TRUE);

	buffers0 = malloc (size);
	memset (buffers0,0xffff,size);
	buffers1 = malloc (size);
	memset (buffers1,0xffff,size);
	buffers2 = malloc (size);
	memset (buffers2,0,size);
	buffers3 = malloc (size);
	memset (buffers3,0,size);
    bufs[0] = buffers0;
    bufs[1] = buffers1;
    bufs[2] = buffers2;
    bufs[3] = buffers3;
	st = ltic_decode((LTICImageH) ImageHandle,
                      xUpperLeft,
                      yUpperLeft,
                      width,
                      height,
                      magnification,
                      bufs);
	if (st)
	{
		if (DisplayErrorMessage)
		{
			statusstr = getRawStatusString (st);
			sprintf (str,"decode st = %ld (%s)\r\n%f,%f,%ld,%ld,%f",(long)st,statusstr,
												xUpperLeft,yUpperLeft,width,height,magnification);
			MessageBox (0,str,"",MB_OK);
		}
		goto Exit;
	}
	NumBands = ltic_getNumBands((LTICImageH) ImageHandle);

	rowlen = width*3;
	if (rowlen%4)
		rowlen += 4-rowlen%4;
	imagesize = height * rowlen;

	hDib = FreeImage_Allocate (width,height,24,0,0,0);
	size = (DWORD)FreeImage_GetDIBSize(hDib);
	pbi  = FreeImage_GetInfoHeader(hDib);
	pbi->biBitCount = 24;
	pbi->biClrImportant = 0;
	pbi->biClrUsed = 0;
	pbi->biCompression = 0;
	pbi->biHeight = height;
	pbi->biWidth = width;
	pbi->biPlanes = 1;
	pbi->biSize = sizeof(BITMAPINFOHEADER);
	pbi->biSizeImage = imagesize;
	pbi->biXPelsPerMeter = 0;
	pbi->biYPelsPerMeter = 0;
	pStartImage = (LPBYTE)(pbi+1);
	switch (NumBands)
	{
	case 4:
		if (TrueColor)
		for (row = 0;row < height;row++)
		{
			i = (height-row-1)*width;
			pTriColor = (RGBTRIPLE*)(pStartImage + rowlen*row);
			for (col = 0;col < width;col++,pTriColor++,i++)
			{
				pTriColor->rgbtBlue = buffers0[i];
				pTriColor->rgbtGreen = buffers1[i];
				pTriColor->rgbtRed = buffers2[i];
			}
		}
		else
		for (row = 0;row < height;row++)
		{
			i = (height-row-1)*width;
			pTriColor = (RGBTRIPLE*)(pStartImage + rowlen*row);
			for (col = 0;col < width;col++,pTriColor++,i++)
			{
				pTriColor->rgbtGreen = pTriColor->rgbtBlue = 0;
				pTriColor->rgbtRed = buffers3[i];
			}
		}
		break;
	default:
		if (Intensity || ConvertToGray)
		for (row = 0;row < height;row++)
		{
			i = (height-row-1)*width;
			pTriColor = (RGBTRIPLE*)(pStartImage + rowlen*row);
			for (col = 0;col < width;col++,pTriColor++,i++)
			{
				pTriColor->rgbtBlue = buffers0[i];
				pTriColor->rgbtGreen = buffers1[i];
				pTriColor->rgbtRed = buffers2[i];
				if (Intensity)
				{
					pTriColor->rgbtRed = pTriColor->rgbtRed + (255 - pTriColor->rgbtRed) * HT;
					pTriColor->rgbtGreen = pTriColor->rgbtGreen + (255 - pTriColor->rgbtGreen) * HT;
					pTriColor->rgbtBlue = pTriColor->rgbtBlue + (255 - pTriColor->rgbtBlue) * HT;
				}
				if (ConvertToGray)
					*pTriColor = ConvertToGrayTriple (*pTriColor);
			}
		}
		else
		for (row = 0;row < height;row++)
		{
			i = (height-row-1)*width;
			pTriColor = (RGBTRIPLE*)(pStartImage + rowlen*row);
			for (col = 0;col < width;col++,pTriColor++,i++)
			{
				pTriColor->rgbtBlue = buffers0[i];
				pTriColor->rgbtGreen = buffers1[i];
				pTriColor->rgbtRed = buffers2[i];
			}
		}
	}
	rtn = hDib;
Exit:
	free (buffers0);
	free (buffers1);
	free (buffers2);
	free (buffers3);
	free (bufs);
	return rtn;
}

DWORD GMMrSidGetImageInfo (HDIB32 ImageHandle,
						   DWORD pWidth,
						   DWORD pHeight,
						   DWORD pColorSpace,
						   DWORD pNumBands,
						   DWORD pDataType,
						   DWORD pMinMag,
						   DWORD pMaxMag,
						   DWORD pIsLocked,
						   DWORD pULX,
						   DWORD pULY,
						   DWORD pXres,
						   DWORD pYres,
						   DWORD pXRot,
						   DWORD pYRot,
						   DWORD pNumMetaRecords
						   )
{
	
	if (!ImageHandle)
		return FALSE;
	*(DWORD *)pWidth = (DWORD)ltic_getWidth((LTICImageH) ImageHandle);
	*(DWORD *)pHeight = (DWORD) ltic_getHeight((LTICImageH) ImageHandle);
	*(DWORD *)pColorSpace = (DWORD) ltic_getColorSpace((LTICImageH) ImageHandle);
	*(DWORD *)pNumBands = (DWORD) ltic_getNumBands((LTICImageH) ImageHandle);
	*(DWORD *)pDataType = (DWORD) ltic_getDataType((LTICImageH) ImageHandle);
	*(double *)pMinMag = ltic_getMinMagnification((LTICImageH) ImageHandle);
	*(double *)pMaxMag = ltic_getMaxMagnification((LTICImageH) ImageHandle);
	//*(DWORD *)pIsLocked = ltic_isMrSIDLocked((LTICImageH) ImageHandle);
	*(double *)pULX = ltic_getGeoXOrigin((LTICImageH) ImageHandle);
    *(double *)pULY = ltic_getGeoYOrigin((LTICImageH) ImageHandle);
	*(double *)pXres = ltic_getGeoXResolution((LTICImageH) ImageHandle);
	*(double *)pYres = ltic_getGeoYResolution((LTICImageH) ImageHandle);
	*(double *)pXRot = ltic_getGeoXRotation((LTICImageH) ImageHandle);
	*(double *)pYRot = ltic_getGeoYRotation((LTICImageH) ImageHandle);
	*(DWORD *)pNumMetaRecords = ltic_getNumMetadataRecords((LTICImageH) ImageHandle);
	return TRUE;
}

DWORD GMMrSidGetMetadataRecord(DWORD ImageHandle,
                               DWORD recordNum,
                               DWORD pptag,
                               DWORD pdatatype,
                               DWORD pnumDims,
                               DWORD ppdims,
                               DWORD ppdata)
{
	DWORD	rtn;
	LTIMetadataDataType datatype;
	DWORD	NumDims;
	LPVOID	pdata;
	LPDWORD	pDims;
	LPSTR	ptag;

	if (!ImageHandle)
		return FALSE;
	rtn = ltic_getMetadataRecord((LTICImageH)ImageHandle,
                                 recordNum,
                                 &ptag,
                                 &datatype,
                                 &NumDims,
                                 &pDims,
                                 &pdata);
//	MessageBox (0,ptag,NULL,MB_OK);
//	*(LPSTR *)pptag = ptag;
	strcpy ((LPSTR)pptag,ptag);
	*(short *)pdatatype = datatype;
/*	*(short *)pnumDims = (short)NumDims;
	*(LPDWORD *)ppdims = pDims;
	*(LPVOID*)ppdata = pdata;*/
	return rtn;
}

#else
DWORD GMMrSidOpen (LPSTR MrSidFile)
{
	return 0;
}
DWORD GMMrSidClose (DWORD ImageHandle)
{
	return 0;
}
DWORD GMMrSidVersion (LPSTR Version)
{
	*Version = 0;
	return FALSE;
}
DWORD GMMrSidGetImage (DWORD ImageHandle,
					   DWORD pxUpperLeft,
					   DWORD pyUpperLeft,
					   DWORD pWidth,
					   DWORD pHeight,
					   DWORD pMagnification)
{
	return 0;
}
DWORD GMMrSidGetImageInfo (DWORD ImageHandle,
						   DWORD pWidth,
						   DWORD pHeight,
						   DWORD pColorSpace,
						   DWORD pNumBands,
						   DWORD pDataType,
						   DWORD pMinMag,
						   DWORD pMaxMag,
						   DWORD pIsLocked,
						   DWORD pULX,
						   DWORD pULY,
						   DWORD pXres,
						   DWORD pYres,
						   DWORD pXRot,
						   DWORD pYRot,
						   DWORD pNumMetaRecords
						   )
{
	return 0;
}
DWORD GMMrSidGetMetadataRecord(DWORD ImageHandle,
                               DWORD recordNum,
                               DWORD pptag,
                               DWORD pdatatype,
                               DWORD pnumDims,
                               DWORD ppdims,
                               DWORD ppdata)
{
	return 0;
}
#endif
   