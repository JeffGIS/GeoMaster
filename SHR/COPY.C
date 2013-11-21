//*************************************************************
//  File name: COPY.C
//
//  Description:
//      This file contains the CopyFile function, and its
//      dependent functions.
//
//  Functions:
//
//             CopyFile (LPSTR, LPSTR, BOOL);
//             GetFileAttrib (LPSTR, unsigned int*);
//             SetFileAttrib (LPSTR, unsigned int);
//             GetFileDateTime (int, unsigned int*, unsigned int *);
//             SetFileDateTime (int, unsigned int, unsigned int);
//
//  Comments:
//      To use the CopyFile function, you need to declare the
//      function proto-type in your application, and compile/link
//      in this C file.  Here is the proto-type for the CopyFile
//      function:
//
//      int _far _pascal CopyFile (LPSTR, LPSTR, BOOL);
//
//  Build Environment:
//
//      Windows SDK v3.1, C7.0
//
//  History:    Date       Author     Comment
//              12/19/91   Eric Flo   Created
//               8/18/92   Eric Flo   Updated to work in a DLL
//
//*************************************************************

#include <windows.h>

#define CHUNKSIZE  32000   // This must not exceed 32K

/* Error messages returned by CopyFile */

#define SUCCESS                       0
#define ERROR_CANNOT_OPEN_SOURCE      1
#define ERROR_CANNOT_OPEN_DEST        2
#define ERROR_CANNOT_ALLOCATE_MEMORY  3
#define ERROR_CANNOT_LOCK_MEMORY      4
#define ERROR_READING_FILE            5
#define ERROR_GETTING_FILE_DATE_TIME  6
#define ERROR_SETTING_FILE_DATE_TIME  7
#define ERROR_GETTING_FILE_ATTRIB     8
#define ERROR_SETTING_FILE_ATTRIB     9

/* Function proto-types for the helper functions */
unsigned int GetFileAttrib (LPSTR, PWORD);
unsigned int SetFileAttrib (LPSTR, unsigned int);
unsigned int GetFileDateTime (int, PWORD, PWORD);
unsigned int SetFileDateTime (int, unsigned int, unsigned int);

/* Function proto-type for Interrupt 21 */
void far pascal DOS3CALL (void);

//*************************************************************
//
//  CopyFile()
//
//  Purpose:
//              Copies the file specified by szSource into
//              szDest, creating szDest if necessary.
//
//
//  Parameters:
//      LPSTR szSource    - Null terminated string containing the
//                          source file name.
//      LPSTR szDest      - Null terminated string containing the
//                          destination file name.
//      BOOL  bAttrib     - Sets file attributes, date and time to
//                          be the same as the source file if this
//                          parameter is TRUE.  Otherwise this is
//                          ignored.
//      
//
//  Return: (int)
//               0 if successful
//
//           or non-zero if an error occurs:
//
//               1 ERROR_CANNOT_OPEN_SOURCE
//               2 ERROR_CANNOT_OPEN_DEST
//               3 ERROR_CANNOT_ALLOCATE_MEMORY
//               4 ERROR_CANNOT_LOCK_MEMORY
//               5 ERROR_READING_FILE
//               6 ERROR_GETTING_FILE_DATE_TIME
//               7 ERROR_SETTING_FILE_DATE_TIME
//               8 ERROR_GETTING_FILE_ATTRIB
//               9 ERROR_SETTING_FILE_ATTRIB
//
//
//  Comments:
//              This function is completely self-contained within
//              this one C file.  It is not dependent upon any global
//              variables.
//
//
//              A sample call to this function is:
//
//    wResult = CopyFile ((LPSTR)"test.txt", (LPSTR)"test.bak", TRUE);
//
//
//  History:    Date       Author     Comment
//              12/19/91   Eric Flo   Created
//
//*************************************************************

int _far _pascal CopyFile (LPSTR szSource, LPSTR szDest, BOOL bAttrib)
  {
  int      hFileSource, hFileDest;
  OFSTRUCT OpenBuff;
  WORD     wBytesRead;
  HANDLE   hMem;
  LPSTR    gPtr;
  static   unsigned int uTime, uDate, uAttrib, uResult;

  /* open source file for reading */
  hFileSource = OpenFile (szSource, &OpenBuff, OF_READ | OF_SHARE_DENY_WRITE);
  if (hFileSource == -1)
    return (ERROR_CANNOT_OPEN_SOURCE);

  /* open destination file for writing */
  hFileDest = OpenFile (szDest, &OpenBuff, OF_CREATE | OF_WRITE | OF_SHARE_EXCLUSIVE);
  if (hFileDest == -1)
    {
    _lclose (hFileSource);
    return (ERROR_CANNOT_OPEN_DEST);
    }

  /* create a buffer to copy through */
  hMem = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, CHUNKSIZE+10);

  if (hMem == 0)
    {
    _lclose (hFileSource);
    _lclose (hFileDest);
    return (ERROR_CANNOT_ALLOCATE_MEMORY);
    }

  gPtr = GlobalLock (hMem);

  if (gPtr == 0)
    {
    GlobalFree (hMem);
    _lclose (hFileSource);
    _lclose (hFileDest);
    return (ERROR_CANNOT_LOCK_MEMORY);
    }

  /* Since _lread and _lwrite can not handle a buffer greater
     than 32K, we need to read the file in chunks.  Hence,
     loop until all the chunks have been copied.  On the last
     loop, wBytesRead will be less than CHUNKSIZE, thus the
     loop will fall out. */

  do
    {
    wBytesRead = _lread(hFileSource, gPtr, (WORD) CHUNKSIZE);
    if (wBytesRead == -1)
        {
        GlobalUnlock (hMem);
        GlobalFree (hMem);
        _lclose (hFileSource);
        _lclose (hFileDest);
        return (ERROR_READING_FILE);
        }
    _lwrite(hFileDest,  gPtr, wBytesRead);
    }
  while (wBytesRead == CHUNKSIZE);

  /* Set file attributes, date, and time if wanted */
  if (bAttrib)
    {
    /* Query and set date and time */  
    
    uResult = GetFileDateTime (hFileSource, &uTime, &uDate);
    if (uResult != 0)
        {
        GlobalUnlock (hMem);
        GlobalFree (hMem);
        _lclose (hFileSource);
        _lclose (hFileDest);
        return (ERROR_GETTING_FILE_DATE_TIME);
        }
    uResult = SetFileDateTime (hFileDest, uTime, uDate);
    if (uResult != 0)
        {
        GlobalUnlock (hMem);
        GlobalFree (hMem);
        _lclose (hFileSource);
        _lclose (hFileDest);
        return (ERROR_SETTING_FILE_DATE_TIME);
        }
    }

  /* clean up */
  GlobalUnlock (hMem);
  GlobalFree (hMem);
  _lclose (hFileSource);
  _lclose (hFileDest);

/*  if (bAttrib)
    {
    * Query and set file attributes *
    uResult = GetFileAttrib (szSource, &uAttrib);
    if (uResult != 0)
        return (ERROR_GETTING_FILE_ATTRIB);

    uResult = SetFileAttrib (szDest, uAttrib);
    if (uResult != 0)
        return (ERROR_SETTING_FILE_ATTRIB);
    }
*/
  return (SUCCESS);
  }

//*************************************************************
//
//  GetFileAttrib()
//
//  Purpose:
//              Gets the specified file attributes.
//
//
//  Parameters:
//      LPSTR szFile      - Null terminated string containing the
//                          file name.
//      unsigned* uAttrib - Pointer to variable to contain the attributes
//      
//
//  Return: (unsigned int)
//
//              0            if successful
//              Error number if unsuccessful
//
//  Comments:
//              This function calls int 21 function 4300h to
//              get the file attributes.
//
//  History:    Date       Author     Comment
//              12/19/91   Eric Flo   Created
//
//*************************************************************

unsigned int GetFileAttrib (LPSTR szFile, PWORD uAttrib)
  {
  unsigned int uResult;

  _asm
     {
     lds  dx,szFile           ; ds:dx contains the filename

     mov  ax,4300h            ; GetFileAttributes
     call DOS3CALL

     jc   error
     mov  bx, uAttrib
     mov  WORD PTR [bx], cx   ; Success, copy the attributes to uAttrib
     mov  uResult, 0h
     jmp  done

error:
     mov  uResult, ax         ; Error

done:
     }

  return (uResult);
  }


//*************************************************************
//
//  SetFileAtrib()
//
//  Purpose:
//              Set file attributes
//
//
//  Parameters:
//      LPSTR szFile      - Null terminated string containing the
//                          file name.
//      unsigned uAttrib  - Unsigned integer containing the file
//                          attributes.
//
//  Return: (unsigned int)
//
//              0            if successful
//              Error number if unsuccessful
//  Comments:
//              This function calls int 21 function 4301h to
//              set the file attributes.
//
//  History:    Date       Author     Comment
//              12/19/91   Eric Flo   Created
//
//*************************************************************


unsigned int SetFileAttrib (LPSTR szFile, unsigned int uAttrib)
  {
  unsigned int uResult;

  _asm
     {
     lds  dx,szFile           ; ds:dx contains the filename
     mov  cx,uAttrib          ; attributes in cx

     mov  ax,4301h            ; SetFileAttributes
     call DOS3CALL

     jc  error
     mov uResult,0000h        ; Success
     jmp done

error:
     mov  uResult, ax         ; Error
done:
     }
  return (uResult);
  }

//*************************************************************
//
//  GetFileDateTime()
//
//  Purpose:
//              Gets the date and time of the file when it
//              was last written to.
//
//
//  Parameters:
//      int   hFile       - File handle.
//      unsigned * uTime  - Pointer to variable to contain the time.
//      unsigned * uDate  - Pointer to variable to contain the date.
//      
//
//  Return: (unsigned int)
//
//              0            if successful
//              Error number if unsuccessful
//
//
//  Comments:
//              This function calls int 21 function 5700h to get
//              the file date and time.
//
//
//  History:    Date       Author     Comment
//              12/23/91   Eric Flo   Created
//
//*************************************************************

unsigned int GetFileDateTime (int hFile, PWORD uTime,PWORD uDate)
  {
  unsigned int uResult;

  _asm
     {
     mov  bx,hFile            ; bx contains the file handle

     mov  ax,5700h            ; GetFileDateTime
     call DOS3CALL

     jc   error
     mov  bx,uTime
     mov  WORD PTR [bx],cx    ; store time in uTime
     mov  bx,uDate
     mov  WORD PTR [bx],dx    ; store date in uDate
     mov  uResult,0000h
     jmp  done

error:                        ; Error
     mov  uResult,ax

done:
     }
  return (uResult);
  }

//*************************************************************
//
//  SetFileDateTime()
//
//  Purpose:
//              Sets the files date and time with the values
//              specified by uTime and uDate.
//
//
//  Parameters:
//      int        hFile  - File handle.
//      unsigned   uTime  - Variable containing the time.
//      unsigned   uDate  - Variable containing the date.
//
//  Return: (unsigned int)
//
//              0            if successful
//              Error number if unsuccessful
//
//  Comments:
//              This function calls int 21 function 5701h to
//              set the file date and time.
//
//
//  History:    Date       Author     Comment
//              12/23/91   Eric Flo   Created
//
//*************************************************************

unsigned int SetFileDateTime (int hFile, unsigned int uTime,
                              unsigned int uDate)
  {
  unsigned int uResult;

  _asm
     {
     mov  bx,hFile            ; bx contains the file handle
     mov  cx,uTime            ; cx contains the time
     mov  dx,uDate            ; dx contains the date

     mov  ax,5701h            ; SetFileDateTime
     call DOS3CALL

     jc   error
     mov  uResult,0000h       ; Success
     jmp  done

error:
     mov  uResult,ax          ; Error

done:
     }
  return (uResult);
  }

/* End of File */  

	