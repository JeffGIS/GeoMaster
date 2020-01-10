#include "graphint.h"   

#define MAXPOINTLIST	4096 
#define MAXPOINTSPERLIST	USHRT_MAX


#include "gmextern.h"


static	HANDLE		hPointList[MAXPOINTLIST];
static	int			nPointLists=0;
static	int			nPointsInList[MAXPOINTLIST];
static	char		PointListID[MAXPOINTLIST][32];
static	BOOL		Closed[MAXPOINTLIST];

HANDLE GetPointListPoints (LPSTR ID,LPINT pnPoints)
{
	LPDPOINT Points1, Points2;
	HANDLE	hList=0;
	int	i,j;

	for (i=0;i<nPointLists;i++)
	{
		if (!stricmp (PointListID[i],ID))
		{
			*pnPoints = nPointsInList[i];
			if (nPointsInList[i])
			{
				hList = GSSiGlobAlloc (1571,GMEM_MOVEABLE,(nPointsInList[i])*sizeof(DPOINT));
				Points1 = GlobalLock (hPointList[i]);
				Points2 = GlobalLock (hList);
				for (j=0;j<nPointsInList[i];j++)
					Points2[j] = Points1[j];
				GlobalUnlock (hList);
				GlobalUnlock (hPointList[i]);
			}
		}
	}
	return hList;
}

BOOL PointCommands (int nArgs,LPSTR *Arg,LPSTR OutLoc)
{
	DPOINT	DPoint;
	POINT	Point;
	int		symnum;
	BOOL	err;
	double	width;

	if (!stricmp (Arg[1],"DISPLAY"))
	{
		DPoint = atopt (Arg[2],&err);
		if (!err)
		{
			Point = BasePtToWinPt (&DPoint);
			symnum = GetDictSymbolNumber (Arg[3]);
			width = atof (Arg[4]);
			if (symnum)
			{
				HANDLE hSymbol = GetDictSymDesc (symnum,0);

				DisplayPointSymbol (hSymbol,CurView->hDC, width, width,0, &Point,0,FALSE,0,0,FALSE,FALSE,Arg[6],0);  
				DestroySymbol (hSymbol);
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL PListCommands (int nArgs,LPSTR *Arg,LPSTR OutLoc)
{
	BOOL	rtn=FALSE;
//$PLIST(CREATE,LIST,CLOSED(TorF),ptlist)
//$PLIST(CREATE,HLT,CLOSED(TorF),hltitem)
//$PLIST(CREATE,ITEM,CLOSED(TorF),hltitem)
//$PLIST(DESTROY,id)
//$PLIST(BETWEENDIST,fromid)


	return rtn;
}
BOOL PointListCommands (int nArgs,LPSTR *Arg,LPSTR OutLoc)
{
	HANDLE	hList;
	HPDPOINT	Points1, Points2;
	DPOINT	Point;
	int		i,j,k,iList;
	BOOL	rtn=FALSE;
	double	az, pct;
	double	d, totd=0, d1, d2;
	BOOL	err;
	char	txt[128];
	int		nNewPoints;
	HANDLE	hNewPoints;
// Pointlist ID must contain at least one alpha, cannot contain spaces or colon and are 31 or less char.
// in the following, ptlist can be either a pointlist ID, list of points in text format, refno or TAG:UDI 
//$POINTLIST(CREATE,name,CLOSED(TorF),ptlist)
//$POINTLIST(CREATE,name,CLOSED(TorF),ITEM,tagorref)
//$POINTLIST(CREATE,name,CLOSED(TorF),HLT,hltno(def=1))
//$POINTLIST(CREATE,name,CLOSED(TorF),POINTLIST,plistname)
//$POINTLIST(CREATE,name,CLOSED(TorF)) creates null list to which points can be added
//$POINTLIST(CREATE,name,CLOSED(TorF),CIRCLE,center,radius,numpoints) 
//$POINTLIST(DESTROY,name)
//$POINTLIST(ADD,name,ptlist)
//$POINTLIST(THIN,name,dist(if 0 removes dup points))
//$POINTLIST(DISPLAY,name,FILL,color)
//$POINTLIST(DISPLAY,name,DRAW,color,width)
//$POINTLIST(LENGTH,name)
//$POINTLIST(AREA,name)
//$POINTLIST(PCT,name,point)
//$POINTLIST(AZM,name,pct,before;after;at(default) at averages before and after if at node point. all 3 the same if not at node pt)
//$POINTLIST(INTERSECT,name,name2,COUNT;id;Farthest;nearest,farornearpoint)
//$POINTLIST(DIST,name,pctfrom,pctto)
//$POINTLIST(BETWEENDIST,name,fromdist,todist)
//$POINTLIST(BOUNDS,name)
//$POINTLIST(POINTATDIST,name,dist);

	if (nArgs < 0)
	{
DestroyAll:
		for (i=0;i<nPointLists;i++)
		{
			GSSiGlobFree (&hPointList[i]);
			nPointsInList[i] = 0;
		}
		nPointLists = 0;
		return TRUE;
	}
	if (nArgs < 1)
		return FALSE;
	if (!stricmp (Arg[1],"CREATE"))
	{
		iList = -1;
		for (i=0;i<nPointLists;i++)
		{
			if (!stricmp (PointListID[i],Arg[2]))
			{
				if (nPointsInList[i])
				{
					GSSiGlobFree (&hPointList[i]);
					nPointsInList[i] = 0;
				}
				iList = i;
			}
		}
		if (iList < 0)
		{
			iList = nPointLists;
			if (nPointLists == MAXPOINTLIST)
			{
				MessageBox (0,"Number of point lists exceeds maximum",0,MB_ICONEXCLAMATION);
				return FALSE;
			}
			strncpy0 (PointListID[nPointLists],Arg[2],31);
		}
		Closed[iList] = atob (Arg[3]);
		if (!*Arg[4])
		{
			nPointsInList[iList] = 0;
			hPointList[iList] = 0;
			if (iList == nPointLists)
				nPointLists++;
			return TRUE;
		}
		else if (strchr (Arg[4],' '))
		{
			nPointsInList[iList] = GetPointsFromList (Arg[4],&hList);
			if (nPointsInList[iList])
			{
				hPointList[iList] = GSSiGlobAlloc (1596,GMEM_MOVEABLE,(nPointsInList[iList])*sizeof(DPOINT));
				Points1 = GlobalLock (hList);
				Points2 = GlobalLock (hPointList[iList]);
				memcpy (Points2,Points1,nPointsInList[iList]*sizeof(DPOINT));
				GSSiGlobUlFree (&hList);
				GlobalUnlock (hPointList[iList]);
			}
			if (iList == nPointLists)
				nPointLists++;
			return TRUE;
		}
		else if (!stricmp (Arg[4],"HLT")) //item in highlight list
		{
			if (GetPolyPnts ((LPPICKDATAHEADER)&PickList[0],FALSE,&nPointsInList[iList],
																  &hPointList[iList],TRUE))
			{
				if (iList == nPointLists)
					nPointLists++;
				rtn = TRUE;
			}
		}
		else if (!stricmp(Arg[4], "ITEM")) //TAG:UDI
		{
		}
		else if (!stricmp(Arg[4], "CIRCLE"))
		{
			BOOL err;
			DPOINT center = atopt(Arg[5],&err);
			double radius = atof(Arg[6]);
			int npt = atoi(Arg[7]);
			nPointsInList[iList] = npt + 1;
			if (nPointsInList[iList])
			{
				hPointList[iList] = GSSiGlobAlloc(1597, GMEM_MOVEABLE, (nPointsInList[iList]) * sizeof(DPOINT));
				Points1 = GlobalLock(hPointList[iList]);
				double azinc = TWOPI / npt;
				double az = 0;
				for (int i = 0; i < npt; i++)
				{
					Points1[i] = dnewpt(center, az, radius);
					az += azinc;
				}
				Points1[npt] = Points1[0];
				GlobalUnlock (hPointList[iList]);
				if (iList == nPointLists)
					nPointLists++;
				rtn = TRUE;
			}

		}
		else if (!stricmp (Arg[4],"POINTLIST"))
		{
			for (i=0;i<nPointLists;i++)
			{
				if (!stricmp (PointListID[i],Arg[5]))
				{
					nPointsInList[iList] = nPointsInList[i];
					if (nPointsInList[iList])
					{
						hPointList[iList] = GSSiGlobAlloc (1597,GMEM_MOVEABLE,(nPointsInList[iList])*sizeof(DPOINT));
						Points1 = GlobalLock (hPointList[i]);
						Points2 = GlobalLock (hPointList[iList]);
						memcpy (Points2,Points1,nPointsInList[iList]*sizeof(DPOINT));
						GlobalUnlock (hPointList[i]);
						GlobalUnlock (hPointList[iList]);
					}
					if (iList == nPointLists)
						nPointLists++;
					rtn = TRUE;
					break;
				}
			}
		}
		else
		{
			for (i=0;i<nPointLists;i++)
			{
				if (!stricmp (PointListID[i],Arg[3]))
				{

					nPointsInList[iList] = nPointsInList[i];
					if (nPointsInList[iList])
					{
						hPointList[iList] = GSSiGlobAlloc (1597,GMEM_MOVEABLE,(nPointsInList[iList])*sizeof(DPOINT));
						Points1 = GlobalLock (hPointList[i]);
						Points2 = GlobalLock (hPointList[iList]);
						memcpy (Points2,Points1,nPointsInList[iList]*sizeof(DPOINT));
						GlobalUnlock (hPointList[i]);
						GlobalUnlock (hPointList[iList]);
					}
					if (iList == nPointLists)
						nPointLists++;
					rtn = TRUE;
					break;
				}
			}

		}
	}
	else if (!stricmp(Arg[1], "POINTATDIST"))
	{
		double Dist = atof(Arg[3]);

		for (i = 0; i < nPointLists; i++)
		{
			if (!stricmp(PointListID[i], Arg[2]) && nPointsInList[i] > 1)
			{
				Points1 = GlobalLock(hPointList[i]);
				DPOINT atPoint = PointAtDistOnPoly(Points1, nPointsInList[i], Dist, 0,0);
				sprintf(OutLoc, "%f %f", atPoint.x, atPoint.y);
				GlobalUnlock(hPointList[i]);
				rtn = TRUE;
				break;
			}
		}
	}
	else if (!stricmp(Arg[1], "BETWEENDIST"))
	{
		double fromDist = atof(Arg[3]);
		double toDist = atof(Arg[4]);

		for (i = 0; i < nPointLists; i++)
		{
			if (!stricmp(PointListID[i], Arg[2]) && nPointsInList[i] > 1)
			{
				Points1 = GlobalLock(hPointList[i]);
				hNewPoints = GetPolyBetweenDist(Points1, nPointsInList[i],
					fromDist, toDist, &nNewPoints, FALSE, FALSE);
				GSSiGlobUlFree(&hPointList[i]);
				hPointList[i] = hNewPoints;
				nPointsInList[i] = nNewPoints;
				rtn = TRUE;
				break;
			}
		}
	}
	else if (!stricmp (Arg[1],"INTERSECT"))
	{
		for (i=0;i<nPointLists;i++)
		{
			if (!stricmp (PointListID[i],Arg[2]) && nPointsInList[i] > 1)
			{
				for (j=0;j<nPointLists;j++)
				{
					if (!stricmp (PointListID[j],Arg[3]) && nPointsInList[j] > 1)
					{
						short	Type1=GF_POLYLINE,Type2=GF_POLYLINE;
						DPOINT	PickPoint = atopt (Arg[5],&err), IntPoint;
						double	D1,D2;

						Points1 = GlobalLock (hPointList[i]);
						Points2 = GlobalLock (hPointList[j]);
						if (Closed[i])
							Type1 = GF_AREA;
						if (Closed[j])
							Type2 = GF_AREA;
						rtn = IntersectPolys1 (Type1,Type2,nPointsInList[i],Points1,0,nPointsInList[j],Points2,0,0,&PickPoint,&IntPoint,&D1,&D2,0);
						GlobalUnlock (hPointList[i]);
						GlobalUnlock (hPointList[j]);
						dpointtoa (OutLoc,&IntPoint);
						rtn = TRUE;
						break;
					}
				}
			}
		}

	}

	else if (!stricmp (Arg[1],"DISPLAY"))
	{
		HBRUSH	hBrush, hOldBrush=0;
		HPEN	hPen, hOldPen=0;
		COLORREF	Color;

		for (i=0;i<nPointLists;i++)
		{
			if (!stricmp (PointListID[i],Arg[2]))
			{
				if (nPointsInList[i])
				{
					Points1 = GlobalLock (hPointList[i]);
					if (!stricmp (Arg[3],"FILL"))
					{
						Color = atoi (Arg[4]);
						hBrush = CreateSolidBrush(Color);
						hOldBrush = SelectObject (CurView->hDC,hBrush); 
						GWPolygonD (CurView->hDC,Points1,nPointsInList[i],0,0,0,TRUE,TRUE,0);  
						SelectObject (CurView->hDC,hOldBrush);
						DeleteObject (hBrush);
						rtn = TRUE;
					}
					else if (!stricmp (Arg[3],"DRAW"))
					{
						Color = atoi (Arg[4]);
						hPen = CreatePen (PS_SOLID,atoi (Arg[5]),Color);
						hOldPen = SelectObject (CurView->hDC,hPen); 
						GWPolylineD (CurView->hDC,Points1,nPointsInList[i],0);  
						SelectObject (CurView->hDC,hOldPen);
						DeleteObject (hPen);
						rtn = TRUE;
					}
					else if (!stricmp (Arg[3],"NUMBER"))
					{
						BOOL	SaveDM=DisplayMarkers;

						DisplayMarkers = TRUE; 
						for (j=0;j<nPointsInList[i];j++)
						{
							itoa (j,txt,10);
							DisplayMarker (*Points1++,2,txt,0.16,0,0,FALSE,FALSE,0,0,0,0,0);
						}
						DisplayMarkers = SaveDM; 
						rtn = TRUE;
					}
					GlobalUnlock (hPointList[i]);
				}
				break;
			}
		}

	}
	else if (!stricmp (Arg[1],"PCT"))
	{
		for (i=0;i<nPointLists;i++)
		{
			if (!stricmp (PointListID[i],Arg[2]) && nPointsInList[i]>1)
			{
				HPDPOINT	Points = GlobalLock (hPointList[i]);
				double len= GetPolyLengthD (Points,nPointsInList[i]);
				DPOINT	atpt = atopt (Arg[3],&err);

				totd = 0;
				if (len && !err)
				{
					for (j=0;j<nPointsInList[i]-1;j++)
					{
						if (SameDPoint (&atpt,&Points[j]))
							break;
						d = ldistpp (&Points[j],&Points[j+1]);
						d1 = ldistpp (&Points[j],&atpt);
						d2 = ldistpp (&Points[j+1],&atpt);
						if (d1 < d && d2 < d)
						{
							totd += d1;
							break;
						}
						totd += d;
					}
					pct = totd / len;
					rtn = TRUE;
				}
				GlobalUnlock (hPointList[i]);
				ftoa (OutLoc,pct);
				rtn = TRUE;
				break;
			}

		}
	}
	else if (!stricmp (Arg[1],"AZM"))
	{
		for (i=0;i<nPointLists;i++)
		{
			if (!stricmp (PointListID[i],Arg[2]) && nPointsInList[i]>1)
			{
				HPDPOINT	Points = GlobalLock (hPointList[i]);
				double len = GetPolyLengthD (Points,nPointsInList[i]);
				double tol = P_TOL / len;
				double az1, az2;
				double wantpct = atof (Arg[3]), atpct=0, totd=0;


				if (wantpct < 0)
				{
					if (Closed[i])
						wantpct = 1.0 - fmod (wantpct,1.0);
					else
						wantpct = P_TOL;
				}
				else if (wantpct > 1)
				{
					if (Closed)
						wantpct = fmod (wantpct,1.0);
					else
						wantpct = 1.0 - P_TOL;
				}
				
				for (j=0;j<nPointsInList[i];j++)
				{
					if (j)
					{
						d = ldistpp (&Points[j],&Points[j-1]);
						totd += d;
						atpct = totd / len;
					}
					if (fabs (atpct - wantpct) < tol)
					{
						DPOINT	PointBefore, PointAfter;

						if (!j)
							PointBefore = Points[nPointsInList[i]-1];
						else
							PointBefore = Points[j-1];
						if (j == nPointsInList[i]-1)
							PointAfter = Points[0];
						else
							PointAfter = Points[j+1];
						if (!stricmp (Arg[4],"BEFORE"))
						{
							az = getazd (&PointBefore,&Points[j]);
							GlobalUnlock (hPointList[i]);
							ftoa (OutLoc,az);
							return TRUE;
						}
						if (!stricmp (Arg[4],"AFTER"))
						{
							az = getazd (&Points[j],&PointAfter);
							GlobalUnlock (hPointList[i]);
							ftoa (OutLoc,az);
							return TRUE;
						}
						az1 = getazd (&PointBefore,&Points[j]);
						az2 = getazd (&Points[j],&PointAfter);
						az = LTWOPI (az1 + DeltaAZ (az1, az2)/2);
						GlobalUnlock (hPointList[i]);
						ftoa (OutLoc,az);
						return TRUE;
					}
					else if (atpct > wantpct)
					{
						az = getazd (&Points[max(0,j-1)],&Points[max(0,j-1)+1]);
						GlobalUnlock (hPointList[i]);
						ftoa (OutLoc,az);
						return TRUE;
					}
				}
				az = getazd (&Points[nPointsInList[i]-1],&Points[0]);
				GlobalUnlock (hPointList[i]);
				ftoa (OutLoc,az);
				rtn = TRUE;
				break;
			}
		}
	
	}

	else if (!stricmp (Arg[1],"DIST"))
	{
		for (i=0;i<nPointLists;i++)
		{
			if (!stricmp (PointListID[i],Arg[2]) && nPointsInList[i]>1)
			{
				HPDPOINT	Points = GlobalLock (hPointList[i]);
				double len= GetPolyLengthD (Points,nPointsInList[i]);
				double	pctfrom = atof (Arg[3]);
				double	pctto	= atof (Arg[4]);
				double	d, d2;


				if (pctfrom > pctto)
				{
					d = pctfrom - pctto;
					if (Closed[i])
					{
						d2 = 1.0 - pctfrom + pctto;
						if (d2 < d)
							d = d2;
					}
				}
				else
				{
					d = pctto - pctfrom;
					if (Closed[i])
					{
						d2 = 1.0 - pctto + pctfrom;
						if (d2 < d)
							d = d2;
					}
				}

				GlobalUnlock (hPointList[i]);
				ftoa (OutLoc,d * len);
				rtn = TRUE;
				break;
			}
		}

	}

	else if (!stricmp (Arg[1],"REMOVE"))
	{
		for (i=0;i<nPointLists;i++)
		{
			if (!stricmp (PointListID[i],Arg[2]) && nPointsInList[i]>1)
			{
				HPDPOINT	Points = GlobalLock (hPointList[i]);
				double		len= GetPolyLengthD (Points,nPointsInList[i]);
				double		tol = P_TOL / len;
				double		frompct = atof (Arg[3]);
				double		throughpct = atof (Arg[4]);
				double		topct = atof (Arg[5]);
				int			nparts=1, np;
				double		begpct[2], endpct[2];
				double		atd, atpct, doff, nextd, nextpct;
				BOOL		skip;
				HANDLE		hList = GSSiGlobAlloc (0,GMEM_MOVEABLE,(nPointsInList[i]+2)*sizeof(DPOINT));
				
				if (frompct < throughpct && throughpct < topct)
				{
					nparts = 1;
					begpct[0] = frompct;
					endpct[0] = topct;
				}
				else if (frompct > throughpct && throughpct > topct)
				{
					nparts = 1;
					begpct[0] = topct;
					endpct[0] = frompct;
				}
				else if (throughpct == 0 && frompct < topct)
				{
					nparts = 2;
					begpct[0] = -0.1;
					endpct[0] = frompct;
					begpct[1] = topct;
					endpct[1] = 1.1;
				}
				else if (throughpct == 0 && frompct > topct)
				{
					nparts = 2;
					begpct[0] = -0.1;
					endpct[0] = topct;
					begpct[1] = frompct;
					endpct[1] = 1.1;
				}
				else if (frompct > throughpct && throughpct < topct ||
						 frompct < throughpct && throughpct > topct)
				{
					nparts = 2;
					begpct[0] = -0.1;
					endpct[0] = frompct;
					begpct[1] = topct;
					endpct[1] = 1.1;
				}
				else
				{
					nparts = 2;
					begpct[0] = topct;
					endpct[0] = 1.1;
					begpct[1] = -0.1;
					endpct[1] = frompct;
				}
				Points2 = GlobalLock (hList);
				atd = nextd = 0;
				atpct = 0;
				np = 0;
				for (j=0;j<nPointsInList[i];j++)
				{
					skip = FALSE;
					for (k=0;k<nparts;k++)
					{
						if (atpct > begpct[k]+tol && atpct < endpct[k]-tol)
							skip = TRUE;
					}
					if (!skip)
						Points2[np++] = Points[j];
					if (j <nPointsInList[i]-1)
					{
						d = ldistpp (&Points[j],&Points[j+1]);
						nextd += d;
						nextpct = nextd / len;
						if (frompct > atpct+tol && frompct < nextpct-tol)
						{
							az = getazd (&Points[j],&Points[j+1]);
							doff = (frompct - atpct) * len;
							Points2[np++] = dnewpt (Points[j],az,doff);
						}
						if (topct > atpct+tol && topct < nextpct-tol)
						{
							az = getazd (&Points[j],&Points[j+1]);
							doff = (topct - atpct) * len;
							Points2[np++] = dnewpt (Points[j],az,doff);
						}
					}
					atd = nextd;
					atpct = nextpct;
				}
				GSSiGlobUlFree (&hPointList[i]);
				GlobalUnlock (hList);
				hPointList[i] = hList;
				nPointsInList[i] = np;
				strcpy (OutLoc,"1");
				rtn = TRUE;
				break;
			}
		}

	}

	else if (!stricmp (Arg[1],"THIN"))
	{
		for (i=0;i<nPointLists;i++)
		{
			if (!stricmp (PointListID[i],Arg[2]) && nPointsInList[i]>1)
			{
				HPDPOINT	Points = GlobalLock (hPointList[i]);
				double		thindist = atof (Arg[3]);
				
				ThinPoly (&nPointsInList[i],Points, max (P_TOL,thindist));
				GlobalUnlock (hPointList[i]);
				rtn = TRUE;
				break;
			}
		}

	}

	else if (!stricmp (Arg[1],"LENGTH"))
	{
		for (i=0;i<nPointLists;i++)
		{
			if (!stricmp (PointListID[i],Arg[2]) && nPointsInList[i]>1)
			{
				HPDPOINT	Points = GlobalLock (hPointList[i]);
				double len= GetPolyLengthD (Points,nPointsInList[i]);

				GlobalUnlock (hPointList[i]);
				ftoa (OutLoc,len);
				rtn = TRUE;
				break;
			}
		}

	}

	else if (!stricmp (Arg[1],"BOUNDS"))
	{
		for (i=0;i<nPointLists;i++)
		{
			if (!stricmp (PointListID[i],Arg[2]) && nPointsInList[i]>1)
			{
				int type = TYPE_POLYLINE;
				MNMXCORD bounds;

				if (Closed[i])
					type = TYPE_AREA;

				GetPolyBoundsD (hPointList[i],nPointsInList[i],&bounds,type);
				boundstoa (OutLoc,&bounds);
				rtn = TRUE;
				break;
			}
		}

	}

	else if (!stricmp (Arg[1],"AREA"))
	{
		for (i=0;i<nPointLists;i++)
		{
			if (!stricmp (PointListID[i],Arg[2]) && nPointsInList[i]>1)
			{
				HPDPOINT	Points = GlobalLock (hPointList[i]);
				double area = ComputeAreaAreaD (Points,nPointsInList[i],0);

				GlobalUnlock (hPointList[i]);
				ftoa (OutLoc,area);
				rtn = TRUE;
				break;
			}
		}

	}

	else if (!stricmp (Arg[1],"REVERSE"))
	{
		for (i=0;i<nPointLists;i++)
		{
			if (!stricmp (PointListID[i],Arg[2]) && nPointsInList[i]>1)
			{
				HPDPOINT	Points = GlobalLock (hPointList[i]);
				HANDLE		hList = GSSiGlobAlloc (0,GMEM_MOVEABLE,nPointsInList[i]*sizeof(DPOINT));
				
				Points2 = GlobalLock (hList);
				k = nPointsInList[i] - 1;
				for (j=0;j<nPointsInList[i];j++)
					Points2[j] = Points[k--];
				GSSiGlobUlFree (&hPointList[i]);
				GlobalUnlock (hList);
				hPointList[i] = hList;
				strcpy (OutLoc,"1");
				rtn = TRUE;
				break;
			}
		}
	}

	else if (!stricmp (Arg[1],"DESTROY"))
	{
		if (!*Arg[2])
			goto DestroyAll;
		for (i=0;i<nPointLists;i++)
		{
			if (!stricmp (PointListID[i],Arg[2]))
			{
				rtn = TRUE;
				GSSiGlobFree (&hPointList[i]);
				nPointsInList[i] = 0;
				if (i != nPointLists-1)
				{
					memmove (&nPointsInList[i],&nPointsInList[i+1],(nPointLists-i-1)*sizeof(int));
					memmove (&hPointList[i],&hPointList[i+1],(nPointLists-i-1)*sizeof(HANDLE));
					memmove (PointListID[i],PointListID[i+1],(nPointLists-i-1)*32);
				}
				nPointLists--;
			}
		}
	}
	return rtn;
}

