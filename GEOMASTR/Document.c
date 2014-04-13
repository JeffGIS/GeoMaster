#include "graphint.h"
#include "winexec.h"       
#include <mmsystem.h>
#include <shellapi.h>

#include "gmextern.h"


BOOL CreateCompressedImage (LPSTR ToName,LPSTR FromName)
{   
	HANDLE	hlpFI;
    LPFILEINDEX lpFI;
    OFSTRUCT    OFStruct;
	short  SizeOpt;   
	HFILE   FidIndex;
	short     Version=1;
	long    Signature=80251;  
	HCURSOR	hcurSave; 
	MNMXCORD FileBounds;
	extern	short	UserDefinedImageQuality;  
	char	Index[128], Drive[8],Dir[128],Name[34], ToDir[128], SetupFile[128];

	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT));
	UserDefinedImageQuality = 7500;
	_splitpath (ToName,Drive,Dir,Name,0);
	sprintf (Index,"%s%sindex",Drive,Dir);                 
	sprintf (ToDir,"%s%s",Drive,Dir);     
	sprintf (SetupFile,"%s%ssetup.txt",Drive,Dir); 
	FidIndex = GSSiOpenFile (SetupFile,&OFStruct,OF_CREATE);
	fputstring ("FEET",FidIndex);           
	fputstring ("0.0",FidIndex);           
	fputstring ("0.0",FidIndex);           
	fputstring ("1.0",FidIndex);           
	fputstring (FromName,FidIndex);
	GSSiClose (FidIndex);
	FidIndex = GSSiOpenFile (Index,(LPOFSTRUCT) &OFStruct,OF_CREATE); 
	
	hlpFI = GSSiGlobAlloc ( 408,GHND,sizeof(FILEINDEX)+sizeof(FILEINDEXENTRY)); 
	lpFI = (LPFILEINDEX) GlobalLock(hlpFI);    
	lpFI->CurrentEntry=(FILEINDEXENTRY *) &lpFI->FirstIndex;
	lpFI->Type = 5;  
	BigWrite (FidIndex,(char *)&lpFI->Type,2,-1);
	BigWrite (FidIndex,(char *)&lpFI->NumFiles,2,-1);
	BigWrite (FidIndex,(char *)&lpFI->Length,4,-1); 
	BigWrite (FidIndex,(char *)&lpFI->OrthoRes,8,-1);  
	
	SizeOpt = 1; 
	                 
	if (!AddMapToDir (hWndMain,SetupFile,ToDir,lpFI,2, 
	              TRUE,FALSE,
	              0,FidIndex,SizeOpt,"",FALSE,0,0,&FileBounds,FALSE,FALSE))
	BigWrite(FidIndex,(char *)&Signature,4,-1);
	BigWrite(FidIndex,(char *)&Version,2,-1);
	GSSillseek (FidIndex,0,0);
	BigWrite (FidIndex,(char *)&lpFI->Type,2,-1);
	BigWrite (FidIndex,(char *)&lpFI->NumFiles,2,-1);
	BigWrite (FidIndex,(char *)&lpFI->Length,4,-1); 
	BigWrite (FidIndex,(char *)&lpFI->OrthoRes,8,-1);
	GSSiClose (FidIndex);  
	GlobalUnlock(hlpFI);
	GSSiGlobFree (&hlpFI);
	AVIOutClose (&hAVIFile); 
	GSSiSetCursor (hcurSave);
	return TRUE;
}

void SetCurDocDir (LPSTR Prefix, LPSTR InUDI)
{
	char	UDI[80], DocDir[MAX_PATH];  
	LPSTR	lpUDI, lpEnd;  
	int		l;  
	
	GetGlobalCVal ("[%DOCDIR]",DocDir,"[%DL]document");
	sprintf (CurDocDir,"%s\\%s\\",DocDir,Prefix); 
 	_fstrcpy (UDI,InUDI);
 	ExpandText (CurDocDir);
 	l=_fstrlen(UDI);
 	lpUDI = UDI;
 	while (l>0)
 	{   
 		char	SaveChar;
			 		
 		lpEnd = lpUDI + 8;
 		SaveChar = *lpEnd;
 		*lpEnd = 0;
		_fstrcat (CurDocDir,lpUDI); 
	 	_fstrcat (CurDocDir,"\\"); 
	 	*lpEnd = SaveChar;
	 	lpUDI = lpEnd;
	 	l-=8;
 	}
 	return;
}


BOOL DisplayDocumentList (HWND hWnd,LPSTR Prefix,LPSTR UDI,int Item, HMENU InMenu)
{
	char	File[130], str[132];
	HMENU	DocMenu; 
	WORD	MenItem=32000; 
	HFILE	FidDoc;
	OFSTRUCT	OFStruct; 
	LPSTR	lpBar;
	BOOL	Found; 
	POINT	position;
	
	SetCurDocDir (PickList[Item].Prefix,PickList[Item].UDI);
	
	if (InMenu)
		DocMenu = InMenu;
	else
		DocMenu = CreatePopupMenu();          
//	AppendMenu (DocMenu,MF_ENABLED,MenItem,"Cancel"); 
	Found = FALSE;
	FidDoc=GSSiOpenFile("document.txt",&OFStruct,OF_READ);
	if (FidDoc!=HFILE_ERROR)
	{
		while (fgetstring(str,128,FidDoc))
		{  
			if ((lpBar = _fstrstr (str,"|")))
			{
			 	*lpBar = '\0';
				lpBar++; 
				MenItem++;
				sprintf (File,"%s%s",CurDocDir,lpBar);
				if (ExistFile(File))
				{
					AppendMenu (DocMenu,MF_ENABLED,MenItem,str);
					Found=TRUE;
				} 
			}
		}
		GSSiClose(FidDoc);
	} 
	if (InMenu) return TRUE;
	if (Found) 
	{
	   	GetCursorPos (&position);
		TrackPopupMenu (DocMenu,TPM_RIGHTBUTTON,position.x,position.y,0,hWndMain,0);
	}
	DestroyMenu (DocMenu); 

	return TRUE;
}  

BOOL HaveDesiredDocType (LPSTR Prefix, LPSTR UDI)
{   char	File[128];
    LPSTR	lpName;
    LPSHORT	pNumDocs;
    int		NumDocs, i;
                                          
	if (!hDesiredDocs) return FALSE;
	SetCurDocDir (Prefix,UDI);
	pNumDocs = (LPSHORT)GlobalLock (hDesiredDocs);
	NumDocs = *pNumDocs++; 
	lpName = (LPSTR)pNumDocs;
	for (i=0;i<NumDocs;i++,lpName+=14)
	{ 
		_fstrcpy (File,CurDocDir);
		_fstrcat (File,lpName);
		if (ExistFile(File))
		{
			GlobalUnlock (hDesiredDocs);
			return TRUE;
		}
	}
	GlobalUnlock (hDesiredDocs);
	return FALSE;
}




BOOL ProcessDocument (HWND hWnd, int Message, WPARAM wParam, LPARAM lParam)
{ 
    int		rec, irec;
 	HFILE	FidNoteType; 
 	char	File[128], str[132]; 
 	OFSTRUCT	OFStruct;
 	LPSTR	lpBar; 
 	BOOL	DisplayProp=TRUE;
   	UINT	hI;

 if (Message != WM_COMMAND) return FALSE;
 if (wParam < 32000 || wParam > 33000) return FALSE;
         	
	HaltMapDisplay(FALSE,TRUE);
 	rec = wParam - 32000;
 	irec = 0;
	FidNoteType=GSSiOpenFile("document.txt",&OFStruct,OF_READ);
	if (FidNoteType!=HFILE_ERROR)
	{
		while (fgetstring(str,128,FidNoteType))
		{   
			irec++;
			if (irec == rec)
			{
				if ((lpBar = _fstrstr (str,"|")))
				{
					*lpBar = '\0';
					lpBar++; 
					_fstrupr(lpBar);
					sprintf (File,"%s%s",CurDocDir,lpBar); 
					if (_fstrstr(File,".AVI"))
					{
						FullScreenPlay(0,File);
					}
					else if (_fstrstr(File,".WAV"))
					{
						sndPlaySound(File,SND_ASYNC|SND_NODEFAULT);
					}
					else if (_fstrstr(File,".BMP") || _fstrstr(File,".CBM") || 
					 		 _fstrstr(File,".PCX"))
					{   
						_fstrcpy (FullBM,File);
						ShowFullBM(TRUE,0,0); 
					}
					else if (_fstrstr(File,".BMZ"))
					{   
						char	cmd[256];
						
						sprintf (cmd,"pkunzip %s -o",File);
						_splitpath (File,0,0,FullBM,0);
						_fstrcat (FullBM,".BMP");
						ShowFullBM(TRUE,0,0); 
					}
					else 
					{
						hI=(UINT)ShellExecute (hWnd,0,File,0,0,SW_SHOWMAXIMIZED);
						DisplayShellExError (hI,File);				      	 
			        }
							
					break; 
				}
			}
		}
		GSSiClose(FidNoteType); 
	}                                 
	return TRUE;
 }   

BOOL DisplayShellExError (UINT ierror, LPSTR DocFile)
{   
	char		File[128]="[%INDIR]shelexer.txt";
	OFSTRUCT	OFStruct;
	HFILE		Fid; 
	char		str[256], Mess[256];  
	LPSTR		lpErMess;
	
	if (ierror > 31)
		return FALSE; 
	*str = 0;
	lpErMess = str;
	Fid = GSSiOpenFile (File,&OFStruct,OF_READ); 
	if (Fid == HFILE_ERROR)
	{
NoMess: 
		sprintf (Mess,"Error code %i attempting to display the following file:\n\r%s\n\r%s",(int)ierror,DocFile,lpErMess);
		MessageBox (GetFocus(),Mess,0,MB_ICONEXCLAMATION);
		return TRUE;
	}
	while (fgetstring (str,128,Fid))
	{   
		if (atoi(str) == ierror)
		{   
			lpErMess = _fstrchr (str,'\t');
			lpErMess++;
			GSSiClose (Fid);
			goto NoMess;
		}
	}
	GSSiClose (Fid);
	goto NoMess;
}

/* 
void LoadDocumentFile (HWND hWnd)
{   
	char	Name[128], str[256], CurFile[128], Prefix[10], UDI[34], NewFile[128];
	FILE	*Fid;  
	LPSTR	lpSpace,lpStr;
	
	HCURSOR	hcurSave;
	
	if (!GetFileName (hWnd,Name,IDS_FILTERTEXT)) return;
	
	hcurSave = SetCursor(LoadCursor(0, IDC_WAIT)); 
	Fid=fopen(Name,"rt");
	while (fgetss(str,256,Fid))
	{   
		if (!(lpSpace = _fstrchr (str,' '))) goto Next;
		*lpSpace=0;
		_fstrcpy (CurFile,str);
		ExpandText(CurFile);
		lpSpace++; 
		lpStr=lpSpace;
		if (!(lpSpace = _fstrchr (lpStr,' '))) goto Next;
		*lpSpace=0;   
		_fstrcpy (Prefix,lpStr);
		lpSpace++;   
		lpStr=lpSpace;
		if (!(lpSpace = _fstrchr (lpStr,' '))) goto Next;
		*lpSpace=0;   
		_fstrcpy (UDI,lpStr);
		lpSpace++;
		lpStr=lpSpace; 
		CreateDocumentDirectory (Prefix,UDI); 
		_fstrcpy (NewFile,CurDocDir);
		_fstrcat (NewFile,lpStr);
		copyfile (NewFile,CurFile);
		remove (CurFile);
		
Next:;
	}
	SetCursor (hcurSave); 
	
	return;
}
*/
 
void LoadDocumentFile (HWND hWnd)
{   
	char	Name[128], str[258], CurFile[128], Prefix[10], UDI[34], NewFile[128], file[16],leaf[16];
	FILE	*Fid;  
	LPSTR	lpSpace,lpStr;    
	char	CurDir[]="s:\\geoimag\\tiecards\\"; 
//	char	CurDir[]="e:\\pcxfiles\\";
	
	HCURSOR	hcurSave;
	              
	if (!GetFileName (hWnd,Name,0,IDS_FILTERTEXT)) return;
	
	hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
	Fid=fopen(Name,"rt");
	while (fgetss(str,256,Fid))
	{   
		_splitpath (str,0,0,leaf,0);
		switch (leaf[0])
		{
			case 'A':
				_fstrcpy (file,"water2.pcx");
				break;
			case 'B':
				_fstrcpy (file,"sewer2.pcx");
				break;
			case 'C':
				_fstrcpy (file,"tcard2.pcx");
				break;
			case 'D':
				_fstrcpy (file,"tcard3.pcx");
				break;
			case 'E':
				_fstrcpy (file,"water3.pcx");
				break;
			case 'S':
				_fstrcpy (file,"sewer1.pcx");
				break;
			case 'W':
				_fstrcpy (file,"water1.pcx");
				break;
			case 'T':
				_fstrcpy (file,"tcard1.pcx");
				break;
			case 'U':
				_fstrcpy (file,"back1.pcx");
				break;
			case 'V':
				_fstrcpy (file,"back2.pcx");
				break;
			case 'X':
				_fstrcpy (file,"back3.pcx");
				break;
			case 'Y':
				_fstrcpy (file,"back4.pcx");
				break;
			default:
				goto Next;
		}
		_fstrcpy (CurFile,str);
		_fstrcpy (Prefix,"PINCA");
		_fstrcpy (UDI,&leaf[1]);
		UDI[2]=0;
		_fstrcat (UDI,"11922");
		_fstrcat (UDI,&leaf[3]);
		UDI[9]='0';
		UDI[10]=0;
		_fstrcat (UDI,&leaf[5]);
		UDI[13]=0;
		CreateDocumentDirectory (Prefix,UDI); 
		_fstrcpy (NewFile,CurDocDir);
		_fstrcat (NewFile,file);
		copyfile (NewFile,CurFile,FALSE,0,0,0,0,0,0);
		GSSiRemove (CurFile);
		
Next:;
	}
	GSSiSetCursor (hcurSave); 
	
	return;
}


BOOL CreateDocumentDirectory (LPSTR Prefix, LPSTR UDI)
{
	LPSTR	lpUDI, lpEnd;  
	int		l;    
	DWORD	Err;
					      	 
	GetGlobalCVal ("[%DOCDIR]",CurDocDir,"[%DL]document");
	ExpandText(CurDocDir);
	GSSiMakeDir (CurDocDir,&Err); 
 	_fstrcat (CurDocDir,"\\");
	_fstrcat (CurDocDir,Prefix); 
	GSSiMakeDir (CurDocDir,&Err); 
 	_fstrcat (CurDocDir,"\\"); 
 	l=_fstrlen(UDI);
 	lpUDI = UDI;
 	while (l>0)
 	{   
 		char	SaveChar;
			 		
 		lpEnd = lpUDI + 8;
 		SaveChar = *lpEnd;
 		*lpEnd = 0;
		_fstrcat (CurDocDir,lpUDI); 
		GSSiMakeDir (CurDocDir,&Err); 
	 	_fstrcat (CurDocDir,"\\"); 
	 	*lpEnd = SaveChar;
	 	lpUDI = lpEnd;
	 	l-=8;
 	}
 	return TRUE;
}

  





