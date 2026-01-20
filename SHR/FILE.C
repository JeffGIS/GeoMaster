//*******************************************************************
//
//  file.c
//
//  Source file for Device-Independent Bitmap (DIB) API.  Provides
//  the following functions:
//
//  SaveDIB()           - Saves the specified dib in a file
//  LoadDIB()           - Loads a DIB from a file
//  DestroyDIB()        - Deletes DIB when finished using it
//
// Development Team: Mark Bader
//                   Patrick Schreiber
//                   Garrett McAuliffe
//                   Eric Flo
//                   Tony Claflin
//
// Written by Microsoft Product Support Services, Developer Support.
// COPYRIGHT:
//
//   (C) Copyright Microsoft Corp. 1993.  All rights reserved.
//
//   You have a royalty-free right to use, modify, reproduce and
//   distribute the Sample Files (and/or any modified version) in
//   any way you find useful, provided that you agree that
//   Microsoft has no warranty obligations or liability for any
//   Sample Application Files which are modified.
//
//*******************************************************************

#include "shr.h"
#include "dibutil.h"
#include "dibapi.h"
enum {
      ERR_MIN = 0,                     // All error #s >= this value
      ERR_NOT_DIB = 0,                 // Tried to load a file, NOT a DIB!
      ERR_MEMORY,                      // Not enough memory!
      ERR_READ,                        // Error reading file!
      ERR_LOCK,                        // Error on a GlobalLock()!
      ERR_OPEN,                        // Error opening a file!
      ERR_CREATEPAL,                   // Error creating palette.
      ERR_GETDC,                       // Couldn't get a DC.
      ERR_CREATEDDB,                   // Error create a DDB.
      ERR_STRETCHBLT,                  // StretchBlt() returned failure.
      ERR_STRETCHDIBITS,               // StretchDIBits() returned failure.
      ERR_SETDIBITSTODEVICE,           // SetDIBitsToDevice() failed.
      ERR_STARTDOC,                    // Error calling StartDoc().
      ERR_NOGDIMODULE,                 // Couldn't find GDI module in memory.
      ERR_SETABORTPROC,                // Error calling SetAbortProc().
      ERR_STARTPAGE,                   // Error calling StartPage().
      ERR_NEWFRAME,                    // Error calling NEWFRAME escape.
      ERR_ENDPAGE,                     // Error calling EndPage().
      ERR_ENDDOC,                      // Error calling EndDoc().
      ERR_SETDIBITS,                   // Error calling SetDIBits().
      ERR_FILENOTFOUND,                // Error opening file in GetDib()
      ERR_INVALIDHANDLE,               // Invalid Handle
      ERR_DIBFUNCTION,                 // Error on call to DIB function
      ERR_MAX                          // All error #s < this value
     };



/*
 * Dib Header Marker - used in writing DIBs to files
 */
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')


/*********************************************************************
 *
 * Local Function Prototypes
 *
 *********************************************************************/


HANDLE ReadDIBFile(HFILE);
BOOL MyRead(int, LPSTR, DWORD);
BOOL SaveDIBFile(void);
BOOL WriteDIB(LPSTR, HANDLE);
DWORD PASCAL MyWrite(int, VOID FAR *, DWORD);

/*************************************************************************
 *
 * LoadDIB()
 *
 * Loads the specified DIB from a file, allocates memory for it,
 * and reads the disk file into the memory.
 *
 *
 * Parameters:
 *
 * LPSTR lpFileName - specifies the file to load a DIB from
 *
 * Returns: A handle to a DIB, or NULL if unsuccessful.
 *
 * NOTE: The DIB API were not written to handle OS/2 DIBs; This
 * function will reject any file that is not a Windows DIB.
 *
 * History:   Date      Author       Reason
 *            9/15/91   Mark Bader   Based on DIBVIEW
 *
 *************************************************************************/


HDIB FAR LoadDIB(LPSTR lpFileName)
{
   HDIB hDIB=0;
   int hFile;
   OFSTRUCTGM ofs;
   short	lName=_fstrlen (lpFileName);
   LPBITMAPINFOHEADER  pDibInfo;
   LPLONG	pColors;
    

   /*
    * Set the cursor to a hourglass, in case the loading operation
    * takes more than a sec, the user will know what's going on.
    */

	if ((hDIB = GetBMPFromCache (lpFileName)))
   		goto Exit;
   
	//SetCursor(LoadCursor(NULL, IDC_WAIT));
	if (lName > 4 && !_fstricmp (&lpFileName[lName-4],".tif"))
	{
//		hDIB = BMPFromTIF (lpFileName,FALSE); 
		hDIB = BMPFromEXT (lpFileName); 
	    AddBMPToCache (lpFileName,hDIB);
	    goto Exit;
	}
	if (lName > 4 && !_fstricmp (&lpFileName[lName-4],".pcx"))
	{
//		hDIB = BMPFromPCX (lpFileName); 
		hDIB = BMPFromEXT (lpFileName); 
	    AddBMPToCache (lpFileName,hDIB);
	    goto Exit;
	}
	if (lName > 4 && !_fstricmp(&lpFileName[lName - 4], ".jpg"))
	{
		hDIB = BMPFromEXT(lpFileName);
		AddBMPToCache(lpFileName, hDIB);
		goto Exit;
	}
	if (lName > 4 && !_fstricmp(&lpFileName[lName - 4], ".bmp"))
	{
		hDIB = BMPFromEXT(lpFileName);
		AddBMPToCache(lpFileName, hDIB);
		goto Exit;
	}
	if (lName > 4 && !_fstricmp(&lpFileName[lName - 4], ".png"))
	{
		hDIB = BMPFromEXT(lpFileName);
		AddBMPToCache(lpFileName, hDIB);
		goto Exit;
	}
	if ((hFile = GSSiOpenFile(lpFileName, &ofs, OF_READ)) != -1)
   {
      hDIB = ReadDIBFile(hFile);
      GSSiClose2 (&hFile);     
      AddBMPToCache (lpFileName,hDIB);
      goto Exit;
   }
   DIBError(ERR_FILENOTFOUND);
Exit: 
	
   //SetCursor(LoadCursor(NULL, IDC_ARROW)); 
   if (hDIB)
   {
	   pDibInfo = (LPBITMAPINFOHEADER) GlobalLock (hDIB); 
	   if (pDibInfo->biClrUsed == 2)
	   {   
	   	   char	str[32];
	   	   LPSTR	pSpace;
	   	   
		   pColors = (LPLONG)((LPSTR) pDibInfo + sizeof(BITMAPINFOHEADER));
		   if (GetGlobalCVal ("[%MONOCOLORS]",str,NULL))
		   {
		   		*pColors++ = atol (str);
		   		if ((pSpace = _fstrchr (str,' ')))
		   			*pColors = atol (pSpace);	
		   }
	   }	 
	   GlobalUnlock (hDIB); 
   } 
   return hDIB;
   
}


HDIB32 LoadDIB32(LPSTR lpFileNameIN,BOOL AdjustColorsToVP,int displayImagePlaneInRed)
{
   HDIB32 hDIB=0;
   LPBITMAPINFOHEADER  pDibInfo;
   LONG	pColors[2];   
   char	lpFileName[MAX_PATH];  
   short	l;
    
//extern BOOL	AllowCache;
   /*
    * Set the cursor to a hourglass, in case the loading operation
    * takes more than a sec, the user will know what's going on.
    */

	if (!lpFileNameIN)
		return 0;
    _fstrcpy (lpFileName,lpFileNameIN);
    ExpandText (lpFileName);
    Truncate (lpFileName); 
	if (!*lpFileName)
		return 0;
/*    l = _fstrlen (lpFileName);
    if (l>4 && !_fstricmp (&lpFileName[l-3],"sbm"))
    	_fstrcpy (&lpFileName[l-3],"bmp");*/
	ConvertFileNameToCacheFileName (lpFileName);
	if ((hDIB = GetBMPFromCache32(lpFileName)))
	{
		if (hDIB && AdjustColorsToVP == 24)
			hDIB = GSSiFreeImage_ConvertTo24Bits(hDIB);

		return hDIB;
	}
   
	//SetCursor(LoadCursor(NULL, IDC_WAIT));
	hDIB = BMPHandleFromEXT(lpFileName);
	if (!hDIB && ExistFile(lpFileName))
	{
		AddBMPToCache32(0, 0, 0);
		hDIB = BMPHandleFromEXT(lpFileName);
	}

	if (strnicmp(lpFileName, "http:", 5) && strnicmp(lpFileName, "https:", 6))
	{
		if (AdjustColorsToVP)
			hDIB = AdjustDIB32Colors (hDIB); 
		AddBMPToCache32 (lpFileName,&hDIB, displayImagePlaneInRed);
	}
   //SetCursor(LoadCursor(NULL, IDC_ARROW)); 
	if (hDIB && AdjustColorsToVP==24)
	{
		hDIB = GSSiFreeImage_ConvertTo24Bits(hDIB);
	}
	else if (hDIB && AdjustColorsToVP)
	{
   	   char	str[32];
   	   LPSTR	pSpace;
	   	   
	   if (GetGlobalCVal ("[%MONOCOLORS]",str,NULL))
	   {
	   		pColors[0] = atol (str);
	   		if ((pSpace = _fstrchr (str,' ')))
	   			pColors[1] = atol (pSpace);	
	   		SetDIBMonoColors (hDIB,pColors);
	   }
   } 
   return hDIB;
   
}


/*************************************************************************
 *
 * SaveDIB()
 *
 * Saves the specified DIB into the specified file name on disk.  No
 * error checking is done, so if the file already exists, it will be
 * written over.
 *
 * Parameters:
 *
 * HDIB hDib - Handle to the dib to save
 *
 * LPSTR lpFileName - pointer to full pathname to save DIB under
 *
 * Return value: 0 if successful, or one of:
 *        ERR_INVALIDHANDLE
 *        ERR_OPEN
 *        ERR_LOCK
 *
 * History:
 *
 * NOTE: The DIB API were not written to handle OS/2 DIBs, so this
 * function will not save a file if it is not a Windows DIB.
 *
 * History:   Date      Author       Reason
 *            9/15/91   Mark Bader   Taken from DIBVIEW (which was taken
 *                                      from SHOWDIB)
 *            1/30/92   Mark Bader   Fixed problem of writing too many 
 *                                      bytes to the file
 *            6/24/92   Mark Bader   Added check for OS/2 DIB
 *
 *************************************************************************/


WORD FAR SaveDIB(HDIB hDib, LPSTR lpFileName)
{
   BITMAPFILEHEADER bmfHdr; // Header for Bitmap file
   LPBITMAPINFOHEADER lpBI;   // Pointer to DIB info structure
   int fh;     // file handle for opened file
   OFSTRUCTGM of;     // OpenFile structure
   DWORD dwDIBSize;
   DWORD dwError;   // Error return from MyWrite

   if (!*lpFileName)
	   return ERR_OPEN;
   if (!hDib)
      return ERR_INVALIDHANDLE;
   fh = GSSiOpenFile(lpFileName, &of, OF_CREATE | OF_READWRITE);
   if (fh == -1)
      return ERR_OPEN;

   /*
    * Get a pointer to the DIB memory, the first of which contains
    * a BITMAPINFO structure
    */
   lpBI = (LPBITMAPINFOHEADER)GlobalLock(hDib);
   if (!lpBI)
      return ERR_LOCK;

   // Check to see if we're dealing with an OS/2 DIB.  If so, don't
   // save it because our functions aren't written to deal with these
   // DIBs.

   if (lpBI->biSize != sizeof(BITMAPINFOHEADER))
   {
     GlobalUnlock(hDib);
     return ERR_NOT_DIB;
   }

   /*
    * Fill in the fields of the file header
    */

   /* Fill in file type (first 2 bytes must be "BM" for a bitmap) */
   bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM"

   // Calculating the size of the DIB is a bit tricky (if we want to
   // do it right).  The easiest way to do this is to call GlobalSize()
   // on our global handle, but since the size of our global memory may have
   // been padded a few bytes, we may end up writing out a few too
   // many bytes to the file (which may cause problems with some apps,
   // like HC 3.0).
   //
   // So, instead let's calculate the size manually.
   //
   // To do this, find size of header plus size of color table.  Since the
   // first DWORD in both BITMAPINFOHEADER and BITMAPCOREHEADER conains
   // the size of the structure, let's use this.

   dwDIBSize = *(LPDWORD)lpBI + PaletteSize((LPSTR)lpBI);  // Partial Calculation

   // Now calculate the size of the image

   if ((lpBI->biCompression == BI_RLE8) || (lpBI->biCompression == BI_RLE4)) {

      // It's an RLE bitmap, we can't calculate size, so trust the
      // biSizeImage field

      dwDIBSize += lpBI->biSizeImage;
      }
   else {
      DWORD dwBmBitsSize;  // Size of Bitmap Bits only

      // It's not RLE, so size is Width (DWORD aligned) * Height

      dwBmBitsSize = WIDTHBYTES((lpBI->biWidth)*((DWORD)lpBI->biBitCount)) * lpBI->biHeight;

      dwDIBSize += dwBmBitsSize;
      
      // Now, since we have calculated the correct size, why don't we
      // fill in the biSizeImage field (this will fix any .BMP files which 
      // have this field incorrect).

      lpBI->biSizeImage = dwBmBitsSize;
      }


   // Calculate the file size by adding the DIB size to sizeof(BITMAPFILEHEADER)
                   
   bmfHdr.bfSize = dwDIBSize + sizeof(BITMAPFILEHEADER);
   bmfHdr.bfReserved1 = 0;
   bmfHdr.bfReserved2 = 0;

   /*
    * Now, calculate the offset the actual bitmap bits will be in
    * the file -- It's the Bitmap file header plus the DIB header,
    * plus the size of the color table.
    */
   bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + lpBI->biSize +
                      PaletteSize((LPSTR)lpBI);

   /* Write the file header */
   BigWrite(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER),-1);

   /*
    * Write the DIB header and the bits -- use local version of
    * MyWrite, so we can write more than 32767 bytes of data
    */
//   dwError = MyWrite(fh, (LPSTR)lpBI, dwDIBSize);
   dwError = BigWrite(fh, (LPSTR)lpBI, dwDIBSize,-1);
   GlobalUnlock(hDib);
   GSSiClose2 (&fh);

   if (dwError == 0)
     return ERR_OPEN; // oops, something happened in the write
   else
     return 0; // Success code
}


/*************************************************************************
 *
 * DestroyDIB ()
 *
 * Purpose:  Frees memory associated with a DIB
 *
 * Returns:  Nothing
 *
 * History:   Date      Author       Reason
 *            9/15/91   Mark Bader   Created
 *
 *************************************************************************/


WORD FAR DestroyDIB(HDIB hDib)
{  
   if (hDIBInCache (hDib))
		return 0;//allows caching
   GSSiGlobFree (&hDib);
   return 0;
}


//************************************************************************
//
// Auxiliary Functions which the above procedures use
//
//************************************************************************


/*************************************************************************
 *
 * Function:  ReadDIBFile (int)
 *
 *  Purpose:  Reads in the specified DIB file into a global chunk of
 *            memory.
 *
 *  Returns:  A handle to a dib (hDIB) if successful.
 *            NULL if an error occurs.
 *
 * Comments:  BITMAPFILEHEADER is stripped off of the DIB.  Everything
 *            from the end of the BITMAPFILEHEADER structure on is
 *            returned in the global memory handle.
 *
 *
 * NOTE: The DIB API were not written to handle OS/2 DIBs, so this
 * function will reject any file that is not a Windows DIB.
 *
 * History:   Date      Author       Reason
 *            9/15/91   Mark Bader   Based on DIBVIEW
 *            6/25/92   Mark Bader   Added check for OS/2 DIB
 *            7/21/92   Mark Bader   Added code to deal with bfOffBits
 *                                     field in BITMAPFILEHEADER      
 *            9/11/92   Mark Bader   Fixed Realloc Code to free original mem
 *
 *************************************************************************/

HANDLE ReadDIBFile(HFILE hFile)
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

   dwBitsSize = GSSifilelength(hFile);

   // Allocate memory for header & color table.	We'll enlarge this
   // memory as needed.

   hDIB = GSSiGlobAlloc(GAIDNO 1739,GMEM_MOVEABLE,
       (DWORD)(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)));
   
   if (!hDIB) return NULL;

   lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);
   if (!lpbi) 
   {
     GSSiGlobUlFree (&hDIB);
     return NULL;
   }

   // read the BITMAPFILEHEADER from our file

   if (sizeof (BITMAPFILEHEADER) != BigRead (hFile, (LPSTR)&bmfHeader, sizeof (BITMAPFILEHEADER)))
     goto ErrExit;

   if (bmfHeader.bfType != 0x4d42)	/* 'BM' */
     goto ErrExit;

   // read the BITMAPINFOHEADER

   if (sizeof(BITMAPINFOHEADER) != BigRead (hFile, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER)))
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
   hDIBtmp = GSSiGlobalReAlloc (0,hDIB, lpbi->biSize +
                        nNumColors * sizeof(RGBQUAD) +
                        lpbi->biSizeImage, GMEM_MOVEABLE);

   if (!hDIBtmp) // can't resize buffer for loading
     goto ErrExitNoUnlock; //MPB
   else
     hDIB = hDIBtmp;

   lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

   // read the color table 
   if (nNumColors)
   		BigRead (hFile, (LPSTR)(lpbi) + lpbi->biSize, nNumColors * sizeof(RGBQUAD));

   // offset to the bits from start of DIB header
   offBits = lpbi->biSize + nNumColors * sizeof(RGBQUAD);

   // If the bfOffBits field is non-zero, then the bits might *not* be
   // directly following the color table in the file.  Use the value in
   // bfOffBits to seek the bits.

   if (bmfHeader.bfOffBits != 0L)
      GSSillseek(hFile, bmfHeader.bfOffBits, SEEK_SET);
   
   if (MyRead(hFile, (LPSTR)lpbi + offBits, lpbi->biSizeImage))
     goto OKExit;


ErrExit:
    GlobalUnlock(hDIB);    
ErrExitNoUnlock:    
    GSSiGlobUlFree (&hDIB);
    return NULL;

OKExit:
    GlobalUnlock(hDIB);
    return hDIB;
}

/*************************************************************************

  Function:  MyRead (int, LPSTR, DWORD)

   Purpose:  Routine to read files greater than 64K in size.

   Returns:  TRUE if successful.
             FALSE if an error occurs.

  
  History:   Date      Author       Reason
             9/15/91   Mark Bader   Based on DIBVIEW
 
*************************************************************************/


BOOL MyRead(int hFile, LPSTR lpBuffer, DWORD dwSize)
{
   char *lpInBuf = (char *)lpBuffer;
   int nBytes;

   /*
    * Read in the data in 32767 byte chunks (or a smaller amount if it's
    * the last chunk of data read)
    */

   while (dwSize)
   {
      nBytes = (int)(dwSize > (DWORD)32767 ? 32767 : LOWORD (dwSize));
      if (BigRead(hFile, (LPSTR)lpInBuf, nBytes) != (WORD)nBytes)
         return FALSE;
      dwSize -= nBytes;
      lpInBuf += nBytes;
   }
   return TRUE;
}


/****************************************************************************

 FUNCTION   : MyWrite(int fh, VOID FAR *pv, DWORD ul)

 PURPOSE    : Writes data in steps of 32k till all the data is written.
              Normal BigWrite uses a WORD as 3rd parameter, so it is
              limited to 32767 bytes, but this procedure is not.

 RETURNS    : 0 - If write did not proceed correctly.
              number of bytes written otherwise.
 
  History:   Date      Author       Reason
             9/15/91   Mark Bader   Based on DIBVIEW

 ****************************************************************************/


DWORD PASCAL MyWrite(int iFileHandle, VOID FAR *lpBuffer, DWORD dwBytes)
{
   DWORD dwBytesTmp = dwBytes;       // Save # of bytes for return value
   BYTE *hpBuffer = lpBuffer;   // make a huge pointer to the data

   /*
    * Write out the data in 32767 byte chunks.
    */

   while (dwBytes > 32767)
   {
      if (BigWrite(iFileHandle, (HPSTR)hpBuffer, (WORD)32767,-1) != 32767)
         return 0;
      dwBytes -= 32767;
      hpBuffer += 32767;
   }

   /* Write out the last chunk (which is < 32767 bytes) */
   if (BigWrite(iFileHandle, (HPSTR)hpBuffer, (WORD)dwBytes,-1) != (WORD)dwBytes)
      return 0;
   return dwBytesTmp;
}

static char *szErrors[] =
{
   "Not a Windows DIB file!",
   "Couldn't allocate memory!",
   "Error reading file!",
   "Error locking memory!",
   "Error opening file!",
   "Error creating palette!",
   "Error getting a DC!",
   "Error creating Device Dependent Bitmap",
   "StretchBlt() failed!",
   "StretchDIBits() failed!",
   "SetDIBitsToDevice() failed!",
   "Printer: StartDoc failed!",
   "Printing: GetModuleHandle() couldn't find GDI!",
   "Printer: SetAbortProc failed!",
   "Printer: StartPage failed!",
   "Printer: NEWFRAME failed!",
   "Printer: EndPage failed!",
   "Printer: EndDoc failed!",
   "SetDIBits() failed!",
   "File Not Found!",
   "Invalid Handle",
   "General Error on call to DIB function"
};


void FAR DIBError(int ErrNo)
{
   if ((ErrNo < ERR_MIN) || (ErrNo >= ERR_MAX))
      GSSiMsgBox(NULL, "Undefined Error!", NULL, MB_OK | MB_ICONHAND,0);
   else
      GSSiMsgBox(NULL, szErrors[ErrNo], NULL, MB_OK | MB_ICONHAND,0);
}
