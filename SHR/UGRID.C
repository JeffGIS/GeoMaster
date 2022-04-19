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

//#include "ugrid.h" 
#include "graphint.h"
#include "ugtable.h"   

#include "gmextern.h"

static VARPNT	Varptr;
static LPSTR	pFile;
static RECT		UGridRect, GMGridRect;

long far pascal WidthDlgProc(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam);
long far pascal InformationDlgProc(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam);
long far pascal UGRID1DlgProc(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam);

int PixelsToDialogBoxTemplateUnits (HWND hWndDlg,int pixels)
{
	int		i=pixels;
	RECT	Rect={0,0,i,10};

	MapDialogRect (hWndDlg,&Rect);
	while (i > 0 && Rect.right > pixels)
	{
		i--;
		Rect.left = Rect.top = 0;
		Rect.right = i;
		Rect.bottom = 10;
		MapDialogRect (hWndDlg,&Rect);
	}
	return i;
}

BOOL WantGridVar (LPSTR Name,LPSTR ID)
{
	char	SearchStr[256];
	LPSTR	pList, loc, pStart;
	BOOL	rtn=FALSE;
	int		ln;

	if (!UgridVarList)
	{
		if (ID)
			strcpy (ID,Name);
		return TRUE;
	}
	pList = GlobalLock (UgridVarList);
	if (*pList == '(')
		pList++;
	loc = pList;
	pStart = loc;
	ln = strlen (Name);
	while ((loc = strstr (pStart,Name)))
	{
		if (*(loc + ln) == ',' || *(loc + ln) == '(' || *(loc + ln) == ')')
		{
			if (loc == pList || *(loc - 1) == ',')
			{
				if (*(loc + ln) == '(')
				{
					LPSTR pID = loc + ln + 1;
					LPSTR pEndID = strchr (pID,')');

					if (pEndID)
					{
						*pEndID = 0;
						if (ID)
							strcpy (ID,pID);
						*pEndID = ')';
					}
				}
				else
					strcpy (ID,Name);
				rtn = TRUE;
				break;
			}
		}
		pStart = loc + ln;
	}
	GlobalUnlock (UgridVarList);
	return rtn;
}


BOOL FAR PASCAL GMNGRID1DlgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
	int		Choice, CurLoc, Offset,ii, windoff=8; 
	HANDLE	hData;
	LPSTR	pData, pOutData;
	char	str[1024],txt[256];
	LPOPENFILEDATA  FilePtr;
	LPOPENSQLDATA   SQLPtr;  
	SIZE	txSize;
	static	HANDLE hDLT=0;  
	static	LPSHORT	pnDLTvar; 
	static	int		nDLTvar,nCol,tot;  
	static	LPSTR	DLTDelim;
	static	LPHANDLE DLTVar;
	static	LPSHORT	DLTStart;
	static	LPSHORT	DLTLen;
	static	LPSHORT	DLTType;
	static	char	Delim; 
	static	HANDLE	hFile;
	static	RECT	Rect;


  switch(Message)
   {
    case WM_INITDIALOG: 
		{
			LPSTR	pFile = GlobalLock (hScreenFile);
			HFILE	fptr = GSSiOpenFile(pFile,0,OF_READ);  
			int		l, maxrowlen, startindex,ncol,rowloc;
			HDC		hDC = GetDC (hWndDlg);
			RECT	ParentRect;
			LOGFONT	LogFont;    
			DWORD	TextExt; 
   			TEXTMETRIC	TextMet;
   			HFONT	OldFont, hFont;

			SetWindowText (hWndDlg,UGridTitle);
			_fmemset (&LogFont,0,sizeof(LOGFONT));
			LogFont.lfHeight = 8; 
			LogFont.lfEscapement = 0;   
			LogFont.lfWeight = FW_NORMAL;    
			LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
			LogFont.lfQuality = DEFAULT_QUALITY; 
			_fstrcpy (LogFont.lfFaceName,"Courier New");  
			hFont = CreateFontIndirect((LPLOGFONT)&LogFont);
			//OldFont = SelectObject (hDC,hFont);
			GetTextMetrics (hDC,&TextMet); 

			rowloc=GSSillseek(fptr,-8,2);
			rowloc=BigRead(fptr,(HPSTR)&l,4);  
			rowloc=BigRead(fptr,(HPSTR)&maxrowlen,4);
			startindex=GSSillseek(fptr,-(8+l*4),2);
			hData=GSSiGlobAlloc ( 389,GMEM_MOVEABLE,8192*2);
			pData=GlobalLock(hData);
			pOutData = pData + 8192;
			GSSillseek(fptr,0,0);
			nCol = ProcessDelimTextHeader(pData, pFile, fptr, &hDLT, 0, 0);
			GlobalUnlock (hScreenFile);
			hFile = hScreenFile;
			hScreenFile = 0;
			pnDLTvar = (LPSHORT)GlobalLock (hDLT); 
			nDLTvar = abs (*pnDLTvar);  
			DLTDelim = (LPSTR)(pnDLTvar + 1);
			DLTVar = (LPHANDLE)(DLTDelim + 1);
			DLTStart = (LPSHORT)(DLTVar + MAXDLTVAR);
			DLTLen = (LPSHORT)(DLTStart + MAXDLTVAR);
			DLTType = (LPSHORT)(DLTLen + MAXDLTVAR);
			Delim = *DLTDelim; 
			{
				int		ivar, bu, row=0;
				HANDLE	hTabStops = GSSiGlobAlloc (0,GHND,sizeof(int)*nDLTvar);
				LPINT	TabStops = GlobalLock (hTabStops);

				bu = GetDialogBaseUnits ();
				bu = LOWORD (bu)/2;
				CurLoc = GSSillseek(fptr,0,1);
				while (row++ < l && fgetstring (pData,4090,fptr))
				{
					GetDelimTextData(pData,hDLT,4090);
					*pOutData = 0;
					for (ivar=0;ivar<nDLTvar;ivar++)
					{   
						VARPNT	VarPtr = (VARPNT)GlobalLock (DLTVar[ivar]);
						
						if (WantGridVar (VarPtr->Name,0))
						{
							strcat (pOutData,VarPtr->Value);
							strcat (pOutData,"\t");
							sprintf (txt,"%s  ",VarPtr->Value);
							GetTextExtentPoint32 (hDC,txt,strlen(txt),&txSize);  
							TabStops[ivar] = max (TabStops[ivar],PixelsToDialogBoxTemplateUnits(hWndDlg,txSize.cx));// strlen (VarPtr->Value)*bu);
						}
						GlobalUnlock (DLTVar[ivar]);
					} 
					Choice = SendDlgItemMessage (hWndDlg,IDC_RECORDLIST,LB_ADDSTRING,0,(LPARAM)pOutData);
					SendDlgItemMessage (hWndDlg,IDC_RECORDLIST,LB_SETITEMDATA,Choice,(LPARAM)CurLoc);
					CurLoc = GSSillseek (fptr,0,1);
				}
				tot=0;
				*str = 0;
				for (ivar=0;ivar<nDLTvar;ivar++)
				{ 
					VARPNT	VarPtr = (VARPNT)GlobalLock (DLTVar[ivar]);
					char	HeaderID[128];
					//bStops[ivar] /= bu;
					if (WantGridVar (VarPtr->Name,HeaderID))
					{
						sprintf (txt,"%s  ",HeaderID);
						GetTextExtentPoint32 (hDC,txt,strlen(txt),&txSize);  
						TabStops[ivar] = max (TabStops[ivar],PixelsToDialogBoxTemplateUnits(hWndDlg,txSize.cx));
					//	tot = TabStops[ivar] = TabStops[ivar]/2 + 12 + tot;
						tot = TabStops[ivar] + tot;
						TabStops[ivar] = tot;
						strcat (str,HeaderID);
						strcat (str,"  ");
						GetTextExtentPoint32 (hDC,str,strlen(str),&txSize); 
						while (PixelsToDialogBoxTemplateUnits (hWndDlg,txSize.cx) < TabStops[ivar])
						{
							strcat (str," ");
							GetTextExtentPoint32 (hDC,str,strlen(str),&txSize);
						}
						TabStops[ivar]+=2;
					}
					GlobalUnlock (DLTVar[ivar]);
				}
				SendDlgItemMessage (hWndDlg,IDC_RECORDLIST,LB_SETTABSTOPS,nDLTvar,(LPARAM)TabStops);
				GSSiGlobUlFree (&hTabStops);
			}
			ii=strlen (str);
			SetDlgItemText (hWndDlg,IDC_HEADINGS,str);
			GSSiClose2 (&fptr);
			GSSiGlobUlFree (&hData);
			//SelectObject (hDC,OldFont);
			DeleteObject (hFont);
			ReleaseDC (hWndDlg,hDC);
			break;
	case GSSI_REPOSITION:
			if (lParam)
				GMGridRect = ((LPVIEWPORT)lParam)->ScreenRect;
			if (IsRectEmpty(&GMGridRect))
			{
				GetWindowRect (hWndDlg,&ParentRect);
				SetWindowPos(hWndDlg,HWND_TOP,Rect.left,Rect.top,
									 (tot-nCol*8)*2+4+windoff,
									 ParentRect.bottom-ParentRect.top,0);
			}
			else
			{
				ParentRect = GMGridRect;
				SetWindowPos(hWndDlg,HWND_TOP,ParentRect.left,ParentRect.top,
											  ParentRect.right-ParentRect.left,
											  ParentRect.bottom-ParentRect.top,0);
			}
			GetWindowRect (GetDlgItem (hWndDlg,IDC_HEADINGS),&Rect);
			ScreenRectToClientRect (hWndDlg,&Rect);
			SetWindowPos(GetDlgItem (hWndDlg,IDC_HEADINGS),HWND_TOP,Rect.left,Rect.top,
															 ParentRect.right-ParentRect.left-windoff*2,
															 Rect.bottom-Rect.top,0);
			GetWindowRect (GetDlgItem (hWndDlg,IDC_RECORDLIST),&Rect);
			ScreenRectToClientRect (hWndDlg,&Rect);
			SetWindowPos(GetDlgItem (hWndDlg,IDC_RECORDLIST),HWND_TOP,Rect.left,Rect.top,
															 ParentRect.right - ParentRect.left - windoff*2,
															 ParentRect.bottom-ParentRect.top - windoff - Rect.top,0);
			ii=SendDlgItemMessage(hWndDlg,IDC_RECORDLIST,LB_GETHORIZONTALEXTENT,0,0);
			ii=SendDlgItemMessage(hWndDlg,IDC_RECORDLIST,LB_SETHORIZONTALEXTENT,3500,0);
	}	
		
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
         break; /* End of WM_CLOSE                                      */
    
    case WM_DESTROY:
		GSSiGlobFree (&hFile);
		GSSiGlobUlFree (&hDLT);
   	 break;
    	 
    case WM_COMMAND:
    	 switch(LOWORD(wParam))
         {
            case IDC_RECORDLIST: 
	            switch(HIWORD(wParam))
	            {   
	                case LBN_DBLCLK:
	            	case LBN_SELCHANGE:
						Choice=SendDlgItemMessage(hWndDlg,IDC_RECORDLIST,LB_GETCURSEL,0,0); 
						if (Choice != LB_ERR)
						{ 
							LPSTR	pFile = GlobalLock (hFile);

							Offset = SendDlgItemMessage(hWndDlg,IDC_RECORDLIST,LB_GETITEMDATA,Choice,0); 
							sprintf (str,"$OPEN(GRID=%s,%%RECORDOFFSET==%i)",pFile,Offset);
							ProcessText (str);
							if (HIWORD(wParam) == LBN_DBLCLK)
								ProcessText (UGridPickMacroDblClk);
							else
								ProcessText (UGridPickMacro);
							sprintf (str,"$CLOSE(GRID)");
							ProcessText (str);
							GlobalUnlock (hFile);
			            }
						break;
					 break;    
				}
		        break; 
			case IDCANCEL:
            case IDC_EXIT:
        		 DestroyWindow(hWndDlg);
        		 hWndSELVAL = 0; 
                 break;
            
            case IDOK:
            {
            }
                break;

         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

HWND ShowGrid(HWND hWnd,LPSTR SavePosVName,LPSTR VPName){

	FARPROC dlgproc;
	int		rc;
	char	DialogName[32]="GMGRID_NOBORDER";
	int		VPID;
	LPVIEWPORT	SaveVP = CurView;
	static	HWND	hWndGrid=0;
    
	if (SavePosVName && *SavePosVName)
		hGridSize = AllocateVar(SavePosVName);
	GMGridRect.left = GMGridRect.right = -1;
    if (!VPName || !*VPName)
	{
		if (hWndGrid)
			DestroyWindow (hWndGrid);
		strcpy (DialogName,"GMGRID");
		hWndGrid = CreateDialog(hInst,"GMGRID_NOBORDER",hWnd, (DLGPROC)GMNGRID1DlgProc);
		SendMessage(CurView->hWndDlg, GSSI_REPOSITION,0, (LPARAM)0); 
		InvalidateRect(CurView->hWndDlg,0,TRUE);
	}
	else
	{
		if ((VPID = GetVPIDFromName (VPName)))
		{
			SetViewport (VPID); 
			GMGridRect = CurView->ScreenRect;
			if (CurView->hWndDlg)
				DestroyWindow (CurView->hWndDlg);
			CurView->hWndDlg = CreateDialog(hInst, "GMGRID_NOBORDER", hWnd, (DLGPROC)GMNGRID1DlgProc);
			SendMessage(CurView->hWndDlg, GSSI_REPOSITION,0, (LPARAM)CurView); 
			InvalidateRect(CurView->hWndDlg,0,TRUE);

		//	ClientRectToScreenRect (hWndMain,&GMGridRect);
		}
	}
	CurView = SaveVP;
	//setDoPaint( TRUE);
	//create a modal dialog box
	//dlgproc = (DLGPROC) MakeProcInstance((FARPROC)GMNGRID1DlgProc, hInst);
//	rc=DialogBox(hInst,"UGRID1",hWnd, dlgproc);
//	FreeProcInstance((FARPROC)dlgproc);

	return hWndGrid;
}

/*******************************************
********************************************/
/*long far pascal UGRID1DlgProc(HWND hwnd,UINT Message,WPARAM wParam,LPARAM lParam){

	//general purpose variables
	int  		t,x;
	long 		l,rowloc;
	double 	d;
	char 		string[50];
	RECT 		rect;
	HMENU 	hmenu;
	LPWINDOWPOS wp;
	DLGPROC dlgproc;  
	OFSTRUCT	OFStruct; 
	LPSTR	lpStr; 
	static	int		nCol; 
	char	val[256];
	HANDLE	hData;
	LPSTR	pData;


 	static TABLEINFO far *ti;


//	static DATA data;             //data file structure
	static HFILE fptr;				//file pointer to the data file
	static long lastrow=-1;			//saves the last row that the table asked for
	static int 	savecol[5];			//stores the column widths
	static int 	align=TA_LEFT;		//text alignment flag
	static int 	color=0;				//text color flag
	static int 	fittowindow=0;		//Fit-To-Window flag  
	static int	bestfit=1;
	static HBRUSH hbrush=0;			//brush handle
	static int vlines=1;				//separation lines ON/OFF flag   
	static long		maxrowlen, startindex;
	static RECT		WindRect; 
	static BOOL		First=TRUE;  
	LPSTR	pNonBlank, lpSizeStr, lpLoc,lpEnd;  
	LPSHORT	lpSize;
	HANDLE	hSize;
	static	HANDLE hDLT=0;                     
	short	n, icol, w;

	//process messages
	switch(message){

		case WM_INITDIALOG:{
            lastrow = -1;
			//create a gray brush (to be used for the dialog background)
			hbrush=CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

            if (!First)
            {  
				SetWindowPos(hwnd,HWND_TOP,WindRect.left,WindRect.top,
										   WindRect.right-WindRect.left,
										   WindRect.bottom-WindRect.top,SWP_NOZORDER);
            }
           	First = FALSE; 
			//open up the data file 
			pFile = GlobalLock (hScreenFile);
			fptr = GSSiOpenFile(pFile,&OFStruct,OF_READ);  
			if(fptr ==HFILE_ERROR){
				MessageBox(hwnd,"Data File failed to open!","Warning",MB_OK);
			}

			//get the number of records in the datafile    
			rowloc=GSSillseek(fptr,-8,2);
			rowloc=BigRead(fptr,(HPSTR)&l,4);  
			rowloc=BigRead(fptr,(HPSTR)&maxrowlen,4);
			startindex=GSSillseek(fptr,-(8+l*4),2);
			hData=GSSiGlobAlloc ( 389,GMEM_MOVEABLE,4096);
			pData=GlobalLock(hData);
			GSSillseek(fptr,0,0);
			fgetstring(pData,2044,fptr);
			nCol=ProcessDelimTextHeader(pData,pFile,fptr,&hDLT,0);   
			GlobalUnlock (hScreenFile);
			GSSiGlobUlFree (&hData);
			//get the tableinfo structure
			ti = (TABLEINFO far *)SendMessage(GetDlgItem(hwnd,IDC_UGTABLE),
         	TB_GETADDRESS,0,0);

			//set up the table    
			if (hGridSize)
			{
				hSize = GSSiGlobAlloc ( 390,GHND,4096);
				lpSizeStr = GlobalLock (hSize);
				lpSize = (LPSHORT)&WindRect;
				GetGlobalVal (hGridSize,lpSizeStr,0);
				if (!*lpSizeStr) 
				{
					lpSize = 0;
					goto NoSize;
				}
				lpLoc = lpSizeStr; 
				n = 4;
				while (n--)
				{
					if ((lpEnd = _fstrchr (lpLoc,',')))
						*lpEnd++ = 0;
					*lpSize++ = atoi (lpLoc);
					if (lpEnd)
						lpLoc = lpEnd;
				}
				SetWindowPos(hwnd,HWND_TOP,WindRect.left,WindRect.top,
										   WindRect.right-WindRect.left,
										   WindRect.bottom-WindRect.top,SWP_NOZORDER);
				lpSize = (LPSHORT)(lpSizeStr + 2048);
				n = nCol;
				while (n--)
				{
					if ((lpEnd = _fstrchr (lpLoc,',')))
						*lpEnd++ = 0;
					*lpSize++ = atoi (lpLoc);
					if (lpEnd)
						lpLoc = lpEnd;
				}
				lpSize = (LPSHORT)(lpSizeStr + 2048);
			}
			else
				lpSize = 0; 
NoSize:
			TB_SetupTable(hwnd,
				GetDlgItem(hwnd,IDC_UGTABLE),		//table window handle
				GetDlgItem(hwnd,IDC_UGTABLEHDG),   //table heading window handle
				GetDlgItem(hwnd,IDC_UGTABLEFTR),   //table footer window handle
				l,                               //number of rows
				nCol,                            //number of columns
				lpSize,                            //column widths
				3,                               //interspace value
				TRUE);                           //user resize option

            if (hGridSize)
            	GSSiGlobUlFree (&hSize);
            else
			//send a WM_SIZE (to adjust the windows to fit within the parent);
				PostMessage(hwnd,WM_SIZE,0,0);

			//set the focus to the table window
			SetFocus(GetDlgItem(hwnd,IDC_UGTABLE));

			return 0;
		}

		case WM_CTLCOLORDLG:{
			//if the dialog box is to be painted then return the gray brush
			if(HIWORD(lParam)==CTLCOLOR_DLG ||HIWORD(lParam)==CTLCOLOR_STATIC){
				SetBkColor(wParam,GetSysColor(COLOR_BTNFACE));
				return hbrush;
			}
			return NULL;
		}

		case WM_DESTROY:{
			//close the data file  
			GSSiClose2 (&fptr);
			//delete the brush
			GSSiDeleteObject(&hbrush);  
			if (hGridSize)
			{
				hSize = GSSiGlobAlloc ( 391,GHND,4096);
				lpSizeStr = GlobalLock (hSize);
				lpSize = (LPSHORT)&WindRect;
				n = 4;
				while (n--)
					sprintf (_fstrchr(lpSizeStr,0),"%i,",*lpSize++);
				for (icol=0;icol<nCol;icol++)
				{
					w=(int)SendDlgItemMessage(hwnd,IDC_UGTABLE,TB_GETCOLWIDTH,icol,0);
					sprintf (_fstrchr(lpSizeStr,0),"%i,",w);
				} 
				SetGlobalValue2 (hGridSize,lpSizeStr,0);
				GSSiGlobUlFree (&hSize);
			}
			
			return 0;
		}

		case WM_CLOSE:{
			//close the dialog
//			EndDialog(hwnd,0);     
			GSSiGlobFree (&hDLT);
			GetWindowRect (hwnd,&WindRect);
			DestroyWindow (hwnd);
			return 0;
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
		case WM_SIZE :{
			//resizes the child windows to fit inside the parent window

			//get the size of the client window
			GetClientRect(hwnd,&rect);
			//adjust the table child window to fit in the parent window
			SetWindowPos(GetDlgItem(hwnd,IDC_UGTABLE),HWND_TOP,5,32,rect.right-10,
				rect.bottom-30,SWP_NOZORDER);
			//adjust the heading child window to fit
			SetWindowPos(GetDlgItem(hwnd,IDC_UGTABLEHDG),HWND_TOP,5,7,
				rect.right-8-GetSystemMetrics(SM_CYVSCROLL),18,SWP_NOZORDER);

			//if the fittowindow flag is set then readjust the coulmn widths to
			//fit inside the new table child window width
			if(fittowindow){
				TB_FitToWindow(GetDlgItem(hwnd,IDC_UGTABLE),0);
			}
			if (bestfit)
				TB_BestFit((HWND) GetDlgItem(hwnd,IDC_UGTABLE),3,0,NULL);
			return 1;
		}

		case WM_COMMAND:{
			switch(wParam){

				case IDC_UGTABLE:{

					//find the message sent (ti->msg)
					switch((HIWORD(lParam))){
						case TBN_WANTTEXT :{
							// check to see if the row is the same as the last
							// if not then get a new record from the database
							// (this way a record doesnt need to be read in each time
							// a cell within the table needs to be drawn)
							hData=GSSiGlobAlloc ( 392,GMEM_MOVEABLE,4096);
							pData=GlobalLock(hData);
							if(ti->row!=lastrow){
								//find the record that coresponds to the row given
								lastrow=ti->row; 
								GSSillseek(fptr,startindex+ti->row*4,0);
								BigRead(fptr,(HPSTR)&rowloc,4);
								GSSillseek(fptr,rowloc,0);
								fgetstring (pData,(short)maxrowlen,fptr);
								GetDelimTextData(pData,hDLT);
							}
                            GSSiGlobUlFree (&hData);
							// get the field that co-responds to the column and put it
							// in the ti->buf parameter     
							Varptr = (VARPNT)GlobalLock(GetDLTVarHandle(ti->col,hDLT)); 
							_fstrncpy(val,Varptr->Value,Varptr->Len);   
							val[Varptr->Len]=0; 
							pNonBlank = FirstNonBlank(val);
							_fstrcpy(ti->buf,pNonBlank);
								
							GlobalUnlock(GetDLTVarHandle(ti->col,hDLT));
							ti->alignment=align;

							// menu selected color options
							if(color>0){
								//color every fifth line
								if(color==1){
									if((ti->row%5)==0){
										ti->textcolor=GetSysColor(COLOR_WINDOWTEXT);
										ti->backcolor=RGB(0,255,0);
									}
								}
								//color all outstanding balances
								else if(color==2){
								//	d=data.Balance;
									if(d>0){
										ti->textcolor=GetSysColor(COLOR_WINDOWTEXT);
										ti->backcolor=RGB(0,255,0);
									}
								}
								//color all selected
								else if(color==3){
								//	if(data.flag==1){
								//		ti->textcolor=GetSysColor(COLOR_HIGHLIGHTTEXT);
								//		ti->backcolor=GetSysColor(COLOR_HIGHLIGHT);
								//	}
								}
							}
							return 1;
						}

						case TBN_ROWCHANGE :{
							//display the new row number in the status window
//							wsprintf(string,"Row Changed To: %ld",ti->row);
//							SetDlgItemText(hwnd,IDC_STATUS,string);

							return 1;
						}

						case TBN_COLCHANGE :{
							//display the new column number in the status window
//							wsprintf(string,"Col Changed To: %d",ti->col);
//							SetDlgItemText(hwnd,IDC_STATUS,string);

							return 1;
						}

						case TBN_ROWSELECTED:{
							if (*UGridPickMacro)
                            {
                            	LPSTR pStr;
                            	HANDLE hStr;
                            	
								lastrow=ti->row; 
								GSSillseek(fptr,startindex+ti->row*4,0);
								BigRead(fptr,(HPSTR)&rowloc,4);
								GSSillseek(fptr,rowloc,0);
								hData=GSSiGlobAlloc ( 393,GMEM_MOVEABLE,4096);
								pData=GlobalLock(hData);
								fgetstring (pData,(short)maxrowlen,fptr);
								GetDelimTextData(pData,hDLT); 
								GSSiGlobUlFree (&hData);
								hStr = GSSiGlobAlloc ( 394,GMEM_MOVEABLE,4096);
								pStr = GlobalLock (hStr);
								_fstrcpy (pStr,UGridPickMacro);
								ExpandText(pStr);	
								GlobalUnlock (hStr);
								GSSiGlobUlFree (&hStr);
							}
							//if multiple selection is on then tag/untag the field
							//if(color==3){
								//retrive the current record
								//fseek(fptr,ti->row * sizeof(DATA),SEEK_SET);
							//	fread(&data,sizeof(DATA),1,fptr);

								//if it is not already selected then select it
							//	if(data.flag==0){
							//		data.flag=1;
							//	}
								//otherwise un-select it
							//	else{
							//		data.flag=0;
							//	}

							  	//save the record
							//	fseek(fptr,ti->row * sizeof(DATA),SEEK_SET);
							//	fwrite(&data,sizeof(DATA),1,fptr);

								//clear the lastrow flag
							//	lastrow=-1;

								//redraw the table so the changes will be shown
							//	TB_RedrawTable(GetDlgItem(hwnd,IDC_UGTABLE));
							//}
							//create a modal dialog box
						//	dlgproc = (DLGPROC) MakeProcInstance((FARPROC)InformationDlgProc, hInst);
						//	DialogBoxParam(hInst,"INFORMATION",NULL, dlgproc,
						//		GetDlgItem(hwnd,IDC_UGTABLE));
						//	FreeProcInstance((FARPROC)dlgproc);

							return 1;
						}

						case TBN_KEYBOARD:{
							//display the key that was hit
						//	wsprintf(string,"Key: %c",ti->wParam);
						//	SetDlgItemText(hwnd,IDC_STATUS,string);
							//search the database for the closest match
						//	t=0;					//set the counter to zero
						//	rewind(fptr);     //start from the beginning of the file
						//	while(1){
								//retrive a record
						//		x=fread(&data,sizeof(DATA),1,fptr);
						//		if(x==0){
                        //	t--;
						//			break;
						//		}

							//	if(data.Company[0] >= ti->wParam){
							//		break;
							//}
						//		t++;
						//	}
							//clear the lastrow flag
						//	lastrow=-1;

							//update the table position
						//	SendDlgItemMessage(hwnd,IDC_UGTABLE,TB_GOTOROW,0,t);

							return 1;
						}
					}
					return 1;
				}
				case IDC_UGTABLEHDG:{

					switch(ti->msg){
						case TBN_WANTHDG :{
							//set the text buffer to the column name    
							Varptr = (VARPNT)GlobalLock(GetDLTVarHandle(ti->col,hDLT)); 
							_fstrcpy(ti->buf,Varptr->Name);
							GlobalUnlock(GetDLTVarHandle(ti->col,hDLT));
							
							return 1;
						}
					}
					return 1;
				}

				case IDC_SEARCH:{
					//if the search edit box has changed do a new search
					//if(HIWORD(lParam)==EN_CHANGE){

						//get the text from the control
					//	GetDlgItemText(hwnd,IDC_SEARCH,string,50);

						//search the database for the closest match
					//	t=0;					//set the counter to zero
					//	rewind(fptr);     //start from the beginning of the file
					//	while(1){
							//retrive a record
						//	x=fread(&data,sizeof(DATA),1,fptr);
					//		if(x==0){
					//			t--;
					//			break;
					//		}

						//	if(_fstricmp(data.Company,string)>=0){
						//		break;
					//		}
					//		t++;
					//	}
						//clear the lastrow flag
					//	lastrow=-1;

						//update the table position
					//	SendDlgItemMessage(hwnd,IDC_UGTABLE,TB_GOTOROW,0,t);
				//	}
					return 0;
				}

				case ID_GOTO_TOP:{
					//update the table position
					SendDlgItemMessage(hwnd,IDC_UGTABLE,TB_GOTOROW,0,0);

					return 0;
				}

				case ID_GOTO_BOTTOM:{ 
				
				long	MaxRow;
					//get the number of records in the database
              // fseek(fptr,0,SEEK_END);
			//		l=(ftell(fptr) / sizeof(DATA)) -1;

					//update the table position
					MaxRow=(long)SendDlgItemMessage(hwnd,IDC_UGTABLE,TB_GETNUMROWS,0,0);
					SendDlgItemMessage(hwnd,IDC_UGTABLE,TB_GOTOROW,0,MaxRow-1);

					return 0;
				}

				case ID_SAVE_WIDTHS:{
					//retrieve the widths of the columns in the table and save them
					for(t=0;t<3;t++){
						savecol[t]=(int)SendDlgItemMessage(hwnd,IDC_UGTABLE,TB_GETCOLWIDTH,t,0);
					}

					return 0;
				}

				case ID_RESTORE_WIDTHS:{
					//use the previously saved column widths to set the coulmn withs
					for(t=0;t<3;t++){
						SendDlgItemMessage(hwnd,IDC_UGTABLE,TB_SETCOLWIDTH,
							t,savecol[t]);
					}
					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_UGTABLE));

					return 0;
				}

				case ID_TEXT_LEFT:{
					//set the alignment flag to 1 (1=left)
					align=TA_LEFT;
					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_UGTABLE));
					return 0;
				}

				case ID_TEXT_RIGHT:{
					//set the alignment flag to 2 (2=right)
					align=TA_RIGHT;
					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_UGTABLE));
					return 0;
				}

				case ID_TEXT_CENTER:{
					//set the alignment flag to 3 (3=center)
					align=TA_CENTER;
					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_UGTABLE));
					return 0;
				}

				case ID_COLOR_NONE:{
					//set the color flag to 0 (0=no color)
					color=0;
					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_UGTABLE));
					return 0;
				}

				case ID_COLOR_5:{
					//set the color flag to 0 (1=color every fifth line)
					color=1;
					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_UGTABLE));
					return 0;
				}

				case ID_COLOR_OUTSTANDING:{
					//set the color flag to 2 (0=color outstanding balances)
					color=2;
					//redraw the table
					TB_RedrawTable(GetDlgItem(hwnd,IDC_UGTABLE));
					return 0;
				}

				case ID_MULTIPLE:{

					//set the color flag to 3 (3=color selected records)
				//	color=3;

					//clear the all of the select fields in the database
				//	rewind(fptr);     //start from the beginning of the file
				//	while(1){
						//store the current file pos
					//	l = ftell(fptr);
						//retrive a record
					//	if( fread(&data,sizeof(DATA),1,fptr) ==0){
							//finished
					//		break;
					//	}
						//clear the flag
					//	data.flag=0;
                  //move back to the beginning of this record
					///	fseek(fptr,l,SEEK_SET);
						//save the record
					//	fwrite(&data,sizeof(DATA),1,fptr);
				//	}
				//	rewind(fptr);

					//clear the last database record position flag
				//	lastrow=-1;

					//redraw the table
				//	TB_RedrawTable(GetDlgItem(hwnd,IDC_UGTABLE));

					return 0;
				}

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

					t=GetMenuState(hmenu,ID_BESTFIT,MF_BYCOMMAND);

					if(t & MF_CHECKED){
						//uncheck the menu item
						CheckMenuItem(hmenu,ID_BESTFIT,MF_BYCOMMAND | MF_UNCHECKED);
						//clear the fit to window flag
						bestfit=0;
					}
					else{
						//check the menu item
						CheckMenuItem(hmenu,ID_BESTFIT,MF_BYCOMMAND | MF_CHECKED);
						//fit to window
						bestfit=1;
					}
					//search the whole table for the best coulmn widths
					
					PostMessage(hwnd,WM_SIZE,0,0);
					return 0;
				}

				case ID_FITTOWINDOW:{
					//check to  see if the menu item is checked or not
					hmenu=GetMenu(hwnd);
					hmenu=GetSubMenu(hmenu,1);
					CheckMenuItem(hmenu,ID_BESTFIT,MF_BYCOMMAND | MF_UNCHECKED);
					t=GetMenuState(hmenu,ID_FITTOWINDOW,MF_BYCOMMAND);
                    bestfit = 0;
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
						//set the fit to window flag for future window sizings
						fittowindow=1;
					}
					PostMessage(hwnd,WM_SIZE,0,0);

					return 0;
				}
				case ID_EXIT:{
					//close the dialog  
					DestroyWindow (hwnd); 
					GSSiGlobFree (&hDLT);
//					EndDialog(hwnd,0);
					return 0;

				}
			}
		}
	}
	
	return 0;
}
*/
