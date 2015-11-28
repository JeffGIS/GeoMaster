#include "graphint.h" 
#include "extrndb.h" 

#include "gmextern.h"

static	double	wfactor, hfactor;   
static	RECT	OldRect;
static HWND	LastWnd=0;
static HWND	CurDlgWnd=0;
static	HWND	hWndEdit; 
static	short	NumControls=0;

void Draw3DFrame(HWND hWnd, int iStyle);


USHORT	AddStringToList (LPSTR str,LPSTR pStrings,LPUSHORT plen)
{
	short l=_fstrlen (str);  
	USHORT	rtn=*plen;
	
	if (!l)
		return 0;
	_fstrcpy (&pStrings[*plen],str);
	(*plen) += l+1;
	return rtn;
}


short GetFieldIDFromName (LPSTR IDName,LPOPENFILEDATA FilePtr,LPSTR Name)
{
	LPFIELDINFO	lpFieldInfo = &FilePtr->FldInfo;    
	short	i,ib = 0, l=_fstrlen (IDName);
	
	if (l)
	{
		if (!_fstrnicmp (IDName,Name,l) && Name[l] == '.')
			ib = l+1;
	}
	for (i=0;i<FilePtr->NumFields;i++) 
		if (!_fstricmp (lpFieldInfo[i].name,&Name[ib]))
			return i;
	return -1;
}
 

BOOL GetFieldValFromSetList (LPSTR List,LPSTR Name,LPSTR Val)
{   
	char	str[70], SaveChr; 
	LPSTR	pLoc, pEnd;
	
	sprintf (str,"%s=",Name);
	pLoc = _fstrstr (List,str);
	if (pLoc)
	{   
		pLoc += _fstrlen (str);
		pEnd = MatchLev (pLoc,',');
		if (!pEnd)
			pEnd = _fstrchr (pLoc,0);
		SaveChr = *pEnd;
		*pEnd = 0;
		if (*pLoc == '\'' && *(pEnd-1) == '\'')
		{
			_fstrcpy (Val,pLoc+1);
			*LastChr(Val) = 0;
		}
		else
			_fstrcpy (Val,pLoc);
		*pEnd = SaveChr; 
		ExpandText (Val);
		return TRUE;
	}
	return FALSE;
}

LONG FAR PASCAL ComboColor(HWND hWnd, UINT uiMsg,
    WORD wParam, LONG lParam)
    {
    WNDPROC lpOrgProc;
    LONG    lRtn = 0;
    int		i; 
    UINT	Prompt; 
    char	str[256];  

    lpOrgProc   = ORIGINALPROC(hWnd);
    switch (uiMsg)
    {   
    	case WM_COMMAND:
    	/*	if (ProcessDynEdit (0,hWnd,wParam,lParam))
    			lRtn = 1;
    		else*/
            	lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	    	break;
    	
    	case WM_NCDESTROY:
        {
	        lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg,(WPARAM) wParam, (LPARAM)lParam);
	        SetWindowLong (hWnd, GWL_WNDPROC, (LONG) lpOrgProc);
	        RemoveProp (hWnd, "PrHI");
	        RemoveProp (hWnd, "PrLO");
	        RemoveProp (hWnd, "GMPr");
	        RemoveProp (hWnd, "GMMr");
        }
        break;
    
    	case WM_NCHITTEST:
	    {   
	        lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	    	if (hWnd != LastWnd)
	    	{
				Prompt = (UINT)GetProp (hWnd, "GMPr");
				SetPromptDlg (Prompt);
				LastWnd = hWnd; 
			}
	    } 
	    break;
	    
	    case WM_RBUTTONDOWN:
	    {
			if (wParam & MK_CONTROL) 
			{
				lRtn = 0; 
				ShowDlgUpdateOptions (hWnd,hWnd);
			}
			else
	        	lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
			Prompt = (UINT)GetProp (hWnd, "GMMr");
			SetPromptDlg (Prompt); 
			LastWnd = hWnd;
	    }
	    break;
	    
	    case WM_RBUTTONUP:
	    {   
	    	LastWnd = 0;
	        lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	    }
    	break;
    	
/*    	case WM_CTLCOLOR:
        {
	        SetBkMode ((HDC) wParam, TRANSPARENT);
	        SetTextColor ((HDC) wParam, RGB (0, 0, 128));
	        lRtn = (LONG)(short)(HBRUSH) GetStockObject (LTGRAY_BRUSH);
        }
        break;*/
        
        case WM_PAINT:
        {
            //Draw3DFrame (hWnd, 2);
            lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
        } 
        break;
    	default:
            lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
        break;
    }

    return (lRtn);
    }
LONG FAR PASCAL ComboColorDynDialog(HWND hWnd, UINT uiMsg,WORD wParam, LONG lParam)
    {
    WNDPROC lpOrgProc;
    LONG    lRtn = 0;
    int		i; 
    UINT	Prompt; 
    char	str[256];  

    lpOrgProc   = ORIGINALPROC(hWnd);
    switch (uiMsg)
    {   
    	case WM_COMMAND:
    		if (ProcessDynEdit (0,hWnd,wParam,lParam))
    			lRtn = 1;
    		else
            	lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	    	break;
    	
    	case WM_NCDESTROY:
        {
	        lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg,(WPARAM) wParam, (LPARAM)lParam);
	        SetWindowLong (hWnd, GWL_WNDPROC, (LONG) lpOrgProc);
	        RemoveProp (hWnd, "PrHI");
	        RemoveProp (hWnd, "PrLO");
	        RemoveProp (hWnd, "GMPr");
	        RemoveProp (hWnd, "GMMr");
        }
        break;
    
    	case WM_NCHITTEST:
	    {   
	        lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	    	if (hWnd != LastWnd)
	    	{
				Prompt = (UINT)GetProp (hWnd, "GMPr");
				SetPromptDlg (Prompt);
				LastWnd = hWnd; 
			}
	    } 
	    break;
	    
	    case WM_RBUTTONDOWN:
	    {
			if (wParam & MK_CONTROL) 
			{
				lRtn = 0; 
				ShowDynDlgUpdateOptions (hWnd,hWnd);
			}
			else
	        	lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
			Prompt = (UINT)GetProp (hWnd, "GMMr");
			SetPromptDlg (Prompt); 
			LastWnd = hWnd;
	    }
	    break;
	    
	    case WM_RBUTTONUP:
	    {   
	    	LastWnd = 0;
	        lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	    }
    	break;
    	
/*    	case WM_CTLCOLOR:
        {
	        SetBkMode ((HDC) wParam, TRANSPARENT);
	        SetTextColor ((HDC) wParam, RGB (0, 0, 128));
	        lRtn = (LONG)(short)(HBRUSH) GetStockObject (LTGRAY_BRUSH);
        }
        break;*/
        
        case WM_PAINT:
        {
            //Draw3DFrame (hWnd, 2);
            lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
        } 
        break;
    	default:
            lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
        break;
    }

    return (lRtn);
    }

/* constants to distinguish "recess" from "raise" */


void Draw3DFrame(HWND hWnd, int iStyle)
    {
    HDC            hDC;
    RECT        r;

    hDC = GetWindowDC (hWnd);
    GetWindowRect (hWnd, &r);
    Draw3DBorder (hDC,&r,iStyle,FALSE);
    ReleaseDC (hWnd, hDC);
    }

void Draw3DBorder(HDC hDC, LPRECT pRect,int inStyle, BOOL DoubleWidth)
    {
    HPEN        hOldPen,
                hLTPen,        /* left & top pen */
                hRBPen;     /* right & bottom pen */  
    int			iStyle=abs(inStyle);
    int			left=0,top=0;
//   char	tmp[256];


//	sprintf (tmp,"%i %i %i %i",pRect->left,pRect->right,pRect->top,pRect->bottom);
//    MessageBox (0,tmp,0,MB_OK);
	if (inStyle < 0)
    {
    	left = pRect->left;
    	top  = pRect->top; 
    }
    switch (iStyle)
        {
        case UP_3D:
            hLTPen = CreatePen (PS_SOLID, 1, RGB (255, 255, 255));
            hRBPen = CreatePen (PS_SOLID, 1, RGB (128, 128, 128));
            break;

        case DOWN_3D:
            hLTPen = CreatePen (PS_SOLID, 1, RGB (128, 128, 128));
            hRBPen = CreatePen (PS_SOLID, 1, RGB (255, 255, 255));
            break;
        case NO_3D:
            hLTPen = CreatePen (PS_SOLID, 1, IconBorderColor);
            hRBPen = CreatePen (PS_SOLID, 1, IconBorderColor);
            break;
        }

    hOldPen = SelectObject (hDC, hLTPen);

    /* draw left */
    MoveToEx (hDC, 0+left, (pRect->bottom - pRect->top)+top,0);
    LineTo (hDC, 0+left, 0+top);

    /* draw top */
    LineTo (hDC, (pRect->right - pRect->left - 1)+left, 0+top);

    SelectObject (hDC, hRBPen);

    /* draw right */
    LineTo (hDC, (pRect->right - pRect->left - 1)+left, (pRect->bottom - pRect->top - 1)+top);

    /* draw bottom */
    LineTo (hDC, 0+left, (pRect->bottom - pRect->top - 1)+top);

    SelectObject (hDC, hOldPen); 
    
    if (DoubleWidth)
    {
	    hOldPen = SelectObject (hDC, hLTPen);
	
	    /* draw left */
	    MoveToEx (hDC, 1+left, (pRect->bottom - pRect->top)-1+top,0);
	    LineTo (hDC, 1+left, 1+top);
	
	    /* draw top */
	    LineTo (hDC, (pRect->right - pRect->left - 1)-1+left, 1+top);
	
	    SelectObject (hDC, hRBPen);
	
	    /* draw right */
	    LineTo (hDC, (pRect->right - pRect->left - 1)-1+left, (pRect->bottom - pRect->top - 1)-1+top);
	
	    /* draw bottom */
	    LineTo (hDC, 1+left, (pRect->bottom - pRect->top - 1)-1+top);
	
	    SelectObject (hDC, hOldPen); 
	}
	
    DeleteObject (hLTPen);
    DeleteObject (hRBPen);
    }

static
LONG Draw3DUpDown(HWND hWnd, unsigned uiMsg,
    WORD wParam, LONG lParam, int UpOrDown)
    {
    WNDPROC lpOrgProc;
    LONG    lRtn = 0; 
    UINT	Prompt;

    lpOrgProc = ORIGINALPROC(hWnd);
    switch (uiMsg)
    {   
    	case WM_COMMAND:
    		if (ProcessDlgEdit (0,hWnd,wParam,lParam))
    			lRtn = 1;
    		else
            	lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	    	break;
    	
    	case WM_NCDESTROY:
        {
	        lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	        SetWindowLong (hWnd, GWL_WNDPROC, (LONG) lpOrgProc);
	        RemoveProp (hWnd, "PrHI");
	        RemoveProp (hWnd, "PrLO");
	        RemoveProp (hWnd, "GMPr");
	        RemoveProp (hWnd, "GMMr");
        } 
        break;
        
        case WM_RBUTTONDOWN:
	    {   
			if (wParam & MK_CONTROL) 
			{
				lRtn = 0; 
				ShowDlgUpdateOptions (hWnd,hWnd);
			}
			else
	        	lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	    	LastWnd = hWnd;
			Prompt = (UINT)GetProp (hWnd, "GMMr");
			SetPromptDlg (Prompt);  
				
	    } 
	    break;
	    
    	case WM_RBUTTONUP:
	    { 
	    	LastWnd = 0;                            
	        lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	    } 
	    break;
	    
    	case WM_NCHITTEST:
	    {   
	        lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);  
	    	if (hWnd != LastWnd)
	    	{   
	    		LastWnd = hWnd;
				Prompt = (UINT)GetProp (hWnd, "GMPr");
				SetPromptDlg (Prompt);
		    }
	    }
	    break;
	    
	    default:
        {
	        lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	
	        //if (uiMsg == WM_PAINT && UpOrDown)
	        //    Draw3DFrame (hWnd, UpOrDown);
        }
        break;
    }

    return (lRtn);
}
LONG Draw3DUpDownDynDialog (HWND hWnd, unsigned uiMsg,
    WORD wParam, LONG lParam, int UpOrDown)
    {
    WNDPROC lpOrgProc;
    LONG    lRtn = 0; 
    UINT	Prompt;

    lpOrgProc = ORIGINALPROC(hWnd);
    switch (uiMsg)
    {   
    	case WM_COMMAND:
    		if (ProcessDynEdit (0,hWnd,wParam,lParam))
    			lRtn = 1;
    		else
            	lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	    	break;
    	
    	case WM_NCDESTROY:
        {
	        lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	        SetWindowLong (hWnd, GWL_WNDPROC, (LONG) lpOrgProc);
	        RemoveProp (hWnd, "PrHI");
	        RemoveProp (hWnd, "PrLO");
	        RemoveProp (hWnd, "GMPr");
	        RemoveProp (hWnd, "GMMr");
        } 
        break;
        
        case WM_RBUTTONDOWN:
	    {   
			if (wParam & MK_CONTROL) 
			{
				lRtn = 0; 
				ShowDynDlgUpdateOptions (hWnd,hWnd);
			}
			else
	        	lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	    	LastWnd = hWnd;
			Prompt = (UINT)GetProp (hWnd, "GMMr");
			SetPromptDlg (Prompt);  
				
	    } 
	    break;
	    
    	case WM_RBUTTONUP:
	    { 
	    	LastWnd = 0;                            
	        lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	    } 
	    break;
	    
    	case WM_NCHITTEST:
	    {   
	        lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);  
	    	if (hWnd != LastWnd)
	    	{   
	    		LastWnd = hWnd;
				Prompt = (UINT)GetProp (hWnd, "GMPr");
				SetPromptDlg (Prompt);
		    }
	    }
	    break;
	    
	    default:
        {
	        lRtn = CallWindowProc ((WNDPROC)lpOrgProc, hWnd, uiMsg, (WPARAM)wParam, lParam);
	
	        //if (uiMsg == WM_PAINT && UpOrDown)
	        //    Draw3DFrame (hWnd, UpOrDown);
        }
        break;
    }

    return (lRtn);
}

LONG FAR PASCAL Draw3DDown(HWND hWnd, unsigned uiMsg,
    WORD wParam, LONG lParam)
    {
    return Draw3DUpDown(hWnd, uiMsg, wParam, lParam, DOWN_3D);
    }

LONG FAR PASCAL Draw3DUp(HWND hWnd, unsigned uiMsg,
    WORD wParam, LONG lParam)
    {
    return Draw3DUpDown(hWnd, uiMsg, wParam, lParam, UP_3D);
    }

LONG FAR PASCAL Draw3DDownDynDialog(HWND hWnd, unsigned uiMsg,
    WORD wParam, LONG lParam)
    {
    return Draw3DUpDownDynDialog(hWnd, uiMsg, wParam, lParam, DOWN_3D);
    }

LONG FAR PASCAL Draw3DUpDynDialog(HWND hWnd, unsigned uiMsg,
    WORD wParam, LONG lParam)
    {
    return Draw3DUpDownDynDialog(hWnd, uiMsg, wParam, lParam, UP_3D);
    }


LONG FAR PASCAL GMMessage (HWND hWnd, unsigned uiMsg,
    WORD wParam, LONG lParam)
    {
    return Draw3DUpDown(hWnd, uiMsg, wParam, lParam, 0);
    }






void FAR PASCAL SubclassControl(HWND hCtrl, void FAR *Callback)
    {
    FARPROC     lpOrgProc; 
    DWORD		PrDat;
    HWND		hWnd;

    lpOrgProc = (FARPROC) SetWindowLong (hCtrl, GWL_WNDPROC,
                       (LONG) (FARPROC) Callback);
    SetProp (hCtrl, "PrHI", (HANDLE) HIWORD (lpOrgProc));
    SetProp (hCtrl, "PrLO", (HANDLE) LOWORD (lpOrgProc));
    if ((hWnd=GetParent(hCtrl)) == CurDlgWnd)
    	hWnd = hCtrl; 
    PrDat = GetDlgItemPrompt (hWnd);
    SetProp (hCtrl, "GMPr", (HANDLE) LOWORD(PrDat));
    SetProp (hCtrl, "GMMr", (HANDLE) HIWORD(PrDat));
    }




BOOL CALLBACK EnumCtrlProc(HWND hCtrl,LONG lParam)
{
    char    str[100];
    long	lUserData;

    GetClassName (hCtrl, str, sizeof (str));
    if (_fstrcmp (str, "Edit") == 0)
        SubclassControl(hCtrl, Draw3DDown);
    else if (_fstrcmp (str, "ListBox") == 0)
        ;//SubclassControl(hCtrl, Draw3DDown);
    else if (_fstrcmp (str, "ComboBox") == 0)
        ;//SubclassControl(hCtrl, ComboColor);
    else if ((_fstrcmp (str, "Static") == 0)
        && (GetWindowLong (hCtrl, GWL_STYLE) & WS_BORDER) != 0)
        {
/* if control contains no text, recess it, else raise it */
        if (GetWindowText (hCtrl, str, sizeof (str)) == 0)
            ;//SubclassControl (hCtrl, Draw3DDown);
        else
            ;//SubclassControl (hCtrl, Draw3DUp);
        } 
    else
        ;//SubclassControl(hCtrl, GMMessage);
    return (TRUE);
}

BOOL CALLBACK EnumCtrlProcDynDialog(HWND hCtrl,LONG lParam)
{
    char    str[100];
    long	lUserData;

    GetClassName (hCtrl, str, sizeof (str));
    if (_fstrcmp (str, "Edit") == 0)
        SubclassControl(hCtrl, Draw3DDownDynDialog);
    else if (_fstrcmp (str, "ListBox") == 0)
        SubclassControl(hCtrl, Draw3DDownDynDialog);
    else if (_fstrcmp (str, "ComboBox") == 0)
        SubclassControl(hCtrl, ComboColorDynDialog);
    else if ((_fstrcmp (str, "Static") == 0)
        && (GetWindowLong (hCtrl, GWL_STYLE) & WS_BORDER) != 0)
        {
/* if control contains no text, recess it, else raise it */
        if (GetWindowText (hCtrl, str, sizeof (str)) == 0)
            SubclassControl (hCtrl, Draw3DDownDynDialog);
        else
            SubclassControl (hCtrl, Draw3DUpDynDialog);
        } 
    else
        SubclassControl(hCtrl, GMMessage);
    return (TRUE);
}


static struct 
    {
    BITMAPINFOHEADER    InfoHeader;
    RGBQUAD             Colors[2];
    } MonoDibSpec =
    {
        {
        sizeof(BITMAPINFOHEADER),   /* biSize */
        8, 8,       /* biWidth, biHeight */
        1, 1,       /* biPlanes, biBitCount */
        BI_RGB,     /* biCompression */
        0,          /* biSizeImage */
        0, 0,       /* biXPelsPerMeter, biYPelsPerMeter */
        0, 0,       /* biClrUsed, biClrImportant */
        },
        {
    /*    blue  green  red    */
        { 0xFF, 0xFF, 0xFF, 0x00 },     /* white */
        { 0xC0, 0xC0, 0xC0, 0x00 }      /* light grey */
        }
    };

/* Monochrome replica of "chiselled steel" bitmap */

static BYTE ChiselledSteel [] =
    {
    0xAA, 0x00, 0x00, 0x00, /* rows padded to four bytes */
    0xFF, 0x00, 0x00, 0x00,
    0xAA, 0x00, 0x00, 0x00,
    0xFF, 0x00, 0x00, 0x00,
    0xAA, 0x00, 0x00, 0x00,
    0xFF, 0x00, 0x00, 0x00,
    0xAA, 0x00, 0x00, 0x00,
    0xFF, 0x00, 0x00, 0x00,
    };

BOOL CALLBACK EnumCtrlProcSizeDlg (HWND hWnd,LONG lParam) 
{ 
	RECT	rect2; 
	long	w,h;
	
	GetWindowRect(hWnd, &rect2);  
	w = rect2.right - rect2.left;
	h = rect2.bottom - rect2.top;  
//	rect2 = rect;
	SetWindowPos(hWnd, (HWND) 0,(int)(OldRect.left + (rect2.left-OldRect.left)*wfactor),
								   (int)(OldRect.top  + (rect2.top-OldRect.top)  *hfactor),
								   (int)(w*wfactor),(int)(h*hfactor),SWP_NOZORDER);
	return TRUE;
}

BOOL SizeDlgToRect (HWND hWndDlg,LPRECT pRect)
{   
	int	w,h; 
	
	GetWindowRect (hWndDlg,&OldRect);
	w = pRect->right - pRect->left;
	h = pRect->bottom - pRect->top;  
    wfactor = ((double)(pRect->right - pRect->left))/(OldRect.right - OldRect.left);
    hfactor = ((double)(pRect->bottom - pRect->top))/(OldRect.bottom - OldRect.top);
	SetWindowPos(hWndDlg, (HWND) 0, pRect->left,pRect->top,(int)w,(int)h,SWP_NOZORDER);
	
	EnumChildWindows (hWndDlg, EnumCtrlProcSizeDlg, 0L);
    return TRUE;
}
BOOL FAR PASCAL DIALOGSTYLEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	BOOL bRtn=FALSE;      
	HANDLE	hDynDlgControls;
	LPCONTROLDEF	pControlDef;
	short	i,ii;
	char	str[256];
//return FALSE;
 switch(Message)
   {
   	case WM_RBUTTONDOWN:
   		if (wParam & MK_CONTROL)
   		{   
   			hWndEdit = 0;
   			if (GetDynDlgHandle (hWndDlg,&hDynDlgControls))
   			{
                POINT	Point; 
                RECT	rect;
                
                GetCursorPos(&Point);
   				pControlDef = (LPCONTROLDEF)GlobalLock (hDynDlgControls); 
   				for (i=0;i<NumControls;i++,pControlDef++)
   				{  
                    GetWindowRect(pControlDef->hWnd,&rect);
                    if (PtInRect(&rect,Point))
                    {   
                    	hWndEdit = pControlDef->hWnd;
						ShowDlgUpdateOptions (hWndEdit,hWndDlg);
                    	bRtn = TRUE;
                    	break;
                    }
   				}
   				GlobalUnlock (hDynDlgControls);  
   				return bRtn;
   			}
   		}
   		break; 
    
    case WM_CLOSE:
/*    	InDynClose = TRUE; 
    	if (!(bRtn = ProcessDynEdit (hWndDlg,hWndEdit,IDC_EXIT,lParam)))
    	{
	   		WriteDynDlgData (hWndDlg);
	   		SetDynDlgData (hWndDlg,0,0,0);   
	   	}
    	InDynClose = FALSE;*/
    	return bRtn;
//	case WM_MOUSEMOVE:
//		ii = 1;
//		break;
    case WM_COMMAND:  
    //	return ProcessDynEdit (hWndDlg,hWndEdit,wParam,lParam);
    	
   	case WM_DESTROY:
		ii=1;
   		break;
   	case WM_NCHITTEST:
   		SetPromptDlg (0);
   		LastWnd = 0;
   		break;
   		
    case WM_INITDIALOG: 
         CurDlgWnd=hWndDlg;
         SetProp (hWndDlg, "PBr", hBackBrush1);
         SetProp (hWndDlg, "SBr", hBackBrush1); 
         EnumChildWindows (hWndDlg, EnumCtrlProc, 0L);
         return FALSE;
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLORSCROLLBAR:
          // SetBkMode ((HDC) wParam, TRANSPARENT);
          // bRtn = (BOOL) (HBRUSH) GetProp (hWndDlg, "PBr");
          // return (bRtn);
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLOREDIT:
           //SetBkColor ((HDC) wParam, RGB (192,192, 192));  
           //SetTextColor ((HDC) wParam, RGB (0, 0, 128));  
          // return (BOOL)GetStockObject(LTGRAY_BRUSH);
		  // return DefWindowProc(hWndDlg, Message, wParam, lParam);
		return FALSE;
        break;                                              	          
   }
  return FALSE;
} 

BOOL FAR PASCAL DIALOGSTYLEDynDialogMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	BOOL bRtn=FALSE;      
	HANDLE	hDynDlgControls;
	LPCONTROLDEF	pControlDef;
	short	i,ii;
//	char	str[256];
//return FALSE;
 switch(Message)
   {
   	case WM_RBUTTONDOWN:
   		if (wParam & MK_CONTROL)
   		{   
   			hWndEdit = 0;
   			if (GetDynDlgHandle (hWndDlg,&hDynDlgControls))
   			{
                POINT	Point; 
                RECT	rect;
                
                GetCursorPos(&Point);
   				pControlDef = (LPCONTROLDEF)GlobalLock (hDynDlgControls); 
   				for (i=0;i<NumControls;i++,pControlDef++)
   				{  
                    GetWindowRect(pControlDef->hWnd,&rect);
                    if (PtInRect(&rect,Point))
                    {   
                    	hWndEdit = pControlDef->hWnd;
						ShowDlgUpdateOptions (hWndEdit,hWndDlg);
                    	bRtn = TRUE;
                    	break;
                    }
   				}
   				GlobalUnlock (hDynDlgControls);  
   				return bRtn;
   			}
   		}
   		break; 
    
    case WM_CLOSE:
    	InDynClose = TRUE; 
    	if (!(bRtn = ProcessDynEdit (hWndDlg,hWndEdit,IDC_EXIT,lParam)))
    	{
	   		WriteDynDlgData (hWndDlg);
	   		SetDynDlgData (hWndDlg,0,0,0);   
	   	}
    	InDynClose = FALSE;
    	return bRtn;
    	
    case WM_COMMAND:  
    	return ProcessDynEdit (hWndDlg,hWndEdit,wParam,lParam);
    	
   	case WM_DESTROY:
		ii=1;
   		break;
   	case WM_NCHITTEST:
   		SetPromptDlg (0);
   		LastWnd = 0;
   		break;
   		
    case WM_INITDIALOG: 
         CurDlgWnd=hWndDlg;
         SetProp (hWndDlg, "PBr", hBackBrush1);
         SetProp (hWndDlg, "SBr", hBackBrush1); 
         EnumChildWindows (hWndDlg, EnumCtrlProcDynDialog, 0L);
         return FALSE;
#if WIN32
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLORSCROLLBAR:
          // SetBkMode ((HDC) wParam, TRANSPARENT);
          // bRtn = (BOOL) (HBRUSH) GetProp (hWndDlg, "PBr");
          // return (bRtn);
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLOREDIT:
           //SetBkColor ((HDC) wParam, RGB (192,192, 192));  
           //SetTextColor ((HDC) wParam, RGB (0, 0, 128));  
          // return (BOOL)GetStockObject(LTGRAY_BRUSH);
		  // return DefWindowProc(hWndDlg, Message, wParam, lParam);
		return FALSE;
#else
    case WM_CTLCOLOR:
        switch (HIWORD (lParam))
            {
            auto BOOL bRtn; 
            
            case CTLCOLOR_DLG:

            case CTLCOLOR_STATIC:
            case CTLCOLOR_BTN:
            case CTLCOLOR_MSGBOX:
            case CTLCOLOR_SCROLLBAR:
                SetBkMode ((HDC) wParam, TRANSPARENT);
                bRtn = (BOOL) (HBRUSH) GetProp (hWndDlg, "PBr");
                return (bRtn);
                break;   
            default:    
            case CTLCOLOR_LISTBOX:
            case CTLCOLOR_EDIT:
                SetBkColor ((HDC) wParam, RGB (192,192, 192));  
                SetTextColor ((HDC) wParam, RGB (0, 0, 128));  
                return GetStockObject(LTGRAY_BRUSH);
                break;

            } 
#endif
        break;                                              	          
   }
  return FALSE;
} 

BOOL ProcessDlgEdit (HWND hWndDlg,HWND hWndEdit,WPARAM wParam,LPARAM lParam) 
{   
	char	str[1024], ControlName[66], FieldName[66], FieldVal[256]; 
	short	id,choice,i,j; 
	BOOL	rtn = FALSE;
	LPOPENSQLDATA	SQLPtr;
	LPFIELDINFO	lpFieldInfo;
	LPOPENFILEDATA	FilePtr; 
	LPSTR	pEq;
	
	if (!hWndDlg)
		hWndDlg = GetParent (hWndEdit);
	if (HIWORD(lParam) == EN_KILLFOCUS)
		hWndEdit = GetDlgItem (hWndDlg,wParam); 
	switch (wParam)
	{   
		case 65001:
			GetWindowText (hWndEdit,str,MAX_PATH);

			if ((pEq = strchr (str,'=')))
				pEq++;
			else
				pEq = str;
			GMEdit (hWndEdit,pEq);
            break;
 
     }
     return rtn;
}




