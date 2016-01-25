#include "graphint.h"  
#include "crypto32.h"

#include "gmextern.h"
#include <shlobj.h>

static long	RemDays;
static DWORD	expnot=851349;

extern short	TrialDays;
extern BOOL		DoReset;
extern BOOL		CheckForLicense;

static	int		Types[]={CSIDL_PERSONAL,CSIDL_FAVORITES,CSIDL_DESKTOP,CSIDL_LOCAL_APPDATA,CSIDL_COMMON_APPDATA};
static	int		numtypes=5;

DWORD	DID[3];

BOOL StoreSerialNumber (LPSTR SN)
{
	char	sc[66], c[66];
	HKEY	hKeysw,hKeylm,hKeyap,hKeyreg;
	DWORD	Disp;
	int		rc=1;
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,"SOFTWARE",0,KEY_READ,&hKeysw) == ERROR_SUCCESS)
	{
		if (RegCreateKeyEx  (hKeysw,"GSSiGeoMaster",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&hKeylm,&Disp) == ERROR_SUCCESS)
		{
			char	ModID[64];

			sprintf (ModID,"%s",MODULEIDSTRING);
			if (RegCreateKeyEx (hKeylm,ModID,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&hKeyap,&Disp) == ERROR_SUCCESS)
			{
				if (RegCreateKeyEx (hKeyap,"Registration",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&hKeyreg,&Disp) == ERROR_SUCCESS)//REG_CREATED_NEW_KEY
				{
					rc=RegSetValueEx (hKeyreg,"Serial Number",0,REG_SZ,SN,strlen(SN)+1);
				}
			}
		
		}
	}
	if (rc == NO_ERROR)
		return TRUE;
	else
		return FALSE;
} 

int abc1 (void) //check for existing eval start date
{
	return 0;
}

BOOL abc6 (DWORD time) //checks all sources of time for consistency
{
	char	FileName[32];
	char	Path[MAX_PATH];
	DWORD	time2;
	BOOL	HaveTime = FALSE;
	int		i, rc;
	HFILE	Fid;

//	strcpy (Path,"C:\\Program Files\\LakeMaster\\Contour Pro Minnesota\\address");
//	GetDirectoryCreateTime (Path,&CurLowTime,&CurHighTime,&TimeDiff);

	sprintf (FileName,"%s.gtf",MODULEIDSTRING);
	for (i=0;i<numtypes;i++)
	{
		*Path = 0;
		if ((rc=SHGetFolderPath (0,Types[i],0,SHGFP_TYPE_CURRENT,Path)) == S_OK)
		{
			sprintf (strchr(Path,0),"\\%s",FileName);
			Fid = GSSiOpenFile (Path,0,OF_READ);
			if (Fid != HFILE_ERROR)
			{
				BigRead (Fid,&time2,4);
				GSSiClose (Fid);
				if (time != time2)
					return FALSE;
				else
					HaveTime = TRUE;
			}
		}
	}
	return HaveTime;
}
BOOL abc7 (DWORD time) //creates mult sources of time
{
	char	FileName[32];
	char	Path[MAX_PATH];
	DWORD	time2;
	BOOL	HaveTime = FALSE;
	int		i, ii,rc;
	HFILE	Fid;

//	strcpy (Path,"C:\\Program Files\\LakeMaster\\Contour Pro Minnesota\\address");
//	GetDirectoryCreateTime (Path,&CurLowTime,&CurHighTime,&TimeDiff);

	sprintf (FileName,"%s.gtf",MODULEIDSTRING);
	for (i=0;i<numtypes;i++)
	{
		*Path = 0;
		if ((rc=SHGetFolderPath (0,Types[i],0,SHGFP_TYPE_CURRENT,Path)) == S_OK)
		{
			sprintf (strchr(Path,0),"\\%s",FileName);
			Fid = GSSiOpenFile (Path,0,OF_CREATE);
			if (Fid != HFILE_ERROR)
			{
				BigWrite (Fid,&time,4,-1);
				GSSiClose (Fid);
				if (!SetFileAttributes(Path,FILE_ATTRIBUTE_HIDDEN))
					ii=1;
			}
		}
	}
	return TRUE;
}

int ab(char	c)
{
	char str[4];
	str[0] = c;
	str[1] = 0;

	return atoi (str);
}

void abc4 (LPSTR cd,LPSTR Mod,LPSTR SerialNumber,LPDWORD VSN) //gets serialization code from mod sn and did
{
	char	SN[12];
	char	str[32], ProcID[128];
	int		tot=0,i;
	HKEY	hKeyhw,hKeyd,hKeys,hKeycp,hKeyz;
	int		lValue;
	DWORD	Type;

	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,"HARDWARE",0,KEY_READ,&hKeyhw) == ERROR_SUCCESS)
	{
		if (RegOpenKeyEx (hKeyhw,"DESCRIPTION",0,KEY_READ,&hKeyd) == ERROR_SUCCESS)
		{
			if (RegOpenKeyEx (hKeyd,"System",0,KEY_READ,&hKeys) == ERROR_SUCCESS)
			{
				if (RegOpenKeyEx (hKeys,"CentralProcessor",0,KEY_READ,&hKeycp) == ERROR_SUCCESS)
				{
					if (RegOpenKeyEx (hKeycp,"0",0,KEY_READ,&hKeyz) == ERROR_SUCCESS)
					{
						lValue = 127;
						if (RegQueryValueEx(hKeyz,"ProcessorNameString",0,&Type,ProcID,&lValue)!=ERROR_SUCCESS)
							*ProcID = 0;
						RegCloseKey (hKeyz);
					}
					RegCloseKey (hKeycp);
				}
				RegCloseKey (hKeys);
			}
			RegCloseKey (hKeyd);
		}
		RegCloseKey (hKeyhw);
	}
	strncpy (SN,SerialNumber,11);
	cd[0]=SN[0];
	cd[1]='A'+ab(Mod[0]);
	cd[2]='A'+ab(Mod[1]);
	cd[3]='A'+ab(Mod[2]);
	cd[4]='A'+ab(Mod[3]);
	cd[5]='-';
	cd[6]=SN[1];
	cd[7]='A'+ab(SN[2]);
	cd[8]='A'+ab(SN[3]);
	cd[9]='A'+ab(SN[4]);
	cd[10]='A'+ab(SN[5]);
	cd[11]='-';
	sprintf (str,"%ld",VSN[0]);
	cd[12]='A'+ab(str[0]);
	cd[13]='A'+ab(str[1]);
	cd[14]='A'+ab(str[2]);
	cd[15]='A'+ab(str[3]);
	cd[16]='A'+ab(str[4]);
	cd[17]='-';
	cd[18]='A'+ab(str[5]);
	cd[19]='A'+ab(str[6]);
	cd[20]='A'+ab(str[7]);
	cd[21]='A'+ab(str[8]);
	cd[22]='A'+ab(str[9]);
	cd[23]='-';
	sprintf (str,"%ld%ld",VSN[2],VSN[1]);
	cd[24]='A'+ab(SN[6]);
	cd[25]='A'+ab(SN[7]);
	cd[26]='A'+ab(SN[8]);
	cd[27]='A'+ab(SN[9]);
	cd[28]='A'+ab(SN[10]);
	cd[29]=0;
	for (i=0;i<29;i++)
		tot += cd[i];
	cd[14] = 'A' + tot%25;
	tot = 0;
	for (i=0;i<strlen(ProcID);i++)
		tot += ProcID[i];
	cd[22] = 'A' + tot%25;
	return;
}

void abc5 (LPSTR ac,LPSTR sc) //convert serial code to activation code
{
	int i,cv[29]={4,22,1,18,13,5,14,21,2,9,15,11,27,20,3,28,6,17,8,12,16,0,19,23,25,24,10,7,26};

	for (i=0;i<29;i++)
		ac[i] = 'B' + (sc[cv[i]]-'A');
	ac[5]=ac[11]=ac[17]=ac[23]='-';
	ac[29]=0;
	return;
}

/*void abc3 (LPSTR cd,LPSTR Mod,LPSTR SerialNumber,LPBYTE VSN) //gets access code from mod sn and did
{
	short	i;
	int		j=atoi (Mod), s=atoi(SerialNumber);
	DWORD	vsn[12];

	for (i=0;i<12;i++)
	{
		vsn[i] = (VSN[i] + j) * s;
		cd[i] = vsn[i] % 26 + 'A';
	}
	cd[i] = 0;
	return;
}*/

int abc (int i)
{
	char	Path[MAX_PATH], FileName[34];
	int		ii,index=0,x=0;
	DWORD	rc;
	DWORD	LowTime,HighTime,TimeDiff,CurLowTime,CurHighTime,AboutOneWeek=1000;
	char	SubKeyName[256];
	FILETIME	FileTime;
	DWORD	SKSize=sizeof(SubKeyName),Disp;
	HKEY	hKeysw,hKeylm,hKeyap,hKeyreg;
	DWORD	VSN,MXFN,FSOPTS;
	char	str[256],str2[256], ac[32], mess[1024];
	SYSTEM_INFO	SysInfo;
	ULARGE_INTEGER MySpace,TotSpace,FreeSpace;
	
	//return TRUE;
	GetSystemInfo (&SysInfo);
	if (!GetVolumeInformation("c:\\",str,256,&VSN,&MXFN,&FSOPTS,str2,256))
		VSN = 0;
	rc = GetDiskFreeSpaceEx ("c:\\",&MySpace,&TotSpace,&FreeSpace);
	DID[0] = VSN;
	DID[1] = TotSpace.LowPart;
	DID[2] = TotSpace.HighPart;
//	abc4 (str,MODULEIDSTRING,"12345",(LPBYTE)DID);
//	abc5 (ac,str);
//	abc3 (str,MODULEIDSTRING,"12345",(LPBYTE)DID);
	//return 1;
/*	while (RegEnumKeyEx(HKEY_LOCAL_MACHINE,index,SubKeyName,&SKSize,NULL,NULL,NULL,&FileTime)!=ERROR_NO_MORE_ITEMS)
	{
		SKSize=sizeof(SubKeyName);
		index++;
	}*/
	//check for registration key
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,"SOFTWARE",0,KEY_READ,&hKeysw) == ERROR_SUCCESS)
	{
		if (RegCreateKeyEx  (hKeysw,"GSSiGeoMaster",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&hKeylm,&Disp) == ERROR_SUCCESS)
		{
			char	ModID[64];

			sprintf (ModID,"%s",MODULEIDSTRING);
			if (RegCreateKeyEx (hKeylm,ModID,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&hKeyap,&Disp) == ERROR_SUCCESS)
			{
				if (RegCreateKeyEx (hKeyap,"Registration",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&hKeyreg,&Disp) == ERROR_SUCCESS)//REG_CREATED_NEW_KEY
				{
					int	i=0;
					char	VName[256];
					int		lVName=sizeof(VName);
					char	Value[256];
					int		lValue=lnSerialNumber;
					DWORD	Type, Time;
					if ((rc=RegQueryValueEx(hKeyreg,"Serial Number",0,&Type,SerialNumber,&lValue))==ERROR_SUCCESS)
					{
						lValue = lnAccessCode;
						if ((rc=RegQueryValueEx(hKeyreg,"Access Code",0,&Type,AccessCode,&lValue))==ERROR_SUCCESS)
						{
							char	cd[66];

							abc4 (cd,MODULEIDSTRING,SerialNumber,DID);
							if (strcmp (cd,AccessCode))
								x = 1;
						}
					}
					else if ((rc=RegQueryValueEx(hKeyreg,"Data1",0,&Type,(LPSTR)&Time,&lValue))==ERROR_SUCCESS)
					{
						time_t	now;
						int		Days;

						if (!abc6 (Time))
							RemDays = 0;
						else
						{
							time (&now);
							now -= 119600120;
							Days = (now - Time) / (60*60*24);
							RemDays = TrialDays - Days;
						}
						x = 0;
					}
					else
					{
						int	i=0;
						char	VName[256];
						int		lVName=sizeof(VName);
						char	Value[256];
						int		lValue=sizeof(Value);
						DWORD	Type;
						time_t	systime;
						
						time (&systime);
						systime -= 119600120;
						rc=RegSetValueEx (hKeyap,"Application",0,REG_SZ,MODULENAME,strlen(MODULENAME)+1);
						rc=RegSetValueEx (hKeyreg,"Data1",0,REG_DWORD,(LPSTR)&systime,4);
						abc7 (systime);
						RemDays = TrialDays;
						x=0;
					}

 					RegCloseKey (hKeyreg);
				}
				else
				{
					GetSystemErrMessage (GetLastError(),mess);
					MessageBox (0,mess,"Creating Registration",MB_ICONEXCLAMATION);
				}
				RegCloseKey (hKeyap);
			}
			else
			{
				GetSystemErrMessage (GetLastError(),mess);
				MessageBox (0,mess,"Creating ModuleID",MB_ICONEXCLAMATION);
			}
			RegCloseKey (hKeylm);
		}
		else
		{
		    GetSystemErrMessage (GetLastError(),mess);
			MessageBox (0,mess,"Creating GSSiGeoMaster",MB_ICONEXCLAMATION);
		}

		RegCloseKey (hKeysw);
		return x;
	}
	else
	{
	    GetSystemErrMessage (GetLastError(),mess);
		MessageBox (0,mess,"Opening HKEY_LOCAL_MACHINE",MB_ICONEXCLAMATION);
	}
	return 0;
}

BOOL ValidSerialNum (LPSTR Product,LPSTR SerialNum)
{
/*	long	n, Sno = atol (SerialNum);
	
	if (!_fstricmp (SerialNum,"BTEST"))
		return TRUE; 
	for (n = 10003; n<99999; n+=13)
		if (n == Sno)
			return TRUE;
	GSSiMsgBox (GetFocus(),"This is not a valid SportMap Serial Number - Please try again",NULL,MB_ICONEXCLAMATION);
	*/
	return FALSE;                                                                                        
}

BOOL ValidateLicense(HWND hWnd)
{ 
  BOOL	nRc=TRUE;
  DLGPROC lpfnLICENSEMsgProc;  
  char	IniName[132], Serno[32]="", str[128];   
  
//  DWORD TR = GetTimerResolution();
  
  if (!CheckForLicense)
  	return TRUE; 
  if (abc(1))
	  return TRUE;
  _fullpath (IniName,"gmapp.ini",255);
//  GetPrivateProfileString ("Install","AppName","GeoMaster",AppName,sizeof(AppName),IniName); 
  if (!_fstrcmp (AppName,"Sportmap"))
  { 
  	do 
		if (!GetTextString (hWnd,Serno,-32,"Please enter the SportMap Serial Number located on the inside cover",NULL,NULL,0,TRUE,TRUE))
			return FALSE;   
	while (!ValidSerialNum ("SportMap",Serno));
	WritePrivateProfileString ("SportMap","SerialNum",Serno,"geomastr.ini"); 
	sprintf (str,"SN%s",Serno);
	WritePrivateProfileString ("SportMap","OpenOrder",str,"geomastr.ini"); 
	WritePrivateProfileString ("Install","AppName","SportMap",IniName);
	return TRUE; 
  }
//  return TRUE;
  lpfnLICENSEMsgProc = MakeProcInstance((DLGPROC)LICENSEMsgProc, hInst);
  nRc = DialogBox(hInst, (LPSTR)"LICENSE", hWnd, lpfnLICENSEMsgProc);
  FreeProcInstance(lpfnLICENSEMsgProc); 
  
  return nRc;
}  

BOOL GetSerialCode (LPSTR Code)
{
/*	char	p[6][8];
	int i=GetEncryptedInfo(p[0],p[1],p[2],p[3],p[4],p[5]);  
	int	n=0;
			    
    sprintf (Code,"%s",p[0]);   
    for (i=1;i<6;i++)
    {
    	if (!_fstricmp (p[i-1],p[i]))
    		n++;
    	else
    	{
    		if (n)
    			sprintf (_fstrchr(Code,0),"-%i",n+1);
    		n=0;
			sprintf (_fstrchr(Code,0),"-%s",p[i]);
    	}
    }
	if (n)
		sprintf (_fstrchr(Code,0),"-%i",n+1);
    _fstrupr (Code); 
    REPLAC (Code,"O","$",250);*/
	SYSTEM_INFO	SysInfo;
	DWORD	VSN=0,MXFN,FSOPTS;
	ULARGE_INTEGER MySpace,TotSpace,FreeSpace;
	int	rc;
	char	str[256], str2[256];
	
	//return TRUE;
	GetSystemInfo (&SysInfo);
	if (!GetVolumeInformation("c:\\",str,256,&VSN,&MXFN,&FSOPTS,str2,256))
		VSN = 0;
	rc = GetDiskFreeSpaceEx ("c:\\",&MySpace,&TotSpace,&FreeSpace);
	DID[0] = VSN;
	DID[1] = TotSpace.LowPart;
	DID[2] = TotSpace.HighPart;
//	abc4 (str,MODULEIDSTRING,"13579",(LPBYTE)DID);
	abc4 (Code,MODULEIDSTRING,SerialNumber,DID); //gets serialization code from mod sn and did
	return TRUE;

}  

BOOL EnterAccessCode (HWND hWnd,LPSTR Code)
{
/*	char	p[6][8];
//	char	c[64];
	LPSTR	subc,last;
	short	i,n=0; 
	DWORD ru, fe; 
				
	_fmemset (p,0,sizeof(p));
            	
	subc = Code;   
	_fstrupr (Code);
	ReplaceChar (Code,'O','0');
	ReplaceChar (Code,'$','O');
            	
	while (subc)
	{
		short	nlast=n-1;

		last = subc;
		subc = _fstrchr (subc,'-');
		if (subc)
			*subc++ = 0; 
		if (_fstrlen (last) == 1)
		{   
			i=atoi(last)-1;
			while (i--)
				_fstrcpy (p[n++],p[nlast]);
		}
		else
			_fstrcpy (p[n++],last);
	}
	if (n != 6)
		return FALSE;
	i=SetActivationCode(p[0],p[1],p[2],p[3],p[4],p[5]);
	ru = GetRunsCount();
	fe = GetDateCount();
	if((LicenseIntegrityCheck() == 1) && (ru > 0))
	{   
		UpdateRandom();
        return TRUE;
	} */
	char	sc[66], c[66];
	HKEY	hKeysw,hKeylm,hKeyap,hKeyreg;
	DWORD	Disp;
	int		rc=1;
	GetSerialCode (sc);
	abc5 (c,sc); 
	if (strcmp (c,Code))
		return FALSE;
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,"SOFTWARE",0,KEY_READ,&hKeysw) == ERROR_SUCCESS)
	{
		if (RegCreateKeyEx  (hKeysw,"GSSiGeoMaster",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&hKeylm,&Disp) == ERROR_SUCCESS)
		{
			char	ModID[64];

			sprintf (ModID,"%s",MODULEIDSTRING);
			if (RegCreateKeyEx (hKeylm,ModID,0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&hKeyap,&Disp) == ERROR_SUCCESS)
			{
				if (RegCreateKeyEx (hKeyap,"Registration",0,NULL,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,0,&hKeyreg,&Disp) == ERROR_SUCCESS)//REG_CREATED_NEW_KEY
				{
					rc=RegSetValueEx (hKeyreg,"Access Code",0,REG_SZ,Code,strlen(Code)+1);
				}
			}
		
		}
	}
	if (rc == NO_ERROR)
		return TRUE;
	else
	{
		MessageBox (hWnd,"Unable to update registry",0,MB_ICONEXCLAMATION);
		return FALSE;
	}
} 

void AddTextToClipboard (LPSTR txt)
{
	HANDLE	hStr = GSSiGlobAlloc (0,GMEM_MOVEABLE,strlen(txt)+256);
	LPSTR	str = GlobalLock (hStr);

	sprintf (str,"$TEXTTOCLIPBOARD(%s)",txt);
	ProcessText (str);
	GSSiGlobUlFree (&hStr);
	return;
}

 BOOL FAR PASCAL LICENSEMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{
	
	UINT	PrevErrMode;
	char	UserID[64], Password[32], mess[256], str[260], VolLabel[32];
	DWORD ru, fe, fech, usr, actusr;
	static	BOOL	OKtoGo;
	short	i;	
	int		StartTrial;

 int	BRtn;
 if (WSAIsBlocking ()) 
 {
	SetCursor (LoadCursor (0,IDC_WAIT));  
 	return TRUE;
 }
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
 	return (BRtn);
 switch(Message)
   {
    case WM_INITDIALOG:
    if (!App)
       	ShowWindow (GetDlgItem(hWndDlg,IDC_REGNOW),SW_HIDE);
ReCheck:		  
    {  
    
			
			int	HPI=0, ii;
			
			//Initialize your own private values
			//-----------------------------------
			//Your Serialization Private Key
			DWORD SC = 802519233;
			//Your Activation Private Key
			DWORD AC = 721063029;
			//Your main control file string
			//Maximum runs allowed
			DWORD MR = 10;
			//Maximum days allowed
			DWORD MD = expnot;
			//Your own key value for NO EXPIRATION
			//--------------------------------------
			
			//Here you set your Private Keys for Serialization and
			//Activation codes as well as the string to create your
			//own Main Control File
			OKtoGo = TRUE;
			//ii=SetGlobals(SC, AC, MODULEIDSTRING);
			
		
			// If you want to allow your application to run if it's never
			//been installed before (Trialware/Demo), then implement this code:
			//Check for DEMO Mode then set Days and Runs as needed as well as
			//the number of allowed users (optional for network environments)  
			//ii=GetDemoMask();
			//ii=ControlFileExists();
//			if((GetDemoMask() == 0) && (ControlFileExists() != 1))
/*			if((ControlFileExists() != 1))
				{
					ii=CreateControlFile();
					ii=SetDemoMask();     
					ii=SetRunsCount(AC, 10);
					ii=SetDateCount(AC, TrialDays);
					ii=SetUsersCount(AC, 1);
				}*/
		
		
			//Check for existence of Control File
/*			if(ControlFileExists() != 1)
				ii=CreateControlFile();
		    else if (DoReset)
		    	ResetControlFile();*/
		
			//License integrity validation
//			if(LicenseIntegrityCheck() != 1) removed to keep sercode from changing
//				ResetControlFile();         
				                     
				                     
			//Updates the user time to the control file and if
			//tampered with then exits
/*			if((TimeUpdate() == 0) && (GetRunsCount() != expnot))
			{
	
					ii=CreateControlFile();
					ii=SetDemoMask();     
					ii=SetRunsCount(AC, 0);
					ii=SetDateCount(AC, 0);
					ii=SetUsersCount(AC, 1);
					ResetControlFile();*/
//					goto RtnFalse;
			//	}	
		
		
			//Randomly updates all control points in the user's computer
			//UpdateRandom();
		
		
			//Check for integrity of Users Control in a network (Crash Protection)
			//(can be ignored in Single User environments)
		//	UsersCountValidate(HPI);
		
			
			//Get Runs and Date count plus User Time
		/*	fech = GetUserTime();
			ru = GetRunsCount();
			fe = GetDateCount();*/
		
		
			//Check for legal settings for days and runs
			//as well as the NO EXPIRATION value 
/*		    if((((DWORD)labs(fe - fech)) > MD) && (ru != expnot))
			{
	
					ii=CreateControlFile();
					ii=SetDemoMask();     
					ii=SetRunsCount(AC, 0);
					ii=SetDateCount(AC, 0);
					ii=SetUsersCount(AC, 1);
					ResetControlFile();
//					goto RtnFalse;
				}
		
			if((ru > MR) && (ru != expnot))
				{
					ResetControlFile();
//					goto RtnFalse;
				}*/	
				
		
			//Check for NO EXPIRATION value
//			if((LicenseIntegrityCheck() == 1) && (ru == expnot))
	        
    	 	if (GetGlobalBVal2 ("[%USEDISKREG]",FALSE))
    	 	{
				PrevErrMode = SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);
	
				for (i=0;i<26;i++)
				{   
					char	DriveID[2]={(char)('A'+i),0};	
					int DriveType = GetDriveType (DriveID);
					if (DriveType > 0)
					{   
						if (GetVolumeLabel(DriveID,VolLabel))
						{
							char	DriveID=(char)('A'+i);	
		                    HFILE	Fid;
							OFSTRUCTGM	OFStruct;
		                    
			            	sprintf (str,"%c:\\gmregist.txt",DriveID); 
			            	Fid = GSSiOpenFile(str,&OFStruct,OF_READ);
			            	if (Fid != HFILE_ERROR)
			            	{
			            		fgetstring (str,100,Fid);
			            		GSSiClose (Fid);
			            		if (EnterAccessCode (hWndDlg,str))  
			            		{
		        					GSSiMsgBox (hWndDlg,"Thank you for registering","",MB_OK,0);
									SetErrorMode(PrevErrMode);
			            			goto RtnTrue;
			            		}
			            	}
			            }
		            }
		        }
				SetErrorMode(PrevErrMode); 
			}
			
			//Decrement runs count by one     
//			GetGlobalCVal ("[%APPID]",AppName,"GeoMaster");
			if(RemDays > 0)
			{   
				
//				DecrementRunsCount();
				sprintf (mess,"You have %ld days before %s must be registered",RemDays,AppName) ;
				SetDlgItemText (hWndDlg,IDC_MESS,mess);
			}
			else
			{
				OKtoGo = FALSE;
				sprintf (mess,"%s must be registered before you can continue",AppName) ;  
				SetWindowText (GetDlgItem(hWndDlg,IDOK),"Exit");
				SetDlgItemText (hWndDlg,IDC_MESS,mess);
			}
		
		
         	cwCenter(hWndDlg, 0);  
         	break;
         	
		}
			
 RtnFalse:
		 EndDialog(hWndDlg, FALSE);  
		 return FALSE;
		 
RtnTrue: 
/*		 {
		 	DWORD ii,jj, MM=0;
		 	unsigned char AppSerNo[4];
		 	for (ii=1;ii<33;ii++)
		 		if (CheckModuleMask (ii))
		 			SetBit2 ((short)(ii-1),(LPSTR)&MM,TRUE);
		 	_fmemmove (&AppSerNo,&MM,4); 
		 	_fmemmove (&SerNo,&AppSerNo[1],3); 
//		 	App = AppSerNo[0];
		 } */
		 Wait (500);
       	 BringWindowToTop(hWndDlg);
		 EndDialog(hWndDlg, TRUE);  
		 return FALSE;
		 
    case WM_CLOSE:
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);

    case WM_COMMAND: 
         switch(LOWORD(wParam))
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);  
            break; 
            
            case IDOK: 
                EndDialog(hWndDlg, OKtoGo);
            break;    
            
            case IDC_REGNOW: 
            {
				char	mess[256];
			    
			    sprintf (mess,"Select either the 'Auto' button to automatically register via the internet or the 'Manual' button to register manually");
			    SetDlgItemText (hWndDlg,IDC_MESS2,mess);  
            	ShowWindow (GetDlgItem(hWndDlg,IDC_AUTOREGISTER),SW_HIDE);
            	ShowWindow (GetDlgItem(hWndDlg,IDC_MANUAL),SW_SHOW);
            	ShowWindow (GetDlgItem(hWndDlg,IDC_REGNOW),SW_HIDE);
			   	PostMessage(hWndDlg, WM_COMMAND, IDC_MANUAL, 0L);
			}
			break;
			    
            case IDC_MANUAL:
            {
            	char	mess[256]; 
			   	if (!GetTextString (hWndDlg,SerialNumber,32,"Enter the LMCODE located on the cover of the CD case.",0,0,0,TRUE,TRUE))
			   		break;
				strupr (SerialNumber);
				if (StoreSerialNumber (SerialNumber))
				{
					sprintf (mess,"Send the above SERIALIZATION CODE to your software vendor to get an ACCESS CODE. Enter the ACCESS CODE below and then select the 'Enter Access Code' button.");
					SetDlgItemText (hWndDlg,IDC_MESS2,mess); 
					GetSerialCode (mess);
					SetDlgItemText (hWndDlg,IDC_SERCODE,mess);
					//AddTextToClipboard (mess); bad idea - overwrites access code user may have in cb
            		ShowWindow (GetDlgItem(hWndDlg,IDC_SERCODE),SW_SHOW);
            		ShowWindow (GetDlgItem(hWndDlg,IDC_ACCESSCODE),SW_SHOW); 
            		ShowWindow (GetDlgItem(hWndDlg,IDC_ENTERACCESS),SW_SHOW);
            		ShowWindow (GetDlgItem(hWndDlg,IDC_AUTOREGISTER),SW_SHOW);
            		ShowWindow (GetDlgItem(hWndDlg,IDC_MANUAL),SW_HIDE);
				}
				else
					MessageBox (hWndDlg,"Unable to store serial number in registry",0,MB_ICONEXCLAMATION);
            //	ShowWindow (GetDlgItem(hWndDlg,IDC_TRANLICENSE),SW_SHOW); 
            //	ShowWindow (GetDlgItem(hWndDlg,IDC_LICENSEHELP),SW_SHOW);
            } 
            break; 
            
            case IDC_AUTOREGISTER:
            {
            	UINT	st=IDOK;

				ProcessText ("$WEB(http://www.lakemap.com/index.asp?PageAction=Custom&ID=11)");
 /*           	
            	AutoReg = TRUE;  
			   	if (!GetTextString (hWndDlg,SerialNumber,32,"Enter the LMCODE located on the cover of the CD case.",0,0,0,TRUE,TRUE))
			   		break;
				StoreSerialNumber (SerialNumber);
			   	while (st == IDOK)
			   	{   
			   		short err = CheckForRegistrationServer (hWndDlg,SerialNumber);

			   		switch (err)
			   		{
			   			case 0: 
			   			if (!_fstricmp (SerialNumber,"JOHNPAULGEORGERINGO"))
			   				EnterAccessCode (AccessCode);
			   			else
		            	{
		                  FARPROC lpfnAUTOREGMsgProc; 
		                  short	nRc;
		
						  WritePrivateProfileString (MODULENAME,"SerialNumber",SerialNumber,"geomastr.ini"); 
		                  lpfnAUTOREGMsgProc = MakeProcInstance((FARPROC)AUTOREGMsgProc, hInst);
		                  nRc = DialogBox(hInst, (LPSTR)"AUTOREG", hWndDlg, lpfnAUTOREGMsgProc);
		                  FreeProcInstance(lpfnAUTOREGMsgProc);  
		                  if (nRc)
						  {   
		        			GSSiMsgBox (hWndDlg,"Thank you for registering","",MB_OK);
			                goto RtnTrue;
						  }
		                  
		            	}
						st = IDCANCEL;
		            	break;
		            	
		            	case 1:
		            		st = GSSiMessageBox ("[REGERRNOCONNECT]",0,MB_OKCANCEL);
		            		if (st == IDCANCEL) 
		            			exit (1);
		            			//BlowOut(0,0);
		            		break;
		            	
		            	case 2:
		            		st = IDCANCEL;
		            		break;
		            	case 3:
		            		GSSiMessageBox ("[REGERRSNINUSE]",0,MB_ICONEXCLAMATION);
		            		st = IDCANCEL;
		            		break;
		            	case 4:
		            		GSSiMessageBox ("[REGERRSNINVALID]",0,MB_ICONEXCLAMATION);
		            		st = IDCANCEL;
		            		break;
	            	}
	            }
			*/
            }
            break;
            
            case IDC_TRANLICENSE:   
/*				if (TransferHardwareID() != 155)
					break;
            	if (GetTransferCode() != 1)
            		break;
				ru = GetRunsCount();
				fe = GetDateCount();
				if((LicenseIntegrityCheck() == 1) && (ru > 0))
				{   
					UpdateRandom();
        			GSSiMsgBox (hWndDlg,"Your license has been transferred","",MB_OK);
	                EndDialog(hWndDlg, TRUE);
				}
				else
           			GSSiMsgBox (hWndDlg,"Invalid Transfer Disk",0,MB_ICONEXCLAMATION);*/
            break;
            
            case IDC_ENTERACCESS:
            {
				char	p[6][8], mess[256];
				char	c[64];
				LPSTR	subc,last;
				short	i,n=0;  
				
				_fmemset (p,0,sizeof(p));
            	GetDlgItemText (hWndDlg,IDC_SERCODE,mess,sizeof(mess)); 
            	GetDlgItemText (hWndDlg,IDC_ACCESSCODE,c,sizeof(c)); 
            	
            	if (!_fstricmp (c,"JOHNPAULGEORGERINGO"))
            	{
            		EndDialog(hWndDlg, TRUE);
            		break;
            	}
            	_fstrupr (c);
            	if (!_fstricmp (mess,c))
            		goto ACErr;
            	if (EnterAccessCode (hWndDlg,c))
				{   
        			GSSiMsgBox (hWndDlg,"Thank you for registering","",MB_OK,0);
	                goto RtnTrue;
				}
				else
        		{   
        ACErr:
        			GSSiMsgBox (hWndDlg,"Invalid Access Code",0,MB_ICONEXCLAMATION,0);
        			return TRUE;
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


