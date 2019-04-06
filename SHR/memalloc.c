#include <windows.h> 
#define _fstrncpy strncpy
#define _fstrncmp strncmp
#define _fstrnicmp strnicmp
#define _fstrlen strlen
#define _fstrcmp strcmp
#define _fstrcpy strcpy
#define _fstrstr strstr
#define _fstrlwr strlwr
#define _fstrcat strcat
#define _fstrupr strupr
#define _fstrchr strchr
#define _fstrrchr strrchr
#define _fstricmp stricmp
#define _fstrcspn strcspn
#define _fstrncat strncat
#define _fstrspn strspn
#define _fstrpbrk strpbrk
#define _fmemccpy memccpy
#define _fmemmove memmove
#define hmemmove memmove
#define _fmemcpy memcpy
#define _fstrset strset
#define _fmemicmp memicmp 
#define _fmemcmp memcmp
#define hmemcpy memcpy
#define _fmemset memset
#include <string.h>
#include <limits.h>  
#include "..\\GEOMASTR\resource.h"    
#include <stdio.h>  
#include <stdlib.h>
typedef char _huge *    HPSTR;          /* a huge version of LPSTR */
extern	BOOL	EnableTrace;
extern	HWND	TraceWnd,TraceWnd2;
void GetProgName (short i, LPSTR Name);

#define OFS_MAXPATHNAMEGM 256
typedef struct _OFSTRUCTGM {
	BYTE cBytes;
	BYTE fFixedDisk;
	WORD nErrCode;
	WORD Reserved1;
	WORD Reserved2;
	CHAR szPathName[OFS_MAXPATHNAMEGM];
} OFSTRUCTGM, *LPOFSTRUCTGM, *POFSTRUCTGM;

#define MAXPROG	4096     
#define MAXLEVEL	512   
static	BOOL	MemTrace=FALSE;
extern	BOOL	HaveBlockingWindow;
extern	HANDLE	hInst; 
extern	HWND	hWndMain;
BOOL	IgnoreLock=FALSE, ReportMemErrors=TRUE,KeepMemLength=FALSE;
static	char	FName[]="..\\GEOMASTR\\funids.txt";
static	short	LastProg[6]={0,0,0,0,0,0};
static	UINT	icount=1;

#if ENABLETRACE
//#include <toolhelp.h>
static	long	LastMessage;
static	short	Level=0, MaxLev=0, LevelProg[MAXLEVEL];
static	long	EnterProg[MAXPROG], ExitProg[MAXPROG]; 
static	__int64 	TotTime[MAXPROG], StartTime[MAXLEVEL];
#endif

static	long	ii, numEnter=0, numExit=0;
static	long	nextid=1, wantid=4268, WantCallID=31,NextLockID=1, WantLockID=8648, NextFreeID=1, WantFreeID=4386;
static	HANDLE	WantHandle=0;

#define	MAXMEM	8192   
#define MAXFREE	256 
#define PREMEM	16
#define POSTMEM	64   

#ifndef _WIN32
#define HUGE __huge  
#else
#define HUGE 
#endif

#if CHECKMEM || ENABLETRACE
typedef char				HUGE *HPSTR;
typedef unsigned char		HUGE *HPBYTE; 


static	short	LogMemDebugID=1572;
static	long	WantCallNo=31;
static	char	mess[1024];  
static	long	NumMemAlloc[MAXMEM];
static	HGLOBAL	hmem[MAXMEM]; 
static	HGLOBAL	RecentlyFreed[MAXFREE]; 
static	long	RecentlyFreedID[MAXFREE]; 
static	short	lockcount[MAXMEM];
static	long	memid[MAXMEM]; 
static	unsigned short	memidID[MAXMEM], CurrentID, wantidid=245;
static	long	memidcall[MAXMEM];
static	long	memlength[MAXMEM]; 
static	short	memprog[MAXMEM][6];
static	long	lockid[MAXMEM];  
static	short	premem[MAXMEM],postmem[MAXMEM];
static	BOOL	First=TRUE; 
static	BYTE	Marker=170; 

extern HANDLE countyLinkedVar,countyVar;

int checkvp(int i);

BOOL hDibIs32Bit (HANDLE hDib)
{
	BOOL rtn=TRUE;
	LPBYTE pByte = (LPBYTE) GlobalLock (hDib);
	
	if (pByte)
	{
		LPBITMAPINFOHEADER pDibInfo = (LPBITMAPINFOHEADER)(pByte + PREMEM);
		rtn = !(pDibInfo->biSize == 40);
		GlobalUnlock (hDib);
	}
	return rtn;

}

void MEMERR (LPSTR Mess) 
{   
	static	BOOL	ShowMess=TRUE;
	int findMEMERR = 0;
//	DebugBreak ();
	HaveBlockingWindow = TRUE;
	if (ShowMess)
		MessageBox (NULL,Mess,NULL,MB_ICONEXCLAMATION|MB_TASKMODAL);
	HaveBlockingWindow = FALSE; 
	return;
}  


void LogMemAlloc (unsigned short MemID,long MemLen)
{   
	short	ii;
	
	CurrentID = MemID;
	NumMemAlloc[MemID]++;
	if (MemID == LogMemDebugID && NumMemAlloc[MemID] == WantCallNo)
		ii=1;  
	if (MemID == 321)
		ii=1;
	return;
}

BOOL FAR PASCAL TRACEMEMMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
	int	TabStops[2]={45,500};
	char	str[512], ProgName[256],FindString[32];
	short	i,n;
	static	lastn=0;
	long	Tot=0;	

 switch(Message)
   {
    case WM_INITDIALOG: 
       	SendDlgItemMessage (hWndDlg,IDC_CALLLIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
       	SendDlgItemMessage (hWndDlg,IDC_TIMELIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);   
   		for (i=0;i<MAXMEM;i++)
   		{   
   			if (NumMemAlloc [i])
   			{   
   				Tot += NumMemAlloc[i];
				sprintf (str,"%10ld\t%i",NumMemAlloc[i],i);
	            SendDlgItemMessage (hWndDlg,IDC_TIMELIST,LB_ADDSTRING,0,(LPARAM)str);
	        }  
        } 
        sprintf (str,"%ld total calls",Tot);
        SetDlgItemText (hWndDlg,IDC_MESSAGE,str);
		memset (NumMemAlloc,0,sizeof(NumMemAlloc));
   		break;
         

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
         {  
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break; 
            
            case IDC_TIMELIST:
                 switch(HIWORD(lParam))
                 {   
                 	LPSTR	lpTAB;
           		 	short Choice=0;
                 	
                     case LBN_SELCHANGE:
                     {
	           		 	WORD	Op=IDC_TIMELIST;
	           		 	         
                         Choice=(short)SendDlgItemMessage(hWndDlg,wParam,LB_GETCURSEL,0,0);
                         SendDlgItemMessage(hWndDlg,wParam,LB_GETTEXT,Choice,(DWORD)str);
                         lpTAB = strchr (str,'\t');
                         *lpTAB++ = 0;
                         LogMemDebugID=atoi (lpTAB);
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

void TraceMem (void)
{
	DLGPROC lpfnTRACELISTMsgProc;

	if (EnableTrace<2)
		return;
	HaveBlockingWindow = TRUE; 
	MemTrace = TRUE;
	lpfnTRACELISTMsgProc = MakeProcInstance((DLGPROC)TRACEMEMMsgProc, hInst);
	DialogBox(hInst, (LPSTR)"TRACELIST", hWndMain, lpfnTRACELISTMsgProc);
	FreeProcInstance(lpfnTRACELISTMsgProc); 
	HaveBlockingWindow = FALSE;    
	MemTrace = FALSE;
    return;
}

BOOL    WINAPI GSSiPOSTMESSAGE(HWND hwnd, UINT cmd, WPARAM wparm, LPARAM lparm)
{   
	short	ii;
	
	if (cmd == 10003)
		ii=1;
	return (PostMessage (hwnd,cmd,wparm,lparm));
}

int CheckBTFID (HANDLE h)
{
typedef struct	{HGLOBAL			hBT_HEAD, hBT_BLOCK;
				 HFILE			BtFid;}BTREE;
typedef BTREE *LPBTREE;
extern HFILE OpenFileFid[950];
LPBTREE pBtree = GlobalLock (h);

	if (pBtree->BtFid >= 0)
	{
		if (OpenFileFid[pBtree->BtFid] < 0)
			ii=1;
	}
	GlobalUnlock (h);
	return 1; 
}

LPVOID glbllock(HANDLE h)
{
	LPVOID	pntr = GlobalLock(h);
	char * p = pntr;
	p += 16;
	pntr = p;
	return pntr;
}
BOOL glblUnlock(HANDLE h)
{
	return GlobalUnlock(h);
}


LPVOID GSSiGLOBALLOCK (HANDLE hglb)
{
	LPVOID	pntr;
	UINT	i; 
	HPBYTE	pstr;
extern LPVOID debugaddress;
//checkvp(1);	
/*	if (debugaddress && *(LPBYTE)debugaddress)
		ii=1;
	if (debugaddress && !*(LPBYTE)debugaddress)
		ii=1;
	if (countyLinkedVar && countyVar)
	{
		checkcounty(1);
	}*/
//checklastbox(1);
	if (hglb)
	{ 
		pntr = GlobalLock(hglb);
		if (!pntr)
			ii = 1;
		if (hglb == WantHandle)
		ii=1;
	for (i=0;i<MAXMEM;i++)
		if (hmem[i] == hglb)
		{   
			if (NextLockID == WantLockID)
				ii=1;
			pstr = (HPBYTE)pntr;
			memset (pstr,Marker,premem[i]);
			memset (&pstr[memlength[i]-postmem[i]],Marker,postmem[i]); 
			pntr = (LPVOID) ((LPSTR)pntr + premem[i]);
			if (!IgnoreLock && lockcount[i])// && memid[i] == wantid)
				ii=1;
			lockcount[i]++;
			if (lockcount[i] > 1)
				ii = 1;
			if (hglb == WantHandle && lockcount[i]>1)
				ii = 1;
			lockid[i] = NextLockID++;
			if (memid[i] == wantid)
			{ 
				ii=1; 
			}
			return pntr;
		} 
	if (IgnoreLock)
		return pntr;
	ii=-1;
	for (i=0;i<MAXFREE;i++)
		if (hglb == RecentlyFreed[i])
			ii=RecentlyFreedID[i]; 
	}
	MEMERR ("Locking invalid handle");
	return 0;
}

BOOL GSSiGLOBALUNLOCK(HANDLE hglb)
{
	BOOL	rtn=GlobalUnlock (hglb);
	UINT	i;
	HPBYTE	pstr; 
	long	j;
 //checkvp(1);	
   
    if (hglb)
    {
	if (hglb == WantHandle)
		ii=1;
	for (i=0;i<MAXMEM;i++)
		if (hmem[i] == hglb)     
		{   
			if (!lockcount[i]) 
			{
				MEMERR ("Not Locked");
				break; 
			}
			else
				lockcount[i]--;	
			if (memid[i] == wantid)
				ii=1;
			pstr = GlobalLock (hglb);
			for (j=0;j<premem[i];j++)
				if (pstr[j] != Marker) 
				{
					MEMERR ("Write before address");   
					break;
				}

			for (j=memlength[i]-postmem[i];j<memlength[i];j++)
				if (pstr[j] != Marker)
				{
					MEMERR ("Write after address");   
					break;
				}

			GlobalUnlock (hglb);	
			return rtn;
		} 
	if (IgnoreLock)
		return rtn;
	}
	MEMERR ("Unlocking invalid handle");
	return 0;
}

HGLOBAL GSSiGLOBALALLOC(UINT fuAlloc, DWORD cbAlloc)
{
	HANDLE	rtn;
	UINT	i; 
	long	memlen;
	long	prmem,pstmem=POSTMEM; 
	HPBYTE	pstr;
	
	if (KeepMemLength)
	{
		prmem = pstmem = 0;
		memlen = cbAlloc;
	}
	else if (PREMEM)   
	{   
		prmem = PREMEM;
		memlen = cbAlloc + PREMEM + POSTMEM;   
	}
	else 
	{   
		prmem = 0;
		memlen = cbAlloc + POSTMEM;
	}
	rtn = GlobalAlloc (fuAlloc,memlen); 
	if (!rtn)
	{
		MEMERR ("Memory allocation failed"); 
		exit (1);
	} 
	if (rtn == WantHandle)
		ii=1;
	pstr = (HPBYTE)GlobalLock (rtn);
	memset (pstr,Marker,prmem);
	memset (&pstr[memlen-pstmem],Marker,pstmem); 
	GlobalUnlock (rtn);
	if (First)
	{
		First = FALSE;
		memset (hmem,0,sizeof(hmem)); 
		memset (RecentlyFreed,0,sizeof(RecentlyFreed)); 
		memset (lockcount,0,sizeof(lockcount));
		memset (NumMemAlloc,0,sizeof(NumMemAlloc));
	}
	for (i=0;i<MAXMEM;i++)
		if (!hmem[i])     
		{
			hmem[i] = rtn;
			lockcount[i] = 0; 
			memlength[i] = memlen;
			memmove (memprog[i],LastProg,sizeof(LastProg));  
			if (/*i==606 && */memlen == 644)
				ii=1; 
			/*{
				HTASK	hTask; 
				TASKENTRY	TE;
				static	WORD	MaxStack=0, UsedStack=0;  
				long	RemStack;
				 
				hTask = GetCurrentTask ();   
				TE.dwSize = sizeof (TE);
				if (TaskFindHandle (&TE,hTask))
				{
					MaxStack = max (MaxStack,TE.wStackBottom-TE.wStackTop); 
					UsedStack = max (UsedStack,TE.wStackBottom-TE.wStackMinimum); 
					RemStack = ((long)MaxStack - (long)UsedStack);
		//			memstack[i]=RemStack;
		//			memstack[i]=TE.wSS;
				} 
			} */
			premem[i] = prmem;
			postmem[i] = pstmem;
			memid[i] = nextid++;
			memidID[i] = CurrentID;
			memidcall[i] = NumMemAlloc[CurrentID];	
			if (memidcall[i] == 1886 && memid[i] == 7490)
				ii = 1;
			if (memidID[i] == wantid)
			{ 
				if (memidcall[i] = WantCallID)
//if (memid[i]>11335&&memid[i]<11380&&memlength[i]==8)
				ii=1;
			}
			if (i > MAXMEM-1000 && memidID[i] == wantidid)
				ii=1;
			if (i > MAXMEM-100)
				ii=1;	
			return rtn;
		}
	MEMERR ("Mem Alloc Limit Reached");
	exit (1);
} 

void GSSiRemoveMem (HGLOBAL hglb)
{    
	UINT	i;
	for (i=0;i<MAXMEM;i++)
		if (hmem[i] == hglb)
		{
			hmem[i] = 0;
			break;
		}
	return;     
}  


HGLOBAL GSSiGLOBALFREE (HANDLE hglb)
{   
	UINT	i; 
	HGLOBAL rtn;

	if (hglb == countyLinkedVar)
		ii = 1;
	if (hglb == WantHandle)
		ii=1;
	for (i=MAXFREE-1;i>0;i--)
	{
		RecentlyFreed[i]=RecentlyFreed[i-1]; 
		RecentlyFreedID[i]=RecentlyFreedID[i-1]; 
	}
	if (WantFreeID == NextFreeID)
		ii=1;
	RecentlyFreed[0] = hglb;
	RecentlyFreedID[0] = NextFreeID++;
	if (hglb)
	{
	for (i=0;i<MAXMEM;i++)
		if (hmem[i] == hglb)     
		{
			hmem[i] = 0;
			if (memid[i] == wantid)
				ii=lockid[i];	
			if (lockcount[i])
				MEMERR ("Free locked handle");
			rtn = GlobalFree(hglb);
			return rtn;
		} 
		rtn = GlobalFree(hglb);
		if (IgnoreLock)
			return rtn; 
	}
	MEMERR ("Free invalid address");
	return 0;
}

HGLOBAL WINAPI GSSiGLOBALREALLOC (HGLOBAL hglb, DWORD cbAlloc, UINT fuAlloc)
{
	HGLOBAL	hnew;
    UINT	i;                          
	long	memlen;
	long	prmem,pstmem=POSTMEM; 
	HPBYTE	pstr; 
	BYTE	LastByte;
	
	if (KeepMemLength)
	{
		prmem = pstmem = 0;
		memlen = cbAlloc;
	}
	else if (PREMEM)   
	{   
		prmem = PREMEM;
		memlen = cbAlloc + PREMEM + POSTMEM;  
	}
	else 
	{   
		prmem = 0;
		memlen = cbAlloc + POSTMEM;
	}
	hnew = GlobalReAlloc (hglb,memlen,fuAlloc); 
	if (!hnew)
	{
		MEMERR ("Memory allocation failed"); 
		exit (1);
	}
	pstr = (HPBYTE)GlobalLock (hnew); 
	LastByte = pstr[memlen-1];
	memset (pstr,Marker,prmem);
	memset (&pstr[memlen-pstmem],Marker,pstmem); 
	GlobalUnlock (hnew);
    
	for (i=0;i<MAXMEM;i++)
		if (hmem[i] == hglb)     
		{
			if (premem[i] && !prmem)
			{   
				long	j;
				
				pstr = (HPBYTE)GlobalLock (hnew);
				memmove (pstr,&pstr[premem[i]],memlength[i]-premem[i]-postmem[i]);
				memset (&pstr[memlength[i]-premem[i]-postmem[i]],LastByte,premem[i]+postmem[i]); 
				j=memlen-postmem[i];
				ii=pstr[j];
				GlobalUnlock (hnew);
			}
			hmem[i] = hnew; 
			memlength[i] = memlen;   
			premem[i] = prmem; 
			postmem[i] = pstmem;
			if (memid[i] == wantid)
				ii=1;	
			if (memlen == 644)
				ii=1;
			if (lockcount[i])	
				MEMERR ("Re-alloc locked handle");
			return hnew;
		}
	MEMERR ("Re-alloc invalid address");
	
	return hnew;
}

void GSSiGLOBALLOCCLOSE (void)
{   
	UINT	i, iprog;
	
	for (i=0;i<MAXMEM;i++)
		if (hmem[i])
		{   
			ii = memid[i]; 
			sprintf (mess,"Memory not freed: %ld-%ld-%ld(%ld)",(long)memidID[i],memid[i],memidcall[i],memlength[i]);
#if ENABLETRACE
			for (iprog=0;iprog<min(5,memprog[i][0]);iprog++)
			{
				_fstrcat (mess,"\r\n");
				GetProgName (memprog[i][iprog+1],strchr (mess,0));
			}
#endif
			MEMERR (mess); 
		}
	return;
} 

DWORD GSSiGLOBALSIZE (HANDLE hglb)
{   
	DWORD	rtn=GlobalSize (hglb);
	UINT	i;
	
	for (i=0;i<MAXMEM;i++)
		if (hmem[i] == hglb)     
		{   
			rtn -= (premem[i] + postmem[i]);
			return rtn;
		}
	return rtn;
} 

#else
 
void MEMERR (LPSTR Mess)
{
	return;
}
BOOL hDibIs32Bit (HANDLE hDib)
{
	BOOL rtn=TRUE;
	LPBYTE pByte = (LPBYTE) GlobalLock (hDib);
	
	if (pByte)
	{
		LPBITMAPINFOHEADER pDibInfo = (LPBITMAPINFOHEADER)(pByte);
		rtn = !(pDibInfo->biSize == 40);
		GlobalUnlock (hDib);
	}
	return rtn;

}


#endif     


#ifdef ENABLETRACE   

void ShowTextTrace (LPSTR str)
{   
//	return;
	if (hWndMain)
	{
		HDC hdc = GetDC (hWndMain); 
		RECT	Rect={45,45,190,70};
		
		SaveDC(hdc);

		SetMapMode(hdc, MM_TEXT);
	    SetWindowOrgEx  ( hdc, 0, 0,0 );
	    SetViewportOrgEx( hdc, 0, 0,0 );    
		SelectClipRgn (hdc,NULL);
        SetTextColor (hdc,0); 
        SetBkColor (hdc,RGB(255, 255, 255));  
        SelectObject (hdc,GetStockObject(SYSTEM_FONT));
//        SetBkMode(hdc, OPAQUE);
        ExtTextOut (hdc,50,50,ETO_OPAQUE,&Rect,str,_fstrlen(str),NULL);  

		RestoreDC(hdc, -1);

	}
	return;
}

LPSTR tgetstring (LPSTR lpStr, WORD len, HFILE Fid)
{   UINT   lrec;
    LPSTR lpEnd; 
    DWORD   loc; 
    
    *lpStr = 0;            
    loc = _llseek (Fid,0,1); 
    lrec = _lread (Fid,lpStr,len+1);
    if (!lrec || lrec == (UINT)HFILE_ERROR)
    	return 0; 
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
    _llseek (Fid,loc,0);
    _llseek (Fid,lrec,1);
    return lpStr;
}

void GetProgName (short i, LPSTR Name)
{   
	char	str[512]; 
	short	n;
	OFSTRUCTGM	OFStruct; 
	LPSTR	pLoc; 
	HFILE Fid=OpenFileGM (FName,&OFStruct,OF_READ);  
	
	*Name = 0;
	while (tgetstring (str,500,Fid))
	{
		LPSTR pchr = str;

		while (*pchr)
		{
			if (*pchr == '\t')
				*pchr = ' ';
			pchr++;
		}
		n = atoi (str);
		if (i == n)
		{   
			pLoc = strchr (str,' ');
			pLoc++;
			_fstrncpy (Name,pLoc,255);
			break;
		}
	}
	_lclose (Fid);
	return;
}

BOOL FAR PASCAL TRACELISTMsgProc(HWND hWndDlg, int Message, WPARAM wParam, LPARAM lParam)
{ 
	int	TabStops[2]={45,500};
	char	str[512], ProgName[256],FindString[32];
	short	i,n;
	static	lastn=0;	

 switch(Message)
   {
    case WM_INITDIALOG: 
		{
			LARGE_INTEGER	liFrequency;
			__int64 Frequency;

			QueryPerformanceFrequency(&liFrequency);
			Frequency = liFrequency.QuadPart;
			Frequency /= 1000;
       		SendDlgItemMessage (hWndDlg,IDC_CALLLIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);
       		SendDlgItemMessage (hWndDlg,IDC_TIMELIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops);   
       		if (MemTrace)
       		{  
       			for (i=0;i<MAXPROG;i++)
       			{   
       				if (NumMemAlloc [i])
       				{
						sprintf (str,"%10ld\t%i",NumMemAlloc[i],i);
						SendDlgItemMessage (hWndDlg,IDC_TIMELIST,LB_ADDSTRING,0,(LPARAM)str);
					}  
				}
				memset (NumMemAlloc,0,sizeof(NumMemAlloc));
       			break;
       		}
    		for (i=0;i<MAXPROG;i++)
    		{
    			if (EnterProg[i])
    			{   
    				GetProgName (i,ProgName);
    				sprintf (str,"%10ld\t%5.5i %s",EnterProg[i],i,ProgName);
					SendDlgItemMessage (hWndDlg,IDC_CALLLIST,LB_ADDSTRING,0,(LPARAM)str);
    				sprintf (str,"%10ld\t%5.5i %s",(long)(TotTime[i]/Frequency),i,ProgName);
					SendDlgItemMessage (hWndDlg,IDC_TIMELIST,LB_ADDSTRING,0,(LPARAM)str);
    			}
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
            case IDCANCEL: 
                EndDialog(hWndDlg, FALSE);

            break; 
            
            case IDC_FIND_NEXT:
            	n = lastn;
            	goto Find;
            case IDC_FIND_FIRST:
            	n=0;
      Find:
      		GetDlgItemText (hWndDlg,IDC_FINDSTRING,FindString,sizeof(FindString)-1);
            {
                 while (SendDlgItemMessage(hWndDlg,IDC_CALLLIST,LB_GETTEXT,n,(DWORD)str)!=LB_ERR)
                 { 
                 	 if (_fstrstr (str,FindString))
                 	 {  
                 	 	SendDlgItemMessage (hWndDlg,IDC_CALLLIST,LB_SETCURSEL,n,0); 
                 	 	SendDlgItemMessage (hWndDlg,IDC_CALLLIST,LB_SETTOPINDEX,n,0); 
                 	 	break;
                 	 }
                 	 n++;  
                 	 lastn = n;
                 }
            } 
            break;
            
            case IDC_TIMELIST:
            case IDC_CALLLIST:
                 switch(HIWORD(wParam))
                 {   
                 	LPSTR	lpTAB;
           		 	short Choice=0;
                 	
                     case LBN_SELCHANGE:
                     {
	           		 	WORD	Op=IDC_TIMELIST;
	           		 	
	           		 	if (LOWORD(wParam) == IDC_TIMELIST)
	           		 		Op = IDC_CALLLIST;
                         Choice=(short)SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETCURSEL,0,0);
                         SendDlgItemMessage(hWndDlg,LOWORD(wParam),LB_GETTEXT,Choice,(DWORD)str);
                         lpTAB = strchr (str,'\t');
                         *lpTAB++ = 0;
                         _fstrcpy (ProgName,lpTAB);
                         n=0;
                         while (SendDlgItemMessage(hWndDlg,Op,LB_GETTEXT,n,(DWORD)str)!=LB_ERR)
                         { 
	                         lpTAB = strchr (str,'\t');
	                         *lpTAB++ = 0;
                         	 if (!_fstricmp (lpTAB,ProgName))
                         	 {  
                         	 	SendDlgItemMessage (hWndDlg,Op,LB_SETCURSEL,n,0); 
                         	 	SendDlgItemMessage (hWndDlg,Op,LB_SETTOPINDEX,n,0); 
                         	 	break;
                         	 }
                         	 n++;
                         } 
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

int	GSSiEnterProg (int progid)
{   
	static	BOOL	First=TRUE; 
	short	killer=0;
	short	FAR	*pCurProg;
	short	Abort, i,n;
	long	j,k;
//	HTASK	hTask; 
//	TASKENTRY	TE;
	static	WORD	MaxStack=0, UsedStack=0;  
	long	RemStack; 
	char	str[256];
	
//	checkvp(1);
//	if (_fstrncmp (FName,"..\\GEOMASTR\\funids.txt",36))
//		ii=1;
	if (!progid)
	{
		for (i=0;i<MAXPROG;i++)
			if (EnterProg[i] != ExitProg[i]) 
			{
				j = EnterProg[i];
				k = ExitProg[i]	;
			}
			if (EnableTrace>1)
			{
				DLGPROC lpfnTRACELISTMsgProc;
				HaveBlockingWindow = TRUE;
				lpfnTRACELISTMsgProc = MakeProcInstance((DLGPROC)TRACELISTMsgProc, hInst);
				DialogBox(hInst, (LPSTR)"TRACELIST", hWndMain, lpfnTRACELISTMsgProc);
				FreeProcInstance(lpfnTRACELISTMsgProc); 
				HaveBlockingWindow = FALSE;

			}
			return 0;
	}
	if (!EnableTrace)
		return 0;  
/*	hTask = GetCurrentTask ();   
	TE.dwSize = sizeof (TE);
	if (TaskFindHandle (&TE,hTask))
	{
		MaxStack = max (MaxStack,TE.wStackBottom-TE.wStackTop); 
		UsedStack = max (UsedStack,TE.wStackBottom-TE.wStackMinimum); 
		RemStack = ((long)MaxStack - (long)UsedStack);
		if (RemStack && RemStack < 1000)
			ii=1;
	}*/
	numEnter++;
	if (First)
	{   
		HANDLE	hstr, hI; 
		LPSTR	str;
		
	    First = FALSE;
	    hstr =  GlobalAlloc (GMEM_MOVEABLE,256);
	    str = GlobalLock (hstr);
//		sprintf (str,"%s %ld","..\\gmtrace\\debug\\gmtrace.exe",(long)hTrace);
//		hI = WinExec (str,SW_SHOWNORMAL);
		GlobalFree (hstr); 
		memset (EnterProg,0,sizeof(EnterProg));
		memset (ExitProg,0,sizeof(ExitProg));   
		memset (TotTime,0,sizeof(TotTime));
		memset (StartTime,0,sizeof(StartTime));
	}
	if (progid == 655)
		ii=1;
	EnterProg[progid]++; 
	LevelProg[Level] = progid; 
//	LastProg[0] = Level;
//	for (i=Level,n=1;i>max(Level-5,0);i--)
//		LastProg[n++] = LevelProg[i-1];
	if (EnableTrace > 1)
	{
		LARGE_INTEGER	largeint;

		BOOL	HaveHHPC = QueryPerformanceCounter(&largeint);
		StartTime[Level] = largeint.QuadPart;
	}
	if (Level < MAXLEVEL-1)
		Level++; 
	else
		ii=1;
	MaxLev = max (MaxLev,Level);
//	sprintf (str,"Enter: %i %i %ld %i 0",progid,Level,LastMessage,LevelProg[max(0,Level-2)]);
	if (TraceWnd2)
	{
		if (!(icount++%1000000))
		//SetWindowText (TraceWnd,str);
			PostMessage (TraceWnd2,10000,1,progid);
		if (icount > 1000000000)
		{
			LPSTR x=0;
			char	y=*x;
		}

	}
//	ShowTextTrace (str);
	return 0;
}

int	GSSiExitProg (int progid)
{   
	short	i,n;
	short	ii;
	char	str[256]; 
	
	if (!EnableTrace || Level <= 0)
		return 0;  
//	checkvp(1);
	Level--;  
//	LastProg[0] = Level;
//	for (i=Level,n=1;i>max(Level-5,0);i--)
//		LastProg[n++] = LevelProg[i-1];
//	sprintf (str,"Exit: %i %i %ld %i 1",LastProg,Level,LastMessage,progid);
	if (TraceWnd2)
	{
		if (!(icount++%1000000))
		//SetWindowText (TraceWnd,str);
			PostMessage (TraceWnd2,10000,2,progid);
		if (icount > 1000000000)
			ii=1;
	}

//	ShowTextTrace (str);
	if (Level >= 0)
	{
		if (LevelProg[Level] != progid)
		{
			if (LevelProg[Level+1] != progid)
				ii=1;
		}
		if (EnableTrace > 1 && StartTime[Level])
		{
			LARGE_INTEGER	CurrentTime;
			
			QueryPerformanceCounter(&CurrentTime);
			TotTime[progid] += CurrentTime.QuadPart - StartTime[Level];
		}
	}
	numExit++;
	ExitProg[progid]++;
	return 0;
} 

int SetLastMessage (long mes, WPARAM wParam)
{
#define MAXMESS	32
	static int numMess = 0;
	static messageList[MAXMESS] = { 0 };
	static wparamList[MAXMESS] = { 0 };
	memmove(&messageList[1], &messageList[0], sizeof(int)*(MAXMESS - 1));
	memmove(&wparamList[1], &wparamList[0], sizeof(int)*(MAXMESS - 1));
	messageList[0] = mes;
	wparamList[0] = wParam;
	LastMessage = mes;
	if (mes == WM_IME_NOTIFY)
	{
		switch (wParam)
		{
		case IMN_CLOSESTATUSWINDOW:
			ii = 1;
			break;
		case IMN_OPENSTATUSWINDOW:
			ii = 1;
			break;
		default:
			ii = 1;
			break;
		}
	}
	if (mes == WM_IME_SETCONTEXT)
	{
		switch (wParam)
		{
		case 0:
			ii = 1;
			break;
		case 1:
			ii = 1;
			break;
		default:
			ii = 1;
			break;
		}
	}

	return 0;
}
#else
	BOOL FAR PASCAL TRACELISTMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
	{
		return FALSE;
	}
#endif
BOOL DumpTraceback (LPSTR AbendFile)
{
#if ENABLETRACE
	int	i=Level;
	char	str[32];

	AppendFile2 (AbendFile,"Traceback:");
	while (i--)
	{
		itoa (LevelProg[i],str,10);
		AppendFile2 (AbendFile,str);
	}
#endif
	return TRUE;
}

