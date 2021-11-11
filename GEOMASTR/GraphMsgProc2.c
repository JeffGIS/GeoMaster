#include "graphint.h"   
#include "extrndb.h"
#include "shapefil.h"
#include "cddemo.h"    
#include "dibapi.h"      
#include "commctrl.h" 
#include "dgnlib.h"        

#include "gmextern.h"  

static	char	MsgAtPos[1024];  
static	int		MsgAtPosLoc; 
static	UINT	MsgAtPosOpt;
static	char	TypeName[5][8]={"Point","Line","Area","Text","Line"};

BOOL FAR PASCAL MESSAGEBOXATPOSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (443);
#endif
{ 	
 switch(Message)
   {
	 case WM_INITDIALOG:
	 {
		HDC hDC = GetDC(hWndDlg);
		SIZE txSize;
		int	iwidth, iheight, border, framewidth, move, i, nbuttons=3;
		RECT	rect, crect;
		HWND buttons[3];

		buttons[0] = GetDlgItem(hWndDlg, IDYES);
		buttons[1] = GetDlgItem(hWndDlg, IDNO);
		buttons[2] = GetDlgItem(hWndDlg, IDCANCEL);

		GetTextExtentPoint32(hDC, MsgAtPos, strlen(MsgAtPos), &txSize);
		ReleaseDC(hWndDlg, hDC);
		GetWindowRect(GetDlgItem(hWndDlg,IDC_MESSAGE), &rect);
		ScreenRectToClientRect(hWndDlg, &rect);
		border = rect.left;
		iheight = RECTHEIGHT(&rect);
		iwidth = max(RECTWIDTH(&rect), txSize.cx + 6);
		SetWindowPos(GetDlgItem(hWndDlg, IDC_MESSAGE), 0, rect.left, rect.top, iwidth, iheight, SWP_NOZORDER | SWP_NOOWNERZORDER);
		GetWindowRect(hWndDlg, &rect);
		GetClientRect(hWndDlg, &crect);
		framewidth = crect.left;
		iheight = RECTHEIGHT(&rect);
		iwidth = max(RECTWIDTH(&rect), txSize.cx + 6+border*2);
		move = (iwidth - RECTWIDTH(&rect))/2;
		SetWindowPos(hWndDlg, 0, rect.left, rect.top, iwidth+framewidth, iheight, SWP_NOZORDER | SWP_NOOWNERZORDER);

		for (i = 0; i < nbuttons; i++)
		{
			RECT brect;

			GetWindowRect(buttons[i], &brect);
			ScreenRectToClientRect(hWndDlg, &brect);
			SetWindowPos(buttons[i], 0, brect.left+move, brect.top, RECTWIDTH(&brect), RECTHEIGHT(&brect), SWP_NOZORDER | SWP_NOOWNERZORDER);
			if (i == 1)
				SetWindowPos(GetDlgItem(hWndDlg,IDOK), 0, brect.left + move, brect.top, RECTWIDTH(&brect), RECTHEIGHT(&brect), SWP_NOZORDER | SWP_NOOWNERZORDER);
		}
		switch (MsgAtPosOpt)
		{
		case MB_OK:
			ShowWindow(GetDlgItem(hWndDlg, IDOK), SW_SHOW);
			ShowWindow(GetDlgItem(hWndDlg, IDCANCEL), SW_HIDE);
			ShowWindow(GetDlgItem(hWndDlg, IDYES), SW_HIDE);
			ShowWindow(GetDlgItem(hWndDlg, IDNO), SW_HIDE);
			break;
		case MB_YESNO:
			ShowWindow(GetDlgItem(hWndDlg, IDOK), SW_HIDE);
			ShowWindow(GetDlgItem(hWndDlg, IDCANCEL), SW_HIDE);
			ShowWindow(GetDlgItem(hWndDlg, IDYES), SW_SHOW);
			ShowWindow(GetDlgItem(hWndDlg, IDNO), SW_SHOW);
			break;
		case MB_YESNOCANCEL:
			ShowWindow(GetDlgItem(hWndDlg, IDOK), SW_HIDE);
			ShowWindow(GetDlgItem(hWndDlg, IDCANCEL), SW_SHOW);
			ShowWindow(GetDlgItem(hWndDlg, IDYES), SW_SHOW);
			ShowWindow(GetDlgItem(hWndDlg, IDNO), SW_SHOW);
			break;
		case MB_OKCANCEL:
			ShowWindow(GetDlgItem(hWndDlg, IDOK), SW_SHOW);
			ShowWindow(GetDlgItem(hWndDlg, IDCANCEL), SW_SHOW);
			ShowWindow(GetDlgItem(hWndDlg, IDYES), SW_HIDE);
			ShowWindow(GetDlgItem(hWndDlg, IDNO), SW_HIDE);
			break;
		}
		SetDlgItemText(hWndDlg, IDC_MESSAGE, MsgAtPos);
		cwCenter(hWndDlg, MsgAtPosLoc);
	 }
         break;  
    case WM_CLOSE:
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */
/*	case WM_PAINT:
		{
			RECT	Rect;

			if (GetUpdateRect (hWndDlg,&Rect,TRUE))
			{
				PAINTSTRUCT	PaintSt;
				HBRUSH	hBr=CreateSolidBrush (RGB(196,196,255)), hOldBr;

				BeginPaint (hWndDlg,&PaintSt);
				hOldBr = SelectObject (PaintSt.hdc,hBr);
				//FillRect (PaintSt.hdc,&Rect,hBr);
				SelectObject (PaintSt.hdc,hOldBr);
				GSSiDeleteObject (&hBr);
				EndPaint (hWndDlg,&PaintSt);
			}
		}
			break;*/

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
			  case IDYES:
			  case IDOK:
			  {
						   SIZE txSize;
						   HDC hDC = GetDC(GetDlgItem(hWndDlg, IDC_MESSAGE));
						   GetTextExtentPoint32(hDC, MsgAtPos, strlen(MsgAtPos), &txSize);
						   ReleaseDC(GetDlgItem(hWndDlg, IDC_MESSAGE), hDC);
			  }
  					 EndDialog(hWndDlg, IDYES);
                  break;
			  case IDNO:
				  EndDialog(hWndDlg, IDNO);
				  break;
			  case IDCANCEL:
				  EndDialog(hWndDlg, IDCANCEL);
				  break;

           }
         break;   

    default:
{
#if ENABLETRACE
GSSiExitProg (443);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (443);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

int MessageBoxAtPosition (HWND hWnd, LPSTR MessIn, LPSTR TitleIn, UINT Flag,LPSTR Position)
{
	int	irc=0;

	strcpy (MsgAtPos,MessIn);
	MsgAtPosLoc = -2;
	if (*Position == 'W')
		MsgAtPosLoc = 0;
	MsgAtPosOpt = Flag;
	irc = DialogBox(hInst, (LPSTR)"MESSAGEBOXATPOS", hWnd, (DLGPROC)MESSAGEBOXATPOSMsgProc);
	return irc;
}

BOOL FAR PASCAL ADDLOC_CREATEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
    HCURSOR OldCursor=0;    
    static   HANDLE hSQL=0;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hBT;    
    long        Offset; 
    double      rtn;
    LPVOID      lpVal; 
    short         st, len,ifield;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    HANDLE      SaveHandle;
    BOOL        More, Err;  
    static		BOOL	FileIsOpen=FALSE;
    short         rc, LenUDI,LenAdditional,SkipNoGraphic; 
    static      short IMDataFileType;
    static      char    IMDataFile[MAX_PATH]; 
    HFILE		FidSave;
	OFSTRUCTGM	OFStruct;
    char	Ext[6]=".AL1";
    char	cHouseNum[128], cZIPCode[128], cCity[256], cStreetName[256], DestName[MAX_PATH], str[256], cUDIValue[128], cAdditionalValue[256];
	char	Name[128], cPrefix[16], cDelims[32], XCoord[128], YCoord[128]; 
	static	BOOL	RecalledName;
	UINT	nPrompts=16;
	UINT	PrmtDat[17*3] = { 
			AL_DATABASE_LIST, PRMT_AL_DATABASE_LIST,0,
			AL_TABLE_NAMES, PRMT_AL_TABLE_NAMES, 0,
			IDC_SETSQL, PRMT_IDC_SETSQL, MORE_IDC_SETSQL,
			IDC_ALSQL, PRMT_IDC_ALSQL, 0,
			IDC_HOUSE_FIELD, PRMT_IDC_HOUSE_FIELD,0,
			IDC_MULT_HOUSENUMS, PRMT_IDC_MULT_HOUSENUMS, MORE_IDC_MULT_HOUSENUMS,
			IDC_STREETNAME_FIELD, PRMT_IDC_STREETNAME_FIELD, 0,
			IDC_ZIPCODE_FIELD, PRMT_IDC_ZIPCODE_FIELD, MORE_IDC_ZIPCODE_FIELD,
			IDC_UDI_VALUE, PRMT_IDC_UDI_VALUE, 0,
			IDC_UDILENGTH, PRMT_IDC_UDILENGTH, 0,
			IDC_ADDITIONAL_FIELD, PRMT_IDC_ADDITIONAL_FIELD, 0,
			IDC_ADDITIONAL_LENGTH, PRMT_IDC_ADDITIONAL_LENGTH, 0,
			IDC_SKIP_NOGRAPHIC, PRMT_IDC_SKIP_NOGRAPHIC, MORE_IDC_SKIP_NOGRAPHIC,    
			IDC_ALFPREFIX, PRMT_IDC_ALFPREFIX,MORE_IDC_ALFPREFIX,
			IDC_ALDEST_FILE, PRMT_IDC_ALDEST_FILE, 0,
			IDC_HNDELIM, PRMT_IDC_HNDELIM, 0,
			IDC_ALNEWFILE, PRMT_ALNEWFILE,0
			};
	int	l;
 int	BRtn;
 if (Message==WM_INITDIALOG)
 {  
 	UINT	pw=0, pn=1, pm=2;
 	
 	InitDlgPrompts (hWndDlg);
 	while (nPrompts--)
 	{
	 	SetDlgPrompt (GetDlgItem(hWndDlg,PrmtDat[pw]),PrmtDat[pn],PrmtDat[pm]);
	 	pw+=3;
	 	pn+=3; 
	 	pm+=3;
	}
 }

 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam,0,
                     SV_SET_FILE, AL_DATABASE_LIST, AL_TABLE_NAMES,SV_TABLE_HEADING, 0,0,
                     IMDataFile, &IMDataFileType, &hSQL,0,TRUE))  return TRUE;
 switch(Message)
   {             
 	case WM_DESTROY: 
	 	InitDlgPrompts (0);    
	 	break;
    case WM_INITDIALOG:
    	 FileIsOpen = FALSE;
     	 ClearDlgPrompts ();
         _fstrcpy (CurHelpTopic,"Direct Address Location");
    	 RecalledName=FALSE; 
    	 SetDlgItemText (hWndDlg,IDC_ALSQL,"ALL ROWS");
    	 SetDlgItemText (hWndDlg,IDC_HNDELIM,"100");
         SendDlgItemMessage (hWndDlg,IDC_ALNEWFILE,(UINT)BM_SETCHECK,(WPARAM)TRUE,(LPARAM)0L);
         if (*AutoExportName)
		 	PostMessage(hWndDlg, WM_COMMAND, IDC_RECALL, 0L);
    case GSSI_REINITDIALOG:
         EnableWindow (GetDlgItem(hWndDlg,IDC_DELIMS_TITLE),
         				(BOOL)SendDlgItemMessage (hWndDlg,IDC_MULT_HOUSENUMS,
         				(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)); 
         EnableWindow (GetDlgItem(hWndDlg,IDC_HNDELIM),
         				(BOOL)SendDlgItemMessage (hWndDlg,IDC_MULT_HOUSENUMS,
         				(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)); 
         EnableWindow (GetDlgItem(hWndDlg,IDC_ALFPREFIX),
         				(BOOL)SendDlgItemMessage (hWndDlg,IDC_SKIP_NOGRAPHIC,
         				(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)); 
         EnableWindow (GetDlgItem(hWndDlg,IDC_ALFPREFIX_TITLE),
         				(BOOL)SendDlgItemMessage (hWndDlg,IDC_SKIP_NOGRAPHIC,
         				(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)); 
         if (FileIsOpen && *AutoExportName)
	         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

         switch(LOWORD(wParam))

         {  
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)  
                 {
					 SetContinueProcessing(FALSE);
	                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Canceled");
				 }                    
                 else
        			 PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
                 break;
            
            case IDC_EXIT: 
                 if (!RecalledName)
                 	goto NoUpdate;   
                 GetCurVal (Name,sizeof(Name),IDS_FILEAL1);
               	 goto Update;
            case IDC_SAVE:
            {    
            	 short	Version=2;
            	 
                 if (!GetSaveName2 (hWndDlg,Name,0,Ext,IDS_FILEAL1)) break; 
        Update:  Version = 3;
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
                 BigWrite (FidSave,(HPSTR)Ext,6,-1);
                 BigWrite (FidSave,(HPSTR)&Version,2,-1); 
                 GetDlgItemText (hWndDlg,AL_DATABASE_LIST,IMDataFile,MAX_PATH-1);  
                 BigWrite (FidSave,(HPSTR)IMDataFile,MAX_PATH,-1);
                 GetDlgItemText (hWndDlg,IDC_ALSQL,str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_ALDEST_FILE,DestName,MAX_PATH-1);
                 BigWrite (FidSave,(HPSTR)DestName,MAX_PATH,-1);
                 GetDlgItemText (hWndDlg,IDC_HOUSE_FIELD,cHouseNum,sizeof(cHouseNum));
                 BigWrite (FidSave,(HPSTR)cHouseNum,sizeof(cHouseNum),-1);
                 GetDlgItemText (hWndDlg,IDC_STREETNAME_FIELD,cStreetName,sizeof(cStreetName)); 
                 BigWrite (FidSave,(HPSTR)cStreetName,sizeof(cStreetName),-1);
                 GetDlgItemText (hWndDlg,IDC_CITY_FIELD,cCity,sizeof(cCity)); 
                 BigWrite (FidSave,(HPSTR)cCity,sizeof(cCity),-1);
                 GetDlgItemText (hWndDlg,IDC_ZIPCODE_FIELD,cZIPCode,sizeof(cZIPCode)); 
                 BigWrite (FidSave,(HPSTR)cZIPCode,sizeof(cZIPCode),-1);
                 GetDlgItemText (hWndDlg,IDC_UDI_VALUE,cUDIValue,sizeof(cUDIValue)); 
                 BigWrite (FidSave,(HPSTR)cUDIValue,sizeof(cUDIValue),-1);
                 LenUDI = GetDlgItemInt (hWndDlg,IDC_UDILENGTH,&Err,FALSE);
                 BigWrite (FidSave,(HPSTR)&LenUDI,2,-1);
                 GetDlgItemText (hWndDlg,IDC_ADDITIONAL_FIELD,cAdditionalValue,sizeof(cAdditionalValue)); 
                 BigWrite (FidSave,(HPSTR)cAdditionalValue,sizeof(cAdditionalValue),-1);
                 LenAdditional = GetDlgItemInt (hWndDlg,IDC_ADDITIONAL_LENGTH,&Err,FALSE);
                 BigWrite (FidSave,(HPSTR)&LenAdditional,2,-1);
                 GetDlgItemText (hWndDlg,IDC_ALFPREFIX,cPrefix,sizeof(cPrefix)); 
                 BigWrite (FidSave,(HPSTR)cPrefix,sizeof(cPrefix),-1);
                 SkipNoGraphic = SendDlgItemMessage (hWndDlg,IDC_SKIP_NOGRAPHIC,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
                 BigWrite (FidSave,(HPSTR)&SkipNoGraphic,2,-1);
                 GetDlgItemText (hWndDlg,IDC_HNDELIM,cDelims,sizeof(cDelims)); 
                 BigWrite (FidSave,(HPSTR)cDelims,sizeof(cDelims),-1);
                 SkipNoGraphic = SendDlgItemMessage (hWndDlg,IDC_MULT_HOUSENUMS,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
                 BigWrite (FidSave,(HPSTR)&SkipNoGraphic,2,-1);
				 SkipNoGraphic = (BOOL)SendDlgItemMessage (hWndDlg,IDC_ALNEWFILE,BM_GETCHECK,0,0);
                 BigWrite (FidSave,(HPSTR)&SkipNoGraphic,2,-1);
                 GSSiClose2 (&FidSave);
            } 
        NoUpdate:   
            	 if (wParam == IDC_EXIT)
            	 {
	                 if (hSQL)
	                    CloseDataFile (TRUE, &hSQL);  
					 DestroyFieldList ();
					 EndDialog(hWndDlg, TRUE);
                 }
                    
            	 break;
            	 
           	case IDC_RECALL: 
           	{
           		 short Version;
           		 
             	 if (!*AutoExportName)
             	 { 
					 if (!GetFileName2 (hWndDlg,Name,Ext,IDS_FILEAL1))
					 	break;
	             }
	             else
	             	_fstrcpy (Name,AutoExportName);  
				 RecalledName=TRUE;
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_READ);
                 BigRead (FidSave,Ext,6);
                 BigRead (FidSave,(HPSTR)&Version,2); 
				 if (Version > 2)
					 l = MAX_PATH;
				 else
					 l = 128;
                 BigRead (FidSave,IMDataFile,l);
                 BigRead (FidSave,str,256);
                 SetDlgItemText (hWndDlg,IDC_ALSQL,str);                
				 if (Version > 2)
					 l = MAX_PATH;
				 else
					 l = 128;
                 BigRead (FidSave,DestName,l);
                 SetDlgItemText (hWndDlg,IDC_ALDEST_FILE,DestName);
                 BigRead (FidSave,cHouseNum,sizeof(cHouseNum));
                 SetDlgItemText (hWndDlg,IDC_HOUSE_FIELD,cHouseNum);
                 BigRead (FidSave,cStreetName,sizeof(cStreetName));
                 SetDlgItemText (hWndDlg,IDC_STREETNAME_FIELD,cStreetName);
                 if (Version > 1)
                 { 
                 	BigRead (FidSave,cCity,sizeof(cCity));
                 	SetDlgItemText (hWndDlg,IDC_CITY_FIELD,cCity);
                 } 
                 BigRead (FidSave,cZIPCode,sizeof(cZIPCode));
                 SetDlgItemText (hWndDlg,IDC_ZIPCODE_FIELD,cZIPCode); 
                 BigRead (FidSave,cUDIValue,sizeof(cUDIValue));
                 SetDlgItemText (hWndDlg,IDC_UDI_VALUE,cUDIValue); 
                 BigRead (FidSave,(HPSTR)&LenUDI,2);
                 SetDlgItemInt (hWndDlg,IDC_UDILENGTH,LenUDI,FALSE);
                 BigRead (FidSave,cAdditionalValue,sizeof(cAdditionalValue));
                 SetDlgItemText (hWndDlg,IDC_ADDITIONAL_FIELD,cAdditionalValue); 
                 BigRead (FidSave,(HPSTR)&LenAdditional,2);
                 SetDlgItemInt (hWndDlg,IDC_ADDITIONAL_LENGTH,LenAdditional,FALSE);
                 BigRead (FidSave,cPrefix,sizeof(cPrefix));
                 SetDlgItemText (hWndDlg,IDC_ALFPREFIX,cPrefix); 
                 BigRead (FidSave,(HPSTR)&SkipNoGraphic,2);
                 SendDlgItemMessage (hWndDlg,IDC_SKIP_NOGRAPHIC,(UINT)BM_SETCHECK,(WPARAM)SkipNoGraphic,(LPARAM)0L);
                 BigRead (FidSave,cDelims,sizeof(cDelims));
                 SetDlgItemText (hWndDlg,IDC_HNDELIM,cDelims); 
                 BigRead (FidSave,(HPSTR)&SkipNoGraphic,2);
                 SendDlgItemMessage (hWndDlg,IDC_MULT_HOUSENUMS,(UINT)BM_SETCHECK,(WPARAM)SkipNoGraphic,(LPARAM)0L);
				 if (Version > 2)
				 {
					BigRead (FidSave,(HPSTR)&SkipNoGraphic,2);
					SendDlgItemMessage (hWndDlg,IDC_ALNEWFILE,(UINT)BM_SETCHECK,(WPARAM)SkipNoGraphic,(LPARAM)0L);
				 }
                 GSSiClose2 (&FidSave);    
                 FileIsOpen = TRUE;
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
            }
           		 break;
           	
           	case IDC_MULT_HOUSENUMS:
           	case IDC_SKIP_NOGRAPHIC:
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
           		 break;
           		 	 
            case IDC_LOCATE_DEST: 
                 *str=0;
                 if (SendDlgItemMessage (hWndDlg,IDC_ALNEWFILE,BM_GETCHECK,0,0)) 
                 {
                    if (!GetSaveName2 (hWndDlg,str,IDS_FILTERGWD,".GMD",IDS_FILEGMD)) break;   
                 }
                 else  
                 {
                    if (!GetFileName3 (hWndDlg,str,IDS_FILTERGWD,IDS_FILEGMD)) break;   
                 }
                 SetDlgItemText (hWndDlg,IDC_ALDEST_FILE,str);
                 break;
                  
            case IDC_OPEN_DB:
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),TRUE);
                 break;  
            
            case IDC_SHOW_FIELDS: 
            	 
            	 DisplayFieldList (hWndDlg,hSQL,0,0,0);
                 break;
                      
            case IDC_SETSQL:      
            {    
                 HANDLE hMem;
                 LPSTR  lpStr;
                 
                 hMem = GSSiGlobAlloc ( 569,GHND,4096);
                 lpStr = GlobalLock (hMem); 
                 GetDlgItemText (hWndDlg,IDC_ALSQL,lpStr,1024);
                 if (GetSQLWhereClause (hWndDlg, hSQL, lpStr))
                 	SetDlgItemText (hWndDlg,IDC_ALSQL,lpStr);    
                 GSSiGlobUlFree (&hMem);
                 break;
            }
            case IDOK: 
            {
                 long   lineno=0, CurLoc, MidLine, ii, TotLen, NumSkip=0, nLoaded=0,MunicNum;  
                 long   NewRefno, HNum, NumHNum, MaxMultNum=100; 
                 BOOL   Done, First=TRUE, MultHNum; 
                 HANDLE hDBDest;
                 LPSTR  lpTAB, pSQL; 
                 LPSTR  lpDot; 
                 UINT	len;  
                 short	LenUDI;
			     LPGWDHEADER	lpGWDHead; 
			     char	OrigcHouse[64];  
			     long	StreetNum;
				 double	XVal, YVal;
                 
                 Processing = TRUE;
				 SetContinueProcessing(TRUE);
                 if (!GetDlgItemText (hWndDlg,IDC_ALDEST_FILE,DestName,lnDestName))
                 {
                    GSSiMsgBox(GetFocus(),"No destination file", 0,MB_ICONQUESTION|MB_OK,0);
                    break;
                 }
                 GetDlgItemText (hWndDlg,IDC_HOUSE_FIELD,cHouseNum,sizeof(cHouseNum));
                 GetDlgItemText (hWndDlg,IDC_STREETNAME_FIELD,cStreetName,sizeof(cStreetName)); 
                if (!*cStreetName ||!*cHouseNum)
                {
                    GSSiMsgBox(GetFocus(),"House Number and/or Street Name field missing", 0,MB_ICONQUESTION|MB_OK,0);
                    break;
                }
                MultHNum = SendDlgItemMessage (hWndDlg,IDC_MULT_HOUSENUMS,
         										(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L); 
         		if (MultHNum)
	                MaxMultNum = GetDlgItemInt (hWndDlg,IDC_HNDELIM,&Err,FALSE);
                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading Data");
                CloseDataFile (TRUE, &hSQL);  
                GetDlgItemText (hWndDlg,IDC_ALSQL,str,sizeof(str));                
                pSQL = str;
                if (!_fstrcmp (pSQL,"ALL ROWS"))
                    *pSQL = 0;
                hSQL = 0;
                if (!OpenDataFile (IMDataFile,pSQL,BT_READ,&hSQL))
                {  
					char mess[512];
					sprintf(mess, "Cannot open data file:%s", IMDataFile);
					GSSiMsgBox(GetFocus(), mess, 0, MB_ICONQUESTION | MB_OK, 0);
                    break;
                }
                
                LenUDI = GetDlgItemInt (hWndDlg,IDC_UDILENGTH,&Err,FALSE);
                LenAdditional = GetDlgItemInt (hWndDlg,IDC_ADDITIONAL_LENGTH,&Err,FALSE);
                
				hDBDest = CreateAddLocTable (DestName,LenUDI,LenAdditional,(BOOL)SendDlgItemMessage (hWndDlg,IDC_ALNEWFILE,BM_GETCHECK,0,0));
                  
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE); 
                DisableHalt = TRUE;

                if (First)
                {   
                	HCURSOR	OldCursor;
                	
                    OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT));
                    TotLen = NumSQLRows (hSQL);  
		            GSSiSetCursor (OldCursor);
                }
                First = FALSE;
                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading Data");
                
                Done = FALSE; 
               	STNDSN_INIT(FALSE);
        NextLine:
                 if (!ContinueProcessing || !FetchDBRec (hSQL))
                    goto EndFile; 
                    
                 GetDlgItemText (hWndDlg,IDC_HOUSE_FIELD,cHouseNum,sizeof(cHouseNum));
                 GetDlgItemText (hWndDlg,IDC_STREETNAME_FIELD,cStreetName,sizeof(cStreetName)); 
                 GetDlgItemText (hWndDlg,IDC_UDI_VALUE,cUDIValue,sizeof(cUDIValue)); 
                 GetDlgItemText (hWndDlg,IDC_CITY_FIELD,cCity,sizeof(cCity)); 
                 GetDlgItemText (hWndDlg,IDC_ZIPCODE_FIELD,cZIPCode,sizeof(cZIPCode)); 
                 ExpandText(cHouseNum);  
                 ExpandText(cStreetName); 
                 OneSpace (cStreetName);  
                 ExpandText (cCity); 
                 ExpandText (cZIPCode);
                 ExpandText (cUDIValue); 
				 XVal = 0;
				 YVal = 0;
                 if (SendDlgItemMessage (hWndDlg,IDC_SKIP_NOGRAPHIC,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
                 {
                 	GetDlgItemText (hWndDlg,IDC_ALFPREFIX,cPrefix,sizeof(cPrefix)); 
                 	if (!PickByRefno (0,cPrefix,cUDIValue,-1) || PickList[0].IsDeleted)
                 	{
                 		NumSkip++;
                 		goto SkipRec;
                 	}
					else
					{
						XVal = PickList[0].BeginPoint.x;
						YVal = PickList[0].BeginPoint.y;
					}
                 }
				 else if (GetDlgItemText (hWndDlg,IDC_XCOORD,XCoord,sizeof(XCoord))>0)
				 {
					 GetDlgItemText (hWndDlg,IDC_YCOORD,YCoord,sizeof(YCoord));
					 ExpandText (XCoord);
					 ExpandText (YCoord);
					 XVal = atof (XCoord);
					 YVal = atof (YCoord);
				 }

                 HNum = atol (cHouseNum);   
                 _fstrcpy (OrigcHouse,cHouseNum);
                 if (MultHNum)
                 { 
                 	LPSTR	lpDash = _fstrrchr (cHouseNum,'-'), lpEnd;
                 	long	EndHouse;    
                 	short	l;
                 	
                 	if (!lpDash)
                 		NumHNum = 1;
                 	else
                 	{   
                 		*lpDash++=0;
                 		l = _fstrspn (lpDash,"0123456789");
                 		if (l)
                 		{   
                 			lpEnd = lpDash + l;
                 			*lpEnd = 0;
                 		}
                 		EndHouse = atol (lpDash); 
                 		if (EndHouse < HNum)
                 		{
                 			OneSpace (cHouseNum);
                 			OneSpace (lpDash);
                 			l = _fstrlen (lpDash);
                 			lpEnd = _fstrchr (cHouseNum,0);
                 			lpEnd -= l;
                 			_fstrcpy (lpEnd,lpDash);
                 			EndHouse = atol (cHouseNum);	
                 		}
                 		NumHNum = (EndHouse - HNum)/2 + 1;
                 	}
                 }
                 else
                 	NumHNum = 1; 
                 if (NumHNum < 0 || NumHNum > MaxMultNum)
                 	NumHNum = 1;
                 while (NumHNum--)
                 {
				     lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest); 
				     lpGWFldInfo=lpGWDHead->pFldInfo; 
					 SetFieldValFromChar(lpGWDHead, lpGWFldInfo++, cStreetName, FALSE, FALSE, TRUE);
				     SetFieldValFromLong(lpGWDHead,lpGWFldInfo++,HNum);
					 SetFieldValFromChar(lpGWDHead, lpGWFldInfo++, cZIPCode, FALSE, FALSE, TRUE);
					 SetFieldValFromChar(lpGWDHead, lpGWFldInfo++, cUDIValue, FALSE, FALSE, TRUE);
					 STNDST(cStreetName, _fstrlen(cStreetName),STDNAMv,NRONAMv,NMONLYv,
							     SANSCHv,NANDCHv,NCMPNMv,ORIGNMv,SANSCPv,SANSCSv,0,0,0,0);   
					 StreetNum = GetStreetNumFromName (STDNAMv,2,BT_FIRST,str);  
					 if (!StreetNum && *cStreetName)
		                 StreetNum = AddStreetName (cStreetName,0,"","","",""); 
				     SetFieldValFromLong(lpGWDHead,lpGWFldInfo++,StreetNum);
	                 MunicNum = GetMunicFromName (cCity);
				     SetFieldValFromLong(lpGWDHead,lpGWFldInfo++,MunicNum);
				     SetFieldValFromReal(lpGWDHead,lpGWFldInfo++,XVal);
				     SetFieldValFromReal(lpGWDHead,lpGWFldInfo++,YVal);
	                 if (LenAdditional)
	                 {
		                 GetDlgItemText (hWndDlg,IDC_ADDITIONAL_FIELD,cAdditionalValue,sizeof(cAdditionalValue)); 
	                     ExpandText (cAdditionalValue);  
						 SetFieldValFromChar(lpGWDHead, lpGWFldInfo, cAdditionalValue, FALSE, FALSE, TRUE);
	                 }
					 GWDFormKey(lpGWDHead,0,FALSE,0,0);
					 if (BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_FIRST,BT_EQ,(LPSTR)&Offset))
					 {
		 				nLoaded++;   
						GWDAddRecord (lpGWDHead,0,0);
					 }
					 GlobalUnlock (hDBDest);
					 HNum += 2; 
				 }
		SkipRec:
                 lineno++; 
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, lineno,0);
                 goto NextLine;
                 
        ErrorEnd: 
                 {  
                    char    mess[256];
                    
                    sprintf (mess,"Error in file at line %ld\r\n%s",lineno,str);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK,0);
                 }
                    
        EndFile: 
                 CloseDataFile (TRUE, &hSQL);  
                 if (!ContinueProcessing)
					 sprintf (str,"Cancelled - %ld loaded, %ld skipped",nLoaded,NumSkip); 
				 else
				 	 sprintf (str,"Finished - %ld loaded, %ld skipped",nLoaded,NumSkip);
	             SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,str);
				 CloseGWDatabase (hDBDest); 
                 DisableHalt = FALSE; 
        Reset:    
        		 Processing = FALSE;
                 DisableHalt = FALSE;
        		 SetContinueProcessing ( TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE); 
				 if (*AutoExportName)
  					 EndDialog(hWndDlg, TRUE);
				 else
	                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); // datafile should always be open so SQL and FIELDS work
                 break;
                 
            }   
          }
          break;

    default:
        return FALSE;
   }
 return TRUE;    
}

BOOL FAR PASCAL CONFIGPARMSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (441);
#endif
{ 	
 
 static	HANDLE	hSaveBM=0;
 int	BRtn;  
 char	str[256];  
 LPSTR	pStr;
 LPSTR	pConfigDescription;
 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (441);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG: 
         cwCenter(hWndDlg, 0);
    	 //hSaveBM = EnterBlockingWindow (hWndDlg);
   	 	 pStr = GlobalLock (hStartupCommand);
    	 SetDlgItemText (hWndDlg,IDC_STARTCMD,pStr);
    	 GlobalUnlock (hStartupCommand);
   	 	 pStr = GlobalLock (hStartupMenu);
    	 SetDlgItemText (hWndDlg,IDC_STARTMENU,pStr);
    	 GlobalUnlock (hStartupMenu);
    	 if (GetNumInfoBox ())
    	 	EnableWindow (GetDlgItem(hWndDlg,IDC_SAVEINFOBOX),TRUE);
         if (BT_NUM_IN_INDEX (hHighlight))
    	 	EnableWindow (GetDlgItem(hWndDlg,IDC_SAVEHLTLIST),TRUE); 
    	 if (NumViewportsArray[0]) 
    	 {
	   	 	EnableWindow (GetDlgItem(hWndDlg,IDC_LINKMENUS),TRUE);
	   	 	EnableWindow (GetDlgItem(hWndDlg,IDC_EMBEDMENUS),TRUE);
	   	 } 
	   	 if (hPDChunk)
	   	 	EnableWindow (GetDlgItem(hWndDlg,IDC_SAVEPRINTSETUP),TRUE);
    	 SendDlgItemMessage (hWndDlg,IDC_SAVEGLOBALS,BM_SETCHECK,SaveGlobals,0);
    	 SendDlgItemMessage (hWndDlg,IDC_SAVEINFOBOX,BM_SETCHECK,SaveInfoBox,0);
    	 SendDlgItemMessage (hWndDlg,IDC_SAVEHLTLIST,BM_SETCHECK,SaveHLTList,0);
//    	 SendDlgItemMessage (hWndDlg,IDC_SAVEIMAGE,BM_SETCHECK,SaveCfgImage,0);
    	 SendDlgItemMessage (hWndDlg,IDC_LINKMENUS,BM_SETCHECK,SaveMenuName,0);
    	 SendDlgItemMessage (hWndDlg,IDC_SAVEPOS,BM_SETCHECK,SaveCfgSizePos,0);
    	 if (EmbededMenuLoc) 
    	 {
	   	 	 EnableWindow (GetDlgItem(hWndDlg,IDC_LINKMENUS),FALSE);
	    	 SendDlgItemMessage (hWndDlg,IDC_EMBEDMENUS,BM_SETCHECK,TRUE,0);
	     } 
    	 SendDlgItemMessage (hWndDlg,IDC_INCLUDEPROMPT,BM_SETCHECK,GetGlobalBVal("[%PROMPTS]"),0);
    	 if (!pViewportsD[0]->Type && pViewportsD[0]->AutoSize) 
    	 {
	   	 	 EnableWindow (GetDlgItem(hWndDlg,IDC_STARTFULL),TRUE);
	    	 SendDlgItemMessage (hWndDlg,IDC_STARTFULL,BM_SETCHECK,pViewportsD[0]->ShowFullScreen,0);
	     } 
	     if (hConfigDescription)
	     {
	     	pConfigDescription = GlobalLock (hConfigDescription);
	     	SetDlgItemText (hWndDlg,IDC_CONFIGDESC,pConfigDescription);
   			GlobalUnlock (hConfigDescription);
	     }

         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
			case IDC_WINDOWCOLOR:
			{
				COLORREF	Color = WindowColor;
				
				 if (GetColor(hWndMain,&Color))
				 	WindowColor = Color; 
			}
				 break;
		
			case IDC_CANCELIMAGES:
				EscapeFunction (TRUE);
				SetContinueProcessing (FALSE);
		    	EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE);  
				break;
			case IDC_SAVEPOS:
				SendDlgItemMessage (hWndDlg,IDC_SAVEPOS_ALL,BM_SETCHECK,FALSE,0L);
				break;
			case IDC_SAVEPOS_ALL:
				SendDlgItemMessage (hWndDlg,IDC_SAVEPOS,BM_SETCHECK,FALSE,0L);
				break;
			case IDC_LINKMENUS:
 				if (SendDlgItemMessage (hWndDlg,LOWORD(wParam),BM_GETCHECK,0,0))
					 SendDlgItemMessage (hWndDlg,IDC_EMBEDMENUS,BM_SETCHECK,FALSE,0L); 
				break;
			case IDC_EMBEDMENUS:
 				if (SendDlgItemMessage (hWndDlg,LOWORD(wParam),BM_GETCHECK,0,0))
					 SendDlgItemMessage (hWndDlg,IDC_LINKMENUS,BM_SETCHECK,FALSE,0L);
				break;
 					
			case IDC_SAVEZOOM:
 				if (SendDlgItemMessage (hWndDlg,LOWORD(wParam),BM_GETCHECK,0,0))
		    	 	EnableWindow (GetDlgItem(hWndDlg,IDC_SAVEIMAGE),TRUE); 
				break;
 					
			case IDC_SAVEIMAGE:
 				if (SendDlgItemMessage (hWndDlg,LOWORD(wParam),BM_GETCHECK,0,0)) 
 				{
		    	 	EnableWindow (GetDlgItem(hWndDlg,IDC_CONFIGDESC),TRUE);  
		    	 	EnableWindow (GetDlgItem(hWndDlg,IDC_SAVEPOS_ALL),TRUE);  
		    	 	EnableWindow (GetDlgItem(hWndDlg,IDC_EMBEDMENUS),FALSE);  
		    	 	EnableWindow (GetDlgItem(hWndDlg,IDC_LINKMENUS),FALSE);  
			    	SendDlgItemMessage (hWndDlg,IDC_EMBEDMENUS,BM_SETCHECK,TRUE,0);
			    	SendDlgItemMessage (hWndDlg,IDC_LINKMENUS,BM_SETCHECK,FALSE,0);
					if (ExistFile ("[%%DL]configs\\savesizes.txt"))
	   	 				EnableWindow (GetDlgItem(hWndDlg,IDC_SAVEPOS_ALL),TRUE);
		    	} 
		    	else
 				{
		    	 	EnableWindow (GetDlgItem(hWndDlg,IDC_CONFIGDESC),FALSE);  
		    	 	EnableWindow (GetDlgItem(hWndDlg,IDC_EMBEDMENUS),TRUE);
		    	 	if (!EmbededMenuLoc)  
		    	 		EnableWindow (GetDlgItem(hWndDlg,IDC_LINKMENUS),TRUE);  
		    	}
				break;
 			
 			case IDC_STARTFULL:
 				if (SendDlgItemMessage (hWndDlg,LOWORD(wParam),BM_GETCHECK,0,0)) 
			    	SendDlgItemMessage (hWndDlg,IDC_SAVEPRINTSETUP,BM_SETCHECK,FALSE,0);     
			    break;
 				 		
 			case IDC_SAVEPRINTSETUP:
 				if (SendDlgItemMessage (hWndDlg,LOWORD(wParam),BM_GETCHECK,0,0)) 
			    	SendDlgItemMessage (hWndDlg,IDC_STARTFULL,BM_SETCHECK,FALSE,0);     
			    break;
 				 		
            case IDOK: 
            	 pStr = GlobalLock (hStartupCommand);
            	 GetDlgItemText (hWndDlg,IDC_STARTCMD,pStr,256);
           	 	 GlobalUnlock (hStartupCommand);
            	 pStr = GlobalLock (hStartupMenu);
            	 GetDlgItemText (hWndDlg,IDC_STARTMENU,pStr,128);
           	 	 GlobalUnlock (hStartupMenu);
            	 SaveZoom = SendDlgItemMessage (hWndDlg,IDC_SAVEZOOM,BM_GETCHECK,0,0);
            	 SaveGlobals = SendDlgItemMessage (hWndDlg,IDC_SAVEGLOBALS,BM_GETCHECK,0,0);
            	 SaveInfoBox = SendDlgItemMessage (hWndDlg,IDC_SAVEINFOBOX,BM_GETCHECK,0,0);
            	 SaveHLTList = SendDlgItemMessage (hWndDlg,IDC_SAVEHLTLIST,BM_GETCHECK,0,0);    
            	 SaveCfgImage = SendDlgItemMessage (hWndDlg,IDC_SAVEIMAGE,BM_GETCHECK,0,0);    
            	 SaveCfgSizePos = SendDlgItemMessage (hWndDlg,IDC_SAVEPOS,BM_GETCHECK,0,0);    
            	 SaveCfgSizePosAll = SendDlgItemMessage (hWndDlg,IDC_SAVEPOS_ALL,BM_GETCHECK,0,0);    
            	 SaveMenuName = SendDlgItemMessage (hWndDlg,IDC_LINKMENUS,BM_GETCHECK,0,0);    
            	 SavePrintSetup = SendDlgItemMessage (hWndDlg,IDC_SAVEPRINTSETUP,BM_GETCHECK,0,0);    
            	 EmbedMenus = SendDlgItemMessage (hWndDlg,IDC_EMBEDMENUS,BM_GETCHECK,0,0); 
				 if (SaveCfgSizePosAll)
				 {
					ShowWindow (GetDlgItem(hWndDlg,IDC_PROGRESS),SW_SHOW);
					ShowWindow (GetDlgItem(hWndDlg,IDC_CANCELIMAGES),SW_SHOW);
		    	 	EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);  
					CreateAllSizes (hWndDlg);
					SetContinueProcessing ( TRUE);
		    	 	EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE);  
					ShowWindow (GetDlgItem(hWndDlg,IDC_PROGRESS),SW_HIDE);
					ShowWindow (GetDlgItem(hWndDlg,IDC_CANCELIMAGES),SW_HIDE);
				 }
				 else if (SaveCfgImage)
            	 {
            	 	SaveMenuName = TRUE;
            	 	EmbedMenus = TRUE;
            	 	GSSiGlobFree (&hConfigDescription);
			    	hConfigDescription = GSSiGlobAlloc (1315,GMEM_MOVEABLE,512);
					pConfigDescription = GlobalLock (hConfigDescription);
	     			GetDlgItemText (hWndDlg,IDC_CONFIGDESC,pConfigDescription,511); 
	     			GlobalUnlock (hConfigDescription);
            	 } 
            	 pViewportsD[0]->ShowFullScreen = SendDlgItemMessage (hWndDlg,IDC_STARTFULL,BM_GETCHECK,0,0);    
            	 SetVarSaveStatus ("%PROMPTS",TRUE);
            	 SetGlobalValueBool ("%PROMPTS",(BOOL)SendDlgItemMessage (hWndDlg,IDC_INCLUDEPROMPT,BM_GETCHECK,0,0));
            	 
	             //GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
	             EndDialog(hWndDlg, TRUE);
            	 break;   
            	 
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window retrning FALSE       */
	            // GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
	             EndDialog(hWndDlg, FALSE);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (441);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (441);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL DEBUGINFOMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (443);
#endif
{ 	
 switch(Message)
   {
    case WM_INITDIALOG:
         /* initialize working variables                                */  
         DebugInfoWnd = hWndDlg;
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
           
              case IDOK:
                  DebugWait = FALSE;
                  break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (443);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (443);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL VISFILESMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (442);
#endif
{ 	
 int	TabStops[2]={100,500};
 //struct	_find_t	FileInfo; 
 char	str[260],desc[100]="", dir[256],File[32],TempFile[MAX_PATH],VisFile[256];
 int	i, rtn, st, Choice,n=0;
 int	BRtn;
 HFILE	FidTemp;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (442);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:
     
	     SendDlgItemMessage (hWndDlg,IDC_LIST,LB_RESETCONTENT,0,0);
       	 SendDlgItemMessage (hWndDlg,IDC_LIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
         if (Pickability) 
         {	SetWindowText (hWndDlg,"Pickability Files");
	    	SetDlgItemText (hWndDlg,IDC_PROMPT,"Select a pickability file from the list below"); 
	     }
	     else
    	 	SetDlgItemText (hWndDlg,IDC_PROMPT,"Select a visibility file from the list below"); 
	     
	     if (!Pickability)
	     {
			 _fstrcpy (dir,"[%DL]vislists\\");
		 	 _fstrcpy (str,"*.vis");
		 } 
		 else
	     {
			 _fstrcpy (dir,"[%DL]piklists\\");
		 	 _fstrcpy (str,"*.pik");
		 }
		 ExpandText (dir);
		 GSSiGetTempFileName (0,"gt",0,TempFile); 
		 FidTemp = GSSiOpenFile (TempFile,0,OF_CREATE);   
		 SearchFilesInDir (dir, "", FidTemp,&n,str,-1,TRUE,TRUE);   
		 GSSiClose2 (&FidTemp);
		 FidTemp= GSSiOpenFile (TempFile,0,OF_READ);  
		 n=0; 
		 while (fgetstring (VisFile,250,FidTemp))
		 {   
			if (*VisFile != '.')
			{
				HFILE	Fid;
				short	Signature, Version;
				LPSTR	lpDot, lpBS;
				
				desc[0]=0;
				Fid = GSSiOpenFile (VisFile,0,OF_READ);
				if (Fid != HFILE_ERROR)
				{
				    GSSillseek(Fid,(LONG)-(4),2);
				
				    BigRead (Fid,(HPSTR)&Signature,2);
				    BigRead (Fid,(HPSTR)&Version,2);
				    if (Signature == 28051 && Version == 1)  
				    {
				    	GSSillseek(Fid,(LONG)-(104),2);
				    	BigRead(Fid,desc,100);
				    } 
				    GSSiClose2 (&Fid);
				}
				if ((lpDot = _fstrrchr(VisFile,'.')))
					*lpDot = 0;
				lpBS = strrchr (VisFile,'\\');
				if (lpBS)
					lpBS++;
				else
					lpBS = VisFile;
				sprintf (str,"%s\t%s",lpBS,desc);
			 	SendDlgItemMessage (hWndDlg,IDC_LIST,LB_ADDSTRING,0,(LPARAM)str); 
			}
		 }
		 GSSiClose2 (&FidTemp);
		 GSSiRemove (TempFile);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDOK: 
				 Choice=SendDlgItemMessage(hWndDlg,IDC_LIST,
									    LB_GETCURSEL,0,0);
				 if (Choice < 0) break; 
				 SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETTEXT,
								 Choice,(DWORD)str);
				 *_fstrchr(str,'\t')=0;   
				 if (Pickability) 
				 {
					_fstrcat(str,".PIK");
				 	LoadPickList (str);
				 }
				 else
				 {
					_fstrcat(str,".VIS");
				 	LoadVisList (str); 
				 }
                 EndDialog(hWndDlg, TRUE);
            	 break;   
            	 
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window retrning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
                 
            case IDC_LIST: /* List box                              */
              {
                switch(HIWORD(wParam))
                    {
                     case LBN_DBLCLK:  
//                     	 IgnoreLbutton = TRUE;
				         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
			    	 	 break;
                    }
		       }
		       break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (442);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (442);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL VIEWPORTSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1002);
#endif
{ 	
 int	TabStops[3]={87,107,120};
 char	str[128], YorN[4],CMD[4];    
 static	BOOL	OrigStatus[MAX_VIEWPORTS];
 int	i, rtn, st,ii, iview;
 static	short Choice=0;
 int	BRtn; 
 
 if (Message==WM_INITDIALOG)
 {  
 	InitDlgPrompts (hWndDlg);
 	SetDlgPrompt (GetDlgItem(hWndDlg,IDC_VIEWPORTS),PRMT_VPLIST,0);
 }
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1002);
#endif
 	return (BRtn);
}
 switch(Message)
   {
 	case WM_DESTROY: 
	 	InitDlgPrompts (0);    
	 	break;
    case WM_INITDIALOG:  
    	 HaltMapDisplay (FALSE,TRUE);
   	 	 HaveBlockingWindow = TRUE;
    	 ClearDlgPrompts ();
         cwCenter(hWndDlg, 0);
		 for (iview = 0;iview<*pNumViewports; iview++)
		 	OrigStatus[iview] = pViewports[iview]->Active;   
Redisplay:
       	 SendDlgItemMessage (hWndDlg,IDC_VIEWPORTS,LB_SETTABSTOPS,3,(LPARAM)&TabStops);
	     SendDlgItemMessage (hWndDlg,IDC_VIEWPORTS,LB_RESETCONTENT,0,0);
		 for (iview = 0;iview<*pNumViewports; iview++)
		 {  
		 	if (pViewports[iview]->Active)        
		 		_fstrcpy (YorN,"Yes");
		 	else
		 		YorN[0]=0;
		 	if (pViewports[iview]->ID == *pCommandViewport)
		 		_fstrcpy (CMD,"Cmd");
		 	else
		 		CMD[0]=0;
        	sprintf (str,"%s\t%s\t%s",pViewports[iview]->Name,CMD,YorN);
		 	SendDlgItemMessage (hWndDlg,IDC_VIEWPORTS,LB_ADDSTRING,0,(LPARAM)str); 
		 }
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDOK:   
                HaveBlockingWindow = FALSE;
                EndDialog(hWndDlg, TRUE);    
            	 break;   
            	 
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */ 
                 SetConfig (1);
				 for (iview = 0;iview<*pNumViewports; iview++)
				 	 pViewports[iview]->Active = OrigStatus[iview];
                 HaveBlockingWindow = FALSE;
                 EndDialog(hWndDlg, FALSE);
                 break;
                 
            case IDC_VIEWPORTS: /* List box                              */
              {
                switch(HIWORD(wParam))
                    {
                     case LBN_DBLCLK:
//				         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
						 SetConfig (1);
			    	 	 SetCurView ( pViewports[Choice]);
					{ 
						
					  FARPROC lpfnVPEDITMsgProc;
					  int	nRc;
						
					  lpfnVPEDITMsgProc = MakeProcInstance((FARPROC)VPEDITMsgProc, hInst);
					  nRc = DialogBox(hInst, (LPSTR)"VPEDIT", hWndMain, (DLGPROC)lpfnVPEDITMsgProc);
					  FreeProcInstance(lpfnVPEDITMsgProc);
					  if (nRc)
					  {  
			 			DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
						setDoPaint( TRUE);      
//						RedisplayWindow();   
				        PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
					  }
					 } 
					 goto SetAct;
                     case LBN_SELCHANGE:
		                 Choice=SendDlgItemMessage(hWndDlg,IDC_VIEWPORTS,
												   LB_GETCURSEL,0,0);  
		SetAct:          
						 SetConfig (1);
			    	 	 if (pViewports[Choice]->Active)  
			    	 	 {
			    	 	 	pViewports[Choice]->Active = FALSE; 
			    	 	 	if (!_fstrnicmp (pViewports[Choice]->Name,"Prim",4))
			    	 	 		ii=1;
			    	 	 }
			    	 	 else
			    	 	 	pViewports[Choice]->Active = TRUE; 
						 TurnOffAutoVis(TRUE);
			    	 	 goto Redisplay;
                    }
		       }
		       break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1002);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1002);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL HLTOUT_FORMATMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (444);
#endif
{ 
	int nItems, i, Version=1, NumFields;  
	char	File[128], Ext[8], ExtID[32];
	LPINT	lpItems;	
 	HFILE	Fid;
	OFSTRUCTGM	OFStruct;
 	BOOL	False=FALSE, tabDlm;  
 	int		IDC_FieldName=IDC_FIELDS;
 	LPSTR	vbar; 
 	char	txt[128], txt2[128];
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (444);
#endif
 	return (BRtn);
}
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam,IDC_SQL,
 					 SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES, SV_TABLE_HEADING,&IDC_FieldName,1,
 					 HLTOutDataFile, &HLTOUTDataFileType, &HLTOuthDB, &False,FALSE))
{
#if ENABLETRACE
GSSiExitProg (444);
#endif
 					 	return TRUE;
}
 switch(Message)
   {
    case WM_INITDIALOG:
         /* initialize working variables                                */
	     GetGlobalCVal ("[%HLTOUTPUTFILE]",HLTOutPath,0);
	case GSSI_REINITDIALOG:
         SetDlgItemText(hWndDlg,IDC_SQL,HLTOutSQL);
         SetDlgItemText(hWndDlg,IDC_OUTPATH,HLTOutPath); 
         if (_fstricmp(HLTOutPath,"SCREEN")) 
         {
			 SendDlgItemMessage (hWndDlg,IDC_OUT_TO_SCREEN,BM_SETCHECK,FALSE,0L);
			 EnableWindow (GetDlgItem(hWndDlg,IDC_FIND_FILE),TRUE);
		 }
         else 
         {
			 SendDlgItemMessage (hWndDlg,IDC_OUT_TO_SCREEN,BM_SETCHECK,TRUE,0L);
			 EnableWindow (GetDlgItem(hWndDlg,IDC_FIND_FILE),FALSE);
		 }         
		 if (HLTOutFields)
		 {
             lpItems = (LPINT)GlobalLock(HLTOutFields);
	         nItems = *lpItems++;
	     
	         for (i=0;i<nItems;i++,lpItems++)  
	         	SendDlgItemMessage(hWndDlg,IDC_FIELDS, LB_SETSEL, TRUE,
									   MAKELPARAM(*lpItems,0)); 
			 GlobalUnlock (HLTOutFields);
         }
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           { 
           	 case IDC_OUT_TO_SCREEN:
            	if (SendDlgItemMessage (hWndDlg,IDC_OUT_TO_SCREEN,BM_GETCHECK,0,0))
            	{
			         SetDlgItemText(hWndDlg,IDC_OUTPATH,"SCREEN"); 
					 EnableWindow (GetDlgItem(hWndDlg,IDC_FIND_FILE),FALSE);
				}
				else
            	{
			         SetDlgItemText(hWndDlg,IDC_OUTPATH,HLTOutPath); 
					 EnableWindow (GetDlgItem(hWndDlg,IDC_FIND_FILE),TRUE);
				}
           	 	break;
           	 
           	 case IDC_FIND_FILE:	
                 if (!GetSaveName2 (hWndDlg,File,IDS_FILTERTEXT,".TXT",IDS_FILETXT))
                 	break;   
                 SetDlgItemText (hWndDlg,IDC_OUTPATH,File);
                 break;
             case IDC_LOAD:  
           		 _fstrcpy (Ext,".HOF"); 
           		 _fstrcpy (ExtID,"Highlight Output Formats");
             	 sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,Ext,_fstrlwr(Ext));
				 if (GetFileName3(hWndDlg,File,0,IDS_FILEHOF))   
				 {
				 	LPSTR	lpID, lpPW;
				 	
	             	if (HLTOutFields)
	             	 	GSSiGlobUlFree (&HLTOutFields);
                 	CloseDataFile (FALSE,&HLTOuthDB);
            	 	Fid = GSSiOpenFile (File,&OFStruct,OF_READ);
            	 	BigRead (Fid,(HPSTR)&Version,2);
            	 	BigRead (Fid,(HPSTR)&HLTOutPath,128);
            	 	BigRead (Fid,(HPSTR)&HLTOutDataFile,128);
            	 	BigRead (Fid,(HPSTR)&HLTOutSQL,lnHLTOutSQL);
            	 	BigRead (Fid,(HPSTR)&nItems,sizeof(int));
	                 HLTOutFields = GSSiGlobAlloc (  21,GHND,nItems*4+4);
	                 lpItems = (LPINT)GlobalLock(HLTOutFields);
			         *lpItems = nItems;
			         lpItems++;       
	                 BigRead (Fid,(HPSTR)lpItems,nItems*sizeof(int));
					 GlobalUnlock(HLTOutFields);
            	 	 GSSiClose2 (&Fid); 
            	 	 lpID = _fstrstr (HLTOutDataFile,";UID="); 
           	 	 	 lpPW = _fstrstr (HLTOutDataFile,";PWD="); 
	            	 if (lpID && lpPW)
	            	 {  
						vbar = _fstrchr (lpID,'|');
		            	*lpID = 0; 
		            	*lpPW = 0;
		            	if (!_fstrncmp (HLTOutDataFile,"ODBC|",5))
							_fstrcpy(CurODBCFile,&HLTOutDataFile[5]); 
						_fstrcpy (txt2,HLTOutDataFile);
	            	 	if (vbar)
	            	 	{
		            		_fstrcat (txt2,vbar);
		            		*vbar = 0; 
		            	}
		            	lpID+=5;
		            	lpPW+=5;
						SetODBCPassword (lpID,lpPW);  
						_fstrcpy (HLTOutDataFile,txt2);
		             }
         			 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
            	 }
				 
            break;
            case IDC_SETSQL:      
            {    
                 HANDLE hMem;
                 LPSTR  lpStr;
                 
                 hMem = GSSiGlobAlloc (  22,GHND,4096);
                 lpStr = GlobalLock (hMem); 
                 GetDlgItemText (hWndDlg,IDC_SQL,lpStr,1024);
                 if (GetSQLWhereClause (hWndDlg, HLTOuthDB, lpStr))
                 	SetDlgItemText (hWndDlg,IDC_SQL,lpStr);    
                 GSSiGlobUlFree (&hMem);
                 break;
            }  
            
             case IDC_SAVE:
           		 _fstrcpy (Ext,".HOF");
            	 if (!GetSaveName2 (hWndDlg,File,0,Ext,IDS_FILEHOF)) 
            	 	break;
            	 if (!_fstrncmp (HLTOutDataFile,"ODBC|",5))
            	 { 
            	 	vbar = _fstrchr (&HLTOutDataFile[5],'|');
            	 	if (!vbar) break;
            	 	*vbar = 0;   
            	 	_fstrcpy (txt,&HLTOutDataFile[5]);
            	 	AddPWtoODBCFile (txt); 
            	 	sprintf (txt2,"ODBC|%s|%s",txt,++vbar); 
            	 	_fstrcpy (HLTOutDataFile,txt2);
            	 }
            	 	
           	 case IDC_CREATE_OUTPUT:
                 nItems=SendDlgItemMessage(hWndDlg,IDC_FIELDS,
										   LB_GETSELCOUNT,
										   0,
										   0);
                 if (!nItems)
                 {
               	 	GSSiMsgBox( GetFocus(), "Error","No Fields Selected", MB_OK,0);
				 	break;
				 }  
				 GSSiGlobFree (&HLTOutFields);
                 HLTOutFields = GSSiGlobAlloc (  23,GHND,nItems*4+4);
                 lpItems = (LPINT)GlobalLock(HLTOutFields);
		         *lpItems = nItems;
		         lpItems++;       
                 SendDlgItemMessage(hWndDlg,IDC_FIELDS,
										   LB_GETSELITEMS,
										   nItems,
										   (LPARAM)lpItems); 
				 GlobalUnlock(HLTOutFields);
                 CloseDataFile (FALSE,&HLTOuthDB);      
                 GetDlgItemText(hWndDlg,IDC_SQL,HLTOutSQL,lnHLTOutSQL);
                 GetDlgItemText(hWndDlg,IDC_OUTPATH,HLTOutPath,128); 
				 SetGlobalValue("%HLTOUTPUTFILE",HLTOutPath); 
				 tabDlm = SendDlgItemMessage(hWndDlg, IDC_TABDLM, BM_GETCHECK, 0, 0);

	             if (wParam == IDC_CREATE_OUTPUT)
	           	 	 CreateHighlightOutput (hWndDlg,GetDlgItem(hWndDlg,IDC_STATUS),HLTOutPath,tabDlm);
	             else
	             {
            	 	Fid = GSSiOpenFile (File,&OFStruct,OF_CREATE);
            	 	BigWrite (Fid,(HPSTR)Version,2,-1);
            	 	BigWrite (Fid,(HPSTR)HLTOutPath,128,-1);
            	 	BigWrite (Fid,(HPSTR)HLTOutDataFile,128,-1);
            	 	BigWrite (Fid,(HPSTR)HLTOutSQL,lnHLTOutSQL,-1);
	                 lpItems = (LPINT)GlobalLock(HLTOutFields);  
	                 BigWrite (Fid,(HPSTR)lpItems,(*lpItems+1)*sizeof(short),-1);
					 GlobalUnlock(HLTOutFields);
            	 	GSSiClose2 (&Fid);
            	 }
                 break;
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window retrning FALSE       */
                 GetDlgItemText(hWndDlg,IDC_OUTPATH,HLTOutPath,128); 
				 SetGlobalValue("%HLTOUTPUTFILE",HLTOutPath); 
                 CloseDataFile (FALSE,&HLTOuthDB);  
                 EndDialog(hWndDlg, FALSE);
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (444);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (444);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL REORGMAPMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (445);
#endif
{
	char	str[256];
	static	HANDLE	hSaveBM=0; 

	
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (445);
#endif
 	return (BRtn);
}
 if (WSAIsBlocking ())
{
#if ENABLETRACE
GSSiExitProg (445);
#endif
 	return(FALSE);
}
 switch(Message)
   {
    case WM_INITDIALOG:    
		 DisableUndo (TRUE);
    	 hSaveBM = EnterBlockingWindow (hWndDlg);
		 SendDlgItemMessage (hWndDlg,IDC_BUILDTAGREF,BM_SETCHECK,TRUE,0L);
		 SendDlgItemMessage (hWndDlg,IDC_REORG_SYMBOLS,BM_SETCHECK,TRUE,0L);
		 SendDlgItemMessage (hWndDlg,IDC_CREATE_BACKUP,BM_SETCHECK,GetGlobalBVal2("[%REORG_CREATEBACKUP]",TRUE),0L);
		 SendDlgItemMessage (hWndDlg,IDC_CONVERT_STREETS,BM_SETCHECK,GetGlobalBVal2("[%REORG_ADDSTREETS]",FALSE),0L);
		 SendDlgItemMessage (hWndDlg,IDC_REBUILD_QUAD,BM_SETCHECK,TRUE,0L);   
		 SendDlgItemMessage (hWndDlg,IDC_REMOVEBADRECS,BM_SETCHECK,FALSE,0L);   
		 SendDlgItemMessage (hWndDlg,IDC_FIXEDTRAN,BM_SETCHECK,TRUE,0L);
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         _fstrcpy (str,"*.CVT");
         DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE);
		 if (PRJ_UNITS[1] == 1)
 		 	SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SETCURSEL,(WPARAM)0,(LPARAM)0); 
		 else if (PRJ_UNITS[1] == 2)
 		 	SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SETCURSEL,(WPARAM)1,(LPARAM)0); 
 		 SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,(WPARAM)-1,(LPARAM)"baseproj"); 
		 GSSiGetTempFileName (0,"gmb",0,str);
		 SetDlgItemText (hWndDlg,IDC_BUDIR,str);
    	 Processing = FALSE;
         cwCenter(hWndDlg, 0); 
         if (hReorgParms)
         {   
         	LPSTR	pParms=GlobalLock (hReorgParms);
         	LPSTR	pTAB=_fstrchr (pParms,'\t');
         	
         	*pTAB++ = 0;
			SendDlgItemMessage (hWndDlg,IDC_CREATE_BACKUP,BM_SETCHECK,atob(pParms),0L); 
			pParms = pTAB;
         	pTAB=_fstrchr (pParms,'\t');
         	*pTAB++ = 0;
			SendDlgItemMessage (hWndDlg,IDC_BUILDTAGREF,BM_SETCHECK,atob(pParms),0L);
			pParms = pTAB;
         	pTAB=_fstrchr (pParms,'\t');
         	*pTAB++ = 0;
			SetDlgItemText (hWndDlg,IDC_TRANFILE,pParms);
			SendDlgItemMessage (hWndDlg,IDC_FIXEDTRAN,BM_SETCHECK,atob(pTAB),0L);
         	GlobalUnlock (hReorgParms);
	        PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
         }
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDC_FIND_MACRO:  
            	GetDlgItemText (hWndDlg,IDC_TCMACRO,str,256);
				if (!GetFileName3 (hWndDlg,str,IDS_FILTERTEXT,IDS_FILETXT)) 
                 	break;   
                SetDlgItemText (hWndDlg,IDC_TCMACRO,str);
            	break;
            case IDC_FIND_TABLE:
            	break;
			case IDC_CREATE_BACKUP:
				if (!SendDlgItemMessage (hWndDlg,IDC_CREATE_BACKUP,BM_GETCHECK,0,0))
				{
		 			SendDlgItemMessage (hWndDlg,IDC_REMOVEBADRECS,BM_SETCHECK,FALSE,0L);
		 			EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVEBADRECS),FALSE);
		 		}
		 		else   
		 			EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVEBADRECS),TRUE);
				break;
           
            case IDC_GETTRANFILE:
		        if (GetFileName3 (hWndDlg,str,IDS_FILTERCPT,IDS_FILECPT))   
                { 
                 	SetDlgItemText (hWndDlg,IDC_TRANFILE,str);
					ShowWindow (GetDlgItem(hWndDlg,IDC_FIXEDTRAN),SW_SHOW);
                }
            	break;
            	
           	case IDOK:
			     {                                     	
					 LPVISLIST	SaveVis, SaveCurVis; 
					 int		SaveNumVis;
					 OFSTRUCTGM	OFStruct;
					 short		i, SaveFPT=CurView->FileProjectionType;
					 HFILE		FidRestart;
					 HANDLE	hSaveVP=GSSiGlobAlloc (  24,GMEM_MOVEABLE,sizeof(VIEWPORT));
					 LPVIEWPORT	pSaveVP=(LPVIEWPORT)GlobalLock (hSaveVP);       
                     char	SaveAltProj[MAX_PATH]; 
                     BOOL	OpenedSeg=FALSE;

					 TimeRangeBeg=0;
					 TimeRangeEnd=LONG_MAX;
					 SetViewport(*pCommandViewport);
			         CloseTRANS2 (&CurView->hTranVPToBase);
			         CloseTRANS2 (&CurView->hTranBaseToVP);
			         CloseTRANS2 (&CurView->hTranVPToScreen);
			         CloseTRANS2 (&CurView->hTranScreenToVP);
			         CloseTRANS2 (&hTranReorg);
				     GSSiGlobFree (&CurView->hTAGList);
   				     GSSiDeleteObject(&CurView->hRgn);
					 *pSaveVP = *CurView;
		             CurView->UpdateFile = 0;
				 	 EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
/*			 		 FidRestart = GSSiOpenFile ("restart.txt",&OFStruct,OF_READ);
			 		 if (FidRestart != HFILE_ERROR)
			 		 {  
			 		 	fgetstring (RestartName,sizeof(RestartName)-4,FidRestart);
			 		 	GSSiClose2 (&FidRestart); 
			 		 }
			 		 else
			 		 	RestartName[0]=0; */
//			 		 RestartOption=TRUE; 
					 
                    if (!GetDlgItemText (hWndDlg,IDC_PROJECTION,curproject,lncurproject))
                    {
                        GSSiMsgBox(GetFocus(),"No output projection set", 0,MB_ICONEXCLAMATION|MB_OK,0);
                        goto Exit;
                    }  
                    ReorgUpdateBounds = SendDlgItemMessage (hWndDlg,IDC_UPDATEBOUNDS,BM_GETCHECK,0,0);
                    ReorgUpdateTime = SendDlgItemMessage (hWndDlg,IDC_UPDATETIME,BM_GETCHECK,0,0);
				 	CurView->FileProjectionType=0;
                    GetGlobalCVal ("[%ALT_PROJECTION]",SaveAltProj,0);
                    SetGlobalValue("%ALT_PROJECTION",curproject);
				    ConvertCoordClose ();
				    ConvertCoordInit();
                    GetDlgItemText (hWndDlg,IDC_UNITS,curunits,lncurunits);
                    if (*curunits)
                    { 
                        if (!_fstrcmp(curunits,"Feet"))
                            PRJ_UNITS[3] = 1;
                        else if (!_fstrcmp(curunits,"Meters"))
                            PRJ_UNITS[3] = 2;
                    }
                   	 if (GetDlgItemText (hWndDlg,IDC_TCMACRO,str,256))
                   	 {  
                   	 	LPSTR pMacro;
                   	 	
                   	 	ConvertTAGValues = 1;
                   	 	hConvertTAGMacro = GSSiGlobAlloc (0,GMEM_MOVEABLE,256);
                   	 	pMacro = GlobalLock (hConvertTAGMacro); 
                   	 	sprintf (pMacro,"$MACRO(%s)",str);
                   	 	GlobalUnlock (hConvertTAGMacro);
                   	 }
					 if (GetDlgItemText (hWndDlg,IDC_TRANFILE,str,sizeof(str)))
					 {  
					 	if (SendDlgItemMessage (hWndDlg,IDC_FIXEDTRAN,BM_GETCHECK,0,0))
					 		_fstrcat (str,"(F,3)");
						hTranReorg = LoadTranFileWithDandT (str);
						if (!hTranReorg)
						{
							GSSiMsgBox(GetFocus(),"Invalid transformation file", str,MB_ICONEXCLAMATION|MB_OK,0);
							goto Exit;
						}  
					 }
					 else if (PRJ_UNITS[1] != PRJ_UNITS[3] || _fstricmp (curproject,"baseproj.cvt"))
					 	hTranReorg = (HANDLE)1;
			    	 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE);  
					 if (SendDlgItemMessage (hWndDlg,IDC_CREATE_BACKUP,BM_GETCHECK,0,0))
					 {  
					 	char	TestFile[256];
						OFSTRUCTGM	OFStruct;
					 	LPSTR	pFile;  
					 	HFILE	hTestFile;
					 	DWORD	Err;
					 	
					 	
					 	hReorgBUDir = GSSiGlobAlloc (  25,GMEM_MOVEABLE,256);
					 	pFile = GlobalLock (hReorgBUDir);
					 	if (!GetDlgItemText (hWndDlg,IDC_BUDIR,pFile,256))
					 	{
					 		GSSiMsgBox (hWndDlg,"No backup directory",0,MB_ICONEXCLAMATION,0); 
					 		GSSiGlobUlFree (&hReorgBUDir);
					 		goto Exit;
					 	}
					 	GSSiMakeDir (pFile,&Err);  
					 	sprintf (TestFile,"%s\\test.tmp",pFile);
					 	hTestFile = GSSiOpenFile (TestFile,&OFStruct,OF_CREATE);
					 	if (hTestFile == HFILE_ERROR)
					 	{
					 		GSSiMsgBox (hWndDlg,"Unable to create backup directory",0,MB_ICONEXCLAMATION,0); 
					 		GSSiGlobUlFree (&hReorgBUDir);
					 		goto Exit;
					 	} 
					 	GSSiClose2 (&hTestFile);
					 	GSSiRemove (TestFile);
					 	GlobalUnlock (hReorgBUDir);
					 }
					 else
		 				 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);
			    	 EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
			    	 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE);
					 if (SendDlgItemMessage (hWndDlg,IDC_REMOVEBADRECS,BM_GETCHECK,0,0))
					 {  
				    	 _fstrcpy (str,"[%INVALIDRECORDOPT]=2");
				    	 ExpandText (str);
				     }
			    	 ReorgBadRecs = 0;
					 DisableHalt = TRUE;  
					 setDoPaint( FALSE);
					 Processing = TRUE;
					 ReorghWnd = hWndDlg;
					 ReorgStatus = IDC_STATUS;
					 ReorgFileTxt = IDC_FILE;
					 SetViewport(*pCommandViewport); 
					 SelectVisList (FALSE);
					 SaveCurVis = CurVis;
					 SaveVis = CurView->pVisList1; 
					 SaveNumVis = CurView->NumVisList; 
					 CurView->NumVisList = 1; 
					 CurVis = CurView->pVisList1;  
					 CurView->pVisListManual=0;
					 InitVis ();  
					 for (i=0;i<MAX_VIEWPORT_FILES;i++)
					 	CurVis->FileIsVisible[i]=SaveCurVis->FileIsVisible[i];
                     ReorgFile=TRUE;  
			    	 IgnoreBounds = TRUE; 
			    	 ReReference = SendDlgItemMessage (hWndDlg,IDC_REREFERENCE,BM_GETCHECK,0,0);
	            	 if (SendDlgItemMessage (hWndDlg,IDC_CONVERT_STREETS,BM_GETCHECK,0,0)) 
	            	 {
						 OpenStreetSegmentTable (FALSE,&OpenedSeg);
						 AddStreetNums=TRUE;
					 }
	            	 if (SendDlgItemMessage (hWndDlg,IDC_REBUILD_QUAD,BM_GETCHECK,0,0))
	            	 {   
	            	 	 LPSTR	pFile;
						 BOOL	saveRedisplayOnly = RedisplayOnly;
				    	 SetDlgItemText (hWndDlg,IDC_MESS,"Rebuild Quad Trees"); 
				         ClearAllBounds();
						 SetViewport(*pCommandViewport);
						 hNewQuadFile = GSSiGlobAlloc (  26,GMEM_MOVEABLE,256);
					 	 pFile = GlobalLock (hNewQuadFile);
						 GSSiGetTempFileName (0,"gm",0,pFile); 
						 NewQuadFID = GSSiOpenFile (pFile,&OFStruct,OF_CREATE);
						 GlobalUnlock (hNewQuadFile);
						 Display=FALSE;		                  
					     CurView->CurZoomAreaRef = 0;
						 RedisplayOnly = FALSE;
						 RedisplayViewport(TRUE,TRUE);
						 RedisplayOnly = saveRedisplayOnly;
					 	 Display=TRUE;		                  
						 GSSiClose2 (&NewQuadFID); 
					 	 pFile = GlobalLock (hNewQuadFile);
						 GSSiRemove (pFile); 
						 GSSiGlobUlFree (&hNewQuadFile);  
					 }
					 NewQuadFID = 0;
				     ReReference = FALSE;
			    	 
	            	 if (!SendDlgItemMessage (hWndDlg,IDC_REORG_SYMBOLS,BM_GETCHECK,0,0))    
						goto Exit;
			    	 if (ContinueProcessing)
			    	 {
				    	 SetDlgItemText (hWndDlg,IDC_MESS,"Reorganize maps"); 
				         ClearAllBounds();
						 SetViewport(*pCommandViewport);
						 Display=FALSE;		                  
					     CurView->CurZoomAreaRef = 0;
						 RedisplayViewport(TRUE,TRUE);
					 	 Display=TRUE;		                  
				    	 SetDlgItemText (hWndDlg,IDC_MESS,"Reorganization complete"); 
				     }
			    	 EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
			    	 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE);
			    	 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);

        Exit:    
					 GSSiGlobFree (&hReorgBUDir);    
					 GSSiGlobFree (&hConvertTAGMacro);
				     SetViewport(*pCommandViewport);
			         CloseTRANS2 (&CurView->hTranVPToBase);
			         CloseTRANS2 (&CurView->hTranBaseToVP); 
			         CloseTRANS2 (&CurView->hTranVPToScreen);
			         CloseTRANS2 (&CurView->hTranScreenToVP); 
			         if (hTranReorg == (HANDLE)1)
			         	hTranReorg = 0;
				     GSSiGlobFree (&CurView->hTAGList);
			         CloseTRANS2 (&hTranReorg);
					 CloseTRANS2 (&hTranBaseToFileReorg);
					 GSSiDeleteObject(&CurView->hRgn);
                     *CurView = *pSaveVP;
                     GSSiGlobUlFree (&hSaveVP);  
			    	 ReorgFile=FALSE;
					 CurView->pVisList1 = SaveVis;
					 CurView->NumVisList =SaveNumVis; 
	                 CurView->FileProjectionType = SaveFPT;  
	                 SetGlobalValue("%ALT_PROJECTION",SaveAltProj);
	           		 ConvertCoordClose ();
					 Processing = FALSE;
					 DisableHalt = FALSE; 
			 		 RestartOption=FALSE; 
			 		 CloseStreetSegmentTable(OpenedSeg); 
			 		 AddStreetNums=FALSE;
			 		 ConvertTAGValues = 0; 
			 		 SetContinueProcessing ( TRUE);
			     } 
        		 IgnoreBounds = FALSE;
		    	 _fstrcpy (str,"[%INVALIDRECORDOPT]=2");
		    	 ExpandText (str);   
		    	 if (ReorgBadRecs)
		    	 {
		    	 	sprintf (str,"%ld invalid records removed",ReorgBadRecs);
		    	 	GSSiMessageBox (0,str,"Warning",MB_ICONEXCLAMATION,0);
		    	 } 
		    	 ReorghWnd = 0; 
		    	 DisableUndo (FALSE);
	             if (SendDlgItemMessage (hWndDlg,IDC_BUILDTAGREF,BM_GETCHECK,0,0))    
		             GSSiEndDialog(hWndDlg, 2,hSaveBM);
	             else
		             GSSiEndDialog(hWndDlg, 1,hSaveBM);
                 break;
                  
            case IDC_EXIT:
            	 ReorghWnd = 0;
		    	 DisableUndo (FALSE);
                 GSSiEndDialog(hWndDlg, 1,hSaveBM);
                 break;
                  
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window retrning FALSE       */   
				 SetContinueProcessing(FALSE);
				 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (445);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (445);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

int GetCurrentUpdateNumber(void)
{
	int rtn = 0;
	HFILE fid = GSSiOpenFile("[%%DL]updates\\lastupdate.txt", 0, OF_READ);

	if (fid)
	{
		char line[1024];
		fgetstring(line, 1020, fid);
		fgetstring(line, 1020, fid);
		rtn = atoi(line);
		GSSiClose2(&fid);
	}
	return rtn;
}
BOOL FAR PASCAL ABOUTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (439);
#endif
{ 
	char	str[256], cUID[32], FIVersion[128], FICopyright[1024], MrSidVer[256];	
	HBITMAP	hBmp=0; 
	HWND	hWnd;
	HDC		hDC; 
	RECT	Rect; 
	long	ii;

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (439);
#endif
 	return (BRtn);
}
 switch(Message)
   {
 case WM_INITDIALOG:
 {
	 int GMUpdate = GetCurrentUpdateNumber();
	 LPSTR fgdbVersion = FGDBVersion();
	 GetFreeImageVersionAndCopyright(FIVersion, FICopyright);
	 ii = MrSidVersion(MrSidVer);
	 SetDlgItemText(hWndDlg, IDC_COPYR2, FICopyright);
	 LoadString(hInst, IDS_LIZTECH, FICopyright, sizeof(FICopyright));
	 SetDlgItemText(hWndDlg, IDC_COPYR3, FICopyright);
	 //		 GetGlobalCVal ("[%APPID]",AppName,"GeoMaster");
	 // SetDlgItemText(hWndDlg, IDC_APP, AppName);
	 SetDlgItemText(hWndDlg, IDC_APP, fgdbVersion);
	 SetDlgItemText(hWndDlg, IDC_APP, MODULENAME);
	 //ShowWindow (GetDlgItem(hWndDlg,IDC_LOGO),SW_SHOW);
	 PostMessage(hWndDlg, WM_COMMAND, IDC_LOGO, 0L);
	 if (*SerialNumber)
	 {
		 sprintf(str, "Serial Number: %s", SerialNumber);
		 SetDlgItemText(hWndDlg, IDC_SERIALNUM, str);
	 }
#if S_VERSION
	 sprintf(FICopyright, "%s_s - Update %i\r\nFreeImage Version %s\r\nMrSID Version %s", GMVersion, GMUpdate, FIVersion, MrSidVer);
#else
	 sprintf(FICopyright, "%s - Update %i\r\nFreeImage Version %s\r\nMrSID Version %s", GMVersion,GMUpdate, FIVersion, MrSidVer);
#endif
	 SetDlgItemText(hWndDlg, IDC_VERSION, FICopyright);
 }
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
		 case IDC_LOGO:
		 {
			 HDIB32 hDIB = 0;
			 char BMPath[MAX_PATH];
			 char UCAPPNAME[32];
			 strcpy(UCAPPNAME, AppName);
			 strupr(UCAPPNAME);
			 //strcpy(UCAPPNAME, "PANZOOMROT");
			 sprintf(BMPath, "[%%DL]icons\\%s.png", AppName);
			 ExpandText(BMPath);
			 if (ExistFile(BMPath))
			 {
				 hDIB = LoadDIB32(BMPath,FALSE);
			 }
			 else if ((hBmp = LoadBitmap(hInst, UCAPPNAME)))
			 {
				 hDIB = BitmapToDIB32(hBmp);
			 }
			 {
				 POINT	mp;
				 BITMAPINFOHEADER	lpbi;
				 RECT	BMRect;

				 GetBitmapInfoFromHandle(&lpbi, hDIB);
				 if (hBmp)
					 DeleteObject(hBmp);
				 BMRect.left = BMRect.top = 0;
				 BMRect.right = lpbi.biWidth;
				 BMRect.bottom = lpbi.biHeight;
				 hWnd = GetDlgItem(hWndDlg, IDC_LOGO);
				 GetWindowRect(hWnd, &Rect);
				 ScreenRectToClientRect(hWndDlg, &Rect);
				 //AdjustRectToRect(&BMRect, &Rect);

				 hDC = GetDC(hWndDlg);
				 SetDisplayMode(hDC, GF_SCREENMODE);
				 /*SetWindowOrgEx  ( hDC, 0, 0,0 );
				 SetViewportOrgEx( hDC, 0, 0,0 );
				 SetMapMode    ( hDC, MM_TEXT );*/
				 SelectClipRgn(hDC, 0);
				 //	FillRect (hDC,&Rect,GetStockObject(LTGRAY_BRUSH ));
				 //	DisplayBMInRect2 (hDC,hDIB, Rect,0,0,0,0);
				 DisplayTransparentBitmapInRect(hDC, hDIB, &Rect, TRUE);
				 ReleaseDC(hWnd, hDC);
				// DestroyDIB(hDIB);
			 }
			 }
         		break;
         		
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break;
         }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (439);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (439);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} /* End of ABOUTMsgProc                                      */
BOOL FAR PASCAL RANCOLORMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
	HDC		hDC;  
	HWND	hScroll;
	RECT	Rect,ClientRect;
	short	irow, icol, NumCols, NumRows, Choice;  
	short	ColGap=2, RowGap=2, ipos;
	LPHBRUSH	pRandBrushes; 
	short	i,j,nums[30]={5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131};
	char	str[128];
	BOOL	Err;

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
         SendDlgItemMessage (hWndDlg,IDC_NUMCOLORS,CB_ADDSTRING,0,(LPARAM)"Use Default Values");
   	 	 SendDlgItemMessage (hWndDlg,IDC_NUMCOLORS,CB_SETCURSEL,(WPARAM)(0),(LPARAM)0); 
		 SendDlgItemMessage (hWndDlg,IDC_NUMCOLORS,CB_SETCURSEL,(WPARAM)0,(LPARAM)0); 
		 for (i=0;i<30;i++)  
		 {
			itoa (nums[i],str,10);
		    j=SendDlgItemMessage (hWndDlg,IDC_NUMCOLORS,CB_ADDSTRING,0,(LPARAM)str);
			if (CurView->NumRanColor == nums[i])
				SendDlgItemMessage (hWndDlg,IDC_NUMCOLORS,CB_SETCURSEL,(WPARAM)j,(LPARAM)0); 
		 }
		 SetDlgItemInt (hWndDlg,IDC_MINRED,CurView->PrimeColorMin[0],FALSE);
		 SetDlgItemInt (hWndDlg,IDC_MAXRED,CurView->PrimeColorMax[0],FALSE);
		 SetDlgItemInt (hWndDlg,IDC_MINGREEN,CurView->PrimeColorMin[1],FALSE);
		 SetDlgItemInt (hWndDlg,IDC_MAXGREEN,CurView->PrimeColorMax[1],FALSE);
		 SetDlgItemInt (hWndDlg,IDC_MINBLUE,CurView->PrimeColorMin[2],FALSE);
		 SetDlgItemInt (hWndDlg,IDC_MAXBLUE,CurView->PrimeColorMax[2],FALSE);
		 
		 SetScrollRange(GetDlgItem(hWndDlg,IDC_MINREDSCROLL), SB_CTL, 0, 100, FALSE);
		 SetScrollPos (GetDlgItem(hWndDlg,IDC_MINREDSCROLL),SB_CTL,(int)CurView->PrimeColorMin[0],TRUE);
		 SetScrollRange(GetDlgItem(hWndDlg,IDC_MAXREDSCROLL), SB_CTL, 0, 100, FALSE);
		 SetScrollPos (GetDlgItem(hWndDlg,IDC_MAXREDSCROLL),SB_CTL,(int)CurView->PrimeColorMax[0],TRUE);
		 SetScrollRange(GetDlgItem(hWndDlg,IDC_MINGREENSCROLL), SB_CTL, 0, 100, FALSE);
		 SetScrollPos (GetDlgItem(hWndDlg,IDC_MINGREENSCROLL),SB_CTL,(int)CurView->PrimeColorMin[1],TRUE);
		 SetScrollRange(GetDlgItem(hWndDlg,IDC_MAXGREENSCROLL), SB_CTL, 0, 100, FALSE);
		 SetScrollPos (GetDlgItem(hWndDlg,IDC_MAXGREENSCROLL),SB_CTL,(int)CurView->PrimeColorMax[1],TRUE);
		 SetScrollRange(GetDlgItem(hWndDlg,IDC_MINBLUESCROLL), SB_CTL, 0, 100, FALSE);
		 SetScrollPos (GetDlgItem(hWndDlg,IDC_MINBLUESCROLL),SB_CTL,(int)CurView->PrimeColorMin[2],TRUE);
		 SetScrollRange(GetDlgItem(hWndDlg,IDC_MAXBLUESCROLL), SB_CTL, 0, 100, FALSE);
		 SetScrollPos (GetDlgItem(hWndDlg,IDC_MAXBLUESCROLL),SB_CTL,(int)CurView->PrimeColorMax[2],TRUE);
    	 PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

	case WM_HSCROLL:
		hScroll = (HWND)lParam;
		ipos = GetScrollPos (hScroll,SB_CTL);
		switch (wParam)
		{
		  case SB_LINEDOWN:
		  ipos++;
		  break;
			
		  case SB_LINEUP:
		  ipos--;
	  	  break;
			
	  	  case SB_THUMBPOSITION:
	  	  ipos = LOWORD(lParam);
		  break;
		  
		  default:
		  	return FALSE;
		}
		SetScrollPos (hScroll,SB_CTL,(int)ipos,TRUE);
		ipos = GetScrollPos (hScroll,SB_CTL);
		if (hScroll == GetDlgItem (hWndDlg,IDC_MINREDSCROLL))
		{
			ipos = min (ipos,CurView->PrimeColorMax[0]);
			SetDlgItemInt (hWndDlg,IDC_MINRED,ipos,FALSE);
		}
		else if (hScroll == GetDlgItem (hWndDlg,IDC_MAXREDSCROLL))
		{
			ipos = max (ipos,CurView->PrimeColorMin[0]);
			SetDlgItemInt (hWndDlg,IDC_MAXRED,ipos,FALSE);
		}
		else if (hScroll == GetDlgItem (hWndDlg,IDC_MINGREENSCROLL))
		{
			ipos = min (ipos,CurView->PrimeColorMax[1]);
			SetDlgItemInt (hWndDlg,IDC_MINGREEN,ipos,FALSE);
		}
		else if (hScroll == GetDlgItem (hWndDlg,IDC_MAXGREENSCROLL))
		{
			ipos = max (ipos,CurView->PrimeColorMin[1]);
			SetDlgItemInt (hWndDlg,IDC_MAXGREEN,ipos,FALSE);
		}
		if (hScroll == GetDlgItem (hWndDlg,IDC_MINBLUESCROLL))
		{
			ipos = min (ipos,CurView->PrimeColorMax[2]);
			SetDlgItemInt (hWndDlg,IDC_MINBLUE,ipos,FALSE);
		}
		else if (hScroll == GetDlgItem (hWndDlg,IDC_MAXBLUESCROLL))
		{
			ipos = max (ipos,CurView->PrimeColorMin[2]);
			SetDlgItemInt (hWndDlg,IDC_MAXBLUE,ipos,FALSE);
		}
			
		SetScrollPos (hScroll,SB_CTL,(int)ipos,TRUE);
    	PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
		
	    break;
	         
    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
         	
         	
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);
            break;
            
            case IDC_NUMCOLORS:
            case IDOK: 
            {
            	hDC = GetDC (GetDlgItem(hWndDlg,IDC_RANCOLORS));
            	GetClientRect (GetDlgItem(hWndDlg,IDC_RANCOLORS),&ClientRect);
		        
		        if ((Choice=SendDlgItemMessage(hWndDlg,IDC_NUMCOLORS,CB_GETCURSEL,0,0)))
		        	CurView->NumRanColor = nums[Choice-1];
		        else
		        	CurView->NumRanColor = 0;
            	
            	CurView->PrimeColorMin[0] = GetDlgItemInt (hWndDlg,IDC_MINRED,&Err,FALSE);
            	CurView->PrimeColorMax[0] = GetDlgItemInt (hWndDlg,IDC_MAXRED,&Err,FALSE);
            	CurView->PrimeColorMin[1] = GetDlgItemInt (hWndDlg,IDC_MINGREEN,&Err,FALSE);
            	CurView->PrimeColorMax[1] = GetDlgItemInt (hWndDlg,IDC_MAXGREEN,&Err,FALSE);
            	CurView->PrimeColorMin[2] = GetDlgItemInt (hWndDlg,IDC_MINBLUE,&Err,FALSE);
            	CurView->PrimeColorMax[2] = GetDlgItemInt (hWndDlg,IDC_MAXBLUE,&Err,FALSE);

				CreateRandomBrushes (CurView,0);
				if (NumRandomColors > 16)
					NumCols = 8;
				else if (NumRandomColors > 4)
					NumCols = 4;             
				else
					NumCols = 2;
				NumRows = NumRandomColors / NumCols;
				pRandBrushes = (LPHBRUSH)GlobalLock (hRandBrushes);
				pRandBrushes++; 
				FillRect (hDC,&ClientRect,GetStockObject (WHITE_BRUSH));
				for (irow = 0;irow < NumRows;irow++)
					for (icol = 0; icol < NumCols; icol++) 
					{
						Rect.left = ClientRect.left + icol * ((ClientRect.right - ClientRect.left)/NumCols) + ColGap;
						Rect.right = ClientRect.left + (icol+1) * ((ClientRect.right - ClientRect.left)/NumCols) - ColGap;
						Rect.top = ClientRect.top + irow * ((ClientRect.bottom - ClientRect.top)/NumRows) + RowGap;
						Rect.bottom = ClientRect.top + (irow+1) * ((ClientRect.bottom - ClientRect.top)/NumRows) - RowGap;
						FillRect (hDC,&Rect,*pRandBrushes); 
						pRandBrushes+=2;
					}
				GlobalUnlock (hRandBrushes);
            	ReleaseDC (GetDlgItem(hWndDlg,IDC_RANCOLORS),hDC);
            }
            break;
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}  

BOOL FAR PASCAL RDFEDITMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (838);
#endif
{ 
 static	LPVIEWPORT	EditVP=0;       
 char	str[256];
 
 int	BRtn,i, Choice;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (838);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:  
         if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (838);
#endif
         	return FALSE;
}
         if (!CurView->NumNewObjects)
{
#if ENABLETRACE
GSSiExitProg (838);
#endif
         	return FALSE;
}
         EditVP = CurView; 
    case GSSI_REINITDIALOG:
       	 EnableWindow (GetDlgItem(hWndDlg,IDC_RDFEDIT),FALSE);
     	 EnableWindow (GetDlgItem(hWndDlg,IDC_RDFSYMREMOVE),FALSE);
		 SendDlgItemMessage (hWndDlg,IDC_RDFSYMBOLS,LB_RESETCONTENT,0,0);
		 SendDlgItemMessage (hWndDlg,IDC_RDFLIST,LB_RESETCONTENT,0,0);
         for (i=0;i<EditVP->NumNewObjects;i++)
         {  
         	switch (EditVP->NewObject[i].Type)
         	{
         		case 1:
    				sprintf (str,"Line: Style %i Width %f Color (%i,%i,%i)",EditVP->NewObject[i].Style, 
    																EditVP->NewObject[i].Width,
    																EditVP->NewObject[i].R,
    																EditVP->NewObject[i].G,
    																EditVP->NewObject[i].B);
    			break;
         		case 3:
    				sprintf (str,"Area: Style %i Color (%i,%i,%i)",EditVP->NewObject[i].Style,
    																EditVP->NewObject[i].R,
    																EditVP->NewObject[i].G,
    																EditVP->NewObject[i].B);
    			break;
    			default:
    			case 0:
    				sprintf (str,"Color (%i,%i,%i)", 
    																EditVP->NewObject[i].R,
    																EditVP->NewObject[i].G,
    																EditVP->NewObject[i].B);   
    			break;
    		}
			SendDlgItemMessage(hWndDlg,IDC_RDFLIST,LB_ADDSTRING,0,(LPARAM)str); 
    	}
         
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
         	case IDOK:	
            case IDCANCEL: 
                EndDialog(hWndDlg, TRUE); 
                EditVP = 0;

            break;
            
            case IDC_RDFEDIT:
	            Choice=SendDlgItemMessage(hWndDlg,IDC_RDFLIST,LB_GETCURSEL,0,0); 
	         	switch (EditVP->NewObject[Choice].Type)
	         	{
	         		case 1:
	         		NewWidth = EditVP->NewObject[Choice].Width;  
	         		INewWidth = IDNINT (NewWidth);
	         		NewColor = RGB(EditVP->NewObject[Choice].R,EditVP->NewObject[Choice].G,EditVP->NewObject[Choice].B);
			        NewProPen = EditVP->NewObject[Choice].ProPen;
			        NewStyle = EditVP->NewObject[Choice].Style;
			        {
				    	FARPROC lpfnLINETYPEMsgProc; 
				    	int		nRc;
					    	
						setDoPaint( FALSE); 
				        lpfnLINETYPEMsgProc = MakeProcInstance((FARPROC)LINETYPEMsgProc, hInst);
						nRc = DialogBox(hInst, (LPSTR)"LINETYPE", hWndDlg, (DLGPROC)lpfnLINETYPEMsgProc);
				        FreeProcInstance(lpfnLINETYPEMsgProc);
						setDoPaint( TRUE);
				        if (nRc)
					        SetClassLineType (Choice,1,NewWidth,NewStyle,NewProPen,NewColor);
					} 
					break;
	         		case 3:
			        NewProPen = EditVP->NewObject[Choice].ProPen;
			        NewStyle = EditVP->NewObject[Choice].Style;
					case 2: 
	         		NewColor = RGBW(EditVP->NewObject[Choice].R,EditVP->NewObject[Choice].G,EditVP->NewObject[Choice].B,IDNINT(EditVP->NewObject[Choice].Width));
				    if(GetColor(hWndDlg,&NewColor))
						SetClassColor (Choice,EditVP->NewObject[Choice].Type,NewColor);
				    break;
		        }
	                  
		    	PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
	            break;
	        
	        case IDC_RDFSYMADD:
                Choice=SendDlgItemMessage(hWndDlg,IDC_RDFLIST,LB_GETCURSEL,0,0);  
                *str = 0;
                GetTextString (hWndDlg,str,128,"Enter the symbol name",0,0,0,TRUE,TRUE); 
                if ((i = GetDictSymbolNumber (str)))
                	EditVP->NewObjectMap[i] = Choice+1;
		    //	PostMessage(hWndDlg, WM_COMMAND,IDC_RDFLIST,MAKELPARAM(Choice,LBN_SELCHANGE));
				goto ListSyms;
	        	break;
	        	    
			case IDC_RDFSYMREMOVE:
                Choice=SendDlgItemMessage(hWndDlg,IDC_RDFSYMBOLS,LB_GETCURSEL,0,0);
			 	SendDlgItemMessage(hWndDlg,IDC_RDFSYMBOLS,LB_GETTEXT,Choice,(LPARAM)str); 
			 	i = GetSymbolNum (str);
			 	EditVP->NewObjectMap[i] = 0;
		    //	PostMessage(hWndDlg, WM_COMMAND,IDC_RDFLIST,MAKELPARAM(Choice,LBN_SELCHANGE));
				goto ListSyms;
				break;
            
            case IDC_RDFLIST:  
              {
                switch(HIWORD(wParam))
                    {
                     case LBN_DBLCLK:
 					   	 PostMessage(hWndDlg, WM_COMMAND, IDC_RDFEDIT, 0L);
	                     break;
                     case LBN_SELCHANGE:
                     	 EnableWindow (GetDlgItem(hWndDlg,IDC_RDFEDIT),TRUE);
                     	 EnableWindow (GetDlgItem(hWndDlg,IDC_RDFSYMREMOVE),FALSE);
ListSyms:
	                     Choice=SendDlgItemMessage(hWndDlg,IDC_RDFLIST,LB_GETCURSEL,0,0);
						 SendDlgItemMessage (hWndDlg,IDC_RDFSYMBOLS,LB_RESETCONTENT,0,0);
	                     for (i=1;i<3201;i++)
	                     {
	                     	if (EditVP->NewObjectMap[i] == Choice+1)
	                     	{   
	                     		char	SymName[34];
	                     		
               					GetSymbolName (i,SymName,0,0,0);
								SendDlgItemMessage(hWndDlg,IDC_RDFSYMBOLS,LB_ADDSTRING,0,(LPARAM)SymName); 

	                     	}
	                     } 
                         break;
                    }
		       }
                 break;

            case IDC_RDFSYMBOLS:  
              {
                switch(HIWORD(wParam))
                    {
                     case LBN_DBLCLK:
 					   	 PostMessage(hWndDlg, WM_COMMAND, IDC_RDFSYMREMOVE, 0L);
	                     break;
                     case LBN_SELCHANGE:
                     	 EnableWindow (GetDlgItem(hWndDlg,IDC_RDFSYMREMOVE),TRUE);
                         break;
                    }
		       }
                 break;



         }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (838);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (838);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL SELECTITEMSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1107);
#endif
{   
	static	HANDLE	hSaveBM;
	char	str[256]; 
	int		i,n, MinWidth=120, MaxHeight=700;   
	HFILE	Fid;
	OFSTRUCTGM	OFStruct;
	LPSTR	Args; 
	RECT	Rect, Rect2, CRect,OKRect, CLRect;
	int		incW, incH, w,h,rtn,CancelWidth, CancelHeight;   
	static	BOOL	UseFile;

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG: 
        
		hSaveBM = EnterBlockingWindow (hWndDlg);
        Args = GlobalLock (hSelectItemsArgs);
        if (*Args == 'N') 
        {
        	UseFile = FALSE;
 			//ShowWindow (GetDlgItem(hWndDlg,IDOK),SW_HIDE);
        }
        else
        	UseFile = TRUE;
        	  
        SetWindowText (hWndDlg,&Args[850]);
    	n = FillList (hWndDlg,IDC_LIST,&Args[2],0,&Rect);
		GlobalUnlock (hSelectItemsArgs);
		if (n == -2) 
    	{ 
            GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
    		break;
    	}
 		w = max (MinWidth,Rect.right - Rect.left);
		h = min (MaxHeight,Rect.bottom -Rect.top); 
        GetWindowRect (GetDlgItem (hWndDlg,IDOK),&OKRect);
 		ScreenRectToClientRect (hWndDlg,&OKRect);
        GetWindowRect (GetDlgItem (hWndDlg,IDCANCEL),&Rect);
		CancelWidth = RECTWIDTH (&Rect);
		CancelHeight = RECTHEIGHT (&Rect);
		MinWidth = 3 * CancelWidth;
        GetWindowRect (GetDlgItem (hWndDlg,IDC_LIST),&Rect);
		CRect = Rect;
		ScreenRectToClientRect (hWndDlg,&CRect);
		GetWindowRect (hWndDlg,&Rect);
		GetClientRect (hWndDlg,&CLRect);
		incW = RECTWIDTH (&Rect) - RECTWIDTH (&CLRect);
		incH = RECTHEIGHT (&Rect) - RECTHEIGHT (&CLRect);
		MoveWindow(hWndDlg, Rect.left,Rect.top, w+2*CRect.left+incW,h+CRect.top+CRect.left*2+incH, FALSE);
       	cwCenter(hWndDlg, -2);   
		SetWindowPos (GetDlgItem (hWndDlg,IDC_LIST),0,0,0,w,h,SWP_NOMOVE|SWP_NOZORDER);
//       GetWindowRect (GetDlgItem (hWndDlg,IDC_LISTHEADER),&Rect);
//		SetWindowPos (GetDlgItem (hWndDlg,IDC_LISTHEADER),0,0,0,w,RECTHEIGHT(&Rect),SWP_NOMOVE|SWP_NOZORDER);
        GetWindowRect (GetDlgItem (hWndDlg,IDC_LIST),&Rect);
        GetWindowRect (hWndDlg,&Rect2);
//		ClientRectToScreenRect (hWndMain,&Rect2);
//		MoveWindow(hWndDlg, Rect2.left+4,Rect2.top+4, max(MinWidth,Rect.right-Rect2.left+5+Rect.left-Rect2.left),
//													  min(MaxHeight,Rect.bottom-Rect2.top+Rect.top-Rect2.top+4), FALSE);
/*        GetWindowRect (GetDlgItem (hWndDlg,IDCANCEL),&Rect);
		w = Rect.right - Rect.left;
        GetWindowRect (hWndDlg,&Rect2);
        if (UseFile)
        {
	        Rect.left = Rect2.right-5-(Rect.right-Rect.left);
	        Rect.right = Rect.left + w; 
	    }
	    else
        {
	        Rect.left = Rect2.left + 5;
	        Rect.right = Rect.left + w; 
	    }
		ScreenRectToClientRect (hWndDlg,&Rect);*/
		MoveWindow(GetDlgItem (hWndDlg,IDCANCEL), w-CancelWidth-OKRect.left,OKRect.top, CancelWidth,CancelHeight, FALSE);
        
		
		 break;                              
    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
            break; 
            
            case IDC_LIST:
				switch(HIWORD(wParam))
				{
					case LBN_SELCHANGE:
						if (!UseFile)
						{
						}
						break;
				 	case LBN_DBLCLK:
				     	PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
				 	break;
				}
			break;
			 
            case IDOK: 
            {
            	short	nItems;
            	BOOL	ReturnFullLine=FALSE;
            	
		        Args = GlobalLock (hSelectItemsArgs);
            	if (*Args == 'C' || *Args == 'c') 
            		Fid = GSSiOpenFile (&Args[512],&OFStruct,OF_CREATE);
            	else if (*Args == 'N')
            		Fid = HFILE_ERROR;	
				else
				{
            		Fid = GSSiOpenFile (&Args[512],&OFStruct,OF_READWRITE);
					if (Fid != HFILE_ERROR)
						GSSillseek (Fid,0,2);
				}
				ReturnFullLine = atob (&Args[1]);
	            rtn=nItems=SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETSELCOUNT,0,0);
	            if (nItems)
				{
	                HANDLE	hItems = GSSiGlobAlloc (1243,GMEM_MOVEABLE,nItems*4);
	                LPINT	lpItems = (LPINT)GlobalLock(hItems);
	                LPSTR	pTAB;
	                
	                SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETSELITEMS,nItems,(LPARAM)lpItems); 
					while (nItems--)
					{   
	                	SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETTEXT,*lpItems++,(LPARAM)str); 
	                	if ((pTAB = _fstrchr (str,'\t')))
	                	{
	                		pTAB++;
							if (Fid == HFILE_ERROR)
	                			_fstrcpy (pTEXTSTRING,pTAB);
	                		else if (ReturnFullLine)
	                		{   
	                			pTAB--;
	                			*pTAB='|';
								fputstring (str,Fid);
							}		
	                		else
								fputstring (pTAB,Fid);		
	                	}
					} 
					GSSiGlobUlFree (&hItems); 
					if (Fid != HFILE_ERROR)
						GSSiClose2 (&Fid);
				}
				GlobalUnlock (hSelectItemsArgs);
                GSSiEndDialog(hWndDlg, rtn,hSaveBM);
            }
            break;    
            
         }
         break; 

    default:
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL FAR PASCAL SELECTDBITEMSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1107);
#endif
{   
	static	HANDLE	hSaveBM; 
	HANDLE	hSQL=0;
	LPSTR	Args, pStr1, pStr2, pStr, pTabs, pRect; 
	HANDLE	hStr, hTabs;
	RECT	Rect, Rect2, Rect3, cRectMain;
	int		err;
	short	w,h, Index;
	int   	TabStops[32]={5000};
	int		nTabs=1;
   	char	VarName[66];

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG: 
        
		hSaveBM = EnterBlockingWindow (hWndDlg);
        Args = GlobalLock (hSelectItemsArgs);
//        if (*Args == 'N')
         	cwCenter(hWndDlg, -2);   
//			_fstrcpy (&Arg1[0],Arg[1]); //database
//			_fstrcpy (&Arg5[128],Arg[2]);//SQL
//			_fstrcpy (&Arg5[128+1024],Arg[3]);//prompt
//			_fstrcpy (&Arg5[128+1024+256],Arg[4]);//displayval
//			_fstrcpy (&Arg5[128+1024+256+1024],Arg[5]);//returnval
//			_fstrcpy (&Arg5[128+1024+256+1024+256],Arg[6]);//returnvarname
//			_fstrcpy (&Arg5[128+1024+256+1024+256+64],Arg[7]);//handlevarname   
//			_fstrcpy (&Arg1[128+1024+256+1024+256+64+64],Arg[8]);//TABS
        	  
        SetWindowText (hWndDlg,&Args[128+1024]);
		pTabs = &Args[128+1024+256+1024+256+64+64];
		nTabs = GetIntsFromList (pTabs,&hTabs);
		if (nTabs)
		{
			LPINT pTabs = GlobalLock (hTabs);
			SendDlgItemMessage (hWndDlg,IDC_LIST,LB_SETTABSTOPS,nTabs,(LPARAM)pTabs);
			GSSiGlobUlFree (&hTabs);
		}
		else
			SendDlgItemMessage (hWndDlg,IDC_LIST,LB_SETTABSTOPS,1,(LPARAM)TabStops);
	    if (OpenDataFile (Args,&Args[128],BT_READ,&hSQL))
		{	    
		    hStr = GSSiGlobAlloc (1245,GMEM_MOVEABLE,4096);
		    pStr = GlobalLock (hStr);
		    pStr1 = pStr+2048;
		    pStr2 = pStr1+1024;
		    while (FetchDBRec (hSQL))
		    {
		        _fstrcpy (pStr1,&Args[128+1024+256]);
		        ExpandText (pStr1);
		        _fstrcpy (pStr2,&Args[128+1024+256+1024]);
		        ExpandText (pStr2);
		        sprintf (pStr,"%s\t%s",pStr1,pStr2);
		        Index = SendDlgItemMessage (hWndDlg,IDC_LIST,LB_ADDSTRING,0,(LPARAM)pStr);  
			    SendDlgItemMessage (hWndDlg,IDC_LIST,LB_GETITEMRECT,Index,(LPARAM)&Rect);
		    }
		    CloseDataFile (TRUE, &hSQL);   
		    GSSiGlobUlFree (&hStr);
		}
		GlobalUnlock (hSelectItemsArgs); 

		pRect = (LPSTR)&Args[128+1024+256+1024+256+64+64+256];
		Rect = atorect (pRect,&err);
		if (!err)
		{
			int margins1,margins2;

			GetWindowRect (hWndDlg,&Rect2);
			GetClientRect (hWndDlg,&cRectMain);
			GetWindowRect (GetDlgItem (hWndDlg,IDC_LIST),&Rect3);
			margins1 = RECTWIDTH (&cRectMain) - RECTWIDTH(&Rect3);
			margins2 = RECTHEIGHT (&cRectMain) - RECTHEIGHT(&Rect3);
			ScreenRectToClientRect (hWndDlg,&Rect3);
			w = Rect.right - Rect.left;
			h = Rect.bottom -Rect.top; 
			MoveWindow(hWndDlg, Rect.left,Rect.top, w,h, FALSE);
			GetClientRect (hWndDlg,&cRectMain);
			MoveWindow(GetDlgItem (hWndDlg,IDC_LIST), Rect3.left,Rect3.top, w-margins1,RECTHEIGHT (&cRectMain)-margins2, FALSE);
//			GetWindowRect (GetDlgItem (hWndDlg,IDC_LIST),&Rect);
//			GetWindowRect (hWndDlg,&Rect2);
//			MoveWindow(hWndDlg, Rect2.left,Rect2.top, Rect.right-Rect2.left+5,Rect.bottom-Rect2.top+5, FALSE);
			GetWindowRect (GetDlgItem (hWndDlg,IDCANCEL),&Rect);
			w = Rect.right - Rect.left;
			GetWindowRect (hWndDlg,&Rect2);
			Rect.left = Rect2.right-5-(Rect.right-Rect.left);
			Rect.right = Rect.left + w;
			ScreenRectToClientRect (hWndDlg,&Rect);
			MoveWindow(GetDlgItem (hWndDlg,IDCANCEL), Rect.left,Rect.top, Rect.right-Rect.left,Rect.bottom-Rect.top, FALSE);
		}       
		
		 break;                              
    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
		        Args = GlobalLock (hSelectItemsArgs); 
				_fstrcpy (VarName,&Args[128+1024+256+1024+256+64]);
				SetGlobalValueHandle (VarName,0);  
				GSSiGlobUlFree (&hSelectItemsArgs);
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
            break; 
            
            case IDC_LIST:
				switch(HIWORD(wParam))
				{   
				 case LBN_DBLCLK:
				     PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
				 break;
				}
			break;
			 
            case IDOK: 
            {
            	short	nItems;
            	HANDLE	hItems, hReturnVals; 
				int rtn;

		        Args = GlobalLock (hSelectItemsArgs); 
		        _fstrcpy (VarName,&Args[128+1024+256+1024+256+64]);
				rtn = nItems = GetLBSelectedItems (hWndDlg,IDC_LIST,&hItems);
	            if (nItems)
				{
	                LPINT	lpItems = (LPINT)GlobalLock(hItems);
	                LPSTR	pTAB; 
	                LPSTR	pReturnVals;
	                long	lReturnVals;
	                short	lVal;
	                
				    hStr = GSSiGlobAlloc (1246,GMEM_MOVEABLE,4096);
				    pStr = GlobalLock (hStr);
					hReturnVals = GSSiGlobAlloc (1247,GHND,USHRT_MAX);
					SetGlobalValueHandle (VarName,hReturnVals);  
					pReturnVals = GlobalLock (hReturnVals); 
					_fstrcpy (pReturnVals,&Args[128+1024+256+1024+256]);
					lReturnVals = _fstrlen (pReturnVals) + 1;
					while (nItems--)
					{   
	                	SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETTEXT,*lpItems++,(LPARAM)pStr); 
	                	if ((pTAB = _fstrrchr (pStr,'\t')))
	                	{
	                		pTAB++;
	                		lVal = _fstrlen (pTAB);
	                		_fstrcpy (&pReturnVals[lReturnVals],pTAB); 
	                		lReturnVals += lVal + 1;
	                	}
					} 
					GlobalUnlock (hReturnVals);
					GSSiGlobUlFree (&hStr);
					GSSiGlobUlFree (&hItems); 
				}
				GSSiGlobUlFree (&hSelectItemsArgs);
                GSSiEndDialog(hWndDlg, rtn,hSaveBM);
            }
            break;    
            
         }
         break; 

    default:
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL SELECTITEMSMsgProc2(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1107);
#endif
{   
	static	HANDLE	hSaveBM;
	char	str[256], Value[66]; 
	int		i;   
	HFILE	Fid;
	OFSTRUCTGM	OFStruct;
	LPSTR	Args; 
	RECT	Rect, Rect2;
	short	w,h,rtn,index;

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG: 
        
		hSaveBM = EnterBlockingWindow (hWndDlg);
        Args = GlobalLock (hSelectItemsArgs);
        if (*Args == 'N')
         	cwCenter(hWndDlg, -2);   
        	  
        SetWindowText (hWndDlg,&Args[850]);
    	FillList (hWndDlg,IDC_LIST,&Args[2],0,&Rect);
		GlobalUnlock (hSelectItemsArgs); 
/*		w = Rect.right - Rect.left;
		h = Rect.bottom -Rect.top; 
		MoveWindow(GetDlgItem (hWndDlg,IDC_LIST), Rect.left,Rect.top, w,h, FALSE);
        GetWindowRect (GetDlgItem (hWndDlg,IDC_LIST),&Rect);
        GetWindowRect (hWndDlg,&Rect2);
		MoveWindow(hWndDlg, Rect2.left,Rect2.top, Rect.right-Rect2.left+5,Rect.bottom-Rect2.top+5, FALSE);
        GetWindowRect (GetDlgItem (hWndDlg,IDCANCEL),&Rect);
		w = Rect.right - Rect.left;
        GetWindowRect (hWndDlg,&Rect2);
        Rect.left = Rect2.right-5-(Rect.right-Rect.left);
        Rect.right = Rect.left + w;
		ScreenRectToClientRect (hWndDlg,&Rect);
		MoveWindow(GetDlgItem (hWndDlg,IDCANCEL), Rect.left,Rect.top, Rect.right-Rect.left,Rect.bottom-Rect.top, FALSE);
*/        
		
		 break;                              
    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
            break; 
            
            case IDC_LIST:
				switch(HIWORD(wParam))
				{   
				 case LBN_DBLCLK:
				     PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
				 break;
				}
			break;
			 
            case IDC_VALUE:
                 switch (HIWORD(wParam))
                 {  case EN_CHANGE:
                        i = GetDlgItemText (hWndDlg,IDC_VALUE,Value,64);
				 		index = SendDlgItemMessage (hWndDlg,IDC_LIST,LB_FINDSTRING,(WPARAM)-1,(LPARAM) Value); 
				 		SendDlgItemMessage (hWndDlg,IDC_LIST,LB_SETTOPINDEX,(WPARAM)index,(LPARAM)0); 
                        break;
                 }
                 break;

            case IDOK: 
            {
            	short	nItems;
            	BOOL	ReturnFullLine=FALSE;
            	
		        Args = GlobalLock (hSelectItemsArgs);
            	if (*Args == 'C' || *Args == 'c') 
            		Fid = GSSiOpenFile (&Args[512],&OFStruct,OF_CREATE);
            	else if (*Args == 'N')
            		Fid = HFILE_ERROR;	
				else
				{
            		Fid = GSSiOpenFile (&Args[512],&OFStruct,OF_READWRITE);
					if (Fid != HFILE_ERROR)
						GSSillseek (Fid,0,2);
				}
				ReturnFullLine = atob (&Args[1]);
	            rtn=nItems=SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETSELCOUNT,0,0);
	            if (nItems)
				{
	                HANDLE	hItems = GSSiGlobAlloc (1244,GMEM_MOVEABLE,nItems*4);
	                LPINT	lpItems = (LPINT)GlobalLock(hItems);
	                LPSTR	pTAB;
	                
	                SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETSELITEMS,nItems,(LPARAM)lpItems); 
					while (nItems--)
					{   
	                	SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETTEXT,*lpItems++,(LPARAM)str); 
	                	if ((pTAB = _fstrchr (str,'\t')))
	                	{
	                		pTAB++;
							if (Fid == HFILE_ERROR)
	                			_fstrcpy (pTEXTSTRING,pTAB);
	                		else if (ReturnFullLine)
	                		{   
	                			pTAB--;
	                			*pTAB='|';
								fputstring (str,Fid);
							}		
	                		else
								fputstring (pTAB,Fid);		
	                	}
					} 
					GSSiGlobUlFree (&hItems); 
					if (Fid != HFILE_ERROR)
						GSSiClose2 (&Fid);
				}
				GlobalUnlock (hSelectItemsArgs);
                GSSiEndDialog(hWndDlg, rtn,hSaveBM);
            }
            break;    
            
         }
         break; 

    default:
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL DTMTOTEXTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
    short nItems, i, Version=1, NumFields;  
    char    File[128],  ExtID[32], Name[128], str[128];
    LPINT   lpItems;    
    HFILE   Fid;
    OFSTRUCTGM    OFStruct;  
    BOOL    False=FALSE, ComputeElev, VoidsOnly;  
    short     IDC_FieldName=IDC_FIELDS,UnitsOpt, FormatOpt;
    LPSTR   vbar; 
    char    txt[128], txt2[128], project[34]; 
    //BTHEAD  BTHead;
    long    NumItems;
    char	DTMFile[128]; 
    char	Ext[6]; 
    short   Filter, FileVarID, OutVarID;  
	HFILE	FidSave;
    
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
         /* initialize working variables                                */   
    
        GetGlobalCVal ("[%DTMFILE]",DTMFile,"[%DL]attribut\\ot1.dtm");
        SetDlgItemText (hWndDlg,IDC_SOURCEDTM,DTMFile);
		SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
		SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
	    SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees");  
	    SendDlgItemMessage (hWndDlg,IDC_DTMOUTFORMAT,CB_ADDSTRING,0,(LPARAM)"Fixed width Elevation Northing and Easting");  
	    SendDlgItemMessage (hWndDlg,IDC_DTMOUTFORMAT,CB_ADDSTRING,0,(LPARAM)"Comma delimited X,Y,Elevation");  
	    SendDlgItemMessage (hWndDlg,IDC_DTMOUTFORMAT,CB_ADDSTRING,0,(LPARAM)"Elevation Grid");  
	    
	    _fstrcpy (str,"*.CVT");
	    DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE);   
         if (!hHighlight)
         { 
    NoItems:
            MessageBox( GetFocus(),"One area must be highlighted","Error", MB_OK);
            PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
            break;
         }  
         //GetBTHeader (hHighlight,&BTHead); 
         NumItems = BT_NUM_IN_INDEX (hHighlight);  
         if (!NumItems || NumItems > 1) goto NoItems;
         sprintf(txt,"%ld items selected",NumItems);
         SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,txt);  
         
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

         switch(LOWORD(wParam))

           { 
             case IDC_SELECT_OUTPATH:
            {   
                LPSTR   lpDot;   
                char    Name[128]; 
                
			    Filter = IDS_FILTERTEXT;
			    _fstrcpy (Ext,".TXT");
	           OutVarID = IDS_FILETXT;
               if (GetSaveName2 (hWndDlg,Name,Filter,Ext,OutVarID))
                {
                    SetDlgItemText (hWndDlg,IDC_MIF_FILE,Name);
    SetOutDB:       
    				GetDlgItemText (hWndDlg,IDC_MIF_FILE,Name,sizeof(Name));
                }
            }
                break;
            
            case IDC_SELECTDTM:
            {
                _fstrcpy (Ext,".DTM"); 
                _fstrcpy (ExtID,"GeoMaster Terrain Models");
                sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,Ext,_fstrlwr(Ext));
                if (GetFileName3(hWndDlg,DTMFile,0,IDS_FILEDTM)) 
                { 
           	        SetDlgItemText (hWndDlg,IDC_SOURCEDTM,DTMFile);
                }
            }  
            break;
            
            case IDC_SAVE:
            {    
            	 short	Version=1; 
            	 
			     _fstrcpy (Ext,".TL7");
                 if (!GetSaveName2 (hWndDlg,Name,0,Ext,IDS_FILETL7)) break; 
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
                 BigWrite (FidSave,(HPSTR)Ext,6,-1);
                 BigWrite (FidSave,(HPSTR)&Version,2,-1);   
                 GetDlgItemText (hWndDlg,IDC_SOURCEDTM,Name,128);
                 BigWrite (FidSave,(HPSTR)Name,128,-1);
                 GetDlgItemText (hWndDlg,IDC_MIF_FILE,Name,128);
                 BigWrite (FidSave,(HPSTR)Name,128,-1);  
				 FormatOpt=SendDlgItemMessage(hWndDlg,IDC_DTMOUTFORMAT,CB_GETCURSEL,0,0);
				 BigWrite (FidSave,(HPSTR)&FormatOpt,2,-1);                 
                 GetDlgItemText (hWndDlg,IDC_VOIDVAL,Name,32);
                 BigWrite (FidSave,(HPSTR)Name,32,-1);
				 VoidsOnly = SendDlgItemMessage (hWndDlg,IDC_VOIDSONLY,BM_GETCHECK,0,0); 
                 BigWrite (FidSave,(HPSTR)&VoidsOnly,2,-1);
                 GetDlgItemText (hWndDlg,IDC_PROJECTION,curproject,lncurproject);
                 BigWrite (FidSave,(HPSTR)curproject,34,-1);
			     UnitsOpt=SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_GETCURSEL,0,0); 
                 BigWrite (FidSave,(HPSTR)&UnitsOpt,2,-1);
                 GetDlgItemText (hWndDlg,IDC_GRIDSPACE,Name,32);
                 BigWrite (FidSave,(HPSTR)Name,32,-1);
                 GSSiClose2 (&FidSave);
            } 
                    
            	 break;
            	 
           	case IDC_RECALL: 
           	{
           		 short Version;     
           		 
			     _fstrcpy (Ext,".TL7");
				 if (!GetFileName2 (hWndDlg,Name,Ext,IDS_FILETL7))
				 	break;
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_READ);
                 BigRead (FidSave,Ext,6);
                 BigRead (FidSave,(HPSTR)&Version,2);   
                 BigRead (FidSave,(HPSTR)Name,128);
                 SetDlgItemText (hWndDlg,IDC_SOURCEDTM,Name);
                 BigRead (FidSave,Name,128);
                 SetDlgItemText (hWndDlg,IDC_MIF_FILE,Name);
                 BigRead (FidSave,(HPSTR)&FormatOpt,2);
			     SendDlgItemMessage(hWndDlg,IDC_DTMOUTFORMAT,CB_SETCURSEL,FormatOpt,0); 
                 BigRead (FidSave,Name,32);
                 SetDlgItemText (hWndDlg,IDC_VOIDVAL,Name);
                 BigRead (FidSave,(HPSTR)&VoidsOnly,2);  
                 SendDlgItemMessage (hWndDlg,IDC_VOIDSONLY,BM_SETCHECK,VoidsOnly,0L); 
                 BigRead (FidSave,curproject,34);
		         SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,-1,(LPARAM)curproject);
                 BigRead (FidSave,(HPSTR)&UnitsOpt,2);
			     SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_SETCURSEL,UnitsOpt,0); 
                 BigRead (FidSave,Name,32);
                 SetDlgItemText (hWndDlg,IDC_GRIDSPACE,Name);
                 GSSiClose2 (&FidSave);
            }
           		 break;
           		 
            case IDOK: 
            {
                HFILE   FidMIF, FidTemp;
                OFSTRUCTGM    OFStruct;  
                short     pos, nTranFile, nParts;  
                LPSTR	lpSC;
                long    CurItem=0, iref, RecNum=0, Index=0, NumDBFRecs=0;
                LPTHEME pTheme;                                        
                HIGHLIGHTDATA   HighlightData;  
                short     nareas;
                DPOINT  CP; 
                BOOL    First, Rtn=FALSE;
                LPSTR	lpDot, lpName; 
                char    SQL[256],Type[32],DBName[128],fmt[34]; 
                HANDLE	hOutRec = GSSiGlobAlloc (1101,GMEM_MOVEABLE,USHRT_MAX);
                LPSTR	OutRec = GlobalLock (hOutRec);
                LPOPENFILEDATA  FilePtr, FilePtrATT;
                LPOPENSQLDATA   SQLPtr, SQLPtrATT;
                DPOINT	BP,POC,EP;
                HANDLE  hSQL;
                short IDB; 
                double	coordcvt=1, MinX=200000.0,MaxX=700000.0,MinY=10000.0,MaxY=350000.0,MinZ=-100.0,MaxZ=1500.0;  
                long	SHPOff, NumMiss,numpoints;
                LPFIELDINFO	lpFldInfo;  
                short	ShapeType;
                HANDLE	hIndex=0;
                LPLONG	pIndex;  
                LPWORD	pPolyParts; 
                BOOL	SaveDisplaySymbol, WantRec,FirstInRec, HaveVoidElev;
                long	nSavePoly2;
                HANDLE	hSavePoly2;
                static	long	debugref=1743054;       
                LPSHORT	pCurvePoints;
                short	Neg1=-1;
                HANDLE	hSurf=0;  
                HPDPOINT	lpFirstPoint;  
                double	VoidElev, VElev=-1000, GridSpace, Elv, HPElev, maxdif=-9999, avedif=0;
                long	nRow, nCol, Col, npt=0;
                DPOINT	BeginPoint, Point;    
               	long		npnts;
               	HANDLE	hPoly=0;
				LPGWDHEADER lpGWDHead;
				HANDLE	hDBTemp=0, hDBTemp2=0; 
				long	Offset,NumVoid=0;   
				short	nCharPerCell=8;
				int		ndp;
			typedef	struct {long x,y;
							float Elev;}TEMPKEY;
			typedef TEMPKEY	FAR	*LPTEMPKEY;
			LPTEMPKEY	pTempKey;
							

                
				if ((FormatOpt=SendDlgItemMessage(hWndDlg,IDC_DTMOUTFORMAT,CB_GETCURSEL,0,0)) == CB_ERR)
                {
                    MessageBox(GetFocus(),"No output format set", 0,MB_ICONEXCLAMATION|MB_OK);
                    goto Exit;
                }
				 
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
                    if (!_fstrcmp(str,"Feet"))
                        PRJ_UNITS[3] = 1;
                    else if (!_fstrcmp(str,"Meters"))
                        PRJ_UNITS[3] = 2;
                }
                else
                {
                    MessageBox(GetFocus(),"Units field not set", 0,MB_ICONQUESTION|MB_OK);
                    goto Exit;
                }
				CloseTRANS2 (&hTranExport[0]);
				CloseTRANS2 (&hTranExport[1]);
                GetDlgItemText (hWndDlg,IDC_GRIDSPACE,str,34);
                rread (str, &GridSpace ,&ndp);
                if (!GridSpace)
                {
                    MessageBox(GetFocus(),"Grid spacing not set", 0,MB_ICONQUESTION|MB_OK);
                    goto Exit;
                }
                sprintf (fmt,"%%.%if,%%.%if,%%.2f",ndp,ndp);  
                GetDlgItemText (hWndDlg,IDC_SOURCEDTM,DTMFile,sizeof(DTMFile)-1);
               	if (!(hSurf = DTMOpen (DTMFile,NULL_ELV,BT_READ,0)))
                {
                    MessageBox(GetFocus(),"Unable to open surface", 0,MB_ICONQUESTION|MB_OK);
                    goto Exit;
                } 
/*                FidTemp = GSSiOpenFile ("[%DL]attribut\\hpelev.txt",&OFStruct,OF_READ);
                while (fgetstring (str,200,FidTemp))
                {
                	sscanf (str,"%f %f %f",&HPElev,&Point.y,&Point.x); 
                	ConvertCoord(&Point,3,1); 
                	Elv = NGIELV (Point,hSurf,0); 
                	maxdif = max (maxdif,fabs(Elv - HPElev));
                	avedif += fabs(Elv - HPElev);
                	npt++;
                } 
                _lclose (FidTemp);  */    
                HaveVoidElev = GetDlgItemText (hWndDlg,IDC_VOIDVAL,str,24); 
                VoidElev = atof (str); 
                if (!VoidElev)
                	HaveVoidElev = FALSE; 
				VoidsOnly = SendDlgItemMessage (hWndDlg,IDC_VOIDSONLY,BM_GETCHECK,0,0); 
//                avedif /= npt;
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT2),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE); 
                DisableHalt = TRUE;  
				SetContinueProcessing(TRUE);
				Processing = TRUE;
                //GetBTHeader (hHighlight,&BTHead); 
                NumItems = BT_NUM_IN_INDEX (hHighlight);                              
                GetDlgItemText (hWndDlg,IDC_MIF_FILE,Name,sizeof(Name));
                FidMIF = GSSiOpenFile (Name,&OFStruct,OF_CREATE); 
                SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,"Creating extract file");

//				hDBTemp = OpenGWDatabase ("[%%DL]attribut\\dtm176.gmd",BT_READ);   
//				hDBTemp2 = OpenGWDatabase ("[%%DL]attribut\\dtmtext2.gmd",BT_READ);   

    			switch (FormatOpt)
    			{
    				case 0:
        				break;
        			case 1:
		            	fputstring ("X,Y,Elevation",FidMIF); 
        				break; 
        			case 2: 
        				break;
        		}
                pos = BT_FIRST;
                if (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData)&&ContinueProcessing)
                {   
                	long	ii;
                	MNMXCORD	NewBounds; 
                	float	OldElev=-1;
                	double	minx,maxx,miny,maxy,NextXIntersect;
                	
					ConvertRectCoord (&NewBounds, &HighlightData.PD.Rect, 1,3); 
					minx = NewBounds.xmn - fmod (NewBounds.xmn,GridSpace);
					if (minx < NewBounds.xmn)
						minx += GridSpace;
					miny = NewBounds.ymn - fmod (NewBounds.ymn,GridSpace);
					if (miny < NewBounds.ymn)
						miny += GridSpace;
					maxx = NewBounds.xmx - fmod (NewBounds.xmx,GridSpace);
					maxy = NewBounds.ymx - fmod (NewBounds.ymx,GridSpace);
                	nRow = IDNINT((maxy - miny) / GridSpace) + 1;
                	nCol = IDNINT((maxx - minx) / GridSpace) + 1; 
                	NumItems = nRow * nCol;  
                	CurItem = 0;
                    pos = BT_NEXT;  
                    BeginPoint.x = minx;
                    BeginPoint.y = miny;
                    if (FormatOpt == 2)
                    {
	                    BeginPoint.y = maxy;
                    	sprintf (OutRec,"%ld %ld %f %ld %i %f %f",nRow,nCol,GridSpace,IDNINT(VoidElev*100),nCharPerCell,BeginPoint.x,BeginPoint.y);
                    	fputstring (OutRec,FidMIF);
                    }
                    BP = BeginPoint; 
					if (GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&npnts,&hPoly))
                    while (ContinueProcessing && nRow--)
                    {   
                    	HPDPOINT	pArea;
                    	BOOL	rtn;
                    	
                    	BP.x = BeginPoint.x;
                    	Col = nCol; 
                    	*OutRec = 0; 
                    	NextXIntersect=-DBL_MAX;
                    	while (Col--)
                    	{
                        	Point = BP;
                        	ConvertCoord(&Point,3,1); 
                        	if (!IS_BASE[3] || Point.x > NextXIntersect)
                        	{
								pArea = (HPDPOINT)GlobalLock (hPoly);
								rtn = POINT_IN_AREAD (Point, npnts, pArea,1,0,&NextXIntersect,0);
								GlobalUnlock (hPoly);
							} 
							if (rtn || FormatOpt == 2)
							{
								if (rtn)
								{
/*									if (hDBTemp)
									{
										lpGWDHead = (LPGWDHEADER)GlobalLock (hDBTemp); 
										pTempKey = &lpGWDHead->GWDData; 
										pTempKey->x = IDNINT (BP.x);
										pTempKey->y = IDNINT (BP.y);
									    GWDFormKey(lpGWDHead,0,TRUE,0);
									   	if (!BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_FIRST,BT_EQ,(LPSTR)&Offset)) 
										{
										   	FillGWDData (lpGWDHead,Offset);
										   	OldElev = pTempKey->Elev;
										}
										else
											OldElev = -1000;
										GlobalUnlock (hDBTemp);
		                        	}
									if (OldElev < 0 && hDBTemp2)
									{
										lpGWDHead = (LPGWDHEADER)GlobalLock (hDBTemp2); 
										pTempKey = &lpGWDHead->GWDData; 
										pTempKey->x = IDNINT (BP.x);
										pTempKey->y = IDNINT (BP.y);
									    GWDFormKey(lpGWDHead,0,TRUE,0);
									   	if (!BT_FIND (lpGWDHead->BTHandle[0],lpGWDHead->pKeys[0],BT_FIRST,BT_EQ,(LPSTR)&Offset)) 
										{
										   	FillGWDData (lpGWDHead,Offset);
										   	OldElev = pTempKey->Elev;
										}
										else
											OldElev = -1000;
										GlobalUnlock (hDBTemp2);
		                        	}*/
		                        	Elv = NGIELV (Point,hSurf,0);
		                        	if (Elv != NULL_ELV || OldElev > 0)
		                        	{   
		                        		if (Elv == NULL_ELV)
		                        			Elv = OldElev;
		                        		if (!VoidsOnly)
		                        		{   
		                        			if (Elv > 10000 || Elv < -10000)
		                        				ii=1;
		                        			switch (FormatOpt)
		                        			{
		                        				case 0:
			                        				sprintf (OutRec,"%7.2f%10.2f%11.2f",Elv,BP.y,BP.x); 
					                        		fputstring (OutRec,FidMIF); 
			                        				break;
			                        			case 1:
			                        				sprintf (OutRec,fmt,BP.x,BP.y,Elv); 
					                        		fputstring (OutRec,FidMIF); 
			                        				break;
			                        			case 2:
			                        				sprintf (_fstrchr(OutRec,0),"%8ld",IDNINT(100*Elv));
			                        				break;
			                        		}
			                        	}
			                        	goto NextCol;
		                        	} 
		                        }
		                        if (VoidsOnly && !rtn)
		                        	goto NextCol;
	                        	if (HaveVoidElev || FormatOpt == 2)
	                        	{   
	                        		NumVoid++;
                        			VElev = VoidElev;
                        			switch (FormatOpt)
                        			{
                        				case 0:
	                        				sprintf (OutRec,"%7.2f%10.2f%11.2f",VElev,BP.y,BP.x); 
				                        	fputstring (OutRec,FidMIF); 
	                        				break;
	                        			case 1:
	                        				sprintf (OutRec,fmt,BP.x,BP.y,VElev); 
	                        				fputstring (OutRec,FidMIF); 
											break; 
	                        			case 2:
	                        				sprintf (_fstrchr(OutRec,0),"%8ld",IDNINT(VElev*100));
	                        				break;
	                        		}
		                        } 
		                        else
		                        	NumVoid++;
	                        	 
	                        }
	               NextCol:
                        	BP.x += GridSpace;
                        }
	                    if (FormatOpt == 2)
	                    {
	                    	fputstring (OutRec,FidMIF);
                        	BP.y -= GridSpace;
                        }
                        else
                        	BP.y += GridSpace;
	                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem+=nCol,0);
                    }
                    GSSiGlobFree (&hPoly);     
                } 
                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0); 
                sprintf (OutRec,"Extract finished: %ld voids",NumVoid);
		        SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,OutRec); 
                GSSiClose2 (&FidMIF);   
Exit:
Exit2:          
				CloseGWDatabase (hDBTemp);
				CloseGWDatabase (hDBTemp2);
				DTMClose (&hSurf);
				CloseTRANS2 (&hTranExport[0]);
				CloseTRANS2 (&hTranExport[1]);
				GSSiGlobUlFree(&hOutRec);
                DisableHalt = FALSE;  
                SetContinueProcessing ( TRUE);   
                Processing = FALSE;
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT2),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE); 
                break;
            }
            case IDC_EXIT2:     
                 EndDialog(hWndDlg, FALSE); 
                 
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)
					 SetContinueProcessing(FALSE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT2),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE); 
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}


BOOL FAR PASCAL CmpImageMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
	char	str[512];
	UINT	hI; 
 switch(Message)
   {
    case WM_INITDIALOG:  
    	 PostMessage(hWndDlg, WM_COMMAND, IDC_LOADFREEIMAGEBMP, 0L);
         return 0;  
         break; 
    
    case WM_PAINT:
//         ShowWindow (hWndDlg,SW_HIDE);
         return 0;
    	 break;
    	 
    case WM_CLOSE:
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; 

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
			case IDC_LOADFREEIMAGEBMP:
			{
					char cmpExe[]="C:\\Gssi\\prog\\cmpframe\\Release\\cmpframe.exe";
            	//sprintf (str,"%s %s;%s;%ld;%ld","[%DL]cmpframe.exe",CmpImageInFile,CmpImageOutFile,CmpImageQuality,(long)hWndDlg);
            	sprintf (str,"%s %s;%s;%ld;%ld",cmpExe,CmpImageInFile,CmpImageOutFile,CmpImageQuality,(long)hWndDlg);
               	ExpandText (str);
				hI = WinExec (str,SW_SHOWNORMAL);
				if (hI <32)
				{
					DisplayShellExError (hI,str);
					PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
				} 
			}
            	break;
            case IDOK:
                EndDialog(hWndDlg, (int)lParam);
            	break;
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break;
         }
         break; 

    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL FillSymbolListFromVariable (HWND hWndDlg,UINT idc_SYMBOL_LIST,UINT idc_SQL_LIST,LPSTR SymName,LPSTR cWidth,LPSTR cRot,LPSTR cColor,HANDLE hDB)
{
	BOOL rtn = FALSE;
	HANDLE hBTDistinct;
	int ln=126;
	char value[256], symStuff[512], symnam[128];
	int	count;
	int pos=BT_FIRST;

	if (*SymName != '[' || *LastChr (SymName) != ']')
		return FALSE;
	strncpy0 (symnam,&SymName[1],strlen(SymName)-2);
	hBTDistinct = GetDistinctValues (hWndDlg,SymName,ln,hDB,1024);

	SendDlgItemMessage (hWndDlg,idc_SYMBOL_LIST,LB_RESETCONTENT,0,0);
	SendDlgItemMessage (hWndDlg,idc_SQL_LIST,LB_RESETCONTENT,0,0);

	while (!BT_FIND (hBTDistinct,value,pos,BT_ANY, (LPSTR)&count))
	{
		pos = BT_NEXT;
		sprintf (symStuff,"%s;%s;%s;%s",value,cWidth,cRot,cColor);
  		SendDlgItemMessage (hWndDlg,IDC_SYMBOL_LIST,LB_ADDSTRING,0,(LPARAM)symStuff);
		sprintf (symStuff,"%s = %s",symnam,value);
   		SendDlgItemMessage (hWndDlg,IDC_SQL_LIST,LB_ADDSTRING,0,(LPARAM)symStuff);
	}

	BT_CLOSEANDDELETE (&hBTDistinct);
	return rtn;
}


BOOL FAR PASCAL SETSHAPEPARAMMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
	char	SymName[66], str[260], GSPName[256], cIF[128],SymStuff[256];
	static  char cpWidth[64] = { 0 }, clWidth[64] = { 0 }, cWidth[64], cRot[64] = { 0 }, cColor[64] = { 0 };
	static	char projection[MAX_PATH] = { 0 }, units[64] = { 0 };
	short	i, Choice, rtn;
	LPSTR	pDOT;
	HFILE	Fid;
	MNMXCORD	Bounds;
	static	short	CurProj = -1, CurUnits = -1;
	static	BOOL	IsPGDB, IsFGDB;
	static	BOOL	Opened;
	long	ii;

	short    BRtn;
	if ((BRtn = DIALOGSTYLEMsgProc(hWndDlg, Message, wParam, lParam)))
		return (BRtn);
	switch (Message)
	{
	case WM_INITDIALOG:
	{
						  char originalName[MAX_PATH];

						  Opened = FALSE;
						  SHPIndexType = SHP_INDEX_SIMPLE;
						  GetSHPName(str);
						  strcpy(originalName, str);
						  SetDlgItemText(hWndDlg, IDC_SHAPEFILE, str);
						  _fstrcpy(GSPName, str);
						  IsPGDB = FALSE;
						  IsFGDB = FALSE;
						  *PGDBTable = 0;
						  if ((pDOT = _fstrrchr(GSPName, '.')))
						  {
							  if (!_fstrnicmp(pDOT, ".mdb", 4))
							  {
								  if (*(pDOT + 4) != '(')
									  return FALSE;
								  IsPGDB = TRUE;
								  _fstrcpy(PGDBTable, pDOT + 5);
								  *LastChr(PGDBTable) = 0;
								  *pDOT = 0;
								  sprintf(str, "%s_%s.gsp", GSPName, PGDBTable);
								  _fstrcpy(GSPName, str);
							  }
							  else if (!_fstrnicmp(pDOT, ".gdb", 4))
							  {
								  char fgdbPath[MAX_PATH];

								  if (*(pDOT + 4) != '(')
									  return FALSE;
								  IsFGDB = TRUE;
								  _fstrcpy(FGDBTable, pDOT + 5);
								  *LastChr(FGDBTable) = 0;
								  *pDOT = 0;
								  sprintf(fgdbPath, "%s.gdb", GSPName);
								  sprintf(str, "%s_%s.gsp", GSPName, FGDBTable);
								  _fstrcpy(GSPName, str);
								  OpenFGDB(fgdbPath, FGDBTable, "");
								  hSHPDBF = FGDBHandle;
								  Opened = TRUE;
							  }
							  else
								  _fstrcpy(pDOT, ".gsp");
						  }
						  else
							  break;
						  SetCurVal(GSPName, IDS_FILEGSP);
						  LoadTAGDef();
						  if (NumTAGDef)
						  {
							  LPTAGDEF    lpTAGDef;

							  lpTAGDef = (LPTAGDEF)GlobalLock(hTAGDef);
							  for (i = 0; i < NumTAGDef; i++, lpTAGDef++)
							  {
								  SendDlgItemMessage(hWndDlg, IDC_TAPREFIX, CB_ADDSTRING, 0, (LPARAM)lpTAGDef->Prefix);
								  SendDlgItemMessage(hWndDlg, IDC_REF_PREFIX, CB_ADDSTRING, 0, (LPARAM)lpTAGDef->Prefix);
							  }
							  GlobalUnlock(hTAGDef);
						  }
						  EnableWindow(GetDlgItem(hWndDlg, IDC_SAADD), TRUE);
						  SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_ADDSTRING, 0, (LPARAM)"Feet");
						  SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_ADDSTRING, 0, (LPARAM)"Meters");
						  SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_ADDSTRING, 0, (LPARAM)"Degrees");
						  SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_ADDSTRING, 0, (LPARAM)"Degrees * 1000000");
						  FillProjectionList(hWndDlg, IDC_PROJECTION, &CurProj, IDC_UNITS, &CurUnits);
						  //         _fstrcpy (str,"*.CVT");
						  //         DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE); 
						  if (PRJ_UNITS[1] == 1)
							  SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
						  else if (PRJ_UNITS[1] == 2)
							  SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_SETCURSEL, (WPARAM)1, (LPARAM)0);
						  SendDlgItemMessage(hWndDlg, IDC_PROJECTION, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)"baseproj");
						  SendDlgItemMessage(hWndDlg, IDC_INDEXSTANDARD, BM_SETCHECK, SHPIndexType == SHP_INDEX_STANDARD, 0L);
						  SendDlgItemMessage(hWndDlg, IDC_INDEXSIMPLE, BM_SETCHECK, SHPIndexType == SHP_INDEX_SIMPLE, 0L);
						  SendDlgItemMessage(hWndDlg, IDC_INDEXQUAD, BM_SETCHECK, SHPIndexType == SHP_INDEX_QUAD, 0L);
						  GetSHPName(str);
						  if (IsPGDB)
							  SHPType = ReadPGDBHeader(str, &Bounds);
						  else if (IsFGDB)
							  SHPType = ReadFGDBHeader(str, &Bounds);
						  else
						  {
							  Fid = GSSiOpenFile(str, 0, OF_READ);
							  if (Fid == HFILE_ERROR)
								  goto LoadGSP;
							  SHPType = ReadSHPHeader(Fid, &Bounds, str);
							  GSSiClose2 (&Fid);
						  }
						  ii = SHPType;
						  SetDlgItemText(hWndDlg, IDC_SHPMINX, ftoa(str, Bounds.xmn));
						  SetDlgItemText(hWndDlg, IDC_SHPMAXX, ftoa(str, Bounds.xmx));
						  SetDlgItemText(hWndDlg, IDC_SHPMINY, ftoa(str, Bounds.ymn));
						  SetDlgItemText(hWndDlg, IDC_SHPMAXY, ftoa(str, Bounds.ymx));
						  goto LoadGSP;
	}
		break; /* End of WM_INITDIALOG                                 */

	case WM_COMMAND:

		switch (LOWORD(wParam))

		{
		case IDC_SHOW_FIELDS:
		{

								if (!hSHPDBF)
								{
									char	DBFName[256];

									GetSHPName(str);
									if (IsPGDB)
									{
										OpenPGDBFileIndex(str, 0);
										SetPGDB_SQL("");
									}
									else
									{
										sprintf(DBFName, "SHP=%s", str);
										OpenDataFile(DBFName, "", BT_READ, &hSHPDBF);
									}
									Opened = TRUE;
								}

								DisplayFieldList(hWndDlg, hSHPDBF, 0, 0, 0);
		}
			break;

		case IDC_COPYFROM:
			*GSPName = 0;
			if (!GetFileName3(hWndDlg, GSPName, IDS_FILTERGSP, IDS_FILEGSP))
				break;
		LoadGSP:
			SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_RESETCONTENT, 0, 0);
			{
				HFILE	Fid = GSSiOpenFile(GSPName, 0, OF_READ);
				short	SymNum = 0;

				if (Fid == HFILE_ERROR)
				{
					short	type=0;

					GetDlgItemText(hWndDlg, IDC_SHAPEFILE, str, 256);
					if (!*projection)
					{
						GuessShapeProjection(str, hWndDlg, IDC_PROJECTION, IDC_UNITS, TRUE);
					}
					else
					{
						HFILE	Fid2;

						strcpy(str, projection);
						ExpandText(str);
						Fid2 = GSSiOpenFile(str, 0, OF_READ);
						if (Fid2 != HFILE_ERROR)
						{
							fgetstring(str, 250, Fid2);
							SendDlgItemMessage(hWndDlg, IDC_PROJECTION, CB_SELECTSTRING, -1, (LPARAM)(str + 1));
							GSSiClose2 (&Fid2);
						}
						if (*units)
							SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_SELECTSTRING, -1, (LPARAM)units);
					}
					if (!*cRot)
						strcpy(cRot, "0D");
					if (!cColor)
						strcpy(cColor, "-1");

					switch (SHPType)
					{
					case SHPT_POINT:
					case SHPT_POINTZ:
						GetGlobalCVal("[%DEFAULTPOINTSYMBOL]", SymName, "SQUARE");
						if (!*cpWidth)
							strcpy(cpWidth, "-1");
						type = 1;
						break;
					case SHPT_ARC:
					case SHPT_ARCZ:
					case SHPT_ARCM:
						GetGlobalCVal("[%DEFAULTLINESYMBOL]", SymName, "PEN1");
						if (!*clWidth)
							strcpy(clWidth, "-1");
						type = 2;
						break;
					case 4:
					case SHPT_TEXT:
					case SHPT_POLYGON:
					case SHPT_POLYGONZ:
						GetGlobalCVal("[%DEFAULTAREASYMBOL]", SymName, "PARCEL");
						if (!*cWidth)
							strcpy(cWidth, "-1");
						type = 3;
						break;
					}
					if (!type)
						break;
					if (*PGDBTable)
					{
						sprintf(SymName, "%s,PAR=NEW", PGDBTable);
						SymNum = GetOrCreateSym(hWndDlg, SymName, 0, 0, GetGlobalLVal2("[%ALLOWSYMBOLCREATION]", 0), type);
						_fstrcpy(SymName, PGDBTable);
					}
					else if (*FGDBTable)
					{
						sprintf(SymName, "%s,PAR=NEW", FGDBTable);
						SymNum = GetOrCreateSym(hWndDlg, SymName, 0, 0, GetGlobalLVal2("[%ALLOWSYMBOLCREATION]", 0), type);
						_fstrcpy(SymName, FGDBTable);
					}
					else
					{
						char	fname[80];

						_splitpath(GSPName, 0, 0, fname, 0);
						sprintf(SymName, "%s,PAR=NEW", fname);
						SymNum = GetOrCreateSym(hWndDlg, SymName, 0, 0, GetGlobalLVal2("[%ALLOWSYMBOLCREATION]", 0), type);
						_fstrcpy(SymName, fname);
					}
					if (!SymNum)
					{
						*cWidth = 0;
						*cRot = 0;
						*cColor = 0;
						rtn = 0;
						switch (SHPType)
						{
						case SHPT_POINT:
						case SHPT_POINTZ:
							rtn = SelectPointSymbol(hWndDlg, 1, SymName, "All", cWidth, cRot, cColor, TRUE);
							break;
						case SHPT_ARC:
						case SHPT_ARCZ:
						case SHPT_ARCM:
							rtn = SelectLineSymbol(hWndDlg, 1, SymName, cWidth, cColor, TRUE);
							break;
						case 4:
						case SHPT_TEXT:
						case SHPT_POLYGON:
						case SHPT_POLYGONZ:
							rtn = SelectAreaSymbol(hWndDlg, 1, SymName, cColor, TRUE);
							break;
						}
					}
					else
						GetDictSymName(SymNum, SymName);
					sprintf(SymStuff, "%s;%s;%s;%s", SymName, cWidth, cRot, cColor);
					SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_ADDSTRING, 0, (LPARAM)SymStuff);
					SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_ADDSTRING, 0, (LPARAM)"ALL ROWS");
					break;
				}
				if (fgetstring(str, 128, Fid))
				{
					HFILE	Fid2;

					ExpandText(str);
					Fid2 = GSSiOpenFile(str, 0, OF_READ);
					if (Fid2 != HFILE_ERROR)
					{
						fgetstring(str, 250, Fid2);
						SendDlgItemMessage(hWndDlg, IDC_PROJECTION, CB_SELECTSTRING, -1, (LPARAM)(str + 1));
						GSSiClose2 (&Fid2);
					}
				}
				if (fgetstring(str, 128, Fid))
					SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_SELECTSTRING, -1, (LPARAM)str);
				if (fgetstring(str, 128, Fid))
					SetDlgItemText(hWndDlg, IDC_STARTNO, str);
				if (fgetstring(str, 200, Fid))
				{
					LPSTR	pSC = _fstrchr(str, ':');

					if (pSC)
					{
						*pSC++ = 0;
						SetDlgItemText(hWndDlg, IDC_UDI, pSC);
					}
					SetDlgItemText(hWndDlg, IDC_TAPREFIX, str);
				}
				if (fgetstring(str, 128, Fid))
					SHPIndexType = atoi(str);
				SendDlgItemMessage(hWndDlg, IDC_INDEXSTANDARD, BM_SETCHECK, SHPIndexType == SHP_INDEX_STANDARD, 0L);
				SendDlgItemMessage(hWndDlg, IDC_INDEXSIMPLE, BM_SETCHECK, SHPIndexType == SHP_INDEX_SIMPLE, 0L);
				SendDlgItemMessage(hWndDlg, IDC_INDEXQUAD, BM_SETCHECK, SHPIndexType == SHP_INDEX_QUAD, 0L);

				while (fgetstring(str, 256, Fid))
				{
					if (*str == '#')
						break;
					DecodeSHPParam(str, SymName, cIF, cColor, cWidth, cRot);
					sprintf(str, "%s;%s;%s;%s", SymName, cWidth, cRot, cColor);
					SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_ADDSTRING, 0, (LPARAM)str);
					if (*cIF)
						SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_ADDSTRING, 0, (LPARAM)cIF);
					else
						SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_ADDSTRING, 0, (LPARAM)"ALL ROWS");
				}
				if (fgetstring(str, 256, Fid))
					SetDlgItemText(hWndDlg, IDC_BEGINDATE, str);
				if (fgetstring(str, 256, Fid))
					SetDlgItemText(hWndDlg, IDC_ENDDATE, str);
				GSSiClose2 (&Fid);
			}

			break;

		case IDC_ASSIGNCVT:
			GetDlgItemText(hWndDlg, IDC_SHAPEFILE, str, 128);
			GuessShapeProjection(str, hWndDlg, IDC_PROJECTION, IDC_UNITS, FALSE);
			break;

		case IDC_SAADD:
		{
						  HANDLE hMem;
						  LPSTR  lpStr, lpWhere;

						  if (!hSHPDBF)
						  {
							  char	DBFName[256];

							  GetSHPName(str);
							  if (IsPGDB)
							  {
								  OpenPGDBFileIndex(str, 0);
								  SetPGDB_SQL("");
							  }
							  else
							  {
								  sprintf(DBFName, "SHP=%s", str);
								  OpenDataFile(DBFName, "", BT_READ, &hSHPDBF);
							  }
							  Opened = TRUE;
						  }
						  *SymName = 0;
						  *cWidth = 0;
						  *cRot = 0;
						  *cColor = 0;
						  rtn = 0;
						  switch (SHPType)
						  {
						  case SHPT_POINT:
						  case SHPT_POINTZ:
							  rtn = SelectPointSymbol(hWndDlg, 1, SymName, "All", cWidth, cRot, cColor, FALSE);
							  break;
						  case SHPT_ARC:
						  case SHPT_ARCZ:
						  case SHPT_ARCM:
							  rtn = SelectLineSymbol(hWndDlg, 1, SymName, cWidth, cColor, FALSE);
							  break;
						  case 4:
						  case SHPT_TEXT:
						  case SHPT_POLYGON:
						  case SHPT_POLYGONZ:
							  rtn = SelectAreaSymbol(hWndDlg, 1, SymName, cColor, FALSE);
							  break;
						  }
						  if (!rtn)
							  break;
						  hMem = GSSiGlobAlloc(1421, GHND, 4096);
						  lpStr = GlobalLock(hMem);
						  if (GetSQLWhereClause(hWndDlg, hSHPDBF, lpStr))
						  {
							  sprintf(str, "%s;%s;%s;%s", SymName, cWidth, cRot, cColor);
							  SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_ADDSTRING, 0, (LPARAM)str);
							  SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_ADDSTRING, 0, (LPARAM)lpStr);
						  }
						  GSSiGlobUlFree(&hMem);
						  break;
		}
		case IDC_SQL_LIST:
		case IDC_SYMBOL_LIST:
		{
								WPARAM	OtherParam;

								switch (HIWORD(wParam))
								{
								case LBN_DBLCLK:
									PostMessage(hWndDlg, WM_COMMAND, IDC_SAMODIFY, 0L);
									break;
								case LBN_SELCHANGE:
									Choice = SendDlgItemMessage(hWndDlg, LOWORD(wParam), LB_GETCURSEL, 0, 0);
									if (LOWORD(wParam) == IDC_SQL_LIST)
										OtherParam = IDC_SYMBOL_LIST;
									else
										OtherParam = IDC_SQL_LIST;
									SendDlgItemMessage(hWndDlg, OtherParam, LB_SETCURSEL, Choice, 0);
									EnableWindow(GetDlgItem(hWndDlg, IDC_SADELETE), TRUE);
									EnableWindow(GetDlgItem(hWndDlg, IDC_SAMODIFY), TRUE);
									break;
								}
		}
			break;
		case IDC_SADELETE:
			Choice = max(SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_GETCURSEL, 0, 0),
				SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_GETCURSEL, 0, 0));
			SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_DELETESTRING, Choice, 0);
			SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_DELETESTRING, Choice, 0);
			break;

		case IDC_SAMODIFY:
			if ((Choice = SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_GETCURSEL, 0, 0)) >= 0)
			{
				SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_GETTEXT, Choice, (DWORD)SymStuff);
				SetSymParms(SymStuff, SymName, cWidth, cRot, cColor);
				rtn = 0;
				switch (SHPType)
				{
				case SHPT_POINT:
				case SHPT_POINTZ:
					rtn = SelectPointSymbol(hWndDlg, 1, SymName, "All", cWidth, cRot, cColor, FALSE);
					break;
				case SHPT_ARC:
				case SHPT_ARCZ:
				case SHPT_ARCM:
					rtn = SelectLineSymbol(hWndDlg, 1, SymName, cWidth, cColor, FALSE);
					break;
				case 4:
				case SHPT_TEXT:
				case SHPT_POLYGON:
				case SHPT_POLYGONZ:
					rtn = SelectAreaSymbol(hWndDlg, 1, SymName, cColor, FALSE);
					break;
				}
				if (!rtn)
					break;
				if (*SymName == '[')
					FillSymbolListFromVariable(hWndDlg, IDC_SYMBOL_LIST, IDC_SQL_LIST, SymName, cWidth, cRot, cColor, hSHPDBF);
				else
				{
					SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_DELETESTRING, Choice, 0);
					sprintf(SymStuff, "%s;%s;%s;%s", SymName, cWidth, cRot, cColor);
					SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_INSERTSTRING, Choice, (LPARAM)SymStuff);
				}
			}
			else
			if ((Choice = SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_GETCURSEL, 0, 0)) >= 0)
			{
				HANDLE	hMem;
				LPSTR	lpStr;

				hMem = GSSiGlobAlloc(1422, GHND, 4096);
				lpStr = GlobalLock(hMem);
				SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_GETTEXT, Choice, (DWORD)lpStr);
				if (!hSHPDBF)
				{
					char	DBFName[256];

					GetSHPName(str);
					if (IsPGDB)
					{
						OpenPGDBFileIndex(str, 0);
						SetPGDB_SQL("");
					}
					else
					{
						sprintf(DBFName, "SHP=%s", str);
						OpenDataFile(DBFName, "", BT_READ, &hSHPDBF);
					}
					Opened = TRUE;
				}
				if (GetSQLWhereClause(hWndDlg, hSHPDBF, lpStr))
				{
					SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_DELETESTRING, Choice, 0);
					SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_INSERTSTRING, Choice, (LPARAM)lpStr);
				}
				GSSiGlobUlFree(&hMem);
			}

			break;

		case IDCANCEL:
			SetGlobalValueBool("%AUTOSHPPARM", FALSE);
			if (Opened)
			{
				if (IsPGDB)
					OpenPGDB(0, 0, 0);
				else if (IsFGDB)
					OpenFGDB(0, 0, 0);
				else
					CloseDataFile(TRUE, &hSHPDBF);
				DestroyFieldList();
				hSHPDBF = 0;
			}
			EndDialog(hWndDlg, FALSE);
			break;

		case IDOK:
			GetSHPName(str);
			_fstrcpy(GSPName, str);
			if ((pDOT = _fstrrchr(GSPName, '.')))
			{
				if (!_fstrnicmp(pDOT, ".mdb", 4))
				{
					if (*(pDOT + 4) != '(')
						break;
					IsPGDB = TRUE;
					_fstrcpy(PGDBTable, pDOT + 5);
					*LastChr(PGDBTable) = 0;
					*pDOT = 0;
					sprintf(str, "%s_%s.gsp", GSPName, PGDBTable);
					_fstrcpy(GSPName, str);
				}
				else if (!_fstrnicmp(pDOT, ".gdb", 4))
				{
					if (*(pDOT + 4) != '(')
						break;
					IsFGDB = TRUE;
					_fstrcpy(FGDBTable, pDOT + 5);
					*LastChr(FGDBTable) = 0;
					*pDOT = 0;
					sprintf(str, "%s_%s.gsp", GSPName, FGDBTable);
					_fstrcpy(GSPName, str);
				}
				else
					_fstrcpy(pDOT, ".gsp");
			}
			else
				break;
			Fid = GSSiOpenFile(GSPName, 0, OF_CREATE);
			if (Fid == HFILE_ERROR)
			{
				MessageBox(hWndDlg, GSPName, "Unable to create file", MB_ICONEXCLAMATION);
				break;
			}
			//GetDlgItemText (hWndDlg,IDC_PROJECTION,str,sizeof(str)-1);  
			GetProjectionFile(hWndDlg, IDC_PROJECTION, str);
			SubstituteDL(str, TRUE);
			//ExpandText (str);
			strcpy(projection, str);
			fputstring(str, Fid);
			GetDlgItemText(hWndDlg, IDC_UNITS, str, sizeof(str)-1);
			strcpy(units, str);
			fputstring(str, Fid);
			GetDlgItemText(hWndDlg, IDC_STARTNO, str, sizeof(str)-1);
			fputstring(str, Fid);
			if (GetDlgItemText(hWndDlg, IDC_TAPREFIX, str, sizeof(str)-1))
			{
				_fstrcat(str, ":");
				GetDlgItemText(hWndDlg, IDC_UDI, _fstrchr(str, 0), sizeof(str)-_fstrlen(str) - 1);
			}
			fputstring(str, Fid);
			SHPIndexType = SHP_INDEX_STANDARD;
			if (SendDlgItemMessage(hWndDlg, IDC_INDEXSIMPLE, BM_GETCHECK, 0, 0))
				SHPIndexType = SHP_INDEX_SIMPLE;
			else if (SendDlgItemMessage(hWndDlg, IDC_INDEXQUAD, BM_GETCHECK, 0, 0))
				SHPIndexType = SHP_INDEX_QUAD;
			itoa(SHPIndexType, str, 10);
			fputstring(str, Fid);
			i = 0;
			while (SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_GETTEXT, i, (LPARAM)str) != LB_ERR)
			{
				SetSymParms(str, SymName, cWidth, cRot, cColor);
				SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_GETTEXT, i++, (LPARAM)cIF);
				if (!_fstrcmp(cIF, "ALL ROWS"))
					*cIF = 0;
				sprintf(str, "%s;%s;%s;%s;%s", SymName, cIF, cColor, cWidth, cRot);
				fputstring(str, Fid);
			}
			fputstring("#ENDOFLIST", Fid);
			GetDlgItemText(hWndDlg, IDC_BEGINDATE, str, sizeof(str)-1);
			fputstring(str, Fid);
			GetDlgItemText(hWndDlg, IDC_ENDDATE, str, sizeof(str)-1);
			fputstring(str, Fid);
			GSSiClose2 (&Fid);
			if (Opened)
			{
				if (IsPGDB)
					OpenPGDB(0, 0, 0);
				else if (IsFGDB)
					OpenFGDB(0, 0, 0);
				else
					CloseDataFile(TRUE, &hSHPDBF);
				DestroyFieldList();
				hSHPDBF = 0;
			}
			EndDialog(hWndDlg, TRUE);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

BOOL FAR PASCAL SETSQLITEPOINTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
	char	SymName[66], str[260], GSPName[256], cIF[128], UDI[66], SymStuff[256];
	static  char cpWidth[64] = { 0 }, clWidth[64] = { 0 }, cWidth[64], cRot[64] = { 0 }, cColor[64] = { 0 };
	static	char projection[MAX_PATH] = { 0 }, units[64] = { 0 };
	short	i, Choice, rtn;
	LPSTR	pDOT;
	HFILE	Fid;
	MNMXCORD	Bounds;
	static	short	CurProj = -1, CurUnits = -1;
	static	BOOL	IsPGDB, IsFGDB;
	static	BOOL	Opened;
	long	ii;

	short    BRtn;
	if ((BRtn = DIALOGSTYLEMsgProc(hWndDlg, Message, wParam, lParam)))
		return (BRtn);
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		char originalName[MAX_PATH];

		Opened = FALSE;
		GetSLTName(str);
		strcpy(originalName, str);
		SetDlgItemText(hWndDlg, IDC_SHAPEFILE, str);
		_fstrcpy(GSPName, str);
		if ((pDOT = _fstrrchr(GSPName, '.')))
			_fstrcpy(pDOT, ".gsp");
		else
			break;
		SetCurVal(GSPName, IDS_FILEGSP);
		LoadTAGDef();
		if (NumTAGDef)
		{
			LPTAGDEF    lpTAGDef;

			lpTAGDef = (LPTAGDEF)GlobalLock(hTAGDef);
			for (i = 0; i < NumTAGDef; i++, lpTAGDef++)
			{
				SendDlgItemMessage(hWndDlg, IDC_TAPREFIX, CB_ADDSTRING, 0, (LPARAM)lpTAGDef->Prefix);
				SendDlgItemMessage(hWndDlg, IDC_REF_PREFIX, CB_ADDSTRING, 0, (LPARAM)lpTAGDef->Prefix);
			}
			GlobalUnlock(hTAGDef);
		}
		EnableWindow(GetDlgItem(hWndDlg, IDC_SAADD), TRUE);
		SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_ADDSTRING, 0, (LPARAM)"Feet");
		SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_ADDSTRING, 0, (LPARAM)"Meters");
		SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_ADDSTRING, 0, (LPARAM)"Degrees");
		SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_ADDSTRING, 0, (LPARAM)"Degrees * 1000000");
		FillProjectionList(hWndDlg, IDC_PROJECTION, &CurProj, IDC_UNITS, &CurUnits);
		//         _fstrcpy (str,"*.CVT");
		//         DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE); 
		if (PRJ_UNITS[1] == 1)
			SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
		else if (PRJ_UNITS[1] == 2)
			SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_SETCURSEL, (WPARAM)1, (LPARAM)0);
		SendDlgItemMessage(hWndDlg, IDC_PROJECTION, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)"baseproj");
		SendDlgItemMessage(hWndDlg, IDC_INDEXSTANDARD, BM_SETCHECK, SHPIndexType == SHP_INDEX_STANDARD, 0L);
		SendDlgItemMessage(hWndDlg, IDC_INDEXSIMPLE, BM_SETCHECK, SHPIndexType == SHP_INDEX_SIMPLE, 0L);
		SendDlgItemMessage(hWndDlg, IDC_INDEXQUAD, BM_SETCHECK, SHPIndexType == SHP_INDEX_QUAD, 0L);
		goto LoadGSP;
	}
		break; /* End of WM_INITDIALOG                                 */

	case WM_COMMAND:

		switch (LOWORD(wParam))

		{
		case IDC_SHOW_FIELDS:
		{
			if (!hSHPDBF)
			{
				char	DBName[256];
				LPSTR	pTable;

				GetSLTName(str);
				pTable = strrchr(str, '_');
				if (pTable)
				{
					LPSTR pDot = strrchr(str, '.');
					if (pDot)
					{
						*pDot = 0;
						*pTable++ = 0;
						sprintf(DBName, "SLT=%s.slt(%s)", str, pTable);
						OpenDataFile(DBName, "", BT_READ, &hSHPDBF);
					}
				}
				Opened = TRUE;
			}

			DisplayFieldList(hWndDlg, hSHPDBF, 0, 0, 0);
		}
			break;

		case IDC_COPYFROM:
			*GSPName = 0;
			if (!GetFileName3(hWndDlg, GSPName, IDS_FILTERGSP, IDS_FILEGSP))
				break;
		LoadGSP:
			SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_RESETCONTENT, 0, 0);
			SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_RESETCONTENT, 0, 0);
			{
				HFILE	Fid = GSSiOpenFile(GSPName, 0, OF_READ);
				short	SymNum = 0;

				if (Fid == HFILE_ERROR)
				{
					short	type = SHPT_POINT;

					GetDlgItemText(hWndDlg, IDC_SHAPEFILE, str, 256);
					if (!*projection)
					{
						GuessShapeProjection(str, hWndDlg, IDC_PROJECTION, IDC_UNITS, TRUE);
					}
					else
					{
						HFILE	Fid2;

						strcpy(str, projection);
						ExpandText(str);
						Fid2 = GSSiOpenFile(str, 0, OF_READ);
						if (Fid2 != HFILE_ERROR)
						{
							fgetstring(str, 250, Fid2);
							SendDlgItemMessage(hWndDlg, IDC_PROJECTION, CB_SELECTSTRING, -1, (LPARAM)(str + 1));
							GSSiClose2 (&Fid2);
						}
						if (*units)
							SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_SELECTSTRING, -1, (LPARAM)units);
					}
					if (!*cRot)
						strcpy(cRot, "0D");
					if (!cColor)
						strcpy(cColor, "-1");

					switch (SHPType)
					{
					case SHPT_POINT:
					case SHPT_POINTZ:
						GetGlobalCVal("[%DEFAULTPOINTSYMBOL]", SymName, "SQUARE");
						if (!*cpWidth)
							strcpy(cpWidth, "-1");
						type = 1;
						break;
					case SHPT_ARC:
					case SHPT_ARCZ:
					case SHPT_ARCM:
						GetGlobalCVal("[%DEFAULTLINESYMBOL]", SymName, "PEN1");
						if (!*clWidth)
							strcpy(clWidth, "-1");
						type = 2;
						break;
					case 4:
					case SHPT_TEXT:
					case SHPT_POLYGON:
					case SHPT_POLYGONZ:
						GetGlobalCVal("[%DEFAULTAREASYMBOL]", SymName, "PARCEL");
						if (!*cWidth)
							strcpy(cWidth, "-1");
						type = 3;
						break;
					}
					if (*PGDBTable)
					{
						sprintf(SymName, "%s,PAR=NEW", PGDBTable);
						SymNum = GetOrCreateSym(hWndDlg, SymName, 0, 0, GetGlobalLVal2("[%ALLOWSYMBOLCREATION]", 0), type);
						_fstrcpy(SymName, PGDBTable);
					}
					else if (*FGDBTable)
					{
						sprintf(SymName, "%s,PAR=NEW", FGDBTable);
						SymNum = GetOrCreateSym(hWndDlg, SymName, 0, 0, GetGlobalLVal2("[%ALLOWSYMBOLCREATION]", 0), type);
						_fstrcpy(SymName, FGDBTable);
					}
					else
					{
						char	fname[80];

						_splitpath(GSPName, 0, 0, fname, 0);
						sprintf(SymName, "%s,PAR=NEW", fname);
						SymNum = GetOrCreateSym(hWndDlg, SymName, 0, 0, GetGlobalLVal2("[%ALLOWSYMBOLCREATION]", 0), type);
						_fstrcpy(SymName, fname);
					}
					if (!SymNum)
					{
						*cWidth = 0;
						*cRot = 0;
						*cColor = 0;
						rtn = 0;
						switch (SHPType)
						{
						case SHPT_POINT:
						case SHPT_POINTZ:
							rtn = SelectPointSymbol(hWndDlg, 1, SymName, "All", cWidth, cRot, cColor, TRUE);
							break;
						case SHPT_ARC:
						case SHPT_ARCZ:
						case SHPT_ARCM:
							rtn = SelectLineSymbol(hWndDlg, 1, SymName, cWidth, cColor, TRUE);
							break;
						case 4:
						case SHPT_TEXT:
						case SHPT_POLYGON:
						case SHPT_POLYGONZ:
							rtn = SelectAreaSymbol(hWndDlg, 1, SymName, cColor, TRUE);
							break;
						}
					}
					else
						GetDictSymName(SymNum, SymName);
					sprintf(SymStuff, "%s;%s;%s;%s", SymName, cWidth, cRot, cColor);
					SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_ADDSTRING, 0, (LPARAM)SymStuff);
					SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_ADDSTRING, 0, (LPARAM)"ALL ROWS");
					break;
				}
				if (fgetstring(str, 128, Fid))
				{
					HFILE	Fid2;

					ExpandText(str);
					Fid2 = GSSiOpenFile(str, 0, OF_READ);
					if (Fid2 != HFILE_ERROR)
					{
						fgetstring(str, 250, Fid2);
						SendDlgItemMessage(hWndDlg, IDC_PROJECTION, CB_SELECTSTRING, -1, (LPARAM)(str + 1));
						GSSiClose2 (&Fid2);
					}
				}
				if (fgetstring(str, 128, Fid))
					SendDlgItemMessage(hWndDlg, IDC_UNITS, CB_SELECTSTRING, -1, (LPARAM)str);
				if (fgetstring(str, 128, Fid))
					SetDlgItemText(hWndDlg, IDC_STARTNO, str);
				if (fgetstring(str, 128, Fid))
				{
					LPSTR	pSC = _fstrchr(str, ':');

					if (pSC)
					{
						*pSC++ = 0;
						SetDlgItemText(hWndDlg, IDC_UDI, pSC);
					}
					SetDlgItemText(hWndDlg, IDC_TAPREFIX, str);
				}
				if (fgetstring(str, 128, Fid))
					SHPIndexType = atoi(str);
				SendDlgItemMessage(hWndDlg, IDC_INDEXSTANDARD, BM_SETCHECK, SHPIndexType == SHP_INDEX_STANDARD, 0L);
				SendDlgItemMessage(hWndDlg, IDC_INDEXSIMPLE, BM_SETCHECK, SHPIndexType == SHP_INDEX_SIMPLE, 0L);
				SendDlgItemMessage(hWndDlg, IDC_INDEXQUAD, BM_SETCHECK, SHPIndexType == SHP_INDEX_QUAD, 0L);

				while (fgetstring(str, 256, Fid))
				{
					if (*str == '#')
						break;
					DecodeSHPParam(str, SymName, cIF, cColor, cWidth, cRot);
					sprintf(str, "%s;%s;%s;%s", SymName, cWidth, cRot, cColor);
					SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_ADDSTRING, 0, (LPARAM)str);
					if (*cIF)
						SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_ADDSTRING, 0, (LPARAM)cIF);
					else
						SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_ADDSTRING, 0, (LPARAM)"ALL ROWS");
				}
				if (fgetstring(str, 256, Fid))
					SetDlgItemText(hWndDlg, IDC_BEGINDATE, str);
				if (fgetstring(str, 256, Fid))
					SetDlgItemText(hWndDlg, IDC_ENDDATE, str);
				GSSiClose2 (&Fid);
			}

			break;

		case IDC_ASSIGNCVT:
			GetDlgItemText(hWndDlg, IDC_SHAPEFILE, str, 128);
			GuessShapeProjection(str, hWndDlg, IDC_PROJECTION, IDC_UNITS, FALSE);
			break;

		case IDC_SAADD:
		{
						  HANDLE hMem;
						  LPSTR  lpStr, lpWhere;

						  if (!hSHPDBF)
						  {
							  char	DBFName[256];

							  GetSHPName(str);
							  if (IsPGDB)
							  {
								  OpenPGDBFileIndex(str, 0);
								  SetPGDB_SQL("");
							  }
							  else
							  {
								  sprintf(DBFName, "SHP=%s", str);
								  OpenDataFile(DBFName, "", BT_READ, &hSHPDBF);
							  }
							  Opened = TRUE;
						  }
						  *SymName = 0;
						  *cWidth = 0;
						  *cRot = 0;
						  *cColor = 0;
						  rtn = 0;
						  switch (SHPType)
						  {
						  case SHPT_POINT:
						  case SHPT_POINTZ:
							  rtn = SelectPointSymbol(hWndDlg, 1, SymName, "All", cWidth, cRot, cColor, FALSE);
							  break;
						  case SHPT_ARC:
						  case SHPT_ARCZ:
						  case SHPT_ARCM:
							  rtn = SelectLineSymbol(hWndDlg, 1, SymName, cWidth, cColor, FALSE);
							  break;
						  case 4:
						  case SHPT_TEXT:
						  case SHPT_POLYGON:
						  case SHPT_POLYGONZ:
							  rtn = SelectAreaSymbol(hWndDlg, 1, SymName, cColor, FALSE);
							  break;
						  }
						  if (!rtn)
							  break;
						  hMem = GSSiGlobAlloc(1421, GHND, 4096);
						  lpStr = GlobalLock(hMem);
						  if (GetSQLWhereClause(hWndDlg, hSHPDBF, lpStr))
						  {
							  sprintf(str, "%s;%s;%s;%s", SymName, cWidth, cRot, cColor);
							  SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_ADDSTRING, 0, (LPARAM)str);
							  SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_ADDSTRING, 0, (LPARAM)lpStr);
						  }
						  GSSiGlobUlFree(&hMem);
						  break;
		}
		case IDC_SQL_LIST:
		case IDC_SYMBOL_LIST:
		{
								WPARAM	OtherParam;

								switch (HIWORD(wParam))
								{
								case LBN_DBLCLK:
									PostMessage(hWndDlg, WM_COMMAND, IDC_SAMODIFY, 0L);
									break;
								case LBN_SELCHANGE:
									Choice = SendDlgItemMessage(hWndDlg, LOWORD(wParam), LB_GETCURSEL, 0, 0);
									if (LOWORD(wParam) == IDC_SQL_LIST)
										OtherParam = IDC_SYMBOL_LIST;
									else
										OtherParam = IDC_SQL_LIST;
									SendDlgItemMessage(hWndDlg, OtherParam, LB_SETCURSEL, Choice, 0);
									EnableWindow(GetDlgItem(hWndDlg, IDC_SADELETE), TRUE);
									EnableWindow(GetDlgItem(hWndDlg, IDC_SAMODIFY), TRUE);
									break;
								}
		}
			break;
		case IDC_SADELETE:
			Choice = max(SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_GETCURSEL, 0, 0),
				SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_GETCURSEL, 0, 0));
			SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_DELETESTRING, Choice, 0);
			SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_DELETESTRING, Choice, 0);
			break;

		case IDC_SAMODIFY:
			if ((Choice = SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_GETCURSEL, 0, 0)) >= 0)
			{
				SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_GETTEXT, Choice, (DWORD)SymStuff);
				SetSymParms(SymStuff, SymName, cWidth, cRot, cColor);
				rtn = 0;
				switch (SHPType)
				{
				case SHPT_POINT:
				case SHPT_POINTZ:
					rtn = SelectPointSymbol(hWndDlg, 1, SymName, "All", cWidth, cRot, cColor, FALSE);
					break;
				case SHPT_ARC:
				case SHPT_ARCZ:
				case SHPT_ARCM:
					rtn = SelectLineSymbol(hWndDlg, 1, SymName, cWidth, cColor, FALSE);
					break;
				case 4:
				case SHPT_TEXT:
				case SHPT_POLYGON:
				case SHPT_POLYGONZ:
					rtn = SelectAreaSymbol(hWndDlg, 1, SymName, cColor, FALSE);
					break;
				}
				if (!rtn)
					break;
				if (*SymName == '[')
					FillSymbolListFromVariable(hWndDlg, IDC_SYMBOL_LIST, IDC_SQL_LIST, SymName, cWidth, cRot, cColor, hSHPDBF);
				else
				{
					SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_DELETESTRING, Choice, 0);
					sprintf(SymStuff, "%s;%s;%s;%s", SymName, cWidth, cRot, cColor);
					SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_INSERTSTRING, Choice, (LPARAM)SymStuff);
				}
			}
			else
			if ((Choice = SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_GETCURSEL, 0, 0)) >= 0)
			{
				HANDLE	hMem;
				LPSTR	lpStr;

				hMem = GSSiGlobAlloc(1422, GHND, 4096);
				lpStr = GlobalLock(hMem);
				SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_GETTEXT, Choice, (DWORD)lpStr);
				if (!hSHPDBF)
				{
					char	DBFName[256];

					GetSHPName(str);
					if (IsPGDB)
					{
						OpenPGDBFileIndex(str, 0);
						SetPGDB_SQL("");
					}
					else
					{
						sprintf(DBFName, "SHP=%s", str);
						OpenDataFile(DBFName, "", BT_READ, &hSHPDBF);
					}
					Opened = TRUE;
				}
				if (GetSQLWhereClause(hWndDlg, hSHPDBF, lpStr))
				{
					SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_DELETESTRING, Choice, 0);
					SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_INSERTSTRING, Choice, (LPARAM)lpStr);
				}
				GSSiGlobUlFree(&hMem);
			}

			break;

		case IDCANCEL:
			SetGlobalValueBool("%AUTOSHPPARM", FALSE);
			if (Opened)
			{
				if (IsPGDB)
					OpenPGDB(0, 0, 0);
				else if (IsFGDB)
					OpenFGDB(0, 0, 0);
				else
					CloseDataFile(TRUE, &hSHPDBF);
				DestroyFieldList();
				hSHPDBF = 0;
			}
			EndDialog(hWndDlg, FALSE);
			break;

		case IDOK:
			GetSLTName(GSPName);
			Fid = GSSiOpenFile(GSPName, 0, OF_CREATE);
			if (Fid == HFILE_ERROR)
			{
				MessageBox(hWndDlg, GSPName, "Unable to create file", MB_ICONEXCLAMATION);
				break;
			}
			//GetDlgItemText (hWndDlg,IDC_PROJECTION,str,sizeof(str)-1);  
			GetProjectionFile(hWndDlg, IDC_PROJECTION, str);
			SubstituteDL(str, TRUE);
			//ExpandText (str);
			strcpy(projection, str);
			fputstring(str, Fid);
			GetDlgItemText(hWndDlg, IDC_UNITS, str, sizeof(str)-1);
			strcpy(units, str);
			fputstring(str, Fid);
			GetDlgItemText(hWndDlg, IDC_STARTNO, str, sizeof(str)-1);
			fputstring(str, Fid);
			if (GetDlgItemText(hWndDlg, IDC_TAPREFIX, str, sizeof(str)-1))
			{
				_fstrcat(str, ":");
				GetDlgItemText(hWndDlg, IDC_UDI, _fstrchr(str, 0), sizeof(str)-_fstrlen(str) - 1);
			}
			fputstring(str, Fid);
			SHPIndexType = SHP_INDEX_STANDARD;
			if (SendDlgItemMessage(hWndDlg, IDC_INDEXSIMPLE, BM_GETCHECK, 0, 0))
				SHPIndexType = SHP_INDEX_SIMPLE;
			else if (SendDlgItemMessage(hWndDlg, IDC_INDEXQUAD, BM_GETCHECK, 0, 0))
				SHPIndexType = SHP_INDEX_QUAD;
			itoa(SHPIndexType, str, 10);
			fputstring(str, Fid);
			i = 0;
			while (SendDlgItemMessage(hWndDlg, IDC_SYMBOL_LIST, LB_GETTEXT, i, (LPARAM)str) != LB_ERR)
			{
				SetSymParms(str, SymName, cWidth, cRot, cColor);
				SendDlgItemMessage(hWndDlg, IDC_SQL_LIST, LB_GETTEXT, i++, (LPARAM)cIF);
				if (!_fstrcmp(cIF, "ALL ROWS"))
					*cIF = 0;
				sprintf(str, "%s;%s;%s;%s;%s", SymName, cIF, cColor, cWidth, cRot);
				fputstring(str, Fid);
			}
			fputstring("#ENDOFLIST", Fid);
			GetDlgItemText(hWndDlg, IDC_BEGINDATE, str, sizeof(str)-1);
			fputstring(str, Fid);
			GetDlgItemText(hWndDlg, IDC_ENDDATE, str, sizeof(str)-1);
			fputstring(str, Fid);
			GSSiClose2 (&Fid);
			if (Opened)
			{
				if (IsPGDB)
					OpenPGDB(0, 0, 0);
				else if (IsFGDB)
					OpenFGDB(0, 0, 0);
				else
					CloseDataFile(TRUE, &hSHPDBF);
				DestroyFieldList();
				hSHPDBF = 0;
			}
			EndDialog(hWndDlg, TRUE);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

BOOL FAR PASCAL CONFIGLISTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
    RECT    rect,rect2;    
    short ii, w, h,l, Choice;
    HDC     hDC;   
    HWND	hWnd;
    PAINTSTRUCT ps;                /* holds PAINT information             */ 
    char	str[256], App[64];  
    DWORD	hDir, Type;   
    LPSTR	pBS;
	static	HANDLE	hSaveBM=0;

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message) 
 {
    case WM_INITDIALOG: 
    { 
    	hSaveBM = EnterBlockingWindow (hWndDlg);    
//		GetWindowRect(hWndMain, &rect);
//    	SizeDlgToRect (hWndDlg,&rect);

/*		GetWindowRect(GetDesktopWindow(), &rect);
		WVer = GetVersion ();
		WinVer = HIBYTE(LOWORD(WVer));
		if (WinVer <95) 
		 	SetWindowPos(hWndDlg, (HWND) 0, 0, 0,rect.right, rect.bottom,SWP_NOZORDER);
		else
			SetWindowPos(hWndDlg, (HWND) HWND_TOPMOST, 0,0,rect.right, rect.bottom,SWP_NOREDRAW); 
		hWnd = GetDlgItem(hWndDlg,IDC_SKIPSCHMOOZE);
		GetWindowRect(hWnd, &rect2);  
		w = rect2.right - rect2.left;
		h = rect2.bottom - rect2.top;  
		rect2 = rect;
		rect2.right -= 2;
		rect2.bottom -= 2;
		rect2.left = rect2.right - w;
		rect2.top = rect2.bottom - h;
		SetWindowPos(hWnd, (HWND) 0, rect2.left,rect2.top,w,h,SWP_NOZORDER);
		hWnd = GetDlgItem(hWndDlg,IDOK);
		GetWindowRect(hWnd, &rect2);  
		w = rect2.right - rect2.left;
		h = rect2.bottom - rect2.top;  
		rect2 = rect;
		rect2.left+=2;
		rect2.right = rect2.left + w;   
		rect2.bottom -= 2;
		rect2.top = rect2.bottom - h;
		SetWindowPos(hWnd, (HWND) 0, rect2.left,rect2.top,w,h,SWP_NOZORDER); */  
       	GetGlobalCVal ("[%CFGDIR1ID]",str,"[%DL]");
		ExpandText (str);     
       	if (*LastChr (str) == '\\')
       		*LastChr (str) = 0;
		SetDlgItemText (hWndDlg,IDC_CFGDIR1,str); 
       	GetGlobalCVal ("[%CFGDIR2ID]",str,"My Documents");
		ExpandText (str);     
       	if (*LastChr (str) == '\\')
       		*LastChr (str) = 0;
		SetDlgItemText (hWndDlg,IDC_CFGDIR2,str); 
       	GetGlobalCVal ("[%CFGDIR3ID]",str,"Desktop");
		ExpandText (str);     
       	if (*LastChr (str) == '\\')
       		*LastChr (str) = 0;
		SetDlgItemText (hWndDlg,IDC_CFGDIR3,str); 

		GetCurVal (str,sizeof(str),IDS_FILEGMC); 
		ExpandText (str);     
		SetDlgItemText (hWndDlg,IDC_CURRENTCFGDIR,str); 
    case GSSI_REINITDIALOG:  
		SendDlgItemMessage (hWndDlg,IDC_CFGLIST,CB_RESETCONTENT,0,0);
		GetDlgItemText (hWndDlg,IDC_CURRENTCFGDIR,str,255); 
       	if (*LastChr (str) == '\\')
       		*LastChr (str) = 0;
		l =_fstrlen (str);
		if (l > 4 && !_fstricmp (&str[l-4],".gmc"))
		{   
			LPSTR	pBS;
			
			*&str[l-4] = 0; 
			pBS = _fstrrchr (str,'\\');
			if (pBS)
				*pBS = 0;
		}
		SetDlgItemText (hWndDlg,IDC_CURRENTCFGDIR,str); 
 		_fstrcat (str,"\\*.*");
		hDir = SearchDirectory32 (str,0,&Type,0);
		while (hDir)
		{   
			if (Type && *str != '.')
				SendDlgItemMessage (hWndDlg,IDC_CFGLIST,CB_ADDSTRING,0,(LPARAM)str);
			hDir = SearchDirectory32 (str,hDir,&Type,0);
		}
		GetDlgItemText (hWndDlg,IDC_CURRENTCFGDIR,str,255); 
		_fstrcat (str,"\\*.gmc");
		hDir = SearchDirectory32 (str,0,&Type,0);
		while (hDir)
		{   
			if (!Type)
				SendDlgItemMessage (hWndDlg,IDC_CFGLIST,CB_ADDSTRING,0,(LPARAM)str);
			hDir = SearchDirectory32 (str,hDir,&Type,0);
		}
    } 
    	break;   
/*    case WM_PAINT:
         _fmemset(&ps, 0x00, sizeof(PAINTSTRUCT));
         hDC = BeginPaint(hWndDlg, &ps);
         DisplayBMFileInRect (hDC,"startup.bmp",ps.rcPaint,FALSE);
         EndPaint(hWndDlg, &ps);
         break;       */  

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

		 switch(LOWORD (wParam))

           {
            case IDCANCEL:
            case IDOK: 

				GetDlgItemText (hWndDlg,IDC_CURRENTCFGDIR,str,255);
				_fstrcat (str,"\\"); 
				GetDlgItemText(hWndDlg,IDC_CFGLIST,_fstrchr (str,0),40);  
				if (FileType (str) == 1) 
				{
					UnallocateConfig ();  
                	_fstrcpy (CfgName,str);   
                }
                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
                break;
            case IDC_CFGDIR1: 
            	GetGlobalCVal ("[%CFGDIR1]",str,"[%DL]");
				ExpandText (str);
				GetSpecialDirectory (str);     
				SetDlgItemText (hWndDlg,IDC_CURRENTCFGDIR,str); 
   	 			PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
            	break;
            case IDC_CFGDIR2:
            	GetGlobalCVal ("[%CFGDIR2]",str,"My Documents");
				ExpandText (str);
				GetSpecialDirectory (str);     
				SetDlgItemText (hWndDlg,IDC_CURRENTCFGDIR,str); 
   	 			PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
            	break;
            case IDC_CFGDIR3:
            	GetGlobalCVal ("[%CFGDIR3]",str,"Desktop");
				ExpandText (str);
				GetSpecialDirectory (str);     
				SetDlgItemText (hWndDlg,IDC_CURRENTCFGDIR,str); 
   	 			PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
            	break;
            case IDC_UP:
				GetDlgItemText (hWndDlg,IDC_CURRENTCFGDIR,str,255);
				if ((pBS = _fstrrchr (str,'\\')))
					*pBS = 0;
				SetDlgItemText (hWndDlg,IDC_CURRENTCFGDIR,str); 
             	PostMessage(hWndDlg, GSSI_REINITDIALOG, 0,0L);
            	break;  
        	case IDC_CFGLIST:
			switch(HIWORD(wParam))
            {
              	case CBN_DBLCLK:   
					Choice=SendDlgItemMessage(hWndDlg,LOWORD(wParam),CB_GETCURSEL,0,0); 
					GetDlgItemText (hWndDlg,IDC_CURRENTCFGDIR,str,255);
					_fstrcat (str,"\\"); 
					SendDlgItemMessage(hWndDlg,LOWORD(wParam),CB_GETLBTEXT,Choice,(DWORD)_fstrchr (str,0));  
					if (FileType (str) == 2)
					{
						SetDlgItemText (hWndDlg,IDC_CURRENTCFGDIR,str); 
	                 	PostMessage(hWndDlg, GSSI_REINITDIALOG, 0,0L);  
	                }
	                else
			        	PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
	                break;
                case CBN_SELCHANGE:
					Choice=SendDlgItemMessage(hWndDlg,LOWORD(wParam),CB_GETCURSEL,0,0); 
					GetDlgItemText (hWndDlg,IDC_CURRENTCFGDIR,str,255);
					_fstrcat (str,"\\"); 
					SendDlgItemMessage(hWndDlg,LOWORD(wParam),CB_GETLBTEXT,Choice,(DWORD)_fstrchr (str,0));  
					if (FileType (str) == 1) 
					{   
						RECT	ImageRect, TextRect;
						HDC		hDC;

						GetWindowRect (GetDlgItem (hWndDlg,IDC_CFGDISPLAY),&ImageRect); 
						ScreenRectToClientRect (hWndDlg,&ImageRect);
    					GetWindowRect (GetDlgItem (hWndDlg,IDC_CFGDESC),&TextRect);
						ScreenRectToClientRect (hWndDlg,&TextRect);
						InflateRect (&TextRect,-4,-4);
						hDC = GetDC (hWndDlg);
						DisplayConfigPreview (hWndDlg,hDC,str,&ImageRect,IDC_CFGDESC);
						ReleaseDC (hWndDlg,hDC);
					}
					break;
            }
              break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

int AddFileToSendList(HWND hWndDlg,UINT idc_XFERFILELISTS, LPSTR pFile)
{
	int n = 0;
	char File[4096+2];

	if (FileType(pFile) == 1)
	{
		SubstituteDL(pFile, FALSE);
		SendDlgItemMessage(hWndDlg, idc_XFERFILELISTS, LB_ADDSTRING, 0, (LPARAM)pFile);
		n++;
	}
	else if (FileType(pFile) == 2)
	{
		char TempName[MAX_PATH];
		HFILE Fid;

		GSSiGetTempFileName(0, "gm", 0, TempName);
		n += GetFileList(TempName, TRUE, pFile, "*.*", TRUE, FALSE, FALSE);
		Fid = GSSiOpenFile(TempName, 0, OF_READ);
		fgetstring(File,4096, Fid);
		while (fgetstring(File, 4096, Fid))
		{
			LPSTR pTab = strchr(File, '\t');
			if (pTab)
				*pTab = 0;
			SubstituteDL(File, FALSE);
			SendDlgItemMessage(hWndDlg, idc_XFERFILELISTS, LB_ADDSTRING, 0, (LPARAM)File);
		}
		GSSiClose(Fid);
	}
	return n;
}

BOOL FAR PASCAL BUILDXFERFILEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
	char	str[256];   
	int		ii,n=0; 
	HANDLE	hFile;
	LPSTR	pFile; 
	char	File[MAX_PATH+2];    
	HCURSOR	hcurSave;
	static	HANDLE	hSaveBM=0;
		
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
	     HaltMapDisplay(TRUE,TRUE); 
		 DisableUndo (TRUE);
	     CloseSymDict();  
		 CloseAllRequestedFiles(FALSE);
    	 hSaveBM = EnterBlockingWindow (hWndDlg);
       	 cwCenter(hWndDlg, 0); 
       	 SetDlgItemText (hWndDlg,IDC_XFERFILENAME,TransferFileName);
       	 if (!_fstricmp (BuildTransferFileOption,"RUN"))
       	 {
			 HFILE	FidTF=GSSiOpenFile (TransferFileName,0,OF_READ);
			 long	NextFileLoc=0, loc, len,FileLength, EndOfFile; 

			 if (FidTF == HFILE_ERROR)
			 {   
			 	 sprintf (str,"Unable to open transfer file\r%s",TransferFileName);
		         MessageBox (hWndDlg,str,0,MB_ICONEXCLAMATION);
		         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
		         break;
		     }
			 FileLength = GSSifilelength (FidTF);
		     BigRead (FidTF,(HPSTR)&loc,4);
		     if (loc != FileLength)
		     {
FileIsInvalid:
			 	 sprintf (str,"Invalid transfer file\r%s",TransferFileName);
		         MessageBox (hWndDlg,str,0,MB_ICONEXCLAMATION);
		         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
		         break;
		     } 
		     GSSillseek (FidTF,loc-4,0);
		     BigRead (FidTF,(HPSTR)&loc,4);
		     if (loc != 32349)
				goto FileIsInvalid;  
			 NextFileLoc = 10;
			 hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
			 while (NextFileLoc > 0)
			 {
			 	GSSillseek (FidTF,NextFileLoc,0); 
		     	BigRead (FidTF,(HPSTR)&len,4);
		     	BigRead (FidTF,(HPSTR)File,len);
		     	BigRead (FidTF,(HPSTR)&NextFileLoc,4);
		     	if (NextFileLoc > 0)
		     		EndOfFile = NextFileLoc - 1;
		     	else
		     		EndOfFile = FileLength - 4;
				SendDlgItemMessage (hWndDlg,IDC_XFERFILELISTS,LB_ADDSTRING,0,(LPARAM)File);
	    	 	n++;
	    	 }
	    	 GSSiClose2 (&FidTF); 
			 GSSiSetCursor(hcurSave);
	         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
	     } 
       	 else if (!*TransferFrom)
	       	 DragAcceptFiles (hWndDlg,TRUE);   
		 else
		 {
			 n += AddFileToSendList(hWndDlg, IDC_XFERFILELISTS, TransferFrom);
			 PostMessage(hWndDlg, WM_COMMAND, IDC_LOADCOMPLETEMESSAGE, 0L);
			 PostMessage(hWndDlg, WM_COMMAND, IDC_ADDXFERCMD, 0L);
			 PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
		 }
         break; /* End of WM_INITDIALOG                                 */
    

    case WM_DROPFILES:
		 hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
    	 hFile = (HANDLE)wParam;   
    	 pFile = File;
		 while (DragQueryFile(hFile, n, pFile, MAX_PATH))
		 {
			 n += AddFileToSendList(hWndDlg, IDC_XFERFILELISTS, pFile);
		 }
		 GSSiSetCursor(hcurSave);
    	 DragFinish (hFile);
    	 break;
    	 
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            
            case IDC_GETFILESFROMSITE: 
            	SetDlgItemText (hWndDlg,IDC_XFERCOMMAND,"$EXECUTE($SHORTPATHNAME([%DL]ftpto.bat))"); 
            	break;
            case IDC_LOADCOMPLETEMESSAGE:
            	SetDlgItemText (hWndDlg,IDC_XFERCOMMAND,"$MESSAGE(Load Complete)"); 
            	break;
            
            case IDC_ADDXFERCMD:  
            	_fstrcpy (str,"[XCMD]");
            	GetDlgItemText (hWndDlg,IDC_XFERCOMMAND,_fstrchr(str,0),250);
				SendDlgItemMessage (hWndDlg,IDC_XFERFILELISTS,LB_ADDSTRING,0,(LPARAM)str); 
				break;
            case IDOK: 
            {   
            	
            	if (!*TransferFileName)
            	{
            		MessageBox (hWndDlg,"Transfer file name not set",0,MB_ICONEXCLAMATION);
            		break;
            	}
            	{
					HANDLE	FidTF=OpenFileGM (TransferFileName,0,OF_CREATE); 
					short	choice = 0,Version=2; 
					LONGLONG	NextFileLoc = 0, loc;
					long marker, len;
					long	TotLen = SendDlgItemMessage(hWndDlg,IDC_XFERFILELISTS,LB_GETCOUNT,0,0); 
					long	MaxLength=8L*(long)USHRT_MAX;
					
           			BigWrite64 (FidTF,(HPSTR)&loc,8,-1);
           			BigWrite64 (FidTF,(HPSTR)&Version,2,-1);
           			BigWrite64 (FidTF,(HPSTR)&MaxLength,4,-1); 
           			Processing = TRUE;
           			*TransferFileRunCommand = 0;
	            	while (ContinueProcessing && SendDlgItemMessage(hWndDlg,IDC_XFERFILELISTS,LB_GETTEXT,choice++,(DWORD)str) != LB_ERR)
	            	{ 
				    	if (_fstrnicmp (str,"[XCMD]",6) || _fstricmp (BuildTransferFileOption,"RUN"))
				    	{
		            		SetDlgItemText (hWndDlg,IDC_MESSAGE,str);  
		            		if (NextFileLoc)
		            		{
		            			loc = GSSillseek64 (FidTF,0,1);
		            			GSSillseek64 (FidTF,NextFileLoc,0);
		            			BigWrite64 (FidTF,(HPSTR)&loc,8,-1);
		            			GSSillseek64 (FidTF,loc,0);  
		            		}
		        			len = _fstrlen (str) + 1;    
		        			BigWrite64 (FidTF,(HPSTR)&len,4,-1);
		        			BigWrite64 (FidTF,(HPSTR)str,len,-1);
		        			NextFileLoc = GSSillseek64 (FidTF,0,1);  
		        			loc = -1;
		        			BigWrite64 (FidTF,(HPSTR)&loc,8,-1);
		        		}
				    	PctBox (GetDlgItem(hWndDlg,IDC_STATUS),TotLen,choice,-1);
				    	if (_fstrnicmp (str,"[XCMD]",6) &&
				    	    !SendDlgItemMessage(hWndDlg,IDC_BUILDNAMESONLY,BM_GETCHECK,0,0)) 
			    			AddFileToTransferFile(GetDlgItem(hWndDlg,IDC_STATUS2),FidTF,str,MaxLength,0);
			    		else if (!_fstricmp (BuildTransferFileOption,"RUN"))
			    			_fstrcpy (TransferFileRunCommand,&str[6]); 
				    }
				    Processing = FALSE;
				    marker = 80251;
        			BigWrite64 (FidTF,(HPSTR)&marker,4,-1);
           			loc = GSSillseek64 (FidTF,0,1);
           			GSSillseek64 (FidTF,0,0);
           			BigWrite64 (FidTF,(HPSTR)&loc,8,-1);
				    GSSiClose64 (&FidTF); 
				    if (!ContinueProcessing)
				    {
				    	SetContinueProcessing ( TRUE); 
				    	SetDlgItemText (hWndDlg,IDC_MESSAGE,"Build cancelled by user");
				    	break;
				    }
				}
                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
            }
            	break;
            	 
            case IDCANCEL:  
            	if (Processing)
            		SetContinueProcessing (FALSE);
            	else
	                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
                break; 
                 
           }
    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL FAR PASCAL LOADXFERFILEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
	char	str[512],Ext[6]=".bin";   
	int		ii,n=0; 
	HANDLE	hFile;
	LPSTR	pFile, pUI; 
	char	File[256];
	HANDLE	FidTF;
   	HFILE	Fid; 
   	static	HANDLE	hSaveBM=0;
	static	int		UpdateID = 0;
		
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
    {
		 LONGLONG	NextFileLoc=0, loc, FileLength, EndOfFile;
		 int len, marker;
		 int lenlen = 4;
		  
	     UpdateID = 0;
		 HaltMapDisplay(TRUE,TRUE); 
		 DisableUndo (TRUE);
	     CloseSymDict();  
		 CloseAllRequestedFiles(FALSE);
    	 hSaveBM = EnterBlockingWindow (hWndDlg);
       	 cwCenter(hWndDlg, 0);  
       	 if (!*TransferFileName)
       	 	break;
    case GSSI_REINITDIALOG:  
		 FidTF=OpenFileGM (TransferFileName,0,OF_READ);
		 if (FidTF == INVALID_HANDLE_VALUE)
		 {   
		 	 sprintf (str,"Unable to open transfer file\r%s",TransferFileName);
	         MessageBox (hWndDlg,str,0,MB_ICONEXCLAMATION);
	         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
	         break;
	     }
		 if ((pUI = strrchr (TransferFileName,'_')))
			 UpdateID = atoi (++pUI);
		 FileLength = GSSifilelength64 (FidTF);
		 GSSillseek64(FidTF, FileLength - 4, 0);
		 BigRead64(FidTF, (HPSTR)&marker, 4);
		 if (marker == 80251)
			 lenlen = 8;
		 else if (marker != 32349)
			 goto FileIsInvalid;
		 NextFileLoc = 6 + lenlen;
		 GSSillseek64(FidTF, 0, 0);
		 if (lenlen == 4)
		 {
			 BigRead64(FidTF, (HPSTR)&len, 4);
			 loc = len;
		 }
		 else
			BigRead64(FidTF, (HPSTR)&loc, sizeof(LONGLONG));
		 if (loc != FileLength)
	     {
FileIsInvalid:
		 	 sprintf (str,"Invalid transfer file\r%s",TransferFileName);
	         MessageBox (hWndDlg,str,0,MB_ICONEXCLAMATION);
	         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
	         break;
	     } 
		 while (NextFileLoc > 0)
		 {
			int iPos;
		 	GSSillseek64 (FidTF,NextFileLoc,0); 
	     	BigRead64 (FidTF,(HPSTR)&len,4);
	     	BigRead64 (FidTF,(HPSTR)File,len);
	     	BigRead64 (FidTF,(HPSTR)&NextFileLoc,8);
	     	if (NextFileLoc > 0)
	     		EndOfFile = NextFileLoc - 1;
	     	else
	     		EndOfFile = FileLength - 4;
			iPos = SendDlgItemMessage (hWndDlg,IDC_XFERFILELISTS,LB_ADDSTRING,0,(LPARAM)File);
    	 	n++;
    	 }
    	 GSSiClose64 (&FidTF);  
    	 if (Message == WM_INITDIALOG && !_fstricmp(BuildTransferFileOption, "LOAD"))
         	PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
    	 
    }
    	 break;
    	 
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDC_SETXFERFILE:
				if (GetFileName2 (hWndDlg,TransferFileName,Ext,0))
					PostMessage (hWndDlg,GSSI_REINITDIALOG,0,0);
            	break;
            	
            case IDOK: 
            {   
				 LONGLONG	NextFileLoc = 0, loc, FileLength, length8, EndOfFile, LenToRead;
				 int MaxLength;
				 short	Version;
				 int	len;
				 int    nSelected = SendDlgItemMessage(hWndDlg, IDC_XFERFILELISTS, LB_GETCURSEL, 0, 0);
				 char	SelectedFile[MAX_PATH];

				 if (nSelected >= 0)
					 SendDlgItemMessage(hWndDlg, IDC_XFERFILELISTS, LB_GETTEXT, nSelected, (DWORD)SelectedFile);


				 FidTF=OpenFileGM (TransferFileName,0,OF_READ);
				 FileLength = GSSifilelength64 (FidTF);
				 NextFileLoc = 14;
			     BigRead64 (FidTF,(HPSTR)&length8,8);
			     BigRead64 (FidTF,(HPSTR)&Version,2);
			     BigRead64 (FidTF,(HPSTR)&MaxLength,4); 
			     Processing = TRUE;
				 while (ContinueProcessing && NextFileLoc > 0)
				 {
				 	GSSillseek64 (FidTF,NextFileLoc,0); 
			     	BigRead64 (FidTF,(HPSTR)&len,4);
			     	BigRead64 (FidTF,(HPSTR)File,len);
			     	BigRead64 (FidTF,(HPSTR)&NextFileLoc,8);
					if (nSelected>= 0  && stricmp(File, SelectedFile))
						continue;
			     	if (NextFileLoc > 0)
			     		EndOfFile = NextFileLoc - 1;
			     	else
			     		EndOfFile = FileLength - 4; 
			     	LenToRead = EndOfFile - GSSillseek64 (FidTF,0,1) +1;
					SetDlgItemText (hWndDlg,IDC_MESSAGE,File);
					if (!_fstrnicmp (File,"[XCMD]",6))
						ExpandText (&File[6]);
					else if (!GetFileFromTransferFile (GetDlgItem (hWndDlg,IDC_STATUS2),FidTF,File,LenToRead,MaxLength))
					{   
						sprintf (str,"Unable to open file\r\n%s",File);
						MessageBox (hWndDlg,str,0,MB_ICONEXCLAMATION);
					}
					else if (UpdateID)
					{
						sprintf (str,"%i\t%s",UpdateID,File);
						AppendFile ("[%%DL]updates\\updatefiles.txt",str);
					}
			    	PctBox (GetDlgItem(hWndDlg,IDC_STATUS),FileLength,GSSillseek64 (FidTF,0,1),0); 
		    	 }  
		    	 Processing = FALSE;
		    	 GSSiClose64 (&FidTF);
			     if (!ContinueProcessing)
			     {
			    	SetContinueProcessing ( TRUE); 
			    	SetDlgItemText (hWndDlg,IDC_MESSAGE,"Load cancelled by user");
			    	break;
			     }
		    	 
                 GSSiEndDialog(hWndDlg, TRUE,hSaveBM); 
            }
            	break;
            	 
            case IDCANCEL:  
            	if (Processing)
            		SetContinueProcessing (FALSE);
            	else
	                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
                break; 
                 
           }
    default:
        return FALSE;
   }
 return TRUE;
} 


BOOL FAR PASCAL LOADMDMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{   
    char    str[512], FromDir[256], ToDir[256], ErrMess[256];
    BOOL    isDir;
    int     nchar, item;
    long    Num;
    HANDLE  Handle, hlpFI=0;
    LPINT   lpItems;
    LPFILEINDEX lpFI;
    OFSTRUCTGM    OFStruct;
    char    Index[512];
    static  char    ctype[64];
    static  short     itype;
    HANDLE  hCheckBox;
    HCURSOR hcurSave;  
    short	ii, i, TileSize; 
    UINT	TileSizeControls[4]={IDC_TSIZE0,IDC_TSIZE1,IDC_TSIZE2,IDC_TSIZE3};
	static		HANDLE	hSaveBM=0;    
	static	BOOL	AutoSelect=TRUE, AutoFileList;
    
 short    BRtn;
 //if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:   
    	 AutoFileList = FALSE;
         hSaveBM = EnterBlockingWindow (hWndDlg);
		 SendDlgItemMessage (hWndDlg,IDC_AUTOSELECT,BM_SETCHECK,AutoSelect,0L);
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");   
       	 GetGlobalCVal ("[%MAKEORTHOUNITS]",str,"METERS");
         if (*str == 'M' || *str == 'm')
	     	SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_SETCURSEL,(WPARAM)(1),0);
	     else 
	     	SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_SETCURSEL,(WPARAM)(0),0);
    	 GetGlobalCVal ("[%MAKEORTHOLEVELS]",str,"1,2,4,8,16,32,64,128,256");
    	 SetDlgItemText (hWndDlg,IDC_ORTHLEVELS,str);
    	 GetGlobalCVal ("[%MAKEORTHOQUALITY]",str,"80");
    	 SetDlgItemText (hWndDlg,IDC_IMAGE_QUALITY,str); 
    	 GetGlobalCVal ("[%MAKEORTHODESTDIR]",str,0);
    	 SetDlgItemText(hWndDlg,IDC_DESTDIR,str);
    	 TileSize = GetGlobalLVal2 ("[%MAKEORTHOTILESIZE]",4);
		 SendDlgItemMessage (hWndDlg,IDC_USEAVI,BM_SETCHECK,TRUE,0L);
		 if (GetGlobalBVal2 ("[%USEFREEIMAGE]",TRUE)) 
			 SendDlgItemMessage (hWndDlg,IDC_USEFREEIMAGE,BM_SETCHECK,TRUE,0L);
		 for (i=0;i<4;i++)
         	SendDlgItemMessage (hWndDlg,TileSizeControls[i],BM_SETCHECK,FALSE,0L);
         SendDlgItemMessage (hWndDlg,TileSizeControls[TileSize-1],BM_SETCHECK,TRUE,0L);
		 SendDlgItemMessage (hWndDlg,IDC_TXT,BM_SETCHECK,TRUE,0L);
         if (*AutoExportName)   
		 {   
		 	 short	l=_fstrlen (AutoExportName);
		 	 
		 	 if (!_fstricmp (&AutoExportName[l-12],"filelist.txt"))
		 	 { 
				HFILE	FidAutoFileList;
				AutoFileList = TRUE;
				itype = 1;
				FidAutoFileList = GSSiOpenFile(AutoExportName, 0, OF_READ);
				if (FidAutoFileList != HFILE_ERROR)
				{
					LPSTR pDot;
					LPSTR pTab;
					fgetstring(str, MAX_PATH, FidAutoFileList);
					if (!strnicmp(str, "FULLNAME\t",9))
						fgetstring(str, MAX_PATH, FidAutoFileList);
					GSSiClose2 (&FidAutoFileList);
					if ((pTab = strchr(str, '\t')))
						*pTab = 0;
					pDot = strrchr(str, '.');
					if (pDot)
					{
						pDot++;
						if (!stricmp(pDot, "BPW"))
						{
							PostMessage(hWndDlg, WM_COMMAND, LMD_TYPE_ORTHO, 0L);
							PostMessage(hWndDlg, WM_COMMAND, IDC_BPW, 0L);
						}
						else if (!stricmp(pDot, "TFW"))
						{
							PostMessage(hWndDlg, WM_COMMAND, LMD_TYPE_ORTHO, 0L);
							PostMessage(hWndDlg, WM_COMMAND, IDC_TFW, 0L);
						}
					}
				}
		 	 }
		 	 else
		 	 {
			 	 if (_fstricmp (&AutoExportName[l-3],"txt") &&
			 	 	 _fstricmp (&AutoExportName[l-3],"tfw") &&
			 	 	 _fstricmp (&AutoExportName[l-3],"bpw"))
			 	 {
				 	SendDlgItemMessage (hWndDlg,IDC_BITMAPCOORD,BM_SETCHECK,TRUE,0L);
				 	SendDlgItemMessage (hWndDlg,IDC_USEFREEIMAGE,BM_SETCHECK,TRUE,0L);
				 }
		         PostMessage(hWndDlg, WM_COMMAND, LMD_TYPE_ORTHO, 0L);   
	         }
	         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
	         break;
	     }
	     else 
	     {
	         itype = 1;
	         _fstrcpy (ctype,"*.plt");
	         SendDlgItemMessage (hWndDlg,LMD_TYPE_MAP,BM_SETCHECK,TRUE,0L); 
	     }
	     _fstrcpy (str,"[%DL]");
	     ExpandText (str); 
	     if (*LastChr (str) == '\\')
	     	*LastChr (str) = 0; 
	     _fstrlwr (str);
	     SetDlgItemText (hWndDlg,LMD_DIR,str);
SetType:
		 GetDlgItemText (hWndDlg,LMD_DIR,str,sizeof(str));
         sprintf (strchr(str,0),"\\%s",ctype);

         DlgDirList (hWndDlg,str,LMD_NEWFILES,LMD_DIR,DDL_DIRECTORY|DDL_DRIVES/*|DDL_EXCLUSIVE*/);
SelectFiles:
         if (AutoSelect && (itype ==1 || itype == 2 || itype == 4|| itype == 5 || itype == 6))
         {
         	item=0;
         	while (SendDlgItemMessage (hWndDlg,LMD_NEWFILES,LB_GETTEXT,item,(LPARAM)str) != LB_ERR)
         	{   
         		_fstrlwr (str);
             	if (_fstrstr (str,&ctype[1]))
             	{
             		SendDlgItemMessage (hWndDlg,LMD_NEWFILES,LB_SETSEL,TRUE,item);
             	} 
             	item++;
            }
         }
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

		 switch(LOWORD (wParam))

           {
            case LMD_NEWFILES: /* List Box                                 */
              {
#if WIN32
				switch(HIWORD(wParam))
#else
                switch(HIWORD(wParam))
#endif
                    {
                    // case LBN_DBLCLK:
                     case LBN_SELCHANGE:
                          isDir = DlgDirSelectEx(hWndDlg,str,sizeof(str),LMD_NEWFILES);
                          if (isDir)
                             {nchar = _fstrlen (str);
                              if (!nchar) break;
                              if (str[nchar-1] == '\\')
                                  _fstrcat (str,ctype);
                              if (str[nchar-1] == ':') 
                              {
                              	  _fstrcat (str,"\\");
                           	      SetDlgItemText (hWndDlg,LMD_DIR,str);
                                  _fstrcat (str,ctype); 
                              }
                              ii=DlgDirList (hWndDlg,str,
                                  LMD_NEWFILES,LMD_DIR,DDL_DIRECTORY|DDL_DRIVES);
                              goto SelectFiles;
                              }
                          else
                          	ii=0;
                          break;
                     }

              }
              break;
            
   	       
			case LMD_TYPE_DGN:
               	_fstrcpy (ctype,"*.dgn");
                goto SetType;
             break;   
			case LMD_TYPE_DGNCOM:
               	_fstrcpy (ctype,"*.com");
                goto SetType;
             break;   
			case LMD_TYPE_SHP:
               	_fstrcpy (ctype,"*.shp");
                goto SetType;
             break;   
			case LMD_TYPE_ORA:
               	_fstrcpy (ctype,"*.ora");
                goto SetType;
             break;   
			case LMD_TYPE_VIRTUALPLOT:
               	_fstrcpy (ctype,"*.tif");
               	itype = 4;
                goto SetType;
             break;  
            case LMD_TYPE_GEOTIFF:  
                 itype = 5;
               	_fstrcpy (ctype,"*.tif");   
               	goto S100;
			case LMD_TYPE_SID:
                 itype = 6;  
               	_fstrcpy (ctype,"*.sid"); 
               	 SetDlgItemText (hWndDlg,IDC_ORTHLEVELS,"");
       S100:
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHSTATUS),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_CFTITLE),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_USEAVI),SW_HIDE);
                 //ShowWindow (GetDlgItem(hWndDlg,IDC_EXCMP),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_USEFREEIMAGE),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHLEVELS),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHLEVELST),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_IMAGE_QUALITY),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_IMAGE_QUALITY_LABEL),SW_HIDE); 
                 ShowWindow (GetDlgItem(hWndDlg,IDC_DESTDIR),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_DESTDIR_LABEL),SW_HIDE); 
                 ShowWindow (GetDlgItem(hWndDlg,IDC_CFTYPE2),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_TXT),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_TFW),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_BPW),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_BITMAPCOORD),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_GEOTIFF),SW_HIDE);
				 if (itype == 6)
				 {
					ShowWindow (GetDlgItem(hWndDlg,IDC_UNITS),SW_HIDE);
					ShowWindow (GetDlgItem(hWndDlg,IDC_UNITSHEADER),SW_HIDE); 
				 }
				 else
				 {
					ShowWindow (GetDlgItem(hWndDlg,IDC_UNITS),SW_SHOW);
					ShowWindow (GetDlgItem(hWndDlg,IDC_UNITSHEADER),SW_SHOW); 
				 }
                goto SetType;
             break;   
			case IDC_TXT:
               	_fstrcpy (ctype,"*.txt");
                goto SetType;
             break;   
			case IDC_TFW:
               	_fstrcpy (ctype,"*.tfw");
                goto SetType;
              break;   
			case IDC_BPW:
               	_fstrcpy (ctype,"*.bpw");
                goto SetType;
              break;   
			case IDC_GEOTIFF:
               	_fstrcpy (ctype,"*.tif");
                goto SetType;
              break;   
            case IDC_BITMAPCOORD:
               	_fstrcpy (ctype,"*.*");
                goto SetType;
              break;
            case IDC_AUTOSELECT:
				AutoSelect = SendDlgItemMessage (hWndDlg,IDC_AUTOSELECT,BM_GETCHECK,0,0);
              	goto SetType; 
            case IDCANCEL:
                 ContinueProcessing=FALSE;   
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);
                 break;
                 
            case IDC_EXIT:
                 GSSiEndDialog(hWndDlg, ContinueProcessing,hSaveBM); 
                 SetContinueProcessing ( TRUE);
                 break;
                 
            case LMD_TYPE_MAP:
                 itype = 1;
                 _fstrcpy (ctype,"*.plt"); 
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHSTATUS),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_CFTITLE),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_USEAVI),SW_HIDE);
                 //ShowWindow (GetDlgItem(hWndDlg,IDC_EXCMP),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_USEFREEIMAGE),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHLEVELS),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHLEVELST),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_IMAGE_QUALITY),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_IMAGE_QUALITY_LABEL),SW_HIDE); 
                 ShowWindow (GetDlgItem(hWndDlg,IDC_DESTDIR),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_DESTDIR_LABEL),SW_HIDE); 
                 ShowWindow (GetDlgItem(hWndDlg,IDC_CFTYPE2),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_TXT),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_TFW),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_BPW),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_BITMAPCOORD),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_GEOTIFF),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_UNITS),SW_HIDE);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_UNITSHEADER),SW_HIDE);
                 
                 goto SetType;
                 break;    
            
/*            case LMD_TYPE_USGSDOQ:
            	 itype = 3;
                 _fstrcpy (ctype,"*.*");
            	 goto ShowOrthoControls;*/

            case LMD_TYPE_ORTHO:
                 itype = 2;
                 _fstrcpy (ctype,"*.txt");   
				 SendDlgItemMessage (hWndDlg,IDC_TXT,BM_SETCHECK,TRUE,0L);
       ShowOrthoControls:
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHSTATUS),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_CFTITLE),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_USEAVI),SW_SHOW);
                // ShowWindow (GetDlgItem(hWndDlg,IDC_EXCMP),SW_SHOW);
                // ShowWindow (GetDlgItem(hWndDlg,IDC_USEFREEIMAGE),SW_SHOW);
				 for (i=0;i<4;i++)
                 	ShowWindow (GetDlgItem(hWndDlg,TileSizeControls[i]),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_TSLABEL),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHLEVELS),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_ORTHLEVELST),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_IMAGE_QUALITY),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_IMAGE_QUALITY_LABEL),SW_SHOW); 
                 ShowWindow (GetDlgItem(hWndDlg,IDC_DESTDIR),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_DESTDIR_LABEL),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_CFTYPE2),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_TXT),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_TFW),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_BPW),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_BITMAPCOORD),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_GEOTIFF),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_UNITS),SW_SHOW);
                 ShowWindow (GetDlgItem(hWndDlg,IDC_UNITSHEADER),SW_SHOW);
                  
                 goto SetType;
                 break;

            case ID_UP: 
                {
                 char   Dir[MAX_PATH];
                 LPSTR  LastDir;
                 short    len;
                 
                 //GetDlgItemText (hWndDlg,LMD_DIR,Dir,MAX_PATH); 
				 _getcwd (Dir,MAX_PATH);
                 LastDir=_fstrrchr(Dir,'\\');
                 if (LastDir) 
                 {
                 	*(++LastDir) = '\0'; 
                 	len = _fstrlen(Dir);
                 	if (len > 3)
                 		*(--LastDir) = '\0';
                 } 
                 else
                 	*Dir = 0;
                 SetDlgItemText (hWndDlg,LMD_DIR,Dir);
                 goto SetType;
                } 
                 
            case IDC_FINDFILES: 
            {
            	char	TempName[MAX_PATH], CurDir[MAX_PATH];
            	HFILE	Fid; 
            	LONG	TotFiles=0; 
            	short	lCurDir;
            	
            	//lCurDir = GetDlgItemText (hWndDlg,LMD_DIR,CurDir,MAX_PATH);   
				_getcwd (CurDir,MAX_PATH);
				lCurDir = strlen (CurDir);
				GSSiGetTempFileName(0,"gm",0,TempName);
				Fid =	GSSiOpenFile (TempName,0,OF_CREATE);
				SearchFilesInDir (CurDir,ctype,Fid,&TotFiles,ctype,1,TRUE,FALSE); 
				GSSillseek (Fid,0,0);
				SendDlgItemMessage(hWndDlg, LMD_NEWFILES, LB_RESETCONTENT, 0, 0);
				while (fgetstring (str,MAX_PATH,Fid))
				{   
					LPSTR	pFile=str;
					LPSTR pTAB = strchr (str,'\t');

					if (pTAB)
						*pTAB = 0;
					if (!_fstrnicmp (str,CurDir,lCurDir))
						pFile = &str[lCurDir+1];
					SendDlgItemMessage (hWndDlg,LMD_NEWFILES,LB_ADDSTRING,0,(LPARAM)pFile);    					
				}
				GSSiClose2 (&Fid); 
				goto SelectFiles;
			}	
            	 break;
            	      
                 
            case IDOK: 
            {
                char saveWT[144], cOrthRes[4], cOrthResList[32], Mess[256];
                short  SizeOpt; 
                BOOL	CurHeaderWritten;  
                HFILE   FidIndex, FidFileList;
                short     Version=2, NumLevs=1, CurLev=0;
                long    Signature=80251, FirstHeaderOffset, LastHeaderOffset, CurOffset=0;
                long	TotLen=0, CurLoc=1, FileListLen;
                LPSTR	pOrthRes, lpComma;
                MNMXCORD	FileBounds; 
                BOOL	UsesTimes, UseCPT; 
                char	SaveTFWUnits[32];
				BOOL SaveBMPCache = AllowBMPCaching;
				char	TempCoordFile[MAX_PATH]=""; 
				HFILE	FidAutoFileList = HFILE_ERROR;  
				BOOL	UseExCmp=TRUE;

                SetContinueProcessing ( TRUE);  
                AddBMPToCache (0,(HANDLE)1);
                Processing = TRUE;
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE);
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE);
				UseCPT = SendDlgItemMessage (hWndDlg,IDC_USECPT,BM_GETCHECK,0,0);  
				//UseExCmp = SendDlgItemMessage (hWndDlg,IDC_EXCMP,BM_GETCHECK,0,0);  
				 UsesTimes = SendDlgItemMessage (hWndDlg,IDC_USETIME,BM_GETCHECK,0,0);  
				 if (itype != 2)
				 	*cOrthResList = 0;
				 else
	                 GetDlgItemText (hWndDlg,IDC_ORTHLEVELS,cOrthResList,32);
                 Strip(cOrthResList,' ');
                 pOrthRes = cOrthResList; 
                 lpComma = _fstrchr (pOrthRes,',');
                 while (lpComma)
                 {
                 	NumLevs++;
                 	lpComma++;
                 	lpComma = _fstrchr (lpComma,',');
                 }
                 do
                 {
	                 lpComma = _fstrchr (pOrthRes,',');
	                 if (lpComma)
	                 	*lpComma++=0;
	                 else
	                 	lpComma = _fstrchr(pOrthRes,0);
	                 _fstrcpy (cOrthRes,pOrthRes);
	                 pOrthRes = lpComma;
	                 
	                 
	                 if (*AutoExportName)   
	                 {  
	                 	char	FullName[MAX_PATH], Drive[6], Dir[MAX_PATH]; 
	                 	LPSTR	lc;
	                 	Num=1;
	                 	*FromDir = 0; 
	                 	if (*AutoMapIndexName)
	                 		_fstrcpy (ToDir,AutoMapIndexName);
	                 	else
	                 	{
		                 	_fullpath (FullName,AutoExportName,sizeof(FullName));
		                 	_splitpath (FullName,Drive,Dir,0,0);
		                 	sprintf (ToDir,"%s%s",Drive,Dir);  
		                 	lc = LastChr (ToDir);
		                 	if (*lc == '\\')
		                 		*lc = 0;
		                }
	                 	if (!GetDlgItemText (hWndDlg,IDC_DESTDIR,FullName,MAX_PATH))
							SetDlgItemText(hWndDlg,IDC_DESTDIR,ToDir);
		                if (AutoFileList)
		                {
							GSSiClose2(&FidAutoFileList);
		                 	FidAutoFileList = GSSiOpenFile (AutoExportName,0,OF_READ);   
		                 	Num = NumRowsInTxtFile (FidAutoFileList);
		                }
	                 }
	                 else
	                 {
		                 Num = SendDlgItemMessage(hWndDlg, LMD_NEWFILES, LB_GETSELCOUNT, 0, 0);
		                 GetDlgItemText(hWndDlg,LMD_DIR,FromDir,255);
		 				 //_getcwd (FromDir,MAX_PATH);
		             }
	                 if (!Num)
	                 	goto Exit2; 
	                 GetGlobalCVal ("[%TFWUNITS]",SaveTFWUnits,0); 
	  			     if (SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_GETCURSEL,0,0)) 
	  			     {
        				PRJ_UNITS[3] = 2;
	                	SetGlobalValue ("%TFWUNITS","METERS"); 
	                 }
	  			     else   
	  			     {
	                	SetGlobalValue ("%TFWUNITS","FEET");
						PRJ_UNITS[3] = 1; 
					 }
	                 DisableHalt = TRUE; 
	                 Processing = TRUE; 
	                 setDoPaint( FALSE);
	                 hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	                 Handle = GSSiGlobAlloc (1112,GHND,Num*4);
	                 lpItems =(LPINT) GlobalLock (Handle);
	                 SendDlgItemMessage((HWND) hWndDlg, LMD_NEWFILES, LB_GETSELITEMS,Num, (LPARAM) lpItems);
	                 if (!GetDlgItemText(hWndDlg,IDC_DESTDIR,ToDir,255))
	                 	_fstrcpy(ToDir,FromDir); 
	                 else 
	                 {
	                 	ExpandText (ToDir); 
	                 	makedirectories (ToDir,TRUE,FALSE);
	                 }
	                 if (GetDlgItemText(hWndDlg,IDC_IMAGE_QUALITY,str,sizeof(str)))  
	                 	UserDefinedImageQuality = atoi(str) * 100; 
	                 else
	                 	UserDefinedImageQuality = 7200; 
	                 if (*FromDir && *LastChr (FromDir) != '\\')
	                 	_fstrcat (FromDir,"\\"); 
	                 sprintf (Index,"%s\\index%s",ToDir,cOrthRes);
	                 
	                 FidIndex = GSSiOpenFile (Index,(LPOFSTRUCTGM) &OFStruct,OF_CREATE); 
				     if (FileErrMess (FidIndex,Index,&OFStruct,OF_CREATE))  
				     	goto Exit;
	                 hlpFI = GSSiGlobAlloc(1742,GHND,sizeof(FILEINDEX)+sizeof(FILEINDEXENTRY)); 
	                 lpFI = (LPFILEINDEX) GlobalLock(hlpFI);    
	                 lpFI->CurrentEntry=(FILEINDEXENTRY *) &lpFI->FirstIndex;
	                 lpFI->Type = min (5,itype + 3);   
	                 lpFI->UsesTimes = UsesTimes;     
	                 
				     DBoundsInit (&FileBounds);
				     BigWrite (FidIndex,(char *)&FileBounds,sizeof(MNMXCORD),-1);  
				     FirstHeaderOffset = LastHeaderOffset = GSSillseek (FidIndex,0,1);    

					 DBoundsInit (&lpFI->Bounds);   
					 lpFI->NumFiles=0;
					 lpFI->Length=0; 
					 BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1); 
					 CurHeaderWritten = TRUE;
	                 GetWindowText(hWndDlg,saveWT,144);
	
	                 SizeOpt = 0; 
	                 if (SendDlgItemMessage (hWndDlg,IDC_TSIZE1,BM_GETCHECK,0,0))
	                    SizeOpt = 1;
	                 if (SendDlgItemMessage (hWndDlg,IDC_TSIZE2,BM_GETCHECK,0,0))
	                    SizeOpt = 2;
	                 if (SendDlgItemMessage (hWndDlg,IDC_TSIZE3,BM_GETCHECK,0,0))
	                    SizeOpt = 3;
	                 
	                 for (item=0;item<Num;item++,lpItems++)
	                 {  
	                 	if (!ContinueProcessing)
	                 		break;  
						if (AutoFileList)
						{
							LPSTR pTab;
							fgetstring(str, 250, FidAutoFileList);
							if ((pTab = strchr(str, '\t')))
								*pTab = 0;
							if (!stricmp(str, "FULLNAME"))
								continue;
						}
	                 	else if (*AutoExportName)
	                 		_fstrcpy (str,AutoExportName);
	                 	else
	                 	{   
		                    _fstrcpy (str,FromDir); 
		                    SendDlgItemMessage (hWndDlg,LMD_NEWFILES,LB_GETTEXT,
		                        *lpItems, (LPARAM) _fstrchr(str,0));  
		                } 
		                ExpandText (str);
	                    _fstrlwr (str);  
	                    //GetLongPathName (str,256);
	                    if (_fstrstr (str,"filelist.txt"))
	                    {   
	                    	HANDLE	hDLT=0;
	                    	char	SkipList[130], SkipDir[130];
	                    	
                    	    GetGlobalCVal ("[%SKIPLIST]",SkipList,0);
                    	    GetGlobalCVal ("[%SKIPDIR]",SkipDir,0); 
	                    	FidFileList = GSSiOpenFile (str,&OFStruct,OF_READ);
	                    	if (FidFileList == HFILE_ERROR)
	                    		goto Exit;     
	                    	if (!TotLen)
	                    	{
	                    		FileListLen = GSSillseek (FidFileList,0,2);
	                    		TotLen = FileListLen * NumLevs;
	                    		GSSillseek (FidFileList,0,0);
	                    	} 
	                    	fgetstring (str,128,FidFileList);
							if (!ProcessDelimTextHeader(str, 0, FidFileList, &hDLT, 0, 0))
								goto Exit;
	                    	
	                    	while (ContinueProcessing && fgetstring (str,258,FidFileList))
	                    	{   
	                    		char	leaf[34]="[FILENAME]";
							    GetDelimTextData(str,hDLT,258); 
							    _fstrcpy (str,"[FULLNAME]");
							    ExpandText (str);  
							    ExpandText (leaf);
							    if (*SkipList)
							    {
							    	HANDLE hDLTSkip=0;
							    	char	skipfile[MAX_PATH];
							    	HFILE	FidSkip=GSSiOpenFile (SkipList,&OFStruct,OF_READ);
							    	if (FidSkip != HFILE_ERROR)
							    	{   
							    		fgetstring (skipfile,MAX_PATH,FidSkip);
										ProcessDelimTextHeader(skipfile, 0, FidSkip, &hDLTSkip, 0, 0);
										while (fgetstring (skipfile,MAX_PATH,FidSkip))
										{
											GetDelimTextData(skipfile,hDLTSkip,MAX_PATH); 
											_fstrcpy (skipfile,"[SKIPNAME]");
											ExpandText (skipfile);
											if (!_fstricmp (skipfile,leaf))
											{  
									    		GSSiClose2 (&FidSkip); 
									    		GSSiGlobFree (&hDLTSkip);  
									    		if (!CurLev)
									    		{
										    		_fstrcpy (skipfile,"[%SKIPDIR]\\[SKIPNAME].bpw");
										    		ExpandText (skipfile);
										    		copyfile (skipfile,str,FALSE,0,0,0,0,0,0);
										    		_fstrcpy (skipfile,"[%SKIPDIR]\\[SKIPNAME].bmp");
										    		ExpandText (skipfile); 
										    		REPLAC (str,".BPW",".BMP",sizeof(str));
										    		copyfile (skipfile,str,FALSE,0,0,0,0,0,0); 
										    	}
												goto NextFile;
											}	
										}
							    		GSSiGlobFree (&hDLTSkip); 
							    		GSSiClose2 (&FidSkip);
							    	}
							    }
	                    		sprintf (Mess,"File %s: Level %s",str,cOrthRes);
		                    	SetDlgItemText(hWndDlg,IDC_LOAD_MESSAGE,Mess);
			                    if (!AddMapToDir (hWndDlg,str,ToDir,lpFI,itype,  
			                                  (BOOL)SendDlgItemMessage (hWndDlg,IDC_USEAVI,BM_GETCHECK,0,0), 
			                                  (BOOL)SendDlgItemMessage (hWndDlg,IDC_USEFREEIMAGE,BM_GETCHECK,0,0),
			                                  GetDlgItem(hWndDlg,IDC_ORTHSTATUS),FidIndex,SizeOpt,cOrthRes,UsesTimes,
			                                  &LastHeaderOffset,&CurHeaderWritten,&FileBounds,UseCPT,UseExCmp))
			                        goto Exit;  
			                    CurLoc = FileListLen * CurLev + GSSillseek (FidFileList,0,1);
			                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS_BAR), TotLen, CurLoc,0);
		                    	CurOffset = GSSillseek (FidIndex,0,1);
			                    if (!CurHeaderWritten && (itype > 1 || lpFI->NumFiles > 255))
			                    {   
			                    	lpFI->NextHeaderOffset = CurOffset;
			                    	GSSillseek (FidIndex,LastHeaderOffset,0);
									BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
									GSSillseek (FidIndex,CurOffset,0);
									DBoundsInit (&lpFI->Bounds);   
									lpFI->NumFiles=0;
									lpFI->Length=0; 
			                    	LastHeaderOffset = CurOffset;
									BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
									CurHeaderWritten = TRUE;
			                    }
			                    else
									CurHeaderWritten = FALSE;
	               NextFile:;
			                    
			                }
			                GSSiClose2 (&FidFileList);  
   							GSSiGlobFree (&hDLT);
	                    }
	                    else
	                    {
                    		sprintf (Mess,"File %s: Level %s",str,cOrthRes);
	                    	SetDlgItemText(hWndDlg,IDC_LOAD_MESSAGE,Mess);
		                    SetWindowText(hWndDlg,Mess);
		                    if ((BOOL)SendDlgItemMessage (hWndDlg,IDC_BITMAPCOORD,BM_GETCHECK,0,0))
		                    {   
		                    	HFILE	FidOut;
		                    	
								GSSiGetTempFileName (0,"gm",0,TempCoordFile); 
						    	FidOut = GSSiOpenFile (TempCoordFile,&OFStruct,OF_CREATE);  
						    	fputstring ("METERS",FidOut);
						    	fputstring ("0.0",FidOut);
						    	fputstring ("0.0",FidOut);
						    	fputstring ("1.0",FidOut);
							    fputstring (str,FidOut);
								GSSiClose2 (&FidOut);
								_fstrcpy (str,TempCoordFile);                    	
		                    } 
		                    if (!AddMapToDir (hWndDlg,str,ToDir,lpFI,itype,   
		                                  (BOOL)SendDlgItemMessage (hWndDlg,IDC_USEAVI,BM_GETCHECK,0,0),
		                                  (BOOL)SendDlgItemMessage (hWndDlg,IDC_USEFREEIMAGE,BM_GETCHECK,0,0),
		                                  GetDlgItem(hWndDlg,IDC_ORTHSTATUS),FidIndex,SizeOpt,cOrthRes,UsesTimes,
		                                  &LastHeaderOffset,&CurHeaderWritten,&FileBounds,UseCPT,UseExCmp))
		                        goto Exit;
		                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS_BAR), Num*NumLevs, CurLoc++,0);
	                    	CurOffset = GSSillseek (FidIndex,0,1);
		                    if (!CurHeaderWritten && (itype > 1 || lpFI->NumFiles > 255))
		                    {   
		                    	lpFI->NextHeaderOffset = CurOffset;
		                    	GSSillseek (FidIndex,LastHeaderOffset,0);
								BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
								GSSillseek (FidIndex,CurOffset,0);
								DBoundsInit (&lpFI->Bounds);   
								lpFI->NumFiles=0;
								lpFI->Length=0; 
		                    	LastHeaderOffset = CurOffset;
								BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
								CurHeaderWritten = TRUE;
		                    }
		                    else
								CurHeaderWritten = FALSE;
		                } 
	                 }
	            Exit:
	            	 if (*TempCoordFile)
	            	 	GSSiRemove (TempCoordFile);
	            	 *TempCoordFile = 0;
	            	 GSSiGlobUlFree (&Handle);
					 if (FidIndex == HFILE_ERROR)
						 goto Exit2;
	                 GSSiChangeLength (FidIndex,CurOffset);
	                 ii=GSSillseek (FidIndex,0,2);
	                 BigWrite(FidIndex,(HPSTR)&Signature,4,-1);
	                 BigWrite(FidIndex,(HPSTR)&Version,2,-1);
	                 GSSillseek (FidIndex,0,0);
				     BigWrite (FidIndex,(HPSTR)&FileBounds,sizeof(MNMXCORD),-1); 
				     if (!CurHeaderWritten)
				     { 
					     GSSillseek (FidIndex,LastHeaderOffset,0);
					     BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
					 }
				     GSSillseek (FidIndex,FirstHeaderOffset,0);
				     BigRead (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH);
				     lpFI->EndOffset = CurOffset;
				     GSSillseek (FidIndex,FirstHeaderOffset,0);
				     BigWrite (FidIndex,(HPSTR)lpFI,STOREDINDEXLENGTH,-1);
	                 GSSiClose2 (&FidIndex);  
	                 GSSiGlobUlFree (&hlpFI);
	                 /*CloseMapIndex (Index,hlpFI,TRUE);*/
	                 if (SendDlgItemMessage (hWndDlg,IDC_USEAVI,BM_GETCHECK,0,0))
	                 {
	                    AVIOutClose(&hAVIFile);
	                 }
	                 DisableHalt = FALSE; 
	                 setDoPaint( TRUE); 
	                 SetWindowText(hWndDlg,saveWT);  
	                 CurLev++;
	             } while (*pOrthRes && ContinueProcessing); 
	             if (itype == 1 && ContinueProcessing)
	             {  
	             	long	lmem=sizeof(VIEWPORT)+128*MAX_VIEWPORT_FILES; 
	             	HANDLE	hVP=GSSiGlobAlloc (1113,GHND,lmem);
	             	LPVIEWPORT	pSaveVP = (LPVIEWPORT)GlobalLock (hVP);  
	             	HANDLE		hSymDesc;
	             	short		NumParent=0, NumSyms=0;
	             	HFILE		FidSyms;
				    LPSYMDESC	pSymDesc;
				    LPSYMBOL    pSymbol;  
				    LPSTR		lpTAB;   
				    VISLIST		SaveVis;
				    char		VisName[MAX_PATH];
	             	
				    SetCurView (pViewports[*pCommandViewport-1]);
	             	_fmemmove (pSaveVP,CurView,(size_t)lmem);  
	             	CurView->NumFiles = 1;
					CurView->FileType[0]=4;
	             	_fstrcpy (CurView->lpFiles[0],ToDir);
	             	_fstrcat (CurView->lpFiles[0],"\\symlist"); 
	             	GSSiRemove (CurView->lpFiles[0]);  
	             	_fstrcpy (VisName,CurView->lpFiles[0]);
	             	_fstrcpy (CurView->lpFiles[0],ToDir);
	             	_fstrcat (CurView->lpFiles[0],"\\index"); 
	             	if (!CurVis)
	             		SelectVisList (FALSE);
					SaveVis = *CurVis; 
					IgnoreBounds = TRUE;
					InitVis ();   
				 	GetVisList (hWndDlg,SYM_VIS_LB, PAR_VIS_LB,0,1,HFILE_ERROR); 
				 	*CurVis = SaveVis; 
				 	IgnoreBounds = FALSE; 
	             	FidSyms = GSSiOpenFile (VisName,&OFStruct,OF_CREATE);
				 	hSymDesc = GSSiGlobAlloc (1114,GMEM_MOVEABLE,USHRT_MAX);
				    pSymDesc = (LPSYMDESC)GlobalLock (hSymDesc);
				 	while (SendDlgItemMessage (hWndDlg,SYM_VIS_LB,LB_GETTEXT,NumSyms,(LPARAM)str)!=LB_ERR) 
				 	{   
						NumSyms++;
				 		pSymDesc->Handle = GSSiGlobAlloc (1115,GHND,sizeof(SYMBOL));
				        pSymbol = (LPSYMBOL)GlobalLock (pSymDesc->Handle); 
				        lpTAB = _fstrrchr (str,'\t');
				        *lpTAB++ = 0;
			            pSymDesc->Number = atoi (lpTAB);
				        lpTAB = _fstrrchr (str,'\t');
				        *lpTAB++ = 0;
			            pSymbol->Parent = atoi (lpTAB);
			            pSymbol->Type = 1;
				        lpTAB = _fstrchr (str,'\t');
				        *lpTAB = 0;
			            _fstrcpy (pSymbol->Name,str);
				        GlobalUnlock (pSymDesc->Handle); 
						pSymDesc++;
				    } 

				 	while (SendDlgItemMessage (hWndDlg,PAR_VIS_LB,LB_GETTEXT,NumParent,(LPARAM)str)!=LB_ERR) 
				 	{   
				 		NumSyms++;
						NumParent++;
				 		pSymDesc->Handle = GSSiGlobAlloc (1116,GHND,sizeof(SYMBOL));
				        pSymbol = (LPSYMBOL)GlobalLock (pSymDesc->Handle); 
				        lpTAB = _fstrrchr (str,'\t');
				        *lpTAB++ = 0;
			            pSymDesc->Number = atoi (lpTAB);
				        lpTAB = _fstrrchr (str,'\t');
				        *lpTAB++ = 0;
			            pSymbol->Parent = atoi (lpTAB);
			            pSymbol->Type = 0;
				        lpTAB = _fstrchr (str,'\t');
				        *lpTAB = 0;
			            _fstrcpy (pSymbol->Name,str);
				        GlobalUnlock (pSymDesc->Handle); 
						pSymDesc++;
				    }  
				    
					GlobalUnlock (hSymDesc); 
					WriteSymList (FidSyms, NumParent, NumSyms,hSymDesc);
					BOOL saveAllowCachedSymbols = allowCachedSymbols;
					allowCachedSymbols = FALSE;
	                DestroySymList (&NumSyms,&hSymDesc);
					allowCachedSymbols = saveAllowCachedSymbols;
	             	_fmemmove (CurView,pSaveVP,(size_t)lmem);
				    GSSiGlobUlFree (&hVP); 
				    GSSiClose2 (&FidSyms);
	             }
	      Exit2:
                 GSSiSetCursor (hcurSave);
                 Processing = FALSE;
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);
                 EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE);  
               	 GSSiClose2 (&FidAutoFileList);
                 if (*AutoExportName)
                 	GSSiEndDialog(hWndDlg, ContinueProcessing,hSaveBM); 
                 else if (ContinueProcessing)
                  	SetDlgItemText(hWndDlg,IDC_LOAD_MESSAGE,"Index created"); 
                 else
                  	SetDlgItemText(hWndDlg,IDC_LOAD_MESSAGE,"Index creation cancelled");
                 SetContinueProcessing ( TRUE);  
                 SetGlobalValue ("%TFWUNITS",SaveTFWUnits);
				 AllowBMPCaching = SaveBMPCache;
                 break;      
            }

           }
         break; 

    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL MULTIFILEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (792);
#endif
{  
	char	str[256], FullPath[MAX_PATH], WildCard[64]; 
	BOOL	isDir; 
	LPSTR	pDir; 
	short	nItems;
	LPINT	pItems;
	HANDLE	hItems;
	short	i, Choice;
 switch(Message)
   {
    case WM_INITDIALOG:
        
        TotFiles = 0;
        pDir = GlobalLock (hDir);
		sprintf (str,"%s*%s",pDir,MFExt);
        DlgDirList (hWndDlg,str,IDC_AVAILABLE,IDC_CURDIR,DDL_DIRECTORY);
        GlobalUnlock (hDir);
        SetDlgItemText (hWndDlg,IDC_WILDCARD,""); 
        goto DisplayMess; 

    case WM_COMMAND:

		 switch(LOWORD (wParam))

           {  
            case IDC_ADD:
                 nItems=SendDlgItemMessage(hWndDlg,IDC_AVAILABLE,
												   LB_GETSELCOUNT,
												   0,
												   0);
				 if (!nItems) 
				 	break;
		         hItems = GSSiGlobAlloc ( 497,GHND,nItems*4);
		         pItems = (LPINT)GlobalLock(hItems);    
		         SendDlgItemMessage(hWndDlg,IDC_AVAILABLE,
										   LB_GETSELITEMS,
										   nItems,
										   (LPARAM)pItems);
				 for (i=0;i<nItems;i++,pItems++)
				 {
				 	SendDlgItemMessage(hWndDlg,IDC_AVAILABLE,LB_GETTEXT,
						                       *pItems,(LPARAM)str); 
					if (*str == '[')
					{
						isDir = TRUE;
						_fmemmove (str,&str[1],_fstrlen(str));
						*_fstrchr(str,']') = 0;
					} 
					else
					{
						isDir = FALSE;
						TotFiles++;
					}
					_fullpath (FullPath,str,sizeof(FullPath));
					_fstrupr (FullPath);  
					if (isDir)
					{   
						HCURSOR hcurSave;  
						long	nFiles;
						
		                hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
		                GetDlgItemText (hWndDlg,IDC_WILDCARD,WildCard,sizeof(WildCard));
						nFiles = SearchFilesInDir (FullPath, MFExt, HFILE_ERROR,&TotFiles,WildCard,1,TRUE,FALSE);    
						sprintf (str,"%s (%ld files)",FullPath,nFiles);
						
						GSSiSetCursor (hcurSave); 
					}
					else
						_fstrcpy (str,FullPath);  
					SendDlgItemMessage(hWndDlg,IDC_SELECTED,LB_ADDSTRING,0,(LPARAM)str); 
				 }
				 goto DisplayMess;
            	break;
            case IDC_REMOVE: 
            {
            	LPSTR	lpEnd;
            	
			 	Choice=SendDlgItemMessage(hWndDlg,IDC_SELECTED,LB_GETCURSEL,
		                       				0,0); 
			 	SendDlgItemMessage(hWndDlg,IDC_SELECTED,LB_GETTEXT,Choice,(LPARAM)str); 
			 	if ((lpEnd = _fstrstr (str," (")))
			 	{   
			 		long	n;
			 		
			 		lpEnd+=2;
			 		n = atol (lpEnd);
			 		TotFiles-=n;
			 	}
			 	else
			 		TotFiles--;
			 	SendDlgItemMessage(hWndDlg,IDC_SELECTED,LB_DELETESTRING,
		                       		Choice,0);  
		    }
            	break;
            case IDC_AVAILABLE:                                
              {
#if WIN32
				switch(HIWORD(wParam))
#else
                switch(HIWORD(wParam))
#endif
                    {
                     case LBN_SELCHANGE: 
                     	EnableWindow (GetDlgItem(hWndDlg,IDC_ADD),TRUE);
                     	EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVE),FALSE);
                     	break;
                     case LBN_DBLCLK:
                          isDir = DlgDirSelectEx(hWndDlg,str,sizeof(str),wParam);
                          if (isDir)
                          {   
								_fstrcat (str,"*");
								_fstrcat (str,MFExt);
								DlgDirList (hWndDlg,str,IDC_AVAILABLE,IDC_CURDIR,DDL_DIRECTORY);
                          } 
                          else
                          {
								TotFiles++;
								_fullpath (FullPath,str,sizeof(FullPath));  
								SendDlgItemMessage(hWndDlg,IDC_SELECTED,LB_ADDSTRING,0,(LPARAM)FullPath);
						  }
                          goto DisplayMess;
                     }
               } 
               break;
            case IDC_SELECTED:                                
              {
#if WIN32
				switch(HIWORD(wParam))
#else
                switch(HIWORD(wParam))
#endif
                    {
                     case LBN_SELCHANGE: 
                     	EnableWindow (GetDlgItem(hWndDlg,IDC_ADD),FALSE);
                     	EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVE),TRUE);
                     	break;
                    }
               } 
               break;
            case IDCANCEL:
                 EndDialog(hWndDlg, FALSE);
                 break;

            case IDOK:
            {
            	HFILE	Fid;
            	LPSTR	pName, lpEnd; 
				OFSTRUCTGM	OFStruct;
				HCURSOR hcurSave;  
						
		        TotFiles = 0;
                 hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
                 nItems=SendDlgItemMessage(hWndDlg,IDC_SELECTED,
												   LB_GETCOUNT,0,0);  
				 if (!nItems)
				 	break;
            	 pName = GlobalLock (hMFName); 
            	 Fid = 	GSSiOpenFile (pName,(LPOFSTRUCTGM)&OFStruct,OF_CREATE);
                 for (i=0;i<nItems;i++)
                 {
				 	SendDlgItemMessage(hWndDlg,IDC_SELECTED,LB_GETTEXT,i,(LPARAM)str); 
				 	if ((lpEnd = _fstrstr (str," (")))
				 	{
				 		*lpEnd = 0;
		                GetDlgItemText (hWndDlg,IDC_WILDCARD,WildCard,sizeof(WildCard));
				 		SearchFilesInDir (str, MFExt, Fid,&TotFiles,WildCard,1,TRUE,FALSE);   
				 	}
				 	else
				 	{
				 		TotFiles++;
				 		fputstring (str,Fid);
				 	}
				 } 
                 
                 GSSiClose2 (&Fid);
				 GSSiSetCursor (hcurSave);
                 GlobalUnlock (hMFName);
                 EndDialog(hWndDlg, TRUE);
                 break;
              }

           }

         break;

    case WM_CLOSE:
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    default:
{
#if ENABLETRACE
GSSiExitProg (792);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (792);
#endif
 return TRUE;   
}
 
DisplayMess:
	sprintf (str,"%ld files selected",TotFiles);
	SetDlgItemText (hWndDlg,IDC_MESS,str);
{
#if ENABLETRACE
GSSiExitProg (792);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL CREATEFILELISTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (921);
#endif
{   
    char    str[512];  
    static	char	ctype[2]="";
    BOOL    isDir;
    short     nchar, item;
    OFSTRUCTGM    OFStruct;
    
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (921);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:
		 SendDlgItemMessage (hWndDlg,IDC_NEWFILE,BM_SETCHECK,TRUE,0L);
		 SendDlgItemMessage (hWndDlg,IDC_ORTHOS,BM_SETCHECK,TRUE,0L);     
		 SetDlgItemText (hWndDlg,IDC_WILDCARD,"index1"); 
		 SendDlgItemMessage (hWndDlg,IDC_USELONGNAMES,BM_SETCHECK,TRUE,0);
		 _fstrcpy (str,"[%DL]");
	     ExpandText (str); 
	     if (*LastChr (str) == '\\')
	     	*LastChr (str) = 0; 
	     _fstrlwr (str);
	     SetDlgItemText (hWndDlg,IDC_CURDIR,str);

SetType:
		 *str = 0;
         DlgDirList (hWndDlg,str,IDC_SEARCHLIST,IDC_CURDIR,DDL_DIRECTORY|DDL_DRIVES|DDL_EXCLUSIVE);

         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

		 switch(LOWORD (wParam))

           {
            case IDC_SEARCHLIST: /* List Box                                 */
              {
#if WIN32
				switch(HIWORD(wParam))
#else
                switch(HIWORD(wParam))
#endif
                    {
                     case LBN_DBLCLK:
                          isDir = DlgDirSelectEx(hWndDlg,str,sizeof(str),IDC_SEARCHLIST);
                          if (isDir)
                             {nchar = _fstrlen (str);
                              if (!nchar)
                              	break;
                              if (str[nchar-1] == '\\')
                                  _fstrcat (str,ctype);
                              if (str[nchar-1] == ':') 
                              {
                              	  _fstrcat (str,"\\");
                           	      SetDlgItemText (hWndDlg,IDC_CURDIR,str);
                                  _fstrcat (str,ctype); 
                              }
					          EnableWindow (GetDlgItem(hWndDlg,IDC_UP),TRUE);  
                              DlgDirList (hWndDlg,str,
                                  IDC_SEARCHLIST,IDC_CURDIR,DDL_DIRECTORY|DDL_DRIVES|DDL_EXCLUSIVE);
/*                          isDir = DlgDirSelect(hWndDlg,str,IDC_SEARCHLIST);
                          if (isDir)
                          {
                              nchar = _fstrlen (str);
                              if (!nchar)
                              	break;
                              if (str[nchar-1] == ':') 
                              {
                              	  _fstrcat (str,"\\");
                           	      SetDlgItemText (hWndDlg,IDC_CURDIR,str);
                                  //_fstrcpy (str,ctype); 
                              }
					          EnableWindow (GetDlgItem(hWndDlg,IDC_UP),TRUE);  
                              DlgDirList32 (hWndDlg,"",
                                  IDC_SEARCHLIST,IDC_CURDIR,DDL_DIRECTORY|DDL_DRIVES|DDL_EXCLUSIVE);  */
                          }
                          break;  
                     case LBN_SELCHANGE:
	                 	EnableWindow (GetDlgItem (hWndDlg,IDC_ADD),(BOOL)SendDlgItemMessage(hWndDlg, IDC_SEARCHLIST, LB_GETSELCOUNT, 0, 0));
                     	break;
                     }

              }
              break;
            
            case IDC_ORTHOS:
				SetDlgItemText (hWndDlg,IDC_WILDCARD,"index1"); 
				break;
            
            case IDC_MAPS:
				SetDlgItemText (hWndDlg,IDC_WILDCARD,"index"); 
				break;
             
            case IDC_FULLPATH:	 
            case IDC_NAMEONLY:
            case IDC_NAMEANDEXTENSION:
            case IDC_MACROINPUT:
			case IDC_LISTINPUT:
				SetDlgItemText (hWndDlg,IDC_WILDCARD,"*.*"); 
				break;
            
            case IDC_ADDLIST: /* List Box                                 */
              {
#if WIN32
				switch(HIWORD(wParam))
#else
                switch(HIWORD(wParam))
#endif
                    {
                     case LBN_SELCHANGE:
	                 	EnableWindow (GetDlgItem (hWndDlg,IDC_REMOVE),(BOOL)SendDlgItemMessage(hWndDlg, IDC_ADDLIST, LB_GETSELCOUNT, 0, 0));
                     	break;
                     }

              }
              break;
            
            case IDC_ADD: 
            {
            	 short	Num;
            	 HANDLE	Handle;
            	 LPINT	lpItems; 
            	 char	str2[260];
            	 LPSTR	lpBrack;
            	 
				 Num = SendDlgItemMessage(hWndDlg, IDC_SEARCHLIST, LB_GETSELCOUNT, 0, 0); 
				 if (!Num) break;
                 Handle = GSSiGlobAlloc ( 679,GHND,Num*4);
                 lpItems =(LPINT) GlobalLock (Handle);
                 SendDlgItemMessage((HWND) hWndDlg, IDC_SEARCHLIST, LB_GETSELITEMS, Num, (LPARAM) lpItems);
                 while (Num--)
                 {
	         		  SendDlgItemMessage(hWndDlg,IDC_SEARCHLIST,LB_GETTEXT,*(lpItems++),(DWORD)&str);
	         		  if (!_fstrnicmp (str,"[-",2))
	         		  { 
	         		  	sprintf (str2,"%c:",str[2]);
	         		  }
	         		  else
	         		  {
		         		  GetDlgItemText (hWndDlg,IDC_CURDIR,str2,sizeof(str2));
		         		  if (*LastChr (str2) != '\\')
		         		  	_fstrcat (str2,"\\");
		         		  _fstrcat (str2,&str[1]); 
		         		  if ((lpBrack = _fstrchr (str2,']')))
		         		  	*lpBrack = 0;
		         	  }
	         		  SendDlgItemMessage(hWndDlg,IDC_ADDLIST,LB_ADDSTRING,0,(DWORD)&str2);
                 } 
                 GSSiGlobUlFree (&Handle);
			}
            	 break;
            case IDC_REMOVE: 
            {
            	 short	Item;
            	 
                 Item = SendDlgItemMessage((HWND) hWndDlg, IDC_ADDLIST, LB_GETCURSEL,0,0);
                 if (Item != LB_ERR)
	         	 	SendDlgItemMessage(hWndDlg,IDC_ADDLIST,LB_DELETESTRING,Item,0);
			}
            	 break;
            case IDCANCEL:
			   	 _chdir (CurDir);
           	  	 _chdrive (SaveDrive);  
                 EndDialog(hWndDlg, FALSE); 
                 SetContinueProcessing ( TRUE);
                 break;
                 

            case IDC_LOCATE_DESTFILE: 
                 *str=0;
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWFILE,BM_GETCHECK,0,0)) 
                 {
                    if (!GetSaveName2 (hWndDlg,str,IDS_FILTERTEXT,".TXT",IDS_FILETXT)) break;   
                 }
                 else  
                 {
                    if (!GetFileName3 (hWndDlg,str,IDS_FILTERTEXT,IDS_FILETXT)) break;   
                 }
                 SetDlgItemText (hWndDlg,IDC_DESTFILE,str);
                 break;
                 
            case IDC_UP: 
                {
                 char   Dir[MAX_PATH];
                 LPSTR  LastDir;
                 short    len;
                 
                 GetDlgItemText (hWndDlg,IDC_CURDIR,Dir,MAX_PATH); 
                 LastDir=_fstrrchr(Dir,'\\');
                 if (LastDir) 
                 {
                 	*(++LastDir) = '\0'; 
                 	len = _fstrlen(Dir);
                 	if (len > 3)
                 		*(--LastDir) = '\0';
                 } 
                 else
                 	*Dir = 0;
                 SetDlgItemText (hWndDlg,IDC_CURDIR,Dir);
/*                 goto SetType;
                } 
                {
                 char   Dir[128];
                 LPSTR  LastDir;
                 short    len;
                 
                 _getcwd (Dir,128); 
                 LastDir=_fstrrchr(Dir,'\\');
                 if (*(LastDir+1))
                 {
                 	*LastDir = 0; 
                 	*(LastDir+1) = 0;
                 	if (!_fstrchr (&Dir[2],'\\'))
                 		*LastDir = '\\';
                 	_chdir (Dir);
                	goto SetType;
                 }*/
				 *str = 0;
		         DlgDirList (hWndDlg,Dir,IDC_SEARCHLIST,IDC_CURDIR,DDL_DRIVES|DDL_DIRECTORY|DDL_EXCLUSIVE);  
		         if (GetDlgItemText (hWndDlg,IDC_CURDIR,str,128) < 4)
		         	EnableWindow (GetDlgItem(hWndDlg,IDC_UP),FALSE);  
		         //SetDlgItemText (hWndDlg,IDC_CURDIR,0);
               } 
                 break;
                 
            case IDOK: 
            {
            	 short	Num,i;
            	 char	str2[2*_MAX_PATH+2],TempName[_MAX_FNAME],Name[_MAX_FNAME], WildCard[16], drive[_MAX_DRIVE], dir[_MAX_DIR], extension[_MAX_EXT];
            	 LPSTR	lpBrack; 
            	 HFILE	OutFileFID, Fid;  
            	 long	TotFiles=0;
            	 HCURSOR	hcurSave; 
            	 
			   	 _chdir (CurDir);
           	  	 _chdrive (SaveDrive);  
            	 GetDlgItemText (hWndDlg,IDC_WILDCARD,WildCard,sizeof(WildCard));
				 Num = SendDlgItemMessage(hWndDlg, IDC_ADDLIST, LB_GETCOUNT, 0, 0); 
				 if (!Num)
				 {
					GSSiMsgBox( hWndDlg,"No directories selected",0, MB_OK|MB_ICONEXCLAMATION,0);
					break;
				 }
				 if (!GetDlgItemText (hWndDlg,IDC_DESTFILE,Name,sizeof(Name)))
				 {
					GSSiMsgBox( hWndDlg,"No destination file",0, MB_OK|MB_ICONEXCLAMATION,0);
					break;
				 }
				 if (SendDlgItemMessage (hWndDlg,IDC_NEWFILE,BM_GETCHECK,0,0))
				 {
				 	OutFileFID = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
					if (SendDlgItemMessage (hWndDlg,IDC_MACROINPUT,BM_GETCHECK,0,0))
						fputstring ("FULLNAME\tFILENAME\tDRIVE\tDIRECTORY\tLASTDIR\tDRIVEDIR\tEXTENSION\tCREATTIME\tLASTACCESS\tLASTWRITE\tFILELENGTH\tSTATUS",OutFileFID);
				 }	
				 else 
				 {
				 	OutFileFID = GSSiOpenFile (Name,&OFStruct,OF_READWRITE);
				 	GSSillseek (OutFileFID,0,2); 
				 }
				 hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
				 GSSiGetTempFileName(0,"gm",0,TempName);
                 for (i=0;i<Num;i++)
                 {
	         		  SendDlgItemMessage(hWndDlg,IDC_ADDLIST,LB_GETTEXT,i,(DWORD)&str);
            	 	  Fid =	GSSiOpenFile (TempName,(LPOFSTRUCTGM)&OFStruct,OF_CREATE);
				 	  SearchFilesInDir (str, "", Fid,&TotFiles,WildCard,1,TRUE,FALSE); 
				 	  GSSillseek (Fid,0,0);
				 	  while (fgetstring (str2,2*_MAX_PATH,Fid))
				 	  { 
					    LPSTR pTAB = strchr (str2,'\t');

						if (pTAB)
							*pTAB++ = 0;
				 	    if (SendDlgItemMessage (hWndDlg,IDC_USELONGNAMES,BM_GETCHECK,0,0))
				 	    	GetLongPathName2 (str2,sizeof(str2)-2);
				 	    if (SendDlgItemMessage (hWndDlg,IDC_ORTHOS,BM_GETCHECK,0,0))
				 	    {   
				 	    	if (!_fstrchr(str2,'.'))
				 	    	{
				 	    		if (isdigit(*LastChr(str2)))
				 	  				_fstrcpy (LastChr(str2),"[%ORTHORES]");
				 	  		}
				 	  		fputstring (str2,OutFileFID); 
				 	  	}
				 	  	else if (SendDlgItemMessage (hWndDlg,IDC_MAPS,BM_GETCHECK,0,0) ||
				 	  			 SendDlgItemMessage (hWndDlg,IDC_FULLPATH,BM_GETCHECK,0,0))
						{
							SubstituteDL (str2,FALSE);
				 	  		fputstring (str2,OutFileFID);
						}
				 	  	else if (SendDlgItemMessage (hWndDlg,IDC_NAMEONLY,BM_GETCHECK,0,0))
				 	  	{
				 	  		_splitpath (str2,drive,dir,Name,extension);
				 	  		sprintf (str,"%s",Name);
				 	  		fputstring (str,OutFileFID);
				 	  	}
				 	  	else if (SendDlgItemMessage (hWndDlg,IDC_NAMEANDEXTENSION,BM_GETCHECK,0,0))
				 	  	{
				 	  		_splitpath (str2,drive,dir,Name,extension);
				 	  		sprintf (str,"%s%s",Name,extension);
				 	  		fputstring (str,OutFileFID);
				 	  	}
				 	  	else if (SendDlgItemMessage (hWndDlg,IDC_LISTINPUT,BM_GETCHECK,0,0))
				 	  	{
				 	  		_splitpath (str2,drive,dir,Name,extension);
				 	  		sprintf (str,"%s|%s",Name,str2);
				 	  		fputstring (str,OutFileFID);
				 	  	}
				 	  	else
				 	  	{   
				 	  		LPSTR	pLastDir;
				 	  		char	LastDir[256];
							char	timesAndLength[256]={0};

							if (pTAB)
								strcpy (timesAndLength,pTAB);
				 	  		_splitpath (str2,drive,dir,Name,extension);  
				 	  		_fstrcpy (LastDir,dir);
				 	  		if (*LastChr (LastDir) == '\\')
				 	  			*LastChr (LastDir) = 0;
				 	  		pLastDir=_fstrrchr (LastDir,'\\');
				 	  		if (pLastDir)
				 	  			pLastDir++;
				 	  		else
				 	  			pLastDir = _fstrchr (LastDir,0);
				 	  		//sprintf (str,"\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s%s\",\"%s\",\"%i\",\"\"",str2,Name,drive,dir,pLastDir,drive,dir,extension);
				 	  		sprintf (str,"%s\t%s\t%s\t%s\t%s\t%s%s\t%s\t%s\t",str2,Name,drive,dir,pLastDir,drive,dir,extension,timesAndLength);
				 	  		fputstring (str,OutFileFID);
				 	  	}
				 	  }  
                      GSSiClose2 (&Fid);
	         	 }
	         	 GSSiClose2 (&OutFileFID);
            	 GSSiRemove (TempName);
				 GSSiSetCursor(hcurSave);
                 EndDialog(hWndDlg, TRUE); 
                 
                 break;      
            }

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (921);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (921);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL SHOWPOLYMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (827);
#endif
{ 

	char	str[32];
	static	short	nPnts, AtPoint;
	LPDPOINT	lpDpoint;
    LPMNMXCORD lpRect;
    static	BOOL	SaveDisplayMarkers; 
    static	HANDLE	hPoly;
    static	short	Delay=100, inc, nInPoly;
    static	BOOL	Active=FALSE; 
    static	LPVIEWPORT SaveVP;
	
 switch(Message)
   {
    case WM_INITDIALOG: 
    	 if (Active) 
{
#if ENABLETRACE
GSSiExitProg (827);
#endif
    	 	return FALSE; 
}
    	 SaveVP = CurView;
    	 Active=TRUE;
		 SetDlgItemText (hWndDlg,IDC_POINT_NUMBER,""); 
		 AtPoint = -1;
         SaveDisplayMarkers = DisplayMarkers;
		 DisplayMarkers=TRUE;
		 hPoly = hSavePoly;
		 hSavePoly = 0;
         nPnts = nSavePoly; 
         nInPoly = nPnts;
         SetScrollRange (GetDlgItem (hWndDlg,IDC_SCROLLPOINT),SB_CTL,0,nPnts,FALSE);
         SetScrollPos (GetDlgItem (hWndDlg,IDC_SCROLLPOINT),SB_CTL,0,TRUE);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

	case WM_HSCROLL:
		switch (wParam)
		{
		  case SB_LINEDOWN: 
		  inc = 1;
		  break;

		  case SB_LINEUP:
		  inc = -1;
	  	  break;

	  	  case SB_THUMBPOSITION:
	  	  AtPoint = LOWORD(lParam);
		  break;  
		  
		  default:
{
#if ENABLETRACE
GSSiExitProg (827);
#endif
		  	return FALSE;
}
		}
		goto DisplayPoint;
		
	   	case WM_TIMER:
	   		if (AtPoint+1 >= nPnts)
	   		{
	   			SetDlgItemText (hWndDlg,IDC_GOSTOP,"Go");
				KillTimer(hWndDlg,1);
	   			break;
	   		}
DisplayPoint: 
			GetDlgItemText (hWndDlg,IDC_POINT_NUMBER,str,sizeof(str));
			AtPoint = atoi (str);
		 	{   
		 		SetCurView ( SaveVP);
	            nPnts = nInPoly; 
	            lpRect = (LPMNMXCORD) GlobalLock (hPoly);
	            lpRect++;
	            lpDpoint = (LPDPOINT) lpRect;
				if (AtPoint >= 0)
	            {  
		            lpDpoint += AtPoint; 
			        SetScrollPos (GetDlgItem (hWndDlg,IDC_SCROLLPOINT),SB_CTL,(int)AtPoint,TRUE);
					DisplayMarker(*lpDpoint,2,0,0,0,RGB(255,0,0),TRUE,FALSE,0,0,0,0,0);
				} 
	            AtPoint+=inc;
				AtPoint = min (max (0,AtPoint),nPnts-1);
	            itoa (AtPoint,str,10);
	            SetDlgItemText (hWndDlg,IDC_POINT_NUMBER,str);
				GlobalUnlock (hPoly); 
	     	}
            break;  
    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
         	case IDOK:
            case IDCANCEL:    
            	GSSiGlobUlFree (&hPoly);
            	DisplayMarkers = SaveDisplayMarkers; 
				DestroyWindow(hWndDlg);  
				Active=FALSE;
//                EndDialog(hWndDlg, FALSE);   
                break;
            case IDC_GOSTOP: 
            	GetDlgItemText (hWndDlg,IDC_GOSTOP,str,sizeof(str));
            	if (!_fstricmp (str,"Go"))  
            	{
            	    SetDlgItemText (hWndDlg,IDC_GOSTOP,"Stop");
	            	inc = 1;
			  		SetTimer(hWndDlg,1, Delay, (TIMERPROC) 0);
            	}
            	else
            	{
	      	  		KillTimer(hWndDlg,1);
            	    SetDlgItemText (hWndDlg,IDC_GOSTOP,"Go");
            	}
            	break;
            	
         }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (827);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (827);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL TRAVERSE_ENTRYMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 	int		st, choice, n, nItems;
	int		TabStopsAddLeg[2]={9000,9100};
	long	Refno; 
	char	str[256]; 
	char	SymName[36];  
	HANDLE	hDB;
    LPGWDHEADER lpGWDHead; 
    LPGWFLDINFO lpGWFldInfo; 
    long	Offset;   
    LPSTR	pTAB; 
//	static	short	symopt=0, lsymopt; 
	static	char	CurTAG[10]="", CurUDI[34]="";  
	LPTRAVIDDATA	pTravIDData;
	LPTRAVLEGDATA	pLegData;    
	DPOINT			Points[3],POB; 
	HANDLE	hExpandedPoints;
	int		nExpandedPoints;   
	LPVIEWPORT	SaveVP=CurView;  
	RECT	TravWndRect; 
	BOOL	MustClose;
	
	
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
	{    
    	 UndoAddWindow (hWndDlg);
 		 ClearHighlightList (FALSE); 
 		 TravDest = DEST_TEST;
      	 SetGlobalValueBool ("%USEFULLMAP",FALSE);
 		 SendDlgItemMessage (hWndDlg,IDC_AUTOLINES,BM_SETCHECK,TRUE,0L);
 		 SendDlgItemMessage (hWndDlg,IDC_SHOWNODES,BM_SETCHECK,FALSE,0L);
  
//		 CurTravID = 0;  
//		 *CurUDI = 0; 
		 GSSiGlobFree (&hSnappedPoints);
		 GSSiGlobFree (&hTravPoints);
		 GSSiGlobFree (&hLegData);
		 nLegs = nTravPoints=0;
		 SetDlgItemTextGlobal (hWndDlg,IDC_SNAPDIST,"[%TRAVSNAPDIST]","");
// 		 SendDlgItemMessage (hWndDlg,IDC_TESTMODE,BM_SETCHECK,UseTestMode,0L);
//		 SendDlgItemMessage (hWndDlg,IDC_FORCECLOSE,BM_SETCHECK,TRUE,0L);
       	 SendDlgItemMessage (hWndDlg,IDC_ADDLEG,LB_SETTABSTOPS,2,(LPARAM)TabStopsAddLeg);
       	 SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETTABSTOPS,2,(LPARAM)TabStopsAddLeg);
		 LoadTravAddLegOptions (hWndDlg,IDC_ADDLEG,1);  
		 if (!LoadTravTAGs (hWndDlg,IDC_TAPREFIX))
		 {
			PostMessage(hWndDlg, WM_CLOSE, 0, 0L);
		 	break;
		 }
		SendDlgItemMessage (hWndDlg,IDC_TAPREFIX,CB_SELECTSTRING,-1,(LPARAM)CurTAG);  
		SetDlgItemText (hWndDlg,IDC_UDI,CurUDI); 
//        FillCBList (hWndDlg,IDC_SYMBOL,"[%DL]travsyms.txt",symopt,0);
//        FillCBList (hWndDlg,IDC_SYMBOLLINE,"[%DL]travsyml.txt",lsymopt,0);
		SetDlgItemTextGlobal (hWndDlg,IDC_TAPREFIX,"[%TRAVPREFIX]",0);
		SetDlgItemTextGlobal (hWndDlg,IDC_UDI,"[%TRAVUDI]",0);
//		SetDlgItemTextGlobal (hWndDlg,IDC_SYMBOL,"[%TRAVSYM]",0);
//		SetDlgItemTextGlobal (hWndDlg,IDC_SYMBOLLINE,"[%TRAVSYML]",0);  
		if (SetConfig (0))
		{   
			RECT	ClientRect;
			
			SetViewport(*pCommandViewport);
			ClientRect  = CurView->Rect;
			ClientRectToScreenRect (CurView->hWnd,&ClientRect);
			SetWindowPos(hWndDlg,HWND_TOP,ClientRect.left+1,ClientRect.top+1,0,0,SWP_NOSIZE|SWP_NOZORDER);
        }
        CurView = SaveVP;
        SetConfig (1);
		SetFocus (GetDlgItem(hWndDlg,IDC_UDI)); 
		if (GetDlgItemText (hWndDlg,IDC_UDI,str,64))
		{
			ShowTrav (hWndDlg,IDC_TRAVLIST,CurTravID);
		} 
//		if (TravDest == DEST_TEST) 
	 	GetWindowRect(hWndDlg, &TravWndRect); 
		SetGlobalValueRect ("%TRAVRECT",TravWndRect);
		PostMessage(hWndDlg, WM_COMMAND, IDC_TESTMODE, 0L);
//		if (SendDlgItemMessage (hWndDlg,IDC_AUTOLINES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
//			PostMessage(hWndDlg, WM_COMMAND, IDC_CREATELINES, 0L);  
	} 

        break; /* End of WM_INITDIALOG                                 */
    case WM_SIZE:     /*  code for sizing client area                   */  
         switch (wParam)
           {
            case SIZE_MINIMIZED: 
            	SetGlobalValueBool ("%USEFULLMAP",TRUE);
                 break;
			case SIZE_MAXIMIZED:
            case SIZE_RESTORED:  
            	SetGlobalValueBool ("%USEFULLMAP",FALSE);
           		 break;
            default: 
                 break;
           }
		    sprintf (str,"[C]=$MACRO([%%DL]macros\\travmode.txt,%i)",4); 
		    ExpandText (str);
         break;

	case GF_UNDOCOMPLETED:
		 ShowTrav (hWndDlg,IDC_TRAVLIST,CurTravID);
		 break;
		 
    case WM_CLOSE: 
		 CloseTRANS2 (&hTravTran);
		 GSSiGlobFree (&hTravPoints);
		 GSSiGlobFree (&hLegData);
		 GSSiGlobFree (&hSnappedPoints);
		 GSSiGlobFree (&hPointType);
		 GSSiGlobFree (&hSnapStatus);
    	 GetDlgItemText (hWndDlg,IDC_UDI,CurUDI,sizeof(CurUDI)-1);  
    	 GetDlgItemText (hWndDlg,IDC_TAPREFIX,CurTAG,9);   
 		 GetDlgItemTextGlobal (hWndDlg,IDC_SNAPDIST,"[%TRAVSNAPDIST]");
 		 GetDlgItemTextGlobal (hWndDlg,IDC_TAPREFIX,"[%TRAVPREFIX]");
 		 GetDlgItemTextGlobal (hWndDlg,IDC_UDI,"[%TRAVUDI]");
//		 GetDlgItemTextGlobal (hWndDlg,IDC_SYMBOL,"[%TRAVSYM]");
//		 GetDlgItemTextGlobal (hWndDlg,IDC_SYMBOLLINE,"[%TRAVSYML]");
//		 UseTestMode = SendDlgItemMessage (hWndDlg,IDC_TESTMODE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L); 

    	 if (CurTravID)
    	 	SaveTrav (CurTravID,hWndDlg,IDC_TRAVLIST);  
//	     symopt =(short)SendDlgItemMessage(hWndDlg,IDC_SYMBOL,CB_GETCURSEL,0,0); 
//	     lsymopt =(short)SendDlgItemMessage(hWndDlg,IDC_SYMBOLLINE,CB_GETCURSEL,0,0);    
	     DestroyWindow(hWndDlg); 
       	 SetGlobalValueBool ("%USEFULLMAP",TRUE);
	     sprintf (str,"[C]=$MACRO([%%DL]macros\\travmode.txt,%i)",1); 
	     if (*CfgName)
	     	ExpandText (str);
         hWndTraverseEntry = 0; 
		 FreeProcInstance(lpfnTRAVERSE_ENTRYMsgProc); 
     	 UndoRemoveWindow (hWndDlg);
		 if (*CfgName)
		 	RedisplayWindow ();
	     break;
	      
    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
			
			case IDOK:
				goto AddLeg;
           	case IDC_ADDLEG:
              	switch(HIWORD(wParam))
                {
		             case CBN_SELCHANGE:  
		    AddLeg:     
		    			//set CurrentAZ from previous leg if there is one
		                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCURSEL,0,0); 
		                if (choice == LB_ERR)
		                {
		                	choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCOUNT,0,0);
		                	choice--;
		                }
		                if (choice != LB_ERR)
		                {
				         	SendDlgItemMessage(hWndDlg,IDC_TRAVLIST,LB_GETTEXT,choice,(DWORD)str);
				         	DecodeTravLegData (CurTravID,choice,&CurLegData,str,Points,0);
				        }
		             	_fmemset (&CurLegData,0,sizeof(TRAVLEGDATA));
		             	choice = SendDlgItemMessage(hWndDlg,IDC_ADDLEG, LB_GETCURSEL,0,0);
		             	if (choice == LB_ERR)
		             		break; 
		             		
			         	SendDlgItemMessage(hWndDlg,IDC_ADDLEG,LB_GETTEXT,choice,(DWORD)str);
			         	pTAB = _fstrchr(str,'\t');
			         	pTAB++;
		                CurLegData.Type=atoi(pTAB);
		                if (!EditTravLegData (hWndDlg,&CurLegData))
		                	break;
						CreateUndoPoint ("Add Traverse Leg");
		                EncodeTravLegData (&CurLegData,str);  
		                if (CurLegData.Type==31)
		                	choice = -1;
		                else 
		                {
			                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCURSEL,0,0);
			                if (choice >= 0)
			                	choice++; 
			            }
						choice = SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_INSERTSTRING,choice,(LPARAM)str);  
		 		        AdjustSubTraverse (hWndDlg,IDC_TRAVLIST,choice,1);
						nItems = SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCOUNT,0,0);
						if (choice + 1 > nItems)
							choice = -1;
						if (choice < 0)
							SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETTOPINDEX,nItems-1,0);
						else
							SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETTOPINDEX,choice,0);
						SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETCURSEL,choice,0);
						SetFocus (GetDlgItem(hWndDlg,IDOK));
						ClearHighlightList (FALSE);
						if (SendDlgItemMessage (hWndDlg,IDC_AUTOLINES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
         					PostMessage(hWndDlg, WM_COMMAND, IDC_CREATELINES, 0L);  
         				else
							TravCompute (hWndDlg);
					 break;
				}
			break;
			
           	case IDC_TRAVLIST:
              	switch(HIWORD(wParam))
                {    
 		             case LBN_SELCHANGE:
						ClearHighlightList (TRUE);
 		                SetDlgItemText (hWndDlg,IDC_CALLNO,""); 
		                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCURSEL,0,0);
		                if (choice > 0) 
		                {
 		             		EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVELEG),TRUE);
 		             		EnableWindow (GetDlgItem(hWndDlg,IDC_EDLEG),TRUE); 
 		             		TravShowSelectedLeg (hWndDlg,IDC_TRAVLIST,IDC_CALLNO,choice);
 		             	}
 		             	else 
 		             	{
 		             		EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVELEG),FALSE);
 		             		EnableWindow (GetDlgItem(hWndDlg,IDC_EDLEG),FALSE); 
 		             	}
						SetFocus (GetDlgItem(hWndDlg,IDOK));
 		             	break; 
		             case LBN_DBLCLK: 
         			 	PostMessage(hWndDlg, WM_COMMAND, IDC_EDLEG, 0L);
					 break;
				}
			break;    
			
			case IDC_EDLEG:
                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCURSEL,0,0);
	         	SendDlgItemMessage(hWndDlg,IDC_TRAVLIST,LB_GETTEXT,choice,(DWORD)str);
	         	DecodeTravLegData (CurTravID,choice,&CurLegData,str,Points,0);
                if (!EditTravLegData (hWndDlg,&CurLegData))
                	break;
				CreateUndoPoint ("Edit Traverse Leg");
                EncodeTravLegData (&CurLegData,str);
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_DELETESTRING,choice,(LPARAM)str);
				choice=SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_INSERTSTRING,choice,(LPARAM)str);
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETCURSEL,choice,0); 
				SetFocus (GetDlgItem(hWndDlg,IDOK));
				if (SendDlgItemMessage (hWndDlg,IDC_AUTOLINES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
 					PostMessage(hWndDlg, WM_COMMAND, IDC_CREATELINES, 0L);  
 				else
 					TravCompute (hWndDlg);

			    break;
			    
			case IDC_TRAVCOMPUTE: 
				TravCompute (hWndDlg);
			break;             
			
			case IDC_NEXT:
				EnableWindow (GetDlgItem(hWndDlg,IDC_NEXT),TRUE);
		        choice=(short)SendDlgItemMessage(hWndDlg,IDC_UDI, CB_GETCURSEL,0,0); 
		        if (choice == CB_ERR)
		        	break;  
		        if (SendDlgItemMessage(hWndDlg,IDC_UDI, CB_SETCURSEL,choice+1,0) != CB_ERR)
					goto ShowNext;
				break;
					
        	case IDC_UDI:
        	{
              switch(HIWORD(wParam))
              {
	             case CBN_SELCHANGE:
	             case CBN_DBLCLK:   
	       ShowNext:
		            choice=(short)SendDlgItemMessage(hWndDlg,IDC_UDI, CB_GETCURSEL,0,0);
		            SendDlgItemMessage(hWndDlg,IDC_UDI,CB_GETLBTEXT,choice, (LPARAM)CurUDI); 
                	CurTravID = SendDlgItemMessage(hWndDlg,IDC_UDI,CB_GETITEMDATA,choice, (LPARAM) 0); 
           ShowTrav:
		 			ClearHighlightList (FALSE);
           			ShowTrav (hWndDlg,IDC_TRAVLIST,CurTravID);
					//EnableWindow (GetDlgItem(hWndDlg,IDC_ADDLEG),TRUE);
					LoadTravAddLegOptions (hWndDlg,IDC_ADDLEG,1);
				    EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVELEG),FALSE);
	    	 		GetDlgItemText (hWndDlg,IDC_TAPREFIX,CurTAG,9);  
/*	                if (PickByRefno(0,CurTAG,CurUDI,-1)) 
	                {
				 		SetViewport (*pCommandViewport);
				 		if (SendDlgItemMessage (hWndDlg,IDC_SHOWNODES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
				 			ShowNodesRef = PickList[0].Refno;
				 		else
				 			ShowNodesRef=LONG_MAX; 
				 		ClearSpecial ();
				 		CreateSpecial ();
 						AddSpecial (PickList[0].Refno,1,RGB(255,255,0),3);
	                	SetWindowText (hWndDlg,"Traverse Entry");
			    		ZoomToPickedItem(0,100,TRUE,TRUE,FALSE); 
			    	}
			    	else
			    		SetWindowText (hWndDlg,"Not located");*/
					TravDest = DEST_TEST; 
					RemoveTravTranFile ();
					TravCompute (hWndDlg);
	         		PostMessage(hWndDlg, WM_COMMAND, IDC_TESTMODE, 0L);
					RedisplayWindow ();
/*					if (SendDlgItemMessage (hWndDlg,IDC_AUTOLINES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
     					PostMessage(hWndDlg, WM_COMMAND, IDC_CREATELINES, 0L);  
     				else*/
              		break;
              	 case CBN_DROPDOWN: 
              	 {
              	 	short	n;
					hDB = OpenGWDatabase ("[%TRAVIDDB]",BT_WRITE);
					if (!hDB)
					{   
						MessageBox(GetFocus(),"Unable to open traverse database", 0,MB_ICONEXCLAMATION|MB_OK);
						break; 
					}
					lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
					pTravIDData = (LPTRAVIDDATA)&lpGWDHead->GWDData;
				    lpGWFldInfo=lpGWDHead->pFldInfo; 
				    lpGWFldInfo++;     
			    	n=0;//GetDlgItemText (hWndDlg,IDC_UDI,CurUDI,33);  
	    	 		GetDlgItemText (hWndDlg,IDC_TAPREFIX,CurTAG,9);  
					SetFieldValFromChar(lpGWDHead, lpGWFldInfo, CurTAG, FALSE, FALSE, TRUE);
				    lpGWFldInfo++;     
					SetFieldValFromChar(lpGWDHead, lpGWFldInfo, "", FALSE, FALSE, TRUE);
	    			GWDFormKey(lpGWDHead,1,TRUE,0,0);
			        SendDlgItemMessage (hWndDlg,IDC_UDI,CB_RESETCONTENT,0,0);
	    			st = BT_FIND (lpGWDHead->BTHandle[1],lpGWDHead->pKeys[1],BT_FIRST,BT_GE, (LPSTR)&Offset);
					while (!st)
					{   
						FillGWDData (lpGWDHead,Offset); 
						if (_fstricmp (CurTAG,pTravIDData->Prefix))
							break;
						if (n && _fstrnicmp (pTravIDData->UDI,CurUDI,n))
							break; 
						strncpy0 (str,(LPSTR)(lpGWDHead->pKeys[1]+8),32);
						choice = SendDlgItemMessage (hWndDlg,IDC_UDI,CB_ADDSTRING,0,(LPARAM)str);  
	         			SendDlgItemMessage (hWndDlg,IDC_UDI,CB_SETITEMDATA,(WPARAM)choice,(LPARAM)pTravIDData->ID);
	    				st = BT_FIND (lpGWDHead->BTHandle[1],lpGWDHead->pKeys[1],BT_NEXT,BT_ANY, (LPSTR)&Offset);
					}   
					SendDlgItemMessage (hWndDlg,IDC_UDI,CB_SELECTSTRING,-1,(LPARAM)CurUDI);
					GlobalUnlock (hDB);  
					CloseGWDatabase (hDB);   
              	  	break;      
              	  }
            	}
            	break;
            }
             
            
			case IDC_NEWTRAV:  
				if (CurTravID)
					SaveTrav (CurTravID,hWndDlg,IDC_TRAVLIST);  
				if (!HaveTravStartPoint)
                { 
					if (MessageBox (GetFocus(),"Start point not set - Use assumed start point?","No Start Point",MB_YESNO|MB_ICONQUESTION) == IDYES)
					{
						GetGlobalPVal ("[%ASSUMEDSTARTPOINT]",0,&TravStartPoint);
					}
					else
					{
                		MessageBox(GetFocus(),"Start point not set", 0,MB_ICONEXCLAMATION|MB_OK);
                		break;
                	}
                } 
				hDB = OpenGWDatabase ("[%TRAVIDDB]",BT_WRITE);
				if (!hDB)
				{   
                	MessageBox(GetFocus(),"Unable to open traverse database", 0,MB_ICONEXCLAMATION|MB_OK);
					break; 
				}
		        SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_RESETCONTENT,0,0);
		    	GetDlgItemText (hWndDlg,IDC_UDI,CurUDI,33);  
    	 		GetDlgItemText (hWndDlg,IDC_TAPREFIX,CurTAG,9);  
/*			    GetDlgItemText (hWndDlg,IDC_SYMBOL,SymName,34);
			    if (!_fstricmp (SymName,"DEFAULT")) */
			    TravGetDefaultSymbol (CurTAG,SymName,1); 
				lpGWDHead = (LPGWDHEADER)GlobalLock (hDB); 
			    lpGWFldInfo=lpGWDHead->pFldInfo; 
			    lpGWFldInfo++;     
				SetFieldValFromChar(lpGWDHead, lpGWFldInfo, CurTAG, FALSE, FALSE, TRUE);
			    lpGWFldInfo++;     
				SetFieldValFromChar(lpGWDHead, lpGWFldInfo, CurUDI, FALSE, FALSE, TRUE);
    			GWDFormKey(lpGWDHead,1,TRUE,0,0);
    			if (!BT_FIND (lpGWDHead->BTHandle[1],lpGWDHead->pKeys[1],BT_FIRST,BT_EQ, (LPSTR)&Offset))
				{   
                	MessageBox(GetFocus(),"This traverse already exists", 0,MB_ICONEXCLAMATION|MB_OK);
					GlobalUnlock (hDB);  
					CloseGWDatabase (hDB);   
					break; 
				}
				CreateUndoPoint ("Create New Traverse");
    			if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&CurTravID,BT_LAST,BT_ANY, (LPSTR)&Offset))
    				CurTravID++;
    			else
    				CurTravID = 1;
				pTravIDData = (LPTRAVIDDATA)&lpGWDHead->GWDData;
				pTravIDData->ID = CurTravID;
			    lpGWFldInfo=lpGWDHead->pFldInfo; 
			    lpGWFldInfo++;     
				SetFieldValFromChar(lpGWDHead, lpGWFldInfo, CurTAG, FALSE, FALSE, TRUE);
			    lpGWFldInfo++;     
				SetFieldValFromChar(lpGWDHead, lpGWFldInfo, CurUDI, FALSE, FALSE, TRUE);
			    lpGWFldInfo++;     
				SetFieldValFromChar(lpGWDHead, lpGWFldInfo, SymName, FALSE, FALSE, TRUE);
				GWDReplaceRecord (lpGWDHead,0,0,-1);
				GlobalUnlock (hDB);  
				CloseGWDatabase (hDB);   
	            EnlargeScreen (0,0);
                sprintf (str,"Starting at %.4lf %.4lf\t0",TravStartPoint.x,TravStartPoint.y); 
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_ADDSTRING,0,(LPARAM)str);
				EnableWindow (GetDlgItem(hWndDlg,IDC_POB),TRUE);
				//EnableWindow (GetDlgItem(hWndDlg,IDC_ADDLEG),TRUE);
				LoadTravAddLegOptions (hWndDlg,IDC_ADDLEG,1);
	            EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVELEG),FALSE); 
				TravDest = DEST_TEST;
				HaveClosure = FALSE; 
				RemoveTravTranFile ();
//				if (SendDlgItemMessage (hWndDlg,IDC_AUTOLINES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
// 					PostMessage(hWndDlg, WM_COMMAND, IDC_CREATELINES, 0L);  
				SetFocus (hWndDlg);
				SetFocus (GetDlgItem(hWndDlg,IDOK));
         		PostMessage(hWndDlg, WM_COMMAND, IDC_TESTMODE, 0L);

			break;
			
			case IDC_TRAVDELETE: 
				GetDlgItemText (hWndDlg,IDC_UDI,CurUDI,33);
				sprintf (str,"Do you really wish to delete the traverse for %s?",CurUDI);
				if (MessageBox (GetFocus(),str,"Verify Delete",MB_YESNO|MB_ICONQUESTION) != IDYES)
					break;
				CreateUndoPoint ("Delete Traverse");
			break;
			
//			case IDC_FORCECLOSE:
//         		PostMessage(hWndDlg, WM_COMMAND, IDC_TRAVCOMPUTE, 0L);
//			break;
			
			case IDC_STARTPOINT:
				if (!HaveTravStartPoint) 
				{
					if (MessageBox (GetFocus(),"Start point not set - Use assumed start point?","No Start Point",MB_YESNO|MB_ICONQUESTION) == IDYES)
					{
						GetGlobalPVal ("[%ASSUMEDSTARTPOINT]",0,&TravStartPoint);
					}
					else
					{
                		MessageBox(GetFocus(),"Start point not set", 0,MB_ICONEXCLAMATION|MB_OK);
                		break;
                	} 
                }
				CreateUndoPoint ("Set Traverse Start Point");
	            EnlargeScreen (0,0);
                sprintf (str,"Starting at %.4lf %.4lf\t0",TravStartPoint.x,TravStartPoint.y); 
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_DELETESTRING,0,0L);
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_INSERTSTRING,0,(LPARAM)str);
				TravCompute (hWndDlg);
				SetFocus (GetDlgItem(hWndDlg,IDOK));
				break;   
				
			case IDC_REMOVELEG:
				CreateUndoPoint ("Remove Traverse Leg");
                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCURSEL,0,0);
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_DELETESTRING,choice,(LPARAM)str);
 		        EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVELEG),FALSE);
 		        AdjustSubTraverse (hWndDlg,IDC_TRAVLIST,choice,-1);
				SetFocus (GetDlgItem(hWndDlg,IDOK));
				if (SendDlgItemMessage (hWndDlg,IDC_AUTOLINES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
 					PostMessage(hWndDlg, WM_COMMAND, IDC_CREATELINES, 0L);  
 				else
					TravCompute (hWndDlg);
          		break;
				
			case IDC_POB:
				CreateUndoPoint ("Set Traverse POB");
                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCURSEL,0,0);
                if (choice >= 0)
                	choice++;
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_INSERTSTRING,choice,(LPARAM)"*** Point of Beginning ***\t1");
				nItems = SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCOUNT,0,0);
				if (choice + 1 > nItems)
					choice = -1;
				if (choice < 0)
					SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETTOPINDEX,nItems-1,0);
				else
					SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETTOPINDEX,choice,0);
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_SETCURSEL,choice,0);
				SetFocus (GetDlgItem(hWndDlg,IDOK));
				TravCompute (hWndDlg);
			break;
			     
			case IDC_CLOSETRAV:
				SendDlgItemMessage (hWndDlg,IDC_TRAVLIST,LB_INSERTSTRING,-1,(LPARAM)"*** Close Traverse ***\t3");
				TravCompute (hWndDlg);
			break;
			     
		 	case IDC_GENERATE: 
		 	{    
		 		short	SymNum=0, NumSyms=0,PickFile; 
		 		HANDLE	hSymDesc=0; 
		 		long	Refno;
		 		double	SnapDist; 
		 		HCURSOR	hcurSave; 
		 		
		    	if (CurTravID)
		    	 	SaveTrav (CurTravID,hWndDlg,IDC_TRAVLIST);  
		 		if (!nTravPoints)
		 			break; 
				if (lParam != 1)
					TravCompute (hWndDlg);
				hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
				if (!CurrentConfig)
					SetConfig (1);
		 		SetViewport (*pCommandViewport);
		   		if (!CurView->UpdateFile)
		   		{
			 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
		            break;
		   		} 
		   		else
		   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
   	            
   	            if (!ExistFile(EditName))
   	            	copyfile (EditName,"[%NULLMAP]",FALSE,0,0,0,0,0,0);

		    	GetDlgItemText (hWndDlg,IDC_UDI,CurUDI,33); 
	    		SetGlobalValue ("%TRAVUDI",CurUDI);
		    	GetDlgItemText (hWndDlg,IDC_TAPREFIX,CurTAG,9); 
		    	SetGlobalValue ("%TRAVPREFIX",CurTAG);
   	 		    if (lParam != 1)
   	 		    {
   	 		    	GetDlgItemText (hWndDlg,IDC_SNAPDIST,str,64);
   	 		    	SnapDist = atof (str) * FTM;
   	 		    }
   	 		    else
   	 		    	SnapDist = 0;
			    TravGetDefaultSymbol (CurTAG,SymName,1);  
   		    	SetGlobalValue ("%TRAVSYMBOL",SymName);
/*			    symopt =(short)SendDlgItemMessage(hWndDlg,IDC_SYMBOL,CB_GETCURSEL,0,0); 
			    if (symopt>=0)
			    {
				    GetDlgItemText (hWndDlg,IDC_SYMBOL,SymName,34);*/ 
	                SymNum = GetOrCreateSym (hWndDlg,SymName,&NumSyms,&hSymDesc,FALSE,0);
	                if (!SymNum)
	                { 
	                	MessageBox(GetFocus(),"Symbol not found", SymName,MB_ICONEXCLAMATION|MB_OK);
	                	break;
	                } 
/*	            }
	            else  
		   		{
			 		MessageBox(GetFocus(), "Symbol not set", 0,MB_ICONEXCLAMATION|MB_OK);
		            break;
		   		} */
/*                if (SendDlgItemMessage (hWndDlg,IDC_FORCECLOSE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
                {
                	HPDPOINT	pPoint1=GlobalLock (hTravPoints);
                	HPDPOINT	pPointLast = pPoint1 + (nTravPoints-1);
                	
                	*pPointLast = *pPoint1;
                	GlobalUnlock (hTravPoints);
                }*/  
				CreateUndoPoint ("Generate Traverse Area");
				PickFile = GetPickFile (-1);
				if (PickByRefno(0,CurTAG,CurUDI,PickFile))
				{				 
					Refno = PickList[0].Refno;
					DeletePickedItem (0,12,92); 
				}
				else
					Refno = GetNewRefno (EditName,0,0,0,0);
				DoNotPickThisRefno=Refno;
				GSSiGlobFree (&hSnappedPoints);
				hSnappedPoints = SnapTravPoints (hTravPoints,hSnapStatus,nTravPoints,SnapDist,HaveClosure);
//					(BOOL)SendDlgItemMessage (hWndDlg,IDC_FORCECLOSE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L));   
				SaveTravTransformation (hTravPoints,hSnappedPoints,nTravPoints);

				DoNotPickThisRefno=LONG_MAX;
				hExpandedPoints = ExpandTravPoints (hSnappedPoints,hPointType,nTravPoints,&nExpandedPoints);
				AddToSymList (SymNum,&NumSyms,&hSymDesc); 
	    		_fstrcpy (PltName,EditName);
	    		PltType = 2;
				OpenMap (CurView->hWnd,CurView->hDC);
				EditBounds = CurView->FileMNMX; 
				CloseMap (FALSE);
				AddPolyToMap (1,&nExpandedPoints, &hExpandedPoints,0,Refno,0,-1,SymNum,0,CurTAG,CurUDI,-1,-1,-1,0,0,0,0,TRUE,0);
				GSSiGlobFree (&hExpandedPoints);
			    CloseMap(TRUE);  
				AddSymToMap (NumSyms,hSymDesc,0,0); 
	            DestroySymList (&NumSyms,&hSymDesc); 
			    SelectVisList (FALSE);
				if (!GetVisibility(SymNum))
					ToggleVisibility (SymNum);
				GSSiSetCursor (hcurSave);    
	             
                if (PickByRefno(Refno,0,0,PickFile))
                {
			 		SetViewport (*pCommandViewport);
			 		if (SendDlgItemMessage (hWndDlg,IDC_SHOWNODES,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
			 			ShowNodesRef = PickList[0].Refno;
			 		else
			 			ShowNodesRef=LONG_MAX; 
			 		ClearSpecial ();
			 		CreateSpecial ();
					AddSpecial (PickList[0].Refno,2,RGB(255,255,0),3);
                	SetWindowText (hWndDlg,"Traverse Entry");
		    		ZoomToPickedItem(0,0,TRUE,TRUE,FALSE); 
		    	} 
		    	else
					ZoomToRect(TravBounds,FALSE);
	 			EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_CREATEBOUNDARYLINES),TRUE);  
//				if (TravDest != DEST_TEST && MinimizeWindowIfOverMain (hWndDlg))
//		    		RedisplayWindow ();
			    SetFocus (hWndDlg);
	            
			}
		 	     break; 
		 	case IDC_TRAVDEST:
		 	{   
                switch (HIWORD(wParam))
                {      
		            case CBN_SELCHANGE:  
                     	choice=(short)SendDlgItemMessage(hWndDlg,IDC_TRAVDEST, CB_GETCURSEL,0,0);
                     	SendDlgItemMessage(hWndDlg ,IDC_TRAVDEST,CB_GETLBTEXT,choice,(LPARAM)str); 
				 		if (!_fstricmp (str,"Test Mode"))
				 			TravDest = DEST_TEST;
				 		else if (!_fstricmp (str,"Update Layer"))
				 			TravDest = DEST_UPDATE;
				 		else if (!_fstricmp (str,"Working Layer"))
				 			TravDest = DEST_WORKING;  
						PostMessage(hWndDlg, WM_COMMAND, IDC_TESTMODE, 0L);
				 	default:
				 		break;
				 }
		 	}
		 		break;
		 	
		 	case IDC_TESTMODE:
		 	{
//		 		short	icfg=1; 
			    HaltMapDisplay (FALSE,TRUE);	
//		 		SetViewport (*pCommandViewport);
//		 		RezoomRect = CurView->WBounds;
//				if (SendDlgItemMessage (hWndDlg,IDC_TESTMODE,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L)) 
//			        icfg = 2;
		        SendDlgItemMessage (hWndDlg,IDC_TRAVDEST,CB_RESETCONTENT,0,0);
//		        if (TravAllowUpdateLayer (CurTAG))
		        SendDlgItemMessage (hWndDlg,IDC_TRAVDEST,CB_ADDSTRING,0,(LPARAM)"Update Layer");
//		        else if (TravDest == DEST_UPDATE)
//		        	TravDest = DEST_TEST; 
		        SendDlgItemMessage (hWndDlg,IDC_TRAVDEST,CB_ADDSTRING,0,(LPARAM)"Test Mode");
		        SendDlgItemMessage (hWndDlg,IDC_TRAVDEST,CB_ADDSTRING,0,(LPARAM)"Working Layer");
			 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_GENERATE),FALSE);
			 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDIST),FALSE);
			 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE),FALSE);
			 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE2),FALSE);
	 			EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_CREATEBOUNDARYLINES),FALSE);
	 			EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_CREATELINES),TRUE);
		        if (TravDest == DEST_UPDATE)
		        {
	   			    TravGetDefaultSymbol (CurTAG,SymName,1); 
		        	SendDlgItemMessage (hWndDlg,IDC_TRAVDEST,CB_SELECTSTRING,-1,(LPARAM)"Update Layer");
			 		if (HaveClosure && _fstricmp (SymName,"NONE")) 
			 		{
			 			EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_GENERATE),TRUE);
					 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDIST),TRUE);
					 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE),TRUE);
					 	EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_SNAPDISTTITLE2),TRUE);
			 		}
	   			    MustClose = TravGetDefaultSymbol (CurTAG,SymName,2); 
			 		if ((HaveClosure || !MustClose) && _fstricmp (SymName,"NONE"))
			 			EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_CREATEBOUNDARYLINES),TRUE);
		 			EnableWindow (GetDlgItem(hWndTraverseEntry,IDC_CREATELINES),FALSE);
		        }
		        else if (TravDest == DEST_WORKING)  
		        	SendDlgItemMessage (hWndDlg,IDC_TRAVDEST,CB_SELECTSTRING,-1,(LPARAM)"Working Layer");
		        else
		        	SendDlgItemMessage (hWndDlg,IDC_TRAVDEST,CB_SELECTSTRING,-1,(LPARAM)"Test Mode");
			    sprintf (str,"[C]=$MACRO([%%DL]macros\\travmode.txt,%i)",TravDest); 
			    ExpandText (str);    
//		        PostMessage(hWndMain, WM_COMMAND, IDM_Z_REZOOM, 0L);
		    }  
		 		break;
			case IDC_CREATEBOUNDARYLINES:
		 	case IDC_CREATELINES:
		 	{    
		 		short	SymNum=0, NumSyms=0,PickFile, iLeg; 
		 		HANDLE	hSymDesc=0; 
		 		long	Refno;
		 		MNMXCORD	Rect;
				LPLEGDATA	pLegData; 
				short	LineType=2;
							 
		    	if (CurTravID)
		    	 	SaveTrav (CurTravID,hWndDlg,IDC_TRAVLIST);  
				TravCompute (hWndDlg);
		 		ClearHighlightList (TRUE);
		 		if (!nLegs)
		 			break; 
			/*	if (TravDest == DEST_TEST) 
			    {
			    	sprintf (str,"[C]=$MACRO([%%DL]macros\\travmode.txt,%i)",2); 
			    	ExpandText (str);
			    }*/
				if (!CurrentConfig)
					SetConfig (1);
		 		SetViewport (*pCommandViewport);
		   		if (!CurView->UpdateFile)
		   		{
			 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
		            break;
		   		} 
		   		else
		   			_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
				CloseTRANS2 (&hTravTran);
                if (TravDest != DEST_UPDATE) 
                {
                	LineType = 3;
				    sprintf (str,"[C]=$MACRO([%%DL]macros\\travmode.txt,%i)",TravDest); 
				    ExpandText (str);
                }
                else
                {     
					if (GetGlobalCVal ("[%TRAVTRANFILE]",str,0))
					{
                		_fstrcat (str,"(F,3)");
						hTravTran = LoadTranFileWithDandT (str);
                    }
                }
   			    TravGetDefaultSymbol (CurTAG,SymName,LineType); 
   		    	SetGlobalValue ("%TRAVSYMBOL",SymName);
/*			    lsymopt =(short)SendDlgItemMessage(hWndDlg,IDC_SYMBOLLINE,CB_GETCURSEL,0,0); 
			    if (lsymopt>=0)
			    {
				    GetDlgItemText (hWndDlg,IDC_SYMBOLLINE,SymName,34); */
	                SymNum = GetOrCreateSym (hWndDlg,SymName,&NumSyms,&hSymDesc,FALSE,2);
	                if (!SymNum)
	                { 
	                	MessageBox(GetFocus(),"Symbol not found", SymName,MB_ICONEXCLAMATION|MB_OK);
	                	break;
	                } 
/*	            }
	            else  
		   		{
			 		MessageBox(GetFocus(), "Symbol not set", 0,MB_ICONEXCLAMATION|MB_OK);
		            break;
		   		} */
   	            DBoundsInit (&Rect);
//				AddToSymList (SymNum,&NumSyms,&hSymDesc); 
	    		_fstrcpy (PltName,EditName);
	    		PltType = 2;

   	            DBoundsInit (&Rect);
//				AddToSymList (SymNum,&NumSyms,&hSymDesc); 
	    		_fstrcpy (PltName,EditName);
	    		PltType = 2;    
				pLegData = (LPLEGDATA)GlobalLock (hLegData); 
	    		if (TravDest == DEST_WORKING)
	    		{
					for (iLeg = 0;iLeg < nLegs; iLeg++,pLegData++)
					{   
						if (pLegData->Type < 4)
						{
							WORD	nPnts=pLegData->Type; 
							short	Type = nPnts,i;
							HANDLE	hPnts=GSSiGlobAlloc (1084,GMEM_MOVEABLE,1024);
							LPDPOINT	pPoints = (HPDPOINT)GlobalLock (hPnts);
							
							for (i=0;i<nPnts;i++)
							{
								pPoints[i] = TranPoint (&pLegData->Points[i],hTravTran);
								AddDPointToMinMax (&pPoints[i],&Rect);
	                        }
							GSSiGlobUlFree (&hPnts);
						}
					}  
        			CreateNewMap (PltName,&Rect,0,NULL,0,NULL,0,0,FALSE);
	    		} 
	    		GlobalUnlock (hLegData);
				CloseTRANS2 (&hTravTran);
				OpenMap (CurView->hWnd,CurView->hDC);
				EditBounds = CurView->FileMNMX; 
				CloseMap (FALSE); 
				pLegData = (LPLEGDATA)GlobalLock (hLegData); 
				for (iLeg = 0;iLeg < nLegs; iLeg++,pLegData++)
				{   
					if (pLegData->Type < 4)
					{
						int		nPnts=pLegData->Type; 
						short	Type = nPnts,i;
						HANDLE	hPnts=GSSiGlobAlloc (1084,GMEM_MOVEABLE,1024);
						LPDPOINT	pPoints = (HPDPOINT)GlobalLock (hPnts);
						
						for (i=0;i<nPnts;i++)
						{
							pPoints[i] = TranPoint (&pLegData->Points[i],hTravTran);
							AddDPointToMinMax (&pPoints[i],&Rect);
                        }
						GlobalUnlock (hPnts); 
						if (Type == 2)
							Type = 1; 
						if (TravDest == DEST_TEST) 
							Refno = iLeg + 1;
						else
							Refno = GetNewRefno (EditName,0,0,0,0);
						AddPolyToMap (1,&nPnts,&hPnts,Type,Refno,0,-1,SymNum,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
						GSSiGlobFree (&hPnts);
					}
					else if (pLegData->Type == 31)
					{   
						if (TravDest == DEST_UPDATE)
							break;
		   			    TravGetDefaultSymbol (CurTAG,SymName,3); 
		                SymNum = GetOrCreateSym (hWndDlg,SymName,&NumSyms,&hSymDesc,FALSE,2);
		                if (!SymNum)
		                { 
		                	MessageBox(GetFocus(),"Symbol not found", SymName,MB_ICONEXCLAMATION|MB_OK);
		                	break;
		                } 
					}
				}
				CloseTRANS2 (&hTravTran);
			    CloseMap(TRUE); 
			    GlobalUnlock (hLegData); 
				AddSymToMap (NumSyms,hSymDesc,0,0); 
	            DestroySymList (&NumSyms,&hSymDesc);  
			    CurView->CurZoomAreaRef = 0;
				SelectVisList (FALSE);
				if (!GetVisibility(SymNum))
					ToggleVisibility (SymNum);
				ZoomToRect(Rect,FALSE);   
				if (TravDest == DEST_TEST)
				{
	                choice=SendDlgItemMessage(hWndDlg,IDC_TRAVLIST, LB_GETCURSEL,0,0);
             		TravShowSelectedLeg (hWndDlg,IDC_TRAVLIST,IDC_CALLNO,choice);
				}
				SetFocus (GetDlgItem(hWndDlg,IDOK));
//				else if	(TravDest == DEST_UPDATE && MinimizeWindowIfOverMain (hWndDlg))
//		    		RedisplayWindow ();
	        }    
		 	break;  
		 	     
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
		         PostMessage(hWndDlg, WM_CLOSE, 0, 0L);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL PNPARMSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{
	
 int	BRtn;  
 RECT	Rect;   
 char	ImageFile[256], FileName[256]; 
 HDIB32	hDib;   
 static	COLORREF	Color;   
 static	RGBQUAD	ColorRGB;      
 static	short		CurPalIndex; 
 char	txt[520],latitude[32],longitude[32],distance[16],imagefile[128],line[520];
 static	short	NearColorIndex;
 short	i,bmw,bmh;
 BOOL	Error; 
 LPSTR	pTab; 
 HFILE	Fid;  
 static	BOOL	SettingsChanged;
 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
	{    
         hWndPNParms = hWndDlg;    
         SettingsChanged = FALSE;  
         LoadColorChanges ();
         if (NumColorChanges)	
			EnableWindow (GetDlgItem(hWndDlg,IDC_CANCELCHANGE),TRUE); 
         SetDlgItemTextGlobal (hWndDlg,IDC_SERVERIP,"[%SERVERIP]","192.168.0.1");
         SetDlgItemTextGlobal (hWndDlg,IDC_SERVERSOCKET,"[%SERVERSOCKET]","500");
         SetDlgItemTextGlobal (hWndDlg,IDC_SERVERPATH,"[%SERVERPATH]","\\Powerspec\\c");
         SetDlgItemTextGlobal (hWndDlg,IDC_MAPFILE,"[%IMAGEFILE_REMOTE]","c:\\testr.tif"); 
         SetDlgItemTextGlobal (hWndDlg,IDC_MAPFILEL,"[%IMAGEFILE_LOCAL]","c:\\testl.tif"); 
         SetDlgItemText (hWndDlg,IDC_LATITUDE,"45.00000");
         SetDlgItemText (hWndDlg,IDC_LONGITUDE,"-93.00000");  
         SetDlgItemText (hWndDlg,IDC_DISTANCE,"2");  
         SetDlgItemText (hWndDlg,IDC_INTENSITY,"100");  
		 SendDlgItemMessage (hWndDlg,IDC_DISPLAYRESULT,BM_SETCHECK,FALSE,0L);
         
	} 

        break; /* End of WM_INITDIALOG                                 */
    case WM_CLOSE: 
         hWndPNParms = 0; 
		 FreeProcInstance(lpfnPNPARMSMsgProc); 
	     break;
	
	case WM_PAINT:
    	 PostMessage(hWndDlg, WM_COMMAND, IDC_SHOWCOLOR, Color);
    	 return FALSE; 
		       
    case WM_COMMAND:
         switch(LOWORD(wParam))
           { 
           	case IDC_INTENSTEST: 
           	{
				short	intens = GetDlgItemInt(hWndDlg,IDC_INTENSITY,&Error,TRUE);   
                
                if (intens < 10 || intens > 100)
                	MessageBox (hWndDlg,"Intensity must be between 10 and 100 percent",0,MB_ICONEXCLAMATION);
                else
                {    
					EnableWindow (GetDlgItem(hWndDlg,IDC_CANCELCHANGE),TRUE); 
                	ColorChangesFrom[NumColorChanges].rgbReserved = 1;
                	ColorChangesTo[NumColorChanges++].rgbRed = (100 - intens);
					SetViewport(*pCommandViewport);
					_fstrcpy (ImageFile,CurView->lpFiles[0]);
					ExpandText (ImageFile); 
					ChangeImageColors (ImageFile,TRUE);  
					SaveColorChanges (hWndDlg);
					RedisplayWindow ();
				}
			}
           		break;
           	
           	case IDC_SHOWSCALE:
           	{
           		HFILE Fid=GSSiOpenFile ("[%DL]scale.txt",0,OF_READ);
           		
           		if (Fid != HFILE_ERROR)
           		{
           			fgetstring (txt,64,Fid);
           			GSSiClose2 (&Fid);
           			SetDlgItemText (hWndDlg,IDC_MAPSCALE,txt);
           		}  
           	}
           		break;
           			
           	case IDC_CANCELCHANGE:
           		NumColorChanges = max (0,NumColorChanges-1); 
				SaveColorChanges (hWndDlg); 
				if (!NumColorChanges)
					EnableWindow (GetDlgItem(hWndDlg,IDC_CANCELCHANGE),FALSE); 
				SetViewport(*pCommandViewport);
				_fstrcpy (ImageFile,CurView->lpFiles[0]);
				ExpandText (ImageFile); 
				ChangeImageColors (ImageFile,TRUE); 
				RedisplayWindow ();
           		break;
			case IDC_SHOWCOLOR:
			{   
				HWND	hWnd = GetDlgItem (hWndDlg,IDC_COLORFRAME);
				HDC		hDC = GetDC (hWnd);
				
				Color = lParam;   
				ColorRGB = RGBQUADFromCOLORREF (Color);
				GetClientRect (hWnd,&Rect);
				FillRectPoly(hDC, &Rect, Color);
				ReleaseDC (hWnd,hDC);
				break;
			} 
			
			case IDC_SERVERCONNECT: 
				GetDlgItemText (hWndDlg,IDC_SERVERIP,txt,64);
			   	SetGlobalValue("%SERVERIP",txt);
			   	UpdateGlobalFile ("[%DL]global.ini","%SERVERIP",txt); 
				GetDlgItemText (hWndDlg,IDC_SERVERPATH,txt,64);
			   	SetGlobalValue("%SERVERPATH",txt);  
			   	UpdateGlobalFile ("[%DL]global.ini","%SERVERPATH",txt); 
				GetDlgItemText (hWndDlg,IDC_SERVERSOCKET,txt,32);
			   	SetGlobalValue("%SERVERSOCKET",txt);
			   	UpdateGlobalFile ("[%DL]global.ini","%SERVERSOCKET",txt); 
			   	_fstrcpy (txt,"$TCPOPEN([%SERVERIP],[%SERVERSOCKET],SOCKET,,@$MACRO([%DL]macros\\tcpinput.txt),@$MACRO([%DL]macros\\tcpclose.txt))");
                ExpandText (txt);
                if (atob (txt)) 
                {
                	SetDlgItemText (hWndDlg,IDC_MESSAGE,"Server Opened");  
                	EnableWindow (GetDlgItem(hWndDlg,IDC_MAKEMAP),TRUE);
                	EnableWindow (GetDlgItem(hWndDlg,IDC_MAKETEXT),TRUE);
                	EnableWindow (GetDlgItem(hWndDlg,IDC_SERVERDISCONNECT),TRUE);
                	EnableWindow (GetDlgItem(hWndDlg,IDC_SERVERCONNECT),FALSE);
					_fstrcpy (txt,"$SERVERFILE(GET,[SOCKET],@[%DL]zoommac1.txt,[%DL]settings.txt)");
	                ExpandText (txt);
                }
                else
                	MessageBox (hWndDlg,"Failed to open server",0,MB_ICONEXCLAMATION);
				break;  
			
			case IDC_SERVERDISCONNECT: 
			   	_fstrcpy (txt,"$TCPCLOSE([SOCKET])");
                ExpandText (txt);
                if (atob (txt)) 
                {
                	SetDlgItemText (hWndDlg,IDC_MESSAGE,"Server Closed");
                	EnableWindow (GetDlgItem(hWndDlg,IDC_MAKEMAP),FALSE);
                	EnableWindow (GetDlgItem(hWndDlg,IDC_MAKETEXT),FALSE);
                	EnableWindow (GetDlgItem(hWndDlg,IDC_SERVERDISCONNECT),FALSE);
                	EnableWindow (GetDlgItem(hWndDlg,IDC_SERVERCONNECT),TRUE);
                }
                else
                	MessageBox (hWndDlg,"Failed to close server",0,MB_ICONEXCLAMATION);
				break;  
			
			case IDC_MAKEMAP: 
				SetViewport(*pCommandViewport);
				bmw = CurView->DrawRect.right - CurView->DrawRect.left + 1;
				bmh = CurView->DrawRect.bottom - CurView->DrawRect.top + 1;
				GetDlgItemText (hWndDlg,IDC_LONGITUDE,longitude,32);
				GetDlgItemText (hWndDlg,IDC_LATITUDE,latitude,32);
				GetDlgItemText (hWndDlg,IDC_DISTANCE,distance,16);
				GetDlgItemText (hWndDlg,IDC_MAPFILE,imagefile,128);
				GetDlgItemText (hWndDlg,IDC_MAPFILEL,txt,128);
			   	SetGlobalValue("%IMAGEFILE2",txt); 
			   	SetGlobalValue("%IMAGEFILE_LOCAL",""); 
			   	HaveSavePalette = FALSE;
       			SetDlgItemText (hWndDlg,IDC_MAPSCALE,"Waiting for map");   
       			if (SettingsChanged)
       			{   
       				HFILE	Fid = GSSiOpenFile ("[%DL]settings.txt",0,OF_READ);
       				
       				if (Fid != HFILE_ERROR)
       				{  
   						sprintf (txt,"$TCPSEND([SOCKET],$STR($DELETEFILE([%%DL]zoommac1.txt)),T)");
   						ExpandText (txt);
       					while (fgetstring (line,512,Fid))
       					{
       						sprintf (txt,"$TCPSEND([SOCKET],$STR($APPEND([%%DL]zoommac1.txt,$STR(%s))),T)",line);
       						ExpandText (txt);
       					}
       					GSSiClose2 (&Fid);
       				}
       				SettingsChanged = FALSE; 
       			}
				sprintf (txt,"$TCPSEND([SOCKET],$STR([%%ALLOWCACHE]=F;[%%ALLOWCACHE]=T;[%%APPLYCOLORCHANGES]=F;[BITMAP_WIDTH]=%i;[BITMAP_HEIGHT]=%i;[C]=$MAKEMAP(COORD,%s,%s,%s,%s,[%%DL]colormap.bin)),T)",bmw,bmh,longitude,latitude,distance,imagefile);
            	SetGlobalValueBool ("SHOWRESULT",(BOOL)SendDlgItemMessage (hWndDlg,IDC_DISPLAYRESULT,BM_GETCHECK,0,0));
				ExpandText (txt); 
				RedisplayWindow ();
				break; 
				
			case IDC_MAKETEXT: 
				GetDlgItemText (hWndDlg,IDC_LONGITUDE,longitude,32);
				GetDlgItemText (hWndDlg,IDC_LATITUDE,latitude,32);
				sprintf (txt,"$TCPSEND([SOCKET],$STR([%%PNFORMAT]=11111111111;[%%C]=$WHEREAT(%s,%s)))",longitude,latitude);
				SetGlobalValueBool ("SHOWRESULT",TRUE);
				ExpandText (txt);
				break; 
				
			case IDC_CHANGECOLOR:
       			ColorChangesFrom[NumColorChanges] = RGBQUADFromCOLORREF ((COLORREF)Color);
				if (!GetColor (hWndDlg,&Color))
					break;  
				EnableWindow (GetDlgItem(hWndDlg,IDC_CANCELCHANGE),TRUE); 
       			ColorChangesTo[NumColorChanges++] = RGBQUADFromCOLORREF ((COLORREF)Color);
				SetViewport(*pCommandViewport);
				_fstrcpy (ImageFile,CurView->lpFiles[0]);
				ExpandText (ImageFile); 
				ChangeImageColors (ImageFile,TRUE);
				EnableWindow (GetDlgItem(hWndDlg,IDC_CANCELCHANGE),TRUE); 
				SetWindowText (GetDlgItem (hWndDlg,IDC_SHOWONLYCOLOR),"Show Usage");
				SaveColorChanges (hWndDlg);
				RedisplayWindow ();
				break;
			
			case IDC_SENDTOSERVER:
       		{   
       				HFILE	Fid = GSSiOpenFile ("[%DL]colorchanges.txt",0,OF_READ);
       				
       				if (Fid != HFILE_ERROR)
       				{  
   						sprintf (txt,"$TCPSEND([SOCKET],$STR($DELETEFILE([%%DL]colorchanges.txt)),T)");
   						ExpandText (txt);
       					while (fgetstring (line,512,Fid))
       					{
       						sprintf (txt,"$TCPSEND([SOCKET],$STR($APPEND([%%DL]colorchanges.txt,$STR(%s))),T)",line);
       						ExpandText (txt);
       					}
       					GSSiClose2 (&Fid); 
       				}
       		}
				break;
				
			case IDC_EDITSETTINGS:  
				_fstrcpy (FileName,"[%DL]settings.txt");
				EditTextFile (hWndDlg,FileName);
				SettingsChanged=TRUE;
				break;
					
/*			case IDC_SHOWPALETTE:
			{   
				HWND	hWnd = GetDlgItem (hWndDlg,IDC_COLORFRAME);
				HDC		hDC = GetDC (hWnd); 
				BOOL	Error;
				short	i=GetDlgItemInt(hWndDlg,IDC_PALINDEX,&Error,TRUE);   
				COLORREF	Color2;
				
				Color2 = RGB (SavePalette[i].rgbRed,SavePalette[i].rgbGreen,SavePalette[i].rgbBlue);
				GetClientRect (hWnd,&Rect);
				FillRectPoly(hDC, &Rect, Color2);
				ReleaseDC (hWnd,hDC); 
				sprintf (txt,"%i-%i-%i",GetRValue(Color2),GetGValue(Color2),GetBValue(Color2));
				SetDlgItemText (hWndDlg,IDC_COLORSEP,txt);
				i++;    
				SetDlgItemInt(hWndDlg,IDC_PALINDEX,i,TRUE);  
				
				break;
			}*/ 
			case IDC_SHOWONLYCOLOR: 
			{   
				long	NumColors;
				RGBQUAD	PaletteRGB[256];  
				USHORT	i;
				double	colordif, maxdif=DBL_MAX;
				RGBQUAD	White=RGBQUADFromCOLORREF(RGB(255,255,255));
				
				SetViewport(*pCommandViewport);
				_fstrcpy (ImageFile,CurView->lpFiles[0]);
				ExpandText (ImageFile); 
				GetWindowText (GetDlgItem (hWndDlg,IDC_SHOWONLYCOLOR),txt,32);
				if (!_fstricmp (txt,"Restore"))
				{
					ChangeImageColors (ImageFile,TRUE); 
					SetWindowText (GetDlgItem (hWndDlg,IDC_SHOWONLYCOLOR),"Show Usage");
				}
				else
				{
					SetViewport(*pCommandViewport);
					AddBMPToCache32 (0,0);
					_fstrcpy (ImageFile,CurView->lpFiles[0]);  
					ExpandText (ImageFile);
					hDib = BMPHandleFromEXT (ImageFile);
					GetDibPalette (hDib,&NumColors,PaletteRGB);
					for (i=0;i<NumColors;i++)
					{
						colordif = RGBQUADDist  (ColorRGB,PaletteRGB[i]);
						if (colordif <= ChangeDist) 
							PaletteRGB[i] = ColorRGB;
						else
							PaletteRGB[i] = White;
					} 
					SetDibPalette (hDib,NumColors,PaletteRGB); 
					SaveDIB32 (hDib,ImageFile,0,-1);
					DestroyDIB32(hDib,TRUE); 
					SetWindowText (GetDlgItem (hWndDlg,IDC_SHOWONLYCOLOR),"Restore");
				}
				RedisplayWindow ();
            }
            	break;

            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
		         PostMessage(hWndDlg, WM_CLOSE, 0, 0L);
                 break;
           }
         break;  

    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL FAR PASCAL SELECTAVMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1107);
#endif
{   
	static	HANDLE	hSaveBM;
	char	str[280], str2[512]; 
	long	n=0; 
	HFILE	Fid;  
	long	loc;
    int     TabStops[2]={2000,2100};
    static	LPVIEWPORT	pVP;
	LPSTR pBS;
	
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG: 
        
		EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
        pVP = CurView;
		hSaveBM = EnterBlockingWindow (hWndDlg);
        SetWindowText (hWndDlg,"Select Auto Visibility File");  
        _fstrcpy (str,pVP->VisName);
        if ((pBS = _fstrrchr (str,'.')))
        	*pBS = 0;
        if ((pBS = _fstrrchr (str,'\\')))
	        SetDlgItemText (hWndDlg,IDC_LISTHEADER,pBS+1);
        SendDlgItemMessage (hWndDlg,IDC_LIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
        Fid = OpenTempNamedFile (); 
        loc = GSSillseek (Fid,0,1);  
        GetGlobalCVal ("[%AVDIR]",str,"[%DL]autovisibility");
		SearchFilesInDir (str,".txt", Fid,&n,"*.txt",-1,TRUE,TRUE); 
		GSSillseek (Fid,loc,0);
		while (fgetstring (str,278,Fid))
		{    
			pBS = _fstrrchr (str,'\\');
			if (pBS) 
				pBS++;
			else
				pBS = str;
			_fstrcpy (str2,pBS);
			if ((pBS = _fstrrchr (str2,'.')))
				*pBS = 0;
			_fstrcat (str2,"\t");
			_fstrcat (str2,str);
			SendDlgItemMessage (hWndDlg,IDC_LIST,LB_ADDSTRING,0,(LPARAM)str2);  
		}  
	  	CloseAndDeleteFile (&Fid); 
//    	n = FillList (hWndDlg,IDC_LIST,&Args[2],0,&Rect);
		
		 break;                              
    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
            break; 
            
            case IDC_LIST:
				switch(HIWORD(wParam))
				{
					case LBN_SELCHANGE: 
					{
                		short	Choice=(short)SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETCURSEL,0,0);
                
                		if (Choice == LB_ERR)
                			break;
	            		SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETTEXT,Choice,(LPARAM)str);  
			            pBS = _fstrrchr (str,'\t');
			            *pBS = 0;  
				        SetDlgItemText (hWndDlg,IDC_LISTHEADER,str);
						EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
						if (GetGlobalLVal2 ("[%USERLEV]",1) > 1)
							ShowWindow (GetDlgItem(hWndDlg,IDC_EDIT),SW_SHOW);
					}
						break;
				 	case LBN_DBLCLK:
				     	PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
				 	break;
				}
			break;
			 
            case IDOK: 
            {
                short	Choice=(short)SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETCURSEL,0,0);
                
                if (Choice == LB_ERR)
                	break;
	            SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETTEXT,Choice,(LPARAM)str);  
	            pBS = _fstrrchr (str,'\t');
	            pBS++;  
	            SubstituteDL (pBS,FALSE);
	            _fstrcpy (pVP->VisName,pBS);
                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
            }
			break;
			case IDC_EDIT:
            {
                short	Choice=(short)SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETCURSEL,0,0);
                
                if (Choice == LB_ERR)
                	break;
	            SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETTEXT,Choice,(LPARAM)str);  
	            pBS = _fstrrchr (str,'\t');
	            pBS++;  
	            SubstituteDL (pBS,FALSE);
	            GMEdit (hWndDlg,pBS);
            }
            break;    
            
         }
         break; 

    default:
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1107);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL FAR PASCAL DISPLAYNETINFOMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
	int			TabStops[3]={50,100,150};
    BOOL		Opened;
	NETLINKSKEY	NetLinksKey; 
	NETLINKSDATA	NetLinksData;
	NETREFSKEY	NetRefsKey;
	NETREFSDATA	NetRefsData;
	double		MP, PCT, MPinc;
	int			st,st2;
	char		str[256], str2[256], TrueName[34]; 
	MARKERVAL	NetMarker;  
	short		Dir, pos=BT_FIRST, cond = BT_GT;
    
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
       	 SendDlgItemMessage (hWndDlg,IDC_NET_INFO,LB_SETTABSTOPS,3,(LPARAM)&TabStops); 
         cwCenter(hWndDlg, 0);  
         if (PickList[NumPicked-1].Type == 1)
		 {
			NETINTREFKEY	NetIntRefKey;
			NETINTREFDATA	NetIntRefData; 
			long			WantID;
			   
			OpenNetIntersect (NetworkID,FALSE,&Opened);
			NetIntRefKey.IntID = WantID = atol(PickList[NumPicked-1].UDI);
			while (!BT_FIND (hBTNetIntRef,(LPSTR)&NetIntRefKey,pos,cond,(LPSTR)&NetIntRefData))
			{   
				pos = BT_NEXT; 
				cond = BT_ANY;
				if (NetIntRefKey.IntID != WantID)
					break; 
				sprintf (str,"%ld\t%i",NetIntRefKey.Refno,NetIntRefData.WhichEnd);
				SendDlgItemMessage (hWndDlg,IDC_NET_INFO,LB_ADDSTRING,0,(LPARAM)str); 
			} 
			CloseNetIntersect(Opened); 
		 }
		 else
		 {
		 	 OpenNetLinkAndRef (NetworkID,FALSE,&Opened);  
		 	 NetLinksKey.Ref = PickList[NumPicked-1].Refno;
		 	 NetLinksKey.NetID = 0;
		 	 st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData); 
		 	 while (!st && NetLinksKey.Ref == PickList[NumPicked-1].Refno)
		 	 {  
				NetRefsKey.Path =  NetLinksKey.Path;
				NetRefsKey.MP = NetLinksData.MP; 
				st2 = BT_FIND (hBTNetRefs,(LPSTR)&NetRefsKey,BT_FIRST,BT_GE,(LPSTR)&NetRefsData);
				PCT = PickList[NumPicked-1].PCT;  
				if (NetRefsData.Dir == 2)
					PCT = 1.0 - PCT;
				MPinc = PCT * PickList[NumPicked-1].Length;
				NetLinksData.MP += MPinc;  
		    	GetTrueStreetName (NetLinksKey.Path,TrueName,0,0);
		    	CurPath = NetLinksKey.Path;
		 	 	sprintf (str,"%s\t%f",TrueName,NetLinksData.MP);
				if (GetMilePointFromMP (NetLinksKey.Path,1,NetLinksData.MP, &NetMarker, &Dir))
					sprintf (str2,"\t%c%c%f%c%c",NetMarker.Prefix[0],NetMarker.Prefix[1],NetMarker.Value,
											   NetMarker.Suffix[0],NetMarker.Suffix[1]);
				else 
					str2[0]=0; 
				_fstrcat (str,str2);
				SendDlgItemMessage (hWndDlg,IDC_NET_INFO,LB_ADDSTRING,0,(LPARAM)str); 
		 	 	st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_NEXT,BT_ANY,(LPSTR)&NetLinksData); 
		 	 }  
		 	 CloseNetLinkAndRef (Opened);
	 	 } 
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
         	case IDOK:
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
		  }
		  break;
    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL ZOOMSCALEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1155);
#endif
{ 	int		st, Choice;
	HFILE	Fid;
	static	int	CurrentScale=-1;
 	static	int		iunits=0, iper=0; 
 	static	char	OtherScale[32]="";
	long	FIPS, Recnum;
	int		nread;
	HANDLE	hBT;
	char	str[260];
	LPSTR	lpTab;
	int		TabStops[2]={400,500};  
	static	HANDLE	hSaveBM;
	OFSTRUCTGM	OFStruct;
	double  BaseDistPerPixel, BaseDistPerInch;
	double	FeetPerInch, MilesPerInch;
	

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1155);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:  
         /* initialize working variables                                */  
    	 hSaveBM = EnterBlockingWindow (hWndDlg);
		 if (App == 1)
			 ShowWindow (GetDlgItem(hWndDlg,IDC_EDIT),SW_HIDE); 
    	 if (CurView->DesiredWidth)  
    	 {
    	 	BaseDistPerInch = (CurView->WBounds.xmx - CurView->WBounds.xmn)/CurView->DesiredWidth;
		 	MilesPerInch = ConvertDist (BaseDistPerInch,4);
    	 }
    	 else
    	 {
    	 	BaseDistPerPixel = (CurView->WBounds.xmx - CurView->WBounds.xmn)/(CurView->DrawRect.right - CurView->DrawRect.left);
		 	MilesPerInch = ConvertDist (BaseDistPerPixel * DevicePixelsPerInch,4);
		 }
		 if (MilesPerInch < 0.5)
		 {
		 	FeetPerInch = MilesPerInch * 5280;
		 	sprintf (str,"Current scale is %.2f feet/inch",FeetPerInch); 
		 }
		 else
		 {
		 	sprintf (str,"Current scale is %.4f miles/inch",MilesPerInch); 
		 }
    	 SetDlgItemText (hWndDlg,IDC_CURRENTSCALE,str);
Redisplay: 
        if (CurView->WindowZoomedToOrtho && CurView->OrthoRes > 0)
        {   
        	int	DeviceRes;
        	
        	CurrentScale = -1;
			DeviceRes = GetDeviceCaps(CurView->hDC, LOGPIXELSX);
        	sprintf (OtherScale,"%.4f",CurView->OrthoRes*DeviceRes);  
        	iper = 0;
        	iunits = PRJ_UNITS[1]-1;
        }
        if (CurView->WindowZoomedToOrtho)
        	SetDlgItemText (hWndDlg,IDC_SCALE_VAL,OtherScale);  
		SendDlgItemMessage (hWndDlg,IDC_SCALE_UNITS,CB_RESETCONTENT,0,0);
		SendDlgItemMessage (hWndDlg,IDC_SCALE_INCHORCM,CB_RESETCONTENT,0,0);
		SendDlgItemMessage (hWndDlg,IDC_SCALES,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
 		SendDlgItemMessage (hWndDlg,IDC_SCALE_UNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Feet"); 
 		SendDlgItemMessage (hWndDlg,IDC_SCALE_UNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Meters"); 
 		SendDlgItemMessage (hWndDlg,IDC_SCALE_UNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Yards"); 
 		SendDlgItemMessage (hWndDlg,IDC_SCALE_UNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Miles"); 
 		SendDlgItemMessage (hWndDlg,IDC_SCALE_UNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Kilometers"); 
 		SendDlgItemMessage (hWndDlg,IDC_SCALE_INCHORCM,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Inch"); 
 		SendDlgItemMessage (hWndDlg,IDC_SCALE_INCHORCM,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Centimeter"); 
 		SendDlgItemMessage (hWndDlg,IDC_SCALE_UNITS,CB_SETCURSEL,(WPARAM)iunits,(LPARAM)0); 
 		SendDlgItemMessage (hWndDlg,IDC_SCALE_INCHORCM,CB_SETCURSEL,(WPARAM)iper,(LPARAM)0); 
		SendDlgItemMessage (hWndDlg,IDC_SCALES,LB_RESETCONTENT,0,0);
		Fid = GSSiOpenFile ("scales.txt",&OFStruct,OF_READ);
	    if (Fid==HFILE_ERROR)
	    {
	    	char	Name[MAX_PATH]="[%DL]scales.txt";
	    	
	    	EnableWindow (GetDlgItem(hWndDlg,IDC_EDIT),FALSE);
	    	ExpandText (Name);
			Fid = GSSiOpenFile (Name,&OFStruct,OF_READ);
		    if (Fid==HFILE_ERROR) 
		    	break; 
		}
		else
	    	EnableWindow (GetDlgItem(hWndDlg,IDC_EDIT),TRUE);
		while (fgetstring (str,256,Fid))
		{   
			REPLAC (str,"|","\t",256);
			if (_fstrchr(str,'\t'))
		 		SendDlgItemMessage (hWndDlg,IDC_SCALES,LB_ADDSTRING,(WPARAM)0,(LPARAM) str); 
		} 
		GSSiClose2 (&Fid);
        if (!CurView->WindowZoomedToOrtho)
        	EnableWindow (GetDlgItem(hWndDlg,IDC_REMOVE),FALSE);
        else
 			SendDlgItemMessage (hWndDlg,IDC_SCALES,LB_SETCURSEL,(WPARAM)CurrentScale,(LPARAM)0); 
        break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDC_SCALES: /* List box                           */
                 switch(HIWORD(wParam))
                 {   
                 	 case LBN_SELCHANGE:
                 	 	 SetDlgItemText (hWndDlg,IDC_SCALE_VAL,"");
                 	 	 break;
                 	 	 
                     case LBN_DBLCLK:
				         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
		 		     break;
		 		 }
		 		 break; 
		 	case IDC_SCALE_VAL:
				switch(HIWORD(wParam))
				{   
				 case EN_CHANGE:
				 	 GetDlgItemText (hWndDlg,IDC_SCALE_VAL,str,256);
				 	 if (str[0]) 
					 	SendDlgItemMessage (hWndDlg,IDC_SCALES,LB_SETCURSEL,-1,0);				 	 
				 	 break;
				                 	 	 
				}
				break; 
		 	
		 	case IDOK:
				 GetDlgItemText (hWndDlg,IDC_SCALE_VAL,OtherScale,sizeof(OtherScale));
				 if (OtherScale[0]) 
				 {  
				 	double	val; 
				 	double	UFac[4]={FTM,1.0,5280*FTM,1000.0}; 
				 	char	CUnits[32], CPer[32], SORB, ScaleText[128];  
				 	LPSTR	lpEnd;
				 	
				 	CurrentScale = -1;
				 	val = atof (OtherScale);
			    	CurView->ScaleDistUnits=SendDlgItemMessage(hWndDlg,IDC_SCALE_UNITS,CB_GETCURSEL,0,0); 
		 		    CurView->ScaleDist = val;
					CurView->OrthoRes = -ConvertInDist (CurView->ScaleDist,CurView->ScaleDistUnits+1);
			    	iper=SendDlgItemMessage(hWndDlg,IDC_SCALE_INCHORCM,CB_GETCURSEL,0,0);
			    	if (iper)  
			    	{
			    		CurView->OrthoRes /= INCHES_PER_CM;   
						CurView->ScaleBase = 2;//cm 
					}
			    	else
						CurView->ScaleBase = 1;//inches 
			    	
			        SendDlgItemMessage(hWndDlg,IDC_SCALE_UNITS,CB_GETLBTEXT,iunits,(DWORD)&CUnits);
			        if (val == 1)
					{
						if (!iunits)
							_fstrcpy (CUnits,"Foot");
						else   
						{
							lpEnd = _fstrchr (CUnits,0);
							lpEnd--;
							*lpEnd=0;
						}
					}
			        SendDlgItemMessage(hWndDlg,IDC_SCALE_INCHORCM,CB_GETLBTEXT,iper,(DWORD)&CPer);
			    	sprintf (ScaleText,"%s %s/%s",OtherScale,CUnits,CPer);
			    	SetGlobalValue("%SCALE",ScaleText);

				 }
		 	     else
		 	     {
			    	 CurrentScale=SendDlgItemMessage(hWndDlg,IDC_SCALES,LB_GETCURSEL,0,0);
			         if (CurrentScale != LB_ERR)
			         {   
			         	 double	Dist; 
			         	 LPSTR	LastC;
			         	 
				         SendDlgItemMessage(hWndDlg,IDC_SCALES,LB_GETTEXT,CurrentScale,(DWORD)&str);
		 		         lpTab = _fstrrchr(str,'\t'); 
		 		         *lpTab = 0;
				    	 SetGlobalValue("%SCALE",str);
		 		         lpTab++;   
		 		         LastC = LastChr (lpTab); 
		 		         if (*LastC == '$')
		 		         	LastC--;
		 		         if (*LastC == 'F')
		 		         	_fstrcpy (LastC," Feet");
		 		         GetDistAndUnits (lpTab,&Dist,&CurView->ScaleDistUnits,TRUE);   
		 		         CurView->ScaleDist = Dist;
						 CurView->OrthoRes = -ConvertInDist (CurView->ScaleDist,CurView->ScaleDistUnits+1);
						 CurView->ScaleBase = 1;//inches 
/*
		 		         CurView->OrthoRes = -atof(lpTab);
		 		         if (_fstrchr(lpTab,'F'))
		 		         	CurView->OrthoRes *= FTM; */
		 		     }
		 		     else
		 		     	break;
		 		 }
	 		     CurView->WindowZoomedToOrtho=TRUE;  
				// GMEnableMenuItem(hWndMain, IDM_Z_IN, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
				// GMEnableMenuItem(hWndMain, IDM_Z_OUT, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
				// GMEnableMenuItem(hWndMain, IDM_Z_WINDOW, MF_BYCOMMAND | MF_DISABLED| MF_GRAYED);
	 		     
	             GSSiEndDialog(hWndDlg, TRUE,hSaveBM); 
		 	     break; 
		 	     
		 	case IDC_REMOVE:  
		 		 SetScale ("REMOVE");
                 GSSiEndDialog(hWndDlg, FALSE,hSaveBM); 
                 break;  
                 
            case IDC_EDIT:  
            	 _fstrcpy (str,"NOTEPAD.EXE ");
            	 _fstrcat (str,"scales.txt");
            	 WinExec (str,SW_SHOWMAXIMIZED);
                 goto Redisplay;
            	 break;
		 	     
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 GSSiEndDialog(hWndDlg, FALSE,hSaveBM); 
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1155);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1155);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL FAR PASCAL LOC_COORDMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1050);
#endif
{ 	int		st, Choice;
	static	int	CurrentScale=-1;
 	static	int		iunits=0, iper=0; 
 	static	char	OtherScale[32]="";
	long	FIPS, Recnum, Loc;
	int		nread;
	HANDLE	hBT;
	char	str[256];
	LPSTR	lpTab, pName;
	static	short	CurProj=-1,CurUnits=-1;
	OFSTRUCTGM	OFStruct;
	HFILE	Fid, Fid2;
	static	char	XVal[32]="", YVal[32]=""; 
	

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1050);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:  
         /* initialize working variables                                */  
Redisplay:
         SetDlgItemText (hWndDlg,IDC_X,XVal);
         SetDlgItemText (hWndDlg,IDC_Y,YVal);
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Decimal Degrees");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees, Decimal Minutes");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees, Minutes, Seconds");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"MGRS");
         FillProjectionList (hWndDlg,IDC_PROJECTION,&CurProj,IDC_UNITS,&CurUnits);
 		 SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SETCURSEL,(WPARAM)CurUnits,(LPARAM)0); 
 		 SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SETCURSEL,(WPARAM)CurProj,(LPARAM)0);     
 		 SetCoordTitle (hWndDlg, CurUnits);
 		 if (InSnap)
 		 	SetWindowText (hWndDlg,"Snap To Coordinate");
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDC_UNITS:  
                 switch(HIWORD(wParam))
                 {   
                 	 case LBN_SELCHANGE:
					 	CurUnits=SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_GETCURSEL,0,0); 
				 		SetCoordTitle (hWndDlg, CurUnits);
		 		     break;
		 		 }
		 		 break; 
            case IDC_PROJECTION:  
                 switch(HIWORD(wParam))
                 {   
                 	 case LBN_SELCHANGE: 
                 	 	if (!hProjectionFile)
                 	 		break;
                 	 	pName=GlobalLock (hProjectionFile);
                 	 	Fid = GSSiOpenFile (pName,0,OF_READ);
                 	 	GlobalUnlock (hProjectionFile);
                 	 	if (Fid == HFILE_ERROR)
                 	 		break;
					 	CurProj=SendDlgItemMessage(hWndDlg,IDC_PROJECTION,CB_GETCURSEL,0,0); 
					 	Loc = SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_GETITEMDATA,(WPARAM)CurProj,(LPARAM)0);  
					 	if (Loc < 0)
					 		_fstrcpy (str,"[%DL]baseproj.cvt");
					 	else
					 	{
					 		GSSillseek (Fid,Loc,0);
							fgetstring (str,140,Fid);
						}
						{    
							Fid2 = GSSiOpenFile (str,0,OF_READ);
							if (Fid2 != HFILE_ERROR)
							{   
								int	Item, TitleOpt=0;
								
								fgetstring (str,140,Fid2);
								fgetstring (str,140,Fid2); 
								GSSiClose2 (&Fid2); 
								CurUnits = UnitsFromText (&str[1]);   
								SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_SETCURSEL,CurUnits,0);     
								SetCoordTitle (hWndDlg,CurUnits);
					        }
					        Loc = GSSillseek (Fid,0,1);
						}
						GSSiClose2 (&Fid);

		 		     break;
		 		 }
		 		 break; 
		 	case IDOK: 
		 	{
		 		 char	SaveProj[MAX_PATH], project[MAX_PATH];
		 		 LPSTR	lpDot, lpEnd, pComma;
		 		 short	SaveUnits;
				 BOOL	Err=0;
		 		 
		 		 SaveUnits = PRJ_UNITS[3];
		 		 _fstrcpy (SaveProj,"[%ALT_PROJECTION]");
		 		 ExpandText (SaveProj); 
		 		 GetProjectionFile (hWndDlg,IDC_PROJECTION,project);
                 //GetDlgItemText (hWndDlg,IDC_PROJECTION,project,sizeof(project));
                 if ((lpDot=_fstrrchr(project,'.')))
                    *lpDot = 0;
                 SetGlobalValue("%ALT_PROJECTION",project);
			     ConvertCoordClose ();
				 ConvertCoordInit();
				 if (!GetDlgItemText (hWndDlg,IDC_X,XVal,sizeof(XVal)))
				 	break;
				 if ((pComma=_fstrchr (XVal,',')))
				 {
				 	*pComma++ = 0;
				 	_fstrcpy (YVal,pComma);
				 }
				 else
				 	GetDlgItemText (hWndDlg,IDC_Y,YVal,sizeof(YVal));  
                 PRJ_UNITS[3] = 1;
                 switch (CurUnits)
                 { 
	                 case 1:
	                    PRJ_UNITS[3] = 2;  
	                 case 0:
					    UserSpecifiedBasePoint.x = atof (XVal);
				 		UserSpecifiedBasePoint.y = atof (YVal);
	                    break;
	                 case 2:
	                 case 3: 
   	                 case 4:
	                 	PRJ_UNITS[3] = 4; 
					 	UserSpecifiedBasePoint.y = DecDegFromDMS (YVal,&Err); 
					 	UserSpecifiedBasePoint.x = DecDegFromDMS (XVal,&Err); 
	                 	break; 
	                 case 5:
	                 	PRJ_UNITS[3] = 4; 
						if (ConvertMGRSToGeodetic (&UserSpecifiedBasePoint.y,&UserSpecifiedBasePoint.x,XVal))
							Err = 1;
	        	 } 
	        	 if (!Err) 
	        	 {   
	        	 	 short 	rtn=ConvertCoord (&UserSpecifiedBasePoint,3,1);
	        	 	 
	                 PRJ_UNITS[3] = SaveUnits;
					 SetGlobalValue ("%ALT_PROJECTION",SaveProj);
				     ConvertCoordClose ();
					 switch (rtn)
					 {
					 	case 0:
		             		EndDialog(hWndDlg, TRUE);
		             		break;
		             	default:
		             		break;
		             }  
		         }
 
	        }
		 	     break; 
		 	     
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1050);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1050);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL LOC_LATLONGMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1051);
#endif
{	 
	short	Deg,Min,ii, iFormat;
	double	Sec, DMin,DDeg;  
	static	DPOINT	Point;
	static	FirstPoint=TRUE;
	DPOINT	GPSPoint;   
	char	str[256]; 
	BOOL	Err;    
	static	BOOL KeepOnScreen=FALSE;
	

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1051);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:  
         /* initialize working variables                                */  
		 cwCenter(hWndDlg, -2);  
 		 if (InSnap)
 		 	SetWindowText (hWndDlg,"Snap To Lat/Lon");  
 		 if (CursorIsLocked)
 		 	Point = CurrentPoint;
 		 else if (FirstPoint)
 		 	Point = MinMaxMidPointD (&CurView->WBounds);
	    FirstPoint = FALSE;
 		ConvertCoord (&Point,1,2); 
 		if (GetVehicleLoc("GPS",&GPSPoint,0))
 			EnableWindow (GetDlgItem(hWndDlg,IDC_SNAPTOGPS),TRUE); 
 		else
 			ShowWindow (GetDlgItem(hWndDlg,IDC_SNAPTOGPS),SW_HIDE); 
    case GSSI_REINITDIALOG:  
 		iFormat = GetGlobalLVal2 ("[%LATLONFORMAT]",1);  
 		switch (iFormat)
 		{   
 			case 1:
				SendDlgItemMessage (hWndDlg,IDC_FORMATDMS,BM_SETCHECK,TRUE,0L);
				SendDlgItemMessage (hWndDlg,IDC_FORMATDM,BM_SETCHECK,FALSE,0L); 
				SendDlgItemMessage (hWndDlg,IDC_FORMATDD,BM_SETCHECK,FALSE,0L);  
				ShowWindow (GetDlgItem(hWndDlg,IDC_MINLAT),TRUE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_MINLONG),TRUE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_MINLAT2),TRUE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_MINLONG2),TRUE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_SECLAT),TRUE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_SECLAT2),TRUE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_SECLONG),TRUE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_SECLONG2),TRUE);
				break;
 			case 2:
				SendDlgItemMessage (hWndDlg,IDC_FORMATDMS,BM_SETCHECK,FALSE,0L);
				SendDlgItemMessage (hWndDlg,IDC_FORMATDM,BM_SETCHECK,TRUE,0L);  
				SendDlgItemMessage (hWndDlg,IDC_FORMATDD,BM_SETCHECK,FALSE,0L);  
				ShowWindow (GetDlgItem(hWndDlg,IDC_MINLAT),TRUE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_MINLONG),TRUE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_MINLAT2),TRUE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_MINLONG2),TRUE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_SECLAT),FALSE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_SECLAT2),FALSE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_SECLONG),FALSE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_SECLONG2),FALSE);
				break;
 			case 3:
				SendDlgItemMessage (hWndDlg,IDC_FORMATDMS,BM_SETCHECK,FALSE,0L);
				SendDlgItemMessage (hWndDlg,IDC_FORMATDM,BM_SETCHECK,FALSE,0L);  
				SendDlgItemMessage (hWndDlg,IDC_FORMATDD,BM_SETCHECK,TRUE,0L);  
				ShowWindow (GetDlgItem(hWndDlg,IDC_MINLAT),FALSE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_MINLONG),FALSE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_MINLAT2),FALSE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_MINLONG2),FALSE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_SECLAT),FALSE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_SECLAT2),FALSE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_SECLONG),FALSE);
				ShowWindow (GetDlgItem(hWndDlg,IDC_SECLONG2),FALSE);
				break;
		}
		GetDMS (Point.x,&Deg,&Min,&Sec);
		DMin = Min + Sec/60e0; 
		DDeg = Point.x;
		switch (iFormat)
		{
			case 1:
				SetDlgItemInt (hWndDlg,IDC_DEGLONG,abs(Deg),TRUE); 
				SetDlgItemInt (hWndDlg,IDC_MINLONG,Min,TRUE);
				sprintf (str,"%4.1f",Sec);
				SetDlgItemText (hWndDlg,IDC_SECLONG,str); 
				break;
			case 2:
				SetDlgItemInt (hWndDlg,IDC_DEGLONG,abs(Deg),TRUE); 
				sprintf (str,"%6.3f",DMin);
				SetDlgItemText (hWndDlg,IDC_MINLONG,str);
				SetDlgItemText (hWndDlg,IDC_SECLONG,"0"); 
				break; 
			case 3:
				sprintf (str,"%8.5f",fabs(Point.x));
				SetDlgItemText (hWndDlg,IDC_DEGLONG,str);
				SetDlgItemText (hWndDlg,IDC_SECLONG,"0"); 
				SetDlgItemText (hWndDlg,IDC_MINLONG,"0"); 
				break; 
		}
		GetDMS (Point.y,&Deg,&Min,&Sec);
		DMin = Min + Sec/60e0;
		DDeg = Point.y; 
		switch (iFormat)
		{
			case 1:
				SetDlgItemInt (hWndDlg,IDC_DEGLAT,Deg,TRUE); 
				SetDlgItemInt (hWndDlg,IDC_MINLAT,Min,TRUE);
				sprintf (str,"%4.1f",Sec);
				SetDlgItemText (hWndDlg,IDC_SECLAT,str); 
				break;
			case 2:
				SetDlgItemInt (hWndDlg,IDC_DEGLAT,Deg,TRUE); 
				sprintf (str,"%6.3f",DMin);
				SetDlgItemText (hWndDlg,IDC_MINLAT,str);
				SetDlgItemText (hWndDlg,IDC_SECLAT,"0"); 
				break; 
			case 3:
				sprintf (str,"%8.5f",Point.y);
				SetDlgItemText (hWndDlg,IDC_DEGLAT,str);
				SetDlgItemText (hWndDlg,IDC_SECLAT,"0"); 
				SetDlgItemText (hWndDlg,IDC_MINLAT,"0"); 
				break; 
		}
 		 if (FirstPoint && !CursorIsLocked)
 		 {
				SetDlgItemText (hWndDlg,IDC_DEGLAT,"");
				SetDlgItemText (hWndDlg,IDC_SECLAT,""); 
				SetDlgItemText (hWndDlg,IDC_MINLAT,""); 
				SetDlgItemText (hWndDlg,IDC_DEGLONG,"");
				SetDlgItemText (hWndDlg,IDC_SECLONG,""); 
				SetDlgItemText (hWndDlg,IDC_MINLONG,""); 
 		 }	
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           { 
            case IDC_KEEPONSCREEN:
            	if ((KeepOnScreen = SendDlgItemMessage (hWndDlg,IDC_KEEPONSCREEN,BM_GETCHECK,0,0)))
	            	EndDialog(hWndDlg, 3); 
            	break;
            	
           	case IDC_SNAPTOGPS:
           		GetVehicleLoc ("GPS",&UserSpecifiedBasePoint,0);
	            EndDialog(hWndDlg, TRUE); 
	            break;
           	
           	case IDC_FORMATDMS:
           		SetGlobalValueLong ("%LATLONFORMAT",1);	 
       	 		PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
           		break;
           		
           	case IDC_FORMATDM:
           		SetGlobalValueLong ("%LATLONFORMAT",2);	 
       	 		PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
           		break;
           		
           	case IDC_FORMATDD:
           		SetGlobalValueLong ("%LATLONFORMAT",3);	 
       	 		PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
           		break;
           		
		 	case IDOK: 
		 	{
		 		 char	SaveProj[32], project[32];
		 		 LPSTR	lpDot, lpEnd;
		 		 short	SaveUnits;
		 		 
/*		 		 SaveUnits = PRJ_UNITS[3];
		 		 _fstrcpy (SaveProj,"[%ALT_PROJECTION]");
		 		 ExpandText (SaveProj);
                 GetDlgItemText (hWndDlg,IDC_PROJECTION,project,sizeof(project));
                 if ((lpDot=_fstrrchr(project,'.')))
                    *lpDot = 0;
                 SetGlobalValue("%ALT_PROJECTION",project);
			     ConvertCoordClose ();
				 ConvertCoordInit();*/
				 //Deg = GetDlgItemInt (hWndDlg,IDC_DEGLONG,&Err,FALSE);
				 GetDlgItemText (hWndDlg,IDC_DEGLONG,str,14);
				 DDeg = atof (str);
				 GetDlgItemText (hWndDlg,IDC_MINLONG,str,14);
				 DMin = atof (str);
				 GetDlgItemText (hWndDlg,IDC_SECLONG,str,14);
				 Sec = atof (str);
				 UserSpecifiedBasePoint.x = -(DDeg + DMin/60E0 + Sec/3600E0);
				 //Deg = GetDlgItemInt (hWndDlg,IDC_DEGLAT,&Err,FALSE);
				 GetDlgItemText (hWndDlg,IDC_DEGLAT,str,14);
				 DDeg = atof (str);
				 GetDlgItemText (hWndDlg,IDC_MINLAT,str,14);
				 DMin = atof (str);
				 GetDlgItemText (hWndDlg,IDC_SECLAT,str,14);
				 Sec = atof (str);
				 UserSpecifiedBasePoint.y = DDeg + DMin/60E0 + Sec/3600E0; 
				 ConvertCoord (&UserSpecifiedBasePoint,2,1);
				 Point = UserSpecifiedBasePoint; 
				 FirstPoint = FALSE;
/*                 PRJ_UNITS[3] = SaveUnits;
				 SetGlobalValue ("%ALT_PROJECTION",SaveProj);
			     ConvertCoordClose ();*/
	             if (!KeepOnScreen)
	             	EndDialog(hWndDlg, TRUE); 
	        }
		 	     break; 
		 	     
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
				 ConvertCoord (&Point,2,1);
                 EndDialog(hWndDlg, FALSE);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1051);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1051);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL PointFromDMS(LPSTR DMSPoint, LPDPOINT pPoint)
{
	BOOL rtn = FALSE;
	DPOINT point;
	int DDeg, DMin;
	float Sec;
	char str[128];
	int n;
	LPSTR pNorth;
	LPSTR pEast;
	int northSign = 1;
	int eastSign = 1;

	strcpy(str, DMSPoint);
	pNorth = strstr(str, " N");
	if (!pNorth)
	{
		pNorth = strstr(str, " S");
		if (pNorth)
			northSign = -1;
	}
	pEast = strstr(str, " E");
	if (!pEast)
	{
		pEast = strstr(str, " W");
		if (pEast)
			eastSign = -1;
	}
	if (pEast)
		*pEast = 0;
	if (pNorth)
	{
		*pNorth++ = 0;
		pNorth++;
		n = sscanf(str, "%i %i' %f""", &DDeg, &DMin, &Sec);
		if (n == 3)
		{
			point.y = northSign * (DDeg + DMin / 60E0 + Sec / 3600E0);
			n = sscanf(pNorth, "%i %i' %f""", &DDeg, &DMin, &Sec);
			if (n == 3)
			{
				point.x = eastSign * (DDeg + DMin / 60E0 + Sec / 3600E0);
				ConvertCoord(&point, 2, 1);
				*pPoint = point;
				rtn = TRUE;
			}
		}
	}
	return rtn;
}

BOOL FAR PASCAL SETTAGMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (831);
#endif
{
 LPTAGDEF   lpTAGDef, pSaveTAGDef; 
 static		HANDLE	hTAGDefSave;
 short		i;
 short    BRtn; 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (831);
#endif
 	return (BRtn);
}

 switch(Message)
   {
    case WM_INITDIALOG:
         cwCenter(hWndDlg, 0);
         LoadTAGDef ();   
         if (NumTAGDef<=0)
{
#if ENABLETRACE
GSSiExitProg (831);
#endif
         	return FALSE; 
}
         lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
		 hTAGDefSave = GSSiGlobAlloc ( 518,GMEM_MOVEABLE,(long)NumTAGDef*sizeof(TAGDEF));
         pSaveTAGDef = (LPTAGDEF)GlobalLock (hTAGDefSave);
         _fmemmove (pSaveTAGDef,lpTAGDef,(size_t)NumTAGDef*sizeof(TAGDEF));
         for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
            SendDlgItemMessage (hWndDlg,IDC_TAGPREFIX,CB_ADDSTRING,0,(LPARAM)lpTAGDef->Prefix);
         GlobalUnlock (hTAGDefSave);
         GlobalUnlock (hTAGDef);

         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

		 switch(LOWORD (wParam))

           {
            case IDCANCEL:
		         lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
		         pSaveTAGDef = (LPTAGDEF)GlobalLock (hTAGDefSave);
		         _fmemmove (lpTAGDef,pSaveTAGDef,(size_t)NumTAGDef*sizeof(TAGDEF));
		         GSSiGlobUlFree (&hTAGDefSave);
		         GlobalUnlock (hTAGDef);
                 EndDialog(hWndDlg, FALSE);
                 break;

            case IDOK: 
            	 if (!UpdateTAGValue (hWndDlg))
            	 	break;
		         GSSiGlobUlFree (&hTAGDefSave);
                 EndDialog(hWndDlg, TRUE);
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (831);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (831);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL FAR PASCAL SETSYMMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (837);
#endif
{ 
	char	SymName[34], str[256]; 
	float	SymSize, SymRot;
	
 short    BRtn; 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (837);
#endif
 	return (BRtn);
}

 switch(Message)
   {
    case WM_INITDIALOG:
         cwCenter(hWndDlg, 0);
         _fstrcpy (str,"[%NEW_POINT_SYM]");
         ExpandText (str);
         SetDlgItemText (hWndDlg,IDC_POINT_SYM,str);
         _fstrcpy (str,"[%NEW_LINE_SYM]");
         ExpandText (str);
         SetDlgItemText (hWndDlg,IDC_LINE_SYM,str);
         _fstrcpy (str,"[%NEW_AREA_SYM]");
         ExpandText (str);
         SetDlgItemText (hWndDlg,IDC_AREA_SYM,str);

         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

		 switch(LOWORD (wParam))

           {
            case IDCANCEL:
                 EndDialog(hWndDlg, FALSE);
                 break;

			case IDC_SET_POINTSYM: 
			{
				 char	NewPointSize[64]="[%POINT_SIZE]", NewPointRot[64]="[%POINT_ROT]", NewPointColor[64]="[%POINT_COLOR]";
				 
				 ExpandText (NewPointSize);
				 ExpandText (NewPointRot);
				 ExpandText (NewPointColor);
			     if (SelectPointSymbol (hWndDlg,1,SymName,"All",NewPointSize,NewPointRot,NewPointColor,FALSE))
			     {
			     	SetDlgItemText (hWndDlg,IDC_POINT_SYM,SymName);
			     }
			}
                 break;
                 
			case IDC_SET_LINESYM:
			{
				 char	NewLineWidth[64]="[%LINE_WIDTH]", NewLineColor[64]="[%LINE_COLOR]";
				 ExpandText (NewLineWidth);
				 ExpandText (NewLineColor);
			     if (SelectLineSymbol (hWndDlg,1,SymName,NewLineWidth,NewLineColor,FALSE))
			     {
			     	SetDlgItemText (hWndDlg,IDC_LINE_SYM,SymName);
			     }  
			}
                 break;
                 
			case IDC_SET_AREASYM: 
			{
				 char	NewAreaColor[64]="[%AREA_COLOR]";
				 
				 ExpandText (NewAreaColor);	
			     if (SelectAreaSymbol (hWndDlg,1,SymName, NewAreaColor,FALSE))
			     {
			     	SetDlgItemText (hWndDlg,IDC_AREA_SYM,SymName);
			     } 
			}
                 break;
                 
            case IDOK:
            	 GetDlgItemText (hWndDlg,IDC_POINT_SYM,SymName,32);
            	 SetGlobalValue ("%NEW_POINT_SYM",SymName); 
            	 GetDlgItemText (hWndDlg,IDC_LINE_SYM,SymName,32);
            	 SetGlobalValue ("%NEW_LINE_SYM",SymName); 
            	 GetDlgItemText (hWndDlg,IDC_AREA_SYM,SymName,32);
            	 SetGlobalValue ("%NEW_AREA_SYM",SymName); 
                 EndDialog(hWndDlg, TRUE);
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (837);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (837);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL NEWPOINTSETMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (894);
#endif
{ 
	char	SymName[34], str[256]; 
	float	SymSize, SymRot;
	static	long	color;
	short	MaxCMDLen; 
	BOOL	Err, WantText;
	static	HANDLE	hSaveBM=0;
	
 short    BRtn; 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (894);
#endif
 	return (BRtn);
}

 switch(Message)
   {
    case WM_INITDIALOG:
    {
		 hSaveBM = EnterBlockingWindow (hWndDlg);
		
         LoadTAGDef ();   
         if (NumTAGDef)
         { 
            LPTAGDEF    lpTAGDef;
            short	i; 
            
            lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
            for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
            {
                SendDlgItemMessage (hWndDlg,IDC_PREFIX,CB_ADDSTRING,0,(LPARAM)lpTAGDef->Prefix);
            }
            GlobalUnlock (hTAGDef);
         } 
		 ExpandGlobalRaw ("%NEW_POINT_SIZE",TRUE,str,256);
		 if (!*str)
		 	_fstrcpy (str,"10P");
		 SetDlgItemText (hWndDlg,IDC_POINTSIZE,str);
		 ExpandGlobalRaw ("%NEW_POINT_ROT",TRUE,str,256);
		 if (!*str)
		 	_fstrcpy (str,"0D");
		 SetDlgItemText (hWndDlg,IDC_POINTROT,str);
		 ExpandGlobalRaw ("%NEW_POINT_UDI",TRUE,str,256);
		 SetDlgItemText (hWndDlg,IDC_UDI,str);
		 ExpandGlobalRaw ("%NEW_POINT_PREFIX",TRUE,str,256);
		 SetDlgItemText (hWndDlg,IDC_PREFIX,str);   
		 ExpandGlobalRaw ("%NEW_POINT_GRCMD",TRUE,str,256);
		 SetDlgItemText (hWndDlg,IDC_EMBEDCMD,str); 
		 ExpandGlobalRaw ("%NEW_POINT_MACRO",TRUE,str,256);
		 SetDlgItemText (hWndDlg,IDC_NPMACRO,str); 
		 MaxCMDLen = GetGlobalLVal ("[%NEW_POINT_MAXCMD]"); 
		 SetDlgItemInt (hWndDlg,IDC_MAXCMD,MaxCMDLen,FALSE);
		 SetDlgItemTextGlobal (hWndDlg,IDC_AUTOINCVAL,"[%NEW_POINT_AUTOINC]","");
		 _fstrcpy (str,"[%NEW_POINT_COLOR]");		 
		 ExpandText (str);
		 color = atol (str);
		 if (color >= 0)
		 {
       	 	SendDlgItemMessage (hWndDlg,IDC_OVERRIDE_SYMCOLOR,BM_SETCHECK,TRUE,0L);
       	 	EnableWindow (GetDlgItem(hWndDlg,IDC_SETCOLOR),TRUE);
		 }
         cwCenter(hWndDlg, 0);
         _fstrcpy (str,"[%NEW_POINT_SYM]");
         ExpandText (str);
		 SetDlgItemText (hWndDlg,IDC_POINT_SYM,str); 
		 WantText = GetGlobalBVal("[%NEW_POINT_WANTTEXT]"); 
		 SendDlgItemMessage (hWndDlg,IDC_ATTACHTEXT,BM_SETCHECK,WantText,0L);
    	 PostMessage(hWndDlg, WM_COMMAND, IDC_SHOWSYM, 0L);
    	 PostMessage(hWndDlg, WM_COMMAND, IDC_ATTACHTEXT, 0L);
    }
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

		 switch(LOWORD (wParam))

           {
            case IDCANCEL:
                 GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
                 break;
            
            case IDC_OVERRIDE_SYMCOLOR:
       	 		 EnableWindow (GetDlgItem(hWndDlg,IDC_SETCOLOR),
       	 					   (BOOL)SendDlgItemMessage (hWndDlg,IDC_OVERRIDE_SYMCOLOR,BM_GETCHECK,0,0));
                 break;
            
            case IDC_SETCOLOR:
            	 if (GetColor(hWndDlg,&color))
			    	 PostMessage(hWndDlg, WM_COMMAND, IDC_SHOWSYM, 0L);
            	 break;
            
            case IDC_ATTACHTEXT:
            	 WantText = SendDlgItemMessage (hWndDlg,IDC_ATTACHTEXT,BM_GETCHECK,0,0);
				 EnableWindow (GetDlgItem(hWndDlg,IDC_SETTEXT),WantText);
            	 SetGlobalValueBool ("%NEW_POINT_WANTTEXT",WantText);
				 GSSiGlobFree (&hGRText); 
				 GSSiGlobFree (&hGRTextTPL); 
				 if (WantText)
				 {
					 SetNewText (-1,&hGRText,&hGRTextTPL,0); 
				 }
            	 break;
            	              
            case IDC_SETTEXT:
			    {
			 	   FARPROC lpfnGRTEXTMsgProc;   
			 	   short	nRc;
			         
				    lpfnGRTEXTMsgProc = MakeProcInstance((FARPROC)GRTEXTMsgProc, hInst);
					nRc = DialogBox(hInst, (LPSTR)"GRTEXT", hWndDlg, (DLGPROC)lpfnGRTEXTMsgProc);
				    FreeProcInstance(lpfnGRTEXTMsgProc);
			
			    } 
            	 break;
            	 
            case IDC_SHOWSYM:
            {
            	short	symnum;
            	HANDLE	hSymbol;
            	BOOL	SaveHVFC = HaveVarFillColor; 
            	long	SaveGC = GlobalColors[0];
            	
				GetDlgItemText (hWndDlg,IDC_POINT_SYM,SymName,sizeof(SymName));
				symnum = GetDictSymbolNumber (SymName); 
				if (!symnum)
					break;  
				if (SendDlgItemMessage (hWndDlg,IDC_OVERRIDE_SYMCOLOR,BM_GETCHECK,0,0L))
				{
					HaveVarFillColor = TRUE;
					GlobalColors[0] = color;
				}
				hSymbol = GetDictSymDesc (symnum,0);
				DisplaySymInWindow (GetDlgItem(hWndDlg,IDC_SYMBOX),0,hSymbol,0,0,hBackBrush1,0);
				DestroySymbol (hSymbol);   
				HaveVarFillColor = SaveHVFC;
				GlobalColors[0] = SaveGC;
			}
                break;
                 
			case IDC_SET_POINTSYM: 
			{
				 char	NewPointSize[64]="[%POINT_SIZE]", NewPointRot[64]="[%POINT_ROT]", NewPointColor[64]="[%POINT_COLOR]";
				 
				 GetDlgItemText (hWndDlg,IDC_POINT_SYM,SymName,sizeof(SymName));
				 GetDlgItemText (hWndDlg,IDC_POINTSIZE,NewPointSize,sizeof(NewPointSize));
				 GetDlgItemText (hWndDlg,IDC_POINTROT,NewPointRot,sizeof(NewPointRot));
				 ExpandText (NewPointSize);
				 ExpandText (NewPointRot);
				 ExpandText (NewPointColor);
				 _fstrcpy (SymName,"[%NEW_POINT_SYM]"); 
				 ExpandText (SymName);
			     if (SelectPointSymbol (hWndDlg,1,SymName,"All",NewPointSize,NewPointRot,NewPointColor,FALSE))
			     {
			     	SetDlgItemText (hWndDlg,IDC_POINT_SYM,SymName);
			     	SetDlgItemText (hWndDlg,IDC_POINTSIZE,NewPointSize);
			     	SetDlgItemText (hWndDlg,IDC_POINTROT,NewPointRot); 
			     	if (*NewPointColor)
			     	{   
			     		color = atol (NewPointColor);
			       	 	SendDlgItemMessage (hWndDlg,IDC_OVERRIDE_SYMCOLOR,BM_SETCHECK,TRUE,0L);
			       	 	EnableWindow (GetDlgItem(hWndDlg,IDC_SETCOLOR),TRUE);
			       	}
			    	PostMessage(hWndDlg, WM_COMMAND, IDC_SHOWSYM, 0L);
			     }
			}
                 break;
                 
            case IDOK:  
            	 if (!SendDlgItemMessage (hWndDlg,IDC_OVERRIDE_SYMCOLOR,BM_GETCHECK,0,0))
            	 	color = -1;
            	 GetDlgItemText (hWndDlg,IDC_POINT_SYM,SymName,32);
            	 SetGlobalValue ("%NEW_POINT_SYM",SymName); 
            	 GetDlgItemText (hWndDlg,IDC_POINTROT,str,256);
            	 SetGlobalValue ("%NEW_POINT_ROT",str); 
            	 GetDlgItemText (hWndDlg,IDC_POINTSIZE,str,256);
            	 SetGlobalValue ("%NEW_POINT_SIZE",str); 
            	 GetDlgItemText (hWndDlg,IDC_PREFIX,str,256);
            	 _fstrupr (str);
            	 SetGlobalValue ("%NEW_POINT_PREFIX",str); 
            	 GetDlgItemText (hWndDlg,IDC_UDI,str,256);
            	 SetGlobalValue ("%NEW_POINT_UDI",str); 
            	 GetDlgItemText (hWndDlg,IDC_EMBEDCMD,str,256);
            	 SetGlobalValue ("%NEW_POINT_GRCMD",str); 
            	 GetDlgItemText (hWndDlg,IDC_NPMACRO,str,256);
            	 SetGlobalValue ("%NEW_POINT_MACRO",str); 
                 MaxCMDLen = GetDlgItemInt (hWndDlg,IDC_MAXCMD,&Err,FALSE);
            	 SetGlobalValueLong ("%NEW_POINT_MAXCMD",MaxCMDLen); 
            	 SetGlobalValueLong ("%NEW_POINT_COLOR",color); 
		 		 GetDlgItemTextGlobal (hWndDlg,IDC_AUTOINCVAL,"[%NEW_POINT_AUTOINC]");
            	 WantText = SendDlgItemMessage (hWndDlg,IDC_ATTACHTEXT,BM_GETCHECK,0,0); 
            	 SetGlobalValueBool ("%NEW_POINT_WANTTEXT",WantText);
            	 if (WantText && hGRText)
            	 	GetNewText (1,hGRText,hGRTextTPL);
                 GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (894);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (894);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}
BOOL FAR PASCAL NEWLINESETMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (895);
#endif
{ 
	char	SymName[34], str[256]; 
	float	SymSize, SymRot;
	static	long	color;  
	short	MaxCMDLen;  
	BOOL	Err, WantText;  
	static	HANDLE	hSaveBM=0;
	
 short    BRtn; 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (895);
#endif
 	return (BRtn);
}

 switch(Message)
   {
    case WM_INITDIALOG:
    {
		 hSaveBM = EnterBlockingWindow (hWndDlg);
		
         LoadTAGDef ();   
         if (NumTAGDef)
         { 
            LPTAGDEF    lpTAGDef;
            short	i; 
            
            lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
            for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
            {
                SendDlgItemMessage (hWndDlg,IDC_PREFIX,CB_ADDSTRING,0,(LPARAM)lpTAGDef->Prefix);
            }
            GlobalUnlock (hTAGDef);
         } 
		 ExpandGlobalRaw ("%NEW_LINE_UDI",TRUE,str,256);
		 SetDlgItemText (hWndDlg,IDC_UDI,str);
		 ExpandGlobalRaw ("%NEW_LINE_PREFIX",TRUE,str,256);
		 SetDlgItemText (hWndDlg,IDC_PREFIX,str);   
		 ExpandGlobalRaw ("%NEW_LINE_WIDTH",TRUE,str,256);
		 SetDlgItemText (hWndDlg,IDC_LINEWIDTH,str);   
		 ExpandGlobalRaw ("%NEW_LINE_GRCMD",TRUE,str,256);
		 SetDlgItemText (hWndDlg,IDC_EMBEDCMD,str);  
		 MaxCMDLen = GetGlobalLVal ("[%NEW_LINE_MAXCMD]"); 
		 SetDlgItemInt (hWndDlg,IDC_MAXCMD,MaxCMDLen,FALSE);
		 SetDlgItemTextGlobal (hWndDlg,IDC_AUTOINCVAL,"[%NEW_LINE_AUTOINC]","");
		 _fstrcpy (str,"[%NEW_LINE_COLOR]");		 
		 ExpandText (str);
		 color = atol (str);
		 if (color >= 0)
		 {
       	 	SendDlgItemMessage (hWndDlg,IDC_OVERRIDE_SYMCOLOR,BM_SETCHECK,TRUE,0L);
       	 	EnableWindow (GetDlgItem(hWndDlg,IDC_SETCOLOR),TRUE);
		 }
         cwCenter(hWndDlg, 0);
         _fstrcpy (str,"[%NEW_LINE_SYM]");
         ExpandText (str);
		 WantText = GetGlobalBVal("[%NEW_LINE_WANTTEXT]"); 
		 SendDlgItemMessage (hWndDlg,IDC_ATTACHTEXT,BM_SETCHECK,WantText,0L);
//    	 PostMessage(hWndDlg, WM_COMMAND, IDC_SHOWSYM, 0L);
    	 PostMessage(hWndDlg, WM_COMMAND, IDC_ATTACHTEXT, 0L);
		 SetDlgItemText (hWndDlg,IDC_LINE_SYM,str);
    }
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

		 switch(LOWORD (wParam))

           {
            case IDCANCEL:
                 GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
                 break;
            
            case IDC_ATTACHTEXT:
            	 WantText = SendDlgItemMessage (hWndDlg,IDC_ATTACHTEXT,BM_GETCHECK,0,0);
				 EnableWindow (GetDlgItem(hWndDlg,IDC_SETTEXT),WantText);
            	 SetGlobalValueBool ("%NEW_LINE_WANTTEXT",WantText);
				 if (WantText)
				 {
					 GSSiGlobFree (&hGRText); 
					 GSSiGlobFree (&hGRTextTPL); 
					 SetNewText (-2,&hGRText,&hGRTextTPL,0); 
				 }
            	 break;
            	              
            case IDC_SETTEXT:
			    {
			 	   FARPROC lpfnGRTEXTMsgProc;   
			 	   short	nRc;
			         
				    lpfnGRTEXTMsgProc = MakeProcInstance((FARPROC)GRTEXTMsgProc, hInst);
					nRc = DialogBox(hInst, (LPSTR)"GRTEXT", hWndDlg, (DLGPROC)lpfnGRTEXTMsgProc);
				    FreeProcInstance(lpfnGRTEXTMsgProc);
			
			    } 
            	 break;
            	 
            case IDC_OVERRIDE_SYMCOLOR:
       	 		 EnableWindow (GetDlgItem(hWndDlg,IDC_SETCOLOR),
       	 					   (BOOL)SendDlgItemMessage (hWndDlg,IDC_OVERRIDE_SYMCOLOR,BM_GETCHECK,0,0L));
                 break;
            
            case IDC_SETCOLOR:
            	 if (GetColor(hWndDlg,&color)) 
            	 break;
                 
			case IDC_SET_LINESYM: 
			{   
				char	SymName[34]="[%NEW_LINE_SYM]", SymSize[64]="[%NEW_LINE_WIDTH]", SymColor[64]="[%NEW_LINE_COLOR]";   
				float	size,rot;
					
				ExpandText (SymName);
				ExpandText (SymSize);
				ExpandText (SymColor);
				if (SelectLineSymbol (CurView->hWnd,1,SymName,SymSize,SymColor,FALSE))
				{   
			     	SetDlgItemText (hWndDlg,IDC_LINE_SYM,SymName);
			     	SetDlgItemText (hWndDlg,IDC_LINEWIDTH,SymSize);
			     	color = atol (SymColor);
			    }
			}
                 break;
                 
            case IDOK:  
            	 if (!SendDlgItemMessage (hWndDlg,IDC_OVERRIDE_SYMCOLOR,BM_GETCHECK,0,0L))
            	 	color = -1;
            	 GetDlgItemText (hWndDlg,IDC_LINE_SYM,SymName,32);
            	 SetGlobalValue ("%NEW_LINE_SYM",SymName); 
            	 GetDlgItemText (hWndDlg,IDC_PREFIX,str,256);  
             	 _fstrupr (str);
            	 SetGlobalValue ("%NEW_LINE_PREFIX",str); 
            	 GetDlgItemText (hWndDlg,IDC_UDI,str,256);
            	 SetGlobalValue ("%NEW_LINE_UDI",str); 
            	 GetDlgItemText (hWndDlg,IDC_EMBEDCMD,str,256);
            	 SetGlobalValue ("%NEW_LINE_GRCMD",str); 
                 MaxCMDLen = GetDlgItemInt (hWndDlg,IDC_MAXCMD,&Err,FALSE);
            	 SetGlobalValueLong ("%NEW_LINE_MAXCMD",MaxCMDLen); 
            	 GetDlgItemText (hWndDlg,IDC_LINEWIDTH,str,256);
            	 SetGlobalValue ("%NEW_LINE_WIDTH",str); 
            	 SetGlobalValueLong ("%NEW_LINE_COLOR",color); 
            	 WantText = SendDlgItemMessage (hWndDlg,IDC_ATTACHTEXT,BM_GETCHECK,0,0); 
            	 SetGlobalValueBool ("%NEW_LINE_WANTTEXT",WantText);
		 		 GetDlgItemTextGlobal (hWndDlg,IDC_AUTOINCVAL,"[%NEW_LINE_AUTOINC]");
            	 if (WantText && hGRText)
            	 	GetNewText (2,hGRText,hGRTextTPL);
            	 
                 GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (895);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (895);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL NEWAREASETMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (896);
#endif
{ 
	char	SymName[34], str[256]; 
	float	SymSize, SymRot;
	static	long	color;
	short	MaxCMDLen;  
	BOOL	Err, WantText=FALSE;
	static	HANDLE	hSaveBM=0;
	
 short    BRtn; 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (896);
#endif
 	return (BRtn);
}

 switch(Message)
   {
    case WM_INITDIALOG:
    {
		 hSaveBM = EnterBlockingWindow (hWndDlg);
		
         LoadTAGDef ();   
         if (NumTAGDef)
         { 
            LPTAGDEF    lpTAGDef;
            short	i; 
            
            lpTAGDef = (LPTAGDEF)GlobalLock (hTAGDef);
            for (i=0;i<NumTAGDef;i++,lpTAGDef++) 
            {
                SendDlgItemMessage (hWndDlg,IDC_PREFIX,CB_ADDSTRING,0,(LPARAM)lpTAGDef->Prefix);
            }
            GlobalUnlock (hTAGDef);
         } 
		 ExpandGlobalRaw ("%NEW_AREA_UDI",TRUE,str,256);
		 SetDlgItemText (hWndDlg,IDC_UDI,str);
		 ExpandGlobalRaw ("%NEW_AREA_PREFIX",TRUE,str,256);
		 SetDlgItemText (hWndDlg,IDC_PREFIX,str);   
		 ExpandGlobalRaw ("%NEW_AREA_GRCMD",TRUE,str,256);
		 SetDlgItemText (hWndDlg,IDC_EMBEDCMD,str);  
		 MaxCMDLen = GetGlobalLVal ("[%NEW_AREA_MAXCMD]"); 
		 SetDlgItemInt (hWndDlg,IDC_MAXCMD,MaxCMDLen,FALSE);
		 SetDlgItemTextGlobal (hWndDlg,IDC_AUTOINCVAL,"[%NEW_AREA_AUTOINC]","");
		 _fstrcpy (str,"[%NEW_AREA_COLOR]");		 
		 ExpandText (str);
		 color = atol (str);
		 if (color >= 0)
		 {
       	 	SendDlgItemMessage (hWndDlg,IDC_OVERRIDE_SYMCOLOR,BM_SETCHECK,TRUE,0L);
       	 	EnableWindow (GetDlgItem(hWndDlg,IDC_SETCOLOR),TRUE);
		 }
         cwCenter(hWndDlg, 0);
         _fstrcpy (str,"[%NEW_AREA_SYM]");
         ExpandText (str);
		 SetDlgItemText (hWndDlg,IDC_AREA_SYM,str);
		 WantText = GetGlobalBVal("[%NEW_AREA_WANTTEXT]"); 
		 SendDlgItemMessage (hWndDlg,IDC_ATTACHTEXT,BM_SETCHECK,WantText,0L);
//    	 PostMessage(hWndDlg, WM_COMMAND, IDC_SHOWSYM, 0L);
    	 PostMessage(hWndDlg, WM_COMMAND, IDC_ATTACHTEXT, 0L);
    }
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

		 switch(LOWORD (wParam))

           {
            case IDCANCEL:
                 GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
                 break;
            
            case IDC_ATTACHTEXT:
            	 WantText = SendDlgItemMessage (hWndDlg,IDC_ATTACHTEXT,BM_GETCHECK,0,0);
				 EnableWindow (GetDlgItem(hWndDlg,IDC_SETTEXT),WantText);
            	 SetGlobalValueBool ("%NEW_AREA_WANTTEXT",WantText);
				 if (WantText)
				 {
					 GSSiGlobFree (&hGRText); 
					 GSSiGlobFree (&hGRTextTPL); 
					 SetNewText (-3,&hGRText,&hGRTextTPL,0); 
				 }
            	 break;
            	              
            case IDC_SETTEXT:
			    {
			 	   FARPROC lpfnGRTEXTMsgProc;   
			 	   short	nRc;
			         
				    lpfnGRTEXTMsgProc = MakeProcInstance((FARPROC)GRTEXTMsgProc, hInst);
					nRc = DialogBox(hInst, (LPSTR)"GRTEXT", hWndDlg, (DLGPROC)lpfnGRTEXTMsgProc);
				    FreeProcInstance(lpfnGRTEXTMsgProc);
			
			    } 
            	 break;
            	 
            case IDC_OVERRIDE_SYMCOLOR:
       	 		 EnableWindow (GetDlgItem(hWndDlg,IDC_SETCOLOR),
       	 					   (BOOL)SendDlgItemMessage (hWndDlg,IDC_OVERRIDE_SYMCOLOR,BM_GETCHECK,0,0L));
                 break;
            
            case IDC_SETCOLOR:
            	 GetColor(hWndDlg,&color); 
            	 break;
                 
			case IDC_SET_AREASYM: 
			{   
				char	SymName[34]="[%NEW_AREA_SYM]", SymColor[64]="[%NEW_AREA_COLOR]";   
				float	size,rot;
					
				ExpandText (SymName);
				ExpandText (SymColor);
				if (SelectAreaSymbol (CurView->hWnd,1,SymName,SymColor,FALSE))
				{   
			     	SetDlgItemText (hWndDlg,IDC_AREA_SYM,SymName);
			     	color = atol (SymColor);
			    }
			}
                 break;
                 
            case IDOK:
            	 if (!SendDlgItemMessage (hWndDlg,IDC_OVERRIDE_SYMCOLOR,BM_GETCHECK,0,0L))
            	 	color = -1;
            	 GetDlgItemText (hWndDlg,IDC_AREA_SYM,SymName,32);
            	 SetGlobalValue ("%NEW_AREA_SYM",SymName); 
            	 GetDlgItemText (hWndDlg,IDC_PREFIX,str,256); 
            	 _fstrupr (str);
            	 SetGlobalValue ("%NEW_AREA_PREFIX",str); 
            	 GetDlgItemText (hWndDlg,IDC_UDI,str,256);
            	 SetGlobalValue ("%NEW_AREA_UDI",str); 
            	 GetDlgItemText (hWndDlg,IDC_EMBEDCMD,str,256);
            	 SetGlobalValue ("%NEW_AREA_GRCMD",str);
                 MaxCMDLen = GetDlgItemInt (hWndDlg,IDC_MAXCMD,&Err,FALSE);
            	 SetGlobalValueLong ("%NEW_AREA_MAXCMD",MaxCMDLen); 
            	 SetGlobalValueLong ("%NEW_AREA_COLOR",color); 
		 		 GetDlgItemTextGlobal (hWndDlg,IDC_AUTOINCVAL,"[%NEW_AREA_AUTOINC]");
            	 if (WantText && hGRText)
            	 	GetNewText (3,hGRText,hGRTextTPL);
            	 
                 GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (896);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (896);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL TEXTHEADEDITMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (844);
#endif
{ 
 char	str[128], txt[128];   
 short    BRtn,i; 
 double	THeight; 
 BOOL	Bold;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (844);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    
    case WM_INITDIALOG: 
    	THeight = GetTextHeadSize (&GRTextHeader);
		sprintf (str,"%.3f",THeight);
		SetDlgItemText (hWndDlg,IDC_TXHEIGHT,str);
    	for (i=0;i<MAXFONTS;i++)
    	{
    		sprintf (txt,"[%cFONTNAME(%i)]",'%',i); 
    		ExpandText (txt);
    		sprintf (str,"%i: %s",i,txt);
			SendDlgItemMessage(hWndDlg,IDC_FONTLIST,LB_ADDSTRING,0,(LPARAM)str); 
		}
		if (GRTextHeader.Weight > 1)
			Bold = TRUE;
		else
			Bold = FALSE;
 		SendDlgItemMessage (hWndDlg,IDC_FONTLIST,LB_SETCURSEL,(WPARAM)GRTextHeader.FontNum,(LPARAM)0); 
    	SendDlgItemMessage (hWndDlg,IDC_BOLD,BM_SETCHECK,Bold,0); 
    	SendDlgItemMessage (hWndDlg,IDC_SHADOWTEXT,BM_SETCHECK,GRTextHeader.shadow,0); 
    	SendDlgItemMessage (hWndDlg,IDC_ITALIC,BM_SETCHECK,GRTextHeader.italic,0); 
    	SendDlgItemMessage (hWndDlg,IDC_AUTOROTATE,BM_SETCHECK,GRTextHeader.FlipForEasyReading,0); 
    	SendDlgItemMessage (hWndDlg,IDC_VJABOVE,BM_SETCHECK,FALSE,0); 
		SendDlgItemMessage (hWndDlg,IDC_VJBASELINE,BM_SETCHECK,FALSE,0);
		SendDlgItemMessage (hWndDlg,IDC_VJCENTER,BM_SETCHECK,FALSE,0);
		SendDlgItemMessage (hWndDlg,IDC_VJBELOW,BM_SETCHECK,FALSE,0);
		SendDlgItemMessage (hWndDlg,IDC_HJLEFT,BM_SETCHECK,FALSE,0); 
		SendDlgItemMessage (hWndDlg,IDC_HJCENTER,BM_SETCHECK,FALSE,0);
  		SendDlgItemMessage (hWndDlg,IDC_HJRIGHT,BM_SETCHECK,FALSE,0);  
     	switch (GRTextHeader.vJust)
    	{
    		case 1:
    			SendDlgItemMessage (hWndDlg,IDC_VJABOVE,BM_SETCHECK,TRUE,0); 
    			break;
    		case 0:
    			SendDlgItemMessage (hWndDlg,IDC_VJBASELINE,BM_SETCHECK,TRUE,0);
    			break;
    		case 2:
    			SendDlgItemMessage (hWndDlg,IDC_VJCENTER,BM_SETCHECK,TRUE,0);
    			break;
    		case 3:
    			SendDlgItemMessage (hWndDlg,IDC_VJBELOW,BM_SETCHECK,TRUE,0);
        }
    	switch (GRTextHeader.hJust)
    	{
    		case 0:
    			SendDlgItemMessage (hWndDlg,IDC_HJLEFT,BM_SETCHECK,TRUE,0); 
    			break;
    		case 1:
    			SendDlgItemMessage (hWndDlg,IDC_HJCENTER,BM_SETCHECK,TRUE,0);
    			break;
    		case 2:
    			SendDlgItemMessage (hWndDlg,IDC_HJRIGHT,BM_SETCHECK,TRUE,0);
        }
        break; /* End of WM_INITDIALOG                                 */
    
   
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

         switch(LOWORD(wParam))

         {
	         	case IDC_FONTLIST: 
	         	{
         	
#if WIN32
				switch(HIWORD(wParam))
#else
                switch(HIWORD(wParam))
#endif
                     case LBN_DBLCLK: 
                     {
                     	LOGFONT	Font; 
                     	COLORREF	TextColor;
                     	double	TextSize;
                     	short	NDP=4, FontNum;			
            	 		
            	 		_fmemset (&Font,0,sizeof(Font));
						FontNum = SendDlgItemMessage(hWndDlg,IDC_FONTLIST,LB_GETCURSEL,0,0);  
						TextColor = FontColors[FontNum];
						_fstrcpy (Font.lfFaceName,FontNames[FontNum]); 
 						GetDlgItemText (hWndDlg,IDC_TXHEIGHT,str,sizeof(str));
            	 		Font.lfHeight = atof (str) * BaseDistToWinDist;
            	 		if (GetFont (hWndDlg, &Font, &TextColor,0,0))
            	 		{
            	 			FontColors[FontNum] = TextColor;
            	 			TextSize = fabs (Font.lfHeight / BaseDistToWinDist);
            	 			RWRITE (TextSize,NDP,str);
							SetDlgItemText (hWndDlg,IDC_TXHEIGHT,str);
							_fstrcpy (FontNames[FontNum],Font.lfFaceName);
						} 
                     }
                     break;
               } 
               break;
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE); 
                 break;
                 
            case IDOK: 
            {   
            	double	dHeight;
            	
            	GetDlgItemText (hWndDlg,IDC_TEXT_FACTOR,str,sizeof(str));
            	TextSizeFactor = atof (str);
            	if (TextSizeFactor <= 0)
            		TextSizeFactor = -1;
				GetDlgItemText (hWndDlg,IDC_TXHEIGHT,str,sizeof(str));
				dHeight = atof (str);
				SetTextHeadSize (&GRTextHeader,dHeight);
                if (SendDlgItemMessage (hWndDlg,IDC_BOLD,BM_GETCHECK,0,0))
                	GRTextHeader.Weight = 2;  
                else
                	GRTextHeader.Weight = 1;  
                GRTextHeader.italic = SendDlgItemMessage (hWndDlg,IDC_ITALIC,BM_GETCHECK,0,0);
                GRTextHeader.shadow = SendDlgItemMessage (hWndDlg,IDC_SHADOWTEXT,BM_GETCHECK,0,0);
                GRTextHeader.FlipForEasyReading = SendDlgItemMessage (hWndDlg,IDC_AUTOROTATE,BM_GETCHECK,0,0);
                if (SendDlgItemMessage (hWndDlg,IDC_VJABOVE,BM_GETCHECK,0,0))
                	GRTextHeader.vJust = 1;
                else if (SendDlgItemMessage (hWndDlg,IDC_VJBASELINE,BM_GETCHECK,0,0))
                	GRTextHeader.vJust = 0;
                else if (SendDlgItemMessage (hWndDlg,IDC_VJCENTER,BM_GETCHECK,0,0))
                	GRTextHeader.vJust = 2;
                else
                	GRTextHeader.vJust = 3;	 
                if (SendDlgItemMessage (hWndDlg,IDC_HJLEFT,BM_GETCHECK,0,0))
                	GRTextHeader.hJust = 0;
                else if (SendDlgItemMessage (hWndDlg,IDC_HJCENTER,BM_GETCHECK,0,0))
                	GRTextHeader.hJust = 1;
                else
                	GRTextHeader.hJust = 2;	 
				GRTextHeader.FontNum = SendDlgItemMessage(hWndDlg,IDC_FONTLIST,LB_GETCURSEL,0,0); 
                EndDialog(hWndDlg, TRUE); 
                break;
                 
            }   
          }
          break;

    default:
{
#if ENABLETRACE
GSSiExitProg (844);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (844);
#endif
 return TRUE;    
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL FUNSTACKMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (754);
#endif
{    
	char	str[144]; 
	int		i;    
	static	LPVIEWPORT	MyVP;

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (754);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:
    	 MyVP = CurView;
    	 sprintf (str,"%s function stack",CurView->Name);     
    	 SetWindowText (hWndDlg,str);
Redisplay:
		SendDlgItemMessage (hWndDlg,IDC_FUNSTACK,LB_RESETCONTENT,0,0);
		CurView->FunStackWnd = hWndDlg;
		{
		    HANDLE	hNext, hCmdStr = CurView->FunStackHandle;
			LPCMDSTRING    pCmdStr;
			
		    while (hCmdStr)
		    { 
		    	pCmdStr = (LPCMDSTRING)GlobalLock (hCmdStr);
		    	if (!pCmdStr)
		    	{   
		    		if (hCmdStr == CurView->FunStackHandle)
		    			CurView->FunStackHandle = 0;
		    		break;
		    	}
		    	hNext = pCmdStr->PrevHandle;
				GetFunName (pCmdStr->CurFun,str);
				SendDlgItemMessage (hWndDlg,IDC_FUNSTACK,LB_ADDSTRING,0,(LPARAM)str);
		    	GlobalUnlock (hCmdStr);
		    	hCmdStr = hNext;
		    }

        }

		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
	     DestroyWindow(hWndDlg);
		 MyVP->lpfnFUNSTACKMsgProc = 0;	      
		 MyVP->DisplayFunStack = FALSE;
         break; /* End of WM_CLOSE                                      */ 
    
    case WM_PAINT:
    	 SetFocus (hWndMain);   
{
#if ENABLETRACE
GSSiExitProg (754);
#endif
    	 return FALSE;
}
    	      
    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDOK: 
            SetViewport ((short)lParam);
            if (CurView == 	MyVP)
            	goto Redisplay; 
           }
          break;
         

    default:
{
#if ENABLETRACE
GSSiExitProg (754);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (754);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL LOC_REFNOMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (842);
#endif
{ 	
	static	char	cRefno[16]="";
	static	BOOL	AddHigh=FALSE; 
	

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (842);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:  
         /* initialize working variables                                */  
Redisplay:
         SetDlgItemText (hWndDlg,IDC_REFNO,cRefno);
		 SendDlgItemMessage (hWndDlg,IDC_ADDTOHIGHLIGHT,BM_SETCHECK,AddHigh,0L);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
		 	case IDOK: 
		 	{    
		 		long	Refno; 
		 		double	rref;
		 		
				 GetDlgItemText (hWndDlg,IDC_REFNO,cRefno,sizeof(cRefno)); 
				 if (_fstrchr (cRefno,'.'))
				 {
				 	rref = atof (cRefno); 
				 	Refno = IDNINT(rref * 100 + ZERO$);
				 }
				 else
				 	Refno = atol (cRefno);  
				 AddHigh = SendDlgItemMessage (hWndDlg,IDC_ADDTOHIGHLIGHT,BM_GETCHECK,0,0);

				 if (PickByRefno(Refno,0,0,-1))
				 {  
				 	if (PickList[0].IsDeleted)
				 	{
			         	Sound (BAD_SOUND);
			         	SetDlgItemText (hWndDlg,IDC_MESS,"This item is deleted");
				 	}
				 	else
				 	{
					 	if (AddHigh) 
	   						AddToHighlightList (PickList[0].Refno,&PickList[0],TRUE);
			            EndDialog(hWndDlg, TRUE);
			        } 
		         }
		         else
		         {
		         	Sound (BAD_SOUND);
		         	SetDlgItemText (hWndDlg,IDC_MESS,"Not found");
		         }
	        }
		 	     break; 
		 	     
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (842);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (842);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL BTREE_REORGMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
	char	Name[132]="", NewName[128], mess[256];	  
	HANDLE	hBT, hBT2;
   	LPGWDHEADER	lpGWDHead;
	//BTHEAD BTHead; 
	long	TotRecs,Done;
	short	pos;
	LPSTR	pKey, pData, lpDot;
	HANDLE	hKey, hData;   
	BTVARDESC   BTVar[8];
	
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
    
			
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);
            break;  
            
            case IDC_FIND_FILE: 
            	 
				 if (!GetFileName2 (hWndDlg,Name,".btr;*.in*;*.rin;filelist.txt",IDS_FILEBTR))
				 	break; 
				 SetDlgItemText (hWndDlg,IDC_FILE_NAME,Name);
				 _fstrlwr (Name);
				 if (_fstrstr (Name,"filelist.txt"))
				 	break;
				 hBT = BT_OPEN (Name, 0, BT_READ, 0);
				 if (hBT)
				 {
	         		 //GetBTHeader (hBT,&BTHead);  
	         		 sprintf (mess,"%ld bytes for %ld records in %i levels",GetBT_LENGTH(hBT),BT_NUM_IN_INDEX(hBT),BT_NUMLEVELS_IN_INDEX(hBT));  
	         		 SetDlgItemText (hWndDlg,IDC_MESS1,mess);
	         		 BT_CLOSE (hBT); 
	         	 }
	         	 else
	         	 	SetDlgItemText (hWndDlg,IDC_MESS1,"Unable to open specified B-Tree");
         		 SetDlgItemText (hWndDlg,IDC_MESS2,0);
         		 break;   
         	
         	case IDC_CAN:
         		 SetContinueProcessing (FALSE);
         		 break;
         		 	 
            case IDOK: 
            	 
				 if (!GetDlgItemText (hWndDlg,IDC_FILE_NAME,Name,sizeof(Name)))
				 	break;
				 SetDlgItemText (hWndDlg,IDC_MESS2,"");
         		 EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_CAN),TRUE);
				 _fstrlwr (Name);
				 if (_fstrstr (Name,"filelist.txt"))
				 {
				 	HFILE	Fid;
					OFSTRUCTGM	OFStruct;
				 	long	TotLen, CurLoc;
				 	
				 	Fid = GSSiOpenFile (Name,&OFStruct,OF_READ);
				 	if (Fid == HFILE_ERROR)
						GSSiMessageBox (0,"Cannot open file",0,MB_ICONEXCLAMATION,0);
					else
					{
				   	 	ShowWindow (GetDlgItem(hWndDlg,IDC_STATUS2),SW_SHOW);
				   	 	TotLen = GSSillseek (Fid,0,2);
				   	 	GSSillseek (Fid,0,0); 
						while (ContinueProcessing && fgetstring (Name,128,Fid))
						{    
							 ExpandText (Name);
							 SetDlgItemText (hWndDlg,IDC_MESS1,Name);
							 CurLoc = GSSillseek (Fid,0,1);
		                     PctBox (GetDlgItem(hWndDlg,IDC_STATUS2), TotLen, CurLoc,0);
							 ReorgBTree (Name,GetDlgItem(hWndDlg,IDC_STATUS));
						}
					}
				 }
				 else
				 {
					 ReorgBTree (Name,GetDlgItem(hWndDlg,IDC_STATUS));
					 hBT = BT_OPEN (Name, 0, BT_READ, 0);
					 if (hBT)
					 {
						 //GetBTHeader (hBT,&BTHead);  
						 sprintf (mess,"%ld bytes for %ld records in %i levels",GetBT_LENGTH(hBT),BT_NUM_IN_INDEX(hBT),BT_NUMLEVELS_IN_INDEX(hBT));  
						 SetDlgItemText (hWndDlg,IDC_MESS2,mess);
		         		 BT_CLOSE (hBT); 
					 }
				 }
				 if (!ContinueProcessing)
				 {  
				 	SetDlgItemText (hWndDlg,IDC_MESS2,"Cancelled by user");
				 	SetContinueProcessing ( TRUE);
				 }
         		 EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_CAN),FALSE);
         		 break;
            
         }
         break;   

    default:
        return FALSE;
   }
 return TRUE;
}  

BOOL WINAPI DoNotify(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
#define BOLDDAY(ds,iDay) if(iDay>0 && iDay<32)\
                            (ds)|=(0x00000001<<(iDay-1))

#define lpnmDS ((NMDAYSTATE *)lParam)
#define MAX_MONTHS 12

   MONTHDAYSTATE mds[MAX_MONTHS];
   INT i, iMax;
   LPNMHDR hdr = (LPNMHDR)lParam;

   switch(hdr->code){
      case MCN_GETDAYSTATE:
         iMax=lpnmDS->cDayState;
                  
         for(i=0;i<iMax;i++){
            mds[i] = (MONTHDAYSTATE)0;
            BOLDDAY(mds[i],15);
         }
         lpnmDS->prgDayState = mds;
         break;
   }
   return FALSE;
}

//Preparing the MONTHDAYSTATE Array
//Both the MCM_SETDAYSTATE message and MCN_GETDAYSTATE notification message require an array of MONTHDAYSTATE values to determine how dates will be displayed. Each month that the control displays must have a corresponding element within the array. 

//To support these messages, your application must properly prepare the array. The following is a simple macro that sets a bit in a MONTHDAYSTATE value for a given day within that month. 

//#define BOLDDAY(ds,iDay) if(iDay>0 && iDay<32)\
//                            (ds)|=(0x00000001<<(iDay-1))

BOOL FAR PASCAL DATELIMITSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1087);
#endif
{ 
	char	str[128], year[8], month[4], day[4];
	static	long	Begtime, Endtime;
	LPSTR	lpEnd, pTAB;	
	struct	tm	tmtime;
	time_t	t, systime;
	int		TabStops[3]={1000,1100},ii;
	short	nchar, iday, Choice, sub;
	static	HANDLE	hSaveBM=0;
	SYSTEMTIME	timerange[2];
	
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1087);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:
        cwCenter(hWndDlg, 0);
        hSaveBM = EnterBlockingWindow (hWndDlg);
      	SendDlgItemMessage (hWndDlg,IDC_DATERANGES,LB_SETTABSTOPS,2,(LPARAM)&TabStops); 
    	Begtime = TimeRangeBeg;
    	Endtime = TimeRangeEnd;  
       	GetCurVal (str,sizeof(str),IDS_FILEDATERANGES); 
        FillList (hWndDlg,IDC_DATERANGES,str,0,0);
		   // Set colors for aesthetics.
		MonthCal_SetColor(GetDlgItem(hWndDlg,IDC_MONTHCALENDAR1), MCSC_BACKGROUND, RGB(175,175,175));
		MonthCal_SetColor(GetDlgItem(hWndDlg,IDC_MONTHCALENDAR1), MCSC_MONTHBK, RGB(248,245,225));
       	SendDlgItemMessage (hWndDlg,IDC_MONTHCALENDAR1,MCM_SETMAXSELCOUNT,365,0); 


    case GSSI_REINITDIALOG:  
    
		if (TimeRangeBeg > 0)	
		{	
			t=TimeRangeBeg;
			tmtime = *localtime (&t);  
			if (tmtime.tm_year > 99)
				sub=100;
			else
				sub=0; 
			sprintf (str,"%2.2i",tmtime.tm_year-sub);
			SetDlgItemText (hWndDlg,IDC_FROMYEAR,str);
			sprintf (str,"%2.2i",tmtime.tm_mon+1);
			SetDlgItemText (hWndDlg,IDC_FROMMONTH,str);
			sprintf (str,"%2.2i",tmtime.tm_mday);
			SetDlgItemText (hWndDlg,IDC_FROMDAY,str);
		}
		if (TimeRangeEnd == LONG_MAX)	
		 	break;   
			t = TimeRangeEnd;
			tmtime = *localtime (&t); 
			if (tmtime.tm_year > 99)
				sub=100;
			else
				sub=0; 
			sprintf (str,"%2.2i",tmtime.tm_year-sub);
			SetDlgItemText (hWndDlg,IDC_TOYEAR,str); 
			sprintf (str,"%2.2i",tmtime.tm_mon+1);
			SetDlgItemText (hWndDlg,IDC_TOMONTH,str);
			sprintf (str,"%2.2i",tmtime.tm_mday);
			SetDlgItemText (hWndDlg,IDC_TODAY,str);
SetCalendar:			
 		UnixTimeToSystemTime(Begtime,&timerange[0]);
		UnixTimeToSystemTime(Endtime,&timerange[1]);
		MonthCal_SetSelRange(GetDlgItem(hWndDlg,IDC_MONTHCALENDAR1), timerange);
        break; /* End of WM_INITDIALOG                                 */

 	case WM_NOTIFY:
		{
			LPNMHDR	pnmh;

			pnmh = (LPNMHDR) lParam; 
			if (pnmh->idFrom == IDC_MONTHCALENDAR1)
			switch (pnmh->code)
			{

				case MCN_SELECT:
					ii=1;
					ii=2;
					break;
/*				case MCN_FIRST:
					ii=1;
					ii=2;
					break;*/
				case MCN_LAST:
					ii=1;
					ii=2;
					break;
				case MCN_SELCHANGE:
					ii=1;
					ii=2;
					break;
				case MCN_GETDAYSTATE:
					ii=1;
					ii=2;
					break;
				default:
					ii=1;
			}
		}
		break;
   case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:  
    	 if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) != IDC_DATERANGES)  
    	 {
	         nchar = GetDlgItemText(hWndDlg,LOWORD(wParam),str,8);
	    	 switch (LOWORD(wParam))
	    	 {
	    	 	case IDC_FROMYEAR: 
	    	 		if (nchar == 2)
	    	 			GoToNextControl (hWndDlg,LOWORD(wParam),IDC_FROMMONTH);
	    	 		break;
	    	 	case IDC_FROMMONTH:
	    	 		if (nchar == 2)
	    	 			GoToNextControl (hWndDlg,LOWORD(wParam),IDC_FROMDAY);
	    	 		break;
	    	 	case IDC_FROMDAY:
	    	 		if (*str == 'l' || *str == 'L')
	    	 		{
						t = Begtime;
						tmtime = *localtime (&t); 
						tmtime.tm_mday = LastDayOfMonth(Begtime); 
						tmtime.tm_isdst = -1;
			    		SetTimeRangeBeg(mktime (&tmtime)); 
		       	 		sprintf (str,"%2.2i",tmtime.tm_mday);
		       	 		SetDlgItemText (hWndDlg,IDC_FROMDAY,str);
	    	 		}
	    	 		if (nchar == 2)
	    	 			GoToNextControl (hWndDlg,LOWORD(wParam),IDC_TOYEAR);
	    	 		break;
	    	 	case IDC_TOYEAR:
	    	 		if (nchar == 2)
	    	 			GoToNextControl (hWndDlg,LOWORD(wParam),IDC_TOMONTH);
	    	 		break;
	    	 	case IDC_TOMONTH:
	    	 		if (nchar == 2)
	    	 			GoToNextControl (hWndDlg,LOWORD(wParam),IDC_TODAY);
	    	 		break;
	    	 	case IDC_TODAY:
	    	 		if (*str == 'l' || *str == 'L')
	    	 		{
						t = Endtime;
						tmtime = *localtime (&t); 
						tmtime.tm_mday = LastDayOfMonth(Endtime); 
						tmtime.tm_isdst = -1;
			    		TimeRangeEnd = mktime (&tmtime); 
		       	 		sprintf (str,"%2.2i",tmtime.tm_mday);
		       	 		SetDlgItemText (hWndDlg,IDC_TODAY,str);
	    	 		}
	    	 		if (nchar == 2)
	    	 			SetFocus (GetDlgItem(hWndDlg,IDOK));
	    	 		break;
	    	 }
	     }	 
         switch(LOWORD(wParam))
         {  
			case IDC_MONTHCALENDAR1:
				ii=1;
				break;

            case IDC_DATERANGES: /* List box                              */
              {
                switch(HIWORD(wParam))
                    {
                     case LBN_DBLCLK:  
				    	PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
                     	break;
                     case LBN_SELCHANGE:
		                  Choice=SendDlgItemMessage(hWndDlg,IDC_DATERANGES,
												    LB_GETCURSEL,0,0); 
		         		  SendDlgItemMessage(hWndDlg,IDC_DATERANGES,LB_GETTEXT,
		         		  					 Choice,(DWORD)&str); 
		         		  if ((pTAB = _fstrchr (str,'\t')))
		         		  {
			         		  *pTAB++ = 0;
			         		  sscanf (pTAB,"%ld %ld",&TimeRangeBeg,&TimeRangeEnd);
				       	 	  PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
				       	  }
		         	}
		    }
		    	break;
		    	
         	case IDC_DATE_ADD: 
         		*str = 0;
           		if (GetTextString (hWndDlg,str,64,"Enter new date identifier",0,0,0,TRUE,TRUE))
           		{   
           			char	Name[144]; 
           			HFILE	Fid;
					OFSTRUCTGM	OFStruct;
           			
			       	GetCurVal (Name,sizeof(Name),IDS_FILEDATERANGES);
			       	if (!ExistFile (Name)) 
			       		Fid = GSSiOpenFile (Name,&OFStruct,OF_CREATE); 
					sprintf (_fstrchr (str,0),"|%ld %ld",Begtime,Endtime);			       		
			       	AppendFile (Name,str);
        			FillList (hWndDlg,IDC_DATERANGES,Name,0,0);
           		}
         		break;
         		
         	case IDC_DATE_REMOVE: 
            {
            	 short	Item;
            	 
                 Item = SendDlgItemMessage((HWND) hWndDlg, IDC_DATERANGES, LB_GETCURSEL,0,0);
                 if (Item != LB_ERR)
	         	 	SendDlgItemMessage(hWndDlg,IDC_DATERANGES,LB_DELETESTRING,Item,0);
			}
         		break;
         		
         	case IDC_DATE_TODAY:  
         		time (&systime);
				tmtime = *localtime (&systime);  
				tmtime.tm_isdst = -1;
				SetTimeRangeBeg(mktime(&tmtime));
				TimeRangeEnd = TimeRangeBeg;
				PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
         		break;
         		
         	case IDC_DATE_YESTERDAY:  
         		time (&systime);
         		systime -= (60L * 60L * 24L);
				tmtime = *localtime (&systime);  
				tmtime.tm_isdst = -1;
				SetTimeRangeBeg(mktime(&tmtime));
				TimeRangeEnd = TimeRangeBeg;
				PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
         		break;
         		
         	case IDC_DATE_THISMONTH:  
         		TimeRangeEnd = time (&systime);
				tmtime = *localtime (&systime); 
				tmtime.tm_mday = 1; 
				tmtime.tm_isdst = -1;
				SetTimeRangeBeg(mktime(&tmtime));
				/*				tmtime.tm_mday = LastDayOfMonth(TimeRangeBeg);
	    		TimeRangeEnd = mktime (&tmtime);*/ 
       	 		PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
         		break;
         		
         	case IDC_DATE_LASTMONTH:  
         		time (&systime);
				tmtime = *localtime (&systime); 
				tmtime.tm_mday = 1; 
				tmtime.tm_isdst = -1;
	    		systime = mktime (&tmtime) - (60L * 60L * 24L * 15L); 
				tmtime = *localtime (&systime); 
				tmtime.tm_mday = 1; 
				tmtime.tm_isdst = -1;
				SetTimeRangeBeg(mktime(&tmtime));
				tmtime.tm_mday = LastDayOfMonth(TimeRangeBeg);
	    		TimeRangeEnd = mktime (&tmtime); 
       	 		PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
         		break;
         		
         	case IDC_DATE_THISYEAR:  
         		TimeRangeEnd = time (&systime);
				tmtime = *localtime (&systime); 
				tmtime.tm_mon = 0; 
				tmtime.tm_mday = 1; 
				tmtime.tm_isdst = -1;
				SetTimeRangeBeg(mktime(&tmtime));
				/*				tmtime.tm_mon = 11;
	    		TimeRangeEnd = mktime (&tmtime); 
				tmtime.tm_mday = LastDayOfMonth(TimeRangeEnd); 
	    		TimeRangeEnd = mktime (&tmtime);*/ 
       	 		PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
         		break;
         		
         	case IDC_DATE_LASTYEAR:  
         		time (&systime);
				tmtime = *localtime (&systime); 
				tmtime.tm_isdst = -1;
				tmtime.tm_mon = 0; 
				tmtime.tm_mday = 1; 
         		systime = mktime (&tmtime) - (60L * 60L * 24L * 182L); 
				tmtime = *localtime (&systime); 
				tmtime.tm_mon = 0; 
				tmtime.tm_mday = 1; 
				tmtime.tm_isdst = -1;
				SetTimeRangeBeg(mktime(&tmtime));
				tmtime.tm_mon = 11;
	    		TimeRangeEnd = mktime (&tmtime); 
				tmtime.tm_mday = LastDayOfMonth(TimeRangeEnd); 
	    		TimeRangeEnd = mktime (&tmtime); 
       	 		PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
         		break;
         		
         	case IDC_DATE_PASTYEAR:  
         		time (&systime);       
         		TimeRangeEnd = systime;
				tmtime = *localtime (&systime);
				tmtime.tm_year--; 
				tmtime.tm_isdst = -1;
         		systime = mktime (&tmtime) + (60L * 60L * 24L); 
				SetTimeRangeBeg(mktime(&tmtime));
				PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
         		break;
         		
         	case IDC_DATE_THISWEEK:  
         		TimeRangeEnd = time (&systime);
				tmtime = *localtime (&systime); 
				tmtime.tm_isdst = -1;
				tmtime.tm_mday -= tmtime.tm_wday; 
				SetTimeRangeBeg(mktime(&tmtime));
				/*				tmtime.tm_mday += 6;
	    		TimeRangeEnd = mktime (&tmtime); */
       	 		PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
         		break;
         		
         	case IDC_DATE_LASTWEEK:  
         		time (&systime); 
         		systime -= (60L * 60L * 24L * 7L);
				tmtime = *localtime (&systime); 
				tmtime.tm_mday -= tmtime.tm_wday; 
				tmtime.tm_isdst = -1;
				SetTimeRangeBeg(mktime(&tmtime));
				tmtime.tm_mday += 6;
				tmtime.tm_isdst = -1;
	    		TimeRangeEnd = mktime (&tmtime); 
       	 		PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
         		break;
         		
         	case IDC_DATE_PASTWEEK:  
         		time (&systime); 
         		systime -= (60L * 60L * 24L * 6L);
				tmtime = *localtime (&systime); 
				tmtime.tm_isdst = -1;
				SetTimeRangeBeg(mktime(&tmtime));
				tmtime.tm_mday += 6;
				tmtime.tm_isdst = -1;
	    		TimeRangeEnd = mktime (&tmtime); 
       	 		PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
         		break;
         		
            case IDC_FROMYEAR:  
            case IDC_FROMMONTH:
            case IDC_FROMDAY:
                 if (HIWORD(wParam) ==  EN_KILLFOCUS)
                 { 
                 	if (!GetDlgItemText (hWndDlg,IDC_FROMYEAR,year,5))
                 	{
//                 		GoToNextControl (hWndDlg,IDC_FROMYEAR,IDC_FROMYEAR); 
                 		break;
                 	} 
                 	if (!GetDlgItemText (hWndDlg,IDC_FROMMONTH,month,3))
                 		break;
                 	GetDlgItemText (hWndDlg,IDC_FROMDAY,day,3);
                 	sprintf (str,"$CLK(%s-%s-1 00:00:00,1)",year,month); 
                 	ExpandText (str);
                 	systime = atol (str); 
                 	iday = atoi (day);
                 	iday = max (1,min (iday,LastDayOfMonth(systime)));
                 	sprintf (str,"$CLK(%s-%s-%i 00:00:00,1)",year,month,iday); 
                 	ExpandText (str);
                 	Begtime = atol (str);
					t = Begtime;
                 	_fstrcpy (str,ctime(&t));
                 	lpEnd = _fstrchr (str,0);
                 	lpEnd--;
                 	*lpEnd = 0;
                 	SetDlgItemText (hWndDlg,IDC_FROMDATETEXT,str);
					goto SetCalendar;
                }
            break;
            case IDC_TOYEAR:  
                 if (HIWORD(wParam) ==  EN_KILLFOCUS)
                 { 
                 	if (!GetDlgItemText (hWndDlg,IDC_TOYEAR,year,5))
                 	{
                 		GetDlgItemText (hWndDlg,IDC_FROMYEAR,year,5);
                 		SetDlgItemText (hWndDlg,IDC_TOYEAR,year);
                 	} 
                 	goto ShowToDate; 
                 }
                 break;
            case IDC_TOMONTH:
                 if (HIWORD(wParam) ==  EN_KILLFOCUS)
                 { 
            
                 	if (!GetDlgItemText (hWndDlg,IDC_TOMONTH,month,3))
                 	{
                 		GetDlgItemText (hWndDlg,IDC_FROMMONTH,month,3);
                 		SetDlgItemText (hWndDlg,IDC_TOMONTH,month);
                 	}
                 	goto ShowToDate;  
                 }
                 break;
            case IDC_TODAY:
                 if (HIWORD(wParam) ==  EN_KILLFOCUS)
                 { 
                 	if (!GetDlgItemText (hWndDlg,IDC_TODAY,day,3))
                 	{
                 		GetDlgItemText (hWndDlg,IDC_FROMDAY,day,3);
                 		SetDlgItemText (hWndDlg,IDC_TODAY,day);
                 	}
                 	goto ShowToDate;  
                 }
                 break;
         ShowToDate:
               		GetDlgItemText (hWndDlg,IDC_TOYEAR,year,5);
               		GetDlgItemText (hWndDlg,IDC_TOMONTH,month,3);
               		GetDlgItemText (hWndDlg,IDC_TODAY,day,3);
                 	sprintf (str,"$CLK(%s-%s-1 00:00:00,1)",year,month); 
                 	ExpandText (str);
                 	systime = atol (str);
                 	iday = atoi (day);
                 	iday = max (1,min (iday,LastDayOfMonth(systime)));
                 	sprintf (str,"$CLK(%s-%s-%i 23:59:59,1)",year,month,iday); 
                 	ExpandText (str);
                 	Endtime = atol (str);
					t = Endtime;
                 	_fstrcpy (str,ctime(&t));
                 	lpEnd = _fstrchr (str,0);
                 	lpEnd--;
                 	*lpEnd = 0;
                 	SetDlgItemText (hWndDlg,IDC_TODATETEXT,str);
					goto SetCalendar;
           	break;
            case IDCANCEL: 
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
            break;      
            
            case IDC_REMOVE_DATE:
            	SetTimeRangeBeg (0);
            	TimeRangeEnd = LONG_MAX;
				MonthBeg = SysMonthFromSymTime (TimeRangeBeg);
				MonthEnd = SysMonthFromSymTime (TimeRangeEnd);
                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
            	break;
            
            case IDOK: 
             	if (!GetDlgItemText (hWndDlg,IDC_FROMYEAR,year,5))
             		break;
             	if (!GetDlgItemText (hWndDlg,IDC_FROMMONTH,month,3))
             		break;
             	if (!GetDlgItemText (hWndDlg,IDC_FROMDAY,day,3))
             		break; 
             	sprintf (str,"$CLK(%s-%s-%s 00:00,1)",year,month,day); 
             	ExpandText (str);
             	SetTimeRangeBeg (atol (str));
             	if (!GetDlgItemText (hWndDlg,IDC_TOYEAR,year,5))
             		break;
             	if (!GetDlgItemText (hWndDlg,IDC_TOMONTH,month,3))
             		break;
             	if (!GetDlgItemText (hWndDlg,IDC_TODAY,day,3))
             		break; 
             	sprintf (str,"$CLK(%s-%s-%s 23:59:59,1)",year,month,day); 
             	ExpandText (str);
             	TimeRangeEnd = atol (str); 
             	if (TimeRangeBeg > TimeRangeEnd)
             	{   
             		GMMessageBox (MSG_DATERANGE,0,MB_ICONEXCLAMATION);
             		break;
             	}
				MonthBeg = SysMonthFromSymTime (TimeRangeBeg);
				MonthEnd = SysMonthFromSymTime (TimeRangeEnd);
                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
         		break;
            
         }
         break;   

    default:
{
#if ENABLETRACE
GSSiExitProg (1087);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1087);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL VEHICLEDEFMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{    
	char	str[144], SymName[34], ColorName[16], CTime[32], FollowID[34], Marker; 
	static	HANDLE	hSaveBM=0;  
	int   	ntab=5, TabStops[5]={52, 88, 112, 144,2000}, Choice,i;
	LPVEHLOCATION	pVehLoc; 
	static	char	CurID[16];
	LPSTR	pBS;
	static	short	iveh; 
	HMENU	EditMenu; 
	POINT	position;
	
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
        hSaveBM = EnterBlockingWindow (hWndDlg);
//    	ntab = loadtabs (TabStops);    
      	SendDlgItemMessage (hWndDlg,IDC_VEHLIST,LB_SETTABSTOPS,ntab,(LPARAM)&TabStops); 
  	case GSSI_REINITDIALOG:
	    
	    GetGlobalCVal ("[%AVLFOLLOW]",FollowID,0);
		SendDlgItemMessage (hWndDlg,IDC_VEHLIST,LB_RESETCONTENT,0,0);
		for (iveh=0;iveh<NumVehicles;iveh++)
		{
			pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);  
			GetDictSymName (pVehLoc->Symbol,SymName);  
			GetColorName (pVehLoc->Color,ColorName);  
			if (pVehLoc->nLoc) 
			{
				time_t t = pVehLoc->LocTime[pVehLoc->nLoc - 1];
				_fstrcpy (CTime,ctime (&t));  
				pBS = _fstrchr (CTime,'\n');
				*pBS = 0;
			}
			else
				*CTime = 0;
			if (!_fstricmp (pVehLoc->Desc,FollowID))
				Marker = '*';
			else
				Marker = ' ';
			sprintf (str,"%c%s\t%s\t%i\t%s\t%s\t%s",Marker,pVehLoc->Desc,SymName,pVehLoc->Size[0],ColorName,CTime,pVehLoc->ID);
			GlobalUnlock (hVehicle[iveh]);
	        SendDlgItemMessage (hWndDlg,IDC_VEHLIST,LB_ADDSTRING,0,(LPARAM)((LPSTR) str));
		}             
		
		 break;                              

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
         break; /* End of WM_CLOSE                                      */

         

    case WM_COMMAND:

        switch(LOWORD(wParam))

           {  
           	  case IDOK:  
           	  {
				for (iveh=0;iveh<NumVehicles;iveh++)
				{
					pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);  
					GetDictSymName (pVehLoc->Symbol,SymName);  
					GetColorName (pVehLoc->Color,ColorName);  
					sprintf (str,"%s|%s|%i|%s",SymName,ColorName,pVehLoc->Size[0],pVehLoc->Desc); 
					WritePrivateProfileString ("VEHICLES",pVehLoc->ID,str,GMIni); 
					GlobalUnlock (hVehicle[iveh]);
				}
              	GSSiEndDialog(hWndDlg, FALSE,hSaveBM);    
              }
              	break; 
              	
              case 10001:
				pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);  
				GetTextString (hWndDlg,pVehLoc->Desc,32,"Enter the new label",0,0,0,TRUE,TRUE);
				GlobalUnlock (hVehicle[iveh]); 
                PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
              	break;

              case 10002:
				pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);  
				GetDictSymName (pVehLoc->Symbol,SymName);  
				GetTextString (hWndDlg,SymName,32,"Select the new symbol","[%DL]vehsyms.txt",0,0,TRUE,TRUE);
				pVehLoc->Symbol = GetDictSymbolNumber (SymName);
				GlobalUnlock (hVehicle[iveh]); 
                PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
              	break;

              case 10003:
				pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);  
				ltoa (pVehLoc->Size[0],str,10);  
				GetTextString (hWndDlg,str,8,"Select the new size","[%DL]vehsize.txt",0,0,TRUE,TRUE);
				pVehLoc->Size[0] = atol (str);
				pVehLoc->Size[1] = pVehLoc->Size[0] / 2;
				GlobalUnlock (hVehicle[iveh]); 
                PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
              	break;

              case 10004:
				pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);  
				GetColorName (pVehLoc->Color,str);  
				GetTextString (hWndDlg,str,32,"Select the new color","[%DL]colordef.txt",0,0,TRUE,TRUE);
				pVehLoc->Color = atol (str);
				GlobalUnlock (hVehicle[iveh]); 
                PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
              	break;

              case 10005:
				pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]); 
				SetGlobalValue("%AVLFOLLOW",pVehLoc->Desc);  
				GlobalUnlock (hVehicle[iveh]); 
                PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L); 
				setDoPaint(TRUE);
				PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
              	break;

              case 10006:
              	break;
              		
	          case IDC_VEHLIST:
	#if WIN32
	          switch(LOWORD(wParam))
	#else
	          switch(HIWORD(wParam))
	#endif
	          {
                case LBN_SELCHANGE:
	            {
                    Choice=(short)SendDlgItemMessage(hWndDlg,IDC_VEHLIST,LB_GETCURSEL,0,0); 
                    if (Choice >= 0)
                    {
						SendDlgItemMessage (hWndDlg,IDC_VEHLIST,LB_GETTEXT,Choice,(LPARAM)str);  
						pBS = _fstrrchr (str,'\t');
						*pBS++=0;
						_fstrcpy (CurID,pBS);
						for (iveh=0;iveh<NumVehicles;iveh++)
						{
							pVehLoc = (LPVEHLOCATION)GlobalLock (hVehicle[iveh]);  
							i = _fstricmp (CurID,pVehLoc->ID);
							GlobalUnlock (hVehicle[iveh]); 
							if (!i)
								break;
						}
			      		EditMenu = CreatePopupMenu();
			      		AppendMenu (EditMenu,MF_ENABLED|MF_STRING,10001,"Change Label");
			      		AppendMenu (EditMenu,MF_ENABLED|MF_STRING,10002,"Change Symbol");
			      		AppendMenu (EditMenu,MF_ENABLED|MF_STRING,10003,"Change Size");
			      		AppendMenu (EditMenu,MF_ENABLED|MF_STRING,10004,"Change Color");
			      		AppendMenu (EditMenu,MF_ENABLED|MF_STRING,10005,"Track This Vehicle");
			      		AppendMenu (EditMenu,MF_ENABLED|MF_STRING,10006,"Cancel");
					   	GetCursorPos (&position);
						TrackPopupMenu (EditMenu,TPM_RIGHTBUTTON,position.x,position.y,0,hWndDlg,0);
				      	DestroyMenu (EditMenu); 
                    }
	             }
                 break;
           	 }
             break; 
           }
         break;
    default:
        return FALSE;
   } 
 return TRUE;
}

BOOL FAR PASCAL DIGCONTROLMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
	static	short	Choice;
	static	BOOL	AutoDig=FALSE;
	BOOL	Show; 
	LPDIGCNTLPOINT pDigCtlPnt;	
	int   	TabStops[4]={20,30,40,1000}, i, npts;
	char	str[128];  
	LPDOUBLE XDIG,YDIG,XBASE,YBASE;   
	HANDLE	hCoord;   
	float	RSQMIN;   
	OFSTRUCTGM	OFStruct;
	HFILE	FidCntl;
    
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
    	hWndDigControl = hWndDlg;
		SendDlgItemMessage (hWndDlg,IDC_CONTROLPNTS,LB_SETTABSTOPS,4,(LPARAM)&TabStops);
		FidCntl = GSSiOpenFile ("digsetup.bin",&OFStruct,OF_READ);
		if (FidCntl != HFILE_ERROR)
		{
			BigRead (FidCntl,(HPSTR)&nDigControlPoints,2);
			hDigControlPoints = GSSiGlobAlloc ( 597,GHND,USHRT_MAX);  
	        pDigCtlPnt = (LPDIGCNTLPOINT)GlobalLock (hDigControlPoints); 
	        BigRead (FidCntl,(HPSTR)pDigCtlPnt,nDigControlPoints*sizeof(DIGCNTLPOINT));
	        GSSiClose2 (&FidCntl);  
	        GlobalUnlock (hDigControlPoints);
	    }
    case GSSI_REINITDIALOG:
		SendDlgItemMessage (hWndDlg,IDC_CONTROLPNTS,LB_RESETCONTENT,0,0); 
		if (!hDigControlPoints)
		{
			hDigControlPoints = GSSiGlobAlloc ( 598,GHND,USHRT_MAX);  
			nDigControlPoints = 0;
			Choice = -1;  
            EnableWindow (GetDlgItem(hWndDlg,IDC_SETDIGITIZERCOORDAUTO),FALSE); 
		}
		else
		{ 
            EnableWindow (GetDlgItem(hWndDlg,IDC_SETDIGITIZERCOORDAUTO),TRUE); 
			hDigControlPoints = GSSiGlobalReAlloc (0,hDigControlPoints,USHRT_MAX,GHND);
		} 
        pDigCtlPnt = (LPDIGCNTLPOINT)GlobalLock (hDigControlPoints);  
        hCoord = GSSiGlobAlloc ( 599,GMEM_MOVEABLE,(1+nDigControlPoints)*4*sizeof(double));
        XDIG = (LPDOUBLE)GlobalLock (hCoord);
        YDIG = XDIG + nDigControlPoints; 
        XBASE = YDIG + nDigControlPoints; 
        YBASE = XBASE + nDigControlPoints; 
        npts = 0;
		DisplayMarkers = TRUE;
		for (i=0;i<nDigControlPoints;i++,pDigCtlPnt++)
		{	
			sprintf (str,"%i\t%c\t%c",i+1,pDigCtlPnt->HaveWorld,pDigCtlPnt->HaveDig);
			SendDlgItemMessage (hWndDlg,IDC_CONTROLPNTS,LB_ADDSTRING,0,(LPARAM)str);   
			if (pDigCtlPnt->HaveWorld != ' ' && pDigCtlPnt->HaveDig != ' ')
			{   
				*(XDIG+npts) = pDigCtlPnt->DigitizerPoint.x;
				*(YDIG+npts) = pDigCtlPnt->DigitizerPoint.y;
				*(XBASE+npts) = pDigCtlPnt->WorldPoint.x;
				*(YBASE+npts) = pDigCtlPnt->WorldPoint.y;
				npts++;
			}
			*_fstrchr (str,'\t') = 0;  
			if (pDigCtlPnt->HaveWorld != ' ')
				DisplayMarker (pDigCtlPnt->WorldPoint,1,str,0.20,0,0,FALSE,FALSE,0,0,0,0,0);
		}
		DisplayMarkers = FALSE;           
		GlobalUnlock (hDigControlPoints); 
		CloseTRANS2 (&hTranDigToWorld);  
		if (npts > 2)  
		{
			hTranDigToWorld = STRAN2 (1637,XDIG,YDIG,XBASE,YBASE,npts,&RSQMIN,1,0); 
			sprintf (str,"RSQ = %f",RSQMIN);
			SetDlgItemText (hWndDlg,IDC_MESS,str);
		}
		else
			SetDlgItemText (hWndDlg,IDC_MESS,"Digitizer transformation not set");
		GSSiGlobUlFree (&hCoord);
 		SendDlgItemMessage (hWndDlg,IDC_CONTROLPNTS,LB_SETCURSEL,(WPARAM)Choice,(LPARAM)0); 
		if (Choice < 0)
		{
			EnableWindow (GetDlgItem(hWndDlg,IDC_SETWORLDCOORD),FALSE);
			EnableWindow (GetDlgItem(hWndDlg,IDC_SETDIGITIZERCOORD),FALSE);
			EnableWindow (GetDlgItem(hWndDlg,IDC_DELETE_CP),FALSE);  
		} 
		
        break;  
   
    case WM_DESTROY:
		 if (!nDigControlPoints)
		 {
			GSSiGlobFree (&hDigControlPoints);
			GSSiRemove ("digsetup.bin");
		 }
		 else 
		 {
			FidCntl = GSSiOpenFile ("digsetup.bin",&OFStruct,OF_CREATE);
			BigWrite (FidCntl,(HPSTR)&nDigControlPoints,2,-1);
	        pDigCtlPnt = (LPDIGCNTLPOINT)GlobalLock (hDigControlPoints); 
	        BigWrite (FidCntl,(HPSTR)pDigCtlPnt,nDigControlPoints*sizeof(DIGCNTLPOINT),-1);
	        GSSiClose2 (&FidCntl);  
	        GSSiGlobUlFree (&hDigControlPoints);
	     }
		 	   
         hWndDigControl = 0; 
         WantDigWorldCtlPnt = FALSE;
         WantDigDigCtlPnt = FALSE;
    	 return 0;
    	 
    case WM_CLOSE:
		 DestroyWindow (hWndDlg);
         break;  

    case WM_COMMAND:

         switch(LOWORD(wParam))

         {  
            case IDOK: 
		         PostMessage(hWndDlg, WM_CLOSE, 0, 0L);
            	 break; 
            	 
            case IDC_ADD_CP: 
	             pDigCtlPnt = (LPDIGCNTLPOINT)GlobalLock (hDigControlPoints);
	             pDigCtlPnt+= nDigControlPoints; 
	             pDigCtlPnt->HaveWorld = pDigCtlPnt->HaveDig = ' ';  
	             GlobalUnlock (hDigControlPoints);
            	 Choice = nDigControlPoints++; 
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
            	 
            	 break;
            
            case IDC_DELETE_CP: 
		         pDigCtlPnt = (LPDIGCNTLPOINT)GlobalLock (hDigControlPoints); 
				 for (i=0;i<nDigControlPoints-1;i++,pDigCtlPnt++)
		  		 {
		  		 	if (i >= Choice)
		  		 		*pDigCtlPnt = *(pDigCtlPnt+1);
				 }
            	 nDigControlPoints--;
				 Choice = -1;
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
            	 
            	 break;
            
            case IDC_SETWORLDCOORD: 
            	 if (WantDigWorldCtlPnt)
            	 {
	            	 pDigCtlPnt = (LPDIGCNTLPOINT)GlobalLock (hDigControlPoints);
	            	 pDigCtlPnt+= (Choice);
	            	 pDigCtlPnt->WorldPoint = DigWorldControlPoint;      
	            	 pDigCtlPnt->HaveWorld = 'W';
	            	 GlobalUnlock (hDigControlPoints);
                     EnableWindow (GetDlgItem(hWndDlg,IDC_SETWORLDCOORD),TRUE); 
                     WantDigWorldCtlPnt = FALSE;
                 	 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
				 }
				 else
				 { 
					 SetDlgItemText (hWndDlg,IDC_MESS,"Waiting for world control point");
				 	 WantDigWorldCtlPnt = TRUE;
                     EnableWindow (GetDlgItem(hWndDlg,IDC_SETWORLDCOORD),FALSE);
				 }
            	 break;
            	 
            case IDC_SETDIGITIZERCOORDAUTO:
            	 Choice = 0;
            	 AutoDig = TRUE; 
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);  
				 PostMessage(hWndDigControl, WM_COMMAND, IDC_SETDIGITIZERCOORD, 0L);
            	 break;
            	 
            case IDC_SETDIGITIZERCOORD: 
            	 if (WantDigDigCtlPnt)
            	 {
	            	 pDigCtlPnt = (LPDIGCNTLPOINT)GlobalLock (hDigControlPoints);
	            	 pDigCtlPnt+= (Choice);
	            	 pDigCtlPnt->DigitizerPoint = CurrentDigPoint;      
	            	 pDigCtlPnt->HaveDig = 'D';
	            	 GlobalUnlock (hDigControlPoints);
                     EnableWindow (GetDlgItem(hWndDlg,IDC_SETDIGITIZERCOORD),TRUE); 
                     WantDigDigCtlPnt = FALSE;
                 	 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);  
                 	 if (AutoDig)
                 	 {
                 	 	Choice++;
                 	 	if (Choice < nDigControlPoints) 
                 	 	{   
                 	 		sprintf (str,"Locate digitizer point %i",Choice+1);
                 	 		SetDlgItemText (hWndDlg,IDC_MESS2,str);
				 			PostMessage(hWndDigControl, WM_COMMAND, IDC_SETDIGITIZERCOORD, 0L);
				 		}
                 	 	else
                 	 	{
                 	 		SetDlgItemText (hWndDlg,IDC_MESS2,"");
                 	 		AutoDig = FALSE;
                 	 		Choice = -1;  
                 	 	}
                 	 }
				 }
				 else
				 { 
					 SetDlgItemText (hWndDlg,IDC_MESS,"Waiting for digitizer control point");
				 	 WantDigDigCtlPnt = TRUE;
                     EnableWindow (GetDlgItem(hWndDlg,IDC_SETDIGITIZERCOORD),FALSE);
				 }
            	 break;
            
            case IDC_CLEAR_CP:
				 if (MessageBox( GetFocus(), "Are you sure you wish to clear the current digitizer setup?","Verify Clear",
				     MB_OKCANCEL) == IDCANCEL) break;
            	 GSSiGlobUlFree (&hDigControlPoints);
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
            	 break;
            	 	 
            case IDC_CONTROLPNTS:
                 switch(HIWORD(wParam))
                 {   
                     case LBN_SELCHANGE: 
                         Choice=(short)SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0);
                         if (Choice >= 0)
                         	Show=TRUE;
                         else
                         	Show=FALSE;
                         EnableWindow (GetDlgItem(hWndDlg,IDC_SETWORLDCOORD),Show);
                         EnableWindow (GetDlgItem(hWndDlg,IDC_SETDIGITIZERCOORD),Show);
                         EnableWindow (GetDlgItem(hWndDlg,IDC_DELETE_CP),Show);
	                     WantDigWorldCtlPnt = FALSE;
                         WantDigDigCtlPnt = FALSE;	
		        		 break;
		         }
		         break;
                 
                              
                 
          }
          break;

    default:
        return FALSE;
   }
 return TRUE;    
} 

BOOL FAR PASCAL DECOMPPOLYMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
 char	str[128], txt[128];   
 short  BRtn,i;
 long	NumItems;
 static	LPVIEWPORT	SaveVP;
 static	MNMXCORD	SaveBounds;
 static	BOOL		SaveHB, SaveWIZ; 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    
    case WM_INITDIALOG:   
    	SaveVP = CurView;   
    	SaveBounds = CurView->WBounds; 
       	SaveHB = CurView->HaveBounds;
       	SaveWIZ = CurView->WindowIsZoomed;
        NumItems = BT_NUM_IN_INDEX(hHighlight);  
        if (!NumItems)
        { 
            MessageBox( GetFocus(),"No items highlighted",0, MB_ICONEXCLAMATION);
            EndDialog(hWndDlg, FALSE); 
            break;
        }  
        sprintf(str,"%ld items selected",NumItems);
		SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_SETCHECK,TRUE,0L);
		SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_SETCHECK,TRUE,0L);
		SendDlgItemMessage (hWndDlg,IDC_LINKBETWEENNODES,BM_SETCHECK,DecompLinkBtwnNodes,0L);
        SetDlgItemText(hWndDlg,IDC_PROCESS_MESS,str);   
		SetDlgItemTextGlobal (hWndDlg,IDC_STARTNO,"[%STARTREFNO]","1000000");
        
        if (hDCPSetup)
        { 
        	LPSTR	pstr=GlobalLock (hDCPSetup);  
        	char	Name[128];
        	
        	_fstrcpy (Name,pstr);
        	GetMapName (Name);
            SetDlgItemText (hWndDlg,IDC_DESTMAP,Name); 
            pstr = _fstrchr (pstr,0);
            pstr++;
            SetDlgItemText (hWndDlg,IDC_INTERIOR_LINES,pstr); 
            pstr = _fstrchr (pstr,0);
            pstr++;
            SetDlgItemText (hWndDlg,IDC_EXTERIOR_LINES,pstr); 
        	GSSiGlobUlFree (&hDCPSetup);
        }
        if (DecompAutoRun)
        	PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
        break; /* End of WM_INITDIALOG                                 */
    
   
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */  
         if (Processing)
         	break;
         PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

         switch(LOWORD(wParam))

         {  
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 SetContinueProcessing (FALSE);
                 break;
                 
            case IDC_LOCATE_DESTMAP: 
                 *str=0;
                 if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
                 {
                    if (!GetSaveName2 (hWndDlg,str,IDS_FILTERPLT,".PLT",IDS_FILEPLT)) break;   
                 }
                 else  
                 {
                    if (!GetFileName3 (hWndDlg,str,IDS_FILTERPLT,IDS_FILEPLT)) break;   
                 }
                 SetDlgItemText (hWndDlg,IDC_DESTMAP,str);
                 break;
            case IDC_GET_INTERIOR_SYM:
            	 *str = 0;
                 if (!SelectLineSymbol (hWndDlg,1,str,0,0,FALSE))
                    break;  
                 SetDlgItemText (hWndDlg,IDC_INTERIOR_LINES,str); 
                 break;  
            case IDC_GET_EXTERIOR_SYM: 
            	 *str = 0;
                 if (!SelectLineSymbol (hWndDlg,1,str,0,0,FALSE))
                    break;  
                 SetDlgItemText (hWndDlg,IDC_EXTERIOR_LINES,str); 
                 break;  
                
            case IDOK:
            {
            	LPTHEME	pTheme;
            	short	pos, nareas, n, LineSymbol, IntLineSym, ExtLineSym, Type;  
            	long	npoints, iref, CurItem=0;
                LPSHORT	pPolyParts;
            	HIGHLIGHTDATA	HighlightData; 
				LINESKEY	LinesKey, LinesKey2;
				LINESDATA	LinesData;   
				LPOINT	LastNode, NodePoint;
            	BOOL	First, FirstLoop,HaveMoreNodes=TRUE, HavePOC;
            	HANDLE	hPoints=GSSiGlobAlloc ( 588,GMEM_MOVEABLE,(long)USHRT_MAX*(long)sizeof(DPOINT));
            	HPDPOINT	pDPoint;
                short    NumSyms=0, NoCurvePoints=-1;  
                HANDLE hSymDesc=0;  
                DPOINT	ToPoint, LastPoint, POC;
                char	NewPltName[128]; 
                long	NewRef = 1,ii;  
                USHORT	PointID; 
                LPSHORT	pCurvePoints; 
                BOOL	WantCurves=FALSE, UniqueRefno, LinkBetweenNodes = SendDlgItemMessage (hWndDlg,IDC_LINKBETWEENNODES,BM_GETCHECK,0,0);
                long	StartRefno; 
                double	MinLineLen=2;  
                BOOL	FileIsEditLayer=FALSE;
				int		Typ, nPol;
				double	Offset;
				MNMXCORD	HBounds;
            	
            	if (!GetDlgItemText (hWndDlg,IDC_DESTMAP,NewPltName,sizeof(NewPltName)))
            	{
                    MessageBox(GetFocus(),"No destination map", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
            	}
            	if (!_fstricmp (NewPltName,"EDIT"))
            	{
			   		if (!CurView->UpdateFile)
			   		{
				 		MessageBox(GetFocus(), "No update file in this viewport", 0,MB_ICONEXCLAMATION|MB_OK);
			            break;
			   		} 
			   		else
			   			_fstrcpy (NewPltName,CurView->lpFiles[CurView->UpdateFile-1]); 
			   		FileIsEditLayer = TRUE;
            	}  
            	if (!GetDlgItemText (hWndDlg,IDC_INTERIOR_LINES,str,sizeof(str)))
            	{
                    MessageBox(GetFocus(),"No interior symbol", 0,MB_ICONEXCLAMATION|MB_OK);
                    break;
            	}  
                IntLineSym = GetDictSymbolNumber (str);
                AddToSymList (IntLineSym,&NumSyms,&hSymDesc);
            	if (!GetDlgItemText (hWndDlg,IDC_EXTERIOR_LINES,str,sizeof(str)))
            		ExtLineSym = IntLineSym;
            	else
            	{
	                ExtLineSym = GetDictSymbolNumber (str);
	                AddToSymList (ExtLineSym,&NumSyms,&hSymDesc); 
	            }
            	Processing = TRUE;  
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE); 

                UniqueRefno=SendDlgItemMessage (hWndDlg,IDC_UNIQUEREFNO,BM_GETCHECK,0,0);
			 	if (GetDlgItemText (hWndDlg,IDC_STARTNO,str,128))
				{
					ExpandText (str);
					StartRefno = atol (str);
				}
				else
					StartRefno = 1;        
				GSSiGlobFree (&hHighlightArea);
				GSSiGlobFree (&hHighlightAreaAccelerator[0]); 
				GSSiGlobFree (&hHighlightAreaAccelerator[1]); 
				GSSiGlobFree (&hHighlightAreaAccelerator[2]); 
				hHighlightArea = GetNextHighlightArea (0,0,&Typ,&nHighlightAreaPoints,&nPol,0,&Offset,0);
				HBounds = HLTBounds;
				DecompInit (&HLTBounds);
		        NumItems = TotHLTPoints;  
                SetDlgItemText(hWndDlg,IDC_PROCESS_MESS,"Step1: decomposing polygons");
                pos = BT_FIRST;
		    	if (!LinkBetweenNodes) 
				{   
					HANDLE	hNewPoints=GSSiGlobAlloc ( 589,GMEM_MOVEABLE,USHRT_MAX*sizeof(DPOINT));
					HPDPOINT	pNewPoints;
					char	TempFile[MAX_PATH];
					HFILE	FidTemp;
		   			int		nNewPoints;
					    
					GSSiGetTempFileName (0,"gm",0,(LPSTR)TempFile);
					FidTemp = GSSiOpenFile (TempFile,0,OF_CREATE);
			        NumItems = BT_NUM_IN_INDEX(hHighlight);  
	                while (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData)&&ContinueProcessing)
	                {   
	                	long	npnts;
	                	HANDLE	hPoly;
	                	
	                    pos = BT_NEXT; 
	                    PickList[0]=HighlightData.PD;
	                    if (PickList[0].Type == 2 || PickList[0].Type == 3) 
	                    {
					   		if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&npnts,&hPoly))  
					   		{   
					   			USHORT	ipnt = 1;
					   			
					   			nNewPoints = 0;
			                    pDPoint = (HPDPOINT)GlobalLock (hPoly); 
		                    	pNewPoints = (HPDPOINT)GlobalLock (hNewPoints); 
			                    while (ipnt < npnts)
			                    {
			                    	pNewPoints[nNewPoints++] = pDPoint[ipnt-1];
			                    	pNewPoints[nNewPoints++] = pDPoint[ipnt++];
			                    	while (ipnt < npnts && ldistp (pDPoint[ipnt-1],pDPoint[ipnt]) < MinLineLen)
				                    	pNewPoints[nNewPoints++] = pDPoint[ipnt++]; 
				                    BigWrite (FidTemp,(HPSTR)&nNewPoints,sizeof(nNewPoints),-1);
				                    BigWrite (FidTemp,(HPSTR)pNewPoints,nNewPoints*sizeof(DPOINT),-1);
				                    nNewPoints = 0;
			                    }
			                    GSSiGlobUlFree (&hPoly);
			                    GlobalUnlock (hNewPoints);	
							}
						}
	                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0);
					} 
				    CloseMap(FALSE);  
	                _fstrcpy (PltName,NewPltName);
	                if (!FileIsEditLayer && SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
			    		CreateNewMap (PltName,&HBounds,NumSyms,hSymDesc,0,0,0,0,TRUE);
			    	else 
			    	{
						OpenMap (CurView->hWnd,CurView->hDC);
						EditBounds = CurView->FileMNMX; 
						CloseMap (FALSE);
			    	} 
			    	NumItems = GSSillseek (FidTemp,0,2);
			    	GSSillseek (FidTemp,0,0);
			    	CurItem = 0;
			    	while (ContinueProcessing && BigRead (FidTemp,(HPSTR)&nNewPoints,sizeof(nNewPoints))) 
			    	{
                    	pNewPoints = (HPDPOINT)GlobalLock (hNewPoints);
                    	BigRead (FidTemp,(HPSTR)pNewPoints,nNewPoints*sizeof(DPOINT));
                    	GlobalUnlock (hNewPoints); 
	    				Type = 1;         
	                    LineSymbol = IntLineSym;
	                    NewRef = GetNextRefno (&StartRefno,UniqueRefno,TRUE);
		                if (PolyInHighlightArea (nNewPoints, hNewPoints))
							AddPolyToMap (1,&nNewPoints, &hNewPoints,Type,NewRef,0,2,LineSymbol,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
	                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, GSSillseek (FidTemp,0,1),0);
					}
					GSSiGlobFree (&hNewPoints);   
					GSSiClose2 (&FidTemp);
					GSSiRemove (TempFile);
				}
				else 
				{
	                while (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData)&&ContinueProcessing)
	                {   
	                    pos = BT_NEXT; 
	                    PickList[0]=HighlightData.PD;
	                    if (PickList[0].Type != 2 && PickList[0].Type != 3)
	                        goto NextHlt; 
					    SetConfig (PickList[0].ConfigID);
					    SetViewport (PickList[0].ViewID);
	                    pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME);
	                    CurView->PassID = 4;
						ProcessSelectedTheme = CurView->NumThemes;
						ProcessPickedItem (0,FALSE);        		
						ProcessSelectedTheme = 0;       		
	                    DeleteTheme (pTheme); 
	                    GetSavedPolys ();
	                    if (hSavePoly)
	                    {   LPMNMXCORD lpRect;
	                        int   nPnts,i; 
	                        long	Totp;
	                        HPDPOINT    lpDpoint;
							LPINT	pPolyParts;
							int		nPParts; 
							DPOINT	FirstPoint;
	
	                        if (hSavePolyParts)
	                        {
	                        	pPolyParts = (LPINT)GlobalLock (hSavePolyParts);
	                        	nareas = *pPolyParts++; 
	                        }
	                        else
	                        { 
	                            nareas=1; 
	                            nPParts = nSavePoly; 
	                            pPolyParts = &nPParts;
	                        }
	                            
	                        lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
	                        lpRect++;
	                        lpDpoint = (LPDPOINT) lpRect;
	                        
	                        if (WantCurves && hCurvePoints) 
	                        {
			 		    		pCurvePoints = (LPSHORT)GlobalLock (hCurvePoints);
	                           	lpDpoint = (HPDPOINT)GlobalLock (hUnSplinedPoly);
	                            nPParts = nUnSplinedPoints; 
	                        }
			 		    	else 
			 		    	{
			 		    		GSSiGlobFree (&hCurvePoints);  
			 		    		GSSiGlobFree (&hUnSplinedPoly);
			 		    		pCurvePoints = &NoCurvePoints;
			 		    	}
	
	                        FirstLoop = TRUE; 
	                        PointID=0;
	                        while (nareas--)
	                        {   
	                        	DPOINT	FirstPoint, LastPoint;  
	                        	
	                        	HavePOC = FALSE;
	                        	
	                        	First = TRUE;
		                        while ((*pPolyParts)--)
		                        {   
		                                
		                            if (First)
		                                FirstPoint = LastPoint = *lpDpoint;
		                            else if (PointID == *pCurvePoints)
		                            {
		                            	HavePOC = TRUE;
		                            	POC = *lpDpoint;
		                            	pCurvePoints++;
		                            } 
		                            else
		                            {   
		                            	if (HavePOC)
											DecompAddLine (&LastPoint,&POC,lpDpoint,iref,TRUE,LinkBetweenNodes); 
										else 
										{
											DPOINT	MidP=MidPointD(LastPoint,*lpDpoint);
											   
											DecompAddLine (&LastPoint,&MidP, lpDpoint,iref,FALSE,LinkBetweenNodes);
										}     
										HavePOC = FALSE;
										LastPoint = *lpDpoint;
									}
		                            First = FALSE;   
		                            lpDpoint++;  
		                            PointID++;
		                        }
		                        if (HavePOC)
									DecompAddLine (&LastPoint,&POC,&FirstPoint,iref,TRUE,LinkBetweenNodes); 
								else if (PickList[0].Type == 3 && !SameDPoint (&LastPoint,&FirstPoint))
								{
									DPOINT	MidP=MidPointD(LastPoint,FirstPoint);
	
									DecompAddLine (&LastPoint,&MidP,&FirstPoint,iref,FALSE,LinkBetweenNodes);
								} 
								if (nareas)
								{
									DPOINT	MidP=MidPointD(LastPoint,*lpDpoint);
	
									DecompAddLine (&LastPoint,&MidP, lpDpoint,LONG_MIN,FALSE,LinkBetweenNodes);
									if (!FirstLoop)
									{
										LastPoint = *lpDpoint++; 
										MidP=MidPointD(LastPoint,*lpDpoint);
										DecompAddLine (&LastPoint, &MidP,lpDpoint,LONG_MIN,FALSE,LinkBetweenNodes);
									}
									FirstLoop = FALSE;
									pPolyParts++; 
								}
		                    } 
							GlobalUnlock (hSavePolyParts);
							GlobalUnlock (hSavePoly);
							DestroySavedPolys();
	                        GSSiGlobUlFree (&hCurvePoints);
	                        GSSiGlobUlFree (&hUnSplinedPoly);
	                    }
	            NextHlt:
	            		CurItem+=HighlightData.PD.NumPoints;                    
	                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem,0);
	                } 
	                
	                SetDlgItemText(hWndDlg,IDC_PROCESS_MESS,"Step 2: locating nodes");
				    CloseMap(FALSE);  
	                _fstrcpy (PltName,NewPltName);
	                if (SendDlgItemMessage (hWndDlg,IDC_NEWMAP,BM_GETCHECK,0,0)) 
			    		CreateNewMap (PltName,&HBounds,NumSyms,hSymDesc,0,0,0,0,TRUE);
			    	else 
			    	{
						OpenMap (CurView->hWnd,CurView->hDC);
						EditBounds = CurView->FileMNMX; 
						CloseMap (FALSE);
			    	} 
			    	
	                pos = BT_FIRST;
	                n=0;
	        		LastNode.x = LONG_MAX;
	        		LastNode.y = LONG_MIN;
	        		CurItem = 0;
	        		NumItems = BT_NUM_IN_INDEX(hLines);  
			    	{
		                while (!BT_FIND (hLines,(LPSTR)&LinesKey,pos,BT_ANY,(LPSTR)&LinesData)&&ContinueProcessing)
		                {   
		                    pos = BT_NEXT; 
		                	if (LinesKey.FromNode.x == LastNode.x && LinesKey.FromNode.y == LastNode.y)
		                		n++;
		                	else
		                	{
		                		if (n>2)
		                			BT_PUT (hNodes,(LPSTR)&LastNode,(LPSTR)&n); 
		                		n=1;
		                		LastNode = LinesKey.FromNode;
		                	}
		                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0);
		                }
		        		if (n>2)
		        			BT_PUT (hNodes,(LPSTR)&LastNode,(LPSTR)&n); 
		       			
		                SetDlgItemText(hWndDlg,IDC_PROCESS_MESS,"Step 3: creating output file");
		        		NumItems = BT_NUM_IN_INDEX(hNodes);  
			NextNode:   
						if (!HaveMoreNodes)
						{
							if (BT_FIND (hLines,(LPSTR)&LinesKey,BT_FIRST,BT_ANY,(LPSTR)&LinesData))
								goto Exit;
							pDPoint = (HPDPOINT)GlobalLock (hPoints); 
							npoints = 0;  
							goto GotNextPoint;
						}
						
						if (!ContinueProcessing)
							goto Exit;
						CurItem = NumItems - BT_NUM_IN_INDEX(hNodes);  
		                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem,0);
						if (BT_FIND (hNodes,(LPSTR)&LinesKey,BT_FIRST,BT_ANY,(LPSTR)&n))
						{
							HaveMoreNodes=FALSE;
							goto NextNode;
						}
						n--;
						if (n)
		                	BT_PUT (hNodes,(LPSTR)&LinesKey,(LPSTR)&n); 
		                else
		                	BT_DELETE (hNodes,(LPSTR)&LinesKey,(LPSTR)&n,FALSE);
						pDPoint = (HPDPOINT)GlobalLock (hPoints); 
						npoints = 0;
			NextPoint:
						LinesKey.ToNode.x = LinesKey.ToNode.y = LONG_MIN;
						LinesKey.POCNode.x = LinesKey.POCNode.y = LONG_MIN;
						LastPoint = LinesData.ToPoint;
						if (BT_FIND (hLines,(LPSTR)&LinesKey,BT_FIRST,BT_GE,(LPSTR)&LinesData))
						{
							if (HaveMoreNodes)
							{
			                    GlobalUnlock (hPoints);
								goto NextNode; 
							}
							goto LastLoop; 
						}  
						if (!HaveMoreNodes && (LinesData.FromPoint.x != LastPoint.x || 
											   LinesData.FromPoint.y != LastPoint.y))
							goto LastLoop;
			GotNextPoint: 
						*pDPoint++ = LinesData.FromPoint; 
						npoints++; 
						if (LinesData.IsCurve)
						{
							HavePOC = TRUE;
							POC = LinesData.POCPoint;
						}
						else
							HavePOC = FALSE;
						ToPoint = LinesData.ToPoint; 
						BT_DELETE (hLines,(LPSTR)&LinesKey,(LPSTR)&LinesData,FALSE);
						LinesKey2.FromNode = LinesKey.ToNode;
						LinesKey2.POCNode = LinesKey.POCNode;
						LinesKey2.ToNode = LinesKey.FromNode; 			
						BT_DELETE (hLines,(LPSTR)&LinesKey2,(LPSTR)&LinesData,FALSE);
						NodePoint = LinesKey.ToNode;
						if (!BT_FIND (hNodes,(LPSTR)&NodePoint,BT_FIRST,BT_EQ,(LPSTR)&n))
						{ 
							n--;
							if (n)
			                	BT_PUT (hNodes,(LPSTR)&NodePoint,(LPSTR)&n); 
			                else
			                	BT_DELETE (hNodes,(LPSTR)&NodePoint,(LPSTR)&n,FALSE);
		    LastLoop:       
		    				if (HavePOC)
		    				{
								*pDPoint++ = POC;
								npoints++;   
								Type = 3;
		    				}
		    				else
		    					Type = 1;         
							*pDPoint++ = ToPoint;
							npoints++; 
		                    GlobalUnlock (hPoints);
		                    if (LinesData.LeftPoly == LONG_MIN || LinesData.RightPoly == LONG_MIN)
								goto NextNode;
		                    LineSymbol = IntLineSym;
		                    if (LinesData.LeftPoly == LONG_MAX || LinesData.RightPoly == LONG_MAX)
		                    	LineSymbol = ExtLineSym;
		                    NewRef = GetNextRefno (&StartRefno,UniqueRefno,TRUE);  
		                    if (PolyInHighlightArea (npoints,hPoints))
								AddPolyToMap (1,&npoints, &hPoints,Type,NewRef,0,2,LineSymbol,0,0,0,-1,-1,-1,0,0,0,0,TRUE,0);
							goto NextNode;
						}
						else
						{
							LinesKey.FromNode = LinesKey.ToNode;
							goto NextPoint;
						} 
					}
				}
	Exit:	    
				if (ContinueProcessing)
		        	SetDlgItemText(hWndDlg,IDC_PROCESS_MESS,"Processing finished");
		        else
		        	SetDlgItemText(hWndDlg,IDC_PROCESS_MESS,"Processing aborted");
			    CloseMap(TRUE);  
			 	AddSymToMap (NumSyms,hSymDesc,0,0); 
                DestroySymList (&NumSyms,&hSymDesc);
				GSSiGlobFree (&hPoints);
				GSSiGlobFree (&hHighlightAreaAccelerator[0]); 
				GSSiGlobFree (&hHighlightAreaAccelerator[1]); 
				GSSiGlobFree (&hHighlightAreaAccelerator[2]); 
				GSSiGlobFree (&hHighlightArea);
                DecompClose();
            	
            	Processing = FALSE;
            	SetContinueProcessing ( TRUE);
                EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);  
                if (DecompAutoRun)
	                PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
            } 
                break;
                 
            case IDC_EXIT: 
            	CurView = SaveVP;
            	CurView->WBounds = SaveBounds;   
            	CurView->HaveBounds = SaveHB;
            	CurView->WindowIsZoomed = SaveWIZ;
                EndDialog(hWndDlg, TRUE); 
                break;
                 
          }
          break;

    default:
        return FALSE;
   }
 return TRUE;    
} 

 BOOL FAR PASCAL BMP_OUTPUTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
    long NumItems, nbufs;  
    char    File[128],  ExtID[32], Name[128], str[128], project[34];
    LPSTR   lpDot;   
    HFILE   Fid;
    OFSTRUCTGM    OFStruct; 
    static	MNMXCORD 	SaveBounds; 
    double	Res, Offset, BaseRes=1;
    static	short	SaveMaxOrtho, SaveFastOrthos;

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
    
        NumItems = BT_NUM_IN_INDEX(hHighlight);  
        if (!NumItems)
        { 
            GSSiMsgBox( GetFocus(),"No items highlighted","Error", MB_OK,0);
            PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
            break;
        }  
         sprintf(str,"%ld items selected",NumItems);      
         SetDlgItemText(hWndDlg,IDC_BASERESOLUTION,"1");
         SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,str);  
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         _fstrcpy (str,"*.CVT");
         DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE);
		 CloseOrthos(TRUE);
         SaveMaxOrtho = GetGlobalLVal ("[%ORTHO_BUFFERS]");
         nbufs = GetGlobalLVal ("[%CVTBUFFERS]");
         if (!nbufs)
         	nbufs = 64;   
		 SetGlobalValueLong ("%ORTHO_BUFFERS",nbufs);
         SaveFastOrthos = FastOrthos;
         FastOrthos = FALSE;
         SaveBounds = CurView->WBounds;
         PostMessage(hWndMain, WM_COMMAND, IDM_Z_REDRAW, 0L);
         break;  

    case WM_CLOSE:
         PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
         break; 

    case WM_COMMAND:

         switch(LOWORD(wParam))

           { 
            case IDCANCEL:
                 if (Processing)
					 SetContinueProcessing(FALSE);
                 Processing = FALSE;
                 break;
                 
            case IDC_EXIT:     
				CloseOrthos(TRUE);
				 SetViewport(*pCommandViewport);
                 SetContinueProcessing ( TRUE);
		         FastOrthos = SaveFastOrthos;
				 SetGlobalValueLong ("%ORTHO_BUFFERS",SaveMaxOrtho);
        		 _chdir (OriginalDir);
            	 _chdrive (OriginalDrive);
                 EndDialog(hWndDlg, TRUE);  
				 PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
                 break;
                 
            case IDC_SELECT_OUTPATH:
            {   
                char    Name[128]="";
                if (GetSaveName2 (hWndDlg,Name,IDS_FILTERBMP,".BMP",IDS_FILEBMP))
                    SetDlgItemText (hWndDlg,IDC_OUT_FILE,Name);
            }
                break;
             
            case IDOK: 
            {   
            	short	units;
            	
				SetViewport(*pCommandViewport);
                GetDlgItemText (hWndDlg,IDC_OUT_FILE,str,sizeof(str));
                if (!GetDlgItemText (hWndDlg,IDC_PROJECTION,project,sizeof(project)))
                {
                    GSSiMsgBox(GetFocus(),"No output projection set", 0,MB_ICONEXCLAMATION|MB_OK,0);
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
                        units = PRJ_UNITS[3] = 1;
                    else if (!_fstrcmp(str,"Meters"))
                        units = PRJ_UNITS[3] = 2;
                }
                else
                {
                    GSSiMsgBox(GetFocus(),"Units field not set", 0,MB_ICONQUESTION|MB_OK,0);
                    break;
                }
                GetDlgItemText (hWndDlg,IDC_RESOLUTION,str,sizeof(str));
                Res = atof (str);  
                if (!Res)
                {
                    GSSiMsgBox(GetFocus(),"Resolution not set", 0,MB_ICONQUESTION|MB_OK,0);
                    break;
                }
                GetDlgItemText (hWndDlg,IDC_BASERESOLUTION,str,sizeof(str));
                BaseRes = ConvertInDist (atof (str),units);  
                if (!BaseRes)
                {
                    GSSiMsgBox(GetFocus(),"Base resolution not set", 0,MB_ICONQUESTION|MB_OK,0);
                    break;
                }
                GetDlgItemText (hWndDlg,IDC_OFFDIST,str,sizeof(str));
                Offset = ConvertInDist (atof (str),units);  
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE); 
				SetContinueProcessing(TRUE);
				Processing = TRUE;
				CloseTRANS2 (&hTranExport[0]);
				CloseTRANS2 (&hTranExport[1]);
                if (GetDlgItemText (hWndDlg,IDC_TRANFILE,File,sizeof(File)))
                {   
                	char	InvertName[140];
                	sprintf (InvertName,"|OPP|%s",File);
					hTranExport[0] = LoadTranFileWithDandT (File);                
					hTranExport[1] = LoadTranFileWithDandT (InvertName);                
                }
                GetDlgItemText (hWndDlg,IDC_OUT_FILE,Name,sizeof(Name));  
                CreateBMPs (Name,Res,BaseRes,Offset,hWndDlg,project,units);  
                Processing = FALSE;
				CloseTRANS2 (&hTranExport[0]);
				CloseTRANS2 (&hTranExport[1]);
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

BOOL FAR PASCAL DXF_OUTPUTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
    short nItems, i, Version=1, NumFields;  
    char    File[128],  ExtID[32], Name[128], str[512];
    LPINT   lpItems;    
    HFILE   Fid;
    OFSTRUCTGM    OFStruct;  
    BOOL    False=FALSE;  
    LPSTR   vbar; 
    char    txt[260], txt2[128], project[34]; 
    long    NumItems;
    static	char	Ext[6], SaveExt[8],DExt[6]; 
    static	short   Filter, FileVarID, OutVarID;  
    static	BOOL	FileIsOpen;
	static	HANDLE	hSaveBM;
    
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
         /* initialize working variables                                */   
        
        if (App)
        {
         PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
         break;  
        }
    	hSaveBM = EnterBlockingWindow (hWndDlg);
		FileIsOpen = FALSE;            
		Filter = IDS_FILTERDXF;
		_fstrcpy (Ext,".DXF");
		_fstrcpy (SaveExt,".DXO"); 
		FileVarID = IDS_FILEDXF;
		OutVarID = IDS_FILEDXO;
		SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
		SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
		SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees");
		_fstrcpy (str,"*.CVT");
		DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE);   
 		SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,(WPARAM)-1,(LPARAM)"baseproj"); 
 		SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SELECTSTRING,(WPARAM)-1,(LPARAM)"Feet"); 
		SendDlgItemMessage (hWndDlg,IDC_INCLUDETABLES,BM_SETCHECK,TRUE,0L);
		NumItems = BT_NUM_IN_INDEX(hHighlight);  
        if (!NumItems)
         { 
            MessageBox( GetFocus(),"No items highlighted","Error", MB_OK);
            PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
            break;
         }  
         sprintf(txt,"%ld items selected",NumItems);
         SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,txt);  
         if (*AutoExportName) 
		 	PostMessage(hWndDlg, WM_COMMAND, IDC_LOAD, 0L); 
    case GSSI_REINITDIALOG:
         SetDlgItemText(hWndDlg,IDC_SQL,MIFOutSQL);
         if (FileIsOpen && *AutoExportName)
	         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
         
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

         switch(LOWORD(wParam))

           { 
             case IDC_SELECT_OUTPATH:
            {   
                LPSTR   lpDot;   
                char    Name[128];
                if (GetSaveName2 (hWndDlg,Name,Filter,Ext,OutVarID))
                {
                    SetDlgItemText (hWndDlg,IDC_OUT_FILE,Name);
                }
            }
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
                     FileIsOpen = TRUE;
                   	 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
                 }
                 
            break;
             case IDC_SAVE:
                 if (!GetSaveName2 (hWndDlg,File,0,SaveExt,FileVarID)) 
                    break;  
                 break; 

            case IDOK: 
            {
                HFILE   FidMIF, FidMID, FidSHP, FidSHPIdx;
                char    IdxName[128];   
                OFSTRUCTGM    OFStruct;  
                short     pos, nTranFile, nParts;  
                LPSTR	lpSC;
                long    CurItem=0, iref, RecNum=0, Index=0, NumDBFRecs=0;
                LPTHEME pTheme;                                        
                HIGHLIGHTDATA   HighlightData;  
                short     nareas;
                DPOINT  CP; 
                BOOL    First, Rtn=FALSE;
                LPSTR	lpDot, lpName; 
                char    SQL[256],Type[32],DBName[128],SymName[34],TableName[34],FldName[34], Layer[34]; 
                HANDLE	hOutRec = GSSiGlobAlloc (1004,GMEM_MOVEABLE,USHRT_MAX);
                LPSTR	OutRec = GlobalLock (hOutRec);
//                SHPPOLYHEADER   SHPPolyHeader;  
//                SHPHEADER   SHPHeader;  
//                SHPRECHEADER SHPRecHeader; 
                LPOPENFILEDATA  FilePtr, FilePtrATT;
                LPOPENSQLDATA   SQLPtr, SQLPtrATT;
                DPOINT	Point,BP,POC,EP;
                HANDLE  hSQL;
                short IDB; 
                double	coordcvt=1, MinX=200000.0,MaxX=700000.0,MinY=10000.0,MaxY=350000.0,MinZ=-100.0,MaxZ=1500.0;  
                long	SHPOff, NumMiss,numpoints, MaxHandle, LocHandle;
                LPFIELDINFO	lpFldInfo;  
                short	ShapeType;
                HANDLE	hIndex=0;
                LPLONG	pIndex;  
                LPINT	pPolyParts; 
                BOOL	SaveDisplaySymbol, WantRec,FirstInRec,FirstTPLField;
                long	nSavePoly2;
                HANDLE	hSavePoly2;
                static	long	debugref=1743054;       
                LPSHORT	pCurvePoints;
                short	Neg1=-1, Pass=0;
                HANDLE	hSurf=0, hLinks=0;  
                HPDPOINT	lpFirstPoint;  
                double	VoidElev;
                LPSTR	pDOT;
                LPMNMXCORD	lpRect;
                HPDPOINT	lpDpoint;
                double	Scale=1, TextAngle, Angle=0;  
                BOOL	WantTables =  SendDlgItemMessage (hWndDlg,IDC_INCLUDETABLES,BM_GETCHECK,0,0); 
                BOOL	UseThemes =  SendDlgItemMessage (hWndDlg,IDC_USETHEMES,BM_GETCHECK,0,0); 
                
                DXFHandle=1;
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
                    if (!_fstrcmp(str,"Feet"))
                        PRJ_UNITS[3] = 1;
                    else if (!_fstrcmp(str,"Meters"))
                        PRJ_UNITS[3] = 2;
                }
                else
                {
                    MessageBox(GetFocus(),"Units field not set", 0,MB_ICONQUESTION|MB_OK);
                    goto Exit;
                }
				CloseTRANS2 (&hTranExport[0]);
				CloseTRANS2 (&hTranExport[1]);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_LOAD),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_SAVE),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE); 
                DisableHalt = TRUE;  
				SetContinueProcessing(TRUE);
				Processing = TRUE;
				NumItems = BT_NUM_IN_INDEX(hHighlight); 
				MaxHandle = 0; 
      			CurItem = 0;
                GetDlgItemText (hWndDlg,IDC_OUT_FILE,Name,sizeof(Name));
                ExpandText (Name); 
                if (*Name && !_fstrchr (Name,'.'))
                	_fstrcat (Name,".dxf");
				if (!makedirectories (Name,FALSE,FALSE))
					FidMIF = HFILE_ERROR;
				else 
				{
                	FidMIF = GSSiOpenFile (Name,0,OF_CREATE);
                }
                if (FidMIF == HFILE_ERROR) 
                {
                    MessageBox(GetFocus(),"Invalid output file name",Name,MB_ICONEXCLAMATION|MB_OK);
                    goto Exit;
                }
                pos = BT_FIRST;
                NumMiss=0;   
                FullCurves = TRUE; 
                SaveDisplaySymbol = DisplaySymbol;
                DisplaySymbol = FALSE; 
                HaveTextPointers = FALSE; 
//Write Header Section
				fputstring ("  0\r\nSECTION\r\n  2\r\nHEADER\r\n  9\r\n$ACADVER\r\n  1\r\nAC1014",FidMIF);
				fputstring ("  9\r\n$INSBASE\r\n 10\r\n0.0\r\n 20\r\n0.0\r\n 30\r\n0.0",FidMIF);
				fputstring ("  9\r\n$EXTMIN\r\n 10",FidMIF);
				Point.x = HLTBounds.xmn;
				Point.y = HLTBounds.ymn;  
               	ConvertCoordExport(&Point); 
				sprintf (str,"%f",Point.x);
				fputstring (str,FidMIF);
				fputstring (" 20",FidMIF);
				sprintf (str,"%f",Point.y);
				fputstring (str,FidMIF);
				fputstring (" 30\r\n0.0",FidMIF);
				fputstring ("  9",FidMIF);
				fputstring ("$EXTMAX",FidMIF);
				fputstring (" 10",FidMIF);
				Point.x = HLTBounds.xmx;
				Point.y = HLTBounds.ymx;  
               	ConvertCoordExport(&Point); 
				sprintf (str,"%f",Point.x);
				fputstring (str,FidMIF);
				fputstring (" 20",FidMIF);
				sprintf (str,"%f",Point.y);
				fputstring (str,FidMIF);
				fputstring (" 30",FidMIF);
				fputstring ("0.0",FidMIF);   
				
				LocHandle = GSSillseek (FidMIF,0,1);
				sprintf (str,"  9\r\n$HANDLING\r\n 70\r\n     1\r\n  9\r\n$HANDSEED\r\n  5\r\n%8.8lX",MaxHandle);
				fputstring (str,FidMIF);
				
				
				fputstring ("  0",FidMIF);
				fputstring ("ENDSEC",FidMIF);  
				
				if (WantTables)
				{   
					short	NumPointSymbols = 0;
					HANDLE	hPointSymbols = GSSiGlobAlloc (1005,GMEM_MOVEABLE,4096);
					LPSHORT	PointSymbols=(LPSHORT)GlobalLock (hPointSymbols);
					
	                SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,"Creating symbol table");
	                while (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData)&&ContinueProcessing)
	                {   
	                	short	idesc;
	                	
                    	pos = BT_NEXT;
                    	if (HighlightData.PD.Type == 1)
                    	{
	                    	if (ConvertPCTSymbols)
	                    	{ 
								GetSymbolName (HighlightData.PD.Desc, SymName,0,1,0); 
								Strip (SymName,'%');
					    		if ((idesc = GetDictSymbolNumber (SymName)))
					    			HighlightData.PD.Desc =  idesc;	
					    	}
                    		for (i=0;i<NumPointSymbols;i++)
                    			if (PointSymbols[i] == HighlightData.PD.Desc)
                    				goto NextSym;
                    		PointSymbols[NumPointSymbols++] = HighlightData.PD.Desc;
                    	}
                NextSym:; 
                    	PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0);
                    }
					DXFOutTables (FidMIF,&DXFHandle,NumPointSymbols,PointSymbols);
					DXFOutBlocks (FidMIF,&DXFHandle,NumPointSymbols,PointSymbols);
					GSSiGlobUlFree (&hPointSymbols); 
				}
				
      			CurItem = 0;
                SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,"Creating extract file");
				fputstring ("  0",FidMIF);
				fputstring ("SECTION",FidMIF);
				fputstring ("  2",FidMIF);
				fputstring ("ENTITIES",FidMIF); 
			 	if (UseThemes)
			 		ThemeBeginDisplayPass(FALSE,CurView->ID);
				pos = BT_FIRST;
                while (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData)&&ContinueProcessing)
                {   
                	long	ii;
                	
                	if (iref == debugref)
                		ii=1;
                    pos = BT_NEXT; 
                    PickList[0]=HighlightData.PD;
                    SetIntRefno (PickList[0].Refno);
                    SetUDIValue (HighlightData.PD.Prefix,HighlightData.PD.UDI);     
                    SetGlobalValue ("%PREFIX",HighlightData.PD.Prefix);  
				    SetConfig (PickList[0].ConfigID);
				    SetViewport (PickList[0].ViewID);
					if (PickList[0].Type == 2 || PickList[0].Type == 3)
					{
	                	if (PickList[0].Type == 3)
	                    	pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME);
	                    else
	                    	pTheme = AddTheme (GF_SAVEPOLY_THEME);
	                    CurView->PassID = 4; 
	                    //WantElement = PickList[0].Element;
						ProcessSelectedTheme = CurView->NumThemes;
	                    ProcessPickedItem (0,FALSE); 
						ProcessSelectedTheme = 0;
	                    WantElement = LONG_MAX;               
	                    DeleteTheme (pTheme);
	                }
			    	if (UseThemes)
			    		ProcessPickedItem (0,-2);
					GetSymbolName (HighlightData.PD.Desc, SymName,0,1,0); 
			    	DXFConvertSymName (SymName);
					Strip (SymName,'%'); 
                    switch (PickList[0].Type)
                    {   //Point","Line","Area","Text","Curve"
                    	
                        case 1: //points     
                        	_fstrcpy (Layer,SymName); 
                    		sprintf (str,"  0\r\nINSERT\r\n  5\r\n%lX\r\n100\r\nAcDbEntity\r\n  8\r\n%s\r\n100\r\nAcDbBlockReference\r\n  2\r\n%s",
                       				     DXFHandle++,Layer,SymName);
                       		fputstring (str,FidMIF);
                        	BP = EP = HighlightData.PD.BeginPoint;
                        	ConvertCoordExport(&BP); 
                        	EP.x += 1;
                        	ConvertCoordExport(&EP); 
                            DXFOutPoint (&BP,10,FidMIF);
                            
                            Scale = HighlightData.PD.Length * ldistp (BP,EP); 
                            sprintf (str," 41\r\n%f\r\n 42\r\n%f\r\n 50\r\n%f",Scale,Scale,HighlightData.PD.BPAZ);
                       		fputstring (str,FidMIF);
                        break;
                        
                        case 2:  
	                    	nParts = GetSavedPolys ();
	                        if (hSavePoly)  
	                        {   
	                        	short	Width = 0;
	                        	
	                            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
	                            lpRect++;
	                            lpDpoint = lpFirstPoint = (LPDPOINT) lpRect; 
	                        	if (nSavePoly == 2)
	                        	{   
	                        		BP = *lpDpoint++;
	                        		EP = *lpDpoint;
	                        	    GlobalUnlock (hSavePoly);
	         DoLine:
	                            	ConvertCoordExport(&BP);
	                            	ConvertCoordExport(&EP);
                                    DXFOutLine (DXFHandle++,SymName,FidMIF,&BP,&EP,Width);
	                        	}
	                        	else
	                        	{
									for (i=0;i<nSavePoly;i++)
		                            	ConvertCoordExport(&lpDpoint[i]);
		                            DXFOutPolyline (&DXFHandle,SymName,FidMIF,nSavePoly,lpDpoint,Width);
	                        	    GlobalUnlock (hSavePoly);
	                        	}
								DestroySavedPolys();
                            }
                        break;  
                        
                        
                        case 5: //curve  
                        {
                        	DPOINT	RP;
                        	double	BackAZ, ForAZ, CLEN, Radius;
                        	
                        	BP = HighlightData.PD.BeginPoint;
                        	ConvertCoordExport(&BP); 
                        	POC = HighlightData.PD.NodePoint;
                        	ConvertCoordExport(&POC); 
                        	EP = HighlightData.PD.EndPoint;
                        	ConvertCoordExport(&EP); 
							if (RCURVE(&BP.x,&BP.y,&POC.x,&POC.y,&EP.x,&EP.y,&RP.x,&RP.y,&CLEN))
								goto DoLine;
                    		sprintf (str,"  0\r\nARC\r\n  5\r\n%lX\r\n100\r\nAcDbEntity\r\n  8\r\n%s\r\n100\r\nAcDbCircle",
                    				     DXFHandle++,SymName); 
	                        fputstring (str,FidMIF); 
                            DXFOutPoint (&RP,10,FidMIF); 
                            Radius = ldistp (RP,BP);
                            if (CLEN > 0)
                            {
                            	BackAZ = AZToDXFAngle (getazd (&RP,&EP));
                            	ForAZ  = AZToDXFAngle (getazd (&RP,&BP));;
                            }
                            else
                            {
                            	BackAZ = AZToDXFAngle (getazd (&RP,&BP));
                            	ForAZ  = AZToDXFAngle (getazd (&RP,&EP));;
                            }
	                        sprintf (str," 40\r\n%f\r\n100\r\nAcDbArc\r\n 50\r\n%f\r\n 51\r\n%f",Radius,BackAZ,ForAZ);
	                        fputstring (str,FidMIF); 
	                    }
                        break; 
                        
                        case 3: //area
	                    	nParts = GetSavedPolys ();
	                        if (hSavePoly)
	                        {   LPMNMXCORD lpRect;
	                            long	Totp,nPnts,i;
	                            HPDPOINT    lpDpoint, lpOutPoint,lpOrigPoint;
	                            DPOINT  FirstPoint, FirstOutPoint;  
	                            HANDLE	hOutPoint=0, hOrigPoint=0;

	                            if (hSavePolyParts)
	                            {
	                            	pPolyParts = (LPINT)GlobalLock (hSavePolyParts); 
	                            	nareas = *pPolyParts++; 
	                            	hIndex = GSSiGlobAlloc (1006,GHND,nareas*4);
	                            	pIndex = (LPLONG)GlobalLock (hIndex);
	                            	i = nareas; 
	                            	Totp = 0;
	                            	while (i--)
	                            	{   
	                            		*pIndex++ = Totp;
	                            		Totp += *pPolyParts++;
	                            	}
	                            	GlobalUnlock (hIndex);  
	                            	GlobalUnlock (hSavePolyParts); 
	                            }
	                            else 
	                            {
	                            	hIndex = 0; 
		                            nareas=1;  
	                            }
	                            
	                            nPnts = nSavePoly; 
	                            pCurvePoints = &Neg1;
                            	numpoints = nPnts - (nareas - 1); 
	                            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
	                            lpRect++;
	                            lpDpoint = lpFirstPoint = (LPDPOINT) lpRect; 

	                            hOutPoint = GSSiGlobAlloc (1007,GMEM_MOVEABLE,(nPnts+1)*(long)sizeof(DPOINT)); 
	                            lpOutPoint = (HPDPOINT)GlobalLock (hOutPoint);
	                            hOrigPoint = GSSiGlobAlloc (1008,GMEM_MOVEABLE,(nPnts+1)*(long)sizeof(DPOINT)); 
	                            lpOrigPoint = (HPDPOINT)GlobalLock (hOrigPoint);
	                            First = TRUE;  
	                            
	                            if (HaveLinkLines (lpDpoint,nPnts,&hLinks))
	                            	ii=1; 
	                            GSSiGlobFree (&hLinks);
                        	    GlobalUnlock (hSavePoly);
	                            while (nPnts--)
	                            {   
	                            	*lpOutPoint = *lpOrigPoint = *lpDpoint;
	                                ConvertCoordExport(lpOutPoint); 
	                                if (First) 
	                                {
	                                	FirstOutPoint = *lpOutPoint;
	                                    FirstPoint = *lpDpoint;     
	                                }
	                                lpDpoint++; 
	                                lpOutPoint++; 
	                                lpOrigPoint++;
	                                First = FALSE;
	                            } 
	                            *lpOutPoint = FirstOutPoint;
	                            *lpOrigPoint = FirstPoint;
	                            GlobalUnlock (hOutPoint);    
	                            GlobalUnlock (hOrigPoint); 
	                        //    numpoints++;
                                if (hIndex)
                                {
                                	pIndex = (LPLONG)GlobalLock (hIndex);
//                                	BigWrite (FidSHP,pIndex,4*SHPPolyHeader.NumParts); 
                                	GlobalUnlock (hIndex);  
                                	lpDpoint = (LPDPOINT) lpRect;
	                            	pPolyParts = (LPINT)GlobalLock (hSavePolyParts); 
                                    pPolyParts++;
                                    i = nareas; 
                                    First=TRUE;
                                    while (i--)
                                    {
//										BigWrite (FidSHP,lpDpoint,sizeof(DPOINT)*(long)(*pPolyParts)); 
										lpDpoint+=*pPolyParts++;
										if (First)
											First=FALSE;
										else                                        
											lpDpoint++;
                                    }
	                            	GlobalUnlock (hSavePolyParts); 
                                }
                                else	
                                {   
                                	FirstInRec=TRUE;
   		                            lpOutPoint = (HPDPOINT)GlobalLock (hOutPoint);
   		                      //      lpOutPoint += numpoints - 1;
   		                            lpOrigPoint = (HPDPOINT)GlobalLock (hOrigPoint);
   		                      //      lpOrigPoint += numpoints - 1;
		                            DXFOutPolygon (&DXFHandle,SymName,FidMIF,numpoints,lpOutPoint);
		                        	GlobalUnlock (hOutPoint);
		                        	GlobalUnlock (hOrigPoint);
                                } 
                                GSSiGlobFree (&hOutPoint);
                                GSSiGlobFree (&hOrigPoint);
                        	}
	                            
                        break;
                    }
                    if (PickList[0].HasText) 
                    {   
                    	LPSHORT	pnText;
                    	LPEXPORTTEXT	pEXText;
                    	double	D1,D2,SizeFactor;
                        	
                    	CurView->PassID = 4;  
                    	hExportText = GSSiGlobAlloc (1009,GHND,4096);
                    	hExportTextPointer = GSSiGlobAlloc (1010,GHND,4096);
	                    ProcessPickedItem (0,FALSE);            
						pnText = (LPSHORT)GlobalLock (hExportText); 
						pEXText = (LPEXPORTTEXT)(pnText+1); 
                        while ((*pnText)--)
                        {
							if (pEXText->Type == 2)
							{   
								DPOINT	RP;
								double	AZ, RAD, CLEN, RadiansPerPixel,cwidth,th,tdc,til,tas,acw,angle;
								LPSTR	pChr = pEXText->Text;
								
								RCURVE (&pEXText->Loc[0].x,&pEXText->Loc[0].y,&pEXText->Loc[1].x,&pEXText->Loc[1].y,
										&pEXText->Loc[2].x,&pEXText->Loc[2].y,&RP.x,&RP.y,&CLEN);
								RAD = ldistp (RP,pEXText->Loc[0]);     
								AZ = getazd (&RP,&pEXText->Loc[0]); 
								RadiansPerPixel = 1e0/RAD;
								while (*pChr)
								{   
									DPOINT	pt = dnewpt (RP,AZ,RAD);  
														
									GetTextWidthAndHeight (pEXText->Font,pEXText->size,pChr,1,&cwidth,&th,&tdc,&til,&tas,&acw);
									AZ = LTWOPI (AZ - (DSIGN(cwidth/2,CLEN))*RadiansPerPixel);
									angle = LTWOPI(AZ-DSIGN(HALFPI,CLEN))/RADDEG;
									pChr++;
								} 
						    }
						    else
						    {
								sprintf (str,"  0\r\nTEXT\r\n  5\r\n%lX\r\n100\r\nAcDbEntity\r\n  8\r\n%s\r\n100\r\nAcDbText",
                    				 	 DXFHandle++,SymName); 
	                        	fputstring (str,FidMIF);   
	                        	if (pEXText->Font > 1)
	                        	{
	                        		sprintf (str,"  7\r\nFONT%i",pEXText->Font);
	                        		fputstring (str,FidMIF);
	                        	}
	                        	BP = pEXText->Loc[0]; 
	                        	EP = pEXText->Loc[2]; 
	                        	D1 = ldistp (BP,EP);
	                        	TextAngle = getazd (&pEXText->Loc[0],&pEXText->Loc[2])*DEGRAD;
	                        	ConvertCoordExport(&BP); 
	                        	ConvertCoordExport(&EP); 
	                        	D2 = ldistp (BP,EP);
	                        	if (D1)
	                        		SizeFactor = D2/D1;
	                        	else
	                        		SizeFactor = 1;  
	                        	SizeFactor *= 0.8;//2005/8/31
                                DXFOutPoint (&BP,10,FidMIF);
                                strncpy0 (txt,pEXText->Text,pEXText->lText);
                                sprintf (str," 40\r\n%f\r\n 50\r\n%f\r\n 72\r\n  0\r\n  1\r\n%s",pEXText->size*SizeFactor,TextAngle,txt);
	                        	fputstring (str,FidMIF); 
                                DXFOutPoint (&BP,11,FidMIF);  
	                        	fputstring ("100\r\nAcDbText\r\n 73\r\n  1",FidMIF); 
	                        } 
		                	pEXText++;
		                }
	                    GlobalUnlock (hExportText);  
	                }
            NextHlt: 
                    GSSiGlobFree (&hExportText);  
                    GSSiGlobFree (&hExportTextPointer);  
            		DestroySavedPolys();                    
                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0);
                } 
				fputstring ("  0\r\nENDSEC",FidMIF);
				if (WantTables)
					DXFOutObjects (FidMIF,&DXFHandle);
				fputstring ("  0\r\nEOF",FidMIF);

				GSSillseek (FidMIF,LocHandle,0);
				sprintf (str,"  9\r\n$HANDLING\r\n 70\r\n     1\r\n  9\r\n$HANDSEED\r\n  5\r\n%8.8lX",DXFHandle);
				fputstring (str,FidMIF);
				
                GSSiClose2 (&FidMIF);
                FullCurves = FALSE;
                DisplaySymbol = SaveDisplaySymbol;
                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0);
		        SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,"Extract finished"); 
		        Rtn=TRUE;
Exit:          
				if (UseThemes)
					ThemeEndDisplayPass (FALSE,FALSE,FALSE);
				CloseTRANS2 (&hTranExport[0]);
				CloseTRANS2 (&hTranExport[1]);
                GSSiGlobUlFree (&hOutRec);
                DisableHalt = FALSE;  
                SetContinueProcessing ( TRUE);   
                Processing = FALSE;
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_LOAD),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_SAVE),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE); 
                if (*AutoExportName)
                {
	                 GSSiEndDialog(hWndDlg,Rtn,hSaveBM); 
	            }
                break;
            }
            case IDC_EXIT:     
                 GSSiEndDialog(hWndDlg,FALSE,hSaveBM); 
                 
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)
					 SetContinueProcessing(FALSE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE); 
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

BOOL SetDGNOutParms (HWND hWndDlg,int iparm,int value)
{
	LPSTR	lpTab;
	int	nItems;
	LPINT	lpItems;
	HANDLE	hItems;
	BOOL	rtn=FALSE;
	char	str[256];
	char	SymName[66],Type[12];
	int		Level,Style,Weight,Font,Color;
	char	ColorC[16];
	LPSTR	loc[8],cloc[3];


	nItems = GetLBSelectedItems (hWndDlg,IDC_LAYER_LIST,&hItems);
	if (nItems)
	{
		 lpItems = (LPINT)GlobalLock(hItems);
		 while (nItems--)
		 {
              SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETTEXT,*lpItems,(DWORD)str);
			  GetParmLoc (7,'\t',str,loc);
			  strcpy (SymName,loc[0]);
			  strcpy (Type,loc[1]);
			  Level = atoi (loc[2]);
			  Style = atoi (loc[3]);
			  Weight = atoi (loc[4]);
			  Font = atoi (loc[5]);
/*			  strcpy (ColorC,loc[6]);
			  GetParmLoc (3,',',ColorC+1,cloc);
			  Color = RGB(atoi(cloc[0]),atoi(cloc[1]),atoi(cloc[2]));*/
			  Color = atoi (loc[6]);
			  SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_DELETESTRING,*lpItems,0);
			  switch (iparm)
			  {
			  case 1:
				  Level = value;
				  break;
			  case 2:
				  Style = value;
				  break;
			  case 3:
				  Weight = value;
				  break;
			  case 4:
				  Font = value;
				  break;
			  case 5:
				  Color = value;
				  break;
			  }
//			  sprintf (ColorC,"(%i,%i,%i)",GetRValue(Color),GetGValue(Color),GetBValue(Color));
			  itoa (Color,ColorC,10);
			  sprintf (str,"%s\t%s\t%i\t%i\t%i\t%i\t%s",SymName,Type,Level,Style,Weight,Font,ColorC);
			  SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_INSERTSTRING,*lpItems,(LPARAM)str);
			  SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_SETSEL,TRUE,*lpItems++);
		 } 
	}
	GSSiGlobUlFree (&hItems);
	return rtn;
}

int AddToColorList (COLORREF Color,LPBYTE ColorTable,LPINT pnColors)
{
	BYTE r=GetRValue (Color), g=GetGValue (Color), b=GetBValue (Color);
	int	i;

	for (i=0;i<*pnColors;i++,ColorTable+=3)
	{
		if (r == *ColorTable && g == *(ColorTable+1) && b == *(ColorTable+2))
			return i;
	}
	*ColorTable = r;
	*(ColorTable+1) = g;
	*(ColorTable+2) = b;
	(*pnColors)++;
	return i;
}

BOOL GetPaletteFromDGN (LPSTR DGNFileIN,LPRGBTRIPLE pColorTable)
{
    DGNHandle   hDGN;
    DGNElemCore *psElement;
	BOOL		rtn=FALSE;
	int	i;
	char		DGNFile[MAX_PATH];

	strcpy (DGNFile,DGNFileIN);

	ExpandText (DGNFile);
	hDGN = DGNOpen(DGNFile, FALSE );
    if( !hDGN)
        return FALSE;

    while( (psElement=DGNReadElement(hDGN)) != NULL )
    {
		if (psElement->level == 1 && psElement->stype ==DGNST_COLORTABLE)
	    {
          DGNElemColorTable *psCT = (DGNElemColorTable *) psElement;

		  for (i=0;i<256;i++)
		  {
			pColorTable[i].rgbtRed = psCT->color_info[i][0];
			pColorTable[i].rgbtGreen = psCT->color_info[i][1];
			pColorTable[i].rgbtBlue = psCT->color_info[i][2];
		  }
		  rtn = TRUE;
		  break;
        }

	}
	DGNClose (hDGN);
	return rtn;
}

BOOL FAR PASCAL DGN_OUTPUTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
    short nItems, i, Version=1, NumFields;  
	int		pos, iref, Choice;
	int		ntabs, tabs1[7]={180,220,250,282,322,354,2000};
    char    File[MAX_PATH], seedFile[MAX_PATH], ExtID[32], Name[128], str[512];
    LPINT   lpItems;    
    HFILE   Fid,FidDSC;
    BOOL    False=FALSE;  
    LPSTR   vbar; 
    HIGHLIGHTDATA   HighlightData; 
	int		Level,Style,Weight,Font;
	COLORREF	Color;
	char	ColorC[16];
    char    txt[260], txt2[128], project[34],SymName[64]; 
    long    NumItems;
    static	char	Ext[6], SaveExt[8],DExt[6]; 
    static	short   Filter, FileVarID, OutVarID;  
    static	BOOL	FileIsOpen;
	static	HANDLE	hSaveBM;
	static	RGBTRIPLE ColorTable[256];
	static	BOOL	HavePalette;
    
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
         /* initialize working variables                                */   
    
		//ntabs = loadtabs (tabs1);
    	hSaveBM = EnterBlockingWindow (hWndDlg);
		HavePalette=FALSE;
		FileIsOpen = FALSE;            
		Filter = IDS_FILTERDGN;
		_fstrcpy (Ext,".DGN");
		_fstrcpy (SaveExt,".DGO"); 
		FileVarID = IDS_FILEDGN;
		OutVarID = IDS_FILEDGO;
		strcpy (str,"[%DL]dgn\\MPLS-3D.dgn");
		ExpandText (str);
		SetDlgItemText (hWndDlg,IDC_SEED_FILE,str);
		SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
		SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
		SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees");
		for (i=1;i<100;i++)
		{
			itoa (i,str,10);
			SendDlgItemMessage (hWndDlg,IDC_DGNPARMLEVEL,CB_ADDSTRING,0,(LPARAM)str);
		}
		for (i=0;i<8;i++)
		{
			itoa (i,str,10);
			SendDlgItemMessage (hWndDlg,IDC_DGNPARMSTYLE,CB_ADDSTRING,0,(LPARAM)str);
		}
		for (i=0;i<32;i++)
		{
			itoa (i,str,10);
			SendDlgItemMessage (hWndDlg,IDC_DGNPARMWEIGHT,CB_ADDSTRING,0,(LPARAM)str);
		}
		for (i=1;i<100;i++)
		{
			itoa (i,str,10);
			SendDlgItemMessage (hWndDlg,IDC_DGNPARMFONT,CB_ADDSTRING,0,(LPARAM)str);
		}
		_fstrcpy (str,"*.CVT");
		DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE);   
 		SendDlgItemMessage (hWndDlg,IDC_PROJECTION,CB_SELECTSTRING,(WPARAM)-1,(LPARAM)"baseproj"); 
 		SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SELECTSTRING,(WPARAM)-1,(LPARAM)"Feet"); 
		SendDlgItemMessage (hWndDlg,IDC_INCLUDETABLES,BM_SETCHECK,TRUE,0L);
		NumItems = BT_NUM_IN_INDEX(hHighlight);  
        if (!NumItems)
         { 
            MessageBox( GetFocus(),"No items highlighted","Error", MB_OK);
            PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
            break;
         }  
         sprintf(txt,"%ld items selected",NumItems);
         SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,txt);
		 
		 SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_SETTABSTOPS,7,(LPARAM)&tabs1);
 		pos = BT_FIRST;
        while (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData))
        {   
            pos = BT_NEXT; 
			GetSymbolName (HighlightData.PD.Desc, SymName,0,1,0); 
			DXFConvertSymName (SymName);
			Strip (SymName,'%');
			sprintf (str,"%s\t%s\t",SymName,TypeName[PickTypeFromSysType(HighlightData.PD.Type)-1]);
			if (SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_FINDSTRING,-1,(LPARAM)str) == LB_ERR)    
			{
				Level = 0;
				Style = 0;
				Weight = 0;
				Font = 32;
				strcpy (ColorC,"0");
				sprintf (strchr (str,0),"%i\t%i\t%i\t%i\t%s",Level,Style,Weight,Font,ColorC);
				SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_ADDSTRING,0,(LPARAM)str);        
			}

		}

         if (*AutoExportName) 
		 	PostMessage(hWndDlg, WM_COMMAND, IDC_RECALL, 0L); 
    case GSSI_REINITDIALOG:
         
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           { 
				case IDC_SAVE:
				{
            		short	item=0;  
            		
					 if (!GetSaveName2 (hWndDlg,str,0,".DGC",IDS_FILEDGC)) break; 
					 FidDSC = GSSiOpenFile (str,0,OF_CREATE);
					 itoa (Version,str,10);
					 fputstring (str,FidDSC);
					 GetDlgItemText (hWndDlg,IDC_OUT_FILE,str,MAX_PATH);
					 fputstring (str,FidDSC);
					 GetDlgItemText (hWndDlg,IDC_SEED_FILE,str,MAX_PATH);
					 fputstring (str,FidDSC);
					 GetDlgItemText (hWndDlg,IDC_PROJECTION,str,MAX_PATH);
					 fputstring (str,FidDSC);
					 GetDlgItemText (hWndDlg,IDC_UNITS,str,MAX_PATH);
					 fputstring (str,FidDSC);
					 item=0;  
              		 while (SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETTEXT,item++,(DWORD)str) != LB_ERR)
              	 		fputstring (str,FidDSC); 
              		 GSSiClose2 (&FidDSC); 
					 SymConvTableChanged = FALSE;  
				}
              	break;
/*		   case IDC_RECALL: 
           		{
           		 
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_READ);
                 BigRead (FidSave,Ext,6);
                 BigRead (FidSave,(HPSTR)&Version,2);   
                 BigRead (FidSave,(HPSTR)DestName,lnDestName);
                 SetDlgItemText (hWndDlg,IDC_DEST_FILE,DestName);
	             BigRead (Fid,project,sizeof(project));
	             SetDlgItemText (hWndDlg,IDC_PROJECTION,project);
	             BigRead (Fid,(HPSTR)&UnitsOpt,2);
				 SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_SETCURSEL,UnitsOpt,0);
                 GSSiClose2 (&FidSave);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EDIT_DEST),TRUE); 
               //  EnableWindow (GetDlgItem(hWndDlg,IDC_EDIT_HELPER),TRUE); 
                 FileIsOpen = TRUE;
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
            }
           		 break;*/
                
             case IDC_RECALL: 
            {    
            	 int	index; 
            	 LPSTR	pTab;
				 int	version;
            	 
              	 if (!*AutoExportName)
             	 { 
					 if (!GetFileName3 (hWndDlg,Name,IDS_FILTERDGC,IDS_FILEDGC))    
					 	break;
	             }
	             else
	             	_fstrcpy (Name,AutoExportName);  
	             FidDSC = GSSiOpenFile (Name,0,OF_READ);  
				 fgetstring (str,MAX_PATH,FidDSC);
				 version = atoi (str);
				 fgetstring (str,MAX_PATH,FidDSC);
				 SetDlgItemText (hWndDlg,IDC_OUT_FILE,str);
				 fgetstring (str,MAX_PATH,FidDSC);
				 SetDlgItemText (hWndDlg,IDC_SEED_FILE,str);
				 fgetstring (str,MAX_PATH,FidDSC);
				 SetDlgItemText (hWndDlg,IDC_PROJECTION,str);
				 fgetstring (str,MAX_PATH,FidDSC);
				 SetDlgItemText (hWndDlg,IDC_UNITS,str);
	             while (fgetstring (str,256,FidDSC))
	             {  
	             	pTab = _fstrchr (str,'\t');
	             	pTab++;
	             	pTab = _fstrchr (pTab,'\t'); 
	             	*pTab = 0;
			 		index = SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_FINDSTRING,(WPARAM)-1,(LPARAM) str); 
		 			*pTab = '\t';
			 		if (index != LB_ERR)
			 		{
			 			SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_DELETESTRING,(WPARAM)index,(LPARAM)0); 
			 			SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_INSERTSTRING,(WPARAM)index,(LPARAM) str); 
			 		}
			 		else
						SendDlgItemMessage (hWndDlg,IDC_LAYER_LIST,LB_ADDSTRING,0,(LPARAM)((LPSTR)str));  
				 }
	             GSSiClose2 (&FidDSC);
              	 if (*AutoExportName)
					PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
	        }
            	 break;
           
				case IDC_DGNPARMLEVEL:
                switch(HIWORD(wParam))
                {
                case CBN_SELCHANGE:
                         Choice=(int)SendDlgItemMessage(hWndDlg,IDC_DGNPARMLEVEL,CB_GETCURSEL,0,0);
						 SetDGNOutParms (hWndDlg,1,Choice+1);
						 SymConvTableChanged = TRUE;
						 break;
				}
				break;
				case IDC_DGNPARMSTYLE:
                switch(HIWORD(wParam))
                {
                     case CBN_SELCHANGE:
                         Choice=(int)SendDlgItemMessage(hWndDlg,IDC_DGNPARMSTYLE,CB_GETCURSEL,0,0);
						 SetDGNOutParms (hWndDlg,2,Choice);
						 SymConvTableChanged = TRUE;
						 break;
				}
				break;
				case IDC_DGNPARMWEIGHT:
                switch(HIWORD(wParam))
                {
                     case CBN_SELCHANGE:
                         Choice=(int)SendDlgItemMessage(hWndDlg,IDC_DGNPARMWEIGHT,CB_GETCURSEL,0,0);
						 SetDGNOutParms (hWndDlg,3,Choice);
						 SymConvTableChanged = TRUE;
						 break;
				}
				break;
				case IDC_DGNPARMFONT:
                switch(HIWORD(wParam))
                {
                     case CBN_SELCHANGE:
                         Choice=(int)SendDlgItemMessage(hWndDlg,IDC_DGNPARMFONT,CB_GETCURSEL,0,0);
						 SetDGNOutParms (hWndDlg,4,Choice+1);
						 SymConvTableChanged = TRUE;
						 break;
				}
				break;
				case IDC_DGNPARMCOLOR:
					if (!HavePalette)
					{
						GetDlgItemText (hWndDlg,IDC_SEED_FILE,seedFile,MAX_PATH);
						HavePalette = GetPaletteFromDGN (seedFile,ColorTable);
					}
					if (HavePalette)
					{
						 if ((Choice = GetColorFromPalette (hWndDlg,(LPRGBTRIPLE) ColorTable,256)) > -1)
						 {
							SetDGNOutParms (hWndDlg,5,Choice);
							SymConvTableChanged = TRUE;
						 }
					}
					else
						MessageBox (hWndDlg,"Cannot get color palette from seed file",0,MB_ICONEXCLAMATION);
			   	    break;
             case IDC_SELECT_OUTPATH:
            {   
                LPSTR   lpDot;   
                char    Name[128];
                if (GetSaveName2 (hWndDlg,Name,Filter,Ext,OutVarID))
                {
                    SetDlgItemText (hWndDlg,IDC_OUT_FILE,Name);
                }
            }
                break;
             
           case IDC_LAYER_LIST:
            {
                switch(HIWORD(wParam))
                    {
                     case LBN_SELCHANGE:
						 break;
					}
			}

			

            break;

            case IDOK: 
            {
                HFILE   FidMIF, FidMID, FidSHP, FidSHPIdx;
                char    IdxName[128];   
                short     pos=BT_FIRST, nTranFile, nParts;  
                LPSTR	lpSC;
                long    CurItem=0, iref, RecNum=0, Index=0, NumDBFRecs=0;
                LPTHEME pTheme;                                        
                short     nareas;
                DPOINT  CP; 
                BOOL    First, Rtn=FALSE;
                LPSTR	lpDot, lpName; 
                char    SQL[256],Type[32],DBName[MAX_PATH],TableName[34],FldName[34], Layer[34]; 
                HANDLE	hOutRec = GSSiGlobAlloc (1411,GMEM_MOVEABLE,USHRT_MAX);
                LPSTR	OutRec = GlobalLock (hOutRec);
//                SHPPOLYHEADER   SHPPolyHeader;  
//                SHPHEADER   SHPHeader;  
//                SHPRECHEADER SHPRecHeader; 
                LPOPENFILEDATA  FilePtr, FilePtrATT;
                LPOPENSQLDATA   SQLPtr, SQLPtrATT;
                DPOINT	Point,BP,POC,EP;
                HANDLE  hSQL;
                short IDB; 
                double	coordcvt=1, MinX=200000.0,MaxX=700000.0,MinY=10000.0,MaxY=350000.0,MinZ=-100.0,MaxZ=1500.0;  
                long	SHPOff, NumMiss,numpoints, MaxHandle, LocHandle;
                LPFIELDINFO	lpFldInfo;  
                short	ShapeType;
                HANDLE	hIndex=0;
                LPLONG	pIndex;  
                LPWORD	pPolyParts; 
                BOOL	SaveDisplaySymbol, WantRec,FirstInRec,FirstTPLField;
                long	nSavePoly2;
                HANDLE	hSavePoly2;
                static	long	debugref=108601347;       
                LPSHORT	pCurvePoints;
                short	Neg1=-1, Pass=0;
                HANDLE	hSurf=0, hLinks=0;  
                HPDPOINT	lpFirstPoint;  
                double	VoidElev;
                LPSTR	pDOT;
                LPMNMXCORD	lpRect;
                HPDPOINT	lpDpoint;
                double	Scale=1, TextAngle, Angle=0;  
                BOOL	WantTables =  FALSE;//SendDlgItemMessage (hWndDlg,IDC_INCLUDETABLES,BM_GETCHECK,0,0); 
                BOOL	dotext=FALSE;
            	DPOINT	RP,PC,PT;
            	double	BackAZ, ForAZ, CLEN, Radius, StartAngle, SweepAngle, LengthInDegrees;  
            	short	st;  
            	char	TmpName[MAX_PATH]=""; 
            	DPOINT	FirstPoint; 
            	HANDLE	hSentPointSym=GSSiGlobAlloc (1412,GHND,4096*4);
            	LPBYTE	SentPointSym=GlobalLock (hSentPointSym);
				short	NumPointSymbols = 0;
				HANDLE	hPointSymbols = GSSiGlobAlloc (1413,GMEM_MOVEABLE,4096*4);
				LPSHORT	PointSymbols=(LPSHORT)GlobalLock (hPointSymbols);  
				BYTE	ColorTable[256][3];
				int		line=0;
				LPSTR	loc[8],cloc[3];
				char	SearchStr[80];
				int		r,g,b, nColors=1, iColor;
				LPSTR	pLoc;
				LPBYTE	pByte;
				int		n;
				static	int	wantn=0;
				int		type;

				wantn++;
                
                GetDlgItemText (hWndDlg,IDC_OUT_FILE,Name,sizeof(Name));
                ExpandText (Name);
				if (!strlen (Name))
                {
                    MessageBox(GetFocus(),"Invalid output file name",Name,MB_ICONEXCLAMATION|MB_OK);
                    goto Exit;
                }
                _fmemset (ColorTable,0,sizeof(ColorTable));
				ColorTable[1][0]=1;
				ColorTable[1][1]=2;
				ColorTable[1][2]=3;
				pByte = &ColorTable[1][0];
				pByte++;
				pByte++;
/*				Choice = 0;
				while (SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETTEXT,Choice++,(DWORD)str) != LB_ERR)
				{
					pLoc = strrchr (str,'\t');
					pLoc+=2;
					*LastChr (pLoc) = 0;
				    GetParmLoc (3,',',pLoc,cloc);
					r = atoi(cloc[0]);
					g = atoi(cloc[1]);
					b = atoi(cloc[2]);
				    Color = RGB(r,g,b);
					AddToColorList (Color,(LPBYTE)ColorTable,&nColors);
				}*/

                DXFHandle=1;
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
                    if (!_fstrcmp(str,"Feet"))
                        PRJ_UNITS[3] = 1;
                    else if (!_fstrcmp(str,"Meters"))
                        PRJ_UNITS[3] = 2;
                }
                else
                {
                    MessageBox(GetFocus(),"Units field not set", 0,MB_ICONQUESTION|MB_OK);
                    goto Exit;
                }
				CloseTRANS2 (&hTranExport[0]);
				CloseTRANS2 (&hTranExport[1]);
/*                if (GetDlgItemText (hWndDlg,IDC_TRANFILE,IdxName,sizeof(IdxName)))
                {   
                	lpSC = _fstrchr (IdxName,';');
                	if (lpSC)
                	{
                		nTranFile = 2;
                		*lpSC++=0;
                	}
                	else
                		nTranFile = 1;
                	lpName = IdxName;
                	for (i=0;i<nTranFile;i++)
                	{   
	                	hTranExport[i] = LoadTranFileWithDandT (lpName);
	                	if (!hTranExport[i])
	                	{
	                    	MessageBox(GetFocus(),"Invalid transformation file", lpName,MB_ICONEXCLAMATION|MB_OK);
	                    	goto Exit;
	                    }  
	                    lpName = lpSC;
	                }
                }*/

                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_LOAD),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_SAVE),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE); 
                DisableHalt = TRUE;  
				SetContinueProcessing(TRUE);
				Processing = TRUE;
				NumItems = BT_NUM_IN_INDEX(hHighlight); 
				MaxHandle = 0; 
      			CurItem = 0;
				GSSiGetTempFileName(0, "gmt", 0, TmpName);
				if (!makedirectories (Name,FALSE,FALSE))
					FidMIF = HFILE_ERROR;
				else  
				{    
					FidMIF = GSSiOpenFile (TmpName,0,OF_CREATE); 
                }
                if (FidMIF == HFILE_ERROR) 
                {
                    MessageBox(GetFocus(),"Invalid output file name",Name,MB_ICONEXCLAMATION|MB_OK);
                    goto Exit;
                }
                NumMiss=0;   
                FullCurves = TRUE; 
                SaveDisplaySymbol = DisplaySymbol;
                DisplaySymbol = FALSE; 
                HaveTextPointers = FALSE; 
				
				if (WantTables)
				{   
					
	                SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,"Creating symbol table");
	                while (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData)&&ContinueProcessing)
	                {   
	                	short	idesc;
	                	
                    	pos = BT_NEXT;
                    	if (HighlightData.PD.Type != 3)
                    	{
                    		for (i=0;i<NumPointSymbols;i++)
                    			if (PointSymbols[i] == HighlightData.PD.Desc)
                    				goto NextSym1;
                    		
                    		PointSymbols[NumPointSymbols++] = HighlightData.PD.Desc;
                    	}
                NextSym1:; 
                    	PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0);
                    } 
                    pos = BT_FIRST;
	                while (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData)&&ContinueProcessing)
	                {   
	                	short	idesc;
	                	
                    	pos = BT_NEXT;
                    	if (HighlightData.PD.Type == 3)
                    	{
                    		for (i=0;i<NumPointSymbols;i++)
                    			if (PointSymbols[i] == HighlightData.PD.Desc)
                    				goto NextSym2;
                    		
                    		PointSymbols[NumPointSymbols++] = HighlightData.PD.Desc;
                    	}
                NextSym2:; 
                    	PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0);
                    }
/*					DXFOutTables (FidMIF,&DXFHandle,NumPointSymbols,PointSymbols);
					DXFOutBlocks (FidMIF,&DXFHandle,NumPointSymbols,PointSymbols);
					GSSiGlobUlFree (&hPointSymbols); */
				}
				fputstring (Name,FidMIF); line++;
				GetDlgItemText (hWndDlg,IDC_SEED_FILE,str,MAX_PATH);
                ExpandText (str);
				fputstring (str,FidMIF); line++;
				sprintf (str,"%i,%ld,%s:%s,%s",99,0L,"COLOR","TABLE","");
				fputstring (str,FidMIF); line++;  
				for (i=0;i<256;i++)
				{
					sprintf (str,"%i,%i,%i",(int)ColorTable[i][0],(int)ColorTable[i][1],(int)ColorTable[i][2]);
					fputstring (str,FidMIF); line++;
				}
      			CurItem = 0;
				//strcpy(DBName, "ODBC|PROPINFO;DBQ=[%DL]ATTRIBUT\\PROPINFO.MDB|PropInfo");
				strcpy(DBName, "[%DL]ATTRIBUT\\[PROP_DATE]\\PROPINFO.GMD");
				sprintf(str, "$OPEN(DB=%s,pid='@[PIDNO]')", DBName);
				ExpandText (str);
                SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,"Creating extract file");
				pos = BT_FIRST;
				//*HLTGraphicsFile = 0;
				n=0;
                while (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData)&&ContinueProcessing)
                {   
                	long	ii;
                	ltoa (iref,str,10);
                	//SetWindowText (hWndDlg,str);
					/*if (n++ != wantn)
						continue;*/
                	if (iref == debugref)
                		ii=1;      
                	for (Level=0;Level<NumPointSymbols;Level++)
                		if (PointSymbols[Level] == HighlightData.PD.Desc)
                			break; 
                	Level+=12;
                    pos = BT_NEXT; 
                    PickList[0]=HighlightData.PD; 
					if (PickList[0].HasText && PickList[0].Type == 1)
						PickList[0].Type = 4;
					HLTGraphicsPos = HighlightData.HLTGraphicsFilePos;
					if (*HLTGraphicsFile)
						PickList[0].Segment = 0;
                    if (PickList[0].Type == 1 && !SentPointSym[PickList[0].Desc])
                    {   
                    	HANDLE	hSymbol;  
                    	LPSYMBOL	pSym;
                    	POINT	SymPoint={0,0}; 
                    	long	HeaderLoc, ReturnLoc;
                    	
                    	SentPointSym[PickList[0].Desc] = 1;
						hSymbol = GetDictSymDesc (PickList[0].Desc,0); 
						pSym = (LPSYMBOL)GlobalLock (hSymbol);
						sprintf (str,"0 %4.4i %s",pSym->NumElements,pSym->Name); 
			if (!_fstricmp (pSym->Name,"USB"))
				ii=1;
			if (pSym->NumElements == 4)
				ii=1;   
						HeaderLoc = GSSillseek (FidMIF,0,1)+2;
						fputstring (str,FidMIF);  line++; 
						PointExportType = 1;
						DisplayPointSymbol (hSymbol,(HANDLE)FidMIF, 1000, 1000,0, &SymPoint,0,FALSE,0,0,FALSE,FALSE,0,0); 
						PointExportType = 0;  
						ReturnLoc = GSSillseek (FidMIF,0,1);
						GSSillseek (FidMIF,HeaderLoc,0);
						sprintf (str,"%4.4i",nExportSymElements);
						BigWrite (FidMIF,(HPSTR)str,4,-1);
						GSSillseek (FidMIF,ReturnLoc,0);      
						GlobalUnlock (hSymbol);
						DestroySymbol (hSymbol); 
                    }
                    SetIntRefno (PickList[0].Refno);
                    SetUDIValue (HighlightData.PD.Prefix,HighlightData.PD.UDI);     
                    SetGlobalValue ("%PREFIX",HighlightData.PD.Prefix);  
				    SetConfig (PickList[0].ConfigID);
				    SetViewport (PickList[0].ViewID);
					SetGlobalValue ("%DGNLEV","");
					SetGlobalValue ("%MSLINK","");
					if (PickList[0].Type == 2 || PickList[0].Type == 3)
					{
	                	if (PickList[0].Type == 3)
	                    	pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME);
	                    else
	                    	pTheme = AddTheme (GF_SAVEPOLY_THEME);
	                    CurView->PassID = 4; 
	                    //WantElement = PickList[0].Element;
						ProcessSelectedTheme = CurView->NumThemes;
	                    ProcessPickedItem (0,FALSE); 
						ProcessSelectedTheme = 0;
	                    WantElement = LONG_MAX;               
	                    DeleteTheme (pTheme);
	                }
	                else
	                    ProcessPickedItem (0,FALSE); 
					GetSymbolName (HighlightData.PD.Desc, SymName,0,1,0); 
			    	DXFConvertSymName (SymName);
					Strip (SymName,'%');
					sprintf (SearchStr,"%s\t%s",SymName,TypeName[PickTypeFromSysType(PickList[0].Type)-1]);
					if ((Choice = SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_FINDSTRING,-1,(LPARAM)SearchStr)) == LB_ERR) 
						goto NextHlt;

					SendDlgItemMessage(hWndDlg,IDC_LAYER_LIST,LB_GETTEXT,Choice,(DWORD)str);
				    GetParmLoc (7,'\t',str,loc);
				    Level = atoi (loc[2]);
				    Style = atoi (loc[3]);
				    Weight = atoi (loc[4]);
				    Font = atoi (loc[5]);
					if (!Level)
						goto NextHlt;
/*				    strcpy (ColorC,loc[6]);
					*LastChr (ColorC) = 0;
				    GetParmLoc (3,',',ColorC+1,cloc);
					r = atoi(cloc[0]);
					g = atoi(cloc[1]);
					b = atoi(cloc[2]);
				    Color = RGB(r,g,b);
					iColor = AddToColorList (Color,(LPBYTE)ColorTable,&nColors);*/
					iColor = atoi (loc[6]);

					if (PickList[0].HasText)
						type = 4;
					else
						type = PickList[0].Type;
					sprintf (str,"%i,%ld,%s:%s,%s",type,PickList[0].Refno,PickList[0].Prefix,PickList[0].UDI,SymName);
					fputstring (str,FidMIF); line++; 
					//GetGlobalCVal ("[%DGNLEV]",str,""); 
					sprintf (str,"%i,%i,%i,%i,%i",Level,iColor,Weight,Style,Font);
					fputstring (str,FidMIF); line++;
					if (!stricmp (PickList[0].Prefix,"PINCA"))
					{
						char str2[128];

						sprintf (str2,"[PIDNO]=%s;[C]=$FETCH(DB);[DB.MSLINK]",PickList[0].UDI);
						ExpandText (str2);
						sprintf (str,"ODBC:1:%s",str2);
					}
					else
						*str = 0;
					fputstring (str,FidMIF); line++;
                    switch (PickList[0].Type)
                    {   //Point","Line","Area","Text","Curve"
                    	
                        case 1: //points     
                        	BP = HighlightData.PD.BeginPoint;
                        	ConvertCoordExport(&BP); 
                            sprintf (str,"%f %f %f",BP.x,BP.y,HighlightData.PD.BPAZ);
                       		fputstring (str,FidMIF); line++;
                        break;
                        
                        case 2: 
                   DoLine: 
/*    	while (GetSavedPolys ())
   		if (hSavePoly)
		{   LPMNMXCORD lpRect;
			LPDPOINT	lpDpoint;  
			long		nPnts;
    		    
            NumPoints = nPnts = nSavePoly; 
            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
            lpRect++;
            lpDpoint = (LPDPOINT) lpRect;   
            if (!Reverse)
            {
	            while (nPnts--)
	            {
					**pPoints = *lpDpoint++;
					(*pPoints)++; 
	            }
	        }
	        else 
	        {   
	        	lpDpoint+=(nPnts-1);
	            while (nPnts--)
	            {
					**pPoints = *lpDpoint--;
					(*pPoints)++; 
	            }
	        }
		  	GSSiGlobUlFree (&hSavePoly);
         }*/ 



	                    	nParts = GetSavedPolys ();
	                    	ltoa (nParts,str,10);
                       		fputstring (str,FidMIF);  line++;
                       		while (nParts)
                       		{   
                       			long	np;
                       			
		                        if (hSavePoly)  
		                        {   
								    LPMNMXCORD	lpRect;
									LPDPOINT	lpDpoint;
									long		j=0;  

						            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
						            lpRect++;
						            lpDpoint = (LPDPOINT) lpRect;   
	                       			
	                       			while (nSavePoly>0)
	                       			{   
	                       				np = min (INT_MAX,nSavePoly);
	                       				nSavePoly -= np;
				                    	ltoa (np,str,10);
			                       		fputstring (str,FidMIF); line++;
								    		    
				                        	
										for (i=0;i<np;i++)
										{   
											BP = lpDpoint[j++];
			                            	ConvertCoordExport(&BP);
				                            sprintf (str,"%f %f",BP.x,BP.y);
				                       		fputstring (str,FidMIF); line++; 
				                       	}
			                        } 
			                        GlobalUnlock (hSavePoly);
			                    }
	                            nParts = GetSavedPolys (); 
	                         }
                        break;  
                        
                        case 3: //area
	                    	nParts = GetSavedPolys ();
	                    	ltoa (nSavePoly,str,10);
                       		fputstring (str,FidMIF); line++;
	                        if (hSavePoly)  
	                        {   
							    LPMNMXCORD	lpRect;
								LPDPOINT	lpDpoint;  
	                        	short		Width = 0;
								long		nPnts;
					    		    
					            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
					            lpRect++;
					            lpDpoint = (LPDPOINT) lpRect;   
	                        	
								for (i=0;i<nSavePoly;i++)
								{   
									BP = lpDpoint[i];
	                            	ConvertCoordExport(&BP);
		                            sprintf (str,"%f %f",BP.x,BP.y);
		                       		fputstring (str,FidMIF);  line++;
		                       	}
	                        	GlobalUnlock (hSavePoly);
								sprintf (str,"%i,%ld,%s:%s,%s",6,PickList[0].Refno,PickList[0].Prefix,PickList[0].UDI,SymName);
								fputstring (str,FidMIF);  line++;
                            }
                        break;  
                        
                        case 5: //curve  
                        {

 // @param dfOriginX the origin (center of rotation) of the arc (X).
 // @param dfOriginY the origin (center of rotation) of the arc (Y).
 // @param dfPrimaryAxis the length of the primary axis.
 // @param dfSecondaryAxis the length of the secondary axis.
 // @param dfRotation Counterclockwise rotation in degrees.
 // @param dfStartAngle start angle, degrees counterclockwise of primary axis.
 // @param dfSweepAngle sweep angle, degrees
                        	
                        	PC = HighlightData.PD.BeginPoint;
                        	ConvertCoordExport(&PC); 
                        	POC = HighlightData.PD.NodePoint;
                        	ConvertCoordExport(&POC); 
                        	PT = HighlightData.PD.EndPoint;
                        	ConvertCoordExport(&PT); 
							st = RCURVE(&PC.x,&PC.y,&POC.x,&POC.y,&PT.x,&PT.y,&RP.x,&RP.y,&CLEN);
							Radius = ldistp (RP,PC); 
                            if (CLEN > 0)
                            {
                            	BackAZ = AZToDXFAngle (getazd (&RP,&PC));
                            	ForAZ  = AZToDXFAngle (getazd (&RP,&PT));
                            }
                            else
                            {
                            	BackAZ = AZToDXFAngle (getazd (&RP,&PT));
                            	ForAZ  = AZToDXFAngle (getazd (&RP,&PC));
                            }  
                            LengthInDegrees = 360 * (CLEN / (2 * PY * Radius));
		                    sprintf (str,"%f %f %f %f %f %f",RP.x,RP.y,Radius,LengthInDegrees,BackAZ,ForAZ);
		                    fputstring (str,FidMIF);  line++;
	                    }
                        break; 
                        
//                        case 3: //area
/*	                    	nParts = GetSavedPolys ();
	                    	ltoa (nParts,str,10);
                       		fputstring (str,FidMIF);
                            if (hCurvePoints)
                            {
                            	lpDpoint = lpFirstPoint = (HPDPOINT)GlobalLock (hUnSplinedPoly);
                            	nPnts = numpoints = nUnSplinedPoints;
                            	pCurvePoints = (LPSHORT)GlobalLock (hCurvePoints);
                            }
                            else 
                            {
	                            nPnts = nSavePoly; 
	                            pCurvePoints = &Neg1;
                            	numpoints = nPnts - (nareas - 1); 
	                            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
	                            lpRect++;
	                            lpDpoint = lpFirstPoint = (LPDPOINT) lpRect; 
	                        } 
	                        FirstPoint = *lpDpoint;
	                    	ltoa (nPnts,str,10);
                       		fputstring (str,FidMIF);
                            if (hSavePolyParts)
                            {   
                            	long	Totp;
                            	
                            	pPolyParts = (LPWORD)GlobalLock (hSavePolyParts); 
                            	nareas = *pPolyParts++; 
                            	hIndex = GSSiGlobAlloc (GHND,nareas*4);
                            	pIndex = (LPLONG)GlobalLock (hIndex);
                            	i = nareas; 
                            	Totp = 0;
                            	while (i--)
                            	{   
                            		*pIndex++ = Totp;
                            		Totp += *pPolyParts++;
                            	}
                            	GlobalUnlock (hIndex);  
                            	GlobalUnlock (hSavePolyParts); 
                            }
                            else 
                            {
                            	hIndex = 0; 
	                            nareas=1;  
                            }
	                            
                        	numpoints = nPnts - (nareas - 1); 
                            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
                            lpRect++; 
                            lpDpoint = (HPDPOINT)lpRect;
                            First = TRUE;  
	                            
                            if (HaveLinkLines (lpDpoint,nPnts,&hLinks))
                            	ii=1; 
                            GSSiGlobFree (&hLinks);
                    	    GlobalUnlock (hSavePoly);
                            for (i=0;i<nPnts;i++)
                            {   
						    	if (i == *pCurvePoints)
						    	{   
						    		POC = lpDpoint[i]; 
						    		if (i + 1 >= nPnts)
						    			EP = FirstPoint;
						    		else
						    			EP = lpDpoint[i];
						    	}
                                ConvertCoordExport(lpOutPoint); 
                                if (First) 
                                {
                                	FirstOutPoint = *lpOutPoint;
                                    FirstPoint = *lpDpoint;     
                                }
                                First = FALSE;
                            } 
                            *lpOutPoint = FirstOutPoint;
                            *lpOrigPoint = FirstPoint;
                            GlobalUnlock (hOutPoint);    
                            GlobalUnlock (hOrigPoint); 
                        //    numpoints++;
                            if (hIndex)
                            {
                            	pIndex = (LPLONG)GlobalLock (hIndex);
//                                	_lwrite (FidSHP,pIndex,4*SHPPolyHeader.NumParts); 
                            	GlobalUnlock (hIndex);  
                            	lpDpoint = (LPDPOINT) lpRect;
                            	pPolyParts = (LPWORD)GlobalLock (hSavePolyParts); 
                                pPolyParts++;
                                i = nareas; 
                                First=TRUE;
                                while (i--)
                                {
//										BigWrite (FidSHP,lpDpoint,sizeof(DPOINT)*(long)(*pPolyParts)); 
									lpDpoint+=*pPolyParts++;
									if (First)
										First=FALSE;
									else                                        
										lpDpoint++;
                                }
                            	GlobalUnlock (hSavePolyParts); 
                            }
                            else	
                            {   
                            	FirstInRec=TRUE;
	                            lpOutPoint = (HPDPOINT)GlobalLock (hOutPoint);
	                      //      lpOutPoint += numpoints - 1;
	                            lpOrigPoint = (HPDPOINT)GlobalLock (hOrigPoint);
	                      //      lpOrigPoint += numpoints - 1;
	                      		for (i=0;i<numpoints;i++) 
	                      		{
	                                sprintf (str,"%f %f",lpDpoint[i].x,lpDpoint[i].y);
		                       		fputstring (str,FidMIF); 
                                }
                            }*/ 
	                            
                        break;
                    }
                    if (PickList[0].HasText) 
                    {   
                    	LPSHORT	pnText;
                    	LPEXPORTTEXT	pEXText;
                    	double	D1,D2,SizeFactor;
                        	
                    	CurView->PassID = 4;  
                    	hExportText = GSSiGlobAlloc (1414,GHND,4096*4);
                    	hExportTextPointer = GSSiGlobAlloc (1415,GHND,4096*4);
	                    ProcessPickedItem (0,FALSE);            
						pnText = (LPSHORT)GlobalLock (hExportText); 
						pEXText = (LPEXPORTTEXT)(pnText+1); 
						itoa (*pnText,str,10);
                       	fputstring (str,FidMIF);  line++;
                        while ((*pnText)--)
                        {
							if (pEXText->Type == 2)
							{   
								DPOINT	RP;
								double	AZ, RAD, CLEN, RadiansPerPixel,cwidth,th,tdc,til,tas,acw,angle;
								LPSTR	pChr = pEXText->Text;
								
								RCURVE (&pEXText->Loc[0].x,&pEXText->Loc[0].y,&pEXText->Loc[1].x,&pEXText->Loc[1].y,
										&pEXText->Loc[2].x,&pEXText->Loc[2].y,&RP.x,&RP.y,&CLEN);
								RAD = ldistp (RP,pEXText->Loc[0]);     
								AZ = getazd (&RP,&pEXText->Loc[0]); 
								RadiansPerPixel = 1e0/RAD;
								while (*pChr)
								{   
									DPOINT	pt = dnewpt (RP,AZ,RAD);  
														
									GetTextWidthAndHeight (pEXText->Font,pEXText->size,pChr,1,&cwidth,&th,&tdc,&til,&tas,&acw);
									AZ = LTWOPI (AZ - (DSIGN(cwidth/2,CLEN))*RadiansPerPixel);
									angle = LTWOPI(AZ-DSIGN(HALFPI,CLEN))/RADDEG;
									pChr++;
								} 
						    }
						    else
						    {
								
	                        	//fputstring ("4",FidMIF);   line++;
	                        	BP = pEXText->Loc[0]; 
	                        	EP = pEXText->Loc[2]; 
	                        	D1 = ldistp (BP,EP);
	                        	TextAngle = getazd (&pEXText->Loc[0],&pEXText->Loc[2])*DEGRAD;
	                        	ConvertCoordExport(&BP); 
	                        	ConvertCoordExport(&EP); 
	                        	D2 = ldistp (BP,EP);
	                        	if (D1)
	                        		SizeFactor = D2/D1;
	                        	else
	                        		SizeFactor = 1;
	                        	sprintf (str,"%f %f %i %f %f",BP.x,BP.y,Font,pEXText->size*SizeFactor*0.6,TextAngle); 
	                        	fputstring (str,FidMIF);  line++;
	                        	fputstring (pEXText->Text,FidMIF);  line++;
	                        } 
		                	pEXText++;
		                }
	                    GlobalUnlock (hExportText);  
	                }
            NextHlt: 
                    GSSiGlobFree (&hExportText);  
                    GSSiGlobFree (&hExportTextPointer);  
            		DestroySavedPolys();                    
                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0);
                } 

//				sprintf (str,"%i,%ld,%s:%s,%s",3,(long)1,"","","TESTAREA");
//               	fputstring (str,FidMIF); 
				
                GSSiClose2 (&FidMIF);
                FullCurves = FALSE;
                DisplaySymbol = SaveDisplaySymbol;
                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0);
		        SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,"Extract finished"); 
		        Rtn=TRUE;
Exit:           
				ProcessText ("$CLOSE(DB)");
				GSSiGlobUlFree (&hPointSymbols);
				GSSiGlobUlFree (&hSentPointSym);
				CloseTRANS2 (&hTranExport[0]);
				CloseTRANS2 (&hTranExport[1]);
                GSSiGlobUlFree (&hOutRec);
                DisableHalt = FALSE;  
                SetContinueProcessing ( TRUE);   
                Processing = FALSE;
				DGNWriteGeoMaster (TmpName);
                if (*AutoExportName)
                {
	                 GSSiEndDialog(hWndDlg,Rtn,hSaveBM); 
	            }
                break;
            }
            case IDC_EXIT:     
				 if (SymConvTableChanged)
				 {
				 	short opt=MessageBox (hWndDlg,"You have made changes to the symbol conversion table.\r\nDo you wish to save them?",0,MB_YESNOCANCEL);
				 	switch (opt)
				 	{   
				 		case IDCANCEL:
				 			return TRUE;
				 		case IDYES:
							PostMessage(hWndDlg, WM_COMMAND, IDC_DXFSYMCONVSAVE, 0L);
				 			return TRUE;
				 	}
				 }
                 GSSiEndDialog(hWndDlg,FALSE,hSaveBM); 
                 
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)
					 SetContinueProcessing(FALSE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE); 
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

BOOL FAR PASCAL ACCUMPOINTOPTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1249);
#endif
{	

 int	SymControls[3]={IDC_ASCIRCLE,IDC_ASSQUARE,IDC_ASNOCHANGE};
 int	SizeControls[5]={IDC_APSIZE1,IDC_APSIZE2,IDC_APSIZE3,IDC_APSIZE4,IDC_APSIZE5};  
 int	BaseSizeControls[3]={IDC_BSORIGNAL,IDC_BSPIXELS,IDC_BSWORLD};
 int	BRtn; 
 char	str[128]; 
 short	i;  
 static	HANDLE	hSaveTheme; 
 LPTHEME	pSaveTheme;
 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1249);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:
         cwCenter(hWndDlg, 0); 
         hSaveTheme = GSSiGlobAlloc ( 637,GMEM_MOVEABLE,sizeof(THEME));
         pSaveTheme = (LPTHEME)GlobalLock (hSaveTheme);
         *pSaveTheme = *CurTheme;
         GlobalUnlock (hSaveTheme);
		 SendDlgItemMessage (hWndDlg,SymControls[CurTheme->AccumPointSymbolOpt],BM_SETCHECK,TRUE,0L);
		 SendDlgItemMessage (hWndDlg,SizeControls[CurTheme->AccumPointSizeOpt],BM_SETCHECK,TRUE,0L);
		 if (CurTheme->AccumPointBaseSize < 0)
		 {
		 	i=2;  
		 	sprintf (str,"%f",-CurTheme->AccumPointBaseSize);
    	 	SetDlgItemText (hWndDlg,IDC_BSWORLDSIZE,str);
    	 } 
		 else if (CurTheme->AccumPointBaseSize > 0) 
		 {
		 	i=1; 
		 	ltoa (IDNINT(CurTheme->AccumPointBaseSize),str,10);
    	 	SetDlgItemText (hWndDlg,IDC_BSPIXSIZE,str);
    	 } 
    	 else
    	 	i=0;
		 SendDlgItemMessage (hWndDlg,BaseSizeControls[i],BM_SETCHECK,TRUE,0L);
		 SendDlgItemMessage (hWndDlg,IDC_APLINK,BM_SETCHECK,CurTheme->AccumPointLink,0L);
		 SendDlgItemMessage (hWndDlg,IDC_ACCUMTEXT,BM_SETCHECK,CurTheme->AccumPointText,0L);
 		 SendDlgItemMessage (hWndDlg,IDC_DISPERSNONE,BM_SETCHECK,CurTheme->ACCDisperse==2,0L);
 		 SendDlgItemMessage (hWndDlg,IDC_DISPERSPARTIAL,BM_SETCHECK,CurTheme->ACCDisperse==1,0L);
 		 SendDlgItemMessage (hWndDlg,IDC_DISPERSEFULL,BM_SETCHECK,CurTheme->ACCDisperse==0,0L);
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
		         pSaveTheme =(LPTHEME)GlobalLock (hSaveTheme);
		         *CurTheme = *pSaveTheme;
		         GlobalUnlock (hSaveTheme); 
		         GSSiGlobUlFree (&hSaveTheme);
                 EndDialog(hWndDlg, FALSE);
                 break; 
            
            case IDC_BSORIGNAL:
            case IDC_BSPIXELS:
            case IDC_BSWORLD:
            	SetDlgItemText (hWndDlg,IDC_BSPIXSIZE,"");
    	 		SetDlgItemText (hWndDlg,IDC_BSWORLDSIZE,"");
    	 		break;
    	 		
            case IDOK:
            {   
            	for (i=0;i<3;i++)
            		if (SendDlgItemMessage (hWndDlg,SymControls[i],BM_GETCHECK,0,0L))
            			CurTheme->AccumPointSymbolOpt = i;
            	for (i=0;i<5;i++)
            		if (SendDlgItemMessage (hWndDlg,SizeControls[i],BM_GETCHECK,0,0L))
            			CurTheme->AccumPointSizeOpt = i;
           		if (SendDlgItemMessage (hWndDlg,IDC_BSPIXELS,BM_GETCHECK,0,0L))
           		{
           			GetDlgItemText (hWndDlg,IDC_BSPIXSIZE,str,sizeof(str));
           			CurTheme->AccumPointBaseSize = atol (str);
           		}
           		else if (SendDlgItemMessage (hWndDlg,IDC_BSWORLD,BM_GETCHECK,0,0L))
           		{
           			GetDlgItemText (hWndDlg,IDC_BSWORLDSIZE,str,sizeof(str));
           			CurTheme->AccumPointBaseSize = -atof (str);
           		} 
           		else
           			CurTheme->AccumPointBaseSize = 0;
           		
		 		CurTheme->AccumPointLink = SendDlgItemMessage (hWndDlg,IDC_APLINK,BM_GETCHECK,0,0L);
		 		CurTheme->AccumPointText = SendDlgItemMessage (hWndDlg,IDC_ACCUMTEXT,BM_GETCHECK,0,0L);
		 		if (SendDlgItemMessage (hWndDlg,IDC_DISPERSNONE,BM_GETCHECK,0,0L))
		 			CurTheme->ACCDisperse = 2;
		 		else if (SendDlgItemMessage (hWndDlg,IDC_DISPERSPARTIAL,BM_GETCHECK,0,0L))
		 			CurTheme->ACCDisperse = 1;
		 		else
		 			CurTheme->ACCDisperse = 0;
                GSSiGlobUlFree (&hSaveTheme);
               	EndDialog(hWndDlg, TRUE);
            }
                break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1249);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1249);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL FAR PASCAL SELECTDISTMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (937);
#endif
{   
 static short   InLocChoice, InOffsetChoice; 
 char   str[128]; 
 static short   VPDefault, OLDefault;
 LPSTR  lpTAB;
 
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (937);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:  
         cwCenter(hWndDlg, 0);
         /* initialize working variables                                */     
         InOffsetChoice = DistanceChoice;
         OLDefault = FillList (hWndDlg,IDC_OFF_LINE,"offline.txt",0,0);  

         SendDlgItemMessage (hWndDlg,IDC_OLUNITS,CB_RESETCONTENT,0,0);
         SendDlgItemMessage (hWndDlg,IDC_OLUNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Feet"); 
         SendDlgItemMessage (hWndDlg,IDC_OLUNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Meters"); 
         SendDlgItemMessage (hWndDlg,IDC_OLUNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Miles"); 
         SendDlgItemMessage (hWndDlg,IDC_OLUNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Kilometers"); 
         SendDlgItemMessage (hWndDlg,IDC_OLUNITS,CB_SETCURSEL,(WPARAM)0,(LPARAM)0); 

         if (DistanceChoice < 0)
            DistanceChoice = OLDefault;
         SendDlgItemMessage (hWndDlg,IDC_OFF_LINE,LB_SETCURSEL,DistanceChoice,0); 
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDC_OFF_LINE: /* List box                           */
                 switch(HIWORD(wParam))
                 {   
                     case LBN_SELCHANGE:
                         SetDlgItemText (hWndDlg,IDC_OLVAL,"");
                         break;
                         
                     case LBN_DBLCLK: 
//                         IgnoreLbutton = TRUE;
                         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
                     break;
                 }
                 break; 
                
            case IDC_OLVAL:
                switch(HIWORD(wParam))
                {   
                 case EN_CHANGE:
                     GetDlgItemText (hWndDlg,IDC_OLVAL,str,sizeof(str));
                     if (str[0]) 
                        SendDlgItemMessage (hWndDlg,IDC_OFF_LINE,LB_SETCURSEL,-1,0);                     
                     break;
                }
                break;
                
            case IDC_EDITOL:  
                 _fstrcpy (str,"NOTEPAD.EXE ");
                 _fstrcat (str,"offline.txt");
                 WinExec (str,SW_SHOWMAXIMIZED);
            
                 break;
                 
            case IDOK: 
            {
                double  UFac[4]={FTM,1.0,5280*FTM,1000.0}; 
                short       iunits;
             
                GetDlgItemText(hWndDlg,IDC_OLVAL,str,sizeof(str));
                if (str[0])
                { 
                    OffsetLineOffset = atof(str);
                    iunits=(short)SendDlgItemMessage(hWndDlg,IDC_OLUNITS,CB_GETCURSEL,0,0); 
                    OffsetLineOffset *= UFac[iunits];
                }
                else
                {               
                    DistanceChoice=(short)SendDlgItemMessage(hWndDlg,IDC_OFF_LINE,LB_GETCURSEL,0,0); 
                    SendDlgItemMessage(hWndDlg,IDC_OFF_LINE,LB_GETTEXT,DistanceChoice,(DWORD)&str); 
                    lpTAB = _fstrrchr(str,'\t');
                    if (lpTAB)
                    {   short   l;
                        
                        lpTAB++;
                        OffsetLineOffset = atof(lpTAB);
                        l = _fstrlen(str);
                        if (str[l-1]=='f' || str[l-1]== 'F') OffsetLineOffset *= FTM;
                    } 
                }
                EndDialog(hWndDlg, TRUE);
            }
                break;
            case IDCANCEL:  
                DistanceChoice = InOffsetChoice;
                EndDialog(hWndDlg, FALSE);
                break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (937);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (937);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL FAR PASCAL EXPORTIMAGEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{   
	static	HANDLE	hSaveBM;
	char	str[256], Ext[16]; 

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
        
		//hSaveBM = EnterBlockingWindow (hWndDlg); 
		GetCurVal (str,256,IDS_FILEEXPORTIMAGE);
		SetDlgItemText (hWndDlg,IDC_IMAGEFILE,str);  
		if (App == 1)
	        EnableWindow (GetDlgItem(hWndDlg,IDC_FORMATGEOTIF),FALSE);
		switch (BitmapFormatOpt)
		{
			case 0:
				SendDlgItemMessage (hWndDlg,IDC_FORMATBMP,BM_SETCHECK,TRUE,0L);
			break;
			case 1:
				SendDlgItemMessage (hWndDlg,IDC_FORMATTIF,BM_SETCHECK,TRUE,0L);
			break;
			case 2:  
				SendDlgItemMessage (hWndDlg,IDC_FORMATGEOTIF,BM_SETCHECK,TRUE,0L);
			break;
			case 3:
				SendDlgItemMessage (hWndDlg,IDC_FORMATJPEG,BM_SETCHECK,TRUE,0L);
			break;
		}		
		
		switch (BitmapSizeOpt)
		{
			case 1:
				SendDlgItemMessage (hWndDlg,IDC_SIZESMALL,BM_SETCHECK,TRUE,0L);
			break;
			case 2:  
				SendDlgItemMessage (hWndDlg,IDC_SIZEMEDIUM,BM_SETCHECK,TRUE,0L);
			break;
			case 3:
				SendDlgItemMessage (hWndDlg,IDC_SIZELARGE,BM_SETCHECK,TRUE,0L);
			break;
		}		
		
		 break;                              
    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
         	case IDC_FINDFILE:
         	{
         		UINT	Filt;
         		
				Filt = GetImageFilterAndExtension (Ext); 
         		if (GetSaveName2 (hWndDlg,str,Filt,Ext,IDS_FILEEXPORTIMAGE))
         			SetDlgItemText (hWndDlg,IDC_IMAGEFILE,str);
         	}
			break;
			
			case IDC_FORMATBMP:
			case IDC_FORMATTIF:
			case IDC_FORMATGEOTIF:
			case IDC_FORMATJPEG:
			case IDC_SIZESMALL:
			case IDC_SIZEMEDIUM:
			case IDC_SIZELARGE:
			
			SetBitmapFormatAndSize (hWndDlg);
			if (GetDlgItemText (hWndDlg,IDC_IMAGEFILE,str,256))
			{
				LPSTR pDot=_fstrrchr (str,'.');
				
				if (!pDot)
					pDot = _fstrchr (str,0);
				GetImageFilterAndExtension (Ext);
				_fstrcpy (pDot,Ext);
				SetDlgItemText (hWndDlg,IDC_IMAGEFILE,str);
			}
			break;

            case IDCANCEL: 
                //GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
                EndDialog(hWndDlg, FALSE);
            break; 
            
            case IDOK: 
            {   
            	SetBitmapFormatAndSize (hWndDlg); 
                if (GetDlgItemText (hWndDlg,IDC_IMAGEFILE,str,256))
                {
					LPSTR pDot=_fstrrchr (str,'.');
					
					if (!pDot)
						pDot = _fstrchr (str,0);
					GetImageFilterAndExtension (Ext);
					_fstrcpy (pDot,Ext);
	                SetCurVal (str,IDS_FILEEXPORTIMAGE);
	                //GSSiEndDialog(hWndDlg,TRUE,hSaveBM);  
	                EndDialog(hWndDlg,TRUE);  
	            }
            }
            break;    
            
         }
         break; 

    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL FAR PASCAL INFOBOX_POSMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
	char	str[128];	

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
         
         sprintf (str,"%f",TAGBox.center.x);
    	 SetDlgItemText (hWndDlg,IDC_CENTERX,str);
         sprintf (str,"%f",TAGBox.center.y);
    	 SetDlgItemText (hWndDlg,IDC_CENTERY,str);
         sprintf (str,"%f",TAGBox.bmWidthD);
    	 SetDlgItemText (hWndDlg,IDC_WIDTH,str);
         sprintf (str,"%f",TAGBox.bmHeightD);
    	 SetDlgItemText (hWndDlg,IDC_HEIGHT,str);
 		 SendDlgItemMessage (hWndDlg,IDC_FIXEDSIZE,BM_SETCHECK,TAGBox.Flags.FixedSize,0L);   
		 			
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break;
            case IDOK: 
            	GetDlgItemText(hWndDlg,IDC_CENTERX,str,sizeof(str));
            	TAGBox.center.x = atof(str);
            	GetDlgItemText(hWndDlg,IDC_CENTERY,str,sizeof(str));
            	TAGBox.center.y = atof(str);
            	GetDlgItemText(hWndDlg,IDC_WIDTH,str,sizeof(str));
            	TAGBox.bmWidthD = atof(str);
            	GetDlgItemText(hWndDlg,IDC_HEIGHT,str,sizeof(str));
            	TAGBox.bmHeightD = atof(str);
                TAGBox.Flags.FixedSize = SendDlgItemMessage (hWndDlg,IDC_FIXEDSIZE,BM_GETCHECK,0,0);
                EndDialog(hWndDlg, TRUE);

            break;
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL FAR PASCAL COMBO_ADD_FILEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (613);
#endif
{ 
    static  HANDLE hDB=0;
    static  LPGWDHEADER lpGWDHead;
    static  short DataFileType=0;
    BOOL    False=FALSE;
    UINT    IDC_FieldName=SV_FIELD_NAME, BRtn;
    
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (613);
#endif
 	return (BRtn);
}
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam, IDC_SQL,
                     SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES, SV_TABLE_HEADING,&IDC_FieldName,1,
                     pAddFile, &DataFileType, &hSQLCombo, &False,FALSE))
{
#if ENABLETRACE
GSSiExitProg (613);
#endif
                     return TRUE;
}
    
 switch(Message)
   {
    case WM_INITDIALOG:  
         /* initialize working variables                                */ 
         hWndAddComboFile = hWndDlg;
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
        switch(LOWORD(wParam))

           {
			  case IDC_SETSQL:      
			  {    
			     HANDLE hMem;
			     LPSTR  lpStr, lpWhere;
				                 
			     hMem = GSSiGlobAlloc ( 248,GHND,4096);
			     lpStr = GlobalLock (hMem); 
			     GetDlgItemText (hWndDlg,IDC_SQL,lpStr,1024);
			     if (GetSQLWhereClause (hWndDlg, hSQLCombo, lpStr))
			     	SetDlgItemText (hWndDlg,IDC_SQL,lpStr);    
			     GSSiGlobUlFree (&hMem);
			     break;
			  }
				            
              case IDC_OPEN_DB:
                  EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),TRUE);
                  EnableWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),TRUE);
                  break;  
            
              case IDOK:
              {
                
                short nItems; 
                char    str[128];
                  
                nItems=(short)SendDlgItemMessage(hWndDlg,SV_FIELD_NAME,LB_GETSELCOUNT,0,0); 
                if (!nItems)
                {
                     GSSiMsgBox(GetFocus(),"No fields selected",
                                    "Error",MB_OK|MB_ICONEXCLAMATION,0);  
                     break;
                }
                GetDlgItemText (hWndDlg,IDC_SQL,str,sizeof(str));
                if (!str[0])
                {    
                    if ((nItems = (short)SendDlgItemMessage(hWndComboFile ,IDC_FILES,LB_GETCOUNT,0,0)))
                    { 

                         GSSiMsgBox(GetFocus(),"No Where Clause",
                                        "Error",MB_OK|MB_ICONEXCLAMATION,0);  
                         break;
                    }
                }
                PostMessage(hWndComboFile, WM_COMMAND, IDC_HAVE_FILE, 0L); 
               } 
                break;

              case IDCANCEL:
                CloseDataFile (TRUE, &hSQLCombo);
                DestroyWindow(hWndDlg);
                break; 
                  
           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (613);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (613);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

void ConvertOracleTimeDate (LPSTR txt)
{
	int	year,mo,day,hour,min,sec;
	LPSTR loc=strchr (txt,'-');
	LPSTR	str=txt;
	char	MonthAbv[12][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

	if (!loc)
		return;
	*loc = 0;
	year = atol (txt);
	*loc++ = '-';
	txt = loc;
	loc = strchr (txt,'-');
	if (!loc)
		return;
	*loc = 0;
	mo = atol (txt);
	*loc++ = '-';
	txt = loc;
	loc = strchr (txt,' ');
	if (!loc)
		return;
	*loc = 0;
	day = atol (txt);
	*loc++ = ' ';
	txt = loc;
	loc = strchr (txt,':');
	if (!loc)
		return;
	*loc = 0;
	hour = atol (txt);
	*loc++ = ':';
	txt = loc;
	loc = strchr (txt,':');
	if (!loc)
		return;
	*loc = 0;
	min = atol (txt);
	*loc++ = ':';
	txt = loc;
	sec = atol (txt);
	sprintf (str,"%i-%s-%i",day,MonthAbv[mo-1],year);
	return;
}

BOOL FAR PASCAL ORACLEMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
{ 
    int nItems;
	short i, Version=1, NumFields,UnitsOpt;  
    char    File[MAX_PATH],  ExtID[32], Name[MAX_PATH], str[1024];
    LPINT   lpItems;    
    HFILE   Fid;
    OFSTRUCTGM    OFStruct;  
    BOOL    False=FALSE, ComputeElev;  
    UINT    IDC_FieldName=IDC_FIELDS;
    LPSTR   vbar; 
    char    txt[4096], txt2[1024], project[34]; 
    //BTHEAD  BTHead;
    long    NumItems;
    static	char	Ext[6], SaveExt[8],DExt[6]; 
    static	short   Filter, FileVarID, OutVarID;  
    static	BOOL	FileIsOpen;
	static	HANDLE	hSaveBM;
    
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam,IDC_SQL,
                     SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, &IDC_FieldName,1,
                     MIFOutDataFile, &MIFOutDataFileType, &MIFOuthDB, &False,FALSE))  return TRUE;
 switch(Message)
   {
    case WM_INITDIALOG:
         /* initialize working variables                                */   
    
	    FileIsOpen = FALSE;            
	   	SetWindowText (hWndDlg,"Export to Oracle Object Format");
	    Filter = IDS_FILTERORACLE;
	    _fstrcpy (Ext,".CTL");
	    _fstrcpy (DExt,".SQL");
	    _fstrcpy (SaveExt,".ORO"); 
	    FileVarID = IDS_FILEORO;
	    OutVarID = IDS_FILEORACLE;
		ShowWindow (GetDlgItem(hWndDlg,IDC_SFTTITLE),SW_SHOW);
		ShowWindow (GetDlgItem(hWndDlg,IDC_SHAPETYPE),SW_SHOW);
		SetDlgItemTextGlobal (hWndDlg,IDC_ORACLETABLENAME,"[%ORACLETABLENAME]",0);
		SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
		SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
	    SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Degrees");
	    _fstrcpy (str,"*.CVT");
	    DlgDirListComboBox (hWndDlg,str,IDC_PROJECTION,0,DDL_READWRITE);   
         if (!hHighlight)
         { 
    NoItems:
            GSSiMsgBox( GetFocus(),"No items highlighted","Error", MB_OK,0);
            PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
            break;
         }  
         //GetBTHeader (hHighlight,&BTHead); 
         NumItems = BT_NUM_IN_INDEX (hHighlight);  
         if (!NumItems) goto NoItems;
         sprintf(txt,"%ld items selected",NumItems);
         SetDlgItemText(hWndDlg,IDC_TOT_ITEMS,txt);  
         SendDlgItemMessage (hWndDlg,IDC_SHAPETYPE,CB_ADDSTRING,0,(LPARAM)"Point");
         SendDlgItemMessage (hWndDlg,IDC_SHAPETYPE,CB_ADDSTRING,0,(LPARAM)"Arc");
         SendDlgItemMessage (hWndDlg,IDC_SHAPETYPE,CB_ADDSTRING,0,(LPARAM)"Polygon");
         SendDlgItemMessage (hWndDlg,IDC_SHAPETYPE,CB_ADDSTRING,0,(LPARAM)"Text");
    	 hSaveBM = EnterBlockingWindow (hWndDlg);
         if (*AutoExportName) 
         {
			SetDlgItemTextGlobal (hWndDlg,IDC_ORACLETABLENAME,"[%ORACLETABLENAME]",0);
			SetDlgItemTextGlobal (hWndDlg,IDC_MIF_FILE,"[%ORACLEOUTPUTPATH]",0);
		 	PostMessage(hWndDlg, WM_COMMAND, IDC_LOAD, 0L); 
		 	goto SetOutDB;
		 }
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

         switch(LOWORD(wParam))

           { 
            case IDC_OPEN_DB:
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),TRUE); 
                 break;
                      
            case IDC_SETSQL:      
            {    
                 HANDLE hMem;
                 LPSTR  lpStr;
                 
                 hMem = GSSiGlobAlloc ( 683,GHND,4096);
                 lpStr = GlobalLock (hMem); 
                 GetDlgItemText (hWndDlg,IDC_SQL,lpStr,1024);
                 if (GetSQLWhereClause (hWndDlg, MIFOuthDB, lpStr))
                 	SetDlgItemText (hWndDlg,IDC_SQL,lpStr);    
                 GSSiGlobUlFree (&hMem);
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
                if (HIWORD(wParam) ==  EN_KILLFOCUS)
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
                    
                    GSSiGlobFree (&MIFOutFields);
                    CloseDataFile (FALSE,&MIFOuthDB);
                    Fid = GSSiOpenFile (File,&OFStruct,OF_READ);  
                    if (Fid == HFILE_ERROR)
	                {
	                    GSSiMsgBox( GetFocus(), "Invalid Oracle Script File",File, MB_ICONEXCLAMATION,0);
	                    break;
	                }  
                    BigRead (Fid,(HPSTR)&Version,2); 
                    BigRead (Fid,Name,sizeof(Name)); 
	             	if (!*AutoExportName) 
	             	{
	                    SetDlgItemText (hWndDlg,IDC_MIF_FILE,Name);
	                    if ((lpDot=_fstrrchr (Name,'.')))
	                    {
	                        *lpDot = 0;
	                        _fstrcat (Name,DExt);
	                        SetDlgItemText (hWndDlg,IDC_MID_FILE,Name);
	                    }
                    }
                    BigRead (Fid,MIFOutDataFile,MAX_PATH);
                    BigRead (Fid,MIFOutSQL,lnMIFOutSQL);
                    BigRead (Fid,(HPSTR)&nItems,sizeof(int));
                     MIFOutFields = GSSiGlobAlloc ( 684,GHND,nItems*4+4);
                     lpItems = (LPINT)GlobalLock(MIFOutFields);
                     *lpItems = nItems;
                     lpItems++;       
                     BigRead (Fid,(HPSTR)lpItems,nItems*sizeof(int));
                     GlobalUnlock(MIFOutFields);
                     BigRead (Fid,str,16);
                     SetDlgItemText (hWndDlg,IDC_SHAPETYPE,str);
	                 BigRead (Fid,project,sizeof(project));
	                 SetDlgItemText (hWndDlg,IDC_PROJECTION,project);
	                 BigRead (Fid,(HPSTR)&UnitsOpt,2);
				     SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_SETCURSEL,UnitsOpt,0);
				     BigRead (Fid,str,64); 
				     str[64] = 0;
	             	 if (!*AutoExportName) 
	                	SetDlgItemText (hWndDlg,IDC_ORACLETABLENAME,str);
				     BigRead (Fid,str,34);
				     str[34] = 0;
	                 SetDlgItemText (hWndDlg,IDC_VOID_ELEV,str);
	                 BigRead (Fid,(HPSTR)&ComputeElev,4); 
           			 SendDlgItemMessage (hWndDlg,IDC_COMPUTE_Z,BM_SETCHECK,ComputeElev,0L);
                     GSSiClose2 (&Fid); 
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
                 GetDlgItemText (hWndDlg,SV_DATABASE_LIST ,MIFOutDataFile,128);
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
                    GSSiMsgBox( GetFocus(), "Error","No Fields Selected", MB_OK,0);
                    break;
                 }  
                 GSSiGlobFree (&MIFOutFields);
                 MIFOutFields = GSSiGlobAlloc ( 685,GHND,nItems*4+4);
                 lpItems = (LPINT)GlobalLock(MIFOutFields);
                 *lpItems = nItems;
                 lpItems++;       
                 SendDlgItemMessage(hWndDlg,IDC_FIELDS,
                                           LB_GETSELITEMS,
                                           nItems,
                                           (LPARAM)lpItems); 
                 GlobalUnlock(MIFOutFields);
                 CloseDataFile (FALSE,&MIFOuthDB);      
                 GetDlgItemText(hWndDlg,IDC_SQL,MIFOutSQL,lnMIFOutSQL);
                 if (wParam == IDOK)
	                 GSSiEndDialog(hWndDlg,TRUE,hSaveBM); 
                     
                 else
                 {
                    Fid = GSSiOpenFile (File,&OFStruct,OF_CREATE);
                    BigWrite (Fid,(HPSTR)&Version,2,-1);
	                 GetDlgItemText (hWndDlg,IDC_MIF_FILE,Name,sizeof(Name));
	                 BigWrite (Fid,Name,sizeof(Name),-1); 
                    BigWrite (Fid,MIFOutDataFile,MAX_PATH,-1);
                    BigWrite (Fid,MIFOutSQL,lnMIFOutSQL,-1);
                     lpItems = (LPINT)GlobalLock(MIFOutFields);  
                     BigWrite (Fid,(char *)lpItems,(*lpItems+1)*sizeof(int),-1); 
                     GlobalUnlock(MIFOutFields);
                     GetDlgItemText (hWndDlg,IDC_SHAPETYPE,str,16);
                     BigWrite (Fid,str,16,-1);
	                 GetDlgItemText (hWndDlg,IDC_PROJECTION,project,sizeof(project));
	                 BigWrite (Fid,project,sizeof(project),-1);
				     UnitsOpt=SendDlgItemMessage(hWndDlg,IDC_UNITS,CB_GETCURSEL,0,0); 
	                 BigWrite (Fid,(HPSTR)&UnitsOpt,2,-1); 
	                 GetDlgItemText (hWndDlg,IDC_ORACLETABLENAME,str,64);
	                 BigWrite (Fid,str,64,-1);
	                 GetDlgItemText (hWndDlg,IDC_VOID_ELEV,str,34);
	                 BigWrite (Fid,str,34,-1);
            	 	 ComputeElev = SendDlgItemMessage (hWndDlg,IDC_COMPUTE_Z,BM_GETCHECK,0,0); 
            	 	 BigWrite (Fid,(HPSTR)&ComputeElev,4,-1);
                    GSSiClose2 (&Fid);
                 }
                 break; 
            
            case IDC_ADDGLOBAL: 
            	*str = 0;
				if (GetTextString (hWndDlg,str,64,"Enter global variable name",0,0,FALSE,TRUE,TRUE))
					SendDlgItemMessage(hWndDlg,IDC_FIELDS,LB_ADDSTRING,0,(LPARAM)str);            		
            	break;
            	
            case IDOK: 
            {
                HFILE   FidMIF, FidMID, FidSHP, FidSHPIdx, FidSKP;
                OFSTRUCTGM    OFStruct;  
                short     pos, nTranFile, nParts;  
                LPSTR	lpSC;
                long    CurItem=0, iref, Index=0, NumOutPoints=0,NumOutLines=0,NumOutPolyLines=0,NumOutCurves=0,NumOutText=0,NumOutTPL=0,NumOutAreas=0;
                LPTHEME pTheme;                                        
                HIGHLIGHTDATA   HighlightData;  
                short     nareas;
                DPOINT  CP; 
                BOOL    First, Rtn=FALSE;
                LPSTR	lpDot, lpName; 
                char    SQL[256],Type[32],DBName[MAX_PATH],SymName[34],TableName[64],FldName[34]; 
                HANDLE	hOutRec = GSSiGlobAlloc ( 686,GMEM_MOVEABLE,USHRT_MAX);
                LPSTR	OutRec = GlobalLock (hOutRec);
//                SHPPOLYHEADER   SHPPolyHeader;  
//                SHPHEADER   SHPHeader;  
//                SHPRECHEADER SHPRecHeader; 
                LPOPENFILEDATA  FilePtr, FilePtrATT;
                LPOPENSQLDATA   SQLPtr, SQLPtrATT;
                DPOINT	BP,POC,EP;
                HANDLE  hSQL;
                short IDB; 
                double	coordcvt=1, MinX=200000.0,MaxX=700000.0,MinY=10000.0,MaxY=350000.0,MinZ=-100.0,MaxZ=1500.0;  
                long	SHPOff, NumMiss,numpoints;
                LPFIELDINFO	lpFldInfo;  
                short	ShapeType;
                HANDLE	hIndex=0;
                LPLONG	pIndex;  
                LPINT	pPolyParts; 
                BOOL	SaveDisplaySymbol, WantRec,FirstInRec,FirstTPLField;
                long	nSavePoly2;
                HANDLE	hSavePoly2;
                static	long	debugref=1743054;       
                LPSHORT	pCurvePoints;
                short	Neg1=-1, Pass=0, nindex;
                HANDLE	hSurf=0;  
                HPDPOINT	lpFirstPoint;  
                double	VoidElev;
                LPSTR	pDOT;
                char	DTMFile[128];   
                char	TPLFldNames[]={"INT_REFNO PREFIX UDI SYMBOL_NAME REFNO PARENT SYMBOL_NUM"};   
                double	SnapTol = GetGlobalDVal2 ("[%SNAPPOLYTOL]",0);
                BOOL	SaveURORTI = UseRefOrTAGIndex;
               	LPSHORT	pnText;
               	LPEXPORTTEXT	pEXText; 
               	LPMNMXCORD lpRect; 
               	short	FldType, FldLength;
                
                GetGlobalCVal ("[%DTMFILE]",DTMFile,"[%DL]attribut\\ot1.dtm");
                if (!GetDlgItemText (hWndDlg,IDC_ORACLETABLENAME,TableName,sizeof(TableName)-1))
                {
                    GSSiMsgBox( GetFocus(),"Table name missing", "Error", MB_OK,0);
                    goto Exit;
                }
                CloseDataFile (FALSE,&MIFOuthDB);  
                GetDlgItemText (hWndDlg,IDC_SHAPETYPE,str,sizeof(str));
                if (!_fstricmp (str,"Point"))    
                	ShapeType = 1;
                else if (!_fstricmp (str,"Arc"))    
                	ShapeType = 3;
                else if (!_fstricmp (str,"Polygon"))    
                	ShapeType = 5;
                else if (!_fstricmp (str,"Text"))    
                	ShapeType = 4;
                GetDlgItemText (hWndDlg,IDC_SQL,SQL,sizeof(SQL));
                if (!OpenDataFile (MIFOutDataFile,SQL,BT_READ,&MIFOuthDB))
                    goto Exit2;   
                nItems=SendDlgItemMessage(hWndDlg,IDC_FIELDS,
                                           LB_GETSELCOUNT,
                                           0,
                                           0);
                if (!nItems)
                {
                    GSSiMsgBox( GetFocus(),"No Fields Selected", "Error", MB_OK,0);
                    goto Exit;
                }  
                GSSiGlobFree (&MIFOutFields);
                if (!GetDlgItemText (hWndDlg,IDC_PROJECTION,project,sizeof(project)))
                {
                    GSSiMsgBox(GetFocus(),"No output projection set", 0,MB_ICONEXCLAMATION|MB_OK,0);
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
                    if (!_fstrcmp(str,"Feet"))
                        PRJ_UNITS[3] = 1;
                    else if (!_fstrcmp(str,"Meters"))
                        PRJ_UNITS[3] = 2;
                }
                else
                {
                    GSSiMsgBox(GetFocus(),"Units field not set", 0,MB_ICONQUESTION|MB_OK,0);
                    goto Exit;
                }
				CloseTRANS2 (&hTranExport[0]);
				CloseTRANS2 (&hTranExport[1]);
                MIFOutFields = GSSiGlobAlloc ( 687,GHND,nItems*4+4);
                lpItems = (LPINT)GlobalLock(MIFOutFields);
                SendDlgItemMessage(hWndDlg,IDC_FIELDS,
                                           LB_GETSELITEMS,
                                           nItems,
                                           (LPARAM)lpItems); 
                GlobalUnlock(MIFOutFields);
                
           	 	ComputeElev = SendDlgItemMessage (hWndDlg,IDC_COMPUTE_Z,BM_GETCHECK,0,0); 
                GetDlgItemText (hWndDlg,IDC_VOID_ELEV,str,34);
                VoidElev = PlaneElev = atof (str);
                if (ComputeElev)
                	hSurf = DTMOpen (DTMFile,VoidElev,BT_READ,0);
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT2),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_LOAD),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_SAVE),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE); 
                DisableHalt = TRUE;  
				SetContinueProcessing(TRUE);
				Processing = TRUE;
                //GetBTHeader (hHighlight,&BTHead); 
                NumItems = BT_NUM_IN_INDEX (hHighlight);                              
            	hExportText = GSSiGlobAlloc ( 688,GHND,4096);
            	hExportTextPointer = GSSiGlobAlloc ( 689,GHND,4096);
      TPLPass:  
      			CurItem = 0;
                GetDlgItemText (hWndDlg,IDC_MIF_FILE,Name,sizeof(Name));
                ExpandText (Name); 
                if (Pass)
                {
                	pDOT = _fstrrchr (Name,'.');
                	_fstrcpy (pDOT,"ll.CTL"); 
                	_fstrcat (TableName,"LL");
                	nItems++;
                }
				if (!makedirectories (Name,FALSE,FALSE))
					FidMIF = HFILE_ERROR;
				else  
				{
                	FidMIF = GSSiOpenFile (Name,0,OF_CREATE);
				}
                if (FidMIF == HFILE_ERROR) 
                {
                    GSSiMsgBox(GetFocus(),"Invalid output file name",Name,MB_ICONEXCLAMATION|MB_OK,0);
                    goto Exit;
                }
                FidSHP = FidMIF;
                GetDlgItemText (hWndDlg,IDC_MID_FILE,Name,sizeof(Name));
                ExpandText (Name);
               	pDOT = _fstrrchr (Name,'.');
               	_fstrcpy (pDOT,".SKP");
                FidSKP = GSSiOpenFile (Name,0,OF_CREATE);  
                GetDlgItemText (hWndDlg,IDC_MID_FILE,Name,sizeof(Name));
                ExpandText (Name);
                if (Pass)
                {
                	pDOT = _fstrrchr (Name,'.');
                	_fstrcpy (pDOT,"ll.SQL");
                }
                FidMID = GSSiOpenFile (Name,0,OF_CREATE);   
				sprintf (OutRec,"prompt Dropping %s spatial layer...",TableName);
				fputstring (OutRec,FidMID);
				sprintf (OutRec,"DROP TABLE %s;",TableName);
				fputstring (OutRec,FidMID);
				sprintf (OutRec,"prompt Creating %s spatial layer...",TableName);
				fputstring (OutRec,FidMID);

				fputstring ("INSERT INTO USER_SDO_GEOM_METADATA (TABLE_NAME, COLUMN_NAME, DIMINFO, SRID)",FidMID); 
  				sprintf (OutRec,"VALUES ('%s', 'GEOM',",TableName); 
				fputstring (OutRec,FidMID);
			    fputstring ("  MDSYS.SDO_DIM_ARRAY",FidMID); 
      			sprintf (OutRec,"    (MDSYS.SDO_DIM_ELEMENT('X',%f, %f, 0.000050),",MinX,MaxX); 
				fputstring (OutRec,FidMID);
       			sprintf (OutRec,"     MDSYS.SDO_DIM_ELEMENT('Y', %f,%f, 0.000050),",MinY,MaxY);
				fputstring (OutRec,FidMID);
      			sprintf (OutRec,"     MDSYS.SDO_DIM_ELEMENT('Z', %f,%f, 0.000050)",MinZ,MaxZ);
				fputstring (OutRec,FidMID);
				fputstring ("      ), ",FidMID);
				fputstring ("  NULL",FidMID);
				fputstring ("     ); ",FidMID);
				sprintf (OutRec,"CREATE TABLE %s(",TableName);
				fputstring (OutRec,FidMID);
  				_fstrcpy (OutRec,"  GEOM MDSYS.SDO_GEOMETRY,");
/*  PIN		VARCHAR(13));

INSERT INTO SDO_GEOM_METADATA (TABLE_NAME, COLUMN_NAME, DIMINFO) 
  VALUES ('PARCELS', 'GEOM', 
    MDSYS.SDO_DIM_ARRAY 
      (MDSYS.SDO_DIM_ELEMENT('X', 500000.0, 555000.0, 0.000050), 
       MDSYS.SDO_DIM_ELEMENT('Y', 130000.0, 200000.0, 0.000050)  
      ) 
     );*/ 
                
				fputstring ("LOAD DATA ",FidMIF);
 				fputstring ("INFILE * ",FidMIF);
				//fputstring ("TRUNCATE ",FidMIF);
				fputstring ("APPEND ",FidMIF);
				fputstring ("CONTINUEIF NEXT(1:1) = '#'",FidMIF); 
				sprintf (str,"INTO TABLE %s ",TableName);
				fputstring (str,FidMIF);
				fputstring ("FIELDS TERMINATED BY '|' OPTIONALLY ENCLOSED BY '^^'",FidMIF);
				fputstring ("TRAILING NULLCOLS (",FidMIF);
				fputstring ("  GEOM COLUMN OBJECT",FidMIF); 
				fputstring ("  (",FidMIF);
				fputstring ("      SDO_GTYPE       INTEGER EXTERNAL,",FidMIF); 
				fputstring ("      SDO_ELEM_INFO   VARRAY TERMINATED BY '|/' ",FidMIF);
				fputstring ("        (X            FLOAT EXTERNAL),",FidMIF); 
				fputstring ("      SDO_ORDINATES   VARRAY TERMINATED BY '|/' ",FidMIF);
				fputstring ("        (X            FLOAT EXTERNAL) ",FidMIF);
				_fstrcpy (str,"  ),");

		
                SQLPtrATT = (LPOPENSQLDATA)GlobalLock (MIFOuthDB);
                FilePtrATT = (LPOPENFILEDATA)GlobalLock (SQLPtrATT->OFHandle); 
                lpItems = (LPINT)GlobalLock (MIFOutFields);     
                FirstTPLField = TRUE;
                for (i=0;i<nItems;i++,lpItems++) 
                {   
                    SendDlgItemMessage(hWndDlg,IDC_FIELDS,
                                               LB_GETTEXT,
                                               *lpItems,(LPARAM)Name);
                	if (Pass && i && !*lpItems)
                	{   
                		if (FirstTPLField)
                		{
                			FirstTPLField = FALSE;
							fputstring (OutRec,FidMID); 
							fputstring (str,FidMIF); 
							_fstrcpy (FldName,"STYLE");
			            	sprintf (OutRec,"  %s\tNUMBER,",FldName);
				            sprintf (str,"  %s,",FldName);
                		}
                		break;
                	}
					fputstring (OutRec,FidMID); 
					fputstring (str,FidMIF);
                    lpFldInfo = &FilePtrATT->FldInfo;
                    lpFldInfo += *lpItems;  
                    REPLAC (FldName," ","_",32);
                    if (*lpItems < FilePtrATT->NumFields) 
                    {
	                    _fstrcpy (FldName,lpFldInfo->name);
                    	FldLength = lpFldInfo->length;
                    	FldType = lpFldInfo->type;
                    }
                    else 
                    {   
                    	_fstrcpy (FldName,Name);
                    	DecodeGMField (FldName,&FldType,&FldLength);
                    }
				    switch (FldType)
				    {   
				        default: 
				        case SQL_CHAR: 
				        case SQL_VARCHAR:
				        case BT_RIGHT_CHAR:
						case SQL_UNKCHAR:
				        case BT_CHAR:
				            sprintf (OutRec,"  %s\tVARCHAR2(%i),",FldName,FldLength);
				        break;
						        
				        case SQL_INTEGER:
				        case SQL_SMALLINT:
				        case SQL_TINYINT:
				        case SQL_BIT:
				        case BT_INTEGER:
				        case SQL_NUMERIC:
				        case SQL_FLOAT:
						case SQL_REAL:
						case SQL_DOUBLE:
				        case BT_REAL:
				        	if (!_fstricmp (FldName,"INT_REFNO"))
				            	sprintf (OutRec,"  %s\tNUMBER(38),",FldName);  
				            else
				            	sprintf (OutRec,"  %s\tNUMBER,",FldName);
				        break;
				     }
		            sprintf (str,"  %s,",FldName);
                }
                _fstrcpy (_fstrrchr (OutRec,','),");");
				fputstring (OutRec,FidMID);  
                _fstrcpy (_fstrrchr (str,','),")");
				fputstring (str,FidMIF);
                GSSiClose2 (&FidMID);  
				fputstring ("BEGINDATA",FidMIF);
				GlobalUnlock (MIFOutFields); 
        		GlobalUnlock (SQLPtrATT->OFHandle);
				GlobalUnlock (MIFOuthDB);
                SetDlgItemText(hWndDlg,IDC_MESSAGE,"Creating extract file");
                pos = BT_FIRST;
                NumMiss=0;   
                FullCurves = TRUE; 
                SaveDisplaySymbol = DisplaySymbol;
                DisplaySymbol = FALSE;   
                UseRefOrTAGIndex = FALSE;
                HaveTextPointers = FALSE; 
                while (!BT_FIND (hHighlight,(LPSTR)&iref,pos,BT_ANY,(LPSTR)&HighlightData)&&ContinueProcessing)
                {   
                	long	ii;
                	
                	if (iref == debugref)
                		ii=1;
                    pos = BT_NEXT; 
                    PickList[0]=HighlightData.PD;
/*                    if (EXType == SHP &&
                    	((PickList[0].Type == 2 && ShapeType != 3) ||
                    	(PickList[0].Type == 3 && ShapeType != 5) ||
                    	((PickList[0].Type == 1 || PickList[0].Type == 4)&& ShapeType != 1)))
                        goto NextHlt;*/ 
                    SetIntRefno (PickList[0].Refno);
                    SetUDIValue (HighlightData.PD.Prefix,HighlightData.PD.UDI);     
                    SetGlobalValue ("%PREFIX",HighlightData.PD.Prefix);  
				    SetConfig (PickList[0].ConfigID);
				    SetViewport (PickList[0].ViewID);
					if (ShapeType != 4)
					{
	                	if (ShapeType == 5)
	                    	pTheme = AddTheme (GF_SAVEPOLYPARTS_THEME);
	                    else
	                    	pTheme = AddTheme (GF_SAVEPOLY_THEME);
	                    CurView->PassID = 4; 
	                    //WantElement = PickList[0].Element;
						ProcessSelectedTheme = CurView->NumThemes;
	                    ProcessPickedItem (0,FALSE); 
						ProcessSelectedTheme = 0;
	                    WantElement = LONG_MAX;               
	                    DeleteTheme (pTheme);
	                }
					GetSymbolName (HighlightData.PD.Desc, SymName,0,1,0);
					WantRec = FALSE;
					pnText = (LPSHORT)GlobalLock (hExportText); 
					*pnText = 0;
					GlobalUnlock (hExportText); 
					pnText = (LPSHORT)GlobalLock (hExportTextPointer); 
					*pnText = 0;
					GlobalUnlock (hExportTextPointer); 
                    switch (PickList[0].Type)
                    {   //Point","Line","Area","Text","Curve"
                    	default:
	                		sprintf (OutRec,"Skipped unknown type %i: Ref %ld, TAG: %s:%s",PickList[0].Type,iref,HighlightData.PD.Prefix,HighlightData.PD.UDI);
	                		fputstring (OutRec,FidSKP);
	                		break;
                    	
                        case 1: //points
                        	if (ShapeType != 1)  
                        	{   
                        		sprintf (OutRec,"Skipped point: Ref %ld, TAG: %s:%s",iref,HighlightData.PD.Prefix,HighlightData.PD.UDI);
                        		fputstring (OutRec,FidSKP);
                        		break;
                        	}
                        	fputstring (" 3001|1|1|1|/",FidMIF); 
                        	BP = HighlightData.PD.BeginPoint;
                        	ConvertCoordExport(&BP); 
                        	sprintf (OutRec,"#%f|%f|%f|/",BP.x,BP.y,NGIELV (HighlightData.PD.BeginPoint,hSurf,0)); 
                        	fputstring (OutRec,FidMIF); 
                        	NumOutPoints++;    
                        	WantRec = TRUE;
                        break;
                            
                        case 2: //line (assumes UltiMap 2 point only)  
                        	if (ShapeType != 3)
                        	{   
                        		sprintf (OutRec,"Skipped line: Ref %ld, TAG: %s:%s",iref,HighlightData.PD.Prefix,HighlightData.PD.UDI);
                        		fputstring (OutRec,FidSKP);
                        		break;
                        	}
                        	fputstring (" 3002|1|2|1|/",FidMIF); 
                        	if (HighlightData.PD.NumPoints < 3)
                        	{
	                        	BP = HighlightData.PD.BeginPoint;
	                        	ConvertCoordExport(&BP); 
	                        	EP = HighlightData.PD.EndPoint;
	                        	ConvertCoordExport(&EP); 
	                        	sprintf (OutRec,"#%f|%f|%f|%f|%f|%f|/",BP.x,BP.y,NGIELV (HighlightData.PD.BeginPoint,hSurf,0),EP.x,EP.y,NGIELV (HighlightData.PD.EndPoint,hSurf,0)); 
	                        	fputstring (OutRec,FidMIF);  
	                        	NumOutLines++; 
                        	}
                        	else
                        	{
                        	   	nParts = GetSavedPolys ();
	                        	if (hSavePoly)
	                        	{
		                            HPDPOINT    lpDpoint;
		                            DPOINT  OutPoint,OrigPoint;
									UINT	nPnts;

		          					fputstring ("#+",FidMIF);  
                                	FirstInRec=TRUE;
		                            nPnts = nSavePoly; 
		                            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
		                            lpRect++;
		                            lpDpoint = lpFirstPoint = (LPDPOINT) lpRect; 
		                            RemoveDupPolyPoints (&nPnts,lpDpoint, P_TOL*2);
	                            
		                            while (nPnts--)
		                            {   
		                            	OutPoint = OrigPoint = *lpDpoint++;
		                                ConvertCoordExport(&OutPoint); 
                                		if (FirstInRec)
                                		{
                                			_fstrcpy (OutRec,"#");  
                                			FirstInRec = FALSE;
                                		}
										sprintf (_fstrchr(OutRec,0),"%f|%f|%f|",OutPoint.x,OutPoint.y,NGIELV(OrigPoint,hSurf,0));                                		
                                	}   
                                	_fstrcat (OutRec,"/");
		                        	fputstring (OutRec,FidMIF);   
									GlobalUnlock (hSavePoly);
	                        	}
	                        	NumOutPolyLines++; 
							}  
                        	WantRec = TRUE;
                        break;  
                        
                        case 4: //text
                        {   
                        	
                        	if (ShapeType != 4)  
                        	{   
                        		sprintf (OutRec,"Skipped text: Ref %ld, TAG: %s:%s",iref,HighlightData.PD.Prefix,HighlightData.PD.UDI);
                        		fputstring (OutRec,FidSKP);
                        		break;
                        	}
                        	CurView->PassID = 4;  
		                    ProcessPickedItem (0,FALSE);            
							pnText = (LPSHORT)GlobalLock (hExportText); 
							pEXText = (LPEXPORTTEXT)(pnText+1); 
							if (*pnText)
							{
								NumOutText++;
	                        	WantRec = TRUE;
	                        }
		                    GlobalUnlock (hExportText);  
		                }
                        break;
                        
                        case 5: //curve  
                        	if (ShapeType != 3)
                        	{   
                        		sprintf (OutRec,"Skipped curve: Ref %ld, TAG: %s:%s",iref,HighlightData.PD.Prefix,HighlightData.PD.UDI);
                        		fputstring (OutRec,FidSKP);
                        		break;
                        	}
                        	fputstring (" 3002|1|2|2|/",FidMIF); 
                        	BP = HighlightData.PD.BeginPoint;
                        	ConvertCoordExport(&BP); 
                        	POC = HighlightData.PD.NodePoint;
                        	ConvertCoordExport(&POC); 
                        	EP = HighlightData.PD.EndPoint;
                        	ConvertCoordExport(&EP); 
                        	sprintf (OutRec,"#%f|%f|%f|%f|%f|%f|%f|%f|%f|/",
                        			 BP.x,BP.y,NGIELV (HighlightData.PD.BeginPoint,hSurf,0),POC.x,POC.y,NGIELV (HighlightData.PD.NodePoint,hSurf,0),EP.x,EP.y,NGIELV (HighlightData.PD.EndPoint,hSurf,0)); 
                        	fputstring (OutRec,FidMIF);
                        	NumOutCurves++;   
                        	WantRec = TRUE;
                        break; 
                        
                        case 3: //area
                        	if (ShapeType != 5)
                        	{   
                        		sprintf (OutRec,"Skipped area: Ref %ld, TAG: %s:%s",iref,HighlightData.PD.Prefix,HighlightData.PD.UDI);
                        		fputstring (OutRec,FidSKP);
                        		break;
                        	}
                        	NumOutAreas++;
	                    	nParts = GetSavedPolys ();
	                        if (hSavePoly)
	                        {   
	                            long	Totp,nPnts,i,Inc;
	                            short	nLinks;
	                            HPDPOINT    lpDpoint, lpOutPoint,lpOrigPoint;
	                            DPOINT  FirstPoint, FirstOutPoint;  
	                            HANDLE	hOutPoint=0, hOrigPoint=0, hLinks=0;
	                            
	                        	WantRec = TRUE;
	                            
	                            nindex = 0;
	                            if (hSavePolyParts)
	                            {
	                            	pPolyParts = (LPINT)GlobalLock (hSavePolyParts); 
	                            	nareas = *pPolyParts++; 
	                            	hIndex = GSSiGlobAlloc ( 690,GHND,nareas*4);
	                            	pIndex = (LPLONG)GlobalLock (hIndex);
	                            	i = nareas; 
	                            	Totp = 0;
	                            	while (i--)
	                            	{   
	                            		if (*pPolyParts > 1)
	                            		{
	                            			*pIndex++ = Totp; 
	                            			Totp += *pPolyParts; 
	                            			nindex++;
	                            		} 
	                            		pPolyParts++;
	                            	}
	                            	GlobalUnlock (hIndex); 
	                            	GlobalUnlock (hSavePolyParts);
	                            }
	                            else 
	                            {
	                            	hIndex = 0; 
		                            nareas=1;  
	                            }
	                            
	                            if (hCurvePoints)
	                            {
	                            	lpDpoint = lpFirstPoint = (HPDPOINT)GlobalLock (hUnSplinedPoly);
	                            	nPnts = numpoints = nUnSplinedPoints;
	                            	pCurvePoints = (LPSHORT)GlobalLock (hCurvePoints);
	                            }
	                            else 
	                            {
		                            nPnts = nSavePoly; 
		                            pCurvePoints = &Neg1;
	                            	numpoints = nPnts - (nareas - 1); 
		                            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);
		                            lpRect++;
		                            lpDpoint = lpFirstPoint = (LPDPOINT) lpRect; 
		                        } 
	                            hOutPoint = GSSiGlobAlloc ( 691,GMEM_MOVEABLE,(nPnts+1)*(long)sizeof(DPOINT)); 
	                            lpOutPoint = (HPDPOINT)GlobalLock (hOutPoint);
	                            hOrigPoint = GSSiGlobAlloc ( 692,GMEM_MOVEABLE,(nPnts+1)*(long)sizeof(DPOINT)); 
	                            lpOrigPoint = (HPDPOINT)GlobalLock (hOrigPoint);
	                            First = TRUE;  
	                            RemoveDupPolyPoints (&nPnts,lpDpoint, P_TOL*2);
	                            while (nPnts--)
	                            {   
	                            	*lpOutPoint = *lpOrigPoint = *lpDpoint;
	                                ConvertCoordExport(lpOutPoint); 
	                                if (First) 
	                                {
	                                	FirstOutPoint = *lpOutPoint;
	                                    FirstPoint = *lpDpoint;     
	                                }
	                            	if (!nPnts)
	                            	{
	                            		if (ldistp (*lpOutPoint,FirstOutPoint) > P_TOL)
	                            			numpoints++;
	                            	}
	                                lpDpoint++; 
	                                lpOutPoint++; 
	                                lpOrigPoint++;
	                                First = FALSE;
	                            } 
	                            *lpOutPoint = FirstOutPoint;
	                            *lpOrigPoint = FirstPoint;
	                            GlobalUnlock (hOutPoint);    
	                            GlobalUnlock (hOrigPoint); 
                                if (hCurvePoints)
                                {   
                                	short nElem = numpoints - nCurvePoints-1, ip=0, OrdOff=1;  
                                	short	nRemCP=nCurvePoints;
                                	long	n=numpoints-2; 
                                	LPSHORT	pCP=pCurvePoints+nCurvePoints-1;
                                	
                                	sprintf (OutRec," 3003|1|1005|%i|",nElem); 
                                	i=1;
                                	while (i<numpoints)
                                	{    
                                		i++;
                                		if (nRemCP && n == *pCP)
                                		{ 
                                			sprintf (_fstrchr (OutRec,0),"%i|2|2|",OrdOff);
	                                		OrdOff += 3; 
	                                		i++;  
	                                		n--;
	                                		pCP--;
	                                		nRemCP--;
                                		}
                                		else 
                                		{
                                			sprintf (_fstrchr (OutRec,0),"%i|2|1|",OrdOff);
                                		} 
                                		OrdOff += 3;  
                                		n--;
                                	}
                                	_fstrcat (OutRec,"/");
		                        	fputstring (OutRec,FidMIF);   
                                	FirstInRec=TRUE;
   		                            lpOutPoint = (HPDPOINT)GlobalLock (hOutPoint);
   		                            lpOrigPoint = (HPDPOINT)GlobalLock (hOrigPoint);
   		                            if (HighlightData.PD.Area > 0) 
   		                            {
   		                            	lpOutPoint += (numpoints - 1);
   		                            	lpOrigPoint += (numpoints - 1); 
   		                            	Inc = -1;
   		                            } 
   		                            else
   		                            	Inc = 1; 
                                	for (i=0;i<numpoints;i++,lpOrigPoint+=Inc,lpOutPoint+=Inc)
                                	{
                                		if (FirstInRec)
                                		{
                                			_fstrcpy (OutRec,"#");  
                                			FirstInRec = FALSE;
                                		}
										sprintf (_fstrchr(OutRec,0),"%f|%f|%f|",lpOutPoint->x,lpOutPoint->y,NGIELV(*lpOrigPoint,hSurf,0));                                		
                                	}   
                                	_fstrcat (OutRec,"/");
		                        	fputstring (OutRec,FidMIF);   
		                        	GlobalUnlock (hOutPoint);  
		                        	GlobalUnlock (hOrigPoint);
                                } 
                                else
                                {   
                                	if (hIndex && nindex)
                                	{
		                            	pIndex = (LPLONG)GlobalLock (hIndex); 
		                        		_fstrcpy (OutRec," 3003|1|1003|1|");
		                        		pIndex++;
		   		                        lpOutPoint = (HPDPOINT)GlobalLock (hOutPoint);
		                            	for (i=1;i<nindex;i++,pIndex++)
		                            	{   
		                            		long	np=numpoints - *pIndex; 
		                            		
		                            		if (i < nindex-1)
		                            			np = *(pIndex+1) - *pIndex;
		                            		if (ComputeAreaAreaD (&lpOutPoint[*pIndex+max(0,i-1)],np,0) < 0)
		                        				sprintf (_fstrchr(OutRec,0),"%ld|2003|1|",*pIndex*3+1); 
		                            		else
		                        				sprintf (_fstrchr(OutRec,0),"%ld|1003|1|",*pIndex*3+1); 
		                            	}
		                            	GlobalUnlock (hOutPoint);
		                            	GlobalUnlock (hIndex);
		                            	_fstrcat (OutRec,"/");
		                            	fputstring (OutRec,FidMIF);
		                            	fputstring ("#+",FidMIF);  
		                            	pIndex = (LPLONG)GlobalLock (hIndex); 
		                            	pPolyParts = (LPINT)GlobalLock (hSavePolyParts); 
		                            	pPolyParts++; 
		                            	for (i=0;i<nareas;i++,pIndex++,pPolyParts++)
		                            	{   
		                            		int	j,numpoints=*pPolyParts; 
		                            				                            				                            		
		                            		FirstInRec=TRUE;
		   		                            lpOutPoint = (HPDPOINT)GlobalLock (hOutPoint);
		   		                            lpOutPoint += (*pIndex+max(0,i-1));
		   		                            lpOrigPoint = (HPDPOINT)GlobalLock (hOrigPoint);  
		   		                            lpOrigPoint += (*pIndex+max(0,i-1));
		   		                            if (HighlightData.PD.Area > 0) 
		   		                            {
		   		                            	lpOutPoint += (numpoints - 1);
		   		                            	lpOrigPoint += (numpoints - 1); 
		   		                            	Inc = -1;
		   		                            } 
		   		                            else
		   		                            	Inc = 1; 
			   		                        
		                                	for (j=0;j<numpoints;j++,lpOrigPoint+=Inc,lpOutPoint+=Inc)
		                                	{
		                                		if (FirstInRec)
		                                		{
		                                			_fstrcpy (OutRec,"#");  
		                                			FirstInRec = FALSE;
		                                		}
												sprintf (_fstrchr(OutRec,0),"%f|%f|%f|",lpOutPoint->x,lpOutPoint->y,NGIELV(*lpOrigPoint,hSurf,0));                                		
		                                	} 
		                                	if (i+1 >= nareas)  
		                                		_fstrcat (OutRec,"/"); 
		                                	if (numpoints > 1)
				                        		fputstring (OutRec,FidMIF);   
				                        	GlobalUnlock (hOutPoint);
				                        	GlobalUnlock (hOrigPoint);  
				                        }   
				                        GlobalUnlock (hSavePolyParts);
		                            	GlobalUnlock (hIndex);
                                	}
                                	else 
                                	{
		                        		fputstring (" 3003|1|1003|1|/",FidMIF); 
		                            	fputstring ("#+",FidMIF);  
	                                	FirstInRec=TRUE;
	   		                            lpOutPoint = (HPDPOINT)GlobalLock (hOutPoint);
	   		                            lpOrigPoint = (HPDPOINT)GlobalLock (hOrigPoint);
	   		                            if (HighlightData.PD.Area > 0) 
	   		                            {
	   		                            	lpOutPoint += (numpoints - 1);
	   		                            	lpOrigPoint += (numpoints - 1); 
	   		                            	Inc = -1;
	   		                            } 
	   		                            else
	   		                            	Inc = 1; 
	                                	for (i=0;i<numpoints;i++,lpOrigPoint+=Inc,lpOutPoint+=Inc)
	                                	{
	                                		if (FirstInRec)
	                                		{
	                                			_fstrcpy (OutRec,"#");  
	                                			FirstInRec = FALSE;
	                                		}
											sprintf (_fstrchr(OutRec,0),"%f|%f|%f|",lpOutPoint->x,lpOutPoint->y,NGIELV(*lpOrigPoint,hSurf,0));                                		
	                                	}   
	                                	_fstrcat (OutRec,"/");
			                        	fputstring (OutRec,FidMIF);   
			                        	GlobalUnlock (hOutPoint);
			                        	GlobalUnlock (hOrigPoint); 
		                        	}
                                } 
                                GSSiGlobFree (&hOutPoint);
                                GSSiGlobFree (&hOrigPoint);  
                                GSSiGlobFree (&hLinks); 
                                GSSiGlobFree (&hIndex);  
                                if (hCurvePoints)
                                {
	                            	GlobalUnlock (hCurvePoints); 
	                            	GlobalUnlock (hUnSplinedPoly); 
	                            }
	                            else
	                            	GlobalUnlock (hSavePoly);
                        	}
	                            
                        break;
                    }
                    if (WantRec)
                    {   
                    	LPSHORT	pnText;
                    	LPEXPORTTEXT		pEXText;
                    	LPEXPORTTEXTPOINTER	pEXTextPointer;
                    	short	nSubRecs = 1;
                    	
                    	if (!Pass && hExportText && PickList[0].Type == 4)
                    	{
   							pnText = (LPSHORT)GlobalLock (hExportText); 
							pEXText = (LPEXPORTTEXT)(pnText+1);
							nSubRecs = *pnText; 
                        }
                    	else if (Pass && hExportTextPointer && PickList[0].Type == 4)
                    	{   
                    		NumOutTPL++;
   							pnText = (LPSHORT)GlobalLock (hExportTextPointer); 
							pEXTextPointer = (LPEXPORTTEXTPOINTER)(pnText+1);
							nSubRecs = *pnText; 
                        }
						while (nSubRecs--)
						{	
							if (hExportText && PickList[0].Type == 4)
							{   
							    if (Pass) 
							    {
									if (pEXTextPointer->nPoints == 3)
									{
			                        	fputstring (" 3002|1|2|1|/",FidMIF); 
			                        	BP = pEXTextPointer->Loc[0];
			                        	ConvertCoordExport(&BP); 
			                        	POC = pEXTextPointer->Loc[1];
			                        	ConvertCoordExport(&POC); 
			                        	EP = pEXTextPointer->Loc[2];
			                        	ConvertCoordExport(&EP); 
			                        	sprintf (OutRec,"#%f|%f|%f|%f|%f|%f|%f|%f|%f|/",
			                        			 BP.x,BP.y,NGIELV (pEXTextPointer->Loc[0],hSurf,0),POC.x,POC.y,NGIELV (pEXTextPointer->Loc[1],hSurf,0),EP.x,EP.y,NGIELV (pEXTextPointer->Loc[2],hSurf,0)); 
								    }
								    else
								    {
			                        	fputstring (" 3002|1|2|1|/",FidMIF); 
			                        	BP = pEXTextPointer->Loc[0];
			                        	ConvertCoordExport(&BP); 
			                        	EP = pEXTextPointer->Loc[1];
			                        	ConvertCoordExport(&EP); 
			                        	sprintf (OutRec,"#%f|%f|%f|%f|%f|%f|/",BP.x,BP.y,NGIELV (pEXTextPointer->Loc[0],hSurf,0),EP.x,EP.y,NGIELV (pEXTextPointer->Loc[1],hSurf,0)); 
			                        }
		                        	fputstring (OutRec,FidMIF); 
		                        	SetGlobalValueLong ("%TPLSTYLE",pEXTextPointer->Type);
		                        }						    
							    else
							    {
									if (pEXText->Type == 2)
									{
			                        	fputstring (" 3002|1|2|2|/",FidMIF); 
			                        	BP = pEXText->Loc[0];
			                        	ConvertCoordExport(&BP); 
			                        	POC = pEXText->Loc[1];
			                        	ConvertCoordExport(&POC); 
			                        	EP = pEXText->Loc[2];
			                        	ConvertCoordExport(&EP); 
			                        	sprintf (OutRec,"#%f|%f|%f|%f|%f|%f|%f|%f|%f|/",
			                        			 BP.x,BP.y,NGIELV (pEXText->Loc[0],hSurf,0),POC.x,POC.y,NGIELV (pEXText->Loc[1],hSurf,0),EP.x,EP.y,NGIELV (pEXText->Loc[2],hSurf,0)); 
								    }
								    else
								    {
			                        	fputstring (" 3002|1|2|1|/",FidMIF); 
			                        	BP = pEXText->Loc[0];
			                        	ConvertCoordExport(&BP); 
			                        	EP = pEXText->Loc[2];
			                        	ConvertCoordExport(&EP); 
			                        	sprintf (OutRec,"#%f|%f|%f|%f|%f|%f|/",BP.x,BP.y,NGIELV (pEXText->Loc[0],hSurf,0),EP.x,EP.y,NGIELV (pEXText->Loc[2],hSurf,0)); 
			                        }
		                        	fputstring (OutRec,FidMIF); 
		                        	SetGlobalValueLong ("%TEXTFONT",pEXText->Font);
		                        	SetGlobalValueReal ("%TEXTSIZE",ConvertDist (fabs(pEXText->size)-fabs(pEXText->intleading),(short)PRJ_UNITS[3]));
		                        	SetGlobalValue ("%TEXTLINE",pEXText->Text);
		                        }
							}						
		                    _fstrcpy (OutRec,"#"); 
		                    FirstTPLField = TRUE; 
	                    	lpItems = (LPINT)GlobalLock (MIFOutFields); 
		                    for (i=0;i<nItems;i++,lpItems++) 
		                    {
		                        SendDlgItemMessage(hWndDlg,IDC_FIELDS,
		                                                   LB_GETTEXT,
		                                                   *lpItems,(LPARAM)Name); 
		                     
			                	if (Pass && i && !*lpItems)
			                	{ 
			                		if (FirstTPLField)
			                		{
			                			FirstTPLField = FALSE; 
			                			_fstrcpy (txt,"[%TPLSTYLE]");
			                			ExpandText (txt);
		                            	sprintf (_fstrchr(OutRec,0),"%s|",txt); 
			                		}
			                		break;
			                	}
			                    lpFldInfo = &FilePtrATT->FldInfo;
			                    lpFldInfo += *lpItems;
			                    if (*lpItems < FilePtrATT->NumFields) 
			                    {
				                    _fstrcpy (FldName,lpFldInfo->name);
			                    	FldLength = lpFldInfo->length;
			                    	FldType = lpFldInfo->type;
		                        	if (GetValFromOpenFiles (FldName,txt,128) < 0) 
		                        		*txt = 0;
			                    }
			                    else 
			                    {   
			                    	_fstrcpy (FldName,Name);
			                    	DecodeGMField (FldName,&FldType,&FldLength);
			                    	sprintf (txt,"[%s]",FldName); 
			                    	ExpandText (txt);
			                    }
		/*                        {   
		                            GlobalUnlock (MIFOutFields);   
		                            NumMiss++;
		                            goto NextHlt;  
		                        } */ 
							    switch (FldType)
							    {   
							    	case -1:
							    	case 0:
							        case SQL_CHAR: 
							        case SQL_VARCHAR:
							        case BT_RIGHT_CHAR:
									case SQL_UNKCHAR:
							        case BT_CHAR:  
							        	Strip (txt,'\'');
										ConvertOracleTimeDate (txt);
		                            	sprintf (_fstrchr(OutRec,0),"^^%s^^|",txt); 
		                            	break;
		                            default:
		                            	sprintf (_fstrchr(OutRec,0),"%s|",txt); 
		                            break;
		                        }
		                    }  
		                    GlobalUnlock (MIFOutFields);   
		                    fputstring (OutRec,FidMIF);  
		                    if (hExportText) 
		                    	pEXText++;
		            	}
		            	if (PickList[0].Type == 4)
		            	{
		            		if (!Pass) 
		                    	GlobalUnlock (hExportText);
		                    else  
		                    	GlobalUnlock (hExportTextPointer); 
		                } 
	                }    
            		DestroySavedPolys();                    
                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0);
                } 
                UseRefOrTAGIndex = SaveURORTI;
                FullCurves = FALSE;
                DisplaySymbol = SaveDisplaySymbol;
                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), NumItems, CurItem++,0);
		        SetDlgItemText(hWndDlg,IDC_MESSAGE,"Extract finished"); 
		        if (NumMiss && !*AutoExportName)
		        {   
		        	char	Mess[128];
		        	
		        	sprintf (Mess,"%ld records skipped due to missing attributes",NumMiss);
		        	GSSiMsgBox (hWndDlg,Mess,"",MB_OK,0);
		        }
		        else
			        Rtn=TRUE;
		        
                GSSiClose2 (&FidMIF); 
                GSSiClose2 (&FidSKP);
Exit:
                if (HaveTextPointers && ShapeType == 4 && !Pass)  
                {
                	Pass = 1;
                	goto TPLPass;      
                }
                CloseDataFile (FALSE,&MIFOuthDB);
Exit2:          

                GSSiGlobFree (&hExportText);  
                GSSiGlobFree (&hExportTextPointer);  
                SetContinueProcessing ( TRUE);   
                Processing = FALSE;
                GetDlgItemText (hWndDlg,IDC_MID_FILE,Name,sizeof(Name));
                ExpandText (Name);
               	pDOT = _fstrrchr (Name,'.');
               	_fstrcpy (pDOT,".TXT");
                FidMIF = GSSiOpenFile (Name,0,OF_CREATE);  
                sprintf (OutRec,"%ld records highlighted",NumItems);
                fputstring (OutRec,FidMIF);  
                sprintf (OutRec,"%ld points exported",NumOutPoints);
                fputstring (OutRec,FidMIF);  
                sprintf (OutRec,"%ld lines exported",NumOutLines);
                fputstring (OutRec,FidMIF);  
                sprintf (OutRec,"%ld polylines exported",NumOutPolyLines);
                fputstring (OutRec,FidMIF);  
                sprintf (OutRec,"%ld curves exported",NumOutCurves);
                fputstring (OutRec,FidMIF);  
                sprintf (OutRec,"%ld text records exported",NumOutText);
                fputstring (OutRec,FidMIF);  
                sprintf (OutRec,"%ld text placement lines exported",NumOutTPL);
                fputstring (OutRec,FidMIF);  
                sprintf (OutRec,"%ld areas exported",NumOutAreas);
                fputstring (OutRec,FidMIF);  
                     
                GSSiClose2 (&FidMIF);

				DTMClose (&hSurf);
				CloseTRANS2 (&hTranExport[0]);
				CloseTRANS2 (&hTranExport[1]);
				GSSiGlobFree(&MIFOutFields);
                GSSiGlobUlFree (&hOutRec);
                DisableHalt = FALSE;  
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT2),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_LOAD),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_SAVE),TRUE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE); 
                if (*AutoExportName)
                {
	                 CloseDataFile (FALSE,&MIFOuthDB);
	                 GSSiEndDialog(hWndDlg,Rtn,hSaveBM); 
	            }
                break;
            }
            case IDC_EXIT2:     
                 CloseDataFile (FALSE,&MIFOuthDB);
	             GSSiEndDialog(hWndDlg,FALSE,hSaveBM); 
                 
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)
					 SetContinueProcessing(FALSE);
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

BOOL FAR PASCAL GETSTREETNUMMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (745);
#endif
{ 
	int			TabStops[2]={1200,1300};
	static	long	InNum, LastNum=0; 
	long	num;
	char	TrueName[64];
	short	i, Trigger=3;

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (745);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:
         cwCenter(hWndDlg, 0); 
       	 InNum = StreetNum;
		 SendDlgItemMessage (hWndDlg,IDC_STREET_NAME_LIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
         if (!OpenStreetNameTable (FALSE))  
         {
	         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         }
         if (InNum)
         	num = InNum;
         else
         	num = LastNum;
		 GetTrueStreetName (num, TrueName, 0,0);
		 SetDlgItemText (hWndDlg,IDC_STREET_NAME,TrueName);
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */  
                 StreetNum = InNum;
                 EndDialog(hWndDlg, FALSE);
                 break;
            
            case IDC_STREET_NAME: /* Edit Control                            */
                 switch (HIWORD(wParam))
                 {  case EN_CHANGE:
                        i = GetDlgItemText (hWndDlg,IDC_STREET_NAME,TrueName,33);
                        DisplayStreetsINT (hWndDlg,IDC_STREET_NAME_LIST,TrueName,i,IDC_STREET_NAME,1,FALSE);
                        break;


                 }
                 break;

            case IDC_STREET_NAME_LIST: /* List box                           */
                 switch(HIWORD(wParam))
                 {   int	choice;
                 	 char	str[64]; 
                 	 LPSTR	lpBar;
                 
                 	 case LBN_SELCHANGE:
						choice=SendDlgItemMessage(hWndDlg,IDC_STREET_NAME_LIST,LB_GETCURSEL,0,0);
						SendDlgItemMessage(hWndDlg,IDC_STREET_NAME_LIST,LB_GETTEXT,choice,(DWORD)str); 
						if ((lpBar = _fstrchr (str,'\t')))
							*lpBar = 0;
						SetDlgItemText (hWndDlg,IDC_STREET_NAME,str);
					 break;
                 	 	 
                     case LBN_DBLCLK:
				         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
		 		     break;
		 		 }
		 		 break;   
		 		 
		 	case IDC_REMOVESTREETNAME:
				 EndDialog(hWndDlg, 2); 
		 		 break; 
		 		 
            case IDOK: 
            {
            	 char Name[34],TrueName[34];
				 char	mess[256];
            	  
                 GetDlgItemText (hWndDlg,IDC_STREET_NAME,Name,32); 
                 _fstrupr (Name);
                 if ((StreetNum = GetStreetNumFromName (Name,1,BT_FIRST,CurrentStreetName))) 
                 {
					 CloseStreetNameTable(); 
					 CurPath = StreetNum; 
					 LastNum = StreetNum;
					 EndDialog(hWndDlg, TRUE); 
                 }
                 else 
                 {
                 	char	str[128]; 
                 	STNDSN_INIT(FALSE);

					STNDST(Name, _fstrlen(Name),STDNAMv,NRONAMv,NMONLYv,
							     SANSCHv,NANDCHv,NCMPNMv,ORIGNMv,SANSCPv,SANSCSv,0,0,0,0);   
					if ((StreetNum = GetStreetNumFromName (STDNAMv,2,BT_FIRST,str)))
					{
						SendDlgItemMessage (hWndDlg,IDC_STREET_NAME_LIST,LB_RESETCONTENT,0,0);
						while (StreetNum)
						{
						//	_fstrcat(str,"\t");
						//	ltoa (StreetNum,_fstrchr(str,0),10);
							SendDlgItemMessage (hWndDlg,IDC_STREET_NAME_LIST,LB_ADDSTRING,0,(LPARAM)str);
						    StreetNum = GetStreetNumFromName (STDNAMv,2,BT_NEXT,str);
						}
						sprintf (mess,"There are one or more names similar to '%s' already in the table (dislayed in the list). Select one or pick Add Name?",Name);
						GSSiMsgBox (hWndDlg,mess,0,MB_OK,0);
					}
                 	else
                 	{
	                 	sprintf (mess,"'%s' is not in the street name table. Pick 'Add Name' if you wish to add this name",Name);
					 	GSSiMsgBox (hWndDlg,mess,0,0,0); 
					} 
                 }
                 break;
				 
            }   
            case IDC_ADD_NAME: 
            {
            	 char Name[34],TrueName[34], str[128];
            	  
                 GetDlgItemText (hWndDlg,IDC_STREET_NAME,Name,32); 
                 
STNDSN_INIT(FALSE);
				 STNDST(Name, _fstrlen(Name),STDNAMv,NRONAMv,NMONLYv,
				 						     SANSCHv,NANDCHv,NCMPNMv,ORIGNMv,SANSCPv,SANSCSv,0,0,0,0);   
                 if ((StreetNum = GetStreetNumFromName (STDNAMv,2,BT_FIRST,str)))
                 {
                 	char	mess[256];
                 	
					SendDlgItemMessage (hWndDlg,IDC_STREET_NAME_LIST,LB_RESETCONTENT,0,0);
					while (StreetNum)
					{
				//		_fstrcat(str,"\t");
				//		ltoa (StreetNum,_fstrchr(str,0),10);
			 			SendDlgItemMessage (hWndDlg,IDC_STREET_NAME_LIST,LB_ADDSTRING,0,(LPARAM)str);
                 	    StreetNum = GetStreetNumFromName (STDNAMv,2,BT_NEXT,str);
                 	}
                 	sprintf (mess,"There are one or more names similar to '%s' already in the table (dislayed in the list). Do you still wish to enter this new name?",Name);
				 	if (GSSiMsgBox (hWndDlg,mess,0,MB_YESNO,0) == IDNO)
				 		break;
                 }
                 StreetNum = AddStreetName (Name,0,"","","",""); 
				 CurPath = StreetNum; 
				 LastNum = StreetNum;
                 _fstrcpy(CurrentStreetName,Name);
				 CloseStreetNameTable();
				 EndDialog(hWndDlg, TRUE); 
	             break; 
            }
		  }
		  break;

    default:
{
#if ENABLETRACE
GSSiExitProg (745);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (745);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL TEXTSTRINGMsgProc(HWND hWndDlg, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1151);
#endif
{ 	
  
  HFILE Fid;
  OFSTRUCTGM OFStruct;
  HANDLE	hSTR;  
  static	HANDLE	hSaveBM;
  LPSTR		str, lpBAR; 
  static	BOOL	UseBAR;       
  int		Item,DefaultItem=-1;
  long		Loc; 
  DWORD		CBData;
  WORD		GeneratedInc;  
  static	POINT	CursorLoc;
  static	long	FileLength;
  BOOL		rtn = FALSE;
 int	BRtn;
 BOOL		centerOnCursor = TRUE;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1151);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:   
    	 hSaveBM = EnterBlockingWindow (hWndDlg);
    	 UseBAR = FALSE; 
    	 if (maxTEXTSTRING < 0)
    	 {
         	Wait (1000); //allows installer time to exit
    	 	maxTEXTSTRING *= -1;
         	cwCenter(hWndDlg, SHRT_MAX);   
         	BringWindowToTop(hWndDlg);
         }
		 else if (centerOnCursor)
			 cwCenter(hWndDlg, -2);   
		 else
			cwCenter(hWndDlg, 0);//-2);   
		 GetCursorPos (&CursorLoc);
         SetWindowText (hWndDlg,pTEXTSTRINGTITLE); 
         if (pTEXTSTRINGLIST && *pTEXTSTRINGLIST)   
         {  
         	if (*pTEXTSTRINGLIST == '*')
         	{
         		char	txt[128];
         		
         		_fstrcpy (txt,pTEXTSTRINGLIST);
         		DlgDirListComboBox (hWndDlg,txt,IDC_STRINGLIST,0,DDL_READWRITE);
         	}
         	else if (*pTEXTSTRINGLIST == '(')
         	{   
         		LPSTR	pVal = pTEXTSTRINGLIST+1;
         		LPSTR	pComma = _fstrchr (pVal,',');
         		LPSTR	pEnd = LastChr (pTEXTSTRINGLIST);
         		
         		if (*pEnd == ')')
         			*pEnd = 0; 
         		while (pVal)
         		{   
         			if (pComma)
         				*pComma++ = 0;
         			if (*pVal)
                		Item = SendDlgItemMessage (hWndDlg,IDC_STRINGLIST,CB_ADDSTRING,0,(LPARAM)pVal);   
                	pVal = pComma; 
                	if (pVal)
                		pComma = _fstrchr (pVal,',');
                } 
         	}
         	else
         	{
	         	Fid = GSSiOpenFile (pTEXTSTRINGLIST,&OFStruct,OF_READ);
	         	if (Fid == HFILE_ERROR) 
	         	{
	                 GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
	                 break;
	            }
	            hSTR = GSSiGlobAlloc (1248,GMEM_MOVEABLE,1024);
	            str = GlobalLock (hSTR);
		        FileLength = GSSillseek (Fid,0,2);
	         	Loc = GSSillseek (Fid,0,0);
	         	while (fgetstring (str,1020,Fid))
	         	{   
	         		if (FileLength < USHRT_MAX && !_fstrnicmp (str,"$GENLIST(",9))
	         		{
	         			DefaultItem = GenList (hWndDlg,IDC_STRINGLIST,TRUE,TRUE,str,pINITVAL,&UseBAR,0,0);
	         		}
	         		else
	         		{
		         		lpBAR = _fstrchr (str,'|');
		         		if (lpBAR)
		         		{
		         			*lpBAR = 0;
		         			UseBAR = TRUE;
		         		} 
						if (FileLength > USHRT_MAX)
							CBData = Loc;
						else
		         			CBData = MAKELPARAM ((WORD)Loc,0);
		                Item = SendDlgItemMessage (hWndDlg,IDC_STRINGLIST,CB_ADDSTRING,0,(LPARAM)str); 
			         	SendDlgItemMessage (hWndDlg,IDC_STRINGLIST,CB_SETITEMDATA,(WPARAM)Item,(LPARAM)CBData);
			        }
		         	Loc = GSSillseek (Fid,0,1);
	            }
	            GSSiClose2 (&Fid);
	            GSSiGlobUlFree (&hSTR);
	        }
         	ShowWindow (GetDlgItem(hWndDlg,IDC_STRINGLIST),SW_SHOW); 
         	SetFocusAndCursor (GetDlgItem(hWndDlg,IDOK));
            if (pINITVAL)
		    	SendDlgItemMessage (hWndDlg,IDC_STRINGLIST,CB_SELECTSTRING,-1,(LPARAM)pINITVAL);
		    else
	    		SendDlgItemMessage (hWndDlg,IDC_STRINGLIST,CB_SETCURSEL,DefaultItem,0L);
         }
         else
         {
         	ShowWindow (GetDlgItem(hWndDlg,IDC_STRING),SW_SHOW);   
         	if (AutoIncIntVal)
         	{
         		long	lval = atol (pINITVAL);
         		lval += AutoIncIntVal;
         		ltoa (lval,pINITVAL,10);
         		SetFocusAndCursor (GetDlgItem(hWndDlg,IDOK));
         	}
         	else
         		SetFocusAndCursor (GetDlgItem(hWndDlg,IDC_STRING));
         	SetDlgItemText(hWndDlg,IDC_STRING,pINITVAL);
         }
		 rtn = TRUE;
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
		 rtn = TRUE;
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDC_STRING:
            	switch (HIWORD(wParam))
            	{
            	        case EN_SETFOCUS: 
            	        	SetPrompt (PRMT_GETSTRING,FALSE);
                        break;

                }
                break;
            
            case IDOK: 
       	         if (pTEXTSTRINGLIST && *pTEXTSTRINGLIST)
       	         {
				    Item=SendDlgItemMessage(hWndDlg,IDC_STRINGLIST,CB_GETCURSEL,0,0);
				    if (Item < 0) break;
       	         	if (UseBAR) 
       	         	{
                	 	CBData = SendDlgItemMessage(hWndDlg,IDC_STRINGLIST,CB_GETITEMDATA,Item, (LPARAM) 0);
                	 	if (FileLength > USHRT_MAX)
						{
							Loc = CBData;
							GeneratedInc = 0;
						}
						else
						{
							Loc = LOWORD (CBData);
                	 		GeneratedInc = HIWORD (CBData);
						}
			         	Fid = GSSiOpenFile (pTEXTSTRINGLIST,&OFStruct,OF_READ);
			            hSTR = GSSiGlobAlloc (1249,GMEM_MOVEABLE,1024);
			            str = GlobalLock (hSTR);  
			            GSSillseek (Fid,Loc,0);
			         	fgetstring (str,1020,Fid); 
			         	if (GeneratedInc)
			         	{ 
			         		GetGenListVal (str,GeneratedInc-1,str);
			         	}
			         	else
			         	{
				         	if ((lpBAR = _fstrchr (str,'|')))
				         	{   
				         		*lpBAR++ = 0;
				         		if (pINITVAL)
				         			_fstrcpy (pINITVAL,str);
				         		str = lpBAR;
				         	}	
				         	if ((lpBAR = _fstrchr (str,'|')))
				         	{   
				         		*lpBAR++ = 0;
				         		ExpandText (lpBAR);
				         	}
				         	else
				         		ExpandText (str);
				        }	
			         	_fstrncpy (pTEXTSTRING,str,maxTEXTSTRING);
			         	GSSiClose2 (&Fid);
			         	GSSiGlobUlFree (&hSTR);
       	         	}
       	         	else 
       	         	{
	            		GetDlgItemText(hWndDlg,IDC_STRINGLIST,pTEXTSTRING,maxTEXTSTRING+1); 
		         		if (pINITVAL)
		         			_fstrcpy (pINITVAL,pTEXTSTRING); 
		         	}
	             }
	             else
	             {
	            	 GetDlgItemText(hWndDlg,IDC_STRING,pTEXTSTRING,maxTEXTSTRING+1);
		         		if (pINITVAL)
		         			_fstrcpy (pINITVAL,pTEXTSTRING);
		         } 
                 GSSiEndDialog(hWndDlg,TRUE,hSaveBM); 
               	 //SetCursorPos (CursorLoc.x,CursorLoc.y);

              break;   
              
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
               	 //SetCursorPos (CursorLoc.x,CursorLoc.y);
                 break;
           }
		 rtn = TRUE;
         break;    /* End of WM_COMMAND                                 */

    default:
		rtn = FALSE;
		break;
   }
{
#if ENABLETRACE
GSSiExitProg (1151);
#endif

	return rtn;
}
#if ENABLETRACE
}
#endif
} /* End of TEXTSTRINGMsgProc                                      */

