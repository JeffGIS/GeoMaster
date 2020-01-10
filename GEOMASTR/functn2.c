#include "graphint.h"     
#include "extrndb.h"
#include "umio.h"  
#include "dibapi.h"
#include "std.h"  

#include "gmextern.h"
#include "freeimage.h"

static	char	PolyProbListFile[MAX_PATH];

LPSTR GetBeginString(LPSTR pFile, LPSTR pAt, LPSTR beginStr, LPSTR containStr, LPSTR endStr)
{
	LPSTR pos = pAt;
	int lb = strlen(beginStr);
	int lc = strlen(containStr);
	int le = strlen(endStr);

	while (pos > pFile)
	{
		pos--;
		if (!strncmp(pos, containStr, lc))
			return 0;
		if (!strncmp(pos, endStr, le))
			return 0;
		if (!strncmp(pos, beginStr, lb))
			return pos;
	}
	return 0;
}

LPSTR GetEndString(LPSTR pAt, LPSTR beginStr, LPSTR containStr, LPSTR endStr)
{
	LPSTR pos = pAt;
	int lb = strlen(beginStr);
	int lc = strlen(containStr);
	int le = strlen(endStr);

	while (pos)
	{
		pos++;
		if (!strncmp(pos, containStr, lc))
			return 0;
		if (!strncmp(pos, beginStr, lb))
			return 0;
		if (!strncmp(pos, endStr, le))
			return pos;
	}
	return 0;
}


short GetFunArgs(LPSTR	Args, LPSTR *Arg, short MaxArgs, LPHANDLE phMem, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen)
#if ENABLETRACE
{GSSiEnterProg (1349);
#endif
{   
	short	nArgs=1; 
	int		bpOffInc;
	LPSTR	*pArg=Arg, LastArg, ParLoc, begArgs; 
	UINT	i;
	                 
	*phMem = GSSiGlobAlloc (1141,GMEM_MOVEABLE,abs(MaxArgs) * MAXARGLENGTH); 
	pArg++;
	LastArg = begArgs = *pArg = GlobalLock (*phMem);   
	pArg++;  
	_fstrcpy (LastArg,Args);
	if (*Args) 
	{
		for (i=1;i<abs(MaxArgs);i++,pArg++) 
		{
			int incPar = 0;
			*pArg = LastArg + MAXARGLENGTH;
			if ((ParLoc = MatchLev (LastArg,',')))
			{ 
				_fstrcpy (*pArg,(LPSTR)(ParLoc+1));
				*ParLoc = 0; 
				nArgs++;
				incPar = 1;
			} 
			else
				*(*pArg) = 0;
			bpOffInc = strlen(LastArg)+incPar;
			if (MaxArgs > 0)
				ExpandTextDB (LastArg,pBrkPt,bpOffset,bpLen);  
			bpOffset += bpOffInc;
			LastArg = *pArg;
		} 
		if (MaxArgs > 0)
			ExpandTextDB(LastArg, pBrkPt, bpOffset, bpLen);
	}  
	else 
	{
		nArgs = 0;
		for (i=1;i<abs(MaxArgs);i++,pArg++) 
		{
			*pArg = LastArg + MAXARGLENGTH*i;
			*(*pArg) = 0;
		}
	} 
	for (i=1;i<=nArgs;i++)
	{
		LPSTR	pEq = strchr (Arg[i],'=');  
		LPSTR	pAt=0;
		
		if (pEq)
		{
			*pEq = 0;
			if (pEq > Arg[i] && *(pEq-1) == literalChar)
			{
				pAt = pEq - 1;
				*pAt = 0;
			}
			if (IsInteger(Arg[i]))
			{
				int iarg=atoi (Arg[i]);

				if (iarg > 0 && iarg <= abs(MaxArgs))
				{
					strcpy (Arg[iarg],pEq+1);
					*Arg[i] = 0;
				}
				else
				{
					*pEq = '=';
					if (pAt)
						*pAt = literalChar;
				}
			}
			else
			{
				*pEq = '=';
				if (pAt)
					*pAt = literalChar;
			}
		}
	}
	for (i=1;i<=nArgs;i++)
	{
		short l = _fstrlen (Arg[i]);  
		
		if (l > 1 && *Arg[i] == '\'' && Arg[i][l-1] == '\'')
		{
			_fmemmove (Arg[i],&Arg[i][1],l-2);
			Arg[i][l-2] = 0;
		}
	}
{
#if ENABLETRACE
GSSiExitProg (1349);
#endif
	return nArgs;		
}
#if ENABLETRACE
}
#endif
}

int	GetFunctionValue3(int FunID, LPSTR Args, LPSTR OutLoc, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen)
#if ENABLETRACE
{GSSiEnterProg (1350);
#endif
{   HANDLE	hMem=0,hDLT,hSurf, hStr;
	LPSTR	Arg1, Arg2, Arg3, Arg4, Arg5, Arg6,Arg7, ParLoc, lpstr, lpstrb, str;  
	LPSTR	pEnd, pCR, pFile, Arg[20] = { 0 };
	HPSTR	pMem1,pMem2;
	HFILE	Fid1, Fid2, Fid3;   
	short	nArgs, l, CvtDir, year;    
	DPOINT	Point, Point2;
	POINT	Point16;
	RECT	Rect; 
	MNMXCORD	Bounds;           
	long	hNum, Refno,ii,systime, nlong,nl, CurLoc;
	BOOL	TORF, rtn, Err, PickVis;  
	LPVIEWPORT	SaveVP=CurView;
	short	SymNum;			
	short	i,n, pos, nrem,st, nRc;  
	double	AZ, Dist,Dist2,Elevation, RVal, Offset;
	HFILE	Fid;
	LPOFSTRUCTGM	pOFStruct;
	HPDPOINT	pPoint;
	HANDLE	hDibInfo;
	long	ImageOffset;
	UINT	Delay;
	LPHIGHLIGHTDATA	pHighlightData;   
	HWND	hWnd;
	SOCKET	socket;
	
	if (LinkToVar)
	{
		ExpandText (Args);
{
#if ENABLETRACE
GSSiExitProg (1350);
#endif
		return 0;
}
	}
	if (TraceOn)
	{
		hMem=GSSiGlobAlloc (1142,GMEM_MOVEABLE,4096);
		lpstr = GlobalLock (hMem);
		sprintf (lpstr,"GFV:%s(%s)",LastFunctionName,Args);
		GSSiTraceLev (lpstr,1,2);
		GSSiGlobUlFree (&hMem);
	}
	switch (FunID)
	{    
		case 801: /* $POINTSYM(StartSym,displayopt) set point symbol variables */
		{   
			short	iopt;
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			iopt = atoi (Arg[2]); 
			if (iopt == 3)
			{
				DLGPROC lpfnNEWPOINTSETMsgProc;
				
				lpfnNEWPOINTSETMsgProc = MakeProcInstance((DLGPROC)NEWPOINTSETMsgProc, hInst);
				rtn = DialogBox(hInst, (LPSTR)"NEWPOINTSET", GetFocus(), lpfnNEWPOINTSETMsgProc);
				FreeProcInstance(lpfnNEWPOINTSETMsgProc);
			
			}
			else
				rtn = SelectSymbol (CurView->hWnd,1,Arg[1],iopt); 
			if (rtn)
				goto RtnTrue;
			else
				goto RtnFalse;
		}
		
		case 802: // $COPYFILE(tofile,fromfile,A (append) R (replace) or anything else (default - create with prompt),TorF statuswindow
		case 839: // $FILECOPY(fromfile,tofile,A (append) R (replace) or anything else (default - create with prompt)
		{	double	rval;
			int		l, len; 
			LPSTR	lpOut; 
			char	fillchar;
			short	Append=-1;
			short	from=2,to=1;
			
			if (FunID == 839)
			{
				from=1;
				to=2;
			}
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			_fstrupr (Arg[3]);
			if (!_fstricmp ("A",Arg[3])) 
				Append = 1;
			else if (!_fstricmp ("R",Arg[3]))
				Append = 0;

			if (!Append)
			{
				if (atob (Arg[4]))
					rtn = CopyFileToCache (Arg[to],Arg[from]);
				else
					rtn = GSSiCopyFile (Arg[from],Arg[to],TRUE);
			}
			else
				rtn = copyfile(Arg[to],Arg[from],Append,0,0,0,0,0,0);  
			if (rtn)
				goto RtnTrue;
			else
            	goto RtnFalse;
		} 
		
		case 803: // $EDITFILE(file,TorF (use GMEdit))
		{	
			LPSTR	cmd;
			HANDLE	hCmd;
						
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			
			if (atob(Arg[2]))
			{
				LPSTR pcmd = malloc(MAX_PATH * 2);
				sprintf(pcmd, "$GMEDIT(%s)", Arg[1]);
				ExpandText(pcmd);
				free(pcmd);
			}
			else
			{
				hCmd = GSSiGlobAlloc(1145, GMEM_MOVEABLE, 1024);
				cmd = GlobalLock(hCmd);
				_fstrcpy(cmd, "[%TEXT_EDITOR]");
				ExpandText(cmd);
				_fstrcat(cmd, " ");
				_fstrcat(cmd, Arg[1]);
				WinExec(cmd, SW_SHOWMAXIMIZED);
				GSSiGlobUlFree(&hCmd);
			}
           	goto RtnTrue;
		}

		case 804: // $CHANGEPT(file,x,y,tol,F or T) changes nearest point in control file if in tolerance F=change
				  //								from point, T to point.
		{	double	rval;   
			BOOL	rtn=FALSE, AddSpace;
			int		l, len, minpt=-1, ipt; 
			LPSTR	lpOut, lpSpace; 
			char	fillchar; 
			HFILE	Fid, Fid2;  
			LPSTR	TmpName, str;  
			HANDLE	hTemp;
			double	fromX, fromY, toX, toY, newX, newY, dist, mindist; 

			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 5)
				goto RtnFalse; 
			newX = atof (Arg[2]);
			newY = atof (Arg[3]);
			mindist = atof (Arg[4]);
			if ((Fid = GSSiOpenFile (Arg[1],0,OF_READ))==HFILE_ERROR)
				goto RtnFalse;  
			hTemp = GSSiGlobAlloc (1147,GMEM_MOVEABLE,1024);
			TmpName = GlobalLock (hTemp);
			str = TmpName + 256;
			GSSiGetTempFileName (0,"gmt",0,(LPSTR)TmpName);
			Fid2 = GSSiOpenFile (TmpName,0,OF_CREATE); 
			ipt = 0; 
			lpSpace = Arg[5];
			lpSpace++;
			if (*lpSpace == 'R') // R for remove (FR or TR)
				AddSpace=TRUE;  // adding a space at begin of line comments it out
			else
				AddSpace=FALSE;
			while (fgetstring (str,128,Fid))
			{
				if (sscanf (str,"%lf %lf %lf %lf",&fromX,&fromY,&toX,&toY)==4)
				{   
					switch (*Arg[5])
					{
						case 'F':
						case 'f':
							if ((dist=LDIST (fromX,fromY,newX,newY)) < mindist)
							{ 
								minpt = ipt;
								mindist = dist;
							}  
							break;
						case 'T':
						case 't':
							if ((dist=LDIST (toX,toY,newX,newY)) < mindist)
							{ 
								minpt = ipt;
								mindist = dist;
							}  
							break; 
					}
				}
				fputstring (str,Fid2);
				ipt++;
			} 
			GSSiClose2 (&Fid);  
			GSSiClose2 (&Fid2);
			if (ipt >= 0)
			{
				Fid = GSSiOpenFile (Arg[1],0,OF_CREATE);
				Fid2 = GSSiOpenFile (TmpName,0,OF_READ); 
				ipt = 0;
				while (fgetstring (str,128,Fid2))   
				{   
					if (ipt == minpt)
					{  
						sscanf (str,"%lf %lf %lf %lf",&fromX,&fromY,&toX,&toY);
						if (AddSpace)					
							sprintf (str,"#%.14lg %.14lg %.14lg %.14lg",fromX,fromY,toX,toY); 
						else
						{
							switch (*Arg[5])
							{
								case 'F':
								case 'f':
									fromX = newX;
									fromY = newY; 
									break;
								case 'T':
								case 't': 
									toX = newX;
									toY = newY;
							} 
							sprintf (str,"%.14lg %.14lg %.14lg %.14lg",fromX,fromY,toX,toY);
						}
					}
					ipt++;
					fputstring (str,Fid);
				}
				GSSiClose2 (&Fid);  
				GSSiClose2 (&Fid2); 
				rtn=TRUE;
			}
			GSSiRemove (TmpName); 
			GlobalUnlock (hTemp);
			GSSiGlobUlFree (&hTemp);
			if (rtn)
				goto RtnTrue;
			else
            	goto RtnFalse;
			
		} 
		
		case 805: // $GOFILEPT(file,N or P or F or num) zooms to next, previous or specified point in file
		{	 
			BOOL	rtn=FALSE;
			HFILE	Fid;  
			LPSTR	str;  
			double	dist; 
			short	ipt;
			LPOFSTRUCTGM	pOFStruct;  
			HANDLE	hTemp;
			static	short	WantPt=0;
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			switch (*Arg[2])
			{
				case 'N':
				case 'n': 
					WantPt++;
				break;
				
				case 'P':
				case 'p':    
					WantPt--;
				break;  
				
				case 'F':
				case 'f':
					WantPt = 0;
					break;
				
				default:
					WantPt = atoi (Arg[2]);
			}
			if ((Fid = GSSiOpenFile (Arg[1],0,OF_READ))==HFILE_ERROR)
				goto RtnFalse;  
			ipt = 0; 
			hTemp = GSSiGlobAlloc (1149,GMEM_MOVEABLE,512);
			str = GlobalLock (hTemp);
			while (fgetstring (str,128,Fid))
			{   
				if (*str && *str != '#')
				{
					if (ipt == WantPt)
					{
						if (sscanf (str,"%lf %lf",&Point.x,&Point.y)==2) 
							rtn = TRUE;
						break;
					}
					ipt++; 
				}
			}
			GSSiClose2 (&Fid);	
			GlobalUnlock (hTemp);
			GSSiGlobUlFree (&hTemp);		
			if (rtn)
			{   
				dist = min (CurView->WBounds.xmx - CurView->WBounds.xmn, CurView->WBounds.ymx - CurView->WBounds.ymn);
				ZoomToPointAndDist (Point,dist/2,FALSE); 
				itoa (WantPt,OutLoc,10);
				goto Rtnl;
			}
			else
			{    
				WantPt = min (max (WantPt,-1),ipt);
				Sound (BAD_SOUND);
            	goto RtnFalse;
            }
			
		}  
		
		case 806: /* $COORDLOC(File,CurrentLocGlobal,D or S(sorted) or N or P) coordinate location file*/
		{	 
            DLGPROC lpfnCOORDLOCMsgProc;
		    HFILE       Fid; 
		    LPSTR       lpBAR, LastChar; 
			LPSTR	str;   
			HANDLE	hTemp;
			int		nRc;
			long	RecNum, WantRec; 
			LPOFSTRUCTGM	pOFStruct;
			
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			lpCLFile = Arg[1];
        	lpCLGlob = Arg[2];
            _fstrupr (Arg[3]);
            switch (*Arg[3])
            {
            	case 'N':
            	case 'P':

				    Fid=GSSiOpenFile (lpCLFile,0,OF_READ);  
	    			if (Fid==HFILE_ERROR)
	    				goto RtnFalse; 
	    			hTemp = GSSiGlobAlloc (1151,GMEM_MOVEABLE,1024);
	    			str = GlobalLock (hTemp);
					if (!GetVal (lpCLGlob, str))
						_fstrcpy (str,"-1");
					WantRec = atol (str); 
					if (*Arg[3] == 'N')
						WantRec++;
	    			else
	    				WantRec--;	
				    RecNum=0;
				    while (fgetstring (str,128,Fid))
				    {   
				    	if (RecNum == WantRec)
				    	{   
				    		GSSiClose2 (&Fid);
			         		SetGlobalValueLong (lpCLGlob,WantRec);
				    		if ((lpBAR = _fstrrchr (str,'|')))
				    		{   
				    			lpBAR++;
								if (sscanf (lpBAR,"%lf %lf",&CLocPoint.x,&CLocPoint.y) == 2)
								{	
									GSSiGlobUlFree (&hTemp);
					              	ZoomToPointAndDist (CLocPoint,LocationOffset,FALSE);
									ExecutePointLocationMacro (CLocPoint,0);
					              	goto RtnTrue;
					            }
					        }   
							GSSiGlobUlFree (&hTemp);
							goto RtnFalse;
						}
				    	RecNum++;
				    }  
				    GSSiClose2 (&Fid);
					GSSiGlobUlFree (&hTemp);
					goto RtnFalse;
            	
            	case 'D':
            	case 'S':
		            lpfnCOORDLOCMsgProc = MakeProcInstance((DLGPROC)COORDLOCMsgProc, hInst); 
		            if (*Arg[3] == 'D')
		            	nRc = DialogBox(hInst, (LPSTR)"COORDLOC", hWndMain, lpfnCOORDLOCMsgProc); 
		            else
		            	nRc = DialogBox(hInst, (LPSTR)"COORDLOC_SORT", hWndMain, lpfnCOORDLOCMsgProc);
		            FreeProcInstance(lpfnCOORDLOCMsgProc); 
		             
		            if (nRc)
		            {
		              	ZoomToPointAndDist (CLocPoint,LocationOffset,FALSE);
						ExecutePointLocationMacro (CLocPoint,0);
		              	goto RtnTrue;
		            }   
		            else
						goto RtnFalse;
			}
		}
		
		case 807: // $IMAGELIM(Indexfile,IMAGE) returns limits of original image file in indexed ortho
		{	 
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (GetOriginalImageBounds(Arg[1], Arg[2], &Bounds))
			{
				UserBounds[0].x = Bounds.xmn;
				UserBounds[0].y = Bounds.ymn;
				UserBounds[1].x = Bounds.xmx;
				UserBounds[1].y = Bounds.ymx;
				goto RtnTrue; 
			}
			else
            	goto RtnFalse;
			
		}  
		break;
		
		case 808: 
/*			if (WMASetUp())
				goto RtnTrue;
			else*/
				goto RtnFalse;
		break;
				
		case 809: // $LOADTRAN(Transformation file,Type (1=default,2=onescale,3=fixedpoint),which(P for projection trans)V( default loads Viewport transformation file)R (loads reverse viewport trans)
		{
			short	Type;
			
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;  
			if (*Arg[4])
				SetCurView (SetVPFromName (Arg[4],&Err));	
			Type = atoi (Arg[2]);
			if (Type < 1 || Type > 3)
				Type = 1;
			if (*Arg[3] == 'P')
			{
				CloseTRANS2 (&hTranProjection);
				CloseTRANS2 (&hTranProjectionReverse);
				if ((hTranProjection=LoadTranFile(Arg[1],1,Type,0,0)))
				{
					hTranProjectionReverse=LoadTranFile(Arg[1],2,Type,0,0); 
					CurView = SaveVP;
					goto RtnTrue;
				}
				else 
				{
					CurView = SaveVP;
					goto RtnFalse;
				}
			}
			else
			{   
				short	from=1,to=2;
				
				if (*Arg[3] == 'R')
				{
					from = 2;
					to = 1;
				}
				if (CurView->LinkedTo)
				{   
					short	SaveVPID = CurView->ID;
					
					SetViewport (CurView->LinkedTo);
					CloseTRANS2 (&CurView->hFileTransIn);
					CloseTRANS2 (&CurView->hFileTransOut);
					SetViewport (SaveVPID);
				}
				CloseTRANS2 (&CurView->hFileTransIn);
				CloseTRANS2 (&CurView->hFileTransOut);
				if ((CurView->hFileTransIn=LoadTranFile(Arg[1],from,Type,0,0)))
				{
					CurView->hFileTransOut=LoadTranFile(Arg[1],to,Type,0,0);
					CurView = SaveVP;
					goto RtnTrue;
				}
				CurView->hFileTransIn=CurView->hFileTransOut=(HANDLE)1;
				CurView = SaveVP;
				goto RtnFalse;
			}
		} 

		case 810: //$GETCOLOR(variablename)
		{
			COLORREF	Color;
			HANDLE hMem;
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (!*Arg[1])
				goto RtnFalse;
			hMem = GSSiGlobAlloc(1142, GMEM_MOVEABLE, 4096);
			lpstr = GlobalLock(hMem);
			sprintf(lpstr, "[%s]", Arg[1]);
			Color = GetGlobalLVal (lpstr);
			GSSiGlobUlFree(&hMem);
			if (GetColor(CurView->hWnd,&Color)) 
			{
				SetGlobalValueLong(Arg[1],Color);
				goto RtnTrue; 
			}  
			else
				goto RtnFalse;   
		}
		break;
				
		case 811: //$ADDPOINT(x y,display(YORN-opt))
		{
			COLORREF	Color;
			 
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (sscanf (Arg[1],"%lf %lf",&Point.x,&Point.y) != 2)
				goto RtnFalse;

			if (AddNewPoint(Point,atob(Arg[2]),Arg[3])>0) 
				goto RtnTrue; 
			else
				goto RtnFalse;   
		}
		break;
				
		case 812:// $LOADEDGE (joinlinefile,allcornerfile,origtpfile,filecornerfile,filelist.txt,mapfilename,outfilename) 
		{
			nArgs = GetFunArgs(Args, Arg, 7, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 7)
				goto RtnFalse;
			if (LoadEdge (Arg[1],Arg[2],Arg[3],Arg[4],Arg[5],Arg[6],Arg[7]))
		    	goto RtnTrue; 
		    goto RtnFalse;
		}
		
		case 813: //$AREAGRID(pathname,symbol,Prefix,width,height,overlap,XAdjust,YAdjust,displaystatuswnd)
				  //$AREAGRID(pathname,symbol,GridPrefix,0,0,bounds,displaystatuswnd) creates area grid from griddef
		{
			short	DistUnits; 
			double Width, Height;
			
			nArgs = GetFunArgs(Args, Arg, 9, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 3)
				goto RtnFalse;
			Width = atof(Arg[4]);
			Height = atof(Arg[5]);
/*			if (PRJ_UNITS[1] == 4)
            { */
			if (!Width)
			{
				MNMXCORD	Bounds=atobounds (Arg[6],&Err);

				if (Err)
					rtn = 0;
				else
					rtn = CreateAreaGridFromGrid (Arg[1],Arg[2],Arg[3],atoi(Arg[5]),&Bounds,atob(Arg[7]));
			}
			else
				rtn = CreateAreaGridLatLon (Arg[1],Arg[2],Arg[3],Width,Height,atof(Arg[6]),atof(Arg[7]),atof(Arg[8]),atob(Arg[9]));
/*            }
            else
            {
				GetDistAndUnits (Arg[3],&Dist,&DistUnits,TRUE);  
				Dist = ConvertInDist (Dist,DistUnits+1); 
				if (CreateAreaGrid (Arg[1],Arg[2],Dist))
					goto RtnTrue;
			}*/
			goto Rtnrtn;
		}
		 
		case 814: //$TCPCLOSE(socket)
		{				
			hMem = GSSiGlobAlloc (1155,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			*OutLoc = 0;
			ExpandText (Arg1);        
			socket = atol (Arg1);
			if (CloseTCPIPSocket (socket,TRUE))
				goto RtnTrue;
			goto RtnFalse;
		}

		case 815: // $PARSECDT((VAR1,VAR2),(val1,val2)) parses variables from comma delimted text (or other delimeter if is first char
		{
			char	delim=',';

			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (*Arg[1] != '(')
				delim = *Arg[1]++;
			if (*Arg[1] == '(' && *LastChr(Arg[1]) == ')')
			{
				Arg[1]++;
				*LastChr(Arg[1]) = 0;
			}
			if (!ProcessDelimTextHeader(Arg[1], 0, HFILE_ERROR, &hDLT, delim, 0))
				goto RtnFalse;
			if (*Arg[2] == '(' && *LastChr(Arg[2]) == ')')
			{
				Arg[2]++;
				*LastChr(Arg[2]) = 0;
			}
		    rtn = GetDelimTextData(Arg[2],hDLT,4090); 
			GSSiGlobFree (&hDLT);
			if (rtn)
				goto RtnTrue;
			else
            	goto RtnFalse;
			
		}  
		break;
		
		case 816: // $NUMLINES(pathname) returns number of lines in text file
		{	 
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			nlong = 0;
			Fid = GSSiOpenFile (Arg[1],0,OF_READ); 
			if (Fid != HFILE_ERROR)
			{
	        	while (fgetstring (Arg[1],2040,Fid))
		   			nlong++; 
		   		GSSiClose2 (&Fid);
		   	}
			ltoa (nlong,OutLoc,10);  
		    goto Rtnl;
        }

		case 817: // $SCALERNG(maxscale,minscale) returns 0 if current scale in range else %NIR% (not in range)
		{	 
			double	MinScale, MaxScale;
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			MaxScale = atof(Arg[1]);
			MinScale = atof (Arg[2]);
			
			if (OrthScale < MaxScale && OrthScale >= MinScale)
				*OutLoc = 0;
			else
				_fstrcpy (OutLoc,"%NIR%");
			goto Rtnl;
			
		}  
		break;
		
		case 818: // $ZOOMLIST(SELECT or FIRST or NEXT or PRIOR)
				  // $ZOOMLIST(ADD,POINTorBOUNDS,Name,pointorbounds)
				  // $ZOOMLIST(CREATE,pathname);
		{	 
			
			nArgs = GetFunArgs(Args, Arg, 8, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs == 1)
			{
				if (!_fstricmp(Arg[1], "NEXT"))
					n = 1;
				else if (!_fstricmp(Arg[1], "PRIOR"))
					n = -1;
				else if (!_fstricmp(Arg[1], "FIRST"))
					n = 0;
				else if (!_fstricmp(Arg[1], "REMOVE"))
					n = -2;
				else
					goto RtnFalse;
				if (DisplayZoomList(n))
					goto RtnTrue;
			}
			else
			{
				MNMXCORD bounds;

				if (!_fstricmp(Arg[1], "ADD"))
				{
					if (!_fstricmp(Arg[2], "POINT"))
					{
						DPOINT pt = atopt(Arg[4], &Err);
						if (!Err)
						{
							bounds.xmn = pt.x;
							bounds.xmx = pt.x;
							bounds.ymn = pt.y;
							bounds.ymx = pt.y;
							SaveZoomToCurrentList(&bounds, Arg[3]);
							goto RtnTrue;
						}
					}
					else if (!_fstricmp(Arg[2], "BOUNDS"))
					{
						bounds = atobounds(Arg[4], &Err);
						if (!Err)
						{
							SaveZoomToCurrentList(&bounds, Arg[3]);
							goto RtnTrue;
						}
					}
				}
				else if (!_fstricmp(Arg[1], "CREATE"))
				{
					rtn = CreateNewZoomList(hWndMain, Arg[2]);
					goto Rtnrtn;
				}
			}
			goto RtnFalse;
			
		}  
		break;
		
		case 819: // $MIDPOINT(coord pair)
				  // $MIDPOINT(ITEM,TagOrRef)
		{	
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (!stricmp (Arg[1],"ITEM"))
			{
				if (GetItemMidpoint (Arg[2],&Point))
					dpointtoa (OutLoc,&Point);
			}
			else
			{
				Bounds = atobounds (Arg[1],&Err);
				if (Err)   
					*OutLoc = 0; 
				else
				{
					Point = MinMaxMidPointD (&Bounds);
					dpointtoa (OutLoc,&Point);
				}
			}
			goto Rtnl;
		}  
		break;
		
		case 820://$NEWPOINT(STARTPOINT,AZ,DIST,OFFSET) 
		{
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			Point2 = atopt (Arg[1],&Err); 
			AZ = atof (Arg[2]);
			Dist = atof (Arg[3]);
			Offset = atof (Arg[4]); 
			Point = dnewpt (Point2,AZ,Dist);
			if (Offset)
			{
				Point2 = Point;
				Point = dnewpt (Point2,LTWOPI(AZ+HALFPI),Offset);
			}
			sprintf (OutLoc,"%.14lg %.14lg",Point.x,Point.y);
			goto Rtnl;
		}
		case 821: // $PATHTYPE(Pathname)
		{	 
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			n = FileType(Arg[1]);
			itoa (n,OutLoc,10);
			goto Rtnl;
			
		}  
		case 822: // $GMDMERGE(DBTO,DBFROM)
		{	 
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (*Arg[1] == '(' && *LastChr(Arg[1]) == ')')
			{
				Arg[1]++;
				*LastChr(Arg[1]) = 0;
			}
			if (GWD_MergeDBs (Arg[1],Arg[2],TRUE))
				goto RtnTrue;
			goto RtnFalse;
		}
		case 823: // $TRUNCATE(arg,char(opt))
		{	 
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs == 2)
				Truncate2(Arg[1],*Arg[2]);
			else
				Truncate(Arg[1]);
			_fstrcpy(OutLoc, Arg[1]);
			goto Rtnl;
		}  
		break;
		
		case 824: // $FILETYPE(arg)
		{	 
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs > 1)
			{
				HFILE Fid = GSSiOpenFile (Arg[1],0,OF_READ);

				if (Fid != HFILE_ERROR)
					GSSiClose2 (&Fid);
				nlong = FileType_old (Arg[1]);
			}
			else
				nlong = FileType (Arg[1]);
			ltoa (nlong,OutLoc,10);
			goto Rtnl;
		}  
		break; 
		
		case 825: //$REGISTER
            {
            	UINT	st=IDOK;
            GetSno:	  
			   	if (!GetTextString (GetFocus(),SerialNumber,32,"Enter the LMCODE located on the cover of the CD case.",0,0,0,TRUE,TRUE))
			   		goto RtnFalse;
			   	while (st == IDOK)
			   	{   
			   		short err = CheckForRegistrationServer (hWndMain,SerialNumber);
             DoReg: 
             		AutoReg = FALSE;
			   		switch (err)
			   		{
			   			case 0: 
		            	{
		                  DLGPROC lpfnAUTOREGMsgProc; 
		                  short	nRc;
		
		                  lpfnAUTOREGMsgProc = MakeProcInstance((DLGPROC)AUTOREGMsgProc, hInst);
		                  nRc = DialogBox(hInst, (LPSTR)"AUTOREG", hWndMain, lpfnAUTOREGMsgProc);
		                  FreeProcInstance(lpfnAUTOREGMsgProc);  
		                  if (nRc)
						  { 
						  	GetTextString (GetFocus(),AccessCode,64,"Use this access code",0,0,0,TRUE,TRUE);  
			                goto RtnTrue;
						  }
		                  
		            	}
						st = IDCANCEL;
		            	break;
		            	
		            	case 1:
		            		st = GSSiMessageBox (0,"Unable to connect to registration server",0,MB_OKCANCEL,0);
		            		break;
		            	
		            	case 2:
		            		st = IDCANCEL;
		            		break;
		            	case 3:
		            		if (GSSiMessageBox (0,"This number already in use - use anyway?",0,MB_YESNO,0) == IDYES)
		            		{
		            			err = 0;
		            			goto DoReg;
		            		}
		            		st = IDCANCEL;
		            		break;
		            	case 4:
		            		GSSiMessageBox (0,"Invalid Serial Number",0,MB_ICONEXCLAMATION,0);
		            		goto GetSno;
		            		break;
	            	}
	            }
	            goto RtnFalse;
            }		
            break;
            
		
		case 826: // $CONTINUE() allows termination with esc
		{	 
			if (CheckForContinue(TRUE, 0))
				goto RtnTrue;
			goto RtnFalse;
		}  
		
		case 827:	//$READLINE(file,lineno(1 based)opt-def1)
		{   
			long	LineNo=0; 
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (nArgs)
			{
				Fid1 = GSSiOpenFile (Arg[1],0,OF_READ);  
				nlong = max(1,atol (Arg[2])); 
				
				if (Fid1 != HFILE_ERROR)
				{   
					while (LineNo < nlong && fgetstring (OutLoc,1024,Fid1))
						LineNo++; 
					GSSiClose2 (&Fid1);
				} 
			}
			goto Rtnl;
		}
		
		case 828:	//$TABLEDEF(type,inspec)
		{   
			long	LineNo=0; 
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (nArgs == 2)
				GMDHeaderToTableDef (Arg[2],OutLoc);
			
			goto Rtnl;
		}
		
		case 829: // $OWNERLOC() -kaufman specific
		{	 
            DLGPROC lpfnOWNERLOCMsgProc;
			int		nRc;
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (!nArgs)
			{
				lpfnOWNERLOCMsgProc = MakeProcInstance((DLGPROC)OWNERLOCMsgProc, hInst);
				nRc = DialogBox(hInst, (LPSTR)"OWNERSEARCH", CurView->hWnd, lpfnOWNERLOCMsgProc);
			}
			else if (!stricmp (Arg[1],"MGVPOLICE"))
			{
				lpfnOWNERLOCMsgProc = MakeProcInstance((DLGPROC)OWNERLOCMsgProc3, hInst);
				nRc = DialogBox(hInst, (LPSTR)"OWNERSEARCH3", CurView->hWnd, lpfnOWNERLOCMsgProc);
			}
			else if (!stricmp (Arg[1],"MPLSPOLICE"))
			{
				lpfnOWNERLOCMsgProc = MakeProcInstance((DLGPROC)OWNERLOCMsgProc4, hInst);
				nRc = DialogBox(hInst, (LPSTR)"OWNERSEARCH4", CurView->hWnd, lpfnOWNERLOCMsgProc);
			}
			else
			{
				lpfnOWNERLOCMsgProc = MakeProcInstance((DLGPROC)OWNERLOCMsgProc2, hInst);
				nRc = DialogBox(hInst, (LPSTR)"OWNERSEARCH2", CurView->hWnd, lpfnOWNERLOCMsgProc);
			}
            FreeProcInstance(lpfnOWNERLOCMsgProc); 
            if (nRc==1)
            {
              	PostMessage(hWndMain, WM_COMMAND, 58000, 0L);        
              	goto RtnTrue;
            }   
            else
				goto RtnFalse;
		}
		
		case 830: //$REORGMAP(TFbackup,TFbuildtagref,tranfile,TFFixedtran)
        {
          DLGPROC	lpfnREORGMAPMsgProc;  
          LPSTR		pReorgParms;
          
		  nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
          hReorgParms = GSSiGlobAlloc (1166,GMEM_MOVEABLE,1024);
          pReorgParms = GlobalLock (hReorgParms);  
          sprintf (pReorgParms,"%s\t%s\t%s\t%s",Arg[1],Arg[2],Arg[3],Arg[4]);
          GlobalUnlock (hReorgParms);
          lpfnREORGMAPMsgProc = MakeProcInstance((DLGPROC)REORGMAPMsgProc, hInst);
          nRc = DialogBox(hInst, (LPSTR)"REORGMAP", CurView->hWnd, lpfnREORGMAPMsgProc);
          FreeProcInstance(lpfnREORGMAPMsgProc);  
		  GSSiGlobFree(&hReorgParms);
		  if (nRc == 2)
     	  	SendMessage(CurView->hWnd, WM_COMMAND, IDM_BUILDREFINDEXES, 0L); 
     	  goto RtnTrue;
        }

		case 831: //$GETFOCUS()
        {
          
		  hWnd = GetFocus ();
		  ltoa ((long)hWnd,OutLoc,10);
		  goto Rtnl;
		}
		  
		case 832: //$SETFOCUS(hwnd)
        {
          HWND	hWnd;
          
		  nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		  if (!nArgs)
		  	goto RtnFalse; 
		  if (!_fstricmp (Arg[1],"MAIN"))
		  	hWnd = hWndMain;
		  else
		  	hWnd = (HWND)atol(Arg[1]);
		  if (SetFocus (hWnd))
		  	goto RtnTrue;
		  goto RtnFalse;
		} 
		
		case 833: //$TRAVERSE(MAKETEXT
        {
          HWND	hWnd;
          
		  nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		  if (!nArgs)
		  	goto RtnFalse; 
		  if (!_fstricmp (Arg[1],"MAKETEXT"))
		  {
		  	if (TravCreateDimText ())
		  		goto RtnTrue;
		  } 
		  if (!_fstricmp (Arg[1],"EXIT"))
		  {
				if (hWndTraverseEntry)
				{
					SendMessage(hWndTraverseEntry, WM_CLOSE, 0, 0L); 
					goto RtnTrue;
			    }
		  }
		  goto RtnFalse;
		}
		
		case 834: //$CHECKTIF(name)
        {
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
		  if (!nArgs)
		  	goto RtnFalse; 
		  if (BMPFromTIF (Arg[1],TRUE))
			goto RtnTrue;
		  goto RtnFalse;
		}
		
		case 835: //$GMDREORG(Name,IndexToReorgOn(0=primary=default),Compress,Verify,AddFieldDef(ex. (NEWF1(B4)=10,NEWF2(C1)=T)
        {
			nArgs = GetFunArgs(Args, Arg, -5, &hMem, pBrkPt, bpOffset, bpLen);
		  if (!nArgs)
		  	goto RtnFalse;  
		  ExpandText (Arg[1]);
		  ExpandText (Arg[2]);
		  ExpandText (Arg[3]);
		  ExpandText (Arg[4]);
		  if (GMDReorg (Arg[1],atoi(Arg[2]),atob(Arg[3]),atob(Arg[4]),0,Arg[5]))
			goto RtnTrue;
		  goto RtnFalse;
		}
		
		case 836: //$MAPINDEX(Create,indexname,filelist)
		{
			DLGPROC lpfnLOADMDMsgProc;
			                  
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 3)
				goto RtnFalse; 
			_fstrcpy (AutoExportName,Arg[3]);
			_fstrcpy (AutoMapIndexName,Arg[2]);
			lpfnLOADMDMsgProc = MakeProcInstance((DLGPROC)LOADMDMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"LOAD_MD", hWndMain, lpfnLOADMDMsgProc);
			FreeProcInstance(lpfnLOADMDMsgProc);  
			*AutoMapIndexName=0;
			*AutoExportName=0;
			if (nRc)
				goto RtnTrue;
			goto RtnFalse;
		}
		
		case 837: //$TIMESPAN(S,M,H,D)
		{
			                  
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse; 
			nlong = GetSecondsInSample ();
			switch (*Arg[1])
			{
				case 'S': 
				case 's':
				break;   
				case 'M':
				case 'm':
				nlong = IDNINT (((double)nlong) / (60L));
				break;
				case 'H':
				case 'h':
				nlong = IDNINT (((double)nlong) / (3600L));
				break;
				case 'D':
				case 'd':
				nlong = IDNINT (((double)nlong) / (24L*3600L));
				break;
			}
			ltoa (nlong,OutLoc,10);
			goto Rtnl; 
		}
		
		case 838: //$COPYTEXT(tofile,fromfile,FIRSTorLAST,nlines)
		{
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse; 
			if (CopyTextFile (Arg[1],Arg[2],Arg[3],Arg[4]))
				goto RtnTrue;
			goto RtnFalse;
		}

		case 840: // $COLORMAP(CREATE,pathname,vpname)
				  //		  (ADD,pathname,color)
		{
			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse; 
			if (!_fstricmp (Arg[1],"CREATE"))
			{   
				SetCurView ( SetVPFromName (Arg[3],&Err));
				CreateColorMap (Arg[2]);
				goto RtnTrue; 
			}
			else if (!_fstricmp (Arg[1],"ADD"))
			{ 
				AddColorToColorMapFile (Arg[2],atol(Arg[3]));
				goto RtnTrue;
			}
			else if (!_fstricmp (Arg[1],"LOAD"))
			{ 
				LoadColorChanges();
				goto RtnTrue;
			}
			else if (!_fstricmp (Arg[1],"ADJUST"))
			{ 
				if (ChangeImageColors (Arg[2],FALSE));
					goto RtnTrue;
			}
			else if (!_fstricmp (Arg[1],"GET"))
			{ 
				int	x,y,tot=0,black=0;

				SetCurView ( SetVPFromName (Arg[2],&Err));
				for (y=CurView->ScreenRect.top;y<CurView->ScreenRect.bottom;y++)
					for (x=CurView->ScreenRect.left;x<CurView->ScreenRect.right;x++)
					{
						tot++;
						if (!GetPixel (CurView->hDC,x,y))
							black++;
					}
				sprintf (Arg[3],"Percent black = %f",((double)(100*black))/tot);
				MessageBox (0,Arg[3],"",MB_OK);
				goto RtnTrue;
			}
			else
				goto RtnFalse;
		}  
		
		case 841: // $MAPIMAGE(GET,MIpathname,MIID,OutFile)
		{
			nArgs = GetFunArgs(Args, Arg, 8, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse; 
			if (!stricmp (Arg[1],"GET"))
			{
				if (MapImageToFile (Arg[2],atol(Arg[3]),Arg[4],0,0))
					goto RtnTrue; 
			}
			else if (!stricmp (Arg[1],"CONVERT"))
			{
				SetViewport(*pCommandViewport);
				if (ConvertHBirdImages (Arg[2],Arg[3],Arg[4],Arg[5],atob(Arg[6])))
					goto RtnTrue; 
			}
			else if (!stricmp (Arg[1],"EXPORT"))
			{
				MNMXCORL FileBounds;

				if (MapImageExport (Arg[2],Arg[3],Arg[4],atol(Arg[5]),&FileBounds,atob(Arg[6])))
					goto RtnTrue; 
			}
			else if (!stricmp (Arg[1],"EXTRACT"))
			{
				MNMXCORL FileBounds;

				if (MapExtractExport (Arg[2],Arg[3],Arg[4],atol(Arg[5])))
					goto RtnTrue; 
			}
			else if (!stricmp (Arg[1],"GETFILEBOUNDS"))
			{
				int	Level=40960*2;
				MNMXCORL FileBounds;
				HANDLE	hFiles=GSSiGlobAlloc (0,GMEM_MOVEABLE,1024);
				LPSTR	pFile=GlobalLock (hFiles), pOutFile=pFile+512;
				char	Subfile='A';
				HFILE	FidOut;

				while (Level > 5)
				{

					Subfile='A';
					sprintf (pOutFile,"%s\\%s%iX.bin",Arg[2],Arg[3],Level);
					FidOut = GSSiOpenFile (pOutFile,0,OF_CREATE);
					while (Subfile < 'Q')
					{
						sprintf (pFile,"%s\\%s%i%c.bin",Arg[2],Arg[3],Level,Subfile);
						FileBounds = GetImageFileBounds (pFile,Level*10);
						BigWrite (FidOut,&FileBounds,sizeof(FileBounds),-1);
						Subfile++;
					}
					GSSiClose2 (&FidOut);
					Level /= 2;
				}
				Subfile='A';
				sprintf (pOutFile,"%s\\%sctextX.bin",Arg[2],Arg[3]);
				Level = GetGlobalLVal2 ("[CONTOURTEXTSCALE]",400);
				FidOut = GSSiOpenFile (pOutFile,0,OF_CREATE);
				while (Subfile < 'Q')
				{
					sprintf (pFile,"%s\\%sctext%c.bin",Arg[2],Arg[3],Subfile);
					FileBounds = GetImageFileBounds (pFile,Level);
					BigWrite (FidOut,&FileBounds,sizeof(FileBounds),-1);
					Subfile++;
				}
				GSSiClose2 (&FidOut);
				GSSiGlobUlFree (&hFiles);
				goto RtnTrue;
			}
			else if (!stricmp (Arg[1],"EXPORTTEXT"))
			{
				if (MapTextExport (Arg[2],Arg[3],Arg[4]))
					goto RtnTrue; 
			}
			else if (!stricmp (Arg[1],"EXPORTCTEXT"))
			{
				if (ContourTextExport (Arg[2],Arg[3],Arg[4],atol(Arg[5])))
					goto RtnTrue; 
			}
			else if (!stricmp (Arg[1],"EXPORTSYMLIST"))
			{
				if (CreateSymlistFile (Arg[2]))
					goto RtnTrue;
			}
			else if (!stricmp (Arg[1],"EXPORTSYMLIST2"))
			{
				if (CreateSymlistFile2 (Arg[2]))
					goto RtnTrue;
			}
			else if (!stricmp (Arg[1],"EXPORTSETUP"))
			{
				typedef struct {int		UTMZone;
								int		MaxDepthLayer;
								char	MapPrefix_SufFile_dek[4];
								char	CardName[12];
								} SETUPDATA;
				SETUPDATA	setupdata;
				BYTE	version=1;

				Fid = GSSiOpenFile (Arg[2],0,OF_CREATE);
				if (Fid == HFILE_ERROR)
					goto RtnFalse;
				strncpy0 (setupdata.CardName,Arg[3],10);
				setupdata.UTMZone = atoi (Arg[4]);
				setupdata.MaxDepthLayer = atoi (Arg[5]);
				strncpy (setupdata.MapPrefix_SufFile_dek,Arg[6],2);
				setupdata.MapPrefix_SufFile_dek[2] = *Arg[7];
				setupdata.MapPrefix_SufFile_dek[3] = atoi(Arg[8]);
				BigWrite (Fid,&setupdata,sizeof(SETUPDATA),-1);
				BigWrite (Fid,&version,1,-1);
				GSSiClose2 (&Fid);
				goto RtnTrue;
			}
			else if (!stricmp (Arg[1],"EXPORTGRIDDEF"))
			{
				if (SaveLayerDefFile (Arg[2],!atob(Arg[3])))
					goto RtnTrue;
			}
			else if (!stricmp (Arg[1],"EXPORTPOINTS"))
			{
				if (MapImagePointExport (Arg[2],Arg[3],Arg[4],atol(Arg[5])))
					goto RtnTrue; 
			}
			else if (!stricmp (Arg[1],"EXPORTNAMES"))
			{
				if (MapImageCityLakeNameExport (Arg[2],Arg[3],Arg[4],atol(Arg[5])))
					goto RtnTrue; 
			}
			else if (!stricmp (Arg[1],"STOREELEVTEXT"))
			{
				int	cno=0;
				LPSTR	pchr =Arg[3];
				int	seqno = atoi (Arg[4]);

				Point16 = atopt16 (Arg[2],&Err);
				while (*pchr)
					rtn = DisplayCharAtPoint (0,Point16,0,atoi(Arg[3]),*pchr++,cno++,seqno++,0);
				goto Rtnrtn; 
			}
			goto RtnFalse;
		}  
		
		case 842: // $XMLTOGMD(type,xmlfile,gmdfile)
		{
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 3)
				goto RtnFalse; 
			if (!stricmp (Arg[1],"MNDOTSTATIONS"))
			{
				if (XMLToGMD1 (Arg[2],Arg[3]))
					goto RtnTrue; 
			}
			if (!stricmp (Arg[1],"MNDOTCLOSURES"))
			{
				if (XMLToGMD2 (Arg[2],Arg[3]))
					goto RtnTrue; 
			}
			goto RtnFalse;
		}  
		
		case 843: // $PNETGRID
		{
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			
			if (!stricmp (Arg[1],"CREATE"))
				CreatePNetStateGrid (atoi(Arg[2]));
			else if (!stricmp (Arg[1],"LOAD"))
				LoadPNetStateGrid (Arg[2],Arg[3],Arg[4]);
			else if (!stricmp (Arg[1],"INIT"))
			{
				LPBYTE	pDataInit;
				int ld = GSSiLength (Arg[2]);
				HFILE	Fid;
				
				if (ld > 0)

				GSSiGlobUlFree (&hPNGrid);
				hPNGrid = GSSiGlobAlloc (0,GMEM_MOVEABLE,ld);
				pDataInit = GlobalLock (hPNGrid);
				Fid = GSSiOpenFile (Arg[2],0,OF_READ);
				BigRead (Fid,pDataInit,ld);
				GSSiClose2 (&Fid);
				GetStateFromLatLonInit (pDataInit);
			}
			else if (!stricmp (Arg[1],"DISPLAY"))
			{
				SetCurView ( SetVPFromName (Arg[3],&Err)); 
				DisplayPNStateGrid (Arg[2]);
			}
			else if (!stricmp (Arg[1],"TEST"))
			{
				Point = atopt (Arg[2],&Err); 
				rtn = GetPNAreaID (Point,Arg[3]);
				itoa (rtn,OutLoc,10);
				goto Rtnl;
			}
			else if (!stricmp (Arg[1],"GET"))
			{
				int	StateOrProvinceArray[4], TZArray[4];

				Point = atopt (Arg[2],&Err); 
				rtn = GetStateFromLatLon ((int)(Point.y*10000000),(int)(Point.x*10000000),StateOrProvinceArray,TZArray);
				if (rtn > 1)
					StateOrProvinceArray[0] = -StateOrProvinceArray[0];
				sprintf (OutLoc,"%i:%i",StateOrProvinceArray[0],TZArray[0]);
				goto Rtnl;
			}
			goto RtnTrue;
		}  
		
		case 844: // $CPTTOBPW(ImageFile)
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse; 
			if (CreateBPWFromCPT (Arg[1]))
				goto RtnTrue;
			goto RtnFalse;
		} 
		
		case 845: // $FILELIST OutFile, New, SearchLoc, WildCard,SearchSubdir,WantDirectories,nameonly(TF or 0,1,2)
		{
			int no;
			nArgs = GetFunArgs(Args, Arg, 8, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse; 
			no = atoi(Arg[7]);
			if (no != 2)
				no = atob(Arg[7]);
			n = GetFileList(Arg[1], atob(Arg[2]), Arg[3], Arg[4], atob(Arg[5]), atob(Arg[6]), no);
			itoa (n,OutLoc,10);
			goto Rtnl;
		}

		case 846: // $CHECKSUM(filename,frombyte(opt),tobyte(opt)) neg frombyte implies last abs(frombyte) bytes
		{
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse; 
			n = GetFileChecksum (Arg[1],atoi(Arg[2]),atoi(Arg[3]));
			itoa (n,OutLoc,10);
			goto Rtnl;
		}
		case 847: // $MAXSLOPE(frontslope,sideslope)
		{
			double frontSlope, sideSlope;

			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse; 
			frontSlope = atof (Arg[1]);
			sideSlope = atof (Arg[2]);
			RVal = ComputeMaxSlope (frontSlope,sideSlope);
			ftoa (OutLoc,RVal);
			goto Rtnl;
		}
		case 848: // $FILETIME(CREATE,file)
				  // $FILETIME(UPDATE,file)
		{
			time_t	fileTime=-1;
			long	time32=-1, TimeDiff;

			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse; 
			if (!stricmp (Arg[1],"CREATE"))
				fileTime = GetFileCreateTime (Arg[2],&TimeDiff);
			else if (!stricmp (Arg[1],"UPDATE"))
				fileTime = GetLastFileWriteTime (Arg[2],&TimeDiff);
			if (fileTime > 0)
				time32 = Time64toTime32 (fileTime);
			itoa (time32,OutLoc,10);
			goto Rtnl;
		}
		case 849: // $FUNCTION(NAME,ARG1,ARG2=n,ARG3...,ARG16=x,{function def})//up to 16 args. If an arg is followed by = it is optional
																				//function def limited to MAXARGLENGTH
		{

			rtn = FALSE;
			nArgs = GetFunArgs(Args, Arg, -16, &hMem, pBrkPt, bpOffset, bpLen);

			for (i=0;i<nArgs;i++)
			{
				int lnArg = strlen (Arg[i]);

				if (*Arg[i] == '{' && *(Arg[i] + lnArg - 1) == '}')
				{
					rtn = TRUE;
				}
			}
			goto Rtnrtn;
		}

		case 850: // $FILEPART(DRIVEDIR,filepath)
			// $FILEPART(DIR,filepath)
			// $FILEPART(DRIVE,filepath)
			// $FILEPART(NAME,filepath)
			// $FILEPART(EXT,filepath)
		{

			rtn = FALSE;
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			LPSTR drive = Arg[3];
			LPSTR dir = drive + _MAX_DRIVE + 1;
			LPSTR name = dir + _MAX_DIR + 1;
			LPSTR ext = name + _MAX_FNAME + 1;
			LPSTR lastdir;

			if (!stricmp(Arg[1], "ACTUAL"))
			{
				ConvertToNewLocation(Arg[2], FALSE);
				strcpy(OutLoc, Arg[2]);
			}
			else
			{
				_splitpath(Arg[2], drive, dir, name, ext);
				if (*dir)
				{
					dir++;
					*LastChr(dir) = 0;
				}
				if (*ext == '.')
					ext++;
				if (!stricmp(Arg[1], "DRIVE"))
					strcpy(OutLoc, drive);
				else if (!stricmp(Arg[1], "DRIVEDIR"))
					sprintf(OutLoc, "%s\\%s", drive, dir);
				else if (!stricmp(Arg[1], "DIR"))
					strcpy(OutLoc, dir);
				else if (!stricmp(Arg[1], "LASTDIR"))
				{
					if ((lastdir = strrchr(dir, '\\')))
						strcpy(OutLoc, ++lastdir);
					else
						strcpy(OutLoc, dir);
				}
				else if (!stricmp(Arg[1], "WOLASTDIR"))
				{
					if ((lastdir = strrchr(dir, '\\')))
						*lastdir = 0;
					sprintf(OutLoc, "%s\\%s", drive, dir);
				}
				else if (!stricmp(Arg[1], "NAME"))
					strcpy(OutLoc, name);
				else if (!stricmp(Arg[1], "NAMEEXT"))
					sprintf(OutLoc, "%s.%s", name, ext);
				else if (!stricmp(Arg[1], "EXT"))
					strcpy(OutLoc, ext);
				else if (!stricmp(Arg[1], "WOEXT"))
				{
					if (!(lastdir = strrchr(Arg[2], '\\')))
						lastdir = Arg[2];
					if ((lastdir = strrchr(lastdir, '.')))
						*lastdir = 0;
					strcpy(OutLoc, Arg[2]);
				}
			}
			goto Rtnl;
		}

		case 851: // $MOVEFILE(Frompath,Topath)
		{
			nArgs = GetFunArgs(Args, Arg, 7, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs == 2)
			{
				if (!makedirectories(Arg[2], FALSE, FALSE))
					goto RtnFalse;
				if (MoveFile(Arg[1], Arg[2]))
					goto RtnTrue;
			}
			goto RtnFalse;
		}
		case 852://$HEADTOAZ(heading in deg)
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			AZ = atof(Arg[1]);
			AZ *= RADDEG;
			AZ = LTWOPI(HALFPI - AZ);
			ftoa(OutLoc, AZ);
			goto Rtnl;
		}
		case 853://$DATABASE()
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (!stricmp(Arg[1], "RECORD"))//$DATABASE(RECORD,DBID) outputs record in text format
			{
				GetDatabaseRecord(Arg[2], OutLoc);
			}
			else
			{
				GetFGDBTable(hWndMain, Arg[1]);
				strcpy(OutLoc, Arg[1]);
			}
			goto Rtnl;
		}
		case 854://$GMMOBILE(DUPREFS)
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (!stricmp(Arg[1], "DUPPAR"))
			{
				nlong = FindDupParcels(Arg[2]);
				ltoa(nlong, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "COMPRESS"))
			{
				nlong = GMMCompression(Arg[2], Arg[3]);
				ltoa(nlong, OutLoc, 10);
				goto Rtnl;
			}

		}
		case 855: //$MAKELONG(v1,v2)
		{
			ULONG ulong;
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			ulong = MAKELONG(atoi(Arg[1]), atoi(Arg[2]));
			ltoa(ulong, OutLoc, 10);
			goto Rtnl;
		}
		case 856://$TEXTFILE(OPEN,pathname,READorWRITEorRorW)  returns fid
			//$TEXTFILE(READ,fid,varname,at(opt)) puts text into varname returns T or F
			//$TEXTFILE(WRITE,fid,text)
			//$TEXTFILE(CLOSE,fid)
			//$TEXTFILE(REPLACE,file,fromtext,totext)
		{
			HFILE fid=-1;
			rtn = 0;
			nArgs = GetFunArgs(Args, Arg, 7, &hMem, pBrkPt, bpOffset, bpLen);
			if (!stricmp(Arg[1], "OPEN"))
			{
				if (*Arg[3] == 'R' ||!*Arg[3])
				{
					fid = GSSiOpenFile(Arg[2], 0, OF_READ);
				}
				else if (*Arg[3] == 'C')
				{
					fid = GSSiOpenFile(Arg[2], 0, OF_CREATE);
				}
				else if (*Arg[3] == 'W')
				{
					fid = GSSiOpenFile(Arg[2], 0, OF_READWRITE);
					GSSillseek(fid, 0, 2);
				}
				ltoa(fid, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "READ"))	//if just arg4 search for next line with arg4 string
												//if arg4,5 and 6 search for string begining with arg4 containing arg5 and ending with arg6, if arg7 it indicates which instance
			{
				LPSTR line = malloc(MAXVARLEN);
				fid = atoi(Arg[2]);
				if (*Arg[4] && !*Arg[5])
				{
					while (fgetstring(line, MAXVARLEN - 2, fid))
					{
						if (strstr(line, Arg[4]))
						{
							rtn = 1;
							SetGlobalValue4(Arg[3], line, TRUE, 0, 0, 0);
							break;
						}
					}
				}
				else if (*Arg[4] && *Arg[5]  && *Arg[6])
				{
					int which = max (1,atoi(Arg[7]));
					int whichat = 0;
					GSSillseek(fid, 0, SEEK_SET);
					int lFile = GSSifilelength(fid);
					HANDLE hFile = GSSiGlobAlloc(0, GMEM_MOVEABLE, lFile+4);
					LPSTR pFile = GlobalLock(hFile);
					LPSTR pAt = pFile, pBeg, pEnd;
					LPSTR pStart = pFile;
					int len = 0;
					while (fgetstring(pAt, lFile - len, fid))
					{
						int l = strlen(pAt);
						pAt += l;
						len += l;
					}
					while (whichat < which)
					{
						pAt = strstr(pStart, Arg[5]);
						if (!pAt)
						{
							GSSiGlobUlFree(&hFile);
							goto RtnFalse;
						}
						pStart = pAt + 1;
						pBeg = GetBeginString(pFile, pAt, Arg[4], Arg[5], Arg[6]);
						if (pBeg)
						{
							pEnd = GetEndString(pAt, Arg[4], Arg[5], Arg[6]);
							if (pEnd)
								whichat++;
						}
					}
					*pEnd = 0;
					pBeg++;
					SetGlobalValue(Arg[3], pBeg);
					GSSiGlobUlFree(&hFile);
					goto RtnTrue;
				}
				else if (fgetstring(line, MAXVARLEN - 2, fid))
				{
					rtn = 1;
					SetGlobalValue4(Arg[3], line,TRUE,0,0,0);
				}
				free(line);
			}
			else if (!stricmp(Arg[1], "WRITE"))
			{
				fid = atoi(Arg[2]);
				rtn = fputstring(Arg[3], fid);
			}
			else if (!stricmp(Arg[1], "CLOSE"))
			{
				fid = atoi(Arg[2]);
				rtn = !GSSiClose2 (&fid);
			}
			else if (!stricmp(Arg[1], "REPLACE"))
			{
				fid = GSSiOpenFile(Arg[2], 0, OF_READ);
				if (fid != HFILE_ERROR)
				{
					int size = GSSifilelength(fid);
					int maxmem = size * 2 + 128;
					LPSTR mem = malloc(maxmem);
					LPSTR end = mem + size;
					BigRead(fid, mem, size);
					*end = 0;
					REPLAC(mem,Arg[3], Arg[4],maxmem);
					size = strlen(mem);
					GSSiClose2 (&fid);
					fid = GSSiOpenFile(Arg[2], 0, OF_CREATE);
					BigWrite(fid, mem, size, -1);
					GSSiClose2 (&fid);
					free(mem);
					rtn = 1;
				}
			}
			goto Rtnrtn;
		}
		case 901: // $ADDSEARCH(address,city,zip,outaddressvar,outcoordvar,matchOpt(1,2 or 3)) address search
		{
			nArgs = GetFunArgs(Args, Arg, 7, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			nlong = AddMatchSingle(Arg[1], Arg[2], Arg[3], Arg[7], &Point, 0, atoi(Arg[6]));
			if (*Arg[4])
				SetGlobalValue (Arg[4],Arg[7]);
			if (*Arg[5])
				SetGlobalValueDPoint (Arg[5],Point);
			ltoa (nlong,OutLoc,10);
			goto Rtnl; 
		} 
			
		case 902:// $REDISPLAY()
		{   
			if (Printing)
				goto RtnTrue;
			skipPaint = 0;
			if (*Args && CurView) 
			{
				 HDC hDC = 0;

       			 HaltMapDisplay(FALSE,FALSE);  
       			 if (!CurrentConfig)
       			 {  
	       			 SetConfig (1); 
	       			 SetViewport (1);
	       		 }
				 DisplayCycle++;
				 TotCopySize = 0;
				 if (!CurView->hDC)
				 {
					 hDC = GetDC(hWndMain);
					 for (i = 0; i < *pNumViewports; i++)
						 pViewports[i]->hDC = hDC;
				 }
		     	 SetMainRect (CurView->hWnd,CurView->hDC,0,0);
			     SetupViewports (CurView->hWnd,CurView->hDC,0,MainRect,0); 
	           	 RedisplayViewports(TRUE);
				 if (hDC)
					 ReleaseDC(hWndMain, hDC);
			}			
			else
			{
				HaltMapDisplay(FALSE,FALSE);
				ii=1;
				ClearFullWindowBitmap(CurView->hWnd);
				setDoPaint(TRUE);
				PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
			}
			if (hWndPNParms)
				PostMessage(hWndPNParms, WM_COMMAND, IDC_SHOWSCALE, 0L);
			goto RtnTrue;
		}
		
		case 903: // $GMDUPDATE(file,setkey,setvars,truncate(0,1or2 def 1 adds even if fields dont exist - 2 adds but warns),updateonly(opt-will not add new rec if true - default F)) 
			      // returns 1 if added and no existing record, 2 if added and previous record existed.
				  // ex: $GMDUPDATE(file.gmd,KEY1=A;KEY2=B,VAL1=1;VAL3=3) 
				  // ex: $GMDUPDATE(file.gmd,KEY1=A;KEY2=B,FileID) updates all fields in file FileID that are in file.gmd
		{	 
			BOOL updateOnly;
			GMDUpdateConvertCommas(Args);
			nArgs = GetFunArgs(Args, Arg, -5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 3)
				goto RtnFalse;  
			ExpandText (Arg[1]);
			ExpandText(Arg[4]);
			ExpandText(Arg[5]);
			if (*Arg[4])
				n=atoi(Arg[4]);
			else
				n=1;
			updateOnly = atob(Arg[5]);
			n = UpdateGMDFile (Arg[1],Arg[2],Arg[3],';',n,updateOnly);
			itoa (n,OutLoc,10);
			goto Rtnl;
		}  
		break;  
		
		case 906: // $GMDDELETE(file,SQL)  
		{	 
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			n = DeleteGMDRecords (Arg[1],Arg[2]);
			itoa (n,OutLoc,10);
			goto Rtnl;
		}  
		break;  
		
		case 905: // $DELETEREF(refno)  
		{	 
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (!nArgs)
				goto RtnFalse;
			SetCurView ( SetVPFromName (Arg[2],&Err)); 
			Refno = atol (Arg[1]);  
			if (DeleteRefno (Refno))
				goto RtnTrue;  
			goto RtnFalse;
		}  
		break;  
		
		
		case 907: // $MATCHTRAN(jlinefile,filelist,Dist) matches points in tran files withing dist  
		{	 
			
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs != 3)
				goto RtnFalse;
            if (MatchTranFunction (Arg[1],Arg[2],Arg[3]))
				goto RtnTrue;
			goto RtnFalse;
		}  
		break;      
		
		case 908: // $CREATESYM (symname,type(0=par,1=point,2=line,3=area),parent,desc,replace)
		{	 
			short	addorcreate=1, Type;
			
			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
		    Type = atoi (Arg[2]);  
		    TORF = atob (Arg[5]);    
		    if (TORF)
		    	addorcreate=3;
			str = Arg[6];
		    sprintf (str,"$STR(%s,PAR=%s,DES=%s)",Arg[1],Arg[3],Arg[4]);
			n = GetOrCreateSym (hWndMain,str,0,0,addorcreate,Type);
			itoa (n,OutLoc,10);
			goto Rtnl;
		}  
		break;
		
		case 909://$GETINIVAL(pathname.ini,section,name,default)
		{
			int ln;
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 
			if (!GetShortPathName(Arg[1], Arg[6], MAX_PATH))
				strcpy (OutLoc,Arg[4]);
			else
				GetPrivateProfileString (Arg[2],Arg[3],Arg[4],OutLoc,256,Arg[6]); 
			goto Rtnl;
		}
		break;
		
		case 910://$SETINIVAL(pathname.ini,section,name,value)
		{
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (WritePrivateProfileString (Arg[2],Arg[3],Arg[4],Arg[1]))
				goto RtnTrue;
			goto RtnFalse;
		}
		break;
		
		case 911: // $POLYPOINT(PointNum,X or Y or B) 
		{	 
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0; 
			if (strcspn (Arg[2],"XxYyBb") < 6)
			{
				if (stricmp (Arg[1],"LAST"))
					nlong = nCurPolyPoints;
				else
					nlong = IDNINT (atof(Arg[1]));
				if (nlong > nCurPolyPoints || nlong < 1 || !hCurPolyPoints)
					goto Rtnl;
				pPoint = (LPDPOINT)GlobalLock (hCurPolyPoints);
				if (*Arg[2] == 'X' || *Arg[2] == 'x') 
					sprintf (OutLoc,"%f",pPoint[nlong-1].x);
				else if (*Arg[2] == 'Y' || *Arg[2] == 'y') 
					sprintf (OutLoc,"%f",pPoint[nlong-1].y);
				else
					sprintf (OutLoc,"%lf %lf",pPoint[nlong-1].x,pPoint[nlong-1].y);
			}
			else
			{
				long	iref = atol(Arg[2]);
		    	long	npnts;
			   	HANDLE	hPoly;
				HPDPOINT	Point;

				nlong = IDNINT (atof(Arg[1]));
				if (!PickByRefno(iref,0,0,-1))
					goto Rtnl; 
				if (!GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&npnts,&hPoly))
					goto Rtnl;
				nlong = max (1,min (npnts,nlong));
				Point = (HPDPOINT)GlobalLock (hPoly);
				dpointtoa (OutLoc,&Point[nlong-1]);
				GSSiGlobUlFree (&hPoly);
			}
			goto Rtnl;
		}
		
		case 912: // $VEHUPDATE(vehid,point,speed,heading,status,display,validcoord,fromhistory) 
		{	 
			BOOL	ValidCoord;

			nArgs = GetFunArgs(Args, Arg, 9, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			if (!_fstrnicmp (Arg[2],"Invalid",7))
				goto RtnFalse;
			Point = atopt (Arg[2],&Err); 
			if (*Arg[7])
				ValidCoord = atob(Arg[7]);
			else
				ValidCoord = TRUE;
			if (SetVehicleLoc (Arg[1],Point,(short)atof(Arg[3]),(short)atof(Arg[4]),atoi(Arg[5]),atob(Arg[6]),ValidCoord,atoi(Arg[8]),atob(Arg[9])))
				goto RtnTrue;
			else
				goto RtnFalse;
		}
		
		case 913: // $HLTOUTPUT()
		{	 
			CreateHighlightOutput (0,0,0,0);
			goto RtnTrue;
		}
		
		case 914: // $INSTALLCD(FromPath,ToPath,SearchString)
		//locates(using searchString) all indexes and global.ini in FromPath and copies to ToPath
		//updates ToPath\filelist.txt and updates [CDLIST] in geomastr.ini
        {
              DLGPROC	lpfnCDINSTALLERMsgProc;
              short		nRc; 
                  
              lpfnCDINSTALLERMsgProc = MakeProcInstance((DLGPROC)CDINSTALLERMsgProc, hInst);
              nRc = DialogBox(hInst, (LPSTR)"CDINSTALLER", hWndMain, lpfnCDINSTALLERMsgProc);
              FreeProcInstance(lpfnCDINSTALLERMsgProc);  
              if (nRc)
              	goto RtnTrue;
              else
              	goto RtnFalse;
        }
		case 915: // $ORTHOSIZE() shows size of orthos plotted since last call  
			hMem = GSSiGlobAlloc (1176,GMEM_MOVEABLE,2048);
			Arg1 = GlobalLock(hMem);  
			sprintf (Arg1,"%ld Records, %lu bytes",nOrthoBlocks,nOrthoBytes); 
			MessageBox (GetFocus(),Arg1,"",MB_OK);
			nOrthoBlocks=0;  
			nOrthoBytes=0;
			goto RtnTrue;
		
		case 916:	//$GMDCREATE(Name,numkeyfld,defstring,Compress(YorN)) if numkeyfld == 0 defstring is path of db to copy from
		{
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 3)
				goto RtnFalse;
			n = atoi (Arg[2]);
			if (CreateGWDDatabase (Arg[1],1,atob(Arg[4]),0,n,Arg[3]))
				goto RtnTrue;
			goto RtnFalse;
		}
		
		case 917:	//$FINDFILES(Dir,wildcard,outfile (opt),WantSub,HeaderRecord,sort(TORF))
		{
			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			if (*Arg[3])
			{
				Fid =	GSSiOpenFile (Arg[3],0,OF_CREATE);
				if (*Arg[5])
					fputstring (Arg[5],Fid);
			}
			else
				Fid = HFILE_ERROR;
			nlong = 0;
			SearchFilesInDir (Arg[1],0, Fid,&nlong,Arg[2],1,atob(Arg[4]),TRUE);
			if (Fid != HFILE_ERROR)
			{
				if (atob(Arg[6]))
				{
					BTVARDESC	BTVar[2];
					HANDLE	hBTTemp;
					HANDLE hTmp = GSSiGlobAlloc(0, GMEM_MOVEABLE, MAX_PATH);
					LPSTR pTempFile = GlobalLock(hTmp);
					HANDLE hLine = GSSiGlobAlloc(0, GMEM_MOVEABLE, 4096);
					LPSTR pLine = GlobalLock(hLine);
					int offset=0;
					int pos = BT_FIRST;
					char sortedFile[MAX_PATH];
					
					sprintf (sortedFile,"%s.srt", Arg[3]);
					Fid2 = GSSiOpenFile(sortedFile, 0, OF_CREATE);

					GSSillseek(Fid, 0, 0);
					GSSiGetTempFileName(0, "gma", 0, pTempFile);

					BTVar[0].BT_VARTYP = BT_CHAR;
					BTVar[0].BT_VARLEN = 128;
					BTVar[0].BT_VAROFF = 0;
					BT_CREATE(pTempFile, 4, FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
					hBTTemp = BT_OPEN(pTempFile, 0, BT_WRITE, 0);
					fgetstring(pLine, 4090, Fid);
					offset = GSSillseek(Fid, 0, 1);
					while (fgetstring(pLine, 4090, Fid))
					{
						LPSTR pTab = strchr(pLine, '\t');
						if (pTab)
							*pTab = 0;
						pTab = strrchr(pLine, '\\');
						if (pTab)
							pTab++;
						else
							pTab = pLine;
						BT_PUT(hBTTemp, pTab, (LPSTR)&offset);
						offset = GSSillseek(Fid, 0, 1);
					}
					GSSillseek(Fid, 0, 0);
					fgetstring(pLine, 4090, Fid);
					fputstring(pLine, Fid2);
					while (!BT_FIND(hBTTemp, pLine, pos, BT_ANY, (LPSTR)&offset))
					{
						pos = BT_NEXT;
						GSSillseek(Fid, 0, offset);
						fgetstring(pLine, 4090, Fid);
						fputstring(pLine, Fid2);
					}
					GSSiClose2 (&Fid2);
					GSSiClose2 (&Fid);
					Fid = HFILE_ERROR;
					GSSiRemove(Arg[3]);
					GSSiRename(sortedFile, Arg[3]);
					BT_CLOSEANDDELETE(&hBTTemp);
					GSSiGlobUlFree(&hLine);
					GSSiGlobUlFree(&hTmp);
				}
				GSSiClose2 (&Fid);
			}
			ltoa (nlong,OutLoc,10);
			goto Rtnl; 
					      
		}
		
		case 918:	//$AZMTOBEAR(azm,format)
		{
			char	DegC[8],MinC[8],SecC[16], PreDir[4], PostDir[4]; 
			
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			n = atoi (Arg[2]);  
			AZ = atof (Arg[1]);
			if (n)
			{
				double AN = LTWOPI(HALFPI - AZ);
				ftoa (OutLoc,AN*DEGRAD);
			}
			else
			{
				AZToBear (AZ,PreDir,DegC,MinC,SecC,PostDir); 
				if ((pEnd = _fstrchr (SecC,'.')))
				{
					pEnd += n+1;
					*pEnd = 0;
				}
				sprintf (OutLoc,"%s %s %s %s %s",PreDir,DegC,MinC,SecC,PostDir);
			}
			
			goto Rtnl; 
		}
		
		case 919: // $SYMDELETE(symnum,close(TF))
		{	 
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			n = atoi (Arg[1]);
			if (DeleteSymbol (n,atob(Arg[2])))
				goto RtnTrue;
			goto RtnFalse;
		}  
		break;
		
		case 920: // $GMDIMPORT(pathname,NorA(new or add),fromfile,SQL(none for all),keyfield1;keyfield2,field1;field2;or no fields for all,YorNCompress,YorNScanfortypes) 
				  // ex: $GMDIMPORT(file.gmd,N,ODBC|DRIVER|TABLE,,keyfield1(opttypelen);keyfield2,) imports all fields for all rows from
				  //                                                                  the specified ODBC table.
				  // note: if the add option is used the fields must match the existing fields. 
		{	 
			HANDLE	hFields, hKeyFields,hFieldTypes=0;
			BOOL	Create = TRUE;
			HANDLE  hValues = 0;
			
			nArgs = GetFunArgs(Args, Arg, 9, &hMem, pBrkPt, bpOffset, bpLen);
			if (!GetFieldIDsFromNames(Arg[3], &hKeyFields, &hFieldTypes, Arg[5], &hValues))
				goto RtnFalse;
			if (!GetFieldIDsFromNames (Arg[3],&hFields,&hFieldTypes,Arg[6],&hValues))
			{
				GSSiGlobFree (&hKeyFields);
				goto RtnFalse; 
			}
			if (atob(Arg[8]))
				ScanForFieldTypes (Arg[3],&hFieldTypes,TRUE,atol(Arg[9]));
			if (*Arg[2] == 'A' || *Arg[2] == 'a')
				Create = FALSE;
			rtn = OutputToFile (Arg[1],Create,Arg[3],Arg[4], hFields,hKeyFields,hFieldTypes,hValues,FALSE,FALSE,2,atob(Arg[7]),atob(Arg[8]),atol(Arg[9]),(HWND)1,hWndMain,TRUE);
			GSSiGlobFree (&hKeyFields);
			GSSiGlobFree (&hFields);
			GSSiGlobFree (&hFieldTypes);
			if (hValues)
			{
				LPHANDLE pHandle = GlobalLock(hValues);
				while (*pHandle)
				{
					GSSiGlobFree(pHandle);
					pHandle++;
				}
				GSSiGlobUlFree(&hValues);
			}
			if (rtn) 
				goto RtnTrue; 
			else
            	goto RtnFalse;
			
		}  
		break;  
		
		case 921: // $LAYERNAME(layernum,viewport name(opt) 
		{	 
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			n = atoi (Arg[1]);
			SetCurView ( SetVPFromName (Arg[2],&Err)); 
			_fstrcpy (OutLoc,CurView->FileID[n]);
			goto Rtnl;
			
		}  
		break;  

		case 922: // $ADDUNIQUE(InFile,OutFile)
		{	
			long	id=1;
			char	Delim=',';
			
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			Fid = GSSiOpenFile (Arg[1],0,OF_READ);
			if (Fid == HFILE_ERROR)
	           	goto RtnFalse; 
			Fid2 = GSSiOpenFile (Arg[2],0,OF_CREATE);    
			fgetstring (Arg[3],2000,Fid);
			if (_fstrchr (Arg[3],'\t')) 
				Delim = '\t';
			sprintf (_fstrchr(Arg[3],0),"%cUNIQUE_ID",Delim);
			fputstring (Arg[3],Fid2);
			while (fgetstring (Arg[3],2000,Fid)) 
			{   
				sprintf (_fstrchr(Arg[3],0),"%c%ld",Delim,id++);
				fputstring (Arg[3],Fid2);
			}
	        GSSiClose2 (&Fid); 
	        GSSiClose2 (&Fid2);
			goto RtnTrue;
		} 
		
		case 923: // $TRANIMAGE(ImageFile,TranFile,OutBMPFile,OutBPWFile,AreaFileOrTAG)
		{	
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (TransformImage (Arg[1],Arg[2],Arg[3],Arg[4],Arg[5]))
				goto RtnTrue;
			goto RtnFalse;
		} 
		
		case 924: // $SYMPARENT(symnum)  
		{				
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			SymNum = atol (Arg[1]);
			if ((SymNum = GetDictSymParent (SymNum)))
				GetDictSymName (SymNum,OutLoc);
			goto Rtnl;   
		}
		
				
		case 925: /* $SEWERDATA(cmdid) invokes menu command */
		{	
			long	ICmd;
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
		    
		    if (!_fstricmp (Arg[1],"STORMPIPE"))
            {
                  DLGPROC	lpfnSTRMPIPEMsgProc; 
                  
                  if (!hWndStrmPipe)
                  {
	                  lpfnSTRMPIPEMsgProc = MakeProcInstance((DLGPROC)STRMPIPEMsgProc, hInst);
	                  CreateDialog(hInst, (LPSTR)"STRMPIPE", hWndMain, lpfnSTRMPIPEMsgProc); 
	              }
            } 
            goto RtnTrue;
        }

		case 926: //$CURSORLOC(none W or C) returns world (default or W) or client coord of cursor
		{   
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			GetCursorPos (&Point16); 
			*OutLoc = 0;
			ScreenToClient (CurView->hWnd,&Point16);
			if (!SelectViewport (Point16,TRUE,FALSE,FALSE))
				goto Rtnl;
			if (!nArgs || stricmp (Arg[1],"C"))
			{
				Point=WinPtToBasePt(Point16);
				dpointtoa (OutLoc,&Point);
			}
			else
				pttoa (OutLoc,Point16);
			SetCurView (SaveVP);
			goto Rtnl;
		}
			
		case 927: // $CHECKTRAN(Transformation file,NorRorM (N returns npoints,R returns RSQ,M returns max resid,dir (def 1),Type (def 1))
		{
			HANDLE	hTran;
			int		dir,type;
			
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			dir = atoi (Arg[3]);
			if (!dir)
				dir = 1;
			type = atoi (Arg[4]);
			if (!type)
				type = 1;
			hTran=LoadTranFile(Arg[1],dir,type,&n,&RVal);
			CloseTRANS2 (&hTran);
			if (*Arg[2] == 'N' || *Arg[2] == 'n')
				itoa (n,OutLoc,10);
			else if (*Arg[2] == 'R' || *Arg[2] == 'r')
				sprintf (OutLoc,"%f",RVal);
			else
			{
				RVal = 
				sprintf (OutLoc,"%f",RVal);
			}
			goto Rtnl;
		} 

		case 928: //$CHANGETAG(Refno,newTAG)  
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			Refno = atol (Arg[1]); 
			if (!PickByRefno (Refno,0,0,-1))
				goto RtnFalse;
			ProcessPickedItem (0,FALSE);        		
			if (EditTAG (0,Arg[2]))
				goto RtnTrue;
			goto RtnFalse;
		}
		
		case 929: //$LAYERPATH(Layer name or #,opt viewport name)  
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0; 
			_fmemmove (Arg[1]+6,Arg[1],_fstrlen (Arg[1])+1);
			_fstrncpy (Arg[1],"LAYER:",6);
			if (GetMapName (Arg[1]))
				_fstrcpy (OutLoc,Arg[1]);
			goto Rtnl;
		}
		
		case 930: //$FINDFIELD(FIELD or TABLE,DBName,partialfieldname,outfile)
        {
		  nArgs = GetFunArgs (Args,Arg,4,&hMem, pBrkPt, bpOffset, bpLen);
		  if (nArgs < 4)
		  	goto RtnFalse;
		  if (!_fstricmp (Arg[1],"FIELD")) 
		  	n = FindFieldName (Arg[2],Arg[3],Arg[4]);
		  else	
		  	n =FindTableName (Arg[2],Arg[3],Arg[4]);
		  ltoa (n,OutLoc,10);
		  goto Rtnl;
		} 
		 
		case 931: //$GETPICKED(item)  
		{
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			n = atoi(Arg[1]);  
			if (n && n <= NumPicked)
			{
				ProcessPickedItem (n-1,FALSE);
				goto RtnTrue;
			}			
			goto RtnFalse; 
		}
			
		case 932: //$SENDEMAIL(from,to,subject,message(or body),attach,html
		{
			nArgs = GetFunArgs (Args,Arg,-7,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 3)
				goto RtnFalse;
			ExpandText(Arg[2]);
			ExpandText(Arg[3]);
			ExpandText(Arg[4]);
			ExpandText (Arg[5]);
			ExpandText (Arg[6]);
			rtn = SendEmail(Arg[1], Arg[2], Arg[3], Arg[4], Arg[5], Arg[6], Arg[7]);
			SetGlobalValue ("%EMAILRESPONSE",Arg[7]);
			goto Rtnrtn; 
		}

		case 933: //$URLTOFILE(URL,File)
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			if (URLToFile (Arg[1],Arg[2]))
				goto RtnTrue;
			goto RtnFalse; 
		}

		case 934: //$STREETNUM(Name,Add(TorF))
		{
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (atob (Arg[2]))
				nlong = AddStreetName (Arg[1],0,"","","",""); 
			else
				nlong = GetStreetNumFromRawName (Arg[1],Arg[2]);
			ltoa (nlong,OutLoc,10); 
			goto Rtnl;
		}  

		case 935: //$FIELDDEFS(DBName,DoScan,RowsToScan,format)
		{  
			HANDLE	hFieldTypes=0;
		    LPGWFLDINFO pFldInfo; 
		    FIELDINFO	FldInfo;
			char delim[2] = { 0 };
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			int format = atoi(Arg[4]);
			n = ScanForFieldTypes(Arg[1], &hFieldTypes, atob(Arg[2]), atol(Arg[3]));
			if (!n)
				goto RtnFalse;
			pFldInfo = (LPGWFLDINFO)GlobalLock (hFieldTypes); 
			*OutLoc = 0; 
			for (i=0; i<n; i++,pFldInfo++)
			{   
				memset (&FldInfo,0,sizeof(FldInfo));
				FldInfo.length = pFldInfo->Len;    
				FldInfo.type = pFldInfo->Type;
				strncpy0 (FldInfo.name,pFldInfo->Name,32);
				if (!strnicmp(pFldInfo->Name, "PedButton",9))
					ii = 1;
				switch (format)
				{
				case 0:
					CreateGMTextHeader (&FldInfo, OutLoc);
					_fstrcpy (_fstrchr (OutLoc,0),",");
					break;
				case 1:
					if (pFldInfo->Type == BT_CHAR)
						sprintf(strchr(OutLoc, 0), "%s'[%s]'", delim, pFldInfo->Name);
					else
						sprintf(strchr(OutLoc, 0), "%s[%s]", delim, pFldInfo->Name);
					break;
				}
				delim[0] = '\t';
			}
			if (!format)
				*LastChr(OutLoc) = 0;	
			GSSiGlobUlFree (&hFieldTypes); 
			goto Rtnl;
		}  
		case 936: //$NEWLATLON(StartPT,distinmeters,AZ)
		{
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			Point = atopt (Arg[1],&Err); 

			Point = NewLatLong(Point.y,Point.x,atof(Arg[2]),atof(Arg[3]));
			dpointtoa (OutLoc,&Point);
			goto Rtnl;
		}
	
		case 937: //$INTERSECT(REF1,REF2,outvarname(opt - use %WX %WY by default)
		{
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			if (IntersectRefs(Arg[1],Arg[2],Arg[3],Arg[4]))
				goto RtnTrue;
			goto RtnFalse;
		}
	
		case 938: //$MOVEPOINT(refno,newcoord)
		{
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			Refno = atol (Arg[1]);
			Point = atopt (Arg[2],&Err);
			if (Err)
				goto RtnFalse;
			if (MovePointItem (Refno,Point))
				goto RtnTrue;
			goto RtnFalse;
		}
	
		case 939: //$COMBOFILE(CREATE,name,fielddefs,reffile1,...reffilen)
		{
			nArgs = GetFunArgs(Args, Arg, 12, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 3)
				goto RtnFalse;
			rtn = CreateComboFile (Arg[2],Arg[3],Arg[4],Arg[5],Arg[6],Arg[7],Arg[8],Arg[9],Arg[10],Arg[11],Arg[12]);
			goto Rtnrtn;
		}

		case 940: //TCPRETURN(val)
		{
			LPSTR	CmdMess = GlobalLock (hCmdMess);

			nArgs = GetFunArgs (Args,Arg,1,&hMem, pBrkPt, bpOffset, bpLen);
			if (strlen(Arg[1]) < MAX_CMDMESSAGE)
				strcpy (CmdMess,Arg[1]);
			GlobalUnlock (hCmdMess);
			goto RtnTrue;
		}

		case 941: //NEARPOINT(CREATE,NPTable,FromDB,FromSQL,FromPoint,FromRef,FromDesc,FromPrefix,FromUDI)
				  //NEARPOINT(FIND,Point,idesc,FoundPointVar,FoundRefVar)
		{
			nArgs = GetFunArgs(Args, Arg, -9, &hMem, pBrkPt, bpOffset, bpLen);
			ExpandText (Arg[1]);
			ExpandText (Arg[2]);
			if (!stricmp (Arg[1],"CREATE"))
			{
				if (CreateNearestPointTable (Arg[2],Arg[3],Arg[4],Arg[5],Arg[6],Arg[7],Arg[8],Arg[9]))
					goto RtnTrue;
			}
			else if (!stricmp (Arg[1],"FIND"))
			{
				ExpandText (Arg[3]);
				ExpandText (Arg[4]);
				Point = atopt (Arg[3],&Err);
				SymNum = atoi (Arg[4]);
				if (!Err)
				{
					if (GetNearestPointFromTable (Arg[2],&Point,SymNum,&Point,&Refno,0,0))
					{
						ExpandText (Arg[5]);
						SetGlobalValueDPoint (Arg[5],Point);
						ExpandText (Arg[6]);
						SetGlobalValueLong (Arg[6],Refno); 
						goto RtnTrue;
					}
				}
			}
			goto RtnFalse;
		}

		case 942: //$POINTLIST(CREATE,name,pointlist)
				  //$POINTLIST(DESTROY,name)
				  //$POINTLIST(ADD,name,pointlist)
				  //$POINTLIST(THIN,name,dist(if 0 removes dup points))
				  //$POINTLIST(DISPLAY,name,FILL,color)
				  //$POINTLIST(DISPLAY,name,DRAW,color,width)
				  //$POINTLIST(LENGTH,name)
				  //$POINTLIST(AREA,name)
				  //$POINTLIST(AZM,name,pct,before;after;at(default)) at averages before and after if at node point
				  //$POINTLIST(INTERSECT,name,name2,COUNT;id;Farthest;nearest,farornearpoint)
		{
			nArgs = GetFunArgs (Args,Arg,8,&hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (PointListCommands (nArgs,Arg,OutLoc))
				goto Rtnl;
			goto RtnFalse;
		}

		case 943: //$POINTINVP(type(WORLD or SCREEN),x y,VPName)
		{
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			Point = atopt (Arg[2],&Err); 
			if (Err)
				goto RtnFalse;
			SetCurView ( SetVPFromName (Arg[3],&Err)); 
			if (Err)
				goto RtnFalse;
			Point16 = BasePtToScreenPt (&Point);
			rtn = PtInRect (&CurView->ScreenRect,Point16);
			SetCurView ( SaveVP);
			goto Rtnrtn;
		}

		case 944: //$BLOCKTEXT(text,factor)
		{
			float f=1;

			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (nArgs > 1)
				f = atof (Arg[2]);
			BlockText (CurView->hDC,Arg[1],4096,f);
			strcpy (OutLoc,Arg[1]);
			goto Rtnl;
		}

		case 945: //$LINKLINES(CLEAR);
				  //$LINKLINES(ADD,linelist)
				  //$LINKLINES(COMPUTE) return #lines
				  //$LINKLINES(GET,line #)
		{
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			LinkLinesFunction (Arg[1],Arg[2],OutLoc);
			goto Rtnl;
		}
		case 946: //$CLEARFILE(filepath)
		{
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			Fid = GSSiOpenFile (Arg[1],0,OF_CREATE);
			if (Fid != HFILE_ERROR)
			{
				GSSiClose2 (&Fid);
				rtn = TRUE;
			}
			else
				rtn = FALSE;
			goto Rtnrtn;
		}
		case 947: //$CLIPBOARD(CAPTURE,title,menu)
		{
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (!stricmp(Arg[1], "CAPTURE"))
			{
				nRc = CaptureClipboard(Arg[2], Arg[3]);
				if (nRc)
					goto RtnTrue;
			}
			goto RtnFalse;
		}
		case 948: //$MAPSERVER(TEST
			PostMessage(hWndMain, GF_MAPSERVER_REQUEST, (WPARAM)hWndMain, 1);
			goto RtnTrue;
			break;
		case 949: //$CASECOUNT(CAPTURE,title,menu)
		{
			int n = 0;
			BOOL wantUpper = TRUE;
			LPSTR pC;

			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			pC = Arg[1];
			if (*Arg[2] != 'U')
				wantUpper = FALSE;
			while (*pC)
			{
				if (isupper(*pC))
				{
					if (wantUpper)
						n++;
				}
				else
				{
					if (!wantUpper)
						n++;
				}
				pC++;
			}
			itoa(n, OutLoc, 10);
			goto Rtnl;
		}
		case 950: // $APPENDRAW(file,line)
		{

			nArgs = GetFunArgs(Args, Arg, -2, &hMem, pBrkPt, bpOffset, bpLen);
			ExpandText(Arg[1]);
			if (nArgs < 3)
				i = AppendFile(Arg[1], Arg[2]);
			else
				i = AppendFile2(Arg[1], Arg[2]);
			itoa(i, OutLoc, 10);
			goto Rtnl;
		}
		case 951: // $ADDQUOTES(dbid,fieldname,val,errorvarname)
		{
			HANDLE	hDB = 0;

			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			if (!(hDB = GetOpenDatabaseFromID(Arg[1])))
				goto RtnFalse;
			BOOL NeedQuote = NeedSQLValueQuote(hDB, Arg[2]);
			if (NeedQuote)
			{
				sprintf(OutLoc, "'%s'", Arg[3]);
			}
			else
			{
				strcpy(OutLoc, Arg[3]);
			}
			goto Rtnl;
		}
		case 1001: // $DECOMPPOLY(OutFile,InteriorLineDesc,ExteriorLineDesc,LinkBetweenNodes(Opt F)
		{	 
            DLGPROC lpfnDECOMPPOLYMsgProc; 
			
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 3)
				goto RtnFalse;
			hDCPSetup = GSSiGlobAlloc (1179,GMEM_MOVEABLE,1024);
			lpstr = GlobalLock (hDCPSetup);
			_fstrcpy (lpstr,Arg[1]);
			lpstr = _fstrchr (lpstr,0);
			lpstr++;
			_fstrcpy (lpstr,Arg[2]);
			lpstr = _fstrchr (lpstr,0);
			lpstr++;
			_fstrcpy (lpstr,Arg[3]); 
			GlobalUnlock (hDCPSetup);
			DecompLinkBtwnNodes = atob (Arg[4]);  
			DecompAutoRun = atob (Arg[5]);
            lpfnDECOMPPOLYMsgProc = MakeProcInstance((DLGPROC)DECOMPPOLYMsgProc, hInst);
            nRc = DialogBox(hInst, (LPSTR)"DECOMPPOLY", hWndMain, lpfnDECOMPPOLYMsgProc);
            FreeProcInstance(lpfnDECOMPPOLYMsgProc); 
             
            if (nRc)
              	goto RtnTrue;
            else
				goto RtnFalse;
		}
		
		case 1002: // $DELETEFILE(path,Y or N or T or F or 1 or 0 (verify switch))
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;

			if (!ExistFile (Arg[1]))
				goto RtnTrue;
			if (atob (Arg[2]))
			{   
				HANDLE	hTemp=GSSiGlobAlloc (1181,GMEM_MOVEABLE,512);
				LPSTR	Mess=GlobalLock (hTemp);
             	sprintf (Mess,"Delete file '%s'?",Arg[1]);
             	st = MessageBox (GetFocus(),Mess,"Verify Delete",MB_YESNO); 
				GSSiGlobUlFree (&hTemp);
			}
			else
				st = IDYES;
            
            if (st != IDYES)
            	goto RtnFalse;
            
			if (!GSSiRemove (Arg[1]))
				goto RtnTrue;
			else
            	goto RtnFalse;
			
		} 
		
		
		case 1003: //$STREETNAME(streetnum,index(opt))
		{
			long	SNum, index;
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
			{
				*OutLoc = 0;
				l=0;        
			}
			else
			{
				SNum = atol (Arg[1]);
				index = atol (Arg[2]);
				if (GetTrueStreetName (SNum,OutLoc,-1,index))
          			l = _fstrlen (OutLoc);
				else
				{
					*OutLoc = 0;
					l=0;        
				}
			}
			goto Rtnl;
		}
		
		case 1004: // $BOUNDPOINT(bounds,1-4 (ll,ul,ur,lr) or 5 center, or 6,7,8,9 for centerl, cu, cr, cl)
		{	
			short	opt;
						
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (sscanf(Arg[1], "%lf %lf %lf %lf", &Bounds.xmn,
										  		   &Bounds.ymn,
										  		   &Bounds.xmx,
										  		   &Bounds.ymx) != 4)
				goto RtnFalse;
			opt = atoi (Arg[2]);
			switch (opt)
			{
				case 1:
					Point.x = Bounds.xmn;
					Point.y = Bounds.ymn;
					break;
				case 2:
					Point.x = Bounds.xmn;
					Point.y = Bounds.ymx;
					break;
				case 3:
					Point.x = Bounds.xmx;
					Point.y = Bounds.ymx;
					break;
				case 4:
					Point.x = Bounds.xmx;
					Point.y = Bounds.ymn;
					break;   
				case 5:
					Point = MinMaxMidPointD (&Bounds);
					break;
				case 6:
					Point.x = Bounds.xmn;
					Point.y = (Bounds.ymn + Bounds.ymx)/2;
					break;
				case 7:
					Point.x = (Bounds.xmn + Bounds.xmx)/2;
					Point.y = Bounds.ymx;
					break;
				case 8:
					Point.x = Bounds.xmx;
					Point.y = (Bounds.ymn + Bounds.ymx)/2;
					break;
				case 9:
					Point.x = (Bounds.xmn + Bounds.xmx)/2;
					Point.y = Bounds.ymn;
					break;   
				default:
					goto RtnFalse;
			}
			sprintf (OutLoc,"%f %f",Point.x,Point.y); 
			l = _fstrlen (OutLoc);  
		    goto Rtnl;
		}  
		
		case 1005: //$PRINTSETUP() 
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (!_fstricmp (Arg[1],"READ"))
			{
           		Fid = GSSiOpenFile (Arg[2],0,OF_READ);
           		rtn = ReadPrintSetupData (Fid);                  
           		GSSiClose2 (&Fid);
			}
			else if (!_fstricmp (Arg[1],"SAVE"))
			{
           		Fid = GSSiOpenFile (Arg[2],0,OF_CREATE);
           		SavePrintSetupData (Fid,0);                  
           		GSSiClose2 (&Fid);  
           		rtn = TRUE;
			}
          	else
          	{
          		if (*Arg[1]) 
          			n = -2;
          		else
          			n = -1;
				rtn = PrintMap (hWndMain,0,n); 
			} 
			goto Rtnrtn;
		}
		
		case 1006: // $SAVESTATUS(varname,YorN) save variable save status
		{	
			BOOL	Save;
			LPSTR	pName;
						
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			Save = atob(Arg[2]);
			if (*Arg[1] == '(')
			{
				Arg[1]++;
				if (!(ParLoc = MatchLev (Arg[1],')')))
					goto Rtn0;
				*ParLoc = 0;
			}
			pName = Arg[1]; 
			while (pName)
			{
				if ((ParLoc = MatchLev (pName,',')))
					*ParLoc++ = 0;
				SetVarSaveStatus (pName,Save);
				pName = ParLoc;
			}
			goto RtnTrue;
		}
			
		case 1007: // $UPDATETIME(varname) last time var was updated
		{   VARPNT  VarPnt;   
			HANDLE	handle;
						
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (handle = FindVar(Arg[1]))
			{ 
				VarPnt = (VARPNT)GlobalLock (handle);
				ultoa (VarPnt->changetime,OutLoc,10);
				GlobalUnlock (handle);
			}
			else
				_fstrcpy (OutLoc,"0");
			goto Rtnl;
		}

		case 1008: // $REGISTERCD(FromPath,ToPath,SearchString)
		//locates(using searchString) all indexes and global.ini in FromPath and copies to ToPath
		//updates ToPath\filelist.txt and updates [CDLIST] in geomastr.ini
		{    
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (RegisterCD(Arg[1], Arg[2], Arg[3]))
            	goto RtnTrue;
            goto RtnFalse;
		}

		case 1009: // $PRINTMERGE(DataFile,MacroFile,YorNTest)
		{    
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			_fstrcpy (PMDataFile,Arg[1]);
			_fstrcpy (PMMacroFile,Arg[2]);
			FirstMergeRecord=1;
			LastMergeRecord=LONG_MAX;   
			if (atob (Arg[3]))
            {
                  
              lpfnPRINTMERGETSTMsgProc = MakeProcInstance((DLGPROC)PRINTMERGETSTMsgProc, hInst);
              CreateDialog(hInst, (LPSTR)"PRINTMERGETST", hWndMain, lpfnPRINTMERGETSTMsgProc); 
              goto RtnTrue;
            }
            if (PrintMerge(hWndMain))
            	goto RtnTrue;
            goto RtnFalse;
		}

        case 1010: //$VEHICLEDEF ()
        {
	          DLGPROC lpfnVEHICLEDEFMsgProc;
	
	          lpfnVEHICLEDEFMsgProc = MakeProcInstance((DLGPROC)VEHICLEDEFMsgProc, hInst);
	          rtn = DialogBox(hInst, (LPSTR)"VEHICLEDEF", hWndMain, lpfnVEHICLEDEFMsgProc);
	          FreeProcInstance(lpfnVEHICLEDEFMsgProc);  
	          if (rtn)
	          	goto RtnTrue;
	          else
	          	goto RtnFalse;
         }

		case 1011: // $SETSYMDESC(Symname,description)
		{   
			HANDLE		hSym;
			LPSYMBOL	pSym;
			 
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			n = GetDictSymbolNumber(Arg[1]);
			if (!n)
            	goto RtnFalse; 
			hSym = GetDictSymDesc (n,0);
			pSym = (LPSYMBOL)GlobalLock (hSym); 
			_fstrncpy (pSym->Desc,Arg[2],64);
			GlobalUnlock (hSym);
            ReplaceSymbol (n,hSym);
			DestroySymbol (hSym);  
            goto RtnTrue;
		}   
		
		case 1012: // $INSERTLINE(file,linetoinsertafter,line)
		{	
			LPSTR	HaveLine;
			
			nArgs = GetFunArgs(Args, Arg,5, &hMem, pBrkPt, bpOffset, bpLen);
			lpstr = Arg[4];
			*lpstr = 0;
			pFile = Arg[5];
			Fid = GSSiOpenFile(Arg[1], 0, OF_READ);
			if (Fid == HFILE_ERROR)
	           	goto RtnFalse; 
	        _fullpath (pFile,Arg[1],256); 
	        pEnd = _fstrrchr (pFile,'\\');
	        _fstrcpy (pEnd,"\\tempfile");
			Fid2 = GSSiOpenFile (pFile,0,OF_CREATE);    
			do
			{ 
				if ((HaveLine = fgetstring (lpstr,1024,Fid)))
					fputstring (lpstr,Fid2);
			}
			while (HaveLine && _fstrcmp (lpstr,Arg[2])); 
			fputstring (Arg[3],Fid2); 
			while (fgetstring (lpstr,1024,Fid))
				fputstring (lpstr,Fid2);
	        GSSiClose2 (&Fid); 
	        GSSiClose2 (&Fid2);
	        GSSiRemove (Arg[1]);
	        GSSiRename (pFile,Arg[1]);
			goto RtnTrue;
		} 
		
		case 1013: //$CROSSMATCH([%DL]attribut\frommap.txt,[%DL]attribut\fromars.txt,@[UDI],@$OS(@[GEOACCT]),[%DL]attribut\notinmap.txt,[%DL]attribut\notinars.txt)
		{
			nArgs = GetFunArgs(Args, Arg, 7, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 7)
				goto RtnFalse;
			l = atoi (Arg[7]);
			if (CrossMatch (Arg[1],Arg[2],Arg[3],Arg[4],Arg[5],Arg[6],l))
				goto RtnTrue;
			goto RtnFalse;
		}
		
		case 1014: //$AREAINAREA(THEME,BoundaryArea,Theme,GMDfile,MinPCT) 
				   //$AREAINAREA(RASTER,BoundaryArea,GMDfile,speed,MinPCT,IncludeBoundaries)
		{
			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
			RVal = atof (Arg[5]);
			SetCurView ( SetVPFromName (Arg[6],&Err)); 
			if (!_fstricmp (Arg[1],"THEME"))
			{
				if (AreaInArea (Arg[2],Arg[3],Arg[4],RVal))
					goto RtnTrue;  
			}
			else if (!_fstricmp (Arg[1],"RASTER"))
			{
				if (AreaInArea2 (Arg[2],Arg[3],atoi(Arg[4]),RVal,atob(Arg[6])))
					goto RtnTrue;  
			}
			goto RtnFalse;
		}
		
		case 1015: //$CREATEFILE(pathname)
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			Fid = GSSiOpenFile(Arg[1], 0, OF_CREATE);
			if (Fid == HFILE_ERROR) 
				goto RtnFalse;
			GSSiClose2 (&Fid);
          	goto RtnTrue;  
          	
		case 1016: // $WINTOWORLD(point,vpname)
		case 1017: // $WORLDTOWIN(point,vpname)
		{   
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			Point = atopt(Arg[1], &Err);
			SetCurView ( SetVPFromName (Arg[2],&Err)); 
			if (FunID == 1016) 
				Point2 = WinPtToBasePtD (&Point);
			else
				Point2 = BasePtToWinPtD (&Point);
			SetCurView ( SaveVP);
			sprintf (OutLoc,"%.14lg %.14lg",Point2.x,Point2.y);
			goto Rtnl;
		}   
		
        case 1018: //$ADJUSTPOLY(opt) 
        {
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (!stricmp (Arg[1],"BPEP"))
			{
				Refno = atol (Arg[2]);
				if (!PickByRefno (Refno,0,0,-101))
					goto RtnFalse;
				n = 5;
			}
			else if (!stricmp (Arg[1],"REDUCE"))
			{
				Dist = atof (Arg[2]);
				n = 6;
			}
			else if (!stricmp (Arg[1],"THIN"))
			{
				Dist = atof (Arg[2]);
				n = 7;
			}
			else if (!stricmp (Arg[1],"REMOVEDUP"))
			{
				Dist = atof (Arg[2]);
				n = 8;
			}
			else
        		n = atoi (Arg[1]); 
            if (AdjustPolygon(n,Arg[3],Dist))
            	goto RtnTrue;
            goto RtnFalse;
        }
        
        case 1019:	//$TABTOCOMMA(INFILE,OUTFILE)
        {
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs != 2)
				goto RtnFalse;
			TABToComma (Arg[1],Arg[2]);
			goto RtnTrue; 
        }
		case 1020: //$SELECTICON(iconlib)
		{   
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			goto Rtnl;
		}  
		 
        case 1021:	//$MOVECURSOR(world coord,vp(opt))
        {
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (*Arg[1]) 
			{
				Point = atopt (Arg[1],&Err);
				if (Err)
					goto RtnFalse;
			} 
			if (*Arg[2])
				SetCurView ( SetVPFromName (Arg[2],&Err)); 
			if (!*Arg[1]) 
			{
				if (CurView->LinkedCursorHandle && ScreenIsRegistered(CurView->LinkedCursorHandle,0))
				{
					LPSAVESCREEN pSaveScreen=(LPSAVESCREEN)GlobalLock (CurView->LinkedCursorHandle);
					
					Point16 = RectMid (&pSaveScreen->Rect); 
					GlobalUnlock (CurView->LinkedCursorHandle);
				}
				else 
				{
					Point = MinMaxMidPointD (&CurView->WBounds);
					if (!PointInWBounds (&Point))
						CenterWindow (Point,TRUE); 
				    Point16 = BasePtToScreenPt (&Point); 
				}
			}
			else
			    Point16 = BasePtToScreenPt (&Point); 
	        ClientToScreen (CurView->hWnd,(LPPOINT)&Point16);
		    SetCursorPosGM (Point16.x,Point16.y,0);
			SetCurView (SaveVP);
			goto RtnTrue; 
        }

        case 1022:	//$CURSORINVP(vpname)
        {
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs)
			{ 
				SetCurView ( SetVPFromName (Arg[1],&Err)); 
				if (Err)
					goto RtnFalse;
			}
			GetCursorPos (&Point16);
			ScreenToClient (CurView->hWnd,&Point16);
		    Point=ScreenPtToBasePt(Point16);
			rtn = PointInWBounds (&Point);
			SetCurView (SaveVP);
			
			if (rtn)
				goto RtnTrue;
			goto RtnFalse; 
        }
        
        case 1023: //$IDPOLYGONS()
        {
        	if (IDPolygons ())
        		goto RtnTrue;
        	goto RtnFalse;
        }
        
        case 1024:	//$FULLSCREEN(vpname)
        {
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			n = 2;
			if (nArgs)
			{ 
				SetCurView ( SetVPFromName (Arg[1],&Err)); 
				if (Err)
					goto RtnFalse;
			}
			if (nArgs == 2)
				n = atoi (Arg[2]);
   			MakeVPFullScreen (CurView->ID,n);
//			PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
			SetCurView (SaveVP);
			
			goto RtnTrue;
        }
        case 1025:	//$REFCONNECT(CHECK or FIX)
        {
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs) 
			{   
				if (!_fstricmp (Arg[1],"CHECK"))
				{
			        if (RefConnectTableCreate (&HLTBounds))
						goto RtnTrue;   
				}
				else if (!_fstricmp (Arg[1],"FIX"))
				{
					if (FixRefConnect ())
						goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[1],"OUTPUT"))
				{
					if (OutputRefConnect ())
						goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[1],"OUTPUTLINKS"))
				{
					if (OutputConnectedLinks (Arg[2]))
						goto RtnTrue; 
				}
				else if (!_fstricmp (Arg[1],"SHOWBADAREA"))
				{
					SetCurView ( SetVPFromName (Arg[2],&Err)); 
					if (DisplayBadArea ())
						goto RtnTrue; 
				}  

				else if (!_fstricmp (Arg[1],"HLT"))
				{   
					Refno = atol (Arg[2]);
					nlong = HltRefConnect (Refno,Arg[3],Arg[4]);
					ltoa (nlong,OutLoc,10); 
					goto Rtnl;
				}
			}
			goto RtnFalse;
        } 

        case 1026:	//$SCREENTOVP(screen coord,vpname(opt))
        {
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs > 1)
			{ 
				SetCurView ( SetVPFromName (Arg[2],&Err)); 
				if (Err)
					goto RtnFalse;
			}
			Point16 = atopt16(Arg[1],&Err);
			if (Err)
				goto RtnFalse; 
			Point = ScreenPointToVPPoint (Point16); 
			dpointtoa (OutLoc,&Point);
			SetCurView (SaveVP);
			
			goto Rtnl;
        }
        
        case 1027:	//$AREACENTER(Outfile,speedfac,usemask)
        {
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (GetAreaCenters (Arg[1],atoi(Arg[2]),atob(Arg[3])))
				goto RtnTrue;
			goto RtnFalse;
		}
		
        case 1028:	//$SERVERFILE(GET or SEND or DELETE,socket,serverfilename,localfilename,statusupdatecommand,completioncommand)
        {
			nArgs = GetFunArgs(Args, Arg, -6, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;  
			ExpandText (Arg[1]);
			ExpandText (Arg[2]);
			socket = atol (Arg[2]);
			ExpandText (Arg[3]);
			ExpandText (Arg[4]);
			if (ServerFile (Arg[1],socket,Arg[3],Arg[4],Arg[5],Arg[6]))
				goto RtnTrue;
			goto RtnFalse;
		}
		
        case 1029:	//$SPLITLINES(MaxPoints)
        {
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			if (SplitHighlightedPolys (atoi(Arg[1])))
				goto RtnTrue;
			goto RtnFalse;
		}
		
        case 1030:	//$INVERTRECT(rect,vp);
        {
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			Rect = atorect (Arg[1],&Err);
			SetCurView (SetVPFromName (Arg[2],&Err));
			SaveDC (CurView->hDC);
			SetDisplayMode (CurView->hDC, GF_SCREENMODE);  
			SelectClipRgn (CurView->hDC,0);
			TRANRect (&Rect,CurView->hTranBaseToVP); 
			TRANRect (&Rect,CurView->hTranVPToScreen); 
 			if (!InvertRect (CurView->hDC,&Rect))
				ii=1;
			RestoreDC (CurView->hDC,-1);
			CurView = SaveVP;

			goto RtnTrue;
		}

        case 1031:	//$SETMAPTIME(MapPath,MinTime,MaxTime);
        {
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;

			if (SetMapMinMaxTime (Arg[1],atol(Arg[2]),atol(Arg[3])))
				goto RtnTrue;
			goto RtnFalse;
		}
		
        case 1032:	//$GETMAPTIME(MapPath,MIN or MAX);
        {
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			nlong = GetMapTime (Arg[1],Arg[2]);
			itoa (nlong,OutLoc,10);
			goto Rtnl;
		}
		
        case 1033:	//$CHECKPOINT();
        {
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			CheckPointBegin ();
			goto RtnTrue;
		}
		
        case 1034:	//$PCTINAREAS(1,bounds); initializes function. Bounds is bounds of items to be tested. Returns handle to structure
				    //$PCTINAREAS(2,item,handle); picked item number of item to be tested. Can be called multiple times. Returns 1 if successful, 0 if not
					//$PCTINAREAS(3,item,handle,Offset,TreatAreasAsPolylines); picked item number of area or (polyline or point with offset). Can be called multiple times. Returns 1 if successful, 0 if not
					//$PCTINAREAS(4,handle); returns result as num between 0 and 1
					//$PCTINAREAS(5,handle); destroys handle
        {
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);

			PCTInAreaFunction (atoi(Arg[1]),Arg[2],Arg[3],OutLoc);
			goto Rtnl;
		}

		case 1035:	//$PROJECTION(DEFINE,projid,def)
					//$PROJECTION(DELETE,projid)
					//$PROJECTION(ROTATION,projid,point)
					//$PROJECTION(SCALE,projid,point)
					//$PROJECTION(CONVERT,point,fromid,toid)
					//$PROJECTION(CONVERSIONGRID,bounds,accuracy,outfile,projidfrm(opt),projidto(opt))
					//$PROJECTION(GUESS,fileOfProjections,bounds)
        {
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 

			ProjectionFunction (Arg[1],Arg[2],Arg[3],Arg[4],Arg[5],Arg[6],OutLoc);
			goto Rtnl;
		}
		case 1036: // $BACKGROUND(SET,ITEM,COLOR,VP)
				   // $BACKGROUND(CLEAR,VP)
 		{	
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (!_fstrcmp (Arg[1],"CLEAR")) 
			{   
				if (*Arg[2])
				{
					SetCurView ( SetVPFromName (Arg[2],&Err));  
					if (Err)
						goto RtnFalse;
				} 
				ClearBackgroundArea ();
				CurView = SaveVP; 
				goto RtnTrue;
			} 
			else if (!_fstrcmp (Arg[1],"SET")) 
			{       
				short	Item=0;
	 			short	opt=1;
				LPSTR	lpColon;
				
				if (*Arg[4])
				{
					SetCurView ( SetVPFromName (Arg[4],&Err));  
					if (Err)
						goto RtnFalse;
				} 
				if ((lpColon = _fstrchr (Arg[2],':')))
				{
					*lpColon++=0;
		            if (PickByRefno (0,Arg[2],lpColon,-1)) 
		            	Item = 1;  
		        }
				else 
					Item =atoi (Arg[2]);
				if (Item < 0)
					goto RtnFalse;
				if (!Item && BT_NUM_IN_INDEX (hHighlight) == 1)
				{
					pHighlightData = (LPHIGHLIGHTDATA)Arg[5];
    				BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)pHighlightData);
					PickList[0]=pHighlightData->PD;
					Item = 1;
				}
				if (SetBackgroundArea(Item-1,atoi(Arg[3])))
					goto RtnTrue;
			} 
			goto RtnFalse;
		}
		
		case 1037: // $WAITFORKEY(useGetMessage)
		{
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = WaitForKeystroke(atob(Arg[1]));
			OutLoc[1] = 0;
			goto Rtnl;
		}

		case 1038:  //$DIALOGITEM(hWndDlg,item,GETTEXT,maxlen)
					//$DIALOGITEM(hWndDlg,item,SETTEXT,value)
		{
			HWND hWndDlg;
			UINT item;
			int  maxlen;

			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			hWndDlg = (HWND)atoi(Arg[1]);
			item = (UINT)atoi(Arg[2]);
			if (!stricmp(Arg[3], "GETTEXT"))
			{ 
				maxlen = atoi(Arg[4]);
				GetDlgItemText(hWndDlg, item, OutLoc, maxlen);
				goto Rtnl;
			}
			else if (!stricmp(Arg[3], "SETTEXT"))
			{
				if (SetDlgItemText(hWndDlg, item, Arg[4]))
					goto RtnTrue;
			}
			goto RtnFalse;
		}

		case 1039:	//$SYMATTRKEY(SYMNAME,override control global(opt),new dir,reference file,addrefno)
		{
			BOOL	AddRef;

			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			AddRef = atob(Arg[4]);
			GetSymAttrKey(Arg[1], Arg[3], AddRef, Arg[5], Arg[2], OutLoc);
			goto Rtnl;
		}

		case 1040:	//$PARCELTRAN
		{
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			ParcelTranFunction(nArgs, Arg, OutLoc);
			goto Rtnl;
		}
		case 1041:	//$MAILLABELS
		{
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			PrintMailLabels (nArgs, Arg, OutLoc);
			goto Rtnl;
		}
		case 1042://$SAVESCREEN(TOHANDLE)
			//$SAVESCREEN(TOFILE,hDIB,path)
			//$SAVESCREEN(UNLOAD,hDIB)
		{
			HBITMAP	hBitmap;
			HDIB32 hDib32;
			RECT	ScreenRect;
			HWND	hWnd = GetDesktopWindow();
			HDC		hDC;

			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (!stricmp(Arg[1], "TOHANDLE"))
			{
				hDC = GetWindowDC(hWnd);
				//SaveDC(hDC);
				//SetDisplayMode(hDC, GF_TEXTMODE);
				GetClientRect(hWndMain, &ScreenRect);
				ClientRectToScreenRect(hWndMain, &ScreenRect);
				hBitmap = SaveScreen(hDC, ScreenRect);
				hDib32 = BitmapToDIB32(hBitmap);
				ltoa((long)hDib32, OutLoc, 10);
				DeleteObject(hBitmap);
				//RestoreDC(hDC,-1);
				ReleaseDC(hWnd, hDC);
				goto Rtnl;
			}
			if (!stricmp(Arg[1], "TOFILE"))
			{
				HDIB32 hDib24;
				hDib32 = (HDIB32)atol(Arg[2]);
				hDib24 = FreeImage_ConvertTo24Bits(hDib32);
				rtn = GM32SaveDIB(hDib24, Arg[3], -1, 0);
				FreeImage_Unload(hDib24);
				goto Rtnrtn;
			}
			if (!stricmp(Arg[1], "UNLOAD"))
			{
				hDib32 = (HDIB32)atol(Arg[2]);
				FreeImage_Unload(hDib32);
				goto RtnTrue;
			}
			

		}

		case 1043:	//$GMDOCUMENT
		{
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			GMDocument(nArgs, Arg, OutLoc);
			goto Rtnl;
		}

		case 1101: //$DUMPGLOBALS(pathname)
			dumpvars (Args);
          	goto RtnTrue;  
          	
        case 1102: //$LAYERBOUNDS (layerid) <100 actual layer, > layer search for layerid, 0=visbounds
        {   
        	short	Layer;
        	BOOL	rtn; 
        	HANDLE	hSaveVis = GSSiGlobAlloc (1194,GMEM_MOVEABLE,sizeof(VISLIST));
        	LPVISLIST	pSaveVis = (LPVISLIST)GlobalLock (hSaveVis);
        	
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			Layer = atol(Arg[1]);
			*pSaveVis = *CurVis; 
			if (Layer < 0)
			{
			}
			else
			if (Layer <= CurView->NumFiles)
			{
				_fmemset (CurVis->FileIsVisible,0,sizeof(CurVis->FileIsVisible));
				CurVis->FileIsVisible[Layer-1]=TRUE;
			}
			else
			{
			}
			rtn = GetVisBounds (&Bounds,CurView->hDC);
			*CurVis = *pSaveVis;
			GSSiGlobUlFree (&hSaveVis);
			if (!rtn)
				goto RtnFalse; 
			sprintf (OutLoc,"%f %f %f %f",Bounds.xmn,Bounds.ymn,Bounds.xmx,Bounds.ymx);
			l = _fstrlen (OutLoc);  
		    goto Rtnl;
		}
		
        case 1103: //$FIXGMERRORS(file)
        {   
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			FixGMErrors(Arg[1]);
		    goto Rtnl;
		} 
		
		case 1104: //$ADDWAYPOINT ()
        {
        	BOOL	nRc;
            
            setDoPaint( FALSE);      
			lpfnADDWAYPOINTMsgProc = MakeProcInstance((DLGPROC)ADDWAYPOINTMsgProc, hInst);
			nRc = DialogBox(hInst, (LPSTR)"ADDWAYPOINT",hWndMain, lpfnADDWAYPOINTMsgProc);
			FreeProcInstance(lpfnADDWAYPOINTMsgProc); 
			setDoPaint( TRUE);
            if (nRc)
            	goto RtnTrue;
            else
				goto RtnFalse;
        }
		
		case 1105: //$COMPILETRAN(tranfile)
		{	
			HANDLE	hTran; 
			long	Tran2Offset;    
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			GSSiRemove(Arg[2]);
			if ((hTran=LoadTranFile (Arg[1],1,3,0,0)))
			{   
				Fid = GSSiOpenFile (Arg[2],0,OF_CREATE);
				WriteTranData (Fid,hTran); 
				Tran2Offset = GSSillseek (Fid,0,2);
				CloseTRANS2 (&hTran);  
				hTran=LoadTranFile(Arg[1],2,3,0,0);
				WriteTranData (Fid,hTran);
				CloseTRANS2 (&hTran); 
				BigWrite (Fid,(HPSTR)&Tran2Offset,4,-1);
				GSSiClose2 (&Fid);
				goto RtnTrue;
			}
			else
				goto RtnFalse;
		} 

		
        case 1106: //$GPSADDPOINT(waypoint)
        {   
        	long	WayPointID;
        	
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			Point.x = atof (Arg[3]);
			Point.y = atof (Arg[4]);
			WayPointID = atol (Arg[1]); 
			if (WayPointID)
				rtn = AddWPToGPSList (Arg[1],WayPointID,Arg[2],&Point);
			else 
			{
				WayPointID = atol (Arg[2]);
				rtn = AddWPToGPSList (Arg[1],WayPointID,"",&Point);
			}
			if (rtn)
		    	goto RtnTrue;
		    else
		    	goto RtnFalse;
		}   
		
		case 1107: // $UPDATEGRCMD(Varname,VarValue,SavedFileID(optional))
		{	 
			
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			n = atoi(Arg[3]);
			if (UpdateCmdStringInFile (Arg[1],Arg[2],n))
              	goto RtnTrue;
            else
				goto RtnFalse;
		} 
		
		case 1108: //$RUNTEXTFILE (pathname!statuswindtitle!statuslooptext#maxloops,command)   or
				   //$RUNTEXTFILE (pathname(lineno),command) runs only single line    or
				   //$RUNTEXTFILE (VARNAME=pathname,command) reads textfile without field header line, sets entire line to VARNAME
        {   
        	LPSTR pStatusText, pLoopText=0, pLineNo, pFileName, pVarName=0;
        	long	ProcessLine = -1, AtLine=0;
			static	int istatus=0;
			LPSTR	pMax;
			int		maxLinesToProcess = -1;
			int		nLinesProcessed = 0;

			if (!(ParLoc = MatchLev (Args,','))) goto Rtn0;
			hMem = GSSiGlobAlloc(1200, GMEM_MOVEABLE, MAXARGLENGTH * 2 + 4096 + 1024);
			Arg1 = GlobalLock(hMem);
			Arg2 = Arg1 + 1024;
			Arg3 = Arg2 + MAXARGLENGTH;
			Arg4 = Arg3 + 4096;
			_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
			*ParLoc = '\0';
			_fstrcpy (Arg1,Args);
			if ((pMax = strrchr(Arg1, '#')))
			{
				*pMax++ = 0;
				maxLinesToProcess = atoi(pMax);
			}
			ExpandTextDB(Arg1,pBrkPt, bpOffset, bpLen);
			bpOffset += strlen(Args) + 1;
			if (!(pEnd = strchr (Arg1,'.')))
				pEnd = Arg1;
            if ((pStatusText = _fstrchr (pEnd,'!')))
            	*pStatusText++ = 0;   
            if ((pLineNo = _fstrchr (pEnd,'(')))
            {
            	*pLineNo++ = 0;    
            	ProcessLine =  atol (pLineNo);
            } 
            pFileName = _fstrchr (Arg1,'=');
            if (pFileName) 
            {
            	pVarName = Arg1;
            	*pFileName++ = 0; 
            }
            else
            	pFileName = Arg1;
			Fid = GSSiOpenFile (pFileName,0,OF_READ);
			if (Fid == HFILE_ERROR)
	           	goto RtnFalse;
			if (pStatusText)
			{   
				if ((pLoopText = _fstrrchr (pStatusText,'!')))
					*pLoopText++ = 0;
		        nlong = GSSillseek (Fid,0,2);
		        GSSillseek (Fid,0,0); 
				if (istatus < 2)
					CreateStatusWind (hWndMain,1,pStatusText);
				istatus++;
		    } 
			IgnoreSelectVP = TRUE;    
			CurLoc = 0;
		    fgetstring (Arg3,4090,Fid);
			SetGlobalValue("%TEXTFILEHEADER", Arg3);
			if (!pVarName)
				while (*LastChr(Arg3) == ';')
				{
					ExpandText (Arg3);
					CurLoc = GSSillseek (Fid,0,1);  
					fgetstring (Arg3,4090,Fid);
				}
			GSSillseek (Fid,CurLoc,0);
		    if (!pVarName)
				ProcessDelimTextHeader(Arg3, pFileName, Fid, &hDLT, 0, 0);
		    else
		    	hDLT = 0;
			while (ContinueProcessing  && nLinesProcessed != maxLinesToProcess && fgetstring(Arg3, 4090, Fid))
			{ 
				nLinesProcessed++;
				SetGlobalValue("%TEXTFILELINE", Arg3);

				if (ProcessLine < 0 || AtLine == ProcessLine)
				{
			    	if (pVarName)
			    		SetGlobalValue3 (pVarName,Arg3,0,FALSE);
			    	else
			    		GetDelimTextData(Arg3,hDLT,4090); 
			    	_fstrcpy (Arg4,Arg2);
		            if (pStatusText)
					{
						int	rc=1;

						switch (istatus)
						{
						case 2:
						case 1:
							rc = StatusWindowUpdate (0,pLoopText,nlong,CurLoc);
							break;
						default:
							break;
						}
						if (!rc)
							break;
					}
					ExpandTextDB(Arg4, pBrkPt, bpOffset, bpLen);
					if (ProcessLine >= 0)
						break; 
					CurLoc = GSSillseek (Fid,0,1);  
				}
				AtLine++;
            }
            IgnoreSelectVP = FALSE;
            rtn = ContinueProcessing;
            SetContinueProcessing ( TRUE);
			GSSiGlobFree (&hDLT);
            GSSiClose2 (&Fid);
            if (pStatusText)
				switch (istatus)
			{
				case 0:
					break;
				case 1:
				case 2:
					DestroyStatusWindow(0);  
				default:
					istatus--;
					break;
			}
            if (rtn)
            	goto RtnTrue;
            goto RtnFalse;
		}
		
		case 1109: //$GETTRANDATA(tranfile) gets tranfile data from highlight list and editlimits
				   //$GETTRANDATA(tranfile,R (rotation of line fitted through points as az),A (azimuth) D (degrees) or B (bounds),type(1 2 or 3),F or R) gets tranfile rotation or bounds
		{	
			short	from=1,to=2, Type;
			HANDLE	hTrans;
			
			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (nArgs == 1)
			{
				pHighlightData = (LPHIGHLIGHTDATA)Arg[5];
				str = Arg[6];
				if (!GetLayerBounds (&Bounds,CurView->hDC, CurView->UpdateFile-1))
					goto RtnFalse;
				Fid = GSSiOpenFile (Arg[1],NULL,OF_CREATE); 
				if (Fid == HFILE_ERROR)
					goto RtnFalse;
				sprintf (str,"L %.14lg %.14lg %.14lg %.14lg",Bounds.xmn,Bounds.ymn,Bounds.xmx,Bounds.ymx);
				fputstring (str,Fid);				
				sprintf (str,"%.14lg %.14lg %.14lg %.14lg",Bounds.xmn,Bounds.ymn,Bounds.xmn,Bounds.ymn);
				fputstring (str,Fid);				
				sprintf (str,"%.14lg %.14lg %.14lg %.14lg",Bounds.xmn,Bounds.ymx,Bounds.xmn,Bounds.ymx);
				fputstring (str,Fid);				
				sprintf (str,"%.14lg %.14lg %.14lg %.14lg",Bounds.xmx,Bounds.ymx,Bounds.xmx,Bounds.ymx);
				fputstring (str,Fid);				
				sprintf (str,"%.14lg %.14lg %.14lg %.14lg",Bounds.xmx,Bounds.ymn,Bounds.xmx,Bounds.ymn);
				fputstring (str,Fid);				
				pos = BT_FIRST;
				while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)pHighlightData))
				{
					pos = BT_NEXT;
					switch (pHighlightData->PD.Type)
					{
						case 1:
							Point = Point2 = pHighlightData->PD.BeginPoint;
							break;
						case 2:
							Point = pHighlightData->PD.BeginPoint;
							Point2 = pHighlightData->PD.EndPoint;
							break;
						default:
							goto NextTranPoint;
							
					}  
					sprintf (str,"%.14lg %.14lg %.14lg %.14lg",Point.x,Point.y,Point2.x,Point2.y);
					fputstring (str,Fid);				
	NextTranPoint:;
				}
				GSSiClose2 (&Fid);
				goto RtnTrue;
			}

			Type = atoi (Arg[3]);
			if (Type < 1 || Type > 3)
				Type = 1;
			if (*Arg[4] == 'R')
			{
				from = 2;
				to = 1;
			}
			hTrans = LoadTranFile(Arg[1],from,Type,0,0);
			if (!hTrans)
				goto RtnFalse;
			if (*Arg[2] == 'R')
			{
				LPTRANDATA TranPtr=GlobalLock (hTrans);

				if (from == 1)
					ftoa (OutLoc,TranPtr->FitPointAZFrom);
				else
					ftoa (OutLoc,TranPtr->FitPointAZTo);
				GlobalUnlock (hTrans);
			}
			if (*Arg[2] == 'B')
			{
				Bounds = GetTranFileBounds (Arg[1],Arg[4]);
				boundstoa (OutLoc,&Bounds); 
			}
			else
			{
				AZ = GetTranAZ (hTrans);
				if (*Arg[2] == 'D')
					ftoa (OutLoc,AZ * DEGRAD);
				else if (*Arg[2] == 'A')
					ftoa (OutLoc,AZ);
			}
			CloseTRANS2 (&hTrans);
			goto Rtnl;
		} 
		
		case 1110: // $GETNEXTLINE(longvar,outvarname,positionvarname,maxline)
		{	 
			HANDLE	hGlobal;
			
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			str = Arg[5];
            n = _fstrlen (Arg[1]); 
    		hGlobal = FindVar(Arg[3]);	
			GetGlobalVal (hGlobal,str,0); 
			pos = atoi (str);
			if (pos < n)
			{   
				if (Arg[1][pos] == '\n')
					pos++;
				nrem = _fstrlen (&Arg[1][pos]);
				if (!nrem)
					goto RtnFalse;
				l = atoi (Arg[4]);
				if (l <= 0)
					goto RtnFalse;
				nrem = min (l,nrem);
				pEnd = &Arg[1][pos] + nrem;
				*pEnd = 0;
				if ((pCR = _fstrchr (&Arg[1][pos],'\r')))
				{
					*pCR++ = ' ';
					*pCR = 0;  
					pEnd = pCR;
				}
				if (nrem >= l && _fstrrchr (&Arg[1][pos],' ')>&Arg[1][pos])
				{
					while (*pEnd != ' ')
					{
						pEnd--;
					} 
					*pEnd = 0;
				} 
				SetGlobalValue(Arg[2],&Arg[1][pos]); 
				if (*(LPSTR)(pEnd+1) == '\n')
					pEnd++;
				nlong = (long)pEnd - (long)Arg[1] + 1;
				SetGlobalValueLong (Arg[3],nlong);
              	goto RtnTrue;
            }
            else  
            {
				SetGlobalValue(Arg[2],""); 
				goto RtnFalse; 
			}
		} 
		
		case 1111: //$SELECTITEMS(ToFile,FromFile,Append or Create,Prompt,return full line(optdefF),menu2option(defF)
		{   
			DLGPROC lpfnSELECTITEMSMsgProc;
			
			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 4)
				goto RtnFalse;
			hSelectItemsArgs = GSSiGlobAlloc (1203,GHND,1024);
			Arg5 = GlobalLock (hSelectItemsArgs);   
			*Arg5 = *Arg[3]; 
			Arg5[1] = *Arg[5];
			_fstrcpy (&Arg5[2],Arg[2]);
			_fstrcpy (&Arg5[512],Arg[1]);
			_fstrcpy (&Arg5[850],Arg[4]);
			GlobalUnlock (hSelectItemsArgs);
            setDoPaint( FALSE);  
            if (atob(Arg[6]))
            {    
				lpfnSELECTITEMSMsgProc = MakeProcInstance((DLGPROC)SELECTITEMSMsgProc2, hInst);
				rtn = DialogBox(hInst, (LPSTR)"SELECTITEMS2",hWndMain, lpfnSELECTITEMSMsgProc);
				FreeProcInstance(lpfnSELECTITEMSMsgProc); 
			} 
			else
            {    
				lpfnSELECTITEMSMsgProc = MakeProcInstance((DLGPROC)SELECTITEMSMsgProc, hInst);
				rtn = DialogBox(hInst, (LPSTR)"SELECTITEMS",hWndMain, lpfnSELECTITEMSMsgProc);
				FreeProcInstance(lpfnSELECTITEMSMsgProc); 
			}
			setDoPaint( TRUE);                              
			GSSiGlobFree (&hSelectItemsArgs);
			itoa (rtn,OutLoc,10);
			goto Rtnl;
        }  
			
		case 1112: //$GPSTRACKING(ONorOFForTOGGLE)
		{   
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)  
			{
				if (GPSTracking (-1))
					goto RtnTrue;
				goto RtnFalse;
			}
			n = atoi (Arg[1]);
			GPSTracking (n);
			goto RtnTrue;
		}
		
		case 1113: //$FONTDISPLAY(FontName)
		{   
			
			hMem = GSSiGlobAlloc (1205,GMEM_MOVEABLE,4*2048);
			Arg1 = GlobalLock(hMem); 
			_fstrcpy (Arg1,Args);       
			ExpandText(Arg1);
			FontDisplay (Arg1);
			goto RtnTrue;
		}
		
		case 1114: // $WETLANDDESC(wetsymname,Compressed COW)
		{   
			if (!(ParLoc = MatchLev (Args,','))) goto Rtn0;
			hMem = GSSiGlobAlloc (1206,GMEM_MOVEABLE,2*2048);
			Arg1 = GlobalLock(hMem);
			Arg2 = Arg1 + 2048; 
			
			_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
			*ParLoc = '\0';
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			ExpandText (Arg2); 
			n = GetWetlandDescription (Arg1,Arg2,OutLoc);    
		    goto Rtnl;
		}

        case 1115: //$GPSADDROUTE(waypoint)
        {   
        	long	RouteRef;
        	
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			RouteRef = atol (Arg[1]); 
			rtn = AddRouteToGPSList (RouteRef);
			if (rtn)
		    	goto RtnTrue;
		    else
		    	goto RtnFalse;
		}  
		
		case 1116:  // $BLOCKEDREFS(OPEN) 
					// $BLOCKEDREFS(CLOSE,outpath)
		{	 
			hMem = GSSiGlobAlloc (1208,GMEM_MOVEABLE,2*2048);
			Arg1 = GlobalLock(hMem);
			Arg2 = Arg1 + 2048; 
			
			if ((ParLoc = MatchLev (Args,','))) 
			{
				_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
				*ParLoc = '\0';
			}
			else
				*Arg2 = 0;
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			ExpandText (Arg2);
			if (!_fstricmp (Arg1,"OPEN"))
			{
				if (!hBlockedRefFile)                                   
				{
					hBlockedRefFile = GSSiGlobAlloc (1209,GMEM_MOVEABLE,256);
		    		pFile = GlobalLock (hBlockedRefFile);
		        	GSSiGetTempFileName (0,"gm",0,pFile);  
		        }
		        else
		        	pFile = GlobalLock (hBlockedRefFile);
				if (FidBlockedRefs != HFILE_ERROR)
					GSSiClose2 (&FidBlockedRefs);  
		        FidBlockedRefs = GSSiOpenFile (pFile,0,OF_CREATE);
				GlobalUnlock (hBlockedRefFile); 
				goto RtnTrue;
			}   
			else if (!_fstricmp (Arg1,"CLOSE") && FidBlockedRefs != HFILE_ERROR)     
			{ 
				if (ProcessBlockedRefsFile (Arg2))
					goto RtnTrue;  
			} 
			else if (!_fstricmp (Arg1,"FIX"))     
			{ 
				if (FixBlockedRefs (Arg2))
					goto RtnTrue;  
			} 

			goto RtnFalse;
        }
		
		case 1117: //$LOADNEWDATA(dir)
		{   
			
			hMem = GSSiGlobAlloc (1210,GMEM_MOVEABLE,4*2048);
			Arg1 = GlobalLock(hMem); 
			_fstrcpy (Arg1,Args);       
			ExpandText(Arg1);
			if (LoadNewData (Arg1))
				goto RtnTrue;  
			goto RtnFalse;
		}
		
		case 1118: //$THEMEINAREA(BoundaryArea,Theme,GMDfile,MinPCT,Precision)
		{
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 5)
				goto RtnFalse;
			RVal = atof (Arg[4]); 
			nlong = atol (Arg[5]);
			if (ThemeInArea (Arg[1],Arg[2],Arg[3],RVal,nlong))
				goto RtnTrue;
			goto RtnFalse;
		}
		
		case 1119: //$SETREDEFINE()
		{   
			
			hMem = GSSiGlobAlloc (1212,GMEM_MOVEABLE,sizeof(HIGHLIGHTDATA)); 
			pHighlightData = (LPHIGHLIGHTDATA)GlobalLock (hMem);
			if (!BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)pHighlightData))
				SetRedefineData (&pHighlightData->PD);  
			else
				SetRedefineData (0);  
			goto RtnTrue;
		} 
		
		case 1120: //$GROUPPOINTS(InFile,SQL,xfieldname,yfieldname,keyfieldname,MinDist,MaxDist,MaxPoints,
				   //			  BoundsFile,PointToBoundsFile(gmd-opt))
		{	
			double	GroupDist;
				
			nArgs = GetFunArgs(Args, Arg, 12, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 10)
				goto RtnFalse;
			Dist = atobasedist (Arg[7],&Err);
			if (Err)
				goto RtnFalse;   
			Dist2 = atobasedist (Arg[8],&Err);
			if (Err)
				goto RtnFalse;
			GroupDist = atobasedist (Arg[9],&Err);
			if (Err)
				goto RtnFalse;
			nlong = atol (Arg[10]);   
			nlong = GroupPoints (Arg[1],Arg[2],Arg[3],Arg[4],Arg[5],Arg[6],Dist,Dist2,GroupDist,nlong,Arg[11],Arg[12]);
			ltoa (nlong,OutLoc,10);
			goto Rtnl;
		}
		
		case 1121: //$GETTEMPFILE(prefix,suffix)
		{	
			LPSTR	pSuf;	
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (!*Arg[1])
				_fstrcpy (Arg[1],"gm"); 
			pSuf = Arg[2];
			if (*pSuf == '.')
				pSuf++;
        	GSSiGetTempFileName (0,Arg[1],0,OutLoc); 
        	if (*Arg[2] && ((pEnd=_fstrrchr (OutLoc,'.'))))
        		_fstrcpy (++pEnd,pSuf);
			goto Rtnl; 
		}
		
		case 1122: // $GMDADDINDEX(pathname,keyfield1;keyfield2,Unique(T/F)or spatial Type,SpatialBounds) 
		{	 
			HANDLE	hKeyFields;
			int		IndexType;
			MNMXCORD	Bounds;
			
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (!GetFieldIDsFromNames (Arg[1],&hKeyFields,0,Arg[2],0))
				goto RtnFalse;
			if (IsInteger(Arg[3]))
				IndexType = atoi (Arg[3]);
			else
				IndexType = atob(Arg[3]);
			Bounds = atobounds (Arg[4],&Err);
			rtn = GWDAddIndex (Arg[1],hKeyFields,IndexType,&Bounds);
			GSSiGlobFree (&hKeyFields);
			if (rtn) 
				goto RtnTrue; 
			else
            	goto RtnFalse;
			
		}  
		break;  
		
		case 1123: // $ADDFALSEINT() 
		{	 
			HANDLE	hKeyFields;
			
			nArgs = GetFunArgs (Args,Arg,1,&hMem, pBrkPt, bpOffset, bpLen);
			rtn = AddFalseIntersection ();
			if (rtn) 
				goto RtnTrue; 
			else
            	goto RtnFalse;
			
		}  
		break;  

		case 1124: // $DUMPDGNSYMS(dgnfile,dumpfile) 
		{	 
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			DumpDGNSyms (Arg[1],Arg[2]);
			goto RtnTrue; 
		}  
		break;  
        
		case 1125: // $SCREENCACHE(CLEAR) 
		{	 
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			DeleteCacheDir ();
			RemoveVPBitmaps ();
			goto RtnTrue; 
		}  
		break;  
        
		case 1126: // $VIRTUALPLOT(Display,name) 
				   // $VIRTUALPLOT(SAVE,vpindex,imagepathname)
		{	 
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (!_fstricmp (Arg[1],"DISPLAY"))
				rtn = DisplayVirtualPlot (Arg[2]);
			if (!_fstricmp (Arg[1],"SAVE"))
				rtn = VirtualPlotToImage (Arg[3],Arg[2],0);
			goto Rtnrtn; 
		}  
		break;  
        
		case 1127: // $GMDADDFIELD(DBName,FieldName,FieldType)
		{	
			GWFLDINFO	FieldInfo;
			 
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
       		_fstrncpy (FieldInfo.Name,Arg[2],sizeof(FieldInfo.Name));
	 		if (GetFieldTypeAndLenFromChar (Arg[3],&FieldInfo,0))
				if (GWDAddField (Arg[1],&FieldInfo))
					goto RtnTrue;
			goto RtnFalse;
        }
        break;

		case 1128: // $GMDCOPYFILE(ToFile,FromFile,SQL)
		{	
			GWFLDINFO	FieldInfo;
			 
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (GMDCopyFile (Arg[1],Arg[2],Arg[3]))
				goto RtnTrue;
			goto RtnFalse;
        }
        break;
        
		case 1129: //$TAGREFINDEX(CREATE,fileorindex)
				   //			  LOADADDITIONAL,fileorindex,openfileid,prefix,newudi_var,oldudi_var)
		{
			HANDLE	hViewport; 
			LPVIEWPORT	pViewport;
			                  
			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse; 
			if (!stricmp (Arg[1],"LOADADDITIONAL"))
			{
				nRc = AddAdditionalUDI (Arg[2],Arg[3],Arg[4],Arg[6],Arg[5]);
				itoa(nRc, OutLoc,10);
				goto Rtnl;
			}
			else if (!stricmp (Arg[1],"CREATE"))
			{
				pViewport = CurView = CreateNullViewport (&hViewport,"",0,0,0);  
				if ((nRc = AddLayerToViewport (pViewport,"",Arg[2])))
	           		nRc = BuildRefIndexes (TRUE);  
				CurView = SaveVP;
				GlobalUnlock (hViewport);
           		DestroyViewport (&hViewport);
				if (nRc)
					goto RtnTrue;
			}
			goto RtnFalse;
		}
		
        case 1130:	//$INSERTLINES(BEGIN,FILE)
        {
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;   
			rtn = Insert (Arg[1],Arg[2],Arg[3]);
			itoa (rtn,OutLoc,10);
			goto Rtnl;
		}
        case 1131:	//$ROTATEIMAGE(ImageFilePath,DegreesRotation)
        {
			HDIB32 hDib32In, hDib32Out;

			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse; 
			hDib32In = BMPHandleFromEXT (Arg[1]);
			if (!hDib32In)
				goto RtnFalse;
			hDib32Out = GMRotateImageClassic (hDib32In,atof(Arg[2]));
			GMDestroyDIB32 (hDib32In);
			if (!hDib32Out)
				goto RtnFalse;
			if (GMFIBMPHandleToEXT (Arg[1],hDib32Out,0))
			{
				AddBMPToCache (0,0);
				AddBMPToCache32 (0,0);
				CloseAllRequestedFiles(FALSE); 
				goto RtnTrue;
			}
			goto RtnFalse;
		}

        case 1132:	//$SYMATTRFILE(SYMNAME,override control global(opt),new dir,reference file,addrefno)
        {
			BOOL	AddRef;

			nArgs = GetFunArgs (Args,Arg,5,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse; 
			AddRef = atob (Arg[4]);
			GetSymAttrFile (Arg[1],Arg[3],AddRef,Arg[5],Arg[2],OutLoc);
			goto Rtnl;
		}

        case 1133:	//$POINTINAREA(LOAD,)
					//$POINTINAREA(TEST,Point)
					//$POINTINAREA(DESTROY)
        {
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse; 
			if (PointInAreaFunctions (nArgs,Arg,OutLoc))
				goto Rtnl;
			goto RtnFalse;
		}

		case 1134:// $SCREENCOLOR(GET,basept,vpname)
				  // $SCREENCOLOR(SET,basept,color,vpname)

			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse; 
			if (!stricmp (Arg[1],"GET"))
			{
				*OutLoc = 0;
				SetCurView (SetVPFromName (Arg[3],&Err));
				Point = atopt(Arg[2],&Err);
				if (!Err)
				{
					SaveDC (CurView->hDC);
					SetDisplayMode (CurView->hDC,GF_SCREENMODE);
					Point16 = BasePtToScreenPt (&Point);
					if (PtInRect (&CurView->ScreenRect,Point16))
					{
						COLORREF Color = GetPixel (CurView->hDC,Point16.x,Point16.y);

						ltoa (Color,OutLoc,10);
					}
					RestoreDC (CurView->hDC,-1);
				}
				goto Rtnl;
			}

			goto RtnFalse; 
		case 1135:// $INTERTWINED(SET,x,y,maxchar) also works with $INTERLEAVED
				  // $INTERTWINED(GETX,intertwinedcoord)
				  // $INTERTWINED(GETY,intertwinedcoord)
			{
				int ix,iy;

				nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
				if (nArgs < 2)
					goto RtnFalse; 
				if (!stricmp (Arg[1],"SET"))
				{
					if (!CreateInterleavedCoord (atof(Arg[2]),atof(Arg[3]),atoi(Arg[4]), OutLoc))
						goto RtnFalse;
				}
				else if (!stricmp (Arg[1],"GETX"))
				{
					ExtractInterleavedCoord (Arg[2],&ix,&iy);
					itoa (ix,OutLoc,10);
				}
				else if (!stricmp (Arg[1],"GETY"))
				{
					ExtractInterleavedCoord (Arg[2],&ix,&iy);
					itoa (iy,OutLoc,10);
				}
				goto Rtnl;
			}

		case 1136:// $STRINGTONUM(string,maxnum(opt))
			{
				unsigned char *pLoc;
				int	nMax;

				nlong = 0;

				nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
				nMax = atoi (Arg[2]);

				pLoc = (unsigned char *)Arg[1];
				n = strlen (Arg[1]);
				pLoc += n-1;

				while (n--)
					nlong += *pLoc--;

				if (nMax)
					nlong %= nMax;
				itoa (nlong,OutLoc,10);
				goto Rtnl;
			}

		case 1137://$DATADISPLAY(BASIC,file name)
		{
				LPSTR	pName;

				nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
				hName = GSSiGlobAlloc(17, GHND, 1024);
				pName = GlobalLock(hName);
				strcpy(pName, Arg[2]);
				GlobalUnlock(hName);
				CreateDialog(hInst, (LPSTR)"DISPLAY_GWD_DATA", hWndMain, (DLGPROC)DISPLAY_GWD_DATAMsgProc);
				goto RtnTrue;
		}
			break;

		case 1138://$ISLOCALFILE(File)
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (IsLocalFile(Arg[1]))
				goto RtnTrue;
			goto RtnFalse;
		}

		case 1139://$FILEMANAGER(file)
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			GMFileManager(Arg[1]);
			goto RtnTrue;
		}
		case 1140://$SYNCHRONIZE()
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 0)
				goto RtnFalse;
			CallSynchronizeMsgProc();
			goto RtnTrue;
		}
		case 1201: //$FINDWAYPOINT ()
        {
            setDoPaint( FALSE);      
			lpfnFINDWAYPOINTMsgProc = MakeProcInstance((DLGPROC)FINDWAYPOINTMsgProc, hInst);
			rtn = DialogBox(hInst, (LPSTR)"FINDWAYPOINT",hWndMain, lpfnFINDWAYPOINTMsgProc);
			FreeProcInstance(lpfnFINDWAYPOINTMsgProc); 
			setDoPaint( TRUE);
            if (rtn)
            	goto RtnTrue;
            else
				goto RtnFalse;
        }  
        
        case 1202: //$SETDATERANGE ()
        {
	          DLGPROC lpfnDATELIMITSMsgProc;
	
	          lpfnDATELIMITSMsgProc = MakeProcInstance((DLGPROC)DATELIMITSMsgProc, hInst);
	          rtn = DialogBox(hInst, (LPSTR)"DATELIMITS", hWndMain, lpfnDATELIMITSMsgProc);
	          FreeProcInstance(lpfnDATELIMITSMsgProc);  
	          if (rtn)
	          	goto RtnTrue;
	          else
	          	goto RtnFalse;
         }

        case 1203: //$LOADUMSYMDEF ()
        {   
        	n = 1;
		    CloseSymDict(); 
		    if (*Args == 'D' || *Args == 'd')
		    	OpenSymDict (OF_DELETE); 
		    else if (*Args == 'B' || *Args == 'b') 
		    	n = 0;
            createsymbols (n);
         	goto RtnTrue;
        }

        case 1204: //$MAKEQUANFILE (pathname)
        {   
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (*Arg[2])
		    	TORF = atob (Arg[2]);    
		    else
		    	TORF = TRUE;
			if (CreateQuantitiesFile (Arg[1],TORF,Arg[3]))
				goto RtnTrue;
			else
				goto RtnFalse;
        }
        
        case 1205: //$USEDREFTABLE(CREATE,REPLACE(YORN))
        		   //$USEDREFTABLE(LOAD,HLT)
        		   //$USEDREFTABLE(CHECK,HLT)
        {
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			if (URTCommands (nArgs,Arg))
				goto RtnTrue;
			goto RtnFalse;
        }
        
        case 1206: //$ADDRECTANGLE (bounds,display(YORN))
        {   
        	HANDLE		hPnts;
        	HPDPOINT	Points;
        	
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			Bounds = atobounds (Arg[1],&Err);
			if (Err)
				goto RtnFalse;  
			hPnts = GSSiGlobAlloc (1214,GMEM_MOVEABLE,4 * sizeof(DPOINT));  
			Points = (HPDPOINT)GlobalLock (hPnts);
			Points[0].x = Bounds.xmn;
			Points[0].y = Bounds.ymn;
			Points[1].x = Bounds.xmn;
			Points[1].y = Bounds.ymx;
			Points[2].x = Bounds.xmx;
			Points[2].y = Bounds.ymx;
			Points[3].x = Bounds.xmx;
			Points[3].y = Bounds.ymn;
			GlobalUnlock (hPnts); 
			n = AddNewPoly (0,4,hPnts,atob(Arg[2]));
			GSSiGlobFree (&hPnts);
			if (n > 0)
				goto RtnTrue;
			goto RtnFalse;
        }
        
        case 1207: //$UPDATEGLOBAL(file,varname,varvalue) 
        {
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			if (nArgs == 2)
			{
	        	if (UpdateGlobalFile (Arg[1],Arg[2],0))
	        		goto RtnTrue;
	        }
			else
			{
	        	if (UpdateGlobalFile (Arg[1],Arg[2],Arg[3]))
	        		goto RtnTrue;
	        }
        	goto RtnFalse;
        }
        
		case 1208: //$CHANGESYMBOL(Refno,newsymbolname)  
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			Refno = atol (Arg[1]); 
			if (!PickByRefno (Refno,0,0,-1))
				goto RtnFalse;
			ProcessPickedItem (0,FALSE);
			SymNum = GetDictSymbolNumber (Arg[2]);        		
			if (EditSym (0,SymNum))
				goto RtnTrue;
			goto RtnFalse;
		}
		
		case 1209: //$ORATABLENAME(ORAFILENAME)
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			GetORATableName (Arg[1],OutLoc);
		    goto Rtnl;
			 
		}
		
		case 1210: //$LONGPATHNAME (ShortName)
		{   
			nArgs = GetFunArgs (Args,Arg,12,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;  
			GetLongPathName2 (Arg[1],256);
			_fstrcpy (OutLoc,Arg[1]);
           	goto Rtnl;
		}
		
		case 1211: //$HOTSPOTVALUE(Point)
		{   
			nArgs = GetFunArgs(Args, Arg, 12, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;  
			Point = atopt(Arg[1],&Err);
			nlong = GetHotSpotValue (Point); 
			ltoa (nlong,OutLoc,10);
           	goto Rtnl;
		}
				
	    case 1212: //$TRANSFERFILE(BUILDorLOAD,name(opt))
        {
            DLGPROC lpfnBUILDXFERFILEMsgProc;
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			
			st = 0;  
			SetTransferFileName (Arg[1],Arg[2]);
			if (!_fstricmp (Arg[1],"BUILD") || !_fstricmp (Arg[1],"RUN"))
			{
	            lpfnBUILDXFERFILEMsgProc = MakeProcInstance((DLGPROC)BUILDXFERFILEMsgProc, hInst);
	            st = DialogBox(hInst, (LPSTR)"XFERFILEBUILD", CurView->hWnd, lpfnBUILDXFERFILEMsgProc);
	            FreeProcInstance(lpfnBUILDXFERFILEMsgProc);
	            RunTransferFileCommand ();
	        }
			else if (!_fstricmp(Arg[1], "LOAD") || !_fstricmp(Arg[1], "VIEW"))
			{
	            lpfnBUILDXFERFILEMsgProc = MakeProcInstance((DLGPROC)LOADXFERFILEMsgProc, hInst);
	            st = DialogBox(hInst, (LPSTR)"XFERFILELOAD", CurView->hWnd, lpfnBUILDXFERFILEMsgProc);
	            FreeProcInstance(lpfnBUILDXFERFILEMsgProc); 
	        }
			if (st)
				goto RtnTrue;
			goto RtnFalse;
        }   
        
		case 1213: //$NAMESPLITTER(Name,Type)
		{   
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;  
			GetSTDNamePart (Arg[1],Arg[2],OutLoc);
           	goto Rtnl;
		}
				
		case 1214: //$CONVERTIMAGE(FromName,ToName,opt,nbitsperpix(opt-def24),transparentcolor(if 32 bit))
		{   
			HDIB32 hDib32In, hDib32Out=0;
			
			rtn = 0;
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;  
			hDib32In = BMPHandleFromEXT (Arg[1]);
			if (!hDib32In)
				goto RtnFalse;
			if (!stricmp(Arg[3],"FENCE"))
				rtn = CreateCompressedFenceFromBitmap (Arg[2],hDib32In,Arg[4]);
			else
			{
				switch (atoi (Arg[4]))
				{
				case 0:
					break;
				case 4:
					hDib32Out = FreeImage_ConvertTo4Bits(hDib32In);
					break;
				case -8:
					hDib32Out = FreeImage_ConvertTo8Bits(hDib32In);
					break;
				case 8:
					hDib32Out = FreeImage_ColorQuantize(hDib32In, FIQ_NNQUANT);
					break;
				case 16:
					hDib32Out = FreeImage_ConvertTo16Bits565(hDib32In);
					break;
				case 24:
				default:
					hDib32Out = FreeImage_ConvertTo24Bits(hDib32In);
					break;
				case 32:
					hDib32Out = FreeImage_ConvertTo32Bits(hDib32In);
					if (*Arg[5])
					{
						COLORREF icolor = atol (Arg[5]);
						BYTE r=GetRValue (icolor), g=GetGValue (icolor), b=GetBValue (icolor);
						DWORD	w = FreeImage_GetWidth(hDib32Out);
						DWORD	h = FreeImage_GetHeight(hDib32Out);
						DWORD	irow, icol;

						for (irow = 0;irow < h;irow++)
						{
							LPRGBQUAD	pC32 = (LPRGBQUAD)FreeImage_GetScanLine (hDib32Out,irow);

							for(icol = 0;icol < w;icol++,pC32++)
							{
								if (pC32->rgbBlue == b && pC32->rgbGreen == g && pC32->rgbRed == r)
									pC32->rgbReserved = 0;
								else
									pC32->rgbReserved = 255;
							}
						}
					}
					break;
				}
				makedirectories (Arg[2],FALSE,FALSE);
				if (hDib32Out)
					rtn = GMFIBMPHandleToEXT (Arg[2],hDib32Out,atoi(Arg[3]));
			}
			GMDestroyDIB32 (hDib32In);
			GMDestroyDIB32 (hDib32Out);
           	goto Rtnrtn;
		}

		case 1215: //$POLYPROBLEMS(Which Prob to check for (1-n) default=0or all),Symbol to display,size
		{ 
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			rtn = 0;
			if (HiPrecis && (CurrentType == GF_AREA || CurrentType == GF_POLYLINE))
			{   
				DPOINT	IntPoint;
				
				rtn=PolyCrossesItself (CurrentType,1,nPnts,lpDCurPoints,&IntPoint);
				if (rtn && *Arg[2])
				{ 
					POINT	PointLoc=BasePtToWinPt (&IntPoint);
					int		SymNum = GetDictSymbolNumber (Arg[2]);
					char	PolyProbListFile[MAX_PATH];

			        if (GetGlobalCVal ("[%POLYPROBLIST]",PolyProbListFile,NULL))
					{
						char	str[80];

						sprintf (str,"%ld,%i,%lf %lf",CurrentRefno,rtn,IntPoint.x,IntPoint.y);
						AppendFile (PolyProbListFile,str);
					}
//                    if (HaveLinkLines (lpDCurPoints,nPnts)) lpDCurPoints[0]
                   	DisplayPointItem (CurView->hDC,PointLoc,atof(Arg[3])*DeviceToScreenFactor(),0,SymNum,0);
                }
			}
			itoa (rtn,OutLoc,10);
			goto Rtnl;
		}
		case 1216: //$AREASTOLINES()
		{   
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;  
			if (AreasToLines (hWndMain,Arg[1],Arg[2],Arg[3]))
           		goto RtnTrue;  
           	goto RtnFalse;
		}

		case 1217: //$PCTINHLTAREA(item)
		{   
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;  
			RVal = PercentOfItemInHighlightAreas (atoi(Arg[1]));
			ftoa (OutLoc,RVal);
			goto Rtnl;
		}
		
		case 1218: //$COMPAREFILES(file1,file2)
		{
			int	nDiff;

			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				nDiff = -1;
			else
				nDiff = NumBytesDifferent (Arg[1],Arg[2]);
			itoa (nDiff,OutLoc,10);
			goto Rtnl;

		}
		case 1219: // $SETSYMPARENT(Symname,Parname)
		{   
			HANDLE		hSym;
			LPSYMBOL	pSym;
			int			np;
			 
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			n = GetDictSymbolNumber (Arg[1]);    
			if (!n)
            	goto RtnFalse; 
			np = GetDictSymbolNumber (Arg[2]);    
			if (!n)
            	goto RtnFalse; 
			hSym = GetDictSymDesc (n,0);
			pSym = (LPSYMBOL)GlobalLock (hSym); 
			pSym->Parent = np;
			GlobalUnlock (hSym);
            ReplaceSymbol (n,hSym);
			DestroySymbol (hSym);  
            goto RtnTrue;
		}   
		
		case 1220: //$SAVECONTOURS(OPEN,file)
				   //$SAVECONTOURS(CLOSE)
				   //$SAVECONTOURS(DISPLAY)
		{
			int	nDiff;

			nArgs = GetFunArgs(Args, Arg, 7, &hMem, pBrkPt, bpOffset, bpLen);
			if (!stricmp (Arg[1],"OPEN"))
			{
				int init[5];
				int rc;
				MNMXCORD rect = atobounds (Arg[2],&rc);//rect
				init[0] = rect.xmn;
				init[1] = rect.ymn;
				init[2] = atoi (Arg[3]);//nrows
				init[3] = atoi (Arg[4]);//ncols
				init[4] = atoi (Arg[5]);//width height
				if (SaveContours (-1,(HPDPOINT)init,Arg[6]))
				/*fidSaveSequence = 1;
				strcpy (saveContoursDir,Arg[2]);
				sprintf (Arg[3],"%s\\file%4.4i.bin",saveContoursDir,fidSaveSequence);
				fidSaveContours = GSSiOpenFile (Arg[3],0,OF_CREATE);
				if (fidSaveContours != HFILE_ERROR)*/
					goto RtnTrue;
			}
			else if (!stricmp (Arg[1],"CLOSE"))
			{
				if (SaveContours (-2,0,""))
				/*GSSiClose2 (&fidSaveContours);
				fidSaveContours = HFILE_ERROR;*/
					goto RtnTrue;
			}
			else if (!stricmp (Arg[1],"DISPLAY"))
			{
				strcpy (saveContoursDir,Arg[2]);
				gridSymbol = GetSymbolNum (Arg[3]);
				goto RtnTrue;
			}
			goto RtnFalse;
		}
		case 1221: //$THINCONTOURS(OPEN,file)
				   //$THINCONTOURS(CLOSE)
		{
			int	nDiff;

			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (!stricmp (Arg[1],"OPEN"))
			{
				strcpy (saveContoursDir,Arg[2]);
				if (OpenThinnedContours (Arg[3]))
					goto RtnTrue;
			}
			else if (!stricmp (Arg[1],"CLOSE"))
			{
				OpenThinnedContours (0);
				goto RtnTrue;
			}
			goto RtnFalse;
		}
		case 1222: //$TRANSPARENCY(SET,value,vp)
		{
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);

			if (!Display)
				goto RtnFalse;
			SetCurView (SetVPFromName (Arg[3],&Err));
			SetTransparency (atoi(Arg[2]));
			goto RtnTrue;
		}
		case 1223: //$SQLFIELDTYPE(gmdtype)
		{
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);

			if (!nArgs)
				goto RtnFalse;
			GMDFieldTypeToSQL (Arg[1],OutLoc);
			goto Rtnl;
		}
		case 1224: //$GMDFIELDTYPE(sqltype)
		{
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);

			if (!nArgs)
				goto RtnFalse;
			SQLFieldTypeToGMD (Arg[1],OutLoc);
			goto Rtnl;
		}

		case 1225: //$INSERTFORMAT(value,type)
		{
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);

			if (nArgs < 2)
				goto RtnFalse;
			if (!stricmp (Arg[2],"INTEGER"))
			{
				int intval = atoi (Arg[1]);
				itoa (intval,OutLoc,10);
			}
			else if (!stricmp (Arg[2],"NUMBER"))
			{
				double fltval = atof (Arg[1]);
				ftoa (OutLoc,fltval);
			}
			else
			{
				REPLAC (Arg[1],"'","''",4096);
				sprintf (OutLoc,"'%s'",Arg[1]);
			}
			goto Rtnl;
		}
		case 1226: //$SMALLMESSAGE(CREATE,text)
				   //$SMALLMESSAGE(UPDATE,hwnd,text)
				   //$SMALLMESSAGE(DESTROY,hwnd)
		{
			int iWnd;

			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (!stricmp(Arg[1], "CREATE"))
			{
				iWnd = (int)CreateSmallMessage(Arg[2]);
				itoa(iWnd, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "UPDATE"))
			{
				UpdateSmallMessage ((HWND)atoi(Arg[2]),Arg[3]);
				goto RtnTrue;
			}
			else if (!stricmp(Arg[1], "DESTROY"))
			{
				DestroyWindow((HWND)atoi(Arg[2]));
				goto RtnTrue;
			}
			goto RtnFalse;
		}
		case 1301: //$DISPLAYCONFIG(configfile,world bounds,display rect)
		{
			BOOL SaveSaveZoom = SaveZoom, SaveSaveGlobals = SaveGlobals, rtn;

			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 3)
				goto RtnFalse;
			if (!ExistFile (Arg[1]))
            	goto RtnFalse;
			if (sscanf (Arg[2],"%lf %lf %lf %lf",&Bounds.xmn,&Bounds.ymn,
											  	   &Bounds.xmx,&Bounds.ymx) != 4)
				goto RtnFalse;
			if (sscanf (Arg[3],"%i %i %i %i",&Rect.left,&Rect.bottom,
										   &Rect.right,&Rect.top) != 4)
				goto RtnFalse; 
			if (!DoDisplayConfigs || !InDrawTAG)
				goto RtnTrue;
			SaveZooms (&Bounds); 
	        ForceBounds = FALSE;
			GSSiGlobFree (&hSavedConfig[ConfigLevel]);
	    	hSavedConfig[ConfigLevel] = GSSiGlobAlloc (1216,GMEM_MOVEABLE,256);  
	    	ConfigRect[ConfigLevel] = Rect;   
	    	NextCFGTAG[ConfigLevel] = TBNum+1;
    		pFile = GlobalLock (hSavedConfig[ConfigLevel]);
        	GSSiGetTempFileName (0,"gm",0,pFile); 
         	SaveZoom = TRUE;
         	SaveGlobals = TRUE;
			SetConfig(1);
            rtn = SaveConfig (pFile,TRUE); 
            GlobalUnlock (hSavedConfig[ConfigLevel]); 
            if (!ConfigLevel)
            	SaveMainRect = LastMainRect;
            ConfigLevel++; 
	    	NextCFGTAG[ConfigLevel] = 0;
            SaveZoom = SaveSaveZoom;
            SaveGlobals = SaveSaveGlobals;
    		_fstrcpy (CfgName,Arg[1]);
    		if (ContinueTAGs == 1)
				PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);    
			else
				ContinueTAGs = 0;
			goto RtnTrue;  
				
        }
        
        case 1302: //$NUMNONCONTROL (text)
        {   
			hMem = GSSiGlobAlloc (1217,GMEM_MOVEABLE,3*2048);
			Arg1 = GlobalLock(hMem); 
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
        	nlong = 0;
        	while (*Arg1)
        	{
        		if (!iscntrl(*Arg1))
        			nlong++;
        		Arg1++;
        	}
			ltoa (nlong,OutLoc,10);  
		    goto Rtnl;
        }

		case 1303: // $SPORTMAPORDER()
        {
/*              DLGPROC	lpfnSPORTMAPORDERMsgProc;
              short		nRc; 
                  
              lpfnSPORTMAPORDERMsgProc = MakeProcInstance((DLGPROC)SPORTMAPORDERMsgProc, hInst);
              nRc = DialogBox(hInst, (LPSTR)"SPORTMAPORDER", hWndMain, lpfnSPORTMAPORDERMsgProc);
              FreeProcInstance(lpfnSPORTMAPORDERMsgProc);  
              if (nRc)
              	goto RtnTrue;
              else*/
              	goto RtnFalse;
        }
		  
		case 1304: //$ZOOMLISTBUILD (pathname,separator) combines areas with similar UDI (part before sep is same) into zoomlist
		{   BOOL SaveSaveZoom = SaveZoom, SaveSaveGlobals = SaveGlobals, rtn;

			if (!(ParLoc = MatchLev (Args,','))) goto Rtn0;
			hMem = GSSiGlobAlloc (1218,GMEM_MOVEABLE,4*2048);
			Arg1 = GlobalLock(hMem);
			Arg2 = Arg1 + 2048; 
			Arg3 = Arg2 + 2048; 
			Arg4 = Arg3 + 2048; 
			
			_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
			*ParLoc = '\0';
			_fstrcpy (Arg1,Args);
			if ((ParLoc = MatchLev (Arg2,',')))
			{
				_fstrcpy (Arg3,(LPSTR)(ParLoc+1));
				*ParLoc = '\0';
			}
			else
				*Arg3 = 0;
			if ((ParLoc = MatchLev (Arg3,',')))
			{
				_fstrcpy (Arg4,(LPSTR)(ParLoc+1));
				*ParLoc = '\0';
			}
			else
				*Arg4 = 0;
			ExpandText (Arg1);
			ExpandText (Arg2);
			ExpandText (Arg3); 
			ExpandText (Arg4); 
			if (BuildZoomList (Arg1,Arg2,Arg3,Arg4))
				goto RtnTrue;
			else
				goto RtnFalse; 
        }
        
		case 1305: // $SPLITXFERFILE(xfername,xferdir,numparts)
		{   
			if (!(ParLoc = MatchLev (Args,','))) goto Rtn0;
			hMem = GSSiGlobAlloc (1219,GMEM_MOVEABLE,3*2048);
			Arg1 = GlobalLock(hMem);
			Arg2 = Arg1 + 2048; 
			Arg3 = Arg2 + 2048;
			
			_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
			*ParLoc = '\0';
			_fstrcpy (Arg1,Args);
			if ((ParLoc = MatchLev (Arg2,','))) 
			{
				_fstrcpy (Arg3,(LPSTR)(ParLoc+1));
				*ParLoc = '\0';
			}
			else
				*Arg3 = 0;
			ExpandText (Arg1);
			ExpandText (Arg2);  
			ExpandText (Arg3);  
			n = atoi (Arg3);
			if (SplitXFERFile (Arg1,Arg2,n))
				goto RtnTrue;
		    goto RtnFalse;
		}

		case 1306: //$SELECTDBITEMS(database,SQL,prompt,displayval,returnval,returnvarname,handlevarname)
		{   
			DLGPROC lpfnSELECTDBITEMSMsgProc;
			
			nArgs = GetFunArgs(Args, Arg, 9, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 7)
				goto RtnFalse;
			hSelectItemsArgs = GSSiGlobAlloc (1220,GHND,128+1024+256+1024+256+64+64+256+256);
			Arg1 = GlobalLock (hSelectItemsArgs);   
			_fstrcpy (&Arg1[0],Arg[1]); //database
			_fstrcpy (&Arg1[128],Arg[2]);//SQL
			_fstrcpy (&Arg1[128+1024],Arg[3]);//prompt
			_fstrcpy (&Arg1[128+1024+256],Arg[4]);//displayval
			_fstrcpy (&Arg1[128+1024+256+1024],Arg[5]);//returnval
			_fstrcpy (&Arg1[128+1024+256+1024+256],Arg[6]);//returnvarname
			_fstrcpy (&Arg1[128+1024+256+1024+256+64],Arg[7]);//handlevarname
			_fstrcpy (&Arg1[128+1024+256+1024+256+64+64],Arg[8]);//TABS
			_fstrcpy (&Arg1[128+1024+256+1024+256+64+64+256],Arg[9]);//RECT
			GlobalUnlock (hSelectItemsArgs);
            setDoPaint( FALSE);      
			lpfnSELECTDBITEMSMsgProc = MakeProcInstance((DLGPROC)SELECTDBITEMSMsgProc, hInst);
			rtn = DialogBox(hInst, (LPSTR)"SELECTITEMS",hWndMain, lpfnSELECTDBITEMSMsgProc);
			FreeProcInstance(lpfnSELECTDBITEMSMsgProc); 
			setDoPaint( TRUE);                              
			GSSiGlobFree (&hSelectItemsArgs);
            itoa (rtn,OutLoc,10);
			goto Rtnl;
        }  
		case 1307: //$CREATESF3FILE (Infile,OutFile)
		{   
			nArgs = GetFunArgs(Args, Arg, 12, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;  
			if (CreateSF3File (Arg[1],Arg[2]))
				goto RtnTrue;
			goto RtnFalse;
		}
				
		case 1308: //$CREATESF1FILE (Infile,OutFile)
		{   
			nArgs = GetFunArgs(Args, Arg, 12, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;  
			if (CreateSF1File (Arg[1],Arg[2]))
				goto RtnTrue;
			goto RtnFalse;
		}

		case 1309: //$SHORTPATHNAME (LongName)
		{   
			nArgs = GetFunArgs (Args,Arg,12,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;  
			GetShortPathName2 (Arg[1],256);
			_fstrcpy (OutLoc,Arg[1]);
           	goto Rtnl;
		}
				
		case 1310: //$PROCESSSTATUS (CREATE,ViewportName,TITLE)
		{   
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;  
			SetCurView (SetVPFromName (Arg[2],&Err));
			SetProcessStatusTitle (Arg[3]); 
			CurView = SaveVP;
           	goto RtnTrue;
		}
				
		case 1311: //$LINESTOPOINTS(OutFile,sampledistance)
		{   
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;  
			if (LinesToPoints (Arg[1],atof(Arg[2])))
           		goto RtnTrue;  
           	goto RtnFalse;
		}

		case 1312: // $PARENTSYMBOLS(underparent)  //displays list of parents to select from
		{				
			hMem = GSSiGlobAlloc (1178,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			*OutLoc = 0;  
			if (!SelectParentSymbol (CurView->hWnd,0,OutLoc))
				*OutLoc = 0;
			goto Rtnl;   
		}
		
		case 1313: //$ACCELEROMETER(FRONTANGLE,x,y,z)
		{
			double x, y, z;
			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 4)
				goto RtnFalse;
			x = atof(Arg[2]);
			y = atof(Arg[3]);
			z = atof(Arg[4]);
			//(void)slopesFromAccelerometer:(int)nReadings
			{
				double denom = sqrt(square(x) + square(y) + square(z));
				double angz = acos(z / denom);
				double angy = acos(y / denom);
				double angx = acos(x / denom);
				double pct;
				angx *= RADtoDEG;
				angy *= RADtoDEG;
				angz *= RADtoDEG;
				angx = (angx - 90.0);
				angy = (angy - 90.0);
				angz = (180.0 - angz);
				if (!strcmp(Arg[1], "FRONTANGLE"))
					ftoa(OutLoc, angy);
				else if (!strcmp(Arg[1], "FRONTPCT"))
				{
					pct = (tan(angy * DEGtoRAD) * 100);
					ftoa(OutLoc, pct);
				}
				else if (!strcmp(Arg[1], "SIDEANGLE"))
					ftoa(OutLoc, angx);
				else if (!strcmp(Arg[1], "SIDEPCT"))
				{
					pct = (tan(angx * DEGtoRAD) * 100);
					ftoa(OutLoc, pct);
				}
				/*				_lastDegrees = angy;

				_data.frontSlopeDegrees = [self adjustSlope : angy
				isSideSlope : NO];

				_data.sideSlopeDegrees = [self adjustSlope : angx
				isSideSlope : YES];

				_data.frontSlopePct = (tan(_data.frontSlopeDegrees * DEGtoRAD) * 100);
				_data.sideSlopePct = (tan(_data.sideSlopeDegrees * DEGtoRAD) * 100);
				_data.maxSlopePct = ComputeMaxSlope(_data.frontSlopePct, _data.sideSlopePct);*/
			}

			goto Rtnl;
		}
		case 1314: //$POINTINBOUNDS(PT,BOUNDS)
		{
			BOOL err;
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs == 2)
			{
				DPOINT pt = atopt(Arg[1], &err);
				if (!err)
				{
					MNMXCORD bounds = atobounds(Arg[2], &err);
					if (!err)
					{
						if (PointInBounds(pt, &bounds))
							goto RtnTrue;
					}
				}
			}
			goto RtnFalse;
		}
        case 1401: //$SETTEXTGLOBALS() 
        {
		
			LPGRTEXTHEADER	pPickedTextHeader; 
			
			GSSiGlobFree (&hTextString);  
    		GSSiGlobFree (&hPickedTextHeader);
		    hPickedTextHeader = GSSiGlobAlloc (1221,GMEM_MOVEABLE,sizeof(GRTEXTHEADER)+512); 
        	if (GetNextHLTListItem (TRUE,TRUE)) 
        	{   
        		if (hPickedTextHeader)
        		{
				    pPickedTextHeader = (LPGRTEXTHEADER)GlobalLock (hPickedTextHeader);
			       	if (lTextString && hTextString)
			       	{   
			       		BOOL	SaveDisableHalt;
			       		char	str[64]; 
			       		LPSTR	pString;
			       		double	THeight;
			       		
			       		pString = GlobalLock (hTextString);
						SetGlobalValue2 (hTEXT,pString,0);
						GSSiGlobUlFree (&hTextString);
						THeight = GetTextHeadSize (pPickedTextHeader);
						sprintf (str,"%f",THeight);
						if (pPickedTextHeader->HeightIsPixels)
							_fstrcat (str,"P");
                        SetGlobalValue ("%TEXTSIZE",str);
			       	}
		    		GSSiGlobUlFree (&hPickedTextHeader); 
		    	}
        		goto RtnTrue;
        	}
        	else 
        	{
	    		GSSiGlobFree (&hPickedTextHeader);
				GSSiGlobFree (&hTextString);  
        		goto RtnFalse; 
        	}
        } 
        
        case 1402: //$LOADSSURGOCOMP (compfile,compfile.gmd)
		{   
			 
			if (!(ParLoc = MatchLev (Args,','))) goto Rtn0;
			hMem = GSSiGlobAlloc (1222,GMEM_MOVEABLE,2*2048);
			Arg1 = GlobalLock(hMem);
			Arg2 = Arg1 + 2048; 
			
			_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
			*ParLoc = '\0';
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			ExpandText (Arg2); 
			if (ImportSSURGOTables (Arg1,Arg2))
            	goto RtnTrue;
            goto RtnFalse;
		} 
		
        case 1403: //$SETDESTINATION(bounds or null to clear) 
        {
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			
			if (nArgs)
			{
				Bounds = atobounds (Arg[1],&Err);
				if (Err)
					goto RtnFalse;
				SetDestination (&Bounds); 
			}
			else
				SetDestination (0);
           	goto RtnTrue;
		}
		
        case 1404: //$SELECTFONTNAME(curname) 
        {   
        	LPLOGFONT pLogFont;
        	
			hMem = GSSiGlobAlloc (1224,GMEM_MOVEABLE,2048+sizeof(LOGFONT));
			Arg1 = GlobalLock(hMem);
			pLogFont = (LPLOGFONT)(Arg1+2048);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			*OutLoc = 0;
            if (GetFont (CurView->hWnd, pLogFont, &nlong,0,0)) 
            	_fstrcpy (OutLoc,pLogFont->lfFaceName);
           	goto Rtnl;
		}  
		
        case 1405: //$BOUNDSTOPOINTS(bounds,tranfile(opt))
        {   
        	HANDLE	hPoints, hTran=0;
        	
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (!nArgs)
				goto RtnFalse;
			*OutLoc = 0; 
			Bounds = atobounds (Arg[1],&Err);
			if (!Err)
			{    
				hPoints = GSSiGlobAlloc (1225,GMEM_MOVEABLE,4*sizeof(DPOINT));
				pPoint = (HPDPOINT)GlobalLock (hPoints);
				BoundsToPoints (&Bounds,pPoint,0);  
				if (*Arg[2])
					hTran=LoadTranFileWithDandT(Arg[2]); 
				for (n=0;n<4;n++)
				{   
					if (n)
						_fstrcat (OutLoc," ");
					pPoint[n] =  TranPoint (&pPoint[n],hTran);
					sprintf (_fstrchr (OutLoc,0),"%f %f",pPoint[n].x,pPoint[n].y);
				}
				GSSiGlobUlFree (&hPoints);
				CloseTRANS2 (&hTran);
			}
			goto Rtnl;
		}		
		
        case 1406: //$SPLITMRSIDFILE(file,MrSidEx)
        {   
        	HANDLE	hPoints, hTran=0;
        	
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs<2)
				goto RtnFalse;     
			if (SplitMrSidFile (Arg[1],Arg[2]))
				goto RtnTrue; 
			goto RtnFalse;
		}

        case 1407: //$FINDDUPLICATES(file)
        {   
        	HANDLE	hPoints, hTran=0;
        	 
			nlong = -1;
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs<1)
				goto RtnFalse; 
			if (nArgs == 1)
			{
				nlong = GetDuplicateRecords (Arg[1]);
			}
			else if (!stricmp (Arg[1],"POINTS"))
			{
				nlong = GetDuplicatePoints (Arg[2],atof(Arg[3]));
			}
			else if (!stricmp (Arg[1],"LINES"))
			{
				nlong = GetDuplicateLines (Arg[2],atof(Arg[3]));
			}
			ltoa (nlong,OutLoc,10);
			goto Rtnl;
		}
		
        case 1408: //$FILTERTEXTFILE(Infile,OutFile,Bounds)
        {   
        	HANDLE	hPoints, hTran=0;
        	
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs<3)
				goto RtnFalse;     
			if (FilterTextFile (Arg[1],Arg[2],atobounds(Arg[3],&Err)))
				goto RtnTrue;
			goto RtnFalse;
		}
		
		case 1409: //$RECOVERPLTFILE(inname,outname)
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			
			if (RecoverPltFile (Arg[1],Arg[2]))
				goto RtnTrue;
			goto RtnFalse;
		
		case 1410: //$STRINGFROMFILE(File,string,nchartoreturnafterstring)
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			strcpy (OutLoc,"0");
			Fid = GSSiOpenFile (Arg[1],0,OF_READ);
			if (Fid != HFILE_ERROR)
			{
				int	ln = GSSifilelength (Fid);
				HANDLE	hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,ln+1);

				if (hMem)
				{
					LPSTR pMem = GlobalLock (hMem);
					LPSTR	pLoc;
					int		nc;

					BigRead (Fid,pMem,ln);
					pMem[ln] = 0;
					pLoc = strstr (pMem,Arg[2]);
					if (pLoc)
					{
						pLoc += strlen (Arg[2]);
						nc = atoi (Arg[3]);
						if (nc > 0)
							strncpy0 (OutLoc,pLoc,nc);
						else
							strcpy (OutLoc,"1");
					}
					GSSiGlobUlFree (&hMem);
				}
				GSSiClose2 (&Fid);
			}
			goto Rtnl;
		
		case 1411: //$GLOBALFROMFILE(File,globalvarname,replace0with)
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			rtn = FALSE;
			SetGlobalValue (Arg[2],"");
			Fid = GSSiOpenFile (Arg[1],0,OF_READ);
			if (Fid != HFILE_ERROR)
			{
				int i;
				char replaceChar='0x1';
				int	ln = GSSifilelength (Fid);
				HANDLE	hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,ln+1);

				if (*Arg[3])
					replaceChar = *Arg[3];
				if (hMem)
				{
					LPSTR pMem = GlobalLock (hMem);
					LPSTR	pLoc;
					int		nc;

					BigRead (Fid,pMem,ln);
					pMem[ln] = 0;
					for (i=0;i<ln;i++)
						if (!pMem[i])
							pMem[i] = replaceChar;
					SetGlobalValue (Arg[2],pMem);
					GSSiGlobUlFree (&hMem);
					rtn = TRUE;
				}
				GSSiClose2 (&Fid);
			}
			goto Rtnl;

		case 1412: //$COMPRESSEDFILE(CREATE,File,filelistfile)
			nArgs = GetFunArgs(Args, Arg, 8, &hMem, pBrkPt, bpOffset, bpLen);
			rtn = CompressedFileCmd (nArgs, Arg);
			goto Rtnrtn;
			
        case 1501: //$OPENVEHICLEFILE(filename,Delay) 
        {
			hMem = GSSiGlobAlloc (1226,GMEM_MOVEABLE,3*2048);
			Arg1 = GlobalLock(hMem);
			Arg2 = Arg1 + 2048; 
			Arg3 = Arg2 + 2048;
			
			_fstrcpy (Arg1,Args);
			if ((ParLoc = MatchLev (Arg1,','))) 
			{
				_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
				*ParLoc = '\0';
			}
			else
				*Arg2 = 0;
			if ((ParLoc = MatchLev (Arg2,','))) 
			{
				_fstrcpy (Arg3,(LPSTR)(ParLoc+1));
				*ParLoc = '\0';
			}
			else
				*Arg3 = 0;
			ExpandText (Arg1);
			ExpandText (Arg2);  
			ExpandText (Arg3); 
			nlong = atol (Arg2); 
            if (OpenDummyVehicleFile(Arg1,nlong))
            	goto RtnTrue;
            goto RtnFalse;
        }
        case 1502: //$REMOVELINKLINES() 
        {
            if (RemoveLinkLines())
            	goto RtnTrue;
            goto RtnFalse;
        }  
        
        case 1503: //$GETADDRESSCOORD (House,Street,City,ZIP,OUTVARNAME,outmacro)
				   //$GETADDRESSCOORD (GOOGLE,full address)
		{   
			 
			nArgs = GetFunArgs(Args, Arg, -9, &hMem, pBrkPt, bpOffset, bpLen);
			if (!nArgs)
			{
				rtn = AddressLocation1 (CurView->hWnd,hInst,IDM_L_NET_ADDRESS);
				if (rtn)
					ExecutePointLocationMacro (UserSpecifiedBasePoint,0);
			}
			else if (!stricmp(Arg[1], "MAPQUEST"))
			{
				DPOINT	Point;
				int	rtn;
				char	Quality[32];

				ExpandText(Arg[2]);
				ExpandText(Arg[3]);
				rtn = atoi(Arg[3]);
				if (!rtn)
					rtn = 3;
				if ((rtn = GetMapQuestLocation(Arg[2], Quality, &CurrentPoint, rtn)))
				{
					ExecutePointLocationMacro(CurrentPoint,0);
				}
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "GOOGLE"))
			{
				DPOINT	Point;
				BOOL	haveVPPoints;
				DPOINT	vpPoints[2];
				char	Quality[32];

				ExpandText(Arg[2]);
				ExpandText(Arg[3]);
				rtn = atoi(Arg[3]);
				if (!rtn)
					rtn = 3;
				if ((rtn = GetGoogleLocation(Arg[2],1,Arg[7],&CurrentPoint,&haveVPPoints,vpPoints,Arg[8],Arg[9])) > 0)
				{
					ExecutePointLocationMacro(CurrentPoint,Arg[7]);
				}
				else
				{
					REPLAC(Arg[2], ",", "@,",1024);
					ExecuteLocationFailedMacro(Arg[2],Arg[7]);
				}
				itoa(rtn, OutLoc, 10);
				goto Rtnl;
			}
			else
			{
				ExpandText (Arg[1]);
				ExpandText (Arg[2]);  
				ExpandText (Arg[3]); 
				ExpandText (Arg[4]);  
				ExpandText (Arg[5]); 
				SetAddEditValues (Arg[1],Arg[2],Arg[3],Arg[4],Arg[5],Arg[6],0);
				*DestName = 0; 
				if (nArgs < 6)
				{
	//                  CreateDialog(hInst, (LPSTR)"ADD_MATCH_EDIT2", hWndMain, lpfnADD_MATCH_EDITMsgProc);
					rtn = DialogBox(hInst, (LPSTR)"ADD_MATCH_EDIT2", hWndMain, (DLGPROC)ADD_MATCH_EDITMsgProc);
				}  
				else
				{
					  HWND	hWndDlg;

					  rtn = TRUE;
					  hWndDlg = CreateDialog(hInst, (LPSTR)"ADD_MATCH_EDIT2", hWndMain, (DLGPROC)ADD_MATCH_EDITMsgProc);
					  PostMessage (hWndDlg,WM_COMMAND,IDC_ISMODELESS,0);
	//                  nRc = DialogBox(hInst, (LPSTR)"ADD_MATCH_EDIT", hWndDlg, lpfnADD_MATCH_EDITMsgProc);
	//                  FreeProcInstance(lpfnADD_MATCH_EDITMsgProc);
				}  
			}
            goto Rtnrtn;
        }
  
        
        case 1504: //$TEXTTOCLIPBOARD(text) 
        {
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
           	if (!OpenClipboard (hWndMain))
           		goto RtnFalse;
			EmptyClipboard();
			if (nArgs) 
			{   
				BOOL	KeepMemLengthSave=KeepMemLength;
				
	           	KeepMemLength = TRUE;  
		        hStr = GSSiGlobAlloc (1228,GHND,4096);  
	   	        KeepMemLength = KeepMemLengthSave;
	   	       	str = GlobalLock (hStr);  
	   	       	_fstrcpy (str,Arg[1]);
				GlobalUnlock (hStr);
				#if CHECKMEM
					GSSiRemoveMem (hStr);
				#endif
				SetClipboardData(CF_TEXT, hStr); 
			}
	        CloseClipboard();
	        goto RtnTrue;
	    }

	    case 1505: //$ADDRESSLOCATION(CREATE,addloc.al1) 
	    		   //$ADDRESSLOCATION(USER,ADD,address,Munic,point,Unmatchable);
	    		   //$ADDRESSLOCATION(BYAREA,outfile,addresslocationfile,areafieldname)
        {
            DLGPROC	lpfnADDLOC_CREATEMsgProc; 
              
			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse; 
			if (!_fstricmp (Arg[1],"CREATE"))
			{
				_fstrcpy (AutoExportName,Arg[2]);
				lpfnADDLOC_CREATEMsgProc = MakeProcInstance((DLGPROC)ADDLOC_CREATEMsgProc, hInst);
				rtn = DialogBox(hInst, (LPSTR)"ADDLOC_CREATE", hWndMain, lpfnADDLOC_CREATEMsgProc);
				FreeProcInstance(lpfnADDLOC_CREATEMsgProc);   
				if (rtn)
					goto RtnTrue;
			}
			if (!_fstricmp (Arg[1],"USER")) 
			{   
				Point = atopt (Arg[5],&Err);
				if (Err)
					goto RtnFalse;
				if (AddUserAddress (Arg[3],Arg[4],Point,atob(Arg[6])))
					goto RtnTrue;
			}
			if (!_fstricmp (Arg[1],"BYAREA")) 
			{   
				if (CreateAddMatchByArea (Arg[3],Arg[2],Arg[4]))
					goto RtnTrue;
			}
			if (!_fstricmp (Arg[1],"USERFROMFILE")) 
			{   
				if (AddToUserDefinedAddressFromFile (Arg[2]))
					goto RtnTrue;
			}

			goto RtnFalse;
        }  
        
	    case 1506: //$CREATEWORDINDEX(FromFile,FromField,ToFile)
			//$CREATEWORDINDEX(REMOVEDUPS,FromFile)
        {
              
			nArgs = GetFunArgs(Args, Arg, -4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			ExpandText(Arg[1]);
			ExpandText(Arg[4]);
			if (!_fstricmp(Arg[1], "REMOVEDUPS"))
			{
				ExpandText(Arg[2]);
				if (WordIndexRemoveDups(Arg[2]))
					goto RtnTrue;
			}
			else 
				if (CreateWordIndex(Arg[1], Arg[2], Arg[3], atob(Arg[4])))
					goto RtnTrue;
			goto RtnFalse;
        }  
        
	    case 1507: //$GETMAXFILEREFNO(filelist.txt)
        {
            DLGPROC	lpfnADDLOC_CREATEMsgProc; 
              
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse; 
			nlong = MaxFileRefno (Arg[1]);
			ltoa (nlong,OutLoc,10);  
			goto Rtnl;
        }  
        
		case 1508: // $DELETEDIRECTORY(path,Y or N or T or F or 1 or 0 (verify switch))
		{
			int	type;

			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (*LastChr(Arg[1]) == '\\')
				*LastChr(Arg[1]) = 0;
			type = GetPathType (Arg[1]);
			if (!type)
				goto RtnTrue;
			if (type != 2)
				goto RtnFalse;
			if (atob (Arg[2]))
			{   
				HANDLE	hTemp=GSSiGlobAlloc (1181,GMEM_MOVEABLE,512);
				LPSTR	Mess=GlobalLock (hTemp);
             	sprintf (Mess,"Delete directory '%s'?",Arg[1]);
				ExpandText(Mess);
             	st = MessageBox (GetFocus(),Mess,"Verify Delete",MB_YESNO); 
				GSSiGlobUlFree (&hTemp);
			}
			else
				st = IDYES;
            
            if (st != IDYES)
            	goto RtnFalse;
            
			if (DeleteDirAndContents (Arg[1]))
				goto RtnTrue;
			else
            	goto RtnFalse;
			
		} 
		case 1509://$NETWORKANALYZER
		{
			rtn = DialogBox(hInst, (LPSTR)"RemoteNetworkAnalyzer", GetFocus(), (DLGPROC)NetworkAnalyzerMsgProc);
			goto RtnTrue;
		}
		case 1510://$COPYWITHREPLACE(tofile,fromfile,fromtext|totext;fromtext2|totext2;etc)
		{
			LPSTR line;

			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 3)
				goto RtnFalse;
			line = Arg[4];
			Fid1 = GSSiOpenFile(Arg[2], 0, OF_READ);
			if (Fid1 == HFILE_ERROR)
				goto RtnFalse;
			Fid2 = GSSiOpenFile(Arg[1], 0, OF_CREATE);
			if (Fid2 == HFILE_ERROR)
			{
				GSSiClose2 (&Fid1);
				goto RtnFalse;
			}
			while (fgetstring(line, 4090, Fid1))
			{
				LPSTR fromText, toText, pBeg = Arg[3], pEnd;
				do {
					pEnd = strchr(pBeg, ';');
					if (pEnd) *pEnd = 0;
					strcpy(Arg[5],pBeg);
					fromText = Arg[5];
					toText = strchr(fromText, '|');
					if (toText)
					{
						*toText++ = 0;
						SubstituteDL(toText, FALSE);
						REPLAC(line, fromText, toText, -4090);
					}
					if (pEnd)
					{
						*pEnd = ';';
						pBeg = pEnd + 1;
					}
				} while (pEnd);
				fputstring(line, Fid2);
			}
			GSSiClose2 (&Fid1);
			GSSiClose2 (&Fid2);
			goto RtnTrue;
		}
		case 1601: //$CREATESPORTMAPCD(orderfile,outdir) 
        {
			if (!(ParLoc = MatchLev (Args,','))) goto Rtn0;
			hMem = GSSiGlobAlloc (1229,GMEM_MOVEABLE,3*2048);
			Arg1 = GlobalLock(hMem);
			Arg2 = Arg1 + 2048; 
			Arg3 = Arg2 + 2048;
			
			_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
			*ParLoc = '\0';
			_fstrcpy (Arg1,Args);
			if ((ParLoc = MatchLev (Arg2,','))) 
			{
				_fstrcpy (Arg3,(LPSTR)(ParLoc+1));
				*ParLoc = '\0';
			}
			else
				*Arg3 = 0;
			ExpandText (Arg1);
			ExpandText (Arg2);  
			ExpandText (Arg3);  
			n = atoi (Arg3);
            if (CreateSportMapCD(Arg1,Arg2,n))
            	goto RtnTrue;
            goto RtnFalse;
        }

	    case 1602: //$ADDISLANDSTOPOLY(Reverse,deleteislands)
        {
              
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (AddIslandsToPoly (atob (Arg[1]),atob (Arg[2])))
				goto RtnTrue;
			goto RtnFalse;
        }   
        
	    case 1603: //$POINTSBETWEENMPS(Path,FromMP,ToMP)
        {
            HANDLE	hPoints;
              
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			nlong = PointsBetweenMP (atol(Arg[1]),atof(Arg[2]),atof(Arg[3]),&hPoints);
			*OutLoc = 0; 
			if (hPoints)
			{
				pPoint = (HPDPOINT)GlobalLock (hPoints);
				for (nl=0;nl<nlong;nl++)
				{
					if (nl)
						_fstrcat (OutLoc," ");
					dpointtoa (_fstrchr (OutLoc,0),&pPoint[nl]);
				}
				GSSiGlobUlFree (&hPoints);
			}
			goto Rtnl;
        }  

	    case 1604: //$POINTSBETWEENPCT(REFNO,FROMPCT,TOPCT,ShapePointsOnly(TorF),Newbp Newep,AddToNewPoly)
				   // if newbp and ep supplied transforms points between pct to new loc
        {
            HANDLE	hPoints, hBPEP=0;
			LPDPOINT	pNewBPEP=0;
              
			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
			if (GetPointsFromList (Arg[5],&hBPEP) != 2)
				GSSiGlobFree (&hBPEP);
			nlong = PointsBetweenPCT (atol(Arg[1]),atof(Arg[2]),atof(Arg[3]),atob(Arg[4]),&hPoints,&hCurvePoints,hBPEP); 
			GSSiGlobFree (&hBPEP);
			if (nlong == 2)
			{
				HPDPOINT	pPoints=GlobalLock (hPoints);
				
				if (SameDPoint (&pPoints[0],&pPoints[1]))
					nlong = 1;
				GlobalUnlock (hPoints);
			}
			if (atob(Arg[6]))
			{
				if (nlong)
				{
					HPDPOINT	pPoints=GlobalLock (hPoints), pNewPoints;

					if (!NumNewPolyPoints)
						hNewPolyPoints = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX*sizeof(DPOINT));

					pNewPoints=GlobalLock (hNewPolyPoints);
					for (i=0;i<nlong;i++)
						pNewPoints[NumNewPolyPoints+i] = pPoints[i];
					NumNewPolyPoints += nlong;
					GlobalUnlock (hNewPolyPoints);
					GSSiGlobUlFree (&hPoints);
					goto RtnTrue;
				}
				else
					goto RtnFalse;
			}
			else
			{
				if (nlong > 500)
				{
					double	maxdist;
					HPDPOINT	pPoints=GlobalLock (hPoints);
					
					maxdist = GetPolyLengthD (pPoints,nlong)/1000;
					ThinPoly (&nlong, pPoints, maxdist);
					if (nlong > 500)
						ThinPoly (&nlong, pPoints, maxdist*10);
					GlobalUnlock (hPoints);
				}
				*OutLoc = 0; 
				if (hPoints)
				{
					pPoint = (HPDPOINT)GlobalLock (hPoints);
					for (nl=0;nl<nlong;nl++)
					{
						if (nl)
							_fstrcat (OutLoc," ");
						//dpointtoatrunc (_fstrchr (OutLoc,0),&pPoint[nl]);
						dpointtoa (_fstrchr (OutLoc,0),&pPoint[nl]);
						//if (NDP)
						//	RWRITE (double RVAL, int NDP, LPSTR OutLoc)
					}
					GSSiGlobUlFree (&hPoints);
				}
			}
			goto Rtnl;
        }  
                        
	    case 1605: //$FILLRECTWITHGRID(pltfile,symbolname,bounds,gridspace)
        {
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
        	
			Bounds = atobounds (Arg[3],&Err);
			if (Err)
				goto RtnFalse;
			if (!ValidBounds (&Bounds))
				goto RtnFalse;
			RVal = atof (Arg[4]);
        	if (FillRectWithGrid (Arg[1],Arg[2],&Bounds,RVal))
        		goto RtnTrue;
        	goto RtnFalse;
        }

		case 1606: //$APPENDFILETOFILE(tofile,fromfile)
		{
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			strcpy (OutLoc,"-1");
			if (nArgs < 2)
				goto Rtnl;
			Fid1 = GSSiOpenFile (Arg[1],0,OF_READWRITE);
			if (Fid1 == HFILE_ERROR)
				Fid1 = GSSiOpenFile (Arg[1],0,OF_CREATE);
			if (Fid1 != HFILE_ERROR)
			{
				int loc = GSSillseek (Fid1,0,2);
				int	lBuf = SHRT_MAX, ln;
				HANDLE	hBuf = GSSiGlobAlloc (0,GMEM_MOVEABLE,lBuf);
				LPSTR	pBuf = GlobalLock (hBuf);

				Fid2 = GSSiOpenFile (Arg[2],0,OF_READ);
				if (Fid2 != HFILE_ERROR)
				{
					while ((ln = BigRead (Fid2,pBuf,lBuf)) > 0)
						BigWrite (Fid1,pBuf,ln,-1);
					GSSiClose2 (&Fid2);
					itoa (loc,OutLoc,10);
				}
				GSSiClose2 (&Fid1);
				GSSiGlobUlFree (&hBuf);
			}
			goto Rtnl;
		}
                
		case 1607: //$BASETODISTFACTOR(FEETorMETERSorMILES,BasePt,ReferenceProjection)
		{
			double refdist = 100.0;
			double dist;
			double factor = 1;
			DPOINT refPoints[2];

			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (nArgs == 3)
			{
				Point = atopt(Arg[2], &Err);
				if (!Err)
				{
					char saveAltProj[MAX_PATH] = "[%ALT_PROJECTION]";

					ExpandText(saveAltProj);
					FreePROJ(3);
					SetGlobalValue("%ALT_PROJECTION", Arg[3]);
					ConvertCoordClose();
					ConvertCoordInit();

					ConvertCoord(&Point, 1, 3);
					if (!_fstricmp(Arg[1], "FEET"))
					{
						dist = ConvertAltDist(refdist, 1);
					}
					else if (!_fstricmp(Arg[1], "METERS"))
					{
						dist = ConvertAltDist(refdist, 2);
					}
					if (!_fstricmp(Arg[1], "MILES"))
					{
						dist = ConvertAltDist(refdist, 4);
					}
					refPoints[0] = dnewpt(Point, HALFPI / 2, dist);
					refPoints[1] = dnewpt(Point, PY + HALFPI / 2, dist);
					ConvertCoord(&refPoints[0], 3, 1);
					ConvertCoord(&refPoints[1], 3, 1);
					dist = ldistp(refPoints[0], refPoints[1]);
					SetGlobalValue("%ALT_PROJECTION",saveAltProj);
					FreePROJ(3);
					ConvertCoordClose();
					ConvertCoordInit();
					factor = dist / 100.0;
					ftoa(OutLoc, factor);
				}
			}
			goto Rtnl;

		}
		case 1701: //$RECOVERPLTFROMRIN(pltname)
        {
              
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (RecoverPLTFromRIN (Arg[1]))
				goto RtnTrue;
			goto RtnFalse;
        }   
        case 1702: //$SCREENTOCLIPBOARD(opt viewport) 
        {   
//        	HDIB	hDIB;
        	
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
           	if (!OpenClipboard (hWndMain))
           		goto RtnFalse; 
           	if (!_fstricmp (Arg[1],"CLIENT"))
           	{ 
				GetClientRect (hWndMain,&Rect);   
				ClientRectToScreenRect (CurView->hWnd,&Rect); 
			}
			if (*Arg[1]) 
			{
				SetCurView (SetVPFromName (Arg[1],&Err));
				Rect = CurView->Rect;
				ClientRectToScreenRect (CurView->hWnd,&Rect);
			}
			else	
				GetWindowRect (hWndMain,&Rect);   
			EmptyClipboard();
           	KeepMemLength = TRUE;  
//			hStr = BitmapToDIB (hBM,0);
			hStr = CopyScreenToDIB (&Rect); 
   	        KeepMemLength = FALSE;  
/*	        pMem1 = GlobalLock (hStr); 
   	        pMem2 = GlobalLock (hFullWindowBitMap);
   	        hmemmove (pMem1,pMem2,nlong);
   	        GlobalUnlock (hFullWindowBitMap);*/
			#if CHECKMEM
				GSSiRemoveMem (hStr);
			#endif
			SetClipboardData(CF_DIB, hStr);
			CloseClipboard();  
	        goto RtnTrue;
	    } 

		case 1703: //$PROFILEAREAPOINTS([%WX],PROFILEAREA,Profile Image);
		{
			double	x;
			DPOINT	Points[2];

			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 3)
				goto RtnFalse;
			*OutLoc = 0;
			x = atof (Arg[1]);
			if (GetProfileAreaPoints (x,Arg[2],Arg[3],Points))
				boundstoa (OutLoc,(LPMNMXCORD)Points);
			goto Rtnl;
		}

		case 1704://$SPLITCONTOURLINES(split line opt)
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			i = atoi (Arg[1]);
			rtn = SplitContourLines (i);
			goto Rtnrtn;

		case 1705://$TEXTFROMCLIPBOARD()
		{
			HANDLE	hText;
			LPSTR	pText;

			OpenClipboard (hWndMain);
			hText  = GetClipboardData(CF_TEXT);
			if (hText)
			{
				IgnoreLock = TRUE;
				pText = GlobalLock (hText);
				strcpy (OutLoc,pText);
				GlobalUnlock (hText);
				IgnoreLock = FALSE;
			}
			CloseClipboard ();
			goto Rtnl;
		}

        case 1801: //$LOADBLOCKINGPOINTS (file)
        {   
			hMem = GSSiGlobAlloc (1230,GMEM_MOVEABLE,3*2048);
			Arg1 = GlobalLock(hMem); 
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			if (LoadBlockingPoints (Arg1))
		    	goto RtnTrue;
		    goto RtnFalse;
        }
        case 1802: //$NEWELEMENTSETTINGS(type) A,PorL
        {   
			hMem = GSSiGlobAlloc (1231,GMEM_MOVEABLE,2048);
			Arg1 = GlobalLock(hMem); 
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			_fstrupr (Arg1); 
			Arg1[1] = 0;
			n = _fstrcspn ("PLA",Arg1);
			if (n < 3)
			{
				if (SetNewElementValues(CurView->hWnd,n+1))
			    	goto RtnTrue; 
		    }
		    goto RtnFalse;
        }
	    case 1803: //$ADJOININGAREASFILE(CREATE,NAME)
        {
              
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (CreateAdjoiningAreasFile (Arg[2]))
				goto RtnTrue;
			goto RtnFalse;
        }  
		case 1804: //$GETNEARESTINTCOORD(x,y,streetnum,nchar,filename)
		{

			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (GetNearestIntPoint(Arg[1], Arg[2], Arg[3], Arg[4], Arg[5], OutLoc))
				goto Rtnl;
			goto RtnFalse;
		}
		case 1805: //$GETPERPPOINTONPOLY(pickitemnum,pt,az)
		{
			int np;
			HANDLE hPoly;
			DPOINT pt;
			int item;
			LPDPOINT pPoints;
			DPOINT IntPoint;
			BOOL err;
			double OffDist, PolyDist;

			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			item = atoi(Arg[1]);
			pt = atopt(Arg[2], &err);
			if (!err)
			if (GetPolyPoints((LPPICKDATAHEADER)&PickList[item], FALSE, &np, &hPoly))
			{
				pPoints = GlobalLock(hPoly);
				if (GetPerpendicularOffsetToPoly(&pt, np, pPoints, &IntPoint, &OffDist, &PolyDist, 0))
				{
					dpointtoa(OutLoc, &IntPoint);
					GSSiGlobUlFree(&hPoly);
					goto Rtnl;
				}
				GSSiGlobUlFree(&hPoly);
			}
			goto RtnFalse;
		}
		case 1901: //$CONVERTTEXTPOINTERS ()
        {   
			hMem = GSSiGlobAlloc (1232,GMEM_MOVEABLE,3*2048);
			Arg1 = GlobalLock(hMem); 
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			if (ConvertTextPointers (CurView->hWnd))
		    	goto RtnTrue;
		    goto RtnFalse;
        }

		case 2001: //$GETINTERSECTIONCOORD
		{
			nArgs = GetFunArgs(Args, Arg, -6, &hMem, pBrkPt, bpOffset, bpLen);
			if (!nArgs)
			{
				DLGPROC lpfnLOC_INTERSECTMsgProc;  

				lpfnLOC_INTERSECTMsgProc = MakeProcInstance((DLGPROC)LOC_INTERSECTMsgProc, hInst);
				rtn = DialogBox(hInst, (LPSTR)"LOC_INTERSECT", CurView->hWnd, lpfnLOC_INTERSECTMsgProc);
				FreeProcInstance(lpfnLOC_INTERSECTMsgProc);  
				if (rtn)
					ExecutePointLocationMacro (UserSpecifiedBasePoint,0);
			}
			else
			{
				ExpandText (Arg[1]);
				ExpandText (Arg[2]);  
				ExpandText (Arg[3]); 
				ExpandText (Arg[4]);  
				ExpandText (Arg[5]); 
				SetAddEditValues (Arg[1],Arg[2],Arg[3],Arg[4],Arg[5],Arg[6],0);
				*DestName = 0; 
				if (nArgs < 6)
				{
					  DLGPROC	lpfnADD_MATCH_EDITMsgProc; 

	//                  CreateDialog(hInst, (LPSTR)"ADD_MATCH_EDIT2", hWndMain, lpfnADD_MATCH_EDITMsgProc);
					  rtn = DialogBox(hInst, (LPSTR)"ADD_MATCH_EDIT2", hWndMain, (DLGPROC)ADD_MATCH_EDITMsgProc);
				}  
				else
				{
					  HWND	hWndDlg;

					  rtn = TRUE;
					  hWndDlg = CreateDialog(hInst, (LPSTR)"ADD_MATCH_EDIT2", hWndMain, (DLGPROC)ADD_MATCH_EDITMsgProc);
					  PostMessage (hWndDlg,WM_COMMAND,IDC_ISMODELESS,0);
	//                  nRc = DialogBox(hInst, (LPSTR)"ADD_MATCH_EDIT", hWndDlg, lpfnADD_MATCH_EDITMsgProc);
	//                  FreeProcInstance(lpfnADD_MATCH_EDITMsgProc);
				}  
			}
            goto Rtnrtn;
        }
  
		case 2002:	//$ADDCONTOURSPLITLINES(nmaxrowcol)
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			Bounds=atobounds (Arg[1],&Err);
			if (Err)
				goto RtnFalse;
			i = atoi (Arg[2]);
			rtn = AddContourSplitLines (&Bounds,i);
			goto Rtnrtn;
        
        case 2201: //$JOINLINESBETWEENPOINTS (SETUP,maxlinelength)
        		   // highlight list contains all lines to join
        		   // current picklist contains points to snap to 
        		   // current pickap contains snap tol
        		   // maxlinelength contains max length of line to join (no max if not set)
         		   //$JOINLINESBETWEENPOINTS (NEXT,reflistglobal,taglistglobal,pointlistglobal)
        		   // returns 0 if done or 1 if line returned
        		   // reflist global contains list of refnos linked in order of length (longest first)
        		   // point list global contains handle to point list
        {   
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			nlong = JoinLinesBetweenPoints (Arg[1],Arg[2],Arg[3],Arg[4]);
			ltoa (nlong,OutLoc,10);
			goto Rtnl;
        }

		case 2601: //$GETSTREETSEGSBETWEENPOINTS(
        {   
			nArgs = GetFunArgs(Args, Arg, 8, &hMem, pBrkPt, bpOffset, bpLen);
			nlong = GetStreetSegsBetweenPoints (Arg[1],Arg[2],Arg[3],Arg[4],Arg[5],Arg[6],Arg[7],Arg[8]);
			ltoa (nlong,OutLoc,10);
			goto Rtnl;
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
	GSSiGlobUlFree (&hMem);
	if (TraceOn)
	{
		hMem=GSSiGlobAlloc (1233,GMEM_MOVEABLE,4096);
		lpstr = GlobalLock (hMem);
		sprintf (lpstr,"GFV:%s",Args);
		GSSiTraceLev (lpstr,-1,2);
		GSSiGlobUlFree (&hMem);
	}
{
#if ENABLETRACE
GSSiExitProg (1350);
#endif
	return (l);
}
#if ENABLETRACE
}
#endif
}
int WhichCursor(HCURSOR hcurs)
{
	int rtn = 0;

	HCURSOR cursors[] = { LoadCursor(0, IDC_ARROW), LoadCursor(hInst, MAKEINTRESOURCE(IDC_CENTERPOINT_CURSOR)), LoadCursor(0, IDC_IBEAM), LoadCursor(0, IDC_WAIT), LoadCursor(0, IDC_CROSS), LoadCursor(0, IDC_UPARROW), LoadCursor(0, IDC_SIZENWSE), LoadCursor(0, IDC_SIZENESW), LoadCursor(0, IDC_SIZEWE), LoadCursor(0, IDC_SIZENS), LoadCursor(0, IDC_SIZEALL), LoadCursor(0, IDC_NO), LoadCursor(0, IDC_WAIT),
		LoadCursor(hInst, "ZOOMIN"), LoadCursor(hInst, "ZOOMOUT"), LoadCursor(hInst, "TOGGLEVIS"), LoadCursor(hInst, "SETTAG"), LoadCursor(hInst, "HANDMOVE1"), LoadCursor(hInst, "HANDMOVE2"), LoadCursor(hInst, "MOVE_CURSOR"), LoadCursor(hInst, "WINZOOM"), LoadCursor(hInst, "SHOW"), LoadCursor(hInst, "CURSOR_LOCKED"), LoadCursor(hInst, "PICK_CURSOR_AP"), LoadCursor(hInst, "PICK_CURSOR_NEAR"), LoadCursor(hInst, "DRAWING"),
		hLockedCursor, hPickAPCursor, hPickNearCursor, hDigCursor, hWaitCursor, hDrawingCursor, IDc_SIZENWSE, IDc_SIZE, IDc_SIZEWE, IDc_SIZENS, IDc_SIZENESW };

	for (int i = 0; i < (sizeof(cursors) / sizeof(HCURSOR)); i++)
	{
		if (hcurs == cursors[i])
			return i + 1;
	}
	return rtn;
}

HWND GetTargetWindow(void)
{
	HWND hWnd = 0;
	HFILE fid = GSSiOpenFile("c:\\temp\\targetwindow.bin", 0, OF_READ);
	BigRead(fid, &hWnd, sizeof(HWND));
	GSSiClose2 (&fid);
	return hWnd;
}
int GMDocument(int nArgs, LPSTR *Arg, LPSTR OutLoc)
{
	int rtn = FALSE;

	HBITMAP	hBitmap;
	HDIB32 hDib32;
	HDIB32 hDib24;
	RECT	ScreenRect, WindowRect;
	HWND	hWnd = GetDesktopWindow();
	HWND	hWndTarget = GetTargetWindow();
	HDC		hDC;
	time_t	systim;
	char	line[1024];
	char	DocDir[MAX_PATH];
	char	ParmFile[MAX_PATH];
	HFILE	fid;

	time(&systim);
	sprintf(DocDir, "[%%DL]GMDocumenter\\%s", Arg[3]);
	ExpandText(DocDir);

	if (!stricmp(Arg[1], "CLEAR"))
	{
		rtn = DeleteDirAndContents(DocDir);
	}
	else
	{
		char stepType[32];
		int  t = (int)systim;
		strcpy(stepType, Arg[1]);
		sprintf(ParmFile, "%s\\%li_%s.txt", DocDir, t, stepType);
		fid = GSSiOpenFile(ParmFile, 0, OF_CREATE);
		if (!stricmp(Arg[1], "CAPWINDOW"))
		{
			char capScreenFile[MAX_PATH];

			sprintf(capScreenFile, "%s\\%lli.png", DocDir, systim);
			ExpandText(capScreenFile);
			hDC = GetWindowDC(hWnd);
			GetWindowRect(hWndTarget,&WindowRect);
			GetClientRect(hWndTarget, &ScreenRect);
			ClientRectToScreenRect(hWndTarget, &ScreenRect);
			ScreenRect.top -= ScreenRect.top / 2;
			hBitmap = SaveScreen(hDC, ScreenRect);
			hDib32 = BitmapToDIB32(hBitmap);
			DeleteObject(hBitmap);
			ReleaseDC(hWnd, hDC);
			hDib24 = FreeImage_ConvertTo24Bits(hDib32);
			rtn = GM32SaveDIB(hDib24, capScreenFile, -1, 0);
			sprintf(line, "CAPWINDOW");
			fputstring(line, fid);
			sprintf(line, "%lli.png", systim);
			fputstring(line, fid);
			recttoa(line, ScreenRect);
			fputstring(line, fid);
			FreeImage_Unload(hDib24);
			FreeImage_Unload(hDib32);
			rtn = TRUE;
		}
		if (!stricmp(Arg[1], "CAPSCREEN"))
		{
			char capScreenFile[MAX_PATH];

			sprintf(capScreenFile, "%s\\%lli.png", DocDir, systim);
			ExpandText(capScreenFile);
			hDC = GetWindowDC(hWnd);
			GetClientRect(hWnd, &ScreenRect);
			hBitmap = SaveScreen(hDC, ScreenRect);
			hDib32 = BitmapToDIB32(hBitmap);
			DeleteObject(hBitmap);
			ReleaseDC(hWnd, hDC);
			hDib24 = FreeImage_ConvertTo24Bits(hDib32);
			rtn = GM32SaveDIB(hDib24, capScreenFile, -1, 0);
			sprintf(line, "CAPSCREEN");
			fputstring(line, fid);
			sprintf(line, "%lli.png", systim);
			fputstring(line, fid);
			recttoa(line, ScreenRect);
			fputstring(line, fid);
			FreeImage_Unload(hDib24);
			FreeImage_Unload(hDib32);
			rtn = TRUE;
		}
		if (!stricmp(Arg[1], "CAPCURSOR"))
		{
			POINT pt;
			CURSORINFO cursInfo;
			ICONINFOEX iconInfo;
			int iCursor;

			GetCursorPos(&pt);
			cursInfo.cbSize = sizeof(CURSORINFO);
			iconInfo.cbSize = sizeof(ICONINFOEX);
			GetCursorInfo(&cursInfo);
			iCursor = WhichCursor(cursInfo.hCursor);
			sprintf(line, "CAPCURSOR");
			fputstring(line, fid);
			pttoa(line, pt);
			fputstring(line, fid);
			itoa(iCursor, line, 10);
			fputstring(line, fid);
			rtn = iCursor;
		}
		if (!stricmp(Arg[1], "CAPSCRIPT"))
		{
			POINT pt;
			CURSORINFO cursInfo;
			ICONINFOEX iconInfo;
			int iCursor;

			GetCursorPos(&pt);
			cursInfo.cbSize = sizeof(CURSORINFO);
			iconInfo.cbSize = sizeof(ICONINFOEX);
			GetCursorInfo(&cursInfo);
			iCursor = WhichCursor(cursInfo.hCursor);
			sprintf(line, "CAPSCRIPT");
			fputstring(line, fid);
			pttoa(line, pt);
			fputstring(line, fid);
			fputstring("scriptname.png", fid);
			rtn = iCursor;
		}
		if (!stricmp(Arg[1], "CAPRIGHTCLICK"))
		{
			POINT pt;
			CURSORINFO cursInfo;
			ICONINFOEX iconInfo;
			int iCursor;

			GetCursorPos(&pt);
			cursInfo.cbSize = sizeof(CURSORINFO);
			iconInfo.cbSize = sizeof(ICONINFOEX);
			GetCursorInfo(&cursInfo);
			iCursor = WhichCursor(cursInfo.hCursor);
			sprintf(line, "CAPRIGHTCLICK");
			fputstring(line, fid);
			pttoa(line, pt);
			fputstring(line, fid);
			itoa(iCursor, line, 10);
			fputstring(line, fid);
			fputstring("rightclicking_140_116.png", fid);
			rtn = iCursor;
		}
		if (!stricmp(Arg[1], "CAPLEFTCLICK"))
		{
			POINT pt;
			CURSORINFO cursInfo;
			ICONINFOEX iconInfo;
			int iCursor;

			GetCursorPos(&pt);
			cursInfo.cbSize = sizeof(CURSORINFO);
			iconInfo.cbSize = sizeof(ICONINFOEX);
			GetCursorInfo(&cursInfo);
			iCursor = WhichCursor(cursInfo.hCursor);
			sprintf(line, "CAPLEFTCLICK");
			fputstring(line, fid);
			pttoa(line, pt);
			fputstring(line, fid);
			itoa(iCursor, line, 10);
			fputstring(line, fid);
			fputstring("LeftClicking_483_36.png", fid);
			rtn = iCursor;
		}
		if (!stricmp(Arg[1], "CAPPOINTER"))
		{
			POINT pt;
			CURSORINFO cursInfo;
			ICONINFOEX iconInfo;
			int iCursor;
			int which = atoi(Arg[2]);

			GetCursorPos(&pt);
			cursInfo.cbSize = sizeof(CURSORINFO);
			iconInfo.cbSize = sizeof(ICONINFOEX);
			GetCursorInfo(&cursInfo);
			iCursor = WhichCursor(cursInfo.hCursor);
			sprintf(line, "CAPPOINTER");
			fputstring(line, fid);
			pttoa(line, pt);
			fputstring(line, fid);
			itoa(iCursor, line, 10);
			fputstring(line, fid);
			switch (which)
			{
			default:
			case 1:
				fputstring("Pointer_136_3.png", fid);
				break;
			case 2:
				fputstring("Pointer_3_2.png", fid);
				break;
			case 3:
				fputstring("Pointer_4_134.png", fid);
				break;
			case 4:
				fputstring("Pointer_135_135.png", fid);
				break;
			}
			rtn = iCursor;
		}
		if (!strnicmp(Arg[1], "CAPKEYSTROKE", 12))
		{
			POINT pt;
			CURSORINFO cursInfo;
			ICONINFOEX iconInfo;
			int iCursor;
			LPSTR pChar = strchr(Arg[1], '-');
			char key[2] = { 0 };

			if (pChar)
			{
				pChar++;
				key[0] = *pChar;
			}
			GetCursorPos(&pt);
			cursInfo.cbSize = sizeof(CURSORINFO);
			iconInfo.cbSize = sizeof(ICONINFOEX);
			GetCursorInfo(&cursInfo);
			iCursor = WhichCursor(cursInfo.hCursor);
			sprintf(line, "CAPKEYSTROKE");
			fputstring(line, fid);
			pttoa(line, pt);
			fputstring(line, fid);
			itoa(iCursor, line, 10);
			fputstring(line, fid);
			fputstring(key, fid);
			fputstring("KeyStroke_490_152-152_15.png", fid);
			rtn = iCursor;
		}
		GSSiClose2 (&fid);
	}
	ltoa(systim, OutLoc, 10);
	return rtn;
}
