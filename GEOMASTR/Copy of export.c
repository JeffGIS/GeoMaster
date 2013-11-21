#include <direct.h>
#include "graphint.h"
#include <float.h>
#include "..\geomastr\resource.h"  
#include "dict.h"
#include "address.h"  
#include "dibapi.h"   
#include "STD.H"  
#include "translat.h"  
#include "16two32.h"
short EXType;

char MIFOutDataFile[128], MIFOutSQL[256];
short  MIFOUTDataFileType, MIFOUTFormat=0; 
HANDLE  MIFOutFields=0;
HANDLE MIFOuthDB=0; 
short	NumMemMaps=0; 
#define	MAXMEMMAPS	256
HBITMAP	hMemMaps[MAXMEMMAPS];
MNMXCORD	MemMapBounds[MAXMEMMAPS]; 
HANDLE	MemMapTrans[MAXMEMMAPS];

extern	BOOL	SetDimensions;
extern	RECT	SetDimRect;
extern	short	MAXORTHOBUFS, UsedOrthoBufs;
extern	LPORTHO	CurOrtho;
extern	int		OrthoID;
extern	HANDLE	hOrthos;  
extern	double	OrthoRes,FileDistToWinDist,BaseDistToWinDist;
extern	long	OrthoUse;
extern	MNMXCORD	EditBounds; 
extern	char	AutoExportName[128];
extern	long	LastQuadOff;
extern	HFILE	Fid;
extern  BOOL    IgnoreBounds, ForceRefIndex;
extern	short	FileProjectionType;
extern	char	CurSymSize[64], CurSymRot[64];  
extern	int		CurSymSizeUnits, CurSymRotUnits;
extern  HANDLE  hSavePoly,hSavePolyParts;
extern	short	nSavePoly;
extern	HANDLE	hDBStreetSegments; 
extern  char    CurODBCFile[64];
extern  long    CurStreetNumbers[4], FromStreet, ToStreet,BlankStreet;  
extern  long  PRJ_UNITS[4];
extern  HANDLE  hHighlight, hIntData;
extern  short   CurState; 
extern	long	UsedDescOffset;
extern  BOOL    DisableHalt, ContinueProcessing, Processing, FastOrthos;
extern  HANDLE  hDBSegdata,hStreetSegFields, hImportLimits, hGRCommand, hGRText, hTimeStamp;
extern  HANDLE  hSegData;
extern  HFILE   SegDataFid;   
extern  HANDLE  hNames1, hNames2;
extern  short     NumTAGDef,CommandViewport;  
extern  HANDLE  hTAGDef;  
extern  long    CurrentRefno;
extern  char    CurrentTAG[10], CurrentUDI[34];
extern  short     CurrentUDILen;
extern  short     PltType;
extern  char    PickName[128],PltName[128];
extern  BOOL    Pick, Printing;
extern  char    FullBM[256];
extern  short   NumPicked; 
extern	short	MapVersion;
extern  PICKDATA    PickList[MAXPICKITEMS];
extern  LPVISLIST   CurVis;
extern  LPVIEWPORT  CurView, pViewports[MAX_VIEWPORTS], pViewportsD[MAX_VIEWPORTS];
extern  HANDLE      hViewports[MAX_VIEWPORTS];
extern  short         NumViewports, NumViewportsToDisplay, DisplayViewID;
extern  LPTHEME     CurTheme;
extern  LPVOID      TranFileToBase, TranBaseToFile, TranBMToBase, TranBaseToBM;
extern  HANDLE      hTranFileToBase, hTranBaseToFile, hTranBMToBase, hTranBaseToBM;
extern  short         OrthoRedAdjust,OrthoGreenAdjust,OrthoBlueAdjust,OrthoIntensity;
extern  HANDLE      hInst;   
extern  HWND        hWndMain;
extern  double      WidthFactor;
extern  RGBQUAD     rgb;
extern  HPEN        hSavePen, hNullPen, hRedPen, pens[MAXPENS], HighlightPen;
extern  HPEN        CurrentPen;
extern  HBRUSH      brushes[MAXPENS], hRedBrush, hGreenBrush, hBlueBrush, HighlightBrush;
extern  short         maxbrush; 
extern  BOOL        DoPaint;
extern  double      OrthoAdjustX, OrthoAdjustY;
extern  char        gszFilter[256];
extern  char    EditName[128];  
extern  short     EditFileNum;
extern  char    RefIndexFile[128];
extern  HANDLE  hRefIdx,hTAGIdx; 
extern  REFINDEXDATA    RefIdxData; 
extern	BOOL	MemMap, FirstMemMap;  
extern	char	MemMapName[128];
extern	HBITMAP	hMemBitmap;  
extern	HDC hdcMemMap;  

extern	int		MemMapWidth,MemMapHeight;

BOOL GetColorVal (LPSTR pRow,LPDPOINT pBasePoint);
BOOL CreateBMPs (LPSTR Name,double Res,double Offset,HWND hWndDlg);
BOOL FindOrthoBuf (LPDPOINT pBasePoint);

BOOL GetColorValx (LPSTR pPel,LPDPOINT pBasePoint)
{   
    POINT	WinPt;  
    COLORREF Color; 
    unsigned short	i; 
    double	X,Y;   
    HBITMAP	hOldBM;
    i = NumMemMaps;
    while (i--)
		if (DPointInBounds (pBasePoint,&MemMapBounds[i]))
			goto InBounds;
	i = NumMemMaps;
	if (NumMemMaps >= MAXMEMMAPS)
	{
		GSSiMessageBox ("Memory map buffer overflow",NULL,MB_ICONEXCLAMATION);
		ContinueProcessing = FALSE;
	}
	CurView->NewBounds.xmn = pBasePoint->x - (MemMapWidth * CurView->OrthoRes)/2;
	CurView->NewBounds.xmx = pBasePoint->x + (MemMapWidth * CurView->OrthoRes)/2;
	CurView->NewBounds.ymn = pBasePoint->y - (MemMapHeight * CurView->OrthoRes)/2;
	CurView->NewBounds.ymx = pBasePoint->y + (MemMapHeight * CurView->OrthoRes)/2;
    SetBounds (CurView->hWnd,NULL);
	CurView->WindowZoomedToOrtho = TRUE;
	CurView->WindowIsZoomed = TRUE; 
	SaveCurView (0);
	PaintMap (CurView->hWnd,hdcMemMap,TRUE);  
	SaveCurView (1); 
	hMemBitmap = CreateCompatibleBitmap (hdcMemMap,MemMapWidth,MemMapHeight);
	hMemMaps[NumMemMaps] = SelectObject(hdcMemMap, hMemBitmap);
    MemMapBounds[NumMemMaps] = CurView->WBounds;
    MemMapTrans[NumMemMaps++] = CurView->hTranBaseToWin; 
    CurView->hTranBaseToWin = 0;
InBounds:	
    TRANS2 (pBasePoint->x,pBasePoint->y,&X,&Y,MemMapTrans[i]);
    WinPt.x = IDNINT (X);
    WinPt.y = IDNINT (Y); 
   	hOldBM = SelectObject (hdcMemMap,hMemMaps[i]);
    Color = GetPixel (hdcMemMap,WinPt.x,WinPt.y);   
    *pPel = GetBValue (Color);     
	SelectObject (hdcMemMap,hOldBM);
    return TRUE;
}  

BOOL GetColorVal (LPSTR pPel,LPDPOINT pBasePoint)
{   
	LPBITMAPINFOHEADER	lpbi;
	long	col, row, nBytesPerPel, RowLen;
	HPSTR	pPixel, startimage; 
    
    if (!FindOrthoBuf (pBasePoint))
    {
    	*pPel = 0;
    	return FALSE;
    }
    CurOrtho->LastUsed = OrthoUse++;
    
    lpbi = GlobalLock (CurOrtho->hDib);
   	startimage = (LPSTR) lpbi + (lpbi->biSize+lpbi->biClrUsed*sizeof(COLORREF));
	col = IDNINT ((pBasePoint->x - CurOrtho->Bounds.xmn) / 
				 ((CurOrtho->Bounds.xmx - CurOrtho->Bounds.xmn) / (CurOrtho->Width - 1)));
	row = IDNINT ((pBasePoint->y - CurOrtho->Bounds.ymn) /
				 ((CurOrtho->Bounds.ymx - CurOrtho->Bounds.ymn) / (CurOrtho->Height - 1)));
	nBytesPerPel = lpbi->biBitCount/8;   
	RowLen = nBytesPerPel * lpbi->biWidth;
	RowLen += RowLen%4;
	pPixel = startimage + (row * RowLen + col * nBytesPerPel);	
	
	*pPel = *pPixel;
	GlobalUnlock (CurOrtho->hDib);
	return (FALSE);
}  

BOOL FindOrthoBuf (LPDPOINT pBasePoint)
{	int i, MinOrthoID;
	static	LastOrthoID=0;
	long	MinUse;
	LPORTHO	MinOrtho;   
    MNMXCORD Bounds;
	BOOL	FirstPass=TRUE, SaveMemMap=MemMap;  
	HBITMAP	hbmpOld;
	
	if (!hOrthos)
		OpenOrthos();

Top:  
	UnlockHandles();  
    CurOrtho = (LPORTHO)GlobalLock(hOrthos); 
    CurOrtho += LastOrthoID;
	if (*CurOrtho->Name)
	{   
		Bounds = CurOrtho->Bounds;
		InflateBounds (&Bounds,CurOrtho->Res/2);
		if (DPointInBounds (pBasePoint,&Bounds))
			return TRUE;
	}
	GlobalUnlock (hOrthos);
    CurOrtho = (LPORTHO)GlobalLock(hOrthos); 
	for (OrthoID=0;OrthoID<UsedOrthoBufs;OrthoID++,CurOrtho++)
	{
		if (*CurOrtho->Name)
		{   
			Bounds = CurOrtho->Bounds;
			InflateBounds (&Bounds,CurOrtho->Res/2);
			if (DPointInBounds (pBasePoint,&Bounds))
			{
				LastOrthoID = OrthoID;
				GlobalUnlock (hOrthos);
				return TRUE;
			}
		}
	}  
	return FALSE;
				
} 
                                
BOOL FAR PASCAL BMP_OUTPUTMsgProc(HWND hWndDlg, WORD Message, WPARAM wParam, LPARAM lParam)
{ 
    long NumItems;  
    char    File[128],  ExtID[32], Name[128], str[128], project[34];
    LPSTR   lpDot;   
    HFILE   Fid;
    OFSTRUCT    OFStruct; 
    static	MNMXCORD 	SaveBounds; 
    double	Res, Offset;
    static	short	SaveMaxOrtho, SaveFastOrthos;

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
    
        NumItems = BT_NUM_IN_INDEX(hHighlight);  
        if (!NumItems)
        { 
            MessageBox( GetFocus(),"No items highlighted","Error", MB_OK);
            PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
            break;
        }  
         sprintf(str,"%ld items selected",NumItems);
         SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,str);  
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         _fstrcpy (str,"*.CVT");
         DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE);
		 CloseOrthos ();
         SaveMaxOrtho = GetGlobalLVal ("[%ORTHO_BUFFERS]");   
		 SetGlobalValueLong ("%ORTHO_BUFFERS",1024);
         SaveFastOrthos = FastOrthos;
         FastOrthos = FALSE;
         SaveBounds = CurView->WBounds;
         PostMessage(hWndMain, WM_COMMAND, IDM_Z_REDRAW, 0L);
         break;  

    case WM_CLOSE:
         PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
         break; 

    case WM_COMMAND:
#if WIN32
         switch(LOWORD(wParam))
#else
         switch(wParam)
#endif
           { 
            case IDCANCEL:
                 if (Processing)
                     ContinueProcessing=FALSE;
                 Processing = FALSE;
                 break;
                 
            case IDC_EXIT:     
				 CloseOrthos ();
				 SetViewport(CommandViewport);
                 ContinueProcessing = TRUE;
		         FastOrthos = SaveFastOrthos;
				 SetGlobalValueLong ("%ORTHO_BUFFERS",SaveMaxOrtho);
                 EndDialog(hWndDlg, TRUE);  
				 PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
                 break;
                 
            case IDC_SELECT_OUTPATH:
            {   
                char    Name[128];
                if (GetSaveName2 (hWndDlg,Name,IDS_FILTERBMP,".BMP",IDS_FILEBMP))
                    SetDlgItemText (hWndDlg,IDC_OUT_FILE,Name);
            }
                break;
             
            case IDOK: 
            {
				SetViewport(CommandViewport);
                GetDlgItemText (hWndDlg,IDC_OUT_FILE,str,sizeof(str));
                if (!GetDlgItemText (hWndDlg,IDC_PROJECTION,project,sizeof(project)))
                {
                    MessageBox(GetFocus(),"No output projection set", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
                }
                if ((lpDot=_fstrrchr(project,'.')))
                    *lpDot = 0;
                SetGlobalValue("%ALT_PROJECTION",project);
			    ConvertCoordClose ();
				ConvertCoordInit();
                GetDlgItemText (hWndDlg,IDC_UNITS,str,sizeof(str));
                if (*str)
                { 
                    if (!_fstrcmp(str,"Feet"))
                        PRJ_UNITS[3] = 1;
                    else if (!_fstrcmp(str,"Meters"))
                        PRJ_UNITS[3] = 2;
                }
                else
                {
                    MessageBox(GetFocus(),"Units field not set", 0,MB_ICONQUESTION|MB_OK);
                    break;
                }
                GetDlgItemText (hWndDlg,IDC_RESOLUTION,str,sizeof(str));
                Res = atof (str);  
                if (!Res)
                {
                    MessageBox(GetFocus(),"Resolution not set", 0,MB_ICONQUESTION|MB_OK);
                    break;
                }
                GetDlgItemText (hWndDlg,IDC_OFFDIST,str,sizeof(str));
                Offset = atof (str);  
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE); 
				ContinueProcessing=TRUE;  
				Processing = TRUE;
                GetDlgItemText (hWndDlg,IDC_OUT_FILE,Name,sizeof(Name));  
                CreateBMPs (Name,Res,Offset,hWndDlg);  
                Processing = FALSE;
		        PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
           }
           break;
           }
         break; 

    default:
        return FALSE;
   }
 return TRUE;
}
                                  
BOOL CreateBMPs (LPSTR Name,double Res,double Offset,HWND hWndDlg)
{   
	short	pos=BT_FIRST; 
	long	iref, OutLen, NumItems, Item=1; 
	HIGHLIGHTDATA	HighlightData;  
	HANDLE	hPoints;
	HPDPOINT	pPoints;
	MNMXCORD	MnMx;
	short	Nump; 
	short	width, height;
    LPBITMAPINFO    pDibInfo, pDibInfoOut;
	HANDLE  hDibInfoOut,hRow;
    BITMAPFILEHEADER bmfHead;
    WORD    HeadLen, irow, icol, i; 
    HFILE	FidBM, FidBPW;
    OFSTRUCT	OFStruct;
    LPSTR	pRow;   
    double	X, Y, XFROM[4],YFROM[4],XTO[4],YTO[4];
    DPOINT	BasePoint, DPoint; 
    short	nBytesPerPel,ii; 
    HANDLE	hTran;  
    float	RSQMIN;
   	HBITMAP	hbmpOld;  
   	char	BPWName[128], str[256];  
   	LPSTR	lpDot; 
   	BOOL	rtn=FALSE;
		
	if (!GetCurOrtho(0))
		return FALSE;
	pDibInfo = (LPBITMAPINFOHEADER) GlobalLock (CurOrtho->hDib);  
	if (!pDibInfo)
		return FALSE;
    HeadLen = sizeof(BITMAPINFOHEADER)+pDibInfo->bmiHeader.biClrUsed*sizeof(RGBQUAD);
    hDibInfoOut = GSSiGlobAlloc (GHND,HeadLen);
    pDibInfoOut = (LPBITMAPINFO)GlobalLock (hDibInfoOut); 
    _fmemmove (pDibInfoOut,pDibInfo,HeadLen);      
	CloseOrthos ();
    NumItems = BT_NUM_IN_INDEX(hHighlight);  
    while (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData)&&ContinueProcessing)
    {   
    	RECT	Rect;
    	
        pos = BT_NEXT;
        PickList[0]=HighlightData.PD;
		CloseOrthos ();
		CurView->NewBounds = PickList[0].Rect;
		InflateBounds (&CurView->NewBounds,Offset+1);
		SetDimensions=TRUE;
        GetClientRect(hWndMain, &Rect);
		MemMapWidth = 0.9 * (Rect.right - Rect.left);
		MemMapHeight = MemMapWidth * (CurView->NewBounds.ymx - CurView->NewBounds.ymn)/
									 (CurView->NewBounds.xmx - CurView->NewBounds.xmn);
		if (MemMapHeight > 0.9 * (Rect.bottom - Rect.top))
		{ 
			MemMapHeight = 0.9 * (Rect.bottom - Rect.top);
			MemMapWidth = MemMapHeight * (CurView->NewBounds.xmx - CurView->NewBounds.xmn)/
									 	 (CurView->NewBounds.ymx - CurView->NewBounds.ymn);			
		}
		SetDimRect.left = (Rect.left + Rect.right) /2 - MemMapWidth/2;
		SetDimRect.bottom = (Rect.bottom + Rect.top) /2 + MemMapHeight/2;
		SetDimRect.right = Rect.left + MemMapWidth;
		SetDimRect.top = Rect.bottom - MemMapHeight;
		  
		CurView->WindowZoomedToOrtho = FALSE;
		CurView->WindowIsZoomed = TRUE; 
	    SetBounds (CurView->hWnd,NULL);
		SaveCurView (0);
		PaintMap (CurView->hWnd,CurView->hDC,TRUE);  
		SaveCurView (1);
	    hPoints = GSSiGlobAlloc (GMEM_MOVEABLE,(long)PickList[0].NumPoints*sizeof(DPOINT)); 
	    pPoints = GlobalLock (hPoints);
		Nump = GetPickItemPoints (0,FALSE,&pPoints); 
		GlobalUnlock (hPoints);  
		if (!Nump)
			return FALSE;
	    pPoints = GlobalLock (hPoints);
		DBoundsInit (&MnMx); 
		while (Nump--)
		{
			ConvertCoord(pPoints,1,3);   
			AddToMinMaxD (&MnMx,pPoints++);
		} 
		GSSiGlobUlFree (&hPoints);
		InflateBounds (&MnMx,Offset);
		MnMx.xmn = floor (MnMx.xmn / Res) * Res;
		MnMx.xmx = ceil (MnMx.xmx / Res) * Res;
		MnMx.ymn = floor (MnMx.ymn / Res) * Res;
		MnMx.ymx = ceil (MnMx.ymx / Res) * Res;
		width = IDNINT((MnMx.xmx - MnMx.xmn) / Res) + 1; 
		width += width%4;
		height = IDNINT((MnMx.ymx - MnMx.ymn) / Res) + 1;   
		height += height%4;  
		
		_fstrcpy (BPWName,Name);
		if ((lpDot=_fstrrchr(BPWName,'.')))
			*lpDot = 0;
		_fstrcat (BPWName,".BPW");
		FidBPW = GSSiOpenFile (BPWName,&OFStruct,OF_CREATE); 
		SetDlgItemText (hWndDlg,IDC_TOT_ITEMS,OFStruct.szPathName);
		sprintf (str,"%f",Res);
		fputstring (str,FidBPW);
		fputstring ("0",FidBPW);
		fputstring ("0",FidBPW);
		sprintf (str,"%f",-Res);
		fputstring (str,FidBPW);
		sprintf (str,"%f",MnMx.xmn);
		fputstring (str,FidBPW);
		sprintf (str,"%f",MnMx.ymx);
		fputstring (str,FidBPW);
		_lclose (FidBPW);
		
//		width = height = 4;
		XFROM[0] = MnMx.xmn;
		XFROM[1] = MnMx.xmn;
		XFROM[2] = MnMx.xmx;
		XFROM[3] = MnMx.xmx; 
		YFROM[0] = MnMx.ymn;
		YFROM[1] = MnMx.ymx;
		YFROM[2] = MnMx.ymx;
		YFROM[3] = MnMx.ymn;
		for (i=0;i<4;i++)
		{
			DPoint.x = XFROM[i];
			DPoint.y = YFROM[i];
			ConvertCoord (&DPoint,3,1); 
			XTO[i] = DPoint.x;
			YTO[i] = DPoint.y;
		}
		
		hTran = STRAN2 (XFROM,YFROM,XTO,YTO,4,&RSQMIN,1);  
		
        nBytesPerPel = pDibInfoOut->bmiHeader.biBitCount/8;
        pDibInfoOut->bmiHeader.biWidth = width;
        pDibInfoOut->bmiHeader.biHeight = height;
		OutLen = pDibInfoOut->bmiHeader.biWidth *(long)nBytesPerPel;
		OutLen += OutLen%4;
		pDibInfoOut->bmiHeader.biSizeImage = (long)pDibInfoOut->bmiHeader.biHeight * OutLen;

		FidBM = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
		bmfHead.bfType = 19778;
		bmfHead.bfSize = sizeof(BITMAPFILEHEADER) + HeadLen + pDibInfoOut->bmiHeader.biSizeImage;
		bmfHead.bfReserved1 = 0;
		bmfHead.bfReserved2 = 0;
		bmfHead.bfOffBits = HeadLen + sizeof(BITMAPFILEHEADER);
		_lwrite (FidBM,(char *)&bmfHead,sizeof(BITMAPFILEHEADER));
		_lwrite (FidBM,(char *)pDibInfoOut,HeadLen); 
        hRow = GSSiGlobAlloc (GMEM_MOVEABLE,OutLen); 
        DBoundsInit (&CurView->WBounds);
		for (irow = 0, Y=MnMx.ymn; irow <pDibInfoOut->bmiHeader.biHeight; irow++, Y+=Res)
		{ 
			pRow = GlobalLock (hRow); 
			if (irow == 220)
				ii=1;
			for (icol = 0, X=MnMx.xmn; icol < pDibInfoOut->bmiHeader.biWidth; icol++, X+=Res)
			{   
				
			    TRANS2 (X,Y,&BasePoint.x,&BasePoint.y,hTran);  
				GetColorVal (pRow,&BasePoint);
				pRow += nBytesPerPel;
	            if (!ContinueProcessing)
	            {
	            	GlobalUnlock (hRow);
	            	goto Exit;          
	            }
			}
			GlobalUnlock (hRow);
			pRow = GlobalLock (hRow); 
			if (_lwrite (FidBM,pRow,(UINT)OutLen) != OutLen)
			{
			    MessageBox(0,"Error writing bitmap - disk may be full",
			               "Fatal Write Error",MB_OK|MB_ICONQUESTION|MB_TASKMODAL);
				GlobalUnlock (hRow);
		        GlobalFree (hRow);
			    _lclose (FidBM);
			    return FALSE;
			} 
			GlobalUnlock (hRow);
            PctBox (GetDlgItem(hWndDlg,IDC_STATUS), pDibInfoOut->bmiHeader.biHeight, irow,0); 
            if (!ContinueProcessing)
            	goto Exit;
		}
		rtn = TRUE; 
Exit:
		_lclose (FidBM);
		GSSiGlobFree (&hTran);  
        GSSiGlobFree (&hRow); 
        while (NumMemMaps--)
        {
			i=DeleteObject (hMemMaps[NumMemMaps]);
        	CloseTRANS2 (&MemMapTrans[NumMemMaps]);
        }
        PctBox (GetDlgItem(hWndDlg,IDC_STATUS2),NumItems,Item++,0); 
		CloseOrthos (); 
    } 
	SetDimensions=FALSE;
    GSSiGlobUlFree (&hDibInfoOut);
	CloseOrthos (); 
	if (MemMap)
	{
		SelectObject(hdcMemMap, hbmpOld);
		DeleteObject (hMemBitmap);
		DeleteDC (hdcMemMap);  
	}
	MemMap = FALSE;
    return rtn;
}

BOOL ExportData (HWND hWnd,short Type)
{
    FARPROC lpfnMIF_OUTPUTMsgProc, lpfnBMP_OUTPUTMsgProc;
    BOOL    nRc; 
    
    EXType = Type;  
    
    if (Type == ORTHBMP)
    {   
	    lpfnBMP_OUTPUTMsgProc = MakeProcInstance((FARPROC)BMP_OUTPUTMsgProc, hInst);
	    nRc = DialogBox(hInst, (LPSTR)"BMP_OUTPUT", hWnd, lpfnBMP_OUTPUTMsgProc);
	    FreeProcInstance(lpfnBMP_OUTPUTMsgProc);  
    	return nRc;
    }

    lpfnMIF_OUTPUTMsgProc = MakeProcInstance((FARPROC)MIF_OUTPUTMsgProc, hInst);
    nRc = DialogBox(hInst, (LPSTR)"MIF_OUTPUT", hWnd, lpfnMIF_OUTPUTMsgProc);
    FreeProcInstance(lpfnMIF_OUTPUTMsgProc);  
    *AutoExportName = 0;
    return nRc;
}

BOOL FAR PASCAL MIF_OUTPUTMsgProc(HWND hWndDlg, WORD Message, WPARAM wParam, LPARAM lParam)
{ 
    short nItems, i, Version=1, NumFields;  
    char    File[128],  ExtID[32], Name[128], str[128];
    LPINT   lpItems;    
    HFILE   Fid;
    OFSTRUCT    OFStruct;  
    BOOL    False=FALSE;  
    short     IDC_FieldName=IDC_FIELDS,UnitsOpt;
    LPSTR   vbar; 
    char    txt[128], txt2[128], project[34]; 
    BTHEAD  BTHead;
    long    NumItems;
    static	char	Ext[6], SaveExt[8],DExt[6]; 
    static	short   Filter, FileVarID, OutVarID;  
    static	BOOL	FileIsOpen;
    
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam,IDC_SQL,
                     SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, &IDC_FieldName,1,
                     MIFOutDataFile, &MIFOUTDataFileType, &MIFOuthDB, &False,FALSE))  return TRUE;
 switch(Message)
   {
    case WM_INITDIALOG:
         /* initialize working variables                                */   
    
    FileIsOpen = FALSE;            
    switch (EXType)
    {
        case MIF:
            Filter = IDS_FILTERMIF;
            _fstrcpy (Ext,".MIF");
            _fstrcpy (DExt,".MID");
            _fstrcpy (SaveExt,".MIO"); 
            FileVarID = IDS_FILEMIO;  
            OutVarID = IDS_FILEMIF;
            break;
        case SHP: 
        	SetWindowText (hWndDlg,"Export to SHP Format");
            Filter = IDS_FILTERSHP;
            _fstrcpy (Ext,".SHP");
            _fstrcpy (DExt,".DBF");
            _fstrcpy (SaveExt,".SFO"); 
            FileVarID = IDS_FILESFO;
            OutVarID = IDS_FILESHP;
			ShowWindow (GetDlgItem(hWndDlg,IDC_SFTTITLE),SW_SHOW);
			ShowWindow (GetDlgItem(hWndDlg,IDC_SHAPETYPE),SW_SHOW);
            break; 
        case TXT:
            Filter = IDS_FILTERTEXT;
            _fstrcpy (Ext,".TXT");
            _fstrcpy (DExt,".TXT");
            _fstrcpy (SaveExt,".TXO"); 
            FileVarID = IDS_FILETXO;
            OutVarID = IDS_FILETXT;
			ShowWindow (GetDlgItem(hWndDlg,IDC_MFTITLE),SW_HIDE);
			ShowWindow (GetDlgItem(hWndDlg,IDC_MID_FILE),SW_HIDE);
            break; 
    }
         
        SendDlgItemMessage (hWndDlg,IDC_SHAPETYPE,CB_ADDSTRING,0,(LPARAM)"Point");
        SendDlgItemMessage (hWndDlg,IDC_SHAPETYPE,CB_ADDSTRING,0,(LPARAM)"Arc");
        SendDlgItemMessage (hWndDlg,IDC_SHAPETYPE,CB_ADDSTRING,0,(LPARAM)"Polygon");
        if (!hHighlight)
        { 
    NoItems:
            MessageBox( GetFocus(),"No items highlighted","Error", MB_OK);
            PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
            break;
        }  
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees * 1000000");
         _fstrcpy (str,"*.CVT");
         DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE);   
         GetBTHeader (hHighlight,&BTHead); 
         NumItems = BTHead.BT_NUMRECS;  
         if (!NumItems) goto NoItems;
         sprintf(txt,"%ld items selected",NumItems);
         SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,txt);  
         if (*AutoExportName)
		 	PostMessage(hWndDlg, WM_COMMAND, IDC_LOAD, 0L);
    case GSSI_REINITDIALOG:
         SetDlgItemText(hWndDlg,IDC_SQL,MIFOutSQL);
         if (MIFOutFields)
         {
             lpItems = (LPINT)GlobalLock(MIFOutFields);
             nItems = *lpItems++;
         
             for (i=0;i<nItems;i++,lpItems++)  
                SendDlgItemMessage(hWndDlg,IDC_FIELDS, LB_SETSEL, TRUE,
                                       MAKELPARAM(*lpItems,0)); 
             GlobalUnlock (MIFOutFields);
         }  
         if (FileIsOpen && *AutoExportName)
	         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
         
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
#if WIN32
         switch(LOWORD(wParam))
#else
         switch(wParam)
#endif
           { 
            case IDC_OPEN_DB:
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),TRUE); 
                 break;
                      
            case IDC_SETSQL:      
            {    
                 HANDLE hMem;
                 LPSTR  lpStr, lpWhere;
                 char   SymName[10];
                 
                 hMem = GSSiGlobAlloc (GHND,4096);
                 lpStr = GlobalLock (hMem);
                 if (GetSQLWhereClause (hWndDlg, MIFOuthDB, lpStr))
                 {
                 	SetDlgItemText (hWndDlg,IDC_SQL,lpStr);    
                 }
                 GlobalUnlock (hMem);
                 GlobalFree (hMem);
                 break;
            }
            
             case IDC_SELECT_OUTPATH:
            {   
                LPSTR   lpDot;   
                char    Name[128];
                if (GetSaveName2 (hWndDlg,Name,Filter,Ext,OutVarID))
                {
                    SetDlgItemText (hWndDlg,IDC_MIF_FILE,Name);
    SetOutDB:       
    				GetDlgItemText (hWndDlg,IDC_MIF_FILE,Name,sizeof(Name));
                    if ((lpDot=_fstrrchr (Name,'.')))
                    {
                        *lpDot = 0;
                        _fstrcat (Name,DExt);
                        SetDlgItemText (hWndDlg,IDC_MID_FILE,Name);
                    }                           
                }
            }
                break;
             
             case IDC_MIF_FILE:
                if (HIWORD(lParam) ==  EN_KILLFOCUS)
                	goto SetOutDB;
             	break;
             	   
             case IDC_LOAD:
             	 if (!*AutoExportName)
             	 { 
	                 if (!GetFileName2(hWndDlg,File,SaveExt,FileVarID))
	                 	break;   
	             }
	             else
	             	_fstrcpy (File,AutoExportName);  
                 {
                    LPSTR   lpID, lpPW, lpDot;
                    
                    if (MIFOutFields)
                        GlobalFree(MIFOutFields);
                    CloseDataFile (FALSE,&MIFOuthDB);
                    Fid = GSSiOpenFile (File,&OFStruct,OF_READ);
                    _lread (Fid,&Version,2); 
                    _lread (Fid,Name,sizeof(Name)); 
                    SetDlgItemText (hWndDlg,IDC_MIF_FILE,Name);
                    if ((lpDot=_fstrrchr (Name,'.')))
                    {
                        *lpDot = 0;
                        _fstrcat (Name,DExt);
                        SetDlgItemText (hWndDlg,IDC_MID_FILE,Name);
                    }
                    _lread (Fid,&MIFOutDataFile,sizeof(MIFOutDataFile));
                    _lread (Fid,&MIFOutSQL,sizeof(MIFOutSQL));
                    _lread (Fid,&nItems,sizeof(short));
                     MIFOutFields = GSSiGlobAlloc(GHND,nItems*2+2);
                     lpItems = (LPINT)GlobalLock(MIFOutFields);
                     *lpItems = nItems;
                     lpItems++;       
                     _lread (Fid,lpItems,nItems*sizeof(short));
                     GlobalUnlock(MIFOutFields);
                     _lread (Fid,str,16);
                     SetDlgItemText (hWndDlg,IDC_SHAPETYPE,str);
	                 _lread (Fid,project,sizeof(project));
	                 SetDlgItemText (hWndDlg,IDC_PROJECTION,project);
	                 _lread (Fid,&UnitsOpt,2);
				     SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_SETCURSEL,UnitsOpt,NULL); 
                     _lclose (Fid); 
                     lpID = _fstrstr (MIFOutDataFile,";UID="); 
                     lpPW = _fstrstr (MIFOutDataFile,";PWD="); 
                     if (lpID && lpPW)
                     {  
                        vbar = _fstrchr (lpID,'|');
                        *lpID = 0; 
                        *lpPW = 0;
                        if (!_fstrncmp (MIFOutDataFile,"ODBC|",5))
                            _fstrcpy(CurODBCFile,&MIFOutDataFile[5]); 
                        _fstrcpy (txt2,MIFOutDataFile);
                        if (vbar)
                        {
                            _fstrcat (txt2,vbar);
                            *vbar = 0; 
                        }
                        lpID+=5;
                        lpPW+=5;
                        SetODBCPassword (lpID,lpPW);  
                        _fstrcpy (MIFOutDataFile,txt2);
                     }
                     FileIsOpen = TRUE;
                   	 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
                 }
                 
            break;
             case IDC_SAVE:
                 if (!GetSaveName2 (hWndDlg,File,0,SaveExt,FileVarID)) 
                    break;  
                 GetDlgItemText (hWndDlg,SV_DATABASE_LIST ,MIFOutDataFile,sizeof(MIFOutDataFile));
                 if (!_fstrncmp (MIFOutDataFile,"ODBC|",5))
                 { 
                    vbar = _fstrchr (&MIFOutDataFile[5],'|');
                    if (!vbar) break;
                    *vbar = 0;   
                    _fstrcpy (txt,&MIFOutDataFile[5]);
                    AddPWtoODBCFile (txt); 
                    sprintf (txt2,"ODBC|%s|%s",txt,++vbar); 
                    _fstrcpy (MIFOutDataFile,txt2);
                 } 
                    
                 nItems=SendDlgItemMessage(hWndDlg,IDC_FIELDS,
                                           LB_GETSELCOUNT,
                                           0,
                                           0);
                 if (!nItems)
                 {
                    MessageBox( GetFocus(), "Error","No Fields Selected", MB_OK);
                    break;
                 }  
                 if (MIFOutFields)
                    GlobalFree(MIFOutFields);
                 MIFOutFields = GSSiGlobAlloc(GHND,nItems*2+2);
                 lpItems = (LPINT)GlobalLock(MIFOutFields);
                 *lpItems = nItems;
                 lpItems++;       
                 SendDlgItemMessage(hWndDlg,IDC_FIELDS,
                                           LB_GETSELITEMS,
                                           nItems,
                                           (LPARAM)lpItems); 
                 GlobalUnlock(MIFOutFields);
                 CloseDataFile (FALSE,&MIFOuthDB);      
                 GetDlgItemText(hWndDlg,IDC_SQL,MIFOutSQL,sizeof(MIFOutSQL));
                 if (wParam == IDOK)
                     EndDialog(hWndDlg, TRUE);
                 else
                 {
                    Fid = GSSiOpenFile (File,&OFStruct,OF_CREATE);
                    _lwrite (Fid,(char *)&Version,2);
	                 GetDlgItemText (hWndDlg,IDC_MIF_FILE,Name,sizeof(Name));
	                 _lwrite (Fid,Name,sizeof(Name)); 
                    _lwrite (Fid,(char *)&MIFOutDataFile,sizeof(MIFOutDataFile));
                    _lwrite (Fid,(char *)&MIFOutSQL,sizeof(MIFOutSQL));
                     lpItems = (LPINT)GlobalLock(MIFOutFields);  
                     _lwrite (Fid,(char *)lpItems,(*lpItems+1)*sizeof(short)); 
                     GlobalUnlock(MIFOutFields);
                     GetDlgItemText (hWndDlg,IDC_SHAPETYPE,str,16);
                     _lwrite (Fid,str,16);
	                 GetDlgItemText (hWndDlg,IDC_PROJECTION,project,sizeof(project));
	                 _lwrite (Fid,project,sizeof(project));
				     UnitsOpt=SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_GETCURSEL,NULL,NULL); 
	                 _lwrite (Fid,&UnitsOpt,2); 
                    _lclose (Fid);
                 }
                 break; 

            case IDOK: 
            {
                HFILE   FidMIF, FidMID, FidSHP, FidSHPIdx;
                char    IdxName[128];   
                OFSTRUCT    OFStruct;  
                short     pos;  
                long    CurItem=0, iref, RecNum=0, Index=0, NumDBFRecs=0;
                LPTHEME pTheme;                                        
                HIGHLIGHTDATA   HighlightData;  
                short     nareas,numpoints;
                DPOINT  CP; 
                BOOL    First;
                LPSTR	lpDot; 
                char    SQL[256],Type[32],DBName[128],SymName[34]; 
                HANDLE	hOutRec = GSSiGlobAlloc (GMEM_MOVEABLE,UINT_MAX);
                LPSTR	OutRec = GlobalLock (hOutRec);
                SHPPOLYHEADER   SHPPolyHeader;  
                SHPHEADER   SHPHeader;  
                SHPRECHEADER SHPRecHeader; 
                LPOPENFILEDATA  FilePtr, FilePtrATT;
                LPOPENSQLDATA   SQLPtr, SQLPtrATT;
                HANDLE  hSQL;
                short IDB; 
                double	coordcvt=1;
                long	SHPOff, NumMiss;
                LPFIELDINFO	lpFldInfo;  
                short	ShapeType;
                HANDLE	hIndex=0;
                LPLONG	pIndex;  
                LPSHORT	pPolyParts;
                
                CloseDataFile (FALSE,&MIFOuthDB);  
                GetDlgItemText (hWndDlg,IDC_SHAPETYPE,str,sizeof(str));
                if (!_fstricmp (str,"Point"))    
                	ShapeType = 1;
                else if (!_fstricmp (str,"Arc"))    
                	ShapeType = 3;
                else if (!_fstricmp (str,"Polygon"))    
                	ShapeType = 5;
                GetDlgItemText (hWndDlg,IDC_SQL,SQL,sizeof(SQL));
                if (!OpenDataFile (MIFOutDataFile,SQL,BT_READ,&MIFOuthDB))
                    goto Exit2;   
                nItems=SendDlgItemMessage(hWndDlg,IDC_FIELDS,
                                           LB_GETSELCOUNT,
                                           0,
                                           0);
                if (!nItems)
                {
                    MessageBox( GetFocus(),"No Fields Selected", "Error", MB_OK);
                    goto Exit;
                }  
                if (MIFOutFields)
                    GlobalFree(MIFOutFields);
                if (!GetDlgItemText (hWndDlg,IDC_PROJECTION,project,sizeof(project)))
                {
                    MessageBox(GetFocus(),"No output projection set", 0,MB_ICONEXCLAMATION|MB_OK);
                    goto Exit;
                }
                if ((lpDot=_fstrrchr(project,'.')))
                    *lpDot = 0;
                SetGlobalValue("%ALT_PROJECTION",project);
			    ConvertCoordClose ();
				ConvertCoordInit();
                GetDlgItemText (hWndDlg,IDC_UNITS,str,sizeof(str));
                if (*str)
                { 
                    if (!_fstrcmp(str,"Degrees * 1000000"))
                        coordcvt = 0.000001; 
                    else if (!_fstrcmp(str,"Feet"))
                        PRJ_UNITS[3] = 1;
                    else if (!_fstrcmp(str,"Meters"))
                        PRJ_UNITS[3] = 2;
                }
                else
                {
                    MessageBox(GetFocus(),"Units field not set", 0,MB_ICONQUESTION|MB_OK);
                    goto Exit;
                }
                MIFOutFields = GSSiGlobAlloc(GHND,nItems*2+2);
                lpItems = (LPINT)GlobalLock(MIFOutFields);
                SendDlgItemMessage(hWndDlg,IDC_FIELDS,
                                           LB_GETSELITEMS,
                                           nItems,
                                           (LPARAM)lpItems); 
                GlobalUnlock(MIFOutFields);
            
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT2),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_LOAD),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_SAVE),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE); 
                DisableHalt = TRUE;  
				ContinueProcessing=TRUE;  
				Processing = TRUE;
                GetBTHeader (hHighlight,&BTHead); 
                NumItems = BTHead.BT_NUMRECS;  
                GetDlgItemText (hWndDlg,IDC_MIF_FILE,Name,sizeof(Name));
                FidMIF = GSSiOpenFile (Name,&OFStruct,OF_CREATE); 
                FidSHP = FidMIF;
                GetDlgItemText (hWndDlg,IDC_MID_FILE,Name,sizeof(Name));
                switch (EXType)
                {
                    case MIF:
                        FidMID = GSSiOpenFile (Name,&OFStruct,OF_CREATE); 
                        fputstring ("Version 2",FidMIF);
                        fputstring ("Delimiter \",\"",FidMIF);
                        fputstring ("Columns 1",FidMIF);
                        fputstring ("  SoilType Char(4)",FidMIF);
                        fputstring ("Data",FidMIF);
                        fputstring ("",FidMIF);  
                    break;
                    
                    case TXT:
                        FidMID = GSSiOpenFile (Name,&OFStruct,OF_CREATE);  
                        _fstrcpy (OutRec,"\"INT_REFNO\",\"Symbol\",\"X\",\"Y\"");
                    break;
                    
                    case SHP:
                        hSQL = 0;
                        if (!OpenDataFile (Name,"",BT_READ,&hSQL))
                        {
                        	MessageBox (GetFocus(),"The ODBC driver 'DBF' cannot be opened",NULL,MB_ICONEXCLAMATION);
                        	goto Exit;
                        } 
                        ExpandText (Name);  
                        _fstrcpy (DBName,Name);
                        _fstrcpy (IdxName,Name);
                        lpDot = _fstrrchr (IdxName,'.');    
                        *lpDot = 0;
                        _fstrcat (IdxName,".shx");
	                    GSSiRemove (Name);  
	                    GSSiRemove (IdxName);  
	                    sprintf (OutRec,"create table %s (",DBName); 
		                FidSHPIdx = GSSiOpenFile (IdxName,&OFStruct,OF_CREATE); 
                }
	                  
                SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL);
                FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
                IDB = FilePtr->FileHandle; 
                SQLPtrATT = (LPOPENSQLDATA)GlobalLock (MIFOuthDB);
                FilePtrATT = (LPOPENFILEDATA)GlobalLock (SQLPtrATT->OFHandle); 
                lpItems = (LPINT)GlobalLock (MIFOutFields); 
                for (i=0;i<nItems;i++,lpItems++) 
                {
                    SendDlgItemMessage(hWndDlg,IDC_FIELDS,
                                               LB_GETTEXT,
                                               *lpItems,(LPARAM)Name);
                    if(i)
                    	_fstrcat (OutRec,",");
                    lpFldInfo = &FilePtrATT->FldInfo;
                    lpFldInfo += *lpItems;
				    switch (lpFldInfo->type)
				    {   
				        default: 
				        case SQL_CHAR: 
				        case SQL_VARCHAR:
				        case BT_RIGHT_CHAR:
				        case BT_CHAR:
				            sprintf (Type,"char(%i)",lpFldInfo->length);
				        break;
						        
				        case SQL_INTEGER:
				        case SQL_SMALLINT:
				        case BT_INTEGER:
				            if (lpFldInfo->length == 2)
				                _fstrcpy (Type,"smallint");
				            else
				                _fstrcpy (Type,"integer");
				        break;
						        
				        case SQL_NUMERIC:
				        case SQL_FLOAT:
						case SQL_REAL:
						case SQL_DOUBLE:
				        case BT_REAL:
				            _fstrcpy (Type,"float");
				        break;
				     }
					switch (EXType)
	                {
	                    case MIF:
	                    break;
									                    
	                    case TXT: 
                    		sprintf (_fstrchr(OutRec,0),",\"%s\"",lpFldInfo->name);
	                    break;
									                    
	                    case SHP:
                    		sprintf (_fstrchr(OutRec,0),"\"%s\" %s",lpFldInfo->name,Type);
					}        
                }
				GlobalUnlock (MIFOutFields); 
        		GlobalUnlock (SQLPtrATT->OFHandle);
				GlobalUnlock (MIFOuthDB);
				switch (EXType)
                {
                    case MIF:
                    break;
									                    
                    case TXT: 
                		fputstring (OutRec,FidMID);
                    break;
									                    
                    case SHP:
	                    _fstrcat (OutRec,")");
                        if (!ExternalSQLDirect (IDB, OutRec))
                        {   
                        	MessageBox (GetFocus(),"Unable to create DBF file",NULL,MB_ICONEXCLAMATION);
                          	goto Exit;
                        }
                        _lwrite (FidSHP,(char *)&SHPHeader,sizeof(SHPHeader)); 
                        _lwrite (FidSHPIdx,(char *)&SHPHeader,sizeof(SHPHeader)); 
                        _fmemset (&SHPHeader,0,sizeof(SHPHeader));
                        SHPHeader.Xmin = DBL_MAX;   
                        SHPHeader.Xmax = -DBL_MAX;
                        SHPHeader.Ymin = DBL_MAX;
                        SHPHeader.Ymax = -DBL_MAX;
				}        
                        
                SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,"Creating extract file");
                pos = BT_FIRST;
                NumMiss=0; 
                while (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData)&&ContinueProcessing)
                {   
                	long	ii;
                	
                	if (iref == 1851)
                		ii=1;
                    pos = BT_NEXT; 
                    PickList[0]=HighlightData.PD;
                    if (EXType == SHP &&
                    	((PickList[0].Type == 2 && ShapeType != 3) ||
                    	(PickList[0].Type == 3 && ShapeType != 5) ||
                    	((PickList[0].Type == 1 || PickList[0].Type == 4)&& ShapeType == 1)))
                        goto NextHlt; 
                    SetIntRefno (iref);
                    SetUDIValue (HighlightData.PD.Prefix,HighlightData.PD.UDI);     
                    SetGlobalValue ("%PREFIX",HighlightData.PD.Prefix);  
                    lpItems = (LPINT)GlobalLock (MIFOutFields); 
                    switch (EXType)
                    {
                        case MIF:
                        break;
                            
                        case SHP: 
               		    	sprintf (OutRec,"INSERT INTO %s VALUES (",DBName); 
                        break;
                        case TXT:
							GetSymbolName (HighlightData.PD.Desc, SymName,NULL,1,NULL);
							ConvertCoord(&HighlightData.PD.BeginPoint,1,3); 
                        	sprintf (OutRec,"%ld,\"%s\",%f,%f",iref,SymName,
                        									   HighlightData.PD.BeginPoint.x,
                        									   HighlightData.PD.BeginPoint.y);
                        break;
                    }
                      
                    for (i=0;i<nItems;i++,lpItems++) 
                    {
                        SendDlgItemMessage(hWndDlg,IDC_FIELDS,
                                                   LB_GETTEXT,
                                                   *lpItems,(LPARAM)Name); 
                     
                        if (GetValFromOpenFiles (Name,txt) < 0) 
//                        	*txt = 0;
                        {   
                            GlobalUnlock (MIFOutFields);   
                            NumMiss++;
                            goto NextHlt;  
                        }  
                        if (i)
                            _fstrcat (OutRec,",");
                        switch (EXType)
                        {
                            case MIF:
                            _fstrcat (OutRec,"\"");
                            _fstrcat (OutRec,txt);
                            _fstrcat (OutRec,"\""); 
                            break;
                            
                            case SHP: 
		                    lpFldInfo = &FilePtrATT->FldInfo;
		                    lpFldInfo += *lpItems;
						    switch (lpFldInfo->type)
						    {   
						    	case 0:
						        case SQL_CHAR: 
						        case SQL_VARCHAR:
						        case BT_RIGHT_CHAR:
						        case BT_CHAR:  
						        	Strip (txt,'\'');
                                	sprintf (_fstrchr(OutRec,0),"'%s'",txt); 
                                	break;
                                default:
                                	_fstrcat (OutRec,txt);
                                break;
                            }
                            break;
                            case TXT: 
		                    lpFldInfo = &FilePtrATT->FldInfo;
		                    lpFldInfo += *lpItems;
						    switch (lpFldInfo->type)
						    {   
						    	case 0:
						        case SQL_CHAR: 
						        case SQL_VARCHAR:
						        case BT_RIGHT_CHAR:
						        case BT_CHAR:
                                	sprintf (_fstrchr(OutRec,0),",\"%s\"",txt); 
                                	break;
                                default:
                                	sprintf (_fstrchr(OutRec,0),",%s",txt); 
                                break;
                            }
                            break;
                        }
                    }  
                    GlobalUnlock (MIFOutFields);   
                    switch (EXType)
                    {
                        case MIF:
                            fputstring (OutRec,FidMID);  
                        break;
                        case TXT:
                            fputstring (OutRec,FidMID);  
                            goto NextHlt;
                        break;
                        case SHP: 
                           	_fstrcat (OutRec,")");
                            if (!ExternalSQLDirect (IDB, OutRec))
                            {
                            	ContinueProcessing=FALSE;
                            	goto NextHlt;
                            } 
                            NumDBFRecs++;
                            	
                        break;
                    } 
                    
                    if ((PickList[0].Type == 2 && ShapeType == 3) ||
                    	(PickList[0].Type == 3 && ShapeType == 5))
                    {   
                        pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME);
                        CurView->PassID = 1;
                        ProcessPickedItem (0,FALSE);                
                        DeleteTheme (pTheme);
                        if (hSavePoly)
                        {   LPMNMXCORD lpRect;
                            short   nPnts,i; 
                            long	Totp;
                            HPDPOINT    lpDpoint;
                            DPOINT  FirstPoint;

                            SHPPolyHeader.Xmin = DBL_MAX;   
                            SHPPolyHeader.Xmax = -DBL_MAX;
                            SHPPolyHeader.Ymin = DBL_MAX;
                            SHPPolyHeader.Ymax = -DBL_MAX; 
                            if (hSavePolyParts)
                            {
                            	pPolyParts = GlobalLock (hSavePolyParts); 
                            	SHPPolyHeader.NumParts = *pPolyParts++; 
                            	hIndex = GSSiGlobAlloc (GHND,SHPPolyHeader.NumParts*4);
                            	pIndex = GlobalLock (hIndex);
                            	i = SHPPolyHeader.NumParts; 
	                            nareas=SHPPolyHeader.NumParts;  
                            	Totp = 0;
                            	while (i--)
                            	{   
                            		*pIndex++ = Totp;
                            		Totp += *pPolyParts++;
                            	}
                            	GlobalUnlock (hIndex);
                            }
                            else 
                            {
                            	hIndex = 0; 
                            	SHPPolyHeader.NumParts = 1;
	                            nareas=1;  
                            }
                            
                            CP.x=0;
                            CP.y=0;
                            nPnts = nSavePoly; 
                            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
                            lpRect++;
                            lpDpoint = (LPDPOINT) lpRect;
                            while (nPnts--)
                            {
                                CP.x+=lpDpoint->x;
                                CP.y+=lpDpoint++->y;
                            }  
                            GlobalUnlock (hSavePoly);
                            nPnts = nSavePoly; 
                            if (ShapeType == 5) 
                            	numpoints = nPnts - (SHPPolyHeader.NumParts - 1); 
                            else
                            	numpoints = nPnts; 
                            SHPPolyHeader.NumPoints = numpoints;
                            
                            CP.x/=nPnts;
                            CP.y/=nPnts; 
                            switch (EXType)
                            {
                                case MIF:
                                    sprintf (txt,"Region %i",nareas);  
                                    fputstring (txt,FidMIF);
                                    sprintf (txt,"%4i",nPnts+1);  
                                    fputstring (txt,FidMIF);  
                                break;
                                case SHP:
                                break;
                            }
                            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
                            lpRect++;
                            lpDpoint = (LPDPOINT) lpRect;  
                            First = TRUE;
                            while (nPnts--)
                            {   
                                double x,y;
                                
                                ConvertCoord(lpDpoint,1,3); 
                                if (First)
                                    FirstPoint = *lpDpoint;
                                First = FALSE;   
                                x=lpDpoint->x;
                                y=lpDpoint->y;  
                                SHPPolyHeader.Xmin = min (x,SHPPolyHeader.Xmin);   
                                SHPPolyHeader.Xmax = max (x,SHPPolyHeader.Xmax);
                                SHPPolyHeader.Ymin = min (y,SHPPolyHeader.Ymin);   
                                SHPPolyHeader.Ymax = max (y,SHPPolyHeader.Ymax);
                                
                                lpDpoint++;  
                                switch (EXType)
                                {
                                    case MIF:
                                        sprintf (txt,"%f %f",x,y); 
                                        fputstring (txt,FidMIF);  
                                        break;
                                    case SHP:
                                        break;
                                }
                            } 
                            switch (EXType)
                            {
                                case MIF:
                                    sprintf (txt,"%f %f",FirstPoint.x,FirstPoint.y); 
                                    fputstring (txt,FidMIF);
                                    ConvertCoord(&CP,1,3);   
                                    fputstring ("    Pen (1,2,0)",FidMIF);
                                    fputstring ("    Brush (2,16777088,16777215)",FidMIF);
                                    sprintf (txt,"    Center %f %f",CP.x,CP.y);  
                                    fputstring (txt,FidMIF);
                                     
                                    break;
                                case SHP: 
                                    SHPRecHeader.RecNum = RecNum++;
                                    SHPRecHeader.Length = (sizeof (SHPPolyHeader) + 
                                                           sizeof(long)*SHPPolyHeader.NumParts +
                                                           sizeof(double) *(long)numpoints * 2) / 2;
                                     
                                    flip ((LPSTR)&SHPRecHeader.RecNum,4);
                                    flip ((LPSTR)&SHPRecHeader.Length,4);   
                                    SHPHeader.Xmin = min (SHPHeader.Xmin,SHPPolyHeader.Xmin);   
                                    SHPHeader.Ymin = min (SHPHeader.Ymin,SHPPolyHeader.Ymin);   
                                    SHPHeader.Xmax = max (SHPHeader.Xmax,SHPPolyHeader.Xmax);   
                                    SHPHeader.Ymax = max (SHPHeader.Ymax,SHPPolyHeader.Ymax);   
                                    
			                        SHPOff = _llseek (FidSHP,0,1)/2;
                                    _lwrite (FidSHP,(char *)&SHPRecHeader,sizeof(SHPRecHeader)); 
                                    SHPRecHeader.RecNum = SHPOff;
                                    flip ((LPSTR)&SHPRecHeader.RecNum,4);
                                    _lwrite (FidSHPIdx,(char *)&SHPRecHeader,sizeof(SHPRecHeader)); 
                                    SHPPolyHeader.Type = ShapeType;
                                    _lwrite (FidSHP,(char *)&SHPPolyHeader,sizeof(SHPPolyHeader));
                                    if (hIndex)
                                    {
                                    	pIndex = GlobalLock (hIndex);
                                    	_lwrite (FidSHP,pIndex,4*SHPPolyHeader.NumParts); 
                                    	GlobalUnlock (hIndex);  
                                    	lpDpoint = (LPDPOINT) lpRect;
		                            	pPolyParts = GlobalLock (hSavePolyParts); 
                                        pPolyParts++;
                                        i = SHPPolyHeader.NumParts; 
                                        First=TRUE;
                                        while (i--)
                                        {
											BigWrite (FidSHP,lpDpoint,sizeof(DPOINT)*(long)(*pPolyParts)); 
											lpDpoint+=*pPolyParts++;
											if (First)
												First=FALSE;
											else                                        
												lpDpoint++;
	                                    }
	                                }	
                                    else
                                    { 
                                    	_lwrite (FidSHP,(char *)&Index,4); 
	                                    if (ShapeType == 5) 
	                                    {
		                                    BigWrite (FidSHP,lpRect,sizeof(DPOINT)*(long)(numpoints-1)); 
		                                    _lwrite (FidSHP,(char *)&FirstPoint,sizeof(DPOINT));
		                                }
		                                else 
	                                    {
		                                    BigWrite  (FidSHP,(char *)lpRect,sizeof(DPOINT)*(long)(numpoints)); 
		                                }
                                    }
                                    break;
                            } 
                            GSSiGlobFree (&hIndex);
                            GSSiGlobFree (&hSavePolyParts);
			                GSSiGlobUlFree (&hSavePoly);
                         }
                     }
                     else if (EXType == SHP &&
                     		  ((PickList[0].Type == 1 || PickList[0].Type == 4)&& ShapeType == 1))
                     {
	                    SHPPOINTREC	SHPPointRec;   
	                    DPOINT DPoint;

                        SHPRecHeader.RecNum = RecNum++;
                        SHPRecHeader.Length = (sizeof (SHPPointRec)) / 2;
                                     
                        flip ((LPSTR)&SHPRecHeader.RecNum,4);
                        flip ((LPSTR)&SHPRecHeader.Length,4); 
                        DPoint = PickList[0].BeginPoint;
                        ConvertCoord(&DPoint,1,3);    
                        SHPHeader.Xmin = min (SHPHeader.Xmin,DPoint.x);   
                        SHPHeader.Ymin = min (SHPHeader.Ymin,DPoint.y);   
                        SHPHeader.Xmax = max (SHPHeader.Xmax,DPoint.x);   
                        SHPHeader.Ymax = max (SHPHeader.Ymax,DPoint.y);   
                                    
                        SHPOff = _llseek (FidSHP,0,1)/2;
                        _lwrite (FidSHP,(char *)&SHPRecHeader,sizeof(SHPRecHeader)); 
                        SHPRecHeader.RecNum = SHPOff;
                        flip ((LPSTR)&SHPRecHeader.RecNum,4);
                        _lwrite (FidSHPIdx,(char *)&SHPRecHeader,sizeof(SHPRecHeader)); 
                        SHPPointRec.Type = 1;
                        SHPPointRec.Point = DPoint;
                        _lwrite (FidSHP,&SHPPointRec,sizeof(SHPPointRec));
                     }
                     else
                     	ii=1;
                    
            NextHlt:                    
                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0);
                } 
                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0);
		        SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,"Extract finished");
		        if (NumDBFRecs != RecNum)
		        	MessageBox (hWndDlg,"The number of DBF records does not match the number of SHP records",NULL,MB_ICONEXCLAMATION);  
		        else if (NumMiss && !*AutoExportName)
		        {   
		        	char	Mess[128];
		        	
		        	sprintf (Mess,"%ld records skipped due to missing attributes",NumMiss);
		        	MessageBox (hWndDlg,Mess,"",MB_OK);
		        }
		        
                switch (EXType)
                {   
                	case TXT:
                    case MIF:
                        _lclose (FidMID);
                        break;
                    case SHP: 
                        SHPHeader.FileLength = _llseek (FidSHP,0,2); 
                        SHPHeader.FileLength /= 2; 
                        SHPHeader.FileCode = 9994;
                        SHPHeader.Version = 1000;
                        SHPHeader.ShapeType = ShapeType;
                        _llseek (FidSHP,0,0);
                        flip ((LPSTR)&SHPHeader.FileCode,4);
                        flip ((LPSTR)&SHPHeader.FileLength,4);  
                        _lwrite (FidSHP,(char *)&SHPHeader,sizeof(SHPHeader));  
                        _lclose (FidSHP);    
                        
                        SHPHeader.FileLength = _llseek (FidSHPIdx,0,2); 
                        SHPHeader.FileLength /= 2; 
                        SHPHeader.FileCode = 9994;
                        SHPHeader.Version = 1000;
                        SHPHeader.ShapeType = ShapeType;
                        _llseek (FidSHPIdx,0,0);
                        flip ((LPSTR)&SHPHeader.FileCode,4);
                        flip ((LPSTR)&SHPHeader.FileLength,4);  
                        _lwrite (FidSHPIdx,(char *)&SHPHeader,sizeof(SHPHeader));  
                        _lclose (FidSHPIdx);    
                        
                        if (hSQL)
                        {
                            GlobalUnlock (SQLPtr->OFHandle);
                            CloseDataFile (TRUE, &hSQL); 
			                GetDlgItemText (hWndDlg,IDC_MID_FILE,Name,sizeof(Name)); 
                        } 
                        
                        break;
                }
Exit:
                CloseDataFile (FALSE,&MIFOuthDB);      
Exit2:
				GSSiGlobFree(&MIFOutFields);
                GlobalUnlock (hOutRec);
                GlobalFree (hOutRec); 
                DisableHalt = FALSE;  
                ContinueProcessing = TRUE;   
                Processing = FALSE;
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT2),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_LOAD),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_SAVE),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE); 
                if (*AutoExportName)
                {
	                 CloseDataFile (FALSE,&MIFOuthDB);
	                 EndDialog(hWndDlg, TRUE);
	            }
                break;
            }
            case IDC_EXIT2:     
                 CloseDataFile (FALSE,&MIFOuthDB);
                 EndDialog(hWndDlg, FALSE); 
                 
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)
                     ContinueProcessing=FALSE;
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT2),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDC_LOAD),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SAVE),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE); 
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

