#include "graphint.h"
#include "gmextern.h"

#define	MaxUserCheckbox	14
#define MaxUserLists	4
#define	MaxUserTextbox	5 
#define MAXDISPLAYEDVOTERS 1024

static	char FirstName[128];
static	char MiddleName[128];
static	char LastName[128];  
static	char ZipCodeC[16];  
static	short	VoterLocOpt;
static	char VoterDB[2][128]={"V0=[%DL]attribut\\voters\\camp\\voters.gmd","V1=[%DL]attribut\\voters\\full\\voters.gmd"};
static	BOOL EditVoterData;
static	short NumUserCheckbox=0, NumUserLists=0, NumUserTextbox=0;
static	UINT	USERCBCNTL[MaxUserCheckbox]={IDC_VOTER_USER1,IDC_VOTER_USER2,IDC_VOTER_USER3,IDC_VOTER_USER4,
											 IDC_VOTER_USER5,IDC_VOTER_USER6,IDC_VOTER_USER7,IDC_VOTER_USER8,
											 IDC_VOTER_USER9,IDC_VOTER_USER11,IDC_VOTER_USER12,IDC_VOTER_USER13,IDC_VOTER_USER14};	
static	UINT	USERLBCNTLT[MaxUserLists]={IDC_VOTER_USERLISTT_1,IDC_VOTER_USERLISTT_2,IDC_VOTER_USERLISTT_3,IDC_VOTER_USERLISTT_4};
static	UINT	USERLBCNTL[MaxUserLists]={IDC_VOTER_USERLIST_1,IDC_VOTER_USERLIST_2,IDC_VOTER_USERLIST_3,IDC_VOTER_USERLIST_4};
static	UINT	USERTXCNTL[MaxUserTextbox]={IDC_VOTER_USERTEXT_1,IDC_VOTER_USERTEXT_2,IDC_VOTER_USERTEXT_3,IDC_VOTER_USERTEXT_4,IDC_VOTER_USERTEXT_5};
static	UINT	USERTXCNTLT[MaxUserTextbox]={IDC_VOTER_USERTEXT_T1,IDC_VOTER_USERTEXT_T2,IDC_VOTER_USERTEXT_T3,IDC_VOTER_USERTEXT_T4,IDC_VOTER_USERTEXT_T5};
static	char	UserCheckboxTitle[MaxUserCheckbox][64];
static	char	UserCheckboxFieldName[MaxUserCheckbox][64];   
static	char	UserListTitle[MaxUserLists][64];
static	char	UserListFieldName[MaxUserLists][64]; 
static	HANDLE	UserListValues[MaxUserLists];
static	short	UserListNumValues[MaxUserLists];
static	short	UserListMaxValueLength[MaxUserLists];  
static	char	UserTextboxTitle[MaxUserTextbox][64];
static	char	UserTextboxFieldName[MaxUserTextbox][64]; 
static	short	UserTextboxMaxValueLength[MaxUserTextbox];  
static	char	NewUserPrompt[80];
static	char	NewUserFieldName[80];  
static	LPSHORT	pNewUserFieldMaxValueLength,pNewUserNumValues;
static	LPHANDLE	phNewUserFieldValueList;  
static	BOOL	NewUserFieldIsNew,VoterFieldsLoaded=FALSE;
static	char	ZipToCityFile[]="[%DL]address\\zipcities.btr"; 
static	char	ZipToPrecinctFile[]="[%DL]address\\zipprecincts.btr";     
static	long	DisplayedVoterIDs[MAXDISPLAYEDVOTERS];
static 	struct	{
			long	ZipCode;
			char	Name[64];
			}	ZipToNameKey;
static	HFILE	NewFid=HFILE_ERROR;
static	HFILE	OldFid=HFILE_ERROR;
static	char	TempFile[256];  
static	int		InsertId;

void InitVoterNameSearch (LPSTR FName,LPSTR MName,LPSTR LName,LPSTR ZIPC)
{
	VoterLocOpt = 1;   
	if (!FName)
		return;
	_fstrcpy (FirstName,FName);
	_fstrcpy (MiddleName,MName);
	_fstrcpy (LastName,LName);
	_fstrcpy (ZipCodeC,ZIPC); 
	return;
}

int	Insert (LPSTR Opt,LPSTR File,LPSTR Text)
{   
	char	str[260];
	
	if (!Opt || *Opt == 'L')
	{   
		if (NewFid == HFILE_ERROR)
			return 0;
		fputstring (Text,NewFid);
		return 1;
	}
	if (*Opt == 'E')
	{
		sprintf (str,"#INSERT%i",InsertId+1);
		fputstring (str,NewFid);
		while (fgetstring (str,255,OldFid))
			fputstring (str,NewFid); 
		GSSiClose (OldFid);
		GSSillseek (NewFid,0,0);
		OldFid = GSSiOpenFile (File,0,OF_CREATE);
		if (OldFid == HFILE_ERROR)
		{
			GSSiClose2 (&NewFid);
			return 0;
		}
		while (fgetstring (str,255,NewFid))
			fputstring (str,OldFid); 
		GSSiClose2 (&OldFid);
		GSSiClose2 (&NewFid);
		GSSiRemove (TempFile);
		return 1;
	}
	if (*Opt == 'B')
	{
		GSSiGetTempFileName (0,"gmv",0,TempFile);
		OldFid = GSSiOpenFile (File,0,OF_READ);
		if (OldFid == HFILE_ERROR)
			return 0;
		NewFid = GSSiOpenFile (TempFile,0,OF_CREATE);
		if (NewFid == HFILE_ERROR)
			return 0;
		while (fgetstring (str,255,OldFid))
		{
			if (!_fstrnicmp (str,"#INSERT",7))
			{
				InsertId = atoi (&str[7]);
				return InsertId;
			}    
			else
				fputstring (str,NewFid);
		}
		GSSiClose2 (&OldFid);
		GSSiClose2 (&NewFid);
		GSSiRemove (TempFile);
		return 0;
	}
	return 0;
}

void DestroyVoterLists (void)
{   
	short	i;
	
	if (!VoterFieldsLoaded)
		return;
	for (i=0;i<NumUserLists;i++)
		GSSiGlobFree (&UserListValues[i]);
	VoterFieldsLoaded = FALSE;
	return;
} 

BOOL AddFieldToPointList (HWND hWnd,GWFLDINFO NewField,LPHANDLE phNewFieldValueList)
{
	short	nRc=FALSE;

	if (!phNewFieldValueList)
	{
		FARPROC lpfnADDCBFIELDTOPOINTLISTMsgProc; 
	
		lpfnADDCBFIELDTOPOINTLISTMsgProc = MakeProcInstance((FARPROC)ADDCBFIELDTOPOINTLISTMsgProc, hInst);
		nRc = DialogBox(hInst, (LPSTR)"ADDCBFIELDTOPOINTLIST", hWnd, lpfnADDCBFIELDTOPOINTLISTMsgProc);
		FreeProcInstance(lpfnADDCBFIELDTOPOINTLISTMsgProc);  
	} 
	return nRc;
}

 

  

void SaveVoterFieldDefs (void)
{    
	HFILE	Fid;
	char	str[256];
	short	i,j; 
	LPSTR	pValue;
	
	if (NumUserCheckbox)
	{
		Fid = GSSiOpenFile ("[%DL]catalog\\checkbox.txt",0,OF_CREATE); 
		for (i=0;i<NumUserCheckbox;i++)
		{ 
			sprintf (str,"%s\t%s",UserCheckboxTitle[i],UserCheckboxFieldName[i]);
			fputstring (str,Fid);
		} 
		GSSiClose (Fid);
	}
	if (NumUserLists)
	{
		Fid = GSSiOpenFile ("[%DL]catalog\\lists.txt",0,OF_CREATE); 
		for (i=0;i<NumUserLists;i++)
		{ 
			sprintf (str,"%s\t%s\t%i\t%i",UserListTitle[i],UserListFieldName[i],UserListMaxValueLength[i],UserListNumValues[i]);
			fputstring (str,Fid); 
			pValue = GlobalLock (UserListValues[i]);
			for (j=0;j<UserListNumValues[i];j++,pValue+=(UserListMaxValueLength[i]+1))
				fputstring (pValue,Fid);  
			GlobalUnlock (UserListValues[i]);
		} 
		GSSiClose (Fid);
	}
	
	if (NumUserTextbox)
	{
		Fid = GSSiOpenFile ("[%DL]catalog\\textbox.txt",0,OF_CREATE); 
		for (i=0;i<NumUserTextbox;i++)
		{ 
			sprintf (str,"%s\t%s\t%i",UserTextboxTitle[i],UserTextboxFieldName[i],UserTextboxMaxValueLength[i]);
			fputstring (str,Fid);
		} 
		GSSiClose (Fid);
	}
	
}

void LoadVoterFields (HWND hWndDlg)
{
	short	i,j;  
	char	str[260];  
	HFILE	Fid;
	LPSTR	pTAB, pNum, pValue; 
	
	if (!VoterFieldsLoaded)
	{
		VoterFieldsLoaded = TRUE;
		NumUserCheckbox=NumUserLists=NumUserTextbox=0;
		Fid = GSSiOpenFile ("[%DL]catalog\\checkbox.txt",0,OF_READ);  
		if (Fid != HFILE_ERROR)
		{
			while (fgetstring (str,256,Fid))
			{  
				if (NumUserCheckbox >= MaxUserCheckbox)
					break;
				if ((pTAB = _fstrchr (str,'\t')))
				{
					*pTAB++ = 0;
					_fstrcpy (UserCheckboxTitle[NumUserCheckbox],str);
					_fstrcpy (UserCheckboxFieldName[NumUserCheckbox++],pTAB); 
				}
			}
			GSSiClose (Fid);
		}
		Fid = GSSiOpenFile ("[%DL]catalog\\lists.txt",0,OF_READ);  
		if (Fid != HFILE_ERROR)
		{
			while (fgetstring (str,256,Fid))
			{  
				if (NumUserLists >= MaxUserLists)
					break;
				if ((pTAB = _fstrchr (str,'\t')))
				{
					*pTAB++ = 0; 
					pNum = _fstrchr (pTAB,'\t');
					*pNum++ = 0;
					UserListMaxValueLength[NumUserLists] = atoi(pNum);
					pNum = _fstrchr (pNum,'\t');
					*pNum++ = 0;
					UserListNumValues[NumUserLists] = atoi(pNum);
					_fstrcpy (UserListTitle[NumUserLists],str);
	           		UserListValues[NumUserLists]=GSSiGlobAlloc (0,GMEM_MOVEABLE,4+(1+(long)UserListMaxValueLength[NumUserLists])*UserListNumValues[NumUserLists]);
					pValue = GlobalLock (UserListValues[NumUserLists]);
					for (j=0;j<UserListNumValues[NumUserLists];j++,pValue+=(UserListMaxValueLength[NumUserLists]+1))
					{
						fgetstring (pValue,UserListMaxValueLength[NumUserLists]+1,Fid);
					}
					GlobalUnlock (UserListValues[NumUserLists]);
					_fstrcpy (UserListFieldName[NumUserLists++],pTAB); 
				}
			}
			GSSiClose (Fid);
		}
		Fid = GSSiOpenFile ("[%DL]catalog\\textbox.txt",0,OF_READ);  
		if (Fid != HFILE_ERROR)
		{
			while (fgetstring (str,256,Fid))
			{  
				if (NumUserTextbox >= MaxUserTextbox)
					break;
				if ((pTAB = _fstrchr (str,'\t')))
				{
					*pTAB++ = 0;
					_fstrcpy (UserTextboxTitle[NumUserTextbox],str);
					pNum = _fstrchr (pTAB,'\t');
					*pNum++ = 0;
					UserListMaxValueLength[NumUserTextbox] = atoi(pNum);
					_fstrcpy (UserTextboxFieldName[NumUserTextbox++],pTAB); 
				}
			}
			GSSiClose (Fid);
		}
	}
	for (i=0;i<NumUserCheckbox;i++)
	{   
   		SetDlgItemText (hWndDlg,USERCBCNTL[i],UserCheckboxTitle[i]);
	 	ShowWindow (GetDlgItem(hWndDlg,USERCBCNTL[i]),SW_SHOW);  
	}
	for (i=0;i<NumUserLists;i++)
	{   
   		SetDlgItemText (hWndDlg,USERLBCNTLT[i],UserListTitle[i]);
	 	ShowWindow (GetDlgItem(hWndDlg,USERLBCNTL[i]),SW_SHOW);  
	 	ShowWindow (GetDlgItem(hWndDlg,USERLBCNTLT[i]),SW_SHOW);  
		SendDlgItemMessage (hWndDlg,USERLBCNTL[i],CB_RESETCONTENT,0,0);
		pValue = GlobalLock (UserListValues[i]);
		for (j=0;j<UserListNumValues[i];j++,pValue+=(UserListMaxValueLength[i]+1))
		{
			SendDlgItemMessage (hWndDlg,USERLBCNTL[i],CB_ADDSTRING,0,(LPARAM)pValue); 
		}
		GlobalUnlock (UserListValues[i]);
	}
	for (i=0;i<NumUserTextbox;i++)
	{   
   		SetDlgItemText (hWndDlg,USERTXCNTLT[i],UserTextboxTitle[i]);
	 	ShowWindow (GetDlgItem(hWndDlg,USERTXCNTL[i]),SW_SHOW);  
	 	ShowWindow (GetDlgItem(hWndDlg,USERTXCNTLT[i]),SW_SHOW);  
	}
	return;
}

BOOL GetUserFieldTitleAndName (HWND hWnd,LPSTR Title,LPSTR FieldName,BOOL New,LPSHORT pMaxValueLength,LPHANDLE phValueList,LPSHORT pNumValues)
{
	FARPROC lpfnFIELD_TITLE_AND_NAMEMsgProc; 
	short	nRc;
	
	_fstrcpy (NewUserPrompt,Title);
	_fstrcpy (NewUserFieldName,FieldName);
	NewUserFieldIsNew = New;
	pNewUserFieldMaxValueLength = pMaxValueLength;
	phNewUserFieldValueList = phValueList;
	pNewUserNumValues = pNumValues;
	lpfnFIELD_TITLE_AND_NAMEMsgProc = MakeProcInstance((FARPROC)FIELD_TITLE_AND_NAMEMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"FIELD_TITLE_AND_NAME", hWnd, lpfnFIELD_TITLE_AND_NAMEMsgProc);
	FreeProcInstance(lpfnFIELD_TITLE_AND_NAMEMsgProc);   
	if (nRc)
	{
		_fstrcpy (Title,NewUserPrompt);
		_fstrcpy (FieldName,NewUserFieldName);
	}
	return nRc;
}

long GetNewVoterID (void)
{
	LPGWDHEADER lpGWDHead;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr; 
    HANDLE	hDBVoter=0; 
    long	last, Offset;
					
    if (!OpenDataFile (VoterDB[0],"",BT_READ, &hDBVoter))
    {
        MessageBox( GetFocus(), VoterDB[0],"Cannot open voter database", MB_OK);
        return 0;
    }
    SQLPtr = (LPOPENSQLDATA) GlobalLock (hDBVoter);
    FilePtr = (LPOPENFILEDATA) GlobalLock (SQLPtr->OFHandle);
	lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);  
	if (BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&last,BT_LAST,BT_ANY,(LPSTR)&Offset))
		last = 10000000;                           
	GlobalUnlock (FilePtr->FileHandle); 
	GlobalUnlock (SQLPtr->OFHandle);
	GlobalUnlock (hDBVoter);  
	CloseDataFile (TRUE,&hDBVoter); 
	return last+1;
} 

BOOL AddNewVoters (HWND hWnd)
{

	FARPROC lpfnVOTER_NEWMsgProc; 
	short	nRc;
	
	EditVoterData = FALSE;			 
	lpfnVOTER_NEWMsgProc = MakeProcInstance((FARPROC)VOTER_NEWMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"VOTER_NEW", hWnd, lpfnVOTER_NEWMsgProc);
	FreeProcInstance(lpfnVOTER_NEWMsgProc);
	return nRc;
}

BOOL EditVoter (HWND hWnd,long VoterID,int VoterFile)
{

	FARPROC lpfnVOTER_NEWMsgProc; 
	short	nRc;  
	long	vid=VoterID;
	short	vf=VoterFile;
	
	EditVoterData = TRUE;	
	CurrentVoterID = vid;
	CurrentVoterDB = vf;		 
	lpfnVOTER_NEWMsgProc = MakeProcInstance((FARPROC)VOTER_NEWMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"VOTER_NEW", hWnd, lpfnVOTER_NEWMsgProc);
	FreeProcInstance(lpfnVOTER_NEWMsgProc);
	return nRc;
}

BOOL DefineVoter (HWND hWnd)
{

	FARPROC lpfnVOTER_NEWMsgProc; 
	short	nRc;
	
	lpfnVOTER_NEWMsgProc = MakeProcInstance((FARPROC)VOTER_DEFINE_USER_DATAMsgProc, hInst);
	nRc = DialogBox(hInst, (LPSTR)"VOTER_NEW", hWnd, lpfnVOTER_NEWMsgProc);
	FreeProcInstance(lpfnVOTER_NEWMsgProc);
	return nRc;
} 

void SetLocationCodeField (HWND hWndDlg,UINT icntl,int icode)
{   
	char	str[64];
	
	switch (icode)
	{
		case 1:
			_fstrcpy (str,"Street Address");
			break;
		case 5:
			_fstrcpy (str,"Precinct Centroid");
			break;
		case 9:
			_fstrcpy (str,"Zipcode Centroid");
			break;
		default:
			*str = 0;
	}
	SetDlgItemText (hWndDlg,icntl,str);
	return;
}

void SetCityNameListFromZIP (HWND hWndDlg,UINT icntl,long ZIP)
{   
//	char	Name[128]="[%DL]address\\zipcities.btr";
	short	pos=BT_FIRST,cond=BT_GE;     
	long	Count;                      
	HANDLE	hBT;
	
	SendDlgItemMessage (hWndDlg,icntl,CB_RESETCONTENT,0,0);  
	_fmemset (&ZipToNameKey,0,sizeof(ZipToNameKey));
	ZipToNameKey.ZipCode = ZIP;
    hBT = BT_OPEN (ZipToCityFile,0,BT_READ,0);  
    if (!hBT)
    	return;
    while (!BT_FIND (hBT,(LPSTR)&ZipToNameKey,pos,cond,(LPSTR)&Count))
    {
    	pos = BT_NEXT;
    	cond = BT_ANY;
    	if (ZipToNameKey.ZipCode != ZIP)
    		break;
		if (Count > 5)
			SendDlgItemMessage (hWndDlg,icntl,CB_ADDSTRING,0,(LPARAM)ZipToNameKey.Name); 
	}
	BT_CLOSE (hBT);
	return;
}

void SetPrecinctNameListFromZIP (HWND hWndDlg,UINT icntl,long ZIP)
{   
//	char	Name[128]="[%DL]address\\zipcities.btr";
	short	pos=BT_FIRST,cond=BT_GE;     
	long	Count;                      
	HANDLE	hBT;
	
	SendDlgItemMessage (hWndDlg,icntl,CB_RESETCONTENT,0,0);  
	_fmemset (&ZipToNameKey,0,sizeof(ZipToNameKey));
	ZipToNameKey.ZipCode = ZIP;
    hBT = BT_OPEN (ZipToPrecinctFile,0,BT_READ,0);  
    if (!hBT)
    	return;
    while (!BT_FIND (hBT,(LPSTR)&ZipToNameKey,pos,cond,(LPSTR)&Count))
    {
    	pos = BT_NEXT;
    	cond = BT_ANY;
    	if (ZipToNameKey.ZipCode != ZIP)
    		break;
		if (Count > 5)
			SendDlgItemMessage (hWndDlg,icntl,CB_ADDSTRING,0,(LPARAM)ZipToNameKey.Name); 
	}
	BT_CLOSE (hBT);
	return;
}

BOOL FAR PASCAL VOTER_NAME_LOCMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 	int		st, choice, n,idx,ifile,i; 
	BOOL	HaveFirstName,HaveMiddleName,HaveLastName;
	char	str[256];
	char	WantStr2[128],propkeyfld[64];
	LPGWDHEADER lpGWDHead;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
	long	Offset, WantZip;
	static	HANDLE	hDBVoter[2]={0,0};
	static	HANDLE	hSaveBM=0;  
	static	BOOL	First;  
	static	short	iVoter=-1; 
	short	Item;    
	int		nFound;
	long	NumRec, CurLoc,LocInc, VoterID;
	TAGKEY TAGKey;  
	LPSTR	pTAB; 
	HWND	hwndCtl;  
	static	BOOL	GetNext, InInit, InDisplay;
	static	char	NameDelim;
	UINT	nPrompts=4, NumDisplayedVoterIDs;
	UINT	PrmtDat[3*3] = {
							  IDC_VOTERNAMEFIRST,PRMT_IDC_VOTERNAMEWC,0,
							  IDC_VOTERNAME, PRMT_IDC_VOTERNAME,0,  
							  IDC_VOTERLIST, PRMT_IDC_VOTERLIST,0
							};


 int	BRtn;
 if (Message==WM_INITDIALOG)
 {  
 	UINT	pw=0, pn=1, pm=2; 
 	HWND	hwnd;
 	short	ii;
 	
 	InitDlgPrompts (hWndDlg);
 	while (nPrompts--)
 	{   
 		hwnd = GetDlgItem(hWndDlg,PrmtDat[pw]);
 		if (!hwnd)
 			ii=1;
	 	SetDlgPrompt (hwnd,PrmtDat[pn],PrmtDat[pm]);
	 	pw+=3;
	 	pn+=3; 
	 	pm+=3;
	}
 }
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
 	case WM_DESTROY: 
	 	InitDlgPrompts (0);    
	 	break;
    case WM_INITDIALOG:  
	{   
		short	nTabs;
		HANDLE	hList;
		LPSHORT	pTabs;
        
        InInit=TRUE;
    	ClearDlgPrompts (); 
    	hSaveBM = EnterBlockingWindow (hWndDlg);
		First = TRUE;
		SendDlgItemMessage (hWndDlg,IDC_AUTOHIGHLIGHT,BM_SETCHECK,AutoHighlight,0L);
		GetGlobalCVal ("[%VOTERDELIM]",str,","); 
		if (!_fstricmp (str,"\' \'"))
			NameDelim = ' ';
		else
			NameDelim = *str;					
		GetGlobalCVal ("[%VOTERTABS]",str,"170 2000");
		nTabs = GetIntsFromList (str,&hList); 
		if (nTabs)
		{
			pTabs = (LPSHORT)GlobalLock (hList);
			SendDlgItemMessage (hWndDlg,IDC_VOTERLIST,LB_SETTABSTOPS,nTabs,(LPARAM)pTabs);
			GSSiGlobUlFree (&hList);
		}
        cwCenter(hWndDlg, 0); 
	    if (!OpenDataFile (VoterDB[0], "",BT_READ, &hDBVoter[0]))
	    {
	        MessageBox( GetFocus(), str,"Cannot open voter database", MB_OK);
	    }  
	    if (!OpenDataFile (VoterDB[1], "",BT_READ, &hDBVoter[1]))
	    {
	        MessageBox( GetFocus(), str,"Cannot open voter database", MB_OK);
	    }  
	    SetDlgItemText (hWndDlg,IDC_VOTERNAMEFIRST,FirstName);            
	    SetDlgItemText (hWndDlg,IDC_VOTERNAMEMIDDLE,MiddleName);            
	    SetDlgItemText (hWndDlg,IDC_VOTERNAME,LastName);  
	    SetDlgItemText (hWndDlg,IDC_VOTERNAMEZIP,ZipCodeC);  
	    InInit=FALSE; 
	    if (VoterLocOpt)
	    { 
        	ShowWindow (GetDlgItem(hWndDlg,IDC_DISPLAYVOTERDATA),SW_SHOW); 
        	ShowWindow (GetDlgItem(hWndDlg,IDC_ZOOMTOVOTER),SW_SHOW); 
        	ShowWindow (GetDlgItem(hWndDlg,IDC_LOCATEOPTIONS),SW_SHOW);
			SendDlgItemMessage (hWndDlg,IDC_DISPLAYVOTERDATA,BM_SETCHECK,1,0L);
        } 
        PostMessage(hWndDlg, WM_COMMAND, IDC_STARTSEARCH, 0L);
//		PostMessage(GetDlgItem(hWndDlg,IDC_VOTERLIST), LB_SETCURSEL,iVoter,0); 
	} 

        break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(LOWORD(wParam))
           {
           	case IDC_STARTSEARCH:
           		goto DoSearch;
			case IDC_VOTERNAME:
			case IDC_VOTERNAMEFIRST:  
			case IDC_VOTERNAMEMIDDLE:    
			case IDC_VOTERNAMEZIP:
		    	 SetDlgItemText (hWndDlg,IDC_PROPMESSAGE,""); 
				 hwndCtl = (HWND) LOWORD(lParam);
                 switch (HIWORD(wParam))
                 {	case EN_CHANGE: 
                 	{   
                 DoSearch:  
						HaveLastName = 	GetDlgItemText (hWndDlg,IDC_VOTERNAME,str,42);
						if (!HaveLastName)
							break;
                 		EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
						SendDlgItemMessage (hWndDlg,IDC_VOTERLIST,LB_RESETCONTENT,0,0);
						NumDisplayedVoterIDs = 0;
						HaveFirstName = GetDlgItemText (hWndDlg,IDC_VOTERNAMEFIRST,FirstName,128);
						HaveMiddleName = GetDlgItemText (hWndDlg,IDC_VOTERNAMEMIDDLE,MiddleName,128);
						GetDlgItemText (hWndDlg,IDC_VOTERNAMEZIP,str,16);
						WantZip = atol (str);
                 		_fmemset (str,0,42);
           		      	SetDlgItemText (hWndDlg,IDC_VOTLOCMESSAGE,"");  
           		      	CurLoc = 0;
           		      	LocInc = 1;
           		      	GetDlgItemText (hWndDlg,IDC_VOTERNAME,str,42);  
           		      	OneSpace (FirstName);
           		      	OneSpace (MiddleName);
           		      	OneSpace (str);
           		      	n = _fstrlen (str);
           		      	InDisplay = TRUE;
           		      	if (n && !InInit)
           		      	{   
           		      		short	NameIndex=2, idb; 
       		      			MSG     msg;  
       		      			LPSTR	pComma; 
                            
							nFound = 0;
   		                    ShowWindow (GetDlgItem(hWndDlg,IDC_VOTERLOCSTATUS),SW_SHOW);  
   		                    
   		                    for (idb=0;idb<2;idb++)
   		                    {
	           		      		short	pos=BT_FIRST, cond=BT_GE, NameLen=0;  
	           		      		
                 				_fmemset (str,0,42);
		           		      	GetDlgItemText (hWndDlg,IDC_VOTERNAME,str,42);  
		           		      	OneSpace (str);
		           		      	n = _fstrlen (str);
	                            if (!_fstricmp (str,"*"))
	                            {
	                            	n = 0;
	                            	*str = 0;
	                            }
           		      			_fstrcpy (LastName,str);
	                            GetNext = TRUE;
							    SQLPtr = (LPOPENSQLDATA) GlobalLock (hDBVoter[idb]);
							    FilePtr = (LPOPENFILEDATA) GlobalLock (SQLPtr->OFHandle);
								lpGWDHead = (LPGWDHEADER)GlobalLock (FilePtr->FileHandle);                             
								for (i=0;i<abs(lpGWDHead->NumIndexFields[NameIndex]);i++)
									NameLen += lpGWDHead->pFldInfo[lpGWDHead->IndexFields[NameIndex][i]].Len;    
				        		while (ContinueProcessing && GetNext && !BT_FIND (lpGWDHead->BTHandle[NameIndex],(LPSTR)str,pos,cond,(LPSTR)&Offset))
				        		{
			        			    SetGWDCurrentOffset (lpGWDHead,-1);
				        			pos = BT_NEXT;
				        			cond = BT_ANY; 
				        			
				        			str[NameLen]=0;   
				        			Truncate (str);
				        			if (!_fstrnicmp (LastName,str,n))
				        			{   
				        				BOOL	ShowThisVoter = TRUE;
				        				
				        				if (HaveFirstName)
				        				{   
				        					LPSTR	WC=FirstName;
				        					
				        					GetDlgItemText (hWndDlg,IDC_VOTERNAMEFIRST,FirstName,128); 
				        					OneSpace (FirstName);
											pComma = &str[lpGWDHead->pFldInfo[lpGWDHead->IndexFields[NameIndex][0]].Len];
				        					while (WC)
				        					{   
				        						LPSTR	pEnd=_fstrchr (WC,',');
				        						BOOL	WildCard=FALSE;
				        						
				        						if (*WC == '*')
				        						{
				        							WildCard=TRUE;
				        							WC++;
				        						}
				        						if (pEnd)
				        							*pEnd++=0;
				        						if (WildCard)
				        						{
					        						if (!_fstrstr (pComma,WC))
					        							ShowThisVoter = FALSE;
					        					} 
					        					else
				        						{
					        						if (_fstrnicmp (pComma,WC,_fstrlen(WC)))
					        							ShowThisVoter = FALSE;
					        					}
				        						WC = pEnd;
				        					}
				        				}
				        				if (ShowThisVoter)
				        				{
											FillGWDData (lpGWDHead,Offset);                                 
								        	SQLPtr->st = 0;  
								        	SQLPtr->Offset = Offset; 
								        	SQLPtr->lastreadtime = ULONG_MAX;     
					        				if (HaveMiddleName)
					        				{   
					        					LPSTR	WC=MiddleName;
					        					
					        					GetDlgItemText (hWndDlg,IDC_VOTERNAMEMIDDLE,MiddleName,128); 
					        					OneSpace (MiddleName);  
					        					if (idb)
									        		_fstrcpy (str,"[V1.MiddleName]"); 
									        	else
									        		_fstrcpy (str,"[V0.MiddleName]");
									        	ExpandText (str);
	                                            pComma = str;
					        					if (*pComma) //no middle name skips middle name match
					        					while (WC)
					        					{   
					        						LPSTR	pEnd=_fstrchr (WC,',');
					        						BOOL	WildCard=FALSE;
					        						
					        						if (*WC == '*')
					        						{
					        							WildCard=TRUE;
					        							WC++;
					        						}
					        						if (pEnd)
					        							*pEnd++=0;
					        						if (WildCard)
					        						{
						        						if (!_fstrstr (pComma,WC))
						        							goto SkipVoter;
						        					} 
						        					else
					        						{
						        						if (_fstrnicmp (pComma,WC,_fstrlen(WC)))
						        							goto SkipVoter;
						        					}
					        						WC = pEnd;
					        					}
					        				}
					        				if (WantZip)
					        				{   
					        					if (idb)
									        		_fstrcpy (str,"[V1.ZIPCODE]");
									        	else
									        		_fstrcpy (str,"[V0.ZIPCODE]");
									        	ExpandText (str);
									        	if (WantZip != atol (str))
									        		goto SkipVoter;
									        }
									        if (idb)
								        		GetGlobalCVal ("[%VOTERDISPLAY]",str,"[V1.FIRSTNAME] [V1.MIDDLENAME] [V1.LASTNAME] [V1.NAMESUFFIX]$CHR(1)[V1.HOUSENUMBER] [V1.STREETNAME] [V1.CITY],[V1.ZIPCODE]$CHR(1)1$CHR(1)[V1.VOTERID]");
								        	else
								        		GetGlobalCVal ("[%VOTERDISPLAY]",str,"[V0.FIRSTNAME] [V0.MIDDLENAME] [V0.LASTNAME] [V0.NAMESUFFIX]$CHR(1)[V0.HOUSENUMBER] [V0.STREETNAME] [V0.CITY],[V0.ZIPCODE]$CHR(1)0$CHR(1)[V0.VOTERID]");
						        			ExpandText (str); 
						        			OneSpace (str); 
						        			ReplaceChar (str,0x01,0x09);
						        			
						        			pTAB = _fstrrchr (str,'\t');
						        			pTAB++;
						        			VoterID = atol (pTAB);
						        			if (idb)
						        			{
						        				UINT	i;
						        				
						        				for (i=0;i<NumDisplayedVoterIDs;i++)
						        					if (VoterID == DisplayedVoterIDs[i])
						        						goto SkipVoter;
						        			}
						        			else if (NumDisplayedVoterIDs < MAXDISPLAYEDVOTERS)
						        				DisplayedVoterIDs[NumDisplayedVoterIDs++] = VoterID;
							 				if ((idx=SendDlgItemMessage (hWndDlg,IDC_VOTERLIST,LB_ADDSTRING,0,(LPARAM)str)) ==
							 					LB_ERRSPACE) 
							 					ContinueProcessing = FALSE;
							 				else
							 				{   
							 					nFound++;
							 					sprintf (str,"%i voters found",nFound);
								        		SetDlgItemText (hWndDlg,IDC_VOTLOCMESSAGE,str);
								        	}
	  
						 			SkipVoter: 
						 					CurLoc += LocInc;
	 					             		PctBox (GetDlgItem(hWndDlg,IDC_VOTERLOCSTATUS), 1000, CurLoc,SHRT_MAX);
		                                    if (CurLoc == 1000)
		                                    	LocInc = -1;
		                                    else if (CurLoc == 0)
		                                    	LocInc = 1;
							 			} 
						 			}
						 			else
						 				GetNext = FALSE;
					 				if (GSSiPeekMessage(&msg,hwndCtl,WM_KEYDOWN,WM_KEYDOWN,PM_NOREMOVE))
					 					GetNext = FALSE;	                                                             
				        		}
				        		GlobalUnlock (FilePtr->FileHandle); 
				           		GlobalUnlock (SQLPtr->OFHandle);
				        		GlobalUnlock (hDBVoter[idb]);  
				        	} 
		             		PctBox (GetDlgItem(hWndDlg,IDC_VOTERLOCSTATUS), 0,0,SHRT_MIN);  
   		                    ShowWindow (GetDlgItem(hWndDlg,IDC_VOTERLOCSTATUS),SW_HIDE); 
			        		nFound = SendDlgItemMessage (hWndDlg,IDC_VOTERLIST,LB_GETCOUNT,0,0);
			        		if (ContinueProcessing)
			        			sprintf (str,"%i voters found",nFound);
			        		else
			        			_fstrcpy (str,"Search not complete");  
			        		if (nFound == 1)
			        		{
						        SendDlgItemMessage (hWndDlg,IDC_VOTERLIST,LB_SETCURSEL,(WPARAM)0,(LPARAM)0); 
			        			EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);  
			        		}
			        		ContinueProcessing = TRUE;
			        		SetDlgItemText (hWndDlg,IDC_VOTLOCMESSAGE,str);
                         }  
                         InDisplay = FALSE;
                         First = FALSE;
                 	}
                 }
                 break;
			     
            case IDC_VOTERLIST: 
                 switch(HIWORD(wParam))
                 {   
                 	 case LBN_SELCHANGE:  
                 	 	if (!InDisplay)
							PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
		 		 }
		 		 break;
		 	
                 	 	
		 	case IDOK:
		 	{   
				LPSTR	pTAB; 
				BOOL	Err=FALSE;    
				
				iVoter = SendDlgItemMessage(hWndDlg,IDC_VOTERLIST,LB_GETCURSEL,0,0);
		 		SendDlgItemMessage(hWndDlg,IDC_VOTERLIST,LB_GETTEXT,iVoter,(DWORD)str); 
		 		if ((pTAB = _fstrrchr (str,'\t')))
		 			*pTAB++ = 0;
				CurrentVoterID = atol(pTAB);
		 		if ((pTAB = _fstrrchr (str,'\t')))
		 			*pTAB++ = 0;
				CurrentVoterDB = atoi(pTAB);
				CloseDataFile (TRUE,&hDBVoter[0]);
				CloseDataFile (TRUE,&hDBVoter[1]);
				if (Err)
					break; 
			    GetDlgItemText (hWndDlg,IDC_VOTERNAME,LastName,sizeof(LastName)-1); 
			    GetDlgItemText (hWndDlg,IDC_VOTERNAMEFIRST,FirstName,sizeof(FirstName)-1);            
			    GetDlgItemText (hWndDlg,IDC_VOTERNAMEMIDDLE,MiddleName,sizeof(MiddleName)-1);            
			    GetDlgItemText (hWndDlg,IDC_VOTERNAMEZIP,ZipCodeC,sizeof(ZipCodeC)-1); 
			    VoterLocOpt = 1;
			    if (SendDlgItemMessage (hWndDlg,IDC_ZOOMTOVOTER,BM_GETCHECK,0,0)) 
			    	VoterLocOpt = 2;
 	            GSSiEndDialog(hWndDlg, VoterLocOpt,hSaveBM);  
			    //PostMessage(hWndMain, WM_COMMAND, IDM_Z_HLTLIMITS, 0L);
	            
	        }
		 	     break;   
		 	     
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */  
                 
                 if (GetNext)
                 {
                 	GetNext = FALSE;
                 	ContinueProcessing = FALSE;  
                 }
                 else 
                 {
					CloseDataFile (TRUE,&hDBVoter[0]); 
					CloseDataFile (TRUE,&hDBVoter[1]); 
					GSSiEndDialog(hWndDlg, FALSE,hSaveBM); 
				 }
                 break;
           }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL FAR PASCAL DISPLAYRECORDMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1055);
#endif
{ 
	int		TabStops[2]={100,1000}; 
    
 short    BRtn;
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1055);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG: 
       	SendDlgItemMessage (hWndDlg,IDC_FIELDS,LB_SETTABSTOPS,2,(LPARAM)TabStops);
    	BasicDataDisplay (DRDataFile,hWndDlg,IDC_FIELDS,IDENTIFY_NEXT,IDENTIFY_PRIOR,1,0,DRSQL,80);
        break; /* End of WM_INITDIALOG                                 */
    
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:

         switch(LOWORD(wParam))

         {  
            case IDCANCEL:
                 EndDialog(hWndDlg, TRUE); 
                 break;
                 
          }
          break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1055);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1055);
#endif
 return TRUE;    
}
#if ENABLETRACE
}
#endif
}
BOOL FAR PASCAL VOTER_NEWMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
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
					GetValFromOpenFiles ("LocationCode",str,256);
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
				irc = AddMatchSingle (str,City,ZipCodeC,OutAddress,&NewCoord,&Match,0);
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
						if (GetTrueStreetName (Match.StreetNum, Street, 0,0))
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
			    time_t	Time; 
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

BOOL FAR PASCAL VOTER_DEFINE_USER_DATAMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
 int	BRtn;  
 char	str[256];
 static	short	i;   
 static	int	CurrentTXi, CurrentLBi, CurrentCBi;
 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
    	 LoadVoterFields (hWndDlg); 
	case GSSI_REINITDIALOG:  
		 CurrentTXi = CurrentCBi = CurrentLBi = -1;
		 if (NumUserCheckbox < MaxUserCheckbox) 
		 {
			ShowWindow (GetDlgItem(hWndDlg,USERCBCNTL[NumUserCheckbox]),SW_SHOW);  
			*UserCheckboxTitle[NumUserCheckbox] = 0; 
			*UserCheckboxFieldName[NumUserCheckbox] = 0; 
		 }
		 if (NumUserLists < MaxUserLists) 
		 {
			ShowWindow (GetDlgItem(hWndDlg,USERLBCNTL[NumUserLists]),SW_SHOW);  
			ShowWindow (GetDlgItem(hWndDlg,USERLBCNTLT[NumUserLists]),SW_SHOW);
			*UserListTitle[NumUserLists] = 0;
			*UserListFieldName[NumUserLists] = 0;
		 } 
		 if (NumUserTextbox < MaxUserTextbox) 
		 {
			ShowWindow (GetDlgItem(hWndDlg,USERTXCNTL[NumUserTextbox]),SW_SHOW);  
			ShowWindow (GetDlgItem(hWndDlg,USERTXCNTLT[NumUserTextbox]),SW_SHOW); 
			*UserTextboxTitle[NumUserTextbox] = 0; 
			*UserTextboxFieldName[NumUserTextbox] = 0; 
		 } 
         EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:   
    	
    	 for (i=0;i<MaxUserCheckbox;i++)
    	 {
    	 	if (wParam == USERCBCNTL[i])
    	 	{
             	if (i == CurrentCBi)
             			break;  
             	CurrentCBi = i;
            	if (GetUserFieldTitleAndName (hWndDlg,UserCheckboxTitle[i],UserCheckboxFieldName[i],
            		(CurrentCBi == NumUserCheckbox),0,0,0))
            	{
            		SetDlgItemText (hWndDlg,USERCBCNTL[CurrentCBi],UserCheckboxTitle[CurrentCBi]);
            		if (CurrentCBi == NumUserCheckbox) 
            			NumUserCheckbox++;
					PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
            	}  
            	else
            		CurrentCBi = -1;
    	 	}
    	 }
    	 for (i=0;i<MaxUserLists;i++)
    	 {
    	 	if (wParam == USERLBCNTL[i])
    	 	{
                 switch (HIWORD(wParam))
                 {  
                 	case WM_SETFOCUS:
                 	
                 	if (i == CurrentLBi)
                 			break;  
                 	CurrentLBi = i;
	            	if (GetUserFieldTitleAndName (hWndDlg,UserListTitle[CurrentLBi],UserListFieldName[CurrentLBi],
	            		(CurrentLBi == NumUserLists),&UserListMaxValueLength[CurrentLBi],&UserListValues[CurrentLBi],&UserListNumValues[CurrentLBi]))
	            	{
	            		SetDlgItemText (hWndDlg,USERLBCNTLT[CurrentLBi],UserListTitle[CurrentLBi]); 
	            		if (CurrentLBi == NumUserLists)
	            			NumUserLists++;
						PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
	            	}
	            	else
	            		CurrentLBi = -1;
	            	break;
                 }  
    	 	}
    	 }
    	 for (i=0;i<MaxUserTextbox;i++)
    	 {
    	 	if (wParam == USERTXCNTL[i])
    	 	{    
    	 		WORD	HW=HIWORD(wParam);
                 switch (HW)
                 {  
                 	case EN_SETFOCUS:  
                 		if (i == CurrentTXi)
                 			break; 
                 	CurrentTXi = i;
	            	if (GetUserFieldTitleAndName (hWndDlg,UserTextboxTitle[i],UserTextboxFieldName[i], 
	            								 (CurrentTXi == NumUserTextbox),&UserTextboxMaxValueLength[CurrentTXi],0,0))
	            	{
	            		SetDlgItemText (hWndDlg,USERTXCNTLT[CurrentTXi],UserTextboxTitle[CurrentTXi]); 
	            		if (CurrentTXi == NumUserTextbox)
	            			NumUserTextbox++; 
	            	
						PostMessage(hWndDlg, GSSI_REINITDIALOG, 0, 0L);
	            	} 
	            	else
	            		CurrentTXi = -1;
	            	break;
                 }  
    	 	}
    	 }
         switch(LOWORD(wParam))
         {
			
			case IDC_BUTTON2:
				break;
         	case IDOK:
         	{   
         		SaveVoterFieldDefs ();
                EndDialog(hWndDlg, TRUE);   
            }
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

BOOL FAR PASCAL FIELD_TITLE_AND_NAMEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
 int	BRtn;  
 char	str[256];
 short	i,j; 
 LPSTR	pValue;
 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
         cwCenter(hWndDlg, -1); 
         SetDlgItemText (hWndDlg,IDC_USERPROMPT,NewUserPrompt);
         SetDlgItemText (hWndDlg,IDC_USERFIELD,NewUserFieldName);
         if (!NewUserFieldIsNew)  
             EnableWindow (GetDlgItem(hWndDlg,IDC_USERFIELD),FALSE);
		 if (pNewUserFieldMaxValueLength)
		 {
		 	 ShowWindow (GetDlgItem(hWndDlg,IDC_MAXVALUELENGTH),SW_SHOW);  
		 	 ShowWindow (GetDlgItem(hWndDlg,IDC_MAXVALUELENGTHT),SW_SHOW);  
		 	 itoa (*pNewUserFieldMaxValueLength,str,10);
		 	 SetDlgItemText (hWndDlg,IDC_MAXVALUELENGTH,str);
	         if (!NewUserFieldIsNew)  
	             EnableWindow (GetDlgItem(hWndDlg,IDC_MAXVALUELENGTH),FALSE);
		 }  
		 if (phNewUserFieldValueList)
		 {
		 	 ShowWindow (GetDlgItem(hWndDlg,IDC_VALUELIST),SW_SHOW);  
		 	 ShowWindow (GetDlgItem(hWndDlg,IDC_NEWVALUE),SW_SHOW);  
		 	 ShowWindow (GetDlgItem(hWndDlg,IDC_ADDNEWVALUE),SW_SHOW);
		 	 ShowWindow (GetDlgItem(hWndDlg,IDC_VALUELISTGROUP),SW_SHOW);
		 	 if (*pNewUserNumValues)
		 	 {
		 	 	pValue = GlobalLock (*phNewUserFieldValueList);
		 	 	for (i=0;i<*pNewUserNumValues;i++,pValue+=(*pNewUserFieldMaxValueLength+1))
				{
					SendDlgItemMessage (hWndDlg,IDC_VALUELIST,LB_ADDSTRING,0,(LPARAM)pValue);
		 	 	}
				GlobalUnlock (*phNewUserFieldValueList);
		 	 }  
         }

         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND: 
         switch(LOWORD(wParam))
         {  
         	case IDC_USERPROMPT:
                 switch (HIWORD(wParam))
                 {  
                 	case EN_KILLFOCUS:
         				if (NewUserFieldIsNew && !GetDlgItemText (hWndDlg,IDC_USERFIELD,str,255))  
         				{
         					GetDlgItemText (hWndDlg,IDC_USERPROMPT,str,32);
         					Strip (str,' ');
         					SetDlgItemText (hWndDlg,IDC_USERFIELD,str);
         				}
         		 }	
         	break;
         	
         	case IDOK:
         	{   
         		GWFLDINFO NewField;
         		
         		if (GetDlgItemText (hWndDlg,IDC_USERPROMPT,NewUserPrompt,64) &&
         			GetDlgItemText (hWndDlg,IDC_USERFIELD,NewUserFieldName,64))
         		{   
			        _fstrcpy (NewField.Name,NewUserFieldName);  
			        NewField.Len = 2;
			        NewField.Type = BT_INTEGER;
         			if (pNewUserFieldMaxValueLength)
         			{
         				GetDlgItemText (hWndDlg,IDC_MAXVALUELENGTH,str,32);
         				*pNewUserFieldMaxValueLength = atoi (str);
         				if (!*pNewUserFieldMaxValueLength || *pNewUserFieldMaxValueLength > 255)
         				{
         					MessageBox (hWndDlg,"Invalid maximum length",0,MB_ICONEXCLAMATION);
         					break;
         				}
				        NewField.Len = *pNewUserFieldMaxValueLength;
				        NewField.Type = BT_CHAR;
         			}
					if (phNewUserFieldValueList)
					{    
						*pNewUserNumValues = SendDlgItemMessage (hWndDlg,IDC_VALUELIST,LB_GETCOUNT,0,0);
						if (!*pNewUserNumValues)
         				{
         					MessageBox (hWndDlg,"No values defined",0,MB_ICONEXCLAMATION);
         					break;
         				}
						 if (!NewUserFieldIsNew)
						 	GSSiGlobFree (phNewUserFieldValueList);  
           				 *phNewUserFieldValueList=GSSiGlobAlloc (0,GMEM_MOVEABLE,4+(1+(long)*pNewUserFieldMaxValueLength)*(*pNewUserNumValues));
						 pValue = GlobalLock (*phNewUserFieldValueList);
						 for (j=0;j<*pNewUserNumValues;j++,pValue+=(*pNewUserFieldMaxValueLength+1))
						 {
							SendDlgItemMessage (hWndDlg,IDC_VALUELIST,LB_GETTEXT,j,(LPARAM)pValue);
						 }
						 GlobalUnlock (*phNewUserFieldValueList);
			        } 
			        if (NewUserFieldIsNew)
			        {
				        if (GWDAddField (VoterDB[0],&NewField)) 
				        {
				        	sprintf (str,"Field %s successfully added to %s",NewUserFieldName,VoterDB[0]);
	                    	MessageBox (hWndDlg,str,"Field added",MB_OK);   
	                    	if (MessageBox (hWndDlg,"Do you want to add an entry to the point layer list based on this field?","",MB_YESNO)
	                    		== IDYES)
	                    	{
	                    		AddFieldToPointList (hWndDlg,NewField,phNewUserFieldValueList);
	                    	}
	                    }
	                    else
	                    {   
				        	sprintf (str,"Unable to add field %s to %s",NewUserFieldName,VoterDB[0]);
	                    	MessageBox (hWndDlg,str,0,MB_ICONEXCLAMATION); 
	                    	break;
				        }
				    }
                	EndDialog(hWndDlg, TRUE); 
                }  
            }
                break; 
                
            case IDC_ADDNEWVALUE:  
            	if (GetDlgItemText (hWndDlg,IDC_NEWVALUE,str,255))
					SendDlgItemMessage (hWndDlg,IDC_VALUELIST,LB_ADDSTRING,0,(LPARAM)str);
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

BOOL FAR PASCAL ADDCBFIELDTOPOINTLISTMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
 int	BRtn;  
 char	str[256], LayerText[130];
 short	i,j; 
 LPSTR	pValue;
 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:  
         cwCenter(hWndDlg, -1);
         sprintf (str,"%s colored by Age with Gender",NewUserPrompt); 
         SetDlgItemText (hWndDlg,IDC_NEWPOINTLISTPROMPT,str);
         SetDlgItemText (hWndDlg,IDC_NEWPOINTLISTCOLORTHEME,"Voter Age");
         SetDlgItemText (hWndDlg,IDC_NEWPOINTLISTOTHERTHEME,"Voter Gender");

         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND: 
         switch(wParam)
         {  
         	case IDOK:
         	{   
         		{
	         		char	File[] = "[%DL]MACROS\\setpointlayer.txt";
	         		short	NextID = Insert ("BEGIN",File,0);  
	         		
	         		if (NextID)
	         		{
						sprintf (str,"IF([%%ARG(1)]==%i)",NextID);
						Insert (0,0,str);
						Insert (0,0,"{"); 
						GetDlgItemText (hWndDlg,IDC_NEWPOINTLISTPROMPT,LayerText,128);
						sprintf (str,"\t[CURRENTPOINTLAYER]=%s;",LayerText);
						Insert (0,0,str);
						Insert (0,0,"\t$VIS(LAYER:Camp Voters,1,Point Data);");
						Insert (0,0,"\t$VP(SHOW,Point Data);");
						sprintf (str,"\t[POINTFILTER]=$STR(VoterID = [%%INT_REFNO] && %s==1);",NewUserFieldName);
						Insert (0,File,str);
						Insert (0,0,"\t$VP(SHOW,Point Filter);");
						Insert (0,0,"\t$VP(SHOW,Voter Age);");
						Insert (0,0,"\t$VP(SHOW,Voter Gender);");
						Insert (0,0,"\t$VP(SHOW,Point Totals);");
						Insert (0,0,"};");
						Insert ("END",File,0);
					} 
				} 
         		{
	         		char	File[] = "[%DL]MENUS\\pointlayer.txt";
	         		short	NextID = Insert ("BEGIN",File,0);  
	         		
	         		if (NextID)
	         		{
					    sprintf (str,"MENUITEM \"%s\",\"[%C]=$MACRO([%DL]macros\\setpointlayer.txt,%i);\"",LayerText,NextID);
						Insert (0,0,str);
						Insert ("END",File,0);
					} 
				} 

               	EndDialog(hWndDlg, TRUE); 
            }
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




     

