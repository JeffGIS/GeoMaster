/*
 *  dibapi.h
 *
 *  Copyright (c) 1991 Microsoft Corporation. All rights reserved
 *
 *  Header file for Device-Independent Bitmap (DIB) API.  Provides
 *  function prototypes and constants for the following functions:
 *
 *  BitmapToDIB()        - Creates a DIB from a bitmap
 *  ChangeBitmapFormat() - Changes a bitmap to a specified DIB format
 *  ChangeDIBFormat()    - Changes a DIB's BPP and/or compression format
 *  CopyScreenToBitmap() - Copies entire screen to a standard Bitmap
 *  CopyScreenToDIB()    - Copies entire screen to a DIB
 *  CopyWindowToBitmap() - Copies a window to a standard Bitmap
 *  CopyWindowToDIB()    - Copies a window to a DIB
 *  CreateDIBPalette()   - Creates a palette from a DIB
 *  CreateDIB()          - Creates a new DIB
 *  DestroyDIB()         - Deletes DIB when finished using it
 *  DIBError()           - Displays message box with error message
 *  DIBHeight()          - Gets the DIB height
 *  DIBNumColors()       - Calculates number of colors in the DIB's color table
 *  DIBToBitmap()        - Creates a bitmap from a DIB
 *  DIBWidth()           - Gets the DIB width
 *  FindDIBBits()        - Sets pointer to the DIB bits
 *  GetSystemPalette()   - Gets the current palette
 *  LoadDIB()            - Loads a DIB from a file
 *  PaintBitmap()        - Displays standard bitmap in the specified DC
 *  PaintDIB()           - Displays DIB in the specified DC
 *  PalEntriesOnDevice() - Gets the number of palette entries
 *  PaletteSize()        - Calculates the buffer size required by a palette
 *  PrintDIB()           - Prints the specified DIB
 *  PrintScreen()        - Prints the entire screen
 *  PrintWindow()        - Prints all or part of a window
 *  SaveDIB()            - Saves the specified dib in a file
 *
 * See the file DIBAPI.TXT for more information about these functions.
 *
 */

#define HDIB HANDLE

/* Print Area selection */
#define PW_WINDOW        1
#define PW_CLIENT        2


/* Print Options selection */
#define PW_BESTFIT       1
#define PW_STRETCHTOPAGE 2
#define PW_SCALE         3

/* DIB Macros*/

// WIDTHBYTES performs DWORD-aligning of DIB scanlines.  The "bits"
// parameter is the bit count for the scanline (biWidth * biBitCount),
// and this macro returns the number of DWORD-aligned bytes needed 
// to hold those bits.

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

/* Error constants */

/* Function prototypes */

HDIB      FAR  BitmapToDIB (HBITMAP hBitmap, HPALETTE hPal);
HDIB FAR BitmapToDIB2(HBITMAP hBitmap, HPALETTE hPal,LPINT piScanLine,LPINT pnScanLine);
HDIB      FAR  ChangeBitmapFormat (HBITMAP  hBitmap,
                                   WORD     wBitCount,
                                   DWORD    dwCompression,
                                   HPALETTE hPal);
HDIB      FAR  ChangeDIBFormat (HDIB hDIB, WORD wBitCount,
                                DWORD dwCompression);
HBITMAP   FAR  CopyScreenToBitmap (LPRECT);
HDIB      FAR  CopyScreenToDIB (LPRECT);
HDIB32 FAR CopyScreenToDIB32(LPRECT lpRect);
HBITMAP   FAR  CopyWindowToBitmap (HWND, WORD);
HDIB      FAR  CopyWindowToDIB (HWND, WORD);
HPALETTE  FAR  CreateDIBPalette (HDIB hDIB);
HDIB      FAR  CreateDIB(DWORD, DWORD, WORD);
WORD      FAR  DestroyDIB (HDIB);
void      FAR  DIBError (int ErrNo);
DWORD     FAR  DIBHeight (LPSTR lpDIB);
WORD      FAR  DIBNumColors (LPSTR lpDIB);
HBITMAP   FAR  DIBToBitmap (HDIB hDIB, HPALETTE hPal);
HBITMAP	  FAR  DIB32ToBitmap(HDIB32 hDIB, HPALETTE hPal);
DWORD     FAR  DIBWidth (LPSTR lpDIB);
LPSTR     FAR  FindDIBBits (LPSTR lpDIB);
HPALETTE  FAR  GetSystemPalette (void);
HDIB      FAR  LoadDIB (LPSTR);
BOOL      FAR  PaintBitmap (HDC, LPRECT, HBITMAP, LPRECT, HPALETTE);
BOOL      FAR  PaintDIB (HDC, LPRECT, HDIB, LPRECT, HPALETTE);
int       FAR  PalEntriesOnDevice (HDC hDC);
WORD      FAR  PaletteSize (LPSTR lpDIB);
WORD      FAR  PrintDIB (HDIB, WORD, WORD, WORD, LPSTR);
WORD      FAR  PrintScreen (LPRECT, WORD, WORD, WORD, LPSTR);
//WORD      FAR  PrintWindow (HWND, WORD, WORD, WORD, WORD, LPSTR);
WORD      FAR  SaveDIB (HDIB, LPSTR);
HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent);
void SetImageFileRotation (int r);
HDIB32 BMPToDIB32(HANDLE hBMP);
