/*--------------------------------------------------------------------
|
| MovPlay.c - Sample Win app to play AVI movies using mciSendString
|
| Movie Functions supported:
|	Play/Pause
|	Home/End
|	Step/ReverseStep
| 
+--------------------------------------------------------------------*/
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
#include <digitalv.h>		
static	char	achCommand[255];
static	int		dw; 
static	long	ClosePos;
static	BOOL	Status=FALSE, Initmovie=FALSE; 
#include "gmextern.h"
HANDLE 	CopyDib(HANDLE hdib);


/**************************************************************
************************ GLOBALS ******************************
**************************************************************/
/* AVI stuff to keep around */
WORD wMCIDeviceID = 0;	/* MCI Device ID for the AVI file */
HWND hwndMovie;		/* window handle of the movie */
RECT rcMovie;		/* the rect where the movie is positioned      */
			/* for QT/W this is the movie rect, for AVI    */
			/* this is the location of the playback window */
BOOL fPlaying = FALSE;	/* Play flag: TRUE == playing, FALSE == paused */
BOOL fMovieOpen = FALSE;/* Open flag: TRUE == movie open, FALSE = none */
HANDLE hAccel = NULL;	/* accelerator table */
HMENU hMenuBar = NULL;	/* menu bar handle */
long	CurAVIFrame;

/********************************************************************
************************** FUNCTIONS ********************************
********************************************************************/
/*--------------------------------------------------------------+
| initAVI - initialize avi libraries				|
|								|
+--------------------------------------------------------------*/
BOOL GSinitAVI(void)
{                     
	if (Initmovie) return(TRUE);  
//	Init = initAVI();
//	return (Init);
    _fstrcpy (achCommand,"open avivideo");
	dw=mciSendString(achCommand, NULL, 0, NULL);
    if (dw)
    {
         if (Driving) KillTimer(VideoCntlWnd,VideoTimerNum);
		 mciGetErrorString(dw, achCommand,sizeof(achCommand));
		 MessageBox(GetFocus(), achCommand, "Video for Windows Error",MB_ICONEXCLAMATION|MB_OK);
		 return FALSE;
	}
	else 
	{
		Initmovie = TRUE;
		return TRUE;
	}
}

void GStermAVI(void)
{    
	if (!Initmovie) return; 
	fileCloseMovie(VideoWnd);
	Initmovie=FALSE;
    dw=mciSendString("close avivideo", NULL, 0, NULL);
    if (dw)
    {
         if (Driving) KillTimer(VideoCntlWnd,VideoTimerNum);
		 mciGetErrorString(dw, achCommand,sizeof(achCommand));
		 MessageBox(GetFocus(), achCommand, "Video for Windows Error",MB_ICONEXCLAMATION|MB_OK);
		 return;
	} 
	return;
}


/*--------------------------------------------------------------+ 
| positionMovie - sets the movie rectange <rcMovie> to be	|
|		centered within the app's window.		|
|								|
+--------------------------------------------------------------*/
VOID positionMovie(HWND hWnd)
{
   RECT	rcClient;
   RECT rcMovieBounds;
   char	achRect[128];
   char *p;
   
   /* if there is no movie yet then just get out of here */
   if (!fMovieOpen)
	   return;
   
   GetClientRect(hWnd, &rcClient);	/* get the parent windows rect */
	   
   /* get the original size of the movie */
   if ((dw=mciSendString("where mov source", (LPSTR)achRect, sizeof(achRect), NULL)))
   {
         if (Driving) KillTimer(VideoCntlWnd,VideoTimerNum);
   		 mciGetErrorString(dw, achCommand,sizeof(achCommand));
		 MessageBox(GetFocus(), achCommand, "Video for Windows Error",MB_ICONEXCLAMATION|MB_OK);
   }
   
   SetRectEmpty(&rcMovieBounds);	// zero out movie rect
   p = achRect;	// point to rectangle string returned by where command 
   while (*p == ' ') p++;	// skip over starting spaces
   while (*p != ' ') p++;	// skip over the x (which is 0) 
   while (*p == ' ') p++;	// skip over spaces between x and y
   while (*p != ' ') p++;	// skip over the y (which is 0)
   while (*p == ' ') p++;	// skip over the spaces between y and width
       
   /* now find the width */
   for (; *p >= '0' && *p <= '9'; p++)
       rcMovieBounds.right = (10 * rcMovieBounds.right) + (*p - '0');
   while (*p == ' ') p++;	// skip spaces between width and height
   
   /* now find the height */
   for (; *p >= '0' && *p <= '9'; p++)
       rcMovieBounds.bottom = (10 * rcMovieBounds.bottom) + (*p - '0');

   /* figure out where to position the window at */
   rcMovie.left = (rcClient.right/2) - (rcMovieBounds.right / 2);
   rcMovie.top = (rcClient.bottom/2) - (rcMovieBounds.bottom / 2);
   rcMovie.right = rcMovie.left + rcMovieBounds.right;
   rcMovie.bottom = rcMovie.top + rcMovieBounds.bottom;
   
   /* reposition the playback (child) window */
   MoveWindow(hwndMovie, rcMovie.left, rcMovie.top,
	   rcMovieBounds.right, rcMovieBounds.bottom, TRUE);
}

/*--------------------------------------------------------------+ 
| fileCloseMovie - close the movie and anything associated	|
|		   with it.					|
|								|
| This function clears the <fPlaying> and <fMovieOpen> flags	|
|								|
+--------------------------------------------------------------*/
void fileCloseMovie(HWND hWnd)
{ char	str[32];
  int	ii;  
  MSG	msg;
  
   
  if (!fMovieOpen) return; 
  CloseFrameList();
  fMovieOpen = FALSE;	// no more movies open
  if ((dw=mciSendString("status mov position", str,32, NULL)))
  {
         if (Driving) KillTimer(VideoCntlWnd,VideoTimerNum);
  		 mciGetErrorString(dw, achCommand,sizeof(achCommand));
		 MessageBox(GetFocus(), achCommand, "Video for Windows Error",MB_ICONEXCLAMATION|MB_OK);
  }
  ClosePos = atol(str);   
  Mounted=FALSE;
  CurMovie[0]='\0';
  SetAVIStatus (FALSE);  
  EnableVideoControls (FALSE);

 
/*  if (!mciSendString("seek mov to end notify", NULL, 0, VideoWnd))
	  GetMessage(&msg, VideoWnd, MM_MCINOTIFY, MM_MCINOTIFY);*/
  if ((dw=mciSendString("close mov notify", NULL, 0, VideoWnd)))
  {
         if (Driving) KillTimer(VideoCntlWnd,VideoTimerNum);
  		 mciGetErrorString(dw, achCommand,sizeof(achCommand));
		 MessageBox(GetFocus(), achCommand,"Video for Windows Error",MB_ICONEXCLAMATION|MB_OK);
  }
  else
  	GetMessage(&msg, VideoWnd, MM_MCINOTIFY, MM_MCINOTIFY);


  fPlaying = FALSE;	// can't be playing any longer
	   
  /* cause a total repaint to occur */
/*  InvalidateRect(hWnd, NULL, TRUE);
  UpdateWindow(hWnd);*/   
}

void ReOpenMovie (HWND hWnd)
{
  if (!hWnd || fMovieOpen || !CurMovie[0]) return;
  fileOpenMovie (hWnd,CurMovie);
  SetAVIFrame (ClosePos);
  return;
} 
void DisplayCurrentFrame (void)
{
 if (!CurMovie[0]) return;
 fileCloseMovie(VideoWnd);
 ReOpenMovie (VideoWnd);
/* SetAVIFrame (CurAVIFrame);
 SetAVIStatus (TRUE); 
		   dw=mciSendString("step mov by 1", NULL,0, NULL); */
 return;
}

/*--------------------------------------------------------------+ 
| fileOpenMovie - open an AVI movie. Use CommDlg open box to	|
|	        open and then handle the initialization to	|
|		show the movie and position it properly.  Keep	|
|		the movie paused when opened.			|
|								|
|		Sets <fMovieOpened> on success.			|
+--------------------------------------------------------------*/
BOOL fileOpenMovie(HWND hWnd, LPSTR Name)
{
   OPENFILENAME ofn;
   int	l,ii;
   HCURSOR	hCurSave;
   static int  nLastFilter = 1;	  /* keep last file-type opened */   
   MSG	msg;
   LPSTR	lpchr;
   int	Fid;
   OFSTRUCT	OFStruct;
   
   l=_fstrlen(Name);
/*   if (_fstricmp(&Name[l-4],".avi") !=0 && _fstricmp(&Name[l-4],".AVI")!=0)
   {
BadMovie:MessageBox(hWnd, Name, "Invalid Movie File",MB_ICONEXCLAMATION|MB_OK);
		return FALSE;
   }   */
   
hCurSave=GSSiSetCursor(LoadCursor(NULL, IDC_WAIT));
     
   	 ShowInitBM = FALSE;
	 if (fMovieOpen)
		 fileCloseMovie(hWnd);	
	
	OpenFrameList(Name,NULL); 
    wsprintf((LPSTR)achCommand,"open %s alias mov style child parent %u notify",
             Name,hWnd);

	 /* try to open the file */
	 if (!(dw=mciSendString((LPSTR)achCommand, NULL, 0, VideoWnd)))
	 {
         
		 GetMessage(&msg, VideoWnd, MM_MCINOTIFY, MM_MCINOTIFY);
         Status=FALSE;
		 fMovieOpen = TRUE;
         _fstrcpy (CurMovie,Name);
		 /* we opened the file o.k., now set up to */
		 /* play it.				   */
		 mciSendString("set mov seek exactly on", NULL, 0, NULL);
		 /* get the window handle */
		 if ((dw = mciSendString("status mov window handle", 
			(LPSTR)achCommand, sizeof(achCommand), 
			NULL)) == 0L)
		    hwndMovie = (HWND)atoi(achCommand);
		 else {
	         if (Driving) KillTimer(VideoCntlWnd,VideoTimerNum);
		     mciGetErrorString(dw, achCommand, 
				     sizeof(achCommand));
		     MessageBox(hWnd, achCommand, NULL,
				 MB_ICONEXCLAMATION|MB_OK);
		 }
		 /* now get the movie centered */
		 positionMovie(hWnd);
	 } else {
		 /* generic error for open */ 
		 mciGetErrorString(dw, achCommand,sizeof(achCommand));
		/* if (Attempts == 1) goto TryAgain;*/
		 MessageBox(hWnd, achCommand, Name,MB_ICONEXCLAMATION|MB_OK);
		 fMovieOpen = FALSE;
	 }
   
   /* cause an update to occur */
/*   InvalidateRect(hWnd, NULL, FALSE);
   UpdateWindow(hWnd);*/   
   if (fMovieOpen)   EnableVideoControls (TRUE);
   GSSiSetCursor (hCurSave);
   return fMovieOpen;
}

/*--------------------------------------------------------------+
| playMovie - play/pause the movie depending on the state	|
|		of the <fPlaying> flag.				|
|								|
| This function sets the <fPlaying> flag appropriately when done|
|								|
+--------------------------------------------------------------*/
void playMovie(HWND hWnd, WORD wDirection)
{
   fPlaying = !fPlaying;	/* swap the play flag */
   if (wDirection == NULL)
	   fPlaying = FALSE;	/* wDirection == NULL means PAUSE */

   /* play/pause the AVI movie */
   if (fPlaying){
	   if (wDirection == 2){
	        mciSendString("play mov reverse notify", NULL, 0, hWnd);
	   } else {
	       mciSendString("play mov notify", NULL, 0, hWnd);
	   }
   } else {
	   /* tell it to pause */
	   mciSendString("pause mov", NULL, 0, NULL);
   }
}

void GSFullScreenPlay(HWND hWnd, LPSTR Name)
{   
	char	str[256];
	
	fileCloseMovie(hWnd);
    wsprintf((LPSTR)str,"open %s alias mov",Name);

	mciSendString((LPSTR)str, NULL, 0, VideoWnd);
	mciSendString("play mov fullscreen", NULL, 0, hWnd); 
	mciSendString("close mov", NULL, 0, hWnd); 
	return;
}

void PlayVideo(HWND hWnd, LPSTR Name)
{   
	MSG		msg;
	char	str[256]; 
	RECT	Rect;
	
	GetWindowRect (hWnd,&Rect);
	fileCloseMovie(hWnd);
//    sprintf((LPSTR)str,"open %s alias mov style child parent %u notify",Name,hWnd);
    sprintf((LPSTR)str,"open %s alias mov style child parent %u",Name,hWnd);

//	PostMessage(VideoWnd, WM_COMMAND, NULL,NULL);
	if (!mciSendString((LPSTR)str, NULL, 0, hWnd))
	{
//	  	GetMessage(&msg, VideoWnd, MM_MCINOTIFY, MM_MCINOTIFY);
//		mciSendString("play mov notify", NULL, 0, 0/*hWnd*/); 
		mciSendString("play mov window notify", NULL, 0, hWnd); 
	  	GetMessage(&msg, VideoWnd, MM_MCINOTIFY, MM_MCINOTIFY);
	}
	mciSendString("close mov", NULL, 0, NULL); 
	return;
}

void PlayVideoFrame0(HWND hWnd, LPSTR Name)
{   
	MSG		msg;
	char	str[256];
	
	PostMessage(VideoWnd, WM_COMMAND, NULL,NULL);
    sprintf((LPSTR)str,"open %s alias mov style child parent %u notify",Name,hWnd);

	if (!mciSendString((LPSTR)str, NULL, 0, hWnd))
	{
	  	GetMessage(&msg, VideoWnd, MM_MCINOTIFY, MM_MCINOTIFY);
		mciSendString("play mov from 0 to 0 notify", NULL, 0, hWnd); 
	  	GetMessage(&msg, VideoWnd, MM_MCINOTIFY, MM_MCINOTIFY);
	}
	mciSendString("close mov", NULL, 0, NULL); 
	return;
}

void FullScreenFrame(HWND hWnd, long Frame)
{  
	char	str[128];
	
	sprintf (str,"play mov from %ld to %ld fullscreen",Frame, Frame);
	mciSendString(str, NULL, 0, hWnd); 
	return;
}

/*--------------------------------------------------------------+
| seekMovie - seek in the movie depending on the wAction.	|
|	      Possible actions are IDM_HOME (start of movie) or	|
|	      IDM_END (end of movie)				|
|								|
|	      Always stop the play before seeking.		|
|								|
+--------------------------------------------------------------*/
void seekMovie(HWND hWnd, WORD wAction)
{  
	return;
   /* first stop the movie from playing if it is playing */
   if (fPlaying){
	   playMovie(hWnd, NULL);	
   }
   if (!fMovieOpen)fileOpenMovie(hWnd, CurMovie);
   if (wAction == 1){
	   /* home the movie */
           mciSendString("seek mov to start", NULL, 0, NULL);
			
   } else if (wAction == 2){
	   /* go to the end of the movie */
           mciSendString("seek mov to end", NULL, 0, NULL);
   }
   if (!Status)SetAVIStatus (TRUE);  
}

/*--------------------------------------------------------------+
| stepMovie - step forward or reverse in the movie.  wDirection	|
|		can be IDM_STEP (forward) or IDM_RSTEP (reverse)|
|								|
|		Again, stop the play if one is in progress.	|
|								|
+--------------------------------------------------------------*/
void stepMovie(HWND hWnd, int wDirection)
{  int i;
   if (fPlaying)
	   playMovie(hWnd, NULL);  /* turn off the movie */
       
   if (!fMovieOpen)return;
   if (SubsetVideo)
   		GoToNextSubFrame (wDirection);
   else if (UseRawVideo)
   		GoToNextRawFrame (wDirection);
   else if (DriveHousesOnly) 
   {
   		if (!GoToNextHouseFrame (wDirection)) goto Next;
   }
   else
   {    
   		long	StepBy; 
   		char	str[128];
Next:  
		StepBy = max (1,GetGlobalLVal ("[%STEPBY]"));
	    if (wDirection == 1)
	   		sprintf (str,"step mov by %ld",StepBy);
	    else
	   		sprintf (str,"step mov reverse by %ld",StepBy);
	    dw=mciSendString(str, NULL,0, NULL); 
	   
   } 
   if (!Status)SetAVIStatus (TRUE);  

}

long GetAVIFrame (void)
{   char	str[256];
	static	long	pos;

    if (!fMovieOpen)return(pos);
	if ((dw=mciSendString("status mov position", str,255, NULL)))
	{
         if (Driving) KillTimer(VideoCntlWnd,VideoTimerNum);
		 mciGetErrorString(dw, achCommand,sizeof(achCommand));
		 MessageBox(GetFocus(), achCommand, "Video for Windows Error",MB_ICONEXCLAMATION|MB_OK);
	}	
	pos = atol(str);
    CurAVIFrame = pos; 
//    if (ShowFileName) SetDlgItemText (PresentedWnd,IDC_COURTESY_OF,str);
	return (pos);
}
void SetAVIFrame (long Frame)
{   char	str[256];
	long	pos;
    
    /*Frame = max (1,Frame);*/
    CurAVIFrame = Frame; 
/*    if (!Frame) 
		mciSendString("seek mov to 1", NULL, 0, NULL);*/
    sprintf (str,"seek mov to %ld",Frame);
    
	if ((dw=mciSendString(str, NULL, 0, NULL)))
	{
         if (Driving) KillTimer(VideoCntlWnd,VideoTimerNum);
		 mciGetErrorString(dw, achCommand,sizeof(achCommand));
		 MessageBox(GetFocus(), achCommand, "Video for Windows Error",MB_ICONEXCLAMATION|MB_OK);
	}	
    if (!Status)SetAVIStatus (TRUE);  
	return;
}

BOOL SetAVIFrameNotify (long Frame, HWND hWndNotify)
{   char	str[256];
	long	pos;
    
    if (!Status)SetAVIStatus (TRUE);  
    /*Frame = max (1,Frame);*/
    CurAVIFrame = Frame;
    sprintf (str,"seek mov to %ld notify",CurAVIFrame);
	if ((dw=mciSendString(str, NULL, 0, hWndNotify)))
	{
         if (Driving) KillTimer(VideoCntlWnd,VideoTimerNum);
		 mciGetErrorString(dw, achCommand,sizeof(achCommand));
		 MessageBox(GetFocus(), achCommand,  "Video for Windows Error",MB_ICONEXCLAMATION|MB_OK);
		 return FALSE;
	}	
	return TRUE;
}

void SetAVIStatus (BOOL On)
{   
    if (On && !Status)
    {
		if ((dw=mciSendString("setvideo mov on", NULL, 0, NULL)))
		{
	         if (Driving) KillTimer(VideoCntlWnd,VideoTimerNum);
			 mciGetErrorString(dw, achCommand,sizeof(achCommand));
			 MessageBox(GetFocus(), achCommand, "Video for Windows Error",MB_ICONEXCLAMATION|MB_OK);
		}
		
		if ((dw=mciSendString("window mov state show", NULL, 0, NULL)))
		{
	         if (Driving) KillTimer(VideoCntlWnd,VideoTimerNum);
			 mciGetErrorString(dw, achCommand,sizeof(achCommand));
			 MessageBox(GetFocus(), achCommand, "Video for Windows Error",MB_ICONEXCLAMATION|MB_OK);
		}
/*		PostMessage(VideoWnd, WM_COMMAND, NULL,NULL);*/
		
    }
	else if (!On && Status)
	{
	/*	if ((dw=mciSendString("setvideo mov off", NULL, 0, NULL)))
		{
	         if (Driving) KillTimer(VideoCntlWnd,VideoTimerNum);
			 mciGetErrorString(dw, achCommand,sizeof(achCommand));
			 MessageBox(GetFocus(), achCommand, "Video for Windows Error",MB_ICONEXCLAMATION|MB_OK);
		}*/
	}
	Status=TRUE;    
	return;
} 
 
HANDLE FrameToDIB (HWND hWnd)
{
	PAVIFILE	pfile=0;
	PAVISTREAM 	pstream=0;
	PGETFRAME	pget=0;

	HPALETTE	hPal, hPalNew = NULL;	// remember to init these
	HRESULT		hResult; 
	BITMAPINFOHEADER	bi;
	LPBITMAPINFOHEADER  lpbi;
	LPSTR		lpDIBBits;
	HDIB		NewDIB=0;
	char		Name[128];
	                      
	_fstrcpy (Name,CurMovie);
	if (!Name[0]) return 0;
	fileCloseMovie(hWnd); 
	GStermAVI();
	AVIFileInit();
	hResult = AVIFileOpen(&pfile, Name, 0, 0L);
	hResult = AVIFileGetStream(pfile, &pstream, streamtypeVIDEO, 0);
	
	_fmemset (&bi,0,sizeof(BITMAPINFOHEADER));
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = 320;
	bi.biHeight = 240;
	bi.biPlanes = 1;
	bi.biBitCount =24; 
	bi.biCompression = BI_RGB;
	bi.biSizeImage = (long) bi.biWidth * (long) bi.biHeight * 3;
	pget = AVIStreamGetFrameOpen(pstream,&bi);
	
	if(pget) { // Great, decompressor open

		// Get our own personal copy of the DIB
		lpbi = AVIStreamGetFrame(pget, CurAVIFrame);
		if (lpbi)  
		{   
			
		/*	NewDIB = ConvertBitmap (lpbi);*/
			NewDIB=CopyDib((HANDLE)HIWORD((DWORD)(lpbi)));
		}
		AVIStreamGetFrameClose(pget);
	} 
	
	// Close the stream and file
	AVIStreamRelease( pstream );
	AVIFileRelease( pfile ); 
	AVIFileExit();
	_fstrcpy (CurMovie,Name); 
	return NewDIB;
}

HANDLE FrameToDIB2 (LPSTR File, long Frame)
{
	PAVIFILE	pfile=0;
	PAVISTREAM 	pstream=0;
	PGETFRAME	pget=0;

	HPALETTE	hPal, hPalNew = NULL;	// remember to init these
	HRESULT		hResult; 
	BITMAPINFOHEADER	bi;
	LPBITMAPINFOHEADER  lpbi;
	LPSTR		lpDIBBits;
	HDIB		NewDIB=0;
	char		Name[128];
	                      
	_fstrcpy (Name,CurMovie);
	fileCloseMovie(VideoWnd); 
	GStermAVI();
	AVIFileInit();
	hResult = AVIFileOpen(&pfile, File, 0, 0L); 
	if (!pfile) goto Exit1;
	hResult = AVIFileGetStream(pfile, &pstream, streamtypeVIDEO, 0);
	if (!pstream) goto Exit2;
	
	_fmemset (&bi,0,sizeof(BITMAPINFOHEADER));
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = 320;
	bi.biHeight = 240;
	bi.biPlanes = 1;
	bi.biBitCount =24; 
	bi.biCompression = BI_RGB;
	bi.biSizeImage = (long) bi.biWidth * (long) bi.biHeight * 3;
	pget = AVIStreamGetFrameOpen(pstream,NULL);
	pget = AVIStreamGetFrameOpen(pstream,&bi);
	
	
	if(pget) { // Great, decompressor open

		// Get our own personal copy of the DIB
		lpbi = AVIStreamGetFrame(pget, Frame);
		if (lpbi)  
		{   
			
		/*	NewDIB = ConvertBitmap (lpbi);*/
			NewDIB=CopyDib((HANDLE)HIWORD((DWORD)(lpbi)));
		}
		AVIStreamGetFrameClose(pget);
	} 
	
	
	// Close the stream and file
	AVIStreamRelease( pstream );
Exit2:
	AVIFileRelease( pfile );
Exit1: 
	AVIFileExit();
	_fstrcpy (CurMovie,Name); 
	return NewDIB;
}

/*********************************************************
 ***  CopyDib, makes a copy of a DIB and locks it down
 *********************************************************/

HANDLE CopyDib (HANDLE hdib)
{
    BYTE HUGE *ps;
    BYTE HUGE *pd;
    HANDLE h;
    DWORD cnt; 
    extern	BOOL IgnoreLock;

    if (h = GSSiGlobAlloc ( 400,GMEM_MOVEABLE, cnt = GlobalSize(hdib)))
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


