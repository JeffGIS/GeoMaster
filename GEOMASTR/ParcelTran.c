#include "graphint.h"   

#define MAXPOINTLIST	4096 
#define MAXPOINTSPERLIST	USHRT_MAX

static char currentParcel[32];

#include "gmextern.h"

/*[~HTRAN] = $PARCELTRAN(INIT);
$RUNTEXTFILE([~TEMPFILE],
	$PARCELTRAN(ADD, [~HTRAN], [.TAG], $STR([PROPDATE] = 2014_04), $STR([PROPDATE] = 2014_10));
);
$PARCELTRAN(COMPUTE, [~HTRAN]);
$PARCELTRAN(OUTPUT, [~HTRAN], [~TRANFILE]);
$PARCELTRAN(FREE, [~HTRAN]);
*/


#define MAX_PARTRAN_POINTS	4096*4

void TranToParcelPoints(LPDOUBLE XIN, LPDOUBLE YIN, LPDOUBLE  XOUT, LPDOUBLE  YOUT, LPPARCELTRAN pTran)
{
	int i;
	LPDPOINT fromPt = GlobalLock(pTran->hFromPt);
	LPDPOINT toPt = GlobalLock(pTran->hToPt);
	DPOINT pt, outPt;
	double tol = 0.01;

	pt.x = *XIN;
	pt.y = *YIN;
	for (i = 0; i < pTran->np; i++)
	{
		if (ldistpp(&pt, &fromPt[i]) < tol)
		{
			*XOUT = toPt[i].x;
			*YOUT = toPt[i].y;
			goto Exit;
		}
	}
	outPt = TranPoint(&pt, pTran->hTran);
	*XOUT = outPt.x;
	*YOUT = outPt.y;
Exit:
	GlobalUnlock(pTran->hFromPt);
	GlobalUnlock(pTran->hToPt);
	return;
}

static BOOL pointsApproxTheSame(int npnts, HANDLE hPoly1, HANDLE hPoly2)
{
	double MAXD = 2;
	BOOL rtn = TRUE;
	LPDPOINT poly1 = (LPDPOINT)GlobalLock(hPoly1);
	LPDPOINT poly2 = (LPDPOINT)GlobalLock(hPoly2);
	double d;
	int i;

	for (i = 0; i < npnts; i++)
	{
		d = ldistpp(poly1++, poly2++);
		if (d > MAXD)
			rtn = FALSE;
	}

	GlobalUnlock(hPoly1);
	GlobalUnlock(hPoly2);
	return rtn;
}

static int findNearestPoint(LPDPOINT ppt, int npts, LPDPOINT points,LPINT pMatchedPoints, LPDOUBLE pMatchedDist,LPDOUBLE pminDist)
{
	double mind = DBL_MAX;
	int i, mini=-1;

	for (i = 0; i < npts; i++)
	{
		double d = ldistpp(ppt, &points[i]);
		if (pMatchedPoints[i] < 0 || d < pMatchedDist[i])
		{
			if (d < mind)
			{
				mind = d;
				mini = i;
			}
		}
	}
	*pminDist = mind;
	return mini;
}

static int pointInList(LPDPOINT pt, LPPARCELTRAN pParTran, double tol)
{
	double mind = DBL_MAX;
	double d;
	int i, rtn = -1;
	LPDPOINT fromPt = GlobalLock(pParTran->hFromPt);

	if (!pParTran->np)
		goto Exit;
	for (i = 0; i < pParTran->np; i++)
		if (ldistpp(pt, &fromPt[i]) < tol)
		{
			rtn = i;
			goto Exit;
		}
Exit:
	GlobalUnlock(pParTran->hFromPt);
	return rtn;
}

static void addPointToList(LPDPOINT fromPt, LPDPOINT toPt, LPPARCELTRAN pParTran)
{
	LPDPOINT ptfromPt = GlobalLock(pParTran->hFromPt);
	LPDPOINT pttoPt = GlobalLock(pParTran->hToPt);

	if (pParTran->np >= MAX_PARTRAN_POINTS - 1)
	{
		BlowOut("MAX_PARTRAN_POINTS exceeded", "");
	}
	ptfromPt[pParTran->np] = *fromPt;
	pttoPt[pParTran->np++] = *toPt;
	GlobalUnlock(pParTran->hFromPt);
	GlobalUnlock(pParTran->hToPt);

	return;
}

static void saveGoodPoints(int nGood, LPDPOINT pFrom, LPDPOINT pTo, LPPARCELTRAN pParTran)
{
	int i;

	for (i = 0; i < nGood; i++)
	{
		if (pointInList(&pFrom[i], pParTran, 0.01) < 0)
		{
			addPointToList(&pFrom[i], &pTo[i],pParTran);
		}
	}
}
static void dumpTranPoints(int npnts1, LPDPOINT poly1,LPINT pMatchedOldToNew, HANDLE hTran, int nTran)
{
	BOOL dumpPoints = TRUE;
	if (dumpPoints)
	{
		HFILE fid;
		char str[128];
		char fileName[MAX_PATH];
		int i;

		sprintf(fileName, "c:\\temp\\partran\\%s_%i.txt", currentParcel,nTran);
		fid = GSSiOpenFile(fileName, 0, OF_CREATE);
		sprintf(str, "X\tY\tMATCH");
		fputstring(str, fid);
		for (i = 0; i < npnts1; i++)
		{
			DPOINT pt = TranPoint(&poly1[i], hTran);
			sprintf(str, "%f\t%f\t%i", pt.x, pt.y, pMatchedOldToNew[i]);
			fputstring(str, fid);
		}
		GSSiClose2 (&fid);
	}
	return;
}

static BOOL loadParcelPoints(int npnts1, int npnts2, HANDLE hPoly1, HANDLE hPoly2, LPPARCELTRAN pParTran)
{
	double MAXD = GetGlobalDVal2 ("[%PARTRANTOL]",1.0);
	BOOL rtn = TRUE;
	LPDPOINT poly1 = (LPDPOINT)GlobalLock(hPoly1);
	LPDPOINT poly2 = (LPDPOINT)GlobalLock(hPoly2);
	double d, maxd=0;
	int i,j,nGood=0, lastGood = -1, nLoops = 0;
	MNMXCORD bounds1, bounds2;
	HANDLE hTran;
	HANDLE hMatchedPoints = GSSiGlobAlloc(GAIDNO 2098, GHND, npnts2 * (sizeof(int)+sizeof(double)+npnts1*sizeof(int)));
	LPINT pMatchedPoints = GlobalLock(hMatchedPoints);
	LPINT pMatchedOldToNew = &pMatchedPoints[npnts2];
	LPDOUBLE pMatchedDist = (LPDOUBLE)&pMatchedOldToNew[npnts1];
	HANDLE hGoodPoints = GSSiGlobAlloc(GAIDNO 2099, GMEM_MOVEABLE, npnts1 * 2 * sizeof(DPOINT));
	LPDPOINT goodPointsFrom = GlobalLock(hGoodPoints), goodPointsTo = &goodPointsFrom[npnts1];
	float RSQMIN;
	int nTran = 0;

	GetPolyBoundsD(hPoly1, npnts1, &bounds1, TYPE_AREA);
	GetPolyBoundsD(hPoly2, npnts2, &bounds2, TYPE_AREA);
	hTran = STRANBoundsToBounds(&bounds1, &bounds2);
	dumpTranPoints(npnts1, poly1, pMatchedOldToNew,hTran, nTran++);
	while (hTran)
	{
		for (i = 0; i < npnts1; i++)
			pMatchedOldToNew[i] = -1;
		for (i = 0; i < npnts2; i++)
			pMatchedPoints[i] = -1;
		for (i = 0; i < npnts1; i++)
		{
			int kNext = i;
			DPOINT pt;

			while (kNext >= 0)
			{
				int k = kNext;
				kNext = -1;
				pt = TranPoint(&poly1[k], hTran);
				j = findNearestPoint(&pt, npnts2, poly2, pMatchedPoints, pMatchedDist, &d);
				if (j < 0 || d > MAXD)
					rtn = FALSE;
				else
				{
					kNext = pMatchedPoints[j];
					if (kNext >= 0)
						ii = 1;
					pMatchedOldToNew[k] = j;
					pMatchedDist[j] = d;
					if (k == i)
						nGood++;
					pMatchedPoints[j] = k;
					maxd = max(d, maxd);
				}
			}
		}
		if (nGood == npnts1)
			CloseTRANS2(&hTran);
		else if (nGood<3 || nGood <= lastGood)
		{
			if (nLoops++ < 2)
			{
				MAXD *= 2;
				nGood = 0;
			}
			else
				CloseTRANS2(&hTran);
		}
		else
		{
			CloseTRANS2(&hTran);
			nGood = 0;
			for (i = 0; i < npnts1; i++)
			{
				if (pMatchedOldToNew[i] >= 0)
				{
					goodPointsFrom[nGood] = poly1[i];
					goodPointsTo[nGood++] = poly2[pMatchedOldToNew[i]];
				}
			}
			hTran = STRANPoints(0, goodPointsFrom, goodPointsTo, nGood, &RSQMIN, 1, 0);
			dumpTranPoints(npnts1, poly1, pMatchedOldToNew,hTran, nTran++);
			lastGood = nGood;
			nGood = 0;
			maxd = 0;
		}
	}
	nGood = 0;
	for (i = 0; i < npnts1; i++)
	{
		if (pMatchedOldToNew[i] >= 0)
		{
			goodPointsFrom[nGood] = poly1[i];
			goodPointsTo[nGood++] = poly2[pMatchedOldToNew[i]];
		}
	}
	GlobalUnlock(hPoly1);
	GlobalUnlock(hPoly2);
	saveGoodPoints(nGood, goodPointsFrom, goodPointsTo,pParTran);
	GSSiGlobUlFree(&hGoodPoints);
	GSSiGlobUlFree(&hMatchedPoints);
	return rtn;
}

BOOL ParcelTranFunction(int nArgs, LPSTR *Arg, LPSTR OutLoc)
{
	BOOL rtn = FALSE;
	HANDLE hParcelTran=0;
	int i;

	*OutLoc = 0;
	if (nArgs < 1)
		goto Exit;
	if (!stricmp(Arg[1], "INIT"))
	{
		int nParcels = atoi(Arg[2]);
		if (nParcels > 0)
		{
			HANDLE hParTran = GSSiGlobAlloc(GAIDNO 1793, GHND, sizeof(PARCELTRAN));
			LPPARCELTRAN pParTran = GlobalLock(hParTran);

			strcpy(pParTran->ID, "PARCELTRAN");
			pParTran->hParNumPt = GSSiGlobAlloc(GAIDNO 1794, GHND, nParcels*sizeof(int));
			pParTran->hpParPnts = GSSiGlobAlloc(GAIDNO 1794, GHND, nParcels*sizeof(HANDLE));
			pParTran->hFromPt = GSSiGlobAlloc(GAIDNO 1794, GMEM_MOVEABLE, MAX_PARTRAN_POINTS * sizeof(DPOINT));
			pParTran->hToPt = GSSiGlobAlloc(GAIDNO 1794, GMEM_MOVEABLE, MAX_PARTRAN_POINTS * sizeof(DPOINT));
			GlobalUnlock(hParTran);
			itoa((int)hParTran, OutLoc, 10);
			rtn = TRUE;
		}
		goto Exit;
	}
	if (!stricmp(Arg[1], "FREE"))
	{
		hParcelTran = (HANDLE)atoi(Arg[2]);
		if (hParcelTran)
		{
			LPPARCELTRAN pParTran = (LPPARCELTRAN)GlobalLock(hParcelTran);
			LPHANDLE phParPnts = GlobalLock(pParTran->hpParPnts);
			for (i = 0; i < pParTran->nParcels; i++,phParPnts++)
			{
				GSSiGlobFree(phParPnts);
			}
			GSSiGlobUlFree(&pParTran->hpParPnts);
			GSSiGlobFree(&pParTran->hParNumPt);
			GSSiGlobFree(&pParTran->hFromPt);
			GSSiGlobFree(&pParTran->hToPt);
			CloseTRANS2(&pParTran->hTran);
			GSSiGlobUlFree(&hParcelTran);
		}
		strcpy(OutLoc, "1");
		rtn = TRUE;
		goto Exit;
	}
	if (!stricmp(Arg[1], "ADD"))
	{
		int npnts1, npnts2;
		HANDLE hPoly1 = 0, hPoly2 = 0;
		LPSTR lpColon = strchr(Arg[3], ':');

		hParcelTran = (HANDLE)atoi(Arg[2]);
		LPPARCELTRAN pParTran = (LPPARCELTRAN)GlobalLock(hParcelTran);


		if (lpColon)
		{
			*lpColon++ = 0;
			strcpy(currentParcel, lpColon);
			ProcessText(Arg[4]);
			if (PickByRefno(0, Arg[3], lpColon,-1))
			{
				if (GetPolyPoints((LPPICKDATAHEADER)&PickList[0], FALSE, &npnts1, &hPoly1, 0))
				{
					LPINT	pnpt = (LPINT)GlobalLock(pParTran->hParNumPt);
					pnpt[pParTran->nParcels] = npnts1;
					GlobalUnlock(pParTran->hParNumPt);
					LPHANDLE phParPnts = (LPHANDLE)GlobalLock(pParTran->hpParPnts);
					phParPnts[pParTran->nParcels] = hPoly1;
					GlobalUnlock(pParTran->hpParPnts);
					ProcessText(Arg[5]);
					if (PickByRefno(0, Arg[3], lpColon, -1))
					{
						if (GetPolyPoints((LPPICKDATAHEADER)&PickList[0], FALSE, &npnts2, &hPoly2, 0))
						{
							loadParcelPoints(npnts1, npnts2, hPoly1, hPoly2, pParTran);
						}
					}

				}
				pParTran->nParcels++;
			}
			*lpColon = ':';
			rtn = TRUE;
		}
		GSSiGlobFree(&hPoly2);
		GlobalUnlock(hParcelTran);

		goto Exit;
	}
	if (!stricmp(Arg[1], "COMPUTE"))
	{
		hParcelTran = (HANDLE)atoi(Arg[2]);
		if (hParcelTran)
		{
			LPPARCELTRAN pParTran = (LPPARCELTRAN)GlobalLock(hParcelTran);
			if (pParTran->np > 2)
			{
				LPDPOINT fromPt = GlobalLock(pParTran->hFromPt);
				LPDPOINT toPt = GlobalLock(pParTran->hToPt);
				float RSQMIN;

				SetCurrentParcelTran(pParTran);
				WaitCursor(1);
				pParTran->hTran = STRANPoints(0, fromPt, toPt, pParTran->np, &RSQMIN, 3, 0);
				GlobalUnlock(pParTran->hFromPt);
				GlobalUnlock(pParTran->hToPt);
				WaitCursor(-1);
			}
			GlobalUnlock(hParcelTran);
		}

		goto Exit;
	}
	if (!stricmp(Arg[1], "OUTPUT"))
	{
		hParcelTran = (HANDLE)atoi(Arg[2]);
		if (hParcelTran)
		{
			LPPARCELTRAN pParTran = (LPPARCELTRAN)GlobalLock(hParcelTran);
			/*if (pParTran->np > 2)
			{
			LPDPOINT fromPt = GlobalLock(pParTran->hFromPt);
			LPDPOINT toPt = GlobalLock(pParTran->hToPt);
			float RSQMIN;

			pParTran->hTran = STRANPoints(0, fromPt, toPt, pParTran->np, &RSQMIN, 1, 0);
			GlobalUnlock(pParTran->hFromPt);
			GlobalUnlock(pParTran->hToPt);
			}*/
			GlobalUnlock(hParcelTran);
		}

		goto Exit;
	}

Exit:
	return rtn;
}

