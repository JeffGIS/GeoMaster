#include "graphint.h"   
#include "extrndb.h"
#include "client.h"

#include "gmextern.h"

static	char	MonthAbv[12][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static short	nSetVals=0;
static UINT	lSetVals=0;
static char	CurrentDialogType[16];
static char	DOW[7][10]={"SUNDAY","MONDAY","TUESDAY","WEDNESDAY","THURSDAY","FRIDAY","SATURDAY"};
static HIGHLIGHTDATA	HighlightData;  
static char	ContourOpts[6][18]={"LightContourColor","LightContourWidth","DarkContourColor","DarkContourWidth","ContourTextColor","ContourTextSize"};  

short OptionInList (LPSTR Val,LPSTR ListVals,int NumInList,int ListItemSize)
{   
	UINT	i,j;
	
	for (i=0;i<NumInList;i++,ListVals+=ListItemSize)
		if (!_fstricmp (Val,ListVals))
			return i+1;
	return 0;
}

static BOOL ConvertColorToTransparent(HDIB32 dib, COLORREF transColorIN)
{
	BOOL rtn = FALSE;
	int r = GetRValue(transColorIN);
	int g = GetGValue(transColorIN);
	int b = GetBValue(transColorIN);
	RGBQUAD rgbQuad;
	COLORREF *ptransColor = (COLORREF *)&rgbQuad;
	
	rgbQuad.rgbBlue = b;
	rgbQuad.rgbGreen = g;
	rgbQuad.rgbRed = r;
	rgbQuad.rgbReserved = 0;

	if (FreeImage_GetWidth(dib))
	{
		int bytespp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);
		if (bytespp == 4)
		{
			for (unsigned y = 0; y < FreeImage_GetHeight(dib); y++)
			{
				LPRGBQUAD pQuadColor = (LPRGBQUAD)FreeImage_GetScanLine(dib, y);
				COLORREF *pcolorRef = (COLORREF *)pQuadColor;
				for (unsigned x = 0; x < FreeImage_GetWidth(dib); x++)
				{
					pQuadColor->rgbReserved = 0;
					if (*pcolorRef == *ptransColor)
					{
						pQuadColor->rgbReserved = 0;
						rtn = TRUE;
					}
					else
						pQuadColor->rgbReserved = 255;
					pQuadColor++;
					pcolorRef++;
				}
			}
		}
	}
	return rtn;
}

int	GetFunctionValue(int FunID, LPSTR Args, LPSTR OutLoc, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen)
#if ENABLETRACE
{GSSiEnterProg (1348);
#endif
{   HANDLE	hMem=0,hDLT,hSurf;
	LPSTR	Arg1, Arg2, Arg3, Arg4, Arg5, Arg6,Arg7, ParLoc, lpstr, lpstrb;  
	BOOL	FromLimits, Immediate; 
	LPSTR	pEnd, pCR, pFile, Arg[20] = { 0 }, pVal;
	short	nArgs, layer;
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
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (FidDisplay != HFILE_ERROR)
			{
				fputstring (Arg[1],FidDisplay);
				goto RtnTrue;
			}
			if (!CurReport)
				goto RtnFalse;
			if (CurReport->hScrollLine)
			{    
				LPSTR	pLine;     
				
				pLine = GlobalLock (CurReport->hScrollLine); 
				if (_fstrlen (pLine) + _fstrlen (Arg[1]) < 1020)
					_fstrcat(pLine,Arg[1]);
				GlobalUnlock (CurReport->hScrollLine);
				goto RtnTrue;
			}
			CurReport->curLineHeight = max (CurrentReportFontHeight (),CurReport->curLineHeight);

			if (CurReport->hDC && CurFont && CurFont <= CurReport->NumFonts)  
			{
				SelectObject (CurReport->hDC,CurReport->hFonts[CurFont-1]);
				SetTextColor (CurReport->hDC,CurReport->FontColor[CurFont-1]); 
			}
			loc = Arg[1];
			endloc = _fstrchr (Arg[1],0);
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
					SIZE	txSize;
					
					GetGlobalCVal ("[%JUST]",just,"L");
					GetTextExtentPoint32 (CurReport->hDC,loc,l,&txSize);
					switch (just[0])
					{
						default:
						case 'l':
						case 'L':
							break;
						case 'c':
						case 'C':
							CurReport->x = CurReport->Rect.left + (CurReport->Rect.right - CurReport->Rect.left)/2 - txSize.cx/2;
							break;
						case 'R':
						case 'r':
							CurReport->x = CurReport->Rect.right - txSize.cx;
							break;
					} 
					x = CurReport->x;
					//TextOut (CurReport->hDC,CurReport->x,CurReport->y,loc,l);
		    		CurReport->x += txSize.cx; 
		    		if (CurReport->WantSize)
		    		{   
		    			POINT	p;
		    			
		    			p.x = x;
		    			p.y = CurReport->y;
		    			AddPointToRect (p,&CurReport->SizeRect);
		    			p.x = CurReport->x;
		    			p.y = CurReport->y+txSize.cy;
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
						
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);

			if (FidMacroOutput != HFILE_ERROR)
			{
				fputstring (Arg[1],FidMacroOutput);
				goto RtnTrue;
			}
			if (!CurReport)
				goto RtnFalse;
			nlines = max (1,atoi (Arg[1]));  
			if (CurReport->hScrollLine)
			{
				LPSTR	pLine;     
				
				pLine = GlobalLock (CurReport->hScrollLine); 
				while (nlines--) 
				{   
					if (ScrollRptDlg == (HWND)1)
						fputstring (pLine,ScrollRptCntl);
					else if (ScrollRptDlg)
			 			SendDlgItemMessage (ScrollRptDlg,ScrollRptCntl,LB_ADDSTRING,(WPARAM)NULL,(LPARAM)pLine); 
					*pLine = 0; 
				}
				GlobalUnlock (CurReport->hScrollLine);
				goto RtnTrue;
			}
			CurReport->curLineHeight = max (CurrentReportFontHeight (),CurReport->curLineHeight);
			CurReport->y += CurReport->curLineHeight;
			CurReport->curLineHeight = 0;
			CurReport->y += (nlines-1) * CurrentReportFontHeight (); 
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
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if ((pEnd = _fstrchr (Arg[1],' ')))
			{
				*pEnd = 0;
				_fstrcpy (OutLoc,Arg[1]);
			}
			goto Rtnl;   
		}
		
		case 104: /* $Y(str) extracts x value from point */
		{				
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if ((pEnd = _fstrchr (Arg[1],' ')))
			{
				*pEnd++ = 0;
				_fstrcpy (OutLoc,pEnd);
			}
			goto Rtnl;   
		}
		
		case 105: //$C(val,i) returns 1 based character from val. i can be L or l for last char
			{
				nArgs = GetFunArgs(Args, Arg, 2, &hMem,pBrkPt, bpOffset,bpLen);
				l = strlen (Arg[1]);
				if (*Arg[2] == 'L' || *Arg[2] == 'l')
					i = l;
				else
					i = atoi (Arg[2]);
				i = max (1,min (i,l));
				OutLoc[0] = Arg[1][i-1];
				OutLoc[1] = 0;
				goto Rtnl;
			}
			break;

		case 106://$B(breakpointid)
			AtBreakPoint (Args,0);
			*OutLoc = 0;
			goto Rtnl;
			break;
		case 107://$E(funStackID)
			RemoveFromMacroStack (atoi(Args));
			*OutLoc = 0;
			goto Rtnl;
			break;
		case 201: /* $OS(string) */
		{
			BOOL LastWasBlank, HaveNonBlank=FALSE; 
			LPSTR	SaveOutLoc; 
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			LastWasBlank = FALSE;
			SaveOutLoc = OutLoc;
			while (*Arg[1])
			{
				if (*Arg[1] != ' ')
				{   
					if (LastWasBlank && HaveNonBlank)
						*OutLoc++ = ' '; 
					LastWasBlank = FALSE;  
					HaveNonBlank=TRUE;
					*OutLoc++ = *Arg[1];
				}                        
				else
					LastWasBlank = TRUE;
				Arg[1]++;
			}
			*OutLoc = '\0';  
			OutLoc = SaveOutLoc;
			goto Rtnl;
		}
		
		case 202: /* $LV(view) returns left view */
		{	double	rval; 
			int		iview;
			int		ndec, ntoadd; 
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (!rread(Arg[1], &rval, &ndec)) goto Rtn0;
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
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (!rread(Arg[1], &rval, &ndec)) goto Rtn0;
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
						(DIM,name,x,y,w,h) sets vp loc and with with standard VP coordinates
					or
						(SIZE,name,x,y,w,h) set vp loc with bitmap units
					or
						(ZOOM, 
					or	(INACTIVE,name) returns 0 or 1                   
				 */
		{	
			short	SaveCurrentConfig=CurrentConfig;
			
			nArgs = GetFunArgs(Args, Arg, 8, &hMem, pBrkPt, bpOffset, bpLen);
			
			SaveVP = CurView; 
			Err = 0;
			SetConfig(1);
			if (*Arg[2])
				SetCurView ( SetVPFromName (Arg[2],&Err));
			if (Err)
				SetViewport (*pCommandViewport);
			if (!CurView)
				goto RtnFalse;	
			if (!_fstrcmp (Arg[1],"EXISTS"))
			{  
				if (Err)
					goto RtnFalse;
				else
					goto RtnTrue;
			}
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
			else if (!_fstrcmp (Arg[1],"LAYER"))
			{ 
				if (!strcmp (Arg[3],"CREATE"))
				{
					rtn = AddLayerToViewport (CurView,Arg[4],Arg[5]);
					goto Rtnrtn;
				}
				if (!strcmp(Arg[3], "GETNUM"))
				{
					for (i = 0; i<CurView->NumFiles; i++)
					{
						if (!stricmp(CurView->FileID[i], Arg[4]))
						{
							itoa(i + 1, OutLoc, 10);
							goto Rtnl;
						}
					}
				}
				if (!strcmp(Arg[3], "GETPATH"))
				{
					for (i = 0; i<CurView->NumFiles; i++)
					{
						if (!stricmp(CurView->FileID[i], Arg[4]))
						{
							strcpy(OutLoc, CurView->lpFiles[i]);
							goto Rtnl;
						}
					}
				}
				if (!strcmp(Arg[3], "SETPATH"))
				{
					strcpy(OutLoc, "0");
					for (i = 0; i<CurView->NumFiles; i++)
					{
						if (!stricmp(CurView->FileID[i], Arg[4]))
						{
							strcpy(CurView->lpFiles[i],Arg[5]);
							strcpy(OutLoc, "1");
						}
					}
					goto Rtnl;
				}
				goto RtnFalse;
			}
			else if (!stricmp (Arg[1],"CREATE")) //$VP(CREATE,vpname,parent,type
			{
				if (*pNumViewports < MAX_VIEWPORTS)
				{
					HANDLE	hViewport;
					LPVIEWPORT pViewport = CreateNullViewport (&hViewports[*pNumViewports],Arg[2],*pNumViewports+1,Arg[3],Arg[4]); 

					pViewports[*pNumViewports] = pViewport;
					pViewportsD[*pNumViewports]=pViewports[*pNumViewports];  

					(*pNumViewports)++; 
					DetermineVPDisplaySequence ();
					goto RtnTrue;
				}
			}
			else if (!_fstrcmp (Arg[1],"SAVEIMAGE"))//$VP(SAVEIMAGE,vp,imagefile,coordfile(opt))
			{
				HBITMAP hbm;
				RECT	rect = CurView->DrawRect;
				int		imageFlag = atoi(Arg[7]);
				COLORREF transColor = atoi(Arg[8]);
				if (Err)
					goto RtnFalse;
				//ClientRectToScreenRect (CurView->hWnd,&rect);
				hbm = SaveScreen (CurView->hDC,rect);
				if (imageFlag < 0)
				{
					HDIB32 dib = BitmapToDIB_32(hbm, NULL);
					if (ConvertColorToTransparent(dib,transColor))
					{
						LPSTR pDot = strrchr(Arg[3], '.');
						strcpy(pDot, ".png");
						rtn = GMFIBMPHandleToEXT(Arg[3], dib, 0);
						FreeImage_Unload(dib);
					}
					else
					{
						HDIB32 dib24 = FreeImage_ConvertTo24Bits(dib);
						FreeImage_Unload(dib);
						rtn = GMFIBMPHandleToEXT(Arg[3], dib24, -imageFlag);
						FreeImage_Unload(dib24);
					}
				}
				else
					SaveBitmap (hbm,Arg[3],0,imageFlag);
				GSSiDeleteObject (&hbm);
				if (*Arg[4])
				{
					int iopt = atoi (Arg[5]);
					switch (iopt)
					{
					default:
						CreateBPWFromViewport (Arg[4]);
						break;
					case 4: //dump vp corners to file in Google Tile coord
						{
							DPOINT wpt;
							POINT pt;
							HFILE fidOut=GSSiOpenFile (Arg[4],0,OF_CREATE);
							int level = atoi (Arg[6]);

							if (fidOut == HFILE_ERROR)
								goto RtnFalse;
							pt.x = rect.left;
							pt.y = rect.top;
							wpt = WinPtToBasePt (pt);
							wpt = BasePtToGoogleTilePt (&wpt,level);
							sprintf (OutLoc,"%i %i %.14lg %.14lg",pt.x-rect.left,pt.y-rect.top,wpt.x,wpt.y); 
							fputstring (OutLoc,fidOut);
							pt.x = rect.right;
							pt.y = rect.top;
							wpt = WinPtToBasePt (pt);
							wpt = BasePtToGoogleTilePt (&wpt,level);
							sprintf (OutLoc,"%i %i %.14lg %.14lg",pt.x-rect.left,pt.y-rect.top,wpt.x,wpt.y); 
							fputstring (OutLoc,fidOut);
							pt.x = rect.right;
							pt.y = rect.bottom;
							wpt = WinPtToBasePt (pt);
							wpt = BasePtToGoogleTilePt (&wpt,level);
							sprintf (OutLoc,"%i %i %.14lg %.14lg",pt.x-rect.left,pt.y-rect.top,wpt.x,wpt.y); 
							fputstring (OutLoc,fidOut);
							pt.x = rect.left;
							pt.y = rect.bottom;
							wpt = WinPtToBasePt (pt);
							wpt = BasePtToGoogleTilePt (&wpt,level);
							sprintf (OutLoc,"%i %i %.14lg %.14lg",pt.x-rect.left,pt.y-rect.top,wpt.x,wpt.y); 
							fputstring (OutLoc,fidOut);
							GSSiClose (fidOut);
						}
						break;
					}
				}

				goto RtnTrue;
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
				else if (!_fstricmp (Arg[3],"GRAY"))
				{
					rtn = CurView->ConvertToGray;
					goto Rtnrtn; 
				}
				else if (!_fstricmp (Arg[3],"NUMBER"))
				{
					itoa (CurView->ID,OutLoc,10);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"ZOOMMACRODISABLED"))
				{
					itoa (CurView->DisableZoomMacro,OutLoc,10);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"HALFTONE"))
				{
					itoa (CurView->HalfTone,OutLoc,10);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"HALFTONEALL"))
				{
					btoa (CurView->AlwaysUseHalfTone,OutLoc);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"ZOOMLOCK"))
				{
					btoa (CurView->ZoomLocked,OutLoc);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"METERSPERPIXEL"))
				{
					ftoa (OutLoc,CurView->MetersPerPixel);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"MAXOFFSETDIST"))
				{
					ftoa (OutLoc,CurView->MaxOffsetDist);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"WIDTH"))
				{
					ftoa (OutLoc,CurView->Width);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"HEIGHT"))
				{
					ftoa (OutLoc,CurView->Height);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"TAGPOINTX"))
				{
					ftoa (OutLoc,CurView->TagPoint.x);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"TAGPOINTY"))
				{
					ftoa (OutLoc,CurView->TagPoint.y);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"RECT"))
				{   
					Rect = CurView->Rect;
					if (!atob(Arg[4]))
						ClientRectToScreenRect (CurView->hWnd,&Rect);
					recttoa (OutLoc,Rect);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"BOUNDS"))
				{   
					boundstoa (OutLoc,&CurView->WBounds);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"CURSORPOS"))
				{   
					Point = ScreenPtToBasePt (CurView->LastCursorPos);
					dpointtoa (OutLoc,&Point);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"MIDPOINT"))
				{   
					dpointtoa (OutLoc,&CurView->MidPointW);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"SCALE"))
				{   
					ftoa (OutLoc,CurView->Scale);
					goto Rtnl; 
				}
				else if (!_fstricmp (Arg[3],"ZMSCALE"))
				{   
					ftoa (OutLoc,CurView->ZMScale);
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
				if (!_fstricmp (Arg[3],"PROFILEROUTE"))
				{
					HANDLE	hPoints;
					int nPoints = GetPointsFromList (Arg[4],&hPoints);

					CurView->nProfileRoute[0] = 0;
					GSSiGlobFree (&CurView->hProfileRoute[0]);
					if (nPoints)
					{
						CurView->nProfileRoute[0] = nPoints;
						CurView->hProfileRoute[0] = hPoints;
						CurView->HaveFixedProfileRoute = TRUE;
					}
					else
						CurView->HaveFixedProfileRoute = FALSE;
				}
				else if (!_fstricmp (Arg[3],"PROJECTION"))//$VP(SETVAL,vp,PROJECTION,projectionid,bounds)
				{
					if (*Arg[5])
					{
						Bounds = atobounds (Arg[5],&Err);
						if (SetVPOutputProjection (atoi(Arg[4]),&Bounds))
							goto RtnTrue;
					}
					else // projectionid == 0 or no bounds clears the projection
					{
						if (SetVPOutputProjection (atoi(Arg[4]),0))
							goto RtnTrue;
					}
				}
				else if (!_fstricmp (Arg[3],"GRAY"))
				{ 
					if (Err)
						goto RtnFalse;
					if (atob (Arg[4]))
					{
						CurView->ConvertToGray = TRUE;    
			 			CurView->HalfToneNewObjectStart = CurView->NumNewObjects;
						DeleteCacheDir ();
						EscapeFunction (FALSE);
					}
					else
					{
						CurView->ConvertToGray = FALSE;
						DeleteCacheDir ();
						EscapeFunction (FALSE);
					}
					goto RtnTrue;
				}
				else if (!_fstricmp (Arg[3],"SIZEFACTOR"))
				{
					CurView->DisplayRectFactor = atof (Arg[3]);
					goto RtnTrue;
				}
				else if (!_fstricmp (Arg[3],"HAVELAYERCOLOR"))
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
				else if (i = OptionInList (Arg[3],ContourOpts[0],6,18))
				{
					for (layer=0;layer<CurView->NumFiles;layer++)
					{
						if (!_fstricmp (Arg[4],CurView->FileID[layer]))
						{ 
							int	w; 
							COLORREF	c;
							
							CurView->HaveLayerColor[layer] = TRUE;
							switch (i)
							{
								case 1:
									w = GetWValue (CurView->LayerColor[layer]);
									CurView->LayerColor[layer] = ColorWithWidth (atol(Arg[5]),w);
									break;  
								case 2:
									c = ColorWOWidth (CurView->LayerColor[layer]);
									CurView->LayerColor[layer] = ColorWithWidth (c,atoi(Arg[5]));
								    break;
								case 3:
									w = GetWValue (CurView->DarkContourColor[layer]);
									CurView->DarkContourColor[layer] = ColorWithWidth (atol(Arg[5]),w);
									break;  
								case 4:
									c = ColorWOWidth (CurView->DarkContourColor[layer]);
									CurView->DarkContourColor[layer] = ColorWithWidth (c,atoi(Arg[5]));
								    break;
								case 5:
									w = GetWValue (CurView->ContourTextColor[layer]);
									CurView->ContourTextColor[layer] = ColorWithWidth (atol(Arg[5]),w);
									break;  
								case 6:
									c = ColorWOWidth (CurView->ContourTextColor[layer]);
									CurView->ContourTextColor[layer] = ColorWithWidth (c,atoi(Arg[5]));
								    break;
								default:
									goto RtnFalse;
							}
							goto RtnTrue;
						}
					}
				}
				else if (!_fstricmp (Arg[3],"RBUT"))
				{
					_fstrncpy (CurView->RButFunction,Arg[4],128);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"DIALOGINIT"))
				{
					_fstrncpy (CurView->DlgInitCmd,Arg[4],256);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"HALFTONE"))
				{
					CurView->HalfTone = atoi (Arg[4]);
					DeleteCacheDir ();
					EscapeFunction (FALSE);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"HALFTONEALL"))
				{
					CurView->AlwaysUseHalfTone = atob (Arg[4]);
					DeleteCacheDir ();
					EscapeFunction (FALSE);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"BACKGROUNDCOLOR"))
				{
					CurView->BackGroundColor = atol (Arg[4]);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"ZOOMLOCK"))
				{
					CurView->ZoomLocked = atob (Arg[4]);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"MAXOFFSETDIST"))
				{
					CurView->MaxOffsetDist = atof (Arg[4]);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"WIDTH"))
				{
					CurView->Width = atof (Arg[4]);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"HEIGHT"))
				{
					CurView->Height = atof (Arg[4]);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"PWIDTH"))
				{
					GetClientRect (hWndMain,&Rect);
					CurView->Width = (100 * (atof (Arg[4])-1)) / RECTWIDTH(&Rect);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"PHEIGHT"))
				{
					GetClientRect (hWndMain,&Rect);
					CurView->Height = (100 * (atof (Arg[4])-1)) /RECTHEIGHT(&Rect);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"TAGPOINTX"))
				{
					CurView->TagPoint.x = atof (Arg[4]);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"TAGPOINTY"))
				{
					CurView->TagPoint.y = atof (Arg[4]);
					goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[3],"ROTATION"))
				{
					_fstrupr (Arg[4]);
					if (*LastChr (Arg[4]) == 'D')
					{
						*LastChr (Arg[4]) = 0;
						CurView->Rotation = atof (Arg[4]) * RADDEG; 
					}
					else
						CurView->Rotation = atof (Arg[4]); 
					SetVPRotation (CurView->Rotation);
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
				else if (!_fstricmp (Arg[3],"SCALEBAR"))
				{
					if (CurView->pTheme)
					{   
						if (CurView->pTheme->ID == GF_DISTANCE_THEME)
						{
							if (*Arg[4] == 'B') 
								CurView->pTheme->ClassType = 1;
							else
								CurView->pTheme->ClassType = 2;
							goto RtnTrue;
						}
					} 
				}
				else if (!_fstricmp (Arg[3],"PICKMACRO"))
				{
					_fstrcpy (CurView->PickMacroFile,Arg[4]);
					goto RtnTrue;
				}
				else if (!_fstricmp (Arg[3],"SVMACRO"))
				{
					if (CurView->pTheme)
					{
						_fstrcpy (CurView->pTheme->ShowValMacro,Arg[4]);
						goto RtnTrue;
					}
				}
				else if (!_fstricmp (Arg[3],"GAMACRO"))
				{
					if (CurView->pTheme)
					{
						_fstrcpy (CurView->pTheme->GraphicsAttributesMacro,Arg[4]);
						goto RtnTrue;
					}
				}
				else if (!_fstricmp (Arg[3],"METERINIT"))
				{
					_fstrcpy (CurView->MeterInit,Arg[4]);
					goto RtnTrue;
				}
				else if (!_fstricmp (Arg[3],"SELECTINIT"))
				{
					_fstrcpy (CurView->RButPickInit,Arg[4]);
					goto RtnTrue;
				}
				
				goto RtnFalse;
			} 
			else if (!_fstrcmp (Arg[1],"AREA"))
			{  
				if (Err)
					goto RtnFalse; 
				if (CurView->hMaskArea)
				{
					LPMNMXCORD	lpRect = (LPMNMXCORD) GlobalLock (CurView->hMaskArea);
					LPDPOINT	lpDpoints;
					
					lpRect++;
					lpDpoints = (LPDPOINT) lpRect;
					RVal = fabs (ComputeAreaAreaD (lpDpoints,CurView->NumMaskPoints,0));
					GlobalUnlock (CurView->hMaskArea);
				}
				else
					RVal = (CurView->WBounds.xmx - CurView->WBounds.xmn) * (CurView->WBounds.ymx - CurView->WBounds.ymn);
				ftoa (OutLoc,RVal); 
				goto Rtnl;
			}
			else if (!_fstrcmp (Arg[1],"DIMENSION"))
			{  
				if (Err)
					goto RtnFalse;
				if (!stricmp (Arg[3],"CLEAR"))
				{
					GSSiGlobFree (&CurView->hDistanceLine);
					goto RtnTrue;
				}
				goto RtnFalse;
			}
			else if (!_fstrcmp (Arg[1],"EXIST"))
			{  
				if (Err)
					goto RtnFalse;
				goto RtnTrue;
			}
			else if (!_fstrcmp (Arg[1],"GRAY"))
			{ 
				if (Err)
					goto RtnFalse;
			    CurView->ConvertToGray = TRUE;    
			 	CurView->HalfToneNewObjectStart = CurView->NumNewObjects;
			    DeleteCacheDir ();
				EscapeFunction (FALSE);
				goto RtnTrue;
			}
			else if (!_fstrcmp (Arg[1],"COLOR"))
			{ 
				if (Err)
					goto RtnFalse;
			    CurView->ConvertToGray = FALSE;
			 	CurView->HalfToneNewObjectStart = 0;
			    DeleteCacheDir ();
				EscapeFunction (FALSE);
				goto RtnTrue;
			}
			else if (!_fstrcmp (Arg[1],"SHOW"))
			{
				if (Err)
					goto RtnFalse;
				CurView->Active = TRUE; 
				CurView->Display = TRUE; 
				if (CurView->pTheme)
					CurView->pTheme->VPDisplayed = TRUE;
				if (CurView->TagPoint.x > 1000)
					CurView->TagPoint.x -= 1000;  
				if (CurView->ShrinkParent && CurView->Parent)   
				{
					SetupViewports (CurView->hWnd,CurView->hDC,0,MainRect,0);
					//RedisplayViewports (FALSE);  
				} 
				CurView->ProfileInCrossSection = FALSE;
				SetCurView ( SaveVP);
			}
			else if (!_fstrcmp (Arg[1],"ISACTIVE"))
			{
				if (Err)
					goto RtnFalse;
				rtn = CurViewActive ();
				SetCurView ( SaveVP);  
				if (rtn)
					goto RtnTrue;
				goto RtnFalse;
			}
			else if (!_fstrcmp (Arg[1],"ACTIVATE"))
			{
				if (Err)
					goto RtnFalse;
				CurView->Active = TRUE; 
				if (CurView->TagPoint.x < 1000)
					CurView->TagPoint.x += 1000;
				SetCurView ( SaveVP);
			}
			else if (!_fstrcmp (Arg[1],"TOGGLE"))
			{
				if (Err)
					goto RtnFalse;
				CurView->Active = !CurView->Active;
				SetCurView ( SaveVP);
			}
			else if (!Err && !_fstrcmp (Arg[1],"HIDE"))
			{
				if (Err)
					goto RtnFalse;
				CurView->Active = FALSE; 
				CurView->ProfileInCrossSection=FALSE;
			     if (CurView->pTheme)
			     	CurView->pTheme->VPDisplayed = FALSE;
				SetCurView ( SaveVP);
			}
			else if (!_fstrcmp (Arg[1],"DISPLAY"))
			{   
				BOOL SaveDisableHalt = DisableHalt;
				
//				DisableHalt = TRUE;
				if (Err)
					goto RtnFalse;
			    if (CurView->DisplayInParent && CurView->Parent)   
					DestroyVehicles (CurView->Parent);
				else
					DestroyVehicles (CurView->ID);
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
				
				if (Err)
					goto RtnFalse;
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
				if (Err)
					goto RtnFalse;
				CurView->TagPoint.x = atof (Arg[3]);
				CurView->TagPoint.y = atof (Arg[4]);
				CurView->Width = atof (Arg[5]);
				CurView->Height = atof (Arg[6]); 
				SetConfig (SaveCurrentConfig);
				SetCurView ( SaveVP);
			}   
			else if (!_fstrcmp (Arg[1],"SIZE"))
			{
				RECT	ParentRect;

				if (Err)
					goto RtnFalse;
				if (!CurView->Parent)
					GetClientRect (CurView->hWnd,&ParentRect);
				else
					ParentRect = pViewports[CurView->Parent - 1]->DrawRect;
				
				CurView->TagPoint.x = 100*atof (Arg[3])/RECTWIDTH(&ParentRect);
				CurView->TagPoint.y = 100*atof (Arg[4])/RECTHEIGHT(&ParentRect);
				CurView->Width		= 100*atof (Arg[5])/RECTWIDTH(&ParentRect);
				CurView->Height		= 100*atof (Arg[6])/RECTHEIGHT(&ParentRect); 
				SetConfig (SaveCurrentConfig);
				SetCurView ( SaveVP);
			}   
			else if (!_fstrcmp (Arg[1],"RESET"))
			{
				if (Err)
					goto RtnFalse;
			    DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
			    CloseTRANS2 (&CurView->hTranVPToBase);
			    CloseTRANS2 (&CurView->hTranBaseToVP);

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

		
			
