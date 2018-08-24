#include "graphint.h"
#include "extrndb.h"     
#include "gps.h" 

#define SELWAYPNTVERSION	2
 

#include "gmextern.h"


static	struct	{short FieldID;
				 long  Value;} WPSelKey;

static	short	nWPSelected;
static	BOOL	LoadWPAsRoute;
static	BOOL	PromptForWPData=TRUE;
static	BOOL    AllowWPSkip=FALSE;   
static	char	LoadWPName[66]="";
static	short	WPFieldID, WPSelCntl; 
static	BOOL	WPAllowRange;
static	HWND	hWndFindWP=0; 
static	char	WPSelTitle[32], WPSelectFile[144]="";   
static	HANDLE	hWPSelect=0;
static	short	DMSFormat=1;       
static	char	WayPointGMD[]={"[%DL]attribut\\waypnt2.gmd"}; 
static	char	SelWaypntDB[]={"[%DL]attribut\\selwaypt.gmd"};    
static	char	SelWaypntGCF[]={"[%DL]attribut\\selwaypt.gcf"};    
static	char	FishGMD[]={"[%DL]attribut\\fish.gmd"};
static	char	GPSUnitsFile[]={"[%DL]gpsunits.txt"};
static	char	DefaultGPSFile[]={"[%DL]gpsfiles\\default.gps"};
static	BOOL	RefreshOnExit;



BOOL FAR PASCAL LOADWAYPOINTSMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
	char	str[128], cUID[32];	
	HDC		hDC; 
	RECT	Rect; 
	static	long	Color; 
	static	BOOL	FirstPaint; 

 int	BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
         cwCenter(hWndDlg, -2);
         if (nWPSelected > 1)
         {
         	sprintf (str,"%i (int)waypoints selected",nWPSelected);
         	ShowWindow (GetDlgItem(hWndDlg,IDC_LOADASROUTE),SW_SHOW);
         	Color = GetGlobalLVal ("[ROUTECOLOR]");   
         }
         else
         { 
         	_fstrcpy (str,"1 waypoint selected");
			SetDlgItemText (hWndDlg,IDC_WPNAME,LoadWPName);
 			SetGlobalValue ("WPNAME",LoadWPName);		
        	ShowWindow (GetDlgItem(hWndDlg,IDC_WPNAME),SW_SHOW);   
         	ShowWindow (GetDlgItem(hWndDlg,IDC_WPNAMET),SW_SHOW);
         	Color = GetGlobalLVal ("[WPCOLOR]");   
         }
       	 SetDlgItemText (hWndDlg,IDC_MESS,str);
       	 SendDlgItemMessage (hWndDlg, IDC_PROMPTFORWPDATA,BM_SETCHECK,PromptForWPData,0);
       	 FirstPaint = TRUE;
         SetTimer(hWndDlg, 100, 10, (TIMERPROC) NULL); 
         break;
         
    case WM_TIMER: 
       	 KillTimer (hWndDlg,100);
    	 FirstPaint = FALSE;  
       	 GetClientRect (GetDlgItem(hWndDlg,IDC_SHOWCOLOR),&Rect);
       	 hDC = GetDC (GetDlgItem(hWndDlg,IDC_SHOWCOLOR));
       	 FillRectPoly (hDC, &Rect,Color);
       	 ReleaseDC (GetDlgItem(hWndDlg,IDC_SHOWCOLOR),hDC);
         break;

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
         {  
         	case IDC_LOADASROUTE:
         		if (SendDlgItemMessage (hWndDlg,IDC_LOADASROUTE,BM_GETCHECK,0,0)) 
         		{
		         	ShowWindow (GetDlgItem(hWndDlg,IDC_WPNAME),SW_SHOW);   
		         	SetDlgItemText (hWndDlg,IDC_WPNAMET,"Route Name");
		         	ShowWindow (GetDlgItem(hWndDlg,IDC_WPNAMET),SW_SHOW);
		        }
		        else
         		{
		         	ShowWindow (GetDlgItem(hWndDlg,IDC_WPNAME),SW_HIDE);   
		         	ShowWindow (GetDlgItem(hWndDlg,IDC_WPNAMET),SW_HIDE);
		        }
         		break;
         	case IDC_GETCOLOR:
         	{
         		COLORREF	NewColor=Color;
			    if(GetColor(hWndDlg,&NewColor)) 
			    	Color = NewColor; 
				GetClientRect (GetDlgItem(hWndDlg,IDC_SHOWCOLOR),&Rect);
				hDC = GetDC (GetDlgItem(hWndDlg,IDC_SHOWCOLOR));
				FillRectPoly (hDC, &Rect,Color);
				ReleaseDC (GetDlgItem(hWndDlg,IDC_SHOWCOLOR),hDC);
			}
         		break;
         		
         	case IDOK: 
         		GetDlgItemText (hWndDlg,IDC_WPNAME,LoadWPName,64);
            	if ((LoadWPAsRoute = SendDlgItemMessage (hWndDlg,IDC_LOADASROUTE,BM_GETCHECK,0,0)))
            		SetGlobalValueLong ("ROUTECOLOR",Color);
            	else
            		SetGlobalValueLong ("WPCOLOR",Color);
            	PromptForWPData = SendDlgItemMessage (hWndDlg,IDC_PROMPTFORWPDATA,BM_GETCHECK,0,0);
                EndDialog(hWndDlg, TRUE);
         		break;
         		
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break;
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL ADDWAYPOINTMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	int	TabStops[6]={81, 95, 139, 184, 258, 328}, ntab=6;
    static	BOOL	First=TRUE, FishChanged;
	static	RECT	WindRect;    
	short	Default, Choice;   
	DPOINT	WayPoint;   
    LPGWDHEADER lpGWDHead; 
	char	str[256];  
	static	long	LakeID;
	long	Offset; 
	static	long	WayPointID;   
	static	short	CurYear=-1,CurMonth,CurDay;  
	static	short	CurWTemp=-1, CurATemp =-1,CurWeather=-1,CurWindSpeed=-1,CurWindDir=-1,CurBarState=-1,CurBarValue=-1;  
	static	BOOL	InCheckSize;
	HANDLE	hDBWP;
	short	st;     
	static	BOOL	NewPoint;
	LPSTR	pBeg, pEnd;
	LPWAYPOINTDATA	pWPData; 
	LPFISHDATA		pFishData; 
	static	HANDLE	hSaveBM=0;   
	static	char	OldName[18];
	struct {long	WayPoint;
			short	FishID;} FishKey;
	
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
//    	 ntab = loadtabs (TabStops);    
    	 hSaveBM = EnterBlockingWindow (hWndDlg);
		 InCheckSize=FALSE;
		 if (CurYear < 0)    
		 {
   			struct tm when;
   			time_t now, result;
			
			time( &now );
   			when = *localtime( &now );
		    CurYear = when.tm_year -99;
		    CurMonth = when.tm_mon;
		    CurDay = when.tm_mday;
		 }
		 if (!AllowWPSkip) 
       	 	ShowWindow (GetDlgItem(hWndDlg,IDC_SKIP),SW_HIDE);   
		 FishChanged = FALSE;
       	 SendDlgItemMessage (hWndDlg,IDC_FISHLIST,LB_SETTABSTOPS,ntab,(LPARAM)&TabStops);
		 if (!First)
		 {  
			SetWindowPos(hWndDlg,HWND_TOP,WindRect.left,WindRect.top,
									   WindRect.right-WindRect.left,
									   WindRect.bottom-WindRect.top,SWP_NOZORDER);
		 } 
		 First = FALSE; 
		 hWndAddWaypoint=hWndDlg;
		 hDBWP = OpenGWDatabase (WayPointGMD,BT_READ); 
		 if (!hDBWP)
		 {
			CreateWaypointGMD (hWndDlg);
		 	hDBWP = OpenGWDatabase (WayPointGMD,BT_READ);
		 }  
		 if (!hDBWP)
		 	break;
		 lpGWDHead = (LPGWDHEADER)GlobalLock (hDBWP); 
		 pWPData = (LPWAYPOINTDATA)&lpGWDHead->GWDData;
		 WayPointID = GetGlobalLVal ("[WPID]");
		 if ((st = BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&WayPointID,BT_FIRST,BT_EQ, (LPSTR)&Offset)))
		 {
		 	short	idepth;
		 	
		 	_fmemset (&lpGWDHead->GWDData,0,(size_t)lpGWDHead->Reclen); 
/*		 	if ((pWPData->LakeID = GetLakeIDFromPick (NewPointLoc)))   
				SetGlobalValueLong ("NEWPOINTLAKENUM",pWPData->LakeID);
		 	else
				pWPData->LakeID = GetGlobalLVal ("[NEWPOINTLAKENUM]"); */ 
			pWPData->LakeID = GetLakeIDFromPick (NewPointLoc);
		 	pWPData->Year = CurYear;
		 	pWPData->Month = CurMonth;
		 	pWPData->Day = CurDay; 
		 	pWPData->WTemp = CurWTemp;  
		 	pWPData->ATemp = CurATemp;  
		 	pWPData->Weather = CurWeather;  
		 	pWPData->WindSpeed = CurWindSpeed;  
		 	pWPData->WindDir = CurWindDir;  
		 	pWPData->BarState = CurBarState;  
		 	pWPData->BarValue = CurBarValue; 
		 	GetGlobalCVal ("[WPDESC]",pWPData->Comment,0); 
		 	GetGlobalCVal ("[WPDEPTH]",str,0); 
		 	idepth = GetListNum ("[%DL]waypopts\\depths.txt",str);
            if (idepth > -1)
            	pWPData->Depth = idepth;
		 	if (!GetGlobalCVal ("[WPNAME]",pWPData->WPName,0))
		 		sprintf (pWPData->WPName,"WP%4.4ld",WayPointID);
		 	NewPoint = TRUE;
		    SetGWDCurrentOffset (lpGWDHead,-1);
		 }
		 else 
		 {
		 	FillGWDData (lpGWDHead,Offset); 
		 	NewPoint = FALSE;
		 	NewPointLoc = pWPData->LatLong; 
		 	ConvertCoord (&NewPointLoc,2,1);   
		 	_fstrncpy (OldName,pWPData->WPName,16);
/*		 	SetGlobalValueReal ("WAYPX",NewCoordPoint.x);
		 	SetGlobalValueReal ("WAYPY",NewCoordPoint.y);*/ 
		 } 
		 LakeID = pWPData->LakeID; 
	 	 SetGlobalValueLong ("DNRID",pWPData->LakeID);
		 *str = 0;
		 {
		 	HANDLE	hDBName = OpenGWDatabase ("[%DL]lakes\\lakename.gmd",BT_READ);
			if (hDBName)
			{
				LPGWDHEADER lpGWDHeadName = (LPGWDHEADER)GlobalLock (hDBName); 
				if (!BT_FIND (lpGWDHeadName->BTHandle[0],(LPSTR)&LakeID,BT_FIRST,BT_EQ, (LPSTR)&Offset))
				{
				 	FillGWDData (lpGWDHeadName,Offset);
		 			GMDGetCharFieldVal (lpGWDHeadName,1,str);  
		 		}
		 		GlobalUnlock (hDBName);
		 		CloseGWDatabase (hDBName);
		    }
		 }
		 SetGlobalValue ("LNAME",str);
		 SetDlgItemTextGlobal (hWndDlg,IDC_LAKENAME,"[LNAME]",0);
		 SetDlgItemTextGlobal (hWndDlg,IDC_WPNUM,"[WPID]",0);
	     SetDlgItemText (hWndDlg,IDC_WPCOMMENT,pWPData->Comment);
	     SetDlgItemText (hWndDlg,IDC_WPNAME,pWPData->WPName);
         Default = FillCBList (hWndDlg,IDC_WAYTYPE,"[%DL]waypopts\\waytype.txt",pWPData->Type,0);
         Default = FillCBList (hWndDlg,IDC_DEPTH,"[%DL]waypopts\\depths.txt",pWPData->Depth,0);
         Default = FillCBList (hWndDlg,IDC_TIME_FROM,"[%DL]waypopts\\times.txt",pWPData->FromTime,0);
         Default = FillCBList (hWndDlg,IDC_TIME_TO,"[%DL]waypopts\\times.txt",pWPData->ToTime,0);
         Default = FillCBList (hWndDlg,IDC_WATERTEMP,"[%DL]waypopts\\wattmp.txt",pWPData->WTemp,0);
         Default = FillCBList (hWndDlg,IDC_AIRTEMP,"[%DL]waypopts\\temps.txt",pWPData->ATemp,0);
         Default = FillCBList (hWndDlg,IDC_MONTH,"[%DL]waypopts\\months.txt",pWPData->Month,0);
         Default = FillCBList (hWndDlg,IDC_DAY,"[%DL]waypopts\\days.txt",pWPData->Day,0);
         Default = FillCBList (hWndDlg,IDC_YEAR,"[%DL]waypopts\\years.txt",pWPData->Year,0);
         Default = FillCBList (hWndDlg,IDC_WEATHER,"[%DL]waypopts\\weather.txt",pWPData->Weather,0);  
         Default = FillCBList (hWndDlg,IDC_WINDSPEED,"[%DL]waypopts\\winspeed.txt",pWPData->WindSpeed,0);
         Default = FillCBList (hWndDlg,IDC_WINDDIR,"[%DL]waypopts\\windir.txt",pWPData->WindDir,0);
         Default = FillCBList (hWndDlg,IDC_BARSTATE,"[%DL]waypopts\\barstate.txt",pWPData->BarState,0);
         Default = FillCBList (hWndDlg,IDC_BARVALUE,"[%DL]waypopts\\barvalue.txt",pWPData->BarValue,0);
         GlobalUnlock (hDBWP);  
    	 CloseGWDatabase (hDBWP);   
    	 
		 hDBWP = OpenGWDatabase (FishGMD,BT_READ);
		 if (!hDBWP) 
		 {
		 	CreateFishGMD (hWndDlg);
		 	hDBWP = OpenGWDatabase (FishGMD,BT_READ); 
		 }
		 if (!hDBWP)
		 	break;
		 lpGWDHead = (LPGWDHEADER)GlobalLock (hDBWP); 
		 pFishData = (LPFISHDATA)&lpGWDHead->GWDData;
		 FishKey.WayPoint = WayPointID;
		 FishKey.FishID = 0;
		 st = BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&FishKey,BT_FIRST,BT_GE, (LPSTR)&Offset);
		 while (!st && FishKey.WayPoint == WayPointID)
		 {
		 	FillGWDData (lpGWDHead,Offset);
		 	GetListValue ("[%DL]waypopts\\typefish.txt", pFishData->Type, str);
        	_fstrcat (str,"\t");              
		 	GetListValue ("[%DL]waypopts\\numfish.txt", pFishData->Num, _fstrchr(str,0));
        	_fstrcat (str,"\t");
			SetGlobalValueLong ("FISHTYPE",(long)pFishData->Type);
		 	GetListValue ("[%DL]waypopts\\size[FISHTYPE].txt", pFishData->Size, _fstrchr(str,0));
        	_fstrcat (str,"\t");
		 	GetListValue ("[%DL]waypopts\\lengths.txt", pFishData->Length, _fstrchr(str,0));
        	_fstrcat (str,"\t");  
		 	GetListValue ("[%DL]waypopts\\tackle.txt", pFishData->Tackle, _fstrchr(str,0));
        	_fstrcat (str,"\t");  
		 	GetListValue ("[%DL]waypopts\\livebait.txt", pFishData->LiveBait, _fstrchr(str,0));
        	_fstrcat (str,"\t");  
        	_fstrcat (str,pFishData->Comment);
		 	SendDlgItemMessage (hWndAddWaypoint,IDC_FISHLIST,LB_ADDSTRING,0,(LPARAM)str); 
		 	st = BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&FishKey,BT_NEXT,BT_ANY, (LPSTR)&Offset);
    	 }
         GlobalUnlock (hDBWP);  
    	 CloseGWDatabase (hDBWP);   
    	 if (!PromptForWPData)
    	 	goto Exit;
//		 NewCoordPoint = CurrentPoint;
//		 ConvertCoord (&NewCoordPoint,1,2);
//         PostMessage(hWndDlg, WM_COMMAND, IDC_UPDATECOORD, 0L);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */
    
    case WM_DESTROY:
    	 hWndAddWaypoint=0;
    	 break;
    	 
    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
		 case IDC_WPNAME:
             switch (HIWORD(wParam))
             {  
				 case EN_CHANGE:
				 {
					 char	WPval[32];
					 int	l;
					 if (InCheckSize)
						 break;
					 InCheckSize = TRUE;
					 l = GetDlgItemText (hWndDlg,IDC_WPNAME,WPval,16);
					 WPval[MAX_WAYPOINT_NAME_LENGTH] = 0;
					 //SetDlgItemText (hWndDlg,IDC_WPNAME,WPval);
					 InCheckSize = FALSE;
				 }
			 break;
			 }
            case IDC_FISHLIST: /* List box                           */
                 switch(HIWORD(wParam))
                 {   
                     case LBN_SELCHANGE:
                         EnableWindow (GetDlgItem(hWndDlg,IDC_EDITFISH),TRUE);
                         EnableWindow (GetDlgItem(hWndDlg,IDC_DELETEFISH),TRUE);
                         break;
                         
                     case LBN_DBLCLK: 
                         PostMessage(hWndDlg, WM_COMMAND, IDC_EDITFISH, 0L);
                     break;
                 }
                 break; 
            
            case IDC_SKIP:   
            	 PromptForWPData = FALSE;
            case IDOK:
         Exit:  
				 hDBWP = OpenGWDatabase (WayPointGMD,BT_WRITE);  
				 if (!hDBWP)
				 {
					 CreateWaypointGMD (hWndDlg);
					 hDBWP = OpenGWDatabase (WayPointGMD,BT_WRITE);  
				 }
				 if (hDBWP)
				 {
					 LPFOUNDWPDATA	pFoundWPData; 
					 LPGWDHEADER lpGWDHeadFound; 
					 HANDLE	hDBFoundWP;  
					 
					 lpGWDHead = (LPGWDHEADER)GlobalLock (hDBWP); 
					 pWPData = (LPWAYPOINTDATA)&lpGWDHead->GWDData;
					 GetDlgItemText (hWndDlg,IDC_WPNUM,str,16);
					 pWPData->WayPointID = atol (str); 
	                 pWPData->Type =(short)SendDlgItemMessage(hWndDlg,IDC_WAYTYPE,CB_GETCURSEL,0,0); 
//					 pWPData->LakeID = GetGlobalLVal ("[NEWPOINTLAKENUM]"); 
					 NewCoordPoint = NewPointLoc;
					 ConvertCoord (&NewCoordPoint,1,2);      
					 pWPData->LakeID = LakeID;
					 pWPData->LatLong = NewCoordPoint;
	                 pWPData->Depth =(short)SendDlgItemMessage(hWndDlg,IDC_DEPTH,CB_GETCURSEL,0,0); 
	                 pWPData->FromTime =(short)SendDlgItemMessage(hWndDlg,IDC_TIME_FROM,CB_GETCURSEL,0,0); 
	                 pWPData->ToTime =(short)SendDlgItemMessage(hWndDlg,IDC_TIME_TO,CB_GETCURSEL,0,0); 
	                 CurWTemp = pWPData->WTemp =(short)SendDlgItemMessage(hWndDlg,IDC_WATERTEMP,CB_GETCURSEL,0,0); 
	                 CurATemp = pWPData->ATemp =(short)SendDlgItemMessage(hWndDlg,IDC_AIRTEMP,CB_GETCURSEL,0,0); 
	                 CurWeather = pWPData->Weather =(short)SendDlgItemMessage(hWndDlg,IDC_WEATHER,CB_GETCURSEL,0,0); 
	                 CurWindSpeed = pWPData->WindSpeed =(short)SendDlgItemMessage(hWndDlg,IDC_WINDSPEED,CB_GETCURSEL,0,0); 
	                 CurWindDir = pWPData->WindDir =(short)SendDlgItemMessage(hWndDlg,IDC_WINDDIR,CB_GETCURSEL,0,0); 
	                 CurBarState = pWPData->BarState =(short)SendDlgItemMessage(hWndDlg,IDC_BARSTATE,CB_GETCURSEL,0,0); 
	                 CurBarValue = pWPData->BarValue =(short)SendDlgItemMessage(hWndDlg,IDC_BARVALUE,CB_GETCURSEL,0,0); 
	                 CurMonth = pWPData->Month =(short)SendDlgItemMessage(hWndDlg,IDC_MONTH,CB_GETCURSEL,0,0); 
	                 CurDay = pWPData->Day =(short)SendDlgItemMessage(hWndDlg,IDC_DAY,CB_GETCURSEL,0,0); 
	                 CurYear = pWPData->Year =(short)SendDlgItemMessage(hWndDlg,IDC_YEAR,CB_GETCURSEL,0,0); 
	                 GetDlgItemText (hWndDlg,IDC_WPCOMMENT,pWPData->Comment,80);
	                 GetDlgItemText (hWndDlg,IDC_WPNAME,pWPData->WPName,16);
					 GWDReplaceRecord (lpGWDHead,0,0,-1);
			    	 if (NewPoint)
			    	 {
						char	lnam[80]="[LNAME]"; 
						
						hDBFoundWP = OpenGWDatabase (SelWaypntDB,BT_WRITE);
						if (!hDBFoundWP)
				 			hDBFoundWP = CreateFoundWPDB ();
				 		lpGWDHeadFound = (LPGWDHEADER)GlobalLock (hDBFoundWP);
				 		if (lpGWDHeadFound->Version < SELWAYPNTVERSION)
				 		{
							GlobalUnlock (hDBFoundWP);  
						    CloseGWDatabase (hDBFoundWP); 
				 			hDBFoundWP = CreateFoundWPDB ();
				 		} 
				 		pFoundWPData = (LPFOUNDWPDATA)&lpGWDHeadFound->GWDData; 
			 			pFoundWPData->ID = WayPointID;
			 			pFoundWPData->Depth = pWPData->Depth;
			 			pFoundWPData->LatLong = pWPData->LatLong;  
			 			_fstrcpy (pFoundWPData->WPName,pWPData->WPName); 
			 			ExpandText (lnam);
			 			sprintf (str,"%s\t%s\t%i/%i/%i\t%i\t%s\t%ld %f %f",pWPData->WPName,lnam,
			 			                              pWPData->Month+1,pWPData->Day,pWPData->Year+1999,  
			 			                              0,"",
			 			                              WayPointID,pWPData->LatLong.x,pWPData->LatLong.y);
			 			_fstrcpy (pFoundWPData->Text,str); 
			 			GWDAddRecord (lpGWDHeadFound,0,0);
						GlobalUnlock (hDBFoundWP);  
					    CloseGWDatabase (hDBFoundWP); 
			    	 } 
			    	 else if (_fstrcmp (pWPData->WPName,OldName))
			    	 {
						hDBFoundWP = OpenGWDatabase (SelWaypntDB,BT_WRITE);
						if (hDBFoundWP)
				 		{
				 			lpGWDHeadFound = (LPGWDHEADER)GlobalLock (hDBFoundWP); 
				 			pFoundWPData = (LPFOUNDWPDATA)&lpGWDHeadFound->GWDData; 
							if (!BT_FIND (lpGWDHeadFound->BTHandle[0],(LPSTR)&WayPointID,BT_FIRST,BT_EQ, (LPSTR)&Offset))
							{
							 	FillGWDData (lpGWDHeadFound,Offset);
			 					_fstrncpy (pFoundWPData->WPName,pWPData->WPName,16); 
			 					REPLAC (pFoundWPData->Text, OldName,pWPData->WPName,255); 
			 					GWDReplaceRecord (lpGWDHeadFound,0,0,-1);
		 					}
							GlobalUnlock (hDBFoundWP);  
					    	CloseGWDatabase (hDBFoundWP);  
					    }
			    	 }
			         GlobalUnlock (hDBWP);  
			    	 CloseGWDatabase (hDBWP); 
			     }   
			     if (FishChanged)
			     {
					 hDBWP = OpenGWDatabase (FishGMD,BT_WRITE);
					 if (hDBWP)
					 {
						 GWFLDINFO	FieldInfo; 
						 short		index;
						 
						 lpGWDHead = (LPGWDHEADER)GlobalLock (hDBWP);     
						 if (!GWDGetFieldInfoFromName (hDBWP,"LENGTH", &FieldInfo,&index))
				 		 {	
													 
	 			             GlobalUnlock (hDBWP);  
				    	     CloseGWDatabase (hDBWP);  
				    	     GMDReorg (FishGMD,1,FALSE,FALSE,hWndDlg,"LENGTH(B2)");
							 hDBWP = OpenGWDatabase (FishGMD,BT_WRITE);
							 lpGWDHead = (LPGWDHEADER)GlobalLock (hDBWP);     
						 }
						 pFishData = (LPFISHDATA)&lpGWDHead->GWDData; 

			DeleteNext:
						 FishKey.WayPoint = WayPointID;
						 FishKey.FishID = 0;
						 st = BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&FishKey,BT_FIRST,BT_GE, (LPSTR)&Offset);
						 if (!st && FishKey.WayPoint == WayPointID)  
						 {
						 	BT_DELETE (lpGWDHead->BTHandle[0],(LPSTR)&FishKey,(LPSTR)&Offset,FALSE);
						 	goto DeleteNext;   
						 }
			         	 Choice=0;
			         	 while (SendDlgItemMessage (hWndDlg,IDC_FISHLIST,LB_GETTEXT,Choice,(LPARAM)str) != LB_ERR)
			         	 {  
			         	 	pFishData->WayPointID = WayPointID;
			         	 	pFishData->FishID = Choice;  
			         	 	pBeg = str;
			         	 	pEnd = _fstrchr (pBeg,'\t');
			         	 	*pEnd++ = 0; 
			         	 	pFishData->Type = GetListNum ("[%DL]waypopts\\typefish.txt",pBeg);
					    	SetGlobalValueLong ("FISHTYPE",(long)pFishData->Type); 
					    	pBeg = pEnd;
			         	 	pEnd = _fstrchr (pBeg,'\t');
			         	 	*pEnd++ = 0; 
			         	 	pFishData->Num = GetListNum ("[%DL]waypopts\\numfish.txt",pBeg);
					    	pBeg = pEnd;  
			         	 	pEnd = _fstrchr (pEnd,'\t');
			         	 	*pEnd++ = 0;
			         	 	pFishData->Size = GetListNum ("[%DL]waypopts\\size[FISHTYPE].txt",pBeg);
					    	pBeg = pEnd;  
			         	 	pEnd = _fstrchr (pEnd,'\t');
			         	 	*pEnd++ = 0;
			         	 	pFishData->Length = GetListNum ("[%DL]waypopts\\lengths.txt",pBeg);
					    	pBeg = pEnd;  
			         	 	pEnd = _fstrchr (pBeg,'\t');
			         	 	*pEnd++ = 0; 
			         	 	pFishData->Tackle = GetListNum ("[%DL]waypopts\\tackle.txt",pBeg);
					    	pBeg = pEnd;  
			         	 	pEnd = _fstrchr (pBeg,'\t');
			         	 	*pEnd++ = 0; 
			         	 	pFishData->LiveBait = GetListNum ("[%DL]waypopts\\livebait.txt",pBeg);
			         	 	_fstrcpy (pFishData->Comment,pEnd);       //sizeof(FISHDATA)
						 	GWDReplaceRecord (lpGWDHead,0,0,-1);
			             	Choice++;
			             }
	 			         GlobalUnlock (hDBWP);  
				    	 CloseGWDatabase (hDBWP); 
					 }  
				 }
				 GetWindowRect (hWndDlg,&WindRect);
                 GSSiEndDialog(hWndDlg, TRUE,hSaveBM);
            	 break;
            
            case IDC_DELETEFISH: 
            	FishChanged = TRUE;
				Choice=SendDlgItemMessage(hWndDlg,IDC_FISHLIST,LB_GETCURSEL,0,0); 
				if (Choice >= 0)
				 	SendDlgItemMessage (hWndDlg,IDC_FISHLIST,LB_DELETESTRING,Choice,(LPARAM)0); 
		        SendDlgItemMessage (hWndDlg,IDC_FISHLIST,LB_SETCURSEL,-1,0); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_EDITFISH),FALSE);
                EnableWindow (GetDlgItem(hWndDlg,IDC_DELETEFISH),FALSE);
            	break;
            	 
            case IDC_ADDFISH:
		        SendDlgItemMessage (hWndDlg,IDC_FISHLIST,LB_SETCURSEL,-1,0); 
                EnableWindow (GetDlgItem(hWndDlg,IDC_EDITFISH),FALSE);
                EnableWindow (GetDlgItem(hWndDlg,IDC_DELETEFISH),FALSE);
            case IDC_EDITFISH:
            {
				DLGPROC lpfnFISHMsgProc; 
				short	nRc;
				 
				lpfnFISHMsgProc = MakeProcInstance((DLGPROC)FISHMsgProc, hInst);
				nRc = DialogBox(hInst, (LPSTR)"FISH", hWndDlg, lpfnFISHMsgProc);
				FreeProcInstance(lpfnFISHMsgProc);
				if (nRc)
				{
	            	FishChanged = TRUE;
	                EnableWindow (GetDlgItem(hWndDlg,IDC_EDITFISH),FALSE);
	                EnableWindow (GetDlgItem(hWndDlg,IDC_DELETEFISH),FALSE);
				}
			}
            	break;
            
            case IDCANCEL:
				 GetWindowRect (hWndDlg,&WindRect);
//                 DestroyWindow (hWndDlg);
				 hWndAddWaypoint=0;
//				 FreeProcInstance((DLGPROC)ADDWAYPOINTMsgProc);
                 GSSiEndDialog(hWndDlg, FALSE,hSaveBM); 
                 if (AllowWPSkip)
                 	SetContinueProcessing (FALSE);
                 break; 
                 
/*            case IDC_UPDATECOORD:
            {
				short	Deg,Min;
				double	Sec; 
				char	txt[16];    
				
				GetDMS (NewCoordPoint.x,&Deg,&Min,&Sec);
			    SetDlgItemInt (hWndDlg,IDC_DEGLONG,abs(Deg),TRUE); 
			    SetDlgItemInt (hWndDlg,IDC_MINLONG,Min,TRUE);
			    sprintf (txt,"%4.1f",Sec);
			    SetDlgItemText (hWndDlg,IDC_SECLONG,txt); 
				GetDMS (NewCoordPoint.y,&Deg,&Min,&Sec);
			    SetDlgItemInt (hWndDlg,IDC_DEGLAT,Deg,TRUE); 
			    SetDlgItemInt (hWndDlg,IDC_MINLAT,Min,TRUE);
			    sprintf (txt,"%4.1f",Sec);
			    SetDlgItemText (hWndDlg,IDC_SECLAT,txt); 
			}
            	break; */
           }
    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL GPSMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
    char    Ext[8], ExtID[32];
    static	char	WPName[32]="";
	char	str[256], Identity[128], Model[128], Port[16], Baud[16], Title[260];  
	int	TabStops[7]={52, 96, 144,276, 340, 410,1000}, ntab=6;
	int	Choice,ii; 
	int	index;
	HFILE	Fid;
	OFSTRUCTGM	OFStruct;
	BOOL	HaveData;
	static	BOOL    SearchALL;
	static	BOOL	HaveGPS, InTran; 
	static	HANDLE	hSaveBM=0;
	LPSTR	pLat;
	int	nItems,i;
	HANDLE	hItems;
	LPINT	pItems;
	
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
    	 RefreshOnExit = FALSE;
    	 hSaveBM = EnterBlockingWindow (hWndDlg);
    	 InTran = HaveGPS=FALSE;
//		 if (!_fstricmp (AppName,"SportMap"))
			ShowWindow (GetDlgItem(hWndDlg,IDC_IMPORTTRKFROMGPS),SW_SHOW);
		 GetPrivateProfileString ("GPS","Model","",str,100,GMIni);
		 ShowWindow (GetDlgItem(hWndDlg,IDC_EXPORTROUTE),SW_SHOW);
		 ShowWindow (GetDlgItem(hWndDlg,IDC_IMPORTTRKFROMGPS),SW_SHOW);
		 GPSFormat = GetGPSFormat (str); 
	  	 if (GPSFormat < 4000)
		 	EnableWindow (GetDlgItem(hWndDlg,IDC_IMPORTTRKFROMGPS),TRUE);
		 else if (GPSFormat == 5000) 
		 {
			ShowWindow (GetDlgItem(hWndDlg,IDC_EXPORTROUTE),SW_HIDE);
			ShowWindow (GetDlgItem(hWndDlg,IDC_IMPORTTRKFROMGPS),SW_HIDE);
		 }
		 	
//    	 ntab = loadtabs (TabStops); 
    case GSSI_REINITDIALOG: 
    	 DMSFormat = GetGlobalLVal2 ("[COORDOPT]",3);
    	 
    	 if (DMSFormat == 1) 
    	 {
			SendDlgItemMessage (hWndDlg,IDC_UNITSDMS,BM_SETCHECK,TRUE,0); 
			SendDlgItemMessage (hWndDlg,IDC_UNITSDM,BM_SETCHECK,FALSE,0); 
		 } 
		 else
    	 {
			SendDlgItemMessage (hWndDlg,IDC_UNITSDMS,BM_SETCHECK,FALSE,0); 
			SendDlgItemMessage (hWndDlg,IDC_UNITSDM,BM_SETCHECK,TRUE,0); 
		 }
       	 SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_SETTABSTOPS,ntab,(LPARAM)&TabStops);
		 SendDlgItemMessage(hWndDlg,IDC_GPSLIST,LB_RESETCONTENT,0,0);  
//		 SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_ADDSTRING,(WPARAM)0,(LPARAM) "1\t2\t3\t4\t5\t6\t7");
    	 GetGlobalCVal ("[%DEPTHORELEV]",str,"Depth");    
    	 SetDlgItemText (hWndDlg,IDC_DORE,str);
    	 GetGlobalCVal ("[%FILEGPS]",str,DefaultGPSFile);
    	 ExpandText (str);
    	 SetDlgItemText (hWndDlg,IDC_FILENAME,str);
    	 Fid = GSSiOpenFile (str,&OFStruct,OF_READ);
    	 if (Fid == HFILE_ERROR)
    	 	break;
		 if (fgetstring (str,250,Fid))
		 {
		 	SetDlgItemText (hWndDlg,IDC_FILETITLE,str);  
		 	while (fgetstring (str,250,Fid))
		 	{   
		 		if (*str)
		 		{
					char savec = str[12];
					   
					str[12]=0;
			 		index = SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_FINDSTRING,(WPARAM)-1,(LPARAM) str); 
			 		str[12] = savec;  
			 		ConvertGPSLatLon (str);
			 		if (index != LB_ERR)
			 		{
			 			SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_DELETESTRING,(WPARAM)index,(LPARAM)0); 
			 			SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_INSERTSTRING,(WPARAM)index,(LPARAM) str); 
			 		}
			 		else
			 			index = SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_ADDSTRING,(WPARAM)0,(LPARAM) str);
			 	} 
		 	}
		 }
    	 GSSiClose (Fid);   
    	 if (*WPName)
    	 {
			index = SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_FINDSTRING,(WPARAM)-1,(LPARAM) WPName); 
			SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_SETTOPINDEX,(WPARAM)index,(LPARAM)0);   
		 }
		 *WPName = 0;
SetGPS:
		 ShowWindow (GetDlgItem(hWndDlg,IDC_EXPORTROUTE),SW_SHOW);
		 ShowWindow (GetDlgItem(hWndDlg,IDC_IMPORTTRKFROMGPS),SW_SHOW);
		 GetPrivateProfileString ("GPS","Port","",Port,16,GMIni);
		 GetPrivateProfileString ("GPS","Baud","",Baud,16,GMIni);
		 if (GetPrivateProfileString ("GPS","Model","",Model,100,GMIni))
		 {
		 	HaveGPS = TRUE;  
		 	if (GPSFormat == 5000)
		 		sprintf (str,"Current GPS: %s",Model); 
		 	else if (!_fstricmp (Port,"USB"))
		 		sprintf (str,"Current GPS: %s on %s",Model,Port); 
		 	else
			 	sprintf (str,"Current GPS: %s on %s at %s baud",Model,Port,Baud);
		 	SetDlgItemText (hWndDlg,IDC_CURGPS,str);
		 }
		 else 
		 {
		 	SetDlgItemText (hWndDlg,IDC_CURGPS,"Current GPS: None");
		 	HaveGPS = FALSE;	
	 	 }
		 GetPrivateProfileString ("GPS","SearchALL","Y",str,2,GMIni);
		 if (*str == 'Y')
			SearchALL = TRUE; 
		 else 
		 	SearchALL = FALSE; 
	 	 EnableWindow (GetDlgItem(hWndDlg,IDC_IMPORTFROMGPS),HaveGPS);
	  	 if (GPSFormat < 4000)
		 	EnableWindow (GetDlgItem(hWndDlg,IDC_IMPORTTRKFROMGPS),TRUE);
		 else if (GPSFormat == 5000) 
		 {
			ShowWindow (GetDlgItem(hWndDlg,IDC_EXPORTROUTE),SW_HIDE);
			ShowWindow (GetDlgItem(hWndDlg,IDC_IMPORTTRKFROMGPS),SW_HIDE);
		 }
		 goto EnableOpts;
         break; /* End of WM_INITDIALOG                                 */
    
    case WM_SETCURSOR:
       	GSSiSetCursor (hCursor);
    	break;
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND: 
    	 if (wParam == IDC_CANCELTRAN)
    	 	SetContinueProcessing (FALSE);
    	 if (!InTran)
         switch(LOWORD(wParam))
           {
            case IDC_SELECTGPSFILE:
            	SaveGPSList (hWndDlg);
				_fstrcpy (Ext,".GPS"); 
				_fstrcpy (ExtID,"GPS Transfer Files");
				sprintf (gszFilter,"%s(*%s)|*%s|",ExtID,Ext,_fstrlwr(Ext));  
				if (GetFileName3(hWndDlg,str,0,IDS_FILEGPS))
				{   
					PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
				} 
            	break;
            
            case IDC_NEWGPSFILE: 
            	SaveGPSList (hWndDlg);
                if (!GetSaveName2 (hWndDlg,str,0,".GPS",IDS_FILEGPS))
            		break; 
            	*Title = 0;
		        if (!GetTextString (hWndDlg,Title,256,"Enter the file title",0,0,0,TRUE,TRUE))
		        	break;
		    	Fid = GSSiOpenFile (str,&OFStruct,OF_CREATE);
		    	if (Fid == HFILE_ERROR)
		    		break;
            	fputstring (Title,Fid);
            	GSSiClose (Fid);
				PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
				break;
            		
            case IDCANCEL:
				GPSClose (&GPSStream);
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
            	break;
            
            case IDOK: 
            {
				LPSTR	Symbol, CRefno;
				long	Refno;
            	
				GPSClose (&GPSStream); 
				SaveGPSList (hWndDlg);
				UpdateGPSWPDB (0);
                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);  
                if (RefreshOnExit)
                	RedisplayWindow ();
            }
            	break;
            
            case IDC_CLEARGPS:
				SendDlgItemMessage(hWndDlg,IDC_GPSLIST,LB_RESETCONTENT,0,0); 
				goto EnableOpts;
            	break;
            	
			case IDC_GPSREMOVE: 
			{
				
				nItems=(short)SendDlgItemMessage(hWndDlg ,IDC_GPSLIST,LB_GETSELCOUNT,0,0); 
				hItems=GSSiGlobAlloc (1034,GMEM_MOVEABLE,nItems*4);
				pItems=  (LPINT) GlobalLock(hItems);
				SendDlgItemMessage(hWndDlg ,IDC_GPSLIST,LB_GETSELITEMS,nItems,(LPARAM)pItems);
				for (i=0;i<nItems;i++)
			 		SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_DELETESTRING,(*pItems++)-i,(LPARAM)0); 
			 	GSSiGlobUlFree (&hItems);
			}
				goto EnableOpts;
				break;
				
			case IDC_UNITSDMS:  
				SetGlobalValueLong ("COORDOPT",1);  
				DMSFormat = 1;
				goto ChangeCoord;
			case IDC_UNITSDM:
				SetGlobalValueLong ("COORDOPT",-3); 
				DMSFormat = -3;
	ChangeCoord:
		 		index = 0;
			    while (SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_GETTEXT,index,(LPARAM)str) != LB_ERR)
			    {  
			 		ConvertGPSLatLon (str);
		 			SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_DELETESTRING,(WPARAM)index,(LPARAM)0); 
			 		SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_INSERTSTRING,(WPARAM)index++,(LPARAM) str); 
			 	} 
				break;
			case IDC_GPSED:
            {
            	 long	WPID; 
            	 LPSTR	pBeg;
            	 
				 SendDlgItemMessage(hWndDlg ,IDC_GPSLIST,LB_GETSELITEMS,1,(LPARAM)&Choice);
		         if (Choice >= 0)
		         {  
		         	SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_GETTEXT,Choice,(LPARAM)str); 
		     	 	pBeg = _fstrrchr (str,'\t');
		     	 	if (!pBeg)
		     	 		break; 
		     	 	pBeg++;
		     	 	WPID = atol (pBeg);
		     	 	if (WPID <=0)
		     	 		break;  
		     	 	pBeg = _fstrchr (str,'\t');
		     	 	if (pBeg)
		     	 	{
		     	 		*pBeg++ = 0;
		     	 		_fstrcpy (WPName,str);
		     	 	}
		     	 	else
		     	 		*WPName = 0;
		     	 	SetGlobalValueLong ("WPID",WPID); 
			        {
			        	BOOL	nRc;
			            
			            setDoPaint( FALSE);   
			            AllowWPSkip=FALSE;   
						lpfnADDWAYPOINTMsgProc = MakeProcInstance((DLGPROC)ADDWAYPOINTMsgProc, hInst);
						nRc = DialogBox(hInst, (LPSTR)"ADDWAYPOINT",hWndDlg, lpfnADDWAYPOINTMsgProc);
						FreeProcInstance(lpfnADDWAYPOINTMsgProc);
						setDoPaint( TRUE); 
         				if (nRc)
         				{ 
					 		SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_DELETESTRING,Choice,(LPARAM)0); 
							RemoveWPNameFromWPFile (WPName);
	                    	AddWPToGPSList (WPName,WPID,0,0);
							PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
 
						}
			        }
		     	 } 
		    }
            	 break;      
				
				break;
				
            case IDC_GPSLIST:
                 switch(HIWORD(wParam))
                 {   
                     case LBN_SELCHANGE:  
            EnableOpts:
					 HaveData = SendDlgItemMessage(hWndDlg,IDC_GPSLIST,LB_GETCOUNT,0,0); 
			     	 SetDlgItemInt (hWndDlg,IDC_POINTSINFILE,HaveData,FALSE);
					 EnableWindow (GetDlgItem(hWndDlg,IDC_CLEARGPS),HaveData);
					 EnableWindow (GetDlgItem(hWndDlg,IDC_SELALL),HaveData);
					 EnableWindow (GetDlgItem(hWndDlg,IDC_PRINTGPS),HaveData);
                     {
	            		short	nItems=SendDlgItemMessage(hWndDlg ,IDC_GPSLIST,LB_GETSELCOUNT,0,0); 
						
						switch (nItems)
						{
							case 0:
								EnableWindow (GetDlgItem(hWndDlg,IDC_EXPORTTOGPS),FALSE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_EXPORTROUTE),FALSE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_GPSREMOVE),FALSE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_GPSED),FALSE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_LOADTODB),FALSE);  
								EnableWindow (GetDlgItem(hWndDlg,IDC_PRINTGPS),FALSE);  
							break;
							case 1:
							{
								LPSTR	pBeg;
								long	WPID;
								
								EnableWindow (GetDlgItem(hWndDlg,IDC_EXPORTTOGPS),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_EXPORTROUTE),FALSE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_GPSREMOVE),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_LOADTODB),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_PRINTGPS),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_GPSED),FALSE);
								SendDlgItemMessage(hWndDlg ,IDC_GPSLIST,LB_GETSELITEMS,1,(LPARAM)&Choice);
						        SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_GETTEXT,Choice,(LPARAM)str); 
						     	pBeg = _fstrrchr (str,'\t');
						     	if (!pBeg)
						     		break; 
						     	pBeg++;
						     	WPID = atol (pBeg);
						     	if (WPID <=0)
						     		break;  
								EnableWindow (GetDlgItem(hWndDlg,IDC_GPSED),TRUE); 
							}
							break; 
							default:
								EnableWindow (GetDlgItem(hWndDlg,IDC_EXPORTTOGPS),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_EXPORTROUTE),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_GPSREMOVE),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_GPSED),FALSE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_LOADTODB),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_PRINTGPS),TRUE);
						} 
					 }
                     break;
                         
                     case LBN_DBLCLK: 
                         PostMessage(hWndDlg, WM_COMMAND, IDC_GPSED, 0L);
                     break;
                 }
            	break;
            
            case IDC_LOADTODB:   
            {
            	short	n=GPSLoadToDB (hWndDlg, IDC_GPSLIST);  
            	
            	if (n < 0)
	        		sprintf (str,"%i point(s) loaded to route",-n);
            	else
	        		sprintf (str,"%i point(s) loaded",n);
				MessageBox (hWndDlg,str," ",MB_OK);
				RefreshOnExit=TRUE;  
			}
            	break;
            	
            case IDC_PRINTGPS:
            {   
            	char	TempFile[128];   
            	HFILE	Fid;
            	
				nItems = GetLBSelectedItems (hWndDlg,IDC_GPSLIST,&hItems);
				if (!nItems)
					break;
				GSSiGetTempFileName (0,"gmf",0,TempFile); 
				Fid = GSSiOpenFile (TempFile,0,OF_CREATE);
				pItems = (LPINT)GlobalLock (hItems);
				fputstring ("Name\tIcon\tDepth\tComments\tLatitude\tLongitude",Fid);  
				for (i=0;i<nItems;i++)
				{
		         	SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_GETTEXT,pItems[i],(LPARAM)str); 
					fputstring (str,Fid);
				}
				GSSiGlobUlFree (&hItems);    
				GSSiClose (Fid);
				PrintTextFile (hWndDlg,TempFile,ntab,TabStops);
				GSSiRemove (TempFile); 
			}
            	break;
            	
            case IDC_SELALL:
				SendDlgItemMessage(hWndDlg ,IDC_GPSLIST,LB_SETSEL,TRUE,(LPARAM)-1);
				goto EnableOpts;
            		
            case IDC_IMPORTFROMGPS:
            	SetDlgItemText (hWndDlg,IDC_TRANSTATUS,"Attempting to open connection to GPS"); 
				if (GPSOpen(hWndDlg,Identity,0,0,0,TRUE)>0)
				{
					sprintf (str,"GPS Functions: Connected to %s",Identity);
					SetWindowText (hWndDlg,str);
					ShowWindow (GetDlgItem(hWndDlg,IDC_TRANSTATUS),SW_SHOW);
					ShowWindow (GetDlgItem(hWndDlg,IDC_CANCELTRAN),SW_SHOW);  
					InTran = TRUE;
            		GPSImportWP (hWndDlg,IDC_GPSLIST,IDC_TRANSTATUS,SearchALL); 
            		InTran = FALSE;
					//ShowWindow (GetDlgItem(hWndDlg,IDC_TRANSTATUS),SW_HIDE);
					ShowWindow (GetDlgItem(hWndDlg,IDC_CANCELTRAN),SW_HIDE);
					goto EnableOpts;
            	}
            	break;
            	
           case IDC_IMPORTTRKFROMGPS:
            	SetDlgItemText (hWndDlg,IDC_TRANSTATUS,"Attempting to open connection to GPS");
				if (GPSOpen(hWndDlg,Identity,0,0,0,TRUE)>0)
				{
					sprintf (str,"GPS Functions: Connected to %s",Identity);
					SetWindowText (hWndDlg,str);
					ShowWindow (GetDlgItem(hWndDlg,IDC_TRANSTATUS),SW_SHOW);
					ShowWindow (GetDlgItem(hWndDlg,IDC_CANCELTRAN),SW_SHOW);  
					InTran = TRUE;
            		GPSImportTrack (hWndDlg,IDC_GPSLIST,IDC_TRANSTATUS); 
            		InTran = FALSE;
//					ShowWindow (GetDlgItem(hWndDlg,IDC_TRANSTATUS),SW_HIDE);
					ShowWindow (GetDlgItem(hWndDlg,IDC_CANCELTRAN),SW_HIDE);
					goto EnableOpts;
            	}
            	break;
            	
            case IDC_EXPORTTOGPS:  
            	SetDlgItemText (hWndDlg,IDC_TRANSTATUS,"Attempting to open connection to GPS");
				if (GPSOpen(hWndDlg,Identity,0,0,0,TRUE)>0)
				{   
					short	n;
            		short	nItems=SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETSELCOUNT,0,0); 
					
/*					if (nItems > MaxGPSWaypoints)
					{
						MessageBox (hWndDlg,"You have selected more waypoints than your GPS receiver can hold"," ",MB_OK);
						break;
					}*/
					sprintf (str,"GPS Functions: Connected to %s",Identity);
					SetWindowText (hWndDlg,str);
					ShowWindow (GetDlgItem(hWndDlg,IDC_TRANSTATUS),SW_SHOW);
					ShowWindow (GetDlgItem(hWndDlg,IDC_CANCELTRAN),SW_SHOW);  
            		n = GPSExportWP (hWndDlg,IDC_GPSLIST,IDC_TRANSTATUS,SearchALL);
//					ShowWindow (GetDlgItem(hWndDlg,IDC_TRANSTATUS),SW_HIDE);
					ShowWindow (GetDlgItem(hWndDlg,IDC_CANCELTRAN),SW_HIDE);
            		sprintf (str,"%i points transferred",n);
					MessageBox (hWndDlg,str," ",MB_OK);
            	}
            	break;
            	
            case IDC_EXPORTROUTE:  
            	SetDlgItemText (hWndDlg,IDC_TRANSTATUS,"Attempting to open connection to GPS");
				if (GPSOpen(hWndDlg,Identity,0,0,0,TRUE)>0)
				{   
					short	n;
					
					sprintf (str,"GPS Functions: Connected to %s",Identity);
					SetWindowText (hWndDlg,str);
					ShowWindow (GetDlgItem(hWndDlg,IDC_TRANSTATUS),SW_SHOW);
					ShowWindow (GetDlgItem(hWndDlg,IDC_CANCELTRAN),SW_SHOW);  
            		n = GPSExportRoute (hWndDlg,IDC_GPSLIST,IDC_TRANSTATUS,SearchALL);
//					ShowWindow (GetDlgItem(hWndDlg,IDC_TRANSTATUS),SW_HIDE);
					ShowWindow (GetDlgItem(hWndDlg,IDC_CANCELTRAN),SW_HIDE);
            		sprintf (str,"Route transferred");
					if (n)
						MessageBox (hWndDlg,str," ",MB_OK);
            	}
            	break;
            	
            case IDC_CONFIGGPS:
            {
				DLGPROC lpfnGPSCONFIGMsgProc; 
				short	nRc;
				 
				lpfnGPSCONFIGMsgProc = MakeProcInstance((DLGPROC)GPSCONFIGMsgProc, hInst);
				nRc = DialogBox(hInst, (LPSTR)"GPSCONFIG", hWndDlg, lpfnGPSCONFIGMsgProc);
				FreeProcInstance(lpfnGPSCONFIGMsgProc);   
				goto SetGPS;
			}
            	break;	 
           }
    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL FAR PASCAL FISHMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	short	Default;
	char	str[512];  
	short	Choice;
	static	short ft=0,fs=0,fn=0,ca=0,lb=0,fl=0;
	LPSTR	pBeg, pEnd;    
	static	BOOL	First;
	
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
    	 First = TRUE;
    case GSSI_REINITDIALOG:  
    	 SetGlobalValueLong ("FISHTYPE",(long)ft);  
    	 fn=0;
         Default = FillCBList (hWndDlg,IDC_FISHTYPE,"[%DL]waypopts\\typefish.txt",ft,0);
         Default = FillCBList (hWndDlg,IDC_NUMFISH,"[%DL]waypopts\\numfish.txt",fn,0);
         Default = FillCBList (hWndDlg,IDC_FISHSIZE,"[%DL]waypopts\\size[FISHTYPE].txt",fs,0);
         Default = FillCBList (hWndDlg,IDC_CAUGHTUSING,"[%DL]waypopts\\tackle.txt",ca,0);
         Default = FillCBList (hWndDlg,IDC_LIVEBAIT,"[%DL]waypopts\\livebait.txt",lb,0);
         Default = FillCBList (hWndDlg,IDC_FISHLEN,"[%DL]waypopts\\lengths.txt",lb,0);
         if (!First)
         	break;
         First = FALSE;
		 Choice=SendDlgItemMessage(hWndAddWaypoint,IDC_FISHLIST,LB_GETCURSEL,0,0); 
         if (Choice >= 0)
         {  
         	pBeg = str;
         	SendDlgItemMessage (hWndAddWaypoint,IDC_FISHLIST,LB_GETTEXT,Choice,(LPARAM)str); 
     	 	pEnd = _fstrchr (str,'\t');
     	 	*pEnd++ = 0; 
     	 	ft = GetListNum ("[%DL]waypopts\\typefish.txt",pBeg);
			SendDlgItemMessage(hWndDlg,IDC_FISHTYPE,CB_SETCURSEL,ft,0);
	    	SetGlobalValueLong ("FISHTYPE",(long)ft);
         	FillCBList (hWndDlg,IDC_FISHSIZE,"[%DL]waypopts\\size[FISHTYPE].txt",fs,0);
			pBeg = pEnd; 
     	 	pEnd = _fstrchr (pEnd,'\t');
     	 	*pEnd++ = 0;
     	 	Choice = GetListNum ("[%DL]waypopts\\numfish.txt",pBeg);
			SendDlgItemMessage(hWndDlg,IDC_NUMFISH,CB_SETCURSEL,Choice,0);
	    	pBeg = pEnd;  
     	 	pEnd = _fstrchr (pEnd,'\t');
     	 	*pEnd++ = 0;
     	 	Choice = GetListNum ("[%DL]waypopts\\size[FISHTYPE].txt",pBeg);
			SendDlgItemMessage(hWndDlg,IDC_FISHSIZE,CB_SETCURSEL,Choice,0);
			pBeg = pEnd; 
     	 	pEnd = _fstrchr (pEnd,'\t');
     	 	*pEnd++ = 0;
     	 	Choice = GetListNum ("[%DL]waypopts\\lengths.txt",pBeg);
			SendDlgItemMessage(hWndDlg,IDC_FISHLEN,CB_SETCURSEL,Choice,0);
	    	pBeg = pEnd;  
     	 	pEnd = _fstrchr (pBeg,'\t');
     	 	*pEnd++ = 0; 
     	 	Choice = GetListNum ("[%DL]waypopts\\tackle.txt",pBeg);
			SendDlgItemMessage(hWndDlg,IDC_CAUGHTUSING,CB_SETCURSEL,Choice,0);
	    	pBeg = pEnd;  
     	 	pEnd = _fstrchr (pBeg,'\t');
     	 	*pEnd++ = 0; 
     	 	Choice = GetListNum ("[%DL]waypopts\\livebait.txt",pBeg);
			SendDlgItemMessage(hWndDlg,IDC_LIVEBAIT,CB_SETCURSEL,Choice,0);
     	 	SetDlgItemText (hWndDlg,IDC_FISHCOMMENT,pEnd);
         }
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {

           	case IDC_FISHTYPE:
              	switch(HIWORD(wParam))
                {
		             case CBN_SELCHANGE:
		                 ft=SendDlgItemMessage(hWndDlg,IDC_FISHTYPE, CB_GETCURSEL,0,0);
						 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
					 break;
				}
			break;
			
            case IDOK:
                ft=(short)SendDlgItemMessage(hWndDlg,IDC_FISHTYPE,CB_GETCURSEL,0,0); 
                fs=(short)SendDlgItemMessage(hWndDlg,IDC_FISHSIZE,CB_GETCURSEL,0,0); 
                fl=(short)SendDlgItemMessage(hWndDlg,IDC_FISHLEN,CB_GETCURSEL,0,0); 
                fn=(short)SendDlgItemMessage(hWndDlg,IDC_NUMFISH,CB_GETCURSEL,0,0); 
                ca=(short)SendDlgItemMessage(hWndDlg,IDC_CAUGHTUSING,CB_GETCURSEL,0,0); 
                lb=(short)SendDlgItemMessage(hWndDlg,IDC_LIVEBAIT,CB_GETCURSEL,0,0); 
            	GetDlgItemText (hWndDlg,IDC_FISHTYPE,str,256);
            	_fstrcat (str,"\t");              
            	GetDlgItemText (hWndDlg,IDC_NUMFISH,_fstrchr(str,0),100);
            	_fstrcat (str,"\t");
            	GetDlgItemText (hWndDlg,IDC_FISHSIZE,_fstrchr(str,0),100);
            	_fstrcat (str,"\t");
            	GetDlgItemText (hWndDlg,IDC_FISHLEN,_fstrchr(str,0),100);
            	_fstrcat (str,"\t");
            	GetDlgItemText (hWndDlg,IDC_CAUGHTUSING,_fstrchr(str,0),100);
            	_fstrcat (str,"\t");
            	GetDlgItemText (hWndDlg,IDC_LIVEBAIT,_fstrchr(str,0),100);
            	_fstrcat (str,"\t");
            	GetDlgItemText (hWndDlg,IDC_FISHCOMMENT,_fstrchr(str,0),100);
				Choice=SendDlgItemMessage(hWndAddWaypoint,IDC_FISHLIST,LB_GETCURSEL,0,0); 
				if (Choice >= 0)
				{
				 	SendDlgItemMessage (hWndAddWaypoint,IDC_FISHLIST,LB_DELETESTRING,Choice,(LPARAM)0); 
				 	SendDlgItemMessage (hWndAddWaypoint,IDC_FISHLIST,LB_INSERTSTRING,Choice,(LPARAM)str); 
				}
				else
					SendDlgItemMessage (hWndAddWaypoint,IDC_FISHLIST,LB_ADDSTRING,Choice,(LPARAM)str); 
				
                EndDialog(hWndDlg, TRUE);
            	break;
            	 
            case IDCANCEL:
                EndDialog(hWndDlg, FALSE);
                break; 
                 
           }
    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL GPSCONFIGMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
    short	Choice;
	char	str[128];   
	int		ii;
		
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
    	GetGPSModels (hWndDlg,IDC_GPSMODEL);
    	GetGPSBaud (hWndDlg,IDC_GPSBAUD);
    	GetGPSPorts (hWndDlg,IDC_GPSPORT);
		GetPrivateProfileString ("GPS","Model","",str,100,GMIni); 
    	SendDlgItemMessage (hWndDlg,IDC_GPSMODEL,CB_SELECTSTRING,-1,(LPARAM)str);
       	cwCenter(hWndDlg, 0);  
		GPSFormat = GetGPSFormat (str); 
	  	if (GPSFormat == 3000) 
	  	{
	  		ShowWindow (GetDlgItem(hWndDlg,IDC_SEARCHALL),SW_SHOW);
	  		ShowWindow (GetDlgItem(hWndDlg,IDC_DELETEALLWP2),SW_SHOW);
	  	}
	  	else if (GPSFormat == 5000) 
	  	{
	  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSPORT),SW_HIDE);
	  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSPORTTITLE),SW_HIDE);
	  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSBAUD),SW_HIDE);
	  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSBAUDTITLE),SW_HIDE);   
	  		ShowWindow (GetDlgItem(hWndDlg,IDC_DISPLAYNMEA),SW_HIDE);   
	  	}
	  	else 
	  	{
	  		ShowWindow (GetDlgItem(hWndDlg,IDC_SEARCHALL),SW_HIDE);
	  		ShowWindow (GetDlgItem(hWndDlg,IDC_DELETEALLWP2),SW_HIDE);
	  	}
		GetPrivateProfileString ("GPS","Port","",str,100,GMIni); 
    	SendDlgItemMessage (hWndDlg,IDC_GPSPORT,CB_SELECTSTRING,-1,(LPARAM)str); 
    	if (GPSFormat != 5000)
    	{ 
			if (_fstricmp (str,"USB"))
	    	{
		  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSBAUDTITLE),SW_SHOW);
		  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSBAUD),SW_SHOW);
		  	}
		  	else 
		  	{
		  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSBAUDTITLE),SW_HIDE);
		  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSBAUD),SW_HIDE);
		  	} 
		}
		GetPrivateProfileString ("GPS","Baud","",str,100,GMIni); 
    	SendDlgItemMessage (hWndDlg,IDC_GPSBAUD,CB_SELECTSTRING,-1,(LPARAM)str);
		GetPrivateProfileString ("GPS","DisplayNMEA","Y",str,2,GMIni);
		 if (*str == 'Y')
			SendDlgItemMessage (hWndDlg, IDC_DISPLAYNMEA,BM_SETCHECK,TRUE,0);  
		GetPrivateProfileString ("GPS","SearchALL","Y",str,2,GMIni);
		 if (*str == 'Y')
			SendDlgItemMessage (hWndDlg, IDC_SEARCHALL,BM_SETCHECK,TRUE,0);  
         break;  

    case WM_CLOSE:
          
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break;  

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {

			case IDC_DELETEALLWP2:
				if (GPSOpen(hWndDlg,str,0,0,0,TRUE)>0)
					ScanLowrance (2);
			break;

			case IDC_GPSMODEL:
                switch(HIWORD(wParam))
                    {
                     case CBN_SELCHANGE:
						Choice=SendDlgItemMessage(hWndDlg,IDC_GPSMODEL,
											    	CB_GETCURSEL,0,0); 
					  	SendDlgItemMessage(hWndDlg,IDC_GPSMODEL,CB_GETLBTEXT,
					  					 	Choice,(DWORD)&str);   
						GPSFormat = GetGPSFormat (str); 
				  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSPORT),SW_SHOW);
				  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSPORTTITLE),SW_SHOW);
				  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSBAUD),SW_SHOW);
				  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSBAUDTITLE),SW_SHOW);
				  		ShowWindow (GetDlgItem(hWndDlg,IDC_DISPLAYNMEA),SW_SHOW);   
					  	if (GPSFormat == 3000) 
					  	{
					  		ShowWindow (GetDlgItem(hWndDlg,IDC_SEARCHALL),SW_SHOW);
					  		ShowWindow (GetDlgItem(hWndDlg,IDC_DELETEALLWP2),SW_SHOW);
					  	}
					  	else if (GPSFormat == 5000) 
					  	{
					  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSPORT),SW_HIDE);
					  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSPORTTITLE),SW_HIDE);
					  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSBAUD),SW_HIDE);
					  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSBAUDTITLE),SW_HIDE);
					  		ShowWindow (GetDlgItem(hWndDlg,IDC_DISPLAYNMEA),SW_HIDE);   
					  	}
					  	else 
					  	{
					  		ShowWindow (GetDlgItem(hWndDlg,IDC_SEARCHALL),SW_HIDE);
					  		ShowWindow (GetDlgItem(hWndDlg,IDC_DELETEALLWP2),SW_HIDE);
					  	}
					  	break;
					}
				break;
			
			case IDC_GPSPORT:
                switch(HIWORD(wParam))
                    {
                     case CBN_SELCHANGE:                                        
						Choice=SendDlgItemMessage(hWndDlg,IDC_GPSPORT,
											    	CB_GETCURSEL,0,0); 
					  	SendDlgItemMessage(hWndDlg,IDC_GPSPORT,CB_GETLBTEXT,
					  					 	Choice,(DWORD)&str);   
				    	if (_fstricmp (str,"USB"))
				    	{
					  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSBAUDTITLE),SW_SHOW);
					  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSBAUD),SW_SHOW);
					  	}
					  	else 
					  	{
					  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSBAUDTITLE),SW_HIDE);
					  		ShowWindow (GetDlgItem(hWndDlg,IDC_GPSBAUD),SW_HIDE);
					  	}
				
					  	break;
					}
				break;
			case IDC_SEARCHFORGPS: 
			{
				short	iport; 
				long	SaveTO = GPSTimeOut;
				short	rtn; 
				char	Port[8], Identity[100]; 
				HCURSOR	hcurSave;
				
				SetDlgItemText (hWndDlg,IDC_SEARCHMESS,"");
				GPSTimeOut = GetGlobalLVal2 ("[%GPSSearchTimeOut]",1000); 
				hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
				if (OpenGarminUSB (Identity))
				{   
					_fstrcpy (Port,"USB");
                	sprintf (str,"Found %s on USB",Identity,Port);
                	SetDlgItemText (hWndDlg,IDC_SEARCHMESS,str); 
					CloseGarminUSB (0);
			    	if (SendDlgItemMessage (hWndDlg,IDC_GPSMODEL,CB_SELECTSTRING,-1,(LPARAM)Identity) == CB_ERR)
			    	{   
			    		sprintf (str,"%s|%i",Identity, 1000);
			    		AppendFile (GPSUnitsFile,str);
				    	GetGPSModels (hWndDlg,IDC_GPSMODEL);
			    		SendDlgItemMessage (hWndDlg,IDC_GPSMODEL,CB_SELECTSTRING,-1,(LPARAM)Identity);
			    	}
			    	SendDlgItemMessage (hWndDlg,IDC_GPSPORT,CB_SELECTSTRING,-1,(LPARAM)Port);
                }
				else 
				{
					Processing = TRUE;
					for (iport=1;iport<19;iport++)
					{
						char	Type[3][10]={"Garmin","Magellan","Lowrance"};
						char	Baud[4][6]={"2400","4800","9600","19200"};
						char	PortC[8];
						int		j,k,ii;
						HANDLE	Stream;
						DCB		SaveCommState;
						
						sprintf (Port,"COM%i",iport); 
						sprintf (PortC,"COM%i",iport); 
						if ((Stream = OpenComm( Port, 4096, 256)) != (HANDLE)-1) 
						{	
							BOOL	HaveModem;
											 
	                    	ii=GetCommState (Stream,&SaveCommState);  
	                    	HaveModem = FoundModem (Stream);
						    ii=CloseComm (Stream); 
						    if (HaveModem)
						    	continue;
	                    }
	                    else
	                    	continue;	
						if ((Stream = OpenSIOConnection(0 ,Port,0,FALSE,0,0,FALSE)) != (HANDLE)-1)
						{     
						    ii=CloseSIOConnection(Stream);
						    for (j=0;j<3;j++)
						    {
						      	for (k=0;k<4;k++)
						      	{   
						      		if (!ContinueProcessing)
						      		{ 
						      Can:  
	                                	SetDlgItemText (hWndDlg,IDC_SEARCHMESS,"Search Canceled");
	                                	goto GotGPS;
	                                }
									GPSTimeOut = 2000;
						      		sprintf (str,"Searching for %s on %s at %s baud",Type[j],PortC,Baud[k]);
		      						SetDlgItemText (hWndDlg,IDC_SEARCHMESS,str);
	                                rtn = GPSOpen (hWndDlg,Identity,(j+1)*1000,Port,Baud[k],FALSE);
	                                if (rtn >0)
	                                {   
	                                	sprintf (str,"Found %s on %s at %s baud",Identity,PortC,Baud[k]);
	                                	SetDlgItemText (hWndDlg,IDC_SEARCHMESS,str); 
	                                	GPSClose (&GPSStream); 
								    	if (SendDlgItemMessage (hWndDlg,IDC_GPSMODEL,CB_SELECTSTRING,-1,(LPARAM)Identity) == CB_ERR)
								    	{   
								    		sprintf (str,"%s|%i",Identity, (j+1)*1000);
								    		AppendFile (GPSUnitsFile,str);
									    	GetGPSModels (hWndDlg,IDC_GPSMODEL);
								    		SendDlgItemMessage (hWndDlg,IDC_GPSMODEL,CB_SELECTSTRING,-1,(LPARAM)Identity);
								    	}
								    	SendDlgItemMessage (hWndDlg,IDC_GPSPORT,CB_SELECTSTRING,-1,(LPARAM)Port);
								    	SendDlgItemMessage (hWndDlg,IDC_GPSBAUD,CB_SELECTSTRING,-1,(LPARAM)Baud[k]);
	                                	
	                                	goto GotGPS;
	                                } 
	                                else if (rtn < 0)
	                                	goto Can;
	                                	
						      	}
						    }
						}
						//SetCommState (&SaveCommState);
					}
					SetDlgItemText (hWndDlg,IDC_SEARCHMESS,"Unable to detect GPS");
				}
		GotGPS:  
				Processing = FALSE;
				SetContinueProcessing ( TRUE);
				GPSTimeOut=SaveTO; 
				GSSiSetCursor (hcurSave); 
			}
				break;
				  	
            case IDOK:
            	GetDlgItemText (hWndDlg,IDC_GPSMODEL,str,100);
				WritePrivateProfileString ("GPS","Model",str,GMIni); 
            	GetDlgItemText (hWndDlg,IDC_GPSPORT,str,100);
				WritePrivateProfileString ("GPS","Port",str,GMIni); 
            	GetDlgItemText (hWndDlg,IDC_GPSBAUD,str,100);
				WritePrivateProfileString ("GPS","Baud",str,GMIni); 
				WritePrivateProfileString ("GPS","FlowControl","None",GMIni);  
		        if (SendDlgItemMessage (hWndDlg,IDC_DISPLAYNMEA,BM_GETCHECK,0,0)) 
		        	_fstrcpy (str,"Y");
		        else
		        	_fstrcpy (str,"N");
				WritePrivateProfileString ("GPS","DisplayNMEA",str,GMIni);  
		        if (SendDlgItemMessage (hWndDlg,IDC_SEARCHALL,BM_GETCHECK,0,0)) 
		        	_fstrcpy (str,"Y");
		        else
		        	_fstrcpy (str,"N");
				WritePrivateProfileString ("GPS","SearchALL",str,GMIni);  
				
                EndDialog(hWndDlg, TRUE);
            	break;
            	 
            case IDCANCEL:  
            	if (Processing)
            		SetContinueProcessing (FALSE);
            	else
	                EndDialog(hWndDlg, FALSE);
                break; 
                 
           }
    default:
        return FALSE;
   }
   
 return TRUE;
} 


BOOL GetLatText (LPSTR Lat,double lat)
{
	short	Deg, Min, ndp=1;
	double	Sec;
	
	GetDMS (lat,&Deg,&Min,&Sec); 
	if (DMSFormat == 1)
		sprintf (Lat,"N%2.2iD %2.2iM %.*fS",abs(Deg),Min,ndp,Sec); 
	else
		sprintf (Lat,"N%2.2iD %6.3fM",abs(Deg),Min + Sec/60);
    
	return TRUE;
} 

BOOL GetLonText (LPSTR Lon, double lon)
{
	short	Deg, Min, ndp=1;
	double	Sec;

	GetDMS (lon,&Deg,&Min,&Sec); 
	if (DMSFormat == 1)
		sprintf (Lon,"W%2.2iD %2.2iM %.*fS",abs(Deg),Min,ndp,Sec); 
	else
		sprintf (Lon,"W%2.2iD %6.3fM",abs(Deg),Min + Sec/60);
    
	return TRUE;
}

BOOL AddRouteToGPSList (long RouteRef)
{   
	OFSTRUCTGM	OFStruct;
	HFILE		Fid;  
	long		npnts;
	long		i;
	char		str[256], Lat[32],Long[32],Depth[128], File[144],WPID[32], Desc[32],Icon[16],RouteCode[4]=""; 
	DPOINT		LatLong;
	HANDLE		hSurf=0, hPoly;
	DPOINT		DPoint;  
	HPDPOINT	pDPoint;
	double		Elev; 
	short		PickFile;

	GetGlobalCVal ("[%FILEGPS]",File,DefaultGPSFile);
	
	Fid = GSSiOpenFile (File,&OFStruct,OF_READWRITE);
    if (Fid == HFILE_ERROR)
    {
		Fid = GSSiOpenFile (File,&OFStruct,OF_CREATE_NODELETE); 
		if (Fid == HFILE_ERROR)
		{
			MessageBox (GetFocus(),"Unable to create GPS transfer file",File,MB_ICONEXCLAMATION);
			return FALSE;
		}
		fputstring ("Default GPS transfer file",Fid);
    }
	GSSillseek (Fid,0,2); 
    	
	PickFile = GetPickFile (-1);
	if (!PickByRefno(RouteRef,0,0,PickFile))
		return FALSE; 
	if (!GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&npnts,&hPoly))
		return FALSE; 
	if (!GetTextString (0,RouteCode,2,"Enter a 2 charater code to uniquely identify this route",0,0,0,TRUE,TRUE))
		return FALSE;
	pDPoint = (HPDPOINT)GlobalLock (hPoly);
	for (i=0;i<npnts;i++,pDPoint++) 
	{
		sprintf (WPID,"RT%s%2.2ld",RouteCode,i+1);
		LatLong = *pDPoint;
		ConvertCoord (&LatLong,1,2);  
		GetLatText (Lat,LatLong.y); 
		GetLonText (Long,LatLong.x);   
		*Icon = 0;
    	*Depth = 0;  
    	*Desc = 0;
		sprintf (str,"%.12s\t%.8s\t%.7s\t%.30s\t%s\t%s\t%ld",WPID,Icon,Depth,Desc,Lat,Long,-(i+10)); //added 10 to ref to prevent problems with track route (-1)
		fputstring (str,Fid); 
	}
	GSSiGlobUlFree (&hPoly);
	DTMClose (&hSurf);
	GSSiClose (Fid);
	return TRUE;
}

BOOL AddWPToGPSList (LPSTR WPID,long Refno,LPSTR SymName,LPDPOINT pPoint) 
{   
	HANDLE	hDBWP;
    LPGWDHEADER lpGWDHead; 
	LPWAYPOINTDATA	pWPData; 
	OFSTRUCTGM	OFStruct;
	HFILE		Fid;  
	long		Offset;
	char		str[256], Lat[32],Long[32],Depth[128], File[144]; 
	DPOINT		LatLong;
	HANDLE		hSurf=0;
	DPOINT	DPoint;
	double	Elev; 

	GetGlobalCVal ("[%FILEGPS]",File,DefaultGPSFile);
	
	Fid = GSSiOpenFile (File,&OFStruct,OF_READWRITE);
    if (Fid == HFILE_ERROR)
    {
		Fid = GSSiOpenFile (File,&OFStruct,OF_CREATE_NODELETE); 
		if (Fid == HFILE_ERROR)
		{
			MessageBox (GetFocus(),"Unable to create GPS transfer file",File,MB_ICONEXCLAMATION);
			return FALSE;
		}
		fputstring ("Default GPS transfer file",Fid);
    }
	GSSillseek (Fid,0,2); 
    if (!_fstrcmp (WPID,"SPORTMAP"))
    {   
    	short	PickFile;

		PickFile = GetPickFile (-1);
		if (!PickByRefno(Refno,0,0,PickFile))
			return FALSE; 
		WPID = PickList[0].UDI;
		DPoint = LatLong = PickList[0].BeginPoint; 
		ConvertCoord (&LatLong,1,2); 
		GetLatText (Lat,LatLong.y);
		GetLonText (Long,LatLong.x);
		GetGlobalCVal ("[%DTMNAME]",str,"");
        hSurf = DTMOpen (str,DBL_MAX,BT_READ,0);
	    Elev = NGIELV (DPoint,hSurf,0);
	    if (Elev < DBL_MAX)
	    	sprintf (Depth,"%.0f",Elev);	
	    else
	    	*Depth = 0; 
		WPID[10]=0;
		PadString (WPID,' ',10);
		sprintf (str,"%s\t\t%s\t\t%s\t%s\t%ld",WPID,Depth,Lat,Long,Refno);
		fputstring (str,Fid); 
		DTMClose (&hSurf);
		GSSiClose (Fid);
	}
    else if (!_fstrcmp (WPID,"AREA"))
    {   
    	HPDPOINT	Point;
    	long	npnts;
    	HANDLE	hPoly;
    	char	PointID[12]; 
    	short	i;

		if (!PickByRefno(Refno,0,0,-1))
			return FALSE; 
		if (!GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&npnts,&hPoly))
			return FALSE;
		Point = (HPDPOINT)GlobalLock (hPoly);
		for (i=0;i<npnts;i++)
		{
			sprintf (PointID,"AR%4.4i",i+1);
			DPoint = LatLong = Point[i]; 
			ConvertCoord (&LatLong,1,2); 
			GetLatText (Lat,LatLong.y);
			GetLonText (Long,LatLong.x);
	    	*Depth = 0; 
			PointID[10]=0;
			PadString (PointID,' ',10);
			sprintf (str,"%s\t\t%s\t\t%s\t%s\t%ld",PointID,Depth,Lat,Long,i+1);
			fputstring (str,Fid);
		} 
		GSSiClose (Fid);   
		GSSiGlobUlFree (&hPoly);
	}
    else if (Refno>0)
    {
		hDBWP = OpenGWDatabase (WayPointGMD,BT_READ);
		if (hDBWP)
		{
			 lpGWDHead = (LPGWDHEADER)GlobalLock (hDBWP); 
			 pWPData = (LPWAYPOINTDATA)&lpGWDHead->GWDData;
			 if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Refno,BT_FIRST,BT_EQ, (LPSTR)&Offset))
			 {  
			 	char	WPID2[18];
			 	
			 	FillGWDData (lpGWDHead,Offset); 
			 	_fstrcpy (WPID2,pWPData->WPName); 
			 	_fstrcpy (WPID,pWPData->WPName); 
			 	GetLatText (Lat,pWPData->LatLong.y);
			 	GetLonText (Long,pWPData->LatLong.x); 
	       		sprintf (Depth,"$DECODE([%DL]waypopts\\depths.txt,%i)",pWPData->Depth);
	       		ExpandText (Depth);
				WPID2[10]=0;
				PadString (WPID2,' ',10);
				sprintf (str,"%s\tFish\t%s\t%s\t%s\t%s\t%ld",WPID2,Depth,pWPData->Comment,Lat,Long,Refno);
				fputstring (str,Fid); 
			 }
			 GlobalUnlock (hDBWP);  
			 CloseGWDatabase (hDBWP);
		}   
		GSSiClose (Fid);
		UpdateGPSWPDB (File);
	} 
	else
	{
		ConvertCoord (pPoint,1,2); 
		GetLatText (Lat,pPoint->y);
		GetLonText (Long,pPoint->x);
		sprintf (WPID,"MP%4.4ld",-Refno);   
		WPID[10]=0;
		PadString (WPID,' ',10);
		sprintf (str,"%s\t%s\t%s\tC\t%s\t%s\t%ld",WPID,SymName,SymName,Lat,Long,-Refno);
		fputstring (str,Fid); 
		GSSiClose (Fid);
		UpdateGPSWPDB (File);
	} 
	return TRUE;
}

   

BOOL WPInSelList (short FieldID,long Value)
{   
	short	Dummy;
	
	WPSelKey.FieldID = FieldID;
	WPSelKey.Value = Value; 
	if (!BT_FIND (hWPSelect,(LPSTR)&WPSelKey,BT_FIRST,BT_EQ,(LPSTR)&Dummy))
		return TRUE; 			  
	WPSelKey.FieldID = FieldID;
	WPSelKey.Value = -1; 
	if (BT_FIND (hWPSelect,(LPSTR)&WPSelKey,BT_FIRST,BT_GE,(LPSTR)&Dummy))
		return TRUE;
	if (WPSelKey.FieldID != FieldID)
		return TRUE;
	return FALSE;
}

BOOL WPInSelListR (short FieldID,long Value1,long Value2)
{   
	short	Dummy;
	
	WPSelKey.FieldID = FieldID;
	WPSelKey.Value = Value1; 
	while (WPSelKey.Value <= Value2)
	{
		if (!BT_FIND (hWPSelect,(LPSTR)&WPSelKey,BT_FIRST,BT_EQ,(LPSTR)&Dummy))
			return TRUE; 
		WPSelKey.Value++;
	}			  
	WPSelKey.FieldID = FieldID;
	WPSelKey.Value = -1; 
	if (BT_FIND (hWPSelect,(LPSTR)&WPSelKey,BT_FIRST,BT_GE,(LPSTR)&Dummy))
		return TRUE;
	if (WPSelKey.FieldID != FieldID)
		return TRUE;
	return FALSE;
}

 

HANDLE	CreateFoundWPDB (void)
{
 	int		i;
	BTVARDESC BTVar[2], *pVars;
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	long	TotFileLen;
	GWDHEADER16 GWDHead; 
	GWDHEADER GWDHead32;
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo;
	HANDLE hBT, hDB;
	HFILE	FidData;
	int		ibeg,NumVars;
	OFSTRUCTGM	OFStruct;
	GWFLDINFO FldInfo;
	char	File[128]; 
	LPSTR	lpDot;  
    
    _fstrcpy (File,SelWaypntDB);
    _fmemset (&GWDHead,0,sizeof(GWDHEADER16));

	 FidData = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=1;
	 GWDHead.Version=SELWAYPNTVERSION;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
	ibeg = 0;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"WAYPID");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 2;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"DEPTH");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"WPX");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_REAL;
	_fstrcpy (FldInfo.Name,"WPY");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 16;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"WPName");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 256;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"Text");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++; 
	
	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = 0;
	 GSSillseek (FidData,0,0);
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
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
     return hDB; 
} 

HANDLE	CreateGPSWPDB (void)
{   
 	int		i;
	BTVARDESC BTVar[2], *pVars;
	int		NumFields, Reclen, len;
	long	Refno, Offset;
	long	TotFileLen;
	GWDHEADER16 GWDHead; 
	GWFLDINFO GWFldInfo;
	LPGWFLDINFO	lpGWFldInfo;
	HANDLE hBT, hDB;
	HFILE	FidData;
	int		ibeg,NumVars;
	OFSTRUCTGM	OFStruct;
	GWFLDINFO FldInfo;
	char	File[]={"[%DL]attribut\\ingps.gmd"}; 
	LPSTR	lpDot;  

    _fmemset (&GWDHead,0,sizeof(GWDHEADER16));

	 FidData = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	 GWDHead.NumFields=0;
	 GWDHead.NumIndex=1;
	 GWDHead.Version=1;
	 GWDHead.NumIndexFields[0]=1;
	 GWDHead.IndexFields[0][0]=0;
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
	ibeg = 0;

	FldInfo.Len = 4;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_INTEGER;
	_fstrcpy (FldInfo.Name,"WAYPID");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"WPID");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	FldInfo.Len = 8;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"Symbol");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;
	
	FldInfo.Len = 16;
	FldInfo.Beg = ibeg;
	ibeg += FldInfo.Len;
	FldInfo.Type = BT_CHAR;
	_fstrcpy (FldInfo.Name,"WPName");
	BigWrite (FidData,(HPSTR)&FldInfo,sizeof(FldInfo),-1);
	GWDHead.NumFields++;

	 GWDHead.Reclen=ibeg; 
	 GWDHead.TimeStamp = 0;
	 GSSillseek (FidData,0,0);
	 BigWrite (FidData,(HPSTR)&GWDHead,sizeof(GWDHEADER16),-1);
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
     return hDB; 
} 

void UpdateGPSWPDB (LPSTR InFile)
{
	HANDLE	hDBGPS;
    LPGWDHEADER lpGWDHeadGPS; 
	LPWAYPOINTGPSDATA	pGPSWPData;  
	LPSTR	Symbol, CRefno;
	long	Refno;
	HFILE	Fid;
	char	str[260],File[MAX_PATH];    
	OFSTRUCTGM	OFStruct;
    
    if (InFile)
    	_fstrcpy (File,InFile);
    else
    	GetGlobalCVal ("[%FILEGPS]",File,DefaultGPSFile);
	hDBGPS = CreateGPSWPDB ();
	lpGWDHeadGPS = (LPGWDHEADER)GlobalLock (hDBGPS); 
	pGPSWPData = (LPWAYPOINTGPSDATA)&lpGWDHeadGPS->GWDData; 
	Fid = GSSiOpenFile (File,&OFStruct,OF_READ);
	if (Fid != HFILE_ERROR)
	{
		fgetstring (str,256,Fid);
 		while (fgetstring (str,256,Fid))
 		{   
 			if (*str)
 			{
	 			CRefno = _fstrrchr (str,'\t')+1;
	 			Refno = atol (CRefno); 
	 			Symbol = _fstrchr (str,'\t');
	 			*Symbol++ = 0; 
	 			_fstrcpy (pGPSWPData->WPName,str);
	 			pGPSWPData->Refno = Refno;
	 			_fstrncpy (pGPSWPData->ID,str,8);
	 			_fstrncpy (pGPSWPData->Symbol,Symbol,8);
	 			GWDAddRecord (lpGWDHeadGPS,0,0); 
	 		} 
 		}
		GSSiClose (Fid);
	} 
	GlobalUnlock (hDBGPS);  
    CloseGWDatabase (hDBGPS); 
    return;
}
            
void SetWPDisplayOpts (HWND hWndDlg,short DisplayOpt)
{ 
    
	SendDlgItemMessage (hWndDlg, IDC_DISPLAYALLWP,BM_SETCHECK,FALSE,0);  
	SendDlgItemMessage (hWndDlg, IDC_DISPLAYSELECTEDWP,BM_SETCHECK,FALSE,0);  
	SendDlgItemMessage (hWndDlg, IDC_DISPLAYONLYGPS,BM_SETCHECK,FALSE,0);  
	switch (DisplayOpt)
	{
		case 0:
			SendDlgItemMessage (hWndDlg, IDC_DISPLAYALLWP,BM_SETCHECK,TRUE,0);  
			break;
		case 1:
			SendDlgItemMessage (hWndDlg, IDC_DISPLAYSELECTEDWP,BM_SETCHECK,TRUE,0);  
			break;
		case 2: 
			SendDlgItemMessage (hWndDlg, IDC_DISPLAYONLYGPS,BM_SETCHECK,TRUE,0);  
//			SendDlgItemMessage (hWndDlg, IDC_LABID,BM_SETCHECK,TRUE,0);  
//			SendDlgItemMessage (hWndDlg, IDC_LABDEPTH,BM_SETCHECK,FALSE,0);  
//			SendDlgItemMessage (hWndDlg, IDC_LABLATLON,BM_SETCHECK,FALSE,0); 
			break;
	}
//    EnableWindow (GetDlgItem(hWndDlg,IDC_LABID),b);
//    EnableWindow (GetDlgItem(hWndDlg,IDC_LABDEPTH),b);
//    EnableWindow (GetDlgItem(hWndDlg,IDC_LABLATLON),b);
	return;
}

 

  

   

long GetLakeIDFromPick (DPOINT DPoint)
{
	char	PickFile[MAX_PATH];
	char	SavePickFile[MAX_PATH]="";  
	char	str[32];
    long	LakeID=0;   
    short	SavePP = PickPerim;
    
	GSSiGetTempFileName (0,"gmb",0,SavePickFile);
    SaveVisFile (SavePickFile,TRUE,"");
	if (LoadPickList ("[%DL]piklists\\lakes.pik"))
	{
    	{
    		short SaveMaxPick = MaxPick;
			
			UseUserPickAp =FALSE;
			SystemPickAp = 0;//-GetGlobalDVal2 ("[%MAXDISTTOLAKE]",100);	
			MaxPick=1;
			PickPerim = 0;  
			PickItems (CurView->hWnd,DPoint);  
			PickPerim = SavePP;
			UseUserPickAp =TRUE;  
			MaxPick = SaveMaxPick;
		}
		if (NumPicked) 
		{   
			double	dToLake = fabs (PickList[NumPicked-1].OffDist);
			
			if (dToLake <= GetGlobalDVal2 ("[%MAXDISTTOLAKE]",100))
			{
				ProcessPickedItem (NumPicked-1,FALSE);  
				GetGlobalCVal ("[%LAKENUMVAR]",str,"[LKNUM]");
				ExpandText (str); 
				LakeID = atol (str); 
			}
		}
	}
   	LoadPickList (SavePickFile);
   	GSSiRemove (SavePickFile);
   	return LakeID;
}                               

short GPSLoadToDB (HWND hWndDlg, UINT Control)
{
	char	str[256], cUID[32];	 
	short	nItems, nLoaded=0,nNotLoaded=0;
	HCURSOR	hcurSave=0;    
	HANDLE	hItems=0;
	LPINT	lpItems; 
	LPSTR	pLat,pLon, pTAB, pOpts, pSym, pDesc, pRef, pDepth;
	double	lat,lon;
	short	n, nRc;
	int		nRoutePoints=0;    
	BOOL	Err;
	DPOINT	DPoint; 
	LPVISLIST	SaveVis;  
	HANDLE	hRoutePoints=0;
	HPDPOINT	RoutePoints;
	long	Ref,ii;    
	BOOL	HaveWPInit = FALSE, IsSportMap = FALSE;
	
	AllowWPSkip = TRUE;  
	if (!_fstricmp (AppName,"SportMap"))
		IsSportMap = TRUE;
	nItems=nWPSelected=(short)SendDlgItemMessage(hWndDlg ,Control,LB_GETSELCOUNT,0,0); 
	hItems=GSSiGlobAlloc (1033,GMEM_MOVEABLE,nItems*4);
	lpItems=(LPINT) GlobalLock(hItems);
	SendDlgItemMessage(hWndDlg ,Control,LB_GETSELITEMS,nItems,(LPARAM)lpItems);
	GlobalUnlock (hItems);
	if (nItems == 1)
	{
		lpItems=  (LPINT) GlobalLock(hItems);
		SendDlgItemMessage (hWndDlg,Control,LB_GETTEXT,*lpItems,(LPARAM)str); 
		pTAB = _fstrchr (str,'\t');
		*pTAB++ = 0; 
		strcpy (LoadWPName,str);
		GlobalUnlock (hItems);
	}
	{
		DLGPROC lpfnLOADWAYPOINTSMsgProc;
		
		lpfnLOADWAYPOINTSMsgProc = MakeProcInstance((DLGPROC)LOADWAYPOINTSMsgProc, hInst);
		nRc = DialogBox(hInst, (LPSTR)"LOADWAYPOINTS", hWndDlg, lpfnLOADWAYPOINTSMsgProc);
		FreeProcInstance(lpfnLOADWAYPOINTSMsgProc);
		if (!nRc)
			goto Exit;
	}
	if (LoadWPAsRoute)
	{
		hRoutePoints = GSSiGlobAlloc (1032,GMEM_MOVEABLE,(long)nItems*sizeof(DPOINT));
		RoutePoints = (HPDPOINT)GlobalLock (hRoutePoints);
	}
	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
	lpItems=  (LPINT) GlobalLock(hItems);
	n = nItems;  
	if (GetGlobalCVal ("[%LOADWPINIT]",str,0))
	{
		HaveWPInit = TRUE;
		ExpandText (str);
		if (*str != '1')
			SetContinueProcessing (FALSE);
	}

	while (ContinueProcessing && n--)
	{   
		SendDlgItemMessage (hWndDlg,Control,LB_GETTEXT,*lpItems++,(LPARAM)str); 
		Ref = 1;
		if ((pRef = _fstrrchr (str,'\t')))
		{   if (!isalpha (*(pRef+1)))
			{
				*pRef++ = 0;
				Ref = atol (pRef);
			}
		}
		pTAB = _fstrchr (str,'\t');
		*pTAB++ = 0; 
		if (*LoadWPName)
			SetGlobalValue ("WPNAME",LoadWPName);		
		else
			SetGlobalValue ("WPNAME",str);		
		pSym = pTAB;
		pTAB = _fstrchr (pSym,'\t');
		*pTAB++ = 0;
		pDepth = pTAB;
		pTAB = _fstrchr (pDepth,'\t');
		*pTAB++ = 0; 
		if (Ref != -1) //indicate track log
		{ 
			pDesc = pTAB;
			pTAB = _fstrchr (pDesc,'\t');
			*pTAB++ = 0; 
		}
		else
			ii=1;
		pLat = pTAB;
		pTAB = _fstrchr (pLat,'\t');
		*pTAB++ = 0;
		pLon = pTAB;
		lat = DecDegFromDMS (pLat,&Err); 
		lon = DecDegFromDMS (pLon,&Err); 
		DPoint.x = lon;
		DPoint.y = lat;
		ConvertCoord (&DPoint,2,1);  
		if (LoadWPAsRoute)
		{ 
			RoutePoints[nRoutePoints++] = DPoint;
		}
		else
		{
			SetViewport(*pCommandViewport);
			if (!IsSportMap && !HaveWPInit)
			{   
				short	idepth;  
				long	LakeID;
				
				SaveVis = CurVis;  
				SetGlobalValueLong ("DNRID",0); 
				SetGlobalValueLong ("NEWPOINTLAKENUM",0); 
				SetGlobalValue ("WPDESC",pDesc); 
				SetGlobalValue ("WPDEPTH",pDepth); 
				LakeID = GetLakeIDFromPick (DPoint);
				sprintf (str,"[NEWPOINTLAKENUM]=%ld",LakeID);
				ExpandText (str);
			}
		 	SetGlobalValueReal ("WAYPX",DPoint.x);
		 	SetGlobalValueReal ("WAYPY",DPoint.y); 
			SetViewport(*pCommandViewport);
			GetGlobalCVal ("[%LOADWPCOMMAND]",str,"$MACRO([%DL]macros\\waypoint.txt)");
			ExpandText (str);
			if (*str == '1')
				nLoaded++; 
			else
				nNotLoaded++;  
		}
	}
	SetContinueProcessing ( TRUE);
	GSSiGlobUlFree (&hItems);
	if (LoadWPAsRoute)
	{  
 		short	SymNum=0, NumSyms=0; 
 		HANDLE	hSymDesc=0, hTime=0; 
 		long	Refno; 
 		long	color= GetGlobalLVal ("[ROUTECOLOR]");   

		SetViewport(*pCommandViewport);
 		GlobalUnlock (hRoutePoints); 
 		ProcessText ("[%NEW_LINE_START_TIME]=[%SYS_CLOCK]");    
 		SetNewTime (2,&hTime);
		_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
		SymNum = GetOrCreateSym (hWndDlg,"ROUTE",&NumSyms,&hSymDesc,FALSE,0);
		Refno = GetNewRefno (EditName,0,0,0,0);
		AddToSymList (SymNum,&NumSyms,&hSymDesc); 
		_fstrcpy (PltName,EditName);
		PltType = 2;
		OpenMap (CurView->hWnd,CurView->hDC);
		EditBounds = CurView->FileMNMX; 
		CloseMap (FALSE);
		AddPolyToMap (1,&nRoutePoints, &hRoutePoints,1,Refno,hTime,-1,SymNum,0,"ROUTE",LoadWPName,-1,color,3,0,0,0,0,TRUE,0);
	    CloseMap(TRUE);  
		AddSymToMap (NumSyms,hSymDesc,0,0); 
        DestroySymList (&NumSyms,&hSymDesc);  
		GSSiGlobFree (&hRoutePoints);
		GSSiGlobFree (&hTime);
		nLoaded = -nRoutePoints;	
	}
Exit:
	GSSiGlobFree (&hItems);
	if (hcurSave)
		GSSiSetCursor (hcurSave); 
	PromptForWPData=TRUE; 
	AllowWPSkip = FALSE;
	return nLoaded;
}

BOOL SaveGPSList (HWND hWndDlg)
{   
	char	File[144], str[256];
	OFSTRUCTGM	OFStruct;
	HFILE	Fid=HFILE_ERROR; 
	short	Choice;
	
	GetDlgItemText (hWndDlg,IDC_FILENAME,File,sizeof(File));  
	_fullpath (str,File,sizeof(str));
	if (makedirectories (str,FALSE,FALSE))
		Fid = GSSiOpenFile (File,&OFStruct,OF_CREATE);
	if (Fid != HFILE_ERROR)
	{
		GetDlgItemText (hWndDlg,IDC_FILETITLE,str,sizeof(str)); 
		fputstring (str,Fid);
		Choice = 0;
 		while (SendDlgItemMessage (hWndDlg,IDC_GPSLIST,LB_GETTEXT,Choice++,(LPARAM)str) != LB_ERR)
 		{
 			fputstring (str,Fid);
 		}
		GSSiClose (Fid);  
		return TRUE;
	}                                           
	GSSiMessageBox (0,"Unable to create GPS file",File,MB_ICONEXCLAMATION,0);
	return FALSE;
}   

BOOL RemoveWPNameFromWPFile (LPSTR WPName)
{
	char	str[256];
	LPSTR	pTAB;  
	long	loc=0;
	HFILE	Fid;
	
	GetGlobalCVal ("[%FILEGPS]",str,DefaultGPSFile);
	Fid = GSSiOpenFile (str,0,OF_READWRITE);
	if (Fid == HFILE_ERROR)
	 	return FALSE;
	while (fgetstring (str,250,Fid))
	{   
		if ((pTAB = _fstrchr (str,'\t')))
		{
			*pTAB = 0;
					
			if (!_fstrcmp (str,WPName))
			{   
				GSSillseek (Fid,loc,0); 
				*str = 0;
				BigWrite (Fid,(HPSTR)str,1,-1);
				break; 
			}
	 	} 
	 	loc = GSSillseek (Fid,0,1);
	}
	GSSiClose (Fid);
	return TRUE;
}

void ConvertGPSLatLon (LPSTR Instr)
{   
	LPSTR pLat,pLon; 
	char	str[256], CLat[32],CLon[32];
	long	Refno=0; 
	double	lat,lon;  
	BOOL	Err;
	
	_fstrcpy (str,Instr);
Top:
	if (!(pLon = _fstrrchr (str,'\t')))
		return;
	*pLon++ = 0;
	if (IsInteger (pLon))
	{
		Refno= atol (pLon);
		goto Top;          
	}
	if (!(pLat = _fstrrchr (str,'\t')))
		return;
	*pLat++ = 0;
	lat = DecDegFromDMS (pLat,&Err); 
	lon = DecDegFromDMS (pLon,&Err); 
	GetLatText (CLat,lat); 
	GetLonText (CLon,lon); 
	sprintf (_fstrchr (str,0),"\t%s\t%s\t%ld",CLat,CLon,Refno);
	_fstrcpy (Instr,str);
	return;
}


 
BOOL RunSportMapStartupCommand (void)
{
	UINT	PrevErrMode = SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);
	int		i;
	char	VolLabel[32], str[260];

	if (_fstricmp (AppName,"SportMap"))
		goto Exit;
	for (i=3;i<26;i++)
	{   
		char	DriveID[2]={(char)('A'+i),0};	
		short DriveType = GetDriveType (DriveID);
		if (DriveType > 0)
		{   
			if (GetVolumeLabel(DriveID,VolLabel))
			{
				char	DriveID=(char)('A'+i);	
                HFILE	Fid;
				OFSTRUCTGM	OFStruct;
	            
	            if (!_fstrncmp (VolLabel,"SM",2))
	            {   
	            	if (!CDIsRegistered (VolLabel))
	            	{     
		            	sprintf (str,"%c:\\instalcd.txt",DriveID); 
		            	Fid = GSSiOpenFile(str,&OFStruct,OF_READ);
		            	if (Fid != HFILE_ERROR)
		            	{   
		            		sprintf (str,"%c:\\",DriveID);
		            		SetGlobalValue ("CDDRIVE",str);
		            		fgetstring (str,256,Fid);
		            		GSSiClose (Fid);
		            		ProcessText (str);  
		            	} 
		            }
	            }
            }
        }
    } 
Exit:
	SetErrorMode(PrevErrMode);
	return TRUE;
}
			
short GetWetlandDescription (LPSTR SymName,LPSTR CompressedCOW,LPSTR Output)
{
	char  Line[5][48];
	LPSTR pLoc;
	short nlines=0,i;
    
    if (_fstrlen(SymName) > 5)
    {
		pLoc = SymName + 5;
		i = atoi (pLoc);
		switch (i)
		{   
			case 1:
				_fstrcpy (Line[nlines],"Seasonally flooded basin or flat");
				break;
			case 2:
				_fstrcpy (Line[nlines],"Wet meadow");
				break;
			case 3:
				_fstrcpy (Line[nlines],"Shallow marsh");
				break;
			case 4:
				_fstrcpy (Line[nlines],"Deep marsh");
				break;
			case 5:
				_fstrcpy (Line[nlines],"Shallow open water");
				break;
			case 6:
				_fstrcpy (Line[nlines],"Shrub swamp");
				break;
			case 7:
				_fstrcpy (Line[nlines],"Wooded swamp");
				break;
			case 8:
				_fstrcpy (Line[nlines],"Bog");
				break;
			case 80:
				_fstrcpy (Line[nlines],"Municipal");
				break;
			case 90:
				_fstrcpy (Line[nlines],"River, creek or stream");
				break;
			default:
				nlines--;
		}
		nlines++;
	}

//	_fstrcpy (Line[nlines++],CompressedCOW);
		
//skip system and subsystem
	pLoc = CompressedCOW;  
NextClass:
	pLoc++;
	if (_fstrchr ("12346",*pLoc))
		pLoc++; 
	if (!_fstrncmp (pLoc,"RS",2))
		_fstrcpy (Line[nlines],"Rock");
	else if (!_fstrncmp (pLoc,"UB",2))
		_fstrcpy (Line[nlines],"Unconsolidated Bottom");  
	else if (!_fstrncmp (pLoc,"SB",2))
		_fstrcpy (Line[nlines],"Streambed");  
	else if (!_fstrncmp (pLoc,"AB",2))
		_fstrcpy (Line[nlines],"Aquatic bed");  
	else if (!_fstrncmp (pLoc,"RS",2))
		_fstrcpy (Line[nlines],"Rocky Shore");  
	else if (!_fstrncmp (pLoc,"US",2))
		_fstrcpy (Line[nlines],"Unconsolidated Shore");  
	else if (!_fstrncmp (pLoc,"EM",2))
		_fstrcpy (Line[nlines],"Emergent");  
	else if (!_fstrncmp (pLoc,"OW",2))
		_fstrcpy (Line[nlines],"Open Water/Unknown Bottom");  
	else if (!_fstrncmp (pLoc,"RB",2))
		_fstrcpy (Line[nlines],"Rock Bottom");  
	else if (!_fstrncmp (pLoc,"ML",2))
		_fstrcpy (Line[nlines],"Moss Lichen");  
	else if (!_fstrncmp (pLoc,"SS",2))
		_fstrcpy (Line[nlines],"Scrub Shrub");  
	else if (!_fstrncmp (pLoc,"FO",2))
		_fstrcpy (Line[nlines],"Forested");
	else
	{
		pLoc -= 2;
		nlines--;
	}  
    pLoc+=2; 
    nlines++;
    while (*pLoc)
    {
    	switch (*pLoc)
    	{
		 	case 'A':
		 		_fstrcpy(Line[nlines],"Temporarily flooded");
		 		nlines++;
		 		break;
		 	case 'B':
		 		_fstrcpy(Line[nlines],"Saturated");
		 		nlines++;
		 		break;
		 	case 'C':
		 		_fstrcpy(Line[nlines],"Seasonally flooded");
		 		nlines++;
		 		break;
		 	case 'D':
		 		_fstrcpy(Line[nlines],"Seasonally flooded/well drained");
		 		nlines++;
		 		break;
		 	case 'E':
		 		_fstrcpy(Line[nlines],"Seasonable flooded/saturated");
		 		nlines++;
		 		break;
		 	case 'F':
		 		_fstrcpy(Line[nlines],"Semi-permanently flooded");
		 		nlines++;
		 		break;
		 	case 'H':
		 		_fstrcpy(Line[nlines],"Permanently flooded");
		 		nlines++;
		 		break;
		 	case 'J':
		 		_fstrcpy(Line[nlines],"Intermittently flooded");
		 		nlines++;
		 		break;
		 	case 'K':
		 		_fstrcpy(Line[nlines],"Artificially flooded");
		 		nlines++;
		 		break;
		 	case 'W':
		 		_fstrcpy(Line[nlines],"Intermittently flooded/temporary");
		 		nlines++;
		 		break;
		 	case 'Y':
		 		_fstrcpy(Line[nlines],"Saturated/semi-permanent/seasonal");
		 		nlines++;
		 		break;
		 	case 'Z':
		 		_fstrcpy(Line[nlines],"Intermittently Exposed/permanent");
		 		nlines++;
		 		break;
		 	case 'b':
		 		_fstrcpy(Line[nlines],"Beaver");
		 		nlines++;
		 		break;
		 	case 'd':
		 		_fstrcpy(Line[nlines],"Partially drained/ditched");
		 		nlines++;
		 		break;
		 	case 'f':
		 		_fstrcpy(Line[nlines],"Framed");
		 		nlines++;
		 		break;
		 	case 'h':
		 		_fstrcpy(Line[nlines],"Diked/impounded");
		 		nlines++;
		 		break;
		 	case 'r':
		 		_fstrcpy(Line[nlines],"Artificially substrate");
		 		nlines++;
		 		break;
		 	case 's':
		 		_fstrcpy(Line[nlines],"Spoil");
		 		nlines++;
		 		break;
		 	case 'x':
		 		_fstrcpy(Line[nlines],"Excavated");
		 		nlines++;
		 		break;
		    case '/':
		    	goto NextClass;
		}
		pLoc++;
	} 
	*Output = 0;
	for (i=0;i<nlines;i++)
	{
		if (i)
			_fstrcpy (_fstrchr(Output,0),"\r\n");
		_fstrcpy (_fstrchr(Output,0),Line[i]);
	}
	return nlines;
} 

BOOL ConvertWaypointGMD (HWND hWnd)
{
    char	DefStr[1026];  
    short	CurrentVersion=2;  
    HANDLE	hWPOld, hWPNew;  
    LPGWDHEADER	lpGWDHeadOld, lpGWDHeadNew;
    LPWAYPOINTDATA	pWPDataOld, pWPDataNew; 
    long	Refno, Offset;
    short	pos = BT_FIRST,len;
    
    if (ExistFile (WayPointGMD))
    	return TRUE;
    
	if (!LoadString(hInst, IDS_WAYPOINTDEF, DefStr, 1024))
		return FALSE;
	if (!CreateGWDDatabase (WayPointGMD,CurrentVersion,FALSE,0,1,DefStr))
		return FALSE;
	hWPOld = OpenGWDatabase ("[%DL]attribut\\waypoint.gmd",BT_READ);
	if (!hWPOld) 
		return FALSE;
	lpGWDHeadOld = (LPGWDHEADER)GlobalLock (hWPOld); 
	pWPDataOld = (LPWAYPOINTDATA)&lpGWDHeadOld->GWDData; 
	hWPNew = OpenGWDatabase (WayPointGMD,BT_WRITE);
	if (!hWPNew) 
		return FALSE;
	lpGWDHeadNew = (LPGWDHEADER)GlobalLock (hWPNew); 
	pWPDataNew = (LPWAYPOINTDATA)&lpGWDHeadNew->GWDData; 

    while (!BT_FIND (lpGWDHeadOld->BTHandle[0],(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&Offset))
    {   
    	pos = BT_NEXT;  
    	len = FillGWDData (lpGWDHeadOld,Offset); 
    	*pWPDataNew = *pWPDataOld;
		sprintf (pWPDataNew->WPName,"WP%4.4ld",Refno);
		GWDReplaceRecord (lpGWDHeadNew,sizeof(WAYPOINTDATA),0,-1);
    }
	GlobalUnlock (hWPOld);
	GlobalUnlock (hWPNew);
	CloseGWDatabase (hWPOld);
	CloseGWDatabase (hWPNew);
	return TRUE;
} 

BOOL CreateWaypointGMD (HWND hWnd)
{
    char	DefStr[1026];  
    short	CurrentVersion=2;  
    
	if (!LoadString(hInst, IDS_WAYPOINTDEF, DefStr, 1024))
		return FALSE;
	if (!CreateGWDDatabase (WayPointGMD,CurrentVersion,FALSE,0,1,DefStr))
		return FALSE;
	return TRUE;
} 

BOOL CreateFishGMD (HWND hWnd)
{
    char	DefStr[1026];  
    short	CurrentVersion=2;  
    int		n = LoadString(hInst, IDS_FISHDEF, DefStr, 1024);

	if (!n)
		return FALSE;
	if (!CreateGWDDatabase (FishGMD,CurrentVersion,FALSE,0,2,DefStr))
		return FALSE;
	return TRUE;
} 

BOOL FAR PASCAL FINDWAYPOINTMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	short	Default, pos, st, DisplayOpt;
	char	str[256];  
	int   	Choice, ntab=4,TabStops[4]={114, 263, 1000,2000};
	LPSTR	pBeg, pEnd;    
    BTVARDESC  BTVar[2];
	static	BOOL	First;
	static	BOOL	LabelChanged;
	short	WPSelSort=0;
	LPFOUNDWPDATA	pFoundWPData; 
	LPGWDHEADER lpGWDHeadFound; 
	HANDLE	hDBFoundWP;
	static	long	WayPointID;
    LPGWDHEADER lpGWDHead,lpGWDHeadName,lpGWDHeadFish; 
	long	LakeID, Offset; 
	HANDLE	hDBWP,hDBName, hDBFish;
	LPWAYPOINTDATA	pWPData; 
	LPFISHDATA		pFishData; 
	struct {long	WayPoint;
			short	FishID;} FishKey; 
	
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
    	hWndFindWP = hWndDlg;  
    	LabelChanged = FALSE; 
		SetViewport(*pCommandViewport);
    	*(LPMNMXCORD)&UserBounds = CurView->WBounds;
//    	 ntab = loadtabs (TabStops);     
       	SendDlgItemMessage (hWndDlg,IDC_WPLIST,LB_SETTABSTOPS,ntab,(LPARAM)&TabStops);
		SendDlgItemMessage (hWndDlg, IDC_DISPLAYALLWP,BM_SETCHECK,TRUE,0);  
		SendDlgItemMessage (hWndDlg, IDC_LABID,BM_SETCHECK,GetGlobalBVal("[WPLabelID]"),0);  
		SendDlgItemMessage (hWndDlg, IDC_LABDEPTH,BM_SETCHECK,GetGlobalBVal("[WPLabelDepth]"),0);  
		SendDlgItemMessage (hWndDlg, IDC_LABLATLON,BM_SETCHECK,GetGlobalBVal("[WPLabelLatLon]"),0); 
		BTVar[0].BT_VARTYP=BT_INTEGER;
		BTVar[0].BT_VARLEN=2;
		BTVar[0].BT_VAROFF=0;
		BTVar[1].BT_VARTYP=BT_INTEGER;
		BTVar[1].BT_VARLEN=4;
		BTVar[1].BT_VAROFF=2;
		if (!*WPSelectFile)
			GSSiGetTempFileName (0,"gmf",0,(LPSTR)WPSelectFile);
		BT_CREATE (WPSelectFile, 2, FALSE, 2, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
		DisplayOpt = GetGlobalLVal2 ("[WPDisplayOpt]",1);
		SetWPDisplayOpts (hWndDlg, DisplayOpt);
    case GSSI_REINITDIALOG:
		SendDlgItemMessage(hWndDlg,IDC_WPLIST,LB_RESETCONTENT,0,0); 
		hDBFoundWP = OpenGWDatabase (SelWaypntDB,BT_READ);
		if (!hDBFoundWP)
			break;
		lpGWDHeadFound = (LPGWDHEADER)GlobalLock (hDBFoundWP); 
		if (lpGWDHeadFound->Version < SELWAYPNTVERSION)  
		{
			GlobalUnlock (hDBFoundWP);  
		    CloseGWDatabase (hDBFoundWP);   
		    GSSiRemove (SelWaypntDB);
		    break;
		}
		pFoundWPData = (LPFOUNDWPDATA)&lpGWDHeadFound->GWDData; 
		pos = BT_FIRST;
		while (!BT_FIND (lpGWDHeadFound->BTHandle[0],(LPSTR)&WayPointID,pos,BT_ANY, (LPSTR)&Offset))
		{   
			pos = BT_NEXT;
			FillGWDData (lpGWDHeadFound,Offset);
		 	SendDlgItemMessage (hWndDlg,IDC_WPLIST,LB_ADDSTRING,0,(LPARAM)pFoundWPData->Text); 
		}
		GlobalUnlock (hDBFoundWP);  
	    CloseGWDatabase (hDBFoundWP); 
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            
            case IDC_LABID:
            case IDC_LABLATLON:
            case IDC_LABDEPTH:
            	LabelChanged = TRUE;
            	break;
			case IDC_DISPLAYALLWP:
				SetWPDisplayOpts (hWndDlg, 0);
            	LabelChanged = TRUE;
            	break;
			case IDC_DISPLAYSELECTEDWP:
				SetWPDisplayOpts (hWndDlg, 1);
            	LabelChanged = TRUE;
            	break;
			case IDC_DISPLAYONLYGPS:
				SetWPDisplayOpts (hWndDlg, 2);
            	LabelChanged = TRUE;
            	break;
            	
            case IDC_SELALL:
				SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_SETSEL,TRUE,(LPARAM)-1); 
				goto CheckSel;
				
            case IDC_WPLIST: /* List box                           */
                 switch(HIWORD(wParam))
                 {   
                     case LBN_SELCHANGE:
               CheckSel:
                     { 
	            		short	nItems=SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETSELCOUNT,0,0); 
						
						switch (nItems)
						{
							case 0:
								EnableWindow (GetDlgItem(hWndDlg,IDC_GOTOWP),FALSE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_EDITWP),FALSE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_DELETEWP),FALSE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_GPSWP),FALSE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_EXPORTWP),FALSE);
							break;
							case 1:
								EnableWindow (GetDlgItem(hWndDlg,IDC_GOTOWP),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_EDITWP),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_DELETEWP),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_GPSWP),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_EXPORTWP),TRUE);
							break; 
							default:
								EnableWindow (GetDlgItem(hWndDlg,IDC_GOTOWP),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_EDITWP),FALSE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_DELETEWP),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_GPSWP),TRUE);
								EnableWindow (GetDlgItem(hWndDlg,IDC_EXPORTWP),TRUE);
						} 
					 }
                     break;
                         
                     case LBN_DBLCLK: 
                         PostMessage(hWndDlg, WM_COMMAND, IDC_EDITWP, 0L);
                     break;
                 }
                 break; 
            
            case IDC_EDITWP:
            {
            	 long	WPID;
            	 
				 SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETSELITEMS,1,(LPARAM)&Choice);
		         if (Choice >= 0)
		         {  
		         	SendDlgItemMessage (hWndDlg,IDC_WPLIST,LB_GETTEXT,Choice,(LPARAM)str); 
		     	 	pBeg = _fstrrchr (str,'\t');
		     	 	if (!pBeg)
		     	 		break; 
		     	 	pBeg++;
		     	 	WPID = atol (pBeg);  
		     	 	SetGlobalValueLong ("WPID",WPID); 
			        {
			        	BOOL	nRc;
			            
			            setDoPaint( FALSE);      
			            AllowWPSkip=FALSE;   
						lpfnADDWAYPOINTMsgProc = MakeProcInstance((DLGPROC)ADDWAYPOINTMsgProc, hInst);
						nRc = DialogBox(hInst, (LPSTR)"ADDWAYPOINT",hWndDlg, lpfnADDWAYPOINTMsgProc);
						FreeProcInstance(lpfnADDWAYPOINTMsgProc);
						setDoPaint( TRUE); 
         				if (nRc)
         					PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
			        }
		     	 } 
		    }
            	 break;      
            
            case IDC_GPSWP:	
            {
        		short	n, nItems=SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETSELCOUNT,0,0); 
				HANDLE	hItems;
				LPINT	lpItems; 
				LPSTR	pTAB; 
				char	WPID[32];
				
				if (!nItems)
					break;  
				n = nItems;
				hItems=GSSiGlobAlloc (1028,GMEM_MOVEABLE,nItems*4);
				lpItems=  (LPINT) GlobalLock(hItems); 
				SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETSELITEMS,nItems,(LPARAM)lpItems);
				while (nItems--)
				{ 
					SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETTEXT,*lpItems,(LPARAM)((LPSTR)str));
					pTAB = _fstrrchr (str,'\t');
					if (pTAB)
					{
	                    WayPointID = atol (pTAB);
						sprintf (WPID,"WP%4.4ld",WayPointID);
						PadString (WPID,' ',6);
	                    AddWPToGPSList (WPID,WayPointID,0,0);
					}
					lpItems++;
				}
				GSSiGlobUlFree (&hItems);  
				sprintf (str,"%i item(s) placed in the GPS transfer file",n);
		 		MessageBox (hWndDlg,str," ",MB_OK);
				
		    }
            	 break;      
            
            case IDC_EXPORTWP:	
            {
        		short	n, nItems=SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETSELCOUNT,0,0); 
				HANDLE	hItems;
				LPINT	lpItems; 
				LPSTR	pTAB; 
				char	WPID[32];
				
				if (!nItems)
					break;  
				n = nItems;
				hItems=GSSiGlobAlloc (1028,GMEM_MOVEABLE,nItems*4);
				lpItems=  (LPINT) GlobalLock(hItems); 
				SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETSELITEMS,nItems,(LPARAM)lpItems);
				while (nItems--)
				{ 
					SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETTEXT,*lpItems,(LPARAM)((LPSTR)str));
					pTAB = _fstrrchr (str,'\t');
					if (pTAB)
					{
	                    WayPointID = atol (pTAB);
						sprintf (WPID,"WP%4.4ld",WayPointID);
						PadString (WPID,' ',6);
	                    AddWPToGPSList (WPID,WayPointID,0,0);
					}
					lpItems++;
				}
				GSSiGlobUlFree (&hItems);  
				sprintf (str,"%i item(s) placed in the GPS transfer file",n);
		 		MessageBox (hWndDlg,str," ",MB_OK);
				
		    }
            	 break;      
            
            case IDC_GOTOWP:
            {
        		short	nItems=SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETSELCOUNT,0,0); 
				HANDLE	hItems;
				LPINT	lpItems; 
				char	List[350]="";
				short	LenList=0; 
				BOOL	GotOne=FALSE; 
				DPOINT	DPoint;  
				LPSTR	pTAB;
				
				DBoundsInit ((LPMNMXCORD)UserBounds);	
				if (!nItems)
					break;
				hItems=GSSiGlobAlloc (1029,GMEM_MOVEABLE,nItems*4);
				lpItems=  (LPINT) GlobalLock(hItems); 
				SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETSELITEMS,nItems,(LPARAM)lpItems);
				while (nItems--)
				{ 
					SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETTEXT,*lpItems,(LPARAM)((LPSTR)str));
					pTAB = _fstrrchr (str,'\t');
					if (pTAB)
					{
						GotOne = TRUE;
						pTAB = _fstrchr (pTAB,' ');
						pTAB++; 
						sscanf (pTAB,"%Flf %Flf",&DPoint.x,&DPoint.y);
						ConvertCoord (&DPoint,2,1);
						AddDPointToMinMax (&DPoint,(LPMNMXCORD)UserBounds);
					}
					lpItems++;
				}
				GSSiGlobUlFree (&hItems);
				ExpandBounds ((LPMNMXCORD)UserBounds,200);
            	GSSiRemove (WPSelectFile);
                EndDialog(hWndDlg, TRUE);
		    }
            	 break;      
            
            case IDC_DELETEWP:
            {
        		short	nItems=SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETSELCOUNT,0,0); 
				HANDLE	hItems;
				LPINT	lpItems;
				LPSTR	pTAB; 
				
				if (!nItems)
					break; 
				if (nItems > 1)
					sprintf (str,"Are you sure you wish to delete these %i waypoints?",nItems);
				else
					_fstrcpy (str,"Are you sure you wish to delete this waypoint?");
		 		if (MessageBox (hWndDlg,str,"Verify Delete",MB_YESNO) == IDNO)
		 			break;
				hDBWP = OpenGWDatabase (WayPointGMD,BT_WRITE);
				lpGWDHead = (LPGWDHEADER)GlobalLock (hDBWP); 
				hDBFish = OpenGWDatabase (FishGMD,BT_WRITE);
				if (hDBFish)
					lpGWDHeadFish = (LPGWDHEADER)GlobalLock (hDBFish); 
				hDBFoundWP = OpenGWDatabase (SelWaypntDB,BT_WRITE);
				lpGWDHeadFound = (LPGWDHEADER)GlobalLock (hDBFoundWP); 
				hItems=GSSiGlobAlloc (1030,GMEM_MOVEABLE,nItems*4);
				lpItems=  (LPINT) GlobalLock(hItems); 
				SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETSELITEMS,nItems,(LPARAM)lpItems);
				while (nItems--)
				{ 
					SendDlgItemMessage(hWndDlg ,IDC_WPLIST,LB_GETTEXT,*lpItems,(LPARAM)((LPSTR)str));
					pTAB = _fstrrchr (str,'\t');
					if (pTAB)
					{   
                        WayPointID = atol (pTAB);  
						if (PickByRefno(WayPointID,0,0,-1))
							DeletePickedItem (0,12,92);
                        
						BT_DELETE (lpGWDHead->BTHandle[0],(LPSTR)&WayPointID,(LPSTR)&Offset,FALSE);
						BT_DELETE (lpGWDHeadFound->BTHandle[0],(LPSTR)&WayPointID,(LPSTR)&Offset,FALSE);
						if (hDBFish)
						{   
				NextDelete:
							FishKey.WayPoint = WayPointID;
							FishKey.FishID = 0;    
							st = BT_FIND (lpGWDHeadFish->BTHandle[0],(LPSTR)&FishKey,BT_FIRST,BT_GE, (LPSTR)&Offset);
							if (!st && FishKey.WayPoint == WayPointID)
							{
								BT_DELETE (lpGWDHeadFish->BTHandle[0],(LPSTR)&FishKey,(LPSTR)&Offset,FALSE);
								goto NextDelete;
							}
						}
					}
					lpItems++;
				}
				GSSiGlobUlFree (&hItems);
				GlobalUnlock (hDBFoundWP);  
			    CloseGWDatabase (hDBFoundWP); 
		 		GlobalUnlock (hDBWP); 
		 		CloseGWDatabase (hDBWP);
		 		if (hDBFish)
		 		{
		 			GlobalUnlock (hDBFish);
		 			CloseGWDatabase (hDBFish);
		 		}
                PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
		    }
            	 break;      
            
            case IDC_LAKE:  
            	WPSelSort = 1;
            	WPFieldID = 0; 
            	WPSelCntl = IDC_LAKELIST; 
            	_fstrcpy (WPSelTitle,"Lakes");
            	WPAllowRange = FALSE;
            	goto GetList;
            case IDC_YEAR:
            	WPFieldID = 1; 
            	WPSelCntl = IDC_YEARLIST; 
            	_fstrcpy (WPSelTitle,"Years");
            	WPAllowRange = TRUE;
            	goto GetList;
            case IDC_MONTH:
            	WPFieldID = 2; 
            	WPSelCntl = IDC_MONTHLIST; 
            	_fstrcpy (WPSelTitle,"Months");
            	WPAllowRange = TRUE;
            	goto GetList;
            case IDC_TIME:
            	WPFieldID = 3; 
            	WPSelCntl = IDC_TIMELIST; 
            	_fstrcpy (WPSelTitle,"Time of Day");
            	WPAllowRange = TRUE;
            	goto GetList;
            case IDC_SPECIES:
            	WPFieldID = 4; 
            	WPSelCntl = IDC_SPECIESLIST; 
            	_fstrcpy (WPSelTitle,"Species");
            	WPAllowRange = FALSE;
            	goto GetList;   
            case IDC_SEARCHBAIT:
            	WPFieldID = 5; 
               	WPSelCntl = IDC_BAITLIST; 
            	_fstrcpy (WPSelTitle,"Bait");
            	WPAllowRange = FALSE;
            	goto GetList;
            case IDC_SEARCHDEPTH:
            	WPFieldID = 6; 
               	WPSelCntl = IDC_DEPTHLIST; 
            	_fstrcpy (WPSelTitle,"Depth");
            	WPAllowRange = TRUE;
            	goto GetList;
            case IDC_WTEMP:
            	WPFieldID = 7; 
            	WPSelCntl = IDC_WTEMPLIST; 
            	_fstrcpy (WPSelTitle,"Water Temperature");
            	WPAllowRange = TRUE;
            	goto GetList;
            case IDC_WEATHER:
            	WPFieldID = 8; 
            	WPSelCntl = IDC_WEATHERLIST; 
            	_fstrcpy (WPSelTitle,"Weather");
            	WPAllowRange = FALSE;
            	goto GetList;
            case IDC_ATEMP:
            	WPFieldID = 9; 
            	WPSelCntl = IDC_ATEMPLIST; 
            	_fstrcpy (WPSelTitle,"Air Temperature");
            	WPAllowRange = TRUE;
            	goto GetList;
            case IDC_WSPEED:
            	WPFieldID = 10; 
            	WPSelCntl = IDC_WSPEEDLIST; 
            	_fstrcpy (WPSelTitle,"Wind Speed");
            	WPAllowRange = TRUE;
            	goto GetList;
            case IDC_WDIR:
            	WPFieldID = 11; 
            	WPSelCntl = IDC_WDIRLIST; 
            	_fstrcpy (WPSelTitle,"Wind Direction");
            	WPAllowRange = FALSE;
            	goto GetList;
            case IDC_BSTATE:
            	WPFieldID = 12; 
            	WPSelCntl = IDC_BSTATELIST; 
            	_fstrcpy (WPSelTitle,"Barometer State");
            	WPAllowRange = FALSE;
            	goto GetList;
            case IDC_BVALUE:
            	WPFieldID = 13; 
            	WPSelCntl = IDC_BVALUELIST; 
            	_fstrcpy (WPSelTitle,"Barometer Value");
            	WPAllowRange = TRUE;
            	goto GetList;
     GetList:
	            {
					DLGPROC lpfnWPSELECTLISTMsgProc; 
					short	nRc;
					char	DialogName[2][16]={"WPSELECTLIST","WPSELECTLISTSRT"}; 
					lpfnWPSELECTLISTMsgProc = MakeProcInstance((DLGPROC)WPSELECTLISTMsgProc, hInst);
					nRc = DialogBox(hInst, (LPSTR)DialogName[WPSelSort], hWndDlg, lpfnWPSELECTLISTMsgProc);
					FreeProcInstance(lpfnWPSELECTLISTMsgProc);
					if (nRc)
					{
					}
				}
            	break;
            	
            case IDOK:
            {
				short	pos=BT_FIRST, cond=BT_GT, NumFish, NumFound=0, st;  
				BOOL	MatchFish, WantNoFish, HaveFish;
				char	LName[100], DepthC[32];
				
//    	 ntab = loadtabs (TabStops);     
//       	SendDlgItemMessage (hWndDlg,IDC_WPLIST,LB_SETTABSTOPS,ntab,(LPARAM)&TabStops);
				EnableWindow (GetDlgItem(hWndDlg,IDC_GOTOWP),FALSE);
				EnableWindow (GetDlgItem(hWndDlg,IDC_EDITWP),FALSE);
				EnableWindow (GetDlgItem(hWndDlg,IDC_DELETEWP),FALSE);
       			SendDlgItemMessage (hWndDlg,IDC_WPLIST,LB_RESETCONTENT,0,0);
				hDBName = OpenGWDatabase ("[%DL]lakes\\lakename.gmd",BT_READ);
				if (!hDBName)
				 	break;
				 hDBWP = OpenGWDatabase (WayPointGMD,BT_READ);
				 if (!hDBWP) 
				 {
				 	CloseGWDatabase (hDBName);
				 	break;     
				 }
				 hWPSelect = BT_OPEN (WPSelectFile,0, BT_READ, 0);
				 lpGWDHeadName = (LPGWDHEADER)GlobalLock (hDBName); 
				 lpGWDHead = (LPGWDHEADER)GlobalLock (hDBWP); 
				 pWPData = (LPWAYPOINTDATA)&lpGWDHead->GWDData; 
				 hDBFish = OpenGWDatabase (FishGMD,BT_READ);
				 if (!hDBFish)
				 	break;   
				 lpGWDHeadFish = (LPGWDHEADER)GlobalLock (hDBFish); 
				 pFishData = (LPFISHDATA)&lpGWDHeadFish->GWDData;   
				 WantNoFish = WPInSelList(4,-1) && WPInSelList(5,-1);
				 hDBFoundWP = CreateFoundWPDB ();
				 lpGWDHeadFound = (LPGWDHEADER)GlobalLock (hDBFoundWP); 
				 pFoundWPData = (LPFOUNDWPDATA)&lpGWDHeadFound->GWDData; 
				 WayPointID = 0;
				 while (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&WayPointID,pos,cond, (LPSTR)&Offset))
				 {
				 	pos = BT_NEXT;
				 	cond = BT_ANY;
				 	FillGWDData (lpGWDHead,Offset);
					if (!WPInSelList(0,pWPData->LakeID))
					 	goto NextWP;
					if (!WPInSelList(1,pWPData->Year))
					 	goto NextWP;
					if (!WPInSelList(2,pWPData->Month))
					 	goto NextWP;
					if (!WPInSelListR(3,pWPData->FromTime,pWPData->ToTime))
					 	goto NextWP;
					if (!WPInSelList(6,pWPData->Depth))
					 	goto NextWP;
					if (!WPInSelList(7,pWPData->WTemp))
					 	goto NextWP;
					if (!WPInSelList(8,pWPData->Weather))
					 	goto NextWP;
					if (!WPInSelList(9,pWPData->ATemp))
					 	goto NextWP;
					if (!WPInSelList(10,pWPData->WindSpeed))
					 	goto NextWP;
					if (!WPInSelList(11,pWPData->WindDir))
					 	goto NextWP;
					if (!WPInSelList(12,pWPData->BarState))
					 	goto NextWP;
					if (!WPInSelList(13,pWPData->BarValue))
					 	goto NextWP;
					FishKey.WayPoint = WayPointID;
					FishKey.FishID = 0;    
					NumFish = 0; 
					MatchFish = HaveFish = FALSE;
					st = BT_FIND (lpGWDHeadFish->BTHandle[0],(LPSTR)&FishKey,BT_FIRST,BT_GE, (LPSTR)&Offset);
					while (!st && FishKey.WayPoint == WayPointID)
					{   
						HaveFish = TRUE;
					 	FillGWDData (lpGWDHeadFish,Offset);
						if (WPInSelList(4,pFishData->Type) && WPInSelList(5,pFishData->LiveBait))
					 		MatchFish=TRUE;
		 				NumFish += pFishData->Num+1;
					 	st = BT_FIND (lpGWDHeadFish->BTHandle[0],(LPSTR)&FishKey,BT_NEXT,BT_ANY, (LPSTR)&Offset);
					} 
					if ((!WantNoFish && !HaveFish) || (HaveFish && !MatchFish))
						goto NextWP;
				    if (!BT_FIND (lpGWDHeadName->BTHandle[0],(LPSTR)&pWPData->LakeID,BT_FIRST,BT_EQ, (LPSTR)&Offset))
				    {
					 	FillGWDData (lpGWDHeadName,Offset);
		 				GMDGetCharFieldVal (lpGWDHeadName,1,LName);
		 			}
		 			else
		 				*LName = 0;  
				 	GetListValue ("[%DL]waypopts\\depths.txt", pWPData->Depth, DepthC);
		 			sprintf (str,"%s\t%s\t%i/%i/%i\t%i\t%s\t%ld %f %f",pWPData->WPName,LName,
		 			                              pWPData->Month+1,pWPData->Day,pWPData->Year+1999,  
		 			                              NumFish,DepthC,
		 			                              WayPointID,pWPData->LatLong.x,pWPData->LatLong.y);
		 			NumFound++;
		 			pFoundWPData->ID = WayPointID;
		 			pFoundWPData->Depth = pWPData->Depth;
		 			pFoundWPData->LatLong = pWPData->LatLong; 
		 			_fstrcpy (pFoundWPData->WPName,pWPData->WPName); 
		 			_fstrcpy (pFoundWPData->Text,str); 
		 			GWDAddRecord (lpGWDHeadFound,0,0);
		 			
				 	SendDlgItemMessage (hWndDlg,IDC_WPLIST,LB_ADDSTRING,0,(LPARAM)str); 
		NextWP:;
		 		} 
		 		if (!NumFound)
				 	SendDlgItemMessage (hWndDlg,IDC_WPLIST,LB_ADDSTRING,0,
				 						(LPARAM)"No waypoints found for these criteria"); 
				GlobalUnlock (hDBFoundWP);  
			    CloseGWDatabase (hDBFoundWP); 
		 		GlobalUnlock (hDBName);
		 		GlobalUnlock (hDBWP);
		 		GlobalUnlock (hDBFish);
		 		CloseGWDatabase (hDBFish);
		 		CloseGWDatabase (hDBWP);
		 		CloseGWDatabase (hDBName);
		        BT_CLOSE (hWPSelect);
				LabelChanged = TRUE;
		        hWPSelect = 0;        
                PostMessage(hWndDlg, WM_COMMAND, IDC_DISPLAYSELECTEDWP, 0L);

		 	}  
            	break;
            	 
            case IDCANCEL:
            {
            	char	idstr[64];
            	  
            	GSSiRemove (WPSelectFile); 
           		_fstrcpy (idstr,"@$MACRO([%DL]macros\\wplabel.txt)"); 
		        if (SendDlgItemMessage (hWndDlg,IDC_DISPLAYALLWP,BM_GETCHECK,0,0)) 
		        {
		        	SetGlobalValueLong ("WPDisplayOpt",0);
            		SetGlobalValue("WPLABELDB",WayPointGMD);  
		        }
		        else if (SendDlgItemMessage (hWndDlg,IDC_DISPLAYSELECTEDWP,BM_GETCHECK,0,0))  
		        {
		        	SetGlobalValueLong ("WPDisplayOpt",1);
            		SetGlobalValue("WPLABELDB",SelWaypntGCF);
            	}
		        else if (SendDlgItemMessage (hWndDlg,IDC_DISPLAYONLYGPS,BM_GETCHECK,0,0)) 
		        {
		        	SetGlobalValueLong ("WPDisplayOpt",2);
            		SetGlobalValue("WPLABELDB","[%DL]attribut\\ingps.gcf"); 
           			_fstrcpy (idstr,"@[WPID]"); 
           			UpdateGPSWPDB (0);
            	}
            	*str = 0;
            	if ((SetGlobalFromCheckBox (hWndDlg,IDC_LABID,"WPLabelID")))
            		_fstrcat (str,idstr); 
            	if ((SetGlobalFromCheckBox (hWndDlg,IDC_LABLATLON,"WPLabelLatLon")))
            	{
            		if (*str)
            			_fstrcat (str,"\r\n");
            		_fstrcat (str,"N@$DMS(@[WPY],@[COORDOPT])");
            		_fstrcat (str,"\r\n");
            		_fstrcat (str,"W@$DMS(@[WPX],@[COORDOPT])");
            	}
            	if ((SetGlobalFromCheckBox (hWndDlg,IDC_LABDEPTH,"WPLabelDepth")))
            	{
            		if (*str)
            			_fstrcat (str,"\r\n");
            		_fstrcat (str,"@$DECODE([%DL]waypopts\\depths.txt,@[DEPTH])");
            	}
            	SetGlobalValue("WPLABEL",str); 
                EndDialog(hWndDlg, LabelChanged);  
             }
                break; 
                 
           }
    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL FAR PASCAL WPSELECTLISTMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	short	Default;
	char	str[256], File[128];  
	short	Choice=0;
	LPSTR	pBeg, pEnd;    
   	LPSTR	pTAB;
	static	BOOL	First;  
	int   	TabStops[2]={1200,1500};
   	short	st, Dummy=0; 
	
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
    	sprintf (str,"All %s",WPSelTitle);
    	SetWindowText (GetDlgItem(hWndDlg,IDC_CHECK),str);
    	sprintf (str,"Select %s",WPSelTitle);
    	SetWindowText (hWndDlg,str);
       	SendDlgItemMessage (hWndDlg,IDC_LIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
    	GetDlgItemText (hWndFindWP,WPSelCntl,str,255);    
    	if (!_fstricmp (str,"ALL"))
			SendDlgItemMessage (hWndDlg,IDC_CHECK,BM_SETCHECK,TRUE,0); 
		else
		{
			EnableWindow (GetDlgItem(hWndDlg,IDC_LIST),TRUE);
         	PostMessage(hWndDlg, WM_COMMAND, IDC_SETSEL, 0L);
         }
    	switch (WPFieldID)
    	{  
    		case 0:
            {
			    LPGWDHEADER lpGWDHead,lpGWDHeadName; 
				long	LakeID, Offset; 
				static	long	WayPointID;
				HANDLE	hDBWP,hDBName, hBT;
				LPWAYPOINTDATA	pWPData;   
				LPFIELDINFO pField;
				short	pos=BT_FIRST; 
				char	LNumC[16];
				
				hDBName = OpenGWDatabase ("[%DL]lakes\\lakename.gmd",BT_READ);
				if (!hDBName)
				 	break;
				 hDBWP = OpenGWDatabase (WayPointGMD,BT_READ);
				 if (!hDBWP) 
				 {
				 	CloseGWDatabase (hDBName);
				 	break;     
				 }
				 lpGWDHeadName = (LPGWDHEADER)GlobalLock (hDBName); 
				 lpGWDHead = (LPGWDHEADER)GlobalLock (hDBWP); 
				 pWPData = (LPWAYPOINTDATA)&lpGWDHead->GWDData; 
				 hBT = CreateUniqueList (12,0);
				 GetGMDUniqueFieldValues (hDBWP,"",1,hBT);
				 while (!BT_FIND (hBT,LNumC,pos,BT_ANY,(LPSTR)&Dummy))
				 {
				 	pos = BT_NEXT;
				 	LakeID = atol (LNumC);
				    if (LakeID && !BT_FIND (lpGWDHeadName->BTHandle[0],(LPSTR)&LakeID,BT_FIRST,BT_EQ, (LPSTR)&Offset))
				    {
					 	FillGWDData (lpGWDHeadName,Offset);
		 				GMDGetCharFieldVal (lpGWDHeadName,1,str); 
		 				_fstrcat (str,"\t");
		 				ltoa (LakeID,_fstrchr(str,0),10);
					 	SendDlgItemMessage (hWndDlg,IDC_LIST,LB_ADDSTRING,0,(LPARAM)str); 
		 			}
		 		} 
		 		GlobalUnlock (hDBName);
		 		GlobalUnlock (hDBWP);
		 		CloseGWDatabase (hDBWP);
		 		CloseGWDatabase (hDBName);
		 		BT_CLOSEANDDELETE (&hBT);
		 		return TRUE;
		 	}  
    		case 1:
    			_fstrcpy (File,"[%DL]waypopts\\years.txt"); 
    			break;
    		case 2:
    			_fstrcpy (File,"[%DL]waypopts\\months.txt"); 
    			break;
    		case 3:
    			_fstrcpy (File,"[%DL]waypopts\\times.txt"); 
    			break;
    		case 4:
    			_fstrcpy (File,"[%DL]waypopts\\typefish.txt"); 
    			break;
    		case 5:
    			_fstrcpy (File,"[%DL]waypopts\\livebait.txt"); 
    			break;
    		case 6:
    			_fstrcpy (File,"[%DL]waypopts\\depths.txt"); 
    			break;
    		case 7:
    			_fstrcpy (File,"[%DL]waypopts\\wattmp.txt"); 
    			break;
    		case 8:
    			_fstrcpy (File,"[%DL]waypopts\\weather.txt"); 
    			break;
    		case 9:
    			_fstrcpy (File,"[%DL]waypopts\\temps.txt"); 
    			break;
    		case 10:
    			_fstrcpy (File,"[%DL]waypopts\\winspeed.txt"); 
    			break;
    		case 11:
    			_fstrcpy (File,"[%DL]waypopts\\windir.txt"); 
    			break;
    		case 12:
    			_fstrcpy (File,"[%DL]waypopts\\barstate.txt"); 
    			break;
    		case 13:
    			_fstrcpy (File,"[%DL]waypopts\\barvalue.txt"); 
    			break;
    	}
        FillList (hWndDlg,IDC_LIST,File,0,0);
        if (WPFieldID == 4)
        	SendDlgItemMessage (hWndDlg,IDC_LIST,LB_ADDSTRING,0,(LPARAM)"None\t-1");
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            
            case IDC_SETSEL:  
				hWPSelect = BT_OPEN (WPSelectFile,0, BT_READ, 0);
				while (SendDlgItemMessage(hWndDlg,IDC_LIST,LB_GETTEXT,Choice,(LPARAM)((LPSTR)str)) != LB_ERR)
				{
					pTAB = _fstrchr (str,'\t');
					*pTAB++ = 0;  
					WPSelKey.FieldID = WPFieldID;
					WPSelKey.Value = atol(pTAB); 
					if (!BT_FIND (hWPSelect,(LPSTR)&WPSelKey,BT_FIRST,BT_EQ,(LPSTR)&Dummy))
						SendDlgItemMessage(hWndDlg ,IDC_LIST,LB_SETSEL, TRUE,MAKELPARAM(Choice,0)); 			  
					Choice++;
		         } 
		         BT_CLOSE (hWPSelect);
		         hWPSelect = 0;
		         break;
            
            case IDC_CHECK:
            	if (SendDlgItemMessage (hWndDlg,IDC_CHECK,BM_GETCHECK,0,0))
                {
                	EnableWindow (GetDlgItem(hWndDlg,IDC_LIST),FALSE);
                }
                else
                {
                	EnableWindow (GetDlgItem(hWndDlg,IDC_LIST),TRUE);
                }
            	break;
            	
            case IDOK: 
            {
            
				hWPSelect = BT_OPEN (WPSelectFile,0, BT_WRITE, 0);
NextDelete: 
				WPSelKey.FieldID = WPFieldID;
				WPSelKey.Value = -1;
				st = BT_FIND (hWPSelect,(LPSTR)&WPSelKey,BT_FIRST,BT_GE,(LPSTR)&Dummy);
				if (!st && WPSelKey.FieldID == WPFieldID)
				{
					BT_DELETE (hWPSelect,(LPSTR)&WPSelKey,(LPSTR)&Dummy,FALSE);
					goto NextDelete;
				} 
            	if (SendDlgItemMessage (hWndDlg,IDC_CHECK,BM_GETCHECK,0,0)) 
            	{ 
			    	SetDlgItemText (hWndFindWP,WPSelCntl,"All");    
            	}
            	else
            	{   
            		int		nItems=SendDlgItemMessage(hWndDlg ,IDC_LIST,LB_GETSELCOUNT,0,0); 
					HANDLE	hItems;
					LPINT	lpItems; 
					LPSTR	LastBeg, pRList;
					char	List[350]="",RList[350]="";
					short	LenList=0, LenRList=0, LastItem;
					
					if (!nItems)
						break;
					hItems=GSSiGlobAlloc (1031,GMEM_MOVEABLE,nItems*4);
					lpItems=  (LPINT) GlobalLock(hItems); 
					SendDlgItemMessage(hWndDlg ,IDC_LIST,LB_GETSELITEMS,nItems,(LPARAM)lpItems);
					while (nItems--)
					{ 
						SendDlgItemMessage(hWndDlg ,IDC_LIST,LB_GETTEXT,*lpItems,(LPARAM)((LPSTR)str));
						pTAB = _fstrchr (str,'\t');
						*pTAB++ = 0;  
						if (LenList < 255)
						{
							if (LenList)
								_fstrcat (List,",");
							_fstrcat (List,str);
							LenList = _fstrlen (List);
						}
						if (LenRList < 255)
						{
							if (LenRList)
							{   
								if ((*lpItems - LastItem) == 1)
								{
									if (*LastBeg == '-')
										pRList = LastBeg+1;
									else
									{
										_fstrcat (RList,"-"); 
										LastBeg = _fstrrchr (RList,'-');  
										pRList = _fstrchr (RList,0);
									}
								}
								else
								{
									_fstrcat (RList,","); 
									LastBeg = _fstrrchr (RList,','); 
									pRList = _fstrchr (RList,0);
								}
							}
							else
							{
								pRList = RList;
								LastBeg = pRList;
							}
							_fstrcpy (pRList,str);
							LenRList = _fstrlen (RList);
						}
						WPSelKey.FieldID = WPFieldID;
						WPSelKey.Value = atol(pTAB);
						BT_PUT (hWPSelect,(LPSTR)&WPSelKey,(LPSTR)&Dummy); 
						LastItem = *lpItems;
						lpItems++;
					}
					GSSiGlobUlFree (&hItems);
					List[256]=0;
					if (WPAllowRange)
			    		SetDlgItemText (hWndFindWP,WPSelCntl,RList); 
			    	else   
			    		SetDlgItemText (hWndFindWP,WPSelCntl,List);    
            	}
				BT_CLOSE (hWPSelect); 
				hWPSelect = 0;
                EndDialog(hWndDlg, TRUE); 
            }
            	break;
            	 
            case IDCANCEL:
                EndDialog(hWndDlg, FALSE);
                break; 
                 
           }
    default:
        return FALSE;
   }
 return TRUE;
} 


