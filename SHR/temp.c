
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
	char	str[1024]; 
	short	n;
	OFSTRUCT	OFStruct; 
	LPSTR	pLoc;
	HFILE Fid=OpenFile ("c:\\gssi\\prog\\geomastr\\funids.txt",&OFStruct,OF_READ);  
	
	*Name = 0;
	while (tgetstring (str,1020,Fid))
	{
		n = atoi (str);
		if (i == n)
		{   
			pLoc = _fstrchr (str,' ');
			pLoc++;
			_fstrncpy (Name,pLoc,255);
			break;
		}
	}
	_lclose (Fid);
	return;
}

BOOL FAR PASCAL TRACELISTMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
	char	str[512], ProgName[256];
	short	i;	

 switch(Message)
   {
    case WM_INITDIALOG: 
    	for (i=0;i<MAXPROG;i++)
    	{
    		if (EnterProg[i])
    		{   
    			GetProgName (i,ProgName);
    			sprintf (str,"%10ld\t%s",EnterProg[i],ProgName);
                SendDlgItemMessage (hWndDlg,IDC_CALLLIST,LB_ADDSTRING,0,(LPARAM)str);
    			sprintf (str,"%10ld\t%s",TotTime[i],ProgName);
                SendDlgItemMessage (hWndDlg,IDC_TIMELIST,LB_ADDSTRING,0,(LPARAM)str);
    		}
    	} 
         
         break; /* End of WM_INITDIALOG                                 */

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
	short	Abort, i;
	long	j,k;
	
	
	if (!progid)
	{
		for (i=0;i<MAXPROG;i++)
			if (EnterProg[i] != ExitProg[i]) 
			{
				j = EnterProg[i];
				k = ExitProg[i]	;
			}
			 
			{
				FARPROC lpfnTRACELISTMsgProc;
				HaveBlockingWindow = TRUE;
				lpfnTRACELISTMsgProc = MakeProcInstance((FARPROC)TRACELISTMsgProc, hInst);
				DialogBox(hInst, (LPSTR)"TRACELIST", hWndMain, lpfnTRACELISTMsgProc);
				FreeProcInstance(lpfnTRACELISTMsgProc); 
				HaveBlockingWindow = FALSE;

			}
			return 0;
	} 
	numEnter++;
	if (First)
	{   
		HANDLE	hstr, hI; 
		LPSTR	str;
		
	    First = FALSE;
	    hstr =  GlobalAlloc (GMEM_MOVEABLE,256);
	    str = GlobalLock (hstr);
//		sprintf (str,"%s %ld","c:\\gssi\\prog\\gmtrace\\debug\\gmtrace.exe",(long)hTrace);
//		hI = WinExec (str,SW_SHOWNORMAL);
		GlobalUnlock (hstr);
		GlobalFree (hstr); 
		_fmemset (EnterProg,0,sizeof(EnterProg));
		_fmemset (ExitProg,0,sizeof(ExitProg));   
		_fmemset (TotTime,0,sizeof(TotTime));
	}
	EnterProg[progid]++; 
	StartTime[Level] = GetTickCount();
	if (Level < MAXLEVEL)
		Level++;
	MaxLev = max (MaxLev,Level);
	return 0;
}

int	GSSiExitProg (int progid)
{   
	Level--;
	if (Level >= 0)
		TotTime[progid] += GetTickCount() - StartTime[Level];
	numExit++;
	ExitProg[progid]++;
	return 0;
}
