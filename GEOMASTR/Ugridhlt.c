/*
	Features Program

	Ultimate Grid Example Program Number 1
	From Dundas Software Ltd.

	NOTE:
		Parts of this code may be used to create new programs.
		This source may not be redistributed.

	Purpose of program:
		- shows how data is retrieved from a propriatary data file
		- shows the different ways to adjust column widths
		- shows the different text color and alignment possiblities
		- shows how to adjust the table size when its parent window resizes

*/
#define CONTROLKEY	    0x0001
#define SHIFTKEY	    0x0002    
#define DELETESORT		0
#define SORTONTAG		1
#define SORTONAREA		2 
#define SORTONREF		3
#define SORTONSEQ		4
#define SORTONLEN		5
#define SORTONTYPE		6
#define SORTONPERDIVAREA 7  
#define SORTONSYMBOL	8
#define	MAXCOLS			256
#include "graphint.h"
#include "ugtable.h"   

#include "gmextern.h"

static RECT		WindRect={20,200,1020,600};
static RECT		UnDockedRect;
static	int		SaveDockHeight;
static	long	SliderColor = -1;
static	int		OriginalWidth=1000,OriginalHeight=16,InitialHDGHeight=26;InitialFTRHeight=20;
static	double	shrink=1;
static int 	fittowindow=1;		//Fit-To-Window flag
static	HIGHLIGHTDATA	HighlightData; 
static	char	nstr[32];  
static	int		nCol=8;
static	int		RowHeight=14; 
static	int		LinPrecision=0, AreaPrecision=0;  
static	short	LinUnitItems[]={ID_OPTIONS_LINEARUNITS_FEET,
								ID_OPTIONS_LINEARUNITS_METERS,    
								ID_OPTIONS_LINEARUNITS_YARDS,
								ID_OPTIONS_LINEARUNITS_MILES,
								ID_OPTIONS_LINEARUNITS_KILOMETERS};
static	short	AreaUnitItems[6]={ID_OPTIONS_AREAUNITS_SQUAREFEET,
								  ID_OPTIONS_AREAUNITS_SQUAREMETERS,
								  ID_OPTIONS_AREAUNITS_SQUAREYARDS,
								  ID_OPTIONS_AREAUNITS_SQUAREMILES,
								  ID_OPTIONS_AREAUNITS_SQUAREKILOMETERS,
								  ID_OPTIONS_AREAUNITS_ACRES};
static	long	Sequence; 
static	int		Dock=0;
static	BOOL	IsDocked=FALSE;
static	int		DockVP;
static	RECT	DockSaveRect;
static	short	UseList; 
static	HANDLE	hUseList, hSortList=0;
static	HANDLE	hSelectList=0; 
#define USEHIGHLIGHT 1
#define USEHIGHLIGHT2 2
#define USEOTHER 3
static long	startdata=0;
static long	CurRow=-1;
static BOOL	InFlash=FALSE;
static short	CurSortList=SORTONSEQ;
static	HWND	hWndList=0;
static	int	isMinMax=0;

static TABLEINFO	tbi;
static TABLEINFO far *ti=&tbi;
static	int	nrows,ncols, listwidth;
static	int	colw[MAXCOLS], intspace;
static	double colpct[MAXCOLS];

long far pascal UGRID_HLTDlgProc(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam);  
long far pascal InformationDlgProc(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam);
long far pascal WidthDlgProc(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam);
void SetLinPrecision (HWND hWnd,short precision);
void SetAreaPrecision (HWND hWnd,short precision);
LPSTR	LinOpt(double dist);
LPSTR	AreaOpt(double area);

int  TB_RedrawTable(HWND hWnd)
{
	int	index = SendMessage (hWndList,LB_GETTOPINDEX,0,0);
	RECT	Rect;

	GetClientRect(hWnd,&Rect);
	if (Rect.right <= Rect.left || Rect.bottom <= Rect.top)
		return FALSE;
	shrink = ((double)(Rect.right-Rect.left))/OriginalWidth;
	InvalidateRect (hWndList,0,TRUE);
//	SendMessage (hWndList,LB_SETTOPINDEX,index,0);
				RowHeight = 16*shrink;
	RowHeight = OriginalHeight * shrink;
	SendDlgItemMessage(hWnd,IDC_UGTABLE,LB_SETITEMHEIGHT,0,MAKELPARAM(RowHeight, 0)); 
	PostMessage(hWnd, WM_COMMAND, IDC_SLIDERPAINT, 0L);
	return TRUE;
}
void SetColPcts (BOOL Force)
{
	int	i;
	RECT	Rect;
	static	BOOL	HavePct=FALSE;

	if (!hWndList)
		return;
	if (Force)
		HavePct = FALSE;
	if (HavePct)
		return;
	GetClientRect(hWndList,&Rect);
	listwidth = Rect.right - Rect.left;
	for (i=0;i<ncols;i++)
		colpct[i] = ((double)colw[i])/listwidth;
	HavePct = TRUE;
	return;
}

int TB_SetupTable(HWND hWndDlg,HWND Tablehwnd,HWND Tableheadhwnd,HWND Tablefoothwnd,LPARAM rows,int cols,
							int far* colwidths,int interspace,int usersize)
{
	UINT	i;

	nrows = rows;
	ncols = cols;
	if (colwidths)
		for (i=0;i<ncols;i++)
			colw[i] = colwidths[i];
	intspace = interspace;
	hWndSlider = Tableheadhwnd;
	hWndTotals = Tablefoothwnd;
	hWndHighlight = hWndDlg;
	hWndList = Tablehwnd;
	return TRUE;
}

int   TB_InitTable(void)
{
	return TRUE;
}

int   TB_ExtendText(TABLEINFO far *ti,WPARAM alignment,COLORREF textcolor,
							COLORREF backcolor)
{
	return TRUE;
}

int   TB_BestFit(HWND tblhwnd,int extra,LPARAM count,
							FARPROC fp)
{
	return TRUE;
}

int   TB_FitToWindow(HWND tblhwnd,int numcols)
{
	return TRUE;
}

int TB_Subclass (HWND hWndDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	LPMEASUREITEMSTRUCT	lpmis;
	LPDRAWITEMSTRUCT	lpdis;
	TEXTMETRIC			tm;
	COLORREF	OldColor;
	int		x,y,linewidth;
	char	str[4096];
	static	RECT	LastRect;
	int	i;
	RECT	rect,Rect;

	switch(message){
	case WM_DESTROY:
		hWndSlider = 0;
		hWndHighlight = 0;
		hWndList = 0;
		break;

	case WM_PAINT:
		{
			RECT	rect;

			GetUpdateRect (hWndDlg,&rect,FALSE);
			if (!IsRectEmpty (&rect))
				PostMessage(hWndDlg, WM_COMMAND, IDC_SLIDERPAINT, 0L);
		}
		break;
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case ID_CANCEL:
		        PostMessage(hWndDlg, WM_CLOSE, 0, 0L);
				break;

				case IDC_UGTABLE:
				{
					int icmd = HIWORD(wParam);
					if (icmd == TB_GOTOROW)
						ii = 1;
					switch (HIWORD(wParam))
					{
						case TB_SETCOLWIDTH:
							colw[wParam]=lParam;
							return TRUE;

						case TB_SETNUMROWS:
							ii=SendDlgItemMessage (hWndDlg,IDC_UGTABLE,LB_SETCOUNT,lParam,0);
							return TRUE;
					}
				}
				break;
                case IDC_SLIDERMOUSE:
					{

						POINT	MousePoint = POINTStoPOINT(MAKEPOINTS(lParam));
					}
					break;
				case IDC_SLIDERPAINT:
					{
						HDC hDC;// = BeginPaint (hWndSlider,&ps);
						RECT	SliderRect;
						HFONT	hFont,hOldFont;
						int		FontSize;

						hDC = GetDC (hWndSlider);
						//SelectClipRgn (hDC,0);
					    //SetDisplayMode (hDC, GF_TEXTMODE);
						ii=GetClientRect(hWndSlider,&SliderRect);
						FontSize = (SliderRect.bottom-SliderRect.top)/2;
						hFont = CreateFont(FontSize, 0, 0, 0, FW_NORMAL, 
    							0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
						hOldFont = SelectObject (hDC,hFont);
						SetTextColor (hDC,0);
						ii=GetTextMetrics(hDC, &tm); 
						Rect = SliderRect;
						linewidth = Rect.right - Rect.left;
						SetColPcts (FALSE);
						if (SliderColor < 0)
							SliderColor = GetPixel (hDC,(Rect.left+Rect.right)/2,(Rect.top+Rect.bottom)/2);
						ii=FillRectColor (hDC,&Rect,SliderColor);
						OldColor = SetBkColor (hDC,SliderColor);
						for (ti->col=0;ti->col < ncols;ti->col++)
						{
							RECT	DrawRect;
							int	colwidth = colpct[ti->col] * linewidth;

							Rect.right = Rect.left + colwidth;
							SendMessage (hWndDlg,WM_COMMAND,MAKEWPARAM(IDC_UGTABLEHDG,TBN_WANTHDG),0);
							//sprintf (strchr(str,0),"%s\t",ti->buf);
							IntersectRect (&DrawRect,&SliderRect,&Rect);
							if (ti->alignment == TA_LEFT)
								DrawText (hDC,ti->buf,-1,&DrawRect,DT_LEFT);
							else if (ti->alignment == TA_CENTER)
								DrawText (hDC,ti->buf,-1,&DrawRect,DT_CENTER);
							else
								DrawText (hDC,ti->buf,-1,&DrawRect,DT_RIGHT);
							Rect.left+=colwidth+intspace;
						}
						SelectObject (hDC,hOldFont);
						GSSiDeleteObject (&hFont);
						SetBkColor (hDC,OldColor);
						ReleaseDC (hWndSlider,hDC);
					}
					{
						HDC hDC;// = BeginPaint (hWndSlider,&ps);
						RECT	TotalsRect;
						HFONT	hFont,hOldFont;
						int		FontSize;

						hDC = GetDC (hWndTotals);
						//SelectClipRgn (hDC,0);
					    //SetDisplayMode (hDC, GF_TEXTMODE);
						ii=GetClientRect(hWndTotals,&TotalsRect);
						InflateRect (&TotalsRect,0,-1);
						FontSize = (TotalsRect.bottom-TotalsRect.top);
						hFont = CreateFont(FontSize, 0, 0, 0, FW_NORMAL, 
    							0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
						SetTextColor (hDC,0);
						hOldFont = SelectObject (hDC,hFont);
						ii=GetTextMetrics(hDC, &tm); 
						Rect = TotalsRect;
						linewidth = Rect.right - Rect.left;
						SetColPcts (FALSE);
						ii=FillRectColor (hDC,&Rect,SliderColor);
						OldColor=SetBkColor (hDC,SliderColor);
						for (ti->col=0;ti->col < ncols;ti->col++)
						{
							RECT	DrawRect;
							int	colwidth = colpct[ti->col] * linewidth;

							Rect.right = Rect.left + colwidth;
							SendMessage (hWndDlg,WM_COMMAND,MAKEWPARAM(IDC_UGTABLEHDG,TBN_WANTFTR),0);
							SendMessage (hWndDlg,WM_COMMAND,MAKEWPARAM(IDC_UGTABLEFTR,TBN_WANTFTR),0);
							//sprintf (strchr(str,0),"%s\t",ti->buf);
							IntersectRect (&DrawRect,&TotalsRect,&Rect);
							if (ti->alignment == TA_LEFT)
								DrawText (hDC,ti->buf,-1,&DrawRect,DT_LEFT);
							else if (ti->alignment == TA_CENTER)
								DrawText (hDC,ti->buf,-1,&DrawRect,DT_CENTER);
							else
								DrawText (hDC,ti->buf,-1,&DrawRect,DT_RIGHT);
							Rect.left+=colwidth+intspace;
						}
						SelectObject (hDC,hOldFont);
						SetBkColor (hDC,OldColor);
						GSSiDeleteObject (&hFont);
						ReleaseDC (hWndTotals,hDC);
					}

					break;
			}
			break;
		}
		break;
		case WM_SIZE:{
			//resizes the child windows to fit inside the parent window   
			short nWidth = LOWORD(lParam);  // width of client area   
			short nHeight = HIWORD(lParam); // height of client area  

			
			isMinMax = 0;
			GetClientRect(hWndDlg,&rect);
			switch (wParam)
			{
            	case SIZE_MAXIMIZED:
					isMinMax = 2;
            	case SIZE_RESTORED:
					//get the size of the client window
					//adjust the table child window to fit in the parent window
					SetWindowPos(GetDlgItem(hWndDlg,IDC_UGTABLE),HWND_TOP,5,(int)(32*shrink),rect.right-10,
						abs((int)(rect.bottom-40*shrink-InitialFTRHeight*shrink)),SWP_NOZORDER);
					//adjust the heading child window to fit
	            	break;
	            	
	            	case SIZE_MINIMIZED:
						isMinMax = 1;
	            	break;
	            }
				SetWindowPos(GetDlgItem(hWndDlg,IDC_UGTABLEHDG),HWND_TOP,5,7,
					rect.right-8-GetSystemMetrics(SM_CYVSCROLL),(int)(InitialHDGHeight*shrink),SWP_NOZORDER);
				SetWindowPos(GetDlgItem(hWndDlg,IDC_UGTABLEFTR),HWND_TOP,5,(int)(rect.bottom-3-InitialFTRHeight*shrink),
					rect.right-8-GetSystemMetrics(SM_CYVSCROLL),(int)(InitialFTRHeight*shrink),SWP_NOZORDER);
	
				//if the fittowindow flag is set then readjust the coulmn widths to
				//fit inside the new table child window width
				if(fittowindow){
					TB_FitToWindow(GetDlgItem(hWndDlg,IDC_UGTABLE),0);
				TB_RedrawTable(hWndDlg);

			}
			return 1;
		}

    case WM_MEASUREITEM: 
 
        lpmis = (LPMEASUREITEMSTRUCT) lParam; 
 
        /* Set the height of the list box items. */ 
 
        lpmis->itemHeight = RowHeight; 
        return TRUE; 
 
    case WM_DRAWITEM: 
 
        lpdis = (LPDRAWITEMSTRUCT) lParam; 
 
        /* If there are no list box items, skip this message. */ 
 
        if (lpdis->itemID == -1) { 
            break; 
        } 
 
        /* 
         * Draw the bitmap and text for the list box item. Draw a 
         * rectangle around the bitmap if it is selected. 
         */ 
 
        switch (lpdis->itemAction) { 
 
            case ODA_SELECT: 
            case ODA_DRAWENTIRE:  
            {
				RECT	LineRect;
				HFONT	hFont,hOldFont;
				int		FontSize=lpdis->rcItem.bottom-lpdis->rcItem.top;

				if (lpdis->CtlID != IDC_UGTABLE)
					break;
                /* Display the text associated with the item. */ 
 
                //SendMessage(lpdis->hwndItem, LB_GETTEXT, 
                  //  lpdis->itemID, (LPARAM) str);
				ti->row = lpdis->itemID;
				*str = 0;
				hFont = CreateFont(FontSize, 0, 0, 0, FW_NORMAL, 
    					0, 0, 0, 0, 0, 0, 0, 0,"Arial");   
				hOldFont = SelectObject (lpdis->hDC,hFont);

                GetTextMetrics(lpdis->hDC, &tm); 
				SetColPcts (FALSE);
				Rect = LineRect = lpdis->rcItem;
				linewidth = Rect.right - Rect.left;
				FillRect (lpdis->hDC,&Rect,GetStockObject(WHITE_BRUSH));
				for (ti->col=0;ti->col < ncols;ti->col++)
				{
					int	colwidth = colpct[ti->col] * linewidth;
					RECT	DrawRect;

					Rect.right = Rect.left + colwidth;
					IntersectRect (&DrawRect,&LineRect,&Rect);
					SendMessage (hWndDlg,WM_COMMAND,MAKEWPARAM(IDC_UGTABLE,TBN_WANTTEXT),0);
					//sprintf (strchr(str,0),"%s\t",ti->buf);
					if (ti->alignment == TA_LEFT)
						DrawText (lpdis->hDC,ti->buf,-1,&DrawRect,DT_LEFT);
					else if (ti->alignment == TA_CENTER)
						DrawText (lpdis->hDC,ti->buf,-1,&DrawRect,DT_CENTER);
					else
						DrawText (lpdis->hDC,ti->buf,-1,&DrawRect,DT_RIGHT);
					Rect.left+=colwidth+intspace;
				}
 
                y = (lpdis->rcItem.bottom + lpdis->rcItem.top - 
                    tm.tmHeight) / 2; 
 
                /*TextOut(lpdis->hDC, 
                    lpdis->rcItem.left, 
                    y, 
                    str, 
                    strlen(str));*/ 
 
                /* Is the item selected? */ 
				SelectObject (lpdis->hDC,hOldFont);
				GSSiDeleteObject (&hFont);
                if (lpdis->itemState & ODS_SELECTED) { 
 
 
                    /* 
                     * Draw a rectangle around bitmap to indicate 
                     * the selection. 
                     */ 
                    
                    //if (LastRect.top != -1000)
                    //	DrawFocusRect(lpdis->hDC, &LastRect);
                    //DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
					InvertRect(lpdis->hDC, &lpdis->rcItem); 
                    LastRect = lpdis->rcItem; 
                }
            } 
				return TRUE;
 
            case ODA_FOCUS: 
                DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
                //InvertRect(lpdis->hDC, &lpdis->rcItem); 
				return TRUE;
        }
	}
	return FALSE;
}

//int   TB_CurrencyToString(char far *String,double d,int Commas,
//							char far *Symbol);
//int   TB_DateToString(char far *String,int Day,int Month,int Year,
//							char far *Separator,int Type);
LPSTR	LinOpt(double dist)
{    
	dist = ConvertDist(dist,OutDistUnits);
	RWRITE (dist, LinPrecision,nstr);
	return nstr;
}

LPSTR	AreaOpt(double area)
{   
	area = ConvertArea (area,OutAreaUnits);
		
	RWRITE (area, AreaPrecision,nstr);
	return nstr;
}  

BOOL HLTRowSelected (long RecNum)
{   
	long Refno;
	
	if (BT_FIND (hSelectList,(LPSTR)&RecNum,BT_FIRST,BT_EQ,(LPSTR)&Refno))
		return FALSE;
	return TRUE;
}

BOOL GetRowHighlightData (long RowNum,LPHIGHLIGHTDATA pHighlightData)
{
	long	Sequence, Refno;
	short	st; 
	char	Key[260];
	
	if (UseList == USEHIGHLIGHT)
		st=BT_FIND_RECORD_NUMBER (hHighlight,RowNum, (LPSTR)&Refno,(LPSTR)pHighlightData);
	else
	{
		if (UseList == USEHIGHLIGHT2)
			hUseList = hHighlight2;//hHighlight2 can change during checkpoint if undoenabled
		if (!(st=BT_FIND_RECORD_NUMBER (hUseList,RowNum, Key,(LPSTR)&Refno)))
			st=BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_EQ,(LPSTR)pHighlightData); 
	}
	if (!st)
		return TRUE;
	return FALSE;
} 

BOOL GetNextHighlightData (LPLONG pRefno,LPHIGHLIGHTDATA pHighlightData,BOOL First,BOOL getSameRec)
{
	long	Sequence;
	short	st, pos=BT_NEXT; 
	char	Key[260];
	static HIGHLIGHTDATA lastHltData;
	
	if (!hHighlight)
		return FALSE;
	if (getSameRec)
	{
		memcpy(pHighlightData, &lastHltData, sizeof(HIGHLIGHTDATA));
		return TRUE;
	}
	if (First)  
	{
		pos = BT_FIRST;
		BuildSortList (CurSortList);
	}
	if (UseList == USEHIGHLIGHT)
		st=BT_FIND (hHighlight,(LPSTR)pRefno,pos,BT_ANY,(LPSTR)pHighlightData); 
	else
	{
		if (UseList == USEHIGHLIGHT2)
			hUseList = hHighlight2;//hHighlight2 can change during checkpoint if undoenabled
		if (!(st=BT_FIND (hUseList,(LPSTR)Key,pos, BT_ANY,(LPSTR)pRefno)))
			st=BT_FIND (hHighlight,(LPSTR)pRefno,BT_FIRST,BT_EQ,(LPSTR)pHighlightData); 
	}
	if (!st)
	{
		memcpy(&lastHltData, pHighlightData, sizeof(HIGHLIGHTDATA));
		HLTGraphicsPos = pHighlightData->HLTGraphicsFilePos;
		if (*HLTGraphicsFile)
			PickList[0].Segment = 0;
		return TRUE; 
	}
	BuildSortList (DELETESORT);
	return FALSE;
} 

void HLTSelectAdd (long RecNum)
{	BTVARDESC	BTVar[1];
	long	Refno;   
    
    if (RecNum <= 0)
    	return;
	if (!hSelectList)
	{   
		char	File[144];
		
		GSSiGetTempFileName (0,"gm",0,File);
		BTVar[0].BT_VARTYP=BT_INTEGER;
		BTVar[0].BT_VARLEN=4;
		BTVar[0].BT_VAROFF=0;
		BT_CREATE (File, 4, FALSE, 1, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
		hSelectList = BT_OPEN (File, 0, BT_WRITE, 0);
	} 
	if (BT_FIND (hSelectList,(LPSTR)&RecNum,BT_FIRST,BT_EQ,(LPSTR)&Refno)) 
	{
		GetRowHighlightData (RecNum,&HighlightData);
		BT_PUT (hSelectList,(LPSTR)&RecNum,(LPSTR)&HighlightData.PD.Refno);
	}
	else
		BT_DELETE (hSelectList,(LPSTR)&RecNum,(LPSTR)&Refno,FALSE);
	return;
}

void HLTSelectClear (BOOL Delete)
{
	if (Delete)
		BT_CLOSEANDDELETE (&hSelectList);
	else
		BT_CLEAR (hSelectList);
	return;
} 

BOOL BuildSortList (short opt)
{
	BTVARDESC	BTVar[2];
	char	File[MAX_PATH], Key[260], TAG[80], SymName[34];
	short	pos=BT_FIRST;   
	long	Refno;
	HCURSOR	hcurSave;

	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
    if (opt)
    	CurSortList = opt;
	BT_CLOSEANDDELETE (&hSortList);  
	HLTSelectClear (FALSE);
	switch (opt)
	{   
		case SORTONREF:
			hUseList = hHighlight;
			UseList = USEHIGHLIGHT; 
		break;
		
		case SORTONSEQ:
			hUseList = hHighlight2;
			UseList = USEHIGHLIGHT2;
		break;
				
		case SORTONTAG:	   
		    UseList = USEOTHER;
			GSSiGetTempFileName (0,"gm",0,File);
			BTVar[0].BT_VARTYP=BT_CHAR;
			BTVar[0].BT_VARLEN=42;
			BTVar[0].BT_VAROFF=0;
			BTVar[1].BT_VARTYP=BT_INTEGER;
			BTVar[1].BT_VARLEN=4;
			BTVar[1].BT_VAROFF=42;
			BT_CREATE (File, 4, FALSE, -2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
			hSortList = BT_OPEN (File, 0, BT_WRITE, 0);  
			while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))  
			{
				pos = BT_NEXT;
				if (*HighlightData.PD.Prefix)
					sprintf (TAG,"%s:%s",HighlightData.PD.Prefix,HighlightData.PD.UDI);
				else
					*TAG = 0; 
				_fstrncpy (Key,TAG,42);
				_fmemcpy (&Key[42],&Refno,4);
				BT_PUT (hSortList,Key,(LPSTR)&Refno);
			}
			UpdateBTCounts (hSortList);
			hUseList = hSortList;
		break; 
		
		case SORTONSYMBOL:	   
		    UseList = USEOTHER;
			GSSiGetTempFileName (0,"gm",0,File);
			BTVar[0].BT_VARTYP=BT_CHAR;
			BTVar[0].BT_VARLEN=32;
			BTVar[0].BT_VAROFF=0;
			BTVar[1].BT_VARTYP=BT_INTEGER;
			BTVar[1].BT_VARLEN=4;
			BTVar[1].BT_VAROFF=32;
			BT_CREATE (File, 4, FALSE, -2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
			hSortList = BT_OPEN (File, 0, BT_WRITE, 0);  
			while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))  
			{
				pos = BT_NEXT;
				GetSymbolName (HighlightData.PD.Desc,SymName,0,-(HighlightData.PD.FileNum+1),0);
				_fstrncpy (Key,SymName,32);
				_fmemcpy (&Key[32],&Refno,4);
				BT_PUT (hSortList,Key,(LPSTR)&Refno);
			}
			UpdateBTCounts (hSortList);
			hUseList = hSortList;
		break; 
		
		case SORTONTYPE:	   
		    UseList = USEOTHER;
			GSSiGetTempFileName (0,"gm",0,File);
			BTVar[0].BT_VARTYP=BT_CHAR;
			BTVar[0].BT_VARLEN=2;
			BTVar[0].BT_VAROFF=0;
			BTVar[1].BT_VARTYP=BT_INTEGER;
			BTVar[1].BT_VARLEN=4;
			BTVar[1].BT_VAROFF=2;
			BT_CREATE (File, 4, FALSE, -2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
			hSortList = BT_OPEN (File, 0, BT_WRITE, 0);  
			while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))  
			{   
				LPSHORT	pType=(LPSHORT)Key;
				pos = BT_NEXT;
				
				*pType = HighlightData.PD.Type*2;
				if (HighlightData.PD.HasText)
					*pType += 1;
				_fmemcpy (&Key[2],&Refno,4);
				BT_PUT (hSortList,Key,(LPSTR)&Refno);
			}
			UpdateBTCounts (hSortList);
			hUseList = hSortList;
		break; 
		
		case SORTONAREA:
		case SORTONLEN: 
		case SORTONPERDIVAREA:
		    UseList = USEOTHER;
			GSSiGetTempFileName (0,"gm",0,File);
			BTVar[0].BT_VARTYP=BT_REAL;
			BTVar[0].BT_VARLEN=8;
			BTVar[0].BT_VAROFF=0;
			BTVar[1].BT_VARTYP=BT_INTEGER;
			BTVar[1].BT_VARLEN=4;
			BTVar[1].BT_VAROFF=8;
			BT_CREATE (File, 4, FALSE, -2, 1,(LPBTVARDESC)BTVar,FALSE, 0, 0, FALSE);
			hSortList = BT_OPEN (File, 0, BT_WRITE, 0);  
			while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))  
			{
				pos = BT_NEXT;  
				switch (opt)
				{
					case SORTONAREA:
						_fmemcpy (Key,&HighlightData.PD.Area,8);  
						break;
					case SORTONLEN:
						_fmemcpy (Key,&HighlightData.PD.Length,8); 
						break;
					case SORTONPERDIVAREA:
					{
						double Value = 0;
						if (HighlightData.PD.Area)
							Value = HighlightData.PD.Length / sqrt (HighlightData.PD.Area);
						_fmemcpy (Key,&Value,8);
					} 
				}	   
				
				_fmemcpy (&Key[8],&Refno,4);
				BT_PUT (hSortList,Key,(LPSTR)&Refno);
			}
			UpdateBTCounts (hSortList); 
			hUseList = hSortList;
		break;
	}
	GSSiSetCursor (hcurSave);
	return TRUE;
}


/*******************************************
********************************************/
int ShowHLTList(HWND hWnd,int DockingStatus){

	DLGPROC dlgproc;
	int		rc;
	HWND	hWndDlg;
	int		HltHeight,VPHeight;
	double	NewHeight;
	RECT	CurRect;

    if (HLTDlgWnd)
		ShowWindow (HLTDlgWnd,SW_RESTORE);
	else
	{	
		dlgproc = (DLGPROC) MakeProcInstance((FARPROC)UGRID_HLTDlgProc, hInst);
		if (DockingStatus)
		{
			UnDockedRect = WindRect;
			SetViewport(*pCommandViewport);
			SaveDockHeight = CurView->Height;
			CurRect = CurView->Rect;
			HltHeight = WindRect.bottom - WindRect.top;
			VPHeight = CurRect.bottom - CurRect.top;
			NewHeight = VPHeight - HltHeight;
			CurView->Height *= NewHeight/VPHeight;
		    SetupViewports (CurView->hWnd,CurView->hDC,0,MainRect,0); 
			SetViewport(*pCommandViewport);
			SubtractRect (&WindRect,&CurRect,&CurView->Rect);
			ClientRectToScreenRect (CurView->hWnd,&WindRect);
			IsDocked = TRUE;
			hWndDlg = CreateDialog(hInst,"UGRID_HLTDOCKED",hWnd, dlgproc);
			RedisplayWindow();
		}
		else
		{
			IsDocked = FALSE;
			hWndDlg = CreateDialog(hInst,"UGRID_HLT",hWnd, dlgproc);
		}
	}
	setDoPaint( TRUE);

	return 0;
}

/*******************************************
********************************************/
long far pascal UGRID_HLTDlgProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){

	
	//general purpose variables
	int  		t,x, st,i;
	static	long 		nRow=0;
	long	iRow,rowloc, Refno;
	double 	d;
	char 		string[50];
	RECT 		rect;
	HMENU 	hmenu;
	LPWINDOWPOS wp;
	DLGPROC dlgproc;  
	OFSTRUCTGM	OFStruct;
	LPSTR	lpStr; 
	char	val[256]; 
	LPLONG	pRefList;
	HANDLE	hItems;
	LPINT	pItems;
	int		nItems;

	char *hdg[]={"Number","Refno","Type","TAG","Symbol","Length","Area","Perimeter"};  
	char *Types[]={" ","Point","Line","Area","Text","Curve","Delete"};  
	char *cLUnits[]={"Feet","Meters","Yards","Miles","Kilometers"};
	char *cAUnits[]={"Square Feet","Square Meters","Square Yards","Square Miles","Square Kilometers","Acres"};



//	static DATA data;             //data file structure
	static HFILE fptr;				//file pointer to the data file
	static long lastrow=-1;			//saves the last row that the table asked for
	static int	colwidths[8]={64,96,48,256,128,96,96,96};
			
	static int 	savecol[8];			//stores the column widths
	static int 	align=TA_LEFT;		//text alignment flag
	static HBRUSH hbrush=0;			//brush handle
	static int vlines=1;				//separation lines ON/OFF flag   
	static HANDLE	hData=0;
	static LPSTR	pData;
	static long		maxrowlen, startindex; 
	static BOOL		First=TRUE; 
     
    ValidVP (); 
	if (TB_Subclass (hwnd,message,wParam,lParam))
		return TRUE;
	//process messages
	switch(message){

		case WM_INITDIALOG:{
 //           BTHEAD	BTHead;   
            
            hUseList = hHighlight2;  
            UseList = USEHIGHLIGHT2; 
           	lastrow = -1;
			isMinMax = 0;

				SetWindowPos(hwnd,HWND_TOP,WindRect.left,WindRect.top,
					abs(WindRect.right-WindRect.left),
						abs(WindRect.bottom-WindRect.top),SWP_NOZORDER);
			if (First)
				cwCenter(hwnd, 0);
			First = FALSE;
           	OutAreaUnits = GetGlobalLVal2 ("[%AREAUNITS]",1);
           	OutDistUnits = GetGlobalLVal2 ("[%DISTANCEUNITS]",1);
			//create a gray brush (to be used for the dialog background)
			hbrush=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

			//open up the data file
			//get the number of records in the datafile  
			nRow=0;
			//get the tableinfo structure
			//ti = (TABLEINFO far *)SendMessage(GetDlgItem(hwnd,IDC_UGTABLE),
         	//TB_GETADDRESS,0,0);
            HLTDlgWnd = hwnd;

			//set up the table
			TB_SetupTable(hwnd,
				GetDlgItem(hwnd,IDC_UGTABLE),		//table window handle
				GetDlgItem(hwnd,IDC_UGTABLEHDG),   //table heading window handle
				GetDlgItem(hwnd,IDC_UGTABLEFTR),   //table footer window handle
				nRow,                               //number of rows
				nCol,                            //number of columns
				colwidths,                            //column widths
				6,                               //interspace value
				TRUE);                           //user resize option



			//send a WM_SIZE (to adjust the windows to fit within the parent);
			PostMessage(hwnd,WM_SIZE,0,0);
			
			PostMessage(hwnd,WM_COMMAND,LinUnitItems[OutDistUnits-1],0);
			PostMessage(hwnd,WM_COMMAND,AreaUnitItems[OutAreaUnits-1],0);
			PostMessage(hwnd,WM_COMMAND,ID_OPTIONS_LINEARPRECISION_WHOLENUMBERS+LinPrecision,0);
			PostMessage(hwnd,WM_COMMAND,ID_OPTIONS_AREAPRECISION_WHOLENUMBERS+AreaPrecision,0);
			
Redisplay:  
			UpdateBTCounts (hHighlight);  
			UpdateBTCounts (hHighlight2);   
			HLTSelectClear (FALSE);
			BuildSortList (CurSortList); 
RedrawTable:
			if (BT_NUM_IN_INDEX (hHighlight))
				nRow =  BT_NUM_IN_INDEX (hHighlight) +startdata;
			else
				nRow = 0; 
			sprintf (string,"Highlight List: %ld items highlighted",max (0,nRow)); 
			SetWindowText (hwnd,string);
			SendMessage(hwnd,WM_COMMAND,MAKEWPARAM(IDC_UGTABLE,TB_SETNUMROWS),nRow); 
			//set the focus to the table window
			SetFocus(GetDlgItem(hwnd,IDC_UGTABLE));
			SendDlgItemMessage (hwnd,IDC_UGTABLE,LB_SETSEL,FALSE,-1);
            TB_RedrawTable (hwnd);
			//SendMessage(hwnd,WM_COMMAND,MAKEWPARAM(IDC_UGTABLE,TB_GOTOROW),0);
			return 0;
		}
        
		case WM_CTLCOLORDLG:{
			//if the dialog box is to be painted then return the gray brush
			if(HIWORD(lParam)==CTLCOLOR_DLG ||HIWORD(lParam)==CTLCOLOR_STATIC){
				SetBkColor((HDC)wParam,GetSysColor(COLOR_BTNFACE));
				return (long)hbrush;
			}
			return 0;
		}

		case WM_DESTROY:{
			//close the data file  
			//delete the brush 
			BuildSortList (DELETESORT);
			HLTSelectClear (TRUE);
			GSSiDeleteObject(&hbrush); 
			HLTDlgWnd = 0; 
			SetGlobalValueLong ("%DISTANCEUNITS",OutDistUnits); 
			SetGlobalValueLong ("%AREAUNITS",OutAreaUnits);
			SetGlobalValueLong ("%DISTANCEPRECISION",LinPrecision); 
			SetGlobalValueLong ("%AREAPRECISION",AreaPrecision);
			if (IsDocked)
			{
				SetConfig (1);
				SetViewport(*pCommandViewport);
				CurView->Height = SaveDockHeight;
				WindRect = UnDockedRect;
			}
			switch (Dock)
			{
			case 0:
				break;
			case 1:
				PostMessage (hWndMain,WM_COMMAND,IDM_DISPLAY_HLT_LIST,1);
				RedisplayWindow ();
				break;
			case 2:
				PostMessage (hWndMain,WM_COMMAND,IDM_DISPLAY_HLT_LIST,0);
				RedisplayWindow ();
				break;
			}
			return 0;
		}

		case WM_CLOSE:{
			//close the dialog  
			Dock = 0;
			if (!isMinMax)
				GetWindowRect(hwnd,&WindRect);
			if (IsDocked)
			{
				SetViewport(*pCommandViewport);
				CurView->Height = SaveDockHeight;
				WindRect = UnDockedRect;
			}
			DestroyWindow (hwnd);
//			EndDialog(hwnd,0);
			return 0;
		}
 		case WM_NOTIFY:
		{
			LPNMHDR	pnmh;

			pnmh = (LPNMHDR) lParam; 
			if (pnmh->idFrom == IDC_UGTABLEFTR)
			switch (pnmh->code)
			{
			default:
				ii=1;
			}
			break;
		}


		case WM_WINDOWPOSCHANGING:{
      	// this message is called just before the WM_SIZE message

			// make sure that the window parent window is greater than the minimum
			// specified size limit when it is resized
			// if it isn't then adjust the window to the minimum size
			wp=(LPWINDOWPOS)lParam;
			//min height is 210 pixels
			if(wp->cy<210) wp->cy=210;
			//min width is 400 pixels
			if(wp->cx<400) wp->cx=400;

			return 0;
		}
		case WM_COMMAND:{
							switch (LOWORD(wParam)){
							case IDC_REDISPLAY:
								lastrow = -1;
								goto Redisplay;

							case ID_SETCOLWIDTHS:
								shrink *= 0.8;
								RowHeight = 16 * shrink;
								SendDlgItemMessage(hwnd, IDC_UGTABLE, LB_SETITEMHEIGHT, 0, MAKELPARAM(RowHeight, 0));
								break;

							case IDC_UGTABLE:{

												 //find the message sent (ti->msg)
								int icmd = HIWORD(wParam);
								if (icmd == TB_GOTOROW)
									ii = 1;
												 switch (icmd)
												 {
												 case LBN_DBLCLK:
													 PostMessage(hwnd, WM_COMMAND, ID_ZOOM_CURRENTITEM, 0L);
													 break;
												 case LBN_SELCHANGE:
													 nItems = GetLBSelectedItems(hwnd, IDC_UGTABLE, &hItems);
													 if (!nItems)
														 break;
													 pItems = GlobalLock(hItems);
													 CurRow = *pItems;
													 GSSiGlobUlFree(&hItems);
													 break;
												 case TBN_WANTTEXT:{
																	   // check to see if the row is the same as the last
																	   // if not then get a new record from the database
																	   // (this way a record doesnt need to be read in each time
																	   // a cell within the table needs to be drawn)   

																	   long	n;

																	   *ti->buf = 0;
																	   if (ti->row < startdata)
																	   {
																		   switch (ti->row)
																		   {
																		   case 0:
																			   switch (ti->col)
																			   {
																			   case 0:
																				   _fstrcpy(ti->buf, "Units");
																				   break;
																			   case 5:
																			   case 7:
																				   _fstrcpy(ti->buf, cLUnits[OutDistUnits - 1]);
																				   ti->alignment = TA_CENTER;
																				   break;
																			   case 6:
																				   ti->alignment = TA_CENTER;
																				   _fstrcpy(ti->buf, cAUnits[OutAreaUnits - 1]);
																				   break;
																			   }
																			   break;
																		   case -1:
																			   switch (ti->col)
																			   {
																			   case 0:
																			   {
																						 int	n = BT_NUM_IN_INDEX(hHighlight);
																						 sprintf(ti->buf, "%ld", n);
																			   }
																				   break;
																			   case 5:
																				   _fstrcpy(ti->buf, LinOpt(TotHLTLength));
																				   ti->alignment = TA_RIGHT;
																				   break;
																			   case 6:
																				   _fstrcpy(ti->buf, AreaOpt(TotHLTArea));
																				   ti->alignment = TA_RIGHT;
																				   break;
																			   case 7:
																				   _fstrcpy(ti->buf, LinOpt(TotHLTPerim));
																				   ti->alignment = TA_RIGHT;
																				   break;
																			   }
																			   break;
																		   case 2:
																		   {
																					 LPSTR lpEnd;

																					 _fmemset(ti->buf, '-', 255);
																					 lpEnd = ti->buf + 255;
																					 *lpEnd = 0;
																		   }
																			   break;
																		   }

																	   }
																	   else if (ti->row != lastrow)
																	   {
																		   //find the record that coresponds to the row given 
																		   long	RecNum = ti->row - startdata + 1;

																		   if (GetRowHighlightData(RecNum, &HighlightData))
																			   lastrow = ti->row;
																	   }

																	   // get the field that co-responds to the column and put it
																	   // in the ti->buf parameter 
																	   if (ti->row >= startdata)
																	   {
																		   long	RecNum = ti->row - startdata + 1;

																		   if (HLTRowSelected(RecNum))
																		   {
																			   ti->textcolor = GetSysColor(COLOR_HIGHLIGHTTEXT);
																			   ti->backcolor = GetSysColor(COLOR_HIGHLIGHT);
																		   }
																		   else
																		   {
																			   ti->textcolor = GetSysColor(COLOR_WINDOWTEXT);
																			   ti->backcolor = GetSysColor(COLOR_WINDOW);
																		   }
																		   switch (ti->col)
																		   {
																		   case 0:
																			   ltoa(ti->row + 1 - startdata, ti->buf, 10);
																			   ti->alignment = TA_RIGHT;
																			   break;
																		   case 1:
																			   ltoa(HighlightData.PD.Refno, ti->buf, 10);
																			   ti->alignment = TA_RIGHT;
																			   break;
																		   case 2:
																			   _fstrcpy(ti->buf, Types[HighlightData.PD.Type]);
																			   if (HighlightData.PD.Type != 4 && HighlightData.PD.HasText)
																				   _fstrcat(ti->buf, "/T");
																			   ti->alignment = TA_LEFT;
																			   break;
																		   case 3:
																			   if (!SetRefno && !_fstricmp(HighlightData.PD.Prefix, "REFNO"))
																				   sprintf(ti->buf, "INTREFNO:%ld", HighlightData.PD.Refno);
																			   else if (*HighlightData.PD.Prefix)
																			   {
																				   _fstrcpy(ti->buf, HighlightData.PD.Prefix);
																				   _fstrcat(ti->buf, ":");
																				   _fstrcat(ti->buf, HighlightData.PD.UDI);
																			   }
																			   else
																				   *ti->buf = 0;
																			   ti->alignment = TA_LEFT;
																			   break;
																		   case 4:
																		   {
																					 char	SymName[34];

																					 if (!CurrentConfig)
																						 SetConfig(1);

																					 SetConfig(HighlightData.PD.ConfigID);
																					 SetViewport(HighlightData.PD.ViewID);
																					 GetSymbolName(HighlightData.PD.Desc, SymName, 0, -(HighlightData.PD.FileNum + 1), 0);
																					 _fstrcpy(ti->buf, SymName);
																					 ti->alignment = TA_LEFT;
																					 break;
																		   }
																		   case 5:
																			   if (HighlightData.PD.Type != 2 && HighlightData.PD.Type != 5) break;
																			   _fstrcpy(ti->buf, LinOpt(HighlightData.PD.Length));
																			   ti->alignment = TA_RIGHT;
																			   break;
																		   case 6:
																			   if (HighlightData.PD.Type != 3) break;
																			   _fstrcpy(ti->buf, AreaOpt(HighlightData.PD.Area));
																			   ti->alignment = TA_RIGHT;
																			   break;
																		   case 7:
																			   if (HighlightData.PD.Type != 3) break;
																			   _fstrcpy(ti->buf, LinOpt(HighlightData.PD.Length));
																			   ti->alignment = TA_RIGHT;
																			   break;

																		   default:;
																		   }
																	   }
																	   // set the alignment for the first three fields according
																	   // to the alignment selected from the menu

																	   // menu selected color options 
																	   //ti->textcolor=GetSysColor(COLOR_HIGHLIGHTTEXT);
																	   //ti->backcolor=GetSysColor(COLOR_HIGHLIGHT);
																	   return 0;
												 }
												 case TB_GOTOROW:
													 ti->row = lParam;
												 case TBN_ROWCHANGE:
												 {
																	   long	RecNum = ti->row - startdata + 1;
																	   static	long	LastRecNum;
																	   //display the new row number in the status window
																	   //wsprintf(string,"Row Changed To: %ld",ti->row);
																	   //SetDlgItemText(hwnd,IDC_STATUS,string); 
																	   if (!HLTRowSelected(RecNum))
																		   SendDlgItemMessage(hwnd, IDC_UGTABLE, TB_SETHIGHLIGHT, FALSE, 0);
																	   CurRow = ti->row;
																	   if (InFlash && CurRow >= startdata - 1)
																	   {
																		   SendDlgItemMessage(hwnd, IDC_UGTABLE, LB_SETSEL, FALSE, -1);
																		   SendDlgItemMessage(hwnd, IDC_UGTABLE, LB_SETSEL, TRUE, CurRow);
																		   PostMessage(hwnd, WM_COMMAND, ID_FLASH_CURRENTITEM, 0L);
																	   }
																	   else if (ti->wParam & CONTROLKEY)
																	   {
																		   LastRecNum = RecNum;
																		   HLTSelectAdd(RecNum);
																		   TB_RedrawTable(hwnd);
																	   }
																	   else if (ti->wParam & SHIFTKEY)
																	   {
																		   long	iRec = min(LastRecNum, RecNum), EndRec = max(LastRecNum, RecNum);
																		   if (iRec == LastRecNum)
																			   iRec++;
																		   if (EndRec == LastRecNum)
																			   EndRec--;
																		   GSSiSetCursor(LoadCursor(0, IDC_WAIT));
																		   while (iRec <= EndRec)
																			   HLTSelectAdd(iRec++);
																		   GSSiSetCursor(0);
																		   TB_RedrawTable(hwnd);
																	   }
																	   else
																	   {
																		   //SendDlgItemMessage(hwnd,IDC_UGTABLE,LB_SETCURSEL,-1,0);
																		   //SendDlgItemMessage(hwnd,IDC_UGTABLE,TB_GOTOROW,0,ti->row);
																		   //TB_RedrawTable (GetDlgItem(hwnd,IDC_UGTABLE));
																		   //HLTSelectClear (FALSE);
																	   }
																	   return 0;
												 }

												 case TBN_COLCHANGE:{
																		//display the new column number in the status window
																		//wsprintf(string,"Col Changed To: %d",ti->col);
																		//SetDlgItemText(hwnd,IDC_STATUS,string);

																		return 1;
												 }

												 case TBN_ROWSELECTED:{
																		  //display the row/column that was selected
																		  //wsprintf(string," Row:%ld  Col:%d  Selected",ti->row,ti->col);
																		  //SetDlgItemText(hwnd,IDC_STATUS,string);

																		  //if multiple selection is on then tag/untag the field
																		  //if(color==3){
																		  //retrive the current record
																		  //	fseek(fptr,ti->row * sizeof(DATA),SEEK_SET);
																		  //	fread(&data,sizeof(DATA),1,fptr);

																		  //if it is not already selected then select it
																		  //		if(data.flag==0){
																		  //			data.flag=1;
																		  //		}
																		  //otherwise un-select it
																		  //		else{
																		  //			data.flag=0;
																		  //		}

																		  //save the record
																		  //		fseek(fptr,ti->row * sizeof(DATA),SEEK_SET);
																		  //		fwrite(&data,sizeof(DATA),1,fptr);

																		  //clear the lastrow flag
																		  //		lastrow=-1;

																		  //redraw the table so the changes will be shown
																		  //		TB_RedrawTable(GetDlgItem(hwnd,IDC_UGTABLE));
																		  //	}
																		  PostMessage(hwnd, WM_COMMAND, ID_ZOOM_CURRENTITEM, 0L);
																		  return 1;
												 }

												 case TBN_KEYBOARD:{
																	   //display the key that was hit
																	   //wsprintf(string,"Key: %c",ti->wParam);
																	   //SetDlgItemText(hwnd,IDC_STATUS,string);
																	   //search the database for the closest match
																	   //	t=0;					//set the counter to zero
																	   //	rewind(fptr);     //start from the beginning of the file
																	   //	while(1){
																	   //retrive a record
																	   //		x=fread(&data,sizeof(DATA),1,fptr);
																	   //		if(x==0){
																	   //	t--;
																	   //				break;
																	   //		}

																	   //		if(data.Company[0] >= ti->wParam){
																	   //			break;
																	   //		}
																	   //		t++;
																	   //	}
																	   //clear the lastrow flag
																	   //lastrow=-1;

																	   //update the table position
																	   //SendDlgItemMessage(hwnd,IDC_UGTABLE,TB_GOTOROW,0,t);

																	   return 1;
												 }
												 }
												 return 0;
							}
							case IDC_UGTABLEHDG:{
													ii = HIWORD(wParam);
													switch (HIWORD(wParam)){
													case TBN_WANTHDG:{
																		 //set the text buffer to the column name  
																		 *ti->buf = 0;
																		 switch (ti->col)
																		 {
																		 default:
																			 _fstrcpy(ti->buf, hdg[ti->col]);
																			 break;
																		 case 5:
																		 case 7:
																			 sprintf(ti->buf, "%s\r\n(%s)", hdg[ti->col], cLUnits[OutDistUnits - 1]);
																			 break;
																		 case 6:
																			 sprintf(ti->buf, "%s\r\n(%s)", hdg[ti->col], cAUnits[OutAreaUnits - 1]);
																			 break;
																		 }
																		 ti->alignment = TA_CENTER;
																		 return 1;
													}
													}
							}
							case IDC_UGTABLEFTR:{
													ii = HIWORD(wParam);
													switch (HIWORD(wParam)){
													case TBN_WANTFTR:{
																		 //set the text buffer to the column name  
																		 *ti->buf = 0;
																		 switch (ti->col)
																		 {
																		 case 0:
																		 {
																				   int	n = BT_NUM_IN_INDEX(hHighlight);
																				   sprintf(ti->buf, "%ld", n);
																				   ti->alignment = TA_RIGHT;
																		 }
																			 break;
																		 case 5:
																			 _fstrcpy(ti->buf, LinOpt(TotHLTLength));
																			 ti->alignment = TA_RIGHT;
																			 break;
																		 case 6:
																			 _fstrcpy(ti->buf, AreaOpt(TotHLTArea));
																			 ti->alignment = TA_RIGHT;
																			 break;
																		 case 7:
																			 _fstrcpy(ti->buf, LinOpt(TotHLTPerim));
																			 break;
																		 }
													}
													}
													return 1;
							}

							case IDC_SEARCH:{
												//if the search edit box has changed do a new search
												if (HIWORD(lParam) == EN_CHANGE){

													//get the text from the control
													GetDlgItemText(hwnd, IDC_SEARCH, string, 50);

													//search the database for the closest match
													t = 0;					//set the counter to zero
													//	rewind(fptr);     //start from the beginning of the file
													//	while(1){
													//retrive a record
													//	x=fread(&data,sizeof(DATA),1,fptr);
													//	if(x==0){
													//		t--;
													//		break;
													//	}

													//	if(_fstricmp(data.Company,string)>=0){
													//		break;
													//	}
													//	t++;
													//	}
													//clear the lastrow flag
													lastrow = -1;

													//update the table position
													SendDlgItemMessage(hwnd, IDC_UGTABLE, TB_GOTOROW, 0, t);
												}
												return 0;
							}

							case ID_DOCK:
								switch (Dock)
								{
								case 0:
								case 2:
									Dock = 1;
									break;
								case 1:
									Dock = 2;
								}
								GetWindowRect(hwnd, &WindRect);
								DestroyWindow(hwnd);
								break;
							case IDM_HLTSORT_SYMBOL:
								BuildSortList(SORTONSYMBOL);
								TB_RedrawTable(hwnd);
								return 0;

							case IDM_HLTSORT_TAG:
								BuildSortList(SORTONTAG);
								TB_RedrawTable(hwnd);
								return 0;

							case IDM_HLTSORT_AREA:
								BuildSortList(SORTONAREA);
								TB_RedrawTable(hwnd);
								return 0;

							case IDM_HLTSORT_HSEQ:
								BuildSortList(SORTONSEQ);
								TB_RedrawTable(hwnd);
								return 0;

							case IDM_HLTSORT_REFNO:
								BuildSortList(SORTONREF);
								TB_RedrawTable(hwnd);
								return 0;

							case IDM_HLTSORT_PERIMETER:
							case IDM_HLTSORT_LENGTH:
								BuildSortList(SORTONLEN);
								TB_RedrawTable(hwnd);
								return 0;

							case IDM_HLTSORT_PDIVAREA:
								BuildSortList(SORTONPERDIVAREA);
								TB_RedrawTable(hwnd);
								return 0;

							case IDM_HLTSORT_TYPE:
								BuildSortList(SORTONTYPE);
								TB_RedrawTable(hwnd);
								return 0;

							case ID_OPTIONS_LINEARUNITS_FEET:
							case ID_OPTIONS_LINEARUNITS_METERS:
							case ID_OPTIONS_LINEARUNITS_MILES:
							case ID_OPTIONS_LINEARUNITS_KILOMETERS:
							case ID_OPTIONS_LINEARUNITS_YARDS:
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARUNITS_FEET, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARUNITS_METERS, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARUNITS_MILES, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARUNITS_KILOMETERS, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARUNITS_YARDS, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), LOWORD(wParam), MF_BYCOMMAND | MF_CHECKED);
								switch (LOWORD(wParam))
								{
								case ID_OPTIONS_LINEARUNITS_FEET:
									OutDistUnits = 1;
									break;
								case ID_OPTIONS_LINEARUNITS_METERS:
									OutDistUnits = 2;
									break;
								case ID_OPTIONS_LINEARUNITS_YARDS:
									OutDistUnits = 3;
									break;
								case ID_OPTIONS_LINEARUNITS_MILES:
									OutDistUnits = 4;
									break;
								case ID_OPTIONS_LINEARUNITS_KILOMETERS:
									OutDistUnits = 5;
									break;
								}
								SetLinPrecision(hwnd, (short)GetGlobalLVal2("[%DISTANCEPRECISION]", 4));
								return 0;

							case ID_OPTIONS_AREAUNITS_SQUAREFEET:
							case ID_OPTIONS_AREAUNITS_SQUAREMETERS:
							case ID_OPTIONS_AREAUNITS_SQUAREYARDS:
							case ID_OPTIONS_AREAUNITS_ACRES:
							case ID_OPTIONS_AREAUNITS_SQUAREMILES:
							case ID_OPTIONS_AREAUNITS_SQUAREKILOMETERS:
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAUNITS_SQUAREFEET, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAUNITS_SQUAREMETERS, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAUNITS_SQUAREYARDS, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAUNITS_ACRES, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAUNITS_SQUAREMILES, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAUNITS_SQUAREKILOMETERS, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), LOWORD(wParam), MF_BYCOMMAND | MF_CHECKED);
								switch (LOWORD(wParam))
								{
								case ID_OPTIONS_AREAUNITS_SQUAREFEET:
									OutAreaUnits = 1;
									break;
								case ID_OPTIONS_AREAUNITS_SQUAREMETERS:
									OutAreaUnits = 2;
									break;
								case ID_OPTIONS_AREAUNITS_SQUAREYARDS:
									OutAreaUnits = 3;
									break;
								case ID_OPTIONS_AREAUNITS_SQUAREMILES:
									OutAreaUnits = 4;
									break;
								case ID_OPTIONS_AREAUNITS_SQUAREKILOMETERS:
									OutAreaUnits = 5;
									break;
								case ID_OPTIONS_AREAUNITS_ACRES:
									OutAreaUnits = 6;
									break;
								}
								SetAreaPrecision(hwnd, (short)GetGlobalLVal2("[%AREAPRECISION]", 0));
								TB_RedrawTable(hwnd);
								return 0;

							case ID_OPTIONS_AREAPRECISION_WHOLENUMBERS:
								AreaPrecision = 0;
								SetGlobalValueLong("%AREAPRECISION", AreaPrecision);

								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_WHOLENUMBERS, MF_BYCOMMAND | MF_CHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_01, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_0001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_00001, MF_BYCOMMAND | MF_UNCHECKED);
								TB_RedrawTable(hwnd);
								return 0;

							case ID_OPTIONS_AREAPRECISION_01:
								AreaPrecision = 1;
								SetGlobalValueLong("%AREAPRECISION", AreaPrecision);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_WHOLENUMBERS, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_01, MF_BYCOMMAND | MF_CHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_0001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_00001, MF_BYCOMMAND | MF_UNCHECKED);
								TB_RedrawTable(hwnd);
								return 0;

							case ID_OPTIONS_AREAPRECISION_001:
								AreaPrecision = 2;
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_WHOLENUMBERS, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_01, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_001, MF_BYCOMMAND | MF_CHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_0001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_00001, MF_BYCOMMAND | MF_UNCHECKED);
								TB_RedrawTable(hwnd);
								return 0;

							case ID_OPTIONS_AREAPRECISION_0001:
								AreaPrecision = 3;
								SetGlobalValueLong("%AREAPRECISION", AreaPrecision);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_WHOLENUMBERS, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_01, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_0001, MF_BYCOMMAND | MF_CHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_00001, MF_BYCOMMAND | MF_UNCHECKED);
								TB_RedrawTable(hwnd);
								return 0;

							case ID_OPTIONS_AREAPRECISION_00001:
								AreaPrecision = 4;
								SetGlobalValueLong("%AREAPRECISION", AreaPrecision);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_WHOLENUMBERS, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_01, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_0001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_AREAPRECISION_00001, MF_BYCOMMAND | MF_CHECKED);
								TB_RedrawTable(hwnd);
								return 0;

							case ID_OPTIONS_LINEARPRECISION_WHOLENUMBERS:
								LinPrecision = 0;
								SetGlobalValueLong("%DISTANCEPRECISION", LinPrecision);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_WHOLENUMBERS, MF_BYCOMMAND | MF_CHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_01, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_0001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_00001, MF_BYCOMMAND | MF_UNCHECKED);
								TB_RedrawTable(hwnd);
								return 0;

							case ID_OPTIONS_LINEARPRECISION_01:
								LinPrecision = 1;
								SetGlobalValueLong("%DISTANCEPRECISION", LinPrecision);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_WHOLENUMBERS, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_01, MF_BYCOMMAND | MF_CHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_0001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_00001, MF_BYCOMMAND | MF_UNCHECKED);
								TB_RedrawTable(hwnd);
								return 0;

							case ID_OPTIONS_LINEARPRECISION_001:
								LinPrecision = 2;
								SetGlobalValueLong("%DISTANCEPRECISION", LinPrecision);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_WHOLENUMBERS, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_01, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_001, MF_BYCOMMAND | MF_CHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_0001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_00001, MF_BYCOMMAND | MF_UNCHECKED);
								TB_RedrawTable(hwnd);
								return 0;

							case ID_OPTIONS_LINEARPRECISION_0001:
								LinPrecision = 3;
								SetGlobalValueLong("%DISTANCEPRECISION", LinPrecision);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_WHOLENUMBERS, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_01, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_0001, MF_BYCOMMAND | MF_CHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_00001, MF_BYCOMMAND | MF_UNCHECKED);
								TB_RedrawTable(hwnd);
								return 0;

							case ID_OPTIONS_LINEARPRECISION_00001:
								LinPrecision = 4;
								SetGlobalValueLong("%DISTANCEPRECISION", LinPrecision);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_WHOLENUMBERS, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_01, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_0001, MF_BYCOMMAND | MF_UNCHECKED);
								CheckMenuItem(GetMenu(hwnd), ID_OPTIONS_LINEARPRECISION_00001, MF_BYCOMMAND | MF_CHECKED);
								TB_RedrawTable(hwnd);
								return 0;

							case ID_GOTO_TOP:{
												 //update the table position
												 SendDlgItemMessage(hwnd, IDC_UGTABLE, TB_GOTOROW, 0, 0);

												 return 0;
							}

							case ID_GOTO_BOTTOM:{
													//get the number of records in the database
													// BTHEAD	BTHead; 
													//get the number of records in the datafile    
													if (BT_NUM_IN_INDEX(hHighlight))
														iRow = BT_NUM_IN_INDEX(hHighlight) + startdata - 1;
													else
														iRow = 0;
													//update the table position
													SendDlgItemMessage(hwnd, IDC_UGTABLE, TB_GOTOROW, 0, iRow);

													return 0;
							}

							case ID_SAVE_WIDTHS:{
													//retrieve the widths of the columns in the table and save them
													for (t = 0; t < nCol; t++){
														savecol[t] = (int)SendDlgItemMessage(hwnd, IDC_UGTABLE, TB_GETCOLWIDTH, t, 0);
													}

													return 0;
							}

							case ID_RESTORE_WIDTHS:{
													   //use the previously saved column widths to set the coulmn withs
													   for (t = 0; t < nCol; t++){
														   SendDlgItemMessage(hwnd, IDC_UGTABLE, TB_SETCOLWIDTH,
															   t, savecol[t]);
													   }
													   //redraw the table
													   TB_RedrawTable(GetDlgItem(hwnd, IDC_UGTABLE));

													   return 0;
							}

							case ID_TEXT_LEFT:{
												  //set the alignment flag to 1 (1=left)
												  align = TA_LEFT;
												  //redraw the table
												  TB_RedrawTable(GetDlgItem(hwnd, IDC_UGTABLE));
												  return 0;
							}

							case ID_TEXT_RIGHT:{
												   //set the alignment flag to 2 (2=right)
												   align = TA_RIGHT;
												   //redraw the table
												   TB_RedrawTable(GetDlgItem(hwnd, IDC_UGTABLE));
												   return 0;
							}

							case ID_TEXT_CENTER:{
													//set the alignment flag to 3 (3=center)
													align = TA_CENTER;
													//redraw the table
													TB_RedrawTable(GetDlgItem(hwnd, IDC_UGTABLE));
													return 0;
							}

							case ID_REMOVE:
							{
											  long	Refno, RecNum;
											  HANDLE	hList = hHighlight2;
											  int		nItems = GetLBSelectedItems(hwnd, IDC_UGTABLE, &hItems);

											  if (nItems)
											  {
												  HANDLE hMem = GSSiGlobAlloc(GAIDNO 0, GMEM_MOVEABLE, nItems * sizeof (long));
												  LPLONG	pRefs = GlobalLock(hMem);

												  GSSiSetCursor(LoadCursor(0, IDC_WAIT));
												  pItems = GlobalLock(hItems);
												  for (i = 0; i < nItems; i++)
												  {
													  GetRowHighlightData((*pItems++) + 1, &HighlightData);
													  pRefs[i] = HighlightData.PD.Refno;
												  }
												  for (i = 0; i < nItems; i++)
													  RemoveFromHighlightList(pRefs[i], 1);
												  GSSiGlobUlFree(&hMem);
											  }
											  GSSiSetCursor(0);
											  GSSiGlobUlFree(&hItems);
											  goto Redisplay;
							}

							case ID_DESELECTHLT:
								HLTSelectClear(FALSE);
								TB_RedrawTable(GetDlgItem(hwnd, IDC_UGTABLE));
								break;

							case ID_CLEARHLT:
							{
												long	Refno, RecNum;
												short	pos = BT_FIRST;
												HANDLE	hHighlightOld = hHighlight;
												int		nItems = GetLBSelectedItems(hwnd, IDC_UGTABLE, &hItems);

												if (!hHighlight)
													break;
												if (nItems)
												{
													HANDLE hMem = GSSiGlobAlloc(GAIDNO 1878, GMEM_MOVEABLE, nItems * sizeof (long));
													LPLONG	pRefs = GlobalLock(hMem);

													GSSiSetCursor(LoadCursor(0, IDC_WAIT));
													pItems = GlobalLock(hItems);
													for (i = 0; i < nItems; i++)
													{
														GetRowHighlightData((*pItems++) + 1, &HighlightData);
														pRefs[i] = HighlightData.PD.Refno;
													}
													hHighlight = 0;
													BT_CLOSEANDDELETE(&hHighlight2);
													OpenHighlightList("", 0);
													for (i = 0; i < nItems; i++)
													{
														if (!BT_FIND(hHighlightOld, (LPSTR)&pRefs[i], BT_FIRST, BT_EQ, (LPSTR)&HighlightData))
														{
															BT_PUT(hHighlight, (LPSTR)&pRefs[i], (LPSTR)&HighlightData);
															BT_PUT(hHighlight2, (LPSTR)&HighlightData.Sequence, (LPSTR)&pRefs[i]);
														}
													}
													BT_CLOSEANDDELETE(&hHighlightOld);
													GSSiGlobUlFree(&hMem);
													GSSiGlobUlFree(&hItems);
												}
												else
													ClearHighlightList(FALSE);
												PostMessage(hWndMain, WM_COMMAND, IDM_Z_REDRAW, 0L);
												goto Redisplay;
							}

								return 0;

					case ID_ZOOM_CURRENTITEM:
					{
						int		nItems = GetLBSelectedItems(hwnd, IDC_UGTABLE, &hItems);

						if (nItems > 1)
						{
							MNMXCORD bounds;

							DBoundsInit(&bounds);

							pItems = GlobalLock(hItems);
							for (i = 0; i < nItems; i++)
							{
								GetRowHighlightData((*pItems++) + 1, &HighlightData);
								AddMinMaxD(&bounds, &HighlightData.PD.Rect);
							}
							ZoomToRect(bounds, FALSE);
							return 0;
						}
					}

					if (CurRow >= startdata)
					{   
						HaltMapDisplay (FALSE,TRUE);
						if (!CurrentConfig)
							SetConfig (1);
						SetViewport(*pCommandViewport);
						if (GetRowHighlightData (CurRow-startdata+1,&HighlightData)) 
						{
							PickList[0]=HighlightData.PD; 
							SetPickGlobals (0);
							ZoomToPickedItem (0,1.0,TRUE,FALSE,FALSE);
						} 
					}
					return 0;
                
				case ID_FLASH_CURRENTITEM:
					if (CurRow >= startdata)
					{   
						short	i, NumFlash=3; 
						BOOL	SavePatternBrush;
						COLORREF	SaveHC, FlashColor[3]={RGB(255,0,0),RGB(0,255,0),RGB(0,0,255)};
						
						if (!CurrentConfig)
							SetConfig (1);
						SetViewport(*pCommandViewport);
						if (InFlash)
							NumFlash--;  
						if (GetRowHighlightData (CurRow-startdata+1,&HighlightData)) 
						{
							PickList[0]=HighlightData.PD; 
							SaveHC = HighlightColor; 
							SavePatternBrush = PatternBrush;
							PatternBrush = FALSE; 
							SetNewBounds = FALSE;
							for (i=0;i<NumFlash*3;i++)
							{   
								HighlightColor = FlashColor[i%NumFlash];
					    		ShowPickedItem (CurView->hWnd,0); 
								Wait2(30);
					    	}  
					    	PatternBrush = SavePatternBrush;
							HighlightColor = SaveHC;
					    	ShowPickedItem (CurView->hWnd,0);
					    } 
					} 
					if (SetNewBounds)
                    	PostMessage(CurView->hWnd, WM_COMMAND, IDM_Z_REDRAW, 0L);
					if (CurRow < (nRow - 1) && InFlash)
					{
						//SendDlgItemMessage(hwnd,IDC_UGTABLE,TB_GOTOROW,0,CurRow+1); 
						DWORD wParam = MAKEWPARAM(IDC_UGTABLE, TB_GOTOROW);

						PostMessage(hwnd, WM_COMMAND, wParam, CurRow + 1);
					}
					else
						InFlash = FALSE;
					return 0;
                
                case ID_STOPFLASH:
                	InFlash = FALSE;
                	return 0;
                	
                case ID_FLASH_ALL:
					InFlash = TRUE; 
					if (CurRow == startdata)
						CurRow = startdata-1;
					else
						CurRow = startdata;
					//SendDlgItemMessage(hwnd,IDC_UGTABLE,TB_GOTOROW,0,CurRow);
					DWORD wParam = MAKEWPARAM(IDC_UGTABLE, TB_GOTOROW);

					PostMessage(hwnd, WM_COMMAND, wParam, CurRow);

                	return 0;
                	
                case ID_FLASH_STARTINGATCURRENTITEM: 
					InFlash = TRUE;  
					PostMessage(hwnd, WM_COMMAND, ID_FLASH_CURRENTITEM, 0L); 
                	return 0;
                	
				case ID_VLINES:{
					if(vlines==0){
						vlines=1;
					}
					else{
						vlines=0;
					}
					SendDlgItemMessage(hwnd,IDC_UGTABLE,TB_SETVLINES,vlines,0);
					return 0;
				}

				case ID_BESTFIT:{
					//clear the fit to window settings (just in case it was set)
					hmenu=GetMenu(hwnd);
					hmenu=GetSubMenu(hmenu,1);
					CheckMenuItem(hmenu,ID_FITTOWINDOW,MF_BYCOMMAND | MF_UNCHECKED);
					fittowindow=0;

					//search the whole table for the best coulmn widths
					//TB_BestFit((HWND) GetDlgItem(hwnd,IDC_UGTABLE),3,0,0);
					
					return 0;
				}

				case ID_FITTOWINDOW:{
					//check to  see if the menu item is checked or not
					hmenu=GetMenu(hwnd);
					hmenu=GetSubMenu(hmenu,1);
					t=GetMenuState(hmenu,ID_FITTOWINDOW,MF_BYCOMMAND);

					if(t & MF_CHECKED){
						//uncheck the menu item
						CheckMenuItem(hmenu,ID_FITTOWINDOW,MF_BYCOMMAND | MF_UNCHECKED);
						//clear the fit to window flag
						fittowindow=0;
					}
					else{
						//check the menu item
						CheckMenuItem(hmenu,ID_FITTOWINDOW,MF_BYCOMMAND | MF_CHECKED);
						//fit to window
						//TB_FitToWindow(GetDlgItem(hwnd,IDC_UGTABLE),0);
						//set the fit to window flag for future window sizings
						fittowindow=1;
					}

					return 0;
				}
				case ID_EXIT:{
					//close the dialog 
					Dock = 0;
		         	PostMessage(hwnd, WM_CLOSE, 0, 0L);
//					EndDialog(hwnd,0);
					return 0;

				}
			}
		}
	}
	
	return 0;
}    

void SetLinPrecision (HWND hWnd,short precision)
{
	short LinPrec[5]={ ID_OPTIONS_LINEARPRECISION_WHOLENUMBERS,
						ID_OPTIONS_LINEARPRECISION_01,
						ID_OPTIONS_LINEARPRECISION_001,
						ID_OPTIONS_LINEARPRECISION_0001,
						ID_OPTIONS_LINEARPRECISION_00001};

	PostMessage(hWnd,WM_COMMAND,LinPrec[precision],0);   
	return;
}
void SetAreaPrecision (HWND hWnd,short precision)
{
	short AreaPrec[5]={ ID_OPTIONS_AREAPRECISION_WHOLENUMBERS,
						ID_OPTIONS_AREAPRECISION_01,
						ID_OPTIONS_AREAPRECISION_001,
						ID_OPTIONS_AREAPRECISION_0001,
						ID_OPTIONS_AREAPRECISION_00001};

	PostMessage(hWnd,WM_COMMAND,AreaPrec[precision],0);   
	return;
}
