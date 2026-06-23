#include "graphint.h"   
#include "extrndb.h"
#include "client.h"
#include <vfw.h>
#include <shlobj.h>
#include <shlwapi.h>

#include "gmextern.h"

static	char	MonthAbv[12][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static short	nSetVals=0;
static UINT	lSetVals=0;
static char	CurrentDialogType[16];
static char	DOW[7][10]={"SUNDAY","MONDAY","TUESDAY","WEDNESDAY","THURSDAY","FRIDAY","SATURDAY"};
static HIGHLIGHTDATA	HighlightData;
static HANDLE hSaveVis = 0;

		struct tm      * sunrise (double lat, double lon, int year, int month, int day);
void DrawAlphaBlend (HWND hWnd, HDC hdcwnd);
void testGDIP(HDC hdc);
LPSTR priorchr(LPSTR pstr, char c)
{
	while (*pstr != c)
		pstr--;

	return pstr;
}
BOOL CheckStructType(HANDLE hStruct, int type)
{
	BOOL rtn = FALSE;

	if (hStruct)
	{
		SIZE_T l = GlobalSize(hStruct);
		if (l > 0)
		{
			LPINT pType = GlobalLock(hStruct);
			if (pType)
			{
				if (*pType == type)
					rtn = TRUE;
				GlobalUnlock(hStruct);
			}
		}
	}
	if (!rtn)
	{
		char mess[256];
		sprintf(mess,"Invalid structure %i", type);
		MessageBox(0, mess, 0, MB_ICONEXCLAMATION);
	}
	return rtn;
}
void GetMassShapeFiles(void)
{
	char file[] = "c:\\temp\\maparcels2.txt";
	int lfile = GSSiLength(file), i, i2;
	HANDLE hFile = GSSiGlobAlloc(GAIDNO 1990, GMEM_MOVEABLE, lfile + 1);
	LPSTR pFile = GlobalLock(hFile);
	//HANDLE hFile2 = GSSiGlobAlloc(GAIDNO 0, GMEM_MOVEABLE, lfile + 1);
	//LPSTR pFile2 = GlobalLock(hFile2);
	LPSTR pEnd, pBeg;
	char outfile[] = "c:\\temp\\ma_shape_files.txt";
	HFILE fid = GSSiOpenFile(file, 0, OF_READ);

	BigRead(fid, pFile, lfile);
	//lfile /= 2;
	pEnd = pFile;
	//for (i = 0, i2 = 0; i < lfile; i++, i2 += 2)
	//	pFile[i] = pFile2[i2];
	pFile[lfile] = 0;
	lfile = strlen(pFile);
	pBeg = strstr(pEnd, "/L3_SHP_");
	while (pBeg)
	{
		LPSTR pStart;
		pStart = priorchr(pBeg, '"');
		pEnd = strchr(pBeg, '"');
		pStart++;
		*pEnd++ = 0;
		AppendFile(outfile, pStart);
		pBeg = strstr(pEnd, "/L3_SHP_");
	}
	GSSiGlobUlFree(&hFile);
	//GSSiGlobUlFree(&hFile2);
	GSSiClose2 (&fid);
	return;
}

void ReopenFTP (LPFTPSTRUCT pFTPStruct)
{
	FTPClose (pFTPStruct->hFTP);
	pFTPStruct->hFTP = FTPOpen (pFTPStruct->ServerName,
								pFTPStruct->Username,
								pFTPStruct->Password,
								pFTPStruct->directory,
								pFTPStruct->errorVarName,
								pFTPStruct->port,
								pFTPStruct->passive);
	return;
}

int	GetFunctionValue1(int FunID, LPSTR Args, LPSTR OutLoc, LPBREAKPOINT pBrkPt, int bpOffset, int bpLen)
#if ENABLETRACE
{GSSiEnterProg (1348);
#endif
{   HANDLE	hMem=0,hDLT,hSurf;
	LPSTR	Arg1, Arg2, Arg3, Arg4, Arg5, Arg6,Arg7, ParLoc, lpstr, lpstrb;  
	BOOL	FromLimits, Immediate; 
	LPSTR	pEnd, pCR, pFile, Arg[20] = { 0 }, pVal;
	short	nArgs,layer;
	HFILE	Fid1, Fid2, Fid3;   
	int		l, CvtDir, year,i, idesc;    
	DPOINT	Point, Point2;
	POINT	Point16;
	RECT	Rect; 
	MNMXCORD	Bounds,Bounds2; 
	MNMXCORL	GridBounds;
	long	hNum, Refno, nlong;
	LPSTR	lpColon;
	time_t	systime; 
	BOOL	TORF, rtn, Err, PickVis;  
	LPVIEWPORT	SaveVP=CurView;  
	long	SaveConfigID = ConfigID; 
	short	SaveCfg = CurrentConfig;
	short SymNum, Mode, irc, nRc;			
	short	n, n1,n2,pos, nrem;  
	long	ICmd;
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
	LPHIGHLIGHTDATA	pHighlightData;

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
		hMem=GSSiGlobAlloc(GAIDNO 785,GMEM_MOVEABLE,4096);
		lpstr = GlobalLock (hMem);
		sprintf (lpstr,"GFV:%s(%s)",LastFunctionName,Args);
		GSSiTraceLev (lpstr,1,2);
		GSSiGlobUlFree (&hMem);
	}
	switch (FunID)
	{    
		case 301: // $RND(numval,decplace,(optional TF add commas),(opt TF remove training zeros and decimal)) rounds numeric values 
				  // if decplace < 0 rounds to nearest pow of 10 (i.e $RND(12345,-3) is 12000
				  
		{	
			char	Format[16]; 
			BOOL	AddC=FALSE;
			BOOL	removeZ=FALSE;
			
			nArgs = GetFunArgs (Args,Arg,4,&hMem, pBrkPt, bpOffset, bpLen); 
			AddC = atob (Arg[3]);
			removeZ = atob (Arg[4]);
			ndec = atoi (Arg[2]);
			RVal = atof (Arg[1]);
			if (ndec >= 0)
			{
				sprintf (Format,"%%.%ilf",ndec);
				sprintf (OutLoc,Format,RVal);  
			}
			else
			{   
				ndec = -ndec;
				RVal = RVal / pow (10,ndec);
				sprintf (OutLoc,"%.0lf",RVal);
				RVal = atof (OutLoc) * pow(10,ndec);
				sprintf (OutLoc,"%.0lf",RVal);
			}
			if (removeZ && strchr (OutLoc,'.'))
			{
				int l = strlen (OutLoc);
				while (l)
				{
					l--;
					if (OutLoc[l] == '0')
					{
						OutLoc[l] = 0;
						continue;
					}
					else if (OutLoc[l] == '.')
					{
						OutLoc[l] = 0;
						break;
					}
					break;
				}
			}
			if (AddC)
				AddCommas (OutLoc);
			goto Rtnl;
		}
		case 302: /* $CMA(numval) Adds commas to numeric values*/ 
		{	double	rval;
			int		ndec, lc, ntoadd; 
			char	Format[16];
			
			hMem = GSSiGlobAlloc(GAIDNO 795,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			if (!rread (Arg1,&rval,&ndec)) goto Rtn0;
			
			sprintf (Format,"%%.%ilf",ndec);
			sprintf (OutLoc,Format,rval); 
			l = _fstrlen(OutLoc); 
			if (ndec)
				lc = l - (ndec + 1);
			else
				lc = l;
			ntoadd = (lc-1)/3;
			
			if (ntoadd)
			{
				LPSTR	lpOut1, lpOut2;
				int		n=3; 
				
				lpOut1 = OutLoc + l;
				lpOut2 = lpOut1 + ntoadd;
				if (ndec)
					n += ndec + 1;
				while (lpOut1 != lpOut2)
				{   
					*lpOut2-- = *lpOut1--;
					if (!n)
					{
						*lpOut2-- = ',';
						n = 2;
					}
					else
						n--;
				}
			}
			goto Rtnl;
		}
		case 303: /* $OPV(view) returns opposite view */
		{	double	rval; 
			int		iview;
			int		ndec, ntoadd; 
			
			hMem = GSSiGlobAlloc(GAIDNO 796,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			if (!rread (Arg1,&rval,&ndec)) goto Rtn0;
			iview = IDNINT(rval); 
//			iview = OppositeView(iview); 
			itoa (iview,OutLoc,10);

			goto Rtnl;
		}
		case 304: /* $RUN(runid) returns run date */
		{				
			hMem = GSSiGlobAlloc(GAIDNO 797,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			GetRunDate (Arg1,OutLoc);

			goto Rtnl;
		}
		case 305: /* $CAL(time,format) returns date in specified format*/  
				 //also sets globals %YEAR, %MONTH, %CMONTH, %MDAY, %WDAY, %YEAR1900
		{	double	rval;
			int		iformat; 
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			iformat = atoi(Arg[2]);
			if (iformat == 11)//hh:mm:ss
			{
				int	input = IDNINT(atof(Arg[1]));
				int hr = input/3600;
				int mn = (input - hr * 3600) / 60;
				int sec = input - hr * 3600 - mn * 60;

				sprintf (OutLoc,"%2.2i:%2.2i:%2.2i",hr,mn,sec);
				goto Rtnl;
			}
			systime = max (0,IDNINT(atof(Arg[1])));
			if (systime < 0)
				goto RtnFalse;
			tmtime = *localtime (&systime); 
			switch (iformat)
			{  
				default:
				case 2:			        
					_fstrcpy (OutLoc,ctime(&systime));
					*_fstrchr(OutLoc,'\n') = 0;   
					break;
				case 1:
					year = tmtime.tm_year + 1900;
					sprintf (OutLoc,"%i/%i/%2.2i",tmtime.tm_mon+1,tmtime.tm_mday,year);
					break;      
				case 8: //ODBC format 1999-12-15 00:00:00.000
					sprintf (OutLoc,"%i-%2.2i-%2.2i %2.2i:%2.2i:%2.2i.000",tmtime.tm_year+1900,tmtime.tm_mon+1,tmtime.tm_mday,
													  tmtime.tm_hour,tmtime.tm_min,tmtime.tm_sec);
					break;
				case 4://ORACLE day format 23-MAR-1949
					sprintf (OutLoc,"%2.2i-%s-%i",tmtime.tm_mday,_fstrupr(MonthAbv[tmtime.tm_mon]),tmtime.tm_year+1900);
					break;      
				case 6://MM-DD-YYYY format
					sprintf (OutLoc,"%2.2i-%2.2i-%i",tmtime.tm_mon+1,tmtime.tm_mday,tmtime.tm_year+1900);
					break;
				case 7://14-Aug-2011 10:08:38 AM
				{
					char	AMPM[4]="AM";
					int		hr = tmtime.tm_hour;

					if (hr > 12)
					{
						hr -= 12;
						strcpy (AMPM,"PM");
					}
					else if (hr == 12)
						strcpy (AMPM,"PM");
					else if (hr < 1)
						hr = 12;
					sprintf (OutLoc,"%2.2i-%s-%i %2.2i:%2.2i:%2.2i %s",tmtime.tm_mday,MonthAbv[tmtime.tm_mon],tmtime.tm_year+1900,
																	   hr,tmtime.tm_min,tmtime.tm_sec,AMPM);
				}
					break;
				case 3: //ODBC format w/o seconds 1999-12-15 00:00
					sprintf(OutLoc, "%i-%2.2i-%2.2i %2.2i:%2.2i", tmtime.tm_year + 1900, tmtime.tm_mon + 1, tmtime.tm_mday,
						tmtime.tm_hour, tmtime.tm_min);
					break;
			}

			goto Rtnl;
		}
		case 311: /* $CLK(datetime,format(optional-defaults to 1) returns systime (seconds) from specified format*/ 
		{	double	rval;
			int		iformat; 
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			Arg1 = Arg[1];
			iformat = atoi(Arg[2]);
			switch (iformat)
			{   
				default:
				case 1: /* ODBC format  1996-12-01 08:48:00.000 */  
                    _fmemset (&tmtime,0,sizeof(tmtime));  
                    if (!(lpstr=_fstrchr (Arg1,'-'))) break; 
                    *lpstr++=0; 
                    tmtime.tm_year = atoi (Arg1);
                    if (tmtime.tm_year < 70)
                    	tmtime.tm_year += 100;
                    else if (tmtime.tm_year >= 1900)
                    	tmtime.tm_year -= 1900;
                    lpstrb = lpstr;
                    tmtime.tm_mon = atoi (lpstrb)-1; 
                    if (!(lpstr=_fstrchr (lpstrb,'-'))) break; 
                    *lpstr++=0;
                    lpstrb = lpstr;
                    tmtime.tm_mday = atoi (lpstrb); 
                    if (!(lpstr=_fstrchr (lpstrb,' '))) break; 
                    *lpstr++=0; 
                    lpstrb = lpstr;
                    tmtime.tm_hour = atoi (lpstrb); 
                    if (!(lpstr=_fstrchr (lpstrb,':'))) break; 
                    *lpstr++=0;
                    lpstrb = lpstr;
                    tmtime.tm_min = atoi (lpstrb); 
                    if (!(lpstr=_fstrchr (lpstrb,':'))) break; 
                    *lpstr++=0;
                    lpstrb = lpstr;  
                    tmtime.tm_sec = atof (lpstrb);
                    break;
                    
             case 2: // mmddyyyyhhmmss 
                    _fmemset (&tmtime,0,sizeof(tmtime));  
                    if (_fstrlen (Arg1) != 14)
                    	break; 
                    tmtime.tm_year = ldread (&Arg1[4],4);
                    if (tmtime.tm_year < 70)
                    	tmtime.tm_year += 100;
                    else if (tmtime.tm_year >= 1900)
                    	tmtime.tm_year -= 1900;
                    tmtime.tm_mon = ldread (Arg1,2)-1; 
                    tmtime.tm_mday = ldread (&Arg1[2],2); 
                    tmtime.tm_hour = ldread (&Arg1[8],2);  
                    tmtime.tm_min = ldread (&Arg1[10],2);  
                    tmtime.tm_sec = ldread (&Arg1[12],2);  
                    break;
             case 3: // yyyymmddhhmmss 
                    _fmemset (&tmtime,0,sizeof(tmtime));  
                    if (_fstrlen (Arg1) != 14)
                    	break; 
                    tmtime.tm_year = ldread (&Arg1[0],4);
                    if (tmtime.tm_year < 70)
                    	tmtime.tm_year += 100;
                    else if (tmtime.tm_year >= 1900)
                    	tmtime.tm_year -= 1900;
                    tmtime.tm_mon = ldread (&Arg1[4],2)-1; 
                    tmtime.tm_mday = ldread (&Arg1[6],2); 
                    tmtime.tm_hour = ldread (&Arg1[8],2);  
                    tmtime.tm_min = ldread (&Arg1[10],2);  
                    tmtime.tm_sec = ldread (&Arg1[12],2);  
                    break;
             case 4: // yymmdd hh:mm:ss 
                    _fmemset (&tmtime,0,sizeof(tmtime));  
                    if (_fstrlen (Arg1) != 15)
                    	break; 
                    tmtime.tm_year = ldread (&Arg1[0],2);
                    if (tmtime.tm_year < 70)
                    	tmtime.tm_year += 100;
                    else if (tmtime.tm_year >= 1900)
                    	tmtime.tm_year -= 1900;
                    tmtime.tm_mon = ldread (&Arg1[2],2)-1; 
                    tmtime.tm_mday = ldread (&Arg1[4],2); 
                    tmtime.tm_hour = ldread (&Arg1[7],2);  
                    tmtime.tm_min = ldread (&Arg1[10],2);  
                    tmtime.tm_sec = ldread (&Arg1[13],2);  
                    break;
				case 5: // 01/02/05 08:48:00.000 
						// or
						// 01/02/2005 08:00:000
                    _fmemset (&tmtime,0,sizeof(tmtime));  
                    if (!(lpstr=_fstrchr (Arg1,'/'))) break; 
                    *lpstr++=0; 
                    tmtime.tm_mon = atoi (Arg1)-1;
                    tmtime.tm_mday = atoi (lpstr); 
                    if (!(lpstr=_fstrchr (lpstr,'/'))) break; 
                    lpstr++;
                    tmtime.tm_year = atoi (lpstr); 
                    if (tmtime.tm_year < 70)
                    	tmtime.tm_year += 100;
                    else if (tmtime.tm_year >= 1900)
                    	tmtime.tm_year -= 1900;
                    if (!(lpstr=_fstrchr (lpstr,' '))) break; 
                    *lpstr++=0;
                    tmtime.tm_hour = atoi (lpstr); 
                    if (!(lpstr=_fstrchr (lpstr,':'))) break; 
                    *lpstr++=0;
                    tmtime.tm_min = atoi (lpstr); 
                    if (!(lpstr=_fstrchr (lpstr,':'))) break; 
                    *lpstr++=0;
                    tmtime.tm_sec = atof (lpstr);
                    break;

				case 6: /* Image format  1996:12:01 08:48:00.000 */  
                    _fmemset (&tmtime,0,sizeof(tmtime));  
                    if (!(lpstr=_fstrchr (Arg1,':'))) break; 
                    *lpstr++=0; 
                    tmtime.tm_year = atoi (Arg1);
                    if (tmtime.tm_year < 70)
                    	tmtime.tm_year += 100;
                    else if (tmtime.tm_year >= 1900)
                    	tmtime.tm_year -= 1900;
                    lpstrb = lpstr;
                    tmtime.tm_mon = atoi (lpstrb)-1; 
                    if (!(lpstr=_fstrchr (lpstrb,':'))) break; 
                    *lpstr++=0;
                    lpstrb = lpstr;
                    tmtime.tm_mday = atoi (lpstrb); 
                    if (!(lpstr=_fstrchr (lpstrb,' '))) break; 
                    *lpstr++=0; 
                    lpstrb = lpstr;
                    tmtime.tm_hour = atoi (lpstrb); 
                    if (!(lpstr=_fstrchr (lpstrb,':'))) break; 
                    *lpstr++=0;
                    lpstrb = lpstr;
                    tmtime.tm_min = atoi (lpstrb); 
                    if (!(lpstr=_fstrchr (lpstrb,':'))) break; 
                    *lpstr++=0;
                    lpstrb = lpstr;  
                    tmtime.tm_sec = atof (lpstrb);
                    break;
                    
				case 7: //14-Aug-2011 10:08:38 AM
				{
					BOOL	isPM=FALSE;

					if ((lpstr=strrchr (Arg1,' ')))
					{
						if (!stricmp (lpstr," PM"))
							isPM = TRUE;
					}
                    _fmemset (&tmtime,0,sizeof(tmtime));  
                    if (!(lpstr=_fstrchr (Arg1,'-'))) break; 
                    *lpstr++=0; 
                    tmtime.tm_mday = atoi (Arg1); 
                    lpstrb = lpstr;
					for (tmtime.tm_mon=0;tmtime.tm_mon<12;tmtime.tm_mon++)
						if (!strnicmp (lpstrb,MonthAbv[tmtime.tm_mon],3))
							break;
                    if (!(lpstr=_fstrchr (lpstrb,'-'))) break; 
                    *lpstr++=0;
                    tmtime.tm_year = atoi (lpstr);
                    if (tmtime.tm_year < 70)
                    	tmtime.tm_year += 100;
                    else if (tmtime.tm_year >= 1900)
                    	tmtime.tm_year -= 1900;
                    if (!(lpstr=_fstrchr (lpstr,' '))) break; 
                    *lpstr++=0; 
                    lpstrb = lpstr;
                    tmtime.tm_hour = atoi (lpstrb); 
					if (isPM && tmtime.tm_hour < 12)
						tmtime.tm_hour += 12;
					else if (!isPM && tmtime.tm_hour > 11)
						tmtime.tm_hour -= 12;
                    if (!(lpstr=_fstrchr (lpstrb,':'))) break; 
                    *lpstr++=0;
                    lpstrb = lpstr;
                    tmtime.tm_min = atoi (lpstrb); 
                    if (!(lpstr=_fstrchr (lpstrb,':'))) break; 
                    *lpstr++=0;
                    lpstrb = lpstr;  
                    tmtime.tm_sec = atof (lpstrb);
				}
                    break;
                    
				case 11:
                    _fmemset (&tmtime,0,sizeof(tmtime));  
                    if ((lpstr=_fstrchr (Arg1,'-')))
					{
						*lpstr++=0; 
						tmtime.tm_year = atoi (Arg1);
						if (tmtime.tm_year < 70)
                    		tmtime.tm_year += 100;
						else if (tmtime.tm_year >= 1900)
                    		tmtime.tm_year -= 1900;
						lpstrb = lpstr;
						tmtime.tm_mon = atoi (lpstrb)-1; 
						if (!(lpstr=_fstrchr (lpstrb,'-'))) break; 
						*lpstr++=0;
						lpstrb = lpstr;
						tmtime.tm_mday = atoi (lpstrb); 
						if (!(lpstr=_fstrchr (lpstrb,' '))) break; 
						*lpstr++=0; 
					}
					else
						lpstr = Arg1;
                    lpstrb = lpstr;
                    tmtime.tm_hour = atoi (lpstrb); 
                    if (!(lpstr=_fstrchr (lpstrb,':'))) break; 
                    *lpstr++=0;
                    lpstrb = lpstr;
                    tmtime.tm_min = atoi (lpstrb); 
                    if ((lpstr=_fstrchr (lpstrb,':')))
					{
						*lpstr++=0;
						lpstrb = lpstr;  
						tmtime.tm_sec = atof (lpstrb);
					}
					if ((lpstr=strrchr (lpstrb,' ')))
					{
						lpstr++;
						if (!strnicmp (lpstr,"PM",2))
							tmtime.tm_hour += 12;
					}
					systime = tmtime.tm_hour * 3600 + tmtime.tm_min * 60 + tmtime.tm_sec;
					ltoa ((long)systime,OutLoc,10);
					goto Rtnl;
                    break;
			}
               
    		tmtime.tm_isdst = -1;
    		if (!tmtime.tm_mday)
    			tmtime.tm_mday = 1;
    		systime = mktime (&tmtime);  
			ltoa ((long)systime,OutLoc,10);
			goto Rtnl;

		}
		case 306: /* $HLT(CLEAR) clears highlight list*/
				  // $HLT(ROUTE,routefilepathname,REFNO,display(YNTF10));
				  // $HLT(CONSTAT,constatfile,refno,dir(BorE),track(LorR),VPName(opt)) 
		{	   
			
			
			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
			if (!_fstrcmp(Arg[1],"COUNT"))
			{
				nlong = BT_NUM_IN_INDEX (hHighlight);
				ltoa (nlong,OutLoc,10); 
				goto Rtnl;
			}
			if (!_fstrcmp(Arg[1],"SELECT"))
			{
				if (!_fstrcmp(Arg[2],"FIRST"))
					pos = BT_FIRST; 
				else
					pos = BT_NEXT;
				if (BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData)) 
					goto RtnFalse; 
				PickList[0]=HighlightData.PD;
			    SetPickGlobals (0); 
				goto RtnTrue;
			}
			if (!_fstrcmp(Arg[1],"DELETE"))
			{
				if (DeleteHighlightedItems (GF_DELETE_HIGHLIGHTED_ITEMS)) 
					goto RtnTrue;
				goto RtnFalse;
			} 
			if (!_fstrcmp(Arg[1],"CLEAR"))
			{
				ClearHighlightList(FALSE); 
				goto RtnTrue;
			} 
			
			if (!_fstricmp (Arg[1],"DUMP"))
			{   
				char	cref[12];
				
				pos = BT_FIRST;
				Fid1 = GSSiOpenFile (Arg[2],0,OF_CREATE);
				while (!BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,Arg[6])) 
				{
					pos = BT_NEXT; 
					ltoa (Refno,cref,10);
					fputstring (cref,Fid1);
				}
				GSSiClose2 (&Fid1);
				goto RtnTrue;
			}   
			if (!_fstrcmp(Arg[1],"VIEWPORT"))//$HLT(VIEWPORT,vpname(opt),exclusionrect(opt))
			{
				int saveType;
				SetCurView ( SetVPFromName (Arg[2],&Err));
				saveType = CurView->Type;
				if (CurView->Type == SUBVIEWPORT)
					CurView->Type = PLANVIEWPORT;
				if (strlen(Arg[3]))
				{
					BOOL err;
					ExclusionBounds = atobounds(Arg[3], &err);
					if (!err)
						haveExclusionBounds = TRUE;		 
				}
				nlong = HighlightInArea (CurView->hWnd,&CurView->WBounds,TRUE,TRUE,CurView->hMaskArea);
				ltoa (nlong,OutLoc,10); 
				CurView->Type = saveType;
				CurView = SaveVP; 
				haveExclusionBounds = FALSE;
				goto Rtnl;
			} 
			if (!_fstrcmp(Arg[1],"AREA"))
			{
				SetCurView ( SetVPFromName (Arg[2],&Err));
				nlong = HighlightInArea (CurView->hWnd,0,TRUE,TRUE,0);
				ltoa (nlong,OutLoc,10); 
				CurView = SaveVP; 
				goto Rtnl;
			} 
			if (!_fstrcmp(Arg[1],"EDLIM"))
			{
				SetCurView ( SetVPFromName (Arg[2],&Err));
    			if (GetLayerBounds (&Bounds,CurView->hDC, CurView->UpdateFile-1)) 
    			{
					HighlightInArea (CurView->hWnd,&Bounds,TRUE,TRUE,0);
					CurView = SaveVP; 
					goto RtnTrue;   
				}
				else
				{
					CurView = SaveVP; 
	   			 	goto RtnFalse;
				}
			} 
			if (!_fstrcmp(Arg[1],"UNHLT"))
			{   
				long	StartRef=LONG_MIN;
				HANDLE	hHlt = GSSiGlobAlloc(GAIDNO 800,GMEM_MOVEABLE,2*sizeof(HIGHLIGHTDATA));
				LPHIGHLIGHTDATA	pHighlightData = (LPHIGHLIGHTDATA)GlobalLock (hHlt); 
				LPHIGHLIGHTDATA pSaveHLTData = pHighlightData + 1;
				
				pSaveHLTData->PD = PickList[0];

				while (!BT_FIND (hHighlight,(LPSTR)&StartRef,BT_FIRST,BT_GT,(LPSTR)pHighlightData))
				{
					PickList[0]=pHighlightData->PD;
				    RemoveFromHighlightList (StartRef,2);
			    	ShowPickedItem (CurView->hWnd,0); 
				    RemoveFromHighlightList (StartRef,0);
			    	ShowPickedItem (CurView->hWnd,0); 
				}
				ClearHighlightList(FALSE); 
				PickList[0] = pSaveHLTData->PD;  
				GSSiGlobUlFree (&hHlt);
				goto RtnTrue;
			} 
			if (!_fstrcmp(Arg[1],"BOUNDS"))
			{   
				Bounds = atobounds (Arg[2],&Err);
				if (Err)
					goto RtnFalse;
				nlong = HighlightInArea (CurView->hWnd,&Bounds,TRUE,TRUE,0);
				ltoa (nlong,OutLoc,10); 
				goto Rtnl;
			} 
			if (!_fstrcmp(Arg[1],"COPY")) //delete existing,showstatus(opt),newrefs(opt),TranFile(opt),ConvertCurvesToPolylines
			{   
				BOOL	SaveAllowCache = AllowCache;
				BOOL	closeTran = TRUE;
				AllowCache = FALSE;
				FromLimits = atob (Arg[2]);//delete existing highlighted items
				Immediate = atob (Arg[3]);//show status option
				if (IsInteger(Arg[5]) && atoi(Arg[5]) > 0)
				{
					hTran = (HANDLE)atoi(Arg[5]);
					closeTran = FALSE;
				}
				else if (*Arg[5])
					hTran=LoadTranFileWithDandT(Arg[5]);
				else
					hTran = 0;
				rtn = CopyHighlightedRecords (FromLimits,Immediate,atob(Arg[4]),hTran,atob(Arg[6]));
				if (closeTran)
					CloseTRANS2 (&hTran);
				AllowCache = SaveAllowCache;
				if (rtn)
					goto RtnTrue;
				goto RtnFalse;
			}
             
			if (!_fstricmp(Arg[1],"ITEM"))
			{
				short	UsePickList=-1;

				nlong = 0;
				Refno = 0;
				if (atob(Arg[5]))
					UsePickList = -101;
				if ((lpColon = _fstrchr (Arg[2],':')))
					*lpColon++=0;
				else 
				{
					Refno = atol (Arg[2]); 
					Arg[2] = 0;
				}
				SetCurView(SetVPFromName(Arg[4], &Err));
	            if (!PickByRefno (Refno,Arg[2],lpColon,UsePickList))
	            {
					CurView = SaveVP; 
				}
				else 
				{
			    	nlong = AddToHighlightList (PickList[0].Refno,&PickList[0],TRUE);
			    	SetPickGlobals (0); 
			    	if (atob (Arg[3]))
   				    	ShowPickedItem (CurView->hWnd,0); 
					CurView = SaveVP; 
				}
				ltoa (nlong,OutLoc,10); 
				goto Rtnl;
			}
			if (!_fstricmp(Arg[1],"PICKED"))
			{
				hNum = atol (Arg[2]);
			    nlong = AddToHighlightList (PickList[hNum-1].Refno,&PickList[hNum-1],TRUE);
		    	if (atob (Arg[3]))
			    	ShowPickedItem (CurView->hWnd,(short)(hNum-1)); 
				ltoa (nlong,OutLoc,10); 
				goto Rtnl;
			}
			if (!_fstricmp(Arg[1],"SAVE"))
			{
				if (!hHighlight) 
					goto RtnFalse;
	            if (CopyHighlightList (Arg[2]))
					goto RtnTrue;
				goto RtnFalse;
			}
			if (!_fstricmp(Arg[1],"SAVETODB"))
			{
				if (!hHighlight) 
					goto RtnFalse;
	            if (CopyHighlightListToDB (Arg[2]))
					goto RtnTrue;
				goto RtnFalse;
			}
			if (!_fstricmp(Arg[1],"RECALL"))
			{
	            if (RecallHighlightList (Arg[2]))
					goto RtnTrue;
				goto RtnFalse;
			}
			if (!_fstricmp(Arg[1],"REFNO")) 
			{
				Refno = atol (Arg[2]);
				if (!atob(Arg[3]))
				{
					if (!PickByRefno (Refno,0,0,-1))
						goto RtnFalse;
				}
			    nlong = AddToHighlightList (Refno,&PickList[0],TRUE);
				ltoa (nlong,OutLoc,10); 
				goto Rtnl;
			}
			if (!_fstricmp(Arg[1],"MACRO"))
			{
				if (nArgs < 3) goto Rtn0;
				hNum = atol (Arg[3]);
				if (!RunHltMacro (Arg[2],hNum,Arg[4]))
					goto RtnFalse;
				else
					goto RtnTrue;
			}
			if (!_fstricmp(Arg[1], "ROUTE"))
			{
				if (nArgs < 4) goto Rtn0;
				Refno = atol(Arg[3]);
				if (!HighlightRoute(Arg[2], Refno, 0, atob(Arg[3])))
					goto RtnFalse;
				else
					goto RtnTrue;
			}
			if (!_fstricmp(Arg[1], "REFSTOTEXT"))
			{
				int nWritten = 0;
				if (nArgs < 2) goto Rtn0;
				HFILE Fid = GSSiOpenFile(Arg[2], 0, OF_CREATE);
				if (Fid == HFILE_ERROR)
					goto RtnFalse;
				HANDLE hLine = GSSiGlobAlloc(GAIDNO 1877, GMEM_MOVEABLE, 1024);
				LPSTR pLine = GlobalLock(hLine);
				sprintf(pLine, "REFNO\tPREFIX\tUDI\tTYPE\tDESC\tBOUNDS\tLENGTH\tBP\tEP");
				fputstring(pLine, Fid);
				int pos = BT_FIRST;
				while (!BT_FIND(hHighlight, (LPSTR)&Refno, pos, BT_ANY, (LPSTR)&HighlightData))
				{
					pos = BT_NEXT;
					sprintf(pLine, "%i\t%s\t%s\t%i\t%i\t%lf %lf %lf %lf	%lf\t%lf %lf\t%lf %lf", HighlightData.PD.Refno, HighlightData.PD.Prefix, HighlightData.PD.UDI, HighlightData.PD.Type, HighlightData.PD.Desc, HighlightData.PD.Rect.xmn, HighlightData.PD.Rect.ymn, HighlightData.PD.Rect.xmx, HighlightData.PD.Rect.ymx, HighlightData.PD.Length,
						HighlightData.PD.BeginPoint.x, HighlightData.PD.BeginPoint.y, HighlightData.PD.EndPoint.x, HighlightData.PD.EndPoint.y);
					fputstring(pLine, Fid);
					nWritten++;
				}
				GSSiClose(Fid);
				GSSiGlobUlFree(&hLine);
				itoa(nWritten, OutLoc, 10);
				goto Rtnl;
			}

			if (!_fstricmp(Arg[1], "REFSTOGMD"))//$HLT(REFSTOGMD,outfile)
			{
				char	DefStr[] = "REFNO(B4),PREFIX(C8),UDI(C64),TAG(C72),TYPE(B4),SYMNUM(B4),SYMNAME(C66),MNX(R8),MNY(R8),MXX(R8),MXY(R8)";

				int nWritten = 0;
				typedef struct {
					long	Refno;
					char	Prefix[8];
					char	UDI[64];
					char	TAG[72];
					int		TYPE;
					int		DESC;
					char	SymName[66];
					double	MNX,MNY,MXX,MXY;
				}HLTREFSDATA;
				typedef HLTREFSDATA	FAR* LPHLTREFSDATA;
				LPHLTREFSDATA	pData;

				if (nArgs < 2) goto Rtn0;
				if (CreateGWDDatabase(Arg[2], 1, FALSE, 0, 2, DefStr))
				{
					HANDLE hOutFile = OpenGWDatabase(Arg[2], BT_WRITE);
					LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock(hOutFile);
					pData = (LPHLTREFSDATA)&lpGWDHead->GWDData;
					int pos = BT_FIRST;
					while (!BT_FIND(hHighlight, (LPSTR)&Refno, pos, BT_ANY, (LPSTR)&HighlightData))
					{
						pos = BT_NEXT;
						pData->Refno = Refno;
						strncpy(pData->Prefix, HighlightData.PD.Prefix, 8);
						strncpy(pData->UDI, HighlightData.PD.UDI, 64);
						sprintf(pData->TAG, "%s:%s", HighlightData.PD.Prefix, HighlightData.PD.UDI);
						pData->TYPE = HighlightData.PD.Type;
						pData->DESC = HighlightData.PD.Desc;
						GetDictSymName(HighlightData.PD.Desc, pData->SymName);
						pData->MNX = HighlightData.PD.Rect.xmn;
						pData->MNY = HighlightData.PD.Rect.ymn;
						pData->MXX = HighlightData.PD.Rect.xmx;
						pData->MXY = HighlightData.PD.Rect.ymx;
						GWDAddRecord(lpGWDHead, 0, 0);

						nWritten++;
					}
					GlobalUnlock(hOutFile);
					CloseGWDatabase(hOutFile);
				}
				itoa(nWritten, OutLoc, 10);
				goto Rtnl;
			}

			if (nArgs < 5) goto Rtn0;
			Offset = atof(Arg[3]);
			FromLimits = atoi(Arg[4]);
			Immediate = atoi(Arg[5]);
			
		//	ZoomToHltItem (hNum,Offset,FromLimits,Immediate);
			goto RtnTrue;
		} 
//		case 307: /* $ZMV(view) returns zoomed view */
/*		{	double	rval; 
			int		iview;
			int		ndec, ntoadd; 
			
			hMem = GSSiGlobAlloc(GAIDNO 801,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			if (!rread (Arg1,&rval,&ndec)) goto Rtn0;
			iview = IDNINT(rval); 
			iview = ZoomedView(iview); 
			itoa (iview,OutLoc,10);

			goto Rtnl;
		} */
		 
		case 308: /* $MID(string,start,end) returns portion of string - missing or neg start returns last end chars */
		{	double	rval;
			int		ndec, start, len, end; 
			LPSTR	lpEnd;
			
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto Rtn0;
			l = strlen (Arg[1]);
			start = IDNINT(FltAP (Arg[2],&Err));
			if (Err)
				goto Rtn0;
			if (*Arg[3] == 'L' || *Arg[3] == 'l')
				end = l;
			else
				end = IDNINT(FltAP (Arg[3],&Err));
			if (Err)
				goto Rtn0;
			if (!end)
				end = start; 
			if (end < 0)
				end = start + abs (end) - 1;
			l = _fstrlen(Arg[1]);
			end = min (end,l);
			if (start <= 0)
			{
				start = l - end + 1;
				end = l;
			}
			len = end - start + 1;
			if (len<=0 || start<=0)
				goto Rtn0;
			if (start <= l)
			{
				start--;
				Arg[1]+=start;
				_fstrncpy (OutLoc,Arg[1],len); 
				lpEnd = OutLoc + len;
				*lpEnd=0;  
			}
			else 
			{
				*OutLoc = 0;
				l=0;        
			}
			goto Rtnl;
		}

		case 309: // $RGB(r,g,b) returns color value
				  // $RGB(n) returns R|G|B
		{	
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (!nArgs)
			{
				HDC hDC = GetDC(hWndMain);
				COLORREF testrefs[16];
				POINT pt;
				RECT windRect;
				int numDiff = 0;
				GetWindowRect(hWndMain, &windRect);
				pt.x = (windRect.left + windRect.right) / 2;
				pt.y = (windRect.top + windRect.bottom) / 2;
				for (i = 1000; i < 1016; i++)
				{
					testrefs[i] = i;
					SetPixel(hDC, pt.x, pt.y,testrefs[i]);
					COLORREF ref = GetPixel(hDC, pt.x, pt.y);
					if (testrefs[i] != ref)
						numDiff++;
					pt.x++;
					pt.y++;
				}
				ReleaseDC(hWndMain, hDC);
				itoa(numDiff, OutLoc, 10);
			}
			else if (nArgs == 1)
			{
				COLORREF cref = atol(Arg[1]);
				int r = GetRValue (cref), g = GetGValue (cref), b= GetBValue (cref);
				sprintf(OutLoc, "%i|%i|%i", r, g, b);
			}
			else
			{
				ltoa((long)RGB(atoi(Arg[1]), atoi(Arg[2]), atoi(Arg[3])), OutLoc, 10);
			}
			goto Rtnl;
		} 
		case 351: /* $HSL(r,g,b) returns color value */ 
		{	DWORD	h,l,s;

		nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			ii = ColorHLSToRGB(atoi (Arg[1]),atoi (Arg[2]),atoi (Arg[3]));
			ltoa ((long)ii,OutLoc,10);
			goto Rtnl;
		} 
		
		case 310: /* $TAB(tabnum) tabs to numbered tab loc */
		{	double	rval; 
			int		itab;
			int		ndec, lc, ntoadd; 
			
			if (!CurReport)
				goto RtnFalse;
			hMem = GSSiGlobAlloc(GAIDNO 804,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			if (!rread (Arg1,&rval,&ndec)) goto Rtn0;
			itab = max (1,IDNINT(rval)); 
			if (CurReport->hScrollLine)
			{    
				LPSTR	pLine;  
				float	DBU; 
				int		lline, CurLoc;  
				int	Tabloc, i, nTabs;  
				LPSTR	lpTab, LastTab;
				
				pLine = GlobalLock (CurReport->hScrollLine);   
				nTabs = 0;
				lpTab = pLine;
				LastTab = pLine;  
				DBU = (float)(LOWORD(GetDialogBaseUnits()))/ 2;

				DBU=1;
				i=0;
				while ((lpTab = _fstrchr(lpTab,'\t')))  
				{    
					LPSTR	lpBeg;
					
					lpBeg = LastTab; 
					if (lpBeg != pLine)
						lpBeg++;
					*lpTab = 0;
					lline = _fstrlen (lpBeg);
					*lpTab = '\t';
					if (CurReport->TabLen[0])
	                	i += (IDNINT((DBU * lline)))/(CurReport->TabLen[0]*DeviceToScreenFactor());
					LastTab = lpTab++;
					i++;
				} 
				CurLoc = 0;
				if (i)  
				{    
					LastTab++;
					if (CurReport->NumTabs < i)
						CurLoc = CurReport->TabLen[0]*DeviceToScreenFactor()*i;
					else 
					{   
						short	j;
						for (j=0;j<i;j++)
							CurLoc += CurReport->TabLen[j]*DeviceToScreenFactor();
					}  
				}	
				lline = _fstrlen (LastTab); 
                CurLoc += IDNINT((lline/DBU));
				Tabloc = 0;
				i=0; 
				while (Tabloc <= CurLoc)
				{
					i++;
					if (CurReport->NumTabs < i)
						Tabloc += CurReport->TabLen[0]*DeviceToScreenFactor();
					else
						Tabloc += CurReport->TabLen[min(i,CurReport->NumTabs)-1]*DeviceToScreenFactor();  
				} 
				i--;
				itab -= i;
				while (itab-->0)
					_fstrcat(pLine,"\t");
				GlobalUnlock (CurReport->hScrollLine);
			}
			else 
			{   
				int	Tabloc, i;  
				double factor = DeviceToScreenFactor();
				
				if (CurReport->WantSize)
					factor = 1.0;
				else
					ii = 1;
				Tabloc = CurReport->Rect.left; 
				i=0;
				while (itab--)
				{
					i++;

					if (CurReport->NumTabs < i)
						Tabloc += CurReport->TabLen[0]*factor;
					else
						Tabloc += CurReport->TabLen[i-1]*factor;  
				} 
				if (CurReport->WantSize)
					ii = 1;
				else
					ii = 2;
				CurReport->x = Tabloc;
			}
			goto RtnTrue;
		}

		case 312: /* $CMD(cmdid,vpname,WaitForNCompletions) invokes menu command */
		{	
			int nCompletions = 0;
			int waitForNCompletions = 0;

			nArgs = GetFunArgs(Args, Arg, -3, &hMem, pBrkPt, bpOffset, bpLen);
			ICmd = GetCmdID (Arg[1],0); 
			if (!ICmd && strchr (Arg[1],';'))
				ICmd = -1;
			ExpandText(Arg[2]);
			ExpandText(Arg[3]);
			if (!*Arg[2])
				SetViewport (*pCommandViewport);  
			else
				SetCurView ( SetVPFromName (Arg[2],&Err));
			if (ICmd < 0)
			{
				UINT st;
				waitForNCompletions = atoi(Arg[3]);
				ICmd = -ICmd;
				st = AddGraphicsCmd (CurView->hWnd,Arg[1],FALSE,0); 
				while (waitForNCompletions)
				{
					MSG msg;
					st = AddGraphicsCmd(CurView->hWnd, Arg[1], FALSE, 0);
					while (GSSiPeekMessage(&msg, CurView->hWnd, 0, 0, PM_REMOVE))
					{
						if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
							waitForNCompletions = 0;
						else
						{
							st = ProcessGraphicsFunction(CurView->hWnd, msg.message, msg.wParam, msg.lParam);
							if (st == GF_INCREASE_SUCCESS_COUNT)
								waitForNCompletions--;
						}
					}
				}
				if (ConfigID == SaveConfigID)
					SetCurView ( SaveVP);   
				else
					SetViewport (*pCommandViewport);  
			}	
		    else if (ICmd)
           		PostMessage(hWndMain, WM_COMMAND, (WPARAM)ICmd, MAKELPARAM(CurView->ID,0));
			else 
				goto RtnFalse;
			goto RtnTrue;
		}
		
		case 313: /* $STR(cmdid) returns unprocessed string*/
		{	
			
			hMem = GSSiGlobAlloc(GAIDNO 805,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (OutLoc,Args);
			goto Rtnl;
		}
		
		case 314: /* $LWR(cmdid) returns lower cased string, if second arg true 1st char is upcase*/
		{	
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			_fstrlwr (Arg[1]);   
			if (atob (Arg[2]))
				UpcaseFirst (Arg[1]);
			_fstrcpy (OutLoc,Arg[1]);
			goto Rtnl;
		}
		
		case 315: /* $UPR(cmdid) returns upper cased string*/
		{	
			
			hMem = GSSiGlobAlloc(GAIDNO 807,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			_fstrupr (Arg1);
			_fstrcpy (OutLoc,Arg1);
			goto Rtnl;
		} 
		
		case 316: //$FLT(str)
		{
			
//			hMem = GSSiGlobAlloc(GAIDNO 807, GMEM_MOVEABLE, 4096);
//			Arg1 = GlobalLock(hMem);
//			strcpy(Arg1, Args);
//			ExpandText(Arg1);
//			RVal = FltAP(Arg1, &Err);
			RVal = FltAP(Args, &Err);
			if (Err)
		    {
	FLTAPErr:
				if (ExpandTextDataNotFound)
					RVal = 0;
				else
				{
		    		switch (Err)
		    		{
		    			case 1:
		    				SetGlobalValue ("%LASTERR","Non-numeric in numeric field");
		    				break;
		    			case 2:
		    				SetGlobalValue ("%LASTERR","Mismatched parenthesis");
		    				break;
		    			case 3:
		    				SetGlobalValue ("%LASTERR","Invalid expression");
		    				break;
		    			case 4:
		    				SetGlobalValue ("%LASTERR","Divide by 0");
		    				break;
		    		}
					switch (GetGlobalLVal2 ("[%FLTERROROPT]",1))
					{
						case 1:
							{
								HANDLE	hstr=GSSiGlobAlloc(GAIDNO 1991,GMEM_MOVEABLE,4096*2);
								LPSTR	str = GlobalLock (hstr);
								LPSTR	ExpArgs = str + 4096;
								char	err[64]="[%LASTERR]";

								ExpandText (err);
								sprintf (ExpArgs,"%s\r\n[%%TRACEVALUE]",Args);
								ExpandText (ExpArgs);
								sprintf (str,"Error in: %s\r\n%s\nCurrent Macro: %s",Args,ExpArgs,currentMacroFile);
								strcpy (OutLoc,"0");
								short opt = MessageBox (hWndMain,str,err,MB_ICONEXCLAMATION| MB_YESNOCANCEL);
								GSSiGlobUlFree(&hstr);
								switch (opt)
								{
									case IDCANCEL:
										SetContinueProcessing(FALSE);
										break;
									case IDNO:
										break;
								case IDYES:
									GMEdit(hWndMain,currentMacroFile);
									break;
								}

							}
							break;
						case 2:
							strcpy (OutLoc,"0");
							break;
						case 3:
		    				SetGlobalValue ("%C","F");
		    				goto RtnFalse; 
						case 4:
							{
								HANDLE	hstr=GSSiGlobAlloc(GAIDNO 1992,GMEM_MOVEABLE,4096*2+256);
								LPSTR	str = GlobalLock (hstr);
								LPSTR	ExpArgs = str + 4096;
								LPSTR	pMacro = ExpArgs + 4096;

								if (GetGlobalCVal ("[%FLTERRORMACRO]",pMacro,0))
								{
									sprintf (str,"$MACRO(%s,[%%LASTERR],%s,%s)",pMacro,Args,ExpArgs);
									ProcessText (str);
								}
								GSSiGlobUlFree (&hstr);
							}

						break;
					}
				}
			}
		    sprintf (OutLoc,"%.14lg",RVal);
		    goto Rtnl;
		}  
		
		case 317: //	$DDE(INITIATE,MSAccess,System,MSACCESS);
				  //	$DDE(EXECUTE,MSACCESS,OpenDatabase i:\rockford\attribut\rem.mdb)
				  //	$DDE(TERMINATE,MSACCESS)
	/*	{	 
			int		ndec, start, len, end, pass=0; 
			LPSTR	lpEnd; 
			HWND	hWndDDEClient, hWndDDEServer;  
			LPSTR	str; 
			BOOL	KeepMemLengthSave=KeepMemLength;
			
			if (!(ParLoc = MatchLev (Args,','))) goto Rtn0;
			hMem = GSSiGlobAlloc(GAIDNO 808,GMEM_MOVEABLE,2048*6+1024);
			Arg1 = GlobalLock(hMem);
			Arg2 = Arg1 + 2048; 
			Arg3 = Arg2 + 2048; 
			Arg4 = Arg3 + 2048;    
			Arg5 = Arg4 + 2048;    
			Arg6 = Arg5 + 2048;    
			str  = Arg6 + 2048;
			
           	KeepMemLength = TRUE;  
			_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
			*ParLoc = '\0';
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			if (*Arg1 == 'T' || *Arg1 == 't')
			{   
				ExpandText (Arg2);
				sprintf (str,"[%s]",Arg2);
				hWndDDEClient = (HWND)GetGlobalLVal (str);
				hWndDDEServer = (HWND)GetHwndServerDDE(hWndDDEClient);
				ClientTerminate(hWndDDEClient,hWndDDEServer);
	   	        KeepMemLength = KeepMemLengthSave;
				goto RtnTrue;
			}
			if (!(ParLoc = MatchLev (Arg2,','))) goto RtnFalse;
			_fstrcpy (Arg3,(LPSTR)(ParLoc+1));
			*ParLoc = '\0';
			ExpandText (Arg2);
			if (*Arg1 == 'I' || *Arg1 == 'i')
			{
				if (!(ParLoc = MatchLev (Arg3,','))) goto RtnFalse;
				_fstrcpy (Arg4,(LPSTR)(ParLoc+1));
				*ParLoc = '\0';
				*Arg5 = 0;
				*Arg6 = 0;
				if ((ParLoc = MatchLev (Arg4,',')))
				{
					_fstrcpy (Arg5,(LPSTR)(ParLoc+1)); 
					*ParLoc = '\0'; 
					if ((ParLoc = MatchLev (Arg5,',')))
					{
						_fstrcpy (Arg6,(LPSTR)(ParLoc+1)); 
						*ParLoc = '\0'; 
					}
				}
				ExpandText (Arg3);
				ExpandText (Arg4);
				ExpandText (Arg5);
				ExpandText (Arg6);
				pass=0;
TryDDEInitAgain:
				if (!(hWndDDEClient = SendDDEInitiate(Arg2, Arg3)))  
				{
					if (*Arg5 && !pass)
					{
						UINT hI; 
						UINT	ShowVal=SW_SHOWMAXIMIZED;
						
						pass++;
						if (_fstrnicmp (Arg6,"MIN",3))
							ShowVal = SW_SHOWMINIMIZED;	
						
						hI = WinExec (Arg5,ShowVal);
						if (hI <32)
						{
							DisplayShellExError (hI,Arg5);
				   	        KeepMemLength = KeepMemLengthSave;
							goto RtnFalse;  
						}
						else
						{   
							Wait (15000);
							goto TryDDEInitAgain;
						}  
					}
					else 
					{
			   	        KeepMemLength = KeepMemLengthSave;
						goto RtnFalse;
					}
				}
				ltoa ((long)hWndDDEClient,str,10);
				SetGlobalValue (Arg4,str);
			}
			else if (*Arg1 == 'E' || *Arg1 == 'e')
			{
				sprintf (str,"[%s]",Arg2);
				hWndDDEClient = (HWND)GetGlobalLVal (str);
				hWndDDEServer = (HWND)GetHwndServerDDE(hWndDDEClient);
				ExpandText (Arg3);
				SendExecute(hWndDDEClient,hWndDDEServer, Arg3);
			}
   	        KeepMemLength = KeepMemLengthSave;
			goto RtnTrue;
		}*/
			goto RtnFalse;
        
		case 318: // $SET(vname,vval)  
		{	double	rval;
			int		ndec; 
			char	Format[16];  
			UINT	ln;
			LPSTR	pSetVals;
			
			if (!*Args) 
			{
				GSSiGlobFree (&hSetVals);
				goto RtnTrue;
			}
			if (!(ParLoc = MatchLev (Args,','))) goto RtnFalse;
			hMem = GSSiGlobAlloc(GAIDNO 809,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			Arg2 = Arg1 + 2048; 
			
			_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
			*ParLoc = '\0';
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			ExpandText (Arg2);
			
			if (!hSetVals)
			{
				nSetVals=0;
				lSetVals=0;
				hSetVals = GSSiGlobAlloc(GAIDNO 810,GHND,USHRT_MAX);
			}
			pSetVals = GlobalLock (hSetVals); 
			pSetVals += lSetVals;
			_fstrcpy (pSetVals,Arg1);
			ln = _fstrlen (Arg1) + 1;
			pSetVals += ln;
			lSetVals += ln;
			_fstrcpy (pSetVals,Arg2);
			ln = _fstrlen (Arg2) + 1;
			lSetVals += ln; 
			GlobalUnlock (hSetVals);
			nSetVals++;
			goto RtnTrue;
		}  
		
		case 319:  //$RAW(global name - not in brackets) gets raw global value
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (*Arg[1] == '#')
				GetRawInput(Arg[1],OutLoc,4096);		
			else
				ExpandGlobalRaw (Arg[1],atob(Arg[2]),OutLoc,4096);
			goto Rtnl;
			
		case 320: // $WEB(web address) executes app associated with file
		{   
		   	UINT	hI;
		   	
			if (strstr (Args,".txt,"))
			{
				SHELLEXECUTEINFO sei;
				HWND	hWnd;
				char	WinTxt[128];
				int		x,y,w,h;
				BOOL	Early;

				nArgs = GetFunArgs(Args, Arg, 8, &hMem, pBrkPt, bpOffset, bpLen);
		//		hI=(UINT)ShellExecute (hWndMain,0,Arg[2],0,0,SW_HIDE);//SW_SHOWMAXIMIZED);
		//		if (DisplayShellExError (hI,Arg[1]))
		//			goto RtnFalse;
				if (*Arg[3] && (hWnd = FindWindowEx (0,0,0,Arg[3])))
				{
					//GetWindowText (hWnd,WinTxt,128);
					//DestroyWindow (hWnd);
					PostMessage(hWnd, WM_CLOSE, 0, 0L);
					SleepEx (100,FALSE);
				}
				TxtToHtm (Arg[1],Arg[2]);
				CloseAllRequestedFiles (FALSE);
				SleepEx (100,FALSE);
				memset (&sei,0,sizeof(sei));
				sei.cbSize = sizeof(sei);
				sei.hwnd = hWndMain;
				sei.fMask = SEE_MASK_NOCLOSEPROCESS;
				sei.lpFile = Arg[2];
				if (*Arg[3])
					sei.nShow = SW_HIDE;
				else
					sei.nShow = SW_SHOW;
				if (!ShellExecuteEx((LPSHELLEXECUTEINFO)&sei))
					goto RtnFalse;
				ii=WaitForInputIdle (sei.hProcess,5000);
				SleepEx (2000,FALSE);
				if (*Arg[3] && (hWnd = FindWindowEx (0,0,0,Arg[3])))
				{
					x = atoi (Arg[4]);
					y = atoi (Arg[5]);
					w = atoi (Arg[6]);
					h = atoi (Arg[7]);
					if (w && h)
						SetWindowPos (hWnd,HWND_TOP,x,y,w,h,0);
					ShowWindow (hWnd,SW_SHOW);
					GetWindowText (hWnd,WinTxt,128);
				}
				ii=1;

			}
			else
			{
				ExpandText (Args);
				hI=(UINT)ShellExecute (hWndMain,0,Args,0,0,SW_SHOWMAXIMIZED);
				if (DisplayShellExError (hI,Args))
					goto RtnFalse;
			}
			goto RtnTrue; 
		}
		case 327://$PIK
			PickVis = 2;
			Pickability = TRUE;
			goto SetVis;
		case 321: // $VIS(SymbolName or #SymbolNumber or LAYER:layername or LAYERSYMBOLS or themename.thm,0 or 1 or 2(default)2 toggles;3 returns current vis) 
			PickVis = FALSE;
			Pickability = FALSE;
		SetVis:		idesc = 0;
		{	
			BOOL	IsPar;
			short	Parent, iopt;
			
			if (!CurrentConfig)
			{
				SetConfig (1);
				SetViewport (*pCommandViewport);
			}
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
			{
				DLGPROC lpfnVISIBLEMsgProc;  
				
				Pickability = PickVis;
				lpfnVISIBLEMsgProc = MakeProcInstance((DLGPROC)VISIBLEMsgProc, hInst);
				nRc = DialogBox(hInst, "VISIBLE", CurView->hWnd, lpfnVISIBLEMsgProc);
				FreeProcInstance(lpfnVISIBLEMsgProc);
				if (nRc)
					goto RtnTrue;
				goto RtnFalse; 
            }
			if (nArgs > 2  && *Arg[3])
				SetCurView (SetVPFromName (Arg[3],&Err));   
			if (nArgs < 2)
				iopt = 2;
			else
				iopt = atoi (Arg[2]);
			SelectVisList (PickVis);
			_fstrupr (Arg[1]);
			if (*Arg[1] == '#')
			{
				Arg[1]++;
				idesc = atoi (Arg[1]);  
			} 
			else if (!_fstricmp(Arg[1], "SAVE"))
			{
				if ((!CurView->pPickList1 && !CurView->pPickListManual) || CurView->PickSame)
					hSaveVis = (HANDLE) - 1;
				else
				{
					hSaveVis = GSSiGlobAlloc(GAIDNO 814, GMEM_MOVEABLE,sizeof(VISLIST) + 4);
					LPSTR pSaveVis = GlobalLock(hSaveVis);
					memcpy(pSaveVis, CurView->CurPick, sizeof(VISLIST));
					GlobalUnlock(hSaveVis);
				}
			}
			else if (!_fstricmp(Arg[1], "RESTORE"))
			{
				if (Pickability && hSaveVis)
				{
					if (hSaveVis == (HANDLE) -1)
						SetPickSame(CurView);
					else
					{
						LPSTR pSaveVis = GlobalLock(hSaveVis);
						memcpy(CurView->CurPick, pSaveVis, sizeof(VISLIST));
						GSSiGlobUlFree(&hSaveVis);
					}
				}
			}
			else if (!_fstricmp(Arg[1], "CONTROL"))
			{
				Rect = atorect (Arg[3],&Err);
				if (Err)
				{
					Point16 = atopt16 (Arg[3],&Err);
					if (!Err)
					{
						POINT	MidPt;

						GetWindowRect(CurView->hWnd,&Rect);
						MidPt = RectMid (&Rect);
						Rect.left = MidPt.x - Point16.x/2;
						Rect.right = MidPt.x + Point16.x/2;
						Rect.top = MidPt.y - Point16.y/2;
						Rect.bottom = MidPt.y + Point16.y/2;
					}
				}
					
				if (VisibilityControl (CurView->hWnd,hInst,Arg[2],&Rect,atof(Arg[4]),0,TRUE,FALSE,TRUE))
					goto RtnTrue;                      
				else
					goto RtnFalse;
			} 
			else if (!_fstricmp (Arg[1],"DUMP"))
			{
				rtn = DumpVisibilityToFile (Arg[2],atob(Arg[3]),0);
				SetCurView ( SaveVP);
				goto Rtnrtn;
			} 
			else if (!_fstricmp (Arg[1],"AUTO"))
			{
				Pickability = FALSE;
				TurnOffAutoVis (!atob(Arg[2]));
				goto RtnTrue;                      
			} 
			else if (!_fstrnicmp (Arg[1],"LAYERSYMBOLS:",13))
			{
				if (SetLayerSymbolsVisibility ((LPSTR)(Arg[1]+13),iopt))
					goto RtnTrue;                      
				else
					goto RtnFalse;
			} 
			else if (!_fstrnicmp (Arg[1],"LAYER:",6))
			{
				if (SetLayerVisibility ((LPSTR)(Arg[1]+6),iopt))
					goto RtnTrue;                      
				else
					goto RtnFalse;
			} 
			else if (!_fstricmp (Arg[1],"SAME") && PickVis == 2)
			{
				SetPickSame (CurView);
				goto RtnTrue;
			}
			else if (!_fstrnicmp (Arg[1],"TYPE:",5))
			{
				if (SetTypeVisByName ((LPSTR)(Arg[1]+5),iopt))
					goto RtnTrue;                      
				else
					goto RtnFalse;
			}
			else if (_fstrstr (Arg[1],".THM"))
			{
				if (SetVisibilityFromTheme (Arg[1],Arg[4],iopt))
					goto RtnTrue;                      
				else
					goto RtnFalse;
			}
			else
				idesc = GetSymbolNum (Arg[1]);  
			if (!idesc)
				goto RtnFalse;
			if (!GetSymbolName (idesc,Arg[1],&Parent,0,&IsPar))
				goto RtnFalse;
			if (IsPar)
			{
				if (iopt == 3)
				{
					if (SetParentVisibility (idesc,iopt,-1))
						goto RtnTrue;
					else
						goto RtnFalse;
				}
				else
				{
					SetParentVisibility (idesc,iopt,-1); 
					Pickability = FALSE;
					if (PickVis)
						TurnOffAutoVis (TRUE);
				}
			}
			else if (iopt == 3)
			{
				if (GetVisibility(idesc))
					goto RtnTrue;
				else
					goto RtnFalse;
			}
			else
			{
				Pickability = FALSE;
				if (iopt < 0)
					SetHalfToneVisibility(idesc, iopt < -1);
				else
				{
					if (GetVisibility(idesc) != iopt)
						ToggleVisibility(idesc);
					if (!PickVis)
						TurnOffAutoVis(TRUE);
				}
			}
			
			goto RtnTrue;
		}  
		
		case 322: //$GPS (WPTRAN OFFSET or CONFIG)
        {
        	BOOL	nRc=FALSE;


			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			setDoPaint( FALSE);      
            if (!_fstricmp (Arg[1],"WPTRAN"))
             {
				lpfnGPSMsgProc = MakeProcInstance((DLGPROC)GPSMsgProc, hInst);
				nRc = DialogBox(hInst, (LPSTR)"GPS",hWndMain, lpfnGPSMsgProc);
				FreeProcInstance(lpfnGPSMsgProc);  
			}
            else if (!_fstricmp (Arg[1],"OFFSET"))
			{
				GPSOffset = atopt (Arg[2],&nRc);
				ConvertCoord (&GPSOffset,1,2);
				SetGPSOffset = TRUE;
				nRc = !nRc;
			}
            else if (!_fstricmp (Arg[1],"PASSTO"))
			{
				GPSPassTo = FindWindowByName (Arg[2]);
				if (GPSPassTo)
					nRc = TRUE;
			}
			else if (!stricmp(Arg[1], "CURRENT"))
			{
				//GetGPSLocation(&Point.y, &Point.x);
				dpointtoa(OutLoc,&Point);
				goto Rtnl;
			}
			else
            {
				lpfnGPSCONFIGMsgProc = MakeProcInstance((DLGPROC)GPSCONFIGMsgProc, hInst);
				nRc = DialogBox(hInst, (LPSTR)"GPSCONFIG", hWndMain, lpfnGPSCONFIGMsgProc);
				FreeProcInstance(lpfnGPSCONFIGMsgProc);   
			}
			setDoPaint( TRUE); 
				
            if (nRc)
            	goto RtnTrue;
            else
				goto RtnFalse;
        }
		
		case 323: //$DMS (dec deg,ndp) returns xxD xxM xx.ndpS or xxD xx.abs(ndp)M if ndp < 0
		{
			short	Deg, Min, ndp;
			double	Sec, decdeg;
			  
			if (!(ParLoc = MatchLev (Args,','))) goto Rtn0;
			hMem = GSSiGlobAlloc(GAIDNO 814,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			Arg2 = Arg1 + 2048; 
			
			_fstrcpy (Arg2,(LPSTR)(ParLoc+1));
			*ParLoc = '\0';
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			ExpandText (Arg2); 
			ndp = atoi (Arg2);
			decdeg = atof (Arg1);
			GetDMS (decdeg,&Deg,&Min,&Sec);
			if (ndp < 0)
				sprintf (OutLoc,"%2.2iD %.*fM",abs(Deg),abs(ndp),(double)Min + Sec/60);
			else 
				sprintf (OutLoc,"%2.2iD %2.2iM %.*fS",abs(Deg),Min,ndp,Sec);
			goto Rtnl;
		}
			
		case 324: // $CFG(LOAD,config file) Load config file 
				  //      SAVE,config file) Save with current zoom
		{	
			HDC	hDC;			
			
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			
			if (!_fstricmp (Arg[1],"LOAD"))
			{
				if (!ExistFile (Arg[2]))
	            	goto RtnFalse;  
	            ForceBounds = FALSE;
				SaveZooms (0);
		        UnallocateConfig ();
	    		_fstrcpy (CfgName,Arg[2]);
				CheckForContinue (TRUE,0);  
				if (atob (Arg[3]))
				{
					if (!OpenConfig(0,0))
						goto RtnFalse;
				}
				else
					PostMessage(hWndMain, WM_COMMAND, IDM_REDISPLAY, 0L);
				goto RtnTrue;  
			}
			else if (!_fstricmp (Arg[1],"SAVE"))
			{   BOOL SaveSaveZoom = SaveZoom, SaveSaveGlobals = SaveGlobals, SaveSaveCfgImage = SaveCfgImage, rtn;
				
             	SaveZoom = TRUE;
             	SaveGlobals = TRUE; 
             	SaveCfgImage = TRUE;
				SetConfig(1);
	            rtn = SaveConfig (Arg[2],TRUE);
	            SaveZoom = SaveSaveZoom;
	            SaveGlobals = SaveSaveGlobals; 
	            SaveCfgImage = SaveSaveCfgImage;
	            if (rtn)
	            	goto RtnTrue;
			}
			goto RtnFalse;
		}
		
		case 325: /* $LEN(string) returns length of string*/
		{	
			
			hMem = GSSiGlobAlloc(GAIDNO 815,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			l = _fstrlen (Arg1);
			itoa (l,OutLoc,10);
			goto Rtnl;
		} 
		
		case 326: /* $HEX(string) returns hex value*/
		{	
			
			hMem = GSSiGlobAlloc(GAIDNO 816,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			hNum = atol (Arg1);
			sprintf (OutLoc,"%xl",hNum);
			goto Rtnl;
		} 
		
		case 328:  //$MON(returns month(1-12) from system time)
				   //$MON(time,2) returns Mar 09
				   //$MON(time,3) returns month number beginning with 0=Jan 1970
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			n = atoi (Arg[2]);
			systime = max (0,atol (Arg[1]));
			if (systime < 0)
				goto RtnFalse;
			tmtime = *localtime (&systime);
			if (tmtime.tm_year < 100)
				year = tmtime.tm_year;
			else
				year = tmtime.tm_year - 100;
			switch (n)
			{
			case 2:
				sprintf (OutLoc,"%s %2.2i",MonthAbv[tmtime.tm_mon],year);
				break;
			case 3:
				itoa (tmtime.tm_year*12 + tmtime.tm_mon,OutLoc,10);
				break;
			default:
				itoa (tmtime.tm_mon+1,OutLoc,10); 
			}
			goto Rtnl;
			
		case 341:  //$DOY(returns day of year (1-366) from system time)
		case 342:  //$DOM(returns day of month (1-32) from system time)
		case 329:  //$DOW(returns day of week (1-7) from system time)
			hMem = GSSiGlobAlloc(GAIDNO 818,GMEM_MOVEABLE,2048);
			Arg1 = GlobalLock(hMem); 
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			systime = atol (Arg1); 
			if (systime < 0)
				goto RtnFalse;
			tmtime = *localtime (&systime); 
			switch (FunID)
			{   
				case 329:
					itoa (tmtime.tm_wday+1,OutLoc,10); 
					break;
				case 341:
					itoa (tmtime.tm_yday+1,OutLoc,10); 
					break;
				case 342:
					itoa (tmtime.tm_mday+1,OutLoc,10); 
					break;
			}
			goto Rtnl;  
			
		case 330: // $SQL(dbid,SQL)
		{	 
			HANDLE	hDB=0;
			int		IDB;
			
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			*OutLoc = 0;
			if (!stricmp (Arg[1],"CREATE"))
			{
				if (SetSQLDatabaseFields (Arg[2]))
					goto RtnTrue;
				else
					goto RtnFalse;
			}
			if ((hDB = GetDBByIDName (Arg[1])))
			{
				IDB = (int)GetDBHandleFromSQL (hDB);
				if (!ExternalSQLDirect (IDB, Arg[2]))
					goto RtnFalse; 
				else
					goto RtnTrue;
			}			
			else
			{   
				LPOPENSQLDATA	SQLPtr;
				LPFIELDINFO	lpFieldInfo;
				LPOPENFILEDATA	FilePtr;
                HANDLE	hVal;
                LPSTR	val;
                LPSTR	str;
                
			    if (!OpenDataFile (Arg[1],Arg[2],BT_READ,&hDB))
            		goto RtnFalse; 
            	if (!FetchDBRec (hDB))
				{
					CloseDataFile (FALSE,&hDB); 
            		goto RtnFalse;
				}
            	hVal = GSSiGlobAlloc(GAIDNO 1993,GMEM_MOVEABLE,4096*2);
            	val = GlobalLock (hVal);  
            	str = val + 4096;
    			SQLPtr = (LPOPENSQLDATA)GlobalLock(hDB);
				FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle);    
				lpFieldInfo = &FilePtr->FldInfo; 
				*str = 0;
				for (i=0;i<FilePtr->NumFields;i++,lpFieldInfo++) 
				{
					sprintf (val,"[%s]",lpFieldInfo->name);   
					ExpandText (val);   
					sprintf (_fstrchr(str,0),"%s=%s;",lpFieldInfo->name,val);
				}
				if (FilePtr->NumFields == 1)
					_fstrcpy (OutLoc,val); 
				else
					_fstrcpy (OutLoc,"1"); 
				SetGlobalValue ("%RESULT",str);
                GlobalUnlock (SQLPtr->OFHandle);
                GlobalUnlock (hDB);
            	HaltReport = FALSE;
				CloseDataFile (FALSE,&hDB); 
				GSSiGlobUlFree (&hVal);
				if (HaltReport)
					goto RtnFalse;
			}
			goto Rtnl;
		}
		
		case 331:  //$CHR(returns character value from number-mult char may be separated by comma
			hMem = GSSiGlobAlloc(GAIDNO 819,GMEM_MOVEABLE,2048);
			Arg1 = GlobalLock(hMem); 
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			n = 0;
			while (*Arg1)
			{
				LPSTR	Endloc = strchr (Arg1,',');

				if (Endloc)
					*Endloc++ = 0;
				else
					Endloc = strchr (Arg1,0);
				OutLoc[n] = atoi (Arg1);
				if (!OutLoc[n])
					OutLoc[n] = *Arg1;
				n++;
				Arg1 = Endloc;
			}
			OutLoc[n] = 0;
			goto Rtnl;  
			
		case 332: // $ELV(point,surfacehandle)
		{	 
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			hSurf = (HANDLE)atol (Arg[2]); 
			if (!hSurf)
			{
				HANDLE	hSQL = GetDBByIDName (Arg[2]);   
				
				if (hSQL)
					hSurf = GetDBHandleFromSQL (hSQL);    
			}
			Point = atopt (Arg[1],&Err);  
			Elevation = NGIELV (Point,hSurf,atoi(Arg[3])); 
			if (Elevation == DBL_MAX) 
			{
				Elevation = 0;
				ExpandTextDataNotFound = TRUE;
			}
			sprintf (OutLoc,"%f",Elevation);
			goto Rtnl;
		}
		
		case 333: // $DTM(OPEN,Pathname,NullElv,surfacehandleglobal)  
				  // $DTM(CLOSE,surfacehandle)  
				  // $DTM(COPY,tosurf,fromsurf,area)
		{	 
			nArgs = GetFunArgs(Args, Arg, 7, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			if (!_fstricmp (Arg[1],"OPEN"))
			{
				Elevation = atof (Arg[3]); 
				if (*Arg[5] == 'W')
					Mode = BT_WRITE;
				else
					Mode = BT_READ;
				hSurf = DTMOpen (Arg[2],Elevation,Mode,0,0); 
				SetGlobalValueLong (Arg[4],(long)hSurf);
				if (hSurf)
					goto RtnTrue;
			}
			else if (!_fstricmp (Arg[1],"CLOSE"))
			{
				hSurf = (HANDLE)atol (Arg[2]); 
				DTMClose (&hSurf);
				goto RtnTrue;
			}
			else if (!_fstricmp (Arg[1],"SETTINGS"))
			{
				SetCurView (SetVPFromName (Arg[2],&Err));  
				if (!(n = GetLayerNumFromName (Arg[3])))
					goto RtnFalse;
				rtn = EditDTMSettings (CurView->hWnd,n);
				goto Rtnrtn;
			}
			else if (!_fstricmp(Arg[1], "COPY"))
			{
				if (nArgs < 4)
					goto RtnFalse;
				if (DTMCopy(Arg[2], Arg[3], Arg[4]))
					goto RtnTrue;
			}
			else if (!_fstricmp(Arg[1], "CONVERT"))
			{
				if (nArgs < 3)
					goto RtnFalse;
				HANDLE hDTM = DTMOpen(Arg[2], DBL_MAX, BT_READ, 0, 0);
				if (!hDTM)
					goto RtnFalse;
				rtn = ConvertDTMToSQLITE(hDTM, Arg[3]);
				DTMClose(&hDTM);
				goto Rtnrtn;
			}
			else if (!_fstricmp (Arg[1],"DUMP"))
			{
				double	minElev, maxElev;

				if (nArgs < 4)
					goto RtnFalse;
				Bounds = atobounds (Arg[3],&Err);
				RVal = atof (Arg[4]);
				if (SurfToFile (Arg[2],&Bounds,RVal,Arg[5],&minElev,&maxElev))
				{
					if (*Arg[6])
					{
						char str[64];

						sprintf (str,"%.3f:%.3f",minElev,maxElev);
						SetGlobalValue (Arg[6],str);  
					}
					goto RtnTrue;
				}
			}

			goto RtnFalse;
		}
		
		case 334:  //$INT(returns integer value (truncates))
			hMem = GSSiGlobAlloc(GAIDNO 821,GMEM_MOVEABLE,2048);
			Arg1 = GlobalLock(hMem); 
			//_fstrcpy (Arg1,Args);
			//ExpandText (Arg1);
			RVal = FltAP (Args,&Err);   
			if (Err)
				goto FLTAPErr;
			nlong = (long)RVal;
			ltoa (nlong,OutLoc,10);
			goto Rtnl;  
			
		case 335:  //$INC(returns value + 1 (or arg2)) 
		{
			static	long	NextNum=1;
			int inc = 1;

			nArgs = GetFunArgs(Args, Arg, -2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs == 2)
			{
				ExpandText(Arg[2]);
				inc = atoi(Arg[2]);
			}
			if (*Args == '[')
			{
				ExpandText(Arg[1]);
				if (nArgs)
					nlong = IDNINT(atof (Arg[1]))+inc; 
			}
			else if (!nArgs)
				nlong = NextNum++;
			else
			{
				char	glob[80];

				sprintf (glob,"[%s]",Arg[1]);
				nlong = GetGlobalLVal (glob);
				nlong += inc;
				SetGlobalValueLong(Arg[1], nlong);
			}
			ltoa (nlong,OutLoc,10);
			goto Rtnl;  
		}
			
		case 336:  //$DEC(returns value - 1)
		{
			static	long	NextNum=-1;

			if (*Args == '[')
			{
				nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
				if (nArgs)
					nlong = IDNINT(atof (Arg[1]))-1; 
			}
			else if (!*Args)
				nlong = NextNum--;
			else
			{
				char	glob[80];

				sprintf (glob,"[%s]",Args);
				nlong = GetGlobalLVal (glob);
				nlong--;
				SetGlobalValueLong (Args, nlong);
			}
			ltoa (nlong,OutLoc,10);
			goto Rtnl;  
		}
			
			
		case 337: /* $PAD(val,len,fillchar,inReport) right fills val with fillchar to length len */
		{
			double	rval;
			int		l, len;
			UCHAR* lpOut;
			char	fillchar = '0';
			UCHAR	startStopChar;

			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);

			l = _fstrlen(Arg[1]);
			len = IDNINT(atof(Arg[2]));
			if (*Arg[3])
				fillchar = *Arg[3];
			if (atob(Arg[4]))
			{
				startStopChar = 32 + 128;
				if (inPrintScrollReport)
					startStopChar = 32;
			}
			else
				startStopChar = fillchar;
			_fstrcpy(OutLoc, Arg[1]);
			lpOut = OutLoc + l;
			*lpOut++ = 32 + 128;
			while (l++ < len - 2)
				*lpOut++ = fillchar;
			*lpOut++ = 32 + 128;
			*lpOut = 0;
			goto Rtnl;
		}

		case 339:  //$MIN(arg1,arg2....argn)  
		case 340:  //$MAX(arg1,arg2....argn)  
		case 338:  //$SUM(arg1,arg2....argn)  
		case 346:  //$AVE(arg1,arg2....argn)  
		{
			double	TotVal;
			int		NumVals=0;
			
			switch (FunID)
			{ 
				case 339:
					TotVal = DBL_MAX;
					break;
				case 340:
					TotVal = -DBL_MAX;
					break;
				default:
					TotVal = 0;
					break;
			}
			hMem = GSSiGlobAlloc(GAIDNO 824,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem); 
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			ndec = 0;
			pVal = Arg1;
			while (*pVal)
			{
				int	nd;

				if ((pEnd = _fstrchr (pVal,',')))
					*pEnd++ = 0;
				else
					pEnd = _fstrchr (pVal,0);  
				rread (pVal, &RVal,&nd); 
				NumVals++;
				switch (FunID)
				{ 
					case 339:
						TotVal = min (TotVal,RVal);
						break;
					case 340:
						TotVal = max (TotVal,RVal);
						break;
					default:
						TotVal += RVal;
						break;
				}
				ndec = max (ndec,nd);
				pVal = pEnd;
			}
			if (FunID == 346 && NumVals)
				TotVal /= NumVals;
			RWRITE (TotVal, max(0,ndec-1), OutLoc);
			goto Rtnl;  
		}
			
		case 343: // $AZM(point1,point2 or pointlist) */ 
				  // $AZM(REVERSE,az)
				  // $AZM(GETMIDS,azms,dlm) returns 2 perpendiculars if only one az
		case 354: // $COG(point1,point2 or pointlist) */ Course over ground
		{
			double az,az2;
			int	nPoints;
			HANDLE	hPoints;
			
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);

			switch (nArgs)
			{
			default:
				goto RtnFalse;
			case 1:
				if ((nPoints = GetPointsFromList (Arg[1],&hPoints)) <2)
				{
					GSSiGlobFree (&hPoints);	
					goto RtnFalse;
				}
			    pPoint = (HPDPOINT)GlobalLock (hPoints);
				Point = pPoint[0];
				Point2 = pPoint[nPoints-1];
				GSSiGlobUlFree (&hPoints);
				break;
			case 2:
				if (!stricmp(Arg[1], "REVERSE"))
				{
					az = atof(Arg[2]);
					az = LTWOPI(az + PY);
					ftoa(OutLoc, az);
					goto Rtnl;
				}
				else if (!stricmp(Arg[1], "NORMALIZE"))
				{
					az = atof(Arg[2]);
					az = LTWOPI(az);
					ftoa(OutLoc, az);
					goto Rtnl;
				}
				Point = atopt(Arg[1], &Err);
				Point2 = atopt (Arg[2],&Err);
				break;
			case 3: 
				if(!stricmp(Arg[1], "GETMIDS"))
				{
					BTVARDESC	BTVar[2];
					HANDLE hSortList;
					char dlm = *Arg[3];
					int	naz = 1;
					short pos;
					LPSTR pDlm = Arg[2];
					LPSTR pAz;

					while ((pDlm = strchr(pDlm, dlm)))
					{
						naz++;
						pDlm++;
					}
					if (naz == 1)
					{
						az = atof(Arg[2]);
						sprintf(OutLoc, "%5.3f%c%5.3f", LTWOPI(az - PY / 2),dlm, LTWOPI(az + PY / 2));
						goto Rtnl;
					}
					GSSiGetTempFileName(0, "gm", 0, Arg[5]);
					BTVar[0].BT_VARTYP = BT_REAL;
					BTVar[0].BT_VARLEN = 8;
					BTVar[0].BT_VAROFF = 0;
					BT_CREATE(Arg[5], 4, FALSE, 1, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
					hSortList = BT_OPEN(Arg[5], 0, BT_WRITE, 0);
					pDlm = pAz = Arg[2];
					naz = 1;
					while ((pDlm = strchr(pDlm, dlm)))
					{
						pDlm++;
						az = atof(pAz);
						pAz = pDlm;
						BT_PUT(hSortList, (LPSTR)&az, (LPSTR)&naz);
						naz++;
					}
					az = atof(pAz);
					BT_PUT(hSortList, (LPSTR)&az, (LPSTR)&naz);
					BT_FIND(hSortList, (LPSTR)&az, BT_LAST, BT_ANY, (LPSTR)&naz);
					az = az - TWOPI;
					pos = BT_FIRST;
					while (!BT_FIND(hSortList, (LPSTR)&az2, pos, BT_ANY, (LPSTR)&naz))
					{
						if (pos == BT_FIRST)
							sprintf(OutLoc, "%5.3f", LTWOPI((az + az2) / 2));
						else
							sprintf(strchr(OutLoc, 0), "%c%5.3f", dlm, LTWOPI((az + az2) / 2));
						pos = BT_NEXT;
						az = az2;
					}
					BT_CLOSEANDDELETE(&hSortList);
					goto Rtnl;
				}
				goto RtnFalse;
				break;
			}
			if (FunID == 354)
				az = CourseOverGroundBase (&Point,&Point2);
			else
				az =  getazd (&Point,&Point2);
			ftoa (OutLoc,az);
			goto Rtnl;
		}  
		
		case 344: // $RDF(SET,VP,symname/parent or LAYER:layername,type (ALL,POINT,LINE,AREA,TEXT),color,width) 
				  // $RDF(REMOVE
				  // $RDF(CLEAR
				  // $RDF(LOAD
				  // $RDF(SAVE
		{	
			short	newob=-1;
			
			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
			SaveVP = CurView;      
			SetCurView ( SetVPFromName (Arg[2],&Err));  
			if (!CurView)
				goto RtnFalse;
				
			if (!_fstricmp (Arg[1],"SET"))
			{
				Pickability = FALSE;
				TurnOffAutoVis (TRUE);
				if (!_fstrnicmp (Arg[3],"LAYER:",6))
				{
					if (!(layer=GetLayerNumFromName (Arg[3]+6)))
						goto RtnFalse; 
					layer--;                     
					CurView->LayerColor[layer] = atol(Arg[5]);
	        		CurView->HaveLayerColor[layer] = 1;
					ConfigChangesMade = TRUE;
					goto RtnTrue;
				}
				else                 
				{
					n = GetSymbolNum (Arg[3]);  
					if (!n)
						goto RtnFalse;  
					if (!SetRDFColorAndWidth (n,&newob,Arg[4],atol(Arg[5]),atoi(Arg[6])))
						goto RtnFalse;
					ConfigChangesMade = TRUE;
					goto RtnTrue; 
				}
			}
			if (!_fstricmp (Arg[1],"REMOVE"))
			{
				if (!_fstrnicmp (Arg[3],"LAYER:",6))
				{
					if (!(layer=GetLayerNumFromName (Arg[3]+6)))
						goto RtnFalse; 
					layer--;                     
					CurView->LayerColor[layer] = atol(Arg[5]);
	        		CurView->HaveLayerColor[layer] = 1;
					ConfigChangesMade = TRUE;
					goto RtnTrue;
				}
				else                 
				{
					n = GetSymbolNum (Arg[3]);  
					if (!n)
						goto RtnFalse;
					newob = -2;  
					if (!SetRDFColorAndWidth (n,&newob,Arg[4],atol(Arg[5]),atoi(Arg[6])))
						goto RtnFalse;
					ConfigChangesMade = TRUE;
					goto RtnTrue; 
				}
			}
			else if (!_fstricmp (Arg[1],"CLEAR"))
			{
				RemoveVPRedef ();
				ConfigChangesMade = TRUE;
				goto RtnTrue;
			} 
			goto RtnFalse;
		}
		
		case 345: /* $ABS(val) */ 
		{	double az;
			
			nArgs = GetFunArgs(Args, Arg, 1, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (strchr(Arg[1], '.'))
			{
				RVal = atof(Arg[1]);
				RVal = fabs(RVal);
				ftoa(OutLoc, RVal);
			}
			else
			{
				int iVal = atoi(Arg[1]);
				iVal = abs(iVal);
				itoa(iVal, OutLoc, 10);
			}
			goto Rtnl;
		}  
		
		case 347: /* $GMD(val) */ 
		{
			nArgs = GetFunArgs(Args, Arg, 8, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			GMDFunctions (nArgs,Arg,OutLoc);
			goto Rtnl;
		}  
		
		case 348: /* $MOD(val) */ 
		{	int	i1,i2;
			
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			i1 = atoi (Arg[1]);
			i2 = atoi (Arg[2]);
			if (i2)
				i1 = i1 % i2;
			else
			{
				i1 = 0;
				Err = 4;
				goto FLTAPErr;
			}
			itoa (i1,OutLoc,10);
			goto Rtnl;
		}  

		case 349: // $NUM(val) converts single character to integer
		{	
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (nArgs == 1)
			{
				i = *(LPBYTE)Arg[1];
				itoa (i,OutLoc,10);
			}
			goto Rtnl;
		}  
		
		case 350: // $SUN(RISE,lat lon,systime)
				  // $SUN(ALT,lat lon,systime)
				  // $SUN(AZ,lat lon,systime)
		{
			//struct tm tms;
			time_t	itime;
			double	Altitude, Az;

			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
		    //tm = *sunrise (double lat, double lon, int year, int month, int day);
			
			Point = atopt (Arg[2],&Err);
			itime = atol (Arg[3]);
			itime = max(0, itime);
			if (!stricmp (Arg[1],"ALT"))
			{
				GetAltitudeAndAzOfSun (&Point,0,itime,&Altitude,&Az);
				ftoa (OutLoc,Altitude);
				goto Rtnl;
			}
			if (!stricmp (Arg[1],"AZ"))
			{
				GetAltitudeAndAzOfSun (&Point,0,itime,&Altitude,&Az);
				ftoa (OutLoc,Az);
				goto Rtnl;
			}

/*		    tms = *sunrise (45*RADDEG,-93.3*RADDEG,2011,2, 18);
			systime = localtime (&tms);  
			systime = time (0);
			tms = *gmtime (&systime);*/
			goto Rtnl;
		}

		case 353: //$LIT(value) store literal value - must be decoded by $UNLIT
		{
			HANDLE hStr = GSSiGlobAlloc(GAIDNO 1776,GMEM_MOVEABLE,4096);
			LPSTR  pStr = GlobalLock (hStr);

			strcpy (pStr,Args);
			sprintf (OutLoc,"$LIT(%s)",pStr);
			GSSiGlobUlFree (&hStr);
			goto Rtnl;
		}

		case 355://$FTP(OPEN,service,username,pw,directory,errvarname,port(opt),passive(opt),numreopenattempts)
				 //$FTP(CLOSE,handle);
				 //$FTP(LIST,handle,wildcard,errvarname)
				 //$FTP(GETFILE,handle,remotename,localname,replace,showStatus,errvarname)
				 //$FTP(PUTFILE,handle,remotename,localname,replace,showStatus,errvarname)
				 //$FTP(DELETEFILE,handle,remotename,errvarname)
				 //$FTP(GETDIRECTORY,handle,errvarname)
				 //$FTP(SETDIRECTORY,handle,remoteDirName,errvarname)
		{
			HANDLE hFTPStruct;
			LPFTPSTRUCT pFTPStruct;

			nArgs = GetFunArgs(Args, Arg, 10, &hMem, pBrkPt, bpOffset, bpLen);

			if (!_fstricmp(Arg[1],"OPEN"))
			{
				hFTPStruct = GSSiGlobAlloc(GAIDNO 1781,GHND,sizeof(FTPSTRUCT));
				pFTPStruct = GlobalLock (hFTPStruct);
				pFTPStruct->structType = ST_FTPSTRUCT;
				pFTPStruct->hFTP = FTPOpen(Arg[2], Arg[3], Arg[4], Arg[5], Arg[6], atoi(Arg[7]), atob(Arg[8]));
				if (!pFTPStruct->hFTP)
				{
					GSSiGlobUlFree (&hFTPStruct);
					goto RtnFalse;
				}
				pFTPStruct->reopenAttempts = atoi(Arg[9]);
				strcpy (pFTPStruct->ServerName,Arg[2]);
				strcpy (pFTPStruct->Username,Arg[3]);
				strcpy (pFTPStruct->Password,Arg[4]);
				strcpy (pFTPStruct->directory,Arg[5]);
				GlobalUnlock (hFTPStruct);
				itoa ((int)hFTPStruct,OutLoc,10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1],"CLOSE"))
			{
				hFTPStruct = (HANDLE)atoi (Arg[2]);
				if (CheckStructType (hFTPStruct,ST_FTPSTRUCT))
				{
					pFTPStruct = GlobalLock (hFTPStruct);
					rtn = FTPClose (pFTPStruct->hFTP);
					GSSiGlobUlFree (&hFTPStruct);
					goto Rtnrtn;
				}
			}
			else if (!stricmp(Arg[1],"SPLIT"))
			{
				LPSTR pName,pType,pSize,pDate;
				*OutLoc = 0;
				pName = Arg[2];
				pType = pSize = pDate = OutLoc;
				pType = strchr (pName,'|');
				if (pType)
				{
					*pType++ = 0;
					pSize = strchr (pType,'|');
					if (pSize)
					{
						*pSize++ = 0;
						pDate = strchr (pSize,'|');
						if (pDate)
							*pDate++ = 0;
						else
							pDate = OutLoc;
					}
					else
						pSize = OutLoc;
				}
				else
					pType = OutLoc;
				if (!stricmp(Arg[3],"NAME"))
					strcpy (OutLoc,pName);
				else if (!stricmp(Arg[3],"TYPE"))
					strcpy (OutLoc,pType);
				else if (!stricmp(Arg[3],"SIZE"))
					strcpy (OutLoc,pSize);
				else if (!stricmp(Arg[3],"DATE"))
					strcpy (OutLoc,pDate);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1],"LIST"))
			{
				hFTPStruct = (HANDLE)atoi (Arg[2]);
				if (CheckStructType(hFTPStruct, ST_FTPSTRUCT))
				{
						pFTPStruct = GlobalLock (hFTPStruct);
						if (*Arg[3])
						{
							pFTPStruct->hFind = ListFtpDir(pFTPStruct->hFTP,0,Arg[3],
															0,pFTPStruct->fileName,&pFTPStruct->fileSize,
															&pFTPStruct->lastUpdateTime,
															&pFTPStruct->isDirectory,Arg[4]);
						}
						else if (pFTPStruct->hFind)
						{
							pFTPStruct->hFind = ListFtpDir(pFTPStruct->hFTP,pFTPStruct->hFind,0,0,
													pFTPStruct->fileName,&pFTPStruct->fileSize,
													&pFTPStruct->lastUpdateTime,&pFTPStruct->isDirectory,Arg[4]);
						}
						if (pFTPStruct->hFind)
						{
							sprintf (OutLoc,"%s|%i|%i|%i",pFTPStruct->fileName,
														  pFTPStruct->isDirectory+1,
														  pFTPStruct->fileSize,
														  pFTPStruct->lastUpdateTime);
							GlobalUnlock (hFTPStruct);
							goto Rtnl;
						}
						GlobalUnlock (hFTPStruct);
				}
			}
			else if (!stricmp(Arg[1],"GETFILE"))
			{
				hFTPStruct = (HANDLE)atoi (Arg[2]);

				if (CheckStructType(hFTPStruct, ST_FTPSTRUCT))
				{
						int nAttemps = 0;
						pFTPStruct = GlobalLock (hFTPStruct);
						do {
							if (nAttemps)
								ReopenFTP (pFTPStruct);
							rtn = FTPGetFile(pFTPStruct->hFTP,Arg[3],Arg[4],atob(Arg[5]),atob(Arg[6]),Arg[7]);
						}while (!rtn && nAttemps++ < pFTPStruct->reopenAttempts);
						GlobalUnlock (hFTPStruct);
						goto Rtnrtn;
				}
				else if (*Arg[7])
					SetGlobalValue (Arg[7],"FTP session not open"); 
			}
			else if (!stricmp(Arg[1],"PUTFILE"))
			{
				hFTPStruct = (HANDLE)atoi (Arg[2]);
				if (CheckStructType(hFTPStruct, ST_FTPSTRUCT))
				{
						pFTPStruct = GlobalLock (hFTPStruct);
						rtn = FTPPutFile(pFTPStruct->hFTP,Arg[3],Arg[4],atob(Arg[5]),atob(Arg[6]),Arg[7]);
						GlobalUnlock (hFTPStruct);
						goto Rtnrtn;
				}
				else if (*Arg[7])
					SetGlobalValue (Arg[7],"FTP session not open"); 
			}
			else if (!stricmp(Arg[1],"DELETEFILE"))
			{
				hFTPStruct = (HANDLE)atoi (Arg[2]);

				if (CheckStructType(hFTPStruct, ST_FTPSTRUCT))
				{
						pFTPStruct = GlobalLock (hFTPStruct);
						rtn = FTPDeleteFile(pFTPStruct->hFTP,Arg[3],Arg[4]);
						GlobalUnlock (hFTPStruct);
						goto Rtnrtn;
				}
				else if (*Arg[4])
					SetGlobalValue (Arg[4],"FTP session not open"); 
			}
			else if (!stricmp(Arg[1],"SETDIRECTORY"))
			{
				hFTPStruct = (HANDLE)atoi (Arg[2]);

				if (CheckStructType(hFTPStruct, ST_FTPSTRUCT))
				{
						pFTPStruct = GlobalLock (hFTPStruct);
						rtn = FTPSetDirectory(pFTPStruct->hFTP,Arg[3],Arg[4]);
						if (rtn)
							FTPGetDirectory(pFTPStruct->hFTP,pFTPStruct->directory,0);
						GlobalUnlock (hFTPStruct);
						goto Rtnrtn;
				}
				else if (*Arg[4])
					SetGlobalValue (Arg[4],"FTP session not open"); 
			}
			else if (!stricmp(Arg[1],"GETDIRECTORY"))
			{
				hFTPStruct = (HANDLE)atoi (Arg[2]);

				if (CheckStructType(hFTPStruct, ST_FTPSTRUCT))
				{
						pFTPStruct = GlobalLock (hFTPStruct);
						rtn = FTPGetDirectory(pFTPStruct->hFTP,Arg[4],Arg[3]);
						GlobalUnlock (hFTPStruct);
						if (rtn)
						{
							strcpy (OutLoc,Arg[4]);
							goto Rtnl;
						}
						goto Rtnrtn;
				}
				else if (*Arg[3])
					SetGlobalValue (Arg[3],"FTP session not open"); 
			}
			goto RtnFalse;

		}

		case 356://$DSN(EXISTS,name,localmachine(TorF))
		{
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);

			if (nArgs < 2)
				goto RtnFalse;
			if (!stricmp (Arg[1],"EXISTS"))
			{
				if (doesDSNExist (Arg[2],atob(Arg[3])))
					goto RtnTrue;
			}
			if (!stricmp(Arg[1], "CREATE"))//$DSN(CREATE,name,F,SQL Server,config)
			{
				if (createDSN(Arg[2], atob(Arg[3]), Arg[4], Arg[5]))
					goto RtnTrue;
			}
			if (!stricmp(Arg[1], "DELETE"))//$DSN(DELETE,name)
			{
				if (deleteDSN(Arg[2]))
					goto RtnTrue;
			}
			goto RtnFalse;
		}

		case 357://$VAR([VAR1],[VAR2]=4,...,[VAR16]=x)
		{
			rtn = FALSE;
			goto Rtnrtn;
		}

		case 358://$TIN(EXPORT,File)
		{
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			GSSiClose2 (&FidTINExtract);
			FidTINExtract = GSSiOpenFile(Arg[2], 0, OF_CREATE);
			if (FidTINExtract == HFILE_ERROR)
				goto RtnFalse;
			goto RtnTrue;
		}
		case 359://$TCP(MYADDRESS)
		{
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (!_fstricmp(Arg[1], "MYADDRESS"))
			{
				if (GetMyIPNetAddress(OutLoc))
					goto Rtnl;
			}
			goto RtnFalse;
		}
		case 360://$SHP(TRANSFORM)
		{
			nArgs = GetFunArgs(Args, Arg, 6, &hMem, pBrkPt, bpOffset, bpLen);
			if (!_fstricmp(Arg[1], "TRANSFORM"))
			{
				if (TransformShapeFile(Arg[2], Arg[3], Arg[4], Arg[5]))
				{
					goto RtnTrue;
				}
			}
			if (!_fstricmp(Arg[1], "BOUNDS"))
			{
				MNMXCORD bounds;
				if (GetShapeBounds(Arg[2], &bounds))
				{
					boundstoa(OutLoc, &bounds);
					goto Rtnl;
				}
			}

			if (!_fstricmp(Arg[1], "TYPE"))
			{
				int type;
				if (GetShapeType(Arg[2], &type))
				{
					itoa(type,OutLoc,10);
					goto Rtnl;
				}
			}

			if (!_fstricmp(Arg[1], "NUMRECS"))
			{
				int numrecs;
				if (GetShapeNumRecs(Arg[2], &numrecs))
				{
					itoa(numrecs, OutLoc, 10);
					goto Rtnl;
				}
			}

			if (!_fstricmp(Arg[1], "PARMCOPY"))//$SHP(PARMCOPY,fromfile,tofile,startref)
			{
				int startref = atoi(Arg[4]);
				if (CopySHPParm(Arg[2], Arg[3], startref))
				{
					goto RtnTrue;
				}
			}

			if (!_fstricmp(Arg[1], "TEST"))//$SHP(TEST,type,file)
			{
				if (main_shptest(nArgs-1,&Arg[1]))
				{
					goto RtnTrue;
				}
			}

			goto RtnFalse;
		}
		case 361://$MSG(message,opt(0=create window,1=destroy window),vp(0 = current,-1 = main window,+ is vpid))
		{
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			HWND hMess;
			int wid = atoi(Arg[3]);
			if (atoi(Arg[2]) == 0)
			{
				hMess = CreateGoogleMessage(Arg[1],wid);
				lltoa((LONGLONG)hMess,OutLoc, 10);
				goto Rtnl;
			}
			else
			{
				hMess = (HWND)atoll(Arg[1]);
				DestroyWindow(hMess);
				goto RtnTrue;
			}
		}

		case 401: /* $ZOOM(HLT,hltnum,offset,fromlimits,immediate)
						   ITEM,TAG or Refno,area offset,viewport offset,immediate,vpname(opt))	
					 	   RECT,minx,miny,maxx,maxy)
					 	   COOR,xcor,ycor)			 
					 	   BITMAP,bitmap path name)  zooms to bitmap minmax 
					 	   SCALE,scale or REMOVE 
					 	   POINT,point,immediate
					 	   CIRCLE,point,distance,immediate
						   FROMCONNECTEDPROCESS,point,scale,fromdir)*/
		{	   
			BOOL	FromLimits, Immediate; 
			
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen);
			if (!_fstricmp(Arg[1],"COOR"))
			{
				if (nArgs < 3)
					goto Rtn0;
			}
			else if (!_fstricmp(Arg[1],"SCALE"))
			{   
				SetCurView(SetVPFromName(Arg[3], &Err));
				if (SetScale(Arg[2]))
					goto RtnTrue;
				goto RtnFalse;
			}
			else if (!_fstricmp(Arg[1],"VIRTUALPRINT"))
			{   
				if (ZoomToVirtualPrint())
					goto RtnTrue;
				goto RtnFalse;
			}
			else if (!_fstricmp(Arg[1],"VISLIM"))
			{   
				 Immediate = atob(Arg[2]);
				 if (*Arg[3])
					SetCurView (SetVPFromName (Arg[3],&Err));   
				 GetVisBounds (&Bounds,CurView->hDC); 
		 	     CurView->CurZoomAreaRef = LONG_MAX - 1;
		 	     CurView->WindowZoomedToOrtho = FALSE;
				 ZoomToRect(Bounds,Immediate);
				 goto RtnTrue;
			}   
            	
			else if (!_fstricmp(Arg[1],"FACTOR"))
			{   
				 RVal = atof (Arg[2]);
				 if (*Arg[3])
					SetCurView (SetVPFromName (Arg[3],&Err));   
                 ZoomWindow (CurView->hWnd, RVal,atob(Arg[4]));
				 goto RtnTrue;
			}   
            	
			else if (!_fstricmp(Arg[1],"MASK"))
			{   
				LPMNMXCORD	pBounds;
				
				SetCurView (SetVPFromName (Arg[3],&Err));   				
				if (CurView->hMaskArea) 
			    {
			    	pBounds = (LPMNMXCORD) GlobalLock (CurView->hMaskArea);
			    	Bounds = *pBounds;
			    	GlobalUnlock (CurView->hMaskArea);
					Immediate = atob(Arg[2]);
					ZoomToRect(Bounds,Immediate);   
					goto RtnTrue;
			    } 
			    goto RtnFalse;
			}
			else if (!_fstricmp(Arg[1],"EDLIM"))
			{   
    			 if (!GetLayerBounds (&Bounds,CurView->hDC, CurView->UpdateFile-1))
    			 	goto RtnFalse;
				 Immediate = atob(Arg[2]);
			     CurView->CurZoomAreaRef = 0;
				 ZoomToRect(Bounds,Immediate);
				 goto RtnTrue;
			}   
			else if (!_fstricmp(Arg[1],"HLTLIM"))
			{   
				 Offset = atof(Arg[2]);
				 Immediate = atob(Arg[3]);
				 Bounds = HLTBounds; 
				 ExpandBounds (&Bounds,Offset);
				 SetCurView (SetVPFromName (Arg[4],&Err));
			     CurView->CurZoomAreaRef = 0;
				 ZoomToRect(Bounds,Immediate); 
				 CurView = SaveVP;
				 goto RtnTrue;
			}   
			else if (!_fstricmp(Arg[1],"BITMAP"))
			{    
				 Fid = GSSiOpenFile (Arg[2],0,OF_READ);
				 if (Fid == HFILE_ERROR)
				 	goto RtnFalse;
				 if (ReadBitMapHeader (Fid,&hDibInfo, &ImageOffset))
				 {    
				    LPBITMAPINFO    pDibInfo=(LPBITMAPINFO)GlobalLock (hDibInfo);
				    
				    Bounds.xmn = 0;
				    Bounds.ymn = 0;
				    Bounds.xmx = pDibInfo->bmiHeader.biWidth;
				    Bounds.ymx = pDibInfo->bmiHeader.biHeight;
				    GSSiClose2 (&Fid);  
				    GSSiGlobUlFree (&hDibInfo);
					Immediate = atob(Arg[3]); 
				    CurView->CurZoomAreaRef = 0;
					ZoomToRect(Bounds,Immediate);
				 	goto RtnTrue;
				 }  else
				 	GSSiClose2 (&Fid);
				 goto RtnFalse;
			}   
			else if (!_fstricmp(Arg[1],"ELEMENT"))
			{   
				if (!stricmp (Arg[2],"PICKED"))
					NumPicked = 1;
				else
				{
					if ((lpColon = _fstrchr (Arg[2],':')))
						*lpColon++=0;
					else 
					{
						Refno = atol (Arg[2]); 
						Arg[2] = 0;
					}
					if (!PickByRefno (Refno,Arg[2],lpColon,-1))
						goto RtnFalse;
				}
				if ((PickList[0].Type == 2 || PickList[0].Type == 3))
				{
					int nPnts, wantPt = atoi (Arg[3]);
					HANDLE hPnts;

					if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPnts,&hPnts, 0))
					{
						LPDPOINT pt = GlobalLock (hPnts);
						if (wantPt < nPnts)
							ZoomToPointAndScale (pt[wantPt],CurView->Scale,atob(Arg[4]));
						GSSiGlobUlFree (&hPnts);
					}
				}
			}
			else if (!_fstricmp(Arg[1],"ITEM"))
			{   
				if (!stricmp (Arg[2],"PICKED"))
					NumPicked = 1;
				else
				{
					if ((lpColon = _fstrchr(Arg[2], ':')))
					{
						*lpColon++ = 0;
						Refno = 0;
					}
					else 
					{
						Refno = atol (Arg[2]); 
						Arg[2] = 0;
					}
					if (!PickByRefno (Refno,Arg[2],lpColon,-1))
						goto RtnFalse;
				}
	            {   
	            	double MaskOffset;
                    LPVIEWPORT	SaveVP=CurView;
					BOOL	DoOffset=TRUE;
                    
					SetCurView ( SetVPFromName (Arg[6],&Err)); 
					if (*Arg[3] == 'n' || *Arg[3] == 'N')
						DoOffset = FALSE;
					else
						MaskOffset = atobasedist (Arg[3],&Err);
					if (!*Arg[3] && !*Arg[4])
						Offset = LocationOffset;
					else
						Offset = atobasedist (Arg[4],&Err);
					if (*Arg[5] == 'S')
					{
						BOOL SaveDisplay = Display;

						Display = FALSE;
						ZoomToPickedItem (NumPicked-1,Offset,FALSE,FALSE,FALSE);
						Display = SaveDisplay;
					}
					else
					{
						Immediate = atob(Arg[5]); 
						if (DoOffset)
						{
							if (MaskOffset || *Arg[3] == '-')
								SetMaskArea(NumPicked-1,MaskOffset,1);  
							else if (MaskOffsetLine)
								SetMaskArea(NumPicked-1,MaskOffset,1);  
						}
						ZoomToPickedItem (NumPicked-1,Offset,OffsetFromLimits,Immediate,FALSE);
					}
					SetCurView ( SaveVP);
	              	goto RtnTrue;
	            }   
			}
			else if (!_fstricmp(Arg[1],"HLT") || !_fstricmp(Arg[1],"RECT"))
			{   
				if (nArgs < 5)
					goto Rtn0;
			}
			else if (!_fstricmp(Arg[1],"BOUNDS"))
			{
				BOOL RestoreTheme=FALSE; 
				short	SaveVPID;
				LPTHEME	SaveTheme;
				
				Bounds = atobounds (Arg[2],&Err);
				if (Err)
					goto RtnFalse;
				Immediate = atob(Arg[3]); 
				if (*Arg[4])
					SetCurView ( SetVPFromName (Arg[4],&Err));   
			    CurView->CurZoomAreaRef = 0; 
			    if (Immediate && CurView->pTheme)
			    {
			    	if (CurView->Type == 7 && CurView->pTheme->ID == GF_BOUNDS_DISPLAY_THEME) 
			    	{
			    		RestoreTheme = TRUE;   
			    		SaveVPID = CurView->ID;
			    		CurView->Type = 1; 
			    		SaveTheme = CurView->pTheme;
			    		CurView->pTheme = 0;     
			    		Immediate = FALSE;
			        	DestroySavedScreen (&CurView->Bitmap,CurView->BitmapID);
			    	}
			    } 
				ZoomToRect(Bounds,Immediate); 
				if (RestoreTheme)
				{          
	      			DisplayCycle++;
					Immediate = TRUE;
					SetViewport (SaveVPID);
					CurView->Type = 7;
					CurView->pTheme = SaveTheme; 
					SetViewport (CurView->ZoomTarget);
			     	BoundsDisplayShow (CurView->lpBoundsDisplay,&CurView->BoundsDisplayed);   
     		     	BoundsDisplayTheme (FALSE); 
				}
				SetCurView ( SaveVP);
				goto RtnTrue;
			}
			else if (!_fstricmp(Arg[1],"RECT"))
			{   
				
				Bounds.xmn = atof (Arg[2]);
				Bounds.ymn = atof (Arg[3]);
				Bounds.xmx = atof (Arg[4]);
				Bounds.ymx = atof (Arg[5]); 
			    CurView->CurZoomAreaRef = 0;
				ZoomToRect(Bounds,FALSE);
			} 
			else if (!_fstricmp(Arg[1],"COOR"))
			{   
				
				Point.x = atof (Arg[2]);
				Point.y = atof (Arg[3]);
/*				if (Highways)
				{   
					LPPICKDATA	pPickList; 

					ShowWindow(hWndMain, SW_SHOWMAXIMIZED);
					SetWindowPos(hWndMain, (HWND) HWND_TOPMOST, 0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
				    PickItems (hWndMain,Point); 
	 				NumSavedPickList = NumPicked;
	 				if (NumSavedPickList)
	 				{    
	 					hSavedPickList = GSSiGlobAlloc(GAIDNO 825,GMEM_MOVEABLE,NumSavedPickList*sizeof(PICKDATA));
	 					pPickList = (LPPICKDATA)GlobalLock (hSavedPickList);
	 					_fmemmove (pPickList,PickList,NumSavedPickList*sizeof(PICKDATA));
	 					GlobalUnlock (hSavedPickList);
		 				DisplayFinOpt = 1; 
		 			}
				}*/
				CenterWindow (Point,FALSE); 
			} 
			else if (!_fstricmp(Arg[1],"POINT"))
			{   
				Point = atopt (Arg[2],&Err);
				if (Err)
					goto RtnFalse;
				Immediate = atob(Arg[3]);
				if (*Arg[4])
					SetCurView ( SetVPFromName (Arg[4],&Err));   
				CenterWindow (Point,Immediate); 
			}
			else if (!_fstricmp(Arg[1],"CIRCLE"))
			{   
				Point = atopt (Arg[2],&Err);
				if (Err)
					goto RtnFalse; 
				Offset = atobasedist (Arg[3],&Err);
				Immediate = atob(Arg[4]);
				SetCurView ( SetVPFromName (Arg[5],&Err)); 
				ZoomToPointAndDist (Point,Offset,Immediate); 
			}
			else if (!_fstricmp(Arg[1], "POINTANDSCALE"))
			{
				Point = atopt(Arg[2], &Err);
				if (Err)
					goto RtnFalse;
				Offset = atobasedist(Arg[3], &Err);
				if (!stricmp(Arg[4], "-1"))
					Immediate = -1;
				else
					Immediate = atob(Arg[4]);
				SetCurView(SetVPFromName(Arg[5], &Err));
				ZoomToPointAndScale(Point, Offset, Immediate);
			}
			else if (!_fstricmp(Arg[1], "FROMCONNECTEDPROCESS"))
			{
				char fromProjection[MAX_PATH];
				double Scale;

				Point = atopt(Arg[2], &Err);
				if (Err)
					goto RtnFalse;
				Scale = atof(Arg[3]);
				if (*Arg[4])
				{
					double factor = 1;
					DPOINT points[2];
					double dist[2];

					sprintf(fromProjection, "%sbaseproj.cvt", Arg[4]);
					points[0].x = Point.x - 1;
					points[0].y = Point.y - 1;
					points[1].x = Point.x + 1;
					points[1].y = Point.y + 1;
					dist[0] = ldistp(points[0], points[1]);
					ConvertPoint(fromProjection, &Point, 1);
					ConvertPoint(fromProjection, &points[0], 1);
					ConvertPoint(fromProjection, &points[1], 1);
					dist[1] = ldistp(points[0], points[1]);
					factor = dist[0] / dist[1];
					Scale *= factor;
				}
				if (NumViewportsArray[1])
					SetConfig(1);
				SetCurView(SetVPFromName("COMMAND", &Err));
				if (!Scale)
					Scale = CurView->Scale;
				fromConnectedProcess = TRUE;
				ZoomToPointAndScale(Point, Scale, TRUE);
				fromConnectedProcess = FALSE;
			}
			else
			{
				n = atoi(Arg[2]);
				Offset = atof(Arg[3]);
				FromLimits = atoi(Arg[4]);
				Immediate = atoi(Arg[5]);
				ZoomToPickedItem (n,Offset,FromLimits,Immediate,FALSE);
			}

			goto RtnTrue;
		} 
		   
		case 405: /* $FCHR(string,char) returns 1 based position of first char in string*/ 
		{	long pos;
			LPSTR	loc;
			
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			loc = _fstrchr (Arg[1],*Arg[2]);
			if (!loc) goto Rtn0; 
			pos = (long)loc - (long)Arg[1] + 1;
			ltoa (pos,OutLoc,10);
			goto Rtnl;
		}

		case 406: /* $LCHR(string,char) returns 1 based position of last char in string*/ 
		{	long pos;
			LPSTR	loc;
			
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			loc = _fstrrchr (Arg[1],*Arg[2]);
			if (!loc)
				goto Rtn0; 
			pos = (long)loc - (long)Arg[1] + 1;
			ltoa (pos,OutLoc,10);
			goto Rtnl;
		}

		case 407: /* $FILL(val,len,fillchar) left fills val with fillchar to length len */ 
		{	double	rval;
			int		l, len; 
			LPSTR	lpOut; 
			char	fillchar='0';
			
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			
			l = _fstrlen(Arg[1]);
			len = IDNINT(atof (Arg[2])); 
			if (*Arg[3])
				fillchar = *Arg[3];
			lpOut = OutLoc;
			while (l++ < len)
				*lpOut++ = fillchar;
			_fstrcpy (lpOut,Arg[1]);
			goto Rtnl;
		} 
		
		case 408: // $OPEN(DBNAME,Optional SQL,Mode(R or W)) opens database
		{	 
			int		nRc;
			short	Mode=BT_READ; 
			HFILE	Fid;
			
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse; 
			switch (*Arg[3])
			{
				default:
					Mode = BT_READ; 
				break;
				
				case 'W':
				case 'w':
					Mode = BT_WRITE;
				break;
				case 'F':
				case 'f':
					Mode = -1;//field list
				break;
				case 'c'://forces cache check
				case 'C':
					{
						HFILE	Fid = GSSiOpenFile (Arg[1],0,OF_READ);

						if (Fid != HFILE_ERROR)
							GSSiClose2 (&Fid);
					}
					goto RtnTrue;
				break;
			}

			{  

			    double      rtn;
			    
			    rtn = FALSE;
			    hSQL=0;
				if (!stricmp (Arg[1],"HLTLIST"))
				{
					HltFetchSort = atoi (Arg[2]);
					*Arg[2] = 0;
				}
			    if (!OpenDataFile (Arg[1],Arg[2],Mode,&hSQL))
			        goto RtnFalse;
            } 
			goto RtnTrue;
		}
		
		case 409: /* $EDIT(Layer Name,VPname(opt))  changes edit layer in current viewport*/
		{	
			short	i;
			
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			SaveVP = CurView;      
			SetCurView ( SetVPFromName (Arg[2],&Err));  
			if (!CurView)
				goto RtnFalse;
			Truncate (Arg[1]);   
			if (*Arg[1] == '?')
			{
				itoa (CurView->UpdateFile,OutLoc,10);
				goto Rtnl;
			}
			CurView->UpdateFile = 0;
			if (!*Arg[1])
				goto RtnTrue;  
			CurView->UpdateFile = GetLayerNumFromName (Arg[1]);
			if (CurView->UpdateFile)
			{
		   		_fstrcpy (EditName,CurView->lpFiles[CurView->UpdateFile-1]);
				GetLayerBounds (&EditBounds,CurView->hDC, CurView->UpdateFile-1);
				goto RtnTrue;
			}
			goto RtnFalse;
		}
		
		case 410: //$BOOL(str)
		{
			BOOL val;      
			BOOL irc;
			
			val = LogicP (Args,&irc);
		    if (irc)
		    {
		    	switch (irc)
		    	{
		    		case 1:
		    			SetGlobalValue ("%LASTERR","Non-numeric in numeric field");
		    			break;
		    		case 2:
		    			SetGlobalValue ("%LASTERR","Mismatched parenthesis");
		    			break;
		    		case 3:
		    			SetGlobalValue ("%LASTERR","Invalid expression");
		    			break;
		    		case 4:
		    			SetGlobalValue ("%LASTERR","Divide by 0");
		    			break;
		    		case 5:
		    			SetGlobalValue ("%LASTERR","Invalid operand");
		    			break;
		    	}
		    	SetGlobalValue ("%C","F");
		    	goto RtnFalse; 
		    }
		    if (val)
		    	goto RtnTrue;
		    else
		    	goto RtnFalse;
		}
		case 411: /* $INDX(string1,string2) returns 1 based position of string2 in string1 */ 
		{	long pos;
			LPSTR	loc;
			
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			loc = _fstrstr (Arg[1],Arg[2]);
			if (!loc)
				goto RtnFalse; 
			pos = (long)loc - (long)Arg[1] + 1;
			ltoa (pos,OutLoc,10);
			goto Rtnl;
		}

		case 412: /* $TEST(string1,string2,Message) tests string1 vs string2. If not same displays message and returns false */ 
		{	
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 3)
				goto RtnFalse;
			if (!_fstricmp (Arg[1],Arg[2])) 
				goto RtnTrue;
			if (*Arg[3])
           		GSSiMessageBox (0,Arg[3],0,MB_ICONEXCLAMATION,0);
			goto RtnFalse;
		} 
		
		case 413: // $MASK(CLEAR)  
 		{	
 			 
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 
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
				ClearMaskArea ();
				CurView = SaveVP; 
				goto RtnTrue;
			} 
			else if (!_fstrcmp (Arg[1],"SAVEAREAS")) 
			{
				if (SaveMaskAreas (Arg[2]))
					goto RtnTrue;
				goto RtnFalse;
			}   
			else if (!_fstrcmp (Arg[1],"CHECK")) 
			{   
				if (*Arg[2])
				{
					SetCurView ( SetVPFromName (Arg[2],&Err));  
					if (Err)
						goto RtnFalse;
				} 
				if (CurView->hMaskArea)
					rtn = TRUE;
				else
					rtn = FALSE;
				CurView = SaveVP; 
				if (rtn)
					goto RtnTrue;
				goto RtnFalse;
			} 
			else if (!_fstrcmp (Arg[1],"SET")) 
			{       
				short	Item=0;
	 			short	opt=1;
				
				if (*Arg[5])
				{
					SetCurView ( SetVPFromName (Arg[5],&Err));  
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
					pHighlightData = (LPHIGHLIGHTDATA)Arg[6];
    				BT_FIND (hHighlight,(LPSTR)&Refno,BT_FIRST,BT_ANY,(LPSTR)pHighlightData);
					PickList[0]=pHighlightData->PD;
					Item = 1;
				}
				Dist = atobasedist (Arg[3],&Err);
				if (!(opt = atoi (Arg[4])))
					opt = 1; 
				if (SetMaskArea(Item-1,Dist,opt))
					goto RtnTrue;
			} 
			goto RtnFalse;
			
		}
		
		case 414: /* $SAVE(filenum) fetch next rec */
		{				
			HANDLE	hSQLPtr; 
			
			hMem = GSSiGlobAlloc(GAIDNO 829,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
	        
	        if (!_fstricmp (Arg1,"HLTLIST"))
        		goto RtnFalse;
        	hSQLPtr = GetDBByIDName (Arg1);
        	if (!hSQLPtr)
        		goto RtnFalse; 
			if (SaveDBRec (hSQLPtr))
				goto RtnTrue;
			else
				goto RtnFalse;
		}

		case 415: /* $DIST(point1,point2) */ 
		{	double dist;
			
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 2)
				goto RtnFalse;
			Point = atopt (Arg[1],&Err);
			Point2 = atopt (Arg[2],&Err);
			dist = ldistp (Point,Point2);
			ftoa (OutLoc,dist);
			goto Rtnl;
		}  
		
		case 416: // $MENU(filenam) display menu file
				  // $MENU() draws main menu bar
				  // $MENU(REDRAW) redraws gftheme menu
				  // $MENU(SET,title,commandline) sets state of gftheme menu
		{				
			POINT	position;
			HMENU	hMenu;
			HANDLE	hPopups;
			
			nArgs = GetFunArgs (Args,Arg,-3,&hMem, pBrkPt, bpOffset, bpLen); 
	        
	        if (!nArgs)
	        {   
	        	DrawMenuBar (hWndMain);
	        	goto RtnTrue;
	        } 
	        ExpandText (Arg[1]);
	        if (!_fstricmp (Arg[1],"REDRAW"))
	        {   
	        	short	SaveVPID = CurView->ID;
	        	short	SaveConfig = CurrentConfig;
	        	
	        	SetConfig (LastMenuConfig);
	        	SetViewport (LastMenuVPID);
	        	ThemeDisplayLegend(3,CurView->ID); 
	        	SetConfig (SaveConfig);
	        	SetViewport (SaveVPID);
	        	goto RtnTrue;
	        }
	        if (!_fstricmp (Arg[1],"SET"))
	        {   
	        	ExpandText (Arg[2]);
		        if (SetGFThemeState (CurTheme,Arg[2],Arg[3]))
	        		goto RtnTrue;
	        	goto RtnFalse;
	        }
	        if (!_fstricmp (Arg[1],"RELOAD"))
	        {  
				rtn = ReloadMainMenu ();
				goto Rtnrtn;
			}
            hMenu = LoadToolBarMenu (Arg[1],&hPopups); 
            if (hMenu)
            {   
				HANDLE	hScreen;
				HDC		hDC = GetDC (hWndMain);
				RECT	Rect;
				int		icmd;

				GetClientRect(hWndMain,&Rect);
				hScreen = SaveScreen2 (hWndMain,hDC,Rect,0,0);
            	MenuDisplayed = TRUE;
			   	GetCursorPos (&position);
				icmd = TrackPopupMenu (hMenu,TPM_RIGHTBUTTON ,position.x,position.y,0,hWndMain,0);
				SetCursorPos (position.x,position.y);
				DestroyUserPopups (&hPopups); 
	    		RestoreScreen2 (hDC, hScreen,0,FALSE);
	    		DestroySavedScreen (&hScreen,0);
				ReleaseDC (hWndMain,hDC);
				MenuDisplayed = FALSE;
				goto RtnTrue;
			}
			else
				goto RtnFalse;
		} 
		
		case 417:  //$HOUR(returns hour of day (0-23) from system time)
			hMem = GSSiGlobAlloc(GAIDNO 831,GMEM_MOVEABLE,2048*6);
			Arg1 = GlobalLock(hMem); 
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			systime = atol (Arg1);
			if (systime < 0)
				goto RtnFalse;
			tmtime = *localtime (&systime); 
			itoa (tmtime.tm_hour,OutLoc,10);                                              
			goto Rtnl;
			
		case 418: //$TRIM(maxdist) trims highligted data to last highlighted item 
			hMem = GSSiGlobAlloc(GAIDNO 832,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			Dist = atof (Arg1);
			if (TrimHighlightedItems (Dist))
				goto RtnTrue;
			else
				goto RtnFalse;
			
		case 419: //$PICK(point,vpname)  
			nArgs = GetFunArgs (Args,Arg,6,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			if (!stricmp(Arg[1], "SAVE"))
			{
				HANDLE hSavedList = SavePickList();
				ii = sizeof(HANDLE);
				ltoa((LONG)hSavedList, OutLoc, 10);
				goto Rtnl;
			}
			else if (!stricmp(Arg[1], "DESTROY"))//$PICK(DESTROY,handle to saved list)
			{
				HANDLE hSavedList = (HANDLE)atol(Arg[2]);
				GSSiGlobFree(&hSavedList);
				goto RtnTrue;
			}
			else if (!stricmp(Arg[1], "SELECT"))
			{
				n = atoi (Arg[2]);
				if (n > NumPicked)
					goto RtnFalse;
				SetPickGlobals (n-1);
				goto RtnTrue;
			}
			else if (!stricmp(Arg[1], "REFNO"))//$PICK(REFNO,refno,file,vp)sets pick globals for item
			{
				short	pickfile = atoi (Arg[3]);
				SetCurView(SetVPFromName(Arg[4], &Err));
				if (pickfile > -1)
					pickfile += CurView->ID * 256;
				if ((lpColon = _fstrchr(Arg[2], ':')))
					*lpColon++ = 0;
				else
				{
					Refno = atol(Arg[2]);
					Arg[2] = 0;
				}
				if (PickByRefno(Refno, Arg[2], lpColon, pickfile))
				{
					SetPickGlobals(0);
					goto RtnTrue;
				}
				goto RtnFalse;
			}

			else if (!stricmp (Arg[1],"ITEM"))//$PICK(ITEM,refno or tag,point,vp) picks nearest point on item to point
											  //$PICK(ITEM,refno or tag) sets pick globals for item
			{
				short	UsePickList=-1;

				if ((lpColon = _fstrchr (Arg[2],':')))
					*lpColon++=0;
				else 
				{
					Refno = atol (Arg[2]); 
					Arg[2] = 0;
				}
				if (!*Arg[3])
				{
					Refno = 0;
					SetCurView(SetVPFromName(Arg[4], &Err));
					int st = PickByRefno(Refno, Arg[2], lpColon, UsePickList);
					if (st)
					{
						SetCurView(SaveVP);

						SetPickGlobals(0);
						itoa(st,OutLoc, 10);
						goto Rtnl;
					}
					SetCurView(SaveVP);

					goto RtnFalse;
				}
				Point = atopt (Arg[3],&Err);
				if (Err)
					goto RtnFalse;
				SetCurView ( SetVPFromName (Arg[4],&Err));
	            if (!PickByRefno (Refno,Arg[2],lpColon,UsePickList))
	            {
					CurView = SaveVP; 
					goto RtnFalse;
				}
				else 
				{
					LPINT	nLoopPoints = (LPINT)GSSiGlobAlloc(GAIDNO 1987,GMEM_MOVEABLE,USHRT_MAX*sizeof(int));
					LPHANDLE	hLoopPoints = (LPHANDLE)GSSiGlobAlloc(GAIDNO 1988,GMEM_MOVEABLE,USHRT_MAX*sizeof(HANDLE));
					int nLoops;
					double NearDist;

			    	SetPickGlobals (0); 
					CurView = SaveVP; 
					nLoops = GetPolyPoints2 ((LPPICKDATAHEADER)&PickList[0],nLoopPoints,hLoopPoints,UINT_MAX);
					for (i=0;i<nLoops;i++)  
		    		{   
						HPDPOINT	pLoopPoints = (HPDPOINT)GlobalLock (hLoopPoints[i]);
						PickNearPolylineD (2,pLoopPoints,nLoopPoints[i],0,&Point,&NearDist,0);
						
						GlobalUnlock (hLoopPoints[i]);  
					}
					goto RtnTrue;
				}
			}
			Point = atopt (Arg[1],&Err);
			if (Err)
				goto RtnFalse;
			SetCurView ( SetVPFromName (Arg[2],&Err));  
			skipSetCursor = TRUE;
			n = PickItems (hWndMain,Point); 
			skipSetCursor = FALSE;

			SetCurView ( SaveVP);  
			if (n)
				ProcessPickedItem (n-1,FALSE);			
			itoa (n,OutLoc,10);
			goto Rtnl; 
			
		case 420: // $NULL(value)  expands value but returns nothing
		{				
			hMem = GSSiGlobAlloc(GAIDNO 834,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1);
			*OutLoc = 0;
   			goto Rtnl;
		}
		
		case 421: //$SNAP(POINT,)
		{
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			Point = atopt (Arg[2],&Err);
			if (Err)
				goto RtnFalse; 
		    SnapToPoint (Point);
		    goto RtnTrue;
		}

		case 422: // $EXIT(message) exits geomaster with message
		{	
			LPSTR	ExitMessage;
						
			hMem = GSSiGlobAlloc(GAIDNO 834,GMEM_MOVEABLE,4096);
			Arg1 = GlobalLock(hMem);
			
			_fstrcpy (Arg1,Args);
			ExpandText (Arg1); 
			if (*Arg1)
			{
				hExitMessage = GSSiGlobAlloc(GAIDNO 1989, GMEM_MOVEABLE, 512);
				ExitMessage = GlobalLock(hExitMessage);
				_fstrcpy(ExitMessage, Arg1);
				GlobalUnlock(hExitMessage);
			}
			PostMessage(hWndMain, WM_CLOSE, 0, 0L);
   			goto RtnTrue;
		}

		case 423: //$UNDO()
		{
			nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 0)
				goto RtnFalse;

			if (UndoChanges (hWndMain))  
				RedisplayWindow();            
		    goto RtnTrue;
		}
		
		case 424: //$MISC()
		{
			double v = FTM;
			v = MFT;

			//int i = ConvertToJP2(2020, 7);
				
				OFSTRUCTGM OFStruct;
				char netFile[MAX_PATH] = "C:\\temp\\fromimg.txt";
				char outFile[MAX_PATH] = "C:\\temp\\toimg.txt";
				HANDLE FidIn = OpenFileGM(netFile, &OFStruct, OF_READ);
				HANDLE FidOut = OpenFileGM(outFile, &OFStruct, OF_CREATE);
				int len = 1024 * 1024 * 1024;
				//len = 64;
				LPSTR pBuf = malloc(len + 4);
				BigRead64(FidIn, pBuf, len);
				double x, y, z;

				BigWrite64(FidOut, pBuf, len,-1);
				GSSiClose64(&FidIn);
				GSSiClose64(&FidOut);
				goto RtnTrue;
				/*
				char cacheFile[MAX_PATH] = "C:\\Users\\smithjx0\\AppData\\Local\\Temp\\gmcache2\\ORTHOS\\ORTH2019\\2019_1\\ORTHOS4$GCI.tbr";
				HANDLE FidNet = OpenFileGM(netFile, &OFStruct, OF_READ);
				HANDLE FidCache = OpenFileGM(cacheFile, &OFStruct, OF_READ);
				BYTE byteNet, byteCache;
				int n = 0;
				DWORD nread = 0;
				while (ReadFile(FidNet, &byteNet, 1,&nread,FALSE))
				{
					ReadFile(FidCache, &byteCache, 1,&nread,FALSE);
					if (byteNet != byteCache)
						ii = 1;
					if (byteCache != 0)
						ii = -1;
					n++;
				}
				goto RtnTrue;
			}
			{
				HFILE Fid = GSSiOpenFile("[%DL]data\\parcelfiles\\textfiles\\ChangeValues.bin", 0, OF_READ);
				int nyears, firstyear, lastyear;
				BigRead(Fid, &nyears, 4);
				BigRead(Fid, &firstyear, 4);
				BigRead(Fid, &lastyear, 4);
				GSSiClose(Fid);
			}
			HDC hDC = GetDC(hWndMain);
			int iLogPixsX = GetDeviceCaps(hDC, LOGPIXELSX);
			int pageWidth = GetDeviceCaps(hDC, HORZRES);
			double pageSize = GetDeviceCaps(hDC, HORZSIZE) * 0.0393701;
			double scale = (double)pageWidth / (double)pageSize;
			ReleaseDC(hWndMain, hDC);
			ftoa(OutLoc, scale);
			*/
			//SetDisplayMode(hDC, GF_TEXTMODE);
			//testGDIP(hDC);
/*			char SSID[40];
			char ipAddress[32];
			GUID Guid;*/
			//BOOL TestSQLiteCrimes(LPMNMXCORD pBounds, int fromDate, int toDate, int fromUCR, int toUCR);

			//int n = TestSQLiteCrimes(&CurView->WBounds, TimeRangeBeg, TimeRangeEnd, 1, 10);
			//BOOL TestSQLiteCrimeOffenseOrder(LPMNMXCORD pBounds, int fromDate, int toDate, int fromUCR, int toUCR);

			//int n = TestSQLiteCrimeOffenseOrder(&CurView->WBounds, TimeRangeBeg, TimeRangeEnd, 1, 10);

			//itoa(n, OutLoc, 10);
			//goto Rtnl;
			/*{
#include "colorsByName.h"
				HDC hDC = CurView->hDC;
				float w = RECTWIDTH(&CurView->DrawRect)/7.0;
				float h = RECTHEIGHT(&CurView->DrawRect)/20.0;
				int icolor = 0;
				for (int irow = 0; irow < 20; irow++)
				{
					for (int icol = 0; icol < 7; icol++)
					{
						RECT rect;
						COLORREF color = RGB(R[icolor], G[icolor], B[icolor]);
						icolor++;
						rect.left = CurView->DrawRect.left + icol * w;
						rect.right = rect.left + w + 1;
						rect.top = CurView->DrawRect.top + irow * h;
						rect.bottom = rect.top + h + 1;
						FillRectColor(hDC, &rect, color);
					}
				}
				GdiFlush();
				HBITMAP hbmp = SaveScreen(hDC, CurView->DrawRect);
				GM32SaveBitmap(hbmp, "c:\\temp\\colormap.bmp", 0, 0);
				DeleteObject(hbmp);

			}*/
			/*netAdaptTest();
			wlanGetCurrentSSID(SSID, sizeof(SSID), &Guid);
			getipAddressForAdapter(ipAddress,&Guid);
			HANDLE hMem = GSSiGlobAlloc(GAIDNO 0, GMEM_MOVEABLE, SHRT_MAX);
			LPSTR  pMem = GlobalLock(hMem);
			HFILE  Fid = GSSiOpenFile("C:\\GEOMas\\projects\\corners\\Macros\\loadadafiles.txt", 0, OF_READ);
			BigRead(Fid, pMem, SHRT_MAX - 2);
			GSSiClose2 (&Fid);
			GSSiGlobUlFree(&hMem);*/
			//GetMassShapeFiles();
			//isLaptop(1);
			//TestConvertToJP2 (1);
			//char	ToFile[MAX_PATH]="c:\\temp\\test.zip";
			//char	FromFile[MAX_PATH]="ftp://ftp.lmic.state.mn.us/pub/data/remote_sensing/naip/2009/naip09_carver.zip";
		//BackgroundUpdateMessage ("This is a test message");
			//DrawAlphaBlend (CurView->hWnd, CurView->hDC);
		//GetModuleFileName(0,ToFile,MAX_PATH);
		/*ii=1;
		nArgs = GetFunArgs (Args,Arg,3,&hMem, pBrkPt, bpOffset, bpLen); 
		{
			double x = -84 - 12.194/60;
			double y = 34 + 6.403/60;
			int	   ix = 0xFF70F77A;
			int	   iy = 0x003D542C;
			DPOINT	DPoint;
			char	txt[128];

			DPoint.x = x;
			DPoint.y = y;

			ii = ConvertCoord (&DPoint,2,GOOGLEMAPSPROJECTION);

			ii = 1;
			sprintf (txt,"%f\t%f\t%i\t%i",y,x,iy,ix);
			AppendFile ("c:\\temp\\davec.txt",txt);

			x = -84 - 12.232/60;
			y = 34 + 6.482/60;
			ix = 0xFF70F734;
			iy = 0x003D54DC;
			sprintf (txt,"%f\t%f\t%i\t%i",y,x,iy,ix);
			AppendFile ("c:\\temp\\davec.txt",txt);

			x = -84 - 12.232/60;
			y = 34 + 6.673/60;
			ix = 0xFF70F734;
			iy = 0x003D5687;
			sprintf (txt,"%f\t%f\t%i\t%i",y,x,iy,ix);
			AppendFile ("c:\\temp\\davec.txt",txt);

			x = -84 - 11.899/60;
			y = 34 + 6.673/60;
			ix = 0xFF70F99E;
			iy = 0x003D5687;
			sprintf (txt,"%f\t%f\t%i\t%i",y,x,iy,ix);
			AppendFile ("c:\\temp\\davec.txt",txt);
		}*/
		/*WP001    N  34° 06.403'         0x003D542C
                W 84° 12.194'         0xFF70F77A
 
		WP002    N  34° 06.482'         0x003D54DC
                W 84° 12.232'         0xFF70F734
 
		WP003    N  34° 06.673'         0x003D5687
                W 84° 12.232'         0xFF70F734
 
		WP004    N  34° 06.673'         0x003D5687
                W 84° 11.899'         0xFF70F99E
 */

		//TestFillTBRows (atoi(Arg[1]),atoi(Arg[2]));
		//testfgdb(1);
	//	LoadMCDS4(1);
		//	CopyFileExtended (ToFile,FromFile);

			//     int nRc = DialogBox(hInst, (LPCSTR)"DECONSTRUCT", CurView->hWnd, DeconstructMsgProc);

			//HWND hWnd = DoCreateTabControl(hWndMain);
			//DoCreateDisplayWindow(hWnd);
			/*LPSTR	x=0;

			HFILE	Fid = GSSiOpenFile ("L:\\geomas\\DataXfer\\cachetestfile.txt",0,OF_READ);

			WaitForKeystroke (TRUE);
			GSSiClose2 (&Fid);
			MessageBox (0,"Closed","",MB_OK);*/


			{
			/*	HFILE Fid1,Fid2;
				int	ndiff1=0, ndiff2=0,len;
				BYTE	val1, val2;

				Fid1 = GSSiOpenFile ("[%DL]attribut\\jtest\\journal\\areaway2.gmd",0,OF_READ);
			//	Fid2 = GSSiOpenFile ("[%DL]attribut\\jtest\\nojournal\\areaway2.gmd",0,OF_READ);
				Fid2 = GSSiOpenFile ("[%DL]attribut\\jtest\\areaway2.gmd",0,OF_READ);

				len = GSSillseek (Fid1,0,2);
				if (GSSillseek (Fid2,0,2) != len)
					MessageBox (0,"Diff len 2",0,MB_OK);
				GSSillseek (Fid1,0,0);
				GSSillseek (Fid2,0,0);
				while (BigRead (Fid1,&val1,1)>0)
				{
					BigRead (Fid2,&val2,1);
					if (val1 != val2)
						ndiff1++;
				}
				GSSiClose2 (&Fid1);
				GSSiClose2 (&Fid2);

				Fid1 = GSSiOpenFile ("[%DL]attribut\\jtest\\journal\\areaway2.in1",0,OF_READ);
			//	Fid2 = GSSiOpenFile ("[%DL]attribut\\jtest\\nojournal\\areaway2.in1",0,OF_READ);
				Fid2 = GSSiOpenFile ("[%DL]attribut\\jtest\\areaway2.in1",0,OF_READ);

				len = GSSillseek (Fid1,0,2);
				if (GSSillseek (Fid2,0,2) != len)
					MessageBox (0,"Diff len 2",0,MB_OK);
				GSSillseek (Fid1,0,0);
				GSSillseek (Fid2,0,0);
				while (BigRead (Fid1,&val1,1)>0)
				{
					BigRead (Fid2,&val2,1);
					if (val1 != val2)
						ndiff2++;
				}
				GSSiClose2 (&Fid1);
				GSSiClose2 (&Fid2);

				sprintf (OutLoc,"Diff:%i - %i",ndiff1,ndiff2);*/
				goto Rtnl;
			}

/*			{
				HANDLE hMem=GSSiGlobAlloc(GAIDNO 0,GHND,4*1024*1024+64);
				LPLONG	pMem=GlobalLock (hMem);
				UINT	i,mx=1024*1024;
				for (i=0;i<mx;i++)
					pMem[i]=rand();
				
				i=mx/2;
				memmove (&pMem[i+1],&pMem[i],i*4);
				i = 10;
				for (i=10;i<100;i++)
					memmove (&pMem[i+1],&pMem[i],(mx-i)*4);
				GSSiGlobUlFree (&hMem);
			}*/

//			strcpy (x,"Crash");

//			SetViewport(*pCommandViewport);
//			TestHBird (CurView->hDC);
//			ConvertHBirdImages ("[%DL]grids\\images.gmd","[%DL]grids\\images_cvt.gmd");
		}

		    goto RtnTrue;
		case 425: //$HELP(help file,topic)
		{
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 0)
				goto RtnFalse;

			if (!*Arg[1])
				GetGlobalCVal ("[%HELPFILE]",Arg[1],"[%DL]help\\gmhelp.chm");
			ExpandText (Arg[1]);
			DisplayHTMLHelp(Arg[1],Arg[2]);   
		    goto RtnTrue;
		}
		case 426: //$POLY(Ref or TAG or PICKED or POINTS|pointlist,LENGTH)
				//		 (Ref or TAG or PICKED or POINTS|pointlist,POINT,dist)
				//		 (Ref or TAG or PICKED or POINTS|pointlist,AZ,dist)
				//		 (Ref or TAG or PICKED or POINTS|pointlist,OFFSET,dist,offsetdist)
				//		 (Ref or TAG or PICKED or POINTS|pointlist,DISPLAY,width,color,vp,cap)
		{
			int			nPoints;
			HANDLE		hPoints=0;
			LPDPOINT	pPoints;
			LPSTR		Prefix,lpColon;
			BOOL		UnSplined=TRUE;

			nArgs = GetFunArgs (Args,Arg,8,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			if (!stricmp(Arg[1], "SAVE"))
			{
				if (SaveAreasToFile(Arg[2]))
					goto RtnTrue;
				goto RtnFalse;
			}
			else if (!stricmp (Arg[1],"PICKED"))
			{
				if (!GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPoints,&hPoints, 0))
					goto RtnFalse;
			}
			else if (!strnicmp (Arg[1],"POINTS|",7))
			{
				if ((nPoints = GetPointsFromList (Arg[1]+7,&hPoints)) <2)
				{
					GSSiGlobFree (&hPoints);	
					goto RtnFalse;
				}
			}
			else
			{
				if ((lpColon = _fstrchr (Arg[1],':')))
				{
					Prefix = Arg[1];
					*lpColon++=0;
				}
				else
				{
					Prefix = 0;
					Refno = atol (Arg[1]); 
				}

				if (!PickByRefno (Refno,Prefix,lpColon,-1))
					goto RtnFalse;
				if (!GetPolyPnts ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPoints,&hPoints,UnSplined))
					goto RtnFalse;
			}
			pPoints = GlobalLock (hPoints);
			if (nArgs == 1)
			{
				*OutLoc = 0; 
				for (i=0;i<nPoints;i++)
				{
					if (i)
						_fstrcat (OutLoc," ");
					dpointtoa (_fstrchr (OutLoc,0),&pPoints[i]);
				}
				GSSiGlobUlFree (&hPoints);
				goto Rtnl;
			}

			if (!stricmp (Arg[2],"THIN"))
			{
				double		maxdist = atobasedist (Arg[3],&Err);

				*OutLoc = 0; 
				ThinPoly (&nPoints, pPoints, maxdist);
				for (i=0;i<nPoints;i++)
				{
					if (i)
						_fstrcat (OutLoc," ");
					dpointtoa (_fstrchr (OutLoc,0),&pPoints[i]);
				}
				GSSiGlobUlFree (&hPoints);
				goto Rtnl;
			}
			if (!stricmp (Arg[2],"OFFSET"))
			{
				double		Offset = atobasedist (Arg[3],&Err);
				int			nOffPoints;
				HPDPOINT	hOffPoints = GetOffsetPoly (pPoints,nPoints,&nOffPoints,Offset);

				*OutLoc = 0; 
				if (hOffPoints)
				{
					pPoint = (HPDPOINT)GlobalLock (hOffPoints);
					for (i=0;i<nOffPoints;i++)
					{
						if (i)
							_fstrcat (OutLoc," ");
						dpointtoa (_fstrchr (OutLoc,0),&pPoint[i]);
					}
					GSSiGlobUlFree (&hOffPoints);
				}
				GSSiGlobUlFree (&hPoints);
				goto Rtnl;
			}
			if (!stricmp(Arg[2], "DISPLAY"))
			{
				double		baseWidth = atobasedist(Arg[3], &Err);
				COLORREF	iColor = atoll(Arg[4]);
				SetCurView(SetVPFromName(Arg[5], &Err));
				int cap = atoi(Arg[6]);
				if (!baseWidth)
					baseWidth = 1;
				int screenWidth = IDNINT(baseWidth/CurView->BaseUnitsPerPixel);
				SaveDC(CurView->hDC);
				SetDisplayMode(CurView->hDC, GF_TEXTMODE);
				if (!CurView->hRgn)
				{
					CurView->hRgn = CreateVPRgn(FALSE, FALSE);
				}
				SelectVPClipRgn(CurView->hRgn);

				*OutLoc = 0;
				HPEN hPen = CreatePen(PS_SOLID, screenWidth, iColor);
				HPEN hOldPen = SelectObject(CurView->hDC, hPen);

				HANDLE hPoints2 = GSSiGlobAlloc(GAIDNO 1323, GMEM_MOVEABLE, (long)sizeof(POINT) * (long)nPoints+4);
				LPPOINT pPoint = (HPPOINT)GlobalLock(hPoints2);
				for (int i = 0; i < nPoints; i++)
				{
					pPoint[i] = BasePtToScreenPt(&pPoints[i]);
				}
				if (!wantGDIPlus)
					Polyline(CurView->hDC, pPoint, nPoints);
				else
				{
					AAPolyLineWithCap(CurView->hDC, pPoint, nPoints, iColor, screenWidth,cap);
				}

				GSSiGlobUlFree(&hPoints);
				GSSiGlobUlFree(&hPoints2);
				SelectObject(CurView->hDC, hOldPen);
				GSSiDeleteObject(&hPen);
				GSSiDeleteObject(&CurView->hRgn);
				RestoreDC(CurView->hDC, -1);
				goto Rtnl;
			}
			if (!stricmp (Arg[2],"BOUNDS"))
			{
				GetPolyBoundsD2 (pPoints,nPoints,&Bounds,1);
				boundstoa (OutLoc,&Bounds);
				GSSiGlobUlFree (&hPoints);
				goto Rtnl;
			}
			if (!stricmp (Arg[2],"LENGTH"))
			{
				RVal = GetPolyLengthD (pPoints,nPoints);
				ftoa (OutLoc,RVal);
				GSSiGlobUlFree (&hPoints);
				goto Rtnl;
			}
			if (!stricmp (Arg[2],"BP"))
			{
				dpointtoa (OutLoc,&pPoints[0]);
				GSSiGlobUlFree (&hPoints);
				goto Rtnl;
			}
			if (!stricmp (Arg[2],"EP"))
			{
				dpointtoa (OutLoc,&pPoints[nPoints-1]);
				GSSiGlobUlFree (&hPoints);
				goto Rtnl;
			}
			Dist = atof(Arg[3]);
			double OffsetDist = atof(Arg[4]);
			Point = PointAtDistOnPoly (pPoints,nPoints,Dist,&RVal,0);
			if (!stricmp (Arg[2],"AZ"))
			{
				ftoa (OutLoc,RVal);
				GSSiGlobUlFree (&hPoints);
				goto Rtnl;
			}
			if (OffsetDist != 0)
			{
				double az = LTWOPI(RVal - HALFPI);
				Point = dnewpt(Point, az, OffsetDist);
			}
			dpointtoa (OutLoc,&Point);
			GSSiGlobUlFree (&hPoints);
			goto Rtnl;
		}

		case 427: //$REAL()
		{
			nArgs = GetFunArgs (Args,Arg,1,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 0)
				goto RtnFalse;

			rtn = IsReal (Arg[1]);
		    goto Rtnrtn;
		}
		case 428: //$PING(n)
		{
			nArgs = GetFunArgs (Args,Arg,1,&hMem, pBrkPt, bpOffset, bpLen); 
			pVal = GlobalLock (hCmdMess);
			sprintf (pVal,">Pong:%s;\r\n",Arg[1]);
			GlobalUnlock (hCmdMess);
		    goto RtnTrue;
		}
		case 429:  //$YEAR(returns year from system time)
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			n = atoi (Arg[2]);
			systime = max (0,atol (Arg[1]));
			if (systime < 0)
				goto RtnFalse;
			tmtime = *localtime (&systime);
			year = tmtime.tm_year;
			if (n == 2)
			{
				if (tmtime.tm_year > 99)
					year = tmtime.tm_year - 100;
			}
			else
				year += 1900;
			itoa (year,OutLoc,10); 
			goto Rtnl;
			
		case 430: //$GRID(GETCELLID,GridID,point)		note:GridID and Zoom are the same
				  //$GRID(GETCELLPCT,GridID,point)
				  //$GRID(GETCELLBOUNDS,GridID,GridCellID)
				  //$GRID(GETZOOMFORSCALE,GridID,scale)
				  //$GRID(SPLIT,GridID,GMDFile,subfile,nfiles)
				  //$GRID(GETCELLCONTAININGBOUNDS,bounds,startzoom(opt -def maxzoom) returns ZOOM:CELL bounds may be point
				  //$GRID(VECTOR,OPEN,ZOOM,TILE,DIRECTORY)
				  //$GRID(VECTOR,CLOSE)
				  //$GRID(QUADKEY,level,x,y)
				  //$GRID(XY,level,basex,basey) returns gridxy from base x base y for specified zoom level
				  //$GRID(LL,level,gridx,gridy) returns lat/lon from gridxy at specified zoom level
		{
			long GridCelID;

			nArgs = GetFunArgs (Args,Arg,7,&hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 2)
				goto RtnFalse;
			if (!stricmp (Arg[1],"SPLIT"))
			{	
				if (!GetGridDef (Arg[2]))
					goto RtnFalse;
				rtn = SplitGridFile (Arg[3],Arg[4],atoi(Arg[5]));
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"XY"))
			{
				i = atoi (Arg[2]);
				if (i < 0 || i > 20)
					goto RtnFalse;
				Point.x = atof (Arg[3]);
				Point.y = atof (Arg[4]);
				ConvertCoord (&Point,1,2);
				LatLongToPixelXY(Point.y,Point.x, i, &Point16.x,&Point16.y);
				sprintf (OutLoc,"%i %i",Point16.x,Point16.y);
				goto Rtnl;
			}
			else if (!stricmp (Arg[1],"LL"))
			{
				i = atoi (Arg[2]);
				if (i < 0 || i > 21)
					goto RtnFalse;
				Point16.x = atoi (Arg[3]);
				Point16.y = atoi (Arg[4]);
				PixelXYToLatLong(Point16.x,Point16.y, i,&Point.y,&Point.x);
				sprintf (OutLoc,"%.14lg %.14lg",Point.x,Point.y);
				goto Rtnl;
			}
			else if (!stricmp (Arg[1],"QUADKEY"))
			{
				TileXYToQuadKey(atoi (Arg[3]),atoi (Arg[4]), atoi (Arg[2]),OutLoc,22);
				goto Rtnl;
			}
			else if (!stricmp (Arg[1],"VECTOR"))
			{	
				if (!stricmp (Arg[2],"OPEN"))
				{
					int zoom = atoi (Arg[3]);
					int irow = atoi (Arg[4]);
					int icol = atoi (Arg[5]);

					rtn = TileGraphicsOpen (Arg[6],zoom,irow,icol);
				}
				else if (!stricmp (Arg[2],"CLOSE"))
				{
					rtn = TileGraphicsClose (Arg[3],Arg[4]);
				}
				else if (!stricmp (Arg[2],"PATOPEN"))
					rtn = CreateContourPatternFile (FALSE);
				else if (!stricmp (Arg[2],"PATCLOSE"))
					rtn = CreateContourPatternFile (TRUE);
				else if (!stricmp (Arg[2],"PATCREATE"))
					rtn = CreateCPPatternInclude ();
				else if (!stricmp (Arg[2],"DEPTHTRIANGLES"))
				{
					if (!stricmp (Arg[3],"INIT"))
						rtn = CreateLakeDepthTriangles (TRUE);
					else if (!stricmp (Arg[3],"CLOSE"))
						rtn = CreateLakeDepthTriangles (FALSE);
					else if (!stricmp (Arg[3],"CREATE"))
						rtn = CreateTileDepthPixels (Arg[4],atoi (Arg[5]),atoi (Arg[6]),atoi (Arg[7]));
				}
				goto Rtnrtn;
			}
			else if (!stricmp (Arg[1],"GETCELLID"))
			{	
				Point = atopt (Arg[3],&Err);
				if (!GetGridDef (Arg[2]) || Err)
					goto RtnFalse;
				Point16 = DPointToPoint (Point);
				GridCelID = GridCellID (&Point16);
				ltoa (GridCelID,OutLoc,10);
			}
			else if (!stricmp (Arg[1],"GETGOOGLECELLID"))
			{	
				double pixelX, pixelY;
				int tileX, tileY;
				int	iLevel = atoi (Arg[2])-1;
				char	quadKey[22];
				char	gridID[4];

				Point = atopt (Arg[3],&Err);
				itoa ((iLevel+1)*10,gridID,10);
				if (!GetGridDef (gridID) || Err)
					goto RtnFalse;

				LatLongToPixelXYd(Point.y,Point.x,iLevel,&pixelX,&pixelY);
				//pixelY = nGridRow * 256 - pixelY - 1;
				PixelXYToTileXYd(pixelX,pixelY,&tileX,&tileY);
				GridCelID = tileY * nGridCol + tileX;
				//TileXYToQuadKey(tileX,tileY, iLevel,OutLoc,22);
				ltoa (GridCelID,OutLoc,10);
			}
			else if (!stricmp (Arg[1],"GETCELLPCT"))
			{	
				Point = atopt (Arg[3],&Err);
				if (!GetGridDef (Arg[2]) || Err)
					goto RtnFalse;
				Point = GridCellPCT (&Point);
				dpointtoa (OutLoc,&Point);
			}
			else if (!stricmp (Arg[1],"GETGOOGLECELLPCT"))
			{	
				int	iLevel = atoi (Arg[2])-1;
				char	gridID[4];
				DPOINT pctPoint, pixelPoint;

				itoa ((iLevel+1)*10,gridID,10);

				Point = atopt (Arg[3],&Err);
				if (!GetGridDef (gridID) || Err)
					goto RtnFalse;
				LatLongToPixelXYd(Point.y,Point.x,iLevel,&pixelPoint.x,&pixelPoint.y);
				pctPoint.x = fmod (pixelPoint.x,256.0)/256.0;
				pctPoint.y = fmod (pixelPoint.y,256.0)/256.0;
				dpointtoa (OutLoc,&pctPoint);
			}
			else if (!stricmp (Arg[1],"GETCELLBOUNDS"))
			{	
				if (!GetGridDef (Arg[2]))
					goto RtnFalse;
				if (!GetGridBounds (atoi(Arg[3]),&GridBounds))
					goto RtnFalse;
				lboundstoa (OutLoc,&GridBounds);
			}
			else if (!stricmp (Arg[1],"GETZOOMFORSCALE"))
			{	
				LPSTR	str = Arg[7];
				int		zoom;

				if (!GetGlobalCVal ("[%GRIDDEF]",str,0))
					return FALSE;
				if (!stricmp (str,"GoogleMaps"))
				{
					RVal = atof (Arg[2]);
					zoom = GetGoogleZoomForSCale (RVal);
					itoa (zoom,OutLoc,10);
					goto Rtnl;
				}
			}
			else if (!stricmp (Arg[1],"GETCELLCONTAININGBOUNDS"))
			{
				LPSTR	str = Arg[7];
				int		zoom, tilex, tiley;
				double	scale;

				if (!GetGlobalCVal ("[%GRIDDEF]",str,0))
					return FALSE;
				if (!stricmp (str,"GoogleMaps"))
				{
					Bounds = atobounds (Arg[2],&Err);
					if (Err)
						goto RtnFalse;
					GetGoogleZoomAndTileFromBounds (&Bounds,atoi(Arg[3]),&zoom,&tilex,&tiley,&scale,0);
					sprintf (OutLoc,"%i:%i-%i",zoom,tilex,tiley);
					goto Rtnl;
				}
			}
			else if (!stricmp (Arg[1],"DISPLAY"))
			{	
				DPOINT	Points[4];

				if (!GetGridDef (Arg[2]))
					goto RtnFalse;
				if (!GetGridBounds (atoi(Arg[3]),&GridBounds))
					goto RtnFalse;
				Bounds = lboundstobounds (&GridBounds);
				BoundsToPoints (&Bounds,Points,0);
				idesc = GetSymbolNum (Arg[4]);  
				if (!idesc)
					goto RtnFalse;
				SaveDC (CurView->hDC);
				SetDisplayMode (CurView->hDC, GF_TEXTMODE);
				GSSiDeleteObject(&CurView->hRgn);
				CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	  			SelectClipRgn (CurView->hDC,CurView->hRgn);
	  			GSSiDeleteObject(&CurView->hRgn);
				if (!stricmp (Arg[5],"LINE"))
				{
					HPEN	hPen = CreatePen (PS_SOLID,atoi (Arg[6]),atoi(Arg[7]));

					hOldPen = SelectObject (CurView->hDC,hPen);
					GWPolylineD (CurView->hDC,Points,4,idesc);
					SelectObject (CurView->hDC,hOldPen);
					GSSiDeleteObject (&hPen);
				}
				else if (!stricmp (Arg[5],"AREA"))
					GWPolygonD (CurView->hDC,Points,4,1,0,idesc,FALSE,TRUE,0);
				if (atob (Arg[6]))
				{
					DPOINT midPoint = MinMaxMidPointD (&Bounds);

					DisplayAddress (CurView->hDC,&midPoint, 0,Arg[3],0,0,FALSE);
				}

				RestoreDC (CurView->hDC,-1);
				goto RtnTrue;
			}
		    goto Rtnl;
		}
		case 431: //$DUMP(FILES)
		{
			nArgs = GetFunArgs (Args,Arg,2,&hMem, pBrkPt, bpOffset, bpLen); 
			if (nArgs < 1)
				goto RtnFalse;
			if (!stricmp (Arg[1],"FILES"))
				DumpOpenFiles (Arg[2]);
		    goto RtnTrue;
		}
		case 432: //$RAND(INIT,seed,RANGEMIN,RANGEMAX)
				  //$RAND()
		{
				// Generate random numbers in the half-closed interval
				// [range_min, range_max). In other words,
				// range_min <= random number < range_max
				static int range_min=0, range_max=RAND_MAX;

				nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
				if (!nArgs)
				{
					int u = (double)rand() / (RAND_MAX + 1) * (range_max - range_min)
							+ range_min;
					itoa(u, OutLoc, 10);

				}
				else
				{
					int seed = atoi(Arg[2]);
					srand(seed);
					range_min = atoi(Arg[3]);
					range_max = atoi(Arg[4]);
					*OutLoc = 0;
				}
				goto Rtnl;
		}
			break;
		case 433: //$AREA(DUMP,FILE,OPT)
		{
#define COORDINATEMULTIPLIER	10000000
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			if (!nArgs)
				goto RtnFalse;
			{
				HANDLE hPoly = 0;
				HANDLE hPolyPartLen = 0;
				int nPnts;
				int nLoops = GetPolyPointsWithParts((LPPICKDATAHEADER)&PickList[0], &nPnts, &hPoly, &hPolyPartLen);
				if (nLoops)
				{
					HFILE fid = GSSiOpenFile(Arg[2], 0, OF_CREATE);
					if (fid != HFILE_ERROR)
					{
						LPMNMXCORD	pBounds = (LPMNMXCORD)GlobalLock(hPoly);
						HPDPOINT lpDpoint = (HPDPOINT)(pBounds + 1);
						BigWrite(fid, &nPnts, 4, -1);
						BigWrite(fid, &nLoops, 4, -1);
						if (nLoops > 1)
						{
							LPINT pPolyParts = GlobalLock(hPolyPartLen);
							pPolyParts++;//first element is npoly
							BigWrite(fid, pPolyParts, nLoops * 4, -1);
							GlobalUnlock(hPolyPartLen);
						}
						for (i = 0; i < nPnts; i++)
						{
							int ix, iy;
							DPOINT pt = lpDpoint[i];
							POINT ipt;
							ConvertCoord(&pt, 1, 2);
							ipt.x = IDNINT(pt.x * COORDINATEMULTIPLIER);
							ipt.y = IDNINT(pt.y * COORDINATEMULTIPLIER);
							BigWrite(fid, &ipt, sizeof(POINT), -1);
						}
						GSSiClose2 (&fid);
						GlobalUnlock(hPoly);
					}
					else
						nLoops = 0;
				}
				GSSiGlobFree(&hPoly);
				GSSiGlobFree(&hPolyPartLen);
				itoa(nLoops, OutLoc, 10);
				goto Rtnl;
			}
		}
			break;
		case 434: //$TRAN(DISPLAY,htran)
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (!stricmp(Arg[1], "DISPLAY"))
			{
				HANDLE hTran = (HANDLE)atoi(Arg[2]);
				rtn = DisplayTranTriangles(hTran, CurView);
			}
			goto Rtnrtn;
		}
		case 435://$WIFI(Name
				 //$WIFI(Address
		{
			nArgs = GetFunArgs(Args, Arg, 2, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (nArgs < 1)
				goto RtnFalse;
			if (!stricmp(Arg[1], "NAME"))
			{
				GetWifiName(OutLoc);
			}
			else if (!stricmp(Arg[1], "ADDRESS"))
			{
				if (!GetWifiAddress(OutLoc))
					GetMyIPNetAddress(OutLoc);
			}
			goto Rtnl;
		}
		case 436: //$FILE(val)
		{
			nArgs = GetFunArgs(Args, Arg, 8, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			FILEFunctions(nArgs, Arg, OutLoc);
			goto Rtnl;
		}
		
		case 437: //$GDAL(OPEN,file)
		{
#define CPL_RESTRICT
#include "cpl_vsi.h"
#include "gdal.h"
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			GDALDatasetH  hDataset;
			double        adfGeoTransform[6];
			GDALAllRegister();
			MNMXCORD bounds;
		   hDataset = GDALOpen(Arg[2], GA_ReadOnly);
		   if (hDataset != NULL)
		   {
			   if (!stricmp(Arg[1], "BOUNDS"))
			   {
				   int xSize = GDALGetRasterXSize(hDataset);
				   int ySize = GDALGetRasterYSize(hDataset);
				   int rasterCount = GDALGetRasterCount(hDataset);
				   if (GDALGetGeoTransform(hDataset, adfGeoTransform) == CE_None)
				   {
					   bounds.xmn = adfGeoTransform[0];
					   bounds.ymn = adfGeoTransform[3];
					   bounds.xmx = bounds.xmn + xSize - 1;
					   bounds.ymx = bounds.ymn + ySize - 1;
					   boundstoa(OutLoc, &bounds);
					   GDALClose(hDataset);
					   goto Rtnl;
				   }
			   }
			   else
			   {
				   char mess[1024];
				   GDALDriverH   hDriver;
				   hDriver = GDALGetDatasetDriver(hDataset);
				   sprintf(mess, "Driver: %s/%s\n",
					   GDALGetDriverShortName(hDriver),
					   GDALGetDriverLongName(hDriver));
				   sprintf(mess, "Size is %dx%dx%d\n",
					   GDALGetRasterXSize(hDataset),
					   GDALGetRasterYSize(hDataset),
					   GDALGetRasterCount(hDataset));
				   if (GDALGetProjectionRef(hDataset) != NULL)
					   sprintf(mess, "Projection is `%s'\n", GDALGetProjectionRef(hDataset));
				   if (GDALGetGeoTransform(hDataset, adfGeoTransform) == CE_None)
				   {
					   sprintf(mess, "Origin = (%.6f,%.6f)\n",
						   adfGeoTransform[0], adfGeoTransform[3]);
					   sprintf(mess, "Pixel Size = (%.6f,%.6f)\n",
						   adfGeoTransform[1], adfGeoTransform[5]);
				   }

				   GDALRasterBandH hBand;
				   int             nBlockXSize, nBlockYSize;
				   int             bGotMin, bGotMax;
				   double          adfMinMax[2];
				   hBand = GDALGetRasterBand(hDataset, 1);
				   GDALGetBlockSize(hBand, &nBlockXSize, &nBlockYSize);
				   sprintf(mess, "Block=%dx%d Type=%s, ColorInterp=%s\n",
					   nBlockXSize, nBlockYSize,
					   GDALGetDataTypeName(GDALGetRasterDataType(hBand)),
					   GDALGetColorInterpretationName(
					   GDALGetRasterColorInterpretation(hBand)));
				   adfMinMax[0] = GDALGetRasterMinimum(hBand, &bGotMin);
				   adfMinMax[1] = GDALGetRasterMaximum(hBand, &bGotMax);
				   if (!(bGotMin && bGotMax))
					   GDALComputeRasterMinMax(hBand, TRUE, adfMinMax);
				   sprintf(mess, "Min=%.3fd, Max=%.3f\n", adfMinMax[0], adfMinMax[1]);
				   if (GDALGetOverviewCount(hBand) > 0)
					   sprintf(mess, "Band has %d overviews.\n", GDALGetOverviewCount(hBand));
				   if (GDALGetRasterColorTable(hBand) != NULL)
					   sprintf(mess, "Band has a color table with %d entries.\n",
					   GDALGetColorEntryCount(
					   GDALGetRasterColorTable(hBand)));

				   float *pafScanline;
				   int   nXSize = GDALGetRasterBandXSize(hBand);
				   pafScanline = (float *)CPLMalloc(sizeof(float)*nXSize);
				   GDALRasterIO(hBand, GF_Read, 0, 0, nXSize, 1,
					   pafScanline, nXSize, 1, GDT_Float32,
					   0, 0);
				   VSIFree(pafScanline);
			   }
			   GDALClose(hDataset);
			   goto RtnTrue;
		   }
		   goto RtnFalse;
		}
		case 438: //$POST(Macro,hWnd(opt))
		{
			int ierr = 0;
			nArgs = GetFunArgs(Args, Arg, -2, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			strcpy(CommandMacro, Arg[1]);
			if (nArgs == 1)
				PostMessage(hWndMain, PROCESS_COMMAND_MACRO,0, 0L);
			else
			{
				SetLastError(0);
				ExpandText(Arg[2]);
				HWND hWnd = (HWND)atoll(Arg[2]);
				PostMessage(hWnd, WM_LBUTTONDOWN, 0, 0L);
				PostMessage(hWnd, WM_LBUTTONUP, 0, 0L);
			}
			ierr = GetLastError();
			ii = ierr;
			goto RtnTrue;
		}

		case 439: //$FGDB(DUMP,FGDBPath,OutFilePath,ListType) dumps table names,types and counts to outfile
			      //$FGDB(CONVERT,FGDBPath,OutFilePath,Version,OutTableName,KeyField,IncludedFields) converts to SQLITE based file
		{
			nArgs = GetFunArgs(Args, Arg, 8, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (nArgs < 1)
				goto RtnFalse;
			if (!stricmp(Arg[1], "DUMP"))
			{
				int n = DumpFGDBTables(Arg[2], Arg[3], atoi(Arg[4]));
				itoa(n, OutLoc, 10);
			}
			else if (!stricmp(Arg[1], "CONVERT"))
			{
				int n = ConvertFGDBTable(Arg[2], Arg[3], Arg[4], Arg[5], Arg[6], Arg[7],Arg[8]);
				itoa(n, OutLoc, 10);
			}
			goto Rtnl;
		}
		case 440: //$CHAR(COUNT,string,char)
		{
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs < 1)
				goto RtnFalse;
			if (!stricmp(Arg[1], "COUNT"))
			{
				int n = 0;
				LPSTR loc = Arg[2];
				loc = strchr(loc, *Arg[3]);
				while (loc)
				{
					n++;
					loc++;
					loc = strchr(loc, *Arg[3]);
				}
				itoa(n, OutLoc, 10);
				goto Rtnl;
			}
			goto RtnFalse;
		}

		case 441: //$JUST(LRorC,string,width)
		{
			nArgs = GetFunArgs(Args, Arg, 3, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;
			if (nArgs > 1)
			{
				if (CurReport && (CurReport->hWnd || CurReport->hDC) && CurReport->currentFont)
				{
					int width = atoi(Arg[3]);
					width *= DeviceToScreenFactor();
					HDC hDC = CurReport->hDC;
					BOOL doRelease = FALSE;
					if (!hDC)
					{
						hDC = GetDC(CurReport->hWnd);
						doRelease = TRUE;
					}
					HFONT oldFont = SelectObject(hDC, CurReport->currentFont);
					SIZE txSize, txSizeSpace;
					char	tenSpace[11] = "          ";
					int rtn = GetTextExtentPoint32(hDC, Arg[2], strlen(Arg[2]), &txSize);
					rtn = GetTextExtentPoint32(hDC, tenSpace, 10, &txSizeSpace);
					int pixelsPerSpace =  txSizeSpace.cx / 10;
					SelectObject(hDC, oldFont);
					if (doRelease)
						ReleaseDC(CurReport->hWnd, hDC);
					int numSpaceNeeded = ((width - txSize.cx) / pixelsPerSpace) / 2;
					for (int i = 0; i < numSpaceNeeded; i++)
					{
						strcat(OutLoc, " ");
					}
					strcat(OutLoc, Arg[2]);
				}
				else
					strcpy(OutLoc, Arg[2]);
			}
			goto Rtnl;
		}
		case 442: //$RECT(WIDTH,rect)
				  //$RECT(HEIGHT,rect)
				  //$RECT(UL,rect)
				  //$RECT(UR,rect)
				  //$RECT(LL,rect)
				  //$RECT(LR,rect)
				  //$RECT(SPLIT,rect,UL)
				  //$RECT(SPLIT,rect,LL)
				  //$RECT(SPLIT,rect,UR)
				  //$RECT(SPLIT,rect,LR)
				  //$RECT(ADJUST,rect,xinc,yinc)
				  //$RECT(DISPLAY,rect,color,text,vpname)
				  //$RECT(MIDPOINT,rect);
		{
			nArgs = GetFunArgs(Args, Arg, 5, &hMem, pBrkPt, bpOffset, bpLen);
			*OutLoc = 0;

			if (nArgs > 1)
			{
				int ival;
				BOOL err;
				RECT inRect = atorect(Arg[2], &err);
				RECT outRect = inRect;
				if (!err)
				{
					if (!stricmp(Arg[1], "MIDPOINT"))
					{
						POINT pt = RectMid(&inRect);
						pttoa(OutLoc, pt);
					}
					else if (!stricmp(Arg[1], "WIDTH"))
					{
						ival = RECTWIDTH(&inRect);
						itoa(ival, OutLoc, 10);
					}
					else if (!stricmp(Arg[1], "HEIGHT"))
					{
						ival = RECTHEIGHT(&inRect);
						itoa(ival, OutLoc, 10);
					}
					else if (!stricmp(Arg[1], "ADJUST"))
					{
						int xinc = atoi(Arg[3]);
						int yinc = atoi(Arg[4]);

						outRect = AdjustRect(&inRect, xinc, yinc);
						recttoa(OutLoc, outRect);
					}
					else if (!stricmp(Arg[1], "MOVE"))
					{
						int xinc = atoi(Arg[3]);
						int yinc = atoi(Arg[4]);

						outRect = MoveRect(&inRect, xinc, yinc);
						recttoa(OutLoc, outRect);
					}
					else if (!stricmp(Arg[1], "DISPLAY"))
					{
						COLORREF color = atoi(Arg[3]);
						LPSTR text = Arg[4];
						SetCurView(SetVPFromName(Arg[5], &Err));
						outRect = MoveRect(&inRect, CurView->Rect.left, CurView->Rect.top);
						FillRectColor(CurView->hDC, &outRect, color);
						strcpy(OutLoc, "1");
					}
					else if (!stricmp(Arg[1], "SPLIT"))
					{
						if (!stricmp(Arg[3], "UL"))
						{
							outRect.right = RECTWIDTH(&inRect) / 2;
							outRect.bottom = RECTHEIGHT(&inRect) / 2;
						}
						else if (!stricmp(Arg[3], "LL"))
						{
							outRect.right = RECTWIDTH(&inRect) / 2;
							outRect.top = RECTHEIGHT(&inRect) / 2;
						}
						if (!stricmp(Arg[3], "UR"))
						{
							outRect.left = RECTWIDTH(&inRect) / 2;
							outRect.bottom = RECTHEIGHT(&inRect) / 2;
						}
						if (!stricmp(Arg[3], "LR"))
						{
							outRect.left = RECTWIDTH(&inRect) / 2;
							outRect.top = RECTHEIGHT(&inRect) / 2;
						}
						recttoa(OutLoc, outRect);
					}
				}
				goto Rtnl;
			}
		}
		case 443: //$TEXT(DISPLAY,text,point,size,color,font,vp(opt))
		{
			nArgs = GetFunArgs(Args, Arg,7, &hMem, pBrkPt, bpOffset, bpLen);
			SetCurView(SetVPFromName(Arg[7], &Err));
			SaveDC(CurView->hDC);
			int fontSize = atoi(Arg[4]);
			//HFONT hFont = GetStockObject(DEVICE_DEFAULT_FONT);
			HFONT hFont = CreateFont(fontSize, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, OUT_TT_PRECIS, 0, PROOF_QUALITY, 0, "Courier New");

			COLORREF	iColor = atoll(Arg[5]);
			DPOINT pt = atopt(Arg[3], &Err);
			pt.x += CurView->Rect.left;
			pt.y += CurView->Rect.top;
			HFONT oldFont = SelectObject(CurView->hDC, hFont);
			SetDisplayMode(CurView->hDC, GF_TEXTMODE);
			if (!CurView->hRgn)
			{
				CurView->hRgn = CreateVPRgn(FALSE, FALSE);
			}
			SelectVPClipRgn(CurView->hRgn);
			SetTextColor(CurView->hDC, iColor);
			TextOut(CurView->hDC, IDNINT(pt.x),IDNINT(pt.y), Arg[2], strlen(Arg[2]));
			SelectObject(CurView->hDC, oldFont);
			DeleteObject(hFont);
			RestoreDC(CurView->hDC, -1);
			goto RtnTrue;
		}
		case 444: // $RGBI(r,g,b,i) returns color value
				  // $RGBI(n) returns R|G|B|I
		{
			nArgs = GetFunArgs(Args, Arg, 4, &hMem, pBrkPt, bpOffset, bpLen);
			if (nArgs == 1)
			{
				COLORREF cref = atol(Arg[1]);
				int r = GetRValue(cref), g = GetGValue(cref), b = GetBValue(cref), i = GetIValue(cref);
				sprintf(OutLoc, "%i-%i-%i-%i", r, g, b,i);
			}
			else
			{
				ltoa((long)RGBI(atoi(Arg[1]), atoi(Arg[2]), atoi(Arg[3]), atoi(Arg[4])), OutLoc, 10);
			}
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
	if (SaveCfg != CurrentConfig)
	{
		SetConfig(SaveCfg);
	}
	if (pNumViewports && *pNumViewports)
	{
			SetCurView ( SaveVP);
	}
	GSSiGlobUlFree (&hMem);
	if (TraceOn)
	{
		hMem=GSSiGlobAlloc(GAIDNO 914,GMEM_MOVEABLE,4096);
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

int SearchGMC (int i)
{
	char	SaveCFG[MAX_PATH];
	char	str[512];
	HFILE	Fid=GSSiOpenFile ("c:\\cfglist.txt",0,OF_READ);

	strcpy (SaveCFG,CfgName);
    UnallocateConfig ();
	while (fgetstring (CfgName,255,Fid))
	{
		if (OpenConfig(0,0))
		{
			for (i=0;i<*pNumViewports;i++) 
			{
				if (pViewports[i]->pTheme && pViewports[i]->pTheme->ID == GF_COMPARE_VIEWPORTS_THEME)
				{
					sprintf (str,"%s:%s",CfgName,pViewports[i]->Name);
					AppendFile ("c:\\cfgfound.txt",str);
				}
			}
		    UnallocateConfig ();
		}
	}
	GSSiClose2 (&Fid);
	strcpy (CfgName,SaveCFG);
	OpenConfig(0,0);
	return 1;
}
			
BOOL SortTextFile(LPSTR InFile, LPSTR OutFile, int maxLineLen,BOOL ascending)
{
	BOOL rtn = FALSE;
	sqlite3* db;
	char tempFile[MAX_PATH + 2];
	HFILE fidIn = GSSiOpenFile(InFile, 0, OF_READ);
	if (fidIn != HFILE_ERROR)
	{
		HFILE fidOut = GSSiOpenFile(OutFile, 0, OF_CREATE);
		if (fidOut != HFILE_ERROR)
		{
			LPSTR line = malloc(maxLineLen + 4);
			LPSTR outline = malloc(maxLineLen + 14);
			LPSTR cmd = malloc(maxLineLen + 64);
			GSSiGetTempFileName(0, "gm", 0, tempFile);

			sprintf(cmd, "CREATE TABLE TEXTTABLE (LINE CHAR(256), LINENO INT);");

			rtn = !sqlite3_open(tempFile, &db);
			if (rtn)
			{
				SLT_StartTrans(db);
				if (SLT_Execute(cmd, db))
				{
					int lineno = 0;
					while (fgetstring(line, maxLineLen, fidIn))
					{
						sprintf(cmd, "INSERT INTO TEXTTABLE VALUES ('%s',%i);", line,lineno++);
						SLT_Execute(cmd, db);
					}
					sprintf(cmd, "SELECT * FROM TEXTTABLE ORDER BY LINE ASC;");
					sqlite3_stmt* statement = NULL;
					if (sqlite3_prepare_v2(db, cmd, -1, &statement, 0) == SQLITE_OK)
					{
						while (sqlite3_step(statement) == SQLITE_ROW)
						{
							LPSTR line = (LPSTR)sqlite3_column_text(statement, 0);
							int lineno = sqlite3_column_int(statement, 1);
							if (strlen(line) > 0)
							{
								sprintf(outline, "%i|%s", lineno, line);
								fputstring(outline, fidOut);
							}
						}	
					}
					sqlite3_finalize(statement);
				}
				SLT_EndTrans(db);
				SLT_Close(db);
			}
			free(cmd);
			free(line);
			free(outline);
			GSSiClose(fidOut);
		}
		GSSiRemove(tempFile);
		GSSiClose(fidIn);
	}
	return rtn;
}
