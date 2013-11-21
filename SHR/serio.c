
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples.
*       Copyright (C) 1993-1997 Microsoft Corporation.
*       All rights reserved.
*       This source code is only intended as a supplement to
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the
*       Microsoft samples programs.
\******************************************************************************/

//---------------------------------------------------------------------------
//
//  Module: tty.c
//
//  Purpose:
//     The sample application demonstrates the usage of the COMM
//     API.  It implements the new COMM API of Windows 3.1.
//
//     NOTE:  no escape sequences are translated, only
//            the necessary control codes (LF, CR, BS, etc.)
//
//  Description of functions:
//     Descriptions are contained in the function headers.
//
//---------------------------------------------------------------------------
//
//  Written by Microsoft Product Support Services, Windows Developer Support.
//
//---------------------------------------------------------------------------

#include "tty.h"
#include <float.h>
#include <stdio.h>
//---------------------------------------------------------------------------
//  BOOL NEAR ProcessTTYCharacter( HWND hWnd, BYTE bOut )
//
//  Description:
//     This simply writes a character to the port and echos it
//     to the TTY screen if fLocalEcho is set.  Some minor
//     keyboard mapping could be performed here.
//
//  Parameters:
//     HWND hWnd
//        handle to TTY window
//
//     BYTE bOut
//        byte from keyboard
//
//---------------------------------------------------------------------------

BOOL NEAR ProcessTTYCharacter( HWND hWnd, BYTE bOut )
{
   NPTTYINFO  npTTYInfo ;

   if (NULL == (npTTYInfo = GETNPTTYINFO( hWnd )))
      return ( FALSE ) ;

   if (!CONNECTED( npTTYInfo ))
      return ( FALSE ) ;

   // a robust app would take appropriate steps if WriteCommBlock failed
   WriteCommBlock( hWnd, &bOut, 1 ) ;
   if (LOCALECHO( npTTYInfo ))
      WriteTTYBlock( hWnd, &bOut, 1 ) ;

   return ( TRUE ) ;

} // end of ProcessTTYCharacter()

//---------------------------------------------------------------------------
//  BOOL NEAR OpenConnection( HWND hWnd )
//
//  Description:
//     Opens communication port specified in the TTYINFO struct.
//     It also sets the CommState and notifies the window via
//     the fConnected flag in the TTYINFO struct.
//
//  Parameters:
//     HWND hWnd
//        handle to TTY window
//
//  Win-32 Porting Issues:
//     - OpenComm() is not supported under Win-32.  Use CreateFile()
//       and setup for OVERLAPPED_IO.
//     - Win-32 has specific communication timeout parameters.
//     - Created the secondary thread for event notification.
//
//---------------------------------------------------------------------------

BOOL NEAR OpenConnection( HWND hWnd )
{
   char       szPort[ 15 ], szTemp[ 10 ] ;
   BOOL       fRetVal ;
   HCURSOR    hOldCursor, hWaitCursor ;
   HMENU      hMenu ;
   NPTTYINFO  npTTYInfo ;

   HANDLE        hCommWatchThread ;
   DWORD         dwThreadID ;
   COMMTIMEOUTS  CommTimeOuts ;

   if (NULL == (npTTYInfo = GETNPTTYINFO( hWnd )))
      return ( FALSE ) ;

   // show the hourglass cursor
   hWaitCursor = LoadCursor( NULL, IDC_WAIT ) ;
   hOldCursor = SetCursor( hWaitCursor ) ;

   // load the COM prefix string and append port number

//   LoadString( GETHINST( hWnd ), IDS_COMPREFIX, szTemp, sizeof( szTemp ) ) ;
   wsprintf( szPort, "%s%d", (LPSTR) szTemp, PORT( npTTYInfo ) ) ;
   wsprintf( szPort, "\\.\\%s%d", (LPSTR) szTemp, PORT( npTTYInfo ) ) ;

   // open COMM device

   if ((COMDEV( npTTYInfo ) =
      CreateFile( szPort, GENERIC_READ | GENERIC_WRITE,
                  0,                    // exclusive access
                  NULL,                 // no security attrs
                  OPEN_EXISTING,
                  FILE_ATTRIBUTE_NORMAL |
                  FILE_FLAG_OVERLAPPED, // overlapped I/O
                  NULL )) == (HANDLE) -1 )
      return ( FALSE ) ;
   else
   {
      // get any early notifications

      SetCommMask( COMDEV( npTTYInfo ), EV_RXCHAR ) ;

      // setup device buffers

      SetupComm( COMDEV( npTTYInfo ), 4096, 4096 ) ;

      // purge any information in the buffer

      PurgeComm( COMDEV( npTTYInfo ), PURGE_TXABORT | PURGE_RXABORT |
                                      PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

      // set up for overlapped I/O

      CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF ;
      CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
      CommTimeOuts.ReadTotalTimeoutConstant = 1000 ;
      // CBR_9600 is approximately 1byte/ms. For our purposes, allow
      // double the expected time per character for a fudge factor.
      CommTimeOuts.WriteTotalTimeoutMultiplier = 2*CBR_9600/BAUDRATE( npTTYInfo ) ;
      CommTimeOuts.WriteTotalTimeoutConstant = 0 ;
      SetCommTimeouts( COMDEV( npTTYInfo ), &CommTimeOuts ) ;
   }

   fRetVal = SetupConnection( hWnd ) ;

   if (fRetVal)
   {
      CONNECTED( npTTYInfo ) = TRUE ;

      // Create a secondary thread
      // to watch for an event.

      if (NULL == (hCommWatchThread =
                      CreateThread( (LPSECURITY_ATTRIBUTES) NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE) CommWatchProc,
                                    (LPVOID) npTTYInfo,
                                    0, &dwThreadID )))
      {
         CONNECTED( npTTYInfo ) = FALSE ;
         CloseHandle( COMDEV( npTTYInfo ) ) ;
         fRetVal = FALSE ;
      }
      else
      {
         THREADID( npTTYInfo ) = dwThreadID ;
         HTHREAD( npTTYInfo ) = hCommWatchThread ;

         // assert DTR

         EscapeCommFunction( COMDEV( npTTYInfo ), SETDTR ) ;

/*         SetTTYFocus( hWnd ) ;

         hMenu = GetMenu( hWnd ) ;
         EnableMenuItem( hMenu, IDM_DISCONNECT,
                        MF_ENABLED | MF_BYCOMMAND ) ;
         EnableMenuItem( hMenu, IDM_CONNECT,
                        MF_GRAYED | MF_DISABLED | MF_BYCOMMAND ) ;
*/
      }
   }
   else
   {
      CONNECTED( npTTYInfo ) = FALSE ;
      CloseHandle( COMDEV( npTTYInfo ) ) ;
   }

   // restore cursor

   SetCursor( hOldCursor ) ;

   return ( fRetVal ) ;

} // end of OpenConnection()

//---------------------------------------------------------------------------
//  BOOL NEAR SetupConnection( HWND hWnd )
//
//  Description:
//     This routines sets up the DCB based on settings in the
//     TTY info structure and performs a SetCommState().
//
//  Parameters:
//     HWND hWnd
//        handle to TTY window
//
//  Win-32 Porting Issues:
//     - Win-32 requires a slightly different processing of the DCB.
//       Changes were made for configuration of the hardware handshaking
//       lines.
//
//---------------------------------------------------------------------------

BOOL NEAR SetupConnection( HWND hWnd )
{
   BOOL       fRetVal ;
   BYTE       bSet ;
   DCB        dcb ;
   NPTTYINFO  npTTYInfo ;

   if (NULL == (npTTYInfo = GETNPTTYINFO( hWnd )))
      return ( FALSE ) ;

   dcb.DCBlength = sizeof( DCB ) ;

   GetCommState( COMDEV( npTTYInfo ), &dcb ) ;

   dcb.BaudRate = BAUDRATE( npTTYInfo ) ;
   dcb.ByteSize = BYTESIZE( npTTYInfo ) ;
   dcb.Parity = PARITY( npTTYInfo ) ;
   dcb.StopBits = STOPBITS( npTTYInfo ) ;

   // setup hardware flow control

   bSet = (BYTE) ((FLOWCTRL( npTTYInfo ) & FC_DTRDSR) != 0) ;
   dcb.fOutxDsrFlow = bSet ;
   if (bSet)
      dcb.fDtrControl = DTR_CONTROL_HANDSHAKE ;
   else
      dcb.fDtrControl = DTR_CONTROL_ENABLE ;

   bSet = (BYTE) ((FLOWCTRL( npTTYInfo ) & FC_RTSCTS) != 0) ;
	dcb.fOutxCtsFlow = bSet ;
   if (bSet)
      dcb.fRtsControl = RTS_CONTROL_HANDSHAKE ;
   else
      dcb.fRtsControl = RTS_CONTROL_ENABLE ;

   // setup software flow control

   bSet = (BYTE) ((FLOWCTRL( npTTYInfo ) & FC_XONXOFF) != 0) ;

   dcb.fInX = dcb.fOutX = bSet ;
   dcb.XonChar = ASCII_XON ;
   dcb.XoffChar = ASCII_XOFF ;
   dcb.XonLim = 100 ;
   dcb.XoffLim = 100 ;

   // other various settings

   dcb.fBinary = TRUE ;
   dcb.fParity = TRUE ;

   fRetVal = SetCommState( COMDEV( npTTYInfo ), &dcb ) ;

   return ( fRetVal ) ;

} // end of SetupConnection()

//---------------------------------------------------------------------------
//  BOOL NEAR CloseConnection( HWND hWnd )
//
//  Description:
//     Closes the connection to the port.  Resets the connect flag
//     in the TTYINFO struct.
//
//  Parameters:
//     HWND hWnd
//        handle to TTY window
//
//  Win-32 Porting Issues:
//     - Needed to stop secondary thread.  SetCommMask() will signal the
//       WaitCommEvent() event and the thread will halt when the
//       CONNECTED() flag is clear.
//     - Use new PurgeComm() API to clear communications driver before
//       closing device.
//
//---------------------------------------------------------------------------

BOOL NEAR CloseConnection( HWND hWnd )
{
   HMENU      hMenu ;
   NPTTYINFO  npTTYInfo ;
   int	ForceStop=640000000;

   if (NULL == (npTTYInfo = GETNPTTYINFO( hWnd )))
      return ( FALSE ) ;

   // set connected flag to FALSE

   CONNECTED( npTTYInfo ) = FALSE ;

   // disable event notification and wait for thread
   // to halt

   SetCommMask( COMDEV( npTTYInfo ), 0 ) ;

   // block until thread has been halted

   while(THREADID(npTTYInfo) != 0 && ForceStop--);

   // kill the focus

   //KillTTYFocus( hWnd ) ;

   // drop DTR

   EscapeCommFunction( COMDEV( npTTYInfo ), CLRDTR ) ;

   // purge any outstanding reads/writes and close device handle

   PurgeComm( COMDEV( npTTYInfo ), PURGE_TXABORT | PURGE_RXABORT |
                                   PURGE_TXCLEAR | PURGE_RXCLEAR ) ;
   CloseHandle( COMDEV( npTTYInfo ) ) ;

   // change the selectable items in the menu

 /*  hMenu = GetMenu( hWnd ) ;
   EnableMenuItem( hMenu, IDM_DISCONNECT,
                   MF_GRAYED | MF_DISABLED | MF_BYCOMMAND ) ;
   EnableMenuItem( hMenu, IDM_CONNECT,
                   MF_ENABLED | MF_BYCOMMAND ) ;*/

   return ( TRUE ) ;

} // end of CloseConnection()

//---------------------------------------------------------------------------
//  int NEAR ReadCommBlock( HWND hWnd, LPSTR lpszBlock, int nMaxLength )
//
//  Description:
//     Reads a block from the COM port and stuffs it into
//     the provided buffer.
//
//  Parameters:
//     HWND hWnd
//        handle to TTY window
//
//     LPSTR lpszBlock
//        block used for storage
//
//     int nMaxLength
//        max length of block to read
//
//  Win-32 Porting Issues:
//     - ReadComm() has been replaced by ReadFile() in Win-32.
//     - Overlapped I/O has been implemented.
//
//---------------------------------------------------------------------------

int NEAR ReadCommBlock( HWND hWnd, LPSTR lpszBlock, int nMaxLength )
{
   BOOL       fReadStat ;
   COMSTAT    ComStat ;
   DWORD      dwErrorFlags;
   DWORD      dwLength;
   DWORD      dwError;
   char       szError[ 10 ] ;
   NPTTYINFO  npTTYInfo ;

   if (NULL == (npTTYInfo = GETNPTTYINFO( hWnd )))
      return ( FALSE ) ;

   // only try to read number of bytes in queue
   ClearCommError( COMDEV( npTTYInfo ), &dwErrorFlags, &ComStat ) ;
   dwLength = min( (DWORD) nMaxLength, ComStat.cbInQue ) ;

   if (dwLength > 0)
   {
      fReadStat = ReadFile( COMDEV( npTTYInfo ), lpszBlock,
		                    dwLength, &dwLength, &READ_OS( npTTYInfo ) ) ;
      if (!fReadStat)
      {
         if (GetLastError() == ERROR_IO_PENDING)
         {
            OutputDebugString("\n\rIO Pending");
            // We have to wait for read to complete.
            // This function will timeout according to the
            // CommTimeOuts.ReadTotalTimeoutConstant variable
            // Every time it times out, check for port errors
            while(!GetOverlappedResult( COMDEV( npTTYInfo ),
               &READ_OS( npTTYInfo ), &dwLength, TRUE ))
            {
               dwError = GetLastError();
               if(dwError == ERROR_IO_INCOMPLETE)
                  // normal result if not finished
                  continue;
               else
               {
                  // an error occurred, try to recover
                  wsprintf( szError, "<CE-%u>", dwError ) ;
                  WriteTTYBlock( hWnd, szError, lstrlen( szError ) ) ;
                  ClearCommError( COMDEV( npTTYInfo ), &dwErrorFlags, &ComStat ) ;
                  if ((dwErrorFlags > 0) && DISPLAYERRORS( npTTYInfo ))
                  {
	                  wsprintf( szError, "<CE-%u>", dwErrorFlags ) ;
	                  WriteTTYBlock( hWnd, szError, lstrlen( szError ) ) ;
                  }
                  break;
               }

            }

	      }
         else
         {
            // some other error occurred
            dwLength = 0 ;
            ClearCommError( COMDEV( npTTYInfo ), &dwErrorFlags, &ComStat ) ;
            if ((dwErrorFlags > 0) && DISPLAYERRORS( npTTYInfo ))
            {
	            wsprintf( szError, "<CE-%u>", dwErrorFlags ) ;
	            WriteTTYBlock( hWnd, szError, lstrlen( szError ) ) ;
            }
         }
      }
   }

   return ( dwLength ) ;

} // end of ReadCommBlock()

//---------------------------------------------------------------------------
//  BOOL NEAR WriteCommBlock( HWND hWnd, BYTE *pByte )
//
//  Description:
//     Writes a block of data to the COM port specified in the associated
//     TTY info structure.
//
//  Parameters:
//     HWND hWnd
//        handle to TTY window
//
//     BYTE *pByte
//        pointer to data to write to port
//
//  Win-32 Porting Issues:
//     - WriteComm() has been replaced by WriteFile() in Win-32.
//     - Overlapped I/O has been implemented.
//
//---------------------------------------------------------------------------

BOOL NEAR WriteCommBlock( HWND hWnd, LPSTR lpByte , DWORD dwBytesToWrite)
{

   BOOL        fWriteStat ;
   DWORD       dwBytesWritten ;
   NPTTYINFO   npTTYInfo ;
   DWORD       dwErrorFlags;
   DWORD   	dwError;
   DWORD       dwBytesSent=0;
   COMSTAT     ComStat;
   char        szError[ 128 ] ;


   if (NULL == (npTTYInfo = GETNPTTYINFO( hWnd )))
      return ( FALSE ) ;

   fWriteStat = WriteFile( COMDEV( npTTYInfo ), lpByte, dwBytesToWrite,
                           &dwBytesWritten, &WRITE_OS( npTTYInfo ) ) ;

   // Note that normally the code will not execute the following
   // because the driver caches write operations. Small I/O requests
   // (up to several thousand bytes) will normally be accepted
   // immediately and WriteFile will return true even though an
   // overlapped operation was specified

   if (!fWriteStat)
   {
      if(GetLastError() == ERROR_IO_PENDING)
      {
         // We should wait for the completion of the write operation
         // so we know if it worked or not

         // This is only one way to do this. It might be beneficial to
         // place the write operation in a separate thread
         // so that blocking on completion will not negatively
         // affect the responsiveness of the UI

         // If the write takes too long to complete, this
         // function will timeout according to the
         // CommTimeOuts.WriteTotalTimeoutMultiplier variable.
         // This code logs the timeout but does not retry
         // the write.

         while(!GetOverlappedResult( COMDEV( npTTYInfo ),
            &WRITE_OS( npTTYInfo ), &dwBytesWritten, TRUE ))
         {
            dwError = GetLastError();
            if(dwError == ERROR_IO_INCOMPLETE)
            {
               // normal result if not finished
               dwBytesSent += dwBytesWritten;
               continue;
            }
            else
            {
               // an error occurred, try to recover
               wsprintf( szError, "<CE-%u>", dwError ) ;
               WriteTTYBlock( hWnd, szError, lstrlen( szError ) ) ;
               ClearCommError( COMDEV( npTTYInfo ), &dwErrorFlags, &ComStat ) ;
               if ((dwErrorFlags > 0) && DISPLAYERRORS( npTTYInfo ))
               {
                  wsprintf( szError, "<CE-%u>", dwErrorFlags ) ;
                  WriteTTYBlock( hWnd, szError, lstrlen( szError ) ) ;
               }
               break;
            }
         }

         dwBytesSent += dwBytesWritten;

         if( dwBytesSent != dwBytesToWrite )
             wsprintf(szError,"\nProbable Write Timeout: Total of %ld bytes sent", dwBytesSent);
         else
             wsprintf(szError,"\n%ld bytes written", dwBytesSent);

         OutputDebugString(szError);

      }
      else
      {
         // some other error occurred
         ClearCommError( COMDEV( npTTYInfo ), &dwErrorFlags, &ComStat ) ;
         if ((dwErrorFlags > 0) && DISPLAYERRORS( npTTYInfo ))
         {
            wsprintf( szError, "<CE-%u>", dwErrorFlags ) ;
            WriteTTYBlock( hWnd, szError, lstrlen( szError ) ) ;
         }
         return ( FALSE );
      }
   }
   return ( TRUE ) ;

} // end of WriteCommBlock()

//---------------------------------------------------------------------------
//  BOOL NEAR WriteTTYBlock( HWND hWnd, LPSTR lpBlock, int nLength )
//
//  Description:
//     Writes block to TTY screen.  Nothing fancy - just
//     straight TTY.
// gl
//  Parameters:
//     HWND hWnd
//        handle to TTY window
//
//     LPSTR lpBlock
//        far pointer to block of data
//
//     int nLength
//        length of block
//
//---------------------------------------------------------------------------

BOOL NEAR WriteTTYBlock( HWND hWnd, LPSTR lpBlock, int nLength )
{
   int        i ;
   NPTTYINFO  npTTYInfo ;
   RECT       rect ;

   if (NULL == (npTTYInfo = GETNPTTYINFO( hWnd )))
      return ( FALSE ) ;

   for (i = 0 ; i < nLength; i++)
   {
      switch (lpBlock[ i ])
      {
         case ASCII_BEL:
            // Bell
            MessageBeep( 0 ) ;
            break ;

         case ASCII_BS:
            // Backspace
            if (COLUMN( npTTYInfo ) > 0)
               COLUMN( npTTYInfo ) -- ;
            //MoveTTYCursor( hWnd ) ;
            break ;

         case ASCII_CR:
            // Carriage return
            *(SCREEN( npTTYInfo ) + ROW( npTTYInfo ) * MAXCOLS +
                COLUMN( npTTYInfo )) = 0 ;
			//GPSOutput (hWnd,npTTYInfo->abScreen,npTTYInfo->nColumn);
             COLUMN( npTTYInfo ) = 0 ;
            //MoveTTYCursor( hWnd ) ;
            if (!NEWLINE( npTTYInfo ))
               break;

            // fall through

         case ASCII_LF:
            // Line feed
           if (ROW( npTTYInfo )++ == MAXROWS - 1)
            {
               _fmemmove( (LPSTR) (SCREEN( npTTYInfo )),
                          (LPSTR) (SCREEN( npTTYInfo ) + MAXCOLS),
                          (MAXROWS - 1) * MAXCOLS ) ;
               _fmemset( (LPSTR) (SCREEN( npTTYInfo ) + (MAXROWS - 1) * MAXCOLS),
                         ' ', MAXCOLS ) ;
               InvalidateRect( hWnd, NULL, FALSE ) ;
               ROW( npTTYInfo )-- ;
            }
           // MoveTTYCursor( hWnd ) ;
            break ;

         default:
            *(SCREEN( npTTYInfo ) + ROW( npTTYInfo ) * MAXCOLS +
                COLUMN( npTTYInfo )) = lpBlock[ i ] ;
            rect.left = (COLUMN( npTTYInfo ) * XCHAR( npTTYInfo )) -
                        XOFFSET( npTTYInfo ) ;
            rect.right = rect.left + XCHAR( npTTYInfo ) ;
            rect.top = (ROW( npTTYInfo ) * YCHAR( npTTYInfo )) -
                       YOFFSET( npTTYInfo ) ;
            rect.bottom = rect.top + YCHAR( npTTYInfo ) ;
            InvalidateRect( hWnd, &rect, FALSE ) ;

            // Line wrap
            if (COLUMN( npTTYInfo ) < MAXCOLS - 1)
               COLUMN( npTTYInfo )++ ;
            else if (AUTOWRAP( npTTYInfo ))
               WriteTTYBlock( hWnd, "\r\n", 2 ) ;
            break;
      }
   }
   return ( TRUE ) ;

} // end of WriteTTYBlock()

//---------------------------------------------------------------------------
//  VOID NEAR GoModalDialogBoxParam( HINSTANCE hInstance,
//                                   LPCSTR lpszTemplate, HWND hWnd,
//                                   DLGPROC lpDlgProc, LPARAM lParam )
//
//  Description:
//     It is a simple utility function that simply performs the
//     MPI and invokes the dialog box with a DWORD paramter.
//
//  Parameters:
//     similar to that of DialogBoxParam() with the exception
//     that the lpDlgProc is not a procedure instance
//
//---------------------------------------------------------------------------

VOID NEAR GoModalDialogBoxParam( HINSTANCE hInstance, LPCSTR lpszTemplate,
                                 HWND hWnd, DLGPROC lpDlgProc, LPARAM lParam )
{
   DLGPROC  lpProcInstance ;

   lpProcInstance = (DLGPROC) MakeProcInstance( (FARPROC) lpDlgProc,
                                                hInstance ) ;
   DialogBoxParam( hInstance, lpszTemplate, hWnd, lpProcInstance, lParam ) ;
   FreeProcInstance( (FARPROC) lpProcInstance ) ;

} // end of GoModalDialogBoxParam()

//---------------------------------------------------------------------------
//  BOOL FAR PASCAL AboutDlgProc( HWND hDlg, UINT uMsg,
//                                WPARAM wParam, LPARAM lParam )
//
//  Description:
//     Simulates the Windows System Dialog Box.
//
//  Parameters:
//     Same as standard dialog procedures.
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//  VOID NEAR FillComboBox( HINSTANCE hInstance, HWND hCtrlWnd, int nIDString,
//                          WORD NEAR *npTable, WORD wTableLen,
//                          WORD wCurrentSetting )
//
//  Description:
//     Fills the given combo box with strings from the resource
//     table starting at nIDString.  Associated items are
//     added from given table.  The combo box is notified of
//     the current setting.
//
//  Parameters:
//     HINSTANCE hInstance
//        handle to application instance
//
//     HWND hCtrlWnd
//        handle to combo box control
//
//     int nIDString
//        first resource string id
//
//     DWORD NEAR *npTable
//        near point to table of associated values
//
//     WORD wTableLen
//        length of table
//
//     DWORD dwCurrentSetting
//        current setting (for combo box selection)
//
//---------------------------------------------------------------------------

VOID NEAR FillComboBox( HINSTANCE hInstance, HWND hCtrlWnd, int nIDString,
                        DWORD NEAR *npTable, WORD wTableLen,
                        DWORD dwCurrentSetting )
{
   char  szBuffer[ MAXLEN_TEMPSTR ] ;
   WORD  wCount, wPosition ;

   for (wCount = 0; wCount < wTableLen; wCount++)
   {
      // load the string from the string resources and
      // add it to the combo box

      LoadString( hInstance, nIDString + wCount, szBuffer, sizeof( szBuffer ) ) ;
      wPosition = LOWORD( SendMessage( hCtrlWnd, CB_ADDSTRING, 0,
                                       (LPARAM) (LPSTR) szBuffer ) ) ;

      // use item data to store the actual table value

      SendMessage( hCtrlWnd, CB_SETITEMDATA, (WPARAM) wPosition,
                   (LPARAM) *(npTable + wCount) ) ;

      // if this is our current setting, select it

      if (*(npTable + wCount) == dwCurrentSetting)
         SendMessage( hCtrlWnd, CB_SETCURSEL, (WPARAM) wPosition, 0L ) ;
   }

} // end of FillComboBox()

//************************************************************************
//  DWORD FAR PASCAL CommWatchProc( LPSTR lpData )
//
//  Description:
//     A secondary thread that will watch for COMM events.
//
//  Parameters:
//     LPSTR lpData
//        32-bit pointer argument
//
//  Win-32 Porting Issues:
//     - Added this thread to watch the communications device and
//       post notifications to the associated window.
//
//************************************************************************

DWORD FAR PASCAL CommWatchProc( LPSTR lpData )
{
   DWORD       dwEvtMask ;
   NPTTYINFO   npTTYInfo = (NPTTYINFO) lpData ;
   OVERLAPPED  os ;
	int        nLength ;
   BYTE       abIn[ MAXBLOCK + 1] ;

   memset( &os, 0, sizeof( OVERLAPPED ) ) ;

   // create I/O event used for overlapped read

   os.hEvent = CreateEvent( NULL,    // no security
                            TRUE,    // explicit reset req
                            FALSE,   // initial event reset
                            NULL ) ; // no name
   if (os.hEvent == NULL)
   {
      MessageBox( NULL, "Failed to create event for thread!", "TTY Error!",
                  MB_ICONEXCLAMATION | MB_OK ) ;
      return ( FALSE ) ;
   }

   if (!SetCommMask( COMDEV( npTTYInfo ), EV_RXCHAR ))
      return ( FALSE ) ;

   while ( CONNECTED( npTTYInfo ) )
   {
      dwEvtMask = 0 ;

      WaitCommEvent( COMDEV( npTTYInfo ), &dwEvtMask, NULL );

      if ((dwEvtMask & EV_RXCHAR) == EV_RXCHAR)
      {
         do
         {
            if (nLength = ReadCommBlock( hTTYWnd, (LPSTR) abIn, MAXBLOCK ))
            {
               WriteTTYBlock( hTTYWnd, (LPSTR) abIn, nLength ) ;

               // force a paint

               UpdateWindow( hTTYWnd ) ;
            }
         }
         while ( nLength > 0 ) ;
      }
   }

   // get rid of event handle

   CloseHandle( os.hEvent ) ;

   // clear information in structure (kind of a "we're done flag")

   THREADID( npTTYInfo ) = 0 ;
   HTHREAD( npTTYInfo ) = NULL ;

   return( TRUE ) ;

} // end of CommWatchProc()


//---------------------------------------------------------------------------
//  End of File: tty.c
//---------------------------------------------------------------------------

