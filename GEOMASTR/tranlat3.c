#include "graphint.h" 
#include "translat.h"
#include "extrndb.h"
#include "pnet.h"
#include "gmextern.h"

static	short	styles[2][100], stylecounts[2][100],nstyles[2]={0,0};
static long	nChronoFiles=0;

double GetTextSizeInBounds (short nchar,double rot,LPMNMXCORD pBounds)
{   
	double	size,rot2=LTWOPI(rot+HALFPI); 
	MNMXCORD	TestBounds;
	DPOINT	MidPoint = MinMaxMidPointD (pBounds), Point;
	
	return (0.065);
	size = 1.01*(pBounds->ymx - pBounds->ymn);    
	do
	{   
		size *= 0.99;
		DBoundsInit (&TestBounds); 
		Point = dnewpt (MidPoint,rot,nchar*0.4*size/2);
		AddDPointToMinMax (&Point,&TestBounds);
		Point = dnewpt (Point,rot2,size/2);
		AddDPointToMinMax (&Point,&TestBounds);
		Point = dnewpt (Point,rot2,-size);
		AddDPointToMinMax (&Point,&TestBounds);
		Point = dnewpt (MidPoint,rot,-nchar*0.4*size/2);
		AddDPointToMinMax (&Point,&TestBounds); 
		Point = dnewpt (Point,rot2,size/2);
		AddDPointToMinMax (&Point,&TestBounds);
		Point = dnewpt (Point,rot2,-size);
		AddDPointToMinMax (&Point,&TestBounds);
	}
	while (!BoundsInBounds (&TestBounds,pBounds,0));
	return size;
}

void ConvertUMText (LPSTR Text,LPSHORT plTxt)
{   
	HANDLE	hNew=GSSiGlobAlloc ( 915,GHND,512);
	LPSTR	NewText=GlobalLock(hNew);
	LPSTR	pBeg, pEnd, pNew=NewText;
	short	ii;
	
	Text[*plTxt] = 0; 
	REPLAC (Text,"#","\r\n",512);  
	if (_fstrstr (Text,"\\"))
		ii=1;
	if (_fstrstr (Text,"<"))
		ii=1;
	if (_fstrstr (Text,"!"))
		ii=1;
	pBeg = Text; 
	while (*pBeg) 
	{   
		if (*pBeg == '[')
		{   
			if ((pEnd = _fstrchr (pBeg,']')))
			{   
				pBeg++;
				*pEnd = 0;
				sprintf (pNew,"$LN(%s)",pBeg); 
				pBeg = _fstrchr (pBeg,0)+1;
				pNew = _fstrchr (pNew,0);
				goto Next;
			}
		}			   
		*pNew++ = *pBeg++; 
Next:;
	}
	*plTxt= _fstrlen (NewText);
	_fstrcpy (Text,NewText);     
	GSSiGlobUlFree (&hNew);
	return;
} 

void AddStyle (short i,short style)
{   short	j;
    
    i--;
	for (j=0;j<nstyles[i];j++)
	{
		if (styles[i][j] == style)
		{
			stylecounts[i][j]++;
			return;
		}
	}
	stylecounts[i][nstyles[i]]=0;
	styles[i][nstyles[i]++]=style;
	return;
}



long ConvertAndTranCoord (LPDPOINT pDPoint,HANDLE hTranFile)
{
	long	rtn;
	
	rtn = ConvertCoord (pDPoint,3,1);
	if (hTranFile)
		TRANS2 (pDPoint->x,pDPoint->y,&pDPoint->x,&pDPoint->y,hTranFile);
	return rtn;
}    

   

BOOL ImportSSURGOTables (LPSTR CompFile,LPSTR CompGMD)
{   
	char	AttFile[128], TextFile[128];
	HANDLE	hStr=GSSiGlobAlloc ( 920,GMEM_MOVEABLE,4096);  
	LPSTR	str=GlobalLock (hStr);
	HANDLE	hVal=GSSiGlobAlloc ( 921,GMEM_MOVEABLE,4096);  
	LPSTR	vals=GlobalLock (hVal);
	HANDLE	hLoc=GSSiGlobAlloc ( 922,GMEM_MOVEABLE,4096);  
	LPSTR	*loc=(LPSTR*)GlobalLock (hLoc);
	HFILE	Fid;
	OFSTRUCTGM	OFStruct;
	LPSTR	lpTAB, EndVar, Val;
	short	nVar=1,nVar2=1, i,j;  
	LPGWDHEADER lpGWDHead;
	HANDLE  hDB=0;
	BOOL	rtn=FALSE;
	char	muid[12]; 
	long	line;  

	
	if (!ExistFile (CompFile)) 
	{
		MessageBox (GetFocus(),"Comp file missing",NULL,MB_ICONEXCLAMATION);
		goto Exit; 
	}
	if (!ExistFile (CompGMD))
	{
		Fid = GSSiOpenFile ("[%INDIR]comp.txt",&OFStruct,OF_READ);
		if (Fid == HFILE_ERROR)
		{
			MessageBox (GetFocus(),"CompGMD template file missing",NULL,MB_ICONEXCLAMATION);
			goto Exit;
		}
		fgetstring (str,4090,Fid); 
		GSSiClose (Fid);
		lpTAB = str;
		while ((lpTAB = _fstrchr (lpTAB,',')))
		{
			nVar++;
			lpTAB++;
		}
		CreateGWDDatabase (CompGMD,1,TRUE,nVar,2,str);
	}
	Fid = GSSiOpenFile (CompFile,&OFStruct,OF_READ);
	fgetstring (str,4090,Fid);   
	*loc = str;
	nVar = 1;
	while ((lpTAB=_fstrchr (*loc,'\t')))
	{   
		nVar++;
		EndVar = _fstrchr (*loc,':');
		*EndVar = 0; 
		lpTAB++;
		loc++;
		*loc = lpTAB;
	}
	EndVar = _fstrchr (*loc,':');
	*EndVar = 0; 
	GlobalUnlock (hLoc);
	hDB = OpenGWDatabase (CompGMD,BT_WRITE);
	if (!hDB)
	{
		MessageBox (GetFocus(),"Unable to open soil attribute database",CompGMD,MB_ICONEXCLAMATION);
		goto Exit;
	}
    lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
	fgetstring (vals,4090,Fid); 
	line = 2;  
	while (fgetstring (vals,4090,Fid))
	{   
		line++;
		loc = (LPSTR*)GlobalLock (hLoc); 
		Val = vals;
		for (i=0;i<nVar;i++)
		{   
			if (!Val)
			{   
				char	mess[512];
				
				sprintf (mess,"Error in %s at line %ld",OFStruct.szPathName,line);
				MessageBox (GetFocus(),mess,NULL,MB_ICONEXCLAMATION);
				goto NextLine;
			}
			if ((lpTAB=_fstrchr (Val,'\t')))
				*lpTAB++ = 0;  
			if (!_fstricmp(*loc,"stssaid"))
			{ 
				_fstrncpy (muid,Val,2);
				muid[2] = 0;
			}  
			else
			{   
				LPSTR	vloc=Val;
				
				if (!_fstricmp(*loc,"muid"))
				{ 
					_fstrcat (muid,Val);  
					vloc = muid;
				} 
				Truncate (vloc); 
				if (!SetFieldValFromCharAndName(lpGWDHead, *loc, vloc, FALSE, TRUE))
				{
					MessageBox (GetFocus(),*loc,"Error loading field",MB_ICONEXCLAMATION);
					GlobalUnlock (hLoc);
					goto Exit;
				}
			}
			Val = lpTAB;
			loc++;
		} 
NextLine:
		GlobalUnlock (hLoc);
		GWDAddRecord (lpGWDHead,0,NULL);
	} 
	GSSiClose (Fid);
	rtn = TRUE;
Exit: 
	if (hDB)
		GlobalUnlock (hDB);
    CloseGWDatabase (hDB);
   	GSSiGlobUlFree (&hStr);
	GSSiGlobFree (&hLoc);
	GSSiGlobUlFree (&hVal);
	return rtn;
}
 
   

  

DWORD MIFColorToWinColor (COLORREF MIFColor)
{ 
	BYTE	R,G,B; 
	
	R = GetRValue (MIFColor);
	G = GetGValue (MIFColor);
	B = GetBValue (MIFColor);
	return RGB (B,G,R);
}

BOOL GetMIFCharacteristics (HFILE FidMIF, LPSTR str,LPLONG lineno)
{
    LPSTR   lpBeg, lpEnd; 
    long    ii;
    
NextLine:
    if (!fgetstring(str,256,FidMIF))
        return TRUE;
    (*lineno)++;  
    lpBeg = FirstNonBlank(str);
    if (!_fstrnicmp (lpBeg,"PEN ",4))
    {
        lpBeg = _fstrchr (str,'(');
        lpEnd = _fstrchr (lpBeg++,',');
        *lpEnd = 0;
        MIFPenWidth = atoi (lpBeg);
        lpBeg = ++lpEnd;
        lpEnd = _fstrchr (lpBeg,',');
        *lpEnd = 0;
        MIFPenStyle = atoi (lpBeg);
        lpBeg = ++lpEnd;
        lpEnd = _fstrchr (lpBeg,')');
        *lpEnd = 0;
        MIFPenColor = MIFColorToWinColor(atol (lpBeg)); 
    }
    else if (!_fstrnicmp (lpBeg,"BRUSH ",6))
    { 
        lpBeg = _fstrchr (str,'(');
        lpEnd = _fstrchr (lpBeg++,',');
        *lpEnd = 0;
        MIFPattern = atoi (lpBeg);
        lpBeg = ++lpEnd;
        lpEnd = _fstrchr (lpBeg,',');
        *lpEnd = 0;
        MIFForeColor = MIFColorToWinColor(atol (lpBeg));
        lpBeg = ++lpEnd;
        lpEnd = _fstrchr (lpBeg,')');
        *lpEnd = 0;
        MIFBackColor = MIFColorToWinColor(atol (lpBeg)); 
    }
    else if (!_fstrnicmp (lpBeg,"SYMBOL ",7))
    {
    }
    else if (!_fstrnicmp (lpBeg,"CENTER ",7))
    {
    } 
    else if (!_fstrnicmp (lpBeg,"SMOOTH",6))
    {
    } 
    else
        return FALSE;
    goto NextLine;
}

BOOL GetMIFTextCharacteristics (HFILE FidMIF, LPSTR str,LPLONG lineno,LPDOUBLE pAngle)
{
    LPSTR   lpBeg, lpEnd; 
    long    ii;
    
    *pAngle = 0;
NextLine:
    if (!fgetstring(str,256,FidMIF))
        return TRUE;
    (*lineno)++;  
    lpBeg = FirstNonBlank(str);
    if (!_fstrnicmp (lpBeg,"FONT ",4))
    {
        lpBeg = _fstrchr (str,'(');
        lpEnd = _fstrchr (lpBeg++,',');
        *lpEnd = 0;
        MIFPenWidth = atoi (lpBeg);
        lpBeg = ++lpEnd;
        lpEnd = _fstrchr (lpBeg,',');
        *lpEnd = 0;
        MIFPenStyle = atoi (lpBeg);
        lpBeg = ++lpEnd;
        lpEnd = _fstrchr (lpBeg,')');
        *lpEnd = 0;
        MIFPenColor = MIFColorToWinColor(atol (lpBeg)); 
    }
    else if (!_fstrnicmp (lpBeg,"SPACING ",8))
    { 
    }
    else if (!_fstrnicmp (lpBeg,"JUSTIFY ",8))
    {
    }
    else if (!_fstrnicmp (lpBeg,"ANGLE ",6))
    { 
    	lpBeg += 6;
    	*pAngle = atof (lpBeg) * RADDEG;
    } 
    else if (!_fstrnicmp (lpBeg,"LABEL ",6))
    {
    } 
    else
        return FALSE;
    goto NextLine;
}


   
BOOL GetMIDData(HFILE FidMID,LPSTR lpMIDstr,HANDLE hDLT) 
{   
    LPSTR   lpStart, lpEnd, lpChar;
    char    EndChar; 
    short     i;
    
    if (FidMID==HFILE_ERROR) return TRUE;
    if (!fgetstring(lpMIDstr,2040,FidMID))
        return TRUE; 
    GetDelimTextData(lpMIDstr,hDLT,2040);
    return TRUE;
} 

COLORREF AutoYellow (COLORREF color)
{   

	if (AutoOrthoColor != (COLORREF)-1 && CurView && CurView->HaveOrthos && !color)
	{
		AutoOpaque = AutoOpaqueSetting;
		return AutoOrthoColor;   
	}
	AutoOpaque = FALSE;
	return color;
}


COLORREF ConvertColor (COLORREF Color,int UseHalfTone)
{   
	//UseHalfTone = -1 always half tone, =-2 never halftone ,= 0 never halftone unless CurView->AlwaysUseHalfTone, > 0 halftone if Symnum=UseHalfTone in viewport halftone vis 
	BOOL	AlwaysUseHalfTone=TRUE;

	if ((long)Color < 0)
		goto Exit2; 
	if (UseHalfTone == -2)
		goto Exit;
	if (CurView)
		AlwaysUseHalfTone=CurView->AlwaysUseHalfTone;
	Color = AutoYellow (Color);
	if (!ForceHalfTone && !AlwaysUseHalfTone)
	{
		if (!UseHalfTone)
			goto Exit;
		if (UseHalfTone > 0)
		{
			if (!GetHalfToneVisibility (UseHalfTone))
				goto Exit;
		} 
	}
	if (CurView && CurView->HalfTone)
	{
		short	R=GetRValue (Color);
		short	G=GetGValue (Color);
		short	B=GetBValue (Color);  
//        double	HT = ((double)CurView->HalfTone*10)/100;
        
//		Color = RGB(R+(255-R)*HT,G+(255-G)*HT,B+(255-B)*HT);
		Color = RGB (R+((255-R)*CurView->HalfTone)/10,
					 G+((255-G)*CurView->HalfTone)/10,
					 B+((255-B)*CurView->HalfTone)/10);
	}
	if (CurView && CurView->ConvertToGray) 
		Color = ConvertToGray (Color);
Exit:
	if (UseSymnumColor)
	{
		int	b=GetBValue (Color);
		int	g=GetGValue (Color);
		int	r=GetRValue (Color);

		if (GetBValue (Color) > AdjustBlue)
			Color = RGB(r,g,251);
		else if (AdjustBlue == 256)
			Color = RGB(r,g,253+CurrentLakeType);
	}
Exit2:
	return Color;
}

BOOL LoadTIN (LPSTR FromFile, LPSTR ToPlt)
{   
	HANDLE	hSymDesc=0;
	short	NumSyms=0, TINSym, n;  
	char	str[260];   
	HFILE	FidTIN;
	OFSTRUCTGM	OFStruct;
	long	TotLen, CurLoc,PointID[4];  
	double	Elev;
	DPOINT	Point;
	HPDPOINT3D	pPoints;
	int		NumPoints=0; 
	HANDLE	hPoints;  
	MNMXCORD	Bounds;
  	long	Ref=1;  
  	BOOL	rtn;
    
    if ((FidTIN = GSSiOpenFile(FromFile,&OFStruct,OF_READ)) == HFILE_ERROR)
    	return FALSE;
//    GetGlobalCVal ("[%LINKMAP]",Name,"testlink.plt");
	CreateStatusWind (hWndMain,1,NULL);
	DBoundsInit (&Bounds);
    TotLen = GSSillseek (FidTIN,0,2); 
    GSSillseek (FidTIN,0,0);   
	StatusWindowUpdate (OFStruct.szPathName,"Scanning for Min/Max",TotLen,0);
    while (fgetstring (str,256,FidTIN) && ContinueProcessing) 
    { 
    	n = sscanf (str,"%ld %lf %lf %lf",PointID,&Point.x,&Point.y,&Elev);
    	Point.x *= FTM;
    	Point.y *= FTM;
    	AddDPointToMinMax (&Point,&Bounds);
		CurLoc = GSSillseek (FidTIN,0,1);
			StatusWindowUpdate (NULL,NULL,TotLen,CurLoc);
    }
    CreateNewMap (ToPlt,&Bounds,0,NULL,0,NULL,0,0,TRUE);
    GSSillseek (FidTIN,0,0);   
    hPoints = GSSiGlobAlloc ( 950,GMEM_MOVEABLE,4*sizeof(DPOINT3D));    
    TINSym =  GetDictSymbolNumber ("TINTRIANGLE");
	AddToSymList (TINSym,&NumSyms,&hSymDesc); 
	StatusWindowUpdate (NULL,"Loading Areas",TotLen,0);
    while (fgetstring (str,256,FidTIN) && ContinueProcessing)
    {
    	short   rtn;
		        
		pPoints = (HPDPOINT3D)GlobalLock (hPoints); 
		pPoints += NumPoints; 
    	n = sscanf (str,"%ld %lf %lf %lf",&PointID[NumPoints],&pPoints->x,&pPoints->y,&pPoints->z);     
    	if (PointID[NumPoints] != 13 && PointID[NumPoints] != 14)
    		continue;
    	pPoints->x *= FTM;
    	pPoints->y *= FTM;
    	pPoints->z *= FTM;
    	GlobalUnlock (hPoints); 
    	NumPoints++;
    	if (NumPoints == 4)
    	{   
    		short	i=NumPoints--;
    		long	id = PointID[0];
    		
    		while (i--)
    			if (PointID[i] != id)
    				id = 0;
    		
			rtn=AddPolyToMap (1,&NumPoints, &hPoints,0,Ref++,0,-1,TINSym,NULL,NULL,NULL,-1,-1,-1,0,0,0,NULL,2,0); 
			NumPoints = 0;   
			CurLoc = GSSillseek (FidTIN,0,1);
			StatusWindowUpdate (NULL,NULL,TotLen,CurLoc);
		}
    }
    rtn = ContinueProcessing;
    SetContinueProcessing ( TRUE);
    GSSiClose (FidTIN);
	GSSiGlobFree (&hPoints);
    CloseMap(TRUE);  
	AddSymToMap (NumSyms,hSymDesc,0,NULL); 
    DestroySymList (&NumSyms,&hSymDesc);
	DestroyStatusWindow(0);  
	return rtn;
}

BOOL CreateVideoPolys (LPSTR InFile,LPSTR PltFile)
{
	char	str[260], TrackID[66];  
	MNMXCORD	Bounds;
	OFSTRUCTGM	OFStruct;
	HFILE	Fid, Fid2; 
	HANDLE	hDLT=0, hDLT2=0;   
	BOOL	err;
	HDIB	hDib;
	
//	hDib = FrameToDIB2 ("c:\\mgvvid\\V0500559\\5413.avi", 10000);
     
    if ((Fid = GSSiOpenFile (InFile,&OFStruct,OF_READ)) == HFILE_ERROR)
    	return FALSE;
    GetGlobalCVal ("[%PROJECTBOUNDS]",str,NULL); 
    Bounds = atobounds (str,&err); 
    _fstrcpy (PltName,PltFile);
    CreateNewMap (PltFile,&Bounds,0,NULL,0,NULL,0,0,TRUE); 
    fgetstring (str,256,Fid);
	ProcessDelimTextHeader(str, NULL, Fid, &hDLT, 0, 0);
 	       
    while (fgetstring (str,256,Fid))
    {
		GetDelimTextData(str,hDLT,256);
	    if ((Fid2 = GSSiOpenFile ("[FULLNAME]",&OFStruct,OF_READ)) != HFILE_ERROR)
	    {
			HANDLE	hSymDesc=0; 
			LPSTR	pEnd;
			short	NumSyms=0, SymNum=GetDictSymbolNumber("VIDEOTRACK");  
	    	short	rtn;
	    	long	Refno;
	    	int		nPoints=0;
	    	HANDLE	hPoints=GSSiGlobAlloc ( 951,GMEM_MOVEABLE,(long)sizeof(DPOINT)*USHRT_MAX); 
			HPDPOINT	pPoints = (HPDPOINT)GlobalLock (hPoints);    

		    fgetstring (str,256,Fid2);
				        
			ProcessDelimTextHeader(str, NULL, Fid, &hDLT2, 0, 0);
		    while (fgetstring (str,256,Fid2))
		    {
				GetDelimTextData(str,hDLT2,256);
				pPoints[nPoints].y = GetGlobalDVal ("[X]");
				pPoints[nPoints].x = GetGlobalDVal ("[Y]");   
				ConvertCoord (&pPoints[nPoints],2,1);
				nPoints++;
		    }
		    GSSiClose (Fid2); 
		    GlobalUnlock (hPoints);
			AddToSymList (SymNum,&NumSyms,&hSymDesc); 
			Refno = GetNewRefno(PltName,NULL,NULL,NULL,NULL);  
			_fstrcpy (str,"[DIRECTORY]");
			ExpandText (str);
			pEnd = _fstrrchr (str,'\\');
			*pEnd = 0;
			pEnd = _fstrrchr (str,'\\');
			pEnd++;
			_fstrcpy (TrackID,pEnd);
			_fstrcat (TrackID,"\\");
			_fstrcat (TrackID,"[FILENAME]");
			ExpandText (TrackID);
			rtn=AddPolyToMap (1,&nPoints, &hPoints,1,Refno,NULL,2,SymNum,NULL,"VIDTRACK",TrackID,-1,-1,-1,0,0,0,0,TRUE,0);
			GSSiGlobFree (&hPoints);
		    CloseMap(TRUE);  
			AddSymToMap (NumSyms,hSymDesc,0,NULL); 
		    DestroySymList (&NumSyms,&hSymDesc);
		}
    } 
    GSSiClose (Fid);
    return TRUE;
}
  
BOOL OpenChronoIndex (LPSTR PName,LPMNMXCORD pMinMaxCoord)
{
	static	char	Drive[8], Dir[MAX_PATH], FullName[MAX_PATH];
    LPFILEINDEX lpIndex;
 	HANDLE	handle;    
 	LPSTR	pChronoDir, pPar, pName;
 	BOOL	rtn=FALSE;
 	LPLONG	pDate;  
 	LPUSHORT	pFileNo;
	
	GSSiGlobFree (&hChronoIndex);
	DBoundsInit (pMinMaxCoord); 
	_fstrcpy (Dir,PName);
	ExpandText (Dir);
	_fullpath (FullName,Dir,sizeof(FullName));
	_splitpath (FullName,Drive,Dir,NULL,NULL); 
	sprintf (FullName,"%s%sindex",Drive,Dir);
   	IgnoreBounds=TRUE;
	handle = OpenMapIndex (FullName,NULL);
	if (!handle)
		goto Exit; 
	rtn = TRUE;
	nChronoFiles = 0;
	hChronoIndex = GSSiGlobAlloc ( 952,GHND,USHRT_MAX);
	pChronoDir = GlobalLock (hChronoIndex);
	sprintf (pChronoDir,"%s%s",Drive,Dir);
	
	lpIndex = (LPFILEINDEX)GlobalLock (handle); 
NextFile:
	CloseMap(FALSE);
	pChronoDir = _fstrchr (pChronoDir,0);  
	pChronoDir++;
    if (lpIndex->FileInIndex >= (long)lpIndex->NumFiles)
    {   
        if (!(lpIndex=GetNextIndexHeader(&handle,TRUE)))
        	goto Exit;
        goto NextFile;
    }
    GetNextIndexEntry(lpIndex); 
    if (*lpIndex->CurrentEntry->Name == '\\')
    	pName = lpIndex->CurrentEntry->Name + 1;
    else
    	pName = lpIndex->CurrentEntry->Name;
	sprintf (PltName,"%s%s%s",Drive,Dir,pName);  
	if ((pPar = _fstrrchr (PltName,'(')))
		*pPar = 0;
 	if (!OpenMap (0,CurView->hDC))
 		goto NextFile;
	AddMinMaxD (pMinMaxCoord, &CurView->FileMNMX);
	pDate = (LPLONG)pChronoDir;
	*pDate++ = MinFileTime;
	*pDate++ = MaxFileTime; 
	pFileNo = (LPUSHORT)pDate;
	*pFileNo++ = lpIndex->FileInIndex; 
	pChronoDir = (LPSTR)pFileNo;
	_fstrcpy (pChronoDir,pName);
	if ((pPar = _fstrrchr (pChronoDir,'(')))
		*pPar = 0;
	nChronoFiles++;
	goto NextFile;
		
Exit:      
	if (hChronoIndex)
		GlobalUnlock (hChronoIndex);
	IgnoreBounds = FALSE;
	return rtn;	
}

void CloseChronoIndex (void)
{
	GSSiGlobFree (&hChronoIndex);
	return;
}

BOOL GetChronoIndexedEditFile (LPSTR PName,long BegTime, long EndTime)
{   
	char	PltName[MAX_PATH];
 	BOOL	rtn=FALSE;
 	LPSTR	pIndex, pChronoIndex;  
 	long	n=nChronoFiles, FTime, TTime; 
 	LPLONG	pDate; 
 	long	DateRange, CurDateRange=LONG_MAX;   
 	USHORT	FileInIdx;
 	LPUSHORT	pFileNo;
	
	if (!hChronoIndex)
		return FALSE;
	pIndex = pChronoIndex = GlobalLock (hChronoIndex);
	while (n--) 
	{
		pChronoIndex = _fstrchr (pChronoIndex,0);
		pChronoIndex++; 
		pDate = (LPLONG)pChronoIndex;
		FTime = *pDate++;
		TTime = *pDate++; 
		pFileNo = (LPUSHORT)pDate;
		FileInIdx = *pFileNo++;
		pChronoIndex = (LPSTR)pFileNo;
		if (BegTime >= FTime && BegTime <= TTime &&
			EndTime >= FTime && EndTime <= TTime)
		{
			rtn = TRUE;
			DateRange = TTime - FTime; 
			if (DateRange < CurDateRange)
			{
				CurDateRange = DateRange;
				sprintf (PltName,"%s%s",pIndex,pChronoIndex); 
                FileInIndex = FileInIdx;
			}
		} 
	} 
	if (rtn)
		if (_fstricmp (PName,PltName))
		{
			DPOINT Dpoint={0,0};
		
			AddPointToMap (Dpoint,0,0,0,0,0,0,0,0,0,0,0,0,0,TRUE,FALSE,NULL,NULL);
			CloseMap (TRUE);
			_fstrcpy (PName,PltName);
		}
	GlobalUnlock (hChronoIndex);
	return rtn;
}

BOOL AddSymToDir (LPSTR DirName,short NumSyms,HANDLE hSymDesc,short NumPens,LPPENDESC pPenDesc)
{
	char	Drive[8], Dir[MAX_PATH], FullName[MAX_PATH];
    LPFILEINDEX lpIndex;
 	HANDLE	handle;
 	BOOL	rtn=FALSE; 
   	HCURSOR	OldCursor; 
   	LPSTR	pName, pPar;
	                	
    OldCursor = GSSiSetCursor (LoadCursor (0,IDC_WAIT));
	_fullpath (FullName,DirName,sizeof(FullName));
	_splitpath (FullName,Drive,Dir,NULL,NULL); 
	sprintf (FullName,"%s%sindex",Drive,Dir);
	
   	IgnoreBounds=TRUE;
	handle = OpenMapIndex (FullName,NULL);
	if (!handle)
		goto Exit;
	lpIndex = (LPFILEINDEX)GlobalLock (handle); 
NextFile:
    if (lpIndex->FileInIndex >= (long)lpIndex->NumFiles)
    {   
        if (!(lpIndex=GetNextIndexHeader(&handle,TRUE)))
        	goto Exit;
        goto NextFile;
    }
    GetNextIndexEntry(lpIndex);     
    pName = lpIndex->CurrentEntry->Name;
    if (*pName == '\\')
    	pName++;
	sprintf (PltName,"%s%s%s",Drive,Dir,pName); 
	if ((pPar = _fstrrchr (PltName,'('))) 
		*pPar = 0;
	rtn=AddSymToMap (NumSyms,hSymDesc,NumPens,pPenDesc);
	goto NextFile;
	
Exit:
    GSSiSetCursor (OldCursor);
	IgnoreBounds = FALSE;
	return rtn;	
}

HANDLE ReversePoints (long NumPoints,HANDLE hPoints)
{
	HANDLE		hNewPoints;
	HPDPOINT	pNewPoint;
	HPDPOINT	pPoint;
	
	if (!NumPoints)
		return 0;  
	hNewPoints = GSSiGlobAlloc ( 953,GMEM_MOVEABLE,NumPoints * sizeof(DPOINT));
	pNewPoint = (HPDPOINT)GlobalLock (hNewPoints); 
	pPoint = (HPDPOINT)GlobalLock (hPoints);
	pPoint += (NumPoints-1);
	while (NumPoints--)
		*pNewPoint++ = *pPoint--;
	GlobalUnlock (hNewPoints);
	GSSiGlobUlFree (&hPoints);
	return hNewPoints;
}

HANDLE ReversePoints3D (long NumPoints,HANDLE hPoints)
{
	HANDLE		hNewPoints;
	HPDPOINT3D	pNewPoint;
	HPDPOINT3D	pPoint;
	
	if (!NumPoints)
		return 0;  
	hNewPoints = GSSiGlobAlloc ( 953,GMEM_MOVEABLE,NumPoints * sizeof(DPOINT3D));
	pNewPoint = (HPDPOINT3D)GlobalLock (hNewPoints); 
	pPoint = (HPDPOINT3D)GlobalLock (hPoints);
	pPoint += (NumPoints-1);
	while (NumPoints--)
		*pNewPoint++ = *pPoint--;
	GlobalUnlock (hNewPoints);
	GSSiGlobUlFree (&hPoints);
	return hNewPoints;
}

void ReversePoints2 (long NumPoints,HPDPOINT pPoint)
{
	long	i;
	
	if (!NumPoints)
		return;  
	{
		HANDLE hNewPoints=GSSiGlobAlloc ( 954,GMEM_MOVEABLE,NumPoints * sizeof(DPOINT));
		HPDPOINT	pNewPoint = (HPDPOINT)GlobalLock (hNewPoints); 
		
		for (i=0;i<NumPoints;i++)
			pNewPoint[i] = pPoint[NumPoints - i - 1];
		for (i=0;i<NumPoints;i++)
			pPoint[i] = pNewPoint[i];
		GSSiGlobUlFree (&hNewPoints); 
	}
	return;
}
void ReversePoints3D2 (long NumPoints,HPDPOINT3D pPoint)
{
	long	i;
	
	if (!NumPoints)
		return;  
	{
		HANDLE hNewPoints=GSSiGlobAlloc ( 954,GMEM_MOVEABLE,NumPoints * sizeof(DPOINT3D));
		HPDPOINT3D	pNewPoint = (HPDPOINT3D)GlobalLock (hNewPoints); \
		
		for (i=0;i<NumPoints;i++)
			pNewPoint[i] = pPoint[NumPoints - i - 1];
		for (i=0;i<NumPoints;i++)
			pPoint[i] = pNewPoint[i];
		GSSiGlobUlFree (&hNewPoints); 
	}
	return;
}

HANDLE ReversePoints3 (long NumPoints,HPDPOINT pPoint)
{
	HANDLE		hNewPoints;
	HPDPOINT	pNewPoint;
	
	if (!NumPoints)
		return 0;  
	hNewPoints = GSSiGlobAlloc ( 953,GMEM_MOVEABLE,NumPoints * sizeof(DPOINT));
	pNewPoint = (HPDPOINT)GlobalLock (hNewPoints); 
	pPoint += (NumPoints-1);
	while (NumPoints--)
		*pNewPoint++ = *pPoint--;
	GlobalUnlock (hNewPoints);
	return hNewPoints;
}


   

HANDLE LoadDLGPoints (LPINT pNumSides,LPSHORT *pLineID,HANDLE hLineIndex,HFILE Fid)
{   
	LPLONG	pLineLoc; 
	HPDPOINT	pPoints, pPoints2;
	HANDLE	hPoints, hPoints2; 
	short	loc, i, nPnts, nrec, n, j;   
	char	str[84]; 
	BOOL	First=TRUE;
	long	NumSides = 1;
	
	hPoints = GSSiGlobAlloc ( 960,GMEM_MOVEABLE,(long)USHRT_MAX*(long)sizeof(DPOINT));
	pPoints = (HPDPOINT)GlobalLock (hPoints); 
	while (**pLineID)
	{
		if (**pLineID < 0)
		{
			hPoints2 = GSSiGlobAlloc ( 961,GMEM_MOVEABLE,(long)USHRT_MAX*(long)sizeof(DPOINT)); 
			pPoints2 = pPoints;
			pPoints = (HPDPOINT)GlobalLock (hPoints2);
		} 
		pLineLoc = (LPLONG)GlobalLock (hLineIndex); 
		pLineLoc += abs(**pLineID);
		GSSillseek (Fid,*pLineLoc,0);
		GlobalUnlock (hLineIndex); 
		fgetstring (str,80,Fid); 
		nPnts = atoi (&str[32]); 
		NumSides+=(nPnts-1); 
		nrec = (nPnts-1)/3 + 1; 
		n=nPnts;
		while (nrec--)
		{
			fgetstring (str,80,Fid);
			loc = 0; 
			j=n;
			for (i=0;i<min(j,3);i++)
			{
				pPoints->x = atof (&str[loc]);
				loc+=12; 
				pPoints->y = atof (&str[loc]);
				loc+=12;
	            ConvertCoord(pPoints,3,1);
	            pPoints++; 
	            n--;
			} 
		} 
		if (**pLineID < 0)
		{   
			while (nPnts--)
			{
				pPoints--;
				*pPoints2++ = *pPoints;
			}
			GlobalUnlock (hPoints2);
			GSSiGlobUlFree (&hPoints2); 
			pPoints = pPoints2;
		} 
		pPoints--; // keeps from duplicating node points
		*(*pLineID)++;     
		First = FALSE;
	}
	*(*pLineID)++;
	GlobalUnlock (hPoints); 
	if (NumSides > SHRT_MAX)
		MessageBox (GetFocus(),"Too many points in area",NULL,MB_ICONEXCLAMATION); 
	*pNumSides = NumSides; 
	hPoints = GSSiGlobalReAlloc (0,hPoints,NumSides*(long)sizeof(DPOINT),GMEM_MOVEABLE);
	return hPoints; 
} 



BOOL GetIndexedEditFile (LPSTR PltName,LPMNMXCORD pBounds)
{ 
	char	Drive[8], Dir[MAX_PATH], FullName[MAX_PATH];
    LPFILEINDEX lpIndex;
 	HANDLE	handle;
 	BOOL	rtn=FALSE; 
	
	_fullpath (FullName,PltName,sizeof(FullName));
	_splitpath (FullName,Drive,Dir,NULL,NULL); 
	sprintf (FullName,"%s%sindex",Drive,Dir);
	
   	IgnoreBounds=TRUE;
	handle = OpenMapIndex (FullName,NULL);
	if (!handle)
		goto Exit;
	lpIndex = (LPFILEINDEX)GlobalLock (handle); 
NextFile:
    if (lpIndex->FileInIndex >= (long)lpIndex->NumFiles)
    {   
        if (!(lpIndex=GetNextIndexHeader(&handle,TRUE)))
        	goto Exit;
        goto NextFile;
    }
    GetNextIndexEntry(lpIndex);
    if (BoundsInBounds (pBounds,&lpIndex->CurrentEntry->Bounds,0))
    {
		sprintf (PltName,"%s%s%s",Drive,Dir,lpIndex->CurrentEntry->Name); 
	    GSSiGlobUlFree (&handle);
		rtn = TRUE;
	}
	else
 		goto NextFile;
	
Exit:
	IgnoreBounds = FALSE;
	return rtn;	
} 

 

