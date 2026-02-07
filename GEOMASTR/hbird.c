#include "graphint.h" 
#include "FreeImage.h"
#include "gmextern.h"   
#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

#define	TGMAXSEGS	4096
#define	MAXPATTERN	8
#define nWantSplinePoints	8
#define MaxSplinePoints	1024

#define  TRIANGLE_DEPTH_MULTIPLIER	2

BPOINT POINTtoBPOINT (POINT p);
POINT BPOINTtoPOINT (BPOINT p);

typedef struct {int width, height;
				int	xoff,  yoff;
				int rectw, recth;
				int	destx, desty;
				int	destw, desth;
				int	PaletteLen;
				RGBQUAD	Palette[1024];
				BYTE Image[4];
				}LKMTILE;

typedef LKMTILE *PLKMTile;
typedef struct {short nPnts;
				CPOINT Points[MAXPATTERN];
				}CPKEY;
typedef struct {CPKEY key;
				int	  count;
				}CPRECORD;
typedef CPRECORD *LPCPRECORD;

typedef struct {
				int				uncompressedLength;
				unsigned short	iConTextOffset;
				unsigned short	iSegmentOffset;
				unsigned short	iTriangleOffset;
				unsigned short	nSegments;
				unsigned short	nLines;
				unsigned short	nTriangles;
				int				totLinePoints;
				int				triPointsLen;
				}TILEGRAPHICSHEADER;
typedef TILEGRAPHICSHEADER *LPTILEGRAPHICSHEADER;

static	TILEGRAPHICSHEADER tileGraphicsHeader;

typedef struct {
				unsigned short	xory;
				short			angleWeight;
				short			depth;
				}POLYCROSSING;
typedef POLYCROSSING *LPPOLYCROSSING;

static	MNMXCORD wtrbounds, wtrbounds45;
static	LPPOLYCROSSING pxCrossings0, pyCrossings0;
static	LPPOLYCROSSING pxCrossings45, pyCrossings45;
static	int maxCrossing;
static	int	nrows0, ncols0, nrows45, ncols45;
static	int ixbase0, iybase0,ixbase45, iybase45;
static	int	maxDTLev = 20;
static	HANDLE hTran45=0;
//static	char	Symnames[2048][16];
static	HFILE	FidHBirdData=HFILE_ERROR, FidHBirdIndex;
static	int		LastHBLoc,LastHBLen;
static	int		NumTiles=0;
static	int		Frequency[1024];
static	short	SymDep[2000][2];
static	short	minsym, maxsym;
static	short	nsym;
static	short	triPoints[257][257];
static	BYTE	triPoints8[257][257];

static	int		tgNumSegPoints[TGMAXSEGS];
static	HANDLE	tghSegs[TGMAXSEGS]={0};
static	int		tgNumSegs;
static	HANDLE	hCPDB=0;
static	BYTE	SymDepth[2500][2];
static	char	MapPrefix[4]="mn";
static	RGBQUAD	ClassicDepthColor = {255,180,180,0};
static	RGBQUAD	HighlightDepthColor = {255,128,128,0};
static	COLORREF	backGroundColor = RGB(255,255,196);
static	BOOL	ShowContourLines;
static	BOOL	ShowDepthColors;
static	int		HighlightDepth;
static	int		HighlightDepthRange;
static	int		DepthFactor=4;
static	HFILE	FidSTG=HFILE_ERROR;
static	int		currentZoom;
static	int		currentRow;
static	int		currentCol;
static	double	currentTileMetersPerPixel;
static	int		maxZoom=18;
static	BOOL	haveTiltFile;
static	BOOL	containsSymbolColors=FALSE;

BOOL DoFilter,FlipText;
int LKMToHBInit (char *PathToLKMData);
int LakeMasterToHBirdImage (HANDLE HBImageHandle,int HBImageWidth,int HBImageHeight,int CenterX,int CenterY,double Scale,int Rotation,
							int WantDepthColors,int WantContourLines,int HighlightDepth,int HighlightDepthRange);
int GetPatternPoints (int iPattern,LPPOINT pPoints);
int GetCPPatternID (int npMax,LPBPOINT points,LPINT pInc);
RGBQUAD ColorOfDepth (int Depth);
int clockwiseDist (POINT fromPt,POINT toPt);
int CreateCrossingSpline (int nCrossings,LPPOLYCROSSING pCrossings,LPDPOINT *ppSplinedPoints);


#include "ContourPatterns.h"

static short cl_freadS16(FILE *pFID)
{
   unsigned char tempChar;
   const size_t numBytesToRead = 2;
   size_t numBytesRead = 0;
   union {
      unsigned char u8[2];
      short s16;
   } buf;

   buf.s16 = 0;

   numBytesRead = fread (&(buf.u8),1,numBytesToRead,pFID);
   if (numBytesRead != numBytesToRead)
   {
 //    WRITE_TO_ERROR_LOG2( clError_FileIo );
     return buf.s16;
   }


   return buf.s16;
}
static	BOOL LoadHBSymList (LPSTR Path)
{
	char	SymListFile[MAX_PATH];
	int		type=2, nsym, i;
	FILE	*Fid;

	//sprintf (SymListFile,"%s%s%s",Path,MapPrefix,SYMLIST2BIN);
	strcpy (SymListFile,Path);
	Fid = fopen (SymListFile,"rb");
/*	if (!Fid)
	{
		sprintf (SymListFile,"%s%s%s",Path,MapPrefix,SYMLISTBIN);
		Fid = fopen2 (SymListFile,"rb");

		if (!Fid)
			return FALSE;
	}
	else
		type = 2;*/
	minsym = cl_freadS16(Fid);
	maxsym = cl_freadS16(Fid);
	nsym = (maxsym-minsym+1);
	if (type == 1)
	{
		ii=fread (SymDepth,1,nsym*2,Fid);
		for (i=0;i<nsym;i++)
		{
			SymDep[i][0] = SymDepth[i][0];
			SymDep[i][1] = SymDepth[i][1];
		}
	}
	else
   {
		fread (SymDep,1,nsym*4,Fid);
		for (i=0;i<nsym;i++)
		{
			swap16(&(SymDep[i][0]));
			swap16(&(SymDep[i][1]));
		}
   }

	fclose (Fid);
	return TRUE;
}
static RGBQUAD ColorOfDepth (int Depth)
{
	RGBQUAD c;

	c.rgbBlue = 255;
	Depth /= 4;
	if (DepthFactor < 4)
		Depth *= (5-DepthFactor);
	else if (DepthFactor > 4)
		Depth /= (DepthFactor-3);
	Depth = min (255,Depth);
	c.rgbGreen = c.rgbRed = max (0,255 - Depth);
	return c;
}

static RGBQUAD DepthColor (int Depth,int HighlightDepth,int HighlightDepthRange,BOOL ShowDepthColors)
{
	RGBQUAD c;

	c.rgbReserved = 0;
	if (!ShowDepthColors && (!HighlightDepth || !HighlightDepthRange))
		c = ClassicDepthColor;
	else if (!HighlightDepth)
		c = ColorOfDepth (Depth);
	else if (Depth >= HighlightDepth - HighlightDepthRange && Depth <= HighlightDepth + HighlightDepthRange)
		c = HighlightDepthColor;
	else if (!ShowDepthColors)
		c = ClassicDepthColor;
	else
		c = ColorOfDepth (Depth);
	return c;
}

int GetSymbolDepth (int SymNum,LPINT prtn)
{
	static	BOOL first=TRUE;
	int	DepthOffset = 0, Depth=0, Depth1=0, Depth2=0;
	int	HighlightDepth = 0;
	int	HighlightDepthRange = 0;
	
	*prtn = 0;
	if (first)
	{
		char symListFile[MAX_PATH];

		GetGlobalCVal ("[%SYMLISTFILE]",symListFile,0);
		LoadHBSymList (symListFile);
	}
	first = FALSE;
	if (SymNum >= minsym && SymNum <= maxsym)
	{
		SymNum -= minsym;
		if (!SymDep[SymNum][1])
		{
			Depth = max (0,SymDep[SymNum][0] + DepthOffset) * 8;
			*prtn = 1;
		}
		else
		{
			Depth1 = max (0,SymDep[SymNum][0] + DepthOffset);
			Depth2 = max (0,SymDep[SymNum][1] + DepthOffset);
			Depth = (Depth1 + Depth2) * 4;
			*prtn = 2;
		}
	}
	return Depth;
}

HPEN GetPenForSymbol (int SymNum)
{
	HPEN hPen;
	RGBQUAD rcolor;
	COLORREF color=0;
	int	depth;
	int	type;

	if ((depth = GetSymbolDepth (SymNum,&type)))
	{
		if (ShowContourLines)
		{
			rcolor = ColorOfDepth (depth);
			color = COLORREFFromRGBQUAD (rcolor);
		}
	}
	hPen = CreatePen (PS_SOLID,1,color);
	return hPen;
}

HBRUSH GetBrushForSymbol (int SymNum)
{
	HBRUSH hBrush;
	RGBQUAD rcolor;
	int	depth;
	int	type;
	COLORREF color=backGroundColor;

	if ((depth = GetSymbolDepth (SymNum,&type)))
	{
		rcolor = DepthColor (depth,HighlightDepth,HighlightDepthRange,TRUE);
		color = COLORREFFromRGBQUAD (rcolor);
	}
	hBrush = CreateSolidBrush (color);
	return hBrush;
}

double distpd(LPDPOINT Point1, LPDPOINT Point2)
{
    return sqrt ((double)(Point1->x-Point2->x)*(double)(Point1->x-Point2->x) +
                 (double)(Point1->y-Point2->y)*(double)(Point1->y-Point2->y));
}
double distp(POINT Point1,POINT Point2)
{
    return sqrt ((double)(Point1.x-Point2.x)*(double)(Point1.x-Point2.x) +
                 (double)(Point1.y-Point2.y)*(double)(Point1.y-Point2.y));
}
BOOL iIntersectLines (LPDPOINT fromPt,LPDPOINT fromEP,double *fromAz,LPDPOINT toPt,LPDPOINT toEP,double *toAz,LPPOINT pIntPt)
{
	DPOINT	IntPt;
	double X1IN = fromPt->x;
	double Y1IN = fromPt->y;
	double X2IN = toPt->x;
	double Y2IN = toPt->y;
	double A1 = *fromAz;
	double A2 = *toAz;
    double X2, Y2, X3, Y3, Z1, Z2, DM1, DM2, TOL = 1e-4, ADIFF,  TAZ, lineLength;
    int       RC;

      X2 = X2IN - X1IN; /* delta_x*/
      Y2 = Y2IN - Y1IN; /*  delta_y */
      RC = 1;
      Z1 = fabs(cos(A1)); /*   cosine of line in radians  */
      Z2 = fabs(cos(A2));
      ADIFF = fabs (A1-A2);
      if (ADIFF < (TOL * 1e-3)) {goto S10;}/*  parallel  check for coincident */
      if (fabs (PY-ADIFF) < (TOL * 1e-3)) {goto S10;} /*parallel  check for coincident*/
      if (min(Z1, Z2) < (TOL * 1e-3)) {goto S30;} /*going straight up */
      DM1 = tan(A1);  /*tangent of line in radians */
      DM2 = tan(A2);
      if (fabs(DM1-DM2) < (TOL * 1e-3))
       {goto S10;} /*parallel  check for coincident */

    /*gets here if lines aren't parallel or going straight up */
      X3 = -(-Y2+DM2*X2)/(DM1-DM2);
      Y3 = DM1* X3;
      goto S1000;

/*c*    gets here if the lines are virtually parallel*/
S10:   X3 = 0;
       Y3 = 0;
       RC = 2; /*lines parallel and NOT coincident */
       if (fabs(Z1+Z2) < (TOL * 1e-3)) 
       		goto S50; /*going straight up */
       RC = 0; /*lines parallel and coincident*/
       TAZ = LDIST(X1IN, Y1IN, X2IN, Y2IN);
       if (TAZ <= TOL)
       		goto S1000; /*lines start the same place*/
       TAZ= LGETAZ(X1IN, Y1IN, X2IN, Y2IN);
       if(fabs(TAZ-A1) <= (TOL * 1e-3)){
             X3 = X2IN;
             Y3 = Y2IN;}
       else
         {if(fabs(TAZ-LTWOPI(A1+PY)) <= (TOL * 1e-3))
            { X3 = X1IN;
              Y3 = Y1IN;}
          else 
          	 return FALSE;
         }
      goto S1000;

/*c*    here it handles lines going straight up*/
S30:  if (Z2 < (TOL * 1e-3)) {goto S40;}
      X3 = 0;
      Y3 = tan(A2)*(X3-X2)+Y2;
      goto S1000;
S40:  if (Z1 < (TOL * 1e-3)){ goto S50;}
      X3 = X2;
      Y3 = tan(A1)* X3;
      goto S1000;
S50:  if (fabs(X2) <= (TOL * 1e-3))
		 return FALSE;
S1000:IntPt.x = X3 + X1IN ;
      IntPt.y = Y3 + Y1IN ;
	  lineLength = distpd (fromPt,fromEP) + 0.1;
	  if (distpd (&IntPt,fromPt) > lineLength)
		  return FALSE;
	  if (distpd (&IntPt,fromEP) > lineLength)
		  return FALSE;
	  lineLength = distpd (toPt,toEP) + 0.1;
	  if (distpd (&IntPt,toPt) > lineLength)
		  return FALSE;
	  if (distpd (&IntPt,toEP) > lineLength)
		  return FALSE;
	  *pIntPt = DPointToPoint (IntPt);
      return(RC==1);
}

POINT GetIntersectWithVP (POINT pt1,POINT pt2,LPPOINT haveIntPt)
{
	POINT	intPt;
	DPOINT	fromPt, fromEP, toPt, toEP;
	double	fromAz, toAz;

	fromPt = PointToDPoint (pt1);
	fromEP = PointToDPoint (pt2);
	fromAz = getaz (pt1,pt2);

	toPt.x = CurView->ScreenRect.left;
	toPt.y = CurView->ScreenRect.top;
	toEP.x = CurView->ScreenRect.right;
	toEP.y = CurView->ScreenRect.top;
	toAz = 0;
	if (iIntersectLines (&fromPt,&fromEP,&fromAz,&toPt,&toEP,&toAz,&intPt))
	{
		if (!haveIntPt || (intPt.x != haveIntPt->x && intPt.y != haveIntPt->y))
			return intPt;
	}
	toPt.x = CurView->ScreenRect.left;
	toPt.y = CurView->ScreenRect.bottom;
	toEP.x = CurView->ScreenRect.right;
	toEP.y = CurView->ScreenRect.bottom;
	toAz = 0;
	if (iIntersectLines (&fromPt,&fromEP,&fromAz,&toPt,&toEP,&toAz,&intPt))
	{
		if (!haveIntPt || (intPt.x != haveIntPt->x && intPt.y != haveIntPt->y))
			return intPt;
	}
	toPt.x = CurView->ScreenRect.left;
	toPt.y = CurView->ScreenRect.top;
	toEP.x = CurView->ScreenRect.left;
	toEP.y = CurView->ScreenRect.bottom;
	toAz = HALFPI;
	if (iIntersectLines (&fromPt,&fromEP,&fromAz,&toPt,&toEP,&toAz,&intPt))
	{
		if (!haveIntPt || (intPt.x != haveIntPt->x && intPt.y != haveIntPt->y))
			return intPt;
	}
	toPt.x = CurView->ScreenRect.right;
	toPt.y = CurView->ScreenRect.top;
	toEP.x = CurView->ScreenRect.right;
	toEP.y = CurView->ScreenRect.bottom;
	toAz = HALFPI;
	if (iIntersectLines (&fromPt,&fromEP,&fromAz,&toPt,&toEP,&toAz,&intPt))
	{
		if (!haveIntPt || (intPt.x != haveIntPt->x && intPt.y != haveIntPt->y))
			return intPt;
	}
	return intPt;
}

BOOL CreateCPPatternInclude (void)
{
	char	numPstr[4096]="";
	char	pntStr[4096]="";
	char	numPdelim = ' ';
	char	pntDelim=' ';
	LPBYTE	pByte;
	LPCPRECORD pCPRecord;
	int		lastx, lasty;

	LPGWDHEADER lpGWDHead;
	int	Offset, len, i,n=0;
	int	pos = BT_FIRST;
	HFILE	Fid;

	hCPDB = OpenGWDatabase ("c:\\temp\\ContourPatterns.gmd",BT_READ);

	if (!hCPDB)
		return FALSE;
	
	lpGWDHead = (LPGWDHEADER)GlobalLock (hCPDB); 
	pCPRecord = (LPCPRECORD)&lpGWDHead->GWDData; 
	
	Fid = GSSiOpenFile ("..\\include\\ContourPatterns.h",0,OF_CREATE);
	fputstring ("static POINTS cpPoints[128][8]={",Fid);
	strcpy (numPstr,"static short cpNumP[128]={");

	while (n++<128 && !BT_FIND (lpGWDHead->BTHandle[1],lpGWDHead->pKeys[1],pos,BT_ANY,(LPSTR)&Offset))
    {   
		pos = BT_NEXT;
    	len = FillGWDData (lpGWDHead,Offset); 
		sprintf (strchr(numPstr,0),"%c%i",numPdelim,pCPRecord->key.nPnts);
		numPdelim = ',';
		pByte = (LPBYTE)&pCPRecord->key.Points[0];
		for (i=0;i<sizeof(pCPRecord->key.Points);i++,pByte++)
			if (*pByte == 0x7f)
				*pByte = 0;

		*pntStr = 0;
		lastx = lasty = 0;
		for (i=0;i<8;i++)
		{
			sprintf (strchr (pntStr,0),"%c%i,%i",pntDelim,pCPRecord->key.Points[i].x,pCPRecord->key.Points[i].y);
			lastx = pCPRecord->key.Points[i].x;
			lasty = pCPRecord->key.Points[i].y;
			pntDelim = ',';
		}
		fputstring (pntStr,Fid);
    }
	GlobalUnlock (hCPDB);
	CloseGWDatabase (hCPDB);
	fputstring ("};",Fid);
	strcat (numPstr,"};");
	fputstring (numPstr,Fid);
	GSSiClose2 (&Fid);
	return TRUE;
}

int GetPatternPoints (int iPattern,LPPOINT pPoints)
{
	int	i;

	iPattern--;
	for (i=0;i<cpNumP[iPattern];i++)
	{
		pPoints[i+1].x = pPoints[i].x + cpPoints[iPattern][i].x;
		pPoints[i+1].y = pPoints[i].y + cpPoints[iPattern][i].y;
	}
	return cpNumP[iPattern];
}

int GetCPPatternID (int npMax,LPBPOINT points,LPINT pInc)
{
	unsigned int id=0, np, i;
	POINTS	patPnt[8];

	for (i=0;i<min(npMax,8);i++)
	{
		patPnt[i].x = points[i+1].x - points[i].x;
		patPnt[i].y = points[i+1].y - points[i].y;
	}

	for (np = min(npMax,8);np > 1;np--)
	{
		for (id=0;id<127;id++)
		{
			if (np != cpNumP[id])
				continue;
			if (memcmp (patPnt,cpPoints[id],np*sizeof(POINTS)))
				continue;
			*pInc = np;
			return id+1;
		}
	}
	return 0;
}
/*short ReplaceWithPatterns (int nPnts,LPCPOINT pSeg,LPHANDLE phPnt2)
{
	int np2=2, nPntsOrig=nPnts;
	int	ncurp;
	LPBYTE	pPnt2, pPnt2Beg, pcurp;
	
	*phPnt2 = GSSiGlobAlloc(GAIDNO 1768,GMEM_MOVEABLE,nPnts*sizeof(POINT));
	pPnt2 = pPnt2Beg = GlobalLock (*phPnt2);
	pPnt2[0] = *(LPBYTE)&pSeg[0];
	pPnt2[1] = *(LPBYTE)&pSeg[0];

	pPnt2 += 2;
	nPnts -= 2;
	pSeg += 2;
	ncurp = 0;
	pcurp = pPnt2;
	while (nPnts > 0)
	{
		int	inc;
		int id = GetCPPatternID (nPnts--,pSeg,&inc);
		
		if (id)
		{
			if (ncurp)
			{
				if (ncurp > 128)
					ii=1;
				*pcurp = ncurp;
			}
			*pPnt2++ = -id;
			ncurp = 0;
			pcurp = pPnt2;
			pSeg += inc;
			nPnts -= inc;
		}
		else
		{
			if (!ncurp++)
				pPnt2++;
			*pPnt2++ = *(LPBYTE)pSeg++;
			nPnts--;
		}
	}
	if (ncurp)
	{
		if (ncurp > 128)
			ii=1;
		*pcurp = ncurp;
	}
	np2 = (int)pPnt2 - (int)pPnt2Beg;
	if (np2 >= nPntsOrig)
	{
		np2 = nPnts;
		GSSiGlobUlFree (phPnt2);
	}
	else
		GlobalUnlock (*phPnt2);
	return np2;
}*/

int	ConvertTGSeg (int np,LPHANDLE phSeg)
{
	int	rtn=np;
	LPBPOINT pSeg=GlobalLock (*phSeg);
	BPOINT	ePt=pSeg[np-1];
	HANDLE	hcSeg=GSSiGlobAlloc(GAIDNO 1766,GMEM_MOVEABLE,(np*8+16)*sizeof(BPOINT));
	LPCPOINT pcSeg=GlobalLock (hcSeg);
	int	xdiff, ydiff;
	int	nNewPt=0;
	int	maxSize = np * sizeof (BPOINT);
	int	iSize = sizeof (BPOINT);
	int	i, j, xinc, yinc, numMoves,pntsInPattern;

	*(LPBPOINT)pcSeg = *pSeg;
	nNewPt = 2;
	for (i=1;i<np;i++)
	{
		int	iPattern = GetCPPatternID (np-i,&pSeg[i-1],&pntsInPattern);

		if (iPattern && iPattern < 16)
		{
			pcSeg[nNewPt].x = -8;
			pcSeg[nNewPt++].y = 8 - iPattern;
			i += pntsInPattern-1;
		}
		else
		{
			xdiff = pSeg[i].x - pSeg[i-1].x;
			ydiff = pSeg[i].y - pSeg[i-1].y;
			if (abs (xdiff) > 14 || abs (ydiff) > 14)
			{
				pcSeg[nNewPt].x = -8;
				pcSeg[nNewPt++].y = -8;
				memcpy (&pcSeg[nNewPt],&pSeg[i],sizeof(BPOINT));
				nNewPt+=2;
			}
			else if (abs (xdiff) > 7 || abs (ydiff) > 7)
			{
				pcSeg[nNewPt].x = xdiff/2;
				pcSeg[nNewPt++].y = ydiff/2;
				pcSeg[nNewPt].x = xdiff - xdiff/2;
				pcSeg[nNewPt++].y = ydiff - ydiff/2;
			}
			else
			{
				pcSeg[nNewPt].x = xdiff;
				pcSeg[nNewPt++].y = ydiff;
			}
		}
//		numMoves = max (abs(xdiff)/7,abs(ydiff)/7)+1;
//		if (numMoves > 15)
//			goto UseOriginal;
//		xinc = xdiff / numMoves;
//
		/*		if (numMoves > 2)
		{
			pcSeg[nNewPt].x = -8;
			pcSeg[nNewPt++].y = (numMoves - 1) - 8;
			pcSeg[nNewPt].x = xinc;
			xdiff -= xinc * (numMoves - 1);
			pcSeg[nNewPt++].y = yinc;
			ydiff -= yinc * (numMoves - 1);
			iSize += 2 * sizeof (CPOINT);
		}
		else
		{
			for (j=0;j<numMoves-1;j++)
			{
				pcSeg[nNewPt].x = xinc;
				xdiff -= xinc;
				pcSeg[nNewPt++].y = yinc;
				ydiff -= yinc;
				iSize += sizeof (CPOINT);
			}
		}
		if (xdiff || ydiff)
		{
			pcSeg[nNewPt].x = xdiff;
			pcSeg[nNewPt++].y = ydiff;
			iSize += sizeof (CPOINT);
		}*/
//		if (iSize >= maxSize)
//			goto UseOriginal;
	}
/*	{
		HANDLE	hPnt2=0;
		short nPnts2 = ReplaceWithPatterns (nNewPt,(LPCPOINT)pcSeg,&hPnt2);
		
		ii=1;
	}*/
	iSize = nNewPt * sizeof (CPOINT);
	if (iSize >= maxSize)
		goto UseOriginal;
	GSSiGlobUlFree (phSeg);
	GlobalUnlock (hcSeg);
	*phSeg = hcSeg;
	return -nNewPt;
UseOriginal:
	GlobalUnlock (*phSeg);
	GSSiGlobUlFree (&hcSeg);
	return np;
}

BOOL LoadDepthTriangles (HDIB32 hDib32,LPSHORT pnTriangles,LPBYTE pTriangles)
{
	int y=CurView->ScreenRect.top;
	int	right = CurView->ScreenRect.right;
	int	bottom = CurView->ScreenRect.bottom;
	int	bpp = FreeImage_GetBPP (hDib32);
	int	height = FreeImage_GetHeight (hDib32);
	int	width = FreeImage_GetWidth (hDib32);
	int	iLoc=0;

	*pnTriangles = 0;
	switch (bpp)
	{
	case 24:

		while (y < bottom)
		{
			int	x=CurView->ScreenRect.left;
			RGBTRIPLE * p24Bit = (RGBTRIPLE	*)FreeImage_GetScanLine (hDib32,height-y-1);
			int	nrep=0,ic,lc=-1;
			LPBYTE pRepeat;

			while (x < right)
			{
	//			COLORREF Color = SetPixel (CurView->hDC,x++,y,RGB(255,255,255));
	//			COLORREF Color = GetPixel (CurView->hDC,x++,y);
				COLORREF Color = RGBTRIPLEToCOLORREF (p24Bit[x]);
				if (Color != (COLORREF)-1 && Color != CurView->BackGroundColor)
					ic=Color;
				else
					ic = 0;
				if (!x || (ic != lc))
				{
					pRepeat=&pTriangles[iLoc++];
					*pRepeat = 0;
					pTriangles[iLoc++] = ic;
					lc = ic;
				}
				else 
					(*pRepeat)++;
				x++;
			}
			y++;
		}
		*pnTriangles = iLoc;
		return TRUE;

	case 32:
		while (y <= bottom)
		{
			int	x=CurView->ScreenRect.left;
			RGBQUAD * p32Bit = (RGBQUAD	*)FreeImage_GetScanLine (hDib32,height-y);

			while (x <= right)
			{
	//			COLORREF Color = SetPixel (CurView->hDC,x++,y,RGB(255,255,255));
	//			COLORREF Color = GetPixel (CurView->hDC,x++,y);
				COLORREF Color = RGBQUADToCOLORREF (p32Bit[x++]);
				if (Color != (COLORREF)-1 && Color != CurView->BackGroundColor)
					return TRUE;
			}
			y++;
		}
		return FALSE;
	}
	return FALSE;
}

float InterpolateElevation (float x, float y)
{
	int ix = max (0,min (255,x));
	int iy = max (0,min (255,y));
	float xinc = fmod (x,1.0f);
	float yinc = fmod (y,1.0f);
	float r, r1, r2;
	int	v1,v2,v3,v4;
	short * gridPoints = &triPoints[0][0];

	v1 = gridPoints[iy*257+ix];
	v2 = gridPoints[min(256,(iy+1))*257+ix];
	v3 = gridPoints[iy*257+min(256,ix+1)];
	v4 = gridPoints[min(256,(iy+1))*257+min(256,ix+1)];
	r1 = v1 + (v2 - v1) * yinc;
	r2 = v3 + (v4 - v3) * yinc;
	r = r1 + (r2 - r1) * xinc;
	return r/(8*TRIANGLE_DEPTH_MULTIPLIER);
}

void GetContextTilt (int x,int y,int z,double az,LPBYTE pTilt1,LPBYTE pTilt2)
{
	*pTilt2 = 0;
	if (!haveTiltFile)
	{
		*pTilt1 = 128;
	}
	else
	{
		float e1, e2;
		DPOINT p={x,y}, p1, p2;
		double charHeight = 3;// * currentTileMetersPerPixel;
		double angle;

		az += HALFPI;
		az = LTWOPI (az);
		p1 = dnewpt (p,az,charHeight);
		p2 = dnewpt (p,az,-charHeight);
		e1 = InterpolateElevation (p1.x,p1.y);
		e2 = InterpolateElevation (p2.x,p2.y);
		if (e1 > z && e2 > z)
			*pTilt2 = min (255,((e1 + e2)/2 - z) * 32);
		angle = atan2 (e1-e2,2*charHeight*currentTileMetersPerPixel) * RADtoDEG;
		if (angle < -90 || angle > 90)
			ii=1;
		*pTilt1 = max (0,min(255,IDNINT (angle + 128))); 
	}
	return;
}

BOOL LoadTiltFile (LPSTR TINPath)
{
	HFILE FidTr = GSSiOpenFile (TINPath,0,OF_READ);
	int	minElev, irow, icol;

	haveTiltFile = FALSE;
	if (FidTr != HFILE_ERROR)
	{
		int ln = GSSifilelength (FidTr);
		HANDLE	hTr = GSSiGlobAlloc(GAIDNO 2065,GMEM_MOVEABLE,ln);
		LPBYTE pTile = GlobalLock (hTr);

		BigRead (FidTr,pTile,ln);
		GSSiClose2 (&FidTr);

		minElev = *(int *)pTile;
		pTile += 4;
		if (minElev == -99999)
		{
			memcpy (triPoints,pTile,ln-4);
		}
		else
		{
			memcpy (triPoints8,pTile,ln-4);
			for (irow = 0;irow < 257; irow++)
				for (icol = 0;icol < 257;icol++)
				{
					if (triPoints8[irow][icol] < 255)
						triPoints[irow][icol] = triPoints8[irow][icol] + (short)minElev;
					else // 255 signifies gap
						triPoints[irow][icol] = -1;
				}
		}
		haveTiltFile = TRUE;
		GSSiGlobUlFree (&hTr);
	}
	return haveTiltFile;
}

void WriteContextPoint (LPSAVECONTEXT2	pSaveC)
{
	char	iCharVal;
	BYTE tilt1, tilt2;
	int	x=pSaveC->x, y=pSaveC->y;
	int	pixelX,pixelY,tileX,tileY;

	LatLongToPixelXY(pSaveC->latitude,pSaveC->longitude,currentZoom,&pixelX,&pixelY);
	PixelXYToTileXY (pixelX,pixelY,&tileX,&tileY);
	if (tileX != currentCol || tileY != currentRow)
		return;
	x = pixelX % 256;
	y = pixelY % 256;
	if (x < 0 || x > 255 ||
		y < 0 || y > 255)
		return;
	if (pSaveC->depth > 127)
	{
		int nRep = pSaveC->depth / 128;

		iCharVal = -nRep;
		BigWrite (FidSTG,&iCharVal,1,-1);
		iCharVal = pSaveC->depth % 128;
	}
	else
		iCharVal = pSaveC->depth;
	BigWrite (FidSTG,&iCharVal,1,-1);
	iCharVal = pSaveC->rot;
	BigWrite (FidSTG,&iCharVal,1,-1);
	iCharVal = x;
	BigWrite (FidSTG,&iCharVal,1,-1);
	iCharVal = y;
	BigWrite (FidSTG,&iCharVal,1,-1);
	GetContextTilt (x,y,pSaveC->depth,pSaveC->az,&tilt1,&tilt2);
	BigWrite (FidSTG,&tilt1,1,-1);
	BigWrite (FidSTG,&tilt2,1,-1);
	return;
}

BOOL TileGraphicsClose (LPSTR ConTextFileID,LPSTR TINPath)
{
	BOOL rtn=FALSE;
	int		i,len,lcmp;
	unsigned short	iSegmentOffset=0;
	unsigned short	nSegments=0;
	HANDLE	hMem,hMemCmp;
	LPSTR	pMem,pMemCmp;
	char	pathName[MAX_PATH];
	HANDLE	hRecDepths=0, hDepthNum=0, hNextID=0, hDepthID=0, hDepthInc=0;
	int		numDepths;
	LPINT	pDepths, pDepthNum, pNextID, pDepthID;
	LPDOUBLE pDepthInc;


	if (FidSTG != HFILE_ERROR)
	{
		tileGraphicsHeader.iConTextOffset = GSSillseek (FidSTG,0,1) - sizeof(tileGraphicsHeader);
		if (*ConTextFileID)
		{
			HANDLE	hDBSaveContourText;

			if ((hDBSaveContourText = GetOpenHandleFromID (ConTextFileID,UMIFS_DATAFILE)))
			{
				LPGWDHEADER	lpGWDHead = (LPGWDHEADER)GlobalLock (hDBSaveContourText);
				LPSAVECONTEXT2	pSaveC = (LPSAVECONTEXT2)&lpGWDHead->GWDData; 
				int	offset, curloc;
				SAVECONTTEXT2KEY key;
				char	quadKey[22];
				int	iPass=0, nRecs=0, nPass;
				int	maxNumDepths=0, id;
				
				LoadTiltFile (TINPath);
				TileXYToQuadKey(currentCol,currentRow,currentZoom,quadKey,sizeof(quadKey));
				
				if (currentZoom == maxZoom)
					nPass = 1;
				else
					nPass = 3;
				while (iPass++ < nPass)
				{
					int	pos = BT_FIRST, cond = BT_GT;

					memcpy (key.QuadKey,quadKey,sizeof(key.QuadKey));
					key.Seq = -1;

					while (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&key,pos,cond,(LPSTR)&offset))
					{
						pos = BT_NEXT;
						cond = BT_ANY;
						if (strnicmp (key.QuadKey,quadKey,currentZoom))
							break;
						if (FillGWDData (lpGWDHead,offset) > 0)
						{
							if (currentZoom == maxZoom)
							{
								curloc = GSSillseek (FidSTG,0,1);
								WriteContextPoint (pSaveC);
							}
							else switch (iPass)
							{
								case 1:
									nRecs++;
									break;
								case 2:
									{
										for (id=0;id<numDepths;id++)
										{
											if (pDepths[id]==pSaveC->depth)
											{
												pDepthNum[id]++;
												goto FoundDepth;
											}
										}
										id = numDepths;
										pDepths[numDepths] = pSaveC->depth;
										pDepthNum[numDepths++] = 1;
FoundDepth:
										maxNumDepths = max (maxNumDepths,pDepthNum[id]);
									}
									break;
								case 3:
									{
										for (id=0;id<numDepths;id++)
										{
											if (pDepths[id]==pSaveC->depth)
											{
												pDepthNum[id]++;
												if (pDepthNum[id] == pNextID[id])
												{
													WriteContextPoint (pSaveC);
													pDepthID[id]++;
													pNextID[id] = IDNINT (pDepthID[id] * pDepthInc[id]);
												}
											}
										}
									}
									break;
							}
						}
					}
					if (currentZoom < maxZoom)
					switch (iPass)
					{
					case 1:
						hRecDepths = GSSiGlobAlloc(GAIDNO 1774,GMEM_MOVEABLE,nRecs*sizeof(int) + 4);
						hDepthNum = GSSiGlobAlloc(GAIDNO 1775,GHND,nRecs*sizeof(int) + 4);
						hNextID = GSSiGlobAlloc(GAIDNO 1776,GHND,nRecs*sizeof(int) + 4);
						hDepthID = GSSiGlobAlloc(GAIDNO 1776,GHND,nRecs*sizeof(int) + 4);
						hDepthInc = GSSiGlobAlloc(GAIDNO 1776,GHND,nRecs*sizeof(double) + 4);
						pDepths = GlobalLock (hRecDepths);
						pDepthNum = GlobalLock (hDepthNum);
						pNextID = GlobalLock (hNextID);
						pDepthID = GlobalLock (hDepthID);
						pDepthInc = GlobalLock (hDepthInc);
						numDepths = 0;
						break;
					case 2:
						{
							int ifactor = (maxZoom - currentZoom)*4;

							//maxNumDepths *= ifactor;
							for (id=0;id<numDepths;id++)
							{
								pDepthInc[id] = (double)pDepthNum[id]/ifactor;
								pNextID[id] = IDNINT (pDepthInc[id]);
								pDepthNum[id] = 0;
								pDepthID[id] = 1;
							}
						}
						break;
					default:
						break;
					}

				}
				GSSiGlobUlFree (&hRecDepths);
				GSSiGlobUlFree (&hDepthNum);
				GSSiGlobUlFree (&hNextID);
				GSSiGlobUlFree (&hDepthID);
				GSSiGlobUlFree (&hDepthInc);
				GlobalUnlock (hDBSaveContourText);
			}
		}
		tileGraphicsHeader.iSegmentOffset = GSSillseek (FidSTG,0,1) - sizeof(tileGraphicsHeader);
		tileGraphicsHeader.totLinePoints = 0;
		for (i=0;i<tgNumSegs;i++)
		{
			short	nPnts = ConvertTGSeg (tgNumSegPoints[i],&tghSegs[i]);
			short	len2;
			LPBPOINT pSeg = GlobalLock (tghSegs[i]);

			tileGraphicsHeader.totLinePoints += abs (nPnts);
			if (nPnts < 0)
				len = (abs(nPnts))*sizeof(CPOINT);
			else
				len = nPnts * sizeof(BPOINT);
			if (len > SHRT_MAX || tgNumSegPoints[i] > SHRT_MAX)
				MessageBox (0,"Segment size exceeds limit",0,MB_ICONEXCLAMATION);
			len2 = len;
			if (nPnts < 0)
				nPnts = -tgNumSegPoints[i];
			BigWrite (FidSTG,(LPSTR)&len2,sizeof(short),-1);
			BigWrite (FidSTG,(LPSTR)&nPnts,sizeof(short),-1);
			BigWrite (FidSTG,(LPSTR)pSeg,len,-1);
			/*if (nPnts < 0)
			{
				HANDLE	hPnt2=0;
				short nPnts2 = ReplaceWithPatterns (abs(nPnts),(LPCPOINT)pSeg,&hPnt2);

				BigWrite (FidSTG,(LPSTR)pSeg,nPnts2*sizeof(CPOINT),-1);
				GSSiGlobFree (&hPnt2);
			}
			else
				BigWrite (FidSTG,(LPSTR)pSeg,len,-1);*/

/*				while (tgNumSegPoints[i] > 0)
			{
				BYTE	nSeg8;
				if (tgNumSegPoints[i] > 127)
					nSeg8 = -127;
				else
					nSeg8 = tgNumSegPoints[i];
				BigWrite (FidSTG,(LPSTR)&nSeg8,sizeof(BYTE),-1);
				BigWrite (FidSTG,(LPSTR)pSeg,abs(nSeg8),-1);
				tgNumSegPoints[i] -= 127;
			}*/
			GSSiGlobUlFree (&tghSegs[i]);
		}
		tileGraphicsHeader.nSegments = tgNumSegs;
		len = GSSillseek (FidSTG,0,1);
		tileGraphicsHeader.iTriangleOffset = len - sizeof(tileGraphicsHeader);
		if (*TINPath)
		{
			LPSTR pDot = strrchr (TINPath,'.');

			if (pDot && !stricmp (pDot,".bin"))
			{
				HFILE FidTr = GSSiOpenFile (TINPath,0,OF_READ);

				if (FidTr != HFILE_ERROR)
				{
					int ln = GSSifilelength (FidTr);
					HANDLE	hTr = GSSiGlobAlloc(GAIDNO 2066,GMEM_MOVEABLE,ln);
					LPSTR	pTr = GlobalLock (hTr);

					BigRead (FidTr,pTr,ln);
					BigWrite (FidSTG,pTr,ln,-1);
					GSSiClose2 (&FidTr);
					GSSiGlobUlFree (&hTr);
					tileGraphicsHeader.triPointsLen = ln;
				}
			}
			else
			{
				HDIB32 hDIB = BMPHandleFromEXT (TINPath); 

				if (hDIB)
				{
					HANDLE hTriangles = GSSiGlobAlloc(GAIDNO 1772,GMEM_MOVEABLE,256*256*2);
					LPBYTE pTriangles = GlobalLock (hTriangles);
					LoadDepthTriangles (hDIB,&tileGraphicsHeader.nTriangles,pTriangles);
					BigWrite (FidSTG,(LPSTR)pTriangles,tileGraphicsHeader.nTriangles,-1);
					GSSiGlobUlFree (&hTriangles);
					GMDestroyDIB32 (hDIB);
				}
			}
		}
		len = GSSillseek (FidSTG,0,1);
		tileGraphicsHeader.uncompressedLength = len;
		GSSillseek (FidSTG,0,0);
		BigWrite (FidSTG,(LPSTR)&tileGraphicsHeader,sizeof(tileGraphicsHeader),-1);
		GSSillseek (FidSTG,0,0);
		hMem = GSSiGlobAlloc(GAIDNO 206,GMEM_MOVEABLE,len);
		pMem = GlobalLock (hMem);
		hMemCmp = GSSiGlobAlloc(GAIDNO 2068,GMEM_MOVEABLE,len+1024);
		pMemCmp = GlobalLock (hMemCmp);
		BigRead (FidSTG,pMem,len);
		lcmp = CompressBinaryRecord ((LPBYTE)pMem,pMemCmp,len);
		GetOpenFilePathname (FidSTG,pathName);
		GSSiClose2 (&FidSTG);
		FidSTG = GSSiOpenFile (pathName,0,OF_CREATE);
		BigWrite (FidSTG,(LPSTR)&lcmp,4,-1);
		BigWrite (FidSTG,(LPSTR)&len,4,-1);
		BigWrite (FidSTG,pMemCmp,lcmp,-1);
		GSSiGlobUlFree (&hMem);
		GSSiGlobUlFree (&hMemCmp);
		GSSiClose2 (&FidSTG);
		FidSTG = HFILE_ERROR;
		tgNumSegs = 0;
		rtn = TRUE;
	}
	InitTileGraphics (HFILE_ERROR);
	return rtn;
}

BOOL TileGraphicsOpen (LPSTR Directory,int zoom,int irow,int icol)
{
	char pathName[MAX_PATH];
	BOOL	rtn=FALSE;
	int		i,len;
	unsigned short lcmp;
	unsigned short	iSegmentOffset=0;
	unsigned short	nSegments=0;
	HANDLE	hMem,hMemCmp;
	LPSTR	pMem,pMemCmp;
	int		worldX, worldY;
	double	latitude, longitude;

	currentZoom = zoom;
	currentRow = irow;
	currentCol = icol;
	TileXYToPixelXY(icol,irow,&worldX,&worldY);
	PixelXYToLatLong(worldX, worldY,currentZoom,&latitude, &longitude);
	currentTileMetersPerPixel = GroundResolution(latitude, currentZoom);
	{
		sprintf (pathName,"%s\\%i\\%i\\%i.bin",Directory,zoom,irow,icol);
		FidSTG = GSSiOpenFile (pathName,0,OF_CREATE);
		if (FidSTG != HFILE_ERROR)
		{
			memset (&tileGraphicsHeader,0,sizeof(tileGraphicsHeader));
			BigWrite (FidSTG,(LPSTR)&tileGraphicsHeader,sizeof(tileGraphicsHeader),-1);
			rtn = TRUE;
		}
	}
	InitTileGraphics (FidSTG);
	tgNumSegs = 0;
	return rtn;
}

BOOL PtInVPRect (POINT pt)
{
	if (pt.x >= CurView->ScreenRect.left && pt.x <= CurView->ScreenRect.right &&
		pt.y >= CurView->ScreenRect.top && pt.y <= CurView->ScreenRect.bottom)
		return TRUE;
	return FALSE;
}

int TGCompareSegments (int np,LPBPOINT pPoints)
{
	int rtn=0;
	int	i,j,iseg;
	LPBPOINT pSeg;

	for (iseg=0; iseg < tgNumSegs; iseg++)
	{
		if (!tghSegs[iseg])
			continue;
		pSeg = GlobalLock (tghSegs[iseg]);
		if (np == tgNumSegPoints[iseg])
		{
			for (i=0;i<np;i++)
				if (pPoints[i].x != pSeg[i].x || pPoints[i].y != pSeg[i].y)
					goto CheckReverse;
			GlobalUnlock (tghSegs[iseg]);
			return iseg+1;

CheckReverse:
			for (i=0,j=np-1;i<np;i++,j--)
				if (pPoints[i].x != pSeg[j].x || pPoints[i].y != pSeg[j].y)
					goto Next;
			rtn = -(iseg+1);
		}
Next:
		GlobalUnlock (tghSegs[iseg]);
		if (rtn)
			break;
	}
	return rtn;
}

BOOL CreateContourPatternFile (BOOL Close)
{
	char	DefStr[128]="NPoints(B2),Pattern(C8),Count(B4)";
	BOOL	rtn;

	if (Close)
	{
		CloseGWDatabase (hCPDB);
		return TRUE;
	}

	rtn = CreateGWDDatabase ("c:\\temp\\ContourPatterns.gmd",1,FALSE,0,2,DefStr);  
	hCPDB = OpenGWDatabase ("c:\\temp\\ContourPatterns.gmd",BT_WRITE);
	return rtn;
}

void AddPatternToCPDB (int nPnts,LPPOINTS pPoints)
{
	LPGWDHEADER lpGWDHead;
	int	Offset, len, i;

	CPKEY	CPKey;
	LPCPRECORD	pCPRecord;
	LPBYTE		pByte;

	if (!hCPDB)
		return;

	
	CPKey.nPnts = nPnts;
	memset (&CPKey.Points[0],0,sizeof(CPKey.Points));
	for (i=0;i<nPnts;i++)
	{
		if (abs (pPoints[i].x) > 15 || abs (pPoints[i].y) > 15)
			return;
		CPKey.Points[i].x = pPoints[i].x;
		CPKey.Points[i].y = pPoints[i].y;
	}

	lpGWDHead = (LPGWDHEADER)GlobalLock (hCPDB); 
	pCPRecord = (LPCPRECORD)&lpGWDHead->GWDData; 
	pByte = (LPBYTE)&CPKey.Points[0];
	for (i=0;i<sizeof(CPKey.Points);i++,pByte++)
		if (*pByte == 0)
			*pByte = 0x7f;
    if (!BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&CPKey,BT_FIRST,BT_EQ,(LPSTR)&Offset))
    {   
    	len = FillGWDData (lpGWDHead,Offset); 
		pCPRecord->count--;
		GWDReplaceRecord (lpGWDHead,sizeof(CPRECORD),0,-1);
    }
	else
	{
		pCPRecord->key = CPKey;
		pCPRecord->count = -1;
		GWDReplaceRecord (lpGWDHead,sizeof(CPRECORD),0,-1);
	}
	GlobalUnlock (hCPDB);
	return;
} 

void AddSegToContourPatterns (int np, LPBPOINT Points)
{
	POINTS pattern[MAXPATTERN];
	int	npat, i, j, n;

	if (!hCPDB)
		return;

	for (i=0;i<np-3;i++)
	{
		BPOINT bp = Points[i];

		for (j=0,n=i+1;j<MAXPATTERN;j++,n++)
		{
			if (n < np)
			{
				pattern[j].x = Points[n].x - Points[n-1].x;
				pattern[j].y = Points[n].y - Points[n-1].y;
			}
			else
				break;
		}
		for (npat=2;npat<j+1;npat++)
		{
			AddPatternToCPDB (npat,pattern);
		}
	}
}

int TGAddSegment (int np,LPBPOINT pPoints,LPINT prePend,LPINT pnprePend)
{
	int	iseg, iCompare, ipe;
	LPBPOINT	pSeg;
BPOINT ipt=pPoints[np-1];

	for (ipe=0;ipe<(*pnprePend);ipe++)
	{
		if (prePend[ipe] == tgNumSegs &&
			pPoints[0].x == pPoints[np-1].x && 
			pPoints[0].y == pPoints[np-1].y)
		{
			prePend[ipe] = -1;
			if (ipe == (*pnprePend)-1)
				(*pnprePend)--;
		}
		else if (prePend[ipe] >= 0 && prePend[ipe] < tgNumSegs)
		{
			pSeg = (LPBPOINT)GlobalLock (tghSegs[prePend[ipe]]);
			if (pPoints[np-1].x == pSeg[0].x && pPoints[np-1].y == pSeg[0].y)
			{
				HANDLE hNewSeg = GSSiGlobAlloc(GAIDNO 1765,GMEM_MOVEABLE,(np+tgNumSegPoints[prePend[ipe]])*sizeof(BPOINT));
				LPBPOINT pNewSeg = (LPBPOINT)GlobalLock (hNewSeg);

ipt=pSeg[tgNumSegPoints[prePend[ipe]]-1];
				memcpy (pNewSeg,pPoints,np*sizeof(BPOINT));
				memcpy (&pNewSeg[np],&pSeg[1],(tgNumSegPoints[prePend[ipe]]-1)*sizeof(BPOINT));
				GlobalUnlock (tghSegs[prePend[ipe]]);
				np += tgNumSegPoints[prePend[ipe]]-1;
	ipt=pNewSeg[np-1];
				iCompare = TGCompareSegments (np,pNewSeg);
	/*			if (iCompare < 0)
				{
					GSSiGlobUlFree (&hNewSeg);
					*prePend = -1;
					return iCompare - TGMAXSEGS;
				}
				else if (iCompare > 0)
				{
					GSSiGlobUlFree (&hNewSeg);
					*prePend = -1;
					return iCompare + TGMAXSEGS;
				}*/ //not working - could reduce db size if it did
				GlobalUnlock (hNewSeg);
				GSSiGlobFree (&tghSegs[prePend[ipe]]);
				tghSegs[prePend[ipe]] = hNewSeg;
				tgNumSegPoints[prePend[ipe]] = np;
				prePend[ipe] = -1;
				if (ipe == (*pnprePend)-1)
					(*pnprePend)--;
				return TGMAXSEGS;
			}
			GlobalUnlock (tghSegs[prePend[ipe]]);
		}
	}
	iCompare = TGCompareSegments (np,pPoints);
	if (iCompare != 0)
		return iCompare;
	AddSegToContourPatterns (np,pPoints);
	if (tgNumSegs >= TGMAXSEGS)
		BlowOut ("Max TileGraphics segments exceeded","ERROR");
	tghSegs[tgNumSegs] = GSSiGlobAlloc(GAIDNO 1765,GMEM_MOVEABLE,np*sizeof(BPOINT));
	tgNumSegPoints[tgNumSegs] = np;
	pSeg = (LPBPOINT)GlobalLock (tghSegs[tgNumSegs]);
	memmove (pSeg,pPoints,np*sizeof(BPOINT));
	GlobalUnlock (tghSegs[tgNumSegs++]);//pPoints[0] pPoints[1] pPoints[2] pPoints[3] pPoints[4] pPoints[8]
ii=np-1;
ipt = pPoints[ii];
	return tgNumSegs;
}

void SaveTileGraphics (HFILE Fid,HDC hDC,int type,LPPOINT points,int np)
{
#define MAXPOLYSEGS	4096
	HANDLE hInPt = GSSiGlobAlloc(GAIDNO 2069,GMEM_MOVEABLE,(np+4) * sizeof(BPOINT));
	LPBPOINT pInPt = GlobalLock (hInPt);
	static	BPOINT	completeTile[4]={0,255,0,0,255,0,255,255};
	int		nInPt=0, i;
	BOOL	thisIn, lastIn;
	short	nPolySeg=0;
	BYTE	nPolySeg8;
	short	polySeg[MAXPOLYSEGS];
	char	polySeg8;
	int		maxSegNum=0, prePend[64]={-1}, nprePend=0;
	TGRECHEADER	tgHead;
	BOOL	inExclusionIsland=FALSE;
	BOOL	islandCompletelyIn=TRUE;

	if (type != TGPOLYLINE)
	{
		GSSiGlobUlFree (&hInPt);
		return;
	}
	tgHead.type = type;
	if (type > 0)
		ii=1;
	ii = (int)tghSegs[8];
	tgHead.refno = CurrentRefno;
	tgHead.symbol = CurrentDesc;
	tgHead.completeTile = 0;
	lastIn = PtInVPRect (points[0]);
	if (lastIn)
	{
		pInPt[nInPt++] = POINTtoBPOINT (points[0]);
		prePend[nprePend++] = tgNumSegs;
	}
	for (i=1;i<np;i++)
	{
		BOOL isLinkLine = FALSE;

if (points[i].x == 734 && points[i].y == 352)
	ii=1;
		if (nPolyPartStart)
		{
			int ipps;

			if (i == pPolyPartStart[0]+1)
			{
				isLinkLine = TRUE;
				if (PtInVPRect (points[i]))
					prePend[nprePend++] = tgNumSegs;
			}
			else for (ipps=0;ipps<nPolyPartStart;ipps++)
			{
				if (i == pPolyPartStart[ipps])
				{
					thisIn = PtInVPRect (points[i]);//points[i-1] points[i+1]
					if (ipps)
						isLinkLine = TRUE;
					else
					{
						if (thisIn && lastIn)
						{
							pInPt[nInPt] = POINTtoBPOINT (points[i]);
							if (!nInPt || (pInPt[nInPt].x != pInPt[nInPt-1].x || pInPt[nInPt].y != pInPt[nInPt-1].y))
								nInPt++;
						}
					}
					if (nInPt > 1)
					{
						polySeg[nPolySeg] = TGAddSegment (nInPt,pInPt,prePend,&nprePend);
						if (abs(polySeg[nPolySeg]) < MAXPOLYSEGS)
						{
							if (inExclusionIsland && islandCompletelyIn)
							{
								if (polySeg[nPolySeg] < 0)
									polySeg[nPolySeg] -= MAXPOLYSEGS;
								else
									polySeg[nPolySeg] += MAXPOLYSEGS;
							}
							nPolySeg++;
						}
					}
					nInPt = 0;
					inExclusionIsland = TRUE;
					if (inExclusionIsland)
						islandCompletelyIn = TRUE;
/*					else if (!islandCompletelyIn)
					{
						i++;
						goto NextPoint;
					}
					thisIn = PtInVPRect (points[i]);
					if (!inExclusionIsland && !thisIn)
					{
						i++;
						goto NextPoint;
					}
					else if (inExclusionIsland && !PtInVPRect (points[i+1]))
					{//if end of linkline not in grid skip linkline
						i++;
						thisIn = FALSE;
						goto NextPoint;
					}
					else if (inExclusionIsland && !PtInVPRect (points[i]))
					{//this is the begin of a linkline - adjust to first polypoint in grid
						thisIn = TRUE;
						if (!nInPt)
							pInPt[nInPt++] = completeTile[0]; 
						else
							pInPt[nInPt++] = pInPt[0];
						goto NextPoint;
					}*/
					goto NextPoint;
				}
				else if (i == pPolyPartStart[ipps]+1)
				{
					isLinkLine = TRUE;
					if (PtInVPRect (points[i]))
						prePend[nprePend++] = tgNumSegs;
					break;
				}
			}
		}
		thisIn = PtInVPRect (points[i]);

		if (!thisIn && inExclusionIsland)
			islandCompletelyIn = FALSE;

		if (thisIn)
		{
			if (lastIn)
			{
				pInPt[nInPt] = POINTtoBPOINT (points[i]);
				if (!nInPt || (pInPt[nInPt].x != pInPt[nInPt-1].x || pInPt[nInPt].y != pInPt[nInPt-1].y))
					nInPt++;
			}
			else
			{
				if (!isLinkLine)
				{
					pInPt[nInPt] = POINTtoBPOINT (GetIntersectWithVP (points[i-1],points[i],0));
					if (!nInPt || (pInPt[nInPt].x != pInPt[nInPt-1].x || pInPt[nInPt].y != pInPt[nInPt-1].y))
						nInPt++;
				}
				pInPt[nInPt] = POINTtoBPOINT (points[i]);
				if (!nInPt || (pInPt[nInPt].x != pInPt[nInPt-1].x || pInPt[nInPt].y != pInPt[nInPt-1].y))
					nInPt++;
			}
		}
		else if (lastIn)
		{
			if (!isLinkLine)
			{
				pInPt[nInPt] = POINTtoBPOINT (GetIntersectWithVP (points[i-1],points[i],0)); //pInPt[nInPt-1]
				if (!nInPt || (pInPt[nInPt].x != pInPt[nInPt-1].x || pInPt[nInPt].y != pInPt[nInPt-1].y))
					nInPt++;
			}
			if (nInPt > 1)
			{
				polySeg[nPolySeg] = TGAddSegment (nInPt,pInPt,prePend,&nprePend);
				if (abs(polySeg[nPolySeg]) < MAXPOLYSEGS)
					nPolySeg++;
				else
					polySeg[nPolySeg] = (polySeg[nPolySeg]<0) ? polySeg[nPolySeg] + MAXPOLYSEGS : polySeg[nPolySeg] - MAXPOLYSEGS;
			}

			nInPt = 0;
		}
		else if (!isLinkLine)
		{
			RECT	rect, outRect;
			RectInit (&rect);
			AddPointToRect (points[i-1],&rect);
			AddPointToRect (points[i],&rect);
			if (IntersectRect (&outRect,&rect,&CurView->ScreenRect))
			{
				POINT intPt1, intPt2;
				double	d1, d2;

				intPt1 = GetIntersectWithVP (points[i-1],points[i],0);
				intPt2 = GetIntersectWithVP (points[i-1],points[i],&intPt1);
				if (PtInVPRect (intPt1) && PtInVPRect (intPt2))
				{
					d1 = distp (points[i-1],intPt1);
					d2 = distp (points[i-1],intPt2);
					if (d1 < d2)
					{
						pInPt[0] = POINTtoBPOINT (intPt1);
						pInPt[1] = POINTtoBPOINT (intPt2);
					}
					else
					{
						pInPt[0] = POINTtoBPOINT (intPt2);
						pInPt[1] = POINTtoBPOINT (intPt1);
					}
					polySeg[nPolySeg] = TGAddSegment (2,pInPt,prePend,&nprePend);
					if (abs(polySeg[nPolySeg]) < MAXPOLYSEGS)
						nPolySeg++;
					else
						polySeg[nPolySeg] = (polySeg[nPolySeg]<0) ? polySeg[nPolySeg] + MAXPOLYSEGS : polySeg[nPolySeg] - MAXPOLYSEGS;
					nInPt = 0;
				}
			}
		}
NextPoint:
		lastIn = thisIn;
	}
	if (nInPt == 4 && !memcmp (completeTile,pInPt,sizeof(completeTile)))
	{
		GSSiGlobUlFree (&hInPt);
		tgHead.completeTile = 1;
		tgHead.coordtype = 0;
		tgHead.nsegtype = 0;
		BigWrite (Fid,(LPSTR)&tgHead,sizeof(tgHead),-1);
		return;
	}
	if (nInPt > 1)
	{
		polySeg[nPolySeg] = TGAddSegment (nInPt,pInPt,prePend,&nprePend);
		if (abs(polySeg[nPolySeg]) < MAXPOLYSEGS)
			nPolySeg++;
		else
			polySeg[nPolySeg] = (polySeg[nPolySeg]<0) ? polySeg[nPolySeg] + MAXPOLYSEGS : polySeg[nPolySeg] - MAXPOLYSEGS;
	}
	GSSiGlobUlFree (&hInPt);
	if (nprePend > 0)
		ii=1;
	if (!nPolySeg)
		return;
	if (nPolySeg < 256)
		tgHead.nsegtype = 0;
	else
		tgHead.nsegtype = 1;
	for (i=0;i<nPolySeg;i++)
		maxSegNum = max (maxSegNum,abs(polySeg[i]));
	if (maxSegNum < 128)
		tgHead.coordtype = 0;
	else
		tgHead.coordtype = 1;
	tileGraphicsHeader.nLines += nPolySeg;
	for (i=0;i<nPolySeg;i++)
		tileGraphicsHeader.totLinePoints += tgNumSegPoints[abs(polySeg[i])-1];
	BigWrite (Fid,(LPSTR)&tgHead,sizeof(tgHead),-1);
	if (nPolySeg < 256)
	{
		nPolySeg8 = nPolySeg;
		BigWrite (Fid,(LPSTR)&nPolySeg8,sizeof(nPolySeg8),-1);
	}
	else
		BigWrite (Fid,(LPSTR)&nPolySeg,sizeof(nPolySeg),-1);
	if (maxSegNum < 128)
	{
		for (i=0;i<nPolySeg;i++)
		{
			polySeg8 = polySeg[i];
			BigWrite (Fid,(LPSTR)&polySeg8,sizeof(polySeg8),-1);
		}
	}
	else
	{
		for (i=0;i<nPolySeg;i++)
		{
			BigWrite (Fid,(LPSTR)&polySeg[i],sizeof(short),-1);
		}
	}

	return;
}

int GetTileSide (POINT pt)
{
	int	side = 0;

	if (pt.x == 0)
		side = 1;
	else if (pt.x == 255)
		side = 3;
	else if (pt.y == 0)
		side = 2;
	else if (pt.y == 255)
		side = 4;
	return side;
}

int AddPolygonCornerPoints (LPPOINT pPoly,int np,POINT nextPt,POINT firstPt)
{
	int	side1=0, side2=0;
	static POINT corner[4]={0,0,255,0,255,255,0,255};

	POINT pt = pPoly[np-1];

	if (pt.x == firstPt.x && pt.y == firstPt.y)
		return -np;
	if (!(side1 = GetTileSide (pt)))
		return np;
	if (clockwiseDist (pt,firstPt) < clockwiseDist (pt,nextPt))
		nextPt = firstPt;
	if (!(side2 = GetTileSide (nextPt)))
		return np;

	while (side1 != side2)
	{
		int	i = side1-1;

		if ((pt.x != corner[i].x || pt.y != corner[i].y) &&
			(nextPt.x != corner[i].x || nextPt.y != corner[i].y))
			pPoly[np++] = corner[i];
		side1++;
		if (side1 == 5)
			side1 = 1;
	}
	if (nextPt.x == firstPt.x && nextPt.y == firstPt.y)
	{
		pPoly[np++] = firstPt;
		return -np;
	}
	return np;
}

int clockwiseDist (POINT fromPt,POINT toPt)
{
	int	dist=0;
	int	side1=0, side2=0;
	static POINT corner[4]={0,0,255,0,255,255,0,255};


	if (!(side1 = GetTileSide (fromPt)))
		return SHRT_MAX;
	if (!(side2 = GetTileSide (toPt)))
		return SHRT_MAX;

	if (side1 == side2)
	{
		switch (side1)
		{
		case 1:
			if (fromPt.y >= toPt.y)
				return fromPt.y - toPt.y;
			else
				return (fromPt.y + 255 * 3 + 255 - toPt.y);
		case 2:
			if (fromPt.x <= toPt.x)
				return toPt.x - fromPt.x;
			else
				return (255 - fromPt.x + 255 * 3 + toPt.x);
		case 3:
			if (fromPt.y <= toPt.y)
				return toPt.y - fromPt.y;
			else
				return (255 - fromPt.y + 255 * 3 + toPt.y);
		case 4:
			if (fromPt.x >= toPt.x)
				return fromPt.x - toPt.x;
			else
				return (fromPt.x + 255 * 3 + 255 - toPt.x);
		}

	}
	else
	{
		switch (side1)
		{
		case 1:
			dist = fromPt.y;
			break;
		case 2:
			dist = 255 - fromPt.x;
			break;
		case 3:
			dist = 255 - fromPt.y;
			break;
		case 4:
			dist = fromPt.x;
			break;
		}
		side1++;
		if (side1 == 5)
			side1 = 1;
		while (side1 != side2)
		{
			dist += 255;
			side1++;
			if (side1 == 5)
				side1 = 1;
		}
		switch (side1)
		{
		case 1:
			dist += 255 - toPt.y;
			break;
		case 2:
			dist += toPt.x;
			break;
		case 3:
			dist += toPt.y;
			break;
		case 4:
			dist += 255 - toPt.x;
			break;
		}
	}

	return dist;
}

void ReorderPolySegs (int nSegs,LPSHORT pSegs,LPSHORT pSegIsExclusion,LPPOINT *pPolySegs,LPINT	pnPolySegPnts)
{
	int i, mini, minDist, dist, nNewSegs;
	LPSHORT	pSegsNew, pSegIsExclusionNew;
	POINT	fromPt;
	POINT	toPt;

	if (nSegs < 2)
		return;
	pSegsNew = malloc (nSegs * sizeof(short));
	pSegIsExclusionNew = malloc (nSegs * sizeof(short));
	pSegsNew[0] = pSegs[0];
	pSegIsExclusionNew[0] = pSegIsExclusion[0];
	if (pSegsNew[0] > 0)
		fromPt = pPolySegs[pSegsNew[0]-1][pnPolySegPnts[pSegsNew[0]-1]-1];
	else
		fromPt = pPolySegs[abs(pSegsNew[0])-1][0];
	pSegs[0] = 0;
	nNewSegs = 1;

	while (nNewSegs < nSegs)
	{
		minDist = INT_MAX;
		for (i=1;i<nSegs;i++)
		{
			if (!pSegs[i])
				continue;
			if (pSegs[i] < 0)
				toPt = pPolySegs[abs(pSegs[i])-1][pnPolySegPnts[abs(pSegs[i])-1]-1];
			else
				toPt = pPolySegs[pSegs[i]-1][0];
			if (pSegIsExclusion[i])
				dist = SHRT_MAX;
			else
				dist = clockwiseDist (fromPt,toPt);
			if (dist <  minDist)
			{
				mini = i;
				minDist = dist;
			}
		}
		pSegsNew[nNewSegs] = pSegs[mini];
		pSegIsExclusionNew[nNewSegs] = pSegIsExclusion[mini];
		pSegs[mini] = 0;
		if (pSegsNew[nNewSegs] > 0)
			fromPt = pPolySegs[pSegsNew[nNewSegs]-1][pnPolySegPnts[pSegsNew[nNewSegs]-1]-1];
		else
			fromPt = pPolySegs[abs(pSegsNew[nNewSegs])-1][0];
		nNewSegs++;
	}

	memcpy (pSegs,pSegsNew,nSegs * sizeof(short));
	free (pSegsNew);
	memcpy (pSegIsExclusion,pSegIsExclusionNew,nSegs * sizeof(short));
	free (pSegIsExclusionNew);
	return;
}

BOOL DisplayTileGraphics (HDC hDC,LPSTR Directory,int iZoom,int tilex,int tiley,LPPOINT pCorners)
{
	char	pathName[MAX_PATH];
	HFILE	Fid;
	int		len, loc=0, nSegs, i, j, np, totPnts, iEnd;
	LPBYTE	pTile;
	HANDLE	hTile;
	unsigned short	iSegmentOffset;
	unsigned short	nPolySegs;
	LPTGRECHEADER pRecHead;
	LPSHORT	pSegs;
	LPSHORT pSegIsExclusion;
	HANDLE	hTran;
	double	XFROM[4],YFROM[4],XTO[4],YTO[4];  
	float	RSQMIN;
	int		iPolySeg=0;
	LPPOINT	*pPolySegs, pPoly;
	LPINT	pnPolySegPnts;
	POINT	firstPt;
	POINT completeTilePoly[4]={0,0,255,0,255,255,0,255};
static	int debugref = 658447;
static	BOOL displayonlydebugref = FALSE;
static	BOOL showBorder=FALSE;
POINT ptt,bp,ep;
char	quadKey[22];
//return FALSE;
//	iZoom = 15;tiley=11803;tilex=7849;
//	iZoom = 19;tiley=188872;tilex=125545;
//	iZoom = 18;tiley=94437;tilex=62780;	
//	iZoom = 17;tiley=47219;tilex=31389;	
//	iZoom = 11;tiley=737;tilex=490;
//	iZoom = 15;tiley=11805;tilex=7845;
//	iZoom = 14;tiley=5902;tilex=3923;
	iZoom = 15,tiley=11805,tilex=7847;

	TileXYToQuadKey(tilex, tiley, iZoom,quadKey,sizeof(quadKey));
	QuadKeyToTileXY(quadKey, &tilex,&tiley,&iZoom);
	sprintf (pathName,"%s\\%i\\%i\\%i.bin",Directory,iZoom,tiley,tilex);
	sprintf (pathName,"%s\\%i\\%i\\%i.bin","G:\\MNVector",iZoom,tiley,tilex);
	Fid = GSSiOpenFile (pathName,0,OF_READ);
	if (Fid == HFILE_ERROR)
		return FALSE;
	len = GSSillseek (Fid,0,2);
	GSSillseek (Fid,0,0);
	hTile = GSSiGlobAlloc(GAIDNO 1769,GMEM_MOVEABLE,len);
	pTile = GlobalLock (hTile);
	BigRead (Fid,pTile,len);
	GSSiClose2 (&Fid);

	ShowContourLines=GetGlobalBVal2 ("[%SHOWHBIRDCONTOURLINES]",FALSE);
	ShowDepthColors=GetGlobalBVal2 ("[%SHOWHBIRDDEPTHCOLORS]",TRUE);
	HighlightDepth = GetGlobalLVal2 ("[%HIGHLIGHTDEPTH]",0);
	HighlightDepthRange = GetGlobalLVal2 ("[%HIGHLIGHTDEPTHRANGE]",5);

	iSegmentOffset = *(unsigned short *)&pTile[loc];
	loc += 2;
	nPolySegs = *(unsigned short *)&pTile[loc];
	loc += 2;
	pPolySegs = malloc (nPolySegs * sizeof(LPPOINT));
	pnPolySegPnts = malloc (nPolySegs * sizeof(int));
	XFROM[0] = 0;
	YFROM[0] = 255;
	XTO[0]	 = pCorners[0].x;
	YTO[0]	 = pCorners[0].y;
	XFROM[1] = 0;
	YFROM[1] = 0;
	XTO[1]	 = pCorners[1].x;
	YTO[1]	 = pCorners[1].y;
	XFROM[2] = 255;
	YFROM[2] = 0;
	XTO[2]	 = pCorners[2].x;
	YTO[2]	 = pCorners[2].y;
	XFROM[3] = 255;
	YFROM[3] = 255;
	XTO[3]	 = pCorners[3].x;
	YTO[3]	 = pCorners[3].y;
	hTran = STRAN2 (1770,XFROM,YFROM,XTO,YTO,4,(LPFLOAT)&RSQMIN,1,NULL);
	for (i=0;i<4;i++)
		completeTilePoly[i] = TranPoint16 (completeTilePoly[i],hTran);
	loc = iSegmentOffset;
	while (iPolySeg < nPolySegs)
	{
		int	compressedLength = *(LPSHORT)&pTile[loc];
		LPPOINT	pPoint;

		loc += 2;
		pnPolySegPnts[iPolySeg] = *(LPSHORT)&pTile[loc];
		loc += 2;
		iEnd = loc + compressedLength;
		if (abs(pnPolySegPnts[iPolySeg]) == 0)
		{
			pPolySegs[iPolySeg] = NULL;
			goto NextSeg;
		}
		pPolySegs[iPolySeg] = (LPPOINT)malloc (abs(pnPolySegPnts[iPolySeg])*8*sizeof(POINT));
		if (pPolySegs[iPolySeg] == NULL)
			ii=1;
		pPoint = pPolySegs[iPolySeg];
		if (pnPolySegPnts[iPolySeg] > 0)
		{
			LPBPOINT	pBPoint = (LPBPOINT)&pTile[loc];

			for (i=0;i<pnPolySegPnts[iPolySeg];i++,pBPoint++,loc+=sizeof(BPOINT))
			{
				pPoint[i].x = pBPoint->x;
				pPoint[i].y = pBPoint->y;
				//pPoint[i] = TranPoint16 (pPoint[i],hTran);
			}
		}
		else
		{
			LPCPOINT	pCPoint;
			int			npt=1;
			
			pnPolySegPnts[iPolySeg] = abs (pnPolySegPnts[iPolySeg]);
			pPoint[0].x = ((LPBPOINT)&pTile[loc])->x;
			pPoint[0].y = ((LPBPOINT)&pTile[loc])->y;
			loc += 2;
			//pCPoint->x = 0x8;
			//if (pCPoint->x == -8)
			//	ii=1;
			while (loc < iEnd)
			{
				pCPoint = (LPCPOINT)&pTile[loc];
				if (pCPoint->x == -8)
				{
					if (pCPoint->y == -8)
					{
						loc++;
						pPoint[npt] = BPOINTtoPOINT (*(LPBPOINT)&pTile[loc]);
						npt++;
						loc++;
					}
					else
					{
						int	nPatPnt = GetPatternPoints (8-pCPoint->y,&pPoint[npt-1]);

						npt += nPatPnt;
					}
				}
				else
				{
					pPoint[npt].x = pPoint[npt-1].x + pCPoint->x;
					pPoint[npt++].y = pPoint[npt-1].y + pCPoint->y;
				}
				loc++;
			}
			pnPolySegPnts[iPolySeg] = npt;
			//for (i=0;i<pnPolySegPnts[iPolySeg];i++)
			//	pPoint[i] = TranPoint16 (pPoint[i],hTran);
		}
NextSeg:
		iPolySeg++;
	}

	loc = 4;
	while (loc < iSegmentOffset)
	{
		HPEN	hPen=0, hOldPen=0;
		HBRUSH	hBrush=0, hOldBrush=0;

		pRecHead = (LPTGRECHEADER) &pTile[loc];
		loc += sizeof (TGRECHEADER);
		if (pRecHead->refno == debugref)
			ii=1;
		if (pRecHead->type == TGPOLYGON)
		{
			if (showBorder)
			{
				hPen = CreatePen (PS_SOLID,1,0);
				SelectObject (hDC,hPen);
			}
			else 
				hOldPen = SelectObject (hDC,GetStockObject(NULL_PEN));
			hBrush = GetBrushForSymbol (pRecHead->symbol);
			hOldBrush = SelectObject (hDC,hBrush);
		}
		else
		{
			hPen = GetPenForSymbol (pRecHead->symbol);
			hOldPen = SelectObject (hDC,hPen);
		}
		if (pRecHead->completeTile)
		{
if (!displayonlydebugref || pRecHead->refno == debugref)
			Polygon (hDC,completeTilePoly,4);
			goto NextElement;
		}
		if (pRecHead->nsegtype)
		{
			nSegs = *(LPSHORT)&pTile[loc];
			loc += 2;
		}
		else
			nSegs = *(LPBYTE)&pTile[loc++];
		if (nSegs <= 0)
			ii=1;
		pSegs = malloc (nSegs * sizeof(short));
		pSegIsExclusion = malloc (nSegs * sizeof(short));
		totPnts = 0;
		if (pRecHead->coordtype)
		{
			for (i=0;i<nSegs;i++)
			{
				pSegs[i] = *(LPSHORT)&pTile[loc];
				if (pSegs[i] > TGMAXSEGS)
				{
					pSegs[i] -= TGMAXSEGS;
					pSegIsExclusion[i] = 1;
				}
				else if (pSegs[i] < -TGMAXSEGS)
				{
					pSegs[i] += TGMAXSEGS;
					pSegIsExclusion[i] = 1;
				}
				else
					pSegIsExclusion[i] = 0;
				totPnts += pnPolySegPnts[abs(pSegs[i])-1];
				loc += 2;
			}
		}
		else
		{
			for (i=0;i<nSegs;i++)
			{
				pSegIsExclusion[i] = 0;
				pSegs[i] = *(char *)&pTile[loc++];
				totPnts += pnPolySegPnts[abs(pSegs[i])-1]; //pnPolySegPnts[2]
			}
		}
		pPoly = malloc (4*(totPnts+nSegs*3)*sizeof(POINT));
		np = 0;
		ReorderPolySegs (nSegs,pSegs,pSegIsExclusion,pPolySegs,pnPolySegPnts);
		for (i=0;i<nSegs;i++)
		{
			if (!pnPolySegPnts[abs (pSegs[i])-1])
				continue;
if (pSegs[i] > 0)
{
bp = pPolySegs[pSegs[i]-1][0];
ep = pPolySegs[pSegs[i]-1][pnPolySegPnts[pSegs[i]-1]-1];
}
else
{
ep = pPolySegs[abs(pSegs[i])-1][0];
bp = pPolySegs[abs(pSegs[i])-1][pnPolySegPnts[abs(pSegs[i])-1]-1];
}
			if (pSegIsExclusion[i])
				continue;
			if (pSegs[i] > 0)
			{
				if (np)
				{
					if (pRecHead->type == TGPOLYGON)
					{
						np = AddPolygonCornerPoints (pPoly,np,pPolySegs[pSegs[i]-1][0],firstPt);
						if (np < 0)
						{
							np = abs (np);
							for (j=0;j<np;j++)
								pPoly[j] = TranPoint16 (pPoly[j],hTran);
if (!displayonlydebugref || pRecHead->refno == debugref)
							Polygon (hDC,pPoly,np);
							if (showBorder)
								Polyline (hDC,pPoly,np);
							np = 0;
							firstPt = pPolySegs[pSegs[i]-1][0];
						}
					}
				}
				else
					firstPt = pPolySegs[pSegs[i]-1][0];
				memcpy (&pPoly[np],pPolySegs[pSegs[i]-1],pnPolySegPnts[pSegs[i]-1]*sizeof(POINT));
				np += pnPolySegPnts[pSegs[i]-1];
			}
			else
			{
				int iSeg = abs (pSegs[i]) - 1;

				if (np)
				{
					if (pRecHead->type == TGPOLYGON)
					{
						np = AddPolygonCornerPoints (pPoly,np,pPolySegs[iSeg][pnPolySegPnts[iSeg]-1],firstPt);
						if (np < 0)
						{
							np = abs (np);
							for (j=0;j<np;j++)
								pPoly[j] = TranPoint16 (pPoly[j],hTran);
if (!displayonlydebugref || pRecHead->refno == debugref)
							Polygon (hDC,pPoly,np);
							if (showBorder)
								Polyline (hDC,pPoly,np);
							np = 0;
							firstPt = pPolySegs[iSeg][pnPolySegPnts[iSeg]-1];
						}
					}
				}
				else
					firstPt = pPolySegs[iSeg][pnPolySegPnts[iSeg]-1];
				for (j = pnPolySegPnts[iSeg]-1;j >= 0;j--)
					pPoly[np++] = pPolySegs[iSeg][j];
			}
			if (pRecHead->type == TGPOLYLINE)
			{
				for (j=0;j<np;j++)
					pPoly[j] = TranPoint16 (pPoly[j],hTran);
				Polyline (hDC,pPoly,np);//pPoly[0] pPoly[1] pPoly[2] pPoly[3] pPoly[24] pPoly[9]
				np = 0;
			}
		}
		if (pRecHead->type == TGPOLYGON)
		{
			POINT	midPt={150,150};

			if (np)
				np = AddPolygonCornerPoints (pPoly,np,firstPt,midPt);
			pPoly[np++] = firstPt;
nSegs=0;
			for (i=0;i<nSegs;i++)
			{
				if (!pnPolySegPnts[abs (pSegs[i])-1])
					continue;
				if (!pSegIsExclusion[i])
					continue;
				if (pSegs[i] > 0)
				{
					memcpy (&pPoly[np],pPolySegs[pSegs[i]-1],pnPolySegPnts[pSegs[i]-1]*sizeof(POINT));
					np += pnPolySegPnts[pSegs[i]-1];
				}
				else
				{
					int iSeg = abs (pSegs[i]) - 1;

					for (j = pnPolySegPnts[iSeg]-1;j >= 0;j--)
					{
						pPoly[np++] = pPolySegs[iSeg][j];
					}
				}
				pPoly[np++] = firstPt;
			}
		}
		
		for (j=0;j<np;j++)
			pPoly[j] = TranPoint16 (pPoly[j],hTran);

if (!displayonlydebugref || pRecHead->refno == debugref)
		if (pRecHead->type == TGPOLYGON)
		{
			Polygon (hDC,pPoly,np);
			if (showBorder)
				Polyline (hDC,pPoly,np);
		}
		else if (np)
			Polyline (hDC,pPoly,np);//pPoly[0] pPoly[1] pPoly[2] pPoly[3] pPoly[24] pPoly[9]
ii=0;
ptt = pPoly[ii];
		free (pPoly);
		free (pSegs);
		free (pSegIsExclusion);
NextElement:
		SelectObject (hDC,hOldPen);
		if (hPen)
			DeleteObject (hPen);
		if (hOldBrush)
		{
			SelectObject (hDC,hOldBrush);
			DeleteObject (hBrush);
		}
	}

	CloseTRANS2 (&hTran);
	if (nPolySegs > 0)
	{
		free (pnPolySegPnts);
		for (i=0;i<nPolySegs;i++)
		{
			if (pPolySegs[i]) 
				free (pPolySegs[i]);
		}
		free (pPolySegs);
	}
	GSSiGlobUlFree (&hTile);
	return TRUE;
}

int AddToPalette (LPINT plPalette,RGBQUAD *prgbpal,RGBTRIPLE *prgb)
{
	UINT		i;
	static	UINT lasti=0;
	COLORREF	color=RGB(prgb->rgbtRed,prgb->rgbtGreen,prgb->rgbtBlue);
	RGBQUAD		qcolor;
	static	RGBQUAD lastcolor={0,0,0,0};
	LPLONG		pcolor=(LPLONG)&qcolor;

	qcolor.rgbBlue = prgb->rgbtBlue;
	qcolor.rgbGreen = prgb->rgbtGreen;
	qcolor.rgbRed = prgb->rgbtRed;
	qcolor.rgbReserved = 0;
	if (*plPalette && *pcolor == *(LPLONG)&lastcolor)
		return lasti;
	*(LPLONG)&lastcolor = *pcolor;
	if (!containsSymbolColors && prgb->rgbtBlue > 253)
		prgb->rgbtBlue = 253;
	for (i=0;i<*plPalette;i++)
	{
		if (prgb->rgbtBlue > 253)//do not adjust symbol encoded colors
		{
			if (*(LPLONG)&prgbpal[i] == *pcolor)
			{
				lasti = i;
				return i;
			}
		}
		else if (containsSymbolColors)
		{
			if (*(LPLONG)&prgbpal[i] == *pcolor || RGBQUADDist (prgbpal[i],qcolor) < 20)
			{
				lasti = i;
				return i;
			}
		}
		else
		{
			if (*(LPLONG)&prgbpal[i] == *pcolor || RGBQUADDist (prgbpal[i],qcolor) < 4)
			{
				lasti = i;
				return i;
			}
		}
	}
	if (*plPalette <1024)
	{
		prgbpal[*plPalette] = qcolor;
		lasti = (*plPalette)++;
		return (lasti);
	}
	else
		ii=1;
	return 0;
}

int CompressByteArray (LPBYTE pMem,LPBYTE pMemCmp,int lMem)
{
	int	lCmp=0;
	int	lNonRun;
	USHORT	nRun=0;
	signed char	len2;
	int	nNonRun=0;
	LPBYTE	pShort = (LPBYTE)pMem;
	LPBYTE	pEnd = pShort + lMem -1;
	LPBYTE  pEndSeg = (LPBYTE)min ((int)(pShort + 128 - 1),(int)pEnd);
	LPBYTE	pCmpShort = (LPBYTE)pMemCmp;
	LPBYTE	pStart=pShort;
	LPBYTE	pStartRun=pShort;

	do
	{
		do
		{
			nRun++;
			pShort++;
		}while (pShort < pEndSeg && nRun < 128 && *pShort == *pStartRun);
		if (nRun > 2)
		{
			lNonRun = (int)pStartRun - (int)pStart;
			if (lNonRun > 0)
			{
				len2 = lNonRun;
				memcpy (pMemCmp,&len2,1);
				pMemCmp++;
				lCmp++;
				memcpy (pMemCmp,pStart,lNonRun);
				pMemCmp += lNonRun;
				lCmp += lNonRun;
			}
			len2 = -nRun;
			memcpy (pMemCmp,&len2,1);
			pMemCmp++;
			lCmp++;
			memcpy (pMemCmp,pStartRun,1);
			pMemCmp++;
			lCmp++;
			pStart = pStartRun = pShort;
			pEndSeg = (LPBYTE)min ((int)(pShort + 128 - 1),(int)pEnd);
			nRun = 0;
		}
		else if (pShort < pEndSeg)
		{
			nRun = 0;
			pStartRun = pShort;
		}
		else
		{
			lNonRun = (int)pShort - (int)pStart;
			if (lNonRun > 0)
			{
				len2 = lNonRun;
				memcpy (pMemCmp,&len2,1);
				pMemCmp++;
				lCmp++;
				memcpy (pMemCmp,pStart,lNonRun);
				pMemCmp += lNonRun;
				lCmp += lNonRun;
			}
			pStart = pStartRun = pShort;
			pEndSeg = (LPBYTE)min ((int)(pShort + 128 - 1),(int)pEnd);
			nRun = 0;
		}
	}while (pStart <= pEnd);
	return lCmp;
}


int CompressByteArray2 (LPBYTE pMem,LPBYTE pMemCmp,int lMem)
{
	int	lCmp=0;
	int	lNonRun;
	USHORT	nRun=0;
	SHORT	len2;
	int	nNonRun=0;
	LPSHORT	pShort = (LPSHORT)pMem;
	LPSHORT	pEnd = pShort + (lMem/2 -1);
	LPSHORT pEndSeg = (LPSHORT)min ((int)(pShort + SHRT_MAX - 1),(int)pEnd);
	LPSHORT	pCmpShort = (LPSHORT)pMemCmp;
	LPSHORT	pStart=pShort;
	LPSHORT	pStartRun=pShort;

	do
	{
		do
		{
			nRun++;
			pShort++;
		}while (pShort < pEndSeg && nRun < SHRT_MAX && *pShort == *pStartRun);
		if (nRun > 2)
		{
			lNonRun = (int)pStartRun - (int)pStart;
			if (lNonRun > 0)
			{
				len2 = lNonRun/2;
				memcpy (pMemCmp,&len2,2);
				pMemCmp += 2;
				lCmp += 2;
				memcpy (pMemCmp,pStart,lNonRun);
				pMemCmp += lNonRun;
				lCmp += lNonRun;
			}
			len2 = -nRun;
			memcpy (pMemCmp,&len2,2);
			pMemCmp += 2;
			lCmp += 2;
			memcpy (pMemCmp,pStartRun,2);
			pMemCmp += 2;
			lCmp += 2;
			pStart = pStartRun = pShort;
			pEndSeg = (LPSHORT)min ((int)(pShort + SHRT_MAX - 1),(int)pEnd);
			nRun = 0;
		}
		else if (pShort < pEndSeg)
		{
			nRun = 0;
			pStartRun = pShort;
		}
		else
		{
			lNonRun = (int)pShort - (int)pStart;
			if (lNonRun > 0)
			{
				len2 = lNonRun/2;
				memcpy (pMemCmp,&len2,2);
				pMemCmp += 2;
				lCmp += 2;
				memcpy (pMemCmp,pStart,lNonRun);
				pMemCmp += lNonRun;
				lCmp += lNonRun;
			}
			pStart = pStartRun = pShort;
			pEndSeg = (LPSHORT)min ((int)(pShort + SHRT_MAX - 1),(int)pEnd);
			nRun = 0;
		}
	}while (pStart < pEnd);
	return lCmp;
}

HDIB32 Create8BitBMP (HDIB32 dibin,RGBQUAD	*rgbpal, LPLONG plPalette)
{
	int	Width=FreeImage_GetWidth (dibin);
	int	Height=FreeImage_GetHeight (dibin);
	int	bpp=FreeImage_GetBPP (dibin);
	int BitsPerPixel=8, rowlen;
	LPBITMAPINFOHEADER pbi = FreeImage_GetInfoHeader (dibin);
	BOOL rtn=FALSE;
	HDIB32	dib = FreeImage_Allocate (Width,Height,BitsPerPixel,0,0,0);
	RGBTRIPLE	*rgb;
	LPBYTE	pByte;
	int	lPalette=0, iPalette;
	double	mind,maxd;
	UINT	i,j,ii, row, col;
	int		nless10=0;
	int		nSymColors;
	long	SymbolNumberColor = RGB(0,128,255);	
	RGBQUAD	*pal;

	rowlen = pbi->biSizeImage/Height;
	for (row = 0;row < Height; row++)
	{
		rgb = (RGBTRIPLE*)FreeImage_GetScanLine(dibin,row);
		pByte = (LPBYTE)FreeImage_GetScanLine(dib,row);
		for (col = 0; col < Width; col++)
		{
			iPalette = AddToPalette (&lPalette,rgbpal,(RGBTRIPLE*)&rgb[col]);
			pByte[col] = iPalette;
		}
	}
	nSymColors = 0;
	for (i=0;i<lPalette;i++)
		if (rgbpal[i].rgbBlue == 255 && rgbpal[i].rgbGreen != 255 && rgbpal[i].rgbRed != 255)
		{
			long	c=RGB(rgbpal[i].rgbRed,rgbpal[i].rgbGreen,rgbpal[i].rgbBlue);
			long	SymNum = c - SymbolNumberColor;
			nSymColors++;
		}
	pal  = FreeImage_GetPalette (dib);
	for (i=0;i<lPalette;i++)
		pal[i] = rgbpal[i];
	*plPalette = lPalette;
	if (lPalette > 250)
		ii=1;
	return dib;
}

int	SymNumFromColor (RGBQUAD rgbpal)
{
	long	SymNum = -1;

	if (rgbpal.rgbBlue > 253)
	{
		long	SymbolNumberColor = RGB(0,128,rgbpal.rgbBlue);	
		long	c=RGB(rgbpal.rgbRed,rgbpal.rgbGreen,rgbpal.rgbBlue);
		
		SymNum = c - SymbolNumberColor;
	}
	return SymNum;
}

int FindNearestSymbolInPalette (int minfreqi,RGBQUAD *rgbpal,int lPalette)
{
	int	nearsym=-1;
	int	FromSymNum = SymNumFromColor (rgbpal[minfreqi]);
	int	ToSymNum;
	int	Depth, Depth2, diff, maxdiff = INT_MAX, maxdiffi=0;
	UINT	i;
	
	if (FromSymNum >= minsym && FromSymNum <= maxsym)
	{
		FromSymNum -= minsym;
		if (!SymDep[FromSymNum][1])	//contour line symbol
		{
			for (i=0;i<lPalette;i++)
			{
				if (i != minfreqi)
				{
					ToSymNum = SymNumFromColor (rgbpal[i]);
					if (ToSymNum >= minsym && FromSymNum <= maxsym)
					{
						ToSymNum -= minsym;
						if (!SymDep[ToSymNum][1])	//contour line symbol
						{
							diff = abs (SymDep[ToSymNum][0] - SymDep[FromSymNum][0]);
							if (diff < maxdiff)
							{
								maxdiff = diff;
								nearsym = i;
							}
						}
					}
				}
			}
		}
		else
		{
			Depth = (SymDep[FromSymNum][0] + SymDep[FromSymNum][1]) / 2;
			for (i=0;i<lPalette;i++)
			{
				if (i != minfreqi)
				{
					ToSymNum = SymNumFromColor (rgbpal[i]);
					if (ToSymNum >= minsym && FromSymNum <= maxsym)
					{
						ToSymNum -= minsym;
						if (SymDep[ToSymNum][1])
						{
							Depth2 = (SymDep[ToSymNum][0] + SymDep[ToSymNum][1]) / 2;
							diff = abs (Depth - Depth2);
							if (diff < maxdiff)
							{
								maxdiff = diff;
								nearsym = i;
							}
						}
					}
				}
			}
		}
	}
	return nearsym;
}

HDIB32 Create8BitBMPSixteenth (HDIB32 dibin,RGBQUAD	*rgbpal, LPLONG plPalette,int iSixteenth,LPBOOL pErr)
{
	int	Width=FreeImage_GetWidth (dibin);
	int	Height=FreeImage_GetHeight (dibin);
	
	if (iSixteenth < 0)
		iSixteenth = 0;
	else
	{
		Width /= 4;
		Height /= 4;
	}
	{

	int	bpp=FreeImage_GetBPP (dibin);
	int BitsPerPixel=8, rowlen;
	LPBITMAPINFOHEADER pbi = FreeImage_GetInfoHeader (dibin);
	BOOL rtn=FALSE;
	HDIB32	dib = FreeImage_AllocateT(FIT_BITMAP,Width,Height,BitsPerPixel,0,0,0);
	RGBTRIPLE	*rgb;
	LPBYTE	pByte;
	int	lPalette=0, iPalette;
	double	mind,maxd;
	UINT	i,j,ii, row, col, inrow=0, incol=0;
	int		nless10=0;
	int		nSymColors;
//	long	SymbolNumberColor = RGB(0,128,255);	
	RGBQUAD	*pal;
	HANDLE	hByte2;
	LPSHORT	pByte2;
	long	lByte2;

	memset (Frequency,0,sizeof(Frequency));
	rowlen = pbi->biSizeImage/Height;
	inrow  = (iSixteenth / 4) * Height;
	incol  = (iSixteenth % 4) * Width;
	lByte2 = Height*Width;
	hByte2  = GSSiGlobAlloc(GAIDNO 2070,GMEM_MOVEABLE,lByte2*sizeof(short));
	pByte2 = GlobalLock (hByte2);
	for (row = 0;row < Height; row++)
	{
		rgb = (RGBTRIPLE*)FreeImage_GetScanLine(dibin,inrow+row);
		pByte = (LPBYTE)FreeImage_GetScanLine(dib,row);
		for (col = 0; col < Width; col++)
		{
			iPalette = AddToPalette (&lPalette,rgbpal,(RGBTRIPLE*)&rgb[incol+col]);
			Frequency[iPalette]++;
			*pByte2++ = iPalette;
			pByte[col] = iPalette;
		}
	}
	GlobalUnlock (hByte2);
	pByte2 = GlobalLock (hByte2);
/*	nSymColors = 0;
	for (i=0;i<lPalette;i++)
		if (rgbpal[i].rgbBlue == 255 && (rgbpal[i].rgbGreen != 255 || rgbpal[i].rgbRed != 255))
		{
			long	c=RGB(rgbpal[i].rgbRed,rgbpal[i].rgbGreen,rgbpal[i].rgbBlue);
			long	SymNum = c - SymbolNumberColor;
			nSymColors++;
		}*/
	pal  = FreeImage_GetPalette (dib);
	if (lPalette > 256)
	{
		int	lmove;
		int	minfreq;
		int	minfreqi;
		int	NearSymPal;

		for (i=0;i<lPalette;i++)
		{
			if (rgbpal[i].rgbBlue < 254)
				Frequency[i] = INT_MAX;
		}
		do
		{
			minfreq = INT_MAX;
			minfreqi = 0;
			for (i=0;i<lPalette;i++)
			{
				if (Frequency[i] < minfreq)// && rgbpal[i].rgbBlue < 253)
				{
					minfreq = Frequency[i];
					minfreqi = i;
				}
			}
			if (minfreq > 1000)
			{
				lPalette = 255;
				*pErr = TRUE;
				goto ErrOut;
			}
			NearSymPal = FindNearestSymbolInPalette (minfreqi,rgbpal,lPalette);
			if (NearSymPal > -1)
			{
				for (i=0;i<lByte2;i++)
				{
					if (pByte2[i] == minfreqi)
						pByte2[i] = NearSymPal;
				}
				for (i=0;i<lByte2;i++)
				{
					if (pByte2[i] > minfreqi)
						pByte2[i]--;
				}
				lmove = lPalette - minfreqi - 1;
				if (lmove)
				{
					memmove (&rgbpal[minfreqi],&rgbpal[minfreqi+1],lmove * sizeof(RGBQUAD));
					memmove (&Frequency[minfreqi],&Frequency[minfreqi+1],lmove * sizeof(int));
				}
				lPalette--;
			}
			else
				Frequency[minfreqi] = INT_MAX;
		}while (lPalette > 256);
		for (row = 0;row < Height; row++)
		{
			rgb = (RGBTRIPLE*)FreeImage_GetScanLine(dibin,inrow+row);
			pByte = (LPBYTE)FreeImage_GetScanLine(dib,row);
			for (col = 0; col < Width; col++)
			{
				pByte[col] = *pByte2++;
			}
		}
	}
ErrOut:
	for (i=0;i<lPalette;i++)
		pal[i] = rgbpal[i];
	*plPalette = lPalette;
	GSSiGlobUlFree (&hByte2);
	return dib;
	}
}

BOOL ConvertHBirdImages (LPSTR InName, LPSTR OutName,LPSTR TitleMess,LPSTR ColorToSymFile,BOOL isBaseMap)
{
	char	OutFile[MAX_PATH];
	char	OutFile2[MAX_PATH];
	char	MapImageFile[MAX_PATH];
	char	MapImageFileOut[MAX_PATH];
	char	bmpname[MAX_PATH];
	HANDLE	hDB = OpenGWDatabase (InName,BT_READ);  
	HANDLE	hDBOut;
	LPSTR	pDot,pDot2; 
	long	Offsetlong, MapID, Offset;  
	BOOL	rtn=FALSE;
	HFILE	FidOut;
	UINT	row, col, i;
	LPBITMAPINFOHEADER pbi;
	BITMAPFILEHEADER bifh;
	HFILE	Fid,FidColors;
	char	str[256];
	int		Flag=TIFF_ADOBE_DEFLATE;
	int		TotTifLen=0;
	int		TotBMPLen=0;
	int		loc,iSixteenth;
	int		lMemCmpTot=0, lbin1Tot = 0, lbin2Tot=0, lfeTot=0, OrigTot=0;
	RGBQUAD	rgbpal[1024];
	char	Title[128];
	int		nSixteenth=16, nInRow=4;

	if (isBaseMap)
	{
		nSixteenth=1;
		nInRow=1;
		containsSymbolColors = FALSE;
	}
	else
		containsSymbolColors = TRUE;
	FidColors = GSSiOpenFile (ColorToSymFile,0,OF_READ);
	if (FidColors == HFILE_ERROR)
	{
		MessageBox (0,"Cannot open symlst2.bin file",ColorToSymFile,MB_ICONEXCLAMATION);
		return FALSE;
	}
	BigRead (FidColors,&minsym,2);
	BigRead (FidColors,&maxsym,2);
	nsym = (maxsym-minsym+1);
	BigRead (FidColors,SymDep,nsym*4);
	GSSiClose2 (&FidColors);
	GSSiGetTempFileName (0,"gmh",0,(LPSTR)OutFile); 
	if ((pDot = strrchr (OutFile,'.')))
		strcpy (pDot,".tif");
	GSSiGetTempFileName (0,"gmh",0,(LPSTR)OutFile2); 
	if ((pDot = strrchr (OutFile2,'.')))
		strcpy (pDot,".bmp");
	if (dbug)
	{
		strcpy (OutFile,"c:\\temp\\test.tif");
		strcpy (OutFile2,"c:\\temp\\test.bmp");
	}
	strcpy (MapImageFile,InName);
	strcpy (MapImageFileOut,OutName);
	pDot = _fstrrchr (MapImageFile,'.');
	pDot2 = _fstrrchr (MapImageFileOut,'.');
	CreateGWDDatabase (OutName,1,FALSE,0,0,InName);
	{
		HANDLE	hKeyFields;
			
		GetFieldIDsFromNames (OutName,&hKeyFields,0,"GridID;GridCellID",0);
		GWDAddIndex (OutName,hKeyFields,1,0);
		GSSiGlobFree (&hKeyFields);
	}

	hDBOut = OpenGWDatabase (OutName,BT_WRITE);  
	if (hDB)
	{
	    LPGWDHEADER lpGWDHead = (LPGWDHEADER)GlobalLock (hDB);
	    LPMAPFILE   pMAPFILE = (LPMAPFILE)&lpGWDHead->GWDData;  
		LPGWDHEADER lpGWDHeadOut = (LPGWDHEADER)GlobalLock (hDBOut);
		LPMAPFILE   pMAPFILEOut = (LPMAPFILE)&lpGWDHeadOut->GWDData;  
		int	pos=BT_FIRST;
		int	nRecs = BT_NUM_IN_INDEX (lpGWDHead->BTHandle[0]);
		int	Curloc = 0;
		int	lPalette;
		LPSTR	pVersion;

		pVersion = (LPSTR)FreeImage_GetVersion();

		sprintf (Title,"Convert Images %s",TitleMess);
		CreateStatusWind (hWndMain,1,Title);
		    
        while (ContinueProcessing && !BT_FIND (lpGWDHead->BTHandle[0],(LPSTR)&MapID,pos,BT_ANY, (LPSTR)&Offset)) 
        {
			pos = BT_NEXT;
        	FillGWDData (lpGWDHead,Offset);
			*pMAPFILEOut = *pMAPFILE;
			pMAPFILEOut->ID *= 16;
			pMAPFILEOut->SubDir /= 4;
			
		    sprintf (pDot,"%i.bin",pMAPFILE->SubDir);
		    sprintf (pDot2,"%i.bin",pMAPFILEOut->SubDir);
			{
				HFILE	FidBIN=GSSiOpenFile (MapImageFile,0,OF_READ);
				
				_fstrcpy (pDot,".gmd");	 
				if (FidBIN != HFILE_ERROR)
				{
					HANDLE	hMem=GSSiGlobAlloc(GAIDNO 2071,GMEM_MOVEABLE,pMAPFILE->Size);
					HPSTR	pMem=GlobalLock (hMem);
					
					GSSillseek2 (FidBIN,pMAPFILE->Loc,0);
					BigRead (FidBIN,pMem,pMAPFILE->Size);
					OrigTot += pMAPFILE->Size;
					FidOut = GSSiOpenFile (OutFile,0,OF_CREATE);
					if (FidOut != HFILE_ERROR)
					{
						HDIB32	dibin, dib8;
						BOOL	Err=FALSE;

						BigWrite (FidOut,pMem,pMAPFILE->Size,-1);
						GSSiClose2 (&FidOut);
						dibin = BMPHandleFromEXT (OutFile);
						for (iSixteenth=0;iSixteenth < nSixteenth;iSixteenth++)
						{
							int	sixrow=iSixteenth / nInRow, sixcol=iSixteenth % nInRow;
							double	OrigWidth  = pMAPFILE->Bounds.xmx - pMAPFILE->Bounds.xmn;
							double	OrigHeight = pMAPFILE->Bounds.ymx - pMAPFILE->Bounds.ymn;
							RGBQUAD	*rgbpalx;
							int	ipal;

							if (isBaseMap)
								dib8 = Create8BitBMPSixteenth (dibin, rgbpal,&lPalette,-1,&Err);
							else
								dib8 = Create8BitBMPSixteenth (dibin, rgbpal,&lPalette,iSixteenth,&Err);

							pbi = FreeImage_GetInfoHeader (dib8);
							pbi->biClrUsed = 256;
							pbi->biClrImportant = lPalette;
							rgbpalx = FreeImage_GetPalette (dib8);
							for (ipal=lPalette;ipal<256;ipal++)
							{
								rgbpalx[ipal].rgbBlue = rgbpalx[ipal].rgbGreen = rgbpalx[ipal].rgbRed = rgbpalx[ipal].rgbReserved = 0;
							}
							GMFIBMPHandleToEXT (OutFile2,dib8,0);
							pbi = FreeImage_GetInfoHeader (dib8);
							{
								HANDLE	hMem, hMem2, hMemCmp, hMembin;
								LPSTR	pMem, pMemCmp, pMem2, pMembin;
								int		l, lfe, lbin,lMemCmp,lMemDeCmp;
								char	GridName[32];
								HFILE	FidBINOut;
								HFILE	FidBMPOut;
								HFILE	Fid;
								DPOINT	Point;
								LPBITMAPFILEHEADER	pbmFileHeader;
								LPBITMAPINFOHEADER	pbmInfoHeader;

								l = GSSiLength (OutFile2);
								hMem = GSSiGlobAlloc(GAIDNO 2072,GMEM_MOVEABLE,l);
								pMem = GlobalLock (hMem);
								Fid  = GSSiOpenFile (OutFile2,0,OF_READ);
								BigRead (Fid,pMem,l);
								GSSiClose2 (&Fid);
								pbmFileHeader = (LPBITMAPFILEHEADER)pMem;
								pbmInfoHeader = (LPBITMAPINFOHEADER)(pbmFileHeader+1);
								pbmInfoHeader->biXPelsPerMeter = pbmInfoHeader->biYPelsPerMeter = 0;
								l = pbmFileHeader->bfSize;
								memset (pbmFileHeader,0,sizeof(BITMAPFILEHEADER));//zero out for compression since not used on HB side
								memset (pbmInfoHeader,0,sizeof(BITMAPINFOHEADER));//zero out for compression since not used on HB side
								hMembin = GSSiGlobAlloc(GAIDNO 2073,GMEM_MOVEABLE,l+1024);
								pMembin = GlobalLock (hMembin);
								hMemCmp = GSSiGlobAlloc(GAIDNO 2074,GMEM_MOVEABLE,l+1024);
								pMemCmp = GlobalLock (hMemCmp);
								lMemCmp = CompressByteArray (pMem,pMemCmp,l);
								lbin = CompressBinaryRecord (pMemCmp,pMembin,lMemCmp); 
								if ((FidBINOut=GSSiOpenFile (MapImageFileOut,0,OF_READWRITE)) == HFILE_ERROR)
									FidBINOut=GSSiOpenFile (MapImageFileOut,0,OF_CREATE);
								loc = GSSillseek (FidBINOut,0,2);
								pMAPFILEOut->Loc = loc;
								pMAPFILEOut->Size = lbin;
								pMAPFILEOut->Format = 1;
								pMAPFILEOut->GridID = pMAPFILE->GridID/nInRow;
								if (!isBaseMap)
								{
									itoa (pMAPFILEOut->GridID,GridName,10);
									GetGridDef (GridName);
									pMAPFILEOut->Bounds.xmn = pMAPFILE->Bounds.xmn + sixcol * OrigWidth/4;
									pMAPFILEOut->Bounds.xmx = pMAPFILE->Bounds.xmn + (sixcol+1) * OrigWidth/4;
									pMAPFILEOut->Bounds.ymn = pMAPFILE->Bounds.ymn + sixrow * OrigHeight/4;
									pMAPFILEOut->Bounds.ymx = pMAPFILE->Bounds.ymn + (sixrow+1) * OrigHeight/4;
									Point = MinMaxMidPointD (&pMAPFILEOut->Bounds);
									pMAPFILEOut->GridCellID = DGridCellID (&Point);
								}
								if (pMAPFILEOut->GridCellID > -1)
								{
									pMAPFILEOut->ID++;
									BigWrite (FidBINOut,pMembin,lbin,-1);
									GWDReplaceRecord (lpGWDHeadOut,0,NULL,-1); 
								}
								GSSiClose2 (&FidBINOut);

								GSSiGlobUlFree (&hMem);
								GSSiGlobUlFree (&hMembin);
								GSSiGlobUlFree (&hMemCmp);
							}
							GSSiFreeImage_Unload (dib8);
						}
						if (Err)
						{
							char	ErrFile[MAX_PATH];

							sprintf (ErrFile,"[OUTDIR]\\bmperr\\%i_%i.bmp",pMAPFILE->SubDir,pMAPFILE->ID);
							ExpandText (ErrFile);
							copyfile (ErrFile,OutFile,FALSE,0,0,0,0,0,0);
						}
						GSSiFreeImage_Unload (dibin);
						rtn=TRUE;
					}
					GSSiClose2 (&FidBIN);  
					GSSiGlobUlFree (&hMem);
				}
			} 
			if (!StatusWindowUpdate (NULL,NULL, nRecs,Curloc++))
				break;
		}
		DestroyStatusWindow(0);  
		SetContinueProcessing ( TRUE);
		GlobalUnlock (hDB);
	    CloseGWDatabase (hDB);   
		GlobalUnlock (hDBOut);
	    CloseGWDatabase (hDBOut);   
	}
	GSSiRemove (OutFile);
	GSSiRemove (OutFile2);
	return rtn;
}	
/*							mind=DBL_MAX;
							maxd=0;
							for (i=0;i<lPalette-1;i++)
								for (j=i+1;j<lPalette;j++)
								{
									if ((rgbpal[i].rgbBlue == 255 && rgbpal[i].rgbGreen != 255 && rgbpal[i].rgbRed != 255) ||
										(rgbpal[i].rgbBlue == 255 && rgbpal[i].rgbGreen != 255 && rgbpal[i].rgbRed != 255))
									{
										ii=1;
									}
									else
									{
										double d=RGBQUADDist (rgbpal[i],rgbpal[j]);

										if (d < 10)
											nless10++;
										sprintf (str,"%i %i %i %i %i %i %i",IDNINT(d),rgbpal[i].rgbRed,rgbpal[j].rgbRed,rgbpal[i].rgbGreen,rgbpal[j].rgbGreen,rgbpal[i].rgbBlue,rgbpal[j].rgbBlue);
										AppendFile ("c:\\temp\\colordist.txt",str);
										mind = min (d,mind);
										maxd = max (d,maxd);
									}
								}

							sprintf (str,"%f %f",mind,maxd);
							AppendFile ("c:\\temp\\colordist.txt",str);*/

						/*	if (lPalette > 256)
							{
								sprintf (bmpname,"c:\\temp\\temp%i.bmp",Offset);
								GMFIBMPHandleToEXT (bmpname,dibin,0);
							}
							else if (nSymColors > 256)
							{
							GMFIBMPHandleToEXT ("c:\\temp\\temp.bmp",dib,0);
							Fid = GSSiOpenFile ("c:\\temp\\temp5.bmp",0,OF_CREATE);
							pbi->biSizeImage =  WIDTHBYTES((DWORD)pbi->biWidth * pbi->biBitCount) * pbi->biHeight;
							memset (&bifh,0,sizeof(BITMAPFILEHEADER));
							memcpy (&bifh.bfType,"BM",2);
							bifh.bfSize = sizeof(BITMAPINFOHEADER) + 1024 + pbi->biSizeImage;
							BigWrite (Fid,&bifh,sizeof(BITMAPFILEHEADER),-1);
							BigWrite (Fid,pbi,sizeof(BITMAPINFOHEADER),-1);
							BigWrite (Fid,rgbpal,1024,-1);
							pByte = (LPBYTE)FreeImage_GetBits (dib);
							BigWrite (Fid,pByte,pbi->biSizeImage,-1);
							GSSiClose2 (&Fid);
								sprintf (bmpname,"c:\\temp\\temp%i.bmp",Offset);
								GMFIBMPHandleToEXT (bmpname,dibin,0);
							/*GMFIBMPHandleToEXT ("c:\\temp\\temp3.bmp",dibt,0);
							//FreeImage_Unload (dibt);
							}
							else*/
							 /*	lbin = DecompressBinaryRecord (pMemCmp,pMembin,lbin); 
							 	lMemDeCmp = DeCompressByteArray (pMemCmp,pMem2,lbin);
								if (l != lMemDeCmp)
									ii=1;
								else
								{
									for (i=0;i<l;i++)
										if (pMem[i] != pMem2[i])
											ii=1;
								}*/
								//BigRead (Fid,pMem,l);
								//GSSiClose2 (&Fid);
							/*	FidBMPOut = GSSiOpenFile ("c:\\temp\\tempbmp.bmp",0,OF_CREATE);
								FileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD);
								FileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD) + lMemDeCmp;
								BigWrite (FidBMPOut,&FileHeader,sizeof(BITMAPFILEHEADER),-1);
								BigWrite (FidBMPOut,pbi,sizeof(BITMAPINFOHEADER),-1);
								BigWrite (FidBMPOut,rgbpalx,256*sizeof(RGBQUAD),-1);
								BigWrite (FidBMPOut,pMem2,lMemDeCmp,-1);
								GSSiClose2 (&FidBMPOut);
								FidBMPOut = GSSiOpenFile ("c:\\temp\\temppaint.bmp",0,OF_READ);
								BigRead (FidBMPOut,&FileHeader,sizeof(BITMAPFILEHEADER));
								BigRead (FidBMPOut,pbi,sizeof(BITMAPINFOHEADER));
								GSSiClose2 (&FidBMPOut);*/
								//TotBMPLen += (GSSiLength (bmpname)-256*sizeof(RGBQUAD)+lPalette*sizeof(RGBQUAD)-sizeof(BITMAPFILEHEADER)-sizeof(BITMAPINFOHEADER));

																//sprintf (bmpname,"c:\\temp\\temp.tif",Offset);
								//GMFIBMPHandleToEXT (bmpname,dib,TIFF_ADOBE_DEFLATE);
								//TotTifLen += GSSiLength (bmpname);
								/*sprintf (bmpname,"c:\\temp\\temp.tif",Offset);
								GMFIBMPHandleToEXT (bmpname,dib,TIFF_ADOBE_DEFLATE);
								lfe = GSSiLength (bmpname);
								lfeTot += lfe;*/
								//Fid = GSSiOpenFile (bmpname,0,OF_READ);
								//l = GSSillseek (Fid,0,2);
								//GSSillseek (Fid,0,0);


BOOL ConvertHBirdColorsOld (HDIB32 hDib,int ClrUsed,RGBQUAD *pal)
{
/*	RGBQUAD	Black = {0,0,0,0};
	long	SymbolNumberColor = RGB(0,128,255);	
	static	BOOL	Loaded=FALSE;
	int		symnum, Depth, Depth1, Depth2;
	int		i, ii;
	BOOL	ShowContourLines=GetGlobalBVal2 ("[%SHOWHBIRDCONTOURLINES]",FALSE);
	BOOL	ShowDepthColors=GetGlobalBVal2 ("[%SHOWHBIRDDEPTHCOLORS]",TRUE);
	int		HighlightDepth = GetGlobalLVal2 ("[%HIGHLIGHTDEPTH]",0);
	int		HighlightDepthRange = GetGlobalLVal2 ("[%HIGHLIGHTDEPTHRANGE]",5);
	char	Symname[32];
	LPSTR	pto;

	if (!hDib)
		return FALSE;
	if (FreeImage_GetBPP (hDib) != 8)
		return FALSE;
	if (!Loaded)
	{
		char	str[130];
		int		numsym=0;
		HFILE Fid = GSSiOpenFile ("[%DL]\\symdump.txt",0,OF_READ);

		minsym = 5000;
		maxsym = 0;
		while (fgetstring (str,128,Fid))
		{
			LPSTR ptab2 = strrchr (str,'\t');
			LPSTR ptab1 = strchr (str,'\t');

			*ptab1 = 0;
			ptab2++;
			symnum = atoi (ptab2);
			if (symnum > 0 && symnum < 2048)
			{
				minsym = min (minsym,symnum);
				maxsym = max (maxsym,symnum);
				strcpy (Symname,strupr(str));
				numsym++;
			}
		}
		GSSillseek (Fid,0,0);
		memset (SymDepth,0,sizeof(SymDepth));
		while (fgetstring (str,128,Fid))
		{
			LPSTR ptab2 = strrchr (str,'\t');
			LPSTR ptab1 = strchr (str,'\t');

			*ptab1 = 0;
			ptab2++;
			symnum = atoi (ptab2);
			strcpy (Symname,strupr(str));
			if (symnum > 0 && symnum < 2048)
			{
				if (!strnicmp (Symname,"LKC",3))
					SymDepth[symnum - minsym][0] = atoi (&Symname[3]);
				else
				{
					SymDepth[symnum - minsym][0] = SymDepth[symnum - minsym][1] = atoi (Symname);
					if ((pto = strstr (Symname,"TO")))
						SymDepth[symnum - minsym][1] = atoi (pto+2);
				}
			}
		}
		GSSiClose2 (&Fid);
		Fid = GSSiOpenFile ("c:\\temp\\hbirdtest\\symlist.bin",0,OF_CREATE);
		BigWrite (Fid,&minsym,2,-1);
		BigWrite (Fid,&maxsym,2,-1);
		BigWrite (Fid,SymDepth,(maxsym-minsym+1)*2,-1);
		GSSiClose2 (&Fid);
		Loaded = TRUE;
	}

	for (i=0;i<ClrUsed;i++,pal++)
	{
		if (pal->rgbBlue == 255	&& pal->rgbGreen != 255 && pal->rgbRed != 255)
		{
			long	c=RGB(pal->rgbRed,pal->rgbGreen,pal->rgbBlue);
			long	SymNum = c - SymbolNumberColor;

			if (SymNum >= minsym && SymNum <= maxsym)
			{
				SymNum -= minsym;
				if (!SymDepth[SymNum][1])
				{
					if (ShowContourLines)
						*pal = Black;
					else
					{
						Depth = SymDepth[SymNum][0];
						*pal = DepthColor (Depth,HighlightDepth,HighlightDepthRange,ShowDepthColors);
					}
				}
				else
				{
					Depth1 = SymDepth[SymNum][0];
					Depth2 = SymDepth[SymNum][1];
					Depth = (Depth1 + Depth2) / 2;
					*pal = DepthColor (Depth,HighlightDepth,HighlightDepthRange,ShowDepthColors);
				}
			}
		}
	}*/
	return TRUE;
}

BOOL SetHBirdStretchDIBits (int destX,int destY,int destW, int destH,int xoff,int yoff,int sourcew, int sourceh,int ImageWidth,int ImageHeight)
{
	int	CurLoc;
	LKMTILE LKMTile;

	if (FidHBirdData == HFILE_ERROR)
		return FALSE;
	LKMTile.width = ImageWidth;
	LKMTile.height = ImageHeight;
	LKMTile.xoff =  xoff;
	LKMTile.yoff =  yoff;
	LKMTile.rectw = sourcew;
	LKMTile.recth = sourceh;
	LKMTile.destx = destX;
	LKMTile.desty = destY;
	LKMTile.destw = destW;
	LKMTile.desth = destH;
	BigWrite (FidHBirdIndex,&ImageWidth,sizeof(int),-1);
	BigWrite (FidHBirdIndex,&ImageHeight,sizeof(int),-1);
	BigWrite (FidHBirdIndex,&LastHBLoc,sizeof(int),-1);
	BigWrite (FidHBirdIndex,&LastHBLen,sizeof(int),-1);
	CurLoc = GSSillseek (FidHBirdData,0,1);
	GSSillseek (FidHBirdData,LastHBLoc,0);
	BigWrite (FidHBirdData,&LKMTile,sizeof(LKMTILE)-4,-1);
	GSSillseek (FidHBirdData,CurLoc,0);
	NumTiles++;
	return TRUE;
}

BOOL WriteToHBird (LPBYTE pMem,int Size)
{
	static	BOOL First=TRUE;
	LKMTILE LKMTile;

//	return FALSE;
	if (!pMem)
	{
		if (!First)
		{
			First = TRUE;
			GSSillseek (FidHBirdIndex,0,0);
			BigWrite (FidHBirdIndex,&NumTiles,sizeof(int),-1);
			GSSiClose2 (&FidHBirdData);
			GSSiClose2 (&FidHBirdIndex);
			FidHBirdData = HFILE_ERROR;
			NumTiles=0;
		}
		return TRUE;
	}
	if (First)
	{
		FidHBirdData = GSSiOpenFile ("c:\\temp\\hbirdtest\\imagelib.bin",0,OF_CREATE);
		FidHBirdIndex = GSSiOpenFile ("c:\\temp\\hbirdtest\\imageindex.bin",0,OF_CREATE);
		BigWrite (FidHBirdIndex,&NumTiles,sizeof(int),-1);
		First = FALSE;
	}
	LastHBLoc = GSSillseek (FidHBirdData,0,1);
	BigWrite (FidHBirdData,&LKMTile,sizeof(LKMTILE)-4,-1);
	BigWrite (FidHBirdData,pMem,Size,-1);
	LastHBLen = sizeof(LKMTILE) + Size -4;

	return TRUE;
}

BOOL TestHBird (HDC hDC)
{
	LKMToHBInit ("c:\\temp\\hbirdtest\\");

	LakeMasterToHBirdImage ((HANDLE)hDC,620,386,0,0,0,0,1,1,0,0);
	return TRUE;
}

BOOL AddCrossing (int maxCrossing,int xory,LPPOLYCROSSING pCrossing,char type,BOOL rotated)
{
	LPBYTE pLoc;
	LPINT	pnCrossings, pnSplinedCrossings;
	int		nrows,	ncols;
	LPPOLYCROSSING pxCrossings, pyCrossings, pCrossings;
	LPFLTPOINT pSpline;
	
	if(pCrossing->depth <= 0)
		ii=1;
	if (rotated)
	{
		nrows = nrows45;
		ncols = ncols45;
		pxCrossings = pxCrossings45;
		pyCrossings = pyCrossings45;
	}
	else
	{
		nrows = nrows0;
		ncols = ncols0;
		pxCrossings = pxCrossings0;
		pyCrossings = pyCrossings0;
	}

	switch (type)
	{
	case 'x':
		if (xory < 0 || xory >= nrows)
			ii=1;
		pLoc =(LPBYTE)pxCrossings;
		break;
	case 'y':
		if (xory < 0 || xory >= ncols)
			ii=1;
		pLoc =(LPBYTE)pyCrossings;
		break;
	}
	pLoc += (sizeof(int)*2 + sizeof(LPFLTPOINT) + maxCrossing * sizeof (POLYCROSSING)) * xory;
	pnCrossings = (LPINT)pLoc;
	pnSplinedCrossings = pnCrossings + 1;
	pSpline = (LPFLTPOINT)(pnSplinedCrossings + 1);
	pCrossings = (LPPOLYCROSSING)(pLoc+sizeof(int)*2 + sizeof(LPFLTPOINT));
	if (!(*pnCrossings))
		*pCrossings = *pCrossing;
	else
	{
		int n = *pnCrossings;

		while (n > 0 && pCrossing->xory > pCrossings->xory)
		{
			n--;
			pCrossings++;
		}
		while (n)
		{
			*(pCrossings + n) = *(pCrossings + (n-1));
			n--;
		}
		*pCrossings = *pCrossing;
	}
	(*pnCrossings)++;
	if (*pnCrossings >= maxCrossing)
		return FALSE;

	return TRUE;
}

BOOL CreateLakeDepthTriangles (BOOL Init)
{
	BOOL rtn=FALSE;
	int	nRecs, iDone=0,pos=BT_FIRST, pixelX, pixelY, Refno;
	MNMXCORD	bounds=HLTBounds;
	DPOINT	point, wpoint;
	int	maxxpx=0, maxxpy=0;
	HIGHLIGHTDATA	HighlightData;
	int	nPnts, i, j, x, y, dx, dy, inc;
	float dxdy, dydx;
	double a;
	HANDLE	 hPnts;
	POLYCROSSING	crossing;
	DPOINT FromPt[2], ToPt[2], ZeroPt={0,0};
	float	RSQMIN;
	LPTRANDATA	TranPtr;
	BOOL	rotated;

	if (hTran45)
	{
		CloseTRANS2 (&hTran45); 
		free (pxCrossings0);
		free (pyCrossings0);
		free (pxCrossings45);
		free (pyCrossings45);
	}
	if (!Init)
		return TRUE;
	if (!hHighlight)
		return FALSE;
	DBoundsInit (&wtrbounds);
	ConvertRectCoord (&bounds, &HLTBounds, 1,2);
	point.x = bounds.xmn;
	point.y = bounds.ymn;
	LatLongToPixelXY(point.y, point.x, maxDTLev, &pixelX, &pixelY);
	wpoint.x = pixelX;
	wpoint.y = pixelY;
	AddDPointToMinMax (&wpoint,&wtrbounds);
	point.x = bounds.xmn;
	point.y = bounds.ymx;
	LatLongToPixelXY(point.y, point.x, maxDTLev, &pixelX, &pixelY);
	wpoint.x = pixelX;
	wpoint.y = pixelY;
	AddDPointToMinMax (&wpoint,&wtrbounds);
	point.x = bounds.xmx;
	point.y = bounds.ymx;
	LatLongToPixelXY(point.y, point.x, maxDTLev, &pixelX, &pixelY);
	wpoint.x = pixelX;
	wpoint.y = pixelY;
	AddDPointToMinMax (&wpoint,&wtrbounds);
	point.x = bounds.xmx;
	point.y = bounds.ymn;
	LatLongToPixelXY(point.y, point.x, maxDTLev, &pixelX, &pixelY);
	wpoint.x = pixelX;
	wpoint.y = pixelY;
	AddDPointToMinMax (&wpoint,&wtrbounds);
	nrows0 = wtrbounds.ymx - wtrbounds.ymn + 1;
	ncols0 = wtrbounds.xmx - wtrbounds.xmn + 1;
	ixbase0 = wtrbounds.xmn;
	iybase0 = wtrbounds.ymn;
	
	wtrbounds45 = wtrbounds;
	wtrbounds45.xmn -= ixbase0;
	wtrbounds45.xmx -= ixbase0;
	wtrbounds45.ymn -= iybase0;
	wtrbounds45.ymx -= iybase0;
	ZeroPt = MinMaxMidPointD (&wtrbounds45);
	FromPt[0].x = wtrbounds45.xmn;
	FromPt[1].x = wtrbounds45.xmx;
	FromPt[0].y = FromPt[1].y = (wtrbounds45.ymn + wtrbounds45.ymx)/2;
	ToPt[0] = dnewpt (ZeroPt,HALFPI/2,-ldistp (FromPt[0],ZeroPt));
	ToPt[1] = dnewpt (ZeroPt,HALFPI/2,ldistp (FromPt[1],ZeroPt));
	hTran45 = STRANPoints (0,FromPt,ToPt,2,&RSQMIN,1,NULL); 

	TranBounds (hTran45,&wtrbounds45);
	nrows45 = wtrbounds45.ymx - wtrbounds45.ymn + 1;
	ncols45 = wtrbounds45.xmx - wtrbounds45.xmn + 1;
	ixbase45 = wtrbounds45.xmn;
	iybase45 = wtrbounds45.ymn;

	CreateStatusWind (hWndMain,1,"Get Depth Polygons");
	nRecs = BT_NUM_IN_INDEX (hHighlight);
	maxCrossing = max (100,nRecs);
	pxCrossings0 = malloc (nrows0 * (sizeof(int)*2 + sizeof(LPFLTPOINT) + maxCrossing * sizeof(POLYCROSSING)));
	memset (pxCrossings0,0,nrows0 * (sizeof(int)*2 + sizeof(LPFLTPOINT) + maxCrossing * sizeof(POLYCROSSING)));
	pyCrossings0 = malloc (ncols0 *(sizeof(int)*2 + sizeof(LPFLTPOINT)  + maxCrossing * sizeof(POLYCROSSING)));
	memset (pyCrossings0,0,ncols0 * (sizeof(int)*2 + sizeof(LPFLTPOINT) + maxCrossing * sizeof(POLYCROSSING)));
	pxCrossings45 = malloc (nrows45 * (sizeof(int)*2 + sizeof(LPFLTPOINT) + maxCrossing * sizeof(POLYCROSSING)));
	memset (pxCrossings45,0,nrows45 * (sizeof(int)*2 + sizeof(LPFLTPOINT) + maxCrossing * sizeof(POLYCROSSING)));
	pyCrossings45 = malloc (ncols45 *(sizeof(int)*2 + sizeof(LPFLTPOINT)  + maxCrossing * sizeof(POLYCROSSING)));
	memset (pyCrossings45,0,ncols45 * (sizeof(int)*2 + sizeof(LPFLTPOINT)  + maxCrossing * sizeof(POLYCROSSING)));
	while (StatusWindowUpdate (0,HighlightData.PD.UDI, nRecs, iDone++) && !BT_FIND (hHighlight,(LPSTR)&Refno,pos,BT_ANY,(LPSTR)&HighlightData))
	{   
		pos = BT_NEXT;     
		
		if (HighlightData.PD.Type == 2)// && Refno == 658740)
		{  
			int	nd, polyDepth = GetSymbolDepth (HighlightData.PD.Desc,&nd); 
			
			if (Refno == 658740)
				ii=1;
			if (GetPolyPoints ((LPPICKDATAHEADER)&HighlightData.PD,FALSE,&nPnts,&hPnts, 0))
			{  
				HPDPOINT pPoly=GlobalLock (hPnts);
				HANDLE	hPolyInt=GSSiGlobAlloc(GAIDNO 1773,GMEM_MOVEABLE,nPnts*sizeof(POINT));
				LPPOINT	pPolyInt=GlobalLock (hPolyInt);

				ConvertPolyCoord (pPoly,nPnts,1,2);
				for (i=0;i<nPnts;i++)
				{
					if (i == 4289)
						ii=1;
					LatLongToPixelXY(pPoly[i].y, pPoly[i].x, maxDTLev, &pPolyInt[i].x, &pPolyInt[i].y);
					pPolyInt[i].x -= ixbase0;
					pPolyInt[i].y -= iybase0;
				}
				GSSiGlobUlFree (&hPnts);
				rotated = FALSE;

Top:
				for (i=1;i<nPnts;i++)
				{
					if (i && (polyDepth == 72||polyDepth == 80) && min (pPolyInt[i-1].x,pPolyInt[i].x) <= 38994 && max (pPolyInt[i-1].x,pPolyInt[i].x) >= 38994)
						ii=1;
					if (i > 0)
					{
						//average the angle for the first point
						dx = pPolyInt[i].x - pPolyInt[max(0,i-2)].x;
						dy = pPolyInt[i].y - pPolyInt[max (0,i-2)].y;
						if (dx)
						{
							dydx = (float)dy/(float)dx;
							a = atan (dydx)/HALFPI;
							crossing.angleWeight = (1.0 - fabs(a)) * 255;
						}
						else
							crossing.angleWeight = 0;
						crossing.xory = pPolyInt[i-1].y;
						crossing.depth = polyDepth;
						if (!AddCrossing (maxCrossing,pPolyInt[i-1].x,&crossing,'y',rotated))
						{
							GSSiGlobUlFree (&hPolyInt);
							goto Exit;
						}
						if (dy)
						{
							dxdy = (float)dx/(float)dy;
							a = atan (dxdy)/HALFPI;
							crossing.angleWeight = (1.0 - fabs(a)) * 255;
						}
						else
							crossing.angleWeight = 0;
						crossing.xory = pPolyInt[i-1].x;
						crossing.depth = polyDepth;
						if (!AddCrossing (maxCrossing,pPolyInt[i-1].y,&crossing,'x',rotated))
						{
							GSSiGlobUlFree (&hPolyInt);
							goto Exit;
						}
					}
					dx = pPolyInt[i].x - pPolyInt[i-1].x;
					dy = pPolyInt[i].y - pPolyInt[i-1].y;
					if (dx)
					{
						inc = (dx < 0) ? -1: 1;
						dydx = inc * (float)dy / (float)dx;
						a = atan (dydx)/HALFPI;
						crossing.angleWeight = (1.0 - fabs(a)) * 255;
						crossing.depth = polyDepth;
						x = pPolyInt[i-1].x + inc;
						j=1;
						while (x != pPolyInt[i].x)
						{
							crossing.xory = IDNINT (pPolyInt[i-1].y + j*dydx);
							if (AddCrossing (maxCrossing,x,&crossing,'y',rotated))
							{
								x += inc;
								j++;
							}
							else
							{
								GSSiGlobUlFree (&hPolyInt);
								goto Exit;
							}
						}
					}
					else
					{
						crossing.angleWeight = 0;
						crossing.depth = polyDepth;
						crossing.xory = pPolyInt[i-1].y;
						if (!AddCrossing (maxCrossing,pPolyInt[i].x,&crossing,'y',rotated))
						{
							GSSiGlobUlFree (&hPolyInt);
							goto Exit;
						}
						crossing.xory = pPolyInt[i].y;
						if (!AddCrossing (maxCrossing,pPolyInt[i].x,&crossing,'y',rotated))
						{
							GSSiGlobUlFree (&hPolyInt);
							goto Exit;
						}
					}
					if (dy)
					{
						inc = (dy < 0) ? -1: 1;
						dxdy = inc * (float)dx / (float)dy;
						a = atan (dxdy)/HALFPI;
						crossing.angleWeight = (1.0 - fabs(a)) * 255;
						crossing.depth = polyDepth;
						y = pPolyInt[i-1].y + inc;
						j=1;
						while (y != pPolyInt[i].y)
						{
							crossing.xory = IDNINT (pPolyInt[i-1].x + j*dxdy);
							if (AddCrossing (maxCrossing,y,&crossing,'x',rotated))
							{
								y += inc;
								j++;
							}
							else
							{
								GSSiGlobUlFree (&hPolyInt);
								goto Exit;
							}
						}
					}
					else
					{
						crossing.angleWeight = 0;
						crossing.depth = polyDepth;
						crossing.xory = pPolyInt[i-1].x;
						if (!AddCrossing (maxCrossing,pPolyInt[i].y,&crossing,'x',rotated))
						{
							GSSiGlobUlFree (&hPolyInt);
							goto Exit;
						}
						crossing.xory = pPolyInt[i].x;
						if (!AddCrossing (maxCrossing,pPolyInt[i].y,&crossing,'x',rotated))
						{
							GSSiGlobUlFree (&hPolyInt);
							goto Exit;
						}
					}
				}
				if (!rotated)
				{
					rotated = TRUE;
					for (i=0;i<nPnts;i++)
					{
						pPolyInt[i] = TranPoint16 (pPolyInt[i],hTran45);
						pPolyInt[i].x -= ixbase45;
						pPolyInt[i].y -= iybase45;
					}
					goto Top;
				}
				GSSiGlobUlFree (&hPolyInt);
			}
		}
	}
	DestroyStatusWindow (0);
	{
		LPPOLYCROSSING pCrossings;
		LPFLTPOINT *pSplinedCrossings;
		int nSplinedPoints;
		LPDPOINT pSplinedPoints;
		int	nGaps = 0;
		UINT totSize=0;
		LPINT	pnCrossings, pnSplinedCrossings;
		LPFLTPOINT pSpline;

		for (i=0;i<ncols0;i++)
		{
			LPBYTE pLoc=(LPBYTE)pyCrossings0;
			pLoc += ((sizeof (int)*2 + sizeof(LPFLTPOINT) + maxCrossing * sizeof (POLYCROSSING)) * i);
			pnCrossings = (LPINT)pLoc;
			pnSplinedCrossings = pnCrossings + 1;
			pSpline = (LPFLTPOINT)(pnSplinedCrossings+1);

			pCrossings = (LPPOLYCROSSING)(pLoc+sizeof(int)*2 + sizeof(LPFLTPOINT));
			if ((nSplinedPoints = CreateCrossingSpline (*pnCrossings,pCrossings,&pSplinedPoints)))
			{
				free (pSplinedPoints);
				totSize += nSplinedPoints;
			}
			maxxpy = max (maxxpy,*pnCrossings);
			if (*pnCrossings)
			{
				int lastDepth = 0;
				int	lastDist = 0;
				for (j=0;j<*pnCrossings;j++)
				{
					if (abs(pCrossings[j].depth - lastDepth) > 8)
						nGaps++;
					lastDepth = pCrossings[j].depth;
					lastDist = pCrossings[j].xory;
				}
			}
		}
//		pFCrossings = malloc (totSize * sizeof(FLTPOINT));
//		free (pFCrossings);
		for (i=0;i<nrows0;i++)
		{
			LPBYTE pLoc=(LPBYTE)pxCrossings0;
			pLoc += ((sizeof (int)*2 + sizeof(LPFLTPOINT) + maxCrossing * sizeof (POLYCROSSING)) * i);
			pnCrossings = (LPINT)pLoc;
			pnSplinedCrossings = pnCrossings + 1;

			pCrossings = (LPPOLYCROSSING)(pLoc+sizeof(int)*2 + sizeof(LPFLTPOINT));
			maxxpx = max (maxxpx,*pnCrossings);
		}
		maxxpx = maxxpy = 0;
		for (i=0;i<ncols45;i++)
		{
			LPBYTE pLoc=(LPBYTE)pyCrossings45;
			pLoc += ((sizeof (int)*2 + sizeof(LPFLTPOINT) + maxCrossing * sizeof (POLYCROSSING)) * i);
			pnCrossings = (LPINT)pLoc;
			pnSplinedCrossings = pnCrossings + 1;

			pCrossings = (LPPOLYCROSSING)(pLoc+sizeof(int)*2 + sizeof(LPFLTPOINT));
			maxxpy = max (maxxpy,*pnCrossings);
		}
		for (i=0;i<nrows45;i++)
		{
			LPBYTE pLoc=(LPBYTE)pxCrossings45;
			pLoc += ((sizeof (int)*2 + sizeof(LPFLTPOINT) + maxCrossing * sizeof (POLYCROSSING)) * i);
			pnCrossings = (LPINT)pLoc;
			pnSplinedCrossings = pnCrossings + 1;

			pCrossings = (LPPOLYCROSSING)(pLoc+sizeof(int)*2 + sizeof(LPFLTPOINT));
			maxxpx = max (maxxpx,*pnCrossings);
		}
	}
	rtn = TRUE;
Exit:
	return rtn;
}

int CreateCrossingSpline (int nCrossings,LPPOLYCROSSING pCrossings,LPDPOINT *ppSplinedPoints)
{
	int nSplinePoints = 0;

	return 0;
	if (nCrossings > 2)
	{
		LPDPOINT pPointsToSpline = malloc (nCrossings * sizeof(DPOINT));
		LPDPOINT pSplinedPoints;
		int isp, maxSplinedPoints,  ib=0;
		
		for (isp=0;isp<nCrossings;isp++)
		{
			pPointsToSpline[isp].x = pCrossings[isp].xory;
			pPointsToSpline[isp].y = pCrossings[isp].depth;
		}
		maxSplinedPoints = pCrossings[nCrossings-1].xory - pCrossings[0].xory;
		pSplinedPoints = malloc ((maxSplinedPoints+1)*sizeof(DPOINT));
		nSplinePoints = SplinePointsD (1,nCrossings,pPointsToSpline,&ib, pSplinedPoints,1,0,0,maxSplinedPoints); 
		free (pPointsToSpline);
		*ppSplinedPoints = pSplinedPoints;
	}
	return nSplinePoints;
}

float GetDirectedElev (UINT iDir,int iFactor,LPPOINT pt,LPINT aWeight,LPINT dWeight)
{
	float elv=-1;
	LPPOLYCROSSING pxCrossings=pxCrossings0, pyCrossings=pyCrossings0, pCrossing, pCrossingBegin;
	LPFLTPOINT *pSplinedCrossing;
	LPBYTE pLoc;
	LPINT	pnCrossings, pnSplinedCrossing;
	int		i, j, n, inc, prevDepth, prevDist, prevWeight;
	BOOL	inLake=FALSE;

	//iFactor /= 2;
	if (iDir > 4)
	{
		pxCrossings=pxCrossings45;
		pyCrossings=pyCrossings45;
		iDir -= 4;
	}
	switch (iDir)
	{
	case 1:
		pLoc=(LPBYTE)pxCrossings;
		inc = 0;
		i = pt->y;
		j = pt->x;
		break;
	case 2:
		pLoc=(LPBYTE)pyCrossings;
		inc = 0;
		i = pt->x;
		j = pt->y;
		break;
	case 3:
		pLoc=(LPBYTE)pxCrossings;
		inc = 1;
		i = pt->y;
		j = pt->x;
		break;
	case 4:
		pLoc=(LPBYTE)pyCrossings;
		inc = 1;
		i = pt->x;
		j = pt->y;
		break;
	}

	pLoc += ((sizeof (int)*2 + sizeof(LPFLTPOINT) + maxCrossing * sizeof (POLYCROSSING)) * i);
	pnCrossings = (LPINT)pLoc;
	pnSplinedCrossing = pnCrossings + 1;
	pSplinedCrossing = (LPFLTPOINT*)(pnSplinedCrossing + 1);
	pCrossing = (LPPOLYCROSSING)(pSplinedCrossing+1);
	pCrossingBegin = pCrossing;
	n = 0;
	prevDepth = 0;
	prevDist = 0;
	prevWeight = 0;
	while (n < *pnCrossings && pCrossing->xory < j)
	{
		if (!pCrossing->depth && pCrossing->angleWeight)
			inLake = !inLake;
		prevDepth = pCrossing->depth;
		prevDist = pCrossing->xory;
		prevWeight = pCrossing->angleWeight;
		n++;
		pCrossing++;
	}
	if (!n)
		return -1;
	if (n < *pnCrossings)
	{
		if (prevDepth > 0 || pCrossing->depth > 0)
			inLake = TRUE;
		if (pCrossing->xory == j)
		{
			*aWeight = pCrossing->angleWeight;
			*dWeight = 0;
			elv = pCrossing->depth;
		}
		else if ((j - prevDist <= pCrossing->xory - j) && j - prevDist < iFactor)
		{
			*aWeight = pCrossing->angleWeight;
			*dWeight = j - prevDist;
			elv = prevDepth;
		}
		else if (pCrossing->xory - j < iFactor)
		{
			*aWeight = pCrossing->angleWeight;
			*dWeight = pCrossing->xory - j;
			elv = pCrossing->depth;
		}
		else if ((prevDepth == 0 && pCrossing->depth == 0) && !inLake)
		{
			*aWeight = 0;
			*dWeight = min (j - prevDist,pCrossing->xory - j);
			elv = -1;
		}
		else
		{
			int nHalf = nWantSplinePoints/2;
			int ibSpline = max (0,n-nHalf);
			int ieSpline = min (*pnCrossings-1,n+nHalf-1);
			int nSpline = 0;//ieSpline - ibSpline + 1;
			DPOINT pPointsToSpline[nWantSplinePoints];
			DPOINT outPoints[MaxSplinePoints+1];
			int nOutPnts=0, isp, npnew;
			
			elv = prevDepth + (float)(pCrossing->depth - prevDepth) * ((float)(j - prevDist)/(float)(pCrossing->xory - prevDist));

			for (isp=0;isp<nSpline;isp++)
			{
				pPointsToSpline[isp].x = pCrossingBegin[ibSpline+isp].xory;
				pPointsToSpline[isp].y = pCrossingBegin[ibSpline+isp].depth;
			}
			if (nSpline > 2)
			{
				int inp;

				npnew = SplinePointsD (1,nSpline,pPointsToSpline,&nOutPnts, outPoints,1,0,0,MaxSplinePoints); 
				for (inp=0;inp<npnew-1;inp++)
				{
					if (outPoints[inp].x <= j && outPoints[inp+1].x >= j)
					{
						elv = outPoints[inp].y + (outPoints[inp+1].y - outPoints[inp].y) * ((j - outPoints[inp].x) / (outPoints[inp+1].x - outPoints[inp].x));
						break;
					}
				}
			}
			if (!inc)
			{
				*aWeight = prevWeight;
				*dWeight = j - prevDist;
			}
			else
			{
				*aWeight = pCrossing->angleWeight;
				*dWeight = pCrossing->xory - j;
			}
		}
	}
	return elv;
}


 void setscreenpixel (POINT screenpt,COLORREF color)
 {
	 return;
 }
int StretchHBBits (HANDLE HBImageHandle,
				   int XDest, int YDest, int nDestWidth, int nDestHeight,
				   int XSrc,  int YSrc,  int nSrcWidth,  int nSrcHeight, 
				   BYTE *lpBits,
				   int TileHeight, int TileWidth,
				   int TilePaletteLen,
				   RGBQUAD *TilePalette)
{
	return 1;
}


int DisplayCurrentLayer (int i)
{
	return 1;
}

int GetLakeOffset (LPVOID pFid)
{
	return 0;
}

void HBDisplayTextChar (HANDLE HBImageHandle,char chr, int nchar, int BitmapX, int BitmapY,int Rotation,int Size)
{
	return;
}

BOOL HBDisplayBitmap (HANDLE HBImageHandle,char * BitmapPathName,int PCTSize,int WorldX,int WorldY)
{
	return TRUE;
}

void *cl_malloc(unsigned long nbytes)
{
  return malloc (nbytes);
  
}

void WRITE_TO_ERROR_LOG (void)
{
	return;
}
void WRITE_TO_ERROR_LOG2 (void)
{
	return;
}



void cl_free(void *ap)
{

  free (ap);
  return;
}

int GetTick (void)
{
	return 0;
}

void OffsetTrackPoints ()
{
	return;
}

POINT DPOINTtoCL_POINT (LPDPOINT p)
{
	POINT pt={0};

	return pt;
}

DPOINT CL_POINTtoDPOINT (DPOINT p)
{
	DPOINT pt={0};

	return pt;
}

void ReSampleToOriginalSpacing (void)
{
	return;
}

int GetAZMInDegrees (LPDPOINT Point1,LPDPOINT Point2)
{
	return 0;
}
double getazmd (LPDPOINT Point1,LPDPOINT Point2)
{
	return 0;
}
void db_fread (void)
{
	return;
}
void db_fseek (void)
{
	return;
}
void db_fopen (void)
{
	return;
}
void ININT (void)
{
	return;
}
void ConvertUTMtoLatLon(void)
{
	return;
}
void db_ftell (void)
{
	return;
}
void db_fclose (void)
{
	return;
}

void ConvertLatLontoUTM (void)
{
	return;
}

void cl_chkmalloc (void)
{
}

void WriteParamToErrorLog (void)
{
}

void WriteInformationToErrorLog (void)
{
}

BOOL CreateTileDepthPixels(LPSTR Directory, int iLevel, int tileX, int tileY)
{
	BOOL rtn = FALSE;
	/*UINT irow, icol, iDir;
	POINT pt, pt2;
	DPOINT	dpt;
	int	iFactor = 1, worldCoordFactor, worldX, worldY;
	int aWeight[8], totaWeight, dWeight[8], totdWeight, totWeight;
#define GRIDWIDTH	257
	short	ptElev[GRIDWIDTH][GRIDWIDTH];
	BYTE	ptElev8[GRIDWIDTH][GRIDWIDTH];
	float rElev[8];
	double rElv, pixelElev;
	char pathName[MAX_PATH];
	HFILE	Fid;
	MNMXCORD	bounds = wtrbounds;
	BOOL	haveData = FALSE;
	int		minWeight;
	int		zeroDist;
	float minElev;
	static int wantrow = 58, wantcol = 7;

	if (iLevel == 16 && tileX == 15695 && tileY == 23610)
		ii = 1;
	//	else
	//		return FALSE;
	bounds.xmn -= ixbase0;
	bounds.ymn -= iybase0;
	bounds.xmx -= ixbase0;
	bounds.ymx -= iybase0;

	sprintf(pathName, "%s\\%i\\%i\\%i.bin", Directory, iLevel, tileX, tileY);
	if (iLevel < maxDTLev)
		iFactor = pow(2, maxDTLev - iLevel);

	TileXYToPixelXY(tileX, tileY, &worldX, &worldY);
	worldX *= iFactor;
	worldY *= iFactor;

	zeroDist = iFactor * 1.5;
	for (irow = 0; irow < GRIDWIDTH; irow++)
	{
		if (irow >= 251)
			ii = 1;
		pt.y = worldY + irow * iFactor;
		for (icol = 0; icol < GRIDWIDTH; icol++)
		{
			if (irow == wantrow && icol == wantcol)
				ii = 1;
			pt.x = worldX + icol * iFactor;
			totaWeight = 0;
			totdWeight = 0;
			pt2.x = pt.x - ixbase0;
			pt2.y = pt.y - iybase0;
			dpt = PointToDPoint(pt2);
			if (!DPointInBounds(&dpt, &bounds))
				ptElev[irow][icol] = -1;
			else
			{
				int nOut = 0;

				for (iDir = 0; iDir < 8; iDir++)
				{
					if (iDir == 4)
					{
						POINT p = TranPoint16(pt2, hTran45);
						pt2.x = p.x - ixbase45;
						pt2.y = p.y - iybase45;
					}
					rElev[iDir] = GetDirectedElev(iDir + 1, zeroDist, &pt2, &aWeight[iDir], &dWeight[iDir]);
					if (rElev[iDir] < 0)
					{
						nOut++;
						aWeight[iDir] = 0;
						rElev[iDir] = 0;
						if (nOut > 2)
						{
							ptElev[irow][icol] = -1;
							goto NextCol;
						}
					}
					else if (dWeight[iDir] < zeroDist)
					{
						ptElev[irow][icol] = IDNINT(TRIANGLE_DEPTH_MULTIPLIER * rElev[iDir]);
						haveData = TRUE;
						goto NextCol;
					}
					totaWeight += aWeight[iDir];
					totdWeight += dWeight[iDir];
				}
				minWeight = 9999;
				for (iDir = 0; iDir < 8; iDir++)
				{
					if (dWeight[iDir] < minWeight)
					{
						minWeight = dWeight[iDir];
						minElev = rElev[iDir];
					}
				}
				if (minWeight < iFactor)
				{
					ptElev[irow][icol] = IDNINT(TRIANGLE_DEPTH_MULTIPLIER * minElev);
					haveData = TRUE;
					goto NextCol;
				}
				rElv = 0;
				totWeight = 0;
				for (iDir = 0; iDir < 8; iDir++)
				{
					int weight = aWeight[iDir] * (totdWeight - dWeight[iDir]);

					if (rElev[iDir] < 0)
					{
						ptElev[irow][icol] = -1;
						goto NextCol;
					}
					rElv += (double)rElev[iDir] * weight;
					totWeight += weight;
				}
				pixelElev = (rElv* TRIANGLE_DEPTH_MULTIPLIER) / totWeight;
				ptElev[irow][icol] = IDNINT(pixelElev);
				if ((ptElev[irow][icol] % (8 * TRIANGLE_DEPTH_MULTIPLIER)) == 0)
				{
					if (pixelElev < ptElev[irow][icol])
						ptElev[irow][icol]--;
					else
						ptElev[irow][icol]++;
				}

				haveData = TRUE;
			}
		NextCol:;
		}
	}
	if (haveData)
	{
		Fid = GSSiOpenFile(pathName, 0, OF_CREATE);
		if (Fid != HFILE_ERROR)
		{
			short minElev = SHRT_MAX;
			short maxElev = SHRT_MIN;
			int	i, j, baseElev;

			for (i = 0; i < GRIDWIDTH; i++)
				for (j = 0; j < GRIDWIDTH; j++)
				{
					if (ptElev[i][j] > -1)
					{
						minElev = min(minElev, ptElev[i][j]);
						maxElev = max(maxElev, ptElev[i][j]);
					}
				}

			if (maxElev - minElev < 255)
			{
				baseElev = minElev;
				BigWrite(Fid, &baseElev, sizeof(int), -1);
				for (i = 0; i < GRIDWIDTH; i++)
					for (j = 0; j < GRIDWIDTH; j++)
					{
						if (ptElev[i][j] > -1)
							ptElev8[i][j] = ptElev[i][j] - baseElev;
						else
							ptElev8[i][j] = 255;
					}

				BigWrite(Fid, ptElev8, sizeof(ptElev8), -1);
			}
			else
			{
				baseElev = -99999;
				BigWrite(Fid, &baseElev, sizeof(int), -1);
				BigWrite(Fid, ptElev, sizeof(ptElev), -1);
			}
			GSSiClose2 (&Fid);
			rtn = TRUE;
		}
	}*/
	return rtn;
}
