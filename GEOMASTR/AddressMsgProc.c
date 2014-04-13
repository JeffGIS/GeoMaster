#include "graphint.h"
#include "extrndb.h" 
#include "std.h"  

#define STORED_STREET_LEN 32
#define STORED_HOUSE_LEN 8
#define STORED_CITY_LEN 32
#define STORED_ZIP_LEN 8     

typedef struct {
		char	Name[64]; 
		char	City[32];
		long	House,
				Inc;
	} HELP1KEY;

typedef struct {
		short	MatchCode; 
		char	Note[60];
		char	OrigKey[64];
		BOOL	Removed;
	} HELP1DATA;

static	char	HouseBufSave[16]="";
static	char	StreetBufSave[32]="";
static short	AddTol=0;
static	long	NumNoMatch, NumInvalid, NumMultMatch; 
static DPOINT	NetPoint;
static  BOOL    AddRangeAll=FALSE;
static  short   PIDAddIndex,StreetNumIndex,PidLen=13;  
static LPOPENFILEDATA	FilePtrADD;
static LPOPENSQLDATA	SQLPtrADD=0;
static  GWFLDINFO   SNField, HNField, ZIPField, SNumField; 
static char		AddEditHouseNum[128];
static char		AddEditStreet[128];
static char		AddEditCity[128];
static char		AddEditZIP[128];
static char		AddEditOutVar[128];
static char		AddEditMacro[4096]="";  
static LPADDMATCH	pAddEditMatch=0;
static long	EditPath1;
static long	EditPath2;
static short	Trigger=3;
static HWND		hWndSecondaryAddInput = 0;
static UINT		intMatchList;
static HWND		intMatchWnd;
static UINT		addMatchList;
static HWND		addMatchWnd;

void AddAddMatch(HWND hWndDlg, LPSTR str);

void SetAddEditValues (LPSTR House,LPSTR Street,LPSTR City,LPSTR ZIP,LPSTR OutVar,LPSTR Macro,LPADDMATCH pMatch)
{   
	_fstrcpy (AddEditHouseNum,House);
	_fstrcpy (AddEditStreet,Street);
	_fstrcpy (AddEditCity,City);
	_fstrcpy (AddEditZIP,ZIP);
	_fstrcpy (AddEditOutVar,OutVar);
	_fstrcpy (AddEditMacro,Macro);
	pAddEditMatch = pMatch;
	return;
} 

#include "gmextern.h"   

BOOL    OpenAddressFilesPID (HWND hWnd)
{   
    short   n=0, index;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    char    DBName[128], str[132]; 
    short       nf, ifile;
    HFILE    Fid;  
    LPSTR   AddDisplayLine;  
    OFSTRUCT	OFStruct;

    PIDAddIndex = 0; 
    StreetNumIndex = 0;
    hPIDPid = 0; 
    _fstrcpy (str,"[%DL]addloc.txt");
    ExpandText (str);
    Fid = GSSiOpenFile (str,&OFStruct,OF_READ);
    if (Fid==HFILE_ERROR)
    {
        GSSiMsgBox( GetFocus(), "Unable to open addloc.txt",str, MB_OK,0);
        return FALSE;
    } 
    str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,128,Fid);n++;}
    nf = atoi(str);
    if (nf<1 || nf>4)         
    {                              
        sprintf(str,"Error in addloc.txt at line %i",n);
        GSSiMsgBox( GetFocus(), str,"Error", MB_OK,0);  
ErrOut: GSSiClose(Fid);        
        return (FALSE);
    } 
    str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,128,Fid);n++;}
    _fstrcpy (DBName,str);

    if (!OpenDataFile (DBName, "",BT_READ, &hPIDAddDBSQL))
    {
        GSSiMsgBox( GetFocus(), DBName,"Cannot open address location database", MB_OK,0);
        goto ErrOut;
    }               
    sprintf (str,"Address location db is: %s",DBName);
    GSSiTrace (str,0);
    SQLPtrADD = (LPOPENSQLDATA) GlobalLock (hPIDAddDBSQL);
    FilePtrADD = (LPOPENFILEDATA) GlobalLock (SQLPtrADD->OFHandle);
    hPIDAddDB = FilePtrADD->FileHandle;
    lpGWDHead = (LPGWDHEADER) GlobalLock (FilePtrADD->FileHandle); 
    for (index=0;index<lpGWDHead->NumIndex;index++)
    {
        if (abs(lpGWDHead->NumIndexFields[index])>1)
        {
            lpGWFldInfo = lpGWDHead->pFldInfo;
            lpGWFldInfo+=lpGWDHead->IndexFields[index][0];
            if (_fstricmp(lpGWFldInfo->Name,"%STREET_NAME")) goto NextIndex;
            SNField = *lpGWFldInfo;
            lpGWFldInfo = lpGWDHead->pFldInfo;
            lpGWFldInfo+=lpGWDHead->IndexFields[index][1];
            if (_fstricmp(lpGWFldInfo->Name,"%HOUSE_NUM")) goto NextIndex; 
            HNField = *lpGWFldInfo;
            PIDAddIndex = index;    
            if (abs(lpGWDHead->NumIndexFields[index])>2)
            {
            	lpGWFldInfo = lpGWDHead->pFldInfo;
	            lpGWFldInfo+=lpGWDHead->IndexFields[index][2];
            	ZIPField = *lpGWFldInfo;
            }
            else
            	ZIPField.Name[0] = 0;
            goto GetStreetNumIndex;
        } 
        NextIndex:;
    }
    GSSiMsgBox( GetFocus(), "No index containing %STREET_NAME and %HOUSE_NUM",
               "Cannot open address location database", MB_OK,0);
    goto ErrOut;
GetStreetNumIndex: 
    for (index=0;index<lpGWDHead->NumIndex;index++)
    {
        if (abs(lpGWDHead->NumIndexFields[index])>1)
        {
            lpGWFldInfo = lpGWDHead->pFldInfo;
            lpGWFldInfo+=lpGWDHead->IndexFields[index][0];
            if (_fstricmp(lpGWFldInfo->Name,"%STREET_NUM")) goto NextIndex2;
            SNumField = *lpGWFldInfo;
            lpGWFldInfo = lpGWDHead->pFldInfo;
            lpGWFldInfo+=lpGWDHead->IndexFields[index][1];
            if (_fstricmp(lpGWFldInfo->Name,"%HOUSE_NUM")) goto NextIndex2; 
            HNField = *lpGWFldInfo;
            StreetNumIndex = index;    
            if (abs(lpGWDHead->NumIndexFields[index])>2)
            {
            	lpGWFldInfo = lpGWDHead->pFldInfo;
	            lpGWFldInfo+=lpGWDHead->IndexFields[index][2];
            	ZIPField = *lpGWFldInfo;
            }
            else
            	ZIPField.Name[0] = 0;
            goto GetPID;
        } 
        NextIndex2:;
    }
GetPID: 
    hPIDPid = lpGWDHead->BTHandle[1]; 
    PidLen = lpGWDHead->lKeys[1];
    GlobalUnlock (hPIDAddDBSQL); 
    for (ifile=1;ifile<nf;ifile++)
    {
        str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,128,Fid);n++;}
    }
	memset(AddPrefix, 0, sizeof(AddPrefix));
    str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,128,Fid);n++;}
	{
		int n = 0;
		LPSTR pBeg = str;
		while (pBeg && n < 4)
		{
			LPSTR pEnd = strchr(str, ';');
			if (pEnd)
				*pEnd++ = 0;
			_fstrcpy(AddPrefix[n++], pBeg);
			pBeg = pEnd;
		}
	}
    str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,128,Fid);n++;} 
    _fstrcpy(AddUDIVar,str); 
    str[0]='#'; while (str[0]=='#') {str[0]=' ';fgetstring (str,128,Fid);n++;}  
    hAddDisplayLine = GSSiGlobAlloc ( 118,GHND,256);
    AddDisplayLine = GlobalLock (hAddDisplayLine);
    _fstrcpy (AddDisplayLine,str);      
    GlobalUnlock (hAddDisplayLine);
    GSSiClose (Fid);
    return (TRUE);
}

void    CloseAddressFilesPID (void)
{
    if (!SQLPtrADD)
    	return;
    GlobalUnlock (SQLPtrADD->OFHandle); 
    GlobalUnlock (FilePtrADD->FileHandle); 
    CloseDataFile (TRUE,&hPIDAddDBSQL); 
    SQLPtrADD = 0;
    if (hAddDisplayLine)
    {
        GSSiGlobFree (&hAddDisplayLine);
        hAddDisplayLine = 0;
    }
    hPIDAddDB = 0;
    return;

}

void SetSecondaryAddInput(HWND hWnd)
{
	hWndSecondaryAddInput = hWnd;
	return;
}

short DisplayStreetsPIDFromStreetNum (HWND hDlg,LONG HouseMin, LONG HouseMax, short OddEven, long InStreet,long WantMunicNum,USHORT EntryControl)
{
    short stName, ch,  pos;
    long    Offset;
    int	   TabStops[3]={500,600,700};
    char     AddText[256];
    long    ihouse, lhouse=1000000, range=HouseMax-HouseMin, MunicNum;
    char    DisplayAdd[256], str[128], LastStreet[64]; 
    LPGWDHEADER lpGWDHeadPID;   
    double  rval;
    MSG		msg;
    
    if (!StreetNumIndex)
    	return 0;
    ch = 0;          
    /* clear the address menu */
    lpGWDHeadPID = (LPGWDHEADER) GlobalLock (hPIDAddDB); 
    SetFieldValFromLong(lpGWDHeadPID,&SNumField,InStreet); 
    pos = BT_FIRST; 
FirstAdd:
    SetFieldValFromLong(lpGWDHeadPID,&HNField,HouseMin);
    if (ZIPField.Name[0])
    	SetFieldValFromChar(lpGWDHeadPID,&ZIPField,"",FALSE,FALSE); 
    GWDFormKey(lpGWDHeadPID,StreetNumIndex,TRUE,0,0);
    stName = BT_FIND (lpGWDHeadPID->BTHandle[StreetNumIndex],lpGWDHeadPID->pKeys[StreetNumIndex],
                              pos,BT_GE,(LPSTR)&Offset);   
    pos = BT_NEXT;
    while (!stName)
    {   if (InStreet != *(LPLONG)lpGWDHeadPID->pKeys[StreetNumIndex])
            stName = 31;
        else
        {   
        	FillGWDData (lpGWDHeadPID,Offset); 
            GMDGetNumericKeyVal (lpGWDHeadPID,StreetNumIndex,1,&rval); 
            ihouse = IDNINT(rval);
            if (ihouse > HouseMax)
            	stName = 31;
            else
            {    
	        	SQLPtrADD->st = 0;  
	        	SQLPtrADD->Offset = Offset; 
	        	SQLPtrADD->lastreadtime = ULONG_MAX;
                if (!AddRangeAll && !(!range && ihouse == HouseMin))
                { 
                    {
                        if (labs(ihouse-(HouseMax+HouseMin)/2)<labs(lhouse-(HouseMax+HouseMin)/2))
                        { 
                            short n=(short)SendDlgItemMessage (hDlg,IDM_STREET_MENU,LB_GETCOUNT,0,0); 
                            
                            if (n)
                            	SendDlgItemMessage (hDlg,IDM_STREET_MENU,LB_DELETESTRING,n-1,0);
                        }
                        else
                            goto NextAdd;  
                    }
                    lhouse = ihouse;
                } 
				GetTrueStreetName (InStreet, str, 0,0); 
                SetGlobalValue ("%STREET_NAME",str);
                ltoa (ihouse,str,10);
                SetGlobalValue ("%HOUSE_NUM",str); 
                _fstrcpy (str,"[%MUNIC_NUM]");
                ExpandText (str);
                MunicNum = atol (str);
                if (!WantMunicNum || MunicNum == WantMunicNum)
                {
	                GetMunicName (MunicNum,str,NULL);
	                SetGlobalValue ("%MUNIC_NAME",str);
	                _fstrcpy (AddText,(LPSTR)GlobalLock (hAddDisplayLine));
	                GlobalUnlock (hAddDisplayLine);
	                ExpandText (AddText);
	                sprintf (DisplayAdd,"%s\t%ld",AddText,Offset);
	                if (SendDlgItemMessage (hDlg,IDM_STREET_MENU,LB_FINDSTRINGEXACT,-1,(LPARAM)DisplayAdd) == LB_ERR)
	                	SendDlgItemMessage (hDlg,IDM_STREET_MENU,LB_ADDSTRING,0,(LPARAM)DisplayAdd);
	            }
        NextAdd:stName = BT_FIND (lpGWDHeadPID->BTHandle[StreetNumIndex],lpGWDHeadPID->pKeys[StreetNumIndex],
                                  BT_NEXT,BT_ANY,(LPSTR)&Offset);
				if (GSSiPeekMessage(&msg,GetDlgItem(hDlg,EntryControl),WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
					stName = 31;	                                                             
				if (hWndSecondaryAddInput)
				{
					if (GSSiPeekMessage(&msg, hWndSecondaryAddInput, WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE))
						stName = 31;
				}

            }
        }
    }

    GlobalUnlock (hPIDAddDB);
    return (0);
}



short DisplayStreetsPID (HWND hDlg,LONG HouseMin, LONG HouseMax, short OddEven, LPSTR InName,short nchar,long WantMunicNum,USHORT EntryControl)
{
    short stName, ch,  pos;
    long    Offset;
    int	   TabStops[3]={500,600,700};
    char     AddText[256];
    long    ihouse, lhouse=1000000, range=HouseMax-HouseMin, MunicNum;
    char    DisplayAdd[256], str[128], LastStreet[64]; 
    LPGWDHEADER lpGWDHeadPID;   
    double  rval;
    MSG		msg;

    ch = 0;          
    /* clear the address menu */
    SendDlgItemMessage (addMatchWnd,addMatchList,LB_RESETCONTENT,0,0);
    if (nchar<1)
    	return 0;    
    LastStreet[0]='\0';
    lpGWDHeadPID = (LPGWDHEADER) GlobalLock (hPIDAddDB); 
    _fmemset (lpGWDHeadPID->pKeys[PIDAddIndex],0,lpGWDHeadPID->lKeys[PIDAddIndex]);
    SetFieldValFromChar(lpGWDHeadPID,&SNField,InName,FALSE,FALSE); 
    pos = BT_FIRST; 
FirstAdd:
    SetFieldValFromLong(lpGWDHeadPID,&HNField,HouseMin);
    if (ZIPField.Name[0])
    	SetFieldValFromChar(lpGWDHeadPID,&ZIPField,"",FALSE,FALSE); 
    GWDFormKey(lpGWDHeadPID,PIDAddIndex,TRUE,0,0);
    _fmemmove (AddText,lpGWDHeadPID->pKeys[PIDAddIndex],32);
    stName = BT_FIND (lpGWDHeadPID->BTHandle[PIDAddIndex],lpGWDHeadPID->pKeys[PIDAddIndex],
                              pos,BT_GE,(LPSTR)&Offset);   
    _fmemmove (AddText,lpGWDHeadPID->pKeys[PIDAddIndex],32);
    pos = BT_NEXT;
    while (!stName)
    {   if (_fstrncmp (InName,lpGWDHeadPID->pKeys[PIDAddIndex],nchar))
            stName = 31;
        else
        {   
        	FillGWDData (lpGWDHeadPID,Offset); 
            GMDGetNumericKeyVal (lpGWDHeadPID,PIDAddIndex,1,&rval); 
            ihouse = IDNINT(rval);
            if (ihouse < HouseMin)
            {
                SetFieldValFromChar(lpGWDHeadPID,&SNField,lpGWDHeadPID->pKeys[PIDAddIndex],TRUE,FALSE); 
                pos = BT_FIRST;
                goto FirstAdd;
            }
            if (ihouse > HouseMax)
            {   
                LPSTR   LastSNChar, lpVal;
                short	i=0;
                
                LastSNChar = lpGWDHeadPID->pKeys[PIDAddIndex]; 
                while (i < SNField.Len && *LastSNChar)
                {
                	i++;
                	LastSNChar++;
                }
                if (i == SNField.Len)
                	LastSNChar--;
                *LastSNChar += 1;   
                lpVal = &lpGWDHeadPID->GWDData[SNField.Beg];
                _fmemmove (lpVal,lpGWDHeadPID->pKeys[PIDAddIndex],SNField.Len);
                pos = BT_FIRST;
                goto FirstAdd;
            }
            else
            {    
	        	SQLPtrADD->st = 0;  
	        	SQLPtrADD->Offset = Offset; 
	        	SQLPtrADD->lastreadtime = ULONG_MAX;
                GMDGetCharKeyVal (lpGWDHeadPID,PIDAddIndex,0,str); 
                if (!AddRangeAll && !(!range && ihouse == HouseMin))
                { 
                    if (!_fstricmp(LastStreet,str))
                    {
                        if (labs(ihouse-(HouseMax+HouseMin)/2)<labs(lhouse-(HouseMax+HouseMin)/2))
                        { 
                            short n=(short)SendDlgItemMessage (hDlg,addMatchList,LB_GETCOUNT,0,0); 
                            
                            if (n)
                            	SendDlgItemMessage (hDlg,addMatchList,LB_DELETESTRING,n-1,0);
                        }
                        else
                            goto NextAdd;  
                    }
                    _fstrcpy(LastStreet,str);
                    lhouse = ihouse;
                }
                SetGlobalValue ("%STREET_NAME",str);
                ltoa (ihouse,str,10);
                SetGlobalValue ("%HOUSE_NUM",str); 
                _fstrcpy (str,"[%MUNIC_NUM]");
                ExpandText (str);
                MunicNum = atol (str);
                if (!WantMunicNum || !MunicNum || MunicNum == WantMunicNum)
                {
	                GetMunicName (MunicNum,str,NULL);
	                SetGlobalValue ("%MUNIC_NAME",str);
	                _fstrcpy (AddText,(LPSTR)GlobalLock (hAddDisplayLine));
	                GlobalUnlock (hAddDisplayLine);
	                ExpandText (AddText);
	                sprintf (DisplayAdd,"%s\t%ld",AddText,Offset);
					AddAddMatch(hDlg, DisplayAdd);
	                //SendDlgItemMessage (hDlg,IDM_STREET_MENU,LB_ADDSTRING,0,
	                //                    (LPARAM)&DisplayAdd);
	            }
        NextAdd:stName = BT_FIND (lpGWDHeadPID->BTHandle[PIDAddIndex],lpGWDHeadPID->pKeys[PIDAddIndex],
                                  BT_NEXT,BT_ANY,(LPSTR)&Offset);
				if (GSSiPeekMessage(&msg,GetDlgItem(hDlg,EntryControl),WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
					stName = 31;
				if (hWndSecondaryAddInput)
				{
					if (GSSiPeekMessage(&msg, hWndSecondaryAddInput, WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE))
						stName = 31;
				}
                
            }
        }
    }
	AddAddMatch(hDlg, 0);

    GlobalUnlock (hPIDAddDB);
    return (0);
}


BOOL FAR PASCAL LOCATION_OFFSETMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{   
 static short   InLocChoice;
 short	OffsetChoice, OLDefault; 
 char   str[128], OffLineVal[32]; 
 static short   VPDefault;
 LPSTR  lpTAB, pUnits;
 
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
         cwCenter(hWndDlg, 0);
         /* initialize working variables                                */     
         InLocChoice = LocationChoice;
       	 GetCurVal (str,sizeof(str),IDS_FILEVPOFF); 
         VPDefault = FillList (hWndDlg,IDC_VP_OFFSET,str,0,0);
       	 GetCurVal (str,sizeof(str),IDS_FILEOFFLINE); 
         OLDefault = FillList (hWndDlg,IDC_OFF_LINE,str,0,0);  

         SendDlgItemMessage (hWndDlg,IDC_VOUNITS,CB_RESETCONTENT,0,0);
         SendDlgItemMessage (hWndDlg,IDC_VOUNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Feet"); 
         SendDlgItemMessage (hWndDlg,IDC_VOUNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Meters"); 
         SendDlgItemMessage (hWndDlg,IDC_VOUNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Miles"); 
         SendDlgItemMessage (hWndDlg,IDC_VOUNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Kilometers"); 
         SendDlgItemMessage (hWndDlg,IDC_VOUNITS,CB_SETCURSEL,(WPARAM)0,(LPARAM)0); 

         SendDlgItemMessage (hWndDlg,IDC_OLUNITS,CB_RESETCONTENT,0,0);
         SendDlgItemMessage (hWndDlg,IDC_OLUNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Feet"); 
         SendDlgItemMessage (hWndDlg,IDC_OLUNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Meters"); 
         SendDlgItemMessage (hWndDlg,IDC_OLUNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Miles"); 
         SendDlgItemMessage (hWndDlg,IDC_OLUNITS,CB_ADDSTRING,(WPARAM)0,(LPARAM) "Kilometers"); 
         SendDlgItemMessage (hWndDlg,IDC_OLUNITS,CB_SETCURSEL,(WPARAM)0,(LPARAM)0); 

         
         if (LocationChoice < 0)
            LocationChoice = VPDefault;
         GetGlobalCVal ("[%OFFSETLINEDIST]",OffLineVal,NULL);
         if ((pUnits = FirstAlpha (OffLineVal)))
         {  
         	char	SaveC = *pUnits;
         	
         	if (*pUnits)
		    	SendDlgItemMessage (hWndDlg,IDC_OLUNITS,CB_FINDSTRING,(WPARAM)-1,(LPARAM) pUnits); 
         	else
         		SetDlgItemText (hWndDlg,IDC_OLUNITS,"Meters");
         	*pUnits = 0;
         	SetDlgItemText (hWndDlg,IDC_OLVAL,OffLineVal); 
         	*pUnits = SaveC;
         }
         else
         	SetDlgItemText (hWndDlg,IDC_OLVAL,OffLineVal);
         
         OffsetChoice = 0;
         while (SendDlgItemMessage(hWndDlg,IDC_OFF_LINE,LB_GETTEXT,OffsetChoice,(DWORD)&str) != LB_ERR)
         {  
         	if ((lpTAB = _fstrchr (str,'\t')))
         	{
         		*lpTAB++ = 0;
         		
         		if (!_fstricmp (lpTAB,OffLineVal)) 
         		{
                	SendDlgItemMessage (hWndDlg,IDC_OFF_LINE,LB_SETCURSEL,OffsetChoice,0);
                	SetDlgItemText (hWndDlg,IDC_OLVAL,"");                     
         			break;
         		}  
         	}
         	OffsetChoice++;
         } 

Display: if (LocationChoice<0)
         {
            SendDlgItemMessage (hWndDlg,IDC_USE_CURRENT_ZOOM,BM_SETCHECK,TRUE,0L); 
            EnableWindow (GetDlgItem(hWndDlg,IDC_VP_OFFSET),FALSE); 
            EnableWindow (GetDlgItem(hWndDlg,IDC_FROM_CENTER),FALSE); 
            EnableWindow (GetDlgItem(hWndDlg,IDC_FROM_LIMITS),FALSE); 
            SendDlgItemMessage (hWndDlg,IDC_VP_OFFSET,LB_SETCURSEL,VPDefault,0);
         }
         else
         {
            SendDlgItemMessage (hWndDlg,IDC_USE_CURRENT_ZOOM,BM_SETCHECK,FALSE,0L);
            EnableWindow (GetDlgItem(hWndDlg,IDC_VP_OFFSET),TRUE); 
            EnableWindow (GetDlgItem(hWndDlg,IDC_FROM_CENTER),TRUE); 
            EnableWindow (GetDlgItem(hWndDlg,IDC_FROM_LIMITS),TRUE); 
            SendDlgItemMessage (hWndDlg,IDC_VP_OFFSET,LB_SETCURSEL,LocationChoice,0);
         } 
         if (OffsetFromLimits)
            SendDlgItemMessage (hWndDlg,IDC_FROM_LIMITS,BM_SETCHECK,TRUE,0L); 
         else
            SendDlgItemMessage (hWndDlg,IDC_FROM_CENTER,BM_SETCHECK,TRUE,0L); 
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDC_USE_CURRENT_ZOOM:
                if (SendDlgItemMessage (hWndDlg,IDC_USE_CURRENT_ZOOM,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
                    LocationChoice = -1;
                else  
                    LocationChoice = max (0,VPDefault);
                goto Display;  
                
            case IDC_VP_OFFSET: /* List box                           */
                 switch(HIWORD(wParam))
                 {   
                     case LBN_SELCHANGE:
                         SetDlgItemText (hWndDlg,IDC_VOVAL,"");
                         break;
                         
                     case LBN_DBLCLK: 
//                         IgnoreLbutton = TRUE;
                         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
                     break;
                 }
                 break; 
                
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
                
            case IDC_VOVAL:
                switch(HIWORD(wParam))
                {   
                 case EN_CHANGE:
                     GetDlgItemText (hWndDlg,IDC_VOVAL,str,sizeof(str));
                     if (str[0]) 
                        SendDlgItemMessage (hWndDlg,IDC_VP_OFFSET,LB_SETCURSEL,-1,0);                    
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
                
            case IDC_EDITVP:  
				 EditTextFile (hWndDlg,"[%DL]vpoffset.txt");
            
                 break;
                 
            case IDC_EDITOL:  
				 EditTextFile (hWndDlg,"[%DL]offline.txt");
            
                 break;
                 
            case IDOK: 
            {
                double  UFac[4]={FTM,1.0,5280*FTM,1000.0}; 
                short       iunits;
             
                GetDlgItemText(hWndDlg,IDC_VOVAL,str,sizeof(str));
                if (str[0])
                { 
                    LocationOffset = atof(str);
                    iunits=(short)SendDlgItemMessage(hWndDlg,IDC_VOUNITS,CB_GETCURSEL,0,0); 
                    LocationOffset *= UFac[iunits];
                }
                else
                {
                    LocationOffset = 0;
                    if (LocationChoice >= 0)
                    {
                        LocationChoice=(short)SendDlgItemMessage(hWndDlg,IDC_VP_OFFSET,
                                                  LB_GETCURSEL,0,0); 
                        SendDlgItemMessage(hWndDlg,IDC_VP_OFFSET,LB_GETTEXT,
                                            LocationChoice,(DWORD)str); 
                        lpTAB = _fstrrchr(str,'\t');
                        if (lpTAB)
                        {   short   l;
                        
                            lpTAB++;
                            LocationOffset = atof(lpTAB);
                            l = _fstrlen(str);
                            if (str[l-1]=='f' || str[l-1]== 'F') LocationOffset *= FTM;
                        }
                    } 
                }
                OffsetFromLimits = SendDlgItemMessage (hWndDlg,IDC_FROM_LIMITS,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
                GetDlgItemText(hWndDlg,IDC_OLVAL,str,sizeof(str));
                if (str[0])
                { 
                    GetDlgItemText (hWndDlg,IDC_OLUNITS,_fstrchr (str,0),16);
                    SetGlobalValue ("%OFFSETLINEDIST",str);
                }
                else
                {               
                    OffsetChoice=(short)SendDlgItemMessage(hWndDlg,IDC_OFF_LINE,LB_GETCURSEL,0,0); 
                    SendDlgItemMessage(hWndDlg,IDC_OFF_LINE,LB_GETTEXT,OffsetChoice,(DWORD)&str); 
                    lpTAB = _fstrrchr(str,'\t');
                    if (lpTAB)
	                    SetGlobalValue ("%OFFSETLINEDIST",++lpTAB);
                } 
                EndDialog(hWndDlg, TRUE);
            }
                break;
            case IDCANCEL:  
                LocationChoice = InLocChoice;  
                EndDialog(hWndDlg, FALSE);
                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

void SetIntMatchControl(HWND hWndDlg,UINT control)
{
	intMatchWnd = hWndDlg;
	if (control)
	{
		intMatchList = control;
	}
	else
	{
		intMatchList = IDC_INT_MATCHES;
	}
	return;
}

void AddIntMatch(HWND hWndDlg,LPSTR str)
{
	if (str)
		SendDlgItemMessage(intMatchWnd, intMatchList, LB_ADDSTRING, 0, (LPARAM)str);
	else if (hWndDlg != intMatchWnd)
	{
		int nInList = SendDlgItemMessage(intMatchWnd, intMatchList, LB_GETCOUNT, 0, 0);
		RECT	rect, prect;
		int		h = nInList * 16 + 6;

		GetWindowRect(GetDlgItem(intMatchWnd, intMatchList), &rect);
		ScreenRectToClientRect(intMatchWnd, &rect);
		MoveWindow(GetDlgItem(intMatchWnd, intMatchList), rect.left, rect.top, RECTWIDTH(&rect), h, TRUE);
		ShowWindow(GetDlgItem(intMatchWnd, intMatchList), SW_SHOW);
		GetWindowRect(GetDlgItem(intMatchWnd, intMatchList), &rect);
		GetWindowRect(intMatchWnd, &prect);
		prect.bottom = rect.bottom + 7;
		MoveWindow(intMatchWnd, prect.left, prect.top, RECTWIDTH(&prect), RECTHEIGHT(&prect),TRUE);
	}

	return;
}

BOOL FAR PASCAL LOC_INTERSECTMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	static	char	Street1[36]="", Street2[36]="";   
	char	str[256];
    int	    TabStops[2]={2000,2100};
	static	short	ls1,ls2;
	long	StreetNum1, StreetNum2;
	static	BOOL	Opened=FALSE;
 	short	Choice, FindOpt; 
 	LPSTR	lpTAB;  
 	char	MunName[66]; 
	HWND	hPar, hParDlg;
 	static	BOOL	OnlyPrime=TRUE, IgnoreChange=FALSE;
	
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
		SetSecondaryIntInput(0);
		intMatchList = IDC_INT_MATCHES;
		intMatchWnd = hWndDlg;
         SendDlgItemMessage (hWndDlg,IDM_STREET_MENU1,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
         SendDlgItemMessage (hWndDlg,IDM_STREET_MENU2,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
         SendDlgItemMessage (hWndDlg,IDC_INT_MATCHES,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
         cwCenter(hWndDlg, 0);
         /* initialize working variables                                */
	     if (!OpenStreetNameTable(FALSE))  
	     {
            GSSiMsgBox( GetFocus(), "Unable to open street name table", NULL,MB_OK|MB_ICONEXCLAMATION,0);
	        return FALSE;
	     }
         if (!OpenNetIntersect (NetworkID,FALSE,&Opened))
        	break; 
         SetDlgItemText(hWndDlg,IDC_STREET1,Street1);
		 if (!stricmp (Street2,"*"))
			 *Street2 = 0;
         SetDlgItemText(hWndDlg,IDC_STREET2,Street2); 
         ls1 = _fstrlen (Street1);
         ls2 = _fstrlen (Street2);  
   		 if (GetGlobalCVal ("[%WANTCITY]",MunName,NULL))
   		 {  
   		 	sprintf (str,"%s only",MunName);
   		 	SetDlgItemText (hWndDlg,IDC_IN_PRIMARY_CITY_ONLY,str);
         	ShowWindow (GetDlgItem (hWndDlg,IDC_IN_PRIMARY_CITY_ONLY),TRUE);
            SendDlgItemMessage (hWndDlg,IDC_IN_PRIMARY_CITY_ONLY,BM_SETCHECK,OnlyPrime,0L); 
         }
         SendDlgItemMessage (hWndDlg,IDC_TRYHARDER,BM_SETCHECK,TRUE,0L); 
		 hParDlg = GetParent(hWndDlg);
		 hPar = GetDlgItem(hParDlg, IDC_GEOCODE_OPERATION);

		 if (hPar)
		 {
			 RECT pRect;
			 RECT wRect;
			 GetWindowRect(hPar, &pRect);
			 GetWindowRect(hWndDlg, &wRect);
			// ClientRectToScreenRect(GetParent(hPar), &pRect);
			// ScreenRectToClientRect(hParDlg, &pRect);
			 MoveWindow(hWndDlg, pRect.left, pRect.top, RECTWIDTH(&pRect), RECTHEIGHT(&pRect), TRUE);
		 }
         PostMessage(hWndDlg, WM_COMMAND, IDC_TEST_INT, 0L);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

	case WM_DESTROY:
		CloseStreetNameTable();
		CloseNetIntersect(Opened);
		OnlyPrime = SendDlgItemMessage(hWndDlg, IDC_IN_PRIMARY_CITY_ONLY, (UINT)BM_GETCHECK, (WPARAM)0, (LPARAM)0L);
		break;

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDC_IN_PRIMARY_CITY_ONLY:
            	if (ls1 && ls2)     
		        	PostMessage(hWndDlg, WM_COMMAND, IDC_TEST_INT, 0L);
		        break;
		        
            case IDC_STREET1: /* Edit Control                            */
                 switch (HIWORD(wParam))
                 {  case EN_CHANGE:
					    if (IgnoreChange)
						{
							IgnoreChange = FALSE;
							PostMessage(hWndDlg, WM_COMMAND, IDC_TEST_INT, 0L);
							break;
						}
		         		SendDlgItemMessage (intMatchWnd,intMatchList,LB_RESETCONTENT,0,0); 
                        ls1 = GetDlgItemText (hWndDlg,IDC_STREET1,Street1,33); 
		         		FindOpt = SendDlgItemMessage (hWndDlg,IDC_TRYHARDER,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
                        if (DisplayStreetsINT (hWndDlg,IDM_STREET_MENU1,Street1,ls1,IDC_STREET1,FindOpt,TRUE)>=0) 
				        	PostMessage(hWndDlg, WM_COMMAND, IDC_TEST_INT, 0L);
                        break;

                 }
                 break;

            case IDC_STREET2: /* Edit Control                            */
                 switch (HIWORD(wParam))
                 {  case EN_CHANGE:
					 SendDlgItemMessage(intMatchWnd, intMatchList, LB_RESETCONTENT, 0, 0);
                        ls2 = GetDlgItemText (hWndDlg,IDC_STREET2,Street2,33);
		         		FindOpt = SendDlgItemMessage (hWndDlg,IDC_TRYHARDER,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
                        if (DisplayStreetsINT (hWndDlg,IDM_STREET_MENU2,Street2,ls2,IDC_STREET2,FindOpt,TRUE)>=0)
				        	PostMessage(hWndDlg, WM_COMMAND, IDC_TEST_INT, 0L);
                        break;

                 }
                 break;

            case IDM_STREET_MENU1: /* List box                           */
            case IDM_STREET_MENU2: /* List box                           */
                 switch(HIWORD(wParam))
                 {   
                 	short	Choice; 
                 	LPSTR	lpTAB;
                 	
                     case LBN_SELCHANGE:
                         Choice=(short)SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0);
                         SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETTEXT,Choice,(DWORD)&str); 
                         if ((lpTAB = _fstrchr (str,'\t')))
                         	*lpTAB = 0;   
                         if (LOWORD(wParam) == IDM_STREET_MENU1)
						 {
							IgnoreChange = TRUE;
                         	SetDlgItemText (hWndDlg,IDC_STREET1,str);
		         			SendDlgItemMessage (hWndDlg,IDM_STREET_MENU1,LB_RESETCONTENT,0,0); 
							if (lpTAB)
							{
								*lpTAB = '\t';
								SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_ADDSTRING,0,(DWORD)&str); 
							}
						 }
                         else
                         	SetDlgItemText (hWndDlg,IDC_STREET2,str);
                     break;
                 }
                 break;
            
            case IDOK:
            	 Choice = 0;
            	 goto GetInt;
            	 
            case IDC_INT_MATCHES: /* List box                           */
                 switch(HIWORD(wParam))
                 {   
                 	
                     case LBN_SELCHANGE:
                     case LBN_DBLCLK:
                         Choice=(short)SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0);
                 GetInt:
                         SendDlgItemMessage(hWndDlg,IDC_INT_MATCHES,LB_GETTEXT,Choice,(DWORD)&str); 
                         if (!(lpTAB = _fstrchr (str,'\t')))
                         	break;
                         lpTAB++;
                         sscanf (lpTAB,"%Flf,%Flf",&UserSpecifiedBasePoint.x,&UserSpecifiedBasePoint.y);
						 CurrentPoint = UserSpecifiedBasePoint;
						 CloseStreetNameTable ();
						 CloseNetIntersect (Opened);
		         		 OnlyPrime = SendDlgItemMessage (hWndDlg,IDC_IN_PRIMARY_CITY_ONLY,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
		                 EndDialog(hWndDlg, TRUE);
                     break;
                 }
                 break;

            case IDC_TEST_INT: 
            case IDC_TRYHARDER:
            {
				 LPADDMATCH	pMatch; 
				 HANDLE		hMatch=0; 
				 short		match,totmatch;
				 BOOL		Enable;
				 char		MunAbv[32];   
				 UINT		List2cntl=IDM_STREET_MENU2;		
				 long		MunicNum, WantMunic=0;
				 int		totmatchStart;
				 
				 SendDlgItemMessage(intMatchWnd, intMatchList, LB_RESETCONTENT, 0, 0);
         		 if (SendDlgItemMessage (hWndDlg,IDC_IN_PRIMARY_CITY_ONLY,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
         		 {
	         		 if (GetGlobalCVal ("[%WANTCITY]",MunName,NULL))
	         		 	WantMunic = GetMunicFromName (MunName);
                 }
                 else
                 	*MunName = 0;
				 GetDlgItemText (hWndDlg,IDC_STREET2,Street2,32);
				 if (!stricmp (Street2,"*"))
					 List2cntl = 0;
				 match = INT_MATCH_DLG (hWndDlg,IDM_STREET_MENU1,List2cntl,WantMunic,0,2,&hMatch,&StreetNum1,&StreetNum2,
					 						GetDlgItem(hWndDlg,IDC_STREET1),GetDlgItem(hWndDlg,IDC_STREET2)); 
				 if (match)                                                                                     
				 	pMatch = (LPADDMATCH)GlobalLock (hMatch); 
                 totmatch = match;
				 while (match--)
				 {  
					GetTrueStreetName (pMatch->StreetNum1, str, 0,0); 
					_fstrcat (str," at ");
					GetTrueStreetName (pMatch->StreetNum2, _fstrchr(str,0), 0,0); 
					if (!WantMunic || WantMunic == pMatch->Munic)
					{ 
						GetMunicName (pMatch->Munic,MunName,MunAbv);
						sprintf (_fstrchr(str,0),",%s\t%20.8f,%20.8f",MunName,pMatch->Point.x,pMatch->Point.y);
						AddIntMatch(hWndDlg, str);
	                }
                    pMatch++;
				 } 
				 if (totmatch)
					 AddIntMatch(hWndDlg, 0);
				 GSSiGlobUlFree(&hMatch);
				 totmatchStart = totmatch;
         		 if (SendDlgItemMessage (hWndDlg,IDC_TRYHARDER,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
         		 {
	                 GetDlgItemText (hWndDlg,IDC_STREET1,Street1,33);
	                 GetDlgItemText (hWndDlg,IDC_STREET2,Street2,33); 
	                 WaitCursor (1);
					 match = INT_MATCH (Street1,Street2,MunName,0,3,&hMatch,&StreetNum1,&StreetNum2,&MunicNum);     
					 WaitCursor (-1); 
					 if (match)                                                                                     
					 	pMatch = (LPADDMATCH)GlobalLock (hMatch); 
					 while (match--)
					 {  
						GetTrueStreetName (pMatch->StreetNum1, str, 0,0); 
						_fstrcat (str," at ");
						GetTrueStreetName (pMatch->StreetNum2, _fstrchr(str,0), 0,0); 
						if (!WantMunic || WantMunic == pMatch->Munic)
						{ 
							GetMunicName (pMatch->Munic,MunName,MunAbv);
							sprintf (_fstrchr(str,0),",%s\t%20.8f,%20.8f",MunName,pMatch->Point.x,pMatch->Point.y);
							if (SendDlgItemMessage(intMatchWnd, intMatchList, LB_FINDSTRINGEXACT, -1, (LPARAM)str) == LB_ERR)
							{
								AddIntMatch(hWndDlg, str);
								totmatch++;
							}
						}
	                    pMatch++;
					 } 
			 		 GSSiGlobUlFree (&hMatch); 
				 }
				 if (totmatch > totmatchStart)
					 AddIntMatch(hWndDlg, 0);

				 if (totmatch == 1)
				 	Enable=TRUE;
				 else
				 	Enable=FALSE;
	 			 EnableWindow (GetDlgItem(hWndDlg,IDOK),Enable);

		 	}
                 break;
                  
/*				 while (match--)
				 {  
					GetTrueStreetName (pMatch->StreetNum1, str, 0); 
					_fstrcat (str," at ");
					GetTrueStreetName (pMatch->StreetNum2, _fstrchr(str,0), 0);
					sprintf (_fstrchr(str,0),"\t%10ld%10ld%10ld%20.8f%20.8f",pMatch->IntID,pMatch->StreetNum1,pMatch->StreetNum2,
							 					pMatch->Point.x,pMatch->Point.y);
                    SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_ADDSTRING,0,(LPARAM)str);
                    pMatch++;
				 } */
		 		 break;

            case IDCANCEL:
				 CloseStreetNameTable ();
				 CloseNetIntersect (Opened);
         		 OnlyPrime = SendDlgItemMessage (hWndDlg,IDC_IN_PRIMARY_CITY_ONLY,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
                 EndDialog(hWndDlg, FALSE);
                 break;
           }
    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL ADDEDIT_HELPERMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	short	i;
    static	BOOL	First=TRUE;
	static RECT		WindRect; 
	static	int		nrecslast=0, LastChoice;
	long	MunicNum;
   	char	str[256],str2[256];
	int   	TabStops[6]={50,250,300,350,530,2000};
	static	HANDLE	hHelp1=0, hHelp2=0;
	HELP1DATA	Help1Data;
	static	char	CurAddress[256], CurMunic[64];
   	
   	static	int	DisplayedStreets=0;  
   	COLORREF	DrawStreetColors[4]={RGB(255,0,0),RGB(0,255,0),RGB(0,255,255),RGB(255,255,0)};
	
  switch(Message)
   {
    case WM_INITDIALOG: 
		 SendDlgItemMessage (hWndDlg,IDC_LIST1,LB_SETTABSTOPS,6,(LPARAM)&TabStops);
		 SendDlgItemMessage (hWndDlg,IDC_LIST2,LB_SETTABSTOPS,6,(LPARAM)&TabStops);
                                    
		 hWndAddEditHelper=hWndDlg;   
		 if (!First)
		 {  
			SetWindowPos(hWndDlg,HWND_TOP,WindRect.left,WindRect.top,
									   WindRect.right-WindRect.left,
									   WindRect.bottom-WindRect.top,SWP_NOZORDER);
		 } 
		 First = FALSE; 
		 if (GetGlobalCVal ("[%ADDEDITHELPERMACRO]",str,0))
    		 SendDlgItemMessage (hWndDlg,IDC_REPORT,BM_SETCHECK,TRUE,0); 
		 GetCurVal (str,sizeof(str),IDS_FILEADDEDITHELP);  
         SetDlgItemText (hWndDlg,IDC_SOURCE_FILE,str);
		 if (ExistFile (str))
			PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

	case WM_DESTROY:
		BT_CLOSEANDDELETE (&hHelp1);
        BT_CLOSEANDDELETE (&hHelp2);
		break;
    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
           	case IDC_VIEW:
				_fstrcpy (str,"NOTEPAD.EXE ");
				_fstrcat (str,"[%%DL]address\\missrang.txt"); 
				ExpandText (str);
				WinExec (str,SW_SHOWMAXIMIZED);  
				break;
				
            case IDC_LOCATE_SOURCE:
            { 
                 *str=0;
                 if (!GetFileName3 (hWndDlg,str,IDS_FILTERGWD,IDS_FILEADDEDITHELP)) break;   
                 SetDlgItemText (hWndDlg,IDC_SOURCE_FILE,str);
			}
			break;

			case IDOK:
			{
				char	mess[256];
				long    Offset, RecNum=1, nRecs, nLoaded=0;
				LPGWDHEADER lpGWDHead;   
				LPGWFLDINFO lpGWFldInfo;  
                 LPADDMATCH	pMatch;
				 LPADDMATCHEDITREC pAMER;
            	 unsigned short len;  
			     BTVARDESC   BTVar[5];
				 HCURSOR	hcurSave;   
            	 char	TrueName[40]; 
            	 LPLONG	pRecnum;
				 //BTHEAD BTHead;  
				 HANDLE	hMatch=0;
				 long	StreetNum, ii=0, iadd;
				 short	match;
				 OFSTRUCT	OFStruct;
				 HFILE	FidOut; 
				 long	TotRecs, NumRecs=0, Inc=0, LastRange, CurCount, Range, LastCount, BegRange, RangeWidth=100, NearAdd;
				 char	LastName[66], LastCity[34], Near[16];
				 short	st, idum, pos=BT_FIRST; 
				 HANDLE	hDBDest;  
				 BOOL	UsePointBased=TRUE,UseNetBased=TRUE;
				 HELP1KEY	Help1Key, LastHelp1Key;
				 char	StartRangeKey[128];
				 BOOL	DoAddMatch=FALSE;
				 struct {   
				 			long	Count;
				 			char	Name[64];   
				 			char	City[32];
				 			long	BegRange,
				 					EndRange;
				 		} Help2Key;
				 
            	 
 	   			 SendDlgItemMessage (hWndDlg,IDC_LIST1,LB_RESETCONTENT,0,0);
	   			 SendDlgItemMessage (hWndDlg,IDC_LIST2,LB_RESETCONTENT,0,0);

                 GetDlgItemText (hWndDlg,IDC_SOURCE_FILE,str,250);
	    		 hDBDest = OpenGWDatabase (str,BT_READ);
	    		 if (!hDBDest)
	    		 	break;
				 OpenAddressFilesPID (hWndMain);

				hcurSave = GSSiSetCursor(LoadCursor(NULL, IDC_WAIT));
			    BTVar[0].BT_VARTYP=BT_CHAR;
			    BTVar[0].BT_VARLEN=64;
			    BTVar[0].BT_VAROFF=0;
			    BTVar[1].BT_VARTYP=BT_CHAR;
			    BTVar[1].BT_VARLEN=32;
			    BTVar[1].BT_VAROFF=64;
			    BTVar[2].BT_VARTYP=BT_INTEGER;
			    BTVar[2].BT_VARLEN=4;
			    BTVar[2].BT_VAROFF=96;
			    BTVar[3].BT_VARTYP=BT_INTEGER;
			    BTVar[3].BT_VARLEN=4;
			    BTVar[3].BT_VAROFF=100;
			    BT_CREATE ("addhelp1.btr", sizeof(HELP1DATA), FALSE, 4, 1,(LPBTVARDESC) BTVar,FALSE, 0, 0, FALSE);
    			hHelp1 = BT_OPEN ("addhelp1.btr", 0, BT_WRITE, 0);
			    BTVar[0].BT_VARTYP=BT_INTEGER;
			    BTVar[0].BT_VARLEN=4;
			    BTVar[0].BT_VAROFF=0;
			    BTVar[1].BT_VARTYP=BT_CHAR;
			    BTVar[1].BT_VARLEN=64;
			    BTVar[1].BT_VAROFF=4;
			    BTVar[2].BT_VARTYP=BT_CHAR;
			    BTVar[2].BT_VARLEN=32;
			    BTVar[2].BT_VAROFF=68;
			    BTVar[3].BT_VARTYP=BT_INTEGER;
			    BTVar[3].BT_VARLEN=4;
			    BTVar[3].BT_VAROFF=100;
			    BTVar[4].BT_VARTYP=BT_INTEGER;
			    BTVar[4].BT_VARLEN=4;
			    BTVar[4].BT_VAROFF=104;
			    BT_CREATE ("addhelp2.btr", sizeof(HELP1KEY), FALSE, 5, 1,(LPBTVARDESC) BTVar,FALSE, 0, 0, FALSE);
    			hHelp2 = BT_OPEN ("addhelp2.btr", 0, BT_WRITE, 0);
			    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest); 
//        		pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData;
                pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData[lpGWDHead->pFldInfo->Len];
				CreateStatusWind (hWndMain,1,"Scanning source");
				nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[1]);
	        	while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[1],lpGWDHead->pKeys[1],pos,BT_ANY,(LPSTR)&Offset))  
	        	{
            		pos = BT_NEXT;
	        		len = FillGWDData (lpGWDHead,Offset);
					if (pAMER->AM.LocationCode)
					 	break;
					else
					{  
					 	NumRecs++;
					 	strncpy0 (Help1Key.Name,pAMER->Street,64); 
					 	strncpy0 (Help1Key.City,pAMER->City,32); 
					 	if (_fstrstr (pAMER->House,"BLOCK"))	
					 		iadd = 50;
					 	else
					 		iadd = 0;
					 	Help1Key.House = atol (pAMER->House)+iadd;
					 	Help1Key.Inc = Inc++;  
						Help1Data.MatchCode = pAMER->AM.MatchCode;
						Help1Data.Removed = FALSE;
						strncpy (Help1Data.Note,pAMER->Note,60);
//						strncpy (Help1Data.OrigKey,pAMER->OrigKey,64);
						GMDGetCharFieldVal (lpGWDHead,0,str);
						strncpy (Help1Data.OrigKey,str,64);
					 	BT_PUT (hHelp1,(LPSTR)&Help1Key,(LPSTR)&Help1Data);
					}
					StatusWindowUpdate (NULL,NULL, nRecs, ++nLoaded);
        		}  
				DestroyStatusWindow(0);  
        		LastRange = -1;
        		*LastName = 0;   
        		*LastCity = 0;
        		LastCount = 0;
        		nLoaded = 0;
				CreateStatusWind (hWndMain,1,"Defining ranges");
				nRecs = BT_NUM_IN_INDEX (hHelp1);
        		pos = BT_FIRST;
        		while (ContinueProcessing && !BT_FIND (hHelp1,(LPSTR)&Help1Key,pos,BT_ANY,(LPSTR)&Help1Data))
        		{ 
        			pos = BT_NEXT; 
        			Range = Help1Key.House/RangeWidth;
        			if (_fstricmp (LastName,Help1Key.Name) || 
        				_fstricmp (LastCity,Help1Key.City) ||
        				labs (LastRange - Range) > 1)
        			{
        				if (LastCount)
        				{ 
        					Help2Key.Count = -LastCount;
        					strncpy0 (Help2Key.Name,LastName,64); 
        					strncpy0 (Help2Key.City,LastCity,32); 
        					Help2Key.BegRange = BegRange; 
        					Help2Key.EndRange = LastRange;
        					BT_PUT (hHelp2,(LPSTR)&Help2Key,(LPSTR)&LastHelp1Key); 
        					BegRange = Range;
        				}
        				else
        					BegRange = Range;
        				strncpy0 (LastName,Help1Key.Name,64);
        				strncpy0 (LastCity,Help1Key.City,32);
						LastHelp1Key = Help1Key;
        				LastCount = 1;
        			}                 
        			else
        			{
        				LastCount++;
        			}
       				LastRange = Range;
					StatusWindowUpdate (NULL,NULL, nRecs, ++nLoaded);
        		}  
        		if (LastCount)
        		{ 
        			Help2Key.Count = -LastCount;
        			strncpy0 (Help2Key.Name,LastName,64); 
        			strncpy0 (Help2Key.City,LastCity,32); 
        			Help2Key.BegRange = BegRange; 
        			Help2Key.EndRange = LastRange;
        			BT_PUT (hHelp2,(LPSTR)&Help2Key,(LPSTR)&LastHelp1Key); 
        			BegRange = Range;
        		}
				DestroyStatusWindow(0);  
        		
        		NumRecs = 0;  
        		FidOut = GSSiOpenFile ("[%%DL]address\\missrang.txt",&OFStruct,OF_CREATE);  
        		fputstring ("RECNUM\tSTREET\tCITY\tMIDADD\tLOWADD\tHIGHADD\tNEAREST\tCOUNT",FidOut);
                pos = BT_FIRST;
				//CreateStatusWindow (hWndMain,1,"Writing output file");
				nRecs = BT_NUM_IN_INDEX (hHelp2);
				TotRecs = 0;
        		while (ContinueProcessing && !BT_FIND (hHelp2,(LPSTR)&Help2Key,pos,BT_ANY,(LPSTR)&LastHelp1Key))
        		{  
        			pos = BT_NEXT; 
        			if (Help2Key.Count <= -1)
        			{   
        				NearAdd = 0;
						_fstrcpy (Near,"- None -");  
						if (DoAddMatch)
						{
							match = ADD_MATCH (Help2Key.Name,ltoa(Help2Key.BegRange*100,str,10),Help2Key.City,"",0,&hMatch,&StreetNum,&MunicNum,UsePointBased,UseNetBased,0,0,NULL); 
							if (match>0)
							{
					 			pMatch = (LPADDMATCH)GlobalLock (hMatch);  
					 			NearAdd = pMatch->HouseNum;
					 			ltoa (pMatch->HouseNum,Near,10);
								GetTrueStreetName (pMatch->StreetNum, TrueName, 0,0); 
						   		GlobalUnlock (hMatch);
							}
							GSSiGlobFree (&hMatch);
						}
						sprintf (StartRangeKey,"%ld|%s|%s",LastHelp1Key.House,LastHelp1Key.Name,LastHelp1Key.City);
        		/*		sprintf (str,"%ld\t%s\t %s\t%ld\t%ld\t%ld\t%ld\t(%s)\t%ld", 
        													  RecNum++,
        													  Help2Key.Name,Help2Key.City,
        													  (Help2Key.BegRange*RangeWidth+(Help2Key.EndRange+1)*RangeWidth)/2,
        													  Help2Key.BegRange*RangeWidth,Help2Key.EndRange*RangeWidth+RangeWidth-1,
        													  NearAdd,StartRangeKey,
        													  -Help2Key.Count);
        				fputstring (str,FidOut);*/
        				sprintf (str,"%ld\t%s\t%s\t%ld\t(%s)", 
        													  -Help2Key.Count,
        													  Help2Key.Name,Help2Key.City,
        													  (Help2Key.BegRange*RangeWidth+(Help2Key.EndRange+1)*RangeWidth)/2,
															  StartRangeKey);
                		SendDlgItemMessage (hWndDlg,IDC_LIST1,LB_ADDSTRING,0,(LPARAM)str);
        				NumRecs++;
						TotRecs += (-Help2Key.Count);
	        		}
	        		else
	        			break;  
	        		sprintf (mess,"%ld:%s %s",-Help2Key.Count,Help2Key.Name,Help2Key.City);
				//	StatusWindowUpdate (NULL,mess, nRecs, NumRecs);
        		}  
				DestroyStatusWindow(0);  
				ContinueProcessing = TRUE;
        		GSSiClose (FidOut);
        		sprintf (str,"%ld records (summarizing %ld unmatched records) dumped to %s",NumRecs,TotRecs,OFStruct.szPathName); 
        		SetDlgItemText (hWndDlg,IDC_MESS,str);
				GSSiSetCursor(hcurSave);
	 			EnableWindow (GetDlgItem(hWndDlg,IDC_NEXT),FALSE);  
    		    GlobalUnlock (hDBDest);
			 	CloseGWDatabase (hDBDest);
			 	hDBDest = 0;  
				CloseSegMaxIndex (TRUE);
                CloseAddressFilesPID();
			 	EnableWindow (GetDlgItem(hWndDlg,IDC_VIEW),TRUE);
			}
                 break;
            
			case IDC_LIST2:
                 switch(HIWORD(wParam))
                 {   
                     case LBN_SELCHANGE: 
					 {
                         int	Inc, IHouse, Choice=(int)SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0);
						 LPSTR	pLoc,pEnd,pStreet,pMunic;
						 HELP1KEY	Help1Key;

                         SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETTEXT,Choice,(DWORD)&str);
						 pLoc = strrchr (str,'\t');
						 *pLoc++ = 0;
						 SetGlobalValue ("%ORIGFILEKEY",pLoc);
 						 pLoc = strrchr (str,'\t');
						 *pLoc++ = 0;
						 Inc = atoi (pLoc);
						 pStreet = strchr (str,'\t');
						 *pStreet++ = 0;
						 IHouse = atoi (str);
						 pMunic = strchr (pStreet,'\t');
						 *pMunic++ = 0;
						 pEnd = strchr (pMunic,'\t');
						 *pEnd = 0;
						 if (IHouse)
							sprintf (CurAddress,"%i %s",IHouse,pStreet);
						 else
							strcpy (CurAddress,pStreet);
						 strcpy (CurMunic,pMunic);
						 if (SendDlgItemMessage (hWndDlg,IDC_REPORT,BM_GETCHECK,0,0))
						 {
							 if (GetGlobalCVal ("[%ADDEDITHELPERMACRO]",str2,0))
							 {
								ExpandText (str2);
								if (!stricmp (str2,"%REMOVE"))
								{
									int	pos=BT_FIRST,cond=BT_GE, st=0,nrecs;
									HELP1KEY Help1KeyTest;
									
									Help1Key.House = IHouse;
									Help1Key.Inc = Inc;
									strncpy (Help1Key.Name,pStreet,sizeof(Help1Key.Name));
									strncpy (Help1Key.City,pMunic,sizeof(Help1Key.City));
									memmove (&Help1KeyTest,&Help1Key,sizeof(HELP1KEY));
									BT_FIND (hHelp1,(LPSTR)&Help1Key,BT_FIRST,BT_EQ,(LPSTR)&Help1Data);
									Help1Data.Removed = TRUE;
									BT_PUT (hHelp1,(LPSTR)&Help1Key,(LPSTR)&Help1Data);
									GetDlgItemText (hWndDlg,IDC_SOURCE_FILE,str,250);
									sprintf (str2,"$GMDDELETE(%s,KEY=%s)",str,Help1Data.OrigKey);
									ExpandText (str2);
									goto ShowList1;

								}
							 }
						 }
						 else if (SendDlgItemMessage (hWndDlg,IDC_LOCATE,BM_GETCHECK,0,0))
						 {
 							 if (!CurrentConfig)
								SetConfig (1);
							 SetViewport (*pCommandViewport);
							 hWndDigControl = hWndDlg; 
							 WantDigWorldCtlPnt = TRUE;
							 AddGraphicsCmd (CurView->hWnd,"[%C]=$RESET();90",FALSE,0); 
						 }
			             EnableWindow (GetDlgItem(hWndDlg,IDC_SETTOUNMATCHABLE),TRUE);
			             EnableWindow (GetDlgItem(hWndDlg,IDC_REPORT),TRUE);
			             EnableWindow (GetDlgItem(hWndDlg,IDC_LOCATE),TRUE);
						 
					 }
					 break;
				 }
				 break;
			case IDC_LIST1:
                 switch(HIWORD(wParam))
                 {   
                     case LBN_SELCHANGE: 
					 {
						 int	pos,cond, nrecs;
						 short	MatchCode;
                         int	Choice=(int)SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0);
						 HELP1KEY	Help1Key;
						 LPSTR	pLoc,pEnd;

						 LastChoice = Choice;
ShowList1:
						 pos = BT_FIRST;
						 cond = BT_GE;
                         SendDlgItemMessage(hWndDlg,IDC_LIST1,LB_GETTEXT,LastChoice,(DWORD)&str);
						 *LastChr (str) = 0;
						 nrecslast = nrecs = atoi (str);
						 pLoc = strrchr (str,'\t');
						 pLoc++;
						 pEnd = strchr (pLoc++,'|');
						 *pEnd++ = 0;
						 Help1Key.House = atoi (pLoc);
						 pLoc = pEnd;
						 pEnd = strchr (pLoc,'|');
						 *pEnd++ = 0;
						 strncpy0 (Help1Key.Name,pLoc,64);
						 strncpy0 (Help1Key.City,pEnd,32);
						 Help1Key.Inc = 0;
			   			 SendDlgItemMessage (hWndDlg,IDC_LIST2,LB_RESETCONTENT,0,0);
						 while (nrecs--)
						 {
							if (!BT_FIND (hHelp1,(LPSTR)&Help1Key,pos,cond,(LPSTR)&Help1Data))
							{
								sprintf (str,"%ld\t%s\t%s\t%i\t%s\t%i\t%s",Help1Key.House,Help1Key.Name,Help1Key.City,Help1Data.MatchCode,Help1Data.Note,Help1Key.Inc,Help1Data.OrigKey);
		                		if (!Help1Data.Removed)
									SendDlgItemMessage (hWndDlg,IDC_LIST2,LB_ADDSTRING,0,(LPARAM)str);
							}
							pos = BT_NEXT;
							cond = BT_ANY;
						 }
					 }
					 break;
				 }
				 break;
            
            case IDC_SETWORLDCOORD:
				 hWndDigControl = 0;  
				 WantDigWorldCtlPnt = FALSE;    
				 AddUserAddress (CurAddress,CurMunic,DigWorldControlPoint,FALSE);
				 sprintf (str,"%s stored as manually located",CurAddress);
				 MessageBox (hWndDlg,str,"",MB_OK);
				 break;

            case IDC_SETTOUNMATCHABLE:
				{
					DPOINT	UnmatchablePoint;

					GetGlobalPVal ("[%BADCOORDPOINT]",NULL,&UnmatchablePoint);
					AddUserAddress (CurAddress,CurMunic,UnmatchablePoint,TRUE);
					sprintf (str,"%s stored as un-matchable",CurAddress);
					MessageBox (hWndDlg,str,"",MB_OK);
				}
			     EnableWindow (GetDlgItem(hWndDlg,IDC_SETTOUNMATCHABLE),FALSE);
				 break;
           case IDCANCEL:      
				 GetWindowRect (hWndDlg,&WindRect);
                 DestroyWindow (hWndDlg);
				 hWndAddEditHelper=0;
				 ClearFullWindowBitmap (0);
                 break;
           }
    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL ABVEDITMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 

   	OFSTRUCT	OFStruct;
	static	HFILE	Fid;
	static	HANDLE	hBT, hName;
	static	struct	{short	Type;
			 		 char	FullName[14];} Key;    
	char	word[14], wantword[14],str[260]; 
	char	OutString[180];
	BOOL	HaveLine;
	static	BOOL	Update=FALSE; 
	static	short	CurrentType=2; 
	short	l,type,st,i,wanttype;  
	LPSTR	pstr, pName;
	UINT	AbvFields[11]={IDC_STANDARD_ABV,
						   IDC_ALT_ABV1,
						   IDC_ALT_ABV2,
						   IDC_ALT_ABV3,
						   IDC_ALT_ABV4,
						   IDC_ALT_ABV5,
						   IDC_ALT_ABV6,
						   IDC_ALT_ABV7,
						   IDC_ALT_ABV8,
						   IDC_ALT_ABV9,
						   IDC_ALT_ABV10};
    
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
    {
		BTVARDESC	BTVar[3];
    	 
    	Update = FALSE;
    	SendDlgItemMessage (hWndDlg,IDC_TYPE_NAME,BM_SETCHECK,TRUE,0); 
		SetAddressDir();
        hName = GSSiGlobAlloc ( 567,GMEM_MOVEABLE,256);
        pName = GlobalLock (hName);
		_fstrcpy (pName,AddMatchDir); 
        _fstrcat (pName,"\\AbbrName.txt"); 
        Fid = GSSiOpenFile (pName,&OFStruct,OF_READ); 
        GlobalUnlock (hName); 
        if (Fid == HFILE_ERROR)
        {
        	GSSiGlobUlFree (&hName);
        	return FALSE;
        }
		GSSiGetTempFileName (0,"gm",0,(LPSTR)str);
	
		BTVar[0].BT_VARTYP=BT_INTEGER;
		BTVar[0].BT_VARLEN=2;
		BTVar[0].BT_VAROFF=0;
		BTVar[1].BT_VARTYP=BT_CHAR;
		BTVar[1].BT_VARLEN=12;
		BTVar[1].BT_VAROFF=2;
		BT_CREATE (str, 132, FALSE, 2, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
		hBT = BT_OPEN (str, 0, BT_WRITE, 0); 
		type = 0;
		HaveLine = FALSE;
		while (fgetstring (str,256,Fid))
		{
			if (*str != '*')
			{ 
				HaveLine = TRUE; 
				Key.Type = type;
				_fstrncpy (word,str,12);
				word[12]=0;
				Truncate (word);
				_fstrncpy (Key.FullName,word,12);
				l = _fstrlen (str);
				if (l < 144)
					_fmemset (&str[l],0,144-l);
				BT_PUT (hBT,(LPSTR)&Key,&str[12]);
			}  
			else if (HaveLine)
				type++;
		} 
		Key.FullName[0]=0;
		GSSiClose (Fid);  
		SetFocus (GetDlgItem(hWndDlg,IDC_FULLWORD));
    }
        break; /* End of WM_INITDIALOG                                 */
    
   
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

         switch(LOWORD(wParam))

         {  
         	case IDC_TYPE_TYPE:
         	case IDC_TYPE_DIR:
         	case IDC_TYPE_NAME:
         		 SetDlgItemText (hWndDlg,IDC_FULLWORD,""); 
         		 SetFocus (GetDlgItem(hWndDlg,IDC_FULLWORD));
         		 SetFocus (GetDlgItem(hWndDlg,IDC_STANDARD_ABV));
         		 SetFocus (GetDlgItem(hWndDlg,IDC_FULLWORD)); 
         		 switch (wParam)
         		 {
         		 	case IDC_TYPE_TYPE:
         		 		CurrentType = 1;
         		 		break;
         		 	case IDC_TYPE_DIR:
         		 		CurrentType = 0;
         		 		break;
         		 	case IDC_TYPE_NAME:
         		 		CurrentType = 2;
         		 		break;
         		 }
         		 break;
         	
         	case IDC_NEXT:
         		 type = Key.Type;	 
				 st = BT_FIND (hBT,(LPSTR)&Key,BT_NEXT,BT_ANY,str); 
				 if (!st && type == Key.Type) 
				 {
                 	_fstrncpy (word,Key.FullName,12);  
                 	word[12]=0;
                 	Truncate (word);
                 	SetDlgItemText (hWndDlg,IDC_FULLWORD,word);
		            EnableWindow (GetDlgItem(hWndDlg,IDC_PRIOR),TRUE);
                 } 
				 else
				 {
         		 	SetDlgItemText (hWndDlg,IDC_FULLWORD,"");
		            EnableWindow (GetDlgItem(hWndDlg,IDC_NEXT),FALSE); 
		         }
         		 break;
         		 
         	case IDC_PRIOR:
         		 type = Key.Type;	 
				 st = BT_FIND (hBT,(LPSTR)&Key,BT_PRIOR,BT_ANY,str); 
				 if (!st && type == Key.Type) 
				 {
                 	_fstrncpy (word,Key.FullName,12);
                 	word[12]=0;
                 	Truncate (word);
                 	SetDlgItemText (hWndDlg,IDC_FULLWORD,word);
		            EnableWindow (GetDlgItem(hWndDlg,IDC_NEXT),TRUE);
                 } 
				 else
				 {
         		 	SetDlgItemText (hWndDlg,IDC_FULLWORD,"");
		            EnableWindow (GetDlgItem(hWndDlg,IDC_PRIOR),FALSE); 
		         }
         		 break;
         		 
         	case IDC_FINDSTRING:
                 switch (HIWORD(wParam))
                 {	
                 	case EN_CHANGE:  
                 	if (GetDlgItemText (hWndDlg,IDC_FINDSTRING,word,16))
		            	EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
		            else
		            	EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
                 }
                 break;
            
            case IDOK: 
            {
            	 short	pos=BT_FIRST;
            	 
            	 GetDlgItemText (hWndDlg,IDC_FINDSTRING,str,12);
            	 str[12]=0;
            	 _fstrcpy (wantword,str);
	             while (!BT_FIND (hBT,(LPSTR)&Key,pos,BT_ANY,str))
	             { 
	             	pos = BT_NEXT; 
	                pstr = str;
	             	for (i=0;i<12;i++,pstr+=12)
	             	{
	                 	_fstrncpy (word,pstr,12);
	                 	word[12]=0;
	                 	Truncate (word);
	                 	if (!_fstricmp (word,wantword))
	                 	{   
	                 		_fstrncpy (word,(LPSTR)&Key.FullName,12);
	                 		word[12]=0;
	                 		Truncate (word);  
             				SendDlgItemMessage (hWndDlg,IDC_TYPE_DIR,BM_SETCHECK,FALSE,0);
             				SendDlgItemMessage (hWndDlg,IDC_TYPE_TYPE,BM_SETCHECK,FALSE,0);
             				SendDlgItemMessage (hWndDlg,IDC_TYPE_NAME,BM_SETCHECK,FALSE,0);
	                 		switch (Key.Type)
	                 		{
	                 			case 0:
	                 				SendDlgItemMessage (hWndDlg,IDC_TYPE_DIR,BM_SETCHECK,TRUE,0);
	                 				break;
	                 			case 1:
	                 				SendDlgItemMessage (hWndDlg,IDC_TYPE_TYPE,BM_SETCHECK,TRUE,0);
	                 				break;
	                 			case 2:
	                 				SendDlgItemMessage (hWndDlg,IDC_TYPE_NAME,BM_SETCHECK,TRUE,0);
	                 				break;
	                 		}
                 			SetDlgItemText (hWndDlg,IDC_FULLWORD,word);
	                 		goto EndFind;
	                 	} 
	             	}
	             }
	   EndFind:
            	 break;
            }
            
            case IDC_DELETE:
             	 GetDlgItemText (hWndDlg,IDC_FULLWORD,word,12);
             	 word[12]=0;
             	 _fstrncpy (Key.FullName,word,14);
             	 BT_DELETE (hBT,(LPSTR)&Key,str,FALSE); 
             	 SetDlgItemText (hWndDlg,IDC_FULLWORD,word);
            	 break;
            
			case IDC_STANDARD_ABV:
			case IDC_ALT_ABV1:
			case IDC_ALT_ABV2:
			case IDC_ALT_ABV3:
			case IDC_ALT_ABV4:
			case IDC_ALT_ABV5:
			case IDC_ALT_ABV6:
			case IDC_ALT_ABV7:
			case IDC_ALT_ABV8:
			case IDC_ALT_ABV9:
			case IDC_ALT_ABV10:
                 switch (HIWORD(wParam))
                 {
                 	case EN_KILLFOCUS:
                 		Update=TRUE;
                 		break;
                 }
                 break;	
	 	    
	 	    case IDC_UPDATEENTRY:
	 	    	 Update = TRUE;
            	 if (!GetDlgItemText (hWndDlg,IDC_STANDARD_ABV,str,16))
            		break;  
            	 if (!GetDlgItemText (hWndDlg,IDC_FULLWORD,word,16))
            	 	break;
             	 if (SendDlgItemMessage (hWndDlg,IDC_TYPE_TYPE,BM_GETCHECK,0,0))
             		Key.Type = 1;
             	 else if (SendDlgItemMessage (hWndDlg,IDC_TYPE_DIR,BM_GETCHECK,0,0))
             		Key.Type = 0; 
             	 else
					Key.Type = 2;
				 word[12]=0;
				 Truncate (word); 
				 _fstrncpy (Key.FullName,word,12);
    		 	 pstr = str; 
    		 	 *OutString = 0;
                 for (i=0;i<11;i++,pstr+=12)
                 {
                 	GetDlgItemText (hWndDlg,AbvFields[i],word,12);
                 	word[12]=0;
                 	PadString (word,' ',12);
                 	_fstrcat (OutString,word);
	             }
				 BT_PUT (hBT,(LPSTR)&Key,OutString);
	 	    	 break;
	 	    	   	
         	case IDC_FULLWORD:
                 switch (HIWORD(wParam))
                 {	
                 	case EN_CHANGE:  
	                 	if (SendDlgItemMessage (hWndDlg,IDC_TYPE_TYPE,BM_GETCHECK,0,0))
	                 		wanttype = 1;
	                 	else if (SendDlgItemMessage (hWndDlg,IDC_TYPE_DIR,BM_GETCHECK,0,0))
	                 		wanttype = 0;
	                 	else
	                 		wanttype = 2; 
	                 	Key.Type = wanttype;
	                 	l=GetDlgItemText (hWndDlg,IDC_FULLWORD,word,12);
	                 	word[12]=0;
	                 	_fstrncpy (Key.FullName,word,14);
	                 	
	                 	st = BT_FIND (hBT,(LPSTR)&Key,BT_FIRST,BT_GE,str);
		                if (!l) 
		                	EnableWindow (GetDlgItem(hWndDlg,IDC_PRIOR),FALSE); 
		                else
		                	EnableWindow (GetDlgItem(hWndDlg,IDC_PRIOR),TRUE);
	                 	if (!l || st || _fstrncmp (word,Key.FullName,l))
	                 	{
	                 		_fmemset (str,0,144);
		                	EnableWindow (GetDlgItem(hWndDlg,IDC_DELETE),FALSE); 
		                }
	                 	else 
		                	EnableWindow (GetDlgItem(hWndDlg,IDC_DELETE),TRUE);   
		                if (st || Key.Type != wanttype)
		                	EnableWindow (GetDlgItem(hWndDlg,IDC_NEXT),FALSE);
		                else 
		                	EnableWindow (GetDlgItem(hWndDlg,IDC_NEXT),TRUE); 
	                 	pstr = str;
	                 	for (i=0;i<11;i++,pstr+=12)
	                 	{
		                 	_fstrncpy (word,pstr,12);
		                 	word[12]=0;
		                 	Truncate (word);
		                 	SetDlgItemText (hWndDlg,AbvFields[i],word); 
		                } 
		                break;
		                
		                case EN_KILLFOCUS:
		                	if (!GetDlgItemText (hWndDlg,IDC_STANDARD_ABV,str,16))
		                		break;  
		                 	_fstrncpy (word,Key.FullName,12); 
		                 	word[12]=0;
		                 	Truncate (word);
		                 	SetDlgItemText (hWndDlg,IDC_FULLWORD,word); 
		                break;
		                
                 }
                 break; 
                 
                 
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */     
                 GSSiGlobFree (&hName);
                 BT_CLOSEANDDELETE (&hBT);
                 EndDialog(hWndDlg, FALSE); 
                 break;
                 
            case IDC_LOAD:
            case IDC_EXIT: 
            {    
            	 short	pos=BT_FIRST, lasttype=0; 
                 
                 if (hName)
                 {
	            	 pName = GlobalLock (hName);
	                 if (Update)
	                 {
		        		 Fid = GSSiOpenFile (pName,&OFStruct,OF_CREATE);  
		        		 fputstring ("*** Street Directions ***",Fid);
			             while (!BT_FIND (hBT,(LPSTR)&Key,pos,BT_ANY,str))
			             { 
			             	pos = BT_NEXT; 
			             	if (Key.Type != lasttype)
			             	{       
			             		if (Key.Type == 1)
		        		 			fputstring ("*** Street Types ***",Fid);
		        		 		else
		        		 			fputstring ("*** Portions of Street Names ***",Fid); 
		        		 	}
		        		 	lasttype = Key.Type;
		        		 	_fstrncpy (word,Key.FullName,12);
		        		 	word[12]=0;
		        		 	PadString (word,' ',12);
		        		 	_fstrcpy (OutString,word); 
		        		 	pstr = str;
			                for (i=0;i<11;i++,pstr+=12)
			                {
			                 	_fstrncpy (word,pstr,12);
			                 	word[12]=0;
			                 	PadString (word,' ',12);
			                 	_fstrcat (OutString,word);
				            }
				            fputstring (OutString,Fid); 
			             } 
		        		 GSSiClose (Fid);             
		        	 }
	            	 GSSiGlobUlFree (&hName);      
	                 BT_CLOSEANDDELETE (&hBT); 
	                 Update = FALSE;
	             }
                 if (LOWORD(wParam) == IDC_EXIT)
                 	EndDialog(hWndDlg, TRUE); 
                 else
            		ReloadSTNDTables ();
                 break;
                 
            }
          }
          break;

    default:
        return FALSE;
   }
 return TRUE;    
}

BOOL FAR PASCAL ADDLOC_FROMINTMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
    int       TabStops[2]={100,1300}, i;
    LPSTR   pPrefix, lpDot, lpMIDstr;  
    HCURSOR OldCursor=0;    
    static   HANDLE hSQL=0;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hBT;    
    float       size=(float)12.0,rot=(float)0.0;
    long        Offset; 
    double      rtn;
    LPVOID      lpVal; 
    char        str1[16], str2[64];
    char        str[256]; 
    short         st, len,ifield;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    HANDLE      SaveHandle;
    BOOL        More;
    short         rc; 
    UINT        FieldLists[3]={IDC_KEY_FIELD,IDC_XFIELD,IDC_YFIELD},
                FieldListTypes[3]={TRUE,TRUE,TRUE}; 
    HFILE		FidSave;
    OFSTRUCT	OFStruct;
    char	Ext[6]=".NL2";
    char	Street1[256], Street2[256], BadNames[MAX_PATH], City[34];
	char	Name[128]; 
	static	BOOL	RecalledName;
    static		BOOL		FileIsOpen=FALSE;
    

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam, 0,
                     SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, FieldLists,0,
                     IMDataFile, &IMDataFileType, &hSQL,FieldListTypes,TRUE))  return TRUE;
 switch(Message)
   {
    case WM_INITDIALOG:
    	 RecalledName=FALSE; 
    	 hWndHidden=hWndDlg;
    	 TotAddLen = 0;
    	 NumMatched = 0;
         SendDlgItemMessage (hWndDlg,IDC_MOPT,CB_ADDSTRING,0,(LPARAM)"No Changes");
         SendDlgItemMessage (hWndDlg,IDC_MOPT,CB_ADDSTRING,0,(LPARAM)"Minor Changes");
         SendDlgItemMessage (hWndDlg,IDC_MOPT,CB_ADDSTRING,0,(LPARAM)"Major Changes");
    	 SetDlgItemText (hWndDlg,IDC_SQL,"ALL ROWS");
         SendDlgItemMessage (hWndDlg,IDC_DIST_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_DIST_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         if (*AutoExportName)
		 	PostMessage(hWndDlg, WM_COMMAND, IDC_RECALL, 0L);
    case GSSI_REINITDIALOG:
 		 SendDlgItemMessage (hWndDlg,IDC_MOPT,CB_SETCURSEL,(WPARAM)(MOPT-1),(LPARAM)NULL); 
    	 if (TotAddLen)
             PctBox (GetDlgItem(hWndDlg,IDC_STATUS1), TotAddLen, NumMatched,0);
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
			case IDC_SHOW:
		      	 PctBox (GetDlgItem(hWndDlg,IDC_STATUS1), TotAddLen, NumMatched,0);
			     break;

            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)  
                 {
                    ContinueProcessing=FALSE;
	                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Canceled");
				 }                    
                 else
        			 PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
                 break;
            
            case IDC_EXIT: 
                 if (!RecalledName)
                 	goto NoUpdate;   
                 GetCurVal (Name,sizeof(Name),IDS_FILENL2);
               	 goto Update;
            case IDC_SAVE:
            {    
            	 short	Version=1;
            	 
                 if (!GetSaveName2 (hWndDlg,Name,0,Ext,IDS_FILENL2)) break; 
        Update:
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
                 BigWrite (FidSave,Ext,6,-1);
                 BigWrite (FidSave,(HPSTR)&Version,2,-1);   
                 BigWrite (FidSave,(HPSTR)IMDataFile,lnIMDataFile,-1);
                 GetDlgItemText (hWndDlg,IDC_DEST_FILE,DestName,lnDestName);
                 BigWrite (FidSave,(HPSTR)DestName,lnDestName,-1);
                 GetDlgItemText (hWndDlg,IDC_STREET1,Street1,sizeof(Street1));
                 BigWrite (FidSave,(HPSTR)Street1,sizeof(Street1),-1);
                 GetDlgItemText (hWndDlg,IDC_STREET2,Street2,sizeof(Street2)); 
                 BigWrite (FidSave,(HPSTR)Street2,sizeof(Street2),-1);
                 GetDlgItemText (hWndDlg,IDC_SQL,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_KEY_FIELD,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_DISTANCE,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_DIRECTION,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_DIST_UNITS,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,32,-1);
                 GetDlgItemText (hWndDlg,IDC_DIR_UNITS,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,32,-1);  
                 GetDlgItemText (hWndDlg,IDC_UPDATE_STRING,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);  
                 GetDlgItemText (hWndDlg,IDC_SYMBOL,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_FROMDATE,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_TODATE,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 BigWrite (FidSave,(HPSTR)&TotAddLen,4,-1);
                 BigWrite (FidSave,(HPSTR)&NumMatched,4,-1);
			     MOPT=SendDlgItemMessage(hWndDlg,IDC_MOPT,CB_GETCURSEL,0,0)+1; 
                 BigWrite (FidSave,(HPSTR)&MOPT,2,-1);
                 GSSiClose (FidSave);
            } 
        NoUpdate:   
            	 if (LOWORD(wParam) == IDC_EXIT)
            	 {
                     CloseDataFile (TRUE, &hSQL);  
					 DestroyFieldList ();
	                 DestroyWindow (hWndDlg);
                 }
                    
            	 break;
            	 
           	case IDC_RECALL: 
           	{
           		 short Version;
           		 
             	 if (!*AutoExportName)
             	 { 
					 if (!GetFileName2 (hWndDlg,Name,Ext,IDS_FILENL2))
					 	break;
	             }
	             else
	             	_fstrcpy (Name,AutoExportName);  
				 RecalledName=TRUE;
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_READ);
                 BigRead (FidSave,Ext,6);
                 BigRead (FidSave,(HPSTR)&Version,2);   
                 BigRead (FidSave,(HPSTR)IMDataFile,lnIMDataFile);
                 BigRead (FidSave,(HPSTR)DestName,lnDestName);
                 SetDlgItemText (hWndDlg,IDC_DEST_FILE,DestName);
                 BigRead (FidSave,(HPSTR)Street1,sizeof(Street1));
                 SetDlgItemText (hWndDlg,IDC_STREET1,Street1);
                 BigRead (FidSave,(HPSTR)Street2,sizeof(Street2));
                 SetDlgItemText (hWndDlg,IDC_STREET2,Street2); 
                 BigRead (FidSave,(HPSTR)str,256);
                 SetDlgItemText (hWndDlg,IDC_SQL,(LPSTR)str);                
                 BigRead (FidSave,(HPSTR)str,256);
                 SetDlgItemText (hWndDlg,IDC_KEY_FIELD,(LPSTR)str);                
                 BigRead (FidSave,(HPSTR)str,256);
                 SetDlgItemText (hWndDlg,IDC_DISTANCE,(LPSTR)str);                
                 BigRead (FidSave,(HPSTR)str,256);
                 SetDlgItemText (hWndDlg,IDC_DIRECTION,(LPSTR)str);                
                 BigRead (FidSave,(HPSTR)str,32);
                 SetDlgItemText (hWndDlg,IDC_DIST_UNITS,(LPSTR)str);                
                 BigRead (FidSave,(HPSTR)str,32);
                 SetDlgItemText (hWndDlg,IDC_DIR_UNITS,(LPSTR)str);                
                 BigRead (FidSave,(HPSTR)str,256);  
                 SetDlgItemText (hWndDlg,IDC_UPDATE_STRING,(LPSTR)str);                
                 BigRead (FidSave,(HPSTR)str,256);  
                 SetDlgItemText (hWndDlg,IDC_SYMBOL,(LPSTR)str);                
                 BigRead (FidSave,(HPSTR)str,256);  
                 SetDlgItemText (hWndDlg,IDC_FROMDATE,(LPSTR)str);                
                 BigRead (FidSave,(HPSTR)str,256);  
                 SetDlgItemText (hWndDlg,IDC_TODATE,(LPSTR)str);                
                 BigRead (FidSave,(HPSTR)&TotAddLen,4);
                 BigRead (FidSave,(HPSTR)&NumMatched,4);
                 BigRead (FidSave,(HPSTR)&MOPT,2);
                 GSSiClose (FidSave);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EDIT_DEST),TRUE); 
               //  EnableWindow (GetDlgItem(hWndDlg,IDC_EDIT_HELPER),TRUE); 
                 FileIsOpen = TRUE;
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
            }
           		 break;
           		 
            case IDC_LOCATE_DEST: 
				{
					char	ext[6]=".GMD";
                 *str=0;
                 if (!GetSaveName2 (hWndDlg,str,IDS_FILTERGWD,ext,IDS_FILEGMD)) break;   
                 SetDlgItemText (hWndDlg,IDC_DEST_FILE,str);
                 break;
				}
                  
            case IDC_EDIT_DEST:
            {
                  FARPROC	lpfnINTERSECT_MATCH_EDITMsgProc; 

                  lpfnINTERSECT_MATCH_EDITMsgProc = MakeProcInstance((FARPROC)INTERSECT_MATCH_EDITMsgProc, hInst);
                  CreateDialog(hInst, (LPSTR)"INTERSECT_MATCH_EDIT", hWndDlg, lpfnINTERSECT_MATCH_EDITMsgProc);
//                  nRc = DialogBox(hInst, (LPSTR)"INTERSECT_MATCH_EDIT", hWndDlg, lpfnINTERSECT_MATCH_EDITMsgProc);
//                  FreeProcInstance(lpfnINTERSECT_MATCH_EDITMsgProc);
            }
            	 break;
            	 
            case IDC_OPEN_DB:
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),TRUE);
                 break;  
            
            case IDC_SHOW_FIELDS: 
            	 
            	 DisplayFieldList (hWndDlg,hSQL,NULL,0,0);
                 break;
                      
            case IDC_SETSQL:      
            {    
                 HANDLE hMem;
                 LPSTR  lpStr, lpWhere;
                 
                 hMem = GSSiGlobAlloc ( 119,GHND,4096);
                 lpStr = GlobalLock (hMem); 
                 GetDlgItemText (hWndDlg,IDC_SQL,lpStr,1024);
                 if (GetSQLWhereClause (hWndDlg, hSQL, lpStr))
                 	SetDlgItemText (hWndDlg,IDC_SQL,lpStr);    
                 GSSiGlobUlFree (&hMem);
                 break;
            }
            
            case IDOK: 
            {
                 long   lineno=0, CurLoc, MidLine, ii;  
                 long   NewRefno, StreetNum1, StreetNum2, MunicNum; 
                 short    st, SymNum, iUDI=0, AreaSym, LineSym, LocOpt, Pass=1;
                 BOOL   Done, First=TRUE; 
                 HANDLE hMIDstr;
                 char   SymName[10], project[32];
                 double X,Y;
                 LPSTR  lpTAB, pSQL; 
                 DPOINT Point;
                 BOOL   Store;
                 short    Symbol=1; 
                 COLORREF   Color;
                 LPSTR  lpDot; 
                 MNMXCORD MinMaxCoord;  
                 double coordcvt=1;
                 short    NumSyms=0;  
                 HANDLE hSymDesc=0;
                 short	match, sn, OrigKeyLen;  
                 BOOL	Opened;
				 short	IndexArray[2]; 
                 HANDLE	hMatch=0, hDBDest, hBTDest, hBTBadNames;
                 UINT	len; 
                 LPADDMATCH	pMatch;
				 LPINTMATCHEDITREC pIMER;
			     LPGWDHEADER	lpGWDHead;
			     BADNAMEKEY		BadNameKey;
                 
				 ContinueProcessing=TRUE;
                 if (!GetDlgItemText (hWndDlg,IDC_DEST_FILE,DestName,lnDestName))
                 {
                    GSSiMsgBox(GetFocus(),"No destination file", 0,MB_ICONQUESTION|MB_OK,0);
                    break;
                 }
                 GetDlgItemText (hWndDlg,IDC_STREET1,Street1,sizeof(Street2));
                 GetDlgItemText (hWndDlg,IDC_STREET2,Street2,sizeof(Street1)); 
                if (!*Street1 ||!*Street2)
                {
                    GSSiMsgBox(GetFocus(),"Street1 and/or Street2 field missing", 0,MB_ICONQUESTION|MB_OK,0);
                    break;
                }
                 
			    MOPT=SendDlgItemMessage(hWndDlg,IDC_MOPT,CB_GETCURSEL,0,0)+1; 
                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading Data");
                CloseDataFile (TRUE, &hSQL);  
                GetDlgItemText (hWndDlg,IDC_SQL,(LPSTR)str,sizeof(str));                
                pSQL = str;
                if (!_fstrcmp (pSQL,"ALL ROWS"))
                    *pSQL = 0;
                hSQL = 0;
                if (!OpenDataFile (IMDataFile,pSQL,BT_READ,&hSQL))
                {  
                    GSSiMsgBox(GetFocus(),"Cannot open data file", 0,MB_ICONQUESTION|MB_OK,0);
                    break;
                }
                
                if (!OpenNetIntersect (NetworkID,FALSE,&Opened))
                	break; 
                GetDlgItemText (hWndDlg,IDC_KEY_FIELD,str,sizeof(str));
                if (!*str)
                	OrigKeyLen = 2;
                else if (_fstrchr (str,'@'))
                	OrigKeyLen = 100;
                else
                	OrigKeyLen = 64;
			    if (!CreateINT_MATCHTable (DestName,BadNames,OrigKeyLen))
				    break;
			    hDBDest = OpenGWDatabase (DestName,BT_WRITE);
			   // hBTBadNames = BT_OPEN (BadNames,0,BT_WRITE,0);
			    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest); 
                  
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE); 

                if (First)
                {   
                	HCURSOR	OldCursor;
                	
                    OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT));
                    TotAddLen = NumSQLRows (hSQL);  
		            GSSiSetCursor (OldCursor);
                }
                First = FALSE;
                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading Data");
                
                pIMER = (LPINTMATCHEDITREC)lpGWDHead->GWDData;
                    
				ContinueProcessing=TRUE;
                Processing = TRUE;
                NewRefno=0;
                Done = FALSE; 
                NumMatched = NumNoMatch = NumInvalid = NumMultMatch = 0;
        NextLine: 
        		 GSSiGlobFree (&hMatch);
                 if (!FetchDBRec (hSQL) || !ContinueProcessing)
                    goto EndFile; 
                    
                 GetDlgItemText (hWndDlg,IDC_STREET1,Street1,sizeof(Street1));
                 GetDlgItemText (hWndDlg,IDC_STREET2,Street2,sizeof(Street2)); 
                 GetDlgItemText (hWndDlg,IDC_CITY,City,sizeof(City)); 
                 ExpandText(Street1);  
                 ExpandText(Street2); 
                 OneSpace (Street1);
                 OneSpace (Street2); 
				 match = INT_MATCH (Street1,Street2,City,0,MOPT,&hMatch,&StreetNum1,&StreetNum2,&MunicNum); 
				 _fmemset (pIMER,0,(size_t)lpGWDHead->Reclen);
				 pIMER->RecordNum = lineno;
                 GetDlgItemText (hWndDlg,IDC_KEY_FIELD,str,sizeof(str));
                 ExpandText (str);
                 _fstrncpy (pIMER->OrigKey,str,OrigKeyLen);
				 switch (match)
				 {  
				 	case 1:
				 		NumMatched++;
				 		pMatch = (LPADDMATCH)GlobalLock (hMatch); 
				 		pIMER->IM = *pMatch;
					   	GlobalUnlock (hMatch);
				 		IndexArray[1]=FALSE;
				 		break;
				 	case 0:  
				 		NumNoMatch++;
				 		BadNameKey.RecNum = lineno;
				 		if (!StreetNum1)
				 		{
				 			_fstrncpy (BadNameKey.Name,Street1,40);
				 			sn = 1;
				 			//BT_PUT (hBTBadNames,(LPSTR)&BadNameKey,(LPSTR)&sn);
				 		}
				 		if (!StreetNum2)
				 		{  
				 			sn = 2;
				 			_fstrncpy (BadNameKey.Name,Street2,40);
				 			//BT_PUT (hBTBadNames,(LPSTR)&BadNameKey,(LPSTR)&sn);
				 		}
				 		IndexArray[1] = TRUE;
				 		goto NextStep;
				 	default:  
				 		IndexArray[1] = TRUE;
				 		NumMultMatch++; 
		NextStep:
				 		pIMER->IM.StreetNum1 = StreetNum1;
				 		pIMER->IM.StreetNum2 = StreetNum2; 
				 		break; 
				 }
	 		 	 _fstrncpy (pIMER->StreetA,Street1,MAXSTREETNAMELEN);
	 			 _fstrncpy (pIMER->StreetB,Street2,MAXSTREETNAMELEN);
		 		 pIMER->IM.MatchCode = match; 
		 		 GetDlgItemText (hWndDlg,IDC_SYMBOL,str,250);
		 		 SetFieldValFromCharAndName(lpGWDHead,"Symbol",str,FALSE);
		 		 GetDlgItemText (hWndDlg,IDC_FROMDATE,str,250);
		 		 SetFieldValFromCharAndName(lpGWDHead,"FromDate",str,FALSE);
		 		 GetDlgItemText (hWndDlg,IDC_TODATE,str,250);
		 		 SetFieldValFromCharAndName(lpGWDHead,"ToDate",str,FALSE);
				 GWDAddRecord (lpGWDHead,0,IndexArray);
                 lineno++; 
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS1), TotAddLen, NumMatched,0);
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS2), TotAddLen, lineno,0);
                 goto NextLine;
                 
        ErrorEnd: 
                 {  
                    char    mess[256];
                    
                    sprintf (mess,"Error in file at line %ld\r\n%s",lineno,str);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK,0);
                 }
                    
        EndFile: 
                 sprintf (str,"Finished - %ld matched, %ld no hits, %ld multiple hits",NumMatched,NumNoMatch,NumMultMatch);
	             SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,str);
                 if (hSQL)
                 {
                    CloseDataFile (TRUE, &hSQL);  
                 } 
    			 GlobalUnlock (hDBDest);
				 CloseGWDatabase (hDBDest);
				 hDBDest = 0; 
				 //BT_CLOSE (hBTBadNames);
				 CloseStreetNameTable();
				 CloseNetIntersect (Opened);
                 DisableHalt = FALSE; 
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EDIT_DEST),TRUE); 
        Reset:
                 Processing = FALSE;
				 ContinueProcessing=TRUE;
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE); 
				 FileIsOpen=FALSE;
				 if (*AutoExportName)
		         	PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
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

BOOL FAR PASCAL STREET_SEGS_BETWEEN_INTSMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
    int		TabStops[2]={100,1300}, i;
    LPSTR   pPrefix, lpDot, lpMIDstr;  
    HCURSOR OldCursor=0;    
    static   HANDLE hSQL=0;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hBT;    
    float       size=(float)12.0,rot=(float)0.0;
    long        Offset; 
    double      rtn;
    LPVOID      lpVal; 
    char        str1[16], str2[64];
    char        str[256]; 
    short         st, len,ifield;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    HANDLE      SaveHandle;
    BOOL        More;
    short         rc; 
    UINT	    FieldLists[3]={IDC_KEY_FIELD,IDC_XFIELD,IDC_YFIELD},
                FieldListTypes[3]={TRUE,TRUE,TRUE}; 
    HFILE		FidSave;
    OFSTRUCT	OFStruct;
    char	Ext[6]=".NL4";
    char	OnStreet[256], FromStreet[256],ToStreet[256], BadNames[MAX_PATH], City[34]; 
    char	OnStreetOrig[256], FromStreetOrig[256],ToStreetOrig[256];//temp debug
	char	Name[128]; 
	static	BOOL	RecalledName;
    static		BOOL		FileIsOpen=FALSE;
    

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam, 0,
                     SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, FieldLists,0,
                     IMDataFile, &IMDataFileType, &hSQL,FieldListTypes,TRUE))  return TRUE;
 switch(Message)
   {
    case WM_INITDIALOG:
    	 RecalledName=FALSE; 
    	 hWndHidden=hWndDlg;
    	 TotAddLen = 0;
    	 NumMatched = 0;
         SendDlgItemMessage (hWndDlg,IDC_MOPT,CB_ADDSTRING,0,(LPARAM)"No Changes");
         SendDlgItemMessage (hWndDlg,IDC_MOPT,CB_ADDSTRING,0,(LPARAM)"Minor Changes");
         SendDlgItemMessage (hWndDlg,IDC_MOPT,CB_ADDSTRING,0,(LPARAM)"Major Changes");
    	 SetDlgItemText (hWndDlg,IDC_SQL,"ALL ROWS");
         SendDlgItemMessage (hWndDlg,IDC_DIST_UNITS,CB_ADDSTRING,0,(LPARAM)"Feet");
         SendDlgItemMessage (hWndDlg,IDC_DIST_UNITS,CB_ADDSTRING,0,(LPARAM)"Meters");
         if (*AutoExportName)
		 	PostMessage(hWndDlg, WM_COMMAND, IDC_RECALL, 0L);
    case GSSI_REINITDIALOG:
 		 SendDlgItemMessage (hWndDlg,IDC_MOPT,CB_SETCURSEL,(WPARAM)(MOPT-1),(LPARAM)NULL); 
    	 if (TotAddLen)
             PctBox (GetDlgItem(hWndDlg,IDC_STATUS1), TotAddLen, NumMatched,0);
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
			case IDC_SHOW:
		      	 PctBox (GetDlgItem(hWndDlg,IDC_STATUS1), TotAddLen, NumMatched,0);
			     break;

            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)  
                 {
                    ContinueProcessing=FALSE;
	                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Canceled");
				 }                    
                 else
        			 PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
                 break;
            
            case IDC_EXIT: 
                 if (!RecalledName)
                 	goto NoUpdate;   
                 GetCurVal (Name,sizeof(Name),IDS_FILENL4);
               	 goto Update;
            case IDC_SAVE:
            {    
            	 short	Version=1;
            	 
                 if (!GetSaveName2 (hWndDlg,Name,0,Ext,IDS_FILENL4)) break; 
        Update:
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_CREATE);
                 BigWrite (FidSave,Ext,6,-1);
                 BigWrite (FidSave,(HPSTR)&Version,2,-1);   
                 BigWrite (FidSave,(HPSTR)IMDataFile,lnIMDataFile,-1);
                 GetDlgItemText (hWndDlg,IDC_DEST_FILE,DestName,lnDestName);
                 BigWrite (FidSave,(HPSTR)DestName,lnDestName,-1);
                 GetDlgItemText (hWndDlg,IDC_ONSTREET,OnStreet,sizeof(OnStreet));
                 BigWrite (FidSave,(HPSTR)OnStreet,sizeof(OnStreet),-1);
                 GetDlgItemText (hWndDlg,IDC_FROMSTREET,FromStreet,sizeof(FromStreet)); 
                 BigWrite (FidSave,(HPSTR)FromStreet,sizeof(FromStreet),-1);
                 GetDlgItemText (hWndDlg,IDC_TOSTREET,ToStreet,sizeof(ToStreet)); 
                 BigWrite (FidSave,(HPSTR)ToStreet,sizeof(ToStreet),-1);
                 GetDlgItemText (hWndDlg,IDC_SQL,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_KEY_FIELD,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_CITY,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_DIRECTION,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_DIST_UNITS,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,32,-1);
                 GetDlgItemText (hWndDlg,IDC_DIR_UNITS,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,32,-1);  
                 GetDlgItemText (hWndDlg,IDC_UPDATE_STRING,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);  
                 GetDlgItemText (hWndDlg,IDC_SYMBOL,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_FROMDATE,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_TODATE,(LPSTR)str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 BigWrite (FidSave,(HPSTR)&TotAddLen,4,-1);
                 BigWrite (FidSave,(HPSTR)&NumMatched,4,-1);
			     MOPT=SendDlgItemMessage(hWndDlg,IDC_MOPT,CB_GETCURSEL,0,0)+1; 
                 BigWrite (FidSave,(HPSTR)&MOPT,2,-1);
                 GSSiClose (FidSave);
            } 
        NoUpdate:   
            	 if (LOWORD(wParam) == IDC_EXIT)
            	 {
                     CloseDataFile (TRUE, &hSQL);  
					 DestroyFieldList ();
	                 DestroyWindow (hWndDlg);
                 }
                    
            	 break;
            	 
           	case IDC_RECALL: 
           	{
           		 short Version;
           		 
             	 if (!*AutoExportName)
             	 { 
					 if (!GetFileName2 (hWndDlg,Name,Ext,IDS_FILENL4))
					 	break;
	             }
	             else
	             	_fstrcpy (Name,AutoExportName);  
				 RecalledName=TRUE;
                 FidSave = GSSiOpenFile (Name,&OFStruct,OF_READ);
                 BigRead (FidSave,Ext,6);
                 BigRead (FidSave,(HPSTR)&Version,2);   
                 BigRead (FidSave,IMDataFile,lnIMDataFile);
                 BigRead (FidSave,DestName,lnDestName);
                 SetDlgItemText (hWndDlg,IDC_DEST_FILE,DestName);
                 BigRead (FidSave,OnStreet,sizeof(OnStreet));
                 SetDlgItemText (hWndDlg,IDC_ONSTREET,OnStreet);
                 BigRead (FidSave,FromStreet,sizeof(FromStreet));
                 SetDlgItemText (hWndDlg,IDC_FROMSTREET,FromStreet); 
                 BigRead (FidSave,ToStreet,sizeof(ToStreet));
                 SetDlgItemText (hWndDlg,IDC_TOSTREET,ToStreet); 
                 BigRead (FidSave,str,256);
                 SetDlgItemText (hWndDlg,IDC_SQL,(LPSTR)str);                
                 BigRead (FidSave,str,256);
                 SetDlgItemText (hWndDlg,IDC_KEY_FIELD,(LPSTR)str);                
                 BigRead (FidSave,str,256);
                 SetDlgItemText (hWndDlg,IDC_CITY,(LPSTR)str);                
                 BigRead (FidSave,str,256);
                 SetDlgItemText (hWndDlg,IDC_DIRECTION,(LPSTR)str);                
                 BigRead (FidSave,str,32);
                 SetDlgItemText (hWndDlg,IDC_DIST_UNITS,(LPSTR)str);                
                 BigRead (FidSave,str,32);
                 SetDlgItemText (hWndDlg,IDC_DIR_UNITS,(LPSTR)str);                
                 BigRead (FidSave,str,256);  
                 SetDlgItemText (hWndDlg,IDC_UPDATE_STRING,(LPSTR)str);                
                 BigRead (FidSave,str,256);  
                 SetDlgItemText (hWndDlg,IDC_SYMBOL,(LPSTR)str);                
                 BigRead (FidSave,str,256);  
                 SetDlgItemText (hWndDlg,IDC_FROMDATE,(LPSTR)str);                
                 BigRead (FidSave,str,256);  
                 SetDlgItemText (hWndDlg,IDC_TODATE,(LPSTR)str);                
                 BigRead (FidSave,(HPSTR)&TotAddLen,4);
                 BigRead (FidSave,(HPSTR)&NumMatched,4);
                 BigRead (FidSave,(HPSTR)&MOPT,2);
                 GSSiClose (FidSave);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EDIT_DEST),TRUE); 
                 FileIsOpen = TRUE;
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
            }
           		 break;
           		 
            case IDC_LOCATE_DEST: 
				{
					char	ext[6]=".GMD";
					*str=0;
					if (!GetSaveName2 (hWndDlg,str,IDS_FILTERGWD,ext,IDS_FILEGMD)) break;   
					SetDlgItemText (hWndDlg,IDC_DEST_FILE,str);
				}
                 break;
                  
            case IDC_EDIT_DEST:
            {
                  FARPROC	lpfnINTERSECT_MATCH_EDITMsgProc; 

                  lpfnINTERSECT_MATCH_EDITMsgProc = MakeProcInstance((FARPROC)INTERSECT_MATCH_EDITMsgProc, hInst);
                  CreateDialog(hInst, (LPSTR)"INTERSECT_MATCH_EDIT", hWndDlg, lpfnINTERSECT_MATCH_EDITMsgProc);
//                  nRc = DialogBox(hInst, (LPSTR)"INTERSECT_MATCH_EDIT", hWndDlg, lpfnINTERSECT_MATCH_EDITMsgProc);
//                  FreeProcInstance(lpfnINTERSECT_MATCH_EDITMsgProc);
            }
            	 break;
            	 
            case IDC_OPEN_DB:
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),TRUE);
                 break;  
            
            case IDC_SHOW_FIELDS: 
            	 
            	 DisplayFieldList (hWndDlg,hSQL,NULL,0,0);
                 break;
                      
            case IDC_SETSQL:      
            {    
                 HANDLE hMem;
                 LPSTR  lpStr, lpWhere;
                 
                 hMem = GSSiGlobAlloc ( 119,GHND,4096);
                 lpStr = GlobalLock (hMem); 
                 GetDlgItemText (hWndDlg,IDC_SQL,lpStr,1024);
                 if (GetSQLWhereClause (hWndDlg, hSQL, lpStr))
                 	SetDlgItemText (hWndDlg,IDC_SQL,lpStr);    
                 GSSiGlobUlFree (&hMem);
                 break;
            }
            
            case IDOK: 
            {
                 long   lineno=1, CurLoc, MidLine, ii;  
                 long   NewRefno, OnStreetNum1, OnStreetNum2, FromStreetNum, ToStreetNum, MunicNumFrom,MunicNumTo; 
                 short    st, SymNum, iUDI=0, AreaSym, LineSym, LocOpt, Pass=1;
                 BOOL   Done, First=TRUE; 
                 HANDLE hMIDstr;
                 char   SymName[10], project[32];
                 double X,Y;
                 LPSTR  lpTAB, pSQL; 
                 DPOINT Point;
                 BOOL   Store;
                 short    Symbol=1; 
                 COLORREF   Color;
                 LPSTR  lpDot; 
                 MNMXCORD MinMaxCoord;  
                 double coordcvt=1;
                 short    NumSyms=0;  
                 HANDLE hSymDesc=0;
                 short	match1,match2, sn, OrigKeyLen;
                 char	OrigKeyType;  
                 BOOL	Opened, OpenedNLR;
				 short	IndexArray[2]; 
                 HANDLE	hMatch1=0, hMatch2=0,hDBDest, hDBDestSegs, hBTDest, hBTBadNames;
                 UINT	len; 
                 LPADDMATCH	pMatchFrom,pMatchTo;
				 LPSTREET_SEGS_BETWEEN_INTSMATCHEDITREC pIMER;
			     LPGWDHEADER	lpGWDHead;
			     BADNAMEKEY		BadNameKey;
			     LPSTR	pPar, pDot;
			     char	DestNameSegs[128],COrigKey[128];
                 
				 ContinueProcessing=TRUE;
                 if (!GetDlgItemText (hWndDlg,IDC_DEST_FILE,DestName,lnDestName))
                 {
                    GSSiMsgBox(GetFocus(),"No destination file", 0,MB_ICONQUESTION|MB_OK,0);
                    break;
                 }
                 GetDlgItemText (hWndDlg,IDC_ONSTREET,OnStreet,sizeof(OnStreet));
                 GetDlgItemText (hWndDlg,IDC_FROMSTREET,FromStreet,sizeof(FromStreet)); 
                 GetDlgItemText (hWndDlg,IDC_TOSTREET,ToStreet,sizeof(ToStreet)); 
                if (!*OnStreet ||!*FromStreet || !*ToStreet)
                {
                    GSSiMsgBox(GetFocus(),"On Street, From Street and/or To Street field missing", 0,MB_ICONQUESTION|MB_OK,0);
                    break;
                }
                 
			    MOPT=SendDlgItemMessage(hWndDlg,IDC_MOPT,CB_GETCURSEL,0,0)+1; 
                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading Data");
                CloseDataFile (TRUE, &hSQL);  
                GetDlgItemText (hWndDlg,IDC_SQL,(LPSTR)str,sizeof(str));                
                pSQL = str;
                if (!_fstrcmp (pSQL,"ALL ROWS"))
                    *pSQL = 0;
                hSQL = 0;
                if (!OpenDataFile (IMDataFile,pSQL,BT_READ,&hSQL))
                {  
                    GSSiMsgBox(GetFocus(),"Cannot open data file", 0,MB_ICONQUESTION|MB_OK,0);
                    break;
                }
                
                GetDlgItemText (hWndDlg,IDC_KEY_FIELD,str,sizeof(str));  
                OrigKeyType = 'C';
                if (!*str)
                	OrigKeyLen = 2;
                else if (_fstrchr (str,'@'))
                	OrigKeyLen = 100; 
                else if (GetFieldInfoFromName (hSQL,str,&OrigKeyType,&OrigKeyLen)) 
                { 
                	OrigKeyLen = min (OrigKeyLen,128);
                }
                else
                	OrigKeyLen = 64;
                if (!OpenNetIntersect (NetworkID,FALSE,&Opened))
                	break; 
				if (!OpenNetLinkAndRef  (NetworkID,FALSE,&OpenedNLR))
					break;
			    if (!CreateSEGBETWEEN_INT_MATCHTable (DestName,OrigKeyType,OrigKeyLen))
				    break;
				_fstrcpy (DestNameSegs,DestName);
				pDot = _fstrrchr (DestNameSegs,'.');
				_fstrcpy (pDot,"_segs.gmd");
			    if (!CreateSEGBETWEEN_INT_SEGS (DestNameSegs,OrigKeyType,OrigKeyLen))
				    break;
			    hDBDest = OpenGWDatabase (DestName,BT_WRITE);
			    hDBDestSegs = OpenGWDatabase (DestNameSegs,BT_WRITE);
			    //hBTBadNames = BT_OPEN (BadNames,0,BT_WRITE,0);
			    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest); 
                  
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE); 

                if (First)
                {   
                	HCURSOR	OldCursor;
                	
                    OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT));
                    TotAddLen = NumSQLRows (hSQL);  
		            GSSiSetCursor (OldCursor);
                }
                First = FALSE;
                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Loading Data");
                
                pIMER = (LPSTREET_SEGS_BETWEEN_INTSMATCHEDITREC)&lpGWDHead->GWDData;
                    
				ContinueProcessing=TRUE;
                Processing = TRUE;
                NewRefno=0;
                Done = FALSE; 
                NumMatched = NumNoMatch = NumInvalid = NumMultMatch = 0;
        NextLine: 
        		 GSSiGlobFree (&hMatch1); 
        		 GSSiGlobFree (&hMatch2);
                 if (!FetchDBRec (hSQL) || !ContinueProcessing)
                    goto EndFile; 
                    
                 GetDlgItemText (hWndDlg,IDC_ONSTREET,OnStreet,sizeof(OnStreet));
                 GetDlgItemText (hWndDlg,IDC_FROMSTREET,FromStreet,sizeof(FromStreet)); 
                 GetDlgItemText (hWndDlg,IDC_TOSTREET,ToStreet,sizeof(ToStreet)); 
                 GetDlgItemText (hWndDlg,IDC_CITY,City,sizeof(City));
                 ExpandText(OnStreet);  
                 ExpandText(FromStreet); 
                 ExpandText(ToStreet); 
                 ExpandText(City); 
                 _fstrcpy (OnStreetOrig,OnStreet);
                 _fstrcpy (FromStreetOrig,FromStreet);
                 _fstrcpy (ToStreetOrig,ToStreet);   
                 if ((pPar=_fstrchr (ToStreet,'(')))
                 	*pPar = 0;
                 OneSpace (OnStreet); 
                 OneSpace (FromStreet);
                 OneSpace (ToStreet); 
				 _fmemset (pIMER,0,(size_t)lpGWDHead->Reclen);
				 pIMER->RecordNum = lineno;
                 GetDlgItemText (hWndDlg,IDC_KEY_FIELD,COrigKey,sizeof(COrigKey));
                 ExpandText (COrigKey); 
                 if (!_fstricmp (COrigKey,"SD-A-5"))
                 	ii=1;
				 SetFieldValFromCharAndName(lpGWDHead ,"OriginalKey",COrigKey,FALSE);
                 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,COrigKey);
               	 MunicNumFrom = MunicNumTo = GetMunicFromName (City);    
				 match1 = INT_MATCH (OnStreet,FromStreet,City,0,MOPT,&hMatch1,&OnStreetNum1,&FromStreetNum,&MunicNumFrom); 
				 match2 = INT_MATCH (OnStreet,ToStreet,City,0,MOPT,&hMatch2,&OnStreetNum2,&ToStreetNum,&MunicNumTo); 
				 if (match1 == 1)
				 {  
				 		pMatchFrom = (LPADDMATCH)GlobalLock (hMatch1); 
				 		
						pIMER->IM.StreetNumFrom = FromStreetNum;
						pIMER->IM.StreetNumOn = OnStreetNum1;
						pIMER->IM.FromMP = pMatchFrom->Street1MP;
						pIMER->IM.PointFrom = pMatchFrom->Point;
						pIMER->IM.MunicFrom = MunicNumFrom;
						pIMER->IM.ZIPFrom = pMatchFrom->ZIP;
					   	GlobalUnlock (hMatch1);
				 }
				 if (match2 == 1)
				 {  
				 		pMatchTo = (LPADDMATCH)GlobalLock (hMatch2); 
				 		
						pIMER->IM.StreetNumTo = ToStreetNum;
						pIMER->IM.StreetNumOn = OnStreetNum2;
						pIMER->IM.ToMP = pMatchTo->Street1MP;
						pIMER->IM.PointTo = pMatchTo->Point;
						pIMER->IM.MunicTo = MunicNumTo;
						pIMER->IM.ZIPTo = pMatchTo->ZIP;
					   	GlobalUnlock (hMatch2);
				 }
				 if (match1 == 1 && match2 == 1)
				 {  
				 		pMatchFrom = (LPADDMATCH)GlobalLock (hMatch1); 
				 		pMatchTo = (LPADDMATCH)GlobalLock (hMatch2); 
						pIMER->IM.StreetNumFrom = FromStreetNum;
						pIMER->IM.StreetNumTo = ToStreetNum;
						pIMER->IM.StreetNumOn = OnStreetNum1;
						pIMER->IM.FromMP = pMatchFrom->Street1MP;
						pIMER->IM.ToMP = pMatchTo->Street1MP;
						pIMER->IM.MunicFrom = MunicNumFrom;
						pIMER->IM.MunicTo = MunicNumTo;
						pIMER->IM.ZIPFrom = pMatchFrom->ZIP;
						pIMER->IM.ZIPTo = pMatchTo->ZIP; 
						pIMER->NumSegsFound = GetStreetSegsBetweenMPs (hDBDestSegs,pIMER->IM.StreetNumOn,pIMER->IM.FromMP,pIMER->IM.ToMP,&pIMER->IM.PointFrom,&pIMER->IM.PointTo,COrigKey);
						if (pIMER->NumSegsFound)
					 		NumMatched++;
pIMER->NumSegsFound=0;
					   	GlobalUnlock (hMatch1);
					   	GlobalUnlock (hMatch2);
				 		IndexArray[1]=FALSE;
				 }
				 else if (match1 == 2 && match2 == 1)
				 {  
				 		for (i=0;i<2;i++)
				 		{
					 		pMatchFrom = (LPADDMATCH)GlobalLock (hMatch1);
					 		pMatchFrom += i; 
					 		pMatchTo = (LPADDMATCH)GlobalLock (hMatch2); 
							pIMER->IM.StreetNumFrom = FromStreetNum;
							pIMER->IM.StreetNumTo = ToStreetNum;
							pIMER->IM.StreetNumOn = OnStreetNum1;
							pIMER->IM.FromMP = pMatchFrom->Street1MP;
							pIMER->IM.ToMP = pMatchTo->Street1MP;
							pIMER->IM.MunicFrom = MunicNumFrom;
							pIMER->IM.MunicTo = MunicNumTo;
							pIMER->IM.ZIPFrom = pMatchFrom->ZIP;
							pIMER->IM.ZIPTo = pMatchTo->ZIP; 
							pIMER->NumSegsFound = GetStreetSegsBetweenMPs (hDBDestSegs,pIMER->IM.StreetNumOn,pIMER->IM.FromMP,pIMER->IM.ToMP,&pIMER->IM.PointFrom,&pIMER->IM.PointTo,COrigKey);
pIMER->NumSegsFound=0;
						   	GlobalUnlock (hMatch1);
						   	GlobalUnlock (hMatch2); 
						   	if (pIMER->NumSegsFound)   
						   	{   
						   		match1 = 1;
						 		NumMatched++;   
						 		break;
						 	}
						}
				 		IndexArray[1]=FALSE;
				 }
				 else if (match1 == 1 && match2 == 2)
				 {  
				 		for (i=0;i<2;i++)
				 		{
					 		pMatchFrom = (LPADDMATCH)GlobalLock (hMatch1);
					 		pMatchTo = (LPADDMATCH)GlobalLock (hMatch2); 
					 		pMatchTo += i;
							pIMER->IM.StreetNumFrom = FromStreetNum;
							pIMER->IM.StreetNumTo = ToStreetNum;
							pIMER->IM.StreetNumOn = OnStreetNum1;
							pIMER->IM.FromMP = pMatchFrom->Street1MP;
							pIMER->IM.ToMP = pMatchTo->Street1MP;
							pIMER->IM.MunicFrom = MunicNumFrom;
							pIMER->IM.MunicTo = MunicNumTo;
							pIMER->IM.ZIPFrom = pMatchFrom->ZIP;
							pIMER->IM.ZIPTo = pMatchTo->ZIP; 
							pIMER->NumSegsFound = GetStreetSegsBetweenMPs (hDBDestSegs,pIMER->IM.StreetNumOn,pIMER->IM.FromMP,pIMER->IM.ToMP,&pIMER->IM.PointFrom,&pIMER->IM.PointTo,COrigKey);
pIMER->NumSegsFound=0;
						   	GlobalUnlock (hMatch1);
						   	GlobalUnlock (hMatch2); 
						   	if (pIMER->NumSegsFound)   
						   	{   
						   		match2 = 1;
						 		NumMatched++;   
						 		break;
						 	}
						}
				 		IndexArray[1]=FALSE;
				 }
				 else if ((match1 + match2) < 0)
				 {
				 		NumNoMatch++;
				 		IndexArray[1] = TRUE;
				 		goto NextStep;
				 } 
				 else
				 {
				 		IndexArray[1] = TRUE;
				 		NumMultMatch++; 
//				 		sprintf (str,"(%s) %i-%i On:%s\tFrom:%s\tTo:%s",COrigKey,match1,match2,OnStreetOrig,FromStreetOrig,ToStreetOrig);
//				 		AppendFile ("c:\\oft.txt",str);
		NextStep:
				 		pIMER->IM.StreetNumOn = OnStreetNum1;
				 		pIMER->IM.StreetNumFrom = FromStreetNum; 
				 		pIMER->IM.StreetNumTo = ToStreetNum; 
				 		 
				 }
	 		 	 _fstrncpy (pIMER->OnStreet,OnStreet,MAXSTREETNAMELEN);
	 		 	 _fstrncpy (pIMER->StreetA,FromStreet,MAXSTREETNAMELEN);
	 			 _fstrncpy (pIMER->StreetB,ToStreet,MAXSTREETNAMELEN);
		 		 pIMER->IM.MatchCodeFrom = match1; 
		 		 pIMER->IM.MatchCodeTo = match2; 
		 		 GetDlgItemText (hWndDlg,IDC_SYMBOL,str,250);
		 		 SetFieldValFromCharAndName(lpGWDHead,"Symbol",str,FALSE);
		 		 GetDlgItemText (hWndDlg,IDC_FROMDATE,str,250);
		 		 SetFieldValFromCharAndName(lpGWDHead,"FromDate",str,FALSE);
		 		 GetDlgItemText (hWndDlg,IDC_TODATE,str,250);
		 		 SetFieldValFromCharAndName(lpGWDHead,"ToDate",str,FALSE);
				 GWDAddRecord (lpGWDHead,0,IndexArray);
                 lineno++; 
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS1), TotAddLen, NumMatched,0);
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS2), TotAddLen, lineno,0);
                 goto NextLine;
                 
        ErrorEnd: 
                 {  
                    char    mess[256];
                    
                    sprintf (mess,"Error in file at line %ld\r\n%s",lineno,str);
                    GSSiMsgBox(GetFocus(),mess, 0,MB_ICONEXCLAMATION|MB_OK,0);
                 }
                    
        EndFile: 
                 sprintf (str,"Finished - %ld matched, %ld no hits, %ld one end matched",NumMatched,NumNoMatch,NumMultMatch);
	             SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,str);
                 if (hSQL)
                 {
                    CloseDataFile (TRUE, &hSQL);  
                 } 
    			 GlobalUnlock (hDBDest);
				 CloseGWDatabase (hDBDest);
				 CloseGWDatabase (hDBDestSegs);
				 hDBDest = 0; 
				 //BT_CLOSE (hBTBadNames);
				 CloseStreetNameTable();
				 CloseNetIntersect (Opened);
				 CloseNetLinkAndRef (OpenedNLR);
                 DisableHalt = FALSE; 
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EDIT_DEST),TRUE); 
        Reset:
                 Processing = FALSE;
				 ContinueProcessing=TRUE;
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE); 
				 FileIsOpen=FALSE;
				 if (*AutoExportName)
		         	PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
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



BOOL FAR PASCAL INTACCIDPROFMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
    HCURSOR OldCursor=0;    
    static   HANDLE hSQL=0;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hBT;    
    long        Offset, TotLen, lineno=0, IntID, TotAccidents, LastID; 
    char        str[256], DestName[128], IntIDField[66]; 
    short       st, len,ifield;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    HANDLE		hDBDest;
    LPINTACCIDENTREC	pINTACC; 
//    static      short IMDataFileType;
//    static      char    IMDataFile[128]; 
    HFILE		FidSave;
    OFSTRUCT	OFStruct;
    char	Ext[6]=".NL3";
    BOOL	Opened;
    

 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam, 0,
                     SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, NULL,0,
                     IMDataFile, &IMDataFileType, &hSQL,NULL,TRUE))  return TRUE;
 switch(Message)
   {
    case WM_INITDIALOG:
    case GSSI_REINITDIALOG:
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

         switch(LOWORD(wParam))

         {  
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)  
                    ContinueProcessing=FALSE;
                 break;
            
            case IDC_EXIT: 
                 if (hSQL)
                    CloseDataFile (TRUE, &hSQL);  
				 DestroyFieldList ();
                 EndDialog(hWndDlg, TRUE); 
                 break;
                 
            case IDC_SAVE:
            {    
            	 short	Version=1;
            	 
            	 *str=0;
                 if (!GetSaveName2 (hWndDlg,str,0,Ext,IDS_FILENL3)) break; 
                 FidSave = GSSiOpenFile (str,&OFStruct,OF_CREATE);
                 BigWrite (FidSave,(HPSTR)Ext,6,-1);
                 BigWrite (FidSave,(HPSTR)&Version,2,-1);   
                 BigWrite (FidSave,(HPSTR)IMDataFile,lnIMDataFile,-1);
                 GetDlgItemText (hWndDlg,IDC_DEST_FILE,DestName,lnDestName);
                 BigWrite (FidSave,(HPSTR)DestName,lnDestName,-1);
                 GetDlgItemText (hWndDlg,IDC_SQL,str,sizeof(str));                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_INTID_FIELD,str,sizeof(str));
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GSSiClose (FidSave);
            } 
            	 break;
            	 
           	case IDC_RECALL: 
           	{
           		 short Version;
           		 
           		 *str=0;
				 if (!GetFileName2 (hWndDlg,str,Ext,IDS_FILENL3))
				 	break;
                 FidSave = GSSiOpenFile (str,&OFStruct,OF_READ);
                 BigRead (FidSave,Ext,6);
                 BigRead (FidSave,(HPSTR)&Version,2);   
                 BigRead (FidSave,IMDataFile,lnIMDataFile);
                 BigRead (FidSave,DestName,lnDestName);
                 SetDlgItemText (hWndDlg,IDC_DEST_FILE,DestName);
                 BigRead (FidSave,str,256);
                 SetDlgItemText (hWndDlg,IDC_SQL,str);                 
                 BigRead (FidSave,str,256);
                 SetDlgItemText (hWndDlg,IDC_INTID_FIELD,str);
                 GSSiClose (FidSave);
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
            }
           		 break;
           		 
            case IDC_LOCATE_DEST: 
				{
					char	ext[6]=".GMD";

                    *str=0;
                    if (!GetSaveName2 (hWndDlg,str,IDS_FILTERGWD,ext,IDS_FILEGMD)) break;   
                    SetDlgItemText (hWndDlg,IDC_DEST_FILE,str);
				}
                 break;
                  
            case IDC_OPEN_DB:
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),TRUE);
                 break;  
            
            case IDC_SHOW_FIELDS: 
            	 
            	 DisplayFieldList (hWndDlg,hSQL,NULL,0,0);
                 break;
                      
            case IDC_SETSQL:      
            {    
                 HANDLE hMem;
                 LPSTR  lpStr;
                 
                 hMem = GSSiGlobAlloc ( 566,GHND,4096);
                 lpStr = GlobalLock (hMem); 
                 GetDlgItemText (hWndDlg,IDC_SQL,lpStr,1024);
                 if (GetSQLWhereClause (hWndDlg, hSQL, lpStr))
                 	SetDlgItemText (hWndDlg,IDC_SQL,lpStr);    
                 GSSiGlobUlFree (&hMem);
                 break;
            }
            
            case IDOK: 
            {    
            	 long	TotalAccidents=0, TotalADT=0, Offset, IntID;
            	 double	AveAccRate;
            	 short	pos;  
            	 BOOL	OpenedSeg=FALSE;

                 
                 if (!GetDlgItemText (hWndDlg,IDC_DEST_FILE,DestName,lnDestName))
                 {
                    GSSiMsgBox(GetFocus(),"No destination file", 0,MB_ICONQUESTION|MB_OK,0);
                    break;
                 }
                if (!GetDlgItemText (hWndDlg,IDC_INTID_FIELD,IntIDField,sizeof(IntIDField)))
                {
                    GSSiMsgBox(GetFocus(),"No Intersection ID Field", 0,MB_ICONQUESTION|MB_OK,0);
                    break;
                }
                CloseDataFile (TRUE, &hSQL);  
                GetDlgItemText (hWndDlg,IDC_SQL,str,sizeof(str));                
                if (!_fstrcmp (str,"ALL ROWS"))
                    *str = 0;  
                _fstrcat (str," ORDER BY ");
                Strip (IntIDField,'[');
                Strip (IntIDField,']');
                _fstrcat (str,IntIDField);
                hSQL = 0;
                if (!OpenDataFile (IMDataFile,str,BT_READ,&hSQL))
                {  
                    GSSiMsgBox(GetFocus(),"Cannot open data file", 0,MB_ICONQUESTION|MB_OK,0);
                    break;
                }
                
                if (!OpenNetIntersect (NetworkID,FALSE,&Opened))
                	break; 
				if (!OpenStreetSegmentTable (FALSE,&OpenedSeg))
					break;
			    if (!CreateINT_ACCIDTable (DestName))
				    break;
			    hDBDest = OpenGWDatabase (DestName,BT_WRITE);
			    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest); 
                  
                EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE); 
                EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE); 
                OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT));
                TotLen = NumSQLRows (hSQL);  
	            GSSiSetCursor (OldCursor);
                
                pINTACC = (LPINTACCIDENTREC)&lpGWDHead->GWDData;
                    
                Processing = TRUE; 
                LastID = LONG_MIN;
                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Step1: Loading Data");
        NextLine:
                 if (!FetchDBRec (hSQL) || !ContinueProcessing)
                    goto EndFile; 
                    
                 GetDlgItemText (hWndDlg,IDC_INTID_FIELD,str,sizeof(str));
                 ExpandText(str); 
                 IntID = atol (str);
                 if (IntID != LastID)
                 {   
                 	if (LastID > LONG_MIN)
                 	{
				 		 len = lpGWDHead->Reclen;  
				 		 pINTACC->IntID = LastID;
				 		 pINTACC->Accidents = TotAccidents; 
				 		 pINTACC->ADT = GetIntADT (LastID,&pINTACC->NumStreets);
				 		 TotalAccidents += TotAccidents;
				 		 TotalADT += pINTACC->ADT;
						 GWDAddRecord (lpGWDHead,0,NULL);
					} 
	                TotAccidents = 1;
	                LastID = IntID;
				 }  
				 else
				 	TotAccidents++;
				 
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, lineno++,0);
                 goto NextLine;
                 
        EndFile: 
                 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Step2: Computing critical rates");
                 if (!TotalADT || !TotAccidents)
                 {
	                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Processing aborted due to invalid data");  
                 	GSSiMsgBox (GetFocus(),"No accident or ADT data",NULL,MB_ICONEXCLAMATION,0);
                 	goto Exit;
                 }
                 AveAccRate = ((double)TotalAccidents * 1000000)/(365 * (double)TotalADT); 
                 
                 lineno = 0;
        		 TotLen = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
                 pos = BT_FIRST;
    			 while (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&IntID,pos,BT_ANY,(LPSTR)&Offset))
			     {  
			     	pos = BT_NEXT;
					len = FillGWDData (lpGWDHead,Offset); 
			 	    pINTACC->AveAccidentRate = AveAccRate; 
			 	    if (pINTACC->ADT)
			 	    { 
			 	    	pINTACC->AccidentRate = ((double)pINTACC->Accidents * 1000000)/(365 * pINTACC->ADT);
			 	    	pINTACC->AccidentRate999 = ComputeCriticalAccidentRate (3.090,AveAccRate,pINTACC->ADT);
			 	    	pINTACC->AccidentRate995 = ComputeCriticalAccidentRate (2.576,AveAccRate,pINTACC->ADT);
			 	    	pINTACC->AccidentRate95 = ComputeCriticalAccidentRate (1.645,AveAccRate,pINTACC->ADT);
			 	    	pINTACC->AccidentRate90 = ComputeCriticalAccidentRate (1.282,AveAccRate,pINTACC->ADT);
			 	    	pINTACC->AccidentLevel = 1;  
			 	    	if (pINTACC->AccidentRate > pINTACC->AccidentRate90)
			 	    		pINTACC->AccidentLevel = 2;  
			 	    	if (pINTACC->AccidentRate > pINTACC->AccidentRate95)
			 	    		pINTACC->AccidentLevel = 3;  
			 	    	if (pINTACC->AccidentRate > pINTACC->AccidentRate995)
			 	    		pINTACC->AccidentLevel = 4;  
			 	    	if (pINTACC->AccidentRate > pINTACC->AccidentRate999)
			 	    		pINTACC->AccidentLevel = 5;  
			 	    }
			 	    else 
			 	    {
			 	    	pINTACC->AccidentRate = -1;
			 	    	pINTACC->AccidentLevel = 0;
			 	    }
				    GSSillseek (lpGWDHead->Fid,Offset,0);
				    BigWrite (lpGWDHead->Fid,(HPSTR)&len,2,-1);
				    BigWrite (lpGWDHead->Fid,(HPSTR)&lpGWDHead->GWDData,len,-1);  
				    SetGWDCurrentOffset (lpGWDHead,-1);
                 	PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, lineno++,0);
				 }
                 
	             SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Processing Complete");  
	   Exit:
                 if (hSQL)
                 {
                    CloseDataFile (TRUE, &hSQL);  
                 } 
    			 GlobalUnlock (hDBDest);
				 CloseGWDatabase (hDBDest); 
				 CloseNetIntersect (Opened);
				 CloseStreetSegmentTable (OpenedSeg);
                 Processing = FALSE;
				 ContinueProcessing=TRUE;
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EXIT),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE); 
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

BOOL FAR PASCAL TEST_STREETMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
    
 short    BRtn; 
 int   	  TabStops[2]={150,2000};
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
        SendDlgItemMessage (hWndDlg,IDC_LIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
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
				 CloseStreetNameTable();
                 EndDialog(hWndDlg, TRUE); 
                 break;
                 
            case IDOK: 
            {
            	char	Street[42], TrueName[64];
            	long	StreetNum;
            	short	pos;
            	
            	       
                GetDlgItemText (hWndDlg,IDC_NAME,Street,40);
				STNDSN_INIT(FALSE);
				STNDST(Street, (short)_fstrlen(Street),STDNAMv,NRONAMv,NMONLYv,
				                        SANSCHv,NANDCHv,NCMPNMv,ORIGNMv,SANSCPv,SANSCSv,NULL,NULL,NULL,NULL);
				SetDlgItemText (hWndDlg,IDC_STDNAME,STDNAMv); 
				pos = BT_FIRST;  
    			SendDlgItemMessage (hWndDlg,IDC_LIST,LB_RESETCONTENT,0,0);
				while ((StreetNum = GetStreetNumFromName (STDNAMv,2,pos,TrueName)))
				{   
					pos = BT_NEXT; 
					sprintf (_fstrchr (TrueName,0),"\t%ld",StreetNum);
                	SendDlgItemMessage (hWndDlg,IDC_LIST,LB_ADDSTRING,0,(LPARAM)TrueName);
				}
				
                break;
                 
            }   
          }
          break;

    default:
        return FALSE;
   }
 return TRUE;    
}

BOOL FAR PASCAL ADDRESS_EDITMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (849);
#endif
{ 
 char	str[32];   
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (849);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:  
        cwCenter(hWndDlg, -2);
    	ltoa (CurSegmentAdd,str,10);
    	SetDlgItemText (hWndDlg,IDC_HOUSE,str);
    	ltoa (CurSegmentAddInc,str,10);
    	SetDlgItemText (hWndDlg,IDC_INCREMENT,str);
		SendDlgItemMessage (hWndDlg,IDC_VERIFY,BM_SETCHECK,VerifyAddIncrement,0L);
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
                 EndDialog(hWndDlg, FALSE); 
                 break;
                 
            case IDOK: 
            {
				GetDlgItemText (hWndDlg,IDC_HOUSE,str,sizeof(str));
				CurSegmentAdd = atol (str); 
				GetDlgItemText (hWndDlg,IDC_INCREMENT,str,sizeof(str));
				CurSegmentAddInc = atol (str);  
				VerifyAddIncrement = SendDlgItemMessage (hWndDlg,IDC_VERIFY,BM_GETCHECK,0,0);
                EndDialog(hWndDlg, TRUE); 
                break;
                 
            }   
          }
          break;

    default:
{
#if ENABLETRACE
GSSiExitProg (849);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (849);
#endif
 return TRUE;    
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL STREET_NAME_EDITMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (850);
#endif
{ 
 char	str[32];   
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (850);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:  
    	ltoa (CurSegmentAdd,str,10);
    	SetDlgItemText (hWndDlg,IDC_HOUSE,str);
    	ltoa (CurSegmentAddInc,str,10);
    	SetDlgItemText (hWndDlg,IDC_INCREMENT,str);
		SendDlgItemMessage (hWndDlg,IDC_VERIFY,BM_SETCHECK,VerifyAddIncrement,0L);
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
                 EndDialog(hWndDlg, FALSE); 
                 break;
                 
            case IDOK: 
            {
				GetDlgItemText (hWndDlg,IDC_HOUSE,str,sizeof(str));
				CurSegmentAdd = atol (str); 
				GetDlgItemText (hWndDlg,IDC_INCREMENT,str,sizeof(str));
				CurSegmentAddInc = atol (str);  
				VerifyAddIncrement = SendDlgItemMessage (hWndDlg,IDC_VERIFY,BM_GETCHECK,0,0);
                EndDialog(hWndDlg, TRUE); 
                break;
                 
            }   
          }
          break;

    default:
{
#if ENABLETRACE
GSSiExitProg (850);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (850);
#endif
 return TRUE;    
}
#if ENABLETRACE
}
#endif
}

void SetAddMatchControl(HWND hWndDlg, UINT control)
{
	addMatchWnd = hWndDlg;
	if (control)
	{
		addMatchList = control;
	}
	else
	{
		addMatchList = IDM_STREET_MENU;
	}
	return;
}

void AddAddMatch(HWND hWndDlg, LPSTR str)
{
	if (str)
		SendDlgItemMessage(addMatchWnd, addMatchList, LB_ADDSTRING, 0, (LPARAM)str);
	else if (hWndDlg != addMatchWnd)
	{
		int nInList = SendDlgItemMessage(addMatchWnd, addMatchList, LB_GETCOUNT, 0, 0);
		RECT	rect, prect;
		int		h = nInList * 16 + 6;

		GetWindowRect(GetDlgItem(addMatchWnd, addMatchList), &rect);
		ScreenRectToClientRect(addMatchWnd, &rect);
		MoveWindow(GetDlgItem(addMatchWnd, addMatchList), rect.left, rect.top, RECTWIDTH(&rect), h, TRUE);
		ShowWindow(GetDlgItem(addMatchWnd, addMatchList), SW_SHOW);
		GetWindowRect(GetDlgItem(addMatchWnd, addMatchList), &rect);
		GetWindowRect(addMatchWnd, &prect);
		prect.bottom = rect.bottom + 7;
		MoveWindow(addMatchWnd, prect.left, prect.top, RECTWIDTH(&prect), RECTHEIGHT(&prect), TRUE);
	}

	return;
}

BOOL FAR PASCAL ADDRESSPIDMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
 short i,nchar,ikey;
 static LONG House;
 static short OddEven;
 BOOL translated;
 BOOL FAR *p_translated = &translated;
 char StreetBuf[34];
 char HouseBuf[16]; 
 char	MunName[68];
 char str[128];
 short  Choice, irc, st;
 static HWND CurFocus;
 int	TabStops[3]={550,600,700};
 long   Offset;  
 BOOL   Error;
 static	BOOL	OnlyPrime=TRUE;  
 static	long	MunicNum=0;  
 HWND	hPar, hParDlg;

 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
		SetSecondaryAddInput(0);
		addMatchList = IDM_STREET_MENU;
		addMatchWnd = hWndDlg;
		cwCenter(hWndDlg, 0);
         /* initialize working variables                                */
         SendDlgItemMessage (hWndDlg,IDM_STREET_MENU,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
#if WIN32
         CurFocus = (void *)IDM_HOUSE;
#else
         CurFocus = IDM_HOUSE;
#endif
         if (!OpenAddressFilesPID (hWndMain))
         	goto Close ;  
		 SendDlgItemMessage (hWndDlg,IDC_AUTOHIGHLIGHT,BM_SETCHECK,AutoHighlight,0L);
         AddRangeAll = GetGlobalBVal ("[%ADDRANGEALL]");  
         AddTol = GetGlobalLVal ("[%ADDTOL]");
         Trigger = GetGlobalLVal ("[%TRIGGER]");  
         itoa (Trigger,str,10);
    	 SetDlgItemText (hWndDlg,IDC_TRIGGER,str);
         itoa (AddTol,str,10);
    	 SetDlgItemText (hWndDlg,IDC_ADD_TOL,str); 
         if (AddRangeAll)
         {
            SendDlgItemMessage (hWndDlg,IDC_ADD_RANGE_NEAREST,BM_SETCHECK,FALSE,0L); 
            SendDlgItemMessage (hWndDlg,IDC_ADD_RANGE_ALL,BM_SETCHECK,TRUE,0L);
         } 
         else
         {
            SendDlgItemMessage (hWndDlg,IDC_ADD_RANGE_NEAREST,BM_SETCHECK,TRUE,0L); 
            SendDlgItemMessage (hWndDlg,IDC_ADD_RANGE_ALL,BM_SETCHECK,FALSE,0L);
         } 
         SetDlgItemText (hWndDlg,IDM_STREET,StreetBufSave);
         SetDlgItemText (hWndDlg,IDM_HOUSE,HouseBufSave); 
   		 if (GetGlobalCVal ("[%WANTCITY]",MunName,NULL))
   		 {  
   		 	sprintf (str,"%s only",MunName);
			MunicNum = GetMunicFromName (MunName);
   		 	SetDlgItemText (hWndDlg,IDC_IN_PRIMARY_CITY_ONLY,str);
         	ShowWindow (GetDlgItem (hWndDlg,IDC_IN_PRIMARY_CITY_ONLY),TRUE);
            SendDlgItemMessage (hWndDlg,IDC_IN_PRIMARY_CITY_ONLY,BM_SETCHECK,OnlyPrime,0L); 
         }
		 else
			MunicNum = 0;
		 hParDlg = GetParent(hWndDlg);
		 hPar = GetDlgItem(hParDlg, IDC_GEOCODE_OPERATION);

		 if (hPar)
		 {
			 RECT pRect;
			 RECT wRect;
			 GetWindowRect(hPar, &pRect);
			 GetWindowRect(hWndDlg, &wRect);
			 // ClientRectToScreenRect(GetParent(hPar), &pRect);
			 // ScreenRectToClientRect(hParDlg, &pRect);
			 MoveWindow(hWndDlg, pRect.left, pRect.top, RECTWIDTH(&pRect), RECTHEIGHT(&pRect), TRUE);
		 }
		 goto Display;
         
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
Close:
		 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDM_HOUSE: /* Edit Control                             */
                 switch (HIWORD(wParam))
                 {  case EN_KILLFOCUS:
                        nchar = GetDlgItemText(hWndDlg,IDM_HOUSE,HouseBuf,16);
                        if (nchar == 0 || HouseBuf[0] == '?')
                        {
                            House = 0;
                            OddEven = 0;
                        }
                        else if (HouseBuf[nchar-1] == '?' &
                                 HouseBuf[nchar-2] == '?')
                        {
                            OddEven = 3;
                            HouseBuf[nchar-2]='\0';
                            House = atol (HouseBuf);
                        }
                        else if (HouseBuf[nchar-1] == '?')
                        {
                            HouseBuf[nchar-1]='\0';
                            House = atol (HouseBuf);
                            OddEven = House % 2 +1;
                        }
                        else
                        {   HouseBuf[nchar]='\0';
                            House = atol (HouseBuf);
                            OddEven = 0;
                        };
                    break;

                    case EN_SETFOCUS:
#if WIN32
                        CurFocus = (void *)IDM_HOUSE;
#else
                        CurFocus = IDM_HOUSE;
#endif
                        break;

                    case EN_CHANGE:
                        nchar = GetDlgItemText(hWndDlg,IDM_HOUSE,HouseBuf,16);
                        if (nchar > 0)
                        {
                            if (!isdigit (HouseBuf[nchar-1]) &
                                 HouseBuf[nchar-1] != '?')
                            {
                                if (HouseBuf[nchar-1]==' ')
                                    StreetBuf[0]='\0';
                                else
                                {
                                    StreetBuf[0]=HouseBuf[nchar-1];
                                    StreetBuf[1]='\0';
                                }
                                HouseBuf[nchar-1]='\0';
                                SetDlgItemText (hWndDlg,IDM_HOUSE,HouseBuf);
                                SetFocus (GetDlgItem(hWndDlg,IDM_STREET));
                                SetDlgItemText(hWndDlg,IDM_STREET,StreetBuf);
                                SendDlgItemMessage(hWndDlg,IDM_STREET,
                                                EM_SETSEL,
                                                1,9999);
                            } 
                            else
                            {
#if WIN32
                                CurFocus = (void *)IDM_HOUSE;
#else
                                CurFocus = IDM_HOUSE;
#endif
                                nchar = GetDlgItemText(hWndDlg,IDM_HOUSE,HouseBuf,16);
                                House = atol (HouseBuf);
                                goto Display;
                            }

                        }
                    break;
                 }
                 break; 
                 
            case IDC_ADD_TOL:
                 AddTol = GetDlgItemInt(hWndDlg,IDC_ADD_TOL,&Error,FALSE); 
                 if (HIWORD(wParam)==EN_KILLFOCUS) goto Display;
                 break;  
                 
            case IDC_ADD_RANGE_ALL:
            case IDC_ADD_RANGE_NEAREST:
                 AddRangeAll = SendDlgItemMessage (hWndDlg,IDC_ADD_RANGE_ALL,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
                 goto Display;
                 
            case IDM_STREET: /* Edit Control                            */
#if WIN32
                CurFocus = (void *)IDM_STREET;
#else
                CurFocus = IDM_STREET;
#endif
                 switch (HIWORD(wParam))
                 {  case EN_CHANGE:
           Display:     
                        AddTol = GetDlgItemInt(hWndDlg,IDC_ADD_TOL,&Error,FALSE);
                        if (!Error)
                        {
                            break;
                        }
                        AddRangeAll = SendDlgItemMessage (hWndDlg,IDC_ADD_RANGE_ALL,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
                        i = GetDlgItemText (hWndDlg,IDM_STREET,StreetBuf,32); 
                       	Trigger = GetDlgItemInt (hWndDlg,IDC_TRIGGER,&Error,FALSE);
                        DisplayStreetsPID (hWndDlg,House-AddTol,House+AddTol,OddEven,StreetBuf,i,MunicNum,IDM_STREET); 
                        if (SendDlgItemMessage (hWndDlg,IDC_TRYHARDER,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
							PostMessage(hWndDlg, WM_COMMAND, IDC_TRY_MATCH, 0L);
                        	
                        break;

                    case EN_SETFOCUS:
#if WIN32
                        CurFocus = (void *)IDM_STREET;
#else
                        CurFocus = IDM_STREET;
#endif
                        break;

                 }
                 break;
            
            case IDC_TRYHARDER:
            case IDC_TRY_MATCH:  
            {
				HANDLE hList = GSSiGlobAlloc ( 574,GMEM_MOVEABLE,USHRT_MAX); 
				short	nList=0, nAdded;
				HANDLE	hMem=GSSiGlobAlloc ( 573,GMEM_MOVEABLE,512);
				LPSTR	STDNAM1=GlobalLock (hMem);
				LPSTR	NRONAM1=STDNAM1+50;
				LPSTR	NMONLY1=NRONAM1+50;
				LPSTR	SANSCH1=NMONLY1+50;
				LPSTR	NANDCH1=SANSCH1+50;
				LPSTR	NCMPNM1=NANDCH1+50;
				LPSTR	ORIGNM1=NCMPNM1+50;
				LPSTR	SANSCP1=ORIGNM1+50;
				LPSTR	SANSCS1=SANSCP1+50;	 
				LPLONG	pList;
				
//			    SendDlgItemMessage (hWndDlg,IDM_STREET_MENU,LB_RESETCONTENT,0,0);
				GetDlgItemText (hWndDlg,IDM_STREET,StreetBuf,32);
			    STNDST(StreetBuf, (short)_fstrlen(StreetBuf),STDNAM1,NRONAM1,NMONLY1,
			                             			   SANSCH1,NANDCH1,NCMPNM1,ORIGNM1,SANSCP1,SANSCS1,NULL,NULL,NULL,NULL); 
			    nAdded = GetNameTypeList (3,&nList,hList,STDNAM1,NRONAM1,NMONLY1,
			                             				 SANSCH1,NANDCH1,NCMPNM1,ORIGNM1,SANSCP1,SANSCS1); 
            	pList = (LPLONG)GlobalLock (hList);
            	for (i=0; i < nList; i++,pList++)
					DisplayStreetsPIDFromStreetNum (hWndDlg,House-AddTol,House+AddTol,OddEven,*pList,MunicNum,IDM_STREET);
            	GSSiGlobUlFree (&hMem);
            	GSSiGlobUlFree (&hList);
            }
            	break;
            	
            case IDM_STREET_MENU: /* List box                           */
                 switch(HIWORD(wParam))
                 {    
                     case LBN_DBLCLK:
                         Choice=(short)SendDlgItemMessage(hWndDlg,IDM_STREET_MENU,LB_GETCURSEL,0,0);
                         SendDlgItemMessage(hWndDlg,IDM_STREET_MENU,LB_GETTEXT,Choice,(DWORD)&str);
                   Exit: Offset = atol(_fstrrchr(str,'\t')+1);
						 AddToView = SendDlgItemMessage (hWndDlg,IDC_ADDTOVIEW,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
                         AddRefno = PIDAddRefno (Offset,AddUDI);
                         SetUDIValue(AddUDIVar,AddUDI);
                         CloseAddressFilesPID();
                         if (House) irc = 1;else irc=2; 
                         if (!AddRefno) irc = 0;
                         GetDlgItemText(hWndDlg,IDM_HOUSE,HouseBufSave,16);
                         GetDlgItemText (hWndDlg,IDM_STREET,StreetBufSave,32);
                         AddTol = GetDlgItemInt(hWndDlg,IDC_ADD_TOL,&Error,FALSE);
                         AddRangeAll = SendDlgItemMessage (hWndDlg,IDC_ADD_RANGE_ALL,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
                       	 Trigger = GetDlgItemInt (hWndDlg,IDC_TRIGGER,&Error,FALSE);
                       	 SetGlobalValueLong ("%ADDTOL",AddTol);
                       	 SetGlobalValueLong ("%TRIGGER",Trigger);
                       	 SetGlobalValueBool ("%ADDRANGEALL",AddRangeAll); 
       	 		    	 AutoHighlight=SendDlgItemMessage(hWndDlg,IDC_AUTOHIGHLIGHT,BM_GETCHECK,0,0); 
		         		 OnlyPrime = SendDlgItemMessage (hWndDlg,IDC_IN_PRIMARY_CITY_ONLY,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
                         EndDialog(hWndDlg,irc);
                     break;
                 }
                 break;

            case IDOK:
                 Choice=(short)SendDlgItemMessage(hWndDlg,IDM_STREET_MENU,LB_GETCURSEL,0,0); 
                 if (Choice<0) Choice=0;
                 st = (short)SendDlgItemMessage(hWndDlg,IDM_STREET_MENU,LB_GETTEXT,Choice,(DWORD)&str);
                 if (st && st != LB_ERR)
                    goto Exit;
                 else
                 {
                    SetFocus (GetDlgItem(hWndDlg,IDM_STREET)); 
#if WIN32
                    CurFocus = (void *)IDM_STREET;
#else
                    CurFocus = IDM_STREET;
#endif
                 }
                 break;

            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */   
                 CloseAddressFilesPID();
                 GetDlgItemText(hWndDlg,IDM_HOUSE,HouseBufSave,16);
                 GetDlgItemText (hWndDlg,IDM_STREET,StreetBufSave,32);
         		 OnlyPrime = SendDlgItemMessage (hWndDlg,IDC_IN_PRIMARY_CITY_ONLY,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
                 EndDialog(hWndDlg, FALSE);
                 break;
           }
         break;    /* End of WM_COMMAND                                 */
    case LB_ADDSTRING:
         i = 1;
         break;

    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL FAR PASCAL LOCATEPIDMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{   static  char    PID[34]="";
    char    str[34];
    short       nchar;

 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
         if (!OpenAddressFilesPID (hWndMain)) goto Close ;     
         SetDlgItemText(hWndDlg,IDC_ENTERPID,PID);

         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         Close: PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 CloseAddressFilesPID();
                 EndDialog(hWndDlg, FALSE);
                 break;

            case IDOK:
                 nchar = GetDlgItemText(hWndDlg,IDC_ENTERPID,PID,sizeof(PID)); 
                 _fstrcpy (AddUDI,PID);  
                 SetUDIValue(AddUDIVar,PID);
    /*             if (nchar < PidLen)
                 {
                     nspace = PidLen-nchar;
                     _fmemset(PID,' ',nspace);
                     PID[nspace]='\0';
                     _fstrcat(PID,str);
                 } 
                 else
                 {
                    _fstrcpy(PID,str);
                 }
                 if (BT_FIND (hPIDPid,PID,BT_FIRST,BT_EQ,(LPSTR)&Offset))
                 {
                    GSSiMsgBox( GetFocus(), PID,"Parcel not found", MB_OK);
                 }
                 else
                 {
                    AddRefno = PIDAddRefno (Offset,AddUDI);
                    EndDialog(hWndDlg, TRUE);
                 } */
                    EndDialog(hWndDlg, TRUE);
                CloseAddressFilesPID();

                 break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL INTERSECT_MATCH_EDITMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
    
	static	BOOL	Opened; 
	long	nChange;
	static	HANDLE	hDBDest, hBTBadNames;
	UINT	len; 
	LPADDMATCH	pMatch;
	LPINTMATCHEDITREC pIMER; 
	static	INTMATCHEDITREC IMER;
	LPGWDHEADER lpGWDHead;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    static	HANDLE	SaveHandle;
    HANDLE		hAddDB;
    static	HANDLE	hChangeRecs=0;   
    char	StreetBuf[42], str[256], Street1[66],Street2[66];
	INTMATCHEDITKEY1	IMEKey1;  
	static	BOOL	Skip1=FALSE, Skip2=FALSE; 
	LPSTR	lpTAB;
    int	    TabStops[2]={2000,2100};
    short	Choice, rc, match; 
    char	OrigName[64];
    long	NewNum, MunicNum;
	long	Offset, StreetNum1, StreetNum2, Recnum;
	HANDLE	hMatch;  
	static	long	CurEditRec, LastRecnum;
	short	IndexArray[2];	
	static	short	Index;   
	RECT	WindRect;
	
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG: 
    {
    	LPSTR	lpDot,pBadNameFile;
    	 
		ShowWindow (hWndHidden,SW_HIDE);
		WindRect = GetGlobalRectVal ("%INTMATCHEDITRECT",NULL); 
		if (WindRect.right > WindRect.left)
		{
			SetWindowPos(hWndDlg,HWND_TOP,WindRect.left,WindRect.top,
									   WindRect.right-WindRect.left,
									   WindRect.bottom-WindRect.top,SWP_NOZORDER);
		}
		CurEditRec = 0;
		Index = 1;
        SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
        SendDlgItemMessage (hWndDlg,IDC_STREET1_LIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
        SendDlgItemMessage (hWndDlg,IDC_STREET2_LIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
        SendDlgItemMessage (hWndDlg,IDC_VIEW_NOHITS,BM_SETCHECK,TRUE,0L); 
        if (!OpenNetIntersect (NetworkID,FALSE,&Opened))
        	break; 
        	
        PostMessage(hWndDlg, WM_COMMAND, IDC_NEXT, 0L);       
   }
        break; /* End of WM_INITDIALOG                                 */
    
   
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */
    
    case WM_DESTROY:
		 ShowWindow (hWndHidden,SW_SHOW);
         PostMessage(hWndHidden, WM_COMMAND, IDC_SHOW, 0L);
		 break;
		     
    case WM_COMMAND:

         switch(LOWORD(wParam))

         {  
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
				 CloseStreetNameTable();
				 CloseNetIntersect (Opened);
				 GetWindowRect (hWndDlg,&WindRect); 
				 SetGlobalValueRect ("%INTMATCHEDITRECT",WindRect);
				 DestroyWindow (hWndDlg);
//                 EndDialog(hWndDlg, TRUE); 
                 break;
                 
            case IDC_STREET1:
                 switch (HIWORD(wParam))
                 {
                   case EN_CHANGE:
                   		if (!Index)
                   			break; 
                   		if (Skip1)
                   		{
                   			Skip1=FALSE;
                   			break;
                   		}
                        GetDlgItemText (hWndDlg,LOWORD(wParam),StreetBuf,40); 
            			SendDlgItemMessage (hWndDlg,IDC_MATCHED1,BM_SETCHECK,FALSE,0L); 
                        ShowStreetMatches (StreetBuf,hWndDlg,IDC_STREET1_LIST,0);
                        goto ShowMatched;
                 }
                 break;

            case IDC_STREET2:
                 switch (HIWORD(wParam))
                 {
                   case EN_CHANGE:
                   		if (!Index)
                   			break; 
                   		if (Skip2)
                   		{
                   			Skip2=FALSE;
                   			break;
                   		}
                        GetDlgItemText (hWndDlg,LOWORD(wParam),StreetBuf,40); 
            			SendDlgItemMessage (hWndDlg,IDC_MATCHED2,BM_SETCHECK,FALSE,0L); 
                        ShowStreetMatches (StreetBuf,hWndDlg,IDC_STREET2_LIST,0);
                        goto ShowMatched;

                 }
                 break; 
                 
            case IDC_POSSIBLE: 
                 switch(HIWORD(wParam))
                 {   
                     case LBN_SELCHANGE: 
                         Choice=(short)SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0);
                         SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETTEXT,Choice,(DWORD)&str);
			    		 hDBDest = OpenGWDatabase (DestName,BT_WRITE);
			    		 if (!hDBDest)
			    		 	break;
					     lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest); 
		        		 pIMER = (LPINTMATCHEDITREC)&lpGWDHead->GWDData;
		        		 if (CurEditRec)
		        		 {   
		        		 	 short	len;
		        		 	 LPSTR	pData;
		        		 	 
			        		 BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&LastRecnum,BT_FIRST,BT_EQ,(LPSTR)&Offset); 
			        		 len = FillGWDData (lpGWDHead,Offset); 
			        		 if ((pData = _fstrchr(str,'\t')))
			        		 {
				        		 pData++;
								 pIMER->IM.MatchCode=1;
								 pIMER->IM.LocationCode=8;
								 pIMER->IM.IntID=ldread (pData,10);
								 pData+=10;
								 pIMER->IM.StreetNum1=ldread (pData,10);
								 pData+=10;
								 pIMER->IM.StreetNum2=ldread (pData,10);
								 pData+=10;
								 pIMER->IM.Point.x=dread (pData,20);
								 pData+=20;
								 pIMER->IM.Point.y=dread (pData,20);
		 				 		 IndexArray[1]=FALSE;
								 GWDReplaceRecord (lpGWDHead,0,IndexArray,Offset);
				    			 GlobalUnlock (hDBDest);
								 CloseGWDatabase (hDBDest);   
								 hDBDest = 0;
						         CurEditRec=0;
						     }       
					         PostMessage(hWndDlg, WM_COMMAND, IDC_NEXT, 0L);
		        		 }
		        		 break;
		         }
		         break;
                              
            case IDC_STREET1_LIST: 
            case IDC_STREET2_LIST: 
                 switch(HIWORD(wParam))
                 {   
                     
                 	 case LBN_SELCHANGE:
                     case LBN_DBLCLK:
                   		 if (!Index)
                   			break; 
                         Choice=(short)SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0);
                         SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETTEXT,Choice,(DWORD)&str);
                         lpTAB = _fstrrchr(str,'\t');
                         if (!lpTAB)
                         	break;
                         *lpTAB++=0;   
                         NewNum = atol (lpTAB); 
                         if (LOWORD(wParam) == IDC_STREET1_LIST)
	                         _fstrcpy (OrigName,IMER.StreetA); 
	                     else
	                         _fstrcpy (OrigName,IMER.StreetB);
                         nChange = ChangeSimilarStreets (DestName,OrigName,IMER.RecordNum,&hChangeRecs);
                         rc = IDYES;
                         if (nChange)
                         {
                         	char	Mess[128];
                         	
                         	sprintf (Mess,"Change %ld similar cases of '%s' to '%s'?",nChange,OrigName,str);
                         	rc = GSSiMsgBox (hWndDlg,Mess,"Verify Changes",MB_YESNO,0);
                         }
                     	 {
                     		LPLONG	pRec;
                     		long	NewMatches=0;
                         		
							SetDlgItemText (hWndDlg,IDC_MESS2,"");
                     		pRec = (LPLONG)GlobalLock (hChangeRecs);
	                     	if (rc == IDNO)
	                     	{
	                     		pRec++;
	                     		*pRec--=0;
	                     	}
                     		
						    hDBDest = OpenGWDatabase (DestName,BT_WRITE);
				    		if (!hDBDest)
				    		 	break;
						    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest); 
			        		pIMER = (LPINTMATCHEDITREC)&lpGWDHead->GWDData;
			        		while (*pRec)
			        		{
								BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)pRec++,BT_FIRST,BT_EQ,(LPSTR)&Offset); 
								FillGWDData (lpGWDHead,Offset);  
        		 				_fmemmove (&IMER,pIMER,(size_t)lpGWDHead->Reclen);
								if (!pIMER->IM.StreetNum1)
								{ 
									if (!_fstricmp (pIMER->StreetA,OrigName))
									{
										_fstrcpy (pIMER->StreetA,str); 
										pIMER->IM.StreetNum1 = NewNum;
									}
								}
								if (!pIMER->IM.StreetNum2)
								{ 
									if (!_fstricmp (pIMER->StreetB,OrigName))
									{
										_fstrcpy (pIMER->StreetB,str); 
										pIMER->IM.StreetNum2 = NewNum;
									}
								}
				 				match = INT_MATCH (pIMER->StreetA,pIMER->StreetB,"",0,MOPT,&hMatch,&StreetNum1,&StreetNum2,&MunicNum); 
								switch (match)
								{  
								case 1:
									NumMatched++; 
									NewMatches++;
									pMatch = (LPADDMATCH)GlobalLock (hMatch); 
									pIMER->IM = *pMatch;
								   	GlobalUnlock (hMatch);
									GSSiGlobFree (&hMatch);
							 		IndexArray[1]=FALSE;
									break;
								case 0:
								default: 
									pIMER->IM.MatchCode = match; 
									pIMER->IM.StreetNum1 = StreetNum1;
									pIMER->IM.StreetNum2 = StreetNum2; 
				 					IndexArray[1]=TRUE;
									break; 
								}
								GWDReplaceRecord (lpGWDHead,0,IndexArray,Offset);
							}
			    			GlobalUnlock (hDBDest);
							CloseGWDatabase (hDBDest);
							hDBDest = 0;
							GlobalUnlock (hChangeRecs);
							GSSiGlobFree (&hChangeRecs); 
							sprintf (str,"%ld new matches created",NewMatches);
							SetDlgItemText (hWndDlg,IDC_MESS2,str);
					        CurEditRec=0;       
					        PostMessage(hWndDlg, WM_COMMAND, IDC_NEXT, 0L);
                     	}
                     break;
                 }
                 break;    
            case IDC_DISPLAY_STREETS:
            	 if (!hWndLocStreet)
                 {
                 	  FARPROC lpfnLOC_STREETMsgProc;
                  
					  lpfnLOC_STREETMsgProc = MakeProcInstance((FARPROC)LOC_STREETMsgProc, hInst);
					  hWndLocStreet=CreateDialog(hInst,"LOC_STREET",hWndMain, lpfnLOC_STREETMsgProc);
                 }
			     else
				 	ShowWindow (hWndLocStreet,SW_SHOW);
		         PostMessage(hWndLocStreet, WM_COMMAND, IDC_CLEAR, 0L);
		         PostMessage(hWndLocStreet, WM_COMMAND, IDC_SHOWEDITSTREETS, 0L);
            	 break;
            	 
            case IDC_VIEW_NOHITS:
            	 CurEditRec = 0; 
            	 Index = 1;
		         PostMessage(hWndDlg, WM_COMMAND, IDC_NEXT, 0L); 
		         break;
            	 
            case IDC_VIEW_ALL:
            	 CurEditRec = 0; 
				 LastRecnum = 0; 
            	 Index = 0;
		         PostMessage(hWndDlg, WM_COMMAND, IDC_NEXT, 0L); 
		         break;
            	 
            case IDC_NEXT:
            {
            	 unsigned short len;  
            	 char	TrueName[40]; 
            	 LPLONG	pRecnum;
//				 BTHEAD BTHead;  
				 long	n=CurEditRec;
				 LPADDMATCH	pMatch; 
				 short	st;
            	 
	    		 hDBDest = OpenGWDatabase (DestName,BT_WRITE);
	    		 if (!hDBDest)
	    		 	break;
			     lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest); 
        		 pIMER = (LPINTMATCHEDITREC)&lpGWDHead->GWDData;
        		 if (CurEditRec)
        		 {   
        		 	 short	len;
        		 	 
	        		 BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&LastRecnum,BT_FIRST,BT_EQ,(LPSTR)&Offset); 
	        		 len = FillGWDData (lpGWDHead,Offset);
					 pIMER->IM.MatchCode+=100;
				 	 IndexArray[1]=TRUE;
					 GWDReplaceRecord (lpGWDHead,len,IndexArray,Offset);
        		 }  
        		 if (Index)    
        		 {
        		 	st = BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)&IMEKey1,BT_FIRST,BT_ANY,(LPSTR)&Offset); 
        		 	if (IMEKey1.MatchCode > 99)
        		 		st = 1;
        		 }
        		 else
        		 	st = BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&LastRecnum,BT_FIRST,BT_GT,(LPSTR)&Offset); 
//        		 while (n--)
//      		 	BT_FIND (lpGWDHead->BTHandle[1],&IMEKey1,BT_NEXT,BT_ANY,(LPSTR)&Offset);
				 if (st)
				 {
		 			EnableWindow (GetDlgItem(hWndDlg,IDC_NEXT),FALSE);  
        		    GlobalUnlock (hDBDest);
				 	CloseGWDatabase (hDBDest);    
				 	hDBDest = 0;
		 			break;
				 } 
        		 FillGWDData (lpGWDHead,Offset);
        		 pRecnum =(LPLONG)&lpGWDHead->GWDData;
        		 LastRecnum = *pRecnum;  
//	         	 GetBTHeader (BT_NUM_IN_INDEX (lpGWDHead->BTHandle[Index]),&BTHead);
	         	 if (Index)
	         	 { 
		         	 CurEditRec++;
	        		 sprintf (str,"Record %ld of %ld (Original record %ld) Match code %i",CurEditRec,BT_NUM_IN_INDEX (lpGWDHead->BTHandle[Index]),*pRecnum,pIMER->IM.MatchCode); 
	        	 }  
	        	 else
	        		 sprintf (str,"Record %ld of %ld - Match code %i",*pRecnum,BT_NUM_IN_INDEX (lpGWDHead->BTHandle[Index]),pIMER->IM.MatchCode); 
				 SetDlgItemText (hWndDlg,IDC_MESS1,str);
        		 _fmemmove (&IMER,pIMER,(size_t)lpGWDHead->Reclen);
        		 if (pIMER->IM.StreetNum1)
        		 {
        		 	Skip1=TRUE; 
					GetTrueStreetName (pIMER->IM.StreetNum1, TrueName, 0,0); 
					SendDlgItemMessage (hWndDlg,IDC_STREET1_LIST,LB_RESETCONTENT,0,0);
				    SendDlgItemMessage (hWndDlg,IDC_STREET1_LIST,LB_ADDSTRING,0,(LPARAM)TrueName);
        		 }
        		 if (pIMER->IM.StreetNum2)
        		 {
        		 	Skip2=TRUE; 
					GetTrueStreetName (pIMER->IM.StreetNum2, TrueName, 0,0); 
					SendDlgItemMessage (hWndDlg,IDC_STREET2_LIST,LB_RESETCONTENT,0,0);
				    SendDlgItemMessage (hWndDlg,IDC_STREET2_LIST,LB_ADDSTRING,0,(LPARAM)TrueName);
        		 }
            	 SendDlgItemMessage (hWndDlg,IDC_MATCHED1,BM_SETCHECK,(WPARAM)(pIMER->IM.StreetNum1>0),0L); 
            	 SendDlgItemMessage (hWndDlg,IDC_MATCHED2,BM_SETCHECK,(WPARAM)(pIMER->IM.StreetNum2>0),0L); 
            	 EditPath1 = pIMER->IM.StreetNum1;
            	 EditPath2 = pIMER->IM.StreetNum2;
            	 ShowMatches = FALSE; 
        		 SetDlgItemText (hWndDlg,IDC_STREET1,pIMER->StreetA);
        		 SetDlgItemText (hWndDlg,IDC_STREET2,pIMER->StreetB);
        		 if (pIMER->IM.MatchCode == 1)
        		 {
	    			SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_RESETCONTENT,0,0);
					GetTrueStreetName (pIMER->IM.StreetNum1, str, 0,0); 
					_fstrcat (str," at ");
					GetTrueStreetName (pIMER->IM.StreetNum2, _fstrchr(str,0), 0,0);
                    SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_ADDSTRING,0,(LPARAM)str);
					ShowMatches = FALSE;
        		 } 
        		 else
					 ShowMatches = TRUE;
        		 GlobalUnlock (hDBDest);
				 CloseGWDatabase (hDBDest);  
				 hDBDest = 0;
     ShowMatched: 
            	 if (SendDlgItemMessage (hWndDlg,IDC_MATCHED1,BM_GETCHECK,0,0L) &&
            	 	 SendDlgItemMessage (hWndDlg,IDC_MATCHED2,BM_GETCHECK,0,0L))
					 EnableWindow (GetDlgItem(hWndDlg,IDC_DISPLAY_STREETS),TRUE);
				 else      			 
					 EnableWindow (GetDlgItem(hWndDlg,IDC_DISPLAY_STREETS),FALSE);      			 
     			 if (!ShowMatches)
     			 	break;
    			 SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_RESETCONTENT,0,0);
        		 GetDlgItemText (hWndDlg,IDC_STREET1,Street1,256);
        		 GetDlgItemText (hWndDlg,IDC_STREET2,Street2,256);
				 match = INT_MATCH (Street1,Street2,"",0,3,&hMatch,&StreetNum1,&StreetNum2,&MunicNum); 
				 if (match)
				 	pMatch = (LPADDMATCH)GlobalLock (hMatch);
				 else
				 	pMatch = NULL;
				 while (match--)
				 {  
					GetTrueStreetName (pMatch->StreetNum1, str, 0,0); 
					_fstrcat (str," at ");
					GetTrueStreetName (pMatch->StreetNum2, _fstrchr(str,0), 0,0);
					sprintf (_fstrchr(str,0),"\t%10ld%10ld%10ld%20.8f%20.8f",pMatch->IntID,pMatch->StreetNum1,pMatch->StreetNum2,
							 					pMatch->Point.x,pMatch->Point.y);
                    SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_ADDSTRING,0,(LPARAM)str);
                    pMatch++;
				 } 
		 		 GSSiGlobUlFree (&hMatch);
        	}
            	 break;  
            	 
            case IDC_CANCEL:	
            	 ContinueProcessing = FALSE;
            	 break;
            	 
            case IDC_APPLY_CHANGES:
            {
            	 HANDLE hSQL=0, hDB=0;
            	 char	OrigDB[128], UpString[256], SQL[256], KeyField[128];
            	 unsigned short len;  
            	 LPLONG	pRecnum;
				 LPADDMATCH	pMatch; 
				 short	st;
				 long	TotLen, num=0;
			     LPGWDHEADER lpGWDHead;
	
            	if (!GetDlgItemText (hWndHidden,SV_DATABASE_LIST,OrigDB,sizeof(OrigDB)))
            		break;
            	if (!GetDlgItemText (hWndHidden,IDC_UPDATE_STRING,UpString,sizeof(UpString)))
            		break;
            	if (!GetDlgItemText (hWndHidden,IDC_KEY_FIELD,KeyField,sizeof(KeyField)))
            		break;
            	
            	Strip (KeyField,'[');
            	Strip (KeyField,']');
	            if (!OpenDataFile (DestName,"",BT_READ,&hDB))
	            {  
                    GSSiMsgBox(GetFocus(),"Cannot open data file", DestName,MB_ICONQUESTION|MB_OK,0);
                    break;
	            }
		        SQLPtr = (LPOPENSQLDATA)GlobalLock (hDB); 
		        FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
        		lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle); 
        		TotLen = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
        		GlobalUnlock (FilePtr->FileHandle); 
           		GlobalUnlock (SQLPtr->OFHandle);
        		GlobalUnlock (hDB);
                if (!OpenDataFile (OrigDB,"",BT_READ,&hSQL))
                	break;
		        SQLPtr = (LPOPENSQLDATA)GlobalLock (hSQL); 
		        FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
		 		EnableWindow (GetDlgItem(hWndDlg,IDC_CANCEL),TRUE);  
	            while (FetchDBRec (hDB) && ContinueProcessing)
	            {   
					GetDlgItemText (hWndHidden,IDC_UPDATE_STRING,UpString,sizeof(UpString)); 
					ExpandText (UpString);
					sprintf (SQL,"%s = [OriginalFileKey]",KeyField);
           			UpdateExternalFieldData (FilePtr,SQL,UpString);
             		PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotLen, num++,0);
           		} 
           		GlobalUnlock (SQLPtr->OFHandle);
           		GlobalUnlock (hSQL);
                CloseDataFile (TRUE, &hSQL);  
                CloseDataFile (TRUE, &hDB);  
		 		EnableWindow (GetDlgItem(hWndDlg,IDC_CANCEL),FALSE);  
                ContinueProcessing = TRUE;
            } 
                break;
                
          }
          break;

    default:
        return FALSE;
   }
 return TRUE;    
} 

BOOL DisplayAddressRecInVP (int type,DPOINT WPoint,DPOINT WPoint2,short PixelSize,COLORREF Color)
#if ENABLETRACE
{GSSiEnterProg (1128);
#endif
{    
	LPSYMBOL    CurSymbol;
	HANDLE	hSymbol;
	double	Rot,QuaterPY=HALFPI/2.0;
    BOOL	SaveHVFC=HaveVarFillColor;
    HBRUSH	hOldBrush,hBrush; 
	HPEN	hOldPen, hPen;
    short	symnum;
	POINT	Point, Point2, VPMidPoint,Points[2];
	HANDLE	hSaveScreen=0;

	if (!(symnum = GetDictSymbolNumber ("CIRCLE")))
{
#if ENABLETRACE
GSSiExitProg (1128);
#endif
		return 0;
}
	hSymbol = GetDictSymDesc (symnum,0); 
	if (!hSymbol)
{
#if ENABLETRACE
GSSiExitProg (1128);
#endif
		return 0;
}
	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
	VPMidPoint = RectMid (&CurView->DrawRect);	
	Point = BasePtToWinPt (&WPoint);
	hBrush = CreateSolidBrush (Color);
	hOldBrush = SelectObject (CurView->hDC,hBrush);
    HaveVarFillColor = TRUE;  
    GlobalColors[0]=Color;
	switch (type)
	{
	case 1:
		DisplayPointSymbol (hSymbol,CurView->hDC, PixelSize, PixelSize,0, &Point,0,FALSE,0,0,FALSE,FALSE,0,0); 
		break;
	case 2:
		Points[0] = Point;
		Points[1] = BasePtToWinPt (&WPoint2);
		hPen = CreatePen (PS_SOLID,3,Color);
        hOldPen = SelectObject (CurView->hDC,hPen);
		Polyline (CurView->hDC,Points,2);
		SelectObject (CurView->hDC,hOldPen);
		DeleteObject (hPen);
		break;
	}
	DestroySymbol (hSymbol); 
	HaveVarFillColor = SaveHVFC;
	SelectObject (CurView->hDC,hOldBrush);    
	DeleteObject (hBrush);
	RestoreDC (CurView->hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (1128);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}


BOOL AddressTestPlot (LPSTR File)
{
	HANDLE	hDBDest = OpenGWDatabase (File,BT_READ);
	LPGWDHEADER	lpGWDHead;
	LPADDMATCHEDITREC	pAMER;
	int		Recid, Offset, pos=BT_FIRST, len, ii;

	if (!hDBDest)
		return FALSE;
	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest); 
	//pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData;
	pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData[lpGWDHead->pFldInfo->Len];
    SetViewport (*pCommandViewport);
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn (FALSE,FALSE);
    SelectClipRgn (CurView->hDC,CurView->hRgn);
    GSSiDeleteObject(&CurView->hRgn);  
	while (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&Recid,pos,BT_ANY,(LPSTR)&Offset))
	{
		pos = BT_NEXT;
		len = FillGWDData (lpGWDHead,Offset); 
		if (pAMER->AM.MatchCode == 1 || pAMER->AM.MatchCode == 2)
		{
			switch (pAMER->AM.LocationCode)
			{
			case 1:
				DisplayAddressRecInVP (1,pAMER->AM.Point,pAMER->AM.Point2,12,RGB(255,0,0));
				break;
			case 2:
				DisplayAddressRecInVP (1,pAMER->AM.Point,pAMER->AM.Point2,12,RGB(0,255,0));
				break;
			case 4:
				DisplayAddressRecInVP (1,pAMER->AM.Point,pAMER->AM.Point2,12,RGB(0,0,255));
				break;
			case 7:
				DisplayAddressRecInVP (1,pAMER->AM.Point,pAMER->AM.Point2,12,RGB(0,255,255));
				break;
			case 8:
				DisplayAddressRecInVP (1,pAMER->AM.Point,pAMER->AM.Point2,12,RGB(255,0,255));
				break;
			case 11:
				DisplayAddressRecInVP (2,pAMER->AM.Point,pAMER->AM.Point2,12,RGB(255,0,0));
				break;
			case 12:
				DisplayAddressRecInVP (2,pAMER->AM.Point,pAMER->AM.Point2,12,RGB(0,255,0));
				break;
			case 13:
				DisplayAddressRecInVP (2,pAMER->AM.Point,pAMER->AM.Point2,12,RGB(0,0,255));
				break;
			default:
				ii=1;
				break;
			}
		}
	}
	GlobalUnlock (hDBDest);
	CloseGWDatabase (hDBDest); 
	return TRUE;
}

BOOL FAR PASCAL ADD_MATCH_EDITMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
    
	static	BOOL	Opened, OpenedZB, OpenedSP; 
	long	nChange;
	static	HANDLE	hDBDest, hBTBadNames;
	UINT	len; 
	LPADDMATCH	pMatch;
	LPADDMATCHEDITREC pAMER; 
	static	ADDMATCHEDITREC AMER;
	LPGWDHEADER lpGWDHead;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    static	HANDLE	SaveHandle;
    HANDLE		hAddDB;
    static	HANDLE	hChangeRecs=0;   
	INTMATCHEDITKEY1	AMEKey1;  
	static	BOOL	Skip=FALSE; 
	LPSTR	lpTAB;
    int     TabStops[2]={2000,2100};
    short	Choice, rc, match, Choice1, Choice2,NumNearHouse, i; 
    long	NewNum, Snum;
	long	Offset, StreetNum, Recnum, WantHouse, NearHouse[32];
	DPOINT	NearPoint[32];
	static	HANDLE	hMatch;  
	static	long	CurEditRec, LastRecnum, CurIndexRec;
	short	IndexArray[2];	
	static	short	Index;
	RECT	WindRect;    
	long	MunicNum, ZIPCode, NumSegs;
	static	USERLOCATEDADDRESSKEY	ULAddKey; 
	static	BOOL	FirstAttempt;
	BOOL	OneMatch;  
	static	HANDLE	hSaveScreen=0, hSaveVPScreen;  
	static	long	SaveVPID=0;  
    DPOINT	p1,p2;
	COLORREF	Color; 
    LPSTR	str, Street, StreetBuf, House, City, ZIP, MunName;
	LPSTR	Street1,Street2, OnStreet;
    LPSTR	OrigName;
	LPSTR	SQL, KeyFieldName, KeyFieldValue;
	LPSTR	TrueName,MunAbv,MapQuestAdd,AddEditReport,AddEditUpdateMacro; 
	HANDLE	hMem=0;
	static	long TotRecs=-1;
	static	short	MC, CurType;   
	static	BOOL	DisplayOnlyAddressesInCurrentBounds;
	long	Munic;  
	int		item,imatch;
#define SELMATCH		1
#define SHOWHOUSES		2 
#define ZOOMTOSTREET	3 
	static	short	NextMatchCode=0, SelMatchOpt = SELMATCH;     
	UINT	EntryControls[3]={IDC_HOUSE,IDC_STREET,IDC_ZIP}; 
	short	nRc;
	static	BOOL	isModeless;
	static	DOUBLE	OrigScale;
	static	DPOINT	OrigMidPointW;
	static	DPOINT	UserLocatedFromPoint;
	static	MNMXCORD	DisplayBounds;
	static	char	CorrectedAddress[128];
	BOOL	UseMapQuest = FALSE;

	
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);

#define	lnstr	256
#define	lnStreet 66
#define	lnStreetBuf 66
#define	lnHouse 16
#define	lnCity 34
#define	lnZIP 34
#define	lnMunName 68
#define	lnStreet1 128
#define	lnStreet2 64
#define lnOnStreet 64
#define	lnOrigName 64
#define	lnSQL 256
#define	lnKeyFieldName 66
#define	lnKeyFieldValue 128
#define	lnTrueName 40
#define lnMunAbv 64
#define lnMapQuestAdd 128
#define lnAddEditReport 256
#define lnAddEditUpdateMacro	256
    
    hMem = GSSiGlobAlloc ( 576,GMEM_MOVEABLE,4096*2);
    str = GlobalLock (hMem);
    Street = str + lnstr;
    StreetBuf = Street + lnStreet;
    House = StreetBuf + lnStreetBuf;
    City = House + lnHouse;
    ZIP = City + lnCity;
    MunName = ZIP + lnZIP;
    Street1 = MunName + lnMunName;
    Street2 = Street1 + lnStreet1;
	OnStreet = Street2 + lnStreet2;
    OrigName = OnStreet + lnOnStreet;
    SQL = OrigName + lnOrigName;
    KeyFieldName = SQL + lnSQL;
    KeyFieldValue = KeyFieldName + lnKeyFieldName;
    TrueName = KeyFieldValue + lnKeyFieldValue;
    MunAbv = TrueName + lnTrueName;
	MapQuestAdd = MunAbv + lnMunAbv;
	AddEditReport = MapQuestAdd + lnMapQuestAdd;
	AddEditUpdateMacro = AddEditReport + lnAddEditReport;
 switch(Message)
   {
    case WM_INITDIALOG: 
    {
    	LPSTR	lpDot,pBadNameFile;  
    	short	width, height;
    	
		isModeless = FALSE;
    	hMatch = 0;
   		SetViewport (*pCommandViewport);
		OrigMidPointW = CurView->MidPointW;
		OrigScale = CurView->Scale;
   		DisplayOnlyAddressesInCurrentBounds = FALSE;
    	SetFocus (GetDlgItem(hWndDlg,IDC_HOUSE));
		if (GetGlobalCVal ("[%ADDEDITREPORT]",AddEditReport,0))
			ShowWindow (GetDlgItem (hWndDlg,IDC_REPORT),SW_SHOW);
    	TotRecs = -1; 
		NextMatchCode=0;
		ShowWindow (hWndHidden,SW_HIDE);   
		GetWindowRect (hWndDlg,&WindRect);
		width = WindRect.right - WindRect.left;
		height = WindRect.bottom - WindRect.top;
		WindRect = GetGlobalRectVal ("[%ADDMATCHEDITRECT]",NULL); 
		if (WindRect.right > WindRect.left)
		{
			SetWindowPos(hWndDlg,HWND_TOP,WindRect.left,WindRect.top,
									   width,height,SWP_NOZORDER);
		}
		else
			cwCenterBelowCursor (hWndDlg, 0);
		if (GetZIPCenter (0,NULL))
			EnableWindow (GetDlgItem(hWndDlg,IDC_MATCHTOZIP),TRUE);
		CurEditRec = LastRecnum = CurIndexRec = 0;
		Index = 1;
        SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
        SendDlgItemMessage (hWndDlg,IDC_STREET_LIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
        SendDlgItemMessage (hWndDlg,IDC_STREET_LIST1,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
        SendDlgItemMessage (hWndDlg,IDC_STREET_LIST2,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
//        SendDlgItemMessage (hWndDlg,IDC_VIEW_NOHITS,BM_SETCHECK,TRUE,0L); 
        if (!OpenNetIntersect (NetworkID,FALSE,&Opened))
        	break;
        if (!OpenZIPBounds (&OpenedZB))
        	break; 
        OpenStreetPolys (&OpenedSP);	
        hWndAddEdit = hWndDlg;
        if (*DestName)      
        	PostMessage(hWndDlg, WM_COMMAND, IDC_NEXT, 0L);  
        else
        {   
        	LPSTR	pStreet=AddEditStreet;
        	
        	SetDlgItemText (hWndDlg,IDC_ZIP,AddEditZIP);
        	if (!_fstricmp (AddEditHouseNum,"STREET")) 
            	pStreet = GetHouseAndStreet (pStreet,AddEditHouseNum); 
            else
            	pStreet = AddEditStreet;
        	SetDlgItemText (hWndDlg,IDC_CITY,AddEditCity);
        	SetDlgItemText (hWndDlg,IDC_HOUSE,AddEditHouseNum);
        	SetDlgItemText (hWndDlg,IDC_STREET,pStreet);
        }
   }
        break; /* End of WM_INITDIALOG                                 */
    
   
    case WM_DESTROY:   
         GSSiGlobFree (&hMatch);
		 CloseStreetNameTable();
		 CloseSegMaxIndex (TRUE);
		 CloseStreetSegmentTable (TRUE); 
		 GetMunicFromName (NULL);
		 GetNumZIPsInMunic (0,NULL);
		 CloseNetIntersect (Opened); 
		 CloseStreetPolys (OpenedSP);
		 CloseZIPBounds (OpenedZB);   
		 GetWindowRect (hWndDlg,&WindRect); 
		 SetGlobalValueRect ("%ADDMATCHEDITRECT",WindRect);
		 DestroySavedScreen (&hSaveVPScreen,SaveVPID);
    	 ClearSpecial ();
		 ClearFullWindowBitmap (0);
    	 if (*AutoExportName)
	         PostMessage(hWndHidden, WM_COMMAND, IDC_EXIT, 0L);  
    	 else
    	 {
			 ShowWindow (hWndHidden,SW_SHOW);
	         PostMessage(hWndHidden, WM_COMMAND, IDC_SHOW, 0L);  
	     } 
		 break;
		     
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

         switch(LOWORD(wParam))

         {
			case IDC_ISMODELESS:
			 isModeless = TRUE;
			 break;

         	case IDOK:
			     hWndAddEdit = 0;
				 if (isModeless)
				 	DestroyWindow (hWndDlg);
				 else
                 	EndDialog(hWndDlg, TRUE); 
                 break;
            
            case IDCANCEL:
			     hWndAddEdit = 0;
				 if (isModeless)
				 	DestroyWindow (hWndDlg);
				 else
                 	EndDialog(hWndDlg, FALSE); 
                 break;

            case IDC_HOUSE: 
            	 if (HIWORD(wParam) == EN_CHANGE)
                 	goto ShowMatched;
            	 if (HIWORD(wParam) == EN_SETFOCUS)
                 {
//                 	SelMatchOpt = SHOWHOUSES;
//                 	SetDlgItemText (hWndDlg,IDC_SELECTMATCH,"Show Similar House Numbers");
                 }
                 break;
     
            case IDC_STREET:
                 switch (HIWORD(wParam))
                 {
                   case EN_CHANGE:
                   		if (!Index)
                   			break; 
/*                   		if (Skip)
                   		{
                   			Skip=FALSE;
                   			break;
                   		}  */
						ClearSpecial();
						DBoundsInit (&DisplayBounds);
                        GetDlgItemText (hWndDlg,LOWORD(wParam),StreetBuf,64); 
						strcpy (CorrectedAddress,StreetBuf);
			 	 	 	if (SeparateIntStreets (StreetBuf,Street1,Street2))
			 	 	 	{   
			 	 	 		ShowWindow (GetDlgItem (hWndDlg,IDC_STREET_LIST),FALSE);
			 	 	 		ShowWindow (GetDlgItem (hWndDlg,IDC_STREET_LIST1),TRUE);
			 	 	 		ShowWindow (GetDlgItem (hWndDlg,IDC_STREET_LIST2),TRUE);
	                        ShowStreetMatches (Street1,hWndDlg,IDC_STREET_LIST1,&DisplayBounds);  
	                        ShowStreetMatches (Street2,hWndDlg,IDC_STREET_LIST2,&DisplayBounds);  
			 	 	 	}
			 	 	 	else  
			 	 	 	{
			 	 	 		ShowWindow (GetDlgItem (hWndDlg,IDC_STREET_LIST),TRUE);
			 	 	 		ShowWindow (GetDlgItem (hWndDlg,IDC_STREET_LIST1),FALSE);
			 	 	 		ShowWindow (GetDlgItem (hWndDlg,IDC_STREET_LIST2),FALSE);
	                        ShowStreetMatches (StreetBuf,hWndDlg,IDC_STREET_LIST,&DisplayBounds);  
	                    }
                        goto ShowMatched;

                 }
                 break;
				 case IDC_TRYMAPQUEST:
					 UseMapQuest = TRUE;
					 goto ShowMatched;

            case 65001:
			case IDC_SELECTMATCH: 
				switch (SelMatchOpt)
				{
					case SELMATCH:
					{
		    		 	 LPSTR	pData;
		    		 	 char	Type;
		    		 	 int	imatch; 
		             	 BOOL	Err;


			             Choice=(short)SendDlgItemMessage(hWndDlg,IDC_POSSIBLE,LB_GETCURSEL,0,0);
			             if (Choice == LB_ERR)
			             	break;
			             SendDlgItemMessage(hWndDlg,IDC_POSSIBLE,LB_GETTEXT,Choice,(DWORD)str);     
			             imatch = SendDlgItemMessage(hWndDlg,IDC_POSSIBLE,LB_GETITEMDATA,Choice,0);
			             if (pAddEditMatch && hMatch)
			             {  
			             	if (imatch >= 0)
			             	{
				             	LPADDMATCH pMatch = (LPADDMATCH)GlobalLock (hMatch);
				             	
				             	pMatch += imatch;
				             	*pAddEditMatch = *pMatch;
				             	GlobalUnlock (hMatch);
				             }
				             else if (imatch == -9)
				             {  
				             	LPSTR	pTab=_fstrrchr (str,'\t'); 
				             	LPSTR	pZIP=pTab+2;
				             	
				             	pTab+=12;   
				             	pAddEditMatch->ZIP = ldread (pZIP,10); 
				             	pAddEditMatch->LocationCode = 9;
				             	pAddEditMatch->Point = atopt (pTab,&Err);
				             }
			             }     
		        		 pData = _fstrchr(str,'\t');
		        		 *pData++ = 0; 
		        		 Type = *pData++;
	                     if (!*DestName)
	                     {   
	                     	DPOINT	Point;
	                     	
							pData+=10;
							Point = atopt (pData,&Err);;
			    			SetGlobalValueDPoint (AddEditOutVar,Point);  
			    			if (*AddEditMacro)
			    				ProcessText (AddEditMacro);
							PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
							break;
	                     }      

			    		 hDBDest = OpenGWDatabase (DestName,BT_WRITE);
			    		 if (!hDBDest)
			    		 	break;
					     lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest); 
			    		 //pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData;
			             pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData[lpGWDHead->pFldInfo->Len];
						 SetGlobalValue ("%ADDMATCHEDITFILE",DestName);
						 strcpy (CorrectedAddress,str);
						 SetGlobalValue ("%CORRECTEDADDRESS",CorrectedAddress);

			    		 if (CurEditRec)
			    		 {   
			    		 	 short	len;
					        		 	 
			        		 BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&CurEditRec,BT_FIRST,BT_EQ,(LPSTR)&Offset); 
			        		 len = FillGWDData (lpGWDHead,Offset); 
							 pAMER->AM.MatchCode=1;
							 pAMER->AM.LocationCode=8;
					 		 pAMER->AM.StreetNum = 0; 
					 		 pAMER->AM.IntID = 0;  
							 switch (Type)
							 {
							 	case 'A':
							 		pAMER->AM.StreetNum=ldread (pData,10); 
									pData+=10;
									pAMER->AM.Point.x=dread (pData,20);
									pData+=20;
									pAMER->AM.Point.y=dread (pData,20);
							 		break;
							 	case 'I':
							 		pAMER->AM.IntID = ldread (pData,10);  
									pData+=10;
									pAMER->AM.Point.x=dread (pData,20);
									pData+=20;
									pAMER->AM.Point.y=dread (pData,20);
							 		break;
							 	case 'U':
								 	pAMER->AM.LocationCode=10;
									pData+=10;
									pAMER->AM.Point.x=dread (pData,20);
									pData+=20;
									pAMER->AM.Point.y=dread (pData,20);
							 		break;
							 	case 'Z':
								 	pAMER->AM.LocationCode=5;
									pData+=10;
									pAMER->AM.Point.x=dread (pData,20);
									pData+=20;
									pAMER->AM.Point.y=dread (pData,20);
							 		break;
							 	case 'R'://user defined
								 	pAMER->AM.LocationCode=7;
									pData+=10;
									pAMER->AM.Point.x=dread (pData,20);
									pData+=20;
									pAMER->AM.Point.y=dread (pData,20);
							 		break;
							 	case 'Q'://user defined questionable
								 	pAMER->AM.LocationCode=6;
									pData+=10;
									pAMER->AM.Point.x=dread (pData,20);
									pData+=20;
									pAMER->AM.Point.y=dread (pData,20);
							 		break;
							 	case 'O':
								 	pAMER->AM.LocationCode=12;
									pData+=10;
									pAMER->AM.Point.x=dread (pData,20);
									pData+=20;
									pAMER->AM.Point.y=dread (pData,20);
									pData+=20;
									pData+=10;
									pAMER->AM.Point2.x=dread (pData,20);
									pData+=20;
									pAMER->AM.Point2.y=dread (pData,20);
							 		break;
							 }
UpdateAddEditFile:
					 		 IndexArray[1]=FALSE;
							 {
								 long	KeyVal = *(LPLONG)lpGWDHead->GWDData;

								 SetGlobalValueLong ("%ADDMATCHEDITKEY",KeyVal);
							 }
							 SetGlobalValue ("%CORRECTEDADDRESS",CorrectedAddress);
							 GWDReplaceRecord (lpGWDHead,0,IndexArray,Offset);
							 AddToUserDefinedAddress (&ULAddKey,&pAMER->AM);
			    			 GlobalUnlock (hDBDest);
							 CloseGWDatabase (hDBDest); 
							 hDBDest = 0;
							 NumMatched++;
							 if (GetGlobalCVal ("[%ADDMATCHEDITUPDATEMACRO]",AddEditUpdateMacro,0))
								 ProcessText (AddEditUpdateMacro);
					         PostMessage(hWndDlg, WM_COMMAND, IDC_NEXT, 0L);
					    }
					}
					break;
				}
            	 break;
            
            case 65002:
            	 break;
            
            case IDC_LOCMANUAL_QUESTIONABLE:
            	 MC=6; 
            	 goto SetMan;
            case IDC_LOCMANUAL: 
            	 MC=7; 
          SetMan:
				 if (CurType == 11)
					 MC=11;
				 if (!CurrentConfig)
					SetConfig (1);
			     SetViewport (*pCommandViewport);
				 hWndDigControl = hWndDlg; 
				 WantDigWorldCtlPnt = TRUE;
				 if (MC == 11)
					AddGraphicsCmd (CurView->hWnd,"[%C]=$RESET();GF_SET_COORD(1,Locate from point);GF_SET_COORD(1,Locate to point)",FALSE,0); 
				 else if (CurType == 2)
					AddGraphicsCmd (CurView->hWnd,"[%C]=$RESET();GF_SET_COORD(1,Locate intersection point)",FALSE,0); 
				 else
					AddGraphicsCmd (CurView->hWnd,"[%C]=$RESET();GF_SET_COORD(1,Locate address point)",FALSE,0); 
            	 break;
            
            case IDC_SETWORLDCOORD:
 				 if (MC == 11)
				 {
					 UserLocatedFromPoint = DigWorldControlPoint;
					 MC = 13;
					 /*SetViewport (*pCommandViewport);
					 hWndDigControl = hWndDlg; 
					 WantDigWorldCtlPnt = TRUE;
					 AddGraphicsCmd (CurView->hWnd,"GF_SET_COORD(1,Locate to point)",FALSE,0);*/
					 break;
				 }

				 hWndDigControl = 0;  
				 WantDigWorldCtlPnt = FALSE;    
/*				 if ((Choice1=SendDlgItemMessage (hWndDlg,IDC_STREET_LIST1,LB_GETCURSEL,0,0L)) != LB_ERR &&
				 	 (Choice2=SendDlgItemMessage (hWndDlg,IDC_STREET_LIST2,LB_GETCURSEL,0,0L)) != LB_ERR)
				 {
					 NETINTPATHSKEY		NetIntPathsKey; 
					 INTPATHSDATA		IntPathsData;

	             	 SendDlgItemMessage(hWndDlg,IDC_STREET_LIST1,LB_GETTEXT,Choice1,(DWORD)str);
					 lpTAB = _fstrrchr (str,'\t');
					 NetIntPathsKey.Path1 = atol (lpTAB);
	             	 SendDlgItemMessage(hWndDlg,IDC_STREET_LIST2,LB_GETTEXT,Choice2,(DWORD)str);
					 lpTAB = _fstrrchr (str,'\t');
					 NetIntPathsKey.Path2 = atol (lpTAB);
					 if (NetIntPathsKey.Path1 > NetIntPathsKey.Path2)
					 {
					 	long	save = NetIntPathsKey.Path1;
					 	NetIntPathsKey.Path1 = NetIntPathsKey.Path2;
					 	NetIntPathsKey.Path2 = save;
					 }
					 NetIntPathsKey.IntID = -1;
					 IntPathsData.Point = DigWorldControlPoint;
					 _fmemset (IntPathsData.Munics,0,sizeof(IntPathsData.Munics)); 
					 CloseNetIntersect (Opened);
			         if (OpenNetIntersect (NetworkID,TRUE,&Opened))
					 	BT_PUT (hBTNetIntPaths,(LPSTR)&NetIntPathsKey,(LPSTR)&IntPathsData);   
					 CloseNetIntersect (Opened);
					 OpenNetIntersect (NetworkID,FALSE,&Opened);
				 }
				 else*/ 
				 {   
				 	 ADDMATCH	AM; 
				 	 
				 	 _fmemset (&AM,0,sizeof(AM));
				 	 AM.MatchCode=1;
					 AM.LocationCode=MC;
					 AM.Point=DigWorldControlPoint;
					 AddToUserDefinedAddress (&ULAddKey,&AM); 
				 }
	    		 hDBDest = OpenGWDatabase (DestName,BT_WRITE);
	    		 if (!hDBDest)
	    		 	break;
			     lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest); 
	    		 //pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData;
                 pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData[lpGWDHead->pFldInfo->Len];
	    		 if (CurEditRec)
	    		 {   
	    		 	 short	len;
	    		 	 LPSTR	pData;
	    		 	 char	Type;
			        		 	 
	        		 BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&CurEditRec,BT_FIRST,BT_EQ,(LPSTR)&Offset); 
	        		 len = FillGWDData (lpGWDHead,Offset); 
					 pAMER->AM.MatchCode=1;
					 pAMER->AM.LocationCode=MC;
					 if (MC == 13)
					 {
						 pAMER->AM.Point=UserLocatedFromPoint;
						 pAMER->AM.Point2=DigWorldControlPoint;
					 }
					 else
						 pAMER->AM.Point=DigWorldControlPoint;
					 goto UpdateAddEditFile;
			    }
            	 break;
            
            case IDC_STREET_LIST1: 
            case IDC_STREET_LIST2: 
                 switch(HIWORD(wParam))
                 {   
                     COLORREF	Color;
                     
                 	 case LBN_SELCHANGE:
//	                 	 SelMatchOpt = ZOOMTOSTREET;
//	                 	 SetDlgItemText (hWndDlg,IDC_SELECTMATCH,"Zoom To Street");
	                     //break;
                     case LBN_DBLCLK:
			             Choice=(short)SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0);
			             if (Choice == LB_ERR)
			             	break;
			             SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETTEXT,Choice,(DWORD)str);
			             lpTAB = _fstrrchr (str,'\t');
			             lpTAB++;
			             Snum = atol (lpTAB);
						 if (!CurrentConfig)
							SetConfig (1);
			             SetViewport (*pCommandViewport);  
			             if (LOWORD(wParam) == IDC_STREET_LIST1)
			             	Color = GetRoadColor (1); 
			             else
			             	Color = GetRoadColor (2);
						 SetDlgItemText (hWndDlg,IDC_MESSAGE2,"Searching for street"); 
						 NumSegs = DrawStreet2 (Snum,0,Color,3,FALSE,&CurView->WBounds,TRUE);
						 sprintf (str,"%ld segments on screen",NumSegs);
						 SetDlgItemText (hWndDlg,IDC_MESSAGE2,str); 
						 ClearFullWindowBitmap (0);

//						 DrawStreet (Snum,-1,RGB(255,0,0),3,FALSE,NULL,FALSE);
		            	 break; 
		          }
		          break;
            	      
            case IDC_POSSIBLE: 
            {   
                  HMENU EditMenu;             
                  POINT position; 
				  char	type;

				  switch(HIWORD(wParam))
                  {
            	   case LBN_SETFOCUS:
                 	SelMatchOpt = SELMATCH;
                 	SetDlgItemText (hWndDlg,IDC_SELECTMATCH,"Select Match");
                    break;
				   case LBN_DBLCLK :
                   case LBN_SELCHANGE: 
			            Choice=(short)SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0);
			            if (Choice == LB_ERR)
			             	break;
			            SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETTEXT,Choice,(DWORD)str);
			            lpTAB = _fstrrchr (str,'\t');
			            lpTAB++; 
						type = *lpTAB;
			            lpTAB+=11;  
			            p1.x = dread (lpTAB,20);
			            lpTAB+=20;
			            p1.y = dread (lpTAB,20); 
			            lpTAB+=20;
						if (type == 'O')
						{
							lpTAB+=10;  
							p2.x = dread (lpTAB,20);
							lpTAB+=20;
							p2.y = dread (lpTAB,20); 
						}
						else
							p2 = dnewpt (p1,0,3000);
						if (!CurrentConfig)
							SetConfig (1);
                   		SetViewport (*pCommandViewport);
				  		SelectClipRgn (CurView->hDC,NULL); 
						if (HIWORD(wParam) == LBN_SELCHANGE && !PointInWBounds (&p1))
						{
							DestroySavedScreen (&hSaveScreen,0);
							ZoomToPointAndScaleOnlyIfDifferent (OrigMidPointW,OrigScale,TRUE);
						}
						else if (CurView->Scale == OrigScale || !PointInWBounds (&p1))
						{
							DestroySavedScreen (&hSaveScreen,0);
							ZoomToPointAndScaleOnlyIfDifferent (p1,OrigScale/10,TRUE);
						}
						RestoreScreen2 (CurView->hDC, hSaveScreen,0,FALSE);
						DestroySavedScreen (&hSaveScreen,0);
				  		if (type == 'O')
							hSaveScreen = DisplayPointerInVP (2,p1,p2,12,RGB(255,0,0));
						else
							hSaveScreen = DisplayPointerInVP (1,p1,p2,12,RGB(255,0,0));
/*                        EditMenu=CreatePopupMenu();             
                        AppendMenu (EditMenu,MF_ENABLED|MF_STRING,65001,IADDR("Select",1));
                        AppendMenu (EditMenu,MF_ENABLED|MF_STRING,65002,IADDR("Cancel",1));
                        GetCursorPos (&position);
                        TrackPopupMenu (EditMenu,TPM_LEFTBUTTON,position.x,position.y,0,hWndDlg,0);
                        DestroyMenu (EditMenu);*/
						ClearFullWindowBitmap (0);
                        EnableWindow (GetDlgItem(hWndDlg,IDC_SELECTMATCH),TRUE); 
                  }
                  break;
              }
              
            case IDC_STREET_LIST: 
                 switch(HIWORD(wParam))
                 {   
                     
                 	 case LBN_SELCHANGE:
	                 	// SelMatchOpt = ZOOMTOSTREET;
	                 	// SetDlgItemText (hWndDlg,IDC_SELECTMATCH,"Zoom To Street");
						 SetDlgItemText (hWndDlg,IDC_MESSAGE2,""); 
						 if (!CurrentConfig)
							SetConfig (1);
			           	 SetViewport (*pCommandViewport); 
//						 RestoreScreen2 (CurView->hDC, hSaveVPScreen,SaveVPID,FALSE);
//	                     break;
//                     case LBN_DBLCLK: 
                     
                   		 if (!Index)
                   			break; 
                         Choice=(short)SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0);
                         SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETTEXT,Choice,(DWORD)str);
                         lpTAB = _fstrrchr(str,'\t');
                         if (!lpTAB)
                         	break;
                         *lpTAB++=0;   
                         NewNum = atol (lpTAB);   
			           	 Color = GetRoadColor (Choice+1);
						 if (!CurrentConfig)
							SetConfig (1);
			           	 SetViewport (*pCommandViewport); 
						 SetDlgItemText (hWndDlg,IDC_MESSAGE2,"Searching for street"); 
						 NumSegs = DrawStreet2 (NewNum,0,Color,3,FALSE,&CurView->WBounds,TRUE); 
						 sprintf (str,"%ld segments on screen",NumSegs);
						 SetDlgItemText (hWndDlg,IDC_MESSAGE2,str); 
						 GetDlgItemText (hWndDlg,IDC_HOUSE,House,lnHouse);
						 WantHouse = atol (House);
		         		 GetDlgItemText (hWndDlg,IDC_ZIP,str,16);  
         		 		 ZIPCode = atol (str); 
		         		 GetDlgItemText (hWndDlg,IDC_CITY,str,64);  
         		 		 Munic = GetMunicFromName (str); 
			             NumNearHouse = GetNearHouse (NewNum,WantHouse,Munic,ZIPCode,NearHouse,NearPoint);
					     SetBkMode (CurView->hDC,OPAQUE);  
					     SetBkColor (CurView->hDC,RGB(255,255,0));
					     SetTextColor (CurView->hDC,0);
					     {
					     	BOOL SaveDM = DisplayMarkers;
					     	
						     DisplayMarkers = TRUE;
							 if (!CurrentConfig)
								SetConfig (1);
				             for (i=0;i<NumNearHouse;i++)
				             {
								 COLORREF	OldColor;

		                   	 	 SetViewport (*pCommandViewport);
						  		 SelectClipRgn (CurView->hDC,NULL); 
								 /*RestoreScreen2 (CurView->hDC, hSaveScreen,0,FALSE);
								 DestroySavedScreen (&hSaveScreen,0);
						  		 hSaveScreen = DisplayPointerInVP (NearPoint[i],12,RGB(255,0,0));*/   
						  		 sprintf (str,"%ld",NearHouse[i]);
								 OldColor = SetTextColor (CurView->hDC,RGB(228,0,0));
								 DisplayMarker (NearPoint[i],2,str,0.16,0,0,FALSE,FALSE,NULL,NULL,0,0,0);
								 SetTextColor (CurView->hDC,OldColor);
	                         } 
	                         DisplayMarkers =  SaveDM;
	                     }
                         _fstrcpy (OrigName,AMER.Street); 
/*                         nChange = ChangeSimilarStreets (DestName,OrigName,AMER.RecordNum,&hChangeRecs);
                         rc = IDYES;
                         if (nChange)
                         {
                         	char	Mess[128];
                         	
                         	sprintf (Mess,"Change %ld similar cases of '%s' to '%s'?",nChange,OrigName,str);
                         	rc = GSSiMsgBox (hWndDlg,Mess,"Verify Changes",MB_YESNO);
                         } 
                     	 {
                     		LPLONG	pRec;
                     		long	NewMatches=0;
                         		
							SetDlgItemText (hWndDlg,IDC_MESS2,"");
                     		pRec = GlobalLock (hChangeRecs);
	                     	if (rc == IDNO)
	                     	{
	                     		pRec++;
	                     		*pRec--=0;
	                     	}
                     		
						    hDBDest = OpenGWDatabase (DestName,BT_WRITE);
				    		if (!hDBDest)
				    		 	break;
						    lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest); 
			        		pAMER = &lpGWDHead->GWDData;
			        		while (*pRec)
			        		{
								BT_FIND (lpGWDHead->BTHandle[0],pRec++,BT_FIRST,BT_EQ,(LPSTR)&Offset); 
								FillGWDData (lpGWDHead,Offset);  
								_fmemmove (&AMER,pAMER,lpGWDHead->Reclen);
								if (!pAMER->AM.StreetNum)
								{ 
									if (!_fstricmp (pAMER->Street,OrigName))
									{
										_fstrcpy (pAMER->Street,str); 
										pAMER->AM.StreetNum = NewNum;
									}
								}
				 				match = ADD_MATCH (pAMER->Street,pAMER->House,pAMER->City,pAMER->ZIP,MOPT,&hMatch,&StreetNum,&MunicNum); 
								switch (match)
								{  
								case 1:
									NumMatched++; 
//					                PctBox (GetDlgItem(hWndDlg,IDC_STATUS1), TotLen, NumMatched,0);
									NewMatches++;
									pMatch = GlobalLock (hMatch); 
									pAMER->AM = *pMatch;
								   	GlobalUnlock (hMatch);
									GSSiGlobFree (&hMatch);
							 		IndexArray[1]=FALSE;
									break;
								case 0:
								default: 
									pAMER->AM.MatchCode = match; 
									pAMER->AM.StreetNum = StreetNum;
				 					IndexArray[1]=TRUE;
									break; 
								}
						 		len = lpGWDHead->Reclen; 
								GWDReplaceRecord (lpGWDHead,len,IndexArray,Offset);
							}
			    			GlobalUnlock (hDBDest);
							CloseGWDatabase (hDBDest);
							hDBDest = 0;
							GlobalUnlock (hChangeRecs);
							GSSiGlobFree (&hChangeRecs); 
							sprintf (str,"%ld new matches created",NewMatches);
							SetDlgItemText (hWndDlg,IDC_MESS2,str);
					        CurEditRec++;       
					        PostMessage(hWndDlg, WM_COMMAND, IDC_NEXT, 0L);
                     	} */
                     break;
                 }
                 break;  
            case IDC_VIEW_NOHITS:
            	 CurEditRec = 0; 
            	 Index = 1;
		         PostMessage(hWndDlg, WM_COMMAND, IDC_NEXT, 0L); 
		         break;
            	 
            case IDC_VIEW_ALL:
            	 CurEditRec = 0; 
				 LastRecnum = 0; 
            	 Index = 0;
		 		 EnableWindow (GetDlgItem(hWndDlg,IDC_NEXT),TRUE);  
		         PostMessage(hWndDlg, WM_COMMAND, IDC_NEXT, 0L); 
		         break;
            
            case IDC_MATCHTOZIP: 
            {    
            	 DPOINT	ZIPPoint; 
            	 long	ZIPCode;
            	 
         		 GetDlgItemText (hWndDlg,IDC_ZIP,str,16);  
         		 ZIPCode = atol (str);
         		 if (!GetZIPCenter (ZIPCode,&ZIPPoint))
         		 	break;
	    		 hDBDest = OpenGWDatabase (DestName,BT_WRITE);
	    		 if (!hDBDest)
	    		 	break;
			     lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest); 
        		 //pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData;
                 pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData[lpGWDHead->pFldInfo->Len];
        		 if (CurEditRec)
        		 {   
        		 	 short	len;
        		 	 LPSTR	pData;
		        		 	 
	        		 BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&CurEditRec,BT_FIRST,BT_EQ,(LPSTR)&Offset); 
	        		 len = FillGWDData (lpGWDHead,Offset); 
	        		 pData = _fstrchr(str,'\t');
	        		 pData++;
					 pAMER->AM.MatchCode=1;
					 pAMER->AM.LocationCode=9;
					 pAMER->AM.StreetNum=0;
					 pAMER->AM.ZIP=ZIPCode;
					 pAMER->AM.Point = ZIPPoint; 
					 _fstrcpy (pAMER->ZIP,str);
					 goto UpdateAddEditFile;
        		 }
        	}
            	 break;
            
            case IDC_ORIGREC:
            {
				 FARPROC lpfnDISPLAYRECORDMsgProc;
            	 LPSTR	lpEnd, lpKFN=KeyFieldName; 
            	 HANDLE	hDB=0; 
            	 FIELDINFO	FieldInfo;   
            	 short	DBType;
            	 
            	 if (!GetDlgItemText (hWndHidden,IDC_KEY_FIELD,KeyFieldName,66)) 
					strcpy (KeyFieldName,"%RECORDOFFSET");
                 if (!OpenDataFile (IMDataFile,"",BT_READ,&hDB))
                 {
			        GSSiMessageBox(IMDataFile,"Unable to open database", MB_ICONEXCLAMATION,0);
                 	break;
                 }
            	 lpEnd = LastChr (lpKFN);
            	 if (*lpEnd == ']')
            	 	*lpEnd = 0;
            	 if (*lpKFN == '[')
            	 	lpKFN++;
                 _fstrcpy (FieldInfo.name,lpKFN);
                 if (_fstricmp (lpKFN,"%RECORDOFFSET") &&
                 	 (DBType = GetDBType (hDB)) != GMTEXT_DATAFILE)
	            	 GetDBFieldInfo (&FieldInfo,hDB);
			     CloseDataFile (TRUE,&hDB); 
            	 GetDlgItemText (hWndDlg,IDC_KEY_FIELD_VALUE,KeyFieldValue,128); 
            	 if (!_fstricmp (FieldInfo.name,"%RECORDOFFSET")) 
            	 	sprintf (SQL,"%s = %s",FieldInfo.name,KeyFieldValue);
            	 else if (DBType == GMTEXT_DATAFILE)
            	 	sprintf (SQL,"[%s]=='%s'",FieldInfo.name,KeyFieldValue);
            	 else
            	 { 
	            	 if (_fstrchr (lpKFN,' ' ) || *lpKFN == '_') 
	            	 	sprintf (SQL,"\"%s\" = ",lpKFN);
	            	 else	
	            	 	sprintf (SQL,"%s = ",lpKFN);
	                 if (FieldInfo.type == BT_RIGHT_CHAR || FieldInfo.type == BT_CHAR ||
	                 	 FieldInfo.type == SQL_CHAR || FieldInfo.type == SQL_VARCHAR || FieldInfo.type == SQL_TIMESTAMP || FieldInfo.type == SQL_UNKCHAR)
	            	 	sprintf (_fstrchr(SQL,0),"'%s'",KeyFieldValue);	
	            	 else 	 
	            	 	sprintf (_fstrchr(SQL,0),"%s",KeyFieldValue);	
				 }
				 DRSQL = SQL;
				 DRDataFile = IMDataFile;   	
				 lpfnDISPLAYRECORDMsgProc = MakeProcInstance((FARPROC)DISPLAYRECORDMsgProc, hInst);
				 nRc = DialogBox(hInst, (LPSTR)"DISPLAYRECORD", hWndDlg, lpfnDISPLAYRECORDMsgProc);
				 FreeProcInstance(lpfnDISPLAYRECORDMsgProc);
       		}
            	 break;

			case IDC_REPORT:
            	 GetDlgItemText (hWndDlg,IDC_KEY_FIELD_VALUE,KeyFieldValue,128); 
				 SetGlobalValue ("%ORIGFILEKEY",KeyFieldValue);
				 GetGlobalCVal ("[%ADDEDITREPORT]",AddEditReport,0);
				 ProcessText (AddEditReport);
				 break;
            
			case IDC_TESTPLOT:
				{
					//ZoomToPointAndScale (OrigMidPointW,OrigScale,TRUE);
					AddressTestPlot (DestName);
				}
				break;
            	 
            case IDC_NEXT:
            {
            	 unsigned short len;  
            	 LPLONG	pRecnum;
//				 BTHEAD BTHead;  
				 short	st;
				 int	iHouse;
			 	 long 	StreetNum1, StreetNum2, OnStreetNum;
				 LPADDMATCH	pMatch; 
				
				 CurEditRec++;

GetNext:
				 ClearSpecial(); 
				 SetDlgItemText (hWndDlg,IDC_MESSAGE1,"");
				 SetDlgItemText (hWndDlg,IDC_MESSAGE2,"");
				 DBoundsInit (&DisplayBounds);
	    		 hDBDest = OpenGWDatabase (DestName,BT_WRITE);
	    		 if (!hDBDest)
	    		 	break;
			     lpGWDHead = (LPGWDHEADER)GlobalLock (hDBDest); 
        		 //pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData;
                 pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData[lpGWDHead->pFldInfo->Len];
				 SetGlobalValue ("%ADDMATCHEDITFILE",DestName);
//	         	 GetBTHeader (lpGWDHead->BTHandle[Index],&BTHead);
	         	 if (TotRecs < 0)
				 {
					 TotRecs = 0;
					 st = BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)&AMEKey1,BT_FIRST,BT_ANY,(LPSTR)&Offset); 
					 while (!st)
					 {
        		 		 switch (AMEKey1.MatchCode)
						 {
        		 			case 1:
								if (!GetGlobalCVal ("[%ADDMATCHEDITUPDATEMACRO]",AddEditUpdateMacro,0))
								{
			        		 		AMEKey1.MatchCode = 2;
			        		 		AMEKey1.RecNum = 0;
									st = 1;
        		 					break;
								}
        		 			default:
								TotRecs++;
								st = BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)&AMEKey1,BT_NEXT,BT_ANY,(LPSTR)&Offset);
       		 			break;
						 } 
					 }
					 sprintf (str,"%ld remaining",TotRecs);
					 SetDlgItemText (hWndDlg,IDC_REMAINING,str);
				 }

	         	 //	TotRecs = BTHead.BT_NUMRECS;
				 PctBox (GetDlgItem(hWndDlg,IDC_STATUS1),TotRecs , CurIndexRec,0);
				 sprintf (str,"%ld remaining",TotRecs - CurIndexRec + 1);
				 SetDlgItemText (hWndDlg,IDC_REMAINING,str);
/*        		 if (CurEditRec)
        		 {   
        		 	 short	len;
        		 	 
	        		 BT_FIND (lpGWDHead->BTHandle[0],&LastRecnum,BT_FIRST,BT_EQ,(LPSTR)&Offset); 
	        		 len = FillGWDData (lpGWDHead,Offset); 
	        		 if (pAMER->AM.MatchCode != 1)
	        		 {
						 pAMER->AM.MatchCode+=100;
					 	 IndexArray[1]=TRUE;
						 GWDReplaceRecord (lpGWDHead,len,IndexArray,Offset); 
					 }
        		 }*/  
        		 if (Index)    
        		 {  
        		 	CurIndexRec++;
        		 	AMEKey1.MatchCode = NextMatchCode;
        		 	AMEKey1.RecNum = CurEditRec;
        		 	st = BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)&AMEKey1,BT_FIRST,BT_GE,(LPSTR)&Offset); 
        		 	if (!st)
						switch (AMEKey1.MatchCode)
        		 	{
        		 		case 1:
							 if (GetGlobalCVal ("[%ADDMATCHEDITUPDATEMACRO]",AddEditUpdateMacro,0))
							 {
								FillGWDData (lpGWDHead,Offset);
								pRecnum =(LPLONG)lpGWDHead->GWDData;
								pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData[lpGWDHead->pFldInfo->Len];
								SetGlobalValueLong ("%ADDMATCHEDITKEY",*pRecnum);
								CurEditRec = *pRecnum;
								sprintf (CorrectedAddress,"%ld %s",pAMER->House,pAMER->Street);
								SetGlobalValue ("%CORRECTEDADDRESS",CorrectedAddress);
			    				GlobalUnlock (hDBDest);
								CloseGWDatabase (hDBDest); 
								hDBDest = 0;
				       		 	NextMatchCode = AMEKey1.MatchCode;
								ProcessText (AddEditUpdateMacro);
								CurEditRec++;
							    goto GetNext;
							 }
							 else
							 {
		        		 		AMEKey1.MatchCode = 2;
			        		 	AMEKey1.RecNum = 0;
			        		 	st = BT_FIND (lpGWDHead->BTHandle[1],(LPSTR)&AMEKey1,BT_FIRST,BT_GT,(LPSTR)&Offset); 
							 }
							//st = 1;
        		 		break;
        		 		default:
        		 		break;
        		 	} 
        		 	NextMatchCode = AMEKey1.MatchCode;
        		 }
        		 else  
        		 {
        		 	st = BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&CurEditRec,BT_FIRST,BT_GT,(LPSTR)&Offset); 
        		 	CurIndexRec = CurEditRec;
        		 }
//        		 while (n--)
//      		 	BT_FIND (lpGWDHead->BTHandle[1],&IMEKey1,BT_NEXT,BT_ANY,(LPSTR)&Offset);
				 if (st)
				 {
		 			EnableWindow (GetDlgItem(hWndDlg,IDC_NEXT),FALSE);  
        		    GlobalUnlock (hDBDest);
				 	CloseGWDatabase (hDBDest);  
				 	hDBDest = 0;
				 	PctBox (GetDlgItem(hWndDlg,IDC_STATUS1), 100, 100,0);
					SendDlgItemMessage (hWndDlg,IDC_STREET_LIST,LB_RESETCONTENT,0,0);
					SendDlgItemMessage (hWndDlg,IDC_STREET_LIST1,LB_RESETCONTENT,0,0);
					SendDlgItemMessage (hWndDlg,IDC_STREET_LIST2,LB_RESETCONTENT,0,0);
	        		SetDlgItemText (hWndDlg,IDC_STREET,"");
	        		SetDlgItemText (hWndDlg,IDC_HOUSE,"");
	        		SetDlgItemText (hWndDlg,IDC_CITY,"");
	        		SetDlgItemText (hWndDlg,IDC_ZIP,"");
	        		SetDlgItemText (hWndDlg,IDC_KEY_FIELD_VALUE,"");
		 			EnableWindow (GetDlgItem(hWndDlg,IDC_NEXT),FALSE);  
		 			EnableWindow (GetDlgItem(hWndDlg,IDC_ORIGREC),FALSE);  
		 			EnableWindow (GetDlgItem(hWndDlg,IDC_LOCMANUAL),FALSE);  
			        PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
		 			break;
				 } 
        		 FillGWDData (lpGWDHead,Offset);
        		 pRecnum =(LPLONG)lpGWDHead->GWDData;
        		 CurEditRec = *pRecnum;  
	         	 //GetBTHeader (lpGWDHead->BTHandle[Index],&BTHead);
/*	         	 if (Index)
	         	 { 
	        		 sprintf (str,"Record %ld of %ld (Original record %ld) Match code %i",CurEditRec,BTHead.BT_NUMRECS,*pRecnum,pAMER->AM.MatchCode); 
	        	 }  
	        	 else
	        		 sprintf (str,"Record %ld of %ld - Match code %i",*pRecnum,BTHead.BT_NUMRECS,pAMER->AM.MatchCode); 
				 SetDlgItemText (hWndDlg,IDC_MESS1,str);  */
	         	 LastRecnum++;
        		 _fmemmove (&AMER,pAMER,sizeof(AMER)); 
        		 if (pAMER->AM.StreetNum)
        		 {
        		 	Skip=TRUE; 
					GetTrueStreetName (pAMER->AM.StreetNum, TrueName, 0,0); 
					SendDlgItemMessage (hWndDlg,IDC_STREET_LIST,LB_RESETCONTENT,0,0);
				    SendDlgItemMessage (hWndDlg,IDC_STREET_LIST,LB_ADDSTRING,0,(LPARAM)TrueName);
        		 }
//            	 SendDlgItemMessage (hWndDlg,IDC_MATCHED,BM_SETCHECK,pAMER->AM.StreetNum,0L); 
            	 ShowMatches = FALSE; 
        		 SetDlgItemText (hWndDlg,IDC_STREET,pAMER->Street);
        		 SetDlgItemText (hWndDlg,IDC_HOUSE,pAMER->House);
        		 SetDlgItemText (hWndDlg,IDC_CITY,pAMER->City);
        		 SetDlgItemText (hWndDlg,IDC_ZIP,pAMER->ZIP);
                 GMDGetCharFieldVal (lpGWDHead,0,str); 
        		 SetDlgItemText (hWndDlg,IDC_KEY_FIELD_VALUE,str);
        		 DisplayOnlyAddressesInCurrentBounds = FALSE;
        		 if (*pAMER->ZIP)
        		 {  
        		 	MNMXCORD	Bounds;
        		 	long	ZIP = atol (pAMER->ZIP);
        		 	
        		 	EnableWindow (GetDlgItem(hWndDlg,IDC_MATCHTOZIP),TRUE);
        		 	if (GetZIPBounds (ZIP,&Bounds)) 
        		 	{
						if (!CurrentConfig)
							SetConfig (1);
		        		SetViewport (*pCommandViewport);
		        		CurView->CurZoomAreaRef = ZIP;
		        		_fstrcpy (CurView->CurVisibilityID,"ZIPCODE"); 
		        		DisplayOnlyAddressesInCurrentBounds = TRUE;
        		 		ZoomToRect(Bounds,TRUE);
						DestroySavedScreen (&hSaveVPScreen,SaveVPID);
						hSaveVPScreen = SaveScreen2 (CurView->hWnd,CurView->hDC, CurView->Rect,CurView,&SaveVPID);
        		 	}
        		 }
        		 else
        		 	EnableWindow (GetDlgItem(hWndDlg,IDC_MATCHTOZIP),FALSE);
        		 if (pAMER->AM.MatchCode == 1)
        		 {
	    			SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_RESETCONTENT,0,0);
                    EnableWindow (GetDlgItem(hWndDlg,IDC_SELECTMATCH),FALSE); 
					GetTrueStreetName (pAMER->AM.StreetNum, TrueName, 0,0); 
					sprintf (str,"%ld %s %ld", 
								 pAMER->AM.HouseNum,TrueName,pAMER->AM.ZIP);
                    SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_ADDSTRING,0,(LPARAM)str);
					ShowMatches = FALSE;
        		 } 
        		 else
					 ShowMatches = TRUE; 
				 if (!SeparateOnFromToStreets (pAMER->Street,OnStreet,Street1,Street2) && SeparateIntStreets (pAMER->Street,Street1,Street2))
		 	 	 {
					_fstrcpy (pAMER->Street,Street1);
					Truncate (pAMER->Street);
					_fstrcat (pAMER->Street,"/");
					_fstrcat (pAMER->Street,Street2);
					Truncate (pAMER->Street);
		 	 	 }
				 _fstrncpy (ULAddKey.Street,pAMER->Street,sizeof(ULAddKey.Street));
				 ULAddKey.HouseNum = atol (pAMER->House);
				 ULAddKey.Munic = pAMER->AM.Munic;
        		 GlobalUnlock (hDBDest);
				 CloseGWDatabase (hDBDest); 
				 hDBDest = 0;
        		 GetDlgItemText (hWndDlg,IDC_STREET,Street,lnStreet);
        		 GetDlgItemText (hWndDlg,IDC_HOUSE,House,lnHouse);
        		 GetDlgItemText (hWndDlg,IDC_CITY,City,lnCity);
        		 GetDlgItemText (hWndDlg,IDC_ZIP,ZIP,lnZIP);   
        		 SetGlobalValue ("%ADDEDITCITY",City);
        		 FirstAttempt = TRUE;
     ShowMatched: 
        		 GetDlgItemText (hWndDlg,IDC_STREET,Street,lnStreet);
        		 GetDlgItemText (hWndDlg,IDC_HOUSE,House,lnHouse);
				 sprintf (CorrectedAddress,"%s %s",House,Street);
				 OneSpace (CorrectedAddress);
     			 if (!ShowMatches)
     			 	break;
    			 SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_RESETCONTENT,0,0);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SELECTMATCH),FALSE); 
				 iHouse = atoi (House);
        		 GetDlgItemText (hWndDlg,IDC_CITY,City,lnCity);
        		 GetDlgItemText (hWndDlg,IDC_ZIP,ZIP,lnZIP);
        		 OneMatch = FALSE; 
				 if (!CurrentConfig)
					SetConfig (1);
        		 SetViewport (*pCommandViewport); 
				 //DBoundsInit (&Bounds);
				 MunicNum = GetMunicFromName (City); 
				 if (FoundUserAssignedAddress (iHouse,Street,MunicNum,FALSE,&hMatch))
				 {
					pMatch = (LPADDMATCH)GlobalLock (hMatch);
					if (pMatch->LocationCode == 6)
			 			sprintf (str,"User defined location (questionable)\tI%10ld%20.8f%20.8f",0,pMatch->Point.x,pMatch->Point.y);
					else
			 			sprintf (str,"User defined location\tI%10ld%20.8f%20.8f",0,pMatch->Point.x,pMatch->Point.y);
					item=SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_ADDSTRING,0,(LPARAM)str);  
					OneMatch = TRUE;
					GlobalUnlock (hMatch);
				 }
        		 else if (*House ||!*Street)
        		 {
					 BOOL	UsePointBased=FALSE,UseNetBased=TRUE;
					 
					 CurType = 1;
					 GSSiGlobFree (&hMatch); 
					 match = ADD_MATCH (Street,House,City,ZIP,3,&hMatch,&StreetNum,&MunicNum,UsePointBased,UseNetBased,hWndDlg,3,EntryControls); 
					 if (hMatch)
					 	pMatch = (LPADDMATCH)GlobalLock (hMatch);
					 else
					 	pMatch = NULL;   
					 if (match == 1)
					 	OneMatch = TRUE;
					 for (imatch=0;imatch<match;imatch++)
					 {  	
					    if (OneMatch || !DisplayOnlyAddressesInCurrentBounds || PointInWBounds (&pMatch->Point))
					    {   
					    	char	dlm[3]=", ";
					    	
							GetTrueStreetName (pMatch->StreetNum, TrueName, 0,0); 
							GetMunicName (pMatch->Munic,MunName,MunAbv); 
							if (!*MunName)
								*dlm = 0;
							if (pMatch->LocationCode)
								AddDPointToMinMax (&pMatch->Point,&DisplayBounds);
							sprintf (str,"%ld %s%s%s %ld\tA%10ld%20.8f%20.8f", 
										 pMatch->HouseNum,TrueName,dlm,MunName,pMatch->ZIP,
										 pMatch->StreetNum,pMatch->Point.x,pMatch->Point.y);
		                    item=SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_ADDSTRING,0,(LPARAM)str);  
		                    SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_SETITEMDATA,item,(LPARAM)imatch);  
	                    }
	                    pMatch++;
					 } 
			 		 if (hMatch)
			 		 	GlobalUnlock (hMatch);  
			 	 }
			 	 else if (SeparateOnFromToStreets (Street,OnStreet,Street1,Street2))
				 {
					 HANDLE hMatch1, hMatch2;
					 int	NumMatch1, NumMatch2;

					 CurType = 11;
				     GSSiGlobFree (&hMatch); 
					 match = OFT_MATCH (OnStreet,Street1,Street2,City,0,3,&hMatch,&OnStreetNum,&StreetNum1,&StreetNum2,&MunicNum,&NumMatch1,&NumMatch2,&hMatch1,&hMatch2);
					 if (hMatch)
					 	pMatch = (LPADDMATCH)GlobalLock (hMatch);
					 else
					 	pMatch = NULL;   
					 if (match == 1)
					 	OneMatch = TRUE;
					 for (imatch=0;imatch<match;imatch++)
					 {  	
					    if (OneMatch || !DisplayOnlyAddressesInCurrentBounds || PointInWBounds (&pMatch->Point))
					    {
							strcpy (str,"On ");
							GetTrueStreetName (pMatch->OnStreetNum,_fstrchr(str,0), 0,0); 
							_fstrcat (str," From ");
							GetTrueStreetName (pMatch->StreetNum1, _fstrchr(str,0), 0,0);
							_fstrcat (str," To ");
							GetTrueStreetName (pMatch->StreetNum2, _fstrchr(str,0), 0,0);
							GetMunicName (pMatch->Munic,MunName,MunAbv); 
							if (*MunName)
								sprintf (_fstrchr(str,0)," in %s",MunName);
							if (pMatch->LocationCode)
							{
								AddDPointToMinMax (&pMatch->Point,&DisplayBounds);
								AddDPointToMinMax (&pMatch->Point2,&DisplayBounds);
							}
							sprintf (_fstrchr(str,0),"\tO%10ld%20.8f%20.8f%10ld%20.8f%20.8f", 
									 pMatch->IntID,pMatch->Point.x,pMatch->Point.y,pMatch->IntIDTo,pMatch->Point2.x,pMatch->Point2.y);
			                item=SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_ADDSTRING,0,(LPARAM)str);
			                SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_SETITEMDATA,item,(LPARAM)imatch);  
	                    }
	                    pMatch++;
					 } 
			 		 if (hMatch)
			 		 	GlobalUnlock (hMatch);
					 if (!OneMatch)
					 {
						 if (NumMatch1 && NumMatch2)
						 {
			 				 LPADDMATCH	pMatch1=GlobalLock (hMatch1);
		 					 LPADDMATCH	pMatch2;
							 LPSTR		str1;
							 int		imatch1, imatch2;

							 for (imatch1=0; imatch1<NumMatch1; imatch1++,pMatch1++)
							 {
								strcpy (str,"From ");
								GetTrueStreetName (pMatch1->StreetNum1,_fstrchr(str,0), 0,0); 
								_fstrcat (str," and ");
								GetTrueStreetName (pMatch1->StreetNum2, _fstrchr(str,0), 0,0);
								AddDPointToMinMax (&pMatch1->Point,&DisplayBounds);
								str1 = strchr (str,0);
								pMatch2 = GlobalLock (hMatch2);
								for (imatch2=0; imatch2<NumMatch2; imatch2++,pMatch2++)
								{
									strcpy (str1,"To ");
									GetTrueStreetName (pMatch2->StreetNum1,_fstrchr(str,0), 0,0); 
									_fstrcat (str," and ");
									GetTrueStreetName (pMatch2->StreetNum2, _fstrchr(str,0), 0,0);
									AddDPointToMinMax (&pMatch2->Point,&DisplayBounds);
									sprintf (_fstrchr(str,0),"\tO%10ld%20.8f%20.8f%10ld%20.8f%20.8f", 
											 pMatch1->IntID,pMatch1->Point.x,pMatch1->Point.y,pMatch2->IntID,pMatch2->Point.x,pMatch2->Point.y);
									item=SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_ADDSTRING,0,(LPARAM)str);
									SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_SETITEMDATA,item,(LPARAM)imatch);  
								 }
								 GlobalUnlock (hMatch2);

							 }
							 GlobalUnlock (hMatch1);
						 }
						 else
						 {
							 if (NumMatch1)
							 {
			 					 LPADDMATCH	pMatch=GlobalLock (hMatch1);

								 for (imatch=0; imatch<NumMatch1; imatch++,pMatch++)
								 {
									strcpy (str,"From ");
									GetTrueStreetName (pMatch->StreetNum1,_fstrchr(str,0), 0,0); 
									_fstrcat (str," and ");
									GetTrueStreetName (pMatch->StreetNum2, _fstrchr(str,0), 0,0);
									AddDPointToMinMax (&pMatch->Point,&DisplayBounds);
									sprintf (_fstrchr(str,0),"\tI%10ld%20.8f%20.8f", 
											 pMatch->IntID,pMatch->Point.x,pMatch->Point.y);
									item=SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_ADDSTRING,0,(LPARAM)str);
									SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_SETITEMDATA,item,(LPARAM)imatch);  
								 }
								 GlobalUnlock (hMatch1);
							 }
							 if (NumMatch2)
							 {
			 					 LPADDMATCH	pMatch=GlobalLock (hMatch2);

								 for (imatch=0; imatch<NumMatch2; imatch++,pMatch++)
								 {
									strcpy (str,"To ");
									GetTrueStreetName (pMatch->StreetNum1,_fstrchr(str,0), 0,0); 
									_fstrcat (str," and ");
									GetTrueStreetName (pMatch->StreetNum2, _fstrchr(str,0), 0,0);
									AddDPointToMinMax (&pMatch->Point,&DisplayBounds);
									sprintf (_fstrchr(str,0),"\tI%10ld%20.8f%20.8f", 
											 pMatch->IntID,pMatch->Point.x,pMatch->Point.y);
									item=SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_ADDSTRING,0,(LPARAM)str);
									SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_SETITEMDATA,item,(LPARAM)imatch);  
								 }
								 GlobalUnlock (hMatch2);
							 }
						 }
					}
				 }
				 else
			 	 {   
					 
			 	 	 if (SeparateIntStreets (Street,Street1,Street2))
			 	 	 {
						 CurType = 2;
						 GSSiGlobFree (&hMatch); 
						 match = INT_MATCH (Street1,Street2,City,0,3,&hMatch,&StreetNum1,&StreetNum2,&MunicNum); 
						 if (hMatch)
						 	pMatch = (LPADDMATCH)GlobalLock (hMatch);
						 else
						 	pMatch = NULL;
						 if (match == 1)
						 	OneMatch = TRUE;
						 for (imatch=0;imatch<match;imatch++)
						 {  
						    if (OneMatch || PointInWBounds (&pMatch->Point))
						    {
								GetTrueStreetName (pMatch->StreetNum1, str, 0,0); 
								_fstrcat (str," at ");
								GetTrueStreetName (pMatch->StreetNum2, _fstrchr(str,0), 0,0);
								GetMunicName (pMatch->Munic,MunName,MunAbv); 
								if (*MunName)
									sprintf (_fstrchr(str,0)," in %s",MunName);
								if (pMatch->LocationCode)
									AddDPointToMinMax (&pMatch->Point,&DisplayBounds);
								sprintf (_fstrchr(str,0),"\tI%10ld%20.8f%20.8f", 
										 pMatch->IntID,pMatch->Point.x,pMatch->Point.y);
			                    item=SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_ADDSTRING,0,(LPARAM)str);
			                    SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_SETITEMDATA,item,(LPARAM)imatch);  
			                }
		                    pMatch++;
						 } 
				 		 if (hMatch)
				 		 	GlobalUnlock (hMatch);  
			 	     } 
			 	 }
			 	 ZIPCode = atol (ZIP);
				 sprintf (MapQuestAdd,"%s %s",Street,City);
				 if (UseMapQuest)
				 {
					 if (!AddMapQuestMatch (hWndDlg,IDC_POSSIBLE,MapQuestAdd))
						 SetDlgItemText (hWndDlg,IDC_MESSAGE1,"No MapQuest match at street or address level");
				 }
			 	 AddZipCenterMatch (hWndDlg,IDC_POSSIBLE,ZIPCode);
			 	 AddPrecinctCenterMatch (hWndDlg,IDC_POSSIBLE,ZIPCode);
			 	 if (OneMatch)
			 	 {
					 SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_SETCURSEL,0,0);		  
		 			 EnableWindow (GetDlgItem(hWndDlg,IDC_SELECTMATCH),TRUE);  
			     }
			     else
			     {  
			     	DPOINT	UnmatchablePoint;
			     	
			     	if (GetGlobalPVal ("[%BADCOORDPOINT]",NULL,&UnmatchablePoint))
			     	{
						sprintf (str,"Mark as Un-Matchable\tU%10ld%20.8f%20.8f", 
								 0L,UnmatchablePoint.x,UnmatchablePoint.y);
	                    item=SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_ADDSTRING,0,(LPARAM)str);
	                    SendDlgItemMessage (hWndDlg,IDC_POSSIBLE,LB_SETITEMDATA,item,(LPARAM)-1);  
	                }

					if (FirstAttempt)
       		 	    	ProcessGlobal ("[%ADDEDITCITYZOOM]");
			 	 	FirstAttempt = FALSE; 
			 	 }
				 HaltMapDisplay (FALSE,FALSE);
				 if (ValidBounds2 (&DisplayBounds))
				 {
					 InflateBounds (&DisplayBounds,50);
					 {
						double Scale = max ((DisplayBounds.xmx - DisplayBounds.xmn) / RECTWIDTH (&CurView->ScreenRect),(DisplayBounds.ymx - DisplayBounds.ymn) / RECTHEIGHT (&CurView->ScreenRect));
						DPOINT	MidP = MinMaxMidPointD (&DisplayBounds);

						ZoomToPointAndScaleOnlyIfDifferent (MidP,max (Scale,OrigScale/10),TRUE);
					 }
				 }
				 else
				 {
					 ZoomToPointAndScaleOnlyIfDifferent (OrigMidPointW,OrigScale,TRUE);
				 }
				 ClearFullWindowBitmap (0);
        	}
			GSSiGlobFree (&hMatch); 
            break; 
            	 
          }
          break;

    default: 
    	GSSiGlobUlFree (&hMem);
        return FALSE;
   }
 GSSiGlobUlFree (&hMem);
 return TRUE;    
} 
BOOL FAR PASCAL LOC_STREETMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	static	char	Street[66]="";   
	short	i;
    int		TabStops[2]={2000,2100};
    int     TabStops2[2]={150,2100};
    static	BOOL	First=TRUE;
	static RECT		WindRect;
   	short	Choice;  
   	char	str[128]; 
   	BOOL	HighlightStreet; 
   	static	MNMXCORD	TotMinMax;
   	
   	static	int	DisplayedStreets=0;  
	
 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
         /* initialize working variables  */ 
         DBoundsInit (&TotMinMax);
         itoa (Trigger,str,10);   
         SetDlgItemText (hWndDlg,IDC_TRIGGER,str);
         DisplayedStreets=0;
                                     
		 if (!First)
		 {  
			SetWindowPos(hWndDlg,HWND_TOP,WindRect.left,WindRect.top,
									   WindRect.right-WindRect.left,
									   WindRect.bottom-WindRect.top,SWP_NOZORDER);
		 } 
		 EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
		 First = FALSE; 
	     if (!OpenStreetNameTable(FALSE))  
	     {
            GSSiMsgBox( GetFocus(), "Unable to open street name table", NULL,MB_OK|MB_ICONEXCLAMATION,0);
	        return FALSE;
	     }
         SendDlgItemMessage (hWndDlg,IDM_STREET_MENU,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
         SendDlgItemMessage (hWndDlg,IDC_CURSTREETS,LB_SETTABSTOPS,2,(LPARAM)&TabStops2);
         SetDlgItemText(hWndDlg,IDC_STREET,Street);
		 hWndLocStreet=hWndDlg;         
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */
	case WM_DESTROY:
		DisplayCurStreets (TRUE,0);
		break;
    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
           	case IDC_SHOWEDITSTREETS:
                AddToCurStreets (hWndDlg,EditPath1,&TotMinMax,&DisplayedStreets);
                AddToCurStreets (hWndDlg,EditPath2,&TotMinMax,&DisplayedStreets);
           		break;
           		
            case IDC_TRIGGER:
            	GetDlgItemText (hWndDlg,IDC_TRIGGER,str,2);
            	Trigger = atoi (str);
            	break;
            	     
            case IDC_STREET: /* Edit Control                            */
                 switch (HIWORD(wParam))
                 {  case EN_CHANGE:
                        i = GetDlgItemText (hWndDlg,IDC_STREET,Street,33);
                        if (DisplayStreetsINT (hWndDlg,IDM_STREET_MENU,Street,i,IDC_STREET,1,TRUE)>=1)
							EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
						else
							EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
                        break;
                 }
                 break;

            case IDC_CURSTREETS: 
                 switch(HIWORD(wParam))
                 {   
                 	LPSTR	lpTAB;
                 	
                     case LBN_DBLCLK:
                     case LBN_SELCHANGE:
                         Choice=(short)SendDlgItemMessage(hWndDlg,IDC_CURSTREETS,LB_GETCURSEL,0,0);
                         SendDlgItemMessage(hWndDlg,IDC_CURSTREETS,LB_GETTEXT,Choice,(DWORD)str);
                         lpTAB = _fstrchr (str,'\t');
                         *lpTAB = 0;
                         SetDlgItemText (hWndDlg,IDC_STREET,str);
                         break;
                 }
                 break;

            case IDM_STREET_MENU: /* List box                           */
                 switch(HIWORD(wParam))
                 {   
                 	LPSTR	lpTAB;
           		 	long	nSegs;
                 	
                     case LBN_DBLCLK:
                     case LBN_SELCHANGE:
                         Choice=(short)SendDlgItemMessage(hWndDlg,IDM_STREET_MENU,LB_GETCURSEL,0,0);
                 
                         if (SendDlgItemMessage(hWndDlg,IDM_STREET_MENU,LB_GETTEXT,Choice,(DWORD)str) == LB_ERR)
                         	break;
                         lpTAB = _fstrchr (str,'\t');
                         lpTAB++;
                         CurPath = atol (lpTAB); 
                         nSegs = AddToCurStreets (hWndDlg,CurPath,&TotMinMax,&DisplayedStreets);
						 SetConfig (1);
						 SetViewport(*pCommandViewport);
						 DisplayCurStreets (FALSE,0);
          				 EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE); 
         				 EnableWindow (GetDlgItem(hWndDlg,IDC_ZOOM_LIMITS),TRUE); 
                    break;
                 }
                 break;

            case IDOK: 
				 SetConfig (1);
				 SetViewport(*pCommandViewport);
            	 DisplayCurStreets (FALSE,15);
            	 break;

			case IDC_ZOOM_LIMITS:
           		CurView->CurZoomAreaRef = 0;
                ZoomToRect (TotMinMax,FALSE); 
				break;
            	 
            case IDC_CLEAR:
                 if (SendDlgItemMessage (hWndDlg,IDC_HIGHLIGHT,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L))
                 	ClearHighlightList (FALSE);
         		 EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE); 
         		 EnableWindow (GetDlgItem(hWndDlg,IDC_ZOOM_LIMITS),FALSE); 
				 ClearSpecial(); 
         		 SendDlgItemMessage (hWndDlg,IDC_CURSTREETS,LB_RESETCONTENT,0,0);
 				 DisplayedStreets=0;
 				 CurPath = 0;  
				 DBoundsInit (&TotMinMax);
				 DisplayCurStreets (TRUE,0);
                 PostMessage(hWndMain, WM_COMMAND, IDM_Z_REDRAW, 0L);
                 break;

            case IDCANCEL:
				 CloseStreetNameTable (); 
				 ClearSpecial();
				 GetWindowRect (hWndDlg,&WindRect);
                 DestroyWindow (hWndDlg);
				 hWndLocStreet=0;
                 break;
           }
    default:
        return FALSE;
   }
 return TRUE;
}

BOOL FAR PASCAL ADDLOC_FROMADDMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1382);
#endif
{   int        TabStops[2]={100,1300}, i;
    LPSTR   pPrefix, lpDot, lpMIDstr, pSQL;  
    HCURSOR OldCursor=0;    
    static   HANDLE hSQL=0;
    static	short rtn=0;
    LPGWFLDINFO lpGWFldInfo;
    LPGWDHEADER lpGWDHead;
    HANDLE      hBT;    
    float       size=(float)12.0,rot=(float)0.0;
    long        Offset; 
    double      rtnx;
    LPVOID      lpVal; 
    char        str1[16], str2[64];
    LPSTR		str; 
    HANDLE		hStr=0;
    short         st, len,ifield;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
    LPFIELDINFO lpFieldInfo; 
    HANDLE      SaveHandle;
    BOOL        More, Err;
    short       rc, AutoEdit; 
    UINT        FieldLists[3]={IDC_KEY_FIELD,IDC_XFIELD,IDC_YFIELD},
                FieldListTypes[3]={TRUE,TRUE,TRUE}; 
    HFILE		FidSave;
    LPOFSTRUCT	pOFStruct;
    char	Ext[6]=".NL1";
    LPSTR	Street, HouseNum, City, ZIP, BadNames, OrigStreet;
    static	BOOL	RecalledName;
    LPSTR	Name;
	long 	StreetNum1, StreetNum2;
	LPSTR	Street1, Street2, OnStreet;  
	static	BOOL	FileIsOpen=FALSE, AutoRun=FALSE, CreateNewFile=TRUE;
	static	short	UseNetBased=1, UsePointBased=0, SelectOne=0, AddTol=0;

 short    BRtn;
 if (Message == 273 && wParam == 1072)
	 BRtn=1;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam, 0,
                     SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, FieldLists,0,
                     IMDataFile, &IMDataFileType, &hSQL,FieldListTypes,TRUE))  goto RtnTrue;  
 hStr = GSSiGlobAlloc ( 120,GHND,256+256+256+256*4+256+256+256+256+sizeof(OFSTRUCT));
 str = GlobalLock (hStr);
 Street = str + 256;
 HouseNum = Street + 256;
 City = HouseNum + 256;
 ZIP = City + 256;
 BadNames = ZIP + 256;
 Name = BadNames + 256;
 Street1 = Name + 256;
 Street2 = Street1 + 256;
 OrigStreet = Street2 + 256; 
 OnStreet = OrigStreet + 256;
 *Street1 = 0;
 *Street2 = 0;  
 pOFStruct = (LPOFSTRUCT)(Street2 + 256);
 switch(Message)
   {
    case WM_INITDIALOG: 
    	 DoPaint = FALSE;
    	 RecalledName=FALSE;
    	 hWndHidden=hWndDlg; 
    	 hWndAddMatch = hWndDlg;
    	 TotAddLen = 0;
    	 NumMatched = 0;
         SendDlgItemMessage (hWndDlg,IDC_MOPT,CB_ADDSTRING,0,(LPARAM)"None");
         SendDlgItemMessage (hWndDlg,IDC_MOPT,CB_ADDSTRING,0,(LPARAM)"Minor");
         SendDlgItemMessage (hWndDlg,IDC_MOPT,CB_ADDSTRING,0,(LPARAM)"Major");
    	 SetDlgItemText (hWndDlg,IDC_SQL,"ALL ROWS");
         SendDlgItemMessage (hWndDlg,IDC_AUTOSAVE,BM_SETCHECK,TRUE,0L);
		 if (!BackgroundTask)
			 ShowWindow (hWndDlg,SW_SHOW);
         if (*AutoExportName)
		 	PostMessage(hWndDlg, WM_COMMAND, IDC_RECALL, 0L);
    case GSSI_REINITDIALOG:
         SendDlgItemMessage (hWndDlg,IDC_USENETWORK_BASED,BM_SETCHECK,UseNetBased,0L);
         SendDlgItemMessage (hWndDlg,IDC_USEPOINT_BASED,BM_SETCHECK,UsePointBased,0L);
         SendDlgItemMessage (hWndDlg,IDC_SELECT_ONE,BM_SETCHECK,SelectOne,0L);
 		 SendDlgItemMessage (hWndDlg,IDC_MOPT,CB_SETCURSEL,(WPARAM)(MOPT-1),(LPARAM)NULL); 
    	 if (TotAddLen)
	         PostMessage(hWndDlg, WM_COMMAND, IDC_SHOW, 0L);
         if (FileIsOpen && *AutoExportName && AutoRun)
	         PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
         break; /* End of WM_INITDIALOG                                 */
    
    case WM_SHOWWINDOW:
         if (!*AutoExportName)
         	PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
         GSSiGlobUlFree (&hStr);
    	 goto RtnFalse; 
    case WM_DESTROY:
    	 hWndAddMatch = 0;
    	 break;	 
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

         switch(LOWORD(wParam))

         {  
			case IDC_SHOW:
		      	 PctBox (GetDlgItem(hWndDlg,IDC_STATUS1), TotAddLen, NumMatched,0);   
		      	 SetGlobalValueLong ("%ADDMATCHATTEMPT",TotAddLen); 
		      	 SetGlobalValueLong ("%ADDMATCHMATCHED",NumMatched); 
			     break;

            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 if (Processing)  
                 {
                    ContinueProcessing=FALSE;
	                SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,"Canceled");
				 }                    
                 else
        			 PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
                 break;
            
            case IDC_EXIT: 
                 if (!RecalledName)
                 	goto NoUpdate; 
                 if (*AutoExportName || !SendDlgItemMessage(hWndDlg,IDC_AUTOSAVE,BM_GETCHECK,0,0L)) 
                 	goto NoUpdate; 
            //     	_fstrcpy (Name,AutoExportName);
                 else
                 	GetCurVal (Name,128,IDS_FILENL1);
               	 goto Update;
            case IDC_SAVE:
            {    
            	 short	Version=1;
            	 
                 if (!GetSaveName2 (hWndDlg,Name,0,Ext,IDS_FILENL1)) break; 
        Update:
				 Version=2;
                 FidSave = GSSiOpenFile (Name,pOFStruct,OF_CREATE);
                 BigWrite (FidSave,Ext,6,-1);
                 BigWrite (FidSave,(HPSTR)&Version,2,-1);   
                 GetDlgItemText (hWndDlg,SV_DATABASE_LIST,IMDataFile,256);
				 BigWrite (FidSave,IMDataFile,256/*lnIMDataFile*/,-1);
                 GetDlgItemText (hWndDlg,IDC_DEST_FILE,DestName,256);
                 BigWrite (FidSave,(HPSTR)DestName,256/*lnDestName*/,-1);
                 GetDlgItemText (hWndDlg,IDC_STREET,Street,256);
                 BigWrite (FidSave,Street,256,-1);
                 GetDlgItemText (hWndDlg,IDC_HOUSE_NUM,HouseNum,256); 
                 BigWrite (FidSave,(HPSTR)HouseNum,256,-1);
                 GetDlgItemText (hWndDlg,IDC_CITY,City,128); 
                 BigWrite (FidSave,(HPSTR)City,128,-1);
                 GetDlgItemText (hWndDlg,IDC_ZIP,ZIP,128); 
                 BigWrite (FidSave,(HPSTR)ZIP,128,-1);
                 GetDlgItemText (hWndDlg,IDC_SQL,(LPSTR)str,256);                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_KEY_FIELD,(LPSTR)str,256);                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_SYMBOL,(LPSTR)str,256);                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_FROMDATE,(LPSTR)str,256);                
                 BigWrite (FidSave,(HPSTR)str,256,-1);
                 GetDlgItemText (hWndDlg,IDC_TODATE,(LPSTR)str,254);                
                 BigWrite (FidSave,(HPSTR)str,254,-1);
                 GetDlgItemText (hWndDlg,IDC_NOTE,(LPSTR)str,254);                
                 BigWrite (FidSave,(HPSTR)str,254,-1);
                 AutoEdit = SendDlgItemMessage (hWndDlg,IDC_AUTOEDIT,BM_GETCHECK,0,0L);
                 BigWrite (FidSave,(HPSTR)&AutoEdit,2,-1);
			     MOPT=SendDlgItemMessage(hWndDlg,IDC_MOPT,CB_GETCURSEL,0,0)+1; 
                 BigWrite (FidSave,(HPSTR)&MOPT,2,-1);
			     UseNetBased=SendDlgItemMessage(hWndDlg,IDC_USENETWORK_BASED,BM_GETCHECK,0,0L);
                 BigWrite (FidSave,(HPSTR)&UseNetBased,2,-1);
			     UsePointBased=SendDlgItemMessage(hWndDlg,IDC_USEPOINT_BASED,BM_GETCHECK,0,0L);
                 BigWrite (FidSave,(HPSTR)&UsePointBased,2,-1);
			     SelectOne=SendDlgItemMessage(hWndDlg,IDC_SELECT_ONE,BM_GETCHECK,0,0L);
                 BigWrite (FidSave,(HPSTR)&SelectOne,2,-1);
				 AddTol = GetDlgItemInt (hWndDlg,IDC_ADDTOL,&Err,FALSE);
                 BigWrite (FidSave,(HPSTR)&AddTol,2,-1);
                 BigWrite (FidSave,(HPSTR)&TotAddLen,4,-1);
                 BigWrite (FidSave,(HPSTR)&NumMatched,4,-1);
                 CreateNewFile = SendDlgItemMessage (hWndDlg,IDC_NEW,BM_GETCHECK,0,0L);
                 BigWrite (FidSave,(HPSTR)&CreateNewFile,4,-1);
                 GSSiClose (FidSave);
            } 
        NoUpdate:   
            	 if (LOWORD(wParam) == IDC_EXIT)
            	 {
	                 CloseDataFile (TRUE, &hSQL);  
					 DestroyFieldList ();
			    	 DoPaint = TRUE;
			    	 if (*AutoExportName)
			            EndDialog(hWndDlg, rtn);  
			    	 else
	                 	DestroyWindow (hWndDlg);
                 }
                    
            	 break;
            	 
           	case IDC_RECALL: 
           	{
           		 short Version;
           		 
             	 if (!*AutoExportName)
             	 { 
					 if (!GetFileName2 (hWndDlg,str,Ext,IDS_FILENL1))
					 	break;
	             }
	             else
                 {  
                 	LPSTR pComma = _fstrrchr (AutoExportName,',');
                 	
                 	if (pComma) 
                 	{
                 		*pComma++ = 0;
                 		AutoRun = atob (pComma);
                 	}
                 	else
                 		AutoRun = TRUE;
                 	_fstrcpy (str,AutoExportName);
                 }
				 RecalledName=TRUE;
                 FidSave = GSSiOpenFile (str,pOFStruct,OF_READ);
                 BigRead (FidSave,Ext,6);
                 BigRead (FidSave,(HPSTR)&Version,2); 
				 switch (Version)
				 {
				 default:
					 BigRead (FidSave,(HPSTR)IMDataFile,128);//lnIMDataFile);
					 BigRead (FidSave,(HPSTR)DestName,128);//lnDestName);
					 SetDlgItemText (hWndDlg,IDC_DEST_FILE,DestName);
					 BigRead (FidSave,(HPSTR)Street,256);
					 SetDlgItemText (hWndDlg,IDC_STREET,Street);
					 BigRead (FidSave,(HPSTR)HouseNum,256);
					 SetDlgItemText (hWndDlg,IDC_HOUSE_NUM,HouseNum); 
					 BigRead (FidSave,(HPSTR)City,128);
					 SetDlgItemText (hWndDlg,IDC_CITY,City); 
					 BigRead (FidSave,(HPSTR)ZIP,128);
					 SetDlgItemText (hWndDlg,IDC_ZIP,ZIP); 
					 BigRead (FidSave,(HPSTR)str,256);
					 SetDlgItemText (hWndDlg,IDC_SQL,(LPSTR)str);                
					 BigRead (FidSave,(HPSTR)str,256);
					 SetDlgItemText (hWndDlg,IDC_KEY_FIELD,(LPSTR)str);                
					 BigRead (FidSave,(HPSTR)str,256);
					 SetDlgItemText (hWndDlg,IDC_SYMBOL,(LPSTR)str);                
					 BigRead (FidSave,(HPSTR)str,256);
					 SetDlgItemText (hWndDlg,IDC_FROMDATE,(LPSTR)str);                
					 BigRead (FidSave,(HPSTR)str,254);
					 SetDlgItemText (hWndDlg,IDC_TODATE,(LPSTR)str); 
					 BigRead (FidSave,(HPSTR)&AutoEdit,2);               
					 SendDlgItemMessage (hWndDlg,IDC_AUTOEDIT,BM_SETCHECK,AutoEdit,0L);
					 BigRead (FidSave,(HPSTR)&TotAddLen,4);
					 BigRead (FidSave,(HPSTR)&NumMatched,4);
					 BigRead (FidSave,(HPSTR)&MOPT,2);
					 break;
				 case 2:
					 BigRead (FidSave,(HPSTR)IMDataFile,256);//lnIMDataFile);
					 BigRead (FidSave,(HPSTR)DestName,256);//lnDestName);
					 SetDlgItemText (hWndDlg,IDC_DEST_FILE,DestName);
					 BigRead (FidSave,(HPSTR)Street,256);
					 SetDlgItemText (hWndDlg,IDC_STREET,Street);
					 BigRead (FidSave,(HPSTR)HouseNum,256);
					 SetDlgItemText (hWndDlg,IDC_HOUSE_NUM,HouseNum); 
					 BigRead (FidSave,(HPSTR)City,128);
					 SetDlgItemText (hWndDlg,IDC_CITY,City); 
					 BigRead (FidSave,(HPSTR)ZIP,128);
					 SetDlgItemText (hWndDlg,IDC_ZIP,ZIP); 
					 BigRead (FidSave,(HPSTR)str,256);
					 SetDlgItemText (hWndDlg,IDC_SQL,(LPSTR)str);                
					 BigRead (FidSave,(HPSTR)str,256);
					 SetDlgItemText (hWndDlg,IDC_KEY_FIELD,(LPSTR)str);                
					 BigRead (FidSave,(HPSTR)str,256);
					 SetDlgItemText (hWndDlg,IDC_SYMBOL,(LPSTR)str);                
					 BigRead (FidSave,(HPSTR)str,256);
					 SetDlgItemText (hWndDlg,IDC_FROMDATE,(LPSTR)str);                
					 BigRead (FidSave,(HPSTR)str,254);
					 SetDlgItemText (hWndDlg,IDC_TODATE,(LPSTR)str); 
					 BigRead (FidSave,(HPSTR)str,254);
					 SetDlgItemText (hWndDlg,IDC_NOTE,(LPSTR)str); 
					 BigRead (FidSave,(HPSTR)&AutoEdit,2);               
					 SendDlgItemMessage (hWndDlg,IDC_AUTOEDIT,BM_SETCHECK,AutoEdit,0L);
					 BigRead (FidSave,(HPSTR)&MOPT,2);               
			 		 SendDlgItemMessage (hWndDlg,IDC_MOPT,CB_SETCURSEL,(WPARAM)(MOPT-1),(LPARAM)NULL); 
					 BigRead (FidSave,(HPSTR)&UseNetBased,2);               
					 SendDlgItemMessage (hWndDlg,IDC_USENETWORK_BASED,BM_SETCHECK,UseNetBased,0L);
					 BigRead (FidSave,(HPSTR)&UsePointBased,2);               
					 SendDlgItemMessage (hWndDlg,IDC_USEPOINT_BASED,BM_SETCHECK,UsePointBased,0L);
					 BigRead (FidSave,(HPSTR)&SelectOne,2);               
					 SendDlgItemMessage (hWndDlg,IDC_SELECT_ONE,BM_SETCHECK,SelectOne,0L);
					 BigRead (FidSave,(HPSTR)&AddTol,2);               
					 SetDlgItemInt (hWndDlg,IDC_ADDTOL,AddTol,FALSE); 
					 BigRead (FidSave,(HPSTR)&TotAddLen,4);
					 BigRead (FidSave,(HPSTR)&NumMatched,4);
					 if (BigRead (FidSave,(HPSTR)&CreateNewFile,4) != 4)
						CreateNewFile = TRUE;
					 SendDlgItemMessage (hWndDlg,IDC_NEW,BM_SETCHECK,CreateNewFile,0L);
					 break;
				 }
                 GSSiClose (FidSave);
                 PctBox (GetDlgItem(hWndDlg,IDC_STATUS1), TotAddLen, NumMatched,0);
                 FileIsOpen = TRUE;
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EDIT_DEST),TRUE); 
                 EnableWindow (GetDlgItem(hWndDlg,IDC_EDIT_HELPER),TRUE); 
                 CloseDataFile (TRUE, &hSQL);  
                 GetDlgItemText (hWndDlg,IDC_SQL,(LPSTR)str,256);                
                 pSQL = str;
                 if (!_fstrcmp (pSQL,"ALL ROWS"))
                    *pSQL = 0;
                 hSQL = 0; 
                 ExpandText (pSQL);
                 if (!OpenDataFile (IMDataFile,pSQL,BT_READ,&hSQL))
                    GSSiMsgBox(GetFocus(),"Cannot open data file", 0,MB_ICONQUESTION|MB_OK,0);
                 PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); 
            }
           		 break;
           		 
            case IDC_LOCATE_DEST: 
                 *str=0;
                 if (!GetSaveName2 (hWndDlg,str,IDS_FILTERGWD,".GMD",IDS_FILEGMD)) break;   
                 SetDlgItemText (hWndDlg,IDC_DEST_FILE,str);
                 break;
                  
            case IDC_EDIT_DEST:
            {
                  FARPROC	lpfnADD_MATCH_EDITMsgProc; 
				  HWND	hdlg;

                  lpfnADD_MATCH_EDITMsgProc = MakeProcInstance((FARPROC)ADD_MATCH_EDITMsgProc, hInst);
                  hdlg = CreateDialog(hInst, (LPSTR)"ADD_MATCH_EDIT", hWndDlg, lpfnADD_MATCH_EDITMsgProc);
				  PostMessage (hdlg,WM_COMMAND,IDC_ISMODELESS,0);
//                  nRc = DialogBox(hInst, (LPSTR)"ADD_MATCH_EDIT", hWndDlg, lpfnADD_MATCH_EDITMsgProc);
//                  FreeProcInstance(lpfnADD_MATCH_EDITMsgProc);
            }
            	 break;

            case IDC_EDIT_HELPER:
            {
				SetCurVal (DestName,IDS_FILEADDEDITHELP);  
		        PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
		        PostMessage(hWndMain, WM_COMMAND, IDM_ADDEDIT_HELPER, 0L);
            }
            	break;
            	 
            case IDC_OPEN_DB:
/*            	 if (IMDataFileType == UMIFS_DATAFILE)
            	 {
                 	EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),FALSE);
                 	EnableWindow (GetDlgItem(hWndDlg,IDC_SQL),FALSE);  
                 }
                 else
                 {
	               	 EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),TRUE);
	               	 EnableWindow (GetDlgItem(hWndDlg,IDC_SQL),TRUE);
	             }  */
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),TRUE);
                 break;  
            
            case IDC_SHOW_FIELDS:
            	 DisplayFieldList (hWndDlg,hSQL,NULL,0,0);
                 break;
                      
            case IDC_SETSQL:      
            {    
                 HANDLE hMem;
                 LPSTR  lpStr, lpWhere;
                 
                 hMem = GSSiGlobAlloc ( 121,GHND,4096);
                 lpStr = GlobalLock (hMem); 
                 GetDlgItemText (hWndDlg,IDC_SQL,lpStr,1024);
                 if (GetSQLWhereClause (hWndDlg, hSQL, lpStr))
                 	SetDlgItemText (hWndDlg,IDC_SQL,lpStr);    
                 GSSiGlobUlFree (&hMem);
                 break;
            }
            
			case IDOK:
			{
				long   lineno = 0, CurLoc, MidLine, ii, FromDate, ToDate;
				long   NewRefno, StreetNum1, StreetNum2, OnStreetNum, MunicNum;
				short    st, SymNum, iUDI = 0, AreaSym, LineSym, LocOpt, Pass = 1;
				BOOL   Done, First = TRUE;
				HANDLE hMIDstr;

				char   SymName[10], project[32];
				double X, Y;
				LPSTR  lpTAB;
				DPOINT Point;
				BOOL   Store;
				short    Symbol = 1;
				COLORREF   Color;
				LPSTR  lpDot, pSpace, pStreet;
				MNMXCORD MinMaxCoord;
				double coordcvt = 1;
				short    NumSyms = 0;
				HANDLE hSymDesc = 0;
				short	match, sn, OrigKeyLen;
				BOOL	Opened, OpenedUAA, OpenedSeg = FALSE, OpenedSM;
				short	IndexArray[2];
				HANDLE	hMatch = 0, hDBDest, hBTDest, hBTBadNames;
				UINT	len;
				LPADDMATCH	pMatch;
				LPADDMATCHEDITREC pAMER;
				LPGWDHEADER	lpGWDHead;
				BADNAMEKEY		BadNameKey;
				BOOL	UseStoredResults = FALSE, RemoveLastPart = FALSE, OpenedSP;
				short	nPartsRemoved = 0, AddUDILength = 0;
				long	UseMult = 0;
				HANDLE	hKeyFields;
				LPSTR	pPar;

				ContinueProcessing = TRUE;
				HaltMapDisplay(FALSE, TRUE);
				CloseAllRequestedFiles(FALSE);
				GetDlgItemText(hWndDlg, SV_DATABASE_LIST, IMDataFile, lnIMDataFile);
				if (!GetDlgItemText(hWndDlg, IDC_DEST_FILE, DestName, lnDestName))
				{
					GSSiMsgBox(GetFocus(), "No destination file", 0, MB_ICONQUESTION | MB_OK, 0);
					break;
				}
				ExpandText(DestName);
				GetDlgItemText(hWndDlg, IDC_STREET, Street, sizeof(Street));
				GetDlgItemText(hWndDlg, IDC_HOUSE_NUM, HouseNum, sizeof(HouseNum));
				if (!*Street || !*HouseNum)
				{
					GSSiMsgBox(GetFocus(), "Street and/or House Number field missing", 0, MB_ICONQUESTION | MB_OK, 0);
					break;
				}

				MOPT = SendDlgItemMessage(hWndDlg, IDC_MOPT, CB_GETCURSEL, 0, 0) + 1;
				if (MOPT > 2)
					RemoveLastPart = TRUE;
				SetDlgItemText(hWndDlg, IDC_PROCESS_MESS, "Loading Data");
				CloseDataFile(TRUE, &hSQL);
				GetDlgItemText(hWndDlg, IDC_SQL, (LPSTR)str, 256);
				pSQL = str;
				if (!_fstrcmp(pSQL, "ALL ROWS"))
					*pSQL = 0;
				hSQL = 0;
				ExpandText(pSQL);
				if (!OpenDataFile(IMDataFile, pSQL, BT_READ, &hSQL))
				{
					GSSiMsgBox(GetFocus(), "Cannot open data file", 0, MB_ICONQUESTION | MB_OK, 0);
					break;
				}
				AutoEdit = SendDlgItemMessage(hWndDlg, IDC_AUTOEDIT, BM_GETCHECK, 0, 0L);
				if (!OpenStreetSegmentTable(FALSE, &OpenedSeg))
					break;
				if (!OpenNetIntersect(NetworkID, FALSE, &Opened))
					break;
				OpenUserDefinedAddress(FALSE, &OpenedUAA);
				OpenSegMaxIndex(&OpenedSM);
				OpenStreetPolys(&OpenedSP);
				GetDlgItemText(hWndDlg, IDC_KEY_FIELD, str, 256);
				if (!*str)
					OrigKeyLen = 2;
				else if (_fstrchr(str, '@'))
					OrigKeyLen = 100;
				else
					OrigKeyLen = 64;
				UseNetBased = SendDlgItemMessage(hWndDlg, IDC_USENETWORK_BASED, BM_GETCHECK, 0, 0L);
				SelectOne = SendDlgItemMessage(hWndDlg, IDC_SELECT_ONE, BM_GETCHECK, 0, 0L);
				if ((UsePointBased = SendDlgItemMessage(hWndDlg, IDC_USEPOINT_BASED, BM_GETCHECK, 0, 0L)))
				{
					GWFLDINFO FieldInfo;

					if (!OpenAddressFilesPID(hWndDlg))
						break;
					if (GWDGetFieldInfoFromName(hPIDAddDB, AddUDIVar, &FieldInfo, &AddUDIVarIndex))
					{
						AddUDILength = FieldInfo.Len;
					}

				}
				if (!AddUDILength)
					UsePointBased = FALSE;
				if (SendDlgItemMessage(hWndDlg, IDC_NEW, BM_GETCHECK, 0, 0L))
				{
					if (!CreateADD_MATCHTable(DestName, BadNames, "KEY(B4)", AddUDILength))
					{
						GSSiMsgBox(GetFocus(), "Unable to create destination file", 0, MB_ICONQUESTION | MB_OK, 0);
						break;
					}
					GetFieldIDsFromNames(DestName, &hKeyFields, 0, "MatchCode;KEY");
					GWDAddIndex(DestName, hKeyFields, 1, 0);
					GSSiGlobFree(&hKeyFields);
				}
				hDBDest = OpenGWDatabase(DestName, BT_WRITE);
				if (!hDBDest)
				{
					GSSiMsgBox(GetFocus(), "Cannot open destination file", 0, MB_ICONQUESTION | MB_OK, 0);
					break;
				}
				//			    hBTBadNames = BT_OPEN (BadNames,0,BT_WRITE,0);
				lpGWDHead = (LPGWDHEADER)GlobalLock(hDBDest);

				EnableWindow(GetDlgItem(hWndDlg, IDC_EXIT), FALSE);
				EnableWindow(GetDlgItem(hWndDlg, IDOK), FALSE);
				EnableWindow(GetDlgItem(hWndDlg, IDCANCEL), TRUE);

				if (First)
				{
					HCURSOR	OldCursor;

					OldCursor = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
					TotAddLen = NumSQLRows(hSQL);
					GSSiSetCursor(OldCursor);
				}
				First = FALSE;
				SetDlgItemText(hWndDlg, IDC_PROCESS_MESS, "Loading Data");

				pAMER = (LPADDMATCHEDITREC)&lpGWDHead->GWDData[lpGWDHead->pFldInfo->Len];

				Processing = TRUE;
				NewRefno = 0;
				Done = FALSE;
				//StartFastPick (65);
				NumMatched = NumNoMatch = NumInvalid = NumMultMatch = 0;
			NextLine:
				if (!FetchDBRec(hSQL) || !ContinueProcessing)
					goto EndFile;

				lineno++;
				GetDlgItemText(hWndDlg, IDC_STREET, Street, 256);
				pStreet = Street;
				GetDlgItemText(hWndDlg, IDC_HOUSE_NUM, HouseNum, 256);
				GetDlgItemText(hWndDlg, IDC_ZIP, ZIP, 128);
				GetDlgItemText(hWndDlg, IDC_CITY, City, 128);
				ExpandText(pStreet);
				if ((pPar = strrchr(pStreet, '(')))
					*pPar = 0;
				if ((pPar = strrchr(pStreet, '[')))
					*pPar = 0;
				ExpandText(HouseNum);
				OneSpace(pStreet);
				OneSpace(HouseNum);
				ExpandText(City);
				ExpandText(ZIP);
				match = 0;
				nPartsRemoved = 0;
				if (!_fstricmp(HouseNum, "STREET"))
					pStreet = GetHouseAndStreet(pStreet, HouseNum);
				_fstrcpy(OrigStreet, pStreet);
				MunicNum = GetMunicFromName(City);
				if (*HouseNum || !*pStreet)
				{
				TryAgain:
					match = ADD_MATCH(pStreet, HouseNum, City, ZIP, MOPT,
						&hMatch, &StreetNum, &MunicNum, UsePointBased, UseNetBased, 0, 0, 0);
					if (!match && RemoveLastPart && nPartsRemoved < 2)
					{
						LPSTR	pSpace = _fstrrchr(pStreet, ' ');

						if (pSpace)
						{
							*pSpace = 0;
							GSSiGlobFree(&hMatch);
							nPartsRemoved++;
							goto TryAgain;
						}
					}
				}
				else if (SeparateOnFromToStreets(pStreet, OnStreet, Street1, Street2))
					match = OFT_MATCH(OnStreet, Street1, Street2, City, 0, MOPT, &hMatch, &OnStreetNum, &StreetNum1, &StreetNum2, &MunicNum, 0, 0, 0, 0);
				else if (SeparateIntStreets(pStreet, Street1, Street2))
					match = INT_MATCH(Street1, Street2, City, 0, MOPT, &hMatch, &StreetNum1, &StreetNum2, &MunicNum);
				else if (FoundUserAssignedAddress(0, Street, MunicNum, FALSE, &hMatch))
					match = 1;

				_fmemset(pAMER, 0, (size_t)lpGWDHead->Reclen);
				//	 pAMER->RecordNum = lineno; 
				_fstrncpy(pAMER->Street, OrigStreet, 64);
				_fstrncpy(pAMER->House, HouseNum, 12);
				_fstrncpy(pAMER->City, City, 32);
				_fstrncpy(pAMER->ZIP, ZIP, 12);
				pAMER->AM.PartsRemoved = nPartsRemoved;
				GetDlgItemText(hWndDlg, IDC_KEY_FIELD, str, 256);
				if (!*str || !_fstricmp(str, "%RECORDOFFSET"))
				{
					LPOPENSQLDATA	SQLPtr = (LPOPENSQLDATA)GlobalLock(hSQL);

					ltoa(SQLPtr->Offset, str, 10);
					GlobalUnlock(hSQL);
				}
				else
					ExpandText(str);
				SetFieldValFromChar(lpGWDHead, &lpGWDHead->pFldInfo[0], str, FALSE, FALSE);
				SetFieldValFromCharAndName(lpGWDHead, "OriginalFileKey", str, FALSE);
				//                _fstrncpy (pAMER->OrigKey,str,OrigKeyLen);
				if (match > 1)
				{
					DPOINT p = { 0, 0 };

					pMatch = (LPADDMATCH)GlobalLock(hMatch);
					for (i = 0; i < match; i++)
					{
						p.x += pMatch[i].Point.x;
						p.y += pMatch[i].Point.y;
					}
					p.x /= match;
					p.y /= match;
					for (i = 0; i<match; i++)
					{
						double d = ldistp(p, pMatch[i].Point);
						if (d > GetGlobalDVal2("[%AddMatchAverageDist]", 25))
							goto OutOfRange;
					}
					pMatch[0].Point = p;
					match = 1;
				OutOfRange:
					GlobalUnlock(hMatch);
				}
				switch (match)
				{
				case -1:
					NumInvalid++;
					goto NextStep;
					break;
				case 1:
					NumMatched++;
					pMatch = (LPADDMATCH)GlobalLock(hMatch);
					pAMER->AM = *pMatch;
					if (UseStoredResults)
					{
						if (_fstrlen(pStreet) <= STORED_STREET_LEN &&
							_fstrlen(HouseNum) <= STORED_HOUSE_LEN &&
							_fstrlen(City) <= STORED_CITY_LEN &&
							_fstrlen(ZIP) <= STORED_ZIP_LEN)
						{
						}
						//	pStreet
						//	HouseNum 
						//	City  
						//	ZIP      

					}
					GlobalUnlock(hMatch);
					IndexArray[1] = FALSE;
					break;
				case 0:
					NumNoMatch++;
					BadNameKey.RecNum = lineno;
					if (!StreetNum)
					{
						_fstrncpy(BadNameKey.Name, pStreet, 40);
						sn = 1;
						//BT_PUT (hBTBadNames,(LPSTR)&BadNameKey,(LPSTR)&sn);
					}
					goto NextStep;
				default:
					NumMultMatch++;
					if (SendDlgItemMessage(hWndDlg, IDC_SELECT_ONE, (UINT)BM_GETCHECK, (WPARAM)0, (LPARAM)0L))
					{
						int which = UseMult++ % match;
						NumMatched++;
						pMatch = (LPADDMATCH)GlobalLock(hMatch);
						pAMER->AM = pMatch[which];
						GlobalUnlock(hMatch);
						IndexArray[1] = FALSE;
						break;
					}
					match = 4;
				NextStep:
					pAMER->AM.MatchCode = match;
					pAMER->AM.StreetNum = StreetNum;
					pAMER->AM.Munic = MunicNum;
					_fstrncpy(pAMER->Street, OrigStreet, MAXSTREETNAMELEN);
					IndexArray[1] = TRUE;
					break;
				}
				if (hMatch && pMatch->LocationCode > 0)
				{
					AddDPointToMinMax(&pMatch->Point, &lpGWDHead->FileBounds);
				}
				GSSiGlobFree(&hMatch);
				GetDlgItemText(hWndDlg, IDC_SYMBOL, str, 250);
				SetFieldValFromCharAndName(lpGWDHead, "Symbol", str, FALSE);
				GetDlgItemText(hWndDlg, IDC_FROMDATE, str, 250);
				ExpandText(str);
				FromDate = max(0, atoi(str));
				pAMER->FromDate = FromDate;
				SetFieldValFromCharAndName(lpGWDHead, "FromDateC", str, FALSE);
				GetDlgItemText(hWndDlg, IDC_TODATE, str, 250);
				ExpandText(str);
				ToDate = max(FromDate, atoi(str));
				pAMER->ToDate = ToDate;
				SetFieldValFromCharAndName(lpGWDHead, "ToDateC", str, FALSE);
				lpGWDHead->MinTime = min(lpGWDHead->MinTime, FromDate);
				lpGWDHead->MaxTime = max(lpGWDHead->MaxTime, ToDate);
				GetDlgItemText(hWndDlg, IDC_NOTE, str, 250);
				SetFieldValFromCharAndName(lpGWDHead, "Note", str, FALSE);
				GWDAddRecord(lpGWDHead, 0, 0);//IndexArray);
				//                 sprintf (str,"%ld %ld",TotLen, NumMatched); 
				//                 SetDlgItemText (hWndDlg,IDC_PROCESS_MESS,str);
				PctBox(GetDlgItem(hWndDlg, IDC_STATUS1), TotAddLen, NumMatched, 0);
				PctBox(GetDlgItem(hWndDlg, IDC_STATUS2), TotAddLen, lineno, 0);
				goto NextLine;

			EndFile:
				rtn = ContinueProcessing;
				ContinueProcessing = TRUE;
				EndFastPick();
				SetGlobalValueLong("%NUMMATCH", NumMatched);
				SetGlobalValueLong("%NUMNOMATCH", NumNoMatch);
				SetGlobalValueLong("%NUMMULTIMATCH", NumMultMatch);

				sprintf(str, "Finished - %ld matched, %ld no hits, %ld multiple hits, %ld invalid data", NumMatched, NumNoMatch, NumMultMatch, NumInvalid);
				SetDlgItemText(hWndDlg, IDC_PROCESS_MESS, str);
				Done = TRUE;
				if (hSQL)
				{
					CloseDataFile(TRUE, &hSQL);
				}
				GlobalUnlock(hDBDest);
				CloseGWDatabase(hDBDest);
				hDBDest = 0;
				// BT_CLOSE (hBTBadNames);  
				CloseStreetNameTable();
				GetMunicFromName(NULL);
				GetNumZIPsInMunic(0, NULL);
				CloseSegMaxIndex(OpenedSM);
				CloseStreetPolys(OpenedSP);
				CloseStreetSegmentTable(TRUE);
				CloseNetIntersect(Opened);
				CloseUserDefinedAddress(OpenedUAA);
				CloseAddressFilesPID();
				DisableHalt = FALSE;
				ContinueProcessing = TRUE;
				HaltMapDisplay(FALSE,TRUE);
				EnableWindow(GetDlgItem(hWndDlg, IDC_EDIT_DEST), TRUE);
				EnableWindow(GetDlgItem(hWndDlg, IDC_EDIT_HELPER), TRUE);
			Reset:
				Processing = FALSE;
				ContinueProcessing = TRUE;
				EnableWindow(GetDlgItem(hWndDlg, IDC_EXIT), TRUE);
				EnableWindow(GetDlgItem(hWndDlg, IDOK), TRUE);
				EnableWindow(GetDlgItem(hWndDlg, IDCANCEL), FALSE);
				FileIsOpen = FALSE;
				if (/*NumMatched != TotAddLen && */AutoEdit)
				{
					HWND hDlg = CreateDialog(hInst, (LPSTR)"ADD_MATCH_EDIT", hWndMain, ADD_MATCH_EDITMsgProc);
					PostMessage(hDlg, WM_COMMAND, IDC_ISMODELESS, 0);
				}
		         	//PostMessage(hWndDlg, WM_COMMAND, IDC_EDIT_DEST, 0L);
				 if (*AutoExportName)
		         	PostMessage(hWndDlg, WM_COMMAND, IDC_EXIT, 0L);
				 else
	                PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L); // datafile should always be open so SQL and FIELDS work
                 break;
                 
            }   
          }
          break;

    default: 
        GSSiGlobUlFree (&hStr);
RtnFalse:
{
#if ENABLETRACE
GSSiExitProg (1382);
#endif
		return FALSE;
}
   }
 GSSiGlobUlFree (&hStr);
RtnTrue:
{
#if ENABLETRACE
GSSiExitProg (1382);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL FAR PASCAL ADDRESS1MsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
 short i, nchar,ikey, st;
 static LONG House;
 static short OddEven;
 BOOL translated;
 BOOL FAR *p_translated = &translated;
 char StreetBuf[32];
 char HouseBuf[16];
 char str[128], TrueStreet[66];
 short  Choice, irc;  
 long	HouseNum, ZIP;
 static HWND CurFocus;
 static	BOOL	OpenedSeg=FALSE, OpenedSM=FALSE,OpenedSP=FALSE;
 int	TabStops[3]={500,600,700}; 
 BOOL	Error; 
 LPSTR	lpTab;

 short  BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam))) return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
         cwCenter(hWndDlg, 0);  
         GetGlobalCVal ("[%NETADDRESS]",str,"NONE");
         if (!_fstrcmp (str,"NONE"))
         	SetGlobalValue ("%NETADDRESS","@[%NETHOUSE] @[%NETSTREET], @[%NETMUNIC] @[%NETZIP]"); 
	     if (!OpenStreetNameTable(FALSE))  
	     {
            GSSiMsgBox( GetFocus(), "Unable to open street name table", NULL,MB_OK|MB_ICONEXCLAMATION,0);
	        return FALSE;
	     }
		 if (!OpenStreetSegmentTable (FALSE,&OpenedSeg))
		 	return FALSE;
		 if (!OpenSegMaxIndex (&OpenedSM)) 
		 {  
			CloseStreetSegmentTable (TRUE);
		 	return FALSE;
		 }
         OpenStreetPolys (&OpenedSP);	
         AddRangeAll = GetGlobalBVal ("[%ADDRANGEALL]");
         AddTol = GetGlobalLVal ("[%ADDTOL]");
         Trigger = GetGlobalLVal ("[%TRIGGER]");
         if (!Trigger)
         	Trigger = 3; 
    	 SetDlgItemInt (hWndDlg,IDC_TRIGGER,Trigger,FALSE);
    	 SetDlgItemInt (hWndDlg,IDC_ADD_TOL,AddTol,FALSE); 
         if (AddRangeAll)
         {
            SendDlgItemMessage (hWndDlg,IDC_ADD_RANGE_NEAREST,BM_SETCHECK,FALSE,0L); 
            SendDlgItemMessage (hWndDlg,IDC_ADD_RANGE_ALL,BM_SETCHECK,TRUE,0L);
         } 
         else
         {
            SendDlgItemMessage (hWndDlg,IDC_ADD_RANGE_NEAREST,BM_SETCHECK,TRUE,0L); 
            SendDlgItemMessage (hWndDlg,IDC_ADD_RANGE_ALL,BM_SETCHECK,FALSE,0L);
         }
         SetDlgItemText (hWndDlg,IDM_STREET,StreetBufSave);
         SetDlgItemText (hWndDlg,IDM_HOUSE,HouseBufSave); 
         SendDlgItemMessage (hWndDlg,IDM_STREET_MENU,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
#if WIN32
         CurFocus = (void *)IDM_HOUSE;
#else
         CurFocus = IDM_HOUSE;
#endif
         break; /* End of WM_INITDIALOG                                 */

	case WM_DESTROY:
		 CloseStreetNameTable();
		 CloseSegMaxIndex (TRUE);
		 CloseStreetSegmentTable (TRUE);
		 CloseStreetPolys (OpenedSP);
		 break;
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
            case IDM_HOUSE: /* Edit Control                             */
                 switch (HIWORD(wParam))
                 {  case EN_CHANGE:
                        nchar = GetDlgItemText(hWndDlg,IDM_HOUSE,HouseBuf,16); 
                        House = 0;
                        OddEven = 0;
                        if (nchar > 0)

                        {
                            if (!isdigit (HouseBuf[nchar-1]) &
                                 HouseBuf[nchar-1] != '?')
                            {
                                if (HouseBuf[nchar-1]==' ')
                                    StreetBuf[0]='\0';
                                else
                                {
                                    StreetBuf[0]=HouseBuf[nchar-1];
                                    StreetBuf[1]='\0';
                                }
                                HouseBuf[nchar-1]='\0';
                                SetDlgItemText (hWndDlg,IDM_HOUSE,HouseBuf);
                                SetFocus (GetDlgItem(hWndDlg,IDM_STREET));
                                SetDlgItemText(hWndDlg,IDM_STREET,StreetBuf);
                                SendDlgItemMessage(hWndDlg,IDM_STREET,
                                                EM_SETSEL,
                                                1,9999);
                            }

                            else 
                            {
		                        if (nchar == 0 || HouseBuf[0] == '?')
		                        {
		                            House = 0;
		                            OddEven = 0;
		                        }
		                        else if (HouseBuf[nchar-1] == '?' &
		                                 HouseBuf[nchar-2] == '?')
		                        {
		                            OddEven = 3;
		                            HouseBuf[nchar-2]='\0';
		                            House = atol (HouseBuf);
		                        }
		                        else if (HouseBuf[nchar-1] == '?')
		                        {
		                            HouseBuf[nchar-1]='\0';
		                            House = atol (HouseBuf);
		                            OddEven = House % 2 +1;
		                        }
		                        else
		                        {   HouseBuf[nchar]='\0';
		                            House = atol (HouseBuf);
		                            OddEven = 0;
		                        };
#if WIN32
		                        CurFocus = (void *)IDM_HOUSE;
#else
		                        CurFocus = IDM_HOUSE;
#endif       
		                        i = GetDlgItemText (hWndDlg,IDM_STREET,StreetBuf,32);
		                        DisplayStreets (hWndDlg,House,OddEven,StreetBuf,i,IDM_HOUSE);
							}

                        }
                    break;

                    case EN_SETFOCUS:
#if WIN32
	                    CurFocus = (void *)IDM_HOUSE;
#else
	                    CurFocus = IDM_HOUSE;
#endif
                        break;

                    
                 }
                 break;

            case IDM_STREET: /* Edit Control                            */
#if WIN32
                CurFocus = (void *)IDM_STREET;
#else
                CurFocus = IDM_STREET;
#endif

                 switch (HIWORD(wParam))
                 {  case EN_CHANGE:
                        i = GetDlgItemText (hWndDlg,IDM_STREET,StreetBuf,32);
                        DisplayStreets (hWndDlg,House,OddEven,StreetBuf,i,IDM_STREET);
                        break;

                    case EN_SETFOCUS:
#if WIN32
                        CurFocus = (void *)IDM_STREET;
#else
                        CurFocus = IDM_STREET;
#endif
                        break;

                 }
                 break;

            case IDM_STREET_MENU: /* List box                           */
                 switch(HIWORD(wParam))
                 {   case LBN_SELCHANGE:
                     case LBN_DBLCLK:
					 PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
                 }
                 break;

			case IDC_TRY_MATCH:
            {
				HANDLE hList = GSSiGlobAlloc ( 574,GMEM_MOVEABLE,USHRT_MAX); 
				short	nList=0, nAdded;
				HANDLE	hMem=GSSiGlobAlloc ( 573,GMEM_MOVEABLE,512);
				LPSTR	STDNAM1=GlobalLock (hMem);
				LPSTR	NRONAM1=STDNAM1+50;
				LPSTR	NMONLY1=NRONAM1+50;
				LPSTR	SANSCH1=NMONLY1+50;
				LPSTR	NANDCH1=SANSCH1+50;
				LPSTR	NCMPNM1=NANDCH1+50;
				LPSTR	ORIGNM1=NCMPNM1+50;
				LPSTR	SANSCP1=ORIGNM1+50;
				LPSTR	SANSCS1=SANSCP1+50;	 
				LPLONG	pList;
				
//			    SendDlgItemMessage (hWndDlg,IDM_STREET_MENU,LB_RESETCONTENT,0,0);
				GetDlgItemText (hWndDlg,IDM_STREET,StreetBuf,32);
			    STNDST(StreetBuf, (short)_fstrlen(StreetBuf),STDNAM1,NRONAM1,NMONLY1,
			                             			   SANSCH1,NANDCH1,NCMPNM1,ORIGNM1,SANSCP1,SANSCS1,NULL,NULL,NULL,NULL); 
			    nAdded = GetNameTypeList (2,&nList,hList,STDNAM1,NRONAM1,NMONLY1,
			                             				 SANSCH1,NANDCH1,NCMPNM1,ORIGNM1,SANSCP1,SANSCS1); 
            	pList = (LPLONG)GlobalLock (hList);
            	for (i=0; i < nList; i++,pList++)
				{
					char	TrueName[80];

					GetTrueStreetName (*pList, TrueName, 0,0);
					strupr (TrueName);
					//DisplayStreetsPIDFromStreetNum (hWndDlg,House-AddTol,House+AddTol,OddEven,*pList,MunicNum,IDM_STREET);
                    DisplayStreets (hWndDlg,House,OddEven,TrueName,strlen(TrueName),IDM_STREET);
				}
            	GSSiGlobUlFree (&hMem);
            	GSSiGlobUlFree (&hList);
            }
				 break;

            case IDOK:
				if (!SendDlgItemMessage(hWndDlg,IDM_STREET_MENU,LB_GETCOUNT,0,0))
				{
					PostMessage(hWndDlg, WM_COMMAND, IDC_TRY_MATCH, 0L);
					break;
				}
                Choice=(short)SendDlgItemMessage(hWndDlg,IDM_STREET_MENU,LB_GETCURSEL,0,0); 
                if (Choice<0) Choice=0;
                st = (short)SendDlgItemMessage(hWndDlg,IDM_STREET_MENU,LB_GETTEXT,Choice,(DWORD)&str);
                if (st && st != LB_ERR)
                {   
                	LPSTR	lpX, lpY;
                	
                	lpTab = _fstrchr (str,'\t');
                	*lpTab++ = 0; 
                	StreetNum = atol (lpTab);                
                	GetTrueStreetName (StreetNum,TrueStreet,0,0);
                	SetGlobalValue ("%NETSTREET",TrueStreet);
                	lpTab = _fstrchr (lpTab,':');
                	lpTab++;
                	HouseNum = atol (lpTab); 
                	SetGlobalValueLong ("%NETHOUSE",HouseNum);  
                	lpTab = _fstrchr (lpTab,':');
                	lpTab++;
                	ZIP = atol (lpTab);
                	if (ZIP)   
                		SetGlobalValueLong ("%NETZIP",ZIP);  
                	else
                		SetGlobalValue ("%NETZIP","");
                	lpX = _fstrrchr (lpTab,'|'); 
                	lpX++;
                	lpY = _fstrchr (lpX,',');  
                	lpY++;
                	UserSpecifiedBasePoint.x = atof (lpX);
                	UserSpecifiedBasePoint.y = atof (lpY);
					CurrentPoint = UserSpecifiedBasePoint;
					AddTol = GetDlgItemInt(hWndDlg,IDC_ADD_TOL,&Error,FALSE);
					AddRangeAll = SendDlgItemMessage (hWndDlg,IDC_ADD_RANGE_ALL,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
					Trigger = GetDlgItemInt (hWndDlg,IDC_TRIGGER,&Error,FALSE);
					SetGlobalValueLong ("%ADDTOL",AddTol);
					SetGlobalValueLong ("%TRIGGER",Trigger);
					SetGlobalValueBool ("%ADDRANGEALL",AddRangeAll);
					GetDlgItemText(hWndDlg,IDM_HOUSE,HouseBufSave,16);
					GetDlgItemText (hWndDlg,IDM_STREET,StreetBufSave,32);
					AddToView = SendDlgItemMessage (hWndDlg,IDC_ADDTOVIEW,(UINT)BM_GETCHECK,(WPARAM)0,(LPARAM)0L);
                    if (House)
                    	irc = 1;
                    else
                    	irc=2;
					irc = 3;
                    EndDialog(hWndDlg,irc);
                 }
                 else
                 {
                    SetFocus (GetDlgItem(hWndDlg,IDM_STREET)); 
#if WIN32
                    CurFocus = (void *)IDM_STREET;
#else
                    CurFocus = IDM_STREET;
#endif

                 }
                 break;

            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 GetDlgItemText(hWndDlg,IDM_HOUSE,HouseBufSave,16);
                 GetDlgItemText (hWndDlg,IDM_STREET,StreetBufSave,32);
                 EndDialog(hWndDlg, FALSE);
                 break; 
         }
    case LB_ADDSTRING:
         i = 1;
         break;

    default:
        return FALSE;
   }
 return TRUE;
} /* End of ADDRESSMsgProc                                      */
