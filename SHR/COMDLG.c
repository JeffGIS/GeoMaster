#include "graphint.h"
#include "cddemo.h"
#include "cderr.h"
#include <winspool.h>
#include <winuser.h>
#include <shlobj.h>

#include "gmextern.h"

UINT CALLBACK  FontHook (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

void SetIgnoreError (BOOL setting);

BOOL	CreatePrintPreview=FALSE; 
BOOL	PrintWasCanceled;
BOOL	InPrintSetup=TRUE;
char	CurVirtPrinter[128]; 
char	VirtPrinterImageFile[256]="";
short	NumPrintCopies=0;
char	CustomHeight[16]="",CustomWidth[16]="";
static	char	customFilter[256] = { 0 };

static	HWND		NameWnd,OpenWnd;
static	HWND        ghFFRDlg;
//extern HWND        ghPrintingDlg;
static	HBRUSH      ghBkgndBrush, ghDlgBrush;
//static	HFONT       ghSelectedFont;
static	WORD        wFRMsg;
static	WORD        wHelpMsg;
static	BOOL        gbMonochrome;
static	BOOL		AllowShadowColor;
static	COLORREF	ShadowColor,FontColor;
//extern BOOL        gbUserAbort;
//extern char        gszCommonWClass[];
//extern char        gszAllocErrorMsg[];
//extern char        gszLockErrorMsg[];
//extern char        gszFontMsg[];
//extern char        gszPrintMsg[];
//extern char        gszTestDoc[];
//extern char        gszWin31wh[];
//extern char        gszLoadStrFail[];
//extern char        gszFOSuccess[];
//extern char        gszFOFailure[];

static char			gszMenuName[]="CommonDlgMenu";
static char         gszCommonWClass[]="CommonWClass";
static char         gszAllocErrorMsg[]="Error Allocating Memory!";
static char         gszLockErrorMsg[]="Error Locking Memory!";
static char         gszFontMsg[]="Hello Windows World!";
static char         gszPrintMsg[]="Common Dialog Print Sample";
static char         gszTestDoc[]="Test-Doc";
static char         gszWin31wh[]="win31wh.hlp";
static char         gszLoadStrFail[]="LoadString failed!";
static char         gszFOSuccess[]="File successfully opened and closed";
static char         gszFOFailure[]="Failure finding specified file";   
static WNDPROC		g_OldEdit=0;
static LPRGBTRIPLE	pColorPalette;
static int			nPaletteColors;
static HWND			hWndCFP;
//static	DLGPROC	lpfnABORTWAITMsgProc;
static	BOOL	AllowCreate=FALSE, HaveAbortProc=FALSE;
static	BYTE	R,G,B,W; 


static HANDLE	hColorsChunk=0;
static short	ExtraOpenFlags=0;
static char		InitialDirectory[MAX_PATH];
static char		InitialFile[MAX_PATH]; 
static char		OFTitle2[256];
static char		Filter[256],CustomFilter[256],FileTitle[256],InitialDir[256],Title[256],DefExt[256];
static UINT		FilterStringID=IDS_FILTERSTRING;   
static short	fsLen;
static char		TempFile32Name[MAX_PATH]="";
static LPOPENFILENAME pOF; 
static short	GetFile32Opt;
static char	PltFilter1[]="GeoMaster Graphics Files(*.PLT)|*.plt|Shape Files(*.SHP)|*.shp|Personal GeoDatabases(*.MDB)|*.mdb|File GeoDatabases(*.GDB)|*.gdb*|DGN Files(*.DGN)|*.dgn|";
static char	PltFilter2[]="Oracle Spatial Export Files(*.ORA)|*.ora|Mr Sid Ortho Files(*.SID)|*.sid|Map or Ortho Indexes(INDEX*)|index*|Map or Ortho Filelists(FILELIST.TXT)|filelist.txt||";

//BOOL FAR PASCAL GETFILE32MsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);

BOOL GetFile32 (short opt,LPOPENFILENAME pof,DWORD lFilters)
{   
	short	nRc;
	
/*    if (GetGlobalBVal2 ("[%USE32BITGETFILE]",FALSE))
    {
		DLGPROC lpfnGETFILE32MsgProc;  
		
		GetFile32Opt = opt;
		pOF = pof;
		lpfnGETFILE32MsgProc = MakeProcInstance((DLGPROC)GETFILE32MsgProc, hInst);
		nRc = DialogBox(hInst, (LPSTR)"GETFILE32", hWndMain, lpfnGETFILE32MsgProc);
		FreeProcInstance(lpfnGETFILE32MsgProc);   
		return nRc;
    }
    else*/
		return (GSSiGetFileName (opt,pof->hwndOwner,pof->hInstance,pof->lpstrFile,pof->lpstrFilter,lFilters,pof->lpstrInitialDir,pof->lpstrTitle,pof->Flags));
	
}

/*BOOL FAR PASCAL GETFILE32MsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
	char	str[512], ExPath[128];
	UINT hI; 
	HFILE	Fid;
	OFSTRUCTGM	OFStruct;  
	short	opt=1, ln, ln0=0;
	static	HANDLE		hSaveBM;
   
	
 switch(Message)
   {
    case WM_INITDIALOG:  
		 hSaveBM = EnterBlockingWindow (hWndDlg);
    	 PostMessage(hWndDlg, WM_COMMAND, IDC_LOADFILE32, 0L);
         break; 
    case WM_CLOSE:
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; 

    case WM_COMMAND:
         switch(wParam)
         {  
            case IDC_LOADFILE32: 
            	if (!*TempFile32Name)
            		GSSiGetTempFileName (0,"gm3",0,TempFile32Name); 
            	Fid = OpenFile (TempFile32Name,&OFStruct,OF_CREATE);
            	BigWrite (Fid,(HPSTR)&pOF->nMaxCustFilter,4,-1); 
            	BigWrite (Fid,(HPSTR)&pOF->nFilterIndex,4,-1); 
            	BigWrite (Fid,(HPSTR)&pOF->nMaxFile,4,-1); 
            	BigWrite (Fid,(HPSTR)&pOF->nMaxFileTitle,4,-1); 
            	BigWrite (Fid,(HPSTR)&pOF->Flags,4,-1); 
            	BigWrite (Fid,(HPSTR)&pOF->nFileOffset,2,-1); 
            	BigWrite (Fid,(HPSTR)&pOF->nFileExtension,2,-1); 
            	
            	if (pOF->lpstrFilter)
            	{
	            	ln = fsLen+1;
	            	BigWrite (Fid,(HPSTR)&ln,2,-1); 
	            	BigWrite (Fid,(HPSTR)pOF->lpstrFilter,ln,-1);
	            }
	            else 
	            	BigWrite (Fid,(HPSTR)&ln0,2,-1); 

            	if (pOF->lpstrCustomFilter)
            	{
	            	ln = _fstrlen (pOF->lpstrCustomFilter)+1;
	            	BigWrite (Fid,(HPSTR)&ln,2,-1); 
	            	BigWrite (Fid,(HPSTR)pOF->lpstrCustomFilter,ln,-1); 
	            }
	            else 
	            	BigWrite (Fid,(HPSTR)&ln0,2,-1); 

            	if (pOF->lpstrFile)
            	{
	            	ln = _fstrlen (pOF->lpstrFile)+1;
	            	BigWrite (Fid,(HPSTR)&ln,2,-1); 
	            	BigWrite (Fid,(HPSTR)pOF->lpstrFile,ln,-1); 
	            }
	            else 
	            	BigWrite (Fid,(HPSTR)&ln0,2,-1); 

            	if (pOF->lpstrFileTitle)
            	{
	            	ln = _fstrlen (pOF->lpstrFileTitle)+1;
	            	BigWrite (Fid,(HPSTR)&ln,2,-1); 
	            	BigWrite (Fid,(HPSTR)pOF->lpstrFileTitle,ln,-1); 
	            }
	            else 
	            	BigWrite (Fid,(HPSTR)&ln0,2,-1); 

            	if (pOF->lpstrInitialDir)
            	{
	            	ln = _fstrlen (pOF->lpstrInitialDir)+1;
	            	BigWrite (Fid,(HPSTR)&ln,2,-1); 
	            	BigWrite (Fid,(HPSTR)pOF->lpstrInitialDir,ln,-1); 
	            }
	            else 
	            	BigWrite (Fid,(HPSTR)&ln0,2,-1); 

            	if (pOF->lpstrTitle)
            	{
	            	ln = _fstrlen (pOF->lpstrTitle)+1;
	            	BigWrite (Fid,(HPSTR)&ln,2,-1); 
	            	BigWrite (Fid,(HPSTR)pOF->lpstrTitle,ln,-1); 
	            }
	            else 
	            	BigWrite (Fid,(HPSTR)&ln0,2,-1); 

            	if (pOF->lpstrDefExt)
            	{
	            	ln = _fstrlen (pOF->lpstrDefExt)+1;
	            	BigWrite (Fid,(HPSTR)&ln,2,-1); 
	            	BigWrite (Fid,(HPSTR)pOF->lpstrDefExt,ln,-1); 
	            }
	            else 
	            	BigWrite (Fid,(HPSTR)&ln0,2,-1); 
            	
            	_lclose (Fid);  
            	_fstrcpy (ExPath,"[%DL]GETFIL32.exe");
            	ExpandText (ExPath);
            	GetShortPathName2 (ExPath,sizeof(ExPath));
            	sprintf (str,"%s %s %i %ld",ExPath,TempFile32Name,(int)GetFile32Opt,(long)hWndDlg);
//				GSSiMsgBox (0,str,NULL,MB_OK);
				hI = WinExec (str,SW_SHOW);
				if (hI <32)
				{
                	GSSiEndDialog(hWndDlg, -1,hSaveBM);
				} 
            	break;
            case IDOK:
				Fid = OpenFile (TempFile32Name,&OFStruct,OF_READ);
				BigRead (Fid,(HPSTR)&pOF->nMaxCustFilter,4); 
				BigRead (Fid,(HPSTR)&pOF->nFilterIndex,4); 
				BigRead (Fid,(HPSTR)&pOF->nMaxFile,4); 
				BigRead (Fid,(HPSTR)&pOF->nMaxFileTitle,4); 
				BigRead (Fid,(HPSTR)&pOF->Flags,4); 
				BigRead (Fid,(HPSTR)&pOF->nFileOffset,2); 
				BigRead (Fid,(HPSTR)&pOF->nFileExtension,2); 
				
				BigRead (Fid,(HPSTR)&ln,2);
				if (ln) 
				{
					BigRead (Fid,(HPSTR)Filter,ln);
					pOF->lpstrFilter = Filter;
				}
				
				BigRead (Fid,(HPSTR)&ln,2); 
				if (ln)
				{
					BigRead (Fid,(HPSTR)CustomFilter,ln); 
					pOF->lpstrCustomFilter = CustomFilter;
				}
				
				BigRead (Fid,(HPSTR)&ln,2);
				if (ln)
				{ 
					BigRead (Fid,(HPSTR)InitialFile,ln); 
					pOF->lpstrFile = InitialFile;
				}
				
				BigRead (Fid,(HPSTR)&ln,2); 
				if (ln)
				{
					BigRead (Fid,(HPSTR)FileTitle,ln); 
					pOF->lpstrFileTitle = FileTitle;
				}
				
				BigRead (Fid,(HPSTR)&ln,2); 
				if (ln)
				{
					BigRead (Fid,(HPSTR)InitialDir,ln); 
					pOF->lpstrInitialDir = InitialDir;
				}
				
				BigRead (Fid,(HPSTR)&ln,2); 
				if (ln)
				{
					BigRead (Fid,(LPSTR)Title,ln);
					pOF->lpstrTitle = Title;
				}
				
				BigRead (Fid,(HPSTR)&ln,2);
				if (ln)
				{ 
					BigRead (Fid,(LPSTR)DefExt,ln);
					pOF->lpstrDefExt = DefExt;
				}
				_lclose (Fid);
				SetWindowPos(pOF->hwndOwner,HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
            	break;
            case IDCANCEL: 
				SetWindowPos(pOF->hwndOwner,HWND_TOP,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);

            break;
         }
         break; 

    default:
        return FALSE;
   }
 return TRUE;
}*/


 
void SetOpenFlags (short Flags)
{
	ExtraOpenFlags = Flags;
	return;
}

void SetCreateFile (void)
{   
	ExtraOpenFlags = OFN_CREATEPROMPT;
	AllowCreate = TRUE;
	return;
}

BOOL CDInit (HWND hWnd, HINSTANCE Inst)
{     HDC	hDC;

      ghWnd = hWnd;
	  ghInst = Inst;
      wFRMsg = RegisterWindowMessage((LPSTR)FINDMSGSTRING);
      wHelpMsg = RegisterWindowMessage((LPSTR)HELPMSGSTRING);
      lpfnFileOpenHook = (LPOFNHOOKPROC)MakeProcInstance(FileOpenHook, ghInst);
      lpfnFindHook = (LPOFNHOOKPROC)MakeProcInstance(FindHook, ghInst);
      lpfnFindReplaceHook = (LPOFNHOOKPROC)MakeProcInstance(FindReplaceHook, ghInst);
      lpfnColorHook = (LPOFNHOOKPROC)MakeProcInstance(ColorHook, ghInst);
      lpfnPrintSetupHook = (LPOFNHOOKPROC)MakeProcInstance(PrintSetupHook, ghInst);
      if (lpfnColorHook == NULL || lpfnFileOpenHook == NULL || lpfnFindHook == NULL ||
          lpfnFindReplaceHook == NULL || lpfnPrintSetupHook == NULL)
         return (FALSE);

      //Are we in monochrome land????
      hDC = GetDC(NULL);
      gbMonochrome = (2 == GetDeviceCaps(hDC, NUMCOLORS));  // Monochrome!
      ReleaseDC(NULL, hDC);

      //Load brush for painting GetOpenFileName dlg box
      if (!gbMonochrome)
      {
         HBITMAP hTempBitmap;

/*         hTempBitmap = LoadBitmap(ghInst, MAKEINTRESOURCE(1));
         if (hTempBitmap)*/
         {
            ghDlgBrush = CreateSolidBrush(RGB(225,225,255));
/*            DeleteObject(hTempBitmap);*/
         }
      }
      return (TRUE);
}

void CDClose (void)
{
    
	if (ghDlgBrush) DeleteObject(ghDlgBrush);
	if (hPDChunk)
	{   
		lpPDChunk = (LPPRINTDLG)GlobalLock (hPDChunk);
	    IgnoreLock = TRUE;
	    GSSiGlobFree(&lpPDChunk->hDevMode);
	    GSSiGlobFree(&lpPDChunk->hDevNames);
	    IgnoreLock = FALSE;
        GSSiGlobUlFree (&hPDChunk);
    }
   	GSSiGlobFree(&hColorsChunk);
	return;
}


BOOL GetFont (HWND ghWnd, LPLOGFONT lpLogFont, COLORREF *dwFontColor, COLORREF *dwShadowColor,HFONT *phSelectedFont)
{
   /*******************************************************************
   *                                                                  *
   *                             FONTS VARIABLES                      *
   *                                                                  *
   *******************************************************************/
   #define DESIREDPOINTSIZE 12

   //   HDC            hDC;
   int iLogPixsY;
   LOGFONT lf;
   LPCHOOSEFONT lpFontChunk;
   HANDLE hFontChunk;
   HFONT hOldFont;
	WORD wSize;
	HDC	hDC;
	BOOL	rtn=FALSE;

         wSize = sizeof(CHOOSEFONT);
         if (!(lpFontChunk = (LPCHOOSEFONT)AllocAndLockMem(&hFontChunk, wSize))
         )
            return(FALSE);
         InitializeStruct(IDC_FONT, (LPSTR)lpFontChunk);

      //Because we are only getting screen fonts, we can set hDC = NULL
      //If printer fonts are desired, a handle to a printer DC must be passed in
         lpFontChunk->hDC = NULL;

         //Now let's initialize the logfont structure.
      	 lpFontChunk->hwndOwner = ghWnd;   
      	 FontColor = lpFontChunk->rgbColors = *dwFontColor;
		 if (dwShadowColor)
		 {
			 AllowShadowColor = TRUE;
			 ShadowColor = *dwShadowColor;
		 }
		 else
			 AllowShadowColor = FALSE;
         hDC = GetDC(ghWnd); 
         
         iLogPixsY = GetDeviceCaps(hDC, LOGPIXELSY);
         ReleaseDC(ghWnd, hDC);
         lf.lfHeight = -1 * (iLogPixsY * DESIREDPOINTSIZE / 72);
         lf.lfWidth = 0;
         lf.lfEscapement = 0;
         lf.lfOrientation = 0;
         lf.lfWeight = 400;
         lf.lfItalic = 0;
         lf.lfUnderline = 0;
         lf.lfStrikeOut = 0;
         lf.lfCharSet = ANSI_CHARSET;
         lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
         lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
         lf.lfQuality = DEFAULT_QUALITY;
         lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
         _fstrcpy(lf.lfFaceName, "Arial Rounded MT Bold");
       	 lpFontChunk->lpLogFont = &lf;
         if (lpLogFont->lfHeight)*lpFontChunk->lpLogFont = *lpLogFont;
         if (ChooseFont(lpFontChunk)) //Now let's create the selected font!
         {
            if (phSelectedFont)
			{
				GSSiDeleteObject(phSelectedFont);
				*phSelectedFont = CreateFontIndirect((LPLOGFONT)(lpFontChunk->
                                                lpLogFont));
			}
            *dwFontColor = FontColor;//lpFontChunk->rgbColors;
			if (dwShadowColor)
				*dwShadowColor = ShadowColor;
            *lpLogFont=*lpFontChunk->lpLogFont;
//            InvalidateRect(ghWnd, NULL, TRUE); 
            rtn = TRUE;
         }
         else
         {
            ProcessCDError(CommDlgExtendedError());
         }
         GSSiGlobUlFree (&hFontChunk);
         return rtn;

}
LRESULT CALLBACK NewColorEditProc (HWND hwnd, UINT message, 
                             WPARAM wParam, LPARAM lParam)
{
	static	int	nPerRow, rowHeight, colorWidth, leftmargin, topmargin;
	int		irow, icol;
	static	int	icolor;

	
	switch (message)
	{
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC	hDC = BeginPaint (hwnd,&ps);
				RECT	rect=ps.rcPaint;
				LPRGBTRIPLE	pPalette = pColorPalette;

				nPerRow = sqrt (nPaletteColors);
				rowHeight = RECTHEIGHT(&ps.rcPaint)/nPerRow;
				colorWidth = RECTWIDTH(&ps.rcPaint)/nPerRow;
				topmargin = (RECTHEIGHT(&ps.rcPaint) - (rowHeight * nPerRow)) / 2;
				leftmargin = (RECTWIDTH(&ps.rcPaint) - (colorWidth * nPerRow)) / 2;
				rect.bottom = rect.top + rowHeight;
				for (irow = 0;irow<nPerRow;irow++)
				{

					rect.left = ps.rcPaint.left + leftmargin;
					rect.right = ps.rcPaint.left + colorWidth + topmargin;
					for (icol = 0;icol<nPerRow;icol++)
					{
						COLORREF	color = COLORREFFromRGBTRIPLE (*pPalette++);
						FillRectColor (hDC,&rect,color);
						rect.left += colorWidth;
						rect.right += colorWidth;
					}
					rect.top += rowHeight;
					rect.bottom += rowHeight;
				}
				EndPaint (hwnd,&ps);
			}
			break;

		case WM_MOUSEMOVE:
		{
			HDC		hDC;
    		POINT	MovePoint = POINTStoPOINT(MAKEPOINTS (lParam)); 
			int		row = min (max (0,(MovePoint.y - topmargin) / rowHeight),nPerRow-1);
			int		col = min (max (0,(MovePoint.x - leftmargin) / colorWidth),nPerRow-1);
			RECT	Rect;
			COLORREF	color;
			char	str[32];
			
			icolor = row * nPerRow + col;
			GetClientRect (GetDlgItem (hWndCFP,IDC_SHOWCOLOR),&Rect);
			hDC = GetDC (GetDlgItem (hWndCFP,IDC_SHOWCOLOR));
			color = COLORREFFromRGBTRIPLE (*(pColorPalette+icolor));
			FillRectColor (hDC,&Rect,color);
			itoa (icolor,str,10);
			SetDlgItemText (hWndCFP,IDC_ICOLOR,str);
			sprintf (str,"%i,%i,%i",GetRValue(color),GetGValue(color),GetBValue(color));
			SetDlgItemText (hWndCFP,IDC_RGB,str);

			ReleaseDC (GetDlgItem (hWndCFP,IDC_SHOWCOLOR),hDC);
		}
		break;
	case WM_LBUTTONDOWN:
        PostMessage(hWndCFP,WM_COMMAND, IDOK, icolor);
		break;

	case WM_CHAR:
//		chCharCode = (TCHAR) wParam;
//		if(chCharCode > 0x20 && !IsCharAlpha(chCharCode))
//			return 0;
		break;
	}
Exit:
	return CallWindowProc (g_OldEdit, hwnd, message, wParam, lParam);
}


BOOL CALLBACK COLORFROMPALETTEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{

	switch(Message) 
	{
		 case WM_INITDIALOG: 
		 { 
			 hWndCFP = hWndDlg;
			 g_OldEdit = (WNDPROC)SetWindowLong(GetDlgItem(hWndDlg,IDC_SHOWPALETTE), GWL_WNDPROC, (LONG)NewColorEditProc);
		 } 
    	 break; 
		 
    case WM_PAINT:
		{
			PAINTSTRUCT	ps;
			HDC	hDC;

			_fmemset(&ps, 0x00, sizeof(PAINTSTRUCT));
            hDC = BeginPaint(hWndDlg, &ps);
            EndPaint(hWndDlg, &ps);
		}
         break; 

    case WM_CLOSE:
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; 

    case WM_COMMAND:
		 switch(LOWORD (wParam))
         {
            case IDCANCEL:
                EndDialog(hWndDlg, -1);
            case IDOK:
                EndDialog(hWndDlg, lParam);
                break;

         }
         break;  

    default:
        return FALSE;
   }
   return TRUE;
}

int GetColorFromPalette (HWND hWnd,LPRGBTRIPLE pColors,int nColors)
{
	int	selectedColor =-1;

/*	RECT	DialogRect={100,100,500,500};

	HWND hWndDlg = CreateWindow("EDIT","Select Color From Palette",
				WS_CAPTION|WS_BORDER|WS_POPUP| WS_THICKFRAME|WS_SYSMENU|WS_VISIBLE|WS_CLIPSIBLINGS,
				DialogRect.left  , DialogRect.top  , RECTWIDTH(&DialogRect)  ,RECTHEIGHT(&DialogRect),
				0,0, hInst, 0);
*/
	pColorPalette = pColors;
	nPaletteColors = nColors;
	selectedColor = DialogBox(hInst, (LPSTR)"COLORFROMPALETTE", hWnd, (DLGPROC)COLORFROMPALETTEMsgProc);
	return selectedColor;
}

BOOL GetColor (HWND hWnd,COLORREF *Color)
{
   /*******************************************************************
   *                                                                  *
   *                             COLORS VARIABLES                     *
   *                                                                  *
   *******************************************************************/
   static LPCOLORSCHUNK lpColorsChunk = 0;
   WORD wSize;
	BYTE		PatByt;
	PATBYTE		PatByte;  
	HANDLE		hSaveBM;
   
	hSaveBM = EnterBlockingWindow (hWnd);
   ghWnd = hWndMain;      

         if (!hColorsChunk)   //ie, haven't called colors yet
         //so let's initialize everything
         {
            wSize = sizeof(COLORSCHUNK);
            if (!(lpColorsChunk = (LPCOLORSCHUNK)AllocAndLockMem(&hColorsChunk,
                                                                 wSize))) 
            {
			   EnableWindow (hWnd,TRUE);
               return (FALSE);
            }
            InitializeStruct(IDC_COLORS, (LPSTR)lpColorsChunk);
         }
         else
         	lpColorsChunk = (LPCOLORSCHUNK)GlobalLock(hColorsChunk);
/*	 EnableWindow (hWndMain,FALSE);*/
		 lpColorsChunk->chsclr.rgbResult = ColorWOWidth(*Color);  
		 PatByt = GetWValue (*Color);
    	 _fmemmove (&PatByte,&PatByt,1);
				
/*				    		if (PatByte.Pattern)
				    		{
		    				    SetTextColor (CurView->hDC,ColorWOWidth (CurTheme->ClassColor[iclass]));     
							    SetBkColor (CurView->hDC,RGB(255,255,255));
							    if (PatByte.Transparent)
									SetROP2(CurView->hDC,R2_MASKPEN);
                            } */
		R = GetRValue (*Color);
		G = GetGValue (*Color);
		B = GetBValue (*Color);
		W = GetWValue (*Color);
         if (ChooseColor(&(lpColorsChunk->chsclr)))
         {
		 	*Color = RGBW (R,G,B,W);
/*	 EnableWindow (hWndMain,TRUE);*/   
			GlobalUnlock(hColorsChunk); 
	     	LeaveBlockingWindow(hSaveBM);
		 	return (TRUE);
		 }
         else
            ProcessCDError(CommDlgExtendedError()); 
/*	 EnableWindow (hWndMain,TRUE);*/
		 GlobalUnlock(hColorsChunk);
	     LeaveBlockingWindow(hSaveBM);
         return (FALSE);                            
         
}

BOOL GetOpenFileCD (HWND hWnd,LPSTR Name,int lname, LPSTR lpInitDir)
{
   /*******************************************************************
   *                                                                  *
   *                    FILEOPEN/FILESAVE VARIABLES                   *
   *                                                                  *
   *******************************************************************/
   LPFOCHUNK lpFOChunk;    //Pointer to File Open block
   LPFSCHUNK lpFSChunk;    //Pointer to File Save block
   HANDLE hfoChunk;     //Handle to File Open block of memory
   HANDLE hfsChunk;     //Handle to File Save block of memory
   char	drive[32],dir[MAX_PATH],nam[MAX_PATH],ext[64];
   BOOL	st;
   WORD wSize;
   BOOL	Result, First = TRUE;  
//   char Dir[128], File[40];
   char	FName[MAX_PATH];
   char ofTemplateName[32];
   DWORD	ErCode;

//		_splitpath (lpInitDir,drive,dir,file,ext);
//		sprintf (Dir,"%s%s",drive,dir);
//		sprintf (File,"%s%s",file,ext);
//         _splitpath (Name,NULL,NULL,nam,ext);
//         sprintf (FName,"%s%s",nam,ext);
//         _fstrcpy (InitialFile,FName);
		 if (!*Name || (*Name && *LastChr (Name) == '\\'))
		 	*InitialFile = 0;
		 else
		 	_fullpath (InitialFile,Name,256);
         _fullpath (InitialDirectory,lpInitDir,256);
         wSize = sizeof(FOCHUNK);
         ghWnd = hWnd;
         if (!(lpFOChunk = (LPFOCHUNK)AllocAndLockMem(&hfoChunk, wSize)))
            return (FALSE); 
	     fsLen = FormatFilterString();  //Formats gszFilter with strings
  TryAgain:
         InitializeStruct(IDC_OPENFILE, (LPSTR)lpFOChunk); 
		 //lpFOChunk->of.FlagsEx = OFN_EX_NOPLACESBAR;
         if (Allow32bitFileNames)
         {  
//         	_fstrcpy (lpFOChunk->of.lpstrInitialDir,"\\everex\\c\\min14");
		 	st = GetFile32 (1,&(lpFOChunk->of),fsLen); 
		 	if (st < 0)
         		goto OldWay;
         }
         else 
         {
   OldWay:
	        _splitpath (InitialFile,NULL,NULL,nam,ext);
	        sprintf (FName,"%s%s",nam,ext);
	        _fstrcpy (InitialFile,FName);
			if (GetGlobalCVal ("%OPENFILETEMPLATE",ofTemplateName,0) && *ofTemplateName)
			{
				lpFOChunk->of.lpTemplateName = ofTemplateName;
				lpFOChunk->of.Flags = lpFOChunk->of.Flags |OFN_ENABLETEMPLATE;
			}
			SetIgnoreError (TRUE);
         	st = GetOpenFileName(&(lpFOChunk->of));  
 			SetIgnoreError (FALSE);
        }
         if (st)
         {
            HFILE hFile;
            OFSTRUCTGM OFStruct;
            
//            if (AllowCreate)
//            {
			if (!lname)
			{
                _fstrcpy (Name,lpFOChunk->of.lpstrFile);  
                GetLongPathName2 (Name,MAX_PATH);
            	Result = TRUE;
			}
			else
			{
				memmove (Name,lpFOChunk->of.lpstrFile,lname); 
            	Result = TRUE;
			}
/*            }
            else
            {
	            hFile = OpenFile(lpFOChunk->of.lpstrFile, &ofstruct, OF_EXIST| OF_SHARE_DENY_NONE);
	              // OF_SHARE_DENY_NONE | OF_SHARE_DENY_READ | OF_SHARE_DENY_WRITE | OF_SHARE_COMPAT);
	            if (hFile != -1)
	            {
	               _fstrcpy (Name,lpFOChunk->of.lpstrFile);
	               Result = TRUE;
	            }
	            else
	            {  GSSiMsgBox(hWnd, (LPSTR)gszFOFailure, gszAppName, MB_OK);
	               Result = FALSE;
	            }
            }*/
            //NOTE!!!  On a closed system (ie, not running on a network) this OpenFile
            //         call should NEVER fail.  This because we passed in the
            //         OFN_FILEMUSTEXIST flag to CD.  However, on a network system,
            //         there is a *very* small chance that between the time CD's checked
            //         for existance of the file and the time the call to OpenFile
            //         was made here, someone else on the network has deleted the file.
            //         MORAL: ALWAYS, ALWAYS, ALWAYS check the retrn code from your
            //         call to OpenFile() or _lopen.
         }
         else
         {  
         	ErCode = CommDlgExtendedError();  
     ShowErr:
         	if (ErCode == FNERR_INVALIDFILENAME && First)
         	{
         		First = FALSE;
         		*InitialFile=0;
         		goto TryAgain;
         	}
            ProcessCDError(ErCode);
            Result = FALSE;
         }
         GSSiGlobUlFree (&hfoChunk);  
         AllowCreate = FALSE;
         return (Result);
}

BOOL GetFolderName (HWND hWnd,LPSTR startDir,LPSTR outDir,LPSTR title)
{
	BROWSEINFO bi={0};
	PCIDLIST_ABSOLUTE pidList;

	bi.hwndOwner = hWnd;
	bi.lpszTitle = title;
	bi.pidlRoot = ILCreateFromPathA(startDir);
	bi.pszDisplayName = outDir;
	bi.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_EDITBOX;
//	bi.ulFlags = BIF_USENEWUI | BIF_DONTGOBELOWDOMAIN | BIF_NEWDIALOGSTYLE;

	if ((pidList = SHBrowseForFolder (&bi)))
	{
		SHGetPathFromIDList( pidList,outDir);
		return TRUE;
	}
	return FALSE;
}

BOOL GetSaveFileCD (HWND hWnd,LPSTR Name, LPSTR lpInitDir)
{
   /*******************************************************************
   *                                                                  *
   *                    FILEOPEN/FILESAVE VARIABLES                   *
   *                                                                  *
   *******************************************************************/
   LPFOCHUNK lpFOChunk;    //Pointer to File Open block
   LPFSCHUNK lpFSChunk;    //Pointer to File Save block
   HANDLE hfoChunk=0;     //Handle to File Open block of memory
   HANDLE hfsChunk=0;     //Handle to File Save block of memory

   WORD wSize;  
   short	st;
   BOOL	Result; 
   char	FName[MAX_PATH];
   char	nam[256],ext[128];
   DWORD	ErCode; 
   BOOL	First=TRUE; 
         
         _splitpath (Name,NULL,NULL,nam,ext);
         sprintf (FName,"%s%s",nam,ext);
		 if (!*Name || (*Name && *LastChr (Name) == '\\'))
		 	*InitialFile = 0;
		 else
		 	_fullpath (InitialFile,Name,256);
         _fullpath (InitialDirectory,lpInitDir,256);
         
         wSize = sizeof(FOCHUNK);
         ghWnd = hWnd;
         if (!(lpFSChunk = (LPFOCHUNK)AllocAndLockMem(&hfsChunk, wSize)))
            return (FALSE);
	     fsLen = FormatFilterString();  //Formats gszFilter with strings
  TryAgain:
         InitializeStruct(IDC_SAVEFILE, (LPSTR)lpFSChunk);
         if (Allow32bitFileNames)
         {
		 	st = GetFile32 (2,&(lpFSChunk->of),fsLen); 
		 	if (st < 0)
         		goto OldWay;
         }
         else 
         {
   OldWay:
	        _splitpath (InitialFile,NULL,NULL,nam,ext);
	        sprintf (FName,"%s%s",nam,ext);
	        _fstrcpy (InitialFile,FName);
         	st = GetSaveFileName(&(lpFSChunk->of));  
         }
         if (st)
         {
         	_fstrcpy (Name,(LPSTR)lpFSChunk->of.lpstrFile);
            GetLongPathName2 (Name,MAX_PATH);
         	Result = TRUE;
         }
         else
         {
         	ErCode = CommDlgExtendedError(); 
         	if (ErCode == FNERR_INVALIDFILENAME && First)
         	{
         		First = FALSE;
         		*InitialFile=0;
         		goto TryAgain;
         	}
            ProcessCDError(ErCode);
            Result = FALSE;
         }
         GlobalUnlock(hfsChunk);
         GSSiGlobFree (&hfsChunk);
         return (Result);
}


/**************************************************************************
*                                                                         *
*  Function:  FormatFilterString(void)                                    *
*                                                                         *
*   Purpose:  To initialize the gszFilter variable with strings from      *
*             the string table.  This method of initializing gszBuffer    *
*             is necessary to ensure that the strings are contiguous      *
*             in memory--which is what COMMDLG.DLL requires.              *
*                                                                         *
*   Retrns:   BOOL  TRUE if successful, FALSE if failure loading string   *
*                                                                         *
*  Comments:  The string loaded from the string table has some wild       *
*             character in it.  This wild character is then replaced      *
*             with NULL.  Note that the wild char can be any unique       *
*             character the developer chooses, and must be included       *
*             as the last character of the string.  A typical string      *
*             might look like "Write Files(*.WRI)|*.WRI|" where | is      *
*             the wild character in this case.  Implementing it this      *
*             way also ensures the string is doubly NULL terminated,      *
*             which is also a requirement of this lovely string.          *
*                                                                         *
*   History:  Date      Reason                                            *
*             --------  -----------------------------------               *
*                                                                         *
*             11/19/91  Created                                           *
*                                                                         *
**************************************************************************/

void SetFilterString (UINT Filter)
{
	FilterStringID = Filter;
	return;
}

short FormatFilterString (void)
{
   WORD wCtr, wStringLen;
   char chWildChar;
   
   if (FilterStringID == IDS_FILTERPLT)
   {
   		_fstrcpy (gszFilter,PltFilter1);
   		_fstrcat (gszFilter,PltFilter2);
	   	wStringLen = _fstrlen(gszFilter);
   }
   else if (FilterStringID)
   {
	   *gszFilter = 0;
	   if (!(wStringLen = LoadString(ghInst, FilterStringID, gszFilter, lngszFilter)))
	   {
	      ReportError(IDC_LOADSTRINGFAIL);
	      return (FALSE);
	   }  
   }
   else
   	wStringLen = _fstrlen(gszFilter);
   chWildChar = gszFilter[wStringLen - 1];    //Grab the wild character
   wCtr = 0;
   while (gszFilter[wCtr])
   {
      if (gszFilter[wCtr] == chWildChar)
         gszFilter[wCtr] = 0;
      wCtr++;
   }
   return wStringLen;
}

/**************************************************************************
*                                                                         *
*  Function:  InitializeStruct(WORD, LPSTR)                               *
*                                                                         *
*   Purpose:  To initialize a structure for the current common dialog.    *
*             This routine is called just before the common dialogs       *
*             API is called.                                              *
*                                                                         *
*   Returns:  void                                                        *
*                                                                         *
*  Comments:                                                              *
*                                                                         *
*   History:  Date      Reason                                            *
*             --------  -----------------------------------               *
*                                                                         *
*             10/01/91  Created                                           *
*                                                                         *
**************************************************************************/


void InitializeStruct (WORD wCommDlgType, LPSTR lpStruct)
{
   LPFOCHUNK lpFOChunk;
   LPFSCHUNK lpFSChunk;
   LPFINDREPLACECHUNK lpFChunk, lpFRChunk;
   LPCOLORSCHUNK lpColorsChunk;
   LPCHOOSEFONT lpFontChunk;
   LPPRINTDLG lpPrintChunk;
   WORD wCtr;
   HDC hDC;    
   static HANDLE	hDevMode=NULL;

   switch (wCommDlgType)
      {
   case IDC_OPENFILE:
      lpFOChunk = (LPFOCHUNK)lpStruct;
      *(lpFOChunk->szFile) = 0;
      *(lpFOChunk->szFileTitle) = 0;
      lpFOChunk->of.lStructSize = sizeof(OPENFILENAME);
      lpFOChunk->of.hwndOwner = (HWND)ghWnd;
      lpFOChunk->of.hInstance = (HANDLE)ghInst;;
      lpFOChunk->of.lpstrFilter = gszFilter;
      lpFOChunk->of.lpstrCustomFilter = customFilter;
      lpFOChunk->of.nMaxCustFilter = 255L;
      lpFOChunk->of.nFilterIndex = 1L;
      lpFOChunk->of.lpstrFile = lpFOChunk->szFile;
      lpFOChunk->of.lpstrFile = InitialFile;
      lpFOChunk->of.nMaxFile = (DWORD)sizeof(InitialFile);
      lpFOChunk->of.lpstrFileTitle = lpFOChunk->szFileTitle;
      lpFOChunk->of.nMaxFileTitle = MAXFILETITLELEN;
      lpFOChunk->of.lpstrInitialDir = InitialDirectory; 
	  strcpy(OFTitle2, OFTitle);
	  lpFOChunk->of.lpstrTitle = OFTitle2;
      *OFTitle = 0;
      lpFOChunk->of.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST |//OFN_EXPLORER |
                            OFN_FILEMUSTEXIST | ExtraOpenFlags;//|OFN_ENABLEHOOK;//|OFN_ENABLETEMPLATE;//
      ExtraOpenFlags  = 0;
      lpFOChunk->of.nFileOffset = 0;
      lpFOChunk->of.nFileExtension = 0;
      lpFOChunk->of.lpstrDefExt = (LPSTR)NULL;
      lpFOChunk->of.lCustData = 0L;
      lpFOChunk->of.lpfnHook = 0;//FileOpenHook;
      lpFOChunk->of.lpTemplateName = (LPSTR)NULL;
      break;

   case IDC_SAVEFILE:
      lpFSChunk = (LPFSCHUNK)lpStruct;
      GetWindowsDirectory(gszBuffer, lngszBuffer);
      *(lpFSChunk->szFile) = 0;
      lpFSChunk->of.lStructSize = sizeof(OPENFILENAME);
      lpFSChunk->of.hwndOwner = (HWND)ghWnd;
      lpFSChunk->of.hInstance = (HANDLE)ghInst;
	  lpFSChunk->of.lpstrFilter = gszFilter;
      lpFSChunk->of.lpstrCustomFilter = customFilter;
      lpFSChunk->of.nMaxCustFilter = 255;
	  lpFSChunk->of.nFilterIndex = 1;
	  lpFSChunk->of.lpstrFile = InitialFile;;
      lpFSChunk->of.nMaxFile = (DWORD)sizeof(lpFSChunk->szFile);
	  lpFSChunk->of.lpstrFileTitle = lpFSChunk->szFileTitle;
	  lpFSChunk->of.nMaxFileTitle =  MAXFILETITLELEN;
	  lpFSChunk->of.lpstrInitialDir = InitialDirectory;
	  strcpy(OFTitle2, OFTitle);
      lpFSChunk->of.lpstrTitle = OFTitle2; 
      *OFTitle = 0;
      if (OverWritePrompt)
		  lpFSChunk->of.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_EXPLORER | ExtraOpenFlags;
      else
      	lpFSChunk->of.Flags =  ExtraOpenFlags;  
      ExtraOpenFlags = 0;
      OverWritePrompt = TRUE;
      lpFSChunk->of.nFileOffset = 0;
      lpFSChunk->of.nFileExtension = 0;
      lpFSChunk->of.lpstrDefExt = (LPSTR)NULL;
      lpFSChunk->of.lCustData = 0L;
      lpFSChunk->of.lpfnHook = (LPOFNHOOKPROC)NULL;
      lpFSChunk->of.lpTemplateName = (LPSTR)NULL;
      break;

   case IDC_FIND:
      lpFChunk = (LPFINDREPLACECHUNK)lpStruct;
      *(lpFChunk->szFindWhat) = 0;
      *(lpFChunk->szReplaceWith) = 0;
      lpFChunk->fr.lStructSize = sizeof(FINDREPLACE);
      lpFChunk->fr.hwndOwner = ghWnd;
      lpFChunk->fr.hInstance = (HANDLE)ghInst;
      lpFChunk->fr.Flags = FR_ENABLEHOOK;
      lpFChunk->fr.lpstrFindWhat = (LPSTR)(lpFChunk->szFindWhat);
      lpFChunk->fr.lpstrReplaceWith = (LPSTR)(lpFChunk->szReplaceWith);
      lpFChunk->fr.wFindWhatLen = sizeof(lpFChunk->szFindWhat);
      lpFChunk->fr.lCustData = (LONG)CDN_FIND;
      lpFChunk->fr.lpfnHook = (LPOFNHOOKPROC)lpfnFindHook;
      lpFChunk->fr.lpTemplateName = (LPSTR)NULL;
      break;

   case IDC_FINDREPLACE:
      lpFRChunk = (LPFINDREPLACECHUNK)lpStruct;
      *(lpFRChunk->szFindWhat) = 0;
      *(lpFRChunk->szReplaceWith) = 0;
      lpFRChunk->fr.lStructSize = sizeof(FINDREPLACE);
      lpFRChunk->fr.hwndOwner = ghWnd;
      lpFRChunk->fr.hInstance = (HANDLE)ghInst;
      lpFRChunk->fr.Flags = FR_ENABLEHOOK;
      lpFRChunk->fr.lpstrFindWhat = (LPSTR)(lpFRChunk->szFindWhat);
      lpFRChunk->fr.wFindWhatLen = sizeof(lpFRChunk->szFindWhat);
      lpFRChunk->fr.lpstrReplaceWith = (LPSTR)(lpFRChunk->szReplaceWith);
      lpFRChunk->fr.wReplaceWithLen = sizeof(lpFRChunk->szReplaceWith);
      lpFRChunk->fr.lCustData = (LONG)CDN_FINDREPLACE;
      lpFRChunk->fr.lpfnHook = (LPOFNHOOKPROC)lpfnFindReplaceHook;
      lpFRChunk->fr.lpTemplateName = (LPSTR)NULL;
      break;

   case IDC_COLORS:
      lpColorsChunk = (LPCOLORSCHUNK)lpStruct;
      hDC = GetDC(ghWnd);
      lpColorsChunk->dwCustClrs[0] = GetBkColor(hDC);
      ReleaseDC(ghWnd, hDC);
      for (wCtr = 1; wCtr <= 15; wCtr++)
         lpColorsChunk->dwCustClrs[wCtr] = lpColorsChunk->dwCustClrs[0];
      lpColorsChunk->chsclr.lStructSize = sizeof(CHOOSECOLOR);
      lpColorsChunk->chsclr.hwndOwner = ghWnd;
      lpColorsChunk->chsclr.hInstance = (HWND)ghInst;
      lpColorsChunk->chsclr.rgbResult = (DWORD)(lpColorsChunk->dwColor);
      lpColorsChunk->chsclr.lpCustColors = (LPDWORD)(lpColorsChunk->dwCustClrs)
   ;
      lpColorsChunk->chsclr.Flags = CC_RGBINIT | CC_ENABLETEMPLATE | CC_ENABLEHOOK |CC_FULLOPEN;
      lpColorsChunk->chsclr.lCustData = 0L;
      lpColorsChunk->chsclr.lpfnHook = (LPOFNHOOKPROC)lpfnColorHook;
      lpColorsChunk->chsclr.lpTemplateName = "CHOOSECOLOR";
      break;

   case IDC_FONT:
      lpFontChunk = (LPCHOOSEFONT)lpStruct;
      lpFontChunk->lStructSize = sizeof(CHOOSEFONT);
      lpFontChunk->hwndOwner = ghWnd;

   //The hDC field will be initialized when we return--this avoids passing
   //the hDC in or getting the DC twice.
   //The LOGFONT field will also be initialized when we get back.
   //            lpFontChunk->hDC            = hDC;
   //            lpFontChunk->lpLogFont      = &lf;
      lpFontChunk->Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT | CF_ENABLETEMPLATE | CF_ENABLEHOOK | OtherFontOpts;
      OtherFontOpts = 0;
      lpFontChunk->rgbColors = RGB(0, 0, 0);
      lpFontChunk->lCustData = 0L;
      lpFontChunk->lpfnHook = (LPOFNHOOKPROC)FontHook;
      lpFontChunk->lpTemplateName = (LPSTR)"FNTSETUPDLGGM";
      lpFontChunk->hInstance = (HANDLE)ghInst;
      lpFontChunk->lpszStyle = (LPSTR)NULL;
      lpFontChunk->nFontType = SCREEN_FONTTYPE;
      lpFontChunk->nSizeMin = 0;
      lpFontChunk->nSizeMax = 0;
      break;
   case IDC_PRINTDLG:
      lpPrintChunk = (LPPRINTDLG)lpStruct;
      lpPrintChunk->lStructSize = sizeof(PRINTDLG);
      lpPrintChunk->hwndOwner = ghWnd; 
   	  lpPrintChunk->hDevMode = NULL;
      lpPrintChunk->hDevNames = (HANDLE)NULL;
      lpPrintChunk->hDC = (HDC)NULL;
      lpPrintChunk->Flags = PD_RETURNDC|PD_NOPAGENUMS|PD_NOSELECTION|PD_ENABLEPRINTTEMPLATE|PD_ENABLEPRINTHOOK/*|PD_ENABLESETUPTEMPLATE|PD_ENABLESETUPHOOK|PD_HIDEPRINTTOFILE*/;
//	  lpPrintChunk->Flags = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION | PD_ENABLEPRINTHOOK/*|PD_ENABLESETUPTEMPLATE|PD_ENABLESETUPHOOK|PD_HIDEPRINTTOFILE*/;
//	  lpPrintChunk->Flags = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION | PD_PRINTSETUP;
	  lpPrintChunk->nFromPage = 1;
      lpPrintChunk->nToPage = 1;
      lpPrintChunk->nMinPage = 1;
      lpPrintChunk->nMaxPage = 1;
      lpPrintChunk->nCopies = 1;
      lpPrintChunk->hInstance = ghInst;
      lpPrintChunk->lCustData = 0L;
	  lpPrintChunk->lpfnPrintHook = (LPOFNHOOKPROC)PrintSetupHook;
	  lpPrintChunk->lpfnSetupHook = (LPOFNHOOKPROC)PrintSetupHook;
	  lpPrintChunk->lpPrintTemplateName = "PRINTDLGGM";
	  lpPrintChunk->lpSetupTemplateName = "PRNSETUPDLGGM";
      lpPrintChunk->hPrintTemplate = (HANDLE)NULL;
      lpPrintChunk->hSetupTemplate = (HANDLE)NULL;
      break;

   default:
      break;
      }
   return;
}

/**************************************************************************
*                                                                         *
*  Function:  AllocAndLockMem(HANDLE *, WORD)                             *
*                                                                         *
*   Purpose:  To allocate and lock a chunk of memory for the CD           *
*             structure.                                                  *
*                                                                         *
*   Returns:  LPSTR                                                       *
*                                                                         *
*  Comments:                                                              *
*                                                                         *
*   History:  Date      Reason                                            *
*             --------  -----------------------------------               *
*                                                                         *
*             10/01/91  Created                                           *
*                                                                         *
**************************************************************************/


LPSTR AllocAndLockMem (HANDLE *hChunk, WORD wSize)
{
   LPSTR lpChunk;

   *hChunk = GSSiGlobAlloc ( 117,GHND, wSize);
   if (*hChunk)
   {
      lpChunk = GlobalLock(*hChunk);
      if (!lpChunk)
      {
         GSSiGlobFree (hChunk);
         ReportError(IDC_LOCKFAIL);
         lpChunk = NULL;
      }
   }
   else
   {
      ReportError(IDC_ALLOCFAIL);
      lpChunk = NULL;
   }
   return (lpChunk);
}

/**************************************************************************
*                                                                         *
*  Function:  MainWndProc(HWND, UINT, WPARAM, LPARAM)			  *
*                                                                         *
*   Purpose:  Standard main window procedure to process messages          *
*             for the main window.                                        *
*                                                                         *
*   Returns:  void                                                        *
*                                                                         *
*  Comments:  Note the logic for processing of CD Help messages and       *
*             Find/FindReplace notification messages.  This is done       *
*             under the default processing of the main switch(message)    *
*             statement.                                                  *
*                                                                         *
*   History:  Date      Reason                                            *
*             --------  -----------------------------------               *
*                                                                         *
*             10/01/91  Created                                           *
*                                                                         *
**************************************************************************/

void vdef ()
{
   /*******************************************************************
   *                                                                  *
   *                    FILEOPEN/FILESAVE VARIABLES                   *
   *                                                                  *
   *******************************************************************/
   LPFOCHUNK lpFOChunk;    //Pointer to File Open block
   LPFSCHUNK lpFSChunk;    //Pointer to File Save block
   HANDLE hfoChunk;     //Handle to File Open block of memory
   HANDLE hfsChunk;     //Handle to File Save block of memory

   /*******************************************************************
   *                                                                  *
   *                             FONTS VARIABLES                      *
   *                                                                  *
   *******************************************************************/
   #define DESIREDPOINTSIZE 12

   //   HDC            hDC;
   int iLogPixsY;
   LOGFONT lf;
   LPCHOOSEFONT lpFontChunk;
   HANDLE hFontChunk;
   HFONT hOldFont;


   /********************************************************************
   *                                                                   *
   *                             FIND/FINDREPLACE VARIABLES            *
   *                                                                   *
   ********************************************************************/
   LPFINDREPLACECHUNK lpFChunk, lpFRChunk;
   static HANDLE hFChunk, hFRChunk;
   BOOL bFCase, bFReverse, bFWord;
   BOOL bFRCase, bFRWord;
   LPFINDREPLACE lpF;
   DWORD dwFFlags;
   char szFDirection[MAXFINDDIRECTIONLEN];
   char szFCase[MAXFINDCASELEN], szFRCase[MAXFINDCASELEN];
   char szFWord[MAXFINDWORDLEN], szFRWord[MAXFINDWORDLEN];
   static HDC hDC;


   /*******************************************************************
   *                                                                  *
   *                             PRINTDLG VARIABLES                   *
   *                                                                  *
   *******************************************************************/
   LPPRINTDLG lpPDChunk;
   short xPage, yPage;

/*   switch (message)
      {
   case WM_CREATE:
      wFRMsg = RegisterWindowMessage((LPSTR)FINDMSGSTRING);
      wHelpMsg = RegisterWindowMessage((LPSTR)HELPMSGSTRING);
      lpfnFileOpenHook = (FARHOOK)MakeProcInstance(FileOpenHook, ghInst);
      lpfnFindHook = (FARHOOK)MakeProcInstance(FindHook, ghInst);
      lpfnFindReplaceHook = (FARHOOK)MakeProcInstance(FindReplaceHook, ghInst);
      if (lpfnFileOpenHook == NULL || lpfnFindHook == NULL ||
          lpfnFindReplaceHook == NULL)
         retrn (FALSE);

      //Are we in monochrome land????
      hDC = GetDC(NULL);
      gbMonochrome = (2 == GetDeviceCaps(hDC, NUMCOLORS));  // Monochrome!
      ReleaseDC(NULL, hDC);

      //Load brush for painting GetOpenFileName dlg box
      if (!gbMonochrome)
      {
         HBITMAP hTempBitmap;

         hTempBitmap = LoadBitmap(ghInst, MAKEINTRESOURCE(1));
         if (hTempBitmap)
         {
            ghDlgBrush = CreatePatternBrush(hTempBitmap);
            DeleteObject(hTempBitmap);
         }
      }
      break;

   case WM_ERASEBKGND:
      if (ghBkgndBrush)
      {
         GetClientRect(ghWnd, &rc);
         FillRect((HDC)wParam, &rc, ghBkgndBrush);
         break;
      }
      else
         retrn (DefWindowProc(hWnd, message, wParam, lParam));

   case WM_PAINT:
      if (ghSelectedFont)  //Let's output the string
      {
         hDC = BeginPaint(hWnd, &ps);
         hOldFont = SelectObject(hDC, ghSelectedFont);
         SetBkMode(hDC, TRANSPARENT);
         SetTextColor(hDC, (COLORREF)dwFontColor);
         TextOut(hDC, 10, 10, gszFontMsg, 20);
         SelectObject(hDC, hOldFont);
         EndPaint(hWnd, &ps);
         break;
      }
      else
         retrn (DefWindowProc(hWnd, message, wParam, lParam));

   case WM_COMMAND:
      switch (wParam)
         {
      case IDM_FILEOPEN:
         wSize = sizeof(FOCHUNK);
         if (!(lpFOChunk = (LPFOCHUNK)AllocAndLockMem(&hfoChunk, wSize)))
            break;
         InitializeStruct(IDC_OPENFILE, (LPSTR)lpFOChunk);
         if (GetOpenFileName(&(lpFOChunk->of)))
         {
            HANDLE hFile;
            OFSTRUCTGM OFStruct;

            hFile = OpenFile(lpFOChunk->of.lpstrFile, &ofstruct, OF_EXIST);
            if (hFile != -1)
               GSSiMsgBox(hWnd, (LPSTR)gszFOSuccess, gszAppName, MB_OK);
            else
               GSSiMsgBox(hWnd, (LPSTR)gszFOFailure, gszAppName, MB_OK);

            //NOTE!!!  On a closed system (ie, not running on a network) this OpenFile
            //         call should NEVER fail.  This because we passed in the
            //         OFN_FILEMUSTEXIST flag to CD.  However, on a network system,
            //         there is a *very* small chance that between the time CD's checked
            //         for existance of the file and the time the call to OpenFile
            //         was made here, someone else on the network has deleted the file.
            //         MORAL: ALWAYS, ALWAYS, ALWAYS check the retrn code from your
            //         call to OpenFile() or _lopen.
         }
         else
         {
            ProcessCDError(CommDlgExtendedError());
         }
         GlobalUnlock(hfoChunk);
         GlobalFree(hfoChunk);
         break;

      case IDM_FILESAVE:
         wSize = sizeof(FOCHUNK);
         if (!(lpFSChunk = (LPFOCHUNK)AllocAndLockMem(&hfsChunk, wSize)))
            break;
         InitializeStruct(IDC_SAVEFILE, (LPSTR)lpFSChunk);
         if (GetSaveFileName(&(lpFSChunk->of)))
         {
            wsprintf(gszBuffer, "Save file as: %s", (LPSTR)lpFSChunk->of.
                     lpstrFile);
            GSSiMsgBox(hWnd, (LPSTR)gszBuffer, gszAppName, MB_OK);
         }
         else
         {
            ProcessCDError(CommDlgExtendedError());
         }
         GlobalUnlock(hfsChunk);
         GlobalFree(hfsChunk);
         break;

      case IDM_FIND:
         EnableMenuItem(GetMenu(ghWnd), IDM_FIND, MF_GRAYED);
         wSize = sizeof(FINDREPLACECHUNK);
         if (!(lpFChunk = (LPFINDREPLACECHUNK)AllocAndLockMem(&hFChunk, wSize))
         )
            break;
         InitializeStruct(IDC_FIND, (LPSTR)lpFChunk);
         if (!(ghFFRDlg = FindText(&(lpFChunk->fr))))
         {
            GlobalUnlock(hFChunk);
            GlobalFree(hFChunk);
            ghFFRDlg = NULL;
            EnableMenuItem(GetMenu(ghWnd), IDM_FIND, MF_ENABLED);
            ProcessCDError(CommDlgExtendedError());
         }

         //Note:  DO NOT call GlobalUnlock() and GlobalFree() here since this is
         //       a modeless dialog.  See default processing below.
         break;

      case IDM_FINDANDREPLACE:
         EnableMenuItem(GetMenu(ghWnd), IDM_FINDANDREPLACE, MF_GRAYED);
         wSize = sizeof(FINDREPLACECHUNK);
         if (!(lpFRChunk = (LPFINDREPLACECHUNK)AllocAndLockMem(&hFRChunk, wSize
                                                               )))
            break;
         InitializeStruct(IDC_FINDREPLACE, (LPSTR)lpFRChunk);
         if (!(ghFFRDlg = ReplaceText(&(lpFRChunk->fr))))
         {
            GlobalUnlock(hFRChunk);
            GlobalFree(hFRChunk);
            ghFFRDlg = NULL;
            EnableMenuItem(GetMenu(ghWnd), IDM_FINDANDREPLACE, MF_ENABLED);
            ProcessCDError(CommDlgExtendedError());
         }

         //Note:  DO NOT call GlobalUnlock() and GlobalFree() here since this is
         //       a modeless dialog.  See default processing below.
         break;

      case IDM_COLORS:
         if (!lpColorsChunk)   //ie, haven't called colors yet
         //so let's initialize everything
         {
            wSize = sizeof(COLORSCHUNK);
            if (!(lpColorsChunk = (LPCOLORSCHUNK)AllocAndLockMem(&hColorsChunk,
                                                                 wSize)))
               break;
            InitializeStruct(IDC_COLORS, (LPSTR)lpColorsChunk);
         }
         if (ChooseColor(&(lpColorsChunk->chsclr)))
         {
            if (ghBkgndBrush)
               DeleteObject(ghBkgndBrush);
            ghBkgndBrush = CreateSolidBrush(lpColorsChunk->chsclr.rgbResult);
            InvalidateRect(ghWnd, NULL, TRUE);
         }
         else
         {
            ProcessCDError(CommDlgExtendedError());
         }
         break;

      case IDM_FONTS:
         wSize = sizeof(CHOOSEFONT);
         if (!(lpFontChunk = (LPCHOOSEFONT)AllocAndLockMem(&hFontChunk, wSize))
         )
            break;
         InitializeStruct(IDC_FONT, (LPSTR)lpFontChunk);

      //Because we are only getting screen fonts, we can set hDC = NULL
      //If printer fonts are desired, a handle to a printer DC must be passed in
         lpFontChunk->hDC = NULL;

         //Now let's initialize the logfont structure.
         hDC = GetDC(ghWnd);
         iLogPixsY = GetDeviceCaps(hDC, LOGPIXELSY);
         ReleaseDC(ghWnd, hDC);
         lf.lfHeight = -1 * (iLogPixsY * DESIREDPOINTSIZE / 72);
         lf.lfWidth = 0;
         lf.lfEscapement = 0;
         lf.lfOrientation = 0;
         lf.lfWeight = 400;
         lf.lfItalic = 0;
         lf.lfUnderline = 0;
         lf.lfStrikeOut = 0;
         lf.lfCharSet = ANSI_CHARSET;
         lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
         lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
         lf.lfQuality = DEFAULT_QUALITY;
         lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
         lstrcpy(lf.lfFaceName, "Modern");
         lpFontChunk->lpLogFont = &lf;
         if (ChooseFont(lpFontChunk)) //Now let's create the selected font!
         {
            if (ghSelectedFont)
               DeleteObject(ghSelectedFont);
            ghSelectedFont = CreateFontIndirect((LPLOGFONT)(lpFontChunk->
                                                lpLogFont));
            dwFontColor = lpFontChunk->rgbColors;
            InvalidateRect(ghWnd, NULL, TRUE);
         }
         else
         {
            ProcessCDError(CommDlgExtendedError());
         }
         GlobalUnlock(hFontChunk);
         GlobalFree(hFontChunk);
         break;

      case IDM_PRINT:
         wSize = sizeof(PRINTDLG);
         if (!(lpPDChunk = (LPPRINTDLG)AllocAndLockMem(&hPDChunk, wSize)))
            break;
         InitializeStruct(IDC_PRINTDLG, (LPSTR)lpPDChunk);
         if (PrintDlg(lpPDChunk) != 0)
         {
            BOOL bError;
            DLGPROC lpfnAbortProc, lpfnPrintDlgProc;

            gbUserAbort = FALSE;
            bError = FALSE;
            lpfnPrintDlgProc = MakeProcInstance(PrintDlgProc, ghInst);
            ghPrintingDlg = CreateDialog(ghInst, "PRINTING", ghWnd,
                                         lpfnPrintDlgProc);
            lpfnAbortProc = MakeProcInstance(AbortProc, ghInst);
            Escape(lpPDChunk->hDC, SETABORTPROC, 0, (LPSTR)lpfnAbortProc, NULL)
            ;
            if (Escape(lpPDChunk->hDC, STARTDOC, 8, gszTestDoc, NULL) > 0)
            {
               xPage = GetDeviceCaps(lpPDChunk->hDC, HORZRES);
               yPage = GetDeviceCaps(lpPDChunk->hDC, VERTRES);
               Rectangle(lpPDChunk->hDC, 0, 0, xPage, yPage);
               MoveTo(lpPDChunk->hDC, 0, 0);
               LineTo(lpPDChunk->hDC, xPage, yPage);
               MoveTo(lpPDChunk->hDC, 0, yPage);
               LineTo(lpPDChunk->hDC, xPage, 0);
               dwLen = GetTextExtent(lpPDChunk->hDC, gszBuffer, 26);
               TextOut(lpPDChunk->hDC, (xPage / 2) - (LOWORD(dwLen) / 2), 50,
                       gszPrintMsg, 26);
               Escape(lpPDChunk->hDC, NEWFRAME, 0, NULL, NULL);
               Escape(lpPDChunk->hDC, ENDDOC, 0, NULL, NULL);
               DeleteDC(lpPDChunk->hDC);
               if (lpPDChunk->hDevMode)
                  GlobalFree(lpPDChunk->hDevMode);
               if (lpPDChunk->hDevNames)
                  GlobalFree(lpPDChunk->hDevNames);
               DeleteDC(lpPDChunk->hDC);
               if (lpPDChunk->hDevMode)
                  GlobalFree(lpPDChunk->hDevMode);
               if (lpPDChunk->hDevNames)
                  GlobalFree(lpPDChunk->hDevNames);
         break;

      case IDM_ABOUT:
         lpProcAbout = MakeProcInstance(About, ghInst);
         DialogBox(ghInst, MAKEINTRESOURCE(1), hWnd, lpProcAbout);
         FreeProcInstance(lpProcAbout);
         break;

      case IDM_EXIT:
         PostMessage(hWnd, WM_CLOSE, 0, 0L);
         break;

      default:
         retrn (DefWindowProc(hWnd, message, wParam, lParam));
         }
      break;

   case WM_DESTROY:
      if (ghDlgBrush)
         DeleteObject(ghDlgBrush);
      if (ghBkgndBrush)
         DeleteObject(ghBkgndBrush);
      if (ghSelectedFont)
         DeleteObject(ghSelectedFont);
      if (lpColorsChunk)  //Clean up memory for colors dialog
      {
         GlobalUnlock(hColorsChunk);
         GlobalFree(hColorsChunk);
      }
      PostQuitMessage(0);
      break;

   default:

   //Let's keep the logic readable and put it in the switch statement.
   //Assign some user defined constants here so a switch can be used.
      if (message == wHelpMsg)
         wMyMessage = IDC_HELPMSG;
      if (message == wFRMsg)
         wMyMessage = IDC_FINDREPLACEMSG;
      switch (wMyMessage)
         {
      case IDC_HELPMSG:
         WinHelp(ghWnd, gszWin31wh, HELP_INDEX, 0L);
         break;

      case IDC_FINDREPLACEMSG:
         lpF = (LPFINDREPLACE)lParam;
         dwFFlags = lpF->Flags;
         if (dwFFlags & FR_DIALOGTERM)
         {
            if (lpF->lCustData == CDN_FIND)
            {
               GlobalUnlock(hFChunk);
               GlobalFree(hFChunk);
               ghFFRDlg = NULL;
               EnableMenuItem(GetMenu(ghWnd), IDM_FIND, MF_ENABLED);
            }
            else if (lpF->lCustData == CDN_FINDREPLACE)
            {
               GlobalUnlock(hFRChunk);
               GlobalFree(hFRChunk);
               ghFFRDlg = NULL;
               EnableMenuItem(GetMenu(ghWnd), IDM_FINDANDREPLACE, MF_ENABLED);
            }
            break;
         }
         if (lpF->lCustData == CDN_FIND)
         {
            bFReverse = (dwFFlags & FR_DOWN ? FALSE : TRUE);
            bFCase = (dwFFlags & FR_MATCHCASE ? TRUE : FALSE);
            bFWord = (dwFFlags & FR_WHOLEWORD ? TRUE : FALSE);
            if (bFReverse)
               iResult = LoadString(ghInst, IDS_SEARCHUP, szFDirection, sizeof(
                                    szFDirection));
            else
               iResult = LoadString(ghInst, IDS_SEARCHDOWN, szFDirection,
                                    sizeof(szFDirection));
            if (!iResult)
            {
               ReportError(IDC_LOADSTRINGFAIL);
               break;
            }
            if (bFCase)
               iResult = LoadString(ghInst, IDS_CASESENSITIVE, szFCase, sizeof(
                                    szFCase));
            else
               iResult = LoadString(ghInst, IDS_IGNORECASE, szFCase, sizeof(
                                    szFCase));
            if (!iResult)
            {
               ReportError(IDC_LOADSTRINGFAIL);
               break;
            }
            if (bFWord)
               iResult = LoadString(ghInst, IDS_WHOLEWORD, szFWord, sizeof(
                                    szFWord));
            else
               iResult = LoadString(ghInst, IDS_WHOLEANDSUB, szFWord, sizeof(
                                    szFWord));
            if (!iResult)
            {
               ReportError(IDC_LOADSTRINGFAIL);
               break;
            }
            wsprintf(gszBuffer, "Find %s, %s, %s, %s", (LPSTR)(lpF->
                     lpstrFindWhat), (LPSTR)szFDirection, (LPSTR)szFWord, (
                     LPSTR)szFCase);
            GSSiMsgBox(ghFFRDlg, gszBuffer, gszAppName, MB_OK);
            break;
         }
         if (lpF->lCustData == CDN_FINDREPLACE)
         {
            bFRCase = (dwFFlags & FR_MATCHCASE ? TRUE : FALSE);
            bFRWord = (dwFFlags & FR_WHOLEWORD ? TRUE : FALSE);
            if (bFRCase)
               iResult = LoadString(ghInst, IDS_CASESENSITIVE, szFRCase, sizeof
                                    (szFRCase));
            else
               iResult = LoadString(ghInst, IDS_IGNORECASE, szFRCase, sizeof(
                                    szFRCase));
            if (!iResult)
            {
               ReportError(IDC_LOADSTRINGFAIL);
               break;
            }
            if (bFRWord)
               iResult = LoadString(ghInst, IDS_WHOLEWORD, szFRWord, sizeof(
                                    szFRWord));
            else
               iResult = LoadString(ghInst, IDS_WHOLEANDSUB, szFRWord, sizeof(
                                    szFRWord));
            if (!iResult)
            {
               ReportError(IDC_LOADSTRINGFAIL);
               break;
            }
            wsprintf(gszBuffer, "Find %s, Replace with %s, %s, %s", (LPSTR)(lpF
                     ->lpstrFindWhat), (LPSTR)(lpF->lpstrReplaceWith), (LPSTR)
                     szFRWord, (LPSTR)szFRCase);
            GSSiMsgBox(ghFFRDlg, gszBuffer, gszAppName, MB_OK);
            break;
         }

      default:
         retrn (DefWindowProc(hWnd, message, wParam, lParam));
         }
      break;
      }
   retrn (NULL); */
} /* end MainWndProc */

BOOL CALLBACK EnumChildProc2(HWND hCtrl,LONG lParam)
    {
    char        str[256];
    long	lUserData;
    POINT	Loc; 
    RECT	Rect;
	WINDOWPLACEMENT wp;
	UINT	CntlID;
    
    GetWindowText (hCtrl,str,200);  
    GetWindowRect (hCtrl,&Rect);
    //if (((Rect.right - Rect.left) < 60) && ((Rect.bottom - Rect.top) < 30))
	GetWindowPlacement(hCtrl,&wp);
	CntlID = GetDlgCtrlID(hCtrl);
	if (CntlID == 1148)
		NameWnd = hCtrl;//1148
	if (!stricmp (str,"&Open"))
		OpenWnd = hCtrl;
     return (TRUE);
    }
BOOL CALLBACK EnumWndProc2(HWND hCtrl,LONG lParam)
    {
    char        str[130];
    long	lUserData; 
    HWND	hPar;
    
    GetWindowText (hCtrl,str,128); 
    hPar = GetParent (hCtrl); 
    if (!stricmp (str,"License Failure"))
//    if (!stricmp (str,"Open Archive"))
    {
		EnumChildWindows (hCtrl,EnumChildProc2, 0L);
    	return FALSE;
    }
    return (TRUE);
    }




/**************************************************************************
*                                                                         *
*  Function:  FileOpenHook(HWND, UINT, WPARAM, LPARAM)			  *
*                                                                         *
*   Purpose:  This function "hooks" the CD GetOpenFileName() procedure    *
*             and allows you to process any messages you want.  In this   *
*             example, the WM_CTLCOLOR message is being processed to      *
*             give the background of the CD a cool color.                 *
*                                                                         *
*   Returns:  BOOL                                                        *
*                                                                         *
*  Comments:  This function must be exported and must have an instance    *
*             thunk created for it (ie, call MakeProcInstance; see        *
*             WM_CREATE processing in the main window procedure above)    *
*                                                                         *
*   History:  Date      Reason                                            *
*             --------  -----------------------------------               *
*                                                                         *
*             10/01/91  Created                                           *
*                                                                         *
**************************************************************************/


UINT CALLBACK  FileOpenHook (HWND hDlg, UINT message, WPARAM wParam, LPARAM
                              lParam)
{ 
	static	HBITMAP	hBM;
	int		ii;
	char	str[256];

	//return (FALSE);
   switch (message)
      {
   case WM_INITDIALOG:
	   {
		   //hBM = LoadBitmap (hInst,"PROJECTWISE");
	   		//SendDlgItemMessage (hDlg,IDC_PROJECTWISE,BM_SETIMAGE,IMAGE_BITMAP,(LPARAM)hBM);
	   }
      return (TRUE);

	  case WM_DESTROY:
		  GSSiDeleteObject (&hBM);
		  break;
   case WM_CTLCOLORMSGBOX:
   //case WM_CTLCOLOR:
      if (gbMonochrome || !ghDlgBrush)
         return (FALSE);
      switch (HIWORD(lParam))
         {
      case CTLCOLOR_LISTBOX:   //Don't mess with the listboxes
         return (FALSE);

      case CTLCOLOR_DLG:
         UnrealizeObject(ghDlgBrush);
         break;

      default:
         break;
         }
      SelectObject((HDC)wParam, ghDlgBrush);
      if (HIWORD(lParam) == CTLCOLOR_DLG)
         SetBrushOrgEx((HDC)wParam, 0, 0,0);
      SetBkMode((HDC)wParam, TRANSPARENT);
      SetTextColor((HDC)wParam, RGB(0, 0, 0));
      return ((UINT)ghDlgBrush);

   case WM_COMMAND:
	   {
		   DWORD ic = LOWORD(wParam);
      switch(LOWORD(wParam))
         {
	  case IDC_PROJECTWISE:
		  EnumChildWindows (GetParent (hDlg),EnumChildProc2, 0L);
		  ii=1;
    	  //PostMessage(hDlg, WM_CLOSE, 0, 0L);
		  GetGlobalCVal ("[%PROJECTWISEDIR]",str,"http://cmean407/pw");
		  SendMessage (NameWnd,WM_SETTEXT,0,(LPARAM)str);
		   {   
    			char	mess[144];
				POINT	Point, Loc;
				RECT	Rect;
    			
     			GetCursorPos (&Point); 
	  			SetFocus (OpenWnd);  
    			GetWindowRect (OpenWnd,&Rect);
    			Loc = RectMid (&Rect);   
    			SetCursorPos (Loc.x,Loc.y); 
    			Loc.x = (Rect.right-Rect.left)/2;
    			Loc.y = (Rect.bottom-Rect.top)/2;
				PostMessage (OpenWnd,WM_LBUTTONDOWN,1,MAKELONG(Loc.x,Loc.y)); 
				PostMessage (OpenWnd,WM_LBUTTONUP,0,MAKELONG(Loc.x,Loc.y));  
				SetCursorPos (Point.x,Point.y);
				Wait2 (500);
				*str = 0;
				SendMessage (NameWnd,WM_SETTEXT,0,(LPARAM)str);
			}
		  break;
      case stc1:
		  ii=1;
         break;
	  case edt1:
		  GetWindowText (GetDlgItem (hDlg,edt1),str,255);
		  ii=1;
		  break;
	  case cmb1:
		  ii=1;
		  break;
      default:
         break;
         }
	   }
      break;

   default:
      break;
      }
   return (FALSE);
}  

void GetPatternOpt (HWND hDlg,LPPATBYTE pPatByte)
{   
	UINT	Control[9]={IDC_XHATCH,IDC_50PCT,IDC_25PCT,IDC_12PCT,IDC_XHATCH2,IDC_50PCT2,IDC_25PCT2,IDC_12PCT2,IDC_0PCT};
	UINT	i=9;
	
	_fmemset (pPatByte,0,sizeof(PATBYTE));
	while (i--)
		if (SendDlgItemMessage (hDlg,Control[i],BM_GETCHECK,0,0))
			goto Next;
	return;
Next:
	if (i < 4)
		pPatByte->Pattern = i+1;
	else
	{
		pPatByte->Pattern = i - 3;
		pPatByte->Transparent = 1;
	}
	return;
}    

void SetPatternOpt (HWND hDlg,LPPATBYTE pPatByte)
{   
	UINT	Control[9]={IDC_XHATCH,IDC_50PCT,IDC_25PCT,IDC_12PCT,IDC_XHATCH2,IDC_50PCT2,IDC_25PCT2,IDC_12PCT2,IDC_0PCT};
	UINT	i=9;
	
	if (!pPatByte->Pattern)
		return;
	i = pPatByte->Pattern + pPatByte->Transparent * 4 - 1;
	SendDlgItemMessage (hDlg,Control[i],BM_SETCHECK,TRUE,0);
		SendDlgItemMessage (hDlg,IDC_SOLID,BM_SETCHECK,FALSE,0);
	return;
}    

HBRUSH CreatePatBrush (HDC hDC, COLORREF color, LPPATBYTE pPatByte)
{
	HBITMAP hbmp;
	HBRUSH	hbrush; 
	
	if (!pPatByte->Pattern)
		hbrush = CreateSolidBrush (color);
/*	else if (pPatByte->Pattern == 1)
	{   LOGBRUSH    NDB;
		    
        NDB.lbStyle = BS_HATCHED;
        NDB.lbColor = color;
        NDB.lbHatch = HS_DIAGCROSS;
        hbrush =  CreateBrushIndirect(&NDB);
	    SetBkColor (hDC,RGB(192,192,192));
    } */
	else if (!pPatByte->Transparent)
	{
		hbmp = LoadBitmap(ghInst, MAKEINTRESOURCE(PatBMP[pPatByte->Pattern-1]));  
		hbrush = CreatePatternBrush(hbmp); 
		DeleteObject (hbmp); 
		if (hDC)
		{
		    SetTextColor (hDC,color);     
		    SetBkColor (hDC,RGB(192,192,192));
		}
	}
	else 
	{
		if (pPatByte->Pattern == 1)
			hbmp = LoadBitmap(ghInst, MAKEINTRESOURCE(IDB_94PCT)); 
		else
			hbmp = LoadBitmap(ghInst, MAKEINTRESOURCE(PatBMP[pPatByte->Pattern-1]));  
		hbrush = CreatePatternBrush(hbmp); 
		DeleteObject (hbmp); 
		if (hDC)
		{
		    SetTextColor (hDC,color);     
		    SetBkColor (hDC,RGB(255,255,255));
			if (GetROP2 (hDC) != R2_NOT)
				SetROP2(hDC,R2_MASKPEN); 
		}
	}
	return hbrush;
}

/**************************************************************************
*                                                                         *
*  Function:  FindHook(HWND, UINT, WPARAM, LPARAM)			  *
*                                                                         *
*   Purpose:  This function "hooks" the CD FindText() procedure           *
*             and allows you to process and messages you want.            *
*             The ONLY reason we are hooking the FindText() CD is to      *
*             keep track of ghFFRDlg.  This allows proper processing      *
*             of IsDialogMessage() in the main message loop.  If your     *
*             app will *not* allow both the FindText() and ReplaceText()  *
*             CD's to be up at the same time, then hooking will not       *
*             be necessary.                                               *
*                                                                         *
*   Returns:  BOOL                                                        *
*                                                                         *
*  Comments:  This function must be exported and must have an instance    *
*             thunk created for it (ie, call MakeProcInstance; see        *
*             WM_CREATE processing in the main window procedure above)    *
*                                                                         *
*   History:  Date      Reason                                            *
*             --------  -----------------------------------               *
*                                                                         *
*             10/01/91  Created                                           *
*                                                                         *
**************************************************************************/


UINT CALLBACK  FindHook (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
      {
   case WM_INITDIALOG:
      return (TRUE);

   case WM_ACTIVATE:
      ghFFRDlg = hDlg;
      break;

   default:
      break;
      }
   return (FALSE);
}

UINT CALLBACK  FontHook (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	COLORREF	Color=0;
   switch (message)
      {
   case WM_INITDIALOG:
	   EnableWindow (GetDlgItem (hDlg,IDC_SHADOWCOLOR),AllowShadowColor);
      return (TRUE);

   case WM_ACTIVATE:
      ghFFRDlg = hDlg;
      break;
   case WM_COMMAND:
	   {
		   DWORD ic = LOWORD(wParam);
      switch(LOWORD(wParam))
         {
	  case IDC_FONTCOLOR:
		  Color = FontColor;
			if (GetColor (hDlg,&Color))
				FontColor = Color;
			break;
	  case IDC_SHADOWCOLOR:
		  Color = ShadowColor;
			if (GetColor (hDlg,&Color))
				ShadowColor = Color;
		  break;
 
      default:
         break;
         }
	   }
      break;


   default:
      break;
      }
   return (FALSE);
}

/**************************************************************************
*                                                                         *
*  Function:  FindReplaceHook(HWND, UINT, WPARAM, LPARAM)		  *
*                                                                         *
*   Purpose:  This function "hooks" the CD ReplaceText() procedure        *
*             and allows you to process and messages you want.            *
*             The ONLY reason we are hooking the FindText() CD is to      *
*             keep track of ghFFRDlg.  This allows proper processing      *
*             of IsDialogMessage() in the main message loop.  If your     *
*             app will *not* allow both the FindText() and ReplaceText()  *
*             CD's to be up at the same time, then hooking will not       *
*             be necessary.                                               *
*                                                                         *
*   Returns:  BOOL                                                        *
*                                                                         *
*  Comments:  This function must be exported and must have an instance    *
*             thunk created for it (ie, call MakeProcInstance; see        *
*             WM_CREATE processing in the main window procedure above)    *
*                                                                         *
*   History:  Date      Reason                                            *
*             --------  -----------------------------------               *
*                                                                         *
*             10/01/91  Created                                           *
*                                                                         *
**************************************************************************/


UINT CALLBACK  FindReplaceHook (HWND hDlg, UINT message, WPARAM wParam, LPARAM
                                 lParam)
{
   switch (message)
      {
   case WM_INITDIALOG:
      return (TRUE);

   case WM_ACTIVATE:
      ghFFRDlg = hDlg;
      break;

   default:
      break;
      }
   return (FALSE);
}

UINT CALLBACK  PrintSetupHook (HWND hDlg, UINT message, WPARAM wParam, LPARAM
                                 lParam)
{   char	str[256], GlobName[32], str2[64];
	UINT	WPCntls[3]={IDC_PRINTWPNONE,IDC_PRINTWPDETAILS,IDC_PRINTWPSUM};
	short	i, BRtn,Choice,ii; 
	BOOL	Err;
	LPSTR	pVPIF; 
	char	VirtualPrinterList[256];
	static	LPPRINTDLG lpPDChunk;	

	
//	if ((BRtn = DIALOGSTYLEMsgProc (hDlg,message, wParam, lParam)))
//		return (BRtn);
   switch (message)
      {
   case WM_INITDIALOG: 
	  lpPDChunk = (LPPRINTDLG)lParam;
   	  PrintWasCanceled = FALSE;
   	  for (i=0;i<3;i++)
	  	SendDlgItemMessage (hDlg,WPCntls[i],BM_SETCHECK,FALSE,0);   
      SendDlgItemMessage (hDlg,IDC_PRINTPREVIEW,BM_SETCHECK,CreatePrintPreview,0);  
      i = GetGlobalLVal2 ("[%PRINTWPOPT]",0);  
	  SendDlgItemMessage (hDlg,WPCntls[i],BM_SETCHECK,TRUE,0);    
	  GetGlobalCVal ("[%PRINTREPORTTEXT]",str,"Report Options");  
	  GetGlobalCVal ("[%APPID]",str2,"GeoMaster");  
      GetCurVal (str,sizeof(str),IDS_FILEIMAGE);   
      SetDlgItemText (hDlg,IDC_VPOUTPUTFILE,str);
	  SetDlgItemInt (hDlg,edt3,max(1,lpPDChunk->nCopies),FALSE); 
	  if (!_fstrnicmp (MODULEIDSTRING,"23",2))
		  ShowWindow(GetDlgItem(hDlg,IDC_PRINTPREVIEW),SW_HIDE);   
	  if (!_fstricmp (str2,"LAKEMASTER") || HaveReports)
	  {
		  ShowWindow(GetDlgItem(hDlg,IDC_VIRTUAL_PRINTER),SW_HIDE);   
		  ShowWindow(GetDlgItem(hDlg,IDC_VIRTUAL_PRINTER_LIST),SW_HIDE);   
		  ShowWindow(GetDlgItem(hDlg,IDC_PRINTPREVIEW),SW_HIDE);   
		  ShowWindow(GetDlgItem(hDlg,IDC_WPOPTS),SW_SHOW);   
		  SetDlgItemText (hDlg,IDC_WPOPTS,str);
		  for (i=0;i<3;i++)
		  {   
		  	  sprintf (GlobName,"[%%PRINTREPORTTEXT(%i)]",(int)(i+1));
			  if (GetGlobalCVal (GlobName,str,""))
			  {
				  ShowWindow(GetDlgItem(hDlg,WPCntls[i]),SW_SHOW);   
				  SetDlgItemText (hDlg,WPCntls[i],str);
			  }
		  }	
		  
	  }  
	  SetDlgItemText (hDlg,printheight,CustomHeight);
	  SetDlgItemText (hDlg,printwidth,CustomWidth); 
	  if (InPrintSetup)
		  SetWindowText (hDlg,"Print Setup Options");
	  else
		  SetWindowText (hDlg,"Print Options");
	  if (AutoPrintOK)
    	 PostMessage(hDlg, WM_COMMAND, IDOK, 0L);
	  if (PrinterIsVirtual)
	  {
	  	 SendDlgItemMessage (hDlg,IDC_VIRTUAL_PRINTER,BM_SETCHECK,TRUE,0);
		//goto SetVirtualPrinter;
    	 PostMessage(hDlg, WM_COMMAND, IDC_VIRTUAL_PRINTER, 0L);
	  }
      break;
   
   case WM_DESTROY:  
   	  NumPrintCopies = GetDlgItemInt (hDlg,edt3,&Err,FALSE);
   	  for (i=0;i<3;i++)
	  	if (SendDlgItemMessage (hDlg,WPCntls[i],BM_GETCHECK,0,0))
	  		SetGlobalValueLong ("%PRINTWPOPT",(long)i);
   	  break;
   	  
   case WM_ACTIVATE:
      ghFFRDlg = hDlg;
      break;
    
/*    case WM_SETFOCUS:
 		if (lpPDChunk->hDevMode)
		{ 
			LPDEVMODE pDevMode;
			IgnoreLock = TRUE;
			pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode);
			SendDlgItemMessage (hDlg,cmb2,CB_SELECTSTRING,-1,(LPARAM)pDevMode->dmFormName);
			SendDlgItemMessage (hDlg,rad1,BM_SETCHECK,(pDevMode->dmOrientation==DMORIENT_PORTRAIT),0);  
			SendDlgItemMessage (hDlg,rad2,BM_SETCHECK,!(pDevMode->dmOrientation==DMORIENT_PORTRAIT),0);  
			GlobalUnlock (lpPDChunk->hDevMode); 
			IgnoreLock = FALSE;
		}
   	if (SendDlgItemMessage (hDlg,rad3,BM_GETCHECK,0,0))
    	{  
    		PostMessage (GetDlgItem(hDlg,rad3),WM_LBUTTONDOWN,0,0);  
    		PostMessage (GetDlgItem(hDlg,rad3),WM_LBUTTONUP,0,0); 
    	} 
    	if (SendDlgItemMessage (hDlg,rad4,BM_GETCHECK,0,0))
    	{  
    		PostMessage (GetDlgItem(hDlg,rad4),WM_LBUTTONDOWN,0,0);  
    		PostMessage (GetDlgItem(hDlg,rad4),WM_LBUTTONUP,0,0); 
    	} */
    	break;
    	
    case WM_COMMAND:
         switch(LOWORD(wParam))
        {   
        	case printheight:
        	case printwidth:
                 switch (HIWORD(wParam))
                 {  case EN_CHANGE:
                 	if (GetDlgItemText (hDlg,wParam,str,32))
                 	{  
                 		GetDlgItemText (hDlg,cmb2,str,64);
                 		if (_fstricmp (str,"Custom print size"))
                 		{
         					if (SendDlgItemMessage (hDlg,cmb2,CB_SELECTSTRING,-1,(LPARAM)"Custom print size")==CB_ERR) 
         						SendDlgItemMessage (hDlg,cmb2,CB_ADDSTRING,0,(LPARAM)"Custom print size"); 
         					SendDlgItemMessage (hDlg,cmb2,CB_SELECTSTRING,-1,(LPARAM)"Custom print size"); 
                 		}
                 	}
                 }
        	break; 
        	
        	case IDOK:  
        		if ((PrinterIsVirtual = SendDlgItemMessage (hDlg,IDC_VIRTUAL_PRINTER,BM_GETCHECK,0,0)))
        		{   
        			 LPSTR 	ByLoc,DPILoc;
        			 
                     GetDlgItemText (hDlg,IDC_VIRTUAL_PRINTER_LIST,CurVirtPrinter,126);  
                     if (!_fstricmp (CurVirtPrinter,"Create New Virtual Printer"))
                     {
					      DLGPROC lpfnVIRTUAL_PRINTER_CREATEMsgProc;
						  short nRc;
						  
					      lpfnVIRTUAL_PRINTER_CREATEMsgProc = MakeProcInstance((DLGPROC)VIRTUAL_PRINTER_CREATEMsgProc, hInst);
					      nRc = DialogBox(hInst, (LPSTR)"VIRTUAL_PRINTER_CREATE", hDlg, lpfnVIRTUAL_PRINTER_CREATEMsgProc);
					      FreeProcInstance(lpfnVIRTUAL_PRINTER_CREATEMsgProc); 
					      if (!nRc)
					      	return TRUE; 
                     }
                     if ((DPILoc = _fstrrchr (CurVirtPrinter,'(')))
                     	VirtualPrintDPI = atoi (++DPILoc);
                     else
                     	VirtualPrintDPI = 300;
					 VirtualPlotWidth = atof (CurVirtPrinter) * VirtualPrintDPI; 
					 ByLoc = _fstrstr (CurVirtPrinter," by ");
					 VirtualPlotHeight = atof (ByLoc+4) * VirtualPrintDPI; 
					 if (SendDlgItemMessage (hDlg,IDC_VPOUTPUTTOFILE,BM_GETCHECK,0,0))
					 	GetDlgItemText (hDlg,IDC_VPOUTPUTFILE,VirtPrinterImageFile,255);  
				     else
         			 	*VirtPrinterImageFile = 0;
					 VirtualOrientation = SendDlgItemMessage (hDlg,rad1,BM_GETCHECK,0,0);
               }
                else  
                {
                	GetDlgItemText (hDlg,printwidth,CustomWidth,15);
                	GetDlgItemText (hDlg,printheight,CustomHeight,15);
	        		CreatePrintPreview = SendDlgItemMessage (hDlg,IDC_PRINTPREVIEW,BM_GETCHECK,0,0);
	        	}
				return 0;
        	case IDCANCEL:
        		PrintWasCanceled = TRUE;
				return 0;
			case rad1:
			case rad2:
				ii=1;
				return FALSE;
        	case rad3:
        	case rad4:
			 	ShowWindow (GetDlgItem(hDlg,stc2),SW_SHOW);
			 	ShowWindow (GetDlgItem(hDlg,cmb2),SW_SHOW);
			 	ShowWindow (GetDlgItem(hDlg,grp2),SW_SHOW);
			 	ShowWindow (GetDlgItem(hDlg,psh1),SW_SHOW);
			 	ShowWindow (GetDlgItem(hDlg,stc5),SW_SHOW); 
			 	ShowWindow (GetDlgItem(hDlg,edt3),SW_SHOW);
		 		 if (_fstrnicmp (MODULEIDSTRING,"23",2))
			 		ShowWindow (GetDlgItem(hDlg,IDC_PRINTPREVIEW),SW_SHOW); 
			 	EnableWindow (GetDlgItem(hDlg,IDC_PRINTPREVIEW),TRUE);   
				SendDlgItemMessage (hDlg,IDC_VIRTUAL_PRINTER,BM_SETCHECK,0,0);  
			 	ShowWindow (GetDlgItem(hDlg,IDC_VPOUTPUTTOFILE),SW_HIDE);
			 	ShowWindow (GetDlgItem(hDlg,IDC_VPSELOUTPUTFILE),SW_HIDE);
			 	ShowWindow (GetDlgItem(hDlg,IDC_VPOUTPUTFILE),SW_HIDE);
				return 0;
        	case IDC_VIRTUAL_PRINTER:  
        	{	
        		HFILE Fid;
        		
        		GetGlobalCVal ("[%VIRTUALPRINTERLIST]",VirtualPrinterList,"[%DL]virtualprinters.txt");
        		Fid = GSSiOpenFile (VirtualPrinterList,NULL,OF_READ);
        		
				SendDlgItemMessage (hDlg,rad1,BM_SETCHECK,VirtualOrientation,0);  
				SendDlgItemMessage (hDlg,rad2,BM_SETCHECK,!VirtualOrientation,0);  
   				SendDlgItemMessage (hDlg,IDC_VIRTUAL_PRINTER_LIST,CB_RESETCONTENT,0,0); 
				SetDlgItemText (hDlg,IDC_VPOUTPUTFILE,VirtPrinterImageFile);  
				SendDlgItemMessage (hDlg,IDC_VPOUTPUTTOFILE,BM_SETCHECK,*VirtPrinterImageFile,0);

        		if (Fid != HFILE_ERROR)
        		{
         			while (fgetstring (str,120,Fid))
         			{
         				SendDlgItemMessage (hDlg,IDC_VIRTUAL_PRINTER_LIST,CB_ADDSTRING,0,(LPARAM)str); 
         			}
         			GSSiClose (Fid);
         		}
         		SendDlgItemMessage (hDlg,IDC_VIRTUAL_PRINTER_LIST,CB_ADDSTRING,0,(LPARAM)"Create New Virtual Printer");
 		 		SendDlgItemMessage (hDlg,IDC_VIRTUAL_PRINTER_LIST,CB_SETCURSEL,(WPARAM)0,(LPARAM)NULL); 
				SendDlgItemMessage (hDlg,IDC_VIRTUAL_PRINTER,BM_SETCHECK,1,0);  
				SendDlgItemMessage (hDlg,rad3,BM_SETCHECK,0,0);  
				SendDlgItemMessage (hDlg,rad4,BM_SETCHECK,0,0);  
				SendDlgItemMessage (hDlg,IDC_PRINTPREVIEW,BM_SETCHECK,0,0);  
		   		SendDlgItemMessage (hDlg,IDC_VIRTUAL_PRINTER_LIST,CB_SELECTSTRING,-1,(LPARAM)CurVirtPrinter); 
			 	EnableWindow (GetDlgItem(hDlg,IDC_PRINTPREVIEW),FALSE);
			 	ShowWindow (GetDlgItem(hDlg,IDC_PRINTPREVIEW),SW_HIDE); 
			 	ShowWindow (GetDlgItem(hDlg,stc2),SW_HIDE);
			 	ShowWindow (GetDlgItem(hDlg,cmb2),SW_HIDE);
			 	ShowWindow (GetDlgItem(hDlg,grp2),SW_HIDE); 
			 	ShowWindow (GetDlgItem(hDlg,psh1),SW_HIDE); 
			 	ShowWindow (GetDlgItem(hDlg,stc5),SW_HIDE); 
			 	ShowWindow (GetDlgItem(hDlg,edt3),SW_HIDE); 
			 	ShowWindow (GetDlgItem(hDlg,IDC_VPOUTPUTTOFILE),SW_SHOW);
			 	ShowWindow (GetDlgItem(hDlg,IDC_VPSELOUTPUTFILE),SW_SHOW);
			 	ShowWindow (GetDlgItem(hDlg,IDC_VPOUTPUTFILE),SW_SHOW);
				EnableWindow (GetDlgItem(hDlg,IDC_VPSELOUTPUTFILE),FALSE); 
				EnableWindow (GetDlgItem(hDlg,IDC_VPOUTPUTFILE),FALSE); 

			 }
        	    break;
            
            case IDC_VPOUTPUTTOFILE:
				if (SendDlgItemMessage (hDlg,IDC_VPOUTPUTTOFILE,BM_GETCHECK,0,0))  
				{
				 	EnableWindow (GetDlgItem(hDlg,IDC_VPSELOUTPUTFILE),TRUE); 
				 	EnableWindow (GetDlgItem(hDlg,IDC_VPOUTPUTFILE),TRUE); 
				}
				return 0; 
            case IDC_VPSELOUTPUTFILE:
                if (GetSaveName2 (hDlg,str,IDS_FILTERIMAGE,0,IDS_FILEIMAGE))
                	SetDlgItemText (hDlg,IDC_VPOUTPUTFILE,str); 
        		return 0;

            case IDC_VIRTUAL_PRINTER_LIST:
                 switch(HIWORD(wParam))
                 {   
                 	
                     case CBN_SELCHANGE:
                         Choice=(short)SendDlgItemMessage(hDlg,HIWORD(wParam),CB_GETCURSEL,0,0);
                         SendDlgItemMessage(hDlg,wParam,CB_GETLBTEXT,Choice,(DWORD)&str);  
                         if (!_fstricmp (str,"Create New Virtual Printer"))
                         {
						      DLGPROC lpfnVIRTUAL_PRINTER_CREATEMsgProc;
						      short	nRc;
						      
						      lpfnVIRTUAL_PRINTER_CREATEMsgProc = MakeProcInstance((DLGPROC)VIRTUAL_PRINTER_CREATEMsgProc, hInst);
						      nRc = DialogBox(hInst, (LPSTR)"VIRTUAL_PRINTER_CREATE", hDlg, lpfnVIRTUAL_PRINTER_CREATEMsgProc);
						      FreeProcInstance(lpfnVIRTUAL_PRINTER_CREATEMsgProc);
						      if (nRc)
						      {
				        		HFILE Fid;
				        		
        		        		GetGlobalCVal ("[%VIRTUALPRINTERLIST]",VirtualPrinterList,"[%DL]virtualprinters.txt");   
        		        		Fid = GSSiOpenFile (VirtualPrinterList,NULL,OF_READ);
				        		if (Fid != HFILE_ERROR)
				        		{
				         			while (fgetstring (str,120,Fid))
				         			{
				         				SendDlgItemMessage (hDlg,IDC_VIRTUAL_PRINTER_LIST,CB_ADDSTRING,0,(LPARAM)str); 
				         			}
				         			GSSiClose (Fid);
				         		}
				         		SendDlgItemMessage (hDlg,IDC_VIRTUAL_PRINTER_LIST,CB_SELECTSTRING,-1,(LPARAM)CurVirtPrinter); 
						      }
                         }
                         
                     return 0;
                 }
                 break;

        	break;	    
        	case psh1: //options
				if (lpPDChunk->hDevMode)
				{ 
					LPDEVMODE pDevMode;
					IgnoreLock = TRUE;
					pDevMode = (LPDEVMODE)GlobalLock (lpPDChunk->hDevMode);
					GlobalUnlock (lpPDChunk->hDevMode); 
					IgnoreLock = FALSE;
				}
				if (SendDlgItemMessage (hDlg,IDC_VIRTUAL_PRINTER,BM_GETCHECK,0,0))  
				{   
					char	VPName[128];
					
					GetDlgItemText (hDlg,IDC_VIRTUAL_PRINTER_LIST,VPName,128);
					SetupVirtualPrinter (hDlg,VPName);
					return TRUE;
				}
				return FALSE;
			break;
			
        	case cmb2:
			switch(HIWORD(wParam))
            {
            	case CBN_SELCHANGE:
					*CustomHeight = 0;
					*CustomWidth = 0;
					SetDlgItemText (hDlg,printheight,CustomHeight);
					SetDlgItemText (hDlg,printwidth,CustomWidth); 
					return 0;
            }
            break;
        }
		return FALSE;
        break;
   default:
	   return FALSE;
      break;
      }
   return FALSE;
}

UINT CALLBACK  ColorHook (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{  
	short	ii, BRtn, Pattern;
	UINT	ColorOKMess;
	static	COLORREF	CurColor;  
	POINT	CPnt;
	HWND	hwndCC, hwndOC; 
	HDC		hDC;
	RECT	Rect;
	HBRUSH	hbrush;
	static	LPCHOOSECOLOR lpcc;
	UINT	uiSetRGB;
	static	PATBYTE	PatByte;
		
	hwndCC = GetDlgItem (hDlg,COLOR_CURRENT);
    if (hwndCC)
    {
SetCurColor:
    	GetWindowRect (hwndCC,&Rect);
    	CPnt.x = (Rect.right - Rect.left)/2;
    	CPnt.y = (Rect.bottom - Rect.top)/2;
   		hDC = GetDC (hwndCC); 
    	CurColor = GetPixel (hDC,CPnt.x,CPnt.y);
   		ReleaseDC (hwndCC,hDC);
   		hwndOC = GetDlgItem (hDlg,IDC_OUTCOLOR);
   		GetWindowRect (hwndOC,&Rect);   
   		Rect.right = Rect.right - Rect.left - 1;
   		Rect.left = 1;
   		Rect.bottom = Rect.bottom - Rect.top - 1;
   		Rect.top = 1; 
   		GetPatternOpt (hDlg,&PatByte);
   		hDC = GetDC (hwndOC);
   		hbrush = CreatePatBrush (hDC,CurColor,&PatByte); 
   		FillRect (hDC,&Rect,hbrush);  
   		DeleteObject (hbrush);
   		ReleaseDC (hwndOC,hDC);
//   		return 0;
   	}	
	if ((BRtn = DIALOGSTYLEMsgProc (hDlg,message, wParam, lParam)))
		return (BRtn);
    
	ColorOKMess = RegisterWindowMessage(COLOROKSTRING); 
	if (message == ColorOKMess)
	{
		long	OutColor; 
		
//		lpcc = (LPCHOOSECOLOR) lParam;
   		GetPatternOpt (hDlg,&PatByte);
	
		R = GetRValue (CurColor);
		G = GetGValue (CurColor);
		B = GetBValue (CurColor); 
		_fmemmove (&W,&PatByte,1);
		lpcc->rgbResult = RGBW (R,G,B,W);
        return 0;  
    }    
   switch (message)
      {
   case WM_INITDIALOG:
		lpcc = (LPCHOOSECOLOR) lParam;
		_fmemmove (&PatByte,&W,1);
		SendDlgItemMessage (hDlg,IDC_SOLID,BM_SETCHECK,TRUE,0);
		SetPatternOpt (hDlg,&PatByte);
		lpcc->rgbResult = RGB (R,G,B);
      return (TRUE);
   
   case WM_COMMAND:
      switch (wParam)
         {  
      case IDC_XHATCH:
      case IDC_XHATCH2:
      case IDC_50PCT:
      case IDC_25PCT:
      case IDC_12PCT:
      case IDC_50PCT2:
      case IDC_25PCT2:
      case IDC_12PCT2: 
      case IDC_0PCT: 
      case IDC_SOLID:
   		GetPatternOpt (hDlg,&PatByte);
         break;
         
      default:
         break;
         }
      break;

   default:
	   return FALSE;
      break;
      }
   return FALSE;
}

/*BOOL FAR PASCAL _export ABORTWAITMsgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{	char	str[256];
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hDlg,message, wParam, lParam)))
 	return (BRtn);
   switch (message)
      {
   case WM_INITDIALOG: 
   		hWndAbortWaitMessage = hDlg;

   		break;
   case WM_DESTROY:
        hWndAbortWaitMessage = 0;  
//		FreeProcInstance(lpfnABORTWAITMsgProc);
        break;
   default:
      return FALSE;
      }
   return TRUE;
}
*/

/**************************************************************************
*                                                                         *
*  Function:  AbortProc (HDC, int)                                        *
*                                                                         *
*   Purpose:  This is the abort procedure for the PrintDlg sample.        *
*                                                                         *
*   Returns:  BOOL                                                        *
*                                                                         *
*  Comments:  This function must be exported and must have an instance    *
*             thunk created for it (ie, call MakeProcInstance)            *
*                                                                         *
*   History:  Date      Reason                                            *
*             --------  -----------------------------------               *
*                                                                         *
*             01/31/92  Created                                           *
*                                                                         *
**************************************************************************/

BOOL FAR PASCAL AbortProc ( HDC hPrinterDC, short nCode )

{
    MSG msg;

    if (!gbUserAbort && !ghPrintingDlg)              /* If the abort dialog isn't up yet */
        return(TRUE);
    HaveAbortProc = TRUE;
    /* Process messages intended for the abort dialog box */
    if (nCode)
    	gbUserAbort = TRUE;
    while (!gbUserAbort && GSSiPeekMessage(&msg, NULL, 0, 0, TRUE)) 
    {
#if ENABLETRACE
SetLastMessage(-1*(long)msg.message);
#endif
        if (!IsDialogMessage(ghPrintingDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    /* bAbort is TRUE (retrn is FALSE) if the user has aborted */
	if (gbUserAbort)
	{
        HaveAbortProc = FALSE;
/*		if(!hWndAbortWaitMessage)
		{
	          lpfnABORTWAITMsgProc = MakeProcInstance((DLGPROC)ABORTWAITMsgProc, ghInst);
	          CreateDialog(ghInst, (LPSTR)"ABORTWAIT", hWndMain, lpfnABORTWAITMsgProc);
			  while (PeekMessage(&msg, NULL, NULL, NULL, TRUE));
	    }*/
	}
    return (!gbUserAbort);
}

/*BOOL FAR PASCAL __export AbortProc ( HDC hPrinterDC, int nCode )

{
   MSG msg;

   while (!gbUserAbort && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
   {
      if (!ghPrintingDlg)
      {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
      else if (IsWindow(ghPrintingDlg)) 
      {
      	if (msg.hwnd == ghPrintingDlg)
         if (!IsDialogMessage(ghPrintingDlg, &msg))
         {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
         }
      }
   }
   retrn !gbUserAbort;
} */

void PrintMessage (short	ViewID, LPSTR File, short record)
{ 
	char	str[256];
	
	wsprintf (str,"Viewport: %d",ViewID);
	SetDlgItemText (PrintMsgWnd,PRINT_VIEW,str);
	_fstrcpy (str,"File: ");
	_fstrcat (str,PltName);
	SetDlgItemText (PrintMsgWnd,PRINT_FILE,str);
    if (record)
    	itoa (record,str,10);
    else
    	str[0]='\0';
    SetDlgItemText (PrintMsgWnd,IDC_PRINT_RECORD,str);
	return;
}

void PrintMessage2 (LPSTR line1, LPSTR line2, LPSTR line3)
{ 
	char	str[256];
	
    if (line1)
    	SetDlgItemText (PrintMsgWnd,IDC_PRINT_RECORD,line1);
	if (line2)
		SetDlgItemText (PrintMsgWnd,PRINT_VIEW,line2);
	if (line3)
		SetDlgItemText (PrintMsgWnd,PRINT_FILE,line3);
	return;
}

/**************************************************************************
*							  											  *
*  Function:  PrintDlgProc (HWND, UINT, WPARAM, LPARAM)		 			  *
*                                                                         *
*   Purpose:  This is the dialog box procedure for the dialog that pops   *
*             up indicating that we are printing.                         *
*                                                                         *
*   Returns:  BOOL                                                        *
*                                                                         *
*  Comments:  This function must be exported and must have an instance    *
*             thunk created for it (ie, call MakeProcInstance)            *
*                                                                         *
*   History:  Date      Reason                                            *
*             --------  -----------------------------------               *
*                                                                         *
*             01/31/92  Created                                           *
*                                                                         *
**************************************************************************/


UINT CALLBACK  PrintDlgProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM
                              lParam)
{	char	str[256];
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hDlg,message, wParam, lParam)))
 	return (BRtn);
/*   {
   		RECT	Rect;
   		HDC		hDC;
   		
   		GetClientRect (hDlg,&Rect);
   		hDC = GetDC (hDlg);
        DisplayBMFileInRect (hDC,"c:\\gmavl\\ambulance.bmp",Rect,FALSE);
   		ReleaseDC (hDlg,hDC);
   	}*/
   switch (message)
      {
   case WM_INITDIALOG: 
   		PrintMsgWnd = hDlg;
   		ContinueProcessing=TRUE; 
        HaveAbortProc = FALSE;
		if (!BackgroundTask)
		{
			cwCenter(hDlg, 0);
			ShowWindow (hDlg,SW_SHOW);
		}

   		break;
   case WM_DESTROY:
        PrintMsgWnd = 0;  
		LeaveBlockingWindow(hSaveStatWindowBM);
		hSaveStatWindowBM = 0;
        break;
/*   case WM_PAINT:
   {
   		PAINTSTRUCT	ps;
        HDC	hDC;
        
          _fmemset(&ps, 0x00, sizeof(PAINTSTRUCT));
         hDC = BeginPaint(hDlg, &ps);
         DisplayBMFileInRect (hDC,"c:\\gmavl\\ambulance.bmp",ps.rcPaint,FALSE);
         EndPaint(hDlg, &ps); 
   }
   		break; */
   case WM_COMMAND:  
   {
   		RECT	Rect;
   		HDC		hDC;
   		
/*   		GetClientRect (hDlg,&Rect);
   		hDC = GetDC (hDlg);
        DisplayBMFileInRect (hDC,"c:\\gmavl\\ambulance.bmp",Rect,FALSE);
   		ReleaseDC (hDlg,hDC);*/
   	}
      switch (wParam)
         {
      case IDCANCEL:
		 EnableWindow (hWndMain,TRUE);
         gbUserAbort = TRUE;
         //ContinueProcessing=FALSE;
         if (!HaveAbortProc)
         {
	         DestroyWindow(PrintMsgWnd); 
	         ghPrintingDlg = NULL;
	     } 
	     else
		 	 SetDlgItemText (PrintMsgWnd,PRINT_VIEW,"Please wait for print to be aborted");
	     
//		 SetCurs(LoadCursor(NULL, IDC_WAIT),TRUE);
         return TRUE;

      default:
         return FALSE;
         }
      break;

   default:
      return FALSE;
      }
   return TRUE;
}
/**************************************************************************
*                                                                         *
*      File:  ERROR.C                                                     *
*                                                                         *
*   Purpose:  Contains the error routines for this program                *
*                                                                         *
* Functions:  void FAR ProcessCDError(DWORD)                              *
*             void FAR ReportError(WORD)                                  *
*                                                                         *
*  Comments:                                                              *
*                                                                         *
*   History:  Date      Reason                                            *
*             --------  -----------------------------------               *
*                                                                         *
*             10/01/91  Created                                           *
*                                                                         *
**************************************************************************/


/**************************************************************************
*                                                                         *
*  Function:  ProcessCDError(DWORD)                                       *
*                                                                         *
*   Purpose:  To report an error that has occurred during the last        *
*             call to a CD routine.                                       *
*                                                                         *
*   Returns:  void                                                        *
*                                                                         *
*  Comments:                                                              *
*                                                                         *
*   History:  Date      Reason                                            *
*             --------  -----------------------------------               *
*                                                                         *
*             10/01/91  Created                                           *
*                                                                         *
**************************************************************************/
void FAR ProcessCDError(DWORD dwErrorCode)
{
   WORD  wStringID;

   switch(dwErrorCode)
      {
         case CDERR_DIALOGFAILURE:   wStringID=IDS_DIALOGFAILURE;   break;
         case CDERR_STRUCTSIZE:      wStringID=IDS_STRUCTSIZE;      break;
         case CDERR_INITIALIZATION:  wStringID=IDS_INITIALIZATION;  break;
         case CDERR_NOTEMPLATE:      wStringID=IDS_NOTEMPLATE;      break;
         case CDERR_NOHINSTANCE:     wStringID=IDS_NOHINSTANCE;     break;
         case CDERR_LOADSTRFAILURE:  wStringID=IDS_LOADSTRFAILURE;  break;
         case CDERR_FINDRESFAILURE:  wStringID=IDS_FINDRESFAILURE;  break;
         case CDERR_LOADRESFAILURE:  wStringID=IDS_LOADRESFAILURE;  break;
         case CDERR_LOCKRESFAILURE:  wStringID=IDS_LOCKRESFAILURE;  break;
         case CDERR_MEMALLOCFAILURE: wStringID=IDS_MEMALLOCFAILURE; break;
         case CDERR_MEMLOCKFAILURE:  wStringID=IDS_MEMLOCKFAILURE;  break;
         case CDERR_NOHOOK:          wStringID=IDS_NOHOOK;          break;
         case PDERR_SETUPFAILURE:    wStringID=IDS_SETUPFAILURE;    break;
         case PDERR_PARSEFAILURE:    wStringID=IDS_PARSEFAILURE;    break;
         case PDERR_RETDEFFAILURE:   wStringID=IDS_RETDEFFAILURE;   break;
         case PDERR_LOADDRVFAILURE:  wStringID=IDS_LOADDRVFAILURE;  break;
         case PDERR_GETDEVMODEFAIL:  wStringID=IDS_GETDEVMODEFAIL;  break;
         case PDERR_INITFAILURE:     wStringID=IDS_INITFAILURE;     break;
         case PDERR_NODEVICES:       wStringID=IDS_NODEVICES;       break;
         case PDERR_NODEFAULTPRN:    wStringID=IDS_NODEFAULTPRN;    break;
         case PDERR_DNDMMISMATCH:    wStringID=IDS_DNDMMISMATCH;    break;
         case PDERR_CREATEICFAILURE: wStringID=IDS_CREATEICFAILURE; break;
         case PDERR_PRINTERNOTFOUND: wStringID=IDS_PRINTERNOTFOUND; break;
         case CFERR_NOFONTS:         wStringID=IDS_NOFONTS;         break;
         case FNERR_SUBCLASSFAILURE: wStringID=IDS_SUBCLASSFAILURE; break;
         case FNERR_INVALIDFILENAME: wStringID=IDS_INVALIDFILENAME; break;
         case FNERR_BUFFERTOOSMALL:  wStringID=IDS_BUFFERTOOSMALL;  break;

         case 0:   //User may have hit CANCEL or we got a *very* random error
         default:
            return;
      }

   if (!LoadString(ghInst, wStringID, gszBuffer, lngszBuffer))
      {
         ReportError(IDC_LOADSTRINGFAIL);
         return;
      }

   GSSiMsgBox(ghWnd, gszBuffer, szAppName, MB_OK,0);
   return;
}


/**************************************************************************
*                                                                         *
*  Function:  ReportError(WORD)                                           *
*                                                                         *
*   Purpose:  To report an error that has occurred while allocating       *
*             memory for the CD struct, locking the memory or while       *
*             trying to load a resource string.                           *
*                                                                         *
*   Returns:  void                                                        *
*                                                                         *
*  Comments:                                                              *
*                                                                         *
*   History:  Date      Reason                                            *
*             --------  -----------------------------------               *
*                                                                         *
*             10/01/91  Created                                           *
*                                                                         *
**************************************************************************/
void FAR ReportError(WORD wErrorType)
{
   LPSTR lpszErrorMsg;

   switch( wErrorType )
      {
         case IDC_ALLOCFAIL:

            lpszErrorMsg=gszAllocErrorMsg;
            break;

         case IDC_LOCKFAIL:

            lpszErrorMsg=gszLockErrorMsg;
            break;

         case IDC_LOADSTRINGFAIL:

            lpszErrorMsg=gszLoadStrFail;
            break;

         default:    //let's hope we never get here!
            return;
      }

   GSSiMsgBox(ghWnd, (LPSTR)lpszErrorMsg, szAppName, MB_OK,0);

   return;
}
