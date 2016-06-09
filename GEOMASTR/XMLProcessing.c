#include "graphint.h"   

#include "gmextern.h"


HANDLE LoadXMLFile (LPSTR File)
{
	HANDLE hFile=0;
	int	lFile = GSSiLength (File);
	HFILE	Fid;
	LPSTR	pFile;
	int		i;

	if (lFile > 0)
	{
		hFile = GSSiGlobAlloc (1700,GMEM_MOVEABLE,lFile+1);
		Fid = GSSiOpenFile (File,0,OF_READ);
		pFile = GlobalLock (hFile);
		BigRead (Fid,pFile,lFile);
		pFile[lFile--] = 0;
		for (i=0;i<lFile;i++,pFile++)
			if (!*pFile)
				*pFile = 1;
		GlobalUnlock (hFile);
		GSSiClose (Fid);
	}

	return hFile;
}

HANDLE GetNextXMLElement (HANDLE FileHandle,LPINT pFileLoc,LPSTR TagID)
{
	HANDLE hTag=0;
	LPSTR	pFile, pFileBeg, pLoc, pEnd, pTag;
	char	search[256];
	int		ii=16;

	if (FileHandle)
	{
		pFile = pFileBeg = GlobalLock (FileHandle);
		if (pFileLoc)
			pFile += *pFileLoc;
		pEnd = strchr (pFile,0);
		sprintf (search,"<%s ",TagID);
		pLoc = strstr (pFile,search);
		if (!pLoc)
		{
			sprintf (search,"<%s>",TagID);
			pLoc = strstr (pFile,search);
		}
		if (pLoc)
		{
			sprintf (search,"</%s>",TagID);
			pEnd = strstr (pLoc,search);
			if (pEnd)
			{
				int lTag = (int)pEnd - (int)pLoc + strlen(search) + 1;

				if (pFileLoc)
					*pFileLoc = ((int)pEnd + strlen (search)) - (int)pFileBeg;

				hTag = GSSiGlobAlloc (1701,GMEM_MOVEABLE,lTag);
				pTag = GlobalLock (hTag);
				strncpy0 (pTag,pLoc,lTag-1);
				GlobalUnlock (hTag);
			}
		}
		GlobalUnlock (FileHandle);
	}

	return hTag;
}

BOOL GetXMLElementAttribute (HANDLE hElem,LPSTR AttrName,LPSTR AttrValue)
{
	LPSTR pElem, pEnd, pAtt, pAttEnd;
	char	search [256];

	*AttrValue = 0;

	if (hElem)
	{
		pElem = GlobalLock (hElem);
		pEnd = strchr (pElem,'>');
		if (pEnd)
			*pEnd = 0;
		sprintf (search, "%s=",AttrName);
		pAtt = strstr (pElem,search);
		if (pAtt)
		{
			pAtt += strlen (search);
			if (*pAtt == '"')
			{
				pAtt++;
				if ((pAttEnd = strchr (pAtt,'"')))
				{
					*pAttEnd = 0;
					strcpy (AttrValue,pAtt);
					*pAttEnd = '"';
				}
			}
		}
		if (pEnd)
			*pEnd = '>';
		GlobalUnlock (hElem);
	}
	return TRUE;
}

HANDLE GetXMLElementValue (HANDLE hElement)
{
	HANDLE	hValue = 0;
	LPSTR	pElement, pValue;
	LPSTR	pBeg, pEnd;
	char	endStr[128];

	if (!hElement)
		return 0;
	pElement = GlobalLock (hElement);
	if ((pBeg = strchr(pElement,'>')))
	{
		LPSTR	pSpace;

		*pBeg++ = 0;
		if ((pSpace = strchr (pElement,' ')))
			*pSpace = 0;
		sprintf (endStr,"</%s>",pElement+1);
		if ((pEnd = strstr (pBeg,endStr)))
		{
			*pEnd = 0;
			hValue = GSSiGlobAlloc (1702,GMEM_MOVEABLE,strlen (pBeg)+1);
			pValue = GlobalLock (hValue);
			strcpy (pValue,pBeg);
			GlobalUnlock (hValue);
			*pEnd = '<';
		}
		if (pSpace)
			*pSpace = ' ';
		*--pBeg = '>';
	}
	GlobalUnlock (hElement);
	return hValue;
}

BOOL TestGPX (LPSTR FileName)
{
	BOOL	rtn=FALSE;
	HANDLE	hFile, hGPXElem, hBoundsElem, hWpt;
	MNMXCORD	Bounds;
	int		loc;
	char	str[256];

	if ((hFile = LoadXMLFile (FileName)))
	{
		if (!(hGPXElem = GetNextXMLElement (hFile,0,"gpx")))
			MessageBox (0,"This is not a valid GPX file",0,MB_ICONEXCLAMATION);
		else
		{
			DBoundsInit (&Bounds);
			if ((hBoundsElem = GetNextXMLElement (hGPXElem,0,"bounds")))
			{
				if (GetXMLElementAttribute (hBoundsElem,"minlat",str))
					Bounds.ymn = atof (str);
				if (GetXMLElementAttribute (hBoundsElem,"minlpn",str))
					Bounds.xmn = atof (str);
				if (GetXMLElementAttribute (hBoundsElem,"maxlat",str))
					Bounds.ymx = atof (str);
				if (GetXMLElementAttribute (hBoundsElem,"maxlon",str))
					Bounds.xmx = atof (str);
			}
//	Load waypoints
			loc = 0;
			while ((hWpt = GetNextXMLElement (hGPXElem,&loc,"wpt")))
			{
				DPOINT	Wpt;

				GetXMLElementAttribute (hWpt,"lat",str);
				Wpt.y = atof (str);
				GetXMLElementAttribute (hWpt,"lon",str);
				Wpt.x = atof (str);

			}
		}
		GSSiGlobFree (&hFile);
	}
	return rtn;
}

BOOL ProcessGPXRecord (HDC hDC,long RecordNumber)
{
	LPSTR	pRecord;
	DPOINT	DPoint;
	char	str[256];
	int		idesc;
	BOOL	rtn=FALSE;

	if (!hGPXRecord)
		return FALSE;
	pRecord = GlobalLock (hGPXRecord);
	ShowValue (hDC,FALSE);  
	InGraphicsProcessor = TRUE;
	ItemIsDeleted = FALSE;
	ItemIsRemoved = FALSE;              	
	InitRecord (hDC); 
    /*SetSHPParms (RecordNumber);  
    if (!CurrentDesc)
    	goto RtnFalse;
	if (GRStartTime > TimeRangeEnd || GREndTime < TimeRangeBeg)
		goto RtnFalse;
    ltag = _fstrlen (SHPTag);
	SetSymNum (CurrentDesc);
	if (!ProcessRefAndTAG (GetVisibility (CurrentDesc),SHPTag,ltag))
		goto RtnFalse;*/
	if (!strnicmp (pRecord,"<wpt ",5))
	{
		HANDLE hWPT = hGPXRecord;
		HANDLE hGPXSym, hSymValue;

		GetXMLElementAttribute (hWPT,"lat",str);
		DPoint.y = atof (str);
		GetXMLElementAttribute (hWPT,"lon",str);
		DPoint.x = atof (str);
		ConvertCoord(&DPoint,2,1);
		hGPXSym = GetNextXMLElement (hWPT,0,"sym");
		hSymValue = GetXMLElementValue (hGPXSym);
		GSSiGlobFree (&hGPXSym);
		if (hSymValue)
		{
			LPSTR pSym = GlobalLock (hSymValue);
			LPSTR pComma = strchr (pSym,',');

			if (pComma)
				*pComma = 0;
			CurrentDesc = GetDictSymbolNumber (pSym);   
			GSSiGlobUlFree (&hSymValue);
		}
		{//from ProcessSHPRecord
			int	 SDCrtn;
			double	size;

			lpDCurPoints = &DPoint;  
			CurrentPoint = CurPointLocD = DPoint;
	        LastElementBeginPoint = LastElementEndPoint = DPoint;
	        nPnts = 1; 
			CurPointLoc = BasePtToWinPt(lpDCurPoints); 
    		if (!PointInMaskAreaWinCoord (CurPointLoc))
    			goto RtnFalse;
    		if (PointIsBlocked (&CurPointLocD,CurrentDesc))
    			goto RtnFalse;   
       		HaveTXLoc = TRUE;
    		CurrentType = GF_POINT; 
			if (CurPointSize < 0)
				CurPointSize = -CurPointSize * DeviceToScreenFactor();
			else
				CurPointSize /= CurView->BaseUnitsPerPixel;
			if ((Pick||PickingByRefno) && GetTypeVisibility(TYPE_POINT))
			{
				PickPointItemD (lpDCurPoints,(CurPointSize*ThemeWidthFactor)*CurView->BaseUnitsPerPixel,PTRot,CurrentDesc);			 		    
			} 
			else if (GetTypeVisibility(TYPE_POINT))
			{   
				short	iDesc=CurrentDesc;
						
				if (CopyRec)
				{   
				    AddPointToBuffer (CurPointLocD,CurrentRefno,0,CurrentDesc,10, PTRot,0,0,0,
									  CurrentPrefix,CurrentUDI,-1,-1,-1,TRUE,&hUpdateBuf,&lUpdateBuf,0);
				}
				else
				{
	 				if (CurrentDesc > 0 && CurrentDesc < 3201)   
	 				{
						if (TSize)
							CurView->CurVisType[CurrentDesc]=5;
						else
							CurView->CurVisType[CurrentDesc]=4;
					}
					HighlightPointSym=FALSE;
					if (SHPColor >= 0)
					{  
						GlobalColors[0]=SHPColor;  
						HaveVarFillColor = TRUE;
					}
					if (!GetTypeVisibility(6) && SymbolIsVisible (iDesc)) 
					{
						CurPointSize = 10*DeviceToScreenFactor();
						iDesc = InvisiblePointSymbol;
					}
					if ((SDCrtn = SetDisplayChar (hDC,GF_POINT,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI)) > 0)
					{   
						if (ThemePointSym)
						{
							iDesc = ThemePointSym;
							if (ThemePointSize < 0)
								size = -ThemePointSize *DeviceToScreenFactor();
							else
								size = ThemePointSize / CurView->BaseUnitsPerPixel; 
							size *= ThemeWidthFactor;
							size = min(max (size,1),MaxPointSize);
						}
						else
							size = CurPointSize*ThemeWidthFactor*GraphicsPointFactor; 
						{
							long	DisplayedWidth=0;

							DisplayPointItem (hDC,CurPointLoc,size,PTRot,iDesc,&DisplayedWidth);
							CurView->MaxSymbolWidth = max (CurView->MaxSymbolWidth,DisplayedWidth);
							CurView->MaxFileDisplayedPointWidth[FileNum] = max (CurView->MaxFileDisplayedPointWidth[FileNum],(DisplayedWidth/ FileDistToWinDist)-(((long)CurrentItemMinMax.xmx)-CurrentItemMinMax.xmn));
						}
		       			HaveTXLoc = 1;
					}
				}
			}
   			TXLoc = CurPointLocD; 
			if (SDCrtn < 0)
       			HaveTXLoc = 1;
			rtn = TRUE;
RtnFalse:;		
		}
	}
	GlobalUnlock (hGPXRecord);
	InGraphicsProcessor = FALSE;
	return rtn;
}

BOOL GetGPXRecordBounds (DWORD Recno,LPMNMXCORD pBounds)
{
	return FALSE;
}

BOOL LoadGPXParm (LPSTR DGNFileName)
{

	return TRUE;
}

BOOL LoadKMLParm (LPSTR DGNFileName)
{

	return TRUE;
}

HANDLE OpenGPXFile (LPSTR InName,LPMNMXCORD pBounds)
{
	BOOL	rtn=FALSE;
	HANDLE	hFile, hBoundsElem, hWpt;
	MNMXCORD	Bounds;
	int		loc;
	char	str[256];
	DPOINT	pt;

	GPXpos = 0;
	if ((hFile = LoadXMLFile (InName)))
	{
		if (!(hGPXElem = GetNextXMLElement (hFile,0,"gpx")))
			MessageBox (0,"This is not a valid GPX file",0,MB_ICONEXCLAMATION);
		else
		{
			DBoundsInit (&Bounds);
			if ((hBoundsElem = GetNextXMLElement (hGPXElem,0,"bounds")))
			{
				if (GetXMLElementAttribute (hBoundsElem,"minlat",str))
					Bounds.ymn = atof (str);
				if (GetXMLElementAttribute (hBoundsElem,"minlpn",str))
					Bounds.xmn = atof (str);
				if (GetXMLElementAttribute (hBoundsElem,"maxlat",str))
					Bounds.ymx = atof (str);
				if (GetXMLElementAttribute (hBoundsElem,"maxlon",str))
					Bounds.xmx = atof (str);
				GSSiGlobFree (&hBoundsElem);
			}
			else
			{
				int loc = 0;
				HANDLE	hWPT;

				while ((hWPT = GetNextXMLElement (hGPXElem,&loc,"wpt")))
				{
					 GetXMLElementAttribute (hWPT,"lon",str);
					 pt.x = atof (str);
					 GetXMLElementAttribute (hWPT,"lat",str);
					 pt.y = atof (str);
					 AddDPointToMinMax (&pt,&Bounds);
					 GSSiGlobFree (&hWPT);
				}
			}
			if (ValidBounds (&Bounds))
			{
				if (pBounds)
					*pBounds = Bounds;
			}
			else
				 CloseGPXFile (&hFile);
		}
	}
	return hFile;
}

BOOL CloseGPXFile (LPHANDLE phFile)
{
	GSSiGlobFree (phFile);
	GSSiGlobFree (&hGPXElem);
	GSSiGlobFree (&hGPXRecord);
	
	return TRUE;
}
BOOL CloseKMLFile (LPHANDLE phFile)
{
	GSSiGlobFree (phFile);
	GSSiGlobFree (&hKMLElem);
	GSSiGlobFree (&hKMLRecord);
	
	return TRUE;
}

BOOL GetNextGPXSegment (BOOL Init)
{
	BOOL	rtn = FALSE;

	GSSiGlobFree (&hGPXRecord);
	hGPXRecord = GetNextXMLElement (hGPXElem,&GPXpos,"wpt");

	if (hGPXRecord)
		rtn = TRUE;

	return rtn;
}

BOOL SetGPXVis (HWND hWndDlg, int DlgItemSym, int DlgItemPar,HFILE FidSymList)
{
	LPSTR	pDesc, pClause, pC, pColor, pWidth, pRot; 
	short	idesc,i; 
	
	if (!hGPX)
		return FALSE;
	else
	{
		HANDLE	 hGPXElem;
		char	str[256];
		DPOINT	pt;
		int loc = 0;
		HANDLE	hWPT;

		if ((hGPXElem = GetNextXMLElement (hGPX,0,"gpx")))
		{
			while ((hWPT = GetNextXMLElement (hGPXElem,&loc,"wpt")))
			{
				HANDLE hGPXSym = GetNextXMLElement (hWPT,0,"sym");
				HANDLE hSymValue = GetXMLElementValue (hGPXSym);

				GSSiGlobFree (&hGPXSym);
				if (hSymValue)
				{
					LPSTR pSym = GlobalLock (hSymValue);
					LPSTR pComma = strchr (pSym,',');

					if (pComma)
						*pComma = 0;
					idesc = GetDictSymbolNumber (pSym);   
					AddSymToList (hWndDlg,DlgItemSym,DlgItemPar,idesc,FidSymList);
					GSSiGlobUlFree (&hSymValue);
				}
				GSSiGlobFree (&hWPT);
			}
		}
		GSSiGlobFree (&hGPXElem);
	}
	return TRUE;
}


HANDLE OpenKMLFile (LPSTR InName,LPMNMXCORD pBounds)
{
	HANDLE	hFile, hBoundsElem, hWpt;
	MNMXCORD	Bounds;
	int		loc;
	char	str[256];
	DPOINT	pt;

	KMLpos = 0;
	if ((hFile = LoadXMLFile (InName)))
	{
		if (!(hKMLElem = GetNextXMLElement (hFile,0,"kml")))
			MessageBox (0,"This is not a valid KML file",0,MB_ICONEXCLAMATION);
		else
		{
			DBoundsInit (&Bounds);
			if ((hBoundsElem = GetNextXMLElement (hKMLElem,0,"bounds")))
			{
				if (GetXMLElementAttribute (hBoundsElem,"minlat",str))
					Bounds.ymn = atof (str);
				if (GetXMLElementAttribute (hBoundsElem,"minlpn",str))
					Bounds.xmn = atof (str);
				if (GetXMLElementAttribute (hBoundsElem,"maxlat",str))
					Bounds.ymx = atof (str);
				if (GetXMLElementAttribute (hBoundsElem,"maxlon",str))
					Bounds.xmx = atof (str);
				GSSiGlobFree (&hBoundsElem);
			}
			else
			{
				int loc = 0;
				HANDLE	hKML;

				while ((hKML = GetNextXMLElement (hKMLElem,&loc,"wpt")))
				{
					 GetXMLElementAttribute (hKML,"lon",str);
					 pt.x = atof (str);
					 GetXMLElementAttribute (hKML,"lat",str);
					 pt.y = atof (str);
					 AddDPointToMinMax (&pt,&Bounds);
					 GSSiGlobFree (&hKML);
				}
			}
			if (ValidBounds (&Bounds))
			{
				if (pBounds)
					*pBounds = Bounds;
			}
			else
				 CloseKMLFile (&hFile);
		}
	}
	return hFile;
}
