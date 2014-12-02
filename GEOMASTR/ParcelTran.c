#include "graphint.h"   

#define MAXPOINTLIST	4096 
#define MAXPOINTSPERLIST	USHRT_MAX


#include "gmextern.h"

/*[~HTRAN] = $PARCELTRAN(INIT);
$RUNTEXTFILE([~TEMPFILE],
	$PARCELTRAN(ADD, [~HTRAN], [.TAG], $STR([PROPDATE] = 2014_04), $STR([PROPDATE] = 2014_10));
);
$PARCELTRAN(COMPUTE, [~HTRAN]);
$PARCELTRAN(OUTPUT, [~HTRAN], [~TRANFILE]);
$PARCELTRAN(FREE, [~HTRAN]);
*/

typedef struct { int np;
				HANDLE hFromPt;
				HANDLE hToPt;
				HANDLE hTran;
				int nParcels, nParcelsMatched;
} PARCELTRAN;
typedef PARCELTRAN *LPPARCELTRAN;

static BOOL pointsApproxTheSame(int npnts, HANDLE hPoly1, HANDLE hPoly2)
{
#define MAXD 2
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

BOOL ParcelTranFunction(int nArgs, LPSTR *Arg, LPSTR OutLoc)
{
	BOOL rtn = FALSE;
	HANDLE hParcelTran=0;

	if (nArgs < 1)
		goto Exit;
	if (!stricmp(Arg[1], "INIT"))
	{
		HANDLE hParTran = GSSiGlobAlloc(1793, GHND, sizeof(PARCELTRAN));
		itoa((int)hParTran, OutLoc, 10);
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

		pParTran->nParcels++;

		if (lpColon)
		{
			*lpColon = 0;
			ProcessText(Arg[4]);
			if (PickByRefno(0, Arg[3], ++lpColon,-1))
			{
				if (GetPolyPoints((LPPICKDATAHEADER)&PickList[0], FALSE, &npnts1, &hPoly1))
				{
					ProcessText(Arg[5]);
					if (PickByRefno(0, Arg[3], lpColon, -1))
					{
						if (GetPolyPoints((LPPICKDATAHEADER)&PickList[0], FALSE, &npnts2, &hPoly2))
						{
							if (npnts1 == npnts2 && pointsApproxTheSame(npnts1, hPoly1, hPoly2))
							{
								pParTran->nParcelsMatched++;
							}
						}
					}

				}
			}
			*lpColon = ':';
			rtn = TRUE;
		}
		GlobalUnlock(hParcelTran);

		goto Exit;
	}
	if (!stricmp(Arg[1], "COMPUTE"))
	{
		hParcelTran = (HANDLE)atoi(Arg[2]);
		LPPARCELTRAN pParTran = (LPPARCELTRAN)GlobalLock(hParcelTran);
		GlobalUnlock(hParcelTran);

		goto Exit;
	}

Exit:
	return rtn;
}

