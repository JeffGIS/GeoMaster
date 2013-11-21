#include "graphint.h"
#include "gmextern.h"

static	BOOL	EditSignData;

BOOL AddNewSigns (HWND hWnd)
{

	FARPROC lpfnSIGNINVMsgProc; 
	short	nRc;
	
	EditSignData = FALSE;			 
	lpfnSign_NEWMsgProc = MakeProcInstance((FARPROC)SIGNINVMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"SIGNINV", hWnd, lpfnSIGNINVMsgProc);
	FreeProcInstance(lpfnSIGNINVMsgProc);
	return nRc;
}

BOOL EditSign (HWND hWnd,long SignID,int SignFile)
{

	FARPROC lpfnSIGNINVMsgProc; 
	short	nRc;  
	long	vid=VoterID;
	short	vf=VoterFile;
	
	EditVoterData = TRUE;	
	CurrentVoterID = vid;
	CurrentVoterDB = vf;		 
	lpfnSIGNINVMsgProc = MakeProcInstance((FARPROC)SIGNINVMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"SIGNINV", hWnd, lpfnSIGNINVMsgProc);
	FreeProcInstance(lpfnSIGNINVMsgProc);
	return nRc;
}
BOOL FAR PASCAL SIGNINVMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
 int	BRtn;  
 char	str[256];
 long	NewID; 
 long	Refno;
 short	i, nRc;   
 BOOL	Err;
 static	DPOINT	NewCoord; 
 static	BOOL	AddChanged;  
 static short	LocationCode;
 static	HANDLE	hSaveBM=0;  
 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
    	 hSaveBM = EnterBlockingWindow (hWndDlg);
    	 AddChanged = FALSE;
    	 LoadVoterFields (hWndDlg);      
         EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
         GetGlobalCVal ("[CURRENTEXTRACTFILE]",str,"None");
         if (_fstricmp (str,"None"))
         	EnableWindow (GetDlgItem(hWndDlg,IDC_ADDTOEXTRACT),TRUE);      
         if (EditVoterData)
         {
         	nRc = 1;
         	goto ShowVoter;
         }
         NewID = GetNewVoterID ();  
         ltoa (NewID,str,10);
		 SetDlgItemText (hWndDlg,IDC_VOTERID,str);	
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND: 
    	
		 for (i=0;i<NumUserCheckbox;i++)
			if (wParam == USERCBCNTL[i]) 
         		EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
         if (HIWORD(wParam) == EN_CHANGE)
         {
			for (i=0;i<NumUserTextbox;i++)
				if (wParam == USERTXCNTL[i]) 
	         		EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
		 }
         if (HIWORD(wParam) == CBN_SELCHANGE)
         {
			for (i=0;i<NumUserLists;i++)
				if (wParam == USERLBCNTL[i]) 
	         		EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
		 }
         switch(LOWORD(wParam))
         {  
         	case IDC_ADDTOEXTRACT:      
                GetDlgItemText (hWndDlg,IDC_VOTERID,str,32);
                Refno = atol (str);
                sprintf (str,"$MACRO([%%DL]MACROS\\addtoextractfile.txt,%ld);",Refno);
                ExpandText (str);
                MessageBox (hWndDlg,str,"",MB_OK);
         		break; 
         	case IDC_VOTER_HOUSEHOLD:
				NewID = GetNewVoterID ();  
				ltoa (NewID,str,10);
				SetDlgItemText (hWndDlg,IDC_VOTERID,str);	
				SetDlgItemText (hWndDlg,IDC_VOTER_FIRST_NAME,"");	
				SetDlgItemText (hWndDlg,IDC_VOTER_MIDDLE_NAME,"");	
				SetDlgItemText (hWndDlg,IDC_VOTER_DOB_YEAR,"");	
				SetDlgItemText (hWndDlg,IDC_VOTER_GENDER,"");	
				SetDlgItemText (hWndDlg,IDC_VOTER_EMAIL,"");	
				SetDlgItemText (hWndDlg,IDC_VOTER_WORK_PHONE,"");	
				SetDlgItemText (hWndDlg,IDC_VOTER_CELL_PHONE,"");	
         		break;
         	case IDC_VOTER_NAME_SEARCH:
			{
	            FARPROC lpfnVOTER_NAME_LOCMsgProc;
				
				GetDlgItemText (hWndDlg,IDC_VOTER_FIRST_NAME,FirstName,128);	
				GetDlgItemText (hWndDlg,IDC_VOTER_MIDDLE_NAME,MiddleName,128);	
				GetDlgItemText (hWndDlg,IDC_VOTER_LAST_NAME,LastName,128);	
				GetDlgItemText (hWndDlg,IDC_VOTER_ZIP,ZipCodeC,16);
				VoterLocOpt = 0;	
	            lpfnVOTER_NAME_LOCMsgProc = MakeProcInstance((FARPROC)VOTER_NAME_LOCMsgProc, hInst);
	            nRc = DialogBox(hInst, (LPSTR)"VOTER_NAME_LOC", hWndDlg, lpfnVOTER_NAME_LOCMsgProc);
	            FreeProcInstance(lpfnVOTER_NAME_LOCMsgProc); 
	  ShowVoter:
	            if (nRc)
	            {   
	            	char	SQL[32];
					HANDLE	hDBVoter=0;
	            	
	            	sprintf (SQL,"VoterID=%ld",CurrentVoterID);
	            	if (CurrentVoterDB < 0)
	            	{
					    if (!OpenDataFile (VoterDB[0],SQL,BT_READ, &hDBVoter))
					    {
					        MessageBox( GetFocus(), VoterDB[0],"Cannot open voter database", MB_OK);  
					        break;
					    }
					    if (!FetchDBRec (hDBVoter))
					    { 
	    					CloseDataFile (TRUE,&hDBVoter);
						    if (!OpenDataFile (VoterDB[1],SQL,BT_READ, &hDBVoter))
						    {
						        MessageBox( GetFocus(), VoterDB[1],"Cannot open voter database", MB_OK);  
						        break;
						    }
					    }
					}
	            	else
	            	{
					    if (!OpenDataFile (VoterDB[CurrentVoterDB],SQL,BT_READ, &hDBVoter))
					    {
					        MessageBox( GetFocus(), VoterDB[CurrentVoterDB],"Cannot open voter database", MB_OK);  
					        break;
					    }
					}
					SetDlgItemTextFromField (hWndDlg,IDC_VOTERID,"VoterID",0);	
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_FIRST_NAME,"FirstName",0);	
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_MIDDLE_NAME,"MiddleName",0);	
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_LAST_NAME,"LastName",0);   
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_NAME_SUFFIX,"NameSuffix",0);   
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_SALUTATION,"Salutation",0);   
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_HOUSE_NUM,"HouseNumber",0);  
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_APT,"UnitNumber",0);  
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_STREET_NAME,"StreetName",0);  
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_CITY_NAME,"City",0);  
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_STATE,"State",0);  
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_PRECINCT,"PrecinctName",0);  
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_DOB_YEAR,"DOBYear",0);  
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_ZIP,"ZIPCODE",0);  
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_HOME_PHONE,"PhoneNumber",0);  
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_WORK_PHONE,"WorkPhone",0);  
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_CELL_PHONE,"CellPhone",0);  
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_EMAIL,"Email",0);  
					SetDlgItemTextFromField (hWndDlg,IDC_VOTER_GENDER,"Gender",0);
					GetValFromOpenFiles ("LocationCode",str);
					LocationCode = atoi (str); 
					SetLocationCodeField (hWndDlg,IDC_LOCATEDTO,atoi(str));  
					_fstrcpy (str,"[XCOR] [YCOR]");
					ExpandText (str);
					NewCoord = atopt (str,&Err);
					for (i=0;i<NumUserCheckbox;i++)
					{   
						SetDlgCheckboxFromField (hWndDlg,USERCBCNTL[i],UserCheckboxFieldName[i],0); 
					}
					for (i=0;i<NumUserLists;i++)
					{   
						SetDlgComboboxFromField (hWndDlg,USERLBCNTL[i],UserListFieldName[i],0); 
					} 
					for (i=0;i<NumUserTextbox;i++)
					{   
						SetDlgItemTextFromField (hWndDlg,USERTXCNTL[i],UserTextboxFieldName[i],0); 
					}
					CloseDataFile (TRUE,&hDBVoter);
			        EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
			        EnableWindow (GetDlgItem(hWndDlg,IDC_VOTER_NAME_SEARCH),TRUE);
	
	            }
			}
         	break;
         	
			case IDC_VOTER_FIRST_NAME:	
			case IDC_VOTER_MIDDLE_NAME:	
			case IDC_VOTER_LAST_NAME: 
			case IDC_VOTER_SALUTATION: 
			case IDC_VOTER_NAME_SUFFIX:  
			case IDC_VOTER_APT:
			case IDC_VOTER_PRECINCT: 
			case IDC_VOTER_DOB_YEAR:  
			case IDC_VOTER_CITY_NAME:  
			case IDC_VOTER_STATE:  
			case IDC_VOTER_HOME_PHONE:
			case IDC_VOTER_WORK_PHONE:  
			case IDC_VOTER_CELL_PHONE:  
			case IDC_VOTER_EMAIL:
                 switch (HIWORD(wParam))
                 {	case EN_CHANGE: 
                 	{ 
				         EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
                 	}
                 }  
			break;

			case IDC_VOTER_HOUSE_NUM: 
			case IDC_VOTER_STREET_NAME:  
			case IDC_VOTER_ZIP:
                 switch (HIWORD(wParam))
                 {	case EN_CHANGE: 
                 	{ 
				         EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
				         AddChanged = TRUE;
                 	} 
                 	break;
                 	case EN_KILLFOCUS:
                 		if (wParam == IDC_VOTER_ZIP)
                 			PostMessage(hWndDlg, WM_COMMAND,IDC_VOTER_ADDLOC, 0L);
                    break;
                 }
            break;  
			
			case IDC_VOTER_ADDLOC:
//					SetFieldValFromCharAndName(lpGWDHead,UserCheckboxFieldName[i],str,FALSE); 
			{
				char	House[18], Street[128], City[66], OutAddress[128], ZipCodeC[16], CityAbv[16], State[8];   
				int		irc;
                FARPROC	lpfnADD_MATCH_EDITMsgProc; 
                RECT	Rect={0,0,0,0};    
                ADDMATCH Match;
				
				GetDlgItemText (hWndDlg,IDC_VOTER_HOUSE_NUM,House,16);
				GetDlgItemText (hWndDlg,IDC_VOTER_STREET_NAME,Street,100);
				GetDlgItemText (hWndDlg,IDC_VOTER_CITY_NAME,City,64);
				GetDlgItemText (hWndDlg,IDC_VOTER_ZIP,ZipCodeC,8); 
				sprintf (str,"%s %s",House,Street);  
				_fmemset (&Match,0,sizeof(Match));
				LocationCode = 0;
				irc = AddMatchSingle (str,City,ZipCodeC,OutAddress,&NewCoord,&Match);
				switch (irc)
				{   
					case 0: 
						SetGlobalValueRect ("[%ADDMATCHEDITRECT]",Rect);
						SetAddEditValues (House,Street,City,ZipCodeC,"ADDLOC","",&Match);
						lpfnADD_MATCH_EDITMsgProc = MakeProcInstance((FARPROC)ADD_MATCH_EDITMsgProc, hInst);
						WaitCursor (1);
						irc = DialogBox(hInst, (LPSTR)"ADD_MATCH_EDIT3", hWndDlg, lpfnADD_MATCH_EDITMsgProc);
						WaitCursor (-1);
						FreeProcInstance(lpfnADD_MATCH_EDITMsgProc);
						if (irc != 1) 
						{
							long	Zip=atol (ZipCodeC);
							
							SetCityNameListFromZIP (hWndDlg,IDC_VOTER_CITY_NAME,Zip);
							SetPrecinctNameListFromZIP (hWndDlg,IDC_VOTER_PRECINCT,Zip);
							break; 
						}
					case 1: 
						SetCityNameListFromZIP (hWndDlg,IDC_VOTER_CITY_NAME,Match.ZIP);
						SetPrecinctNameListFromZIP (hWndDlg,IDC_VOTER_PRECINCT,Match.ZIP);
						if (!GetDlgItemText (hWndDlg,IDC_VOTER_CITY_NAME,City,64))
						{
							GetMunicName (Match.Munic,City,CityAbv);
							SetDlgItemText (hWndDlg,IDC_VOTER_CITY_NAME,City);
						}
						if (!GetDlgItemText (hWndDlg,IDC_VOTER_STATE,State,8))
							SetDlgItemText (hWndDlg,IDC_VOTER_STATE,"MN");  
						ltoa (Match.ZIP,str,10);
						SetDlgItemText (hWndDlg,IDC_VOTER_ZIP,str);
						if (GetTrueStreetName (Match.StreetNum, Street, 0))
							SetDlgItemText (hWndDlg,IDC_VOTER_STREET_NAME,Street);
						SetLocationCodeField (hWndDlg,IDC_LOCATEDTO,Match.LocationCode);
						NewCoord = Match.Point;   
						LocationCode = Match.LocationCode;
					break;
				}
			}
				break;
				
         	case IDOK:
         	{
				HANDLE	hDBVoter=0;
				LPGWDHEADER lpGWDHead;
			    LPOPENFILEDATA  FilePtr;
			    LPOPENSQLDATA   SQLPtr; 
			    long	Time; 
			    LPOINT	lPoint={0,0};
					
			    if (!OpenDataFile (VoterDB[0],"",BT_WRITE, &hDBVoter))
			    {
			        MessageBox( GetFocus(), VoterDB[0],"Cannot open voter database", MB_OK);
			        break;
			    }  
			    SQLPtr = (LPOPENSQLDATA) GlobalLock (hDBVoter);
			    FilePtr = (LPOPENFILEDATA) GlobalLock (SQLPtr->OFHandle);
				lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);                             
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTERID,"VoterID");	
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_FIRST_NAME,"FirstName");	
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_MIDDLE_NAME,"MiddleName");	
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_LAST_NAME,"LastName");   
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_NAME_SUFFIX,"NameSuffix");   
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_SALUTATION,"Salutation");   
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_HOUSE_NUM,"HouseNumber");  
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_APT,"UnitNumber");  
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_STREET_NAME,"StreetName");  
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_CITY_NAME,"City");  
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_STATE,"State");  
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_PRECINCT,"PrecinctName");  
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_STREET_NAME,"StreetName");  
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_DOB_YEAR,"DOBYear");  
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_ZIP,"ZIPCODE");  
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_HOME_PHONE,"PhoneNumber");  
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_WORK_PHONE,"WorkPhone");  
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_CELL_PHONE,"CellPhone");  
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_EMAIL,"Email");  
				SetFieldValFromDlgItem (lpGWDHead,hWndDlg,IDC_VOTER_GENDER,"Gender");  
				SetFieldValFromCharAndName(lpGWDHead,"LocationCode",(LPSTR)&LocationCode,TRUE);  
				if (LocationCode)
					lPoint = DPointToLPoint (&NewCoord);
				SetFieldValFromCharAndName(lpGWDHead,"XCOR",(LPSTR)&lPoint.x,TRUE);
				SetFieldValFromCharAndName(lpGWDHead,"YCOR",(LPSTR)&lPoint.y,TRUE);
				for (i=0;i<NumUserCheckbox;i++)
				{   
					*str = 0;
					if (SendDlgItemMessage (hWndDlg,USERCBCNTL[i],BM_GETCHECK,0,0))
						_fstrcpy (str,"1");
					SetFieldValFromCharAndName(lpGWDHead,UserCheckboxFieldName[i],str,FALSE);
				}
				for (i=0;i<NumUserLists;i++)
				{   
					SetFieldValFromDlgItem (lpGWDHead,hWndDlg,USERLBCNTL[i],UserListFieldName[i]); 
				} 
				for (i=0;i<NumUserTextbox;i++)
				{   
					SetFieldValFromDlgItem (lpGWDHead,hWndDlg,USERTXCNTL[i],UserTextboxFieldName[i]); 
				}
				time (&Time);
				SetFieldValFromCharAndName(lpGWDHead,"TimeStamp",(LPSTR)&Time,TRUE);
				GWDReplaceRecord (lpGWDHead,0,0,-1);
        		GlobalUnlock (FilePtr->FileHandle); 
           		GlobalUnlock (SQLPtr->OFHandle);
        		GlobalUnlock (hDBVoter);  
				CloseDataFile (TRUE,&hDBVoter); 
				if (LocationCode > 0)
				{
					HANDLE	hSymDesc=0;
					short	NumSyms=0;   
					short	SymNum;
					char	SymName[32];
					double	SymSize=5;
                    
                    sprintf (SymName,"VOTER%i",LocationCode);
                    SymNum = GetDictSymbolNumber (SymName);
                    GetDlgItemText (hWndDlg,IDC_VOTERID,str,32);
                    Refno = atol (str);
					AddToSymList (SymNum,&NumSyms,&hSymDesc);  
					_fstrcpy (PltName,"[%DL]maplib\\voters\\camp\\voters.plt");
	           		AddPointToMap (NewCoord,Refno,0,SymNum,SymSize,0,0,0,0,0,0,-1,-1,-1,FALSE,FALSE,0,0); 
					CloseMap(TRUE); 
		 	 		AddSymToMap (NumSyms,hSymDesc,0,0); 
                	DestroySymList (&NumSyms,&hSymDesc);  
                }
                GSSiEndDialog(hWndDlg, TRUE,hSaveBM);   
            }
                break;
                
            case IDCANCEL: 
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);

            break;
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 

