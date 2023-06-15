#include "graphint.h"

static	LPTHEME	LastGFTheme=0;
static	short	LastGFConfig;

#include "gmextern.h"

BOOL SetZoomVP (LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (425);
#endif
{ 
	BOOL	rtn=FALSE;

	if (!CurrentConfig)
		SetConfig (1);
    if (lParam > 0 && lParam <MAX_VIEWPORTS)
	{
        SetViewport((short)lParam);	
		rtn = TRUE;
	}
	else if (!CurView)                                   
		SetViewport(*pCommandViewport);
	else if (GetGlobalBVal2 ("[%ZOOMCMDVPONLY]",FALSE))
		SetViewport(*pCommandViewport);
    else if (!VPIsMap(CurView->ID))
		SetViewport(*pCommandViewport);
{
#if ENABLETRACE
GSSiExitProg (425);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}  

HWND GetParFocus ()
#if ENABLETRACE
{GSSiEnterProg (426);
#endif
{    
	HWND hFocus, hPar, hOwn;
	
	hPar = hFocus =GetFocus();
	while (hFocus)
	{
		hPar = hFocus;
		hFocus = GetParent (hPar);    
//		hOwn = GetWindow (hPar,GW_OWNER);
	}
{
#if ENABLETRACE
GSSiExitProg (426);
#endif
	return hPar;
}
#if ENABLETRACE
}
#endif
}
BOOL IsGFunctionKey (int Key,BOOL SendCmd)
#if ENABLETRACE
{GSSiEnterProg (450);
#endif
{   
	long	CurLoc,LastLoc;
	HFILE	Fid; 
	HANDLE	hStr = GSSiGlobAlloc (  38,GMEM_MOVEABLE,2048);
	LPSTR	lpStr = GlobalLock (hStr);
	LPSTR	pFile = lpStr + 1024;
	LPSTR	lpBar, pGCmd, lpCarrot; 
	BOOL	HaveCmd=FALSE, rtn, ProcessedCurView=FALSE; 
	short	FileNo=0, iview, ifile;  
	LPTHEME	SaveTheme = CurTheme;
	LPVIEWPORT	SaveVP = CurView;
    
    if (!CurrentConfig)
    	goto RtnFalse; 
    goto NextFile;
Top:
	LastLoc = 0; 
	CurLoc = -1;
	Fid = GSSiOpenFile (pFile,0,OF_READ);
	if (Fid == HFILE_ERROR)  
		goto NextFile;
	while (fgetstring (lpStr,1020,Fid))
	{
		if (*lpStr != '|' && *lpStr != '#')
		{
		 	if ((lpBar = _fstrchr(lpStr,'|')))
		 	{
				*lpBar++=0; 
				if ((lpCarrot = _fstrchr(lpStr,'^')))
				{   
					lpCarrot++;   
					if (Key > 2000)
					{
						char	keytxt[8]="F";
						short	l;
						
						if ((Key == 2001 && !_fstrnicmp ("<-",lpCarrot,2)) ||
							(Key == 2002 && !_fstrnicmp ("/\\",lpCarrot,2)) ||
							(Key == 2003 && !_fstrnicmp ("->",lpCarrot,2)) ||
							(Key == 2004 && !_fstrnicmp ("\\/",lpCarrot,2)))
						{
							CurLoc = LastLoc;
							break;
						}
					}
					else if (Key > 1000)
					{
						char	keytxt[8]="F";
						short	l;
						
						itoa ((short)Key-1000,_fstrchr(keytxt,0),10); 
						l = _fstrlen (keytxt);
						if (!_fstrnicmp (keytxt,lpCarrot,l))
						{
							CurLoc = LastLoc;
							break;
						}
					}
					else if (*lpCarrot == Key)
					{
						CurLoc = LastLoc;
						break;
					}
				}
			}
		}
		LastLoc = GSSillseek (Fid,0,1); 
	}
	GSSiClose2 (&Fid);
	if (!CurrentConfig)
		SetConfig (1);
	CurView = SaveVP;
	CurTheme = SaveTheme;
	if (CurLoc >= 0) 
	{
		rtn = RunGFCommandFromFileAtLoc (pFile,CurLoc,SendCmd,0);
		GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (450);  
#endif
		return rtn;
}
	}  
NextFile:
	if (NumViewportsArray[0])
	{
		ifile = 0;
		SetConfig (0);
		for (iview = 0;iview < *pNumViewports; iview++)
		{
			if (pViewportsD[iview]->pTheme)
			{
				if (pViewportsD[iview]->pTheme->ID == GF_GRAPHICS_FUNCTION_THEME)
				{
					if (FileNo == ifile)
					{   
						LPTHEME	SaveTheme=CurTheme;
						
						FileNo++; 
						CurTheme = pViewportsD[iview]->pTheme;
						GetGFFile (pFile,pViewportsD[iview]->pTheme->SQL,2); 
						CurTheme = SaveTheme;
						goto Top;
					}
					ifile++;
				}
			}
		}
	}
	if (!ProcessedCurView)  //keydefs in curview only used if not in menus
	{
		ProcessedCurView = TRUE;
		CurView = SaveVP;
		CurTheme = SaveTheme;
    	_fstrcpy (pFile,CurView->FunctionFile); 
    	goto Top;
    }

RtnFalse:
	GSSiGlobUlFree (&hStr);
	if (!CurrentConfig)
		SetConfig (1);
	CurView = SaveVP;
	CurTheme = SaveTheme;
{
#if ENABLETRACE
GSSiExitProg (450);
#endif
		return FALSE;
}
#if ENABLETRACE
}
#endif
}

UINT GetOPENERR00(void)
#if ENABLETRACE
{GSSiEnterProg (455);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (455);
#endif
	return OPENERR00;
}
#if ENABLETRACE
}
#endif
}
BOOL ProcessGraphicsFunction (HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1338);
#endif
{   
	int	st,ii;
	int		CmdLim;
	LPCMDSTRING    pCmdStr;
   	HCURSOR	hcurSave=0; 
	POINT	CursorLoc; 
	long	TotLen;
	
/*	if (Message == WM_COMMAND) retrn (FALSE);*/
//	if (!HavePaint) retrn (FALSE);
	if (!*pNumViewports)
{
#if ENABLETRACE
GSSiExitProg (1338);
#endif
		return (FALSE);
}
	if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (1338);
#endif
		return(FALSE); 
}   
	if (Message == GF_REMOVE_FUNCTION)
	{
		RemoveGraphicsFunction (hWnd,wParam);
{
#if ENABLETRACE
GSSiExitProg (1338);
#endif
		return TRUE;
}   
	}
	if (Message == GF_CLEAR_FUN_STACK)
	{
		ResetFunStack (TRUE);
{
#if ENABLETRACE
GSSiExitProg (1338);
#endif
		return TRUE;
}
	} 
	if (CurView->DisplayInParent && CurView->Parent && Message == WM_MOUSEMOVE)            	
//	 SetViewport(CurView->Parent); 
		ii=1;
		
/*	if (ProcessTurnArrows(hWnd,Message,wParam,lParam))
{
#if ENABLETRACE
GSSiExitProg (1338);
#endif
		return (TRUE);
} */
	if (MarginPan(hWnd,Message,wParam,lParam))
{
#if ENABLETRACE
GSSiExitProg (1338);
#endif
		return (TRUE);
}   
		if (Message == 514)
          ii=1;
	st = ProcessGraphicsFunction2 (CurView->CurrentFunction,hWnd,Message, wParam,lParam); 
	if (CurView && CurView->FunStackHandle)
	{
	    pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle);  
	    CmdLim = pCmdStr->CommandLimit;
	    GlobalUnlock (CurView->FunStackHandle);  
		if (hHighlight2)
			TotLen = BT_NUM_IN_INDEX (hHighlight2);
		else
			TotLen = 0;
	    if (st == GF_READY_TO_PROCESS && !TotLen)
	    	st = 1;
		if (st == GF_READY_TO_PROCESS && CmdLim == USHRT_MAX)
		{   
			AddLBUTTON = FALSE;
			if (TotLen)
			{   
				short	pos=BT_FIRST; 
				long	Ref,Sequence, CurLoc=0; 
				HANDLE	hMem = GSSiGlobAlloc (1235,GMEM_MOVEABLE,sizeof(HIGHLIGHTDATA)+sizeof(PICKDATA));
				LPHIGHLIGHTDATA	pHighlightData=(LPHIGHLIGHTDATA)GlobalLock (hMem);   
				LPPICKDATA	pSavePickList0=(LPPICKDATA)(pHighlightData+1);
				LPVIEWPORT	SaveVP=CurView; 
				
				*pSavePickList0 = PickList[0];
			    while (!BT_FIND (hHighlight2,(LPSTR)&Sequence,pos,BT_ANY,(LPSTR)&Ref))
			    {   
			    	pos = BT_NEXT; 
				    if (!BT_FIND (hHighlight,(LPSTR)&Ref,BT_FIRST,BT_EQ,(LPSTR)pHighlightData))
				    { 
			    		PickList[0] = pHighlightData->PD;  
			    		SetCurView ( SaveVP);
						if (ProcessGraphicsFunction2 (CurView->CurrentFunction,hWnd,GF_EXECUTE, 1,lParam) == GF_EXECUTE_CANCELED)
							break;
					}
					if (TotLen > 1 && GetGlobalBVal2("[%SHOWSTATUS]",TRUE)) 
					{
						if (!CurLoc)
							CreateStatusWind (CurView->hWnd,1,"Command Execution from Highlight List");
						StatusWindowUpdate (0,0, TotLen, CurLoc++);
					}
					else if (TotLen)
	                {
						hcurSave = GSSiSetCursor(LoadCursor(0, IDC_WAIT)); 
					}
				} 
				SetCurView ( SaveVP);
				ProcessGraphicsFunction2 (CurView->CurrentFunction,hWnd,GF_EXECUTE_FINISHED, wParam,lParam); 
				if (TotLen > 1 && GetGlobalBVal2("[%SHOWSTATUS]",TRUE))
					DestroyStatusWindow (0);
				else if (hcurSave)
				{
					GSSiSetCursor(hcurSave);
				}
				PickList[0] = *pSavePickList0;   
				GSSiGlobUlFree (&hMem);
			} 
		    pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle);  
			pCmdStr->CommandLimit = 0;
		    GlobalUnlock (CurView->FunStackHandle);  
			PostMessage(hWnd, GF_CLOSE,0, 0L);	
		}
		else if (st == GF_INCREASE_SUCCESS_COUNT)
		{
		    pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle);  
			if (pCmdStr->CommandLimit)
			{   
				if (pCmdStr->CommandLimit == USHRT_MAX-1) //use command coord one time
					pCmdStr->CommandLimit = 0;
				pCmdStr->CommandSuccessCount++; 
				if (pCmdStr->CommandSuccessCount >= pCmdStr->CommandLimit)
					PostMessage(hWnd, GF_CLOSE,0, 0L);	
			} 
		    GlobalUnlock (CurView->FunStackHandle);  
		}
		else if (st == GF_DECREASE_SUCCESS_COUNT)
		{
		    pCmdStr = (LPCMDSTRING)GlobalLock (CurView->FunStackHandle);  
			if (pCmdStr->CommandLimit && pCmdStr->CommandSuccessCount)
				pCmdStr->CommandSuccessCount--; 
		    GlobalUnlock (CurView->FunStackHandle);  
		}  
		else if (st == -1 && pCmdStr->CommandLimit)
		{   
			PostMessage(hWnd, GF_CLOSE,0, 0L);	
		}
	}
	if (st)
{   
			
#if ENABLETRACE
GSSiExitProg (1338);
#endif
		return TRUE;  
}
	if (Message == GF_CLOSE)
	{
		if (wParam != GF_ONEPICK)
			DisableMarginPan=FALSE; 
//		PostMessage (hWnd,GF_REMOVE_FUNCTION,wParam,0);
		RemoveGraphicsFunction (hWnd,wParam); //reinstated 8/24/02 - movesize viewport redisplays vp which changes curview
	}
	else if (Message == GF_ENTER_VIEWPORT)
	{
	}  
	else if ((Message == WM_RBUTTONUP && !(wParam & MK_LBUTTON)) ||
			 Message == WM_LBUTTONDBLCLK) 
	{   
		short	VPID;
		POINT	ButtonPoint = POINTStoPOINT(MAKEPOINTS(lParam));
		
		if (PtInInfoBoxRect (ButtonPoint,&VPID))   
		{ 
			AddGraphicsCmd (CurView->hWnd,"182",FALSE,0);
		}
		else if (*CurView->RButFunction)
			AddGraphicsCmd (CurView->hWnd,CurView->RButFunction,FALSE,0);
{
#if ENABLETRACE
GSSiExitProg (1338);
#endif
		return TRUE;
} 
	}
	else if (Message == WM_KEYDOWN)
	{ 
		switch (wParam)
		{   
			case 37://left arrow 
			case 38://up arrow
			case 39://right arrow
			case 40://down arrow     
			{
				WORD	FunctionKey = 2000+wParam-36;
				
				GetCursorPos (&CursorLoc);  
				ScreenToClient (hWnd,&CursorLoc);
				if (!SelectViewport (CursorLoc,TRUE,FALSE,FALSE)) 
                	SetViewport(*pCommandViewport);
                if (IsGFunctionKey (FunctionKey,FALSE))
                {
			    	LPSTR	pCmd=GlobalLock (hAddGraphicsFun); 
				    short	lcmd = _fstrlen (pCmd);
				    HANDLE	hCmd2 = GSSiGlobAlloc (1236,GMEM_MOVEABLE,lcmd+1);
				    LPSTR	pCmd2 = GlobalLock (hCmd2);
				    
				    _fstrcpy (pCmd2,pCmd);            	
			    	GSSiGlobUlFree (&hAddGraphicsFun);
					CurrentLBUTDOWNLoc=CursorLoc; 
                	HaveCurrentLBUTTON=TRUE;
                	AddGraphicsCmd (hWndMain,pCmd2,TRUE,0); 
                	GSSiGlobUlFree (&hCmd2);

{
#if ENABLETRACE
GSSiExitProg (1338);
#endif
                	return TRUE;
}
                } 
            }
				break;
			case 112://F1 
				break;
			case 113://F2
			case 114://F3
			case 115://F4
			case 116://F5
			case 117://F6
			case 118://F7
			case 119://F8
			case 120://F9
			case 121://F10
			case 122://F11
			case 123://F12 
			{  
				WORD	FunctionKey = 1000+wParam-111;
				
				GetCursorPos (&CursorLoc);  
				ScreenToClient (hWnd,&CursorLoc);
				if (!SelectViewport (CursorLoc,TRUE,FALSE,FALSE)) 
                	SetViewport(*pCommandViewport);
                if (IsGFunctionKey (FunctionKey,FALSE))
                {
			    	LPSTR	pCmd=GlobalLock (hAddGraphicsFun); 
				    short	lcmd = _fstrlen (pCmd);
				    HANDLE	hCmd2 = GSSiGlobAlloc (1237,GMEM_MOVEABLE,lcmd+1);
				    LPSTR	pCmd2 = GlobalLock (hCmd2);
				    
				    _fstrcpy (pCmd2,pCmd);            	
			    	GSSiGlobUlFree (&hAddGraphicsFun);
					CurrentLBUTDOWNLoc=CursorLoc; 
                	HaveCurrentLBUTTON=TRUE;
                	AddGraphicsCmd (hWndMain,pCmd2,TRUE,0); 
                	GSSiGlobUlFree (&hCmd2);

{
#if ENABLETRACE
GSSiExitProg (1338);
#endif
                	return TRUE;
}
                } 
            }
				break;
			default:
				ii=1;  
			break;
		}
	}
	else if (Message == WM_CHAR)
    {
		switch (wParam)
		{   
			case 17: //CNTLQ
				PostMessage(hWnd, GF_CANCEL,0, 0L);
				PostMessage(hWnd, GF_CLOSE,0, 0L);
			break;
			
			case 27: //ESC 
			{   
				short	i,j;
//				PostMessage(hWnd, GF_CLOSE,0, 0L); 
            	SetGlobalValue ("%DIGCURSOR",""); 
				SetConfig (1);   
				ProcessErrorString ();	
				for (j=0;j<2;j++)
				{
					if (NumViewportsArray[j])
					{
						SetConfig(j);
						for (i=0;i<*pNumViewports;i++)
						{
							SetViewport (i+1);
							ResetFunStack(TRUE); 
						}
					}
				}
				setDoPaint( TRUE);
				DisableHalt = FALSE;    
				SetContinueProcessing ( TRUE); 
				ResetOriginalDrive ();
            	SetGlobalValue ("%NEXTCVAL",""); 
            	HaltMapDisplay (TRUE,TRUE);
            	CacheAlreadyChecked (0,0,0);
			}	
            break;

            default:
				GetCursorPos (&CursorLoc);  
				ScreenToClient (hWnd,&CursorLoc);
				if (!SelectViewport (CursorLoc,TRUE,FALSE,FALSE)) 
                	SetViewport(*pCommandViewport);
				SetGlobalValue ("%PICKED_VIEWPORT",CurView->Name);   
                if (IsGFunctionKey (wParam,FALSE))
                {
			    	LPSTR	pCmd=GlobalLock (hAddGraphicsFun); 
				    short	lcmd = _fstrlen (pCmd);
				    HANDLE	hCmd2 = GSSiGlobAlloc (1238,GMEM_MOVEABLE,lcmd+1);
				    LPSTR	pCmd2 = GlobalLock (hCmd2);
				    
				    _fstrcpy (pCmd2,pCmd);            	
			    	GSSiGlobUlFree (&hAddGraphicsFun);
					CurrentLBUTDOWNLoc=CursorLoc; 
                	HaveCurrentLBUTTON=TRUE;
	            	HaltMapDisplay (FALSE,TRUE);
                	AddGraphicsCmd (hWndMain,pCmd2,TRUE,0); 
                	GSSiGlobUlFree (&hCmd2);

{
#if ENABLETRACE
GSSiExitProg (1338);
#endif
                	return TRUE;
}
                }
{
#if ENABLETRACE
GSSiExitProg (1338);
#endif
            	return FALSE;
}
            break;
		}  
{
#if ENABLETRACE
GSSiExitProg (1338);
#endif
		return TRUE;
}
	}
{
#if ENABLETRACE
GSSiExitProg (1338);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 

void ResetFunStack (BOOL All)
#if ENABLETRACE
{GSSiEnterProg (1339);
#endif
{
    HANDLE	hLast, hSave;
	LPCMDSTRING    pCmdStr;
	HANDLE	handle; 
	BOOL	First=TRUE;
	LPSTR	pE;
    
	CancelWindowZoom();
    DisableMarginPan=FALSE;	
    UnlockCursor ();  
    EnlargeScreen (0,0);
    SetGlobalValue("%P","");	 
	if (CurView && CurView->FunStackHandle)
	{
		InReset = TRUE;
	    handle = hSave = CurView->FunStackHandle;
		while (handle)
		{   
	    	pCmdStr = (LPCMDSTRING)GlobalLock (handle); 
	    	if (!pCmdStr)
	    	{   
	    		if (handle == CurView->FunStackHandle)
	    			CurView->FunStackHandle = 0;
	    		break;
	    	}
	    	hLast = pCmdStr->PrevHandle;
	    	if (!First || All)
	    	{ 
				if (pCmdStr->CurFun) 
				{
				    ProcessGraphicsFunction2 (pCmdStr->CurFun,hWndMain, GF_CANCEL, 0, 0); 
				    ProcessGraphicsFunction2 (pCmdStr->CurFun,hWndMain, GF_CLOSE, 0, 0); 
				}
				GSSiGlobFree (&pCmdStr->hError);
				GSSiGlobFree (&pCmdStr->hWhile);
				if ((pE = strstr (pCmdStr->Cmd,"$E(")))
					ProcessText (pE);
				GSSiGlobUlFree (&handle);
			} 
			else
			{
				pCmdStr->PrevHandle = 0;
				GlobalUnlock (handle);
			}
			First = FALSE; 
			handle = hLast; 
			CurView->FunStackHandle = handle;
		}
		if (All)
		{   
			CurView->FunStackHandle = 0;
			CurView->CurrentFunction = 0; 
		}
		else
			CurView->FunStackHandle = hSave;
	}
    DisplayFunctionStack(); 
	SetCurs (0,FALSE);
    InReset = FALSE;
    if (CurView && CurView == CurPolyVP)
    { 
	   	GSSiGlobFree(&hCurPolyPoints);
	   	nCurPolyPoints=0; 
	} 
{
#if ENABLETRACE
GSSiExitProg (1339);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

int ProcessGraphicsFunction2 (short Function,HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
#if ENABLETRACE
{GSSiEnterProg (1340);
#endif
{
	LPVIEWPORT	SaveVP=CurView;  
	int			st;  
	HCURSOR		hCursor=0;
    
    st = ProcessGraphicsFunction3 (Function,hWnd,Message,wParam,lParam);  
    if (Message == GF_ENTER_VIEWPORT)
    	st=0;
	if (!st && Message == GF_ENTER_VIEWPORT)
	{   
        SetPrompt (GetFunStackPrompt(&hCursor),FALSE);
        if (!CursorIsLocked)
        	SetCurs (hCursor,FALSE);                     
    }
	if (ConfigLoaded && !IgnoreSelectVP)
		SetCurView ( SaveVP); 
	  
{
#if ENABLETRACE
GSSiExitProg (1340);
#endif
    return st;
}
#if ENABLETRACE
}
#endif
}


BOOL RunGFCommandFromFileAtLoc (LPSTR File,long CurLoc,BOOL SendCmd,short opt)
#if ENABLETRACE
{GSSiEnterProg (1341);
#endif
{   
	LPSTR	pGCmd, lpBar;  
	HFILE	Fid; 
	BOOL	rtn = FALSE, First=TRUE;
	HANDLE	hStr = GSSiGlobAlloc (1240,GMEM_MOVEABLE,4096);
	LPSTR	lpStr = GlobalLock (hStr);  
	BOOL	SaveIgnoreFileOpenError=IgnoreFileOpenError;
	int		iStack, lcmd;
	char	fullFileName[MAX_PATH];
    
   	GetGFFile (lpStr,File,opt);
Open: 
	IgnoreFileOpenError = TRUE;
	strcpy(fullFileName, lpStr);
	SubstituteDL(fullFileName, FALSE);
   	Fid  = GSSiOpenFile (lpStr,0,OF_READ); 
	IgnoreFileOpenError = SaveIgnoreFileOpenError;
	if (Fid==HFILE_ERROR)
	{
		if (First)
		{   
			LPSTR	pFunDir, NewName=lpStr + 256;
			First = FALSE;
			if ((pFunDir=_fstrstr (lpStr,"\\fundir\\")))
			{   
				sprintf (NewName,"[%%DL]%s",(pFunDir+1));
				_fstrcpy (lpStr,NewName);
				goto Open;
			}
		}
		MessageBox( GetFocus(), lpStr,"Unable to load function file", MB_OK);
		GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (1341);
#endif
		return FALSE;
}
	}
	First = TRUE;
	if (fgetstring (lpStr,4090,Fid))
	{
		if (!strchr(lpStr,'|'))
			ExpandText (lpStr);
	}
	GSSillseek (Fid,CurLoc,0);
   	GSSiGlobUlFree (&hAddGraphicsFun);
   	if (CurView) 
   		AddGraphicsFunVP = CurView->ID;
   	else
   		AddGraphicsFunVP = 0;
	hAddGraphicsFun = GSSiGlobAlloc (1239,GHND,USHRT_MAX);
	pGCmd = GlobalLock (hAddGraphicsFun);  
	                     	  
	while (fgetstring (lpStr,4090,Fid))
	{
		if (First || *lpStr == '|')
		{   
			Truncate (lpStr);
		 	if ((lpBar = _fstrchr(lpStr,'|')))
		 	{
				*lpBar++ = 0;
				while (*lpBar == '\t')
					lpBar++;
				_fstrcat (pGCmd,lpBar);  
				rtn = TRUE; 
				if (First)
				{   
					CreateUndoPoint (lpStr);
				}
			}
		} 
		else if (*lpStr == '#')
			continue;
		else
			break;
		First=FALSE;
	}
	GSSiClose2 (&Fid);
	iStack = AddToMacroStack (2,0,fullFileName,0,CurLoc);
	sprintf (strchr (pGCmd,0),"$E(%i)",iStack);
	lcmd = strlen(pGCmd);
	GSSiGlobUlFree (&hStr);
	GlobalUnlock (hAddGraphicsFun);  
	if (SendCmd)
    {
    	LPSTR	pCmd=GlobalLock (hAddGraphicsFun); 
	                	
    	HaveCurrentLBUTTON=FALSE; 
    	AddGraphicsCmd (hWndMain,pCmd,TRUE,0); 
    	GSSiGlobUlFree (&hAddGraphicsFun);
    } 
{
#if ENABLETRACE
GSSiExitProg (1341);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetGFIndex (LPSTR IndexPath,short opt)
#if ENABLETRACE
{GSSiEnterProg (1342);
#endif
{   
	char	str[256];
	LPSTR	pBS;
	
	if (!CurView || !CurTheme)
		return FALSE;
	if (opt > 1)
		_fstrcpy (str,CurTheme->DataFile);
	else
		_fstrcpy (str,CurView->FunctionDir);
	ExpandText (str);
	if ((pBS = _fstrrchr (str,'\\')))
	{
		_fstrcpy (IndexPath,str);
/*		++pBS;
		if (*pBS)
			_fstrcat (IndexPath,"\\");*/
	}
	else
	{   
		sprintf (IndexPath,"[%%DL]fundir\\%s",str);
		if (!_fstrchr (str,'.'))
			_fstrcat (IndexPath,".txt");
	} 
	ExpandText (IndexPath);
{
#if ENABLETRACE
GSSiExitProg (1342);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL GetGFFile (LPSTR File,LPSTR from,short opt)
#if ENABLETRACE
{GSSiEnterProg (1343);
#endif
{
	char	str[256], gfdir[256];
	LPSTR	lpBS;
	
	if (*from)
		_fstrcpy (str,from);
	else
		_fstrcpy (str,CurView->FunctionFile);
	ExpandText (str);
	if ((lpBS = _fstrrchr (str,'\\')))
		_fstrcpy (File,str);
	else
	{
		GetGFIndex (gfdir,opt);
		_fullpath (File,gfdir,MAX_PATH); 
		if ((lpBS = _fstrrchr (File,'\\')))
		{
			lpBS++;
			*lpBS = 0;
		}
		else
			*File = 0;
		_fstrcat (File,str);
	}
	if (!_fstrchr (File,'.'))
		_fstrcat (File,".txt");
{
#if ENABLETRACE
GSSiExitProg (1343);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

BOOL ExecuteGFFromTheme (void)
#if ENABLETRACE
{GSSiEnterProg (1344);
#endif
{
{
#if ENABLETRACE
GSSiExitProg (1344);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL SetGFThemeState (LPTHEME pTheme,LPSTR Title,LPSTR CommandLine)
{  
	short	SaveConfig = CurrentConfig;
	LPTHEME	SaveCurTheme = CurTheme;
	
	CurTheme = pTheme;
	if (*CurTheme->Title == '[')
		SetGlobalValue("%GFTHEMETITLE", Title);
	else
		strcpy (CurTheme->Title,Title);
	if (*CurTheme->Title == '[')
		SetGlobalValue("%GFTHEMEFUNFILE", CommandLine);
	else
		strcpy(CurTheme->SQL, CommandLine);
	SetConfig (CurTheme->Config);
	ThemeDisplayLegend2(3,0); 
	SetConfig (SaveConfig);  
	CurTheme = SaveCurTheme;
	return TRUE;
}

BOOL LoadFunctionLists (HWND hWndDlg,HMENU hMenu,short WantLine,short opt,BOOL sort) 
#if ENABLETRACE
{GSSiEnterProg (1345);
#endif
{
	char	str[512],path[MAX_PATH],curpath[MAX_PATH], str2[MAX_PATH+32], delim='\t';
	char	fileToDelete[MAX_PATH + 2];
	HFILE	Fid=HFILE_ERROR;
	LPSTR	lpBar;
	short	i,LineNo=0;
	BOOL	deleteFile = FALSE;
	
	if (opt == 3) 
	{
		LastGFConfig = CurrentConfig;
		LastGFTheme = CurTheme;      
	}
	if (opt > 1)
		delim = 0;
	GetGFIndex (str,opt);
	if (opt == 4)
	{
		if (WantLine == 1)
		{
			sprintf (str2,"$EDITFILE(%s,T)",str);
			ProcessText (str2);
			goto Exit;
		}
		else if (WantLine > 0)
			WantLine--;
	}
	if (sort)
	{
		char sortFile[MAX_PATH + 2];
		GSSiGetTempFileName(0, "gm", 0, sortFile);
		SortTextFile(str, sortFile, 512, TRUE);
		strcpy(str, sortFile);
		strcpy(fileToDelete, str);
		deleteFile = TRUE;
	}
	Fid = GSSiOpenFile (str,0,OF_READ);
	if (Fid == HFILE_ERROR)
	{  
		char	buffer[MAX_PATH];
				     	
		MessageBox( GetFocus(), _fullpath(buffer,str,sizeof(buffer)),"Unable to load function Index", MB_OK);
{
#if ENABLETRACE
GSSiExitProg (1345);
#endif
		return (FALSE);
}
	} 
			     
	GetGFFile (str2,CurView->FunctionFile,opt); 
	_fullpath (curpath,str2,sizeof(curpath));
	while (fgetstring (str,254,Fid))
	{
		LPSTR beginLine = str;
		if (sort)
		{
			LineNo = atoi(str);
			beginLine = strchr(str, '|');
			beginLine++;
		}
		LineNo++;
		lpBar = _fstrchr(beginLine,'|');
		if (lpBar)
		{
			*lpBar++ = delim; 
			GetGFFile (str2,lpBar,opt); 
			_fullpath (path,str2,sizeof(path));
		} 
		else
			*path = 0; 
		ExpandText (beginLine);
		Truncate (beginLine);
		if (*beginLine)
		switch (opt)
		{
			case 1:
				i=SendDlgItemMessage (hWndDlg,FUNCTION_LIST_LB,LB_ADDSTRING,0,(LPARAM)beginLine);
				if (!_fstricmp (path,curpath))
					SendDlgItemMessage (hWndDlg,FUNCTION_LIST_LB,LB_SETCURSEL,i,0); 
				break;
			case 2:
				if (!_fstricmp (path,curpath))
				{
					_fstrcpy (CurTheme->Title, beginLine);
					_fstrcpy (CurTheme->SQL,lpBar);
				}
				break;
			case 3:
			{
				UINT	CmdID=63800+LineNo+1;  
				AppendMenu (hMenu,MF_ENABLED|MF_STRING,CmdID, beginLine);
				break;
			} 
			case 4:
				if (LineNo == WantLine && lpBar)
				{   
					SetGFThemeState (CurTheme, beginLine,lpBar);
					goto Exit;
				}
				break;
		}
	}
Exit:
	GSSiClose2 (&Fid); 
	if (deleteFile)
		GSSiRemove(fileToDelete);
{
#if ENABLETRACE
GSSiExitProg (1345);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL LoadGFFunctionList (short WantLine)
#if ENABLETRACE
{GSSiEnterProg (1346);
#endif
{                          
	LPTHEME	SaveTheme = CurTheme;
	
	CurTheme = LastGFTheme; 
	if (CurrentConfig != LastGFConfig)
		SetConfig (LastGFConfig);
	LoadFunctionLists (0,0,WantLine,4,FALSE);
	CurTheme = SaveTheme;
{
#if ENABLETRACE
GSSiExitProg (1346);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL LoadGFFile (HWND hWndDlg,LPSTR InFile,short opt,BOOL FloatingTB)
#if ENABLETRACE
{GSSiEnterProg (1347);
#endif
{   
	LPSTR	lpBar, lpCarrot, pDesc;
	HANDLE	hStr = GSSiGlobAlloc (1241,GMEM_MOVEABLE,4096);
	LPSTR	lpStr = GlobalLock (hStr);
	HFILE	Fid;
	char	Title[512];
	long	CurLoc; 
	short	MaxClass=MAX_THEME_CLASSES; 
	BOOL	First=TRUE;  
	LPTHEME	SaveTheme;
	LPVIEWPORT	SaveVP;
	BOOL	SaveIgnoreFileOpenError=IgnoreFileOpenError;
	BOOL	SkipThisEntry,HaveFirstLine=FALSE;
	char	BMPath[MAX_PATH];
	int		iButton=0;
	
	switch (opt)
	{
		case 3:
		case 4:
		case 5://toolbar redisplay
			strcpy (lpStr,InFile);
		break;
		case 2:
		{
			if (CurTheme->NumDesiredClass)
				MaxClass = CurTheme->NumDesiredClass;   
			_fmemset (CurTheme->ClassCount,0,sizeof(CurTheme->ClassCount));   
			_fmemset (CurTheme->ClassBM,0,sizeof(CurTheme->ClassBM));  
			GetGFFile (lpStr,CurTheme->SQL,opt); 
		}
		break;
		case 6://just process settings at top of file
		case 1:
			GetGFFile (lpStr,CurView->FunctionFile,0); 
	}
	_fstrlwr (lpStr);
Open: 
	IgnoreFileOpenError = TRUE;
	Fid = GSSiOpenFile (lpStr,0,OF_READ);
	IgnoreFileOpenError = SaveIgnoreFileOpenError;
	if (Fid==HFILE_ERROR)
	{   
		if (First)
		{   
			LPSTR	pFunDir, NewName=lpStr + 256;
			First = FALSE;
			if ((pFunDir=_fstrstr (lpStr,"\\fundir\\")))
			{   
				sprintf (NewName,"[%%DL]%s",(pFunDir+1));
				_fstrcpy (lpStr,NewName);
				goto Open;
			}
		}
		MessageBox( GetFocus(), lpStr,"Unable to load function file", MB_OK);
		GSSiGlobUlFree (&hStr);
{
#if ENABLETRACE
GSSiExitProg (1347);
#endif
		return (FALSE);
}
	}
	AddToMacroStack(6, 0, lpStr, 0, 0);
	if (opt == 3)
	{
		_splitpath (lpStr,0,0,Title,0);
		SetGlobalValue("[%MENUTITLE]", Title);
	}
	CurLoc = 0;
	if (opt == 3 && !FloatingTB)
	{
		char	bmPath[6];
		UINT	idBmp = IDB_UNDOCKTOOL_HOR;

		*bmPath = 1;
		memmove (&bmPath[1],&idBmp,sizeof(UINT));
		//AddButtonToToolbar (hWndDlg,bmPath,"",-1,&iButton);
	}
	while (fgetstring (lpStr,4090,Fid))
	{   
		if (!CurLoc && opt == 2)
		{   
			SetGlobalValue ("%ICONLIB",""); 
			SaveTheme = CurTheme;
			if (*lpStr == '[')
				ExpandText (lpStr);
			CurTheme = SaveTheme;
			if (GetGlobalCVal ("[%ICONLIB]",CurTheme->IconLibrary,0))
				_fstrcat (CurTheme->IconLibrary,"\\");
		}
		if (*lpStr == '#')
			goto NextLine;
		lpBar = _fstrchr(lpStr,'|');
		if (!lpBar)
		{
			SaveTheme = CurTheme;
			SaveVP = CurView;
			ExpandText(lpStr);
			CurTheme = SaveTheme;
			CurView = SaveVP;
			if (!ContinueProcessing)
			{
				PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
				SetContinueProcessing(TRUE);
				GSSiClose2(&Fid);
				GSSiGlobUlFree(&hStr);
				{
#if ENABLETRACE
					GSSiExitProg(1347);
#endif
					return TRUE;
				}
			}
			if (*lpStr == '[')
				goto NextLine;
		}
		else if (opt == 6)
			break;
		SkipThisEntry=FALSE;
		if (!_fstrncmp (lpStr,"IF(",3))
		{
			LPSTR IfExp = lpStr + 3;
			LPSTR pEnd = MatchLev (IfExp,')');
			BOOL	rc;
			
			if (pEnd && *(pEnd+1)!='{')
			{   
				*pEnd++ = 0;
				lpStr = pEnd;
				SkipThisEntry = !LogicP (IfExp,&rc);
			}
		}
		if (*lpStr != '|')
		{    
			LPSTR	lpAt;
			
			if (lpBar)
		 	{   
		 		char	key[256]; 
					 		
		 		*lpBar++ = 0; 
		 		_fstrcpy (Title,lpStr); 
		 		if ((lpAt = _fstrchr (lpStr,'&')))
		 			*lpAt = 0;
		 		if ((lpCarrot = _fstrchr (lpStr,'^'))) 
		 		{   
		 			*lpCarrot++ = '\t';
		 			_fstrcpy (key,lpCarrot); 
		 			if ((pDesc=_fstrchr (key,'/')))
		 				*pDesc = 0;
		 			sprintf (lpCarrot,"(%s)\t%ld",key,CurLoc);
		 		}
		 		else
		 		{
					sprintf (_fstrchr(lpStr,0),"\t\t%ld",CurLoc); 
				}
			}
			else if (*lpStr || !HaveFirstLine)
				goto NextLine;
			else
				_fstrcpy (lpStr,"\t\t-1"); 
			HaveFirstLine = TRUE;
			switch (opt)
			{
				case 1:
					//ExpandText (Title);
					//SendDlgItemMessage (hWndDlg,ACTIVE_FUN_LB,LB_ADDSTRING,0,(LPARAM)Title);
					ExpandText (lpStr);
					SendDlgItemMessage (hWndDlg,ACTIVE_FUN_LB,LB_ADDSTRING,0,(LPARAM)lpStr);
					break;
				case 2:
					if (!lpBar)
						break;
					CurTheme->ClassCount[CurTheme->NumClass] = CurLoc+1; 
					if (SkipThisEntry)
						CurTheme->ClassCount[CurTheme->NumClass] *= -1;	
					ExpandText(Title);
					if (_fstrlen(Title) > 127)
					{
						if (_fstrlen (Title) > 127) 
							_fstrcpy (CurTheme->ClassBM[CurTheme->NumClass++],"Title too long");
						else
							_fstrcpy (CurTheme->ClassBM[CurTheme->NumClass++],Title);
					}
					else
						_fstrcpy (CurTheme->ClassBM[CurTheme->NumClass++],Title);
					if (CurTheme->NumClass >= MaxClass)
						goto Exit;
					break;
				case 3:
					if (!lpBar)
						break;
					if (!SkipThisEntry)
					{
						LPSTR	lpBM;

						GetGlobalCVal ("[%ICONLIB]",BMPath,"[%DL]icons");
						Truncate(BMPath); // allows BMPath to be ' ' so full pathname can be entered after &
						ExpandText (Title);
						if ((lpBM = strrchr(Title, '&')))
						{
							*lpBM++ = 0;
							if (*BMPath)
								sprintf(strchr(BMPath, 0), "\\%s", lpBM);
							else
								sprintf(strchr(BMPath, 0), "%s", lpBM);
							AddButtonToToolbar(hWndDlg, BMPath, Title, CurLoc, &iButton);
						}
						else
						{
							AddButtonToToolbar(hWndDlg,0, Title, CurLoc, &iButton);
						}
					}
					break;
				case 4:
				case 5:
					if (!lpBar)
						break;
					if (!SkipThisEntry)
					{
						LPSTR	lpBM;

						GetGlobalCVal("[%ICONLIB]", BMPath, "[%DL]icons");
						Truncate(BMPath); // allows BMPath to be ' ' so full pathname can be entered after &
						ExpandText(Title);
						if ((lpBM = strrchr(Title, '&')))
						{
							*lpBM++ = 0;
							if (*BMPath)
								sprintf(strchr(BMPath, 0), "\\%s", lpBM);
							else
								sprintf(strchr(BMPath, 0), "%s", lpBM);
							if (opt == 5)
								RedisplayButtonToCMDMenu(hWndDlg, BMPath, Title, iButton);
							else
								AddButtonToCMDMenu(hWndDlg, BMPath, Title, CurLoc);
						}
						else if (opt == 4)
							AddButtonToCMDMenu (hWndDlg,0,Title,CurLoc);
						iButton++;
					}
					break;
			}
		}
NextLine:
		CurLoc = GSSillseek (Fid,0,1);
	}
Exit: 
	if (opt == 2 && CurTheme->NumDesiredClass)
		CurTheme->NumClass = CurTheme->NumDesiredClass;	
	GSSiClose2 (&Fid);
	GSSiGlobUlFree (&hStr);
	if (opt == 3)
	{
		GetGlobalCVal("[%MENUTITLE]", Title, 0);
		SetWindowText(hWndDlg, Title);
	}

			 
{
#if ENABLETRACE
GSSiExitProg (1347);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

int ProcessGraphicsFunction4 (short Function,
							   HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{   
	if (Message == GF_CLOSE && wParam && Function != wParam)
		return FALSE;
	WalkOutRouteOpt=0; 
	switch (Function)
	{    
	    case GF_ZOOM_VARRECT:
	    case GF_ZOOM_RECT:
	    	 return (ZoomRectangle (hWnd,Message,wParam,lParam,Function));
	         break;
	    case GF_MOVE_TAG:
	    	 return (MoveTAG (hWnd,Message,wParam,lParam));
	         break;
	    case GF_SIZE_TAG:
	    	 return (SizeTAG (hWnd,Message,wParam,lParam));
	         break;
	    case GF_WINDOW_ZOOM:
	    	 return (WindowZoom (hWnd,Message,wParam,lParam));
	         break;
	    case GF_BLOWUP:
	    	 return (BlowUp (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_PAN_TO_POINT_NOINT:
	    case GF_PAN_TO_POINT:
	    	 return (PanToPoint (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_SHOW_ITEM:
	    	 return (ShowItem (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_HIDE_ITEM:
	    	 return (HideItem (hWnd,Message,wParam,lParam)); 
	    	 break; 
	    case GF_COLOR_CLASSTEXT:
	    case GF_COLOR_CLASS:   
	    	 ForAllVis=FALSE;
	    	 return (ColorClass (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_COLOR_CLASS_ALLVIS:
	    case GF_COLOR_CLASSTEXT_ALLVIS:  
	    	 ForAllVis=TRUE;
	    	 return (ColorClass (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_LINE_TYPE_CLASS:    
	    	 ForAllVis=FALSE;
	    	 return (LineTypeClass (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_LINE_TYPE_CLASS_ALLVIS: 
	    	 ForAllVis=TRUE;
	    	 return (LineTypeClass (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_CREATE_TAG:
	    	 return (CreateTAG (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_PICK_SCATPOINT:
	    	 return (PickScatterPoint (hWnd,Message,wParam,lParam)); 
	    case GF_SET_CLASS_COLOR:
	    	 return (ThemeSVChangeColor (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_PICK_IMAGE:
	    	 return (PickImage (hWnd,Message,wParam,lParam));
	    	 break;  
		case GF_AUTO_IDENTIFY:
			ii = 1;
		case GF_TOOLBAR:
	    case GF_RUN_CLASS_MACRO:
		case GF_AUTOPICK: 
		case GF_AUTOPICK_NOZOOM:
		case GF_ONEPICK:
		case GF_RUN_COORDDISPLAY_MACRO:
		case GF_RUN_DISTDISPLAY_MACRO:
		case GF_RUN_LEGEND_MACRO: 
		case GF_SHOW_DIFFERENCE:
	    case GF_PAN_ZOOM_TARGET:
	    	 return (PanZoomTarget (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_THEME_ACTIVATE:
	    	 return (ThemeActivate (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_THEME_DEACTIVATE:
	    case GF_THEME_REMOVE:
	    	 return (ThemeDeActivate (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_THEME_EDIT:
	    	 return (ThemeEdit (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_THEME_LOAD:
	    	 return (ThemeLoad (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_MOVE_INTERSECTION:
	    	 return (MoveIntersection (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_ZOOM_IN:
	    	 return (ZoomIn (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_SMOOTH_ZOOM:
	    	 return (SmoothZoom (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_CREATE_TILES:
	    	 return (CreateTiles (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_ZOOM_OUT:
	    	 return (ZoomOut (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_MOVE_TEXTBOX:
	    	 return (MoveTextBox (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_EDIT_TEXTBOX_TEXT:
	    case GF_DELETE_TEXTBOX:
	    case GF_EDIT_TEXTBOX:  
	    case GF_SET_TEXTBOX_FACTOR:
	    	 return (EditTextBox (hWnd,Message,wParam,lParam,Function)); 
	    	 break;  
	    case GF_SELECT_HIGHLIGHT_AREA:
	    case GF_OFFSET_AREA:
	    	 return (OffsetAreaHP (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_CHANGE_CLASS_VIS:
	    	 return (ChangeClassVis (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_CHANGE_REDEF_COLOR:   
	    	 ForAllVis=FALSE;
	    	 return (ChangeRedefColor (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_CHANGE_PEN_COLOR:   
	    	 ForAllVis=FALSE;
	    	 return (ChangePenColor (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_CHANGE_PEN_NUMBER:   
	    	 ForAllVis=FALSE;
	    	 return (ChangePenNumber (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_MOVESIZE_VIEWPORT:
	    	 return (MoveSizeViewport(hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_EDIT_VIEWPORT:
	    	 return (EditViewport (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_LOAD_USGSORTHO:
	    	 MessageBox (GetFocus(),"Use the Load DOQs graphics import function"," ",MB_OK); 
	    	 break;
	    case GF_ADJUST_ORTHO_COLORS:
	    	 return (AdjustOrthoColors (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_SELECT_ORIGINAL_ORTHO:
	    	 return (SelectOrigOrtho (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_DISPLAY_ALL_ORTHOS:
	    	 CurView->OrthoDisplayName[0]=0; 
			 PostMessage(hWnd, GF_CLOSE,0, 0L); 
			 //RemoveGraphicsFunction (hWnd,0);
		     CurView->CurZoomAreaRef = 0;
			 PostMessage(hWnd, WM_COMMAND, IDM_Z_REDRAW, CurView->ID);
	    	 break;
	    case GF_ADJUST_DISPLAYED_ORTHOS:
	    	 return (AdjustOrthos (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_ADD_DOCUMENT:
	    	 return (AddDocument (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_DISPLAY_DOCUMENT:
	    	 return (DisplayDocument (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_DIGITIZE_POLYLINE:
	    	 return (CreatePoly (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_CREATE_HLT_AREA:
	    	 return (CreateHLTArea (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_CREATE_NEW_AREA:   
	    	 NewPolyType = 0;
	    	 return (CreateNewPolyline (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_CREATE_NEW_POLYLINE:
	    	 NewPolyType = 1;
	    	 return (CreateNewPolyline (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_CREATE_NEW_POINT:
	    	 return (CreateNewPoint (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_SHOW_DIST:
	    case GF_SHOW_AREA:
		case GF_SHOW_AZ:
	    	 return (DistancePolyline (hWnd,Message,wParam,lParam,Function)); 
	    	 break; 
	    case GF_UNHIGHLIGHT:
	    case GF_TOGGLE_HIGHLIGHT:
	    case GF_HIGHLIGHT:
	    	 return (HighlightItem (hWnd,Message,wParam,lParam,Function));
	    	 break;    
	    case GF_CLEAR_AND_HIGHLIGHT:
	    	 if (Message == WM_LBUTTONUP) 
	    	 {
				ClearHighlightList(FALSE); 	    	 
	    	 	return (HighlightItem (hWnd,Message,wParam,lParam,Function));
	    	 }
	    	 break;    
	    case GF_HIGHLIGHT_SEQUENTIAL:
			 RouteType = TWOPOINTROUTE;
	    	 return (HighlightSequentialItems (hWnd,Message,wParam,lParam,Function));
	    	 break;    
	    case GF_HIGHLIGHT_SEQUENTIAL_CLOSED:
			 RouteType = CLOSEDTWOPOINTROUTE;
	    	 return (HighlightSequentialItems (hWnd,Message,wParam,lParam,Function));
	    	 break;    
	    case GF_PTP_ROUTE:
			 RouteType = TWOPOINTROUTENET;
	    	 return (HighlightSequentialItems (hWnd,Message,wParam,lParam,Function));
	    	 break;    
	    case GF_WALKOUT_ROUTE:
	    case GF_CONNECTIVITY_TEST:
	    case GF_BUILD_NETWORK:
	    	 WalkOutRouteOpt = Function;
			 RouteType = WALKOUTROUTE;
	    	 return (HighlightSequentialItems (hWnd,Message,wParam,lParam,Function));
	    	 break;    
	    case GF_SET_STREET_NAME:
	    	 return (SetStreetName (hWnd,Message,wParam,lParam));
	    	 break;    
	    case GF_DEFINE_STREET_LINK_AUTO:
	    case GF_DEFINE_STREET_LINK:
	    	 return (CreateNetLink (hWnd,Message,wParam,lParam,Function));
	    	 break;  
	    case GF_DISPLAY_NET_INFO:
	    	 return (DisplayNetInfo (hWnd,Message,wParam,lParam));
	    	 break;  
	    case GF_DISPLAY_NET_MARKERS:
	    	 return (DisplayNetMarkers (hWnd,Message,wParam,lParam));
	    	 break;  
	    case GF_DELETE_NET_MARKER:
	    	 return (DeleteNetMarker (hWnd,Message,wParam,lParam));
	    	 break;  
	    case GF_EDIT_PICKMACRO:
	    	 return (EditPickMacro (hWnd,Message,wParam,lParam));
	    	 break;  
	    case GF_CLEAR_REDEF:
	    	 return (ClearRedef (hWnd,Message,wParam,lParam));
	    	 break; 
	    case GF_CLEAR_MASK: 
	    case GF_TOGGLE_FUNSTACK:
	    	 return (ToggleFunStack (hWnd,Message,wParam,lParam,Function));
	    	 break;  
	    case GF_SET_COMMAND_VIEWPORT:
	    	 return (SetCommandViewport (hWnd,Message,wParam,lParam));
	    	 break;  
	    case GF_SHOW_POLY_POINTS:
	    	 return (ShowPolyPoints (hWnd,Message,wParam,lParam));
	    	 break;    
	    case GF_SAVE_RDF:
	    	 return (SaveRedefFile (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_LOAD_RDF:
	    	 return (LoadRedefFile (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_EDIT_GRAPHICS_CMDSTRING:
	    	 return (EditCmdString (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_SETTAG:
	    	 return (SetTAGGF (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_SETSYM:
	    	 return (SetSYMGF (hWnd,Message,wParam,lParam)); 
	    	 break;     
	    case GF_CHANGE_DESC:
	    	 return (ChangeDesc (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_SPLIT_STREET_SEGMENT:
	    case GF_SPLIT_SEGMENT:
	    	 return (SplitSegment (hWnd,Message,wParam,lParam,Function)); 
	    	 break; 
	    case GF_SPLIT_POLYGON:
	    	 return (SplitPolygon (hWnd,Message,wParam,lParam,Function)); 
             break;
	    case GF_REPLACE_POLY_POINTS:
	    	 return (ReplacePolyPoints (hWnd,Message,wParam,lParam,Function)); 
             break;
	    case GF_DELETE_HIGHLIGHTED_ITEMS:
	    case GF_UNDELETE_HIGHLIGHTED_ITEMS: 
	    case GF_REMOVE_HIGHLIGHTED_ITEMS:
	    	 return (DeleteItems (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_DELETE_STREET_NETWORK:  
	    	 CurLinkID = -1;
	    	 return (DeleteStreetNet (hWnd,Message,wParam,lParam,Function));
	    	 break;
	    case GF_DELETE_STREET_LINK:
	    	 return (DeleteStreetNet (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_SELECT_STREET_LINK:
	    	 return (SelectStreetLink (hWnd,Message,wParam,lParam)); 
	    	 break; 
	    case GF_SNAP_CURSOR:
	    case GF_SNAP_ON:
	    case GF_SNAP_END: 
	    case GF_SNAP_END_POINT: 
	    case GF_SNAP_BEGIN_POINT:    
	    case GF_SNAP_NODE_POINT:    
	    case GF_SNAP_MID_POINT:  
	    case GF_SNAP_CHORD_MID_POINT:  
		case GF_SNAP_RADIUS_POINT:
	    case GF_SNAP_AVE_POINT:  
	    case GF_SNAP_AVE_HLTPOINT:  
	    case GF_SET_COORD:
		case GF_SET_VPCOORD:
		case GF_SNAP_DIST_AND_DIR:
		case GF_SNAP_COORD: 
		case GF_SNAP_LATLONG:
	    case GF_SNAP_UNLOCK:
	    case GF_SNAP_COLOR: 
		case GF_SNAP_BLACK_EDGE:
		case GF_SNAP_BLACK_MID:
	    case GF_UNDO:
	    case GF_LBUTTON:
	    case GF_SET_POC:
	    case GF_RBUTTON:
	    	 return (SnapTo (hWnd,Message,wParam,lParam,Function));
	    	 break;    
	    case GF_EDIT_ADDRESS:
	    	 return (EditAddress (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_ADD_STREET_NAME:
	    	 return (AddSegStreetName (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_LOAD_SINGLE_MARKER:
	    case GF_LOAD_MARKERS_HLT:   
	    case GF_DELETE_MARKERS_HLT:
	    	 return (LoadMarkersFromHighlight (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_ADD_INTS_TO_NET:
	    	 return (AddIntsToNet(hWnd,Message)); 
	    	 break;
	    case GF_PROCESS_GRAPHICS_CMDSTRING:
	    	 return (ProcessCmdString (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_SET_PHOTO_TRANS:
	    	 return (SetPhotoTrans (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_TRACK_PHOTO_LOC:
	    	 return (TrackPhotoLoc (hWnd,Message,wParam,lParam)); 
	    	 break;
		case GF_SELECT_STREET_TEMPLATE:
		case GF_SET_NULL_TEMPLATE:
		case GF_CREATE_STREET_SEGMENT:
	    	 return (SelectStreetTemplate (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_TRACK_LOC_IN_OTHER_VP:
	    	 return (TrackLocInOtherVP (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_CREATE_NULL_DIR:
	    case GF_CREATE_NULL_MAP:
	    	 return CreateNullMap (hWnd,Message,Function);
	    	 break;	 
	    case GF_CREATE_REFCONNECT_TABLE:	 
	    	 return (CreateRefConnectTable (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_REFCONNECT_FIX:	 
	    	 return (RefConnectFix (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_REFCONNECT_OUTPUT:	 
	    	 return (RefConnectOutput (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_ID_POLYGONS:	 
	    	 return (IdentifyPolygons (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_EDIT_TEXT: 
	    case GF_CHANGE_TEXT_COLOR:
	    	 return (EditText (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_SPONGE:
	    	 return (Sponge (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_EDIT_TEXT_MULTIPLE:
	    	 return (EditTextMultiple (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_ADDEDIT_TAG:
	    	 return (AddEditTAG (hWnd,Message,wParam,lParam)); 
	    	 break; 
	    case GF_FACTOR_TEXT:
	    case GF_EDIT_TEXT_HEADER: 
	    case GF_SET_TEXT_GLOBALS:
	    	 return (EditTextHeader (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_ADD_TURN_DATA:
	    	 return (AddTurnData (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_OFFSET_POLYLINES:
	    	 return (OffsetPolylinesHLT (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_ORTHO_FILTER:
	    	 return (OrthoFilterFunction (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_ADJUST_BITMAP_COLORS:
	    	 return (AdjustBitmapColors (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_HLT_BY_CLASS:
	    	 return (HighlightByClass (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_ADD_CIRCLE_POLY:
	    	 return (AddCirclePolyline (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_ADD_RECTANGLE_POLY:
	    	 return (AddRectanglePolyline (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_SELECT_DIST:
	    	 return (SelectDist (hWnd,Message,wParam,lParam)); 
	    	 break;
		case GF_DRAG_DIST:
			return (DragDist(hWnd, Message, wParam, lParam));
			break;
		case GF_GRAPHICS_MACRO:
			return (GraphicsMacroFunction(hWnd, Message, wParam, lParam));
			break;
			//case GF_GOLF_SHOT:
	    	 //return (GolfShot (hWnd,Message,wParam,lParam)); 
	    	 //break;
	    case GF_POINTS_FROM_HLT:
	    case GF_AREA_FROM_HLT:
		case GF_COPY_POLY:
	    	 return (PointsFromHLT (hWnd,Message,wParam,lParam,Function)); 
	    	 break;  
	    case GF_SLIDE_SCREEN:
	    	 return (SlideScreen (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_TRAVERSE_ENTRY:
	    	 return (TraverseEntry (hWnd,Message,wParam,lParam)); 
	    	 break;
		case GF_CHANGE_POINT_SIZE:
			 return (ResizePoint (hWnd,Message,wParam,lParam,Function)); 
	    case GF_ROTATE_SYMBOLS:  
	    	 return (RotateSymbols (hWnd,Message,wParam,lParam,Function)); 
             break;
	    case GF_FACTOR_SYMBOL:  
	    	 return (EditSymbol (hWnd,Message,wParam,lParam,Function)); 
             break;
	    case GF_POINTS_FROM_OFFSET:
	    	 return (PointsFromOffset (hWnd,Message,wParam,lParam)); 
	    	 break;  
	    case GF_LINFIT: 
	    case GF_HORZFIT:
	    case GF_VERTFIT:
	    	 return (PointsFromLinfit (hWnd,Message,wParam,lParam,Function)); 
	    	 break;  
	    case GF_SPLIT_AT_INTERSECTION:
	    case GF_SNAP_INTERSECTION:
	    	 return (SnapToIntersection (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_RELOCATE_POINT:  
	    	 return (RelocatePoint (hWnd,Message,wParam,lParam,Function)); 
             break;
	    case GF_TRACE_SPILL:  
	    	 return (TraceSpill (hWnd,Message,wParam,lParam,Function)); 
             break;
	    case GF_TRAVPOINT_ADJUST:  
	    	 return (AdjustTravPoint (hWnd,Message,wParam,lParam,Function)); 
             break;
        case GF_AREA_AROUND_POINT:
	    	 return (CreateAreaAroundPoint (hWnd,Message,wParam,lParam)); 
	    	 break;  
	    case GF_SET_CLASS_SYMBOL:
	    	 return (ThemeChangeSymbol (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_SET_CLASS_FACTOR:
	    	 return (ThemeChangeFactor (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_SET_CLASS_WIDTH:
	    	 return (ThemeChangeWidth (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_SHOW_CLASS_MEMBERS:
	    	 return (ThemeShowClassMembers (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_EDGE_MATCH:
	    	 return (EdgeMatchLines (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_MOVE_POINT:  
	    	 return (MovePoint (hWnd,Message,wParam,lParam,Function)); 
             break;
	    case GF_REFCONNECT_OUTPUT_LINES:	 
	    	 return (RefConnectOutputLines (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_SELECT_BY_CLASS:
	    	 return (SelectByClass (hWnd,Message,wParam,lParam)); 
	    	 break; 
	    case GF_ASSIGN_NEWREF:
	    case GF_CHANGE_REFNO:
	    	 return (ChangeRefno (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_IDENTIFY_LEG:
	    	 return (IdentifyTraverseLeg (hWnd,Message,wParam,lParam,Function)); 
	    	 break;  
	    case GF_CHANGE_LINE_COLOR:
	    case GF_CHANGE_BRUSH_COLOR:
	    	 return (EditBrushColor (hWnd,Message,wParam,lParam,Function)); 
	    	 break;  
	    case GF_SNAP_TO_POLY:
	    	 return (SnapToPolyline (hWnd,Message,wParam,lParam)); 
	    	 break;
		case GF_REMOVE_POLY_POINT:
	    case GF_REDEFINE_POLY:
	    	 return (RedefinePolyline (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_EDIT_RDF:
	    	 return (EditRedefData (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_ADD_HLTAREA_POLY:
	    	 return (AddHltAreaPolyline (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_AUTO_SPLINE:
	    	 return (AutoSplinePoints (hWnd,Message,wParam,lParam)); 
	    	 break;
		case GF_LOAD_PICKABILITY:
		case GF_LOAD_VISIBILITY:
		case GF_SET_VISIBILITY:
		case GF_SET_PICKABILITY:
		case GF_SPLIT_VIEWPORT:
		case GF_QUAD_VIEWPORT:
	    	 return (SetViewportParms (hWnd,Message,wParam,lParam,Function)); 
	    case GF_SIZE_CLASSTEXT:
	    case GF_SIZE_POINT:
	    	 ForAllVis=FALSE;
	    	 return (TextSizeClass (hWnd,Message,wParam,lParam,Function)); 
	    	 break;
	    case GF_SIZE_CLASSTEXT_ALLVIS:  
	    case GF_SIZE_POINT_ALLVIS:  
	    	 ForAllVis=TRUE;
	    	 return (TextSizeClass (hWnd,Message,wParam,lParam,Function)); 
	    	 break;  
	    case GF_REMOVE_ITEM:
        case GF_DELETE_ITEM:
        	 return (DeleteItem (hWnd,Message,wParam,lParam,Function));
			 break;
        case GF_COPYWITHOFFSET:
        	 return (CopyPolylineWithOffset (hWnd,Message,wParam,lParam,Function));
			 break;
	    case GF_LEGEND_SETUP:
	    	 return (LegendSetup (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_LEGEND_SELECT:
	    	 return (LegendSelect (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_EDIT_CITYLOC:
	    	 return (EditCityLoc (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_REMOVE_POLY_LOOP:
	    	 return (RemovePolyLoop (hWnd,Message,wParam,lParam,Function)); 
             break;
	    case GF_SET_LAYER_COLOR:
	    case GF_REMOVE_LAYER_COLOR:
	    	 return (SetLayerColor (hWnd,Message,wParam,lParam,Function)); 
             break;
        case GF_TRACE_DOWNSTREAM:
	    	 return (TraceDownstream (hWnd,Message,wParam,lParam,Function)); 
             break;
	    case GF_TOGGLE_CLASS_STATUS:
	    	 return (ThemeToggleClassStatus (hWnd,Message,wParam,lParam)); 
	    	 break;
	    case GF_PNPARMS:
	    	 return (SetPNParms (hWnd,Message,wParam,lParam)); 
	    	 break;	
		case GF_MOVE_OFFSETLINE:
			 return (MoveOffsetLine (hWnd,Message,wParam,lParam,Function));
			 break;
		case GF_IMAGEZOOM:
			return (ImageZoom(hWnd, Message, wParam, lParam));
			break;
		case GF_SCREENZOOM:
			return (ScreenZoom(hWnd, Message, wParam, lParam));
			break;
		case GF_DISPLAY_DATED_ORTHOS:
			return (DisplayDatedOrthos(hWnd, Message, wParam, lParam, Function));
			break;
  }
   return (FALSE);
} 
int ProcessGraphicsFunction3(short Function,
	HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	int rtn;
	LPVIEWPORT SaveVP = CurView;

	if (Message == WM_TIMER)
	{
		int vpid = HIWORD(wParam);
		int lo = LOWORD(wParam);

		if (vpid)
		{
			SetViewport(vpid);
			Function = CurView->CurrentFunction;
			wParam = lo;
		}
	}
	else if (Message == GF_MAPSERVER_RESPONSE)
	{
		int vpid = HIWORD(wParam);
		int lo = LOWORD(wParam);

		if (vpid)
		{
			SetViewport(vpid);
			Function = CurView->CurrentFunction;
			wParam = lo;
		}
	}
	else if (Message == GF_MAPSERVER_READY || Message == GF_MAPSERVER_FAILED)
	{
		int vpid = lParam;

		if (vpid)
		{
			SetViewport(vpid);
			Function = CurView->CurrentFunction;
			lParam = 0;
		}
	}

	rtn = ProcessGraphicsFunction4(Function, hWnd, Message, wParam, lParam);

	CurView = SaveVP;
	return rtn;

}

void DisplayGFList (void)
{
	HMENU	ApMenu = CreatePopupMenu(); 
	POINT	position;
		
	if (GetDebug ())
	{
		UINT	CmdID=63801;  
		AppendMenu (ApMenu,MF_ENABLED|MF_STRING,CmdID,"Edit Menu"); 
	}
    LoadFunctionLists (0,ApMenu,-1,3,TRUE);
   	GetCursorPos (&position);  
   	if (CurView && GetMenuItemCount(ApMenu))
  		TrackPopupMenu (ApMenu,TPM_CENTERALIGN|TPM_LEFTBUTTON,position.x,position.y,0,CurView->hWnd,0);
  	DestroyMenu (ApMenu);  
  	return;
} 
 

