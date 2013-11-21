#include "graphint.h"   

#include "gmextern.h"

static LPSTR	pBrowseFile;
static LPSTR	BrowseSearch;
static long	BrowseLoc;
static short	BrowseLines;
static short	BrowseBackLines;
static short	reptype=0;
static	char	PrintReportListFile[144]="";
static	long	NextReportNum;
static	HANDLE	hViewScroll=0;

BOOL DecodeReportFont (LPSTR pLine, short ifont, LPREPORT pReport);
BOOL FAR PASCAL SCROLLREPORTMsgProc2(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);
BOOL FAR PASCAL SCROLLREPORTMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam);

int	CurrentReportFontHeight (void)
{   
	DWORD	TextExt;
	char	str[]="AyqfXYZ|_";
	if (!CurReport)
		return 0;
	if (CurFont && CurFont <= CurReport->NumFonts)
			SelectObject (CurReport->hDC,CurReport->hFonts[CurFont-1]);
	TextExt = GetTextExtent (CurReport->hDC,str,_fstrlen(str));
    return (HIWORD(TextExt));
}

HANDLE LoadReport2 (HFILE Fid,HANDLE hBuf)
{
	LPREPORT	pReport;
	HANDLE	hReport;
	HPSTR pBuf=GlobalLock (hBuf);
	LPSTR	lpBeg, lpEnd, lpEnd2, pEnd, pRow;
	long	len; 
	HANDLE	hLine=GSSiGlobAlloc ( 728,GMEM_MOVEABLE,1024);
	LPSTR	pLine = GlobalLock (hLine);
	LPSTR	pNextLine, lpComma;
	short	LineNum=0;
	
	reptype=1;
	hReport = GlobalAlloc (GHND,sizeof(REPORT));
	pReport = (LPREPORT)GlobalLock (hReport);  
	pReport->Type = 2;   
	pReport->First = TRUE;
	pReport->Margin = 0.01;
	while (fgetstring (pLine,1020,Fid))
	{
		pLine = FirstNonBlank (pLine);
		_fstrcat (pBuf,pLine);
	}  
	GSSiGlobUlFree (&hLine);
	GlobalUnlock (hBuf);
	pBuf = GlobalLock (hBuf);  
	pEnd = _fstrchr (pBuf,0);                          
	lpBeg = _fstrchr (pBuf,'{');
	if (!lpBeg)
		goto ErrExit; 
	lpBeg++;
	if (!(lpEnd = MatchLev (lpBeg,'}'))) goto ErrExit;
	lpBeg = lpEnd + 1; 
	while (lpBeg < pEnd)
	{
		if (!_fstrnicmp (lpBeg,"FONTS{",6))
		{
			lpBeg += 6;
			if (!(lpEnd = MatchLev (lpBeg,'}'))) goto ErrExit;  
			while (lpBeg < lpEnd)
			{
				if (!(lpEnd2 = MatchLev (lpBeg,';'))) lpEnd2 = lpEnd;
				*lpEnd2 = 0;
				if (!DecodeReportFont(lpBeg,pReport->NumFonts++,pReport))
					goto ErrExit;
				lpBeg = lpEnd2 + 1; 
			}
		}
		else if (!_fstrnicmp (lpBeg,"TABS{",5))  
		{
			lpBeg += 5;
			if (!(lpEnd = MatchLev (lpBeg,'}'))) goto ErrExit; 
			if (!(lpEnd2 = MatchLev (lpBeg,';'))) lpEnd2 = lpEnd; 
			*lpEnd2 = 0;   
			lpComma = lpBeg;
			while (lpComma) 
			{
				if ((lpComma = _fstrchr (lpBeg,',')))
					*lpComma = 0;  
				if (!_fstricmp (lpBeg,"AUTO")) 
					pReport->TabLen[pReport->NumTabs++] = -99;
				else
					pReport->TabLen[pReport->NumTabs++]=atoi(lpBeg);
				lpBeg = lpComma + 1; 
			}
		}	
		else if (!_fstrnicmp (lpBeg,"TABLES{",7))  
		{
			lpBeg += 7;
			if (!(lpEnd = MatchLev (lpBeg,'}'))) goto ErrExit; 
			*lpEnd = 0;  
			pNextLine = lpBeg;
			if (!(pReport->NumFiles=LoadStrings(0,&pReport->hFiles,&pNextLine,&LineNum)))
				goto ErrExit;
		}	
		else if (!_fstrnicmp (lpBeg,"START{",6))  
		{   
			lpBeg += 6;
			if (!(lpEnd = MatchLev (lpBeg,'}'))) goto ErrExit; 
			len = lpEnd - lpBeg + 1; 
			pReport->hRows = GSSiGlobAlloc ( 729,GMEM_MOVEABLE,len);
			pRow = GlobalLock (pReport->hRows);  
			pReport->NumRows=1;
			*lpEnd = 0;
			_fstrcpy (pRow,lpBeg);
			GlobalUnlock (pReport->hRows); 
		} 
		else
			lpEnd++;
		lpBeg = lpEnd + 1;
	}
	GSSiGlobUlFree (&hBuf);	
	GlobalUnlock (hReport); 
	GSSiClose (Fid);
	return hReport;        
ErrExit:
	GSSiGlobUlFree (&hBuf);	  
	GSSiClose (Fid);
	return 0;
}
HANDLE LoadReport (LPSTR Name)
{   
	LPREPORT	pReport;
	HANDLE	hReport, hBuf, hTempLine;
	HFILE	Fid;                                
	OFSTRUCT	OFStruct;
	long	len; 
	LPSTR	pBuf, pNextLine, pLine, templine;
	int		LineNum;
	LPLONG	pTemp;  
	char	HeadLine[36];
	
	reptype = 0;
	Fid = GSSiOpenFile (Name,&OFStruct,OF_READ); 
	if (Fid == HFILE_ERROR) return NULL;
	SetCurVal (Name,IDS_FILERPT);
	len = GSSillseek (Fid,0,2); 
	GSSillseek (Fid,0,0);
	hBuf = GlobalAlloc (GHND,len+1);
	fgetstring (HeadLine,32,Fid);
	GSSillseek (Fid,0,0);
	if (!_fstrnicmp (HeadLine,"HEADER",6))
		return LoadReport2 (Fid,hBuf);
	pBuf = GlobalLock (hBuf);
	hTempLine = GlobalAlloc (GMEM_MOVEABLE,2096);
	templine = GlobalLock (hTempLine);
	BigRead (Fid,pBuf,len); 
	GSSiClose (Fid); 
	hReport = GlobalAlloc (GHND,sizeof(REPORT));
	pReport = (LPREPORT)GlobalLock (hReport);  
	pReport->Type = 1;   
	pReport->First = TRUE;
	pReport->Margin = 0.01;
	pNextLine = pBuf;
	LineNum = 0;
	while ((pLine = NextRepLine (&pNextLine,&LineNum)))
	{   
		if (!_fstrnicmp (pLine,"[%NUM_FONTS]=",13))
		{   
			short	ifont;
			
			pLine += 13;                    
			pReport->NumFonts = atoi (pLine);
			for (ifont = 0;ifont<pReport->NumFonts;ifont++)
			{   
				pLine = NextRepLine (&pNextLine,&LineNum);
				if (!pLine)
					goto ErrOut;   
				if (!DecodeReportFont(pLine,ifont,pReport))
					goto ErrOut;
			}
		}
		else if (!_fstrnicmp (pLine,"[%NUM_FILES]=",13))
		{   
			int		ifile;
			LPSTR	FirstFile;
			long	TotLen=0;
			
			pLine += 13;                    
			pReport->NumFiles = atoi (pLine);
			for (ifile=0;ifile<pReport->NumFiles;ifile++)
				pReport->FileTypes[ifile]=0;
			if (pReport->NumFiles)
			{
				if (!LoadStrings(pReport->NumFiles,&pReport->hFiles,&pNextLine,&LineNum))
					goto ErrOut; 
			}
		}
		else if (!_fstrnicmp (pLine,"[%NUM_ROWS]=",12))
		{   
			int		ifile;
			LPSTR	FirstFile;
			long	TotLen=0;
			
			pLine += 12;                    
			pReport->NumRows = atoi (pLine); 
			if (!LoadStrings(pReport->NumRows,&pReport->hRows,&pNextLine,&LineNum))
				goto ErrOut;
		}
		else if (!_fstrnicmp (pLine,"START",5))
		{   
			int		ifile;
			LPSTR	FirstFile, pRow, pEnd;
			long	TotLen;
			
			pReport->Type = 2;   
			pReport->hRows = GSSiGlobAlloc ( 730,GMEM_MOVEABLE,UINT_MAX);
			pRow = GlobalLock (pReport->hRows);  
			pReport->NumRows=1;
			*pRow = 0;
			while ((pLine = NextRepLine (&pNextLine,&LineNum)))
			{
				pLine = FirstNonBlank (pLine);
				_fstrcat (pRow,pLine); 
			}
			TotLen = _fstrlen (pRow)+1;
			GlobalUnlock (pReport->hRows); 
			pReport->hRows=GlobalReAlloc (pReport->hRows,TotLen,GMEM_MOVEABLE);
		}
		else if (!_fstrnicmp (pLine,"[%NUM_TABS]=",12))
		{   
			int		itab;
			
			pLine += 12;                    
			pReport->NumTabs = atoi (pLine); 
			for (itab = 0;itab<pReport->NumTabs;itab++)
			{   
				pLine = NextRepLine (&pNextLine,&LineNum); 
				if (!_fstricmp (pLine,"AUTO")) 
					pReport->TabLen[itab] = -99;
				else
					pReport->TabLen[itab]=atoi(pLine);
			}
		}
		else
		{    
			_fstrcpy (templine,pLine);
			ExpandText (templine);
		}
	}
	
	GSSiGlobUlFree (&hBuf);
	GSSiGlobUlFree (&hTempLine);
	GlobalUnlock (hReport); 
	return hReport;
	
ErrOut:
	GSSiGlobUlFree (&hBuf);
	GSSiGlobUlFree (&hTempLine);
	GSSiGlobUlFree (&hReport);
	return (NULL);
}  

BOOL DecodeReportFont (LPSTR pLine, short ifont, LPREPORT pReport)
{
	short	FontWeight, FontSize, Italic, Underline;
	LPSTR	FontName;  
	
	FontName = pLine;
	if (!(pLine = _fstrchr (pLine,','))) goto ErrOut; 
	*pLine = '\0';
	pLine++;
	switch (*pLine)
	{
		case 'L':
			FontWeight = FW_LIGHT;
			break;
		case 'T':
			FontWeight = FW_THIN;
			break;
		case 'N':
			FontWeight = FW_NORMAL;
			break;
		case 'B':
			FontWeight = FW_BOLD;
			break;
		case 'H':
			FontWeight = FW_HEAVY;
			break;   
		default:
			goto ErrOut;
	}
	pLine++;
	Italic = FALSE;
	if (*pLine=='I')
	{
		Italic = TRUE;
		pLine++;
	}    
	Underline = FALSE;
	if (*pLine=='U')
	{
		Underline = TRUE;
		pLine++;
	} 
	if (*pLine != ',') goto ErrOut;
	pLine++;
	FontSize = atoi (pLine);
	if (!(pLine = _fstrchr (pLine,','))) goto ErrOut;
	pLine++;
	pReport->FontColor[ifont] = SetColor (pLine);
	pReport->FontSize[ifont] = FontSize;		
	pReport->FontWeight[ifont]=FontWeight;
	pReport->FontItalic[ifont]=Italic;
	pReport->FontUnderline[ifont]=Underline;
	_fstrcpy(pReport->FontName[ifont],FontName);
	return TRUE;
ErrOut:
	return FALSE;
}

void CreateReportFonts(LPREPORT	pReport,double Factor)
{
	int	ifont;
	
	for (ifont=0;ifont<pReport->NumFonts;ifont++)
		pReport->hFonts[ifont]=CreateFont ((int)-(int)(((double)pReport->FontSize[ifont])*Factor),
											0,0,0,
											pReport->FontWeight[ifont],
											(BYTE)pReport->FontItalic[ifont],
									  		(BYTE)pReport->FontUnderline[ifont],
									  		0,0,0,0,0,0,
									  		pReport->FontName[ifont]);
	return;
} 

void DestroyReportFonts (LPREPORT pReport)
{
	int	ifont;
	
	for (ifont=0;ifont<pReport->NumFonts;ifont++)
		DeleteObject(pReport->hFonts[ifont]);
	return;
}

short LoadStrings (short nstrings ,HANDLE *hstrings,LPSTR *pNextLine,LPSHORT LineNum)
{   int	i, MaxStrings=16000;
	HANDLE	hTemp;
	LPLONG	pTemp;
	LPSTR	pLine, FirstString, pStrings, StartStrings;
	long	TotLen=0;
    
    if (!nstrings)
    	nstrings = MaxStrings;
	hTemp = GlobalAlloc (GMEM_MOVEABLE,(long)nstrings*4);
	pTemp = (LPLONG)GlobalLock (hTemp);
	for (i = 0;i<nstrings;i++,pTemp++)
	{   
		pLine = NextRepLine (pNextLine,LineNum); 
		if (!pLine)
		{   
			if (nstrings < MaxStrings)
			{
				GlobalUnlock (hTemp);
				GlobalFree (hTemp);
				return FALSE; 
			}
			nstrings = i;
			goto EndStrings;
		}
		if (!i) FirstString = pLine;
		*pTemp = pLine - FirstString;
		TotLen += _fstrlen (pLine)+2;
	}                       
EndStrings:
	*hstrings = GlobalAlloc (GMEM_MOVEABLE, nstrings*4+TotLen); 
	pStrings = GlobalLock (*hstrings);
	GlobalUnlock (hTemp);
	pTemp = (LPLONG)GlobalLock (hTemp);
	_fmemmove (pStrings,pTemp,nstrings*4); 
	pStrings += nstrings*4; 
	StartStrings = pStrings;
	for (i = 0;i<nstrings;i++,pTemp++)
	{
		pLine=FirstString+*pTemp;
		pStrings = StartStrings+*pTemp;
		_fstrcpy (pStrings,pLine);
	}
	GlobalUnlock (hTemp);
	GlobalFree (hTemp);
	GlobalUnlock (*hstrings); 
	return nstrings;
}

LPSTR NextRepLine (LPSTR *NextLine,LPSHORT LineNum)
{
	LPSTR rtn;
	
	rtn = *NextLine; 
	if (rtn)
	{    
		if (*rtn)
		{
			(*LineNum)++;
			if (reptype)
			{
				*NextLine = MatchLev (*NextLine,';');
				if (*NextLine)
				{
					**NextLine = 0;
					(*NextLine)++; 
				} 
			}
			else
			{
				*NextLine = _fstrchr (*NextLine,'\r');
				if (*NextLine)
				{
					**NextLine = 0;
					(*NextLine)+=2; 
				}
			}
		}
		else
			rtn=0;     
	}
	
	return (rtn);
}

BOOL OpenReportFiles (HANDLE hReport)
{
	LPREPORT	pReport;
	LPLONG		pFiles; 
	LPSTR		pFile, pFileBeg, pSQL, lpEq; 
	LPHANDLE	FileHandle, SaveHandle;
	LPSHORT		FileType; 
	short		ifile; 
	BOOL		rtn=FALSE; 
	char		FileAndSQL[256]; 
	char		NullSQL[2]="";
	
	if (!hReport)
		return FALSE;
	pReport = (LPREPORT)GlobalLock (hReport);  
	if (pReport->FilesAreOpen || !pReport->NumFiles)
	{
		GlobalUnlock (hReport);
		return TRUE;
	}
	pFiles = (LPLONG)GlobalLock (pReport->hFiles); 
	pFileBeg = (LPSTR) (pFiles + pReport->NumFiles);
	FileType = (LPSHORT)&pReport->FileTypes;
	FileHandle = (LPHANDLE)&pReport->FileHandles; 
	for (ifile = 0;ifile<pReport->NumFiles;ifile++,FileType++,FileHandle++)
	{
		pFile = pFileBeg;
		pFile += *pFiles++;  
		if (pReport->NumFiles<0)
			pSQL = pFile + *pFiles++;
		else
		{   
			_fstrcpy (FileAndSQL,pFile); 
			if (*LastChr (FileAndSQL) == ';')
				*LastChr (FileAndSQL) = 0;
			pFile = FileAndSQL;
			pSQL = ldelim (pFile,','); 
			if (!pSQL)
				pSQL = NullSQL; 
			else
				*pSQL++=0;
		}
	    if (!OpenDataFile (pFile,pSQL,BT_READ,FileHandle))
	    	goto Exit;
	} 
	rtn = TRUE; 
	pReport->FilesAreOpen = TRUE;
Exit:
	GlobalUnlock (pReport->hFiles);
	GlobalUnlock (hReport);
	return (rtn);
} 

void CloseReportFiles (HANDLE hReport) 
{
	LPREPORT	pReport;
	LPHANDLE	FileHandle, lpFilePathHandle;
	HANDLE		LastHandle;
	short		ifile, i;
	
	pReport = (LPREPORT)GlobalLock (hReport);
	FileHandle = (LPHANDLE)&pReport->FileHandles; 
	for (ifile = 0;ifile<pReport->NumFiles;ifile++,FileHandle++)
	{
		CloseDataFile (FALSE, FileHandle);
	}
	pReport->FilesAreOpen = FALSE;
	GlobalUnlock (hReport); 
	
	return;
} 

void ReportTextOut (LPREPORT CurReport,LPSTR txt)
{   
	int	l=_fstrlen (txt);                 
	DWORD TextExt = GetTextExtent (CurReport->hDC,txt,l);
	
	if (CurReport->WantSize)
	{   
		POINT	p;
		    			
		p.x = CurReport->x;
		p.y = CurReport->y;
		AddPointToRect (p,&CurReport->SizeRect);
		p.x = CurReport->x+LOWORD(TextExt);
		p.y = CurReport->y+HIWORD(TextExt);
		AddPointToRect (p,&CurReport->SizeRect);
	}
	else
//		ExtTextOut (CurReport->hDC,CurReport->x,CurReport->y,0,NULL,txt,l,NULL);
		TextOut (CurReport->hDC,CurReport->x,CurReport->y,txt,l);
	CurReport->x += LOWORD(TextExt); 
    return;
}

BOOL DisplayReport (HDC hDC, HANDLE hReport, RECT Rect, double Factor, long Refno,LPRECT pSizeRect)
{
	LPREPORT	pReport;
	int			irow, itab, MaxRowLen=0, ReportHeight=0, RowHeight, ReportWidth, x, y,xj,yj=0,w,lt, Margin,ifont;  
	LPSTR		pRow, Tabloc, StartTab, EndTab, pFirstRow, Tabstr;
	LPLONG		startrow, pRows;
	char		tabstr[16], FontStr[32];  
	DWORD		TextExt; 
	HFONT		OldFont;   
	BOOL		NullLine;
	int			hinc, winc; 
	HRGN		hRgn;
	HANDLE		hStr;
	LPSTR		str; 
	short		i;
	
	HaltReport = FALSE;                   
	pReport = (LPREPORT)GlobalLock (hReport);  
	pReport->hDC = hDC;
	pReport->Rect = Rect;
	if (pReport->Type == 2)
	{
		GlobalUnlock (hReport);
		return (DisplayReport2 (hDC,hReport,Rect,Factor,TRUE,pSizeRect));
	}
	GlobalUnlock (hReport);
	if (!OpenReportFiles (hReport))
		return FALSE;    
	if (pSizeRect)
	{
		pReport->WantSize = TRUE;
		RectInit (&pReport->SizeRect); 
	}
	else
		pReport->WantSize = FALSE;
 
    SaveDC (hDC);
	hStr = GSSiGlobAlloc ( 731,GMEM_MOVEABLE,4096);
	str = GlobalLock (hStr);
	SetDisplayMode (hDC, GF_TEXTMODE);    
	SelectClipRgn (hDC,NULL);
	pReport = (LPREPORT)GlobalLock (hReport);
	hinc = ((Rect.bottom-Rect.top)*pReport->Height*Factor)/2;
	winc = ((Rect.right-Rect.left)*pReport->Width*Factor)/2;
	Rect.top += hinc;
	Rect.left += winc;
	Rect.bottom -= hinc;
	Rect.right -= winc;  
	pReport->Rect = Rect; 
	for (i=0;i< pReport->NumTabs;i++)
		if (pReport->TabLen[i] < 0) 
			pReport->TabLen[i] = -((long)Rect.right - (long)Rect.left) / 2;
	CreateReportFonts(pReport,Factor);
	_fstrcpy (FontStr,"[%FONT]=0;");
	ExpandText (FontStr); 
    OldFont = SelectObject(hDC, GetStockObject(SYSTEM_FONT));
    SelectObject (hDC,OldFont); 
	SetTextColor (CurView->hDC,0);
    if (pReport->hRows)
    {
		pRows = (LPLONG)GlobalLock (pReport->hRows); 
		pFirstRow = (LPSTR) (pRows + pReport->NumRows); 
		Margin = min (IDNINT(pReport->Margin * (Rect.bottom-Rect.top)), 
					  IDNINT(pReport->Margin * (Rect.right-Rect.left))); 
		if (Printing)
			hRgn = 0;
		else
			hRgn = CreateRectRgn (Rect.left + Margin,Rect.top + Margin,Rect.right - Margin,Rect.bottom-Margin);
		SelectClipRgn (hDC,hRgn);
		GSSiDeleteObject(&hRgn);
//SelectClipRgn (hDC,NULL);//tempdebug
		y = Rect.top + Margin;	
		for (irow = 0;irow<pReport->NumRows;irow++)
		{   
			x = Rect.left + Margin;
			startrow = pRows;
			startrow += irow;  
			RowHeight = 0;
			pRow = pFirstRow + *startrow; 
			StartTab = pRow; 
			Tabloc = _fstrstr (StartTab,"$TAB(");
			if (Tabloc)
				*Tabloc = '\0';
			itab = 0;
			tabstr[0]='\0';
			_fstrcpy (str,StartTab);
			while (Tabloc)
			{   
				ExpandText (str);
				Truncate (str); 
				if (!str[0]) 
				{
					NullLine = TRUE;
					_fstrcpy (str," "); 
				}
				else
					NullLine = FALSE;
				_fstrcpy (FontStr,"[%FONT]");
				ExpandText (FontStr);
				ifont = atoi (FontStr); 
				if (ifont)
					SelectObject (hDC,pReport->hFonts[ifont-1]);
				else
				    SelectObject (hDC,OldFont);
				SetTextColor (CurView->hDC,0);
				Tabstr = (LPSTR)(Tabloc+5);
	            EndTab = _fstrchr (Tabstr,')');
	            if (EndTab)
	            {
	            	*EndTab = '\0';
	            	_fstrcpy (tabstr,Tabstr);
	            	*EndTab = ')';
					TextExt = GetTextExtent (hDC,tabstr,_fstrlen(tabstr));
					lt = LOWORD(TextExt);
					xj = abs(pReport->TabLen[itab]) - lt;   
					pReport->x = x+xj;
					pReport->y = y+yj;
					ReportTextOut (pReport,tabstr);
				//	TextOut (hDC,x+xj,y+yj,tabstr,_fstrlen(tabstr));
				}
				else
					lt=0;
				TextExt = GetTextExtent (hDC,str,_fstrlen(str));
	            RowHeight = max (RowHeight,HIWORD(TextExt)); 
				_fstrcpy (FontStr,"[%JUST]");
				ExpandText (FontStr);
//				w = Factor*pReport->TabLen[itab]*DeviceToScreenFactor-lt;
				w = abs(pReport->TabLen[itab])-lt;
				if (FontStr[0]=='C') 
					xj = (w - LOWORD(TextExt)) / 2;
				else if (FontStr[0]=='R') 
					xj = w - LOWORD(TextExt);
				else 
					xj = 0;
				if (!NullLine)
				{
					pReport->x = x+xj;
					pReport->y = y+yj;
					ReportTextOut (pReport,str);
					// TextOut (hDC,x+xj,y+yj,str,_fstrlen(str)); 
				}
	            *Tabloc = '$';
	            if (EndTab)
	            {
	            	StartTab = EndTab+1;            
					Tabloc = _fstrstr (StartTab,"$TAB(");
					if (Tabloc)
						*Tabloc = '\0';
					_fstrcpy (str,StartTab);
				}
	            else 
	            {
	            	str[0]='\0';
	            	Tabloc = 0; 
	            } 
//	            x += Factor*pReport->TabLen[itab]*DeviceToScreenFactor;
	            x += abs(pReport->TabLen[itab]);
	            itab++;
			}
			ExpandText (str); 
			Truncate (str);
			if (!str[0]) 
			{
				NullLine = TRUE;
				_fstrcpy (str," "); 
			}
			else
				NullLine = FALSE;
			_fstrcpy (FontStr,"[%FONT]");
			ExpandText (FontStr);
			ifont = atoi (FontStr); 
			if (ifont)
				SelectObject (hDC,pReport->hFonts[ifont-1]);
			else
			    SelectObject (hDC,OldFont);
			SetTextColor (CurView->hDC,0);
			TextExt = GetTextExtent (hDC,str,_fstrlen(str));
	        RowHeight = max (RowHeight,HIWORD(TextExt)); 
	        lt = LOWORD(TextExt);
			_fstrcpy (FontStr,"[%JUST]");
			ExpandText (FontStr);
			if (itab)
				//w = Factor*pReport->TabLen[itab]*DeviceToScreenFactor;
				w = abs(pReport->TabLen[itab]);
			else
				w = (Rect.right - Rect.left) - Margin * 2;
			if (FontStr[0]=='C') 
				xj = (w - lt) / 2;
			else if (FontStr[0]=='R') 
				xj = w - lt;
			else
				xj = 0;
			if (!NullLine)
			{
				pReport->x = x+xj;
				pReport->y = y+yj;
				ReportTextOut (pReport,str);
				// TextOut (hDC,x+xj,y+yj,str,_fstrlen(str)); 
			}
			y += RowHeight;
		}
		GlobalUnlock (pReport->hRows); 
	}  
    SelectObject (hDC,OldFont);
    DestroyReportFonts(pReport);
	CloseReportFiles (hReport); 
	if (pSizeRect)
		*pSizeRect = pReport->SizeRect; 
	
	GlobalUnlock (hReport);  
	GSSiGlobUlFree (&hStr); 
	RestoreDC (hDC,-1);
	return TRUE;
}   

BOOL BrowseTextFile (LPSTR File, long Loc, short backlines, short forwardlines,LPSTR SearchString)
{ 
	FARPROC lpfnBROWSETEXTMsgProc;
	int	Rtn; 
	
	pBrowseFile = File;
	BrowseLoc = Loc;
	BrowseLines = forwardlines; 
	BrowseBackLines = backlines;
	BrowseSearch = SearchString;
	lpfnBROWSETEXTMsgProc = MakeProcInstance((FARPROC)BROWSETEXTMsgProc, hInst);
	Rtn = DialogBox(hInst, (LPSTR)"BROWSE", hWndMain, lpfnBROWSETEXTMsgProc);
	FreeProcInstance(lpfnBROWSETEXTMsgProc); 
	return TRUE;
} 

BOOL FAR PASCAL BROWSETEXTMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
    RECT	rect,sbrect; 
    POINT	pt;  
	static	HANDLE	hSaveBM=0; 
	static	long	EndLoc;
	static	long	AveLineLength=0, BrowseLinesIn;  
	static	TEXTMETRIC tm; 
	long	nLines=0, CurLoc;
    int		height,width,x,y;  
    HFILE	Fid;  
    OFSTRUCT	OFStruct;
	HANDLE	hSTR;
	LPSTR	str;    
    
 switch(Message)
   {
    case WM_INITDIALOG:  
	{   
		HFONT	hFontOld,hFontNew;
		HDC hdcLB=GetDC (GetDlgItem(hWndDlg,IDC_SCROLLBOX));   
		
					
		BrowseLinesIn = BrowseLines;
    	hSaveBM = EnterBlockingWindow (hWndDlg);
		hFontNew = SendDlgItemMessage(hWndDlg, IDC_SCROLLBOX, WM_GETFONT, NULL, NULL); 
		hFontOld = SelectObject (hdcLB,hFontNew);
		GetTextMetrics (hdcLB,&tm);
		SelectObject (hdcLB,hFontOld);
		ReleaseDC (GetDlgItem(hWndDlg,IDC_SCROLLBOX),hdcLB);

         CurView->hWnd = hWndDlg;
		 GetWindowRect(hWndMain, &rect);    
		 rect.left = max(0,rect.left);
		 rect.top = max(0,rect.top); 
		 x = rect.left;
		 y = rect.top;
		 height = rect.bottom-rect.top-6;    
		 width = rect.right - rect.left-6;
	 	 SetWindowPos(hWndDlg, (HWND) NULL, x+1, y+1,width, height,0); 
	 	 GetClientRect(hWndDlg,&rect);  
	 	 GetWindowRect (GetDlgItem(hWndDlg,IDC_SCROLLBOX),&sbrect);
	 	 SetWindowPos(GetDlgItem(hWndDlg,IDC_SCROLLBOX),(HWND)NULL, sbrect.left, sbrect.top,rect.right, rect.bottom,0);
		 PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
		}
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDM_EXIT, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
         {  
            case IDC_CANCEL: 
		    	ContinueProcessing = FALSE;
		    	break;	 
		    case IDM_EXIT:
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);
            break;

		    case IDM_PRINT:
		 		 hSTR = GSSiGlobAlloc ( 732,GMEM_MOVEABLE,4096);
		 		 str = GlobalLock (hSTR);
		 		 SendDlgItemMessage (hWndDlg,IDC_SCROLLBOX,LB_RESETCONTENT,0,0);
		
			 	 Fid = GSSiOpenFile (pBrowseFile,&OFStruct,OF_READ);
		    	 EndLoc = GSSillseek (Fid,0,2);
		    	 GSSillseek (Fid,0,0);
		    	 while (ContinueProcessing && fgetstring (str,4090,Fid))
		    	 {  
		    	 	LPSTR EndChar = LastChr (str);
		    	 	
		    	 	if (*EndChar != '/' && *EndChar != '|')
			 			SendDlgItemMessage (hWndDlg,IDC_SCROLLBOX,LB_ADDSTRING,(WPARAM)NULL,(LPARAM)str);  
			 		EndChar = str;
			 		while (*EndChar)
			 		{
			 			if (iscntrl (*EndChar))
			 			{
				 			SendDlgItemMessage (hWndDlg,IDC_SCROLLBOX,LB_ADDSTRING,(WPARAM)NULL,(LPARAM)str);  
			 				break;
			 			}
			 			EndChar++;
			 		}
	                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), EndLoc, GSSillseek (Fid,0,1),0);
		    	 }
		    	 GSSiGlobUlFree (&hSTR);     
		    	 GSSiClose (Fid);
		    	break;   
		    case IDM_BROWSEEND: 
		    	BrowseLines = BrowseLinesIn;   
		    	BrowseLoc = max (0,EndLoc - AveLineLength * BrowseLines); 
		    	nLines = AveLineLength = 0;
		    	PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
		    	break; 
		    case IDOK: 
		    {
				HANDLE	hBackLines;
				LPLONG	pBackLines;
				DWORD dwStringExt, CurExtent=SendDlgItemMessage(hWndDlg, IDC_SCROLLBOX, LB_GETHORIZONTALEXTENT,0, 0L);  
				
		 		 hSTR = GSSiGlobAlloc ( 733,GMEM_MOVEABLE,4096);
		 		 str = GlobalLock (hSTR);
		 		 SendDlgItemMessage (hWndDlg,IDC_SCROLLBOX,LB_RESETCONTENT,0,0);
		
			 	 Fid = GSSiOpenFile (pBrowseFile,&OFStruct,OF_READ);
			 	 if (Fid == HFILE_ERROR)
			 	 {
		    	 	PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);  
		    	 	break;
		    	 } 
		    	 EndLoc = GSSillseek (Fid,0,2);
		    	 GSSillseek (Fid,0,0);
		    	 if (BrowseLoc < 0 && ContinueProcessing)
		    	 {  
		    	 	long	BeginLoc = BrowseLoc;
		    	 	
		    	 	while (BrowseLoc++ < 0)  
		    	 	{
		    	 		if (!fgetstring (str,4090,Fid))
		    	 			break; 
		                PctBox (GetDlgItem(hWndDlg,IDC_STATUS), BeginLoc, BrowseLoc - BeginLoc,-1);
		    	 	}
		    	 }
		    	 else
				 	GSSillseek (Fid,BrowseLoc,0);
				 while (BrowseLines && ContinueProcessing)
				 {
				 	if (!fgetstring (str,4090,Fid))	
				 		break;
				 	CurLoc = GSSillseek (Fid,0,1); 
		            PctBox (GetDlgItem(hWndDlg,IDC_STATUS), EndLoc, CurLoc,0);
				 	AveLineLength += _fstrlen (str);
				 	nLines++; 
				 	if (BrowseBackLines)
				 	{
				 	}
				 	if (BrowseSearch)
				 	{
				 		if (!_fstrstr (str,BrowseSearch))
				 			goto NextLine; 
				 		else
				 			BrowseSearch = NULL;
				 	}
				 	if (BrowseBackLines)
				 	{
				 	}
			 		BrowseLines--;
					dwStringExt = (tm.tmAveCharWidth+1)*_fstrlen (str);
					if (dwStringExt > CurExtent)
					{
						CurExtent = dwStringExt;	
						SendDlgItemMessage(hWndDlg, IDC_SCROLLBOX, LB_SETHORIZONTALEXTENT,
						    			   LOWORD(dwStringExt), 0L); 
					}
			 		SendDlgItemMessage (hWndDlg,IDC_SCROLLBOX,LB_ADDSTRING,(WPARAM)NULL,(LPARAM)str); 
			NextLine:;
			
				 }
				 ContinueProcessing = TRUE;
				 GSSiClose (Fid); 
				 GSSiGlobUlFree (&hSTR);
				 if (nLines) 
				 	AveLineLength /= nLines;
		 	}
		    	break;
    
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
}  

BOOL ProcessMacroReport (LPSTR Name, LPSTR Prefix, LPSTR UDI, long ref)
{
	int	Rtn;
	LPVIEWPORT	SaveView, SaveView2;        
	HANDLE	hView;
	
	SaveView = CurView;
	hView = GSSiGlobAlloc ( 734,GHND,sizeof(VIEWPORT));
	CurView = SaveView2 = (LPVIEWPORT)GlobalLock (hView); 
	if (Prefix)
		_fstrcpy (CurView->Prefix,Prefix); 
	if (UDI)                   
		_fstrcpy (CurView->UDI,UDI);
	CurView->ReportRefno=ref;
	CurView->hReport = LoadReport (Name);
    if (!CurView->hReport) 
    {
		Rtn = FALSE;
		goto Exit;
	}
	CurView->Active = TRUE;
	CurView->ShrinkToFit=FALSE; 
	hReportScroll = CurView->hReport;	
	DisplayReportScroll (0,0);
	UnloadReport (&SaveView2->hReport); 
Exit:
	GSSiGlobUlFree (&hView);
	SetCurView ( SaveView);
	return Rtn;
}

BOOL DisplayReportScroll (HWND hWndDlg, int ScrollCntl)
{       
	RECT	Rect;
	
	ScrollRptDlg = hWndDlg;
	ScrollRptCntl = ScrollCntl;	
	return (DisplayReport2 (0,0,Rect,1,TRUE,NULL));
}  

BOOL DisplayScrollReport (LPSTR Name, LPSTR Prefix, LPSTR UDI, long ref)
{
	FARPROC lpfnSCROLLREPORTMsgProc;
	BOOL	Rtn=FALSE;
	LPVIEWPORT	SaveView, SaveView2;        
	HANDLE	hView;
	
	SaveView = CurView;
	hView = GSSiGlobAlloc ( 735,GHND,sizeof(VIEWPORT));
	CurView = SaveView2 = (LPVIEWPORT)GlobalLock (hView); 
	if (Prefix)
		_fstrcpy (CurView->Prefix,Prefix); 
	if (UDI)                   
		_fstrcpy (CurView->UDI,UDI);
	CurView->ReportRefno=ref; 
	CurView->hReport = LoadReport (Name);
    if (!CurView->hReport) 
		goto Exit;
	CurView->Active = TRUE;
	CurView->ShrinkToFit=FALSE; 
	hReportScroll = CurView->hReport;	
	lpfnSCROLLREPORTMsgProc = MakeProcInstance((FARPROC)SCROLLREPORTMsgProc, hInst);
	Rtn = DialogBox(hInst, (LPSTR)"SCROLLREPORT", hWndMain, lpfnSCROLLREPORTMsgProc);
	FreeProcInstance(lpfnSCROLLREPORTMsgProc); 
	UnloadReport (&SaveView2->hReport); 
Exit:
	GSSiGlobUlFree (&hView); 
	SetCurView ( SaveView);
	return Rtn;
}

BOOL DisplayScrollReport2 (LPSTR Name, LPSTR Prefix, LPSTR UDI, long ref)
{
	FARPROC lpfnSCROLLREPORTMsgProc;
	BOOL	Rtn=FALSE;
	LPVIEWPORT	SaveView, SaveView2;        
	
	SaveView = CurView;
	if (hViewScroll)
	{
		 LPVIEWPORT	VP = (LPVIEWPORT)GlobalLock (hViewScroll);
		 UnloadReport (&VP->hReport);
		 GlobalUnlock (hViewScroll); 
		 GSSiGlobUlFree (&hViewScroll);
	}    
	hViewScroll = GSSiGlobAlloc ( 736,GHND,sizeof(VIEWPORT));
	CurView = SaveView2 = (LPVIEWPORT)GlobalLock (hViewScroll); 
	if (Prefix)
		_fstrcpy (CurView->Prefix,Prefix); 
	if (UDI)                   
		_fstrcpy (CurView->UDI,UDI);
	CurView->ReportRefno=ref; 
	CurView->hReport = LoadReport (Name);
    if (!CurView->hReport) 
	{
		GSSiGlobUlFree (&hViewScroll);
	} 
	else
	{
		CurView->Active = TRUE;
		CurView->ShrinkToFit=FALSE; 
		hReportScroll = CurView->hReport;
		if (hWndScroll2) 
		{
			Rtn = TRUE;
			PostMessage(hWndScroll2, WM_COMMAND, IDOK, 0L);  
		}
		else
		{	
			lpfnSCROLLREPORTMsgProc = MakeProcInstance((FARPROC)SCROLLREPORTMsgProc2, hInst);
			Rtn = CreateDialog(hInst, (LPSTR)"SCROLLREPORT2", hWndMain, lpfnSCROLLREPORTMsgProc);
		}   
	}
	SetCurView ( SaveView);
	return Rtn;
}

BOOL ReportToFile (LPSTR Name, LPSTR Prefix, LPSTR UDI, long ref,LPSTR File)
{
	BOOL	Rtn=FALSE;
	LPVIEWPORT	SaveView, SaveView2;        
	HANDLE	hView;   
	RECT	Rect;
	OFSTRUCT	OFStruct;
	
	SaveView = CurView;
	hView = GSSiGlobAlloc ( 737,GHND,sizeof(VIEWPORT));
	CurView = SaveView2 = (LPVIEWPORT)GlobalLock (hView); 
	if (Prefix)
		_fstrcpy (CurView->Prefix,Prefix); 
	if (UDI)                   
		_fstrcpy (CurView->UDI,UDI);
	CurView->ReportRefno=ref;
	CurView->hReport = LoadReport (Name);
    if (!CurView->hReport) 
		goto Exit;
	CurView->Active = TRUE;
	CurView->ShrinkToFit=FALSE; 
	hReportScroll = CurView->hReport;	
	ScrollRptDlg = 1;
	ScrollRptCntl = GSSiOpenFile (File,NULL,OF_CREATE);
	if (ScrollRptCntl != HFILE_ERROR)
	{   
		Rtn = DisplayReport2 (0,0,Rect,1,TRUE,NULL);  
		GSSiClose (ScrollRptCntl);
	}
	UnloadReport (&SaveView2->hReport); 
Exit:
	GSSiGlobUlFree (&hView);
	SetCurView ( SaveView);
	return Rtn;
}

BOOL DisplayReport2 (HDC hDC, HANDLE hReport, RECT Rect, double Factor,BOOL CloseFiles,LPRECT pSizeRect)
{
	LPREPORT	pReport, SaveCurReport;
	int			irow, itab, MaxRowLen=0, ReportHeight=0, RowHeight, ReportWidth, x, y,xj,yj=0,w,lt, Margin,ifont;  
	LPSTR		pRow, Tabloc, StartTab, EndTab, pFirstRow, Tabstr;
	char		tabstr[16], FontStr[32];  
	int			PixPerInch; 
	static		double	LastFactor;
	DWORD		TextExt; 
	HFONT		OldFont;   
	BOOL		NullLine;
	int			hinc, winc,i;
	HRGN		hRgn;  
	HANDLE		hTemp;
	LPSTR		pRow2;
	
	if (hDC)
	{
		SetDisplayMode (hDC, GF_TEXTMODE);    
		SelectClipRgn (hDC,NULL);
	}
	else
		hReport = hReportScroll;  
	if (!hReport)
		return FALSE;
	if (!OpenReportFiles (hReport))
		return FALSE;
	pReport = (LPREPORT)GlobalLock (hReport);   
	pReport->hDC = hDC;
	pReport->Rect = Rect;  
	if (pSizeRect)
	{
		pReport->WantSize = TRUE;
		RectInit (&pReport->SizeRect); 
	}
	else
		pReport->WantSize = FALSE;
	if (hDC) 
	{
		SaveDC (hDC);
		SetTextColor (CurView->hDC,0);
		CreateReportFonts(pReport,Factor);
		_fstrcpy (FontStr,"[%FONT]=0;");
		ExpandText (FontStr);
    	OldFont = SelectObject(hDC, GetStockObject(SYSTEM_FONT));
    	SelectObject (hDC,OldFont);  
    		
		Margin = min (IDNINT(pReport->Margin * (Rect.bottom-Rect.top)), 
					  IDNINT(pReport->Margin * (Rect.right-Rect.left)));
		pReport->Rect.left += Margin;
		pReport->Rect.top += Margin;
		pReport->Rect.right -= Margin;
		pReport->Rect.bottom -= Margin;
		pReport->y = pReport->Rect.top;	
		pReport->x = pReport->Rect.left;  
		PixPerInch = GetDeviceCaps(hDC, LOGPIXELSY);  

/*    	if (pReport->First)
    		LastFactor = 1;
		for (i=0;i<pReport->NumTabs;i++)
			pReport->TabLen[i]=(((float)pReport->TabLen[i])/LastFactor)*DeviceToScreenFactor;  
		LastFactor = DeviceToScreenFactor; */
		hRgn = CreateRectRgn (pReport->Rect.left,pReport->Rect.top,pReport->Rect.right,pReport->Rect.bottom);
		SelectClipRgn (hDC,hRgn);
		DeleteObject (hRgn);
//SelectClipRgn (hDC,NULL);
    }  
    else if (ScrollRptDlg > 1) 
    {   
    	HDC	hDlgDC;
    	HWND	hWnd;
    	float	DBU;
    	
    	hDlgDC = GetDC (CurView->hWnd);
		PixPerInch = GetDeviceCaps(hDlgDC, LOGPIXELSY);   
		ReleaseDC (CurView->hWnd,hDlgDC);
		DBU = (float)(LOWORD(GetDialogBaseUnits())) / 4;
		for (i=0;i<pReport->NumTabs;i++)
			pReport->TabLen[i]=IDNINT(((float)pReport->TabLen[i]/100)*(PixPerInch/DBU));
		pReport->hScrollLine = GSSiGlobAlloc ( 738,GHND,1024);
		if (ScrollRptDlg)
		{
	    	HANDLE	hTabs=GSSiGlobAlloc ( 739,GMEM_MOVEABLE,pReport->NumTabs*sizeof(short));
	    	LPSHORT	pTabs=(LPSHORT)GlobalLock (hTabs); 
	    	pTabs[0] = pReport->TabLen[0];
	    	for (i=1;i<pReport->NumTabs;i++)
	    		pTabs[i] = pTabs[i-1] + pReport->TabLen[i];
			SendDlgItemMessage (ScrollRptDlg,ScrollRptCntl,LB_SETTABSTOPS,pReport->NumTabs,(LPARAM)pTabs);
			GSSiGlobUlFree (&hTabs);
		}
    }  
    else
		pReport->hScrollLine = GSSiGlobAlloc ( 740,GHND,1024);
	SaveCurReport = CurReport;
	CurReport = pReport; 
	
	if (pReport->hRows)
	{   
		LPSTR	pRowBeg, pRowEnd;
		
		pRow = GlobalLock (pReport->hRows); 
		hTemp = GSSiGlobAlloc ( 741,GMEM_MOVEABLE,UINT_MAX);
		pRow2 = GlobalLock (hTemp);
		pRowBeg = pRow;
		do
		{
			pRowEnd = MatchLev (pRowBeg,';'); 
			if (pRowEnd)
				*pRowEnd = 0;
			_fstrcpy (pRow2,pRowBeg); 
			ExpandText (pRow2);  
			if (pRowEnd)
				*pRowEnd++ = ';'; 
			pRowBeg = pRowEnd;
		}
		while (pRowEnd);       
		GlobalUnlock (pReport->hRows);
		GSSiGlobUlFree (&hTemp);   
	}   
    if (hDC) 
    {
    	SelectObject (hDC,OldFont);
	    DestroyReportFonts(pReport);
	}
	else if (CurReport->hScrollLine)
	{ 
		LPSTR	pLine = GlobalLock (CurReport->hScrollLine);
		
		if (*pLine)
		{
			if (ScrollRptDlg == 1)
				fputstring (pLine,ScrollRptCntl);
			else if (ScrollRptDlg)
	 			SendDlgItemMessage (ScrollRptDlg,ScrollRptCntl,LB_ADDSTRING,(WPARAM)NULL,(LPARAM)pLine); 
	 	}
		GSSiGlobUlFree (&pReport->hScrollLine);
	}
	if (CloseFiles)
		CloseReportFiles (hReport);   
	pReport->First = FALSE; 
	if (pSizeRect)
		*pSizeRect = pReport->SizeRect;
	GlobalUnlock (hReport);
	CurReport = SaveCurReport; 
	if (hDC)
		RestoreDC (hDC,-1);
	return TRUE;
} 

void UnloadReport (LPHANDLE phReport)
{
	LPREPORT	pReport; 
	    
	if (!*phReport) return;    
	pReport = (LPREPORT)GlobalLock (*phReport); 
	if (!pReport)
	{
		*phReport = 0;
		return;
	}
	if (pReport->FilesAreOpen)
		CloseReportFiles (*phReport);
	GSSiGlobFree (&pReport->hFiles);
	GSSiGlobFree (&pReport->hRows);
	GSSiGlobUlFree (phReport);
	return;
}

BOOL AddReportToPrintList (LPSTR ReportFile,long Refno,LPSTR Prefix, LPSTR UDI)
{   
	char	str[260];
	
	if (!ReportFile) 
	{
		GSSiRemove (PrintReportListFile);
		return TRUE;
	}
	if (!*PrintReportListFile)
		GSSiGetTempFileName (NULL,"gm",NULL,(LPSTR)PrintReportListFile); 
	sprintf (str,"%s,%ld,%s,%s",ReportFile,Refno,Prefix,UDI);
	AppendFile (PrintReportListFile,str); 
	return TRUE;
}
  
BOOL GetNextPrintReport (BOOL First,LPSTR ReportName,LPLONG pRefno,LPSTR Prefix,LPSTR UDI)
{   
	HFILE	Fid;
	long	ReportNum=0;  
	char	str[260];  
	LPSTR	pComma;
	
	if (First)
		NextReportNum = 0; 
	NextReportNum++;  
	if (!*PrintReportListFile)
		return FALSE;
	Fid = GSSiOpenFile (PrintReportListFile,NULL,OF_READ);
	if (Fid == HFILE_ERROR)
		return FALSE;
	while (ReportNum < NextReportNum && fgetstring (str,256,Fid))
		ReportNum++;                            
	GSSiClose (Fid);
	if (ReportNum < NextReportNum)   
	{
		GSSiRemove (PrintReportListFile);
		return FALSE;                   
	} 
	if (!(pComma = _fstrrchr (str,',')))
		return FALSE;
	*pComma++ = 0;
	_fstrcpy (UDI,pComma);
	pComma = _fstrrchr (str,',');
	*pComma++ = 0;
	_fstrcpy (Prefix,pComma);
	pComma = _fstrrchr (str,',');
	*pComma++ = 0;
	*pRefno = atol (pComma);
	_fstrcpy (ReportName,str);
	
	return TRUE;
		
}

BOOL FAR PASCAL SCROLLREPORTMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
	char	UserID[64], Password[32];	
    RECT	rect; 
    POINT	pt;    
    static	HANDLE	hSaveBM=0;
    int		height,width,x,y;
 switch(Message)
   {
    case WM_INITDIALOG:  
        hSaveBM = EnterBlockingWindow (hWndDlg);
		{ 
			DWORD dwStringExt;
			TEXTMETRIC tm;
			HDC hdcLB=GetDC (hWndDlg);
			
			GetTextMetrics (hdcLB,&tm);
			dwStringExt = tm.tmAveCharWidth*255;
		
		    SendDlgItemMessage(hWndDlg, IDC_SCROLLBOX, LB_SETHORIZONTALEXTENT,
		        			   LOWORD(dwStringExt), 0L);
		    ReleaseDC (hWndDlg,hdcLB);
		}

         CurView->hWnd = hWndDlg;
		 GetWindowRect(hWndMain, &rect);    
		 rect.left = max(0,rect.left);
		 rect.top = max(0,rect.top); 
		 x = rect.left;
		 y = rect.top;
		 height = rect.bottom-rect.top-6;    
		 width = rect.right - rect.left-6;
	 	 SetWindowPos(hWndDlg, (HWND) NULL, x+1, y+1,width, height,0); 
	 	 GetClientRect(hWndDlg,&rect);
	 	 SetWindowPos(GetDlgItem(hWndDlg,IDC_SCROLLBOX),(HWND)NULL, 0, 0,rect.right, rect.bottom,0);
		 if ( !DisplayReportScroll (hWndDlg,IDC_SCROLLBOX))
    	 	PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
			
         break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
    	 PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
         {  
		    case IDM_EXIT:	 
            case IDCANCEL: 
                GSSiEndDialog(hWndDlg, FALSE,hSaveBM);  
            break;

		    case IDM_PRINT:
				PrintScrollReport (hWndDlg);
		    	break;
    
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 

BOOL FAR PASCAL SCROLLREPORTMsgProc2(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{ 
    POINT	pt;    
    int		height,width,x,y; 
    LPVIEWPORT	VP; 
    RECT	rect; 
    short	Choice;
 switch(Message)
   {
    case WM_INITDIALOG:  
		{ 
			DWORD dwStringExt;
			TEXTMETRIC tm;
			HDC hdcLB=GetDC (hWndDlg);
			
			GetTextMetrics (hdcLB,&tm);
			dwStringExt = tm.tmAveCharWidth*255;
		
		    SendDlgItemMessage(hWndDlg, IDC_SCROLLBOX, LB_SETHORIZONTALEXTENT,
		        			   LOWORD(dwStringExt), 0L);
		    ReleaseDC (hWndDlg,hdcLB);
		}

	 	 if (ScrollRect.right>ScrollRect.left)
	 	 	rect = ScrollRect;
	 	 else
		 	GetWindowRect(hWndMain, &rect);    
		 rect.left = max(0,rect.left);
		 rect.top = max(0,rect.top); 
		 x = rect.left;
		 y = rect.top;
		 height = rect.bottom-rect.top;    
		 width = rect.right - rect.left;
	 	 SetWindowPos(hWndDlg, (HWND) NULL, x, y,width, height,0); 
	 	 GetClientRect(hWndDlg,&rect);
	 	 SetWindowPos(GetDlgItem(hWndDlg,IDC_SCROLLBOX),(HWND)NULL, 0, 0,rect.right, rect.bottom,0);  
	 	 hWndScroll2 = hWndDlg;
		 PostMessage(hWndDlg, WM_COMMAND, IDOK, 0L);
			
         break; /* End of WM_INITDIALOG                                 */
	case WM_DESTROY:
	     if (!hViewScroll)
	     	break;
		 VP = (LPVIEWPORT)GlobalLock (hViewScroll);
		 UnloadReport (&VP->hReport);
		 GlobalUnlock (hViewScroll); 
		 GSSiGlobUlFree (&hViewScroll);    
		 hWndScroll2 = 0;
         break;
    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
	     DestroyWindow(hWndDlg);  
         RedisplayWindow ();
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
         {  
         	case IDOK: 
		 		SendDlgItemMessage (hWndDlg,IDC_SCROLLBOX,LB_RESETCONTENT,0,0);
				if ( !DisplayReportScroll (hWndDlg,IDC_SCROLLBOX))
		    	 	PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         		break;
		    case IDM_EXIT:	 
            case IDCANCEL: 
    	 		PostMessage(hWndDlg, WM_CLOSE, 0, 0L);
            break;

		    case IDM_PRINT:
				PrintScrollReport (hWndDlg);
		    	break;
		    case IDC_SCROLLBOX: 
		    {
                ContinueProcessing = TRUE;
                switch(HIWORD(lParam))
                {    
                     case LBN_DBLCLK:
                     {
                     	HANDLE	hStr=GSSiGlobAlloc ( 742,GMEM_MOVEABLE,4096);
                     	LPSTR	pStr=GlobalLock (hStr);
						if (GetGlobalCVal ("[%REPORT2MACRODBLCLK]",pStr,""))
							ExpandText (pStr);
		         		GSSiGlobUlFree (&hStr); 
		         		break;
		         	 }
                     case LBN_SELCHANGE: 
                     {
                     	HANDLE	hStr=GSSiGlobAlloc ( 743,GMEM_MOVEABLE,4096);
                     	LPSTR	pStr=GlobalLock (hStr);
                     	
						Choice=SendDlgItemMessage(hWndDlg,IDC_SCROLLBOX,LB_GETCURSEL,NULL,NULL); 
						SendDlgItemMessage(hWndDlg,IDC_SCROLLBOX,LB_GETTEXT,Choice,(DWORD)pStr); 
						SetGlobalValue("%REPORT2TEXT",pStr);
						SetFocus (hWndMain);
						SetConfig (1); 
						SetViewport(*pCommandViewport); 
						if (GetGlobalCVal ("[%REPORT2MACRO]",pStr,""))
							ExpandText (pStr);
		         		GSSiGlobUlFree (&hStr);  
		         		break;
		         	}
		         }
		     }
		     	break;
    
         }
         break;    /* End of WM_COMMAND                                 */

    default:
        return FALSE;
   }
 return TRUE;
} 

