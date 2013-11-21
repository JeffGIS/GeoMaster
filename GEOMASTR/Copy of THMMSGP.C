#include "graphint.h"
#include "extrndb.h"

static	COLORREF	SaveClassColor[MAX_THEME_CLASSES]; 
static	char	ClassLink[MAX_THEME_CLASSES][256]; 
static	char	SELVALDataFile[128]="",SELVALSQL[256]="",SELVALFieldName[66]="";
static	short	SELVALDataFileType=0;
static	HANDLE	SELVALhDB=0;
static	HWND	hWndSELVAL=0;
static	FARPROC lpfnSELECTVALUESMsgProc;
static	BOOL	SaveColorsInUse=FALSE;
static	COLORREF	SaveBKGColor;     
static	char	ViewportStatusMessages[MAX_VIEWPORTS][128];

#include "gmextern.h"

#define MAXHOTSPOTDIMENSION	500

BOOL FAR PASCAL STREET_TEXTMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1276);
#endif
{	
	int	IDC_FieldName=SV_FIELD_NAME; 
	BOOL	True=TRUE, Err; 
	short	show;   
	COLORREF	Color;

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1276);
#endif
 	return (BRtn);
}
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam,IDC_SQL,
 					 SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, &IDC_FieldName,1,
 					 CurTheme->DataFile, &CurTheme->DataFileType, &CurTheme->hThemeDB, &True,FALSE))
{
#if ENABLETRACE
GSSiExitProg (1276);
#endif
 					  return TRUE;
}
 if (ThemeCommonCode (hWndDlg, Message, wParam, lParam,CurTheme->hThemeDB))
{
#if ENABLETRACE
GSSiExitProg (1276);
#endif
 	return TRUE;
}
 switch(Message)
   {
    case WM_INITDIALOG:
    {
	    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurTheme->ClassBM;

        SetDlgItemText(hWndDlg,SV_FIELD_NAME,(LPCSTR)&CurTheme->Field.name); 
        SetDlgItemText(hWndDlg,IDC_SQL,(LPCSTR)&CurTheme->SQL);
       	SendDlgItemMessage (hWndDlg,IDC_SKIPNOREC,BM_SETCHECK,CurTheme->MissOpt,0L);  
	    SetDlgItemInt (hWndDlg,IDC_LAYERID,pStreetData->LayerID,TRUE); 
	    show = SW_HIDE;
       	SendDlgItemMessage (hWndDlg,IDC_ITALIC,BM_SETCHECK,pStreetData->Italic,0L);  
       	
       	SendDlgItemMessage (hWndDlg,IDC_SHADOWTEXT,BM_SETCHECK,pStreetData->Shadow,0L);  
       	SendDlgItemMessage (hWndDlg,IDC_OPAQUETEXT,BM_SETCHECK,pStreetData->BackgroundOpt,0L);  
       	SendDlgItemMessage (hWndDlg,IDC_HORTEXT,BM_SETCHECK,pStreetData->HorizontalText,0L);  
       	SendDlgItemMessage (hWndDlg,IDC_ABOVELINE,BM_SETCHECK,!pStreetData->VJust,0L);  
   	 	SetDlgItemText (hWndDlg,IDC_EXPRESSION,pStreetData->Expression);
   	 	SetDlgItemInt (hWndDlg,IDC_EXLEN,pStreetData->NameLength,FALSE); 
   	 	SetDlgItemText (hWndDlg,IDC_LISTDB,pStreetData->ListFile);
   	 	SetDlgItemText (hWndDlg,IDC_LISTDATA,pStreetData->ListData);
   	 	SetDlgItemInt (hWndDlg,IDC_LISTLEN,pStreetData->ListLen,FALSE); 
   	 	SetDlgItemInt (hWndDlg,IDC_BOUNDSCUTOFF,pStreetData->MinSize,FALSE); 
   	 	SetDlgItemText (hWndDlg,IDC_NAMEMACRO,pStreetData->DisplayNameMacro);
   	 	SetDlgItemText (hWndDlg,IDC_COLORMACRO,pStreetData->ColorMacro);
   	 	SetDlgItemText (hWndDlg,IDC_VISMACRO,pStreetData->VisMacro);
    case GSSI_REINITDIALOG:
		pStreetData=(LPSTREETTEXTDATA)CurTheme->ClassBM;
	    if (pStreetData->NameSource)
	    {
       		SendDlgItemMessage (hWndDlg,IDC_EXTABLE,BM_SETCHECK,TRUE,0L);  
       		show = SW_SHOW;
	   	} 
       	else
       		SendDlgItemMessage (hWndDlg,IDC_USE_STREETS,BM_SETCHECK,TRUE,0L);
   		SendDlgItemMessage (hWndDlg,IDC_SHOW_CITIES,BM_SETCHECK,pStreetData->ShowCities,0L);
   		SendDlgItemMessage (hWndDlg,IDC_SCALETEXT,BM_SETCHECK,pStreetData->ScaleText,0L);
   		SendDlgItemMessage (hWndDlg,IDC_ALLOWOVERLAP,BM_SETCHECK,pStreetData->AllowOverlap,0L);  
   		SendDlgItemMessage (hWndDlg,IDC_ALLOWHOLLOW,BM_SETCHECK,pStreetData->AllowHollow,0L);
   		SendDlgItemMessage (hWndDlg,IDC_USEFONT,BM_SETCHECK,pStreetData->UseFont,0L);
   		SendDlgItemMessage (hWndDlg,IDC_USESHIELDS,BM_SETCHECK,!pStreetData->IgnoreShields,0L);
   		SendDlgItemMessage (hWndDlg,IDC_SHOW_ALLELEM,BM_SETCHECK,pStreetData->ShowAllElements,0L); 
   	 	ShowWindow (GetDlgItem(hWndDlg,SV_DATABASE_LIST),show); 
//   	 	ShowWindow (GetDlgItem(hWndDlg,SV_TABLE_NAMES),show); 
//   	 	ShowWindow (GetDlgItem(hWndDlg,SV_TABLE_HEADING),show); 
   	 	ShowWindow (GetDlgItem(hWndDlg,IDC_EXPRESSION),show); 
   	 	ShowWindow (GetDlgItem(hWndDlg,IDC_EXLEN),show); 
   	 	ShowWindow (GetDlgItem(hWndDlg,IDC_EXLEN2),show); 
   	 	ShowWindow (GetDlgItem(hWndDlg,IDC_SQL),show); 
   	 	ShowWindow (GetDlgItem(hWndDlg,IDC_SETSQL),show); 
   	 	ShowWindow (GetDlgItem(hWndDlg,SV_DATA_FILE),show); 
   	 	ShowWindow (GetDlgItem(hWndDlg,SV_DATA_FILE2),show);
   	 	ShowWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),show);
    }  
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */
                       
    case WM_COMMAND:
    {
		 LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurTheme->ClassBM;    
         switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 CloseThemeDataFile(TRUE);
				 DestroyFieldList ();
                 EndDialog(hWndDlg, FALSE);
                 break;
            case IDC_STREETTEXTFONT:
            	 pStreetData->StreetTextFont.lfHeight = pStreetData->TextSize * BaseDistToWinDist;
            	 GetFont (hWndDlg, &pStreetData->StreetTextFont, &pStreetData->TextColor);
            	 pStreetData->TextSize = pStreetData->StreetTextFont.lfHeight / BaseDistToWinDist;
            	 break;
            case IDC_SHADOWCOLOR:
           		 Color = pStreetData->ShadowColor;  
            	 if (GetColor(hWndDlg,&Color))
            	 	pStreetData->ShadowColor = Color;
            	 break;
            
           	case IDC_TEXTCOLOR: 
           		 Color = pStreetData->TextColor; 
            	 if (GetColor(hWndDlg,&Color))
            	 	pStreetData->TextColor = Color;
           		 break;
            
            case IDC_USE_STREETS:  
            	 pStreetData->NameSource = 0;
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
                 break;
            case IDC_EXTABLE:
            	 pStreetData->NameSource = 1;
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
                 break;
            case IDC_SHOW_FIELDS: 
            	 
            	 DisplayFieldList (hWndDlg,CurTheme->hThemeDB,NULL,0);
                 break;
                      
            case IDC_SAVE_THEME:  
            case IDOK:
            {
            	HANDLE	hItems;
            	LPSHORT	lpItems;
			    BOOL	Err; 
			    LPSTR	lpEQ;  
			    char	FieldName[128];
            	
				OpenThemeDataFile ();  
            	pStreetData->ShowAllElements = SendDlgItemMessage (hWndDlg,IDC_SHOW_ALLELEM,BM_GETCHECK,0,0L);  
            	pStreetData->UseFont = SendDlgItemMessage (hWndDlg,IDC_USEFONT,BM_GETCHECK,0,0L);  
            	pStreetData->IgnoreShields = !SendDlgItemMessage (hWndDlg,IDC_USESHIELDS,BM_GETCHECK,0,0L);  
            	pStreetData->ShowCities = SendDlgItemMessage (hWndDlg,IDC_SHOW_CITIES,BM_GETCHECK,0,0L);  
            	pStreetData->ScaleText = SendDlgItemMessage (hWndDlg,IDC_SCALETEXT,BM_GETCHECK,0,0L);  
            	pStreetData->AllowOverlap = SendDlgItemMessage (hWndDlg,IDC_ALLOWOVERLAP,BM_GETCHECK,0,0L);  
            	pStreetData->AllowHollow = SendDlgItemMessage (hWndDlg,IDC_ALLOWHOLLOW,BM_GETCHECK,0,0L);  
            	pStreetData->Italic = SendDlgItemMessage (hWndDlg,IDC_ITALIC,BM_GETCHECK,0,0L);  
            	pStreetData->Shadow = SendDlgItemMessage (hWndDlg,IDC_SHADOWTEXT,BM_GETCHECK,0,0L);  
            	CurTheme->MissOpt = SendDlgItemMessage (hWndDlg,IDC_SKIPNOREC,BM_GETCHECK,0,0L);  
            	pStreetData->HorizontalText = SendDlgItemMessage (hWndDlg,IDC_HORTEXT,BM_GETCHECK,0,0L);  
            	pStreetData->VJust = !SendDlgItemMessage (hWndDlg,IDC_ABOVELINE,BM_GETCHECK,0,0L);  
            	pStreetData->BackgroundOpt = SendDlgItemMessage (hWndDlg,IDC_OPAQUETEXT,BM_GETCHECK,0,0L);  
            	pStreetData->LayerID = GetDlgItemInt (hWndDlg,IDC_LAYERID,&Err,TRUE);
   	 			GetDlgItemText (hWndDlg,IDC_EXPRESSION,pStreetData->Expression,256);
   	 			pStreetData->NameLength = GetDlgItemInt (hWndDlg,IDC_EXLEN,&Err,FALSE); 
		   	 	GetDlgItemText (hWndDlg,IDC_LISTDB,pStreetData->ListFile,128);
		   	 	GetDlgItemText (hWndDlg,IDC_LISTDATA,pStreetData->ListData,128);
		   	 	GetDlgItemText (hWndDlg,IDC_NAMEMACRO,pStreetData->DisplayNameMacro,255);
		   	 	GetDlgItemText (hWndDlg,IDC_COLORMACRO,pStreetData->ColorMacro,255);
		   	 	GetDlgItemText (hWndDlg,IDC_VISMACRO,pStreetData->VisMacro,255);
   	 			pStreetData->ListLen = GetDlgItemInt (hWndDlg,IDC_LISTLEN,&Err,FALSE);
   	 			pStreetData->ListLen = max (2,pStreetData->ListLen); 
   	 			pStreetData->MinSize = GetDlgItemInt (hWndDlg,IDC_BOUNDSCUTOFF,&Err,FALSE);
/*		        GetDlgItemText(hWndDlg,SV_FIELD_NAME,FieldName,128);
                GetFieldDefFromOpenFiles (FieldName,&CurTheme->Field[0]);
                pStreetData->NameLength = CurTheme->Field[0].length;*/ 
       	 		GetDlgItemText (hWndDlg,IDC_SQL,CurTheme->SQL,256);   
       	 		GetDlgItemText (hWndDlg,SV_DATABASE_LIST,CurTheme->DataFile,sizeof(CurTheme->DataFile));
       	 		if ((lpEQ = _fstrchr (CurTheme->SQL,'=')))
       	 		{
       	 			lpEQ++;
       	 			_fstrcpy (pStreetData->Macro,lpEQ);
       	 		}
       	 		else
       	 			*pStreetData->Macro = 0;
                if (wParam == IDC_SAVE_THEME)
            		SaveCurTheme(hWndDlg);
				else 
				{   
	                CloseThemeDataFile(TRUE);
					DestroyFieldList ();
                	EndDialog(hWndDlg, TRUE);
                }
            }
                break;

           }
    }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1276);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1276);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}
   
BOOL FAR PASCAL BOUNDS_DISPLAYMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1277);
#endif
{	
 char	str[256];
 LPBOUNDSDISPLAY lpBoundsDisplay;
 
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1277);
#endif
 	return (BRtn);
}
 if (ThemeCommonCode (hWndDlg, Message, wParam, lParam,0))
{
#if ENABLETRACE
GSSiExitProg (1277);
#endif
 	return TRUE;
}
 switch(Message)
   {
    case WM_INITDIALOG:  
		 if (CurTheme->ID != GF_BOUNDS_DISPLAY_THEME)
		 	break;
		 lpBoundsDisplay = (LPBOUNDSDISPLAY)&CurTheme->ClassBM; 
		 SetDlgItemText (hWndDlg,BD_MACRO,CurTheme->Title);        
		 SetDlgItemText (hWndDlg,IDC_FOCUSREFGLOBAL,lpBoundsDisplay->CurAreaRefGlobal);        
		 SetDlgItemText (hWndDlg,IDC_BOUNDSGLOBAL,lpBoundsDisplay->BoundsGlobal);        
       	 SendDlgItemMessage (hWndDlg,IDC_SHOWAREAB,BM_SETCHECK,CurTheme->ZeroIsMissing,0L); 
       	 SendDlgItemMessage (hWndDlg,IDC_MASKAREA,BM_SETCHECK,CurTheme->AddCommas,0L); 
       	 SendDlgItemMessage (hWndDlg,IDC_DOCLEAR,BM_SETCHECK,!lpBoundsDisplay->IgnoreClear,0L); 
       	 SendDlgItemMessage (hWndDlg,IDC_KEEPBOUNDS,BM_SETCHECK,!lpBoundsDisplay->IgnoreClear,0L); 
       	 sprintf (str,"%f",CurTheme->RefValDbl);
       	 SetDlgItemText (hWndDlg,IDC_VPOFFSET,str);

		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
            case IDC_SAVE_THEME:  
            case IDOK:
            {
            	HANDLE	hItems;
            	LPSHORT	lpItems;
            	
                if (wParam == IDC_SAVE_THEME)
            		SaveCurTheme(hWndDlg);
				else 
				{   
					if (CurTheme->ID == GF_BOUNDS_DISPLAY_THEME)
					{   
						lpBoundsDisplay = (LPBOUNDSDISPLAY)&CurTheme->ClassBM; 
						GetDlgItemText (hWndDlg,IDC_FOCUSREFGLOBAL,lpBoundsDisplay->CurAreaRefGlobal,34);        
						GetDlgItemText (hWndDlg,IDC_BOUNDSGLOBAL,lpBoundsDisplay->BoundsGlobal,34);        
			       	    GetDlgItemText (hWndDlg,IDC_VPOFFSET,str,32);
			       	    CurTheme->RefValDbl = atof (str);
	            	 	CurTheme->ZeroIsMissing = SendDlgItemMessage (hWndDlg,IDC_SHOWAREAB,BM_GETCHECK,0,0L);  
	            	 	CurTheme->AddCommas = SendDlgItemMessage (hWndDlg,IDC_MASKAREA,BM_GETCHECK,0,0L);  
	            	 	lpBoundsDisplay->IgnoreClear = !SendDlgItemMessage (hWndDlg,IDC_DOCLEAR,BM_GETCHECK,0,0L);  
	            	 	lpBoundsDisplay->VPBoundsOpt = SendDlgItemMessage (hWndDlg,IDC_KEEPBOUNDS,BM_GETCHECK,0,0L);  
						GetDlgItemText (hWndDlg,BD_MACRO,CurTheme->Title,256);
				        BoundsDisplayDestroy (CurView->lpBoundsDisplay); 
				        CurView->lpBoundsDisplay=0;
						CurView->Type = 7;
						CurView->ZoomTarget = CurTheme->TargetViewport;   
						ResetFunStack (TRUE);
						AddGraphicsFunction (CurView->hWnd, GF_PAN_ZOOM_TARGET,0);
						CurView->StartupFunction = GF_PAN_ZOOM_TARGET;   
					}    	
                	EndDialog(hWndDlg, TRUE);
                }
            }
                break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1277);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1277);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL FAR PASCAL DISTANCEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1278);
#endif
{	
 char	str[256];
 LPBOUNDSDISPLAY lpBoundsDisplay;
 
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1278);
#endif
 	return (BRtn);
}
 if (ThemeCommonCode (hWndDlg, Message, wParam, lParam,0))
{
#if ENABLETRACE
GSSiExitProg (1278);
#endif
 	return TRUE;
}
 switch(Message)
   {
    case WM_INITDIALOG:  
         SendDlgItemMessage (hWndDlg,IDC_DDTYPE,CB_ADDSTRING,0,(LPARAM)"Simple");
         SendDlgItemMessage (hWndDlg,IDC_DDTYPE,CB_ADDSTRING,0,(LPARAM)"Scale Bar");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Auto");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Yards");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Miles");
         SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_ADDSTRING,0,(LPARAM)"Kilometers");
		 SendDlgItemMessage (hWndDlg,IDC_DDTYPE,CB_SETCURSEL,CurTheme->ClassType-1,NULL);
		 SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_SETCURSEL,max(0,CurTheme->ValConv),NULL);
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
            case IDC_SAVE_THEME:  
            	 SaveCurTheme(hWndDlg);
            	 break;
            case IDOK:
            {   
            	CurTheme->ClassType = SendDlgItemMessage (hWndDlg,IDC_DDTYPE,CB_GETCURSEL,0,0)+1;
            	CurTheme->ValConv = SendDlgItemMessage (hWndDlg,IDC_UNITS,CB_GETCURSEL,0,0);
               	EndDialog(hWndDlg, TRUE);
            }
                break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1278);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1278);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}
 
BOOL FAR PASCAL COORDGRIDMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1280);
#endif
{	
 char	str[256];
 LPBOUNDSDISPLAY lpBoundsDisplay;
 
 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1280);
#endif
 	return (BRtn);
}
 if (ThemeCommonCode (hWndDlg, Message, wParam, lParam,0))
{
#if ENABLETRACE
GSSiExitProg (1280);
#endif
 	return TRUE;
}
 switch(Message)
   {
    case WM_INITDIALOG:  

		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
            case IDC_SAVE_THEME:  
            	 SaveCurTheme(hWndDlg);
            	 break;
            case IDOK:
            {
               	EndDialog(hWndDlg, TRUE);
            }
                break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1280);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1280);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}         

BOOL GetDMSMajorMinorInc (double DegDifx,LPDOUBLE pMajor, LPDOUBLE pMinor, LPSHORT pFormat)
#if ENABLETRACE
{GSSiEnterProg (1281);
#endif
{   
	double	MajorInc, MinorInc, SecDif,MIN=(double)1/60,SEC=MIN/60;
	short	DegDif, MinDif, fmt=0;
	
	GetDMS (DegDifx,&DegDif,&MinDif,&SecDif); 
	if (DegDif>10)
	{
		MajorInc = 2;
		MinorInc = 1;
	}
	else if (DegDif>2)
	{
		MajorInc = 1;
		MinorInc = MIN*10;
	}
	else
	{   
		fmt = 1;
		MinDif += DegDif*60;
		if (MinDif > 40)
    	{
    		MajorInc = MIN*20;
    		MinorInc = MIN*2;
    	}
		else if (MinDif > 20)
    	{
    		MajorInc = MIN*10;
    		MinorInc = MIN;
    	}
    	else if (MinDif > 10)
    	{
    		MajorInc = MIN*5;
    		MinorInc = MIN;
    	}
    	else if (MinDif > 5)
    	{
    		MajorInc = MIN * 2;
    		MinorInc = SEC * 10;
    	}
    	else if (MinDif > 2)
    	{
    		MajorInc = MIN;
    		MinorInc = SEC * 10;
    	}
    	else 
    	{   
    		fmt = 2;
    		SecDif += 60 * MinDif;
    		if (SecDif > 100)
    		{
	    		MajorInc = SEC * 30;
	    		MinorInc = SEC * 10;
    		}
    		else if (SecDif > 40)
    		{
	    		MajorInc = SEC * 20;
	    		MinorInc = SEC * 10;
    		}
    		else if (SecDif > 20)
    		{
	    		MajorInc = SEC * 10;
	    		MinorInc = SEC * 1;
    		}
    		else if (SecDif > 10)
    		{
	    		MajorInc = SEC * 5;
	    		MinorInc = SEC * 1;
    		}
    		else if (SecDif > 5)
    		{
	    		MajorInc = SEC * 2;
	    		MinorInc = SEC * 1;
    		}
    		else if (SecDif > 2)
    		{
	    		MajorInc = SEC;
	    		MinorInc = SEC/10;
	    	}
	    	else
	    	{
	    		fmt = 3;
	    		MajorInc = SEC / 2;
	    		MinorInc = SEC / 10; 
	    	}
	    }
	}  
	*pMajor = MajorInc;
	*pMinor = MinorInc; 
	*pFormat = fmt;
{
#if ENABLETRACE
GSSiExitProg (1281);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}
	
 
void DisplayCoordGridThemeLegend(short From)
#if ENABLETRACE
{GSSiEnterProg (1282);
#endif
{    
	short	CurViewID = CurView->ID, reps, h,w;    
	double	BaseDistPerPixel, MaxWidth, BarWidth, step;
	POINT	Points[5], MinorPoints[2];  
	long	IntVal;  
	char	txt[128], str[16], fmt[16]=" %.0f mile "; 
	HPEN	OldPen, LinePen, RedPen, WhitePen;    
	BOOL	DoMinorTics = GetGlobalBVal2 ("[%MINORTICS]",FALSE);
	
//	SetCurView (pViewports[CurTheme->TargetViewport-1]); //tempdebu
	SetViewport (CurTheme->TargetViewport);   
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn (2,FALSE);
  	SelectClipRgn (CurView->hDC,CurView->hRgn);
  	GSSiDeleteObject(&CurView->hRgn);
    SetTextColor (CurView->hDC,0);  
    if (CurView->MarginPan && !Printing)
	{   
		POINT	Point;
		RECT	Rect;
		short	symnum, width, height;
	    LPSYMBOL    CurSymbol;
	    HANDLE	hSymbol;
	    BOOL	SaveHVFC=HaveVarFillColor;
	    HBRUSH	hOldBrush; 
	    double	Rot=0, QuaterPY=HALFPI/2.0;
		
		
	    width = CurView->Rect.right - CurView->DrawRect.right -2;
	    height = CurView->Rect.bottom - CurView->DrawRect.bottom -2;
		symnum = GetDictSymbolNumber ("ARROW1");
		hSymbol = GetDictSymDesc (symnum,0);
		hOldBrush = SelectObject (CurView->hDC,GetStockObject(WHITE_BRUSH));
	    HaveVarFillColor = TRUE; 
	    GlobalColors[0]=RGB(255,255,255);
	    Point.x=(CurView->Rect.right + CurView->DrawRect.right)/2;
	    Point.y=(CurView->Rect.top + CurView->Rect.bottom)/2;  
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,NULL,FALSE,0,0,FALSE,FALSE);  
	    Point.y=(CurView->Rect.top + CurView->DrawRect.top)/2;  
		Rot = QuaterPY;					
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,NULL,FALSE,0,0,FALSE,FALSE);  
	    Point.x=(CurView->Rect.left + CurView->DrawRect.right)/2;
		Rot += QuaterPY;					
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,NULL,FALSE,0,0,FALSE,FALSE);  
	    Point.x=(CurView->Rect.left + CurView->DrawRect.left)/2;
		Rot += QuaterPY;					
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,NULL,FALSE,0,0,FALSE,FALSE);  
	    Point.y=(CurView->Rect.top + CurView->Rect.bottom)/2;  
		Rot += QuaterPY;					
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,NULL,FALSE,0,0,FALSE,FALSE);  
	    Point.y=(CurView->Rect.bottom + CurView->DrawRect.bottom)/2;
		Rot += QuaterPY;					
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,NULL,FALSE,0,0,FALSE,FALSE);  
	    Point.x=(CurView->Rect.left + CurView->Rect.right)/2;  
		Rot += QuaterPY;					
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,NULL,FALSE,0,0,FALSE,FALSE);  
	    Point.x=(CurView->Rect.right + CurView->DrawRect.right)/2;
		Rot += QuaterPY;					
		DisplayPointSymbol (hSymbol,CurView->hDC, -height, width,Rot, &Point,NULL,FALSE,0,0,FALSE,FALSE);  
		DestroySymbol (hSymbol); 
		HaveVarFillColor = SaveHVFC;
		SelectObject (CurView->hDC,hOldBrush);
    }
    else if (!Printing || From == 3)
    {   
    	DPOINT	Point1, Point2;
    	char	Dir; 
    	short	Deg1, Deg2, Min1, Min2, DegDif, MinDif, Format, Pass;
    	double	Rot, pct, Sec1, Sec2, SecDif, finc, MajorInc, MinorInc, MajorX, MinorX, LastX;
    	HANDLE	hMem=GSSiGlobAlloc (1011,GMEM_MOVEABLE,512*sizeof(double));
    	LPDOUBLE	XTics=(LPDOUBLE)GlobalLock (hMem);
    	LPDOUBLE	YTics=XTics+256;  
    	short	nXTics=0, nYTics=0, i, j;  
    	short	ticwidthfactor = IDNINT(DeviceToScreenFactor);
    	
    	if (DoMinorTics)
    		ticwidthfactor = 1;
    	
		LinePen = CreatePen (PS_SOLID,(int)IDNINT(DeviceToScreenFactor),0);
		RedPen = CreatePen (PS_SOLID,ticwidthfactor,RGB(0,0,0));
		WhitePen = CreatePen (PS_SOLID,ticwidthfactor*3,RGB(255,255,255));
		OldPen = SelectObject (CurView->hDC,LinePen);
	    SetBkMode(CurView->hDC, OPAQUE);  
	    SetBkColor (CurView->hDC,CurView->BackGroundColor); 
	    
	    Points[0].x = CurView->DrawRect.left;  
        Points[0].y = CurView->DrawRect.bottom;
	    Points[1].x = CurView->DrawRect.left;  
        Points[1].y = CurView->DrawRect.top;
	    Points[2].x = CurView->DrawRect.right;  
        Points[2].y = CurView->DrawRect.top;
	    Points[3].x = CurView->DrawRect.right;  
        Points[3].y = CurView->DrawRect.bottom;
	    Points[4]   = Points[0];
	    Polyline (CurView->hDC,Points,5);
        
    	w = (long)CurView->DrawRect.right - (long)CurView->DrawRect.left;
		h = ((long)CurView->Rect.bottom - (long)CurView->DrawRect.bottom)/2;// - 5*DeviceToScreenFactor;

    	Point1.x = CurView->NewBounds.xmn;
    	Point1.y = CurView->NewBounds.ymn;
    	Point2.x = CurView->NewBounds.xmx;
    	Point2.y = CurView->NewBounds.ymn;
    	Dir = 'W';
	    Points[0].y = CurView->DrawRect.bottom - 4 * DeviceToScreenFactor;
	    Points[1].y = ((long)CurView->DrawRect.bottom + (long)CurView->Rect.bottom)/2; 
	    MinorPoints[0].y = CurView->DrawRect.bottom - 1 * DeviceToScreenFactor;
	    MinorPoints[1].y = ((long)CurView->DrawRect.bottom + (long)CurView->Rect.bottom)/2 - 3 * DeviceToScreenFactor; 
	    Pass = 2;
    	
    	while (Pass--)
    	{
	    	ConvertCoord (&Point1,1,2);  
	    	GetDMS (Point1.x,&Deg1,&Min1,&Sec1);
	    	ConvertCoord (&Point2,1,2);
	    	GetDMS (Point2.x,&Deg2,&Min2,&Sec2); 
	    	GetDMSMajorMinorInc (fabs (Point1.x-Point2.x),&MajorInc,&MinorInc,&Format);
	    	finc = fmod (Point1.x,MajorInc); 
	    	MajorX = Point1.x-finc;
	    	GetDMS (MajorX,&DegDif,&MinDif,&SecDif); 
	    	MajorX -= MajorInc;
		    while (MajorX < Point2.x)
		    {
			    pct = (MajorX - Point1.x) / (Point2.x - Point1.x);
			    Points[0].x = CurView->DrawRect.left + w * pct; 
			    Points[1].x = Points[0].x;
		    	GetDMS (MajorX,&Deg1,&Min1,&Sec1);
		    	switch (Format)
		    	{
		    		case 0:
			    		sprintf (txt,"%c%iD",Dir,abs(Deg1));
		    			break;
		    		case 1:
			    		sprintf (txt,"%c%iD %2iM",Dir,abs(Deg1),Min1);
		    			break;
		    		case 2:
			    		sprintf (txt,"%c%iD %2iM %2.0fS",Dir,abs(Deg1),Min1,Sec1);
			    		break;
			    	case 3:
			    		sprintf (txt,"%c%iD %2iM %2.1fS",Dir,abs(Deg1),Min1,Sec1);
			    		break; 
			    }
			    if (Pass)
			    	XTics[nXTics++] = MajorX;
				SelectObject (CurView->hDC,WhitePen);
			    Polyline (CurView->hDC,Points,2);
				SelectObject (CurView->hDC,RedPen);
			    Polyline (CurView->hDC,Points,2);
				SelectObject (CurView->hDC,LinePen);
				MinorX = MajorX + MinorInc;
				LastX = MajorX;
				MajorX += MajorInc;
				while (MinorX < min (MajorX,Point2.x))
				{ 
				    if (Pass && DoMinorTics)
				    	XTics[nXTics++] = MinorX;
				    pct = (MinorX - Point1.x) / (Point2.x - Point1.x);
				    MinorPoints[0].x = CurView->DrawRect.left + w * pct; 
				    MinorPoints[1].x = MinorPoints[0].x;
				    Polyline (CurView->hDC,MinorPoints,2);
				    MinorX += MinorInc;
				} 
				if (LastX > Point1.x)
					DispText (CurView->hDC,Points[1].x-10,Points[1].x+10, Points[1].y,0, 2,2,
					 		  h,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,NULL,0,NULL,0,0,0);
			} 
		    Points[0].y = CurView->DrawRect.top + 4 * DeviceToScreenFactor;
		    Points[1].y = (CurView->DrawRect.top + CurView->Rect.top)/2; 
		    MinorPoints[0].y = CurView->DrawRect.top + 1 * DeviceToScreenFactor;
		    MinorPoints[1].y = (CurView->DrawRect.top + CurView->Rect.top)/2 + 3 * DeviceToScreenFactor; 
	    	Point1.x = CurView->NewBounds.xmn;
	    	Point1.y = CurView->NewBounds.ymx;
	    	Point2.x = CurView->NewBounds.xmx;
	    	Point2.y = CurView->NewBounds.ymx;
		}
		
    	w = (long)CurView->DrawRect.bottom - (long)CurView->DrawRect.top;
		h = ((long)CurView->Rect.right - (long)CurView->DrawRect.right)/2;// - 5*DeviceToScreenFactor;

    	Dir = 'N';    
    	Rot = HALFPI;
    	Point1.x = CurView->NewBounds.xmn;
    	Point1.y = CurView->NewBounds.ymn;
    	Point2.x = CurView->NewBounds.xmn;
    	Point2.y = CurView->NewBounds.ymx;
	    Points[0].x = CurView->DrawRect.left + 4 * DeviceToScreenFactor;
	    Points[1].x = (CurView->DrawRect.left + CurView->Rect.left)/2; 
	    MinorPoints[0].x = CurView->DrawRect.left + 1 * DeviceToScreenFactor;
	    MinorPoints[1].x = (CurView->DrawRect.left + CurView->Rect.left)/2 + 3 * DeviceToScreenFactor; 
	    Pass = 2;
    	
    	while (Pass--)
    	{
	    	ConvertCoord (&Point1,1,2);  
	    	GetDMS (Point1.y,&Deg1,&Min1,&Sec1);
	    	ConvertCoord (&Point2,1,2);
	    	GetDMS (Point2.y,&Deg2,&Min2,&Sec2); 
	    	GetDMSMajorMinorInc (fabs (Point1.y-Point2.y),&MajorInc,&MinorInc,&Format);
	    	finc = fmod (Point1.y,MajorInc); 
	    	MajorX = Point1.y-finc;
	    	GetDMS (MajorX,&DegDif,&MinDif,&SecDif); 
	    	MajorX -= MajorInc;
		    while (MajorX < Point2.y)
		    {
			    pct = (MajorX - Point1.y) / (Point2.y - Point1.y);
			    Points[0].y = CurView->DrawRect.bottom - w * pct; 
			    Points[1].y = Points[0].y;
		    	GetDMS (MajorX,&Deg1,&Min1,&Sec1);
		    	switch (Format)
		    	{
		    		case 0:
			    		sprintf (txt,"%c%iD",Dir,abs(Deg1));
		    			break;
		    		case 1:
			    		sprintf (txt,"%c%iD %2iM",Dir,abs(Deg1),Min1);
		    			break;
		    		case 2:
			    		sprintf (txt,"%c%iD %2iM %2.0fS",Dir,abs(Deg1),Min1,Sec1);
			    		break;
			    	case 3:
			    		sprintf (txt,"%c%iD %2iM %2.1fS",Dir,abs(Deg1),Min1,Sec1);
			    		break; 
			    }
			    if (Pass)
			    	YTics[nYTics++] = MajorX;
				SelectObject (CurView->hDC,WhitePen);
			    Polyline (CurView->hDC,Points,2);
				SelectObject (CurView->hDC,RedPen);
			    Polyline (CurView->hDC,Points,2);
				SelectObject (CurView->hDC,LinePen);
				MinorX = MajorX + MinorInc;
				LastX = MajorX;
				MajorX += MajorInc;
				while (MinorX < min (MajorX,Point2.y))
				{ 
				    if (Pass && DoMinorTics)
				    	YTics[nYTics++] = MinorX;
				    pct = (MinorX - Point1.y) / (Point2.y - Point1.y);
				    MinorPoints[0].y = CurView->DrawRect.bottom - w * pct; 
				    MinorPoints[1].y = MinorPoints[0].y;
				    Polyline (CurView->hDC,MinorPoints,2);
				    MinorX += MinorInc;
				} 
				if (LastX > Point1.y)
					DispText (CurView->hDC,Points[1].x-10,Points[1].x+10, Points[1].y, 0,2,2,
					 		  h,1,2, FALSE,Rot,txt,0,FALSE,0,0,-1,0,NULL,0,NULL,0,0,0);
			} 

		    Points[0].x = CurView->DrawRect.right - 4 * DeviceToScreenFactor;
		    Points[1].x = (CurView->DrawRect.right + CurView->Rect.right)/2; 
		    MinorPoints[0].x = CurView->DrawRect.right - 1 * DeviceToScreenFactor;
		    MinorPoints[1].x = (CurView->DrawRect.right + CurView->Rect.right)/2 - 3 * DeviceToScreenFactor; 
	    	Point1.x = CurView->NewBounds.xmx;
	    	Point1.y = CurView->NewBounds.ymn;
	    	Point2.x = CurView->NewBounds.xmx;
	    	Point2.y = CurView->NewBounds.ymx;
		}  
		
		for (i=0;i<nXTics;i++)
			for (j=0;j<nYTics;j++)
			{   
				DPOINT	TicPointD;
				POINT	TicPoint;
				
				TicPointD.x = XTics[i];
				TicPointD.y = YTics[j];
		    	ConvertCoord (&TicPointD,2,1);  
				TicPoint = BasePtToWinPt (&TicPointD);
				if (PtInRect (&CurView->DrawRect,TicPoint))
				{   
					Points[0] = Points[1] = TicPoint;
					Points[0].x -= 3*DeviceToScreenFactor;
					Points[1].x += 3*DeviceToScreenFactor;
					Points[2] = Points[3] = TicPoint;
					Points[2].y -= 3*DeviceToScreenFactor;
					Points[3].y += 3*DeviceToScreenFactor;
					SelectObject (CurView->hDC,WhitePen);
				    Polyline (CurView->hDC,Points,2);
				    Polyline (CurView->hDC,&Points[2],2);
					SelectObject (CurView->hDC,RedPen);
				    Polyline (CurView->hDC,Points,2); 
				    Polyline (CurView->hDC,&Points[2],2); 
				}
			}
			
		SelectObject (CurView->hDC,OldPen);
		DeleteObject (LinePen); 
		DeleteObject (RedPen); 
		DeleteObject (WhitePen); 
		GSSiGlobUlFree (&hMem);
	}
	RestoreDC (CurView->hDC,-1);
//	SetCurView (pViewports[CurViewID-1]); //tempdebu
    SetViewport (CurViewID);
	
{
#if ENABLETRACE
GSSiExitProg (1282);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
   
HANDLE	CreateListFile (LPSTR File,short lenListData,BOOL SortOnData)
#if ENABLETRACE
{GSSiEnterProg (1283);
#endif
{
 	int		i;
	BTVARDESC BTVar[2], *pVars;
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	long	TotFileLen;
	GWDHEADER GWDHead; 
	LPGWDHEADER	lpGWDHead;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo;
	HANDLE hBT, hDB;
	HFILE	FidData;
	int		ibeg,NumVars;
	OFSTRUCT	OFStruct;
	GWFLDINFO FldInfo;
	LPSTR	lpDot;  

	 lpGWDHead = &GWDHead; 
    _fmemset (lpGWDHead,0,sizeof(GWDHEADER));

	 FidData = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=1; 
	 if (SortOnData)
		 GWDHead.NumIndex=2; 
	 GWDHead.Version=1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 GWDHead.NumIndexFields[1]=1;
	 GWDHead.IndexFields[1][0]=4;
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER),-1);
	ibeg = 0;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"Refno");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"Prefix");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"UDI");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 32;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"Symbol");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = min(2,lenListData);
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"ListData");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = 0;
	 GSSillseek (FidData,0,0);
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER),-1);
 	 GSSillseek (FidData,0,2);
			     
	 NumVars = 1;
			
	 BTVar[0].BT_VARLEN=4;
	 BTVar[0].BT_VARTYP=BT_INTEGER;
	 BTVar[0].BT_VAROFF=0;
	 lpDot = _fstrrchr (File,'.');
	 _fstrcpy (lpDot,".in1");	
	 BT_CREATE (File, 4, FALSE, 1, 1,BTVar,FALSE, 0, 0, FALSE);
 	 GSSiClose (FidData);
	 _fstrcpy (lpDot,".gmd");	

     hDB = OpenGWDatabase (File,BT_WRITE);
{
#if ENABLETRACE
GSSiExitProg (1283);
#endif
     return hDB; 
}
#if ENABLETRACE
}
#endif
}  

BOOL AddToListFile (long Refno,LPSTREETTEXTDATA	pStreetData)
#if ENABLETRACE
{GSSiEnterProg (1284);
#endif
{   
	LPGWDHEADER	lpGWDHead;    
	LPLONG		pRefno;     
	char		str[256];
	
	if (!pStreetData->hListDB)
{
#if ENABLETRACE
GSSiExitProg (1284);
#endif
		return FALSE;   
}
	lpGWDHead = (LPGWDHEADER)GlobalLock (pStreetData->hListDB); 
	pRefno = (LPLONG)&lpGWDHead->GWDData;
	*pRefno = Refno;
	SetFieldValFromCharAndName(lpGWDHead,"Symbol","",FALSE);
	SetFieldValFromCharAndName(lpGWDHead,"Prefix",CurrentTAG,FALSE);
	SetFieldValFromCharAndName(lpGWDHead,"UDI",CurrentUDI,FALSE); 
	_fstrcpy (str,pStreetData->ListData);
	SetFieldValFromCharAndName(lpGWDHead,"ListData",str,FALSE);
	GWDAddRecord (lpGWDHead,0,NULL);      
	GlobalUnlock (pStreetData->hListDB); 
{
#if ENABLETRACE
GSSiExitProg (1284);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void DisplayCoordPrintText (void)
#if ENABLETRACE
{GSSiEnterProg (1285);
#endif
{   
	short	h,w;
	LPCOORDINATEDISPLAY	CD; 
	POINT	Points[3];  
	char	txt[256]; 
	
	CD = (LPCOORDINATEDISPLAY)CurTheme;
	if (!CD->PrintText)
{
#if ENABLETRACE
GSSiExitProg (1285);
#endif
		return;  
}
	_fstrcpy (txt,CD->PrintText);
	ExpandText (txt);
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn(FALSE,FALSE);
    SelectClipRgn (CurView->hDC,CurView->hRgn);
    GSSiDeleteObject(&CurView->hRgn);    
    SetTextColor (CurView->hDC,0);     
	FillRectPoly (CurView->hDC,&CurView->DrawRect,ConvertColor(CurView->BackGroundColor,-1));  
	h = (((CurView->DrawRect.bottom - CurView->DrawRect.top) / 2)-3*DeviceToScreenFactor);
	w = (h+4*DeviceToScreenFactor);
    SetBkMode(CurView->hDC, OPAQUE);  
    SetBkColor (CurView->hDC,CurView->BackGroundColor);
    Points[0].y = (CurView->DrawRect.bottom + CurView->DrawRect.top) / 2;
    Points[0].x = (CurView->DrawRect.left + CurView->DrawRect.right) / 2; 
	DispText (CurView->hDC,Points[0].x-10,Points[0].x+10, Points[0].y,0, 2,2,
			  h*2+6,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,NULL,0,NULL,0,0,0); 
	RestoreDC (CurView->hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (1285);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
  
void SBHash (HDC hDC,short nbars,short left,short top,long Width,long Height)
{
	POINT	Points[4];
	short	i; 
	HBRUSH	hBrush; 
	
	SaveDC (CurView->hDC);
	hBrush = SelectObject (hDC,GetStockObject(BLACK_BRUSH));
	
//	nbars *= DeviceToScreenFactor;
	Points[0].x = left;
	Points[0].y = top;
	Points[1].x = left;
	Points[1].y = top + Height;
	Points[2].x = left+Width;
	Points[2].y = top + Height;
	Points[3].x = left+Width;
	Points[3].y = top; 
	Polygon (hDC,Points,4);  
	if (hBrush)
		SelectObject (hDC,hBrush);
	RestoreDC (hDC,-1);
	return;
	Polyline (hDC,Points,2);
	Points[1].x = left + Width;
	Points[1].y = top;
	Polyline (hDC,Points,2);
	Points[1].x = left + Width;
	for (i=0;i<nbars;i++)
	{   
		Points[0].y = top + ((i+1) * Height)/nbars; 
		Points[1].y = Points[0].y;
		Polyline (hDC,Points,2);
	}
	Points[0] = Points[1];
	Points[1].y = top;
	Polyline (hDC,Points,2);
	return;
}

void GetDistDecimals (double Dist,LPSTR DistC)
{   
	short	l,n;   
	LPSTR	pEnd;
	
	sprintf (DistC,"%.2f",Dist);
	l = _fstrlen (DistC);
	if (l<7)
		return;
	pEnd = LastChr (DistC);
	n = min (2,l - 7); 
	if (n==1)
		n=2;
	pEnd -= n;
	*pEnd = 0;
	return;
}

void DisplayDistanceThemeLegend(short From,double ThisDist,double AZ,double TotDist)
#if ENABLETRACE
{GSSiEnterProg (1286);
#endif
{    
	short	CurViewID = CurView->ID, reps, h,w, Type;    
	double	BaseDistPerPixel, MaxWidth, BarWidth, step;
	POINT	Points[3];  
	long	IntVal,DisplayVPWidth;  
	char	txt[64], str[16], fmt[16]=" %.0f mile ";
	char	DistC[32], TotDistC[32], BearingC[32]; 
	char	DegC[8],MinC[8],SecC[16], PreDir[4], PostDir[4]; 
	HPEN	OldPen, LinePen;
    HFONT	hFont, hFontOld;
	int	ilog,RegionType;
	
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn(FALSE,FALSE);
    RegionType = SelectClipRgn (CurView->hDC,CurView->hRgn); 
    GSSiDeleteObject(&CurView->hRgn);    
    if (RegionType == NULLREGION)
    	goto Exit;
    SetTextColor (CurView->hDC,0);     
	FillRectPoly (CurView->hDC,&CurView->DrawRect,ConvertColor(CurView->BackGroundColor,-1));  
	h = (((CurView->DrawRect.bottom - CurView->DrawRect.top) / 2)-2*DeviceToScreenFactor);
	w = (h+4*DeviceToScreenFactor);
	if (From == 2)
	{    
		short	DistUnits = abs(CurTheme->ValConv);
		
		if (!DistUnits)
			DistUnits = OutDistUnits;
		h = min (16,0.9 * (CurView->DrawRect.bottom - CurView->DrawRect.top));
		Points[0] = RectMid (&CurView->DrawRect); 
		if (TotDist > 0)
		{
			GetDistDecimals (ConvertDist(ThisDist,DistUnits),DistC);
			GetDistDecimals (ConvertDist(TotDist+ThisDist,DistUnits),TotDistC);
			sprintf (txt,"%s (%s total) %s", DistC,TotDistC,DistUnitOpts[DistUnits-1]); 
		} 
		else if (TotDist < 0)
		{
			GetDistDecimals (ConvertDist(ThisDist,DistUnits),DistC);
			AZToBear (AZ,PreDir,DegC,MinC,SecC,PostDir); 
			sprintf (BearingC,"%s %s %s %s %s",PreDir,DegC,MinC,SecC,PostDir);
			sprintf (txt,"%s %s    %s", DistC,DistUnitOpts[DistUnits-1],BearingC); 
		}
		else
		{
			GetDistDecimals (ConvertDist(ThisDist,DistUnits),DistC);
			sprintf (txt,"%s %s", DistC,DistUnitOpts[DistUnits-1]); 
		}
	    SetBkMode(CurView->hDC, OPAQUE);  
	    SetBkColor (CurView->hDC,CurView->BackGroundColor);
		DispText (CurView->hDC,Points[0].x-10,Points[0].x+10, Points[0].y,0, 2,2,
				  h,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,NULL,0,NULL,0,0,0); 
		goto Exit;
	} 
	DisplayVPWidth = CurView->DrawRect.right - CurView->DrawRect.left;
    SetViewport (CurTheme->TargetViewport);  
    if (CurView->NewBounds.xmx < CurView->NewBounds.xmn)
    	goto Exit;
    if (Printing)
    	Type = 2;
    else
    	Type = CurTheme->ClassType;  
	switch (Type)
	{
		case 1:
		{   DPOINT	Point1,Point2; 
			double	BaseDist,WinDist;
		
			LinePen = CreatePen (PS_SOLID,(int)IDNINT(DeviceToScreenFactor),CurTheme->ScatterColor);
			OldPen = SelectObject (CurView->hDC,LinePen);
			Point1.x = CurView->NewBounds.xmn;
			Point1.y = CurView->NewBounds.ymn;
			Point2.x = CurView->NewBounds.xmx;
			Point2.y = CurView->NewBounds.ymx; 
			BaseDist = GetBaseDist (&Point1,&Point2);
			Point1.x = CurView->DrawRect.left;
			Point1.y = CurView->DrawRect.bottom;
			Point2.x = CurView->DrawRect.right;
			Point2.y = CurView->DrawRect.top; 
			WinDist = ldistp (Point1,Point2);
		    BaseDistPerPixel = BaseDist/WinDist;	
		    BaseDistPerPixel = ConvertDist (BaseDistPerPixel,CurTheme->ValConv);  
		    SetViewport (CurViewID);
		    MaxWidth = ((long)CurView->DrawRect.right - (long)CurView->DrawRect.left) * BaseDistPerPixel;
		    reps = 0;
		    if (MaxWidth > 1)            
		    {   
		    	step = 10.0;
		    	while (MaxWidth > 1)
		    	{
		    		MaxWidth /= 10.0;
		    		reps++;
		    	}
		    	IntVal = MaxWidth * 10;
			    reps--;
		    	if (IntVal > 1 || reps)
		    		_fstrcpy (fmt," %.0f %s ");
		    }
		    else
		    {   
		    	step = 0.1;    
		    	_fstrcpy (fmt," %.1f %s ");
		    	while (MaxWidth < 1)
		    	{
		    		MaxWidth *= 10.0;
		    		reps++;
		    	} 
		    	IntVal = MaxWidth; 
		    	itoa (reps,str,10);
		    	fmt[3] = *str;
		    }
		    MaxWidth = IntVal; 
		    while (reps--)
		    	MaxWidth *= step;
		    BarWidth = MaxWidth;
		    Points[0].y = (CurView->DrawRect.bottom + CurView->DrawRect.top) / 2;
		    Points[0].x = (CurView->DrawRect.left + CurView->DrawRect.right) / 2; 
		    Points[1].y = Points[0].y;
		    Points[1].x = Points[0].x + IDNINT(BarWidth/(2 * BaseDistPerPixel));
		    Polyline (CurView->hDC,Points,2);
		    Points[2] = Points[1];  
		    Points[2].x -= w;
		    Points[2].y -= h;
		    Polyline (CurView->hDC,&Points[1],2);
		    Points[2].y += h*2;   
		    Polyline (CurView->hDC,&Points[1],2);
		    Points[1].x = Points[0].x - IDNINT(BarWidth/(2 * BaseDistPerPixel));
		    Polyline (CurView->hDC,Points,2);
		    Points[2] = Points[1];  
		    Points[2].x += w;
		    Points[2].y -= h;
		    Polyline (CurView->hDC,&Points[1],2);
		    Points[2].y += h*2;   
		    Polyline (CurView->hDC,&Points[1],2); 
		    sprintf (txt,fmt,BarWidth,DistUnitOpts[max(0,CurTheme->ValConv-1)]);
			SelectObject (CurView->hDC,OldPen);
			DeleteObject (LinePen);
		    SetBkMode(CurView->hDC, OPAQUE);  
		    SetBkColor (CurView->hDC,CurView->BackGroundColor);
			DispText (CurView->hDC,Points[0].x-10,Points[0].x+10, Points[0].y,0, 2,2,
					  h*2+6,1,2, FALSE,0,txt,0,FALSE,0,0,-1,0,NULL,0,NULL,0,0,0); 
		}  
		break;
		default: 
		{   
			long	VPWidth = (long)CurView->DrawRect.right - (long)CurView->DrawRect.left;
		    double  BaseDistPerPixel = (CurView->NewBounds.xmx - CurView->NewBounds.xmn)/VPWidth;
		    double  BaseDistPerUnit  = -CurView->OrthoRes; 
		    double	MinorInc, MajorInc,ScaleDist=CurView->ScaleDist;  
		    long	BarHeight, BarWidth, Width, Height;
		    RECT	Bar;   
		    short	x=0,y=0, xmid, ymid, ScaleDistUnits=CurView->ScaleDistUnits;  
		    short	NumHash=5, NumMinorInc=5, StartMinorInc, NumMajorInc=3, i, j,Left, Right,Top, startinc=0;
		    short	sign=1, UpInc, LowInc;  
		    long	Dist1=1, Dist2=5, DistFactor=1;
		    short	NumDec;
		    
		    if (!DevicePixelsPerInch)
		    	break;
 			if (!CurView->WindowZoomedToOrtho || CurView->OrthoRes >= 0)
 			{   
 				double	Min, Max;
 				
 				if (CurTheme->ValConv <= 0)
 				{ 
 					if (ConvertDist (DisplayVPWidth * BaseDistPerPixel,4) < GetGlobalDVal2 ("[%SWITCHTOMILESAT]",1.0))
 						CurTheme->ValConv = -1;
 					else
 						CurTheme->ValConv = -4;
 				}
 				ScaleDistUnits = abs (CurTheme->ValConv);  
 				ScaleDist = ConvertDist (BaseDistPerPixel * DevicePixelsPerInch,ScaleDistUnits);
 				ilog = log10 (ScaleDist);
 				Min = pow (10,ilog);
 				Max = pow (10,ilog+1);
 				if (ScaleDist < Max/2)
 				{
 					UpInc = 5;
 					LowInc = 10;
 					Max /= 2;
 				}
 				else 
 				{
 					UpInc = 10;
 					LowInc = 5;
 					Min = Max / 2;
 				} 
 				if (ScaleDist - Min < Max - ScaleDist)  
 				{
 					NumMinorInc = LowInc;
 					ScaleDist = Min;     
 				}
 				else
 				{
 					NumMinorInc = UpInc;
 					ScaleDist = Max;
 				}
 				BaseDistPerUnit = ConvertInDist (ScaleDist,ScaleDistUnits);
 			} 
 			else
 			{   
 				ScaleDistUnits++;
				i = 1; 
				StartMinorInc = NumMinorInc;
				while (fmod (log10 (ScaleDist/NumMinorInc),1) && i < 30)
				{   
					i++;
					j = i/2;
					NumMinorInc = max (StartMinorInc+ sign*j,3); 
					sign = -1 * sign; 
				}
			}
 			SetBkMode(CurView->hDC, TRANSPARENT);  
		    SetViewport (CurViewID);   
			VPWidth = CurView->DrawRect.right - CurView->DrawRect.left;
			MinorInc = BaseDistPerUnit / NumMinorInc;
			MajorInc = BaseDistPerUnit;
			BarHeight = (CurView->DrawRect.bottom - CurView->DrawRect.top)/3; 
			NumMajorInc = 1;
			BarWidth = IDNINT ((NumMajorInc + 1) * MajorInc / BaseDistPerPixel);   
			while (BarWidth > VPWidth)
			{   
				ScaleDist /= 10;
				MajorInc /= 10;
				MinorInc /= 10;
				BarWidth = IDNINT ((NumMajorInc + 1) * MajorInc / BaseDistPerPixel);   
			}
			ilog = log10 (ScaleDist);
			if (ScaleDist < 1)
				NumDec = abs (ilog);
			else
				NumDec = 0;
			while (BarWidth < VPWidth -  1.5 * IDNINT (MajorInc / BaseDistPerPixel))
			{
				NumMajorInc++;
				BarWidth = IDNINT ((NumMajorInc + 1) * MajorInc / BaseDistPerPixel);
			}	
			if (NumMajorInc > 10 && NumMajorInc%2)
				NumMajorInc--;
			BarWidth = IDNINT ((NumMajorInc + 1) * MajorInc / BaseDistPerPixel);
			Bar.left = CurView->DrawRect.left + (CurView->DrawRect.right - CurView->DrawRect.left)/2 - BarWidth/2;
			Bar.right = Bar.left + BarWidth; 
			Bar.bottom = CurView->DrawRect.bottom - BarHeight; 
			Bar.top = Bar.bottom - BarHeight;    
			xmid = Bar.left + (Bar.right - Bar.left) / 2;    
			ymid = Bar.top  + (Bar.bottom - Bar.top) / 2;
			LinePen = CreatePen (PS_SOLID,(int)IDNINT(DeviceToScreenFactor),CurTheme->ScatterColor);
			OldPen = SelectObject (CurView->hDC,LinePen);
		    Points[0].y = Bar.bottom;
		    Points[0].x = Bar.left; 
		    Points[1].y = Bar.bottom;
		    Points[1].x = Bar.right; 
		    Polyline (CurView->hDC,Points,2);  
		    Points[0].y -= BarHeight;
		    Points[1].y = Points[0].y;
		    Polyline (CurView->hDC,Points,2);  
		    Points[1].y += BarHeight;
		    Points[1].x = Points[0].x;
		    Polyline (CurView->hDC,Points,2);  
		    Points[0].x = Points[1].x = Bar.right;
		    Polyline (CurView->hDC,Points,2);  
	    	Width = IDNINT (MinorInc/BaseDistPerPixel); 
	    	Height = BarHeight/2;
		    for (i=0;i<NumMinorInc;i++)
		    {   
		    	j = i;
		    	Left = IDNINT (Bar.left + (i*MinorInc) / BaseDistPerPixel);
		    	Right = IDNINT (Bar.left + ((i+1)*MinorInc) / BaseDistPerPixel);
		    	Top  = Bar.top + j%2 * Height;
			    SBHash (CurView->hDC,NumHash,Left,Top,Right-Left,Height);
		    }
		    
	    	Width = IDNINT (MajorInc/BaseDistPerPixel); 
			CurTheme->TitleFont.lfHeight = -BarHeight * 0.70;
			hFont = CreateFontIndirect((PLOGFONT)&CurTheme->TitleFont);
			hFontOld = SelectObject(CurView->hDC, hFont);    
			SetTextAlign (CurView->hDC,TA_CENTER|TA_BOTTOM);  
			y = Bar.top;
		    x = Bar.left; 
		    if (startinc)
		    	_fstrcpy (txt,"0");
			else	
			    RWRITE (ScaleDist,NumDec, txt); 
//				ltoa (IDNINT(ScaleDist),txt,10);
			ExtTextOut (CurView->hDC,x,y,0,NULL,txt,_fstrlen(txt),NULL);
		    for (i=0;i<NumMajorInc;i++)
		    {    
		    	j++;
		    	Left = IDNINT (Bar.left + ((i+1)*MajorInc) / BaseDistPerPixel);
		    	Right = IDNINT (Bar.left + ((i+2)*MajorInc) / BaseDistPerPixel);
		    	Top  = Bar.top + j%2 * Height;
			    SBHash (CurView->hDC,NumHash,Left,Top,Right-Left,Height);
			    RWRITE ((i+startinc) * ScaleDist,NumDec, txt); 
//				ltoa ((i+startinc) * IDNINT(ScaleDist),txt,10);
				x = Left;
				if (NumMajorInc < 10 || i < 2 || (i-1)%2)
					ExtTextOut (CurView->hDC,x,y,0,NULL,txt,_fstrlen(txt),NULL);
		    }
		    x = Bar.right; 
		    RWRITE ((i+startinc) * ScaleDist,NumDec, txt); 
//			ltoa ((i+startinc) * IDNINT(ScaleDist),txt,10);
			ExtTextOut (CurView->hDC,x,y,0,NULL,txt,_fstrlen(txt),NULL);
			SelectObject (CurView->hDC,OldPen);
			DeleteObject (LinePen);
	
			_fstrcpy (txt,DistUnitOpts[ScaleDistUnits-1]); 
			_fstrlwr (&txt[1]);  
			x = xmid;
			y = Bar.bottom;
			SetTextAlign (CurView->hDC,TA_CENTER|TA_TOP);
			ExtTextOut (CurView->hDC,x,y,0,NULL,txt,_fstrlen(txt),NULL); 
			SelectObject(CurView->hDC, hFontOld);
			GSSiDeleteObject (&hFont);
		}
		break;
	} 
Exit:
    SetViewport (CurViewID);   
	RestoreDC (CurView->hDC,-1);
	
{
#if ENABLETRACE
GSSiExitProg (1286);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

HANDLE ExpandThemeValues (HANDLE hBT)
#if ENABLETRACE
{GSSiEnterProg (1289);
#endif
{   
	HANDLE	hBTNew;       
	char	Value[256], NewValue[130];
	short	iclass, pos=BT_FIRST;
	
	hBTNew = CreateUniqueList (128,NULL);
    
    while (!BT_FIND (hBT,Value,pos,BT_ANY,(LPSTR)&iclass))
    {
    	pos = BT_NEXT;  
    	ExpandText (Value);  
    	_fstrncpy (NewValue,Value,128);
    	BT_PUT (hBTNew,NewValue,(LPSTR)&iclass);
    } 
    BT_CLOSE (hBT);
{
#if ENABLETRACE
GSSiExitProg (1289);
#endif
	return hBTNew;
}
#if ENABLETRACE
}
#endif
}

COLORREF GetNextUniqueColor (LPLONG pColor)
{
	POINT	p=RectMid (&CurView->DrawRect);
	long	LastColor = *pColor;
	long	Color, ii;
	
	(*pColor)+=UniqueColorInc; 
	if ((long)SetPixel (CurView->hDC,p.x,p.y,*pColor) < 0)
		return *pColor;
	while ((Color=GetPixel (CurView->hDC,p.x,p.y)) != *pColor && *pColor > LastColor)
	{
		LastColor = *pColor;
		(*pColor)++; 
		ii=SetPixel (CurView->hDC,p.x,p.y,*pColor);
	}
	return *pColor;  
}

BOOL BeginThemePCTByArea (void)
#if ENABLETRACE
{GSSiEnterProg (1290);
#endif
{   
	UINT	iclass;    
	long	NextColor;
	
	if (!CurTheme)
{
#if ENABLETRACE
GSSiExitProg (1290);
#endif
		return FALSE;
}
	if (!CurTheme->PCTByArea)
{
#if ENABLETRACE
GSSiExitProg (1290);
#endif
		return FALSE;
}
//	if (Printing || CurTheme->PCTDisplayCycle == DisplayCycle)
	if (CurTheme->PCTDisplayCycle == DisplayCycle)
{
#if ENABLETRACE
GSSiExitProg (1290);
#endif
		return FALSE; 
}
	if (!Printing)
		CurTheme->NumNonMask = 0;  
	SaveDC (CurView->hDC);
    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
  	SelectClipRgn (CurView->hDC,NULL);
	NextColor = 31; 
	ComputePCTMaskColor = GetNextUniqueColor (&NextColor);
	for (iclass=0;iclass<MAX_THEME_CLASSES;iclass++) 
	{   
		if (!SaveColorsInUse)
			SaveClassColor[iclass] = CurTheme->ClassColor[iclass];
		CurTheme->ClassColor[iclass] = GetNextUniqueColor (&NextColor); 
		if (!Printing)
			CurTheme->ClassCount[iclass] = 0;                      
	}
	SaveColorsInUse = TRUE;
	RestoreDC (CurView->hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (1290);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL ComputeThemePCTByArea (BOOL CloseAll)
#if ENABLETRACE
{GSSiEnterProg (1291);
#endif
{   
	COLORREF	Color;   
	UINT	x,y,iclass;
   	HCURSOR	hcurSave; 
	 
	if (!CurTheme)
{
#if ENABLETRACE
GSSiExitProg (1291);
#endif
		return FALSE;
}
	if (!CurTheme->PCTByArea)
{
#if ENABLETRACE
GSSiExitProg (1291);
#endif
		return FALSE;
}
	if (CloseAll)
	{
		ComputePCTTheme=0;
{
#if ENABLETRACE
GSSiExitProg (1291);
#endif
		return FALSE;           
}
	}
//	if (Printing || CurTheme->PCTDisplayCycle == DisplayCycle)
	if (CurTheme->PCTDisplayCycle == DisplayCycle)
{
#if ENABLETRACE
GSSiExitProg (1291);
#endif
		return FALSE;
}
	
	CurTheme->PCTDisplayCycle = DisplayCycle;
	
	if (!Printing)
	{   
		DisplayPolyOff ();
		if (!CurView->hMaskArea)
		{   
			MaskRect = CurView->DrawRect;
		}
		else
	        DisplayMaskArea(); 
		if (!PrintMsgWnd)
		{	
			hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
		}
	    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
		y = MaskRect.top;
		while (y <= MaskRect.bottom)
		{
			x = MaskRect.left;
			while (x <= MaskRect.right)
			{   
				Color = GetPixel (CurView->hDC,x,y);
				if (Color != ComputePCTMaskColor)
				{
					CurTheme->NumNonMask++;
					for (iclass=0;iclass<CurTheme->NumClass;iclass++)
					{
						if (Color == CurTheme->ClassColor[iclass])
						{   
							CurTheme->ClassCount[iclass]++;
							break;
						}
					}
				}				
				x++;
			}
			y++;
		}
	}
	for (iclass=0;iclass<MAX_THEME_CLASSES;iclass++) 
		CurTheme->ClassColor[iclass] = SaveClassColor[iclass]; 
	SaveColorsInUse = FALSE; 
	if (!PrintMsgWnd)
	{
		GSSiSetCursor(hcurSave);
	}	
{
#if ENABLETRACE
GSSiExitProg (1291);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

void CompareViewportsThemeLegend (short From,short FromVPID)	  
{
	RECT	Rect=CurView->DrawRect; 
    short	BorderWidth=0;
    
    SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
  	SelectClipRgn (CurView->hDC,NULL); 
  	if (CurView->BorderPct > 0)
		BorderWidth = -IDNINT ((double)((MaxDimension * 2 * CurView->BorderPct)/100));
	InflateRect (&Rect,BorderWidth,BorderWidth);   
    FillRectPoly (CurView->hDC,&Rect,CurView->BackGroundColor);
    switch (From)
    {   
    	case 0:
    		CurTheme->HaveVP[0] = FALSE;
    		CurTheme->HaveVP[1] = FALSE;
    		goto Exit; 
    	case 1:
    		if (FromVPID == CurTheme->TargetViewport)
    			CurTheme->HaveVP[0] = TRUE;
    		else if (FromVPID == CurTheme->DataType)
    			CurTheme->HaveVP[1] = TRUE; 
    		goto Exit;
    	case 2: 
    	case 3:
    		goto Exit;
    	case 4: 
    		if (CurTheme->HaveVP[0] && CurTheme->HaveVP[1])
    		{
				RECT	Rect1=pViewports[CurTheme->TargetViewport-1]->DrawRect;
				RECT	Rect2=pViewports[CurTheme->DataType-1]->DrawRect;
				RECT	Rect3=pViewports[CurTheme->DisplayViewport-1]->DrawRect;
				UINT	x1,x2,x3,y1,y2,y3, w=Rect1.right - Rect1.left +1, h=Rect1.bottom - Rect1.top + 1;
				COLORREF	DeleteColor=RGB(255,0,0), AddColor=RGB(0,0,255), ChangeColor=0, Color1, Color2, BkColor;
				
				BkColor = CurView->BackGroundColor;
				y1 = Rect1.top;
				y2 = Rect2.top;
				y3 = Rect3.top;
				while (y1 <= Rect1.bottom)
				{
					x1 = Rect1.left; 
					x2 = Rect2.left; 
					x3 = Rect3.left; 
					while (x1 <= Rect1.right)
					{   
						Color1 = GetPixel (CurView->hDC,x1,y1);
						Color2 = GetPixel (CurView->hDC,x2,y2);
						if (Color1 != Color2)
						{
							if (Color1 == BkColor)
								SetPixel (CurView->hDC,x3,y3,DeleteColor);
							else if (Color2 == BkColor)
								SetPixel (CurView->hDC,x3,y3,AddColor); 
							else
								SetPixel (CurView->hDC,x3,y3,ChangeColor); 
						}
						x1++;
						x2++;
						x3++;
					}
					y1++; 
					y2++;
					y3++;
				}
			    			
    		}
    		goto Exit; 
    } 
Exit:
	RestoreDC (CurView->hDC,-1);
	return;
}
   
void Display2DThemeLegend (short From)
#if ENABLETRACE
{GSSiEnterProg (1292);
#endif
{   int		xmargin, ymargin;
	long	h, w;
	int		width, x, y, fHeight, MaxTextWidth, twidth, i, j;
	HBRUSH	BkBrush;
	int		iclass;
	RECT	ClassColorBox;
	HFONT	hfont, hfontOld=0, hfont2;
	DWORD	TextExtent;
	char	Text[256], Title[256];
	char	Val1[32], Val2[32];
	long	TotCount;
	float	Pct;
	int		MinFontHeight=2, left,right, tWidth;
	int		inc;
	LPSTR	lpText;
	char	lpLine[256];  
	long	Counts[MAX_THEME_CLASSES][MAX_THEME_CLASSES], nMax;
	HANDLE	hBTX, hBTY;
	LPTHEME	pThemeX, pThemeY, SaveTheme=CurTheme;
	short	nClassX, nClassY, VPNumX=CurTheme->DataFileType, VPNumY=CurTheme->DataType;
	short	pos=BT_FIRST, cval; 
	COLORREF	Color;
	THEMEHIGHLIGHTKEY	ThemeHighlightKeyX,ThemeHighlightKeyY;
	THEMEHIGHLIGHTDATA	ThemeHighlightData;    
	double	xinc, yinc;
	RECT	Rect=CurView->DrawRect;  
	HCURSOR	hcurSave;    
	BOOL	Opened1=FALSE, Opened2=FALSE;
    
    SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn (FALSE,FALSE);
    SelectClipRgn (CurView->hDC,CurView->hRgn);
    GSSiDeleteObject(&CurView->hRgn); 
	InflateRect (&Rect,1,1);
    FillRectPoly (CurView->hDC,&Rect,WindowColor);
    if (From < 2)
    	goto Exit;
    pThemeX = pViewports[VPNumX]->pTheme;
    pThemeY = pViewports[VPNumY]->pTheme;
    if (!pThemeX || !pThemeY)
    	goto Exit;  
    CurTheme = pThemeX; 
    if (!CurTheme->hHighlightFile)
    {
		if (!OpenThemeHighlightFile (BT_READ))
		{
			CurTheme = SaveTheme;
			goto Exit;
		} 
		Opened1 = TRUE;
	}
    hBTX = CurTheme->hHighlightFile;
    CurTheme = pThemeY;
    if (!CurTheme->hHighlightFile)
    {
		if (!OpenThemeHighlightFile (BT_READ))
		{   
		    CurTheme = pThemeX;
			CloseThemeHighlightFile ();
			CurTheme = SaveTheme;
			goto Exit;
		}
		Opened2 = TRUE;
	}
    hBTY = CurTheme->hHighlightFile;
	CurTheme = SaveTheme;
    nClassX = pThemeX->NumClass;
    nClassY = pThemeY->NumClass; 
    if (!hBTX || !hBTY || !nClassX || !nClassY)
    	goto Exit; 
	hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT)); 
    _fmemset (Counts,0,MAX_THEME_CLASSES*MAX_THEME_CLASSES*sizeof(long));
	while (!BT_FIND (hBTX,(LPSTR)&ThemeHighlightKeyX,pos,BT_ANY,(LPSTR)&ThemeHighlightData))
    {   
    	pos = BT_NEXT; 
    	if (ThemeHighlightKeyX.Class > -1)  
    	{
    		for (iclass=0;iclass<nClassY;iclass++)
    		{   
    			ThemeHighlightKeyY.Class = iclass;
    			ThemeHighlightKeyY.Refno = ThemeHighlightKeyX.Refno;
    			if (!BT_FIND (hBTY,(LPSTR)&ThemeHighlightKeyY,BT_FIRST,BT_EQ,(LPSTR)&ThemeHighlightData)) 
    			{ 
    				Counts[ThemeHighlightKeyX.Class][ThemeHighlightKeyY.Class]++;
    				continue;
    			}
    		}
    	}
    }  
	GSSiSetCursor(hcurSave); 
	if (Opened1)
	{
	    CurTheme = pThemeX;
		CloseThemeHighlightFile (); 
	}
	if (Opened2)
	{
	    CurTheme = pThemeY;
		CloseThemeHighlightFile (); 
	}
	CurTheme = SaveTheme;
    
    _fstrcpy (Title,CurTheme->Title);
    ExpandText (Title);
    
	CurTheme->Rect=PctRect (CurView->DrawRect,-CurTheme->Margin);
	xmargin = (((long)CurTheme->Rect.right - CurTheme->Rect.left) * CurTheme->InnerMargin) / 100;
	ymargin = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->InnerMargin) / 100;
	h = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->TitleHeight) / 100;
	CurTheme->TitleBox.top = CurTheme->Rect.top + ymargin;
	CurTheme->TitleBox.bottom = CurTheme->TitleBox.top + h;
	CurTheme->TitleBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->TitleBox.right = CurTheme->Rect.right - xmargin;
	CurTheme->InfoBox.top = CurTheme->TitleBox.bottom + ymargin;
	CurTheme->InfoBox.bottom = CurTheme->Rect.bottom - ymargin;
	CurTheme->InfoBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->InfoBox.right = CurTheme->Rect.right - ymargin;
	CurTheme->ScatterBox = CurTheme->InfoBox;
	CurTheme->ScatterBox.right = CurTheme->InfoBox.right;

	FillRectPoly (CurView->hDC,&CurTheme->Rect,CurTheme->BGColor);
	FillRectPoly (CurView->hDC,&CurTheme->InfoBox,RGB(255,255,255));
	FillRectPoly (CurView->hDC,&CurTheme->TitleBox,CurTheme->TitleBoxBG);

	width = CurTheme->TitleBox.right - CurTheme->TitleBox.left;
	fHeight = CurTheme->TitleHeight+1;
	twidth = INT_MAX;
	while (twidth>width && fHeight>2)
	{   
		fHeight--;
		CurTheme->TitleFont.lfHeight = -MulDiv(fHeight,
											   GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
		hfont = CreateFontIndirect((PLOGFONT)&CurTheme->TitleFont);
		hfontOld = SelectObject(CurView->hDC, hfont);
		TextExtent = GetTextExtent (CurView->hDC,Title,_fstrlen(Title));
		lpText = Title;
		twidth = 0;	
	    while (NextLine (&lpText,lpLine,0))
	    {
			TextExtent = GetTextExtent (CurView->hDC,lpLine,_fstrlen(lpLine));
			twidth = max (twidth,LOWORD (TextExtent));
		}  
		SelectObject(CurView->hDC, hfontOld); 
		DeleteObject(hfont);
	}
	x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - twidth)/2;
	CurTheme->TitleFont.lfHeight = -MulDiv(CurTheme->TitleHeight-2,
										   GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
	hfont = CreateFontIndirect((PLOGFONT)&CurTheme->TitleFont);
	hfontOld = SelectObject(CurView->hDC, hfont); 

    SetTextColor (CurView->hDC,0); 
	lpText = Title;	
	y = CurTheme->TitleBox.top + 1;
    while (NextLine (&lpText,lpLine,0))
    {
		TextExtent = GetTextExtent (CurView->hDC,lpLine,_fstrlen(lpLine));
		width = LOWORD (TextExtent);
		x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - width)/2;
		TextOut(CurView->hDC, x, y, lpLine,_fstrlen(lpLine));  
		y+= HIWORD (TextExtent);
	}
	SelectObject(CurView->hDC, hfontOld); 
	DeleteObject(hfont);
    
    nMax = 0;
    for (i=0;i<nClassX;i++)
    	for (j=0;j<nClassY;j++)
    		nMax = max (nMax,Counts[i][j]); 
    if (!nMax)
    	goto Exit;
	xinc = ((double)CurTheme->InfoBox.right - CurTheme->InfoBox.left)/nClassX;
	yinc = ((double)CurTheme->InfoBox.bottom - CurTheme->InfoBox.top)/nClassY;
    for (i=0;i<nClassX;i++)
    {
    	for (j=0;j<nClassY;j++)
    	{
			ClassColorBox.left = IDNINT (CurTheme->InfoBox.left + xinc * i);
			ClassColorBox.right = IDNINT (CurTheme->InfoBox.left + xinc * (i+1));
			ClassColorBox.bottom = IDNINT (CurTheme->InfoBox.bottom - yinc * j);
			ClassColorBox.top = IDNINT (CurTheme->InfoBox.bottom - yinc * (j+1)); 
			cval = ((nMax-Counts[i][j]) * 255) / nMax;
			Color = RGB(cval,cval,cval); 
			FillRectPoly (CurView->hDC,&ClassColorBox,Color); 
			if (CurTheme->DisplayCount)
			{
			    SetBkMode(CurView->hDC, TRANSPARENT);  
				CurTheme->ClassFont2.lfHeight = -(ClassColorBox.bottom - ClassColorBox.top - 2* DeviceToScreenFactor); 
				hfont2 = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont2);
		        hfontOld = SelectObject(CurView->hDC, hfont2);
				sprintf (Text,"%ld",Counts[i][j]); 
				TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
				width = LOWORD (TextExtent);
				x = ClassColorBox.left + ((ClassColorBox.right - ClassColorBox.left) - width)/2; 
				y = ClassColorBox.top + 1* DeviceToScreenFactor; 
				if (cval >150)
				    SetTextColor (CurView->hDC,0); 
				else
				    SetTextColor (CurView->hDC,RGB(255,255,255));  
				TextOut(CurView->hDC, x, y, Text,_fstrlen(Text)); 
				SelectObject(CurView->hDC, hfontOld); 
				DeleteObject(hfont2);
			}
    	}
    }
    SetTextColor (CurView->hDC,0); 
	fHeight = 12 * DeviceToScreenFactor; 
	CurTheme->ClassFont1.lfHeight = fHeight;
	hfont = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont1);
	hfontOld = SelectObject(CurView->hDC, hfont);
	y = ClassColorBox.top - 2*yinc/3 - 2;
    for (i=0;i<nClassX;i++)
    {   
    	LPSTR	pPar;
    	 
    	_fstrcpy (Text,pThemeX->ClassBM[i]);
    	if ((pPar=_fstrrchr (Text,'(')))
    		*pPar = 0;
		left = IDNINT (CurTheme->InfoBox.left + xinc * i);
		right = IDNINT (CurTheme->InfoBox.left + xinc * (i+1));
       	TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
       	tWidth = LOWORD (TextExtent); 
       	x = (left + right) / 2 - tWidth/2;
		TextOut(CurView->hDC, x,y,Text,_fstrlen(Text));
    }
	SelectObject(CurView->hDC, hfontOld);
	DeleteObject(hfont); 
Exit:
	RestoreDC (CurView->hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (1292);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

BOOL FAR PASCAL DISPLAY2DMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1293);
#endif
{	HWND	hCheckBox;
	RECT	rect;
	HDC		hDC;
	char	str[256], names[84], *ptr;
	int	Choice,i;

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1293);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:  
    	SetDlgItemText(hWndDlg,SV_TITLE,(LPSTR)&CurTheme->Title);
        i=SendDlgItemMessage (hWndDlg,SV_TARGET,CB_RESETCONTENT,NULL,NULL);
		for (i=0;i<*pNumViewports;i++)
		{
			SendDlgItemMessage (hWndDlg,SV_VP_NAME1,CB_ADDSTRING,NULL,(LPARAM)pViewports[i]->Name);
			SendDlgItemMessage (hWndDlg,SV_VP_NAME2,CB_ADDSTRING,NULL,(LPARAM)pViewports[i]->Name);
		}
		SendDlgItemMessage (hWndDlg,SV_VP_NAME1,CB_SETCURSEL,CurTheme->DataFileType,NULL);
		SendDlgItemMessage (hWndDlg,SV_VP_NAME2,CB_SETCURSEL,CurTheme->DataType,NULL);
       	SendDlgItemMessage (hWndDlg,SV_DISPLAY_COUNT,BM_SETCHECK,CurTheme->DisplayCount,0L);
       	SendDlgItemMessage (hWndDlg,IDC_SETCOLOR,BM_SETCHECK,(!CurTheme->NotSetColor),0L);   

		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 CloseThemeDataFile(TRUE);
                 EndDialog(hWndDlg, FALSE);
                 break;
            case IDC_SAVE_THEME:  
            case IDOK:
       	 		 GetDlgItemText (hWndDlg,SV_TITLE,CurTheme->Title,256);
                 CurTheme->DataFileType=SendDlgItemMessage(hWndDlg,SV_VP_NAME1, CB_GETCURSEL,NULL,NULL);
                 CurTheme->DataType=SendDlgItemMessage(hWndDlg,SV_VP_NAME2, CB_GETCURSEL,NULL,NULL);
            	 CurTheme->DisplayCount = SendDlgItemMessage (hWndDlg,SV_DISPLAY_COUNT,BM_GETCHECK,0,0L);  
            	 CurTheme->NotSetColor = !SendDlgItemMessage (hWndDlg,IDC_SETCOLOR,BM_GETCHECK,0,0L);  
            	 if (wParam == IDC_SAVE_THEME)
            	 	SaveCurTheme(hWndDlg);
            	 else
                 	EndDialog(hWndDlg, TRUE);
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1293);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1293);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL COMPARE_VIEWPORTSMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1293);
#endif
{	HWND	hCheckBox;
	RECT	rect;
	HDC		hDC;
	char	str[256], names[84], *ptr;
	int	Choice,i;

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1293);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:  
		for (i=0;i<*pNumViewports;i++)
		{
			SendDlgItemMessage (hWndDlg,SV_TARGET1,CB_ADDSTRING,NULL,(LPARAM)pViewports[i]->Name);
			SendDlgItemMessage (hWndDlg,SV_TARGET2,CB_ADDSTRING,NULL,(LPARAM)pViewports[i]->Name);
		}
		SendDlgItemMessage (hWndDlg,SV_TARGET1,CB_SETCURSEL,CurTheme->TargetViewport-1,NULL);
		SendDlgItemMessage (hWndDlg,SV_TARGET2,CB_SETCURSEL,CurTheme->DataType-1,NULL);

		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
            case IDC_SAVE_THEME:  
            case IDOK:
       	 		 GetDlgItemText (hWndDlg,SV_TITLE,CurTheme->Title,256);
                 CurTheme->TargetViewport=SendDlgItemMessage(hWndDlg,SV_TARGET1, CB_GETCURSEL,NULL,NULL)+1;
                 CurTheme->DataType=SendDlgItemMessage(hWndDlg,SV_TARGET2, CB_GETCURSEL,NULL,NULL)+1;
            	 if (wParam == IDC_SAVE_THEME)
            	 	SaveCurTheme(hWndDlg);
            	 else
                 	EndDialog(hWndDlg, TRUE);
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1293);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1293);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL ThemeCommonCode (HWND hWndDlg, WORD Message, WORD wParam, LONG lParam, HANDLE hSQL)
#if ENABLETRACE
{GSSiEnterProg (1294);
#endif
{
	int	idesc, i, n;
	LPVIEWPORT	SaveView;
	COLORREF	Color;  
	char		Contents[36], str[32];
	
 switch(Message)
   { 
   
//    case GSSI_REINITDIALOG: 12/6/99 causing contents to reset in streetnametheme
    case WM_INITDIALOG: 
    	if (CurTheme->SymNum > 0)
    	{   
       		LPVIEWPORT	SaveView=CurView;
       		
       		if (CurTheme->TargetViewport && CurTheme->ID != GF_BOUNDS_DISPLAY_THEME)
	    		SetCurView (pViewports[CurTheme->TargetViewport-1]);
			GetSymbolName (CurTheme->SymNum,Contents,NULL,0,NULL);
			SetCurView (SaveView);
   			SetDlgItemText(hWndDlg,SV_CONTENTS_LIST,Contents);
    	}
    	else 
   			SetDlgItemText(hWndDlg,SV_CONTENTS_LIST,CurTheme->Contents);
case GSSI_REINITDIALOG:
        SendDlgItemMessage (hWndDlg,SV_COLOR_SCHEME,LB_RESETCONTENT,NULL,NULL);
		SendDlgItemMessage (hWndDlg,SV_COLOR_SCHEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"User Defined"));
		SendDlgItemMessage (hWndDlg,SV_COLOR_SCHEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Greys"));
		SendDlgItemMessage (hWndDlg,SV_COLOR_SCHEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Reds"));
		SendDlgItemMessage (hWndDlg,SV_COLOR_SCHEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Greens"));
		SendDlgItemMessage (hWndDlg,SV_COLOR_SCHEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Blues"));
		SendDlgItemMessage (hWndDlg,SV_COLOR_SCHEME,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Mixed"));
        i=SendDlgItemMessage (hWndDlg,SV_TARGET,CB_RESETCONTENT,NULL,NULL);
		for (i=0;i<*pNumViewports;i++)
		{
			SendDlgItemMessage (hWndDlg,SV_TARGET,CB_ADDSTRING,NULL,
								(LPARAM)pViewports[i]->Name);
		}
		SendDlgItemMessage (hWndDlg,SV_TARGET,CB_SETCURSEL,
							CurTheme->TargetViewport-1,NULL);
       	SendDlgItemMessage (hWndDlg,SV_DISPLAY_DIST,BM_SETCHECK,CurTheme->DisplayDistance,0L);
       	SendDlgItemMessage (hWndDlg,SV_DISPLAY_PCT,BM_SETCHECK,CurTheme->DisplayPCT,0L);
       	SendDlgItemMessage (hWndDlg,SV_DISPLAY_COUNT,BM_SETCHECK,CurTheme->DisplayCount,0L);
       	SendDlgItemMessage (hWndDlg,SV_APPEND_COUNT,BM_SETCHECK,CurTheme->AppendCount,0L);
       	SendDlgItemMessage (hWndDlg,SV_PCTBYAREA,BM_SETCHECK,CurTheme->PCTByArea,0L);
       	SendDlgItemMessage (hWndDlg,SV_INVERT,BM_SETCHECK,CurTheme->InvertLegend,0L);
       	SendDlgItemMessage (hWndDlg,IDC_FILLROW,BM_SETCHECK,CurTheme->FillRow,0L);
       	SendDlgItemMessage (hWndDlg,SV_FLIP,BM_SETCHECK,CurTheme->FlipLegend,0L);
       	SendDlgItemMessage (hWndDlg,SV_FACTOR,BM_SETCHECK,CurTheme->FactorLegend,0L);   
       	SendDlgItemMessage (hWndDlg,SV_HIDENULLCLASSES,BM_SETCHECK,CurTheme->HideNullClasses,0L);   
       	SendDlgItemMessage (hWndDlg,SV_COMPRESSNULLCLASSES,BM_SETCHECK,CurTheme->CompressNullClasses,0L);   
       	SendDlgItemMessage (hWndDlg,IDC_CENTERTEXT,BM_SETCHECK,CurTheme->CenterText,0L); 
       	ltoa (IDNINT(CurTheme->ShowValAZ * RADtoDEG),str,10);
       	SetDlgItemText (hWndDlg,IDC_SHOWVALROT,str);
       	
       	SendDlgItemMessage (hWndDlg,IDC_SETCOLOR,BM_SETCHECK,(!CurTheme->NotSetColor),0L);   
       	
{
#if ENABLETRACE
GSSiExitProg (1294);
#endif
		return FALSE;
}
	   
 	case WM_DESTROY: 
	 	InitDlgPrompts (0);    
		DestroyFieldList ();
 		return FALSE;
 		 
    case WM_COMMAND:
         switch(wParam)
           {
            case SV_TARGET:
              switch(HIWORD(lParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
               {
               		LPVIEWPORT	SaveView;
               		short	n,i;
            
		    		if (CurTheme->TargetViewport)
		    		{
			    		SaveView = CurView; 
			    		SetCurView (pViewports[CurTheme->TargetViewport-1]);
			    		for (i=0,n=0;i<CurView->NumThemes;i++)
			    		{
			    			if (CurView->pThemes[i]!=CurTheme)
			    				CurView->pThemes[n++]=CurView->pThemes[i];
			    		}	
			    		CurView->NumThemes=n;
			    		SetCurView (SaveView);
			    	}  
			    }
		           break;  
		      }    		  		    
              break;
            
            case SV_CONTENTS_LIST:
               { 
              	switch(HIWORD(lParam))
                {
	                 case CBN_DROPDOWN:
	                 {
	                 	short	idesc, iparent, i, j;
                	    LPSTR	pTAGList;  
                	    HANDLE	hPar=GSSiGlobAlloc (1015,GMEM_MOVEABLE,4096);
                	    short	nPar=0;
                	    LPSHORT	pPar, pPar2; 
                	    char	str[64], SymbolName[34];
                	    BOOL	IsPar;  
                	    LPVIEWPORT	SaveVP=CurView;
	                 	 
		                CurTheme->TargetViewport=SendDlgItemMessage(hWndDlg,SV_TARGET,
											       CB_GETCURSEL,NULL,NULL)+1;
			            SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST,CB_RESETCONTENT,NULL,NULL); 
			            if (!CurTheme->TargetViewport) break;
			            
			            if (CurTheme->ID != GF_BOUNDS_DISPLAY_THEME)
			            { 
				            SetViewport (CurTheme->TargetViewport);
				            SelectVisList (FALSE);
				        }
		        		//GetVisList (hWndDlg,0,-SV_CONTENTS_LIST,0,-1);
	 	                //SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST,CB_ADDSTRING,NULL,(LPARAM)((LPSTR) "(ALL)"));
				    	for (idesc=1;idesc<3201;idesc++) 
				    	{
							if (CurView->CurVisType[idesc])
							{   char	SymbolName[34];
							
								GetSymbolName (idesc,SymbolName,&iparent,0,&IsPar);
								if (SymbolName[0] && !IsPar)
								{
			 	                	SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST,CB_ADDSTRING,NULL,(LPARAM)((LPSTR) SymbolName));
			 	                	pPar = (LPSHORT)GlobalLock (hPar);
			 	                	for (i=0;i<nPar;i++,pPar++)
			 	                		if (iparent == *pPar)
			 	                			goto GotPar;
			 	                	*pPar = iparent;
			 	                	nPar++;
			 	            GotPar: GlobalUnlock (hPar);  
			 	                }
		 	                }
						}     
 	                	pPar = (LPSHORT)GlobalLock (hPar);
 	                	for (i=0;i<nPar;i++,pPar++)
 	                	{
							GetSymbolName (*pPar,SymbolName,&iparent,0,NULL);
							if (SymbolName[0])
							{   
								sprintf (str,"(%s)",SymbolName);
			 	                SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)str));
		 	                	pPar2 = (LPSHORT)GlobalLock (hPar);
		 	                	for (j=0;j<nPar;j++,pPar2++)
		 	                		if (iparent == *pPar2)
		 	                			goto GotPar2;
		 	                	*pPar2 = iparent;
		 	                	nPar++;
		 	            GotPar2: GlobalUnlock (hPar); 
		 	                }
		 	            } 
 	            		GSSiGlobUlFree (&hPar);
                	    
                	    if (CurView->hTAGList)
                	    {
							pTAGList = GlobalLock(CurView->hTAGList);
							while (*pTAGList)
							{   
								char	tag[16];
								
								sprintf (tag,"-%s",pTAGList);
		 	               		SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)tag));
								pTAGList+=10;
							}
							GlobalUnlock (CurView->hTAGList);
						}
                        SetCurView (SaveVP);
					}
				    break; 
				}
                break;
               }
            
           	case IDC_BGCOLOR: 
           		 Color = CurTheme->BGColor; 
            	 if (GetColor(hWndDlg,&Color))
            	 	CurTheme->BGColor = Color;
           		 break;
                 
           	case IDC_VALBGCOLOR:
           		 Color = CurTheme->IBBGColor;  
            	 if (GetColor(hWndDlg,&Color))
            	 	CurTheme->IBBGColor = Color;
           		 break;
                 
           	case IDC_TITBGCOLOR:  
           		 Color = CurTheme->TitleBoxBG;
            	 if (GetColor(hWndDlg,&Color))
            	 	CurTheme->TitleBoxBG = Color;
           		 break;
                 
            case IDC_TITLEFONT:
            	 CurTheme->TitleFont.lfHeight = 10;
            	 GetFont (hWndDlg, &CurTheme->TitleFont, &CurTheme->TitleTextColor);
            	 break;
            	 
            case IDC_VALUEFONT:  
            	 CurTheme->ClassFont1.lfHeight = 10;
            	 GetFont (hWndDlg, &CurTheme->ClassFont1, &CurTheme->IBTextColor[0]); 
            	 break;
            	      
            case IDC_PCTFONT:  
            	 CurTheme->ClassFont2.lfHeight = 10;
            	 GetFont (hWndDlg, &CurTheme->ClassFont2, &CurTheme->IBTextColor[1]); 
            	 break;
            	      
            case IDC_SHOWVALFONT:  
            	 GetFont (hWndDlg, &CurTheme->ShowValueFont, &CurTheme->ShowValueTextColor); 
            	 break;
            	      
            case IDC_SETSQL:      
            {    
                 HANDLE hMem;
                 LPSTR  lpStr;
                 
                 hMem = GSSiGlobAlloc (1016,GHND,4096);
                 lpStr = GlobalLock (hMem); 
                 GetDlgItemText (hWndDlg,IDC_SQL,lpStr,1024);
                 if (GetSQLWhereClause (hWndDlg, hSQL, lpStr))
                 	SetDlgItemText (hWndDlg,IDC_SQL,lpStr);    
                 GSSiGlobUlFree (&hMem);
                 break;
            }
            case IDC_LOAD_THEME:  
            {
            	HFILE	Fid;  
            	char	File[128]=""; 
            	OFSTRUCT	OFStruct;
            	LPTHEME	NewTheme; 
            	short	i,n;
            	LPVIEWPORT	SaveView;
            	
             	 _fstrcpy (gszFilter,"GeoMaster Themes(*.thm)|*.THM|");
				 if (GetFileName3(hWndDlg,File,0,IDS_FILETHM))
				 {   
					 Fid = GSSiOpenFile (File,(LPOFSTRUCT)&OFStruct,OF_READ); 
					 ReadObject (&Fid,TRUE,&NewTheme,0); 
					 GSSiClose (Fid);

					if (CurView->pTheme)
					{   
						CurTheme = CurView->pTheme;
			    		SaveView = CurView;
			    		SetCurView (pViewports[CurTheme->TargetViewport-1]);
			    		for (i=0,n=0;i<CurView->NumThemes;i++)
			    		{
			    			if (CurView->pThemes[i]!=CurTheme)
			    				CurView->pThemes[n++]=CurView->pThemes[i];
			    		}	
			    		CurView->NumThemes=n;
			    		SetCurView (SaveView);
			   	    	CloseObject (CurView->pTheme);
			   	    } 
                     CurTheme = NewTheme;
                     CurView->pTheme = NewTheme;
				 	 if (CurTheme->TargetViewport > *pNumViewports) 
				 	 	CurTheme->TargetViewport = 1; 
				 	 if (pViewports[CurTheme->TargetViewport-1]->NumThemes)
	    		 	 	pViewports[CurTheme->TargetViewport-1]->NumThemes--;
					 CurTheme->IsActive=FALSE;
	                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
				 }
			}
            	 break;
            	 
            case IDC_SAVE_THEME:  
 
            case IDOK:  
            	 GetDlgItemText (hWndDlg,SV_DATABASE_LIST,CurTheme->DataFile,sizeof(CurTheme->DataFile)-1);
                 CurTheme->TargetViewport=SendDlgItemMessage(hWndDlg,SV_TARGET,
									       CB_GETCURSEL,NULL,NULL)+1;  
	             CurTheme->DisplayViewport = CurView->ID;
	    		 SaveView = CurView;
            	 CurTheme->DisplayDistance = SendDlgItemMessage (hWndDlg,SV_DISPLAY_DIST,BM_GETCHECK,0,0L);  
            	 CurTheme->DisplayCount = SendDlgItemMessage (hWndDlg,SV_DISPLAY_COUNT,BM_GETCHECK,0,0L);  
            	 CurTheme->AppendCount = SendDlgItemMessage (hWndDlg,SV_APPEND_COUNT,BM_GETCHECK,0,0L);  
            	 CurTheme->DisplayPCT = SendDlgItemMessage (hWndDlg,SV_DISPLAY_PCT,BM_GETCHECK,0,0L);  
            	 CurTheme->PCTByArea = SendDlgItemMessage (hWndDlg,SV_PCTBYAREA,BM_GETCHECK,0,0L);  
            	 CurTheme->InvertLegend = SendDlgItemMessage (hWndDlg,SV_INVERT,BM_GETCHECK,0,0L);  
            	 CurTheme->FillRow = SendDlgItemMessage (hWndDlg,IDC_FILLROW,BM_GETCHECK,0,0L);  
            	 CurTheme->FlipLegend = SendDlgItemMessage (hWndDlg,SV_FLIP,BM_GETCHECK,0,0L);  
            	 CurTheme->FactorLegend = SendDlgItemMessage (hWndDlg,SV_FACTOR,BM_GETCHECK,0,0L);  
            	 CurTheme->NotSetColor = !SendDlgItemMessage (hWndDlg,IDC_SETCOLOR,BM_GETCHECK,0,0L);  
            	 CurTheme->HideNullClasses = SendDlgItemMessage (hWndDlg,SV_HIDENULLCLASSES,BM_GETCHECK,0,0L);  
            	 CurTheme->CompressNullClasses = SendDlgItemMessage (hWndDlg,SV_COMPRESSNULLCLASSES,BM_GETCHECK,0,0L);  
            	 CurTheme->CenterText = SendDlgItemMessage (hWndDlg,IDC_CENTERTEXT,BM_GETCHECK,0,0L); 
            	 GetDlgItemText (hWndDlg,IDC_SHOWVALROT,str,16); 
            	 CurTheme->ShowValAZ = atof (str) * DEGtoRAD;
 	    		 SetCurView (pViewports[CurTheme->TargetViewport-1]);
	    		 for (i=0,n=0;i<CurView->NumThemes;i++)
	    		 { 
	    			if (CurView->pThemes[i]==CurTheme)
	    				n++;
	    		 }
	    		 if (!n)
    				CurView->pThemes[CurView->NumThemes++]=CurTheme;
	    		 SetCurView (SaveView);
				 GetDlgItemText (hWndDlg,SV_CONTENTS_LIST,Contents,34);
				 SetThemeContents (CurTheme,Contents);
                 if (CurTheme->NumClass)
                 	CurTheme->IsActive = TRUE;
{
#if ENABLETRACE
GSSiExitProg (1294);
#endif
                 return FALSE; 
}
                 
		    default:
{
#if ENABLETRACE
GSSiExitProg (1294);
#endif
		        return FALSE;
}
		

           }
         break; 

    default:
{
#if ENABLETRACE
GSSiExitProg (1294);
#endif
        return FALSE;
}
   }  
{
#if ENABLETRACE
GSSiExitProg (1294);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}  

BOOL SetThemeContents (LPTHEME CurTheme,LPSTR Contents) 
{   
	LPVIEWPORT	SaveView = CurView;

	if (Contents[0] == '-')
	{
		_fstrncpy (CurTheme->Contents,Contents,9);
		CurTheme->Contents[9]=0;
		CurTheme->SymNum = -1;  
	}
	else if (Contents[0] == '$')
	{
		_fstrncpy (CurTheme->Contents,Contents,9);
		CurTheme->Contents[9]=0;
		CurTheme->SymNum = -2;  
	}
	else if (!_fstricmp (Contents,"(ALL)"))
	{
		_fstrncpy (CurTheme->Contents,Contents,9);
		CurTheme->SymNum = 0;  
	}
	else
	{   
		LPSTR	lpContents=Contents;
		if (CurTheme->ID != GF_BOUNDS_DISPLAY_THEME)
			SetViewport (CurTheme->TargetViewport);
		if (*lpContents == '(')
		{
		 	LPSTR	lpEnd;
							 	
		 	lpEnd = _fstrchr (Contents,0);
		 	lpEnd--;
		 	if (*lpEnd == ')')
		 	{
		 		lpContents++;
		 		*lpEnd = 0;
		 	} 
		}
		*CurTheme->Contents	=0; 
		{
		    LPVISLIST	SaveVis=CurVis; 
		    HANDLE		handle;
				    
		    handle=GSSiGlobAlloc (1017,GHND,sizeof(VISLIST));
			CurVis = (LPVISLIST)GlobalLock (handle); 
		    InitVis ();
			CurTheme->SymNum=GetSymbolNum (lpContents);
			CurVis = SaveVis;
			GSSiGlobUlFree (&handle); 
		}
	}
	SetCurView (SaveView); 
	return TRUE;
}        

BOOL GetValsFromTable(HWND hWnd)
#if ENABLETRACE
{GSSiEnterProg (1295);
#endif
{
	
	GSSiGlobFree (&hVALUELIST); 
	if (hWndSELVAL)
{
#if ENABLETRACE
GSSiExitProg (1295);
#endif
		return FALSE;
}
	lpfnSELECTVALUESMsgProc = MakeProcInstance((FARPROC)SELECTVALUESMsgProc, hInst);
	CreateDialog(hInst, (LPSTR)"SELECTVALUES", hWnd, lpfnSELECTVALUESMsgProc);
//	DialogBox(hInst, (LPSTR)"SELECTVALUES", hWnd, lpfnSELECTVALUESMsgProc);
//	FreeProcInstance(lpfnSELECTVALUESMsgProc);
{
#if ENABLETRACE
GSSiExitProg (1295);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

void DestroySELECTVALUES (void)
#if ENABLETRACE
{GSSiEnterProg (1296);
#endif
{   
	GSSiGlobFree (&hVALUELIST);
	if (!hWndSELVAL)
{
#if ENABLETRACE
GSSiExitProg (1296);
#endif
		return;
}
	DestroyWindow(hWndSELVAL);
	hWndSELVAL = 0;
	FreeProcInstance((FARPROC) lpfnSELECTVALUESMsgProc);
{
#if ENABLETRACE
GSSiExitProg (1296);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL SELECTVALUESMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1297);
#endif
{
	int	IDC_FieldName=SV_FIELD_NAME; 
	BOOL	True=TRUE;  
	LPOPENFILEDATA  FilePtr;
	LPOPENSQLDATA   SQLPtr;  

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1297);
#endif
 	return (BRtn);
}
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam,IDC_SQL,
 					 SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, &IDC_FieldName,1,
 					 SELVALDataFile, &SELVALDataFileType, &SELVALhDB, &True,FALSE))
{
#if ENABLETRACE
GSSiExitProg (1297);
#endif
 					 return TRUE;
}
 switch(Message)
   {
    case WM_INITDIALOG: 
         hWndSELVAL=hWndDlg;
       	 SetDlgItemText (hWndDlg,IDC_SQL,SELVALSQL);    
         SetDlgItemText(hWndDlg,SV_FIELD_NAME,SELVALFieldName); 
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
         break; /* End of WM_CLOSE                                      */
    
    case WM_DESTROY:
		 CloseDataFile (FALSE,&SELVALhDB);              
		 GSSiGlobFree (&hVALUELIST);
   		 hWndSELVAL = 0; 
    	 break;
    	 
    case WM_COMMAND:
    	 switch(wParam)
           {
            case IDC_OPEN_DB:
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),TRUE); 
                 break;
                      
            case IDC_SETSQL:      
            {    
                 if (GetSQLWhereClause (hWndDlg, SELVALhDB, SELVALSQL))
                 	SetDlgItemText (hWndDlg,IDC_SQL,SELVALSQL);    
                 break;
            }
            
            case IDC_EXIT:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
        		 DestroyWindow(hWndDlg);
        		 hWndSELVAL = 0; 
                 break;
            
            case IDOK:
            {
            	HANDLE	hBT;
				char	mess[128];  
				short	cond = BT_FIRST, Dummy;     
				long	NumVals=0;
				LPSTR	pVal, pSQL;
				            	
                SQLPtr = (LPOPENSQLDATA)GlobalLock (SELVALhDB);
                FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
				hBT = CreateUniqueList (32,NULL);  
				GetDlgItemText (hWndDlg,IDC_SQL,SELVALSQL,256); 
				if (_fstricmp (SELVALSQL,"ALL ROWS"))
					pSQL = SELVALSQL;
				else
					pSQL = NULL;
				GetDlgItemText (hWndDlg,SV_FIELD_NAME,SELVALFieldName,64);
				GetODBCUniqueFieldValues (FilePtr->FileHandle,pSQL,SELVALFieldName,32,hBT); 
				GlobalUnlock (SELVALhDB); 
				hVALUELIST = GSSiGlobAlloc (1018,GHND,UINT_MAX);
				pVal = GlobalLock (hVALUELIST);
				while (!BT_FIND (hBT,pVal,cond,BT_ANY,(LPSTR)&Dummy))
				{  
					cond = BT_NEXT;
					pVal = _fstrchr (pVal,0);
					pVal++;       
					NumVals++;
				}  
				BT_CLOSEANDDELETE (&hBT);  
				GlobalUnlock (hVALUELIST);
				sprintf (mess,"%ld values retrieved",NumVals);
				SetDlgItemText (hWndDlg,IDC_MESS,mess); 
				EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE);
             }
                break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1297);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1297);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 


BOOL FAR PASCAL CLASSESFROMTABLEMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1298);
#endif
{
	int	IDC_FieldName[3]={SV_KEYFIELD_NAME,SV_SYMBOLFIELD_NAME,SV_TITLEFIELD_NAME}; 
	int	IDC_FieldName2 = SV_VALUEFIELD_NAME;
	BOOL	True=TRUE;
	LPOPENFILEDATA  FilePtr;
	LPOPENSQLDATA   SQLPtr;  

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1298);
#endif
 	return (BRtn);
}
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam,IDC_SQL,
 					 SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, IDC_FieldName,3,
 					 SELVALDataFile, &SELVALDataFileType, &SELVALhDB, &True,FALSE))
{
#if ENABLETRACE
GSSiExitProg (1298);
#endif
 					 return TRUE;
}
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam,NULL,
 					 NULL, IDC_CLASSVALDB, IDC_CLASSVALTABLE,SV_TABLE_HEADING2, &IDC_FieldName2,1,
 					 SELVALDataFile, &SELVALDataFileType, &SELVALhDB, &True,FALSE))
{
#if ENABLETRACE
GSSiExitProg (1298);
#endif
 					 return TRUE;
}
 switch(Message)
   {
    case WM_INITDIALOG:  
    	 SetDlgItemText (hWndDlg,SV_DATABASE_LIST,CurTheme->ClassDefDB);
       	 SetDlgItemText (hWndDlg,IDC_SQL,SELVALSQL);    
         SetDlgItemText(hWndDlg,SV_FIELD_NAME,SELVALFieldName);  
         SetDlgItemText (hWndDlg,IDC_SQL,"ALL ROWS");
		 SetDlgItemText (hWndDlg,IDC_SQL,CurTheme->ClassDefSQL);
       	 SetDlgItemText (hWndDlg,SV_TITLEFIELD_NAME,CurTheme->ClassDefTitleField);
		 SetDlgItemText (hWndDlg,SV_KEYFIELD_NAME,CurTheme->ClassDefKeyField);
		 SetDlgItemText (hWndDlg,SV_VALUEFIELD_NAME,CurTheme->ClassDefValField);
		 SetDlgItemText (hWndDlg,SV_SYMBOLFIELD_NAME,CurTheme->ClassDefSymField);
         SetDlgItemText (hWndDlg,IDC_CLASSVALDB,CurTheme->ClassDefValDB);
         SetDlgItemText (hWndDlg,IDC_SQL2,CurTheme->ClassDefValSQL);
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */
    
    case WM_COMMAND:
    	 switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break;
            
            case IDOK:
            {
            	HANDLE	hBT;
				char	Value[132], str[256],SymName[128],CSize[128],CRot[128],CColor[128];  
				short	cond = BT_FIRST, Dummy, ClassNo;     
				long	NumVals=0;
				LPSTR	pVal, pSQL,pColon;
				 
				_fmemset (CurTheme->SymbolFont,0,sizeof(CurTheme->SymbolFont));           	
				CloseDataFile (FALSE,&SELVALhDB); 
            	GetDlgItemText (hWndDlg,SV_DATABASE_LIST,SELVALDataFile,128);
            	_fstrcpy (CurTheme->ClassDefDB,SELVALDataFile);
            	GetDlgItemText (hWndDlg,IDC_SQL,CurTheme->ClassDefSQL,256);
            	GetDlgItemText (hWndDlg,SV_TITLEFIELD_NAME,CurTheme->ClassDefTitleField,32);
            	GetDlgItemText (hWndDlg,SV_KEYFIELD_NAME,CurTheme->ClassDefKeyField,32);
            	GetDlgItemText (hWndDlg,SV_VALUEFIELD_NAME,CurTheme->ClassDefValField,32);
            	GetDlgItemText (hWndDlg,SV_SYMBOLFIELD_NAME,CurTheme->ClassDefSymField,32);
				if (!OpenDataFile (SELVALDataFile,CurTheme->ClassDefSQL,BT_READ,&SELVALhDB))
					break;
                SQLPtr = (LPOPENSQLDATA)GlobalLock (SELVALhDB);
                FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
//				hBT = CreateUniqueList (128,TempName);  
				GetDlgItemText (hWndDlg,IDC_SQL,CurTheme->ClassDefSQL,256); 
				if (_fstricmp (CurTheme->ClassDefSQL,"ALL ROWS"))
					pSQL = CurTheme->ClassDefSQL;
				else
					pSQL = NULL;
				GetDlgItemText (hWndDlg,SV_FIELD_NAME,SELVALFieldName,64);
				GetDBUniqueFieldValues (SELVALhDB,pSQL,CurTheme->ClassDefTitleField,&hBT); 
				CurTheme->NumClass = 0;
				while (!BT_FIND (hBT,CurTheme->ClassBM[CurTheme->NumClass],cond,BT_ANY,(LPSTR)&Dummy))
				{  
					cond = BT_NEXT; 
					CurTheme->NumClass++; 
				}  
				BT_CLOSEANDDELETE (&hBT); 
				GlobalUnlock (SQLPtr->OFHandle); 
				GlobalUnlock (SELVALhDB); 
				CloseDataFile (FALSE,&SELVALhDB); 
				if (*CurTheme->ClassDefSymField)
				{
					 sprintf (str,"%s = [%%TEMP]",CurTheme->ClassDefTitleField);            
					 OpenDataFile (SELVALDataFile,str,BT_READ,&SELVALhDB);
					 for (ClassNo=0;ClassNo<CurTheme->NumClass;ClassNo++) 
					 {  
						SetGlobalValue("%TEMP",CurTheme->ClassBM[ClassNo]);
						sprintf (ClassLink[ClassNo],"[%s]",CurTheme->ClassDefKeyField);
						ExpandText (ClassLink[ClassNo]);
						sprintf (str,"[%s]",CurTheme->ClassDefSymField);
						ExpandText (str);
						DecodePointSym (str,SymName,CSize,CRot,CColor);
						if ((pColon = _fstrchr (SymName,':')))
						{   
							short	FontNum;
							
							*pColon++ = 0;
							for (FontNum=0;FontNum<4;FontNum++)
							{
								if (!_fstricmp (CurTheme->SymbolFont[FontNum],SymName))
									break;   
								if (!*CurTheme->SymbolFont[FontNum])
								{   
									_fstrcpy (CurTheme->SymbolFont[FontNum],SymName);
									break;
								}
							}
							CurTheme->ClassSymbol[ClassNo] = atoi (pColon)+FontNum*1000;
						}
						else
							CurTheme->ClassSymbol[ClassNo] = GetDictSymbolNumber (SymName);
						CurTheme->ClassColor[ClassNo] = GetColorFromName (CColor);  
						_fstrncpy (CurTheme->SymSizeC,CSize,32);
					 }
					 CloseDataFile (FALSE,&SELVALhDB); 
				}

    			CurTheme->hScatterFile = CreateClassValueList(); 
    			CurTheme->AllValueClass = 0;
            	GetDlgItemText (hWndDlg,IDC_CLASSVALDB,SELVALDataFile,128);
            	_fstrcpy (CurTheme->ClassDefValDB,SELVALDataFile);
            	if (!*SELVALDataFile)
            	{
					for (ClassNo=1;ClassNo<CurTheme->NumClass+1;ClassNo++) 
					{  
						_fstrncpy (Value,ClassLink[ClassNo-1],GetBTKeyLen(CurTheme->hScatterFile)); 
			    		BT_PUT (CurTheme->hScatterFile,Value,(LPSTR)&ClassNo); 
			    	}
            	}
            	else
            	{
					if (!OpenDataFile (SELVALDataFile,CurTheme->ClassDefValSQL,BT_READ,&SELVALhDB))
						break;
	                SQLPtr = (LPOPENSQLDATA)GlobalLock (SELVALhDB);
	                FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
					for (ClassNo=1;ClassNo<CurTheme->NumClass+1;ClassNo++) 
					{  
					    
//						hBT = CreateUniqueList (128,TempName);
						if (!_fstrnicmp (ClassLink[ClassNo-1],"SQL(",4))  
						{
							_fstrcpy (str,&ClassLink[ClassNo-1][4]); 
							*LastChr(str) = 0;
						}						
						else
							sprintf (str,"%s = '%s'",CurTheme->ClassDefKeyField,ClassLink[ClassNo-1]);
//						GetODBCUniqueFieldValues (FilePtr->FileHandle,str,CurTheme->ClassDefValField,128,hBT);
						GetDBUniqueFieldValues (SELVALhDB,str,CurTheme->ClassDefValField,&hBT); 
						cond = BT_FIRST; 
						while (!BT_FIND (hBT,str,cond,BT_ANY,(LPSTR)&Dummy))
						{  
							cond = BT_NEXT; 
							_fstrncpy (Value,str,GetBTKeyLen(CurTheme->hScatterFile)); 
				    		BT_PUT (CurTheme->hScatterFile,Value,(LPSTR)&ClassNo);
				    	} 
						BT_CLOSEANDDELETE (&hBT);  
	                 }
					 GlobalUnlock (SQLPtr->OFHandle); 
					 GlobalUnlock (SELVALhDB); 
					 CloseDataFile (FALSE,&SELVALhDB);
				} 
           		BT_CLOSE (CurTheme->hScatterFile);
           		CurTheme->hScatterFile = 0;
                EndDialog(hWndDlg, TRUE);
            }
                break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1298);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1298);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL SetProcessStatusTitle (LPSTR Title)
{   
	USHORT	i;
	
	if (!Title)
	{
		for (i=0;i<MAX_VIEWPORTS;i++)
			*ViewportStatusMessages[i]=0;   
		return TRUE;
	}
	_fstrcpy (ViewportStatusMessages[CurView->ID-1],Title);
	return TRUE;
} 

void DisplayDataPassMessage (void)
{   
	if (*ViewportStatusMessages[CurView->ID-1])
		CreateProcessStatusWindow (CurView->hWnd,ViewportStatusMessages[CurView->ID-1]); 
	return;
}
   
void RemoveDataPassMessage (void)
{   
	DestroyProcessStatusWindow ();
	return;
}

BOOL ThemeNeedsDataPass (BOOL PixelThemesOnly)
#if ENABLETRACE
{GSSiEnterProg (1319);
#endif
{	BTVARDESC	BTVar[3];
	int	i;
	OFSTRUCT	OFStruct; 
	char		str[128];
    
    if (CurTheme->DisplayViewport && CurTheme->DisplayViewport <= *pNumViewports)
    	CurTheme->VPDisplayed = pViewports[CurTheme->DisplayViewport-1]->Active;
	if (!CurTheme->VPDisplayed || !CurTheme->IsActive ||
		 CurTheme->ID == PF_COORD_DISPLAY || CurTheme->ID == PF_BOUNDS_DISPLAY)
{
#if ENABLETRACE
GSSiExitProg (1319);
#endif
		return (FALSE);
}   
	if ((PixelThemesOnly && CurTheme->DataType != THEMEDATATYPE_PIXEL) || 
		(!PixelThemesOnly && CurTheme->DataType == THEMEDATATYPE_PIXEL))
{
#if ENABLETRACE
GSSiExitProg (1319);
#endif
		return (FALSE);
}   
	if (CurTheme->PCTByArea && CurTheme->PCTDisplayCycle != DisplayCycle)
		ComputePCTTheme = CurTheme;	                                     
	else
		ComputePCTTheme = 0;	                                     
	if (!CurTheme->WantDataPass) 
	{
	    if (CurTheme->ID != GF_GRAPHICS_FUNCTION_THEME && CurTheme->DataFileType != MSACCESS_DATAFILE)
	    {
	    	OpenThemeDataFile ();
            if (CurTheme->hThemeDB && GetDBType (CurTheme->hThemeDB) != GMTEXT_DATAFILE)
            	 GetDBFieldInfo (&CurTheme->Field,CurTheme->hThemeDB);
	    }
{
#if ENABLETRACE
GSSiExitProg (1319);
#endif
		return FALSE;
}   
	}
	if (!CurTheme->Recompute && CurTheme->NumClass)
{
#if ENABLETRACE
GSSiExitProg (1319);
#endif
		return (FALSE);
}
	if (*CurTheme->RefValChar)
	{
		_fstrcpy (str,CurTheme->RefValChar);
		ExpandText (str);  
		CurTheme->RefValDbl = atof (str);
	}
	CurTheme->Xmin = DBL_MAX;
	CurTheme->Xmax = -DBL_MAX;
	if (CurTheme->ZeroBased)
		CurTheme->Ymin = 0;
	else
		CurTheme->Ymin = DBL_MAX;
	CurTheme->Ymax = -DBL_MAX;
	CurTheme->NumVals = 0;
	if (CurTheme->SymNum > 0)
	{
		GSSiGlobFree (&CurTheme->hVisList);
		CurTheme->hVisList = SetThemeVisList (CurTheme->SymNum);   
	}
	if (CurTheme->ID == GF_HOTSPOT_THEME)
	{   
		LPHOTSPOTDATA	pHSData = &CurTheme->HotSpotData;
		double dw;//, shrinkfactor=1;                             
		LPVIEWPORT	SaveVP = CurView; 
		long	GridSize;
	    double  BASEX[4], BASEY[4], HSX[4], HSY[4];  
	    double	BaseDToWinD=1, d1, d2;
	    DPOINT	dp1, dp2,HotSpotPoint[2];
	    float	RSQMIN;
		
		GSSiGlobFree (&pHSData->hGrid);
		GSSiGlobFree (&pHSData->hMask);
		CloseTRANS2 (&pHSData->hTranBaseToHotSpot);
        
		SetViewport (CurTheme->TargetViewport);   
	    if (CurView->DisplayInParent && CurView->Parent> 0) 
	    	CurView = pViewports[CurView->Parent-1]; 
/*        dp1.x = CurView->WBounds.xmn;
        dp1.y = CurView->WBounds.ymn;
        dp2.x = CurView->WBounds.xmx;
        dp2.y = CurView->WBounds.ymx;
        d1 = ldistp (dp1,dp2);
        dp1.x = CurView->DrawRect.left;
        dp1.y = CurView->DrawRect.right;
        dp2.x = CurView->DrawRect.top;
        dp2.y = CurView->DrawRect.bottom;
        d2 = ldistp (dp1,dp2);
        if (d1)
        	BaseDToWinD = d2/d1;
        
		dw = BaseDToWinD * pHSData->Radius;
		if (dw > 128)
			shrinkfactor = dw / 128;
		pHSData->MaskWidth = max (2,IDNINT (dw * shrinkfactor));*/   
		if (!Printing)
		if (CurView->DrawRect.right - CurView->DrawRect.left > CurView->DrawRect.bottom - CurView->DrawRect.top)  
		{
			pHSData->GridWidth = MAXHOTSPOTDIMENSION;
			pHSData->GridHeight = pHSData->GridWidth * (double)(CurView->DrawRect.bottom - CurView->DrawRect.top)/(double)(CurView->DrawRect.right - CurView->DrawRect.left);
		}
		else
		{
			pHSData->GridHeight = MAXHOTSPOTDIMENSION;
			pHSData->GridWidth = pHSData->GridHeight * (double)(CurView->DrawRect.right - CurView->DrawRect.left)/(double)(CurView->DrawRect.bottom - CurView->DrawRect.top);
		}
//		pHSData->GridWidth = ((CurView->DrawRect.right - CurView->DrawRect.left) * shrinkfactor)/pHSData->Granularity;
//		pHSData->GridHeight = ((CurView->DrawRect.bottom - CurView->DrawRect.top) * shrinkfactor)/pHSData->Granularity;  
		GridSize =  (long)pHSData->GridWidth * (long)pHSData->GridHeight * 4; 
		pHSData->hGrid = GSSiGlobAlloc (1023,GHND,GridSize);
		pHSData->TotalIncidents=0; 
		pHSData->SecondsRepresented=GetSecondsInSample();
		HSX[0]=HSX[1]=0;
		HSX[2]=HSX[3]=pHSData->GridWidth-1;
		HSY[0]=HSY[3]=0;
		HSY[1]=HSY[2]=pHSData->GridHeight-1;
		BASEX[0]=BASEX[1]=CurView->NewBounds.xmn;
		BASEX[2]=BASEX[3]=CurView->NewBounds.xmx;
		BASEY[0]=BASEY[3]=CurView->NewBounds.ymn;
		BASEY[1]=BASEY[2]=CurView->NewBounds.ymx;
		pHSData->hTranBaseToHotSpot = STRAN2 (BASEX,BASEY,HSX,HSY,4,&RSQMIN,1,NULL);  
		dp1 = dp2 = MinMaxMidPointD (&CurView->NewBounds); 
		_fstrcpy (str,CurTheme->ClassDefValSQL);
		ExpandText (str);
		ExpandText (str);
		pHSData->Radius = atof (str); 
		if (!pHSData->Radius)
			pHSData->Radius = 500;
		dp2.x += pHSData->Radius; 
		HotSpotPoint[0] = TranPoint (&dp1,pHSData->hTranBaseToHotSpot);
		HotSpotPoint[1] = TranPoint (&dp2,pHSData->hTranBaseToHotSpot);  
		pHSData->MaskWidth = max (1,ldistp (HotSpotPoint[0],HotSpotPoint[1]));
		pHSData->hMask = GSSiGlobAlloc (1024,GMEM_MOVEABLE,(long)pHSData->MaskWidth * (long)pHSData->MaskWidth * 4); 
		CurTheme->Ymin = 0;
		CurTheme->Ymax = 0;
		SetupHotSpotMask (pHSData->MaskWidth,pHSData->hMask,pHSData->DecayOpt);  
    	OpenThemeDataFile ();
		SetCurView (SaveVP);
	}
	else
	{
		BT_CLOSEANDDELETE (&CurTheme->hScatterFile); 
		if (CurTheme->DisplayScatterDiagram || CurTheme->ClassType == 2)
		{
			if (CurTheme->ScatterFile[0])
				GSSiRemove (CurTheme->ScatterFile);
			else
				GSSiGetTempFileName (NULL,"gmt",NULL,(LPSTR)CurTheme->ScatterFile);
		
			BTVar[0].BT_VARTYP=BT_REAL;
			BTVar[0].BT_VARLEN=8;
			BTVar[0].BT_VAROFF=0;
			BTVar[1].BT_VARTYP=BT_REAL;
			BTVar[1].BT_VARLEN=8;
			BTVar[1].BT_VAROFF=8;
			BTVar[2].BT_VARTYP=BT_INTEGER;
			BTVar[2].BT_VARLEN=4;
			BTVar[2].BT_VAROFF=16;
			BT_CREATE (CurTheme->ScatterFile, 16, FALSE, 3, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
			CurTheme->hScatterFile = BT_OPEN (CurTheme->ScatterFile,0, BT_WRITE, 0);  
		}
	    if (CurTheme->DataFileType != MSACCESS_DATAFILE)
	    {
	    	OpenThemeDataFile ();
            if (CurTheme->hThemeDB && GetDBType (CurTheme->hThemeDB) != GMTEXT_DATAFILE)
            	 GetDBFieldInfo (&CurTheme->Field,CurTheme->hThemeDB);
	    }
    }
{
#if ENABLETRACE
GSSiExitProg (1319);
#endif
	return(TRUE);
}
#if ENABLETRACE
}
#endif
}

BOOL SetVisibilityFromTheme (LPSTR Name,short setopt)
#if ENABLETRACE
{GSSiEnterProg (1325);
#endif
{   
	HFILE		Fid;  
	OFSTRUCT	OFStruct;
	LPTHEME		pTheme, SaveTheme=CurTheme; 
	short		pos=BT_FIRST, idesc;
	char		SymName[128],Data[128];
	
	Fid = GSSiOpenFile (Name,(LPOFSTRUCT)&OFStruct,OF_READ);  
	if (Fid == HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (1325);
#endif
		return FALSE;
}
	ReadObject (&Fid,FALSE,&pTheme,NULL); 
	GSSiClose (Fid);
	if (pTheme->ScatterFile)
	{
		pTheme->hScatterFile = BT_OPEN (pTheme->ScatterFile,0, BT_READ, 0); 
		while (!BT_FIND (pTheme->hScatterFile,SymName,pos,BT_ANY,Data))
		{
			pos = BT_NEXT;
			if ((idesc = GetSymbolNum (SymName)))
			{
				if (GetVisibility(idesc) != setopt)
					ToggleVisibility (idesc);
			}
		} 
	}	
	CloseObject (pTheme);  
	CurTheme = SaveTheme;
    TurnOffAutoVis (TRUE);		   
{
#if ENABLETRACE
GSSiExitProg (1325);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

void BoundsDisplayTheme (BOOL Init)
#if ENABLETRACE
{GSSiEnterProg (1329);
#endif
{
	short		SaveNum, SaveDVI, itheme;    
	static		BOOL	InBDT=FALSE;
	
    if (InBDT)
{
#if ENABLETRACE
GSSiExitProg (1329);
#endif
    	return;
}
    InBDT = TRUE;
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{
		CurTheme = CurView->pThemes[itheme];
		BoundsDisplayTheme2 (Init);
	}
	InBDT = FALSE;
{
#if ENABLETRACE
GSSiExitProg (1329);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void ClearChildVP (short VPID)
{
	UINT	i;
	
	for (i=0;i<*pNumViewports;i++)
	{
		if (pViewports[i]->Parent == VPID) 
		{
			SaveDC (pViewports[i]->hDC);
			SetDisplayMode (pViewports[i]->hDC, GF_TEXTMODE);
		  	SelectClipRgn (pViewports[i]->hDC,NULL);
		    FillRectPoly (pViewports[i]->hDC,&pViewports[i]->Rect,WindowColor);
	       	DisplayTAGs2 (pViewports[i]->hDC,2,0);
	       	ClearChildVP (pViewports[i]->ID);  
	       	RestoreDC (pViewports[i]->hDC,-1);
	    }
	}
	return;
}

void BoundsDisplayTheme2 (BOOL Init)
#if ENABLETRACE
{GSSiEnterProg (1330);
#endif
{
	LPVIEWPORT	SaveVP=CurView;
	DPOINT		WPoint;  
	BOOL		HaveRef, ClearVP=FALSE;
	LPSAVESCREEN	pSaveScreen;
	LPBOUNDSDISPLAY lpBoundsDisplay;
	MNMXCORD	WBounds, PKBounds;
	POINT		BegPoint, EndPoint;
	HPEN		ArrowPen; 
	long		SaveDispC; 
	static		LPTHEME		InBDTheme=NULL; 
	LPTHEME		SaveIBDT; 
	int	RegionType;
	DWORD	Err;

	if (CurTheme->ID != GF_BOUNDS_DISPLAY_THEME ||
		!CurTheme->IsActive|| 
		!CurTheme->VPDisplayed)
{
#if ENABLETRACE
GSSiExitProg (1330);
#endif
		return; 
}
	if (CurTheme == InBDTheme)
{
#if ENABLETRACE
GSSiExitProg (1330);
#endif
		return; 
}
	lpBoundsDisplay = (LPBOUNDSDISPLAY)&CurTheme->ClassBM; 
	if (Init) 
	{
		lpBoundsDisplay->SavedScreen = NULL;
		lpBoundsDisplay->CurAreaRef = LONG_MAX;
{
#if ENABLETRACE
GSSiExitProg (1330);
#endif
		return;
}
	}   
//	SetCurView (pViewports[CurTheme->TargetViewport-1]); //tempdebu
	SetViewport (CurTheme->TargetViewport);
    if (CurView->DisplayInParent && CurView->Parent > 0) 
    	CurView = pViewports[CurView->Parent-1]; 
	WBounds = CurView->NewBounds;
//	SetCurView (pViewports[CurTheme->DisplayViewport-1]); //tempdebu
	SetViewport (CurTheme->DisplayViewport);
	if (CurTheme->NumVals == DisplayCycle && !CurView->LinkedTo)  
	{
		SetCurView (SaveVP);  
{
#if ENABLETRACE
GSSiExitProg (1330);
#endif
		return;
}
	}
	SaveIBDT = InBDTheme;  
	InBDTheme = CurTheme;
	CurTheme->NumVals = DisplayCycle;
	CurView->BoundsDisplayVP = CurTheme->TargetViewport;   
	if (*lpBoundsDisplay->BoundsGlobal) 
	{   
		HANDLE	hStr=GSSiGlobAlloc (1026,GMEM_MOVEABLE,256);
		LPSTR	pStr=GlobalLock (hStr); 
		short	err;
		
		_fstrcpy (pStr,lpBoundsDisplay->BoundsGlobal);
		ExpandText (pStr); 
		CurView->NewBounds = atobounds (pStr,&err); 
		GSSiGlobUlFree (&hStr);
		if (err)
			ClearVP = TRUE; 
		else
			SetBoundsRect2 (CurView->DrawRect,CurView->hDC);
	}
	else if (!CurView->hTranWinToBase) 
	{
		if (lpBoundsDisplay->VPBoundsOpt != 1) 
			GetVisBounds (&CurView->NewBounds,CurView->hDC);   //MOD:020313
		SetBoundsRect2 (CurView->DrawRect,CurView->hDC); 
	}  
	if (!CurView->Bitmap) 
	{     
		lpBoundsDisplay->SavedScreen = NULL;
		lpBoundsDisplay->CurAreaRef = LONG_MAX;
	}
	HaveRef = FALSE;
	if (*lpBoundsDisplay->CurAreaRefGlobal && !lpBoundsDisplay->IgnoreClear)
	{   
		long		WantRef;
		LPVIEWPORT	SaveView=CurView;
		LPTHEME		SaveTheme=CurTheme;     
		LPTHEME		pTheme;
			
		WantRef = GetGlobalLVal3 (lpBoundsDisplay->CurAreaRefGlobal,LONG_MAX);
		if (WantRef != LONG_MAX)
		{
			if (PickByRefno(WantRef,NULL,NULL,-100))
			{
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
					HPDPOINT	lpDpoint; 
					BOOL	rtn;  
					DPOINT	AreaPoints[5]; 
					LPWORD	pPolyParts; 
					WORD	nPParts;
					short	nareas; 
					long	nPnts;
						
		            BoundsToPoints (&WBounds,AreaPoints,NULL);   
		            AreaPoints[4] = AreaPoints[0];
                    if (hSavePolyParts)
                    {
                    	pPolyParts = (LPWORD)GlobalLock (hSavePolyParts);
                    	nareas = *pPolyParts++; 
                    }
                    else
                    { 
                        nareas=1;
                        nPParts = nSavePoly;  
                        pPolyParts = &nPParts;
                    }
		            lpRect = (LPMNMXCORD) GlobalLock (hSavePoly);  
		            HaveRef = BoundsInBounds (&WBounds,lpRect,0);
		            lpRect++;
		            lpDpoint = (HPDPOINT) lpRect; 
		            while (nareas-- && !HaveRef)
		            {   
		            	
			            HaveRef = PolyInArea (0,*pPolyParts,lpDpoint,NULL,5,AreaPoints,InclusionOpt,0);
			            lpDpoint += (*pPolyParts+1);
			            pPolyParts++;
			        }
		            GSSiGlobUlFree (&hSavePoly);
		            GSSiGlobUlFree (&hSavePolyParts);
		        } 
		        DestroySavedPolys ();
			}
		} 
		CurTheme = SaveTheme;
		SetCurView (SaveView);
	}
	if (CurTheme->SymNum && !HaveRef)
	{   
		long	SaveDisplayCycle = DisplayCycle;
		LPTHEME	SaveTheme=CurTheme;     
		short	SaveMaxPick = MaxPick, SavePP = PickPerim;     
		BOOL	SaveDH = DisableHalt; 
PICKDATA	pd;			
		WPoint =  MinMaxMidPointD (&WBounds);
		UseUserPickAp =FALSE;
		SystemPickAp = -5;	
		MaxPick = 10;  
		DisableHalt=TRUE;
		PickPerim = 0; 
		ClearMaskArea ();
		{
			short SaveNumVehicles=NumVehicles; 
		
			NumVehicles = 0;
			HaveRef = PickItems (CurView->hWnd,WPoint); 
			NumVehicles = SaveNumVehicles;
		} 
if(NumPicked)
	pd=PickList[NumPicked-1];		
		PickPerim = SavePP; 
		DisableHalt=SaveDH;
		MaxPick = SaveMaxPick;
		UseUserPickAp =TRUE;
		CurTheme = SaveTheme;
	}
	if (HaveRef)
	{	
		long	SaveDisplayCycle = DisplayCycle;
		LPTHEME	SaveTheme=CurTheme; 
			    
    	while (NumPicked--)
    	{   
    		if (SymInContents (PickList[NumPicked].Desc,CurTheme->SymNum,CurTheme->Contents))
    		{
	    		ProcessPickedItem (NumPicked,FALSE);
				CurTheme = SaveTheme;	
	    		ProcessText (CurTheme->Title);
				CurTheme = SaveTheme;	
	    		if (PickList[NumPicked].Refno != lpBoundsDisplay->CurAreaRef)  
	    		{
		    		short SaveNumVP=NumViewportsToDisplay,n;
		    		BOOL	DisplayVP[MAX_VIEWPORTS]; 
		    		HANDLE	hMem = GSSiGlobAlloc (1027,GMEM_MOVEABLE,1024);
		    		LPSTR	CacheFile=GlobalLock (hMem), SaveScreenFile=CacheFile+256; 
		    		LPOFSTRUCT	pOFStruct=(LPOFSTRUCT) (SaveScreenFile + 256);  
		    		long	SaveSize;
			    		 
		    		lpBoundsDisplay->CurAreaRef = PickList[NumPicked].Refno;
		    		n=SaveNumVP;
		    		while (n--)
		    			DisplayVP[n] = pViewports[n]->Display;
		    		if (!Printing)
		    		{   
//			    		_fstrcpy (SaveScreenFile,"[%DL]savescrn");  
//			    		ExpandText (SaveScreenFile);
			    		GetTempDir (SaveScreenFile);  
			    		_fstrcat (SaveScreenFile,"\\gmsavesc");
			    		GSSiMakeDir (SaveScreenFile,&Err);
			    		sprintf (_fstrchr(SaveScreenFile,0),"\\%lx.scr",lpBoundsDisplay->CurAreaRef);
			        	DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
		                if (GetCacheFile (CacheFile, SaveScreenFile,FALSE, 0))
		                {   
		                	long	UpdateTime;
		                	if ((CurView->Bitmap = ReadSavedScreen (CacheFile,&UpdateTime)))
		                	{
		                		pSaveScreen = (LPSAVESCREEN)GlobalLock (CurView->Bitmap);
	                			pSaveScreen->pVP = CurView; 
	                			CurView->BitmapID = pSaveScreen->ID;
		                		if (CurView->Rect.left != pSaveScreen->Rect.left ||
		                			CurView->Rect.right != pSaveScreen->Rect.right ||
		                			CurView->Rect.top != pSaveScreen->Rect.top ||
		                			CurView->Rect.bottom != pSaveScreen->Rect.bottom)
		                		{
		                			GlobalUnlock (CurView->Bitmap);
		                			DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID); 
		                			RemoveCacheFile (SaveScreenFile);
		                		}
		                		else
		                		{
		                			GlobalUnlock (CurView->Bitmap);
		                		}
		                	}
		                } 
		            } 
		            PKBounds = PickList[NumPicked].Rect;
					InflateBounds (&PKBounds,CurTheme->RefValDbl);
	                if (Printing || !CurView->Bitmap)
	                {   
	                	BOOL SaveOZA = OutlineZoomArea, SaveGMA = GetMaskArea, SaveMOL = MaskOffsetLine, SaveDH=DisableHalt; 
	                	LPVIEWPORT	SaveVP2;
		                	
						OutlineZoomArea = CurTheme->ZeroIsMissing;   
						MaskOffsetLine = CurTheme->AddCommas;
		            	if (MaskOffsetLine || OutlineZoomArea)
		            		GetMaskArea = TRUE;
						SetMaskArea(NumPicked,0,1);
						DisableHalt = TRUE; 
						DisplayCycle--; 
						SaveVP2 = CurView; 
					    CurView->CurZoomAreaRef = 0;
			    		ZoomToRect(PKBounds,TRUE); 
			    		SetCurView (SaveVP2);
			    		DisableHalt = SaveDH; 
			    		if (!Printing)
			    		{ 
							DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
			                CurView->Bitmap = SaveScreen2 (CurView->hDC, CurView->Rect,CurView,&CurView->BitmapID);
			                CurView->BitmapRect = CurView->Rect;
			                WriteSavedScreen (SaveScreenFile,CurView->Bitmap);
		                	GetCacheFile (CacheFile, SaveScreenFile,TRUE, 0); 
		                	GSSiRemove (SaveScreenFile);
		                }
	                	OutlineZoomArea = SaveOZA;
						GetMaskArea = SaveGMA;  
						MaskOffsetLine = SaveMOL;
		            }
		            else 
		            {
		            	BOOL SaveDisplay=Display;
			            	
		            	Display = FALSE;
						DisplayCycle--;
					    CurView->CurZoomAreaRef = 0;
		            	ZoomToRect(PKBounds,2);
		            	Display = SaveDisplay;                    
		            	DisplayTAGs2 (CurView->hDC,1,0); 
		            }
	                CurView->BitmapRect = CurView->Rect;
		    		NumViewportsToDisplay = SaveNumVP;
		    		n=SaveNumVP;
		    		while (n--)
		    			pViewports[n]->Display = DisplayVP[n]; 
		    		NumPicked = 0; 
		    		GSSiGlobUlFree (&hMem);
		    	}
	    	}
		}
		DisplayCycle = SaveDisplayCycle;
	}
	else if (CurTheme->RefValDbl < 0)
	{	
		long	SaveDisplayCycle = DisplayCycle; 
   		short SaveNumVP=NumViewportsToDisplay,n;
   		BOOL	DisplayVP[MAX_VIEWPORTS]; 
		
		n=SaveNumVP;
		while (n--)
			DisplayVP[n] = pViewports[n]->Display; 
		if (CurTheme->RefValDbl < -1000000)
			GetVisBounds (&PKBounds,CurView->hDC);
		else 
		{			
			if (CurView->Bitmap && BoundsInBounds (&WBounds,&CurView->NewBounds,0)) 
				PKBounds = CurView->NewBounds; 
			else
			{
				DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
	        	PKBounds = WBounds;
				InflateBounds (&PKBounds,-CurTheme->RefValDbl);
			}
		}
        if (Printing || !CurView->Bitmap)
        {   
        	BOOL SaveOZA = OutlineZoomArea, SaveGMA = GetMaskArea, SaveMOL = MaskOffsetLine; 
        	LPVIEWPORT	SaveVP2;
		                	
			OutlineZoomArea = CurTheme->ZeroIsMissing;   
			MaskOffsetLine = CurTheme->AddCommas;
        	if (MaskOffsetLine || OutlineZoomArea)
        		GetMaskArea = TRUE;
			SetMaskArea(NumPicked,0,1);
			DisableHalt = TRUE; 
			DisplayCycle--; 
			SaveVP2 = CurView; 
		    CurView->CurZoomAreaRef = 0;
    		ZoomToRect(PKBounds,TRUE); 
    		SetCurView (SaveVP2);
    		DisableHalt = FALSE; 
    		if (!Printing)
    		{ 
				DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
                CurView->Bitmap = SaveScreen2 (CurView->hDC, CurView->Rect,CurView,&CurView->BitmapID);
                CurView->BitmapRect = CurView->Rect;
/*                WriteSavedScreen (SaveScreenFile,CurView->Bitmap);
            	GetCacheFile (CacheFile, SaveScreenFile,TRUE, 0); 
            	GSSiRemove (SaveScreenFile);*/
            } 
        	OutlineZoomArea = SaveOZA;
			GetMaskArea = SaveGMA; 
			MaskOffsetLine = SaveMOL;
        }
        else 
        {
        	BOOL SaveDisplay=Display;
			            	
        	Display = FALSE;
			DisplayCycle--;
		    CurView->CurZoomAreaRef = 0;
        	ZoomToRect(PKBounds,2);
        	Display = SaveDisplay;                    
        	DisplayTAGs2 (CurView->hDC,1,0); 
        }
        CurView->BitmapRect = CurView->Rect;
		NumViewportsToDisplay = SaveNumVP;
		n=SaveNumVP;
		while (n--)
			pViewports[n]->Display = DisplayVP[n]; 
		NumPicked = 0; 
		DisplayCycle = SaveDisplayCycle;
    }
	else if  (CurTheme->SymNum || *lpBoundsDisplay->CurAreaRefGlobal)  
		ClearVP = TRUE; 
	if (lpBoundsDisplay->IgnoreClear)
		ClearVP = FALSE;
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	if (ClearVP)
	{   
		RECT	Rect=CurView->Rect;
			
	  	SelectClipRgn (CurView->hDC,NULL);
		InflateRect (&Rect,1,1);
		if (*lpBoundsDisplay->CurAreaRefGlobal)
			SetGlobalValueLong (lpBoundsDisplay->CurAreaRefGlobal,LONG_MAX);
	    FillRectPoly (CurView->hDC,&Rect,WindowColor);
       	DisplayTAGs2 (CurView->hDC,2,0);
       	ClearChildVP (CurView->ID); 
	}
    GSSiDeleteObject(&CurView->hRgn);
	CurView->hRgn = CreateVPRgn(FALSE,FALSE);
  	RegionType = SelectClipRgn (CurView->hDC,CurView->hRgn);
  	GSSiDeleteObject(&CurView->hRgn);
	if (!ClearVP && RegionType != NULLREGION)
		ShowZoomArea (CurView->hDC,&WBounds,
					               &lpBoundsDisplay->SavedScreen,
					               &lpBoundsDisplay->SavedRect,
								   lpBoundsDisplay->BoxPoints);


	RestoreDC (CurView->hDC,-1);  
	SetCurView (SaveVP);  
	InBDTheme = SaveIBDT;
{
#if ENABLETRACE
GSSiExitProg (1330);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetDifferenceValue (LPTHEME pDiffTheme,long ValueColor,LPSTR Value)
{   
	short	pos=BT_FIRST;
	short	Color;
	
	pDiffTheme->hScatterFile = BT_OPEN (pDiffTheme->ScatterFile,0, BT_READ, 0); 
	while (!BT_FIND (pDiffTheme->hScatterFile,Value,pos,BT_ANY,(LPSTR)&Color))
	{   
		if (Color == ValueColor)
		{   
			BT_CLOSE (pDiffTheme->hScatterFile);
			pDiffTheme->hScatterFile = 0;
			return TRUE;
		}
		pos = BT_NEXT;
	}
	*Value = 0;
	BT_CLOSE (pDiffTheme->hScatterFile);
	pDiffTheme->hScatterFile = 0;
	return FALSE;
}

BOOL GetNextPointInAp (LPPOINT pPoint,short iPickAP,LPSHORT pFirst,LPRECT pRect)
{   
	static	short	Ring;  
	static	double	NextAZ;  
	static	POINT	MidPoint;
	double	AZInc;
	POINT	NewPt;
	
	if (*pFirst)
	{
		*pFirst = FALSE; 
		Ring = 1;
		NextAZ = 0;  
		MidPoint = *pPoint;
	} 
	do
	{
		if (NextAZ >= 2 * PY)
		{
			Ring++;
			NextAZ = 0;
		} 
		if (Ring > iPickAP)
			return FALSE;
		NewPt = newpt (MidPoint, NextAZ,Ring);
		AZInc = 1.0/(2 * PY * Ring);
		NextAZ += AZInc; 
	} 
	while (!PtInRect (pRect,NewPt));
	*pPoint = NewPt;
	return TRUE;
}

BOOL GetValueDifference (POINT Point,LPHANDLE phBox,LPVIEWPORT *ppVP,LPLONG pValueColor1,LPLONG pValueColor2)
{   
	LPSAVESCREEN	pSaveScreen;
	COLORREF Color;
	POINT	Point1,Point2, ScreenPoint, OrigPoint=Point;
	char	str[512], Value1[256], Value2[256]; 
	short	iPickAP, inc, i, XInc, YInc;  
	LPTHEME	pDiffTheme, SaveTheme=CurTheme;    
	BOOL	First;
    
	SaveDC (CurView->hDC);
    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
  	SelectClipRgn (CurView->hDC,NULL);
    if (!CurView->pTheme)
    {
		RestoreDC (CurView->hDC,-1);
    	return FALSE;
    }
    iPickAP = SetPickAp(0)/2;
    First = TRUE;   
    do
    {
	    Color  = GetPixel (CurView->hDC,Point.x,Point.y);
    }
    while (Color == CurView->BackGroundColor && GetNextPointInAp (&Point,iPickAP,&First,&CurView->DrawRect));
	if (Color == CurView->BackGroundColor) 
	{
		RestoreDC (CurView->hDC,-1);
    	return FALSE; 
    }
    CurTheme = CurView->pTheme;
    for (i=0;i<pViewports[CurTheme->TargetViewport-1]->NumThemes;i++)
    {
	    pDiffTheme = pViewports[CurTheme->TargetViewport-1]->pThemes[i];
	    if (pDiffTheme->ID == GF_SINGLE_NONNUM_VALUE_THEME && !pDiffTheme->NumDesiredClass)
    		goto HaveTheme;
    }
    CurTheme = SaveTheme;
	RestoreDC (CurView->hDC,-1);
    return FALSE;
HaveTheme:  
	RemoveLinkedCursors ();
	Point1 = pViewports[CurTheme->TargetViewport-1]->LastCursorPos;
	Point2 = pViewports[CurTheme->DataType-1]->LastCursorPos;  
	XInc = Point.x - OrigPoint.x;
	YInc = Point.y - OrigPoint.y;
	Point1.x += XInc;
	Point1.y += YInc;  
	Point2.x += XInc;
	Point2.y += YInc;  
    *pValueColor1 = GetPixel (CurView->hDC,Point1.x,Point1.y);	
    *pValueColor2 = GetPixel (CurView->hDC,Point2.x,Point2.y);
    ScreenPoint = Point;
    ClientToScreen (CurView->hWnd,(LPPOINT)&ScreenPoint);
    SetCursorPosGM (ScreenPoint.x,ScreenPoint.y,TRUE);
    GetDifferenceValue (pDiffTheme,*pValueColor1,Value1);
    GetDifferenceValue (pDiffTheme,*pValueColor2,Value2);
    sprintf (str,"%s\r\n%s",Value1,Value2);	
    Point.y -= 10;
	*phBox = YellowTextBox (CurView->hWnd,str,Point,0,FALSE);  
	if (*phBox)
	{
		pSaveScreen = (LPSAVESCREEN)GlobalLock (*phBox);
		pSaveScreen->UserID = 0;
        GlobalUnlock (*phBox); 
    }
	if (ppVP)
		*ppVP = CurView;
	CurTheme = SaveTheme; 
	RestoreDC (CurView->hDC,-1);
	return FALSE;
} 
