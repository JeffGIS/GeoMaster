#include	<stdlib.h>
#include	<math.h>
#include	<string.h>
#include	<stdio.h>
#include	"lkmhbird.h"
#include	"clmalloc.h"
#include	"classert.h"
#include	"LatLong-UTMconversion.h"
#include	"fs.h"


#define BOUNDSPARM "B="

extern char	LKMPath[CL_MAX_PATH];

void SavePoints (int np,LPDPOINT p);
int RestorePoints (LPDPOINT *Points);
int RemoveKnots (int nPnts,LPDPOINT *Points,double OffsetDist,int knotRemovalRange);
int SmoothTrackLine (int nPnts,LPDPOINT *Points,int nSmooth,double TrackPointSpacing);
void UTMtoLL(int ReferenceEllipsoid, const double UTMNorthing, const double UTMEasting, int iUTMZone,
			  double* pLat,  double* pLong );

long    ININT (double X)
{
    if (X < 0)
        return ((long) (X - 0.5));
    return ((long) (X + 0.5));
}

int iSign (double *val)
{
	if (*val < 0)
		return -1;
	else
		return 1;
}

double BTWNTWOPI (double AZ1)
{
      if (AZ1<0)
      	return (TWOPI + fmod(AZ1,TWOPI));
      if (AZ1 == 0)
      	return (AZ1);
      return (fmod(AZ1,TWOPI));
}

double LTWOPI (double az)
{
	return BTWNTWOPI (az);
}

double distpd(LPDPOINT Point1, LPDPOINT Point2)
{
    return sqrt ((double)(Point1->x-Point2->x)*(double)(Point1->x-Point2->x) +
                 (double)(Point1->y-Point2->y)*(double)(Point1->y-Point2->y));
}
double distp(CL_POINT Point1, CL_POINT Point2)
{
    return sqrt ((double)(Point1.x-Point2.x)*(double)(Point1.x-Point2.x) +
                 (double)(Point1.y-Point2.y)*(double)(Point1.y-Point2.y));
}
double LDIST(double X1,double Y1,double X2,double Y2)
{ 
	double xdf = (X1-X2);
	double ydf = (Y1-Y2);
  	
  	return sqrt(xdf * xdf + ydf * ydf);
}

double LGETAZ(double X1, double Y1, double X2, double Y2)
{
	double	rtn=LTWOPI(atan2(Y2 - Y1,X2 - X1));
	return rtn;
}

CL_POINT inewpt (CL_POINT OldPoint, double AZM, double DIS)
{
	  CL_POINT NewPoint;

      NewPoint.x= ININT((OldPoint.x+DIS*cos(AZM)));
      NewPoint.y= ININT((OldPoint.y+DIS*sin(AZM)));
      return (NewPoint);
}

DPOINT dnewpt (LPDPOINT OldPoint, double AZM, double DIS)
{
	  DPOINT NewPoint;

      NewPoint.x= OldPoint->x+DIS*cos(AZM);
      NewPoint.y= OldPoint->y+DIS*sin(AZM);
      return NewPoint;
}

double getazm (CL_POINT Point1, CL_POINT Point2)
{   
	double	rtn=BTWNTWOPI(atan2(((double)Point2.y-(double)Point1.y),((double)Point2.x-(double)Point1.x)));
	return rtn;
}

double getazmd (LPDPOINT Point1,LPDPOINT Point2)
{   
	double	rtn=BTWNTWOPI(atan2(((double)Point2->y-(double)Point1->y),((double)Point2->x-(double)Point1->x)));
	return rtn;
}
int GetAZMInDegrees (LPDPOINT Point1,LPDPOINT Point2)
{
	double az = getazmd (Point1,Point2);

	return ININT(az * RADtoDEG);
}

double DeltaAZ (double *AZ1, double *AZ2)
{         
	double daz;
	
	daz =  *AZ2 - *AZ1;
	if (daz < -PY)
		daz += TWOPI;
	else if (daz > PY)
		daz -= TWOPI;
	return daz;
}   

int ReSampleToOriginalSpacing (int nPnts,LPDPOINT *pPointsIn,int MaxPoints,double TrackPointSpacing)
{
	int nNewPnts = 1;
	int i = 1;
	double lastDist, nextDist;
	LPDPOINT pPoints = *pPointsIn;
	LPDPOINT newPoints;

	if (!nPnts)
		return 0;
	newPoints  = (LPDPOINT)lm_malloc (MaxPoints*sizeof(DPOINT));
	if (newPoints == NULL)
		return 0;

	newPoints[0] = pPoints[0];

	lastDist = nextDist = 0;
	while (i < nPnts && nNewPnts < MaxPoints)
	{
		nextDist = distpd (&newPoints[nNewPnts-1],&pPoints[i]);
		if (nextDist >= TrackPointSpacing)
		{
			double	az = getazmd (&pPoints[i-1],&pPoints[i]);
			double	dist = ((TrackPointSpacing - lastDist) /(nextDist - lastDist)) * distpd (&pPoints[i-1],&pPoints[i]);

			newPoints[nNewPnts++] = pPoints[i-1] = dnewpt (&pPoints[i-1],az, dist);
			lastDist = nextDist = 0;
		}
		else
		{
			lastDist = nextDist;
			i++;
		}
	}
	if (i < nPnts)
		newPoints[nNewPnts-1] = pPoints[nPnts-1];

	cl_free (*pPointsIn);
	*pPointsIn = newPoints; //newPoints[nNewPnts-1]
	return nNewPnts;
}

void AddNewTrackPoint (DPOINT newPoint,LPDPOINT trackPoint,LPINT pnTrackPoints)
{
	if (fabs (newPoint.x - trackPoint[*pnTrackPoints-1].x) > 0.2 ||
		fabs (newPoint.y - trackPoint[*pnTrackPoints-1].y) > 0.2)
		trackPoint[(*pnTrackPoints)++] = newPoint;
	return;
}

void OffsetTrackPoints (LPIPILOTSTRUCT piPilot)
{
	int	i, nNewPnts=1;
	double	az1, az2, az, azdiff;
	int		maxNewPoints = piPilot->lenTrackArray * 3;
	LPDPOINT newPoints;

	if (!piPilot->lenTrackArray)
	{
		piPilot->lenTrackArrayOut = 0;
		return;
	}
	if (piPilot->offsetInMeters == 0)
	{
		piPilot->pTrackArrayTemp = (LPDPOINT)lm_malloc (piPilot->lenTrackArray * sizeof(DPOINT));
		piPilot->lenTrackArrayTemp = piPilot->lenTrackArray;
		ClAssert( NULL != piPilot->pTrackArrayTemp );
		memcpy (piPilot->pTrackArrayTemp,piPilot->pTrackArray,piPilot->lenTrackArray*sizeof(DPOINT));
	}
	else
	{
		newPoints  = (LPDPOINT)lm_malloc (maxNewPoints * sizeof(DPOINT));
		ClAssert( NULL != newPoints );
		az2 = getazmd (&piPilot->pTrackArray[0],&piPilot->pTrackArray[1]);
		newPoints[0] = dnewpt (&piPilot->pTrackArray[0],az2-HALFPI,piPilot->offsetInMeters);
		for (i=1;i<piPilot->lenTrackArray-1;i++)
		{
			az1 = az2;
			az2 = getazmd (&piPilot->pTrackArray[i],&piPilot->pTrackArray[i+1]);
			azdiff = DeltaAZ (&az1,&az2);
			az = BTWNTWOPI (az1 + (PY + azdiff) / 2);
			if (iSign (&azdiff) == iSign (&piPilot->offsetInMeters))
			{
				// turning away from offset - add 3 points
				AddNewTrackPoint (dnewpt (&piPilot->pTrackArray[i],az1-HALFPI,piPilot->offsetInMeters),newPoints,&nNewPnts);
				AddNewTrackPoint (dnewpt (&piPilot->pTrackArray[i],az,-piPilot->offsetInMeters),newPoints,&nNewPnts);
				AddNewTrackPoint (dnewpt (&piPilot->pTrackArray[i],az2-HALFPI,piPilot->offsetInMeters),newPoints,&nNewPnts);
			}
			else //newPoints[nNewPnts-3] newPoints[nNewPnts-2] newPoints[nNewPnts-1]
			{
				double d = piPilot->offsetInMeters / sin ((PY -fabs (azdiff)) / 2);

				if (piPilot->offsetInMeters > 0)
					d = CL_MIN (piPilot->offsetInMeters,d);
				else
					d = CL_MAX (piPilot->offsetInMeters,d);
				AddNewTrackPoint (dnewpt (&piPilot->pTrackArray[i],az,-d),newPoints,&nNewPnts);
			}
			if (nNewPnts > maxNewPoints - 4)
				break;
		}
		AddNewTrackPoint (dnewpt (&piPilot->pTrackArray[piPilot->lenTrackArray-1],az2-HALFPI,piPilot->offsetInMeters),newPoints,&nNewPnts);
		piPilot->lenTrackArrayTemp = nNewPnts;
		if (piPilot->pTrackArrayTemp != NULL)
			cl_free (piPilot->pTrackArrayTemp);
		piPilot->pTrackArrayTemp = newPoints;
		piPilot->lenTrackArrayTemp = RemoveKnots (piPilot->lenTrackArrayTemp,&piPilot->pTrackArrayTemp,piPilot->offsetInMeters,piPilot->knotRemovalRange);
		piPilot->lenTrackArrayTemp = SmoothTrackLine (piPilot->lenTrackArrayTemp,&piPilot->pTrackArrayTemp,piPilot->nSmoothPass,piPilot->trackPointSpacing);
	}
	return; //piPilot->pTrackArray[0]  piPilot->pTrackArray[piPilot->lenTrackArray-1] 
			//piPilot->pTrackArrayTemp[0]  piPilot->pTrackArrayTemp[piPilot->lenTrackArrayTemp-1] 
}

CL_BOOL IntersectLines (LPDPOINT fromPt,LPDPOINT fromEP,double *fromAz,LPDPOINT toPt,LPDPOINT toEP,double *toAz,LPDPOINT pIntPt)
{

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
      if (CL_MIN(Z1, Z2) < (TOL * 1e-3)) {goto S30;} /*going straight up */
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
S1000:pIntPt->x = X3 + X1IN ;
      pIntPt->y = Y3 + Y1IN ;
	  lineLength = distpd (fromPt,fromEP);
	  if (distpd (pIntPt,fromPt) > lineLength)
		  return FALSE;
	  if (distpd (pIntPt,fromEP) > lineLength)
		  return FALSE;
	  lineLength = distpd (toPt,toEP);
	  if (distpd (pIntPt,toPt) > lineLength)
		  return FALSE;
	  if (distpd (pIntPt,toEP) > lineLength)
		  return FALSE;
      return(RC==1);
}

DPOINT CL_POINTtoDPOINT (CL_POINT point)
{
	DPOINT dpoint;

	dpoint.x = point.x;
	dpoint.y = point.y;
	return dpoint;
}
CL_POINT DPOINTtoCL_POINT (LPDPOINT dpt)
{
	CL_POINT pt;

	pt.x = (int)((dpt->x >= 0)? dpt->x + 0.5 : dpt->x - 0.5);
	pt.y = (int)((dpt->y >= 0)? dpt->y + 0.5 : dpt->y - 0.5);
	return pt;
}

int SmoothTrackLine (int nPnts,LPDPOINT *Points,int nSmooth,double TrackPointSpacing)
{
	int	i;
	LPDPOINT	smoothPoints, pPoints = *Points;

	if (!nSmooth)
		return nPnts;
//smoothing
	smoothPoints = lm_malloc (nPnts * sizeof (DPOINT));
	smoothPoints[0] = pPoints[0];
	if (nSmooth < 2)
	{
		for (i=1;i<nPnts-1;i++)
		{
			smoothPoints[i].x = (pPoints[i-1].x + pPoints[i].x + pPoints[i+1].x)/3;
			smoothPoints[i].y = (pPoints[i-1].y + pPoints[i].y + pPoints[i+1].y)/3;
		}
	}
	else
	{
		smoothPoints[1].x = (pPoints[0].x + pPoints[1].x + pPoints[2].x)/3;
		smoothPoints[1].y = (pPoints[0].y + pPoints[1].y + pPoints[2].y)/3;
		for (i=2;i<nPnts-2;i++)
		{
			smoothPoints[i].x = (pPoints[i-2].x + pPoints[i-1].x + pPoints[i].x + pPoints[i+1].x + pPoints[i+2].x)/5;
			smoothPoints[i].y = (pPoints[i-2].y + pPoints[i-1].y + pPoints[i].y + pPoints[i+1].y + pPoints[i+2].y)/5;
		}
		smoothPoints[i].x = (pPoints[i-1].x + pPoints[i].x + pPoints[i+1].x)/3;
		smoothPoints[i].y = (pPoints[i-1].y + pPoints[i].y + pPoints[i+1].y)/3;
	}
	smoothPoints[nPnts-1] = pPoints[nPnts-1];

	cl_free (*Points);
	*Points = smoothPoints;
	return nPnts;
}

int RemoveKnots (int nPnts,LPDPOINT *Points,double OffsetDist,int knotRemovalRange)
{
	int nNewPnts = 1;
	int	nLines = nPnts-1;
	int	iLine, i;
	int	onLine = 1;			//line current point is on and direction (+ going from bp to ep and - going ep to bp)
	int	nextOnLine;
	LPDPOINT pPoints = *Points, newPoints;
	DPOINT	fromPt, toPt, intPt, lineBP, lineEP;
	double	fromAz, toAz, dAz;
	double *lineAz;
	double	minDist, dist;
	int		maxNewPnts = nPnts * 2;
	double	TOL=0.000001;
	if (nLines < 3 || knotRemovalRange < 3)
		return nPnts;
	newPoints  = (LPDPOINT)lm_malloc ((maxNewPnts+1) *sizeof(DPOINT));
	if (newPoints == NULL)
		return nPnts;

//knot removal
	lineAz = (double *)lm_malloc ((nPnts-1) * sizeof(double));
	if (lineAz == NULL)
	{
		cl_free (newPoints);
		return nPnts;
	}
	for (i=0;i<nPnts-1;i++)
		lineAz[i] = getazmd (&pPoints[i],&pPoints[i+1]);

	newPoints[0] = fromPt = pPoints[0];
	newPoints[1] = toPt = pPoints[1];
	minDist = distpd (&newPoints[0],&newPoints[1]);
	onLine = 1;
	nextOnLine = 2;
	fromAz = lineAz[0];
	
	do
	{
		int	beginLine = CL_MAX (1,abs(onLine)-knotRemovalRange);
		int	endLine = CL_MIN (nLines,abs(onLine)+knotRemovalRange);

		for  (iLine = beginLine;iLine < endLine;iLine++)
		{
			switch (iLine - abs (onLine))
			{
			case  0:
			case  1:
			case -1:
				continue;
			default:
				toAz = lineAz[iLine-1];
				lineBP = pPoints[iLine-1];
				lineEP = pPoints[iLine];
				if (IntersectLines (&newPoints[nNewPnts-1],&toPt,&fromAz,&lineBP,&lineEP,&toAz,&intPt))
				{
					dist = distpd (&newPoints[nNewPnts-1],&intPt);
					if (dist > TOL && dist < minDist)
					{
						minDist = dist;
						newPoints[nNewPnts] = intPt;
						dAz = DeltaAZ (&fromAz,&toAz);
						if (iSign (&dAz) != iSign (&OffsetDist))
							nextOnLine = iLine;
						else
							nextOnLine = -iLine;
					}
				}
				else
					continue;
			}
		}
		nNewPnts++;
		onLine = nextOnLine;
		if (onLine < 0)
		{
			toPt = pPoints[abs(onLine)-1];
			fromAz = BTWNTWOPI(PY + lineAz[abs(onLine)-1]);
			nextOnLine = onLine + 1;
		}
		else
		{
			toPt = pPoints[onLine];
			fromAz = lineAz[onLine-1];
			nextOnLine = onLine + 1;
		}
		minDist = distpd (&newPoints[nNewPnts-1],&toPt);
		newPoints[nNewPnts] = toPt;
	} while (onLine && abs(onLine) <= nLines && nNewPnts < maxNewPnts);
	cl_free (lineAz);

	cl_free (*Points);
	*Points = newPoints;
	return nNewPnts;
}

void ConvertUTMtoLatLon (LPDPOINT pPoint,int iUTMZone)
{
	DPOINT point;
	int ReferenceEllipsoid = 23;//WGS84


	UTMtoLL(ReferenceEllipsoid, pPoint->y, pPoint->x, iUTMZone,
			&point.y,  &point.x);
	pPoint->x = point.x;
	pPoint->y = point.y;
	return;
}

void ConvertLatLontoUTM (LPDPOINT pPoint,int iUTMZone, char *ActualZone)
{
	DPOINT point;
	int ReferenceEllipsoid = 23;//WGS84

	LLtoUTM(ReferenceEllipsoid, pPoint->y, pPoint->x,
			&point.y,  &point.x, iUTMZone,ActualZone);
	pPoint->x = point.x;
	pPoint->y = point.y;
	return;
}



void ConvertUTMtoLatLon_grid (LPDPOINT pPoint,CL_BOOL init)
{
	double XI, YI;
	static	double BASX, BASY, A1, A2, B1, B2, C1, C2;
	static	double xMin,xMax,yMin,yMax;
	static	CL_BOOL haveInit=FALSE;
	int		idb;

Top:
	if (init)
	{
		char	TranFile[CL_MAX_PATH];
		CLFILE	*Fid;
		char	pHead[100];
		char	*pLine;
		char	*pLoc;
		double	gridWidth,gridHeight,gridX,gridY,maxErr;
		int		iGrid, lineLen, lineNum, irow, icol;

		sprintf (TranFile,"%sLLTRAN.txt",LKMPath);
		Fid = db_fopen (TranFile,"rb");

		if (!Fid)
			return; // FALSE;

		db_fread2 (pHead,14,100,Fid,FALSE);
		pHead[98] = 0;
		sscanf (pHead,"%i %i %lf %lf %lf %lf",&iGrid,&lineLen,&gridWidth,&gridHeight,&gridX,&gridY);
		pLine = lm_malloc (lineLen+1);
		ClAssert( NULL != pLine );
		irow = (pPoint->y - gridY) / gridHeight;
		icol = (pPoint->x - gridX) / gridWidth;
		if (irow < 0 || irow > iGrid ||
			icol < 0 || icol > iGrid)
		{
			cl_fclose (Fid);
			free (pLine);
			return;
		}
		lineNum = irow * iGrid + icol;
		db_fseek (Fid,100+lineNum*lineLen,SEEK_SET);
		db_fread2 (pLine,14,lineLen,Fid,FALSE);
		pLine[lineLen] = 0;
		cl_fclose (Fid);
		pLoc = pLine;
		pLoc += strlen(BOUNDSPARM);
		sscanf (pLoc,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",&xMin,&yMin,&xMax,&yMax,&BASX, &BASY, &A1, &B1, &C1, &A2,  &B2,  &C2, &maxErr);
		free (pLine);
		haveInit = TRUE;
	}
	if (!haveInit)
		return;
	if (pPoint->x >= xMin && pPoint->x <= xMax &&
		pPoint->y >= yMin && pPoint->y <= yMax)
		idb=1;
	else
	{
		init = TRUE;
		goto Top;
	}
	XI = pPoint->x - BASX;
	YI = pPoint->y - BASY;
	pPoint->x = A1 + B1 * XI + C1 * YI;
	pPoint->y = A2 + B2 * YI + C2 * XI;

	return;
}

long Convert_Geodetic_To_UTM (double Lat_Degrees,
                              double Long_Degrees,
                              int    Zone_Override,
                              long   *ActualZone,
                              char   *Hemisphere,
                              double *Easting,
                              double *Northing)
{
	DPOINT	point;
	char	cActualZone[4];

	point.x = Long_Degrees;
	point.y = Lat_Degrees;

	ConvertLatLontoUTM (&point,Zone_Override,cActualZone);
	*ActualZone = atoi (cActualZone);
	*Easting = point.x;
	*Northing = point.y;
	return 0;
}
