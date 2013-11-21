#include <windows.h>
#include <winbase.h>
#include <shlobj.h>
#include <commdlg.h>
#include <stdio.h>
#include <direct.h>
#include <winnt.h> 
#include <Wownt32.h>
#include "minilzo.h"
#include <vfw.h>
#include <string.h>
#include <assert.h>
#include <io.h>
#include <memory.h>
#include <stdlib.h>
#include "gmlimits.h"
#include "FreeImage.h"
#include <winreg.h>
#include <errno.h>
#include <urlmon.h>
#include <wingdi.h>
#include "resource.h"
#include <dlgs.h>
#include "gm32lib.h"
#define	MAXFILEHANDLES	80   

static	HFILE	MaxHFile=HFILE_ERROR;
static	int		NumOpenFiles=0;
static	OFSTRUCT	OpenFileStruct[MAXFILEHANDLES];
static	UINT		OpenFileMode[MAXFILEHANDLES];
static	HANDLE		OpenFileHandle[MAXFILEHANDLES];
static	long		OpenFileLength[MAXFILEHANDLES];
static	long		OpenFilePosition[MAXFILEHANDLES];
static	BOOL		OpenFileCloseRequested[MAXFILEHANDLES]; 
static	BOOL		InOpenFile=FALSE;
static	BOOL		UndoEnabled=FALSE;
static	BOOL		TraceOn=FALSE,KeepFilesOpen=FALSE;
static	HFILE	OpenFileFid[MAXFILEHANDLES]; 
static	short	OpenFileUndoFileID[MAXFILEHANDLES];
static	HFILE	UndoFileFid[MAXUNDOFILES];
static  char    CacheDir[132]="", CacheIndex[128]; 
static	char	SubstitutePathnameFrom[64]="", SubstitutePathnameTo[64];
static	char	CachePathnameFrom[4][64], CachePathnameTo[64]="";  
static	HANDLE	hCacheAlreadyChecked=0;
static	long	LastFGSLoc;
static	long	LastFGSlRec;   

void GSSiTrace (LPSTR str,short From)
{
	return;
}

int GSSiMsgBox (HWND hWnd, LPCSTR MessIn, LPCSTR TitleIn, UINT Flag)
{
	return 0;
}

LONG GSSillseek (HFILE Fid, LONG loc, int opt)
{
	if (Fid >= 2000)
	{   
		switch (opt)
		{
			case 0:
				OpenFilePosition[Fid-2000] = loc;
			break;
			case 1:
				OpenFilePosition[Fid-2000] += loc;
			break;
			case 2:
				OpenFilePosition[Fid-2000] = OpenFileLength[Fid-2000] + loc;
			break;
		}				
		return OpenFilePosition[Fid-2000];
	}
	return _llseek (Fid,loc,opt);
}
 
HFILE GSSiOpenFile (LPSTR Name,LPOFSTRUCT pOFStruct,UINT uStyle)
{
	OFSTRUCT	OFStruct;

	if (!pOFStruct)
		pOFStruct = &OFStruct;
	return OpenFile (Name,pOFStruct,uStyle);
}

void BufWrite (HPSTR *pBuf,LPLONG plBuf,HPSTR data,long ldata)
{   
	while (ldata--) 
	{
		*(*pBuf)++ = *data++;   
		(*plBuf)++;
	}
	return;
}

long  GSSilread(HFILE Fid, void _huge* ptr, long len)
{   
	
	if (Fid >= 2000)
	{
		HPSTR	pFile=GlobalLock (OpenFileHandle[Fid-2000]);
		long	endpos = min (OpenFileLength[Fid-2000],OpenFilePosition[Fid-2000]+len);
		
		len = endpos - OpenFilePosition[Fid-2000];
		if (len)
		{     
			memmove (ptr,pFile+OpenFilePosition[Fid-2000],len);
			OpenFilePosition[Fid-2000] = endpos;
		}
		GlobalUnlock (OpenFileHandle[Fid-2000]);
		return len;
	}   
	return _lread (Fid,ptr,len);
}

HFILE SetMemFile (HANDLE handle,long len)
{   
	short	i;
	
	for (i=0;i<MAXFILEHANDLES;i++) 
	{
		if (!OpenFileHandle[i])
		{
			OpenFileHandle[i] = handle; 
			OpenFileLength[i] = len;
			OpenFilePosition[i] = 0;
			return (i+2000); 
		}
	}
	return HFILE_ERROR;
}

HFILE GSSiClose (HFILE Fid)
																							#if ENABLETRACE
																							{GSSiEnterProg (301);
																							#endif
{   HFILE rtn;  
	short	ii; 
	USHORT	i;
	static	USHORT	debugi=12;
	static	HFILE	debugid=35;
	 
	if (Fid == HFILE_ERROR)
																							{
																							#if ENABLETRACE
																							GSSiExitProg (301);
																							#endif
		return 0; 
																							}
	if (Fid <= 0)
		ii=1; 
	if (Fid >= 2000)
	{
		GSSiGlobFree (&OpenFileHandle[Fid-2000]);
																							{
																							#if ENABLETRACE
																							GSSiExitProg (301);
																							#endif
		return 0; 
																							}
	}		
	if (!InOpenFile && RequestFileClose (Fid))
																							{
																							#if ENABLETRACE
																							GSSiExitProg (301);
																							#endif
		return 0;
																							}
    if (UndoEnabled)
	{
		short	ID = 0;//GetUndoFileIDFromFid (Fid);
		
		if (ID > -1)
			UndoFileFid[ID] = -1; 
		for (i=0;i<MAXFILEHANDLES;i++)
			if (OpenFileFid[i] == Fid)
			{
				OpenFileUndoFileID[i] = -1;
				if (i==debugi)
					ii=1; 
				break;
			}
	}
    rtn = _lclose (Fid);  
    if (Fid ==debugid)
    	ii=1; 
    if (rtn == HFILE_ERROR || TraceOn) 
    {
	    HANDLE	hMem=GSSiGlobAlloc (  90,GMEM_MOVEABLE,128);
	    LPSTR	str = GlobalLock (hMem);
	    if (rtn) 
	    {
	    	sprintf (str,"File close failed for %i",(short)Fid);
	    	GSSiMsgBox (GetFocus(),str,NULL,MB_ICONEXCLAMATION);
	    }
        sprintf(str,"Close file: %i %i",Fid,rtn);
        GSSiTrace(str,0);
    	GSSiGlobUlFree (&hMem);
    }
    LogOpenFilesClose (Fid);

																							{
																							#if ENABLETRACE
																							GSSiExitProg (301);
																							#endif
    return rtn;
																							}
																							#if ENABLETRACE
																							}
																							#endif
}
void DumpOpenFiles (void)
{   
	char	str[256];  
	UINT	i; 
	OFSTRUCT	OFStruct;
	HFILE	Fid = OpenFile ("c:\\openfile.txt",&OFStruct,OF_CREATE);
	   
	sprintf (str,"%i Open Files:");   
	fputstring (str,Fid);
	for (i=0;i<MAXFILEHANDLES;i++)
	{
		if (OpenFileFid[i] != HFILE_ERROR)
		{ 
			sprintf (str,"    -%i %s %u %i",OpenFileFid[i],OpenFileStruct[i].szPathName,OpenFileMode[i],(int)OpenFileCloseRequested[i]);    
			fputstring (str,Fid);
		}
	}
	return;
}


void CloseAllRequestedFiles (BOOL FirstOnly)
																							#if ENABLETRACE
																							{GSSiEnterProg (171);
																							#endif
{ 
	UINT	i;
	
	if (KeepFilesOpen)
	{
		for (i=0;i<MAXFILEHANDLES;i++)
		{   
			if (OpenFileFid[i] != HFILE_ERROR)
			{
				if (OpenFileCloseRequested[i])
				{
					_lclose (OpenFileFid[i]); 
					LogOpenFilesClose (OpenFileFid[i]);  
					if (FirstOnly)
						return;
				}
			}
		}
//		CloseAllRequestedDGNFiles ();
	}  
																							{
																							#if ENABLETRACE
																							GSSiExitProg (171);
																							#endif
	return;
																							}
																							#if ENABLETRACE
																							}
																							#endif
}

void LogOpenFilesOpen (UINT Mode,HFILE Fid,LPOFSTRUCT pOFStruct)
																							#if ENABLETRACE
																							{GSSiEnterProg (172);
																							#endif
{   
	short	i,j,ii; 
	LPSTR	pDot;   
	static	BOOL	FirstFileOpen=TRUE;
	
	if (FirstFileOpen)
	{ 
		for (i=0;i<MAXFILEHANDLES;i++)
		{
			OpenFileFid[i] = HFILE_ERROR; 
			OpenFileHandle[i] = 0; 
			OpenFileUndoFileID[i] = -1;
		}  
		FirstFileOpen = FALSE;
    }
	if (Fid == HFILE_ERROR)
																							{
																							#if ENABLETRACE
																							GSSiExitProg (172);
																							#endif
		return;
																							}
/*	if ((pDot=_fstrstr (pOFStruct->szPathName,".PLT")))
	{
		char	test[128];  
		
		*pDot = 0;
		_fstrcpy (test,pOFStruct->szPathName);  
		*pDot = '.';  
		_fstrcat (test,".rin");
		for (i=0;i<MAXFILEHANDLES;i++) 
		{
			if (OpenFileFid[i] != HFILE_ERROR)
			{ 
				if (!_fstricmp (test,OpenFileStruct[i].szPathName))
					ii=1;//BlowOut();
			}
		}
	} */
	if (Mode & OF_EXIST)
																							{
																							#if ENABLETRACE
																							GSSiExitProg (172);
																							#endif
		return;
																							}
	if (Mode & OF_DELETE)
																							{
																							#if ENABLETRACE
																							GSSiExitProg (172);
																							#endif
		return; 
																							}
	MaxHFile = max (MaxHFile,Fid);
	for (i=0;i<MAXFILEHANDLES;i++) 
		if (OpenFileFid[i] == HFILE_ERROR)
			goto Gotid;
	i = -1;
//	if (KeepFilesOpen) 
	{   
		for (j=0;j<MAXFILEHANDLES;j++)
		{
			if (OpenFileFid[j] != HFILE_ERROR) 
			{
				if (OpenFileCloseRequested[j])
				{
					_lclose (OpenFileFid[j]); 
					LogOpenFilesClose (OpenFileFid[j]);
					i = j; 
					break;
				}
			}
		}
	}
	if (i >= 0)
		goto Gotid;  
    DumpOpenFiles ();
	GSSiMsgBox (GetFocus(),"Open file limit exceeded",NULL,MB_ICONEXCLAMATION); 
 	 BlowOut(NULL,NULL);
																							{
																							#if ENABLETRACE
																							GSSiExitProg (172);
																							#endif
	return;
																							}
Gotid:			
	OpenFileFid[i] = Fid;     
	OpenFileStruct[i] = *pOFStruct;  
/*_fstrlwr (pOFStruct->szPathName);
	if (_fstrstr (pOFStruct->szPathName,"minnhenn.plt"))
		ii=1;*/
/*	{
		UINT j;
		
		for (j=0;j<MAXFILEHANDLES;j++)
			if (i!=j && OpenFileFid[j]!=HFILE_ERROR && !_fstricmp (OpenFileStruct[i].szPathName,OpenFileStruct[j].szPathName))
				MessageBox (0,OpenFileStruct[i].szPathName,"Dup File Open",MB_OK);
	}*/
	OpenFileMode[i] = Mode;
	OpenFileCloseRequested[i] = FALSE; 
	OpenFileUndoFileID[i] = 0;//GetUndoFileIDFromName (pOFStruct->szPathName,FALSE);
   
	NumOpenFiles++;
																							{
																							#if ENABLETRACE
																							GSSiExitProg (172);
																							#endif
	return;
																							}
																							#if ENABLETRACE
																							}
																							#endif
}

void LogOpenFilesClose (HFILE Fid)
																							#if ENABLETRACE
																							{GSSiEnterProg (173);
																							#endif
{   
	UINT	i,ii;  
	static	UINT	debugi=12;  
	static	HFILE	debugid=35;

	
	if (Fid == HFILE_ERROR)
																							{
																							#if ENABLETRACE
																							GSSiExitProg (173);
																							#endif
		return;
																							}
	if (Fid == debugid)
		ii=1;
		for (i=0;i<MAXFILEHANDLES;i++)
		if (OpenFileFid[i] == Fid)
		{
			OpenFileFid[i] = HFILE_ERROR;
			if (i == debugi)
				ii=1;
			NumOpenFiles--; 
																							{
																							#if ENABLETRACE
																							GSSiExitProg (173);
																							#endif
			return;
																							}
		} 
	ii=1;
		
																							{
																							#if ENABLETRACE
																							GSSiExitProg (173);
																							#endif
	return;
																							}
																							#if ENABLETRACE
																							}
																							#endif
} 

void CheckOpenFiles (void)
																							#if ENABLETRACE
																							{GSSiEnterProg (173);
																							#endif
{   
	UINT	i,ii;
	
	for (i=0;i<MAXFILEHANDLES;i++)
		if (OpenFileFid[i] != HFILE_ERROR)
		{   
			char	mess[256];
				
			GetOpenFilePathname (OpenFileFid[i],mess);
			_fstrcat (mess," still open");
#if CHECKMEM
			GSSiMsgBox (0,mess,NULL,MB_ICONEXCLAMATION);
#endif 
			_lclose (OpenFileFid[i]); 
			if (OpenFileFid[i]==42)
				ii=1;
		} 
																							{
																							#if ENABLETRACE
																							GSSiExitProg (173);
																							#endif
	return;
																							}
																							#if ENABLETRACE
																							}
																							#endif
} 

HFILE FileAlreadyOpen (LPSTR InName,UINT Mode,LPOFSTRUCT pOFStruct)
																							#if ENABLETRACE
																							{GSSiEnterProg (174);
																							#endif
{   
	UINT	i;
	
	if (KeepFilesOpen) 
	{   
		LPSTR	pFullPath = _fullpath (NULL,InName,0);    
		HANDLE	hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,512);
		LPSTR	pMem = GlobalLock (hMem);
		
		if (pFullPath)
		{	
			GetShortPathName (pFullPath,pMem,512);
			free (pFullPath);
			pFullPath = pMem;
			for (i=0;i<MAXFILEHANDLES;i++)
			{
				if (OpenFileFid[i] != HFILE_ERROR)
				{
					if (!_fstricmp (pFullPath,OpenFileStruct[i].szPathName))
					{
						if ((Mode == OF_READ || Mode == OF_EXIST) && OpenFileMode[i] == OF_READ)
						{   
							if (!OpenFileCloseRequested[i])
								continue;
							if (pOFStruct) 
								*pOFStruct = OpenFileStruct[i]; 
							if (Mode == OF_READ)
							{
								_llseek (OpenFileFid[i],0,0);
								OpenFileCloseRequested[i] = FALSE;
							}
							GSSiGlobUlFree (&hMem);
							 
																								{
																								#if ENABLETRACE
																								GSSiExitProg (174);
																								#endif
							return OpenFileFid[i];       
																								}
						}
						GSSiGlobUlFree (&hMem);
						_lclose (OpenFileFid[i]); 
						LogOpenFilesClose (OpenFileFid[i]);
																								{
																								#if ENABLETRACE
																								GSSiExitProg (174);
																								#endif
						return HFILE_ERROR;
																								}
					}
				}
			}  
		}
		GSSiGlobUlFree (&hMem);
	}
																							{
																							#if ENABLETRACE
																							GSSiExitProg (174);
																							#endif
	return HFILE_ERROR;
																							}
																							#if ENABLETRACE
																							}
																							#endif
}

BOOL RequestFileClose (HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (175);
#endif
{   
	UINT	i;
	
	if (!KeepFilesOpen)
{
#if ENABLETRACE
GSSiExitProg (175);
#endif
		return FALSE;
}
	for (i=0;i<MAXFILEHANDLES;i++)
		if (OpenFileFid[i] == Fid) 
		{   
	    	short	l= _fstrlen(CachePathnameTo);

    		if (l && !_fstrnicmp (OpenFileStruct[i].szPathName,CachePathnameTo,l))
	    	{
{
#if ENABLETRACE
GSSiExitProg (175);
#endif
					return FALSE;
}
	    	}
			if (OpenFileMode[i] == OF_READ)
			{ 
				OpenFileCloseRequested[i] = TRUE;
{
#if ENABLETRACE
GSSiExitProg (175);
#endif
				return TRUE;
}
			} 
{
#if ENABLETRACE
GSSiExitProg (175);
#endif
			return FALSE;
}
		}   
{
#if ENABLETRACE
GSSiExitProg (175);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

short	GetNumOpenFiles (void)
#if ENABLETRACE
{GSSiEnterProg (176);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (176);
#endif
	return NumOpenFiles;
}
#if ENABLETRACE
}
#endif
}

short GetOpenFileID (HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (177);
#endif
{   
	short	i;
	
	if (Fid < 0)
{
#if ENABLETRACE
GSSiExitProg (177);
#endif
		return -1;
}
	for (i=0;i<MAXFILEHANDLES;i++)
		if (OpenFileFid[i] == Fid) 
		{   
{
#if ENABLETRACE
GSSiExitProg (177);
#endif
			return i;
}
		}
{
#if ENABLETRACE
GSSiExitProg (177);
#endif
	return -1;
}
#if ENABLETRACE
}
#endif
}  

BOOL GetOpenFilePathname (HFILE Fid,LPSTR Name)
#if ENABLETRACE
{GSSiEnterProg (351);
#endif
{   
	short	i;
	
	if ((i=GetOpenFileID (Fid)) >= 0)
	{
		_fstrcpy (Name,OpenFileStruct[i].szPathName); 
{
#if ENABLETRACE
GSSiExitProg (351);
#endif
		return TRUE;
}
	}
{
#if ENABLETRACE
GSSiExitProg (351);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}

long BigWrite (HFILE Fid,HPSTR pMF,DWORD isize,long loc)
{
	return _lwrite (Fid,pMF,isize);
}



LPSTR fgetstring (LPSTR lpStr, USHORT len, HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (352);
#endif
{   UINT   lrec;
    LPSTR lpEnd; 
    DWORD   loc; 
    
    *lpStr = 0;            
    loc = _llseek (Fid,0,1); 
	LastFGSLoc = loc;    
    lrec = _lread (Fid,lpStr,len+1);
    if (!lrec || lrec == (UINT)HFILE_ERROR)
{
#if ENABLETRACE
GSSiExitProg (352);
#endif
    	return 0; 
}
    lpEnd = lpStr;
    while (lrec--)
    {
    	if (*lpEnd == '\r' || *lpEnd == '\n')
    		break;
    	lpEnd++;
    }
    *lpEnd++ = 0; 
    if (*lpEnd == '\n') lpEnd++;
    lrec =  lpEnd - lpStr;
    LastFGSlRec = lrec;
    _llseek (Fid,loc,0);
    _llseek (Fid,lrec,1);
{
#if ENABLETRACE
GSSiExitProg (352);
#endif
    return lpStr;
}
#if ENABLETRACE
}
#endif
}

BOOL fputstring(LPSTR lpStr, HFILE Fid)
#if ENABLETRACE
{GSSiEnterProg (350);
#endif
{   
    UINT    len; 
    
    len=_fstrlen(lpStr);
    if (len)
        BigWrite (Fid,(char *)lpStr,len,-1); 
    BigWrite (Fid,"\r\n",2,-1);
{
#if ENABLETRACE
GSSiExitProg (350);
#endif
    return TRUE;
}
#if ENABLETRACE
}
#endif
} 

void BlowOut (LPSTR Message, LPSTR Title)
{
	return;
}
