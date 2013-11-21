#include "graphint.h"

#include "gmextern.h"

#include <sys\types.h>
#include <sys\stat.h>         

static	char	Value[512], KeyVal[512], VarVal[1024], MinClassValue[512];
static short	DefaultHltClass;
static THEMEHIGHLIGHTKEY	ThemeHighlightKey;
static THEMEHIGHLIGHTDATA	ThemeHighlightData;
static	struct {POINT Point;
		double	AZ; 
		short	Type; 
		UINT	nPnts;
		HANDLE	hPoints; 
		short	nPoly;
		HANDLE	hPolyPartLen;
		LPTHEME pTheme;  
		long	Refno;
		char	Text[1024];} ShowVal;
                                         
static struct {
		long	MaskAreaRefno;
		long	AreaRefno;
		} ShowValKey;  

typedef struct {
		long	MaskAreaRefno;
		long	AreaRefno; 
		char	Prefix[8],
				UDI[64];
		DPOINT	Point;
		float	Size;
		} SHOWVALDATA;  
typedef SHOWVALDATA	FAR	*LPSHOWVALDATA;

static	HANDLE	hShowValDB=0;
static	char	ShowValDB[256]="";                                         

void SetShowValDB (LPSTR DBName)
{
	if (DBName)
		_fstrcpy (ShowValDB,DBName);
	else
	{
		CloseGWDatabase (hShowValDB);
		hShowValDB = 0;
	}
	return;
}

void SetShowValPoly (long Refno,BOOL Close)
#if ENABLETRACE
{GSSiEnterProg (1225);
#endif
{   
	UINT	i; 
	HPPOINT	pPoint, lpPoints=lpCurPoints;
	HPDPOINT	lpDPoints=lpDCurPoints;
	
	GSSiGlobFree (&ShowVal.hPoints); 
	GSSiGlobFree (&ShowVal.hPolyPartLen); 
	if (Close)
		return; 
	ShowVal.Refno = Refno;
	ShowVal.nPnts = nPnts;
	ShowVal.hPoints = GSSiGlobAlloc (1323,GMEM_MOVEABLE,(long)sizeof(POINT) * (long)nPnts); 
	pPoint = (HPPOINT)GlobalLock (ShowVal.hPoints);
	for (i=0;i<nPnts;i++,pPoint++) 
	{   
		if (HiPrecis)
			*pPoint = BasePtToWinPt (lpDPoints++);   
		else
    		*pPoint = FileCoordToWinCoord(*lpPoints++);
    } 
    GlobalUnlock (ShowVal.hPoints);
    if (hPolyPartLen)
    {    
    	LPWORD	pPartLen, pPartLen2;
    	
    	ShowVal.nPoly = nPoly;
    	ShowVal.hPolyPartLen = GSSiGlobAlloc ( 309,GMEM_MOVEABLE,(long)nPoly*2);
  		pPartLen = (LPWORD)GlobalLock (hPolyPartLen);
   		pPartLen2 = (LPWORD)GlobalLock (ShowVal.hPolyPartLen);    
   		hmemmove ((HPSTR)pPartLen2,(HPSTR)pPartLen,(long)nPoly*2);
   		GlobalUnlock (hPolyPartLen);
   		GlobalUnlock (ShowVal.hPolyPartLen);
    }
    else 
    {
    	ShowVal.nPoly = 0;  
    	ShowVal.hPolyPartLen = 0;
    }
{
#if ENABLETRACE
GSSiExitProg (1225);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

BOOL SwitchThemeSHPFile (void)
{   
	LPSTR	CurShpFile, SaveThemeDataFile, DBFullPath;
	BOOL	rtn=FALSE;
	LPOPENSQLDATA	SQLPtr;
	LPOPENFILEDATA	FilePtr;
	HANDLE	hMem=0; 
	LPSTR	pDot;  
	long	SaveCSR;
	
	if (!CurTheme)
		return FALSE;
	if (CurTheme->DataFileType != SHAPE_DATAFILE && !_fstrchr(&CurTheme->DataFile[1],'['))
		return FALSE;   
	hMem = GSSiGlobAlloc (0,GMEM_MOVEABLE,1024);
	CurShpFile = GlobalLock (hMem);
	SaveThemeDataFile = CurShpFile + 256;
	DBFullPath = SaveThemeDataFile + 256;
	if (CurTheme->DataFileType == SHAPE_DATAFILE)
	   	GetSHPName (CurShpFile); 
	else
	{
		_fstrcpy (CurShpFile,CurTheme->DataFile);
		ExpandText (CurShpFile);
	} 
   	if (CurTheme->hThemeDB)
   	{
		SQLPtr = (LPOPENSQLDATA) GlobalLock (CurTheme->hThemeDB);
	    FilePtr = (LPOPENFILEDATA)GlobalLock (SQLPtr->OFHandle); 
	    _fstrcpy (DBFullPath,FilePtr->fullpath);
		GlobalUnlock (SQLPtr->OFHandle);
		GlobalUnlock (CurTheme->hThemeDB); 
		if ((pDot = _fstrrchr (DBFullPath,'.')))
			*pDot = 0;
		if ((pDot = _fstrrchr (CurShpFile,'.')))
			*pDot = 0;
		if (!_fstricmp (CurShpFile,DBFullPath))
			goto Exit; 
		if (pDot)
			*pDot = '.';
		CloseThemeDataFile(TRUE);
	}
	_fstrcpy (SaveThemeDataFile,CurTheme->DataFile);
	_fstrcpy (CurTheme->DataFile,CurShpFile); 
	SaveCSR = CurrentSHPRec;
	OpenThemeDataFile (); 
	CurrentSHPRec = SaveCSR;
	_fstrcpy (CurTheme->DataFile,SaveThemeDataFile); 
Exit:  
	GSSiGlobUlFree (&hMem);
	return rtn;
}

void GetNextBlack (LPLONG pColor)
{
	BYTE r = GetRValue (*pColor);
	BYTE g = GetGValue (*pColor);
	BYTE b = 0;//use when classno expanded to 4 bytes
	if (g < r)
		g++;
	else 
	{
		r++; 
		g = 0;
	}
	*pColor = RGB(r,g,b); 
	return;
}
	
long GetNextUniqueValueColor (LPLONG pNextColor,short VP)
{
	POINT	p=RectMid (&pViewports[VP-1]->DrawRect);   //pViewports[17]
	long	LastColor = *pNextColor;
	long	Color, SaveColor,ii;
	
	SaveDC (pViewports[VP-1]->hDC);
    SetDisplayMode (pViewports[VP-1]->hDC, GF_TEXTMODE);
  	SelectClipRgn (pViewports[VP-1]->hDC,NULL);
	SaveColor = GetPixel (pViewports[VP-1]->hDC,p.x,p.y);
	GetNextBlack (pNextColor);
	if ((long)SetPixel (pViewports[VP-1]->hDC,p.x,p.y,*pNextColor) < 0) 
	{
		if (SaveColor > -1)
			SetPixel (pViewports[VP-1]->hDC,p.x,p.y,SaveColor);
		RestoreDC (pViewports[VP-1]->hDC,-1);
		return *pNextColor;
	}
	while ((Color=GetPixel (pViewports[VP-1]->hDC,p.x,p.y)) != *pNextColor && *pNextColor > LastColor)
	{
		LastColor = *pNextColor;
		GetNextBlack (pNextColor); 
		ii=SetPixel (pViewports[VP-1]->hDC,p.x,p.y,*pNextColor);
	}
	if (SaveColor > -1)
		SetPixel (pViewports[VP-1]->hDC,p.x,p.y,SaveColor);
	RestoreDC (pViewports[VP-1]->hDC,-1);
	return *pNextColor;  
}

short ThemeSetChar (int Type, long iref, int desc, LPSTR TAG, LPSTR UDI)
#if ENABLETRACE
{GSSiEnterProg (1262);
#endif
{
// Returns -1 if element not processed by this theme
//          0 if element should not be displayed (missing data and theme missopt set to do not display)
//			1 if characteristics set by this theme   
//			2 if processed but graphics char not set (Set Color false)
	long MktVal;   
	static	long	LastDSRef; 
	static	double	StartDSDist, CurDSLength;
	int	 iclass, ClassNo, ClassNo2;
	double	MktValD, MidPointAZ;
	double	ValD, Rem;
	short	status;
	MIDPOINT	MidPoint; 
	LPMIDPOINT	pMidPoint;
	double	AZ;  
	LPSHORT	pInt;   
	POINT	Point;
	NETLINKSKEY	NetLinksKey; 
	NETLINKSDATA	NetLinksData;
	NETMARKERSTHEMEKEY	NetMarkersThemeKey1, NetMarkersThemeKey2;
	double		LastEndMP, NextEndMP, EndMP, ClassMin, ClassMax;
	short		st, st1, st2, Case, ii, ipos, iposinc, lnKey;
	static		ULONG		iposswitch=0;
	BOOL		rc, InClass; 
	BYTE		PatByt;
	PATBYTE		PatByte;
	LPPROFILETHEMEDATA	lpProfileData;  
	double		TrueValD; 
	BOOL		GetFirstClass=TRUE;
    
    if (!CurTheme)
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
    	return (-1);
}
	if (CurTheme->ID == PF_COORD_DISPLAY || CurTheme->ID == PF_BOUNDS_DISPLAY ||  CurTheme->ID == GF_CACHE_DISPLAY_THEME ||
		!CurTheme->IsActive|| !CurTheme->VPDisplayed)
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
			return (-1);
}
    
    HaltReport = FALSE;
	CreateThemePens (CurTheme,FALSE); 
	ThemeDisplayPass = 1;
	switch (CurTheme->ID)    
	{   
		case GF_PROFILE_THEME: 
			lpProfileData = (LPPROFILETHEMEDATA)&CurTheme->ClassBM; 
			if (Type == GF_LINE || Type == GF_POLYLINE || Type == GF_CURVE)
			{   
				HANDLE	hHLT=GSSiGlobAlloc (1325,GMEM_MOVEABLE,sizeof(HIGHLIGHTDATA));
				LPHIGHLIGHTDATA	pHighlightData=(LPHIGHLIGHTDATA)GlobalLock (hHLT);
				
		    	if (!BT_FIND (hHighlight,(LPSTR)&iref,BT_FIRST,BT_EQ,(LPSTR)pHighlightData))
		    	{
		    		lpProfileData->MinSeq = min (lpProfileData->MinSeq,pHighlightData->Sequence);
		    		lpProfileData->MaxSeq = max (lpProfileData->MaxSeq,pHighlightData->Sequence);
		    	}
		    	GSSiGlobUlFree (&hHLT);
		    }
			break;
			
		case GF_DYNAMIC_SEG_THEME:
			GSSiGlobFree (&hDynamicSeg); 
			if (PolyIsHiPrecis && (Type == GF_POLYLINE || Type == GF_CURVE)) 
			{
				double		SegDist;
				if (CurTheme->SymNum == -1)
				{
					if (_fstricmp (TAG,&CurTheme->Contents[1])) 
					{
						LastDSRef = LONG_MAX;
						goto RtnNotProcessed;
					}
	
				}
				else if (CurTheme->hVisList)
				{   
					LPVISLIST	SaveVis=CurVis; 
					BOOL		WantDesc; 
					
					CurVis = (LPVISLIST)GlobalLock (CurTheme->hVisList);
					WantDesc = GetVisibility (desc);
					GlobalUnlock (CurTheme->hVisList);
					CurVis = SaveVis;
					if (!WantDesc)
					{
						LastDSRef = LONG_MAX;
						goto RtnNotProcessed;
					}
				}
				if (iref != LastDSRef)
				{ 
					StartDSDist = 0;
					CurDSLength = GetPolyLengthD (lpDCurPoints,nCurPoints);
				}
				if (StartDSDist >= CurDSLength)
				{
					LastDSRef = LONG_MAX;
					goto RtnNotProcessed;
				} 
				SegDist = max (BaseDistToWinDist*2,CurTheme->RoundTo);
				hDynamicSeg = GetPolyBetweenDist (lpDCurPoints,nCurPoints,StartDSDist,StartDSDist+SegDist,&nDynamicCoord,FALSE,FALSE);
				StartDSDist += SegDist;
				LastDSRef = iref;
			}
			else
				LastDSRef = LONG_MAX;
			goto RtnNotProcessed;
		
		case GF_STREET_ADDRESS_THEME:
		{   
			
			if (!CurTheme->hScatterFile)
				goto RtnNotProcessed;
			pInt = (LPSHORT)GlobalLock (CurTheme->hScatterFile);
			if ((long)(*pInt+2) * sizeof(long) < UINT_MAX)
			{	
				int	n; 
				LPLONG	pRef;
					
				n = *pInt;
				(*pInt)++;
				pInt++;
				pRef = (LPLONG)pInt;
				pRef += n; 
				*pRef = CurrentRefno;
				GlobalUnlock (CurTheme->hScatterFile);
			} 
			else
			{
				GSSiGlobUlFree (&CurTheme->hScatterFile);
				goto RtnNotProcessed;
			}
		}
		goto RtnNotProcessed;
		
		case GF_POLYINFO_THEME:
			PolyInfoTheme = 0;  
			if (CurrentType == GF_AREA)
			{   
				DPOINT	IntPoint;
				
				if (HiPrecis) 
					if ((PolyInfoTheme=PolyCrossesItself (1,nPnts,lpDCurPoints,&IntPoint)))
					{ 
						POINT	PointLoc=BasePtToWinPt (&IntPoint);
//                    if (HaveLinkLines (lpDCurPoints,nPnts)) 
                    	DisplayPointItem (CurView->hDC,PointLoc,15*DeviceToScreenFactor,0,InvisiblePointSymbol,NULL);
                    }
			}
			goto RtnNotProcessed;
			
		case GF_BOUNDS_DISPLAY_THEME:
		case GF_TRANSFORM_THEME:
			goto RtnNotProcessed;
			
		case GF_STREET_TEXT_THEME:
		{   
			mnmxCor MinMax;    
			short	Piece=0,ii;
		    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurTheme->ClassBM;
		    static long	DebugRef=222115; 
		    long	MaxBounds;  
		    BOOL	ColorSet=FALSE; 
		    double	WantDist=GetGlobalDVal2 ("[%MINDISTBETWEENNAMES]",0)/2;
			
			if (CurTheme->SymNum == -1)
			{
				if (_fstricmp (TAG,&CurTheme->Contents[1])) 
					goto RtnNotProcessed;

			}
			else if (CurTheme->hVisList)
			{   
				LPVISLIST	SaveVis=CurVis; 
				BOOL		WantDesc;
				
				CurVis = (LPVISLIST)GlobalLock (CurTheme->hVisList);
				WantDesc = GetVisibility (desc);
				GlobalUnlock (CurTheme->hVisList);
				CurVis = SaveVis;
				if (!WantDesc)
					goto RtnNotProcessed;
			} 
			if (iref == DebugRef)
				ii=1;
			if (FileNum >= MaxTextLayer || LayerID != pStreetData->LayerID || !CurTheme->hScatterFile || 
				/*CurrentType == GF_AREA || 07/11/2006 for lakemaster names on lakes*/ 
				(!pStreetData->NameSource && !pStreetData->MinSize && iref%TextFreq))
				goto RtnNotProcessed;  
			if (TextFreq < 2 && pStreetData->AllowHollow)
					StreetCenterline = UseHollowStreets;
			if (*pStreetData->VisMacro)
			{
				HANDLE	hMem=GSSiGlobAlloc (1326,GMEM_MOVEABLE,512);
				LPSTR	pMem=GlobalLock (hMem);
				
				_fstrcpy (pMem,pStreetData->VisMacro);
				ExpandText (pMem);
				rc = atob (pMem);
				GSSiGlobUlFree (&hMem);
				if (!rc)
					goto RtnNoDisplay;
			} 
			if (pStreetData->NameSource && (CurrentType == GF_POINT || CurrentType == GF_AREA))
				pStreetData->IgnoreShields = TRUE;
			if (*pStreetData->ColorMacro)
			{
				HANDLE	hMem=GSSiGlobAlloc (1327,GMEM_MOVEABLE,512);
				LPSTR	pMem=GlobalLock (hMem);
				COLORREF	Color;
				short	R,G,B,W;
				
				_fstrcpy (pMem,pStreetData->ColorMacro);
				ExpandText (pMem);  
				if (*pMem)
				{
					if (sscanf (pMem,"%ld %i",&Color,&W) == 1)
						W = 1;
					R = GetRValue(Color);
					G = GetGValue(Color);
					B = GetBValue(Color);
					CurTheme->ClassColor[0] = RGBW(R,G,B,W); 
					CurTheme->NumDesiredClass = 1;  
					CurTheme->DataType = 2;   
					CurTheme->NotSetColor = 0; 
					SelectObject (CurView->hDC,GetStockObject(NULL_BRUSH));
					SelectObject (CurView->hDC,GetStockObject(NULL_PEN));
					CreateThemePens (CurTheme,TRUE); 
					SetThemeElementCharacteristics (0);  
					ColorSet = TRUE; 
				}
				GSSiGlobUlFree (&hMem); 
			} 
	   		if (!pStreetData->NameSource && !CurStreet && !*CurStreetNumbers)
	   		{
	   			if (ColorSet)
		   			goto RtnProcessed;
	   			else
					goto RtnNotProcessed; 
			}
			MaxBounds = max ((long)CurrentItemMinMax.xmx - (long)CurrentItemMinMax.xmn,(long)CurrentItemMinMax.ymx-(long)CurrentItemMinMax.ymn);
			if (MaxBounds * FileDistToWinDist < pStreetData->MinSize)
	   		{    
	   			mnmxCor	CurrentItemMinMax2=CurrentItemMinMax;
	   			if (ColorSet)
		   			goto RtnProcessed;
	   			else
					goto RtnNotProcessed; 
			}
			if (*pStreetData->DisplayNameMacro)
			{
				HANDLE	hMem=GSSiGlobAlloc (1328,GMEM_MOVEABLE,512);
				LPSTR	pMem=GlobalLock (hMem);
				
				_fstrcpy (pMem,pStreetData->DisplayNameMacro);
				ExpandText (pMem);
				rc = atob (pMem);
				GSSiGlobUlFree (&hMem);
				if (!rc)
		   		{
		   			if (ColorSet)
			   			goto RtnProcessed;
		   			else
						goto RtnNotProcessed; 
				}
			} 
			
			while ((!StreetCenterline || pStreetData->NameSource) && CurrentMidPoint (Type,&MidPoint.AZ,&MidPoint.Length,&MidPoint.height,TRUE, &MinMax,&Point,&Piece,1,&WantDist))
			{   
            	ipos = 0;  
            	if (Piece == 1)
            		WantDist *= 2;   
				if (pStreetData->ShowAllElements && !pStreetData->NameSource && !CurStreet)
				{
					while (ipos < 4 && CurStreetNumbers[ipos])
						ipos++; 
					ipos = iposswitch % ipos;
					iposinc = -1;
				}
				else if (PrimeNameOnly)
				{
					for (ipos = 3;ipos;ipos--)
						if (ipos < 0)
							break;
				}
				else 
				{
	            	iposinc = 1;
	            }
	            iposswitch++; 
   NextSNUM: 
	   			if (pStreetData->NameSource && !ipos)
	   			{   
	   				long	NameID; 
	   				LPSTR	pSlash, pValue;
					short iname = 0; 
	   				
	   			//	ipos = -1;
			    	_fstrcpy (Value,pStreetData->Expression);
					ExpandTextDataNotFound=FALSE;
			    	ExpandText (Value);
			    	if (!CurTheme->hThemeDB || ExpandTextDataNotFound)
			    	{
			    		if (CurTheme->MissOpt)
			    			goto RtnNoDisplay; 
			    		else
				   		{
				   			if (ColorSet)
					   			goto RtnProcessed;
				   			else
								goto RtnNotProcessed; 
						}
					}
					if (!*Value)
			   		{
			   			if (ColorSet)
				   			goto RtnProcessed;
			   			else
							goto RtnNotProcessed; 
					}
//					GetCharFieldData (CurTheme->hThemeDB,&CurTheme->Statement,
//									  &CurTheme->Field[0],iref,0,CurTheme->Value,Value,&status);
//					if (status || !*Value)
//						goto RtnNotProcessed;
					_fmemset (&CurStreetNumbers[1],0,16);  
					pValue = Value; 
					iname = 0;
					if ((pSlash = _fstrchr (pValue,'/'))) 
					{
						if (pSlash != pValue && _fstrncmp (pSlash-1,"1/2",3))
						{
							*pSlash++ = 0;
							Truncate (pValue);   
							pSlash = FirstNonBlank (pSlash); 
							iname = 1;
						} 
						else
							pSlash = 0;
					} 
					_fstrcpy (KeyVal,pValue);
					{
			NextName:
						PadString (KeyVal,0,GetBTKeyLen(pStreetData->hNameFile1));
						if (BT_FIND (pStreetData->hNameFile1,(LPSTR)KeyVal,BT_FIRST,BT_EQ,(LPSTR)&NameID))
						{
			   				HANDLE	hName2=GSSiGlobAlloc (1329,GMEM_MOVEABLE,256);
			   				LPSTR	pName2 = GlobalLock (hName2);
			   				
							if (BT_FIND (pStreetData->hNameFile2,(LPSTR)&NameID,BT_LAST,BT_ANY,pName2))
								NameID=0;  
							GSSiGlobUlFree (&hName2);
							NameID++;
							BT_PUT (pStreetData->hNameFile1,(LPSTR)KeyVal,(LPSTR)&NameID);
							BT_PUT (pStreetData->hNameFile2,(LPSTR)&NameID,(LPSTR)KeyVal); 
						}
		   				MidPoint.ref = NameID;    
						CurStreetNumbers[iname] = NameID;  
						if (pSlash)
						{
							_fstrcpy (KeyVal,pSlash);
							pSlash = 0;
							iname = 0;
							goto NextName;
						} 
					}
	   			}
	            else if (CurStreet) 
	            {
	            	ipos = -1;
	            	MidPoint.ref = CurStreet;  
	            }
	            else if (ipos < 0)
		   		{
		   			if (ColorSet)
			   			goto RtnProcessed;
		   			else
						goto RtnNotProcessed; 
				}
				else
	            {
					MidPoint.ref = CurStreetNumbers[ipos]; 
					if (MidPoint.ref < 0 && PrimeNameOnly)//pStreetData->PrimeNameOnly)
					{
						MidPoint.ref = labs (MidPoint.ref);
						ipos =-1;
					}
					else if (!MidPoint.ref)
			   		{
			   			if (ColorSet)
				   			goto RtnProcessed;
			   			else
							goto RtnNotProcessed; 
					}
					MidPoint.ref = labs (MidPoint.ref);  
				}	 
				MidPoint.Layer = FileNum;  
				MidPoint.State = CurState;  
				MidPoint.symnum = desc;
   				MidPoint.Refno = iref;
				if (PtInDrawRect (Point))
				{   
					if (ipos>0 && !ShieldsOnly)   
					{
						DPOINT	BasePt = dnewpt (WinPtToBasePt(Point),MidPoint.AZ,ipos * WinDistToWorldDist (25));
						MidPoint.Point= BasePtToWinPt (&BasePt); 
					}
					else
						MidPoint.Point=Point;
					MidPoint.width = MinMax.xmx - MinMax.xmn;
					pMidPoint = (LPMIDPOINT)GlobalLock (CurTheme->hScatterFile);
					if (CurTheme->NumMidpoint < MaxMidpoints)
					{	
						pMidPoint += CurTheme->NumMidpoint; 
						CurTheme->NumMidpoint++;
						*pMidPoint = MidPoint;
						GlobalUnlock (CurTheme->hScatterFile);
						AddToListFile (iref,pStreetData);  
					} 
					else
					{
						GSSiGlobUlFree (&CurTheme->hScatterFile);
				   		{
				   			if (ColorSet)
					   			goto RtnProcessed;
				   			else
								goto RtnNotProcessed; 
						}
					}
					if (!pStreetData->ShowAllElements && ipos >= 0 && !PrimeNameOnly)
					{
						ipos+=iposinc; 
						if (ipos >= 0 && ipos < 4)
							goto NextSNUM;
					}
				}
			}
	   		{
	   			if (ColorSet)
		   			goto RtnProcessed;
	   			else
					goto RtnNotProcessed; 
			}
		}
			break; 
			
		case GF_POINT_IN_AREA_THEME: 
			ii=1;         
		case GF_HOTSPOT_THEME:
		case GF_TIME_DISPLAY_THEME:	
		case GF_SINGLE_VALUE_THEME:
			if (CurTheme->SymNum == -1)
			{
				if (_fstricmp (TAG,&CurTheme->Contents[1])) 
					goto RtnNotProcessed;

			}
			else if (CurTheme->hVisList)
			{   
				LPVISLIST	SaveVis=CurVis; 
				BOOL		WantDesc;
				
				CurVis = (LPVISLIST)GlobalLock (CurTheme->hVisList);
				WantDesc = GetVisibility (desc);
				GlobalUnlock (CurTheme->hVisList);
				CurVis = SaveVis;
				if (!WantDesc)
					goto RtnNotProcessed;
			} 
			if (CurTheme->ID == GF_HOTSPOT_THEME) 
			{
				if (CurTheme->HotSpotData.PassThrough) 
					goto RtnNotProcessed;
				else 
					goto RtnNoDisplay;
            }
			switch (CurTheme->DataType)
			{
				case 0: //area
					switch (CurrentType)
					{
						case GF_TEXT:
						case GF_LINE:
						case GF_CURVE:
						case GF_POLYLINE:
							goto RtnNotProcessed;
						break;
					} 
					break;
				case 1: //point
					if (CurrentType != GF_POINT && CurrentType != GF_TEXT)
						goto RtnNotProcessed;
					break;
				case 2: //line
					if (CurrentType != GF_LINE && CurrentType != GF_POLYLINE && CurrentType != GF_CURVE)
						goto RtnNotProcessed;
					break;
				case 3: //text  
					if (CurrentType == GF_AREA)
						goto RtnNotProcessed;
					break;
			}
			if (CurTheme->ID == GF_POINT_IN_AREA_THEME && CurTheme->Pass == 0)
			{   
				
				if (!CurTheme->PointInAreaBrush)
				{
					short		ColorInc=1, maxattempts=256, iattempt;
					COLORREF	TestColor, SaveColor, GotColor;   
					POINT		pt=RectMid (&CurView->DrawRect);   
					int			r,g,b;
					
					SaveDC (CurView->hDC); 
				    SetMapMode    (CurView->hDC, MM_TEXT );
				    SetWindowOrgEx  ( CurView->hDC, 0,   0,0 );
				    SetViewportOrgEx( CurView->hDC, 0, 0,0 ); 
			        SelectClipRgn (CurView->hDC,0);
					SaveColor = GetPixel (CurView->hDC,pt.x,pt.y);
					if (CurView->BackGroundColor)
						ColorInc = -1;
					r=GetRValue (CurView->BackGroundColor);
					g=GetGValue (CurView->BackGroundColor);
					b=GetBValue (CurView->BackGroundColor);  
					r+=ColorInc;
					g+=ColorInc;
					b+=ColorInc;
					TestColor = RGB(r,g,b);  
					SetPixel (CurView->hDC,pt.x,pt.y,TestColor);
					while (iattempt++ < maxattempts && (GotColor=GetPixel (CurView->hDC,pt.x,pt.y)) == CurView->BackGroundColor)
					{ 
						r+=ColorInc;
						g+=ColorInc;
						b+=ColorInc;
						TestColor = RGB(r,g,b);  
						SetPixel (CurView->hDC,pt.x,pt.y,TestColor);
					} 
					GSSiDeleteObject (&CurTheme->PointInAreaBrush);
					GSSiDeleteObject (&CurTheme->PointInAreaPen);
					CurTheme->PointInAreaBrush = CreateSolidBrush (TestColor);
					CurTheme->PointInAreaPen = CreatePen (PS_SOLID,1,TestColor); 
					CurTheme->PointInAreaColor = TestColor; 
					RestoreDC (CurView->hDC,-1);     
				}
				if (CurrentType != GF_AREA || CurTheme->FidAreas == HFILE_ERROR)
					goto RtnNotProcessed; 
				BigWrite (CurTheme->FidAreas,(HPSTR)&iref,4,-1);
				BigWrite (CurTheme->FidAreas,(HPSTR)&nCurPoints,4,-1);
				BigWrite (CurTheme->FidAreas,(HPSTR)lpDCurPoints,nCurPoints * (long)sizeof(DPOINT),-1);  
				CurTheme->NumAreas++;
				SelectObject (CurView->hDC,CurTheme->PointInAreaBrush);
				//SelectObject (CurView->hDC,CurTheme->PointInAreaPen);  
				goto RtnProcessed;
			}
					
			SwitchThemeSHPFile ();		
			ValD = GetNumericFieldData (CurTheme->hThemeDB,&CurTheme->Statement,
										&CurTheme->Field,CurTheme->FieldFun,CurTheme->Value,iref,Type,&status);
			InClass=FALSE;
			if (status == 1 && CurTheme->MissOpt == 4)
			{
				status = 0;
				ValD = 0;
			}
			if (status==1 || (CurTheme->ZeroIsMissing && ValD == 0))
			{   
ProcessMissing: 
				CurTheme->NumMissing++;
				switch (CurTheme->MissOpt)
				{
					case 0:  
						if (CurTheme->NoDataBrush && !CurTheme->NotSetColor)
						{
							SetBkMode (CurView->hDC,OPAQUE);
							SetTextColor (CurView->hDC,0);
							SelectObject (CurView->hDC,CurTheme->NoDataBrush); 
							HaveVarFillColor = TRUE;  
							GlobalColors[0] = RGB(255,0,0);
						}
						else
							ii=1;
						if (DispersePoint (iref,-1,desc,&ValD) == 2)
							goto RtnNoDisplay; 
						else
							goto RtnProcessed;
						break;    
					case 1:
						goto RtnNoDisplay;
						break;
					case 2:
					case 3:
						if (DispersePoint (iref,-1,desc,&ValD) == 2)
							goto RtnNoDisplay; 
						else
							goto RtnNotProcessed;
						break;
				}
				goto RtnNotProcessed;
			}
			if (status==-1)
			{
InvalidSV1:     
				CurTheme->NumInvalid++;
				if (!CurTheme->MarkInvalid) 
					goto RtnNoDisplay;
				if (CurTheme->InvalidDataBrush)
					SelectObject (CurView->hDC,CurTheme->InvalidDataBrush);
				if (DispersePoint (iref,-2,desc,&ValD) == 2)
					goto RtnNoDisplay;
				else
					goto RtnProcessed;
			}   
			if (CurTheme->MissOpt==3)
				goto RtnNoDisplay;
			if (ValD && CurTheme->FieldCorrection)
			{   
				double	Area,Dist;
				
				switch (CurTheme->DataType)
				{
					case 0:
						if (HiPrecis)
							Area = ComputeProjectedAreaAreaD (lpDCurPoints,nCurPoints,&Dist); 
						else 
							Area = ComputeProjectedAreaArea (lpCurPoints,nCurPoints,&Dist);  
						Area = ConvertArea (Area,CurTheme->FieldCorrection); 
						if (!Area)
							goto InvalidSV1;
						ValD /= Area;
					break;
					case 1:
					break;
					case 2:
						if (HiPrecis)
							ComputeProjectedAreaAreaD (lpDCurPoints,-nCurPoints,&Dist); 
						else 
							ComputeProjectedAreaArea (lpCurPoints,-nCurPoints,&Dist);  
						Dist = ConvertDist (Dist,CurTheme->FieldCorrection); 
						if (!Dist)
							goto InvalidSV1;
						ValD /= Dist;
					break;
				}
			}
			TrueValD = ValD;
			ValD = Round (ValD,CurTheme->RoundTo);
			
			if (ValD > CurTheme->YLimit) ValD = CurTheme->YLimit;
			for (iclass=0;iclass<CurTheme->NumClass;iclass++)
			{	
				GetClassMinMax (iclass,&ClassMin,&ClassMax);
								
				if (ValD>=ClassMin && ValD<=ClassMax)
			    {   
			    	InClass=TRUE;
					if (!CurTheme->PCTByArea)
			    		CurTheme->ClassCount[iclass]++;
					if (DispersePoint (iref,iclass,desc,&ValD) == 2)
						goto RtnNoDisplay;  
					SetThemeElementCharacteristics (iclass);
			    	break;
			    }
			}
			if (CurTheme->ShowValue)
			{   
				float	MidPointAZ;
				float	Length;    
				mnmxCor	MinMax;
				DPOINT	BasePt;
				POINT	WinPoint;
				short	n=0, Height;
				
				if (CurrentMidPoint (Type,&MidPointAZ,&Length,&Height,TRUE, &MinMax,&WinPoint,&n,1,NULL)) 
				{   
					ShowVal.pTheme = CurTheme;
					ShowVal.Point = WinPoint; 
					ShowVal.Type = CurTheme->DataType;
					if (CurTheme->DataType == 2)	
						ShowVal.AZ = MidPointAZ;			
					else
						ShowVal.AZ = CurTheme->ShowValAZ; 
					SetShowValPoly (iref,FALSE);
					_fstrcpy (ShowVal.Text,ValueConv (TrueValD,CurTheme->ValConv,min(1,CurTheme->RoundTo),CurTheme->AddCommas));
				}
			}
			if (!InClass)
				goto InvalidSV1;
			goto RtnProcessed;
			break;

		case GF_SINGLE_NONNUM_VALUE_THEME:
			if (CurTheme->LayerID && (LayerID != CurTheme->LayerID))
				goto RtnNotProcessed;
			if (CurTheme->SymNum == -1)
			{
				if (_fstricmp (TAG,&CurTheme->Contents[1]))
				{
					if (CurTheme == ComputePCTTheme)
						goto RtnNoDisplay;
					else
						goto RtnNotProcessed;
				}
			}
			else if (CurTheme->hVisList)
			{   
				LPVISLIST	SaveVis=CurVis; 
				BOOL		WantDesc;
				
				CurVis = (LPVISLIST)GlobalLock (CurTheme->hVisList);
				WantDesc = GetVisibility (desc);
				GlobalUnlock (CurTheme->hVisList);
				CurVis = SaveVis;
				if (!WantDesc)
				{
					if (CurTheme == ComputePCTTheme)
						goto RtnNoDisplay;
					else
						goto RtnNotProcessed;
				}
			}
			if (CurTheme->UseFirstSymbol && !SymbolIsVisible(desc))
				goto RtnNotProcessed;
			switch (CurTheme->DataType)
			{
				case 0: //area
					switch (CurrentType)
					{
						case GF_TEXT:
						case GF_LINE:
						case GF_CURVE:
						case GF_POLYLINE:
							goto RtnNotProcessed;
						break;
					} 
					break;
				case 1: //point
					if (CurrentType != GF_POINT && CurrentType != GF_TEXT)
						goto RtnNotProcessed;
					break;
				case 2: //line
					if (CurrentType != GF_LINE && CurrentType != GF_POLYLINE && CurrentType != GF_CURVE)
						goto RtnNotProcessed;
					break;
				case 3: //text  
					if (CurrentType == GF_AREA)
						goto RtnNotProcessed;
					break;
			}

			lnKey = GetBTKeyLen(CurTheme->hScatterFile); 
			if (CurTheme->GetFirstClass && !StartAutoClassDef ())
			{  
				short MinClass=MAX_THEME_CLASSES+1;
NextValue:		
				SwitchThemeSHPFile ();		
				status = GetCharFieldData (CurTheme->hThemeDB,&CurTheme->Statement,
								  &CurTheme->Field,iref,CurTheme->FieldFun,CurTheme->Value,Value); 
CheckStatus:
				if (status)
				{   
					SetVarChangeTimes (1);
					if (MinClass > MAX_THEME_CLASSES)
						goto ProcessMissing;
					_fstrcpy (Value,MinClassValue);
					status = 0;
				}
				else
				{
					ClassNo = 0; 
					_fstrncpy (KeyVal,Value,lnKey);
					KeyVal[lnKey]=0;
        			if (!BT_FIND (CurTheme->hScatterFile,KeyVal,BT_FIRST,BT_EQ,(LPSTR)&ClassNo))
					{
						if (ClassNo < MinClass)   
						{
							_fstrcpy (MinClassValue,Value);
							MinClass = ClassNo;
						}
					}
					if (FetchDBRec (CurTheme->hThemeDB))
						goto NextValue;
					else
					{
						status = 1;
						goto CheckStatus;
					}
				}
            }
			else
			{		
				SwitchThemeSHPFile ();		
				status = GetCharFieldData (CurTheme->hThemeDB,&CurTheme->Statement,
								  &CurTheme->Field,iref,CurTheme->FieldFun,CurTheme->Value,Value);
			} 
			if (status==1)
				goto ProcessMissing; 
			
			if (status==-1)//dont think this can happen 7/21/04
			{   
				if (CurTheme->InvalidDataBrush)
					SelectObject (CurView->hDC,CurTheme->InvalidDataBrush);
				if (DispersePoint (iref,-2,desc,Value) == 2)
					goto RtnNoDisplay;
				else
					goto RtnProcessed;
			}
			if (CurTheme->ShowValue)
			{   
				float	MidPointAZ;
				float  Length; 
				mnmxCor	MinMax;  
				DPOINT	BasePt;
				POINT	WinPoint;
				short	n=0, Height;
				
				if (CurrentMidPoint (Type,&MidPointAZ,&Length,&Height,FALSE, &MinMax,&WinPoint,&n,3,NULL)) 
				{    
					ShowVal.pTheme = CurTheme;
					ShowVal.Point = WinPoint; 
					ShowVal.Type = CurTheme->DataType;
					if (CurTheme->DataType == 2)	
						ShowVal.AZ = MidPointAZ;			
					else
						ShowVal.AZ = CurTheme->ShowValAZ; 
					SetShowValPoly (iref,FALSE);
					_fstrcpy (ShowVal.Text,Value);
				}
			} 
			_fstrncpy (KeyVal,Value,lnKey);
			KeyVal[lnKey]=0;
			ClassNo = 0;
			if (!BT_FIND (CurTheme->hScatterFile,KeyVal,BT_FIRST,BT_EQ,(LPSTR)&ClassNo))
			{   
				if (!CurTheme->NumDesiredClass)
				{
					CurTheme->ValueColor = ClassNo;
					ClassNo = 1;
				}
				else if (!ClassNo || ClassNo > MAX_THEME_CLASSES || ClassNo > CurTheme->NumClass)
					goto InvalidSV1;
	GotClass:
				if (!CurTheme->PCTByArea && !CurTheme->UseStoredCounts)
		    		CurTheme->ClassCount[ClassNo-1]++;
		    	if (CurTheme->UseFirstSymbol && !CurTheme->ClassSymbol[ClassNo-1])
		    		CurTheme->ClassSymbol[ClassNo-1] = desc;
				SetThemeElementCharacteristics (ClassNo-1);
				if (DispersePoint (iref,ClassNo-1,desc,Value) == 2)
					goto RtnNoDisplay; 
			} 
			else if (StartAutoClassDef ())
			{   
				if (!CurTheme->NumDesiredClass)
				{
					ClassNo = GetNextUniqueValueColor (&CurTheme->NextValueColor,CurTheme->TargetViewport);
				}
				else if (CurTheme->NumClass < CurTheme->NumDesiredClass) 
				{   
					_fstrcpy (CurTheme->ClassBM[CurTheme->NumClass++],KeyVal);
					ClassNo = CurTheme->NumClass; 
				}
				else
				{
					ClassNo = CurTheme->NumDesiredClass;
					_fstrcpy (CurTheme->ClassBM[CurTheme->NumClass-1],"All other values");
				}
				BT_PUT (CurTheme->hScatterFile,KeyVal,(LPSTR)&ClassNo);
				CurTheme->ValueColor = ClassNo;   
				if (!CurTheme->NumDesiredClass)
					ClassNo = 1;
				goto GotClass;
			}
			else
			{
				_fstrncpy (VarVal,"[",lnKey);  
				st = BT_FIND (CurTheme->hScatterFile,VarVal,BT_FIRST,BT_GE,(LPSTR)&ClassNo2);
				while (!st)
				{
					if (*VarVal == '[')
					{   
						ExpandText (VarVal);
						if (!_fstrcmp (VarVal,Value))
						{
							ClassNo = ClassNo2;
							goto GotClass;
						}
					}
					else
						st = 1;
					st = BT_FIND (CurTheme->hScatterFile,VarVal,BT_NEXT,BT_ANY,(LPSTR)&ClassNo2);
				}
			}
			if (!ClassNo)
			{
				if (CurTheme->AllValueClass && CurTheme->AllValueClass < CurTheme->NumClass+1)
				{
					ClassNo = CurTheme->AllValueClass;
					goto GotClass;
				}
				if (CurTheme->SkipInvalid)
					goto RtnNoDisplay;
				goto ProcessMissing; 
			}
			CurTheme->NumVals++;
			if (CurTheme->MissOpt==3)
				goto RtnNoDisplay;
			goto RtnProcessed;
			break;    		

		case GF_TWO_VALUE_THEME:
			goto RtnNotProcessed;
			break;

		case GF_CRIME_THEME:
			goto RtnNotProcessed;
			break;

		case GF_DOCUMENTS_THEME: 
			if (CurTheme->SymNum == -1)
			{
				if (_fstricmp (TAG,&CurTheme->Contents[1]))
				{
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
					return (-1);
}
				}
			}
			else if (CurTheme->SymNum)
			{
				if (desc != CurTheme->SymNum)
					goto RtnNotProcessed;
			}
			if (HaveDesiredDocType (TAG, UDI))
			{
				SetLineHighlight(CurView->hDC,0);
				SetAreaHighlight(CurView->hDC);
				goto RtnProcessed;
			}
			else
				goto RtnNotProcessed;

		case GF_MOVE_POLY_THEME:
			MovePolyLine(CurTheme->ClassPnt[0]);
			break;

		case GF_SNAP_POLY_THEME:
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
			return (SnapPolyLine());
}
			break;

		case GF_SAVEPOLY_THEME:
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
			return (SavePolyLine(1));
}
			break;

		case GF_SAVEPOLYFILE_THEME:
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
			return (SavePolyLine(2));
}
			break;

		case GF_SAVEPOLYPARTS_THEME:
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
			return (SavePolyLine(3));
}
			break;

		case GF_UNDRAW_THEME:
			SelectObject (CurView->hDC,GetStockObject(WHITE_PEN));
			goto RtnProcessed;
			break; 
			
        case GF_CONTEST_THEME: 
        {
			ATDATA		AtData;
			
			if (!BT_FIND (hBTNetCon,(LPSTR)&iref,BT_FIRST,BT_EQ,(LPSTR)&AtData))
			{
				if (AtData.NextRef)
					SelectObject (CurView->hDC,CurTheme->ClassPen[0]); 
				else
					SelectObject (CurView->hDC,CurTheme->ClassPen[1]);
				goto RtnProcessed;
			} 
			goto RtnNotProcessed;
        }
        	break;
        	
		case GF_NETMARKER_THEME: 
		{   short	pos; 
			double	MinMP, MaxMP;
            
            if (CurrentType != GF_POLYLINE && CurrentType != GF_LINE) break; 
            if (!CurTheme->hScatterFile)
				goto RtnNotProcessed;
			NetLinksKey.Ref = iref;
			NetLinksKey.NetID = NetworkID;  
			NetLinksKey.Path = 0;  
			rc = -1;
			st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_FIRST,BT_GE,(LPSTR)&NetLinksData);
			while (!st && NetLinksKey.Ref == iref && NetLinksKey.NetID == NetworkID)
			{   
				rc = 1; 
				NetLinksData.Length = fabs (NetLinksData.Length); 
				MinMP = NetLinksData.MP;
				MaxMP = MinMP + NetLinksData.Length;
				CurStreet = NetLinksKey.Path;
				NetMarkersThemeKey2.Path = NetLinksKey.Path;
				NetMarkersThemeKey2.StartMP = NetLinksData.MP;
				st2 = BT_FIND (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey2,BT_FIRST,BT_GE,
								(LPSTR)&NextEndMP);
				if (st2)
					pos = BT_LAST;
				else
					pos = BT_PRIOR;
				st1 = BT_FIND (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey1,pos,BT_ANY,
								(LPSTR)&LastEndMP);
				if (NetMarkersThemeKey1.Path != NetLinksKey.Path) st1 = 1; 
				if (NetMarkersThemeKey2.Path != NetLinksKey.Path) st2 = 1;   
		SetCase:
				if (st1 && st2)
					Case = 1;
				else 
				{
					if (st1) 
					{ 
						if (fabs ((NetLinksData.MP + NetLinksData.Length)
							- NetMarkersThemeKey2.StartMP) <= NetTOL*2)
							Case = 3;
						else
							Case = 1;
					} 
					else if (NetMarkersThemeKey1.StartMP <= MinMP && LastEndMP >= MaxMP)
						Case = 5;
					else if (st2)
					{  
						if (fabs (NetLinksData.MP - LastEndMP) <= NetTOL*2)
							Case = 2;
						else
							Case = 1;
					
					}
					else
					{  
						if (fabs (NetLinksData.MP - LastEndMP) > NetTOL*2)
							st1 = 1;  
						if (fabs ((NetLinksData.MP + NetLinksData.Length)
							- NetMarkersThemeKey2.StartMP) > NetTOL*2) 
							st2 = 1;
						if (st1 || st2)
							goto SetCase; 
						else
							Case = 4;
					}
				}	
				NetMarkersThemeKey1.Path = NetLinksKey.Path;
				switch (Case)
				{
					case 1:	// insert without connection    
						NetMarkersThemeKey1.Path = NetLinksKey.Path;
						NetMarkersThemeKey1.StartMP = NetLinksData.MP;  
						EndMP = NetLinksData.MP + NetLinksData.Length;
						BT_PUT (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey1,(LPSTR)&EndMP);
						break;
					case 2:	// connects to prior seg
						EndMP = NetLinksData.MP + NetLinksData.Length;
						BT_PUT (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey1,(LPSTR)&EndMP);
						break;
					case 3:	// connects to next seg
						if (BT_DELETE (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey2,(LPSTR)&NextEndMP,FALSE))
							ii=1;
						NetMarkersThemeKey1.StartMP = NetLinksData.MP;
						BT_PUT (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey1,(LPSTR)&NextEndMP);
						break;
					case 4:	// links prior and next seg
						if (BT_DELETE (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey2,(LPSTR)&NextEndMP,FALSE))
							ii=1;
						BT_PUT (CurTheme->hScatterFile,(LPSTR)&NetMarkersThemeKey1,(LPSTR)&NextEndMP);
						break; 
					case 5:
						break;
				}
					
				st = BT_FIND (hBTNetLinks,(LPSTR)&NetLinksKey,BT_NEXT,BT_ANY,(LPSTR)&NetLinksData);
			}
		
		if (rc==1)
			SelectObject(CurView->hDC, CurTheme->ClassPen[0]);

		if (HaltReport)
		{
			CloseThemeDataFile(TRUE);
			CurTheme->IsActive = FALSE; 
		}
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
		return rc;
}
		}

	}

RtnNotProcessed:
	if (HaltReport)
	{
		CloseThemeDataFile(TRUE);
		CurTheme->IsActive = FALSE; 
	}
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
	return -1;
}
RtnNoDisplay:
	if (HaltReport)
	{
		CloseThemeDataFile(TRUE);
		CurTheme->IsActive = FALSE; 
	}
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
	return  0;
}
RtnProcessed:
	if (HaltReport)
	{
		CloseThemeDataFile(TRUE);
		CurTheme->IsActive = FALSE; 
	} 
	else if (CurTheme->NotSetColor)
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
		return	2;  
}
//	else if (!CurTheme->DataType)
//		HaveVarFillColor = TRUE; 
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
	return	1;  
}
	
RtnColorNotSet:
	if (HaltReport)
	{
		CloseThemeDataFile(TRUE);
		CurTheme->IsActive = FALSE; 
	} 
//	else if (!CurTheme->DataType)
//		HaveVarFillColor = TRUE; 
{
#if ENABLETRACE
GSSiExitProg (1262);
#endif
	return	1;  
}
	
#if ENABLETRACE
}
#endif
}  
void ShowValue (HDC hDC, BOOL Init)
#if ENABLETRACE
{GSSiEnterProg (1275);
#endif
{   
	float	sizex = ShowValSize;// * DeviceToScreenFactor;
	short	VJust=2, Weight=100;    
	static	BOOL	First=TRUE;   
	DWORD	TextExt;  
	COLORREF	OldColor,ShadowColor=RGB(255,255,255); 
	BOOL	Shadow=0; 
	HANDLE	hSaveFont=0; 
	LPSTR	pSaveFont,pBar=0;
	short	ii; 
	static	double	FontPixelsToSizeFactor=1;
	static	DPOINT	MaskPoints[4]={0,0,0,0};  
	DPOINT	TextPoints[4];    
	MNMXCORD	DrawBounds;
   	HANDLE	hAccelerator=0;
   	double	TtoAfac; 
   	char	str[64];
	double	SavePIASizeFactor = PIASizeFactor; 
	LPTHEME	SaveTheme=CurTheme;  
	BOOL	UseTestRect=TRUE;
	
	if (Init) 
	{
		if (First) 
		{
			ShowVal.hPoints = 0;   
			ShowVal.hPolyPartLen = 0;
		}
		else
			GSSiGlobFree (&ShowVal.hPoints); 
		First = FALSE;
		ShowVal.Text[0]=0;
{
#if ENABLETRACE
GSSiExitProg (1275);
#endif
		return;   
}
	}
	Truncate (ShowVal.Text); 
	if (!ShowVal.Text[0])
{
#if ENABLETRACE
GSSiExitProg (1275);
#endif
		return;   
}   
	ShowValRef = ShowVal.Refno;  
//	sprintf (str,"%ld - %f",ShowValRef,PIASizeFactor);
//	SetWindowText (hWndMain,str);
	if (ShowValRef == 111007437)
		ii=1;
	if (ShowVal.Text[0])
	{   
	    SaveDC (hDC);
	    SetDisplayMode (hDC, GF_TEXTMODE); 
		SetBkMode (hDC,TRANSPARENT);     
		CurTheme = ShowVal.pTheme;
	    if (*ShowVal.pTheme->ShowValueFont.lfFaceName)
	    { 
			DispText (0,0,0,0,0,0,0,0,0,0,0,0,NULL,-1,0,0,0,-1,0,NULL,0,NULL,0,0,0);
			hSaveFont = GSSiGlobAlloc (1330,GMEM_MOVEABLE,256);  
			pSaveFont = GlobalLock (hSaveFont);
			GetGlobalCVal ("[%LABELFONT]",pSaveFont,NULL);
			SetGlobalValue ("%LABELFONT",ShowVal.pTheme->ShowValueFont.lfFaceName);
			OldColor = SetTextColor (hDC,ConvertColor (ShowVal.pTheme->ShowValueTextColor,ShowVal.pTheme->UseHalfTone));  
			sizex = fabs (PixelsToTAGFontHt (&ShowVal.pTheme->ShowValueFont,3));// * DeviceToScreenFactor; 
			FontPixelsToSizeFactor = sizex/max(1,abs(ShowVal.pTheme->ShowValueFont.lfHeight));  
			Weight = ShowVal.pTheme->ShowValueFont.lfWeight;
			if (ShowVal.pTheme->ShowValueFont.lfUnderline)
				SetBkMode (hDC,OPAQUE); 
			Shadow = ShowVal.pTheme->ShowValueFont.lfStrikeOut; 
			if (Shadow)
				Shadow = 2;
		}
		else
			OldColor = SetTextColor (hDC,ConvertColor (ShowValColor,ShowVal.pTheme->UseHalfTone));
		switch (ShowVal.Type)
		{
			case 1:
			case 2:
				VJust = 1;  
/*		{
			RECT	rect;
		    SaveDC (hDC);
		    SetDisplayMode (hDC, GF_TEXTMODE);  
			rect.left = ShowVal.Point.x-3;
			rect.right = ShowVal.Point.x+3;
			rect.top = ShowVal.Point.y-3;
			rect.bottom = ShowVal.Point.y+3;
			FillRectPoly (hDC,&rect,0);
			RestoreDC (hDC,-1);
		} */
			DispText (hDC,ShowVal.Point.x,ShowVal.Point.x, ShowVal.Point.y,0, 2,VJust,
				  		sizex,1,Weight, FALSE,ShowVal.AZ,ShowVal.Text,0,UseTestRect,Shadow,ShadowColor,-1,0,NULL,0,NULL,ShowVal.pTheme->UseHalfTone,0,0);
			break;
			
			case 0:
			{   
				USHORT	twidth, theight,i;
				BOOL	pass=FALSE;
				double	PolyArea, TextArea, AreaAZ=100;
				float   tsize, mintsize=GetGlobalDVal2 ("[%MINTEXTSIZE]",0.06);//* DeviceToScreenFactor; 
				HPPOINT	pPoints; 
				HPDPOINT	pAreaPoints;
				DPOINT	MidPt;
				POINT	CenterPoint[7];
				float	PointMaxTSize[7];
				short	NumCenterPoints=-1;
				HANDLE	hAreaPoints; 
				MNMXCORD	AreaBounds;  
				LPSTR	txt;
				HANDLE	htxt; 
				BOOL	AllowTextRotation=GetGlobalBVal2 ("[%ALLOWTEXTROTATION]",TRUE);
				HANDLE	hMaskAccelerator=0;  
				long	MaskAreaRefno=0;  
				BOOL	DeleteMaskAccelerator=FALSE;
    
			    if (CurView->DisplayInParent && CurView->Parent)   
			    {
			    	if (pViewports[CurView->Parent-1]->hMaskArea) 
			    	{
			    		hMaskAccelerator = pViewports[CurView->Parent-1]->hMaskAccelerator[0];     
			    		MaskAreaRefno = pViewports[CurView->Parent-1]->MaskAreaRefno; 
			    	}
			    }
			    else if (CurView->hMaskArea) 
		    	{
		    		hMaskAccelerator = CurView->hMaskAccelerator[0];
		    		MaskAreaRefno = CurView->MaskAreaRefno;
		    	} 
		    	else
		    	{   
		    		/*TextPoints[0].x = CurView->DrawRect.left;
		    		TextPoints[0].y = CurView->DrawRect.bottom;
		    		TextPoints[1].x = CurView->DrawRect.left;
		    		TextPoints[1].y = CurView->DrawRect.top;
		    		TextPoints[2].x = CurView->DrawRect.right;
		    		TextPoints[2].y = CurView->DrawRect.top;
		    		TextPoints[3].x = CurView->DrawRect.right;
		    		TextPoints[3].y = CurView->DrawRect.bottom;*/ 
		    		/*if (MaskPoints[0].x != CurView->WBounds.xmn ||
		    			MaskPoints[0].y != CurView->WBounds.ymn ||
		    			MaskPoints[1].y != CurView->WBounds.ymx ||
		    			MaskPoints[2].x != CurView->WBounds.xmx)
		    			{
				    		MaskPoints[0].x = CurView->WBounds.xmn;
				    		MaskPoints[0].y = CurView->WBounds.ymn;
				    		MaskPoints[1].x = CurView->WBounds.xmn;
				    		MaskPoints[1].y = CurView->WBounds.ymx;
				    		MaskPoints[2].x = CurView->WBounds.xmx;
				    		MaskPoints[2].y = CurView->WBounds.ymx;
				    		MaskPoints[3].x = CurView->WBounds.xmx;
				    		MaskPoints[3].y = CurView->WBounds.ymn; */
		    		if (MaskPoints[0].x != CurView->DrawRect.left ||
		    			MaskPoints[0].y != CurView->DrawRect.top ||
		    			MaskPoints[1].y != CurView->DrawRect.bottom ||
		    			MaskPoints[2].x != CurView->DrawRect.right)
		    			{
				    		MaskPoints[0].x = CurView->DrawRect.left;
				    		MaskPoints[0].y = CurView->DrawRect.top;
				    		MaskPoints[1].x = CurView->DrawRect.left;
				    		MaskPoints[1].y = CurView->DrawRect.bottom;
				    		MaskPoints[2].x = CurView->DrawRect.right;
				    		MaskPoints[2].y = CurView->DrawRect.bottom;
				    		MaskPoints[3].x = CurView->DrawRect.right;
				    		MaskPoints[3].y = CurView->DrawRect.top;
							GSSiGlobFree (&hShowValMaskAccelerator); 
				      		hShowValMaskAccelerator = PointInAreaAcceleratorSetupWindow (4,MaskPoints,0); 
				      	} 
		      		hMaskAccelerator = hShowValMaskAccelerator;
			    } 
				
				if (*ShowValDB)
				{   
					BOOL	Found=FALSE;
					
					if (!hShowValDB)
					     hShowValDB = OpenGWDatabase (ShowValDB,BT_READ);
					if (hShowValDB)
					{
					 	LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock (hShowValDB);  
					 	long	Offset;
					 	LPSHOWVALDATA	pSVData=(LPSHOWVALDATA)lpGWDHead->GWDData;
					 	
					 	ShowValKey.MaskAreaRefno = MaskAreaRefno; 
					 	ShowValKey.AreaRefno = ShowVal.Refno;
					 	if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&ShowValKey,BT_FIRST,BT_EQ,(LPSTR)&Offset))
					 	{   
					 		POINT	Point;
					 		
					 		Found = TRUE;
					 		FillGWDData (lpGWDHead,Offset);  
					 		Point = BasePtToWinPt (&pSVData->Point);
					 		if (pSVData->Size)
					 		{  
					 			double	MaxPixels = BaseDistToWinDist * pSVData->Size/2;
					 			double	MaxSize = MaxPixels * FontPixelsToSizeFactor;
					 			
				 				sizex = min (sizex,MaxSize);
					 		}
					 		if (sizex > mintsize)
							DispText (hDC,Point.x,Point.x, Point.y,0, 2,VJust,
								  		sizex,1,Weight, FALSE,0,ShowVal.Text,0,UseTestRect,Shadow,ShadowColor,-1,0,0,0,NULL,ShowVal.pTheme->UseHalfTone,0,0);
					 	} 
					 	if (Found) 
					 	{
					 		GlobalUnlock (hShowValDB); 
					 		break;  
					 	}
					 	ShowValKey.MaskAreaRefno = 0; 
					 	ShowValKey.AreaRefno = ShowVal.Refno;
					 	if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&ShowValKey,BT_FIRST,BT_EQ,(LPSTR)&Offset))
					 	{   
					 		POINT	Point;
					 		
					 		Found = TRUE;
					 		FillGWDData (lpGWDHead,Offset);  
					 		Point = BasePtToWinPt (&pSVData->Point);
							DispText (hDC,Point.x,Point.x, Point.y,0, 2,VJust,
								  		sizex,1,Weight, FALSE,0,ShowVal.Text,0,UseTestRect,Shadow,ShadowColor,-1,0,0,0,NULL,ShowVal.pTheme->UseHalfTone,0,0);
					 	}
					 	GlobalUnlock (hShowValDB); 
					 	if (Found)
					 		break;
					}
				}
				if (!ShowVal.hPoints)
					break;  
				mintsize = max (mintsize,0.001);
				htxt = GSSiGlobAlloc (1331,GMEM_MOVEABLE,1024);
				txt = GlobalLock (htxt);
				_fstrcpy (txt,ShowVal.Text);
				pPoints = (HPPOINT)GlobalLock (ShowVal.hPoints);
				hAreaPoints = GSSiGlobAlloc (1332,GMEM_MOVEABLE,(long)nPnts*sizeof(DPOINT));
				pAreaPoints=(HPDPOINT)GlobalLock (hAreaPoints); 
				DBoundsInit (&AreaBounds);
				for (i=0;i<nPnts;i++)
				{
					pAreaPoints[i].x = pPoints[i].x;
					pAreaPoints[i].y = pPoints[i].y;  
					AddDPointToMinMax (&pAreaPoints[i],&AreaBounds);
				}
			    GSSiGlobUlFree (&ShowVal.hPoints);
			    RectToBounds (&CurView->DrawRect,&DrawBounds);  
			    if (!IntersectBounds (&AreaBounds,&DrawBounds,&AreaBounds))
				{
					GlobalUnlock (hAreaPoints);
			    	goto Exit;    
			    }
			    MidPt = MinMaxMidPointD (&AreaBounds);
				PolyArea = fabs (ComputeAreaAreaD (pAreaPoints,ShowVal.nPnts,NULL)); 
				tsize = sizex;
				TextExt = DispText (hDC,(int)MidPt.x,(int)MidPt.x, -1, 0, 2,VJust,
					  		tsize,1,Weight, FALSE,ShowVal.AZ,txt,0,UseTestRect,Shadow,ShadowColor,-1,0,NULL,0,NULL,ShowVal.pTheme->UseHalfTone,0,0);
				twidth = LOWORD(TextExt)+1; 
				theight = HIWORD(TextExt)+1;     
				TextArea = (double)twidth * (double)theight * 1.1; 
				if (PolyArea)
					TtoAfac  =  TextArea / PolyArea; 
				else
					TtoAfac = 1;
			    if (TtoAfac > 1)
			    	tsize /= TtoAfac;
				{
					BOOL	TextInView=TRUE;
					PIASizeFactor = GetGlobalDVal2 ("[%SVAFACTOR]",0.5);   

					if (tsize > mintsize && POINT_IN_AREAD (MidPt, nPnts,pAreaPoints,NULL,0))  
					{
						if (DispText (hDC,(int)MidPt.x,(int)MidPt.x, (int)MidPt.y,0, 2,VJust,
						  		tsize,1,Weight, FALSE,0,txt,0,UseTestRect,Shadow,ShadowColor,-1,nPnts,hAreaPoints,ShowVal.nPoly,ShowVal.hPolyPartLen,ShowVal.pTheme->UseHalfTone,pBar,0)) 
						{
							GlobalUnlock (hAreaPoints);
					    	goto Exit;    
					    } 
					}
					GSSiGlobFree (&hAccelerator); 
		      		hAccelerator = PointInAreaAcceleratorSetupWindow (nPnts,pAreaPoints,hMaskAccelerator);
					if (hAccelerator && hMaskAccelerator)
					{
						LPPIAAStruct pPIAA=(LPPIAAStruct)GlobalLock (hAccelerator);  
						
						AreaBounds = pPIAA->InBounds; 
						if ((TextInView = pPIAA->HaveInPoints))
						{
							POINT PIAAPoint = PIAACenter (pPIAA,NULL,NULL,FALSE);
							DPOINT	PIAAPointD = PIAAPointToDPoint (PIAAPoint,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
							
							//SetPixel (hDC,PIAAPointD.x,PIAAPointD.y,0);
							ShowVal.Point = DPointToPoint (PIAAPointD);
						}
						GlobalUnlock (hAccelerator); 
                    }
					GlobalUnlock (hAreaPoints);
					if (!TextInView)
						goto Exit; 
				    MidPt = MinMaxMidPointD (&AreaBounds);
				    BoundsToPoints (&AreaBounds,TextPoints,NULL);
			Next:  
					tsize = sizex;
				    if (PolyArea && TextArea)
				    {   
				    	float	OrigTSize = tsize;
				    	short	ip;
				    	
					    if (TtoAfac > 1)
					    	tsize /= TtoAfac;
					    while (tsize >= mintsize)
						{   
							if (NumCenterPoints < 0)
							{   
								DPOINT TestPoint = PointToDPoint (ShowVal.Point);
								double	factor = 2.0;
				                
				                NumCenterPoints = 0;
								pAreaPoints=(HPDPOINT)GlobalLock (hAreaPoints);    
								if (POINT_IN_AREAD (TestPoint, nPnts,pAreaPoints,NULL,&hAccelerator)) 
								{
									PointMaxTSize[NumCenterPoints] = OrigTSize;
									CenterPoint[NumCenterPoints++] = DPointToPoint (TestPoint);
								}
								if (POINT_IN_AREAD (MidPt, nPnts,pAreaPoints,NULL,&hAccelerator)) 
								{
									PointMaxTSize[NumCenterPoints] = OrigTSize;
									CenterPoint[NumCenterPoints++] = DPointToPoint (MidPt);
								}
								for (ip=0;ip < 4; ip++)
								{    
									BOOL	st;
									
							      	TestPoint = MidPointD (MidPt,TextPoints[ip]);
									if (hAccelerator)
							      		st = (PointInAreaAccelerator (&TestPoint,hAccelerator) == 1);
							      	else
										st = POINT_IN_AREAD (TestPoint, nPnts,pAreaPoints,NULL,NULL);
									if (st)
									{   
										if (TtoAfac * factor > 1)
											PointMaxTSize[NumCenterPoints] = OrigTSize / (TtoAfac * factor);
										else
											PointMaxTSize[NumCenterPoints] = OrigTSize;
										CenterPoint[NumCenterPoints++] = DPointToPoint (TestPoint);
									}
								}
								GlobalUnlock (hAreaPoints); 
							}
							for (ip=0;ip<NumCenterPoints;ip++)
							{		    	
								//if (tsize <= PointMaxTSize[ip])
								{    
									//SetPixel (hDC,CenterPoint[ip].x,CenterPoint[ip].y,0);
									if (DispText (hDC,CenterPoint[ip].x,CenterPoint[ip].x, CenterPoint[ip].y,0, 2,VJust,
									  		tsize,1,Weight, FALSE,0,txt,0,UseTestRect,Shadow,ShadowColor,-1,nPnts,hAreaPoints,ShowVal.nPoly,ShowVal.hPolyPartLen,ShowVal.pTheme->UseHalfTone,pBar,0)) 
										goto Exit;
									if (AllowTextRotation)
									{
										if (AreaAZ > TWOPI) 
										{
											HANDLE	hInPoints=GSSiGlobAlloc (0,GMEM_MOVEABLE,((long)nPnts)*sizeof(DPOINT));
											HPDPOINT	InPoints=(HPDPOINT)GlobalLock (hInPoints);
											HPDPOINT	AreaPoint = (HPDPOINT)GlobalLock (hAreaPoints);
											DWORD	j;
											long	nInPnts=0;	
											
											for (j=0;j<nPnts;j++)
												if (PointInAreaAccelerator (&AreaPoint[j],hMaskAccelerator))	 
													InPoints[nInPnts++] = AreaPoint[j];
											if (nInPnts > 2)
												AreaAZ = GetAreaAZ (nInPnts,hInPoints);
											else
												AreaAZ = 0;
											GSSiGlobUlFree (&hInPoints); 
											GlobalUnlock (hAreaPoints);
										} 
										if (AreaAZ) 
										{
											if (DispText (hDC,CenterPoint[ip].x,CenterPoint[ip].x, CenterPoint[ip].y,0, 2,VJust,
												  		tsize,1,Weight, FALSE,AreaAZ,txt,0,UseTestRect,Shadow,ShadowColor,-1,nPnts,hAreaPoints,ShowVal.nPoly,ShowVal.hPolyPartLen,ShowVal.pTheme->UseHalfTone,pBar,0)) 
												goto Exit;
										}
									}
								}
							} 
							tsize -= 0.01;
						}
					} 
					if (!pass)
					{   
						pass = TRUE;
						//mintsize /= 2;
						if (GetGlobalCVal ("[%MINTEXT]",txt,NULL)) 
						{
							if ((pBar = _fstrchr (txt,'|')))
								*pBar++ = 0; 
							else
								ExpandText (txt); 
							goto Next;
						}  
					}
					else if (hAccelerator)
					{
						LPPIAAStruct pPIAA=(LPPIAAStruct)GlobalLock (hAccelerator);  
						POINT PIAAPoint = PIAACenter (pPIAA,NULL,NULL,FALSE);
						DPOINT	PIAAPointD = PIAAPointToDPoint (PIAAPoint,&pPIAA->Bounds,&pPIAA->Factor,pPIAA->Offset,pPIAA->Type); 
								
						ShowVal.Point = DPointToPoint (PIAAPointD);  
						if (PtInRect (&CurView->DrawRect,ShowVal.Point))
						{
							if (pBar)
							{
								if (GetGlobalCVal ("[%MINTEXT]",txt,NULL)) 
								{
									if ((pBar = _fstrchr (txt,'|')))  
									{
										txt = pBar+1;
										ExpandText (txt);
									}
								}
							}
							DispText (hDC,ShowVal.Point.x,ShowVal.Point.x, ShowVal.Point.y,0, 2,VJust,
							  		tsize,1,Weight, FALSE,0,txt,0,UseTestRect,Shadow,ShadowColor,-1,nPnts,hAreaPoints,ShowVal.nPoly,ShowVal.hPolyPartLen,ShowVal.pTheme->UseHalfTone,0,0); 
						}
						GlobalUnlock (hAccelerator); 
	                }
 
		Exit:   
					GSSiGlobFree (&hAccelerator); 
					PIASizeFactor = SavePIASizeFactor;
				}
				GSSiGlobUlFree (&htxt);
				GSSiGlobFree (&hAreaPoints);
			}	
			break;
		} 
		SetTextColor (hDC,OldColor);
		RestoreDC (hDC,-1);
	}
	ShowVal.Text[0]=0;
	if (hSaveFont)
	{
		SetGlobalValue ("%LABELFONT",pSaveFont);
		GSSiGlobUlFree (&hSaveFont);
		DispText (0,0,0,0,0,0,0,0,0,0,0,0,NULL,-1,0,0,0,-1,0,NULL,0,NULL,0,0,0); 
	} 
	CurTheme = SaveTheme; 
//	sprintf (str,"Return %f",PIASizeFactor);
//	SetWindowText (hWndMain,str);
{
#if ENABLETRACE
GSSiExitProg (1275);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

BOOL CacheDisplayTheme (short from)
#if ENABLETRACE
{GSSiEnterProg (1332);
#endif
{   
	LPVIEWPORT	SaveVP=CurView;
	HANDLE	hMem = GSSiGlobAlloc (1333,GMEM_MOVEABLE,1024);
	LPSTR	CacheFile=GlobalLock (hMem), SaveScreenFile=CacheFile+256; 
	LPOFSTRUCT	pOFStruct=(LPOFSTRUCT) (SaveScreenFile + 256); 
	LPSAVESCREEN	pSaveScreen; 
	HANDLE	hBitmap; 
	BOOL	rtn=FALSE;  
	DWORD	Err;
	
	if (Printing)
		goto Exit;
	switch (from)
	{   
		default: 
			goto Exit;
		case 2:
		case 0: 
			SetViewport (CurTheme->TargetViewport);
			if (!CurView->CurZoomAreaRef || !*CurView->CurVisibilityID || CurView->DisplayedFullScreen)
				break;
			GetTempDir (SaveScreenFile);  
			_fstrcat (SaveScreenFile,"\\gmsavesc");
			ExpandText (SaveScreenFile);
			GSSiMakeDir (SaveScreenFile,&Err);
			sprintf (_fstrchr(SaveScreenFile,0),"\\%lx\\%s.scr",CurView->CurZoomAreaRef,CurView->CurVisibilityID); 
			if (from == 0)
			{
		        if (GetCacheFile (CacheFile, SaveScreenFile,FALSE, 0))
		        {   
		        	long	UpdateTime;
		        	
		        	if ((hBitmap = ReadSavedScreen (CacheFile, &UpdateTime)))
		        	{   
		        		double	dtime = difftime (UpdateTime,OpenConfigStat.st_mtime); 
                        
		        		pSaveScreen = (LPSAVESCREEN)GlobalLock (hBitmap);
		    			pSaveScreen->pVP = CurView; 
		    			CurView->BitmapID = pSaveScreen->ID;
		        		if (dtime < 0 ||
		        			CurView->Rect.left != pSaveScreen->Rect.left ||
		        			CurView->Rect.right != pSaveScreen->Rect.right ||
		        			CurView->Rect.top != pSaveScreen->Rect.top ||
		        			CurView->Rect.bottom != pSaveScreen->Rect.bottom)
		        		{
		        			GlobalUnlock (hBitmap);
		        			DestroySavedScreen (&hBitmap,CurView->BitmapID); 
		        			RemoveCacheFile (SaveScreenFile);
		        		}
		        		else
		        		{
		        			GlobalUnlock (hBitmap);
	        				RestoreScreen2 (CurView->hDC,hBitmap,CurView->BitmapID,FALSE);
		        			DestroySavedScreen (&hBitmap,CurView->BitmapID); 
	   				        CurView->PassID = 99; 
		        		}
		        	}
		        }
			} 
			else
			{   
				hBitmap = SaveScreen2 (CurView->hDC, CurView->Rect,CurView,&CurView->BitmapID);
		        WriteSavedScreen (SaveScreenFile,hBitmap);
		    	GetCacheFile (CacheFile, SaveScreenFile,TRUE, 0); 
		    	GSSiRemove (SaveScreenFile); 
		    } 
		    rtn = TRUE;
		break;
	} 
Exit:
	GSSiGlobUlFree (&hMem);
	SetCurView (SaveVP);
{
#if ENABLETRACE
GSSiExitProg (1332);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}


POINT AddjustPointLoc (POINT Point,float Size,short Sequence)
#if ENABLETRACE
{GSSiEnterProg (1331);
#endif
{   
	short	xmove, ymove, shift=0; 
	float	shiftdist;
	
	if (Sequence <= 1)
{
#if ENABLETRACE
GSSiExitProg (1331);
#endif
		return Point;
}
Top:
	if (Sequence == 1)
	{
		xmove = 0;
		ymove = 0;
	}
	else if (Sequence < 4)
	{  
		xmove = -1;
		ymove = Sequence - 2;
	}	
	else if (Sequence < 6)
	{  
		ymove = 1;
		xmove = Sequence - 4;
	}	
	else if (Sequence < 8)
	{  
		xmove = 1;
		ymove = -(Sequence - 6);
	}	
	else if (Sequence < 10)
	{  
		ymove = -1;
		xmove = -(Sequence - 8);
	}	
	else if (Sequence < 14)
	{  
		xmove = -2;
		ymove = Sequence - 11;
	}	
	else if (Sequence < 18)
	{  
		ymove = 2;
		xmove = Sequence - 15;
	}	
	else if (Sequence < 22)
	{  
		xmove = 2;
		ymove = -(Sequence - 19);
	}	
	else if (Sequence < 27)
	{  
		ymove = -2;
		xmove = -(Sequence - 23);
	}	
	else if (Sequence < 32)
	{  
		xmove = -3;
		ymove = Sequence - 28;
	}	
	else if (Sequence < 38)
	{  
		ymove = 3;
		xmove = Sequence - 34;
	}	
	else if (Sequence < 44)
	{  
		xmove = 3;
		ymove = -(Sequence - 40);
	}	
	else if (Sequence < 51)
	{  
		ymove = -3;
		xmove = -(Sequence - 46);
	}	
	else if (Sequence < 58)
	{  
		xmove = -4;
		ymove = Sequence - 53;
	}	
	else if (Sequence < 66)
	{  
		ymove = 4;
		xmove = Sequence - 61;
	}	
	else if (Sequence < 74)
	{  
		xmove = 4;
		ymove = -(Sequence - 69);
	}	
	else if (Sequence < 83)
	{  
		ymove = -4;
		xmove = -(Sequence - 77);
	}	
	else if (Sequence < 92)
	{  
		xmove = -5;
		ymove = Sequence - 86;
	}	
	else if (Sequence < 102)
	{  
		ymove = 5;
		xmove = Sequence - 96;
	}	
	else if (Sequence < 112)
	{  
		xmove = 5;
		ymove = -(Sequence - 106);
	}	
	else if (Sequence < 122)
	{  
		ymove = -5;
		xmove = -(Sequence - 116);
	}	
	else 
	{  
		shift++;
		Sequence -= 121;
		goto Top;
	}
	shiftdist = shift * 0.25 * Size;	
	Point.x += IDNINT (xmove * Size*1.2 + shiftdist);
	Point.y += IDNINT (-ymove * Size*1.2 + shiftdist);
{
#if ENABLETRACE
GSSiExitProg (1331);
#endif
	return Point;
}
#if ENABLETRACE
}
#endif
} 

BOOL FAR PASCAL HIGHLIGHTCLASSMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1299);
#endif
{	

 int	BRtn; 
 char	str[256]; 
 short	i, iclass, ntab;  
 int		TabStops[2]={138,1110};  
 static	LPTHEME	SaveTheme;
 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1299);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:
        cwCenter(hWndDlg, 0);  
        SaveTheme = CurTheme;
//   	    ntab = loadtabs (TabStops);    
       	SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLIST,LB_SETTABSTOPS,2,(LPARAM)&TabStops); 
		for (iclass=0;iclass<CurTheme->NumClass;iclass++) 
		{   
			sprintf (str,"%s\t%ld",CurTheme->ClassBM[iclass],CurTheme->ClassCount[iclass]);
			SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLIST,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)str)); 
		}
		if (CurTheme->NumMissing || CurTheme->NumInvalid)  
		{
			if (CurTheme->NumMissing) 
			{
				sprintf (str,"- Missing -\t%ld",CurTheme->NumMissing);
				SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLIST,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)str)); 
			} 
			else 
				SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLIST,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)""));  
			if (CurTheme->NumInvalid)
			{
				sprintf (str,"- Invalid -\t%ld",CurTheme->NumInvalid);
				SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLIST,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)str)); 
			} 
		}
		SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLIST,LB_SETSEL,TRUE,(LPARAM)DefaultHltClass);
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break; 
            
            case IDC_CLEARHLTLIST:
            	 ClearHighlightList(FALSE); 
            	 break;      
            case IDC_CANCEL:
            	 ContinueProcessing = FALSE;
            	 break;
            case IDC_UNHLT:
            case IDOK:
            {   
            	BOOL SavePick;
            	short	nItems, SaveNThemes;  
            	HANDLE	hItems;
            	LPSHORT	pItems; 
            	LPVIEWPORT	SaveView; 
            	long	TotItems, CurLoc=0;  
            	HANDLE	hPoly;
            	long	nPnts;
            	
				nItems=SendDlgItemMessage(hWndDlg,IDC_CLASSHLTLIST,LB_GETSELCOUNT,NULL,NULL); 
				if (!nItems) break;   
				if (!OpenThemeHighlightFile (BT_READ)) break;
					
				hItems=GSSiGlobAlloc (1019,GHND,nItems*2);
				pItems=  (LPSHORT) GlobalLock(hItems);
				SendDlgItemMessage(hWndDlg,IDC_CLASSHLTLIST,LB_GETSELITEMS,nItems,(LPARAM)pItems); 
				PickingByRefno=TRUE; 
				SavePick = Pick;   
				SaveView = CurView; 
				SetCurView (pViewports[CurTheme->TargetViewport-1]);
				Pick = FALSE; 
				TotItems = 0;
				for (i=0;i<nItems;i++,pItems++) 
				{
					if (*pItems == CurTheme->NumClass)
						TotItems += CurTheme->NumMissing;
					else if (*pItems > CurTheme->NumClass)
						TotItems += CurTheme->NumInvalid;
					else
						TotItems += CurTheme->ClassCount[*pItems];
				} 
				GlobalUnlock (hItems);
				pItems=  (LPSHORT) GlobalLock(hItems);   
				EnableWindow (GetDlgItem(hWndDlg,IDC_CANCEL),TRUE);
				EnableWindow (GetDlgItem(hWndDlg,IDC_UNHLT),FALSE);
				EnableWindow (GetDlgItem(hWndDlg,IDOK),FALSE);
				EnableWindow (GetDlgItem(hWndDlg,IDC_CLEARHLTLIST),FALSE);
				EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),FALSE);
				for (i=0;i<nItems;i++,pItems++)
				{   
					short	WantClass = *pItems;

					if (*pItems == CurTheme->NumClass)
						WantClass = -1;
					if (*pItems > CurTheme->NumClass)
						WantClass = -2;
					ThemeHighlightKey.Class = WantClass;
					ThemeHighlightKey.Refno = LONG_MIN;
					if (BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,BT_FIRST,BT_GE,(LPSTR)&ThemeHighlightData))
						ThemeHighlightKey.Class=-10;
					while (ContinueProcessing && ThemeHighlightKey.Class == WantClass)
					{   
						if (wParam == IDC_UNHLT)
							RemoveFromHighlightList (ThemeHighlightKey.Refno,0);
						else
						{   
							_fmemset (&PickList[0],0,sizeof(PICKDATA));
						    PickList[0].ViewID = CurView->ID;
							PickList[0].FileNum = ThemeHighlightData.FileNum;  
							PickList[0].SubFile = ThemeHighlightData.SubFile;
							PickList[0].FileInIndex = ThemeHighlightData.FileInIndex;
							PickList[0].Segment = ThemeHighlightData.Segment;
							PickList[0].Refno = ThemeHighlightKey.Refno;
							PickList[0].Desc = 0;
							PickList[0].Offset = ThemeHighlightData.Offset; 
							PickList[0].Element = ThemeHighlightData.Element; 
							PickList[0].Length = ThemeHighlightData.Length; 
							PickList[0].Area = ThemeHighlightData.Area; 
							SaveNThemes = CurView->NumThemes;
							CurView->NumThemes = 0;
							ProcessPickedItem (0,FALSE);    
							CurTheme = SaveTheme;
							CurView->NumThemes = SaveNThemes; 
							PickList[0].Type = 2; 
							if (CurrentType == GF_LINE || CurrentType == GF_POLYLINE) 
							{
								PickList[0].Type = 2; 
								if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPnts,&hPoly))  
								{   
									HPDPOINT lpPoints=(HPDPOINT)GlobalLock (hPoly);
									
									PickList[0].Length = GetPolyLengthD (lpPoints,nPnts);
		                            GSSiGlobUlFree (&hPoly);
								}
							}
							else if (CurrentType == GF_AREA) 
							{
								PickList[0].Type = 3; 
								if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPnts,&hPoly))  
								{   
									HPDPOINT lpPoints=(HPDPOINT)GlobalLock (hPoly);
									
									PickList[0].Area = ComputeProjectedAreaAreaD (lpPoints,nPnts,&PickList[0].Length);
		                            GSSiGlobUlFree (&hPoly);
								}
							}
							else if (CurrentType == GF_POINT)
								PickList[0].Type = 1; 
							else if (CurrentType == GF_TEXT)
								PickList[0].Type = 4;  
							PickList[0].Desc = CurrentDesc;   
							PickList[0].MSLink = CurMSLink;
							_fstrcpy (PickList[0].Prefix,CurrentTAG);
							_fstrcpy (PickList[0].UDI,CurrentUDI);
			     			AddToHighlightList (ThemeHighlightKey.Refno,&PickList[0],TRUE); 
				     	}
						if (BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,BT_NEXT,BT_ANY,(LPSTR)&ThemeHighlightData))
							ThemeHighlightKey.Class=-10;
	                    PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotItems, CurLoc++,0);
					}
				} 
				ContinueProcessing = TRUE;
				EnableWindow (GetDlgItem(hWndDlg,IDC_CANCEL),FALSE);
				EnableWindow (GetDlgItem(hWndDlg,IDC_UNHLT),TRUE);
				EnableWindow (GetDlgItem(hWndDlg,IDOK),TRUE);
				EnableWindow (GetDlgItem(hWndDlg,IDC_CLEARHLTLIST),TRUE);
				EnableWindow (GetDlgItem(hWndDlg,IDCANCEL),TRUE);
	            PctBox (GetDlgItem(hWndDlg,IDC_STATUS), TotItems, TotItems,0);
				GlobalUnlock (hItems);
				GlobalFree (hItems);
				PickingByRefno=FALSE;
				Pick = SavePick;  
				SetCurView (SaveView);
            }
                break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1299);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1299);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL FAR PASCAL DISPLAYSELECTEDCLASSESMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1300);
#endif
{	

 int	BRtn; 
 char	str[256]; 
 short	i, iclass, Choice;   
 int		TabStops[2]={2000,3000};  
 static	BOOL	Sorted=FALSE;
 
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1300);
#endif
 	return (BRtn);
}
 switch(Message)
   {
    case WM_INITDIALOG:
        cwCenter(hWndDlg, 0);  
	   	
	   	Sorted=FALSE;
	   	ShowWindow (GetDlgItem(hWndDlg,IDC_CLASSHLTLISTSORTED),SW_HIDE);
       	SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLISTSORTED,LB_SETTABSTOPS,2,(LPARAM)&TabStops); 
        
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{
			Choice = SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLIST,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)CurTheme->ClassBM[iclass])); 
			if (!CurTheme->ClassStatus[iclass])
				SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLIST,LB_SETSEL,TRUE,(LPARAM)Choice);
			sprintf (str,"%s\t%i",CurTheme->ClassBM[iclass],iclass);
			Choice = SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLISTSORTED,LB_ADDSTRING,NULL,(LPARAM)((LPSTR)str));
			if (!CurTheme->ClassStatus[iclass])
				SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLISTSORTED,LB_SETSEL,TRUE,(LPARAM)Choice);
		 }
		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
           	case IDC_SORT: 
           		Sorted = TRUE;
			   	ShowWindow (GetDlgItem(hWndDlg,IDC_CLASSHLTLIST),SW_HIDE);
	   			ShowWindow (GetDlgItem(hWndDlg,IDC_CLASSHLTLISTSORTED),SW_SHOW);
           	break;
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 EndDialog(hWndDlg, FALSE);
                 break; 
            
            case IDC_SELALL:
				SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLIST,LB_SETSEL,TRUE,(LPARAM)-1); 
				break;
				
            case IDC_CLEAR:
				SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLIST,LB_SETSEL,FALSE,(LPARAM)-1); 
				break;
				
            case IDC_INVERT:
				for (i=0;i<CurTheme->NumClass;i++) 
				{   
					BOOL IsSelected = SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLIST,LB_GETSEL,i,0);
					
					SendDlgItemMessage (hWndDlg,IDC_CLASSHLTLIST,LB_SETSEL,!IsSelected,(LPARAM)i);
				} 
				break;
				
            case IDOK:
            {   
            	short	nItems;  
            	HANDLE	hItems;
            	LPSHORT	pItems; 
            	
				for (i=0;i<MAX_THEME_CLASSES;i++) 
					CurTheme->ClassStatus[i] = 0; 
				if (Sorted)
				{
					nItems=SendDlgItemMessage(hWndDlg,IDC_CLASSHLTLISTSORTED,LB_GETCOUNT,NULL,NULL); 
					for (i=0;i<nItems;i++)
						CurTheme->ClassStatus[i] = 1;
					nItems=SendDlgItemMessage(hWndDlg,IDC_CLASSHLTLISTSORTED,LB_GETSELCOUNT,NULL,NULL); 
					if (!nItems) break;   
					
					hItems=GSSiGlobAlloc (1020,GHND,nItems*2);
					pItems=  (LPSHORT) GlobalLock(hItems);
					SendDlgItemMessage(hWndDlg,IDC_CLASSHLTLISTSORTED,LB_GETSELITEMS,nItems,(LPARAM)pItems);
					for (i=0;i<nItems;i++,pItems++)  
					{   
						LPSTR	pTab;
						
						SendDlgItemMessage(hWndDlg,IDC_CLASSHLTLISTSORTED,LB_GETTEXT,*pItems,(DWORD)str); 
						pTab = _fstrrchr (str,'\t')+1; 
						iclass = atoi (pTab);
						CurTheme->ClassStatus[iclass] = 0;
					} 
				}
				else
				{
					nItems=SendDlgItemMessage(hWndDlg,IDC_CLASSHLTLIST,LB_GETCOUNT,NULL,NULL); 
					for (i=0;i<nItems;i++)
						CurTheme->ClassStatus[i] = 1;
					nItems=SendDlgItemMessage(hWndDlg,IDC_CLASSHLTLIST,LB_GETSELCOUNT,NULL,NULL); 
					if (!nItems) break;   
					
					hItems=GSSiGlobAlloc (1020,GHND,nItems*2);
					pItems=  (LPSHORT) GlobalLock(hItems);
					SendDlgItemMessage(hWndDlg,IDC_CLASSHLTLIST,LB_GETSELITEMS,nItems,(LPARAM)pItems);
					for (i=0;i<nItems;i++,pItems++)
						CurTheme->ClassStatus[*pItems] = 0; 
				}
				GSSiGlobUlFree (&hItems);
                EndDialog(hWndDlg, TRUE);
            }
                break;

           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1300);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1300);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL HighlightByClass (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1313);
#endif
{
 char key;
 POINT	MousePoint;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONUP: 
    {
    	FARPROC lpfnHIGHLIGHTCLASSMsgProc;
    	short	iclass,i; 
    	
    	MousePoint = MAKEPOINT(lParam);
		if (!CurView->pTheme) break;
		CurTheme = CurView->pTheme;
		if (!CurTheme->IsActive) break; 
		DefaultHltClass = -1;
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{
			if (PickThemeClass(iclass,MousePoint))
			{
				DefaultHltClass = iclass;    
				break;
			}
		} 
		DoPaint = FALSE; 
        lpfnHIGHLIGHTCLASSMsgProc = MakeProcInstance((FARPROC)HIGHLIGHTCLASSMsgProc, hInst);
        DialogBox(hInst, (LPSTR)"HIGHLIGHTCLASS", CurView->hWnd, lpfnHIGHLIGHTCLASSMsgProc);
        FreeProcInstance(lpfnHIGHLIGHTCLASSMsgProc);
		DoPaint=TRUE;
	}
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1313);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1313);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

BOOL SelectThemeClasses (short iclass) 
#if ENABLETRACE
{GSSiEnterProg (1314);
#endif
{
	FARPROC lpfnDISPLAYSELECTEDCLASSESMsgProc;

	if (!CurView->pTheme)
{
#if ENABLETRACE
GSSiExitProg (1314);
#endif
		return FALSE;
}
	CurTheme = CurView->pTheme;
	if (!CurTheme->IsActive)
{
#if ENABLETRACE
GSSiExitProg (1314);
#endif
		return FALSE;
}
	if (iclass >= 0) //eventually used to specify class
{
#if ENABLETRACE
GSSiExitProg (1314);
#endif
		return FALSE;
}
	DoPaint = FALSE; 
    lpfnDISPLAYSELECTEDCLASSESMsgProc = MakeProcInstance((FARPROC)DISPLAYSELECTEDCLASSESMsgProc, hInst);
    DialogBox(hInst, (LPSTR)"DISPLAYSELECTEDCLASSES", CurView->hWnd, lpfnDISPLAYSELECTEDCLASSESMsgProc);
    FreeProcInstance(lpfnDISPLAYSELECTEDCLASSESMsgProc);
	DoPaint=TRUE; 
{
#if ENABLETRACE
GSSiExitProg (1314);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}


BOOL SelectByClass (HWND hWnd, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1315);
#endif
{
 char key;
 POINT	MousePoint;

 switch (Message)
   {
   	case GF_INIT:
       	AddLBUTTON = TRUE;
   		break;

    case WM_LBUTTONUP: 
    {
    	
    	SelectThemeClasses (-1);
	}
		break;

    default:
{
#if ENABLETRACE
GSSiExitProg (1315);
#endif
    	return (FALSE);
}
    }
{
#if ENABLETRACE
GSSiExitProg (1315);
#endif
    return (TRUE);
}
#if ENABLETRACE
}
#endif
} 

short GetThemeRefClass (long Refno)
#if ENABLETRACE
{GSSiEnterProg (1301);
#endif
{   
	short	class=-1, iclass;
	
	if (CurTheme)
	{
		if (OpenThemeHighlightFile (BT_READ))
		{   
			for (iclass=0;iclass<CurTheme->NumClass;iclass++)
			{
				ThemeHighlightKey.Class = iclass;
				ThemeHighlightKey.Refno = Refno;
				if (!BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,BT_FIRST,BT_EQ,(LPSTR)&ThemeHighlightData))
				{
					class = iclass+1;
					break;
				}
			}
			CloseThemeHighlightFile ();
		}
	}
{
#if ENABLETRACE
GSSiExitProg (1301);
#endif
	return class;
}
#if ENABLETRACE
}
#endif
}

BOOL CreateThemeHighlightFile (void)
#if ENABLETRACE
{GSSiEnterProg (1303);
#endif
{   
	LPSTR pName;
	BTVARDESC	BTVar[2];	
	short	len =  128;
	
	if (CurTheme->ID == GF_SINGLE_VALUE_THEME)
		len = 8;
	CurTheme->ValueLen = len;
	CurTheme->hHighlightFileName = GSSiGlobAlloc (1021,GMEM_MOVEABLE,256);
	pName = GlobalLock (CurTheme->hHighlightFileName);
	GSSiGetTempFileName (NULL,"gml",NULL,pName);
	BTVar[0].BT_VARTYP=BT_INTEGER;
	BTVar[0].BT_VARLEN=2;
	BTVar[0].BT_VAROFF=0;
	BTVar[1].BT_VARTYP=BT_INTEGER;
	BTVar[1].BT_VARLEN=4;
	BTVar[1].BT_VAROFF=2;
	BT_CREATE (pName, sizeof(ThemeHighlightData)-128+len, FALSE, 2, 1, (LPBTVARDESC)BTVar, FALSE, 0, 0, FALSE);
	GlobalUnlock (CurTheme->hHighlightFileName);    
{
#if ENABLETRACE
GSSiExitProg (1303);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL OpenThemeHighlightFile (short mode)
#if ENABLETRACE
{GSSiEnterProg (1304);
#endif
{   
	LPSTR	pName;
	

	if (!CurTheme->hHighlightFileName)
	{
		if (mode == BT_READ)
{
#if ENABLETRACE
GSSiExitProg (1304);
#endif
			return FALSE;
}
		CreateThemeHighlightFile ();
	}
	pName = GlobalLock (CurTheme->hHighlightFileName);
	if (pName)
	{
		CurTheme->hHighlightFile = BT_OPEN (pName, 0, mode, 0); 
		if (mode == BT_WRITE)
			BT_CLEAR (CurTheme->hHighlightFile);
	}
	GlobalUnlock (CurTheme->hHighlightFileName);
{
#if ENABLETRACE
GSSiExitProg (1304);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}

BOOL CloseThemeHighlightFile (void)
#if ENABLETRACE
{GSSiEnterProg (1305);
#endif
{
	BT_CLOSE (CurTheme->hHighlightFile);
	CurTheme->hHighlightFile = 0;
{
#if ENABLETRACE
GSSiExitProg (1305);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL DeleteThemeHighlightFile (void)
#if ENABLETRACE
{GSSiEnterProg (1306);
#endif
{   
	LPSTR pName; 
	OFSTRUCT	OFStruct;
	
	if (!CurTheme->hHighlightFileName)
{
#if ENABLETRACE
GSSiExitProg (1306);
#endif
		return FALSE;
}
	CloseThemeHighlightFile (); 
	pName = GlobalLock (CurTheme->hHighlightFileName);
	GSSiRemove (pName); 
	GSSiGlobUlFree (&CurTheme->hHighlightFileName);
{
#if ENABLETRACE
GSSiExitProg (1306);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
}  

double GetThemeClassDistance (short iclass)
#if ENABLETRACE
{GSSiEnterProg (1326);
#endif
{   
	double	Dist=0, MinDist, Dist2;  
	DPOINT	MidPoint, FromPoint;
	HANDLE	handle;  
	HPDPOINT	pPoint; 
	long	nPoints=0,i,Mini;
	short	pos, cond;
	static	double	ClassDist[MAX_THEME_CLASSES];
	
	if (iclass < 0)
		_fmemset (ClassDist,0,sizeof(double)*MAX_THEME_CLASSES);
	if (iclass < 0)
	{	
		if (!OpenThemeHighlightFile (BT_READ))
{
#if ENABLETRACE
GSSiExitProg (1326);
#endif
			return 0;
}
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{   
			Dist = 0;
			if (CurTheme->ClassCount[iclass])
			{
				handle = GSSiGlobAlloc (1025,GMEM_MOVEABLE,(CurTheme->ClassCount[iclass]+1)*sizeof(DPOINT));
				pPoint = (HPDPOINT)GlobalLock (handle); 
				ThemeHighlightKey.Class = iclass;
				ThemeHighlightKey.Refno = LONG_MIN; 
				nPoints = 0;
				pos = BT_FIRST;
				cond = BT_GE;
				while (!BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,pos,cond,(LPSTR)&ThemeHighlightData))
				{
					pos = BT_NEXT;
					cond = BT_ANY;
					if (ThemeHighlightKey.Class != iclass)
						break;
					if (nPoints)
					{
						MidPoint.x += ThemeHighlightData.Point.x;
						MidPoint.y += ThemeHighlightData.Point.y;
					}
					else
						MidPoint =  ThemeHighlightData.Point;
					pPoint[nPoints++] = ThemeHighlightData.Point;
				}
				MidPoint.x /= nPoints;
				MidPoint.y /= nPoints;
				GetGlobalPVal ("[%STARTPOINT]",&MidPoint,&FromPoint); 
				while (nPoints)
				{   
					MinDist = DBL_MAX;
					for (i=0;i<nPoints;i++) 
					{
						Dist2 = ldistp (FromPoint,pPoint[i]);
						if (Dist2 < MinDist)
						{
							MinDist = Dist2;
							Mini = i;
						}
					}
					Dist += MinDist;
					FromPoint = pPoint[Mini];
					nPoints--;
					if (Mini < nPoints)
						hmemmove ((HPSTR)&pPoint[Mini],(HPSTR)&pPoint[Mini+1],(long)sizeof(DPOINT)*(nPoints-Mini));
				}
				GSSiGlobUlFree (&handle);
				ClassDist[iclass] = Dist*MFT/5280;
			}
		} 
		CloseThemeHighlightFile (); 
{
#if ENABLETRACE
GSSiExitProg (1326);
#endif
		return 0;
}
	}
{
#if ENABLETRACE
GSSiExitProg (1326);
#endif
	return ClassDist[iclass];
}
#if ENABLETRACE
}
#endif
}
void EndProcessingThemeLegends (void)
#if ENABLETRACE
{GSSiEnterProg (1327);
#endif
{   
	short	iview, itheme;  
	LPVIEWPORT	SaveView = CurView;
	
	for (iview = 0;iview < *pNumViewports; iview++)
    {
    	SetCurView (pViewports[iview]); 
        if (CurViewActive())
        {
        	if (CurView->pTheme)
			{
				CurTheme=CurView->pTheme;
				if ((CurTheme->ID == PF_COORD_DISPLAY || CurTheme->IsActive) &&
				    CurTheme->ID != GF_BOUNDS_DISPLAY_THEME)
				    ThemeDisplayLegend(4,SaveView->ID);
			}
		}
	}
	for (iview = 0;iview < *pNumViewports; iview++)
    {
    	SetCurView (pViewports[iview]); 
        if (CurViewActive())
			DisplayCloseIcon ();
	}
	SetCurView (SaveView);
{
#if ENABLETRACE
GSSiExitProg (1327);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
void GetClassOrder (short NumClass,LPLONG ClassCounts,LPSHORT Order)
{   
	short	i,j,n=1;
	
	Order[0] = 0;
	for (i=1;i<NumClass;i++)
	{
		for (j=0;j<n;j++)
		{
			if (ClassCounts[i] < ClassCounts[Order[j]]) 
			{
				_fmemmove (&Order[j+1],&Order[j],(n-j)*2);
				Order[j] = i;
				goto Next;
			}
		}     
		Order[n] = i;
Next:	n++;
	}
	return;
}

BOOL ThemeEndDisplayPass(BOOL CloseAll,BOOL PixelThemesOnly,BOOL FromHalt)
#if ENABLETRACE
{GSSiEnterProg (1274);
#endif
{	int	itheme, iclass,ii; 
    LPTHEME SaveTheme=CurTheme, CacheDisplayTheme=NULL;
     
                       
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{   
		if (!CurView->pThemes[itheme])
			goto NextTheme;
		CurTheme=CurView->pThemes[itheme];  
		if (FromHalt && CurTheme->FidDelayedText>0)
		  	CloseAndDeleteFile (&CurTheme->FidDelayedText); 
		if (!CurTheme->IsActive|| !CurTheme->VPDisplayed)
			goto NextTheme;
		if (PixelThemesOnly && CurTheme->DataType != THEMEDATATYPE_PIXEL)
			goto NextTheme;
		if (!PixelThemesOnly && CurTheme->DataType == THEMEDATATYPE_PIXEL)
			goto NextTheme;
		GSSiGlobFree (&CurTheme->hVisList);  
		if (CurTheme->ComputeStoredCounts && !FromHalt)
		 	CurTheme->UseStoredCounts = TRUE;
		switch (CurTheme->ID)
		{   
	        case GF_CACHE_DISPLAY_THEME:
	        	CacheDisplayTheme = CurTheme;
			    break;
	        case GF_CONTEST_THEME:
				GSSiDeleteObject(&CurTheme->ClassPen[0]);
				GSSiDeleteObject(&CurTheme->ClassPen[1]);
	        	BT_CLOSE (hBTNetCon); 
	        	hBTNetCon = 0;
	        	break;
	        	
			case GF_NETMARKER_THEME: 
				CloseNetLinkAndRef (CurTheme->ReScan);  
				CurTheme->ReScan=FALSE;
				BT_CLOSE (CurTheme->hScatterFile);
				CurTheme->hScatterFile = 0; 
				if (NetMarkTimer)
					KillTimer(hWndMain,NetMarkTimer); 
				NetMarkTimer = 0;
				break;
			case GF_POINT_IN_AREA_THEME:
				ii=1;
			case GF_SINGLE_NONNUM_VALUE_THEME: 
				if (CurTheme->FieldFun && !StartAutoClassDef ())
					BT_CLOSEANDDELETE (&CurTheme->hScatterFile);
				else
					BT_CLOSE (CurTheme->hScatterFile);
				CurTheme->hScatterFile = 0;
			case GF_SINGLE_VALUE_THEME: 
			case GF_TIME_DISPLAY_THEME:
			case GF_TWO_VALUE_THEME: 
				CloseThemeHighlightFile ();
				if (FromHalt)
					BT_CLOSE (CurTheme->hDisperseFile);
            	else
					ClosePointDispersionFile (CloseAll);
				CurTheme->hDisperseFile = 0;
				CloseThemeDataFile(FALSE); 
				SetShowValDB (NULL); 
				if (!FromHalt)
					ProcessShowValMacro (FALSE);
				if (ComputeThemePCTByArea (CloseAll))
				{
					GSSiDeleteObject (&CurTheme->NoDataBrush);
					GSSiDeleteObject (&CurTheme->InvalidDataBrush);
					CurTheme = SaveTheme;
{
#if ENABLETRACE
GSSiExitProg (1274);
#endif
					return FALSE;
}
                }
				break;   
			case GF_STREET_TEXT_THEME:
			{  
			    LPSTREETTEXTDATA	pStreetData=(LPSTREETTEXTDATA)CurTheme->ClassBM;
				CloseThemeDataFile(FALSE); 
				BT_CLOSEANDDELETE (&pStreetData->hNameFile1);     
				BT_CLOSE(pStreetData->hNameFile2);  
				pStreetData->hNameFile2 = 0; 
				if (pStreetData->hListDB)
					CloseGWDatabase (pStreetData->hListDB);   
				pStreetData->hListDB = 0;
				ProcessAllElements = pStreetData->SavePAE;
				if (CloseAll)
					GSSiGlobFree (&CurTheme->hScatterFile);   
			}
				break;
			case GF_STREET_ADDRESS_THEME: 
				if (StreetEditTimer)
					KillTimer(hWndMain,StreetEditTimer); 
				StreetEditTimer = 0;    
				if (CloseAll)
					GSSiGlobFree (&CurTheme->hScatterFile);   
				break;
				
			case GF_POLYINFO_THEME:
				DisplayCurveFactor = 0;
				break;
		}
		GSSiDeleteObject (&CurTheme->NoDataBrush);
		GSSiDeleteObject (&CurTheme->InvalidDataBrush);
NextTheme:
	;
	}  
	CurTheme = SaveTheme;
{
#if ENABLETRACE
GSSiExitProg (1274);
#endif
	return FALSE;
}

#if ENABLETRACE
}
#endif
} 

void CreateThemePens (LPTHEME CurTheme,BOOL AlwaysCreate)
#if ENABLETRACE
{GSSiEnterProg (1270);
#endif
{   
	short	iclass, pattern,ii;
	LPVIEWPORT	SaveVP=CurView;  
	double	WF;
	
	if (!AlwaysCreate && CurTheme->HiPrecis == HiPrecis)
{
#if ENABLETRACE
GSSiExitProg (1270);
#endif
		return;
}
	DestroyThemePens (CurTheme);
//	SetCurView (pViewports[CurTheme->TargetViewport-1]); //tempdebu
    SetViewport (CurTheme->TargetViewport); 
    if (HiPrecis)
    	WF = 1;
    else
    	WF = WidthFactor;
	for (iclass=0;iclass<CurTheme->NumDesiredClass;iclass++)
	{   
		COLORREF	color; 
		int			width=0;
		BYTE		PatByt;
		PATBYTE		PatByte;
				
    	width = PatByt = GetWValue (CurTheme->ClassColor[iclass]); 
    	if (CurTheme->DataType != 2)
    		width = 0; 
    	else if (CurTheme->ClassFactor[iclass] > 0)
    		width = CurTheme->ClassFactor[iclass];
    	if (CurTheme->DataType)
    		PatByt = 0;
    	if (width > 128)
    		width -= 256;
    	_fmemmove (&PatByte,&PatByt,1);
    	color = ColorWOWidth (CurTheme->ClassColor[iclass]);
        if (!width)
        {  
            if (*PenWIDTH > 0)
            	width = *PenWIDTH;   
//            else
//            	width = 1;
        } 
        else
        	ii=1;
        if (width >= 0)
        	width = IDNINT(((double)(width/*+0.5*/)) * WF * DeviceToScreenFactor * PenWidthFactor); 
        else if (CurView->BaseUnitsPerPixel)
        	width = IDNINT(((double)-width / CurView->BaseUnitsPerPixel)* WF * DeviceToScreenFactor * PenWidthFactor); 
		if (ComputePCTTheme || !PatByte.Pattern)
			CurTheme->ClassBrush[iclass]=CreateSolidBrush(ConvertColor(ColorWOWidth (CurTheme->ClassColor[iclass]),CurTheme->UseHalfTone));
/*		else if (PatByte.Pattern == 1) 
		{ 
		    LOGBRUSH	NDB;   
				    
			NDB.lbStyle = BS_HATCHED;
			NDB.lbColor = ColorWOWidth (CurTheme->ClassColor[iclass]);
			NDB.lbHatch	= HS_DIAGCROSS;
			CurTheme->ClassBrush[iclass] = CreateBrushIndirect(&NDB);
	    	SetBkMode (CurView->hDC,TRANSPARENT); 
        } */
        else
        {   
			HBITMAP hbmp = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(PatBMP[PatByte.Pattern-1]));  
			CurTheme->ClassBrush[iclass] = CreatePatternBrush(hbmp);
	        DeleteObject (hbmp); 
        }
		CurTheme->ClassPen[iclass]= CreatePen(PS_SOLID,width,ConvertColor(color,CurTheme->UseHalfTone));
	} 
	SetCurView (SaveVP);     
	CurTheme->HiPrecis = HiPrecis;
{
#if ENABLETRACE
GSSiExitProg (1270);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void DestroyClassPens (void)
#if ENABLETRACE
{GSSiEnterProg (1271);
#endif
{   int	itheme, iclass, i; 
    LPTHEME	SaveCurTheme=CurTheme;  
    
    if (!CurView)
{
#if ENABLETRACE
GSSiExitProg (1271);
#endif
    	return;
}
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{
		CurTheme=CurView->pThemes[itheme];
		if (CurTheme)
		{
			DestroyThemePens (CurTheme);
		}
	}
	CurTheme = SaveCurTheme;
{
#if ENABLETRACE
GSSiExitProg (1271);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void DestroyThemePens (LPTHEME CurTheme) 
#if ENABLETRACE
{GSSiEnterProg (1272);
#endif
{
	short iclass;
	BOOL PenInUse = FALSE;
	BOOL BrushInUse = FALSE;     
	HPEN	OldPen=0;
	HBRUSH	OldBrush=0;
	
    if (CurView)
    {
		OldPen = SelectObject (CurView->hDC,GetStockObject(NULL_PEN));
		OldBrush = SelectObject (CurView->hDC,GetStockObject(NULL_BRUSH)); 
	}
	
//	for (iclass=0;iclass<CurTheme->NumDesiredClass;iclass++)
	for (iclass=0;iclass<MAX_THEME_CLASSES;iclass++)
	{   
		if (CurTheme->ClassPen[iclass] == OldPen)
			PenInUse = TRUE;
		if (CurTheme->ClassBrush[iclass] == OldBrush)
			BrushInUse = TRUE;
		GSSiDeleteObject(&CurTheme->ClassBrush[iclass]);
		GSSiDeleteObject(&CurTheme->ClassPen[iclass]);
	}
	CurTheme->HiPrecis = 2; 
	if (OldPen && !PenInUse)
		SelectObject (CurView->hDC,OldPen);
	if (OldBrush && !BrushInUse)
		SelectObject (CurView->hDC,OldBrush);
{
#if ENABLETRACE
GSSiExitProg (1272);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

void SetThemeElementCharacteristics (UINT iclass) 
#if ENABLETRACE
{GSSiEnterProg (1228);
#endif
{   
	BYTE		PatByt;
	PATBYTE		PatByte;
	
	if (!Display)
{
#if ENABLETRACE
GSSiExitProg (1228);
#endif
		return;
}   
	if (CurTheme->ID == GF_SINGLE_NONNUM_VALUE_THEME && !CurTheme->NumDesiredClass)
	{   
		CurTheme->ClassColor[0] = CurTheme->ValueColor; 
		CurTheme->NumDesiredClass = 1;
		CreateThemePens (CurTheme,TRUE);  
		CurTheme->NumDesiredClass = 0;
		iclass = 0;  
	}
	switch (CurTheme->DataType) 
	{
		case THEMEDATATYPE_AREA: 
			if (CurrentType != GF_AREA)
				break;
			goto SetArea;
		break;
					    		
		case THEMEDATATYPE_POINT: 
			if (CurrentType != GF_POINT)
				break;
			if (!CurTheme->UseFirstSymbol && (ThemePointSym = CurTheme->ClassSymbol[iclass])) 
			{
				SetPointSize (&ThemePointSize,CurTheme->SymSizeC); 
				if (*CurTheme->SymbolFont[0])
				{
					ThemePointSym = -ThemePointSym;   
					_fmemmove (CurSymbolFont,CurTheme->SymbolFont,sizeof(CurSymbolFont));
				}
			}
		//	ThemeWidthFactor = GetWValue (CurTheme->ClassColor[iclass]) + 1;  
			if (CurTheme->ClassFactor[iclass])
				ThemeWidthFactor = CurTheme->ClassFactor[iclass];
			else
				ThemeWidthFactor = 1;
			if (!CurTheme->NotSetColor) 
			{
				SelectObject (CurView->hDC,CurTheme->ClassBrush[iclass]);
				HaveVarFillColor = TRUE;  
				ThemePointColor = GlobalColors[0]=CurTheme->ClassColor[iclass];
				ThemePointUseHalfTone = CurTheme->UseHalfTone; 
				SetTextColor (CurView->hDC,CurTheme->ClassColor[iclass]);
			} 
			break;
		case THEMEDATATYPE_LINE: 
			if (CurrentType != GF_LINE && CurrentType != GF_POLYLINE && CurrentType != GF_CURVE)
				break;
			goto SetLine; 
			break; 
		case THEMEDATATYPE_TEXT: //text  
			if (CurrentType != GF_TEXT)
				break;
			if (!CurTheme->NotSetColor) 
			{
				SetTextColor (CurView->hDC,CurTheme->ClassColor[iclass]);
				ThemeTextSizeFactor = CurTheme->ClassFactor[iclass];
			}
			break;
		case THEMEDATATYPE_ALL: //all   
			switch (CurrentType)
			{
				case GF_AREA:
				case GF_POINT:
				case GF_TEXT:
					goto SetArea;
				break;
				default:
					goto SetLine;
				break;
			}
			break; 
		case THEMEDATATYPE_PIXEL:  
			ThemePointColor = CurTheme->ClassColor[iclass];
			ThemePointUseHalfTone = CurTheme->UseHalfTone; 
			goto SetArea;
			break;
	}
{
#if ENABLETRACE
GSSiExitProg (1228);
#endif
	return; 
}

SetLine:
	if (!CurTheme->NotSetColor) 
	{
		SelectObject (CurView->hDC,CurTheme->ClassPen[iclass]);
		HaveVarFillColor = TRUE;  
		ThemePointColor = GlobalColors[0]=CurTheme->ClassColor[iclass]; 
		ThemePointUseHalfTone = CurTheme->UseHalfTone; 
		if (CurTheme->ClassFactor[iclass] > 0)  
			ThemeWidthFactor = CurTheme->ClassFactor[iclass];  
		else if (CurTheme->ClassFactor[iclass] < 0)
			ThemeWidthFactor = -BaseDistToWinDist * CurTheme->ClassFactor[iclass];  
		else
			ThemeWidthFactor = 1;
		SetTextColor (CurView->hDC,CurTheme->ClassColor[iclass]);
	}
{
#if ENABLETRACE
GSSiExitProg (1228);
#endif
	return;
}
		 
SetArea:
	if (CurrentType == GF_AREA && nPoly>1)
	{   
		if (!CurTheme->NotSetColor) 
		{
			SelectObject (CurView->hDC, h0Pen);
			ThemePolyPen = CurTheme->ClassPen[iclass]; 
		}
	}
	else
	{
		if (GetBit (5,(LPSTR)&CurVis->WantType[7])) 
		{   
			short	ii;
			if (!SelectObject (CurView->hDC,hAreaBorderPen[HiPrecis]))
				ii=1;
		}
        else if (!CurTheme->NotSetColor)
    		SelectObject (CurView->hDC,CurTheme->ClassPen[iclass]);
    }
						
	if (!CurTheme->NotSetColor) 
	{
		if (SolidAreas && GetBit (7,(LPSTR)&CurVis->WantType[7]))
			SelectObject (CurView->hDC,CurTheme->ClassBrush[iclass]);
		HaveVarFillColor = TRUE;  
		ThemePointColor = GlobalColors[0]=CurTheme->ClassColor[iclass]; 
		SetTextColor (CurView->hDC,CurTheme->ClassColor[iclass]);
		PatByt = GetWValue (CurTheme->ClassColor[iclass]); 
		_fmemmove (&PatByte,&PatByt,1);
	    SetROP2(CurView->hDC,DisplayRasterOpt);
		if (PatByte.Pattern)
		{
		    SetTextColor (CurView->hDC,ColorWOWidth (CurTheme->ClassColor[iclass]));     
		    SetBkColor (CurView->hDC,RGB(255,255,255));
		    if (PatByte.Transparent)
				SetROP2(CurView->hDC,R2_MASKPEN);
	    } 
	}
{
#if ENABLETRACE
GSSiExitProg (1228);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}  

BOOL CreatePointInAreaDB (LPSTR PIADataFile)
{    
	LPSTR	pDot;    
	BOOL	rtn;   
	char	DefStr[]="Refno(B4),TotCount(B4),TotValue(R8)";
	
	if (!ExistFile (PIADataFile)) 
	{
		GSSiGetTempFileName (0,"gmp",0,PIADataFile);
		if ((pDot= _fstrrchr (PIADataFile,'.')))
			*pDot = 0;
		else
			pDot = _fstrchr (PIADataFile,0);
		_fstrcat (pDot,"\\pia.gmd"); 
	}
	rtn = CreateGWDDatabase (PIADataFile,1,FALSE,0,1,DefStr);
	return rtn;
}

BOOL FAR PASCAL TIME_DISPLAYMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
{
 return TRUE;
}

BOOL FAR PASCAL POINT_IN_AREAMsgProc(HWND hWndDlg, WORD Message, WORD wParam, LONG lParam)
#if ENABLETRACE
{GSSiEnterProg (1247);
#endif
{	HWND	hCheckBox;
	RECT	rect;
	HDC		hDC;
	char	str[256],  *ptr;
LPSTR lpSTRING;
static  FIELDINFO FIELD;
static	LPFIELDINFO lpmess = &FIELD;
//lda addition
static	LPFIELDINFO lpFieldInfo = &FIELD;
HENV henv;
HDBC hdbc;
SWORD iptr, outlen, deslen;
UCHAR namel[256];
SDWORD namelen;
RETCODE rc;
int IRC, dlgitem,n;
LPSHORT irc = &IRC;  
static int i, NumFieldNames;
static LPVIEWPORT SaveView;   
LPTHEME	pSaveTheme;
static	HANDLE	hSaveTheme=0;
// end of lda addition
//FIELDINFO FAR *LPFIELDINFO ;
	long	icount;
	char index[] = "refno", keydata[] = "    -97492987" ;
	LPVOID LPIndex = &index, LPKeydata = &keydata;
	char ANSWER[64];
	char *answer = ANSWER;
	int	Choice;  
	BOOL	True=TRUE, Error;  
	int	IDC_FieldName=SV_FIELD_NAME; 
	COLORREF	Color;    
	long	ii;  
	UINT	nPrompts=42;
	UINT	PrmtDat[42*3] = {
				SV_TARGET,PRMT_SV_TARGET,0,
				SV_CB_ZEROBASED, PRMT_SV_CB_ZEROBASED,0,
				SV_CONTENTS_LIST, PRMT_SV_CONTENTS_LIST,MORE_SV_CONTENTS_LIST,
				SV_ITEM_TYPE, PRMT_SV_ITEM_TYPE,0,
				SV_DATABASE_LIST, PRMT_SV_DATABASE_LIST,0,
				SV_TABLE_NAMES, PRMT_SV_TABLE_NAMES,0,
				SV_FIELDFUNCTION, PRMT_SV_FIELDFUNCTION,MORE_SV_FIELDFUNCTION,
				SV_FIELD_NAME, PRMT_SV_FIELD_NAME,0,
				SV_CORRECTION, PRMT_SV_CORRECTION,0,
				SV_FIELD_VALUE, PRMT_SV_FIELD_VALUE,MORE_SV_FIELD_VALUE,
				IDC_SETSQL, PRMT_IDC_SETSQL, MORE_IDC_SETSQL,
				IDC_SQL, PRMT_IDC_SQL, MORE_IDC_SQL,
				SV_TITLE, PRMT_SV_TITLE, 0, 
				SV_NUM_CLASSES, PRMT_SV_NUM_CLASSES,MORE_SV_NUM_CLASSES,
				SV_MAXVAL, PRMT_SV_MAXVAL,0,
				IDC_ROUND_TO, PRMT_IDC_ROUND_TO,0,
				SV_CB_PERCENTILES, PRMT_SV_CB_PERCENTILES,0,
				SV_CB_EVENRANGES, PRMT_SV_CB_EVENRANGES, 0,
				SV_CB_MANUAL, PRMT_SV_CB_MANUAL,0,
				IDC_EDITRANGES, PRMT_IDC_EDITRANGES,0,
				IDC_MISSING_OPT0, PRMT_IDC_MISSING_OPT0,MORE_IDC_MISSING_OPT0,
				IDC_MISSING_OPT1, PRMT_IDC_MISSING_OPT1,0,
				IDC_MISSING_OPT2, PRMT_IDC_MISSING_OPT2,0,
				IDC_MISSING_OPT3, PRMT_IDC_MISSING_OPT3, MORE_IDC_MISSING_OPT3,
				SV_DISPLAY_DATAPOINTS, PRMT_SV_DISPLAY_DATAPOINTS, MORE_SV_DISPLAY_DATAPOINTS,
				SV_INSERT_COMMAS, PRMT_SV_INSERT_COMMAS, 0,
				SV_DISPLAY_VALUE, PRMT_SV_DISPLAY_VALUE, MORE_SV_DISPLAY_VALUE,
				SV_CB_ZEROASMISS, PRMT_SV_CB_ZEROASMISS, 0,
				IDC_MARK_INVALID, PRMT_IDC_MARK_INVALID, MORE_IDC_MARK_INVALID,
				SV_CB_RECOMPUTE, PRMT_SV_CB_RECOMPUTE, MORE_SV_CB_RECOMPUTE,
				SV_DISPLAY_PCT, PRMT_SV_DISPLAY_PCT, MORE_SV_DISPLAY_PCT,
				IDC_SAVE_THEME, PRMT_IDC_SAVE_THEME, MORE_IDC_SAVE_THEME,
				IDC_LOAD_THEME, PRMT_IDC_LOAD_THEME, MORE_IDC_LOAD_THEME,
				SV_COLOR_SCHEME,PRMT_SV_COLOR_SCHEME, MORE_SV_COLOR_SCHEME,
				IDC_DEFINE_COLOR_SCHEME, PRMT_IDC_DEFINE_COLOR_SCHEME, 0,
				IDC_BGCOLOR, PRMT_IDC_BGCOLOR, MORE_IDC_BGCOLOR,
				IDC_TITBGCOLOR, PRMT_IDC_TITBGCOLOR, MORE_IDC_TITBGCOLOR,
				IDC_VALBGCOLOR, PRMT_IDC_VALBGCOLOR, MORE_IDC_VALBGCOLOR,
				IDC_TITSIZE, PRMT_IDC_TITSIZE, MORE_IDC_TITSIZE,
				IDC_MARGIN, PRMT_IDC_MARGIN, MORE_IDC_MARGIN,
				SV_DISPERSE, PRMT_SV_DISPERSE,0,
				SV_PCTBYAREA, PRMT_SV_PCTBYAREA, 0
			};
  
 int	BRtn;
 if (Message==WM_INITDIALOG)
 {  
 	UINT	pw=0, pn=1, pm=2; 
 	HWND	hwnd;
 	short	ii;
 	
 	InitDlgPrompts (hWndDlg);
 	while (nPrompts--)
 	{   
 		hwnd = GetDlgItem(hWndDlg,PrmtDat[pw]);
 		if (!hwnd)
 			ii=1;
	 	SetDlgPrompt (hwnd,PrmtDat[pn],PrmtDat[pm]);
	 	pw+=3;
	 	pn+=3; 
	 	pm+=3;
	} 
	CreatePointInAreaDB (CurTheme->DataFile);
 }
 if ((BRtn = DIALOGSTYLEMsgProc (hWndDlg,Message, wParam, lParam)))
{
#if ENABLETRACE
GSSiExitProg (1247);
#endif
 	return (BRtn);
}
 if (CurTheme->ID != GF_GRAPHICS_FUNCTION_THEME)
 if (DATAFILEMsgProc(hWndDlg,Message,wParam,lParam,IDC_SQL,
 					 SV_SET_FILE, SV_DATABASE_LIST, SV_TABLE_NAMES,SV_TABLE_HEADING, &IDC_FieldName,1,
 					 CurTheme->DataFile, &CurTheme->DataFileType, &CurTheme->hThemeDB, &True,FALSE))
{
#if ENABLETRACE
GSSiExitProg (1247);
#endif
	return TRUE;
}
 if (ThemeCommonCode (hWndDlg, Message, wParam, lParam,CurTheme->hThemeDB))
{
#if ENABLETRACE
GSSiExitProg (1247);
#endif
 	return TRUE;
}
 switch(Message)
   { 
    case WM_INITDIALOG:  
    	ClearDlgPrompts (); 
    	DoPaint = FALSE; 
    	if (CurTheme->ID == GF_GRAPHICS_FUNCTION_THEME)
        	SetDlgItemText (hWndDlg,SV_DATABASE_LIST,CurTheme->DataFile);
    	hSaveTheme = GSSiGlobAlloc ( 636,GHND,sizeof(THEME));
    	pSaveTheme = (LPTHEME)GlobalLock (hSaveTheme);
    	*pSaveTheme = *CurTheme;
    	GlobalUnlock (hSaveTheme);
		for (i=0;i<MAX_THEME_CLASSES;i++)
		{
			itoa (i+1,str,10);
			SendDlgItemMessage (hWndDlg,SV_COLUMNS,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)str)); 
		}
		SendDlgItemMessage (hWndDlg,SV_COLUMNS,CB_SETCURSEL,CurTheme->NumCols,NULL); 
        if (CurTheme->FieldFun==6 && !*CurTheme->Value)
         	sprintf (CurTheme->Value,"[%s]",CurTheme->Field.name); 
    	SaveView = CurView;
	    NumFieldNames = 0;									       
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Value of"));
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Count of"));
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Average of"));
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Sum of")); 
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Min of")); 
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Max of")); 
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Expression")); 
		SendDlgItemMessage (hWndDlg,SV_FIELDFUNCTION,CB_SETCURSEL,CurTheme->FieldFun,NULL); 
		
		SetFieldCorrectionOpts (hWndDlg,CurTheme->DataType); 
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Areas"));
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Points"));
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Lines")); 
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Text")); 
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"All")); 
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"Pixels")); 
		SendDlgItemMessage (hWndDlg,SV_ITEM_TYPE,CB_SETCURSEL,CurTheme->DataType,NULL); 
		
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"100,000"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"10,000"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"1,000"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"100"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"10"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"1"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"0.1"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"0.01"));
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"0.001")); 
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"0.0001")); 
		SendDlgItemMessage (hWndDlg,IDC_ROUND_TO,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)"0.00001")); 
		 sprintf (str,"%f",CurTheme->RoundTo);
    	 SetDlgItemText(hWndDlg,IDC_ROUND_TO,str);
    	 SetDlgItemText(hWndDlg,SV_TITLE,(LPSTR)CurTheme->Title);
       	 SendDlgItemMessage (hWndDlg,SV_DISPLAY_DATAPOINTS,BM_SETCHECK,CurTheme->DisplayScatterDiagram,0L);
       	 SendDlgItemMessage (hWndDlg,SV_INSERT_COMMAS,BM_SETCHECK,CurTheme->AddCommas,0L);
       	 SendDlgItemMessage (hWndDlg,SV_CB_ZEROBASED,BM_SETCHECK,CurTheme->ZeroBased,0L);
       	 SendDlgItemMessage (hWndDlg,SV_CB_ZEROASMISS,BM_SETCHECK,CurTheme->ZeroIsMissing,0L);
       	 SendDlgItemMessage (hWndDlg,IDC_MARK_INVALID,BM_SETCHECK,CurTheme->MarkInvalid,0L);
       	 SendDlgItemMessage (hWndDlg,IDC_SKIP_INVALID,BM_SETCHECK,CurTheme->SkipInvalid,0L);
       	 SendDlgItemMessage (hWndDlg,SV_DISPLAY_VALUE,BM_SETCHECK,CurTheme->ShowValue,0L); 
		 SendDlgItemMessage (hWndDlg,SV_DISPERSE,BM_SETCHECK,FALSE,0L);
 		 SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_SETCHECK,FALSE,0L);
 		 for (i=0;i<*pNumViewports;i++)
 		 {
			Choice = SendDlgItemMessage (hWndDlg,IDC_POINTTHEME_LIST,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)pViewports[i]->Name));  
			if (pViewports[i]->ID == CurTheme->PointInAreaPointThemeVPID)
				SendDlgItemMessage (hWndDlg,IDC_POINTTHEME_LIST,CB_SETCURSEL,Choice,NULL); 
 		 }
       	 switch (CurTheme->DispersePoints)
       	 {
       	 	case 1:
       	 		SendDlgItemMessage (hWndDlg,SV_DISPERSE,BM_SETCHECK,TRUE,0L);
       	 		break;
       	 	case 2:
       	 		SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_SETCHECK,TRUE,0L);
       	 		break;
       	 }
	   	 ShowWindow (GetDlgItem(hWndDlg,IDC_EDITRANGES),SW_HIDE);
       	 if (CurTheme->ClassType ==1) dlgitem = SV_CB_EVENRANGES;
       	 if (CurTheme->ClassType ==2) dlgitem = SV_CB_PERCENTILES;
       	 if (CurTheme->ClassType ==3)
       	 {
       	 	dlgitem = SV_CB_MANUAL;
		   	ShowWindow (GetDlgItem(hWndDlg,IDC_EDITRANGES),SW_SHOW);
//10/17/03			CurTheme->Recompute=FALSE;
       	 }
       	 SendDlgItemMessage (hWndDlg,SV_CB_RECOMPUTE,BM_SETCHECK,CurTheme->Recompute,0L);
	     SendDlgItemMessage (hWndDlg,dlgitem,BM_SETCHECK,TRUE,0L);
       	 itoa (CurTheme->NumDesiredClass,str,10);
       	 SetDlgItemText (hWndDlg,SV_NUM_CLASSES,str);
       	 if (CurTheme->YLimit < DBL_MAX)
       	 	sprintf (str,"%f",CurTheme->YLimit);
       	 else
       	 	str[0]='\0';
       	 SetDlgItemText (hWndDlg,SV_MAXVAL,str);  


         SetDlgItemText(hWndDlg,SV_FIELD_NAME,(LPCSTR)&CurTheme->Field.name); 
         SetDlgItemText(hWndDlg,IDC_SQL,(LPCSTR)CurTheme->SQL);
         
         SetDlgItemInt (hWndDlg,IDC_TITSIZE,CurTheme->TitleHeight,TRUE);
         SetDlgItemInt (hWndDlg,IDC_INMARGIN,CurTheme->InnerMargin,TRUE);
         SetDlgItemInt (hWndDlg,IDC_MARGIN,CurTheme->Margin,TRUE);
         SetDlgItemInt (hWndDlg,IDC_BOXSIZE,CurTheme->ColorsWidth,TRUE);
         if (CurTheme->FieldFun<6)  
		 {
			ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_VALUE),SW_HIDE);
			ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_NAME),SW_SHOW); 
			ShowWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),SW_HIDE); 
		 }
		 else
		 {
			ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_VALUE),SW_SHOW);
			ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_NAME),SW_HIDE);
			ShowWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),SW_SHOW); 
		 }
         SetDlgItemText (hWndDlg,SV_FIELD_VALUE,CurTheme->Value);

		 break; /* End of WM_INITDIALOG                                 */

    case WM_CLOSE:
         /* Closing the Dialog behaves the same as Cancel               */
         PostMessage(hWndDlg, WM_COMMAND, IDCANCEL, 0L);
         break; /* End of WM_CLOSE                                      */

    case WM_COMMAND:
         switch(wParam)
           {
            case IDC_THEME_HELP:
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_CONTEXT,IDD_SV_THEME);
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_CONTEXTPOPUP,IDD_SV_THEME);
              WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_PARTIALKEY,(DWORD)"Theme Editing");
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_KEY,(DWORD)"Theme Editing");
              //  WinHelp(hWndDlg,"gwizhelp\\gwizhelp.hlp",HELP_MULTIKEY,(DWORD)"Theme Editing");
               break;
            case IDCANCEL:
                 /* Ignore data values entered into the controls        */
                 /* and dismiss the dialog window returning FALSE       */
                 CloseThemeDataFile(TRUE);   
		    	 pSaveTheme = (LPTHEME)GlobalLock (hSaveTheme);
		    	 *CurTheme = *pSaveTheme;  
		    	 CurTheme->hThemeDB = 0;
		    	 GSSiGlobUlFree (&hSaveTheme); 
			     SetCurView (SaveView);
                 DestroyFieldList ();
    			 DoPaint = TRUE;
                 EndDialog(hWndDlg, FALSE);
                 break;  
                 
            case SV_CONTENTS_LIST2:
               { 
              	switch(HIWORD(lParam))
                {
	                 case CBN_DROPDOWN:
	                 {
	                 	short	idesc, iparent, i, j;
                	    LPSTR	pTAGList;  
                	    HANDLE	hPar=GSSiGlobAlloc (1015,GMEM_MOVEABLE,4096);
                	    short	nPar=0;
                	    LPSHORT	pPar, pPar2; 
                	    char	str[64], SymbolName[34];
                	    BOOL	IsPar;  
                	    LPVIEWPORT	SaveVP=CurView;
	                 	 
		                CurTheme->TargetViewport=SendDlgItemMessage(hWndDlg,SV_TARGET,
											       CB_GETCURSEL,NULL,NULL)+1;
			            SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST2,CB_RESETCONTENT,NULL,NULL); 
			            if (!CurTheme->TargetViewport) break;
			            
			            if (CurTheme->ID != GF_BOUNDS_DISPLAY_THEME)
			            { 
				            SetViewport (CurTheme->TargetViewport);
				            SelectVisList (FALSE);
				        }
		        		//GetVisList (hWndDlg,0,-SV_CONTENTS_LIST,0,-1);
	 	                //SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST,CB_ADDSTRING,NULL,(LPARAM)((LPSTR) "(ALL)"));
				    	for (idesc=1;idesc<3201;idesc++) 
				    	{
							if (CurView->CurVisType[idesc])
							{   char	SymbolName[34];
							
								GetSymbolName (idesc,SymbolName,&iparent,0,&IsPar);
								if (SymbolName[0] && !IsPar)
								{
			 	                	SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST2,CB_ADDSTRING,NULL,(LPARAM)((LPSTR) SymbolName));
			 	                	pPar = (LPSHORT)GlobalLock (hPar);
			 	                	for (i=0;i<nPar;i++,pPar++)
			 	                		if (iparent == *pPar)
			 	                			goto GotPar;
			 	                	*pPar = iparent;
			 	                	nPar++;
			 	            GotPar: GlobalUnlock (hPar);  
			 	                }
		 	                }
						}     
 	                	pPar = (LPSHORT)GlobalLock (hPar);
 	                	for (i=0;i<nPar;i++,pPar++)
 	                	{
							GetSymbolName (*pPar,SymbolName,&iparent,0,NULL);
							if (SymbolName[0])
							{   
								sprintf (str,"(%s)",SymbolName);
			 	                SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST2,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)str));
		 	                	pPar2 = (LPSHORT)GlobalLock (hPar);
		 	                	for (j=0;j<nPar;j++,pPar2++)
		 	                		if (iparent == *pPar2)
		 	                			goto GotPar2;
		 	                	*pPar2 = iparent;
		 	                	nPar++;
		 	            GotPar2: GlobalUnlock (hPar); 
		 	                }
		 	            } 
 	            		GSSiGlobUlFree (&hPar);
                	    
                	    if (CurView->hTAGList)
                	    {
							pTAGList = GlobalLock(CurView->hTAGList);
							while (*pTAGList)
							{   
								char	tag[16];
								
								sprintf (tag,"-%s",pTAGList);
		 	               		SendDlgItemMessage (hWndDlg,SV_CONTENTS_LIST2,CB_ADDSTRING,NULL,(LPARAM)((LPSTR)tag));
								pTAGList+=10;
							}
							GlobalUnlock (CurView->hTAGList);
						}
                        SetCurView (SaveVP);
					}
				    break; 
				}
                break;
               }
            
            case SV_FIELDFUNCTION:
              switch(HIWORD(lParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
                  Choice=SendDlgItemMessage(hWndDlg,SV_FIELDFUNCTION,
									       CB_GETCURSEL,NULL,NULL);
				  if (Choice<6)
		          {
		         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_VALUE),SW_HIDE);
		         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_NAME),SW_SHOW); 
		         	ShowWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),SW_HIDE); 
		          }
		          else
		          {
		         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_VALUE),SW_SHOW);
		         	ShowWindow (GetDlgItem(hWndDlg,SV_FIELD_NAME),SW_HIDE);
		         	ShowWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),SW_SHOW); 
		          }
				  break;
			  }
			  break;
			   
            case IDC_SHOW_FIELDS: 
            	 
            	 CensusValueField = SV_FIELD_VALUE;
            	 CensusValueWnd = hWndDlg;
            	 CensusTitleField = SV_TITLE;
            	 DisplayFieldList (hWndDlg,CurTheme->hThemeDB,NULL,0);
            	 CensusValueField = 0;
            	 CensusTitleField = 0;
                 break;
                      
            case SV_ITEM_TYPE:
              switch(HIWORD(lParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
                  Choice=SendDlgItemMessage(hWndDlg,SV_ITEM_TYPE,
									       CB_GETCURSEL,NULL,NULL);
				  CurTheme->FieldCorrection=0;
				  SetFieldCorrectionOpts (hWndDlg,Choice); 
				  break;
			  }
			  break; 
			  
			case IDC_TITTEXTFONT:
				break;
			 
            case SV_FIELD_NAME:
              switch(HIWORD(lParam))
              {
               case CBN_DBLCLK:
               case CBN_SELCHANGE:
            
                  Choice=SendDlgItemMessage(hWndDlg,SV_FIELD_NAME,
									       CB_GETCURSEL,NULL,NULL); 
				  if(Choice >= 0)
			      {					        
		               SendDlgItemMessage(hWndDlg,SV_FIELD_NAME,CB_GETLBTEXT,
		         		  		    Choice,(DWORD)&CurTheme->Field.name); 
		           }
		           break;  
		      }// end of the switch       		  		    
              break;
            
            case IDC_EDITRANGES:
		       	 GetDlgItemText (hWndDlg,SV_NUM_CLASSES,str,3);
		       	 CurTheme->NumDesiredClass = atoi (str);
            	 GetDlgItemText (hWndDlg,IDC_ROUND_TO,str,10); 
            	 Strip (str,',');
            	 CurTheme->RoundTo = atof(str);
		         {
			          FARPROC lpfnCLASS_RANGESMsgProc;
			          lpfnCLASS_RANGESMsgProc = MakeProcInstance((FARPROC)CLASS_RANGESMsgProc, hInst);
			          DialogBox(hInst, (LPSTR)"CLASS_RANGES", hWndDlg, lpfnCLASS_RANGESMsgProc);
			          FreeProcInstance(lpfnCLASS_RANGESMsgProc);
		         }
              	break;
              
            case SV_CB_EVENRANGES:
       	 	case SV_CB_PERCENTILES:
			   	ShowWindow (GetDlgItem(hWndDlg,IDC_EDITRANGES),SW_HIDE);
				CurTheme->Recompute=TRUE;
			   	break;
			   	
			case SV_CB_MANUAL:
			   	ShowWindow (GetDlgItem(hWndDlg,IDC_EDITRANGES),SW_SHOW);
				CurTheme->Recompute=FALSE;
       	 		SendDlgItemMessage (hWndDlg,SV_CB_RECOMPUTE,BM_SETCHECK,FALSE,0L);
			  	break;
            
            case SV_DISPERSE:
				SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_SETCHECK,FALSE,0L);
                break;
                
            case SV_ACCUMULATE:
				SendDlgItemMessage (hWndDlg,SV_DISPERSE,BM_SETCHECK,FALSE,0L);
            	if (SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_GETCHECK,0,0L))
		        {
			    	FARPROC lpfnACCUMPOINTOPTMsgProc; 
				    	
					DoPaint = FALSE; 
			        lpfnACCUMPOINTOPTMsgProc = MakeProcInstance((FARPROC)ACCUMPOINTOPTMsgProc, hInst);
			        DialogBox(hInst, (LPSTR)"ACCUMPOINTOPT", hWndDlg, lpfnACCUMPOINTOPTMsgProc);
			        FreeProcInstance(lpfnACCUMPOINTOPTMsgProc);
					DoPaint=TRUE;
				}
                break;
                
            case IDC_OPEN_DB:
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SETSQL),TRUE);
                 EnableWindow (GetDlgItem(hWndDlg,IDC_SHOW_FIELDS),TRUE);
                 break;  
            
            case IDC_SAVE_THEME:  
 
            case IDOK: 
            	 if (CurTheme->DataFileType == UMIFS_DATAFILE || CurTheme->DataFileType == GMCENSUS_DATAFILE)
            	 	GetDlgItemText (hWndDlg,SV_DATABASE_LIST,CurTheme->DataFile,sizeof(CurTheme->DataFile));
                 CurTheme->DataType=SendDlgItemMessage(hWndDlg,SV_ITEM_TYPE,
									       CB_GETCURSEL,NULL,NULL);
            	 GetDlgItemText (hWndDlg,SV_FIELD_VALUE,CurTheme->Value,sizeof(CurTheme->Value));
                 CurTheme->FieldFun=SendDlgItemMessage(hWndDlg,SV_FIELDFUNCTION,
									       CB_GETCURSEL,NULL,NULL);
                 CurTheme->FieldCorrection=SendDlgItemMessage(hWndDlg,SV_CORRECTION,
									       CB_GETCURSEL,NULL,NULL); 
				 CurTheme->PointInAreaPointThemeVPID = SendDlgItemMessage(hWndDlg,IDC_POINTTHEME_LIST,
									       CB_GETCURSEL,NULL,NULL)+1;
            	 GetDlgItemText (hWndDlg,IDC_ROUND_TO,str,10); 
            	 Strip (str,',');
            	 CurTheme->RoundTo = atof(str);
  				 CurTheme->Field.type = SQL_VARCHAR;
                 Choice=SendDlgItemMessage(hWndDlg,SV_FIELD_NAME,
									       CB_GETCURSEL,NULL,NULL);
		         SendDlgItemMessage(hWndDlg,SV_FIELD_NAME,CB_GETLBTEXT,
		         		  		    Choice,(DWORD)&CurTheme->Field.name);
                 
                 CurTheme->NumCols=SendDlgItemMessage(hWndDlg,SV_COLUMNS,CB_GETCURSEL,NULL,NULL);
         		 CurTheme->TitleHeight=GetDlgItemInt (hWndDlg,IDC_TITSIZE,&Error,TRUE);
         		 CurTheme->InnerMargin=GetDlgItemInt (hWndDlg,IDC_INMARGIN,&Error,TRUE);
         		 CurTheme->Margin=GetDlgItemInt (hWndDlg,IDC_MARGIN,&Error,TRUE);
         		 CurTheme->ColorsWidth=GetDlgItemInt (hWndDlg,IDC_BOXSIZE,&Error,TRUE);
                 CurTheme->ColorScheme = SendDlgItemMessage(hWndDlg,SV_COLOR_SCHEME,LB_GETCURSEL,NULL,NULL);
                 SetThemeColorsFromScheme ();
       	 		 GetDlgItemText (hWndDlg,SV_TITLE,CurTheme->Title,256);
       	 		 GetDlgItemText (hWndDlg,IDC_SQL,CurTheme->SQL,256);
            	 CurTheme->DisplayScatterDiagram = SendDlgItemMessage (hWndDlg,SV_DISPLAY_DATAPOINTS,BM_GETCHECK,0,0L);  
            	 CurTheme->AddCommas = SendDlgItemMessage (hWndDlg,SV_INSERT_COMMAS,BM_GETCHECK,0,0L); 
            	 CurTheme->ZeroBased = SendDlgItemMessage (hWndDlg,SV_CB_ZEROBASED,BM_GETCHECK,0,0L); 
            	 CurTheme->ShowValue = SendDlgItemMessage (hWndDlg,SV_DISPLAY_VALUE,BM_GETCHECK,0,0L); 
            	 CurTheme->DelayTextDisplay = SendDlgItemMessage (hWndDlg,SV_DELAY_VALUE,BM_GETCHECK,0,0L); 
            	 CurTheme->DispersePoints = 0;
            	 if (SendDlgItemMessage (hWndDlg,SV_DISPERSE,BM_GETCHECK,0,0L))
            	 	CurTheme->DispersePoints = 1;
            	 if (SendDlgItemMessage (hWndDlg,SV_ACCUMULATE,BM_GETCHECK,0,0L))
            	 	CurTheme->DispersePoints = 2;
            	 CurTheme->WantDataPass = CurTheme->Recompute = SendDlgItemMessage (hWndDlg,SV_CB_RECOMPUTE,BM_GETCHECK,0,0L); 
            	 CurTheme->ZeroIsMissing = SendDlgItemMessage (hWndDlg,SV_CB_ZEROASMISS,BM_GETCHECK,0,0L); 
            	 CurTheme->MarkInvalid = SendDlgItemMessage (hWndDlg,IDC_MARK_INVALID,BM_GETCHECK,0,0L); 
            	 CurTheme->SkipInvalid = SendDlgItemMessage (hWndDlg,IDC_SKIP_INVALID,BM_GETCHECK,0,0L); 

		       	 if (SendDlgItemMessage (hWndDlg,SV_CB_EVENRANGES,BM_GETCHECK,0,0L)) CurTheme->ClassType =1;
		       	 if (SendDlgItemMessage (hWndDlg,SV_CB_PERCENTILES,BM_GETCHECK,0,0L)) CurTheme->ClassType =2;
		       	 if (SendDlgItemMessage (hWndDlg,SV_CB_MANUAL,BM_GETCHECK,0,0L)) CurTheme->ClassType =3;
		       	 if (CurTheme->ClassType!=1) CurTheme->DisplayScatterDiagram = FALSE;
		       	 GetDlgItemText (hWndDlg,SV_NUM_CLASSES,str,3);
		       	 CurTheme->NumDesiredClass = atoi (str);
		       	 if (CurTheme->ClassType == 3) 
		       	 	CurTheme->NumClass = CurTheme->NumDesiredClass;
		       	 GetDlgItemText (hWndDlg,SV_MAXVAL,str,16);
		       	 Truncate (str);
		       	 if (str[0])
		       	 	CurTheme->YLimit = atof (str); 
		       	 else
		       	 	CurTheme->YLimit = DBL_MAX;
		       	 
		       	 if (wParam == IDC_SAVE_THEME)
		       	   	SaveCurTheme(hWndDlg);
		       	 else
		       	 {
			       	 SetCurView (SaveView);
	                 CloseThemeDataFile(TRUE);
		    	 	 GSSiGlobFree (&hSaveTheme);
				     DoPaint = FALSE;
	                 EndDialog(hWndDlg, TRUE); 
	             }
                 break;

          /*  case SV_DATA_FILE_ICON:
            	 CloseThemeDataFile(TRUE);
            	 SetFilterString (IDS_FILTERDB);

    no longer   if (GetOpenFileCD (hWndDlg,CurTheme->DataFile,"C:"))
    use this    {
           	    	 SetDlgItemText(hWndDlg,SV_DATA_FILE,CurTheme->DataFile);
           	    	 if (_fstrstr (CurTheme->DataFile,".GWD")) CurTheme->DataFileType = UMIFS_DATAFILE;
           	    	 if (_fstrstr (CurTheme->DataFile,".DBF")) CurTheme->DataFileType = FOXPRO_DATAFILE;
           	    	 if (_fstrstr (CurTheme->DataFile,".MDB")) CurTheme->DataFileType = MSACCESS_DATAFILE;
           	    	 goto LoadFields;
                 }  */


           }
         break;    /* End of WM_COMMAND                                 */

    default:
{
#if ENABLETRACE
GSSiExitProg (1247);
#endif
        return FALSE;
}
   }
{
#if ENABLETRACE
GSSiExitProg (1247);
#endif
 return TRUE;
}
#if ENABLETRACE
}
#endif
}

void DisplayTimeLegend (short From)
#if ENABLETRACE
{GSSiEnterProg (1292);
#endif
{   int		xmargin, ymargin;
	long	h, w;
	int		width, height,x, y, fHeight, MaxTextWidth, twidth, i, j;
	HBRUSH	BkBrush;
	int		iclass;
	RECT	ClassColorBox;
	HFONT	hfont, hfontOld=0, hfont2;
	DWORD	TextExtent;
	char	Text[256], Title[256];
	char	Val1[32], Val2[32];
	long	TotCount;
	float	Pct;
	int		MinFontHeight=2, left,right, tWidth;
	int		inc;
	LPSTR	lpText;
	char	lpLine[256];  
	long	Counts[MAX_THEME_CLASSES][MAX_THEME_CLASSES], nMax;
	HANDLE	hBTX, hBTY;
	short	nClassX, nClassY, VPNumX=CurTheme->DataFileType, VPNumY=CurTheme->DataType;
	short	pos=BT_FIRST, cval; 
	COLORREF	Color;
	THEMEHIGHLIGHTKEY	ThemeHighlightKey;
	THEMEHIGHLIGHTDATA	ThemeHighlightData;    
	double	xinc, yinc;
	RECT	Rect=CurView->DrawRect;  
	HCURSOR	hcurSave;    
	BOOL	Opened1=FALSE, Opened2=FALSE;
	time_t	systime; 
	struct tm	tmtime;      
	LPDOUBLE	pTimeD;  
	long	SecondsFromStart, SecondsFromMidnite, DaysFromStart, SecondsInDay = 60L*60L*24L;           
	short	Type=1;
	
#define SCATTER_DIAGRAM	1
	
    
    SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn (FALSE,FALSE);
    SelectClipRgn (CurView->hDC,CurView->hRgn);
    GSSiDeleteObject(&CurView->hRgn); 
	InflateRect (&Rect,1,1);
    FillRectPoly (CurView->hDC,&Rect,WindowColor);
    if (From < 2)
    	goto Exit;
    
    _fstrcpy (Title,CurTheme->Title);
    ExpandText (Title);
    
	CurTheme->Rect=PctRect (CurView->DrawRect,-CurTheme->Margin);
	xmargin = (((long)CurTheme->Rect.right - CurTheme->Rect.left) * CurTheme->InnerMargin) / 100;
	ymargin = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->InnerMargin) / 100;
	h = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->TitleHeight) / 100;
	CurTheme->TitleBox.top = CurTheme->Rect.top + ymargin;
	CurTheme->TitleBox.bottom = CurTheme->TitleBox.top + h;
	CurTheme->TitleBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->TitleBox.right = CurTheme->Rect.right - xmargin;
	CurTheme->InfoBox.top = CurTheme->TitleBox.bottom + ymargin;
	CurTheme->InfoBox.bottom = CurTheme->Rect.bottom - ymargin;
	CurTheme->InfoBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->InfoBox.right = CurTheme->Rect.right - ymargin;
	CurTheme->ScatterBox = CurTheme->InfoBox;
	CurTheme->ScatterBox.right = CurTheme->InfoBox.right;

	FillRectPoly (CurView->hDC,&CurTheme->Rect,CurTheme->BGColor);
	FillRectPoly (CurView->hDC,&CurTheme->InfoBox,RGB(255,255,255));
	FillRectPoly (CurView->hDC,&CurTheme->TitleBox,CurTheme->TitleBoxBG);

	width = CurTheme->TitleBox.right - CurTheme->TitleBox.left;
	fHeight = CurTheme->TitleHeight+1;
	twidth = INT_MAX;
	while (twidth>width && fHeight>2)
	{   
		fHeight--;
		CurTheme->TitleFont.lfHeight = -MulDiv(fHeight,
											   GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
		hfont = CreateFontIndirect((PLOGFONT)&CurTheme->TitleFont);
		hfontOld = SelectObject(CurView->hDC, hfont);
		TextExtent = GetTextExtent (CurView->hDC,Title,_fstrlen(Title));
		lpText = Title;
		twidth = 0;	
	    while (NextLine (&lpText,lpLine,0))
	    {
			TextExtent = GetTextExtent (CurView->hDC,lpLine,_fstrlen(lpLine));
			twidth = max (twidth,LOWORD (TextExtent));
		}  
		SelectObject(CurView->hDC, hfontOld); 
		DeleteObject(hfont);
	}
	x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - twidth)/2;
	CurTheme->TitleFont.lfHeight = -MulDiv(CurTheme->TitleHeight-2,
										   GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
	hfont = CreateFontIndirect((PLOGFONT)&CurTheme->TitleFont);
	hfontOld = SelectObject(CurView->hDC, hfont); 

    SetTextColor (CurView->hDC,0); 
	lpText = Title;	
	y = CurTheme->TitleBox.top + 1;
    while (NextLine (&lpText,lpLine,0))
    {
		TextExtent = GetTextExtent (CurView->hDC,lpLine,_fstrlen(lpLine));
		width = LOWORD (TextExtent);
		x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - width)/2;
		TextOut(CurView->hDC, x, y, lpLine,_fstrlen(lpLine));  
		y+= HIWORD (TextExtent);
	}
	SelectObject(CurView->hDC, hfontOld); 
	DeleteObject(hfont);
    
    if (Type == SCATTER_DIAGRAM)
    {   
    	long	TotDays = (TimeRangeEnd - TimeRangeBeg)/SecondsInDay;
    	POINT	WinPoint;
    	short	isym;
    	
		if (!OpenThemeHighlightFile (BT_READ))
			goto Exit;
    	isym = GetDictSymbolNumber ("CIRCLE");
		width = CurTheme->InfoBox.right - CurTheme->InfoBox.left;
		height = CurTheme->InfoBox.top - CurTheme->InfoBox.bottom;
	    _fmemset (Counts,0,MAX_THEME_CLASSES*MAX_THEME_CLASSES*sizeof(long));
		while (!BT_FIND (CurTheme->hHighlightFile,(LPSTR)&ThemeHighlightKey,pos,BT_ANY,(LPSTR)&ThemeHighlightData))
	    {   
	    	pos = BT_NEXT;    
	    	pTimeD = (LPDOUBLE)ThemeHighlightData.Value;  
	    	systime = *pTimeD;  
	    	SecondsFromStart = systime - TimeRangeBeg;
	    	DaysFromStart = SecondsFromStart/SecondsInDay;
	    	SecondsFromMidnite = SecondsFromStart % SecondsInDay; 
	    	WinPoint.x = CurTheme->InfoBox.left + (DaysFromStart*width)/TotDays;
	    	WinPoint.y = CurTheme->InfoBox.bottom + (SecondsFromMidnite*height)/SecondsInDay;
			DisplayPointItem (CurView->hDC,WinPoint,5,0,isym,NULL);
//		tmtime = *localtime (&systime); 
	    }  
		GSSiSetCursor(hcurSave); 
		CloseThemeHighlightFile (); 
    }
    else
    {
	    nMax = 0;
	    for (i=0;i<nClassX;i++)
	    	for (j=0;j<nClassY;j++)
	    		nMax = max (nMax,Counts[i][j]); 
	    if (!nMax)
	    	goto Exit;
		xinc = ((double)CurTheme->InfoBox.right - CurTheme->InfoBox.left)/nClassX;
		yinc = ((double)CurTheme->InfoBox.bottom - CurTheme->InfoBox.top)/nClassY;
	    for (i=0;i<nClassX;i++)
	    {
	    	for (j=0;j<nClassY;j++)
	    	{
				ClassColorBox.left = IDNINT (CurTheme->InfoBox.left + xinc * i);
				ClassColorBox.right = IDNINT (CurTheme->InfoBox.left + xinc * (i+1));
				ClassColorBox.bottom = IDNINT (CurTheme->InfoBox.bottom - yinc * j);
				ClassColorBox.top = IDNINT (CurTheme->InfoBox.bottom - yinc * (j+1)); 
				cval = ((nMax-Counts[i][j]) * 255) / nMax;
				Color = RGB(cval,cval,cval); 
				FillRectPoly (CurView->hDC,&ClassColorBox,Color); 
				if (CurTheme->DisplayCount)
				{
				    SetBkMode(CurView->hDC, TRANSPARENT);  
					CurTheme->ClassFont2.lfHeight = -(ClassColorBox.bottom - ClassColorBox.top - 2* DeviceToScreenFactor); 
					hfont2 = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont2);
			        hfontOld = SelectObject(CurView->hDC, hfont2);
					sprintf (Text,"%ld",Counts[i][j]); 
					TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
					width = LOWORD (TextExtent);
					x = ClassColorBox.left + ((ClassColorBox.right - ClassColorBox.left) - width)/2; 
					y = ClassColorBox.top + 1* DeviceToScreenFactor; 
					if (cval >150)
					    SetTextColor (CurView->hDC,0); 
					else
					    SetTextColor (CurView->hDC,RGB(255,255,255));  
					TextOut(CurView->hDC, x, y, Text,_fstrlen(Text)); 
					SelectObject(CurView->hDC, hfontOld); 
					DeleteObject(hfont2);
				}
	    	}
	    }
	    SetTextColor (CurView->hDC,0); 
		fHeight = 12 * DeviceToScreenFactor; 
		CurTheme->ClassFont1.lfHeight = fHeight;
		hfont = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont1);
		hfontOld = SelectObject(CurView->hDC, hfont);
		y = ClassColorBox.top - 2*yinc/3 - 2;
	    for (i=0;i<nClassX;i++)
	    {   
	    	LPSTR	pPar;
	    	 
	    	_fstrcpy (Text,CurTheme->ClassBM[i]);
	    	if ((pPar=_fstrrchr (Text,'(')))
	    		*pPar = 0;
			left = IDNINT (CurTheme->InfoBox.left + xinc * i);
			right = IDNINT (CurTheme->InfoBox.left + xinc * (i+1));
	       	TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
	       	tWidth = LOWORD (TextExtent); 
	       	x = (left + right) / 2 - tWidth/2;
			TextOut(CurView->hDC, x,y,Text,_fstrlen(Text));
	    }
		SelectObject(CurView->hDC, hfontOld);
		DeleteObject(hfont);
	} 
Exit:
	RestoreDC (CurView->hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (1292);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  

void BeginDelayedText (HFILE FidDelayedText)
{   
  	if (CurTheme->FidDelayedText > 0)
  		CloseAndDeleteFile (&CurTheme->FidDelayedText); 
	if (!CurTheme->DelayTextDisplay)
		return;
	CurTheme->FidDelayedText = OpenTempNamedFile ();
	return;
}

void DisplaySVThemeLegend(short From)
#if ENABLETRACE
{GSSiEnterProg (169);
#endif
{   int		xmargin, ymargin, i;
	long	h, w;
	int		height, width, x, y, fHeight, MaxTextWidth, MaxTextHeight, twidth, theight;
	HBRUSH	BkBrush;
	int		iclass;
	RECT	ClassColorBox;
	HFONT	hfont=0, hfontOld=0, hfont2=0;
	DWORD	TextExtent;
	char	Text[256], Title[256], ExpLine[256];
	char	Val1[32], Val2[32];
	long	TotCount,MaxCount=0;    
	double	MaxCountD=0, TotCountD=0;
	float	Pct;
	int		MinFontHeight=3;
	int		NumCols=1, MaxClassPerCol, icol, irow;
	double	colw;
	LPSTR	lpText;
	char	lpLine[256];        
	LPTHEME	SavePCTTheme; 
	LPVIEWPORT	SaveVP=CurView;
	double	MaskSize, fontfactor=1, ClassMin,ClassMax;  
	short	SaveFF1=CurTheme->ClassFont1.lfHeight;
	short	SaveFF2=CurTheme->ClassFont2.lfHeight;
	RECT	Rect=CurView->DrawRect;  
	short	BeginCount, CountTextWidth,CountTextHeight=0, usedclass, classinc;   
	RECT	UsedBox[MAX_THEME_CLASSES];    
	double	LineSymFactor = BaseDistToWinDist;
	short	nAutoLines = 0, nal;
	short	tw, inc,ii;
	LPSTR	pAt, pCarrot, pDesc;
	LPSTR	pSemiColon; 
	int		RegionType;  
	BOOL	Circular=FALSE;
	POINT	Point;  
	RECT	BMRect;
	char	IconFile[128];
	char	CheckMarkSymbol[32]="check1.bmp";
    
    if (From == 4)
    	DrawDelayedText ();
    if (!CurTheme->AddCommas)//UseCheckmark)
    	*CheckMarkSymbol = 0;
    SaveDC (CurView->hDC);
    SetDisplayMode (CurView->hDC, GF_TEXTMODE);
    SetBkMode(CurView->hDC, TRANSPARENT);
    if (CurTheme->DisplayDistance) 
    	GetThemeClassDistance (-1);
    GSSiDeleteObject(&CurView->hRgn);
    CurView->hRgn = CreateVPRgn (FALSE,FALSE);
    RegionType = SelectClipRgn (CurView->hDC,CurView->hRgn);
    GSSiDeleteObject(&CurView->hRgn); 
//  	SelectClipRgn (CurView->hDC,NULL);
//	InflateRect (&Rect,1,1);  
	if (RegionType == NULLREGION)
		goto Exit;
    if (CurTheme->ID == GF_GRAPHICS_FUNCTION_THEME) 
    {
    	switch (From)
    	{   
    		case 0:
    		case 2:
    		case 4:
    			goto Exit;
    		default:
    			break;
    	}
    	FillThemeFromGFMenu (); 
	    FillRectPoly (CurView->hDC,&Rect,WindowColor);
    }
    else
    {
	    FillRectPoly (CurView->hDC,&Rect,WindowColor);
    	if (From < 2)
    		goto Exit;
    }
    if (!CurTheme->NumClass)
    	goto Exit; 
    if (CurTheme->ClassFont1.lfHeight)  
    	fontfactor = (double)CurTheme->ClassFont2.lfHeight/(double)CurTheme->ClassFont1.lfHeight;
    TotCount = 0;
	for (iclass=0;iclass<CurTheme->NumClass;iclass++) 
	{
		MaxCount = max (MaxCount,CurTheme->ClassCount[iclass]);
		TotCount += CurTheme->ClassCount[iclass];
		if (CurTheme->DisplayDistance)
		{ 
			MaxCountD = max (MaxCountD,GetThemeClassDistance(iclass));
			TotCountD += GetThemeClassDistance(iclass);
		}
	}
	if (!TotCount && CurTheme->ClearIfNoCount)
	{   
       	DisplayTAGs2 (CurView->hDC,2,0); 
        goto Exit;
	}
	if (CurTheme->NumCols == MAX_THEME_CLASSES) 
		Circular = TRUE;
	else
    	NumCols = max (1,CurTheme->NumCols + 1);
    SetViewport (CurTheme->TargetViewport); 
    MaskSize = SetMaskSizeVar ();
	SetCurView ( SaveVP); 
//    CurView->hRgn = CreateVPRgn (FALSE,FALSE);
//    SelectClipRgn (CurView->hDC,CurView->hRgn);
//    DeleteObject(CurView->hRgn); 
	CurTheme->Rect=PctRect (CurView->DrawRect,-CurTheme->Margin);
	xmargin = (((long)CurTheme->Rect.right - CurTheme->Rect.left) * CurTheme->InnerMargin) / 100;
	ymargin = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->InnerMargin) / 100;
	xmargin = min (xmargin,ymargin);
	ymargin = xmargin; 
	CurTheme->ActualXMargin = xmargin/2;
	h = (((long)CurTheme->Rect.bottom - CurTheme->Rect.top) * CurTheme->TitleHeight) / 100;
	CurTheme->TitleBox.top = CurTheme->Rect.top + ymargin;
	CurTheme->TitleBox.bottom = CurTheme->TitleBox.top + h;
	CurTheme->TitleBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->TitleBox.right = CurTheme->Rect.right - xmargin;
	CurTheme->InfoBox.top = CurTheme->TitleBox.bottom + ymargin;
	CurTheme->InfoBox.bottom = CurTheme->Rect.bottom - ymargin;
	CurTheme->InfoBox.left = CurTheme->Rect.left + xmargin;
	CurTheme->InfoBox.right = CurTheme->Rect.right - ymargin;
	CurTheme->ScatterBox = CurTheme->InfoBox;
	w = (((long)CurTheme->InfoBox.right - CurTheme->InfoBox.left) * CurTheme->ScatterWidth) / 100;
	if (!CurTheme->DisplayScatterDiagram) w=0;
	CurTheme->ScatterBox.right = CurTheme->InfoBox.left + w;

	FillRectPoly (CurView->hDC,&CurTheme->Rect,CurTheme->BGColor);
	FillRectPoly (CurView->hDC,&CurTheme->InfoBox,CurTheme->IBBGColor);
	FillRectPoly (CurView->hDC,&CurTheme->TitleBox,CurTheme->TitleBoxBG);
    CurTheme->RadiusPoint = Point = RectMid (&CurTheme->InfoBox); 
    Point.x = CurTheme->InfoBox.right;
    CurTheme->Radius = idist (CurTheme->RadiusPoint,Point);
GetTitleSize: 
	width = CurTheme->TitleBox.right - CurTheme->TitleBox.left - xmargin;
	fHeight = h;
	twidth = theight = INT_MAX;
	while ((twidth>width || theight>h) && fHeight>4)
	{   
		fHeight--;
//		CurTheme->TitleFont.lfWeight=FW_BOLD;
		CurTheme->TitleFont.lfHeight = fHeight+1; 
		if (hfontOld && hfont &&(hfontOld == hfont))
			ii=1;
		if (hfontOld) SelectObject(CurView->hDC, hfontOld);
		if (hfont) DeleteObject(hfont);
		hfont = CreateFontIndirect((PLOGFONT)&CurTheme->TitleFont);
		hfontOld = SelectObject(CurView->hDC, hfont);
	    _fstrcpy (Title,CurTheme->Title);
	    ExpandText (Title);
		TextExtent = GetTextExtent (CurView->hDC,Title,_fstrlen(Title));
		lpText = Title;
		twidth = 0;	
		theight = 0;
		nal = nAutoLines;
	    while (NextLine (&lpText,lpLine,nal))
	    {   
	    	_fstrcpy(ExpLine,lpLine);
	    	ExpandText (ExpLine);
	    	Truncate (ExpLine);
			TextExtent = GetTextExtent (CurView->hDC,ExpLine,_fstrlen(ExpLine));
			twidth = max (twidth,LOWORD (TextExtent)); 
			theight += HIWORD (TextExtent);  
			if (nal)
				nal--;
		}
	}
	if (NumCharInString (Title,' ') > 1 && !_fstrchr (Title,'\r') && !nAutoLines && fHeight < h/3) 
	{
		nAutoLines = 3;
		goto GetTitleSize; 
	} 
	if (NumCharInString (Title,' ') > 0 && !_fstrchr (Title,'\r') && nAutoLines==3 && fHeight < h/2) 
	{
		nAutoLines = 2;
		goto GetTitleSize; 
	} 
	SetTextColor (CurView->hDC,CurTheme->TitleTextColor);
	x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - twidth)/2;
	y = CurTheme->TitleBox.top + ((CurTheme->TitleBox.bottom - CurTheme->TitleBox.top) - theight)/2;
    _fstrcpy (Title,CurTheme->Title);
    ExpandText (Title);
	lpText = Title;
	nal = nAutoLines;
    while (NextLine (&lpText,lpLine,nal))
    {
    	_fstrcpy(ExpLine,lpLine);
    	ExpandText (ExpLine);
    	Truncate (ExpLine);
		TextExtent = GetTextExtent (CurView->hDC,ExpLine,_fstrlen(ExpLine));
		width = LOWORD (TextExtent);
		x = CurTheme->TitleBox.left + ((CurTheme->TitleBox.right - CurTheme->TitleBox.left) - width)/2;
		TextOut(CurView->hDC, x, y, ExpLine,_fstrlen(ExpLine));  
		y+= HIWORD (TextExtent);
		if (nal)
			nal--;
	}

	if (hfontOld)
		SelectObject(CurView->hDC, hfontOld);
	if (hfont)
		DeleteObject(hfont); 
	hfont = hfontOld = 0;
	ThemeDisplayScatterDiagram();

	/* Display the color boxes */
	if (CurTheme->DisplayScatterDiagram)
		inc = 0; /* boxes connect */
	else
		inc = ymargin / 2;
	if (NumCols > 1)
	{
		MaxClassPerCol = CurTheme->NumClass/NumCols;
		if (CurTheme->NumClass % NumCols)
			MaxClassPerCol++;
	}
	else
		MaxClassPerCol = CurTheme->NumClass;
	h = (CurTheme->ScatterBox.bottom - CurTheme->ScatterBox.top) / MaxClassPerCol;    
	colw = (CurTheme->InfoBox.right - CurTheme->InfoBox.left) / (double)NumCols; 
	w = (colw *  CurTheme->ColorsWidth) / 100;
	SavePCTTheme = ComputePCTTheme;
	ComputePCTTheme = 0;
	CreateThemePens (CurTheme,FALSE);
	for (i=0;i<CurTheme->NumClass;i++)
	{   
		if (CurTheme->FillRow)  
		{
			irow =  i / NumCols;
			icol = i - irow * NumCols;
		}
		else
		{
			icol = i / MaxClassPerCol;
			irow = i - icol * MaxClassPerCol;
		}
		ClassColorBox.left = IDNINT (CurTheme->ScatterBox.right+xmargin/2 + (NumCols - icol -1) * colw);
		ClassColorBox.right = ClassColorBox.left + w;
		ClassColorBox.bottom = CurTheme->ScatterBox.bottom-inc/2 - (irow * h); 
		ClassColorBox.top = ClassColorBox.bottom - h + inc;
		CurTheme->ClassClrBox[i] = ClassColorBox;
	}
	if (CurTheme->InvertLegend)
	{
		usedclass = CurTheme->NumClass - 1;    
		classinc = -1;
	}
	else
	{
		usedclass = 0;
		classinc = 1;
	}
	for (i=0;i<CurTheme->NumClass;i++)
	{


		if (CurTheme->InvertLegend)
		{
			iclass = CurTheme->NumClass - i -1;    
		}
		else  
		{
			iclass = i;
		} 
		iclass = i; 
		ClassColorBox = CurTheme->ClassClrBox[usedclass];
	    SetTextColor (CurView->hDC,ColorWOWidth (CurTheme->ClassColor[iclass]));     
	    SetBkColor (CurView->hDC,RGB(255,255,255));
	    if (w && h && (!CurTheme->HideNullClasses || CurTheme->ClassCount[iclass]>0))
	    {        
			if (CurTheme->ID == GF_GRAPHICS_FUNCTION_THEME && *CurTheme->IconLibrary)
	    	{   
	    		short	minhktsize=(ClassColorBox.bottom - ClassColorBox.top)/3;  
	    		
	    		if (*CheckMarkSymbol)
	    			minhktsize = 0;
	    		_fstrcpy (IconFile,CurTheme->IconLibrary);
	    		if ((pSemiColon = _fstrchr (CurTheme->ClassBM[iclass],';')))
	    			*pSemiColon = 0;
	    		if ((pAt = _fstrchr (CurTheme->ClassBM[iclass],'&')))
	    		{   
	    			short bmh;
	    			
	    			BMRect=ClassColorBox;
	    			*pAt++ = 0;
	    			if ((pCarrot = _fstrchr (CurTheme->ClassBM[iclass],'^')))
	    				*pCarrot++;
	    			_fstrcat (IconFile,pAt);
	    			BMRect.bottom -= minhktsize;
	    			bmh = DisplayBMFileInRect (CurView->hDC,IconFile, BMRect,1);//-2);
	    			if (pCarrot)
	    			{   
	    				short	h,w;
	    				if (*pCarrot != '@') //11/5/2005
	    				{  
		    				SetTextColor (CurView->hDC,0);     
		    				SetBkColor (CurView->hDC,RGB(255,255,255));
							SetBkMode(CurView->hDC, TRANSPARENT);  
							CurTheme->ClassFont2.lfHeight = -max (1,((ClassColorBox.bottom - ClassColorBox.top) - bmh -1)); 
							hfont2 = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont2);
					        hfontOld = SelectObject(CurView->hDC, hfont2);  
					        TextExtent = GetTextExtent (CurView->hDC,pCarrot, _fstrlen(pCarrot)); 
							h = HIWORD (TextExtent);  
	                        w = LOWORD (TextExtent);
		        			TextOut(CurView->hDC, (ClassColorBox.right + ClassColorBox.left)/2 - w/2,
		        								  ClassColorBox.bottom-h, pCarrot, _fstrlen(pCarrot)); 
							if (hfontOld && hfont2 &&(hfontOld == hfont2))
								ii=1;
		        			if (hfontOld)
		        				SelectObject(CurView->hDC, hfontOld);
							GSSiDeleteObject(&hfont2);
	                	}
	    			}
	    			*(pAt-1) = '&';  
	    			if (pSemiColon)
	    				*pSemiColon = ';';
	    		}
			}
	    	else if (CurTheme->DataType == THEMEDATATYPE_AREA || 
	    		CurTheme->DataType == THEMEDATATYPE_TEXT || 
	    		CurTheme->DataType == THEMEDATATYPE_PIXEL ||
	    		 !CurTheme->ClassSymbol[iclass])
		    {   
		    	RECT	FactoredRect=ClassColorBox;
		    	
		    	if (CurTheme->FactorLegend)
		    	{
		    		if (!MaxCount)
		    			break; 
		    		if (CurTheme->DisplayDistance)
		    			FactoredRect.right = IDNINT ((GetThemeClassDistance(iclass)/MaxCountD)
		    									 * (FactoredRect.right - FactoredRect.left) +
		    									 	FactoredRect.left);
		    		else
		    			FactoredRect.right = IDNINT (((double)CurTheme->ClassCount[iclass]/MaxCount)
		    									 * (FactoredRect.right - FactoredRect.left) +
		    									 	FactoredRect.left);
		    	}
		    	if (!CurTheme->FactorLegend || CurTheme->ClassCount[iclass])
		    	{  
		    		if (Circular) 
		    		{
		    			HANDLE	hPoints=GSSiGlobAlloc (0,GMEM_MOVEABLE,360*sizeof(POINT));
		    			LPPOINT	Points = (LPPOINT)GlobalLock (hPoints);
		    			short	nPnts;
		    			HBRUSH	OldBrush=SelectObject (CurView->hDC,CurTheme->ClassBrush[iclass]);
		    			HPEN	OldPen = SelectObject (CurView->hDC,GetStockObject(WHITE_PEN));
		    			
		    			nPnts = GetPieSlice (CurTheme->RadiusPoint,CurTheme->Radius,CurTheme->NumClass,iclass,Points);
    					Polygon (CurView->hDC,Points,nPnts);
    					GSSiGlobUlFree (&hPoints); 
    					SelectObject (CurView->hDC,OldBrush); 
    					SelectObject (CurView->hDC,OldPen); 
		    		}
		    		else
		    		{
						FillRect (CurView->hDC,&FactoredRect,CurTheme->ClassBrush[iclass]);
						FrameRect (CurView->hDC,&FactoredRect,GetStockObject(BLACK_BRUSH));
					}  
				}
			}
			else 
			{   short	symnum=CurTheme->ClassSymbol[iclass];
			
			    if (*CurTheme->SymbolFont[0])
			    {
			    	_fmemmove (CurSymbolFont,CurTheme->SymbolFont,sizeof(CurSymbolFont));
			    	symnum = -symnum;
			    } 
			    CurrentType = GF_AREA;
			    nPoly = 0;
				SetThemeElementCharacteristics (iclass);
				DisplaySymInRect (CurView->hDC,symnum,ClassColorBox,LineSymFactor);   
				*CurSymbolFont[0] = 0;
			} 
			if (CurTheme->ClassStatus[iclass])  
				DrawUnSelectedClass (iclass,ClassColorBox);
		}
		if (!CurTheme->CompressNullClasses || !CurTheme->HideNullClasses || CurTheme->ClassCount[iclass]>0) 
			usedclass += classinc;
	}
	if (hfontOld)
		SelectObject(CurView->hDC, hfontOld);
	if (hfont2)
		DeleteObject(hfont2); 
	hfont2 = hfontOld = 0;
    DestroyThemePens (CurTheme); 
    ComputePCTTheme = SavePCTTheme;
	fHeight = h-inc;
	width = colw - (CurTheme->ClassClrBox[0].right - CurTheme->ClassClrBox[0].left) - xmargin;
	height = CurTheme->ClassClrBox[0].bottom - CurTheme->ClassClrBox[0].top;
	MaxTextWidth = MaxTextHeight = INT_MAX;
//	CurTheme->ClassFont2.lfItalic=FALSE;
//	CurTheme->ClassFont2.lfWeight=FW_THIN;
	while (MaxTextWidth > width || MaxTextHeight > height)
	{   if (fHeight <= MinFontHeight) goto TooSmall;
		fHeight--;

		MaxTextWidth = MaxTextHeight = 0;
		CurTheme->ClassFont1.lfHeight = fHeight+1;//-MulDiv(fHeight,GetDeviceCaps(CurView->hDC, LOGPIXELSY), 72);
//		CurTheme->ClassFont1.lfWeight=FW_BOLD;

	    TotCount = 0;
		for (iclass=0;iclass<CurTheme->NumClass;iclass++)
		{   
			if (hfontOld && hfont &&(hfontOld == hfont))
				ii=1;
			if (hfontOld) SelectObject(CurView->hDC, hfontOld);
			if (hfont) DeleteObject(hfont);
			hfont = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont1);
			hfontOld = SelectObject(CurView->hDC, hfont);
			TotCount += CurTheme->ClassCount[iclass];  
			SetGlobalValueLong ("%CLASSCOUNT",CurTheme->ClassCount[iclass]);  
			GetClassMinMax (iclass,&ClassMin,&ClassMax);
			SetGlobalValueReal ("%CLASSMIN",ClassMin);
			SetGlobalValueReal ("%CLASSMAX",ClassMax);
			if (CurTheme->ClassType ==3)
			{ 
				_fstrcpy(Text,CurTheme->ClassBM[iclass]); 
	    		if ((pSemiColon = _fstrchr (Text,';')) && CurTheme->ID == GF_GRAPHICS_FUNCTION_THEME)
	    			*pSemiColon = 0;
				if ((pDesc = MatchLev (Text,'/'))) 
				{
					LPSTR pEnd = _fstrchr (pDesc+1,'/');
					if (pEnd)
					{
						short l = _fstrlen (pEnd);
						_fmemmove (pDesc,pEnd+1,l);
					}
				} 
				ExpandText (Text);
			}
			else
			{
		       	_fstrcpy (Val1,ValueConv ((double)CurTheme->ClassMin[iclass],CurTheme->ValConv,CurTheme->RoundTo,CurTheme->AddCommas));
		       	_fstrcpy (Val2,ValueConv ((double)CurTheme->ClassMax[iclass],CurTheme->ValConv,CurTheme->RoundTo,CurTheme->AddCommas));
	            
	            if (_fstrcmp (Val1,Val2))
		       		wsprintf (Text,"%s to %s",Val1, Val2);
		       	else
		       		_fstrcpy (Text,Val1); 
		       	_fstrcpy (CurTheme->ClassBM[iclass],Text);
		    }
			if (CurTheme->AppendCount)
			{
				if (CurTheme->DisplayDistance)
					sprintf (_fstrchr(Text,0),"%.2f",MaxCountD);
			    else
					sprintf (_fstrchr(Text,0)," (%ld)",MaxCount);
			}
           	TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
           	MaxTextWidth = max (MaxTextWidth,LOWORD (TextExtent));
           	theight = HIWORD (TextExtent);  
       		if (CurTheme->PCTByArea)
            	TotCount = CurTheme->NumNonMask;
           	if (CurTheme->DisplayPCT || CurTheme->DisplayCount)
           	{
				Pct = 100.0;
				if (CurTheme->PCTByArea)
					sprintf (Text,"(%5.1f%%) %.2f acres",Pct,MaskSize*Pct/100); 
				else if (CurTheme->DisplayPCT && CurTheme->DisplayCount)
					sprintf (Text,"%ld (%5.1f%%)",CurTheme->ClassCount[iclass],Pct);
				else if (CurTheme->DisplayPCT)
					sprintf (Text,"%5.1f%%",Pct);
				else if (CurTheme->DisplayCount)
					sprintf (Text,"%ld",CurTheme->ClassCount[iclass]);
				else if (CurTheme->DisplayDistance)
					sprintf (Text,"%.2f",GetThemeClassDistance(iclass));
				if (hfontOld && hfont2 &&(hfontOld == hfont2))
					ii=1;
				if (hfontOld) SelectObject(CurView->hDC, hfontOld);
				if (hfont2) DeleteObject(hfont2);
				CurTheme->ClassFont2.lfHeight = IDNINT(CurTheme->ClassFont1.lfHeight * fontfactor); 
				hfont2 = CreateFontIndirect((PLOGFONT)&CurTheme->ClassFont2);
		        SelectObject(CurView->hDC, hfont2);
		       	TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
	           	MaxTextWidth = max (MaxTextWidth,LOWORD (TextExtent)); 
	           	theight += HIWORD (TextExtent);
	        }
           	MaxTextHeight = max (MaxTextHeight,theight);  
		}
    }

TooSmall:	fHeight *= 0.80;
	if (CurTheme->AppendCount)
	{ 
		if (CurTheme->DisplayDistance)
			sprintf (Text," (%.2f)",MaxCountD);  
		else
			sprintf (Text," (%ld)",MaxCount);
       	TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
       	CountTextWidth = LOWORD (TextExtent); 
       	BeginCount = MaxTextWidth - CountTextWidth;
	}
	if (CurTheme->InvertLegend)
	{
		usedclass = CurTheme->NumClass - 1;    
		classinc = -1;
	}
	else
	{
		usedclass = 0;
		classinc = 1;
	}
	for (iclass=0;iclass<CurTheme->NumClass;iclass++)
	{   
		UsedBox[iclass] = CurTheme->ClassClrBox[usedclass];
       	y = CurTheme->ClassClrBox[usedclass].top+
       		((CurTheme->ClassClrBox[usedclass].bottom-CurTheme->ClassClrBox[usedclass].top)-MaxTextHeight)/2;
		SetGlobalValueLong ("%CLASSCOUNT",CurTheme->ClassCount[iclass]);
		GetClassMinMax (iclass,&ClassMin,&ClassMax);
		SetGlobalValueReal ("%CLASSMIN",ClassMin);
		SetGlobalValueReal ("%CLASSMAX",ClassMax);
		if (CurTheme->ClassType ==3)
		{   
			_fstrcpy(Text,CurTheme->ClassBM[iclass]); 
    		if ((pSemiColon = _fstrchr (Text,';')) && CurTheme->ID == GF_GRAPHICS_FUNCTION_THEME)
    			*pSemiColon = 0;
			if ((pDesc = MatchLev (Text,'/'))) 
			{
				LPSTR pEnd = _fstrchr (pDesc+1,'/');
				if (pEnd)
				{
					short l = _fstrlen (pEnd);
					_fmemmove (pDesc,pEnd+1,l);
				}
			} 
			ExpandText (Text);
		}
		else
		{
	       	_fstrcpy (Val1,ValueConv ((double)CurTheme->ClassMin[iclass],CurTheme->ValConv,CurTheme->RoundTo,CurTheme->AddCommas));
	       	_fstrcpy (Val2,ValueConv ((double)CurTheme->ClassMax[iclass],CurTheme->ValConv,CurTheme->RoundTo,CurTheme->AddCommas));
            if (_fstrcmp (Val1,Val2))
	       		wsprintf (Text,"%s to %s",Val1, Val2);
	       	else
	       		_fstrcpy (Text,Val1); 
	       	_fstrcpy (CurTheme->ClassBM[iclass],Text);
	    } 
		SetTextColor (CurView->hDC,CurTheme->IBTextColor[0]);
        if (hfont) SelectObject(CurView->hDC, hfont);
		x =  CurTheme->ClassClrBox[usedclass].right + xmargin/2; 
		if ((!CurTheme->HideNullClasses || CurTheme->ClassCount[iclass]>0) &&
			  CurTheme->ColorsWidth < 100)
		{   
			if ((pCarrot = _fstrchr (Text,'^')))
				*pCarrot++ = 0; 
			if (CurTheme->CenterText)
			{   
				
		       	TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text)); 
		       	tw = LOWORD (TextExtent); 
		       	inc = max (0,(colw - (tw + xmargin))/2);
			}
			else
				inc = 0;
		    SetBkMode(CurView->hDC, TRANSPARENT);
		    if (!pCarrot || !_fstrchr (pCarrot,'&'))
				TextOut(CurView->hDC, x+inc, y, Text, _fstrlen(Text)); 
			if (pCarrot)
			{   
				short	BeginCount2;
				char	CountText[32];   
				LPSTR	pAnd;
				
				if ((pAnd = _fstrchr (pCarrot,'&')))
					*pAnd = 0;
				if (*pCarrot == '[')
				{
					pCarrot++;
					if (*CheckMarkSymbol)
					{
						BMRect = UsedBox[iclass];   
						BMRect.right = BMRect.left + colw;
						BMRect.left = 1 + BMRect.right - CurTheme->ActualXMargin - (colw-(UsedBox[iclass].right - UsedBox[iclass].left));
	    				_fstrcpy (IconFile,CurTheme->IconLibrary);
		    			_fstrcat (IconFile,CheckMarkSymbol);
		    			if (atob (pCarrot))
		    				DisplayBMFileInRect (CurView->hDC,IconFile, BMRect,1);//-2);
					}
					else
					{   
						char	OnOff[3]="  ";
						
						if (atob (pCarrot))
							_fstrcpy (OnOff,"X");  
						sprintf (CountText,"[%s]",OnOff);
					    TextExtent = GetTextExtent (CurView->hDC,CountText,_fstrlen(CountText));
					    CountTextWidth = LOWORD (TextExtent); 
					    BeginCount2 = MaxTextWidth - CountTextWidth;
						TextOut(CurView->hDC, x+BeginCount2, y,CountText, _fstrlen(CountText));
					} 
				}
				else
				{ 
					sprintf (CountText,"(%s)",pCarrot);
			       	TextExtent = GetTextExtent (CurView->hDC,CountText,_fstrlen(CountText));
			       	CountTextWidth = LOWORD (TextExtent); 
			       	BeginCount2 = MaxTextWidth - CountTextWidth;
					TextOut(CurView->hDC, x+BeginCount2, y,CountText, _fstrlen(CountText)); 
				}
			}
			else if (CurTheme->AppendCount)
			{   
				short	BeginCount2;
				char	CountText[32];
				
				TextOut(CurView->hDC, x+BeginCount, y, " (", 2); 
				if (CurTheme->DisplayDistance)
					sprintf (CountText,"%.2f)",GetThemeClassDistance(iclass)); 
				else
			    	sprintf (CountText,"%ld)",CurTheme->ClassCount[iclass]);
		       	TextExtent = GetTextExtent (CurView->hDC,CountText,_fstrlen(CountText));
		       	CountTextWidth = LOWORD (TextExtent); 
		       	CountTextHeight = HIWORD (TextExtent); 
		       	BeginCount2 = MaxTextWidth - CountTextWidth;
				TextOut(CurView->hDC, x+BeginCount2, y,CountText, _fstrlen(CountText)); 
			} 
		} 
		SetTextColor (CurView->hDC,CurTheme->IBTextColor[1]);
       	TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
		if (!CurTheme->CompressNullClasses || !CurTheme->HideNullClasses || CurTheme->ClassCount[iclass]>0) 
		{
			usedclass += classinc;
       		y += max (CountTextHeight,HIWORD (TextExtent));
       	}
		if ((CurTheme->DisplayPCT || CurTheme->DisplayCount)
		 && (!CurTheme->HideNullClasses || CurTheme->ClassCount[iclass]>0))
		{   
			if (TotCount)
				Pct = (100.0 * CurTheme->ClassCount[iclass]) / TotCount; 
			else
				Pct = 0;
			sprintf (Text,"[%%CLASSPCT(%i)]=%5.1f",iclass,Pct);
			ExpandText (Text);
			if (CurTheme->PCTByArea)
				sprintf (Text,"(%5.1f%%) %.2f acres",Pct,MaskSize*Pct/100);
			else if (CurTheme->DisplayPCT && CurTheme->DisplayCount)
				sprintf (Text,"%ld (%5.1f%%)",CurTheme->ClassCount[iclass],Pct);
			else if (CurTheme->DisplayPCT)
				sprintf (Text,"%5.1f%%",Pct);
			else if (CurTheme->DisplayCount)
				sprintf (Text,"%ld",CurTheme->ClassCount[iclass]);
			if (hfont2)
	        	SelectObject(CurView->hDC, hfont2);
	       	TextExtent = GetTextExtent (CurView->hDC,Text,_fstrlen(Text));
			TextOut(CurView->hDC, x+MaxTextWidth/2-LOWORD(TextExtent)/2,
								  y, Text, _fstrlen(Text));    
	       	y += HIWORD (TextExtent);
		}

	} 

	for (iclass=0;iclass<CurTheme->NumClass;iclass++)  //give theme editing functions correct final location
		CurTheme->ClassClrBox[iclass] = UsedBox[iclass];


	if (hfontOld && hfont &&(hfontOld == hfont))
		ii=1;
	if (hfontOld)
		SelectObject(CurView->hDC, hfontOld);
	GSSiDeleteObject(&hfont);
	GSSiDeleteObject(&hfont2);
    CurTheme->ClassFont1.lfHeight = SaveFF1;
    CurTheme->ClassFont2.lfHeight = SaveFF2; 
Exit:
	RestoreDC (CurView->hDC,-1);
	SetCurView ( SaveVP);
{
#if ENABLETRACE
GSSiExitProg (169);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}
void CreateClassPens (void)
#if ENABLETRACE
{GSSiEnterProg (1269);
#endif
{   int	itheme, i; 
    LPTHEME	SaveCurTheme=CurTheme;
    
	for (itheme=0;itheme<CurView->NumThemes;itheme++)
	{
		CurTheme=CurView->pThemes[itheme];
		if (CurTheme)
		{
			if (CurTheme->IsActive && CurTheme->VPDisplayed)
			{
				CurTheme->HiPrecis = 2; 
				CreateThemePens (CurTheme,FALSE);  
			}
		}
	} 
	CurTheme = SaveCurTheme;
{
#if ENABLETRACE
GSSiExitProg (1269);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}  


  


