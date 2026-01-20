#include "graphint.h"  

#include "gmextern.h"

static	int		AveFontCharWidth[MAXFONTS], FontDescent[MAXFONTS],FontIntLeading[MAXFONTS], FontAscent[MAXFONTS],FontAveCharWidth[MAXFONTS];
static	char	Text2[1024];
static	short	FontWeights[4]={FW_THIN,FW_NORMAL,FW_BOLD,FW_HEAVY}; 
static	long	NumDeletesProcessed=0;
static	short	nchar;  
static	BOOL	debugtpl=FALSE; 
static	BOOL	WantBaseRec=FALSE;

static RECT	TextRect;
static BOOL	ShowBaseLine=FALSE;
static long	Numcont=0;
static short	PostTextPointer=0;
static BOOL	Visible=TRUE;
static BOOL	TextIsVisible=TRUE;
static BOOL	DescIsVisible=TRUE;
static short	CurSavedPoly=0;
static short	NumSymVectors;
static HANDLE	hSymVectors;
static double	TXLineLen;
static POINT	PointLoc;
static POINT	BPLoc;
static POINT	MPLoc;
static POINT	EPLoc;
static short	TXFact;
static short	TXFont;
static short	TXColor;
static char	TXChar; 
static double	TextLineLength;

BOOL DisplayPointerLine (HDC hDC,BOOL Pick,LPDPOINT PLPoints,short np,short plsym,HANDLE hExport)
#if ENABLETRACE
{GSSiEnterProg (673);
#endif
{   
	short	i;
	
    if (hExport)
	{
		LPSHORT	pnText = (LPSHORT)GlobalLock (hExport);
		LPEXPORTTEXTPOINTER	pEXText = (LPEXPORTTEXTPOINTER)(pnText+1);
												
		pEXText += *pnText; 
		pEXText->Type = plsym; 
		pEXText->nPoints = np; 
		for (i=0;i<np;i++)
			pEXText->Loc[i] = PLPoints[i];
		(*pnText)++;
		GlobalUnlock (hExport);
	} 
	else if (Pick)
		PickPolylineD (PLPoints,np,0,2,0,0,0, 0,0);
    else if (hDC)
    {   
    	HPEN	hOldPen=0;
    	
    	if (!plsym)
    		plsym = DefaultTextPointer;
    	if (!plsym && ItemIsHighlighted)
			hOldPen = SelectObject(hDC, hRedPen); 
		GWPolylineD (hDC,PLPoints,np,plsym); 
		if (hOldPen)
			SelectObject (hDC,hOldPen);
	} 
	HaveTextPointers = TRUE;
{
#if ENABLETRACE
GSSiExitProg (673);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

void ConvertTextChar (LPBYTE Text)
{   
	LPBYTE	pFrom, pTo;
	
	if (!*ConvertTextFrom)
		return;
	while (*Text)
	{   
		pFrom = ConvertTextFrom;
		pTo = ConvertTextTo;
		while (*pFrom)
		{
			if (*Text == *pFrom)
			{
				*Text = *pTo;
				break;
			} 
			pFrom++;
			pTo++;
		}
		Text++;
	}
	return;
}

BOOL RemoveEmbeddedColor (LPSTR pText,LPCOLORREF pEmbeddedColor)
{
	BOOL	rtn=FALSE;

	if (!_fstrnicmp (pText,"$RGB(",5))
	{   
		LPSTR	pEndColor = _fstrchr (pText+5,')');
		short	lcolor = (long)pEndColor - (long)pText + 1;
		char	CColor[256];
		int		nchar = strlen (pText);
		
		if (lcolor < 255)
		{
    		strncpy0 (CColor,pText,lcolor);
    		memmove (pText,pText+lcolor,min (511,nchar-lcolor));
    		ExpandText (CColor); 
    		*pEmbeddedColor = atol (CColor);
			rtn = TRUE;
		}
	}
	return rtn;
}

BOOL ProcessTextObject (HDC hDC,LPGRTEXTHEADER	pGRTextHeader,LPSTR pText,int nchar,LPSTR BeginSeg,LPMNMXCORD pBounds,
						LPDPOINT pPickPointBase, LPDOUBLE pNearDist, LPMINMAX pMinMax)
#if ENABLETRACE
{GSSiEnterProg (674);
#endif
{
	BOOL	rtn=FALSE;
	int		SavedDC=0; 
	HFONT	hFont=0, OldFont=0, hFontShadow=0; 
	int		fheight, angle, weight, ltext;
	BOOL	italic; 
	HANDLE	hText;
	LPSTR	pText2;
	LOGFONT	LogFont;    
	DWORD	TextExt; 
	POINT	pWin;  
	DPOINT	pt, pt2, RP;    
	double	THeight, xmove, ymove, TXRot2, CLEN, RAD,RADBottom, lfHeightD, twidth, Maxtwidth, theight,tmDescentD,tmIntLeadingD, tmAscentD, AveCharWidth;
	float	TSize;
	short	x,y, NumLines, iline, NumUpLines, MaxLineLen,ii,RotationInc;   
	long	OldColor=-1, EmbeddedColor=-1; 
	char	SaveChr;
	BOOL	NullLine, Flipped;  
	UINT	i;
	static float	RIFact=0.2;      
	float	FontShrinkFactor = 1;  
	short	DoShrink=0; 
	double	TotTwidth;    
	BOOL	Shadow=FALSE;
    
    if ((Pick && !PickText)||!DisplayText)
    	return rtn;
	if (Pick)
		HaveTXLoc = TRUE;
    if (!hDC && hExportText)
    	hDC = CurView->hDC;
    CurrentTextBaseType = CurrentType; 
    if (CurrentTextBaseType == GF_POLYLINE || CurrentTextBaseType == GF_LINE)
    	DoShrink = 1;
	CurrentType = GF_TEXT;
ShrinkText: 
	pText2 = Text2;
    strncpy0 (pText2,pText,min (511,nchar));
	if (RemoveEmbeddedColor (pText2,&EmbeddedColor))
		nchar = strlen (pText2);
    if (ExpandGrText)
	{
		ExpandText (pText2); 
		if (RemoveEmbeddedColor (pText2,&EmbeddedColor))
			nchar = strlen (pText2);
		ConvertTextChar (pText2);
	}
//    ExpandText (pText2); 
	SetGlobalValue2 (hTEXT,pText2,0);
	_fmemset (&LogFont,0,sizeof(LOGFONT)); 
	THeight = GraphicsTextFactor * GetTextHeadSize (pGRTextHeader); 
	THeight *= ThemeTextSizeFactor;
	if (CurView->NewObjectMap[CurrentDesc])
	{
		THeight *= CurView->NewObjectTextFactor[CurView->NewObjectMap[CurrentDesc]-1]; 
	}
	if (pGRTextHeader->HeightIsPixels)
		lfHeightD = THeight / BaseDistToWinDist;
	else
		lfHeightD = THeight;
	TotTwidth = 0;
	lfHeightD *= FontShrinkFactor; 
	LogFont.lfHeight = IDNINT(lfHeightD * BaseDistToWinDist); 
	if (lfHeightD && DeviceRes)
	{   
		float	Inches = (lfHeightD * BaseDistToWinDist) / DeviceRes;
		TSize = Inches * 72.0;
    	TextIsVisible = GetTextVisibility (TSize,0);
    }
    else
    	TextIsVisible = FALSE;
	if (DescIsVisible && TextIsVisible && CurVis->WantType[2])
		Visible=TRUE;
	else
		Visible=FALSE;
    if (pBounds || (Visible && (Display || Pick || hExportText)))
    {   
		POINT	RectPoints[4];
					    	
	    if (hDC && !(Pick || hExportText))
	    {
	    	SavedDC = SaveDC (hDC);
			SetDisplayMode (hDC,GF_TEXTMODE); 
		}
/*						    if (Pick)
		{
			LogFont.lfHeight = -max(1,IDNINT((THeight * BaseDistToWinDist)/FileDistToWinDist));  
			SetDisplayMode (hDC,GF_MAPMODE);
			TXRot = -TXRot;
		}	
		else */
		if (pGRTextHeader->FlipForEasyReading && TXRot > HALFPI && TXRot < 3 * HALFPI)  
		{
			Flipped = TRUE;
			TXRot2 = LTWOPI (TXRot + PY);
		}
		else
		{
			Flipped = FALSE;
			TXRot2 = TXRot; 
		}
		if (TXRot2)	 
			angle  = IDNINT(3600-((LTWOPI(-TXRot2)/RADDEG)*10));
		else
			angle = 0; 
		if (angle >= 3600)
			angle = 0;
		LogFont.lfEscapement = LogFont.lfOrientation = angle; 
		if (LogFont.lfEscapement % 900)
			RotationInc = RIFact * lfHeightD;
		else
			RotationInc = 0;
		if (GraphicsTextWeight)
			LogFont.lfWeight = FontWeights[GraphicsTextWeight-1];    
		else  
			LogFont.lfWeight = FontWeights[pGRTextHeader->Weight];    
		if (pGRTextHeader->Weight == 3)    
		{
			LogFont.lfWeight = FW_THIN;
			Shadow = TRUE;       
		}
		else
			Shadow = pGRTextHeader->shadow;
		LogFont.lfItalic = pGRTextHeader->italic;    
		LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
		LogFont.lfQuality = PROOF_QUALITY; 
		_fstrcpy (LogFont.lfFaceName,FontNames[pGRTextHeader->FontNum]); 
		if (EmbeddedColor < 0 && MapType != MT_DGN7 && MapType != MT_DGN8 && !HaveTextColor)
			EmbeddedColor = FontColors[pGRTextHeader->FontNum];
		LogFont.lfWidth = abs (LogFont.lfHeight) * FontWidthFactor[pGRTextHeader->FontNum]; 
	    if (hDC && LogFont.lfHeight && Display && !(Pick || hExportText))
		{   
	//LogFont.lfHeight=abs(LogFont.lfHeight);//temp
			hFont = CreateFontIndirect((LPLOGFONT)&LogFont);
			if (Shadow)
			{
				LogFont.lfWeight = FW_HEAVY;
				hFontShadow = CreateFontIndirect((LPLOGFONT)&LogFont); 
			}
			OldFont = SelectObject (hDC,hFont);
		}
		NumLines = GetNumTextLines (pText2,&NumUpLines,&MaxLineLen,pGRTextHeader->UltiMapStyle);  
		Maxtwidth = MaxLineLen * GetFontAveCharWidth (pGRTextHeader->FontNum,lfHeightD);
		pText2 += NumUpLines;
		for (iline =0;iline < NumLines;iline++)
		{   
			double FlipInc;
			
			if ((ltext = _fstrlen(pText2)))
				NullLine=FALSE;
			else              
			{
				ltext=1;
				*pText2='A';
				NullLine=TRUE; 
				SaveChr = *(pText2+1);
				*(pText2+1) = 0;
			}
			GetTextWidthAndHeight (pGRTextHeader->FontNum,lfHeightD,pText2,ltext,&twidth,&theight,&tmDescentD,&tmIntLeadingD,&tmAscentD,&AveCharWidth);
			if (FontWidthFactor[pGRTextHeader->FontNum])
				twidth *= (2*FontWidthFactor[pGRTextHeader->FontNum]);
			if (DoShrink == 1  && TextLineLength < twidth)
			{   
        		if (!twidth)
        			FontShrinkFactor = 1;
        		else
        			FontShrinkFactor = TextLineLength/twidth;
        		DoShrink++;
				if (OldFont)
					SelectObject (hDC,OldFont); 
				OldFont = 0;
				GSSiDeleteObject (&hFont); 
        		goto ShrinkText;
			}
			if (Flipped)
			{
				FlipInc = tmDescentD + tmIntLeadingD;
				FlipInc = 0;
				theight = -theight; 
			}
			else
				FlipInc = 0;
			if (NullLine)
				ltext = 0; 
			if (pGRTextHeader->UltiMapStyle && PostTextPointer == 15)
				pGRTextHeader->vJust = 2;	
			switch (pGRTextHeader->vJust)
			{
				case 0: //baseline
					ymove = (NumLines - iline) * theight - tmDescentD;
					break;
				default:
				case 1://above  
					if (pGRTextHeader->UltiMapStyle)
						ymove = (1-iline) * (theight);
					else
						ymove = (NumLines - iline) * (theight);
					break;
				case 2: //centered
					ymove = (((double) NumLines) / 2) * theight - iline * theight; 
					break;
				case 3: //below
					ymove = -iline * theight;
					break;    
			} 
			ymove += theight * NumUpLines; 
			ymove += FlipInc+RotationInc;
			if (Pick || hExportText || pBounds)
				ymove -= tmIntLeadingD;//for picking and exporting Y value represents top of glyph not internal leading
			if (Flipped)
				ymove -= theight;   
			switch (HaveTXLoc)  
			{
				case 2:
					TXLineLen = ldistp (TXLocBPBase,TXLocEPBase)/2; 
					TXLineLen -= TXLineLen * (double)pGRTextHeader->hJust2 / 15;
					switch (pGRTextHeader->hJust)
					{
						case 2:
							xmove = -TXLineLen;
							break;
						default:
						case 1:
							xmove = -twidth/2;
							break;
						case 0:
							xmove = TXLineLen - Maxtwidth;
							break;            
					}
					break;
				case 3:
				{
					double	PctOfCir, CLSign;
										
					RCURVE (&TXLocBPBase.x,&TXLocBPBase.y,&TXLocPOCBase.x,&TXLocPOCBase.y,
							&TXLocEPBase.x,&TXLocEPBase.y,&RP.x,&RP.y,&CLEN);
					RADBottom = RAD = ldistp (RP,TXLocBPBase);     
					CLSign = DSIGN (1E0,CLEN);
					PctOfCir = fabs (CLEN)/(TWOPI * RAD); 
					RAD += CLSign * ymove;
					CLEN = CLSign * TWOPI * RAD * PctOfCir;  
					TXLineLen = fabs (CLEN)/2;
					TXLineLen -= TXLineLen * (double)pGRTextHeader->hJust2 / 15;
					switch (pGRTextHeader->hJust)
					{
						case 2:
							xmove = -TXLineLen;
							break;
						default:
						case 1:
							xmove = -twidth/2;
							break;
						case 0:
							xmove = TXLineLen - Maxtwidth;
							break;            
					}
				}
					break;
				default:  
					switch (pGRTextHeader->hJust)
					{
						case 2:
							xmove = -twidth;
							break;
						default:
						case 1:
							xmove = -twidth/2;
							break;
						case 0:
							xmove = 0;
							break;            
					} 
			} 
            if (!Pick && hDC && Display)
		    {
				if (ItemIsHighlighted)
					OldColor = SetTextColor (hDC,HighlightColor); 
				else if (CurView->NewObjectMap[CurrentDesc])
				{
			        short	UseHalfTone=0;
			        
			        if (CurView->NewObjectMap[CurrentDesc] < CurView->HalfToneNewObjectStart)
			        	UseHalfTone = -1;
					if (CurView->NewObjectSetTextColor[CurView->NewObjectMap[CurrentDesc]-1])
					{
						OldColor = SetTextColor (hDC,ConvertColor(RGB(CurView->NewObjectTextColor[CurView->NewObjectMap[CurrentDesc]-1].rgbtRed,
														 			  CurView->NewObjectTextColor[CurView->NewObjectMap[CurrentDesc]-1].rgbtGreen,
														    		  CurView->NewObjectTextColor[CurView->NewObjectMap[CurrentDesc]-1].rgbtBlue),UseHalfTone));
					}
				} 
				else if (EmbeddedColor >= 0)
					OldColor = SetTextColor (hDC,ConvertColor(EmbeddedColor,CurrentDesc));
					
				if (AutoOpaque || pGRTextHeader->Opaque)  
				{
//					OldColor = SetTextColor (hDC,ConvertColor(1,0));
					SetBkMode (hDC,OPAQUE);
				}
				else 
					SetBkMode (hDC,TRANSPARENT);
			} 
			if (HaveTXLoc < 3)
			{ 
//									ymove = -ymove;
				pt.x = TXLoc.x;
				pt.y = TXLoc.y;
                if (ShowBaseLine)
                {
                	DPOINT pt3;
                	POINT	p[2];
		                            	
					pt3 = dnewpt (pt,TXRot2,xmove);  
					p[0].x = IDNINT(pt3.x);
					p[0].y = IDNINT(pt3.y); 
					pt3 = dnewpt (pt3,TXRot2,twidth);  
					p[1].x = IDNINT(pt3.x);
					p[1].y = IDNINT(pt3.y); 
					Polyline (hDC,p,2);
				} 
									
				pt2 = dnewpt (pt,TXRot2,xmove);  
				pt = dnewpt (pt2,TXRot2+HALFPI,ymove); 
				pWin = BasePtToWinPt (&pt);
				x = pWin.x;
				y = pWin.y;
				if (Pick || hExportText || pBounds)
				{   
					DPOINT	RectPoints[4];
										
//									if (!hHighlightArea)
					{   
						double width = twidth;
						double height = fabs (theight) - tmIntLeadingD;  
						
						RectPoints[0] = pt;
						RectPoints[1] = dnewpt (pt,TXRot2,width);
						RectPoints[2] = dnewpt (RectPoints[1],TXRot2+HALFPI,-height);
						RectPoints[3] = dnewpt (RectPoints[2],TXRot2,-width); 
						if (hExportText && !NullLine)
						{
							LPSHORT	pnText = (LPSHORT)GlobalLock (hExportText);
							LPEXPORTTEXT	pEXText = (LPEXPORTTEXT)(pnText+1);
												
							pEXText += *pnText; 
							strncpy0 (pEXText->Text,pText2,ltext);
							pEXText->lText = ltext;  
							pEXText->Type = 1;
							pEXText->Loc[0] = RectPoints[3];
							pEXText->Loc[2] = RectPoints[2];
							pEXText->size = fabs (theight); 
							pEXText->intleading = tmIntLeadingD;
							pEXText->ascent = tmAscentD;   
							pEXText->avecharwidth = AveCharWidth;
							pEXText->Font = pGRTextHeader->FontNum;
							(*pnText)++;
							GlobalUnlock (hExportText);
						}
						else if (pBounds && !NullLine) 
						{   
							for (i=0;i<4;i++)
								AddDPointToMinMax (&RectPoints[i],pBounds);
							rtn = TRUE;
						}
						else if (hHighlightArea && !NullLine)
							PickPolyInAreaD(1, RectPoints, 4, 0, 0, 0, 0, 0, 0, 0);
						else if (!NullLine)
						{	
							short SavePP = PickPerim;
							
							PickPerim = 0;  
							if (pPickPointBase)
								PickNearPolylineD (4,RectPoints,4,0,pPickPointBase,pNearDist,pMinMax);
							else									
								PickPolygonD (RectPoints,4,0,0,0,0); 
							PickPerim = SavePP;
                        }
						if (ShowBaseLine)
						{
							SelectClipRgn (hDC,0);							
							Polygon (hDC,(LPPOINT)RectPoints,4);
						}
						if (HaveTXLoc == 2 && PostTextPointer)
							DisplayPostTextPointer (hDC,Pick,PostTextPointer,pt,TXRot2,theight,twidth,pBounds,hExportTextPointer); 
					}						
				}
				else
				{   
					if (HaveTXLoc == 2 && PostTextPointer)
						DisplayPostTextPointer (hDC,Pick,PostTextPointer,pt,TXRot2,theight,twidth,pBounds,hExportTextPointer); 
					if (hDC && hFont && ltext && !ShowBadSyms)
					{   
						TotTwidth = max (twidth,TotTwidth);
					//	if (DoShrink != 1 || (NumLines == 1 && TextLineLength >= TotTwidth))
						if (DoShrink == 1  && TextLineLength < TotTwidth)
						{   
        					if (TextLineLength >= TotTwidth)
        						FontShrinkFactor = 1;
        					else
        						FontShrinkFactor = TextLineLength/TotTwidth;
        					DoShrink++;
        					goto ShrinkText;
						}
						if (Shadow && GetTextColor (hDC) != RGB(255,255,255))
						{  
							COLORREF	SaveColor = SetTextColor (hDC,RGB(255,255,255));   
							int i = 1;
							int		iDeviceToScreenFactor = IDNINT(DeviceToScreenFactor());

							while (i <= iDeviceToScreenFactor)
							{
								//SelectObject (hDC,hFontShadow);
								ExtTextOut (hDC,x+i,y+i,0,0,pText2,ltext,0);
								ExtTextOut (hDC,x-i,y+i,0,0,pText2,ltext,0);
								ExtTextOut (hDC,x-i,y-i,0,0,pText2,ltext,0);
								ExtTextOut (hDC,x+i,y-i,0,0,pText2,ltext,0);
								//SelectObject (hDC,hFont);   
								SetTextColor (hDC,SaveColor);
								i++;
							}
						}
						ExtTextOut (hDC,x,y,0,0,pText2,ltext,0);
					}
					TextRect.left = x;
					TextRect.top = y;
					TextRect.bottom = y + theight*BaseDistToWinDist;
					TextRect.right = x + twidth*BaseDistToWinDist;    
				}
			}
			else
			{   
				DPOINT	MP;
				double	AZ, RadiansPerPixel; 
				LPSTR	pChr;
				short	cwidth;
									
				MP = MidPointD (TXLocBPBase,TXLocEPBase);  
				AZ = getazd (&RP,&MP); 
				RadiansPerPixel = 1e0/RAD;
				AZ = LTWOPI (AZ + (-xmove * DSIGN(1e0,CLEN))*RadiansPerPixel); 
				pChr = pText2;
				if (ltext) 
				{
					double	cwidth, th, tdc, til,tas,acw,AZ2;  
					if (hExportText && !NullLine)
					{
						LPSHORT	pnText = (LPSHORT)GlobalLock (hExportText);
						DPOINT	pt = dnewpt (RP,AZ,RADBottom);  
						LPEXPORTTEXT	pEXText = (LPEXPORTTEXT)(pnText+1); 
											
						GetTextWidthAndHeight (pGRTextHeader->FontNum,lfHeightD,pChr,ltext,&cwidth,&th,&tdc,&til,&tas,&acw);
													
						pEXText += *pnText; 
						strncpy0 (pEXText->Text,pText2,ltext);
						pEXText->lText = ltext;  
						pEXText->Type = 2;
						pEXText->Loc[0] = pt;
						AZ -= (DSIGN(cwidth/2,CLEN))*RadiansPerPixel; 
						pt = dnewpt (RP,AZ,RADBottom);
						pEXText->Loc[1] = pt;
						AZ -= (DSIGN(cwidth/2,CLEN))*RadiansPerPixel; 
						pt = dnewpt (RP,AZ,RADBottom);
						pEXText->Loc[2] = pt;
						pEXText->size = th;
						pEXText->Font = pGRTextHeader->FontNum;
						(*pnText)++;
						GlobalUnlock (hExportText);
					}
					else
					{
				        BOOL	Picked = FALSE; 
				        short	lt = _fstrlen (pChr), inc=1;
				        LPSTR	pc=pChr;
				        
				        if (Flipped)
				        {
				        	inc = -1;
				        	pc += (lt-1);
				        }
						while (*pChr && !Picked)
						{   
							DPOINT	pt = dnewpt (RP,AZ,RAD);  
							POINT	p;
							HFONT	hFontC,OldFontC=0;
							
							GetTextWidthAndHeight (pGRTextHeader->FontNum,lfHeightD,pc,1,&cwidth,&th,&tdc,&til,&tas,&acw);
							AZ = LTWOPI (AZ - (DSIGN(cwidth/2,CLEN))*RadiansPerPixel);
							AZ2 = AZ;
							if (Flipped)
								AZ2 = LTWOPI (AZ2 + PY);
							angle  = IDNINT((LTWOPI(AZ2-DSIGN(HALFPI,CLEN))/RADDEG)*10);
							if (angle >= 3600)
								angle = 0;
							LogFont.lfEscapement = LogFont.lfOrientation = angle; 
							p = BasePtToWinPt (&pt); 
												
							if (Pick || hExportText || pBounds)
							{   
								DPOINT	RectPoints[4];
								double width = cwidth;
								double height = th;
								POINT	wpt;
														
								wpt = p;
								pt2 = WinPtToBasePt (wpt);
								RectPoints[0] = pt2;
								RectPoints[1] = dnewpt (pt2,TXRot2,width);
								RectPoints[2] = dnewpt (RectPoints[1],TXRot2+HALFPI,-height);
								RectPoints[3] = dnewpt (RectPoints[2],TXRot2,-width); 
								if (hHighlightArea)
									Picked = PickPolyInAreaD(1, RectPoints, 4, 0, 0, 0, 0, 0, 0, 0);
								else if (pBounds) 
								{   
									for (i=0;i<4;i++)
										AddDPointToMinMax (&RectPoints[i],pBounds); 
									rtn = TRUE;
								}
								else
								{										
									if (pPickPointBase)
										Picked = PickNearPolylineD (4,RectPoints,4,0,pPickPointBase,pNearDist,pMinMax);
									else									
										Picked = PickPolygonD (RectPoints,4,0,0,0,0); 
								}
							}	
							else if (hDC)
							{
								if (LogFont.lfHeight)
								{  
									hFontC = CreateFontIndirect((LPLOGFONT)&LogFont);
									OldFontC = SelectObject (hDC,hFontC); 
								}
								if (OldFontC)
								{   
									if (!ShowBadSyms)
										TextOut (hDC,p.x,p.y,pc,1);
									SelectObject (hDC,OldFontC); 
									GSSiDeleteObject (&hFontC); 
								} 
							}
							pChr++; 
							pc += inc;
							AZ -= (DSIGN(cwidth/2,CLEN))*RadiansPerPixel;
						} 
					}
				}
			}
			if (NullLine)
			{
				pText2++;
				*pText2 = SaveChr;
			}
			else
			{ 
				pText2 = _fstrchr (pText2,0);
				pText2++;
			}
			if (*pText2 == '\n')
				pText2++;
		} 
		if (hDC)
		{
	        if (OldColor >= 0)
				OldColor = SetTextColor (hDC,OldColor);
			if (OldFont)
			{ 
				SelectObject (hDC, OldFont);
				GSSiDeleteObject (&hFont);   
				GSSiDeleteObject (&hFontShadow);
			}
			if (SavedDC)  
				RestoreDC (hDC,-1); 
		}
		if (DoShrink == 1  && (NumLines > 1 || TextLineLength < TotTwidth))
        {   
        	if (TextLineLength >= TotTwidth)
        		FontShrinkFactor = 1;
        	else
        		FontShrinkFactor = TextLineLength/TotTwidth;
        	DoShrink++;
        	goto ShrinkText;
        }
	} 
	else
    {   
    	LPSTR	pString;
		
		if (BeginSeg)			    	
			CurTextStringLoc = CurrentSeg + (long)((LPSTR) pGRTextHeader - (LPSTR)BeginSeg); 
		lTextString = nchar; 
		GSSiGlobFree (&hTextString);   
		hTextString = GSSiGlobAlloc(GAIDNO 297,GMEM_MOVEABLE,512);
		pString = GlobalLock (hTextString);
	    strncpy0 (pString,pText,min (511,nchar));
		GlobalUnlock (hTextString);
    }
{
#if ENABLETRACE
GSSiExitProg (674);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}

BOOL ProcessUMTextPointer (HDC hDC, BOOL Pick, LPUMTEXTTPL	lpTextTPL,LPDPOINT TPLpoints, LPDPOINT PLPoints, LPMNMXCORD pBounds,HANDLE hExport)
#if ENABLETRACE
{GSSiEnterProg (675);
#endif
{   
	DPOINT	BPoint = TXLocBPBase, dPoint;   
	double	BPRot = PTRot;
	short	plsym=0, ii, np;
	DPOINT	ZPoint={0,0};
	BOOL	rtn=FALSE;
	
  	switch (lpTextTPL->style1)
  	{   
		case 14:
		case 15:
			if (TextType == 2)
				TextType = 4;
			else 
				TextType = 3;
			PostTextPointer = lpTextTPL->style1;
  		case 0:
		case 1:  
		{
			double	AZ, Dist;
								
			switch (lpTextTPL->lorc1)
			{   
				default:
				case 1:
					AZ = LTWOPI(getazd (&ZPoint,&TPLpoints[0]) + PTRot);
					Dist = ldistp (ZPoint,TPLpoints[0]);
					PLPoints[0] = TXLocBPBase = dnewpt (BPoint,AZ,Dist);
					AZ = LTWOPI(getazd (&ZPoint,&TPLpoints[1]) + PTRot);
					Dist = ldistp (ZPoint,TPLpoints[1]);
					PLPoints[1] = TXLocEPBase = dnewpt (BPoint,AZ,Dist);
					dPoint = MidPointD(TXLocBPBase,TXLocEPBase);
	 		    	PTRot = getazd (&TXLocBPBase,&TXLocEPBase);
					TXLoc = dPoint; 
					HaveTXLoc = 2;
		    		TXRot = PTRot;
		    		if (debugtpl) 
						GWPolylineD (hDC,PLPoints,2,0);
		    		break;
		    	case 3:
					AZ = LTWOPI(getazd (&ZPoint,&TPLpoints[0]));
					Dist = ldistp (ZPoint,TPLpoints[0]);
					TXLocBPBase = dnewpt (BPoint,AZ,Dist);
					AZ = LTWOPI(getazd (&ZPoint,&TPLpoints[1]));
					Dist = ldistp (ZPoint,TPLpoints[1]);
					TXLocPOCBase = dnewpt (BPoint,AZ,Dist);
					AZ = LTWOPI(getazd (&ZPoint,&TPLpoints[2]));
					Dist = ldistp (ZPoint,TPLpoints[2]);
					TXLocEPBase = dnewpt (BPoint,AZ,Dist);
					HaveTXLoc = 3;
		    		break;
		    }
    	}
		break;

		default:
			ii=1;
		break;
	}
						
  	switch (lpTextTPL->style2)
  	{   
  		case 0:
		case 1:
			DisplayPointerLine (hDC,Pick,PLPoints,2,plsym,hExport);
			break; 
		case 5:
		case 9: 
			if (lpTextTPL->style2 == 5)
				plsym = TXPntrSym[0];
			else
				plsym = TXPntrSym[1];
		case 2:  
		case 3:
		{
			double	AZ, Dist;
								
			if (TextType == 2)
				TextType = 4;
			else 
				TextType = 3;
			switch (lpTextTPL->lorc2)
			{   
				default:
				case 1:
					AZ = LTWOPI(getazd (&ZPoint,&TPLpoints[2]) + BPRot);
					Dist = ldistp (ZPoint,TPLpoints[2]);
					PLPoints[0] = dnewpt (BPoint,AZ,Dist);
					AZ = LTWOPI(getazd (&ZPoint,&TPLpoints[3]) + BPRot);
					Dist = ldistp (ZPoint,TPLpoints[3]);
					PLPoints[1] = dnewpt (BPoint,AZ,Dist);
					np = 2;
		    		break;
				case 2:
					AZ = LTWOPI(getazd (&ZPoint,&TPLpoints[2]) + BPRot);
					Dist = ldistp (ZPoint,TPLpoints[2]);
					PLPoints[0] = dnewpt (BPoint,AZ,Dist);
					AZ = LTWOPI(getazd (&ZPoint,&TPLpoints[3]) + BPRot);
					Dist = ldistp (ZPoint,TPLpoints[3]);
					PLPoints[2] = dnewpt (BPoint,AZ,Dist);
					AZ = LTWOPI(getazd (&ZPoint,&TPLpoints[4]) + BPRot);
					Dist = ldistp (ZPoint,TPLpoints[4]);
					PLPoints[1] = dnewpt (BPoint,AZ,Dist);
					np = 3;
		    		break;
		    	case 3:
					AZ = LTWOPI(getazd (&ZPoint,&TPLpoints[0]));
					Dist = ldistp (ZPoint,TPLpoints[2]);
					PLPoints[0] = dnewpt (BPoint,AZ,Dist);
					AZ = LTWOPI(getazd (&ZPoint,&TPLpoints[1]));
					Dist = ldistp (ZPoint,TPLpoints[3]);
					PLPoints[1] = dnewpt (BPoint,AZ,Dist);
					AZ = LTWOPI(getazd (&ZPoint,&TPLpoints[2]));
					Dist = ldistp (ZPoint,TPLpoints[4]);
					PLPoints[2] = dnewpt (BPoint,AZ,Dist);  
					np = 3;
		    		break;
		    } 
			DisplayPointerLine (hDC,Pick,PLPoints,np,plsym,hExport);
		    if (pBounds)
		    {   
		    	short	i;
		    	
		    	for (i=0;i<np;i++)
					AddDPointToMinMax (&PLPoints[i],pBounds);
		    }  
		    rtn = TRUE;
    	}
		break;

		default: 
			ii=1;
		break;
	} 
{
#if ENABLETRACE
GSSiExitProg (675);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

short GetNumTextLines (LPSTR pText,LPSHORT pNumUpLines,LPSHORT MaxLineLen,BOOL UltiMapStyle)
#if ENABLETRACE
{GSSiEnterProg (676);
#endif
{
//calcs num lines and converts CR to NULL in pText
	short	nl=1, l;
	LPSTR	pCR=pText, pLineBeg;
	
	*pNumUpLines = 0;
	*MaxLineLen = 0;
	if (UltiMapStyle)
	while (*pCR == '!')
	{
		(*pNumUpLines)++;
		pCR++;
	} 
	pLineBeg = pCR;
	while ((pCR = _fstrchr (pCR,'\r')))
	{
		nl++;
		*pCR++ = 0; 
		if (*pCR == '\n')
			pCR++;
		*MaxLineLen = max (*MaxLineLen,_fstrlen (pLineBeg));
		pLineBeg = pCR;
	}
	*MaxLineLen = max (*MaxLineLen,_fstrlen (pLineBeg));
{
#if ENABLETRACE
GSSiExitProg (676);
#endif
	return nl;
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayPostTextPointer (HDC hDC,BOOL Pick,short PostTextPointer,DPOINT pt,double AZ,double theight,double twidth, LPMNMXCORD pBounds,HANDLE hExport)
#if ENABLETRACE
{GSSiEnterProg (680);
#endif
{   
	DPOINT	PLPoints[2];
	short	plsym=0;
	
	switch (PostTextPointer)
	{
		case 15:
			plsym = TXPntrSym[0];
			PLPoints[0] = dnewpt (pt,LTWOPI(AZ-HALFPI),theight/2);
			PLPoints[1] = TXLocBPBase; 
			DisplayPointerLine (hDC,Pick,PLPoints,2,plsym,hExport);
			PLPoints[0] = dnewpt (PLPoints[0],AZ,twidth);
			PLPoints[1] = TXLocEPBase;
			DisplayPointerLine (hDC,Pick,PLPoints,2,plsym,hExport);
		    if (pBounds)
		    {   
		    	short	i;
		    	
		    	for (i=0;i<2;i++)
					AddDPointToMinMax (&PLPoints[i],pBounds);
		    }  
{
#if ENABLETRACE
GSSiExitProg (680);
#endif
			return TRUE;
}
	}
{
#if ENABLETRACE
GSSiExitProg (680);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
} 

void SetTextLocVars (LPBOOL pTLSet)
#if ENABLETRACE
{GSSiEnterProg (681);
#endif
{   
	if (*pTLSet)
		goto Exit;
	*pTLSet = TRUE; 
	if (!HaveTXLoc)
		goto Exit;
	TXLineLen = 0;  
	TextType = 1;
    switch (CurrentType)
    {
    	case GF_CURVE:
	    {
	    	DPOINT dPoint = MidPointD(BP,EP);
	    	PTRot = getazd (&BP,&EP); 
			TXLoc = dPoint; 
			TXLocBPBase = BP;
			TXLocEPBase = EP;
			TXLocPOCBase = CurPOCW;
			HaveTXLoc = 3;  
    		TXRot = PTRot;   
    		TextType = 2;
		}
    	break;
				    	
    	case GF_AREA: 
    	{
			TXLoc = ComputeAreaMidpoint2 (lpDCurPoints,nPnts);
    		TXRot = 0;  
    		HaveTXLoc = 1; 
    	}
    	break;
    	
    	case GF_LINE:
    	case GF_POLYLINE: 
    	if (PolyIsHiPrecis)
    	{
 		    if (nPnts > 1)
 		    {   
 		    	DPOINT dPoint;
				TXLocBPBase = *lpDCurPoints;
				TXLocEPBase = *(lpDCurPoints+(nPnts-1));
				dPoint = MidPointD(TXLocBPBase,TXLocEPBase);
 		    	PTRot = getazd (&TXLocBPBase,&TXLocEPBase);
				TXLoc = dPoint;
				HaveTXLoc = 2;
	    		TXRot = PTRot; 
	    		TextLineLength = ldistp (TXLocBPBase,TXLocEPBase);
            } 
        }
        else
        {
 		    if (nPnts > 1)
 		    {   
 		    	POINT	MidPFile = MidPoint (POINTStoPOINT(*lpCurPoints),POINTStoPOINT(*(lpCurPoints+(nPnts-1))));
 		    	DPOINT	dPoint = FilePtToBasePt (MidPFile);
 		    	PTRot = getaz (POINTStoPOINT(*lpCurPoints),POINTStoPOINT(*(lpCurPoints+1)));
				TXLoc = dPoint;
				HaveTXLoc = 2;
	    		TXRot = PTRot; 
            } 
        }
    	break;
				    	
    	default: 
			TXLoc = TXLocBPBase = CurPointLocD; 
			TXLocEPBase = CurPointLocD;
    		TXRot = PTRot;  
    		//HaveTXLoc = 1;  if theme skips record should be set to 0
    	break;
    } 
Exit:
{
#if ENABLETRACE
GSSiExitProg (681);
#endif
	return;
}
#if ENABLETRACE
}
#endif
} 

void ClearSizingFonts (void)
#if ENABLETRACE
{GSSiEnterProg (682);
#endif
{
	short	i;
	
	for (i=0;i<MAXFONTS;i++)
	{
    	if (hSizingFont[i])
    	{
    		GSSiDeleteObject (&hSizingFont[i]);
    		hSizingFont[i]=0; 
    	}
    } 
{
#if ENABLETRACE
GSSiExitProg (682);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

HFONT GetSizingFont (int FontNum)
#if ENABLETRACE
{GSSiEnterProg (683);
#endif
{
	LOGFONT	LogFont;    
	DWORD	TextExt; 
   	TEXTMETRIC	TextMet;
   	HFONT	OldFont;
    if (FontNum < 0 || FontNum > MAXFONTS-1)
    	FontNum = 0;
	if (!hSizingFont[FontNum])
	{  
		_fmemset (&LogFont,0,sizeof(LOGFONT));
		LogFont.lfHeight = 5000; 
		LogFont.lfEscapement = 0;   
		LogFont.lfWeight = FW_NORMAL;    
		LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
		LogFont.lfQuality = DEFAULT_QUALITY; 
		_fstrcpy (LogFont.lfFaceName,FontNames[FontNum]);  
		hSizingFont[FontNum] = CreateFontIndirect((LPLOGFONT)&LogFont);
		OldFont = SelectObject (CurView->hDC,hSizingFont[FontNum]);
		GetTextMetrics (CurView->hDC,&TextMet); 
		AveFontCharWidth[FontNum] = TextMet.tmAveCharWidth;
		FontDescent[FontNum] = TextMet.tmDescent;
		FontIntLeading[FontNum] = TextMet.tmInternalLeading;
		FontAscent[FontNum] = TextMet.tmAscent;
		FontAveCharWidth[FontNum] = TextMet.tmAveCharWidth;
		
		SelectObject (CurView->hDC,OldFont);
	}
{
#if ENABLETRACE
GSSiExitProg (683);
#endif
	return hSizingFont[FontNum];
}
#if ENABLETRACE
}
#endif
} 

BOOL GetTextWidthAndHeight (int FontNum,double lfHeightD,LPSTR pText,int ltext,LPDOUBLE ptwidth, LPDOUBLE ptheight, LPDOUBLE ptmDescentD,LPDOUBLE ptmIntLeadingD,LPDOUBLE ptmAscentD, LPDOUBLE ptmAveCharWidthD)
#if ENABLETRACE
{GSSiEnterProg (684);
#endif
{   
	int		twidth, theight;
	HFONT	hFont, OldFont;
	SIZE	txSize;
	int		SFFac=5000;
	
   	SaveDC (CurView->hDC);
	SetDisplayMode (CurView->hDC,GF_TEXTMODE); 
	hFont = GetSizingFont(FontNum);  
	OldFont = SelectObject (CurView->hDC,hFont);
	GetTextExtentPoint32 (CurView->hDC,pText,ltext,&txSize); 
	if (OldFont)
		SelectObject (CurView->hDC,OldFont);
	twidth = txSize.cx;
	theight = txSize.cy;
	*ptwidth = twidth * lfHeightD / SFFac;
	*ptheight = theight * lfHeightD / SFFac; 
	*ptmDescentD = FontDescent[FontNum] * lfHeightD / SFFac;
	*ptmIntLeadingD = FontIntLeading[FontNum] * lfHeightD / SFFac;
	*ptmAscentD = FontAscent[FontNum] * lfHeightD / SFFac;
	*ptmAveCharWidthD = FontAveCharWidth[FontNum] * lfHeightD / SFFac;
	RestoreDC (CurView->hDC,-1); 
{
#if ENABLETRACE
GSSiExitProg (684);
#endif
	return TRUE;  
}
#if ENABLETRACE
}
#endif
}

void AdjustPointRotation (LPDOUBLE pRot)
{   
	switch (PointRotOpt)
	{
		case 1:
			*pRot = PointRotValue;
			break;
		case 2:
			*pRot = LTWOPI (*pRot + PY);
			break;
		case 3:
			*pRot = LTWOPI (*pRot + PointRotValue);
			break;
	}
	return;
}

double GetFontAveCharWidth (int FontNum,double lfHeightD)
#if ENABLETRACE
{GSSiEnterProg (685);
#endif
{   
	double	dwidth;
	
	GetSizingFont(FontNum);  
	dwidth = AveFontCharWidth[FontNum] * lfHeightD / 5000;
{
#if ENABLETRACE
GSSiExitProg (685);
#endif
	return dwidth;
}
#if ENABLETRACE
}
#endif
}  

BOOL ItemInHotSpotWithoutPassThrough (int desc)
{   
	UINT	itheme;
	BOOL	rtn=FALSE;
	
//	return FALSE;
	if (CurView && CurView->PassID)
	{
		for (itheme=0;itheme<CurView->NumThemes;itheme++) 
		{
			if (CurView->pThemes[itheme]->ID == GF_HOTSPOT_THEME && CurView->pThemes[itheme]->HotSpotData.PassThrough == 2)
			{   
				LPTHEME	SaveTheme = CurTheme;
				
				CurTheme =  CurView->pThemes[itheme]; 
		  		if (CurTheme->IsActive && CurTheme->VPDisplayed) 
		  		{
					if (CurTheme->hVisList)
					{   
						LPVISLIST	SaveVis=CurVis; 
						
						CurVis = (LPVISLIST)GlobalLock (CurTheme->hVisList);
						if (GetVisibility (desc))
							rtn = TRUE;
						GlobalUnlock (CurTheme->hVisList);
						CurVis = SaveVis; 
					}
					CurTheme = SaveTheme;  
				}
			}
		}  
	}
	return rtn;
}

BOOL RefInSkipList (int Refno)
{
	if (nSkipRefs)
	{
		LPSKIPREF	pSkipRefs = GlobalLock (hSkipRefs);
		UINT	i;

		for (i=0;i<nSkipRefs;i++)
		{
			if ((!pSkipRefs[i].VPID || CurView->ID == pSkipRefs[i].VPID) && Refno == pSkipRefs[i].Refno)
			{
				GlobalUnlock (hSkipRefs);
				return TRUE;
			}
		}
		GlobalUnlock (hSkipRefs);
	}
	return FALSE;
}

BOOL ProcessRefAndTAG (BOOL DescIsVisible,LPSTR lpTAG,int ltag)
#if ENABLETRACE
{GSSiEnterProg (1385);
#endif
{    
	char	str[16]; 
	BOOL	Visible=DescIsVisible;
	HIGHLIGHTDATA	HighlightData;
    static	long	debugrefno= 9000009;
    short	ii;
	static	BOOL	ShowOnlyDebugRef = FALSE;
	static	BOOL	ShowOnlyDebugUDI = FALSE;
	static	char	debugUDI[34] = "170933353401";
    
    if (TraceRef)
    {
    	ltoa (CurrentRefno,str,10);
    	SetWindowText (hWndMain,str);
    }
	if (ShowRefno)
	{
	    HANDLE	hMem = GSSiGlobAlloc(GAIDNO 304,GMEM_MOVEABLE,256); 
	    LPSTR 	str = GlobalLock (hMem);  
	    
    	sprintf (str,"%s: %ld",PltName,CurrentRefno);
    	SetWindowText (TraceWnd,str);  
    	GSSiGlobUlFree (&hMem);
    } 
	if (*debugUDI)
	{
		LPSTR pUDI = strchr(lpTAG, ':');

		if (pUDI)
		{
			pUDI++;
			if (!stricmp(debugUDI, pUDI))
				ii = 1;
			else if (ShowOnlyDebugUDI)
			{
				Visible = FALSE;
				goto Exit;
			}
		}
	}
    if (CurrentRefno == debugrefno)
	   	ii=1;
	else if (ShowOnlyDebugRef)
	{
		Visible = FALSE;
		goto Exit; 
	}
	if (FastMapCopyHltOnly)
	{
		if (BT_FIND (hHighlight,(LPSTR)&CurrentRefno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData))
		{
			Visible = FALSE;
			goto Exit;
		}
	}
	SetVarChangeTimes (0);
	NumRecordsProcessed++;
   	CurItemHLTShow = -1;  
   	if (ItemInHotSpotWithoutPassThrough (CurrentDesc))
	{
		Visible = FALSE;
		goto Exit; 
	}
	if (RefInSkipList (CurrentRefno))
		Visible = FALSE;
	else if (DescIsVisible || ForceRefIndex || ForceTAGIndex || PickDeletes)
	{   
		short	RIPL = RefInPrevLayer (CurrentRefno);
                		
    	if (ItemIsRemoved || RIPL == 1)
    		Visible = FALSE;    
    	else if (ProcessSingleItem || 
    	         (!ShowDeletedOpt && !RIPL && !ItemIsDeleted) ||
    	         (ShowDeletedOpt && RIPL == 2) ||
    	         (ShowDeletedOpt && ItemIsDeleted) ||
    	          ForceRefIndex || ForceTAGIndex || 
    	         (Pick && PickDeletes))
    	{   
    		if (ShowDeletedOpt && RIPL == 2)
    			ItemIsBlocked = TRUE;
        	SetIntRefno (CurrentRefno); 
			LastRef=LONG_MIN;
        	BuildRefIndex (FALSE,ItemIsDeleted); 
        	CurlTAG = ltag;
        	if (ltag) 
        	{
        		LPSTR	pTAGList;
        		LPSTR	lpColon = _fstrchr (lpTAG,':');   
        		short	len;
        		
        		if (*lpTAG == '"' || !lpColon || !_fstricmp (lpTAG,"REFNO"))
        			goto TAGIsRefno;
        		*lpColon = 0;
				SetGlobalValue2 (hPrefix,lpTAG,0); 
				strncpy0(CurrentPrefix,lpTAG,MAX_PREFIX_LEN);
				len = ltag - (lpColon - lpTAG + 1);
				SetUDIValueLen (lpTAG,++lpColon,len); 
				CurrentUDILen = len;
				strncpy0 (CurrentUDI,lpColon,min(len,MAX_UDI_LEN));
				if (!stricmp (CurrentPrefix,"MSLINK"))
					CurMSLink = atoi (CurrentUDI);
				BuildTAGIndex (lpTAG,lpColon,len,CurrentRefno,ItemIsDeleted);
				if (CurView->hTAGList)
				{
					pTAGList = GlobalLock(CurView->hTAGList);
					while (*pTAGList)
					{
						if (!_fstricmp (lpTAG,pTAGList)) goto TAGOut;
						pTAGList+=10;
									
					}
					_fstrcpy (pTAGList,lpTAG);
			TAGOut:	GlobalUnlock (CurView->hTAGList); 
				}
             	if (lpColon)
             		*(--lpColon) = ':';   
        	}
        	else
        	{   
TAGIsRefno:		_fstrcpy(CurrentPrefix,"REFNO");
				SetGlobalValue2 (hPrefix,"REFNO",0);
				if (SetRefno)  
					sprintf (str,"%.2f",(((double)CurrentRefno) - (-2146450000))/100); 
				else
					ltoa(CurrentRefno, str, 10);
				SetGlobalValue2 (hUDI,str,0);
				CurrentUDILen = _fstrlen(str);
				strncpy0 (CurrentUDI,str,MAX_UDI_LEN); 
         	} 
		    if (TextIsVisible && !(ForceRefIndex || ForceTAGIndex))
		    {
		    	Visible = TRUE;
		    	if (Display && hHighlight)
		    	{
			    	if (!BT_FIND (hHighlight,(LPSTR)&CurrentRefno,BT_FIRST,BT_EQ,(LPSTR)&HighlightData))
						CurItemHLTShow = HighlightData.Show;
		        }
		        if (ForceHLT)
		        	CurItemHLTShow = 1;
				if (CurItemHLTShow == 1) 
						ItemIsHighlighted = TRUE; 
				if (!ItemIsHighlighted && DisplayOnlyHLT)
					Visible = FALSE;
				if (ItemIsHighlighted && DisplayOnlyNonHLT)
					Visible = FALSE;
		    }
	    }
	    else if ((!ShowDeletedOpt && (ItemIsDeleted || RIPL)) ||
	    		 (ShowDeletedOpt && !(ItemIsDeleted || RIPL))) 
	    	Visible=FALSE; 
	}
	else
		Visible=FALSE; 
	if (Visible && ProcessStatusWnd) 
	{
		char	TAGUDI[80];
		
		sprintf (TAGUDI,"%s:%s",CurrentPrefix,CurrentUDI);
		ProcessStatusWindowUpdate (0,TAGUDI);
	}
	if (Visible)
	{
		Visible = AddToDupRefList (CurrentRefno);
		SetSavedGraphicsFid (SymbolDisplayPos(CurrentDesc));
	}
Exit:
{
#if ENABLETRACE
GSSiExitProg (1385);
#endif
	return Visible;
}
#if ENABLETRACE
}
#endif
}

void InitRecord (HDC hDC)
{   
	if (hDC)
	{
		SetBkMode (hDC,OPAQUE); //was transparent 2010-10-29
		SetROP2(hDC,DisplayRasterOpt);
	}
	TSize = 0;  
	GSSiGlobFree (&ShowVal.hPoints); 
	GSSiGlobFree (&ShowVal.hPolyPartLen); 
	memset (&ShowVal,0,sizeof(ShowVal));
	_fmemset (CurStreetNumbers,0,16); 
    GSSiGlobFree (&hUnSplinedPoly);
	GSSiGlobUlFree (&hCoords);   
	GSSiGlobFree (&hCurvePoints);  
	GSSiGlobFree (&hGCmdString);  
	ThemePointColor = -1; 
	pnPnts = 0; 
	pCurveBP = 0;
	ItemIsBlocked=ThemePointSym=HaveVarFillColor=ItemIsHighlighted=FromStreet=ToStreet=CurStreet=HighlightThisItem=SpecialThisItem=0;
	CurTextHeaderLoc=CurSymSizeLoc=CurSymSizeLocD=CurGCmdStringLoc=CurTextStringLoc=CurBrushLoc=CurPenColorLoc=0;
	CurTAGLoc=CurSNamesLoc=nPoly=iPoly=HaveTXLoc=TLSet=nCurvePoints=nPolySegments=CurMSLink=0;
	PostTextPointer=PenFromUMList=Visible=HaveTextColor=StreetOneWay=*StreetBPType=*StreetEPType=0;  
	StreetCenterline=0;
	*CurrentPrefix = *CurrentUDI = 0; 
	CompareDC = 0;
	CurlTAG = 0;
	GRStartTime=LONG_MIN;
	GREndTime=LONG_MAX;
	LastRef=LONG_MIN;
	CurrentType = CurrentTextBaseType = 0;
	CurArea = CurLength = ItemSymbolWidth  = 0; 
	ThemeWidthFactor = ThemeTextSizeFactor = CurPointSize = 1; 
	TempLineColor = CurThemeClass = -1;    
	TempLineWidth = 0;
	RouteOffset = 0;
	CurrentPen = 0;
	AdjustBlue = 253;
	SetGlobalValue2 (hTEXT,0,0);
	GSSiGlobFree (&hPolyBuffer); 
	GSSiGlobFree (&hElevBuffer);  
	DescIsVisible=TextIsVisible=TRUE; 
	if (CurView->HaveLayerColor[FileNum])
	{
		GlobalColors[0]=CurView->LayerColor[FileNum];  
		HaveVarFillColor = TRUE;
	}
	curItemSQMeters = -1;
	curItemPerim = -1;
	return;
}

BOOL ProcessPolygon (HDC hDC,BOOL ShowBorder,int PltType,HPEN hRandPen,HPEN hTempPen,int ipen)
{
	BOOL	DoFill;
	HPEN	hSymbolBorderPen=0;
	int		BorderSymbolNum=0;
	UINT	i;
	BOOL	rtn=FALSE;
	HPEN	hSavePen=0;
	BOOL	HiPrecisSave = HiPrecis;
	HANDLE	hHiPrecis=0;

	if (CurrentDesc == gridSymbol)
	{
		DisplayContours (hDC,CurrentUDI);
		return TRUE;
	}
	HaveTXLoc = TRUE;
	DoFill = GetBit (7,(LPSTR)&CurVis->WantType[7]);
	if (!HiPrecis && UseShortSymbols)
	{
		HiPrecis = TRUE;
		hHiPrecis = GSSiGlobAlloc(GAIDNO 1767,GMEM_MOVEABLE,(long)nPnts*sizeof(DPOINT));
		lpDCurPoints = (HPDPOINT) GlobalLock (hHiPrecis);

		for (i=0;i<nPnts;i++)
			lpDCurPoints[i] = FilePtToBasePt (POINTStoPOINT(lpCurPoints[i]));   
	}
	if (FillAreas)// && (hSpecialBrush || (GetBit (6,(LPSTR)&CurVis->WantType[7]))))
	{ 
		
		if (Highlight)
		{
			HBRUSH	hBrush, hOldBrush;
			
			if (hDC && ClearHighlightedAreaSpace)
			{
				if (hDC && ShowLinkLines)
					hSavePen = SelectObject(hDC, hAreaBorderPen[HiPrecis]); 
				hBrush = CreateSolidBrush(CurView->BackGroundColor);
				hOldBrush = SelectObject (hDC,hBrush); 
				if (HiPrecis)
					GWPolygonD (hDC,lpDCurPoints,nPnts,nPoly,hPolyPartLen,0,ShowBorder,DoFill,0);  
				else
					GWPolygon (hDC,lpCurPoints,nPnts,nPoly,hPolyPartLen,0,ShowBorder,DoFill,0);  
				SelectObject (hDC,hOldBrush);
				DeleteObject (hBrush);
			}
		} 
		else if ((!nPoly || ShowLinkLines) && ShowBorder)
		{   
			if (Display && !Pick && hDC && !hSpecialPen)
				hSavePen = SelectObject(hDC, hAreaBorderPen[HiPrecis]); 
		}
		if (HiPrecis)
		{   
			if (PltType == 9) 
				ShowBorder = ProcessTINPoly (lpDCurPoints,hElevBuffer,nPnts);
			else
				hSymbolBorderPen = GWPolygonD (hDC,lpDCurPoints,nPnts,nPoly,hPolyPartLen,CurrentDesc,ShowBorder,DoFill,&BorderSymbolNum);  
			if (CurrentRefno == ShowNodesRef)
			{
				ShowNodePoints (hDC,nPnts,lpDCurPoints,hUnSplinedPoly,nUnSplinedPoints,TRUE);
			}
		}
		else
			hSymbolBorderPen = GWPolygon (hDC,lpCurPoints,nPnts,nPoly,hPolyPartLen,CurrentDesc,ShowBorder,DoFill,&BorderSymbolNum);  
		if (hDC)
		{
			SetBkMode (hDC,OPAQUE);
			SetROP2(hDC,DisplayRasterOpt);
		}
		if (nPoly || ForceBorder || ShowBorder || BorderSymbolNum)
		{   
			if (Display && !Pick && hDC)
			{   
				if (ItemIsHighlighted && HighlightWidth < 0)//shows highlighted area with border line
					hSavePen = SelectObject (hDC,HighlightBrush);
				else if (hSymbolBorderPen/* && CurView->FileFactor < 2*/) 
				{
					hSavePen = SelectObject (hDC,hSymbolBorderPen);
					//ShowBorder = TRUE;
				}
				else if (ThemePolyPen)
					hSavePen = SelectObject (hDC,ThemePolyPen);
				else if (hTempPen)
					hSavePen = SelectObject (hDC,hTempPen);
				else if (ShowBorder && hAreaBorderPen[HiPrecis])// && !GetTypeVisibility(TYPE_SYMBOL)) 
					hSavePen = SelectObject (hDC,hAreaBorderPen[HiPrecis]);
				else if (hRandPen)
					hSavePen = SelectObject (hDC,hRandPen);
				else if (!ComputePCTTheme) //Oct 13,99
				    SelectObject(CurView->hDC,GetStockObject(NULL_PEN));
				else if (ipen > 0 && pens[ipen])
					SelectObject(hDC, pens[ipen]); 
	            else
					hSavePen = SelectObject(hDC, h0Pen);
			}
			if (ShowBorder || BorderSymbolNum)
			{
				if (nPoly)
				{
					LPINT	pPartLen = (LPINT)GlobalLock (hPolyPartLen);

					for (i=0;i<nPoly;i++)
					{ 
						int	np = *pPartLen;
						int	dec = 0;

						if (BorderSymbolNum > 10000)
						{
							np = -np;
							dec = 10000;
						}
						SetSavedGraphicsFid (SymbolDisplayPos(BorderSymbolNum-dec));
						if (HiPrecis) 
						{
							GWPolylineD (hDC,lpDCurPoints,np,BorderSymbolNum-dec); 
							lpDCurPoints+=*pPartLen++;
							if (i)
								lpDCurPoints++;
						}
						else
						{
							GWPolyline (hDC,lpCurPoints,np,BorderSymbolNum-dec); 
							lpCurPoints+=*pPartLen++;
							if (i)
								lpCurPoints++;
						}
					}   
					GlobalUnlock (hPolyPartLen); 
				}
				else if (nPnts)
				{
					int	np = nPnts;
					int	dec = 0;

					if (BorderSymbolNum > 10000)
					{
						np = -np;
						dec = 10000;
					}
					SetSavedGraphicsFid (SymbolDisplayPos(BorderSymbolNum-dec));
					if (HiPrecis)
					{
						DPOINT	LastLine[2];
						
						LastLine[0]=*lpDCurPoints;
						LastLine[1]=lpDCurPoints[nPnts-1];
						GWPolylineD (hDC,lpDCurPoints,np,BorderSymbolNum-dec);
						if (np < 0)
							GWPolylineD (hDC,LastLine,-2,BorderSymbolNum-dec);
						else
							GWPolylineD (hDC,LastLine,2,BorderSymbolNum-dec);
					} 
					else
					{
						POINTS	LastLine[2];
						
						LastLine[0]=*lpCurPoints;
						LastLine[1]=lpCurPoints[nPnts-1];
						GWPolyline (hDC,lpCurPoints,np,BorderSymbolNum-dec);
						if (np < 0)
							GWPolyline (hDC,LastLine,-2,BorderSymbolNum-dec); 
						else
							GWPolyline (hDC,LastLine,2,BorderSymbolNum-dec); 
					}
					SelectObject(CurView->hDC,GetStockObject(BLACK_PEN));
				}
			}
		}
	}
	else if (ShowBorder)
	{   
		BOOL	AreaIsNull = FALSE;
		if (Display && !Pick && hDC)
		{
			if (ThemePolyPen)
				hSavePen = SelectObject (hDC,ThemePolyPen);
			else if (hSymbolBorderPen)
				hSavePen = SelectObject (hDC,hSymbolBorderPen);
			else if (hSpecialPen)
				hSavePen = SelectObject (hDC,hSpecialPen);
			else
			{
				HBRUSH	hBrush;
				int		BorderSymNum;

				AreaIsNull = SetAreaPenAndBrush (hDC,0,CurrentDesc,ItemIsHighlighted,ShowBorder,&hSymbolBorderPen,&hBrush,&BorderSymNum); 
				if (hBrush)
					GSSiDeleteObject (&hBrush);
                if (hSymbolBorderPen)
					hSavePen = SelectObject (hDC,hSymbolBorderPen);
				else
					hSavePen = SelectObject (hDC,hAreaBorderPen[HiPrecis]);  
			}
			if (nPoly)
			{
				LPINT	pPartLen = (LPINT)GlobalLock (hPolyPartLen);

				for (i=0;i<nPoly;i++)
				{   
					if (HiPrecis)
					{
						GWPolylineD (hDC,lpDCurPoints,*pPartLen,0); 
						lpDCurPoints+=*pPartLen++;
						if (i)
							lpDCurPoints++;
					}
					else
					{
						GWPolyline (hDC,lpCurPoints,*pPartLen,0); 
						lpCurPoints+=*pPartLen++;
						if (i)
							lpCurPoints++;
					}
				}   
				GlobalUnlock (hPolyPartLen);
			}
			else
			{
				if (HiPrecis)
				{
					DPOINT	LastLine[2];
					
					LastLine[0]=*lpDCurPoints;
					LastLine[1]=lpDCurPoints[nPnts-1];
					GWPolylineD (hDC,lpDCurPoints,nPnts,0);
					GWPolylineD (hDC,LastLine,2,0);
				} 
				else
				{
					POINTS	LastLine[2];
					
					LastLine[0]=*lpCurPoints;
					LastLine[1]=lpCurPoints[nPnts-1];
					GWPolyline (hDC,lpCurPoints,nPnts,0);
					GWPolyline (hDC,LastLine,2,0); 
				}
			}
		} 
	}
	if (hSavePen)
		SelectObject (hDC,hSavePen);
	GSSiDeleteObject (&hSymbolBorderPen);
	HiPrecis = HiPrecisSave;
	GSSiGlobUlFree (&hHiPrecis);
	return rtn;
}

BOOL ProcessGraphicsRec (HDC hDC, LPSHORT ipnt,LPSTR BeginSeg,int LenSeg)
#if ENABLETRACE
{GSSiEnterProg (686);
#endif
{	int		idesc, ItemLen, ipen;
    LPITEM  ItemHeader; 
    LPSHORT	FieldPnt;
	LPLONG	pRefno=0;  
	LPSHORT	pDesc=0;
	long	remlen;
	LPBYTE	Pcode;
	short	x1,y1,x2,y2;  
	DPOINT	xdPoint;
	LPTEXTTPL	lpTextTPL;
	LPSTR	lpTAG, pTXChar, lpColon;  
	static	HPEN	hTempPen=0;
	HPPOINTS lpPoints;
	HPDPOINT pUSPoint; 
	int		ltag, len, pwidth;
	int		TempPattern,ii; 
	clock_t	starttime, endtime, starttime2;
	static	BOOL	AlreadyProcessed;
	HANDLE	hMem=0; 
	LPSTR	str;
	long	TotBlockLen=0, LastItemLen=0, LastItemLenActual; 
	WORD	LastCurrentItem=0; 
	static	long	StartElement=-1; 
	static	BOOL	Deleted=FALSE;   
	static	long	debugrefno=300367499,debugitem=2744; 
	HPSTR	pCoords;  
	short	i;
	static	POINTS	LinkPoint;
	static	DPOINT	LinkPointD;
	POINT	WinPoint;
	LPINT	pPartLen;
	BOOL	rtn=FALSE, Closed=TRUE; 
	DPOINT	SaveCurrentPoint=CurrentPoint;
	long	PointSize;
	double	BackAZ;
	double	size; 
	static	BOOL	SkipToNextHeader=FALSE; 
	static	HPEN	SaveCurrentPen=0; 
	static	long	DebugElement=8700; 
	static	long	PolyBufferLen;
	static	COLORREF	RouteColor;
	MNMXCORD RecordBounds;
	double   RecordLength;


/*typedef struct {short i2array[100];} DEBUGARRAY;
typedef DEBUGARRAY	FAR	*LPDEBUGARRAY;
LPDEBUGARRAY	pDB=0;   */

   	if (ShowRefno)
   		SetWindowText (hWndMain,"Enter PGR");
   // if (DoTime)
    	starttime=GetTickCount();   
    if (!LenSeg)
    	LenSeg = INT_MAX;
	InGraphicsProcessor = TRUE;
	ShowValue (hDC,TRUE);
	/*if (Display && !Pick && hDC && CurrentPen)
	{
        if (!SelectObject(hDC, CurrentPen))
        	ii=1;
	} */  
		if (ipnt && StartElement > 0)
			ipnt += StartElement;
		StartElement = -1;
		if (FidMap != HFILE_ERROR)
        while (ipnt && ((LPSTR)ipnt - BeginSeg) <= LenSeg && *ipnt != 0 && ContinueProcessing)
        {   
			DWORD ptime = GetTickCount();
			if (ptime - starttime > 1000)
			{
				starttime = ptime;
				SetContinueProcessing(CheckForContinue(FALSE, 0));
			}
//        	if (DoTime)
//        		starttime2=GetTickCount();
//        	starttime2 = clock();  this call alone slows system by 150%     
			HiPrecis = FALSE; 
			PointSize = 4;
        	Pcode = (LPBYTE) ipnt;
        	FieldPnt = ipnt;
        	CurElementPnt = ipnt++; 
			CurElement=(LPSTR) ipnt - (LPSTR)(BeginSeg +2);
            if (CurElement >= DebugElement)
            	ii=1;   
            if (SkipToNextHeader && (*Pcode != 12 && *Pcode != 92 && *Pcode != 13))
				SkipSubRec (Pcode,(HPSHORT*)&ipnt,0);            
            else if (CopyRec == 1)
            	rtn = CopySubRec (Pcode,(HPSHORT*)&ipnt);
            else
			{
				if (CopyRec == 2)
				{
					HPSHORT Saveipnt = ipnt;
					LPBYTE	SavePcode=Pcode;
					BYTE	Savecode = *Pcode;
					int		SavenPolyPoints = nPolyPoints;


	            	CopySubRec (Pcode,(HPSHORT*)&ipnt);
					ipnt = Saveipnt;
					Pcode = SavePcode;
					*Pcode = Savecode;
					nPolyPoints = SavenPolyPoints;
				}

				if (FidMap != HFILE_ERROR)
				switch (*Pcode)
			  {   
	            case 2: /* pen up */
		 		{
					ipnt+=2;
					pu++;
				}
	            break;

	            case 3: /* set pen */
		 		{   ipen = *ipnt;   
		 			CurrentiPen = ipen;
		 			ipnt++;
		 			CurrentPen = pens[ipen]; 
		 			if (!CurrentPen)
		 				CurrentPen = pens[0];
				}
	            break;

	            case 4: /* put line */
		 		{
					nPnts = 2;
		 		    lpCurPoints = (HPPOINTS) ipnt;
 		    		ipnt = ipnt + nPnts * 2;  
 		    		CurrentType = GF_LINE;
 		    		goto ProcessPolyLine;
				}
	            break;
	            
                case 41:
		 		{
					nPnts = 2;
                	HiPrecis = TRUE;
                	PointSize = 16;
		 		    lpDCurPoints = (HPDPOINT) ipnt;
 		    		ipnt = ipnt + nPnts * 8;  
 		    		CurrentType = GF_LINE;
 		    		goto ProcessPolyLine;
				}
	            break;
	            
				case 151: //Route Area (not closed)
					RouteOffset = *(LPDOUBLE)ipnt;
					ipnt += 4;
					RouteColor = *(LPCOLORREF)ipnt;
					ipnt += 2;
					pwidth = BaseDistToWinDist * RouteOffset;
					GSSiDeleteObject (&hTempPen);
					hTempPen = CreatePen(PS_SOLID,pwidth,ConvertColor(RouteColor,CurrentDesc)); 
					break;

                case 51:
                	HiPrecis = TRUE;
                	PointSize = 16;
	            case 5: /* put area */

		 		{   
		 			PolyIsHiPrecis = HiPrecis;
		 			ipen = *ipnt; 
		 			CurrentiPen = ipen;
		 			ipnt++;
		 		    nPnts = *ipnt;
		 		    ipnt++;
   		 		    if (Visible && CurrentDesc > 0 && CurrentDesc < 3201)
						CurView->CurVisType[CurrentDesc]=3;
		 		    if (HiPrecis)
		 		    {
			 		    lpDCurPoints = (HPDPOINT)ipnt;
			 		    ipnt = ipnt + nPnts * 8;
			 		}
			 		else
			 		{
			 		    lpCurPoints = (HPPOINTS)ipnt;
			 		    ipnt = ipnt + nPnts * 2;
			 		}
		 		    CurrentType = GF_AREA; 
		 		    ThemePolyPen = 0; 
		 		    
				    if (hCoords) 
				    {
				    	nCoords += nPnts;  
				    	GlobalUnlock (hCoords);
				    	hCoords = GSSiGlobalReAlloc (0,hCoords,(long)nCoords*PointSize,GMEM_MOVEABLE);
					    pCoords = GlobalLock (hCoords);
					    pCoords += lCoords;
				    	if (HiPrecis)
				    	{
						    BufWrite (&pCoords,&lCoords,(HPSTR)lpDCurPoints,nPnts*PointSize);  
						    GlobalUnlock (hCoords);
						    lpDCurPoints = (HPDPOINT)GlobalLock (hCoords);
						}
						else
						{
						    BufWrite (&pCoords,&lCoords,(HPSTR)lpCurPoints,(long)nPnts*PointSize);  
						    GlobalUnlock (hCoords);
						    lpCurPoints = (HPPOINTS)GlobalLock (hCoords); 
						}
					    nPnts = nCoords;
				    }
		 		    if (HiPrecis && nPnts == 2 && nPoly == 0 && !hCoords)
					{
						double	Radius = ldistpp (lpDCurPoints,lpDCurPoints+1);
						LPDPOINT	pUSPoints;
						if (Radius)
						{
							GSSiGlobFree (&hUnSplinedPoly);
							hUnSplinedPoly = GSSiGlobAlloc(GAIDNO 0,GMEM_MOVEABLE,2*sizeof(DPOINT));
							pUSPoints = GlobalLock (hUnSplinedPoly);
							*pUSPoints++ = *lpDCurPoints;
							*pUSPoints = *(lpDCurPoints+1);
							GlobalUnlock (hUnSplinedPoly);
							nUnSplinedPoints = 2;

							hCoords = CreateCirclePoly (*lpDCurPoints,Radius,&nPnts,0);
							lpDCurPoints = (HPDPOINT)GlobalLock (hCoords);
						}
					}
		 		    
		 		    if (DescIsVisible && Visible)
		 		    {   
		 		    	HPEN hSavePen=0, hRandPen=0;
						HPEN hSymbolBorderPen = 0;
						
						if (nPoly)
						{
							HPPOINTS	lpPoints3; 
							HPDPOINT	lpDPoints3; 
							long	lbuf=0;
							
							pCoords = GlobalLock (hPolyBuffer);
							pCoords += ((long)nPolyPoints*PointSize);
							pPartLen = (LPINT)GlobalLock (hPolyPartLen);
							pPartLen+=iPoly;
							*pPartLen = nPnts;
							GlobalUnlock (hPolyPartLen);
							if (HiPrecis)
							{  
								BufWrite (&pCoords,&lbuf,(HPSTR)lpDCurPoints,(long)nPnts*PointSize); 
								if (!iPoly)
									LinkPointD = *lpDCurPoints;
								else
								{
									lpDPoints3 = (HPDPOINT)pCoords;
									*lpDPoints3 = LinkPointD;
									nPolyPoints++; 
								}
							}
							else
							{
								BufWrite (&pCoords,&lbuf,(HPSTR)lpCurPoints,(long)nPnts*PointSize); 
								if (!iPoly)
									LinkPoint = *lpCurPoints;
								else
								{
									lpPoints3 = (HPPOINTS)pCoords;
									*lpPoints3 = LinkPoint;
									nPolyPoints++; 
								}
							}
							nPolyPoints += nPnts; 
							iPoly++;   
							GlobalUnlock (hPolyBuffer);
						}           
						if (iPoly == nPoly)      
						{    
							BOOL	ShowBorder = GetBit (5,(LPSTR)&CurVis->WantType[7]);
	
							if (Display && !Pick && hDC)
							{	
								if (ShowBorder)
								{
									if (nPoly && !ShowLinkLines)
										hRandPen = (h0Pen);
									else 
									{   
				        				hRandPen = hAreaBorderPen[HiPrecis];
									}
									hSavePen = SelectObject(hDC, hRandPen); 
					        	} 
					        	else if (nPoly)
					        	{
									hSavePen = SelectObject(hDC, h0Pen); 
									hRandPen = h0Pen;
								}
					        	else if (hTempPen)
					        		hSavePen = SelectObject (hDC,hTempPen);
				        		else if (ipen > 0 && pens[ipen])
									SelectObject(hDC, pens[ipen]); 
					        	else
									hSavePen = 0;
								if (!hTempBrush && !CurView->HaveLayerColor[FileNum]) 
								{
									if (ipen > 0) 
									{                       
										if (brushes[ipen])
											SelectObject(hDC, brushes[ipen]); 
										else
											SelectObject(hDC, hRedBrush);
									} 
									if (ipen == -1)
									{ 
										SelectRandomBrush (hDC,CurrentRefno,&hRandPen,CurrentDesc);
									}
								} 
								else if (hTempBrush)
					        		hOldBrush = SelectObject (hDC,hTempBrush);
								
							}
							if (nPoly)
							{   
								if (HiPrecis)
									lpDCurPoints = (HPDPOINT)GlobalLock (hPolyBuffer);
								else    
									lpCurPoints = (HPPOINTS)GlobalLock (hPolyBuffer);    
								nPnts = nPolyPoints;
							} 
				 		    if (hCurvePoints)
				 		    {   
				 		    	HANDLE	hOldPoints=hCoords;
				 		    	HPDPOINT	pOldPoints, pOldPointsBeg=lpDCurPoints; 
				 		    	LPSHORT	pCurvePoints;
				 		    	short	nOldPnts = nPnts, LoopFactor = 1;  
				 		    	DPOINT	BP, POC, EP, FirstPoint = *pOldPointsBeg; 
				 		    	USHORT	nxtpl=0, lastpl = 0, nextlinkloc;

				 		    	GSSiGlobFree (&hUnSplinedPoly);
				 		    	hUnSplinedPoly = GSSiGlobAlloc(GAIDNO 300,GMEM_MOVEABLE,sizeof(DPOINT)*nPnts); 
				 		    	pUSPoint = (HPDPOINT)GlobalLock (hUnSplinedPoly);
				 		    	hmemmove ((HPSTR)pUSPoint,(HPSTR)pOldPointsBeg,sizeof(DPOINT)*nPnts);
				 		    	GlobalUnlock (hUnSplinedPoly);   
				 		    	nUnSplinedPoints = nPnts;  
			NextCurvedAreaLoop:
				 		    	nPnts = 0;
				 		    	pOldPoints=pOldPointsBeg;  
				 		    	pCurvePoints = (LPSHORT)GlobalLock (hCurvePoints);
				 		    	hCoords = GSSiGlobAlloc(GAIDNO 301,GMEM_MOVEABLE,(long)sizeof(DPOINT)*(long)MAX_POLY_POINTS);
							    lpDCurPoints = (HPDPOINT)GlobalLock (hCoords);  
							    if (nPoly) 
							    {
						    		pPartLen = (LPINT)GlobalLock (hPolyPartLen);   
						    		nextlinkloc = *pPartLen;
						    	}
							    for (i=0;i<nOldPnts;i++)
							    {   
							    	if (nPoly) 
							    	{
							    		if (nextlinkloc == i)
							    		{   
							    			nextlinkloc += pPartLen[nxtpl]+1;
							    			pPartLen[nxtpl++] = nPnts - lastpl;
							    			lastpl = nPnts;
							    		}
                                    }
							    	if (i == *pCurvePoints)
							    	{   
							    		POC = *pOldPoints++; 
							    		if (i + 1 >= nOldPnts)
							    			EP = FirstPoint;
							    		else
							    			EP = *pOldPoints;
							    		lpDCurPoints--;
					                    if (!CurvePointsD(&BP,&POC,&EP, &nPnts, &lpDCurPoints,&BackAZ,(long)MAX_POLY_POINTS-nPnts-(nOldPnts-i)-1,DisplayCurveFactor,LoopFactor))
					                    {
					                    	GSSiGlobUlFree (&hCoords);
										    GlobalUnlock (hCurvePoints); 
					                    	LoopFactor++;
					                    	goto NextCurvedAreaLoop;
					                    }
					                    lpDCurPoints--;
					                    nPnts-=2;
					                    pCurvePoints++;
					                    BP = EP;
							    	}
							    	else
							    	{ 
							    		BP = *lpDCurPoints++ = *pOldPoints++; 
							    		nPnts++;
							    	}
							    }          
							    if (nPoly) 
						    		GlobalUnlock (hPolyPartLen);
							    GlobalUnlock (hCurvePoints); 
							    GSSiGlobUlFree (&hOldPoints);  
							    GlobalUnlock (hCoords);
							    lpDCurPoints = (HPDPOINT)GlobalLock (hCoords);  
				 		    } 
							nCurPoints = nPnts; 
							if (nPnts > SHRT_MAX)
								ii=1;
							if (FileMode)
							{	
								HANDLE	hPoints; 
								HPPOINTS	lpPoints3; 
								long	i;
								
							    hPoints = GSSiGlobAlloc(GAIDNO 302,GMEM_MOVEABLE,(long)nPnts*sizeof(POINT));
							 	lpPoints3 = (HPPOINTS) GlobalLock (hPoints);
							 	lpPoints = lpPoints3; 
							 	if (PointSize == 16)
							 	{
							 		HPDPOINT	lpPoints4 = lpDCurPoints;
							 		
									for (i=0;i<nPnts;i++,lpPoints++,lpPoints4++)
										*lpPoints = POINTtoPOINTS(BasePtToWinPt (lpPoints4));
								}
							 	else 
							 	{
							 		HPPOINTS lpPoints4=lpCurPoints;
							 		
									for (i=0;i<nPnts;i++,lpPoints++,lpPoints4++)
									{   
										xdPoint = FilePtToBasePt (POINTStoPOINT(*lpPoints4));
										*lpPoints = POINTtoPOINTS(BasePtToWinPt (&xdPoint));
									}
								}
								if (ItemInRegion (hRgn,lpPoints3, nPnts))
								{
									int SDCrtn = SetDisplayChar (hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI);

									if (SDCrtn > 0)
									{
										GWPolygon (hDC,lpPoints3,nPnts,nPoly,hPolyPartLen,CurrentDesc,ShowBorder,TRUE,0);
										HaveTXLoc = TRUE;
									}
									else if (SDCrtn < 0)
										HaveTXLoc = TRUE;
								}
								GSSiGlobUlFree (&hPoints);
							}
							else 
							{
								if (Pick)
								{
									if (HiPrecis)
										PickPolygonD (lpDCurPoints,nPnts,nPoly,hPolyPartLen,pickedAreaOffsetDist,hElevBuffer);
									else
										PickPolygon (lpCurPoints,nPnts, pickedAreaOffsetDist);
								}
								else if (PolyInMaskAreaFileCoord (CurrentType,&nPnts,&hCoords,&lpCurPoints,&lpDCurPoints,HiPrecis))
								{
									int	SDCrtn = SetDisplayChar (hDC,GF_AREA,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI);
									
									if (SDCrtn > 0)
									{
										if (hElevBuffer && nPnts == 3 && FidTINExtract != HFILE_ERROR)
										{
											SaveTINData(lpDCurPoints, hElevBuffer);
										}
										ProcessPolygon(hDC, ShowBorder, PltType, hRandPen, hTempPen, ipen);
									}
									else if (SDCrtn < 0)
										HaveTXLoc = TRUE;
								}
							}
							if (hPolyBuffer)
							{
								GSSiGlobUlFree (&hPolyBuffer);
								GSSiGlobFree (&hPolyPartLen);
								nPoly = 0;
							}
						}
						if (hSavePen && hDC)
							SelectObject(hDC, hSavePen);  
						if (hSymbolBorderPen != (HPEN)HighlightBrush && hSymbolBorderPen != (HPEN)GetStockObject(NULL_PEN))
							GSSiDeleteObject (&hSymbolBorderPen);
					}
					GSSiGlobUlFree (&hCoords);
				}
				break;
                
                case 61:
                	HiPrecis = TRUE;
                	PointSize = 16;
	            case 6: /* put polyline */

		 		{   pnPnts = ipnt;
		 			nPnts = *pnPnts;
		 		    ipnt++; 
	TwoPointArea:
		 		    if (HiPrecis)
		 		    {
		 		    	lpDCurPoints = (HPDPOINT) ipnt; //lpDCurPoints[nPnts-2]
			 		    ipnt = ipnt + nPnts * 8;        
			 		}
		 		    else 
		 		    {   
		 		    	if ((((LPSTR)ipnt - BeginSeg) + (long)nPnts * 2) > LenSeg)
	                	{
	                		ProcessInvalidRecord ((short)*Pcode,ipnt,1);
	                		break;
	                	}
		 		    
			 		    lpCurPoints = (HPPOINTS) ipnt;
			 		    ipnt = ipnt + nPnts * 2; 
			 		}
		 		    CurrentType = GF_POLYLINE;
				    if (hCoords) 
				    {   
				    	pnPnts = 0;
				    	nCoords += nPnts; 
				    	GlobalUnlock (hCoords);
				    	hCoords = GSSiGlobalReAlloc (0,hCoords,(long)nCoords*PointSize,GMEM_MOVEABLE);
					    pCoords = GlobalLock (hCoords);
					    pCoords += lCoords;
				    	if (HiPrecis)
				    	{
						    BufWrite (&pCoords,&lCoords,(HPSTR)lpDCurPoints,nPnts*PointSize);  
						    GlobalUnlock (hCoords);
						    lpDCurPoints = (HPDPOINT)GlobalLock (hCoords);
						}
				    	else
				    	{
						    BufWrite (&pCoords,&lCoords,(HPSTR)lpCurPoints,nPnts*PointSize);  
						    GlobalUnlock (hCoords);
						    lpCurPoints = (HPPOINTS)GlobalLock (hCoords);
						}
					    nPnts = nCoords;
				    }
				ProcessPolyLine:
					nCurPoints = nPnts;
					if (!nPolySegments)
					{
						DBoundsInit(&RecordBounds);
						RecordLength = 0;
					}
					nPolySegments++;
					PolyIsHiPrecis = HiPrecis;
				    if (Visible && CurrentDesc > 0 && CurrentDesc < 3201)
				    {
						if (TSize)
							CurView->CurVisType[CurrentDesc]=2;
						else
							CurView->CurVisType[CurrentDesc]=1; 
					}
				//5/24/04	if (TSize > 0) CurrentType = GF_TEXT;
					if (Display && !Pick && hDC)
					{
						if (hTempPen)
						{   
							SaveCurrentPen = CurrentPen;
							CurrentPen = hTempPen;
				       		hOldPen = SelectObject (hDC,hTempPen);
				       	}
				    }
					if ((GetTypeVisibility(TYPE_LINECURVE)||RouteOffset) && DescIsVisible && Visible & !AlreadyProcessed)
		 		    {
						if (FileMode)
						{	
							HANDLE	hPoints; 
							HPPOINTS	lpPoints3; 
							UINT	i;
							
						    hPoints = GSSiGlobAlloc(GAIDNO 303,GMEM_MOVEABLE,(long)nPnts*PointSize);
						 	lpPoints3 = (HPPOINTS) GlobalLock (hPoints);
						 	lpPoints = lpPoints3;
						 	if (PointSize == 16)
						 	{
						 		HPDPOINT	lpPoints4 = lpDCurPoints;
							 		
								for (i=0;i<nPnts;i++,lpPoints++,lpPoints4++)
									*lpPoints = POINTtoPOINTS(BasePtToWinPt (lpPoints4));
							}
						 	else 
						 	{
						 		HPPOINTS lpPoints4=lpCurPoints;
							 		
								for (i=0;i<nPnts;i++,lpPoints++,lpPoints4++)
								{   
									xdPoint = FilePtToBasePt (POINTStoPOINT(*lpPoints4));
									*lpPoints = POINTtoPOINTS(BasePtToWinPt (&xdPoint));
								}
							}
							if (ItemInRegion (hRgn,lpPoints3,nPnts))
							{
								int SDCrtn = SetDisplayChar (hDC,GF_POLYLINE,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI);
					 		    
								if (SDCrtn > 0)
								{
									GWPolyline (hDC,lpPoints3,nPnts,CurrentDesc); 
									HaveTXLoc = TRUE;
								}
								else if (SDCrtn < 0)
									HaveTXLoc = TRUE;
							}
							GSSiGlobUlFree (&hPoints);
						}

						else 
						{                               
							if (Pick)
							{
								if (HiPrecis)
									PickPolylineD (lpDCurPoints,nPnts, nPolySegments-1,2,0,0,0, &RecordBounds,&RecordLength);
								else
									PickPolyline (lpCurPoints,nPnts, nPolySegments - 1,2,0,0);
							}
							else if (PolyInMaskAreaFileCoord (CurrentType,&nPnts,&hCoords,&lpCurPoints,&lpDCurPoints,HiPrecis))
							{
								if (debugLineDir)
									LastRef = 0;

								int SDCrtn = SetDisplayChar (hDC,CurrentType,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI);
								if (SDCrtn)
								{ 
									HaveTXLoc = TRUE;
									if (hDynamicSeg)
										do
										{   
											HPDPOINT pDynamicCoord=(HPDPOINT)GlobalLock (hDynamicSeg);
											
											GWPolylineD (hDC,pDynamicCoord,nDynamicCoord,CurrentDesc);
											GlobalUnlock (hDynamicSeg);
											SetDisplayChar (hDC,CurrentType,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI);
										}
										while (hDynamicSeg);
									else
									{
										if (HiPrecis)  
										{
											int	desc = CurrentDesc;

											if (hTempPen)
												desc = 0;
											GWPolylineD (hDC,lpDCurPoints,nPnts,desc);
											if (CurrentRefno == ShowNodesRef)
											{
												ShowNodePoints (hDC,nPnts,lpDCurPoints,hUnSplinedPoly,nUnSplinedPoints,FALSE);
											}
										}
										else
											GWPolyline (hDC,lpCurPoints,nPnts,CurrentDesc);//lpCurPoints[8]
									}  
								}
								else if (SDCrtn < 0)
									HaveTXLoc = TRUE;

							}
						} 
					}
					GSSiGlobUlFree (&hCoords);
				}
				break;

				case 7: /* block minmax */
				{	pMinMax = (LPMINMAX) ipnt;
					ipnt += 4;
					if (!BlockInWindow (pMinMax,2)) *ipnt = 0;
                }
                break;

                case 8:	/*	description */
		 		{
	    			CurDescLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
	    			pDesc = ipnt;  
		 		    idesc = *ipnt++;
		 			SetSymNum (idesc);
//		 			DescScan(idesc,0);
                	DescIsVisible = GetVisibility (idesc); 
       				if (!pRefno)
       					break;
       ProcessRefno:
       				Visible = ProcessRefAndTAG (DescIsVisible,lpTAG,ltag);
                	if (!Visible && !(ForceRefIndex || ForceTAGIndex || PickingByRefno))
	                   	SkipToNextHeader=TRUE;
		 			if (hDC && Display && Visible)
		 			{
	 					SetROP2(hDC,DisplayRasterOpt);
						if (HaveVarFillColor)
		        			SetTextColor (hDC,ConvertColor(GlobalColors[0],CurrentDesc));
		        		else
		        			SetTextColor (hDC,ConvertColor(DefaultTextColor,CurrentDesc));
        				SelectObject (hDC,GetStockObject(BLACK_PEN));
        				GSSiDeleteObject (&hBlackPen);
        				hBlackPen = CurrentPen = CreatePen (PS_SOLID,0,ConvertColor(0,CurrentDesc));		
        				SelectObject (hDC,hBlackPen);
	        		}
                }
                break;

                case 9:	/*	refno	*/
                {   
	    			CurTAGLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
                	pRefno =(LPLONG) ipnt;
                	CurrentRefno = *pRefno;  
                	if (HighlightMultRefs)
                		CurrentRefno = HighlightMultRefs++;
                	if (CurrentRefno==debugrefno)
                		ii=1; 
                	ipnt += 2;    
                	ltag = *++Pcode;   
               		lpTAG = (LPSTR)ipnt;
					ipnt = (LPSHORT) (lpTAG + ltag + ltag%2);   
					if (pDesc)
						goto ProcessRefno;
			    }
                break;    
                
	            case 10: /* street number */
		 		{   
		 			long	snums[4];
		 			
	    			CurSNamesLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
		 			_fmemmove (CurStreetNumbers,(LPLONG)ipnt,16); 
		 			_fmemmove (snums,(LPLONG)ipnt,16); 
		 			if (snums[1])
		 				ii=1;
					ipnt+=(2*4);
				}
	            break;
                
                case 93:
					if (HaveFirstHeader)
					{   
						HaveFirstHeader=FALSE;
						*ipnt=0;
						break;
					}
                	ItemIsRemoved = TRUE;
                	goto ProcessHeader;
                case 92: /* deleted item */
					if (HaveFirstHeader)
					{   
						HaveFirstHeader=FALSE;
						*ipnt=0;
						break;
					}
                	Deleted = TRUE;  
				//	if (!ShowDeletedOpt)
						ItemIsDeleted = TRUE;  
					ItemIsRemoved = FALSE;              	
                	goto ProcessHeader;
				case 12: /* item minmax */
				{	
					if (HaveFirstHeader)
					{   
						HaveFirstHeader=FALSE;
						*ipnt=0;
						break;
					}
					ItemIsDeleted = Deleted = FALSE;
					ItemIsRemoved = FALSE;              	
	ProcessHeader:  
					ShowValue (hDC,FALSE);
				    ItemHeader = (LPITEM) FieldPnt;
					if (ProcessSingleItem)
						HaveFirstHeader=TRUE;
					SkipToNextHeader=FALSE;
					pMinMax = (LPMINMAX)ipnt;
					CurrentItemMinMax = *pMinMax;  
					if (MinPickItemWidth)
					{
						long	iw = max((long)CurrentItemMinMax.xmx - (long)CurrentItemMinMax.xmn,(long)CurrentItemMinMax.ymx - (long)CurrentItemMinMax.ymn);
						
						if (iw < MinPickItemWidth)
						{   
							long	dif = (MinPickItemWidth - iw)/2;
							
							CurrentItemMinMax.xmn = max (SHRT_MIN,(long)CurrentItemMinMax.xmn - dif);
							CurrentItemMinMax.ymn = max (SHRT_MIN,(long)CurrentItemMinMax.ymn - dif);
							CurrentItemMinMax.xmx = min (SHRT_MAX,(long)CurrentItemMinMax.xmx + dif);
							CurrentItemMinMax.ymx = min (SHRT_MAX,(long)CurrentItemMinMax.ymx + dif);
						}
					}
					ItemSeg = CurrentSeg;
					CurrentItem=(LPSTR) ipnt - (LPSTR)(BeginSeg +2);   
					if (CurrentItem == debugitem)
						ii=1;
					LastItemLenActual = CurrentItem - LastCurrentItem; 
					if (LastItemLen && (LastItemLenActual != LastItemLen))
						LogItemLengthError (LastItemLen,LastItemLenActual);
					LastCurrentItem = CurrentItem;
	    			CurItemHeadLoc = CurrentSeg + (long)((LPSTR) ItemHeader - (LPSTR)BeginSeg); 
					ipnt += 4; 
					if (*ipnt < 0)  
						CurrentChangeDate=1;
					else
						CurrentChangeDate=0;
					ItemLen = abs(*ipnt);                                          
					if (!ItemLen)
						LastItemLen=0; 
					else
						LastItemLen = (ItemLen + 6)*2;
					ipnt++;  
					TotBlockLen += LastItemLen;    
					AlreadyProcessed=FALSE;    
					pRefno=0;
					pDesc=0;
					InitRecord (hDC);
					GSSiDeleteObject (&hTempPen);
					if (ReorgfileFID != HFILE_ERROR)
					{   
						if (!Deleted && !ItemIsRemoved)
						{
							if (!ItemLen)
							{   
								long	ActualLength=GetActualItemLength((LPSHORT)Pcode);
								HANDLE	hSeg = GSSiGlobAlloc(GAIDNO 305,GMEM_MOVEABLE,ActualLength);
								HPSTR	pSeg = GlobalLock (hSeg);
								
								LoadCompleteItem (pSeg,(LPSHORT)Pcode,ActualLength);
								ReorgOut2 (pSeg,ActualLength);
								GSSiGlobUlFree (&hSeg);
								if (ItemLen < 0)
									goto Exit;
							}
							else  
								ReorgOut2 (Pcode,(long)ItemLen*2 + 12);
						}
						remlen = nBytes - ((LPSTR) ipnt - BeginSeg); 
						if (ItemLen >= remlen)
							ipnt++; 
						if (ItemLen)
							ipnt+=ItemLen;
						else
							SkipToNextHeader = TRUE;

					}
					else if (ItemIsRemoved || !BlockInWindow (&CurrentItemMinMax,1))
					{	
						remlen = nBytes - ((LPSTR) ipnt - BeginSeg); 
						if (!UseItemLen)
							ItemLen = 0;
						if (ItemLen >= remlen)
							ipnt++;
						ipnt+=ItemLen;
                        nBlocksOut++;  
                        if (!ItemLen)
                        	SkipToNextHeader=TRUE;
					}
					else
					{
			 			CurrentiPen = *++Pcode;
			 			if (CurrentiPen)       //     6/26/03 for halftone
			 				CurrentPen = pens[CurrentiPen];
			 			if (!CurrentPen || !CurrentiPen || CurrentiPen == 255)
			 				CurrentPen = pens[0];
			 			else
			 				PenFromUMList = TRUE;  
			 			if (PeopleNet)
			 				CurrentPen = pens[0];
			 			nBlocksIn++;
			 		}
                }
                break;

				case 13: /* continuation offset */
				{	
					ShowValue (hDC,FALSE);
					LastItemLenActual = (LPSTR) ipnt - (LPSTR)(BeginSeg +2) - LastCurrentItem; 
					if (LastItemLen && (LastItemLenActual != LastItemLen))
						LogItemLengthError (LastItemLen,LastItemLenActual);
					ContinuationOffset = *(LPLONG)ipnt; 
					if (ContinuationOffset >=0)  
					{
						static	int debugco=15516;
						Numcont++; 
						rtn = TRUE; 
						if (ContinuationOffset == debugco)
							ii=1;
					} 
					else
						SkipToNextHeader=FALSE;
				/*	ContinuationOffset=-1;*/
					ipnt += 2;
					*ipnt = 0;
					TotBlockLen += 2; 
                }
                break;

				case 14: /* text size */
				{
					TSize = *ipnt;
					ipnt++;
                	TextIsVisible = GetTextVisibility (TSize,1);
                	if (DescIsVisible && TextIsVisible && CurVis->WantType[2])
                		Visible=TRUE;
                	else
                		Visible=FALSE;
                }
                break;

				case 15: /* point coordinates */
				{
					CurrentType = GF_POINT; 				
					PointLoc.x = *ipnt++;
					PointLoc.y = *ipnt++;
					WinPoint = FileCoordToWinCoord(PointLoc); 
					PointRot = *ipnt++;
					if (Pick & DescIsVisible && Visible)
					{   
						if (!HighlightMultRefs)
						{
							AlreadyProcessed=TRUE;
							PickPointItem (PointLoc,1,0,CurrentDesc);
						}			 		    
					} 
					else if (Display && DescIsVisible && (!DisplaySymbol && Visible))
					{
						AlreadyProcessed = TRUE;
						DisplayPointItem (hDC,WinPoint,1,PointRot/1000,CurrentDesc,&CurView->MaxSymbolWidth);
					}
                }
                break;

				case 16: /* line coordinates */
				{	
		 		    lpCurPoints = (LPPOINTS) ipnt;
					BPLoc.x = *ipnt++;
					BPLoc.y = *ipnt++;
					EPLoc.x = *ipnt++; 
					EPLoc.y = *ipnt++; 
		 		    nPnts = 2; 
		 		    goto ProcessBaseRec;
                }
                break;
                
                case 172:
                {
					HPDPOINT	pDPoint;
					int		st;
					
                	HiPrecis = TRUE;
                	ipnt++;  
					pCurveBP = (HPDPOINT)ipnt;ipnt+=8;
					BP = *pCurveBP;
					pDPoint = (HPDPOINT)ipnt;ipnt+=8;
					CurPOCW = POC = *pDPoint;
					pDPoint = (HPDPOINT)ipnt;ipnt+=8;
					EP = *pDPoint;
		 		    CurrentType = GF_CURVE;
					if (DescIsVisible && Visible && GetTypeVisibility(TYPE_LINECURVE))  
					{   
						 
			 		    hCoords = GSSiGlobAlloc(GAIDNO 306,GMEM_MOVEABLE,(long)4096*16);
			 		    lpDCurPoints = (HPDPOINT)GlobalLock (hCoords);   
			 		    pDPoint = lpDCurPoints;
		 		    	GSSiGlobFree (&hUnSplinedPoly);
		 		    	hUnSplinedPoly = GSSiGlobAlloc(GAIDNO 300,GMEM_MOVEABLE,sizeof(DPOINT)*3); 
		 		    	pUSPoint = (HPDPOINT)GlobalLock (hUnSplinedPoly);
		 		    	pUSPoint[0] = BP;
		 		    	pUSPoint[1] = POC;
		 		    	pUSPoint[2] = EP;
		 		    	GlobalUnlock (hUnSplinedPoly);   
		 		    	nUnSplinedPoints = 3;  
			 		    nPnts = 0;
						if (!Display)   
		                    st =  CurvePointsD(&BP,&POC,&EP, &nPnts, &pDPoint,&BackAZ,4090,CurveChordDist,1);
	                    else
	                    	st =  CurvePointsD(&BP,&POC,&EP, &nPnts, &pDPoint,&BackAZ,4090,DisplayCurveFactor,1);
						if (Pick)
							PickCurve (lpDCurPoints,nPnts,&BP,&POC,&EP);	
						else 
							goto ProcessPolyLine;
/*						{	
							nCurPoints = nPnts;	 		    
							if (SetDisplayChar (hDC,CurrentType,CurrentRefno,CurrentDesc,CurrentTAG,CurrentUDI))
								GWPolylineD (hDC,lpDCurPoints,nPnts);
						} */
						GSSiGlobUlFree (&hCoords);
					} 
                }
                break;  
                
                case 171: 
                	ipnt++;
				case 17: /* curve coordinates */
				{	
					DPOINT	BP,POC,EP; 
					int		st;
					
					BP.x = *ipnt++;
					BP.y = *ipnt++;
					POC.x = *ipnt++; 
					POC.y = *ipnt++; 
					EP.x = *ipnt++; 
					EP.y = *ipnt++;  
				*Pcode = 171;
					if (*Pcode==17)
						break; //errors in some curve data from UltiMap  
		ProcessBaseRec:
					if (!WantBaseRec)
						break; 
		 		    CurrentType = GF_POLYLINE;
					if (GetTypeVisibility(TYPE_LINECURVE) && DescIsVisible && Visible && (Pick || !DisplaySymbol || *Pcode == 171 || *Pcode == 16))  
					{    
						if (*Pcode == 171)
						{
				 		    hMem = GSSiGlobAlloc(GAIDNO 307,GMEM_MOVEABLE,USHRT_MAX);
				 		    lpCurPoints = (HPPOINTS)GlobalLock (hMem);   
				 		    lpPoints = lpCurPoints;
				 		    nPnts = 0;
		                    st =  CurvePointsS(&BP,&POC,&EP, &nPnts,  &lpPoints, CurveExpansionFactor,USHRT_MAX/4);
					    }
						AlreadyProcessed=TRUE; 
						if (Pick)
							PickPolyline (lpCurPoints,nPnts,0,2,0,0);	
						else
						{
							int SDCrtn = SetDisplayChar (hDC,CurrentType,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI);
							
							if (SDCrtn > 0)
							{
								GWPolyline (hDC,lpCurPoints,nPnts,CurrentDesc);
								HaveTXLoc = TRUE;
							}
							else if (SDCrtn < 0)
								HaveTXLoc = TRUE;
						}
					} 
					GSSiGlobUlFree (&hMem);
                }
                break;

				case 18: /* text character */
				{	HFONT	hFont, OldFont; 
					int		fheight, fwidth, angle, weight;
					BOOL	italic;
				
					TXLoc.x = *ipnt++;
					TXLoc.y = *ipnt++;
					TXFact  = *ipnt++; 
					TXRot   = *ipnt++; 
					TXFont  = HIBYTE(*ipnt); 
					TXColor = LOBYTE(*ipnt++); 
					nchar	= *ipnt++;
					pTXChar = (LPSTR)ipnt; 
					ipnt    += (nchar+1)/2; 
                }
                break;  
                
				case 19: /* grahpics text */
				{    
					LPGRTEXTHEADER	pGRTextHeader = (LPGRTEXTHEADER)ipnt;  
					LPGRTEXTHEADER	pPickedTextHeader;
					LPSTR pText;
					
   	    			CurTextHeaderLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
				    ipnt += sizeof(GRTEXTHEADER) / 2;  
				    nchar = pGRTextHeader->lText; 
				    if (hPickedTextHeader)
				    {
				    	pPickedTextHeader = (LPGRTEXTHEADER)GlobalLock (hPickedTextHeader);
				    	_fmemmove (pPickedTextHeader,pGRTextHeader,sizeof(GRTEXTHEADER)+nchar); 
				    	GlobalUnlock (hPickedTextHeader);
				    }
				    pText = (LPSTR)ipnt;   
				    ipnt += nchar/2; 
				    SetTextLocVars (&TLSet);
                	if (DescIsVisible && CurVis->WantType[2] && HaveTXLoc && (CurView->PassID || Pick)) 
                	{
						ProcessTextObject (hDC,pGRTextHeader,pText,nchar,BeginSeg,NULL,NULL,NULL,NULL);
					}
                }
                break; 
                
                case 191: //text pointer 
                {   
                	DPOINT	FromPoint;
                	
                	lpTextTPL = (LPTEXTTPL)ipnt;    
                	ipnt += 38/2;
				    SetTextLocVars (&TLSet);
                	if (Display && DescIsVisible && Visible && !Pick)
                	{
					    SaveDC (hDC);
	                	FromPoint = TXLoc;  
						{   HRGN    NewRgn, OvrLapRgn; 
						    RECT	TBRect=TextRect;
						    
						    //InflateRect (&TBRect,1*DeviceToScreenFactor(),1*DeviceToScreenFactor()); 
						    NewRgn = CreateVPRgn (FALSE,FALSE);
						    OvrLapRgn = CreateRectRgnIndirect (&TBRect);
				        	CombineRgn (NewRgn,NewRgn,OvrLapRgn,RGN_DIFF);
					  		SelectClipRgn (CurView->hDC,NewRgn); 
				            GSSiDeleteObject(&OvrLapRgn);  
				            GSSiDeleteObject(&NewRgn);
					  	}
					  	switch (lpTextTPL->style)
					  	{
					  		case 0:
								SimplePointer (hDC,&FromPoint,&lpTextTPL->points[0],1,2,-15,0);
							break; 
							
							case 1:
							{
								DPOINT	BPoint = TXLocBPBase, ZPoint={0,0}; 
								double	AZ, Dist;
								
								AZ = LTWOPI(getazd (&ZPoint,&lpTextTPL->points[0]) + PTRot);
								Dist = ldistp (ZPoint,lpTextTPL->points[0]);
								TXLocBPBase = dnewpt (BPoint,AZ,Dist);
								AZ = LTWOPI(getazd (&ZPoint,&lpTextTPL->points[1]) + PTRot);
								Dist = ldistp (ZPoint,lpTextTPL->points[1]);
								TXLocEPBase = dnewpt (BPoint,AZ,Dist);
								xdPoint = MidPointD(TXLocBPBase,TXLocEPBase);
				 		    	PTRot = getazd (&TXLocBPBase,&TXLocEPBase);
								TXLoc = xdPoint; 
								HaveTXLoc = 2;
					    		TXRot = PTRot;  
					    	}
							break;

							default:
							break;
						}
						
						RestoreDC (hDC,-1); 
					}
                }
                break; 
                
                case 192: //text pointer (UltiMap Style) 
                {   
                	LPUMTEXTTPL	lpTextTPL; 
                	DPOINT	TPLpoints[6];
					DPOINT	PLPoints[3];  
                	
                	lpTextTPL = (LPUMTEXTTPL)ipnt;    
                	ipnt += 50/2; 
				    SetTextLocVars (&TLSet);   
				    for (i=0;i<6;i++)
				    	TPLpoints[i] = FPointToDPoint (lpTextTPL->points[i]);
                	if ((Pick || CurView->PassID) && Visible && DescIsVisible && TextIsVisible && CurVis->WantType[2])  
                		ProcessUMTextPointer (hDC, Pick,lpTextTPL, TPLpoints, PLPoints,NULL,hExportTextPointer);
                }
                break; 
                
	        	case 20:	/* point symbol */   
	        	{
 		    		CurPointSize = *(LPFLOAT)ipnt;
   	    			CurSymSizeLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
	        		ipnt += 2;
	        		CurPointAZ = *(LPFLOAT)ipnt;
	        		//AdjustPointRotation (&CurPointAZ);
                    PTRot = CurPointAZ;
	        		ipnt += 2;
	        		lpPoints = (HPPOINTS)ipnt; 
	        		lpCurPoints = lpPoints;
	        		nPnts = 1;     
        			CurrentPoint = CurPointLocD = FilePtToBasePt (POINTStoPOINT(*lpPoints)); 
	        		if (DisplayDispersedPoint) 
	        		{
	        			CurrentPoint = CurPointLocD = DispersedPointLoc;
	        			CurPointLoc = BasePtToWinPt (&CurrentPoint);
	        		}
	        		else if (Highlight)
						CurPointLoc = BasePtToWinPt (&PickedPointLoc);
					else	
						CurPointLoc = FileCoordToWinCoord(POINTStoPOINT(*lpPoints)); 
					
//					ProjectFilePt (lpPoints); DisplayPointItem converts to windows coord
	        		if (Pick)  
	        			TXLoc = CurPointLocD; 
	        		ipnt += 2;  
	        		if (!PointInMaskAreaWinCoord (CurPointLoc))
	        			break;
	        		if (PointIsBlocked (&CurPointLocD,CurrentDesc))
	        			break;   
	        		HaveTXLoc = TRUE;
 		    		CurrentType = GF_POINT; 
					if (CurPointSize < 0)
						CurPointSize = -CurPointSize *DeviceToScreenFactor();
					else
						CurPointSize /= CurView->BaseUnitsPerPixel;
//					CurPointSize = min(max (CurPointSize,1),MaxPointSize);
										
					if (Pick & DescIsVisible && Visible)
					{
						AlreadyProcessed=TRUE;
						PickPointItem (POINTStoPOINT(*lpPoints),CurPointSize*ThemeWidthFactor,PTRot,CurrentDesc);			 		    
					} 
					else if (DescIsVisible && Visible && GetTypeVisibility(TYPE_POINT))
					{   
						short	iDesc=CurrentDesc;
						int		SDCrtn;
						
	   		 		    if (Visible && CurrentDesc > 0 && CurrentDesc < 3201) 
	   		 		    {
							if (TSize)
								CurView->CurVisType[CurrentDesc]=5;
							else
								CurView->CurVisType[CurrentDesc]=4;   
						}
						AlreadyProcessed = TRUE; 
						HighlightPointSym=FALSE;
						if (!GetTypeVisibility(6)) 
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
								size = min(max (size,1),MaxPointSize);
							}
							else if (ItemSymbolWidth > 0)
								size = ItemSymbolWidth * CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
							else if (ItemSymbolWidth < 0)
								size = -ItemSymbolWidth * BaseDistToWinDist * CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
							else
								size = CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
							if (iDesc < 0) 
							{   
								COLORREF	OldColor;
								
								if (ThemePointColor > -1)
									OldColor = 	SetTextColor (hDC,ConvertColor(ThemePointColor,ThemePointUseHalfTone));
								DisplayCharAtLoc (hDC,CurPointLoc,(short)IDNINT(size),-iDesc); 
								if (ThemePointColor > -1)
									SetTextColor (hDC,OldColor);
							}
							else
							{
								long	DisplayedWidth=0;

								DisplayPointItem (hDC,CurPointLoc,size,PTRot,iDesc,&DisplayedWidth);
								CurView->MaxSymbolWidth = max (CurView->MaxSymbolWidth,DisplayedWidth);
								CurView->MaxFileDisplayedPointWidth[FileNum] = max (CurView->MaxFileDisplayedPointWidth[FileNum],(DisplayedWidth/ FileDistToWinDist)-(((long)CurrentItemMinMax.xmx)-CurrentItemMinMax.xmn));

							}
						}
						else if (SDCrtn == 0)
			        		HaveTXLoc = FALSE;

					}
		        	TXLoc = CurPointLocD; 
	        		//HaveTXLoc = 1;
					//	TextOut (hDC,lpPoints->x,lpPoints->y,CurrentUDI,_fstrlen(CurrentUDI));
                }
	        	break;
	        	
	        	case 120:	/* point symbol (HiPrecis)*/   
	        	{
	        		long	dbref = CurrentRefno;
	        		
	        		HiPrecis = TRUE;  
 		    		CurPointSize = *(LPDOUBLE)ipnt;
   	    			CurSymSizeLocD = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
	        		ipnt += 4;
	        		CurPointAZ = *(LPDOUBLE)ipnt; 
	        		//AdjustPointRotation (&CurPointAZ);
                    PTRot = CurPointAZ;
	        		ipnt += 4; 
	        		if (HaveNewPoint)
	        			lpDCurPoints = (HPDPOINT)&NewPointD; 
	        		else if (DisplayDispersedPoint)
	        			lpDCurPoints = &DispersedPointLoc;
	        		else
	        			lpDCurPoints = (HPDPOINT)ipnt;  
			        CurrentPoint = CurPointLocD = *lpDCurPoints;
	        		LastElementBeginPoint = LastElementEndPoint = *lpDCurPoints;  
	        		LastElementAZ = CurPointAZ;
	        		nPnts = 1; 
//	        		if (Pick)
//	        			TXLoc = BasePtToFilePt (*lpDCurPoints); 
					CurPointLoc = BasePtToWinPt(lpDCurPoints); 
//					if (Highlight)
//						CurPointLoc = BasePtToWinPt (&PickedPointLoc);	
//					ProjectFilePt (lpPoints); DisplayPointItem converts to windows coord
	        		ipnt += 8;  
	        		if (!PointInMaskAreaWinCoord (CurPointLoc))
	        			break;
	        		if (PointIsBlocked (&CurPointLocD,CurrentDesc))
	        			break;   
	        		HaveTXLoc = TRUE;
 		    		CurrentType = GF_POINT; 
					if (CurPointSize < 0)
						CurPointSize = -CurPointSize * DeviceToScreenFactor();
					else
						CurPointSize /= CurView->BaseUnitsPerPixel;
//					CurPointSize = min(max (CurPointSize,1),MaxPointSize);
// need both DescIsVisible and Visible so pickdeletes works
					if ((Pick||PickingByRefno) && DescIsVisible && Visible && GetTypeVisibility(TYPE_POINT) && (PointIsVisible (CurrentDesc) || CurVis->WantType[8]))
					{
						AlreadyProcessed=TRUE;
						if (SetDisplayChar (hDC,GF_POINT,CurrentRefno,CurrentDesc,CurrentPrefix,CurrentUDI))
							PickPointItemD(lpDCurPoints, (CurPointSize*ThemeWidthFactor*GraphicsPointFactor)*CurView->BaseUnitsPerPixel, PTRot, CurrentDesc);
					} 
					else if (DescIsVisible && Visible && GetTypeVisibility(TYPE_POINT))
					{   
						short	iDesc=CurrentDesc;
						int		SDCrtn;
						
   		 		    	if (Visible && CurrentDesc > 0 && CurrentDesc < 3201)
   		 		    	{
							if (TSize)
								CurView->CurVisType[CurrentDesc]=5;
							else
								CurView->CurVisType[CurrentDesc]=4; 
						}
						AlreadyProcessed = TRUE; 
						HighlightPointSym=FALSE;
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
								size *= ThemeWidthFactor*GraphicsPointFactor;
								size = min(max (size,1),MaxPointSize);
							}
							else if (ItemSymbolWidth > 0)
								size = ItemSymbolWidth * CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
							else if (ItemSymbolWidth < 0)
								size = -ItemSymbolWidth * BaseDistToWinDist * CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
							else
								size = CurPointSize*ThemeWidthFactor*GraphicsPointFactor;
							if (iDesc < 0) 
							{   
								COLORREF	OldColor;
								
								if (ThemePointColor > -1)
									OldColor = 	SetTextColor (hDC,ConvertColor(ThemePointColor,ThemePointUseHalfTone));
								DisplayCharAtLoc (hDC,CurPointLoc,(short)IDNINT(size),-iDesc); 
								if (ThemePointColor > -1)
									SetTextColor (hDC,OldColor);
							}
							else 
							{
								long	DisplayedWidth=0;

								DisplayPointItem (hDC,CurPointLoc,size,PTRot,iDesc,&DisplayedWidth);
								CurView->MaxSymbolWidth = max (CurView->MaxSymbolWidth,DisplayedWidth);
								CurView->MaxFileDisplayedPointWidth[FileNum] = max (CurView->MaxFileDisplayedPointWidth[FileNum],(DisplayedWidth/ FileDistToWinDist)-(((long)CurrentItemMinMax.xmx)-CurrentItemMinMax.xmn));
							}
						} 
						else if (SDCrtn == 0)
			        		HaveTXLoc = FALSE;
					}
        			TXLoc = CurPointLocD; 
	        		//HaveTXLoc = 1;
					//	TextOut (hDC,lpPoints->x,lpPoints->y,CurrentUDI,_fstrlen(CurrentUDI));
                }
	        	break;
	        	
	        	case 121:
	        		HiPrecis = TRUE;
	        	case 21: /* set pen color, width and style*/
	        	{   
   	    			CurPenColorLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
	        		TempLineColor = *(LPLONG)ipnt;
	        		ipnt += 2; 
	        		TempLineWidth = *ipnt++;  
	        		//TempLineWidth = max (TempLineWidth,0);
	        		TempLineStyle = *ipnt++;  
	        		if (!Display || Pick || !hDC)
	        			break; 
	        		PenFromUMList = FALSE;
                    if (*PenCOLOR >= 0) 
                    	TempLineColor = *PenCOLOR;  
                    if (*PenWIDTH > 0)
                    	TempLineWidth = *PenWIDTH;
           			if (!GetTypeVisibility (9))
                    	TempLineWidth = 1;
                    if (HiPrecis)
					{
						if (TempLineWidth < 0)
							pwidth = -BaseDistToWinDist * TempLineWidth;
						else
                    		pwidth = TempLineWidth;
					}
                    else   
		 				pwidth = IDNINT(WidthFactor*TempLineWidth*DeviceToScreenFactor() * PenWidthFactor);  
					GSSiDeleteObject (&hTempPen);
					hTempPen = CreatePen(PS_SOLID,pwidth,ConvertColor(TempLineColor,CurrentDesc)); 
					SaveCurrentPen = 0;
	        	}
	        	break;
	        	
	        	case 22: /* set brush pattern color and forecolor*/
	        	{   
				    LOGBRUSH	NDB; 
				    
   	    			CurBrushLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
	        		TempPattern = *ipnt++;           
	        		TempFillColor = *(LPLONG)ipnt;  
	        		GlobalColors[0]=TempFillColor;  
	        		HaveVarFillColor = TRUE;
	        		ipnt += 2; 
	        		TempBackColor = *(LPLONG)ipnt;
	        		ipnt += 2;  
	        		if (!Display || Pick || !hDC)
	        			break;
					GSSiDeleteObject (&hTempBrush);
					if (SolidAreas && GetBit (7,(LPSTR)&CurVis->WantType[7]))
					{   
						if (!hTempPen)
							hTempPen = CreatePen (PS_SOLID,0,TempFillColor);
						hTempBrush = CreateSolidBrush(ConvertColor(TempFillColor,CurrentDesc));
					} 
		 			else
		 			{ 
						NDB.lbStyle = BS_HATCHED;
						NDB.lbColor = ConvertColor(TempFillColor,CurrentDesc);
						NDB.lbHatch	= HS_DIAGCROSS;
		 				hTempBrush = CreateBrushIndirect(&NDB);
                    }
					hOldBrush = SelectObject (hDC,hTempBrush);					
	        	}
	        	break;
	        	
	        	case 23: /* clear temp pen and brush */
	        	{              
	        		if (!Display || Pick || !hDC)
	        			break;
					if (hTempBrush)
					{
        SelectObject(hDC, GetStockObject(BLACK_BRUSH));
        				//SelectObject(hDC, hOldBrush);
						GSSiDeleteObject (&hTempBrush);
					}
					if (hTempPen)
					{
				        //SelectObject(hDC, hOldPen);
        SelectObject(hDC, GetStockObject(BLACK_PEN));
        				if (SaveCurrentPen && SaveCurrentPen != hTempPen) 
        					CurrentPen = SaveCurrentPen; 
        				else
        					CurrentPen = GetStockObject(BLACK_PEN);
						GSSiDeleteObject (&hTempPen);
					}
	        	}
	        	break; 
	        	
	        	case 24: //dummy delete  
	        		NumDeletesProcessed++;
					ItemIsDeleted = 2;   
	        		if (Visible && Pick && PickDeletes)
						PickPolyInAreaD(6, 0, 0, 0, 0, 0, 0, 0, 0, 0);
					if (ForceRefIndex || ForceTAGIndex)
						goto ProcessRefno;
	        	break;
	        	
	        	case 25: //text color  
	        		if (hDC)
	        			SetTextColor (hDC,ConvertColor(*(LPLONG)ipnt,CurrentDesc));   
	        		HaveTextColor = TRUE;
	        		ipnt += 2; 
	        	break;
	        	
	        	case 127:
	        		PointSize = 16;
	        	case 27: // Multipolygon indicator
	        	{
	        		short	n; 
	        		long	ln=(long)MAX_POLY_POINTS*PointSize;
	        		
					GSSiGlobFree (&hPolyBuffer); 
					GSSiGlobFree (&hPolyPartLen);  
	        		nPoly = n = *ipnt++; 
	        		iPoly = 0;
	        		PolyBufferLen = 2 * (nPoly - 1);    //is this needed - yes for addition of linkpoints
	        		while (n--)
	        			PolyBufferLen += *(LPWORD)ipnt++; 
	        		if (!hCurvePoints)
	        			ln = (long)PolyBufferLen*PointSize;
	        		hPolyBuffer = GSSiGlobAlloc(GAIDNO 308,GMEM_MOVEABLE,ln);  
	        		hPolyPartLen = GSSiGlobAlloc(GAIDNO 309,GMEM_MOVEABLE,(long)nPoly*sizeof(int));  
	        		nPolyPoints = 0;
                }
	        	break;
	        	
	        	case 227: // Multipolygon indicator
	        		PointSize = 16;
				case 228:
	        	{
	        		int		n; 
	        		long	ln=(long)MAX_POLY_POINTS*PointSize;
	        		
					GSSiGlobFree (&hPolyBuffer); 
					GSSiGlobFree (&hPolyPartLen);  
	        		nPoly = n = *(LPINT)ipnt++;
					ipnt++;
	        		iPoly = 0;
	        		PolyBufferLen = sizeof(int) * (nPoly - 1);    //is this needed - yes for addition of linkpoints
	        		while (n--)
					{
	        			PolyBufferLen += *(LPINT)ipnt++;
						ipnt++;
					}
	        		if (!hCurvePoints)
	        			ln = (long)PolyBufferLen*PointSize;
	        		hPolyBuffer = GSSiGlobAlloc(GAIDNO 308,GMEM_MOVEABLE,ln);  
	        		hPolyPartLen = GSSiGlobAlloc(GAIDNO 309,GMEM_MOVEABLE,(long)nPoly*sizeof(int));  
	        		nPolyPoints = 0;
                }
	        	break;
	        	
	        	case 28: // Curve Point ID's
	        	{
	        		short	n; 
	        		LPSHORT	pCurvePoints;
	        		
					GSSiGlobFree (&hCurvePoints); 
	        		nCurvePoints = n = *ipnt++; 
	        		hCurvePoints = GSSiGlobAlloc(GAIDNO 310,GMEM_MOVEABLE,(nCurvePoints+1)*sizeof(short));
	        		pCurvePoints = (LPSHORT)GlobalLock (hCurvePoints);
	        		while (n--)
	        			*pCurvePoints++ = *ipnt++;  
	        		*pCurvePoints = -1;
	        		GlobalUnlock (hCurvePoints);
	        	}
	        	break;
	        	
	        	case 30: /* intersection data: totlen/2, num_from_int, (ref,street, 1..num)
	        										   num_to_int,	 (ref,street, 1..num) */
	       		{   
	       			LPVOID	pIntData;
	       			if (hIntData)
	       			{
	       				pIntData = GlobalLock (hIntData);
	       				_fmemmove (pIntData,(LPVOID)(ipnt+1),*ipnt*2); 
	       				GlobalUnlock (hIntData);
	       			}
	        		ipnt += *ipnt+1;	
	        	}
	        	break;   
	        	
	        	case 31: /* from and to street */
	        	{  
	        		_fmemmove (&FromStreet,ipnt,4);
	        		_fmemmove (&ToStreet,ipnt+6,4);
	        		ipnt += 12;
	        	}
	        	break;  
	        	
	        	case 32: /* address data: totlen/2, ZIPL(5),ZIPR(5),COUNTYL(3),COUNTYR(3)
	        	         							addlengths(i:4,i:4,i:4,i:4) Addresses(fraddl,toaddl,fraddr,toaddr) */
	       		{   
	       			LPSTR	pAddData;
	       			if (hPNAddData)
	       			{
	       				pAddData = GlobalLock (hPNAddData);
	       				_fmemmove (pAddData,(LPVOID)(ipnt+1),*ipnt*2); 
	       				GlobalUnlock (hPNAddData);
	       			}
	        		ipnt += *ipnt+1;	
	        	}
	        	break;   
	        	
	        	case 33: /* jump to long rec and back */
	        	{
					ShowValue (hDC,FALSE);
					LastItemLenActual = (LPSTR) ipnt - (LPSTR)(BeginSeg +2) - LastCurrentItem; 
					if (LastItemLen && (LastItemLenActual != LastItemLen))
						LogItemLengthError (LastItemLen,LastItemLenActual);
					ContinuationOffset = *(LPLONG)ipnt; 
					if (ContinuationOffset >=0)
						Numcont++; 
					ipnt += 2;    
					JumpBackSeg = CurrentSeg;
					JumpBackElement = (LPSTR)ipnt - BeginSeg;
					JumpBackElement++;
					TotBlockLen += 2; 
				}
				break; 
				
	        	case 34: /* jump to long rec and back */
	        	{
					ContinuationOffset = JumpBackSeg; 
					StartElement = JumpBackElement;
					*ipnt = 0;	
				}
				break; 
				
				case 135:
					PointSize = 16;
			    case 35: /* point array */
			
				{   nPnts = *ipnt;
				    ipnt++; 
				    if (hCoords) 
				    {
				    	nCoords += nPnts;
				    	GlobalUnlock (hCoords);
				    	hCoords = GSSiGlobalReAlloc (0,hCoords,(long)nCoords*PointSize,GMEM_MOVEABLE);
				    }
				    else 
				    {
				    	hCoords = GSSiGlobAlloc(GAIDNO 311,GMEM_MOVEABLE,(long)nPnts*PointSize);
				    	nCoords = nPnts;
				    	lCoords = 0;
				    }
				    pCoords = GlobalLock (hCoords);
				    pCoords += lCoords;
				    BufWrite (&pCoords,&lCoords,(HPSTR)ipnt,nPnts*PointSize); 
				    GlobalUnlock (hCoords);
				    if (PointSize == 16)
					    lpDCurPoints = (HPDPOINT)GlobalLock (hCoords);
					else
					    lpCurPoints = (HPPOINTS)GlobalLock (hCoords); 
				    ipnt += nPnts * PointSize/2;
				}
				break;   
				
				case 36: /* null code */
				break; 
				
				case 37: 
				{
					
					GRStartTime = *(LPLONG)ipnt;
					ipnt+=2;
					GREndTime = *(LPLONG)ipnt;
					ipnt+=2; 
					if (GRStartTime > TimeRangeEnd || GREndTime < TimeRangeBeg)
						Visible = FALSE;
				}
				break; 
				
				case 38: //elevation (R*4)
				{   
					HPFLOAT	pElev, pElev2;
					
					GSSiGlobFree (&hElevBuffer);
				    nPnts = *ipnt++;
				    pElev = (HPFLOAT)ipnt;  
				    hElevBuffer = GSSiGlobAlloc(GAIDNO 312,GMEM_MOVEABLE,nPnts*sizeof(float));
				    pElev2 = (HPFLOAT)GlobalLock (hElevBuffer);
				    hmemmove ((HPSTR)pElev2,(HPSTR)pElev,nPnts * sizeof(float));
				    GlobalUnlock (hElevBuffer);
				    ipnt += nPnts * 2;
				}
				break;
	        	
				case 40: /* command string */
				{                    
					LPSTR	pString;
					
					LastRecTime = NextVarTime();
	    			CurGCmdStringLoc = CurrentSeg + (long)((LPSTR) ipnt - (LPSTR)BeginSeg); 
					lGCmdString = *ipnt++; 
					GSSiGlobFree (&hGCmdString);   
					hGCmdString = GSSiGlobAlloc(GAIDNO 313,GMEM_MOVEABLE,4096);
					pString = GlobalLock (hGCmdString);
					strncpy0 (pString,(LPSTR)ipnt,min(4095,lGCmdString));
					if (DescIsVisible && Visible)
					switch (ProcessGCmdStrings)
					{                          
						case 0:
							break;
						case 1:
							if (Pick)
								break; 
							goto DoCmd;
						case 2:
							if (!Pick)
								break;
						case 3: 
				DoCmd:  
						{
							LPVIEWPORT	SaveView=CurView;
							HANDLE		SaveH1=hTranFileToBase, SaveH2=hTranBaseToFile, SaveH3=hTranFileToVP;
							
							hTranFileToBase=0;
							hTranBaseToFile=0; 
							hTranFileToVP=0;
							InGRCmd = TRUE;
							ExpandText (pString); 
							InGRCmd = FALSE;
							strncpy0 (pString,(LPSTR)ipnt,min(4095,lGCmdString)); 
							SetCurView ( SaveView);
							hTranFileToBase=SaveH1;
							hTranBaseToFile=SaveH2;
							hTranFileToVP=SaveH3;
						} 
					}
					ipnt += lGCmdString/2; 
					GlobalUnlock (hGCmdString);
				}
				break;	
				
                default: 
                	ProcessInvalidRecord ((short)*Pcode,ipnt,1);
                break;
			  } 
			}
//			if (DoTime)
//				TypeTime[*Pcode] += (GetTickCount() - starttime2);
        }
//        if (((LPSTR)ipnt - BeginSeg) > LenSeg) causes problem with mpls old data set
//        	ProcessInvalidRecord (0,0,1);
Exit:    
    TotBlockLen += 1;
	if (Display && !Pick && hDC)
	{
        SelectObject(hDC, GetStockObject(BLACK_BRUSH));
        SelectObject(hDC, GetStockObject(BLACK_PEN));
		GSSiDeleteObject (&hBlackPen);
    }
    if (hSpecialPen)
    {
    	DeleteObject (hSpecialPen);
    	hSpecialPen = 0;
    }  
    if (hSpecialBrush)
    {
    	DeleteObject (hSpecialBrush);
    	hSpecialBrush = 0;
    }  
    if (DoTime)  
    	TotDisplayTime+=(GetTickCount()-starttime);
	InGraphicsProcessor = FALSE;   
	CurrentPoint = SaveCurrentPoint; 
   	if (ShowRefno)
   		SetWindowText (hWndMain,"Exit PGR");
{
#if ENABLETRACE
GSSiExitProg (686);
#endif
    return rtn;
}
#if ENABLETRACE
}
#endif
}
void ProcessInvalidRecord (short Pcode,LPSHORT ipnt,int from)
#if ENABLETRACE
{GSSiEnterProg (687);
#endif
{   
	HFILE		Fid;
	char		str[256];
	short		InvalidRecOpt=GetGlobalLVal2 ("[%INVALIDRECORDOPT]",0);
	
	switch (InvalidRecOpt)
	{
		case 0:
			CloseMap (FALSE); 
			setDoPaint( FALSE);
			_fstrcpy (str,PltName);
			ExpandText (str);
			if (FirstError)
			{
				if (GSSiMsgBox(hWndMain, "Invalid graphics record encountered",str,MB_OKCANCEL|MB_ICONEXCLAMATION,0) ==  IDCANCEL)
            		BlowOut(0,0);
            }
			FirstError = FALSE;  
			break;
		case 1:  
			Fid = GSSiOpenFile ("invalid.txt",0,OF_READWRITE);
			if (Fid == HFILE_ERROR)
				Fid = GSSiOpenFile ("invalid.txt",0,OF_CREATE);
			else
				GSSillseek (Fid,0,2); 
			sprintf (str,"Invalid graphics record: %s  (%ld,%ld,%i,%i)",PltName,CurrentSeg,CurElement,Pcode,from);
			ExpandText (str);
			fputstring (str,Fid);
			GSSiClose2 (&Fid);	
			break;
		case 2:
			break;
	}
	FoundInvalidRec = TRUE; 
	if (ipnt)
		*ipnt = 0;
{
#if ENABLETRACE
GSSiExitProg (687);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}         

void ScanItemForDBVals(void)
#if ENABLETRACE
{GSSiEnterProg (688);
#endif
{	 
	LPBYTE	Pcode;
	LPSHORT	ipnt;

	if (MapType != MT_PLT)
		goto Exit;     
    ipnt = CurElementPnt;
    while (*ipnt)
    {   Pcode = (LPBYTE) ipnt;
    	ipnt++;

	    switch (*Pcode)
        {   
        	case 12:    
        	case 92: 
        	case 93:
        	case 13:
				goto Exit;
			case 19: /* grahpics text */
			{	 
				LPGRTEXTHEADER	pGRTextHeader,pPickedTextHeader;
				LPSTR	pText; 
				HANDLE	hText;
				LPSTR	pText2;
				    
			    pGRTextHeader = (LPGRTEXTHEADER)ipnt;  
			    ipnt += sizeof(GRTEXTHEADER) / 2;  
			    nchar = pGRTextHeader->lText; 
			    pText = (LPSTR)ipnt;   
			    ipnt += nchar/2;
			    hText = GSSiGlobAlloc(GAIDNO 314,GMEM_MOVEABLE,512); 
			    pText2 = GlobalLock (hText);
			    strncpy0 (pText2,pText,min(511,nchar));  
				SetGlobalValue2 (hTEXT,pText2,0);
			    GSSiGlobUlFree (&hText); 
				goto Exit;
            }
            break; 
                

            default: 
            	SkipSubRec (Pcode,(HPSHORT*)&ipnt,0);
            break;

		}
    }
Exit:
{
#if ENABLETRACE
GSSiExitProg (688);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}
  

int	GetRecDesc (HPSTR rec, long l)
#if ENABLETRACE
{GSSiEnterProg (689);
#endif
{	HPSTR	Endpnt;
	LPBYTE	Pcode;
	HPSHORT	ipnt;
     
    ipnt = (LPSHORT)rec;
    Endpnt = rec + l;
    while (*ipnt != 0 && ipnt < (HPSHORT)Endpnt)
    {   Pcode = (LPBYTE) ipnt;
    	ipnt++;

	    switch (*Pcode)
        {   
            case 8:	/*	description */
	 		{   
{
#if ENABLETRACE
GSSiExitProg (689);
#endif
	 			return (*ipnt);
}
            }
            break;

            default: 
            	SkipSubRec (Pcode,&ipnt,0);
            break;

		}
    }

{
#if ENABLETRACE
GSSiExitProg (689);
#endif
    return 0;
}
#if ENABLETRACE
}
#endif
}

BOOL ProcessPrimarySeg (HWND hWndDlg, int DlgItemSym, int DlgItemPar, BOOL SetTran, short Opt,HFILE FidSymList)
#if ENABLETRACE
{GSSiEnterProg (690);
#endif
{	HANDLE 		hpltBuf=0;
	LPSTR		LPpltBuf;
	LPSHORT		ipnt;
	LPLONG		pOffset;
	int			idesc, ndesc,iparent;  
	HANDLE		hMem=GSSiGlobAlloc(GAIDNO 315,GMEM_MOVEABLE,512);
	LPSTR		DescName=GlobalLock (hMem);
	LPSTR		cdesc=DescName+256;
	LPSTR		str=cdesc+16;
	BOOL		parent, SymIsVisible, Invisible;
	int			pwidth; 
	long		ii;
	double		ifac,penwidth=0;
	BOOL		DupSet;
    LOGBRUSH	NDB;
    LRESULT     lResult; 
    BOOL		ValidDesc;
	RGBQUAD 	rgb;
	WORD		nRead, nBytes=0; 
	short			Pcode; 
	int			ipen, i;  
	COLORREF	Color;  
	long		StartLoc;
	LPSTR		pStart, pLoc;   
	BOOL		rtn=TRUE;
	
	ifac = 0;
	if (CurVis)
		if (CurVis->WantType[9])
			ifac = WidthFactor;
		
	if (DlgItemPar)
		DupSet = TRUE;
	else
		DupSet = FALSE;


    if ((!Opt && !PltName[0]) || FidMap == HFILE_ERROR)
	{
	   	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (690);
#endif
    	return (FALSE);
}
	}

    nRead = BigRead (FidMap,(HPSTR)&nBytes,2);
    if (!nRead || nBytes <= 0)
    	goto Exit;
    hpltBuf = GSSiGlobAlloc(GAIDNO 316,GHND,(DWORD)nBytes+4);
    LPpltBuf = GlobalLock (hpltBuf);
    StartLoc = GSSillseek (FidMap,0,1);
    nRead = BigRead (FidMap,LPpltBuf,(WORD)nBytes);   
    if (nRead != nBytes)
    	ii=1;
    ipnt = (LPSHORT)LPpltBuf;   
    pStart = (LPSTR)ipnt;
        while (*ipnt > 0)
        {   Pcode = *ipnt;
        	ipnt++;
		    switch (Pcode)
	        {
			    case 1: /* color palette */
		 		{   ipen = *ipnt;
		 			if (ipen > maxbrush) maxbrush=ipen; 
		 			ipnt++;
		 			if (ipen == ModifyPen)
		 				ModPenLoc = (LPSTR) ipnt - LPpltBuf + 2;
		 			rgb.rgbRed = *ipnt;
		 			ipnt++;
		 			rgb.rgbGreen = *ipnt;
		 			ipnt++;
		 			rgb.rgbBlue = *ipnt;
		 			ipnt++;
					break;//color palette no longer supported 11/13/2009
		 			Color = RGB(rgb.rgbRed,rgb.rgbGreen,rgb.rgbBlue);
		 			GetPenRedefColor(ipen,&Color);
                    if (*PenCOLOR >= 0) 
                    	Color = *PenCOLOR;  
                    if (*PenWIDTH > 0)
                    	penwidth = *PenWIDTH; 
                    else
                    	penwidth = 1;  
		 			pwidth = IDNINT(ifac*penwidth*DeviceToScreenFactor() * PenWidthFactor);  
//		 			if (penwidth > 1)
//		 				pwidth = max ((long)pwidth,IDNINT(penwidth)); removed 12/29/99
					pens[ipen] =   CreatePen(PS_SOLID,pwidth,ConvertColor(Color,-1));
                    if (*AreaCOLOR >= 0) 
                    	Color = *AreaCOLOR;  
					if (!CurVis || (SolidAreas && GetBit (7,(LPSTR)&CurVis->WantType[7])))
		 				brushes[ipen] = CreateSolidBrush(ConvertColor(Color,-1));
		 			else
		 			{ 
						NDB.lbStyle = BS_HATCHED;
						NDB.lbColor = ConvertColor(Color,-1);
						NDB.lbHatch	= HS_DIAGCROSS;
		 				brushes[ipen] =CreateBrushIndirect(&NDB);
                    }
		 		}
	            break;

                case 101:	/*	used description offset	*/
                {   pOffset = (LPLONG)ipnt;
                	UsedDescOffset = *pOffset; 
                	ipnt += 2;
                }
                break;

			    case 102: /* color palette offset */
                {   pOffset = (LPLONG)ipnt;
                	ColorPaletteOffset = *pOffset; 
                	ipnt += 2;
                }
                break;

                case 103: /* transformation points */
                {   pOffset = (LPLONG)ipnt;
                	TranPointOffset = *pOffset;
                	ipnt += 2;
                }
                break;

                case 200: /* quad tree offset */
                {   pOffset = (LPLONG)ipnt;
                	GraphicsOffset = *pOffset;
                	ipnt += 2;
                }
                break;

                case 201: /* desc block offset */
                {   pOffset = (LPLONG)ipnt;
                	DescBlockOffset = *pOffset;
                	ipnt += 2;
                }
                break;

                case 202: /* date range */
                {   
                	FileDateOffset = (LPSTR)ipnt - LPpltBuf + StartLoc - 2;
               		MinFileTime = *(LPLONG)ipnt;
                	ipnt += 2; 
                	MaxFileTime = *(LPLONG)ipnt;
                	ipnt += 2;
                }
                break;

                case 11: /* get used description list*/
                {   
                    short SymNameLen=8;
                    
                    if (Opt == 1)
                    	rtn = FALSE;
                	if (MapVersion >= 8)
                		SymNameLen = 32;
                	parent = FALSE;    
                	moredesc:   ndesc = *ipnt;
                	ipnt++;
                	for (i=0;i<ndesc;i++)
                	{	idesc=abs (*ipnt); 
                		pLoc = (LPSTR)ipnt;
                		if (Opt == 1)
                		{
                			if (!parent && GetVisibility(idesc))
                				rtn = TRUE;
                		}
                		else if (*ipnt < 0 && !parent)
                		{
                			if (!GetInVisibility(idesc))
                				ToggleInVisibility(idesc);
                		}
                		ipnt++;
                		iparent=*ipnt;
                		ipnt++; 
                    	_fmemmove (DescName,ipnt,SymNameLen); 
                    	DescName[SymNameLen]=0;
                    	Truncate (DescName);
                    	if (!_fstricmp (DescName,NewDescName))
                    	{
                    		SymbolNameLoc = StartLoc + (pLoc + 4 - pStart);
                    		SymbolNameLen = SymNameLen;
                    	}
                    	if (!_fstricmp(DescName,"ALL"))
                    		iparent=0;
                    	ipnt+=(SymNameLen/2);  
                    	ConvertSymName (DescName,1,parent,idesc);  
                    	OpenFileSymListAdd (idesc,DescName);
						if (idesc == 580 || idesc == 581)
							ii=1;
                    	if (!DuplicateDesc(idesc,DupSet))
                    	{
	                		if (iparent == SetVisByPar && SetVisByPar > 0)
	                		{	SymIsVisible = GetVisibility(idesc);
	                			if (SymIsVisible != ParIsVisible) ToggleVisibility(idesc);
	                			if (parent) AddParToList(idesc);
	                		}
	                    	if (DlgItemSym<0)
	                    	{       
	                    		if (DlgItemSym == -9999)
	                    		{  
	                    			if (!_fstricmp (CurrentSymName,DescName)) 
	                    				CurrentSymNum = idesc;
	                    		}
	                    		else
	                    		{
	                    			if (idesc==-DlgItemSym) 
	                    			{
	                    				_fstrcpy (CurrentSymName,DescName);
	                    				CurrentSymParent = iparent;   
	                    				CurSymIsParent = parent;
	                    			}
	                    		}
	                    	} 
	                    	if (_fstricmp(DescName,"        ")>0)
	                    		ValidDesc=TRUE;
	                    	else
	                    		ValidDesc=FALSE;
	                    	if (!Opt)
	                    	{ 
		                    	if (DlgItemPar >= 0)
		                    	{
			                    	if(GetVisibility(idesc))
			                    		_fstrcat (DescName,"\t<on>\t");
			                    	else
			                    		_fstrcat (DescName,"\t\t");
			                    	sprintf (_fstrchr(DescName,0),"%i\t%i",iparent,idesc);      
			                		if ((DlgItemSym > 0 || FidSymList != HFILE_ERROR) && ValidDesc)
			                		{
			                			if (parent)
			                			{
			                				if (DlgItemPar > 0)
			                	    			lResult=SendDlgItemMessage ((HWND) hWndDlg, DlgItemPar,
			                				     	LB_ADDSTRING, 0, (LPARAM) DescName); 
			                			}
			                			else if (FidSymList != HFILE_ERROR)
										{
											sprintf (strchr (DescName,0),"\t%i",CurView->CurFile);
			                				fputstring (DescName,FidSymList);
										}
			                			else if (hWndDlg && DlgItemSym)
			                				lResult=SendDlgItemMessage ((HWND)hWndDlg,
			                	          		DlgItemSym,LB_ADDSTRING,0,(LPARAM) DescName); 
			                		}
			                	}
			                	else if (parent) 
			                	{   
			                		if(GetVisibility(idesc))
			                		{
				                		if (_fstricmp (DescName,"ALL"))   
					                   		sprintf (str,"(%s)",DescName);
		                	    		lResult=SendDlgItemMessage ((HWND) hWndDlg, -DlgItemPar,
					                				     	CB_ADDSTRING, 0, (LPARAM) str);
			                	    }
			                	}
			                }
	                	}
                	}
                	if (DlgItemPar < -3200)
                		goto Exit;
                	parent = TRUE;
                	if (ndesc) goto moredesc;
                }
                break;

				case 13: /* continuation offset */
				{	ContinuationOffset = *(LPLONG)ipnt;
					ipnt += 2;
					*ipnt = 0; 
					if (ContinuationOffset >=0)
						ii=1;
                }
                break;

                default:
                	ProcessInvalidRecord (Pcode,ipnt,3);
                break;

			}
        }

Exit:  	GSSiGlobUlFree (&hpltBuf);
    	if (SetTran)
    		ReadTranRecord();   
    	GSSiGlobUlFree (&hMem);
{
#if ENABLETRACE
GSSiExitProg (690);
#endif
    	return (rtn);
}

#if ENABLETRACE
}
#endif
}     

void SetMapTypeKey(int MapType,int fromItem)
{
	switch (MapType)
	{
	case MT_SQLITE:
		CurrentSQLITERec = PickList[fromItem].Segment;
		break;
	case MT_SHP:
		CurrentSHPRec = PickList[fromItem].Segment;
		break;
	}
}
BOOL ProcessPickedItem (int Item,short DoDisplayIn /*0=nodisplay-no themes,1=display with themes,2=display no themes,-1=withpick,-2=no display process themes,-3=no display-no close*/)
#if ENABLETRACE
{GSSiEnterProg (692);
#endif
{	LPSHORT		ipnt, EndItem;
    HANDLE 		hpltBuf=0;
	LPSTR		LPpltBuf;
	ITEM		*ItemHeader;
	HDC			hDC=0;
	POINT		CenterPoint, WinPoint;
	BOOL		DoDisplay=FALSE, SaveDisplay, SaveIgnoreBounds, SaveIPL=IgnorePrevLayers, SavePick=Pick;   
	HANDLE		hVisList=0;
	LPVISLIST	SaveVis;    
	LPVIEWPORT	SaveVP=CurView, DisplayVP;
	int			ii; 
	WORD		nBytes, nRead;
	short		SaveNumThemes; 
	BOOL		Close=TRUE;   
	HANDLE		hIndex=0; 
	char		str[64];
	long		SaveMinPickItemWidth = MinPickItemWidth;
	BOOL		OpenBP=FALSE;
	int			PickMapType;
	BOOL		CallSetPickGlobals = TRUE;

	MinPickItemWidth = 0;
	ProcessSingleItem=TRUE;
	if (DoDisplayIn == -4)
	{
		DoDisplayIn = -3;
		CallSetPickGlobals = FALSE;
	}
	if (DoDisplayIn == -3)
	{
		DoDisplayIn = 0;
		Close = FALSE;
	}
	if (DoDisplayIn > 0)
		DoDisplay = TRUE;
	Pick = FALSE;
	IgnorePrevLayers = TRUE; 
	SetConfig (PickList[Item].ConfigID);
    SetViewport (PickList[Item].ViewID); 
	SetGlobalValueLong("%PICKED_REFNO", PickList[Item].Refno);
	SetGlobalValue("%PICKED_PREFIX", PickList[Item].Prefix);
	SetGlobalValue("%PICKED_UDI", PickList[Item].UDI);

    DisplayVP = CurView; 
    SaveNumThemes = CurView->NumThemes;
    if (!DoDisplayIn || DoDisplayIn == 2)
    	CurView->NumThemes = 0;
    SaveDisplay = Display; 
    Display = DoDisplay;
    if ((DoDisplayIn >= 0 && PickList[Item].FileNum<0) || PickList[Item].Desc < 0 || !GetPickName (Item))
    	goto RtnFalse;

	if (!PickName[0])
	   	goto RtnFalse;  
	PickMapType = MapType;
	if (CallSetPickGlobals)	
		SetPickGlobals (Item);
	LoadIndexParm (PickDirectory);
    if (PickList[Item].IsDispersed)
    {
	    DisplayDispersedPoint = TRUE;
	    DispersedPointLoc = PickList[Item].BeginPoint;
    }
	CurrentProcessedPickedItem = Item;  
	ExpandText(PickName);
	if (/*FidMap != HFILE_ERROR ||*/ _fstricmp (PltName,PickName))
	{
		CloseMap (FALSE);
		_fstrcpy (PltName,PickName);
	}
	{
		BOOL	SaveWDB = WantDescBlock; 
		BOOL	omst;
		WantDescBlock = FALSE; 
		SetPGDB_SQL ("ObjectID = [%OBJECTID]");
		SetMapTypeKey(PickMapType,Item);
		omst = OpenMap (CurView->hWnd,CurView->hDC);
		SetPGDB_SQL ("");
	   	WantDescBlock = SaveWDB;  
	   	if (!omst)
	    	goto RtnFalse; 
	}
    if (FidMap == HFILE_ERROR)
    	goto RtnFalse;   
    {   
    	HANDLE	hFile=GSSiGlobAlloc(GAIDNO 1843,GMEM_MOVEABLE,1024);
    	LPSTR	File = GlobalLock (hFile); 
    	LPSTR	drive=File+512;
    	LPSTR	dir=drive+32;
    	
        UseDGNColors = TRUE;
		ProcessGlobal ("[%LAYERINIT]");
	    SetGlobalValue ("%LAYER_PROJECTION","");
        _fstrcpy (File,PickName);
        ExpandText (File);
        _splitpath (File,drive,dir,0,0);
        sprintf (File,"%s%sglobal.ini",drive,dir); 
        SetGlobalValue ("%WANTPASS","");
        LoadGlobalInit (File,FALSE);
        GSSiGlobUlFree (&hFile); 
    }
	hVisList=GSSiGlobAlloc(GAIDNO 317,GHND,sizeof(VISLIST));
	SaveVis = CurVis;
	CurVis = (LPVISLIST)GlobalLock (hVisList); 
	CurVis->hVisList=hVisList;
	InitVis ();
    if (FidMap == HFILE_ERROR)
    	goto RtnFalse;  
	if (SaveVis)   //if this is removed vanzandt parcels dont display when digitized until refresh cause border not displayed
	{
		if (SaveVis->WantType[7])
			CurVis->WantType[7] = SaveVis->WantType[7];
		if (SaveVis->WantType[8])
			CurVis->WantType[8] = SaveVis->WantType[8];
	} 
	CurVis->WantType[8] = 1;
	if (MapType == MT_SHP)
	{
	    CurView->PassID=4;
    	CurrentSHPRec = PickList[Item].Segment;
    	SHPRecOffset = GetSHPRecordOffset (CurrentSHPRec,FALSE);
		ReadSHPRecordHeader (FidMap,SHPRecOffset,&PickList[Item].Rect);
	}
	else if (MapType == MT_PERSONAL_GEO_DB)
	{
		SetPGDB_SQL ("ObjectID = [%OBJECTID]");
		if (!OpenPGDBFileIndex (PltName,0))
			goto RtnFalse;
	    CurView->PassID=4;  
    	CurrentSHPRec = PickList[Item].Segment;
	    ltoa (PickList[Item].Segment,str,10);
    	SetGlobalValue("%OBJECTID",str);
		if (!FetchDBRec (PGDBHandle))
    	{ 
    		CloseMap(FALSE);
	    	goto RtnFalse;
    	}
		ReadPGDBRecordHeader (&PickList[Item].Rect);
	}
	else if (MapType == MT_FILE_GEO_DB)
	{
		SetFGDB_SQL ("OBJECTID = [%OBJECTID]");
		if (!OpenFGDBFileIndex (PltName,0))
			goto RtnFalse;
	    CurView->PassID=4;  
    	CurrentSHPRec = PickList[Item].Segment;
	    ltoa (PickList[Item].Segment,str,10);
    	SetGlobalValue("%OBJECTID",str);
		if (!FetchDBRec (FGDBHandle))
    	{ 
    		CloseMap(FALSE);
	    	goto RtnFalse;
    	}
		ReadFGDBRecordHeader (&PickList[Item].Rect);
	}
	else if (MapType == MT_ORA)
	{
	    CurView->PassID=4;
    	CurrentORARec = PickList[Item].Segment;
    	ORARecOffset = GetORARecordOffset (CurrentORARec,FALSE); 
    	if (ORARecOffset < 0)
    	{ 
    		CloseMap(FALSE);
	    	goto RtnFalse;
    	}
		ReadORARecordHeader (FidMap,ORARecOffset,&PickList[Item].Rect);
	}
	else if (MapType == MT_DGN7)
	{
		CurView->PassID = 4;
		CurrentDGNRec = PickList[Item].Segment;
		GetDGNRecordBounds(CurrentDGNRec, &PickList[Item].Rect);
	}
	else if (MapType == MT_DGN8)
	{
		CurView->PassID = 4;
		CurrentDGN8Rec = PickList[Item].Segment;
		GetDGN8RecordBounds(CurrentDGNRec, &PickList[Item].Rect);
	}
	else if (MapType == MT_GPX)
	{
		CurView->PassID=4;
    	CurrentGPXRec = PickList[Item].Segment;
    	GetGPXRecordBounds (CurrentDGNRec,&PickList[Item].Rect);
	}
    else if (MapType == MT_MACRO)
    {   
    	CurrentMacroRec = PickList[Item].Segment;
    	if (!ProcessDisplayMacro (FidMap,4))
    		goto RtnFalse;
    }
    else if (MapType == MT_GMD)
    {   
    	CurrentGMDRec = PickList[Item].Segment;
    	GMDRecOffset = GetGMDRecordOffset (CurrentGMDRec,FALSE); 
    	if (GMDRecOffset < 0)
    	{ 
    		CloseMap(FALSE);
	    	goto RtnFalse;
    	}
    }
	else if (MapType == MT_SQLITE)
	{
		CurView->PassID = 4;
		CurrentSQLITERec = PickList[Item].Segment;
		GetSQLITERecordBounds(CurrentSQLITERec, &PickList[Item].Rect);
	}
	else if (MapType == MT_HGF)
    {
		int	nBytes4;

		CurrentSeg = HLTGraphicsPos; 
	    GSSillseek (FidMap,HLTGraphicsPos,0);
	    nRead = BigRead (FidMap,(HPSTR)&nBytes4,4); 
	    if (!nRead || !nBytes4)
	    {
	    	InvalidItem (0,TRUE);
	    	goto RtnFalse;
	    }
	    hpltBuf = GSSiGlobAlloc(GAIDNO 318,GMEM_MOVEABLE,(DWORD)nBytes4+16);
	    LPpltBuf = GlobalLock (hpltBuf);
	    nRead = BigRead (FidMap,LPpltBuf,nBytes4);
	    if (nRead != nBytes4) 
	    {
	    	InvalidItem (0,TRUE);
	    	goto RtnFalse;
	    }
	    ipnt = (LPSHORT)(LPpltBuf);
	    ItemHeader = (LPITEM) ipnt;
	    if (InvalidItem (ItemHeader,TRUE))
	    	goto RtnFalse; 
		ItemHeader->Len = 0;
	    if (ItemHeader->Len)
	    {
		    EndItem = ipnt + abs(ItemHeader->Len);
		    EndItem+=6;
		    *EndItem = 0; 
		}
		else
			_fmemset ((LPSTR)(LPpltBuf + nBytes4),0,16); 
	}
	else
	{
		CurrentSeg = PickList[Item].Segment; 
	    GSSillseek (FidMap,CurrentSeg,0);
	    nRead = BigRead (FidMap,(HPSTR)&nBytes,2); 
	    if (!nRead || !nBytes)
	    {
	    	InvalidItem (0,TRUE);
	    	goto RtnFalse;
	    }
	    hpltBuf = GSSiGlobAlloc(GAIDNO 318,GMEM_MOVEABLE,(DWORD)nBytes+16);
	    LPpltBuf = GlobalLock (hpltBuf);
	    nRead = BigRead (FidMap,LPpltBuf,nBytes);
	    if (nRead != nBytes || PickList[Item].Offset > nRead) 
	    {
	    	InvalidItem (0,TRUE);
	    	goto RtnFalse;
	    }
	    ipnt = (LPSHORT)(LPpltBuf + PickList[Item].Offset);
	    ItemHeader = (LPITEM) ipnt;
	    if (InvalidItem (ItemHeader,TRUE))
	    	goto RtnFalse; 
	    if (ItemHeader->Len)
	    {
		    EndItem = ipnt + abs(ItemHeader->Len);
		    EndItem+=6;
		    *EndItem = 0; 
		}
		else
			_fmemset ((LPSTR)(LPpltBuf + nBytes),0,16); 
	}
    CurView->PassID=4;  
    if (Display)
    {
		hDC = ScreenBufferDC (CurView->hWnd,CurView->hDC);
		ii = SaveDC (hDC);
	    GSSiDeleteObject(&CurView->hRgn);
		CurView->hRgn = CreateVPRgn(FALSE,FALSE);
	  	SelectClipRgn (CurView->hDC,CurView->hRgn);
	  	GSSiDeleteObject(&CurView->hRgn);
	    SetDisplayMode (CurView->hDC, GF_MAPMODE); 
	    //hDC = CurView->hDC;
	    //ii=SaveDC (hDC);
	    OpenBP = OpenBasePens();
	    ProcessPickItemDesc = PickList[Item].Desc;
	    ProcessPickItemType = PickList[Item].Type;
	    _fstrcpy (ProcessPickItemPrefix,PickList[Item].Prefix);   
        ThemeBeginDisplayPass(FALSE,CurView->ID);    
        ProcessPickItemDesc = 0;
	} 
	SaveIgnoreBounds = IgnoreBounds;
	IgnoreBounds = TRUE;
	ProcessSingleItem=TRUE;
	HaveFirstHeader=FALSE;   
	if (DoDisplayIn == -1)
		Pick = TRUE;
	switch (MapType)
	{
		case MT_SHP:
			ProcessSHPRecord (hDC,FidMap,CurrentSHPRec); 
		break; 
		case MT_PERSONAL_GEO_DB:
			ProcessPGDBRecord (hDC,CurrentSHPRec,0); 
		break; 
		case MT_FILE_GEO_DB:
			ProcessFGDBRecord (hDC,CurrentSHPRec); 
		break; 
		case MT_ORA:
			ProcessORARecord (hDC,FidMap,CurrentORARec); 
		break;
		case MT_GMD:
			ProcessGMDRecord(hDC, (HANDLE)FidMap, CurrentGMDRec);
			break;
		case MT_SQLITE:
			if (GetSQLITERecord(CurrentSQLITERec))
				ProcessSQLITERecord(hDC,CurrentSQLITERec);
			break;
		case MT_DGN7:
			ProcessDGNRecord(hDC, CurrentDGNRec);
			break;
		case MT_DGN8:
			ProcessDGN8Record(hDC, CurrentDGNRec);
			break;
		case MT_GPX:
			ProcessGPXRecord (hDC,CurrentGPXRec);
			break;
		case MT_MACRO:
	    	CurrentMacroRec = PickList[Item].Segment;
	    	ProcessDisplayMacro (FidMap,4);
		break;
		default:
			while (ProcessGraphicsRec (hDC,ipnt,LPpltBuf,nRead))
		    {
				GSSiGlobUlFree (&hpltBuf);
			    GSSillseek (FidMap,ContinuationOffset,0);
			    nRead = BigRead (FidMap,(HPSTR)&nBytes,2);
			    hpltBuf = GSSiGlobAlloc(GAIDNO 319,GMEM_MOVEABLE,(DWORD)nBytes+2);
			    LPpltBuf = GlobalLock (hpltBuf);
			    nRead = BigRead (FidMap,(HPSTR)LPpltBuf,nBytes);
				ipnt = (LPSHORT)LPpltBuf;
		    } 
		break;
	}
	ProcessSingleItem=HaveFirstHeader=FALSE;
    IgnoreBounds = SaveIgnoreBounds; 
//    if (DoDisplayIn)
	   	CloseBasePens(OpenBP);
	GetItemMinMax (&CurrentItemMinMax,&PickList[Item].Rect); 
    if (Close)
		CloseMap(FALSE);
	GSSiGlobUlFree (&hpltBuf);
	if (CallSetPickGlobals)
		SetPickGlobals (Item);
	if (Display) 
	{   
		ThemeEndDisplayPass(FALSE,FALSE,TRUE);
        DisplayVPThemeLegends ();
		SaveFullWindowBitmap (CurView->hWnd);
		RestoreDC (hDC,ii); 
		ShowBufferedScreen (TRUE,TRUE,0,0);
	}
	Display = SaveDisplay;  
	GSSiGlobUlFree (&hVisList);
	CurVis = SaveVis; 
	DisplayVP->NumThemes = SaveNumThemes;
	SetCurView ( SaveVP);  
	IgnorePrevLayers = SaveIPL; 
	Pick = SavePick; 
	if (hIndex)
	   	CloseMapIndex (0,hIndex,FALSE,TRUE);  
	LoadIndexParm (0);   
	DisplayDispersedPoint = FALSE; 
	MinPickItemWidth = SaveMinPickItemWidth;
{
#if ENABLETRACE
GSSiExitProg (692);
#endif
	return(TRUE); 
}
	 
RtnFalse:  
	SetUDIValue(PickList[Item].Prefix, PickList[Item].UDI);

	GSSiGlobUlFree (&hVisList);
	ProcessSingleItem = FALSE;
	DisplayDispersedPoint = FALSE;    
	DisplayVP->NumThemes = SaveNumThemes;
	SetCurView ( SaveVP);
	Display = SaveDisplay;  
	IgnorePrevLayers = SaveIPL;
	Pick = SavePick;    
	MinPickItemWidth = SaveMinPickItemWidth;
	LoadIndexParm (0);
{
#if ENABLETRACE
GSSiExitProg (692);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}  

BOOL InvalidItem (LPITEM ItemHeader,short AbortOpt)
#if ENABLETRACE
{GSSiEnterProg (693);
#endif
{   
	LPBYTE	Pcode;
	BYTE	NullCode=0;
	
	if (ItemHeader)
		Pcode = (LPBYTE) ItemHeader; 
	else
		Pcode = &NullCode;
	if (*Pcode == 92 && !ShowDeletedOpt)
{
#if ENABLETRACE
GSSiExitProg (693);
#endif
		return FALSE;
}
	if (*Pcode !=12 && !(*Pcode == 92 && ShowDeletedOpt))
	{   
		if (AbortOpt)
		{
			char	mess[512];

			HaltMapDisplay(FALSE,FALSE);
			sprintf (mess,"Invalid header (%i)\r\nReference and TAG indexes may need to be recreated",*Pcode);
			GSSiMsgBox (0,mess,PltName,MB_ICONEXCLAMATION,0);
	        BlowOut(0,0);   
        }
{
#if ENABLETRACE
GSSiExitProg (693);
#endif
		return TRUE;
}
	}
{
#if ENABLETRACE
GSSiExitProg (693);
#endif
	return FALSE;
}
#if ENABLETRACE
}
#endif
}


double PointInPickArea (POINT	Point)
#if ENABLETRACE
{GSSiEnterProg (700);
#endif
{
	double	iPickAp,d=DBL_MAX; 
	DPOINT	DPoint;
    
	if (FileDistToBaseDist == 0)
{
#if ENABLETRACE
GSSiExitProg (700);
#endif
		return d;
}

    SetPickAp(0);    
	d = idist (Point,PickPoint);
    if (PickAp)
    {
		iPickAp = PickAp * WindowToFileFactor;  
		if (d <= iPickAp)
{
#if ENABLETRACE
GSSiExitProg (700);
#endif
			return (d);
}
		else
{
#if ENABLETRACE
GSSiExitProg (700);
#endif
			return (DBL_MAX); 
}
	}
{
#if ENABLETRACE
GSSiExitProg (700);
#endif
    return (d);
}
#if ENABLETRACE
}
#endif
}

double PointInPickAreaD (LPDPOINT pPoint)
#if ENABLETRACE
{GSSiEnterProg (701);
#endif
{
	double	iPickAp,d=DBL_MAX; 
	DPOINT	DPoint;
    
	if (FileDistToBaseDist == 0)
{
#if ENABLETRACE
GSSiExitProg (701);
#endif
		return d;
}

    SetPickAp(0);    
	d = ldistp (*pPoint,PickPointBase);
    if (PickAp)
    {
		if (d <= PickApW)
{
#if ENABLETRACE
GSSiExitProg (701);
#endif
			return (d);
}
		else
{
#if ENABLETRACE
GSSiExitProg (701);
#endif
			return (DBL_MAX); 
}
	}
{
#if ENABLETRACE
GSSiExitProg (701);
#endif
    return (d);
}
#if ENABLETRACE
}
#endif
}

BOOL PickPointItem (POINT Point, float size, float rot, int Symbol)
#if ENABLETRACE
{GSSiEnterProg (702);
#endif
{ 
	DPOINT	DPoint = FilePtToBasePt (Point);
	BOOL	rtn = PickPointItemD (&DPoint,size,rot,Symbol);
{
#if ENABLETRACE
GSSiExitProg (702);
#endif
	return rtn;
}
	
#if ENABLETRACE
}
#endif
} 

BOOL PickPointItemD (LPDPOINT pPoint, double size, double rot,int Symbol)
#if ENABLETRACE
{GSSiEnterProg (703);
#endif
{   
	DPOINT	Points[4];
	BOOL	SavePP, rtn;  
	MNMXCORD	SaveBounds;
	double	halfsize=size/2, symsize;  
	short	RectMax; 
	RECT	SymRect;
	
	if (!PickPoints)
{
#if ENABLETRACE
GSSiExitProg (703);
#endif
		return FALSE;
}
	CurPointW = *pPoint;
	CurPointF = BasePtToFilePt (CurPointW); 
/*	Points[0].x=pPoint->x - halfsize;
	Points[0].y=pPoint->y - halfsize;
	Points[1].x=pPoint->x - halfsize;
	Points[1].y=pPoint->y + halfsize;
	Points[2].x=pPoint->x + halfsize;
	Points[2].y=pPoint->y + halfsize;
	Points[3].x=pPoint->x + halfsize;
	Points[3].y=pPoint->y - halfsize; 
	SavePP = PickPerim;
	PickPerim = FALSE;
	rtn = PickPolygonD (Points,4,9999999);  
	PickPerim = SavePP;  */ 
	SaveBounds = CurView->WBounds; 
	if (PickPointSymbol)
	{
		SymRect=GetSymRect(Symbol);
		AddToPickAp = halfsize; 
		RectMax = max ((long)SymRect.right - (long)SymRect.left,(long)SymRect.bottom - (long)SymRect.top);   
		symsize = halfsize*((double)RectMax)/200;
		InflateBounds (&CurView->WBounds,symsize);
	}
	else
		symsize = 0;
	rtn = PickPolylineD (pPoint,1,0,1,&rot,&symsize,0, 0, 0);
	CurView->WBounds = SaveBounds; 
	AddToPickAp = 0;
{
#if ENABLETRACE
GSSiExitProg (703);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
} 

HANDLE DisplayPointItem2 (HDC hDC,POINT WinPoint,double size, double rot, int Symbol) 
#if ENABLETRACE
{GSSiEnterProg (704);
#endif
{   
	HANDLE	hScreen;
	RECT	Rect;
	
	Rect.left = WinPoint.x - size - 1;
	Rect.right = WinPoint.x + size + 1;
	Rect.top = WinPoint.y - size - 1;
	Rect.bottom = WinPoint.y + size + 1;
	hScreen = SaveScreen2 (hWndMain,hDC, Rect,CurView,0);
	DisplayPointItem (hDC,WinPoint,size,rot,Symbol,0);
{
#if ENABLETRACE
GSSiExitProg (704);
#endif
	return hScreen;
}
#if ENABLETRACE
}
#endif
}

BOOL DisplayPointItem (HDC hDC,POINT WinPoint,double size, double rot, int Symbol,LPLONG pMaxWidth)
#if ENABLETRACE
{GSSiEnterProg (705);
#endif
{ 
	HANDLE hSymbol; 
	int		ii; 
	double	height=0, width=0;
	static	BOOL	fast=FALSE;
	static BOOL first = TRUE;

	if (fast)
	{
		//if (first)
		{
			SaveDC(hDC);
			first = FALSE;
			SetDisplayMode(hDC, GF_TEXTMODE);
		}
		SetPixel (hDC,WinPoint.x,WinPoint.y,0);
		//RestoreDC (hDC,-1);
	{
#if ENABLETRACE
GSSiExitProg (705);
#endif
		return FALSE; 
}
	}
	if (!hDC || !Display)
{
#if ENABLETRACE
GSSiExitProg (705);
#endif
		return FALSE; 
}
	if (size > 0)
		width = size;//max (1,size);
	else
		height = size;					   
	hSymbol = GetDictSymDesc (Symbol,0); 
	hSymdb = hSymbol; 
	if (!hSymbol)
	{
		Symbol = GetDictSymbolNumber ("CIRCLE");
		hSymbol = GetDictSymDesc (Symbol,0); 
		if (!hSymbol)
{
#if ENABLETRACE
GSSiExitProg (705);
#endif
			return FALSE;
}
	}  
	SaveDC (hDC);  
    SetDisplayMode (hDC, GF_TEXTMODE);
    if (pMaxWidth)
    {
    	RECT	Rect;
    	
    	RectInit (&Rect);
		DisplayPointSymbol (hSymbol, hDC, height,width,rot, &WinPoint,&Rect,HighlightPointSym,0,0,FALSE,TRUE,0,0);   
    	*pMaxWidth = max (*pMaxWidth,max (Rect.right-Rect.left,Rect.bottom-Rect.top));
    }
    else
		DisplayPointSymbol (hSymbol, hDC, height,width,rot, &WinPoint,0,HighlightPointSym,0,0,FALSE,TRUE,0,0);   
	if (hSymbol != hSymdb || !hSymbol)
		ii=1;
	DestroySymbol (hSymbol);
	hSymdb = 0; 
    RestoreDC (hDC,-1);
{
#if ENABLETRACE
GSSiExitProg (705);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

BOOL PickPolyline (HPPOINTS lpPointsIn,long nPnts, int PolyID,short InType,LPFLOAT pAZ,long PinA)
#if ENABLETRACE
{GSSiEnterProg (706);
#endif
{
	HPPOINTS	lpPoints = lpPointsIn;
	HANDLE Handle;
	HPPOINT	lpNewPoints, lpPntNew;
	POINT	LastPoint, LastPointN;
	POINT	BeginPointFile, EndPointFile;
	POINT	PickedPointFile;
	POINT	PickAtPoint;
	DPOINT	BeginPoint, EndPoint, PickedPoint, WPoint, LastWPoint, NearPoint,FromPoint,ToPoint;
	DWORD	i, mini; 
	short	n, st;
	double	TotDist, PCT, OffDist, TotDistW, AZ[3], MinDist, Dist, Area=0;
	BOOL	First=TRUE, rtn=FALSE;   
	MNMXCORD	Rect;  
	short	Type=abs(InType); 
	MNMXCORL	Bounds;   
	LPMNMXCORL	pBounds;
	long	FromPtID=-1;
	
	if (nPnts < 1)
	{
#if ENABLETRACE
		GSSiExitProg(706);
#endif
		return FALSE;
	}

	if (PinA)       
	{
		MinMaxInitL (&Bounds);
		pBounds = &Bounds;
		PickAtPoint.x = (CurView->Bounds.xmn + CurView->Bounds.xmx) / 2;
		PickAtPoint.y = (CurView->Bounds.ymn + CurView->Bounds.ymx) / 2;
	}
	else 
	{
		pBounds = &CurView->Bounds;   
	}
	if (PickNET)
	{
		if (!RefInNet (CurrentRefno))
{
#if ENABLETRACE
GSSiExitProg (706);
#endif
			return FALSE;
}
	}
    if (hHighlightArea)
{
#if ENABLETRACE
GSSiExitProg (706);
#endif
    	return (PickPolyInArea(1,lpPoints,nPnts,PolyID)); 
}
    	
    if (Type == 3)
    	nPnts++;
	Handle = GSSiGlobAlloc(GAIDNO 320,GMEM_MOVEABLE,nPnts * (long)sizeof(POINT));
	lpNewPoints = (HPPOINT)GlobalLock (Handle);
	TotDist=TotDistW=0;   
	BeginPointFile = POINTStoPOINT(*lpPoints);
	BeginPoint = FilePtToBasePt(POINTStoPOINT(*lpPoints));
	if (!CurView->FileFactor) CurView->FileFactor=1; 
	MinDist = DBL_MAX;
	for (i=0,lpPntNew = lpNewPoints;i<nPnts;i++,lpPntNew++,lpPoints++)
	{   
		if (Type == 3 && i+1 == nPnts)
			*lpPntNew = BeginPointFile;
		else
			*lpPntNew = POINTStoPOINT(*lpPoints);
		WPoint = FilePtToBasePt(*lpPntNew);
		LastPoint = *lpPntNew; 
		ProjectFilePt (lpPntNew);          
		if (PinA)
			AddPointToMinMaxL (*lpPntNew,pBounds);
		lpPntNew->x /= CurView->FileFactor;
		lpPntNew->y /= CurView->FileFactor;
		if (!First) 
		{
			TotDist += idist(LastPointN,*lpPntNew);
			TotDistW += ldistp(LastWPoint,WPoint);
		} 
		else
			First = FALSE;
		LastPointN = *lpPntNew;
		LastWPoint = WPoint;  
		Dist = ldistp (PickPointBase,WPoint);
		if (Dist < MinDist)
		{
			MinDist = Dist;
			NearPoint = WPoint;
			mini = i;
		}
	}
	EndPointFile = LastPoint;
	EndPoint = FilePtToBasePt(LastPoint);
	if (nPnts > 1)
	{   
		DPOINT Point=FilePtToBasePt(POINTStoPOINT(lpPointsIn[1]));
		AZ[0] = getazd (&BeginPoint,&Point);  
		Point=FilePtToBasePt(POINTStoPOINT(lpPointsIn[nPnts-2]));
		AZ[2] = getazd (&Point,&EndPoint);
	} 
	else if (pAZ)
		AZ[0]=AZ[1]=AZ[2]=*pAZ; 
	else
		AZ[0]=AZ[1]=AZ[2]=0; 
    if (PickingByRefno)
    {
    	PCT = 0.5;
    	OffDist = 0;
    	goto Add;
    } 
    if (PinA)
    {
		st = LineInBounds (nPnts,lpNewPoints,TotDist,&PCT,&OffDist,&AZ[1],pBounds,&PickedPoint,&PickAtPoint,&FromPtID); 
		if (st && OffDist > PickApW)
			st = 0;
	}
    else
		st = LineInBounds (nPnts,lpNewPoints,TotDist,&PCT,&OffDist,&AZ[1],pBounds,&PickedPoint,0,&FromPtID);
	if (!st)
		goto Exit;
    if (WantOnlyShapePoints) 
    {
	   	PickedPoint = NearPoint;  
    	OffDist = ldistp (PickPointBase,PickedPoint);
    }
	for (n=0; n<NumPicked; n++)
	{
		if (CurrentRefno == PickList[n].Refno)
		{
			if (fabs(OffDist)<fabs(PickList[n].OffDist))
				PickListDelete(n);
			else
				goto Exit;
		}
	} 
Add: 
	rtn = TRUE;
	GetItemMinMax (&CurrentItemMinMax,&Rect); 
	if (!PickingByRefno)
		PickedPointFile = BasePtToFilePt (PickedPoint); 
	if (InType < 0)
		OffDist = 9999999 + 10;  
	if (PinA)
//		OffDist = PinA; 
		OffDist = fabs(OffDist) + PinA;
	if (Type == 3)
		Area = ComputeAreaArea (lpNewPoints,nPnts,0);
	if (FromPtID > -1 && nPnts > 1)
	{
		FromPoint = FilePtToBasePt(POINTStoPOINT(lpPointsIn[FromPtID]));
		ToPoint = FilePtToBasePt(POINTStoPOINT(lpPointsIn[FromPtID+1]));
	}
	else
		FromPoint = ToPoint = FilePtToBasePt(POINTStoPOINT(*lpPointsIn));
	PickListAdd (FileNum,SubFile,FileInIndex,ItemSeg,CurrentRefno,CurrentDesc,PolyID,
				   -Type,PCT,OffDist,AZ,TotDistW,CurrentItem,CurElement,
				   BeginPoint,EndPoint,PickedPoint,NearPoint,mini,Area,&Rect,
				   BeginPointFile, EndPointFile,PickedPointFile,nPnts,NULL_ELEV,&FromPoint,&ToPoint);
Exit: 
	GSSiGlobUlFree (&Handle);
{
#if ENABLETRACE
GSSiExitProg (706);
#endif
	return (rtn);
}
#if ENABLETRACE
}
#endif
} 

BOOL PickPolylineD (HPDPOINT lpPointsIn,long nPnts, int PolyID,short Type,LPDOUBLE pAZ,LPDOUBLE pSize, long PinA, LPMNMXCORD pInRect,LPDOUBLE pInLength)
#if ENABLETRACE
{GSSiEnterProg (707);
#endif
{
	DPOINT	BeginPoint, EndPoint, PickedPoint, WPoint, LastWPoint, NearPoint,FromPoint,ToPoint; 
	HPDPOINT lpPoints = lpPointsIn;//lpPointsIn[1]
	POINT	BeginPointFile, EndPointFile, PickedPointFile;
	DWORD	i, mini;
	double	PCT, OffDist, TotDistW, AZ[3], MinDist, Dist, Area=0;
	BOOL	First=TRUE, rtn=FALSE;   
	MNMXCORD	Rect;       
	long	FromPtID=-1;
   	double	PickDist;   

	if (PickNET)
	{
		if (!RefInNet (CurrentRefno))
{
#if ENABLETRACE
GSSiExitProg (707);
#endif
			return FALSE;
}
	}
	if (hProfileSymbols)
{
#if ENABLETRACE
GSSiExitProg (707);
#endif
    	return (AddPolySymbolToProfile(CurrentDesc,1,lpPoints,nPnts,0,hProfileSymbols,&NumProfileSymbols));
}
    if (hHighlightArea)
{
#if ENABLETRACE
GSSiExitProg (707);
#endif
return (PickPolyInAreaD(1, lpPoints, nPnts, 0, pAZ, pSize, 0, 0, 0, 0));
}
	
	if (pInLength)
		TotDistW = *pInLength;
	else
		TotDistW=0;  
	BeginPoint = *lpPoints;
	MinDist = DBL_MAX;
	if (pInRect)
		Rect = *pInRect;
	else
		DBoundsInit (&Rect);
	if (nPnts > 1)
	{
		AZ[0] = getazd (&lpPoints[0],&lpPoints[1]);
		AZ[2] = getazd (&lpPoints[nPnts-2],&lpPoints[nPnts-1]);
	} 
	else if (pAZ)
		AZ[0]=AZ[1]=AZ[2]=*pAZ; 
	else
		AZ[0]=AZ[1]=AZ[2]=0;
	for (i=0;i<nPnts;i++,lpPoints++)
	{   
		AddDPointToMinMax (lpPoints,&Rect);
		WPoint = *lpPoints;
		if (!First) 
			TotDistW += ldistp(LastWPoint,WPoint);
		else
			First = FALSE;
		LastWPoint = WPoint;
		if (!PickOnlyEndPoints || !i || i==nPnts-1)
		{  
			Dist = ldistp (PickPointBase,WPoint);
			if (Dist < MinDist)
			{
				MinDist = Dist;
				NearPoint = WPoint;
				mini = i;
			}
		}
	}
	for (i = 0; i < NumPicked; i++)
	{
		if (CurrentRefno == PickList[i].Refno)
		{
			AddBoundsToBounds(&Rect, &PickList[i].Rect);
			AddBoundsToBounds(&Rect, &HLTBounds);
			PickList[i].Length += TotDistW;
		}
	}
	if (pInRect)
		AddBoundsToBounds(&Rect,pInRect);
	if (pInLength)
		*pInLength += TotDistW;
	EndPoint = LastWPoint;
    if (PickingByRefno)
    {
    	PCT = 0.5;
    	OffDist = 0;
		PickedPoint = LastWPoint;
		PickedPointFile = EndPointFile = BasePtToFilePt (PickedPoint);
		BeginPointFile = BasePtToFilePt (BeginPoint);
    	goto Add;
    }
    if (PickOnlyEndPoints)
    {   
    	OffDist = MinDist;
	    if (OffDist > PickApW) 
			goto Exit;  
		if (mini)
		{
			PCT = 1;  
			AZ[1] = AZ[2];
		}
		else
		{
			PCT = 0; 
			AZ[1] = AZ[0];
		}
		PickedPoint = NearPoint;
	}
	else
	{
		if (!LineInBoundsD (nPnts,lpPointsIn,TotDistW,&PCT,&OffDist,&AZ[1],
						    &CurView->WBounds,&PickedPoint,&FromPtID))
						     goto Exit;   
	    if (WantOnlyShapePoints)
	    {
		   	PickedPoint = NearPoint;  
	    	OffDist = ldistp (PickPointBase,PickedPoint);
	    }
		if (pSize)
			PickDist = max (0,OffDist - *pSize);  
		else
			PickDist = OffDist;
		if (PickDist > PickApW)
			goto Exit;
	}
//	if (nPnts == 1 && AddToPickAp > 0)
//		OffDist = max (0,fabs(OffDist)-AddToPickAp);
	for (i=0; i<NumPicked; i++)
	{
		if (CurrentRefno == PickList[i].Refno)
		{
			if (fabs(OffDist)<fabs(PickList[i].OffDist))
				PickListDelete((int)i);
			else
				goto Exit;
		}
	} 
Add: 
	rtn = TRUE;
	if (PinA)
//		OffDist = PinA; 
		OffDist = fabs(OffDist) + PinA;  
	if (InSnapNode)
		OffDist = ldistp (PickPointBase,NearPoint);
	if (!PickingByRefno)
	{
		PickedPointFile = BasePtToFilePt (PickedPoint);
		BeginPointFile = BasePtToFilePt (BeginPoint);
		EndPointFile = BasePtToFilePt (EndPoint);
	}
	if (Type == 3)
		Area = ComputeAreaAreaD (lpPointsIn,nPnts,&TotDistW); 
	if (pSize)
		TotDistW = *pSize; 
	if (GetGlobalBVal2 ("[%PICKPOINTFIRST]",FALSE))
	{
		if (Type == 1)
			OffDist = 0;
		else if (OffDist >= 0)
			OffDist += P_TOL;
		else
			OffDist -= P_TOL;
	}
	if (nPnts == 1 && pSize)
		ExpandBounds (&Rect, *pSize);  
	if (FromPtID > -1 && nPnts > 1)
	{
		FromPoint = lpPointsIn[FromPtID];
		ToPoint = lpPointsIn[FromPtID+1];
	}
	else
		FromPoint = ToPoint = *lpPointsIn;
	PickListAdd (FileNum,SubFile,FileInIndex,ItemSeg,CurrentRefno,CurrentDesc,PolyID,
				   Type,PCT,OffDist,AZ,TotDistW,CurrentItem,CurElement,
				   BeginPoint,EndPoint,PickedPoint,NearPoint,mini,fabs(Area),&Rect,
				   BeginPointFile, EndPointFile,PickedPointFile,nPnts,NULL_ELEV,&FromPoint,&ToPoint);
Exit:
{
#if ENABLETRACE
GSSiExitProg (707);
#endif
	return (rtn);
}
#if ENABLETRACE
}
#endif
} 

void PickListDelete (int item)
#if ENABLETRACE
{GSSiEnterProg (708);
#endif
{   int i;
	for (i=item+1;i<NumPicked;i++)
	{
		PickList[i-1]=PickList[i];
	}
	NumPicked--;
{
#if ENABLETRACE
GSSiExitProg (708);
#endif
	return;
}
#if ENABLETRACE
}
#endif
}

static int NumUnconnected(int numEndPoints, LPINT ConnectedTo)
{
	int n = 0;
	for (int i = 0; i < numEndPoints; i++)
	{
		if (!ConnectedTo[i])
			n++;
	}
	return n;
}
static double Connect2ClosestPoints(LPDPOINT EndPoints, int numEndPoints, LPINT ConnectedTo)
{
	double minDist = DBL_MAX;
	int p1=0, p2=0;

	for (int i = 0; i < numEndPoints; i++)
	{
		if (!ConnectedTo[i])
		{
			for (int j = i+1; j < numEndPoints; j++)
			{
				if (!ConnectedTo[j])
				{
					if (abs (i-j)>1 || i % 2)
					{
						double dist = ldistpp(&EndPoints[i], &EndPoints[j]);

						if (dist < minDist)
						{
							minDist = dist;
							p1 = i;
							p2 = j;
						}
					}
				}
			}
		}
	}
	ConnectedTo[p1] = p2 +1;
	ConnectedTo[p2] = p1 +1;
	return minDist;
}

static int OppositeEndPoint(int endPoint)
{
	if (endPoint % 2)
		return endPoint - 1;
	return endPoint + 1;
}
BOOL ReorderSavedPolys(void)
{
	BOOL rtn = FALSE;
	BOOL outOfOrder = FALSE;

	if (hSavedPolys && NumSavedPolys > 1)
	{
		LPSAVEPOLY pSavedPolys = (LPSAVEPOLY)GlobalLock(hSavedPolys);
		rtn = TRUE;
		for (int i = 0; i < NumSavedPolys; i++, pSavedPolys++)
		{
			if (pSavedPolys->Type != 1 || pSavedPolys->hSavePolyElev || pSavedPolys->hSavePolyParts)
				rtn = FALSE;
		}
		GlobalUnlock(hSavedPolys);
		if (rtn)
		{
			LPSAVEPOLY pSavedPolys = (LPSAVEPOLY)GlobalLock(hSavedPolys);
			HANDLE hSavedPolysNew = GSSiGlobAlloc(GAIDNO 1813, GHND, NumSavedPolys * sizeof(SAVEPOLY));
			LPSAVEPOLY pSavedPolysNew = (LPSAVEPOLY)GlobalLock(hSavedPolysNew);
			HANDLE hOrderOrig = GSSiGlobAlloc(GAIDNO 1814, GHND, NumSavedPolys * 2 * sizeof(int));
			HANDLE hOrderNew = GSSiGlobAlloc(GAIDNO 1815, GHND, NumSavedPolys * 2 * sizeof(int));
			HANDLE hConnectedTo = GSSiGlobAlloc(GAIDNO 1815, GHND, NumSavedPolys * 2 * sizeof(int));
			LPINT OrderOrig = GlobalLock(hOrderOrig);
			LPINT OrderNew = GlobalLock(hOrderNew);
			LPINT ConnectedTo = GlobalLock(hConnectedTo);
			HANDLE hEndPoints = GSSiGlobAlloc(GAIDNO 1815, GMEM_MOVEABLE, NumSavedPolys * 2 * sizeof(DPOINT));
			LPDPOINT EndPoints = GlobalLock(hEndPoints);
			HANDLE hSavedPolysJoined = 0;

			int numEndPoints = NumSavedPolys * 2;
			int nextEndPoint=0;
			int numNewOrder = 0;
			BOOL haveGap = FALSE;

			BOOL needToReorder = FALSE;
			int nNewPoly = 0;
			int j = 0;
			for (int i = 0; i < NumSavedPolys; i++, pSavedPolys++)
			{
				LPMNMXCORD pBounds = GlobalLock(pSavedPolys->hSavePoly);
				pBounds++;
				LPDPOINT pPoints = (LPDPOINT)pBounds;
				EndPoints[j] = pPoints[0];
				OrderOrig[j] = j++;
				EndPoints[j] = pPoints[pSavedPolys->nPoints-1];
				OrderOrig[j] = j++;
				GlobalUnlock (pSavedPolys->hSavePoly);
			}

			//find the closest unconnected points and connect them until only 2 remain

			while (NumUnconnected (numEndPoints,ConnectedTo) > 2)
			{
				double gap = Connect2ClosestPoints(EndPoints, numEndPoints, ConnectedTo);
				if (gap > P_TOL)
					haveGap = TRUE;
			}
			for (int i = 0; i < numEndPoints; i++)
			{
				if (!ConnectedTo[i])
				{
					nextEndPoint = i;
					break;
				}
			}
			while (numNewOrder < numEndPoints)
			{
				if (numNewOrder > 20)
					ii = 1;
				OrderNew[numNewOrder++] = nextEndPoint;
				nextEndPoint = OppositeEndPoint(nextEndPoint);
				OrderNew[numNewOrder++] = nextEndPoint;
				nextEndPoint = ConnectedTo[nextEndPoint]-1;
				if (nextEndPoint < 0)
					break;
			}
			if (numNewOrder != numEndPoints || memcmp(OrderOrig, OrderNew, numEndPoints * sizeof(int)))
			{
				outOfOrder = TRUE;
				needToReorder = TRUE;
			}
			if (!haveGap)
				needToReorder = TRUE;
			rtn = outOfOrder;
			numEndPoints = numNewOrder;
			if (needToReorder)
			{
				GlobalUnlock(hSavedPolys);
				pSavedPolys = (LPSAVEPOLY)GlobalLock(hSavedPolys);
				int nTotPoints = 0;

				for (int i = 0; i < numEndPoints; i+=2)
				{
					int iPoly = OrderNew[i] / 2;
					pSavedPolysNew[nNewPoly].hSavePoly = pSavedPolys[iPoly].hSavePoly;
					pSavedPolysNew[nNewPoly].nPoints = pSavedPolys[iPoly].nPoints;
					nTotPoints += pSavedPolysNew[nNewPoly++].nPoints;
					if (OrderNew[i] % 2)
					{
						LPMNMXCORD pBounds = GlobalLock(pSavedPolys[iPoly].hSavePoly);
						pBounds++;
						LPDPOINT pPoints = (LPDPOINT)pBounds;
						ReversePoints2(pSavedPolys[iPoly].nPoints, pPoints);
						GlobalUnlock(pSavedPolys[iPoly].hSavePoly);
					}
				}
				if (!haveGap)
				{
					int nJoinedPolys = 1;
					hSavedPolysJoined = GSSiGlobAlloc(GAIDNO 1860, GHND, nJoinedPolys * sizeof(SAVEPOLY));
					LPSAVEPOLY pSavedPolysJoined = (LPSAVEPOLY)GlobalLock(hSavedPolysJoined);
					pSavedPolysJoined->hSavePoly = GSSiGlobAlloc(GAIDNO 1861, GHND, sizeof(MNMXCORD) * nTotPoints * sizeof(DPOINT));
					LPMNMXCORD pBoundsJoined = GlobalLock(pSavedPolysJoined->hSavePoly);
					LPDPOINT pPointsJoined = (LPDPOINT)&pBoundsJoined[1];
					DBoundsInit(pBoundsJoined);
					nTotPoints = 0;
					for (int i = 0; i < nNewPoly;i++)
					{
						LPMNMXCORD pBounds = GlobalLock(pSavedPolysNew[i].hSavePoly);
						AddMinMaxD(pBoundsJoined, pBounds);
						pBounds++;
						LPDPOINT pPoints = (LPDPOINT)pBounds;
						for (j = 0; j < pSavedPolysNew[i].nPoints; j++)
						{
							pPointsJoined[nTotPoints++] = pPoints[j];
						}
						if (i < nNewPoly - 1)
							nTotPoints--;
						GlobalUnlock(pSavedPolysNew[i].hSavePoly);
					}
					pSavedPolysJoined->nPoints = nTotPoints;
					GlobalUnlock(pSavedPolysJoined->hSavePoly);
					GlobalUnlock(hSavedPolysJoined);
				}
			}
			GlobalUnlock(hSavedPolys);
			GlobalUnlock(hSavedPolysNew);
			if (needToReorder && haveGap)
			{
				DestroySavedPolys();
				hSavedPolys = hSavedPolysNew;
			}
			else if (!haveGap)
			{
				DestroySavedPolys();
				hSavedPolys = hSavedPolysJoined;
				NumSavedPolys = 1;
				GSSiGlobFree(&hSavedPolysNew);
			}
			else
			{
				GSSiGlobFree(&hSavedPolysNew);
				hSavedPolys = hSavedPolys;
			}
			GSSiGlobUlFree(&hOrderOrig);
			GSSiGlobUlFree(&hOrderNew);
			GSSiGlobUlFree(&hConnectedTo);
			GSSiGlobUlFree(&hEndPoints);
		}
	}
	return rtn;
}

int ReducePoly(int nPoly, LPINT pnPnts, LPHANDLE phDPoints)
{
	if (nPoly < 2)
		return nPoly;
	int rtn = 0;
	HANDLE hOrderOrig = GSSiGlobAlloc(GAIDNO 1814, GHND, nPoly * 2 * sizeof(int));
	HANDLE hOrderNew = GSSiGlobAlloc(GAIDNO 1815, GHND, nPoly * 2 * sizeof(int));
	HANDLE hConnectedTo = GSSiGlobAlloc(GAIDNO 1815, GHND, nPoly * 2 * sizeof(int));
	LPINT OrderOrig = GlobalLock(hOrderOrig);
	LPINT OrderNew = GlobalLock(hOrderNew);
	LPINT ConnectedTo = GlobalLock(hConnectedTo);
	HANDLE hEndPoints = GSSiGlobAlloc(GAIDNO 1815, GMEM_MOVEABLE, nPoly * 2 * sizeof(DPOINT));
	LPDPOINT EndPoints = GlobalLock(hEndPoints);
	HANDLE hSavedPolysJoined = 0;

	int totPnts = 0;
	int numEndPoints = nPoly * 2;
	int nextEndPoint = 0;
	int numNewOrder = 0;
	BOOL haveGap = FALSE;

	BOOL needToReorder = FALSE;
	int nNewPoly = 0;
	int j = 0;
	for (int iPoly = 0; iPoly < nPoly; iPoly++)
	{
		LPDPOINT pPoints = (LPDPOINT)GlobalLock(phDPoints[iPoly]);
		EndPoints[j] = pPoints[0];
		OrderOrig[j] = j++;
		EndPoints[j] = pPoints[pnPnts[iPoly] - 1];
		OrderOrig[j] = j++;
		totPnts += pnPnts[iPoly];
		GlobalUnlock(phDPoints[iPoly]);
	}
	HANDLE hNewPoints = GSSiGlobAlloc(GAIDNO 1865, GMEM_MOVEABLE, totPnts * sizeof(DPOINT) + 4);
	LPDPOINT pNewPoints = GlobalLock(hNewPoints);
	HANDLE hNewNPoints = GSSiGlobAlloc(GAIDNO 1866, GMEM_MOVEABLE, nPoly * sizeof(int) + 4);
	LPINT pNumNewPoints = GlobalLock(hNewNPoints);
	//find the closest unconnected points and connect them until only 2 remain

	while (NumUnconnected(numEndPoints, ConnectedTo) > 2)
	{
		double gap = Connect2ClosestPoints(EndPoints, numEndPoints, ConnectedTo);
		if (gap > P_TOL)
			haveGap = TRUE;
	}
	for (int i = 0; i < numEndPoints; i++)
	{
		if (!ConnectedTo[i])
		{
			nextEndPoint = i;
			break;
		}
	}
	while (numNewOrder < numEndPoints)
	{
		if (numNewOrder > 20)
			ii = 1;
		OrderNew[numNewOrder++] = nextEndPoint;
		nextEndPoint = OppositeEndPoint(nextEndPoint);
		OrderNew[numNewOrder++] = nextEndPoint;
		nextEndPoint = ConnectedTo[nextEndPoint] - 1;
		if (nextEndPoint < 0)
			break;
	}
	if (numNewOrder != numEndPoints || memcmp(OrderOrig, OrderNew, numEndPoints * sizeof(int)))
		needToReorder = TRUE;
	if (!haveGap)
		needToReorder = TRUE;
	rtn = needToReorder;
	numEndPoints = numNewOrder;
	if (needToReorder)
	{
		int nTotPoints = 0;

		for (int i = 0; i < numEndPoints; i += 2)
		{
			int iPoly = OrderNew[i] / 2;
			LPDPOINT pPoints = (LPDPOINT)GlobalLock(phDPoints[iPoly]);
			memcpy(&pNewPoints[nTotPoints], pPoints, pnPnts[iPoly]*sizeof(DPOINT));
			pNumNewPoints[nNewPoly] = pnPnts[iPoly];
			if (OrderNew[i] % 2)
			{
				ReversePoints2(pNumNewPoints[nNewPoly++], &pNewPoints[nTotPoints]);
			}
			nTotPoints += pnPnts[iPoly];
			GlobalUnlock(phDPoints[iPoly]);
		}
		if (!haveGap)
		{
			int nJoinedPolys = 1;
			rtn = nJoinedPolys;
			for (int iPoly = 0; iPoly < nPoly; iPoly++)
			{
				GSSiGlobFree(&phDPoints[iPoly]);
			}
			GlobalUnlock(hNewPoints);
			phDPoints[0] = hNewPoints;
			pnPnts[0] = nTotPoints;
			/*			LPSAVEPOLY pSavedPolysJoined = (LPSAVEPOLY)GlobalLock(hSavedPolysJoined);
						pSavedPolysJoined->hSavePoly = GSSiGlobAlloc(GAIDNO 1861, GHND, sizeof(MNMXCORD) * nTotPoints * sizeof(DPOINT));
						LPMNMXCORD pBoundsJoined = GlobalLock(pSavedPolysJoined->hSavePoly);
						LPDPOINT pPointsJoined = (LPDPOINT)&pBoundsJoined[1];
						DBoundsInit(pBoundsJoined);
						nTotPoints = 0;
						for (int i = 0; i < nNewPoly; i++)
						{
							LPMNMXCORD pBounds = GlobalLock(pSavedPolysNew[i].hSavePoly);
							AddMinMaxD(pBoundsJoined, pBounds);
							pBounds++;
							LPDPOINT pPoints = (LPDPOINT)pBounds;
							for (j = 0; j < pSavedPolysNew[i].nPoints; j++)
							{
								pPointsJoined[nTotPoints++] = pPoints[j];
							}
							if (i < nNewPoly - 1)
								nTotPoints--;
							GlobalUnlock(pSavedPolysNew[i].hSavePoly);
						}
						pSavedPolysJoined->nPoints = nTotPoints;
						GlobalUnlock(pSavedPolysJoined->hSavePoly);
						GlobalUnlock(hSavedPolysJoined);*/
		}
		else
			ii = 1;
	}
	//GlobalUnlock(hSavedPolys);
	//GlobalUnlock(hSavedPolysNew);
	if (needToReorder && haveGap)
	{
		//DestroySavedPolys();
		//hSavedPolys = hSavedPolysNew;
	}
	else if (!haveGap)
	{
		//DestroySavedPolys();
		//hSavedPolys = hSavedPolysJoined;
		nPoly = 1;
		//GSSiGlobFree(&hSavedPolysNew);
	}
	else
	{
		//GSSiGlobFree(&hSavedPolysNew);
		//hSavedPolys = hSavedPolys;
		rtn = nPoly;
	}
	if (!rtn)
		rtn = nPoly;
	GSSiGlobUlFree(&hNewNPoints);
	GSSiGlobUlFree(&hOrderOrig);
	GSSiGlobUlFree(&hOrderNew);
	GSSiGlobUlFree(&hConnectedTo);
	GSSiGlobUlFree(&hEndPoints);

	return rtn;
}

short GetSavedPolys (void)
#if ENABLETRACE
{GSSiEnterProg (709);
#endif
{   
	LPSAVEPOLY pSavedPolys;
	short	rtn;
    
/*    GSSiGlobFree (&hSavePoly);  
    GSSiGlobFree (&hSavePolyElev);  
    GSSiGlobFree (&hSavePolyParts);
	*/
    if (!hSavedPolys)
{
#if ENABLETRACE
GSSiExitProg (709);
#endif
    	return FALSE;
}
    
    rtn = NumSavedPolys - CurSavedPoly;	
	if (rtn)
	{
    	pSavedPolys = (LPSAVEPOLY)GlobalLock (hSavedPolys);
    	pSavedPolys += CurSavedPoly; 
    	hSavePoly = pSavedPolys->hSavePoly; 
		hSavePolyParts = pSavedPolys->hSavePolyParts;
		nSavePoly = pSavedPolys->nPoints;
		hSavePolyElev = pSavedPolys->hSavePolyElev;
    	GlobalUnlock (hSavedPolys);
    	CurSavedPoly++;
{
#if ENABLETRACE
GSSiExitProg (709);
#endif
		return rtn;
}
	}
	DestroySavedPolys ();
{
#if ENABLETRACE
GSSiExitProg (709);
#endif
	return rtn;
}
#if ENABLETRACE
}
#endif
}

void DestroySavedPolys (void)
#if ENABLETRACE
{GSSiEnterProg (710);
#endif
{   
	LPSAVEPOLY pSavedPolys;
	
/*    GSSiGlobFree (&hSavePoly);  
    GSSiGlobFree (&hSavePolyParts);
    GSSiGlobFree (&hSavePolyElev);
	*/
	hSavePolyParts = 0;
	hSavePolyElev = 0;
	CurSavedPoly = 0;
	if (hSavedPolys)
    {
	    while (CurSavedPoly < NumSavedPolys)
	    {
	    	pSavedPolys = (LPSAVEPOLY)GlobalLock (hSavedPolys);
	    	pSavedPolys += CurSavedPoly; 
	    	GSSiGlobFree (&pSavedPolys->hSavePoly); 
			GSSiGlobFree (&pSavedPolys->hSavePolyParts); 
			GSSiGlobFree (&pSavedPolys->hSavePolyElev);
	    	GlobalUnlock (hSavedPolys);
	    	CurSavedPoly++;
	    }
	    GSSiGlobFree (&hSavedPolys);
	}
	NumSavedPolys = 0;   
	CurSavedPoly = 0;
{
#if ENABLETRACE
GSSiExitProg (710);
#endif
    return;
}
#if ENABLETRACE
}
#endif
}

BOOL SavePolyLine (short type)
#if ENABLETRACE
{GSSiEnterProg (711);
#endif
{	 
	HPPOINTS	lpPoints, lppoints; 
	DPOINT	FirstPoint;
	HPDPOINT	lpDpoints, lpDPoints, pp; 
	LPMNMXCORD	lpRect, lpRect2;    
	LPMINMAX	lpRectFile, lpRectFile2;
	BOOL	FirstIn, BothIn, NeitherIn;
	float	factor;
	long	RefDist,np;
	POINT	RefPoint;
	DWORD	i;
    
    if (WantElement < LONG_MAX && CurElement != WantElement)
{
#if ENABLETRACE
GSSiExitProg (711);
#endif
    	return FALSE;              
}
    if (CurrentType != GF_POLYLINE && CurrentType != GF_LINE && CurrentType != GF_AREA &&
    	CurrentType != GF_TEXT && CurrentType != GF_CURVE)
{
#if ENABLETRACE
GSSiExitProg (711);
#endif
    	return FALSE; 
}
    if (type == 3 && (CurrentType == GF_POLYLINE || CurrentType == GF_LINE))
    	type = 1;
    switch (type)
    {
    	case 1:
    	case 3:
	    if (WantUnsplinedPoints && hUnSplinedPoly) 
	    {
	    	pp = (HPDPOINT)GlobalLock (hUnSplinedPoly);
			np = nUnSplinedPoints; 
		}
		else  
		{
			pp = lpDCurPoints;
			np = nPnts;       
		}
		hSavePoly = GSSiGlobAlloc(GAIDNO 321,GMEM_MOVEABLE,(long)sizeof(MNMXCORD)+(long)(np+1)*sizeof(DPOINT));
		nSavePoly = np;  
		if (hElevBuffer)
		{
			HPFLOAT	pElev, pElev2;
			
			hSavePolyElev = GSSiGlobAlloc(GAIDNO 1726,GMEM_MOVEABLE,(long)(np+1)*sizeof(double)); 
			pElev = (HPFLOAT)GlobalLock (hElevBuffer);
			pElev2 = (HPFLOAT)GlobalLock (hSavePolyElev);
			for (i=0;i<np;i++) 
				pElev2[i] = pElev[i];
			GlobalUnlock (hElevBuffer);
			GlobalUnlock (hSavePolyElev);
		}
	    lpRect = (LPMNMXCORD) GlobalLock (hSavePoly); 
	    DBoundsInit (lpRect);
	    lpRect2 = lpRect;
	    lpRect2++;
	    lpDpoints = (HPDPOINT) lpRect2; 
	    if (HiPrecis)
	    {
			lpDPoints = pp;  
			for (i=0;i<np;i++,lpDpoints++,lpDPoints++)
			{   
				*lpDpoints = *lpDPoints;
				if (!i)
					FirstPoint = *lpDpoints;
				AddDPointToMinMax (lpDpoints,lpRect); 
			} 
		}
	    else
	    {
			lpPoints = lpCurPoints;  
			for (i=0;i<nPnts;i++,lpPoints++,lpDpoints++)
			{
				*lpDpoints = FilePtToBasePt(POINTStoPOINT(*lpPoints));  
				if (!i)
					FirstPoint = *lpDpoints;
				AddDPointToMinMax (lpDpoints,lpRect); 
			} 
		}
	    if (WantUnsplinedPoints && hUnSplinedPoly) 
	    	GlobalUnlock (hUnSplinedPoly);
		if (type == 3)
		{   
			LPINT	pPartLen, pPolyParts;
			
			lpDpoints--;  
			if (ldistp (FirstPoint,*lpDpoints) > P_TOL)
			{
				nSavePoly++;
				lpDpoints++;  
				*lpDpoints = FirstPoint;
			}
			if (nPoly)
			{
				hSavePolyParts = GSSiGlobAlloc(GAIDNO 322,GMEM_MOVEABLE,(nPoly+1)*sizeof(int));
				pPolyParts = (LPINT)GlobalLock (hSavePolyParts); 
				*pPolyParts++ = nPoly;
				pPartLen = (LPINT)GlobalLock (hPolyPartLen);
				_fmemmove (pPolyParts,pPartLen,nPoly*sizeof(int));
				GlobalUnlock (hSavePolyParts);
				GlobalUnlock (hPolyPartLen); 
			}
			else
				GSSiGlobFree (&hSavePolyParts);
		}   
		break;
		
		case 2:
		np = nPnts;
	    if (HiPrecis)
	    {
			lpDPoints = lpDCurPoints; 
		    if (WantUnsplinedPoints && hUnSplinedPoly) 
		    {
		    	pp = (HPDPOINT)GlobalLock (hUnSplinedPoly);
				np = nUnSplinedPoints; 
			}
			else  
			{
				pp = lpDCurPoints;
			} 
		}	
		else 
			lpPoints = lpCurPoints;  
		hSavePoly = GSSiGlobAlloc(GAIDNO 323,GMEM_MOVEABLE,(long)sizeof(mnmxCor)+(long)np*sizeof(POINT));
	    nSavePoly = np;
	    lpRectFile = (LPMINMAX) GlobalLock (hSavePoly);
	    MinMaxInit (lpRectFile); 
	    lpRectFile2 = lpRectFile;
	    lpRectFile2++;
	    lppoints = (HPPOINTS) lpRectFile2;
		for (i=0;i<np;i++,lppoints++)
		{
		    if (HiPrecis)
				*lppoints = POINTtoPOINTS(BasePtToFilePt (*pp++)); 
		    else
				*lppoints = *lpPoints++; 
			lpRectFile->xmn = min (lpRectFile->xmn,lppoints->x);
			lpRectFile->ymn = min (lpRectFile->ymn,lppoints->y);
			lpRectFile->xmx = max (lpRectFile->xmx,lppoints->x);
			lpRectFile->ymx = max (lpRectFile->ymx,lppoints->y);
		}   
	    if (HiPrecis && WantUnsplinedPoints && hUnSplinedPoly) 
	    	GlobalUnlock (hUnSplinedPoly);
		break;  
	}
	GlobalUnlock(hSavePoly);  
	SavePolyType = type;
	AddSavedPolys ();
{
#if ENABLETRACE
GSSiExitProg (711);
#endif
	return(TRUE);
}
#if ENABLETRACE
}
#endif
}




BOOL OffsetPickedArea (int Item, double Dist)
#if ENABLETRACE
{GSSiEnterProg (713);
#endif
{
	long np;
	HANDLE hDPoints;  
	HPDPOINT	lpDPoints;
	
	if ((hDPoints = OffsetPickedArea2 (Item,Dist,&np)))
	{
        lpDPoints = (HPDPOINT)GlobalLock (hDPoints);
		AddAreaToOffsetFile (PickList[Item].Refno,3,np, lpDPoints,1,0,0);
		GSSiGlobUlFree (&hDPoints);
{
#if ENABLETRACE
GSSiExitProg (713);
#endif
		return TRUE;
}
    }               
{
#if ENABLETRACE
GSSiExitProg (713);
#endif
    return FALSE;
}
#if ENABLETRACE
}
#endif
}

void SaveTINData(LPDPOINT pDPoint, HANDLE hElev)
{
	static BOOL haveData = FALSE;
	if (pDPoint)
	{
		char cbuf[256];
		LPFLOAT pElev = GlobalLock(hElev);

		sprintf(cbuf, "%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f",
			pDPoint[0].x*MFT, pDPoint[0].y*MFT, pElev[0] * MFT,
			pDPoint[1].x*MFT, pDPoint[1].y*MFT, pElev[1] * MFT,
			pDPoint[2].x*MFT, pDPoint[2].y*MFT, pElev[2] * MFT);
		GlobalUnlock(hElev);
		fputstring(cbuf, FidTINExtract);
		haveData = TRUE;
	}
	else if (haveData)
	{
		GSSiClose2 (&FidTINExtract);
		FidTINExtract = HFILE_ERROR;
		haveData = FALSE;
	}
	return;
}

 

