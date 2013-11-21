#include "graphint.h"   
#include "extrndb.h"
#include "client.h"

#include "gmextern.h"

static	char	MonthAbv[12][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static	BOOL	InAtPrint=FALSE;
static short	nSetVals=0;
static UINT	lSetVals=0;
static char	CurrentDialogType[16];
static char	DOW[7][10]={"SUNDAY","MONDAY","TUESDAY","WEDNESDAY","THURSDAY","FRIDAY","SATURDAY"};
static HIGHLIGHTDATA	HighlightData;


int	GetFunctionValue (int FunID,LPSTR Args, LPSTR OutLoc)
#if ENABLETRACE
{GSSiEnterProg (1348);
#endif
{   HANDLE	hMem=0,hDLT,hSurf;
	LPSTR	Arg1, Arg2, Arg3, Arg4, Arg5, Arg6,Arg7, ParLoc, lpstr, lpstrb;  
	BOOL	FromLimits, Immediate; 
	LPSTR	pEnd, pCR, pFile, Arg[16], pVal;
	short	nArgs;
	HFILE	Fid1, Fid2, Fid3;   
	int		l, CvtDir, year,i;    
	DPOINT	Point, Point2;
	POINT	Point16;
	RECT	Rect; 
	MNMXCORD	Bounds,Bounds2;           
	long	hNum, Refno,ii, nlong;
	LPSTR	lpColon;
	time_t	systime; 
	BOOL	TORF, rtn, Err, PickVis;  
	LPVIEWPORT	SaveVP=CurView;   
	short	SaveCfg = CurrentConfig;
	short SymNum, Mode, irc;			
	short	n, n1,n2,pos, nrem;  
	double	AZ, Dist,Elevation, RVal;
	HFILE	Fid;
	LPDPOINT	pPoint;
	HANDLE	hDibInfo;
	long	ImageOffset; 
	double	Offset;
	HANDLE	hSQL;
	HANDLE	hTran;
    LPOPENFILEDATA  FilePtr;
    LPOPENSQLDATA   SQLPtr;
	LPFIELDINFO	lpFieldInfo;
	LPSQLFIELD	lpSQLField;
	LPFILEPATH	FilePathPtr;
	LPHANDLE	lpFileHandle; 
	short	ndec; 
	
	if (LinkToVar)
	{
		ExpandText (Args);
{
#if ENABLETRACE
GSSiExitProg (1348);
#endif
		return 0;
}
	}
	if (TraceOn)
	{
		hMem=GSSiGlobAlloc ( 785,GMEM_MOVEABLE,4096);
		lpstr = GlobalLock (hMem);
		sprintf (lpstr,"GFV:%s(%s)",LastFunctionName,Args);
		GSSiTraceLev (lpstr,1,2);
		GSSiGlobUlFree (&hMem);
	}
	switch (FunID)
	{    
		case 101: /* $D(str) displays str */
		{				
			DWORD	TextExt;  
			int		l,i,x;
			LPSTR	loc,endloc,tab;
			
			hMem = GSSiGlobAlloc ( 786,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);    
			if (FidDisplay != HFILE_ERROR)
			{
				fputstring (Arg1,FidDisplay);
				goto RtnTrue;
			}
			if (!CurReport)
				goto RtnFalse;
			if (CurReport->hScrollLine)
			{    
				LPSTR	pLine;     
				
				pLine = GlobalLock (CurReport->hScrollLine); 
				if (_fstrlen (pLine) + _fstrlen (Arg1) < 1020)
					_fstrcat(pLine,Arg1);
				GlobalUnlock (CurReport->hScrollLine);
				goto RtnTrue;
			}
			if (CurReport->hDC && CurFont && CurFont <= CurReport->NumFonts)  
			{
				SelectObject (CurReport->hDC,CurReport->hFonts[CurFont-1]);
				SetTextColor (CurReport->hDC,CurReport->FontColor[CurFont-1]); 
			}
			loc = Arg1;
			endloc = _fstrchr (Arg1,0);
			while (loc < endloc)
			{ 
				tab = _fstrchr (loc,'\t');
				if (tab)
				{   
					*tab = 0;
					for (i=0;i<CurReport->NumTabs;i++)
					{
						if (CurReport->x < CurReport->TabLen[i]*DeviceToScreenFactor)
						{
							CurReport->x = CurReport->TabLen[i]*DeviceToScreenFactor;
							break;
						}
					}
				}
				else
					tab=endloc;
				l = _fstrlen (loc);
				if (l)   
				{   
					char just[8];
					
					GetGlobalCVal ("[%JUST]",just,"L");
					TextExt = GetTextExtent (CurReport->hDC,loc,l);
					switch (just[0])
					{
						default:
						case 'l':
						case 'L':
							break;
						case 'c':
						case 'C':
							CurReport->x = CurReport->Rect.left + (CurReport->Rect.right - CurReport->Rect.left)/2 - LOWORD(TextExt)/2;
							break;
						case 'R':
						case 'r':
							CurReport->x = CurReport->Rect.right - LOWORD(TextExt);
							break;
					} 
					x = CurReport->x;
					//TextOut (CurReport->hDC,CurReport->x,CurReport->y,loc,l);
		    		CurReport->x += LOWORD(TextExt); 
		    		if (CurReport->WantSize)
		    		{   
		    			POINT	p;
		    			
		    			p.x = x;
		    			p.y = CurReport->y;
		    			AddPointToRect (p,&CurReport->SizeRect);
		    			p.x = CurReport->x;
		    			p.y = CurReport->y+HIWORD(TextExt);
		    			AddPointToRect (p,&CurReport->SizeRect);
		    		}
		    		else
						ExtTextOut (CurReport->hDC,x,CurReport->y,0,NULL,loc,l,NULL);	
				}
				loc = tab+1;
			}

			goto RtnTrue;
		}
		
		case 102: /* $L(num) increases line counter by num*/
		{	
			int	nlines;
						
			hMem = GSSiGlobAlloc ( 787,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			if (!CurReport)
				goto RtnFalse;
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			nlines = max (1,atoi (Arg1));  
			if (CurReport->hScrollLine)
			{
				LPSTR	pLine;     
				
				pLine = GlobalLock (CurReport->hScrollLine); 
				while (nlines--) 
				{   
					if (ScrollRptDlg == 1)
						fputstring (pLine,ScrollRptCntl);
					else if (ScrollRptDlg)
			 			SendDlgItemMessage (ScrollRptDlg,ScrollRptCntl,LB_ADDSTRING,(WPARAM)NULL,(LPARAM)pLine); 
					*pLine = 0; 
				}
				GlobalUnlock (CurReport->hScrollLine);
				goto RtnTrue;
			}
			CurReport->y += nlines * CurrentReportFontHeight (); 
			if (!CurReport->WantSize && UseMultReportPages && (CurReport->y+CurrentReportFontHeight ()) > CurReport->Rect.bottom) 
			{
				NextPage (CurReport->hDC);  
				if (Printing)
					CurReport->y = CurReport->Rect.top;  
			}
			CurReport->x = CurReport->Rect.left;
    		if (CurReport->WantSize)
    		{   
    			POINT	p;
		    			
    			p.x = CurReport->x;
    			p.y = CurReport->y;
    			AddPointToRect (p,&CurReport->SizeRect);
    		}

			goto RtnTrue;
		} 
		
		case 103: /* $X(str) extracts x value from point */
		{				
			
			hMem = GSSiGlobAlloc ( 788,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			*OutLoc = 0;
			if ((pEnd = _fstrchr (Arg1,' ')))
			{
				*pEnd = 0;
				_fstrcpy (OutLoc,Arg1);
			}
			goto Rtnl;   
		}
		
		case 104: /* $Y(str) extracts x value from point */
		{				
			
			hMem = GSSiGlobAlloc ( 789,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			*OutLoc = 0;
			if ((pEnd = _fstrchr (Arg1,' ')))
			{
				*pEnd++ = 0;
				_fstrcpy (OutLoc,pEnd);
			}
			goto Rtnl;   
		}
		
		case 201: /* $OS(string) */
		{
			BOOL LastWasBlank, HaveNonBlank=FALSE; 
			LPSTR	SaveOutLoc; 
			
			hMem = GSSiGlobAlloc ( 790,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			LastWasBlank = FALSE;  
			SaveOutLoc = OutLoc;
			while (*Arg1)
			{
				if (*Arg1 != ' ')
				{   
					if (LastWasBlank && HaveNonBlank)
						*OutLoc++ = ' '; 
					LastWasBlank = FALSE;  
					HaveNonBlank=TRUE;
					*OutLoc++ = *Arg1;
				}                        
				else
					LastWasBlank = TRUE;
				Arg1++;
			}
			*OutLoc = '\0';  
			OutLoc = SaveOutLoc;
			goto Rtnl;
		}
		
		case 202: /* $LV(view) returns left view */
		{	double	rval; 
			int		iview;
			int		ndec, ntoadd; 
			
			hMem = GSSiGlobAlloc ( 791,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			if (!rread (Arg1,&rval,&ndec)) goto Rtn0;
			iview = IDNINT(rval); 
			if (iview == 3)
				iview = 1; 
			else
				iview = 5;
			itoa (iview,OutLoc,10);

			goto Rtnl;
		}
		
		case 203: /* $RV(view) returns right view */
		{	double	rval; 
			int		iview;
			int		ndec, ntoadd; 
			
			hMem = GSSiGlobAlloc ( 792,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			if (!rread (Arg1,&rval,&ndec)) goto Rtn0;
			iview = IDNINT(rval); 
			if (iview == 3)
				iview = 5; 
			else
				iview = 1;
			itoa (iview,OutLoc,10);

			goto Rtnl;
		}
		
		case 204: /* $VP(SET,name)sets current viewport
					or
						(SHOW,name) displays current viewport. If xcord > 1000 subtracts 1000   
					or
						(ACTIVATE,name) runs but sets adds 1000 to xcord to move offscreen
					or
						(HIDE,name) hides current viewport 
					or	
						(DISPLAY,name,immediate) redisplays viewport
					or
						(REZOOM,name) rezooms viewport
					or  
						(DIM,name,x,y,w,h)
					or
						(ZOOM, 
					or	(INACTIVE,name) returns 0 or 1                   
				 */
		{	
			short	SaveCurrentConfig=CurrentConfig;
			
			nArgs = GetFunArgs (Args,Arg,6,&hMem); 
			
			SaveVP = CurView; 
			if (*Arg[2])
				SetCurView ( SetVPFromName (Arg[2],&Err));
			if (Err)
				SetViewport (*pCommandViewport);
			if (!CurView)
				goto RtnFalse;	
			if (!_fstrcmp (Arg[1],"SET"))
			{   
			    LPCMDSTRING	pCmdStr=NULL;
			    
			    if (CurView == SaveVP)
    				goto RtnTrue;
    			if (InGRFCmd)
    			{
				    if (SaveVP->FunStackHandle)
				    {
				    	LPCMDSTRING	pCmdStr = (LPCMDSTRING)GlobalLock (SaveVP->FunStackHandle); 
						LPSTR		pLoc = pCmdStr->Cmd; 
						
						pLoc += pCmdStr->CurLoc; 
						if (*pLoc == ';')
							pLoc++;
						if (*pLoc)
						{   
							LPSTR	pGCmd; 
							long	lcmd = _fstrlen (pLoc);
							short	Inc = 0;
							
						   	GSSiGlobFree (&hAddGraphicsFun2);
					   		AddGraphicsFunVP = CurView->ID;
							hAddGraphicsFun2 = GSSiGlobAlloc ( 793,GHND,lcmd+1);
							pGCmd = GlobalLock (hAddGraphicsFun2); 
							_fstrcpy (pGCmd,pLoc);
							GlobalUnlock (hAddGraphicsFun2);  
							pCmdStr->CurLoc = pCmdStr->EndLoc;
							GlobalUnlock (SaveVP->FunStackHandle);
							if (CurrentConfig)
								Inc = 1000;
							PostMessage(hWndMain, GSSI_ADDGF, 0, CurView->ID+Inc); 
						}
						else
							GlobalUnlock (SaveVP->FunStackHandle);
					}
					SetCurView ( SaveVP);
				}
			
//				if (InGRCmd) goto Rtn0;    
//				IgnoreSelectVP = TRUE;
			}
			else if (!_fstrcmp (Arg[1],"GETVAL"))
			{  
				if (Err)
					goto RtnFalse;
				if (!_fstricmp (Arg[3],"HAVELAYERCOLOR"))
				{
					for (i=0;i<CurView->NumFiles;i++)
					{
						if (!_fstricmp (Arg[4],CurView->FileID[i]))
						{
							if (CurView->HaveLayerColor[i])
								goto RtnTrue;
							goto RtnFalse;
						}
					}
				}
				else if (!_fstricmp (Arg[3],"HALFTONE"))
				{
					itoa (CurView->HalfTone,OutLoc,10);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"RECT"))
				{   
					Rect = CurView->Rect;
					ClientRectToScreenRect (CurView->hWnd,&Rect);
					recttoa (OutLoc,Rect);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"TEXTOPAQUE"))
				{
					if (CurView->pTheme)
					{   
						if (CurView->pTheme->ID == GF_STREET_TEXT_THEME)
						{
						    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurView->pTheme->ClassBM;   
						    
							if (pStreetData->BackgroundOpt)
								goto RtnTrue;
						}
					} 
				}
				goto RtnFalse;
			}
			else if (!_fstrcmp (Arg[1],"SETVAL"))
			{  
				if (Err)
					goto RtnFalse;
				if (!_fstricmp (Arg[3],"HAVELAYERCOLOR"))
				{
					for (i=0;i<CurView->NumFiles;i++)
					{
						if (!_fstricmp (Arg[4],CurView->FileID[i]))
						{
							CurView->HaveLayerColor[i] = atob (Arg[4]);
							goto RtnTrue;
						}
					}
				}
				else if (!_fstricmp (Arg[3],"RBUT"))
				{
					_fstrncpy (CurView->RButFunction,Arg[4],128);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"HALFTONE"))
				{
					CurView->HalfTone = atoi (Arg[4]);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"TEXTOPAQUE"))
				{
					if (CurView->pTheme)
					{   
						if (CurView->pTheme->ID == GF_STREET_TEXT_THEME)
						{
						    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurView->pTheme->ClassBM;   
						    
							if (*Arg[4] == 'R') 
								pStreetData->BackgroundOpt = !pStreetData->BackgroundOpt;
							else
								pStreetData->BackgroundOpt = atob (Arg[4]);
							goto RtnTrue;
						}
					} 
				}
				goto RtnFalse;
			} 
			else if (!_fstrcmp (Arg[1],"AREA"))
			{  
				if (Err)
					goto RtnFalse; 
				RVal = (CurView->WBounds.xmx - CurView->WBounds.xmn) * (CurView->WBounds.ymx - CurView->WBounds.ymn);
				ftoa (OutLoc,RVal); 
				goto Rtnl;
			}
			else if (!_fstrcmp (Arg[1],"EXIST"))
			{  
				if (Err)
					goto RtnFalse;
				goto RtnTrue;
			}
			else if (!_fstrcmp (Arg[1],"GRAY"))
			{ 
			    CurView->ConvertToGray = TRUE;    
			 	CurView->HalfToneNewObjectStart = CurView->NumNewObjects;
			    DeleteCacheDir ();
				goto RtnTrue;
			}
			else if (!_fstrcmp (Arg[1],"COLOR"))
			{ 
			    CurView->ConvertToGray = FALSE;
			 	CurView->HalfToneNewObjectStart = 0;
			    DeleteCacheDir ();
				goto RtnTrue;
			}
			else if (!_fstrcmp (Arg[1],"SHOW"))
			{
				CurView->Active = TRUE; 
				CurView->Display = TRUE; 
				if (CurView->pTheme)
					CurView->pTheme->VPDisplayed = TRUE;
				if (CurView->TagPoint.x > 1000)
					CurView->TagPoint.x -= 1000;
				SetCurView ( SaveVP);
			}
			else if (!_fstrcmp (Arg[1],"ISACTIVE"))
			{
				rtn = CurViewActive ();
				SetCurView ( SaveVP);  
				if (rtn)
					goto RtnTrue;
				goto RtnFalse;
			}
			else if (!_fstrcmp (Arg[1],"ACTIVATE"))
			{
				CurView->Active = TRUE; 
				if (CurView->TagPoint.x < 1000)
					CurView->TagPoint.x += 1000;
				SetCurView ( SaveVP);
			}
			else if (!_fstrcmp (Arg[1],"TOGGLE"))
			{
				CurView->Active = !CurView->Active;
				SetCurView ( SaveVP);
			}
			else if (!Err && !_fstrcmp (Arg[1],"HIDE"))
			{
				CurView->Active = FALSE; 
				CurView->ProfileInCrossSection=FALSE;
				SetCurView ( SaveVP);
			}
			else if (!_fstrcmp (Arg[1],"DISPLAY"))
			{   
				BOOL SaveDisableHalt = DisableHalt;
				
//				DisableHalt = TRUE;
				if (!atob (Arg[3]))
					PostMessage(CurView->hWnd, WM_COMMAND, IDM_Z_REDRAW, CurView->ID); 
				else
		         	RedisplayViewport (TRUE,FALSE);  
//	         	DisableHalt = SaveDisableHalt;
				SetCurView ( SaveVP);
			}
			else if (!_fstrcmp (Arg[1],"MULTIZOOM")) 
			{   
				MultiZoomBegin (atoi (Arg[3]),atoi (Arg[4]),1);
				SetCurView ( SaveVP);
			}
			else if (!_fstrcmp (Arg[1],"REZOOM"))
			{   
				double	dist;
				
				if (!GetVisBounds (&Bounds,CurView->hDC))
					goto RtnFalse; 
				dist = min(Bounds.xmx - Bounds.xmn,Bounds.ymx - Bounds.ymn);
				dist *= 0.01;
				Bounds.xmn -= dist;
				Bounds.ymn -= dist;
				Bounds.xmx += dist;
				Bounds.ymx += dist; 
			    CurView->CurZoomAreaRef = LONG_MAX;
				ZoomToRect(Bounds,FALSE);
			}
			else if (!_fstrcmp (Arg[1],"DIM"))
			{
				CurView->TagPoint.x = atof (Arg[3]);
				CurView->TagPoint.y = atof (Arg[4]);
				CurView->Width = atof (Arg[5]);
				CurView->Height = atof (Arg[6]); 
				SetConfig (SaveCurrentConfig);
				SetCurView ( SaveVP);
			}   
			else
			{
				SetCurView ( SaveVP);
				goto RtnFalse;
			}
			goto RtnTrue;
		}
		default:
			goto Rtn0;
	}
Rtnrtn:
	if (!rtn)
		goto RtnFalse;
RtnTrue: 
	l = 1;                
	_fstrcpy(OutLoc,"1");
	goto Exit;
RtnFalse: 
	l = 1;
	_fstrcpy(OutLoc,"0");
	goto Exit;
Rtn0:
	l = 0;
	goto Exit;
Rtnl:
	l = _fstrlen(OutLoc); 
Exit: 
	if (SaveCfg != CurrentConfig)
	{
		SetConfig (SaveCfg);
		if (*pNumViewports)
			SetCurView ( SaveVP);
	}
	GSSiGlobUlFree (&hMem);
	if (TraceOn)
	{
		hMem=GSSiGlobAlloc ( 914,GMEM_MOVEABLE,4096);
		lpstr = GlobalLock (hMem);
		sprintf (lpstr,"GFV:%s",Args);
		GSSiTraceLev (lpstr,-1,2);
		GSSiGlobUlFree (&hMem);
	}
{
#if ENABLETRACE
GSSiExitProg (1348);
#endif
	return (l);
}
#if ENABLETRACE
}
#endif
}  

		
			
